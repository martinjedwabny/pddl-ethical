
#include "simplify.h"
#include "enumerators.h"
#include "pdb.h"

BEGIN_HSPS_NAMESPACE

int Simplifier::default_trace_level = 0;
index_type Simplifier::composition_limit_order = 2;
index_type Simplifier::composition_limit_factor = 2;


SASInstance* Simplifier::copy_variables(SASInstance* ins)
{
  // copy variables
  SASInstance* cpy = new SASInstance(ins->name);
  for (index_type k = 0; k < ins->n_variables(); k++) {
    SASInstance::Variable& v = cpy->new_variable(ins->variables[k].name);
    v.s_index = ins->variables[k].s_index;
    for (index_type i = 0; i < ins->variables[k].n_values(); i++)
      v.domain.append(ins->variables[k].domain[i]);
    v.default_val = ins->variables[k].default_val;
    cpy->signature.append(ins->signature[k]);
  }
  // copy initial and goal state
  cpy->init_state = ins->init_state;
  cpy->goal_state = ins->goal_state;
  // copy atom/partial state mapping, if instance has it
  if (ins->atom_map && ins->atom_vars) {
    cpy->atom_map = new partial_state[ins->n_atoms];
    cpy->atom_vars = new index_set[ins->n_atoms];
    for (index_type k = 0; k < ins->n_atoms; k++) {
      cpy->atom_map[k] = ins->atom_map[k];
      cpy->atom_vars[k] = ins->atom_vars[k];
    }
    cpy->n_atoms = ins->n_atoms;
  }
  return cpy;
}

void Simplifier::extend_atom_map(Preprocessor& prep)
{
  assert(instance->atom_map && instance->atom_vars);
  for (index_type p = 0; p < instance->n_atoms; p++) {
    for (index_type q = 0; q < instance->n_atoms; q++) {
      if ((q != p) && prep.implies(p, q)) {
	if (trace_level > 1) {
	  std::cerr << "extending map["
		    << prep.instance.atoms[p].name
		    << "] = ";
	  instance->write_partial_state(std::cerr, instance->atom_map[p]);
	  std::cerr << " with map["
		    << prep.instance.atoms[q].name
		    << "] = ";
	  instance->write_partial_state(std::cerr, instance->atom_map[q]);
	  std::cerr << std::endl;
	}
	instance->atom_map[p].insert(instance->atom_map[q]);
      }
    }
  }
}

Simplifier::Simplifier(SASInstance* ins, Statistics& s)
  : instance(ins), stats(s), trace_level(default_trace_level)
{
  // done
}

Simplifier::~Simplifier()
{
  // done
}

void Simplifier::compute_elimination_graph
(graph& determination_graph, index_set& eliminable_variables)
{
  if (trace_level > 2) {
    std::cerr << "computing elimination graph..." << std::endl;
  }
  determination_graph.init(instance->n_variables());
  eliminable_variables.clear();
  for (index_type k = 0; k < instance->n_variables(); k++) {
    index_set det;
    if (instance->determining_set(k, det)) {
      for (index_type i = 0; i < det.length(); i++)
	determination_graph.add_edge(k, det[i]);
      // check that there is no irreplaceable goal condition on the variable...
      partial_state s(instance->goal_state);
      s.undefine(k);
      partial_state x;
      instance->extend_determined(s, x);
      if (x.value_of(k) == instance->goal_state.value_of(k)) {
	eliminable_variables.insert(k);
      }
    }
  }
}

void Simplifier::write_elimination_graph(std::ostream& s)
{
  graph dg;
  index_set ev;

  compute_elimination_graph(dg, ev);

  s << "digraph DG {" << std::endl;
  s << "label=\"Variable Elimination Graph\";" << std::endl;
  s << "overlap=false;" << std::endl;
  s << "splines=true;" << std::endl;
  s << "sep=0.1;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;
  s << "edge [len=2.0];" << std::endl;
  for (index_type k = 0; k < instance->n_variables(); k++) {
    s << "V" << k << " [label=\"";
    instance->variables[k].name->write(s, Name::NC_INSTANCE);
    s << "\"";
    if (!ev.contains(k)) {
      s << ",style=filled";
    }
    s << "];" << std::endl;
  }
  for (index_type i = 0; i < instance->n_variables(); i++)
    for (index_type j = 0; j < instance->n_variables(); j++)
      if (dg.adjacent(i, j)) {
	s << "V" << i << " -> V" << j << std::endl;
      }
  s << "}" << std::endl;
}

bool Simplifier::eliminate_strictly_determined_variables()
{
  if (trace_level > 0) {
    std::cerr << "simplifier: eliminating strictly determined variables..."
	      << std::endl;
  }
  stats.start();

  bool done = false;
  bool modified = false;

  while (!done && !stats.break_signal_raised()) {
    done = true;
    for (index_type k = 0; (k < instance->n_variables()) && done; k++) {
      index_set_vec dets;
      SASInstance::determined_check_result res =
	instance->minimal_determining_sets(k, dets);
      if (res == SASInstance::determined_strict) {
	index_type r = dets.first_minimum_cardinality_set();
	assert(r < dets.length());

	partial_state_vec new_goal_state;
	instance->replace_determined_variable_in_condition
	  (k, dets[r], instance->goal_state, new_goal_state);
	if (new_goal_state.length() == 1) {
	  if (trace_level > 0) {
	    std::cerr << "simplifier: eliminating variable "
		      << instance->variables[k].name
		      << " strictly determined by ";
	    instance->write_variable_set(std::cerr, dets[r]);
	    std::cerr << std::endl;
	  }
	  bool ok = instance->replace_determined_variable(k, dets[r]);
	  if (!ok) {
	    std::cerr << "error: replacing strictly determined variable ";
	    instance->variables[k].name->write(std::cerr, Name::NC_INSTANCE);
	    std::cerr << " with ";
	    instance->write_variable_set(std::cerr, dets[r]);
	    std::cerr << " produced inconsistent/disjunctive goal state"
		      << std::endl;
	    exit(255);
	  }
	}
	else {
	  std::cerr << "info: replacing strictly determined variable ";
	  instance->variables[k].name->write(std::cerr, Name::NC_INSTANCE);
	  std::cerr << " with ";
	  instance->write_variable_set(std::cerr, dets[r]);
	  std::cerr << " may produce inconsistent/disjunctive goal - skipping"
		    << std::endl;
	}

	index_set remaining_vars;
	remaining_vars.fill(instance->n_variables());
	remaining_vars.subtract(k);
	SASInstance* new_instance = instance->reduce(remaining_vars);
	delete instance;
	instance = new_instance;
	modified = true;
	done = false; 
      }
    }
  } // end while (!done)

  stats.stop();
  return modified;
}

void Simplifier::compute_domain_equivalence
(index_type var, equivalence& eq)
{
  if (trace_level > 1) {
    std::cerr << "computing domain equivalence for ";
    instance->variables[var].name->write(std::cerr, Name::NC_INSTANCE);
    std::cerr << " (" << instance->variables[var].n_values()
	      << " values)..." << std::endl;
  }
  eq.clear();
  eq.extend(instance->variables[var].n_values());
  graph dtg(instance->variables[var].n_values());
  compute_free_domain_transition_graph(var, dtg);
  dtg.strongly_connected_components();
  for (index_type i = 0; i < instance->variables[var].n_values(); i++)
    for (index_type j = i + 1; j < instance->variables[var].n_values(); j++)
      if ((dtg.component(i) == dtg.component(j))) {
	if (trace_level > 1) {
	  std::cerr << "identifying ";
	  instance->variables[var].domain[i]->write(std::cerr, Name::NC_INSTANCE);
	  std::cerr << " with ";
	  instance->variables[var].domain[j]->write(std::cerr, Name::NC_INSTANCE);
	  std::cerr << " in domain of ";
	  instance->variables[var].name->write(std::cerr, Name::NC_INSTANCE);
	  std::cerr << std::endl;
	}
	eq.merge(i, j);
      }
  if (trace_level > 2) {
    std::cerr << "domain equivalence graph for ";
    instance->variables[var].name->write(std::cerr, Name::NC_INSTANCE);
    std::cerr << ":" << std::endl;
    dtg.write_component_labeled_digraph(std::cerr, "DEQG");
  }
}

partial_state Simplifier::reduce_condition
(const partial_state& c, index_vec& var_map, index_matrix& val_map)
{
  partial_state rc;
  for (index_type k = 0; k < c.length(); k++) {
    index_type var = c[k].first;
    index_type val = c[k].second;
    if (var_map[var] != no_such_index)
      rc.assign(var_map[var], val_map[var][val]);
  }
  return rc;
}

bool Simplifier::reduce_variable_domains()
{
  if (trace_level > 0) {
    std::cerr << "simplifier: reducing variable domains..." << std::endl;
  }
  lvector<equivalence> eq;
  for (index_type k = 0; k < instance->n_variables(); k++)
    eq.append(equivalence(instance->signature[k]));
  index_matrix map;
  map.set_size(instance->n_variables(), instance->n_variables());
  for (index_type k = 0; k < instance->n_variables(); k++) {
    compute_domain_equivalence(k, eq[k]);
    eq[k].make_map(map[k]);
  }
  index_vec var_map(no_such_index, instance->n_variables());

  bool red = false;
  for (index_type k = 0; k < instance->n_variables(); k++)
    if (eq[k].n_classes() < instance->variables[k].n_values()) red = true;
  if (!red) return false;

  SASInstance* new_instance = new SASInstance(instance->name);
  for (index_type k = 0; k < instance->n_variables(); k++) {
    // copy the variable unchanged
    if (eq[k].n_classes() == instance->variables[k].n_values()) {
      if (trace_level > 0) {
	std::cerr << "simplifier: no reduction in domain of variable ";
	instance->variables[k].name->write(std::cerr, Name::NC_INSTANCE);
	std::cerr << "..." << std::endl;
      }
      SASInstance::Variable& v =
	new_instance->new_variable(instance->variables[k].name);
      v.s_index = instance->variables[k].s_index;
      v.domain = instance->variables[k].domain;
      v.default_val = instance->variables[k].default_val;
      var_map[k] = v.index;
      new_instance->signature.append(v.n_values());
    }
    // create a variable with eq. classes as values
    else if (eq[k].n_classes() > 1) {
      if (trace_level > 0) {
	std::cerr << "simplifier: reducing domain of variable ";
	instance->variables[k].name->write(std::cerr, Name::NC_INSTANCE);
	std::cerr << " to " << eq[k].n_classes() << " values..."
		  << std::endl;
      }
      SASInstance::Variable& v =
	new_instance->new_variable(instance->variables[k].name);
      for (index_type i = 0; i < eq[k].n_classes(); i++)
	v.domain.append(new ConcatenatedName());
      for (index_type i = 0; i < instance->variables[k].n_values(); i++) {
	index_type d = map[k][i];
	assert(d < v.domain.length());
	((ConcatenatedName*)v.domain[d])->
	  append(instance->variables[k].domain[i]);
      }
      if (instance->variables[k].default_val != no_such_index) {
	v.default_val = map[k][instance->variables[k].default_val];
	assert(v.default_val != no_such_index);
      }
      var_map[k] = v.index;
      new_instance->signature.append(v.n_values());
    }
    // variable has only one eq. class, remove it
    else {
      if (trace_level > 0) {
	std::cerr << "simplifier: domain of variable ";
	instance->variables[k].name->write(std::cerr, Name::NC_INSTANCE);
	std::cerr << " contains only a single value!"
		  << std::endl;
      }
      var_map[k] = no_such_index;
    }
  }

  for (index_type k = 0; k < instance->n_actions(); k++) {
    partial_state pre =
      reduce_condition(instance->actions[k].pre, var_map, map);
    partial_state post =
      reduce_condition(instance->actions[k].post, var_map, map);
    partial_state prv =
      reduce_condition(instance->actions[k].prv, var_map, map);
    index_type i = new_instance->find_action_with_cond(pre, prv, post);
    if (i == no_such_index) {
      SASInstance::Action& a = new_instance->
	new_action(new ConcatenatedName(instance->actions[k].name));
      a.s_index = instance->actions[k].s_index;
      a.pre = pre;
      a.post = post;
      a.prv = prv;
      a.cost = instance->actions[k].cost;
    }
    else {
      SASInstance::Action& a = new_instance->actions[i];
      ((ConcatenatedName*)a.name)->append(instance->actions[k].name);
      a.s_index.insert(instance->actions[k].s_index);
      a.cost = MIN(a.cost, instance->actions[k].cost);
    }
  }

  new_instance->init_state =
    reduce_condition(instance->init_state, var_map, map);
  new_instance->goal_state =
    reduce_condition(instance->goal_state, var_map, map);

  new_instance->cross_reference();
  new_instance->compute_graphs();
  delete instance;
  instance = new_instance;

  return true;
}

