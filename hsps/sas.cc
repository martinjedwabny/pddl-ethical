
#include "sas.h"
#include "enumerators.h"
#include "random.h"
#include "bfs.h"

BEGIN_HSPS_NAMESPACE


partial_state::partial_state
(const partial_state& s, const index_set& res)
  : pair_set()
{
  for (index_type k = 0; k < s.length(); k++)
    if (res.contains(s[k].first)) append(s[k]);
}

partial_state::partial_state
(const partial_state& s, const index_vec& map)
  : pair_set()
{
  for (index_type k = 0; k < s.length(); k++)
    if (map[s[k].first] != no_such_index)
      insert(index_pair(map[s[k].first], s[k].second));
}

bool partial_state::defines(index_type var) const
{
  for (index_type k = 0; k < length(); k++) {
    if ((*this)[k].first == var) return true;
    if ((*this)[k].first > var) return false;
  }
  return false;
}

bool partial_state::defines_any(const index_set& set) const
{
  for (index_type k = 0; k < set.length(); k++)
    if (defines(set[k])) return true;
  return false;
}

bool partial_state::defines_any_not_in(const index_set& set) const
{
  for (index_type k = 0; k < length(); k++)
    if (!set.contains((*this)[k].first)) return true;
  return false;
}

bool partial_state::defines_all(const index_set& set) const
{
  for (index_type k = 0; k < set.length(); k++)
    if (!defines(set[k])) return false;
  return true;
}

index_type partial_state::value_of(index_type var) const
{
  for (index_type k = 0; k < length(); k++) {
    if ((*this)[k].first == var) return (*this)[k].second;
    if ((*this)[k].first > var) return no_such_index;
  }
  return no_such_index;
}

bool partial_state::consistent() const
{
  if (empty()) return true;
  for (index_type k = 0; k < length() - 1; k++) {
    if ((*this)[k].first == (*this)[k + 1].first) return false;
  }
  return true;
}

bool partial_state::consistent(index_type var) const
{
  bool found = false;
  for (index_type k = 0; (k < length()) && ((*this)[k].first <= var); k++)
    if ((*this)[k].first == var) {
      if (found) return false;
      else found = true;
    }
  return true;
}

index_type partial_state::first_inconsistent_variable() const
{
  if (empty()) return no_such_index;
  for (index_type k = 0; k < length() - 1; k++) {
    if ((*this)[k].first == (*this)[k + 1].first)
      return (*this)[k].first;
  }
  return no_such_index;
}

index_type partial_state::first_undefined_variable
(const index_set& set) const
{
  for (index_type k = 0; k < set.length(); k++)
    if (!defines(set[k])) return set[k];
  return no_such_index;
}

void partial_state::assign(index_type var, index_type val)
{
  bool found = false;
  bool var_cons = true;
  for (index_type k = 0; (k < length()) && ((*this)[k].first <= var); k++) {
    if ((*this)[k].first == var) {
      ((*this)[k]).second = val;
      if (found) var_cons = false;
      found = true;
    }
  }
  if (!var_cons) undefine(var);
  if (!var_cons || !found) insert(index_pair(var, val));
}

void partial_state::assign(const partial_state& s)
{
// for (index_type k = 0; k < s.length(); k++)
//   assign(s[k].first, s[k].second);
  index_type sp = 0;
  index_type tp = 0;
  index_type last_assigned = no_such_index;
  while ((sp < s.length()) && (tp < length())) {
    if ((*this)[tp].first == last_assigned) {
      remove(tp);
    }
    else if ((*this)[tp].first == s[sp].first) {
      (*this)[tp].second = s[sp].second;
      last_assigned = s[sp].first;
      sp += 1;
      tp += 1;
    }
    else if ((*this)[tp].first > s[sp].first) {
      pair_vec::insert(s[sp], tp);
      sp += 1;
      tp += 1;
    }
    else {
      assert((*this)[tp].first < s[sp].first);
      tp += 1;
    }
  }
  while (sp < s.length()) {
    append(s[sp]);
    sp += 1;
  }
}

void partial_state::undefine(index_type var)
{
  index_type k = 0;
  while (k < length()) {
    if ((*this)[k].first == var)
      remove(k);
    else if ((*this)[k].first < var)
      k += 1;
    else
      return;
  }
}

void partial_state::restrict_to(const index_set& vars)
{
//   index_set defs;
//   defined_set(defs);
//   for (index_type k = 0; k < defs.length(); k++)
//     if (!vars.contains(defs[k])) undefine(defs[k]);
  index_type k = 0;
  while (k < length()) {
    if (!vars.contains((*this)[k].first))
      remove(k);
    else
      k += 1;
  }
}

void partial_state::defined_set(index_set& set) const
{
  for (index_type k = 0; k < length(); k++)
    set.insert((*this)[k].first);
}

void partial_state::value_set(index_type var, index_set& set) const
{
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k].first == var) set.insert((*this)[k].second);
}

void partial_state::conflict_set
(const partial_state& s, index_set& set) const
{
  for (index_type k = 0; k < length(); k++)
    if (s.value_of((*this)[k].first) != (*this)[k].second)
      set.insert((*this)[k].first);
  for (index_type k = 0; k < s.length(); k++)
    if (value_of(s[k].first) != s[k].second)
      set.insert(s[k].first);
}

bool partial_state::consistent_with(const partial_state& s) const
{
  for (index_type k = 0; k < length(); k++)
    for (index_type i = 0; i < s.length(); i++)
      if (s[i].first == (*this)[k].first)
	if (s[i].second != (*this)[k].second)
	  return false;
  return true;
}

bool partial_state::implies(const partial_state& s) const
{
  for (index_type k = 0; k < s.length(); k++)
    if (value_of(s[k].first) != s[k].second)
      return false;
  return true;
}

bool partial_state::implies_reduced
(const partial_state& s, const index_vec& map) const
{
  for (index_type k = 0; k < s.length(); k++) {
    index_type i = map[s[k].first];
    if (i != no_such_index)
      if (value_of(i) != s[k].second) return false;
  }
  return true;
}

bool partial_state::implies_restricted
(const partial_state& s, const index_set& res) const
{
  for (index_type k = 0; k < s.length(); k++) {
    if (res.contains(s[k].first))
      if (value_of(s[k].first) != s[k].second)
	return false;
  }
  return true;
}

bool partial_state::equals_restricted
(const partial_state& s, const index_set& res) const
{
  index_type p_this = 0; 
  index_type p_s = 0;
  index_type k = 0;
  while (k < res.length()) {
    index_type v_this = no_such_index;
    bool seeking = (p_this < length());
    while (seeking) {
      if ((*this)[p_this].first == res[k]) {
	v_this = (*this)[p_this].second;
	seeking = false;
      }
      else if ((*this)[p_this].first > res[k]) {
	seeking = false;
      }
      else {
	p_this += 1;
	if (p_this >= length())
	  seeking = false;
      }
    }
    index_type v_s = no_such_index;
    seeking = (p_s < s.length());
    while (seeking) {
      if (s[p_s].first == res[k]) {
	v_s = s[p_s].second;
	seeking = false;
      }
      else if (s[p_s].first > res[k]) {
	seeking = false;
      }
      else {
	p_s += 1;
	if (p_s >= s.length())
	  seeking = false;
      }
    }
    if (v_this != v_s) return false;
    k += 1;
  }
  return true;
}

void partial_state::assign_reduced
(const partial_state& s, const index_vec& map)
{
  clear();
  for (index_type k = 0; k < s.length(); k++) {
    index_type i = map[s[k].first];
    if (i != no_such_index)
      assign(i, s[k].second);
  }
}

void partial_state::assign_restricted
(const partial_state& s, const index_set& res)
{
  for (index_type k = 0; k < s.length(); k++) {
    if (res.contains(s[k].first))
      assign(s[k].first, s[k].second);
  }
}


const Name* SASInstance::val_nil = new StringName("none");
const Name* SASInstance::val_false = new StringName("false");
const Name* SASInstance::val_true = new StringName("true");
const Name* SASInstance::act_default_name = new StringName("A");
bool SASInstance::write_symbolic_names = true;
bool SASInstance::write_variable_sources = false;
bool SASInstance::write_variable_relevance = false;
bool SASInstance::write_action_cost = false;
bool SASInstance::write_action_sources = false;
bool SASInstance::write_info_in_domain = false;
bool SASInstance::additional_strictness_check = true;

SASInstance::SASInstance()
  : atom_map(0),
    atom_vars(0),
    n_atoms(0),
    name(0),
    variables(Variable(), 0),
    signature(0, 0),
    actions(Action(), 0),
    variable_reduce_map(no_such_index, 0),
    variable_expand_map(no_such_index, 0),
    trace_level(Instance::default_trace_level)
{
  // done
}

SASInstance::SASInstance(const Name* n)
  : atom_map(0),
    atom_vars(0),
    n_atoms(0),
    name(n),
    variables(Variable(), 0),
    signature(0, 0),
    actions(Action(), 0),
    variable_reduce_map(no_such_index, 0),
    variable_expand_map(no_such_index, 0),
    trace_level(Instance::default_trace_level)
{
  // done
}

SASInstance::SASInstance
(const Instance& ins, bool selective, bool minimal, bool safe)
  : atom_map(0),
    atom_vars(0),
    n_atoms(0),
    name(ins.name),
    variables(Variable(), 0),
    signature(0, 0),
    actions(Action(), 0),
    variable_reduce_map(no_such_index, 0),
    variable_expand_map(no_such_index, 0),
    trace_level(Instance::default_trace_level)
{
  construct(ins, selective, minimal, safe);
}

SASInstance::SASInstance
(const SASInstance& ins, const index_set& vars)
  : atom_map(0),
    atom_vars(0),
    n_atoms(0),
    name(ins.name),
    variables(Variable(), 0),
    signature(0, 0),
    actions(Action(), 0),
    variable_reduce_map(no_such_index, 0),
    variable_expand_map(no_such_index, 0),
    trace_level(Instance::default_trace_level)
{
  construct(ins, vars);
}

SASInstance::SASInstance
(const SASInstance& ins)
  : atom_map(0),
    atom_vars(0),
    n_atoms(0),
    name(ins.name),
    variables(ins.variables),
    signature(ins.signature),
    actions(ins.actions),
    variable_reduce_map(ins.variable_reduce_map),
    variable_expand_map(ins.variable_expand_map),
    trace_level(Instance::default_trace_level)
{
  if (ins.n_atoms > 0) {
    assert(ins.atom_map);
    assert(ins.atom_vars);
    n_atoms = ins.n_atoms;
    atom_map = new partial_state[n_atoms];
    atom_vars = new index_set[n_atoms];
    for (index_type k = 0; k < n_atoms; k++) {
      atom_map[k] = ins.atom_map[k];
      atom_vars[k] = ins.atom_vars[k];
    }
  }
}

SASInstance::~SASInstance()
{
  if (atom_map) delete [] atom_map;
  if (atom_vars) delete [] atom_vars;
}

SASInstance::Variable& SASInstance::new_variable()
{
  Variable& v = variables.append();
  v.index = variables.length() - 1;
  v.name = new EnumName("V", v.index);
  if (trace_level > 2) {
    std::cerr << "variable " << v.name << " created" << std::endl;
  }
  return v;
}

SASInstance::Variable& SASInstance::new_variable(const Name* name)
{
  Variable& v = variables.append();
  v.index = variables.length() - 1;
  v.name = name;
  if (trace_level > 2) {
    std::cerr << "variable " << v.index << "." << v.name << " created"
	      << std::endl;
  }
  return v;
}

SASInstance::Action& SASInstance::new_action(const Name* name)
{
  Action& a = actions.append();
  a.index = actions.length() - 1;
  a.name = name;
  if (trace_level > 2) {
    std::cerr << "action " << a.index << "." << a.name
	      << " created" << std::endl;
  }
  return a;
}

SASInstance::Action& SASInstance::new_action(const SASInstance::Action& act)
{
  Action& a = actions.append();
  a = act;
  a.index = actions.length() - 1;
  if (trace_level > 2) {
    std::cerr << "action " << a.index << "." << a.name
	      << " created" << std::endl;
  }
  return a;
}

void SASInstance::variable_names(name_vec& names) const
{
  names.clear();
  for (index_type k = 0; k < n_variables(); k++)
    names.append(variables[k].name);
}

void SASInstance::action_names(name_vec& names) const
{
  names.clear();
  for (index_type k = 0; k < n_actions(); k++)
    names.append(actions[k].name);
}

void SASInstance::select_invariants
(const Instance& ins, Instance::constraint_vec& invs)
{
  invs.clear();
  index_set usable;
  for (index_type k = 0; k < ins.n_invariants(); k++) {
    if ((ins.invariants[k].lim == 1) &&
	(ins.invariants[k].set.length() > 1))
      usable.insert(k);
  }
  if (usable.empty()) {
    std::cerr << "warning (select invariants): NO usable invariants"
	      << std::endl;
    return;
  }

  graph atom_ccg;
  ins.cochange_graph(atom_ccg);
  graph ccg(ins.n_invariants());
  for (index_type i = 0; i < ins.n_invariants(); i++)
    for (index_type j = i + 1; j < ins.n_invariants(); j++) {
      if (ins.invariants[i].set.count_common(ins.invariants[j].set) > 0)
	ccg.add_undirected_edge(i, j);
      else if (atom_ccg.adjacent(ins.invariants[i].set, ins.invariants[j].set))
	ccg.add_undirected_edge(i, j);
    }

  // std::cerr << "ccg = " << ccg << std::endl;
  index_set covered;

  while (!usable.empty()) {
    // std::cerr << "covered = " << covered << std::endl;
    // std::cerr << "usable:" << std::endl;
    // for (index_type k = 0; k < usable.length(); k++) {
    // std::cerr << " " << usable[k] << ". ";
    // ins.print_invariant(std::cerr, ins.invariants[usable[k]]);
    // }
    weighted_graph g(ccg, usable);
    for (index_type k = 0; k < usable.length(); k++) {
      index_type r = ins.invariants[usable[k]].set.count_common(covered);
      index_type n = (ins.invariants[usable[k]].set.length() - r);
      bool e = (ins.invariants[usable[k]].exact && (r == 0));
      NTYPE w = n * (e ? R_TO_N(n, ilog(n)) : R_TO_N(n, ilog(n + 1)));
      g.set_weight(k, w);
    }
    // std::cerr << "g = " << g << std::endl;
    index_set selected;
    g.apx_weighted_independent_set(selected);
    // std::cerr << "selected = " << selected << std::endl;
    index_set selected_usable(usable, selected);
    for (index_type k = 0; k < selected_usable.length(); k++) {
      Instance::Constraint& c = invs.append();
      c = ins.invariants[selected_usable[k]];
      c.set.subtract(covered);
      if (c.set.length() < ins.invariants[selected_usable[k]].set.length())
	c.exact = false;
      covered.insert(c.set);
      // std::cerr << "selected invariant: ";
      // ins.print_invariant(std::cerr, c);
    }
    usable.subtract(selected_usable);
    index_set no_longer_usable;
    for (index_type k = 0; k < usable.length(); k++) {
      index_type r = ins.invariants[usable[k]].set.count_common(covered);
      index_type n = (ins.invariants[usable[k]].set.length() - r);
      if (n < 2) {
	// std::cerr << "invariant " << usable[k] << ": ";
	// ins.print_invariant(std::cerr, ins.invariants[usable[k]]);
	// std::cerr << " is no longer usable (r = " << r << ", n = "
	// << n << ")" << std::endl;
	no_longer_usable.insert(usable[k]);
      }
    }
    // std::cerr << "no longer usable = " << no_longer_usable << std::endl;
    usable.subtract(no_longer_usable);
  }
}

void SASInstance::construct_variables
(const Instance& ins, const Instance::constraint_vec& invs)
{
  n_atoms = ins.n_atoms();
  atom_map = new partial_state[n_atoms];
  atom_vars = new index_set[n_atoms];

  index_set rem_atoms;
  rem_atoms.fill(ins.n_atoms());

  for (index_type k = 0; k < invs.length(); k++) {
    const Instance::Constraint& c = invs[k];
    assert(c.lim == 1);
    Variable& v = (c.name ? new_variable(c.name) : new_variable());
    v.s_index = c.set;
    for (index_type i = 0; i < c.set.length(); i++) {
      assert(c.set[i] < ins.n_atoms());
      v.domain.append(ins.atoms[c.set[i]].name);
      atom_map[c.set[i]].insert(index_pair(v.index, i));
      atom_vars[c.set[i]].insert(v.index);
    }
    if (!c.exact) {
      v.domain.append(val_nil);
      v.default_val = v.domain.length() - 1;
    }
    signature.append(v.n_values());
    variable_reduce_map.append(v.index);
    variable_expand_map.append(v.index);
    rem_atoms.subtract(c.set);
  }

  for (index_type k = 0; k < rem_atoms.length(); k++) {
    Variable& v = new_variable(ins.atoms[rem_atoms[k]].name);
    v.s_index.assign_singleton(rem_atoms[k]);
    v.domain.append(val_true);
    v.domain.append(val_false);
    atom_map[rem_atoms[k]].insert(index_pair(v.index, 0));
    atom_vars[rem_atoms[k]].insert(v.index);
    v.default_val = 1;
    signature.append(v.n_values());
    variable_reduce_map.append(v.index);
    variable_expand_map.append(v.index);
  }
}

SASInstance::Variable& SASInstance::binary_variable
(const Instance& ins, index_type atom)
{
  assert(atom_map_defined() && (ins.n_atoms() == n_atoms));
  if (atom >= ins.n_atoms()) {
    std::cerr << "error: can't create binary variable for atom #"
	      << atom << " - instance has only " << ins.n_atoms()
	      << " atoms" << std::endl;
    exit(255);
  }

  // check if requested variable exists...
  for (index_type k = 0; k < n_variables(); k++)
    if (variables[k].is_binary())
      if (variables[k].s_index[0] == atom)
	return variables[k];

  // if not, create new...
  Variable& v = new_variable(ins.atoms[atom].name);
  v.s_index.assign_singleton(atom);
  v.domain.append(val_true);
  v.domain.append(val_false);
  atom_map[atom].insert(index_pair(v.index, 0));
  atom_vars[atom].insert(v.index);
  v.default_val = 1;
  signature.append(v.n_values());

  // add new variable to existing actions...
  for (index_type k = 0; k < n_actions(); k++) {
    bool all_pre = true;
    bool any_pre = false;
    bool all_add = true;
    bool any_add = false;
    bool all_del = true;
    bool any_del = false;
    for (index_type i = 0; i < actions[k].s_index.length(); i++) {
      index_type j = actions[k].s_index[i];
      if (ins.actions[j].pre.contains(atom)) any_pre = true;
      else all_pre = false;
      if (ins.actions[j].add.contains(atom)) any_add = true;
      else all_add = false;
      if (ins.actions[j].del.contains(atom)) any_del = true;
      else all_del = false;
    }
    if ((all_pre != any_pre) || (all_add != any_add) || (all_del != any_del)) {
      std::cerr << "error: can't create binary variable for atom "
		<< ins.atoms[atom].name << " - action ";
      write_action(std::cerr, actions[k]);
      std::cerr << " (sources " << actions[k].s_index
		<< ") has conflicting requirements: "
		<< all_pre << " - " << any_pre << ", "
		<< all_add << " - " << any_add << ", "
		<< all_del << " - " << any_del << std::endl;
      exit(255);
    }
    if (any_add) actions[k].post.insert(index_pair(v.index, 0));
    if (any_del) actions[k].post.insert(index_pair(v.index, 1));
    if (any_pre) {
      if (any_add || any_del) actions[k].pre.insert(index_pair(v.index, 0));
      else actions[k].prv.insert(index_pair(v.index, 0));
    }
  }

  // and to init state, but not goal
  if (ins.atoms[atom].init) {
    init_state.insert(index_pair(v.index, 0));
  }
  else {
    init_state.insert(index_pair(v.index, 1));
  }

  return v;
}

