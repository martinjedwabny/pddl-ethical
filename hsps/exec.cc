
#include "exec.h"
#include "base.h"

BEGIN_HSPS_NAMESPACE

ExecError::~ExecError()
{
  // done
}

ExecError::ErrorSeverity ExecError::severity_of_error() const
{
  switch (type_of_error()) {
  case ExecError::warning_redundant_action:
    return severity_warning;
  case ExecError::error_unachieved_goal:
    return severity_plan_failure;
  case ExecError::error_unsatisfied_precondition:
  case ExecError::error_incompatible_actions:
  case ExecError::error_resource_conflict:
  case ExecError::error_resource_shortage:
  default:
    return severity_execution_failure;
  }
}

const char* ExecError::error_severity_string(ErrorSeverity s)
{
  switch (s) {
  case ExecError::severity_none:
    return "none";
  case ExecError::severity_warning:
    return "warning";
  case ExecError::severity_plan_failure:
    return "plan failure";
  case ExecError::severity_execution_failure:
    return "execution failure";
  default:
    return "unknown error";
  }
}

const char* ExecError::error_type_string(ErrorType t)
{
  switch (t) {
  case ExecError::error_unsatisfied_precondition:
    return "unsatisfied precondition";
  case ExecError::error_incompatible_actions:
    return "incompatible actions";
  case ExecError::error_resource_conflict:
    return "resource conflict";
  case ExecError::error_resource_shortage:
    return "resource shortage";
  case ExecError::error_unachieved_goal:
    return "unsatisifed goal";
  case ExecError::warning_redundant_action:
    return "redundant action";
  default:
    return "unknown error";
  }
}

void ExecError::remap_step(const index_vec& map)
{
  if (step != no_such_index) step = map[step];
}

ExecError* ExecError::copy() const
{
  return new ExecError(type_of_error(), time_of_error(), step_of_error());
}

void ExecError::write(::std::ostream& s) const
{
  s << '[' << PRINT_NTYPE(time_of_error());
  if (step_of_error() != no_such_index) {
    s << " (step #" << step_of_error() << ")";
  }
  else {
    s << " (no step)";
  }
  s << ']' << error_type_string(type_of_error());
}

::std::ostream& operator<<(::std::ostream& s, ExecError::ErrorType t)
{
  return s << ExecError::error_type_string(t);
}

::std::ostream& operator<<(::std::ostream& s, ExecError::ErrorSeverity se)
{
  return s << ExecError::error_severity_string(se);
}

ExecErrorSet::~ExecErrorSet()
{
  for (index_type k = 0; k < length(); k++) {
    assert((*this)[k]);
    delete (*this)[k];
  }
}

void ExecErrorSet::ignore_error_type(ExecError::ErrorType t)
{
  ignored_error_types.insert(t);
}

void ExecErrorSet::ignore_error_severity(ExecError::ErrorSeverity s)
{
  switch (s) {
  case ExecError::severity_warning:
    ignored_error_types.insert(ExecError::warning_redundant_action);
    break;
  case ExecError::severity_plan_failure:
    ignored_error_types.insert(ExecError::error_unachieved_goal);
    break;
  case ExecError::severity_execution_failure:
    ignored_error_types.insert(ExecError::error_unsatisfied_precondition);
    ignored_error_types.insert(ExecError::error_incompatible_actions);
    ignored_error_types.insert(ExecError::error_resource_conflict);
    ignored_error_types.insert(ExecError::error_resource_shortage);
    break;
  }
}

void ExecErrorSet::clear_ignored_error_types()
{
  ignored_error_types.clear();
}

void ExecErrorSet::new_error(ExecError* e)
{
  if (ignore(e->type_of_error())) {
    delete e;
    return;
  }
  append(e);
}

bool ExecErrorSet::ignore(ExecError::ErrorType t)
{
  return ignored_error_types.contains(t);
}

void ExecErrorSet::remap_steps(const index_vec& map)
{
  for (index_type k = 0; k < length(); k++)
    (*this)[k]->remap_step(map);
}

NTYPE ExecErrorSet::earliest_time_of_error()
{
  NTYPE t_min = POS_INF;
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k]->time_of_error() < t_min)
      t_min = (*this)[k]->time_of_error();
  return t_min;
}

ExecError::ErrorSeverity ExecErrorSet::greatest_error_severity()
{
  ExecError::ErrorSeverity s_max = ExecError::severity_none;
  for (index_type k = 0; k < length(); k++) {
//     ::std::cerr << "error: ";
//     (*this)[k]->write(::std::cerr);
//     ::std::cerr << ", severity = " << (*this)[k]->severity_of_error()
// 	      << ", s_max = " << s_max
// 	      << ", > s_max? " << ((*this)[k]->severity_of_error() > s_max)
// 	      << ::std::endl;
    if ((*this)[k]->severity_of_error() > s_max)
      s_max = (*this)[k]->severity_of_error();
  }
  return s_max;
}

ExecErrorSet* ExecErrorSet::earliest()
{
  NTYPE t_min = earliest_time_of_error();
  ExecErrorSet* s = new ExecErrorSet();
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k]->time_of_error() == t_min)
      s->append((*this)[k]->copy());
  return s;
}

ExecErrorSet* ExecErrorSet::earliest_of_type(ExecError::ErrorType t)
{
  ExecErrorSet* s1 = all_of_type(t);
  if (s1->length() == 0) return s1;
  ExecErrorSet* s0 = s1->earliest();
  delete s1;
  return s0;
}

ExecErrorSet* ExecErrorSet::all_of_type(ExecError::ErrorType t)
{
  ExecErrorSet* s = new ExecErrorSet();
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k]->type_of_error() == t)
      s->append((*this)[k]->copy());
  return s;
}

ExecErrorSet* ExecErrorSet::all_of_severity(ExecError::ErrorSeverity s)
{
  ExecErrorSet* ns = new ExecErrorSet();
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k]->severity_of_error() == s)
      ns->append((*this)[k]->copy());
  return ns;
}

index_type ExecErrorSet::count_of_type(ExecError::ErrorType t)
{
  index_type n = 0;
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k]->type_of_error() == t) n += 1;
  return n;
}

bool ExecErrorSet::executable()
{
  return (greatest_error_severity() < ExecError::severity_execution_failure);
}

bool ExecErrorSet::valid()
{
  return (greatest_error_severity() < ExecError::severity_plan_failure);
}

void ExecErrorSet::write(::std::ostream& s) const
{
  s << "{";
  for (index_type k = 0; k < length(); k++) {
    if (k > 0) s << "; ";
    (*this)[k]->write(s);
  }
  s << "}";
}

ExecError* UnsatisfiedPreconditionError::copy()
{
  return new UnsatisfiedPreconditionError(ins, act, pre, holds,
					  time_of_error(), step_of_error());
}

void UnsatisfiedPreconditionError::write(::std::ostream& s) const
{
  s << "[" << PRINT_NTYPE(time_of_error()) << "] precondition "
    << ins.atoms[pre].name << " of " << ins.actions[act].name << " false";
}

ExecError* RedundantActionWarning::copy()
{
  return
    new RedundantActionWarning(ins, act, time_of_error(), step_of_error());
}

void RedundantActionWarning::write(::std::ostream& s) const
{
  s << "[" << PRINT_NTYPE(time_of_error()) << "] action "
    << ins.actions[act].name << " is redundant";
}

ExecError* IncompatibleActionError::copy()
{
  return new IncompatibleActionError(ins, act0, act1,
				     time_of_error(), step_of_error());
}

void IncompatibleActionError::write(::std::ostream& s) const
{
  s << "[" << PRINT_NTYPE(time_of_error()) << "] action "
    << ins.actions[act0].name << " incompatible with "
    << ins.actions[act1].name;
}

ExecError* ResourceConflictError::copy()
{
  return new ResourceConflictError(ins, res, c_acts, c_steps,
				   time_of_error(), step_of_error());
}

void ResourceConflictError::write(::std::ostream& s) const
{
  s << "[" << PRINT_NTYPE(time_of_error()) << "] actions ";
  NTYPE sum = 0;
  for (index_type k = 0; k < c_acts.length(); k++) {
    if (k > 0) s << ",";
    s << ins.actions[c_acts[k]].name;
    sum += ins.actions[c_acts[k]].use[res];
  }
  s << " use " << PRINT_NTYPE(sum) << " of resource "
    << ins.resources[res].name
    << " with capacity " << PRINT_NTYPE(ins.resources[res].init);
}

ExecError* ResourceShortageError::copy()
{
  return new ResourceShortageError(ins, res, avail, act,
				   time_of_error(), step_of_error());
}

void ResourceShortageError::write(::std::ostream& s) const
{
  s << "[" << PRINT_NTYPE(time_of_error()) << "] action "
    << ins.actions[act].name
    << " requires " << PRINT_NTYPE(ins.actions[act].cons[res])
    << " of resource " << ins.resources[res].name
    << ", only " << PRINT_NTYPE(avail) << " available";
}

ExecError* UnachievedGoalError::copy()
{
  return new UnachievedGoalError(ins, atom, time_of_error(), step_of_error());
}

void UnachievedGoalError::write(::std::ostream& s) const
{
  s << "[" << PRINT_NTYPE(time_of_error()) << "] goal "
    << ins.atoms[atom].name << " not achieved";
}

bool ExecState::extended_action_definition = false;

ExecState::ExecState(Instance& i)
  : instance(i),
    atoms(false, instance.n_atoms()),
    res(0, instance.n_resources()),
    abs_t(0),
    delta_t(0),
    dur(POS_INF),
    trace_level(0)
{
  for (index_type k = 0; k < instance.n_resources(); k++)
    res[k] = instance.resources[k].init;
}

ExecState::ExecState(Instance& i, index_set g)
  : instance(i),
    atoms(g, instance.n_atoms()),
    res(0, instance.n_resources()),
    abs_t(0),
    delta_t(0),
    dur(POS_INF),
    trace_level(0)
{
  for (index_type k = 0; k < instance.n_resources(); k++)
    res[k] = instance.resources[k].init;
}

ExecState::ExecState(Instance& i, const bool_vec& g)
  : instance(i),
    atoms(g),
    res(0, instance.n_resources()),
    abs_t(0),
    delta_t(0),
    dur(POS_INF),
    trace_level(0)
{
  for (index_type k = 0; k < instance.n_resources(); k++)
    res[k] = instance.resources[k].init;
}

ExecState::ExecState(const ExecState& s)
  : ProgressionState(s),
    instance(s.instance),
    atoms(s.atoms),
    res(s.res),
    actions(s.actions),
    abs_t(s.abs_t),
    delta_t(s.delta_t),
    dur(s.dur),
    trace_level(s.trace_level)
{
  for (index_type k = 0; k < instance.n_atoms(); k++)
    atoms[k] = s.atoms[k];
}

ExecState::~ExecState()
{
  // done (?)
}

void ExecState::set_trace_level(int level)
{
  trace_level = level;
}

NTYPE ExecState::delta_cost()
{
  return delta_t;
}

NTYPE ExecState::est_cost()
{
  return 0;
}

bool ExecState::is_final(ExecErrorSet* errors)
{
  bool ok = true;
  for (index_type k = 0; k < instance.goal_atoms.length(); k++)
    if (!atoms[instance.goal_atoms[k]]) {
      if (errors) {
	errors->new_error(new UnachievedGoalError
			  (instance, instance.goal_atoms[k], abs_t));
      }
      ok = false;
    }
  return ok;
}

bool ExecState::is_final()
{
  return is_final(0);
}

bool ExecState::is_max()
{
  return false;
}

NTYPE ExecState::expand(Search& s, NTYPE bound)
{
  ::std::cerr << "program error: can't expand ExecState " << *this << ::std::endl;
  exit(255);
}

void ExecState::store(NTYPE cost, bool opt)
{
  ::std::cerr << "program error: can't store ExecState " << *this << ::std::endl;
  exit(255);
}

void ExecState::reevaluate()
{
  // does nothing
}

int ExecState::compare(const State& s) {
  const ExecState& ws = (const ExecState&)s;
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    if (!atoms[k] && ws.atoms[k]) return -1;
    if (atoms[k] && !ws.atoms[k]) return 1;
  }
  if (actions.length() < ws.actions.length()) return -1;
  else if (actions.length() > ws.actions.length()) return 1;
  else {
    for (index_type k = 0; k < actions.length(); k++) {
      if (actions[k].act < ws.actions[k].act) return -1;
      else if (actions[k].act > ws.actions[k].act) return 1;
      else  {
	if (actions[k].rem < ws.actions[k].rem) return -1;
	if (actions[k].rem > ws.actions[k].rem) return 1;
      }
    }
  }
  if (delta_t < ws.delta_t) return -1;
  if (delta_t > ws.delta_t) return 1;
  return 0;
}

index_type ExecState::hash() {
  index_type val = instance.atom_set_hash(atoms);
  for (index_type k = 0; k < actions.length(); k++)
    val = instance.action_set_hash(actions[k].act, val);
  return val;
}

State* ExecState::new_state(index_set& s)
{
  return new ExecState(instance, s);
}

State* ExecState::copy()
{
  return new ExecState(*this);
}

void ExecState::insert(Plan& p)
{
  p.advance(delta_t);
  for (index_type k = 0; k < actions.length(); k++)
    if (actions[k].rem == instance.actions[actions[k].act].dur)
      p.insert(k);
}

void ExecState::write_plan(::std::ostream& s)
{
  index_set acts;
  finishing_actions(acts);
  bool first = true;
  for (index_type k = 0; k < acts.length(); k++) {
    if (!first) s << ", ";
    first = false;
    s << "finish:" << instance.actions[acts[k]].name;
  }
  starting_actions(acts);
  for (index_type k = 0; k < acts.length(); k++) {
    if (!first) s << ", ";
    first = false;
    s << "start:" << instance.actions[acts[k]].name;
  }
}

void ExecState::write(::std::ostream& s)
{
  index_set currently_locked;
  for (index_type k = 0; k < actions.length(); k++)
    currently_locked.insert(instance.actions[actions[k].act].lck);
  s << ";; [" << PRINT_NTYPE(current_time()) << ", "
    << PRINT_NTYPE(end_time()) << "]" << ::std::endl;
  s << "(:init";
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (atoms[k] && !currently_locked.contains(k))
      s << " " << instance.atoms[k].name;
  s << ::std::endl;
  amt_vec in_use(0, instance.n_resources());
  for (index_type k = 0; k < actions.length(); k++) {
    for (index_type r = 0; r < instance.n_resources(); r++)
      in_use[r] += instance.actions[actions[k].act].use[r];
  }
  for (index_type r = 0; r < instance.n_resources(); r++)
    s << " (= " << instance.resources[r].name << " "
      << PRINT_NTYPE(res[r]) << ") ;; " << PRINT_NTYPE(in_use[r])
      << " in use" << ::std::endl;
  for (index_type k = 0; k < actions.length(); k++) {
    s << " ;; " << instance.actions[actions[k].act].name << ", "
      << PRINT_NTYPE(actions[k].rem) << " remaining" << ::std::endl;
  }
  s << ")" << ::std::endl;
}

bool ExecState::applicable
(Instance::Action& a, ExecErrorSet* errors, index_type step)
{
  bool pre_ok = true;
  bool compat_ok = true;
  bool res_ok = true;
  bool cons_ok = true;
  for (index_type k = 0; k < a.pre.length(); k++) {
    bool is_sat = atoms[a.pre[k]];
    if (!is_sat) {
      if (trace_level > 0) {
	::std::cerr << "action " << a.name << ": precondition "
		  << instance.atoms[a.pre[k]].name << " not satisfied"
		  << ::std::endl;
      }
      if (errors) {
	index_set s(atoms);
	UnsatisfiedPreconditionError* upe =
	  new UnsatisfiedPreconditionError(instance, a.index, a.pre[k], s,
					   abs_t, step);
	errors->new_error(upe);
      }
      pre_ok = false;
    }
  }
  if (extended_action_definition) {
    assert(a.src);
    ptr_pair* p = (ptr_pair*)a.src;
    PDDL_Base::ActionSymbol* act = (PDDL_Base::ActionSymbol*)p->first;
//     if (act->n_dis_pre > 0) {
//       ptr_table* ins = (ptr_table*)p->second;
//       ptr_table::key_vec* args = ins->key_sequence();
//       for (index_type k = 0; k < act->n_param; k++)
// 	act->param[k]->value = (PDDL_Base::Symbol*)((*args)[k + 1]);
//       delete args;
//       for (index_type k = 0; k < act->n_dset_pre; k++) {
// 	index_set s;
// 	act->dset_pre[k]->instantiate(instance, s);
// 	bool d_sat = false;
// 	for (index_type i = 0; (i < s.length()) && !d_sat; i++) {
// 	  bool a_sat = atoms[s[i]];
// 	  for (index_type j = 0; (j < instance.n_actions()) && !a_sat; j++) {
// 	    if (actions[j] && (remains[j] <= dt)) {
// 	      a_sat = instance.actions[j].add.contains(s[i]);
// 	    }
// 	  }
// 	  if (a_sat) d_sat = true;
// 	}
// 	if (!d_sat) {
// 	  if (trace_level > 0) {
// 	    ::std::cerr << "action " << a.name << ": precondition ";
// 	    act->dset_pre[k]->print(::std::cerr);
// 	    ::std::cerr << " not satisfied" << ::std::endl;
// 	  }
// 	  pre_ok = false;
// 	}
//       }
//     }
  }
  amt_vec in_use(0, instance.n_resources());
  for (index_type k = 0; k < actions.length(); k++) {
    if (!instance.non_interfering(a.index, actions[k].act) ||
	!instance.lock_compatible(a.index, actions[k].act)) {
      if (trace_level > 0) {
	::std::cerr << "action " << a.name << ": conflict with "
		    << instance.actions[k].name << ::std::endl;
      }
      if (errors) {
	IncompatibleActionError* iae =
	  new IncompatibleActionError(instance, a.index, k, abs_t, step);
	errors->new_error(iae);
      }
      compat_ok = false;
    }
    for (index_type r = 0; r < instance.n_resources(); r++)
      in_use[r] += instance.actions[k].req(r);
  }
  for (index_type r = 0; r < instance.n_resources(); r++) {
    if (a.cons[r] > res[r]) {
      if (trace_level > 0) {
	::std::cerr << "action " << a.name << ": not enough of resource "
		  << instance.resources[r].name << " remaining"
		  << ::std::endl;
      }
      if (errors) {
	ResourceShortageError* rse = 
	  new ResourceShortageError(instance, r, res[r], a.index, abs_t, step);
	errors->new_error(rse);
      }
      cons_ok = false;
    }
    if ((in_use[r] + a.req(r)) > res[r]) {
      if (trace_level > 0) {
	::std::cerr << "action " << a.name << ": not enough of resource "
		  << instance.resources[r].name << " free" << ::std::endl;
      }
      if (errors) {
	ResourceConflictError* rce =
	  new ResourceConflictError(instance, r, abs_t, step);
	for (index_type k = 0; k < actions.length(); k++)
	  if (instance.actions[actions[k].act].req(r) > 0) {
	    if (actions[k].step != no_such_index)
	      rce->add_action(actions[k].act, actions[k].step);
	    else
	      rce->add_action(actions[k].act);
	  }
	if (step != no_such_index)
	  rce->add_action(a.index, step);
	else
	  rce->add_action(a.index);
	errors->new_error(rce);
      }
      res_ok = false;
    }
  }
  return (pre_ok && compat_ok && res_ok && cons_ok);
}

void ExecState::apply
(Instance::Action& a, ExecErrorSet* errors, index_type step)
{
  exec_act e(a.index, a.dur, step);
  if (extended_action_definition)
    e.start_state = (ExecState*)copy();
  for (index_type k = 0; k < a.del.length(); k++) atoms[a.del[k]] = false;
  if (extended_action_definition)
    apply_conditional_delete_effects(a, e.start_state, atoms);
  actions.append(e);
}

void ExecState::apply_conditional_delete_effects
(Instance::Action& a, const ExecState* start_state, bool_vec& to)
{
  assert(a.src);
  assert(start_state);
  PDDL_Base::ActionSymbol* act =
    PDDL_Base::src_action_symbol((ptr_pair*)a.src);
  if (act->cond_eff.length() == 0) return;
  for (index_type k = 0; k < act->cond_eff.length(); k++) {
    rule_set pe;
    rule_set ne;
    act->cond_eff[k]->instantiate_conditional(instance, pe, ne);
    for (index_type i = 0; i < ne.length(); i++) {
      bool app = true;
      for (index_type j = 0; (j < ne[i].antecedent.length()) && app; j++)
	if (!start_state->atoms[ne[i].antecedent[j]]) app = false;
      if (app) {
	to[ne[i].consequent] = false;
      }
    }
  }
}

void ExecState::active_conditional_add_effects
(Instance::Action& a, const ExecState* start_state, bool_vec& eff)
{
  assert(a.src);
  assert(start_state);
  PDDL_Base::ActionSymbol* act =
    PDDL_Base::src_action_symbol((ptr_pair*)a.src);
  if (act->cond_eff.length() == 0) return;
  for (index_type k = 0; k < act->cond_eff.length(); k++) {
    rule_set pe;
    rule_set ne;
    act->cond_eff[k]->instantiate_conditional(instance, pe, ne);
    for (index_type i = 0; i < pe.length(); i++) {
      bool app = true;
      for (index_type j = 0; (j < pe[i].antecedent.length()) && app; j++)
	if (!start_state->atoms[pe[i].antecedent[j]]) app = false;
      if (app) {
	eff[pe[i].consequent] = true;
      }
    }
  }
}

