
#include "preprocess.h"
#include "cost_table.h"
#include "enumerators.h"

#include <list>

BEGIN_HSPS_NAMESPACE

int Preprocessor::default_trace_level = 0;

Preprocessor::Preprocessor(Instance& ins, Statistics& s)
  : instance(ins),
    atom_map(no_such_index, 0),
    action_map(no_such_index, 0),
    inv_map(no_such_index, 0),
    stats(s),
    inc_relation(0),
    inc_graph(0),
    trace_level(default_trace_level)
{
  atom_map.set_length(instance.n_atoms());
  for (index_type k = 0; k < instance.n_atoms(); k++) atom_map[k] = k;
  action_map.set_length(instance.n_actions());
  for (index_type k = 0; k < instance.n_actions(); k++) action_map[k] = k;
  inv_map.set_length(instance.n_invariants());
  for (index_type k = 0; k < instance.n_invariants(); k++) inv_map[k] = k;
}

Preprocessor::~Preprocessor()
{
  // done
}

CostTable* Preprocessor::inconsistency()
{
  if (inc_relation != 0) {
    if (inc_relation->size() != instance.n_atoms()) {
      if (trace_level > 0) {
	std::cerr << "inconsistency relation invalidated by changes to instance..." << std::endl;
      }
      delete inc_relation;
      inc_relation = 0;
    }
  }
  if (inc_relation == 0) {
    stats.start();
    if (!instance.cross_referenced()) {
      if (trace_level > 0) {
	std::cerr << "cross referencing..." << std::endl;
      }
      instance.cross_reference();
    }
    if (trace_level > 0) {
      std::cerr << "computing inconsistency relation..." << std::endl;
    }
    inc_relation = new CostTable(instance, stats);
    inc_relation->compute_H2(ZeroACF());
    if (trace_level > 0) {
      std::cerr << "constructing inconsisteny graph..." << std::endl;
    }
    inc_graph = new graph(instance.n_atoms());
    for (index_type i = 0; i < instance.n_atoms(); i++)
      for (index_type j = i+1; j < instance.n_atoms(); j++)
	if (INFINITE(inc_relation->eval(i, j)))
	  inc_graph->add_undirected_edge(i, j);
    stats.stop();
  }
  return inc_relation;
}

graph* Preprocessor::inconsistency_graph()
{
  if (inc_graph == 0) {
    inconsistency();
  }
  return inc_graph;
}

void Preprocessor::compute_reachability
(bool_vec& reachable_atoms, bool_vec& reachable_actions, bool opt_H2)
{
  CostTable reachable(instance, stats);
  if (opt_H2) {
    reachable.compute_H2(ZeroACF());
  }
  else {
    reachable.compute_H1(ZeroACF());
  }
  reachable_atoms.set_length(instance.atoms.length());
  for (index_type k = 0; k < instance.atoms.length(); k++) {
    NTYPE c = reachable.eval(k);
    if (!FINITE(c)) reachable_atoms[k] = false;
    else reachable_atoms[k] = true;
  }
  reachable_actions.set_length(instance.actions.length());
  for (index_type k = 0; k < instance.actions.length(); k++) {
    NTYPE c = reachable.eval(instance.actions[k].pre);
    if (!FINITE(c)) reachable_actions[k] = false;
    else reachable_actions[k] = true;
  }
}

void Preprocessor::compute_static_atoms
(const bool_vec& reachable_actions, bool_vec& static_atoms)
{
  static_atoms.set_length(instance.atoms.length());
  for (index_type k = 0; k < instance.atoms.length(); k++)
    static_atoms[k] = instance.atoms[k].init;
  for (index_type k = 0; k < instance.actions.length(); k++) if (reachable_actions[k]) {
    for (index_type i = 0; i < instance.actions[k].del.length(); i++)
      static_atoms[instance.actions[k].del[i]] = false;
    for (index_type i = 0; i < instance.actions[k].lck.length(); i++)
      static_atoms[instance.actions[k].lck[i]] = false;
  }
//    static_atoms.set_length(instance.atoms.length());
//    for (index_type k = 0; k < instance.atoms.length(); k++) {
//      if (instance.atoms[k].init) {
//        bool deletable = false;
//        for (index_type i = 0; (i < instance.atoms[k].del_by.length()) &&
//  	     !deletable; i++) {
//  	if (reachable_actions[instance.atoms[k].del_by[i]]) deletable = true;
//        }
//        if (!deletable) static_atoms[k] = true;
//        else static_atoms[k] = false;
//      }
//      else static_atoms[k] = false;
//    }
}

void Preprocessor::compute_ncw_sets()
{
  stats.start();
  for (index_type k = 0; k < instance.n_actions(); k++)
    instance.actions[k].ncw_atms.clear();
  for (index_type k = 0; k < instance.n_actions(); k++)
    for (index_type i = 0; i < instance.n_atoms(); i++) {
      bool ce = false;
      for (index_type j = 0; (j < instance.atoms[i].add_by.length()) && !ce; j++) {
	if (stats.break_signal_raised()) return;
	if (instance.non_interfering(k, instance.atoms[i].add_by[j]) &&
	    instance.lock_compatible(k, instance.atoms[i].add_by[j]))
	  ce = true;
      }
      if (!ce) instance.actions[k].ncw_atms.insert(i);
    }
  stats.stop();
}

void Preprocessor::compute_ncw_sets(Heuristic& inc)
{
  stats.start();
  for (index_type k = 0; k < instance.n_actions(); k++)
    instance.actions[k].ncw_atms.clear();
  for (index_type k = 0; k < instance.n_actions(); k++)
    for (index_type i = 0; i < instance.n_atoms(); i++) {
      bool ce = false;
      for (index_type j = 0; (j < instance.atoms[i].add_by.length()) && !ce; j++) {
	if (stats.break_signal_raised()) return;
	if (instance.non_interfering(k, instance.atoms[i].add_by[j]) &&
	    instance.lock_compatible(k, instance.atoms[i].add_by[j])) {
	  index_set p(instance.actions[k].pre);
	  p.insert(instance.actions[instance.atoms[i].add_by[j]].pre);
	  if (FINITE(inc.eval(p))) ce = true;
	}
      }
      if (!ce) instance.actions[k].ncw_atms.insert(i);
    }
  stats.stop();
}

void Preprocessor::compute_relevance
(const index_set& check,
 index_type d_check,
 index_vec& d_atm,
 index_vec& d_act)
{
  for (index_type k = 0; k < check.length(); k++)
    if (d_atm[check[k]] > d_check) {
      d_atm[check[k]] = d_check;
      for (index_type i = 0; i < instance.atoms[check[k]].add_by.length(); i++)
	if (d_act[instance.atoms[check[k]].add_by[i]] > d_check) {
	  d_act[instance.atoms[check[k]].add_by[i]] = d_check;
	  compute_relevance(instance.actions[instance.atoms[check[k]].add_by[i]].pre, d_check + 1, d_atm, d_act);
	}
    }
}

void Preprocessor::compute_relevance
(const index_set& check, bool_vec& r_atm, bool_vec& r_act)
{
  for (index_type k = 0; k < check.length(); k++)
    if (!r_atm[check[k]]) {
      r_atm[check[k]] = true;
      for (index_type i = 0; i < instance.atoms[check[k]].add_by.length(); i++)
	if (!r_act[instance.atoms[check[k]].add_by[i]]) {
	  r_act[instance.atoms[check[k]].add_by[i]] = true;
	  compute_relevance(instance.actions[instance.atoms[check[k]].add_by[i]].pre, r_atm, r_act);
	}
    }
}

void Preprocessor::between
(index_type p,
 index_type q,
 const graph& lmg,
 bool_vec& b_atm,
 bool_vec& b_act)
{
  if (!b_atm[q]) {
    b_atm[q] = true;
    for (index_type i = 0; i < instance.atoms[q].add_by.length(); i++)
      if (!b_act[instance.atoms[q].add_by[i]]) {
	b_act[instance.atoms[q].add_by[i]] = true;
	for (index_type j = 0; j < instance.actions[instance.atoms[q].add_by[i]].pre.length(); j++)
	  if ((instance.actions[instance.atoms[q].add_by[i]].pre[j] != p) &&
	      lmg.adjacent(p, instance.actions[instance.atoms[q].add_by[i]].pre[j]))
	    between(p, instance.actions[instance.atoms[q].add_by[i]].pre[j], lmg, b_atm, b_act);
      }
  }
}

void Preprocessor::compute_L1C1_relevance
(const index_set& check,
 index_type d_check,
 const graph& lmg,
 index_vec& d_atm,
 index_vec& d_act)
{
  for (index_type k = 0; k < check.length(); k++)
    if (d_atm[check[k]] > d_check) {
      Instance::Atom& atm = instance.atoms[check[k]];
      d_atm[atm.index] = d_check;
      if (trace_level > 1) {
	std::cerr << "atom " << atm.name << " is relevant" << std::endl;
	if (atm.goal) {
	  std::cerr << " - it is a goal" << std::endl;
	}
	for (index_type i = 0; i < atm.req_by.length(); i++)
	  if (d_act[atm.req_by[i]] != no_such_index) {
	    std::cerr << " - it is a precondition of relevant action "
		      << instance.actions[atm.req_by[i]].name
		      << std::endl;
	  }
      }
      for (index_type i = 0; i < atm.add_by.length(); i++) {
        Instance::Action& act = instance.actions[atm.add_by[i]];
	if (d_act[act.index] > d_check) {
	  bool ok = true;
	  for (index_type j = 0; (j < act.pre.length()) && ok; j++)
	    if (lmg.adjacent(atm.index, act.pre[j]))
	      ok = false;
	  if (ok) {
	    if (trace_level > 1) {
	      std::cerr << act.name << " is an L1(C1)-relevant establisher of "
			<< atm.name << std::endl;
	    }
	    d_act[act.index] = d_check;
	    compute_L1C1_relevance(act.pre, d_check + 1, lmg, d_atm, d_act);
	  }
	}
      }
    }
}

