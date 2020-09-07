
#include "problem.h"
#include "heuristic.h"

BEGIN_HSPS_NAMESPACE

bool Instance::write_negation = true;
bool Instance::write_DKEL = true;
bool Instance::write_PDDL2 = true;
bool Instance::write_time = true;
bool Instance::write_PDDL3 = true;
bool Instance::write_metric = true;
bool Instance::write_extra = true;
bool Instance::write_resource_constraints_at_start = false;
bool Instance::always_write_parameters = false;
bool Instance::always_write_requirements = false;
bool Instance::always_write_precondition = false;
bool Instance::always_write_effect = false;
bool Instance::always_write_conjunction = false;

bool Instance::write_atom_set_with_symbolic_names = true;
bool Instance::write_action_set_with_symbolic_names = true;
int  Instance::default_trace_level = 0;

const char* Instance::goal_atom_name = "GoalReached";
const char* Instance::goal_action_name = "GoalAction";
NTYPE       Instance::goal_action_cost = 0;

const char* Instance::pc_name = "constraint";
index_type  Instance::pc_count = 0;

index_type rule_set::find_rule(index_type c) const
{
  for (index_type k = 0; k < length(); k++) {
    if ((*this)[k].consequent == c) return k;
    if ((*this)[k].consequent > c) return no_such_index;
  }
  return no_such_index;
}

void rule_set::compute_dependency_graph
(index_type n, index_graph& g) const
{
  g.init(n);
  for (index_type k = 0; k < length(); k++) {
    g.node_label((*this)[k].consequent) = (*this)[k].consequent;
    for (index_type i = 0; i < (*this)[k].antecedent.length(); i++) {
      g.add_edge((*this)[k].consequent, (*this)[k].antecedent[i]);
      g.edge_label((*this)[k].consequent, (*this)[k].antecedent[i]) = k;
      g.node_label((*this)[k].antecedent[i]) = (*this)[k].antecedent[i];
    }
  }
}

void rule_set::remove(const bool_vec& set, index_vec& map)
{
  index_type scan_p = 0;
  index_type put_p = 0;
  map.assign_value(no_such_index, length());
  while (scan_p < length()) {
    if (!set[scan_p]) {
      if (put_p < scan_p) {
	(*this)[put_p] = (*this)[scan_p];
      }
      map[scan_p] = put_p;
      put_p += 1;
    }
    scan_p += 1;
  }
  set_length(put_p);
}

void rule_set::remove(const bool_vec& set, index_graph& g)
{
  index_vec map;
  remove(set, map);
  for (index_type i = 0; i < g.size(); i++)
    for (index_type j = 0; j < g.size(); j++)
      if (g.adjacent(i, j)) {
	if (map[g.edge_label(i, j)] == no_such_index) {
	  g.remove_edge(i, j);
	  g.edge_label(i, j) = no_such_index;
	}
	else {
	  g.edge_label(i, j) = map[g.edge_label(i, j)];
	}
      }
}

void rule_set::remove(const bool_vec& set)
{
  index_vec map;
  remove(set, map);
}

void rule_set::make_acyclic(index_graph& dg)
{
  if (empty()) return;
  bool_vec keep(true, length());
  dg.strongly_connected_components();
  index_type c = dg.maximal_non_unit_component();
  while (c != no_such_index) {
    index_set rc;
    for (index_type k = 0; k < length(); k++) {
      const rule& r = (*this)[k];
      if (dg.component(r.consequent) == c) {
	bool f = false;
	for (index_type i = 0; (i < r.antecedent.length()) && !f; i++) {
	  if (dg.component(r.antecedent[i]) == c) {
	    f = true;
	    rc.insert(k);
	  }
	}
      }
    }
    assert(!rc.empty());
//     ::std::cerr << "axioms involved in cycle: ";
//     for (index_type k = 0; k < rc.length(); k++) {
//       if (k > 0) ::std::cerr << ", ";
//       instance.write_iff_axiom(::std::cerr, sd[rc[k]]);
//     }
//     ::std::cerr << ::std::endl;
    index_type p = no_such_index;
    // if there's a rule in the cyclic set whose consequent is also the
    // consequent of a rule not in the cyclic set, prefer to remove that
    // rule
    for (index_type k = 0; (k < rc.length()) && (p == no_such_index); k++) {
      const rule& r = (*this)[rc[k]];
      for (index_type i = 0; (i < length()) && (p == no_such_index); i++)
	if (!rc.contains(i) && ((*this)[i].consequent == r.consequent))
	  p = rc[k];
    }
    // if there are two rules in the cyclic set with the same consequent,
    // prefer to remove one of them (the one with the smaller antecedent)
    for (index_type k = 0; (k < rc.length()) && (p == no_such_index); k++) {
      const rule& r = (*this)[rc[k]];
      for (index_type i = 0; (i < rc.length()) && (p == no_such_index); i++)
	if ((*this)[rc[i]].consequent == r.consequent) {
	  if ((*this)[rc[i]].antecedent.length() < r.antecedent.length())
	    p = rc[i];
	  else
	    p = rc[k];
	}
    }
    // if we prefer no rule, just pick one with a maximal-size antecedent
    if (p == no_such_index) {
      p = rc[0];
      for (index_type k = 1; k < rc.length(); k++) {
	const rule& r = (*this)[rc[k]];
	if (r.antecedent.length() > (*this)[p].antecedent.length())
	  p = rc[k];
      }
    }

    assert((p < length()) && keep[p]);
    dg.remove_edges_with_label(p);
    keep[p] = false;
    dg.strongly_connected_components();
    c = dg.maximal_non_unit_component();
  }

  if (keep.count(true) < length()) {
    keep.complement();
    remove(keep, dg);
  }
}

void rule_set::make_post_unique(index_graph& dg)
{
  if (empty()) return;
  bool_vec keep(true, length());
  for (index_type k = 0; k < length(); k++) if (keep[k]) {
    const rule& r = (*this)[k];
    index_set rsc;
    for (index_type i = k + 1; i < length(); i++)
      if ((*this)[i].consequent == r.consequent)
	rsc.insert(i);
    if (!rsc.empty()) {
      rsc.insert(k);
      index_type p = rsc[0];
      index_type s = dg.count_reachable((*this)[p].consequent);
      for (index_type i = 1; i < rsc.length(); i++) {
	index_type si = dg.count_reachable((*this)[rsc[i]].consequent);
	if (si < s) {
	  p = rsc[i];
	  s = si;
	}
      }
      assert(p < rsc.length());
      for (index_type i = 0; i < rsc.length(); i++)
	if (rsc[i] != p) keep[rsc[i]] = false;
    }
  }

  if (keep.count(true) < length()) {
    keep.complement();
    remove(keep, dg);
  }
}

bool Instance::Action::e_deletes(index_type p, Heuristic* inc) const
{
  if (del.contains(p)) return true;
  if (inc) {
    if (INFINITE(inc->incremental_eval(pre, p))) return true;
  }
  return false;
}

bool Instance::Action::e_deletes(const index_set& s, Heuristic* inc) const
{
  if (add.first_common_element(s) != no_such_index)
    return false;
  for (index_type k = 0; k < s.length(); k++)
    if (!e_deletes(s[k], inc)) return false;
  return true;
}

Instance::Instance()
  : xrf(false),
    name(0),
    atoms(Atom(), 0),
    actions(Action(), 0),
    resources(Resource(), 0),
    invariants(Constraint(), 0),
    init_atoms(),
    goal_atoms(),
    max_pre(0), max_add(0), max_del(0), max_lck(0),
    max_add_by(0), max_del_by(0), max_req_by(0),
    min_dur(POS_INF), max_dur(NEG_INF),
    min_cost(POS_INF), max_cost(NEG_INF),
    atom_set_hash(0), action_set_hash(0),
    trace_level(default_trace_level)
{
  // done
}

Instance::Instance(const Name* n)
  : xrf(false),
    name(n),
    atoms(Atom(), 0),
    actions(Action(), 0),
    resources(Resource(), 0),
    invariants(Constraint(), 0),
    init_atoms(),
    goal_atoms(),
    max_pre(0), max_add(0), max_del(0), max_lck(0),
    max_add_by(0), max_del_by(0), max_req_by(0),
    min_dur(POS_INF), max_dur(NEG_INF),
    min_cost(POS_INF), max_cost(NEG_INF),
    atom_set_hash(0), action_set_hash(0),
    trace_level(default_trace_level)
{
  // done
}

Instance::Instance(const Instance& ins)
  : xrf(false),
    name(ins.name),
    atoms(ins.atoms),
    actions(ins.actions),
    resources(ins.resources),
    invariants(ins.invariants),
    init_atoms(ins.init_atoms),
    goal_atoms(ins.goal_atoms),
    max_pre(0), max_add(0), max_del(0), max_lck(0),
    max_add_by(0), max_del_by(0), max_req_by(0),
    min_dur(POS_INF), max_dur(NEG_INF),
    min_cost(POS_INF), max_cost(NEG_INF),
    atom_set_hash(ins.n_atoms()),
    action_set_hash(ins.n_atoms()),
    trace_level(ins.trace_level)
{
  // done - note that copy is not cross referenced
}

Instance::Atom& Instance::new_atom(const Name* name)
{
  Atom& p = atoms.append();
  p.index = atoms.length() - 1;
  p.name = name;
  p.neg = no_such_index;
  p.init = false;
  p.init_t = 0;
  p.goal = false;
  p.goal_t = POS_INF;
  p.irrelevant = false;
  p.req_by.clear();
  p.add_by.clear();
  p.del_by.clear();
  if (trace_level > 2) {
    std::cerr << "atom " << p.index << "." << p.name
	      << " created" << ::std::endl;
  }
  return p;
}

Instance::Resource& Instance::new_resource(const Name* name)
{
  Resource& r = resources.append();
  r.index = resources.length() - 1;
  r.name = name;
  r.init = 0;
  if (trace_level > 2) {
    std::cerr << "resouce " << r.index << "." << r.name
	      << " created" << ::std::endl;
  }
  return r;
}

Instance::Action& Instance::new_action(const Name* name)
{
  Action& a = actions.append();
  a.index = actions.length() - 1;
  a.name = name;
  a.pre.clear();
  a.add.clear();
  a.del.clear();
  a.lck.clear();
  a.use.assign_value(0);
  a.cons.assign_value(0);
  a.dur = 1;
  a.cost = 1;
  a.ncw_atms.clear();
  if (trace_level > 2) {
    ::std::cerr << "action " << a.index << "." << a.name
	      << " created" << ::std::endl;
  }
  return a;
}

Instance::Action& Instance::copy_action(index_type a1)
{
  Action& a = actions.append();
  a.index = actions.length() - 1;
  a.name = actions[a1].name;
  a.pre = actions[a1].pre;
  a.add = actions[a1].add;
  a.del = actions[a1].del;
  a.lck = actions[a1].lck;
  a.use = actions[a1].use;
  a.cons = actions[a1].cons;
  a.dur = actions[a1].dur;
  a.cost = actions[a1].cost;
  a.ncw_atms.clear();
  if (trace_level > 2) {
    ::std::cerr << "action " << a.index << "." << a.name
		<< " created as copy of " << a1
		<< ::std::endl;
  }
  return a;
}

Instance::Constraint& Instance::new_invariant()
{
  Constraint& c = invariants.append();
  c.index = invariants.length() - 1;
  c.name = 0;
  c.set.clear();
  c.lim = 0;
  c.exact = false;
  c.verified = false;
  if (trace_level > 2) {
    std::cerr << "invariant " << invariants.length() - 1
	      << " created" << ::std::endl;
  }
  return c;
}

Instance::Constraint& Instance::new_invariant(const Name* name)
{
  Constraint& c = invariants.append();
  c.index = invariants.length() - 1;
  c.name = name;
  c.set.clear();
  c.lim = 0;
  c.exact = false;
  c.verified = false;
  if (trace_level > 2) {
    std::cerr << "invariant " << name << " (" << invariants.length() - 1
	      << ") created" << ::std::endl;
  }
  return c;
}

Instance::Constraint& Instance::new_invariant
(const index_set& s, index_type l, bool e)
{
  for (index_type k = 0; k < invariants.length(); k++) {
    if ((invariants[k].lim == l) && (invariants[k].exact == e)) {
      if (invariants[k].set.contains(s)) {
	return invariants[k];
      }
      else if (s.contains(invariants[k].set)) {
	invariants[k].set = s;
	return invariants[k];
      }
    }
  }
  Constraint& c = new_invariant();
  c.set = s;
  c.lim = l;
  c.exact = e;
  c.verified = false;
  return c;
}

void Instance::atom_names(name_vec& names) const
{
  names.clear();
  for (index_type k = 0; k < n_atoms(); k++)
    names.append(atoms[k].name);
}

void Instance::action_names(name_vec& names) const
{
  names.clear();
  for (index_type k = 0; k < n_actions(); k++)
    names.append(actions[k].name);
}

void Instance::assign_unique_action_names()
{
  bool_vec cov(false, n_actions());
  for (index_type k = 0; k < n_actions(); k++) if (!cov[k]) {
    index_set s;
    for (index_type i = k; i < n_actions(); i++)
      if (actions[i].name->equals(actions[k].name)) {
	s.insert(i);
	cov[i] = true;
      }
    if (s.length() > 0) {
      const Name* b = actions[k].name;
      for (index_type i = 0; i < s.length(); i++)
	actions[s[i]].name = new CopyName(b, i);
    }
  }
}

void Instance::cochange_graph(graph& g) const
{
  g.init(n_atoms());
  for (index_type k = 0; k < n_actions(); k++) {
    for (index_type i = 0; i < actions[k].add.length(); i++)
      for (index_type j = i + 1; j < actions[k].add.length(); j++)
	g.add_undirected_edge(actions[k].add[i], actions[k].add[j]);
    for (index_type i = 0; i < actions[k].del.length(); i++)
      for (index_type j = i + 1; j < actions[k].del.length(); j++)
	g.add_undirected_edge(actions[k].del[i], actions[k].del[j]);
    for (index_type i = 0; i < actions[k].add.length(); i++)
      for (index_type j = 0; j < actions[k].del.length(); j++)
	g.add_undirected_edge(actions[k].add[i], actions[k].del[j]);
  }
}

void Instance::causal_graph(graph& g) const
{
  cochange_graph(g);
  for (index_type k = 0; k < n_actions(); k++) {
    for (index_type i = 0; i < actions[k].add.length(); i++)
      for (index_type j = 0; j < actions[k].pre.length(); j++)
	g.add_edge(actions[k].add[i], actions[k].pre[j]);
  }
}

// void Instance::partitioning_graph
// (const index_set& goal, index_set_graph& g) const
// {
//   assert(goal.length() > 0);
//   equivalence unsep(goal.length());
//   for (index_type i = 0; i < goal.length(); i++)
//     for (index_type j = i + 1; j < goal.length(); j++)
//       if (cochanged(goal[i], goal[j]))
// 	unsep.merge(i, j);
// 
//   index_set_vec p;
//   p.set_length(unsep.n_classes());
//   // unsep.classes(p);
// 
//   index_set c;
//   unsep.canonical_elements(c);
//   for (index_type k = 0; k < c.length(); k++) {
//     index_set e;
//     unsep.class_elements(c[k], e);
//     p[k].assign_mapped_copy(e, goal);
//   }
// 
//   std::cerr << "goals: ";
//   write_atom_set(std::cerr, goal);
//   std::cerr << std::endl << "initial sets:" << std::endl;
//   for (index_type k = 0; k < p.length(); k++) {
//     std::cerr << k << ": ";
//     write_atom_set(std::cerr, p[k]);
//     std::cerr << std::endl;
//   }
// 
//   graph cg;
//   cochange_graph(cg);
//   index_set_vec f;
// 
//   index_type d = 0;
//   bool done = false;
//   while (!done) {
//     done = true;
//     d += 1;
//     for (index_type k = 0; k < p.length(); k++)
//       cg.bi_fringe(p[k], f[k]);
//     for (index_type k = 0; k < p.length(); k++) {
//       index_set p_new;
//       for (index_type i = 0; i < f[k].length(); i++) {
// 	bool cfl = false;
// 	for (index_type j = 0; (j < p.length()) && !cfl; j++)
// 	  if (j != k)
// 	    if (f[j].contains(f[k][i]) || p[j].contains(f[k][i]))
// 	      cfl = true;
// 	if (!cfl)
// 	  p_new.insert(f[k][i]);
//       }
// 
//       std::cerr << "at distance " << d << ":" << std::endl;
//       std::cerr << "set " << k << " fringe: ";
//       write_atom_set(std::cerr, f[k]);
//       std::cerr << std::endl << "set " << k << " new: ";
//       write_atom_set(std::cerr, p_new);
//       std::cerr << std::endl;
// 
//       if (!p_new.empty()) {
// 	p[k].insert(p_new);
// 	done = false;
//       }
//     }    
//   }
// 
//   g.init(p.length());
//   for (index_type k = 0; k < p.length(); k++) {
//     g.node_label(k) = p[k];
//     for (index_type j = k + 1; j < p.length(); j++) {
//       index_set c(p[k]);
//       c.intersect(p[j]);
//       if (!c.empty()) {
// 	g.add_undirected_edge(k, j);
// 	g.edge_label(k, j) = c;
// 	g.edge_label(j, k) = c;
//       }
//     }
//   }
// }