void ExecState::advance(NTYPE dt, ExecErrorSet* errors)
{
  bool_vec new_atoms(atoms);
  bool_vec finishing(false, actions.length());
  for (index_type k = 0; k < actions.length(); k++) {
    actions[k].rem -= dt;
    if (actions[k].rem <= 0) {
      finishing[k] = true;
      Instance::Action& a = instance.actions[actions[k].act];
      if (trace_level > 1) {
	::std::cerr << "action " << a.name << " started at "
		    << PRINT_NTYPE(abs_t + dt - a.dur) << " as step "
		    << actions[k].step << " finishing at "
		    << PRINT_NTYPE(abs_t + dt - actions[k].rem)
		    << "..." << ::std::endl;
      }

      bool_vec added(false, instance.n_atoms());
      added.insert(a.add);
      if (extended_action_definition) {
	active_conditional_add_effects(a, actions[k].start_state, added);
	delete actions[k].start_state;
	actions[k].start_state = 0;
      }
      bool is_red = true;
      for (index_type i = 0; i < instance.n_atoms(); i++) if (added[i]) {
	if (!atoms[i]) is_red = false;
	new_atoms[i] = true;
      }
      if (is_red) {
	if (trace_level > 0) {
	  ::std::cerr << "action " << a.name << " finishing at "
		      << PRINT_NTYPE(abs_t + dt - actions[k].rem)
		      << " is redundant (no effect)" << ::std::endl;
	}
	if (errors) {
	  RedundantActionWarning* raw =
	    new RedundantActionWarning(instance, k, abs_t+dt, actions[k].step);
	  errors->new_error(raw);
	}
      }
    }
  }
  atoms.assign_copy(new_atoms);
  actions.remove(finishing);
  abs_t += dt;
  delta_t = dt;
}

void ExecState::finish(ExecErrorSet* errors)
{
  advance(max_delta(), errors);
}

void ExecState::clip(NTYPE at_t)
{
  if (at_t >= abs_t) {
    dur = at_t - abs_t;
  }
  else {
    ::std::cerr << "error: can't clip state " << this << " at "
	      << at_t << " since it only starts at " << abs_t
	      << ::std::endl;
    exit(255);
  }
}

void ExecState::intersect(const bool_vec& atms)
{
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (!atms[k]) atoms[k] = false;
}

NTYPE ExecState::current_time() const
{
  return abs_t;
}

NTYPE ExecState::end_time() const
{
  return (abs_t + dur);
}

void ExecState::current_atoms(index_set& atms) const
{
  atms.clear();
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (atoms[k]) atms.insert(k);
}

const bool_vec& ExecState::current_atoms() const
{
  return atoms;
}

void ExecState::current_actions(index_set& acts) const
{
  acts.clear();
  for (index_type k = 0; k < actions.length(); k++)
    acts.insert(actions[k].act);
}

index_type ExecState::n_current_actions() const
{
  return actions.length();
}

void ExecState::starting_actions(index_set& acts) const
{
  acts.clear();
  for (index_type k = 0; k < actions.length(); k++)
    if (actions[k].rem == instance.actions[actions[k].act].dur)
      acts.insert(actions[k].act);
}

void ExecState::finishing_actions(index_set& acts) const
{
  acts.clear();
  for (index_type k = 0; k < actions.length(); k++)
    if (actions[k].rem <= dur)
      acts.insert(actions[k].act);
}

NTYPE ExecState::min_delta() const
{
  NTYPE d = POS_INF;
  for (index_type k = 0; k < actions.length(); k++)
    d = MIN(d, actions[k].rem);
  return d;
}

NTYPE ExecState::max_delta() const
{
  NTYPE d = 0;
  for (index_type k = 0; k < actions.length(); k++)
    d = MAX(d, actions[k].rem);
  return d;
}

void ExecState::current_resource_use(amt_vec& in_use) const
{
  for (index_type r = 0; r < instance.n_resources(); r++) in_use[r] = 0;
  for (index_type k = 0; k < actions.length(); k++) {
    for (index_type r = 0; r < instance.n_resources(); r++)
      if (instance.resources[r].reusable())
	in_use[r] += instance.actions[actions[k].act].use[r];
  }
}

void ExecState::current_resource_levels(amt_vec& avail, amt_vec& in_use) const
{
  for (index_type r = 0; r < instance.n_resources(); r++) avail[r] = res[r];
  for (index_type r = 0; r < instance.n_resources(); r++) in_use[r] = 0;
  for (index_type k = 0; k < actions.length(); k++) {
    for (index_type r = 0; r < instance.n_resources(); r++)
      in_use[r] += instance.actions[actions[k].act].use[r];
  }
}

void ExecState::current_resource_levels
(index_type r, NTYPE& avail, NTYPE& in_use) const
{
  assert(r < instance.n_resources());
  avail = res[r];
  in_use = 0;
  for (index_type k = 0; k < actions.length(); k++) {
    in_use += instance.actions[actions[k].act].use[r];
  }
}

bool ExecState::check_atoms(const index_set& set) const
{
  for (index_type k = 0; k < set.length(); k++)
    if (!atoms[set[k]]) return false;
  return true;
}

bool ExecState::check_atoms(const index_set& set, index_set& holds) const
{
  holds.clear();
  for (index_type k = 0; k < set.length(); k++)
    if (atoms[set[k]]) holds.insert(set[k]);
  return (holds.length() == set.length());
}

Timeline::~Timeline()
{
  // done
}

NTYPE Timeline::total_time()
{
  if (n_intervals() == 0) {
    ::std::cerr << "error: total time of zero-interval makespan is undefined"
	      << ::std::endl;
  }
  NTYPE s = interval_start_time(0);
  NTYPE t = interval_end_time(n_intervals() - 1);
  if (FINITE(s) && FINITE(t)) {
    return t - s;
  }
  else {
    return POS_INF;
  }
}

void BasicTimeline::set_point(NTYPE t)
{
  index_type k = 0;
  while (k < points.length()) {
    if (t < points[k]) {
      points.insert(t, k);
      return;
    }
    k += 1;
  }
  points.append(t);
}

void BasicTimeline::clip_start(NTYPE t)
{
  if (points.length() == 0) {
    points.assign_value(t, 1);
  }
  else {
    if (points[0] > t) points.insert(t, 0);
  }
  open_start = false;
}

void BasicTimeline::clip_end(NTYPE t)
{
  if (points.length() == 0) {
    points.assign_value(t, 1);
  }
  else {
    if (points[points.length() - 1] < t) points.append(t);
  }
  open_end = false;
}

BasicTimeline::BasicTimeline()
  : points(0, 0), open_start(true), open_end(true)
{
  // done
}

BasicTimeline::~BasicTimeline()
{
  // done
}

index_type BasicTimeline::n_intervals()
{
  return (points.length() + (open_start ? 1 : 0) + (open_end ? 1 : 0) - 1);
}

index_type BasicTimeline::n_points()
{
  return points.length();
}

index_type BasicTimeline::interval_start_point(index_type i)
{
  if (open_start) {
    if (i == 0) return no_such_index;
    else {
      if (i > points.length()) {
	::std::cerr << "invalid interval " << i
		  << " in BasicTimeline::interval_start_point"
		  << ::std::endl;
	exit(255);
      }
      return i - 1;
    }
  }
  else {
    if (i >= points.length()) {
      ::std::cerr << "invalid interval " << i
		<< " in BasicTimeline::interval_start_point"
		<< ::std::endl;
      exit(255);
    }
    return i;
  }
}

index_type BasicTimeline::interval_end_point(index_type i)
{
  if (open_start) {
    if (i > points.length()) {
      ::std::cerr << "invalid interval " << i
		<< " in BasicTimeline::interval_end_point"
		<< ::std::endl;
      exit(255);
    }
    else if (i == points.length()) {
      if (open_end) return no_such_index;
      else {
	::std::cerr << "invalid interval " << i
		  << " in BasicTimeline::interval_end_point"
		  << ::std::endl;
	exit(255);
      }
    }
    else {
      return i;
    }
  }
  else {
    if ((i + 1) < points.length()) {
      return i + 1;
    }
    else if (((i + 1) == points.length()) && open_end) {
      return no_such_index;
    }
    else {
      ::std::cerr << "invalid interval " << i
		<< " in BasicTimeline::interval_end_point"
		<< ::std::endl;
      exit(255);
    }
  }
}

NTYPE BasicTimeline::point_time(index_type i)
{
  if (i >= points.length()) {
    ::std::cerr << "error: invalid point " << i
	      << " in BasicTimeline::point_time"
	      << ::std::endl;
    exit(255);
  }
  return points[i];
}

NTYPE BasicTimeline::interval_start_time(index_type i)
{
  index_type j = interval_start_point(i);
  if (j == no_such_index) return NEG_INF;
  return point_time(j);
}

NTYPE BasicTimeline::interval_end_time(index_type i)
{
  index_type j = interval_end_point(i);
  if (j == no_such_index) return POS_INF;
  return point_time(j);
}

void BasicTimeline::write(::std::ostream& s)
{
  for (index_type i = 0; i < n_intervals(); i++) {
    if (i > 0) s << ", ";
    s << '[' << interval_start_time(i) << ',' << interval_end_time(i) << ']';
  }
}

ResourceProfile::ResourceProfile
(Instance& ins, index_type r, ExecTrace& trace)
  : instance(ins),
    res(r),
    avail(0, 1),
    in_use(0, 1),
    a_start(EMPTYSET, 1),
    a_finish(EMPTYSET, 1),
    max_req(0)
{
  if (trace.length() == 0) {
    ::std::cerr << "error: can't construct resource profile from empty trace"
		<< ::std::endl;
    exit(255);
  }
  clip_start(trace[0]->current_time());
  trace[0]->current_resource_levels(res, avail[0], in_use[0]);
  index_set s;
  trace[0]->starting_actions(s);
  for (index_type j = 0; j < s.length(); j++)
    if (instance.actions[s[j]].req(res) > 0) {
      a_start[0].insert(s[j]);
      max_req = MAX(max_req, instance.actions[s[j]].req(res));
    }
  index_type i = 0;
  for (index_type k = 1; k < trace.length(); k++) {
    index_type n_start = 0;
    index_type n_finish = 0;
    trace[k]->starting_actions(s);
    for (index_type j = 0; j < s.length(); j++)
      if (instance.actions[s[j]].req(res) > 0) {
	n_start += 1;
	max_req = MAX(max_req, instance.actions[s[j]].req(res));
      }
    trace[k - 1]->finishing_actions(s);
    for (index_type j = 0; j < s.length(); j++)
      if (instance.actions[s[j]].req(res) > 0)
	n_finish += 1;
    if (n_start + n_finish > 0) {
      if (trace[k]->current_time() > interval_start_time(i)) {
	set_point(trace[k]->current_time());
	i += 1;
	trace[k]->current_resource_levels(res, avail[i], in_use[i]);
      }
    }
    a_start.inc_length_to(i + 1, EMPTYSET);
    trace[k]->starting_actions(s);
    for (index_type j = 0; j < s.length(); j++)
      if (instance.actions[s[j]].req(res) > 0) {
	a_start[i].insert(s[j]);
	max_req = MAX(max_req, instance.actions[s[j]].req(res));
      }
    a_finish.inc_length_to(i + 1, EMPTYSET);
    trace[k - 1]->finishing_actions(s);
    for (index_type j = 0; j < s.length(); j++)
      if (instance.actions[s[j]].req(res) > 0)
	a_finish[i].insert(s[j]);
    trace[k]->current_resource_levels(res, avail[i], in_use[i]);
  }
  if (FINITE(trace.final_state()->end_time()))
    clip_end(trace.final_state()->end_time());
}

ResourceProfile::~ResourceProfile()
{
  // done
}

void ResourceProfile::set_makespan(NTYPE t)
{
  clip_end(t);
}

NTYPE ResourceProfile::amount_available(index_type i)
{
  if (i >= n_intervals()) {
    ::std::cerr << "error: invalid interval " << i
	      << " in ResourceProfile::amount_available"
	      << ::std::endl;
    exit(255);
  }
  return avail[i];
}

NTYPE ResourceProfile::amount_in_use(index_type i)
{
  if (i >= n_intervals()) {
    ::std::cerr << "error: invalid interval " << i
	      << " in ResourceProfile::amount_in_use"
	      << ::std::endl;
    exit(255);
  }
  return in_use[i];
}

index_type ResourceProfile::first_use_interval(index_type i)
{
  while (i < n_intervals()) {
    if (amount_in_use(i) > 0) return i;
    i += 1;
  }
  return no_such_index;
}

NTYPE ResourceProfile::amount_free(index_type i)
{
  if (i >= n_intervals()) {
    ::std::cerr << "error: invalid interval " << i
	      << " in ResourceProfile::amount_free"
	      << ::std::endl;
    exit(255);
  }
  return (avail[i] - in_use[i]);
}

NTYPE ResourceProfile::possible_unexpected_loss_to(index_type i)
{
  assert(i < n_intervals());
  NTYPE pul = 0;
  for (index_type j = 0; j <= i; j++)
    for (index_type k = 0; k < a_finish[j].length(); k++)
      pul += instance.actions[a_finish[j][k]].use[res];
  return pul;
}

// NTYPE ResourceProfile::tolerable_loss_defective()
// {
//   index_type i = 0;
//   while (i < n_intervals()) {
//     if (in_use[i] > 0) {
//       if (first_use_interval(i + 1) != no_such_index) {
// 	return min_free_from(i + 1);
//       }
//       else {
// 	return POS_INF;
//       }
//     }
//     i += 1;
//   }
//   return POS_INF;
// }

NTYPE ResourceProfile::tolerable_unexpected_loss()
{
  NTYPE tul = POS_INF;
  for (index_type i = 0; i < n_intervals(); i++) if (in_use[i] > 0) {
    NTYPE pul_to_i = possible_unexpected_loss_to(i);
    if (pul_to_i > amount_free(i))
      tul = MIN(amount_free(i), tul);
  }
  return tul;
}

NTYPE ResourceProfile::min_free_from(index_type i)
{
  if (i >= n_intervals()) {
    ::std::cerr << "error: invalid interval " << i
	      << " in ResourceProfile::amount_available"
	      << ::std::endl;
    exit(255);
  }
  NTYPE f = amount_free(i);
  for (index_type j = i + 1; j < n_intervals(); j++)
    f = MIN(f, amount_free(j));
  return f;
}

index_type ResourceProfile::first_min_free_interval(index_type i)
{
  NTYPE f = POS_INF;
  index_type p = no_such_index;
  for (index_type j = i; j < n_intervals(); j++)
    if (amount_free(j) < f) {
      f = amount_free(j);
      p = j;
    }
  return p;
}

NTYPE ResourceProfile::min_free()
{
  return min_free_from(0);
}

NTYPE ResourceProfile::peak_use()
{
  return (instance.resources[res].init - min_free_from(0));
}

NTYPE ResourceProfile::min_peak_use()
{
  return max_req;
}

NTYPE ResourceProfile::total_consumption()
{
  return (instance.resources[res].init - amount_available(n_intervals() - 1));
}

void ResourceProfile::write(::std::ostream& s)
{
  for (index_type i = 0; i < n_intervals(); i++) {
    for (index_type k = 0; k < a_finish[i].length(); k++)
      s << " - finish " << instance.actions[a_finish[i][k]].name << ::std::endl;
    for (index_type k = 0; k < a_start[i].length(); k++)
      s << " - start " << instance.actions[a_start[i][k]].name << ::std::endl;
    s << "[" << PRINT_NTYPE(interval_start_time(i))
      << "," << PRINT_NTYPE(interval_end_time(i))
      << "]: a = " << PRINT_NTYPE(amount_available(i))
      << ", u = " << PRINT_NTYPE(amount_in_use(i))
      << ", f = " << PRINT_NTYPE(amount_free(i))
      << ", tl = ";
    if (i < (n_intervals() - 1)) {
      s << PRINT_NTYPE(min_free_from(i + 1));
    }
    else {
      s << PRINT_NTYPE(amount_available(i));
    }
    s << ::std::endl;
  }
}

double ResourceProfile::GANTT_EXTRA_WIDTH = 10;
double ResourceProfile::GANTT_EXTRA_HEIGHT = 10;

void ResourceProfile::writeGantt(::std::ostream& s)
{
#ifdef NTYPE_RATIONAL
  double pic_width = (points[points.length() - 1] - points[0]).decimal();
  double pic_height = instance.resources[res].init.decimal();
#else
  double pic_width = (points[points.length() - 1] - points[0]);
  double pic_height = instance.resources[res].init;
#endif
  s << "\\newcommand{\\ganttunitwidth}{" << Schedule::GANTT_UNIT_WIDTH << "}"
    << ::std::endl;
  s << "\\newcommand{\\ganttunitheight}{" << Schedule::GANTT_UNIT_HEIGHT << "}"
    << ::std::endl;
  s << "\\newcommand{\\ganttextrawidth}{" << GANTT_EXTRA_WIDTH << "}"
    << ::std::endl;
  s << "\\newcommand{\\gantthalfextrawidth}{" << GANTT_EXTRA_WIDTH/2 << "}"
    << ::std::endl;
  s << "\\newcommand{\\ganttextraheight}{" << GANTT_EXTRA_HEIGHT << "}"
    << ::std::endl;
  s << "\\newcommand{\\gantttextxoff}{" << Schedule::GANTT_TEXT_XOFF << "}"
    << ::std::endl;
  s << "\\newcommand{\\gantttextyoff}{" << Schedule::GANTT_TEXT_YOFF << "}"
    << ::std::endl;
  s << "\\newcount\\xzero" << ::std::endl;
  s << "\\newcount\\xone" << ::std::endl;
  s << "\\newcount\\yzero" << ::std::endl;
  s << "\\newcount\\yone" << ::std::endl;
  s << "\\xzero=" << pic_width << "\\ganttunitwidth" << ::std::endl;
  s << "\\advance\\xzero by \\ganttextrawidth\\ganttunitwidth" << ::std::endl;
  s << "\\yzero=" << pic_height << "\\ganttunitheight" << ::std::endl;
  s << "\\advance\\yzero by \\ganttextraheight\\ganttunitheight" << ::std::endl;
  s << "\\begin{picture}(\\xzero,\\yzero)(0,0)" << ::std::endl;

  // base line
  s << "\\xzero=0" << ::std::endl;
  s << "\\xone=" << pic_width << "\\ganttunitwidth" << ::std::endl;
  s << "\\advance\\xone by \\ganttextrawidth\\ganttunitwidth" << ::std::endl;
  s << "\\yzero=\\ganttextraheight\\ganttunitheight" << ::std::endl;
  s << "\\drawline(\\xzero,\\yzero)(\\xone,\\yzero)" << ::std::endl;

  // left line
  s << "\\xzero=\\gantthalfextrawidth\\ganttunitwidth" << ::std::endl;
  s << "\\yzero=0" << ::std::endl;
  s << "\\yone=" << pic_height << "\\ganttunitheight" << ::std::endl;
  s << "\\advance\\yone by \\ganttextraheight\\ganttunitheight" << ::std::endl;
  s << "\\drawline(\\xzero,\\yzero)(\\xzero,\\yone)" << ::std::endl;

  // event lines/times
  for (index_type k = 0; k < n_intervals(); k++) {
#ifdef NTYPE_RATIONAL
    double x0 = interval_start_time(k).decimal();
#else
    double x0 = interval_start_time(k);
#endif
    s << "\\xzero=" << x0 << "\\ganttunitwidth" << ::std::endl;
    s << "\\advance\\xzero by \\gantthalfextrawidth\\ganttunitwidth"
      << ::std::endl;
    s << "\\yzero=0" << ::std::endl;
    s << "\\yone=" << pic_height << "\\ganttunitheight" << ::std::endl;
    s << "\\advance\\yone by \\ganttextraheight\\ganttunitheight" << ::std::endl;
    if (interval_start_time(k) > interval_start_time(0)) {
      s << "\\dashline{1}(\\xzero,\\yzero)(\\xzero,\\yone)"
	<< ::std::endl;
    }
    s << "\\advance\\xzero by \\gantttextxoff" << ::std::endl;
    s << "\\put(\\xzero,0){\\makebox(0,0)[lb]{\\smash{{\\footnotesize{\\tt "
      << PRINT_NTYPE(interval_start_time(k)) << "}}}}}" << ::std::endl;
  }

  for (index_type k = 0; k < n_intervals(); k++) {
#ifdef NTYPE_RATIONAL
    double x0 = interval_start_time(k).decimal();
    double x1 = interval_end_time(k).decimal();
    double y0 = amount_available(k).decimal();
    double y1 = amount_in_use(k).decimal();
#else
    double x0 = interval_start_time(k);
    double x1 = interval_end_time(k);
    double y0 = amount_available(k);
    double y1 = amount_in_use(k);
#endif
    s << "\\xzero=" << x0 << "\\ganttunitwidth" << ::std::endl;
    s << "\\advance\\xzero by \\gantthalfextrawidth\\ganttunitwidth"
      << ::std::endl;
    if (FINITE(interval_end_time(k))) {
      s << "\\xone=" << x1 << "\\ganttunitwidth" << ::std::endl;
      s << "\\advance\\xone by \\gantthalfextrawidth\\ganttunitwidth"
	<< ::std::endl;
    }
    else {
      s << "\\xone=" << pic_width << "\\ganttunitwidth" << ::std::endl;
      s << "\\advance\\xone by \\ganttextrawidth\\ganttunitwidth" << ::std::endl;
    }
    s << "\\yzero=" << y0 << "\\ganttunitheight" << ::std::endl;
    s << "\\advance\\yzero by \\ganttextraheight\\ganttunitheight"
      << ::std::endl;
    s << "\\drawline(\\xzero,\\yzero)(\\xone,\\yzero)" << ::std::endl;
    s << "\\yone=" << y1 << "\\ganttunitheight" << ::std::endl;
    s << "\\advance\\yone by \\ganttextraheight\\ganttunitheight" << ::std::endl;
    s << "\\dashline{1}(\\xzero,\\yone)(\\xone,\\yone)" << ::std::endl;
  }
  s << "\\end{picture}" << ::std::endl;

  s << ::std::endl << "{\\small\\begin{tabular}{|l|l|}"
    << ::std::endl << "\\hline"
    << ::std::endl << "\\multicolumn{2}{|c|}{Events}\\\\"
    << ::std::endl << "\\hline"
    << ::std::endl;
  for (index_type i = 0; i < n_intervals(); i++) {
    for (index_type k = 0; k < a_finish[i].length(); k++) {
      if (k == 0) s << PRINT_NTYPE(interval_start_time(i));
      s << " & " << " finish ";
      instance.actions[a_finish[i][k]].name->write(s, Name::NC_LATEX);
      s << "\\\\" << ::std::endl;
    }
    for (index_type k = 0; k < a_start[i].length(); k++) {
      if ((k + a_finish[i].length()) == 0)
	s << PRINT_NTYPE(interval_start_time(i));
      s << " & " << " start ";
      instance.actions[a_start[i][k]].name->write(s, Name::NC_LATEX);
      s << "\\\\" << ::std::endl;
    }
    if ((a_start[i].length() + a_finish[i].length()) > 0) {
      s << "\\hline" << ::std::endl;
    }
  }
  s << "\\end{tabular}}" << ::std::endl;
}

