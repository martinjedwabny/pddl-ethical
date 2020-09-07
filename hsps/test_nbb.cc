
#include "soft.h"
#include "parser.h"
#include "preprocess.h"
#include "exec.h"
#include "cost_table.h"
#include "enumerators.h"

#include <sstream>
#include <fstream>

BEGIN_HSPS_NAMESPACE

StringTable symbols(50, lowercase_map);
Parser* reader = new Parser(symbols);

int main(int argc, char *argv[]) {
  bool     opt_preprocess = true;
  bool     opt_nvz = false;
  bool     opt_cost = true;
  bool     opt_count = true;
  bool     opt_print_bounds = false;
  bool     opt_save_dp = false;
  bool     opt_print_atoms = false;
  bool     opt_solved_only = false;
  bool     opt_improving_only = false;
  bool     opt_tight_only = false;
  bool     opt_open_only = false;
  NTYPE    nb_best = NEG_INF;
  index_type fix_selection_size = no_such_index;
  index_type max_size = no_such_index;
  bool     opt_H2 = true;
  bool     opt_H3 = false;
  bool     opt_AH = false;
  bool     opt_load_partition = false;
  bool     opt_ia_partition = false;
  bool     opt_load_h = true;
  bool     opt_store = false;
  unsigned int random_seed = 0;

  PDDL_Base::write_warnings = false;
  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      Instance::default_trace_level = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-extended-action-definition") == 0) {
      ExecState::extended_action_definition = true;
      PDDL_Base::compile_away_disjunctive_preconditions = false;
      PDDL_Base::compile_away_conditional_effects = false;
      PDDL_Base::create_all_atoms = true;
      opt_preprocess = false;
    }
    else if (strcmp(argv[k],"-no-prep") == 0) {
      opt_preprocess = false;
    }
    else if (strcmp(argv[k],"-nvz") == 0) {
      opt_nvz = true;
    }
    else if (strcmp(argv[k],"-no-cost") == 0) {
      opt_cost = false;
    }
    else if (strcmp(argv[k],"-print-bounds") == 0) {
      opt_print_bounds = true;
    }
    else if (strcmp(argv[k],"-print-atoms") == 0) {
      opt_print_atoms = true;
    }
    else if (strcmp(argv[k],"-nsn") == 0) {
      Instance::write_atom_set_with_symbolic_names = false;
    }
    else if (strcmp(argv[k],"-solved") == 0) {
      opt_solved_only = true;
    }
    else if (strcmp(argv[k],"-improving") == 0) {
      opt_improving_only = true;
    }
    else if (strcmp(argv[k],"-tight") == 0) {
      opt_tight_only = true;
      opt_open_only = false;
    }
    else if (strcmp(argv[k],"-open") == 0) {
      opt_open_only = true;
      opt_tight_only = false;
    }
    else if ((strcmp(argv[k],"-best-known") == 0) && (k < argc - 1)) {
      nb_best = A_TO_N(argv[++k]);
    }
    else if ((strcmp(argv[k],"-max-size") == 0) && (k < argc - 1)) {
      max_size = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-fix-ss") == 0) && (k < argc - 1)) {
      fix_selection_size = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-no-count") == 0) {
      opt_count = false;
    }
    else if (strcmp(argv[k],"-save") == 0) {
      opt_save_dp = true;
    }
    else if ((strcmp(argv[k],"-catc") == 0) && (k < argc - 1)) {
      HSPS::PDDL_Name::catc = *argv[++k];
    }
    else if (strcmp(argv[k],"-3") == 0) {
      opt_H3 = true;
    }
    else if (strcmp(argv[k],"-no-h") == 0) {
      opt_H2 = false;
      opt_H3 = false;
      opt_AH = false;
    }
    else if (strcmp(argv[k],"-AH") == 0) {
      opt_AH = true;
    }
    else if (strcmp(argv[k],"-load-p") == 0) {
      opt_load_partition = true;
    }
    else if (strcmp(argv[k],"-pia") == 0) {
      opt_ia_partition = true;
    }
    else if (strcmp(argv[k],"-load") == 0) {
      opt_load_h = true;
    }
    else if (strcmp(argv[k],"-no-load") == 0) {
      opt_load_h = false;
    }
    else if (strcmp(argv[k],"-store") == 0) {
      opt_store = true;
    }
    else if (((strcmp(argv[k],"-rnd") == 0) ||
	      (strcmp(argv[k],"-r") == 0)) &&
	     (k < argc - 1)) {
      random_seed = atoi(argv[++k]);
    }
    else if (*argv[k] != '-') {
      reader->read(argv[k], false);
    }
  }

  Statistics stats;
  stats.enable_interrupt(true);
  SoftInstance instance;

  reader->post_process();

  stats.start();
  reader->instantiate(instance);
  reader->instantiate_soft(instance);

  Preprocessor prep(instance, stats);

  if (opt_preprocess) {
    prep.preprocess();
    instance.remap_hard_goals(prep.atom_map);
    instance.remap_soft_goals(prep.atom_map);
  }
  else {
    instance.cross_reference();
  }

  if (opt_nvz) instance.null_value = 0;

  stats.stop();
  std::cerr << "instantiation and preprocessing finished in "
	    << stats.time() << " seconds" << std::endl;

  std::cerr << "goal atoms = ";
  instance.write_atom_set(std::cerr, instance.goal_atoms);
  std::cerr << std::endl;

  CostTable* ub = new CostTable(instance, stats, POS_INF);
  Heuristic* lb;
  NTYPE      v_best = instance.null_value;
  index_set_vec g_best;
  if (v_best > nb_best) {
    nb_best = v_best;
    g_best.append(EMPTYSET);
  }
  index_type s_min = no_such_index;
  index_type s_max = no_such_index;

  ACF* acf = (opt_cost ? (ACF*)new CostACF(instance) : (ACF*)new ZeroACF());

  if (opt_AH || opt_H2 || opt_H3) {
    std::cerr << "computing heuristic..." << std::endl;
    stats.start();
    if (opt_AH) {
      AH* h = new AH(instance, stats);
      if (opt_load_partition) {
	name_vec pnames(0, 0);
	index_set_vec partition;
	reader->export_action_partitions(pnames, partition);
	if (partition.length() == 0) {
	  std::cerr << "warning: action partitioning not defined by domain"
		    << " - trying alternate load method..." << std::endl;
	  name_vec anames(0, 0);
	  instance.action_names(anames);
	  reader->export_sets(anames, partition);
	}
	if (partition.length() == 0) {
	  std::cerr << "warning: no action partitioning loaded" << std::endl;
	}
	instance.remap_sets(partition, prep.action_map);
	h->compute_additive_H2(*acf, partition);
	h->compute_max_H2(*acf);
      }
      else if (opt_ia_partition) {
	h->compute_with_iterative_assignment(*acf, instance.goal_atoms,
					     true, false, true);
      }
      else {
	h->compute_with_relevance_partitioning(*acf, instance.goal_atoms);
      }
      lb = h;
    }
    else if (opt_H3 || opt_H2) {
      CostTable* h = new CostTable(instance, stats);
      if (opt_H3) {
	h->compute_H3(*acf);
      }
      else {
	h->compute_H2(*acf);
      }
      lb = h;
    }

    stats.stop();
    std::cerr << "heuristic computed in " << stats.time() << " seconds"
	      << std::endl;
  }
  else {
    lb = new ZeroHeuristic(instance);
  }

  if (opt_load_h) {
    std::cerr << "loading additional heuristic information..." << std::endl;
    stats.start();
    reader->export_heuristic(instance, prep.atom_map, true, *lb);
    stats.stop();
    std::cerr << "finished in " << stats.time() << " seconds" << std::endl;
  }

  std::cerr << "analyzing " << reader->n_plans() << " plans..." << std::endl;
  stats.start();
  for (index_type k = 0; k < reader->n_plans(); k++) {
    Schedule* plan = new Schedule(instance);
    reader->export_plan(k, instance, prep.action_map, *plan);
    index_set ach;
    ExecErrorSet* errors = new ExecErrorSet();
    bool ok = plan->simulate(ach, errors);
    if (!ok) {
      std::cerr << "plan " << k << " not executable: ";
      errors->write(std::cerr);
      std::cerr << std::endl;
    }
    else {
      ok = ach.contains(instance.hard);
      if (!ok) {
	std::cerr << "plan " << k << " fails to achieve hard goals: ";
	index_set u(instance.hard);
	u.subtract(ach);
	instance.write_atom_set(std::cerr, u);
	std::cerr << std::endl;
      }
      else {
	// ach.intersect(instance.goal_atoms);
	index_set g;
	NTYPE v_plan = instance.eval_goal_state(ach, g);
	NTYPE nb_plan = v_plan - plan->cost();
	std::cerr << "plan " << k << " achieves ";
	instance.write_atom_set(std::cerr, ach);
	std::cerr << " at cost " << PRINT_NTYPE(plan->cost());
	if (plan->plan_is_optimal()) {
	  std::cerr << " (optimal)";
	}
	std::cerr << ", soft goals achieved are ";
	instance.write_soft_goal_set(std::cerr, g);
	std::cerr << " with a value of " << PRINT_NTYPE(v_plan)
		  << " for a net benefit of " << PRINT_NTYPE(nb_plan)
		  << std::endl;
	if (!ach.empty()) {
	  NTYPE c_max = ub->eval_min(ach);
	  if (plan->cost() < c_max) {
	    ub->store(ach, plan->cost());
	  }
	  if (opt_store && plan->plan_is_optimal()) {
	    NTYPE c_min = lb->eval(ach);
	    if (plan->cost() > c_min) {
	      std::cerr << "storing lb(";
	      instance.write_atom_set(std::cerr, ach);
	      std::cerr << ") = " << PRINT_NTYPE(plan->cost())
			<< std::endl;
	      lb->store(ach, plan->cost(), true);
	    }
	  }
	}
	if (v_plan > v_best) v_best = v_plan;
	if (nb_plan > nb_best) {
	  nb_best = nb_plan;
	  g_best.assign_value(g, 1);
	}
	else if (nb_plan == nb_best) {
	  g_best.append(g);
	}
      }
    }
    delete plan;
    delete errors;
  }

  stats.stop();
  std::cerr << reader->n_plans() << " plans analyzed in "
	    << stats.time() << " seconds" << std::endl;