bool SASInstance::construct_actions(const Instance& ins)
{
  bool ok = true;

  for (index_type k = 0; k < ins.n_actions(); k++) {
    const Instance::Action& a = ins.actions[k];

    Action& b = new_action(a.name);
    b.s_index.insert(k);
    make_partial_state(a.pre, b.pre);
    bool pc = b.pre.consistent();

    make_partial_state(a.add, b.post);
    if (pc && !b.post.consistent()) {
      std::cerr << "warning (construction): action " << a.name
		<< " has inconsistent positive effects ";
      write_partial_state(std::cerr, b.post);
      std::cerr << std::endl;
      ok = false;
    }
    for (index_type i = 0; i < a.del.length(); i++) {
      index_set& vs = atom_vars[a.del[i]];
      for (index_type j = 0; j < vs.length(); j++)
	if (!b.post.defines(vs[j])) {
	  if (variables[vs[j]].default_val != no_such_index) {
	    b.post.assign(vs[j], variables[vs[j]].default_val);
	  }
	  else if (pc) {
	    std::cerr << "warning (construction): action " << a.name
		      << " deletes " << ins.atoms[a.del[i]].name
		      << " leaving ";
	    write_variable(std::cerr, variables[vs[j]]);
	    std::cerr << " without a value" << std::endl;
	    std::cerr << "(STRIPS action: pre = ";
	    ins.write_atom_set(std::cerr, a.pre);
	    std::cerr << ", add = ";
	    ins.write_atom_set(std::cerr, a.add);
	    std::cerr << ", del = ";
	    ins.write_atom_set(std::cerr, a.del);
	    std::cerr << ", SAS action: post = ";
	    write_partial_state(std::cerr, b.post);
	    std::cerr << ", variables affected by deletion of "
		      << ins.atoms[a.del[i]].name << " = ";
	    write_variable_set(std::cerr, vs);
	    std::cerr << ")" << std::endl;
	    ok = false;
	  }
	}
    }

    for (index_type i = 0; i < b.pre.length(); i++) {
      if (!b.post.defines(b.pre[i].first) ||
	  b.post.value_of(b.pre[i].first) == b.pre[i].second)
	b.prv.insert(b.pre[i]);
    }
    b.pre.subtract(b.prv);
    b.post.subtract(b.prv);
    b.cost = a.cost;
  }

  return ok;
}

bool SASInstance::construct_safe_actions(const Instance& ins)
{
  bool ok = true;

  for (index_type k = 0; k < ins.n_actions(); k++) {
    const Instance::Action& a = ins.actions[k];

    partial_state b_pre;
    partial_state b_post;
    partial_state b_prv;
    make_partial_state(a.pre, b_pre);
    make_partial_state(a.add, b_post);
    bool pc = b_pre.consistent();

    if (pc && !b_post.consistent()) {
      std::cerr << "warning (construction): action " << a.name
		<< " has inconsistent positive effects ";
      write_partial_state(std::cerr, b_post);
      std::cerr << std::endl;
      ok = false;
    }

    for (index_type i = 0; i < a.del.length(); i++) {
      index_set& vs = atom_vars[a.del[i]];
      for (index_type j = 0; j < vs.length(); j++)
	if (!b_post.defines(vs[j])) {
	  if (variables[vs[j]].default_val != no_such_index) {
	    b_post.assign(vs[j], variables[vs[j]].default_val);
	  }
	  else if (pc) {
	    std::cerr << "warning (construction): action " << a.name
		      << " deletes " << ins.atoms[a.del[i]].name
		      << " leaving ";
	    write_variable(std::cerr, variables[vs[j]]);
	    std::cerr << " without a value" << std::endl;
	    std::cerr << "(STRIPS action: pre = ";
	    ins.write_atom_set(std::cerr, a.pre);
	    std::cerr << ", add = ";
	    ins.write_atom_set(std::cerr, a.add);
	    std::cerr << ", del = ";
	    ins.write_atom_set(std::cerr, a.del);
	    std::cerr << ", SAS action: post = ";
	    write_partial_state(std::cerr, b_post);
	    std::cerr << ", variables affected by deletion of "
		      << ins.atoms[a.del[i]].name << " = ";
	    write_variable_set(std::cerr, vs);
	    std::cerr << ")" << std::endl;
	    ok = false;
	  }
	}
    }

    for (index_type i = 0; i < b_pre.length(); i++)
      if (!b_post.defines(b_pre[i].first) ||
	  b_post.value_of(b_pre[i].first) == b_pre[i].second)
	b_prv.insert(b_pre[i]);
    b_pre.subtract(b_prv);
    b_post.subtract(b_prv);
    index_set u_pre;
    for (index_type i = 0; i < b_post.length(); i++)
      if (!b_pre.defines(b_post[i].first))
	u_pre.insert(b_post[i].first);

    if (!u_pre.empty()) {
      PartialStateEnumerator e(u_pre, signature);
      bool more = e.first();
      while (more) {
	bool is_useless = false;
	if (b_pre.empty()) {
	  is_useless = true;
	  for (index_type i = 0; (i < u_pre.length()) && is_useless; i++)
	    if (e.current_state().value_of(u_pre[i]) !=
		b_post.value_of(u_pre[i]))
	      is_useless = false;
	}
	if (!is_useless) {
	  Action& b = new_action(a.name);
	  for (index_type i = 0; i < b_post.length(); i++) {
	    if (!u_pre.contains(b_post[i].first)) {
	      b.post.insert(b_post[i]);
	    }
	    else if (e.current_state().value_of(b_post[i].first) != b_post[i].second) {
	      b.post.insert(b_post[i]);
	      b.pre.insert(index_pair(b_post[i].first, e.current_state().value_of(b_post[i].first)));
	    }
	    b.pre.insert(b_pre);
	    b.prv = b_prv;
	    b.s_index.insert(k);
	    b.cost = a.cost;
	  }
	}
	more = e.next();
      }
    }
    else {
      Action& b = new_action(a.name);
      b.pre = b_pre;
      b.post = b_post;
      b.prv = b_prv;
      b.s_index.insert(k);
      b.cost = a.cost;
    }
  }

  return ok;
}

index_type SASInstance::find_action_with_post
(const partial_state& post, NTYPE cost)
{
  for (index_type k = 0; k < n_actions(); k++)
    if ((actions[k].post == post) && (actions[k].cost == cost)) return k;
  return no_such_index;
}

index_type SASInstance::find_action_with_cond
(const partial_state& pre, const partial_state& prv, const partial_state& post)
{
  for (index_type k = 0; k < n_actions(); k++)
    if ((actions[k].pre == pre) &&
	(actions[k].prv == prv) &&
	(actions[k].post == post))
      return k;
  return no_such_index;
}

void SASInstance::construct_condition
(const Instance& ins, const index_set& acts, partial_state& cond)
{
  cond.clear();
  for (index_type k = 0; k < acts.length(); k++) {
    map_to_partial_state(ins.actions[acts[k]].pre, cond);
  }
}

void SASInstance::construct_final_action_condition(SASInstance::Action& act)
{
  // undefine inconsistent preconditions (this should by now be safe)
  for (index_type i = 0; i < n_variables(); i++)
    if (!act.pre.consistent(i)) act.pre.undefine(i);

  // find prevail conditions
  for (index_type i = 0; i < act.pre.length(); i++) {
    if (!act.post.defines(act.pre[i].first) ||
	act.post.value_of(act.pre[i].first) == act.pre[i].second)
      act.prv.insert(act.pre[i]);
  }

  // remove prevail conditions from pre and post
  act.pre.subtract(act.prv);
  act.post.subtract(act.prv);
}

void SASInstance::restrict_source_set
(const Instance& ins, const index_set& set, index_set& r_set,
 index_type var, index_type val)
{
  r_set.clear();
  for (index_type k = 0; k < set.length(); k++) {
    const Instance::Action& a = ins.actions[set[k]];
    for (index_type i = 0; i < a.pre.length(); i++) {
      if (atom_map[a.pre[i]].value_of(var) == val)
	r_set.insert(set[k]);
    }
  }
}

index_type SASInstance::find_split_variable
(const partial_state& cond, const partial_state& post, index_set& values)
{
  index_set max_inc;
  for (index_type k = 0; k < n_variables(); k++) if (!cond.consistent(k)) {
    values.clear();
    cond.value_set(k, values);
    if (((values.length() == variables[k].domain.length()) ||
	 (post.defines(k) &&
	  !values.contains(post.value_of(k)) &&
	  (values.length() == (variables[k].domain.length() - 1)))))
      max_inc.insert(k);
    else
      return k;
  }
  if (max_inc.empty()) {
    values.clear();
    return no_such_index;
  }
  else if (max_inc.length() == 1) {
    values.clear();
    return no_such_index;
  }
  else {
    // if cond covers every combination of values (except for values assigned
    // by post) for all variables in max_inc, we could return no_such_index...
    values.clear();
    cond.value_set(max_inc[0], values);
    return max_inc[0];
  }
}

bool SASInstance::construct_minimal_actions(const Instance& ins)
{
  bool ok = true;

  for (index_type k = 0; k < ins.n_actions(); k++) {
    const Instance::Action& a = ins.actions[k];
    partial_state post;
    make_partial_state(a.add, post);
    bool post_ok = true;
    if (!post.consistent()) {
      post_ok = false;
      std::cerr << "warning (construction): action " << a.name
		<< " has inconsistent positive effects ";
      write_partial_state(std::cerr, post);
      std::cerr << std::endl;
      ok = false;
    }
    for (index_type i = 0; i < a.del.length(); i++) {
      index_set& vs = atom_vars[a.del[i]];
      for (index_type j = 0; j < vs.length(); j++)
	if (!post.defines(vs[j])) {
	  if (variables[vs[j]].default_val != no_such_index) {
	    post.assign(vs[j], variables[vs[j]].default_val);
	  }
	  else {
	    post_ok = false;
	    std::cerr << "warning (construction): action " << a.name
		      << " deletes " << ins.atoms[a.del[i]].name
		      << " leaving ";
	    write_variable(std::cerr, variables[vs[j]]);
	    std::cerr << " without a value" << std::endl;
	    ok = false;
	  }
	}
    }

    if (post_ok) {
      index_type i = find_action_with_post(post, a.cost);
      if (i == no_such_index) {
	Action& b = new_action(a.name);
	b.s_index.insert(k);
	b.post = post;
	b.cost = a.cost;
	make_partial_state(a.pre, b.pre);
      }
      else {
	actions[i].s_index.insert(k);
	map_to_partial_state(a.pre, actions[i].pre);
      }
    }
  }

  if (trace_level > 2) {
    std::cerr << "initial list of actions:";
    for (index_type k = 0; k < actions.length(); k++) {
      std::cerr << std::endl << " ";
      write_action(std::cerr, actions[k]);
    }
    std::cerr << std::endl;
  }

  index_type k = 0;
  while (k < n_actions()) {
    index_set values;
    index_type split_v =
      find_split_variable(actions[k].pre, actions[k].post, values);

    if (split_v == no_such_index) {
      construct_final_action_condition(actions[k]);
      if (trace_level > 2) {
	std::cerr << "finished action ";
	write_action(std::cerr, actions[k]);
	std::cerr << std::endl;
      }
      k += 1;
    }

    else if (actions[k].s_index.length() <= 1) {
      std::cerr << "warning (construction): action " << actions[k].name
		<< " has inconsistent preconditions ";
      write_partial_state(std::cerr, actions[k].pre);
      std::cerr << std::endl;
      ok = false;
      k += 1;
    }

    else {
      if (trace_level > 2) {
	std::cerr << "splitting action ";
	write_action(std::cerr, actions[k]);
	std::cerr << " on variable " << variables[split_v].name
		  << std::endl;
      }

      index_set sources(actions[k].s_index);
      index_set covered;

      // assign first value to actions[k]
      restrict_source_set(ins, sources, actions[k].s_index, split_v,values[0]);
      assert(!actions[k].s_index.empty());
      actions[k].name = ins.actions[actions[k].s_index[0]].name;
      covered.insert(actions[k].s_index);
      construct_condition(ins, actions[k].s_index, actions[k].pre);

      // for each remaining value, create a new action
      for (index_type i = 1; i < values.length(); i++) {
	Action& b = new_action(act_default_name);
	restrict_source_set(ins, sources, b.s_index, split_v, values[i]);
	assert(!b.s_index.empty());
	b.name = ins.actions[b.s_index[0]].name;
	covered.insert(b.s_index);
	construct_condition(ins, b.s_index, b.pre);
	b.post = actions[k].post;
	b.cost = actions[k].cost;
      }

      // if this did not cover all source indices, create an additional
      // action
      sources.subtract(covered);
      if (!sources.empty()) {
	Action& b = new_action(ins.actions[sources[0]].name);
	b.s_index = sources;
	construct_condition(ins, b.s_index, b.pre);
	b.post = actions[k].post;
	b.cost = actions[k].cost;
      }
    }
  }

  return ok;
}

bool SASInstance::construct_init_and_goal_state(const Instance& ins)
{
  bool ok = true;

  make_complete_state(ins.init_atoms, init_state);
  if (!init_state.consistent()) {
    std::cerr << "warning (construction): initial state ";
    write_partial_state(std::cerr, init_state);
    std::cerr << " is inconsistent" << std::endl;
    ok = false;
  }

  make_partial_state(ins.goal_atoms, goal_state);
  if (!goal_state.consistent()) {
    std::cerr << "warning (construction): goal state ";
    write_partial_state(std::cerr, goal_state);
    std::cerr << " is inconsistent" << std::endl;
    ok = false;
  }

  return ok;
}

bool SASInstance::construct
(const Instance& ins, bool selective, bool minimal, bool safe)
{
  if (selective) {
    if (trace_level > 0) {
      std::cerr << "selecting invariants..." << std::endl;
    }
    Instance::constraint_vec invs;
    select_invariants(ins, invs);
    if (trace_level > 0) {
      std::cerr << "constructing variables...";
    }
    construct_variables(ins, invs);
    if (trace_level > 0) {
      std::cerr << " " << n_variables() << " variables" << std::endl;
    }
  }
  else {
    if (trace_level > 0) {
      std::cerr << "constructing variables...";
    }
    construct_variables(ins, ins.invariants);
    if (trace_level > 0) {
      std::cerr << " " << n_variables() << " variables" << std::endl;
    }
  }
  bool ok = true;

  if (trace_level > 0) {
    std::cerr << "constructing actions...";
  }
  assert(!minimal || !safe);
  if (minimal) {
    ok &= construct_minimal_actions(ins);
  }
  else if (safe) {
    ok &= construct_safe_actions(ins);
  }
  else {
    ok &= construct_actions(ins);
  }
  remove_inconsistent_actions();
  if (trace_level > 0) {
    std::cerr << " " << n_actions() << " actions" << std::endl;
  }

  if (trace_level > 0) {
    std::cerr << "constructing initial & goal state..." << std::endl;
  }
  ok &= construct_init_and_goal_state(ins);

  if (trace_level > 0) {
    std::cerr << "cross referencing and computing graphs..." << std::endl;
  }
  cross_reference();
  compute_graphs();

  return ok;
}

void SASInstance::remove_inconsistent_actions()
{
  index_type b = 0;
  index_type a = 0;
  while (a < n_actions()) {
    if (actions[a].pre.consistent() &&
	actions[a].post.consistent() &&
	actions[a].prv.consistent()) {
      if (b < a) {
	actions[b] = actions[a];
      }
      b += 1;
      a += 1;
    }
    else {
      if (trace_level > 1) {
	std::cerr << "removing inconsistent action ";
	write_action(std::cerr, actions[a]);
	std::cerr << std::endl;
      }
      a += 1;
    }
  }
  actions.set_length(b);
}

void SASInstance::remove_variables(const bool_vec& set, index_vec& map)
{
  index_type b = 0;
  index_type a = 0;
  index_vec m(no_such_index, variables.length());
  while (a < n_variables()) {
    if (!set[a]) {
      if (b < a) {
	variables[b] = variables[a];
	signature[b] = signature[a];
      }
      m[a] = b;
      b += 1;
    }
    else {
      m[a] = no_such_index;
    }
    a += 1;
  }
  variables.set_length(b);
  signature.set_length(b);

  // remap action conditions
  for (index_type k = 0; k < n_actions(); k++) {
    actions[k].pre = partial_state(actions[k].pre, m);
    actions[k].prv = partial_state(actions[k].prv, m);
    actions[k].post = partial_state(actions[k].post, m);
  }
  // remap init and goal states
  init_state = partial_state(init_state, m);
  goal_state = partial_state(goal_state, m);

  // remap STRIPS-to-SAS map
  if (atom_map_defined()) {
    bool_vec cset(set);
    cset.complement();
    index_set vars;
    cset.copy_to(vars);
    for (index_type k = 0; k < n_atoms; k++) {
      atom_vars[k].intersect(vars);
      atom_map[k] = partial_state(atom_map[k], m);
    }
  }

  // remap the input map
  for (index_type k = 0; k < map.length(); k++)
    if (map[k] != no_such_index) {
      assert(map[k] < m.length());
      map[k] = m[map[k]];
    }
}

void SASInstance::remove_actions(const bool_vec& set, index_vec& map)
{
  index_type b = 0;
  index_type a = 0;
  index_vec m(no_such_index, actions.length());
  while (a < n_actions()) {
    if (!set[a]) {
      if (b < a) {
	actions[b] = actions[a];
      }
      m[a] = b;
      b += 1;
    }
    else {
      m[a] = no_such_index;
    }
    a += 1;
  }
  actions.set_length(b);

  for (index_type k = 0; k < map.length(); k++)
    if (map[k] != no_such_index) {
      assert(map[k] < m.length());
      map[k] = m[map[k]];
    }
}

void SASInstance::cross_reference()
{
  for (index_type k = 0; k < n_variables(); k++) {
    variables[k].enabling_values.clear();
    variables[k].accidental_values.clear();
  }
  for (index_type k = 0; k < n_actions(); k++) {
    index_set p;
    for (index_type i = 0; i < actions[k].prv.length(); i++) {
      index_type var = actions[k].prv[i].first;
      index_type val = actions[k].prv[i].second;
      variables[var].enabling_values.insert(val);
      p.insert(var);
    }
    if (actions[k].post.length() == 1) { // action is unary
      index_type var = actions[k].post[0].first;
    }
    else if (actions[k].post.length() > 1) { // action is non-unary
      for (index_type i = 0; i < actions[k].post.length(); i++) {
	index_type var = actions[k].post[i].first;
	index_type post_val = actions[k].post[i].second;
	variables[var].accidental_values.insert(post_val);
	index_type pre_val = actions[k].pre.value_of(var);
	if (pre_val != no_such_index) {
	  variables[var].enabling_values.insert(pre_val);
	  p.insert(var);
	}
      }
    } // else, action is zero-ary, i.e. useless
    for (index_type i = 0; i < actions[k].post.length(); i++)
      variables[actions[k].post[i].first].set_by.insert(k);
  }
  // compute u_pre for actions
  for (index_type k = 0; k < n_actions(); k++) {
    actions[k].u_pre.clear();
    for (index_type i = 0; i < actions[k].post.length(); i++)
      if (!actions[k].pre.defines(actions[k].post[i].first))
	actions[k].u_pre.insert(actions[k].post[i].first);
  }
  // add goal state values to enabling values
  for (index_type i = 0; i < goal_state.length(); i++) {
    index_type var = goal_state[i].first;
    index_type val = goal_state[i].second;
    variables[var].enabling_values.insert(val);
  }
  // add init state values to accidental values
  for (index_type i = 0; i < init_state.length(); i++) {
    index_type var = init_state[i].first;
    index_type val = init_state[i].second;
    variables[var].accidental_values.insert(val);
  }
  state_hash_function.init(*this);
}

void SASInstance::compute_graphs()
{
  causal_graph.init(n_variables());
  dependency_graph.init(n_variables());
  interference_graph.init(n_variables());

  for (index_type k = 0; k < n_actions(); k++) {
    for (index_type i = 0; i < actions[k].post.length(); i++) {
      for (index_type j = i + 1; j < actions[k].post.length(); j++) {
	causal_graph.add_edge(actions[k].post[i].first,
			      actions[k].post[j].first);
	causal_graph.add_edge(actions[k].post[j].first,
			      actions[k].post[i].first);
	interference_graph.add_undirected_edge(actions[k].post[i].first,
					       actions[k].post[j].first);
      }
      for (index_type j = 0; j < actions[k].prv.length(); j++) {
	causal_graph.add_edge(actions[k].post[i].first,
			      actions[k].prv[j].first);
	dependency_graph.add_edge(actions[k].post[i].first,
				  actions[k].prv[j].first);
      }
      for (index_type j = 0; j < actions[k].pre.length(); j++) {
	dependency_graph.add_edge(actions[k].post[i].first,
				  actions[k].pre[j].first);
      }
    }
  }

//   for (index_type i = 0; i < n_variables(); i++)
//     for (index_type j = 0; j < n_variables(); j++) if (i != j) {
//       for (index_type k = 0; k < n_actions(); k++)
// 	if (actions[k].post.defines(i)) {
// 	  if (actions[k].post.defines(j)) {
// 	    causal_graph.add_edge(i, j);
// 	    interference_graph.add_undirected_edge(i, j);
// 	  }
// 	  else if (actions[k].prv.defines(j)) {
// 	    causal_graph.add_edge(i, j);
// 	  }
// 	}
//     }

  transitive_causal_graph.copy(causal_graph);
  transitive_causal_graph.transitive_closure();
  independence_graph.copy(interference_graph);
  independence_graph.complement();
}

