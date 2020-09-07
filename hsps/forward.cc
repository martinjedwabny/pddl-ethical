
#include "forward.h"
#include "base.h"

BEGIN_HSPS_NAMESPACE

bool SeqProgState::separate_update_actions = false;

SeqProgState::SeqProgState
(Instance& i, Heuristic& h, const ACF& c)
  : AtomSetState(i, h), cost(c), act(no_such_index), res(0)
{
  if (separate_update_actions) {
    apply_update_actions();
  }
}

SeqProgState::SeqProgState
(Instance& i, Heuristic& h, const ACF& c, const index_set& s)
  : AtomSetState(i, h, s), cost(c), act(no_such_index), res(0)
{
  if (separate_update_actions) {
    apply_update_actions();
  }
}

SeqProgState::SeqProgState
(Instance& i, Heuristic& h, const ACF& c, const bool_vec& s)
  : AtomSetState(i, h, s), cost(c), act(no_such_index), res(0)
{
  if (separate_update_actions) {
    apply_update_actions();
  }
}

SeqProgState::SeqProgState
(Instance& i, Heuristic& h, const ACF& c, BasicResourceState* r)
  : AtomSetState(i, h), cost(c), act(no_such_index), res(r)
{
  if (separate_update_actions) {
    apply_update_actions();
  }
}

SeqProgState::SeqProgState
(Instance& i, Heuristic& h, const ACF& c, const index_set& s,
 BasicResourceState* r)
  : AtomSetState(i, h, s), cost(c), act(no_such_index), res(r)
{
  if (separate_update_actions) {
    apply_update_actions();
  }
}

SeqProgState::SeqProgState
(Instance& i, Heuristic& h, const ACF& c, const bool_vec& s,
 BasicResourceState* r)
  : AtomSetState(i, h, s), cost(c), act(no_such_index), res(r)
{
  if (separate_update_actions) {
    apply_update_actions();
  }
}

SeqProgState::SeqProgState(const SeqProgState& s)
  : AtomSetState(s), cost(s.cost), act(s.act), res(0)
{
  if (s.res) res = s.res->copy();
}

SeqProgState::~SeqProgState()
{
  if (res) delete res;
}

bool SeqProgState::applicable(Instance::Action& a)
{
  bool app = true;
  for (index_type i = 0; (i < a.pre.length()) && app; i++)
    if (!set[a.pre[i]]) app = false;
  if (res) {
    if (app) {
      if (!res->applicable(a)) app = false;
    }
  }
  return app;
}

SeqProgState* SeqProgState::apply(Instance::Action& a)
{
  SeqProgState* s = (SeqProgState*)copy();
  for (index_type k = 0; k < a.add.length(); k++) {
    if (!s->set[a.add[k]]) s->size += 1;
    s->set[a.add[k]] = true;
  }
  for (index_type k = 0; k < a.del.length(); k++) {
    if (s->set[a.del[k]]) s->size -= 1;
    s->set[a.del[k]] = false;
  }
  if (separate_update_actions) {
    s->apply_update_actions();
  }
  s->State::set_predecessor(this);
  s->act = a.index;
  s->eval();
  if (s->res) {
    s->res->apply(a);
  }
  return s;
}

void SeqProgState::apply_update_actions()
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "state BEFORE applying update actions: " << *this << std::endl;
#endif
  bool done = false;
  while (!done) {
    done = true;
    for (index_type k = 0; k < instance.n_actions(); k++) {
      Instance::Action& a = instance.actions[k];
      if (is_update(a) && applicable(a)) {
	for (index_type i = 0; i < instance.actions[k].add.length(); i++) {
	  if (!set[instance.actions[k].add[i]]) done = false;
	  set[instance.actions[k].add[i]] = true;
	}
      }
    }
  }
  count();
#ifdef TRACE_PRINT_LOTS
  std::cerr << "state AFTER applying update actions: " << *this << std::endl;
#endif
}