ExecTrace::ExecTrace(Instance& ins)
  : lvector<ExecState*>(0, 0), instance(ins)
{
  // done
}

ExecTrace::~ExecTrace()
{
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k]) delete (*this)[k];
}

index_type ExecTrace::n_intervals()
{
  return length();
}

index_type ExecTrace::n_points()
{
  if (length() == 0) return 0;
  if (FINITE(final_state()->end_time())) return length() + 1;
  return length();
}

index_type ExecTrace::interval_start_point(index_type i)
{
  if (i >= length()) {
    ::std::cerr << "error: invalid interval " << i
	      << " in ExecTrace::interval_start_point"
	      << ::std::endl;
    exit(255);
  }
  return i;
}

index_type ExecTrace::interval_end_point(index_type i)
{
  if (i >= length()) {
    ::std::cerr << "error: invalid interval " << i
	      << " in ExecTrace::interval_end_point"
	      << ::std::endl;
    exit(255);
  }
  if (i == (length() - 1)) {
    if (FINITE(final_state()->end_time())) return i + 1;
    return no_such_index;
  }
  else {
    return i + 1;
  }
}

NTYPE ExecTrace::point_time(index_type i)
{
  if (i > length()) {
    ::std::cerr << "error: invalid point " << i
	      << " in ExecTrace::point_time"
	      << ::std::endl;
    exit(255);
  }
  else if (i == length()) {
    if (FINITE(final_state()->end_time())) {
      return final_state()->end_time();
    }
    else {
      ::std::cerr << "error: invalid point " << i
		<< " in ExecTrace::point_time (open last interval)"
		<< ::std::endl;
      exit(255);
    }
  }
  else {
    return (*this)[i]->current_time();
  }
}

NTYPE ExecTrace::interval_start_time(index_type i)
{
  if (i >= length()) {
    ::std::cerr << "error: invalid interval " << i
	      << " in ExecTrace::interval_start_time"
	      << ::std::endl;
    exit(255);
  }
  return (*this)[i]->current_time();
}

NTYPE ExecTrace::interval_end_time(index_type i)
{
  if (i >= length()) {
    ::std::cerr << "error: invalid interval " << i
	      << " in ExecTrace::interval_start_time"
	      << ::std::endl;
    exit(255);
  }
  return (*this)[i]->end_time();
}

ExecState* ExecTrace::final_state()
{
  if (length() > 0) {
    return (*this)[length() - 1];
  }
  else {
    return 0;
  }
}

ExecTrace* ExecTrace::copy()
{
  ExecTrace* t = new ExecTrace(instance);
  for (index_type k = 0; k < length(); k++) {
    assert((*this)[k]);
    t->append((ExecState*)(*this)[k]->copy());
  }
  return t;
}

ExecTrace* ExecTrace::necessary_trace()
{
  assert(length() > 0);
  ExecTrace* t = copy();
  bool_vec nec(instance.goal_atoms, instance.n_atoms());
  for (index_type k = length(); k > 0; k--) {
    (*t)[k - 1]->intersect(nec);
    index_set af;
    (*t)[k - 1]->finishing_actions(af);
    for (index_type i = 0; i < af.length(); i++)
      nec.subtract(instance.actions[af[i]].add);
    index_set as;
    (*t)[k - 1]->starting_actions(as);
    for (index_type i = 0; i < as.length(); i++)
      nec.insert(instance.actions[as[i]].pre);
  }
  return t;
}

void ExecTrace::clip_last_state(NTYPE at_t)
{
  if (length() > 0) {
    (*this)[length() - 1]->clip(at_t);
  }
}

void ExecTrace::write(::std::ostream& s)
{
  for (index_type k = 0; k < length(); k++) {
    (*this)[k]->write(s);
  }
}

void ExecTrace::peak_resource_use(amt_vec& res)
{
  res.assign_value(0, instance.n_resources());
  amt_vec cur(0, instance.n_resources());
  for (index_type k = 0; k < length(); k++) {
    (*this)[k]->current_resource_use(cur);
    for (index_type r = 0; r < instance.n_resources(); r++)
      if (cur[r] > res[r]) res[r] = cur[r];
  }
}

bool ExecTrace::test_always(index_type p)
{
  for (index_type k = 0; k < length(); k++) {
    if (!(*this)[k]->current_atoms()[p]) return false;
  }
  return true;
}

bool ExecTrace::test_sometime(index_type p)
{
  for (index_type k = 0; k < length(); k++) {
    if ((*this)[k]->current_atoms()[p]) return true;
  }
  return false;
}

bool ExecTrace::test_sometime_after(index_type p, index_type q)
{
  bool f = false;
  for (index_type k = 0; k < length(); k++) {
    if (f) {
      if ((*this)[k]->current_atoms()[q]) return true;
    }
    else {
      if ((*this)[k]->current_atoms()[p]) f = true;
      if ((*this)[k]->current_atoms()[q]) return true;
    }
  }
  return !f;
}

bool ExecTrace::test_sometime_before(index_type p, index_type q)
{
  for (index_type k = 0; k < length(); k++) {
    if ((*this)[k]->current_atoms()[q]) {
      return true;
    }
    else if ((*this)[k]->current_atoms()[p]) {
      return false;
    }
  }
  return true;
}

bool ExecTrace::test_at_most_once(index_type p)
{
  bool f = false;
  bool g = false;
  for (index_type k = 0; k < length(); k++) {
    if (g) {
      if ((*this)[k]->current_atoms()[p]) return false;
    }
    else if (f) {
      if (!(*this)[k]->current_atoms()[p]) g = true;
    }
    else {
      if ((*this)[k]->current_atoms()[p]) f = true;
    }
  }
  return true;
}

void ExecTrace::extract_always_within(bool_matrix& c, cost_matrix& t)
{
  assert(length() > 0);
  c.assign_value(false, instance.n_atoms(), instance.n_atoms());
  t.assign_value(0, instance.n_atoms(), instance.n_atoms());
  bool_matrix is_open(false, instance.n_atoms(), instance.n_atoms());
  cost_matrix open_since(0, instance.n_atoms(), instance.n_atoms());
  for (index_type k = 0; k < length(); k++) {
    NTYPE tk = (*this)[k]->current_time();
    for (index_type i = 0; i < instance.n_atoms(); i++) {
      for (index_type j = 0; j < instance.n_atoms(); j++) {
	if ((*this)[k]->current_atoms()[i] && !is_open[i][j]) {
	  is_open[i][j] = true;
	  open_since[i][j] = tk;
	  c[i][j] = true;
	}
	if ((*this)[k]->current_atoms()[j] && is_open[i][j]) {
	  is_open[i][j] = false;
	  NTYPE d = (tk - open_since[i][j]);
	  t[i][j] = MAX(d, t[i][j]);
	}
      }
    }
  }
  for (index_type i = 0; i < instance.n_atoms(); i++)
    for (index_type j = 0; j < instance.n_atoms(); j++)
      if (is_open[i][j]) c[i][j] = false;
}

bool Schedule::write_traits = true;

Schedule::Schedule(Instance& i)
  : instance(i),
    end_t(0),
    action_set(EMPTYSET),
    n_tracks(0),
    ann_name(0),
    ann_optimal(false),
    traits(0, 0),
    current_t(0),
    finished(false),
    trace_level(Instance::default_trace_level)
{
  // done
}

Schedule::Schedule(const Schedule& s)
  : instance(s.instance),
    steps(s.steps),
    end_t(s.end_t),
    action_set(s.action_set),
    action_vec(s.action_vec),
    n_tracks(s.n_tracks),
    ann_name(s.ann_name),
    ann_optimal(s.ann_optimal),
    traits(0, 0),
    current_t(s.current_t),
    finished(s.finished),
    trace_level(Instance::default_trace_level)
{
  // done - note that traits are NOT copied
}

Schedule::~Schedule()
{
  // delete traits - schedule owns them
  for (index_type k = 0; k < traits.length(); k++)
    delete traits[k];
}

index_type Schedule::length() const
{
  return steps.length();
}

NTYPE Schedule::makespan() const
{
  return end_t;
}

NTYPE Schedule::cost() const
{
  NTYPE c = 0;
  for (index_type k = 0; k < steps.length(); k++)
    c += instance.actions[steps[k].act].cost;
  return c;
}

void Schedule::step_action_names(name_vec& nv)
{
  nv.assign_value(0, steps.length());
  for (index_type k = 0; k < steps.length(); k++)
    nv[k] = instance.actions[steps[k].act].name;
}

bool Schedule::step_in_interval
(index_type s, NTYPE i_start, NTYPE i_end) const
{
  assert(s < steps.length());
  if ((steps[s].at + instance.actions[steps[s].act].dur) <= i_start)
    return false;
  if (steps[s].at >= i_end)
    return false;
  return true;
}

index_type Schedule::step_action(index_type s) const
{
  assert(s < steps.length());
  return steps[s].act;
}

const Name* Schedule::plan_name() const
{
  return ann_name;
}

bool Schedule::plan_is_optimal() const
{
  return ann_optimal;
}

void Schedule::clear()
{
  steps.clear();
  end_t = 0;
  current_t = 0;
  finished = false;
}

void Schedule::protect(index_type atom)
{
  // ignored
}

void Schedule::insert_step(NTYPE at, index_type act)
{
  index_type k = 0;
  while (k < steps.length()) {
    if (at < steps[k].at) {
      steps.insert(step(act, at), k);
      return;
    }
    k += 1;
  }
  steps.append(step(act, at));
}

void Schedule::insert(index_type act)
{
  if (finished) {
    ::std::cerr << "program error: schedule::insert called after schedule::end"
	      << ::std::endl;
    exit(255);
  }
  insert_step(current_t, act);
}

void Schedule::advance(NTYPE delta)
{
  current_t += delta;
}

void Schedule::set_start_time(NTYPE t)
{
  if (steps.length() == 0) return;
  NTYPE d = (t - steps[0].at);
  if (d != 0) {
    for (index_type k = 0; k < steps.length(); k++) {
      steps[k].at += d;
    }
    end_t += d;
  }
}

void Schedule::compute_action_set_and_vec()
{
  action_set.clear();
  action_vec.set_length(0);
  for (index_type k = 0; k < steps.length(); k++) {
    action_set.insert(steps[k].act);
    action_vec.append(steps[k].act);
  }
}

void Schedule::end()
{
  if (finished) {
    ::std::cerr << "program error: schedule::end called twice" << ::std::endl;
    exit(255);
  }
  end_t = 0;
  for (index_type k = 0; k < steps.length(); k++)
    end_t = MAX(end_t, steps[k].at + instance.actions[steps[k].act].dur);
  set_start_time(0);
  assign_tracks();
  compute_action_set_and_vec();
  finished = true;
}

void Schedule::reduce(ExecErrorSet* warnings)
{
  if (!finished) {
    ::std::cerr << "program error: schedule::remove_redundant_steps called before schedule::end" << ::std::endl;
    exit(255);
  }
  bool_vec steps_to_remove(false, steps.length());
  index_type k = 0;
  while (k < warnings->length()) {
    if (((*warnings)[k]->type_of_error() ==
	 ExecError::warning_redundant_action) &&
	((*warnings)[k]->step_of_error() != no_such_index)) {
      steps_to_remove[(*warnings)[k]->step_of_error()] = true;
      warnings->remove(k);
    }
    else {
      k += 1;
    }
  }
  if (trace_level > 0) {
    ::std::cerr << "removing " << steps_to_remove.count(true)
	      << " redudant steps from plan";
    if (plan_name()) ::std::cerr << " " << plan_name();
    ::std::cerr << "..." << ::std::endl;
  }
  if (steps_to_remove.count(true) > 0) {
    mapping map(steps.length());
    steps.remove(steps_to_remove, map);
    compute_action_set_and_vec();
    for (index_type k = 0; k < traits.length(); k++) {
      PlanPrecedenceRelation* pp =
	(PlanPrecedenceRelation*)
	traits[k]->cast_to("PlanPrecedenceRelation");
      if (pp) {
	graph p_new;
	p_new.copy(pp->precedence_relation(), map);
	delete traits[k];
	traits[k] = new PlanPrecedenceRelation(this, p_new);
      }
    }
    warnings->remap_steps(map);
  }
}

void Schedule::set_name(const Name* n)
{
  ann_name = n;
}

void Schedule::set_optimal(bool o)
{
  ann_optimal = o;
}

void Schedule::add_trait(PlanTrait* t)
{
  traits.append(t);
}

const PlanTrait* Schedule::find_trait(const char* cn)
{
  for (index_type k = 0; k < traits.length(); k++) {
    const PlanTrait* t = traits[k]->cast_to(cn);
    if (t) return t;
  }
  return 0;
}

void Schedule::output(Plan& plan) const
{
  NTYPE t = 0;
  for (index_type k = 0; k < steps.length(); k++) {
    NTYPE dt = steps[k].at - t;
    if (dt > 0) plan.advance(dt);
    plan.insert(steps[k].act);
    t = steps[k].at;
  }
  if (t < end_t) plan.advance(end_t - t);
  plan.end();
}

void Schedule::output
(Plan& plan, const index_vec& act_map) const
{
  NTYPE t = 0;
  for (index_type k = 0; k < steps.length(); k++) {
    NTYPE dt = steps[k].at - t;
    if (dt > 0) plan.advance(dt);
    plan.insert(act_map[steps[k].act]);
    t = steps[k].at;
  }
  if (t < end_t) plan.advance(end_t - t);
  plan.end();
}

void Schedule::write(::std::ostream& s, unsigned int c) const
{
  s << "(:plan";
  if (ann_name) {
    s << ::std::endl << "  :name ";
    ann_name->write(s, c);
  }
  if (ann_optimal) {
    s << ::std::endl << "  :opt";
  }
  for (index_type k = 0; k < steps.length(); k++) {
    Instance::Action& act = instance.actions[steps[k].act];
    s << ::std::endl << "  " << PRINT_NTYPE(steps[k].at)
      << " : ";
    if (Name::context_is_instance(c)) {
      s << "(";
      act.name->write(s, c);
      s << ")";
    }
    else {
      act.name->write(s, c);
    }
  }
  s << ")" << ::std::endl;
  s << ";; length: " << length() << ::std::endl;
  s << ";; makespan: " << PRINT_NTYPE(makespan()) << ::std::endl;
  s << ";; cost: " << PRINT_NTYPE(cost()) << ::std::endl;
  if (write_traits) {
    s << ";; " << traits.length() << " traits: ";
    bool first = true;
    for (index_type k = 0; k < traits.length(); k++) {
      const ScheduleTrait* t =
	(const ScheduleTrait*)traits[k]->cast_to("ScheduleTrait");
      if (t) {
	if (!first) s << ", ";
	first = false;
	t->write_short(s);
      }
    }
  }
  s << std::endl;
}

void Schedule::write_step_set(::std::ostream& s, const index_set& set) const
{
  for (index_type k = 0; k < set.length(); k++) {
    if (k > 0) s << ", ";
    s << "[" << steps[set[k]].at << " (#" << set[k] << ")] "
      << instance.actions[steps[set[k]].act].name;
  }
}

void Schedule::write_steps(::std::ostream& s) const
{
  index_set all;
  all.fill(n_steps());
  write_step_set(s, all);
}

NTYPE Schedule::next_start_time(NTYPE t) const
{
  NTYPE next_t = POS_INF;
  for (index_type k = 0; k < steps.length(); k++) {
    NTYPE d = steps[k].at - t;
    if ((d > 0) && (d < (next_t - t))) next_t = steps[k].at;
  }
  return next_t;
}

NTYPE Schedule::next_finish_time(NTYPE t) const
{
  NTYPE next_t = POS_INF;
  for (index_type k = 0; k < steps.length(); k++) {
    NTYPE f = (steps[k].at + instance.actions[steps[k].act].dur);
    if ((f > t) && (f < next_t)) next_t = f;
  }
  return next_t;
}

NTYPE Schedule::last_finish_time() const
{
  NTYPE next_t = 0;
  for (index_type k = 0; k < steps.length(); k++) {
    NTYPE f = (steps[k].at + instance.actions[steps[k].act].dur);
    if (f > next_t) next_t = f;
  }
  return next_t;
}

double Schedule::GANTT_UNIT_WIDTH = 1;
double Schedule::GANTT_UNIT_HEIGHT = 1;
double Schedule::GANTT_TEXT_XOFF = 0;
double Schedule::GANTT_TEXT_YOFF = 0;
NTYPE  Schedule::GANTT_TIME_MARK_INTERVAL = 100;
bool   Schedule::GANTT_ACTION_NAMES_ON_CHART = true;

void Schedule::writeGantt(::std::ostream& s) const
{
#ifdef NTYPE_RATIONAL
  double pic_width = makespan().decimal();
#else
  double pic_width = makespan();
#endif
  double pic_height = (n_tracks + 2);
  s << "\\newcommand{\\ganttunitwidth}{" << GANTT_UNIT_WIDTH << "}"
    << ::std::endl;
  s << "\\newcommand{\\ganttunitheight}{" << GANTT_UNIT_HEIGHT << "}"
    << ::std::endl;
  s << "\\newcommand{\\gantttextxoff}{" << GANTT_TEXT_XOFF << "}"
    << ::std::endl;
  s << "\\newcommand{\\gantttextyoff}{" << GANTT_TEXT_YOFF << "}"
    << ::std::endl;
  s << "\\newcount\\x" << ::std::endl;
  s << "\\newcount\\y" << ::std::endl;
  s << "\\begin{picture}("
    << pic_width << "\\ganttunitwidth,"
    << pic_height << "\\ganttunitheight)(0,0)"
    << ::std::endl;
  for (index_type k = 0; k < steps.length(); k++) {
    Instance::Action& a = instance.actions[steps[k].act];
    s << "%% [" << steps[k].at << "," << steps[k].at + a.dur
      << "] " << a.name << " on track " << steps[k].track << ::std::endl;
#ifdef NTYPE_RATIONAL
    double x0 = steps[k].at.decimal();
    double x1 = (steps[k].at+a.dur).decimal();
#else
    double x0 = steps[k].at;
    double x1 = (steps[k].at+a.dur);
#endif
    double y0 = (steps[k].track + 1);
    double y1 = (steps[k].track + 2);
    s << "\\path(" << x0 << "\\ganttunitwidth," << y0
      << "\\ganttunitheight)(" << x0 << "\\ganttunitwidth," << y1
      << "\\ganttunitheight)(" << x1 << "\\ganttunitwidth," << y1
      << "\\ganttunitheight)(" << x1 << "\\ganttunitwidth," << y0
      << "\\ganttunitheight)(" << x0 << "\\ganttunitwidth," << y0
      << "\\ganttunitheight)" << ::std::endl;
    s << "\\x=" << x0 << "\\ganttunitwidth" << ::std::endl;
    s << "\\advance\\x by \\gantttextxoff" << ::std::endl;
    s << "\\y=" << y0 << "\\ganttunitheight" << ::std::endl;
    s << "\\advance\\y by \\gantttextyoff" << ::std::endl;
    if (GANTT_ACTION_NAMES_ON_CHART) {
      s << "\\put(\\x,\\y){\\makebox(0,0)[lb]{\\smash{{\\footnotesize{\\tt ";
      a.name->write(s, Name::NC_LATEX);
      s << "}}}}}" << ::std::endl;
    }
    else {
      s << "\\put(\\x,\\y){\\makebox(0,0)[lb]{\\smash{{\\footnotesize{\\tt "
	<< a.index << "}}}}}" << ::std::endl;
    }
  }
  s << "\\end{picture}" << ::std::endl;

  if (!GANTT_ACTION_NAMES_ON_CHART) {
    s << ::std::endl << "{\\small\\begin{tabular}{|l|l|}"
      << ::std::endl << "\\hline"
      << ::std::endl << "\\multicolumn{2}{|c|}{Actions}\\\\"
      << ::std::endl << "\\hline"
      << ::std::endl;
    for (index_type k = 0; k < action_set.length(); k++) {
      s << "\\#" << action_set[k] << " & ";
      instance.actions[action_set[k]].name->write(s, Name::NC_LATEX);
      s << "\\\\" << ::std::endl << "\\hline" << ::std::endl;
    }
    s << "\\end{tabular}}" << ::std::endl;
  }
}