void SASInstance::compute_DTG(index_type v, graph& g)
{
  assert(v < n_variables());
  g.init(variables[v].n_values());
  for (index_type k = 0; k < n_actions(); k++) {
    index_type v_post = actions[k].post.value_of(v);
    if (v_post != no_such_index) {
      index_type v_pre = actions[k].pre.value_of(v);
      if (v_pre == no_such_index) {
	for (index_type i = 0; i < variables[v].n_values(); i++)
	  g.add_edge(i, v_post);
      }
      else {
	g.add_edge(v_pre, v_post);
      }
    }
  }
}

void SASInstance::compute_extended_causal_graph
(graph& g, Heuristic& inc)
{
  assert(atom_map_defined());
  g.copy(causal_graph);
  assert(g.size() == n_variables());
  for (index_type k = 0; k < n_actions(); k++) {
    index_set a_pre;
    map_to_atom_set(actions[k].pre, a_pre);
    map_to_atom_set(actions[k].prv, a_pre);
    for (index_type i = 0; i < n_variables(); i++) {
      if (!actions[k].pre.defines(i) && !actions[k].prv.defines(i)) {
	bool f = false;
	for (index_type j = 0; (j<variables[i].s_index.length()) && !f; j++) {
	  if (INFINITE(inc.incremental_eval(a_pre, variables[i].s_index[j])))
	    f = true;
	}
	if (f) {
	  for (index_type j = 0; j < actions[k].post.length(); j++) {
	    assert(actions[k].post[j].first < n_variables());
	    assert(i < n_variables());
	    g.add_edge(actions[k].post[j].first, i);
	  }
	}
      }
    }
  }
}

void SASInstance::action_to_partial_state_sequence
(const index_vec& plan, partial_state_vec& psv)
{
  psv.assign_value(partial_state(), plan.length() + 1);
  for (index_type k = 0; k < plan.length(); k++) {
    psv[k].insert(actions[plan[k]].pre);
    psv[k + 1].insert(actions[plan[k]].post);
    psv[k].insert(actions[plan[k]].prv);
    psv[k + 1].insert(actions[plan[k]].prv);
  }
}

partial_state SASInstance::side_constraints(index_type act, index_type var)
{
  partial_state c(actions[act].prv);
  c.insert(actions[act].pre);
  c.undefine(var);
  return c;
}

partial_state SASInstance::side_effects(index_type act, index_type var)
{
  partial_state c(actions[act].post);
  c.undefine(var);
  return c;
}

// void SASInstance::compute_domain_transitions(const ACF& cost)
// {
//   for (index_type v = 0; v < n_variables(); v++) {
//     Variable& var = variables[v];
//     if (trace_level > 1) {
//       std::cerr << "computing domain transition info for ";
//       write_variable(std::cerr, var);
//       std::cerr << "..." << std::endl;
//     }
//     var.dtcost.init(POS_INF, var.n_values());
//     var.nsc.init(var.n_values());
//     var.nse.init(var.n_values());
//     for (index_type k = 0; k < var.n_values(); k++) var.dtcost[k][k] = 0;
//     for (index_type k = 0; k < n_actions(); k++) {
//       index_type post_val = actions[k].post.value_of(var.index);
//       if (post_val != no_such_index) {
// 	index_type pre_val = actions[k].pre.value_of(var.index);
// 	if (pre_val != no_such_index) {
// 	  if (INFINITE(var.dtcost[pre_val][post_val])) {
// 	    var.nsc[pre_val][post_val] = side_constraints(k, var.index);
// 	    var.nse[pre_val][post_val] = side_effects(k, var.index);
// 	    var.dtcost[pre_val][post_val] = cost(k);
// 	  }
// 	  else {
// 	    var.dtcost[pre_val][post_val] =
// 	      MIN(var.dtcost[pre_val][post_val], cost(k));
// 	    var.nsc[pre_val][post_val].intersect(side_constraints(k, var.index));
// 	    var.nse[pre_val][post_val].intersect(side_effects(k, var.index));
// 	  }
// 	}
// 	else {
// 	  for (index_type pre_val = 0; pre_val < var.n_values(); pre_val++) {
// 	    if (INFINITE(var.dtcost[pre_val][post_val])) {
// 	      var.nsc[pre_val][post_val] = side_constraints(k, var.index);
// 	      var.nse[pre_val][post_val] = side_effects(k, var.index);
// 	      var.dtcost[pre_val][post_val] = cost(k);
// 	    }
// 	    else {
// 	      var.dtcost[pre_val][post_val] =
// 		MIN(var.dtcost[pre_val][post_val], cost(k));
// 	      var.nsc[pre_val][post_val].intersect(side_constraints(k, var.index));
// 	      var.nse[pre_val][post_val].intersect(side_effects(k, var.index));
// 	    }
// 	  }
// 	}
//       }
//     }
//     if (trace_level > 2) {
//       std::cerr << "base transitions (no transitive closure):" << std::endl;
//       write_variable_info(std::cerr, var);
//     }
//     for (index_type k = 0; k < var.n_values(); k++)
//       for (index_type i = 0; i < var.n_values(); i++)
// 	for (index_type j = 0; j < var.n_values(); j++) {
// 	  NTYPE ikj_cost = var.dtcost[i][k] + var.dtcost[k][j];
// 	  if (FINITE(ikj_cost)) {
// 	    if (INFINITE(var.dtcost[i][j])) {
// 	      var.dtcost[i][j] = ikj_cost;
// 	      var.nsc[i][j] = var.nsc[i][k];
// 	      var.nsc[i][j].insert(var.nsc[k][j]);
// 	      var.nse[i][j] = var.nse[i][k];
// 	      var.nse[i][j].insert(var.nse[k][j]);
// 	    }
// 	    else {
// 	      var.dtcost[i][j] = MIN(var.dtcost[i][j], ikj_cost);
// 	      partial_state c(var.nsc[i][k]);
// 	      c.insert(var.nsc[k][j]);
// 	      var.nsc[i][j].intersect(c);
// 	      partial_state e(var.nse[i][k]);
// 	      e.insert(var.nse[k][j]);
// 	      var.nse[i][j].intersect(e);
// 	    }
// 	  }
// 	}
//     if (trace_level > 1) {
//       write_variable_info(std::cerr, var);
//     }
//   }
// }
// 
// void SASInstance::path_sce
// (index_type var, index_type init_val, index_type goal_val, NTYPE bound,
//  const partial_state& sc, const partial_state& se, bool& solved,
//  partial_state& nec_sc, partial_state& nec_se,
//  partial_state& pos_sc, partial_state& pos_se)
// {
//   if (init_val == goal_val) {
//     if (!solved) {
//       nec_sc = sc;
//       nec_se = se;
//       pos_sc = sc;
//       pos_se = se;
//       solved = true;
//     }
//     else {
//       nec_sc.intersect(sc);
//       nec_se.intersect(se);
//       pos_sc.insert(sc);
//       pos_se.insert(se);
//     }
//   }
//   else {
//     for (index_type k = 0; k < n_actions(); k++) {
//       index_type new_val = actions[k].post.value_of(var);
//       index_type pre_val = actions[k].pre.value_of(var);
//       if ((new_val != no_such_index) &&
// 	  ((pre_val == init_val) || (pre_val == no_such_index)) &&
// 	  ((variables[var].dtcost[new_val][goal_val] + actions[k].cost) <= bound)) {
// 	partial_state new_sc(sc);
// 	new_sc.insert(actions[k].pre);
// 	new_sc.insert(actions[k].prv);
// 	new_sc.undefine(var);
// 	partial_state new_se(se);
// 	new_se.insert(actions[k].post);
// 	new_se.undefine(var);
// 	path_sce(var, new_val, goal_val, bound - actions[k].cost,
// 		 new_sc, new_se, solved, nec_sc, nec_se, pos_sc, pos_se);
//       }
//     }
//   }
// }
// 
// void SASInstance::optimal_path_sce
// (index_type var, index_type init_val, index_type goal_val,
//  partial_state& nec_sc, partial_state& nec_se,
//  partial_state& pos_sc, partial_state& pos_se)
// {
//   nec_sc.clear();
//   nec_se.clear();
//   pos_sc.clear();
//   pos_se.clear();
//   if (INFINITE(variables[var].dtcost[init_val][goal_val])) return;
//   partial_state sc;
//   partial_state se;
//   bool solved = false;
//   path_sce(var, init_val, goal_val, variables[var].dtcost[init_val][goal_val],
// 	   sc, se, solved, nec_sc, nec_se, pos_sc, pos_se);
//   std::cerr << "solved = " << solved
// 	    << ", nec sc = ";
//   write_partial_state(std::cerr, nec_sc);
//   std::cerr << ", nec se = ";
//   write_partial_state(std::cerr, nec_se);
//   std::cerr << ", pos sc = ";
//   write_partial_state(std::cerr, pos_sc);
//   std::cerr << ", pos se = ";
//   write_partial_state(std::cerr, pos_se);
//   std::cerr << std::endl;
// }

// void SASInstance::compute_nsce_cg
// (const partial_state& init, const partial_state& goal, weighted_graph& g)
// {
//   // compute_domain_transitions(cost);
//   partial_state x_goal;
//   extend_nsc(init, goal, x_goal);
//   g.init(n_variables());
//   for (index_type i = 0; i < n_variables(); i++) {
//     index_type init_val = init.value_of(i);
//     assert(init_val != no_such_index);
//     index_set goal_vals;
//     x_goal.value_set(i, goal_vals);
//     if (trace_level > 2) {
//       std::cerr << "checking variable ";
//       variables[i].name->write(std::cerr, Name::NC_INSTANCE);
//       std::cerr << " with necessary values = ";
//       write_value_set(std::cerr, i, goal_vals);
//       std::cerr << "..." << std::endl;
//     }
//     for (index_type k = 0; k < goal_vals.length(); k++) {
//       if (trace_level > 2) {
// 	std::cerr << " necessary transition ";
// 	variables[i].name->write(std::cerr, Name::NC_INSTANCE);
// 	std::cerr << ": ";
// 	variables[i].domain[init_val]->write(std::cerr, Name::NC_INSTANCE);
// 	std::cerr << " -> ";
// 	variables[i].domain[goal_vals[k]]->write(std::cerr, Name::NC_INSTANCE);
// 	std::cerr << " (nsc = ";
// 	write_partial_state(std::cerr,
// 			    variables[i].nsc[init_val][goal_vals[k]]);
// 	std::cerr << ", nse = ";
// 	write_partial_state(std::cerr,
// 			    variables[i].nse[init_val][goal_vals[k]]);
// 	std::cerr << ")" << std::endl;
//       }
//       for (index_type j = 0; j < n_variables(); j++) if (i != j) {
// 	if (variables[i].nsc[init_val][goal_vals[k]].defines(j))
// 	  g.increment_edge_weight(j, i, 1);
// 	if (variables[i].nse[init_val][goal_vals[k]].defines(j))
// 	  g.increment_edge_weight(j, i, 1);
//       }
//     }
//   }
// }

void SASInstance::construct(const SASInstance& ins, const index_set& vars)
{
  variable_reduce_map.assign_value(no_such_index, ins.n_variables());
  variable_expand_map.assign_value(no_such_index, 0);
  action_reduce_map.assign_value(no_such_index, ins.n_actions());
  signature.clear();

  // copy variables
  for (index_type k = 0; k < vars.length(); k++) {
    Variable& v = new_variable(ins.variables[vars[k]].name);
    v.s_index = ins.variables[vars[k]].s_index;
    for (index_type i = 0; i < ins.variables[vars[k]].n_values(); i++)
      v.domain.append(ins.variables[vars[k]].domain[i]);
    v.default_val = ins.variables[vars[k]].default_val;
    signature.append(ins.signature[vars[k]]);

    variable_reduce_map[vars[k]] = v.index;
    variable_expand_map.append(vars[k]);
  }

  // construct reduced actions
  for (index_type k = 0; k < ins.n_actions(); k++) {
    partial_state post(ins.actions[k].post, variable_reduce_map);
    if (!post.empty()) {
      partial_state pre(ins.actions[k].pre, variable_reduce_map);
      partial_state prv(ins.actions[k].prv, variable_reduce_map);

      // check for duplicates
      index_type i = find_action_with_cond(pre, prv, post);
      if (i == no_such_index) {
	Action& a = new_action(ins.actions[k].name);
	a.s_index = ins.actions[k].s_index;
	a.pre = pre;
	a.prv = prv;
	a.post = post;
	a.cost = ins.actions[k].cost;
	action_reduce_map[k] = a.index;
      }
      else {
	actions[i].s_index.insert(ins.actions[k].s_index);
	// if new reduced action has lower cost, remove earlier
	// (higher-cost) actions from the action_reduce_map
	if (ins.actions[k].cost < actions[i].cost) {
	  actions[i].cost = ins.actions[k].cost;
	  for (index_type j = 0; j < k; j++)
	    if (action_reduce_map[j] == i)
	      action_reduce_map[j] = no_such_index;
	}
	action_reduce_map[k] = i;
      }
    }
  }

  init_state = partial_state(ins.init_state, variable_reduce_map);
  goal_state = partial_state(ins.goal_state, variable_reduce_map);
}

void SASInstance::construct_atom_map
(const SASInstance& ins, const index_set& vars)
{
  assert(ins.atom_map_defined());
  atom_map = new partial_state[ins.n_atoms];
  atom_vars = new index_set[ins.n_atoms];
  n_atoms = ins.n_atoms;
  for (index_type k = 0; k < ins.n_atoms; k++) {
    atom_vars[k] = ins.atom_vars[k];
    atom_vars[k].intersect(vars);
    for (index_type i = 0; i < ins.atom_map[k].length(); i++) {
      index_type var = ins.atom_map[k][i].first;
      index_type val = ins.atom_map[k][i].second;
      if (vars.contains(var)) {
 	assert(variable_reduce_map[var] != no_such_index);
 	atom_map[k].assign(variable_reduce_map[var], val);
      }
    }
  }
}

Instance* SASInstance::convert_to_STRIPS()
{
  Instance* ins = new Instance(name);
  for (index_type k = 0; k < n_variables(); k++) {
    Instance::Constraint& c = ins->new_invariant(variables[k].name);
    c.lim = 1;
    c.exact = true;
    for (index_type v = 0; v < variables[k].n_values(); v++) {
      Name* n =
	new ConcatenatedName(variables[k].name, variables[k].domain[v], '=');
      Instance::Atom& p = ins->new_atom(n);
      c.set.insert(p.index);
      if (init_state.value_of(k) == v) p.init = true;
      if (goal_state.value_of(k) == v) p.goal = true;
    }
  }
  assert(ins->n_invariants() == n_variables());
  for (index_type k = 0; k < n_actions(); k++) {
    Instance::Action& a = ins->new_action(actions[k].name);
    for (index_type i = 0; i < actions[k].pre.length(); i++) {
      index_type var = actions[k].pre[i].first;
      index_type val = actions[k].pre[i].second;
      index_type p = ins->invariants[var].set[val];
      a.pre.insert(p);
      a.del.insert(p);
    }
    for (index_type i = 0; i < actions[k].prv.length(); i++) {
      index_type var = actions[k].prv[i].first;
      index_type val = actions[k].prv[i].second;
      index_type p = ins->invariants[var].set[val];
      a.pre.insert(p);
    }
    for (index_type i = 0; i < actions[k].post.length(); i++) {
      index_type var = actions[k].post[i].first;
      index_type val = actions[k].post[i].second;
      index_type p = ins->invariants[var].set[val];
      a.add.insert(p);
      if (!actions[k].pre.defines(var)) {
	for (index_type j = 0; j < ins->invariants[var].set.length(); j++)
	  if (j != val) a.del.insert(ins->invariants[var].set[j]);
      }
    }
    a.cost = actions[k].cost;
  }

  return ins;
}

Instance* SASInstance::reconstruct_STRIPS()
{
  assert(atom_map_defined());

  Instance* ins = new Instance(name);
  index_set atoms;
  index_vec new_atom_map(no_such_index, 0);
  for (index_type k = 0; k < n_variables(); k++) {
    if (variables[k].s_index.length() == 1) {
      index_type o = variables[k].s_index[0];
      if (!atoms.contains(o)) {
	Instance::Atom& p = ins->new_atom(variables[k].name);
	if (init_state.value_of(k) == 0) p.init = true;
	if (goal_state.value_of(k) == 0) p.goal = true;
	atoms.insert(o);
	if (o >= new_atom_map.length()) new_atom_map.set_length(o + 1);
	new_atom_map[o] = p.index;
      }
    }
    else {
      for (index_type v = 0; v < variables[k].s_index.length(); v++) {
	index_type o = variables[k].s_index[v];
	if (!atoms.contains(o)) {
	  Instance::Atom& p = ins->new_atom(variables[k].domain[v]);
	  if (init_state.value_of(k) == v) p.init = true;
	  if (goal_state.value_of(k) == v) p.goal = true;
	  atoms.insert(o);
	  if (o >= new_atom_map.length()) new_atom_map.set_length(o + 1);
	  new_atom_map[o] = p.index;
	}
      }
    }
  }

  bool ok = true;
  for (index_type k = 0; (k < n_actions()) && ok; k++) {
    Instance::Action& a = ins->new_action(actions[k].name);
    for (index_type i = 0; (i < actions[k].pre.length()) && ok; i++) {
      index_type var = actions[k].pre[i].first;
      index_type val = actions[k].pre[i].second;
      if (val < variables[var].s_index.length()) {
	index_type o = variables[var].s_index[val];
	assert(new_atom_map[o] < ins->n_atoms());
	a.pre.insert(new_atom_map[o]);
	a.del.insert(new_atom_map[o]);
      }
      else {
	if (trace_level > 0) {
	  std::cerr << "reconstruction failed: precondition "
		    << variables[var].name << " = "
		    << variables[var].domain[val]
		    << " (" << var << "=" << val << ") of action "
		    << a.name << " can not be expressed as an atom (s: "
		    << variables[var].s_index << ")" << std::endl;
	}
	ok = false;
      }
    }
    for (index_type i = 0; (i < actions[k].prv.length()) && ok; i++) {
      index_type var = actions[k].prv[i].first;
      index_type val = actions[k].prv[i].second;
      if (val < variables[var].s_index.length()) {
	index_type o = variables[var].s_index[val];
	assert(new_atom_map[o] < ins->n_atoms());
	a.pre.insert(new_atom_map[o]);
      }
      else {
	if (trace_level > 0) {
	  std::cerr << "reconstruction failed: prevail-condition "
		    << variables[var].name << " = "
		    << variables[var].domain[val]
		    << " (" << var << "=" << val << ") of action "
		    << a.name << " can not be expressed as an atom (s: "
		    << variables[var].s_index << ")" << std::endl;
	}
	ok = false;
      }
    }
    for (index_type i = 0; (i < actions[k].post.length()) && ok; i++) {
      index_type var = actions[k].post[i].first;
      index_type val = actions[k].post[i].second;
      if (val < variables[var].s_index.length()) {
	index_type o = variables[var].s_index[val];
	assert(new_atom_map[o] < ins->n_atoms());
	a.add.insert(new_atom_map[o]);
      }
      // note: if post-cond value does not correspond to any atom, the delete
      // effect (added when pre-cond is encoded) is the complete effect
    }
    a.cost = actions[k].cost;
  }

  if (!ok) {
    delete ins;
    return 0;
  }
  else {
    return ins;
  }
}

const partial_state* SASInstance::atom_map_defined() const
{
  if ((atom_map == 0) || (atom_vars == 0)) return 0;
  return atom_map;
}

index_type SASInstance::atom_map_n() const
{
  if (!atom_map_defined()) return 0;
  return n_atoms;
}

void SASInstance::make_partial_state
(const bool_vec& s, partial_state& m) const
{
  m.clear();
  map_to_partial_state(s, m);
}

void SASInstance::make_partial_state
(const index_set& s, partial_state& m) const
{
  m.clear();
  map_to_partial_state(s, m);
}

void SASInstance::make_complete_state
(const bool_vec& s, partial_state& m) const
{
  m.clear();
  map_to_partial_state(s, m);
  make_state_complete(m);
}

void SASInstance::make_complete_state
(const index_set& s, partial_state& m) const
{
  m.clear();
  map_to_partial_state(s, m);
  make_state_complete(m);
}