void Preprocessor::compute_L1C1_relevance
(const index_set& check,
 const graph& lmg,
 bool_vec& r_atm,
 bool_vec& r_act)
{
  for (index_type k = 0; k < check.length(); k++)
    if (!r_atm[check[k]]) {
      Instance::Atom& atm = instance.atoms[check[k]];
      r_atm[atm.index] = true;
      for (index_type i = 0; i < atm.add_by.length(); i++)
	if (!r_act[atm.add_by[i]]) {
	  Instance::Action& act = instance.actions[atm.add_by[i]];
	  bool ok = true;
	  for (index_type j = 0; (j < act.pre.length()) && ok; j++)
	    if (lmg.adjacent(atm.index, act.pre[j]))
	      ok = false;
	  if (ok) {
	    r_act[act.index] = true;
	    compute_L1C1_relevance(act.pre, lmg, r_atm, r_act);
	  }
	}
    }
}

void Preprocessor::extend_LsC1_relevance
(const index_set& check,
 const graph& lmg,
 const bool_vec& p_act,
 const bool_vec& p_add,
 bool_vec& rel_atms,
 bool_vec& rel_acts)
{
  for (index_type k = 0; k < check.length(); k++)
    if (!rel_atms[check[k]]) {
      Instance::Atom& atm = instance.atoms[check[k]];
      rel_atms[atm.index] = true;
      for (index_type i = 0; i < atm.add_by.length(); i++)
	if (!rel_acts[atm.add_by[i]]) {
	  Instance::Action& act = instance.actions[atm.add_by[i]];
	  bool ok = true;
	  if (!p_act[atm.add_by[i]])
	    for (index_type j = 0; (j < act.pre.length()) && ok; j++)
	      if (lmg.adjacent(atm.index, act.pre[j])) {
		bool_vec b_atm(false, instance.n_atoms());
		bool_vec b_act(false, instance.n_actions());
		between(atm.index, act.pre[j], lmg, b_atm, b_act);
		// std::cerr << "extend L*(C1): atom " << atm.name
		// 	  << " prec. of action " << act.name
		// 	  << ", between = ";
		// instance.write_atom_set(std::cerr, b_atm);
		// std::cerr << std::endl;
		// std::cerr << "b_atm = " << b_atm << std::endl;
		// std::cerr << "p_add = " << p_add << std::endl;
		// std::cerr << "first common element = "
		// 	  << p_add.first_common_element(b_atm)
		// 	  << std::endl;
		if (p_add.first_common_element(b_atm) == no_such_index)
		  ok = false;
	      }
	  if (ok) {
	    rel_acts[act.index] = true;
	    extend_LsC1_relevance(act.pre, lmg, p_act, p_add, rel_atms, rel_acts);
	  }
	}
    }
}

void Preprocessor::compute_LsC1_relevance
(const index_set& check,
 const graph& lmg,
 bool_vec& rel_atms,
 bool_vec& rel_acts)
{
  compute_L1C1_relevance(check, lmg, rel_atms, rel_acts);
  bool_vec p_add(false, instance.n_atoms());
  index_type n = 0;
  bool done = false;
  while (!done) {
    n += 1;
    std::cerr << "iteration #" << n << ", " << rel_acts.count(true)
	      << " relevant actions" << std::endl;
    for (index_type k = 0; k < instance.n_actions(); k++) if (rel_acts[k]) {
      for (index_type i = 0; i < instance.actions[k].add.length(); i++)
	p_add[instance.actions[k].add[i]] = true;
    }
    bool_vec new_rel_atms(false, instance.n_atoms());
    bool_vec new_rel_acts(false, instance.n_actions());
    extend_LsC1_relevance(check, lmg, rel_acts, p_add, new_rel_atms, new_rel_acts);
    if (rel_acts.contains(new_rel_acts)) {
      done = true;
    }
    else {
      rel_atms.insert(new_rel_atms);
      rel_acts.insert(new_rel_acts);
    }
  }
}

void Preprocessor::compute_relaxed_relevance
(const index_set& g,
 index_vec& path,
 const index_set& pre,
 bool_vec& rel_acts)
{
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].add.count_common(pre) > 0) {
      std::cerr << "action " << instance.actions[k].name
		<< " connects to ";
      instance.write_atom_set(std::cerr, pre);
      std::cerr << std::endl;
      bool is_legal = true;
      index_set u_pre(instance.actions[k].pre);
      path.append(k);
      for (index_type i = path.length(); (i > 1) && is_legal; i--) {
	index_set cf(instance.actions[path[i - 2]].pre);
	cf.intersect(instance.actions[path[i - 1]].add);
	if (u_pre.contains(cf)) {
	  std::cerr << "path ";
	  for (index_type j = path.length(); j > 0; j--) {
	    if (j < path.length()) std::cerr << ",";
	    std::cerr << instance.actions[path[j - 1]].name;
	  }
	  std::cerr << " is NOT legal: atoms ";
	  instance.write_atom_set(std::cerr, cf);
	  std::cerr << " connecting " << i - 1  << "th add to "
		    << i - 2 << "th pre contained in U_pre = ";
	  instance.write_atom_set(std::cerr, u_pre);
	  std::cerr << std::endl;
	  is_legal = false;
	}
	u_pre.insert(instance.actions[path[i - 2]].pre);
      }
      if (is_legal) {
	index_set cf(g);
	cf.intersect(instance.actions[path[0]].add);
	if (u_pre.contains(cf)) {
	  std::cerr << "path ";
	  for (index_type j = path.length(); j > 0; j--) {
	    if (j < path.length()) std::cerr << ",";
	    std::cerr << instance.actions[path[j - 1]].name;
	  }
	  std::cerr << " is NOT legal: atoms ";
	  instance.write_atom_set(std::cerr, cf);
	  std::cerr << " connecting last add to goal contained in U_pre = ";
	  instance.write_atom_set(std::cerr, u_pre);
	  std::cerr << std::endl;
	  is_legal = false;
	}
      }
      if (is_legal) {
	std::cerr << "path ";
	for (index_type j = path.length(); j > 0; j--) {
	  if (j < path.length()) std::cerr << ",";
	  std::cerr << instance.actions[path[j - 1]].name;
	}
	std::cerr << " is legal" << std::endl;
	rel_acts[k] = true;
	compute_relaxed_relevance(g, path, instance.actions[k].pre, rel_acts);
      }
      path.dec_length();
    }
    else {
      std::cerr << "action " << instance.actions[k].name
		<< " does NOT connect to ";
      instance.write_atom_set(std::cerr, pre);
      std::cerr << std::endl;
    }
}

void Preprocessor::compute_irrelevant_atoms()
{
  stats.start();
  bool_vec r_atm(false, instance.n_atoms());
  bool_vec r_act(false, instance.n_actions());
  compute_relevance(instance.goal_atoms, r_atm, r_act);
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (!r_atm[k]) instance.atoms[k].irrelevant = true;
  stats.stop();
}

void Preprocessor::relevant_at_cost
(index_type p, NTYPE c, Heuristic& h, const ACF& f,
 bool_vec& r_atoms, bool_vec& r_actions)
{
  r_atoms[p] = true;
  for (index_type k = 0; k < instance.atoms[p].add_by.length(); k++) {
    Instance::Action& act = instance.actions[instance.atoms[p].add_by[k]];
    if (!r_actions[act.index]) {
      NTYPE c_pre = h.eval(act.pre);
      NTYPE c_post = c - f(act.index);
      if (c_pre <= c_post) {
	r_actions[act.index] = true;
	for (index_type i = 0; i < act.pre.length(); i++) {
	  relevant_at_cost(act.pre[i], c_post, h, f, r_atoms, r_actions);
	}
      }
    }
  }
}

void Preprocessor::relevant_at_cost
(const index_set& s, NTYPE c, Heuristic& h, const ACF& f,
 bool_vec& r_atoms, bool_vec& r_actions)
{
  for (index_type k = 0; k < s.length(); k++)
    relevant_at_cost(s[k], c, h, f, r_atoms, r_actions);
}

void Preprocessor::strictly_relevant_actions
(index_type p, NTYPE c, Heuristic& h, const ACF& f, bool_vec& rel)
{
  for (index_type k = 0; k < instance.atoms[p].add_by.length(); k++) {
    Instance::Action& act = instance.actions[instance.atoms[p].add_by[k]];
    if (!rel[act.index]) {
      NTYPE c_pre = h.eval(act.pre);
      NTYPE c_post = c - f(act.index);
      if (c_pre <= c_post) {
	rel[act.index] = true;
	for (index_type i = 0; i < act.pre.length(); i++) {
	  strictly_relevant_actions(act.pre[i], c_post, h, f, rel);
	}
      }
    }
  }
}