void Schedule::assign_tracks()
{
  for (index_type k = 0; k < steps.length(); k++)
    steps[k].track = no_such_index;
  n_tracks = 0;

  bool_vec   occ(false, 0);
  amt_vec    occ_to;
  NTYPE      t = 0;
  while (FINITE(t)) {
    for (index_type k = 0; k < steps.length(); k++) if (steps[k].at == t) {
      index_type p = occ.first(false);
      if (p == no_such_index) {
	occ.append(false);
	p = occ.length() - 1;
      }
      occ[p] = true;
      // occ_to.inc_length_to(p + 1, 0);
      occ_to[p] = t + instance.actions[steps[k].act].dur;
      steps[k].track = p;
      if (p >= n_tracks) n_tracks = p + 1;
    }
    t = next_start_time(t);
    for (index_type i = 0; i < occ.length(); i++) if (occ[i]) {
      if (occ_to[i] <= t) occ[i] = false;
    }
  }
}

void Schedule::writeXML
(::std::ostream& s, ExecErrorSet* errors, ExecTrace* trace, graph* prec,
 index_type id) const
{
  s << "<schedule xmlns=\"http://dummy.net/pddlcat/schedule\" id=\""
    << id << "\"";
  if (plan_name()) {
    s << " name=\"";
    plan_name()->write(s, Name::NC_XML);
    s << "\"";
  }
  s << " makespan=\"" << makespan() << "\">" << ::std::endl;

  if (prec) {
    if (prec->size() != steps.length()) {
      ::std::cerr << "error in Schedule::writeXML: size of precedence graph "
		<< *prec << " does not match number of steps "
		<< steps.length() << ::std::endl;
      exit(255);
    }
  }

  s << "<actions>" << ::std::endl;
  for (index_type k = 0; k < action_set.length(); k++) {
    s << "<action id=\"" << action_set[k] << "\" name=\"";
    instance.actions[action_set[k]].name->write(s, Name::NC_XML);
    s << "\" duration=\"" << instance.actions[action_set[k]].dur
      << "\">" << ::std::endl;
    s << "<dump><![CDATA[" << ::std::endl;
    instance.print_action(s, instance.actions[action_set[k]]);
    s << "]]></dump>" << ::std::endl;
    for (index_type i = 0; i < steps.length(); i++) {
      if (steps[i].act == action_set[k]) {
	s << "<link step=\"" << i << "\" type=\"occurs\"/>" << ::std::endl;
      }
    }
    for (index_type i = 0; i < instance.n_resources(); i++) {
      if (instance.actions[action_set[k]].use[i] > 0) {
	s << "<link resource=\"" << i << "\" type=\"use\" amount=\""
	  << instance.actions[action_set[k]].use[i] << "\"/>" << ::std::endl;
      }
      if (instance.actions[action_set[k]].cons[i] > 0) {
	s << "<link resource=\"" << i << "\" type=\"consume\" amount=\""
	  << instance.actions[action_set[k]].cons[i] << "\"/>" << ::std::endl;
      }
    }
    s << "</action>" << ::std::endl;
  }
  s << "</actions>" << ::std::endl;

  s << "<steps maxtrack=\"" << n_tracks - 1 << "\">" << ::std::endl;
  for (index_type k = 0; k < steps.length(); k++) {
    s << "<step id=\"" << k << "\" action=\"" << steps[k].act
      << "\" start=\"" << PRINT_NTYPE(steps[k].at)
      << "\" finish=\""
      << PRINT_NTYPE(steps[k].at + instance.actions[steps[k].act].dur)
      << "\" track=\"" << steps[k].track
      << "\">" << ::std::endl;
    for (index_type r = 0; r < instance.n_resources(); r++) {
      if (instance.actions[steps[k].act].use[r] > 0) {
	s << "<link resource=\"" << r << "\" type=\"use\" amount=\""
	  << instance.actions[steps[k].act].use[r] << "\"/>" << ::std::endl;
      }
      if (instance.actions[steps[k].act].cons[r] > 0) {
	s << "<link resource=\"" << r << "\" type=\"consume\" amount=\""
	  << instance.actions[steps[k].act].cons[r] << "\"/>" << ::std::endl;
      }
    }
    if (prec) {
      for (index_type i = 0; i < steps.length(); i++)
	if (prec->adjacent(k, i)) {
	  s << "<link step=\"" << i << "\" type=\"prec\"/>" << ::std::endl;
	}
    }
    s << "</step>" << ::std::endl;
  }
  s << "</steps>" << ::std::endl;

  if (errors) {
    s << "<errors>" << ::std::endl;
    for (index_type k = 0; k < errors->length(); k++) {
      s << "<error id=\"error" << k << "\" type=\""
	<< ExecError::error_type_string((*errors)[k]->type_of_error())
	<< "\" severity=\""
	<< ExecError::error_severity_string((*errors)[k]->severity_of_error())
	<< "\" time=\"" << PRINT_NTYPE((*errors)[k]->time_of_error())
	<< "\"";
      if ((*errors)[k]->step_of_error() != no_such_index) {
	s << " step=\"" << (*errors)[k]->step_of_error() << "\"";
      }
      s << ">" << ::std::endl;
      s << "<dump><![CDATA[" << ::std::endl;
      (*errors)[k]->write(s);
      s << "]]></dump>" << ::std::endl;
      if ((*errors)[k]->type_of_error() == ExecError::error_resource_conflict) {
	ResourceConflictError* rce = (ResourceConflictError*)(*errors)[k];
	s << "<link resource=\"" << rce->resource().index << "\"/>";
	for (index_type i = 0; i < rce->conflict_actions().length(); i++) {
	  s << "<link action=\"" << rce->conflict_actions()[i] << "\"/>";
	}
	for (index_type i = 0; i < rce->conflict_steps().length(); i++) {
	  s << "<link step=\"" << rce->conflict_steps()[i] << "\"/>";
	}
      }
      else if ((*errors)[k]->step_of_error() != no_such_index) {
	s << "<link step=\"" << (*errors)[k]->step_of_error() << "\"/>";
      }
      s << "</error>" << ::std::endl;
    }
    s << "</errors>" << ::std::endl;
  }

  if (trace) {
    s << "<resources>" << ::std::endl;
    for (index_type k = 0; k < instance.n_resources(); k++) {
      ResourceProfile* p = new ResourceProfile(instance, k, *trace);
      p->set_makespan(makespan());
      s << "<resource id=\"" << k << "\" name=\"";
      instance.resources[k].name->write(s, Name::NC_XML);
      s << "\" initial-capacity=\"" << PRINT_NTYPE(instance.resources[k].init)
	<< "\" peak-use=\"" << PRINT_NTYPE(p->peak_use())
	<< "\" peak-use-percent=\""
	<< PRINT_NTYPE((p->peak_use() * 100) / instance.resources[k].init)
	<< "\" min-free=\"" << PRINT_NTYPE(p->min_free())
	<< "\" tolerable-loss=\"" << PRINT_NTYPE(p->tolerable_unexpected_loss())
	<< "\">" << ::std::endl;
      s << "<dump><![CDATA[" << ::std::endl;
      instance.print_resource(s, instance.resources[k]);
      s << "]]></dump>" << ::std::endl;
      s << "<profile name=\"available\">" << ::std::endl;
      for (index_type i = 0; i < p->n_intervals(); i++) {
	s << "<interval start=\"" << PRINT_NTYPE(p->interval_start_time(i))
	  << "\" end=\"" << PRINT_NTYPE(p->interval_end_time(i))
	  << "\" amount=\"" << PRINT_NTYPE(p->amount_available(i))
	  << "\"/>" << ::std::endl;
      }
      s << "</profile>" << ::std::endl;
      s << "<profile name=\"in_use\">" << ::std::endl;
      for (index_type i = 0; i < p->n_intervals(); i++) {
	s << "<interval start=\"" << PRINT_NTYPE(p->interval_start_time(i))
	  << "\" end=\"" << PRINT_NTYPE(p->interval_end_time(i))
	  << "\" amount=\"" << PRINT_NTYPE(p->amount_in_use(i))
	  << "\"/>" << ::std::endl;
      }
      s << "</profile>" << ::std::endl;
      s << "<profile name=\"free\">" << ::std::endl;
      for (index_type i = 0; i < p->n_intervals(); i++) {
	s << "<interval start=\"" << PRINT_NTYPE(p->interval_start_time(i))
	  << "\" end=\"" << PRINT_NTYPE(p->interval_end_time(i))
	  << "\" amount=\"" << PRINT_NTYPE(p->amount_free(i))
	  << "\"/>" << ::std::endl;
      }
      s << "</profile>" << ::std::endl;
      for (index_type i = 0; i < action_set.length(); i++) {
	if (instance.actions[action_set[i]].use[k] > 0) {
	  s << "<link action=\"" << i << "\" type=\"use\" amount=\""
	    << instance.actions[action_set[i]].use[k] << "\"/>" << ::std::endl;
	}
	if (instance.actions[action_set[i]].cons[k] > 0) {
	  s << "<link action=\"" << i << "\" type=\"consume\" amount=\""
	    << instance.actions[action_set[i]].cons[k] << "\"/>" << ::std::endl;
	}
      }
      for (index_type i = 0; i < steps.length(); i++) {
	if (instance.actions[steps[i].act].use[k] > 0) {
	  s << "<link step=\"" << i << "\" type=\"use\" amount=\""
	    << instance.actions[steps[i].act].use[k] << "\"/>" << ::std::endl;
	}
	if (instance.actions[steps[i].act].cons[k] > 0) {
	  s << "<link step=\"" << i << "\" type=\"consume\" amount=\""
	    << instance.actions[steps[i].act].cons[k] << "\"/>" << ::std::endl;
	}
      }
      s << "</resource>" << ::std::endl;
    }
    s << "</resources>" << ::std::endl;
  }

  s << "<traits>" << ::std::endl;
  for (index_type k = 0; k < traits.length(); k++) {
    const ScheduleTrait* t =
      (const ScheduleTrait*)traits[k]->cast_to("ScheduleTrait");
    if (t) {
      t->writeXML(s);
    }
  }
  s << "</traits>" << ::std::endl;

  s << "</schedule>" << ::std::endl;
}

bool Schedule::simulate
(ExecTrace* trace, ExecErrorSet* errors, bool finish) const
{
  if (trace_level > 0) {
    ::std::cerr << "simulating (high resolution)..." << ::std::endl;
  }

  ExecState cs(instance, instance.init_atoms);
  cs.set_trace_level(trace_level - 1);
  if (trace) {
    trace->append((ExecState*)cs.copy());
  }

  NTYPE t = 0;
  bool_vec done(false, steps.length());
  bool executable = true;

  while (t < POS_INF) {
    bool any_starting = false;
    for (index_type k = 0; k < steps.length(); k++)
      if ((steps[k].at == t) && !done[k]) {
	Instance::Action& act = instance.actions[steps[k].act];
	if (!cs.applicable(act, errors, k)) {
	  if (trace_level > 0) {
	    ::std::cerr << steps[k].at << ": " << act.name << " NOT applicable"
			<< ::std::endl;
	  }
	  executable = false;
	  if (!finish) return false;
	}
	cs.apply(act, errors, k);
	done[k] = true;
	any_starting = true;
      }
    if (any_starting && trace) {
      trace->clip_last_state(cs.current_time());
      trace->append((ExecState*)cs.copy());
    }
    if (trace_level > 2) {
      cs.write(::std::cerr);
    }

    if (cs.min_delta() == 0) {
      std::cerr << "warning: zero duration state at " << t
		<< " in Schedule::simulate" << std::endl;
    }
    NTYPE next_t = MIN(t + cs.min_delta(), next_start_time(t));
    if (next_t == t) {
      std::cerr << "warning: empty interval at " << t
		<< " in Schedule::simulate (min delta = "
		<< cs.min_delta() << ", next start = "
		<< next_start_time(t) << ")"
		<< std::endl;
    }
    if (trace_level > 2) {
      ::std::cerr << "at " << t << ": " << ::std::endl << cs;
      ::std::cerr << "next_t = min(" << t + cs.min_delta()
		  << ", " << next_start_time(t) << ") = " << next_t
		  << ::std::endl;
    }
    if (FINITE(next_t)) {
      cs.advance(next_t - t, errors);
      if (trace) {
	trace->clip_last_state(cs.current_time());
	trace->append((ExecState*)cs.copy());
      }
    }
    else {
      assert(cs.n_current_actions() == 0);
    }
    t = next_t;
  }

  bool accept = cs.is_final(errors);
  if (trace_level > 0) {
    if (accept) ::std::cerr << "goals achieved" << ::std::endl;
    else ::std::cerr << "goals NOT achieved" << ::std::endl;
  }

  return (executable && accept);
}

bool Schedule::simulate_low_resolution
(ExecTrace* trace, ExecErrorSet* errors, bool finish) const
{
  if (trace_level > 0) {
    ::std::cerr << "simulating (low resolution)..." << ::std::endl;
  }

  ExecState cs(instance, instance.init_atoms);
  cs.set_trace_level(trace_level - 1);
  if (trace) {
    trace->append((ExecState*)cs.copy());
  }
  if (trace_level > 1) {
    cs.write(::std::cerr);
  }

  NTYPE t = 0;
  bool executable = true;
  for (index_type k = 0; k < steps.length(); k++) {
    NTYPE dt = steps[k].at - t;
    Instance::Action& act = instance.actions[steps[k].act];
    if (dt > 0) cs.advance(dt, errors);
    if (!cs.applicable(act, errors, k)) {
      if (trace_level > 0) {
	::std::cerr << steps[k].at << ": " << act.name << " NOT applicable"
		  << ::std::endl;
      }
      executable = false;
      if (!finish) return false;
    }
    cs.apply(act, errors, k);
    if (trace) {
      trace->clip_last_state(cs.current_time());
      trace->append((ExecState*)cs.copy());
    }
    t = steps[k].at;
  }

  cs.finish(errors);
  if (trace) {
    trace->clip_last_state(cs.current_time());
    trace->append((ExecState*)cs.copy());
  }
  if (trace_level > 1) {
    cs.write(::std::cerr);
  }

  bool accept = cs.is_final(errors);
  if (trace_level > 0) {
    if (accept) ::std::cerr << "goals achieved" << ::std::endl;
    else ::std::cerr << "goals NOT achieved" << ::std::endl;
  }

  return (executable && accept);
}

bool Schedule::simulate(index_set& achieved, ExecErrorSet* errors) const
{
  ExecTrace* trace = new ExecTrace(instance);
  bool own_errors = false;
  if (errors == 0) {
    errors = new ExecErrorSet();
    own_errors = true;
  }
  simulate(trace, errors);
  bool ok = errors->executable();
  achieved.clear();
  if (ok) {
    assert(trace->length() > 0);
    index_type l = trace->length() - 1;
    (*trace)[l]->current_atoms(achieved);
  }
  delete trace;
  if (own_errors) delete errors;
  return ok;
}

bool Schedule::simulate(amt_vec& rtl) const
{
  ExecTrace* trace = new ExecTrace(instance);
  bool ok = simulate(trace);
  if (ok) {
    for (index_type k = 0; k < instance.n_resources(); k++) {
      ResourceProfile* p = new ResourceProfile(instance, k, *trace);
      rtl[k] = p->tolerable_unexpected_loss();
      delete p;
    }
  }
  delete trace;
  return ok;
}

void Schedule::deorder(graph& prec) const
{
  prec.init(steps.length());
  for (index_type i = 0; i < steps.length(); i++)
    for (index_type j = 0; j < steps.length(); j++)
      if ((steps[i].at + instance.actions[steps[i].act].dur) <= steps[j].at)
	if (!instance.commutative(steps[i].act, steps[j].act) ||
	    !instance.lock_compatible(steps[i].act, steps[j].act))
	  prec.add_edge(i, j);
}

void Schedule::base_precedence_graph(graph& prec) const
{
  prec.init(steps.length());
  for (index_type i = 0; i < steps.length(); i++)
    for (index_type j = 0; j < steps.length(); j++)
      if ((steps[i].at + instance.actions[steps[i].act].dur) <= steps[j].at)
	prec.add_edge(i, j);
}

void Schedule::deorder(weighted_graph& prec) const
{
  prec.init(steps.length());
  for (index_type i = 0; i < steps.length(); i++)
    for (index_type j = 0; j < steps.length(); j++)
      if ((steps[i].at + instance.actions[steps[i].act].dur) <= steps[j].at)
	if (!instance.commutative(steps[i].act, steps[j].act) ||
	    !instance.lock_compatible(steps[i].act, steps[j].act))
	  prec.add_edge(i, j, instance.actions[steps[i].act].dur);
}

bool Schedule::schedule
(const index_vec& acts, const graph& prec, index_vec* sindex)
{
  assert(prec.size() == acts.length());
  graph p(prec); // copy of precedence graph (need to modify it)
  index_set r;   // set of "ready" steps (min. in prec graph, not scheduled)
  index_vec s;   // already scheduled steps
  for (index_type k = 0; k < p.size(); k++)
    if (p.in_degree(k) == 0) r.insert(k);
  amt_vec f(0, instance.n_resources());  // currently available resources
  for (index_type k = 0; k < instance.n_resources(); k++)
    f[k] = instance.resources[k].init;

#ifdef TRACE_PRINT_LOTS
  write_labeled_digraph<index_vec>
    (std::cerr, prec, acts, false, "Precedence Graph");
#endif

  clear();
  while (s.length() < acts.length()) {
#ifdef TRACE_PRINT_LOTS
    ::std::cerr << "r = " << r << ::std::endl;
    ::std::cerr << "f = " << f << ::std::endl;
#endif
    // find next step that can be scheduled at current time
    index_type n = no_such_index;
    for (index_type k = 0; (k < r.length()) && (n == no_such_index); k++) {
      bool can_go = true;
      for (index_type i = 0; (i < instance.n_resources()) && can_go; i++)
	if (f[i] < instance.actions[acts[r[k]]].req(i)) {
#ifdef TRACE_PRINT_LOTS
	  ::std::cerr << "step " << r[k] << ":"
		    << instance.actions[acts[r[k]]].name
		    << " can not be scheduled at current time = "
		    << current_t
		    << " because it needs "
		    << instance.actions[acts[r[k]]].req(i)
		    << " of resource "
		    << i << ":" << instance.resources[i].name
		    << " and only " << f[i] << " is available"
		    << ::std::endl;
#endif
	  can_go = false;
	}
      if (can_go) n = r[k];
    }
#ifdef TRACE_PRINT_LOTS
    ::std::cerr << "n = " << n << ::std::endl;
#endif

    // if such a step found, insert it at current time
    if (n != no_such_index) {
      insert(n); // NOTE: this inserts the step index, not the action index!
      // reserve resources required (used + consumed) by corresponding action
      for (index_type i = 0; i < instance.n_resources(); i++)
	f[i] -= instance.actions[acts[n]].req(i);
      // remove step from ready set and add it to scheduled set
      r.subtract(n);
      s.append(n);
    }

    // if not, advance current time to earliest finishing time later
    // than current time
    else {
      NTYPE eft = POS_INF;  // earliest finishing time
      index_set e;          // steps ending at EFT
      for (index_type k = 0; k < steps.length(); k++) {
	NTYPE sft = steps[k].at + instance.actions[acts[steps[k].act]].dur;
	if (sft > current_t) {
	  if (sft < eft) {
	    eft = sft;
	    e.assign_singleton(steps[k].act);
	  }
	  else if (sft == eft) {
	    e.insert(steps[k].act);
	  }
	}
      }
      if (INFINITE(eft)) {
	if (trace_level > 1) {
	  ::std::cerr << "error: failed to schedule";
	  for (index_type k = 0; k < acts.length(); k++)
	    ::std::cerr << " " << k << ":" << instance.actions[acts[k]].name;
	  ::std::cerr << " with precedence graph " << prec
		    << ::std::endl;
	  for (index_type k = 0; k < steps.length(); k++) {
	    assert(steps[k].act < acts.length());
	    steps[k].act = acts[steps[k].act];
	  }
	  ::std::cerr << "current schedule:" << ::std::endl;
	  write(::std::cerr);
	  ::std::cerr << "current time = " << current_t << ::std::endl;
	  ::std::cerr << "ready set = ";
	  instance.write_action_set(::std::cerr, r);
	  ::std::cerr << ::std::endl;
	  ::std::cerr << "free resources:" << ::std::endl;
	  for (index_type i = 0; i < instance.n_resources(); i++) {
	    ::std::cerr << " " << instance.resources[i].name << " = "
		      << f[i] << ::std::endl;
	  }
	}
	return false;
      }
      else if (eft == current_t) {
	std::cerr << "warning: empty interval at " << current_t
		  << " in Schedule::schedule" << std::endl;
      }
#ifdef TRACE_PRINT_LOTS
      ::std::cerr << "eft = " << eft << ", e = " << e << ::std::endl;
#endif
      // for every finished step,
      for (index_type k = 0; k < e.length(); k++) {
	// remove its edges from prec graph and insert steps that become
	// minimal into ready set
 	for (index_type i = 0; i < p.size(); i++) {
 	  p.remove_edge(e[k], i);
 	  if ((p.in_degree(i) == 0) && !s.contains(i))
 	    r.insert(i);
 	}
	// return resources used (not consumed) by corresponding action
	for (index_type i = 0; i < instance.n_resources(); i++)
	  f[i] += instance.actions[acts[e[k]]].use[i];
      }
      assert((eft - current_t) > 0);
      advance(eft - current_t);
    }
  }

  // replace step indices by proper action indices in schedule
  for (index_type k = 0; k < steps.length(); k++) {
    assert(steps[k].act < acts.length());
    steps[k].act = acts[steps[k].act];
  }

  // call to end() computes the proper schedule end time (makespan) and
  // other secondaries (action_vec, action_set) - it DEPENDS on the step
  // action indices having been set right!
  end();

  if (sindex) {
    mapping::invert_map(s, *sindex);
  }

  return true;
}