void Instance::partitioning_graph
(const index_set& goal, index_set_graph& g, index_set& n_goal) const
{
  graph ccg;
  cochange_graph(ccg);

  equivalence eq(n_atoms());
  for (index_type i = 0; i < goal.length(); i++) {
    for (index_type j = i + 1; j < goal.length(); j++)
      if (ccg.adjacent(goal[i], goal[j]))
	eq.merge(goal[i], goal[j]);
    for (index_type k = 0; k < n_invariants(); k++)
      if ((invariants[k].lim == 1) &&
	  invariants[k].set.contains(goal[i]) &&
	  (invariants[k].exact)) {
	eq.merge(invariants[k].set);
      }
  }

  index_set_graph g_tmp(ccg, eq);
  g.copy(g_tmp);
  index_vec m;
  eq.make_map(m);
  n_goal.assign_remap(goal, m);

  bool done = false;
  index_type d = 1;
  while (!done) {
    done = true;
    // std::cerr << "iteration " << d << ":" << std::endl;
    // g.write_digraph(std::cerr, "current partitioning graph");
    // std::cerr << std::endl << "mapped goals = " << n_goal << std::endl;
    eq.reset(g.size());
    for (index_type k = 0; k < n_goal.length(); k++) {
      for (index_type i = 0; i < g.bidirectional(n_goal[k]).length(); i++) {
	bool in_cfl = false;
	for (index_type j = 0; j < n_goal.length(); j++)
	  if ((j != k) &&
	      (g.bi_adjacent(g.bidirectional(n_goal[k])[i], n_goal[j]) ||
	       g.bi_adjacent(g.bidirectional(n_goal[k])[i],
			     g.bidirectional(n_goal[j]))))
	    in_cfl = true;
	if (!in_cfl) {
	  // std::cerr << "merging " << g.bidirectional(n_goal[k])[i]
	  // << " into " << n_goal[k] << " = " << g.node_label(n_goal[k])
	  // << std::endl;
	  eq.merge(n_goal[k], g.bidirectional(n_goal[k])[i]);
	  done = false;
	}
      }
    }
    g.quotient(g_tmp, eq);
    g.copy(g_tmp);
    eq.make_map(m);
    n_goal.remap(m);
  }
}

void Instance::make_graph_representation(index_graph& g, name_vec& nn)
{
  g.init(n_atoms() + n_actions());
  nn.assign_value(0, n_atoms() + n_actions());
  index_type o = n_atoms();
  for (index_type k = 0; k < n_atoms(); k++) {
    g.node_label(k) = (index_graph::NS_ELLIPSE +
		       (atoms[k].init ? index_graph::NS_FILLED : 0) +
		       (atoms[k].goal ? index_graph::NS_DOUBLE : 0));
    nn[k] = atoms[k].name;
  }
  for (index_type k = 0; k < n_actions(); k++) {
    g.node_label(o + k) = index_graph::NS_BOX;
    nn[o + k] = actions[k].name;
    for (index_type i = 0; i < actions[k].add.length(); i++) {
      g.add_edge(o + k, actions[k].add[i]);
      g.edge_label(o + k, actions[k].add[i]) =
	index_graph::ES_NORMAL + index_graph::ED_FORWARD;
    }
    for (index_type i = 0; i < actions[k].pre.length(); i++) {
      g.add_edge(actions[k].pre[i], o + k);
      if (actions[k].del.contains(actions[k].pre[i]))
	g.edge_label(actions[k].pre[i], o + k) =
	  index_graph::ES_NORMAL + index_graph::ED_FORWARD;
      else
	g.edge_label(actions[k].pre[i], o + k) =
	  index_graph::ES_DASHED + index_graph::ED_FORWARD;
    }
    for (index_type i = 0; i < actions[k].del.length(); i++)
      if (!actions[k].pre.contains(actions[k].del[i])) {
	g.add_edge(o + k, actions[k].del[i]);
	g.edge_label(o + k, actions[k].del[i]) =
	  index_graph::ES_BOLD + index_graph::ED_NONE;
      }
  }
}

void Instance::copy(const Instance& ins)
{
  name = ins.name;
  atoms = ins.atoms;
  resources = ins.resources;
  actions = ins.actions;
  invariants = ins.invariants;
  xrf = false;
}

Instance* Instance::copy() const
{
  Instance* new_ins = new Instance();
  new_ins->copy(*this);
  return new_ins;
}

void Instance::clear()
{
  atoms.clear();
  actions.clear();
  resources.clear();
  invariants.clear();
  clear_cross_reference();
}

void Instance::restricted_copy
(const Instance& ins, const index_set& atms, const index_set& rc,
 index_set& acts, index_vec& map)
{
  clear();
  map.assign_value(no_such_index, ins.n_atoms());
  for (index_type k = 0; k < atms.length(); k++) {
    assert(atms[k] < ins.n_atoms());
    Atom& a = new_atom(ins.atoms[atms[k]].name);
    a.init = ins.atoms[atms[k]].init;
    a.goal = ins.atoms[atms[k]].goal;
    map[atms[k]] = a.index;
  }
  for (index_type k = 0; k < rc.length(); k++) {
    assert(rc[k] < ins.n_resources());
    Resource& r = new_resource(ins.resources[rc[k]].name);
    r.init = ins.resources[rc[k]].init;
  }
  acts.clear();
  for (index_type k = 0; k < ins.actions.length(); k++)
    if (ins.actions[k].add.count_common(atms) > 0) {
      Action& a = new_action(ins.actions[k].name);
      for (index_type i = 0; i < ins.actions[k].pre.length(); i++)
	if (map[ins.actions[k].pre[i]] != no_such_index)
	  a.pre.insert(map[ins.actions[k].pre[i]]);
      for (index_type i = 0; i < ins.actions[k].add.length(); i++)
	if (map[ins.actions[k].add[i]] != no_such_index)
	  a.add.insert(map[ins.actions[k].add[i]]);
      for (index_type i = 0; i < ins.actions[k].del.length(); i++)
	if (map[ins.actions[k].del[i]] != no_such_index)
	  a.del.insert(map[ins.actions[k].del[i]]);
      for (index_type i = 0; i < ins.actions[k].lck.length(); i++)
	if (map[ins.actions[k].lck[i]] != no_such_index)
	  a.lck.insert(map[ins.actions[k].lck[i]]);
      for (index_type i = 0; i < n_resources(); i++)
	a.use[i] = ins.actions[k].use[rc[i]];
      for (index_type i = 0; i < n_resources(); i++)
	a.cons[i] = ins.actions[k].cons[rc[i]];
      a.dur = ins.actions[k].dur;
      a.cost = ins.actions[k].cost;
      acts.insert(k);
    }
  for (index_type k = 0; k < ins.invariants.length(); k++)
    if (ins.invariants[k].set.count_common(atms) > ins.invariants[k].lim) {
      Constraint& c = new_invariant(ins.invariants[k].name);
      for (index_type i = 0; i < ins.invariants[k].set.length(); i++)
	if (map[ins.invariants[k].set[i]] != no_such_index)
	  c.set.insert(map[ins.invariants[k].set[i]]);
      c.lim = ins.invariants[k].lim;
      c.exact = false;
      c.verified = false;
    }
}

void Instance::restricted_copy
(const Instance& ins, const index_set& acts, index_vec& map)
{
  clear();
  index_set atms;
  for (index_type k = 0; k < acts.length(); k++) {
    atms.insert(ins.actions[acts[k]].pre);
    atms.insert(ins.actions[acts[k]].add);
    atms.insert(ins.actions[acts[k]].del);
    atms.insert(ins.actions[acts[k]].lck);
  }
  map.assign_value(no_such_index, ins.n_atoms());
  for (index_type k = 0; k < atms.length(); k++) {
    Atom& a = new_atom(ins.atoms[atms[k]].name);
    a.init = ins.atoms[atms[k]].init;
    a.goal = ins.atoms[atms[k]].goal;
    map[atms[k]] = a.index;
  }
  for (index_type k = 0; k < acts.length(); k++) {
    Action& a = new_action(ins.actions[acts[k]].name);
    for (index_type i = 0; i < ins.actions[acts[k]].pre.length(); i++)
      a.pre.insert(map[ins.actions[acts[k]].pre[i]]);
    for (index_type i = 0; i < ins.actions[acts[k]].add.length(); i++)
      a.add.insert(map[ins.actions[acts[k]].add[i]]);
    for (index_type i = 0; i < ins.actions[acts[k]].del.length(); i++)
      a.del.insert(map[ins.actions[acts[k]].del[i]]);
    for (index_type i = 0; i < ins.actions[acts[k]].lck.length(); i++)
      a.lck.insert(map[ins.actions[acts[k]].lck[i]]);
    a.dur = ins.actions[acts[k]].dur;
    a.cost = ins.actions[acts[k]].cost;
  }
  for (index_type k = 0; k < ins.invariants.length(); k++)
    if (ins.invariants[k].set.count_common(atms) > ins.invariants[k].lim) {
      Constraint& c = new_invariant(ins.invariants[k].name);
      for (index_type i = 0; i < ins.invariants[k].set.length(); i++)
	if (map[ins.invariants[k].set[i]] != no_such_index)
	  c.set.insert(map[ins.invariants[k].set[i]]);
      c.lim = ins.invariants[k].lim;
      c.exact = false;
      c.verified = false;
    }
}

void Instance::abstracted_copy
(const Instance& ins, const index_set& atms, index_vec& atm_map,
 index_vec& act_map)
{
  clear();
  atm_map.assign_value(no_such_index, ins.n_atoms());
  for (index_type k = 0; k < atms.length(); k++) {
    assert(atms[k] < ins.n_atoms());
    Atom& a = new_atom(ins.atoms[atms[k]].name);
    a.init = ins.atoms[atms[k]].init;
    a.goal = ins.atoms[atms[k]].goal;
    atm_map[atms[k]] = a.index;
  }
  act_map.assign_value(no_such_index, ins.n_actions());
  for (index_type k = 0; k < ins.actions.length(); k++) {
    if (ins.actions[k].add.count_common(atms) > 0) {
      index_set pre;
      index_set add;
      index_set del;
      index_set lck;
      for (index_type i = 0; i < ins.actions[k].pre.length(); i++)
	if (atm_map[ins.actions[k].pre[i]] != no_such_index)
	  pre.insert(atm_map[ins.actions[k].pre[i]]);
      for (index_type i = 0; i < ins.actions[k].add.length(); i++)
	if (atm_map[ins.actions[k].add[i]] != no_such_index)
	  add.insert(atm_map[ins.actions[k].add[i]]);
      for (index_type i = 0; i < ins.actions[k].del.length(); i++)
	if (atm_map[ins.actions[k].del[i]] != no_such_index)
	  del.insert(atm_map[ins.actions[k].del[i]]);
      for (index_type i = 0; i < ins.actions[k].lck.length(); i++)
	if (atm_map[ins.actions[k].lck[i]] != no_such_index)
	  lck.insert(atm_map[ins.actions[k].lck[i]]);
      bool found = false;
      for (index_type i = 0; (i < n_actions()) && !found; i++) {
	if ((actions[i].pre == pre) && (actions[i].add == add) &&
	    (actions[i].del == del) && (actions[i].lck == lck)) {
	  actions[i].dur = MIN(actions[i].dur, ins.actions[k].dur);
	  actions[i].cost = MIN(actions[i].cost, ins.actions[k].cost);
	  act_map[k] = i;
	  found = true;
	}
      }
      if (!found) {
	Action& a = new_action(ins.actions[k].name);
	a.pre = pre;
	a.add = add;
	a.del = del;
	a.lck = lck;
	a.dur = ins.actions[k].dur;
	a.cost = ins.actions[k].cost;
	act_map[k] = a.index;
      }
    }
    else {
      act_map[k] = no_such_index;
    }
  }
  for (index_type k = 0; k < ins.invariants.length(); k++) {
    index_set s;
    for (index_type i = 0; i < ins.invariants[k].set.length(); i++)
      if (atm_map[ins.invariants[k].set[i]] != no_such_index)
	s.insert(atm_map[ins.invariants[k].set[i]]);
    if (s.length() > ins.invariants[k].lim) {
      Constraint& c = new_invariant(s, ins.invariants[k].lim, false);
      if (!c.name && ins.invariants[k].name)
	c.name = ins.invariants[k].name;
      c.verified = false;
    }
  }
}

void Instance::reverse_copy(const Instance& ins)
{
  clear();

  for (index_type k = 0; k < ins.n_atoms(); k++) {
    assert(ins.atoms[k].neg != no_such_index);
    Atom& a = new_atom(ins.atoms[k].name);
    if (ins.atoms[ins.atoms[k].neg].goal)
      a.init = false;
    else
      a.init = true;
    a.goal = ins.atoms[k].init;
  }

  for (index_type k = 0; k < ins.n_actions(); k++) {
    index_set per_pre(ins.actions[k].pre);
    per_pre.subtract(ins.actions[k].del);
    index_set del_pre(ins.actions[k].pre);
    del_pre.intersect(ins.actions[k].del);
    Action& a = new_action(ins.actions[k].name);
    a.pre = ins.actions[k].add;
    a.pre.insert(per_pre);
    ins.negation_atom_set(ins.actions[k].del, a.pre);
    a.add = del_pre;
    ins.negation_atom_set(ins.actions[k].pre, a.del);
    for (index_type i = 0; i < ins.actions[k].add.length(); i++)
      if (!ins.actions[k].pre.contains(ins.actions[k].add[i]) &&
	  !ins.actions[k].pre.contains(ins.atoms[ins.actions[k].add[i]].neg)) {
	a.add.insert(ins.actions[k].add[i]);
	a.add.insert(ins.atoms[ins.actions[k].add[i]].neg);
      }
    for (index_type i = 0; i < ins.actions[k].del.length(); i++)
      if (!ins.actions[k].pre.contains(ins.actions[k].del[i]) &&
	  !ins.actions[k].pre.contains(ins.atoms[ins.actions[k].del[i]].neg)) {
	a.add.insert(ins.actions[k].del[i]);
	a.add.insert(ins.atoms[ins.actions[k].del[i]].neg);
      }
    a.lck = ins.actions[k].lck;
    a.cost = ins.actions[k].cost;
    a.dmin = ins.actions[k].dmin;
    a.dmax = ins.actions[k].dmax;
    a.dur = ins.actions[k].dur;
    a.assoc = ins.actions[k].assoc;
    a.src = ins.actions[k].src;
  }
}