void SASInstance::map_to_partial_state
(const bool_vec& s, partial_state& m) const
{
  assert(atom_map_defined());
  for (index_type k = 0; k < n_atoms; k++)
    if (s[k]) m.insert(atom_map[k]);
}

void SASInstance::map_to_partial_state
(const index_set& s, partial_state& m) const
{
  assert(atom_map_defined());
  for (index_type k = 0; k < s.length(); k++)
    m.insert(atom_map[s[k]]);
}

void SASInstance::map_to_partial_state
(index_type p, partial_state& m) const
{
  assert(atom_map_defined());
  m.insert(atom_map[p]);
}

void SASInstance::make_state_complete
(partial_state& s) const
{
  for (index_type k = 0; k < n_variables(); k++)
    if (variables[k].default_val != no_such_index)
      if (!s.defines(k)) s.assign(k, variables[k].default_val);
}

void SASInstance::make_atom_set
(const partial_state& s, index_set& set) const
{
  set.clear();
  map_to_atom_set(s, set);
}

void SASInstance::make_dual_atom_sets
(const partial_state& s, index_set& p_set, index_set& n_set) const
{
  p_set.clear();
  n_set.clear();
  map_to_dual_atom_sets(s, p_set, n_set);
}

void SASInstance::map_to_atom_set
(index_type var, index_type val, index_set& set) const
{
  assert(!variables[var].s_index.empty());
  if (val < variables[var].s_index.length())
    set.insert(variables[var].s_index[val]);
}

void SASInstance::map_to_atom_set
(const partial_state& s, index_set& set) const
{
  for (index_type k = 0; k < s.length(); k++) {
    assert(!variables[s[k].first].s_index.empty());
    if (s[k].second < variables[s[k].first].s_index.length())
      set.insert(variables[s[k].first].s_index[s[k].second]);
  }
}

void SASInstance::map_to_dual_atom_sets
(const partial_state& s, index_set& p_set, index_set& n_set) const
{
  for (index_type k = 0; k < s.length(); k++) {
    assert(!variables[s[k].first].s_index.empty());
    if (s[k].second < variables[s[k].first].s_index.length()) {
      p_set.insert(variables[s[k].first].s_index[s[k].second]);
    }
    else {
      n_set.insert(variables[s[k].first].s_index);
    }
  }
}

void SASInstance::variable_atom_set
(const index_set& vars, index_set& set) const
{
  set.clear();
  for (index_type k = 0; k < vars.length(); k++) {
    assert(vars[k] < n_variables());
    set.insert(variables[vars[k]].s_index);
  }
}

void SASInstance::atom_variable_set
(const index_set& atoms, index_set& set) const
{
  assert(atom_vars);
  set.clear();
  for (index_type k = 0; k < atoms.length(); k++) {
    assert(atoms[k] < n_atoms);
    set.insert(atom_vars[atoms[k]]);
  }
}

void SASInstance::map_action_set
(const index_set& sas_acts, index_set& strips_acts) const
{
  strips_acts.clear();
  for (index_type k = 0; k < sas_acts.length(); k++)
    strips_acts.insert(actions[sas_acts[k]].s_index);
}

void SASInstance::map_action_set
(const bool_vec& sas_acts, bool_vec& strips_acts) const
{
  strips_acts.assign_value(false);
  for (index_type k = 0; k < n_actions(); k++)
    if (sas_acts[k])
      for (index_type i = 0; i < actions[k].s_index.length(); i++)
	strips_acts[actions[k].s_index[i]] = true;
}

index_type SASInstance::corresponding_action(index_type strips_act) const
{
  for (index_type k = 0; k < n_actions(); k++)
    if (actions[k].s_index.contains(strips_act)) return k;
  return no_such_index;
}

SASInstance* SASInstance::reduce(const index_set& vars) const
{
  SASInstance* r = new SASInstance(*this, vars);
  r->cross_reference();
  if (atom_map_defined()) {
    r->construct_atom_map(*this, vars);
  }
  r->compute_graphs();
  return r;
}

SASInstance* SASInstance::reduce_to_goal_variables() const
{
  index_set goal_variables;
  goal_state.defined_set(goal_variables);
  return reduce(goal_variables);
}

SASInstance* SASInstance::copy() const
{
  return new SASInstance(*this);
}

void SASInstance::construct_random_domain
(index_type n_var, index_type n_val, index_type dtg_type, rational mixf,
 const graph& cg, index_type c_max, rational t_cg, index_type a_min,
 RNG& rnd)
{
  if ((dtg_type <= 1) && (mixf <= 0)) {
    mixf = 20;
  }
  count_type mixn = rational::floor(n_val * n_val * mixf).numerator();
  if (trace_level > 0) {
    std::cerr << "random SAS domain parameters: #variables = " << n_var
	      << ", #values = " << n_val
	      << ", DTG type = " << dtg_type
	      << " (mix N = " << mixn
	      << "), target causal graph density = "
	      << rational(cg.n_edges(), n_var * (n_var - 1))
	      << " (min allowed = " << t_cg << "*target)"
	      << ", max coupling size = " << c_max
	      << ", min #actions = " << a_min
	      << std::endl;
  }
  if ((n_var < 1) ||
      (n_val < 2) ||
      (cg.size() != n_var) ||
      (t_cg < 0) || (t_cg > 1) ||
      (c_max < 2)) {
    std::cerr << "error: invalid parameters" << std::endl;
    exit(255);
  }

  graph* dtgs = new graph[n_var];
  pair_set* trans = new pair_set[n_var];
  bool_vec* t_free = new bool_vec[n_var];
  index_type n_free = 0;

  if (trace_level > 0) {
    std::cerr << "creating " << n_var << " variables..." << std::endl;
  }
  for (index_type k = 0; k < n_var; k++) {
    Name* n = new EnumName("var", k);
    Variable& v = new_variable(n);
    for (index_type i = 0; i < n_val; i++)
      v.domain.append(new EnumName("val", i));
    dtgs[k].init(n_val);
    if (dtg_type == 5) { // tree
      index_type b = (index_type)(ceil(log(n_val)));
      dtgs[k].random_tree(b, index_type_max, rnd);
      if (mixn > 0) dtgs[k].randomize_connected(mixn, rnd);
    }
    else if (dtg_type == 4) { // undirected grid DTG
      index_type w = (index_type)(floor(sqrt(n_val)));
      if ((w < 2) || (w*w != n_val)) {
	std::cerr << "error: can't make grid-type DTG with " << n_val
		  << " values" << std::endl;
	exit(255);
      }
      for (index_type i = 0; i < w; i++) {
	for (index_type j = 0; j < w - 1; j++)
	  dtgs[k].add_undirected_edge((i * w) + j, (i * w) + j + 1);
      }
      for (index_type i = 0; i < w - 1; i++) {
	for (index_type j = 0; j < w; j++)
	  dtgs[k].add_undirected_edge((i * w) + j, ((i + 1) * w) + j);
      }
      if (mixn > 0) dtgs[k].randomize_strongly_connected(mixn, rnd);
    }
    else if (dtg_type == 3) { // directed cyclic DTG
      for (index_type i = 0; i < n_val - 1; i++) dtgs[k].add_edge(i, i + 1);
      dtgs[k].add_edge(n_val - 1, 0);
      if (mixn > 0) dtgs[k].randomize_strongly_connected(mixn, rnd);
    }
    else if (dtg_type == 2) { // undirected cyclic DTG
      for (index_type i = 0; i < n_val - 1; i++)
	dtgs[k].add_undirected_edge(i, i + 1);
      dtgs[k].add_undirected_edge(n_val - 1, 0);
      if (mixn > 0) dtgs[k].randomize_strongly_connected(mixn, rnd);
    }
    else if (dtg_type == 1) { // strongly connected uniform random DTG
      dtgs[k].random_strongly_connected_digraph(mixn, rnd);
    }
    else { // uniform random DTG
      dtgs[k].random_digraph(mixn, rnd);
    }
    dtgs[k].edges(trans[k]);
    t_free[k].assign_value(true, trans[k].length());
    n_free += trans[k].length();
  }

  if (trace_level > 0) {
    std::cerr << n_free << " domain transitions" << std::endl;
    std::cerr << "target causal graph has " << cg.n_edges() << " edges"
	      << std::endl;
    if (trace_level > 1) {
      std::cerr << "target causal graph: " << cg << std::endl;
    }
  }

  index_type n_generated = 0;

  if (trace_level > 0) {
    std::cerr << "generating actions..." << std::endl;
  }

  // first, generate actions to cover all domain transitions
  for (index_type v = 0; v < n_variables(); v++)
    for (index_type t = 0; t < trans[v].length(); t++)
      if (t_free[v][t]) {
	make_random_action(v,t,trans,t_free,n_free,cg,c_max,true,rnd);
	n_generated += 1;
      }
  compute_graphs();
  index_type n_actual = causal_graph.n_edges();
  index_type n_since_last_check = 0;

  // next, generate actions to reach target cg density and #actions
  while ((n_actual < (cg.n_edges()*t_cg)) || (n_actions() <= a_min)) {
    index_type v = rnd.random_in_range(n_var);
    index_type t = rnd.random_in_range(trans[v].length());
    make_random_action(v, t, trans, t_free, n_free, cg, c_max, false, rnd);

    n_generated += 1;
    n_since_last_check += 1;
    if (n_since_last_check > (n_var*n_val)) {
      compute_graphs();
      n_actual = causal_graph.n_edges();
      n_since_last_check = 0;
      if (trace_level > 0) {
	std::cerr << n_actions() << " actions ("
		  << n_generated << " generated), "
		  << n_actual << " edges in current causal graph, "
		  << n_free << " domain transitions remaining..."
		  << std::endl;
      }
    }
  }

  cross_reference();
  compute_graphs();

  if (trace_level > 0) {
    std::cerr << "edges in final causal graph " << causal_graph.n_edges()
	      << " (target = " << cg.n_edges() << ")"
	      << std::endl;
    std::cerr << "#actions " << n_actions()
	      << " (target = " << a_min
	      << ", generated = " << n_generated << ")"
	      << std::endl;
    std::cerr << "#domain transitions remaining = " << n_free
	      << std::endl;
    if (trace_level > 1) {
      std::cerr << "difference between final and target causal graph:"
		<< std::endl;
      for (index_type i = 0; i < n_variables(); i++)
	for (index_type j = 0; j < n_variables(); j++) {
	  if (cg.adjacent(i, j) && !causal_graph.adjacent(i, j)) {
	    std::cerr << "missing edge " << variables[i].name
		      << " -> " << variables[j].name << std::endl;
	  }
	  else if (!cg.adjacent(i, j) && causal_graph.adjacent(i, j)) {
	    std::cerr << "extra edge " << variables[i].name
		      << " -> " << variables[j].name << std::endl;
	  }
	}
    }
  }
}

void SASInstance::make_random_action
(index_type v, index_type t, pair_set* trans, bool_vec* t_free,
 index_type& n_free, const graph& cg, index_type c_max, bool prefer_free,
 RNG& rnd)
{
  if (trace_level > 1) {
    std::cerr << "building action around transition "
	      << variables[v].name << ": "
	      << variables[v].domain[trans[v][t].first] << " -> "
	      << variables[v].domain[trans[v][t].second] << "..."
	      << std::endl;
  }
  index_set vn(cg.successors(v));
  index_set s_couple;
  index_set s_dep;
  while (!vn.empty() && (s_couple.length() + s_dep.length() < c_max - 1)) {
    if (trace_level > 1) {
      std::cerr << "selectable neighbours of "
		<< variables[v].name << ": ";
      write_variable_set(std::cerr, vn);
      std::cerr << std::endl;
    }
    index_type next = vn[rnd.random_in_range(vn.length())];
    bool can_couple = cg.adjacent(next, v);
//     for (index_type i = 0; (i < s_couple.length()) && can_couple; i++)
//       if (!cg.adjacent(s_couple[i], next)) can_couple = false;
    if (can_couple) {
      if (trace_level > 1) {
	std::cerr << "selected " << variables[next].name
		  << " (coupling)" << std::endl;
      }
      s_couple.insert(next);
      vn.intersect(cg.successors(next));
    }
    else {
      if (trace_level > 1) {
	std::cerr << "selected " << variables[next].name
		  << " (dependency)" << std::endl;
      }
      s_dep.insert(next);
      vn.intersect(cg.predecessors(next));
    }
  }
  if (trace_level > 1) {
    std::cerr << (s_couple.length() + s_dep.length())
	      << " selected, remaining: ";
    write_variable_set(std::cerr, vn);
    std::cerr << std::endl;
  }

  partial_state pre;
  partial_state prv;
  partial_state post;

  pre.assign(v, trans[v][t].first);
  post.assign(v, trans[v][t].second);
  if (t_free[v][t]) n_free -= 1;
  t_free[v][t] = false;
  for (index_type i = 0; i < s_couple.length(); i++) {
    index_type v1 = s_couple[i];
    if (trans[v1].length() == 0) {
      std::cerr << "error: coupling between variables "
		<< variables[v].name << " and "
		<< variables[v1].name
		<< " but " << variables[v1].name
		<< " has no transitions"
		<< " (use strongly connected DTGs to prevent)"
		<< std::endl;
      exit(255);
    }
    index_type t1 = 0;
    index_type v1_free = 0;
    if (prefer_free) {
      for (index_type k = 0; k < trans[v1].length(); k++)
	if (t_free[v1][k]) v1_free += 1;
      if (v1_free > 0) {
	index_type l = rnd.random_in_range(v1_free) + 1;
	while (l > 0) {
	  if (t_free[v1][t1]) l -= 1;
	  if (l > 0) t1 += 1;
	}
	assert(t1 < trans[v1].length());
	assert(t_free[v1][t1]);
      }
      else {
	t1 = rnd.random_in_range(trans[v1].length());
      }
    }
    else {
      t1 = rnd.random_in_range(trans[v1].length());
    }
    pre.assign(v1, trans[v1][t1].first);
    post.assign(v1, trans[v1][t1].second);
    if (t_free[v1][t1]) n_free -= 1;
    t_free[v1][t1] = false;
  }
  for (index_type i = 0; i < s_dep.length(); i++) {
    index_type v1 = s_dep[i];
    prv.assign(v1, rnd.random_in_range(variables[v1].n_values()));
  }

  if (trace_level > 1) {
    std::cerr << "pre = ";
    write_partial_state(std::cerr, pre);
    std::cerr << ", post = ";
    write_partial_state(std::cerr, post);
    std::cerr << ", prevail = ";
    write_partial_state(std::cerr, prv);
    std::cerr << std::endl;
  }

  index_type e = find_action_with_cond(pre, prv, post);
  if (e == no_such_index) {
    Name* n = new EnumName("act", n_actions());
    Action& a = new_action(n);
    a.pre = pre;
    a.post = post;
    a.prv = prv;
    if (trace_level > 1) {
      std::cerr << "created new action " << n << std::endl;
    }
  }
  else if (trace_level > 1) {
    std::cerr << "identical action " << actions[e].name << " exists"
	      << std::endl;
  }
}

void SASInstance::construct_random_instance
(index_type n_goals, const index_set& goal_vars, RNG& rnd)
{
  if (trace_level > 0) {
    std::cerr << "random SAS instance parameters: #goals = " << n_goals
	      << ", selectable variables = ";
    write_variable_set(std::cerr, goal_vars);
    std::cerr << std::endl;
  }
  if (n_goals > goal_vars.length()) {
    std::cerr << "error: invalid parameters" << std::endl;
    exit(255);
  }
  init_state.clear();
  for (index_type k = 0; k < n_variables(); k++) {
    graph dtg;
    compute_DTG(k, dtg);
    dtg.strongly_connected_components();
    assert(dtg.n_components() > 0);
    if (dtg.n_components() == 1) {
      init_state.assign(k, rnd.random_in_range(variables[k].n_values()));
    }
    else {
      graph scc_tree;
      dtg.component_tree(scc_tree);
      index_set r;
      for (index_type i = 0; i < scc_tree.size(); i++)
	if (scc_tree.in_degree(i) == 0)
	  r.insert(i);
      index_type j = rnd.select_one_of(r);
      index_set c;
      dtg.component_node_set(j, c);
      init_state.assign(k, rnd.select_one_of(c));
    }
  }
  goal_state.clear();
  index_set s;
  index_set sg;
  rnd.select_fixed_set(s, n_goals, goal_vars.length());
  for (index_type k = 0; k < s.length(); k++) {
    assert(s[k] < goal_vars.length());
    sg.insert(goal_vars[s[k]]);
  }
  bool triv = true;
  for (index_type k = 0; k < sg.length(); k++) {
    index_type vg = rnd.random_in_range(variables[sg[k]].n_values());
    if (vg != init_state.value_of(sg[k])) triv = false;
    goal_state.assign(sg[k], vg);
  }
  if (triv) { // make non-trivial
    index_type ng = rnd.random_in_range(sg.length());
    index_type gv = rnd.random_in_range(variables[ng].n_values(),
					init_state.value_of(ng));
    goal_state.assign(ng, gv);
  }
}

void SASInstance::construct_random_instance
(index_type n_goals, RNG& rnd)
{
  index_set gv;
  gv.fill(n_variables());
  construct_random_instance(n_goals, gv, rnd);
}

// void SASInstance::extend_nsc
// (const partial_state& init, const partial_state& goal, partial_state& x_goal)
// {
//   if (trace_level > 2) {
//     std::cerr << "extend_nsc: init = ";
//     write_partial_state(std::cerr, init);
//     std::cerr << ", goal = ";
//     write_partial_state(std::cerr, goal);
//     std::cerr << std::endl;
//   }
//   x_goal = goal;
//   partial_state c_open(goal);
//   partial_state n_open;
//   while (!c_open.empty()) {
//     n_open.clear();
//     for (index_type k = 0; k < c_open.length(); k++) {
//       index_type var = c_open[k].first;
//       index_type goal_val = c_open[k].second;
//       index_type init_val = init.value_of(var);
//       if (init_val != no_such_index) {
// 	const partial_state& sc = variables[var].nsc[init_val][goal_val];
// 	if (trace_level > 2) {
// 	  std::cerr << "necessary transition: " << variables[var].name
// 		    << " = " << variables[var].domain[init_val]
// 		    << " -> " << variables[var].domain[goal_val]
// 		    << ", nsc = ";
// 	  write_partial_state(std::cerr, sc);
// 	  std::cerr << std::endl;
// 	}
// 	for (index_type i = 0; i < sc.length(); i++) {
// 	  if (!x_goal.contains(sc[i])) {
// 	    if (trace_level > 2) {
// 	      std::cerr << "condition " << variables[sc[i].first].name
// 			<< " = " << variables[sc[i].first].domain[sc[i].second]
// 			<< " added to extended goal/open" << std::endl;
// 	    }
// 	    x_goal.insert(sc[i]);
// 	    n_open.append(sc[i]);
// 	  }
// 	  else if (trace_level > 2) {
// 	    std::cerr << "condition " << variables[sc[i].first].name
// 		      << " = " << variables[sc[i].first].domain[sc[i].second]
// 		      << " already in extended goal" << std::endl;
// 	  }
// 	}
//       }
//       else if (trace_level > 2) {
// 	std::cerr << "extend_nsc: variable ";
// 	variables[var].name->write(std::cerr, Name::NC_INSTANCE);
// 	std::cerr << " undefined in init" << std::endl;
//       }
//     }
//     c_open = n_open;
//     if (trace_level > 2) {
//       std::cerr << "extend_nsc: extended goal = ";
//       write_partial_state(std::cerr, x_goal);
//       std::cerr << ", open = ";
//       write_partial_state(std::cerr, c_open);
//       std::cerr << " at end of iteration" << std::endl;
//     }
//   }
// }