void Simplifier::mark
(index_type i,
 index_type j,
 graph& neq,
 std::map<index_pair, pair_set>& pending)
{
  neq.add_undirected_edge(i, j);
  std::map<index_pair, pair_set>::iterator p = pending.find(index_pair(i, j));
  if (p != pending.end()) {
    pair_set l(p->second);
    pending.erase(index_pair(i, j));
    for (index_type k = 0; k < l.length(); k++)
      mark(l[k].first, l[k].second, neq, pending);
  }
}

bool Simplifier::compute_state_equivalence
(const index_set& vars, index_type limit, equivalence& eq)
{
  if (trace_level > 1) {
    std::cerr << "computing state equivalence over ";
    instance->write_variable_set(std::cerr, vars);
    std::cerr << "..." << std::endl;
  }

  CompleteStatePerfectHashFunction csa(*instance, vars);
  if (csa.overflow()) {
    if (trace_level > 1) {
      std::cerr << "minimization failed: component too large to analyse"
		<< std::endl;
    }
    return false;
  }
  if (csa.n_values() > limit) {
    if (trace_level > 1) {
      std::cerr << "minimization failed: component size " << csa.n_values()
		<< " exceeds limit " << limit
		<< std::endl;
    }
    return false;
  }

  if (trace_level > 2) {
    StateAbstraction sa(*instance, vars);
    sa.write_graph(std::cerr, instance->goal_state);
  }

  eq.reset(csa.n_values());

  index_set av;
  for (index_type k = 0; k < instance->n_actions(); k++)
    if (instance->actions[k].pre.defines_any(vars) ||
	instance->actions[k].post.defines_any(vars) ||
	instance->actions[k].prv.defines_any(vars))
      av.insert(k);

  graph neq(csa.n_values());
  std::map<index_pair, pair_set> pending;

  // mark pairs of goal/non-goal states as non-equivalent
  if (instance->goal_state.defines_any(vars)) {
    PartialStateEnumerator eg(vars, instance->signature, instance->goal_state);
    bool more_g = eg.first();
    while (more_g) {
      PartialStateEnumerator e(vars, instance->signature);
      bool more = e.first();
      while (more) {
	if (!e.current_state().implies_restricted(instance->goal_state,vars)) {
	  neq.add_undirected_edge
	    (csa.index(eg.current_state()), csa.index(e.current_state()));
	}
	more = e.next();
      }
      more_g = eg.next();
    }
  }

  // main loop computing non-equivalence relation
  for (index_type i = 0; i < csa.n_values(); i++) {
    partial_state si;
    csa.state(i, si);
    for (index_type j = i + 1; j < csa.n_values(); j++) {
      partial_state sj;
      csa.state(j, sj);
      bool sep_ij = false;
#ifdef TRACE_PRINT_LOTS
      std::cerr << "checking states ";
      instance->write_partial_state(std::cerr, si);
      std::cerr << " and ";
      instance->write_partial_state(std::cerr, sj);
      std::cerr << std::endl;
#endif
      pair_set new_p;
      for (index_type k = 0; (k < av.length()) && !sep_ij; k++) {
	if (si.implies_restricted(instance->actions[av[k]].pre, vars) &&
	    si.implies_restricted(instance->actions[av[k]].prv, vars)) {
	  if (sj.implies_restricted(instance->actions[av[k]].pre, vars) &&
	      sj.implies_restricted(instance->actions[av[k]].prv, vars)) {
	    partial_state s_si(si);
	    s_si.assign_restricted(instance->actions[av[k]].post, vars);
	    index_type i_ssi = csa.index(s_si);
	    partial_state s_sj(sj);
	    s_sj.assign_restricted(instance->actions[av[k]].post, vars);
	    index_type i_ssj = csa.index(s_sj);
	    if (neq.bi_adjacent(i_ssi, i_ssj)) {
	      sep_ij = true;
#ifdef TRACE_PRINT_LOTS
	      std::cerr << "equivalence failed: applying "
			<< instance->actions[av[k]].name
			<< " results in ";
	      instance->write_partial_state(std::cerr, s_si);
	      std::cerr << " resp. ";
	      instance->write_partial_state(std::cerr, s_sj);
	      std::cerr << " which are separated"
			<< std::endl;
#endif
	    }
	    else if (i_ssi != i_ssj) {
	      new_p.insert(index_pair(i_ssi, i_ssj));
	    }
	  }
	  else {
	    sep_ij = true;
#ifdef TRACE_PRINT_LOTS
	    std::cerr << "equivalence failed: action "
		      << instance->actions[av[k]].name
		      << " is applicable in first but not in second"
		      << std::endl;
#endif
	  }
	}
	else if (sj.implies_restricted(instance->actions[av[k]].pre, vars) &&
		 sj.implies_restricted(instance->actions[av[k]].prv, vars)) {
	  sep_ij = true;
#ifdef TRACE_PRINT_LOTS
	  std::cerr << "equivalence failed: action "
		    << instance->actions[av[k]].name
		    << " is applicable in second but not in first"
		    << std::endl;
#endif
	}
      }
      if (sep_ij) {
	mark(i, j, neq, pending);
      }
      else {
	pending[index_pair(i, j)] = new_p;
      }
    }
  }

  // convert non-equivalence relation to an equivalence relation
  neq.complement();
  neq.induced_partitioning(eq);
  index_set ce;
  eq.canonical_elements(ce);
  assert(ce.length() == eq.n_classes());

  if (trace_level > 1) {
    std::cerr << "computed state equivalence over ";
    instance->write_variable_set(std::cerr, vars);
    std::cerr << ": " << ce.length() << " / " << csa.n_values()
	      << std::endl;
    if (ce.length() < csa.n_values()) {
      for (index_type k = 0; k < ce.length(); k++) {
	index_set e;
	eq.class_elements(ce[k], e);
	if (e.length() > 1) {
	  // std::cerr << "[" << ce[k] << "] = " << e << std::endl;
	  for (index_type i = 0; i < e.length(); i++) {
	    if (i > 0) std::cerr << " EQ ";
	    partial_state si;
	    csa.state(e[i], si);
	    instance->write_partial_state(std::cerr, si);
	  }
	  std::cerr << std::endl;
	}
      }
    }
  }

  return (eq.n_classes() < csa.n_values());
}

void Simplifier::canonize_action
(SASInstance::Action& act,
 const partial_state& cs_pre,
 const partial_state& cs_post)
{
  if (trace_level > 2) {
    std::cerr << "canonizing action " << act.name << ":" << std::endl;
    std::cerr << "pre-condition: ";
    instance->write_partial_state(std::cerr, act.pre);
    std::cerr << std::endl << "prevail-condition: ";
    instance->write_partial_state(std::cerr, act.prv);
    std::cerr << std::endl << "post-condition: ";
    instance->write_partial_state(std::cerr, act.post);
    std::cerr << std::endl << "cs_pre: ";
    instance->write_partial_state(std::cerr, cs_pre);
    std::cerr << std::endl << "cs_post: ";
    instance->write_partial_state(std::cerr, cs_post);
    std::cerr << std::endl;
  }
  for (index_type i = 0; i < cs_post.length(); i++) {
    index_type v_pre = cs_pre.value_of(cs_post[i].first);
    if (v_pre != cs_post[i].second) {
      act.pre.assign(cs_post[i].first, v_pre);
      act.post.assign(cs_post[i].first, cs_post[i].second);
      act.prv.undefine(cs_post[i].first);
    }
    else {
      act.prv.assign(cs_post[i].first, v_pre);
      act.pre.undefine(cs_post[i].first);
      act.post.undefine(cs_post[i].first);
    }
  }
  if (trace_level > 2) {
    std::cerr << "action " << act.name << " after canonization:" << std::endl;
    std::cerr << "pre-condition: ";
    instance->write_partial_state(std::cerr, act.pre);
    std::cerr << std::endl << "prevail-condition: ";
    instance->write_partial_state(std::cerr, act.prv);
    std::cerr << std::endl << "post-condition: ";
    instance->write_partial_state(std::cerr, act.post);
    std::cerr << std::endl;
  }
}