bool Schedule::construct_conflict_free
(const index_vec& acts,
 const graph& prec,
 index_set& cs,
 index_vec& map)
{
  assert(prec.size() == acts.length());
  lvector<s_status> s(s_pending, acts.length());
  for (index_type k = 0; k < acts.length(); k++)
    if (prec.in_degree(k) == 0) s[k] = s_ready;
  amt_vec f(0, instance.n_resources());
  for (index_type k = 0; k < instance.n_resources(); k++)
    f[k] = instance.resources[k].init;
  index_vec e;

  clear();
  cs.clear();
  mapping::identity_map(acts.length(), map);

  while (s.contains(s_pending) || s.contains(s_ready)) {
    // find next step that can be scheduled at current time
    index_type next = s.first(s_ready);

    // if there is a ready step...
    if (next != no_such_index) {
      // ...check for resource conflicts...
      for (index_type k = 0; k < instance.n_resources(); k++)
	if (f[k] < instance.actions[acts[next]].req(k)) {
	  for (index_type i = 0; i < acts.length(); i++) {
	    // every currently executing and ready step that reqiures resource
	    // k is part of the conflict set
	    if ((s[i] == s_executing) &&
		(instance.actions[acts[i]].req(k) > 0))
	      cs.insert(i);
	    if ((s[i] == s_ready) &&
		(instance.actions[acts[i]].req(k) > 0))
	      cs.insert(i);
	  }
	  return false;
	}

      // if no resource conflict, insert next step into schedule at current
      // time and update its status
      insert(next); // NOTE: this inserts the step index, not the action index!
      s[next] = s_executing;
      // reserve resources required (used + consumed) by corresponding action
      for (index_type k = 0; k < instance.n_resources(); k++)
	f[k] -= instance.actions[acts[next]].req(k);
      map[next] = steps.length() - 1;
    }

    // if there is no ready step, advance current time to earliest finishing
    // time later than current time
    else {
      NTYPE eft = POS_INF;  // earliest finishing time
      for (index_type k = 0; k < steps.length(); k++) {
	if (s[steps[k].act] == s_executing) {
	  NTYPE sft = steps[k].at + instance.actions[acts[steps[k].act]].dur;
	  if (sft >= current_t) {
	    if (sft < eft) {
	      eft = sft;
	      e.assign_value(steps[k].act, 1);
	    }
	    else if (sft == eft) {
	      e.append(steps[k].act);
	    }
	  }
	}
      }

      // if earliest finishing time is infinite, this can only be because
      // there is no executing step; if this is the case, and there is also
      // no ready step, the precedence relation must be cyclic: return
      // failure with an empty conflict set
      if (INFINITE(eft)) {
	std::cerr << "ERROR! CYCLIC PRECEDENCE GRAPH!" << std::endl;
	assert(!prec.acyclic());
	return false;
      }
      // if eft == current_t, this can only be because some currently
      // executing step has a duration equal to zero - print a warning
      if (eft == current_t) {
	std::cerr << "warning: empty interval at " << current_t
		  << " in Schedule::construct" << std::endl;
      }
      // for every step that finishes at eft...
      for (index_type k = 0; k < e.length(); k++) {
	// ...set its status to finished
	s[e[k]] = s_finished;
	// ...release resources used but not consumed by corresponding action
	for (index_type i = 0; i < instance.n_resources(); i++)
	  f[i] += instance.actions[acts[e[k]]].use[i];
      }
      // ...next, update status of remaining pending steps
      for (index_type k = 0; k < acts.length(); k++)
	if (s[k] == s_pending) {
	  bool now_ready = true;
	  for (index_type i = 0;
	       (i < prec.predecessors(k).length()) && now_ready; i++)
	    if (s[prec.predecessors(k)[i]] != s_finished)
	      now_ready = false;
	  if (now_ready)
	    s[k] = s_ready;
	}
      // ...and finally, advance current time
      assert((eft - current_t) > 0);
      advance(eft - current_t);
    }
  }

  // if we get here, the schedule has been successfully constructed: now...
  // ...replace step indices by proper action indices in schedule
  for (index_type k = 0; k < steps.length(); k++) {
    assert(steps[k].act < acts.length());
    steps[k].act = acts[steps[k].act];
  }

  // ...call end() to compute the proper schedule end time (makespan) and
  // other secondaries (action_vec, action_set) - it DEPENDS on the step
  // action indices having been set right!
  end();

  return true;
}

bool Schedule::construct_minimal_makespan
(const index_vec& acts,
 graph& prec,
 const index_set& c,
 NTYPE& best,
 index_vec& sindex)
{
  if (c.length() < 2) return false;
  bool ok = false;
  for (index_type i = 0; i < c.length(); i++)
    for (index_type j = 0; j < c.length(); j++) if (i != j) {
      assert(!prec.adjacent(c[j], c[i]));
      pair_set e;
      prec.add_edge_to_transitive_closure(c[i], c[j], e);
      Schedule* s1 = new Schedule(instance);
      index_set c1;
      index_vec x1;
      bool ok1 = s1->construct_conflict_free(acts, prec, c1, x1);
      if (ok1) {
	assert(c1.empty());
	if (s1->makespan() < best) {
	  clear();
	  s1->output(*this);
	  sindex.assign_copy(x1);
	  best = makespan();
	}
      }
      else {
	ok1 = construct_minimal_makespan(acts, prec, c1, best, sindex);
      }
      if (ok1) ok = true;
      prec.remove_edges(e);
      delete s1;
    }
  return ok;
}

bool Schedule::construct_minimal_makespan
(const index_vec& acts, const graph& prec, index_vec& sindex)
{
  index_set c;
  bool ok = construct_conflict_free(acts, prec, c, sindex);
  if (ok) {
    assert(c.empty());
    return true;
  }
  graph p(prec);
  p.transitive_closure();
  NTYPE b = POS_INF;
  return construct_minimal_makespan(acts, p, c, b, sindex);
}

bool Schedule::equivalent(const Schedule& s, index_vec* c) const
{
  graph p;
  deorder(p);
  assert(p.acyclic());
  p.transitive_closure();

  graph q;
  s.deorder(q);
  assert(q.acyclic());
  q.transitive_closure();

  if (trace_level > 1) {
    ::std::cerr << "comparing plans (" << step_actions() << "," << p
	      << ") and (" << s.step_actions() << "," << q << ")..."
	      << ::std::endl;
  }

  CorrespondanceEnumerator e(step_actions(), s.step_actions());
  bool more = e.first();
  if (!more) {
    if (trace_level > 1) {
      ::std::cerr << " - no correspondence" << ::std::endl;
    }
    return false;
  }
  while (more) {
    if (trace_level > 1) {
      ::std::cerr << " - correspondance: " << e.current() << ::std::endl;
    }
    if (p.equals(q, e.current())) {
      if (trace_level > 1) {
	::std::cerr << " - match!" << ::std::endl;
      }
      if (c) {
	c->assign_copy(e.current());
      }
      return true;
    }
    if (trace_level > 1) {
      ::std::cerr << " - precedence graphs differ:" << ::std::endl;
      pair_set d0;
      pair_set d1;
      p.difference(q, e.current(), d0, d1);
      ::std::cerr << "  in first but not second: " << d0 << ::std::endl;
      ::std::cerr << "  in second but not first: " << d1 << ::std::endl;
      p.write_graph_correspondance(::std::cerr, q, e.current(), "Precedence Graph Difference");
    }
    more = e.next();
  }
  if (trace_level > 1) {
    ::std::cerr << " - no match" << ::std::endl;
  }
  return false;
}

bool Schedule::random_sequence
(index_type ln_max, index_type ln_avg, bool continue_from_goal,
 ExecTrace* trace, RNG& rnd)
{
  if (trace_level > 0) {
    ::std::cerr << "constructing random plan (max l = " << ln_max
		<< ", E[l] = " << ln_avg
		<< (continue_from_goal ? ", continuing from goal states" :
		    ", stopping at first goal state")
		<< ")..." << ::std::endl;
  }

  clear();

  ExecState cs(instance, instance.init_atoms);
  cs.set_trace_level(trace_level - 1);
  if (trace_level > 1) {
    cs.write(::std::cerr);
  }
  if (trace) {
    trace->append((ExecState*)cs.copy());
  }

  bool_vec app(false, instance.n_actions());
  index_set s_app;

  NTYPE dt = 0;

  while (true) {
    // find applicable actions
    for (index_type k = 0; k < instance.n_actions(); k++)
      app[k] = cs.applicable(instance.actions[k], 0, no_such_index);
    app.copy_to(s_app);
    if (s_app.empty()) {
      if (trace_level > 0) {
	::std::cerr << "dead end reached (l = " << length()
		  << ", current state = " << cs << ")" << ::std::endl;
      }
      end();
      return false;
    }
    // select one at random
    index_type sel = rnd.random_in_range(s_app.length());

    // insert it into the plan
    advance(dt);
    insert(s_app[sel]);

    // apply it to current state
    cs.apply(instance.actions[s_app[sel]], 0, no_such_index);

    // advance to end of action and remember dt
    cs.advance(instance.actions[s_app[sel]].dur, 0);
    dt = instance.actions[s_app[sel]].dur;

    if (trace_level > 1) {
      cs.write(::std::cerr);
    }
    if (trace) {
      trace->append((ExecState*)cs.copy());
    }

    // check if we have reached a goal state (or should continue)
    if (cs.is_final() && !continue_from_goal) {
      if (trace_level > 0) {
	::std::cerr << "goal state reached (l = " << length()
		  << ", current state = " << cs << ")" << ::std::endl;
      }
      end();
      return true;
    }

    // check length limits
    if (ln_avg != no_such_index) {
      bool stop = (rnd.random_in_range(ln_avg) == 0);
      if (stop) {
	if (trace_level > 0) {
	  ::std::cerr << "stop (l = " << length() << ")" << ::std::endl;
	}
	end();
	return false;
      }
    }
    if (ln_max != no_such_index) {
      if (length() >= ln_max) {
	if (trace_level > 0) {
	  ::std::cerr << "maximum length reached" << ::std::endl;
	}
	end();
	return false;
      }
    }
  }
}

void compute_plan_dependency_graph
(Instance& instance,
 const index_vec& acts,
 const graph& prec,
 const index_set& goal,
 index_graph& dg,
 name_vec* oc_names)
{
  graph pp(prec);
  index_type s_goal = pp.size();
  pp.add_node();
  for (index_type k = 0; k < s_goal; k++)
    pp.add_edge(k, s_goal);
  pp.transitive_closure();

  pair_vec oc(index_pair(no_such_index, no_such_index), 0);
  for (index_type k = 0; k < goal.length(); k++)
    oc.append(index_pair(goal[k], s_goal));
  for (index_type k = 0; k < acts.length(); k++) {
    assert(acts[k] < instance.n_actions());
    for (index_type i = 0; i < instance.actions[acts[k]].pre.length(); i++)
      oc.append(index_pair(instance.actions[acts[k]].pre[i], k));
  }

  dg.init(oc.length());
  if (oc_names) {
    oc_names->assign_value(0, oc.length());
  }
  for (index_type k = 0; k < oc.length(); k++) {
//     std::cerr << "next open condition: " << oc[k]
// 	      << " = " << instance.atoms[oc[k].first].name
// 	      << " at ";
//     if (oc[k].second == s_goal) {
//       std::cerr << "goal";
//     }
//     else {
//       std::cerr << instance.actions[acts[oc[k].second]].name;
//     }
//     std::cerr << std::endl;
    dg.node_label(k) = oc[k].first;
    index_type s_k = oc[k].second;
    if (oc_names) {
      if (s_k == s_goal)
	(*oc_names)[k] =
	  new NameAtIndex(instance.atoms[oc[k].first].name, no_such_index);
      else
	(*oc_names)[k] =
	  new NameAtIndex(instance.atoms[oc[k].first].name, s_k);
    }
    for (index_type s_i = 0; s_i < (pp.size() - 1); s_i++) {
//       std::cerr << "s_i = " << s_i << std::endl;
      if (pp.adjacent(s_i, s_k) &&
	  instance.actions[acts[s_i]].add.contains(oc[k].first)) {
//  	std::cerr << "possible establisher: " << s_i << ": "
//  		  << instance.actions[acts[s_i]].name << std::endl;
	bool is_safe = true;
	for (index_type s_j = 0; (s_j < (pp.size() - 1)) && is_safe; s_j++)
	  if ((s_j != s_k) &&
	      !pp.adjacent(s_j, s_i) &&
	      !pp.adjacent(s_k, s_j) &&
	      instance.actions[acts[s_j]].del.contains(oc[k].first)) {
	    is_safe = false;
//  	    std::cerr << "establisher " << s_i << ": "
//  		      << instance.actions[acts[s_i]].name
//  		      << " threatened by " << s_j << ": "
//  		      << instance.actions[acts[s_j]].name
//  		      << std::endl;
	  }
	if (is_safe) {
// 	  std::cerr << "establisher " << s_i << ": "
// 		    << instance.actions[acts[s_i]].name << " is safe"
// 		    << std::endl;
	  for (index_type i = 0; i < oc.length(); i++)
	    if (oc[i].second == s_i) {
	      dg.add_edge(k, i);
	      dg.edge_label(k, i) = acts[s_i];
	    }
	}
      }
//       if (!pp.adjacent(s_i, s_k)) {
// 	std::cerr << "step " << s_i << ": "
// 		  << instance.actions[acts[s_i]].name
// 		  << " can not be an establisher for OC at " << s_k
// 		  << " (not ordered before)"
// 		  << std::endl;
//       }
//       if (!instance.actions[acts[s_i]].add.contains(oc[k].first)) {
// 	std::cerr << "step " << s_i << ": "
// 		  << instance.actions[acts[s_i]].name
// 		  << " can not be an establisher for OC " << oc[k].first
// 		  << " (does not add)"
// 		  << std::endl;
//       }
    }
  }
}


void compute_plan_exec_graph
(Instance& instance,
 Schedule& schedule,
 index_graph& g,
 name_vec& node_name)
{
  const Schedule::step_vec& sv = schedule.plan_steps();
  ExecTrace* trace = new ExecTrace(instance);
  bool ok = schedule.simulate(trace, 0, true);
  assert(ok);
  // ExecTrace* trace = trace0->necessary_trace();
  // delete trace0;
  bool_vec na(false, instance.n_atoms());
  // for (index_type k = 0; k < trace->length(); k++)
  //  na.insert((*trace)[k]->current_atoms());
  na.insert(instance.goal_atoms);
  for (index_type k = 0; k < sv.length(); k++) {
    na.insert(instance.actions[sv[k].act].pre);
    // na.insert(instance.actions[sv[k].act].add);
    // na.insert(instance.actions[sv[k].act].del);
  }
  index_vec atom_occ;
  index_type i_occ;
  index_set_vec est_by;
  index_set_vec del_by;
  index_set_vec supp;
  bool_vec is_init;
  bool_vec is_goal;
  for (index_type k = 0; k < instance.n_atoms(); k++) if (na[k]) {
    bool h = false;
    for (index_type i = 0; i < trace->length(); i++) {
      if ((*trace)[i]->current_atoms()[k]) {
	if (!h) {
	  atom_occ.append(k);
	  i_occ = atom_occ.length() - 1;
	  is_init.append(false);
	  is_goal.append(false);
	  est_by.append(EMPTYSET);
	  del_by.append(EMPTYSET);
	  supp.append(EMPTYSET);
	  if (i == 0) {
	    is_init[i_occ] = true;
	  }
	  else {
	    for (index_type j = 0; j < sv.length(); j++)
	      if (((sv[j].at + instance.actions[sv[j].act].dur)
		   == (*trace)[i]->current_time()) &&
		  (instance.actions[sv[j].act].add.contains(k)))
		est_by[i_occ].insert(j);
	  }
	  h = true;
	}
	for (index_type j = 0; j < sv.length(); j++)
	  if ((sv[j].at == (*trace)[i]->current_time()) &&
	      (instance.actions[sv[j].act].pre.contains(k)))
	    supp[i_occ].insert(j);
      }
      else if (!((*trace)[i]->current_atoms()[k]) && h) {
	for (index_type j = 0; j < sv.length(); j++)
	  if ((sv[j].at == (*trace)[i]->current_time()) &&
	      (instance.actions[sv[j].act].del.contains(k)))
	    del_by[i_occ].insert(j);
	h = false;
      }
    }
    if (instance.atoms[k].goal) {
      is_goal[i_occ] = true;
    }
  }
  delete trace;
  std::cerr << "compute_plan_exec_graph: size = "
	    << sv.length() + atom_occ.length() << std::endl;
  g.init(sv.length() + atom_occ.length());
  node_name.assign_value(0, sv.length() + atom_occ.length());
  for (index_type k = 0; k < sv.length(); k++) {
    node_name[k] = instance.actions[sv[k].act].name;
    g.node_label(k) = index_graph::NS_BOX;
  }
  for (index_type k = 0; k < atom_occ.length(); k++) {
    node_name[k + sv.length()] = instance.atoms[atom_occ[k]].name;
    g.node_label(k + sv.length()) =
      index_graph::NS_ELLIPSE +
      (is_init[k] ? index_graph::NS_FILLED : 0) +
      (is_goal[k] ? index_graph::NS_DOUBLE : 0);
    for (index_type i = 0; i < est_by[k].length(); i++) {
      g.add_edge(est_by[k][i], k + sv.length());
      g.edge_label(est_by[k][i], k + sv.length()) =
	index_graph::ES_NORMAL + index_graph::ED_FORWARD;
    }
    for (index_type i = 0; i < del_by[k].length(); i++) {
      g.add_edge(k + sv.length(), del_by[k][i]);
      if (supp[k].contains(del_by[k][i]))
	g.edge_label(k + sv.length(), del_by[k][i]) =
	  index_graph::ES_NORMAL + index_graph::ED_FORWARD;
      else
	g.edge_label(k + sv.length(), del_by[k][i]) =
	  index_graph::ES_BOLD + index_graph::ED_NONE;
    }
    for (index_type i = 0; i < supp[k].length(); i++)
      if (!del_by[k].contains(supp[k][i])) {
	g.add_edge(k + sv.length(), supp[k][i]);
	g.edge_label(k + sv.length(), supp[k][i]) =
	  index_graph::ES_DASHED + index_graph::ED_FORWARD;
      }
  }
  for (index_type k = 0; (k + 1) < atom_occ.length(); k++)
    if (atom_occ[k] == atom_occ[k + 1]) {
      g.add_edge(k + sv.length(), k + 1 + sv.length());
      g.edge_label(k + sv.length(), k + 1 + sv.length()) =
	index_graph::ES_DOTTED + index_graph::ED_FORWARD;
    }
  graph g1(g);
  g1.transitive_closure();
  graph prec;
  schedule.deorder(prec);
  prec.transitive_reduction();
  for (index_type i = 0; i < sv.length(); i++)
    for (index_type j = 0; j < sv.length(); j++)
      if (prec.adjacent(i, j) && !g1.adjacent(i, j)) {
	g.add_edge(i, j);
	g.edge_label(i, j) =
	  index_graph::ES_DOTTED + index_graph::ED_FORWARD;
      }
}