SASInstance::determined_check_result SASInstance::weak_determined_check
(const index_set& set, index_type var) const
{
  assert(atom_map_defined());
  if (trace_level > 1) {
    std::cerr << "checking if ";
    variables[var].name->write(std::cerr, Name::NC_INSTANCE);
    std::cerr << " is determined by ";
    write_variable_set(std::cerr, set);
    std::cerr << "..." << std::endl;
  }
  index_type n_acc = 0;
  index_type non_implied_value = no_such_index;
  for (index_type k = 0; k < variables[var].s_index.length(); k++) {
    index_type p = variables[var].s_index[k];
    if (atom_map[p].defines_any(set)) {
      if (trace_level > 2) {
	std::cerr << " - value ";
	variables[var].domain[k]->write(std::cerr, Name::NC_INSTANCE);
	std::cerr << " implied by ";
	write_partial_state(std::cerr, atom_map[p]);
	std::cerr << std::endl;
      }
      n_acc += 1;
    }
    else {
      if (trace_level > 2) {
	std::cerr << " - value ";
	variables[var].domain[k]->write(std::cerr, Name::NC_INSTANCE);
	std::cerr << " not implied" << std::endl;
      }
      non_implied_value = k;
    }
  }
  index_type n_not_acc = (variables[var].s_index.length() - n_acc);
  if (n_not_acc == 0) {
    if (variables[var].n_values() > variables[var].s_index.length()) {
      if (goal_state.value_of(var) == (variables[var].n_values() - 1)) {
	if (trace_level > 1) {
	  std::cerr << " - ok, but not goal-preserving (1)" << std::endl;
	}
	return determined_yes;
      }
      else {
	if (trace_level > 1) {
	  std::cerr << " - ok, all except non-atom value implied" << std::endl;
	}
	return determined_goal_preserving;
      }
    }
    else {
      if (trace_level > 1) {
	std::cerr << " - ok (strict)" << std::endl;
      }
      return determined_strict;
    }
  }
  else if (n_not_acc == 1) {
    if (variables[var].n_values() > variables[var].s_index.length()) {
      if (trace_level > 1) {
	std::cerr << " - variable has non-atom value and non-implied value"
		  << std::endl;
      }
      return determined_no;
    }
    if (additional_strictness_check) {
      if (trace_level > 1) {
	std::cerr << " - non-strictly determined, making additional check..."
		  << std::endl;
      }
      assert(non_implied_value < variables[var].s_index.length());
      PartialStateEnumerator pe(set, signature);
      bool more = pe.first();
      index_type n_non_implied = 0;
      while (more && (n_non_implied <= 1)) {
	partial_state sx;
	extend_determined(pe.current_state(), sx);
	index_type implied_value = sx.value_of(var);
	if (implied_value == non_implied_value) {
	  std::cerr << "error: non-implied value ";
	  variables[var].domain[non_implied_value]->write(std::cerr, Name::NC_INSTANCE);
	  std::cerr << " of ";
	  variables[var].name->write(std::cerr, Name::NC_INSTANCE);
	  std::cerr << " is implied by ";
	  write_partial_state(std::cerr, pe.current_state());
	  std::cerr << std::endl;
	  exit(255);
	}
	if (!sx.consistent()) {
	  if (trace_level > 2) {
	    std::cerr << "  - assignment ";
	    write_partial_state(std::cerr, pe.current_state());
	    std::cerr << " is inconsistent" << std::endl;
	  }
	}
	else if (implied_value == no_such_index) {
	  if (trace_level > 2) {
	    std::cerr << "  - assignment ";
	    write_partial_state(std::cerr, pe.current_state());
	    std::cerr << " implies no value for ";
	    variables[var].name->write(std::cerr, Name::NC_INSTANCE);
	    std::cerr << std::endl;
	  }
	  n_non_implied += 1;
	}
	else if (trace_level > 2) {
	  std::cerr << "  - assignment ";
	  write_partial_state(std::cerr, pe.current_state());
	  std::cerr << " implies ";
	  variables[var].name->write(std::cerr, Name::NC_INSTANCE);
	  std::cerr << " = ";
	  variables[var].domain[implied_value]->write(std::cerr, Name::NC_INSTANCE);
	  std::cerr << std::endl;
	}
	more = pe.next();
      }
      if (n_non_implied <= 1) {
	if (trace_level > 1) {
	  std::cerr << " - ok (strict)" << std::endl;
	}
	return determined_strict;
      }
    }
    if (goal_state.value_of(var) == non_implied_value) {
      if (trace_level > 1) {
	std::cerr << " - ok, but not goal-value-preserving (2)" << std::endl;
      }
      return determined_yes;
    }
    else {
      if (trace_level > 1) {
	std::cerr << " - ok (goal-preserving, non-strict)" << std::endl;
      }
      return determined_goal_preserving;
    }
  }
  else {
    if (trace_level > 1) {
      std::cerr << " - " << n_not_acc << " values not accounted for"
		<< std::endl;
    }
    return determined_no;
  }
}

SASInstance::determined_check_result SASInstance::determining_set
(index_type var, index_set& set) const
{
  assert(atom_map_defined());
  index_set det;
  index_type n_acc = 0;
  set.clear();
  for (index_type k = 0; k < variables[var].s_index.length(); k++) {
    index_type p = variables[var].s_index[k];
    det.clear();
    atom_map[p].defined_set(det);
    set.insert(det);
  }
  set.subtract(var);
  return weak_determined_check(set, var);
}

SASInstance::determined_check_result SASInstance::minimal_determining_sets
(index_type var, index_set_vec& sets) const
{
  index_set base;
  determined_check_result res = determining_set(var, base);
  if (res == determined_no) return determined_no;
  SubsetEnumerator se(base.length());
  bool more = se.first();
  while (more) {
    index_set ss;
    se.current_set(base, ss);
    determined_check_result ss_res = weak_determined_check(ss, var);
    if ((ss_res == determined_strict) && (res != determined_strict)) {
      res = determined_strict;
      sets.clear();
    }
    else if ((ss_res == determined_goal_preserving) &&
	     (res != determined_goal_preserving)) {
      res = determined_goal_preserving;
      sets.clear();
    }
    if (ss_res == res) {
      sets.insert_minimal(ss);
    }
    more = se.next();
  }
  return res;
}

SASInstance::determined_check_result SASInstance::smallest_determining_set
(index_type var, index_set& set) const
{
  index_set base;
  determined_check_result res = determining_set(var, base);
  if (res == determined_no) return determined_no;
  set = base;
  SubsetEnumerator se(base.length());
  bool more = se.first();
  while (more) {
    index_set ss;
    se.current_set(base, ss);
    determined_check_result ss_res = weak_determined_check(ss, var);
    if ((ss_res == determined_strict) && (res != determined_strict)) {
      res = determined_strict;
      set = ss;
    }
    else if ((ss_res == determined_goal_preserving) &&
	     (res != determined_goal_preserving)) {
      res = determined_goal_preserving;
      set = ss;
    }
    else if (ss_res == res) {
      if (ss.length() < set.length()) set = ss;
    }
    more = se.next();
  }
  return res;
}

void SASInstance::extend_determined
(const partial_state& s, partial_state& x) const
{
  x = s;
  for (index_type k = 0; k < s.length(); k++) {
    index_type var = s[k].first;
    index_type val = s[k].second;
    if (val < variables[var].s_index.length()) {
      index_type p = variables[var].s_index[val];
      map_to_partial_state(p, x);
    }
  }
  if (trace_level > 2) {
    std::cerr << "info: state ";
    write_partial_state(std::cerr, s);
    std::cerr << " extends to ";
    write_partial_state(std::cerr, x);
    std::cerr << std::endl;
  }
}

bool SASInstance::extended_consistent
(const partial_state& s0, const partial_state& s1) const
{
  partial_state x0;
  extend_determined(s0, x0);
  partial_state x1;
  extend_determined(s1, x1);
  x0.insert(x1);
  return x0.consistent();
}

bool SASInstance::check_P() const
{
  for (index_type i = 0; i < n_actions(); i++)
    for (index_type j = i+1; j < n_actions(); j++) {
      index_pair p = actions[i].post.first_common(actions[j].post);
      if (p != index_pair(no_such_index, no_such_index)) {
	if (trace_level > 1) {
	  std::cerr << "info: check P failed: actions "
		    << actions[i].name << " and " << actions[j].name
		    << " have common postcondition "
		    << variables[actions[i].post[p.first].first].name << "="
		    << variables[actions[i].post[p.first].first].domain
	    [actions[i].post[p.first].second] << std::endl;
	}
	return false;
      }
    }
  return true;
}

bool SASInstance::check_U() const
{
  for (index_type k = 0; k < n_actions(); k++)
    if (actions[k].post.length() > 1) {
      if (trace_level > 1) {
	std::cerr << "info: check U failed: action "
		  << actions[k].name << " has postcondition size "
		  << actions[k].post.length() << std::endl;
      }
      return false;
    }
  return true;
}

bool SASInstance::check_B() const
{
  for (index_type k = 0; k < n_variables(); k++)
    if (variables[k].n_values() > 2) {
      if (trace_level > 1) {
	std::cerr << "info: check B failed: variable "
		  << variables[k].name << " has domain size "
		  << variables[k].n_values() << std::endl;
      }
      return false;
    }
  return true;
}

bool SASInstance::check_S() const
{
  index_set_vec prv_vals(EMPTYSET, n_variables());
  for (index_type k = 0; k < n_actions(); k++)
    for (index_type i = 0; i < actions[k].prv.length(); i++)
      prv_vals[actions[k].prv[i].first].insert(actions[k].prv[i].second);
  for (index_type k = 0; k < n_variables(); k++)
    if (prv_vals[k].length() > 1) {
      if (trace_level > 1) {
	std::cerr << "info: check S failed: variable "
		  << variables[k].name << " has " << prv_vals[k].length()
		  << " prevail values" << std::endl;
      }
      return false;
    }
  return true;
}

bool SASInstance::commutative
(const SASInstance::Action& a0, const SASInstance::Action& a1)
{
  if (!a0.post.consistent_with(a1.post))
    return false; // a0 destroys an effect of a1, or vice versa
  if ((a0.post.count_common(a1.pre) > 0) ||
      (a0.post.count_common(a1.prv) > 0))
    return false; // a0 adds a pre- or prevail-condition of a1
  if ((a1.post.count_common(a0.pre) > 0) ||
      (a1.post.count_common(a0.prv) > 0))
    return false; // a1 adds a pre- or prevail-condition of a0
  return true;
}

bool SASInstance::commutative(index_type a0, index_type a1)
{
  return commutative(actions[a0], actions[a1]);
}

bool SASInstance::additive(const index_set& v0, const index_set& v1) const
{
  if (v0.count_common(v1) > 0) return false;
  for (index_type k = 0; k < n_actions(); k++)
    if (actions[k].post.defines_any(v0) &&
	actions[k].post.defines_any(v1))
      return false;
  return true;
}

bool SASInstance::contributes_to
(const Action& a0, const Action& a1, partial_state& i) const
{
  partial_state e0(a0.post);
  e0.insert(a0.prv);
  partial_state e1(a1.pre);
  e1.insert(a1.prv);

  if (!extended_consistent(e0, e1)) return false;
  i.assign_copy(a0.post);
  i.intersect(e1);
  if (i.empty()) return false;
  return true;
}

void SASInstance::replace_determined_variable_in_condition
(index_type var, const index_set& det, const partial_state& cond,
 partial_state_vec& repl) const
{
  assert(atom_map_defined());
  if (trace_level > 1) {
    std::cerr << "replacing variable ";
    write_variable(std::cerr, variables[var]);
    std::cerr << " determined by ";
    write_variable_set(std::cerr, det);
    std::cerr << " in ";
    write_partial_state(std::cerr, cond);
    std::cerr << "..." << std::endl;
  }

  repl.clear();
  if (!cond.consistent()) {
    if (trace_level > 1) {
      std::cerr << "condition ";
      write_partial_state(std::cerr, cond);
      std::cerr << " is not consistent!" << std::endl;
    }
    return;
  }

  index_type v = cond.value_of(var);

  // var does not occur in cond: replacement condition is the same
  if (v == no_such_index) {
    if (trace_level > 1) {
      std::cerr << "variable ";
      variables[var].name->write(std::cerr, false);
      std::cerr << " does not appear in condition ";
      write_partial_state(std::cerr, cond);
      std::cerr << std::endl;
    }
    repl.append(cond);
    return;
  }

  if (v >= variables[var].s_index.length()) {
    if (trace_level > 1) {
      std::cerr << "variable ";
      variables[var].name->write(std::cerr, false);
      std::cerr << " can not be determined: ";
      variables[var].domain[v]->write(std::cerr, false);
      std::cerr << " is a non-atom value" << std::endl;
    }
    return;
  }

  partial_state v_def(atom_map[variables[var].s_index[v]], det);
  // v is implied (by v_def)
  if (!v_def.empty()) {
    if (trace_level > 2) {
      std::cerr << "condition ";
      variables[var].name->write(std::cerr, false);
      std::cerr << " = ";
      variables[var].domain[v]->write(std::cerr, false);
      std::cerr << " replaced by ";
      write_partial_state(std::cerr, v_def);
      std::cerr << std::endl;
    }
    partial_state new_cond(cond);
    new_cond.undefine(var);
    new_cond.insert(v_def);
    if (new_cond.consistent()) {
      repl.append(new_cond);
    }
    else {
      std::cerr << "warning: replacing ";
      variables[var].name->write(std::cerr, false);
      std::cerr << " = ";
      variables[var].domain[v]->write(std::cerr, false);
      std::cerr << " by ";
      write_partial_state(std::cerr, v_def);
      std::cerr << " not consistent with ";
      write_partial_state(std::cerr, cond);
      std::cerr << std::endl;
    }
  }
  // v is the non-implied value
  else {
    if (trace_level > 2) {
      std::cerr << "condition ";
      variables[var].name->write(std::cerr, false);
      std::cerr << " = ";
      variables[var].domain[v]->write(std::cerr, false);
      std::cerr << " is non-implied..." << std::endl;
    }
    index_set decided;
    cond.defined_set(decided);
    index_set varying(det);
    varying.subtract(decided);
    PartialStateEnumerator e(varying, signature);
    bool more = e.first();
    while (more) {
      partial_state x;
      extend_determined(e.current_state(), x);
      if (!x.defines(var)) {
	if (trace_level > 2) {
	  std::cerr << "...found assignment ";
	  write_partial_state(std::cerr, e.current_state());
	  std::cerr << std::endl;
	}
	partial_state new_cond(cond);
	new_cond.undefine(var);
	new_cond.insert(v_def);
	if (new_cond.consistent()) {
	  repl.append(new_cond);
	}
	else {
	  std::cerr << "warning: replacing ";
	  variables[var].name->write(std::cerr, false);
	  std::cerr << " = ";
	  variables[var].domain[v]->write(std::cerr, false);
	  std::cerr << " by ";
	  write_partial_state(std::cerr, v_def);
	  std::cerr << " not consistent with ";
	  write_partial_state(std::cerr, cond);
	  std::cerr << std::endl;
	}
      }
      more = e.next();
    }
  }
}

void SASInstance::new_action_with_replacement_condition
(const SASInstance::Action& act, const partial_state& c,
 index_type var, index_type val)
{
  if (act.pre.consistent_with(c) && act.prv.consistent_with(c)) {
    Action& a = new_action(act);
    a.prv.insert(c);
    a.prv.subtract(act.pre);
  }
  else {
    std::cerr << "warning: replacing ";
    variables[var].name->write(std::cerr, false);
    std::cerr << " = ";
    variables[var].domain[val]->write(std::cerr, false);
    std::cerr << " by ";
    write_partial_state(std::cerr, c);
    std::cerr << " not consistent with conditions ";
    write_partial_state(std::cerr, act.pre);
    std::cerr << " / ";
    write_partial_state(std::cerr, act.prv);
    std::cerr << " of action ";
    act.name->write(std::cerr, false);
    std::cerr << std::endl;
  }
}

bool SASInstance::replace_determined_variable
(index_type var, const index_set& det)
{
  assert(atom_map_defined());
  if (variables[var].n_values() > variables[var].s_index.length()) {
    std::cerr << "error: variable ";
    write_variable(std::cerr, variables[var]);
    std::cerr << " can not be determined by ";
    write_variable_set(std::cerr, det);
    std::cerr << " since it has a default value"
	      << std::endl;
    exit(255);
  }
  action_vec original(actions);
  actions.set_length(0);
  if (trace_level > 1) {
    std::cerr << "replacing variable ";
    write_variable(std::cerr, variables[var]);
    std::cerr << " determined by ";
    write_variable_set(std::cerr, det);
    std::cerr << " (" << original.length() << " actions)..." << std::endl;
  }
  for (index_type k = 0; k < original.length(); k++) {
    index_type v_pre = original[k].pre.value_of(var);
    index_type v_prv = original[k].prv.value_of(var);
    if (v_pre != no_such_index) {
      if (trace_level > 2) {
	std::cerr << "replacing variable ";
	variables[var].name->write(std::cerr, false);
	std::cerr << " in precondition of ";
	original[k].name->write(std::cerr, false);
	std::cerr << "..." << std::endl;
      }
      original[k].pre.undefine(var);
      original[k].post.undefine(var);
      partial_state v_def(atom_map[variables[var].s_index[v_pre]], det);
      if (!v_def.empty()) { // v_pre is implied
	if (trace_level > 2) {
	  std::cerr << "condition ";
	  variables[var].name->write(std::cerr, false);
	  std::cerr << " = ";
	  variables[var].domain[v_pre]->write(std::cerr, false);
	  std::cerr << " replaced by ";
	  write_partial_state(std::cerr, v_def);
	  std::cerr << std::endl;
	}
	new_action_with_replacement_condition(original[k], v_def, var, v_pre);
      }
      else { // v_pre is the non-implied value
	if (trace_level > 2) {
	  std::cerr << "condition ";
	  variables[var].name->write(std::cerr, false);
	  std::cerr << " = ";
	  variables[var].domain[v_pre]->write(std::cerr, false);
	  std::cerr << " is non-implied..." << std::endl;
	}
	index_set decided;
	original[k].pre.defined_set(decided);
	original[k].prv.defined_set(decided);
	index_set varying(det);
	varying.subtract(decided);
	PartialStateEnumerator e(varying, signature);
	bool more = e.first();
	while (more) {
	  partial_state x;
	  extend_determined(e.current_state(), x);
	  if (!x.defines(var)) {
	    if (trace_level > 2) {
	      std::cerr << "...found assignment ";
	      write_partial_state(std::cerr, e.current_state());
	      std::cerr << std::endl;
	    }
	    new_action_with_replacement_condition
	      (original[k], e.current_state(), var, v_pre);
	  }
	  more = e.next();
	}
      }
    }
    else if (v_prv != no_such_index) {
      if (trace_level > 2) {
	std::cerr << "replacing variable ";
	variables[var].name->write(std::cerr, false);
	std::cerr << " in prevail condition of ";
	original[k].name->write(std::cerr, false);
	std::cerr << "..." << std::endl;
      }
      original[k].prv.undefine(var);
      partial_state v_def(atom_map[variables[var].s_index[v_prv]], det);
      if (!v_def.empty()) { // v_prv is implied
	if (trace_level > 2) {
	  std::cerr << "condition ";
	  variables[var].name->write(std::cerr, false);
	  std::cerr << " = ";
	  variables[var].domain[v_prv]->write(std::cerr, false);
	  std::cerr << " replaced by ";
	  write_partial_state(std::cerr, v_def);
	  std::cerr << std::endl;
	}
	new_action_with_replacement_condition(original[k], v_def, var, v_prv);
      }
      else { // v_prv is the non-implied value
	if (trace_level > 2) {
	  std::cerr << "condition ";
	  variables[var].name->write(std::cerr, false);
	  std::cerr << " = ";
	  variables[var].domain[v_prv]->write(std::cerr, false);
	  std::cerr << " is non-implied..." << std::endl;
	}
	index_set decided;
	original[k].pre.defined_set(decided);
	original[k].prv.defined_set(decided);
	index_set varying(det);
	varying.subtract(decided);
	PartialStateEnumerator e(varying, signature);
	bool more = e.first();
	while (more) {
	  partial_state x;
	  extend_determined(e.current_state(), x);
	  if (!x.defines(var)) {
	    if (trace_level > 2) {
	      std::cerr << "...found assignment ";
	      write_partial_state(std::cerr, e.current_state());
	      std::cerr << std::endl;
	    }
	    new_action_with_replacement_condition
	      (original[k], e.current_state(), var, v_prv);
	  }
	  more = e.next();
	}
      }
    }
    else {
      Action& a = new_action(original[k]);
    }
  }

  partial_state_vec new_goal_state;
  replace_determined_variable_in_condition
    (var, det, goal_state, new_goal_state);
  if (new_goal_state.length() == 0) {
    std::cerr << "error: no consistent replacement for ";
    variables[var].name->write(std::cerr, false);
    std::cerr << " in goal condition ";
    write_partial_state(std::cerr, goal_state);
    std::cerr << std::endl;
    return false;
  }
  else if (new_goal_state.length() > 1) {
    std::cerr << "error: replacement for ";
    variables[var].name->write(std::cerr, false);
    std::cerr << " in goal condition ";
    write_partial_state(std::cerr, goal_state);
    std::cerr << " is disjunctive: ";
    for (index_type k = 0; k < new_goal_state.length(); k++) {
      if (k > 0) std::cerr << " OR ";
      write_partial_state(std::cerr, new_goal_state[k]);
    }
    std::cerr << std::endl;
    return false;
  }
  else {
    goal_state = new_goal_state[0];
    return true;
  }
}