bool Simplifier::minimize
(const index_set& vars, index_type limit)
{
  if (trace_level > 1) {
    std::cerr << "minimizing over ";
    instance->write_variable_set(std::cerr, vars);
    std::cerr << "..." << std::endl;
  }

  equivalence eq;
  bool ok = compute_state_equivalence(vars, limit, eq);
  if (!ok) return false;

  CompleteStatePerfectHashFunction csa(*instance, vars);
  assert(!csa.overflow());
  assert(csa.n_values() <= limit);

  bool modified = false;

  partial_state s_init(instance->init_state, vars);
  index_type i_init = csa.index(s_init);
  index_type c_init = eq.canonical(i_init);
  if (i_init != c_init) {
    partial_state cs_init;
    csa.state(c_init, cs_init);
    if (trace_level > 1) {
      std::cerr << "initial state ";
      instance->write_partial_state(std::cerr, s_init);
      std::cerr << " is non-canonical: replace by ";
      instance->write_partial_state(std::cerr, cs_init);
      std::cerr << std::endl;
    }
    instance->init_state.assign_restricted(cs_init, vars);
  }
  else if (trace_level > 1) {
    std::cerr << "initial state ";
    instance->write_partial_state(std::cerr, s_init);
    std::cerr << " is canonical"
	      << std::endl;
  }

  for (index_type k = 0; k < instance->n_actions(); k++)
    if (instance->actions[k].post.defines_any(vars)) {
      partial_state s_pre(instance->actions[k].pre);
      s_pre.insert(instance->actions[k].prv);
      s_pre.restrict_to(vars);
      PartialStateEnumerator e(vars, instance->signature, s_pre);
      bool more = e.first();
      assert(more);
      bool first = true;
      SASInstance::Action act(instance->actions[k]);
      while (more) {
	index_type i_pre = csa.index(e.current_state());
	index_type c_pre = eq.canonical(i_pre);
	partial_state cs_pre;
	if (c_pre != i_pre) {
	  csa.state(c_pre, cs_pre);
	  if (!cs_pre.implies(s_pre)) {
	    std::cerr << "error: action " << instance->actions[k].name
		      << " is applicable in state #" << i_pre << ":";
	    instance->write_partial_state(std::cerr, e.current_state());
	    std::cerr << " (extension of ";
	    instance->write_partial_state(std::cerr, s_pre);
	    std::cerr << " but not in equivalent state #" << c_pre << ":";
	    instance->write_partial_state(std::cerr, cs_pre);
	    std::cerr << std::endl;
	    exit(255);
	  }
	}
	else {
	  cs_pre = e.current_state();
	}
	partial_state s_post(cs_pre);
	s_post.assign_restricted(instance->actions[k].post, vars);
	index_type i_post = csa.index(s_post);
	index_type c_post = eq.canonical(i_post);
	if (c_post != i_post) {
	  partial_state cs_post;
	  csa.state(c_post, cs_post);
	  if (trace_level > 1) {
	    std::cerr << "applying " << instance->actions[k].name << " in ";
	    instance->write_partial_state(std::cerr, cs_pre);
	    std::cerr << " results in ";
	    instance->write_partial_state(std::cerr, s_post);
	    std::cerr << " which is non-canonical: replace by ";
	    instance->write_partial_state(std::cerr, cs_post);
	    if (first) std::cerr << " (first)";
	    std::cerr << std::endl;
	  }
	  if (first) {
	    canonize_action(instance->actions[k], cs_pre, cs_post);
	  }
	  else {
	    SASInstance::Action& new_act =
	      instance->new_action(act);
	    canonize_action(new_act, cs_pre, cs_post);
	  }
	  modified = true;
	  first = false;
	}
	more = e.next();
      }
    }

  if (trace_level > 1) {
    std::cerr << "minimization over ";
    instance->write_variable_set(std::cerr, vars);
    if (modified) {
      std::cerr << ": instance changed" << std::endl;
    }
    else {
      std::cerr << ": no change" << std::endl;
    }
  }

  return modified;
}

bool Simplifier::apply_minimization(index_type limit)
{
  if (trace_level > 0) {
    std::cerr << "simplifier: applying minimization..." << std::endl;
  }
  stats.start();
  bool modified = false;

  for (index_type i = 0; i < instance->n_variables(); i++)
    for (index_type j = i + 1; j < instance->n_variables(); j++)
      if (instance->interference_graph.adjacent(i, j)) {
	index_set c;
	c.insert(i);
	c.insert(j);
	bool chg = minimize(c, index_type_max);
	if (chg) modified = true;
      }

  // index_set_vec c;
  // index_set     d;
  // if (trace_level > 1) {
  //   std::cerr << "searching for cliques..." << std::endl;
  // }
  // instance->interference_graph.all_maximal_cliques(c);
  // for (index_type k = 0; k < c.length(); k++) {
  //   bool chg = minimize(c[k], limit);
  //   if (chg) modified = true;
  // }

  return modified;
}

bool Simplifier::check_WSA(const index_set& vars, index_type limit)
{
  CompleteStatePerfectHashFunction csa(*instance, vars);
  if (csa.overflow()) {
    if (trace_level > 1) {
      std::cerr << "WSA failed: component too large to analyse" << std::endl;
    }
    return false;
  }
  if (csa.n_values() > limit) {
    if (trace_level > 1) {
      std::cerr << "WSA failed: component size " << csa.n_values()
		<< " exceeds limit " << limit
		<< std::endl;
    }
    return false;
  }

  graph fdtg(csa.n_values());
  bool_vec ex_caused(false, csa.n_values());

  ex_caused[csa.index(instance->init_state)] = true;

  if (trace_level > 2) {
    std::cerr << "Free DTG:" << std::endl;
    std::cerr << "digraph FDTG {" << std::endl;
    PartialStateEnumerator e(vars, instance->signature);
    bool more = e.first();
    while (more) {
      std::cerr << "S" << csa.index(e.current_state()) << " [label=\"";
      instance->write_partial_state(std::cerr, e.current_state());
      std::cerr << " (" << csa.index(e.current_state()) << ")\"];"
		<< std::endl;
      more = e.next();
    }
  }

  // std::cerr << "constructing free DTG (" << stats.time() << ")..."
  // << std::endl;

  for (index_type k = 0; k < instance->n_actions(); k++) {
    SASInstance::Action& act = instance->actions[k];
    if (!act.pre.defines_any_not_in(vars) &&
	!act.post.defines_any_not_in(vars) &&
	!act.prv.defines_any_not_in(vars)) {
      partial_state p(act.pre);
      p.insert(act.prv);
      if (!p.consistent()) {
	std::cerr << "assertion failure: act = " << act.name
		  << ", act.pre = " << act.pre
		  << ", act.prv = " << act.prv
		  << ", p = " << p
		  << std::endl;
      }
      assert(p.consistent());
      if (trace_level > 2) {
	std::cerr << "/* adding transitions due to " << act.name << "... */"
		  << std::endl;
	std::cerr << "/* p = ";
	instance->write_partial_state(std::cerr, p);
	std::cerr << "*/" << std::endl;
      }
      PartialStateEnumerator e(vars, instance->signature, p);
      bool more = e.first();
      while (more) {
	partial_state q(e.current_state());
	q.assign(act.post);
	index_type i = csa.index(e.current_state());
	index_type j = csa.index(q);
	if (trace_level > 2) {
	  std::cerr << "/* - from ";
	  instance->write_partial_state(std::cerr, e.current_state());
	  std::cerr << " (" << i << ") to ";
	  instance->write_partial_state(std::cerr, q);
	  std::cerr << " (" << j << ") */" << std::endl;
	}
	if (i != j) {
	  fdtg.add_edge(i, j);
	  if (trace_level > 2) {
	    std::cerr << "S" << i << " -> S" << j << " [label=\""
		      << act.name << "\"];" << std::endl;
	  }
	}
	more = e.next();
      }
    }
    if (act.post.defines_any(vars) &&
	act.post.defines_any_not_in(vars)) {
      partial_state p(act.pre);
      p.insert(act.prv);
      p.assign(act.post);
      PartialStateEnumerator e(vars, instance->signature, p);
      bool more = e.first();
      while (more) {
	index_type i = csa.index(e.current_state());
	assert(i < csa.n_values());
	ex_caused[i] = true;
	more = e.next();
      }
    }
  }

  if (trace_level > 2) {
    std::cerr << "}" << std::endl;
  }

  // std::cerr << "computing transitive closure (" << stats.time() << ")..."
  // << std::endl;

  // fdtg.transitive_closure();
  fdtg.strongly_connected_components();

  if (fdtg.n_components() == 1) {
    if (trace_level > 1) {
      std::cerr << "SA condition holds (one component)" << std::endl;
    }
    return true;
  }

  graph rev_fdtg_tree;
  fdtg.component_tree(rev_fdtg_tree);
  rev_fdtg_tree.reverse();

  // std::cerr << "checking reachability (" << stats.time() << ")..."
  // << std::endl;

  bool_vec c_erv(true, fdtg.n_components());
  bool any_erv = false;

  for (index_type k = 0; k < instance->n_actions(); k++) {
    SASInstance::Action& act = instance->actions[k];
    if (act.post.defines_any_not_in(vars) &&
	(act.pre.defines_any(vars) || act.prv.defines_any(vars))) {
      partial_state p(act.pre);
      p.insert(act.prv);
      if (trace_level > 2) {
	std::cerr << "ex. req. value due to " << act.name << ": ";
	instance->write_partial_state(std::cerr, p);
	std::cerr << std::endl;
      }
      // assert(p.consistent());
      bool_vec c_p(false, fdtg.n_components());
      PartialStateEnumerator e(vars, instance->signature, p);
      bool more = e.first();
      while (more) {
	index_type i = csa.index(e.current_state());
	assert(i < csa.n_values());
	index_type c_i = fdtg.component(i);
	if (trace_level > 2) {
	  std::cerr << " - completion (" << i << ") in component " << c_i
		    << std::endl;
	}
	c_p[c_i] = true;
	more = e.next();
      }
      if (trace_level > 2) {
	std::cerr << " components of all completions: " << c_p << std::endl;
      }
      c_erv.intersect(c_p);
      any_erv = true;
      if (c_erv.count(true) == 0) {
	if (trace_level > 1) {
	  std::cerr << "WSA failed: ex. req. values have no common component"
		    << std::endl;
	}
	return false;
      }
    }
  }

  if (instance->goal_state.defines_any(vars)) {
    bool_vec c_goal(false, fdtg.n_components());
    PartialStateEnumerator e(vars, instance->signature, instance->goal_state);
    bool more = e.first();
    while (more) {
      index_type i = csa.index(e.current_state());
      assert(i < csa.n_values());
      index_type c_i = fdtg.component(i);
      c_goal[c_i] = true;
      more = e.next();
    }

    rev_fdtg_tree.reachable(c_goal);
    c_erv.intersect(c_goal);
    if (c_erv.count(true) == 0) {
      if (trace_level > 1) {
	std::cerr << "WSA failed: no reachable goal value component ("
		  << c_erv << ", " << c_goal << ")" << std::endl;
      }
      return false;
    }
  }

  bool_vec r_erv(c_erv);
  rev_fdtg_tree.reachable(r_erv);

  if (trace_level > 2) {
    std::cerr << "ex. req. values common components: " << c_erv
	      << std::endl;
    std::cerr << "components from which those are reachable: " << r_erv
	      << std::endl;
  }

  for (index_type i = 0; i < csa.n_values(); i++) if (ex_caused[i]) {
    index_type c_i = fdtg.component(i);
    if (!r_erv[c_i]) {
      if (trace_level > 1) {
	std::cerr << "WSA failed: no ex. req. value component reachable from ";
	partial_state s_i;
	csa.state(i, s_i);
	instance->write_partial_state(std::cerr, s_i);
	std::cerr << " (" << i << " in " << c_i << ")" << std::endl;
      }
      return false;
    }
  }

  return true;
}