void SeqProgState::set_predecessor(State* p)
{
  if (p == 0) {
    act = no_such_index;
    pre = 0;
    return;
  }
  SeqProgState* s = (SeqProgState*)p;
  for (index_type k = 0; k < instance.n_actions(); k++) {
    Instance::Action& a = instance.actions[k];
    if (a.sel && s->applicable(a)) {
      SeqProgState* new_s = s->apply(a);
      int d = compare(*new_s);
      delete new_s;
      if (d == 0) {
	act = k;
	pre = s;
	return;
      }
    }
  }
  std::cerr << "error in SeqProgState::set_predecessor: state " << *this
	    << " can not be a successor of " << *s << std::endl;
  exit(255);
}

bool SeqProgState::is_final()
{
  for (index_type k = 0; k < instance.goal_atoms.length(); k++)
    if (!set[instance.goal_atoms[k]]) return false;
  return true;
}

NTYPE SeqProgState::delta_cost() {
  if (act == no_such_index)
    return 0;
  else
    return cost(act);
}

NTYPE SeqProgState::expand(Search& s, NTYPE bound) {
  NTYPE c_min = POS_INF;
  for (index_type k = 0; k < instance.n_actions(); k++) {
    Instance::Action& a = instance.actions[k];
    if (a.sel && (!separate_update_actions || !is_update(a))) {
      if (applicable(a)) {
#ifdef TRACE_PRINT_LOTS
	std::cerr << depth() << ". action " << a.name << " is applicable"
		  << std::endl;
#endif
	SeqProgState* new_s = apply(a);
#ifdef TRACE_PRINT_LOTS
	std::cerr << depth() << ". successor state: " << *new_s << std::endl;
#endif
	if (FINITE(new_s->est) && (cost(a.index) + new_s->est <= bound)) {
	  NTYPE c_new =
	    cost(a.index) + s.new_state(*new_s, bound - cost(a.index));
	  if (s.solved()) {
	    if (size > max_set_size_encountered)
	      max_set_size_encountered = size;
	  }
	  if (s.done()) {
	    delete new_s;
	    return c_new;
	  }
	  else {
	    c_min = MIN(c_min, c_new);
	  }
	}
	else {
	  c_min = MIN(c_min, cost(a.index) + new_s->est);
	}
	delete new_s;
      }
    }
#ifdef TRACE_PRINT_LOTS
    else {
      std::cerr << depth() << ". action " << a.name
		<< " is non-selectable | update" << std::endl;
    }
#endif
  }
  return c_min;
}

void SeqProgState::store(NTYPE cost, bool opt) {
  if (res) {
    if (!res->is_root()) return;
  }
  heuristic.store(set, cost, opt);
}

void SeqProgState::insert(Plan& p) {
  if (act != no_such_index) {
    p.insert(act);
    for (index_type k = 0; k < instance.n_atoms(); k++)
      if (set[k] && !instance.actions[act].add.contains(k))
	p.protect(k);
    p.advance(instance.actions[act].dur);
  }
}

void SeqProgState::insert_path(Plan& p)
{
  if (predecessor()) {
    predecessor()->insert_path(p);
  }
  insert(p);
}

void SeqProgState::write_plan(std::ostream& s)
{
  if (act != no_such_index) {
    s << instance.actions[act].name;
  }
}

State* SeqProgState::copy() {
  return new SeqProgState(*this);
}

State* SeqProgState::new_state(const index_set& s, State* p) {
  SeqProgState* new_s =
    (res ? new SeqProgState(instance, heuristic, cost, s, res->new_state())
         : new SeqProgState(instance, heuristic, cost, s));
  new_s->State::set_predecessor(p);
  return new_s;
}

State* SeqProgState::new_state(const bool_vec& s, State* p) {
  SeqProgState* new_s =
    (res ? new SeqProgState(instance, heuristic, cost, s, res->new_state())
         : new SeqProgState(instance, heuristic, cost, s));
  new_s->State::set_predecessor(p);
  return new_s;
}