// void Instance::reversed_copy(const Instance& ins)
// {
//   clear();
//   // create true/false atoms for each atom in ins
//   for (index_type k = 0; k < ins.n_atoms(); k++) {
//     Atom& a_true = new_atom(new ModName(ins.atoms[k].name, "true"));
//     Atom& a_false = new_atom(new ModName(ins.atoms[k].name, "false"));
//     a_true.neg = a_false.index;
//     a_false.neg = a_true.index;
//     if (ins.atoms[k].goal) {
//       a_true.init = true;
//       a_false.init = false;
//     }
//     else {
//       a_true.init = true;
//       a_false.init = true;
//     }
//     if (ins.atoms[k].init) {
//       a_true.goal = true;
//       a_false.goal = false;
//     }
//     else {
//       a_true.goal = false;
//       a_false.goal = true;
//     }
//   }
// 
//   for (index_type k = 0; k < ins.n_actions(); k++) {
//     Action& a = new_action(ins.actions[k].name);
//     for (index_type i = 0; i < ins.actions[k].pre.length(); i++) {
//       index_type p = ins.actions[k].pre[i];
//       if (ins.actions[k].del.contains(p)) {
// 	a.pre.insert((2*p)+1); // (2*p)+1 == p_false
// 	a.add.insert(2*p); // 2*p == p_true
// 	a.del.insert((2*p)+1); // (2*p)+1 == p_false
//       }
//       else {
// 	a.pre.insert(2*p); // 2*p == p_true
// 	a.add.insert(2*p); // 2*p == p_true
// 	a.del.insert((2*p)+1); // (2*p)+1 == p_false
//       }
//     }
//     for (index_type i = 0; i < ins.actions[k].add.length(); i++) {
//       index_type p = ins.actions[k].add[i];
//       if (!ins.actions[k].pre.contains(p)) {
// 	a.pre.insert(2*p); // 2*p == p_true
// 	a.add.insert(2*p); // 2*p == p_true
// 	a.add.insert((2*p)+1); // (2*p)+1 == p_false
//       }
//     }
//     for (index_type i = 0; i < ins.actions[k].del.length(); i++) {
//       index_type p = ins.actions[k].del[i];
//       if (!ins.actions[k].pre.contains(p)) {
// 	a.pre.insert((2*p)+1); // (2*p)+1 == p_false
// 	a.add.insert(2*p); // 2*p == p_true
// 	a.add.insert((2*p)+1); // (2*p)+1 == p_false
//       }
//     }
//     a.cost = ins.actions[k].cost;
//     a.dur = ins.actions[k].dur;
//   }
// }

void Instance::delete_relax(const index_set& x_atms)
{
  for (index_type k = 0; k < n_actions(); k++)
    actions[k].del.intersect(x_atms);
}

void Instance::delete_relax_less(const index_set& x_atms)
{
  for (index_type k = 0; k < n_actions(); k++)
    if (actions[k].del.first_common_element(x_atms) == no_such_index)
      actions[k].del.clear();
}

bool Instance::non_interfering(index_type a0, index_type a1) const
{
  for (index_type k = 0; k < actions[a0].del.length(); k++) {
    if (actions[a1].pre.contains(actions[a0].del[k])) {
      if (trace_level > 3) {
	::std::cerr << actions[a0].name << " deletes "
		  << atoms[actions[a0].del[k]].name
		  << " precondition of "
		  << actions[a1].name << ::std::endl;
      }
      return false;
    }
    if (actions[a1].add.contains(actions[a0].del[k])) {
      if (trace_level > 3) {
	::std::cerr << actions[a0].name << " deletes "
		  << atoms[actions[a0].del[k]].name
		  << " added by "
		  << actions[a1].name << ::std::endl;
      }
      return false;
    }
  }
  for (index_type k = 0; k < actions[a1].del.length(); k++) {
    if (actions[a0].pre.contains(actions[a1].del[k])) {
      if (trace_level > 3) {
	::std::cerr << actions[a1].name << " deletes "
		  << atoms[actions[a1].del[k]].name
		  << " precondition of "
		  << actions[a0].name << ::std::endl;
      }
      return false;
    }
    if (actions[a0].add.contains(actions[a1].del[k])) {
      if (trace_level > 3) {
	::std::cerr << actions[a1].name << " deletes "
		  << atoms[actions[a1].del[k]].name
		  << " added by "
		  << actions[a0].name << ::std::endl;
      }
      return false;
    }
  }
  return true;
}

bool Instance::lock_compatible(index_type a0, index_type a1) const
{
  for (index_type k = 0; k < actions[a0].lck.length(); k++) {
    if (actions[a1].lck.contains(actions[a0].lck[k])) {
      if (trace_level > 3) {
	::std::cerr << actions[a0].name << " and "
		  << actions[a1].name << " both require "
		  << atoms[actions[a0].lck[k]].name << ::std::endl;
      }
      return false;
    }
    else if (actions[a1].del.contains(actions[a0].lck[k])) {
      if (trace_level > 3) {
	::std::cerr << actions[a1].name << " deletes "
		  << atoms[actions[a0].lck[k]].name << " locked by "
		  << actions[a0].name << ::std::endl;
      }
      return false;
    }
    else if (actions[a1].pre.contains(actions[a0].lck[k])) {
      if (trace_level > 3) {
	::std::cerr << actions[a0].name << " locks "
		  << atoms[actions[a0].lck[k]].name
		  << " persistent precondition of "
		  << actions[a1].name << ::std::endl;
      }
      return false;
    }
  }
  for (index_type k = 0; k < actions[a1].lck.length(); k++) {
    if (actions[a0].del.contains(actions[a1].lck[k])) {
      if (trace_level > 3) {
	::std::cerr << actions[a0].name << " deletes "
		  << atoms[actions[a1].lck[k]].name << " locked by "
		  << actions[a1].name << ::std::endl;
      }
      return false;
    }
    else if (actions[a0].pre.contains(actions[a1].lck[k])) {
      if (trace_level > 3) {
	::std::cerr << actions[a1].name << " locks "
		  << atoms[actions[a1].lck[k]].name
		  << " persistent precondition of "
		  << actions[a0].name << ::std::endl;
      }
      return false;
    }
  }
  return true;
}

bool Instance::resource_compatible(index_type a0, index_type a1) const
{
  for (index_type k = 0; k < n_resources(); k++) {
    NTYPE sum = actions[a0].use[k] + actions[a1].use[k];
    if (sum > resources[k].init) return false;
  }
  return true;
}

bool Instance::commutative
(const Instance::Action& a0, const Instance::Action& a1) const
{
  // a0 does not delete a precondition or positive effect of a1
  for (index_type k = 0; k < a0.del.length(); k++) {
    if (a1.pre.contains(a0.del[k])) {
      if (trace_level > 3) {
	::std::cerr << a0.name << " deletes "
		  << atoms[a0.del[k]].name
		  << " precondition of "
		  << a1.name << ::std::endl;
      }
      return false;
    }
    if (a1.add.contains(a0.del[k])) {
      if (trace_level > 3) {
	::std::cerr << a0.name << " deletes "
		  << atoms[a0.del[k]].name
		  << " added by "
		  << a1.name << ::std::endl;
      }
      return false;
    }
  }
  // a0 does not add a precondition of a1
  for (index_type k = 0; k < a0.add.length(); k++) {
    if (a1.pre.contains(a0.add[k])) {
      if (trace_level > 3) {
	::std::cerr << a0.name << " adds "
		  << atoms[a0.add[k]].name
		  << " precondition of "
		  << a1.name << ::std::endl;
      }
      return false;
    }
  }
  // a1 does not delete a precondition or positive effect of a0
  for (index_type k = 0; k < a1.del.length(); k++) {
    if (a0.pre.contains(a1.del[k])) {
      if (trace_level > 3) {
	::std::cerr << a1.name << " deletes "
		  << atoms[a1.del[k]].name
		  << " precondition of "
		  << a0.name << ::std::endl;
      }
      return false;
    }
    if (a0.add.contains(a1.del[k])) {
      if (trace_level > 3) {
	::std::cerr << a1.name << " deletes "
		  << atoms[a1.del[k]].name
		  << " added by "
		  << a0.name << ::std::endl;
      }
      return false;
    }
  }
  // a1 does not add a precondition of a0
  for (index_type k = 0; k < a1.add.length(); k++) {
    if (a0.pre.contains(a1.add[k])) {
      if (trace_level > 3) {
	::std::cerr << a1.name << " adds "
		  << atoms[a1.add[k]].name
		  << " precondition of "
		  << a0.name << ::std::endl;
      }
      return false;
    }
  }
  return true;
}

bool Instance::commutative(index_type a0, index_type a1) const
{
  return commutative(actions[a0], actions[a1]);
}

bool Instance::additive(index_type p0, index_type p1) const
{
  for (index_type k = 0; k < n_actions(); k++) {
    if (actions[k].add.contains(p0) &&
	actions[k].add.contains(p1))
      return false;
  }
  return true;
}

bool Instance::cochanged(index_type p0, index_type p1) const
{
  for (index_type k = 0; k < n_actions(); k++) {
    if (actions[k].add.contains(p0)) {
      if (actions[k].add.contains(p1))
	return true;
      if (actions[k].del.contains(p1))
	return true;
    }
    else if (actions[k].del.contains(p0)) {
      if (actions[k].add.contains(p1))
	return true;
      if (actions[k].del.contains(p1))
	return true;
    }
  }
  return false;
}

bool Instance::eval_invariant_in_partial_state
(const index_set& s, const Instance::Constraint& inv)
{
  index_type n = 0;
  for (index_type i = 0; (i < inv.set.length()) && (n <= inv.lim); i++)
    if (s.contains(inv.set[i])) n += 1;
  if (n > inv.lim) {
    if (trace_level > 2) {
      ::std::cerr << "state ";
      write_atom_set(::std::cerr, s);
      ::std::cerr << " violates invariant " << '|';
      write_atom_set(::std::cerr, inv.set);
      ::std::cerr << "| <= " << inv.lim << ::std::endl;
    }
    return false;
  }
  else {
    return true;
  }
}

bool Instance::eval_invariant_in_partial_state
(const bool_vec& s, const Instance::Constraint& inv)
{
  index_type n = 0;
  for (index_type i = 0; (i < inv.set.length()) && (n <= inv.lim); i++)
    if (s[inv.set[i]]) n += 1;
  if (n > inv.lim) {
    if (trace_level > 2) {
      ::std::cerr << "state ";
      write_atom_set(::std::cerr, s);
      ::std::cerr << " violates invariant " << '|';
      write_atom_set(::std::cerr, inv.set);
      ::std::cerr << "| <= " << inv.lim << ::std::endl;
    }
    return false;
  }
  else {
    return true;
  }
}

bool Instance::eval_invariant_in_complete_state
(const index_set& s, const Instance::Constraint& inv)
{
  index_type n = 0;
  for (index_type i = 0; (i < inv.set.length()) && (n <= inv.lim); i++)
    if (s.contains(inv.set[i])) n += 1;
  if (n > inv.lim) {
    if (trace_level > 2) {
      ::std::cerr << "state ";
      write_atom_set(::std::cerr, s);
      ::std::cerr << " violates invariant " << '|';
      write_atom_set(::std::cerr, inv.set);
      ::std::cerr << "| <= " << inv.lim << ::std::endl;
    }
    return false;
  }
  else if ((n < inv.lim) && inv.exact) {
    if (trace_level > 2) {
      ::std::cerr << "state ";
      write_atom_set(::std::cerr, s);
      ::std::cerr << " violates invariant " << '|';
      write_atom_set(::std::cerr, inv.set);
      ::std::cerr << "| == " << inv.lim << ::std::endl;
    }
    return false;
  }
  else {
    return true;
  }
}

bool Instance::eval_invariant_in_complete_state
(const bool_vec& s, const Instance::Constraint& inv)
{
  index_type n = 0;
  for (index_type i = 0; (i < inv.set.length()) && (n <= inv.lim); i++)
    if (s[inv.set[i]]) n += 1;
  if (n > inv.lim) {
    if (trace_level > 2) {
      ::std::cerr << "state ";
      write_atom_set(::std::cerr, s);
      ::std::cerr << " violates invariant " << '|';
      write_atom_set(::std::cerr, inv.set);
      ::std::cerr << "| <= " << inv.lim << ::std::endl;
    }
    return false;
  }
  else if ((n < inv.lim) && inv.exact) {
    if (trace_level > 2) {
      ::std::cerr << "state ";
      write_atom_set(::std::cerr, s);
      ::std::cerr << " violates invariant " << '|';
      write_atom_set(::std::cerr, inv.set);
      ::std::cerr << "| == " << inv.lim << ::std::endl;
    }
    return false;
  }
  else {
    return true;
  }
}

void Instance::negation_atom_set(const index_set& pset, index_set& nset) const
{
  for (index_type k = 0; k < pset.length(); k++)
    if (atoms[pset[k]].neg != no_such_index)
      nset.insert(atoms[pset[k]].neg);
}

bool Instance::verify_invariant(Instance::Constraint& inv)
{
  if (!cross_referenced()) cross_reference();

  if (trace_level > 1) {
    ::std::cerr << "verifying invariant |";
    write_atom_set(::std::cerr, inv.set);
    ::std::cerr << "| " << (inv.exact ? "==" : "<=") << " " << inv.lim
	      << "..." << ::std::endl;
  }

  index_type init_n = init_atoms.count_common(inv.set);
  if (init_n > inv.lim) {
    if (trace_level > 2) {
      ::std::cerr << " - violated by initial state" << ::std::endl;
    }
    return false;
  }
  else if (init_n < inv.lim) {
    if (trace_level > 2) {
      ::std::cerr << " - not exact in initial state" << ::std::endl;
    }
    return false;
  }

  for (index_type k = 0; k < n_actions(); k++) {
    index_type n_pre = actions[k].pre.count_common(inv.set);
    if (n_pre <= inv.lim) {
      index_type n_add = actions[k].add.count_common(inv.set);
      index_type n_del = actions[k].del.count_common(inv.set);
      if (n_add > n_del) {
	if (trace_level > 2) {
	  ::std::cerr << " - violated by action "
		    << actions[k].name << ::std::endl;
	}
	return false;
      }
      if (n_add < n_del) {
	if (trace_level > 2) {
	  ::std::cerr << " - not exact by action "
		    << actions[k].name << ::std::endl;
	}
	return false;
      }
    }
  }

  if (!inv.exact) {
    if (trace_level > 2) {
      ::std::cerr << " - is in fact exact: strengthening" << ::std::endl;
    }
    inv.exact = true;
  }
  else if (trace_level > 2) {
    ::std::cerr << " - ok" << ::std::endl;
  }

  return true;
}

void Instance::verify_invariants()
{
  if (trace_level > 0) {
    ::std::cerr << "verifying invariants (instance only)..." << ::std::endl;
  }
  for (index_type k = 0; k < n_invariants(); k++)
    invariants[k].verified = verify_invariant(invariants[k]);
}

index_type Instance::n_verified_invariants() const
{
  index_type n = 0;
  for (index_type k = 0; k < n_invariants(); k++)
    if (invariants[k].verified) n += 1;
  return n;
}

#ifdef NOT_YET_IMPLEMENTED
void Instance::extend_inv
(index_type p, index_set& inv, index_set& exc, index_set& acts)
{
  for (index_type k = 0; k < n_actions(); k++) {
    if (actions[k].pre.contains(p) && actions[k].del.contains(p)) {
      index_set cons(actions[k].pre);
      cons.intersect(actions[k].del);
      cons.subtract(p);
      new_exc.insert(cons);
      index_type n_add = actions[k].add.count_common(inv);
      if (n_add > 1) return;
      else if (n_add == 1) {
	// ...
      }
      else {
	// ...
      }
    }
  }
}
#endif