bool Simplifier::check_WSA(index_type var)
{
  index_set vars;
  vars.assign_singleton(var);
  return check_WSA(vars, instance->variables[var].n_values() + 1);
}

bool Simplifier::apply_WSA(index_type limit, bool single_variable_only)
{
  if (trace_level > 0) {
    std::cerr << "simplifier: applying weak safe abstraction..." << std::endl;
  }
  stats.start();
  bool modified = false;

  bool_vec v_abs(false, instance->n_variables());
  for (index_type k = 0; k < instance->n_variables(); k++) {
    if (trace_level > 1) {
      std::cerr << "checking WSA for variable "
		<< instance->variables[k].name
		<< "..." << std::endl;
    }
    v_abs[k] = check_WSA(k);
    if (trace_level > 1) {
      if (v_abs[k]) {
	std::cerr << "WSA condition holds" << std::endl;
      }
      else {
	std::cerr << "WSA condition does NOT hold" << std::endl;
      }
    }
  }

  if ((v_abs.count(true) == 0) && !single_variable_only) {
    graph cg(instance->causal_graph);
    graph cg_tree;
    cg.strongly_connected_components();
    cg.component_tree(cg_tree);
    if (trace_level > 1) {
      equivalence cg_comp;
      cg.component_partitioning(cg_comp);
      index_set_graph cg_scc(cg, cg_comp);
      std::cerr << "causal graph scc tree: " << cg_scc << std::endl;
    }

    for (index_type k = 0; k < cg.n_components(); k++)
      if (cg_tree.out_degree(k) == 0) {
	index_set vs;
	cg.component_node_set(k, vs);
	if (trace_level > 1) {
	  std::cerr << "checking WSA for component ";
	  instance->write_variable_set(std::cerr, vs);
	  std::cerr << "..." << std::endl;
	}
	bool wsa_holds = check_WSA(vs, limit);
	if (wsa_holds) {
	  if (trace_level > 1) {
	    std::cerr << "WSA condition holds" << std::endl;
	  }
	  v_abs.insert(vs);
	}
	else if (trace_level > 1) {
	  std::cerr << "WSA condition does NOT hold" << std::endl;
	}
      }
  }

  if ((v_abs.count(true) > 0) && (trace_level > 0)) {
    index_set v_abs2;
    v_abs.copy_to(v_abs2);
    std::cerr << "simplifier: abstracting variables ";
    instance->write_variable_set(std::cerr, v_abs2);
    std::cerr << std::endl;
  }

  index_set v_kept;
  v_abs.complement();
  v_abs.copy_to(v_kept);

  if (v_kept.length() < instance->n_variables()) {
    if (trace_level > 0) {
      std::cerr << "simplifier: keeping variables ";
      instance->write_variable_set(std::cerr, v_kept);
      std::cerr << std::endl;
    }
    SASInstance* new_instance = instance->reduce(v_kept);
    delete instance;
    instance = new_instance;
    modified = true;
  }

  stats.stop();
  return modified;
}

bool Simplifier::remove_irrelevant_variables()
{
  if (trace_level > 0) {
    std::cerr << "simplifier: removing irrelevant variables..."
	      << std::endl;
  }
  stats.start();
  bool modified = false;

  index_set v_kept;
  for (index_type k = 0; k < instance->n_variables(); k++) {
    if (!instance->variables[k].enabling_values.empty()) {
      v_kept.insert(k);
    }
    else if (trace_level > 0) {
      std::cerr << "simplifier: variable " << instance->variables[k].name
		<< " is irrelevant (no ex. req. value)" << std::endl;
    }
  }

  if (v_kept.length() < instance->n_variables()) {
    SASInstance* new_instance = instance->reduce(v_kept);
    delete instance;
    instance = new_instance;
    modified = true;
  }

  stats.stop();
  return modified;
}

void Simplifier::compute_free_domain_transition_graph
(index_type var, graph& fdtg)
{
  fdtg.init(instance->variables[var].n_values());
  for (index_type k = 0; k < instance->n_actions(); k++)
    if ((instance->actions[k].pre.length() <= 1) &&
	(instance->actions[k].post.length() == 1) &&
	instance->actions[k].prv.empty()) {
      index_type v_pre = instance->actions[k].pre.value_of(var);
      index_type v_post = instance->actions[k].post.value_of(var);
      if (v_post != no_such_index) {
	if (v_pre != no_such_index) {
	  fdtg.add_edge(v_pre, v_post);
	}
	else {
	  for (index_type i = 0; i < instance->variables[var].n_values(); i++)
	    fdtg.add_edge(i, v_post);
	}
      }
    }
}

bool Simplifier::apply_old_WSA()
{
  if (trace_level > 0) {
    std::cerr << "removing insignificant variables..." << std::endl;
  }
  stats.start();

  bool_vec insignificant(false, instance->n_variables());
  for (index_type k = 0; k < instance->n_variables(); k++) {
    SASInstance::Variable& var = instance->variables[k];
    if (var.enabling_values.empty()) {
      if (trace_level > 0) {
	std::cerr << "variable " << var.name
		  << " is insignificant (no enabling values)"
		  << std::endl;
      }
      insignificant[k] = true;
    }
    else {
      graph dtg(var.n_values());
      compute_free_domain_transition_graph(k, dtg);
      dtg.strongly_connected_components();
      graph dtg_scc;
      dtg.component_tree(dtg_scc);
      index_type c_rv = dtg.component(var.enabling_values[0]);
      bool all_in_c = true;
      for (index_type i = 1; (i < var.enabling_values.length())
	     && all_in_c; i++)
	if (dtg.component(var.enabling_values[i]) != c_rv)
	  all_in_c = false;
      if (all_in_c) {
	bool c_from_all = true;
	for (index_type i = 0; (i < var.accidental_values.length())
	       && c_from_all; i++) {
	  index_type c_iv = dtg.component(var.accidental_values[i]);
 	  // bool_vec r(false, dtg_scc.size());
 	  // dtg_scc.reachable(c_iv, r);
 	  // if (!r[c_rv]) c_from_all = false;
	  if (!dtg_scc.reachable(c_iv, c_rv)) c_from_all = false;
	}
	if (c_from_all) {
	  if (trace_level > 0) {
	    std::cerr << "variable " << var.name << " is insignificant"
		      << " (free dtg = ";
	    dtg.write_compact(std::cerr);
	    std::cerr << ", enabling values = " << var.enabling_values
		      << ", accidental values = " << var.accidental_values
		      << ")"
		      << std::endl;
	  }
	  insignificant[k] = true;
	}
      }
    }
  }

  index_set significant;
  insignificant.complement();
  insignificant.copy_to(significant);

  bool modified = false;
  if (significant.length() < instance->n_variables()) {
    SASInstance* new_instance = instance->reduce(significant);
    delete instance;
    instance = new_instance;
    modified = true;
  }

  stats.stop();
  return modified;
}

bool Simplifier::build_composite_values
(const partial_state& s, const index_set& vars, partial_state_vec& vals,
 index_type limit)
{
  if (vals.first(s) == no_such_index) {
    if (limit != no_such_index) {
      if (vals.length() >= limit) {
	if (trace_level > 2) {
	  std::cerr << "composition of ";
	  instance->write_variable_set(std::cerr, vars);
	  std::cerr << " results in too many values: ";
	  for (index_type k = 0; k < vals.length(); k++) {
	    if (k > 0) std::cerr << ", ";
	    instance->write_partial_state(std::cerr, vals[k]);
	  }
	  std::cerr << " and ";
	  instance->write_partial_state(std::cerr, s);
	  std::cerr << " (limit is " << limit << ")" << std::endl;
	}
	return false;
      }
    }
    vals.append(s);
    for (index_type k = 0; k < instance->n_actions(); k++) {
      if (s.implies_restricted(instance->actions[k].pre, vars) &&
	  s.implies_restricted(instance->actions[k].prv, vars)) {
	partial_state res(s);
	res.assign_restricted(instance->actions[k].post, vars);
	if (!build_composite_values(res, vars, vals, limit))
	  return false;
      }
    }
  }
  return true;
}

SASInstance::Variable& Simplifier::build_composite_variable
(const index_set& vars, partial_state_vec& vals, SASInstance* new_instance)
{
  assert(vars.length() > 0);
  assert(vals.length() > 0);
  ConcatenatedName* n = new ConcatenatedName('*');
  for (index_type v = 0; v < vars.length(); v++) {
    if (vars[v] >= instance->n_variables()) {
      std::cerr << "error: variable index " << vars[v]
		<< " in " << vars << " exceeds n_variables = "
		<< instance->n_variables() << std::endl;
      exit(255);
    }
    n->append(instance->variables[vars[v]].name);
  }
  SASInstance::Variable& new_v = new_instance->new_variable(n);
  for (index_type v = 0; v < vals.length(); v++) {
    ConcatenatedName* n = new ConcatenatedName('*');
    for (index_type k = 0; k < vals[v].length(); k++) {
      n->append(instance->variables[vals[v][k].first].domain[vals[v][k].second]);
      if (vals[v][k].second < instance->variables[vals[v][k].first].s_index.length())
	new_v.s_index.insert(instance->variables[vals[v][k].first].s_index[vals[v][k].second]);
    }
    new_v.domain.append(n);
  }
  return new_v;
}