//   std::cerr << "lower bounds:" << std::endl;
//   ((CostTable*)lb)->write(std::cerr);
//   std::cerr << "upper bounds:" << std::endl;
//   ub->write(std::cerr);

  index_type n_goal_sets = 0;
  NTYPE      nb_maybe_best = nb_best;
  index_set_vec g_maybe_best(EMPTYSET, 0);

  if (opt_print_bounds || opt_count) {
    SubsetEnumerator* e =
      (fix_selection_size == no_such_index ?
       (SubsetEnumerator*)new SubsetEnumerator(instance.n_soft()) :
       (SubsetEnumerator*)new mSubsetEnumerator(instance.n_soft(), fix_selection_size));
    bool more = e->first();
    while (more) {
      index_set s(instance.hard);
      NTYPE     v = instance.null_value;
      for (index_type k = 0; k < instance.n_soft(); k++)
	if (e->current_set()[k]) {
	  s.insert(instance.soft[k].atoms);
	  v += instance.soft[k].weight;
	}
      NTYPE c_min = lb->eval(s);
      NTYPE c_max = ub->eval_min(s);
      if (FINITE(c_max) || !opt_solved_only) {
	NTYPE nb_max = v - c_min;
	NTYPE nb_min = v - c_max;
	if (((nb_max > nb_best) || !opt_improving_only) &&
	    ((nb_min == nb_max) || !opt_tight_only) &&
	    ((nb_min < nb_max) || !opt_open_only) &&
	    ((s.length() <= max_size) || (max_size == no_such_index))) {
	  if (opt_print_bounds) {
	    std::cout << "set ";
	    instance.write_soft_goal_set(std::cout, e->current_set());
	    std::cout << " (" << s.length() << " atoms";
	    if (opt_print_atoms) {
	      std::cout << ", ";
	      instance.write_atom_set(std::cout, s);
	    }
	    std::cout << ")";
	    std::cout << ": value = " << PRINT_NTYPE(v)
		      << ", " << PRINT_NTYPE(c_min)
		      << " <= cost <= " << PRINT_NTYPE(c_max)
		      << ", " << PRINT_NTYPE(nb_min)
		      << " <= NB <= " << PRINT_NTYPE(nb_max)
		      << std::endl;
	  }
	  if (opt_save_dp) {
	    Instance* dp = new Instance();
	    if (opt_cost) {
	      instance.create_decision_problem(e->current_set(), nb_best, *dp);
	      Instance::write_PDDL3 = false;
	      Instance::write_PDDL2 = true;
	      Instance::write_metric = true;
	      Instance::write_time = false;
	    }
	    else {
	      instance.create_decision_problem(e->current_set(), *dp);
	      Instance::write_PDDL3 = false;
	      Instance::write_PDDL2 = false;
	      Instance::write_metric = false;
	      Instance::write_time = false;
	    }
	    std::ostringstream fname;
	    char* b = reader->problem_file_basename();
	    if (b) {
	      fname << b << "-dp" << n_goal_sets + 1 << ".pddl";
	    }
	    else {
	      fname << "dp" << n_goal_sets + 1 << ".pddl";
	    }
	    std::ofstream s_out(fname.str().c_str());

	    s_out << ";; " << instance.name << ", decision problem #"
		  << (n_goal_sets + 1) << std::endl;
	    s_out << ";; soft goals: ";
	    instance.write_soft_goal_set(s_out, e->current_set());
	    s_out << std::endl;
	    s_out << ";; goal atoms: ";
	    instance.write_atom_set(s_out, s);
	    s_out << std::endl;
	    s_out << ";; value = " << v
		  << ", min est. cost = " << c_min
		  << ", max NB = " << nb_max
		  << std::endl;
	    s_out << ";; min NB req. = " << nb_best
		  << ", cost limit = " << (v - nb_best)
		  << std::endl;

	    dp->write_domain_init(s_out);
	    dp->write_domain_declarations(s_out);
	    dp->write_domain_actions(s_out);
	    dp->write_domain_DKEL_items(s_out);
	    if (HSPS::Instance::write_extra) {
	      name_vec set_names(0, 0);
	      index_set_vec sets;
	      reader->export_action_partitions(set_names, sets);
	      s_out << ";; action partitions" << std::endl;
	      for (index_type k = 0; k < sets.length(); k++) {
		instance.remap_set(sets[k], prep.action_map);
		instance.write_domain_action_set(s_out, sets[k], set_names[k]);
	      }
	    }
	    s_out << ")";
	    dp->write_problem(s_out);
	    s_out.close();
	    delete dp;
	  }
	  if (nb_max > nb_maybe_best) {
	    nb_maybe_best = nb_max;
	    g_maybe_best.assign_value(e->current_set(), 1);
	  }
	  else if (nb_max == nb_maybe_best) {
	    g_maybe_best.append(e->current_set());
	  }
	  if ((s_min == no_such_index) || (s.length() < s_min))
	    s_min = s.length();
	  if ((s_max == no_such_index) || (s.length() > s_max))
	    s_max = s.length();
	  n_goal_sets += 1;
	}
      }
      more = e->next();
    }
    std::cout << n_goal_sets << " soft goals sets matching criteria found"
	      << std::endl;
    std::cout << "smallest set size: " << s_min << std::endl;
    std::cout << "largest set size: " << s_max << std::endl;
  }

  std::cout << "max value: " << PRINT_NTYPE(v_best) << std::endl;
  std::cout << "max net benefit " << PRINT_NTYPE(nb_best)
	    << " ACHIEVED by " << g_best.length() << " soft goal sets: ";
  for (index_type k = 0; k < g_best.length(); k++) {
    if (k > 0) std::cout << ", ";
    instance.write_soft_goal_set(std::cout, g_best[k]);
  }
  std::cout << std::endl;
  std::cout << "max net benefit " << PRINT_NTYPE(nb_maybe_best)
	    << " POSSIBLE for " << g_maybe_best.length()
	    << " soft goal sets: ";
  for (index_type k = 0; k < g_maybe_best.length(); k++) {
    if (k > 0) std::cout << ", ";
    instance.write_soft_goal_set(std::cout, g_maybe_best[k]);
  }
  std::cout << std::endl;
}

END_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