void Preprocessor::strictly_relevant_actions
(index_type p, Heuristic& h, const ACF& f, bool_vec& rel)
{
  rel.assign_value(false, instance.n_actions());
  index_set s;
  s.assign_singleton(p);
  strictly_relevant_actions(p, h.eval(s), h, f, rel);
}

void Preprocessor::remove_useless_actions()
{
  bool_vec u_act(false, instance.n_actions());
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].pre.contains(instance.actions[k].add)) {
      if (trace_level > 1) {
	std::cerr << "action " << instance.actions[k].name
		  << " is useless" << std::endl;
      }
      u_act[k] = true;
    }
  instance.remove_actions(u_act, action_map);
}

void Preprocessor::remove_useless_invariants()
{
  bool_vec u_inv(false, instance.n_invariants());
  for (index_type k = 0; k < instance.n_invariants(); k++)
    if (instance.invariants[k].set.length() <= instance.invariants[k].lim) {
      if (trace_level > 1) {
	std::cerr << "invariant |";
	instance.write_atom_set(std::cerr, instance.invariants[k].set);
	std::cerr << "| " << (instance.invariants[k].exact ? "==" : "<=")
		  << " " << instance.invariants[k].lim << " is useless"
		  << std::endl;
      }
      u_inv[k] = true;
    }
  instance.remove_invariants(u_inv, inv_map);
}

void Preprocessor::preprocess(bool opt_H2) {
  stats.start();
  instance.clear_cross_reference();

  if (trace_level > 0)
    std::cerr << "removing inconsistent & useless actions..." << std::endl;
  bool_vec inc(false, instance.n_actions());
  bool_vec useless(true, instance.n_actions());
  for (index_type k = 0; k < instance.n_actions(); k++) {
    // check for inconsistency...
    for (index_type i = 0; (i < instance.actions[k].add.length()) &&
	   !inc[k]; i++)
      if (instance.actions[k].del.contains(instance.actions[k].add[i]))
	inc[k] = true;
    if (inc[k] && (trace_level > 1)) {
      std::cerr << "action " << instance.actions[k].name
		<< " is inconsistent" << std::endl;
    }
    // check for uselessness...
    for (index_type i = 0; (i < instance.actions[k].add.length()) &&
	   useless[k]; i++)
      if (!instance.actions[k].pre.contains(instance.actions[k].add[i]))
	useless[k] = false;
    if (useless[k] && (trace_level > 1)) {
      std::cerr << "action " << instance.actions[k].name
		<< " is useless" << std::endl;
    }
  }
  inc.insert(useless);
  instance.remove_actions(inc, action_map);

  if (trace_level > 0) std::cerr << "cross referencing..." << std::endl;
  instance.cross_reference();

  if (trace_level > 0) std::cerr << "computing reachability..." << std::endl;
  bool_vec reachable_atoms(false, instance.n_atoms());
  bool_vec reachable_actions(false, instance.n_actions());
  compute_reachability(reachable_atoms, reachable_actions, opt_H2);

  if (trace_level > 0) std::cerr << "computing static atoms..." << std::endl;
  bool_vec atoms_to_remove(false, instance.n_atoms());
  compute_static_atoms(reachable_actions, atoms_to_remove);

  reachable_actions.complement();
  if (trace_level > 0) {
    std::cerr << "removing unreachable actions..." << std::endl;
    if (trace_level > 1) {
      instance.write_action_set(std::cerr, reachable_actions);
      std::cerr << std::endl;
    }
  }
  instance.remove_actions(reachable_actions, action_map);

  reachable_atoms.complement();
  atoms_to_remove.insert(reachable_atoms);

  // make sure we don't remove unreachable goals!
  for (index_type k = 0; k < instance.atoms.length(); k++)
    if (instance.atoms[k].goal) atoms_to_remove[k] = false;

  if (trace_level > 0) {
    std::cerr << "removing unreachable and static atoms..." << std::endl;
    if (trace_level > 1) {
      instance.write_atom_set(std::cerr, atoms_to_remove);
      std::cerr << std::endl;
    }
  }

  if (atoms_to_remove.count(true) > 0) {
    instance.remove_atoms(atoms_to_remove, atom_map);
    remove_useless_actions();
    remove_useless_invariants();
  }

  if (trace_level > 0) std::cerr << "re-cross referencing..." << std::endl;
  instance.clear_cross_reference();
  instance.cross_reference();
  stats.stop();
}

void Preprocessor::remove_irrelevant_atoms()
{
  stats.start();
  bool_vec atoms_to_remove(false, instance.n_atoms());
  for (index_type k = 0; k < instance.atoms.length(); k++)
    if (instance.atoms[k].irrelevant) atoms_to_remove[k] = true;

  if (trace_level > 0) {
    std::cerr << "removing irrelevant atoms..." << std::endl;
    if (trace_level > 1) {
      instance.write_atom_set(std::cerr, atoms_to_remove);
      std::cerr << std::endl;
    }
  }

  if (atoms_to_remove.count(true) > 0) {
    instance.remove_atoms(atoms_to_remove, atom_map);
    remove_useless_actions();
    remove_useless_invariants();
    instance.clear_cross_reference();
  }
  stats.stop();
}

void Preprocessor::remove_redundant_preconditions()
{
  stats.start();
  for (index_type k = 0; k < instance.n_actions(); k++) {
    index_set r;
    for (index_type i = 0; i < instance.actions[k].pre.length(); i++)
      for (index_type j = 0; j < instance.actions[k].pre.length(); j++)
	if ((i != j) && !r.contains(instance.actions[k].pre[j])) {
	  if (implies(instance.actions[k].pre[j], instance.actions[k].pre[i]))
	    r.insert(instance.actions[k].pre[i]);
	}
    if (!r.empty()) {
      if (trace_level > 1) {
	std::cerr << "preprocessor: removing redundant preconditions ";
	instance.write_atom_set(std::cerr, r);
	std::cerr << " from action " << instance.actions[k].name
		  << std::endl;
      }
      instance.actions[k].pre.subtract(r);
    }
  }
  stats.stop();
}

void Preprocessor::eliminate_strictly_determined_atoms()
{
  stats.start();
  rule_set sd;
  instance.compute_iff_axioms(sd);
  if (sd.empty()) {
    if (trace_level > 0) {
      std::cerr << "preprocessor: axiom set is empty" << std::endl;
    }
    stats.stop();
    return;
  }
  index_type n = sd.length();
  if (trace_level > 0) {
    std::cerr << "preprocessor: eliminating via axiom set ";
    instance.write_iff_axiom_set(std::cerr, sd);
    std::cerr << std::endl;
  }
  index_graph dg;
  sd.compute_dependency_graph(instance.n_atoms(), dg);
  sd.make_acyclic(dg);
  if ((n > sd.length()) && (trace_level > 0)) {
    std::cerr << "preprocessor: acyclic axiom set ";
    instance.write_iff_axiom_set(std::cerr, sd);
    std::cerr << std::endl;
    n = sd.length();
  }
  sd.make_post_unique(dg);
  if ((n > sd.length()) && (trace_level > 0)) {
    std::cerr << "preprocessor: post-unique axiom set ";
    instance.write_iff_axiom_set(std::cerr, sd);
    std::cerr << std::endl;
  }

  if (trace_level > 0) {
    instance.write_axiom_dependency_graph(std::cerr, dg, "Dependency Graph");
  }

  // d: true for atoms remaining to eliminate
  bool_vec d(true, instance.n_atoms());
  // e: set of eliminated atoms (to be removed at end)
  bool_vec e(false, instance.n_atoms());
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (dg.out_degree(k) == 0) d[k] = false;
  assert(d.count(true) == sd.length());
  bool done = false;
  while (!done) {
    // p: an eliminable atom that is a root in the dependency graph
    index_type p = no_such_index;
    for (index_type k = 0; (k < instance.n_atoms()) && p == no_such_index; k++)
      if (d[k] && (dg.in_degree(k) == 0))
	p = k;
    if (p != no_such_index) {
      index_type r = sd.find_rule(p);
      assert(r < sd.length());
      instance.replace_atom_by_conjunction(p, sd[r].antecedent);
      d[p] = false;
      e[p] = true;
      dg.remove_edges_with_label(r);
      assert(dg.out_degree(p) == 0);
    }
    else {
      done = true;
    }
  }
  assert(e.count(true) == sd.length());
  if (trace_level > 0) {
    std::cerr << "preprocessor: " << e.count(true) << " atoms eliminated (";
    instance.write_atom_set(std::cerr, e);
    std::cerr << ")" << std::endl;
  }

  if (e.count(true) > 0) {
    instance.remove_atoms(e, atom_map);
    remove_useless_actions();
    remove_useless_invariants();
    instance.clear_cross_reference();
  }
  stats.stop();
}