void Simplifier::build_actions3
(const SASInstance::Action& act, const index_set_vec& cvar,
 const partial_state_matrix& cval, const index_set& uvar,
 const index_set& pre_cond_cvar, const index_set& post_cond_cvar,
 index_type next_p, const index_set& prv_cond_cvar,
 const partial_state& c_cond, const partial_state& pre_cond,
 partial_state& post_cond, const partial_state& prv_cond,
 SASInstance* new_instance)
{
  if (next_p < post_cond_cvar.length()) {
    index_type next_cond_cvar = post_cond_cvar[next_p];
    assert(next_cond_cvar < cval.length());
    bool found = false;
    for (index_type k = 0; k < cval[next_cond_cvar].length(); k++)
      if (cval[next_cond_cvar][k].consistent_with(c_cond)) {
	if (pre_cond.value_of(next_cond_cvar) != k) {
	  partial_state ext_c_cond(c_cond);
	  ext_c_cond.assign(cval[next_cond_cvar][k]);
	  post_cond.assign(next_cond_cvar, k);
	  build_actions3(act, cvar, cval, uvar, pre_cond_cvar, post_cond_cvar,
			 next_p + 1, prv_cond_cvar, ext_c_cond, pre_cond,
			 post_cond, prv_cond, new_instance);
	}
	found = true; 
      }
    post_cond.undefine(next_cond_cvar);
    if (!found) {
      std::cerr << "warning: no assignment for composite variable ";
      instance->write_variable_set(std::cerr, cvar[next_cond_cvar]);
      std::cerr << " consistent with ";
      instance->write_partial_state(std::cerr, c_cond);
      std::cerr << " building action ";
      act.name->write(std::cerr, Name::NC_INSTANCE);
      std::cerr << std::endl;
    }
  }
  else {
    SASInstance::Action& b = new_instance->new_action(act.name);
    b.s_index = act.s_index;
    b.pre = pre_cond;
    b.post = post_cond;
    b.prv = prv_cond;
    b.cost = act.cost;
  }
}

void Simplifier::build_actions2
(const SASInstance::Action& act, const index_set_vec& cvar,
 const partial_state_matrix& cval, const index_set& uvar,
 const index_set& pre_cond_cvar, const index_set& post_cond_cvar,
 const index_set& prv_cond_cvar, index_type next_p,
 const partial_state& c_cond, const partial_state& pre_cond,
 partial_state& post_cond, partial_state& prv_cond,
 SASInstance* new_instance)
{
  if (next_p < prv_cond_cvar.length()) {
    index_type next_cond_cvar = prv_cond_cvar[next_p];
    assert(next_cond_cvar < cval.length());
    bool found = false;
    for (index_type k = 0; k < cval[next_cond_cvar].length(); k++)
      if (cval[next_cond_cvar][k].consistent_with(act.pre) &&
	  cval[next_cond_cvar][k].consistent_with(act.prv) &&
	  cval[next_cond_cvar][k].consistent_with(c_cond)) {
	partial_state ext_c_cond(c_cond);
	ext_c_cond.assign(cval[next_cond_cvar][k]);
	prv_cond.assign(next_cond_cvar, k);
	build_actions2(act, cvar, cval, uvar, pre_cond_cvar, post_cond_cvar,
		       prv_cond_cvar, next_p + 1, ext_c_cond, pre_cond,
		       post_cond, prv_cond, new_instance);
	found = true;
      }
    prv_cond.undefine(next_cond_cvar);
    if (!found) {
      std::cerr << "warning: no assignment for composite variable ";
      instance->write_variable_set(std::cerr, cvar[next_cond_cvar]);
      std::cerr << " consistent with ";
      instance->write_partial_state(std::cerr, act.pre);
      std::cerr << " and ";
      instance->write_partial_state(std::cerr, act.prv);
      std::cerr << " building action ";
      act.name->write(std::cerr, Name::NC_INSTANCE);
      std::cerr << std::endl;
    }
  }
  else {
    partial_state c_post_cond(c_cond);
    c_post_cond.assign(act.post);
    build_actions3(act, cvar, cval, uvar, pre_cond_cvar, post_cond_cvar, 0,
		   prv_cond_cvar, c_post_cond, pre_cond, post_cond, prv_cond,
		   new_instance);
  }
}

void Simplifier::build_actions1
(const SASInstance::Action& act, const index_set_vec& cvar,
 const partial_state_matrix& cval, const index_set& uvar,
 const index_set& pre_cond_cvar, index_type next_p,
 const index_set& post_cond_cvar, const index_set& prv_cond_cvar,
 const partial_state& c_cond, partial_state& pre_cond,
 partial_state& post_cond, partial_state& prv_cond,
 SASInstance* new_instance)
{
  if (next_p < pre_cond_cvar.length()) {
    index_type next_cond_cvar = pre_cond_cvar[next_p];
    assert(next_cond_cvar < cval.length());
    bool found = false;
    for (index_type k = 0; k < cval[next_cond_cvar].length(); k++)
      if (cval[next_cond_cvar][k].consistent_with(act.pre) &&
	  cval[next_cond_cvar][k].consistent_with(act.prv) &&
	  cval[next_cond_cvar][k].consistent_with(c_cond)) {
	partial_state ext_c_cond(c_cond);
	ext_c_cond.assign(cval[next_cond_cvar][k]);
	pre_cond.assign(next_cond_cvar, k);
	build_actions1(act, cvar, cval, uvar, pre_cond_cvar, next_p + 1,
		       post_cond_cvar, prv_cond_cvar, ext_c_cond, pre_cond,
		       post_cond, prv_cond, new_instance);
	found = true;
      }
    pre_cond.undefine(next_cond_cvar);
    if (!found) {
      std::cerr << "warning: no assignment for composite variable ";
      instance->write_variable_set(std::cerr, cvar[next_cond_cvar]);
      std::cerr << " consistent with ";
      instance->write_partial_state(std::cerr, act.pre);
      std::cerr << " and ";
      instance->write_partial_state(std::cerr, act.prv);
      std::cerr << " building action ";
      act.name->write(std::cerr, Name::NC_INSTANCE);
      std::cerr << std::endl;
    }
  }
  else {
    build_actions2(act, cvar, cval, uvar, pre_cond_cvar, post_cond_cvar,
		   prv_cond_cvar, 0, c_cond, pre_cond, post_cond, prv_cond,
		   new_instance);
  }
}

void Simplifier::build_actions
(const SASInstance::Action& act, const index_set_vec& cvar,
 const partial_state_matrix& cval, const index_set& uvar,
 SASInstance* new_instance)
{
  index_set pre_def_set;
  act.pre.defined_set(pre_def_set);
  index_set post_def_set;
  act.post.defined_set(post_def_set);
  index_set prv_def_set;
  act.prv.defined_set(prv_def_set);

  index_set pre_cond_cvar;
  index_set post_cond_cvar;
  index_set prv_cond_cvar;
  for (index_type k = 0; k < cvar.length(); k++) {
    if (cvar[k].count_common(pre_def_set) > 0) pre_cond_cvar.insert(k);
    if (cvar[k].count_common(post_def_set) > 0) post_cond_cvar.insert(k);
    if (cvar[k].count_common(prv_def_set) > 0) prv_cond_cvar.insert(k);
  }

  // some composite variables may now belong to both prevail and post
  index_set b(prv_cond_cvar);
  b.intersect(post_cond_cvar);
  prv_cond_cvar.subtract(post_cond_cvar);
  pre_cond_cvar.insert(b);

  partial_state pre_cond;
  partial_state post_cond;
  partial_state prv_cond;
  for (index_type k = 0; k < act.pre.length(); k++) {
    index_type v = uvar.first(act.pre[k].first);
    if (v != no_such_index)
      pre_cond.assign(v + cvar.length(), act.pre[k].second);
  }
  for (index_type k = 0; k < act.post.length(); k++) {
    index_type v = uvar.first(act.post[k].first);
    if (v != no_such_index)
      post_cond.assign(v + cvar.length(), act.post[k].second);
  }
  for (index_type k = 0; k < act.prv.length(); k++) {
    index_type v = uvar.first(act.prv[k].first);
    if (v != no_such_index)
      prv_cond.assign(v + cvar.length(), act.prv[k].second);
  }

  partial_state c_cond;
  build_actions1(act, cvar, cval, uvar, pre_cond_cvar, 0, post_cond_cvar,
		 prv_cond_cvar, c_cond, pre_cond, post_cond, prv_cond,
		 new_instance);
}

void Simplifier::build_condition_set1
(const partial_state& cond, const index_set_vec& cvar,
 const partial_state_matrix& cval, const index_set& uvar,
 const index_set& cond_cvar, index_type next_p,
 partial_state& new_cond, partial_state_set& cond_set)
{
  if (next_p < cond_cvar.length()) {
    index_type next_cond_cvar = cond_cvar[next_p];
    assert(next_cond_cvar < cval.length());
    bool found = false;
    for (index_type k = 0; k < cval[next_cond_cvar].length(); k++)
      if (cval[next_cond_cvar][k].consistent_with(cond)) {
	partial_state ext_cond(cond);
	ext_cond.assign(cval[next_cond_cvar][k]);
	new_cond.assign(next_cond_cvar, k);
	build_condition_set1(ext_cond, cvar, cval, uvar, cond_cvar, next_p + 1,
			     new_cond, cond_set);
	found = true;
      }
    new_cond.undefine(next_cond_cvar);
    if (!found) {
      std::cerr << "warning: no assignment for composite variable ";
      instance->write_variable_set(std::cerr, cvar[next_cond_cvar]);
      std::cerr << " consistent with ";
      instance->write_partial_state(std::cerr, cond);
      std::cerr << " building condition set" << std::endl;
    }
  }
  else {
    cond_set.insert(new_cond);
  }
}

void Simplifier::build_condition_set
(const partial_state& cond, const index_set_vec& cvar,
 const partial_state_matrix& cval, const index_set& uvar,
 partial_state_set& cond_set)
{
  index_set def_set;
  cond.defined_set(def_set);
  index_set cond_cvar;
  for (index_type k = 0; k < cvar.length(); k++) {
    if (cvar[k].count_common(def_set) > 0) cond_cvar.insert(k);
  }
  partial_state new_cond;
  for (index_type k = 0; k < cond.length(); k++) {
    index_type v = uvar.first(cond[k].first);
    if (v != no_such_index)
      new_cond.assign(v + cvar.length(), cond[k].second);
  }
  build_condition_set1(cond, cvar, cval, uvar, cond_cvar, 0,
		       new_cond, cond_set);
}