void SeqProgState::write(std::ostream& s) {
  instance.write_atom_set(s, set);
  if (res) {
    s << " (";
    res->write(s);
    s << ")";
  }
}

NTYPE SeqCProgState::expand(Search& s, NTYPE bound) {
  NTYPE c_min = POS_INF;
  for (index_type k = 0; k < instance.n_actions(); k++) {
    Instance::Action& a = instance.actions[k];
    if (a.sel && (!separate_update_actions || !is_update(a))) {
      bool app = applicable(a);
      if (app) {
#ifdef TRACE_PRINT_LOTS
	std::cerr << depth() << ". action " << instance.actions[k].name
		  << " is applicable" << std::endl;
#endif
	if ((act != no_such_index) && (act < a.index) &&
	    instance.commutative(act, a.index)) app = false;
#ifdef TRACE_PRINT_LOTS
	if (!app) {
	  std::cerr << depth() << ". commutativity cut: action "
		    << instance.actions[k].name << ", predecessor "
		    << instance.actions[act].name << std::endl;
	}
#endif
      }
      if (app) {
	SeqCProgState* new_s = (SeqCProgState*)apply(a);
#ifdef TRACE_PRINT_LOTS
	std::cerr << depth() << ". successor state: " << *new_s << std::endl;
#endif
	if (FINITE(new_s->est) && (cost(a.index) + new_s->est <= bound)) {
	  NTYPE c_new =
	    cost(a.index) + s.new_state(*new_s, bound - cost(a.index));
	  if (s.solved()) {
	    if (size > max_set_size_encountered)
	      max_set_size_encountered = size;
	  }
	  if (s.done()) {
	    delete new_s;
	    return c_new;
	  }
	  else {
	    c_min = MIN(c_min, c_new);
	  }
	}
	else {
	  c_min = MIN(c_min, cost(a.index) + new_s->est);
	}
	delete new_s;
      }
    }
  }
  return c_min;
}

State* SeqCProgState::copy() {
  return new SeqCProgState(*this);
}

State* SeqCProgState::new_state(const index_set& s, State* p) {
  SeqCProgState* new_s =
    (res ? new SeqCProgState(instance, heuristic, cost, s, res)
         : new SeqCProgState(instance, heuristic, cost, s));
  new_s->State::set_predecessor(p);
  return new_s;
}

State* SeqCProgState::new_state(const bool_vec& s, State* p) {
  SeqCProgState* new_s =
    (res ? new SeqCProgState(instance, heuristic, cost, s, res)
         : new SeqCProgState(instance, heuristic, cost, s));
  new_s->State::set_predecessor(p);
  return new_s;
}

int SeqCProgState::compare(const State& s) {
  if (act < ((SeqCProgState&)s).act) return -1;
  else if (act > ((SeqCProgState&)s).act) return 1;
  else return AtomSetState::compare(s);
}

index_type SeqCProgState::hash()
{
  return (AtomSetState::hash() + act);
}

void SeqCProgState::write(std::ostream& s) {
  if (act != no_such_index)
    s << "(" << instance.actions[act].name << ") ";
  else
    s << "() ";
  instance.write_atom_set(s, set);
  if (res) {
    s << " (";
    res->write(s);
    s << ")";
  }
}


NTYPE RestrictedSeqProgState::expand(Search& s, NTYPE bound)
{
  NTYPE c_min = POS_INF;
  for (index_type k = 0; k < instance.n_actions(); k++) if (instance.actions[k].sel) {
    Instance::Action& a = instance.actions[k];
    if (applicable(a)) {
      RestrictedSeqProgState* new_s = (RestrictedSeqProgState*)apply(a);
      if (FINITE(new_s->est) && (cost(a.index) + new_s->est <= bound)) {
	bool allowed = true;
	for (index_type i = 0; i < a.del.length(); i++)
	  if (p_atoms[a.del[i]]) {
	    allowed = false;
	    f_atoms[a.del[i]] = true;
	  }
	if (allowed) {
	  NTYPE c_new =
	    cost(a.index) + s.new_state(*new_s, bound - cost(a.index));
	  if (s.done()) {
	    delete new_s;
	    return c_new;
	  }
	  else {
	    c_min = MIN(c_min, c_new);
	  }
	}
      }
      else {
	c_min = MIN(c_min, cost(a.index) + new_s->est);
      }
      delete new_s;
    }
  }
  return c_min;
}