void Preprocessor::necessary_completions_for_safeness
(index_set& atoms_missing_negation)
{
  inconsistency();
  atoms_missing_negation.clear();
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (consistent(instance.actions[k].pre)) {
      for (index_type i = 0; i < instance.actions[k].add.length(); i++) {
	if (!instance.actions[k].pre.contains(instance.actions[k].add[i])) {
	  if (consistent(instance.actions[k].pre, instance.actions[k].add[i]) &&
	      !implies(instance.actions[k].pre, instance.actions[k].add[i])) {
	    if (instance.atoms[instance.actions[k].add[i]].neg == no_such_index)
	      atoms_missing_negation.insert(instance.actions[k].add[i]);
	  }
	}
      }
      for (index_type i = 0; i < instance.actions[k].del.length(); i++) {
	if (!instance.actions[k].pre.contains(instance.actions[k].del[i])) {
	  if (consistent(instance.actions[k].pre, instance.actions[k].del[i]) &&
	      !implies(instance.actions[k].pre, instance.actions[k].del[i])) {
	    if (instance.atoms[instance.actions[k].del[i]].neg == no_such_index)
	      atoms_missing_negation.insert(instance.actions[k].del[i]);
	  }
	}
      }
    }
}

void Preprocessor::make_safe()
{
  inconsistency();
  Instance::action_vec acts(instance.actions);
  instance.actions.clear();

  for (index_type k = 0; k < acts.length(); k++) {
    if (consistent(acts[k].pre)) {
      index_set i_pre;
      index_set r_add;
      index_set r_del;
      index_set v_add;
      index_set v_del;
      for (index_type i = 0; i < acts[k].add.length(); i++) {
	// assert(instance.atoms[acts[k].add[i]].neg != no_such_index);
	if (!acts[k].pre.contains(acts[k].add[i])) {
	  if (inconsistent(acts[k].pre, acts[k].add[i])) {
	    if (instance.atoms[acts[k].add[i]].neg != no_such_index)
	      i_pre.insert(instance.atoms[acts[k].add[i]].neg);
	  }
	  else if (implies(acts[k].pre, acts[k].add[i])) {
	    r_add.insert(acts[k].add[i]);
	  }
	  else {
	    v_add.insert(acts[k].add[i]);
	  }
	}
	else {
	  r_add.insert(acts[k].add[i]);
	}
      }
      for (index_type i = 0; i < acts[k].del.length(); i++) {
	// assert(instance.atoms[acts[k].add[i]].neg != no_such_index);
	if (!acts[k].pre.contains(acts[k].del[i])) {
	  if (inconsistent(acts[k].pre, acts[k].del[i])) {
	    r_del.insert(acts[k].del[i]);
	  }
	  else if (implies(acts[k].pre, acts[k].del[i])) {
	    i_pre.insert(acts[k].del[i]);
	  }
	  else {
	    v_del.insert(acts[k].del[i]);
	  }
	}
      }
      if (trace_level > 1) {
	std::cerr << "completing action ";
	instance.print_action(std::cerr, acts[k]);
	std::cerr << "implied preconditions: ";
	instance.write_atom_set(std::cerr, i_pre);
	std::cerr << std::endl << "redundant added atoms: ";
	instance.write_atom_set(std::cerr, r_add);
	std::cerr << std::endl << "redundant deleted atoms: ";
	instance.write_atom_set(std::cerr, r_del);
	std::cerr << std::endl << "variable added atoms: ";
	instance.write_atom_set(std::cerr, v_add);
	std::cerr << std::endl << "variable deleted atoms: ";
	instance.write_atom_set(std::cerr, v_del);
	std::cerr << std::endl;
      }
      index_type n = 0;
      SubsetEnumerator e_add(v_add.length());
      bool more = e_add.first();
      while (more) {
	SubsetEnumerator e_del(v_del.length());
	bool more1 = e_del.first();
	while (more1) {
	  index_set c_pre(acts[k].pre);
	  c_pre.insert(i_pre);
	  index_set c_add(acts[k].add);
	  c_add.subtract(r_add);
	  index_set c_del(acts[k].del);
	  c_del.subtract(r_del);
	  for (index_type i = 0; i < v_add.length(); i++)
	    if (e_add.current_set()[i]) {
	      c_pre.insert(v_add[i]);
	      c_add.subtract(v_add[i]);
	    }
	    else {
	      index_type pos_pre = v_add[i];
	      index_type neg_pre = instance.atoms[pos_pre].neg;
	      assert(neg_pre != no_such_index);
	      c_pre.insert(neg_pre);
	    }
	  for (index_type i = 0; i < v_del.length(); i++)
	    if (e_del.current_set()[i]) {
	      c_pre.insert(v_del[i]);
	    }
	    else {
	      index_type pos_pre = v_del[i];
	      index_type neg_pre = instance.atoms[pos_pre].neg;
	      assert(neg_pre != no_such_index);
	      c_pre.insert(neg_pre);
	      c_del.subtract(v_del[i]);
	    }
	  if (trace_level > 1) {
	    std::cerr << "variable added atoms TRUE:";
	    for (index_type i = 0; i < v_add.length(); i++)
	      if (e_add.current_set()[i])
		std::cerr << " " << instance.atoms[v_add[i]].name;
	    std::cerr << std::endl << "variable added atoms FALSE:";
	    for (index_type i = 0; i < v_add.length(); i++)
	      if (!e_add.current_set()[i])
		std::cerr << " " << instance.atoms[v_add[i]].name;
	    std::cerr << std::endl << "variable deleted atoms TRUE:";
	    for (index_type i = 0; i < v_del.length(); i++)
	      if (e_del.current_set()[i])
		std::cerr << " " << instance.atoms[v_del[i]].name;
	    std::cerr << std::endl << "variable deleted atoms FALSE:";
	    for (index_type i = 0; i < v_del.length(); i++)
	      if (!e_del.current_set()[i])
		std::cerr << " " << instance.atoms[v_del[i]].name;
	    std::cerr << std::endl;
	  }
	  if (consistent(c_pre) && !c_add.empty()) {
	    if (trace_level > 1) {
	      std::cerr << "completed precondition ";
	      instance.write_atom_set(std::cerr, c_pre);
	      std::cerr << " is consistent and add set is non-empty:"
			<< " creating action " << n
			<< std::endl;
	    }
	    Instance::Action& c_act =
	      instance.new_action(new NameAtIndex(acts[k].name, n++));
	    c_act.sel = acts[k].sel;
	    c_act.pre = c_pre;
	    c_act.add = c_add;
	    c_act.del = c_del;
	    c_act.lck = acts[k].lck;
	    c_act.use = acts[k].use;
	    c_act.cons = acts[k].cons;
	    c_act.dur = acts[k].dur;
	    c_act.dmin = acts[k].dmin;
	    c_act.dmax = acts[k].dmax;
	    c_act.cost = acts[k].cost;
	    c_act.assoc = acts[k].assoc;
	    c_act.src = acts[k].src;
	  }
	  else if (trace_level > 1) {
	    if (!consistent(c_pre)) {
	      std::cerr << "completed precondition ";
	      instance.write_atom_set(std::cerr, c_pre);
	      std::cerr << " is NOT consistent" << std::endl;
	    }
	    if (c_add.empty()) {
	      std::cerr << "add set of completion is EMPTY" << std::endl;
	    }
	  }
	  more1 = e_del.next();
	}
	more = e_add.next();
      }
      if ((trace_level > 0) && (n == 0)) {
	std::cerr << "warning: ZERO completions of action "
		  << acts[k].name << " created"
		  << std::endl;
      }
    }
    else if (trace_level > 0) {
      std::cerr << "warning: original action "
		<< acts[k].name
		<< " has inconsistent preconditions ";
      instance.write_atom_set(std::cerr, acts[k].pre);
      std::cerr << std::endl;
    }
  }
  instance.clear_cross_reference();
}

void Preprocessor::apply_place_replication()
{
  index_type n_orig = instance.n_atoms();
  for (index_type k = 0; k < n_orig; k++) {
    index_set ca;
    for (index_type i = 0; i < instance.atoms[k].req_by.length(); i++)
      if (!instance.atoms[k].del_by.contains(instance.atoms[k].req_by[i]))
	ca.insert(instance.atoms[k].req_by[i]);
    if (!ca.empty()) {
      graph cag(ca.length());
      for (index_type i = 0; i < ca.length(); i++)
	for (index_type j = i + 1; j < ca.length(); j++)
	  if (instance.non_interfering(ca[i], ca[j]) &&
	      consistent(instance.actions[ca[i]].pre,
			 instance.actions[ca[j]].pre))
	    cag.add_undirected_edge(i, j);
      index_set_vec groups(EMPTYSET, 1);
      index_set rem;
      rem.fill(ca.length());
      cag.apx_independent_set(rem, groups[0]);
      rem.subtract(groups[0]);
      while (!rem.empty()) {
	index_set& g = groups.append();
	cag.apx_independent_set(rem, g);
	rem.subtract(g);
      }
      for (index_type i = 0; i < groups.length(); i++)
	groups[i].remap(ca);
      std::cerr << "action groups conflicting on "
		<< instance.atoms[k].name << ":";
      for (index_type i = 0; i < groups.length(); i++) {
	std::cerr << " ";
	instance.write_action_set(std::cerr, groups[i]);
      }
      std::cerr << std::endl;
      index_set new_pk;
      for (index_type i = 1; i < groups.length(); i++) {
	Instance::Atom& pki =
	  instance.new_atom(new CopyName(instance.atoms[k].name, i));
	pki.neg = instance.atoms[k].neg;
	pki.init = instance.atoms[k].init;
	pki.init_t = instance.atoms[k].init_t;
	pki.goal = instance.atoms[k].goal;
	pki.goal_t = instance.atoms[k].goal_t;
	pki.irrelevant = instance.atoms[k].irrelevant;
	pki.src = instance.atoms[k].src;
	new_pk.insert(pki.index);
	for (index_type j = 0; j < groups[i].length(); j++) {
	  instance.actions[groups[i][j]].pre.subtract(k);
	  instance.actions[groups[i][j]].pre.insert(pki.index);
	}
      }
      if (!new_pk.empty()) {
	for (index_type i = 0; i < instance.atoms[k].add_by.length(); i++)
	  instance.actions[instance.atoms[k].add_by[i]].add.insert(new_pk);
	for (index_type i = 0; i < instance.atoms[k].del_by.length(); i++) {
	  instance.actions[instance.atoms[k].del_by[i]].del.insert(new_pk);
	  if (instance.actions[instance.atoms[k].del_by[i]].pre.contains(k))
	    instance.actions[instance.atoms[k].del_by[i]].pre.insert(new_pk);
	}
      }
    }
  }
  if (instance.n_atoms() > n_orig)
    instance.clear_cross_reference();
}