bool Simplifier::compose_variables
(const index_set_vec& sets, const index_vec& limit)
{
  if (trace_level > 0) {
    std::cerr << "simplifier: composing";
    for (index_type k = 0; k < sets.length(); k++) {
      std::cerr << " ";
      instance->write_variable_set(std::cerr, sets[k]);
      std::cerr << " (" << limit[k] << ")";
    }
    std::cerr << "..." << std::endl;
  }

  SASInstance* new_instance = new SASInstance(instance->name);
  index_set remaining;
  remaining.fill(instance->n_variables());
  partial_state EMPTYSTATE;
  partial_state_matrix vals(EMPTYSTATE, sets.length(), sets.length());

  // construct composite variables
  for (index_type k = 0; k < sets.length(); k++) {
    if (sets[k].empty()) {
      std::cerr << "error: can't make composite variable from empty set in "
		<< sets << std::endl;
      exit(255);
    }
    partial_state s(instance->init_state, sets[k]);
    if (!build_composite_values(s, sets[k], vals[k], limit[k])) {
      if (trace_level > 0) {
	std::cerr << "simplifier: construction of composite variable ";
	instance->write_variable_set(std::cerr, sets[k]);
	std::cerr << " with limit " << limit[k] << " failed"
		  << std::endl;
      }
      delete new_instance;
      return false;
    }
    SASInstance::Variable& new_v =
      build_composite_variable(sets[k], vals[k], new_instance);
    new_instance->signature.append(new_v.n_values());
    remaining.subtract(sets[k]);
    if (trace_level > 1) {
      std::cerr << "composite variable ";
      new_instance->write_variable(std::cerr, new_v);
      std::cerr << " created" << std::endl;
    }
  }

  // copy remaining variables
  for (index_type k = 0; k < remaining.length(); k++) {
    SASInstance::Variable& v =
      new_instance->new_variable(instance->variables[remaining[k]].name);
    v.s_index = instance->variables[remaining[k]].s_index;
    v.domain = instance->variables[remaining[k]].domain;
    v.default_val = instance->variables[remaining[k]].default_val;
    new_instance->signature.append(v.n_values());
    if (trace_level > 1) {
      std::cerr << "simple variable ";
      new_instance->write_variable(std::cerr, v);
      std::cerr << " created" << std::endl;
    }
  }

  // assign init state values to composite variables
  for (index_type k = 0; k < sets.length(); k++) {
    partial_state c_init(instance->init_state, sets[k]);
    index_type init_val = vals[k].first(c_init);
    if (init_val == no_such_index) {
      std::cerr << "error: no value of composite variable ";
      new_instance->variables[k].name->write(std::cerr, Name::NC_INSTANCE);
      std::cerr << " matches init state ";
      instance->write_partial_state(std::cerr, instance->init_state);
      std::cerr << std::endl;
      exit(255);
    }
    new_instance->init_state.assign(k, init_val);
  }

  // assign init state values to remaining simple variables
  for (index_type k = 0; k < remaining.length(); k++) {
    index_type init_val = instance->init_state.value_of(remaining[k]);
    if (init_val == no_such_index) {
      std::cerr << "error: simple variable ";
      instance->variables[remaining[k]].name->write(std::cerr, Name::NC_INSTANCE);
      std::cerr << " undefined in init state ";
      instance->write_partial_state(std::cerr, instance->init_state);
      std::cerr << std::endl;
      exit(255);
    }
    new_instance->init_state.assign(k + sets.length(), init_val);
  }

  // build goal condition, fail if new goal is disjunctive
  partial_state_set goal_set;
  build_condition_set(instance->goal_state, sets, vals, remaining, goal_set);
  if (goal_set.empty()) {
    std::cerr << "error: goal condition ";
    instance->write_partial_state(std::cerr, instance->goal_state);
    std::cerr << " not expressible in composed instance!" << std::endl;
    exit(255);
  }
  if (goal_set.length() > 1) {
    if (trace_level > 0) {
      std::cerr << "warning (simplifier): goal condition ";
      instance->write_partial_state(std::cerr, instance->goal_state);
      std::cerr << " disjunctive in composed instance: ";
      for (index_type k = 0; k < goal_set.length(); k++) {
	if (k > 0) std::cerr << " OR ";
	new_instance->write_partial_state(std::cerr, goal_set[k]);
      }
      std::cerr << std::endl;
    }
    delete new_instance;
    return false;
  }
  new_instance->goal_state = goal_set[0];

  // modify action conditions
  for (index_type k = 0; k < instance->n_actions(); k++)
    build_actions(instance->actions[k], sets, vals, remaining, new_instance);

  new_instance->cross_reference();
  new_instance->compute_graphs();
  delete instance;
  instance = new_instance;

  if (trace_level > 2) {
    std::cerr << "composite variable DTGs:" << std::endl;
    for (index_type k = 0; k < sets.length(); k++) {
      instance->write_domain_transition_graph
	(std::cerr, k, SASInstance::ls_action_name_set, true, false, false);
    }
  }

  return true;
}

bool Simplifier::apply_variable_composition()
{
  if (trace_level > 0) {
    std::cerr << "trying bounded composition of interfering variables..."
	      << std::endl;
  }
  stats.start();
  bool modified = false;
  bool done = false;

  while (!done) {
    done = true;
    graph cg(instance->causal_graph);
    graph cg_tree;
    cg.strongly_connected_components();
    cg.component_tree(cg_tree);
    equivalence cg_comp;
    cg.component_partitioning(cg_comp);
    index_set_graph cg_scc(cg, cg_comp);
    if (trace_level > 1) {
      std::cerr << "causal graph scc tree: " << cg_scc
		<< " (" << cg_tree << ")" << std::endl;
    }
    for (index_type k = 0; (k < cg_tree.size()) && done; k++) {
      if ((cg.component_size(k) > 1) && (cg_tree.out_degree(k) == 0)) {
	index_set_vec sets(1);
	cg.component_node_set(k, sets[0]);
	if (trace_level > 1) {
	  std::cerr << "candidate set: ";
	  instance->write_variable_set(std::cerr, sets[0]);
	  std::cerr << " (" << k << ")" << std::endl;
	}
	if (instance->goal_state.defines_all(sets[0]) ||
	    !instance->goal_state.defines_any(sets[0])) {
	  index_vec limit(0, 1);
	  for (index_type i = 0; i < sets[0].length(); i++)
	    limit[0] += instance->variables[sets[0][i]].n_values();
	  // note: with lines below commented out, composition limit is
	  // strictly linear
 	  // for (index_type i = 1; i < composition_limit_order; i++)
	  //  limit[0] *= sets[0].length();
	  // limit[0] *= composition_limit_factor;
	  if (compose_variables(sets, limit)) {
	    if (trace_level > 1) {
	      std::cerr << "- composition ok" << std::endl;
	    }
	    done = false;
	    modified = true;
	  }
	  else if (trace_level > 1) {
	    std::cerr << "- composition failed" << std::endl;
	  }
	}
	else if (trace_level > 1) {
	  std::cerr << "- candidate failed all/no goal check" << std::endl;
	}
      }
    }
  } // while (!done) ...

  stats.stop();
  return modified;
}

void Simplifier::find_linear_action_sequences
(SASInstance::Variable& var, index_type val,
 const bool_vec& erv, const index_set_vec& app,
 index_vec& aseq, index_vec& vseq, bool_vec& visited,
 ActionSequenceSet& seqs)
{
  if (vseq.first(val) != no_such_index) {
    if (trace_level > 2) {
      std::cerr << " - cycle to " << var.domain[val] << std::endl;
    }
    return;
  }
  if (erv[val] || visited[val]) {
    if (trace_level > 2) {
      std::cerr << " - arrived at " << var.domain[val]
		<< " (sequence end value)" << std::endl;
    }
    if (aseq.length() > 1) {
      if (trace_level > 2) {
	std::cerr << " - found sequence:";
	for (index_type k = 0; k < aseq.length(); k++)
	  std::cerr << " " << instance->actions[aseq[k]].name;
	std::cerr << std::endl;
      }
      partial_state s0(instance->actions[aseq[0]].pre);
      s0.insert(instance->actions[aseq[0]].prv);
      partial_state s(s0);
      bool ok = true;
      for (index_type k = 0; (k < aseq.length()) && ok; k++) {
	if (s.implies(instance->actions[aseq[k]].pre) &&
	    s.implies(instance->actions[aseq[k]].prv)) {
	  s.assign(instance->actions[aseq[k]].post);
	}
	else {
	  if (trace_level > 2) {
	    std::cerr << " - extra precondition at action "
		      << instance->actions[aseq[k]].name << " (s = ";
	    instance->write_partial_state(std::cerr, s);
	    std::cerr << ")" << std::endl;
	  }
	  ok = false;
	}
      }
      if (ok) {
	s0.undefine(var.index);
	s.undefine(var.index);
	if (s != s0) {
	  if (trace_level > 2) {
	    std::cerr << " - extra effect: s0 = ";
	    instance->write_partial_state(std::cerr, s0);
	    std::cerr << ", s = ";
	    instance->write_partial_state(std::cerr, s);
	    std::cerr << std::endl;
	  }
	  ok = false;
	}
      }
      if (ok) {
	if (trace_level > 1) {
	  std::cerr << " - accepted sequence:";
	  for (index_type k = 0; k < aseq.length(); k++)
	    std::cerr << " " << instance->actions[aseq[k]].name;
	  std::cerr << std::endl;
	}
	seqs.append(ActionSequence(aseq));
      }
    }
    if (!visited[val]) {
      visited[val] = true;
      if (trace_level > 2) {
	std::cerr << " - searching for sequences starting from "
		  << var.domain[val] << "..." << std::endl;
      }
      index_vec new_aseq(no_such_index, 0);
      index_vec new_vseq(no_such_index, 0);
      new_vseq.append(val);
      for (index_type k = 0; k < app[val].length(); k++) {
	index_type v_post =
	  instance->actions[app[val][k]].post.value_of(var.index);
	new_aseq.append(app[val][k]);
	find_linear_action_sequences(var, v_post, erv, app,
				     new_aseq, new_vseq, visited, seqs);
	new_aseq.dec_length();
      }
    }
  }
  else {
    if (trace_level > 2) {
      std::cerr << " - arrived at " << var.domain[val]
		<< " (intermediary value)" << std::endl;
    }
    for (index_type k = 0; k < app[val].length(); k++) {
      index_type v_post =
	instance->actions[app[val][k]].post.value_of(var.index);
      aseq.append(app[val][k]);
      vseq.append(val);
      find_linear_action_sequences(var, v_post, erv, app,
				   aseq, vseq, visited, seqs);
      aseq.dec_length();
      vseq.dec_length();
    }
  }
}

bool Simplifier::compose_linear_action_sequences
(SASInstance::Variable& var)
{
  if (trace_level > 1) {
    std::cerr << "composing linear action sequences in variable ";
    instance->write_variable(std::cerr, var);
    std::cerr << "..." << std::endl;
  }
  bool_vec ext_req_values(false, var.n_values());
  index_set_vec app(EMPTYSET, var.n_values());
  for (index_type k = 0; k < instance->n_actions(); k++) {
    index_type v_pre = instance->actions[k].pre.value_of(var.index);
    if (v_pre != no_such_index) {
      assert(v_pre < var.n_values());
      assert(instance->actions[k].post.value_of(var.index) != no_such_index);
      app[v_pre].insert(k);
//       if (instance->actions[k].post.length() > 1)
// 	ext_req_values[v_pre] = true;
    }
    else {
      index_type v_prv = instance->actions[k].prv.value_of(var.index);
      if (v_prv != no_such_index) {
	ext_req_values[v_prv] = true;
      }
    }
  }
  index_type v_goal = instance->goal_state.value_of(var.index);
  if (v_goal != no_such_index) {
    ext_req_values[v_goal] = true;
  }
  index_type v_init = instance->init_state.value_of(var.index);
  assert(v_init != no_such_index);
  // ext_req_values[v_init] = true;
  bool_vec visited(false, var.n_values());
  visited[v_init] = true;
  index_vec aseq(no_such_index, 0);
  index_vec vseq(no_such_index, 0);
  vseq.append(v_init);
  ActionSequenceSet seqs;
  if (trace_level > 2) {
    std::cerr << " - searching for sequences starting from "
	      << var.domain[v_init] << "..." << std::endl;
  }
  for (index_type k = 0; k < app[v_init].length(); k++) {
    index_type v_post =
      instance->actions[app[v_init][k]].post.value_of(var.index);
    aseq.append(app[v_init][k]);
    find_linear_action_sequences(var, v_post, ext_req_values, app,
				 aseq, vseq, visited, seqs);
    aseq.dec_length();
  }
  if (trace_level > 1) {
    std::cerr << seqs.length() << " linear sequences found" << std::endl;
  }

  return false;
}