State* RestrictedSeqProgState::new_state(const index_set& s, State* p)
{
  RestrictedSeqProgState* new_s =
    (res ? new RestrictedSeqProgState(instance, p_atoms, f_atoms,
				      heuristic, cost, s, res)
         : new RestrictedSeqProgState(instance, p_atoms, f_atoms,
				      heuristic, cost, s));
  new_s->State::set_predecessor(p);
  return new_s;
}

State* RestrictedSeqProgState::new_state(const bool_vec& s, State* p)
{
  RestrictedSeqProgState* new_s =
    (res ? new RestrictedSeqProgState(instance, p_atoms, f_atoms,
				      heuristic, cost, s, res)
         : new RestrictedSeqProgState(instance, p_atoms, f_atoms,
				      heuristic, cost, s));
  new_s->State::set_predecessor(p);
  return new_s;
}

State* RestrictedSeqProgState::copy()
{
  return new RestrictedSeqProgState(*this);
}


bool ExtendedSeqProgState::applicable(Instance::Action& a)
{
  if (!SeqProgState::applicable(a)) return false;
  if (PDDL_Base::compile_away_disjunctive_preconditions) return true;
  ptr_pair* p = (ptr_pair*)a.src;
  PDDL_Base::ActionSymbol* act = (PDDL_Base::ActionSymbol*)p->first;
//   if (act->n_dset_pre > 0) {
//     ptr_table* ins = (ptr_table*)p->second;
//     ptr_table::key_vec* args = ins->key_sequence();
//     for (index_type k = 0; k < act->n_param; k++)
//       act->param[k]->value = (PDDL_Base::Symbol*)((*args)[k + 1]);
//     delete args;
//     for (index_type k = 0; k < act->n_dset_pre; k++) {
//       index_set s;
//       act->dset_pre[k]->instantiate(instance, s);
//       bool is_sat = false;
//       for (index_type i = 0; (i < s.length()) && !is_sat; i++)
// 	if (set[s[i]]) is_sat = true;
//       if (!is_sat) return false;
//     }
//   }
  return true;
}

SeqProgState* ExtendedSeqProgState::apply(Instance::Action& a)
{
  ExtendedSeqProgState* s = (ExtendedSeqProgState*)copy();
  for (index_type k = 0; k < a.add.length(); k++) {
    s->set[a.add[k]] = true;
  }
  for (index_type k = 0; k < a.del.length(); k++) {
    s->set[a.del[k]] = false;
  }
  if (!PDDL_Base::compile_away_conditional_effects) {
    ptr_pair* p = (ptr_pair*)a.src;
    PDDL_Base::ActionSymbol* act = (PDDL_Base::ActionSymbol*)p->first;
    if (act->cond_eff.length() > 0) {
      ptr_table* ins = (ptr_table*)p->second;
      ptr_table::key_vec* args = ins->key_sequence();
      for (index_type k = 0; k < act->param.length(); k++)
	act->param[k]->value = (PDDL_Base::Symbol*)((*args)[k + 1]);
      delete args;
      for (index_type k = 0; k < act->cond_eff.length(); k++) {
	rule_set ne;
	rule_set pe;
	act->cond_eff[k]->instantiate_conditional(instance, pe, ne);
	for (index_type i = 0; i < ne.length(); i++) {
	  bool app = true;
	  for (index_type j = 0; (j < ne[i].antecedent.length()) && app; j++)
	    if (!set[ne[i].antecedent[j]]) app = false;
	  if (app) {
	    s->set[ne[i].consequent] = false;
	  }
	}
	for (index_type i = 0; i < pe.length(); i++) {
	  bool app = true;
	  for (index_type j = 0; (j < pe[i].antecedent.length()) && app; j++)
	    if (!set[pe[i].antecedent[j]]) app = false;
	  if (app) {
	    s->set[pe[i].consequent] = true;
	  }
	}
      }
    }
  }
  s->count();
  s->State::set_predecessor(this);
  s->act = a.index;
  s->eval();
  return s;
}