void Instance::cross_reference()
{
  if (xrf) return;
  for (index_type k = 0; k < actions.length(); k++) {
    Action& act = actions[k];
    for (index_type i = 0; i < act.pre.length(); i++)
      atoms[act.pre[i]].req_by.append(k);
    for (index_type i = 0; i < act.add.length(); i++)
      atoms[act.add[i]].add_by.append(k);
    for (index_type i = 0; i < act.del.length(); i++)
      atoms[act.del[i]].del_by.append(k);
    for (index_type i = 0; i < n_resources(); i++) {
      if (act.use[i] > 0) resources[i].used = true;
      if (act.cons[i] > 0) resources[i].consumed = true;
    }
    if (act.pre.length() > max_pre) max_pre = act.pre.length();
    if (act.add.length() > max_add) max_add = act.add.length();
    if (act.del.length() > max_del) max_del = act.del.length();
    if (act.lck.length() > max_lck) max_lck = act.lck.length();
    if (act.dur < min_dur) min_dur = act.dur;
    if (act.dur > max_dur) max_dur = act.dur;
    if (act.cost < min_cost) min_cost = act.cost;
    if (act.cost > max_cost) max_cost = act.cost;
  }
  init_atoms.clear();
  goal_atoms.clear();
  for (index_type k = 0; k < atoms.length(); k++) {
    Atom& prop = atoms[k];
    if (prop.req_by.length() > max_req_by)
      max_req_by = prop.req_by.length();
    if (prop.add_by.length() > max_add_by)
      max_add_by = prop.add_by.length();
    if (prop.del_by.length() > max_del_by)
      max_del_by = prop.del_by.length();
    if (prop.init) init_atoms.insert(k);
    if (prop.goal) goal_atoms.insert(k);
  }
  xrf = true;

  atom_set_hash.init(n_atoms());
  action_set_hash.init(n_actions());
}

void Instance::remap_set(index_set& set, const index_vec& map)
{
  index_type scan_p = 0;
  index_type put_p = 0;
  while (scan_p < set.length()) {
    if (map[set[scan_p]] != no_such_index) {
      set[put_p] = map[set[scan_p]];
      put_p += 1;
    }
    scan_p += 1;
  }
  set.set_length(put_p);
}

void Instance::remap_sets(index_set_vec& sets, const index_vec& map)
{
  for (index_type k = 0; k < sets.length(); k++)
    remap_set(sets[k], map);
}

void Instance::remove_actions(const bool_vec& set, index_vec& map)
{
  assert(set.length() >= actions.length());
  index_type scan_p = 0;
  index_type put_p = 0;
  index_vec rm_map(no_such_index, actions.length());
  while (scan_p < actions.length()) {
    if (!set[scan_p]) {
      if (put_p < scan_p) {
	actions[put_p] = actions[scan_p];
	actions[put_p].index = put_p;
      }
      rm_map[scan_p] = put_p;
      put_p += 1;
    }
    else {
      rm_map[scan_p] = no_such_index;
    }
    scan_p += 1;
  }
  actions.set_length(put_p);

  for (index_type k = 0; k < map.length(); k++)
    if (map[k] != no_such_index) {
      assert(map[k] < rm_map.length());
      map[k] = rm_map[map[k]];
    }
}

void Instance::remove_atoms(const bool_vec& set, index_vec& map)
{
  index_type scan_p = 0;
  index_type put_p = 0;
  index_vec rm_map(no_such_index, atoms.length());
  while (scan_p < atoms.length()) {
    if (!set[scan_p]) {
      if (put_p < scan_p) {
	atoms[put_p] = atoms[scan_p];
	atoms[put_p].index = put_p;
      }
      rm_map[scan_p] = put_p;
      put_p += 1;
    }
    else {
      rm_map[scan_p] = no_such_index;
    }
    scan_p += 1;
  }
  atoms.set_length(put_p);

  // remap negations
  for (index_type k = 0; k < atoms.length(); k++) {
    if (atoms[k].neg != no_such_index)
      atoms[k].neg = rm_map[atoms[k].neg];
  }

  for (index_type k = 0; k < actions.length(); k++) {
    remap_set(actions[k].pre, rm_map);
    remap_set(actions[k].add, rm_map);
    remap_set(actions[k].del, rm_map);
    remap_set(actions[k].lck, rm_map);
  }
  for (index_type k = 0; k < invariants.length(); k++) {
    remap_set(invariants[k].set, rm_map);
  }

  // should be equivalent to:
  // mapping::compose(map, rm_map, map);
  for (index_type k = 0; k < map.length(); k++)
    if (map[k] != no_such_index) {
      assert(map[k] < rm_map.length());
      map[k] = rm_map[map[k]];
    }
}

void Instance::remove_invariants(const bool_vec& set, index_vec& map)
{
  index_type scan_p = 0;
  index_type put_p = 0;
  index_vec rm_map(no_such_index, invariants.length());
  while (scan_p < invariants.length()) {
    if (!set[scan_p]) {
      if (put_p < scan_p) {
	invariants[put_p] = invariants[scan_p];
	invariants[put_p].index = put_p;
      }
      rm_map[scan_p] = put_p;
      put_p += 1;
    }
    else {
      rm_map[scan_p] = no_such_index;
    }
    scan_p += 1;
  }
  invariants.set_length(put_p);

  for (index_type k = 0; k < map.length(); k++)
    if (map[k] != no_such_index) {
      assert(map[k] < rm_map.length());
      map[k] = rm_map[map[k]];
    }
}

// note: set_initial does not have to clear cross-reference information,
// because this information is maintained (if valid before call).
void Instance::set_initial(const index_set& init)
{
  for (index_type k = 0; k < n_atoms(); k++) atoms[k].init = false;
  for (index_type k = 0; k < init.length(); k++) atoms[init[k]].init = true;
  init_atoms = init;
}

// note: set_goal does not have to clear cross-reference information,
// because this information is maintained (if valid before call).
void Instance::set_goal(const index_set& goal)
{
  for (index_type k = 0; k < n_atoms(); k++) {
    atoms[k].goal = false;
    atoms[k].irrelevant = false;
  }
  for (index_type k = 0; k < goal.length(); k++) atoms[goal[k]].goal = true;
  goal_atoms = goal;
}

void Instance::set_DNF_goal(const index_set_vec& goal)
{
  for (index_type k = 0; k < n_atoms(); k++) {
    atoms[k].goal = false;
    atoms[k].irrelevant = false;
  }
  Atom& g_atom = new_atom(new StringName(goal_atom_name));
  g_atom.goal = true;
  for (index_type k = 0; k < goal.length(); k++) {
    Action& g_act = new_action(new EnumName(goal_action_name, k));
    g_act.pre = goal[k];
    g_act.add.assign_singleton(g_atom.index);
    g_act.cost = goal_action_cost;
    g_act.dmin = goal_action_cost;
    g_act.dmax = goal_action_cost;
    g_act.dur = goal_action_cost;
  }
  clear_cross_reference();
}

void Instance::replace_atom_by_conjunction(index_type p, const index_set& c)
{
  assert(p < n_atoms());
  for (index_type k = 0; k < n_actions(); k++) {
    if (actions[k].pre.contains(p))
      actions[k].pre.insert(c);
  }
  if (atoms[p].goal) {
    for (index_type k = 0; k < c.length(); k++)
      atoms[c[k]].goal = true;
  }
}

void Instance::set_cost_bound(NTYPE b)
{
  Resource& r = new_resource(new StringName("_bcost"));
  for (index_type k = 0; k < n_actions(); k++)
    actions[k].cons[r.index] = actions[k].cost;
  r.init = b;
}

void Instance::create_composite_resource(const index_set& set)
{
  ConcatenatedName* cn = new ConcatenatedName();
  for (index_type r = 0; r < set.length(); r++) {
    assert(set[r] < n_resources());
    cn->append(resources[set[r]].name);
  }
  // ::std::cerr << "composite resource name: " << cn << ::std::endl;
  Resource& cr = new_resource(cn);
  for (index_type r = 0; r < set.length(); r++) {
    cr.init += resources[set[r]].init;
  }
  // ::std::cerr << "composite resource init value: " << cr.init << ::std::endl;
  for (index_type k = 0; k < n_actions(); k++) {
    NTYPE cu = 0;
    NTYPE cc = 0;
    for (index_type r = 0; r < set.length(); r++) {
      cu += actions[k].use[set[r]];
      cc += actions[k].cons[set[r]];
    }
    actions[k].use[cr.index] = cu;
    actions[k].cons[cr.index] = cc;
    // ::std::cerr << "action " << k << " consumes " << cc << ::std::endl;
    if (cu > 0) resources[cr.index].used = true;
    if (cc > 0) resources[cr.index].consumed = true;
  }
}

void Instance::create_total_resource()
{
  index_set s;
  s.fill(n_resources());
  create_composite_resource(s);
}

void Instance::add_all_negation_invariants()
{
  index_type n_new = 0;
  for (index_type k = 0; k < n_atoms(); k++) {
    if (atoms[k].neg != no_such_index) {
      Constraint& c = new_invariant();
      c.set.insert(k);
      c.set.insert(atoms[k].neg);
      c.lim = 1;
      c.exact = true;
      n_new += 1;
    }
  }
  if (trace_level > 0) {
    ::std::cerr << n_new << " negation invariants added" << ::std::endl;
  }
}

void Instance::add_missing_negation_invariants()
{
  bool_vec a(false, n_atoms());
  for (index_type k = 0; k < n_invariants(); k++)
    for (index_type i = 0; i < invariants[k].set.length(); i++)
      a[invariants[k].set[i]] = true;
  if (trace_level > 0) {
    ::std::cerr << a.count(true) << " of " << n_atoms()
	      << " atoms part of some invariant..." << ::std::endl;
  }
  index_type n_new = 0;
  for (index_type k = 0; k < n_atoms(); k++) if (!a[k]) {
    if (atoms[k].neg != no_such_index) {
      Constraint& c = new_invariant();
      c.set.insert(k);
      c.set.insert(atoms[k].neg);
      c.lim = 1;
      c.exact = true;
      a[k] = true;
      a[atoms[k].neg] = true;
      n_new += 1;
    }
  }
  if (trace_level > 0) {
    ::std::cerr << n_new << " negation invariants added" << ::std::endl;
  }
}

void Instance::create_atom_negation(index_type a)
{
  Atom& not_a = new_atom(new ModName(atoms[a].name, "neg-of"));
  not_a.init = !atoms[a].init;
  not_a.neg = atoms[a].index;
  atoms[a].neg = not_a.index;
  for (index_type k = 0; k < n_actions(); k++) {
    if (actions[k].add.contains(a)) actions[k].del.insert(not_a.index);
    if (actions[k].del.contains(a)) actions[k].add.insert(not_a.index);
  }
}

index_type Instance::complete_atom_negation(index_type a)
{
  assert(a < atoms.length());
  if (atoms[a].neg == no_such_index) {
    create_atom_negation(a);
    clear_cross_reference();
  }
  assert(atoms[a].neg != no_such_index);
  return atoms[a].neg;
}

void Instance::complete_atom_negations(const index_set& s)
{
  index_type n = n_atoms();
  for (index_type k = 0; k < s.length(); k++)
    if (atoms[s[k]].neg == no_such_index)
      create_atom_negation(s[k]);
  if (n_atoms() > n)
    clear_cross_reference();
}

void Instance::complete_atom_negations()
{
  index_type n = n_atoms();
  for (index_type k = 0; k < n; k++)
    if (atoms[k].neg == no_such_index)
      create_atom_negation(k);
  if (n_atoms() > n)
    clear_cross_reference();
}

void Instance::extract_atom_negations_from_invariants()
{
  for (index_type k = 0; k < n_invariants(); k++)
    if ((invariants[k].set.length() == 2) &&
	(invariants[k].lim == 1) &&
	invariants[k].exact) {
      index_type p = invariants[k].set[0];
      index_type q = invariants[k].set[1];
      assert(p < n_atoms());
      assert(q < n_atoms());
      if ((atoms[p].neg == no_such_index) &&
	  (atoms[q].neg == no_such_index)) {
	atoms[p].neg = q;
	atoms[q].neg = p;
      }
    }
}

void Instance::compute_iff_axioms(rule_set& ax)
{
  extract_atom_negations_from_invariants();
  for (index_type k = 0; k < n_invariants(); k++)
    if ((invariants[k].lim == 1) && invariants[k].exact) {
      index_set n_set;
      index_type p0 = no_such_index;
      bool ok = true;
      for (index_type i = 0; (i < invariants[k].set.length()) && ok; i++) {
	index_type p = invariants[k].set[i];
	if ((atoms[p].neg != no_such_index) &&
	    !invariants[k].set.contains(atoms[p].neg)) {
	  n_set.insert(atoms[p].neg);
	}
	else if (p0 == no_such_index) {
	  p0 = p;
	}
	else {
	  ok = false;
	}
      }
      if (ok && (n_set.length() == invariants[k].set.length())) {
	for (index_type i = 0; i < invariants[k].set.length(); i++) {
	  rule r(n_set, invariants[k].set[i]);
	  assert(r.antecedent.contains(atoms[r.consequent].neg));
	  r.antecedent.subtract(atoms[r.consequent].neg);
	  ax.insert(r);
	}
      }
      if (ok && (n_set.length() == (invariants[k].set.length() - 1))) {
	assert(p0 < n_atoms());
	rule r(n_set, p0);
	ax.insert(r);
      }
    }
}

index_type Instance::create_history_atom(index_type a)
{
  Atom& ha = new_atom(new ModName(atoms[a].name, "reached"));
  ha.init = atoms[a].init;
  for (index_type k = 0; k < n_actions(); k++)
    if (actions[k].add.contains(a)) actions[k].add.insert(ha.index);
  return ha.index;
}

index_type Instance::compile_pc_always(const index_set& f, const Name* n)
{
  if (!n) n = new EnumName(pc_name, pc_count++);
  if (trace_level > 2) {
    ::std::cerr << "compiling " << n << ": always ";
    write_atom_set(std::cerr, f);
    std::cerr << "..." << ::std::endl;
  }
  Atom& a_ok = new_atom(new ModName(n, "ok"));
  a_ok.init = true;
  a_ok.goal = true;
  for (index_type k = 0; (k < f.length()) && a_ok.init; k++)
    if (!atoms[f[k]].init) a_ok.init = false;
  if (a_ok.init) {
    for (index_type k = 0; k < n_actions(); k++)
      if (actions[k].del.first_common_element(f) != no_such_index)
	actions[k].del.insert(a_ok.index);
  }
  clear_cross_reference();
  return a_ok.index;
}

index_type Instance::compile_pc_sometime(const index_set& f, const Name* n)
{
  if (!n) n = new EnumName(pc_name, pc_count++);
  if (trace_level > 2) {
    ::std::cerr << "compiling " << n << ": sometime ";
    write_atom_set(std::cerr, f);
    std::cerr << "..." << ::std::endl;
  }
  Atom& a_ok = new_atom(new ModName(n, "ok"));
  a_ok.init = true;
  a_ok.goal = true;
  for (index_type k = 0; (k < f.length()) && a_ok.init; k++)
    if (!atoms[f[k]].init) a_ok.init = false;
  if (!a_ok.init) {
    index_type n_act = n_actions();
    for (index_type k = 0; k < n_act; k++)
      if ((actions[k].add.first_common_element(f) != no_such_index) &&
	  (actions[k].del.first_common_element(f) == no_such_index)) {
	index_set c(f);
	c.subtract(actions[k].add);
	if (c.empty()) {
	  actions[k].add.insert(a_ok.index);
	}
	else {
	  Action& a2 = copy_action(k);
	  a2.pre.insert(c);
	  a2.add.insert(a_ok.index);
	}
      }
  }
  clear_cross_reference();
  return a_ok.index;
}

