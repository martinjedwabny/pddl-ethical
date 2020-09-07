
#include "plans.h"
#include "base.h"

BEGIN_HSPS_NAMESPACE

void ActionSequence::protect(index_type atom)
{
  // ignored
}

void ActionSequence::insert(index_type act)
{
  append(act);
}

void ActionSequence::advance(NTYPE delta)
{
  // ignored
}

void ActionSequence::end()
{
  // ignored
}

void ActionSequence::output(Plan& to)
{
  for (index_type k = 0; k < length(); k++) {
    to.insert((*this)[k]);
    if (k + 1 < length()) to.advance(1);
  }
}

Plan* ActionSequenceSet::new_plan() {
  inc_length();
  (*this)[length() - 1].set_length(0);
  return &((*this)[length() - 1]);
}

void ActionSequenceSet::output(PlanSet& to)
{
  for (index_type k = 0; k < length(); k++) {
    Plan* p = to.new_plan();
    if (p) {
      (*this)[k].output(*p);
    }
  }
}

count_type CountActions::sum()
{
  count_type s = 0;
  for (index_type k = 0; k < length(); k++)
    s += (*this)[k];
  return s;
}

Plan* CountActions::new_plan()
{
  return this;
}

void CountActions::protect(index_type atom)
{
  // ignored
}

void CountActions::insert(index_type act)
{
  assert(act < length());
  (*this)[act] += 1;
}

void CountActions::advance(NTYPE delta)
{
  // ignored
}

void CountActions::end()
{
  // ignored
}

void ApxResult::solution(State& s, NTYPE cost)
{
  Result::solution(s, cost);
  bool max_state = false;
  for (State* sp = &s; sp && !max_state; sp = sp->predecessor())
    if (sp->is_max()) max_state = true;
  if (!max_state) min_sol = true;
  index_type d = s.depth();
  if (d > sol_depth) sol_depth = d;
}

bool ApxResult::more()
{
  return false;
}

bool Print::print_noops = false;
bool Print::decimal_time = true;

Print::Print(Instance& i, std::ostream& s)
  : instance(i),
    to(s),
    current_t(0),
    peak_use(0, 0),
    res_cons(0, 0),
    n_actions(0),
    n_plans(0),
    sum_cost(0)
{
  // done
}

Print::~Print()
{
  // done
}

Plan* Print::new_plan()
{
  n_plans += 1;

  current_t = 0;
  peak_use.assign_value(I_TO_N(0));
  res_cons.assign_value(I_TO_N(0));
  n_actions = 0;

  to << "plan:" << std::endl;

  return this;
}

void Print::protect(index_type atom) {
  if (print_noops) {
    to << " " << current_t << ": noop"
       << instance.atoms[atom].name
       << " (#" << atom << ")" << std::endl;
  }
}

void Print::insert(index_type act) {
#ifdef NTYPE_RATIONAL
  if (decimal_time) {
    NTYPE end_t = current_t + instance.actions[act].dur;
    to << " [" << current_t.decimal() << "," << end_t.decimal() << "]";
  }
  else {
    to << " [" << current_t << ","
       << current_t + instance.actions[act].dur << "]";
  }
#else
  to << " [" << current_t << ","
     << current_t + instance.actions[act].dur << "]";
#endif
  to << instance.actions[act].name << " (#" << act << ")" << std::endl;
  for (index_type k = 0; k < instance.n_resources(); k++)
    res_cons[k] += instance.actions[act].cons[k];
  n_actions += 1;
  sum_cost += instance.actions[act].cost;
}

void Print::advance(NTYPE delta)
{
  current_t += delta;
}

void Print::end()
{
#ifdef NTYPE_RATIONAL
  if (decimal_time) {
    to << "makespan: " << current_t.decimal();
  }
  else {
    to << "makespan: " << current_t;
  }
#else
  to << "makespan: " << current_t;
#endif
  to << ", " << n_actions << " actions" << std::endl;
  if (instance.n_resources() > 0) {
    to << "resources consumed:";
    for (index_type k = 0; k < instance.n_resources(); k++) {
      to << " " << instance.resources[k].name << "=" << res_cons[k];
    }
    to << std::endl;
  }
}

PrintActions::PrintActions(Instance& i, std::ostream& s)
  : instance(i), to(s),
    action_sep('\n'), first_action(true),
    plan_sep('\n'), first_plan(true)
{
  // done
}

PrintActions::PrintActions(Instance& i, std::ostream& s, char as)
  : instance(i), to(s),
    action_sep(as), first_action(true),
    plan_sep('\n'), first_plan(true)
{
  // done
}

PrintActions::PrintActions(Instance& i, std::ostream& s, char as, char ps)
  : instance(i), to(s),
    action_sep(as), first_action(true),
    plan_sep(ps), first_plan(true)
{
  // done
}