bool SASInstance::is_spanning(const index_set& set) const
{
  for (index_type k = 0; k < n_variables(); k++)
    if (!set.contains(k)) {
      index_set_vec dets;
      SASInstance::determined_check_result res =
	minimal_determining_sets(k, dets);
      if ((res == SASInstance::determined_yes) ||
	  (res == SASInstance::determined_goal_preserving) ||
	  (res == SASInstance::determined_strict)) {
	bool ok = false;
	for (index_type i = 0; (i < dets.length()) && !ok; i++) {
	  if (set.contains(dets[i])) {
	    partial_state_vec new_goal_state;
	    replace_determined_variable_in_condition
	      (k, dets[i], goal_state, new_goal_state);
	    // if determined check result is goal-preserving or strict,
	    // the new goal should not be disjunctive!
	    if ((res == SASInstance::determined_goal_preserving) ||
		(res == SASInstance::determined_strict))
	      assert(new_goal_state.length() == 1);
	    if (new_goal_state.length() == 1) {
	      ok = true;
	    }
	  }
	}
	if (!ok) {
	  return false;
	}
      }
      else {
	return false;
      }
    }
  return true;
}

SASInstance* SASInstance::reduce_to_spanning(const index_set& set) const
{
  SASInstance* d_ins = copy();

  for (index_type k = 0; k < n_variables(); k++)
    if (!set.contains(k)) {
      index_set_vec dets;
      SASInstance::determined_check_result res =
	minimal_determining_sets(k, dets);
      if ((res == SASInstance::determined_yes) ||
	  (res == SASInstance::determined_goal_preserving) ||
	  (res == SASInstance::determined_strict)) {
	bool ok = false;
	for (index_type i = 0; (i < dets.length()) && !ok; i++) {
	  if (set.contains(dets[i])) {
	    partial_state_vec new_goal_state;
	    d_ins->replace_determined_variable_in_condition
	      (k, dets[i], d_ins->goal_state, new_goal_state);
	    if (new_goal_state.length() == 1) {
	      ok = d_ins->replace_determined_variable(k, dets[i]);
	      if (!ok) {
		std::cerr << "error: replacing determined variable ";
		variables[k].name->write(std::cerr, false);
		std::cerr << " with ";
		write_variable_set(std::cerr, dets[i]);
		std::cerr << " produced inconsistent/disjunctive goal state"
			  << std::endl;
		exit(255);
	      }
	    }
	  }
	}
	if (!ok) {
	  std::cerr << "error: set ";
	  write_variable_set(std::cerr, set);
	  std::cerr << " is not spanning - no determining set found for "
		    << variables[k].name
		    << std::endl;
	  delete d_ins;
	  return 0;
	}
      }
      else {
	std::cerr << "error: set ";
	write_variable_set(std::cerr, set);
	std::cerr << " is not spanning - variable " << variables[k].name
		  << " is not determined"
		  << std::endl;
	delete d_ins;
	return 0;
      }
    }

  SASInstance* r_ins = d_ins->reduce(set);
  delete d_ins;
  return r_ins;
}

bool SASInstance::is_safe(bool_vec& sa)
{
  sa.assign_value(true, n_actions());
  for (index_type k = 0; k < n_actions(); k++)
    for (index_type i = 0; i < actions[k].post.length(); i++) {
      index_type v = actions[k].post[i].first;
      index_type v_post = actions[k].post[i].second;
      index_type v_pre = actions[k].pre.value_of(v);
      if (v_pre == no_such_index)
	sa[k] = false;
    }
  return (sa.count(false) > 0);
}

void SASInstance::relevant_actions(index_type v, index_set& acts)
{
  acts.clear();
  for (index_type k = 0; k < n_actions(); k++)
    if (actions[k].post.defines(v)) acts.insert(k);
}

rational SASInstance::interference_ratio
(index_type var0, index_type val0, index_type var1, index_type val1)
{
  partial_state s;
  s.assign(var0, val0);
  s.assign(var1, val1);
  partial_state xs;
  extend_determined(s, xs);
  if (!xs.consistent()) {
    std::cerr << "warning: state ";
    write_partial_state(std::cerr, s);
    std::cerr << " extends to ";
    write_partial_state(std::cerr, xs);
    std::cerr << " which is inconsistent" << std::endl;
    return rational::infinity(1);
  }
  index_type app0 = 0;
  index_type co_app0 = 0;
  index_type app1 = 0;
  index_type co_app1 = 0;
  for (index_type k = 0; k < n_actions(); k++) {
    if (actions[k].pre.value_of(var0) == val0) {
      app0 += 1;
      if (xs.consistent_with(actions[k].pre) &&
	  xs.consistent_with(actions[k].prv))
	co_app0 += 1;
    }
    if (actions[k].pre.value_of(var1) == val1) {
      app1 += 1;
      if (xs.consistent_with(actions[k].pre) &&
	  xs.consistent_with(actions[k].prv))
	co_app1 += 1;
    }
  }
  if ((app0 + app1) == 0) return 0;
  rational r(co_app0 + co_app1, app0 + app1);
  assert(0 <= r);
  assert(r <= 1);
  return 1 - r;
}

void SASInstance::write_partial_state
(std::ostream& s, const partial_state& v) const
{
  for (index_type k = 0; k < v.length(); k++) {
    if (k > 0) s << ", ";
    if (v[k].first >= n_variables()) {
      s << v[k].first << "?" << " = " << v[k].second << "?";
    }
    else if (v[k].second >= variables[v[k].first].n_values()) {
      if (write_symbolic_names)
	variables[v[k].first].name->write(s, Name::NC_INSTANCE);
      else
	s << v[k].first;
      s << " = " << v[k].second << "?";
    }
    else {
      if (write_symbolic_names)
	variables[v[k].first].name->write(s, Name::NC_INSTANCE);
      else
	s << v[k].first;
      s << " = ";
      if (write_symbolic_names)
	variables[v[k].first].domain[v[k].second]->write(s, Name::NC_INSTANCE);
      else
	s << v[k].second;
    }
  }
}

void SASInstance::write_partial_state_sequence
(std::ostream& s, const partial_state_vec& v) const
{
  s << "<";
  for (index_type k = 0; k < v.length(); k++) {
    if (k > 0) s << ", ";
    s << "{";
    write_partial_state(s, v[k]);
    s << "}";
  }
  s << ">";
}

void SASInstance::write_variable
(std::ostream& s, const Variable& var) const
{
  if (write_symbolic_names)
    var.name->write(s, Name::NC_INSTANCE);
  else
    s << var.index;
  s << ": ";
  for (index_type k = 0; k < var.domain.length(); k++) {
    if (k > 0) s << ", ";
    var.domain[k]->write(s, Name::NC_INSTANCE);
    if (write_variable_relevance) {
      if (var.relevant[k]) {
	s << "+";
      }
      else {
	s << "-";
      }
    }
  }
  s << ";";
  if (write_variable_sources) {
    s << " {sources: ";
    for (index_type k = 0; k < var.s_index.length(); k++) {
      if (k > 0) s << ", ";
      s << var.s_index[k];
    }
    s << "}";
  }
}

void SASInstance::write_variable_info
(std::ostream& s, const Variable& var) const
{
  s << "{sources: ";
  for (index_type k = 0; k < var.s_index.length(); k++) {
    if (k > 0) s << ", ";
    s << var.s_index[k];
  }
  s << "}" << std::endl << "{initial value: ";
  if (init_state.value_of(var.index) != no_such_index) {
    var.domain[init_state.value_of(var.index)]->write(s, Name::NC_INSTANCE);
  }
  s << ", goal value: ";
  if (goal_state.value_of(var.index) != no_such_index) {
    var.domain[goal_state.value_of(var.index)]->write(s, Name::NC_INSTANCE);
  }
  s << "}" << std::endl << "{required values: ";
  for (index_type k = 0; k < var.enabling_values.length(); k++) {
    if (k > 0) s << ", ";
    var.domain[var.enabling_values[k]]->write(s, Name::NC_INSTANCE);
  }
  s << "}" << std::endl << "{caused values: ";
  for (index_type k = 0; k < var.accidental_values.length(); k++) {
    if (k > 0) s << ", ";
    var.domain[var.accidental_values[k]]->write(s, Name::NC_INSTANCE);
  }
//   s << "}" << std::endl << "{domain transitions: ";
//   for (index_type i = 0; i < var.domain.length(); i++)
//     for (index_type j = 0; j < var.domain.length(); j++) if (i != j) {
//       s << std::endl << "  ";
//       var.domain[i]->write(s, Name::NC_INSTANCE);
//       s << " -> ";
//       var.domain[j]->write(s, Name::NC_INSTANCE);
//       s << ": " << var.dtcost[i][j] << " / ";
//       write_partial_state(s, var.nsc[i][j]);
//       s << " / ";
//       write_partial_state(s, var.nse[i][j]);
//     }
  s << " }" << std::endl;
}

void SASInstance::write_action
(std::ostream& s, const Action& act) const
{
  if (write_symbolic_names)
    act.name->write(s, Name::NC_INSTANCE);
  else
    s << act.index;
  s << ": ";
  write_partial_state(s, act.pre);
  s << "; ";
  write_partial_state(s, act.post);
  s << "; ";
  write_partial_state(s, act.prv);
  s << ";";
  if (write_action_cost) {
    s << " " << act.cost << ";";
  }
  if (write_action_sources) {
    s << " {sources: ";
    for (index_type k = 0; k < act.s_index.length(); k++) {
      if (k > 0) s << ", ";
      s << act.s_index[k];
    }
    s << "}";
  }
}

void SASInstance::write_action_info
(std::ostream& s, const Action& act) const
{
  s << "{undefined pre: ";
  for (index_type k = 0; k < act.u_pre.length(); k++) {
    if (k > 0) s << ", ";
    variables[act.u_pre[k]].name->write(s, Name::NC_INSTANCE);
  }
  s << "}" << std::endl;
}

void SASInstance::write_domain
(std::ostream& s) const
{
  s << "variables:" << std::endl;
  for (index_type k = 0; k < n_variables(); k++) {
    s << " ";
    write_variable(s, variables[k]);
    s << std::endl;
    if (write_info_in_domain) {
      write_variable_info(s, variables[k]);
    }
  }
  s << "operators:" << std::endl;
  for (index_type k = 0; k < n_actions(); k++) {
    s << " ";
    write_action(s, actions[k]);
    s << std::endl;
    if (write_info_in_domain) {
      write_action_info(s, actions[k]);
    }
  }
  s << "initial: ";
  write_partial_state(s, init_state);
  s << ";" << std::endl;
  s << "goal: ";
  write_partial_state(s, goal_state);
  s << ";" << std::endl;
}

void SASInstance::write_variable_set
(std::ostream& s, const index_set& set) const
{
  s << "{";
  for (index_type k = 0; k < set.length(); k++) {
    if (k > 0) s << ", ";
    variables[set[k]].name->write(s, Name::NC_INSTANCE);
  }
  s << "}";
}

void SASInstance::write_value_set
(std::ostream& s, index_type var, const index_set& set) const
{
  s << "{";
  for (index_type k = 0; k < set.length(); k++) {
    if (k > 0) s << ", ";
    variables[var].domain[set[k]]->write(s, Name::NC_INSTANCE);
  }
  s << "}";
}

void SASInstance::write_action_set
(std::ostream& s, const index_set& set) const
{
  s << "{";
  for (index_type k = 0; k < set.length(); k++) {
    if (k > 0) s << ", ";
    actions[set[k]].name->write(s, Name::NC_INSTANCE);
  }
  s << "}";
}

void SASInstance::write_action_sequence
(std::ostream& s, const index_vec& seq) const
{
  for (index_type k = 0; k < seq.length(); k++) {
    if (k > 0) s << ", ";
    actions[seq[k]].name->write(s, Name::NC_INSTANCE);
  }
}

void SASInstance::random_partial_state
(partial_state& s, index_type n_avg, RNG& rng) const
{
  s.clear();
  for (index_type k = 0; k < n_variables(); k++)
    if ((rng.random() % n_variables()) < n_avg) {
      s.assign(k, rng.random() % variables[k].n_values());
    }
  if (s.empty()) {
    index_type v = rng.random() % n_variables();
    s.assign(v, rng.random() % variables[v].n_values());
  }
}

void SASInstance::write_domain_transition_graph
(std::ostream& s,
 index_type var,
 label_style ls,
 bool with_loops,
 bool with_act_nodes,
 bool as_subgraph) const
{
  if (as_subgraph) {
    s << "subgraph cluster_DTG" << var << " {" << std::endl;
  }
  else {
    s << "digraph DTG" << var << " {" << std::endl;
    s << "rankdir=LR;" << std::endl;
  }
  s << "node [shape=ellipse,width=0,height=0];" << std::endl;
  if (ls != ls_none) {
    s << "edge [len=2.0];" << std::endl;
  }

  s << "label=\"";
  variables[var].name->write(s, Name::NC_INSTANCE);
  s << "\";" << std::endl;

  index_type v_init = init_state.value_of(var);
  assert(v_init != no_such_index);

  graph g(variables[var].n_values());
  for (index_type i = 0; i < variables[var].n_values(); i++)
    for (index_type j = 0; j < variables[var].n_values(); j++) if (i != j) {
      bool has_edge = false;
      for (index_type k = 0; (k < n_actions()) && !has_edge; k++)
	if (((actions[k].pre.value_of(var) == i) ||
	     (actions[k].pre.value_of(var) == no_such_index)) &&
	    (actions[k].post.value_of(var) == j))
	  has_edge = true;
      if (has_edge)
	g.add_edge(i, j);
    }
  index_vec d;
  g.distance(v_init, d);

  index_vec_util::increasing_value_order o(d);
  index_vec vo;
  for (index_type k = 0; k < variables[var].n_values(); k++)
    vo.insert_ordered(k, o);
  assert(vo.length() == variables[var].n_values());

  for (index_type k = 0; k < vo.length(); k++) {
    s << "V" << var << "_" << vo[k] << " [shape=box,";
    if (init_state.value_of(var) == vo[k]) {
      s << "style=filled,";
    }
    if (goal_state.value_of(var) == vo[k]) {
      s << "style=bold,";
    }
    s << "label=\"";
    variables[var].domain[vo[k]]->write(s, Name::NC_INSTANCE);
    s << "\",fontsize=10];" << std::endl;
  }

  for (index_type i = 0; i < variables[var].n_values(); i++)
    for (index_type j = 0; j < variables[var].n_values(); j++) {
      index_set acts;
      for (index_type k = 0; k < n_actions(); k++) {
	if (((actions[k].pre.value_of(var) == i) ||
	     (actions[k].pre.value_of(var) == no_such_index)) &&
	    (actions[k].post.value_of(var) == j))
	  acts.insert(k);
      }

      if (acts.length() > 0) {
	if (ls == ls_action_index_set) {
	  if (with_act_nodes) {
	    s << "A" << var << "_" << i << "_" << j
	      << " [shape=box,label=\"{";
	    for (index_type k = 0; k < acts.length(); k++) {
	      if (k > 0) s << ",";
	      s << acts[k];
	    }
	    s << "}\",fontsize=8];" << std::endl;
	    s << "V" << var << "_" << i
	      << " -> A" << var << "_" << i << "_" << j << ";" << std::endl;
	    s << "A" << var << "_" << i << "_" << j
	      << " -> V" << var << "_" << j << ";" << std::endl;
	  }
	  else {
	    s << "V" << var << "_" << i << " -> V" << var << "_" << j
	      << " [label=\"{";
	    for (index_type k = 0; k < acts.length(); k++) {
	      if (k > 0) s << ",";
	      s << acts[k];
	    }
	    s << "}\",fontsize=8];" << std::endl;
	  }
	}
	else if (ls == ls_action_name_set) {
	  s << "V" << var << "_" << i << " -> V" << var << "_" << j
	    << " [label=\"{";
	  for (index_type k = 0; k < acts.length(); k++) {
	    if (k > 0) s << ",";
	    actions[acts[k]].name->write(s, Name::NC_INSTANCE);
	  }
	  s << "}\",fontsize=8];" << std::endl;
	}
// 	else if (ls == ls_nec_side_constraints) {
// 	  s << "V" << var << "_" << i << " -> V" << var << "_" << j
// 	    << " [label=\"";
// 	  write_partial_state(s, variables[var].nsc[i][j]);
// 	  s << "\",fontsize=8];" << std::endl;
// 	}
//  	else if (ls == ls_nec_side_constraints_and_effects) {
// 	  s << "V" << var << "_" << i << " -> V" << var << "_" << j
// 	    << " [label=\"";
// 	  write_partial_state(s, variables[var].nsc[i][j]);
// 	  s << " / ";
// 	  if ((variables[var].nsc[i][j].length() > 0) &&
// 	      (variables[var].nse[i][j].length() > 0))
// 	    s << "\\n";
// 	  write_partial_state(s, variables[var].nse[i][j]);
// 	  s << "\",fontsize=8];" << std::endl;
// 	}
	else if (ls == ls_related_variables) {
	  equivalence eq(acts.length());
	  for (index_type ai = 0; ai < acts.length(); ai++) {
	    index_set ai_pre;
	    actions[acts[ai]].pre.defined_set(ai_pre);
	    actions[acts[ai]].prv.defined_set(ai_pre);
	    index_set ai_post;
	    actions[acts[ai]].post.defined_set(ai_post);
	    for (index_type aj = ai + 1; aj < acts.length(); aj++) {
	      index_set aj_post;
	      actions[acts[aj]].post.defined_set(aj_post);
	      if (aj_post == ai_post) {
		index_set aj_pre;
		actions[acts[aj]].pre.defined_set(aj_pre);
		actions[acts[aj]].prv.defined_set(aj_pre);
		if (aj_pre == ai_pre)
		  eq.merge(ai, aj);
	      }
	    }
	  }
	  index_set_vec g;
	  eq.classes(g);
	  for (index_type k = 0; k < g.length(); k++) {
	    s << "V" << var << "_" << i << " -> V" << var << "_" << j
	      << " [label=\"";
	    index_set g_pre;
	    actions[acts[g[k][0]]].pre.defined_set(g_pre);
	    actions[acts[g[k][0]]].prv.defined_set(g_pre);
	    g_pre.subtract(var);
	    write_variable_set(s, g_pre);
	    s << " / ";
	    index_set g_post;
	    actions[acts[g[k][0]]].post.defined_set(g_post);
	    g_post.subtract(var);
	    write_variable_set(s, g_post);
	    s << "\",fontsize=8];" << std::endl;
	  }
	}
	else if (ls == ls_action_name) {
	  for (index_type k = 0; k < acts.length(); k++) {
	    s << "V" << var << "_" << i << " -> V" << var << "_" << j
	      << " [label=\"";
	    actions[acts[k]].name->write(s, Name::NC_INSTANCE);
	    s << "\",fontsize=8];" << std::endl;
	  }
	}
	else if (ls == ls_prevail_conditions) {
	  for (index_type k = 0; k < acts.length(); k++) {
	    s << "V" << var << "_" << i << " -> V" << var << "_" << j
	      << " [label=\"";
	    write_partial_state(s, actions[acts[k]].prv);
	    s << "\",fontsize=8];" << std::endl;
	  }
	}
	else if (ls == ls_side_conditions_and_effects) {
	  for (index_type k = 0; k < acts.length(); k++) {
	    s << "V" << var << "_" << i << " -> V" << var << "_" << j
	      << " [label=\"";
	    partial_state side_pre(actions[acts[k]].pre);
	    side_pre.undefine(var);
	    partial_state side_post(actions[acts[k]].post);
	    side_post.undefine(var);
	    write_partial_state(s, side_pre);
	    s << " / ";
	    if ((side_pre.length() > 0) && (side_post.length() > 0))
	      s << "\\n";
	    write_partial_state(s, side_post);
	    s << " / ";
	    if (((side_pre.length() > 0) || (side_post.length() > 0)) &&
		(actions[acts[k]].prv.length() > 0))
	      s << "\\n";
	    write_partial_state(s, actions[acts[k]].prv);
	    s << "\",fontsize=8];" << std::endl;
	  }
	}
	else {
	  if (with_act_nodes) {
	    s << "A" << var << "_" << i << "_" << j
	      << " [shape=circle,width=0.25,label=\"\"];" << std::endl;
	    s << "V" << var << "_" << i
	      << " -> A" << var << "_" << i << "_" << j << ";" << std::endl;
	    s << "A" << var << "_" << i << "_" << j
	      << " -> V" << var << "_" << j << ";" << std::endl;
	  }
	  else {
	    s << "V" << var << "_" << i << " -> V" << var << "_" << j
	      << ";" << std::endl;
	  }
	}
      }
    }

  if (with_loops)
    for (index_type i = 0; i < variables[var].n_values(); i++) {
      index_set acts;
      for (index_type k = 0; k < n_actions(); k++)
	if (actions[k].prv.value_of(var) == i)
	  acts.insert(k);
      if (acts.length() > 0) {
	if (ls == ls_action_index_set) {
	  s << "V" << var << "_" << i << " -> V" << var << "_" << i
	    << " [label=\"{";
	  for (index_type k = 0; k < acts.length(); k++) {
	    if (k > 0) s << ",";
	    s << acts[k];
	  }
	  s << "}\",fontsize=8];" << std::endl;
	}
	else if (ls == ls_action_name_set) {
	  s << "V" << var << "_" << i << " -> V" << var << "_" << i
	    << " [label=\"{";
	  for (index_type k = 0; k < acts.length(); k++) {
	    if (k > 0) s << ",";
	    actions[acts[k]].name->write(s, Name::NC_INSTANCE);
	  }
	  s << "}\",fontsize=8];" << std::endl;
	}
	else if (ls == ls_action_name) {
	  for (index_type k = 0; k < acts.length(); k++) {
	    s << "V" << var << "_" << i << " -> V" << var << "_" << i
	      << " [label=\"";
	    actions[acts[k]].name->write(s, Name::NC_INSTANCE);
	    s << "\",fontsize=8];" << std::endl;
	  }
	}
	else if (ls == ls_side_conditions_and_effects) {
	  for (index_type k = 0; k < acts.length(); k++) {
	    s << "V" << var << "_" << i << " -> V" << var << "_" << i
	      << " [label=\"";
	    partial_state side_pre(actions[acts[k]].pre);
	    side_pre.undefine(var);
	    partial_state side_post(actions[acts[k]].post);
	    side_post.undefine(var);
	    write_partial_state(s, actions[acts[k]].pre);
	    s << " / ";
	    if ((actions[acts[k]].pre.length() > 0) &&
		(actions[acts[k]].post.length() > 0))
	      s << "\\n";
	    write_partial_state(s, actions[acts[k]].post);
	    s << "\",fontsize=8];" << std::endl;
	  }
	}
	else {
	  s << "V" << var << "_" << i << " -> V" << var << "_" << i
	    << ";" << std::endl;
	}
      }
    }

  s << "}" << std::endl;
}