bool Preprocessor::make_invariant
(index_type init_atom, index_set& inv_set, bool& exact, index_set& branch_set)
{
  if (trace_level > 2) {
    std::cerr << "checking ";
    instance.write_atom_set(std::cerr, inv_set);
    std::cerr << " with init atom " << instance.atoms[init_atom].name
	      << " for invariance..." << std::endl;
  }

  branch_set.clear();

  // remove atoms added by 0-consistent actions and 1-consistent actions
  // without deleted precondition

  bool converged = false;
  while (!converged && !inv_set.empty()) {
    converged = true;
    bool changed = false;
    for (index_type k = 0; k < instance.n_actions(); k++) {
      Instance::Action& act = instance.actions[k];
      index_set p(act.pre);
      p.intersect(inv_set);
      if (p.empty()) {
	if (act.add.count_common(inv_set) > 0) {
	  if (trace_level > 2) {
	    std::cerr << "action " << act.name << " is 0-consistent and violates invariant: removing ";
	    instance.write_atom_set(std::cerr, act.add);
	    std::cerr << std::endl;
	  }
	  for (index_type i = 0; i < act.add.length(); i++)
	    if ((act.add[i] != init_atom) && inv_set.contains(act.add[i])) {
	      inv_set.subtract(act.add[i]);
	      changed = true;
	    }
	  converged = false;
	}
	if (exact) {
	  if (act.del.count_common(inv_set) > 0) exact = false;
	}
      }
      else if ((p.length() == 1) && !act.del.contains(p[0])) {
	if (act.add.count_common(inv_set) > 0) {
	  if (trace_level > 2) {
	    std::cerr << "action " << act.name << " is 1-consistent and violates invariant: removing ";
	    instance.write_atom_set(std::cerr, act.add);
	    std::cerr << std::endl;
	  }
	  for (index_type i = 0; i < act.add.length(); i++)
	    if ((act.add[i] != init_atom) && inv_set.contains(act.add[i])) {
	      inv_set.subtract(act.add[i]);
	      changed = true;
	    }
	  converged = false;
	}
	if (exact) {
	  if (act.del.count_common(inv_set) > 0) exact = false;
	}
      }
    }
    if (trace_level > 2) {
      std::cerr << "set at end of current iteration: ";
      instance.write_atom_set(std::cerr, inv_set);
      std::cerr << std::endl;
    }
    if (!converged && !changed) {
      if (trace_level > 2) {
	std::cerr << "set can not converge (need to remove protected atom)"
		  << std::endl;
      }
      return false;
    }
  }

  // if zero or one atoms remain in the set, it is useless
  if (inv_set.length() <= 1) return false;

  bool verified = true;
  for (index_type k = 0; k < instance.n_actions(); k++) {
    Instance::Action& act = instance.actions[k];
    index_set p(act.pre);
    p.intersect(inv_set);
    if ((p.length() == 1) && act.del.contains(p[0])) {
      if (act.add.count_common(inv_set) > 1) {
	if (trace_level > 2) {
	  std::cerr << "action " << act.name << " is 1-consistent with delete and violates invariant: adding ";
	  instance.write_atom_set(std::cerr, act.add);
	  std::cerr << " to branch set" << std::endl;
	}
	branch_set.insert(act.add);
	branch_set.intersect(inv_set);
	verified = false;
      }
      if (exact) {
	if (act.del.count_common(inv_set) > 1) exact = false;
      }
    }
  }
  branch_set.subtract(init_atom);

  return verified;
}

void Preprocessor::analyze_branch_set
(const index_set& branch_set, const index_set& inv_set, index_set& effective)
{
  effective.clear();
  for (index_type k = 0; k < branch_set.length(); k++) {
    index_type b_atom = branch_set[k];
    if (trace_level > 2) {
      std::cerr << "checking effects of branching on "
		<< instance.atoms[b_atom].name
		<< "..." << std::endl;
    }
    const index_set& a_acts = instance.atoms[branch_set[k]].req_by;
    for (index_type i = 0; i < a_acts.length(); i++) {
      Instance::Action& act = instance.actions[a_acts[i]];
      index_type n_pre = act.pre.count_common(inv_set);
      if (n_pre == 1) {
	index_type n_add = act.add.count_common(inv_set);
	if (n_add > 0) {
	  if (trace_level > 2) {
	    std::cerr << "effect found: action " << act.name
		      << " becomes 0-consistent and violating"
		      << std::endl;
	  }
	  effective.insert(b_atom);
	}
      }
      else if (n_pre == 2) {
	index_type n_del = act.del.count_common(inv_set);
	if ((n_del == 0) || ((n_del == 1) && act.del.contains(b_atom))) {
	  index_type n_add = act.add.count_common(inv_set);
	  if (n_add > 0) {
	    if (trace_level > 2) {
	      std::cerr << "effect found: action " << act.name
			<< " becomes 1-consistent w/o delete and violating"
			<< std::endl;
	    }
	    effective.insert(b_atom);
	  }
	}
      }
    }
  }
}

void Preprocessor::dfs_find_invariants
(index_type init_atom, const index_set& base_set, const index_set& branch_set,
 index_type branch_depth, index_type max_branch_depth)
{
  // std::cerr << "(" << branch_depth << ")";
  index_set rem_branch_set;
  analyze_branch_set(branch_set, base_set, rem_branch_set);
  index_set ne_branch_set(branch_set);
  ne_branch_set.subtract(rem_branch_set);

  index_set inv;
  index_set b;

  while (!rem_branch_set.empty() && !stats.break_signal_raised()) {
    inv.assign_copy(base_set);
    inv.subtract(rem_branch_set[0]);
    if (trace_level > 2) {
      std::cerr << "(" << branch_depth << ") checking set ";
      instance.write_atom_set(std::cerr, inv);
      std::cerr << "..." << std::endl;
    }
    bool exact = false; // dummy
    bool ok = make_invariant(init_atom, inv, exact, b);
    if (ok) {
      if (trace_level > 1) {
	std::cerr << "(" << branch_depth << ") invariant found: ";
	instance.write_atom_set(std::cerr, inv);
	std::cerr << std::endl;
      }
      Instance::Constraint& c = instance.new_invariant(inv, 1, false);
    }
    else if (!b.empty() && (branch_depth < max_branch_depth)) {
      if (trace_level > 2) {
	std::cerr << "(" << branch_depth << ") found potential invariants ";
	instance.write_atom_set(std::cerr, inv);
	std::cerr << std::endl << "(" << branch_depth << ") branching on ";
	instance.write_atom_set(std::cerr, b);
	std::cerr << "..." << std::endl;
      }
      dfs_find_invariants(init_atom, inv, b, branch_depth + 1,
			  max_branch_depth);
    }
    rem_branch_set.remove(0);
  }

  if (!ne_branch_set.empty()) {
    inv.assign_copy(base_set);
    inv.subtract(ne_branch_set);
    if (trace_level > 2) {
      std::cerr << "(" << branch_depth << ") checking set ";
      instance.write_atom_set(std::cerr, inv);
      std::cerr << "..." << std::endl;
    }
    bool exact = false; // dummy
    bool ok = make_invariant(init_atom, inv, exact, b);
    if (ok) {
      if (trace_level > 1) {
	std::cerr << "(" << branch_depth << ") invariant found: ";
	instance.write_atom_set(std::cerr, inv);
	std::cerr << std::endl;
      }
      Instance::Constraint& c = instance.new_invariant(inv, 1, false);
    }
    else if (!b.empty() && (branch_depth < max_branch_depth)) {
      if (trace_level > 2) {
	std::cerr << "(" << branch_depth << ") found potential invariants ";
	instance.write_atom_set(std::cerr, inv);
	std::cerr << std::endl << "(" << branch_depth << ") branching on ";
	instance.write_atom_set(std::cerr, b);
	std::cerr << "..." << std::endl;
      }
      dfs_find_invariants(init_atom, inv, b, branch_depth + 1,
			  max_branch_depth);
    }
  }
}