// #define WEAKER_SEQUENCE_COMPOSITION
#ifdef WEAKER_SEQUENCE_COMPOSITION
bool Simplifier::try_compose_action_sequences()
{
  if (trace_level > 0) {
    std::cerr << "simplifier: composing action sequences..." << std::endl;
  }
  stats.start();
  bool modified = false;

  bool_vec rem(true, instance->n_actions());
  SASInstance* new_instance = copy_variables(instance);

  index_type k = 0;
  while (k < instance->n_actions()) {
    if (rem[k]) {
      if (instance->actions[k].post.length() > 1) {
	SASInstance::Action& ak = instance->actions[k];
	bool ok = true;
	index_set aff_vars;
	index_set post_set;
	index_set pre_set;
	if (instance->goal_state.consistent_with(ak.post) &&
	    (instance->goal_state.count_common(ak.post) > 0)) {
	  ok = false;
	  if (trace_level > 2) {
	    std::cerr << ak.name << " failed at 1" << std::endl;
	  }
	}
	if (instance->init_state.consistent_with(ak.post)) {
	  ok = false;
	  if (trace_level > 2) {
	    std::cerr << ak.name << " failed at 2" << std::endl;
	  }
	}
	if (ok) {
	  ak.post.defined_set(aff_vars);
	  if (trace_level > 1) {
	    std::cerr << "starting point: " << ak.name << ", post = ";
	    instance->write_partial_state(std::cerr, ak.post);
	    std::cerr << ", aff. vars = ";
	    instance->write_variable_set(std::cerr, aff_vars);
	    std::cerr << std::endl;
	  }
	  for (index_type i = 0; (i < instance->n_actions()) && ok; i++) {
	    if (((ak.post.count_common(instance->actions[i].pre) > 0) ||
		 (ak.post.count_common(instance->actions[i].prv) > 0)) &&
		(ak.post.consistent_with(instance->actions[i].pre) > 0) &&
		(ak.post.consistent_with(instance->actions[i].prv) > 0)) {
	      if (instance->actions[i].pre.defines_any_not_in(aff_vars)) {
		ok = false;
		if (trace_level > 1) {
		  index_set ipv;
		  instance->actions[i].pre.defined_set(ipv);
		  ipv.subtract(aff_vars);
		  std::cerr << ak.name << " failed at 3 due to "
			    << instance->actions[i].name
			    << " which also defines ";
		  instance->write_variable_set(std::cerr, ipv);
		  std::cerr << std::endl;
		}
	      }
	      else if (!ak.post.implies_restricted(instance->actions[i].pre, aff_vars)) {
		ok = false;
		if (trace_level > 1) {
		  std::cerr << ak.name << " failed at 4 due to "
			    << instance->actions[i].name
			    << std::endl;
		}
	      }
	      else {
		pre_set.insert(i);
	      }
	    }
	    else if (ak.post.count_common(instance->actions[i].prv) > 0) {
	      ok = false;
	      if (trace_level > 1) {
		std::cerr << ak.name << " failed at 5 due to "
			  << instance->actions[i].name
			  << std::endl;
	      }
	    }
	  }
	}
	if (ok) {
	  post_set.assign_singleton(k);
	  for (index_type i = k + 1; i < instance->n_actions(); i++)
	    if (instance->actions[i].post == ak.post)
	      post_set.insert(i);
	  if (trace_level > 0) {
	    std::cerr << "simplifier: found composable sequences over ";
	    instance->write_partial_state(std::cerr, ak.post);
	    std::cerr << ", post set = ";
	    instance->write_action_set(std::cerr, post_set);
	    std::cerr << ", pre set = ";
	    instance->write_action_set(std::cerr, pre_set);
	    std::cerr << std::endl;
	  }
	  for (index_type i = 0; i < post_set.length(); i++)
	    for (index_type j = 0; j < pre_set.length(); j++) {
	      SASInstance::Action& ai = instance->actions[post_set[i]];
	      SASInstance::Action& aj = instance->actions[pre_set[j]];
	      partial_state c_post = ai.post;
	      c_post.assign_restricted(aj.post, aff_vars);
	      c_post.subtract(ai.pre);
	      if (!c_post.empty()) {
		SASInstance::Action& a = new_instance->new_action
		  (new ConcatenatedName(ai.name, aj.name, '+'));
		for (index_type l = 0; l < ai.pre.length(); l++)
		  if (c_post.defines(ai.pre[l].first))
		    a.pre.insert(ai.pre[l]);
		  else
		    a.prv.insert(ai.pre[l]);
		a.prv.insert(instance->actions[post_set[i]].prv);
		a.prv.insert(instance->actions[pre_set[j]].prv);
		a.post = c_post;
	      }
	      else if (trace_level > 1) {
		std::cerr << "sequence " << instance->actions[post_set[i]].name
			  << "; " << instance->actions[pre_set[j]].name
			  << " skipped as it has no effect" << std::endl;
	      }
	    }
	  for (index_type i = 0; i < post_set.length(); i++)
	    rem[post_set[i]] = false;
	  for (index_type j = 0; j < pre_set.length(); j++)
	    rem[pre_set[j]] = false;
	  modified = true;
	}
	else {
	  SASInstance::Action& a =
	    new_instance->new_action(instance->actions[k]);
	  rem[k] = false;
	}
      }
      else {
	SASInstance::Action& a =
	  new_instance->new_action(instance->actions[k]);
	rem[k] = false;
      }
    }
    k += 1;
  }

  if (modified) {
    new_instance->cross_reference();
    new_instance->compute_graphs();
    delete instance;
    instance = new_instance;
  }
  else {
    delete new_instance;
  }

  return modified;
}

#else

bool Simplifier::apply_sequence_composition()
{
  if (trace_level > 0) {
    std::cerr << "simplifier: composing action sequences..." << std::endl;
  }
  stats.start();
  bool modified = false;

  bool_vec rem(true, instance->n_actions());
  SASInstance* new_instance = copy_variables(instance);

  index_type k = 0;
  while (k < instance->n_actions()) {
    if (rem[k]) {
      if (instance->actions[k].post.length() > 1) {
	SASInstance::Action& ak = instance->actions[k];
	bool ok = true;
	index_set aff_vars;
	index_set setA;
	index_set setB;
	if (instance->goal_state.consistent_with(ak.post) &&
	    (instance->goal_state.count_common(ak.post) > 0)) {
	  ok = false;
	  if (trace_level > 2) {
	    std::cerr << ak.name << " failed (1)" << std::endl;
	  }
	}
	if (instance->init_state.consistent_with(ak.post)) {
	  ok = false;
	  if (trace_level > 2) {
	    std::cerr << ak.name << " failed (2)" << std::endl;
	  }
	}
	if (ok) {
	  ak.post.defined_set(aff_vars);
	  if (trace_level > 1) {
	    std::cerr << "starting point: " << ak.name << ", post = ";
	    instance->write_partial_state(std::cerr, ak.post);
	    std::cerr << ", aff. vars = ";
	    instance->write_variable_set(std::cerr, aff_vars);
	    std::cerr << std::endl;
	  }
	  setA.assign_singleton(k);
	  setB.clear();
	  for (index_type i = 0; (i < instance->n_actions()) && ok; i++) {
	    if (instance->actions[i].post == ak.post)
	      setA.insert(i);
	    else if ((instance->actions[i].post.count_common(ak.post) > 0) &&
		     instance->actions[i].post.consistent_with(ak.post)) {
	      ok = false;
	      if (trace_level > 1) {
		std::cerr << ak.name << " failed (3) due to "
			  << instance->actions[i].name
			  << " with postcondition ";
		instance->write_partial_state(std::cerr,
					      instance->actions[i].post);
		std::cerr << std::endl;
	      }
	    }
	  }

	  for (index_type i = 0; (i < instance->n_actions()) && ok; i++) {
	    partial_state cpre_i(instance->actions[i].pre);
	    cpre_i.insert(instance->actions[i].prv);
	    if (cpre_i.contains(ak.post)) {
	      if (!instance->actions[i].post.consistent_with(ak.post)) {
		setB.insert(i);
	      }
	      else {
		ok = false;
		if (trace_level > 1) {
		  std::cerr << ak.name << " failed (6) due to "
			    << instance->actions[i].name
			    << " with combined precondition ";
		  instance->write_partial_state(std::cerr, cpre_i);
		  std::cerr << " and postcondition ";
		  instance->write_partial_state(std::cerr,
						instance->actions[i].post);
		  std::cerr << std::endl;
		}
	      }
	    }
	    else if (cpre_i.consistent_with(ak.post)) {
	      if (!instance->actions[i].post.consistent_with(ak.post)) {
		ok = false;
		if (trace_level > 1) {
		  std::cerr << ak.name << " failed (4) due to "
			    << instance->actions[i].name
			    << " (not in B) with combined precondition ";
		  instance->write_partial_state(std::cerr, cpre_i);
		  std::cerr << " and postcondition ";
		  instance->write_partial_state(std::cerr,
						instance->actions[i].post);
		  std::cerr << std::endl;
		}
	      }
	      for (index_type j = 0; (j < setA.length()) && ok; j++)
		if (!instance->commutative(instance->actions[i], ak)) {
		  ok = false;
		  if (trace_level > 1) {
		    std::cerr << ak.name << " failed (5) due to "
			      << instance->actions[i].name
			      << " (not in B) with combined precondition ";
		    instance->write_partial_state(std::cerr, cpre_i);
		    std::cerr << " and postcondition ";
		    instance->write_partial_state(std::cerr,
						  instance->actions[i].post);
		    std::cerr << " and " << instance->actions[setA[j]].name
			      << " (in A) with combined precondition ";
		    instance->write_partial_state(std::cerr, instance->actions[setA[j]].pre);
		    std::cerr << " and prevail condition ";
		    instance->write_partial_state(std::cerr, instance->actions[setA[j]].prv);
		    std::cerr << std::endl;
		  }
		}
	    }
	  }
	}

	if (ok) {
	  if (trace_level > 0) {
	    std::cerr << "simplifier: found composable sequences over c = ";
	    instance->write_partial_state(std::cerr, ak.post);
	    std::cerr << ", A = ";
	    instance->write_action_set(std::cerr, setA);
	    std::cerr << ", B = ";
	    instance->write_action_set(std::cerr, setB);
	    std::cerr << std::endl;
	  }
	  for (index_type i = 0; i < setA.length(); i++)
	    for (index_type j = 0; j < setB.length(); j++) {
	      SASInstance::Action& ai = instance->actions[setA[i]];
	      SASInstance::Action& aj = instance->actions[setB[j]];
	      partial_state c_post = ai.post;
	      c_post.assign(aj.post);
	      if (ai.post.consistent_with(aj.pre) &&
		  ai.post.consistent_with(aj.prv) &&
		  !c_post.contains(ai.pre)) {
		partial_state c_pre(ai.pre);
		c_pre.insert(aj.pre);
		c_pre.subtract(ai.post);
		SASInstance::Action& a = new_instance->new_action
		  (new ConcatenatedName(ai.name, aj.name, '+'));
		a.prv = c_pre;
		a.prv.intersect(c_post);
		a.pre = c_pre;
		a.pre.subtract(a.prv);
		a.post = c_post;
		a.post.subtract(a.prv);
	      }
	      else if (trace_level > 1) {
		std::cerr << "sequence " << ai.name << "; " << aj.name
			  << " skipped because it is non-executable or"
			  << " has no effect (c. post = ";
		instance->write_partial_state(std::cerr, c_post);
		std::cerr << ")" << std::endl;
	      }
	    }
	  for (index_type i = 0; i < setA.length(); i++)
	    rem[setA[i]] = false;
	  for (index_type j = 0; j < setB.length(); j++)
	    rem[setB[j]] = false;
	  modified = true;
	}
	else {
	  SASInstance::Action& a =
	    new_instance->new_action(instance->actions[k]);
	  rem[k] = false;
	}
      }
      else {
	SASInstance::Action& a =
	  new_instance->new_action(instance->actions[k]);
	rem[k] = false;
      }
    }
    k += 1;
  }

  if (modified) {
    new_instance->cross_reference();
    new_instance->compute_graphs();
#define TEST_CG_CONN_AFTER_SEQUENCE_COMPOSITION
#ifdef TEST_CG_CONN_AFTER_SEQUENCE_COMPOSITION
    if (new_instance->causal_graph.n_edges() <
	instance->causal_graph.n_edges()) {
      delete instance;
      instance = new_instance;
    }
    else {
      if (trace_level > 0) {
	std::cerr << "simplifier: sequence composition did not reduce CG connectivity" << std::endl;
      }
      delete new_instance;
      modified = false;
    }
#else
    delete instance;
    instance = new_instance;
#endif
  }
  else {
    delete new_instance;
  }

  return modified;
}