void SASInstance::write_composite_transition_graph(std::ostream& s) const
{
  s << "digraph DTG {" << std::endl;
  s << "rankdir=LR;" << std::endl;

  for (index_type k = 0; k < n_variables(); k++) {
    write_domain_transition_graph(s, k, ls_none, false, true, true);
  }

  for (index_type k = 0; k < n_actions(); k++) {
    for (index_type i = 0; i < actions[k].post.length(); i++)
      for (index_type j = i + 1; j < actions[k].post.length(); j++) {
	index_type pre_i = actions[k].pre.value_of(actions[k].post[i].first);
	assert(pre_i != no_such_index);
	index_type pre_j = actions[k].pre.value_of(actions[k].post[j].first);
	assert(pre_j != no_such_index);
	s << "A" << actions[k].post[i].first << "_" << pre_i
	  << "_" << actions[k].post[i].second
	  << " -> A" << actions[k].post[j].first << "_" << pre_j
	  << "_" << actions[k].post[j].second
	  << " [dir=none,style=dashed];" << std::endl;
      }
    for (index_type i = 0; i < actions[k].post.length(); i++)
      for (index_type j = 0; j < actions[k].prv.length(); j++) {
	index_type pre_i = actions[k].pre.value_of(actions[k].post[i].first);
	assert(pre_i != no_such_index);
	s << "V" << actions[k].prv[j].first
	  << "_" << actions[k].prv[j].second
	  << " -> A" << actions[k].post[i].first
	  << "_" << pre_i << "_" << actions[k].post[i].second
	  << ";" << std::endl;
      }
  }

  s << "}" << std::endl;
}

// void SASInstance::write_composite_transition_graph(std::ostream& s) const
// {
//   s << "digraph DTG {" << std::endl;
//   // s << "rankdir=LR;" << std::endl;
//   s << "overlap=false;" << std::endl;
//   s << "splines=true;" << std::endl;
//   s << "sep=0.1;" << std::endl;
//   s << "node [width=0,height=0];" << std::endl;
//   s << "edge [len=1.0];" << std::endl;
//
//   for (index_type var = 0; var < n_variables(); var++) {
//     for (index_type val = 0; val < variables[var].n_values(); val++) {
//       s << "V" << var << "_" << val << " [shape=box,label=\"";
//       variables[var].name->write(s, Name::NC_INSTANCE);
//       s << "=";
//       variables[var].domain[val]->write(s, Name::NC_INSTANCE);
//       s << "\",fontsize=10];" << std::endl;
//     }
//   }
//
//   for (index_type act = 0; act < n_actions(); act++) {
//     s << "A" << act << " [shape=circle,label=\"" << act << "\"];"
//       << std::endl;
//     for (index_type k = 0; k < actions[act].post.length(); k++) {
//       index_type v_pre = actions[act].pre.value_of(actions[act].post[k].first);
//       if (v_pre != no_such_index) {
// 	s << "V" << actions[act].post[k].first << "_" << v_pre
// 	  << " -> A" << act
// 	  << " -> V" << actions[act].post[k].first << "_"
// 	  << actions[act].post[k].second << " [weight=100];" << std::endl;
//       }
//     }
//     for (index_type k = 0; k < actions[act].prv.length(); k++) {
//       s << "A" << act << " -> V" << actions[act].prv[k].first << "_"
// 	<< actions[act].prv[k].second << " [style=dashed,weight=1];"
// 	<< std::endl;
//     }
//   }
//
//   s << "}" << std::endl;
// }

void SASInstance::write_dc_graph(std::ostream& s) const
{
  s << "digraph SAS_CG {" << std::endl;
  s << "label=\"Dependency and Co-Change Graph\";" << std::endl;
  s << "overlap=false;" << std::endl;
  s << "splines=true;" << std::endl;
  s << "sep=0.1;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;
  s << "edge [len=2.0];" << std::endl;

  for (index_type k = 0; k < n_variables(); k++) {
    s << "V" << k << " [label=\"";
    variables[k].name->write(s, Name::NC_INSTANCE);
    s << "\"";
    if (goal_state.defines(k)) {
      s << ",style=bold";
    }
    s << "]; /* ";
    write_variable(s, variables[k]);
    s << " */" << std::endl;
  }

  index_set g_vars;
  goal_state.defined_set(g_vars);
  graph mdg;
  dependency_graph.minimal_distance_graph(mdg, g_vars);

  for (index_type i = 0; i < n_variables(); i++)
    for (index_type j = 0; j < n_variables(); j++) {
      if (interference_graph.adjacent(i, j)) {
	if (i < j) {
	  double w = 0.1;
	  if (mdg.adjacent(i, j) || mdg.adjacent(j, i)) w = 1.0;
	  if (dependency_graph.adjacent(i, j) &&
	      dependency_graph.adjacent(j, i)) {
	    s << "V" << i << " -> V" << j
	      << " [dir=both,style=bold,weight=" << w << "];"
	      << std::endl;
	  }
	  else if (dependency_graph.adjacent(i, j)) {
	    s << "V" << i << " -> V" << j
	      << " [dir=forward,style=bold,weight=" << w << "];"
	      << std::endl;
	  }
	  else if (dependency_graph.adjacent(j, i)) {
	    s << "V" << i << " -> V" << j
	      << " [dir=back,style=bold,weight=" << w << "];"
	      << std::endl;
	  }
	  else {
	    s << "V" << i << " -> V" << j
	      << " [dir=none,style=bold,weight=" << w << "];"
	      << std::endl;
	  }
	}
      }
      else if (dependency_graph.adjacent(i, j)) {
	double w = 0.1;
	if (mdg.adjacent(i, j) || mdg.adjacent(j, i)) w = 1.0;
	if (dependency_graph.adjacent(j, i) && (i < j)) {
	  s << "V" << i << " -> V" << j
	    << " [dir=both,weight=" << w << "];"
	    << std::endl;
	}
	else if (!dependency_graph.adjacent(j, i)) {
	  s << "V" << i << " -> V" << j
	    << " [dir=forward,weight=" << w << "];"
	    << std::endl;
	}
      }
    }

  s << "}" << std::endl;
}

void SASInstance::write_action_group_graph(std::ostream& s) const
{
  equivalence eq(n_actions());
  for (index_type i = 0; i < n_actions(); i++) {
    index_set ai_pre;
    actions[i].pre.defined_set(ai_pre);
    actions[i].prv.defined_set(ai_pre);
    index_set ai_post;
    actions[i].post.defined_set(ai_post);
    for (index_type j = i + 1; j < n_actions(); j++) {
      index_set aj_post;
      actions[j].post.defined_set(aj_post);
      if (ai_post == aj_post) {
	index_set aj_pre;
	actions[j].pre.defined_set(aj_pre);
	actions[j].prv.defined_set(aj_pre);
	if (aj_pre == ai_pre)
	  eq.merge(i, j);
      }
    }
  }
  index_set_vec g;
  eq.classes(g);

  s << "digraph SAS_CG {" << std::endl;
  s << "label=\"Action Group Graph\";" << std::endl;
  s << "overlap=false;" << std::endl;
  s << "splines=true;" << std::endl;
  s << "sep=0.1;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;
  s << "edge [len=1.0];" << std::endl;

  for (index_type k = 0; k < n_variables(); k++) {
    s << "V" << k << " [shape=ellipse,label=\"";
    variables[k].name->write(s, Name::NC_INSTANCE);
    s << "\"";
    if (goal_state.defines(k)) {
      s << ",style=bold";
    }
    s << "]; /* ";
    write_variable(s, variables[k]);
    s << " */" << std::endl;
  }

  for (index_type k = 0; k < g.length(); k++) {
    s << "A" << k << " [shape=box,label=\"";
    for (index_type i = 0; i < g[k].length(); i++) {
      if (i > 0) s << ",\\n";
      s << actions[g[k][i]].name;
    }
    s << "\"];" << std::endl;
  }

  for (index_type k = 0; k < g.length(); k++) {
    index_set g_pre;
    actions[g[k][0]].pre.defined_set(g_pre);
    actions[g[k][0]].prv.defined_set(g_pre);
    for (index_type i = 0; i < g_pre.length(); i++)
      s << "V" << g_pre[i] << " -> A" << k << std::endl;
    index_set g_post;
    actions[g[k][0]].post.defined_set(g_post);
    for (index_type i = 0; i < g_post.length(); i++)
      s << "A" << k << " -> V" << g_post[i] << std::endl;
  }

  s << "}" << std::endl;
}

void SASInstance::write_variable_digraph
(std::ostream& s, const graph& g, const char* label, bool opt_reverse) const
{
  s << "digraph SAS_VAR_DG {" << std::endl;
  s << "label=\"" << label << "\";" << std::endl;
  s << "overlap=false;" << std::endl;
  s << "splines=true;" << std::endl;
  s << "sep=0.1;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;
  s << "edge [len=2.0];" << std::endl;

  for (index_type k = 0; k < n_variables(); k++) {
    s << "V" << k << " [label=\"";
    variables[k].name->write(s, Name::NC_INSTANCE);
    s << "\"";
    if (goal_state.defines(k)) {
      s << ",style=bold";
    }
    s << "]; /* ";
    write_variable(s, variables[k]);
    s << " */" << std::endl;
  }

  for (index_type i = 0; i < n_variables(); i++)
    for (index_type j = 0; j < n_variables(); j++)
      if (g.adjacent(i, j)) {
	s << "V" << i << " -> V" << j;
	if (opt_reverse) {
	  s << " [dir=back]";
	}
	s << std::endl;
      }

  s << "}" << std::endl;
}

void SASInstance::write_weighted_variable_digraph
(std::ostream& s, const weighted_graph& g, const char* label,
 index_type n_weight_classes, index_type weight_class_delta) const
{
  s << "digraph SAS_WEIGHTED_VAR_DG {" << std::endl;
  s << "label=\"" << label << "\";" << std::endl;
  s << "overlap=false;" << std::endl;
  s << "splines=true;" << std::endl;
  s << "sep=0.1;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;
  s << "edge [len=2.0];" << std::endl;

  for (index_type k = 0; k < n_variables(); k++) {
    s << "V" << k << " [label=\"";
    variables[k].name->write(s, Name::NC_INSTANCE);
    s << "\"";
    if (goal_state.defines(k)) {
      s << ",style=bold";
    }
    s << "]; /* ";
    write_variable(s, variables[k]);
    s << " */" << std::endl;
  }

  NTYPE min_weight = POS_INF;
  NTYPE max_weight = NEG_INF;
  for (index_type i = 0; i < n_variables(); i++)
    for (index_type j = 0; j < n_variables(); j++)
      if (g.adjacent(i, j)) {
	min_weight = MIN(min_weight, g.weight(i, j));
	max_weight = MAX(max_weight, g.weight(i, j));
      }
  NTYPE span = (max_weight - min_weight);
  NTYPE step = (span / n_weight_classes);
  if (step < 1) step = 1;

  for (index_type i = 0; i < n_variables(); i++)
    for (index_type j = 0; j < n_variables(); j++)
      if (g.adjacent(i, j)) {
	NTYPE      ew = step;
	index_type lw = 1;
	while (ew < g.weight(i, j)) {
	  ew += step;
	  lw += weight_class_delta;
	}
	s << "V" << i << " -> V" << j
	  << "[label=\"" << g.weight(i, j)
	  << "\",style=\"setlinewidth(" << lw << ")\"]"
	  << std::endl;
      }

  s << "}" << std::endl;
}

void SASInstance::write_variable_graph
(std::ostream& s, const graph& g, const char* label) const
{
  s << "graph SAS_VAR_G {" << std::endl;
  s << "label=\"" << label << "\";" << std::endl;
  s << "overlap=false;" << std::endl;
  s << "splines=true;" << std::endl;
  s << "sep=0.1;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;

  for (index_type k = 0; k < n_variables(); k++) {
    s << "V" << k << " [label=\"";
    variables[k].name->write(s, Name::NC_INSTANCE);
    s << "\"";
    if (goal_state.defines(k)) {
      s << ",style=bold";
    }
    s << "]; /* ";
    write_variable(s, variables[k]);
    s << " */" << std::endl;
  }

  for (index_type i = 0; i < n_variables(); i++)
    for (index_type j = i + 1; j < n_variables(); j++)
      if (g.adjacent(i, j)) {
	s << "V" << i << " -- V" << j << std::endl;
      }

  s << "}" << std::endl;
}

void SASInstance::write_weighted_variable_graph
(std::ostream& s, const weighted_graph& g, const char* label) const
{
  s << "graph WG {" << std::endl;
  s << "label=\"" << label << "\";" << std::endl;
  s << "overlap=false;" << std::endl;
  s << "splines=true;" << std::endl;
  s << "sep=0.1;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;

  for (index_type k = 0; k < n_variables(); k++) {
    s << "V" << k << " [label=\"";
    variables[k].name->write(s, Name::NC_INSTANCE);
    s << "\"";
    if (goal_state.defines(k)) {
      s << ",style=bold";
    }
    s << "]; /* ";
    write_variable(s, variables[k]);
    s << " */" << std::endl;
  }

  for (index_type i = 0; i < n_variables(); i++)
    for (index_type j = i + 1; j < n_variables(); j++)
      if (g.adjacent(i, j)) {
	s << "V" << i << " -- V" << j
	  << "[label=\"" << g.weight(i, j) << "\"]"
	  << std::endl;
      }

  s << "}" << std::endl;
}

void SASInstance::write_action_sequence_graph(std::ostream& s) const
{
  s << "digraph ASG {" << std::endl;
  s << "label=\"Action Sequence Graph\";" << std::endl;
  // s << "rankdir=LR;" << std::endl;
  s << "node [shape=box,width=0,height=0];" << std::endl;

  for (index_type k = 0; k < n_actions(); k++) {
    s << "A" << k << " [label=\"" << actions[k].name << "\"];" << std::endl;
  }

  partial_state_vec c;
  for (index_type i = 0; i < n_actions(); i++)
    for (index_type j = 0; j < n_actions(); j++)
      if (i != j) {
	partial_state s_ij;
	bool l = contributes_to(actions[i], actions[j], s_ij);
	if (l) {
	  index_type p = c.first(s_ij);
	  if (p == no_such_index) {
	    c.append(s_ij);
	    p = (c.length() - 1);
	    s << "S" << p << " [shape=ellipse,label=\"";
	    write_partial_state(s, s_ij);
	    s << "\"];" << std::endl;
	  }
	  s << "A" << i << " -> S" << p << " ;" << std::endl;
	  s << "S" << p << " -> A" << j << " ;" << std::endl;
	}
      }

  s << "}" << std::endl;
}

void SASInstance::write_plan_transition_graph
(std::ostream& s, const index_vec& p, bool lockstep) const
{
  s << "digraph PTG {" << std::endl;
  s << "label=\"Plan Transition Graph\";" << std::endl;
  s << "rankdir=LR;" << std::endl;
  s << "node [shape=plaintext,width=0,height=0];" << std::endl;

  index_set rv; // relevant variables
  index_set vch; // variables that change
  for (index_type k = 0; k < p.length(); k++) {
    actions[p[k]].pre.defined_set(rv);
    actions[p[k]].post.defined_set(rv);
    actions[p[k]].post.defined_set(vch);
    actions[p[k]].prv.defined_set(rv);
  }

  // write variable subgraphs
  for (index_type i = 0; i < rv.length(); i++) {
    s << "subgraph cluster" << rv[i] << " {" << std::endl
      << "label=\"" << variables[rv[i]].name << "\";" << std::endl
      << "labeljust=l;" << std::endl
      << "rankdir=LR;" << std::endl;
    if (!vch.contains(rv[i])) {
      s << "style=filled;" << std::endl
	<< "color=lightgrey;" << std::endl;
    }
    if (goal_state.defines(rv[i])) {
      s << "style=bold;" << std::endl;
    }
    index_type last = 0;
    index_type val = init_state.value_of(rv[i]);
    assert(val < variables[rv[i]].n_values());
    s << "V" << rv[i] << "P0 [label=\""
      << variables[rv[i]].domain[val]
      << "\"];" << std::endl;
    for (index_type k = 0; k < p.length(); k++) {
      val = actions[p[k]].post.value_of(rv[i]);
      if (val != no_such_index) {
	s << "V" << rv[i] << "P" << k + 1 << " [label=\""
	  << variables[rv[i]].domain[val]
	  << "\"];" << std::endl;
	s << "V" << rv[i] << "P" << last << " -> V" << rv[i] << "P" << k + 1
	  << ";" << std::endl;
	last = k + 1;
      }
      else if (lockstep) {
	s << "V" << rv[i] << "P" << k + 1 << " [label=\"...\"];" << std::endl;
	s << "V" << rv[i] << "P" << last << " -> V" << rv[i] << "P" << k + 1
	  << ";" << std::endl;
	last = k + 1;
      }
    }
    s << "}" << std::endl; // end subgraph for rv[i]
  }

  // write cross-variable dependencies
  for (index_type k = 0; k < p.length(); k++) {
    for (index_type i = 0; i < actions[p[k]].pre.length(); i++) {
      index_type var = actions[p[k]].pre[i].first;
      index_type val = actions[p[k]].pre[i].second;
      index_type f = no_such_index;
      index_type l = k;
      while ((l > 0) && (f == no_such_index)) {
	if (actions[p[l - 1]].post.value_of(var) == val) f = l;
	l = l - 1;
      }
      if (f == no_such_index) f = 0;
      for (index_type j = 0; j < actions[p[k]].post.length(); j++)
	if (actions[p[k]].post[j].first != var) {
	  s << "V" << var << "P" << f << " -> V"
	    << actions[p[k]].post[j].first << "P" << k + 1
	    << ";" << std::endl;
	}
    }
    for (index_type i = 0; i < actions[p[k]].prv.length(); i++) {
      index_type var = actions[p[k]].prv[i].first;
      index_type val = actions[p[k]].prv[i].second;
      index_type f = no_such_index;
      index_type l = k;
      while ((l > 0) && (f == no_such_index)) {
	if (actions[p[l - 1]].post.value_of(var) == val) f = l;
	l = l - 1;
      }
      if (f == no_such_index) f = 0;
      for (index_type j = 0; j < actions[p[k]].post.length(); j++) {
	s << "V" << var << "P" << f << " -> V"
	  << actions[p[k]].post[j].first << "P" << k + 1
	  << ";" << std::endl;
      }
    }
  }

  s << "{ rank=same;" << std::endl;
  for (index_type i = 0; i < rv.length(); i++) {
    s << "V" << rv[i] << "P0;" << std::endl;
  }
  s << "}" << std::endl;

  for (index_type k = 0; k < p.length(); k++) {
    s << "{ rank=same;" << std::endl;
    for (index_type j = 0; j < actions[p[k]].post.length(); j++) {
      s << "V" << actions[p[k]].post[j].first << "P" << k + 1
	<< ";" << std::endl;
    }
    s << "}" << std::endl;
  }

  s << "}" << std::endl;
}