void Preprocessor::dfs_find_invariants(index_type max_branch_depth)
{
  stats.start();
  if (trace_level > 0) {
    std::cerr << "searching for invariants (DFS)..." << std::endl;
  }

  index_set non_init_atoms;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (!instance.atoms[k].init) non_init_atoms.insert(k);

  for (index_type k = 0; (k < instance.init_atoms.length()) &&
	 !stats.break_signal_raised(); k++) {
    index_set inv(non_init_atoms);
    index_set b;
    inv.insert(instance.init_atoms[k]);
    if (trace_level > 2) {
      std::cerr << "(0) checking set ";
      instance.write_atom_set(std::cerr, inv);
      std::cerr << "..." << std::endl;
    }
    bool exact = true;
    bool ok = make_invariant(instance.init_atoms[k], inv, exact, b);
    if (ok) {
      if (trace_level > 1) {
	std::cerr << "(0) invariant found: ";
	instance.write_atom_set(std::cerr, inv);
	if (exact) std::cerr << " (exactly-1)";
	else std::cerr << " (at-most-1)";
	std::cerr << std::endl;
      }
      Instance::Constraint& c = instance.new_invariant(inv, 1, exact);
    }
    else if (!b.empty() && (max_branch_depth > 0)) {
      if (trace_level > 2) {
	std::cerr << "(0) found potential invariants ";
	instance.write_atom_set(std::cerr, inv);
	std::cerr << std::endl << "(0) branching on ";
	instance.write_atom_set(std::cerr, b);
	std::cerr << "..." << std::endl;
      }
      dfs_find_invariants(instance.init_atoms[k], inv, b, 1, max_branch_depth);
    }
  }
  stats.stop();
}

struct InvariantSearchState {
  index_type init_atom;
  index_set  set;

  InvariantSearchState(index_type i, const index_set& s)
    : init_atom(i), set(s) { };

  bool equals(const InvariantSearchState& s);
};

bool InvariantSearchState::equals(const InvariantSearchState& s)
{
  if (init_atom != s.init_atom) return false;
  if (!(set == s.set)) return false;
  return true;
}

void Preprocessor::bfs_find_invariants()
{
  stats.start();
  if (trace_level > 0) {
    std::cerr << "searching for invariants (BFS)..." << std::endl;
  }

  std::list<InvariantSearchState> cand;

  index_set non_init_atoms;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (!instance.atoms[k].init) non_init_atoms.insert(k);

  for (index_type k = 0; k < instance.init_atoms.length(); k++) {
    index_set inv(non_init_atoms);
    inv.insert(instance.init_atoms[k]);
    InvariantSearchState s(instance.init_atoms[k], inv);
    cand.push_back(s);
  }

  std::list<InvariantSearchState>::iterator next = cand.begin();
  index_type proc = 0;

  while ((next != cand.end()) && !stats.break_signal_raised()) {
    index_set inv(next->set);
    bool exact = false;
    index_set b;
    bool ok = make_invariant(next->init_atom, inv, exact, b);
    if (ok) {
      if (trace_level > 1) {
	std::cerr << "invariant found: ";
	instance.write_atom_set(std::cerr, inv);
	std::cerr << std::endl;
      }
      Instance::Constraint& c = instance.new_invariant(inv, 1, exact);
    }
    else if (!b.empty()) {
      index_set e;
      analyze_branch_set(b, inv, e);
      index_set ne(b);
      ne.subtract(e);
      if (trace_level > 2) {
	std::cerr << "found potential invariants ";
	instance.write_atom_set(std::cerr, inv);
	std::cerr << " branching on ";
	instance.write_atom_set(std::cerr, e);
	std::cerr << " (effective) and ";
	instance.write_atom_set(std::cerr, ne);
	std::cerr << "..." << std::endl;
      }
      for (index_type k = 0; k < e.length(); k++) {
	index_set new_inv(inv);
	new_inv.subtract(e[k]);
	InvariantSearchState s(next->init_atom, new_inv);
	bool skip = false;
	for (std::list<InvariantSearchState>::iterator p = cand.begin();
	     (p != cand.end()) && !skip; p++)
	  if (p->equals(s)) skip = true;
	if (!skip) cand.push_back(s);
      }
      inv.subtract(ne);
      InvariantSearchState s0(next->init_atom, inv);
      bool skip = false;
      for (std::list<InvariantSearchState>::iterator p = cand.begin();
	   (p != cand.end()) && !skip; p++)
	if (p->equals(s0)) skip = true;
      if (!skip) cand.push_back(s0);
    }
    next++;
    proc++;
    if (trace_level > 2) {
      std::cerr << "|candidates| = " << cand.size() - proc
		<< " (" << cand.size() << ")" << std::endl;
    }
  }
  stats.stop();
}

bool Preprocessor::verify_invariant
(Instance::Constraint& inv, Heuristic& inc)
{
  if (trace_level > 1) {
    std::cerr << "verifying invariant " << inv.index << ": |";
    instance.write_atom_set(std::cerr, inv.set);
    std::cerr << "| " << (inv.exact ? "==" : "<=") << " " << inv.lim
	      << "..." << std::endl;
  }

  bool v_exact = true;

  index_type init_n = instance.init_atoms.count_common(inv.set);
  if (init_n > inv.lim) {
    if (trace_level > 1) {
      std::cerr << " - violated by initial state" << std::endl;
    }
    return false;
  }
  else if (init_n < inv.lim) {
    if (trace_level > 1) {
      std::cerr << " - not exact in initial state";
      if (inv.exact) std::cerr << ": weakening";
      std::cerr << std::endl;
    }
    v_exact = false;
  }

  for (index_type k = 0; (k < instance.n_actions()) &&
	 !stats.break_signal_raised(); k++) {
    index_type n_pre = instance.actions[k].pre.count_common(inv.set);
    if (n_pre <= inv.lim) {
      index_type n_true = 0;
      for (index_type i = 0; i < inv.set.length(); i++) {
	if (FINITE(inc.incremental_eval(instance.actions[k].pre, inv.set[i])))
	  n_true += 1;
      }
      index_type n_safe = (n_true < inv.lim ? inv.lim - n_true : 0);
      index_type n_add = instance.actions[k].add.count_common(inv.set);
      index_type n_del = instance.actions[k].del.count_common(inv.set);
      if (trace_level > 2) {
	std::cerr << " - action " << instance.actions[k].name
		  << ": n_true = " << n_true
		  << ", n_safe = " << n_safe
		  << ", n_add = " << n_add
		  << ", n_del = " << n_del
		  << std::endl;
      }
      if (n_add > (n_del + n_safe)) {
	if (trace_level > 1) {
	  std::cerr << " - violated by action "
		    << instance.actions[k].name << std::endl;
	}
	return false;
      }
      if (v_exact && (n_add < n_del)) {
	if (trace_level > 1) {
	  std::cerr << " - not exact by action "
		    << instance.actions[k].name;
	  if (inv.exact) std::cerr << ": weakening";
	  std::cerr << std::endl;
	}
	v_exact = false;
      }
    }
  }

  if (v_exact) {
    if (trace_level > 1) {
      std::cerr << " - is exact";
      if (!inv.exact) std::cerr << ": strengthening";
      std::cerr << std::endl;
    }
    inv.exact = true;
  }
  else if (inv.exact && !v_exact) {
    inv.exact = false;
  }

  return true;
}

bool Preprocessor::verify_invariants(Heuristic& inc)
{
  stats.start();
  if (trace_level > 0) {
    std::cerr << "verifying invariants (using inconsistency relation)..."
	      << std::endl;
  }
  for (index_type k = 0; (k < instance.n_invariants()) &&
	 !stats.break_signal_raised(); k++)
    if (!instance.invariants[k].verified)
      instance.invariants[k].verified =
	verify_invariant(instance.invariants[k], inc);
  stats.stop();
}

void Preprocessor::remove_unverified_invariants()
{
  bool_vec inv_to_remove(false, instance.n_invariants());
  for (index_type k = 0; k < instance.n_invariants(); k++)
    if (!instance.invariants[k].verified) inv_to_remove[k] = true;
  instance.remove_invariants(inv_to_remove, inv_map);
}

bool Preprocessor::find_inverse_actions
(const Instance::Action& act, Heuristic& inc, index_set& invs)
{
  index_set q(act.pre);
  q.subtract(act.del);
  q.insert(act.add);
  invs.clear();
  for (index_type k = act.index + 1; k < instance.n_actions(); k++) {
    Instance::Action& b(instance.actions[k]);
    bool ok = (q.contains(b.pre) && (b.add == act.del));
    for (index_type i = 0; (i < b.del.length()) && ok; i++)
      if (FINITE(inc.incremental_eval(act.pre, b.del[i]))) ok = false;
    if (ok) invs.insert(k);
  }
  return !invs.empty();
}

void Preprocessor::find_binary_iff_invariants(bool opt_weak_verify)
{
  stats.start();
  for (index_type i = 0; i < instance.n_atoms(); i++)
    for (index_type j = i + 1; j < instance.n_atoms(); j++)
      if (inconsistent(i, j)) {
	Instance::Constraint h;
	h.set.insert(i);
	h.set.insert(j);
	h.lim = 1;
	h.exact = true;
	if (opt_weak_verify) {
	  h.verified = instance.verify_invariant(h);
	}
	else {
	  h.verified = verify_invariant(h, *inconsistency());
	}
	if (h.verified && h.exact) {
	  instance.new_invariant(h.set, h.lim, true);
	}
      }
  stats.stop();
}