struct localvalue {
  index_type val;
  index_type est;
  index_set  sup;
  localvalue() : val(no_such_index), est(no_such_index), sup(EMPTYSET) { };
  localvalue(index_type v, index_type e) : val(v), est(e), sup(EMPTYSET) { };
  localvalue(const localvalue& v) : val(v.val), est(v.est), sup(v.sup) { };
  localvalue& operator=(const localvalue& v) {
    val = v.val;
    est = v.est;
    sup = v.sup;
  };
};

typedef lvector<localvalue> localview;

void compute_plan_transition_graph
(Instance& instance,
 Schedule& schedule,
 index_graph& g,
 name_vec& node_name)
{
  const Schedule::step_vec& sv = schedule.plan_steps();
  graph prec;
  schedule.deorder(prec);
  index_vec ss; // schedule steps, sorted
  bool ok = prec.top_sort(ss);
  assert(ok);

  index_set ra; // relevant atoms
  ra.insert(instance.goal_atoms);
  for (index_type k = 0; k < sv.length(); k++) {
    ra.insert(instance.actions[sv[k].act].pre);
    ra.insert(instance.actions[sv[k].act].add);
    ra.insert(instance.actions[sv[k].act].del);
  }

  lvector<localview> trans;
  for (index_type k = 0; k < instance.n_invariants(); k++)
    if (instance.invariants[k].set.have_common_element(ra) &&
	(instance.invariants[k].lim == 1)) {
      localview v;
      index_type v_cur =
	instance.invariants[k].set.first_common_element(instance.init_atoms);
      v.append(localvalue(v_cur, no_such_index));
      for (index_type i = 0; i < ss.length(); i++) {
	index_type v_add =
	  instance.actions[sv[ss[i]].act].add.first_common_element(instance.invariants[k].set);
	if ((v_add != no_such_index) && (v_add != v_cur)) {
	  assert((v_cur == no_such_index) ||
		 instance.actions[sv[ss[i]].act].del.contains(v_cur));
	  v.append(localvalue(v_add, ss[i]));
	  v_cur = v_add;
	}
	else if (instance.actions[sv[ss[i]].act].del.contains(v_cur)) {
	  v.append(localvalue(no_such_index, ss[i]));
	  v_cur = no_such_index;
	}
	else if (instance.actions[sv[ss[i]].act].pre.contains(v_cur)) {
	  v[v.length() - 1].sup.insert(ss[i]);
	}
      }
      trans.append(v);
    }

  index_vec off(0, trans.length());
  for (index_type k = 1; k < trans.length(); k++)
    off[k] = off[k - 1] + trans[k - 1].length();
  index_type n_val = 0;
  for (index_type k = 0; k < trans.length(); k++)
    n_val += trans[k].length();

  g.init(n_val);
  node_name.assign_value(0, n_val);

  for (index_type k = 0; k < trans.length(); k++) {
    for (index_type i = 0; i < trans[k].length(); i++) {
      g.node_label(off[k] + i) = index_graph::NS_ELLIPSE;
      if (trans[k][i].val != no_such_index)
	node_name[off[k] + i] = instance.atoms[trans[k][i].val].name;
    }
    g.node_label(off[k] + 0) += index_graph::NS_FILLED;
    index_type v_end = trans[k][trans[k].length() - 1].val;
    if (instance.goal_atoms.contains(v_end))
      g.node_label(off[k] + trans[k].length() - 1) += index_graph::NS_DOUBLE;
    for (index_type i = 0; (i + 1) < trans[k].length(); i++) {
      g.add_edge(off[k] + i, off[k] + i + 1);
      g.edge_label(off[k] + i, off[k] + i + 1) =
	index_graph::ES_BOLD + index_graph::ED_FORWARD;
    }
  }

  for (index_type k = 0; k < sv.length(); k++)
    for (index_type i0 = 0; i0 < trans.length(); i0++)
      for (index_type j0 = 0; j0 < trans[i0].length(); j0++)
	if (trans[i0][j0].est == k)
	  for (index_type i1 = 0; i1 < trans.length(); i1++)
	    if (i1 != i0)
	      for (index_type j1 = 0; j1 < trans[i1].length(); j1++) {
		if (trans[i1][j1].est == k) {
		  g.add_edge(off[i0] + j0 - 1, off[i1] + j1);
		  g.edge_label(off[i0] + j0 - 1, off[i1] + j1) =
		    index_graph::ES_NORMAL + index_graph::ED_FORWARD;
		  g.add_edge(off[i1] + j1 - 1, off[i0] + j0);
		  g.edge_label(off[i1] + j1 - 1, off[i0] + j0) =
		    index_graph::ES_NORMAL + index_graph::ED_FORWARD;
		}
		if (trans[i1][j1].sup.contains(k)) {
		  g.add_edge(off[i1] + j1, off[i0] + j0);
		  g.edge_label(off[i1] + j1, off[i0] + j0) =
		    index_graph::ES_DASHED + index_graph::ED_FORWARD;
		}
	      }
}


ScheduleSet::ScheduleSet(Instance& i)
  : plan_vec(0, 0), instance(i), props(0, 0),
    trace_level(Instance::default_trace_level)
{
  // done
}

ScheduleSet::ScheduleSet(ScheduleSet& s, const bool_vec& sel)
  : plan_vec(0, 0), instance(s.instance), props(0, 0),
    trace_level(s.trace_level)
{
  s.output(*this, sel);
}

ScheduleSet::~ScheduleSet()
{
  clear();
}

bool ScheduleSet::ScheduleProperties::dominates(const ScheduleProperties& p)
{
  if (!valid && p.valid) return false;
  if (p.makespan < makespan) return false;
  assert(n_resources == p.n_resources);
  for (index_type k = 0; k < n_resources; k++) {
    if (p.total_consumption[k] < total_consumption[k]) return false;
    if (p.peak_use[k] < peak_use[k]) return false;
    if (p.tolerable_loss[k] > tolerable_loss[k]) return false;
  }
  return true;
}

ScheduleSet::ScheduleProperties* ScheduleSet::compute_properties(Schedule* s)
{
  ScheduleProperties* p = new ScheduleProperties(instance.n_resources());

  p->trace = new ExecTrace(instance);
  p->valid = s->simulate(p->trace, 0, true);
  p->makespan = s->makespan();

  for (index_type k = 0; k < instance.n_resources(); k++) {
    ResourceProfile* rp = new ResourceProfile(instance, k, *(p->trace));
    p->peak_use[k] = rp->peak_use();
    p->total_consumption[k] = rp->total_consumption();
    p->tolerable_loss[k] = rp->tolerable_unexpected_loss();
    delete rp;
  }

  return p;
}

void ScheduleSet::cache_properties(index_type i, ScheduleProperties* p)
{
  assert(i < length());
  if (i >= props.length()) props.set_length(i + 1);
  if (p) {
    if (props[i]) delete props[i];
    props[i] = p;
  }
  else if (props[i] == 0) {
    props[i] = compute_properties((*this)[i]);
  }
}

bool ScheduleSet::dominated(const ScheduleProperties& p, prop_vec& pv)
{
  for (index_type k = 0; k < pv.length(); k++) {
    if (pv[k]->dominates(p)) return true;
  }
  return false;
}

void ScheduleSet::dominated(prop_vec& pv, bool_vec& dom)
{
  dom.assign_value(false, pv.length());
  for (index_type k = 0; k < pv.length(); k++)
    for (index_type i = 0; (i < pv.length()) && !dom[k]; i++) if (k != i) {
      if (pv[i]->dominates(*(pv[k])) && !dom[i]) dom[k] = true;
    }
}

bool ScheduleSet::dominated(const ScheduleProperties& p)
{
  for (index_type k = 0; k < length(); k++) {
    cache_properties(k, 0);
    if (props[k]->dominates(p)) return true;
  }
  return false;
}

void ScheduleSet::replace_schedule_with_properties
(index_type i, Schedule* s, ScheduleProperties* p)
{
  assert(i < length());
  if ((*this)[i]) delete (*this)[i];
  (*this)[i] = s;
  cache_properties(i, p);
}

void ScheduleSet::add_schedule_with_properties
(Schedule* s, ScheduleProperties* p)
{
  index_type i = length();
  append(s);
  cache_properties(i, p);
}

void ScheduleSet::add_schedule(Schedule* s)
{
  append(s);
}

void ScheduleSet::add_schedule_if_different(Schedule* s)
{
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k]->equivalent(*s)) {
      delete s;
      return;
    }
  append(s);
}

void ScheduleSet::remove(bool_vec& set)
{
  for (index_type k = 0; k < length(); k++) if (set[k]) {
    if ((*this)[k]) delete (*this)[k];
    if (k < props.length())
      if (props[k]) delete props[k];
  }
  plan_vec::remove(set);
  props.remove(set);
}

void ScheduleSet::clear()
{
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k]) delete (*this)[k];
  set_length(0);
  for (index_type k = 0; k < props.length(); k++)
    if (props[k]) delete props[k];
  props.set_length(0);
}

void ScheduleSet::set_trace_level(int level)
{
  trace_level = level;
}

void ScheduleSet::reduce_plans()
{
  bool_vec ok(true, length());
  for (index_type k = 0; k < length(); k++) {
    ExecErrorSet* errors = new ExecErrorSet();
    errors->ignore_error_type(ExecError::error_resource_conflict);
    errors->ignore_error_type(ExecError::error_resource_shortage);
    (*this)[k]->simulate(0, errors, true);
    if (errors->valid()) {
      if (trace_level > 0) {
	ExecErrorSet* warnings =
	  errors->all_of_severity(ExecError::severity_warning);
	if (warnings->length() > 0) {
	  ::std::cerr << "plan #" << k;
	  if ((*this)[k]->plan_name())
	    ::std::cerr << " (" << (*this)[k]->plan_name() << ")";
	  ::std::cerr << ": " << warnings->length() << " warnings:";
	  warnings->write(::std::cerr);
	  ::std::cerr << ::std::endl;
	}
	delete warnings;
      }
      (*this)[k]->reduce(errors);
    }
    else {
      ok[k] = false;
      if (trace_level > 0) {
	::std::cerr << "plan #" << k;
	if ((*this)[k]->plan_name())
	  ::std::cerr << " (" << (*this)[k]->plan_name() << ")";
	::std::cerr << " not logically valid: "
		  << errors->length() << " errors:";
	errors->write(::std::cerr);
	::std::cerr << ::std::endl;
      }
    }
    delete errors;
  }
  if (trace_level > 0) {
    ::std::cerr << "removing " << ok.count(false) << " logically invalid plans"
	      << ::std::endl;
  }
  ok.complement();
  remove(ok);
}

void ScheduleSet::filter_invalid_plans()
{
  bool_vec ok(true, length());
  for (index_type k = 0; k < length(); k++) {
    ok[k] = (*this)[k]->simulate_low_resolution();
    if (!ok[k] && (trace_level > 0)) {
      ::std::cerr << "plan #" << k;
      if ((*this)[k]->plan_name())
	::std::cerr << (*this)[k]->plan_name();
      ::std::cerr << " not valid" << ::std::endl;
    }
  }
  if (trace_level > 0) {
    ::std::cerr << "removing " << ok.count(false) << " invalid plans"
	      << ::std::endl;
  }
  ok.complement();
  remove(ok);
}

void ScheduleSet::filter_unschedulable_plans()
{
  bool_vec ok(true, length());
  for (index_type k = 0; k < length(); k++) {
    if (trace_level > 1) {
      ::std::cerr << "trying to reschedule plan #" << k;
      if ((*this)[k]->plan_name())
	::std::cerr << " (" << (*this)[k]->plan_name() << ")";
      ::std::cerr << "..." << ::std::endl;
    }
    graph p;
    (*this)[k]->deorder(p);
    ok[k] = feasible(instance, (*this)[k]->step_actions(), p, 0);
    if (trace_level > 0) {
      if (!ok[k]) {
	::std::cerr << "plan #" << k;
	if ((*this)[k]->plan_name())
	  ::std::cerr << " (" << (*this)[k]->plan_name() << ")";
	::std::cerr << " can not be scheduled" << ::std::endl;
      }
    }
  }
  if (trace_level > 0) {
    ::std::cerr << "removing " << ok.count(false) << " unschedulable plans"
	      << ::std::endl;
  }
  ok.complement();
  remove(ok);
}

void ScheduleSet::filter_equivalent_plans()
{
  bool_vec n(true, length());
  for (index_type k = 0; k < length(); k++) {
    for (index_type i = 0; (i < k) && n[k]; i++) if (n[i]) {
      index_vec c;
      if ((*this)[k]->equivalent(*(*this)[i], &c)) {
	n[k] = false;
	if (trace_level > 0) {
	  ::std::cerr << "plan #" << k;
	  if ((*this)[k]->plan_name())
	    ::std::cerr << " (" << (*this)[k]->plan_name() << ")";
	  ::std::cerr << " is equivalent to plan #" << i;
	  if ((*this)[i]->plan_name())
	    ::std::cerr << " (" << (*this)[i]->plan_name() << ")";
	  ::std::cerr << ::std::endl;
	}
	delete (*this)[k];
	(*this)[k] = 0;
      }
    }
    if ((n[k]) && (trace_level > 0)) {
      ::std::cerr << "plan #" << k;
      if ((*this)[k]->plan_name())
	::std::cerr << " (" << (*this)[k]->plan_name() << ")";
      ::std::cerr << " is new" << ::std::endl;
    }
  }
  n.complement();
  remove(n);
}

void ScheduleSet::find_tweaks
(Schedule* src_plan, ScheduleProperties* src_plan_props,
 const index_vec& acts, const graph& prec, index_type res, ResourceProfile* rp,
 pair_vec& tweaks, plan_vec& new_scheds, prop_vec& new_props)
{
  // tweaks.clear();
  // new_scheds.clear();
  // new_props.clear();

  if (trace_level > 0) {
    ::std::cerr << "searching for tweaks for resource " << rp->resource_name();
    if (src_plan->plan_name()) {
      ::std::cerr << " in plan " << src_plan->plan_name();
    }
    ::std::cerr << "..." << ::std::endl;
  }

  index_type i0 = rp->first_use_interval(0);
  if ((i0 != no_such_index) && (i0 < rp->n_intervals())) {
    if (trace_level > 1) {
      ::std::cerr << " - first use interval is " << i0 << ::std::endl;
    }
    index_type i1 = rp->first_min_free_interval(i0 + 1);
    if (i1 != no_such_index) {
      if (trace_level > 1) {
	::std::cerr << " - constraining interval is " << i1 << ::std::endl;
      }
      index_set s1;
      for (index_type k = 0; k < src_plan->n_steps(); k++) {
	NTYPE i_start = rp->interval_start_time(i1);
	NTYPE i_end = rp->interval_end_time(i1);
	if (src_plan->step_in_interval(k, i_start, i_end) &&
	    (instance.actions[src_plan->step_action(k)].req(res) > 0))
	  s1.insert(k);
      }
      assert(s1.length() > 0);

      if (trace_level > 1) {
	::std::cerr << " - relevant steps in interval " << i1 << ": ";
	src_plan->write_step_set(::std::cerr, s1);
	::std::cerr << ::std::endl;
      }

      // if |s1| > 1, there is more than one action at first peak use
      // step after first use step, so tolerable loss may be reduced by
      // levelling use in this step
      if (s1.length() > 1) {
	for (index_type k1 = 0; k1 < s1.length(); k1++)
	  for (index_type k2 = 0; k2 < s1.length(); k2++)
	    if ((k1 != k2) && !prec.adjacent(s1[k2], s1[k1])) {
	      if (trace_level > 1) {
		::std::cerr << " - trying type-1 tweak " << s1[k1]
			    << " < " << s1[k2] << "..." << ::std::endl;
	      }
	      graph tprec(prec);
	      tprec.add_edge(s1[k1], s1[k2]);
	      tprec.transitive_closure();
	      Schedule* s = new Schedule(instance);
	      index_vec m;
	      bool ok = s->construct_minimal_makespan(acts, tprec, m);
	      if (!ok) {
		::std::cerr << "error: failed to schedule acts " << acts
			  << " with precedence graph " << tprec
			  << ::std::endl;
		exit(255);
	      }
	      s->add_trait(new PlanPrecedenceRelation(s, tprec, m));
	      bool is_derived = false;
	      for (index_type t = 0; t < src_plan->traits.length(); t++) {
		DerivedFrom* dftrait =
		  (DerivedFrom*)src_plan->traits[t]->cast_to("DerivedFrom");
		if (dftrait) {
		  s->add_trait(new DerivedFrom(s, this, dftrait, m, index_pair(m[s1[k1]], m[s1[k2]])));
		  is_derived = true;
		}
	      }
	      if (!is_derived) {
		s->add_trait(new DerivedFrom(s, this, src_plan, index_pair(m[s1[k1]], m[s1[k2]])));
	      }
	      s->set_name(new PlanName(src_plan->plan_name(), "tweak I",
				       tweaks.length()));
	      ScheduleProperties* p = compute_properties(s);
	      bool dom = false;
	      if (src_plan_props) {
		if (src_plan_props->dominates(*p)) dom = true;
	      }
	      if (!dom) {
		if (trace_level > 1) {
		  ::std::cerr << " - new non-dominated schedule:"
			      << ::std::endl;
		  s->write(::std::cerr);
		}
		tweaks.append(index_pair(s1[k1], s1[k2]));
		new_scheds.append(s);
		new_props.append(p);
	      }
	      else {
		if (trace_level > 1) {
		  ::std::cerr << " - new schedule dominated by source plan"
			      << ::std::endl;
		}
		delete s;
		delete p;
	      }
	    }
	if (trace_level > 0) {
	  ::std::cerr << " - " << tweaks.length()
		      << " successful type-1 tweaks"
		      << ::std::endl;
	}
      }

      // else, maybe we can do something else...
      index_set s0;
      for (index_type k = 0; k < src_plan->n_steps(); k++) {
	NTYPE i_start = rp->interval_start_time(i0);
	NTYPE i_end = rp->interval_end_time(i0);
	if (src_plan->step_in_interval(k, i_start, i_end) &&
	    (instance.actions[src_plan->step_action(k)].req(res) > 0))
	  s0.insert(k);
      }
      assert(s0.length() > 0);

      if (trace_level > 1) {
	::std::cerr << " - relevant steps in interval " << i0 << ": ";
	src_plan->write_step_set(::std::cerr, s0);
	::std::cerr << ::std::endl;
      }

      for (index_type k0 = 0; k0 < s0.length(); k0++)
	for (index_type k1 = 0; k1 < s1.length(); k1++)
	  if ((s0[k0] != s1[k1]) && !prec.adjacent(s0[k0], s1[k1])) {
	    if (trace_level > 1) {
	      ::std::cerr << " - trying type-2 tweak " << s1[k1]
			  << " < " << s0[k0] << "..." << ::std::endl;
	    }
	    graph tprec(prec);
	    tprec.add_edge(s1[k1], s0[k0]);
	    tprec.transitive_closure();
	    Schedule* s = new Schedule(instance);
	    index_vec m;
	    bool ok = s->construct_minimal_makespan(acts, tprec, m);
	    if (!ok) {
	      ::std::cerr << "error: failed to schedule acts: " << acts
			<< " with precedence graph: " << tprec
			<< ::std::endl;
	      exit(255);
	    }
	    s->add_trait(new PlanPrecedenceRelation(s, tprec, m));
	    bool is_derived = false;
	    for (index_type t = 0; t < src_plan->traits.length(); t++) {
	      DerivedFrom* dftrait =
		(DerivedFrom*)src_plan->traits[t]->cast_to("DerivedFrom");
	      if (dftrait) {
		s->add_trait(new DerivedFrom(s, this, dftrait, m, index_pair(m[s1[k1]], m[s0[k0]])));
		is_derived = true;
	      }
	    }
	    if (!is_derived) {
	      s->add_trait(new DerivedFrom(s, this, src_plan, index_pair(m[s1[k1]], m[s0[k0]])));
	    }
	    s->set_name(new PlanName(src_plan->plan_name(), "tweak II",
				     tweaks.length()));
	    ScheduleProperties* p = compute_properties(s);
	    bool dom = false;
	    if (src_plan_props) {
	      if (src_plan_props->dominates(*p)) dom = true;
	    }
	    if (!dom) {
	      if (trace_level > 1) {
		::std::cerr << " - new non-dominated schedule:" << ::std::endl;
		s->write(::std::cerr);
	      }
	      tweaks.append(index_pair(s1[k1], s0[k0]));
	      new_scheds.append(s);
	      new_props.append(p);
	    }
	    else {
	      if (trace_level > 1) {
		::std::cerr << " - no local improvement" << ::std::endl;
	      }
	      delete s;
	      delete p;
	    }
	  }
    }
  }

  if (trace_level > 0) {
    ::std::cerr << " - " << tweaks.length()
		<< " successful tweaks in total"
		<< ::std::endl;
  }
}