index_type Instance::compile_pc_at_most_once(const index_set& f, const Name* n)
{
  if (!n) n = new EnumName(pc_name, pc_count++);
  if (trace_level > 2) {
    ::std::cerr << "compiling " << n << ": at-most-once ";
    write_atom_set(std::cerr, f);
    std::cerr << "..." << ::std::endl;
  }
  Atom& a_ok = new_atom(new ModName(n, "ok"));
  a_ok.init = true;
  a_ok.goal = true;
  Atom& a_once = new_atom(new ModName(n, "once"));
  a_once.init = false;
  a_once.goal = false;

  // compute set of "missing" atom negations
  index_set m;
  for (index_type k = 0; k < n_actions(); k++) {
    if (actions[k].del.first_common_element(f) != no_such_index) {
      index_set c(f);
      c.subtract(actions[k].pre);
      m.insert(c);
    }
    if ((actions[k].add.first_common_element(f) != no_such_index) &&
	(actions[k].del.first_common_element(f) == no_such_index)) {
      index_set c(f);
      c.subtract(actions[k].add);
      m.insert(c);
    }
  }
  complete_atom_negations(m);

  index_type n_act = n_actions();
  for (index_type k = 0; k < n_act; k++)
    if (actions[k].del.first_common_element(f) != no_such_index) {
      index_set c(f);
      c.subtract(actions[k].pre);
      if (c.empty()) {
	actions[k].add.insert(a_once.index);
      }
      else {
	for (index_type i = 0; i < c.length(); i++) {
	  assert(atoms[c[i]].neg != no_such_index);
	  Action& a2 = copy_action(k);
	  a2.pre.insert(atoms[c[i]].neg);
	}
	actions[k].pre.insert(c);
	actions[k].add.insert(a_once.index);
      }
    }

  index_type a_not_once = complete_atom_negation(a_once.index);

  n_act = n_actions();
  for (index_type k = 0; k < n_act; k++)
    if ((actions[k].add.first_common_element(f) != no_such_index) &&
	(actions[k].del.first_common_element(f) == no_such_index)) {
      index_set c(f);
      c.subtract(actions[k].add);
      if (c.empty()) {
	Action& a2 = copy_action(k);
	actions[k].pre.insert(a_not_once);
	a2.pre.insert(a_once.index);
	a2.del.insert(a_ok.index);
      }
      else {
	for (index_type i = 0; i < c.length(); i++) {
	  assert(atoms[c[i]].neg != no_such_index);
	  Action& a2 = copy_action(k);
	  a2.pre.insert(atoms[c[i]].neg);
	}
	Action& a3 = copy_action(k);
	a3.pre.insert(c);
	a3.pre.insert(a_once.index);
	a3.del.insert(a_ok.index);
	actions[k].pre.insert(c);
	actions[k].pre.insert(a_not_once);
      }
    }

  clear_cross_reference();
  return a_ok.index;
}

index_type Instance::compile_pc_sometime_before
(const index_set& f_t, const index_set& f_c, const Name* n)
{
  if (!n) n = new EnumName(pc_name, pc_count++);
  if (trace_level > 2) {
    ::std::cerr << "compiling " << n << ": sometime-before ";
    write_atom_set(std::cerr, f_t);
    ::std::cerr << " ";
    write_atom_set(std::cerr, f_c);
    std::cerr << "..." << ::std::endl;
  }
  Atom& a_safe =  new_atom(new ModName(n, "safe"));
  a_safe.init = true;
  for (index_type k = 0; k < f_c.length(); k++)
    if (!atoms[f_c[k]].init) a_safe.init = false;
  a_safe.goal = false;
  Atom& a_ok =  new_atom(new ModName(n, "ok"));
  a_ok.init = false;
  for (index_type k = 0; k < f_t.length(); k++)
    if (!atoms[f_t[k]].init) a_ok.init = true;
  a_ok.goal = true;

  if (a_ok.init && !a_safe.init) {
    // compute set of "missing" atom negations
    index_set m;
    for (index_type k = 0; k < n_actions(); k++) {
      if ((actions[k].add.first_common_element(f_t) != no_such_index) &&
	  (actions[k].del.first_common_element(f_t) == no_such_index)) {
	index_set c(f_t);
	c.subtract(actions[k].add);
	m.insert(c);
      }
    }
    if (trace_level > 2) {
      ::std::cerr << "completing negations of ";
      write_atom_set(std::cerr, m);
      std::cerr << "..." << ::std::endl;
    }
    complete_atom_negations(m);

    index_type n_act = n_actions();
    for (index_type k = 0; k < n_act; k++)
      if ((actions[k].add.first_common_element(f_c) != no_such_index) &&
	  (actions[k].del.first_common_element(f_c) == no_such_index)) {
	index_set c(f_c);
	c.subtract(actions[k].add);
	if (c.empty()) {
	  actions[k].add.insert(a_safe.index);
	  if (trace_level > 2) {
	    std::cerr << "case 1: atom " << a_safe.index << "." << a_safe.name
		      << " add by " << k << "." << actions[k].name
		      << std::endl;
	  }
	}
	else {
	  Action& a2 = copy_action(k);
	  a2.pre.insert(c);
	  a2.add.insert(a_safe.index);
	  if (trace_level > 2) {
	    std::cerr << "case 2: atoms " << c << " pre of "
		      << a2.index << "." << a2.name
		      << std::endl;
	    std::cerr << "case 2: atom " << a_safe.index << "." << a_safe.name
		      << " add by " << a2.index << "." << a2.name
		      << std::endl;
	  }
	  // print_action(std::cerr << "new action ", a2);
	}
      }

    n_act = n_actions();
    for (index_type k = 0; k < n_act; k++)
      if ((actions[k].add.first_common_element(f_t) != no_such_index) &&
	  (actions[k].del.first_common_element(f_t) == no_such_index)) {
	index_set c(f_t);
	c.subtract(actions[k].add);
	if (c.empty()) {
	  Action& a2 = copy_action(k);
	  a2.del.insert(a_ok.index);
	  if (trace_level > 2) {
	    std::cerr << "case 3: atom " << a_ok.index << "." << a_ok.name
		      << " del by " << a2.index << "." << a2.name
		      << std::endl;
	  }
	  // print_action(std::cerr << "new action ", a2);
	  actions[k].pre.insert(a_safe.index);
	  if (trace_level > 2) {
	    std::cerr << "case 4: atom " << a_safe.index << "." << a_safe.name
		      << " pre of " << k << "." << actions[k].name
		      << std::endl;
	  }
	}
	else {
	  for (index_type i = 0; i < c.length(); i++) {
	    assert(atoms[c[i]].neg != no_such_index);
	    Action& a2 = copy_action(k);
	    a2.pre.insert(atoms[c[i]].neg);
	    if (trace_level > 2) {
	      std::cerr << "case 5: atom " << atoms[c[i]].neg
			<< " pre of " << a2.index << "." << a2.name
			<< std::endl;
	    }
	    // print_action(std::cerr << "new action ", a2);
	  }
	  Action& a3 = copy_action(k);
	  a3.del.insert(a_ok.index);
	  if (trace_level > 2) {
	    std::cerr << "case 6: atom " << a_ok.index << "." << a_ok.name
		      << " del by " << a3.index << "." << a3.name
		      << std::endl;
	  }
	  // print_action(std::cerr << "new action ", a3);
	  actions[k].pre.insert(a_safe.index);
	  if (trace_level > 2) {
	    std::cerr << "case 7: atom " << a_safe.index << "." << a_safe.name
		      << " pre of " << k << "." << actions[k].name
		      << std::endl;
	  }
	}
      }
  }

  clear_cross_reference();
  return a_ok.index;
}

void Instance::enforce_pc_always(const index_set& f, const Name* n)
{
  if (trace_level > 2) {
    ::std::cerr << "enforcing ";
    if (n) std::cerr << n << ": ";
    std::cerr << "always ";
    write_atom_set(std::cerr, f);
    std::cerr << "..." << ::std::endl;
  }
  bool_vec d(false, n_actions());
  for (index_type k = 0; k < n_actions(); k++)
    if (actions[k].del.first_common_element(f) != no_such_index)
      d[k] = true;
  actions.remove(d);
  clear_cross_reference();
}

void Instance::enforce_pc_sometime(const index_set& f, const Name* n)
{
  if (trace_level > 2) {
    ::std::cerr << "enforcing ";
    if (n) std::cerr << n << ": ";
    std::cerr << "sometime ";
    write_atom_set(std::cerr, f);
    std::cerr << "..." << ::std::endl;
  }
  index_type g = compile_pc_sometime(f, n);
  atoms[g].goal = true;
  clear_cross_reference();
}

void Instance::enforce_pc_at_most_once(const index_set& f, const Name* n)
{
  if (!n) n = new EnumName(pc_name, pc_count++);
  if (trace_level > 2) {
    ::std::cerr << "enforcing " << n << ": at-most-once ";
    write_atom_set(std::cerr, f);
    std::cerr << "..." << ::std::endl;
  }
  index_type a_first = new_atom(new ModName(n, "first")).index;
  atoms[a_first].init = true;
  atoms[a_first].goal = false;

  // compute set of "missing" atom negations
  index_set m;
  for (index_type k = 0; k < n_actions(); k++) {
    if (actions[k].del.first_common_element(f) != no_such_index) {
      index_set c(f);
      c.subtract(actions[k].pre);
      m.insert(c);
    }
    if ((actions[k].add.first_common_element(f) != no_such_index) &&
	(actions[k].del.first_common_element(f) == no_such_index)) {
      index_set c(f);
      c.subtract(actions[k].add);
      m.insert(c);
    }
  }
  complete_atom_negations(m);

  index_type n_act = n_actions();
  for (index_type k = 0; k < n_act; k++)
    if (actions[k].del.first_common_element(f) != no_such_index) {
      index_set c(f);
      c.subtract(actions[k].pre);
      if (c.empty()) {
	actions[k].del.insert(a_first);
      }
      else {
	for (index_type i = 0; i < c.length(); i++) {
	  assert(atoms[c[i]].neg != no_such_index);
	  Action& a2 = copy_action(k);
	  a2.pre.insert(atoms[c[i]].neg);
	}
	actions[k].pre.insert(c);
	actions[k].del.insert(a_first);
      }
    }

  n_act = n_actions();
  for (index_type k = 0; k < n_act; k++)
    if ((actions[k].add.first_common_element(f) != no_such_index) &&
	(actions[k].del.first_common_element(f) == no_such_index)) {
      index_set c(f);
      c.subtract(actions[k].add);
      if (c.empty()) {
	actions[k].pre.insert(a_first);
      }
      else {
	for (index_type i = 0; i < c.length(); i++) {
	  assert(atoms[c[i]].neg != no_such_index);
	  Action& a2 = copy_action(k);
	  a2.pre.insert(atoms[c[i]].neg);
	}
	actions[k].pre.insert(c);
	actions[k].pre.insert(a_first);
      }
    }

  clear_cross_reference();
}

void Instance::enforce_pc_sometime_before
(const index_set& f_t, const index_set& f_c, const Name* n)
{
  if (!n) n = new EnumName(pc_name, pc_count++);
  if (trace_level > 2) {
    ::std::cerr << "compiling " << n << ": sometime-before ";
    write_atom_set(std::cerr, f_t);
    ::std::cerr << " ";
    write_atom_set(std::cerr, f_c);
    std::cerr << "..." << ::std::endl;
  }

  bool is_safe = true;
  for (index_type k = 0; k < f_c.length(); k++)
    if (!atoms[f_c[k]].init) is_safe = false;

  if (!is_safe) {
    index_type a_safe =  new_atom(new ModName(n, "safe")).index;
    atoms[a_safe].init = false;
    atoms[a_safe].goal = false;

    // compute set of "missing" atom negations
    index_set m;
    for (index_type k = 0; k < n_actions(); k++) {
      if ((actions[k].add.first_common_element(f_t) != no_such_index) &&
	  (actions[k].del.first_common_element(f_t) == no_such_index)) {
	index_set c(f_t);
	c.subtract(actions[k].add);
	m.insert(c);
      }
    }
    if (trace_level > 2) {
      ::std::cerr << "completing negations of ";
      write_atom_set(std::cerr, m);
      std::cerr << "..." << ::std::endl;
    }
    complete_atom_negations(m);

    index_type n_act = n_actions();
    for (index_type k = 0; k < n_act; k++)
      if ((actions[k].add.first_common_element(f_c) != no_such_index) &&
	  (actions[k].del.first_common_element(f_c) == no_such_index)) {
	index_set c(f_c);
	c.subtract(actions[k].add);
	c.subtract(actions[k].pre);
	if (c.empty()) {
	  actions[k].add.insert(a_safe);
	}
	else {
	  Action& a2 = copy_action(k);
	  a2.pre.insert(c);
	  a2.add.insert(a_safe);
	}
      }

    n_act = n_actions();
    for (index_type k = 0; k < n_act; k++)
      if ((actions[k].add.first_common_element(f_t) != no_such_index) &&
	  (actions[k].del.first_common_element(f_t) == no_such_index)) {
	index_set c(f_t);
	c.subtract(actions[k].add);
	if (c.empty()) {
	  actions[k].pre.insert(a_safe);
	}
	else {
	  for (index_type i = 0; i < c.length(); i++) {
	    assert(atoms[c[i]].neg != no_such_index);
	    Action& a2 = copy_action(k);
	    a2.pre.insert(atoms[c[i]].neg);
	  }
	  actions[k].pre.insert(a_safe);
	}
      }
  }

  clear_cross_reference();
}

void Instance::clear_cross_reference()
{
  max_pre = 0;
  max_add = 0;
  max_del = 0;
  max_lck = 0;
  max_req_by = 0;
  max_add_by = 0;
  max_del_by = 0;
  min_dur = POS_INF;
  max_dur = NEG_INF;
  min_cost = POS_INF;
  max_cost = NEG_INF;
  for (index_type k = 0; k < atoms.length(); k++) {
    atoms[k].req_by.clear();
    atoms[k].add_by.clear();
    atoms[k].del_by.clear();
  }
  init_atoms.clear();
  goal_atoms.clear();

  for (index_type k = 0; k < actions.length(); k++)
    actions[k].ncw_atms.clear();

  for (index_type k = 0; k < resources.length(); k++) {
    resources[k].used = false;
    resources[k].consumed = false;
  }

  xrf = false;
}

bool Instance::cross_referenced() const
{
  return xrf;
}

void Instance::save_durations(cost_vec& out) const
{
  out.assign_value(0, n_actions());
  for (index_type k = 0; k < n_actions(); k++)
    out[k] = actions[k].dur;
}

void Instance::set_durations(const cost_vec& in)
{
  for (index_type k = 0; k < n_actions(); k++)
    actions[k].dur = in[k];
}

void Instance::set_durations(const cost_vec& in, cost_vec& out)
{
  out.assign_value(0, n_actions());
  for (index_type k = 0; k < n_actions(); k++) {
    out[k] = actions[k].dur;
    actions[k].dur = in[k];
  }
}

void Instance::assign_unit_durations(NTYPE unit)
{
  for (index_type k = 0; k < n_actions(); k++)
    actions[k].dur = unit;
  min_dur = unit;
  max_dur = unit;
}

void Instance::discretize_durations(NTYPE interval_width)
{
  min_dur = POS_INF;
  max_dur = NEG_INF;
  for (index_type k = 0; k < n_actions(); k++) {
    NTYPE d = (actions[k].dur / interval_width);
    if (!INTEGRAL(d)) d = FLOOR(d) + 1;
    actions[k].dur = d*interval_width;
    min_dur = MIN(min_dur, actions[k].dur);
    max_dur = MAX(max_dur, actions[k].dur);
  }
}

void Instance::quantize_durations(index_type n_intervals)
{
  max_dur = NEG_INF;
  for (index_type k = 0; k < n_actions(); k++)
    max_dur = MAX(max_dur, actions[k].dur);
  NTYPE w = max_dur / n_intervals;
  min_dur = POS_INF;
  max_dur = NEG_INF;
  for (index_type k = 0; k < n_actions(); k++) {
    NTYPE d = (actions[k].dur / w);
    if (!INTEGRAL(d)) d = FLOOR(d) + 1;
    actions[k].dur = d*w;
    min_dur = MIN(min_dur, actions[k].dur);
    max_dur = MAX(max_dur, actions[k].dur);
  }
}

void Instance::round_durations_up()
{
  min_dur = POS_INF;
  max_dur = NEG_INF;
  for (index_type k = 0; k < n_actions(); k++) {
    if (!INTEGRAL(actions[k].dur)) {
      actions[k].dur = FLOOR(actions[k].dur) + 1;
    }
    min_dur = MIN(min_dur, actions[k].dur);
    max_dur = MAX(max_dur, actions[k].dur);
  }
}

void Instance::round_durations_down()
{
  min_dur = POS_INF;
  max_dur = NEG_INF;
  for (index_type k = 0; k < n_actions(); k++) {
    actions[k].dur = FLOOR(actions[k].dur);
    min_dur = MIN(min_dur, actions[k].dur);
    max_dur = MAX(max_dur, actions[k].dur);
  }
}