void Preprocessor::find_inconsistent_set_invariants(graph& inc)
{
  if (trace_level > 0) {
    std::cerr << "searching for cliques (approximate)..." << std::endl;
  }
  stats.start();
  graph iig(inc);
  iig.complement();

  index_type n = instance.n_invariants();
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    index_set inv;
    iig.apx_independent_set_including(k, inv);
    if (trace_level > 1) {
      instance.write_atom_set(std::cerr << "|", inv);
      std::cerr << "| <= 1" << std::endl;
    }
    if (inv.length() > 1) {
      instance.new_invariant(inv, 1, false);
    }
  }

  stats.stop();
  if (trace_level > 0) {
    std::cerr << instance.n_invariants() - n << " invariants found in "
	      << stats.time() << " seconds" << std::endl;
  }
}

void Preprocessor::find_maximal_inconsistent_set_invariants(graph& inc)
{
  if (trace_level > 0) {
    std::cerr << "searching for maximal cliques..." << std::endl;
  }
  stats.start();

  index_type n = instance.n_invariants();
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    index_set_vec inv;
    inc.all_maximal_cliques_including(k, inv);
    for (index_type i = 0; i < inv.length(); i++) {
      if (trace_level > 1) {
	instance.write_atom_set(std::cerr << "|", inv[i]);
	std::cerr << "| <= 1" << std::endl;
      }
      if (inv[i].length() > 1) {
	instance.new_invariant(inv[i], 1, false);
      }
    }
  }

  stats.stop();
  if (trace_level > 0) {
    std::cerr << instance.n_invariants() - n << " invariants found in "
	      << stats.time() << " seconds" << std::endl;
  }
}

bool Preprocessor::inconsistent(index_type p, index_type q, Heuristic& inc)
{
  index_set s;
  s.insert(p);
  s.insert(q);
  return INFINITE(inc.eval(s));
}

bool Preprocessor::consistent(index_type p, index_type q, Heuristic& inc)
{
  index_set s;
  s.insert(p);
  s.insert(q);
  return FINITE(inc.eval(s));
}

bool Preprocessor::inconsistent
(const index_set& s, index_type p, Heuristic& inc)
{
  return INFINITE(inc.incremental_eval(s, p));
}

bool Preprocessor::consistent
(const index_set& s, index_type p, Heuristic& inc)
{
  return FINITE(inc.incremental_eval(s, p));
}

bool Preprocessor::inconsistent(index_type p, index_type q)
{
  inconsistency();
  return INFINITE(inc_relation->eval(p, q));
}

bool Preprocessor::consistent(index_type p, index_type q)
{
  inconsistency();
  return FINITE(inc_relation->eval(p, q));
}

bool Preprocessor::inconsistent(const index_set& s, index_type p)
{
  inconsistency();
  return INFINITE(inc_relation->incremental_eval(s, p));
}

bool Preprocessor::consistent(const index_set& s, index_type p)
{
  inconsistency();
  return FINITE(inc_relation->incremental_eval(s, p));
}

bool Preprocessor::inconsistent(const index_set& s)
{
  inconsistency();
  return INFINITE(inc_relation->eval(s));
}

bool Preprocessor::consistent(const index_set& s)
{
  inconsistency();
  return FINITE(inc_relation->eval(s));
}

bool Preprocessor::inconsistent(const index_set& s0, const index_set& s1)
{
  index_set s(s0);
  s.insert(s1);
  inconsistency();
  return INFINITE(inc_relation->eval(s));
}

bool Preprocessor::consistent(const index_set& s0, const index_set& s1)
{
  index_set s(s0);
  s.insert(s1);
  inconsistency();
  return FINITE(inc_relation->eval(s));
}

bool Preprocessor::implies(index_type p, index_type q, Heuristic& inc)
{
  for (index_type k = 0; k < instance.n_invariants(); k++)
    if ((instance.invariants[k].lim == 1) &&
	instance.invariants[k].exact &&
	instance.invariants[k].set.contains(q)) {
      bool f = true;
      for (index_type i = 0; (i < instance.invariants[k].set.length()) && f; i++)
	if (instance.invariants[k].set[i] != q)
	  if (consistent(p, instance.invariants[k].set[i], inc))
	    f = false;
      if (f) return true;
    }
  return false;
}

bool Preprocessor::implies(const index_set& s, index_type q, Heuristic& inc)
{
  if (trace_level > 2) {
    std::cerr << "checking if ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " implies " << instance.atoms[q].name
	      << "..." << std::endl;
  }
  if (instance.atoms[q].neg != no_such_index) {
    if (INFINITE(inc.incremental_eval(s, instance.atoms[q].neg)))
      return true;
  }
  for (index_type k = 0; k < instance.n_invariants(); k++)
    if ((instance.invariants[k].lim == 1) &&
	instance.invariants[k].exact &&
	instance.invariants[k].set.contains(q)) {
      if (trace_level > 2) {
	std::cerr << "testing against invariant ";
	instance.print_invariant(std::cerr, instance.invariants[k]);
      }
      bool f = true;
      for (index_type i = 0; (i < instance.invariants[k].set.length()) && f; i++)
	if (instance.invariants[k].set[i] != q)
	  if (FINITE(inc.incremental_eval(s, instance.invariants[k].set[i]))) {
	    if (trace_level > 2) {
	      std::cerr << " - failed: set consistent with "
			<< instance.atoms[instance.invariants[k].set[i]].name
			<< std::endl;
	    }
	    f = false;
	  }
      if (f) {
	if (trace_level > 2) {
	  std::cerr << " - ok" << std::endl;
	}
	f = false;
	return true;
      }
    }
  return false;
}

bool Preprocessor::implies(index_type p, index_type q)
{
  return implies(p, q, *inconsistency());
}

bool Preprocessor::implies(const index_set& s, index_type q)
{
  return implies(s, q, *inconsistency());
}

void Preprocessor::implied_atom_set
(const index_set& s, index_set& is, Heuristic& inc)
{
  stats.start();
  is.clear();
  index_set sx(s);
  bool done = false;
  while (!done) {
    if (trace_level > 1) {
      std::cerr << "begin iteration..." << std::endl;
    }
    done = true;
    for (index_type k = 0; k < instance.n_atoms(); k++)
      if (!sx.contains(k))
	if (implies(sx, k, inc)) {
	  if (trace_level > 1) {
	    std::cerr << instance.atoms[k].name
		      << " implied by ";
	    instance.write_atom_set(std::cerr, sx);
	    std::cerr << std::endl;
	  }
	  sx.insert(k);
	  is.insert(k);
	  done = false;
	}
    if (trace_level > 1) {
      std::cerr << "end iteration, done = " << done << std::endl;
    }
  }
  stats.stop();
}

bool Preprocessor::landmark_test
(const index_set& init, const index_set& goal, index_type p, bool opt_H2)
{
  // if the atom is a goal, it is a landmark by definition (?)
  if (goal.contains(p)) return true;

  stats.start();
  Instance* test_instance = new Instance(instance);
  test_instance->set_initial(init);
  test_instance->set_goal(goal);
  test_instance->atoms[p].init = false;
  for (index_type k = 0; k < test_instance->n_actions(); k++)
    test_instance->actions[k].add.subtract(p);
  test_instance->cross_reference();
  CostTable* test_reachability = new CostTable(*test_instance, stats);
  if (opt_H2)
    test_reachability->compute_H2(ZeroACF());
  else
    test_reachability->compute_H1(ZeroACF());
  NTYPE val = test_reachability->eval(goal);
  delete test_reachability;
  delete test_instance;
  stats.stop();
  return INFINITE(val);
}

void Preprocessor::compute_landmarks
(bool exclude_goal, bool exclude_init, bool_vec& landmark_atoms)
{
  stats.start();
  landmark_atoms.assign_value(false, instance.n_atoms());
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    if (instance.atoms[k].goal) {
      if (!exclude_goal) landmark_atoms[k] = true;
    }
    else if (instance.atoms[k].init) {
      if (!exclude_init)
	if (landmark_test(instance.init_atoms, instance.goal_atoms, k))
	  landmark_atoms[k] = true;
    }
    else {
      if (landmark_test(instance.init_atoms, instance.goal_atoms, k))
	landmark_atoms[k] = true;
    }
  }
  stats.stop();
}

void Preprocessor::compute_landmarks
(bool exclude_goal, bool exclude_init, index_set& landmark_atoms)
{
  bool_vec lm;
  compute_landmarks(exclude_goal, exclude_init, lm);
  lm.copy_to(landmark_atoms);
}

void Preprocessor::compute_landmarks
(const index_set& init, const index_set& goal, index_set& landmark_atoms)
{
  stats.start();
  landmark_atoms.clear();
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    if (landmark_test(init, goal, k)) landmark_atoms.insert(k);
  }
  stats.stop();
}

void Preprocessor::compute_landmark_graph(graph& g, bool opt_H2)
{
  stats.start();
  g.init(instance.n_atoms());
  for (index_type i = 0; i < instance.n_atoms(); i++) {
    index_set goal_i;
    goal_i.assign_singleton(i);
    for (index_type j = 0; j < instance.n_atoms(); j++) if (j != i) {
      if (landmark_test(instance.init_atoms, goal_i, j, opt_H2))
	g.add_edge(j, i);
    }
  }
  stats.stop();
}