void ScheduleSet::explore_options
(Schedule* s, ScheduleProperties* p, index_type res,
 plan_vec& new_scheds, prop_vec& new_props)
{
  if (trace_level > 1) {
    ::std::cerr << "exploring options for schedule:" << ::std::endl;
    s->write(::std::cerr);
    ::std::cerr << " - next resource is " << res << ". "
		<< instance.resources[res].name << ::std::endl;
  }

  if (!p) {
    p = compute_properties(s);
  }

  new_scheds.assign_value(0, 0);
  new_props.assign_value(0, 0);

  graph prec;
  PlanPrecedenceRelation* pp =
    (PlanPrecedenceRelation*)s->find_trait("PlanPrecedenceRelation");
  if (pp) {
    prec = pp->precedence_relation();
    if (trace_level > 1) {
      ::std::cerr << " - using attached precedence graph: " << prec
		  << ::std::endl;
    }
  }
  else {
    s->deorder(prec);
    if (trace_level > 1) {
      ::std::cerr << " - deordering to obtain precedence graph: " << prec
		  << ::std::endl;
    }
  }
  prec.transitive_closure();

  while (res < instance.n_resources()) {
    ResourceProfile* rp = new ResourceProfile(instance, res, *(p->trace));
    if (FINITE(rp->tolerable_unexpected_loss()) ||
	(rp->peak_use() > rp->min_peak_use())) {
      if ((trace_level > 0) && FINITE(rp->tolerable_unexpected_loss())) {
	::std::cerr << " - tolerable loss of resource "
		    << instance.resources[res].name
		    << " is only " << rp->tolerable_unexpected_loss()
		    << ", let's see if we can fix that..."
		    << ::std::endl;
      }
      if ((trace_level > 0) && (rp->peak_use() > rp->min_peak_use())) {
	::std::cerr << " - peak use of resource "
		    << instance.resources[res].name
		    << " is " << rp->peak_use()
		    << " greater than min " << rp->min_peak_use()
		    << ", let's see if we can fix that..."
		    << ::std::endl;
      }
      pair_vec tweaks(no_such_index, 0);
      plan_vec new_scheds_1(0, 0);
      prop_vec new_props_1(0, 0);

      find_tweaks(s, p, s->step_actions(), prec, res, rp,
		  tweaks, new_scheds_1, new_props_1);
      assert(new_scheds_1.length() == new_props_1.length());

      if (new_scheds_1.length() > 0) {
	if (trace_level > 0) {
	  ::std::cerr << " - " << new_scheds_1.length()
		      << " locally improving tweaks found"
		      << ::std::endl;
	}
	for (index_type k = 0; k < new_scheds_1.length(); k++) {
	  if (trace_level > 1) {
	    ::std::cerr << " - continuing exploration from "
			<< new_scheds_1[k]->plan_name()
			<< "..." << ::std::endl;
	  }
	  plan_vec new_scheds_k(0, 0);
	  prop_vec new_props_k(0, 0);
	  explore_options(new_scheds_1[k], new_props_1[k], res,
			  new_scheds_k, new_props_k);
	  assert(new_scheds_k.length() == new_props_k.length());
	  if (trace_level > 1) {
	    ::std::cerr << " - " << new_scheds_k.length()
			<< " improving schedules found"
			<< ::std::endl;
	  }
	  if (new_scheds_k.length() > 0) {
	    new_scheds.append(new_scheds_k);
	    new_props.append(new_props_k);
	  }
	  else {
	    new_scheds.append(new_scheds_1[k]);
	    new_props.append(new_props_1[k]);
	  }
	}
	assert(new_scheds.length() == new_props.length());

	if (trace_level > 0) {
	  ::std::cerr << " - continuing exploration from source schedule..."
		      << ::std::endl;
	}
	plan_vec new_scheds_0(0, 0);
	prop_vec new_props_0(0, 0);
	explore_options(s, p, res + 1, new_scheds_0, new_props_0);
	assert(new_scheds_0.length() == new_props_0.length());
	if (trace_level > 0) {
	  ::std::cerr << " - " << new_scheds_0.length()
		      << " improving schedules found"
		      << ::std::endl;
	}
	if (new_scheds_0.length() > 0) {
	  new_scheds.append(new_scheds_0);
	  new_props.append(new_props_0);
	}

	// at this point, we do dominance pruning...
	bool_vec dom;
	dominated(new_props, dom);
	if (trace_level > 0) {
	  ::std::cerr << " - " << dom.count(false)
		      << " of " << new_scheds.length()
		      << " new schedules are non-dominated"
		      << ::std::endl;
	}
	for (index_type k = 0; k < new_scheds.length(); k++)
	  if (dom[k]) {
	    delete new_scheds[k];
	    delete new_props[k];
	  }
	new_scheds.remove(dom);
	new_props.remove(dom);

	return; // break the loop
      }
      else {
	if (trace_level > 0) {
	  if (res + 1 < instance.n_resources()) {
	    ::std::cerr << " - no improving tweaks found, next resource is "
			<< instance.resources[res + 1].name << "..."
			<< ::std::endl;
	  }
	  else {
	    ::std::cerr << " - no improving tweaks found, no more resources"
			<< ::std::endl;
	  }
	}
	res += 1;
      }
    }
    else {
      if (trace_level > 0) {
	::std::cerr << " - resource "
		    << instance.resources[res].name
		    << " ok..."
		    << ::std::endl;
      }
      res += 1;
    }
  }
}

void ScheduleSet::explore_options
(bool cross_dominance_check, bool src_dominance_check)
{
  plan_vec new_scheds(0, 0);
  prop_vec new_props(0, 0);

  for (index_type k = 0; k < length(); k++) {
    assert((*this)[k]);
    cache_properties(k, 0);

    ::std::cerr << "exploring options for schedule " << k << "..." << ::std::endl;
    plan_vec new_scheds_k(0, 0);
    prop_vec new_props_k(0, 0);
    explore_options((*this)[k], props[k], 0, new_scheds_k, new_props_k);
    assert(new_scheds_k.length() == new_props_k.length());
    ::std::cerr << " - " << new_scheds_k.length()
	      << " improving schedules found (from schedule " << k << ")"
	      << ::std::endl;
    new_scheds.append(new_scheds_k);
    new_props.append(new_props_k);
  }

  if (cross_dominance_check) {
    if (src_dominance_check) {
      for (index_type k = 0; k < length(); k++) {
	new_scheds.append((*this)[k]);
	new_props.append(props[k]);
      }
      set_length(0);
      props.set_length(0);
    }

    bool_vec dom;
    dominated(new_props, dom);
    ::std::cerr << " - " << dom.count(false) << " of "
	      << new_scheds.length() << " schedules are non-dominated"
	      << ::std::endl;
    for (index_type k = 0; k < new_scheds.length(); k++)
      if (dom[k]) {
	delete new_scheds[k];
	delete new_props[k];
      }
    new_scheds.remove(dom);
    new_props.remove(dom);
  }

  for (index_type k = 0; k < new_scheds.length(); k++) {
    add_schedule_with_properties(new_scheds[k], new_props[k]);
  }
}

void ScheduleSet::sequential_variations
(const Name* src_name, index_type& n,
 const index_vec acts, const graph& p, const graph& p0, const pair_set& v,
 index_type d, index_type d_min, index_type d_max)
{
  if (!v.empty()) {
    if (trace_level > 1) {
      std::cerr << "chosing REVERSE edge "
		<< v[0].first << " <- " << v[0].second
		<< std::endl;
    }
    graph p1(p);
    p1.add_edge(v[0].second, v[0].first);
    p1.transitive_closure();
    assert(p1.acyclic());
    pair_set v1;
    index_type d1 = d;
    for (index_type k = 0; k < v.length(); k++) {
      if (p1.adjacent(v[k].second, v[k].first)) d1 += 1;
      else if (!p1.adjacent(v[k].first, v[k].second)) v1.insert(v[k]);
    }
    if (d1 <= d_max) {
      sequential_variations(src_name, n, acts, p1, p0, v1, d1, d_min, d_max);
    }

    if (trace_level > 1) {
      std::cerr << "chosing forward edge "
		<< v[0].first << " -> " << v[0].second
		<< std::endl;
    }
    graph p2(p);
    p2.add_edge(v[0].first, v[0].second);
    p2.transitive_closure();
    assert(p2.acyclic());
    pair_set v2;
    index_type d2 = d;
    for (index_type k = 0; k < v.length(); k++) {
      if (p2.adjacent(v[k].second, v[k].first)) d2 += 1;
      else if (!p2.adjacent(v[k].first, v[k].second)) v2.insert(v[k]);
    }
    if (d2 <= d_max) {
      sequential_variations(src_name, n, acts, p2, p0, v2, d2, d_min, d_max);
    }
  }
  else if (d >= d_min) {
    if (trace_level > 0) {
      std::cerr << "found schedule variation at d = " << d << std::endl;
    }
    Schedule* s = new Schedule(instance);
    s->set_name(new PlanName(src_name, "resequencing", n++));
    bool ok = s->schedule(acts, p);
    if (!ok) {
      std::cerr << "error: failed to re-schedule " << acts
		<< " with precedence graph " << p << std::endl;
      exit(255);
    }
    add_schedule(s);
  }
}

void ScheduleSet::sequential_variations
(Schedule* s, index_type d_min, index_type d_max)
{
  std::cerr << "generating sequential variations of plan";
  if (s->plan_name()) std::cerr << " " << s->plan_name();
  std::cerr << "..." << std::endl;

  graph p0;
  s->base_precedence_graph(p0);
  p0.transitive_closure();
  graph p;
  s->deorder(p);
  p.transitive_closure();

  if (trace_level > 1) {
    std::cerr << "base precedence graph: " << p0 << std::endl;
    std::cerr << "deordered precedence graph: " << p << std::endl;

    name_vec nv;
    s->step_action_names(nv);
    graph rp0(p0);
    rp0.transitive_reduction();
    write_labeled_digraph<name_vec>
      (std::cerr, rp0, nv, false, "base precedence graph");
    graph rp(p);
    rp.transitive_reduction();
    write_labeled_digraph<name_vec>
      (std::cerr, rp, nv, false, "deordered precedence graph");
  }

  pair_set d0;
  pair_set d1;
  p0.difference(p, d0, d1);
  if (!d1.empty()) {
    std::cerr << "error: deordering did not produce a subgraph!"
	      << std::endl;
    s->write(std::cerr);
    std::cerr << "base precedence graph: " << p0 << std::endl;
    std::cerr << "deordered precedence graph: " << p << std::endl;
    exit(255);
  }

  if (trace_level > 0) {
    std::cerr << "non-essential orderings: " << d0 << std::endl;
  }

  if (d_min == no_such_index) d_min = 1;
  if (d_max == no_such_index) d_max = d0.length();

  index_type n = 0;
  sequential_variations(s->plan_name(), n, s->step_actions(), p, p0, d0, 0,
			d_min, d_max);
}

index_type ScheduleSet::random_sequential_variations
(Schedule* s,
 index_type d_min,
 index_type d_max,
 index_type n,
 RNG& rng)
{
  std::cerr << "generating sequential variations of plan";
  if (s->plan_name()) std::cerr << " " << s->plan_name();
  std::cerr << "..." << std::endl;

  graph p0;
  // std::cerr << "extracting base precedence graph..." << std::endl;
  s->base_precedence_graph(p0);
  // std::cerr << "computing transitive closure..." << std::endl;
  p0.transitive_closure();
  graph p;
  // std::cerr << "deordering..." << std::endl;
  s->deorder(p);
  // std::cerr << "computing transitive closure..." << std::endl;
  p.transitive_closure();

  // std::cerr << "computing edge-set difference..." << std::endl;

  pair_set d0;
  pair_set d1;
  p0.difference(p, d0, d1);
  if (!d1.empty()) {
    std::cerr << "error: deordering did not produce a subgraph!"
	      << std::endl;
    s->write(std::cerr);
    std::cerr << "base precedence graph: " << p0 << std::endl;
    std::cerr << "deordered precedence graph: " << p << std::endl;
    exit(255);
  }

  if (trace_level > 0) {
    std::cerr << "non-essential orderings: " << d0 << std::endl;
  }

  if ((d_min < 1) || (d_min == no_such_index))
    d_min = 1;
  if ((d_max > d0.length()) || (d_max == no_such_index))
    d_max = d0.length();

  if ((d0.length() < d_min) || (d_min == 0))
    return 0;

  index_type g = 0;
  index_type f = 0;
  while ((g < n) && (f < 2*n)) {
    // std::cerr << "g = " << g << ", f = " << f << std::endl;
    graph p1(p);
    bool_vec rem(true, d0.length());
    index_type d_target = rng.random_in_range((d_max - d_min)) + d_min;
    index_type d = 0;
    bool done = false;
    while (!done) {
      index_type i = rng.select_one_of(rem);
      assert(i < d0.length());
      assert(!p1.adjacent(d0[i].first, d0[i].second) &&
	     !p1.adjacent(d0[i].second, d0[i].first));
      pair_set dummy;
      if (rng.random_in_range(2) == 1)
	p1.add_edge_to_transitive_closure(d0[i].first, d0[i].second, dummy);
      else
	p1.add_edge_to_transitive_closure(d0[i].second, d0[i].first, dummy);
      for (index_type j = 0; j < d0.length(); j++)
	if (rem[j]) {
	  if (p1.adjacent(d0[j].second, d0[j].first)) {
	    d += 1;
	    rem[j] = false;
	  }
	  else if (p1.adjacent(d0[j].first, d0[j].second)) {
	    rem[j] = false;
	  }
	}
      if (d >= d_target) done = true;
      else if (rem.count(true) == 0) done = true;
    }
    if ((d >= d_min) && (d <= d_max)) {
      Schedule* rs = new Schedule(instance);
      rs->set_name(new PlanName(s->plan_name(), "resequencing", g++));
      bool ok = rs->schedule(s->step_actions(), p1);
      assert(ok);
      add_schedule(rs);
    }
    else {
      f += 1;
    }
  }

  // std::cerr << "done" << std::endl;
}

void ScheduleSet::add_distinguishing_traits_1()
{
  if (empty()) return; // so we can assume there is at least one plan

  index_set all_acts;
  for (index_type k = 0; k < length(); k++)
    all_acts.insert((*this)[k]->plan_actions());

  for (index_type k = 0; k < all_acts.length(); k++) {
    index_vec n_occ(0, length());
    index_set d_occ;
    for (index_type i = 0; i < length(); i++) {
      n_occ[i] = (*this)[i]->step_actions().count(all_acts[k]);
      d_occ.insert(n_occ[i]);
    }
    if (d_occ.length() > 1) {
      for (index_type i = 0; i < length(); i++) {
	PlanActionOccurs* ao_trait =
	  new PlanActionOccurs((*this)[i], instance, all_acts[k], n_occ[i]);
	if (n_occ[i] == d_occ[0]) ao_trait->set_min();
	if (n_occ[i] == d_occ[d_occ.length() - 1]) ao_trait->set_max();
	if (n_occ.count(n_occ[i]) == 1) ao_trait->set_unique();
	(*this)[i]->add_trait(ao_trait);
      }
    }
  }

  for (index_type i = 0; i < length(); i++)
    for (index_type j = i + 1; j < length(); j++) {
      CorrespondanceEnumerator e((*this)[i]->step_actions(),
				 (*this)[j]->step_actions());
      bool more = e.first();
      if (more) {
	graph pi;
	(*this)[i]->base_precedence_graph(pi);
	pi.transitive_closure();
	graph pj;
	(*this)[j]->base_precedence_graph(pj);
	pj.transitive_closure();

	pair_set ei;
	pair_set ej;
	pi.difference(pj, e.current(), ei, ej);

	for (index_type k = 0; k < ej.length(); k++) {
	  if (!pj.adjacent(ej[k].first, ej[k].second)) {
	    std::cerr << "error: identified ordering " << ej[k].first
		      << " < " << ej[k].second << " does not hold!"
		      << std::endl;
	    std::cerr << "pi = " << pi << std::endl;
	    std::cerr << "pj = " << pj << std::endl;
	    exit(255);
	  }
	}

	more = e.next();
	while (more) {
	  pair_set d0;
	  pair_set d1;
	  pi.difference(pj, e.current(), d0, d1);
	  ei.intersect(d0);
	  ej.intersect(d1);
	  more = e.next();
	}

	for (index_type k = 0; k < ei.length(); k++) {
	  if (trace_level > 1) {
	    std::cerr << "comparing plan #" << i << " and plan #" << j
		      << ": step #" << ei[k].first
		      << " before step #" << ei[k].second
		      << " is specific to plan #" << i << std::endl;
	  }
	  bool found = false;
	  for (index_type l=0; (l<(*this)[i]->traits.length())&&!found; l++) {
	    PlanStepOrder* pso_trait =
	      (PlanStepOrder*)(*this)[i]->traits[l]->cast_to("PlanStepOrder");
	    if (pso_trait) {
	      if (pso_trait->precedence() == ei[k]) found = true;
	    }
	  }
	  if (!found) {
	    PlanStepOrder* new_pso_trait =
	      new PlanStepOrder((*this)[i], instance, ei[k]);
	    (*this)[i]->add_trait(new_pso_trait);
	  }
	}
	for (index_type k = 0; k < ej.length(); k++) {
	  if (trace_level > 1) {
	    std::cerr << "comparing plan #" << i << " and plan #" << j
		      << ": step #" << ej[k].first
		      << " before step #" << ej[k].second
		      << " is specific to plan #" << j << std::endl;
	  }
	  bool found = false;
	  for (index_type l=0; (l<(*this)[j]->traits.length())&&!found; l++) {
	    PlanStepOrder* pso_trait =
	      (PlanStepOrder*)(*this)[j]->traits[l]->cast_to("PlanStepOrder");
	    if (pso_trait) {
	      if (pso_trait->precedence() == ej[k]) found = true;
	    }
	  }
	  if (!found) {
	    PlanStepOrder* new_pso_trait =
	      new PlanStepOrder((*this)[j], instance, ej[k]);
	    (*this)[j]->add_trait(new_pso_trait);
	  }
	}
      }
    }
}

void ScheduleSet::add_distinguishing_traits_2()
{
  if (empty()) return;

  cost_vec val(0, length());
  cost_set vals;
  for (index_type i = 0; i < length(); i++) {
    val[i] = (*this)[i]->makespan();
    vals.insert((*this)[i]->makespan());
  }
  if (vals.length() > 1) {
    for (index_type i = 0; i < length(); i++) {
      NTYPE v = (*this)[i]->makespan();
      PlanFeatureValue* fv_trait =
	new PlanFeatureValue((*this)[i], PlanFeatureValue::makespan,
			     instance, no_such_index, v);
      if (v == vals[0]) fv_trait->set_min();
      if (v == vals[vals.length() - 1]) fv_trait->set_max();
      if (val.count(v) == 1) fv_trait->set_unique();
      (*this)[i]->add_trait(fv_trait);
    }
  }

  if (instance.n_resources() > 0) {
    lvector<ExecTrace*> trajs(0, 0);
    for (index_type i = 0; i < length(); i++) {
      ExecTrace* trace = new ExecTrace(instance);
      bool ok = (*this)[i]->simulate(trace, 0, true);
      trajs.append(trace);
    }
    for (index_type k = 0; k < instance.n_resources(); k++) {
      cost_vec peak_use(0, length());
      cost_vec amt_cons(0, length());
      cost_vec tol_loss(0, length());
      cost_set peak_use_vals;
      cost_set amt_cons_vals;
      cost_set tol_loss_vals;
      for (index_type i = 0; i < length(); i++) {
	ResourceProfile* rp = new ResourceProfile(instance, k, *(trajs[i]));
	peak_use[i] = rp->peak_use();
	peak_use_vals.insert(rp->peak_use());
	amt_cons[i] = rp->total_consumption();
	amt_cons_vals.insert(rp->total_consumption());
	tol_loss[i] = rp->tolerable_unexpected_loss();
	tol_loss_vals.insert(rp->tolerable_unexpected_loss());
	delete rp;
      }
      for (index_type i = 0; i < length(); i++) {
	if (peak_use_vals.length() > 1) {
	  NTYPE v = peak_use[i];
	  PlanFeatureValue* fv_trait =
	    new PlanFeatureValue((*this)[i],
				 PlanFeatureValue::resource_peak_use,
				 instance, k, v);
	  if (v == peak_use_vals[0])
	    fv_trait->set_min();
	  if (v == peak_use_vals[peak_use_vals.length() - 1])
	    fv_trait->set_max();
	  if (peak_use.count(v) == 1)
	    fv_trait->set_unique();
	  (*this)[i]->add_trait(fv_trait);
	}
	if (amt_cons_vals.length() > 1) {
	  NTYPE v = amt_cons[i];
	  PlanFeatureValue* fv_trait =
	    new PlanFeatureValue((*this)[i],
				 PlanFeatureValue::resource_total_consumption,
				 instance, k, v);
	  if (v == amt_cons_vals[0])
	    fv_trait->set_min();
	  if (v == amt_cons_vals[amt_cons_vals.length() - 1])
	    fv_trait->set_max();
	  if (amt_cons.count(v) == 1)
	    fv_trait->set_unique();
	  (*this)[i]->add_trait(fv_trait);
	}
	if (tol_loss_vals.length() > 1) {
	  NTYPE v = tol_loss[i];
	  PlanFeatureValue* fv_trait =
	    new PlanFeatureValue((*this)[i],
				 PlanFeatureValue::resource_tolerable_loss,
				 instance, k, v);
	  if (v == tol_loss_vals[0])
	    fv_trait->set_min();
	  if (v == tol_loss_vals[tol_loss_vals.length() - 1])
	    fv_trait->set_max();
	  if (tol_loss.count(v) == 1)
	    fv_trait->set_unique();
	  (*this)[i]->add_trait(fv_trait);
	}
      }
    }
  }
}

