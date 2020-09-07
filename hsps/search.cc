
#include "search.h"

BEGIN_HSPS_NAMESPACE

State::~State()
{
  // done
}

const State* State::predecessor() const
{
  return pre;
}

State* State::predecessor()
{
  return pre;
}

void State::set_predecessor(State* p)
{
  pre = p;
}

bool State::is_encapsulated()
{
  return false;
}

NTYPE State::acc_cost()
{
  if (predecessor()) {
    return delta_cost() + predecessor()->acc_cost();
  }
  else {
    return 0;
  }
}

index_type State::depth() const
{
  if (predecessor()) {
    return 1 + predecessor()->depth();
  }
  else {
    return 0;
  }
}

void State::write_eval(::std::ostream& s, char* p, bool e)
{
  if (p) s << p << " ";
  s << est_cost();
  if (e) s << ::std::endl;
}

State* State::copy_path()
{
  State* s = copy();
  if (predecessor()) {
    s->set_predecessor(predecessor()->copy_path());
  }
  return s;
}

void State::delete_path()
{
  if (predecessor()) {
    predecessor()->delete_path();
  }
  delete this;
}

int State::compare_path(const State* s)
{
  if (s == 0) return 1;
  int c = compare(*s);
  if (c == 0) {
    if (predecessor() == 0) return -1;
    return predecessor()->compare_path(s->predecessor());
  }
  return c;
}

void State::write_path(::std::ostream& s)
{
  State* p = predecessor();
  if (p) {
    p->write_path(s);
  }
  write(s);
  s << ::std::endl;
}

void State::write_path_as_graph(::std::ostream& s)
{
  State* p = predecessor();
  if (p) {
    p->write_path_as_graph(s);
  }
  s << "S" << depth() << " [label=\"";
  write(s);
  if (is_final()) {
    s << ",style=\"bold\"";
  }
  s << "\"];" << ::std::endl;
  if (p) {
    s << "S" << depth() << " -> S" << p->depth()
      << " [label=\"";
    write_plan(s);
    s << "\"];" << ::std::endl;
  }
}

void ProgressionState::insert_path(Plan& p)
{
  if (predecessor()) {
    predecessor()->insert_path(p);
  }
  insert(p);
}

void RegressionState::insert_path(Plan& p)
{
  State* sp = this;
  while (sp != 0) {
    sp->insert(p);
    sp = sp->predecessor();
  }
}

PlanTrait::~PlanTrait()
{
  // done
}

const PlanTrait* PlanTrait::cast_to(const char* n) const
{
  return 0;
}

Plan::~Plan()
{
  // done
}

void Plan::output(Plan& to)
{
  // default: do nothing
}

void Plan::set_name(const Name* n)
{
  // default: ignore
}

void Plan::set_optimal(bool o)
{
  // default: ignore
}

void Plan::add_trait(PlanTrait* t)
{
  delete t;
}

Search::~Search()
{
  // done
}

NoSearch::~NoSearch()
{
  // done
}

void NoSearch::reset()
{
  _solved = false;
}

NTYPE NoSearch::new_state(State& s, NTYPE bound)
{
  if (s.is_final()) _solved = true;
  return s.est_cost();
}

bool NoSearch::solved() const
{
  return _solved;
}

bool NoSearch::optimal() const
{
  return false;
}

bool NoSearch::done() const
{
  return false;
}

Transitions::Transitions()
  : state_vec((State*)0, 0), target_state(0)
{
  // done
}

Transitions::Transitions(State* from, State* to, NTYPE d)
  : state_vec((State*)0, 0), target_state(0)
{
  find(from, to, d, true);
}

Transitions::~Transitions()
{
  clear();
}

void Transitions::clear()
{
  for (index_type k = 0; k < length(); k++)
    delete (*this)[k];
  state_vec::clear();
}

bool Transitions::find(State* from, State* to, NTYPE d, bool x)
{
  target_state = to;
  delta_bound = d;
  bound_is_exact = x;
  from->expand(*this, delta_bound + to->est_cost());
  target_state = 0;
  return (length() > 0);
}

NTYPE Transitions::new_state(State& s, NTYPE bound)
{
  if (!target_state) {
    ::std::cerr << "error: Transitions::new_state called with state = ";
    s.write(::std::cerr);
    ::std::cerr << " and target_state == nil" << ::std::endl;
    exit(255);
  }
  if (((s.delta_cost() == delta_bound) || !bound_is_exact) &&
      (target_state->compare(s) == 0)) {
    append(s.copy());
  }
}

bool Transitions::solved() const
{
  return (length() > 0);
}

bool Transitions::optimal() const
{
  return false;
}

bool Transitions::done() const
{
  return false;
}

StateFactory::~StateFactory()
{
  // done
}

PlanSet::~PlanSet()
{
  // done
}

void PlanSet::output(PlanSet& to)
{
  // done
}

void PlanSet::output(PlanSet& to, const bool_vec& s)
{
  // done
}

END_HSPS_NAMESPACE