PrintActions::~PrintActions()
{
  // done
}

Plan* PrintActions::new_plan()
{
  if (!first_plan) to << plan_sep;
  first_plan = false;
  first_action = true;
  return this;
}

void PrintActions::protect(index_type atom)
{
  // ignore
}

void PrintActions::insert(index_type act)
{
  if (!first_action) to << action_sep;
  instance.actions[act].name->write(to, Name::NC_PLAN);
  first_action = false;
}

void PrintActions::advance(NTYPE delta)
{
  // ignore
}

void PrintActions::end()
{
  // ignore
}

void PrintActionSetNames::insert(index_type act)
{
  if (!first_action) to << action_sep;
  if (instance.actions[act].src) {
    ptr_pair* p = (ptr_pair*)instance.actions[act].src;
    PDDL_Base::ActionSymbol* a = (PDDL_Base::ActionSymbol*)p->first;
    if (a->part) {
      ptr_table* ai = (ptr_table*)p->second;
      ptr_table::key_vec* args = ai->key_sequence();
      for (index_type k = 0; k < a->param.length(); k++)
	a->param[k]->value = (PDDL_Base::Symbol*)((*args)[k + 1]);
      delete args;
      a->part->print_instance(to);
    }
    else {
      instance.actions[act].name->write(to, Name::NC_PLAN);
    }
  }
  else {
    instance.actions[act].name->write(to, Name::NC_PLAN);
  }
  first_action = false;
}

PrintPDDL::PrintPDDL(Instance& i, std::ostream& s)
  : instance(i), to(s), start_t(0)
{
  // done
}

PrintPDDL::~PrintPDDL()
{
  // done
}

Plan* PrintPDDL::new_plan()
{
  start_t = 0;
  to << "(:plan";
  return this;
}

void PrintPDDL::protect(index_type atom)
{
  // ignored
}

void PrintPDDL::insert(index_type act)
{
  to << std::endl << "  " << PRINT_NTYPE(start_t)
     << " : ";
  instance.actions[act].name->write(to, Name::NC_PLAN | Name::NC_PDDL);
}

void PrintPDDL::advance(NTYPE delta)
{
  start_t += delta;
}

void PrintPDDL::end()
{
  to << ")" << std::endl;
}


PrintIPC::PrintIPC(Instance& i, std::ostream& s)
  : instance(i),
    to(s),
    temporal(false),
    epsilon(0),
    included_epsilon(false),
    strict_separation(false),
    start_t(0),
    occ_current_t(false)
{
  // done
}

PrintIPC::~PrintIPC()
{
  // done
}

void PrintIPC::set_epsilon(NTYPE e, bool included, bool strict)
{
  temporal = true;
  epsilon = e;
  included_epsilon = included;
  strict_separation = strict;
}

Plan* PrintIPC::new_plan()
{
  start_t = 0;
  occ_current_t = false;
  return this;
}

void PrintIPC::protect(index_type atom)
{
  // ignored
}

void PrintIPC::insert(index_type act)
{
  if (temporal && strict_separation) {
    if (occ_current_t) {
      start_t += epsilon;
    }
    else {
      occ_current_t = true;
    }
  }
  to << PRINT_NTYPE(start_t) << " : ";
  instance.actions[act].name->write(to, Name::NC_PLAN | Name::NC_IPC);
  if (temporal) {
    if (included_epsilon) {
      NTYPE d = instance.actions[act].dur;
      d -= epsilon;
      to << " [" << PRINT_NTYPE(d) << "]";
    }
    else {
      to << " [" << PRINT_NTYPE(instance.actions[act].dur) << "]";
    }
  }
  else {
    to << " [1]"; // TEMP HACK
  }
  to << std::endl;
}

void PrintIPC::advance(NTYPE delta)
{
  start_t += delta;
  if (!included_epsilon) start_t += epsilon;
  occ_current_t = false;
}

void PrintIPC::end()
{
  // ignored
}

PrintAssoc::PrintAssoc(Instance& i, std::ostream& s)
  : instance(i), to(s)
{
  // done
}

PrintAssoc::~PrintAssoc()
{
  // done
}

void PrintAssoc::protect(index_type atom)
{
  // ignored
}

void PrintAssoc::insert(index_type act)
{
  if (act > instance.n_actions()) {
    std::cerr << "error: undefined action " << act << " in plan" << std::endl;
    exit(255);
  }
  if (instance.actions[act].assoc) {
    to << instance.actions[act].assoc;
  }
}

void PrintAssoc::advance(NTYPE delta)
{
  // ignored
}

void PrintAssoc::end()
{
  // ignored
}

Store::Store(Instance& i)
  : ScheduleSet(i)
{
  // done
}