bool ScheduleSet::common_precedence_constraints(graph& prec)
{
  assert(!empty());

  (*this)[0]->base_precedence_graph(prec);
  for (index_type k = 1; k < length(); k++) {
    CorrespondanceEnumerator e((*this)[k]->step_actions(),
			       (*this)[0]->step_actions());
    bool ok = e.first();
    if (!ok) return false;

    graph pk;
    (*this)[k]->base_precedence_graph(pk);
    graph p0;
    p0.copy_and_rename(pk, e.current());
    prec.intersect(p0);

    ok = !e.next();
    if (!ok) return false;
  }

  return true;
}

bool ScheduleSet::separating_precedence_constraints
(ScheduleSet& s, pair_set& d0, pair_set& d1)
{
  graph p_this;
  if (!common_precedence_constraints(p_this)) return false;

  graph p_that;
  if (!s.common_precedence_constraints(p_that)) return false;

  CorrespondanceEnumerator e(s[0]->step_actions(),
			     (*this)[0]->step_actions());
  bool ok = e.first();
  if (!ok) return false;

  graph p_trans;
  p_trans.copy_and_rename(p_that, e.current());

  ok = !e.next();
  if (!ok) return false;

  p_this.difference(p_trans, d0, d1);

  return true;
}

void ScheduleSet::write_deordered_graphs(::std::ostream& s, bool w_names)
{
  for (index_type k = 0; k < length(); k++) {
    graph p;
    (*this)[k]->deorder(p);
    p.transitive_reduction();
    if (w_names) {
      name_vec a_names;
      (*this)[k]->step_action_names(a_names);
      write_labeled_digraph<name_vec>
	(s, p, a_names, false, (*this)[k]->plan_name()->to_cstring());
    }
    else {
      write_labeled_digraph<index_vec>
	(s, p, (*this)[k]->step_actions(), false,
	 (*this)[k]->plan_name()->to_cstring());
    }
  }
}

void ScheduleSet::writeXML(::std::ostream& s)
{
  s << "<schedule-set xmlns=\"http://dummy.net/pddlcat/schedule\">"
    << ::std::endl;
  for (index_type k = 0; k < length(); k++) {
    ExecTrace* trace = new ExecTrace(instance);
    ExecErrorSet* errors = new ExecErrorSet();
    (*this)[k]->simulate(trace, errors, true);
    graph p;
    (*this)[k]->deorder(p);
    p.transitive_closure();
    (*this)[k]->writeXML(s, errors, trace, &p, k);
    delete trace;
    delete errors;
  }
  s << "</schedule-set>" << ::std::endl;
}

Plan* ScheduleSet::new_plan()
{
  Schedule* p = new Schedule(instance);
  p->set_trace_level(trace_level);
  append(p);
  return p;
}

void ScheduleSet::output(PlanSet& to)
{
  for (index_type k = 0; k < length(); k++) {
    Plan* p = to.new_plan();
    if (p) {
      (*this)[k]->output(*p);
    }
  }
}

void ScheduleSet::output(PlanSet& to, const bool_vec& s)
{
  for (index_type k = 0; k < length(); k++) if (s[k]) {
    Plan* p = to.new_plan();
    if (p) {
      (*this)[k]->output(*p);
    }
  }
}

bool feasible
(Instance& ins,         // instance
 const index_vec& acts, // actions
 index_type r,          // resource (index)
 const index_set& a, // set of acts (indices) with req[r] > 0
 index_type i_a,     // a[i_a] = current element in acts
 const index_set& b, // set of acts with cons[r] > 0 not ordered w.r.t. a[i_a]
 index_type i_b,     // b[i_b] = next element in acts to branch on
 NTYPE c_max,        // worst case total cons of acts (pos/nec) < a[i_a]
 graph& uc,          // prec. graph under construction
 graph_vec* rfps,
 index_type* rff)
{
  bool ok = false;

  // try with additional constraint a[i_a] < b[i_b]
  NTYPE c = c_max - ins.actions[acts[b[i_b]]].cons[r];
  pair_set e;
  uc.add_edge_to_transitive_closure(a[i_a], b[i_b], e);

  // if this is enough to ensure feasibility of a[i_a], go to next...
  if ((ins.resources[r].init - c) >=  ins.actions[acts[a[i_a]]].req(r)) {
    if ((i_a + 1) == a.length()) {
      if ((r + 1) == ins.n_resources()) {
	if (rfps) rfps->append(uc);
	ok = true;
      }
      else {
	ok = feasible(ins, acts, r + 1, uc, rfps, rff);
      }
    }
    else {
      ok = feasible(ins, acts, r, a, i_a + 1, uc, rfps, rff);
    }
  }
  else {
    if ((i_b + 1) == b.length()) {
      return false;
    }
    else {
      ok = feasible(ins, acts, r, a, i_a, b, i_b + 1, c, uc, rfps, rff);
    }
  }
  uc.remove_edges(e);

  // if adding the constraint did not lead to a solution, OR if we desire
  // all solutions, try also without constraint a[i_a] < b[i_b]
  if (!ok || rfps) {
    ok = (ok || feasible(ins, acts, r, a, i_a, b, i_b + 1, c, uc, rfps, rff));
  }

  return ok;
}

bool feasible
(Instance& ins,         // instance
 const index_vec& acts, // actions
 index_type r,          // resource (index)
 const index_set& a,    // set of acts (indices) with req. > 0 on r
 index_type i,          // a[i] = next element in acts to check
 graph& uc,             // prec. graph under construction
 graph_vec* rfps,
 index_type* rff)
{
  assert(i < a.length());
  while (true) {
    index_set b_nec; // set of acts (indices) nec. < a[i]
    NTYPE c_nec = 0; // total cons[r] of acts in b_nec
    index_set b_pos; // set of acts (indices) not ordered w.r.t. a[i]
    NTYPE c_pos = 0; // total cons[r] of acts in b_pos
    for (index_type k = 0; k < a.length(); k++) if (k != i) {
      if (ins.actions[acts[a[k]]].cons[r] > 0) {
	if (uc.adjacent(a[k], a[i])) {
	  b_nec.insert(a[k]);
	  c_nec += ins.actions[acts[a[k]]].cons[r];
	}
	else if (!uc.adjacent(a[i], a[k])) {
	  b_pos.insert(a[k]);
	  c_pos += ins.actions[acts[a[k]]].cons[r];
	}
      }
    }

    // if initial cap. - total cons. of acts nec. < a[i] less than a[i]'s
    // requirement, there is no solution
    if ((ins.resources[r].init - c_nec) <  ins.actions[acts[a[i]]].req(r)) {
      return false;
    }

    // else, if initial cap. - total cons. of acts possibly < a[i] less
    // than a[i]'s requirement, additional ordering constraints must be
    // added
    else if ((ins.resources[r].init - (c_nec + c_pos)) <
	     ins.actions[acts[a[i]]].req(r)) {
      assert(b_pos.length() > 0);
      return feasible(ins, acts, r, a, i, b_pos, 0, c_nec + c_pos, uc, rfps, rff);
    }

    // else (init cap. - worst case total cons. > a[i]'s req.) there is no
    // conflict, so go to next action/resource...
    else {
      i += 1;
      if (i == a.length()) {
	if ((r + 1) == ins.n_resources()) {
	  if (rfps) rfps->append(uc);
	  return true;
	}
	else {
	  return feasible(ins, acts, r + 1, uc, rfps, rff);
	}
      }
    }
  }
}

bool feasible
(Instance& ins,         // instance
 const index_vec& acts, // actions
 index_type r,          // resource (index)
 graph& uc,             // prec. graph under construction
 graph_vec* rfps,
 index_type* rff)
{
  assert(r < ins.n_resources());
  while (true) {
    index_set a;     // set of acts (indices) with req. on resorce r
    NTYPE c_sum = 0; // sum cons of r over set a
    NTYPE u_max = 0; // max use of r over set a
    for (index_type k = 0; k < acts.length(); k++)
      if (ins.actions[acts[k]].req(r) > 0) {
	a.insert(k);
	u_max = MAX(u_max, ins.actions[acts[k]].use[r]);
	c_sum += ins.actions[acts[k]].cons[r];
      }

    // if max use exceeds initial capacity, there is no solution
    if (ins.resources[r].init < u_max) {
      std::cerr << "max use of resource " << ins.resources[r].name
		<< " = " << u_max << " exceeds capacity "
		<< ins.resources[r].init << std::endl;
      if (rff)
	*rff = r;
      return false;
    }

    // if initial cap - total consumption < max use, ordering may be needed
    if ((ins.resources[r].init - c_sum) < u_max) {
      bool ok = feasible(ins, acts, r, a, 0, uc, rfps, rff);
      if (!ok) {
	std::cerr << "can not resolve use/loss conflicts on resource "
		  << ins.resources[r].name << std::endl;
	if (rff)
	  *rff = r;
	return false;
      }
      return true;
    }

    // else go to next resource...
    else {
      r += 1;
      if (r == ins.n_resources()) {
	if (rfps) rfps->append(uc);
	if (rff) *rff = no_such_index;
	return true;
      }
    }
  }
}

bool feasible
(Instance& ins,
 const index_vec& acts,
 const graph& prec,
 graph_vec* rfps,
 index_type* rff)
{
  if (rff)
    *rff = no_such_index;
  if (!prec.acyclic()) {
    return false;
  }
  if (ins.n_resources() == 0) {
    if (rfps) rfps->append(prec);
    return true;
  }
  graph p(prec);
  p.transitive_closure();
  return feasible(ins, acts, 0, p, rfps, rff);
}

ScheduleTrait::~ScheduleTrait()
{
  // done
}

void ScheduleTrait::write_meta_short(::std::ostream& s) const
{
  bool first = true;
  if (is_min) {
    s << "[min";
    first = false;
  }
  if (is_max) {
    s << (first ? "[" : ",") << "max";
    first = false;
  }
  if (is_unique) {
    s << (first ? "[" : ",") << "unique";
    first = false;
  }
  if (!first) s << "]";
}

void ScheduleTrait::write_meta_attributes(::std::ostream& s) const
{
  s << " min=\"" << (is_min ? "true" : "false" )
    << "\" max=\"" << (is_max ? "true" : "false" )
    << "\" unique=\"" << (is_unique ? "true" : "false" )
    << "\"";
}

const PlanTrait* ScheduleTrait::cast_to(const char* n) const
{
  if (strcmp(n, "ScheduleTrait") == 0) return this;
  return 0;
}

void ScheduleTrait::write_detail(::std::ostream& s) const
{
  write_short(s);
  s << ::std::endl;
}

const PlanTrait* EquivalentTo::cast_to(const char* class_name) const
{
  if (strcmp(class_name, "EquivalentTo") == 0) return this;
  return ScheduleTrait::cast_to(class_name);
}

void EquivalentTo::write_short(::std::ostream& s) const
{
  assert(plan_set);
  index_type i_eq = plan_set->first(s_eq);
  if (i_eq != no_such_index) {
    s << "equivalent to plan #" << i_eq;
    if (s_eq->plan_name()) {
      s << " (" << s_eq->plan_name() << ")";
    }
  }
  else {
    s << "equivalent to plan@" << s_eq << " (not in plan set)";
  }
}

void EquivalentTo::write_detail(::std::ostream& s) const
{
  write_short(s);
  index_type i_eq = plan_set->first(s_eq);
  if (i_eq != no_such_index) {
    s << ::std::endl << "correpondance: ";
    write_correspondance(s, cor);
    s << ::std::endl;
    graph p;
    graph q;
    plan->deorder(p);
    p.transitive_reduction();
    s_eq->deorder(q);
    q.transitive_reduction();
    p.write_graph_correspondance(s, q, cor, "Correspondance Graph");
  }
}

void EquivalentTo::writeXML(::std::ostream& s) const
{
  index_type i_eq = plan_set->first(s_eq);
  if (i_eq != no_such_index) {
    s << "<equivalent-to";
    s << " plan=\"" << i_eq << "\"";
    if (s_eq->plan_name()) {
      s << " name=\"";
      s_eq->plan_name()->write(s, Name::NC_XML);
      s << "\"";
    }
    s << ">" << ::std::endl;
    s << "<cgraph><![CDATA[" << ::std::endl;
    graph p;
    graph q;
    plan->deorder(p);
    p.transitive_reduction();
    s_eq->deorder(q);
    q.transitive_reduction();
    p.write_graph_correspondance(s, q, cor, "Correspondance Graph");
    s << "]]></cgraph>" << ::std::endl;
    s << "</equivalent-to>" << ::std::endl;
  }
  else {
    s << "<equivalent-to/>" << ::std::endl;
  }
}

DerivedFrom::DerivedFrom
(Schedule* p, ScheduleSet* ss,
 DerivedFrom* a, const index_vec& m,
 const index_pair& e)
  : ScheduleTrait(p, ss), s_src(a->s_src)
{
  for (index_type k = 0; k < a->e_prec.length(); k++) {
    assert(m[a->e_prec[k].first] != no_such_index);
    assert(m[a->e_prec[k].second] != no_such_index);
    e_prec.insert(index_pair(m[a->e_prec[k].first], m[a->e_prec[k].second]));
  }
  e_prec.insert(e);
};

const PlanTrait* DerivedFrom::cast_to(const char* class_name) const
{
  if (strcmp(class_name, "DerivedFrom") == 0) return this;
  return ScheduleTrait::cast_to(class_name);
}

void DerivedFrom::write_short(::std::ostream& s) const
{
  assert(plan_set);
  index_type i_src = plan_set->first(s_src);
  if (i_src != no_such_index) {
    s << "derived from plan #" << i_src;
    if (s_src->plan_name()) {
      s << " (" << s_src->plan_name() << ")";
    }
    s << " by additional constraints " << e_prec;
  }
  else {
    s << "derived from plan@" << s_src << " (not in plan set)";
  }
}

void DerivedFrom::write_detail(::std::ostream& s) const
{
  write_short(s);
  s << std::endl;
}

void DerivedFrom::writeXML(::std::ostream& s) const
{
  index_type i_src = plan_set->first(s_src);
  if (i_src != no_such_index) {
    s << "<derived-from";
    s << " plan=\"" << i_src << "\"";
    if (s_src->plan_name()) {
      s << " name=\"";
      s_src->plan_name()->write(s, Name::NC_XML);
      s << "\"";
    }
    s << ">" << ::std::endl;
  }
  else {
    s << "<derived-from>" << ::std::endl;
  }
  for (index_type k = 0; k < e_prec.length(); k++) {
    s << "<prec-link from=\"" << e_prec[k].first
      << "\" to=\"" << e_prec[k].second << "\"/>" << ::std::endl;
  }
  s << "</derived-from>" << ::std::endl;
}

const PlanTrait* PlanPrecedenceRelation::cast_to(const char* class_name) const
{
  if (strcmp(class_name, "PlanPrecedenceRelation") == 0) return this;
  return ScheduleTrait::cast_to(class_name);
}

void PlanPrecedenceRelation::write_short(::std::ostream& s) const
{
  s << "precedence relation = " << prec;
}

void PlanPrecedenceRelation::writeXML(::std::ostream& s) const
{
  s << "<plan-precedence-relation>" << ::std::endl;
  graph p(prec);
  p.transitive_closure();
  for (index_type k = 0; k < p.size(); k++) {
    s << "<step id=\"" << k << "\">" << ::std::endl;
    for (index_type j = 0; j < p.size(); j++) if (p.adjacent(k, j)) {
      s << "<link step=\"" << j << "\" type=\"prec\"/>" << ::std::endl;
    }
    s << "</step>" << ::std::endl;
  }
  s << "</plan-precedence-relation>" << ::std::endl;
}

const PlanTrait* PlanActionOccurs::cast_to(const char* class_name) const
{
  if (strcmp(class_name, "PlanActionOccurs") == 0) return this;
  return ScheduleTrait::cast_to(class_name);
}

void PlanActionOccurs::write_short(::std::ostream& s) const
{
  if (n_of_times == 0) {
    s << "does not contain action " << instance.actions[act].name;
  }
  else if (n_of_times == 1) {
    s << "contains action " << instance.actions[act].name;
  }
  else {
    s << "action " << instance.actions[act].name
      << " occurs " << n_of_times << " times";
  }
  write_meta_short(s << " ");
}

void PlanActionOccurs::writeXML(::std::ostream& s) const
{
  s << "<plan-action-occurs action=\"" << act
    << "\" name=\"";
  instance.actions[act].name->write(s, Name::NC_XML);
  s << "\" count=\"" << n_of_times << "\"";
  write_meta_attributes(s);
  if (n_of_times > 0) {
    s << ">" << ::std::endl;
    for (index_type k = 0; k < plan->plan_steps().length(); k++)
      if (plan->plan_steps()[k].act == act) {
	s << "<link step=\"" << k << "\"/>" << ::std::endl;
      }
    s << "</plan-action-occurs>" << ::std::endl;
  }
  else {
    s << "/>" << ::std::endl;
  }
}

const PlanTrait* PlanStepOrder::cast_to(const char* class_name) const
{
  if (strcmp(class_name, "PlanStepOrder") == 0) return this;
  return ScheduleTrait::cast_to(class_name);  
}

void PlanStepOrder::write_short(::std::ostream& s) const
{
  assert(plan);
  assert(order.first < plan->n_steps());
  assert(order.second < plan->n_steps());
  assert(plan->step_actions()[order.first] < instance.n_actions());
  assert(plan->step_actions()[order.second] < instance.n_actions());
  s << instance.actions[plan->step_actions()[order.first]].name
    << " preceeds "
    << instance.actions[plan->step_actions()[order.second]].name;
}

void PlanStepOrder::writeXML(::std::ostream& s) const
{
  s << "<plan-step-order";
  write_meta_attributes(s);
  s << ">" << ::std::endl;
  s << "<prec-link from=\"" << order.first
    << "\" to=\"" << order.second << "\"/>" << ::std::endl;
  s << "</plan-step-order>" << ::std::endl;
}

const PlanTrait* PlanFeatureValue::cast_to(const char* class_name) const
{
  if (strcmp(class_name, "PlanFeatureValue") == 0) return this;
  return ScheduleTrait::cast_to(class_name);
}

void PlanFeatureValue::write_short(::std::ostream& s) const
{
  switch (ftype) {
  case makespan:
    s << "makespan = " << value;
    break;
  case cost:
    s << "cost = " << value;
    break;
  case resource_peak_use:
    s << "peak use of " << instance.resources[index].name << " = " << value;
    break;
  case resource_total_consumption:
    s << "amount of " << instance.resources[index].name << " consumed = "
      << value;
    break;
  case resource_tolerable_loss:
    s << "resource " << instance.resources[index].name << " tolerable loss = "
      << value;
    break;
  default:
    ::std::cerr << "error: invalid plan feature type " << ftype << ::std::endl;
    exit(255);
  }
  write_meta_short(s << " ");
}

void PlanFeatureValue::writeXML(::std::ostream& s) const
{
  s << "<plan-feature-value";
  switch (ftype) {
  case makespan:
    s << " type=\"makespan\"";
    break;
  case cost:
    s << " type=\"cost\"";
    break;
  case resource_peak_use:
    s << " type=\"peak-use\" resource=\"" << index << "\" name=\"";
    instance.resources[index].name->write(s, Name::NC_XML);
    s << "\"";
    break;
  case resource_total_consumption:
    s << " type=\"total-consumption\" resource=\"" << index << "\" name=\"";
    instance.resources[index].name->write(s, Name::NC_XML);
    s << "\"";
    break;
  case resource_tolerable_loss:
    s << " type=\"tolerable-loss\" resource=\"" << index << "\" name=\"";
    instance.resources[index].name->write(s, Name::NC_XML);
    s << "\"";
    break;
  default:
    ::std::cerr << "error: invalid plan feature type " << ftype << ::std::endl;
    exit(255);
  }
  s << " value=\"" << value << "\"";
  write_meta_attributes(s);
  switch (ftype) {
  case resource_peak_use:
  case resource_total_consumption:
  case resource_tolerable_loss:
    s << ">" << ::std::endl
      << "<link resource=\"" << index << "\"/>" << ::std::endl
      << "</plan-feature-value>" << ::std::endl;
    break;
  default:
    s << "/>" << ::std::endl;
  }
}

void PlanName::write(::std::ostream& s, unsigned int c) const
{
  write_string_escaped(s, desc, c);
  write_string_escaped(s, " #", c);
  s << index;
  if (src) {
    write_string_escaped(s, " of ", c);
    src->write(s, c);
  }
}

END_HSPS_NAMESPACE
