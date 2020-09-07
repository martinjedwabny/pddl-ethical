
#include "base.h"
#include "enumerators.h"
#include <stdlib.h>
#include <sstream>

BEGIN_HSPS_NAMESPACE

bool PDDL_Base::use_default_function_value = true;
NTYPE PDDL_Base::default_function_value(0);
bool PDDL_Base::use_strict_borrow_definition = false;
bool PDDL_Base::use_extended_borrow_definition = false;
bool PDDL_Base::compact_resource_effects = true;
bool PDDL_Base::compile_away_disjunctive_preconditions = true;
bool PDDL_Base::check_precondition_consistency = true;
bool PDDL_Base::compile_away_conditional_effects = true;
bool PDDL_Base::compile_away_plan_constraints = true;
bool PDDL_Base::compile_away_object_functions = true;
bool PDDL_Base::compile_for_validator = false;
bool PDDL_Base::create_all_atoms = false;
bool PDDL_Base::create_all_actions = false;
bool PDDL_Base::number_multiple_action_instances = false;
bool PDDL_Base::exclude_all_dkel_items = false;
string_set PDDL_Base::excluded_dkel_tags;
string_set PDDL_Base::required_dkel_tags;
bool PDDL_Base::strict_set_export = true;
bool PDDL_Base::write_PDDL31 = true;
bool PDDL_Base::best_effort = true;
bool PDDL_Base::write_warnings = true;
bool PDDL_Base::write_info = false;
bool PDDL_Base::write_trace = false;
bool PDDL_Base::name_instance_by_problem_file = false;
const char* PDDL_Base::instance_name_prefix = 0;

bool PDDL_Base::trace_print_context = false;
bool PDDL_Base::AtomBase::print_bindings = false;
bool PDDL_Base::Expression::print_nary = false;

PDDL_Base::PredicateSymbol* PDDL_Base::current_eq_predicate = 0;

char* PDDL_Base::problem_file_basename()
{
  if (problem_file) {
    char* p0 = strdup(problem_file);
    char* p1 = strrchr(p0, '/');
    char* p2 = strrchr(p0, '.');
    if ((p1 != 0) && (p2 != 0) && (p1 < p2)) {
      *p2 = '\0';
    }
    else if ((p1 == 0) && (p2 != 0)) {
      *p2 = '\0';
    }
    if (p1)
      return p1 + 1;
    else
      return p0;
  }
  else {
    return 0;
  }
}

char* PDDL_Base::enum_problem_filename(const char* s, index_type i)
{
  std::ostringstream fname;
  char* b = problem_file_basename();
  if (b) {
    fname << b << "-" << s << i << ".pddl";
  }
  else {
    fname << s << i << ".pddl";
  }
  // delete b;
  return strdup(fname.str().c_str());
}

index_type PDDL_Base::TypeSet::n_elements() const
{
  index_type n = 0;
  for (index_type k = 0; k < length(); k++)
    n += (*this)[k]->elements.length();
  return n;
}

PDDL_Base::Symbol* PDDL_Base::TypeSet::get_element
(index_type n)
{
  index_type k = 0;
  while (k < length()) {
    if (n < (*this)[k]->elements.length())
      return (*this)[k]->elements[n];
    n -= (*this)[k]->elements.length();
    k += 1;
  }
  std::cerr << "error: index out of range in get_element ";
  print(std::cerr);
  exit(255);
}

bool PDDL_Base::TypeSet::is_object() const
{
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k]->is_object()) return true;
  return false;
}

bool PDDL_Base::TypeSet::subtype_or_equal(const TypeSet& s) const
{
  if (s.length() == 0) return true;
  if (length() == 0) {
    for (index_type l = 0; l < s.length(); l++)
      if (s[l]->is_object()) return true;
    return false;
  }
  else {
    for (index_type k = 0; k < length(); k++) {
      bool ok = false;
      for (index_type l = 0; (l < s.length()) && !ok; l++)
	if ((*this)[k]->subtype_or_equal(s[l]))
	  ok = true;
      if (!ok) return false;
    }
    return true;
  }
}

bool PDDL_Base::TypeSet::subtype_or_equal(TypeSymbol* t) const
{
  if (length() == 0) {
    if (t->is_object()) return true;
    return false;
  }
  else {
    for (index_type k = 0; k < length(); k++) {
      if (!(*this)[k]->subtype_or_equal(t))
	return false;
    }
    return true;
  }
}

void PDDL_Base::TypeSet::print(std::ostream& s) const
{
  if (length() > 1) {
    s << "(either";
    for (index_type k = 0; k < length(); k++) {
      s << " ";
      (*this)[k]->print(s);
    }
    s << ")";
  }
  else if (length() == 1) {
    (*this)[0]->print(s);
  }
}

void PDDL_Base::TypeSet::write_type(std::ostream& s) const
{
  if (length() > 1) {
    s << " - (either";
    for (index_type k = 0; k < length(); k++) {
      s << " " << (*this)[k]->print_name;
    }
    s << ")";
  }
  else if (length() == 1) {
    s << " - " << (*this)[0]->print_name;
  }
  else {
    s << " - object";
  }
}

PDDL_Base::TypeSymbol* PDDL_Base::find_type(const char* name)
{
  for (index_type k = 0; k < dom_types.length(); k++)
    if (tab.table_char_map().strcmp(dom_types[k]->print_name, name) == 0)
      return dom_types[k];
  return 0;
}

PDDL_Base::PredicateSymbol* PDDL_Base::find_predicate(const char* name)
{
  for (index_type k = 0; k < dom_predicates.length(); k++)
    if (tab.table_char_map().strcmp(dom_predicates[k]->print_name, name) == 0)
      return dom_predicates[k];
  return 0;
}

PDDL_Base::FunctionSymbol* PDDL_Base::find_function(const char* name)
{
  for (index_type k = 0; k < dom_functions.length(); k++)
    if (tab.table_char_map().strcmp(dom_functions[k]->print_name, name) == 0)
      return dom_functions[k];
  return 0;
}

bool PDDL_Base::find_initial_fact(const char* pname, const symbol_vec& arg)
{
  PredicateSymbol* pred = find_predicate(pname);
  if (pred == 0) {
    std::cerr << "error: can't check static fact " << pname << arg
	      << " - no such predicate" << std::endl;
    exit(255);
  }
  ptr_table* r = &(pred->init);
  for (index_type k = 0; (k < arg.length()) && r; k++) {
    if (arg[k]->sym_class == sym_variable) {
      if (((VariableSymbol*)arg[k])->value == 0) {
	std::cerr << "error: can't evaluate " << pname << arg
		  << " - " << arg[k]->print_name << " not set"
		  << std::endl;
	exit(255);
      }
      r = r->find_next(((VariableSymbol*)arg[k])->value);
    }
    else {
      r = r->find_next(arg[k]);
    }
  }
  if (r) {
    if (r->val) return true;
  }
  return false;
}

NTYPE PDDL_Base::find_function_value(const char* fname, const symbol_vec& arg)
{
  FunctionSymbol* fun = find_function(fname);
  if (fun == 0) {
    std::cerr << "error: can't eval static exp " << fname << arg
	      << " - no such function" << std::endl;
    exit(255);
  }
  if (!fun->is_static()) {
    std::cerr << "error: can't eval static exp " << fname << arg
	      << " - function not static" << std::endl;
    exit(255);
  }
  ptr_table* r = &(fun->init);
  for (index_type k = 0; (k < arg.length()) && r; k++) {
    if (arg[k]->sym_class == sym_variable) {
      if (((VariableSymbol*)arg[k])->value == 0) {
	std::cerr << "error: can't evaluate " << fname << arg
		  << " - " << arg[k]->print_name << " not set"
		  << std::endl;
	exit(255);
      }
      r = r->find_next(((VariableSymbol*)arg[k])->value);
    }
    else {
      r = r->find_next(arg[k]);
    }
  }
  if (r) {
    if (r->val) {
      return ((FInitAtom*)r->val)->val;
    }
  }
  if (use_default_function_value) {
    return default_function_value;
  }
  else {
    std::cerr << "error: " << fname << arg << " has no value" << std::endl;
    exit(255);
  }
}

index_type PDDL_Base::find_element_satisfying
(const symbol_vec& elements,
 const char* pname,
 symbol_vec& arg,
 index_type element_arg_p)
{
  for (index_type k = 0; k < elements.length(); k++) {
    arg[element_arg_p] = elements[k];
    if (find_initial_fact(pname, arg))
      return k;
  }
  return no_such_index;
}

void PDDL_Base::find_elements_satisfying
(const symbol_vec& elements,
 const char* pname,
 symbol_vec& arg,
 index_type element_arg_p,
 index_set& sats)
{
  sats.clear();
  for (index_type k = 0; k < elements.length(); k++) {
    arg[element_arg_p] = elements[k];
    if (find_initial_fact(pname, arg))
      sats.insert(k);
  }
}

PDDL_Base::Atom* PDDL_Base::goal_to_atom(Goal* g)
{
  if ((g->g_class == goal_pos_atom) || (g->g_class == goal_neg_atom))
    return ((AtomicGoal*)g)->atom;
  else
    return 0;
}

bool PDDL_Base::goal_to_atom_vec(Goal* g, atom_vec& av)
{
  av.clear();
  if ((g->g_class == goal_pos_atom) || (g->g_class == goal_neg_atom)) {
    av.append(((AtomicGoal*)g)->atom);
    return true;
  }
  else if (g->g_class == goal_conjunction) {
    ConjunctiveGoal* cg = (ConjunctiveGoal*)g;
    for (index_type k = 0; k < cg->goals.length(); k++) {
      Atom* g_k = goal_to_atom(cg->goals[k]);
      if (g_k == 0) return false;
      av.append(g_k);
    }
    return true;
  }
  else {
    return false;
  }
}

PDDL_Base::ActionSymbol* PDDL_Base::src_action_symbol(ptr_pair* p)
{
  PDDL_Base::ActionSymbol* act = (PDDL_Base::ActionSymbol*)p->first;
  ptr_table* ins = (ptr_table*)p->second;
  ptr_table::key_vec* args = ins->key_sequence();
  for (index_type k = 0; k < act->param.length(); k++)
    act->param[k]->value = (PDDL_Base::Symbol*)((*args)[k + 1]);
  delete args;
  return act;
}


PDDL_Base::PDDL_Base(StringTable& t)
  : domain_name(0),
    problem_name(0),
    domain_file(0),
    problem_file(0),
    ready_to_instantiate(false),
    tab(t),
    dom_types(0, 0),
    dom_top_type(0),
    dom_constants(0, 0),
    dom_predicates(0, 0),
    dom_object_functions(0, 0),
    dom_functions(0, 0),
    dom_actions(0, 0),
    dom_sc_invariants(0, 0),
    dom_f_invariants(0, 0),
    dom_irrelevant(0, 0),
    dom_init(0, 0),
    dom_fun_init(0, 0),
    dom_obj_init(0, 0),
    dom_goals(0, 0),
    dom_preferences(0, 0),
    goal_tasks(0, 0),
    metric_type(metric_none),
    metric(0),
    serial_length(POS_INF),
    parallel_length(POS_INF),
    input_plans(0, 0),
    h_table(0, 0),
    input_sets(0, 0),
    partitions(0, 0)
{
  // create some predefined symbols
  StringTable::Cell* sc = tab.inserta("object");
  dom_top_type = new TypeSymbol(sc->text);
  sc->val = dom_top_type;
  sc = tab.inserta("=");
  dom_eq_pred = new PredicateSymbol(sc->text);
  dom_eq_pred->param.append(new VariableSymbol("?x"));
  dom_eq_pred->param.append(new VariableSymbol("?y"));
  sc = tab.inserta("assign");
  dom_assign_pred = new PredicateSymbol(sc->text);
  dom_assign_pred->param.append(new VariableSymbol("?x"));
  dom_assign_pred->param.append(new VariableSymbol("?y"));
  sc->val = dom_assign_pred;
  sc = tab.inserta("undefined");
  dom_undefined_obj = new Symbol(sc->text);
  sc->val = dom_undefined_obj;
}

PDDL_Base::~PDDL_Base()
{
  // not implemented
}

void PDDL_Base::set_variable_type(variable_vec& vec, TypeSymbol* t)
{
  for (index_type k = vec.length(); k > 0; k--) {
    if (vec[k - 1]->sym_types.length() > 0) return;
    vec[k - 1]->sym_types.append(t);
    if (trace_print_context) {
      std::cerr << "set type of " << k - 1 << "th variable ";
      vec[k - 1]->print(std::cerr);
      std::cerr << " to ";
      t->print(std::cerr);
      std::cerr << std::endl;
    }
  }
}

void PDDL_Base::set_variable_type(variable_vec& vec, const TypeSet& t)
{
  for (index_type k = vec.length(); k > 0; k--) {
    if (vec[k - 1]->sym_types.length() > 0) return;
    vec[k - 1]->sym_types.assign_copy(t);
    if (trace_print_context) {
      std::cerr << "set type of " << k - 1 << "th variable ";
      vec[k - 1]->print(std::cerr);
      std::cerr << " to ";
      t.print(std::cerr);
      std::cerr << std::endl;
    }
  }
}

void PDDL_Base::set_type_type(type_vec& vec, TypeSymbol* t)
{
  for (index_type k = vec.length(); k > 0; k--) {
    if (vec[k - 1]->sym_types.length() > 0) return;
    vec[k - 1]->sym_types.append(t);
  }
}

void PDDL_Base::set_constant_type(symbol_vec& vec, TypeSymbol* t)
{
  for (index_type k = vec.length(); k > 0; k--) {
    if (vec[k - 1]->sym_types.length() > 0) return;
    vec[k - 1]->sym_types.append(t);
    t->add_element(vec[k - 1]);
  }
}

void PDDL_Base::clear_context(variable_vec& vec)
{
  for (index_type k = 0; k < vec.length(); k++) {
    tab.set(vec[k]->print_name, (void*)0);
    if (trace_print_context) {
      std::cerr << k << "th variable ";
      vec[k]->print(std::cerr);
      std::cerr << " cleared from context"
		<< std::endl;
    }
  }
}

void PDDL_Base::clear_context
(variable_vec& vec, index_type n_min, index_type n_max)
{
  assert((n_min <= n_max) && n_max <= vec.length());
  for (index_type k = n_min; k < n_max; k++) {
    tab.set(vec[k]->print_name, (void*)0);
    if (trace_print_context) {
      std::cerr << k << "th variable ";
      vec[k]->print(std::cerr);
      std::cerr << " cleared from context"
		<< std::endl;
    }
  }
}

bool PDDL_Base::merge_type_vectors
(type_vec& v0, type_vec& v1)
{
  if (v0.length() != v1.length()) return false;
  bool_vec set(false, v0.length());
  for (index_type k = 0; k < v0.length(); k++) {
    if (v1[k]->subtype_or_equal(v0[k])) {
      set[k] = false;
    }
    else if (v0[k]->subtype_or_equal(v1[k])) {
      set[k] = true;
    }
    else {
      return false;
    }
  }
  for (index_type k = 0; k < v0.length(); k++)
    if (set[k]) v0[k] = v1[k];
  return true;
}

void PDDL_Base::make_parameters
(type_vec& t, const char* prefix, variable_vec& v)
{
  v.set_length(t.length());
  for (index_type k = 0; k < t.length(); k++) {
    EnumName v_name(prefix, k);
    v[k] = new VariableSymbol(v_name.to_cstring(Name::NC_INSTANCE));
    v[k]->sym_types.assign_value(t[k], 1);
  }
}

PDDL_Base::Symbol* PDDL_Base::gensym
(symbol_class c, const char* p, const TypeSet& t)
{
  StringTable::Cell* sc = tab.inserta(p);
  index_type i = 0;
  while (sc->val) {
    std::ostringstream s;
    s << p << i++;
    sc = tab.inserta(s.str().c_str());
  }
  Symbol* s = 0;
  switch (c) {
  case sym_action:
    s = new ActionSymbol(sc->text);
    break;
  case sym_predicate:
    s = new PredicateSymbol(sc->text);
    break;
  case sym_variable:
    s = new VariableSymbol(sc->text);
    break;
  case sym_typename:
    s = new TypeSymbol(sc->text);
    break;
  default:
    s = new Symbol(c, sc->text);
  }
  sc->val = s;
  s->sym_types = t;
  return s;
}

PDDL_Base::Symbol* PDDL_Base::gensym
(symbol_class c, const char* p, TypeSymbol* t)
{
  assert(t != 0);
  TypeSet v(t);
  return gensym(c, p, v);
}

PDDL_Base::Symbol* PDDL_Base::gensym_i
(symbol_class c, const char* p, index_type i, TypeSymbol* t)
{
  std::ostringstream s;
  s << p << i;
  return gensym(c, s.str().c_str(), t);
}

PDDL_Base::Symbol* PDDL_Base::gensym_n
(symbol_class c, const char* p, const Name* n, TypeSymbol* t)
{
  std::ostringstream s;
  s << p << n;
  return gensym(c, s.str().c_str(), t);
}

PDDL_Base::Symbol* PDDL_Base::gensym_s
(symbol_class c, const char* p, const Symbol* s, TypeSymbol* t)
{
  std::ostringstream ss;
  ss << p << s->print_name;
  return gensym(c, ss.str().c_str(), t);
}

PDDL_Base::Atom* PDDL_Base::new_meta_atom(PredicateSymbol* p)
{
  Atom* a = new Atom(p);
  a->param.set_length(p->param.length());
  for (index_type k = 0; k < p->param.length(); k++) {
    a->param[k] = gensym(sym_meta_variable, "V", p->param[k]->sym_types);
  }
  return a;
}

void PDDL_Base::new_variable_substitution
(Atom* a, symbol_pair_vec& u, symbol_pair_vec& new_u)
{
  for (index_type k = 0; k < u.length(); k++) {
    if (u[k].second->sym_class == sym_variable) {
      new_u.append(symbol_pair(u[k].first,
			       gensym(sym_variable,
				      u[k].second->print_name,
				      u[k].second->sym_types)));
    }
    else {
      new_u.append(u[k]);
    }
  }
  for (index_type k = 0; k < a->param.length(); k++)
    if (a->param[k]->sym_class == sym_variable) {
      bool found = false;
      for (index_type i = 0; (i < new_u.length()) && !found; i++)
	if (new_u[i].first == a->param[k]) found = true;
      if (!found) {
	new_u.append(symbol_pair(a->param[k],
				 gensym(sym_variable,
					a->param[k]->print_name,
					a->param[k]->sym_types)));
      }
    }
}

PDDL_Base::CAtom* PDDL_Base::new_CAtom
(Atom* a, symbol_pair_vec& u)
{
  symbol_pair_vec new_u;
  new_variable_substitution(a, u, new_u);
  return new CAtom(a, new_u);
}

PDDL_Base::CAtom* PDDL_Base::new_CAtom
(Atom* a, symbol_pair_vec& n, symbol_pair_vec& u)
{
  symbol_pair_vec new_u;
  new_variable_substitution(a, u, new_u);
  return new CAtom(a, n, new_u);
}

PDDL_Base::CAtom::CAtom(const Atom* a, symbol_pair_vec& u)
  : Atom(a->pred), neq(0, 0)
{
  param.set_length(a->param.length());
  for (index_type k = 0; k < a->param.length(); k++) {
    bool found = false;
    for (index_type i = 0; (i < u.length()) && !found; i++)
      if (a->param[k] == u[i].first) {
	param[k] = u[i].second;
	found = true;
      }
    if (!found) {
      param[k] = a->param[k];
    }
  }
}

PDDL_Base::CAtom::CAtom(const Atom* a, symbol_pair_vec& n, symbol_pair_vec& u)
  : Atom(a->pred), neq(0, 0)
{
  param.set_length(a->param.length());
  for (index_type k = 0; k < a->param.length(); k++) {
    bool found = false;
    for (index_type i = 0; (i < u.length()) && !found; i++)
      if (a->param[k] == u[i].first) {
	param[k] = u[i].second;
	found = true;
      }
    if (!found) {
      param[k] = a->param[k];
    }
  }
  for (index_type k = 0; k < n.length(); k++) {
    Symbol* s1 = 0;
    Symbol* s2 = 0;
    for (index_type i = 0; i < u.length(); i++) {
      if ((s1 == 0) && (u[i].first == n[k].first)) s1 = u[i].second;
      if ((s2 == 0) && (u[i].first == n[k].second)) s2 = u[i].second;
    }
    if (s1 == 0) s1 = n[k].first;
    if (s2 == 0) s2 = n[k].second;
    neq.append(symbol_pair(s1, s2));
  }
  //   ((Atom*)a)->print(std::cerr);
  //   std::cerr << ", ";
  //   print_inequality(std::cerr, n);
  //   std::cerr << ", ";
  //   print_substitution(std::cerr, u);
  //   std::cerr << " => ";
  //   print(std::cerr);
  //   std::cerr << std::endl;
}

void PDDL_Base::extend_cc
(CAtom* lit,
 catom_vec& m,
 catom_vec& nm,
 lvector< swapable_pair<catom_vec> >& q,
 index_type d)
{
  symbol_pair_vec u;

  if (write_info) {
    std::cerr << "[" << d << "] extend_cc: lit = ";
    lit->print(std::cerr);
    std::cerr << ", m = {";
    for (index_type k = 0; k < m.length(); k++) {
      if (k > 0) std::cerr << ", ";
      m[k]->print(std::cerr);
    }
    std::cerr << "}, nm = {";
    for (index_type k = 0; k < nm.length(); k++) {
      if (k > 0) std::cerr << ", ";
      nm[k]->print(std::cerr);
    }
    std::cerr << "}, |q| = " << q.length()
	      << std::endl;
  }

  // if lit is an instance of any excluded atom (in nm), fail
  for (index_type k = 0; k < nm.length(); k++) {
    if (lit->unify(nm[k], u)) {
      if (write_info) {
	std::cerr << "[" << d << "] cut (1) due to ";
	nm[k]->print(std::cerr);
	std::cerr << ", ";
	print_substitution(std::cerr, u);
	std::cerr << std::endl;
      }
      return;
    }
  }

  bool lit_subsumed = false;
  for (index_type k = 0; (k < m.length()) && !lit_subsumed; k++)
    if (lit->instance_of(m[k], u)) {
      if (write_info) {
	std::cerr << "[" << d << "] ";
	lit->print(std::cerr);
	std::cerr << " subsumed by ";
	m[k]->print(std::cerr);
	std::cerr << ", ";
	print_substitution(std::cerr, u);
	std::cerr << std::endl;
      }
      lit_subsumed = true;
    }

  index_type reset_m_to = m.length();
  index_type reset_q_to = q.length();

  if (!lit_subsumed) {
    // find every action/effect that can add lit; add extension for each
    symbol_pair_vec actneq;
    for (index_type k = 0; k < dom_actions.length(); k++) {
      dom_actions[k]->get_param_inequalities(actneq);
      for (index_type i = 0; i < dom_actions[k]->adds.length(); i++) {
	if (lit->instance_of(dom_actions[k]->adds[i], actneq, u)) {
	  if (write_info) {
	    std::cerr << "[" << d << "] match with ";
	    dom_actions[k]->adds[i]->print(std::cerr);
	    std::cerr << " of " << dom_actions[k]->print_name << ", ";
	    print_inequality(std::cerr, actneq);
	    std::cerr << ", ";
	    print_substitution(std::cerr, u);
	    std::cerr << std::endl;
	  }
	  swapable_pair<catom_vec> e;
	  for (index_type j = 0; j < dom_actions[k]->cons.length(); j++) {
	    CAtom* xa = new_CAtom(dom_actions[k]->cons[j], actneq, u);
	    e.first.append(xa);
	  }
	  for (index_type j = 0; j < dom_actions[k]->adds.length(); j++) if (j != i) {
	    CAtom* nxa = new_CAtom(dom_actions[k]->adds[j], actneq, u);
	    e.second.append(nxa);
	  }
	  q.append(e);
	}
      }
    }
    m.append(lit);
  }

  if (q.length() == 0) {
    if (write_info) {
      std::cerr << "info: found invariant {";
      for (index_type k = 0; k < m.length(); k++) {
	if (k > 0) std::cerr << ", ";
	m[k]->print(std::cerr);
      }
      std::cerr << "}" << std::endl;
    }

    graph lsg(m.length());
    for (index_type i = 0; i < m.length(); i++)
      for (index_type j = 0; j < m.length(); j++) if (i != j) {
	if (m[i]->instance_of(m[j], u))
	  lsg.add_edge(j, i);
      }
    graph lsg_tree;
    lsg.strongly_connected_components();
    lsg.component_tree(lsg_tree);
    catom_vec mprime(0, 0);
    for (index_type k = 0; k < lsg_tree.size(); k++)
      if (lsg_tree.in_degree(k) == 0) {
	index_type i = lsg.component_node(k);
	mprime.append(m[i]);
      }

    bool is_exclusive = true;
    for (index_type i = 0; (i < mprime.length()) && is_exclusive; i++)
      for (index_type j = i + 1; (j < mprime.length()) && is_exclusive; j++)
	if (mprime[i]->unify(mprime[j], u))
	  is_exclusive = false;

    if (write_info) {
      std::cerr << "info: invariant reduced to {";
      for (index_type k = 0; k < mprime.length(); k++) {
	if (k > 0) std::cerr << ", ";
	mprime[k]->print(std::cerr);
      }
      std::cerr << "}" << std::endl;
    }

    symbol_set meta_vars;
    for (index_type k = 0; k < mprime.length(); k++)
      for (index_type p = 0; p < mprime[k]->param.length(); p++)
	if (mprime[k]->param[p]->sym_class == sym_meta_variable)
	  meta_vars.insert(mprime[k]->param[p]);

    SetConstraint* c = new SetConstraint();
    for (index_type k = 0; k < meta_vars.length(); k++)
      c->param.append((VariableSymbol*)gensym(sym_variable, "?v", meta_vars[k]->sym_types));

    for (index_type k = 0; k < mprime.length(); k++) {
      bool is_set = false;
      for (index_type i = 0; i < mprime[k]->param.length(); i++)
	if (mprime[k]->param[i]->sym_class == sym_variable)
	  is_set = true;

      if (is_set) {
	SetOf* s = new SetOf();
	s->pos_atoms.append(new Atom(mprime[k]->pred));
	for (index_type i = 0; i < mprime[k]->param.length(); i++) {
	  if (mprime[k]->param[i]->sym_class == sym_meta_variable) {
	    index_type j = meta_vars.first(mprime[k]->param[i]);
	    assert(j < c->param.length());
	    s->pos_atoms[0]->param.append(c->param[j]);
	  }
	  else {
	    if (mprime[k]->param[i]->sym_class == sym_variable) {
	      s->param.append((VariableSymbol*)mprime[k]->param[i]);
	    }
	    s->pos_atoms[0]->param.append(mprime[k]->param[i]);
	  }
	}
	for (index_type i = 0; i < mprime[k]->neq.length(); i++) {
	  if ((mprime[k]->neq[i].first->sym_class == sym_variable) ||
	      (mprime[k]->neq[i].second->sym_class == sym_variable)) {
	    if (mprime[k]->neq[i].first->sym_class == sym_meta_variable) {
	      index_type j1 = meta_vars.first(mprime[k]->neq[i].first);
	      index_type j2 = s->param.first((VariableSymbol*)mprime[k]->neq[i].second);
	      if ((j1 < c->param.length()) && (j2 != no_such_index)) {
		Atom* q = new Atom(dom_eq_pred);
		q->param.append(c->param[j1]);
		q->param.append(mprime[k]->neq[i].second);
		s->neg_con.append(q);
	      }
	    }
	    else if (mprime[k]->neq[i].second->sym_class == sym_meta_variable) {
	      index_type j1 = s->param.first((VariableSymbol*)mprime[k]->neq[i].first);
	      index_type j2 = meta_vars.first(mprime[k]->neq[i].second);
	      if ((j1 != no_such_index) && (j2 < c->param.length())) {
		Atom* q = new Atom(dom_eq_pred);
		q->param.append(mprime[k]->neq[i].first);
		q->param.append(c->param[j2]);
		s->neg_con.append(q);
	      }
	    }
	    else {
	      index_type j1 = s->param.first((VariableSymbol*)mprime[k]->neq[i].first);
	      index_type j2 = s->param.first((VariableSymbol*)mprime[k]->neq[i].second);
	      if (((j1 != no_such_index) ||
		   (mprime[k]->neq[i].first->sym_class == sym_object)) &&
		  ((j2 != no_such_index) ||
		   (mprime[k]->neq[i].second->sym_class == sym_object))) {
		Atom* q = new Atom(dom_eq_pred);
		q->param.append(mprime[k]->neq[i].first);
		q->param.append(mprime[k]->neq[i].second);
		s->neg_con.append(q);
	      }
	    }
	  }
	}
	c->atom_sets.append(s);
      }
      else {
	Atom* a = new Atom(mprime[k]->pred);
	for (index_type i = 0; i < mprime[k]->param.length(); i++) {
	  if (mprime[k]->param[i]->sym_class == sym_meta_variable) {
	    index_type j = meta_vars.first(mprime[k]->param[i]);
	    assert(j < c->param.length());
	    a->param.append(c->param[j]);
	  }
	  else {
	    assert(mprime[k]->param[i]->sym_class == sym_object);
	    a->param.append(mprime[k]->param[i]);
	  }
	}
	c->pos_atoms.append(a);

	for (index_type i = 0; i < mprime[k]->neq.length(); i++) {
	  index_type j1 = meta_vars.first(mprime[k]->neq[i].first);
	  index_type j2 = meta_vars.first(mprime[k]->neq[i].second);
	  if ((j1 < c->param.length()) && (j2 < c->param.length())) {
	    Atom* q = new Atom(dom_eq_pred);
	    q->param.append(c->param[j1]);
	    q->param.append(c->param[j2]);
	    c->neg_con.append(q);
	  }
	}
      }
    }

    c->sc_type = sc_at_most;
    c->sc_count = 1;
    c->item_tags.append((char*)"c-constraint");
    if (is_exclusive) {
      c->item_tags.append((char*)"is-exclusive");
    }

    if ((c->atom_sets.length() > 0) || (c->pos_atoms.length() > 1)) {
      dom_sc_invariants.append(c);
    }
    else if (write_info) {
      std::cerr << "info: useless invariant {";
      for (index_type k = 0; k < mprime.length(); k++) {
	if (k > 0) std::cerr << ", ";
	mprime[k]->print(std::cerr);
      }
      std::cerr << "} ignored" << std::endl;
    }

    m.set_length(reset_m_to);
    q.set_length(reset_q_to);
    return;
  }

  index_type last = q.length() - 1;
  catom_vec x(q[last].first);
  catom_vec not_x(q[last].second);

  if (write_info) {
    std::cerr << "[" << d << "] next: x = {";
    for (index_type k = 0; k < x.length(); k++) {
      if (k > 0) std::cerr << ", ";
      x[k]->print(std::cerr);
    }
    std::cerr << "}, nx = {";
    for (index_type k = 0; k < not_x.length(); k++) {
      if (k > 0) std::cerr << ", ";
      not_x[k]->print(std::cerr);
    }
    std::cerr << "}" << std::endl;
  }

  for (index_type k = 0; k < not_x.length(); k++)
    for (index_type i = 0; i < m.length(); i++)
      if (not_x[k]->unify(m[i], u)) {
	if (write_info) {
	  std::cerr << "[" << d << "] cut (2) due to ";
	  not_x[k]->print(std::cerr);
	  std::cerr << ", ";
	  m[i]->print(std::cerr);
	  std::cerr << ", ";
	  print_substitution(std::cerr, u);
	  std::cerr << std::endl;
	}
	m.set_length(reset_m_to);
	q.set_length(reset_q_to);
	return;
      }

  q.dec_length();
  index_type reset_nm_to = nm.length();
  nm.append(not_x);

  index_type local_reset_nm_to = nm.length();

  for (index_type k = 0; k < x.length(); k++) {
    bool ok = true;
    for (index_type i = 0; (i < x.length()) && ok; i++) if (i != k) {
      for (index_type j = 0; (j < m.length()) && ok; j++)
	if (x[i]->unify(m[j], u)) ok = false;
      if (ok) nm.append(x[i]);
    }
    if (ok) {
      extend_cc(x[k], m, nm, q, d + 1);
    }
    nm.set_length(local_reset_nm_to);
  }

  m.set_length(reset_m_to);
  nm.set_length(reset_nm_to);
  q.append(swapable_pair<catom_vec>(x, not_x));
  q.set_length(reset_q_to);

  if (write_info) {
    std::cerr << "[" << d << "] finished" << std::endl;
  }
}

void PDDL_Base::find_cc()
{
  catom_vec m(0, 0);
  catom_vec nm(0, 0);
  lvector< swapable_pair<catom_vec> > q;
  for (index_type k = 0; k < dom_predicates.length(); k++) {
    if (!dom_predicates[k]->is_static()) {
      CAtom* lit = new CAtom(new_meta_atom(dom_predicates[k]));
      m.assign_value(0, 0);
      nm.assign_value(0, 0);
      q.set_length(0);
      extend_cc(lit, m, nm, q, 0);
    }
  }
}

void PDDL_Base::TypeSymbol::add_element(Symbol* e)
{
  elements.append(e);
  for (index_type k = 0; k < sym_types.length(); k++)
    sym_types[k]->add_element(e);
}

bool PDDL_Base::TypeSymbol::subtype_or_equal(TypeSymbol* t) const
{
  if (t == 0) return true;
  if (t == this) return true;
  if (sym_types.subtype_or_equal(t))
    return true;
  return false;
}

bool PDDL_Base::TypeSymbol::subtype_or_equal(const TypeSet& t) const
{
  if (t.length() == 0) return true;
  for (index_type k = 0; k < t.length(); k++)
    if (!subtype_or_equal(t[k]))
      return false;
  return true;
}

bool PDDL_Base::VariableSymbol::equality_type_check(Symbol* s)
{
  if ((sym_types.length() > 0) && (s->sym_types.length() > 0)) {
    for (index_type i = 0; i < sym_types.length(); i++) {
      for (index_type j = 0; j < s->sym_types.length(); j++) {
	if (sym_types[i]->subtype_or_equal(s->sym_types[j]))
	  return true;
	if (s->sym_types[j]->subtype_or_equal(sym_types[i]))
	  return true;
      }
    }
    return false;
  }
  else { // this variable or s has no type, can equal anything
    return true;
  }
}

bool PDDL_Base::Expression::is_static()
{
  switch (exp_class) {
  case exp_fun:
    {
      FunctionSymbol* fun = ((FunctionExpression*)this)->fun;
      return fun->is_static();
    }
  case exp_list:
  case exp_time:
    {
      std::cerr << "error (is_static): expression ";
      print(std::cerr, false);
      std::cerr << " can not be evaluated" << std::endl;
      exit(255);
    }
  case exp_const:
    {
      return true;
    }
  case exp_add:
  case exp_sub:
  case exp_mul:
  case exp_div:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      return (bexp->first->is_static() && bexp->second->is_static());
    }
  case exp_preference:
    return false;
  }
  assert(0);
}

bool PDDL_Base::Expression::is_constant()
{
  switch (exp_class) {
  case exp_fun:
    {
      FunctionSymbol* fun = ((FunctionExpression*)this)->fun;
      return (fun->is_static() && (((FunctionExpression*)this)->args == 0));
    }
  case exp_list:
  case exp_time:
    {
      std::cerr << "error (is_constant): expression ";
      print(std::cerr, false);
      std::cerr << " can not be evaluated" << std::endl;
      exit(255);
    }
  case exp_const:
    {
      return true;
    }
  case exp_add:
  case exp_sub:
  case exp_mul:
  case exp_div:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      return (bexp->first->is_constant() && bexp->second->is_constant());
    }
  case exp_preference:
    return false;
  }
  assert(0);
}

bool PDDL_Base::Expression::is_integral()
{
  switch (exp_class) {
  case exp_fun:
    {
      FunctionSymbol* fun = ((FunctionExpression*)this)->fun;
      return fun->integral;
    }
  case exp_list:
  case exp_time:
    {
      std::cerr << "error (is_integral): expression ";
      print(std::cerr, false);
      std::cerr << " can not be evaluated" << std::endl;
      exit(255);
    }
  case exp_const:
    {
      return INTEGRAL(((ConstantExpression*)this)->val);
    }
  case exp_add:
  case exp_sub:
  case exp_mul:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      return (bexp->first->is_integral() && bexp->second->is_integral());
    }
  case exp_div:
    return false;
  case exp_preference:
    return true;
  }
  assert(0);
}

PDDL_Base::Expression* PDDL_Base::Expression::copy()
{
  switch (exp_class) {
  case exp_fun:
    {
      FunctionExpression* f_exp = (FunctionExpression*)this;
      if (f_exp->args) {
	ListExpression* args = (ListExpression*)f_exp->args->copy();
	return new FunctionExpression(f_exp->fun, args);
      }
      else {
	return new FunctionExpression(f_exp->fun, 0);
      }
    }
  case exp_list:
    {
      ListExpression* l_exp = (ListExpression*)this;
      if (l_exp->rest) {
	ListExpression* rest = (ListExpression*)l_exp->rest->copy();
	return new ListExpression(l_exp->sym, rest);
      }
      else {
	return new ListExpression(l_exp->sym, 0);
      }
    }
  case exp_time:
    {
      TimeExpression* t_exp = (TimeExpression*)this;
      if (t_exp->time_exp == 0)
	return new TimeExpression();
      else
	return new TimeExpression(t_exp->time_exp->copy());
    }
  case exp_const:
    {
      ConstantExpression* c_exp = (ConstantExpression*)this;
      return new ConstantExpression(c_exp->val);
    }
  case exp_add:
  case exp_sub:
  case exp_mul:
  case exp_div:
    {
      BinaryExpression* b_exp = (BinaryExpression*)this;
      return new BinaryExpression(exp_class, b_exp->first->copy(),
				  b_exp->second->copy());
    }
  case exp_preference:
    {
      PreferenceExpression* p_exp = (PreferenceExpression*)this;
      return new PreferenceExpression(p_exp->name);
    }
  }
  assert(0);
}

void PDDL_Base::Expression::substitute_for_time(Expression* e)
{
  switch (exp_class) {
  case exp_time:
    {
      ((TimeExpression*)this)->time_exp = e;
      break;
    }
  case exp_add:
  case exp_sub:
  case exp_mul:
  case exp_div:
    {
      BinaryExpression* b_exp = (BinaryExpression*)this;
      b_exp->first->substitute_for_time(e);
      b_exp->second->substitute_for_time(e);
    }
  }
}

PDDL_Base::Expression* PDDL_Base::Expression::substitute_for_preference
(Symbol* n, Expression* e)
{
  switch (exp_class) {
  case exp_preference:
    {
      PreferenceExpression* p_exp = (PreferenceExpression*)this;
      if (p_exp->name == n) {
	return e;
      }
      else {
	return this;
      }
    }
  case exp_add:
  case exp_sub:
  case exp_mul:
  case exp_div:
    {
      BinaryExpression* b_exp = (BinaryExpression*)this;
      b_exp->first = b_exp->first->substitute_for_preference(n, e);
      b_exp->second = b_exp->second->substitute_for_preference(n, e);
      return this;
    }
  default:
    return this;
  }
}

NTYPE PDDL_Base::Expression::eval_static()
{
  switch (exp_class) {
  case exp_fun:
    {
      FunctionSymbol* fun = ((FunctionExpression*)this)->fun;
      if (!fun->is_static()) {
	std::cerr << "error: function " << fun->print_name
		  << " is not static" << std::endl;
	exit(255);
      }
      ListExpression* args = ((FunctionExpression*)this)->args;
      ptr_table* r = &(fun->init);
      while (args && r) {
	if (args->sym->sym_class == sym_variable) {
	  if (((VariableSymbol*)args->sym)->value == 0) {
	    std::cerr << "error: unbound variable "
		      << args->sym->print_name << " in ";
	    print(std::cerr, true);
	    std::cerr << " - uncompiled object function?"
		      << std::endl;
	    exit(255);
	  }
	  r = r->find_next(((VariableSymbol*)args->sym)->value);
	}
	else {
	  r = r->find_next(args->sym);
	}
	if (!r) {
	  if (use_default_function_value) {
	    return default_function_value;
	  }
	  else {
	    std::cerr << "error: ";
	    print(std::cerr, true);
	    std::cerr << " is undefined" << std::endl;
	    exit(255);
	  }
	}
	args = args->rest;
      }
      if (!r->val) {
	if (use_default_function_value) {
	  return default_function_value;
	}
	else {
	  std::cerr << "error: ";
	  this->print(std::cerr, true);
	  std::cerr << " has no value (2)" << std::endl;
	  exit(255);
	}
      }
      return ((FInitAtom*)r->val)->val;
    }
  case exp_list:
    {
      std::cerr << "error: expression ";
      print(std::cerr, false);
      std::cerr << " can not be evaluated" << std::endl;
      exit(255);
    }
  case exp_time:
    {
      TimeExpression* texp = (TimeExpression*)this;
      if (texp->time_exp == 0) {
	std::cerr << "error (eval_static): expression ";
	print(std::cerr, false);
	std::cerr << " can not be evaluated" << std::endl;
	exit(255);
      }
      return texp->time_exp->eval_static();
    }
  case exp_const:
    {
      ConstantExpression* cexp = (ConstantExpression*)this;
      return cexp->val;
    }
  case exp_add:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_static();
      NTYPE v2 = bexp->second->eval_static();
      return v1 + v2;
    }
  case exp_sub:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_static();
      NTYPE v2 = bexp->second->eval_static();
      return v1 - v2;
    }
  case exp_mul:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_static();
      NTYPE v2 = bexp->second->eval_static();
      return v1 * v2;
    }
  case exp_div:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_static();
      NTYPE v2 = bexp->second->eval_static();
      return v1 / v2;
    }
  case exp_preference:
    {
      return 0;
    }
  }
  assert(0);
}

bool PDDL_Base::Expression::eval_partial(NTYPE& val)
{
  switch (exp_class) {
  case exp_fun:
    {
      FunctionSymbol* fun = ((FunctionExpression*)this)->fun;
      if (!fun->is_static()) {
	return false;
      }
      ListExpression* args = ((FunctionExpression*)this)->args;
      ptr_table* r = &(fun->init);
      while (args && r) {
	if (args->sym->sym_class == sym_variable) {
	  if (((VariableSymbol*)args->sym)->value == 0) {
	    std::cerr << "error: unbound variable "
		      << args->sym->print_name << " in ";
	    print(std::cerr, true);
	    std::cerr << " - uncompiled object function?"
		      << std::endl;
	    exit(255);
	  }
	  r = r->find_next(((VariableSymbol*)args->sym)->value);
	}
	else {
	  r = r->find_next(args->sym);
	}
	if (!r) {
	  return false;
	}
	args = args->rest;
      }
      if (!r->val) {
	return false;
      }
      val = ((FInitAtom*)r->val)->val;
      return true;
    }
  case exp_list:
    {
      std::cerr << "error: expression ";
      print(std::cerr, false);
      std::cerr << " can not be evaluated" << std::endl;
      exit(255);
    }
  case exp_time:
    {
      TimeExpression* texp = (TimeExpression*)this;
      if (texp->time_exp == 0) {
	std::cerr << "error (eval_partial): expression ";
	print(std::cerr, false);
	std::cerr << " can not be evaluated" << std::endl;
	exit(255);
      }
      return texp->time_exp->eval_partial(val);
    }
  case exp_const:
    {
      ConstantExpression* cexp = (ConstantExpression*)this;
      val = cexp->val;
      return true;
    }
  case exp_add:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1;
      if (!bexp->first->eval_partial(v1)) return false;
      NTYPE v2;
      if (!bexp->first->eval_partial(v2)) return false;
      val = (v1 + v2);
      return true;
    }
  case exp_sub:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1;
      if (!bexp->first->eval_partial(v1)) return false;
      NTYPE v2;
      if (!bexp->first->eval_partial(v2)) return false;
      val = (v1 - v2);
      return true;
    }
  case exp_mul:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1;
      if (!bexp->first->eval_partial(v1)) return false;
      NTYPE v2;
      if (!bexp->first->eval_partial(v2)) return false;
      val = (v1 * v2);
      return true;
    }
  case exp_div:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1;
      if (!bexp->first->eval_partial(v1)) return false;
      NTYPE v2;
      if (!bexp->first->eval_partial(v2)) return false;
      val = (v1 / v2);
      return true;
    }
  case exp_preference:
    {
      return false;
    }
  }
  assert(0);
}

interval PDDL_Base::FunctionSymbol::eval_init_bounds
(ptr_table* p, index_type i, ListExpression* r)
{
  if (!p) {
    if (use_default_function_value) {
      return interval(default_function_value);
    }
    else {
      return interval(NEG_INF, POS_INF);
    }
  }
  if (r == 0) {
    if (!p->val) {
      ptr_table::key_vec* k = p->key_sequence();
      std::cerr << "error: key sequence " << k
		<< " stored without value in function ";
      print(std::cerr);
      std::cerr << " init table"
		<< std::endl;
      exit(255);
    }
    return interval(((FInitAtom*)p->val)->val);
  }
  else if (r->sym->sym_class == sym_variable) {
    if (((VariableSymbol*)r->sym)->value) {
      return eval_init_bounds(p->find_next(((VariableSymbol*)r->sym)->value), i + 1, r->rest);
    }
    else {
      NTYPE v_min = POS_INF;
      NTYPE v_max = NEG_INF;
      for (index_type k = 0; k < param[i]->sym_types.n_elements(); k++) {
	interval v =
	  eval_init_bounds(p->find_next(param[i]->sym_types.get_element(k)),
			   i + 1, r->rest);
	v_min = MIN(v_min, v.first);
	v_max = MAX(v_max, v.second);
      }
      return interval(v_min, v_max);
    }
  }
  else {
    return eval_init_bounds(p->find_next(r->sym), i + 1, r->rest);
  }
}

interval PDDL_Base::Expression::eval_bounds()
{
  switch (exp_class) {
  case exp_fun:
    {
      FunctionSymbol* fun = ((FunctionExpression*)this)->fun;
      if (!fun->is_static()) {
	return interval(NEG_INF, POS_INF);
      }
      ListExpression* args = ((FunctionExpression*)this)->args;
      ptr_table* p = &(fun->init);
      return fun->eval_init_bounds(p, 0, args);
    }
  case exp_list:
    {
      std::cerr << "error: expression ";
      print(std::cerr, false);
      std::cerr << " can not be evaluated" << std::endl;
      exit(255);
    }
  case exp_time:
    {
      TimeExpression* texp = (TimeExpression*)this;
      if (texp->time_exp == 0) {
	std::cerr << "error (eval_bounds): expression ";
	print(std::cerr, false);
	std::cerr << " can not be evaluated" << std::endl;
	exit(255);
      }
      return texp->time_exp->eval_bounds();
    }
  case exp_const:
    {
      ConstantExpression* cexp = (ConstantExpression*)this;
      return interval(cexp->val);
    }
  case exp_add:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      interval v1 = bexp->first->eval_bounds();
      interval v2 = bexp->second->eval_bounds();
      return interval(v1.first + v2.first, v1.second + v2.second);
    }
  case exp_sub:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      interval v1 = bexp->first->eval_bounds();
      interval v2 = bexp->second->eval_bounds();
      return interval(v1.first - v2.second, v1.second - v2.first);
    }
  case exp_mul:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      interval v1 = bexp->first->eval_bounds();
      interval v2 = bexp->second->eval_bounds();
      return interval(MIN(MIN(v1.first * v2.first, v1.first * v2.second),
			  MIN(v1.second * v2.first, v1.second * v2.second)),
		      MAX(MAX(v1.first * v2.first, v1.first * v2.second),
			  MAX(v1.second * v2.first, v1.second * v2.second)));
    }
  case exp_div:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      if (bexp->second->is_static()) {
	interval v1 = bexp->first->eval_bounds();
	NTYPE v2 = bexp->second->eval_static();
	return interval(v1.first / v2, v1.second / v2);
      }
      else if (bexp->first->is_static()) {
	NTYPE v1 = bexp->first->eval_static();
	interval v2 = bexp->second->eval_bounds();
	return interval(v1 / v2.second, v1 / v2.first);
      } 
      else {
	std::cerr << "error: in eval_bounds(";
	bexp->print(std::cerr, false);
	std::cerr << "): interval division not implemented"
		  << std::endl;
	exit(255);
      }
    }
  case exp_preference:
    {
      std::cerr << "error: eval_bounds(PreferenceExpression) not implemented"
		<< std::endl;
      exit(255);      
    }
  }
  assert(0);
}

NTYPE PDDL_Base::Expression::eval_init()
{
  switch (exp_class) {
  case exp_fun:
    {
      FunctionSymbol* fun = ((FunctionExpression*)this)->fun;
      ListExpression* args = ((FunctionExpression*)this)->args;
      ptr_table* r = &(fun->init);
      while (args && r) {
	if (args->sym->sym_class == sym_variable) {
	  if (((VariableSymbol*)args->sym)->value == 0) {
	    std::cerr << "error: unbound variable "
		      << args->sym->print_name << " in ";
	    print(std::cerr, true);
	    std::cerr << " - uncompiled object function?"
		      << std::endl;
	    exit(255);
	  }
	  r = r->find_next(((VariableSymbol*)args->sym)->value);
	}
	else {
	  r = r->find_next(args->sym);
	}
	if (!r) {
	  if (use_default_function_value) {
	    return default_function_value;
	  }
	  else {
	    std::cerr << "error: ";
	    print(std::cerr, true);
	    std::cerr << " is undefined" << std::endl;
	    exit(255);
	  }
	}
	args = args->rest;
      }
      if (!r->val) {
	if (use_default_function_value) {
	  return default_function_value;
	}
	else {
	  std::cerr << "error: ";
	  this->print(std::cerr, true);
	  std::cerr << " has no value (2)" << std::endl;
	  exit(255);
	}
      }
      return ((FInitAtom*)r->val)->val;
    }
  case exp_list:
    {
      std::cerr << "error: expression ";
      print(std::cerr, false);
      std::cerr << " can not be evaluated" << std::endl;
      exit(255);
    }
  case exp_time:
    {
      TimeExpression* texp = (TimeExpression*)this;
      if (texp->time_exp == 0) {
	std::cerr << "error (eval_static): expression ";
	print(std::cerr, false);
	std::cerr << " can not be evaluated" << std::endl;
	exit(255);
      }
      return texp->time_exp->eval_static();
    }
  case exp_const:
    {
      ConstantExpression* cexp = (ConstantExpression*)this;
      return cexp->val;
    }
  case exp_add:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_init();
      NTYPE v2 = bexp->second->eval_init();
      return v1 + v2;
    }
  case exp_sub:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_init();
      NTYPE v2 = bexp->second->eval_init();
      return v1 - v2;
    }
  case exp_mul:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_init();
      NTYPE v2 = bexp->second->eval_init();
      return v1 * v2;
    }
  case exp_div:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_init();
      NTYPE v2 = bexp->second->eval_init();
      return v1 / v2;
    }
  case exp_preference:
    {
      // here we should actually check if the preference is violated
      // in the initial state or not, but we assume it is not...
      return 0;
    }
  }
  assert(0);
}

NTYPE PDDL_Base::Expression::eval_delta
(ch_atom_vec& incs, ch_atom_vec& decs)
{
  switch (exp_class) {
  case exp_fun:
    {
      return ((FunctionExpression*)this)->eval_delta(incs, decs);
    }
  case exp_list:
    {
      std::cerr << "error (eval_delta): expression ";
      print(std::cerr, false);
      std::cerr << " can not be evaluated" << std::endl;
      exit(255);
    }
  case exp_time:
    {
      // TimeExpression* texp = (TimeExpression*)this;
      // if (texp->time_exp == 0) {
      // 	std::cerr << "error (eval_delta): expression ";
      // 	print(std::cerr, false);
      // 	std::cerr << " can not be evaluated" << std::endl;
      // 	exit(255);
      // }
      // return texp->time_exp->eval_static();
      return 0;
    }
  case exp_const:
    {
      // ConstantExpression* cexp = (ConstantExpression*)this;
      // return cexp->val;
      return 0; // delta of a constant is zero!
    }
  case exp_add:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_delta(incs, decs);
      NTYPE v2 = bexp->second->eval_delta(incs, decs);
      return v1 + v2;
    }
  case exp_sub:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_delta(incs, decs);
      NTYPE v2 = bexp->second->eval_delta(incs, decs);
      return v1 - v2;
    }
  case exp_mul:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      if (bexp->first->is_static()) {
	if (bexp->second->is_static()) {
	  return 0;
	}
	else {
	  NTYPE v1 = bexp->first->eval_static();
	  NTYPE v2 = bexp->second->eval_delta(incs, decs);
	  return v1 * v2;
	}
      }
      else if (bexp->second->is_static()) {
	NTYPE v1 = bexp->first->eval_delta(incs, decs);
	NTYPE v2 = bexp->second->eval_static();
	return v1 * v2;
      }
      else {
	std::cerr << "error (eval_delta): expression ";
	print(std::cerr, false);
	std::cerr << " is non-linear" << std::endl;
	exit(255);
	// NTYPE v1 = bexp->first->eval_delta(incs, decs);
	// NTYPE v2 = bexp->second->eval_delta(incs, decs);
	// return v1 * v2;
      }
    }
  case exp_div:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      if (bexp->first->is_static()) {
	if (bexp->second->is_static()) {
	  return 0;
	}
	else {
	  NTYPE v1 = bexp->first->eval_static();
	  NTYPE v2 = bexp->second->eval_delta(incs, decs);
	  return v1 / v2;
	}
      }
      else if (bexp->second->is_static()) {
	NTYPE v1 = bexp->first->eval_delta(incs, decs);
	NTYPE v2 = bexp->second->eval_static();
	return v1 / v2;
      }
      else {
	std::cerr << "error (eval_delta): expression ";
	print(std::cerr, false);
	std::cerr << " is non-linear" << std::endl;
	exit(255);
	// NTYPE v1 = bexp->first->eval_delta(incs, decs);
	// NTYPE v2 = bexp->second->eval_delta(incs, decs);
	// return v1 / v2;
      }
    }
  case exp_preference:
    return 0;
  }
  assert(0);
}

NTYPE PDDL_Base::Expression::eval_delta
(Symbol* preference, NTYPE p_value, NTYPE d_value)
{
  switch (exp_class) {
  case exp_fun:
    {
      FunctionSymbol* fun = ((FunctionExpression*)this)->fun;
      ListExpression* args = ((FunctionExpression*)this)->args;
      ptr_table* r = &(fun->init);
      while (args && r) {
	if (args->sym->sym_class == sym_variable) {
	  if (((VariableSymbol*)args->sym)->value == 0) {
	    std::cerr << "error: unbound variable "
		      << args->sym->print_name << " in ";
	    print(std::cerr, true);
	    std::cerr << " - uncompiled object function?"
		      << std::endl;
	    exit(255);
	  }
	  r = r->find_next(((VariableSymbol*)args->sym)->value);
	}
	else {
	  r = r->find_next(args->sym);
	}
	if (!r) {
	  std::cerr << "error: ";
	  print(std::cerr, true);
	  std::cerr << " has no value (1)" << std::endl;
	  exit(255);
	}
	args = args->rest;
      }
      if (!r->val) {
	std::cerr << "error: ";
	this->print(std::cerr, true);
	std::cerr << " has no value (2)" << std::endl;
	exit(255);
      }
      return ((FInitAtom*)r->val)->val;
    }
  case exp_list:
    {
      std::cerr << "error (eval_delta): expression ";
      print(std::cerr, false);
      std::cerr << " can not be evaluated" << std::endl;
      exit(255);
    }
  case exp_time:
    {
      TimeExpression* texp = (TimeExpression*)this;
      if (texp->time_exp == 0) {
	std::cerr << "error (eval_static): expression ";
	print(std::cerr, false);
	std::cerr << " can not be evaluated" << std::endl;
	exit(255);
      }
      return texp->time_exp->eval_static();
    }
  case exp_const:
    {
      ConstantExpression* cexp = (ConstantExpression*)this;
      return cexp->val;
    }
  case exp_add:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_delta(preference, p_value, d_value);
      NTYPE v2 = bexp->second->eval_delta(preference, p_value, d_value);
      return v1 + v2;
    }
  case exp_sub:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      NTYPE v1 = bexp->first->eval_delta(preference, p_value, d_value);
      NTYPE v2 = bexp->second->eval_delta(preference, p_value, d_value);
      return v1 - v2;
    }
  case exp_mul:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      if (bexp->first->is_static()) {
	if (bexp->second->is_static()) {
	  NTYPE v1 = bexp->first->eval_static();
	  NTYPE v2 = bexp->second->eval_static();
	  return v1 * v2;
	}
	else {
	  NTYPE v1 = bexp->first->eval_static();
	  NTYPE v2 = bexp->second->eval_delta(preference, p_value, d_value);
	  return v1 * v2;
	}
      }
      else if (bexp->second->is_static()) {
	NTYPE v1 = bexp->first->eval_delta(preference, p_value, d_value);
	NTYPE v2 = bexp->second->eval_static();
	return v1 * v2;
      }
      else {
	std::cerr << "error (eval_delta): expression ";
	print(std::cerr, false);
	std::cerr << " is non-linear" << std::endl;
	exit(255);
	// NTYPE v1 = bexp->first->eval_delta(preference, p_value, d_value);
	// NTYPE v2 = bexp->second->eval_delta(preference, p_value, d_value);
	// return v1 * v2;
      }
    }
  case exp_div:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      if (bexp->first->is_static()) {
	if (bexp->second->is_static()) {
	  NTYPE v1 = bexp->first->eval_static();
	  NTYPE v2 = bexp->second->eval_static();
	  return v1 / v2;
	}
	else {
	  NTYPE v1 = bexp->first->eval_static();
	  NTYPE v2 = bexp->second->eval_delta(preference, p_value, d_value);
	  return v1 / v2;
	}
      }
      else if (bexp->second->is_static()) {
	NTYPE v1 = bexp->first->eval_delta(preference, p_value, d_value);
	NTYPE v2 = bexp->second->eval_static();
	return v1 / v2;
      }
      else {
	std::cerr << "error (eval_delta): expression ";
	print(std::cerr, false);
	std::cerr << " is non-linear" << std::endl;
	exit(255);
	// NTYPE v1 = bexp->first->eval_delta(preference, p_value, d_value);
	// NTYPE v2 = bexp->second->eval_delta(preference, p_value, d_value);
	// return v1 / v2;
      }
    }
  case exp_preference:
    {
      PreferenceExpression* pexp = (PreferenceExpression*)this;
      if (pexp->name == preference) {
	return p_value;
      }
      else {
	return d_value;
      }
    }
  }
  assert(0);
}

bool PDDL_Base::Expression::equals(PDDL_Base::Expression* exp)
{
  if (!exp) return false;
  if (exp_class != exp->exp_class) return false;
  switch (exp_class) {
  case exp_fun:
    {
      FunctionExpression* f_this = (FunctionExpression*)this;
      FunctionExpression* f_exp  = (FunctionExpression*)exp;
      if (f_this->fun != f_exp->fun) return false;
      if (f_this->args)
	return f_this->args->equals(f_exp->args);
      else
	return f_exp->args == 0;
    }
  case exp_list:
    {
      ListExpression* l_this = (ListExpression*)this;
      ListExpression* l_exp  = (ListExpression*)exp;
      if (l_this->sym != l_exp->sym) return false;
      if (l_this->rest)
	return l_this->rest->equals(l_exp->rest);
      else
	return l_exp->rest == 0;
    }
  case exp_const:
    {
      ConstantExpression* c_this = (ConstantExpression*)this;
      ConstantExpression* c_exp  = (ConstantExpression*)exp;
      return c_this->val == c_exp->val;
    }
  case exp_time:
    {
      TimeExpression* t_this = (TimeExpression*)this;
      TimeExpression* t_exp  = (TimeExpression*)exp;
      return t_this->time_exp == t_exp->time_exp;
    }
  case exp_add:
  case exp_sub:
  case exp_mul:
  case exp_div:
    {
      BinaryExpression* b_this = (BinaryExpression*)this;
      BinaryExpression* b_exp  = (BinaryExpression*)exp;
      return (b_this->first->equals(b_exp->first) &&
	      b_this->second->equals(b_exp->second));
    }
  }
}

PDDL_Base::Expression* PDDL_Base::Expression::simplify()
{
  switch (exp_class) {
  case exp_fun:
  case exp_list:
  case exp_time:
  case exp_const:
  case exp_preference:
    return this;
  case exp_add:
    {
      if (is_static()) return new ConstantExpression(eval_static());
      BinaryExpression* b_exp = (BinaryExpression*)this;
      b_exp->first = b_exp->first->simplify();
      b_exp->second = b_exp->second->simplify();
      if ((b_exp->first->exp_class == exp_add) &&
	  (b_exp->second->exp_class == exp_add)) {
	BinaryExpression* b1 = (BinaryExpression*)(b_exp->first);
	BinaryExpression* b2 = (BinaryExpression*)(b_exp->second);
	if (b1->first->is_static()) {
	  if (b2->first->is_static()) {
	    NTYPE v1 = b1->first->eval_static();
	    NTYPE v2 = b2->first->eval_static();
	    b1->first = b2->second;
	    b_exp->second = new ConstantExpression(v1 + v2);
	  }
	  else if (b2->second->is_static()) {
	    NTYPE v1 = b1->first->eval_static();
	    NTYPE v2 = b2->second->eval_static();
	    b1->first = b2->first;
	    b_exp->second = new ConstantExpression(v1 + v2);
	  }
	}
	else if (b1->second->is_static()) {
	  if (b2->first->is_static()) {
	    NTYPE v1 = b1->second->eval_static();
	    NTYPE v2 = b2->first->eval_static();
	    b1->second = b2->second;
	    b_exp->second = new ConstantExpression(v1 + v2);
	  }
	  else if (b2->second->is_static()) {
	    NTYPE v1 = b1->second->eval_static();
	    NTYPE v2 = b2->second->eval_static();
	    b1->second = b2->first;
	    b_exp->second = new ConstantExpression(v1 + v2);
	  }
	}
      }
      else if ((b_exp->first->exp_class == exp_add) &&
	       (b_exp->second->is_static())) {
	BinaryExpression* b1 = (BinaryExpression*)(b_exp->first);
	if (b1->first->is_static()) {
	  NTYPE v1 = b1->first->eval_static();
	  NTYPE v2 = b_exp->second->eval_static();
	  b_exp->first = b1->second;
	  b_exp->second = new ConstantExpression(v1 + v2);
	}
	else if (b1->second->is_static()) {
	  NTYPE v1 = b1->second->eval_static();
	  NTYPE v2 = b_exp->second->eval_static();
	  b_exp->first = b1->first;
	  b_exp->second = new ConstantExpression(v1 + v2);
	}
      }
      else if ((b_exp->second->exp_class == exp_add) &&
	       (b_exp->first->is_static())) {
	BinaryExpression* b1 = (BinaryExpression*)(b_exp->second);
	if (b1->first->is_static()) {
	  NTYPE v1 = b1->first->eval_static();
	  NTYPE v2 = b_exp->first->eval_static();
	  b_exp->first = b1->second;
	  b_exp->second = new ConstantExpression(v1 + v2);
	}
	else if (b1->second->is_static()) {
	  NTYPE v1 = b1->second->eval_static();
	  NTYPE v2 = b_exp->first->eval_static();
	  b_exp->first = b1->first;
	  b_exp->second = new ConstantExpression(v1 + v2);
	}
      }
      return this;
    }
  case exp_sub:
    {
      if (is_static()) return new ConstantExpression(eval_static());
      BinaryExpression* b_exp = (BinaryExpression*)this;
      Expression* second =
	new BinaryExpression(exp_mul,
			     new ConstantExpression(-1),
			     b_exp->second->simplify());
      Expression* e =
	new BinaryExpression(exp_add,
			     b_exp->first->simplify(),
			     second->simplify());
      return e->simplify();
    }
  case exp_mul:
    {
      if (is_static()) return new ConstantExpression(eval_static());
      BinaryExpression* b_exp = (BinaryExpression*)this;
      b_exp->first = b_exp->first->simplify();
      b_exp->second = b_exp->second->simplify();
      if (b_exp->first->exp_class == exp_add) {
	BinaryExpression* b1 = (BinaryExpression*)(b_exp->first);
	Expression* e1 =
	  new BinaryExpression(exp_mul, b1->first, b_exp->second);
	Expression* e2 =
	  new BinaryExpression(exp_mul, b1->second, b_exp->second);
	Expression* e =
	  new BinaryExpression(exp_add, e1->simplify(), e2->simplify());
	return e->simplify();
      }
      else if (b_exp->second->exp_class == exp_add) {
	BinaryExpression* b1 = (BinaryExpression*)(b_exp->second);
	Expression* e1 =
	  new BinaryExpression(exp_mul, b1->first, b_exp->first);
	Expression* e2 =
	  new BinaryExpression(exp_mul, b1->second, b_exp->first);
	Expression* e =
	  new BinaryExpression(exp_add, e1->simplify(), e2->simplify());
	return e->simplify();
      }
      else {
	if ((b_exp->first->exp_class == exp_mul) &&
	    (b_exp->second->exp_class == exp_mul)) {
	  BinaryExpression* b1 = (BinaryExpression*)(b_exp->first);
	  BinaryExpression* b2 = (BinaryExpression*)(b_exp->second);
	  if (b1->first->is_static()) {
	    if (b2->first->is_static()) {
	      NTYPE v1 = b1->first->eval_static();
	      NTYPE v2 = b2->first->eval_static();
	      if ((v1 * v2) == 1) {
		b_exp->first = b1->second;
		b_exp->second = b2->second;
	      }
	      else {
		b1->first = b2->second;
		b_exp->second = new ConstantExpression(v1 * v2);
	      }
	    }
	    else if (b2->second->is_static()) {
	      NTYPE v1 = b1->first->eval_static();
	      NTYPE v2 = b2->second->eval_static();
	      if ((v1 * v2) == 1) {
		b_exp->first = b1->second;
		b_exp->second = b2->first;
	      }
	      else {
		b1->first = b2->first;
		b_exp->second = new ConstantExpression(v1 * v2);
	      }
	    }
	  }
	  else if (b1->second->is_static()) {
	    if (b2->first->is_static()) {
	      NTYPE v1 = b1->second->eval_static();
	      NTYPE v2 = b2->first->eval_static();
	      if ((v1 * v2) == 1) {
		b_exp->first = b1->first;
		b_exp->second = b2->second;
	      }
	      else {
		b1->second = b2->second;
		b_exp->second = new ConstantExpression(v1 * v2);
	      }
	    }
	    else if (b2->second->is_static()) {
	      NTYPE v1 = b1->second->eval_static();
	      NTYPE v2 = b2->second->eval_static();
	      if ((v1 * v2) == 1) {
		b_exp->first = b1->first;
		b_exp->second = b2->first;
	      }
	      else {
		b1->second = b2->first;
		b_exp->second = new ConstantExpression(v1 * v2);
	      }
	    }
	  }
	}
	else if ((b_exp->first->exp_class == exp_mul) &&
		 (b_exp->second->is_static())) {
	  BinaryExpression* b1 = (BinaryExpression*)(b_exp->first);
	  if (b1->first->is_static()) {
	    NTYPE v1 = b1->first->eval_static();
	    NTYPE v2 = b_exp->second->eval_static();
	    if ((v1 * v2) == 1) {
	      return b1->second;
	    }
	    else {
	      b_exp->first = b1->second;
	      b_exp->second = new ConstantExpression(v1 * v2);
	    }
	  }
	  else if (b1->second->is_static()) {
	    NTYPE v1 = b1->second->eval_static();
	    NTYPE v2 = b_exp->second->eval_static();
	    if ((v1 * v2) == 1) {
	      return b1->first;
	    }
	    else {
	      b_exp->first = b1->first;
	      b_exp->second = new ConstantExpression(v1 * v2);
	    }
	  }
	}
	else if ((b_exp->second->exp_class == exp_mul) &&
		 (b_exp->first->is_static())) {
	  BinaryExpression* b1 = (BinaryExpression*)(b_exp->second);
	  if (b1->first->is_static()) {
	    NTYPE v1 = b1->first->eval_static();
	    NTYPE v2 = b_exp->first->eval_static();
	    if ((v1 * v2) == 1) {
	      return b1->second;
	    }
	    else {
	      b_exp->first = b1->second;
	      b_exp->second = new ConstantExpression(v1 * v2);
	    }
	  }
	  else if (b1->second->is_static()) {
	    NTYPE v1 = b1->second->eval_static();
	    NTYPE v2 = b_exp->first->eval_static();
	    if ((v1 * v2) == 1) {
	      return b1->first;
	    }
	    else {
	      b_exp->first = b1->first;
	      b_exp->second = new ConstantExpression(v1 * v2);
	    }
	  }
	}
	return this;
      }
    }
  case exp_div:
    {
      if (is_static()) return new ConstantExpression(eval_static());
      BinaryExpression* b_exp = (BinaryExpression*)this;
      b_exp->first = b_exp->first->simplify();
      b_exp->second = b_exp->second->simplify();
      if (b_exp->first->exp_class == exp_add) {
	BinaryExpression* b1 = (BinaryExpression*)(b_exp->first);
	Expression* e1 =
	  new BinaryExpression(exp_div, b1->first, b_exp->second);
	Expression* e2 =
	  new BinaryExpression(exp_div, b1->second, b_exp->second);
	Expression* e =
	  new BinaryExpression(exp_add, e1->simplify(), e2->simplify());
	return e->simplify();
      }
      else if (b_exp->second->exp_class == exp_add) {
	BinaryExpression* b1 = (BinaryExpression*)(b_exp->second);
	Expression* e1 =
	  new BinaryExpression(exp_div, b1->first, b_exp->first);
	Expression* e2 =
	  new BinaryExpression(exp_div, b1->second, b_exp->first);
	Expression* e =
	  new BinaryExpression(exp_add, e1->simplify(), e2->simplify());
	return e->simplify();
      }
      else {
	return this;
      }
    }
  }
  assert(0);
}

void PDDL_Base::Expression::collect_constants(exp_vec& c)
{
  switch (exp_class) {
  case exp_const:
    c.append(this);
    break;
  case exp_add:
  case exp_sub:
  case exp_mul:
  case exp_div:
    ((BinaryExpression*)this)->first->collect_constants(c);
    ((BinaryExpression*)this)->second->collect_constants(c);
    break;
  }
}

NTYPE PDDL_Base::Expression::integrify()
{
  exp_vec cexp(0, 0);
  collect_constants(cexp);

  if (cexp.length() == 0) return 1;

#ifdef NTYPE_RATIONAL
  svector<long> factors;
  for (index_type k = 0; k < cexp.length(); k++)
    factors.insert(((ConstantExpression*)cexp[k])->val.divisor());

  long common_m = factors[0];
  for (index_type k = 1; k < factors.length(); k++) {
    std::cerr << "lcm(" << common_m << ", " << factors[k] << ") = "
	      << lcm(common_m, factors[k]) << std::endl;
    common_m = lcm(common_m, factors[k]);
  }

  std::cerr << "factors = " << factors
	    << ", lcm = " << common_m
	    << std::endl;

  for (index_type k = 0; k < cexp.length(); k++) {
    NTYPE v0 = ((ConstantExpression*)cexp[k])->val;
    index_type i = factors.first(v0.divisor());
    assert(i < factors.length());
    NTYPE v1(v0.numerator() * (common_m / v0.divisor()), 1);
    std::cerr << "replacing " << v0 << " by " << v1 << std::endl;
    ((ConstantExpression*)cexp[k])->val = v1;
  }

  return common_m;
#else
  std::cerr << "error: Expression::integrify not implemented for NTYPE_FLOAT"
	    << std::endl;
  exit(255);
#endif
}

bool PDDL_Base::ListExpression::match(AtomBase* atom)
{
  ListExpression* f = this;
  index_type n = 0;
  while (f && (n < atom->param.length())) {
    if (f->sym->sym_class == sym_variable) {
      if (atom->param[n]->sym_class == sym_variable) {
	if (f->sym != atom->param[n]) return false;
      }
      else {
	if (((VariableSymbol*)f->sym)->value != atom->param[n]) return false;
      }
    }
    else {
      if (atom->param[n]->sym_class == sym_variable) {
	if (f->sym != ((VariableSymbol*)atom->param[n])->value) return false;
      }
      else {
	if (f->sym != atom->param[n]) return false;
      }
    }
    f = f->rest;
    n += 1;
  }
  return ((f == 0) && (n == atom->param.length()));
}

bool PDDL_Base::FunctionExpression::match(FChangeAtom* atom)
{
  if (fun != atom->fun) return false;
  if (args) {
    return args->match(atom);
  }
  else {
    return (atom->param.length() == 0);
  }
}

NTYPE PDDL_Base::FunctionExpression::eval_delta
(ch_atom_vec& incs, ch_atom_vec& decs)
{
  NTYPE delta(0);
  for (index_type k = 0; k < incs.length(); k++) {
    if (match(incs[k])) delta += incs[k]->val->eval_static();
  }
  for (index_type k = 0; k < decs.length(); k++) {
    if (match(decs[k])) delta -= decs[k]->val->eval_static();
  }
  return delta;
}

PDDL_Base::FChangeAtom* PDDL_Base::FunctionExpression::make_atom_base()
{
  FChangeAtom* atom = new FChangeAtom(fun);
  for (ListExpression* l = args; l != 0; l = l->rest) {
    atom->param.append(l->sym);
  }
  return atom;
}

PDDL_Base::Expression* PDDL_Base::Relation::match_gteq_constant
(FChangeAtom* atom)
{
  if ((at == md_all) && (rel == rel_greater_equal)) {
    if (first->exp_class != exp_fun) return 0;
    if (!((FunctionExpression*)first)->match(atom)) return 0;
    if (!second->is_constant()) return 0;
    return second;
  }
  else if ((at == md_all) && (rel == rel_greater_equal)) {
    if (second->exp_class != exp_fun) return 0;
    if (!((FunctionExpression*)second)->match(atom)) return 0;
    if (!first->is_constant()) return 0;
    return first;
  }
  else {
    return 0;
  }
}

PDDL_Base::FunctionExpression* PDDL_Base::Relation::match_lteq_fun
(FChangeAtom* atom)
{
  if ((at == md_start) &&
      (rel == rel_less_equal) &&
      (first->exp_class == exp_fun) &&
      (second->exp_class == exp_sub)) {
    FunctionExpression* lhs = (FunctionExpression*)first;
    BinaryExpression* rhs = (BinaryExpression*)second;
    if (!lhs->match(atom)) return 0;
    if (!rhs->second->equals(atom->val)) return 0;
    if (rhs->first->exp_class != exp_fun) return 0;
    return (FunctionExpression*)(rhs->first);
  }
  else if ((at == md_start) &&
	   (rel == rel_less) &&
	   (first->exp_class == exp_fun) &&
	   (second->exp_class == exp_fun)) {
    FunctionExpression* lhs = (FunctionExpression*)first;
    FunctionExpression* rhs = (FunctionExpression*)second;
    if (!lhs->match(atom)) return 0;
    if (!atom->fun->integral) return 0;
    if (!atom->val->is_constant()) return 0;
    if (!(atom->val->eval_static() == 1)) return 0;
    return rhs;
  }
  else if ((at == md_all) &&
	   (rel == rel_less_equal) &&
	   (first->exp_class == exp_fun) &&
	   (second->exp_class == exp_fun)) {
    FunctionExpression* lhs = (FunctionExpression*)first;
    FunctionExpression* rhs = (FunctionExpression*)second;
    if (!lhs->match(atom)) return 0;
    return rhs;
  }
  else {
    return 0;
  }
}

bool PDDL_Base::Relation::is_static()
{
  return (first->is_static() && second->is_static());
}

PDDL_Base::partial_value PDDL_Base::Relation::partial_eval()
{
  // std::cerr << "evaluating ";
  // print(std::cerr, true);
  // std::cerr << "...";
  NTYPE v1;
  if (!first->eval_partial(v1)) {
    // std::cerr << " #1 undefined" << std::endl;
    return p_unknown;
  }
  NTYPE v2;
  if (!second->eval_partial(v2)) {
    // std::cerr << " #2 undefined" << std::endl;
    return p_unknown;
  }
  // std::cerr << " #1 = " << v1 << ", #2 = " << v2 << std::endl;
  switch (rel) {
  case rel_equal:
    {
      if (v1 == v2)
	return p_true;
      else
	return p_false;
    }
  case rel_greater:
    {
      if (v1 > v2)
	return p_true;
      else
	return p_false;
    }
  case rel_greater_equal:
    {
      if (v1 >= v2)
	return p_true;
      else
	return p_false;
    }
  case rel_less:
    {
      if (v1 < v2)
	return p_true;
      else
	return p_false;
    }
  case rel_less_equal:
    {
      if (v1 <= v2)
	return p_true;
      else
	return p_false;
    }
  default:
    {
      std::cerr << "program error: invalid relation " << rel
		<< " in ";
      print(std::cerr, true);
      std::cerr << std::endl;
      exit(255);
    }
  }
}

PDDL_Base::AtomBase::AtomBase(AtomBase* b)
  : param(b->param), at(b->at), at_time(b->at_time)
{
  // done
}

bool PDDL_Base::AtomBase::equals(AtomBase& b)
{
  if (param.length() != b.param.length()) return false;
  for (index_type k = 0; k < param.length(); k++) {
    if (param[k] != b.param[k]) {
      if ((param[k]->sym_class == sym_variable) &&
	  (b.param[k]->sym_class == sym_variable)) {
	VariableSymbol* v1 = (VariableSymbol*)param[k];
	VariableSymbol* v2 = (VariableSymbol*)b.param[k];
	if ((v1->binding != 0) && (v2->binding != 0)) {
	  if (!(v1->binding->equals(*(v2->binding))))
	    return false;
	}
	else {
	  return false;
	}
      }
      else {
	return false;
      }
    }
  }
  return true;
}

void PDDL_Base::AtomBase::free_variables(variable_vec& v)
{
  for (index_type k = 0; k < param.length(); k++)
    if (param[k]->sym_class == sym_variable)
      v.append((VariableSymbol*)param[k]);
}

bool PDDL_Base::AtomBase::occurs(Symbol* s)
{
  for (index_type k = 0; k < param.length(); k++)
    if (param[k] == s) return true;
  return false;
}

void PDDL_Base::AtomBase::fill_in_args(AtomBase* b)
{
  b->param.set_length(0);
  for (index_type k = 0; k < param.length(); k++)
    if (param[k]->sym_class == sym_variable)
      if (((VariableSymbol*)param[k])->value != 0)
	b->param.append(((VariableSymbol*)param[k])->value);
      else
	b->param.append(param[k]);
    else
      b->param.append(param[k]);
}

void PDDL_Base::AtomBase::collect_bound_variables(variable_vec& v)
{
  for (index_type k = 0; k < param.length(); k++)
    if (param[k]->sym_class == sym_variable)
      if (((VariableSymbol*)param[k])->binding != 0) {
	v.append((VariableSymbol*)param[k]);
	((VariableSymbol*)param[k])->binding->collect_bound_variables(v);
      }
}

bool PDDL_Base::Atom::check()
{
  if (pred->param.length() != param.length()) {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: wrong number of arguments for ";
      pred->print(std::cerr);
      std::cerr << " in ";
      print(std::cerr);
      std::cerr << std::endl;
      if (!best_effort) exit(1);
    }
    return false;
  }
  for (index_type k = 0; k < pred->param.length(); k++)
    if (!pred->param[k]->equality_type_check(param[k])) {
      if (write_warnings || !best_effort) {
	std::cerr << "warning: argument " << k << " in ";
	print(std::cerr);
	std::cerr << " has wrong type for ";
	pred->print(std::cerr);
	std::cerr << std::endl;
	if (!best_effort) exit(1);
      }
      return false;
    }
  return true;
}

PDDL_Base::Atom* PDDL_Base::Atom::instantiate_partially()
{
  Atom* a = new Atom(pred, at);
  fill_in_args(a);
  return a;
}

bool PDDL_Base::Atom::equals(Atom& a)
{
  if (pred != a.pred) return false;
  return AtomBase::equals(a);
}

PDDL_Base::partial_value PDDL_Base::Atom::partial_eval
(ptr_table* r, index_type p)
{
  if (!r) return p_false;
  if (p == param.length()) {
    if (r->val) return p_true;
    else return p_false;
  }
  if (param[p]->sym_class == sym_variable) {
    VariableSymbol* v = (VariableSymbol*)param[p];
    if (v->value) {
      ptr_table* n = r->find_next(v->value);
      if (n) return partial_eval(n, p+1);
      else return p_false;
    }
    else {
      bool poss_false = false;
      bool poss_true = false;
      for (index_type k = 0; k < v->sym_types.n_elements(); k++) {
	ptr_table* n = r->find_next(v->sym_types.get_element(k));
	if (n) {
	  partial_value nv = partial_eval(n, p+1);
	  if (nv == p_unknown) return p_unknown;
	  if (nv == p_false) {
	    poss_false = true;
	    if (poss_true) return p_unknown;
	  }
	  if (nv == p_true) {
	    poss_true = true;
	    if (poss_false) return p_unknown;
	  }
	}
	else {
	  poss_false = true;
	  if (poss_true) return p_unknown;
	}
      }
      if (poss_true && !poss_false) return p_true;
      else if (poss_false && !poss_true) return p_false;
      else return p_unknown; /* shouldn't ever happen */
    }
  }
  else {
    ptr_table* n = r->find_next(param[p]);
    if (n) return partial_eval(n, p+1);
    else return p_false;
  }
  return p_false; /* not actually necessary */
}

PDDL_Base::partial_value PDDL_Base::Atom::partial_eval()
{
  if (at == md_pos_goal) {
    return partial_eval(&(pred->pos_goal), 0);
  }
  else if (at == md_neg_goal) {
    return partial_eval(&(pred->neg_goal), 0);
  }
  else {
    return partial_eval(&(pred->init), 0);
  }
}

bool PDDL_Base::Atom::initial_value()
{
  ptr_table* r = &(pred->init);
  for (index_type k = 0; (k < param.length()) && r; k++) {
    if (param[k]->sym_class == sym_variable) {
      if (((VariableSymbol*)param[k])->value == 0) {
	std::cerr << "error: can't evaluate ";
	print(std::cerr);
	std::cerr << " in initial state because "
		  << param[k]->print_name << " not set"
		  << std::endl;
	exit(255);
      }
      r = r->find_next(((VariableSymbol*)param[k])->value);
    }
    else {
      r = r->find_next(param[k]);
    }
  }
  if (r) {
    if (r->val) return true;
  }
  return false;
}

Instance::Atom* PDDL_Base::Atom::find_prop
(Instance& ins, bool neg, bool create)
{
  ptr_table* r = (neg ? &(pred->neg_prop) : &(pred->pos_prop));
  for (index_type k = 0; k < param.length(); k++) {
    if (param[k]->sym_class == sym_variable) {
      if (((VariableSymbol*)param[k])->value == 0) {
	std::cerr << "error: unbound variable "
		  << param[k]->print_name << " in ";
	print(std::cerr);
	std::cerr << " - uncompiled object function?"
		  << std::endl;
	exit(255);
      }
      r = r->insert_next(((VariableSymbol*)param[k])->value);
    }
    else {
      r = r->insert_next(param[k]);
    }
  }
  if (!r->val) {
    if (!create) return 0;

    ptr_table* ri = &(pred->init);

    PDDL_Name* a_name = new PDDL_Name(pred, neg);
    for (index_type k = 0; k < param.length(); k++) {
      if (param[k]->sym_class == sym_variable) {
	assert(((VariableSymbol*)param[k])->value != 0);
	a_name->add(((VariableSymbol*)param[k])->value);
	if (ri) ri = ri->find_next(((VariableSymbol*)param[k])->value);
      }
      else {
	a_name->add(param[k]);
	if (ri) ri = ri->find_next(param[k]);
      }
    }

    Instance::Atom& p = ins.new_atom(a_name);
    bool init_val = false;
    NTYPE init_t = 0;
    if (ri) {
      if (ri->val) {
	init_val = true;
	init_t = ((Atom*)(ri->val))->at_time;
      }
    }
    p.init = (init_val != neg);
    p.init_t = init_t;
    p.src = new ptr_pair(pred, r);
    r->val = new Instance::atom_ref(ins.atoms, p.index);

    Instance::Atom* not_p = find_prop(ins, !neg, false);
    if (not_p) {
      if (not_p->neg != no_such_index) {
	std::cerr << "wierd error: negation of atom "
		  << not_p->index << "." << not_p->name << " is atom "
		  << not_p->neg << "." << ins.atoms[not_p->neg].name
		  << " and new atom " << p.index << "." << p.name
		  << std::endl;
	exit(255);
      }
      p.neg = not_p->index;
      not_p->neg = p.index;
    }

    /* check if this instance is marked by any :irrelevant item */
    for (index_type k = 0; k < pred->irr_ins.length(); k++)
      if (pred->irr_ins[k]->included) {
	if (pred->irr_ins[k]->context_is_static()) {
	  if (pred->irr_ins[k]->match(param)) {
	    if (write_info) {
	      std::cerr << "DKEL: atom " << a_name << " marked irrelevant by"
			<< std::endl;
	      pred->irr_ins[k]->print(std::cerr);
	    }
	    p.irrelevant = true;
	  }
	}
	else if (write_warnings || !best_effort) {
	  std::cerr << "warning: ignoring :irrelevant ";
	  pred->irr_ins[k]->entity->print(std::cerr);
	  std::cerr << " with non-static context" << std::endl;
	  if (!best_effort) exit(1);
	}
      }
  }

  Instance::atom_ref* a = (Instance::atom_ref*)r->val;
  return *a;
}

bool PDDL_Base::FTerm::equals(FTerm& ft)
{
  if (fun != ft.fun) return false;
  return AtomBase::equals(ft);
}

bool PDDL_Base::extend_substitution
(Symbol* out, Symbol* in, symbol_pair_vec& u)
{
  for (index_type k = 0; k < u.length(); k++) if (u[k].first == out) {
    if (u[k].second == in) {
      return true;
    }
    else {
      return false;
    }
  }
  u.append(symbol_pair(out, in));
  return true;
}

bool PDDL_Base::substitution_violates_inequality
(const symbol_pair_vec& neq, const symbol_pair_vec& u)
{
  for (index_type k = 0; k < neq.length(); k++) {
    Symbol* s1 = 0;
    Symbol* s2 = 0;
    for (index_type i = 0; i < u.length(); i++) {
      if ((s1 == 0) && (u[i].first == neq[k].first)) s1 = u[i].second;
      if ((s2 == 0) && (u[i].first == neq[k].second)) s2 = u[i].second;
    }
    if (s1 == 0) s1 = neq[k].first;
    if (s2 == 0) s2 = neq[k].second;
    if (s1 == s2) {
      return true;
    }
  }
  return false;
}

bool PDDL_Base::print_substitution
(std::ostream& s, const symbol_pair_vec& u)
{
  s << "{";
  for (index_type k = 0; k < u.length(); k++) {
    if (k > 0) s << ", ";
    s << u[k].first->print_name << "\\" << u[k].second->print_name;
  }
  s << "}";
}

bool PDDL_Base::print_inequality
(std::ostream& s, const symbol_pair_vec& neq)
{
  s << "{";
  for (index_type k = 0; k < neq.length(); k++) {
    if (k > 0) s << ", ";
    s << neq[k].first->print_name << " =/= " << neq[k].second->print_name;
  }
  s << "}";
}

bool PDDL_Base::Atom::instance_of(Atom* a, symbol_pair_vec& u)
{
  u.clear();
  if (a->pred != pred) return false;
  if (a->param.length() != param.length()) {
    std::cerr << "error: atom ";
    a->print(std::cerr);
    std::cerr << " and atom schema ";
    print(std::cerr);
    std::cerr << " have same predicate and different number of arguments"
	      << std::endl;
    return false;
  }
  for (index_type k = 0; k < param.length(); k++) if (param[k] != a->param[k]) {
    if (param[k]->sym_class == sym_variable) {
      if (a->param[k]->sym_class == sym_variable) {
	if (param[k]->sym_types.subtype_or_equal(a->param[k]->sym_types)) {
	  if (!extend_substitution(a->param[k], param[k], u)) return false;
	}
	else {
	  return false;
	}
      }
      else {
	return false;
      }
    }
    else {
      if (a->param[k]->sym_class == sym_variable) {
	if (((VariableSymbol*)a->param[k])->equality_type_check(param[k])) {
	  if (!extend_substitution(a->param[k], param[k], u)) return false;
	}
	else {
	  return false;
	}
      }
      else {
	if (a->param[k] != param[k]) return false;
      }
    }
  }
  return true;
}

bool PDDL_Base::Atom::instance_of
(Atom* a, const symbol_pair_vec& neq, symbol_pair_vec& u)
{
  if (!instance_of(a, u)) return false;
  if (substitution_violates_inequality(neq, u)) return false;
  return true;
}

bool PDDL_Base::Atom::unify
(Atom* a, symbol_pair_vec& u)
{
  u.clear();
  if (a->pred != pred) return false;
  if (a->param.length() != param.length()) {
    std::cerr << "error: atom ";
    a->print(std::cerr);
    std::cerr << " and atom ";
    print(std::cerr);
    std::cerr << " have same predicate but different #arguments"
	      << std::endl;
    return false;
  }
  for (index_type k = 0; k < param.length(); k++) if (param[k] != a->param[k]) {
    if (param[k]->sym_class == sym_variable) {
      if (a->param[k]->sym_class == sym_variable) {
	if (param[k]->sym_types.subtype_or_equal(a->param[k]->sym_types)) {
	  if (!extend_substitution(a->param[k], param[k], u)) return false;
	}
	else if (a->param[k]->sym_types.subtype_or_equal(param[k]->sym_types)) {
	  if (!extend_substitution(param[k], a->param[k], u)) return false;
	}
	else {
	  return false;
	}
      }
      else {
	if (((VariableSymbol*)param[k])->equality_type_check(a->param[k])) {
	  if (!extend_substitution(param[k], a->param[k], u)) return false;
	}
	else {
	  return false;
	}
      }
    }
    else {
      if (a->param[k]->sym_class == sym_variable) {
	if (((VariableSymbol*)a->param[k])->equality_type_check(param[k])) {
	  if (!extend_substitution(a->param[k], param[k], u)) return false;
	}
	else {
	  return false;
	}
      }
      else {
	if (a->param[k] != param[k]) return false;
      }
    }
  }
  return true;
}

bool PDDL_Base::Atom::unify
(Atom* a, const symbol_pair_vec& neq, symbol_pair_vec& u)
{
  if (!unify(a, u)) return false;
  if (substitution_violates_inequality(neq, u)) return false;
  return true;
}

bool PDDL_Base::CAtom::instance_of(Atom* a, symbol_pair_vec& u)
{
  return Atom::instance_of(a, u);
}

bool PDDL_Base::CAtom::instance_of(CAtom* a, symbol_pair_vec& u)
{
  return Atom::instance_of(a, a->neq, u);
}

bool PDDL_Base::CAtom::instance_of
(Atom* a, const symbol_pair_vec& neq, symbol_pair_vec& u)
{
  return Atom::instance_of(a, neq, u);
}

bool PDDL_Base::CAtom::unify(Atom* a, symbol_pair_vec& u)
{
  return Atom::unify(a, neq, u);
}

bool PDDL_Base::CAtom::unify(CAtom* a, symbol_pair_vec& u)
{
  // std::cerr << "unifying ";
  // print(std::cerr);
  // std::cerr << " and ";
  // a->print(std::cerr);
  // std::cerr << "...";
  if (!Atom::unify(a, u)) {
    // std::cerr << " can not unify" << std::endl;
    return false;
  }
  // std::cerr << " unified with ";
  // print_substitution(std::cerr, u);
  // std::cerr << std::endl;

  // std::cerr << "checking ";
  // print_inequality(std::cerr, a->neq);
  // std::cerr << " under substitution...";
  if (substitution_violates_inequality(a->neq, u)) {
    // std::cerr << " violated" << std::endl;
    return false;
  }
  // std::cerr << " satisfied" << std::endl;

  // std::cerr << "checking ";
  // print_inequality(std::cerr, neq);
  // std::cerr << " under substitution...";
  if (substitution_violates_inequality(neq, u)) {
    // std::cerr << " violated" << std::endl;
    return false;
  }
  // std::cerr << " satisfied" << std::endl;

  return true;
}

void PDDL_Base::CAtom::print(std::ostream& s)
{
  if (neq.length() > 0) {
    s << "{";
    Atom::print(s);
    s << " |";
    for (index_type k = 0; k < neq.length(); k++) {
      s << " ";
      neq[k].first->print(s);
      s << " =/= ";
      neq[k].second->print(s);
    }
    s << "}";
  }
  else {
    Atom::print(s);
  }
}

PDDL_Base::Formula* PDDL_Base::Formula::simplify()
{
  switch (fc) {
  case fc_false:
    {
      return this;
    }
  case fc_true:
    {
      return this;
    }
  case fc_atom:
    {
      return this;
    }
  case fc_equality:
    {
      return this;
    }
  case fc_negation:
    {
      ((NFormula*)this)->f = ((NFormula*)this)->f->simplify();
      if (((NFormula*)this)->f->fc == fc_true) {
	return new Formula(fc_false);
      }
      else if (((NFormula*)this)->f->fc == fc_false) {
	return new Formula(fc_true);
      }
      else if (((NFormula*)this)->f->fc == fc_negation) {
	return ((NFormula*)((NFormula*)this)->f)->f;
      }
      else {
	return this;
      }
    }
  case fc_conjunction:
    {
      formula_vec sp(0, 0);
      for (index_type k = 0; k < ((CFormula*)this)->parts.length(); k++)
	sp.append(((CFormula*)this)->parts[k]->simplify());
      ((CFormula*)this)->parts.assign_value(0, 0);
      for (index_type k = 0; k < sp.length(); k++) {
	if (sp[k]->fc == fc_false) {
	  return new Formula(fc_false);
	}
	else if (sp[k]->fc == fc_conjunction) {
	  for (index_type i = 0; i < ((CFormula*)sp[k])->parts.length(); i++)
	    sp.append(((CFormula*)sp[k])->parts[i]);
	}
	else if (sp[k]->fc != fc_true) {
	  ((CFormula*)this)->parts.append(sp[k]);
	}
      }
      if (((CFormula*)this)->parts.length() == 0) {
	return new Formula(fc_true);
      }
      else if (((CFormula*)this)->parts.length() == 1) {
	return ((CFormula*)this)->parts[0];
      }
      else {
	return this;
      }
    }
  case fc_disjunction:
    {
      formula_vec sp(0, 0);
      for (index_type k = 0; k < ((CFormula*)this)->parts.length(); k++)
	sp.append(((CFormula*)this)->parts[k]->simplify());
      ((CFormula*)this)->parts.assign_value(0, 0);
      for (index_type k = 0; k < sp.length(); k++) {
	if (sp[k]->fc == fc_true) {
	  return new Formula(fc_true);
	}
	else if (sp[k]->fc == fc_disjunction) {
	  for (index_type i = 0; i < ((CFormula*)sp[k])->parts.length(); i++)
	    sp.append(((CFormula*)sp[k])->parts[i]);
	}
	else if (sp[k]->fc != fc_false) {
	  ((CFormula*)this)->parts.append(sp[k]);
	}
      }
      if (((CFormula*)this)->parts.length() == 0) {
	return new Formula(fc_false);
      }
      else if (((CFormula*)this)->parts.length() == 1) {
	return ((CFormula*)this)->parts[0];
      }
      else {
	return this;
      }
    }
  case fc_implication:
    {
      ((BFormula*)this)->f1 = ((BFormula*)this)->f1->simplify();
      ((BFormula*)this)->f2 = ((BFormula*)this)->f2->simplify();
      if (((BFormula*)this)->f1->fc == fc_true) {
	return ((BFormula*)this)->f2;
      }
      else if (((BFormula*)this)->f1->fc == fc_false) {
	return new Formula(fc_true);
      }
      else {
	if (((BFormula*)this)->f2->fc == fc_true) {
	  return new Formula(fc_true);
	}
	else if (((BFormula*)this)->f2->fc == fc_false) {
	  if (((BFormula*)this)->f1->fc == fc_negation) {
	    return ((NFormula*)((BFormula*)this)->f1)->f;
	  }
	  else {
	    return new NFormula(((BFormula*)this)->f1);
	  }
	}
	else {
	  return this;
	}
      }
    }
  case fc_equivalence:
    {
      ((BFormula*)this)->f1 = ((BFormula*)this)->f1->simplify();
      ((BFormula*)this)->f2 = ((BFormula*)this)->f2->simplify();
      return this;
    }
  case fc_universal:
    {
      ((QFormula*)this)->f = ((QFormula*)this)->f->simplify();
      return this;
    }
  case fc_existential:
    {
      ((QFormula*)this)->f = ((QFormula*)this)->f->simplify();
      return this;
    }
  }
  assert(0);
}

void PDDL_Base::Formula::rename_variables_1(symbol_pair_vec& sub)
{
  switch (fc) {
  case fc_atom:
    {
      AFormula* f = (AFormula*)this;
      for (index_type k = 0; k < f->param.length(); k++) {
	bool found = false;
	for (index_type i = 0; (i < sub.length()) && !found; i++)
	  if (f->param[k] == sub[i].first) {
	    f->param[k] = sub[i].second;
	    found = true;
	  }
      }
      return;
    }
  case fc_equality:
    {
      EqFormula* f = (EqFormula*)this;
      bool found = false;
      for (index_type i = 0; (i < sub.length()) && !found; i++)
	if (f->t1 == sub[i].first) {
	  f->t1 = sub[i].second;
	  found = true;
	}
      found = false;
      for (index_type i = 0; (i < sub.length()) && !found; i++)
	if (f->t2 == sub[i].first) {
	  f->t2 = sub[i].second;
	  found = true;
	}
      return;
    }
  case fc_negation:
    {
      NFormula* f = (NFormula*)this;
      f->f->rename_variables_1(sub);
      return;
    }
  case fc_conjunction:
  case fc_disjunction:
    {
      CFormula* f = (CFormula*)this;
      for (index_type k = 0; k < f->parts.length(); k++) {
	f->parts[k]->rename_variables_1(sub);
      }
      return;
    }
  case fc_implication:
  case fc_equivalence:
    {
      BFormula* f = (BFormula*)this;
      f->f1->rename_variables_1(sub);
      f->f2->rename_variables_1(sub);
      return;
    }
  case fc_universal:
  case fc_existential:
    {
      QFormula* f = (QFormula*)this;
      // option 1: do renaming on quantified variables, and formula
      for (index_type k = 0; k < f->vars.length(); k++) {
	bool found = false;
	for (index_type i = 0; (i < sub.length()) && !found; i++)
	  if (f->vars[k] == sub[i].first) {
	    f->vars[k] = (VariableSymbol*)sub[i].second;
	    found = true;
	  }
      }
      f->f->rename_variables_1(sub);
      // option 2: remove quantified variables from sub, apply to subformula
//       symbol_pair_vec new_sub;
//       for (index_type i = 0; i < sub.length(); i++) {
// 	if (f->vars.first((VariableSymbol*)sub[i].first) == no_such_index)
// 	  new_sub.append(sub[i]);
//       }
//       f->f->rename_variables_1(new_sub);
      return;
    }
  }
  assert(0);
}

// replace 2nd by 1st
void PDDL_Base::Formula::rename_variables_2(symbol_pair_vec& sub)
{
  symbol_pair_vec rsub;
  for (index_type i = 0; i < sub.length(); i++)
    rsub.append(symbol_pair(sub[i].second, sub[i].first));
  rename_variables_1(rsub);
}

// replace 1st by 2nd
void PDDL_Base::Formula::rename_predicates_1(symbol_pair_vec& sub)
{
  switch (fc) {
  case fc_atom:
    {
      AFormula* f = (AFormula*)this;
      for (index_type i = 0; i < sub.length(); i++)
	if (f->pred == sub[i].first) {
	  f->pred = (PredicateSymbol*)sub[i].second;
	  return;
	}
      return;
    }
  case fc_negation:
    {
      NFormula* f = (NFormula*)this;
      f->f->rename_predicates_1(sub);
      return;
    }
  case fc_conjunction:
  case fc_disjunction:
    {
      CFormula* f = (CFormula*)this;
      for (index_type k = 0; k < f->parts.length(); k++) {
	f->parts[k]->rename_predicates_1(sub);
      }
      return;
    }
  case fc_implication:
  case fc_equivalence:
    {
      BFormula* f = (BFormula*)this;
      f->f1->rename_predicates_1(sub);
      f->f2->rename_predicates_1(sub);
      return;
    }
  case fc_universal:
  case fc_existential:
    {
      QFormula* f = (QFormula*)this;
      f->f->rename_predicates_1(sub);
      return;
    }
  }
  assert(0);
}

// replace 2nd by 1st
void PDDL_Base::Formula::rename_predicates_2(symbol_pair_vec& sub)
{
  symbol_pair_vec rsub;
  for (index_type i = 0; i < sub.length(); i++)
    rsub.append(symbol_pair(sub[i].second, sub[i].first));
  rename_predicates_1(rsub);
}

void PDDL_Base::Formula::print(std::ostream& s) const
{
  switch (fc) {
  case fc_false:
    {
      s << "false";
      return;
    }
  case fc_true:
    {
      s << "true";
      return;
    }
  case fc_atom:
    {
      ((PDDL_Base::AFormula*)this)->print(s);
      return;
    }
  case fc_equality:
    {
      s << "(= ";
      ((EqFormula*)this)->t1->print(s);
      s << " ";
      ((EqFormula*)this)->t2->print(s);
      s << ")";
      return;
    }
  case fc_negation:
    {
      s << "(not ";
      ((NFormula*)this)->f->print(s);
      s << ")";
      return;
    }
  case fc_conjunction:
    {
      s << "(and";
      for (index_type k = 0; k < ((CFormula*)this)->parts.length(); k++) {
	s << " ";
	((CFormula*)this)->parts[k]->print(s);
      }
      s << ")";
      return;
    }
  case fc_disjunction:
    {
      s << "(or";
      for (index_type k = 0; k < ((CFormula*)this)->parts.length(); k++) {
	s << " ";
	((CFormula*)this)->parts[k]->print(s);
      }
      s << ")";
      return;
    }
  case fc_implication:
    {
      s << "(imply ";
      ((BFormula*)this)->f1->print(s);
      s << " ";
      ((BFormula*)this)->f2->print(s);
      s << ")";
      return;
    }
  case fc_equivalence:
    {
      s << "(iff ";
      ((BFormula*)this)->f1->print(s);
      s << " ";
      ((BFormula*)this)->f2->print(s);
      s << ")";
      return;
    }
  case fc_universal:
    {
      s << "(forall (";
      for (index_type k = 0; k < ((QFormula*)this)->vars.length(); k++) {
	if (k > 0) s << " ";
	((QFormula*)this)->vars[k]->print(s);
      }
      s << ") ";
      ((QFormula*)this)->f->print(s);
      s << ")";
      return;
    }
  case fc_existential:
    {
      s << "(exists (";
      for (index_type k = 0; k < ((QFormula*)this)->vars.length(); k++) {
	if (k > 0) s << " ";
	((QFormula*)this)->vars[k]->print(s);
      }
      s << ") ";
      ((QFormula*)this)->f->print(s);
      s << ")";
      return;
    }
  }
  assert(0);
}

void PDDL_Base::AFormula::print(std::ostream& s) const
{
  PDDL_Base::Atom::print(s, false);
}

void PDDL_Base::Formula::write_otter(std::ostream& s) const
{
  switch (fc) {
  case fc_false:
    {
      s << "$F";
      return;
    }
  case fc_true:
    {
      s << "$T";
      return;
    }
  case fc_atom:
    {
      ((PDDL_Base::AFormula*)this)->write_otter(s);
      return;
    }
  case fc_equality:
    {
      ((EqFormula*)this)->write_otter(s);
      return;
    }
  case fc_negation:
    {
      s << "-(";
      ((NFormula*)this)->f->write_otter(s);
      s << ")";
      return;
    }
  case fc_conjunction:
  case fc_disjunction:
    {
      ((CFormula*)this)->write_otter(s);
      return;
    }
  case fc_implication:
    {
      s << "->(";
      ((BFormula*)this)->f1->write_otter(s);
      s << ",";
      ((BFormula*)this)->f2->write_otter(s);
      s << ")";
      return;
    }
  case fc_equivalence:
    {
      s << "<->(";
      ((BFormula*)this)->f1->write_otter(s);
      s << ",";
      ((BFormula*)this)->f2->write_otter(s);
      s << ")";
      return;
    }
  case fc_universal:
  case fc_existential:
    {
      ((QFormula*)this)->write_otter(s);
      return;
    }
  }
  assert(0);
}

void PDDL_Base::AFormula::write_otter(std::ostream& s) const
{
  s << pred->print_name << "(";
  for (index_type k = 0; k < param.length(); k++) {
    if (k > 0) s << ",";
    if (param[k]->sym_class == sym_variable) {
      s << (param[k]->print_name + 1);
    }
    else {
      s << param[k]->print_name;
    }
  }
  s << ")";
}

void PDDL_Base::EqFormula::write_otter(std::ostream& s) const
{
  s << "=(";
  if (t1->sym_class == sym_variable) {
    s << (t1->print_name + 1);
  }
  else {
    s << t1->print_name;
  }
  s << ",";
  if (t2->sym_class == sym_variable) {
    s << (t2->print_name + 1);
  }
  else {
    s << t2->print_name;
  }
  s << ")";
}

void PDDL_Base::CFormula::write_otter(std::ostream& s) const
{
  if (fc == fc_conjunction) s << "&(";
  else if (fc == fc_disjunction) s << "|(";
  else {
    std::cerr << "error: invalide class " << fc
	      << " for composite formula"
	      << std::endl;
    exit(255);
  }
  for (index_type k = 0; k < parts.length(); k++) {
    if (k > 0) s << ",";
    parts[k]->write_otter(s);
  }
  s << ")";
}

void PDDL_Base::QFormula::write_otter(std::ostream& s) const
{
  if (fc == fc_universal) s << "$Quantified(all";
  else if (fc == fc_existential) s << "$Quantified(exists";
  else {
    std::cerr << "error: invalide class " << fc
	      << " for quantified formula"
	      << std::endl;
    exit(255);
  }
  for (index_type k = 0; k < vars.length(); k++) {
    s << "," << (vars[k]->print_name + 1);
  }
  s << ",";
  f->write_otter(s);
  s << ")";
}

PDDL_Base::FInitAtom::FInitAtom(FChangeAtom* a)
  : AtomBase(a), fun(a->fun), val(0)
{
  // done
}

bool PDDL_Base::FChangeAtom::equals(FChangeAtom& a)
{
  if (fun != a.fun) return false;
  if (!AtomBase::equals(a)) return false;
  return val->equals(a.val);
}

bool PDDL_Base::FChangeAtom::fluent_equals(FChangeAtom& a)
{
  if (fun != a.fun) return false;
  if (!AtomBase::equals(a)) return false;
  return true;
}

bool PDDL_Base::FChangeAtom::fluent_and_mode_equals(FChangeAtom& a)
{
  if (at != a.at) return false;
  return fluent_equals(a);
}

PDDL_Base::FChangeAtom* PDDL_Base::FChangeAtom::find_fluent_equals
(ch_atom_vec& vec)
{
  for (index_type k = 0; k < vec.length(); k++)
    if (fluent_equals(*vec[k])) return vec[k];
  return 0;
}

Instance::Resource* PDDL_Base::FChangeAtom::find_resource(Instance& ins)
{
  ptr_table* r = &(fun->init);
  for (index_type k = 0; k < param.length(); k++) {
    if (param[k]->sym_class == sym_variable) {
      if (((VariableSymbol*)param[k])->value == 0) {
	std::cerr << "error: unbound variable "
		  << param[k]->print_name << " in ";
	print(std::cerr);
	std::cerr << " - uncompiled object function?"
		  << std::endl;
	exit(255);
      }
      r = r->insert_next(((VariableSymbol*)param[k])->value);
    }
    else {
      r = r->insert_next(param[k]);
    }
  }
  if (!r->val) {
    r->val = new FInitAtom(this);
  }
  FInitAtom* a = (FInitAtom*)(r->val);
  if (!a->res) {

    PDDL_Name* fa_name = new PDDL_Name(fun, false);
    for (index_type k = 0; k < param.length(); k++) {
      if (param[k]->sym_class == sym_variable)
	fa_name->add(((VariableSymbol*)param[k])->value);
      else
	fa_name->add(param[k]);
    }

    Instance::Resource& i = ins.new_resource(fa_name);
    i.init = a->val;
    i.src = new ptr_pair(fun, r);
    a->res = Instance::resource_ref(ins.resources, i.index);
  }

  return (a->res);
}

void PDDL_Base::AtomBase::insert(ptr_table& t)
{
  ptr_table* r = &t;
  for (index_type k = 0; k < param.length(); k++) {
    assert(param[k] != 0);
    r = r->insert_next(param[k]);
  }
  r->val = this;
}

index_type PDDL_Base::find_matching_atom(Atom* a, atom_vec& v)
{
  for (index_type i = 0; i < v.length(); i++)
    if (a->equals(*v[i]))
      return i;
  return no_such_index;
}

index_type PDDL_Base::find_matching_atom(Atom* a, mode_keyword m, atom_vec& v)
{
  for (index_type i = 0; i < v.length(); i++)
    if (a->equals(*v[i]) && (v[i]->at == m))
      return i;
  return no_such_index;
}

index_type PDDL_Base::find_matching_fluent_atom
(FChangeAtom* a, mode_keyword m, ch_atom_vec& v)
{
  for (index_type i = 0; i < v.length(); i++)
    if (a->equals(*v[i]) && (v[i]->at == m))
      return i;
  return no_such_index;
}

void PDDL_Base::ActionSymbol::post_process()
{
  if (write_trace) {
    std::cerr << "post processing action " << print_name << "..." << std::endl;
  }

  /* mark predicates occuring in positive/negative preconditions */
  for (index_type k = 0; k < pos_pre.length(); k++)
    pos_pre[k]->pred->pos_pre = true;
  for (index_type k = 0; k < neg_pre.length(); k++)
    neg_pre[k]->pred->neg_pre = true;

  /* separate conditional effects from set effects */
  for (index_type k = 0; k < set_eff.length(); k++)
    if (!set_eff[k]->context_is_static()) {
      cond_eff.append(set_eff[k]);
      set_eff.remove(k);
      k -= 1;
    }

  /* search for locks */
  for (index_type k = 0; k < dels.length(); k++)
    if (dels[k]->at == md_start) {
      index_type add_i = find_matching_atom(dels[k], md_end, adds);
      index_type pre_i = find_matching_atom(dels[k], md_start, pos_pre);
      if ((add_i != no_such_index) && (pre_i != no_such_index)) {
	locks.append(dels[k]);
	locks[locks.length() - 1]->at = md_all;
	dels.remove(k);
	adds.remove(add_i);
      }
    }

  /* search for enables */
  for (index_type k = 0; k < adds.length(); k++)
    if (adds[k]->at == md_start) {
      index_type del_i = find_matching_atom(adds[k], md_end, dels);
      if (del_i != no_such_index) {
	enables.append(adds[k]);
	enables[enables.length() - 1]->at = md_all;
	adds.remove(k);
	dels.remove(del_i);
      }
    }

  // extract consumed atoms
  cons.set_length(0);
  for (index_type k = 0; k < dels.length(); k++) {
    index_type pre_i = find_matching_atom(dels[k], pos_pre);
    if (pre_i != no_such_index)
      cons.append(dels[k]);
  }

  /* mark predicates added/deleted/locked */
  for (index_type k = 0; k < adds.length(); k++)
    adds[k]->pred->added = true;
  for (index_type k = 0; k < dels.length(); k++)
    dels[k]->pred->deleted = true;
  for (index_type k = 0; k < locks.length(); k++)
    locks[k]->pred->locked = true;

  if (compact_resource_effects) {
    for (index_type k = 0; k < decs.length(); k++)
      if (decs[k]->val->exp_class == exp_const) {
	// match effect decs[k] agains other decs
	index_type i = k + 1;
	while (i < decs.length()) {
	  if (decs[i]->fluent_and_mode_equals(*decs[k]) &&
	      (decs[i]->val->exp_class == exp_const)) {
	    if (write_info) {
	      std::cerr << "info: collapsing effects decrease ";
	      decs[k]->print(std::cerr);
	      std::cerr << " and decrease ";
	      decs[i]->print(std::cerr);
	      std::cerr << " in action " << print_name
			<< std::endl;
	    }
	    ((ConstantExpression*)decs[k]->val)->val +=
	      ((ConstantExpression*)decs[i]->val)->val;
	    decs.remove(i);
	  }
	  else {
	    if (write_trace) {
	      std::cerr << "debug: effects ";
	      decs[k]->print(std::cerr);
	      std::cerr << " and ";
	      decs[i]->print(std::cerr);
	      std::cerr << " in action " << print_name << " do not match"
			<< std::endl;
	    }
	    i += 1;
	  }
	}
	// match effect decs[k] agains incs
	i = 0;
	while (i < incs.length()) {
	  if (incs[i]->fluent_and_mode_equals(*decs[k]) &&
	      (incs[i]->val->exp_class == exp_const)) {
	    if (((ConstantExpression*)decs[k]->val)->val >
		((ConstantExpression*)incs[i]->val)->val) {
	      if (write_info) {
		std::cerr << "info: collapsing effects decrease ";
		decs[k]->print(std::cerr);
		std::cerr << " and increase ";
		incs[i]->print(std::cerr);
		std::cerr << " in action " << print_name
			  << std::endl;
	      }
	      ((ConstantExpression*)decs[k]->val)->val -=
		((ConstantExpression*)incs[i]->val)->val;
	      decs.remove(i);
	    }
	    else {
	      if (write_trace) {
		std::cerr << "debug: effects decrease ";
		decs[k]->print(std::cerr);
		std::cerr << " and increase ";
		incs[i]->print(std::cerr);
		std::cerr << " in action " << print_name
			  << " should not be collapsed to a decrease effect"
			  << std::endl;
	      }
	      i += 1;
	    }
	  }
	  else {
	    if (write_trace) {
	      std::cerr << "debug: effects ";
	      decs[k]->print(std::cerr);
	      std::cerr << " and ";
	      incs[i]->print(std::cerr);
	      std::cerr << " in action " << print_name << " do not match"
			<< std::endl;
	    }
	    i += 1;
	  }
	}
      }
    for (index_type k = 0; k < incs.length(); k++)
      if (incs[k]->val->exp_class == exp_const) {
	// match effect incs[k] agains other incs
	index_type i = k + 1;
	while (i < incs.length()) {
	  if (incs[i]->fluent_and_mode_equals(*incs[k]) &&
	      (incs[i]->val->exp_class == exp_const)) {
	    if (write_info) {
	      std::cerr << "info: collapsing effects increase ";
	      incs[k]->print(std::cerr);
	      std::cerr << " and increase ";
	      incs[i]->print(std::cerr);
	      std::cerr << " in action " << print_name
			<< std::endl;
	    }
	    ((ConstantExpression*)incs[k]->val)->val +=
	      ((ConstantExpression*)incs[i]->val)->val;
	    incs.remove(i);
	  }
	  else {
	    if (write_trace) {
	      std::cerr << "debug: effects ";
	      incs[k]->print(std::cerr);
	      std::cerr << " and ";
	      incs[i]->print(std::cerr);
	      std::cerr << " in action " << print_name << " do not match"
			<< std::endl;
	    }
	    i += 1;
	  }
	}
	// match effect incs[k] agains decs
	i = 0;
	while (i < decs.length()) {
	  if (decs[i]->fluent_and_mode_equals(*incs[k]) &&
	      (decs[i]->val->exp_class == exp_const)) {
	    if (((ConstantExpression*)incs[k]->val)->val >
		((ConstantExpression*)decs[i]->val)->val) {
	      if (write_info) {
		std::cerr << "info: collapsing effects increase ";
		incs[k]->print(std::cerr);
		std::cerr << " and decrease ";
		decs[i]->print(std::cerr);
		std::cerr << " in action " << print_name
			  << std::endl;
	      }
	      ((ConstantExpression*)incs[k]->val)->val -=
		((ConstantExpression*)decs[i]->val)->val;
	      decs.remove(i);
	    }
	    else {
	      if (write_trace) {
		std::cerr << "debug: effects increase ";
		incs[k]->print(std::cerr);
		std::cerr << " and decrease ";
		decs[i]->print(std::cerr);
		std::cerr << " in action " << print_name
			  << " should not be collapsed to an increase effect"
			  << std::endl;
	      }
	      i += 1;
	    }
	  }
	  else {
	    if (write_trace) {
	      std::cerr << "debug: effects ";
	      incs[k]->print(std::cerr);
	      std::cerr << " and ";
	      decs[i]->print(std::cerr);
	      std::cerr << " in action " << print_name << " do not match"
			<< std::endl;
	    }
	    i += 1;
	  }
	}
      }
  }

  /* search for "borrows"s */
  for (index_type k = 0; k < decs.length(); k++)
    if (decs[k]->at == md_start) {
      index_type inc_i = find_matching_fluent_atom(decs[k], md_end, incs);

      if (inc_i != no_such_index) {
	if (use_strict_borrow_definition) {
	  bool check = false;
	  for (index_type i = 0; (i < num_pre.length()) && check; i++) {
	    Expression* f = num_pre[i]->match_gteq_constant(decs[k]);
	    if (f) {
	      if (f->eval_static() == 0) check = true;
	    }
	  }
	  if (!check) inc_i = no_such_index;
	}

	if (inc_i != no_such_index) {
	  decs[k]->at = md_all;
	  reqs.append(decs[k]);
	  decs.remove(k);
	  incs.remove(inc_i);
	  k -= 1;
	}
      }

      /* check for possible borrow with loss */
      else if (decs[k]->val->exp_class == exp_const) {
	for (index_type i = 0;
	     ((i < incs.length()) && (inc_i == no_such_index)); i++)
	  if (decs[k]->fluent_equals(*incs[i]) &&
	      (incs[i]->at == md_end) &&
	      (incs[i]->val->exp_class == exp_const))
	    if (((ConstantExpression*)incs[i]->val)->val <
		((ConstantExpression*)decs[k]->val)->val) {
	      inc_i = i;
	    }
	if (inc_i != no_such_index) {
	  if (use_strict_borrow_definition) {
	    bool check = false;
	    for (index_type i = 0; (i < num_pre.length()) && check; i++) {
	      Expression* f = num_pre[i]->match_gteq_constant(decs[k]);
	      if (f) {
		if (f->eval_static() == 0) check = true;
	      }
	    }
	    if (!check) inc_i = no_such_index;
	  }

	  if (inc_i != no_such_index) {
	    NTYPE c = (((ConstantExpression*)decs[k]->val)->val -
		       ((ConstantExpression*)incs[inc_i]->val)->val);
	    NTYPE u = ((ConstantExpression*)incs[inc_i]->val)->val;
	    FChangeAtom* r_atom = new FChangeAtom(decs[k], u);
	    r_atom->at = md_all;
	    reqs.append(r_atom);
	    ((ConstantExpression*)decs[k]->val)->val = c;
	    incs.remove(inc_i);
	  }
	}
      }
    }

  /* extended search for "borrows"s */
  if (use_extended_borrow_definition) {
    for (index_type k = 0; k < incs.length(); k++)
      if (incs[k]->at == md_start) {
	index_type dec_i = find_matching_fluent_atom(incs[k], md_end, decs);
	FunctionExpression* f = 0;
	if (dec_i != no_such_index) {
	  for (index_type i = 0; (i < num_pre.length()) && (f == 0); i++)
	    f = num_pre[i]->match_lteq_fun(incs[k]);
	}
	if ((dec_i != no_such_index) && (f != 0)) {
	  FChangeAtom* r_atom = f->make_atom_base();
	  r_atom->at = md_all;
	  r_atom->val = incs[k]->val;
	  reqs.append(r_atom);
	  incs.remove(k);
	  decs.remove(dec_i);
	  k -= 1;
	}
      }
  }

  /* mark functions borrowed/increased/decreased */
  for (index_type k = 0; k < reqs.length(); k++) {
    reqs[k]->fun->borrowed = true;
    if (!reqs[k]->val->is_static())
      reqs[k]->fun->linear = false;
  }
  for (index_type k = 0; k < incs.length(); k++) {
    incs[k]->fun->increased = true;
    if (!incs[k]->val->is_static())
      incs[k]->fun->linear = false;
  }
  for (index_type k = 0; k < decs.length(); k++) {
    decs[k]->fun->decreased = true;
    if (!decs[k]->val->is_static())
      decs[k]->fun->linear = false;
  }
  for (index_type k = 0; k < fass.length(); k++) {
    fass[k]->fun->assigned = true;
    fass[k]->fun->linear = false;
  }
}

void PDDL_Base::post_process()
{
  if (compile_away_object_functions) {
    compile_object_functions();
  }

  if (write_trace) {
    std::cerr << "trace: post processing PDDL definition..." << std::endl;
  }

  dom_base_types.assign_value(0, 0);
  for (index_type k = 0; k < dom_types.length(); k++) {
    dom_types[k]->is_base_type = true;
    for (index_type i = 0;
	 (i < dom_types.length()) && dom_types[k]->is_base_type; i++)
      if (dom_types[i]->sym_types.contains(dom_types[k]))
	dom_types[k]->is_base_type = false;
    if (dom_types[k]->is_base_type)
      dom_base_types.append(dom_types[k]);
  }
  for (index_type k = 0; k < dom_actions.length(); k++) {
    dom_actions[k]->post_process();
  }
  ready_to_instantiate = true;
}

bool PDDL_Base::Goal::makeCPG(CPG& g)
{
  if ((g_class == goal_pos_atom) || (g_class == goal_neg_atom)) {
    AtomicGoal* a = (AtomicGoal*)this;
    g.atoms.append(a->atom);
    if (g_class == goal_neg_atom)
      g.neg.append(true);
    else
      g.neg.append(false);
    g.atom_first_arg.append(g.args.length());
    assert((g.atoms.length() == g.neg.length()) &&
	   (g.atoms.length() == g.atom_first_arg.length()));
    assert(a->atom->param.length() == a->atom->pred->param.length());
    for (index_type k = 0; k < a->atom->param.length(); k++) {
      g.args.append(a->atom->param[k]);
      assert(a->atom->param[k]->sym_types.length() == 1);
      g.arg_types.append(a->atom->param[k]->sym_types[0]);
    }
    assert(g.args.length() == g.arg_types.length());
    return true;
  }
  else if (g_class == goal_conjunction) {
    ConjunctiveGoal* c = (ConjunctiveGoal*)this;
    for (index_type i = 0; i < c->goals.length(); i++) {
      if (!c->goals[i]->makeCPG(g)) return false;
    }
    return true;
  }
  std::cerr << "error: can't make CPG from goal ";
  print(std::cerr);
  std::cerr << std::endl;
  return false;
}

bool PDDL_Base::Goal::makeCPG(CPG& g, index_vec& s)
{
  if ((g_class == goal_pos_atom) || (g_class == goal_neg_atom)) {
    AtomicGoal* a = (AtomicGoal*)this;
    index_type i = no_such_index;
    for (index_type k = 0; (k < g.atoms.length()) && (i == no_such_index); k++)
      if ((g.atoms[k]->equals(*(a->atom))) &&
	  (((g_class == goal_pos_atom) && !g.neg[k]) ||
	   ((g_class == goal_neg_atom) && g.neg[k])))
	i = k;
    if (i == no_such_index) {
      g.atoms.append(a->atom);
      if (g_class == goal_neg_atom)
	g.neg.append(true);
      else
	g.neg.append(false);
      g.atom_first_arg.append(g.args.length());
      assert((g.atoms.length() == g.neg.length()) &&
	     (g.atoms.length() == g.atom_first_arg.length()));
      assert(a->atom->param.length() == a->atom->pred->param.length());
      for (index_type k = 0; k < a->atom->param.length(); k++) {
	g.args.append(a->atom->param[k]);
	assert(a->atom->param[k]->sym_types.length() == 1);
	g.arg_types.append(a->atom->param[k]->sym_types[0]);
      }
      assert(g.args.length() == g.arg_types.length());
      s.append(g.atoms.length() - 1);
    }
    else {
      s.append(i);
    }
    return true;
  }
  else if (g_class == goal_conjunction) {
    ConjunctiveGoal* c = (ConjunctiveGoal*)this;
    for (index_type i = 0; i < c->goals.length(); i++) {
      if (!c->goals[i]->makeCPG(g)) return false;
    }
    return true;
  }
  std::cerr << "error: can't make CPG from goal ";
  print(std::cerr);
  std::cerr << std::endl;
  return false;
}

PDDL_Base::CPG::CPG(CPG& g, index_vec& s)
  : atoms(0, 0), neg(false, 0), args(g.args), arg_types(g.arg_types)
{
  for (index_type k = 0; k < s.length(); k++) {
    assert(s[k] < g.atoms.length());
    atoms.append(g.atoms[s[k]]);
    assert(s[k] < g.atom_first_arg.length());
    atom_first_arg.append(g.atom_first_arg[s[k]]);
    assert(s[k] < g.neg.length());
    neg.append(g.neg[s[k]]);
  }
}

void PDDL_Base::CPG::make_key(ptr_table::key_vec& key)
{
  key.clear();
  for (index_type k = 0; k < atoms.length(); k++)
    key.append(atoms[k]->pred);
}

void PDDL_Base::CPG::make_typed_key(ptr_table::key_vec& key)
{
  key.clear();
  for (index_type k = 0; k < atoms.length(); k++)
    key.append(atoms[k]->pred);
  for (index_type k = 0; k < args.length(); k++) {
    assert(args[k]->sym_types.length() == 1);
    key.append(args[k]->sym_types[0]);
  }
}

void PDDL_Base::CPG::make_parameters(variable_vec& param)
{
  param.set_length(args.length());
  for (index_type k = 0; k < args.length(); k++) {
    EnumName v_name("?arg", k);
    VariableSymbol* v = new VariableSymbol(v_name.to_cstring());
    v->sym_types.assign_value(arg_types[k]);
    param[k] = v;
  }
}

PDDL_Base::ListExpression* PDDL_Base::CPG::make_argument_list
(index_type first)
{
  if (first < args.length()) {
    return new ListExpression(args[first], make_argument_list(first + 1));
  }
  else {
    return 0;
  }
}

bool PDDL_Base::CPG::initial_value()
{
  for (index_type k = 0; k < atoms.length(); k++) {
    bool atom_init_val = atoms[k]->initial_value();
    if (atom_init_val == neg[k]) return false;
  }
  return true;
}

void PDDL_Base::CPG::add_effect_conditions
(Context* e, ParamSymbol* pf, bool_vec& sat, symbol_vec& subs,
 symbol_pair_vec& eq, symbol_pair_vec& neq)
{
  for (index_type k = 0; k < eq.length(); k++) {
    index_type p = pf->param.first((VariableSymbol*)eq[k].first);
    assert(p != no_such_index);
    Atom* eq_atom = new Atom(PDDL_Base::current_eq_predicate);
    if (subs[p]) {
      eq_atom->param.append(subs[p]);
    }
    else {
      eq_atom->param.append(eq[k].first);
    }
    eq_atom->param.append(eq[k].second);
    e->pos_con.append(eq_atom);
  }
  for (index_type k = 0; k < neq.length(); k++) {
    index_type p = pf->param.first((VariableSymbol*)neq[k].first);
    assert(p != no_such_index);
    Atom* neq_atom = new Atom(PDDL_Base::current_eq_predicate);
    if (subs[p]) {
      neq_atom->param.append(subs[p]);
    }
    else {
      neq_atom->param.append(neq[k].first);
    }
    neq_atom->param.append(neq[k].second);
    e->neg_con.append(neq_atom);
  }
  for (index_type k = 0; k < atoms.length(); k++) if (!sat[k]) {
    PredicateSymbol* pred = atoms[k]->pred;
    Atom* atom = new Atom(pred);
    atom->param.set_length(pred->param.length());
    for (index_type i = 0; i < pred->param.length(); i++) {
      if (subs[atom_first_arg[k] + i]) {
	atom->param[i] = subs[atom_first_arg[k] + i];
      }
      else {
	atom->param[i] = pf->param[atom_first_arg[k] + i];
      }
    }
    if (neg[k]) {
      e->neg_con.append(atom);
    }
    else {
      e->pos_con.append(atom);
    }
  }
}

void PDDL_Base::CPG::add_asserting_effects
(ActionSymbol* act, PredicateSymbol* p, bool p_val, PredicateSymbol* g,
 FunctionSymbol* f, Expression* f_val, bool strict)
{
  bool_vec sat(false, atoms.length());
  symbol_pair_vec eq;
  symbol_pair_vec neq;
  add_asserting_effects(0, act, 0, sat, p, p_val, g, f, f_val,
			eq, neq, strict);
}

void PDDL_Base::CPG::add_asserting_effects
(index_type c_atom, ActionSymbol* act, index_type c_eff, bool_vec& sat,
 PredicateSymbol* p, bool p_val, PredicateSymbol* g,
 FunctionSymbol* f, Expression* f_val,
 symbol_pair_vec& eq, symbol_pair_vec& neq, bool strict)
{
  assert((p == 0) || (f == 0));
  ParamSymbol* pf = (p ? (ParamSymbol*)p : (ParamSymbol*)f);
  assert(pf != 0);
  if (c_atom < atoms.length()) {
    if (c_eff < act->adds.length()) {
//       std::cerr << "current atom = ";
//       atoms[c_atom]->print(std::cerr);
//       std::cerr << ", current effect = :add ";
//       act->adds[c_eff]->print(std::cerr);
//       std::cerr << ", matched = " << matched << std::endl;
      if (atoms[c_atom]->pred == act->adds[c_eff]->pred) {
	if (neg[c_atom]) { // falsifying case (adds negative atom)
	  bool threat = true;
	  for (index_type k = 0; k < act->adds[c_eff]->param.length(); k++)
	    if (!pf->param[atom_first_arg[c_atom] + k]->
		equality_type_check(act->adds[c_eff]->param[k]))
	      threat = false;
	  if (threat) { // effect can threaten atom
	    for (index_type k = 0; k < act->adds[c_eff]->param.length(); k++) {
	      bool poss = true;
	      for (index_type i = 0; i < eq.length(); i++)
		if ((eq[i].first == pf->param[atom_first_arg[c_atom] + k]) &&
		    (eq[i].second == act->adds[c_eff]->param[k]))
		  poss = false;
	      if (poss) {
		neq.append(symbol_pair(pf->param[atom_first_arg[c_atom] + k],
				       act->adds[c_eff]->param[k]));
		add_asserting_effects(c_atom, act, c_eff + 1, sat,
				      p, p_val, g, f, f_val, eq, neq, strict);
		neq.dec_length();
	      }
	    }
	  }
	  else { // no threat
	    add_asserting_effects(c_atom, act, c_eff + 1, sat,
				  p, p_val, g, f, f_val, eq, neq, strict);
	  }
	}
	else { // satisfying case (adds positive atom)
	  if (!sat[c_atom]) {
	    bool poss = true;
	    for (index_type k = 0; (k < act->adds[c_eff]->param.length()) && poss; k++) {
	      for (index_type i = 0; (i < neq.length()) && poss; i++)
		if ((neq[i].first == pf->param[atom_first_arg[c_atom] + k]) &&
		    (neq[i].second == act->adds[c_eff]->param[k]))
		  poss = false;
	      if (!pf->param[atom_first_arg[c_atom] + k]->
		  equality_type_check(act->adds[c_eff]->param[k]))
		poss = false;
	    }
	    if (poss) {
	      for (index_type k = 0; k < act->adds[c_eff]->param.length(); k++)
		eq.append(symbol_pair(pf->param[atom_first_arg[c_atom] + k],
				      act->adds[c_eff]->param[k]));
	      sat[c_atom] = true;
	      add_asserting_effects(c_atom, act, c_eff + 1, sat,
				    p, p_val, g, f, f_val, eq, neq, strict);
	      sat[c_atom] = false;
	      eq.dec_length(act->adds[c_eff]->param.length());
	    }
	  }
	  add_asserting_effects(c_atom, act, c_eff + 1, sat,
				p, p_val, g, f, f_val, eq, neq, strict);
	}
      }
      else { // different predicate
	add_asserting_effects(c_atom, act, c_eff + 1, sat,
			      p, p_val, g, f, f_val, eq, neq, strict);
      }
    }
    else if (c_eff < (act->adds.length() + act->dels.length())) {
      index_type d_eff = c_eff - act->adds.length();
//       std::cerr << "current atom = ";
//       atoms[c_atom]->print(std::cerr);
//       std::cerr << ", current effect = :del ";
//       act->dels[d_eff]->print(std::cerr);
//       std::cerr << ", matched = " << matched << std::endl;
      if (atoms[c_atom]->pred == act->dels[d_eff]->pred) {
	if (!neg[c_atom]) { // falsifying case (dels positive atom)
	  bool threat = true;
	  for (index_type k = 0; k < act->dels[d_eff]->param.length(); k++)
	    if (!pf->param[atom_first_arg[c_atom] + k]->
		equality_type_check(act->dels[d_eff]->param[k]))
	      threat = false;
	  if (threat) { // effect can threaten atom
	    for (index_type k = 0; k < act->dels[d_eff]->param.length(); k++) {
	      bool poss = true;
	      for (index_type i = 0; i < eq.length(); i++)
		if ((eq[i].first == pf->param[atom_first_arg[c_atom] + k]) &&
		    (eq[i].second == act->dels[d_eff]->param[k]))
		  poss = false;
	      if (poss) {
		neq.append(symbol_pair(pf->param[atom_first_arg[c_atom] + k],
				       act->dels[d_eff]->param[k]));
		add_asserting_effects(c_atom, act, c_eff + 1, sat,
				      p, p_val, g, f, f_val, eq, neq, strict);
		neq.dec_length();
	      }
	    }
	  }
	  else {
	    add_asserting_effects(c_atom, act, c_eff + 1, sat,
				  p, p_val, g, f, f_val, eq, neq, strict);
	  }
	}
	else { // satisfying case (dels negative atom)
	  if (!sat[c_atom]) {
	    bool poss = true;
	    for (index_type k = 0; (k < act->dels[d_eff]->param.length()) && poss; k++) {
	      for (index_type i = 0; (i < neq.length()) && poss; i++)
		if ((neq[i].first == pf->param[atom_first_arg[c_atom] + k]) &&
		    (neq[i].second == act->dels[d_eff]->param[k]))
		  poss = false;
	      if (!pf->param[atom_first_arg[c_atom] + k]->
		  equality_type_check(act->adds[c_eff]->param[k]))
		poss = false;
	    }
	    if (poss) {
	      for (index_type k = 0; k < act->dels[d_eff]->param.length(); k++)
		eq.append(symbol_pair(pf->param[atom_first_arg[c_atom] + k],
				      act->dels[d_eff]->param[k]));
	      sat[c_atom] = true;
	      add_asserting_effects(c_atom, act, c_eff + 1, sat,
				    p, p_val, g, f, f_val, eq, neq, strict);
	      sat[c_atom] = false;
	      eq.dec_length(act->dels[d_eff]->param.length());
	    }
	  }
	  add_asserting_effects(c_atom, act, c_eff + 1, sat,
				p, p_val, g, f, f_val, eq, neq, strict);
	}
      }
      else { // different predicate
	add_asserting_effects(c_atom, act, c_eff + 1, sat,
			      p, p_val, g, f, f_val, eq, neq, strict);
      }
    }
    else {
      add_asserting_effects(c_atom + 1, act, 0, sat,
			    p, p_val, g, f, f_val, eq, neq, strict);
    }
  }

  else {
    index_type n_sat = sat.count(true);
    if (n_sat > 0) {
      symbol_vec subs(0, args.length());
      index_type n_sub = 0;
      symbol_pair_vec rem_eq;
      for (index_type k = 0; k < eq.length(); k++) {
	index_type i = pf->param.first((VariableSymbol*)eq[k].first);
	assert(i != no_such_index);
	assert(i < subs.length());
	if (subs[i] == 0) {
	  subs[i] = eq[k].second;
	  n_sub += 1;
	}
	else {
	  rem_eq.append(eq[k]);
	}
      }

      if (p) {
	index_type i = sat.first(true);
	bool done = false;
	while (!done) {
	  SetOf* e = new SetOf();
	  Atom* a = new Atom(p);
	  a->param.set_length(args.length());
	  for (index_type k = 0; k < args.length(); k++) {
	    if (subs[k]) {
	      a->param[k] = subs[k];
	    }
	    else {
	      a->param[k] = p->param[k];
	      e->param.append(p->param[k]);
	    }
	  }
	  if (p_val)
	    e->neg_atoms.append(a);
	  else
	    e->pos_atoms.append(a);
	  add_effect_conditions(e, p, sat, subs, rem_eq, neq);
	  if (strict) {
	    PredicateSymbol* pred = atoms[i]->pred;
	    Atom* atom = new Atom(pred);
	    atom->param.set_length(pred->param.length());
	    for (index_type j = 0; j < pred->param.length(); j++) {
	      if (subs[atom_first_arg[i] + j]) {
		atom->param[j] = subs[atom_first_arg[i] + j];
	      }
	      else {
		atom->param[j] = p->param[atom_first_arg[i] + j];
	      }
	    }
	    if (neg[i]) {
	      e->pos_con.append(atom);
	    }
	    else {
	      e->neg_con.append(atom);
	    }
	  }
	  if (g) {
	    Atom* g_atom = new Atom(g);
	    g_atom->param.set_length(args.length());
	    for (index_type k = 0; k < args.length(); k++) {
	      if (subs[k]) {
		g_atom->param[k] = subs[k];
	      }
	      else {
		g_atom->param[k] = p->param[k];
	      }
	    }
	    e->pos_con.append(g_atom);
	  }
	  if (e->context_is_static()) {
	    act->set_eff.append(e);
	    if (write_info) {
	      std::cerr << "info: added set effect ";
	      e->print(std::cerr);
	      std::cerr << " to action " << act->print_name << std::endl;
	    }
	  }
	  else {
	    act->cond_eff.append(e);
	    if (write_info) {
	      std::cerr << "info: added conditional effect ";
	      e->print(std::cerr);
	      std::cerr << " to action " << act->print_name << std::endl;
	    }
	  }
	  if (strict) {
	    i = sat.next(true, i);
	    if (i == no_such_index) done = true;
	  }
	  else {
	    done = true;
	  }
	}
      }

      if (f) {
	FChangeAtom* a = new FChangeAtom(f);
	QCNumericEffect* e = new QCNumericEffect(a);
	a->param.set_length(args.length());
	for (index_type k = 0; k < args.length(); k++) {
	  if (subs[k]) {
	    a->param[k] = subs[k];
	  }
	  else {
	    a->param[k] = f->param[k];
	    e->param.append(f->param[k]);
	  }
	}
	a->val = f_val->copy();
	add_effect_conditions(e, p, sat, subs, rem_eq, neq);
	act->qc_fass.append(e);

	if (write_info) {
	  std::cerr << "info: added assignment ";
	  e->print(std::cerr);
	  std::cerr << " to action " << act->print_name << std::endl;
	}
      }
    }
  }
}

void PDDL_Base::CPG::add_propositional_effect
(ActionSymbol* act, PredicateSymbol* p, bool p_val,
 index_type c_atom, Atom* a_eff, bool strict)
{
  SetOf* e = new SetOf();
  Atom* a = new Atom(p);
  a->param.set_length(args.length());
  for (index_type k = 0; k < atoms.length(); k++) {
    if (k == c_atom) {
      for (index_type i = 0; i < atoms[k]->param.length(); i++)
	a->param[atom_first_arg[k] + i] = a_eff->param[i];
    }
    else {
      for (index_type i = 0; i < atoms[k]->param.length(); i++) {
	a->param[atom_first_arg[k] + i] =
	  p->param[atom_first_arg[k] + i];
	e->param.append(p->param[atom_first_arg[k] + i]);
      }
    }
  }
  if (p_val) 
    e->neg_atoms.append(a);
  else
    e->pos_atoms.append(a);
  // if strict, add atoms to effect conditions
  if (strict) {
    for (index_type k = 0; k < atoms.length(); k++) {
      a = new Atom(atoms[k]->pred);
      a->param.set_length(args.length());
      if (k == c_atom) {
	for (index_type i = 0; i < atoms[k]->param.length(); i++)
	  a->param[atom_first_arg[k] + i] = a_eff->param[i];
      }
      else {
	for (index_type i = 0; i < atoms[k]->param.length(); i++) {
	  a->param[atom_first_arg[k] + i] = p->param[atom_first_arg[k] + i];
	}
      }
      if (neg[k]) {
	e->neg_con.append(a);
      }
      else {
	e->pos_con.append(a);
      }
    }
  }
  if (!e->context_is_static()) {
    act->cond_eff.append(e);
    if (write_info) {
      std::cerr << "info: added conditional effect ";
      e->print(std::cerr);
      std::cerr << " to action " << act->print_name << std::endl;
    }
  }
  else {
    act->set_eff.append(e);
    if (write_info) {
      std::cerr << "info: added set effect ";
      e->print(std::cerr);
      std::cerr << " to action " << act->print_name << std::endl;
    }
  }
}

void PDDL_Base::CPG::add_fluent_effect
(ActionSymbol* act, FunctionSymbol* f, Expression* f_val,
 index_type c_atom, Atom* a_eff)
{
  FChangeAtom* a = new FChangeAtom(f);
  a->param.set_length(args.length());
  QCNumericEffect* e = new QCNumericEffect(a);
  for (index_type k = 0; k < atoms.length(); k++) {
    if (k == c_atom) {
      for (index_type i = 0; i < atoms[k]->param.length(); i++)
	a->param[atom_first_arg[k] + i] = a_eff->param[i];
    }
    else {
      for (index_type i = 0; i < atoms[k]->param.length(); i++) {
	a->param[atom_first_arg[k] + i] =
	  f->param[atom_first_arg[k] + i];
	e->param.append(f->param[atom_first_arg[k] + i]);
      }
    }
  }
  a->val = f_val->copy();
  act->qc_fass.append(e);
  if (write_info) {
    std::cerr << "info: added assignment ";
    e->print(std::cerr);
    std::cerr << " to action " << act->print_name << std::endl;
  }
}

void PDDL_Base::CPG::add_destroying_effects
(ActionSymbol* act, ParamSymbol* pf,
 PredicateSymbol* p, bool p_val, PredicateSymbol* g,
 FunctionSymbol* f, Expression* f_val, bool strict)
{
  for (index_type c_atom = 0; c_atom < atoms.length(); c_atom++) {
    if (neg[c_atom]) {
      for (index_type c_eff = 0; c_eff < act->adds.length(); c_eff++)
	if (atoms[c_atom]->pred == act->adds[c_eff]->pred) {
	  // potential falsifying case (adds negative atom)
	  bool poss = true;
	  for (index_type k = 0; k < act->adds[c_eff]->param.length(); k++)
	    if (!pf->param[atom_first_arg[c_atom] + k]->
		equality_type_check(act->adds[c_eff]->param[k]))
	      poss = false;
	  if (poss) {
	    // if p != 0, add propositional effect
	    if (p) {
	      add_propositional_effect(act, p, p_val, c_atom, act->adds[c_eff], strict);
	    }
	    // if f != 0, add fluent assignment effect
	    if (f) {
	      assert(!strict); // not implemented for this case
	      add_fluent_effect(act, f, f_val, c_atom, act->adds[c_eff]);
	    }
	  }
	}
    }
    else {
      for (index_type c_eff = 0; c_eff < act->dels.length(); c_eff++)
	if (atoms[c_atom]->pred == act->dels[c_eff]->pred) {
	  // potential falsifying case (dels positive atom)
	  bool poss = true;
	  for (index_type k = 0; k < act->dels[c_eff]->param.length(); k++)
	    if (!pf->param[atom_first_arg[c_atom] + k]->
		equality_type_check(act->dels[c_eff]->param[k]))
	      poss = false;
	  if (poss) {
	    // if p != 0, add propositional effect
	    if (p) {
	      add_propositional_effect(act, p, p_val, c_atom, act->dels[c_eff], strict);
	    }
	    // if f != 0, add fluent assignment effect
	    if (f) {
	      assert(!strict); // not implemented for this case
	      add_fluent_effect(act, f, f_val, c_atom, act->dels[c_eff]);
	    }
	  }
	}
    }
  }
}

PDDL_Base::Expression* PDDL_Base::replace_violations_1
(Expression* exp, CPG* cpg[], FunctionSymbol* f_violated[])
{
  switch (exp->exp_class) {
  case exp_fun:
  case exp_list:
  case exp_const:
    return exp;
  case exp_add:
  case exp_sub:
  case exp_mul:
  case exp_div:
    {
      BinaryExpression* b_exp = (BinaryExpression*)exp;
      Expression* first =
	replace_violations_1(b_exp->first, cpg, f_violated);
      Expression* second =
	replace_violations_1(b_exp->second, cpg, f_violated);
      return new BinaryExpression(exp->exp_class, first, second);
    }
  case exp_time:
    return exp;
  case exp_preference:
    {
      PreferenceExpression* p_exp = (PreferenceExpression*)exp;
      index_type p = no_such_index;
      for (index_type k = 0;
	   (k < dom_preferences.length()) && (p == no_such_index); k++)
	if (dom_preferences[k]->name == p_exp->name) p = k;
      if (p == no_such_index) {
	std::cerr << "error: undefined preference in expression ";
	p_exp->print(std::cerr, false);
	std::cerr << std::endl;
	exit(255);
      }
      if (cpg[p]) {
	ListExpression* args = cpg[p]->make_argument_list(0);
	FunctionExpression* f_exp =
	  new FunctionExpression(f_violated[p], args);
	if (write_info) {
	  std::cerr << "info: replaced ";
	  p_exp->print(std::cerr, false);
	  std::cerr << " by ";
	  f_exp->print(std::cerr, false);
	  std::cerr << " in :metric" << std::endl;
	}
	return f_exp;
      }
      else {
	return p_exp;
      }
    }
  default:
    std::cerr << "error: invalid expression class (" << exp->exp_class << ")"
	      << std::endl;
    exit(255);
  }
}

void PDDL_Base::compile_preferences()
{
  if (dom_preferences.length() == 0) return;
  current_eq_predicate = dom_eq_pred;

  CPG* cpg[dom_preferences.length()];
  FunctionSymbol* f_violated[dom_preferences.length()];
  ptr_table signatures;
  ptr_table* signature[dom_preferences.length()];
  bool_vec compilable(false, dom_preferences.length());
  index_type n_compilable = 0;
  index_type n_signatures = 0;

  for (index_type p = 0; p < dom_preferences.length(); p++) {
    CPG* g = new CPG();
    if (dom_preferences[p]->goal->makeCPG(*g)) {
      cpg[p] = g;
      ptr_table::key_vec sig;
      g->make_key(sig);
      signature[p] = signatures.insert(sig);
      compilable[p] = true;
      n_compilable += 1;
    }
    else {
      cpg[p] = 0;
      signature[p] = 0;
    }
    f_violated[p] = 0;
  }
  if (write_info) {
    std::cerr << "info: " << n_compilable << " compilable preferences found"
	      << std::endl;
  }

  if ((n_compilable < dom_preferences.length()) &&
      (write_warnings || !best_effort)) {
    std::cerr << "warning: " << (dom_preferences.length() - n_compilable)
	      << " preferences can not be compiled"
	      << std::endl;
    if (!best_effort) exit(1);
  }

  if (n_compilable == 0) return;

  for (index_type p = 0; p < dom_preferences.length(); p++) if (cpg[p]) {
    if (write_info) {
      std::cerr << "info: compiling ";
      dom_preferences[p]->print(std::cerr);
      std::cerr << std::endl;
    }
    if (f_violated[p] == 0) {
      EnumName f_name("violated", n_signatures);
      f_violated[p] = new FunctionSymbol(f_name.to_cstring());
      index_type f_arity = cpg[p]->args.length();
      type_vec f_types(cpg[p]->arg_types);
      // copy_type_vector(cpg[p]->arg_types, f_arity, f_types);
      for (index_type q = p + 1; q < dom_preferences.length(); q++)
	if (signature[q] == signature[p])
	  if (merge_type_vectors(f_types, cpg[q]->arg_types))
	    f_violated[q] = f_violated[p];
      make_parameters(f_types, "?arg", f_violated[p]->param);
      f_violated[p]->modified = true;
      f_violated[p]->integral = true;
      f_violated[p]->linear = false;
      f_violated[p]->assigned = true;
      dom_functions.append(f_violated[p]);
      if (write_info) {
	std::cerr << "info: created CPG function ";
	f_violated[p]->print(std::cerr);
      }

      Expression* exp0 = new ConstantExpression(0);
      Expression* exp1 = new ConstantExpression(1);
      for (index_type a = 0; a < dom_actions.length(); a++) {
	bool_vec sat(false, cpg[p]->atoms.length());
	symbol_pair_vec eq;
	symbol_pair_vec neq;
	cpg[p]->add_asserting_effects(0, dom_actions[a], 0, sat,
				      0, false, 0, f_violated[p], exp0,
				      eq, neq, false);
	cpg[p]->add_destroying_effects(dom_actions[a], f_violated[p],
				       0, false, 0, f_violated[p], exp1, false);
      }

      n_signatures += 1;
    }

    FInitAtom* f_init = new FInitAtom(f_violated[p]);
    f_init->param = cpg[p]->args;
    if (cpg[p]->initial_value()) {
      f_init->val = 0;
    }
    else {
      f_init->val = 1;
    }
    f_init->insert(f_violated[p]->init);
    dom_fun_init.append(f_init);
  }
  else {
    if (write_info) {
      std::cerr << "info: ";
      dom_preferences[p]->print(std::cerr);
      std::cerr << " can not be compiled" << std::endl;
    }
  }

  metric = replace_violations_1(metric, cpg, f_violated);

  dom_preferences.remove(compilable);

  if (write_info) {
    std::cerr << "info: "
	      << n_compilable << " preferences compiled, "
	      << n_signatures << " CPG functions created, "
	      << dom_preferences.length() << " preferences remain"
	      << std::endl;
  }
}

void PDDL_Base::add_precondition_formula
(ActionSymbol* a, CPG* f, bool is_neg)
{
  assert(f);
  if (is_neg) {
    if (f->atoms.length() > 1) {
      SetOf* d = new SetOf();
      for (index_type k = 0; k < f->atoms.length(); k++) {
	if (f->neg[k])
	  d->pos_atoms.append(new Atom(f->atoms[k]));
	else
	  d->neg_atoms.append(new Atom(f->atoms[k]));
      }
      a->dis_pre.append(d);
    }
    else if (f->atoms.length() == 1) {
      if (f->neg[0])
	a->pos_pre.append(new Atom(f->atoms[0]));
      else
	a->neg_pre.append(new Atom(f->atoms[0]));
    }
  }
  else {
    for (index_type k = 0; k < f->atoms.length(); k++) {
      if (f->neg[k])
	a->neg_pre.append(new Atom(f->atoms[k]));
      else
	a->pos_pre.append(new Atom(f->atoms[k]));
    }
  }
}

void PDDL_Base::make_automaton_transition
(Symbol* s_from, Symbol* s_to, bool is_accept,
 CPG* f, bool neg_f, CPG* g, bool neg_g,
 PredicateSymbol* p_state, PredicateSymbol* p_accept,
 PredicateSymbol* p_synch)
{
  ActionSymbol* a_trans = (ActionSymbol*)gensym(sym_action, "trans", 0);
  // preconditions
  a_trans->pos_pre.append(new Atom(p_state, s_from));
  a_trans->neg_pre.append(new Atom(p_synch));
  if (f) {
    add_precondition_formula(a_trans, f, neg_f);
  }
  if (g) {
    add_precondition_formula(a_trans, g, neg_g);
  }
  // effect: change of state
  if (s_to != s_from) {
    a_trans->dels.append(new Atom(p_state, s_from));
    a_trans->adds.append(new Atom(p_state, s_to));
  }
  // effect: set/clear accept flag
  if (is_accept)
    a_trans->adds.append(new Atom(p_accept));
  else
    a_trans->dels.append(new Atom(p_accept));
  // effect: set synch flag
  a_trans->adds.append(new Atom(p_synch));

  dom_actions.append(a_trans);
}

PDDL_Base::AtomicGoal* PDDL_Base::make_automaton_type_a
(CPG& f, index_type i, const Symbol* n, symbol_vec& aut_state, index_type n_ra)
{
  assert(aut_state.length() >= 2);
  assert(n_ra <= dom_actions.length());

  // create predicates
  PredicateSymbol* p_state = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "state-", n, 0) :
     gensym_i(sym_predicate, "state-c", i, 0));
  VariableSymbol* param0 =
    (VariableSymbol*)gensym(sym_variable, "?s", aut_state[0]->sym_types);
  p_state->param.append(param0);
  p_state->pos_pre = true;
  p_state->added = true;
  p_state->deleted = true;
  p_state->modded = true;
  PredicateSymbol* p_accept = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "accepting-", n, 0) :
     gensym_i(sym_predicate, "accepting-c", i, 0));
  p_accept->pos_pre = true;
  p_accept->added = true;
  p_accept->deleted = true;
  p_accept->modded = true;
  PredicateSymbol* p_synch = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "updated-", n, 0) :
     gensym_i(sym_predicate, "updated-c", i, 0));
  p_synch->pos_pre = true;
  p_synch->neg_pre = true;
  p_synch->added = true;
  p_synch->deleted = true;
  p_synch->modded = true;

  dom_predicates.append(p_state);
  dom_predicates.append(p_accept);
  dom_predicates.append(p_synch);

  // create transition actions
  // s0 -> s1
  make_automaton_transition(aut_state[0], aut_state[1], true,
			    &f, false, 0, false,
			    p_state, p_accept, p_synch);
  // s0 -> s2
  make_automaton_transition(aut_state[0], aut_state[2], false,
			    0, false, 0, false,
			    p_state, p_accept, p_synch);
  // s1 -> s1
  make_automaton_transition(aut_state[1], aut_state[1], true,
			    &f, false, 0, false,
			    p_state, p_accept, p_synch);
  // s1 -> s2
  make_automaton_transition(aut_state[1], aut_state[2], false,
			    0, false, 0, false,
			    p_state, p_accept, p_synch);
  // s2 -> s2
  make_automaton_transition(aut_state[2], aut_state[2], false,
			    0, false, 0, false,
			    p_state, p_accept, p_synch);

  // add synch flag to regular actions
  for (index_type k = 0; k < n_ra; k++) {
    dom_actions[k]->pos_pre.append(new Atom(p_synch));
    dom_actions[k]->dels.append(new Atom(p_synch));
  }

  // and to goal
  Atom* a_synch = new Atom(p_synch);
  a_synch->insert(p_synch->pos_goal);
  dom_goals.append(new AtomicGoal(a_synch, false));

  // add initial state to init
  Atom* a_init = new Atom(p_state);
  a_init->param.append(aut_state[0]);
  a_init->insert(p_state->init);
  dom_init.append(a_init);

  // and return goal atom
  Atom* a_accept = new Atom(p_accept);
  a_accept->insert(p_accept->pos_goal);
  return new AtomicGoal(a_accept, false);
}

PDDL_Base::AtomicGoal* PDDL_Base::make_automaton_type_e
(CPG& f, index_type i, const Symbol* n, symbol_vec& aut_state, index_type n_ra)
{
  assert(aut_state.length() >= 2);
  assert(n_ra <= dom_actions.length());

  // create predicates
  PredicateSymbol* p_state = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "state-", n, 0) :
     gensym_i(sym_predicate, "state-c", i, 0));
  VariableSymbol* param0 =
    (VariableSymbol*)gensym(sym_variable, "?s", aut_state[0]->sym_types);
  p_state->param.append(param0);
  p_state->pos_pre = true;
  p_state->added = true;
  p_state->deleted = true;
  p_state->modded = true;
  PredicateSymbol* p_accept = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "accepting-", n, 0) :
     gensym_i(sym_predicate, "accepting-c", i, 0));
  p_accept->pos_pre = true;
  p_accept->added = true;
  p_accept->deleted = true;
  p_accept->modded = true;
  PredicateSymbol* p_synch = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "updated-", n, 0) :
     gensym_i(sym_predicate, "updated-c", i, 0));
  p_synch->pos_pre = true;
  p_synch->neg_pre = true;
  p_synch->added = true;
  p_synch->deleted = true;
  p_synch->modded = true;

  dom_predicates.append(p_state);
  dom_predicates.append(p_accept);
  dom_predicates.append(p_synch);

  // create transition actions
  // s0 -> s1
  make_automaton_transition(aut_state[0], aut_state[1], false,
			    0, false, 0, false,
			    p_state, p_accept, p_synch);
  // s0 -> s2
  make_automaton_transition(aut_state[0], aut_state[2], true,
			    &f, false, 0, false,
			    p_state, p_accept, p_synch);
  // s1 -> s1
  make_automaton_transition(aut_state[1], aut_state[1], false,
			    0, false, 0, false,
			    p_state, p_accept, p_synch);
  // s1 -> s2
  make_automaton_transition(aut_state[1], aut_state[2], true,
			    &f, false, 0, false,
			    p_state, p_accept, p_synch);
  // s2 -> s3
  make_automaton_transition(aut_state[2], aut_state[3], true,
			    0, false, 0, false,
			    p_state, p_accept, p_synch);

  // add synch flag to regular actions
  for (index_type k = 0; k < n_ra; k++) {
    dom_actions[k]->pos_pre.append(new Atom(p_synch));
    dom_actions[k]->dels.append(new Atom(p_synch));
  }

  // and to goal
  Atom* a_synch = new Atom(p_synch);
  a_synch->insert(p_synch->pos_goal);
  dom_goals.append(new AtomicGoal(a_synch, false));

  // add initial state to init
  Atom* a_init = new Atom(p_state);
  a_init->param.append(aut_state[0]);
  a_init->insert(p_state->init);
  dom_init.append(a_init);

  // and return goal atom
  Atom* a_accept = new Atom(p_accept);
  a_accept->insert(p_accept->pos_goal);
  return new AtomicGoal(a_accept, false);
}

PDDL_Base::AtomicGoal* PDDL_Base::make_automaton_type_o
(CPG& f, index_type i, const Symbol* n, symbol_vec& aut_state, index_type n_ra)
{
  assert(aut_state.length() >= 2);
  assert(n_ra <= dom_actions.length());

  // create predicates
  PredicateSymbol* p_state = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "state-", n, 0) :
     gensym_i(sym_predicate, "state-c", i, 0));
  VariableSymbol* param0 =
    (VariableSymbol*)gensym(sym_variable, "?s", aut_state[0]->sym_types);
  p_state->param.append(param0);
  p_state->pos_pre = true;
  p_state->added = true;
  p_state->deleted = true;
  p_state->modded = true;
  PredicateSymbol* p_accept = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "accepting-", n, 0) :
     gensym_i(sym_predicate, "accepting-c", i, 0));
  p_accept->pos_pre = true;
  p_accept->added = true;
  p_accept->deleted = true;
  p_accept->modded = true;
  PredicateSymbol* p_synch = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "updated-", n, 0) :
     gensym_i(sym_predicate, "updated-c", i, 0));
  p_synch->pos_pre = true;
  p_synch->neg_pre = true;
  p_synch->added = true;
  p_synch->deleted = true;
  p_synch->modded = true;

  dom_predicates.append(p_state);
  dom_predicates.append(p_accept);
  dom_predicates.append(p_synch);

  // create transition actions
  // s0 -> s1
  make_automaton_transition(aut_state[0], aut_state[1], true,
			    &f, true, 0, false,
			    p_state, p_accept, p_synch);
  // s0 -> s2
  make_automaton_transition(aut_state[0], aut_state[2], true,
			    &f, false, 0, false,
			    p_state, p_accept, p_synch);
  // s1 -> s1
  make_automaton_transition(aut_state[1], aut_state[1], true,
			    &f, true, 0, false,
			    p_state, p_accept, p_synch);
  // s1 -> s2
  make_automaton_transition(aut_state[1], aut_state[2], true,
			    &f, false, 0, false,
			    p_state, p_accept, p_synch);
  // s2 -> s2
  make_automaton_transition(aut_state[2], aut_state[2], true,
			    &f, false, 0, false,
			    p_state, p_accept, p_synch);
  // s2 -> s3
  make_automaton_transition(aut_state[2], aut_state[3], true,
			    &f, true, 0, false,
			    p_state, p_accept, p_synch);
  // s3 -> s3
  make_automaton_transition(aut_state[3], aut_state[3], true,
			    &f, true, 0, false,
			    p_state, p_accept, p_synch);
  // s3 -> s4
  make_automaton_transition(aut_state[3], aut_state[4], false,
			    0, false, 0, false,
			    p_state, p_accept, p_synch);
  // s4 -> s4
  make_automaton_transition(aut_state[4], aut_state[4], false,
			    0, false, 0, false,
			    p_state, p_accept, p_synch);

  // add synch flag to regular actions
  for (index_type k = 0; k < n_ra; k++) {
    dom_actions[k]->pos_pre.append(new Atom(p_synch));
    dom_actions[k]->dels.append(new Atom(p_synch));
  }

  // and to goal
  Atom* a_synch = new Atom(p_synch);
  a_synch->insert(p_synch->pos_goal);
  dom_goals.append(new AtomicGoal(a_synch, false));

  // add initial state to init
  Atom* a_init = new Atom(p_state);
  a_init->param.append(aut_state[0]);
  a_init->insert(p_state->init);
  dom_init.append(a_init);

  // and return goal atom
  Atom* a_accept = new Atom(p_accept);
  a_accept->insert(p_accept->pos_goal);
  return new AtomicGoal(a_accept, false);
}

PDDL_Base::AtomicGoal* PDDL_Base::make_automaton_type_sb
(CPG& f, CPG& g, index_type i, const Symbol* n, symbol_vec& aut_state,
 index_type n_ra)
{
  assert(aut_state.length() >= 2);
  assert(n_ra <= dom_actions.length());

  // create predicates
  PredicateSymbol* p_state = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "state-", n, 0) :
     gensym_i(sym_predicate, "state-c", i, 0));
  VariableSymbol* param0 =
    (VariableSymbol*)gensym(sym_variable, "?s", aut_state[0]->sym_types);
  p_state->param.append(param0);
  p_state->pos_pre = true;
  p_state->added = true;
  p_state->deleted = true;
  p_state->modded = true;
  PredicateSymbol* p_accept = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "accepting-", n, 0) :
     gensym_i(sym_predicate, "accepting-c", i, 0));
  p_accept->pos_pre = true;
  p_accept->added = true;
  p_accept->deleted = true;
  p_accept->modded = true;
  PredicateSymbol* p_synch = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "updated-", n, 0) :
     gensym_i(sym_predicate, "updated-c", i, 0));
  p_synch->pos_pre = true;
  p_synch->neg_pre = true;
  p_synch->added = true;
  p_synch->deleted = true;
  p_synch->modded = true;

  dom_predicates.append(p_state);
  dom_predicates.append(p_accept);
  dom_predicates.append(p_synch);

  // create transition actions
  // s0 -> s1
  make_automaton_transition(aut_state[0], aut_state[1], true,
			    &f, true, 0, false,
			    p_state, p_accept, p_synch);
  // s0 -> s2
  make_automaton_transition(aut_state[0], aut_state[2], true,
			    &f, true, &g, false,
			    p_state, p_accept, p_synch);
  // s0 -> s4
  make_automaton_transition(aut_state[0], aut_state[4], false,
			    0, true, 0, false,
			    p_state, p_accept, p_synch);
  // s1 -> s1
  make_automaton_transition(aut_state[1], aut_state[1], true,
			    &f, true, 0, false,
			    p_state, p_accept, p_synch);
  // s1 -> s2
  make_automaton_transition(aut_state[1], aut_state[2], true,
			    &f, true, &g, false,
			    p_state, p_accept, p_synch);
  // s1 -> s4
  make_automaton_transition(aut_state[1], aut_state[4], false,
			    0, true, 0, false,
			    p_state, p_accept, p_synch);
  // s2 -> s3
  make_automaton_transition(aut_state[2], aut_state[3], true,
			    0, true, 0, false,
			    p_state, p_accept, p_synch);
  // s3 -> s3
  make_automaton_transition(aut_state[3], aut_state[3], true,
			    0, true, 0, false,
			    p_state, p_accept, p_synch);
  // s4 -> s4
  make_automaton_transition(aut_state[4], aut_state[4], false,
			    0, true, 0, false,
			    p_state, p_accept, p_synch);

  // add synch flag to regular actions
  for (index_type k = 0; k < n_ra; k++) {
    dom_actions[k]->pos_pre.append(new Atom(p_synch));
    dom_actions[k]->dels.append(new Atom(p_synch));
  }

  // and to goal
  Atom* a_synch = new Atom(p_synch);
  a_synch->insert(p_synch->pos_goal);
  dom_goals.append(new AtomicGoal(a_synch, false));

  // add initial state to init
  Atom* a_init = new Atom(p_state);
  a_init->param.append(aut_state[0]);
  a_init->insert(p_state->init);
  dom_init.append(a_init);

  // and return goal atom
  Atom* a_accept = new Atom(p_accept);
  a_accept->insert(p_accept->pos_goal);
  return new AtomicGoal(a_accept, false);
}

PDDL_Base::AtomicGoal* PDDL_Base::make_automaton_type_sa
(CPG& f, CPG& g, index_type i, const Symbol* n, symbol_vec& aut_state,
 index_type n_ra)
{
  assert(aut_state.length() >= 2);
  assert(n_ra <= dom_actions.length());

  // create predicates
  PredicateSymbol* p_state = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "state-", n, 0) :
     gensym_i(sym_predicate, "state-c", i, 0));
  VariableSymbol* param0 =
    (VariableSymbol*)gensym(sym_variable, "?s", aut_state[0]->sym_types);
  p_state->param.append(param0);
  p_state->pos_pre = true;
  p_state->added = true;
  p_state->deleted = true;
  p_state->modded = true;
  PredicateSymbol* p_accept = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "accepting-", n, 0) :
     gensym_i(sym_predicate, "accepting-c", i, 0));
  p_accept->pos_pre = true;
  p_accept->added = true;
  p_accept->deleted = true;
  p_accept->modded = true;
  PredicateSymbol* p_synch = (PredicateSymbol*)
    (n ? gensym_s(sym_predicate, "updated-", n, 0) :
     gensym_i(sym_predicate, "updated-c", i, 0));
  p_synch->pos_pre = true;
  p_synch->neg_pre = true;
  p_synch->added = true;
  p_synch->deleted = true;
  p_synch->modded = true;

  dom_predicates.append(p_state);
  dom_predicates.append(p_accept);
  dom_predicates.append(p_synch);

  // create transition actions
  // s0 -> s1
  make_automaton_transition(aut_state[0], aut_state[1], true,
			    0, false, &g, false,
			    p_state, p_accept, p_synch);
  // s0 -> s2
  make_automaton_transition(aut_state[0], aut_state[2], false,
			    &f, false, &g, false,
			    p_state, p_accept, p_synch);
  // s0 -> s3
  make_automaton_transition(aut_state[0], aut_state[3], true,
			    &f, true, 0, false,
			    p_state, p_accept, p_synch);
  // s1 -> s1
  make_automaton_transition(aut_state[1], aut_state[1], true,
			    0, false, &g, false,
			    p_state, p_accept, p_synch);
  // s1 -> s2
  make_automaton_transition(aut_state[1], aut_state[2], false,
			    0, false, &g, false,
			    p_state, p_accept, p_synch);
  // s1 -> s3
  make_automaton_transition(aut_state[1], aut_state[3], true,
			    &f, true, 0, false,
			    p_state, p_accept, p_synch);
  // s2 -> s1
  make_automaton_transition(aut_state[2], aut_state[1], true,
			    0, false, &g, false,
			    p_state, p_accept, p_synch);
  // s2 -> s4
  make_automaton_transition(aut_state[2], aut_state[4], false,
			    0, false, 0, false,
			    p_state, p_accept, p_synch);
  // s3 -> s1
  make_automaton_transition(aut_state[3], aut_state[1], true,
			    0, false, &g, false,
			    p_state, p_accept, p_synch);
  // s3 -> s2
  make_automaton_transition(aut_state[3], aut_state[2], false,
			    0, false, &g, false,
			    p_state, p_accept, p_synch);
  // s3 -> s3
  make_automaton_transition(aut_state[3], aut_state[3], true,
			    &f, true, 0, false,
			    p_state, p_accept, p_synch);
  // s4 -> s1
  make_automaton_transition(aut_state[4], aut_state[1], true,
			    0, false, &g, false,
			    p_state, p_accept, p_synch);
  // s4 -> s4
  make_automaton_transition(aut_state[2], aut_state[4], false,
			    0, false, 0, false,
			    p_state, p_accept, p_synch);

  // add synch flag to regular actions
  for (index_type k = 0; k < n_ra; k++) {
    dom_actions[k]->pos_pre.append(new Atom(p_synch));
    dom_actions[k]->dels.append(new Atom(p_synch));
  }

  // and to goal
  Atom* a_synch = new Atom(p_synch);
  a_synch->insert(p_synch->pos_goal);
  dom_goals.append(new AtomicGoal(a_synch, false));

  // add initial state to init
  Atom* a_init = new Atom(p_state);
  a_init->param.append(aut_state[0]);
  a_init->insert(p_state->init);
  dom_init.append(a_init);

  // and return goal atom
  Atom* a_accept = new Atom(p_accept);
  a_accept->insert(p_accept->pos_goal);
  return new AtomicGoal(a_accept, false);
}

PDDL_Base::Goal* PDDL_Base::compile_constraint_1
(PDDL_Base::Goal* g, index_type i, const Symbol* n,
 symbol_vec& aut_states, index_type n_ra)
{
  if (g->g_class == goal_always) {
    SimpleSequenceGoal* g1 = (SimpleSequenceGoal*)g;
    CPG f;
    bool ok = g1->constraint->makeCPG(f);
    if (!ok) {
      if (write_warnings || !best_effort) {
	std::cerr << "warning: can't compile goal ";
	g->print(std::cerr);
	std::cerr << std::endl;
	if (!best_effort) exit(1);
      }
      return g;
    }
    return make_automaton_type_a(f, i, n, aut_states, n_ra);
  }
  else if (g->g_class == goal_sometime) {
    SimpleSequenceGoal* g1 = (SimpleSequenceGoal*)g;
    CPG f;
    bool ok = g1->constraint->makeCPG(f);
    if (!ok) {
      if (write_warnings || !best_effort) {
	std::cerr << "warning: can't compile goal ";
	g->print(std::cerr);
	std::cerr << std::endl;
	if (!best_effort) exit(1);
      }
      return g;
    }
    return make_automaton_type_e(f, i, n, aut_states, n_ra);
  }
  else if (g->g_class == goal_at_most_once) {
    SimpleSequenceGoal* g1 = (SimpleSequenceGoal*)g;
    CPG f;
    bool ok = g1->constraint->makeCPG(f);
    if (!ok) {
      if (write_warnings || !best_effort) {
	std::cerr << "warning: can't compile goal ";
	g->print(std::cerr);
	std::cerr << std::endl;
	if (!best_effort) exit(1);
      }
      return g;
    }
    return make_automaton_type_o(f, i, n, aut_states, n_ra);
  }
  else if (g->g_class == goal_sometime_before) {
    TriggeredSequenceGoal* g1 = (TriggeredSequenceGoal*)g;
    CPG f1;
    bool ok = g1->trigger->makeCPG(f1);
    CPG f2;
    ok = (ok && g1->constraint->makeCPG(f2));
    if (!ok) {
      if (write_warnings || !best_effort) {
	std::cerr << "warning: can't compile goal ";
	g->print(std::cerr);
	std::cerr << std::endl;
	if (!best_effort) exit(1);
      }
      return g;
    }
    return make_automaton_type_sb(f1, f2, i, n, aut_states, n_ra);
  }
  else if (g->g_class == goal_sometime_after) {
    TriggeredSequenceGoal* g1 = (TriggeredSequenceGoal*)g;
    CPG f1;
    bool ok = g1->trigger->makeCPG(f1);
    CPG f2;
    ok = (ok && g1->constraint->makeCPG(f2));
    if (!ok) {
      if (write_warnings || !best_effort) {
	std::cerr << "warning: can't compile goal ";
	g->print(std::cerr);
	std::cerr << std::endl;
	if (!best_effort) exit(1);
      }
      return g;
    }
    return make_automaton_type_sa(f1, f2, i, n, aut_states, n_ra);
  }
  else if (!g->is_state()) {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: can't compile goal ";
      g->print(std::cerr);
      std::cerr << std::endl;
      if (!best_effort) exit(1);
    }
  }
  return g;
}

void PDDL_Base::compile_constraints_1()
{
  index_type n_regular_actions = dom_actions.length();
  symbol_vec aut_states;
  aut_states.append(gensym(sym_object, "aut-state-s0", 0));
  aut_states.append(gensym(sym_object, "aut-state-s1", 0));
  aut_states.append(gensym(sym_object, "aut-state-s2", 0));
  aut_states.append(gensym(sym_object, "aut-state-s3", 0));
  aut_states.append(gensym(sym_object, "aut-state-s4", 0));
  index_type n_goals = dom_goals.length();
  for (index_type k = 0; k < n_goals; k++) {
    dom_goals[k] =
      compile_constraint_1(dom_goals[k], k, 0, aut_states, n_regular_actions);
  }
  for (index_type k = 0; k < dom_preferences.length(); k++) {
    dom_preferences[k]->goal =
      compile_constraint_1(dom_preferences[k]->goal,
			   dom_goals.length() + k,
			   dom_preferences[k]->name,
			   aut_states, n_regular_actions);
  }
  dom_constants.append(aut_states);
  for (index_type k = 0; k < dom_constants.length(); k++)
    dom_constants[k]->defined_in_problem = false;
}

PDDL_Base::Goal* PDDL_Base::compile_always_constraint
(SimpleSequenceGoal* g, const Symbol* n)
{
  current_eq_predicate = dom_eq_pred;
  if (write_info) {
    std::cerr << "info: compiling constraint ";
    g->print(std::cerr);
    std::cerr << std::endl;
  }
  CPG f;
  if (!g->constraint->makeCPG(f)) {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: can't compile goal ";
      g->print(std::cerr);
      std::cerr << std::endl;
      if (!best_effort) exit(1);
    }
    return g;
  }
  PredicateSymbol* p_ok =
    (PredicateSymbol*)gensym_s(sym_predicate, "ok-", n, 0);
  make_parameters(f.arg_types, "?arg", p_ok->param);
  p_ok->pos_pre = true;
  dom_predicates.append(p_ok);
  if (write_info) {
    std::cerr << "info: created predicate ";
    p_ok->print(std::cerr);
    std::cerr << std::endl;
  }

  Atom* a_ok = new Atom(p_ok);
  a_ok->param = f.args;
  a_ok->insert(p_ok->pos_goal);
  if (f.initial_value()) {
    // add atom to init
    Atom* a_init = new Atom(p_ok);
    a_init->param = f.args;
    a_init->insert(p_ok->init);
    dom_init.append(a_init);
    p_ok->deleted = true;
    p_ok->modded = true;

    for (index_type k = 0; k < dom_actions.length(); k++) {
      f.add_destroying_effects(dom_actions[k], p_ok, p_ok, false, 0, 0, 0, false);
    }
  }

  return new AtomicGoal(a_ok, false);
}

PDDL_Base::Goal* PDDL_Base::compile_sometime_constraint
(SimpleSequenceGoal* g, const Symbol* n)
{
  current_eq_predicate = dom_eq_pred;
  if (write_info) {
    std::cerr << "info: compiling constraint ";
    g->print(std::cerr);
    std::cerr << std::endl;
  }
  CPG f;
  if (!g->constraint->makeCPG(f)) {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: can't compile goal ";
      g->print(std::cerr);
      std::cerr << std::endl;
      if (!best_effort) exit(1);
    }
    return g;
  }
  PredicateSymbol* p_ok =
    (PredicateSymbol*)gensym_s(sym_predicate, "ok-", n, 0);
  make_parameters(f.arg_types, "?arg", p_ok->param);
  p_ok->pos_pre = true;
  dom_predicates.append(p_ok);
  if (write_info) {
    std::cerr << "info: created predicate ";
    p_ok->print(std::cerr);
    std::cerr << std::endl;
  }
  Atom* a_ok = new Atom(p_ok);
  a_ok->param = f.args;
  a_ok->insert(p_ok->pos_goal);
  if (f.initial_value()) {
    Atom* a_init = new Atom(p_ok);
    a_init->param = f.args;
    a_init->insert(p_ok->init);
    dom_init.append(a_init);
  }
  else {
    p_ok->added = true;
    p_ok->modded = true;
    for (index_type k = 0; k < dom_actions.length(); k++) {
      f.add_asserting_effects(dom_actions[k], p_ok, true, 0, 0, 0, false);
    }
  }
  return new AtomicGoal(a_ok, false);
}

PDDL_Base::Goal* PDDL_Base::compile_at_most_once_constraint
(SimpleSequenceGoal* g, const Symbol* n)
{
  current_eq_predicate = dom_eq_pred;
  if (write_info) {
    std::cerr << "info: compiling constraint ";
    g->print(std::cerr);
    std::cerr << std::endl;
  }
  CPG f;
  if (!g->constraint->makeCPG(f)) {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: can't compile goal ";
      g->print(std::cerr);
      std::cerr << std::endl;
      if (!best_effort) exit(1);
    }
    return g;
  }
  PredicateSymbol* p_once =
    (PredicateSymbol*)gensym_s(sym_predicate, "once-", n, 0);
  make_parameters(f.arg_types, "?arg", p_once->param);
  p_once->pos_pre = true;
  p_once->added = true;
  p_once->modded = true;
  dom_predicates.append(p_once);
  PredicateSymbol* p_ok =
    (PredicateSymbol*)gensym_s(sym_predicate, "ok-", n, 0);
  make_parameters(f.arg_types, "?arg", p_ok->param);
  p_ok->pos_pre = true;
  p_ok->deleted = true;
  p_ok->modded = true;
  dom_predicates.append(p_ok);
  if (write_info) {
    std::cerr << "info: created predicates ";
    p_once->print(std::cerr);
    std::cerr << " and ";
    p_ok->print(std::cerr);
    std::cerr << std::endl;
  }
  Atom* a_ok = new Atom(p_ok);
  a_ok->param = f.args;
  a_ok->insert(p_ok->pos_goal);

  Atom* a_init = new Atom(p_ok);
  a_init->param = f.args;
  a_init->insert(p_ok->init);
  dom_init.append(a_init);

  // every action that destroys f when it is true adds "once"
  for (index_type k = 0; k < dom_actions.length(); k++) {
    f.add_destroying_effects(dom_actions[k], p_once, p_once, true, 0, 0, 0, true);
  }

  // every action that adds f when it is false and "once" is true dels "ok"
  for (index_type k = 0; k < dom_actions.length(); k++) {
    bool_vec sat(false, f.atoms.length());
    symbol_pair_vec eq;
    symbol_pair_vec neq;
    f.add_asserting_effects(dom_actions[k], p_ok, false, p_once, 0, 0, true);
  }
  return new AtomicGoal(a_ok, false);
}

PDDL_Base::Goal* PDDL_Base::compile_sometime_before_constraint
(TriggeredSequenceGoal* g, const Symbol* n)
{
  current_eq_predicate = dom_eq_pred;
  if (write_info) {
    std::cerr << "info: compiling constraint ";
    g->print(std::cerr);
    std::cerr << std::endl;
  }
  CPG m;
  index_vec s1;
  bool ok = g->trigger->makeCPG(m, s1);
  index_vec s2;
  ok = (ok && g->constraint->makeCPG(m, s2));
  if (!ok) {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: can't compile goal ";
      g->print(std::cerr);
      std::cerr << std::endl;
      if (!best_effort) exit(1);
    }
    return g;
  }
  CPG f_tr(m, s1);
  CPG f_c(m, s2);
  PredicateSymbol* p_not_safe =
    (PredicateSymbol*)gensym_s(sym_predicate, "not-safe-", n, 0);
  make_parameters(f_c.arg_types, "?arg", p_not_safe->param);
  p_not_safe->pos_pre = true;
  p_not_safe->deleted = true;
  p_not_safe->modded = true;
  dom_predicates.append(p_not_safe);
  PredicateSymbol* p_ok =
    (PredicateSymbol*)gensym_s(sym_predicate, "ok-", n, 0);
  make_parameters(f_tr.arg_types, "?arg", p_ok->param);
  p_ok->pos_pre = true;
  p_ok->deleted = true;
  p_ok->modded = true;
  dom_predicates.append(p_ok);
  if (write_info) {
    std::cerr << "info: created predicates ";
    p_not_safe->print(std::cerr);
    std::cerr << " and ";
    p_ok->print(std::cerr);
    std::cerr << std::endl;
  }
  Atom* a_ok = new Atom(p_ok);
  a_ok->param = f_tr.args;
  a_ok->insert(p_ok->pos_goal);

  if (!f_tr.initial_value()) {
    Atom* a_init = new Atom(p_ok);
    a_init->param = f_tr.args;
    a_init->insert(p_ok->init);
    dom_init.append(a_init);
  }

  if (!f_c.initial_value()) {
    Atom* a_init = new Atom(p_not_safe);
    a_init->param = f_c.args;
    a_init->insert(p_not_safe->init);
    dom_init.append(a_init);
  }

  // every action that adds f_c deletes "not-safe"
  for (index_type k = 0; k < dom_actions.length(); k++) {
    bool_vec sat(false, f_c.atoms.length());
    symbol_pair_vec eq;
    symbol_pair_vec neq;
    f_c.add_asserting_effects(dom_actions[k], p_not_safe, false, 0, 0, 0, false);
  }

  // every action that adds f_tr "not-safe" is true dels "ok"
  for (index_type k = 0; k < dom_actions.length(); k++) {
    bool_vec sat(false, f_c.atoms.length());
    symbol_pair_vec eq;
    symbol_pair_vec neq;
    f_tr.add_asserting_effects(dom_actions[k], p_ok, false, p_not_safe, 0, 0, false);
  }
  return new AtomicGoal(a_ok, false);
}

PDDL_Base::Goal* PDDL_Base::compile_constraint_2
(PDDL_Base::Goal* g, const Symbol* n)
{
  if (g->g_class == goal_always) {
    return compile_always_constraint((SimpleSequenceGoal*)g, n);
  }
  else if (g->g_class == goal_sometime) {
    return compile_sometime_constraint((SimpleSequenceGoal*)g, n);
  }
  else if (g->g_class == goal_at_most_once) {
    return compile_at_most_once_constraint((SimpleSequenceGoal*)g, n);
  }
  else if (g->g_class == goal_sometime_before) {
    return compile_sometime_before_constraint((TriggeredSequenceGoal*)g, n);
  }
  else if (g->g_class == goal_sometime_after) {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: compilation of goal ";
      g->print(std::cerr);
      std::cerr << " not implemented" << std::endl;
      if (!best_effort) exit(1);
    }
    return g;
  }
  else if (!g->is_state()) {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: can't compile goal ";
      g->print(std::cerr);
      std::cerr << std::endl;
      if (!best_effort) exit(1);
    }
  }
  return g;
}

void PDDL_Base::compile_constraints_2()
{
  for (index_type k = 0; k < dom_goals.length(); k++) {
    Symbol* c_name = gensym_i(sym_misc, "c", k, 0);
    dom_goals[k] = compile_constraint_2(dom_goals[k], c_name);
  }
  for (index_type k = 0; k < dom_preferences.length(); k++) {
    Symbol* c_name = (dom_preferences[k]->name ?
		      dom_preferences[k]->name :
		      gensym_i(sym_misc, "p", k, 0));
    dom_preferences[k]->goal =
      compile_constraint_2(dom_preferences[k]->goal, c_name);
  }
}

void PDDL_Base::metric_to_goal(NTYPE bound)
{
  if ((metric_type == metric_minimize) ||
      (metric_type == metric_maximize)) {
    Relation* r = new Relation((metric_type == metric_minimize ?
				rel_less : rel_greater),
			       metric,
			       new ConstantExpression(bound));
    dom_goals.append(new NumericGoal(r));
    metric_type = metric_none;
    metric = 0;
  }
  else if (write_warnings || !best_effort) {
    std::cerr << "warning: metric type " << metric_type
	      << " can not be converted to bounded goal"
	      << std::endl;
    if (!best_effort) exit(1);
  }
}

void PDDL_Base::select_preferences(const bool_vec& sel)
{
  for (index_type k = 0; k < dom_preferences.length(); k++) {
    if (sel[k]) {
      dom_goals.append(dom_preferences[k]->goal);
    }
    if (dom_preferences[k]->name && metric) {
      Expression* e = new ConstantExpression(sel[k] ? 0 : 1);
      metric =	metric->substitute_for_preference(dom_preferences[k]->name, e);
    }
  }
  dom_preferences.assign_value(0, 0);
}

void PDDL_Base::compile_set_conditions_and_effects
(ActionSymbol* act, variable_vec& i_param, variable_vec& d_param, index_type p)
{
  if (p < d_param.length()) {
    for (index_type k = 0; k < d_param[p]->sym_types.n_elements(); k++) {
      d_param[p]->value = d_param[p]->sym_types.get_element(k);
      /* early check of static preconditions */
      bool pass = true;
      for (index_type k = 0; (k < act->pos_pre.length()) && pass; k++)
	if (act->pos_pre[k]->is_static()) {
	  partial_value v = act->pos_pre[k]->partial_eval();
	  if (v == p_false) pass = false;
	}
      for (index_type k = 0; (k < act->neg_pre.length()) && pass; k++)
	if (act->neg_pre[k]->is_static()) {
	  partial_value v = act->neg_pre[k]->partial_eval();
	  if (v == p_true) pass = false;
	}
      for (index_type k = 0; (k < act->num_pre.length()) && pass; k++) {
	partial_value v = act->num_pre[k]->partial_eval();
	if (v == p_false) pass = false;
      }
      if (pass)
	compile_set_conditions_and_effects(act, i_param, d_param, p + 1);
    }
    d_param[p]->value = 0;
  }
  else {
    if (d_param.length() == 0) {
      for (index_type k = 0; k < act->pos_pre.length(); k++)
	if (act->pos_pre[k]->is_static()) {
	  partial_value v = act->pos_pre[k]->partial_eval();
	  if (v == p_false) return;
	}
      for (index_type k = 0; k < act->neg_pre.length(); k++)
	if (act->neg_pre[k]->is_static()) {
	  partial_value v = act->neg_pre[k]->partial_eval();
	  if (v == p_true) return;
	}
    }
    ActionSymbol* new_act =
      (ActionSymbol*)gensym(sym_action, act->print_name, 0);
    new_act->param = i_param;
    for (index_type k = 0; k < act->pos_pre.length(); k++)
      new_act->pos_pre.append(act->pos_pre[k]->instantiate_partially());
    for (index_type k = 0; k < act->neg_pre.length(); k++)
      new_act->neg_pre.append(act->neg_pre[k]->instantiate_partially());
    for (index_type k = 0; k < act->dis_pre.length(); k++)
      new_act->dis_pre.append(act->dis_pre[k]->instantiate_partially());
    for (index_type k = 0; k < act->set_pre.length(); k++) {
      if (act->set_pre[k]->context_is_static()) {
	act->set_pre[k]->compile(new_act->pos_pre, new_act->neg_pre, 0);
      }
      else {
	act->set_pre[k]->compile_non_static(new_act->dis_pre, 0);
      }
    }
    for (index_type k = 0; k < act->adds.length(); k++)
      new_act->adds.append(act->adds[k]->instantiate_partially());
    for (index_type k = 0; k < act->dels.length(); k++)
      new_act->dels.append(act->dels[k]->instantiate_partially());
    for (index_type k = 0; k < act->set_eff.length(); k++) {
      assert(act->set_eff[k]->context_is_static());
      act->set_pre[k]->compile(new_act->pos_pre, new_act->neg_pre, 0);
    }
    for (index_type k = 0; k < act->cond_eff.length(); k++)
      new_act->cond_eff.append(act->cond_eff[k]->instantiate_partially());
    for (index_type k = 0; k < act->locks.length(); k++)
      new_act->locks.append(act->locks[k]->instantiate_partially());
    for (index_type k = 0; k < act->enables.length(); k++)
      new_act->enables.append(act->enables[k]->instantiate_partially());
    // new_act->reqs = act->reqs;
    // new_act->incs = act->incs;
    // new_act->decs = act->decs;
    // new_act->fass = act->fass;
    // new_act->qc_incs = act->qc_incs;
    // new_act->qc_decs = act->qc_decs;
    // new_act->qc_fass = act->qc_fass;
    new_act->dmin = act->dmin;
    new_act->dmax = act->dmax;
    for (index_type k = 0; k < act->irr_ins.length(); k++)
      new_act->irr_ins.append(act->irr_ins[k]->instantiate_partially());
    for (index_type k = 0; k < act->refs.length(); k++)
      new_act->refs.append(act->refs[k]->instantiate_partially());
    if (act->part)
      new_act->part = act->part->instantiate_partially();
    new_act->assoc = act->assoc;
    new_act->post_process();
    dom_actions.append(new_act);
  }
}

void PDDL_Base::compile_set_conditions_and_effects(ActionSymbol* act)
{
  variable_vec dp(0, 0);
  variable_vec ip(0, 0);
  for (index_type k = 0; k < act->param.length(); k++) {
    bool found = false;
    for (index_type i = 0; (i < act->set_pre.length()) && !found; i++)
      if (act->set_pre[i]->occurs_in_context(act->param[k])) {
	dp.append(act->param[k]);
	found = true;
      }
    for (index_type i = 0; (i < act->set_eff.length()) && !found; i++)
      if (act->set_eff[i]->occurs_in_context(act->param[k])) {
	dp.append(act->param[k]);
	found = true;
      }
    if (!found)
      ip.append(act->param[k]);
  }
  act->clear_arguments();
  compile_set_conditions_and_effects(act, ip, dp, 0);
}

void PDDL_Base::compile_set_conditions_and_effects()
{
  lvector<ActionSymbol*> acts(dom_actions);
  dom_actions.clear();
  for (index_type k = 0; k < acts.length(); k++) {
    if ((acts[k]->set_pre.length() > 0) ||
	(acts[k]->set_eff.length() > 0))
      compile_set_conditions_and_effects(acts[k]);
    else
      dom_actions.append(acts[k]);
  }
}

PDDL_Base::Atom* PDDL_Base::make_binding_atom(VariableSymbol* v)
{
  FTerm* bt = v->binding;
  assert(bt != 0);
  HSPS::StringTable::Cell* c =
    (HSPS::StringTable::Cell*)tab.find(bt->fun->print_name);
  assert(c != 0);
  PredicateSymbol* p = (PredicateSymbol*)c->val;
  assert(p->sym_class == sym_predicate);
  assert(p->param.length() == (bt->fun->param.length() + 1));
  assert(bt->param.length() == bt->fun->param.length());
  Atom* a = new Atom(p);
  for (index_type i = 0; i < bt->param.length(); i++)
    a->param.append(bt->param[i]);
  a->param.append(v);
  return a;
}

void PDDL_Base::compile_object_functions
(ActionSymbol* act, Symbol* undefined_value)
{
  // std::cerr << "removing object functions from action "
  // << act->print_name << "..." << std::endl;
  variable_vec pre_omsk_vars(0, 0);
  for (index_type k = 0; k < act->pos_pre.length(); k++)
    act->pos_pre[k]->collect_bound_variables(pre_omsk_vars);
  for (index_type k = 0; k < act->neg_pre.length(); k++)
    act->neg_pre[k]->collect_bound_variables(pre_omsk_vars);
  for (index_type k = 0; k < act->adds.length(); k++) {
    if (act->adds[k]->pred == dom_assign_pred) {
      assert(act->adds[k]->param[0]->sym_class == sym_variable);
      assert(((VariableSymbol*)(act->adds[k]->param[0]))->binding != 0);
      ((VariableSymbol*)(act->adds[k]->param[0]))->binding->collect_bound_variables(pre_omsk_vars);
      if (act->adds[k]->param[1]->sym_class == sym_variable) {
	if (((VariableSymbol*)(act->adds[k]->param[1]))->binding != 0) {
	  pre_omsk_vars.append((VariableSymbol*)act->adds[k]->param[1]);
	  ((VariableSymbol*)(act->adds[k]->param[1]))->binding->collect_bound_variables(pre_omsk_vars);
	}
      }
    }
    else {
      act->adds[k]->collect_bound_variables(pre_omsk_vars);
    }
  }
  for (index_type k = 0; k < act->dels.length(); k++)
    act->dels[k]->collect_bound_variables(pre_omsk_vars);

  for (index_type k = 0; k < pre_omsk_vars.length(); k++) {
    act->pos_pre.append(make_binding_atom(pre_omsk_vars[k]));
  }

  index_set assigns;
  index_type n_adds = act->adds.length();
  for (index_type k = 0; k < n_adds; k++)
    if (act->adds[k]->pred == dom_assign_pred) {
      assigns.insert(k);
      assert(act->adds[k]->param[0]->sym_class == sym_variable);
      assert(((VariableSymbol*)(act->adds[k]->param[0]))->binding != 0);
      FTerm* bt = ((VariableSymbol*)(act->adds[k]->param[0]))->binding;
      HSPS::StringTable::Cell* c =
	(HSPS::StringTable::Cell*)tab.find(bt->fun->print_name);
      assert(c != 0);
      PredicateSymbol* p = (PredicateSymbol*)c->val;
      assert(p->sym_class == sym_predicate);
      assert(p->param.length() == (bt->fun->param.length() + 1));
      assert(bt->param.length() == bt->fun->param.length());
      VariableSymbol* m = 0;
      for (index_type i = 0; (i < pre_omsk_vars.length()) && (m == 0); i++)
	if (pre_omsk_vars[i]->binding->equals(*bt))
	  m = pre_omsk_vars[i];
      if (m != 0) {
	Atom* a_out = new Atom(p);
	for (index_type i = 0; i < bt->param.length(); i++)
	  a_out->param.append(bt->param[i]);
	a_out->param.append(m);
	act->dels.append(a_out);
      }
      else {
	VariableSymbol* v_pre =
	  (VariableSymbol*)gensym(sym_variable, "?fval",
				  p->param[p->param.length() - 1]->sym_types);
	v_pre->visible = false;
	Atom* a_pre = new Atom(p);
	for (index_type i = 0; i < bt->param.length(); i++)
	  a_pre->param.append(bt->param[i]);
	a_pre->param.append(v_pre);
	act->param.append(v_pre);
	act->pos_pre.append(a_pre);
	act->dels.append(a_pre);
      }
      Atom* a_in = new Atom(p);
      for (index_type i = 0; i < bt->param.length(); i++)
	a_in->param.append(bt->param[i]);
      if (act->adds[k]->param[1] == dom_undefined_obj)
	a_in->param.append(undefined_value);
      else
	a_in->param.append(act->adds[k]->param[1]);
      act->adds.append(a_in);
    }

  act->adds.remove(assigns);

  for (index_type k = 0; k < pre_omsk_vars.length(); k++) {
    act->param.append(pre_omsk_vars[k]);
    pre_omsk_vars[k]->visible = false;
    pre_omsk_vars[k]->binding = 0;
  }
}

void PDDL_Base::compile_object_functions_for_validator(ActionSymbol* act)
{
  // std::cerr << "removing object functions from action "
  // << act->print_name << "..." << std::endl;

  index_set rm_pos_pre;
  for (index_type k = 0; k < act->pos_pre.length(); k++) {
    variable_vec pre_omsk_vars(0, 0);
    act->pos_pre[k]->collect_bound_variables(pre_omsk_vars);
    if (pre_omsk_vars.length() > 0) {
      SetOf* q = new SetOf();
      for (index_type i = 0; i < pre_omsk_vars.length(); i++) {
	q->param.append(pre_omsk_vars[i]);
	q->pos_con.append(make_binding_atom(pre_omsk_vars[i]));
	pre_omsk_vars[i]->binding = 0;
      }
      q->pos_atoms.append(act->pos_pre[k]);
      act->dis_pre.append(q);
      rm_pos_pre.insert(k);
    }
  }
  act->pos_pre.remove(rm_pos_pre);

  index_set rm_neg_pre;
  for (index_type k = 0; k < act->neg_pre.length(); k++) {
    variable_vec pre_omsk_vars(0, 0);
    act->neg_pre[k]->collect_bound_variables(pre_omsk_vars);
    if (pre_omsk_vars.length() > 0) {
      SetOf* q = new SetOf();
      for (index_type i = 0; i < pre_omsk_vars.length(); i++) {
	q->param.append(pre_omsk_vars[i]);
	q->pos_con.append(make_binding_atom(pre_omsk_vars[i]));
	pre_omsk_vars[i]->binding = 0;
      }
      q->neg_atoms.append(act->neg_pre[k]);
      act->dis_pre.append(q);
      rm_neg_pre.insert(k);
    }
  }
  act->neg_pre.remove(rm_neg_pre);

  index_set rm_adds;
  for (index_type k = 0; k < act->adds.length(); k++) {
    // assignment effect
    if (act->adds[k]->pred == dom_assign_pred) {
      // collect omsk variables in lhs args and in rhs
      variable_vec omsk_vars(0, 0);
      assert(act->adds[k]->param[0]->sym_class == sym_variable);
      assert(((VariableSymbol*)(act->adds[k]->param[0]))->binding != 0);
      ((VariableSymbol*)(act->adds[k]->param[0]))->binding->collect_bound_variables(omsk_vars);
      if (act->adds[k]->param[1]->sym_class == sym_variable) {
	if (((VariableSymbol*)(act->adds[k]->param[1]))->binding != 0) {
	  omsk_vars.append((VariableSymbol*)act->adds[k]->param[1]);
	  ((VariableSymbol*)(act->adds[k]->param[1]))->binding->collect_bound_variables(omsk_vars);
	}
      }

      // find lhs bt
      FTerm* bt = ((VariableSymbol*)(act->adds[k]->param[0]))->binding;
      HSPS::StringTable::Cell* c =
	(HSPS::StringTable::Cell*)tab.find(bt->fun->print_name);
      assert(c != 0);
      PredicateSymbol* p = (PredicateSymbol*)c->val;
      assert(p->sym_class == sym_predicate);
      assert(p->param.length() == (bt->fun->param.length() + 1));
      assert(bt->param.length() == bt->fun->param.length());

      // s_out - set of atoms to delete: all instances with matching
      // values of omsk vars in lhs args and in rhs, all f-values that
      // do not equal rhs value (unless rhs value is undefined)
      SetOf* s_out = new SetOf();
      VariableSymbol* v_nb =
	new VariableSymbol(act->adds[k]->param[0]->print_name);
      v_nb->sym_types.assign_copy(act->adds[k]->param[0]->sym_types);
      s_out->param.append(v_nb);
      if (act->adds[k]->param[1] != dom_undefined_obj) {
	Atom* a_neq = new Atom(dom_eq_pred);
	a_neq->param.append(v_nb);
	a_neq->param.append(act->adds[k]->param[1]);
	s_out->neg_con.append(a_neq);
      }
      for (index_type i = 0; i < omsk_vars.length(); i++) {
	s_out->param.append(omsk_vars[i]);
	s_out->pos_con.append(make_binding_atom(omsk_vars[i]));
      }
      Atom* a_out = new Atom(p);
      for (index_type i = 0; i < bt->param.length(); i++)
	a_out->param.append(bt->param[i]);
      a_out->param.append(v_nb);
      s_out->neg_atoms.append(a_out);
      act->cond_eff.append(s_out);

      // if rhs not equal to undefined...
      if (act->adds[k]->param[1] != dom_undefined_obj) {
	if (omsk_vars.length() > 0) {
	  // s_in - cond. eff. over all omsk vars in lhs args and in rhs,
	  // when matching add corresponding instance
	  SetOf* s_in = new SetOf();
	  for (index_type i = 0; i < omsk_vars.length(); i++) {
	    s_in->param.append(omsk_vars[i]);
	    s_in->pos_con.append(make_binding_atom(omsk_vars[i]));
	  }
	  Atom* a_in = new Atom(p);
	  for (index_type i = 0; i < bt->param.length(); i++)
	    a_in->param.append(bt->param[i]);
	  a_in->param.append(act->adds[k]->param[1]);
	  s_in->pos_atoms.append(a_in);
	  act->cond_eff.append(s_in);
	}
	else {
	  // if no omsk vars in lhs args or in rhs, it's a simple add
	  Atom* a_in = new Atom(p);
	  for (index_type i = 0; i < bt->param.length(); i++)
	    a_in->param.append(bt->param[i]);
	  a_in->param.append(act->adds[k]->param[1]);
	  act->adds.append(a_in);
	}
      }
      for (index_type i = 0; i < omsk_vars.length(); i++)
	omsk_vars[i]->binding = 0;
      rm_adds.insert(k);
    }
    else {
      variable_vec omsk_vars(0, 0);
      act->adds[k]->collect_bound_variables(omsk_vars);
      if (omsk_vars.length() > 0) {
	SetOf* q = new SetOf();
	for (index_type i = 0; i < omsk_vars.length(); i++) {
	  q->param.append(omsk_vars[i]);
	  q->pos_con.append(make_binding_atom(omsk_vars[i]));
	  omsk_vars[i]->binding = 0;
	}
	q->pos_atoms.append(act->adds[k]);
	act->cond_eff.append(q);
	rm_adds.insert(k);
      }
    }

  }
  act->adds.remove(rm_adds);

  index_set rm_dels;
  for (index_type k = 0; k < act->dels.length(); k++) {
    variable_vec omsk_vars(0, 0);
    act->dels[k]->collect_bound_variables(omsk_vars);
    if (omsk_vars.length() > 0) {
      SetOf* q = new SetOf();
      for (index_type i = 0; i < omsk_vars.length(); i++) {
	q->param.append(omsk_vars[i]);
	q->pos_con.append(make_binding_atom(omsk_vars[i]));
	omsk_vars[i]->binding = 0;
      }
      q->neg_atoms.append(act->dels[k]);
      act->cond_eff.append(q);
      rm_dels.insert(k);
    }
  }
  act->dels.remove(rm_dels);
}

PDDL_Base::Goal* PDDL_Base::compile_object_functions(Goal* g)
{
  if ((g->g_class == goal_pos_atom) ||
      (g->g_class == goal_neg_atom)) {
    AtomicGoal* ag = (AtomicGoal*)g;
    // special case check:
    if (ag->atom->pred == dom_eq_pred) {
      assert(ag->atom->param.length() == 2);
      if ((ag->atom->param[0]->sym_class == sym_variable) &&
	  (ag->atom->param[1]->sym_class == sym_object)) {
	if (((VariableSymbol*)ag->atom->param[0])->binding != 0) {
	  VariableSymbol* v = (VariableSymbol*)ag->atom->param[0];
	  Symbol* co = ag->atom->param[1];
	  FTerm* bt = v->binding;
	  assert(bt != 0);
	  HSPS::StringTable::Cell* c =
	    (HSPS::StringTable::Cell*)tab.find(bt->fun->print_name);
	  assert(c != 0);
	  PredicateSymbol* p = (PredicateSymbol*)c->val;
	  assert(p->sym_class == sym_predicate);
	  assert(p->param.length() == (bt->fun->param.length() + 1));
	  assert(bt->param.length() == bt->fun->param.length());
	  Atom* a = new Atom(p);
	  for (index_type i = 0; i < bt->param.length(); i++)
	    a->param.append(bt->param[i]);
	  a->param.append(co);
	  ag->atom = a;
	  return g;
	}
      }
      if ((ag->atom->param[1]->sym_class == sym_variable) &&
	  (ag->atom->param[0]->sym_class == sym_object)) {
	if (((VariableSymbol*)ag->atom->param[1])->binding != 0) {
	  VariableSymbol* v = (VariableSymbol*)ag->atom->param[1];
	  Symbol* co = ag->atom->param[0];
	  FTerm* bt = v->binding;
	  assert(bt != 0);
	  HSPS::StringTable::Cell* c =
	    (HSPS::StringTable::Cell*)tab.find(bt->fun->print_name);
	  assert(c != 0);
	  PredicateSymbol* p = (PredicateSymbol*)c->val;
	  assert(p->sym_class == sym_predicate);
	  assert(p->param.length() == (bt->fun->param.length() + 1));
	  assert(bt->param.length() == bt->fun->param.length());
	  Atom* a = new Atom(p);
	  for (index_type i = 0; i < bt->param.length(); i++)
	    a->param.append(bt->param[i]);
	  a->param.append(co);
	  ag->atom = a;
	  return g;
	}
      }
    }
    variable_vec omsk_vars(0, 0);
    ag->atom->collect_bound_variables(omsk_vars);
    if (omsk_vars.length() > 0) {
      QuantifiedGoal* qg = new QuantifiedGoal(goal_exists);
      for (index_type k = 0; k < omsk_vars.length(); k++) {
	qg->param.append(omsk_vars[k]);
	FTerm* bt = omsk_vars[k]->binding;
	omsk_vars[k]->binding = 0;
	assert(bt != 0);
	HSPS::StringTable::Cell* c =
	  (HSPS::StringTable::Cell*)tab.find(bt->fun->print_name);
	assert(c != 0);
	PredicateSymbol* p = (PredicateSymbol*)c->val;
	assert(p->sym_class == sym_predicate);
	assert(p->param.length() == (bt->fun->param.length() + 1));
	assert(bt->param.length() == bt->fun->param.length());
	Atom* a = new Atom(p);
	for (index_type i = 0; i < bt->param.length(); i++)
	  a->param.append(bt->param[i]);
	a->param.append(omsk_vars[k]);
	qg->pos_con.append(a);
      }
      qg->goal = g;
      return qg;
    }
    else {
      return g;
    }
  }
  else if ((g->g_class == goal_conjunction) ||
	   (g->g_class == goal_disjunction)) {
    ConjunctiveGoal* cg = (ConjunctiveGoal*)this;
    for (index_type k = 0; k < cg->goals.length(); k++)
      cg->goals[k] = compile_object_functions(cg->goals[k]);
    return g;
  }
  else {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: compilation of object functions in goal ";
      print(std::cerr);
      std::cerr << " not implemented" << std::endl;
      if (!best_effort) exit(1);
    }
    return g;
  }
}

void PDDL_Base::compile_object_functions()
{
  Symbol* undefined_value = 0;
  TypeSymbol* undefined_type = 0;
  if (!compile_for_validator) {
    // create undefined-value of undefined-type
    undefined_type =
      (TypeSymbol*)gensym(sym_typename, "undefined-type", dom_top_type);
    dom_types.append(undefined_type);
    undefined_value =
      gensym(sym_object, "undefined-value", undefined_type);
    undefined_type->add_element(undefined_value);
    dom_constants.append(undefined_value);
  }
  lvector<PredicateSymbol*> fpred(0, 0);
  // for each object function...
  for (index_type k = 0; k < dom_object_functions.length(); k++) {
    ObjectFunctionSymbol* f = dom_object_functions[k];
    // ...create a predicate
    PredicateSymbol* p = new PredicateSymbol(f->print_name);
    p->param = f->param;
    VariableSymbol* vf =
      (VariableSymbol*)gensym(sym_variable, "?fval", f->sym_types);
    if (!compile_for_validator)
      vf->sym_types.append(undefined_type);
    p->param.append(vf);
    if (f->modded) {
      p->added = true;
      p->deleted = true;
      p->modded = true;
    }
    p->pos_pre = true; // safe to assume, probably doesn't matter anyway
    HSPS::StringTable::Cell* c =
      (HSPS::StringTable::Cell*)tab.find(f->print_name);
    if (c == 0) {
      std::cerr << "very bad error: object function " << f->print_name
		<< " declared but not found in string table!"
		<< std::endl;
      exit(255);
    }
    c->val = p;
    dom_predicates.append(p);
    fpred.append(p); // keep pointer to new predicates
    // ...create a DKEL invariant (set constraint), if fn isn't static
    if (!p->is_static()) {
      SetConstraint* f_inv = new SetConstraint();
      if (!compile_for_validator)
	f_inv->sc_type = sc_exactly;
      else
	f_inv->sc_type = sc_at_most;
      f_inv->sc_count = 1;
      f_inv->param = f->param;
      SetOf* s_inv = new SetOf();
      s_inv->param.append(vf);
      Atom* a_inv = new Atom(p);
      for (index_type i = 0; i < f_inv->param.length(); i++)
	a_inv->param.append(f_inv->param[i]);
      a_inv->param.append(vf);
      s_inv->pos_atoms.append(a_inv);
      f_inv->atom_sets.append(s_inv);
      dom_sc_invariants.append(f_inv);
    }
  }
  // convert object function inits to predicate inits
  for (index_type k = 0; k < dom_obj_init.length(); k++) {
    ObjectFunctionSymbol* f = dom_obj_init[k]->fun;
    HSPS::StringTable::Cell* c =
      (HSPS::StringTable::Cell*)tab.find(f->print_name);
    assert(c != 0);
    PredicateSymbol* p = (PredicateSymbol*)c->val;
    assert(p->sym_class == sym_predicate);
    assert(p->param.length() == (f->param.length() + 1));
    Atom* a = new Atom(p);
    for (index_type i = 0; i < dom_obj_init[k]->param.length(); i++)
      a->param.append(dom_obj_init[k]->param[i]);
    a->param.append(dom_obj_init[k]->val);
    a->at_time = dom_obj_init[k]->at_time;
    a->insert(p->init);
    dom_init.append(a);
  }
  if (!compile_for_validator)
    // need to explicitly initialise undefined object function instances
    for (index_type k = 0; k < fpred.length(); k++) {
      symbol_vec pattern(0, fpred[k]->param.length());
      pattern[pattern.length() - 1] = undefined_value;
      fpred[k]->initialise_missing(pattern, &dom_init);
    }
  // remove (well, more like forget) object functions & inits
  dom_object_functions.clear();
  dom_obj_init.clear();
  // compile object functions in action preconds and effects
  for (index_type k = 0; k < dom_actions.length(); k++) {
    if (compile_for_validator)
      compile_object_functions_for_validator(dom_actions[k]);
    else
      compile_object_functions(dom_actions[k], undefined_value);
  }
  // compile object functions in the goal
  for (index_type k = 0;k < dom_goals.length(); k++) {
    dom_goals[k] = compile_object_functions(dom_goals[k]);
  }
  // should have a check and warn for uncompiled parts here...
}

void PDDL_Base::Atom::build(Instance& ins, bool neg, index_type p)
{
  if (p < param.length()) {
    if (param[p]->sym_class == sym_variable) {
      for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
	((VariableSymbol*)param[p])->value =
	  param[p]->sym_types.get_element(k);
	build(ins, neg, p + 1);
      }
      ((VariableSymbol*)param[p])->value = 0;
    }
    else {
      build(ins, neg, p + 1);
    }
  }
  else {
    find_prop(ins, neg, true);
  }
}

void PDDL_Base::PredicateSymbol::instantiate(Instance& ins)
{
  Atom* a = new Atom(this, param, false);
  a->build(ins, false, 0);
  a->build(ins, true, 0);
  delete a;
}

void PDDL_Base::PredicateSymbol::initialise_missing
(const symbol_vec& p, atom_vec* created, index_type i)
{
  assert(p.length() == param.length());
  if (i < param.length()) {
    if (p[i] == 0) {
      for (index_type k = 0; k < param[i]->sym_types.n_elements(); k++) {
	param[i]->value = param[i]->sym_types.get_element(k);
	initialise_missing(p, created, i + 1);
      }
      param[i]->value = 0;
    }
    else {
      param[i]->value = 0;
      initialise_missing(p, created, i + 1);
    }
  }
  else {
    Atom a(this, param, false);
    partial_value v = a.partial_eval(&init, 0);
    if (v == p_false) { // no instance exists
      Atom* b = new Atom(this);
      for (index_type k = 0; k < param.length(); k++) {
	if (param[k]->value == 0) {
	  assert(p[k] != 0);
	  b->param.append(p[k]);
	}
	else {
	  b->param.append(param[k]->value);
	}
      }
      b->insert(init);
      if (created) {
	created->append(b);
      }
    }
  }
}

void* PDDL_Base::ActionSymbol::find_instance()
{
  ptr_table* r = &instances;
  for (index_type k = 0; (k < param.length()) && r; k++) {
    r = r->find_next(param[k]->value);
  }
  if (r) {
    if (r->val) return r->val;
  }
  return 0;
}

void PDDL_Base::ActionSymbol::build
(Instance& ins, index_type p, Expression* cost_exp)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);

      /* early check of static preconditions */
      bool pass = true;
      if (!create_all_actions) {
	for (index_type k = 0; (k < pos_pre.length()) && pass; k++)
	  if (pos_pre[k]->is_static()) {
	    partial_value v = pos_pre[k]->partial_eval();
	    if (v == p_false) pass = false;
	  }
	for (index_type k = 0; (k < neg_pre.length()) && pass; k++)
	  if (neg_pre[k]->is_static()) {
	    partial_value v = neg_pre[k]->partial_eval();
	    if (v == p_true) pass = false;
	  }
	for (index_type k = 0; (k < num_pre.length()) && pass; k++) {
	  partial_value v = num_pre[k]->partial_eval();
	  if (v == p_false) pass = false;
	}
	for (index_type k = 0; (k < set_pre.length()) && pass; k++)
	  if (set_pre[k]->is_static()) {
	    partial_value v = set_pre[k]->partial_eval();
	    if (v == p_false) pass = false;
	  }
 	for (index_type k = 0; (k < dis_pre.length()) && pass; k++)
 	  if (dis_pre[k]->is_static()) {
 	    partial_value v = dis_pre[k]->partial_eval(true);
 	    if (v == p_false) pass = false;
	  }
      }
      if (pass) build(ins, p + 1, cost_exp);
    }
    param[p]->value = 0;
  }
  else {
    if ((param.length() == 0) && !create_all_actions) {
      // check static preconditions (this is normally done when
      // instantiating parameters)
      for (index_type k = 0; k < pos_pre.length(); k++)
	if (pos_pre[k]->is_static()) {
	  partial_value v = pos_pre[k]->partial_eval();
	  if (v == p_false) return;
	}
      for (index_type k = 0; k < neg_pre.length(); k++)
	if (neg_pre[k]->is_static()) {
	  partial_value v = neg_pre[k]->partial_eval();
	  if (v == p_true) return;
	}
      for (index_type k = 0; k < set_pre.length(); k++)
	if (set_pre[k]->is_static()) {
	  partial_value v = set_pre[k]->partial_eval();
	  if (v == p_false) return;
	}
      for (index_type k = 0; k < dis_pre.length(); k++)
	if (dis_pre[k]->is_static()) {
	  partial_value v = dis_pre[k]->partial_eval(true);
	  if (v == p_false) return;
	}
    }

    /* check if this instance is excluded by any :irrelevant item */
    for (index_type k = 0; k < irr_ins.length(); k++)
      if (irr_ins[k]->included) {
	if (irr_ins[k]->context_is_static()) {
	  if (irr_ins[k]->match(param)) {
	    if (write_info) {
	      std::cerr << "DKEL: action instance " << print_name;
	      for (index_type i = 0; i < param.length(); i++)
		std::cerr << " " << param[i]->print_name << "\\"
			  << param[i]->value->print_name;
	      std::cerr << " excluded by" << std::endl;
	      irr_ins[k]->print(std::cerr);
	      std::cerr << std::endl;
	    }
	    return;
	  }
	}
	else if (write_warnings) {
	  std::cerr << "warning: ignoring :irrelevant ";
	  irr_ins[k]->entity->print(std::cerr);
	  std::cerr << " with non-static context" << std::endl;
	}
      }

    PDDL_Name* name = new PDDL_Name(this, param, false);
    index_type count_name = 0;

    index_set ac_pre;
    index_set ac_add;
    index_set_vec ac_dc(EMPTYSET, 0);
    index_set ac_del;
    index_set ac_lck;
    index_cost_vec ac_use(index_cost_pair(0, 0), 0);
    index_cost_vec ac_cons(index_cost_pair(0, 0), 0);
    NTYPE ac_dmin = 1;
    NTYPE ac_dmax = 1;
    NTYPE ac_dur = 1;
    NTYPE ac_cost = 1;

    // instantiate fixed preconditions
    for (index_type k = 0; k < pos_pre.length(); k++)
      if (!pos_pre[k]->is_static() ||
	  (create_all_actions && !pos_pre[k]->pred->is_equality())) {
	Instance::Atom* pp = pos_pre[k]->find_prop(ins, false, true);
	ac_pre.insert(pp->index);
      }
    for (index_type k = 0; k < neg_pre.length(); k++)
      if (!neg_pre[k]->is_static() ||
	  (create_all_actions && !neg_pre[k]->pred->is_equality())) {
	Instance::Atom* np = neg_pre[k]->find_prop(ins, true, true);
	ac_pre.insert(np->index);
      }

    for (index_type k = 0; k < set_pre.length(); k++) {
      if (!set_pre[k]->is_static() || create_all_actions) {
	index_set s;
	set_pre[k]->instantiate_as_set(ins, s);
	ac_pre.insert(s);
      }
    }

    // instantiate fixed effects
    for (index_type k = 0; k < adds.length(); k++)  {
      Instance::Atom* pp = adds[k]->find_prop(ins, false, true);
      ac_add.insert(pp->index);
      if (adds[k]->pred->neg_pre) {
	Instance::Atom* np = adds[k]->find_prop(ins, true, true);
	ac_del.insert(np->index);
      }
    }
    for (index_type k = 0; k < dels.length(); k++)  {
      Instance::Atom* pp = dels[k]->find_prop(ins, false, true);
      ac_del.insert(pp->index);
      if (dels[k]->pred->neg_pre) {
	Instance::Atom* np = dels[k]->find_prop(ins, true, true);
	ac_add.insert(np->index);
      }
    }

    for (index_type k = 0; k < set_eff.length(); k++) {
      index_set s_add;
      index_set s_del;
      set_eff[k]->instantiate_as_effect(ins, s_add, s_del);
      ac_add.insert(s_add);
      ac_del.insert(s_del);
    }

    // instantiate locks
    for (index_type k = 0; k < locks.length(); k++) {
      Instance::Atom* lp = locks[k]->find_prop(ins, false, true);
      ac_lck.insert(lp->index);
    }

    // warn about ignored parts
    if ((!enables.empty() || !incs.empty() || !fass.empty() ||
	 !qc_incs.empty() || !qc_decs.empty() || !qc_fass.empty()) &&
	(write_warnings || !best_effort)) {
      std::cerr << "warning: effects of action " << print_name
		<< " ignored in instantiation:" << std::endl;
      for (index_type k = 0; k < enables.length(); k++) {
	std::cerr << " - temporary add atom ";
	enables[k]->print(std::cerr);
	std::cerr << std::endl;
      }
      for (index_type k = 0; k < incs.length(); k++) {
	std::cerr << " - fluent increase ";
	incs[k]->print(std::cerr);
	std::cerr << std::endl;
      }
      for (index_type k = 0; k < fass.length(); k++) {
	std::cerr << " - fluent assignment ";
	fass[k]->print(std::cerr);
	std::cerr << std::endl;
      }
      for (index_type k = 0; k < qc_incs.length(); k++) {
	std::cerr << " - quantified/conditional fluent increase ";
	qc_incs[k]->print(std::cerr);
	std::cerr << std::endl;
      }
      for (index_type k = 0; k < qc_decs.length(); k++) {
	std::cerr << " - quantified/conditional fluent decrease ";
	qc_decs[k]->print(std::cerr);
	std::cerr << std::endl;
      }
      for (index_type k = 0; k < qc_fass.length(); k++) {
	std::cerr << " - quantified/conditional fluent assignment ";
	qc_fass[k]->print(std::cerr);
	std::cerr << std::endl;
      }
      if (!best_effort) exit(1);
    }

    // instantiate resource effects
    for (index_type k = 0; k < reqs.length(); k++) {
      Instance::Resource* rc = reqs[k]->find_resource(ins);
      ac_use.append(index_cost_pair(rc->index, reqs[k]->val->eval_static()));
    }
    for (index_type k = 0; k < decs.length(); k++) {
      Instance::Resource* rc = decs[k]->find_resource(ins);
      ac_cons.append(index_cost_pair(rc->index, decs[k]->val->eval_static()));
    }

    // calculate duration and cost
    if (dmin) {
      ac_dmin = dmin->eval_static();
      if ((ac_dmin <= 0) && PDDL_Base::write_warnings) {
	std::cerr << "warning: action instance " << name
		  << " has (min) duration " << ac_dmin << " <= 0"
		  << std::endl;
      }
    }
    if (dmax) {
      ac_dmax = dmax->eval_static();
      if ((ac_dmax <= 0) && PDDL_Base::write_warnings) {
	std::cerr << "warning: action instance " << name
		  << " has (max) duration " << ac_dmax << " <= 0"
		  << std::endl;
      }
    }
    if (dmax) {
      ac_dur = ac_dmax;
    }
    else if (dmin) {
      ac_dur = ac_dmin;
    }

    if (cost_exp) {
      ac_cost = cost_exp->eval_delta(incs, decs);
      if ((ac_cost <= 0) && PDDL_Base::write_warnings) {
	std::cerr << "warning: action instance " << name
		  << " has cost " << ac_cost << " <= 0" << std::endl;
      }
    }

    // separate out non-static disjunctive preconditions
    atom_set_vec ns_dis_pre(0, 0);
    for (index_type k = 0; k < dis_pre.length(); k++)
      if (!dis_pre[k]->is_static())
	ns_dis_pre.append(dis_pre[k]);

    // if we have conditional effects to compile
    if ((cond_eff.length() > 0) &&
	compile_away_conditional_effects) {
      // if we also have disjunctive precondition to compile...
      if ((ns_dis_pre.length() > 0) &&
	  compile_away_disjunctive_preconditions) {
	ac_dc.set_length(ns_dis_pre.length());
	for (index_type k = 0; k < ns_dis_pre.length(); k++)
	  ns_dis_pre[k]->instantiate_as_set(ins, ac_dc[k]);
      }
      rule_set pce;
      rule_set nce;
      for (index_type k = 0; k < cond_eff.length(); k++) {
	cond_eff[k]->instantiate_conditional(ins, pce, nce);
      }
      SubsetEnumerator se(pce.length() + nce.length());
      bool more = se.first();
      if (write_info) {
	std::cerr << "info: compiling " << pce.length() + nce.length()
		  << " conditional effects in action "
		  << print_name << "..." << std::endl;
	if (write_trace) {
	  std::cerr << "debug: instantiated conditional effects of action "
		    << print_name << " are:" << std::endl;
	  for (index_type k = 0; k < pce.length(); k++)
	    std::cerr << "add: " << pce[k] << std::endl;
	  for (index_type k = 0; k < nce.length(); k++)
	    std::cerr << "del: " << nce[k] << std::endl;
	}
      }
      while (more) {
	build_actions_with_dc_and_ce(ins, name, count_name, ac_pre, ac_add,
				     ac_del, ac_lck, ac_use, ac_cons, ac_dmin,
				     ac_dmax, ac_dur, ac_cost, ac_dc, pce, nce,
				     se.current_set());
	more = se.next();
      }
    }
    // else, if we have disjunctive preconditions to compile
    else if ((ns_dis_pre.length() > 0) &&
	     compile_away_disjunctive_preconditions) {
      if ((cond_eff.length() > 0) && (write_warnings || !best_effort)) {
	std::cerr << "warning: conditional effects of action " << print_name
		  << " ignored in instantiation" << std::endl;
	if (!best_effort) exit(1);
      }
      ac_dc.set_length(dis_pre.length());
      for (index_type k = 0; k < ns_dis_pre.length(); k++)
	ns_dis_pre[k]->instantiate_as_set(ins, ac_dc[k]);
      build_actions_with_dc(ins, name, count_name, ac_pre, ac_add, ac_del,
			    ac_lck, ac_use, ac_cons, ac_dmin, ac_dmax, ac_dur,
			    ac_cost, ac_dc);
    }
    else {
      if ((cond_eff.length() > 0) && (write_warnings || !best_effort)) {
	std::cerr << "warning: conditional effects of action " << print_name
		  << " ignored in instantiation" << std::endl;
	if (!best_effort) exit(1);
      }
      if ((ns_dis_pre.length() > 0) && (write_warnings || !best_effort)) {
	std::cerr << "warning: disjunctive preconditions of action "
		  << print_name << " ignored in instantiation" << std::endl;
	if (!best_effort) exit(1);
      }
      build_action(ins, name, count_name, ac_pre, ac_add, ac_del, ac_lck,
		   ac_use, ac_cons, ac_dmin, ac_dmax, ac_dur, ac_cost);
    }
  }
}

Instance::Action& PDDL_Base::ActionSymbol::build_action
(Instance& ins, PDDL_Name* name, index_type& count,
 const index_set& pre, const index_set& add, const index_set& del,
 const index_set& lck, const index_cost_vec& r_use,
 const index_cost_vec& r_cons, NTYPE dmin, NTYPE dmax, NTYPE d, NTYPE c)
{
  /* find action in instance tree */
  ptr_table* r = &instances;
  for (index_type k = 0; k < name->argc(); k++) {
    r = r->insert_next(name->args()[k]);
  }
  if (r->val) {
    if (write_warnings && (count >= 1)) {
      std::cerr << "warning: multiple instances of (" << print_name;
      for (index_type i = 0; i < param.length(); i++)
	std::cerr << " " << param[i]->print_name << "\\"
		  << param[i]->value->print_name;
      std::cerr << ")" << std::endl;
    }
    if (number_multiple_action_instances) {
      if (count == 1) {
	((Instance::Action*)r->val)->name = new Numbered_PDDL_Name(name, 0);
      }
      name = new Numbered_PDDL_Name(name, count++);
    }
  }

  /* create the action */
  Instance::Action& act = ins.new_action(name);
  act.src = new ptr_pair(this, r);

  /* if this is the first time this instance is created, insert it into the
     instance tree */
  if (r->val == 0) {
    r->val = new Instance::action_ref(ins.actions, act.index);
    count = 1;
  }

  /* fill in references to the action */
  for (index_type k = 0; k < refs.length(); k++) {
    if (refs[k]->match(param)) {
      if (write_info) {
	std::cerr << "info: action reference ";
	refs[k]->print(std::cerr);
	std::cerr << " matched to "
		  << act.index << "." << act.name
		  << std::endl;
      }
      if (write_warnings && !refs[k]->index.empty()) {
	std::cerr << "warning: action reference ";
	refs[k]->print(std::cerr);
	std::cerr << " has multiple matches" << std::endl;
      }
      refs[k]->index.insert(act.index);
    }
  }

  /* add action to partition, if defined */
  if (part) {
    index_set* s = part->find();
    s->insert(act.index);
  }

  act.pre = pre;
  act.add = add;
  act.del = del;
  act.lck = lck;
  for (index_type k = 0; k < r_use.length(); k++) {
    act.use.inc_length_to(r_use[k].first + 1, 0);
    act.use[r_use[k].first] += r_use[k].second;
  }
  for (index_type k = 0; k < r_cons.length(); k++) {
    act.cons.inc_length_to(r_cons[k].first + 1, 0);
    act.cons[r_cons[k].first] += r_cons[k].second;
  }
  act.dmin = dmin;
  act.dmax = dmax;
  act.dur = d;
  act.cost = c;

  /* copy associated info */
  act.assoc = assoc;

  return act;
}

void PDDL_Base::ActionSymbol::build_actions_with_dc
(Instance& ins, PDDL_Name* name, index_type& count,
 const index_set& pre, const index_set& add, const index_set& del,
 const index_set& lck, const index_cost_vec& r_use,
 const index_cost_vec& r_cons, NTYPE dmin, NTYPE dmax, NTYPE d, NTYPE c,
 const index_set_vec& dc, index_vec& s, index_type p)
{
  if (p < dc.length()) {
    if ((dc[p].first_common_element(s) != no_such_index) ||
	(dc[p].first_common_element(pre) != no_such_index)) {
      build_actions_with_dc(ins, name, count, pre, add, del, lck, r_use,
			    r_cons, dmin, dmax, d, c, dc, s, p + 1);
    }
    else {
      for (index_type k = 0; k < dc[p].length(); k++) {
	bool consistent = true;
	if (check_precondition_consistency) {
	  index_type not_dk = ins.atoms[dc[p][k]].neg;
	  if (not_dk != no_such_index) {
	    if (s.first(not_dk) != no_such_index) consistent = false;
	    if (pre.contains(not_dk)) consistent = false;
	  }
	}
	if (consistent) {
	  s.append(dc[p][k]);
	  build_actions_with_dc(ins, name, count, pre, add, del, lck, r_use,
				r_cons, dmin, dmax, d, c, dc, s, p + 1);
	  s.dec_length();
	}
      }
    }
  }
  else {
    Instance::Action& act =
      build_action(ins, name, count, pre, add, del, lck, r_use, r_cons,
		   dmin, dmax, d, c);
    for (index_type k = 0; k < s.length(); k++)
      act.pre.insert(s[k]);
  }
}

void PDDL_Base::ActionSymbol::build_actions_with_dc
(Instance& ins, PDDL_Name* name, index_type& count,
 const index_set& pre, const index_set& add, const index_set& del,
 const index_set& lck, const index_cost_vec& r_use,
 const index_cost_vec& r_cons, NTYPE dmin, NTYPE dmax, NTYPE d, NTYPE c,
 const index_set_vec& dc)
{
  index_vec s(no_such_index, 0);
  build_actions_with_dc(ins, name, count, pre, add, del, lck, r_use, r_cons,
			dmin, dmax, d, c, dc, s, 0);
}

void PDDL_Base::ActionSymbol::build_actions_with_dc_and_ce
(Instance& ins, PDDL_Name* name, index_type& count,
 const index_set& pre, const index_set& add, const index_set& del,
 const index_set& lck, const index_cost_vec& r_use,
 const index_cost_vec& r_cons, NTYPE dmin, NTYPE dmax, NTYPE d, NTYPE c,
 const index_set_vec& dc, const rule_set& pce, const rule_set& nce,
 const bool_vec& ece)
{
  index_set x_pre(pre);
  index_set x_add(add);
  index_set x_del(del);
  index_set_vec x_dc(dc);

  for (index_type k = 0; k < pce.length(); k++) {
    if (ece[k]) {
      x_pre.insert(pce[k].antecedent);
      x_add.insert(pce[k].consequent);
      index_type not_c = ins.atoms[pce[k].consequent].neg;
      if (not_c != no_such_index)
	x_del.insert(not_c);
    }
    else {
      if (pce[k].antecedent.length() == 1) {
	index_type not_a = ins.atoms[pce[k].antecedent[0]].neg;
	if (not_a == no_such_index) {
	  std::cerr << "error (compiling conditional effects): negation of "
		    << ins.atoms[pce[k].antecedent[0]].name
		    << " not defined" << std::endl;
	  exit(255);
	}
	x_pre.insert(not_a);
      }
      else {
	index_set all_not_a;
	for (index_type i = 0; i < pce[k].antecedent.length(); i++) {
	  index_type not_a = ins.atoms[pce[k].antecedent[i]].neg;
	  if (not_a == no_such_index) {
	    std::cerr << "error (compiling conditional effects): negation of "
		      << ins.atoms[pce[k].antecedent[i]].name
		      << " not defined" << std::endl;
	    exit(255);
	  }
	  all_not_a.insert(not_a);
	}
	x_dc.append(all_not_a);
      }
    }
  }

  for (index_type k = 0; k < nce.length(); k++) {
    if (ece[pce.length() + k]) {
      x_pre.insert(nce[k].antecedent);
      x_del.insert(nce[k].consequent);
      index_type not_c = ins.atoms[nce[k].consequent].neg;
      if (not_c != no_such_index)
	x_add.insert(not_c);
    }
    else {
      if (nce[k].antecedent.length() == 1) {
	index_type not_a = ins.atoms[nce[k].antecedent[0]].neg;
	if (not_a == no_such_index) {
	  std::cerr << "error (compiling conditional effects): negation of "
		    << ins.atoms[nce[k].antecedent[0]].name
		    << " not defined" << std::endl;
	  exit(255);
	}
	x_pre.insert(not_a);
      }
      else {
	index_set all_not_a;
	for (index_type i = 0; i < nce[k].antecedent.length(); i++) {
	  index_type not_a = ins.atoms[nce[k].antecedent[i]].neg;
	  if (not_a == no_such_index) {
	    std::cerr << "error (compiling conditional effects): negation of "
		      << ins.atoms[nce[k].antecedent[i]].name
		      << " not defined" << std::endl;
	    exit(255);
	  }
	  all_not_a.insert(not_a);
	}
	x_dc.append(all_not_a);
      }
    }
  }

  index_vec s(no_such_index, 0);
  build_actions_with_dc(ins, name, count, x_pre, x_add, x_del, lck, r_use,
			r_cons, dmin, dmax, d, c, x_dc, s, 0);
}

index_type PDDL_Base::ActionSymbol::param_index(VariableSymbol* p)
{
  return param.first(p);
  // for (index_type k = 0; k < param.length(); k++)
  //   if (param[k] == p) return k;
  // return no_such_index;
}

void PDDL_Base::ActionSymbol::get_param_inequalities(symbol_pair_vec& neq)
{
  neq.clear();
  for (index_type k = 0; k < neg_pre.length(); k++)
    if (neg_pre[k]->pred->is_equality()) {
      assert(neg_pre[k]->param.length() == 2);
      neq.append(symbol_pair(neg_pre[k]->param[0], neg_pre[k]->param[1]));
    }
}

void PDDL_Base::ActionSymbol::set_arguments(const symbol_vec& args)
{
  if (args.length() != param.length()) {
    std::cerr << "error: can't set arguments of (" << print_name;
    for (index_type i = 0; i < param.length(); i++)
      std::cerr << " " << param[i]->print_name;
    std::cerr << ") with ";
    for (index_type i = 0; i < args.length(); i++) {
      std::cerr << args[i]->print_name;
      if (i + 1 < args.length()) std::cerr << ";";
    }
    std::cerr << " -- wrong number!" << std::endl;
    exit(255);
  }
  for (index_type k = 0; k < param.length(); k++) param[k]->value = args[k];
}

void PDDL_Base::ActionSymbol::set_arguments(const ptr_table::key_vec& args)
{
  if (args.length() != param.length()) {
    std::cerr << "error: can't set arguments of (" << print_name;
    for (index_type i = 0; i < param.length(); i++)
      std::cerr << " " << param[i]->print_name;
    std::cerr << ") with " << args << " -- wrong number!"
	      << std::endl;
    exit(255);
  }
  for (index_type k = 0; k < param.length(); k++) {
    param[k]->value = (PDDL_Base::Symbol*)args[k];
  }
}

void PDDL_Base::ActionSymbol::clear_arguments()
{
  for (index_type k = 0; k < param.length(); k++)
    param[k]->value = 0;
  for (index_type k = 0; k < set_pre.length(); k++)
    set_pre[k]->clear_arguments();
  for (index_type k = 0; k < set_eff.length(); k++)
    set_eff[k]->clear_arguments();
}

bool PDDL_Base::ActionSymbol::is_abstract()
{
  return (exps.length() > 0);
}

void PDDL_Base::ActionSymbol::instantiate(Instance& ins, Expression* cost_exp)
{
  assert(!is_abstract());
  for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
  build(ins, 0, cost_exp);
}


void PDDL_Base::ActionSymbol::build_abstract(HTNInstance& ins, index_type p)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);

      /* early check of static preconditions */
      bool pass = true;
      for (index_type k = 0; (k < pos_pre.length()) && pass; k++)
	if (pos_pre[k]->is_static()) {
	  partial_value v = pos_pre[k]->partial_eval();
	  if (v == p_false) pass = false;
	}
      for (index_type k = 0; (k < neg_pre.length()) && pass; k++)
	if (neg_pre[k]->is_static()) {
	  partial_value v = neg_pre[k]->partial_eval();
	  if (v == p_true) pass = false;
	}
      if (pass) build_abstract(ins, p+1);
    }
    param[p]->value = 0;
  }
  else {

    /* final check of static preconditions */
#if 0
    for (index_type k = 0; k < pos_pre.length(); k++)
      if (pos_pre[k]->is_static())
	if (!pos_pre[k]->initial_value()) return;
    for (index_type k = 0; k < neg_pre.length(); k++)
      if (neg_pre[k]->is_static())
	if (neg_pre[k]->initial_value()) return;
#endif

    /* check if this instance is excluded by any :irrelevant item */
    for (index_type k = 0; k < irr_ins.length(); k++)
      if (irr_ins[k]->included) {
	if (irr_ins[k]->context_is_static()) {
	  if (irr_ins[k]->match(param)) {
	    if (write_info) {
	      std::cerr << "DKEL: action instance " << print_name;
	      for (index_type i = 0; i < param.length(); i++)
		std::cerr << " " << param[i]->print_name << "\\"
			  << param[i]->value->print_name;
	      std::cerr << " excluded by" << std::endl;
	      irr_ins[k]->print(std::cerr);
	      std::cerr << std::endl;
	    }
	    return;
	  }
	}
	else if (write_warnings) {
	  std::cerr << "warning: ignoring :irrelevant ";
	  irr_ins[k]->entity->print(std::cerr);
	  std::cerr << " with non-static context" << std::endl;
	}
      }

    PDDL_Name* n = new PDDL_Name(this, param, false);
    HTNInstance::Task& t = ins.new_task(n);

    ptr_table* r = &instances;
    for (index_type k = 0; k < n->argc(); k++) {
      r = r->insert_next(n->args()[k]);
    }
    if (r->val) {
      std::cerr << "error: duplicate instances of (" << print_name;
      for (index_type i = 0; i < param.length(); i++)
	std::cerr << " " << param[i]->print_name << "\\"
		  << param[i]->value->print_name;
	std::cerr << std::endl;
	exit(255);
    }
    r->val = &t;
  }
}

void PDDL_Base::ActionSymbol::instantiate_abstract_1(HTNInstance& ins)
{
  assert(is_abstract());
  for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
  build_abstract(ins, 0);
}

void PDDL_Base::ActionSymbol::instantiate_abstract_2(HTNInstance& ins)
{
  ptr_table::value_vec* v = instances.values();
  if (v->length() == 0) {
    if (write_warnings) {
      std::cerr << "warning: no instances of action " << print_name
		<< std::endl;
    }
  }
  std::cerr << "creating expansions for " << v->length()
	    << " instances of " << print_name << "..." << std::endl;
  for (index_type k = 0; k < v->length(); k++) {
    HTNInstance::Task* m = (HTNInstance::Task*)((*v)[k]);
    PDDL_Name* n = (PDDL_Name*)(m->name);
    set_arguments(n->args());
    for (index_type k = 0; k < exps.length(); k++) {
      exps[k]->instantiate(ins, *m);
    }
  }
  delete v;
}

index_set* PDDL_Base::SetName::find()
{
  ptr_table* r = &(sym->set_table);
  for (index_type k = 0; k < param.length(); k++) {
    if (param[k]->sym_class == sym_variable) {
      if (((VariableSymbol*)param[k])->value == 0) {
	std::cerr << "error: unbound variable "
		  << param[k]->print_name << " in ";
	print(std::cerr);
	std::cerr << " - uncompiled object function?"
		  << std::endl;
	exit(255);
      }
      r = r->insert_next(((VariableSymbol*)param[k])->value);
    }
    else {
      r = r->insert_next(param[k]);
    }
  }
  if (!r->val) {
    PDDL_Name* s_name = new PDDL_Name(sym);
    for (index_type k = 0; k < param.length(); k++) {
      if (param[k]->sym_class == sym_variable) {
	s_name->add(((VariableSymbol*)param[k])->value);
      }
      else {
	s_name->add(param[k]);
      }
    }
    index_set* s = new index_set;
    r->val = s;
    sym->sets.append(s);
    sym->names.append(s_name);
  }
  return (index_set*)(r->val);
}

PDDL_Base::SetName* PDDL_Base::SetName::instantiate_partially()
{
  SetName* n = new SetName(sym);
  fill_in_args(n);
  return n;
}

void PDDL_Base::SequentialTaskNet::build
(HTNInstance& ins, HTNInstance::Task& tk, index_type p)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);
      if (Context::partial_eval() != p_false) {
	build(ins, tk, p+1);
      }
    }
    param[p]->value = 0;
  }
  else if (is_true()) {
    HTNInstance::Method met;
    met.pre.clear();
    met.steps.set_length(0);
    bool ok = true;
    for (index_type k = 0; k < tasks.length(); k++) {
      if (tasks[k]->name->sym_class != sym_action) {
	std::cerr << "error: ";
	tasks[k]->name->print(std::cerr);
	std::cerr << " of ";
	tasks[k]->print(std::cerr);
	std::cerr << " is not an action symbol" << std::endl;
	exit(255);
      }
      ActionSymbol* act = (ActionSymbol*)(tasks[k]->name);
      void* ains = tasks[k]->find_action();
      if (!ains) {
	if (write_warnings) {
	  std::cerr << "warning: no match for ";
	  tasks[k]->print(std::cerr);
	  std::cerr << " in ";
	  print(std::cerr);
	  if (param.length() > 0) {
	    std::cerr << " with ";
	    for (index_type i = 0; i < param.length(); i++)
	      std::cerr << " " << param[i]->print_name << "\\"
			<< param[i]->value->print_name;
	  }
	  std::cerr << std::endl;
	  if (abs_act) {
	    std::cerr << " in instance of " << abs_act->print_name;
	    if (abs_act->param.length() > 0) {
	      std::cerr << " with";
	      for (index_type i = 0; i < abs_act->param.length(); i++)
		std::cerr << " " << abs_act->param[i]->print_name << "\\"
			  << abs_act->param[i]->value->print_name;
	    }
	    std::cerr << std::endl;
	  }
	}
	ok = false;
      }
      else {
	if (act->is_abstract()) {
	  HTNInstance::Task* t = (HTNInstance::Task*)ains;
	  met.steps.append(HTNInstance::step(true, t->index));
	}
	else {
	  Instance::Action* a = (Instance::Action*)ains;
	  met.steps.append(HTNInstance::step(false, a->index));
	}
      }
    }
    if (ok) {
      /* fill in preconditions */
      for (index_type k = 0; k < pos_con.length(); k++) {
	if (!pos_con[k]->is_static()) {
	  Instance::Atom* pp = pos_con[k]->find_prop(ins, false, true);
	  met.pre.insert(pp->index);
	}
      }
      for (index_type k = 0; k < neg_con.length(); k++) {
	if (!neg_con[k]->is_static()) {
	  Instance::Atom* np = neg_con[k]->find_prop(ins, true, true);
	  met.pre.insert(np->index);
	}
      }
//       if (write_info) {
// 	std::cerr << "info: expansion ";
// 	ins.write_atom_set(std::cerr, met.pre);
// 	std::cerr << " / ";
// 	ins.write_step_sequence(std::cerr, met.steps);
// 	std::cerr << " added to method " << tk.name
// 		  << std::endl;
//       }
      tk.exp.insert(met);
    }
  }
}

void PDDL_Base::SequentialTaskNet::instantiate
(HTNInstance& ins, HTNInstance::Task& tk)
{
//   if (!context_is_static()) {
//     if (write_warnings) {
//       std::cerr << "warning: non-static context in ";
//       print(std::cerr);
//       std::cerr << " ignored" << std::endl;
//     }
//   }
  for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
  build(ins, tk, 0);
}

bool PDDL_Base::Reference::match(symbol_vec& args)
{
  if (args.length() != param.length()) {
    std::cerr << "error: wrong number of arguments (" << args.length()
	      << ", should be " << param.length() << ") in match"
	      << std::endl;
    return false;
  }
  for (index_type k = 0; k < param.length(); k++) {
    if (param[k]->sym_class == sym_variable) {
      if (((VariableSymbol*)param[k])->value) {
	if (args[k]->sym_class == sym_variable) {
	  if (((VariableSymbol*)args[k])->value !=
	      ((VariableSymbol*)param[k])->value)
	    return false;
	}
	else {
	  if (args[k] != ((VariableSymbol*)param[k])->value) return false;
	}
      }
      else {
	if (args[k]->sym_class == sym_variable) {
	  ((VariableSymbol*)param[k])->value =
	    ((VariableSymbol*)args[k])->value;
	}
	else {
	  ((VariableSymbol*)param[k])->value = args[k];
	}
      }
    }
    else if (param[k]->sym_class == sym_object) {
      if (args[k]->sym_class == sym_variable) {
	if (((VariableSymbol*)args[k])->value != param[k]) return false;
      }
      else {
	if (args[k] != param[k]) return false;
      }
    }
    else {
      std::cerr << "program error: arg " << k + 1
		<< " has bad sym_class in match ";
      print(std::cerr);
      std::cerr << std::endl;
      exit(255);
    }
  }
  return true;
}

bool PDDL_Base::Reference::match(variable_vec& args)
{
  if (args.length() != param.length()) {
    std::cerr << "error: wrong number of arguments (" << args.length()
	      << ", should be " << param.length() << ") in match"
	      << std::endl;
    return false;
  }
  for (index_type k = 0; k < param.length(); k++) {
    if (param[k]->sym_class == sym_variable) {
      if (((VariableSymbol*)param[k])->value) {
	if (args[k]->value != ((VariableSymbol*)param[k])->value) return false;
      }
      else {
	((VariableSymbol*)param[k])->value = args[k]->value;
      }
    }
    else if (param[k]->sym_class == sym_object) {
      if (args[k]->value != param[k]) return false;
    }
    else {
      std::cerr << "program error: arg " << k + 1
		<< " has bad sym_class in match ";
      print(std::cerr);
      std::cerr << std::endl;
      exit(255);
    }
  }
  return true;
}

void* PDDL_Base::Reference::find_action()
{
  if (name->sym_class != sym_action) {
    std::cerr << "error: ";
    name->print(std::cerr);
    std::cerr << " of ";
    print(std::cerr);
    std::cerr << " is not an action symbol" << std::endl;
    exit(255);
  }
  ActionSymbol* act = (ActionSymbol*)name;

  ptr_table* r = &(act->instances);
  for (index_type k = 0; (k < param.length()) && r; k++) {
    if (param[k]->sym_class == sym_variable)
      r = r->find_next(((VariableSymbol*)param[k])->value);
    else
      r = r->find_next(param[k]);
  }
  if (r) {
    return r->val;
  }
  else {
    return 0;
  }
}

void PDDL_Base::Reference::find(const name_vec& names, index_set& ind)
{
  ind.clear();
  Name* n = 0;
  if (has_args) {
    PDDL_Name* pn = new PDDL_Name(name, neg);
    for (index_type k = 0; k < param.length(); k++) {
      if (param[k]->sym_class == sym_variable) {
	VariableSymbol* v = (VariableSymbol*)param[k];
	if (v->value == 0) {
	  if (write_warnings) {
	    std::cerr << "warning: ";
	    v->print(std::cerr);
	    std::cerr << " in ";
	    print(std::cerr);
	    std::cerr << " has no value" << std::endl;
	  }
	  delete pn;
	  return;
	}
	else {
	  pn->add(v->value);
	}
      }
      else {
	pn->add(param[k]);
      }
    }
    n = pn;
  }
  else {
    n = new StringName(name->print_name);
  }
  for (index_type k = 0; k < names.length(); k++) {
    if (n->equals(names[k])) {
      if (write_warnings && !ind.empty()) {
	std::cerr << "warning: multiple matches for " << n
		  << " in " << names << std::endl;
      }
      ind.insert(k);
    }
  }
  if (write_warnings && ind.empty()) {
    std::cerr << "warning: no match for " << n
	      << " in " << names << std::endl;
  }
  delete n;
}

PDDL_Base::Reference* PDDL_Base::Reference::instantiate_partially()
{
  Reference* r = new Reference(name, neg, has_args);
  if (has_args) fill_in_args(r);
  return r;
}

PDDL_Base::IrrelevantItem* PDDL_Base::IrrelevantItem::instantiate_partially()
{
  IrrelevantItem* item = new IrrelevantItem();
  item->param = param;
  for (index_type k = 0; k < pos_con.length(); k++)
    item->pos_con.append(pos_con[k]->instantiate_partially());
  for (index_type k = 0; k < neg_con.length(); k++)
    item->neg_con.append(neg_con[k]->instantiate_partially());
  for (index_type k = 0; k < type_con.length(); k++)
    if (type_con[k]->var->value == 0)
      item->type_con.append(type_con[k]);
  item->entity = entity->instantiate_partially();
  return item;
}

bool PDDL_Base::IrrelevantItem::match(symbol_vec& args)
{
  for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
  bool schema_match = entity->match(args);
  if (schema_match) {
    /*
    std::cerr << "schema match with";
    for (index_type k = 0; k < param.length(); k++) {
      std::cerr << " " << param[k]->print_name << " = ";
      if (param[k]->value) std::cerr << param[k]->value->print_name;
      else std::cerr << "0";
    }
    std::cerr << std::endl;
    */
    partial_value v = Context::partial_eval();
    if (v == p_true) {
      return true;
    }
    else if (v == p_false) {
      return false;
    }
    else {
      if (write_warnings) {
	std::cerr << "warning: undecided context condition in :irrelevant ";
	entity->print(std::cerr);
	std::cerr << " treated as false" << std::endl;
      }
      return false;
    }
  }
  else {
    return false;
  }
}

bool PDDL_Base::IrrelevantItem::match(variable_vec& args)
{
  for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
  bool schema_match = entity->match(args);
  if (schema_match) {
    /*
    std::cerr << "schema match with";
    for (index_type k = 0; k < param.length(); k++) {
      std::cerr << " " << param[k]->print_name << " = ";
      if (param[k]->value) std::cerr << param[k]->value->print_name;
      else std::cerr << "0";
    }
    std::cerr << std::endl;
    */
    partial_value v = Context::partial_eval();
    if (v == p_true) {
      return true;
    }
    else if (v == p_false) {
      return false;
    }
    else {
      if (write_warnings) {
	std::cerr << "warning: undecided context condition in :irrelevant ";
	entity->print(std::cerr);
	std::cerr << " treated as false" << std::endl;
      }
      return false;
    }
  }
  else {
    return false;
  }
}

bool PDDL_Base::TypeConstraint::is_true()
{
  if (var->value) {
    for (index_type k = 0; k < typ->elements.length(); k++) {
      if (typ->elements[k] == var->value) return true;
    }
    return false;
  }
  else {
    return true;
  }
}

PDDL_Base::partial_value PDDL_Base::TypeConstraint::partial_eval()
{
  if (var->value) {
    if (is_true()) {
      return p_true;
    }
    else {
      return p_false;
    }
  }
  else {
    return p_unknown;
  }
}

bool PDDL_Base::Context::context_is_static() const
{
  for (index_type k = 0; k < pos_con.length(); k++)
    if (!pos_con[k]->is_static()) return false;
  for (index_type k = 0; k < neg_con.length(); k++)
    if (!neg_con[k]->is_static()) return false;
  return true;
}

bool PDDL_Base::Context::occurs_in_context(Symbol* s)
{
  for (index_type k = 0; k < pos_con.length(); k++)
    if (pos_con[k]->occurs(s)) return true;
  for (index_type k = 0; k < neg_con.length(); k++)
    if (neg_con[k]->occurs(s)) return true;
  return false;
}

bool PDDL_Base::Context::is_true()
{
  for (index_type k = 0; k < type_con.length(); k++) {
    if (!type_con[k]->is_true()) return false;
  }
  for (index_type k = 0; k < pos_con.length(); k++) {
    if (pos_con[k]->is_static()) {
      partial_value v = pos_con[k]->partial_eval();
      if (v == p_true) return true;
      else if (v == p_false) return false;
      else {
	std::cerr << "error: context ";
	AtomBase::print_bindings = true;
	print(std::cerr);
	AtomBase::print_bindings = false;
	std::cerr << " undecided with complete assignment"
		  << std::endl;
	exit(255);
      }
    }
  }
  for (index_type k = 0; k < neg_con.length(); k++) {
    if (neg_con[k]->is_static()) {
      partial_value v = neg_con[k]->partial_eval();
      if (v == p_false) return true;
      else if (v == p_true) return false;
      else {
	std::cerr << "error: context ";
	AtomBase::print_bindings = true;
	print(std::cerr);
	AtomBase::print_bindings = false;
	std::cerr << " undecided with complete assignment"
		  << std::endl;
	exit(255);
      }
    }
  }
  return true;
}

void PDDL_Base::Context::clear_arguments()
{
  for (index_type k = 0; k < param.length(); k++)
    param[k]->value = 0;
}

void PDDL_Base::Context::set_mode(mode_keyword m)
{
  for (index_type k = 0; k < pos_con.length(); k++)
    if (pos_con[k]->at == md_none)
      pos_con[k]->at = m;
  for (index_type k = 0; k < neg_con.length(); k++)
    if (neg_con[k]->at == md_none)
      neg_con[k]->at = m;
}

PDDL_Base::partial_value PDDL_Base::Context::partial_eval()
{
  bool is_undecided = false;
  for (index_type k = 0; k < type_con.length(); k++) {
    partial_value v = type_con[k]->partial_eval();
    if (v == p_false) return p_false;
    if (v != p_true) is_undecided = true;
  }
  for (index_type k = 0; k < pos_con.length(); k++) {
    if (pos_con[k]->is_static()) {
      partial_value v = pos_con[k]->partial_eval();
      if (v == p_false) return p_false;
      if (v != p_true) is_undecided = true;
    }
  }
  for (index_type k = 0; k < neg_con.length(); k++) {
    if (neg_con[k]->is_static()) {
      partial_value v = neg_con[k]->partial_eval();
      if (v == p_true) return p_false;
      if (v != p_false) is_undecided = true;
    }
  }
  if (is_undecided)
    return p_unknown;
  return p_true;
}

bool PDDL_Base::SetOf::is_static() const {
  for (index_type k = 0; k < pos_atoms.length(); k++)
    if (!pos_atoms[k]->is_static()) return false;
  for (index_type k = 0; k < neg_atoms.length(); k++)
    if (!neg_atoms[k]->is_static()) return false;
  return true;
}

void PDDL_Base::SetOf::set_mode(mode_keyword m)
{
  Context::set_mode(m);
  for (index_type k = 0; k < pos_atoms.length(); k++)
    if (pos_atoms[k]->at == md_none)
      pos_atoms[k]->at = m;
  for (index_type k = 0; k < neg_atoms.length(); k++)
    if (neg_atoms[k]->at == md_none)
      neg_atoms[k]->at = m;
}

PDDL_Base::partial_value PDDL_Base::SetOf::partial_eval
(index_type p, bool as_disjunction)
{
  partial_value v_c = Context::partial_eval();
  if ((v_c == p_false) && !as_disjunction) return p_true;
  if ((v_c == p_false) && as_disjunction) return p_false;
  bool all_sat = true;
  bool some_sat = false;
  for (index_type k = 0; k < pos_atoms.length(); k++) {
    partial_value v_a = pos_atoms[k]->partial_eval();
    // conjunction: antecedent (context) is true but one atom is unsat:
    if ((v_c == p_true) && (v_a == p_false) && !as_disjunction) {
      if (PDDL_Base::write_info) {
	AtomBase::print_bindings = true;
	std::cerr << "info: set condition ";
	print(std::cerr);
	std::cerr << " evaluated to false"
		  << std::endl;	    
	AtomBase::print_bindings = true;
      }
      return p_false;
    }
    // disjunction: context and some atom is sat:
    if ((v_c == p_true) && (v_a == p_true) && as_disjunction) {
      return p_true;
    }
    // one atom is unsat (false or undecided)
    if (v_a != p_true) all_sat = false;
    // one atom is sat
    if (v_a == p_true) some_sat = true;
  }
  for (index_type k = 0; k < neg_atoms.length(); k++) {
    partial_value v_a = neg_atoms[k]->partial_eval();
    // conjunction: antecedent is true but one (negated) atom is unsat:
    if ((v_c == p_true) && (v_a == p_true) && !as_disjunction) {
      if (PDDL_Base::write_info) {
	AtomBase::print_bindings = true;
	std::cerr << "info: set condition ";
	print(std::cerr);
	std::cerr << " evaluated to false"
		  << std::endl;	    
	AtomBase::print_bindings = true;
      }
      return p_false;
    }
    // disjunction: context and some atom is sat:
    if ((v_c == p_true) && (v_a == p_false) && as_disjunction) {
      return p_true;
    }
    // one (negated) atom is unsat (true or undecided)
    if (v_a != p_false) all_sat = false;
    // one atom is sat
    if (v_a == p_false) some_sat = true;
  }
  // conjunction: antecedent is true or undecided, but all atoms are sat:
  if (!as_disjunction && all_sat) return p_true;
  // dijjunction: no atom is sat:
  if (as_disjunction && !some_sat) return p_false;
  if (p < param.length()) {
    partial_value v = p_true;
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);
      partial_value v_i = partial_eval(p+1);
      if (v_i == p_false) {
	param[p]->value = 0;
	return p_false;
      }
      else if (v_i == p_unknown) {
	v = p_unknown;
      }
    }
    param[p]->value = 0;
    return v;
  }
  return p_unknown;
}

PDDL_Base::partial_value PDDL_Base::SetOf::partial_eval(bool as_disjunction)
{
  if (!context_is_static()) {
    std::cerr << "error: expression ";
    print(std::cerr);
    std::cerr << " has non-static context" << std::endl;
    exit(255);
  }
  if (!is_static()) {
    std::cerr << "error: expression ";
    print(std::cerr);
    std::cerr << " has non-static atoms" << std::endl;
    exit(255);
  }
  for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
  return partial_eval(0, as_disjunction);
}

void PDDL_Base::SetOf::build_set
(Instance& ins, index_set& s, index_type p)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);
      if (Context::partial_eval() != p_false) {
	build_set(ins, s, p+1);
      }
    }
    param[p]->value = 0;
  }
  else if (Context::is_true()) {
    for (index_type k = 0; k < pos_atoms.length(); k++)
      if (!pos_atoms[k]->pred->is_equality()) {
	Instance::Atom* p = pos_atoms[k]->find_prop(ins, false, true);
	s.insert(p->index);
      }
    for (index_type k = 0; k < neg_atoms.length(); k++)
      if (!pos_atoms[k]->pred->is_equality()) {
	Instance::Atom* p = neg_atoms[k]->find_prop(ins, true, true);
	s.insert(p->index);
      }
  }
}

void PDDL_Base::SetOf::instantiate_as_set(Instance& ins, index_set& s)
{
  if (!context_is_static()) {
    std::cerr << "error: expression ";
    print(std::cerr);
    std::cerr << " has non-static context" << std::endl;
    exit(255);
  }
  for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
  build_set(ins, s, 0);
}

void PDDL_Base::SetOf::build_effect
(Instance& ins, index_set& s_add, index_set& s_del, index_type p)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);
      if (Context::partial_eval() != p_false) {
	build_effect(ins, s_add, s_del, p+1);
      }
    }
    param[p]->value = 0;
  }
  else if (Context::is_true()) {
    for (index_type k = 0; k < pos_atoms.length(); k++) {
      Instance::Atom* p = pos_atoms[k]->find_prop(ins, false, true);
      s_add.insert(p->index);
      if (pos_atoms[k]->pred->neg_pre) {
	Instance::Atom* np = pos_atoms[k]->find_prop(ins, true, true);
	s_del.insert(np->index);
      }
    }
    for (index_type k = 0; k < neg_atoms.length(); k++) {
      Instance::Atom* p = neg_atoms[k]->find_prop(ins, false, true);
      s_del.insert(p->index);
      if (neg_atoms[k]->pred->neg_pre) {
	Instance::Atom* np = neg_atoms[k]->find_prop(ins, true, true);
	s_add.insert(np->index);
      }
    }
  }
}

void PDDL_Base::SetOf::instantiate_as_effect
(Instance& ins, index_set& s_add, index_set& s_del)
{
  if (!context_is_static()) {
    std::cerr << "error: expression ";
    print(std::cerr);
    std::cerr << " has non-static context" << std::endl;
    exit(255);
  }
  for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
  build_effect(ins, s_add, s_del, 0);
}

void PDDL_Base::SetOf::build_conditional
(Instance& ins, rule_set& s_pos, rule_set& s_neg, index_type p)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);
      if (Context::partial_eval() != p_false) {
	build_conditional(ins, s_pos, s_neg, p + 1);
      }
    }
    param[p]->value = 0;
  }
  else {
    if (param.length() == 0) {
      if (!is_true()) return;
    }
    index_set ant;
    for (index_type k = 0; k < pos_con.length(); k++)
      if (!pos_con[k]->is_static()) {
	Instance::Atom* p = pos_con[k]->find_prop(ins, false, true);
	Instance::Atom* neg_p = pos_con[k]->find_prop(ins, true, true);
	ant.insert(p->index);
      }
    for (index_type k = 0; k < neg_con.length(); k++)
      if (!neg_con[k]->is_static()) {
	Instance::Atom* p = neg_con[k]->find_prop(ins, false, true);
	Instance::Atom* neg_p = neg_con[k]->find_prop(ins, true, true);
	ant.insert(neg_p->index);
      }
    for (index_type k = 0; k < pos_atoms.length(); k++) {
      Instance::Atom* q = pos_atoms[k]->find_prop(ins, false, true);
      if (pos_atoms[k]->pred->neg_pre) {
	Instance::Atom* not_q = pos_atoms[k]->find_prop(ins, true, true);
      }
      rule& r = s_pos.append();
      r.antecedent = ant;
      r.consequent = q->index;
    }
    for (index_type k = 0; k < neg_atoms.length(); k++) {
      Instance::Atom* q = neg_atoms[k]->find_prop(ins, false, true);
      if (pos_atoms[k]->pred->neg_pre) {
	Instance::Atom* not_q = neg_atoms[k]->find_prop(ins, true, true);
      }
      rule& r = s_neg.append();
      r.antecedent = ant;
      r.consequent = q->index;
    }
  }
}

void PDDL_Base::SetOf::instantiate_conditional
(Instance& ins, rule_set& s_pos, rule_set& s_neg)
{
  for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
  build_conditional(ins, s_pos, s_neg, 0);
}

PDDL_Base::SetOf* PDDL_Base::SetOf::instantiate_partially()
{
  SetOf* s = new SetOf();
  s->param = param;
  for (index_type k = 0; k < pos_con.length(); k++)
    s->pos_con.append(pos_con[k]->instantiate_partially());
  for (index_type k = 0; k < neg_con.length(); k++)
    s->neg_con.append(neg_con[k]->instantiate_partially());
  for (index_type k = 0; k < type_con.length(); k++)
    if (type_con[k]->var->value == 0)
      s->type_con.append(type_con[k]);
  for (index_type k = 0; k < pos_atoms.length(); k++)
    s->pos_atoms.append(pos_atoms[k]->instantiate_partially());
  for (index_type k = 0; k < neg_atoms.length(); k++)
    s->neg_atoms.append(neg_atoms[k]->instantiate_partially());
  return s;
}

void PDDL_Base::SetOf::compile
(atom_vec& pos_ins, atom_vec& neg_ins, index_type p)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);
      if (Context::partial_eval() != p_false) {
	compile(pos_ins, neg_ins, p + 1);
      }
    }
    param[p]->value = 0;
  }
  else {
    assert(context_is_static());
    if (is_true()) {
      for (index_type k = 0; k < pos_atoms.length(); k++)
	pos_ins.append(pos_atoms[k]->instantiate_partially());
      for (index_type k = 0; k < neg_atoms.length(); k++)
	neg_ins.append(neg_atoms[k]->instantiate_partially());
    }
  }
}

void PDDL_Base::SetOf::compile_non_static
(atom_set_vec& ins, index_type p)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);
      if (Context::partial_eval() != p_false) {
	compile_non_static(ins, p + 1);
      }
    }
    param[p]->value = 0;
  }
  else if (is_true()) {
    SetOf* d = new SetOf();
    for (index_type k = 0; k < pos_con.length(); k++)
      d->neg_atoms.append(pos_con[k]->instantiate_partially());
    for (index_type k = 0; k < neg_con.length(); k++)
      d->pos_atoms.append(neg_con[k]->instantiate_partially());
    for (index_type k = 0; k < pos_atoms.length(); k++)
      d->pos_atoms.append(pos_atoms[k]->instantiate_partially());
    for (index_type k = 0; k < neg_atoms.length(); k++)
      d->neg_atoms.append(neg_atoms[k]->instantiate_partially());
    ins.append(d);
  }
}

// bool PDDL_Base::Disjunction::is_static()
// {
//   for (index_type k = 0; k < pos_atoms.length(); k++)
//     if (!pos_atoms[k]->is_static()) return false;
//   for (index_type k = 0; k < neg_atoms.length(); k++)
//     if (!neg_atoms[k]->is_static()) return false;
//   return true;
// }

// PDDL_Base::partial_value PDDL_Base::Disjunction::partial_eval(index_type p)
// {
//   if (p < param.length()) {
//     std::cerr << "warning: partial evaluation of existentially quantified preconditions N.Y.I." << std::endl;
//     return p_unknown;
//   }
//   else {
//     partial_value v = p_false;
//     for (index_type k = 0; k < pos_atoms.length(); k++) {
//       partial_value vk = pos_atoms[k]->partial_eval();
//       if (vk == p_true) return p_true;
//       if (vk == p_unknown) v = p_unknown;
//     }
//     for (index_type k = 0; k < neg_atoms.length(); k++) {
//       partial_value vk = neg_atoms[k]->partial_eval();
//       if (vk == p_false) return p_true;
//       if (vk == p_unknown) v = p_unknown;
//     }
//     return v;
//   }
// }

// PDDL_Base::partial_value PDDL_Base::Disjunction::partial_eval()
// {
//   if (!context_is_static() || !is_static()) {
//     std::cerr << "error: expression ";
//     print(std::cerr);
//     std::cerr << " is non-static" << std::endl;
//     exit(255);
//   }
//   for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
//   return partial_eval(0);
// }

// PDDL_Base::Disjunction* PDDL_Base::Disjunction::instantiate_partially()
// {
//   Disjunction* d = new Disjunction();
//   for (index_type k = 0; k < pos_atoms.length(); k++)
//     d->pos_atoms.append(pos_atoms[k]->instantiate_partially());
//   for (index_type k = 0; k < neg_atoms.length(); k++)
//     d->neg_atoms.append(neg_atoms[k]->instantiate_partially());
//   d->param = param;
//   for (index_type k = 0; k < pos_con.length(); k++)
//     d->pos_con.append(pos_con[k]->instantiate_partially());
//   for (index_type k = 0; k < neg_con.length(); k++)
//     d->neg_con.append(neg_con[k]->instantiate_partially());
//   for (index_type k = 0; k < type_con.length(); k++)
//     if (type_con[k]->var->value == 0)
//       d->type_con.append(type_con[k]);
//   return d;
// }

// void PDDL_Base::Disjunction::build(Instance& ins, index_set& s, index_type p)
// {
//   if (p < param.length()) {
//     for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
//       param[p]->value = param[p]->sym_types.get_element(k);
//       /* check context conditions */
//       if (Context::partial_eval() != p_false) build(ins, s, p + 1);
//     }
//     param[p]->value = 0;
//   }
//   else if (is_true()) {
//     for (index_type k = 0; k < pos_atoms.length(); k++) {
//       Instance::Atom* a = pos_atoms[k]->find_prop(ins, false, true);
//       s.insert(a->index);
//     }
//     for (index_type k = 0; k < neg_atoms.length(); k++) {
//       Instance::Atom* a = neg_atoms[k]->find_prop(ins, true, true);
//       s.insert(a->index);
//     }
//   }
// }

// void PDDL_Base::Disjunction::instantiate(Instance& ins, index_set& s)
// {
//   s.clear();
//   for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
//   build(ins, s, 0);
// }

void PDDL_Base::SetConstraint::build(Instance& ins, index_type p)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);
      /* check context conditions */
      if (Context::partial_eval() != p_false) build(ins, p+1);
    }
    param[p]->value = 0;
  }
  else {
    index_set c_set;
    for (index_type k = 0; k < atom_sets.length(); k++) {
      atom_sets[k]->instantiate_as_set(ins, c_set);
    }
    for (index_type k = 0; k < pos_atoms.length(); k++) {
      Instance::Atom* p = pos_atoms[k]->find_prop(ins, false, true);
      c_set.insert(p->index);
    }
    for (index_type k = 0; k < neg_atoms.length(); k++) {
      Instance::Atom* p = neg_atoms[k]->find_prop(ins, true, true);
      c_set.insert(p->index);
    }
    if (c_set.empty()) {
      if (write_warnings) {
	std::cerr << "warning: ignoring" << std::endl;
	AtomBase::print_bindings = true;
	print(std::cerr);
	AtomBase::print_bindings = false;
	std::cerr << "because atom set is empty" << std::endl;
      }
      return;
    }

    Instance::Constraint& c =
      ins.new_invariant(c_set, sc_count, (sc_type == sc_exactly));
    if (!c.src) {
      c.src = this;
    }
    if (name && !c.name) {
      c.name = new PDDL_Name(name, param, false);
    }
  }
}

void PDDL_Base::SetConstraint::instantiate(Instance& ins)
{
  if (sc_type == sc_at_least) {
    if (write_warnings)
      std::cerr << "warning: skipping :at-least-n invariant" << std::endl;
    return;
  }
  if (!included) return;

  if (!context_is_static()) {
    if (write_warnings)
      std::cerr << "warning: skipping invariant with non-static context"
		<< std::endl;
    return;
  }

  bool any_static = false;
  for (index_type k = 0; k < pos_atoms.length(); k++)
    if (pos_atoms[k]->is_static()) any_static = true;
  for (index_type k = 0; k < neg_atoms.length(); k++)
    if (neg_atoms[k]->is_static()) any_static = true;
  for (index_type k = 0; k < atom_sets.length(); k++)
    if (atom_sets[k]->is_static()) any_static = true;
  if (any_static) {
    if (write_warnings)
      std::cerr << "warning: skipping invariant with static atoms"
		<< std::endl;
    return;
  }

  for (index_type k = 0; k < param.length(); k++) param[k]->value = 0;
  build(ins, 0);
}

void PDDL_Base::instantiate(Instance& ins)
{
  if (!ready_to_instantiate) post_process();

  for (index_type k = 0; k < dom_constants.length(); k++) {
    Atom* a = new Atom(dom_eq_pred);
    a->param.append(dom_constants[k]);
    a->param.append(dom_constants[k]);
    a->insert(dom_eq_pred->init);
  }

  if (name_instance_by_problem_file) {
    char* s = problem_file_basename();
    if (s) {
      ins.name = new StringName(s);
    }
    else {
      ins.name = new StringName("NONAME");
    }
  }
  else {
    ins.name = new InstanceName
      (domain_name ? tab.table_char_map().strdup(domain_name) :
       tab.table_char_map().strdup("??"),
       problem_name ? tab.table_char_map().strdup(problem_name) :
       tab.table_char_map().strdup("??"));
  }
  if (instance_name_prefix != 0) {
    ins.name = new ConcatenatedName(new StringName(instance_name_prefix),
				    ins.name, '-');
  }

  // mark included/excluded items
  for (index_type k = 0; k < dom_sc_invariants.length(); k++) {
    dom_sc_invariants[k]->included =
      dom_sc_invariants[k]->item_is_included(excluded_dkel_tags,
					     required_dkel_tags);
    if (!dom_sc_invariants[k]->included) {
      if (PDDL_Base::write_info) {
	std::cerr << "DKEL: excluding DKEL item" << std::endl;
	dom_sc_invariants[k]->print(std::cerr);
      }
    }
  }
  for (index_type k = 0; k < dom_irrelevant.length(); k++) {
    dom_irrelevant[k]->included =
      dom_irrelevant[k]->item_is_included(excluded_dkel_tags,
					  required_dkel_tags);
    if (!dom_irrelevant[k]->included) {
      if (PDDL_Base::write_info) {
	std::cerr << "DKEL: excluding DKEL item" << std::endl;
	dom_irrelevant[k]->print(std::cerr);
      }
    }
  }

  if (create_all_atoms) {
    for (index_type k = 0; k < dom_predicates.length(); k++)
      dom_predicates[k]->instantiate(ins);
  }

  for (index_type k = 0; k < dom_actions.length(); k++)
    if (!dom_actions[k]->is_abstract()) {
      if (metric) {
	// if metric is MAX, flip the expression and set to MIN
	Expression* c_exp = metric->copy();
	if (c_exp) {
	  if (metric_type == metric_maximize) {
	    c_exp = new BinaryExpression(exp_mul, c_exp,
					 new ConstantExpression(-1));
	  }
	  if (dom_actions[k]->dmax) {
	    c_exp->substitute_for_time(dom_actions[k]->dmax);
	  }
	  else if (dom_actions[k]->dmin) {
	    c_exp->substitute_for_time(dom_actions[k]->dmin);
	  }
	  else {
	    c_exp->substitute_for_time(new ConstantExpression(1));
	  }
	}
	dom_actions[k]->instantiate(ins, c_exp);
      }
      else {
	dom_actions[k]->instantiate(ins, 0);
      }
    }

  // need to post-process grounded actions once we know # of resources
  for (index_type k = 0; k < ins.n_actions(); k++) {
    ins.actions[k].use.inc_length_to(ins.n_resources(), 0);
    ins.actions[k].cons.inc_length_to(ins.n_resources(), 0);
  }

  for (index_type k = 0; k < dom_goals.length(); k++) {
    if (dom_goals[k]->is_propositional()) {
      dom_goals[k]->instantiate(ins, POS_INF);
    }
    else if (write_warnings || !best_effort) {
      std::cerr << "warning: goal ";
      print(std::cerr);
      std::cerr << " ignored in instantiation" << std::endl;
      if (!best_effort) exit(1);
    }
  }

  for (index_type k = 0; k < dom_sc_invariants.length(); k++)
    dom_sc_invariants[k]->instantiate(ins);
}

void PDDL_Base::instantiate_atom_set
(Instance& ins, atom_vec& a, index_set& s)
{
  for (index_type k = 0; k < a.length(); k++) {
    Instance::Atom* p = a[k]->find_prop(ins, false, true);
    s.insert(p->index);
  }
}

void PDDL_Base::Goal::instantiate(Instance& ins, NTYPE deadline)
{
  if (g_class == goal_pos_atom) {
    Instance::Atom* p = ((AtomicGoal*)this)->atom->find_prop(ins, false, true);
    p->goal = true;
    p->goal_t = deadline;
  }
  else if (g_class == goal_neg_atom) {
    Instance::Atom* p = ((AtomicGoal*)this)->atom->find_prop(ins, true, true);
    // std::cerr << "setting goal " << p->index << std::endl;
    p->goal = true;
    p->goal_t = deadline;
  }
  else if (g_class == goal_conjunction) {
    ConjunctiveGoal* g = (ConjunctiveGoal*)this;
    for (index_type k = 0; k < g->goals.length(); k++)
      g->goals[k]->instantiate(ins, deadline);
  }
  else if (g_class == goal_within) {
    DeadlineGoal* g = (DeadlineGoal*)this;
    g->goal->instantiate(ins, g->at);
  }
  else if ((g_class == goal_always) && compile_away_plan_constraints) {
    index_set a;
    ((SimpleSequenceGoal*)this)->constraint->instantiate(ins, a, 0, 0);
    ins.compile_pc_always(a, 0);
  }
  else if ((g_class == goal_sometime) && compile_away_plan_constraints) {
    index_set a;
    ((SimpleSequenceGoal*)this)->constraint->instantiate(ins, a, 0, 0);
    ins.compile_pc_sometime(a, 0);
  }
  else if ((g_class == goal_at_most_once) && compile_away_plan_constraints) {
    index_set a;
    ((SimpleSequenceGoal*)this)->constraint->instantiate(ins, a, 0, 0);
    ins.compile_pc_at_most_once(a, 0);
  }
  else if ((g_class == goal_sometime_before) && compile_away_plan_constraints) {
    index_set a;
    ((TriggeredSequenceGoal*)this)->trigger->instantiate(ins, a, 0, 0);
    index_set b;
    ((TriggeredSequenceGoal*)this)->constraint->instantiate(ins, b, 0, 0);
    ins.compile_pc_sometime_before(a, b, 0);
  }
  else if (write_warnings || !best_effort) {
    std::cerr << "warning: goal ";
    print(std::cerr);
    std::cerr << " ignored in instantiation (1)" << std::endl;
    if (!best_effort) exit(1);
  }
}

void PDDL_Base::Goal::instantiate
(Instance& ins, index_set& set, Symbol* p, index_type i)
{
  if (g_class == goal_pos_atom) {
    Instance::Atom* p = ((AtomicGoal*)this)->atom->find_prop(ins, false, true);
    set.insert(p->index);
  }
  else if (g_class == goal_neg_atom) {
    Instance::Atom* p = ((AtomicGoal*)this)->atom->find_prop(ins, true, true);
    set.insert(p->index);
  }
  else if (g_class == goal_conjunction) {
    ConjunctiveGoal* g = (ConjunctiveGoal*)this;
    for (index_type k = 0; k < g->goals.length(); k++)
      g->goals[k]->instantiate(ins, set, p, k);
  }
  else if ((g_class == goal_always) && compile_away_plan_constraints) {
    index_set a;
    ((SimpleSequenceGoal*)this)->constraint->instantiate(ins, a, p, i);
    Name* n = (p ? (i != no_such_index ?
		    (Name*)new EnumName(p->print_name, i) :
		    (Name*)new StringName(p->print_name)) :
	       (Name*)0);
    set.insert(ins.compile_pc_always(a, n));
  }
  else if ((g_class == goal_sometime) && compile_away_plan_constraints) {
    index_set a;
    ((SimpleSequenceGoal*)this)->constraint->instantiate(ins, a, p, i);
    Name* n = (p ? (i != no_such_index ?
		    (Name*)new EnumName(p->print_name, i) :
		    (Name*)new StringName(p->print_name)) :
	       (Name*)0);
    set.insert(ins.compile_pc_sometime(a, n));
  }
  else if ((g_class == goal_at_most_once) && compile_away_plan_constraints) {
    index_set a;
    ((SimpleSequenceGoal*)this)->constraint->instantiate(ins, a, p, i);
    Name* n = (p ? (i != no_such_index ?
		    (Name*)new EnumName(p->print_name, i) :
		    (Name*)new StringName(p->print_name)) :
	       (Name*)0);
    set.insert(ins.compile_pc_at_most_once(a, n));
  }
  else if ((g_class == goal_sometime_before) && compile_away_plan_constraints) {
    index_set a;
    ((TriggeredSequenceGoal*)this)->trigger->instantiate(ins, a, p, i);
    index_set b;
    ((TriggeredSequenceGoal*)this)->constraint->instantiate(ins, b, p, i);
    Name* n = (p ? (i != no_such_index ?
		    (Name*)new EnumName(p->print_name, i) :
		    (Name*)new StringName(p->print_name)) :
	       (Name*)0);
    set.insert(ins.compile_pc_sometime_before(a, b, n));
  }
  else if (write_warnings || !best_effort) {
    std::cerr << "warning: goal ";
    print(std::cerr);
    std::cerr << " ignored in instantiation (2)" << std::endl;
    if (!best_effort) exit(1);
  }
}

void PDDL_Base::instantiateHTN(HTNInstance& ins)
{
  std::cerr << "instantiateHTN: first pass..." << std::endl;
  for (index_type k = 0; k < dom_actions.length(); k++)
    if (dom_actions[k]->is_abstract())
      dom_actions[k]->instantiate_abstract_1(ins);
  std::cerr << "instantiateHTN: second pass..." << std::endl;
  for (index_type k = 0; k < dom_actions.length(); k++)
    if (dom_actions[k]->is_abstract())
      dom_actions[k]->instantiate_abstract_2(ins);

  ins.goal_tasks.set_length(0);
  for (index_type k = 0; k < goal_tasks.length(); k++) {
    ActionSymbol* act = (ActionSymbol*)(goal_tasks[k]->name);
    void* ains = goal_tasks[k]->find_action();
    if (!ains) {
      std::cerr << "error: no match for ";
      goal_tasks[k]->print(std::cerr);
      std::cerr << " in goal" << std::endl;
      exit(255);
    }
    else {
      if (act->is_abstract()) {
	HTNInstance::Task* tk = (HTNInstance::Task*)ains;
	ins.goal_tasks.append(HTNInstance::step(true, tk->index));
      }
      else {
	Instance::Action* a = (Instance::Action*)ains;
	ins.goal_tasks.append(HTNInstance::step(false, a->index));
      }
    }
  }

  std::cerr << "instantiateHTN: done." << std::endl;
}

NTYPE PDDL_Base::Preference::value(metric_class metric_type, Expression* m)
{
  assert(m);
  // std::cerr << "evaluating ";
  // m->print(std::cerr, false);
  // std::cerr << " w.r.t. ";
  // name->print(std::cerr);
  // std::cerr << "...";
  NTYPE b = m->eval_delta(0, 0, 1);
  NTYPE d = m->eval_delta(name, 0, 1);
  // std::cerr << " = -(" << d << " - " << b << ") = "
  // << -1*(d - b) << std::endl;
  return (metric_type == metric_minimize ? -1 : 1) * (d - b);
}

void PDDL_Base::Preference::instantiate
(SoftInstance& ins, metric_class metric_type, Expression* m)
{
  SoftInstance::SoftGoal& g = ins.new_soft_goal();
  g.name = new StringName(name->print_name, true);
  g.src = this;
  goal->instantiate(ins, g.atoms, name, no_such_index);
  for (index_type k = 0; k < g.atoms.length(); k++)
    ins.atoms[g.atoms[k]].goal = true;
  g.weight = (m ? value(metric_type, m) : 1);
}

void PDDL_Base::instantiate_soft(SoftInstance& ins)
{
  for (index_type k = 0; k < ins.n_atoms(); k++)
    if (ins.atoms[k].goal) ins.hard.insert(k);
  for (index_type k = 0; k < dom_preferences.length(); k++) {
    if (dom_preferences[k]->is_propositional()) {
      dom_preferences[k]->instantiate(ins, metric_type, metric);
    }
    else if (write_warnings || !best_effort) {
      std::cerr << "warning: non-propositional soft goal ";
      dom_preferences[k]->print(std::cerr);
      std::cerr << " ignored" << std::endl;
      if (!best_effort) exit(1);
    }
  }
  if (metric) {
    ins.null_value =
      (metric_type == metric_minimize ? -1 : 1) * metric->eval_delta(0, 0, 1);
  }
}

bool PDDL_Base::InputPlan::export_to_instance
(Instance& ins, const index_vec& map, Plan& p)
{
  if (steps.length() == 0) {
    p.end();
    return true;
  }

  InputPlanStep* sorted[steps.length() + 1];
  for (index_type k = 0; k < steps.length(); k++) {
    if (steps[k]->act->index.empty()) {
      std::cerr << "error (export plan): action ";
      steps[k]->act->print(std::cerr);
      std::cerr << " does not exist in instance!" << std::endl;
      exit(255);
    }

    index_type i = 0;
    bool found = false;
    while ((i < k) && !found) {
      if (sorted[i]->start_time > steps[k]->start_time) {
	for (index_type j = k - 1; j >= i; j--) sorted[j+1] = sorted[j];
	sorted[i] = steps[k];
	found = true;
      }
      else {
	i += 1;
      }
    }
    if (!found) {
      sorted[k] = steps[k];
    }
  }

  for (index_type k = 0; k < steps.length(); k++) {
    if (sorted[k]->act->index.empty()) {
      std::cerr << "warning (export plan): no action ";
      sorted[k]->act->print(std::cerr);
      std::cerr << " in instance" << std::endl;
      return false;
    }
    if (sorted[k]->act->index.length() > 1) {
      std::cerr << "warning (export plan): reference to action ";
      sorted[k]->act->print(std::cerr);
      std::cerr << " is ambiguous" << std::endl;
      return false;
    }
    index_type i0 = sorted[k]->act->index[0];
    if (i0 > map.length()) {
      std::cerr << "error (export plan): action ";
      sorted[k]->act->print(std::cerr);
      std::cerr << " has invalid index " << sorted[k]->act->index
		<< std::endl;
      exit(255);
    }
    index_type new_index = map[i0];
    if (new_index == no_such_index) {
      std::cerr << "warning (export plan): action ";
      sorted[k]->act->print(std::cerr);
      std::cerr << " has been removed" << std::endl;
      return false;
    }
    p.insert(new_index);
    if (k < steps.length() - 1) {
      p.advance(sorted[k + 1]->start_time - sorted[k]->start_time);
    }
    else {
      p.end();
    }
  }

  return true;
}

bool PDDL_Base::export_plan
(index_type i, Instance& ins, const index_vec& map, Plan& p)
{
  if (input_plans.length() <= i) {
    std::cerr << "error: can't export plan " << i << " since there are only "
	      << input_plans.length() << " plans in input" << std::endl;
    exit(255);
  }
  bool ok = input_plans[i]->export_to_instance(ins, map, p);
  if (!ok) return false;
  if (input_plans[i]->name)
    p.set_name(new StringName(input_plans[i]->name->print_name, true));
  p.set_optimal(input_plans[i]->is_opt);
  return true;
}

bool PDDL_Base::export_plan
(index_type i, Instance& ins, Plan& p)
{
  if (input_plans.length() <= i) {
    std::cerr << "error: can't export plan " << i << " since there are only "
	      << input_plans.length() << " plans in input" << std::endl;
    exit(255);
  }
  index_vec map(no_such_index, ins.n_actions());
  for (index_type k = 0; k < ins.n_actions(); k++) map[k] = k;
  bool ok = input_plans[i]->export_to_instance(ins, map, p);
  if (!ok) return false;
  if (input_plans[i]->name)
    p.set_name(new StringName(input_plans[i]->name->print_name, true));
  p.set_optimal(input_plans[i]->is_opt);
  return true;
}

void PDDL_Base::export_heuristic
(Instance& ins, const index_vec& atom_map, bool opt_maximize, Heuristic& h)
{
  for (index_type k = 0; k < h_table.length(); k++) {
    index_set s;
    for (index_type i = 0; i < h_table[k]->atoms.length(); i++) {
      Instance::Atom* a =
	h_table[k]->atoms[i]->find_prop(ins, h_table[k]->neg[i], false);
      if (!a) {
	std::cerr << "error (export heuristic): atom ";
	h_table[k]->atoms[i]->print(std::cerr);
	std::cerr << " does not exist in instance" << std::endl;
	exit(255);
      }
      s.insert(a->index);
    }
    index_type l = s.length();
    ins.remap_set(s, atom_map);
    if (s.length() != l) {
      std::cerr << "warning: set ";
      ins.write_atom_set(std::cerr, s);
      std::cerr << " shrunk in remap, ignoring it" << std::endl;
    }
    else {
      if (opt_maximize) {
	NTYPE v = h.eval(s);
	if (v < h_table[k]->cost) {
	  if (write_info) {
	    std::cerr << "info: storing ";
	    ins.write_atom_set(std::cerr, s);
	    std::cerr << " = " << v << std::endl;
	  }
	  h.store(s, h_table[k]->cost, h_table[k]->opt);
	}
      }
      else {
	h.store(s, h_table[k]->cost, h_table[k]->opt);
      }
    }
  }
}

bool PDDL_Base::SimpleReferenceSet::build
(const name_vec& names, index_set& set, index_type p)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);
      if (Context::partial_eval() != p_false) {
	bool ok = build(names, set, p + 1);
	if (!ok && PDDL_Base::strict_set_export) return false;
      }
    }
    param[p]->value = 0;
    return true;
  }
  else if (is_true()) {
    index_set m;
    ref->find(names, m);
    if (m.empty() && PDDL_Base::strict_set_export) {
      return false;
    }
    if ((m.length() > 1) && !PDDL_Base::strict_set_export) {
      return false;
    }
    if (write_info) {
      std::cerr << "info: reference ";
      print(std::cerr);
      std::cerr << " with binding ";
      print_assignment(std::cerr);
      std::cerr << " matched to {";
      for (index_type k = 0; k < m.length(); k++) {
	if (k > 0) std::cerr << ", ";
	std::cerr << m[k] << "." << names[m[k]];
      }
      std::cerr << "}" << std::endl;
    }
    set.insert(m);
    return true;
  }
}

void PDDL_Base::ReferenceSet::build
(const name_vec& names, index_set_vec& sets, index_type p)
{
  if (p < param.length()) {
    for (index_type k = 0; k < param[p]->sym_types.n_elements(); k++) {
      param[p]->value = param[p]->sym_types.get_element(k);
      if (Context::partial_eval() != p_false) {
	build(names, sets, p + 1);
      }
    }
    param[p]->value = 0;
  }
  else if (is_true()) {
    index_set s;
    bool all_ok = true;
    for (index_type k = 0; (k < refs.length()) && all_ok; k++) {
      bool ok = refs[k]->build(names, s, 0);
      if (!ok && write_warnings) {
	std::cerr << "warning: failed to build instance ";
	print_assignment(std::cerr);
	std::cerr << " of ";
	print(std::cerr);
	std::cerr << std::endl;
      }
      all_ok = (all_ok && ok);
    }
    if (all_ok) sets.append(s);
  }
}

void PDDL_Base::export_sets(const name_vec& names, index_set_vec& sets)
{
  for (index_type k = 0; k < input_sets.length(); k++) {
    input_sets[k]->build(names, sets, 0);
  }
}

void PDDL_Base::export_action_partitions(name_vec& names, index_set_vec& sets)
{
  names.clear();
  sets.clear();
  for (index_type k = 0; k < partitions.length(); k++) {
    SetSymbol* s = partitions[k];
    assert(s->sets.length() == s->names.length());
    for (index_type i = 0; i < s->sets.length(); i++) {
      names.append(s->names[i]);
      sets.append(*(s->sets[i]));
    }
  }
}

bool PDDL_Base::DKEL_Item::item_is_included
(string_set& ex_tags, string_set& req_tags)
{
  if (exclude_all_dkel_items) return false;
  for (index_type k = 0; k < item_tags.length(); k++) {
    if (ex_tags.contains(item_tags[k])) {
      return false;
    }
  }
  for (index_type k = 0; k < req_tags.length(); k++) {
    if (!item_tags.contains(req_tags[k])) {
      return false;
    }
  }
  return true;
}

void PDDL_Base::lift_DKEL_Items(const Instance& ins)
{
  for (index_type k = 0; k < ins.n_invariants(); k++)
    if (ins.invariants[k].src == 0) {
      SetConstraint* c = new SetConstraint();
      for (index_type i = 0; i < ins.invariants[k].set.length(); i++) {
	ptr_pair* p = (ptr_pair*)ins.atoms[ins.invariants[k].set[i]].src;
	assert(p);
	bool n;
	Atom* a = make_atom_from_prop(*p, n);
	if (!n) {
	  c->pos_atoms.append(a);
	}
	else {
	  c->neg_atoms.append(a);
	}
      }
      if (ins.invariants[k].exact) {
	c->sc_type = sc_exactly;
      }
      else {
	c->sc_type = sc_at_most;
      }
      c->sc_count = ins.invariants[k].lim;
      if (ins.invariants[k].verified) {
	c->item_tags.append((char*)"verified");
      }
      c->defined_in_problem = true;
      dom_sc_invariants.append(c);
    }
}

PDDL_Base::PredicateSymbol*
PDDL_Base::find_type_predicate(Symbol* type_sym)
{
  for (index_type k = 0; k < dom_predicates.length(); k++)
    if (dom_predicates[k]->print_name == type_sym->print_name)
      return dom_predicates[k];
  std::cerr << "error: no type predicate found for type "
	    << type_sym->print_name << std::endl;
  exit(255);
}

void PDDL_Base::Formula::untype(PDDL_Base* base)
{
  switch (fc) {
  case fc_false:
  case fc_true:
  case fc_atom:
  case fc_equality:
    return;
  case fc_negation:
    ((NFormula*)this)->f->untype(base);
    return;
  case fc_conjunction:
  case fc_disjunction:
    for (index_type k = 0; k < ((CFormula*)this)->parts.length(); k++) {
      ((CFormula*)this)->parts[k]->untype(base);
    }
    return;
  case fc_implication:
  case fc_equivalence:
    ((BFormula*)this)->f1->untype(base);
    ((BFormula*)this)->f2->untype(base);
    return;
  case fc_universal:
    {
      QFormula* qf = (QFormula*)this;
      CFormula* df = new CFormula(fc_disjunction);
      for (index_type k = 0; k < qf->vars.length(); k++) {
	if (qf->vars[k]->sym_types.length() == 1) {
	  if (qf->vars[k]->sym_types[0] != base->dom_top_type) {
	    PredicateSymbol* type_pred =
	      base->find_type_predicate(qf->vars[k]->sym_types[0]);
	    AFormula* type_atom = new AFormula(type_pred);
	    type_atom->param.append(qf->vars[k]);
	    df->add(new NFormula(type_atom));
	  }
	  qf->vars[k]->sym_types.clear();
	}
	else if (qf->vars[k]->sym_types.length() > 1) {
	  CFormula* ddf = new CFormula(fc_disjunction);
	  for (index_type i = 0; i < qf->vars[k]->sym_types.length(); i++) {
	    if (qf->vars[k]->sym_types[i] != base->dom_top_type) {
	      PredicateSymbol* type_pred =
		base->find_type_predicate(qf->vars[k]->sym_types[i]);
	      AFormula* type_atom = new AFormula(type_pred);
	      type_atom->param.append(qf->vars[k]);
	      ddf->add(type_atom);
	    }
	  }
	  df->add(new NFormula(ddf));
	  qf->vars[k]->sym_types.clear();
	}
      }
      qf->f->untype(base);
      df->add(qf->f);
      qf->f = df;
      return;
    }
  case fc_existential:
    {
      QFormula* qf = (QFormula*)this;
      CFormula* cf = new CFormula(fc_conjunction);
      for (index_type k = 0; k < qf->vars.length(); k++) {
	if (qf->vars[k]->sym_types.length() == 1) {
	  if (qf->vars[k]->sym_types[0] != base->dom_top_type) {
	    PredicateSymbol* type_pred =
	      base->find_type_predicate(qf->vars[k]->sym_types[0]);
	    AFormula* type_atom = new AFormula(type_pred);
	    type_atom->param.append(qf->vars[k]);
	    cf->add(type_atom);
	  }
	  qf->vars[k]->sym_types.clear();
	}
	else if (qf->vars[k]->sym_types.length() > 1) {
	  CFormula* ddf = new CFormula(fc_disjunction);
	  for (index_type i = 0; i < qf->vars[k]->sym_types.length(); i++) {
	    if (qf->vars[k]->sym_types[i] != base->dom_top_type) {
	      PredicateSymbol* type_pred =
		base->find_type_predicate(qf->vars[k]->sym_types[i]);
	      AFormula* type_atom = new AFormula(type_pred);
	      type_atom->param.append(qf->vars[k]);
	      ddf->add(type_atom);
	    }
	  }
	  cf->add(ddf);
	  qf->vars[k]->sym_types.clear();
	}
      }
      qf->f->untype(base);
      cf->add(qf->f);
      qf->f = cf;
      return;
    }
  }
}

void PDDL_Base::Context::untype(PDDL_Base* base)
{
  // untype action parameters
  for (index_type k = 0; k < param.length(); k++) {
    if (param[k]->sym_types.length() == 1) {
      if (param[k]->sym_types[0] != base->dom_top_type) {
	PredicateSymbol* type_pred =
	  base->find_type_predicate(param[k]->sym_types[0]);
	Atom* type_atom = new Atom(type_pred);
	type_atom->param.append(param[k]);
	pos_con.append(type_atom);
      }
      param[k]->sym_types.clear();
    }
    else if (param[k]->sym_types.length() > 1) {
      std::cerr << "error: can't untype ";
      param[k]->print(std::cerr);
      std::cerr << " in ";
      print(std::cerr);
      std::cerr << std::endl;
      exit(255);
    }
  }
}

void PDDL_Base::SetConstraint::untype(PDDL_Base* base)
{
  PDDL_Base::Context::untype(base);
  for (index_type k = 0; k < atom_sets.length(); k++)
    atom_sets[k]->untype(base);
}

void PDDL_Base::InvariantFormula::untype(PDDL_Base* base)
{
  PDDL_Base::Context::untype(base);
  f->untype(base);
}

void PDDL_Base::ActionSymbol::untype(PDDL_Base* base)
{
  // untype action parameters
  for (index_type k = 0; k < param.length(); k++) {
    if (param[k]->sym_types.length() == 1) {
      if (param[k]->sym_types[0] != base->dom_top_type) {
	PredicateSymbol* type_pred =
	  base->find_type_predicate(param[k]->sym_types[0]);
	Atom* type_atom = new Atom(type_pred);
	type_atom->param.append(param[k]);
	pos_pre.append(type_atom);
      }
      param[k]->sym_types.clear();
    }
    else if (param[k]->sym_types.length() > 1) {
      SetOf* d = new SetOf();
      for (index_type i = 0; i < param[k]->sym_types.length(); i++)
	if (param[k]->sym_types[i] != base->dom_top_type) {
	  PredicateSymbol* type_pred =
	    base->find_type_predicate(param[k]->sym_types[i]);
	  Atom* type_atom = new Atom(type_pred);
	  type_atom->param.append(param[k]);
	  d->pos_atoms.append(type_atom);
	}
      dis_pre.append(d);
      param[k]->sym_types.clear();
    }
  }

  // untype set preconditions
  for (index_type k = 0; k < set_pre.length(); k++) set_pre[k]->untype(base);

  // untype set/cond effects
  for (index_type k = 0; k < set_eff.length(); k++) set_eff[k]->untype(base);
  for (index_type k = 0; k < cond_eff.length(); k++) cond_eff[k]->untype(base);
}

void PDDL_Base::untype()
{
  StringTable::Cell* var_x = tab.inserta("?X");

  // for each type...
  for (index_type k = 0; k < dom_types.length(); k++) {
    // ...create a new type predicate
    PredicateSymbol* type_pred =
      new PredicateSymbol(dom_types[k]->print_name);
    type_pred->param.append(new VariableSymbol(var_x->text));
    type_pred->pos_pre = true;
    dom_predicates.append(type_pred);

    // ...and make it initially true for each element of the type
    for (index_type i = 0; i < dom_types[k]->elements.length(); i++) {
      Atom* a = new Atom(type_pred);
      a->param.append(dom_types[k]->elements[i]);
      a->insert(type_pred->init);
      dom_init.append(a);
    }
  }

  // remove types from predicate and function declarations
  for (index_type k = 0; k < dom_predicates.length(); k++) {
    PredicateSymbol* pred = dom_predicates[k];
    for (index_type i = 0; i < pred->param.length(); i++)
      pred->param[i]->sym_types.clear();
  }
  for (index_type k = 0; k < dom_functions.length(); k++) {
    FunctionSymbol* fun = dom_functions[k];
    for (index_type i = 0; i < fun->param.length(); i++)
      fun->param[i]->sym_types.clear();
  }

  // remove types from constants/objects
  for (index_type k = 0; k < dom_constants.length(); k++) {
    dom_constants[k]->sym_types.clear();
  }

  // untype actions
  for (index_type k = 0; k < dom_actions.length(); k++)
    dom_actions[k]->untype(this);

  // untype DKEL items
  for (index_type k = 0; k < dom_sc_invariants.length(); k++)
    dom_sc_invariants[k]->untype(this);
  for (index_type k = 0; k < dom_f_invariants.length(); k++)
    dom_f_invariants[k]->untype(this);
  for (index_type k = 0; k < dom_irrelevant.length(); k++)
    dom_irrelevant[k]->untype(this);

  dom_types.clear();
}

bool PDDL_Base::Goal::is_state()
{
  switch (g_class) {
  case goal_pos_atom:
  case goal_neg_atom:
  case goal_relation:
    return true;
  case goal_conjunction:
  case goal_disjunction:
    {
      ConjunctiveGoal* cg = (ConjunctiveGoal*)this;
      for (index_type k = 0; k < cg->goals.length(); k++)
	if (!cg->goals[k]->is_state()) return false;
      return true;
    }
  case goal_forall:
  case goal_exists:
    {
      QuantifiedGoal* qg = (QuantifiedGoal*)this;
      return qg->goal->is_state();
    }
  case goal_task:
  case goal_always:
  case goal_sometime:
  case goal_at_most_once:
  case goal_within:
  case goal_always_within:
  case goal_sometime_before:
  case goal_sometime_after:
    return false;
  default:
    std::cerr << "error: invalid goal class (" << g_class << ")" << std::endl;
    exit(255);
  }
}

bool PDDL_Base::Goal::is_propositional()
{
  switch (g_class) {
  case goal_pos_atom:
  case goal_neg_atom:
    return true;
  case goal_conjunction:
  case goal_disjunction:
    {
      ConjunctiveGoal* cg = (ConjunctiveGoal*)this;
      for (index_type k = 0; k < cg->goals.length(); k++)
	if (!cg->goals[k]->is_propositional()) return false;
      return true;
    }
  case goal_forall:
  case goal_exists:
    {
      QuantifiedGoal* qg = (QuantifiedGoal*)this;
      return qg->goal->is_state();
    }
  case goal_relation:
  case goal_task:
    return false;
  case goal_always:
  case goal_sometime:
  case goal_at_most_once:
    return ((SimpleSequenceGoal*)this)->constraint->is_propositional();
  case goal_within:
  case goal_always_within:
    return false;
  case goal_sometime_before:
  case goal_sometime_after:
    return (((TriggeredSequenceGoal*)this)->trigger->is_propositional() &&
	    ((TriggeredSequenceGoal*)this)->constraint->is_propositional());
  default:
    std::cerr << "error: invalid goal class (" << g_class << ")" << std::endl;
    exit(255);
  }
}

bool PDDL_Base::Goal::is_singular()
{
  switch (g_class) {
  case goal_pos_atom:
  case goal_neg_atom:
  case goal_relation:
    return true;
  case goal_conjunction:
  case goal_disjunction:
  case goal_forall:
  case goal_exists:
  case goal_task:
  case goal_always:
  case goal_sometime:
  case goal_at_most_once:
  case goal_within:
  case goal_always_within:
  case goal_sometime_before:
  case goal_sometime_after:
    return false;
  default:
    std::cerr << "error: invalid goal class (" << g_class << ")" << std::endl;
    exit(255);
  }
}

void PDDL_Base::Goal::print(std::ostream& s)
{
  switch (g_class) {
  case goal_pos_atom:
  case goal_neg_atom:
    ((AtomicGoal*)this)->print(s);
    break;
  case goal_relation:
    ((NumericGoal*)this)->print(s);
    break;
  case goal_conjunction:
    ((ConjunctiveGoal*)this)->print(s);
    break;
  case goal_disjunction:
    ((DisjunctiveGoal*)this)->print(s);
    break;
  case goal_forall:
  case goal_exists:
    ((QuantifiedGoal*)this)->print(s);
    break;
  case goal_task:
    ((TaskGoal*)this)->print(s);
    break;
  case goal_always:
  case goal_sometime:
  case goal_at_most_once:
    ((SimpleSequenceGoal*)this)->print(s);
    break;
  case goal_within:
    ((DeadlineGoal*)this)->print(s);
    break;
  case goal_always_within:
    ((TriggeredDeadlineGoal*)this)->print(s);
    break;
  case goal_sometime_before:
  case goal_sometime_after:
    ((TriggeredSequenceGoal*)this)->print(s);
    break;
  default:
    std::cerr << "error: invalid goal class (" << g_class << ")" << std::endl;
    exit(255);
  }
}

void PDDL_Base::AtomicGoal::print(std::ostream& s)
{
  atom->print(s, (g_class == goal_neg_atom));
}

void PDDL_Base::NumericGoal::print(std::ostream& s)
{
  rel->print(s, false);
}

void PDDL_Base::TaskGoal::print(std::ostream& s)
{
  task->print(s);
}

void PDDL_Base::ConjunctiveGoal::print(std::ostream& s)
{
  if (goals.length() > 1) s << "(and";
  for (index_type k = 0; k < goals.length(); k++) {
    s << " ";
    goals[k]->print(s);
  }
  if (goals.length() > 1) s << ")";
}

void PDDL_Base::DisjunctiveGoal::print(std::ostream& s)
{
  if (goals.length() > 1) s << "(or";
  for (index_type k = 0; k < goals.length(); k++) {
    s << " ";
    goals[k]->print(s);
  }
  if (goals.length() > 1) s << ")";
}

void PDDL_Base::QuantifiedGoal::print(std::ostream& s)
{
  if (g_class == goal_forall) {
    s << "(forall (";
  }
  else if (g_class == goal_exists) {
    s << "(exists (";
  }
  else {
    std::cerr << "error: invalid goal class " << g_class
	      << " for QuantifiedGoal" << std::endl;
    exit(255);
  }
  bool first = true;
  for (index_type k = 0; k < param.length(); k++) {
    if (!first) s << " ";
    s << param[k]->print_name;
    param[k]->sym_types.write_type(s);
    first = false;
  }
  s << ")";
  if (pos_con.length() + neg_con.length() > 0) {
    if (g_class == goal_forall) {
      s << "(imply";
      if (pos_con.length() + neg_con.length() > 1) s << " (and";
      for (index_type k = 0; k < pos_con.length(); k++) {
	s << " ";
	pos_con[k]->print(s);
      }
      for (index_type k = 0; k < neg_con.length(); k++) {
	s << " (not ";
	neg_con[k]->print(s);
	s << ")";
      }
      if (pos_con.length() + neg_con.length() > 1) s << ")";
      s << " ";
      goal->print(s);
      s << "))";
    }
    else {
      s << "(and";
      for (index_type k = 0; k < pos_con.length(); k++) {
	s << " ";
	pos_con[k]->print(s);
      }
      for (index_type k = 0; k < neg_con.length(); k++) {
	s << " (not ";
	neg_con[k]->print(s);
	s << ")";
      }
      s << " ";
      goal->print(s);
      s << "))";
    }
  }
  else {
    s << " ";
    goal->print(s);
    s << ")";
  }
}

void PDDL_Base::SimpleSequenceGoal::print(std::ostream& s)
{
  switch (g_class) {
  case goal_always:
    s << "(always ";
    break;
  case goal_sometime:
    s << "(sometime ";
    break;
  case goal_at_most_once:
    s << "(at-most-once ";
    break;
  default:
    std::cerr << "error: invalid goal class " << g_class
	      << " for SimpleSequenceGoal" << std::endl;
    exit(255);
  }
  constraint->print(s);
  s << ")";
}

void PDDL_Base::TriggeredSequenceGoal::print(std::ostream& s)
{
  switch (g_class) {
  case goal_sometime_before:
    s << "(sometime-before ";
    break;
  case goal_sometime_after:
    s << "(sometime-after ";
    break;
  default:
    std::cerr << "error: invalid goal class " << g_class << " for TriggeredSequenceGoal" << std::endl;
    exit(255);
  }
  trigger->print(s);
  s << " ";
  constraint->print(s);
  s << ")";
}

void PDDL_Base::DeadlineGoal::print(std::ostream& s)
{
  s << "(within " << PRINT_NTYPE(at) << " ";
  goal->print(s);
  s << ")";
}

void PDDL_Base::TriggeredDeadlineGoal::print(std::ostream& s)
{
  s << "(always-within " << PRINT_NTYPE(delay) << " ";
  trigger->print(s);
  s << " ";
  goal->print(s);
  s << ")";
}

void PDDL_Base::Symbol::print(std::ostream& s) const
{
  s << print_name;
}

void PDDL_Base::VariableSymbol::print(std::ostream& s)
{
  s << print_name;
  sym_types.write_type(s);
  if (binding) {
    s << " :binding ";
    binding->print(s);
  }
}

void PDDL_Base::TypeSymbol::print(std::ostream& s) const
{
  s << "(:type " << print_name;
  sym_types.write_type(s);
  s << "):";
  if (is_base_type) s << " basetype";
  s << " {";
  for (index_type k = 0; k < elements.length(); k++) {
    elements[k]->print(s);
    if (k < elements.length() - 1) s << ", ";
  }
  s << "}" << std::endl;
}

void PDDL_Base::PredicateSymbol::print(std::ostream& s)
{
  s << "(:predicate " << print_name;
  for (index_type k = 0; k < param.length(); k++) {
    s << " ";
    param[k]->print(s);
  }
  s << "):";
  if (is_static()) s << " static";
  if (pos_pre) s << " pos prec.";
  if (neg_pre) s << " neg prec.";
  if (added) s << " added";
  if (deleted) s << " deleted";
  if (locked) s << " locked";
  index_type c = init.count_values();
  element_vec* its = init.values();
  s << " init (" << c << ") = {";
  for (index_type k = 0; k < its->length(); k++) {
    if (k > 0) s << ' ';
    ((PDDL_Base::Atom*)((*its)[k]))->print(s);
  }
  delete its;
  s << "}" << std::endl;
}

void PDDL_Base::PredicateSymbol::write_prototype(std::ostream& s)
{
  s << "(" << print_name;
  for (index_type k = 0; k < param.length(); k++) {
    s << " ";
    param[k]->print(s);
  }
  s << ")";
}

void PDDL_Base::ObjectFunctionSymbol::print(std::ostream& s)
{
  s << "(:function " << print_name;
  for (index_type k = 0; k < param.length(); k++) {
    s << " ";
    param[k]->print(s);
  }
  s << ")";
  sym_types.write_type(s);
  s << ": ";
  if (is_static()) s << " static";
  index_type c = init.count_values();
  element_vec* its = init.values();
  s << " init (" << c << ") = {";
  for (index_type k = 0; k < its->length(); k++) {
    if (k > 0) s << ' ';
    ((PDDL_Base::FInitAtom*)((*its)[k]))->print(s);
  }
  delete its;
  s << "}" << std::endl;
}

void PDDL_Base::FunctionSymbol::print(std::ostream& s)
{
  s << "(:function " << print_name;
  for (index_type k = 0; k < param.length(); k++) {
    s << " ";
    param[k]->print(s);
  }
  s << "):";
  if (is_static()) s << " static";
  if (increased) s << " increased";
  if (decreased) s << " decreased";
  if (assigned) s << " assigned";
  if (borrowed) s << " borrowed";
  if (linear) s << " linear";
  if (integral) s << " integral";
  index_type c = init.count_values();
  element_vec* its = init.values();
  s << " init (" << c << ") = {";
  for (index_type k = 0; k < its->length(); k++) {
    if (k > 0) s << ' ';
    ((PDDL_Base::FInitAtom*)((*its)[k]))->print(s);
  }
  delete its;
  s << "}" << std::endl;
}

void PDDL_Base::Expression::print_sum(std::ostream& s, bool grnd)
{
  if (exp_class == exp_add) {
    BinaryExpression* bexp = (BinaryExpression*)this;
    bexp->first->print_sum(s, grnd);
    s << ' ';
    bexp->second->print_sum(s, grnd);
  }
  else {
    print(s, grnd);
  }
}

void PDDL_Base::Expression::print_product(std::ostream& s, bool grnd)
{
  if (exp_class == exp_mul) {
    BinaryExpression* bexp = (BinaryExpression*)this;
    bexp->first->print_product(s, grnd);
    s << ' ';
    bexp->second->print_product(s, grnd);
  }
  else {
    print(s, grnd);
  }
}

void PDDL_Base::Expression::print(std::ostream& s, bool grnd)
{
  switch (exp_class) {
  case exp_fun:
    {
      if (Instance::write_PDDL2) {
	FunctionExpression* fexp = (FunctionExpression*)this;
	s << '(' << fexp->fun->print_name;
	if (fexp->args) fexp->args->print(s, grnd);
	s << ')';
      }
      else {
	s << "0";
      }
    }
    return;
  case exp_list:
    {
      ListExpression* lexp = (ListExpression*)this;
      if ((lexp->sym->sym_class == sym_variable) &&
	  (((VariableSymbol*)lexp->sym)->value != 0) &&
	  grnd) {
	s << ' ' << ((VariableSymbol*)lexp->sym)->value->print_name;
      }
      else {
	s << ' ' << lexp->sym->print_name;
      }
      if (lexp->rest) lexp->rest->print(s, grnd);
    }
    return;
  case exp_time:
    {
      TimeExpression* texp = (TimeExpression*)this;
      if (texp->time_exp == 0) {
	s << "(total-time)";
      }
      else {
	texp->time_exp->print(s, grnd);
      }
    }
    return;
  case exp_const:
    {
      ConstantExpression* cexp = (ConstantExpression*)this;
      s << PRINT_NTYPE(cexp->val);
    }
    return;
  case exp_add:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      s << "(+ ";
      if (print_nary) {
	bexp->first->print_sum(s, grnd);
      }
      else {
	bexp->first->print(s, grnd);
      }
      s << ' ';
      if (print_nary) {
	bexp->second->print_sum(s, grnd);
      }
      else {
	bexp->second->print(s, grnd);
      }
      s << ')';
    }
    return;
  case exp_sub:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      s << "(- ";
      bexp->first->print(s, grnd);
      s << ' ';
      bexp->second->print(s, grnd);
      s << ')';
    }
    return;
  case exp_mul:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      s << "(* ";
      if (print_nary) {
	bexp->first->print_product(s, grnd);
      }
      else {
	bexp->first->print(s, grnd);
      }
      s << ' ';
      if (print_nary) {
	bexp->second->print_product(s, grnd);
      }
      else {
	bexp->second->print(s, grnd);
      }
      s << ')';
    }
    return;
  case exp_div:
    {
      BinaryExpression* bexp = (BinaryExpression*)this;
      s << "(/ ";
      bexp->first->print(s, grnd);
      s << ' ';
      bexp->second->print(s, grnd);
      s << ')';
    }
    return;
  case exp_preference:
    {
      if (Instance::write_PDDL3) {
	PreferenceExpression* pexp = (PreferenceExpression*)this;
	s << "(is-violated ";
	pexp->name->print(s);
	s << ")";
      }
      else {
	s << "0";
      }
    }
    return;
  }
}

void PDDL_Base::Relation::print(std::ostream& s, bool grnd)
{
  switch (at) {
  case md_start:
    s << "(at start ";
    break;
  case md_end:
    s << "(at end ";
    break;
  case md_all:
    s << "(over all ";
    break;
  case md_init:
    s << "(:init ";
    break;
  case md_pos_goal:
    s << "(:goal ";
    break;
  case md_neg_goal:
    s << "(:goal (not ";
    break;
  }
  s << "(";
  switch (rel) {
  case rel_equal:
    s << "= ";
    break;
  case rel_greater:
    s << "> ";
    break;
  case rel_greater_equal:
    s << ">= ";
    break;
  case rel_less:
    s << "< ";
    break;
  case rel_less_equal:
    s << "<= ";
    break;
  }
  first->print(s, grnd);
  s << " ";
  second->print(s, grnd);
  s << ")";
  switch (at) {
  case md_start:
  case md_end:
  case md_all:
  case md_init:
  case md_pos_goal:
    s << ")";
    break;
  case md_neg_goal:
    s << "))";
    break;
  }
}

void PDDL_Base::ActionSymbol::print(std::ostream& s)
{
  s << "(:action " << print_name << std::endl;
  s << "  :parameters (";
  for (index_type k = 0; k < param.length(); k++) {
    if (k > 0) s << ' ';
    param[k]->print(s);
  }
  s << ")" << std::endl;
  if (part) {
    s << "  :set ";
    part->print(s);
    s << std::endl;
  }
  if (assoc) {
    s << "  :assoc \"" << assoc << "\"" << std::endl;
  }
  s << "  :pos_prec (";
  for (index_type k = 0; k < pos_pre.length(); k++) {
    pos_pre[k]->print(s);
    if (k + 1 < pos_pre.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :neg_prec (";
  for (index_type k = 0; k < neg_pre.length(); k++) {
    neg_pre[k]->print(s);
    if (k + 1 < neg_pre.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :set_prec (";
  for (index_type k = 0; k < set_pre.length(); k++) {
    set_pre[k]->print(s);
    if (k + 1 < set_pre.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :dis_prec (";
  for (index_type k = 0; k < dis_pre.length(); k++) {
    dis_pre[k]->print_as_disjunction(s);
    if (k + 1 < dis_pre.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :num_prec (";
  for (index_type k = 0; k < num_pre.length(); k++) {
    num_pre[k]->print(s, false);
    if (k + 1 < num_pre.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :add (";
  for (index_type k = 0; k < adds.length(); k++) {
    adds[k]->print(s);
    if (k + 1 < adds.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :del (";
  for (index_type k = 0; k < dels.length(); k++) {
    dels[k]->print(s);
    if (k + 1 < dels.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :lock (";
  for (index_type k = 0; k < locks.length(); k++) {
    locks[k]->print(s);
    if (k + 1 < locks.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :enable (";
  for (index_type k = 0; k < enables.length(); k++) {
    enables[k]->print(s);
    if (k + 1 < enables.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :cons - " << cons.length() << " - (";
  for (index_type k = 0; k < cons.length(); k++) {
    cons[k]->print(s);
    if (k + 1 < cons.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :require (";
  for (index_type k = 0; k < reqs.length(); k++) {
    reqs[k]->print(s);
    if (k + 1 < reqs.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :increase (";
  for (index_type k = 0; k < incs.length(); k++) {
    if (k > 0) s << " ";
    incs[k]->print(s);
  }
  for (index_type k = 0; k < qc_incs.length(); k++) {
    if ((k > 0) || (incs.length() > 0)) s << " ";
    qc_incs[k]->print(s);
  }
  s << ")" << std::endl;
  s << "  :decrease (";
  for (index_type k = 0; k < decs.length(); k++) {
    if (k > 0) s << " ";
    decs[k]->print(s);
  }
  for (index_type k = 0; k < qc_decs.length(); k++) {
    if ((k > 0) || (decs.length() > 0)) s << " ";
    qc_decs[k]->print(s);
  }
  s << ")" << std::endl;
  s << "  :assign (";
  for (index_type k = 0; k < fass.length(); k++) {
    if (k > 0) s << " ";
    fass[k]->print(s);
  }
  for (index_type k = 0; k < qc_fass.length(); k++) {
    if ((k > 0) || (fass.length() > 0)) s << " ";
    qc_fass[k]->print(s);
  }
  s << ")" << std::endl;
  s << "  :set_effects (";
  for (index_type k = 0; k < set_eff.length(); k++) {
    set_eff[k]->print(s);
    if (k + 1 < set_eff.length()) s << " ";
  }
  s << ")" << std::endl;
  s << "  :cond_effects (";
  for (index_type k = 0; k < cond_eff.length(); k++) {
    cond_eff[k]->print(s);
    if (k + 1 < cond_eff.length()) s << " ";
  }
  s << ")" << std::endl;
  if (dmin) {
    s << "  :dmin ";
    dmin->print(s, false);
    s << std::endl;
  }
  if (dmax) {
    s << "  :dmax ";
    dmax->print(s, false);
    s << std::endl;
  }
  for (index_type k = 0; k < exps.length(); k++) {
    s << "  (:expansion " << std::endl;
    exps[k]->print(s);
    s << "  )" << std::endl;
  }
  if (irr_ins.length() > 0) {
    s << "%% irrelevant instances" << std::endl;
    for (index_type k = 0; k < irr_ins.length(); k++) {
      irr_ins[k]->print(s);
    }
    s << "%%" << std::endl;
  }
  s << "  :refs";
  for (index_type k = 0; k < refs.length(); k++) {
    s << " ";
    refs[k]->print(s);
  }
  s << ")" << std::endl;
}

void PDDL_Base::ActionSymbol::write_prototype(std::ostream& s)
{
  s << "(" << print_name;
  for (index_type k = 0; k < param.length(); k++) {
    if (k > 0) s << ' ';
    param[k]->print(s);
  }
  s << ")";
}

void PDDL_Base::SetName::print(std::ostream& s)
{
  s << "(" << sym->print_name;
  PDDL_Base::AtomBase::print(s);
  s << ")";
}

void PDDL_Base::AtomBase::print(std::ostream& s) const
{
  for (index_type k = 0; k < param.length(); k++) {
    s << ' ';
    if (param[k]->sym_class == sym_variable) {
      VariableSymbol* v = (VariableSymbol*)param[k];
      if (v->binding != 0) {
	v->binding->print(s);
      }
      else {
	s << v->print_name;
      }
    }
    else {
      s << param[k]->print_name;
    }
    if (print_bindings) {
      if (param[k]->sym_class == sym_variable) {
	VariableSymbol* v = (VariableSymbol*)param[k];
	if (v->value)
	  v->value->print(s << " = ");
	else
	  s << " = ??";
      }
    }
  }
}

void PDDL_Base::SetName::print_instance(std::ostream& s)
{
  s << "(" << sym->print_name;
  PDDL_Base::AtomBase::print_instance(s);
  s << ")";
}

void PDDL_Base::AtomBase::print_instance(std::ostream& s)
{
  for (index_type k = 0; k < param.length(); k++) {
    s << ' ';
    if (param[k]->sym_class == sym_variable) {
      VariableSymbol* v = (VariableSymbol*)param[k];
      if (v->value)
	v->value->print(s);
      else
	s << "?";
    }
    else {
      param[k]->print(s);
    }
  }
}

void PDDL_Base::Atom::print(std::ostream& s, bool neg) const
{
  if (Instance::write_PDDL2) {
    switch (at) {
    case md_start:
      s << "(at start ";
      break;
    case md_end:
      s << "(at end ";
      break;
    case md_all:
      s << "(over all ";
      break;
    }
  }
  if (neg) s << "(not ";
  if (Instance::write_DKEL) {
    switch (at) {
    case md_init:
      s << "(:init ";
      break;
    case md_pos_goal:
      s << "(:goal ";
      break;
    case md_neg_goal:
      s << "(:goal (not ";
      break;
    }
  }
  s << "(" << pred->print_name;
  PDDL_Base::AtomBase::print(s);
  s << ")";
  if (Instance::write_DKEL) {
    switch (at) {
    case md_init:
    case md_pos_goal:
      s << ")";
      break;
    case md_neg_goal:
      s << "))";
    }
  }
  if (neg) s << ")";
  if (Instance::write_PDDL2) {
    switch (at) {
    case md_start:
    case md_end:
    case md_all:
      s << ")";
    }
  }
}

void PDDL_Base::FTerm::print(std::ostream& s) const
{
  s << "(" << fun->print_name;
  PDDL_Base::AtomBase::print(s);
  s << ")";
}

void PDDL_Base::FInitAtom::print(std::ostream& s)
{
  s << "(= (" << fun->print_name;
  PDDL_Base::AtomBase::print(s);
  s << ") " << val << ")";
}

void PDDL_Base::OInitAtom::print(std::ostream& s)
{
  s << "(= (" << fun->print_name;
  PDDL_Base::AtomBase::print(s);
  s << ") " << val->print_name << ")";
}

void PDDL_Base::FChangeAtom::print(std::ostream& s)
{
  switch (at) {
  case md_start:
    s << "(at start ";
    break;
  case md_end:
    s << "(at end ";
    break;
  case md_all:
    s << "(over all ";
    break;
  }
  s << "((" << fun->print_name;
  PDDL_Base::AtomBase::print(s);
  s << ") . ";
  val->print(s, false);
  s << ")";
  if (at != md_none) s << ")";
}

void PDDL_Base::Reference::print(std::ostream& s)
{
  s << '(' << name->print_name;
  PDDL_Base::AtomBase::print(s);
  s << ')';
}

void PDDL_Base::SimpleReferenceSet::print(std::ostream& s)
{
  if (param.length() > 0) s << "(setof";
  PDDL_Base::Context::print(s);
  ref->print(s);
  if (param.length() > 0) s << ")";
}

void PDDL_Base::ReferenceSet::print(std::ostream& s)
{
  s << "(:set";
  if (name) {
    s << " :name" << name->print_name;
  }
  PDDL_Base::Context::print(s);
  for (index_type k = 0; k < refs.length(); k++) {
    s << " ";
    refs[k]->print(s);
  }
  s << ")";
}

void PDDL_Base::SequentialTaskNet::print(std::ostream& s)
{
  s << "(:expansion" << std::endl;
  PDDL_Base::Context::print(s);
  s << "  :tasks (";
  for (index_type k = 0; k < tasks.length(); k++) {
    tasks[k]->print(s);
    if (k + 1 < tasks.length()) s << std::endl;
  }
  s << "))" << std::endl;
}

void PDDL_Base::TypeConstraint::print(std::ostream& s)
{
  s << "( " << typ->print_name << " ";
  if (var->binding != 0) {
    var->binding->print(s);
  }
  else {
    s << var->print_name;
  }
  s << ")";
}

void PDDL_Base::Context::print(std::ostream& s)
{
  if (param.length() > 0) {
    s << "  :vars (";
    for (index_type k = 0; k < param.length(); k++) {
      if (k > 0) s << ' ';
      param[k]->print(s);
    }
    s << ")" << std::endl;
  }
  if (pos_con.length() > 0) {
    s << "  :pos_context (";
    for (index_type k = 0; k < pos_con.length(); k++) {
      pos_con[k]->print(s);
      if (k + 1 < pos_con.length()) s << " ";
    }
    s << ")" << std::endl;
  }
  if (neg_con.length() > 0) {
    s << "  :neg_context (";
    for (index_type k = 0; k < neg_con.length(); k++) {
      neg_con[k]->print(s);
      if (k + 1 < neg_con.length()) s << " ";
    }
    s << ")" << std::endl;
  }
  if (type_con.length() > 0) {
    s << "  :type_con (";
    for (index_type k = 0; k < type_con.length(); k++) {
      type_con[k]->print(s);
      if (k + 1 < type_con.length()) s << " ";
    }
    s << ")" << std::endl;
  }
}

void PDDL_Base::Context::print_assignment(std::ostream& s)
{
  for (index_type k = 0; k < param.length(); k++) {
    if (k > 0) s << ", ";
    param[k]->print(s);
    s << " = ";
    if (param[k]->value) {
      param[k]->value->print(s);
    }
    else {
      s << "?";
    }
  }
}

void PDDL_Base::DKEL_Item::print_begin(std::ostream& s)
{
  s << '(' << item_name << std::endl;
  if (name) {
    s << "  :name " << name->print_name << std::endl;
  }
  for (index_type k = 0; k < item_tags.length(); k++) {
    s << "  :tag " << item_tags[k] << std::endl;
  }
  Context::print(s);
}

void PDDL_Base::DKEL_Item::write_dkel(std::ostream& s)
{
  s << " (" << item_name << std::endl;
  if (name) {
    s << "  :name " << name->print_name << std::endl;
  }
  for (index_type k = 0; k < item_tags.length(); k++) {
    s << "  :tag " << item_tags[k] << std::endl;
  }
  if (param.length() > 0) {
    s << "  :vars (";
    for (index_type k = 0; k < param.length(); k++) {
      if (k > 0) s << ' ';
      param[k]->print(s);
    }
    s << ")" << std::endl;
  }
  if ((pos_con.length() + neg_con.length()) > 0) {
    s << "  :context (and";
    for (index_type k = 0; k < pos_con.length(); k++) {
      s << " ";
      pos_con[k]->print(s, false);
    }
    for (index_type k = 0; k < neg_con.length(); k++) {
      s << " ";
      neg_con[k]->print(s, true);
    }
    s << ")" << std::endl;
  }
}

void PDDL_Base::DKEL_Item::print_end(std::ostream& s)
{
  s << ")" << std::endl;
}

void PDDL_Base::DKEL_Item::print(std::ostream& s)
{
  print_begin(s);
  print_end(s);
}

void PDDL_Base::SetOf::print(std::ostream& s)
{
  s << "(setof :vars (";
  for (index_type k = 0; k < param.length(); k++) {
    if (k > 0) s << ' ';
    param[k]->print(s);
  }
  s << ")";
  if (pos_con.length() + neg_con.length() > 0) {
    s << ":context";
    if (pos_con.length() + neg_con.length() > 1) s << " (and";
    for (index_type k = 0; k < pos_con.length(); k++) {
      s << " ";
      pos_con[k]->print(s);
    }
    for (index_type k = 0; k < neg_con.length(); k++) {
      s << " (not ";
      neg_con[k]->print(s);
      s << ")";
    }
    if (pos_con.length() + neg_con.length() > 1) s << ")";
  }
  if (pos_atoms.length() + neg_atoms.length() > 1) s << " (and";
  for (index_type k = 0; k < pos_atoms.length(); k++) {
    s << " ";
    pos_atoms[k]->print(s);
  }
  for (index_type k = 0; k < neg_atoms.length(); k++) {
    s << " ";
    neg_atoms[k]->print(s);
  }
  if (pos_atoms.length() + neg_atoms.length() > 1) s << ")";
  s << ")";
}

void PDDL_Base::SetOf::print_as_disjunction(std::ostream& s)
{
  if (param.length() > 0) {
    s << "(exists (";
    for (index_type k = 0; k < param.length(); k++) {
      if (k > 0) s << ' ';
      param[k]->print(s);
    }
    s << ") ";
  }
  if (pos_con.length() + neg_con.length() > 0) {
    s << "(and ";
    for (index_type k = 0; k < pos_con.length(); k++) {
      s << " ";
      pos_con[k]->print(s);
    }
    for (index_type k = 0; k < neg_con.length(); k++) {
      s << " (not ";
      neg_con[k]->print(s);
      s << ")";
    }
    s << " ";
  }
  if (pos_atoms.length() + neg_atoms.length() > 1) s << "(or ";
  for (index_type k = 0; k < pos_atoms.length(); k++) {
    if (k > 0) s << " ";
    pos_atoms[k]->print(s);
  }
  for (index_type k = 0; k < neg_atoms.length(); k++) {
    if ((k > 0) || (pos_atoms.length() > 0)) s << " ";
    neg_atoms[k]->print(s);
  }
  if (pos_atoms.length() + neg_atoms.length() > 1) s << ")";
  if (pos_con.length() + neg_con.length() > 0) s << ")";
  if (param.length() > 0) s << ")";
}

void PDDL_Base::QCNumericEffect::print(std::ostream& s)
{
  s << "(setof :vars (";
  for (index_type k = 0; k < param.length(); k++) {
    if (k > 0) s << ' ';
    param[k]->print(s);
  }
  s << ") ";
  if (pos_con.length() + neg_con.length() > 0) {
    s << ":context";
    if (pos_con.length() + neg_con.length() > 1) s << " (and";
    for (index_type k = 0; k < pos_con.length(); k++) {
      s << " ";
      pos_con[k]->print(s);
    }
    for (index_type k = 0; k < neg_con.length(); k++) {
      s << " (not ";
      neg_con[k]->print(s);
      s << ")";
    }
    if (pos_con.length() + neg_con.length() > 1) s << ")";
    s << " ";
  }
  atom->print(s);
  s << ")";
}

void PDDL_Base::SetConstraint::print(std::ostream& s)
{
  PDDL_Base::DKEL_Item::print_begin(s);
  s << "  :set-constraint (";
  switch (sc_type) {
  case sc_at_most:
    s << "at-most-n";
    break;
  case sc_at_least:
    s << "at-least-n";
    break;
  case sc_exactly:
    s << "exactly-n";
    break;
  }
  s << " " << sc_count;
  for (index_type k = 0; k < pos_atoms.length(); k++) {
    s << " ";
    pos_atoms[k]->print(s);
  }
  for (index_type k = 0; k < neg_atoms.length(); k++) {
    s << " (not ";
    neg_atoms[k]->print(s);
    s << ")";
  }
  for (index_type k = 0; k < atom_sets.length(); k++) {
    s << " ";
    atom_sets[k]->print(s);
  }
  s << ")" << std::endl;
  PDDL_Base::DKEL_Item::print_end(s);  
}

void PDDL_Base::SetConstraint::write_dkel(std::ostream& s)
{
  PDDL_Base::DKEL_Item::write_dkel(s);
  s << "  :set-constraint (";
  switch (sc_type) {
  case sc_at_most:
    s << "at-most-n";
    break;
  case sc_at_least:
    s << "at-least-n";
    break;
  case sc_exactly:
    s << "exactly-n";
    break;
  }
  s << " " << sc_count;
  for (index_type k = 0; k < pos_atoms.length(); k++) {
    s << " ";
    pos_atoms[k]->print(s);
  }
  for (index_type k = 0; k < neg_atoms.length(); k++) {
    s << " (not ";
    neg_atoms[k]->print(s);
    s << ")";
  }
  for (index_type k = 0; k < atom_sets.length(); k++) {
    s << " ";
    atom_sets[k]->print(s);
  }
  s << "))" << std::endl;
}

void PDDL_Base::InvariantFormula::write_dkel(std::ostream& s)
{
  PDDL_Base::DKEL_Item::write_dkel(s);
  s << "  :formula ";
  f->print(s);
  s << ")" << std::endl;
}

void PDDL_Base::IrrelevantItem::print(std::ostream& s)
{
  PDDL_Base::DKEL_Item::print_begin(s);
  if (entity->name->sym_class == sym_predicate)
    s << "  :fact ";
  else
    s << "  :action ";
  entity->print(s);
  PDDL_Base::DKEL_Item::print_end(s);  
}

void PDDL_Base::IrrelevantItem::write_dkel(std::ostream& s)
{
  PDDL_Base::DKEL_Item::write_dkel(s);
  if (entity->name->sym_class == sym_predicate)
    s << "  :fact ";
  else
    s << "  :action ";
  entity->print(s);
  s << ")" << std::endl;
}

void PDDL_Base::Preference::print(std::ostream& s)
{
  s << "(preference ";
  if (name) {
    name->print(s);
    s << " ";
  }
  goal->print(s);
  s << ")";
}

void PDDL_Base::InputPlan::print(std::ostream& s)
{
  s << "(:plan";
  if (name) {
    s << std::endl << "  :name ";
    name->print(s);
  }
  if (is_opt) {
    s << std::endl << "  :opt";
  }
  for (index_type k = 0; k < steps.length(); k++) {
    s << std::endl << "  " << PRINT_NTYPE(steps[k]->start_time) << " : ";
    steps[k]->act->print(s);
    s << " ; " << steps[k]->act->index;
  }
  s << std::endl << ")" << std::endl;
}

void PDDL_Base::print(std::ostream& s)
{
  s << "domain: " << (domain_name ? domain_name : "<not defined>") << std::endl;
  s << "problem: " << (problem_name ? problem_name : "<not defined>") << std::endl;

  s << "<" << dom_predicates.length() << "," << dom_functions.length() << "," << dom_actions.length()
    << ">" << std::endl;

  dom_top_type->print(s);
  for (index_type k = 0; k < dom_types.length(); k++) dom_types[k]->print(s);
  for (index_type k = 0; k < dom_predicates.length(); k++) dom_predicates[k]->print(s);
  for (index_type k = 0; k < dom_object_functions.length(); k++) dom_object_functions[k]->print(s);
  for (index_type k = 0; k < dom_functions.length(); k++) dom_functions[k]->print(s);
  for (index_type k = 0; k < dom_actions.length(); k++) dom_actions[k]->print(s);

  for (index_type k = 0; k < dom_sc_invariants.length(); k++)
    dom_sc_invariants[k]->print(s);
  for (index_type k = 0; k < dom_f_invariants.length(); k++)
    dom_f_invariants[k]->print(s);

  s << "goals:";
  for (index_type k = 0; k < dom_goals.length(); k++) {
    s << " ";
    dom_goals[k]->print(s);
  }
  s << std::endl << "preferences:";
  for (index_type k = 0; k < dom_preferences.length(); k++) {
    s << " ";
    dom_preferences[k]->print(s);
  }
  s << std::endl;
}

void PDDL_Base::write_declarations(std::ostream& s)
{
  if (dom_types.length() > 0) {
    s << " (:types";
    for (index_type k = 0; k < dom_types.length(); k++) {
      s << " " << dom_types[k]->print_name;
      dom_types[k]->sym_types.write_type(s);
    }
    s << ")" << std::endl;
  }

  if (dom_predicates.length() > 0) {
    s << " (:predicates";
    for (index_type k = 0; k < dom_predicates.length(); k++) {
      s << " (" << dom_predicates[k]->print_name;
      for (index_type i = 0; i < dom_predicates[k]->param.length(); i++) {
	s << " " << dom_predicates[k]->param[i]->print_name;
	if (dom_types.length() > 0) {
	  dom_predicates[k]->param[i]->sym_types.write_type(s);
	}
      }
      s << ")";
    }
    s << ")" << std::endl;
  }

  if (Instance::write_PDDL2 && (dom_functions.length() > 0)) {
    s << " (:functions";
    for (index_type k = 0; k < dom_functions.length(); k++) {
      s << " (" << dom_functions[k]->print_name;
      for (index_type i = 0; i < dom_functions[k]->param.length(); i++) {
	s << " " << dom_functions[k]->param[i]->print_name;
	if (dom_types.length() > 0) {
	  dom_functions[k]->param[i]->sym_types.write_type(s);
	}
      }
      s << ")";
    }
    s << ")" << std::endl;
  }
}

void PDDL_Base::write_set_precondition(std::ostream& s, SetOf* set)
{
  // ugly hack: look at one atom to find mode of entire set
  mode_keyword m = md_none;
  if (set->pos_atoms.length() > 0)
    m = set->pos_atoms[0]->at;
  else if (set->neg_atoms.length() > 0)
    m = set->neg_atoms[0]->at;
  if (Instance::write_PDDL2) {
    switch (m) {
    case md_start:
      s << "(at start ";
      break;
    case md_end:
      s << "(at end ";
      break;
    case md_all:
      s << "(over all ";
      break;
    }
  }
  s << "(forall (";
  for (index_type j = 0; j < set->param.length(); j++) {
    s << " " << set->param[j]->print_name;
    if (dom_types.length() > 0) {
      set->param[j]->sym_types.write_type(s);
    }
  }
  s << ")";
  if (set->pos_con.length() + set->neg_con.length() + set->type_con.length() > 0) {
    s << " (imply (and";
    for (index_type j = 0; j < set->pos_con.length(); j++) {
      s << " ";
      set->pos_con[j]->print(s, false);
    }
    for (index_type j = 0; j < set->neg_con.length(); j++) {
      s << " ";
      set->neg_con[j]->print(s, true);
    }
    for (index_type j = 0; j < set->type_con.length(); j++) {
      s << " ";
      set->type_con[j]->print(s);
    }
    s << ")";
  }
  if (set->pos_atoms.length() + set->neg_atoms.length() > 1) s << " (and";
  for (index_type k = 0; k < set->pos_atoms.length(); k++) {
    s << " (" << set->pos_atoms[k]->pred->print_name;
    ((PDDL_Base::AtomBase*)set->pos_atoms[k])->print(s);
    s << ")";
  }
  for (index_type k = 0; k < set->neg_atoms.length(); k++) {
    s << " (not (" << set->neg_atoms[k]->pred->print_name;
    ((PDDL_Base::AtomBase*)set->neg_atoms[k])->print(s);
    s << "))";
  }
  if (set->pos_atoms.length() + set->neg_atoms.length() > 1) s << ")";
  if (set->pos_con.length() + set->neg_con.length() + set->type_con.length() > 0) s << ")";
  s << ")";
  if (Instance::write_PDDL2) {
    switch (m) {
    case md_start:
    case md_end:
    case md_all:
      s << ")";
    }
  }
}

void PDDL_Base::write_disjunctive_set_precondition(std::ostream& s, SetOf* set)
{
  // ugly hack: look at one atom to find mode of entire set
  mode_keyword m = md_none;
  if (set->pos_atoms.length() > 0)
    m = set->pos_atoms[0]->at;
  else if (set->neg_atoms.length() > 0)
    m = set->neg_atoms[0]->at;
  if (Instance::write_PDDL2) {
    switch (m) {
    case md_start:
      s << "(at start ";
      break;
    case md_end:
      s << "(at end ";
      break;
    case md_all:
      s << "(over all ";
      break;
    }
  }
  s << "(exists (";
  for (index_type j = 0; j < set->param.length(); j++) {
    s << " " << set->param[j]->print_name;
    if (dom_types.length() > 0) {
      set->param[j]->sym_types.write_type(s);
    }
  }
  s << ")";
  if (set->pos_con.length() + set->neg_con.length() + set->type_con.length() > 0) {
    s << " (and";
    for (index_type j = 0; j < set->pos_con.length(); j++) {
      s << " ";
      set->pos_con[j]->print(s, false);
    }
    for (index_type j = 0; j < set->neg_con.length(); j++) {
      s << " ";
      set->neg_con[j]->print(s, true);
    }
    for (index_type j = 0; j < set->type_con.length(); j++) {
      s << " ";
      set->type_con[j]->print(s);
    }
  }
  if (set->pos_atoms.length() + set->neg_atoms.length() > 1) s << " (or";
  for (index_type k = 0; k < set->pos_atoms.length(); k++) {
    s << " (" << set->pos_atoms[k]->pred->print_name;
    ((PDDL_Base::AtomBase*)set->pos_atoms[k])->print(s);
    s << ")";
  }
  for (index_type k = 0; k < set->neg_atoms.length(); k++) {
    s << " (not (" << set->neg_atoms[k]->pred->print_name;
    ((PDDL_Base::AtomBase*)set->neg_atoms[k])->print(s);
    s << "))";
  }
  if (set->pos_atoms.length() + set->neg_atoms.length() > 1) s << ")";
  if (set->pos_con.length() + set->neg_con.length() + set->type_con.length() > 0) s << ")";
  s << ")";
  if (Instance::write_PDDL2) {
    switch (m) {
    case md_start:
    case md_end:
    case md_all:
      s << ")";
    }
  }
}

void PDDL_Base::write_set_effect(std::ostream& s, SetOf* set)
{
  // ugly hack: look at one atom to find mode of entire set
  mode_keyword m = md_none;
  if (set->pos_atoms.length() > 0)
    m = set->pos_atoms[0]->at;
  else if (set->neg_atoms.length() > 0)
    m = set->neg_atoms[0]->at;
  if (Instance::write_PDDL2) {
    switch (m) {
    case md_start:
      s << "(at start ";
      break;
    case md_end:
      s << "(at end ";
      break;
    case md_all:
      s << "(over all ";
      break;
    }
  }
  if (set->param.length() > 0) {
    s << "(forall (";
    for (index_type j = 0; j < set->param.length(); j++) {
      s << " " << set->param[j]->print_name;
      if (dom_types.length() > 0) {
	set->param[j]->sym_types.write_type(s);
      }
    }
    s << ") ";
  }
  if (set->pos_con.length() + set->neg_con.length() + set->type_con.length() > 0) {
    s << "(when (and";
    for (index_type j = 0; j < set->pos_con.length(); j++) {
      s << " ";
      set->pos_con[j]->print(s, false);
    }
    for (index_type j = 0; j < set->neg_con.length(); j++) {
      s << " ";
      set->neg_con[j]->print(s, true);
    }
    for (index_type j = 0; j < set->type_con.length(); j++) {
      s << " ";
      set->type_con[j]->print(s);
    }
    s << ") ";
  }
  if (set->pos_atoms.length() + set->neg_atoms.length() > 1) s << "(and";
  for (index_type k = 0; k < set->pos_atoms.length(); k++) {
    s << " (" << set->pos_atoms[k]->pred->print_name;
    ((PDDL_Base::AtomBase*)set->pos_atoms[k])->print(s);
    s << ")";
  }
  for (index_type k = 0; k < set->neg_atoms.length(); k++) {
    s << " (not (" << set->neg_atoms[k]->pred->print_name;
    ((PDDL_Base::AtomBase*)set->neg_atoms[k])->print(s);
    s << "))";
  }
  if (set->pos_atoms.length() + set->neg_atoms.length() > 1) s << ")";
  if (set->pos_con.length() + set->neg_con.length() + set->type_con.length() > 0) s << ")";
  if (set->param.length() > 0) s << ")";
  if (Instance::write_PDDL2) {
    switch (m) {
    case md_start:
    case md_end:
    case md_all:
      s << ")";
    }
  }
}

void PDDL_Base::write_QCN_effect(std::ostream& s, const char* effect_type, QCNumericEffect* qcn)
{
  if (Instance::write_PDDL2) {
    switch (qcn->atom->at) {
    case md_start:
      s << "(at start ";
      break;
    case md_end:
      s << "(at end ";
      break;
    }
  }
  if (qcn->param.length() > 0) {
    s << "(forall (";
    for (index_type j = 0; j < qcn->param.length(); j++) {
      if (j > 0) s << " ";
      s << qcn->param[j]->print_name;
      if (dom_types.length() > 0) {
	qcn->param[j]->sym_types.write_type(s);
      }
    }
    s << ") ";
  }
  if (qcn->pos_con.length() + qcn->neg_con.length() + qcn->type_con.length() > 0) {
    s << "(when (and";
    for (index_type j = 0; j < qcn->pos_con.length(); j++) {
      s << " ";
      qcn->pos_con[j]->print(s, false);
    }
    for (index_type j = 0; j < qcn->neg_con.length(); j++) {
      s << " ";
      qcn->neg_con[j]->print(s, true);
    }
    for (index_type j = 0; j < qcn->type_con.length(); j++) {
      s << " ";
      qcn->type_con[j]->print(s);
    }
    s << ") ";
  }
  s << "(" << effect_type << " ";
  s << "(" << qcn->atom->fun->print_name;
  ((PDDL_Base::AtomBase*)qcn->atom)->print(s);
  s << ") ";
  qcn->atom->val->print(s, false);
  s << ")";
  if (qcn->pos_con.length() + qcn->neg_con.length() + qcn->type_con.length() > 0) s << ")";
  if (qcn->param.length() > 0) s << ")";
  if (Instance::write_PDDL2) {
    switch (qcn->atom->at) {
    case md_start:
    case md_end:
    case md_all:
      s << ")";
    }
  }
}

void PDDL_Base::write_action(std::ostream& s, ActionSymbol* act)
{
  bool is_durative = (Instance::write_PDDL2 && (act->dmin || act->dmax));
  index_type n_pre = (act->pos_pre.length() + act->neg_pre.length() +
		      act->set_pre.length() + act->dis_pre.length() +
		      (Instance::write_PDDL2 ? act->num_pre.length() : 0));
  index_type n_eff = (act->adds.length() + act->dels.length() +
		      act->set_eff.length() + act->cond_eff.length() +
		      (Instance::write_PDDL2 ? act->locks.length() : 0) +
		      (Instance::write_PDDL2 ? act->enables.length() : 0) +
		      (Instance::write_PDDL2 ? act->reqs.length() : 0) +
		      (Instance::write_PDDL2 ? act->incs.length() : 0) +
		      (Instance::write_PDDL2 ? act->decs.length() : 0) +
		      (Instance::write_PDDL2 ? act->fass.length() : 0) +
		      (Instance::write_PDDL2 ? act->qc_fass.length() : 0));

  if (is_durative)
    s << " (:durative-action " << act->print_name << std::endl;
  else
    s << " (:action " << act->print_name << std::endl;

  if ((act->param.length() > 0) ||
      Instance::always_write_parameters) {
    s << "  :parameters (";
    for (index_type i = 0; i < act->param.length(); i++) {
      s << " " << act->param[i]->print_name;
      if (dom_types.length() > 0) {
	act->param[i]->sym_types.write_type(s);
      }
    }
    s << ")" << std::endl;
  }

  if ((act->part != 0) && Instance::write_extra) {
    s << "  :set ";
    act->part->print(s);
    s << std::endl;
  }

  if (act->assoc && Instance::write_extra) {
    s << "  :assoc \"" << act->assoc << "\"" << std::endl;
  }

  if (Instance::write_PDDL2 && (act->dmin || act->dmax)) {
    if (act->dmin == act->dmax) {
      s << "  :duration (= ?duration ";
      act->dmax->print(s, false);
      s << ")" << std::endl;
    }
    else if (act->dmax) {
      if (act->dmin) {
	s << "  :duration (and (>= ?duration ";
	act->dmin->print(s, false);
	s << ") (<= ?duration ";
	act->dmax->print(s, false);
	s << "))" << std::endl;
      }
      else {
	s << "  :duration (<= ?duration ";
	act->dmax->print(s, false);
	s << ")" << std::endl;
      }
    }
    else if (act->dmin) {
      s << "  :duration (>= ?duration ";
      act->dmin->print(s, false);
      s << ")" << std::endl;
    }
  }

  if (n_pre > 0) {
    if (is_durative)
      s << "  :condition";
    else
      s << "  :precondition";
    if ((n_pre > 1) || Instance::always_write_conjunction) s << " (and";
    for (index_type i = 0; i < act->pos_pre.length(); i++) {
      s << " ";
      act->pos_pre[i]->print(s, false);
    }
    for (index_type i = 0; i < act->neg_pre.length(); i++) {
      s << " ";
      act->neg_pre[i]->print(s, true);
    }
    for (index_type i = 0; i < act->set_pre.length(); i++) {
      s << " ";
      write_set_precondition(s, act->set_pre[i]);
    }
    for (index_type i = 0; i < act->dis_pre.length(); i++) {
      s << " ";
      write_disjunctive_set_precondition(s, act->dis_pre[i]);
    }
    for (index_type i = 0; i < act->num_pre.length(); i++) {
      s << " ";
      act->num_pre[i]->print(s, false);
    }
    if ((n_pre > 1) || Instance::always_write_conjunction) s << ")";
    s << std::endl;
  }
  else if (Instance::always_write_precondition) {
    if (is_durative)
      s << "  :condition (";
    else
      s << "  :precondition (";
    if (Instance::always_write_conjunction)
      s << "and)";
    else
      s << ")";
  }

  if (n_eff > 0) {
    s << "  :effect ";
    if ((n_eff > 1) || Instance::always_write_conjunction) s << "(and";
    // s << std::endl << ";; adds" << std::endl;
    for (index_type i = 0; i < act->adds.length(); i++) {
      s << " ";
      act->adds[i]->print(s, false);
    }
    // s << std::endl << ";; dels" << std::endl;
    for (index_type i = 0; i < act->dels.length(); i++) {
      s << " ";
      act->dels[i]->print(s, true);
    }

    for (index_type i = 0; i < act->set_eff.length(); i++) {
      s << " ";
      write_set_effect(s, act->set_eff[i]);
    }
    for (index_type i = 0; i < act->cond_eff.length(); i++) {
      s << " ";
      write_set_effect(s, act->cond_eff[i]);
    }

    if (Instance::write_PDDL2) {
      // s << std::endl << ";; locks" << std::endl;
      for (index_type i = 0; i < act->locks.length(); i++) {
	s << " (at start (not (" << act->locks[i]->pred->print_name;
	((PDDL_Base::AtomBase*)act->locks[i])->print(s);
	s << ")))";
	s << " (at end (" << act->locks[i]->pred->print_name;
	((PDDL_Base::AtomBase*)act->locks[i])->print(s);
	s << "))";
      }
      // s << std::endl << ";; enables" << std::endl;
      for (index_type i = 0; i < act->enables.length(); i++) {
	s << " (at start (" << act->enables[i]->pred->print_name;
	((PDDL_Base::AtomBase*)act->enables[i])->print(s);
	s << "))";
	s << " (at end (not (" << act->enables[i]->pred->print_name;
	((PDDL_Base::AtomBase*)act->enables[i])->print(s);
	s << ")))";
      }
      // s << std::endl << ";; incs" << std::endl;
      for (index_type i = 0; i < act->incs.length(); i++) {
	s << " ";
	switch (act->incs[i]->at) {
	case md_start:
	  s << "(at start ";
	  break;
	case md_end:
	  s << "(at end ";
	  break;
	case md_all:
	  s << "(over all ";
	  break;
	}
	s << "(increase (" << act->incs[i]->fun->print_name;
	((PDDL_Base::AtomBase*)act->incs[i])->print(s);
	s << ") ";
	act->incs[i]->val->print(s, false);
	s << ")";
	if ((act->incs[i]->at == md_start) ||
	    (act->incs[i]->at == md_end) ||
	    (act->incs[i]->at == md_all))
	  s << ")";
      }
      // s << std::endl << ";; decs" << std::endl;
      for (index_type i = 0; i < act->decs.length(); i++) {
	s << " ";
	switch (act->decs[i]->at) {
	case md_start:
	  s << "(at start ";
	  break;
	case md_end:
	  s << "(at end ";
	  break;
	case md_all:
	  s << "(over all ";
	  break;
	}
	s << "(decrease (" << act->decs[i]->fun->print_name;
	((PDDL_Base::AtomBase*)act->decs[i])->print(s);
	s << ") ";
	act->decs[i]->val->print(s, false);
	s << ")";
	if ((act->decs[i]->at == md_start) ||
	    (act->decs[i]->at == md_end) ||
	    (act->decs[i]->at == md_all))
	  s << ")";
      }
      // s << std::endl << ";; fluent assignments" << std::endl;
      for (index_type i = 0; i < act->fass.length(); i++) {
	s << " ";
	switch (act->fass[i]->at) {
	case md_start:
	  s << "(at start ";
	  break;
	case md_end:
	  s << "(at end ";
	  break;
	case md_all:
	  s << "(over all ";
	  break;
	}
	s << "(assign (" << act->fass[i]->fun->print_name;
	((PDDL_Base::AtomBase*)act->fass[i])->print(s);
	s << ") ";
	act->fass[i]->val->print(s, false);
	s << ")";
	if ((act->fass[i]->at == md_start) ||
	    (act->fass[i]->at == md_end) ||
	    (act->fass[i]->at == md_all))
	  s << ")";
      }
      for (index_type i = 0; i < act->qc_fass.length(); i++) {
	s << " ";
	write_QCN_effect(s, "assign", act->qc_fass[i]);
      }
      // s << std::endl << ";; reqs" << std::endl;
      for (index_type i = 0; i < act->reqs.length(); i++) {
	s << " (at start (decrease (" << act->reqs[i]->fun->print_name;
	((PDDL_Base::AtomBase*)act->reqs[i])->print(s);
	s << ") ";
	act->reqs[i]->val->print(s, false);
	s << ")) (at end (increase (" << act->reqs[i]->fun->print_name;
	((PDDL_Base::AtomBase*)act->reqs[i])->print(s);
	s << ") ";
	act->reqs[i]->val->print(s, false);
	s << "))";
      }
    }
    if ((n_eff > 1) || Instance::always_write_conjunction) s << ")";
    s << std::endl;
  }
  else if (Instance::always_write_effect) {
    if (Instance::always_write_conjunction)
      s << "  :effect (and)" << std::endl;
    else
      s << "  :effect ()" << std::endl;
  }

  if (HTNInstance::write_HTN) {
    for (index_type i = 0; i < act->exps.length(); i++) {
      s << "  (:expansion" << std::endl;
      if (act->exps[i]->param.length() > 0) {
	s << "   :vars (";
	for (index_type j = 0; j < act->exps[i]->param.length(); j++) {
	  s << " " << act->exps[i]->param[j]->print_name;
	  if (dom_types.length() > 0) {
	    act->exps[i]->param[j]->sym_types.write_type(s);
	  }
	}
	s << ")" << std::endl;
      }
      if ((act->exps[i]->pos_con.length() + 
	   act->exps[i]->neg_con.length()) > 0) {
	s << "   :context (and";
	for (index_type j = 0; j < act->exps[i]->pos_con.length(); j++) {
	  s << " ";
	  act->exps[i]->pos_con[j]->print(s, false);
	}
	for (index_type j = 0; j < act->exps[i]->neg_con.length(); j++) {
	  s << " ";
	  act->exps[i]->neg_con[j]->print(s, true);
	}
	s << ")" << std::endl;
      }
      s << "   :tasks (";
      for (index_type j = 0; j < act->exps[i]->tasks.length(); j++) {
	act->exps[i]->tasks[j]->print(s);
      }
      s << "))" << std::endl;
    }
  }

  // end of action def
  s << " )" << std::endl;
}

void PDDL_Base::write_objects(std::ostream& s, bool defined_in_problem)
{
  index_type n_obj = 0;
  for (index_type k = 0; k < dom_constants.length(); k++)
    if (dom_constants[k]->defined_in_problem == defined_in_problem) n_obj += 1;

  if (n_obj > 0) {
    if (defined_in_problem) {
      s << " (:objects";
    }
    else {
      s << " (:constants";
    }
    for (index_type k = 0; k < dom_constants.length(); k++)
      if (dom_constants[k]->defined_in_problem == defined_in_problem) {
	s << " " << dom_constants[k]->print_name;
	if (dom_types.length() > 0) {
	  dom_constants[k]->sym_types.write_type(s);
	}
      }
    s << ")" << std::endl;
  }
}

void PDDL_Base::write_init(std::ostream& s)
{
  index_type n_init = dom_init.length();
  if (Instance::write_PDDL2) n_init += dom_fun_init.length();
  if (write_PDDL31) n_init += dom_obj_init.length();
  if (n_init > 0) {
    s << " (:init";
    for (index_type k = 0; k < dom_init.length(); k++) {
      s << " (" << dom_init[k]->pred->print_name;
      ((AtomBase*)dom_init[k])->print(s);
      s << ")";
    }
    if (write_PDDL31) {
      for (index_type k = 0; k < dom_obj_init.length(); k++) {
	s << " (= (" << dom_obj_init[k]->fun->print_name;
	((AtomBase*)dom_obj_init[k])->print(s);
	s << ") " << dom_obj_init[k]->val->print_name << ")";
      }
    }
    if (Instance::write_PDDL2) {
      for (index_type k = 0; k < dom_fun_init.length(); k++) {
	s << " (= (" << dom_fun_init[k]->fun->print_name;
	((AtomBase*)dom_fun_init[k])->print(s);
	s << ") " << PRINT_NTYPE(dom_fun_init[k]->val) << ")";
      }
    }
    s << ")" << std::endl;
  }
}

void PDDL_Base::write_goal(std::ostream& s)
{
  index_type n_in_goal = 0;
  for (index_type k = 0; k < dom_goals.length(); k++)
    if (dom_goals[k]->is_state())
      if (dom_goals[k]->is_propositional() || Instance::write_PDDL2)
	n_in_goal += 1;
  if (Instance::write_PDDL3)
    for (index_type k = 0; k < dom_preferences.length(); k++)
      if (dom_preferences[k]->is_state())
	if (dom_preferences[k]->is_propositional() || Instance::write_PDDL2)
	  n_in_goal += 1;

  if (n_in_goal > 0) {
    s << " (:goal";
    if (n_in_goal > 1) s << " (and";
    for (index_type k = 0; k < dom_goals.length(); k++) {
      if (dom_goals[k]->is_state()) {
	if (dom_goals[k]->is_propositional() || Instance::write_PDDL2) {
	  s << " ";
	  dom_goals[k]->print(s);
	}
      }
    }
    if (Instance::write_PDDL3) {
      for (index_type k = 0; k < dom_preferences.length(); k++) {
	if (dom_preferences[k]->is_state()) {
	  if (dom_preferences[k]->is_propositional() ||
	      Instance::write_PDDL2) {
	    s << " ";
	    dom_preferences[k]->print(s);
	  }
	}
      }
    }
    if (n_in_goal > 1) s << ")";
    s << ")" << std::endl;
  }

  if (Instance::write_PDDL3) {
    index_type n_in_cons = 0;
    for (index_type k = 0; k < dom_goals.length(); k++)
      if (!dom_goals[k]->is_state())
	if (dom_goals[k]->is_propositional() || Instance::write_PDDL2)
	  n_in_cons += 1;
    for (index_type k = 0; k < dom_preferences.length(); k++)
      if (!dom_preferences[k]->is_state())
	if (dom_preferences[k]->is_propositional() || Instance::write_PDDL2)
	  n_in_cons += 1;

    if (n_in_cons > 0) {
      s << " (:constraints";
      if (n_in_cons > 1) s << " (and";
      for (index_type k = 0; k < dom_goals.length(); k++) {
	if (!dom_goals[k]->is_state()) {
	  if (dom_goals[k]->is_propositional() || Instance::write_PDDL2) {
	    s << " ";
	    dom_goals[k]->print(s);
	  }
	}
      }
      for (index_type k = 0; k < dom_preferences.length(); k++) {
	if (!dom_preferences[k]->is_state()) {
	  if (dom_preferences[k]->is_propositional() ||
	      Instance::write_PDDL2) {
	    s << " ";
	    dom_preferences[k]->print(s);
	  }
	}
      }
      if (n_in_cons > 1) s << ")";
      s << ")" << std::endl;
    }
  }
}

void PDDL_Base::write_metric(std::ostream& s)
{
  if (Instance::write_metric &&
      (Instance::write_PDDL2 || Instance::write_PDDL3)) {
    switch (metric_type) {
    case metric_makespan:
      s << " (:metric minimize (total-time))" << std::endl;
      break;
    case metric_minimize:
      s << " (:metric minimize ";
      metric->print(s, false);
      s << ")" << std::endl;
      break;
    case metric_maximize:
      s << " (:metric maximize ";
      metric->print(s, false);
      s << ")" << std::endl;
      break;
    }
  }
}

void PDDL_Base::write_dkel_items(std::ostream& s, bool defined_in_problem)
{
  if (Instance::write_DKEL) {
    for (index_type k = 0; k < dom_irrelevant.length(); k++) {
      if ((dom_irrelevant[k]->
	   item_is_included(excluded_dkel_tags, required_dkel_tags)) &&
	  (dom_irrelevant[k]->defined_in_problem == defined_in_problem))
	dom_irrelevant[k]->write_dkel(s);
    }
    for (index_type k = 0; k < dom_sc_invariants.length(); k++) {
      if ((dom_sc_invariants[k]->
	   item_is_included(excluded_dkel_tags, required_dkel_tags)) &&
	  (dom_sc_invariants[k]->defined_in_problem == defined_in_problem))
	dom_sc_invariants[k]->write_dkel(s);
    }
    for (index_type k = 0; k < dom_f_invariants.length(); k++) {
      if ((dom_f_invariants[k]->
	   item_is_included(excluded_dkel_tags, required_dkel_tags)) &&
	  (dom_f_invariants[k]->defined_in_problem == defined_in_problem))
	dom_f_invariants[k]->write_dkel(s);
    }
  }
}

void PDDL_Base::write_domain_begin(std::ostream& s)
{
  s << "(define (domain " << domain_name << ")" << std::endl;
}

void PDDL_Base::write_problem_begin(std::ostream& s)
{
  s << "(define (problem " << problem_name << ")" << std::endl;
  if (domain_name) {
    s << " (:domain " << domain_name << ")" << std::endl;
  }
}

void PDDL_Base::write_end(std::ostream& s)
{
  s << ")" << std::endl;
}

void PDDL_Base::write_dkel_domain(std::ostream& s, bool leave_open)
{
  write_domain_begin(s);
  write_declarations(s);
  write_objects(s, false);
  for (index_type k = 0; k < dom_actions.length(); k++) {
    write_action(s, dom_actions[k]);
  }
  write_dkel_items(s, false);
  if (!leave_open) write_end(s);
}

void PDDL_Base::write_dkel_problem(std::ostream& s, bool leave_open)
{
  write_problem_begin(s);
  write_objects(s, true);
  write_init(s);
  write_goal(s);
  write_metric(s);
  write_dkel_items(s, true);
  if (!leave_open) write_end(s);
}

void PDDL_Base::write_plans(std::ostream& s)
{
  for (index_type k = 0; k < input_plans.length(); k++) {
    input_plans[k]->print(s);
  }
}

void PDDL_Base::write_heuristic_table(std::ostream& s)
{
  s << "(:heuristic";
  for (index_type k = 0; k < h_table.length(); k++) {
    s << std::endl << "(";
    for (index_type i = 0; i < h_table[k]->atoms.length(); i++)
      h_table[k]->atoms[i]->print(s);
    s << ") ";
    if (h_table[k]->opt) s << ":opt ";
    if (INFINITE(h_table[k]->cost)) s << ":inf";
    else s << h_table[k]->cost;
  }
  s << ")" << std::endl;
}

void PDDL_Base::write_sets(std::ostream& s)
{
  for (index_type k = 0; k < input_sets.length(); k++) {
    input_sets[k]->print(s);
    s << std::endl;
  }
}

PDDL_Base::Atom* PDDL_Base::make_atom_from_prop(ptr_pair& src, bool& neg)
{
  index_type i = dom_predicates.first((PredicateSymbol*)src.first);
  if (i == no_such_index) {
    std::cerr << "error in make_atom_from_prop: "
	      << src.first << " is not a predicate symbol in the domain"
	      << std::endl;
    exit(255);
  }
  ptr_table* ins = (ptr_table*)src.second;
  ptr_table::key_vec* args = ins->key_sequence();
  ptr_table* root = ins->root();
  if (root == &(dom_predicates[i]->pos_prop)) {
    neg = false;
  }
  else if (root == &(dom_predicates[i]->neg_prop)) {
    neg = true;
  }
  else {
    std::cerr << "error: root " << root << " of atom instance "
	      << ins << " does not equal pos. or neg. set of predicate "
	      << dom_predicates[i]->print_name
	      << std::endl;
    exit(255);
  }
  Atom* a = new Atom(dom_predicates[i]);
  for (index_type k = 0; k < dom_predicates[i]->param.length(); k++)
    a->param.append((PDDL_Base::Symbol*)((*args)[k + 1]));
  delete args;
  return a;
}

void InstanceName::write(std::ostream& s, unsigned int c) const
{
  if (context_is_domain(c)) write_string_escaped(s, domain_name, c);
  else if (context_is_problem(c)) write_string_escaped(s, problem_name, c);
  else {
    write_string_escaped(s, domain_name, c);
    write_string_escaped(s, "::", c);
    write_string_escaped(s, problem_name, c);
  }
}

const Name* InstanceName::cast_to(const char* cname) const
{
  if (strcmp(cname, "InstanceName") == 0) return this;
  return 0;
}

char PDDL_Name::catc = '_';

PDDL_Name::PDDL_Name
(PDDL_Base::Symbol* s, PDDL_Base::variable_vec a, bool n)
  : neg(n), sym(s), arg(0, a.length()), vis(s->visible), avis(true, a.length())
{
  for (index_type k = 0; k < a.length(); k++) {
    arg[k] = a[k]->value;
    avis[k] = a[k]->visible;
  }
}

void PDDL_Name::add(PDDL_Base::Symbol* s)
{
  arg.append(s);
  avis.append(s->visible);
}

void PDDL_Name::add(PDDL_Base::Symbol* s, bool v)
{
  arg.append(s);
  avis.append(v);
}

void PDDL_Name::write(std::ostream& s, unsigned int c) const
{
  if (context_is_instance(c)) {
    if (neg) {
      s << "not";
      write_char_escaped(s, catc, c);
    }
    write_string_escaped(s, sym->print_name, c);
    for (index_type k = 0; k < arg.length(); k++) {
      write_char_escaped(s, catc, c);
      write_string_escaped(s, arg[k]->print_name, c);
    }
  }
  else {
    if (context_is_plan(c) && conform_to_IPC(c) && !vis) return;
    if (neg) write_string_escaped(s, "(not ", c);
    write_char_escaped(s, '(', c);
    write_string_escaped(s, sym->print_name, c);
    for (index_type k = 0; k < arg.length(); k++) {
      bool do_print = true;
      if (context_is_plan(c) && conform_to_IPC(c)) {
	assert(avis.length() > k);
	if (!avis[k]) do_print = false;
      }
      if (do_print) {
	s << ' ';
	write_string_escaped(s, arg[k]->print_name, c);
      }
    }
    write_char_escaped(s, ')', c);
    if (neg) write_char_escaped(s, ')', c);
  }
}

const Name* PDDL_Name::cast_to(const char* cname) const
{
  if (strcmp(cname, "PDDL_Name") == 0) return this;
  return 0;
}

Numbered_PDDL_Name::Numbered_PDDL_Name(PDDL_Name* n, index_type c)
  : PDDL_Name(n->symbol(), n->args(), n->is_neg()), copy(c)
{
  // done
}

void Numbered_PDDL_Name::write(std::ostream& s, unsigned int c) const
{
  if (context_is_instance(c)) {
    if (neg) write_string_escaped(s, "not_", c);
    write_string_escaped(s, sym->print_name, c);
    for (index_type k = 0; k < arg.length(); k++) {
      write_char_escaped(s, '_', c);
      write_string_escaped(s, arg[k]->print_name, c);
    }
    write_char_escaped(s, '_', c);
    s << copy;
  }
  else {
    if (neg) write_string_escaped(s, "(not ", c);
    write_char_escaped(s, '(', c);
    write_string_escaped(s, sym->print_name, c);
    for (index_type k = 0; k < arg.length(); k++) {
      write_char_escaped(s, ' ', c);
      write_string_escaped(s, arg[k]->print_name, c);
    }
    write_char_escaped(s, ' ', c);
    if (!context_is_plan(c)) {
      write_char_escaped(s, '#', c);
      s << copy;
    }
    if (neg) write_char_escaped(s, ')', c);
  }
}

END_HSPS_NAMESPACE