Store::~Store()
{
  clear();
}

void Store::reset()
{
  clear();
  Result::reset();
}

void Store::clear()
{
  for (index_type k = 0; k < solutions.length(); k++)
    if (solutions[k]) solutions[k]->delete_path();
  solutions.clear();
  ScheduleSet::clear();
  Result::reset();
}

void Store::solution(State& s, NTYPE cost)
{
  // std::cerr << "storing solution..." << std::endl;
  Result::solution(s, cost);
  solutions.append(s.copy_path());
  Schedule* new_plan = new Schedule(instance);
  s.insert_path(*new_plan);
  new_plan->end();
  append(new_plan);
}

void StoreFilter::no_more_solutions(NTYPE cost)
{
  store.no_more_solutions(cost);
}

bool StoreFilter::more()
{
  return store.more();
}

void StoreMinCost::solution(State& s, NTYPE cost)
{
  NTYPE c_min = POS_INF;
  for (index_type k = 0; k < store.n_solutions(); k++) {
    if (store.solution(k)->acc_cost() < cost) return;
    c_min = MIN(c_min, store.solution(k)->acc_cost());
  }
  if (cost < c_min) {
    store.clear();
    store.solution(s, cost);
  }
  else if (cost == c_min) {
    store.solution(s, cost);
  }
}

count_type StoreDistinct::n_discarded = 0;

void StoreDistinct::solution(State& s, NTYPE cost)
{
  Schedule* new_plan = new Schedule(instance);
  s.insert_path(*new_plan);
  new_plan->end();
//   std::cerr << "comparing new plan to " << store.n_plans()
// 	    << " stored plans..." << std::endl;
  for (index_type k = 0; k < store.n_plans(); k++) {
    if (store.plan(k)->equivalent(*new_plan)) {
//       std::cerr << "new plan equivalent to stored plan " << k
// 		<< " - discarding it " << std::endl;
      delete new_plan;
      n_discarded += 1;
      std::cerr << store.n_solutions() << " solutions kept, "
		<< n_discarded << " discarded" << std::endl;
      return;
    }
  }
  delete new_plan;
//   std::cerr << "new plan not equivalent to any stored plan - storing it "
// 	    << std::endl;
  store.solution(s, cost);
  std::cerr << store.n_solutions() << " solutions kept, "
	    << n_discarded << " discarded" << std::endl;
}

State* Store::solution(index_type k)
{
  if (k < solutions.length()) return solutions[k];
  else return 0;
}

Schedule* Store::plan(index_type k)
{
  if (k < length()) return (*this)[k];
  else return 0;
}

void Store::output(PlanSet& to)
{
  ScheduleSet::output(to);
}

void Store::output(Result& to)
{
  for (index_type k = 0; k < solutions.length(); k++) {
    to.solution(*solutions[k], solutions[k]->acc_cost());
  }
  to.no_more_solutions(POS_INF);
}

Conflicts::Conflicts(Instance& ins)
  : instance(ins),
    nec_del(EMPTYSET),
    pos_del(EMPTYSET),
    min_peak(0, 0),
    max_peak(0, 0),
    first(true),
    need_to_clear(false)
{
  // done
}

Conflicts::~Conflicts()
{
  // done
}

void Conflicts::clear()
{
  nec_del.clear();
  pos_del.clear();
  min_peak.assign_value(0);
  max_peak.assign_value(0);
  first = true;
  need_to_clear = false;
}

void Conflicts::solution(State& s, NTYPE cost)
{
  Result::solution(s, cost);

  if (need_to_clear) clear();

  Schedule* plan = new Schedule(instance);
  plan->set_trace_level(0);
  s.insert_path(*plan);
  plan->end();

  const Schedule::step_vec& seq = plan->plan_steps();
  index_set del;
  for (index_type k = 0; k < seq.length(); k++) {
    del.insert(instance.actions[seq[k].act].del);
  }
  if (first) {
    nec_del = del;
  }
  else {
    nec_del.intersect(del);
  }
  pos_del.insert(del);

  ExecTrace* trace = new ExecTrace(instance);
  plan->simulate(trace);
  amt_vec peak(0, instance.n_resources());
  trace->peak_resource_use(peak);
  if (first) {
    for (index_type k = 0; k < instance.n_resources(); k++) {
      min_peak[k] = peak[k];
      max_peak[k] = peak[k];
    }
  }
  else {
    for (index_type k = 0; k < instance.n_resources(); k++) {
      min_peak[k] = MIN(min_peak[k], peak[k]);
      max_peak[k] = MAX(max_peak[k], peak[k]);
    }
  }

  first = false;
}

void Conflicts::no_more_solutions()
{
  need_to_clear = true;
  first = true;
}

END_HSPS_NAMESPACE
