
#include "reduce.h"
#include "plans.h"
#include "forward.h"
#include "bfs.h"
#include "ida.h"

BEGIN_HSPS_NAMESPACE

NTYPE Reduce::implement
(index_type act, NTYPE bound, Result& imps)
{
  Instance::Action& as = instance.actions[act];

  if (trace_level > 1) {
    std::cerr << "searching for implementations of "
	      << as.name << "..." << std::endl;
  }

  // save instance state
  bool_vec saved_sel(false, instance.n_actions());
  for (index_type k = 0; k < instance.n_actions(); k++)
    saved_sel[k] = instance.actions[k].sel;
  index_set saved_init(instance.init_atoms);
  index_set saved_goal(instance.goal_atoms);

  // compute protected atoms
  Heuristic* inc = inconsistency();
  bool own_inc = false;
  if (instance.n_invariants() > 0) {
    inc = new RegressionInvariantCheck(instance, *inc, true);
    own_inc = true;
  }

  bool_vec prot_atoms(true, instance.n_atoms());
  for (index_type k = 0; k < as.del.length(); k++)
    prot_atoms[as.del[k]] = false;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (prot_atoms[k] && !as.pre.contains(k)) {
      NTYPE v = inc->incremental_eval(as.pre, k);
      if (INFINITE(v)) prot_atoms[k] = false;
    }

  if (own_inc) delete inc;

  if (trace_level > 2) {
    std::cerr << "init atoms: ";
    instance.write_atom_set(std::cerr, as.pre);
    std::cerr << std::endl;
    std::cerr << "protected atoms: ";
    instance.write_atom_set(std::cerr, prot_atoms);
    std::cerr << " (" << prot_atoms.count(true)
	      << " of " << instance.n_atoms()
	      << ")" << std::endl;
    std::cerr << "goal atoms: ";
    instance.write_atom_set(std::cerr, as.add);
    std::cerr << std::endl;
  }

  // set and solve search problem
  NTYPE min_imp_cost = POS_INF;

  // remove the action to be implemented
  instance.actions[act].sel = false;

  // remove actions that delete protected atoms
  index_type n_sel = 0;
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].sel) {
      if (prot_atoms.contains_any(instance.actions[k].del))
	instance.actions[k].sel = false;
      else
	n_sel += 1;
    }

  // set initial and goal state
  instance.set_initial(as.pre);
  instance.set_goal(as.add);

  if (trace_level > 2) {
    std::cerr << n_sel << " of " << instance.n_actions()
	      << " actions remain selectable" << std::endl;
  }

  UnitACF a;
  ZeroHeuristic h(instance);
  SeqProgState root(instance, h, a, as.pre);

  if (root.is_final()) {
    min_imp_cost = 0;
  }
  else {
    BFS search(stats, imps);
    search.set_cost_limit(bound);
    search.set_trace_level(trace_level - 1);
    search.start(root);
    if (search.solved()) {
      min_imp_cost = search.cost();
    }
  }

  // restore instance to original state
  for (index_type k = 0; k < instance.n_actions(); k++)
    instance.actions[k].sel = saved_sel[k];
  instance.set_initial(saved_init);
  instance.set_goal(saved_goal);

  return min_imp_cost;
}

index_pair Reduce::count_useless_and_redundant_actions(NTYPE bound)
{
  index_pair count(0, 0);
  Result imps;
  imps.set_n_to_find(1);
  if (trace_level > 2) {
    imps.set_plan_set(new Print(instance, std::cerr));
  }

  stats.start();
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].sel) {
      if (trace_level > 1) {
	std::cerr << "checking action " << k << ". "
		  << instance.actions[k].name << std::endl;
      }

      NTYPE imp_cost = implement(k, bound, imps);

      if (imp_cost == 0) {
	if (trace_level > 0) {
	  std::cerr << "action " << k << ". " << instance.actions[k].name
		    << " is useless" << std::endl;
	}
	count.second += 1;
      }
      else if (FINITE(imp_cost)) {
	if (trace_level > 0) {
	  std::cerr << "action " << k << ". " << instance.actions[k].name
		    << " is redundant (" << stats << ")" << std::endl;
	}
	count.first += 1;
      }
      else if (trace_level > 1) {
	std::cerr << "action " << k << ". " << instance.actions[k].name
		  << " is NOT redundant (" << stats << ")" << std::endl;
      }
    }

  stats.stop();
  if (trace_level > 0) {
    std::cerr << "reduction complete (" << stats << ")" << std::endl;
  }
  return count;
}

void Reduce::reduced_action_set(NTYPE bound, bool_vec& deleted)
{
  if (trace_level > 0) {
    std::cerr << "computing reduced action set with bound "
	      << bound << std::endl;
  }
  stats.start();

  // save instance state
  bool_vec saved_sel(false, instance.n_actions());
  for (index_type k = 0; k < instance.n_actions(); k++)
    saved_sel[k] = instance.actions[k].sel;

  Result imps;
  imps.set_n_to_find(1);
  if (trace_level > 2) {
    imps.set_plan_set(new Print(instance, std::cerr));
  }

  deleted.assign_value(false, instance.n_actions());
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].sel) {
      if (trace_level > 1) {
	std::cerr << "checking action " << k << ". "
		  << instance.actions[k].name << std::endl;
      }
      NTYPE imp_cost = implement(k, bound, imps);
      if (imp_cost == 0) {
	if (trace_level > 0) {
	  std::cerr << "action " << k << ". " << instance.actions[k].name
		    << " is useless" << std::endl;
	}
	deleted[k] = true;
	instance.actions[k].sel = false;
      }
      else if (FINITE(imp_cost)) {
	if (trace_level > 0) {
	  std::cerr << "action " << k << ". " << instance.actions[k].name
		    << " is redundant (" << stats << ")" << std::endl;
	}
	deleted[k] = true;
	instance.actions[k].sel = false;
      }
      else if (trace_level > 1) {
	std::cerr << "action " << k << ". " << instance.actions[k].name
		  << " is NOT redundant (" << stats << ")" << std::endl;
      }
    }

  // restore instance
  for (index_type k = 0; k < instance.n_actions(); k++)
    instance.actions[k].sel = saved_sel[k];

  stats.stop();
  if (trace_level > 0) {
    std::cerr << "reduction complete (" << stats << ")" << std::endl;
  }
}

void Reduce::reduce(NTYPE bound)
{
  bool_vec deleted;
  reduced_action_set(bound, deleted);
  if (deleted.count(true) > 0) {
    if (trace_level > 0) {
      std::cerr << "removing " << deleted.count(true) << " actions..."
		<< std::endl;
    }
    instance.remove_actions(deleted, action_map);
    instance.clear_cross_reference();
  }
  else if (trace_level > 0) {
    std::cerr << "no redundant actions" << std::endl;
  }
}

END_HSPS_NAMESPACE