void StateHashFunction::init(SASInstance& ins)
{
  base.assign_value(0, ins.n_variables());
  if (ins.n_variables() == 0) return;
  base[0] = 1;
  for (index_type k = 1; k < ins.n_variables(); k++) {
    unsigned long long tmp =
      ((base[k - 1] * ins.variables[k].n_values()) % LARGE_PRIME);
    base[k] = tmp;
  }
}

void StateHashFunction::init(SASInstance& ins, const index_set& vars)
{
  base.assign_value(0, ins.n_variables());
  if (vars.length() == 0) return;
  index_type prev = 1;
  base[vars[0]] = prev;
  for (index_type k = 1; k < vars.length(); k++) {
    unsigned long long tmp =
      ((prev * ins.variables[vars[k]].n_values()) % LARGE_PRIME);
    base[vars[k]] = tmp;
    prev = tmp;
  }
}

index_type StateHashFunction::index(const partial_state& s) const
{
  index_type v = 0;
  for (index_type k = 0; k < s.length(); k++) {
    assert(s[k].first < base.length());
    v = (v + ((s[k].second * base[s[k].first]) % LARGE_PRIME));
  }
  return v;
}


PartialStatePerfectHashFunction::PartialStatePerfectHashFunction
(SASInstance& ins, const index_set& vars)
  : index_vec(0, ins.n_variables()),
    size(1),
    overflow_flag(false)
{
  assert(!vars.empty());
  for (index_type k = 0; k < vars.length(); k++) {
    assert(vars[k] < ins.n_variables());
    (*this)[vars[k]] = size;
    if ((index_type_max / (ins.variables[vars[k]].n_values() + 1)) < size)
      overflow_flag = true;
    size = (size * (ins.variables[vars[k]].n_values() + 1));
  }
}

PartialStatePerfectHashFunction::PartialStatePerfectHashFunction
(SASInstance& ins)
  : index_vec(0, ins.n_variables()),
    size(1),
    overflow_flag(false)
{
  // done (non-initializing constructor; used by subclass constructors only)
}

index_type PartialStatePerfectHashFunction::index
(const partial_state& s) const
{
  index_type i = 0;
#ifdef TRACE_PRINT_LOTS
  std::cerr << "PSPHF: indexing " << s << std::endl;
  std::cerr << "PSPHF: base = " << (*this) << std::endl;
#endif
  for (index_type k = 0; k < s.length(); k++) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "PSPHF: s[" << k << "] = " << s[k] << std::endl;
    std::cerr << "PSPHF: base[" << s[k].first << "] = " << (*this)[s[k].first]
	      << std::endl;
#endif
    i += ((*this)[s[k].first] * (s[k].second + 1));
#ifdef TRACE_PRINT_LOTS
    std::cerr << "PSPHF: i = " << i << std::endl;
#endif
  }
  if (i > size) {
    std::cerr << "error in PartialStatePerfectHashFunction::index: state "
	      << s << " has index " << i
	      << " > size " << size
	      << ", base = " << (*this)
	      << ", size = " << size
	      << ")" << std::endl;
    exit(255);
  }
  return i;
}

index_type PartialStatePerfectHashFunction::index1
(index_type var, index_type val) const
{
  if (val == no_such_index) return 0;
  return ((*this)[var] * (val + 1));
}

void PartialStatePerfectHashFunction::state
(index_type i, partial_state& s) const
{
  index_type i0 = i;
  s.clear();
  for (index_type k = (*this).length(); k > 0; k--) if ((*this)[k - 1] > 0) {
    index_type vk = (i / (*this)[k - 1]);
    if (vk > 0) s.assign(k - 1, vk - 1);
    i = (i - (vk * (*this)[k - 1]));
  }
  // sanity check of answer
  index_type is = index(s);
  assert(i0 == is);
}

CompleteStatePerfectHashFunction::CompleteStatePerfectHashFunction
(SASInstance& ins, const index_set& vars)
  : PartialStatePerfectHashFunction(ins),
    n_vars(vars.length())
{
  assert(!vars.empty());
  for (index_type k = 0; k < vars.length(); k++) {
    assert(vars[k] < ins.n_variables());
    (*this)[vars[k]] = size;
    if ((index_type_max / ins.variables[vars[k]].n_values()) < size)
      overflow_flag = true;
    size = (size * ins.variables[vars[k]].n_values());
  }
}

index_type CompleteStatePerfectHashFunction::index
(const partial_state& s) const
{
  index_type i = 0;
  index_type n = 0;
  for (index_type k = 0; k < s.length(); k++)
    if ((*this)[s[k].first] > 0) {
      i += ((*this)[s[k].first] * s[k].second);
      n += 1;
    }
  if (n < n_vars) {
    std::cerr << "error in CompleteStateAbstraction::index: state "
	      << s << " is not complete w.r.t. abstraction"
	      << " (n = " << n
	      << ", n_vars = " << n_vars
	      << ", base = " << (*this)
	      << ", size = " << size
	      << ")" << std::endl;
    assert(0);
  }
  if (i > size) {
    std::cerr << "error in CompleteStateAbstraction::index: state "
	      << s << " has index " << i
	      << " > size " << size
	      << " (n_vars = " << n_vars
	      << ", base = " << (*this)
	      << ", size = " << size
	      << ")" << std::endl;
    assert(0);
  }
  return i;
}

index_type CompleteStatePerfectHashFunction::index1
(index_type var, index_type val) const
{
  if (val == no_such_index) return 0;
  return ((*this)[var] * val);
}

void CompleteStatePerfectHashFunction::state
(index_type i, partial_state& s) const
{
  index_type i0 = i;
  s.clear();
  for (index_type k = (*this).length(); k > 0; k--) if ((*this)[k - 1] > 0) {
    index_type vk = (i / (*this)[k - 1]);
    s.assign(k - 1, vk);
    i = (i - (vk * (*this)[k - 1]));
  }
  // sanity check of answer
  index_type is = index(s);
  assert(i0 == is);
}


IndependentVariableSets::IndependentVariableSets
(SASInstance& ins)
  : index_set_vec(EMPTYSET, 1), instance(ins)
{
  (*this)[0].fill(instance.n_variables());
}

IndependentVariableSets::IndependentVariableSets
(SASInstance& ins, const index_set& vars)
  : index_set_vec(EMPTYSET, 1), instance(ins)
{
  (*this)[0].assign_copy(vars);
}

IndependentVariableSets::IndependentVariableSets
(const IndependentVariableSets& ivs)
  : index_set_vec(ivs), instance(ivs.instance)
{
  // done
}

IndependentVariableSets::~IndependentVariableSets()
{
  // done
}

void IndependentVariableSets::compute_maximal_independent_sets()
{
  index_set v;
  for (index_type k = 0; k < length(); k++) v.insert((*this)[k]);
  assert(v.length() > 0);
  graph g(instance.independence_graph, v);
  set_length(0);
  g.maximal_clique_cover(*this);
  // g.all_nondominated_cliques(*this);
  for (index_type k = 0; k < length(); k++) {
    index_set s((*this)[k]);
    (*this)[k].assign_remap(s, v);
  }
}

void IndependentVariableSets::compute_approximate_independent_sets()
{
  index_set v;
  for (index_type k = 0; k < length(); k++) v.insert((*this)[k]);
  assert(v.length() > 0);
  graph g(instance.interference_graph, v);
  set_length(0);
  g.apx_independent_set_cover(*this);
  for (index_type k = 0; k < length(); k++) {
    index_set s((*this)[k]);
    (*this)[k].assign_remap(s, v);
  }
}

void IndependentVariableSets::find_spanning_sets
(const index_set& cs, const index_set& cv,
 const index_set_vec& dets, const index_set& av,
 index_set& bs, index_type& bs_nv,
 bool preserve_goals, index_type& n_bt, index_type max_bt)
{
  if (cs.length() > bs.length()) return;
  if (cv.length() == av.length()) {
    if (cs.length() < bs.length()) {
      bs = cs;
      index_set csu;
      dets.selected_union_set(cs, csu);
      bs_nv = csu.length();
    }
    else {
      assert(cs.length() == bs.length());
      index_set csu;
      dets.selected_union_set(cs, csu);
      if (csu.length() < bs_nv) {
	bs = cs;
	bs_nv = csu.length();
      }
      else if (csu.length() == bs_nv) {
	if (dets.selected_maximum_cardinality(cs) < 
	    dets.selected_maximum_cardinality(bs)) {
	  bs = cs;
	  bs_nv = csu.length();
	}
      }
    }
    return;
  }
  if ((cs.length() + 1) > bs.length()) return;
  for (index_type k = 0; k < dets.length(); k++)
    if (!cv.contains(dets[k])) {
      index_set ns(cs);
      ns.insert(k);
      index_set nv(cv);
      nv.insert(dets[k]);
      if (nv.length() == av.length()) {
	if (ns.length() < bs.length()) {
	  bs = ns;
	  index_set nsu;
	  dets.selected_union_set(ns, nsu);
	  bs_nv = nsu.length();
	}
	else {
	  assert(ns.length() == bs.length());
	  index_set nsu;
	  dets.selected_union_set(ns, nsu);
	  if (nsu.length() < bs_nv) {
	    bs = ns;
	    bs_nv = nsu.length();
	  }
	  else if (nsu.length() == bs_nv) {
	    if (dets.selected_maximum_cardinality(ns) < 
		dets.selected_maximum_cardinality(bs)) {
	      bs = ns;
	      bs_nv = nsu.length();
	    }
	  }
	}
	return;
      }
      index_set nd;
      compute_extended_set(nv, av, preserve_goals, nd);
      find_spanning_sets(ns, nd, dets, av, bs, bs_nv,
			 preserve_goals, n_bt, max_bt);
      if ((cs.length() + 1) > bs.length()) return;
      n_bt += 1;
      if ((max_bt != no_such_index) && (n_bt >= max_bt)) return;
    }
}

void IndependentVariableSets::compute_extended_set
(const index_set& s, const index_set& u, bool preserve_goals, index_set& x)
{
  bool asc_state = SASInstance::additional_strictness_check;
  SASInstance::additional_strictness_check = false;
  x.assign_copy(s);
  for (index_type k = 0; k < u.length(); k++) if (!s.contains(u[k])) {
    SASInstance::determined_check_result res =
      instance.weak_determined_check(s, u[k]);
    if ((res == SASInstance::determined_strict) ||
	(res == SASInstance::determined_goal_preserving) ||
	((res == SASInstance::determined_yes) && !preserve_goals))
      x.insert(u[k]);
  }
  SASInstance::additional_strictness_check = asc_state;
}

void IndependentVariableSets::compute_spanning_sets
(bool preserve_goal_sets, index_type max_bt)
{
  // std::cerr << "independent sets: " << *this << std::endl;
  index_set uset;
  union_set(uset);

  // first, check if any variable is determined at all; any set that is the
  // only one to contain a given non-determined variable must be included in
  // the final collection
  bool asc_state = SASInstance::additional_strictness_check;
  SASInstance::additional_strictness_check = false;
  bool empty_det_relation = true;
  index_set necs;
  for (index_type k = 0; k < uset.length(); k++) {
    index_set d;
    SASInstance::determined_check_result res =
      instance.determining_set(uset[k], d);
    if ((res == SASInstance::determined_no) ||
	((res == SASInstance::determined_yes) && preserve_goal_sets)) {
      index_type j = no_such_index;
      bool unique = true;
      for (index_type i = 0; (i < length()) && unique; i++)
	if ((*this)[i].contains(uset[k])) {
	  if (j == no_such_index)
	    j = i;
	  else
	    unique = false;
	}
      assert(j != no_such_index);
      if (unique) {
	necs.insert(j);
      }
    }
    else {
      empty_det_relation = false;
    }
  }
  SASInstance::additional_strictness_check = asc_state;
  if (empty_det_relation) {
    std::cerr << "determines relation is empty" << std::endl;
    return;
  }
  // std::cerr << "necessary sets: " << necs << std::endl;

  // next, compute the basic extended sets, and order them by decreasing
  // size; smap maps from sorted to original order, rmap maps in reverse
  index_set_vec dets(EMPTYSET, 0);
  mapping smap;
  mapping rmap;
  decreasing_cardinality_order o;
  for (index_type k = 0; k < length(); k++) {
    index_set d;
    compute_extended_set((*this)[k], uset, preserve_goal_sets, d);
    index_type i = dets.insert_ordered(d, o);
    smap.insert(k, i);
  }
  smap.invert(rmap);
  // std::cerr << "extended independent sets: " << dets << std::endl;
  // std::cerr << "smap: " << smap << std::endl;
  // std::cerr << "rmap: " << rmap << std::endl;

  // call the branch-and-bound procedure (find_spanning_sets); initial
  // partial solution contains the necessarily included sets
  // (note remap: find_spanning_sets operates on sorted indices!)
  index_set inis;
  inis.assign_remap(necs, rmap);
  index_set iniv;
  for (index_type k = 0; k < inis.length(); k++)
    iniv.insert(dets[inis[k]]);
  index_set bs;
  bs.fill(length());
  index_type bs_nv = uset.length();
  index_type n_bt = 0;
  find_spanning_sets(inis, iniv, dets, uset, bs, bs_nv,
		     preserve_goal_sets, n_bt, max_bt);
  // std::cerr << "best solution: " << bs << " (" << n_bt << " backtracks)"
  // << std::endl;

  // map solution back to original order, and remove sets not in in
  index_set mbs;
  mbs.assign_remap(bs, smap);
  // std::cerr << "remapped best solution: " << mbs << std::endl;
  bool_vec trm(mbs, length());
  trm.complement();
  remove(trm);
  // std::cerr << "spanning sets: " << *this << std::endl;
}

index_type IndependentVariables::n_sets()
{
  if (have_spanning) return spanning_sets.length();
  if (have_independent) return independent.length();
  return 0;
}

const index_set& IndependentVariables::set(index_type i)
{
  if (have_spanning) {
    if (i < spanning_sets.length()) return independent[spanning_sets[i]];
    return EMPTYSET;
  }
  if (have_independent) {
    if (i < independent.length()) return independent[i];
  }
  return EMPTYSET;
}

const index_set& IndependentVariables::spanning_variables()
{
  if (!have_spanning) compute_spanning_sets();
  return spanning_vars;
}

void IndependentVariables::compute_maximal_independent_sets()
{
  instance.compute_graphs();
  instance.independence_graph.maximal_clique_cover(independent);
  // instance.independence_graph.all_nondominated_cliques(independent);
  have_independent = true;
}

void IndependentVariables::compute_approximate_independent_sets()
{
  instance.compute_graphs();
  instance.interference_graph.apx_independent_set_cover(independent);
  have_independent = true;
}

void IndependentVariables::compute_determined()
{
  if (!have_independent) compute_maximal_independent_sets();
  bool asc_state = SASInstance::additional_strictness_check;
  SASInstance::additional_strictness_check = false;
  determined.assign_copy(independent);
  for (index_type k = 0; k < independent.length(); k++)
    for (index_type d = 0; d < instance.n_variables(); d++)
      if (!independent[k].contains(d))
	if (instance.weak_determined_check(independent[k], d)
	    != SASInstance::determined_no)
	  determined[k].insert(d);
  SASInstance::additional_strictness_check = asc_state;
}

void IndependentVariables::compute_spanning_sets()
{
  if (!have_independent) compute_maximal_independent_sets();
  compute_determined();
  covers.init(independent.length());
  for (index_type i = 0; i < independent.length(); i++)
    for (index_type j = 0; j < independent.length(); j++)
      if (i != j)
	if (determined[i].contains(independent[j]))
	  covers.add_edge(i, j);
  covers.strongly_connected_components();
  covers.component_tree(covers_scc);
  spanning_sets.clear();
  for (index_type i = 0; i < covers_scc.size(); i++)
    if (covers_scc.in_degree(i) == 0)
      spanning_sets.insert(covers.component_node(i));
  spanning_vars.clear();
  for (index_type i = 0; i < spanning_sets.length(); i++)
    spanning_vars.insert(independent[spanning_sets[i]]);
  have_spanning = true;
}

SASInstance* IndependentVariables::reduced_instance()
{
  return new SASInstance(instance, spanning_variables());
}

PrintSASActions::PrintSASActions
(SASInstance& i, std::ostream& s)
  : instance(i), to(s),
    action_sep('\n'), first_action(true),
    plan_sep('\n'), first_plan(true)
{
  // done
}

PrintSASActions::PrintSASActions
(SASInstance& i, std::ostream& s, char as)
  : instance(i), to(s),
    action_sep(as), first_action(true),
    plan_sep('\n'), first_plan(true)
{
  // done
}

PrintSASActions::PrintSASActions
(SASInstance& i, std::ostream& s, char as, char ps)
  : instance(i), to(s),
    action_sep(as), first_action(true),
    plan_sep(ps), first_plan(true)
{
  // done
}

PrintSASActions::~PrintSASActions()
{
  // done
}

Plan* PrintSASActions::new_plan()
{
  if (!first_plan) to << plan_sep;
  first_plan = false;
  first_action = true;
  return this;
}

void PrintSASActions::protect(index_type atom)
{
  // ignore
}

void PrintSASActions::insert(index_type act)
{
  if (!first_action) to << action_sep;
  instance.actions[act].name->write(to, Name::NC_PLAN);
  first_action = false;
}

void PrintSASActions::advance(NTYPE delta)
{
  // ignore
}

void PrintSASActions::end()
{
  // ignore
}


SASPlanSummary::SASPlanSummary(SASInstance& i)
  : instance(i)
{
  clear();
}

void SASPlanSummary::protect(index_type atom)
{
  // nothing
}

void SASPlanSummary::insert(index_type act)
{
  if (need_to_clear) clear();
  index_type c_act = instance.corresponding_action(act);
  if (c_act == no_such_index) {
    std::cerr << "error: action " << act
	      << " has no match in SAS instance"
	      << std::endl;
    exit(255);
  }
  SASInstance::Action& a = instance.actions[c_act];
  for (index_type k = 0; k < a.post.length(); k++) {
    n_value_changes[a.post[k].first] += 1;
  }
  for (index_type k = 0; k < a.pre.length(); k++) {
    n_required_values[a.pre[k].first] += 1;
  }
  for (index_type k = 0; k < a.prv.length(); k++) {
    n_required_values[a.prv[k].first] += 1;
  }
}

void SASPlanSummary::advance(NTYPE delta)
{
  // nothing
}

void SASPlanSummary::end()
{
  need_to_clear = true;
}


void SASPlanSummary::clear()
{
  n_value_changes.assign_value(0, instance.n_variables());
  n_required_values.assign_value(0, instance.n_variables());
  need_to_clear = false;
}

double SASPlanSummary::average_variable_value_changes()
{
  double sum = 0;
  for (index_type k = 0; k < instance.n_variables(); k++)
    sum += n_value_changes[k];
  return sum/(double)instance.n_variables();
}

index_type SASPlanSummary::min_variable_value_changes()
{
  if (instance.n_variables() == 0) return 0;
  index_type min = n_value_changes[0];
  for (index_type k = 1; k < instance.n_variables(); k++)
    if (n_value_changes[k] < min) min = n_value_changes[k];
  return min;
}

index_type SASPlanSummary::max_variable_value_changes()
{
  if (instance.n_variables() == 0) return 0;
  index_type max = n_value_changes[0];
  for (index_type k = 1; k < instance.n_variables(); k++)
    if (n_value_changes[k] > max) max = n_value_changes[k];
  return max;
}

double SASPlanSummary::average_variable_required_values()
{
  double sum = 0;
  for (index_type k = 0; k < instance.n_variables(); k++)
    sum += n_required_values[k];
  return sum/(double)instance.n_variables();
}

index_type SASPlanSummary::n_secondary_goal_variables()
{
  index_type count = 0;
  for (index_type k = 0; k < instance.n_variables(); k++)
    if ((n_required_values[k] > 0) && !instance.goal_state.defines(k))
      count += 1;
  return count;
}

END_HSPS_NAMESPACE