#endif

bool Simplifier::split_non_unary_actions()
{
  index_set non_unary;
  for (index_type k = 0; k < instance->n_actions(); k++)
    if (instance->actions[k].post.length() > 1)
      non_unary.insert(k);
  if (non_unary.empty()) return false;

  SASInstance* new_instance = copy_variables(instance);

  // create current-action-variable
  SASInstance::Variable& cav =
    new_instance->new_variable(new StringName("current_action"));
  for (index_type k = 0; k < non_unary.length(); k++)
    cav.domain.append(instance->actions[non_unary[k]].name);
  cav.domain.append(new StringName("none"));
  cav.default_val = non_unary.length();
  new_instance->signature.append(cav.domain.length());

  // copy unary actions
  for (index_type k = 0; k < instance->n_actions(); k++)
    if (instance->actions[k].post.length() == 1) {
      SASInstance::Action& a = new_instance->new_action(instance->actions[k]);
      a.prv.assign(cav.index, cav.default_val);
    }
  // create split actions
  for (index_type k = 0; k < non_unary.length(); k++) {
    index_type s = (instance->actions[non_unary[k]].post.length() + 2);
    ConcatenatedName* n_start = new ConcatenatedName('.');
    n_start->append(instance->actions[non_unary[k]].name);
    n_start->append(new StringName("start"));
    SASInstance::Action& a_start = new_instance->new_action(n_start);
    a_start.pre = instance->actions[non_unary[k]].pre;
    a_start.pre.assign(cav.index, cav.default_val);
    a_start.prv = instance->actions[non_unary[k]].prv;
    a_start.post.assign(cav.index, k);
    a_start.cost = instance->actions[non_unary[k]].cost / s;
    for (index_type i = 0; i < instance->actions[non_unary[k]].post.length(); i++) {
      index_type var = instance->actions[non_unary[k]].post[i].first;
      ConcatenatedName* n_var = new ConcatenatedName('.');
      n_var->append(instance->actions[non_unary[k]].name);
      n_var->append(new_instance->variables[var].name);
      SASInstance::Action& a_var = new_instance->new_action(n_var);
      index_type pre_val = instance->actions[non_unary[k]].pre.value_of(var);
      if (pre_val != no_such_index) a_var.pre.assign(var, pre_val);
      a_var.prv = instance->actions[non_unary[k]].prv;
      a_var.prv.assign(cav.index, k);
      a_var.post.assign(var, instance->actions[non_unary[k]].post[i].second);
      a_var.cost = instance->actions[non_unary[k]].cost / s;
    }
    ConcatenatedName* n_fin = new ConcatenatedName('.');
    n_fin->append(instance->actions[non_unary[k]].name);
    n_fin->append(new StringName("finish"));
    SASInstance::Action& a_fin = new_instance->new_action(n_fin);
    a_fin.pre.assign(cav.index, k);
    a_fin.prv = instance->actions[non_unary[k]].prv;
    a_fin.prv.insert(instance->actions[non_unary[k]].post);
    a_fin.post.assign(cav.index, cav.default_val);
    a_fin.cost = instance->actions[non_unary[k]].cost / s;
  }

  // copy and extend init/goal states
  new_instance->init_state = instance->init_state;
  new_instance->init_state.assign(cav.index, cav.default_val);
  new_instance->goal_state = instance->goal_state;
  new_instance->goal_state.assign(cav.index, cav.default_val);

  new_instance->cross_reference();
  new_instance->compute_graphs();
  delete instance;
  instance = new_instance;
  return true;
}

#ifdef _JUNK_

bool Simplifier::compose_interfering_variables()
{
  equivalence interference_sets;
  instance->interference_graph.induced_partitioning(interference_sets);
  index_set_vec composite_sets;
  interference_sets.classes(composite_sets);
  composite_sets.remove_sets_size_le(1);
  if (composite_sets.length() > 0) {
    compose_variables(composite_sets);
    return true;
  }
  else {
    return false;
  }
}

bool Simplifier::eliminate_determined_variables()
{
  if (trace_level > 0) {
    std::cerr << "simplifier: eliminating determined variables..."
	      << std::endl;
  }
  stats.start();

  index_set ev;
  graph dg;
  bool done = false;
  bool modified = false;

  while (!done && !stats.break_signal_raised()) {
    done = true;

    // find eliminable variables and (re)construct determination graph...
    compute_elimination_graph(dg, ev);

    // look for minimal eliminable variables...
    index_set minimal_ev;
    for (index_type k = 0; k < ev.length(); k++)
      if (dg.in_degree(ev[k]) == 0) minimal_ev.insert(ev[k]);

    // if there are minimal eliminable variables, eliminate them...
    if (minimal_ev.length() > 0) {
      if (trace_level > 0) {
	std::cerr << "simplifier: minimal eliminable variables: ";
	instance->write_variable_set(std::cerr, minimal_ev);
	std::cerr << std::endl;
      }
      for (index_type k = 0; k < minimal_ev.length(); k++) {
	index_set det;
	if (!instance->smallest_determining_set(minimal_ev[k], det)) {
	  std::cerr << "program error: variable ";
	  instance->write_variable(std::cerr,
				   instance->variables[minimal_ev[k]]);
	  std::cerr << " minimal eliminable, but no determining set found"
		    << std::endl;
	  exit(255);
	}
	assert(det.length() > 0);
	instance->replace_determined_variable(minimal_ev[k], det);
      }

      index_set remaining_vars;
      remaining_vars.fill(instance->n_variables());
      remaining_vars.subtract(minimal_ev);
      SASInstance* new_instance = instance->restrict(remaining_vars);
      // new_instance->remove_inconsistent_actions();
      delete instance;
      instance = new_instance;
      modified = true;
      done = false;
    }

    // if there are no minimal eliminable variables, look for "semi-minimal"
    // variables (whose incomming edges in the determination graph are all
    // bidirectional)...
    if (done) {
      index_set semi_minimal_ev;
      for (index_type k = 0; k < ev.length(); k++)
	if (dg.in_degree(ev[k]) == dg.bi_degree(ev[k]))
	  semi_minimal_ev.insert(ev[k]);

      if (semi_minimal_ev.length() > 0) {
	index_type chosen = semi_minimal_ev[0];
	if (trace_level > 0) {
	  std::cerr << "simplifier: semi-minimal eliminable variables: ";
	  instance->write_variable_set(std::cerr, semi_minimal_ev);
	  std::cerr << ", chosen = ";
	  instance->variables[chosen].name->write(std::cerr, Name::NC_INSTANCE);
	  std::cerr << std::endl;
	}
	index_set det;
	if (!instance->smallest_determining_set(chosen, det)) {
	  std::cerr << "program error: variable ";
	  instance->write_variable(std::cerr, instance->variables[chosen]);
	  std::cerr << " semi-minimal eliminable, but no determining set found"
		    << std::endl;
	  exit(255);
	}
	assert(det.length() > 0);
	instance->replace_determined_variable(chosen, det);

	index_set remaining_vars;
	remaining_vars.fill(instance->n_variables());
	remaining_vars.subtract(chosen);
	SASInstance* new_instance = instance->restrict(remaining_vars);
	// new_instance->remove_inconsistent_actions();
	delete instance;
	instance = new_instance;
	modified = true;
	done = false;
      }
    }

    // if this did not work either, we have to do some more analysis...
    // if (done) {
    //   equivalence interference_groups;
    //  instance->interference_graph.induced_partitioning(interference_groups);
    //   index_set_graph gdg(dg, interference_groups);
    //   index_set_graph non_singleton_gdg;
    //   gdg.subgraph_set_size_gt(non_singleton_gdg, 1);
    //   non_singleton_gdg.remove_loops();
    // }

  } // end while (!done)

  stats.stop();
  return modified;
}

#endif

END_HSPS_NAMESPACE