void Preprocessor::quick_landmark_graph(graph& g, bool opt_H2)
{
  stats.start();
  g.init(instance.n_atoms());

  for (index_type i = 0; i < instance.n_atoms(); i++) {
    Instance* test_instance = new Instance(instance);
    test_instance->atoms[i].init = false;
    for (index_type k = 0; k < test_instance->n_actions(); k++)
      test_instance->actions[k].add.subtract(i);
    test_instance->cross_reference();
    CostTable* test_reachability = new CostTable(*test_instance, stats);
    if (opt_H2)
      test_reachability->compute_H2(ZeroACF());
    else
      test_reachability->compute_H1(ZeroACF());
    for (index_type j = 0; j < instance.n_atoms(); j++) if (i != j) {
      NTYPE val = test_reachability->eval(j);
      if (INFINITE(val)) g.add_edge(i, j);
    }
    delete test_reachability;
    delete test_instance;
  }

  stats.stop();
}

void Preprocessor::compute_hierarchy(graph& g0, index_set_graph& g)
{
  g0.init(instance.n_atoms());
  for (index_type k = 0; k < instance.n_actions(); k++) {
    for (index_type i = 0; i < instance.actions[k].add.length(); i++) {
      for (index_type j = i+1; j < instance.actions[k].add.length(); j++)
        g0.add_undirected_edge(instance.actions[k].add[i],
			       instance.actions[k].add[j]);
      for (index_type j = 0; j < instance.actions[k].del.length(); j++)
        g0.add_undirected_edge(instance.actions[k].add[i],
			       instance.actions[k].del[j]);
      for (index_type j = 0; j < instance.actions[k].pre.length(); j++)
        g0.add_edge(instance.actions[k].add[i],
		    instance.actions[k].pre[j]);
    }
    for (index_type i = 0; i < instance.actions[k].del.length(); i++) {
      for (index_type j = i+1; j < instance.actions[k].del.length(); j++)
        g0.add_undirected_edge(instance.actions[k].del[i],
			       instance.actions[k].del[j]);
      for (index_type j = 0; j < instance.actions[k].pre.length(); j++)
        g0.add_edge(instance.actions[k].del[i],
		    instance.actions[k].pre[j]);
    }
  }
  g0.remove_loops();
  g0.strongly_connected_components();
  g0.component_tree(g);
  for (index_type k = 0; k < g0.n_components(); k++)
    g0.component_node_set(k, g.node_label(k));
}

void Preprocessor::compute_hierarchy(index_set_graph& g)
{
  graph g0(instance.n_atoms());
  compute_hierarchy(g0, g);
}

void Preprocessor::write_heuristic_graph
(std::ostream& s, Heuristic& h, const ACF& cost,
 const bool_vec& atoms, const bool_vec& actions)
{
  NTYPE c_max = 0;
  for (index_type i = 0; i < instance.n_atoms(); i++)
    if (atoms[i])
      if (FINITE(h.eval(i)))
	c_max = MAX(c_max, h.eval(i));
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (actions[k])
      if (FINITE(h.eval(instance.actions[k].pre)))
	c_max = MAX(c_max, h.eval(instance.actions[k].pre));

  s << "digraph HG  {" << std::endl;
  s << "rankdir=LR;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;
  NTYPE l = 0;
  while (l <= c_max) {
    s << "{ rank = same; /* atom layer " << l << "*/" << std::endl;
    s << "LP" << l << "[shape=plaintext,label=\"P" << l << "\"];"
	   << std::endl;
    for (index_type k = 0; k < instance.n_atoms(); k++) {
      if (atoms[k] && (h.eval(k) == l)) {
	s << "P" << k << "[shape=ellipse,label=\""
	  << instance.atoms[k].name << "\"";
	if (instance.atoms[k].goal) {
	  s << ",style=bold";
	}
	s << "];" << std::endl;
      }
    }
    s << "}" << std::endl;
    s << "{ rank = same; /* action layer " << l << "*/" << std::endl;
    s << "LA" << l << "[shape=plaintext,label=\"A" << l << "\"];"
      << std::endl;
    for (index_type k = 0; k < instance.n_actions(); k++) {
      if (actions[k] && (h.eval(instance.actions[k].pre) == l)) {
	s << "A" << k << "[shape=box,label=\""
	  << instance.actions[k].name << "\"];" << std::endl;
      }
    }
    s << "}" << std::endl;
    l = l + 1;
  }
  l = 0;
  while (l <= c_max) {
    s << "LP" << l << " -> LA" << l << " [style=invis];" << std::endl;
    if (l > 0) {
      s << "LA" << l-1 << " -> LP" << l << " [style=invis];" << std::endl;
    }
    l = l + 1;
  }
  for (index_type k = 0; k < instance.n_actions(); k++) if (actions[k]) {
    Instance::Action& act = instance.actions[k];
    NTYPE c_pre = h.eval(act.pre);
    for (index_type i = 0; i < act.pre.length(); i++) if (atoms[act.pre[i]]) {
      s << "P" << act.pre[i] << " -> A" << k;
      if (c_pre == h.eval(act.pre[i])) {
	s << " [style=bold]";
      }
      s << ";" << std::endl;
    }
    for (index_type i = 0; i < act.add.length(); i++) if (atoms[act.add[i]]) {
      s << "A" << k << " -> P" << act.add[i];
      if ((c_pre + cost(act.index)) <= h.eval(act.add[i])) {
	s << " [style=bold]";
      }
      else {
	s << " [constraint=false]";
      }
      s << ";" << std::endl;
    }
  }
  s << "}" << std::endl;
}

void Preprocessor::write_heuristic_graph
(std::ostream& s, Heuristic& h, const ACF& cost)
{
  bool_vec atoms(true, instance.n_atoms());
  bool_vec actions(true, instance.n_actions());
  write_heuristic_graph(s, h, cost, atoms, actions);
}

void Preprocessor::write_relevance_graph
(std::ostream& s,
 const index_vec& d_atm,
 const index_vec& d_act,
 const bool_vec& mark_atm,
 const bool_vec& mark_act,
 const char* label)
{
  index_type m = 0;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if ((d_atm[k] != no_such_index) && (d_atm[k] > m)) m = d_atm[k];
  for (index_type k = 0; k < instance.n_actions(); k++)
    if ((d_act[k] != no_such_index) && (d_act[k] > m)) m = d_act[k];

  s << "digraph RG  {" << std::endl;
  if (label) {
    s << "label=\"" << label << "\";" << std::endl; 
  }
  s << "rankdir=TB;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;

  for (index_type d = 0; d <= m; d++) {
    s << "{ rank = same; /* " << d << " */" << std::endl;
    for (index_type k = 0; k < instance.n_atoms(); k++)
      if (d_atm[k] == d) {
	s << " ATM" << k << " [shape=ellipse,";
	if (mark_atm[k]) s << "style=bold,";
	s << "label=\"" << instance.atoms[k].name << "\"];" << std::endl;
      }
    s << "}" << std::endl << "{ rank = same; /* " << d << " */" << std::endl;
    for (index_type k = 0; k < instance.n_actions(); k++)
      if (d_act[k] == d) {
	s << " ACT" << k << " [shape=box,";
	if (mark_act[k]) s << "style=bold,";
	s << "label=\"" << instance.actions[k].name << "\"];" << std::endl;
      }
    s << "}" << std::endl;
  }

  for (index_type k = 0; k < instance.n_actions(); k++)
    if (d_act[k] != no_such_index) {
      for (index_type i = 0; i < instance.actions[k].add.length(); i++)
	if (d_atm[instance.actions[k].add[i]] != no_such_index) {
	  s << " ATM" << instance.actions[k].add[i] << " -> ACT" << k;
	  if (mark_atm[instance.actions[k].add[i]] && mark_act[k]) {
	    if (d_atm[instance.actions[k].add[i]] > d_act[k])
	      s << " [style=bold,constraint=false]";
	    else
	      s << " [style=bold]";
	  }
	  else if (d_atm[instance.actions[k].add[i]] > d_act[k])
	    s << " [constraint=false]";
	  s << ";" << std::endl;
	}
      for (index_type i = 0; i < instance.actions[k].pre.length(); i++) {
	assert(d_atm[instance.actions[k].pre[i]] != no_such_index);
	s << " ACT" << k << " -> ATM" << instance.actions[k].pre[i];
	if (mark_atm[instance.actions[k].pre[i]] && mark_act[k]) {
	  if (d_atm[instance.actions[k].pre[i]] <= d_act[k])
	    s << " [style=bold,constraint=false]";
	  else
	    s << " [style=bold]";
	}
	else if (d_atm[instance.actions[k].pre[i]] <= d_act[k])
	  s << " [constraint=false]";
	s << ";" << std::endl;
      }
    }

  s << "}" << std::endl;
}

void Preprocessor::write_relevance_graph
(std::ostream& s, const index_vec& d_atm, const index_vec& d_act,
 const char* label)
{
  bool_vec mark_atm(false, instance.n_atoms());
  bool_vec mark_act(false, instance.n_actions());
  write_relevance_graph(s, d_atm, d_act, mark_atm, mark_act, label);
}

END_HSPS_NAMESPACE