State* ExtendedSeqProgState::new_state(const index_set& s, State* p)
{
  ExtendedSeqProgState* new_s =
    new ExtendedSeqProgState(instance, heuristic, cost, s);
  new_s->State::set_predecessor(p);
  return new_s;
}

State* ExtendedSeqProgState::new_state(const bool_vec& s, State* p)
{
  ExtendedSeqProgState* new_s =
    new ExtendedSeqProgState(instance, heuristic, cost, s);
  new_s->State::set_predecessor(p);
  return new_s;
}

State* ExtendedSeqProgState::copy()
{
  return new ExtendedSeqProgState(*this);
}


RelaxedSeqProgState::RelaxedSeqProgState(const RelaxedSeqProgState& s)
  : SeqProgState(s), x_atoms(s.x_atoms), x_action(s.x_action)
{
  // done
}

void RelaxedSeqProgState::RelaxedSeqProgState::compute_x_actions()
{
  x_action.assign_value(false, instance.n_actions());
  for (index_type k = 0; k < instance.n_actions(); k++) {
    if (instance.actions[k].del.first_common_element(x_atoms) != no_such_index)
      x_action[k] = true;
  }
}

void RelaxedSeqProgState::closure()
{
  bool done = false;
  while (!done) {
    done = true;
    for (index_type k = 0; k < instance.n_actions(); k++) if (!x_action[k])
      if (SeqProgState::applicable(instance.actions[k]))
	for (index_type i = 0; i < instance.actions[k].add.length(); i++) {
	  if (!set[instance.actions[k].add[i]]) done = false;
	  set[instance.actions[k].add[i]] = true;
	}
  }
  count();
}

bool RelaxedSeqProgState::applicable(Instance::Action& a)
{
  if (!x_action[a.index]) return false;
  return SeqProgState::applicable(a);
}

SeqProgState* RelaxedSeqProgState::apply(Instance::Action& a)
{
  RelaxedSeqProgState* s = new RelaxedSeqProgState(*this);
  for (index_type k = 0; k < a.add.length(); k++)
    s->set[a.add[k]] = true;
  for (index_type k = 0; k < a.del.length(); k++)
    s->set[a.del[k]] = false;
  s->closure();
  s->State::set_predecessor(this);
  s->act = a.index;
  s->eval();
  return s;
}

State* RelaxedSeqProgState::new_state(const index_set& s, State* p)
{
  RelaxedSeqProgState* new_s =
    new RelaxedSeqProgState(instance, x_atoms, heuristic, cost, s);
  new_s->State::set_predecessor(p);
  return new_s;
}

State* RelaxedSeqProgState::new_state(const bool_vec& s, State* p)
{
  RelaxedSeqProgState* new_s =
    new RelaxedSeqProgState(instance, x_atoms, heuristic, cost, s);
  new_s->State::set_predecessor(p);
  return new_s;
}

State* RelaxedSeqProgState::copy()
{
  return new RelaxedSeqProgState(*this);
}

NTYPE FwdUnitHeuristic::eval(const index_set& s)
{
  if (s.contains(instance.goal_atoms))
    return 0;
  else
    return 1;
}

NTYPE FwdUnitHeuristic::eval(const bool_vec& s)
{
  if (s.contains(instance.goal_atoms))
    return 0;
  else
    return 1;
}

END_HSPS_NAMESPACE