void Instance::round_durations()
{
  min_dur = POS_INF;
  max_dur = NEG_INF;
  for (index_type k = 0; k < n_actions(); k++) {
    if (FRAC(actions[k].dur) > R_TO_N(1,2)) {
      actions[k].dur = FLOOR(actions[k].dur) + 1;
    }
    else {
      actions[k].dur = FLOOR(actions[k].dur);
    }
    min_dur = MIN(min_dur, actions[k].dur);
    max_dur = MAX(max_dur, actions[k].dur);
  }
}

void Instance::assign_unit_costs(cost_vec& save)
{
  save.assign_value(0, n_actions());
  for (index_type k = 0; k < n_actions(); k++) {
    save[k] = actions[k].cost;
    actions[k].cost = 1;
  }
  min_cost = 1;
  max_cost = 1;
}

void Instance::restore_costs(const cost_vec& saved)
{
  for (index_type k = 0; k < n_actions(); k++) {
    actions[k].dur = saved[k];
  }
}

void Instance::assign_unlimited_resources(cost_vec& save)
{
  save.assign_value(0, n_resources());
  for (index_type k = 0; k < n_resources(); k++) {
    save[k] = resources[k].init;
    resources[k].init = POS_INF;
  }
}

void Instance::restore_resources(const cost_vec& saved)
{
  for (index_type k = 0; k < n_resources(); k++)
    resources[k].init = saved[k];
}

index_type Instance::n_reusable_resources() const
{
  index_type n = 0;
  for (index_type k = 0; k < n_resources(); k++)
    if (resources[k].used && !resources[k].consumed) n += 1;
  return n;
}

index_type Instance::n_consumable_resources() const
{
  index_type n = 0;
  for (index_type k = 0; k < n_resources(); k++)
    if (resources[k].consumed) n += 1;
  return n;
}

void Instance::write_atom_set
(::std::ostream& s, const index_vec& set, unsigned int c) const
{
  s << '{';
  for (index_type k = 0; k < set.length(); k++) {
    if (k > 0) s << ',';
    if (write_atom_set_with_symbolic_names)
      atoms[set[k]].name->write(s, c);
    else
      s << set[k];
  }
  s << '}';
}

void Instance::write_atom_set
(::std::ostream& s, const bool_vec& set, unsigned int c) const
{
  s << '{';
  bool need_comma = false;
  for (index_type k = 0; k < n_atoms(); k++) if (set[k]) {
    if (need_comma) s << ',';
    if (write_atom_set_with_symbolic_names)
      atoms[k].name->write(s, c);
    else
      s << k;
    need_comma = true;
  }
  s << '}';
}

void Instance::write_atom_sets
(::std::ostream& s, const index_set_vec& sets, unsigned int c) const
{
  s << '{';
  for (index_type k = 0; k < sets.length(); k++) {
    if (k > 0) s << ',';
    write_atom_set(s, sets[k], c);
  }
  s << '}';
}

void Instance::write_action_set
(::std::ostream& s, const index_vec& set, unsigned int c) const
{
  s << '{';
  for (index_type k = 0; k < set.length(); k++) {
    if (k > 0) s << ',';
    if (write_action_set_with_symbolic_names)
      actions[set[k]].name->write(s, c);
    else
      s << set[k];
  }
  s << '}';
}

void Instance::write_action_set
(::std::ostream& s, const bool_vec& set, unsigned int c) const
{
  s << '{';
  bool need_comma = false;
  for (index_type k = 0; k < n_actions(); k++) if (set[k]) {
    if (need_comma) s << ',';
    if (write_action_set_with_symbolic_names)
      actions[k].name->write(s, c);
    else
      s << k;
    need_comma = true;
  }
  s << '}';
}

void Instance::write_iff_axiom
(::std::ostream& s, const rule& r) const
{
  s << atoms[r.consequent].name << " <-> (";
  for (index_type k = 0; k < r.antecedent.length(); k++) {
    if (k > 0) s << " & ";
    s << atoms[r.antecedent[k]].name;
  }
  s << ")";
}

void Instance::write_iff_axiom_set
(::std::ostream& s, const rule_set& rset) const
{
  s << '{';
  for (index_type k = 0; k < rset.length(); k++) {
    if (k > 0) s << ", ";
    s << k << ": ";
    write_iff_axiom(s, rset[k]);
  }
  s << '}';
}

void Instance::write_atom_digraph
(::std::ostream& s,
 const graph& g,
 const index_set& atomset,
 const bool_vec& mark_shaded,
 const bool_vec& mark_dashed,
 const char* label) const
{
  s << "digraph ADG {" << ::std::endl;
  s << "label=\"" << label << "\";" << ::std::endl;
  s << "node [width=0,height=0];" << ::std::endl;

  for (index_type k = 0; k < atomset.length(); k++) {
    s << "A" << k << " [";
    if (mark_shaded[atomset[k]]) {
      s << "style=filled,";
    }
    if (mark_dashed[atomset[k]]) {
      s << "style=dashed,";
    }
    s << "label=\"" << atoms[atomset[k]].name << "\"];"
      << ::std::endl;
  }

  for (index_type i = 0; i < atomset.length(); i++)
    for (index_type j = 0; j < atomset.length(); j++)
      if (g.adjacent(i, j)) {
	s << "A" << i << " -> A" << j << ";" << ::std::endl;
      }

  s << "}" << ::std::endl;
}

void Instance::write_atom_digraph
(::std::ostream& s, const graph& g, const char* label) const
{
  index_set all_atoms;
  all_atoms.fill(n_atoms());
  bool_vec no_atoms(false, n_atoms());
  write_atom_digraph(s, g, all_atoms, no_atoms, no_atoms, label);
}

void Instance::write_atom_action_digraph
(::std::ostream& s,
 const graph& g,
 const index_set& atomset,
 const index_set& actionset,
 const bool_vec& mark_shaded,
 const bool_vec& mark_bold,
 const bool_vec& mark_dashed,
 const char* label) const
{
  s << "digraph AADG {" << ::std::endl;
  s << "label=\"" << label << "\";" << ::std::endl;
  s << "node [width=0,height=0];" << ::std::endl;

  for (index_type k = 0; k < atomset.length(); k++) {
    s << "ATM" << k << " [shape=ellipse,";
    if (mark_shaded[atomset[k]]) {
      s << "style=filled,";
    }
    if (mark_bold[atomset[k]]) {
      s << "style=bold,";
    }
    if (mark_dashed[atomset[k]]) {
      s << "style=dashed,";
    }
    s << "label=\"" << atoms[atomset[k]].name << "\"];"
      << ::std::endl;
  }

  for (index_type k = 0; k < actionset.length(); k++) {
    s << "ACT" << k << " [shape=box,";
    if (mark_shaded[atomset.length() + actionset[k]]) {
      s << "style=filled,";
    }
    if (mark_bold[atomset.length() + actionset[k]]) {
      s << "style=bold,";
    }
    if (mark_dashed[atomset.length() + actionset[k]]) {
      s << "style=dashed,";
    }
    s << "label=\"" << actions[actionset[k]].name << "\"];"
      << ::std::endl;
  }

  for (index_type i = 0; i < atomset.length(); i++) {
    for (index_type j = 0; j < atomset.length(); j++)
      if (g.adjacent(i, j))
	s << "ATM" << i << " -> ATM" << j << ";" << ::std::endl;
    for (index_type j = 0; j < actionset.length(); j++)
      if (g.adjacent(i, atomset.length() + j))
	s << "ATM" << i << " -> ACT" << j << ";" << ::std::endl;
  }

  for (index_type i = 0; i < actionset.length(); i++) {
    for (index_type j = 0; j < atomset.length(); j++)
      if (g.adjacent(atomset.length() + i, j))
	s << "ACT" << i << " -> ATM" << j << ";" << ::std::endl;
    for (index_type j = 0; j < actionset.length(); j++)
      if (g.adjacent(atomset.length() + i, atomset.length() + j))
	s << "ACT" << i << " -> ACT" << j << ";" << ::std::endl;
  }

  s << "}" << ::std::endl;
}

void Instance::write_atom_set_digraph
(::std::ostream& s, const index_set_graph& g, const char* label) const
{
  s << "digraph SDG {" << ::std::endl;
  s << "label=\"" << label << "\";" << ::std::endl;
  s << "node [width=0,height=0];" << ::std::endl;

  for (index_type k = 0; k < g.size(); k++) {
    s << "S" << k << " [label=\"";
    write_atom_set(s, g.node_label(k));
    s << "\"];" << ::std::endl;
  }

  for (index_type i = 0; i < g.size(); i++)
    for (index_type j = 0; j < g.size(); j++)
      if (g.adjacent(i, j)) {
	s << "S" << i << " -> S" << j << ";" << ::std::endl;
      }

  s << "}" << ::std::endl;
}

void Instance::write_atom_set_graph
(::std::ostream& s, const index_set_graph& g, const char* label) const
{
  s << "graph SG {" << ::std::endl;
  s << "label=\"" << label << "\";" << ::std::endl;
  s << "node [width=0,height=0];" << ::std::endl;
  s << "edge [len=1.0];" << ::std::endl;
  s << "overlap=false;" << ::std::endl;
  // s << "spline=true;" << ::std::endl;
  // s << "sep=0.1;" << ::std::endl;

  for (index_type k = 0; k < g.size(); k++) {
    s << "S" << k << " [label=\"";
    if (g.node_has_label(k))
      write_atom_set(s, g.node_label(k));
    else
      s << "{}";
    s << "\"];" << ::std::endl;
  }

  for (index_type i = 0; i < g.size(); i++)
    for (index_type j = i + 1; j < g.size(); j++)
      if (g.adjacent(i, j) && g.adjacent(i, j)) {
	s << "S" << i << " -- S" << j;
	if (g.edge_has_label(i, j)) {
	  if (g.edge_has_label(j, i)) {
	    if (g.edge_label(i, j) == g.edge_label(j, i)) {
	      s << "[label=\"";
	      write_atom_set(s, g.edge_label(i, j));
	      s << "\"]";
	    }
	    else {
	      s << "[label=\"";
	      write_atom_set(s, g.edge_label(i, j));
	      s << " / ";
	      write_atom_set(s, g.edge_label(j, i));
	      s << "\"]";
	    }
	  }
	  else {
	    s << "[label=\"";
	    write_atom_set(s, g.edge_label(i, j));
	    s << " / {}\"]";
	  }
	}
	else if (g.edge_has_label(j, i)) {
	  s << "[label=\"{} / ";
	  write_atom_set(s, g.edge_label(j, i));
	  s << "\"]";
	}
	s << ";" << ::std::endl;
      }

  s << "}" << ::std::endl;
}

void Instance::write_axiom_dependency_graph
(::std::ostream& s, const index_graph& g, const char* label) const
{
  s << "digraph DG {" << ::std::endl;
  s << "label=\"" << label << "\";" << ::std::endl;
  s << "node [width=0,height=0];" << ::std::endl;

  for (index_type k = 0; k < g.size(); k++)
    if ((g.out_degree(k) > 0) || (g.in_degree(k) > 0)) {
      s << "N" << k << " [shape=ellipse,";
      if (g.out_degree(k) > 0) {
	s << "style=filled,";
      }
      if (g.node_label(k) != no_such_index) {
	assert(g.node_label(k) < n_atoms());
	s << "label=\""
	  << atoms[g.node_label(k)].name
	  << "\"];";
      }
      else {
	s << "label=\"?\"];";
      }
      s << ::std::endl;
    }

  for (index_type i = 0; i < g.size(); i++)
    for (index_type j = 0; j < g.size(); j++)
      if (g.adjacent(i, j)) {
	s << "N" << i << " -> N" << j << " [label=\""
	  << g.edge_label(i, j)
	  << "\"];" << ::std::endl;
      }

  s << "}" << ::std::endl;
}

void Instance::write_PDDL_action(::std::ostream& s, const Action& act) const
{
  index_type n_use = 0;
  for (index_type i = 0; i < n_resources(); i++)
    if (act.use[i] > 0) n_use += 1;
  index_type n_cons = 0;
  for (index_type i = 0; i < n_resources(); i++)
    if (act.cons[i] > 0) n_cons += 1;
  s << " (:action ";
  act.name->write(s, Name::NC_INSTANCE);
  if (always_write_parameters) {
    s << ::std::endl << "  :parameters ()";
  }
  if (act.assoc && write_extra) {
    s << ::std::endl << "  :assoc \"" << act.assoc << "\"";
  }
  if (write_PDDL2) {
    if ((act.pre.length() + act.lck.length() + n_use + n_cons) > 0) {
      s << ::std::endl << "  :precondition";
      if (((act.pre.length() + act.lck.length() + n_use + n_cons) > 1) ||
	  always_write_conjunction)
	s << " (and";
      for (index_type i = 0; i < act.pre.length(); i++)
	if (!act.lck.contains(act.pre[i])) {
	  if (write_negation &&
	      (atoms[act.pre[i]].neg != no_such_index) &&
	      (atoms[act.pre[i]].neg < act.pre[i])) {
	    s << " (not (";
	    atoms[atoms[act.pre[i]].neg].name->write(s, Name::NC_INSTANCE);
	    s << "))";
	  }
	  else {
	    s << " (";
	    atoms[act.pre[i]].name->write(s, Name::NC_INSTANCE);
	    s << ")";
	  }
	}
      if (write_time) {
	for (index_type i = 0; i < act.lck.length(); i++) {
	  if (write_negation &&
	      (atoms[act.lck[i]].neg != no_such_index) &&
	      (atoms[act.lck[i]].neg < act.lck[i])) {
	    s << " (at start (not (";
	    atoms[atoms[act.lck[i]].neg].name->write(s, Name::NC_INSTANCE);
	    s << ")))";
	  }
	  else {
	    s << " (at start (";
	    atoms[act.lck[i]].name->write(s, Name::NC_INSTANCE);
	    s << "))";
	  }
	}
      }
      for (index_type i = 0; i < n_resources(); i++) {
	if (write_time) {
	  if (write_resource_constraints_at_start) {
	    if ((act.use[i] + act.cons[i]) > 0) {
	      s << " (at start (>= (";
	      resources[i].name->write(s, Name::NC_INSTANCE);
	      s << ") " << PRINT_NTYPE(act.use[i] + act.cons[i]) << "))";
	    }
	  }
	  else {
	    if ((act.use[i] + act.cons[i]) > 0) {
	      s << " (over all (>= (";
	      resources[i].name->write(s, Name::NC_INSTANCE);
	      s << ") 0))";
	    }
	  }
	}
	else {
	  if ((act.use[i] + act.cons[i]) > 0) {
	    s << " (>= (";
	    resources[i].name->write(s, Name::NC_INSTANCE);
	    s << ") " << PRINT_NTYPE(act.use[i] + act.cons[i]) << ")";
	  }
	}
      }
      if (((act.pre.length() + act.lck.length() + n_use + n_cons) > 1) ||
	  always_write_conjunction)
	s << ")";
    }
    else if (always_write_precondition) {
      if (always_write_conjunction)
	s << ::std::endl << "  :precondition (and)";
      else
	s << ::std::endl << "  :precondition ()";
    }
  }
  else {
    if (act.pre.length() > 0) {
      s << ::std::endl << "  :precondition";
      if ((act.pre.length() > 1) || always_write_conjunction)
	s << " (and";
      for (index_type i = 0; i < act.pre.length(); i++) {
	if (write_negation &&
	    (atoms[act.pre[i]].neg != no_such_index) &&
	    (atoms[act.pre[i]].neg < act.pre[i])) {
	  s << " (not (";
	  atoms[atoms[act.pre[i]].neg].name->write(s, Name::NC_INSTANCE);
	  s << "))";
	}
	else {
	  s << " (";
	  atoms[act.pre[i]].name->write(s, Name::NC_INSTANCE);
	  s << ")";
	}
      }
      if ((act.pre.length() > 1) || always_write_conjunction)
	s << ")";
    }
    else if (always_write_precondition) {
      if (always_write_conjunction)
	s << ::std::endl << "  :precondition (and)";
      else
	s << ::std::endl << "  :precondition ()";
    }
  }
  if (write_PDDL2) {
    if (((act.add.length() + act.del.length() +
	  act.lck.length() + n_use + n_cons) > 0) ||
	write_metric) {
      s << ::std::endl << "  :effect";
      if ((act.add.length() + act.del.length() +
	   act.lck.length() + n_use + n_cons +
	   (write_metric ? 1 : 0)) > 1)
	s << " (and";
      for (index_type i = 0; i < act.add.length(); i++) {
	if (write_negation &&
	    (atoms[act.add[i]].neg != no_such_index) &&
	    (atoms[act.add[i]].neg < act.add[i])) {
	  s << " (not (";
	  atoms[atoms[act.add[i]].neg].name->write(s, Name::NC_INSTANCE);
	  s << "))";
	}
	else {
	  s << " (";
	  atoms[act.add[i]].name->write(s, Name::NC_INSTANCE);
	  s << ")";
	}
      }
      for (index_type i = 0; i < act.del.length(); i++) {
	if (write_negation &&
	    (atoms[act.del[i]].neg != no_such_index) &&
	    (atoms[act.del[i]].neg < act.del[i])) {
	  s << " (";
	  atoms[atoms[act.del[i]].neg].name->write(s, Name::NC_INSTANCE);
	  s << ")";
	}
	else {
	  s << " (not (";
	  atoms[act.del[i]].name->write(s, Name::NC_INSTANCE);
	  s << "))";
	}
      }
      if (write_time) {
	for (index_type i = 0; i < act.lck.length(); i++) {
	  if (write_negation &&
	      (atoms[act.lck[i]].neg != no_such_index) &&
	      (atoms[act.lck[i]].neg < act.lck[i])) {
	    s << " (at start (";
	    atoms[atoms[act.lck[i]].neg].name->write(s, Name::NC_INSTANCE);
	    s << ")) (at end (not (";
	    atoms[atoms[act.lck[i]].neg].name->write(s, Name::NC_INSTANCE);
	    s << ")))";
	  }
	  else {
	    s << " (at start (not (";
	    atoms[act.lck[i]].name->write(s, Name::NC_INSTANCE);
	    s << "))) (at end (";
	    atoms[act.lck[i]].name->write(s, Name::NC_INSTANCE);
	    s << "))";
	  }
	}
      }
      for (index_type i = 0; i < n_resources(); i++) {
	if (write_time) {
	  if ((act.use[i] + act.cons[i]) > 0) {
	    s << " (at start (decrease (";
	    resources[i].name->write(s, Name::NC_INSTANCE);
	    s << ") " << PRINT_NTYPE(act.use[i] + act.cons[i]) << "))";
	  }
	  if (act.use[i] > 0) {
	    s << " (at end (increase (";
	    resources[i].name->write(s, Name::NC_INSTANCE);
	    s << ") " << PRINT_NTYPE(act.use[i]) << "))";
	  }
	}
	else {
	  if (act.cons[i] > 0) {
	    s << " (decrease (";
	    resources[i].name->write(s, Name::NC_INSTANCE);
	    s << ") " << PRINT_NTYPE(act.cons[i]) << ")";
	  }
	}
      }
      if (write_metric) {
	s << " (increase (_cost) " << act.cost << ")";
      }
      if ((act.add.length() + act.del.length() +
	   act.lck.length() + n_use + n_cons +
	   (write_metric ? 1 : 0)) > 1)
	s << ")";
    }
  }
  else {
    if ((act.add.length() + act.del.length()) > 0) {
      s << ::std::endl << "  :effect";
      if ((act.add.length() + act.del.length()) > 1)
	s << " (and";
      for (index_type i = 0; i < act.add.length(); i++) {
	if (write_negation &&
	    (atoms[act.add[i]].neg != no_such_index) &&
	    (atoms[act.add[i]].neg < act.add[i])) {
	  s << " (not (";
	  atoms[atoms[act.add[i]].neg].name->write(s, Name::NC_INSTANCE);
	  s << "))";
	}
	else {
	  s << " (";
	  atoms[act.add[i]].name->write(s, Name::NC_INSTANCE);
	  s << ")";
	}
      }
      for (index_type i = 0; i < act.del.length(); i++) {
	if (write_negation &&
	    (atoms[act.del[i]].neg != no_such_index) &&
	    (atoms[act.del[i]].neg < act.del[i])) {
	  s << " (";
	  atoms[atoms[act.del[i]].neg].name->write(s, Name::NC_INSTANCE);
	  s << ")";
	}
	else {
	  s << " (not (";
	  atoms[act.del[i]].name->write(s, Name::NC_INSTANCE);
	  s << "))";
	}
      }
      if ((act.add.length() + act.del.length()) > 1)
	s << ")";
    }
  }
  if (write_PDDL2 && write_time) {
    if (act.dmin == act.dmax) {
      s << ::std::endl
	<< "  :duration (= ?duration "
	<< PRINT_NTYPE(act.dur)
	<< ")";
    }
    else {
      s << ::std::endl
	<< "  :duration (and (>= ?duration "
	<< PRINT_NTYPE(act.dmin)
	<< ") (<= ?duration "
	<< PRINT_NTYPE(act.dmax)
	<< "))";
    }
  }
  s << ")" << ::std::endl;
}

void Instance::write_DKEL_invariant_item
(::std::ostream& s, const Constraint& inv, string_set& tags) const
{
  s << " (:invariant";
  if (inv.name) {
    s << " :name ";
    inv.name->write(s, Name::NC_INSTANCE);
  }
  for (index_type k = 0; k < tags.length(); k++)
    s << " :tag " << tags[k];
  s << " :set-constraint";
  if (inv.exact)
    s << " (exactly-n " << inv.lim;
  else
    s << " (at-most-n " << inv.lim;
  for (index_type i = 0; i < inv.set.length(); i++) {
    if (write_negation &&
	(atoms[inv.set[i]].neg != no_such_index) &&
	(atoms[inv.set[i]].neg < inv.set[i])) {
      s << " (not (";
      atoms[atoms[inv.set[i]].neg].name->write(s, Name::NC_INSTANCE);
      s << "))";
    }
    else {
      s << " (";
      atoms[inv.set[i]].name->write(s, Name::NC_INSTANCE);
      s << ")";
    }
  }
  s << "))" << ::std::endl;
}

void Instance::write_DKEL_irrelevant_atom_item
(::std::ostream& s, const Atom& atm, string_set& tags) const
{
  if (write_negation &&
      (atm.neg != no_such_index) &&
      (atm.neg < atm.index))
    return;
  s << " (:irrelevant";
  for (index_type k = 0; k < tags.length(); k++)
    s << " :tag " << tags[k];
  s << " :fact (";
  atm.name->write(s, Name::NC_INSTANCE);
  s << "))" << ::std::endl;
}

void Instance::write_DKEL_irrelevant_action_item
(::std::ostream& s, const Action& act, string_set& tags) const
{
  s << " (:irrelevant";
  for (index_type k = 0; k < tags.length(); k++)
    s << " :tag " << tags[k];
  s << " :action (";
  act.name->write(s, Name::NC_INSTANCE);
  s << "))" << ::std::endl;
}

void Instance::write_domain_atom_set
(::std::ostream& s, const index_set& set) const
{
  s << "(:set";
  for (index_type i = 0; i < set.length(); i++) {
    s << " (";
    atoms[set[i]].name->write(s, Name::NC_INSTANCE);
    s << ")";
  }
  s << ")" << ::std::endl;
}

void Instance::write_domain_action_set
(::std::ostream& s, const index_set& set) const
{
  s << "(:set";
  for (index_type i = 0; i < set.length(); i++) {
    s << " (";
    actions[set[i]].name->write(s, Name::NC_INSTANCE);
    s << ")";
  }
  s << ")" << ::std::endl;
}

void Instance::write_domain_action_set
(::std::ostream& s, const index_set& set, const Name* name) const
{
  s << "(:set";
  if (name) {
    s << " :name ";
    name->write(s, Name::NC_INSTANCE);
  }
  for (index_type i = 0; i < set.length(); i++) {
    s << " (";
    actions[set[i]].name->write(s, Name::NC_INSTANCE);
    s << ")";
  }
  s << ")" << ::std::endl;
}

void Instance::write_domain_init(::std::ostream& s) const
{
  if (name) {
    s << "(define (domain ";
    name->write(s, Name::NC_INSTANCE | Name::NC_DOMAIN);
    s << ")" << ::std::endl;
  }
  else {
    s << "(define (domain NONAME)" << ::std::endl;
  }

  if (always_write_requirements) {
    if (write_PDDL2) {
      s << " (:requirements :strips :durative-actions)" << ::std::endl;
    }
    else {
      s << " (:requirements :strips)" << ::std::endl;
    }
  }
}

void Instance::write_domain_declarations(::std::ostream& s) const
{
  if (n_atoms() > 0) {
    s << " (:predicates";
    for (index_type k = 0; k < n_atoms(); k++) {
      if (!write_negation ||
	  (atoms[k].neg == no_such_index) ||
	  (k < atoms[k].neg)) {
	s << " (";
	atoms[k].name->write(s, Name::NC_INSTANCE);
	s << ")";
      }
    }
    s << ")" << ::std::endl;
  }
  if (write_PDDL2 && ((n_resources() > 0) || write_metric)) {
    s << " (:functions";
    for (index_type k = 0; k < n_resources(); k++) {
      s << " (";
      resources[k].name->write(s, Name::NC_INSTANCE);
      s << ")";
    }
    if (write_metric) {
      s << " (_cost)";
    }
    s << ")" << ::std::endl;
  }
}

void Instance::write_domain_actions(::std::ostream& s) const
{
  for (index_type k = 0; k < n_actions(); k++) if (actions[k].sel) {
    write_PDDL_action(s, actions[k]);
  }
}

void Instance::write_domain_DKEL_items(::std::ostream& s) const
{
  if (!write_DKEL) return;

  string_set no_tags;
  for (index_type k = 0; k < n_invariants(); k++) {
    write_DKEL_invariant_item(s, invariants[k], no_tags);
  }

  for (index_type k = 0; k < n_atoms(); k++) if (atoms[k].irrelevant) {
    write_DKEL_irrelevant_atom_item(s, atoms[k], no_tags);
  }
}

void Instance::write_domain(::std::ostream& s) const
{
  write_domain_init(s);
  write_domain_declarations(s);
  write_domain_actions(s);
  write_domain_DKEL_items(s);
  s << ")" << ::std::endl;
}

void Instance::write_problem_init(::std::ostream& s) const
{
  bool write_init = write_metric;
  if (write_PDDL2) {
    write_init = (n_resources() > 0);
  }
  for (index_type k = 0; (k < n_atoms()) && !write_init; k++)
    if (atoms[k].init) write_init = true;
  if (write_init) {
    s << " (:init";
    for (index_type k = 0; k < n_atoms(); k++) if (atoms[k].init) {
      if (!write_negation ||
	  (atoms[k].neg == no_such_index) ||
	  (k < atoms[k].neg)) {
	s << " (";
	atoms[k].name->write(s, Name::NC_INSTANCE);
	s << ")";
      }
    }
    if (write_PDDL2) {
      for (index_type k = 0; k < n_resources(); k++) {
	s << " (= (";
	resources[k].name->write(s, Name::NC_INSTANCE);
	s << ") " << PRINT_NTYPE(resources[k].init) << ")";
      }
      if (write_metric) {
	s << " (= (_cost) 0)";
      }
    }
    s << ")" << ::std::endl;
  }
}

void Instance::write_problem_goal(::std::ostream& s) const
{
  index_type write_goal = 0;
  for (index_type k = 0; (k < n_atoms()) && (write_goal < 2); k++)
    if (atoms[k].goal) write_goal += 1;
  if (write_goal > 0) {
    s << " (:goal";
    if (write_goal > 1) s << " (and";
    for (index_type k = 0; k < n_atoms(); k++) if (atoms[k].goal) {
      if (write_negation &&
	  (atoms[k].neg != no_such_index) &&
	  (atoms[k].neg < k)) {
	s << " (not (";
	atoms[atoms[k].neg].name->write(s, Name::NC_INSTANCE);
	s << "))";
      }
      else {
	s << " (";
	atoms[k].name->write(s, Name::NC_INSTANCE);
	s << ")";
      }
    }
    if (write_goal > 1) s << ")";
    s << ")" << ::std::endl;
  }
}

void Instance::write_problem_metric(::std::ostream& s) const
{
  if (write_PDDL2 && write_metric) {
    s << " (:metric minimize (_cost))" << ::std::endl;
  }
}

void Instance::write_problem(::std::ostream& s) const
{
  if (name) {
    s << "(define (problem ";
    name->write(s, Name::NC_INSTANCE | Name::NC_PROBLEM);
    s << ")" << ::std::endl;
    s << " (:domain ";
    name->write(s, Name::NC_INSTANCE | Name::NC_DOMAIN);
    s << ")" << ::std::endl;
  }
  else {
    s << "(define (problem NONAME)" << ::std::endl;
    s << " (:domain NONAME)" << ::std::endl;
  }

  write_problem_init(s);
  write_problem_goal(s);
  write_problem_metric(s);

  s << ")" << ::std::endl;
}

void Instance::print(::std::ostream& s) const
{
  s << "atoms:" << ::std::endl;
  for (index_type k = 0; k < n_atoms(); k++)
    print_atom(s, atoms[k]);
  s << "resources:" << ::std::endl;
  for (index_type k = 0; k < n_resources(); k++)
    print_resource(s, resources[k]);
  s << "actions:" << ::std::endl;
  for (index_type k = 0; k < n_actions(); k++)
    print_action(s, actions[k]);
  s << "invariants:" << ::std::endl;
  for (index_type k = 0; k < n_invariants(); k++)
    print_invariant(s, invariants[k]);
}

void Instance::print_atom(::std::ostream& s, const Atom& atm) const
{
  s << atm.index << ". " << atm.name;
  if (atm.neg != no_such_index) {
    s << " (neg: " << atoms[atm.neg].index
      << "." << atoms[atm.neg].name
      << ")";
  }
  else {
    s << " (no neg.)";
  }
  s << ::std::endl;
  s << "  init: " << (atm.init ? 'T' : 'F');
  if (atm.init) {
    s << " (init_t = " << atm.init_t << ")";
  }
  s << ", goal: " << (atm.goal ? 'T' : 'F');
  if (atm.goal) {
    s << " (goal_t = " << atm.goal_t << ")";
  }
#ifdef SUPPORT_VOLATILE_ATOMS
  if (atm.volatile) {
    s << ", volatile";
  }
#endif
  s << ", irrel: " << (atm.irrelevant ? 'T' : 'F')
    << ::std::endl;
  if (xrf) {
    s << "  req. by:";
    for (index_type i = 0; i < atm.req_by.length(); i++)
      s << ' ' << atm.req_by[i] << '.'
	<< actions[atm.req_by[i]].name;
    s << ::std::endl;
    s << "  add by:";
    for (index_type i = 0; i < atm.add_by.length(); i++)
      s << ' ' << atm.add_by[i] << '.'
	<< actions[atm.add_by[i]].name;
    s << ::std::endl;
    s << "  del by:";
    for (index_type i = 0; i < atm.del_by.length(); i++)
      s << ' ' << atm.del_by[i] << '.'
	<< actions[atm.del_by[i]].name;
    s << ::std::endl;
  }
}

void Instance::print_resource(::std::ostream& s, const Resource& res) const
{
  s << res.index << ". " << res.name << " (";
  if (res.consumed) s << "consumed, ";
  if (res.used) s << "used, ";
  s << res.init << ")" << ::std::endl;
  s << " used by:";
  for (index_type i = 0; i < n_actions(); i++)
    if (actions[i].use[res.index] > 0)
      s << " " << i << "." << actions[i].name
	<< "=" << actions[i].use[res.index];
  s << ::std::endl << " consumed by:";
  for (index_type i = 0; i < n_actions(); i++)
    if (actions[i].cons[res.index] > 0)
      s << " " << i << "." << actions[i].name
	<< "=" << actions[i].cons[res.index];
  s << ::std::endl;
}

void Instance::print_action(::std::ostream& s, const Action& act) const
{
  s << act.index << ". " << act.name
    << (act.sel ? " (selectable)" : " (non-selectable)")
    << ":" << ::std::endl;
  s << "  pre:";
  for (index_type i = 0; i < act.pre.length(); i++) {
    s << ' ' << act.pre[i] << '.';
    if (act.pre[i] < n_atoms())
      s << atoms[act.pre[i]].name;
    else
      s << "?";
    if (act.del.contains(act.pre[i])) s << " (del)";
    if (act.lck.contains(act.pre[i])) s << " (lck)";
  }
  s << ::std::endl << "  add:";
  for (index_type i = 0; i < act.add.length(); i++) {
    s << ' ' << act.add[i] << '.';
    if (act.add[i] < n_atoms())
      s << atoms[act.add[i]].name;
    else
      s << "?";
  }
  s << ::std::endl << "  del:";
  for (index_type i = 0; i < act.del.length(); i++) {
    s << ' ' <<  act.del[i] << '.';
    if (act.del[i] < n_atoms())
      s << atoms[act.del[i]].name;
    else
      s << "?";
  }
  s << ::std::endl << "  lock:";
  for (index_type i = 0; i < act.lck.length(); i++)
    s << ' ' <<  act.lck[i] << '.' << atoms[act.lck[i]].name;
  s << ::std::endl << "  use:";
  for (index_type i = 0; i < n_resources(); i++)
    s << ' ' << i << '.' << resources[i].name << '=' << act.use[i];
  s << ::std::endl << "  cons:";
  for (index_type i = 0; i < n_resources(); i++)
    s << ' ' << i << '.' << resources[i].name << '=' << act.cons[i];
  s << ::std::endl << "  dur: " << act.dur
    << ::std::endl << "  cost: " << act.cost
    << ::std::endl;
  if (act.ncw_atms.length() > 0) {
    s << "  n.c.w. atoms:";
    for (index_type i = 0; i < act.ncw_atms.length(); i++)
      s << ' ' << act.ncw_atms[i] << '.'
	<< atoms[act.ncw_atms[i]].name;
    s << ::std::endl;
  }
}

void Instance::print_invariant(::std::ostream& s, const Constraint& inv) const
{
  s << inv.index << ". ";
  if (inv.name) {
    s << inv.name << " ";
  }
  s << "|{";
  for (index_type k = 0; k < inv.set.length(); k++) {
    if (k > 0) s << ", ";
    s << k << "=" << inv.set[k] << "." << atoms[inv.set[k]].name;
  }
  s << "}|";
  if (inv.exact) {
    s << " = ";
  }
  else {
    s << " <= ";
  }
  s << inv.lim;
  s << " (size: " << inv.set.length();
  if (inv.verified) {
    s << ", verified";
  }
  s << ")" << ::std::endl;
}

PreconditionEvaluator::PreconditionEvaluator(Instance& ins)
  : instance(ins),
    node_type(undecided_leaf),
    i_test(no_such_index),
    next(0, 0),
    prev(0),
    n_positive(0)
{
  // done
}

PreconditionEvaluator::~PreconditionEvaluator()
{
  for (index_type k = 0; k < next.length(); k++) {
    assert(next[k]);
    delete next[k];
  }
}

void PreconditionEvaluator::construct
(Instance& ins,
 PreconditionEvaluator* p,
 bool_vec& s,
 bool_vec& ua,
 index_type n_ua,
 index_type n_pos,
 bool_vec& rem_invs,
 bool_vec& rem_atoms,
 NTYPE T)
{
  assert(n_ua > 0);

  // std::cerr << "|undecided| = " << n_ua << std::endl;

  index_type best_option = no_such_index;
  NTYPE best_option_val = POS_INF;
  index_type b = ins.n_invariants();

  if (n_ua > 2) {
    for (index_type k = 0; k < ins.n_invariants(); k++) if (rem_invs[k]) {
      Instance::Constraint& c = ins.invariants[k];
      index_type n1 = 0;
      for (index_type i = 0; i < c.set.length(); i++)
	n1 += ua.count_common(ins.atoms[c.set[i]].req_by);
      index_type n = 0;
      for (index_type i = 0; i < c.set.length(); i++) {
	// first term: #undecided action that have no prec. in c
	// second term: #undecided action that have prec. c = c_i
	// should also subtract action whose prec. become true!
	n += ((n_ua - n1) + ua.count_common(ins.atoms[c.set[i]].req_by));
      }
      if (!c.exact) {
	n += (n_ua - n1);
      }
      index_type m = (c.exact ? c.set.length() : c.set.length() + 1);
      NTYPE val = R_TO_N(n, m);
      // std::cerr << "option " << k << ": test invariant ";
      // ins.print_invariant(std::cerr, c);
      // std::cerr << "value = " << val << std::endl;
      if (val < best_option_val) {
	best_option = k;
	best_option_val = val;
      }
    }
    for (index_type k = 0; k < ins.n_atoms(); k++) if (rem_atoms[k]) {
      index_type n = 0;
      s[k] = true;
      for (index_type i = 0; i < ins.n_actions(); i++) if (ua[i]) {
	if (s.contains(ins.actions[i].pre)) n += 1;
      }
      s[k] = false;
      NTYPE val = R_TO_N((n_ua - ua.count_common(ins.atoms[k].req_by)) +
			 (n_ua - n), 2);
      // std::cerr << "option " << k + b << ": test atom "
      // << ins.atoms[k].name << ", value = " << val << std::endl;
      if (val < best_option_val) {
	best_option = k + b;
	best_option_val = val;
      }
    }
  }

  // std::cerr << "best option = " << best_option << " (b = " << b << ")"
  // << ", best option value = " << best_option_val
  // << ", T*|undecided| = " << (T * n_ua)
  // << std::endl;

  if (best_option_val > (T * n_ua)) {
    if (p->n_positive > 0) {
      PreconditionEvaluator* pp = new PreconditionEvaluator(ins);
      pp->prev = p;
      pp->node_type = undecided_leaf;
      for (index_type k = 0; k < ins.n_actions(); k++)
	if (ua[k]) pp->acts.append(k);
      pp->n_positive = p->n_positive;
      p->node_type = no_test;
      p->next.set_length(1);
      p->next[0] = pp;
    }
    else {
      p->node_type = undecided_leaf;
      for (index_type k = 0; k < ins.n_actions(); k++)
	if (ua[k]) p->acts.append(k);
    }
  }

  else if (best_option < b) {
    assert(best_option < ins.n_invariants());
    assert(rem_invs[best_option]);
    assert(p->next.empty());
    Instance::Constraint& c = ins.invariants[best_option];
    bool_vec rem_invs_copy(rem_invs);
    bool_vec rem_atoms_copy(rem_atoms);
    for (index_type k = 0; k < c.set.length(); k++)
      rem_atoms_copy[c.set[k]] = false;
    bool_vec ua_copy(ua);
    for (index_type k = 0; k < c.set.length(); k++) {
      if (k > 0) ua_copy.assign_copy(ua);
      PreconditionEvaluator* pp = new PreconditionEvaluator(ins);
      pp->prev = p;
      for (index_type i = 0; i < c.set.length(); i++) if (i != k) {
	for (index_type j = 0; j < ins.atoms[c.set[i]].req_by.length(); j++)
	  ua_copy[ins.atoms[c.set[i]].req_by[j]] = false;
      }
      s[c.set[k]] = true;
      for (index_type i = 0; i < ins.n_actions(); i++) if (ua_copy[i]) {
	if (s.contains(ins.actions[i].pre)) {
	  pp->acts.append(i);
	  pp->n_positive += 1;
	  ua_copy[i] = false;
	}
      }
      index_type r_ua = ua_copy.count(true);
      // std::cerr << "for value " << k
      // << ": |remaining undecided| = " << r_ua
      // << std::endl;
      if (r_ua == 0) {
	pp->node_type = positive_leaf;
      }
      else {
	if (k > 0) rem_invs_copy.assign_copy(rem_invs);
	rem_invs_copy[best_option] = false;
	for (index_type j = 0; j < ins.n_invariants(); j++)
	  if (ins.invariants[j].set.contains(c.set[k]))
	    rem_invs_copy[j] = false;
	construct(ins, pp, s, ua_copy, r_ua, n_pos + p->n_positive,
		  rem_invs_copy, rem_atoms_copy, T);
      }
      s[c.set[k]] = false;
      p->next.append(pp);
    }
    if (!c.exact) {
      PreconditionEvaluator* pp = new PreconditionEvaluator(ins);
      pp->prev = p;
      ua_copy.assign_copy(ua);
      for (index_type i = 0; i < c.set.length(); i++)
	for (index_type j = 0; j < ins.atoms[c.set[i]].req_by.length(); j++)
	  ua_copy[ins.atoms[c.set[i]].req_by[j]] = false;
      index_type r_ua = ua_copy.count(true);
      if (r_ua == 0) {
	pp->node_type = positive_leaf;
      }
      else {
	rem_invs_copy.assign_copy(rem_invs);
	rem_invs_copy[best_option] = false;
	construct(ins, pp, s, ua_copy, r_ua, n_pos + p->n_positive,
		  rem_invs_copy, rem_atoms_copy, T);
      }
      p->next.append(pp);
    }
    p->node_type = test_invariant;
    p->i_test = best_option;
  }

  else {
    assert((best_option >= b) && ((best_option - b) < ins.n_atoms()));
    best_option -= b;
    assert(rem_atoms[best_option]);
    assert(p->next.empty());
    rem_atoms[best_option] = false;
    bool_vec ua_copy(ua);
    PreconditionEvaluator* p_true = new PreconditionEvaluator(ins);
    p_true->prev = p;
    s[best_option] = true;
    for (index_type i = 0; i < ins.n_actions(); i++) if (ua_copy[i]) {
      if (s.contains(ins.actions[i].pre)) {
	p_true->acts.append(i);
	p_true->n_positive += 1;
	ua_copy[i] = false;
      }
    }
    index_type r_ua = ua_copy.count(true);
    if (r_ua == 0) {
      p_true->node_type = positive_leaf;
    }
    else {
      bool_vec rem_invs_copy(rem_invs);
      for (index_type j = 0; j < ins.n_invariants(); j++)
	if (ins.invariants[j].set.contains(best_option))
	  rem_invs_copy[j] = false;
      construct(ins, p_true, s, ua_copy, r_ua, n_pos + p->n_positive,
		rem_invs_copy, rem_atoms, T);
    }
    s[best_option] = false;
    p->next.append(p_true);
    ua_copy.assign_copy(ua);
    PreconditionEvaluator* p_false = new PreconditionEvaluator(ins);
    p_false->prev = p;
    for (index_type j = 0; j < ins.atoms[best_option].req_by.length(); j++)
      ua_copy[ins.atoms[best_option].req_by[j]] = false;
    r_ua = ua_copy.count(true);
    if (r_ua == 0) {
      p_false->node_type = positive_leaf;
    }
    else {
      construct(ins, p_false, s, ua_copy, r_ua, n_pos + p->n_positive,
		rem_invs, rem_atoms, T);
    }
    p->next.append(p_false);
    p->node_type = test_atom;
    p->i_test = best_option;
    rem_atoms[best_option] = true;
  }

  p->n_positive += n_pos;
}

PreconditionEvaluator* PreconditionEvaluator::construct
(Instance& ins, NTYPE T)
{
  PreconditionEvaluator* root = new PreconditionEvaluator(ins);
  bool_vec ua(true, ins.n_actions());
  for (index_type k = 0; k < ins.n_actions(); k++)
    if (ins.actions[k].pre.empty()) {
      root->acts.append(k);
      root->n_positive += 1;
      ua[k] = false;
    }
  if (ua.count(true) == 0) {
    root->node_type = positive_leaf;
    return root;
  }
  bool_vec s(false, ins.n_atoms());
  bool_vec rem_invs(false, ins.n_invariants());
  for (index_type k = 0; k < ins.n_invariants(); k++)
    if ((ins.invariants[k].lim == 1) && (ins.invariants[k].set.length() > 1))
      rem_invs[k] = true;
  bool_vec rem_atoms(true, ins.n_atoms());
  construct(ins, root, s, ua, ua.count(true), 0, rem_invs, rem_atoms, T);
  return root;
}

PreconditionEvaluator* PreconditionEvaluator::node(const bool_vec& s)
{
  PreconditionEvaluator* p = this;
  while ((p->node_type != positive_leaf) && (p->node_type != undecided_leaf)) {
    if (p->node_type == no_test) {
      assert(p->next[0]);
      p = p->next[0];
    }
    else if (p->node_type == test_invariant) {
      assert(p->i_test < instance.n_invariants());
      Instance::Constraint& inv = instance.invariants[p->i_test];
      assert(inv.lim == 1);
      index_type k = 0;
      bool hit = false;
      while ((k < inv.set.length()) && !hit) {
	if (s[inv.set[k]]) {
	  assert(p->next[k]);
	  p = p->next[k];
	  hit = true;
	}
	k += 1;
      }
      if (!hit) {
	assert(!inv.exact);
	assert(p->next[inv.set.length()]);
	p = p->next[inv.set.length()];
      }
    }
    else if (p->node_type == test_atom) {
      assert(p->i_test < instance.n_atoms());
      if (s[p->i_test]) {
	assert(p->next[0]);
	p = p->next[0];
      }
      else {
	assert(p->next[1]);
	p = p->next[1];
      }
    }
    else {
      assert(0);
    }
  }
  return p;
}

index_type PreconditionEvaluator::eval
(const bool_vec& s, const bool_vec& a, index_type* app, index_type c)
{
  PreconditionEvaluator* p = node(s);
  if (p->node_type == undecided_leaf) {
    for (index_type k = 0; k < p->acts.length(); k++) if (a[p->acts[k]]) {
      bool f = true;
      for (index_type i = 0;
	   (i < instance.actions[p->acts[k]].pre.length()) && f; i++)
	if (!s[instance.actions[p->acts[k]].pre[i]]) f = false;
      if (f) {
	app[c++] = p->acts[k];
      }
    }
  }
  else {
    assert(p->node_type == positive_leaf);
    for (index_type k = 0; k < p->acts.length(); k++)
      if (a[p->acts[k]]) app[c++] = p->acts[k];
  }
  p = p->prev;
  while (p) {
    if (p->n_positive == 0) return c;
    for (index_type k = 0; k < p->acts.length(); k++)
      if (a[p->acts[k]]) app[c++] = p->acts[k];
    p = p->prev;
  }
  return c;
}

void PreconditionEvaluator::write_graph(std::ostream& s, bool root)
{
  if (root) {
    s << "digraph PreconditionEvaluator {" << std::endl;
  }

  s << "N" << this << " [shape=box,label=\"";
  if (node_type == test_invariant) {
    s << "inv #" << i_test;
  }
  else if (node_type == test_atom) {
    s << "atom #" << i_test;
  }
  else {
    s << "no test";
  }
  s << " :" << n_positive
    << " / " << acts.length()
    << "\\n" << acts
    << "\"";
  if (node_type == positive_leaf) {
    s << ",style=bold";
  }
  else if (node_type == undecided_leaf) {
    s << ",style=dashed";
  }
  s << "];" << std::endl;

  for (index_type k = 0; k < next.length(); k++) {
    assert(next[k]);
    next[k]->write_graph(s, false);
  }

  for (index_type k = 0; k < next.length(); k++) {
    s << "N" << this << " -> N" << next[k];
    if (node_type == test_invariant) {
      s << " [label=\"" << k
	<< "=" << instance.invariants[i_test].set[k] << "\"];"
	<< std::endl;
    }
    else if (node_type == test_atom) {
      if (k == 0) s << " [label=\"T\"];" << std::endl;
      else s << " [label=\"F\"];" << std::endl;
    }
    else {
      s << ";" << std::endl;
    }
  }

  if (root) {
    s << "}" << std::endl;
  }
}

END_HSPS_NAMESPACE
