
#include "collector.h"

BEGIN_HSPS_NAMESPACE

Collector::Collector(Instance& i, State& s)
  : instance(i),
    state(s),
    sp(s.is_final()),
    max(0),
    first(false),
    search_p(0)
{
  if (sp) {
    state.insert(*this);
    max = pre.length();
  }
}

State* Collector::predecessor() {
  return state.predecessor();
}

void Collector::set_predecessor(State* p)
{
  state.set_predecessor(p);
}

NTYPE Collector::delta_cost() {
  return state.delta_cost();
}

NTYPE Collector::est_cost() {
  return state.est_cost();
}

bool Collector::is_final() {
  return state.is_final();
}

bool Collector::is_max() {
  return state.is_max();
}

int Collector::compare(const State& s) {
  return state.compare(s);
}

index_type Collector::hash() {
  return state.hash();
}

State* Collector::copy() {
  return state.copy();
}

void Collector::insert(Plan& p) {
  state.insert(p);
}

void Collector::insert_path(Plan& p) {
  state.insert_path(p);
}

void Collector::write(std::ostream& s) {
  state.write(s);
}

void Collector::write_plan(std::ostream& s) {
  state.write_plan(s);
}

NTYPE Collector::expand(Search& s, NTYPE bound) {
  search_p = &s;
  sp = state.is_max();
  first = true;
  NTYPE val = state.expand(*this, bound);
  if (sp) {
    state.insert(*this);
    if (pre.length() > max) max = pre.length();
  }
  return val;
}

void Collector::store(NTYPE cost, bool opt)
{
  state.store(cost, opt);
}

void Collector::reevaluate()
{
  state.reevaluate();
}

NTYPE Collector::new_state(State& s, NTYPE bound) {
  Collector c(instance, s);
  NTYPE val = search_p->new_state(c, bound);
  if (state.is_max()) {
    if (c.sp) {
      nec_del.insert(c.nec_del);
      pos_del.insert(c.pos_del);
      if (c.max > max) max = c.max;
    }
    sp = (sp && c.sp);
  }
  else {
    if (c.sp) {
      if (first) {
	nec_del = c.nec_del;
	pos_del = c.pos_del;
      }
      else {
	nec_del.intersect(c.nec_del);
	pos_del.insert(c.pos_del);
      }
      first = false;
      if (c.max > max) max = c.max;
    }
    sp = (sp || c.sp);
  }
  return val;
}

bool Collector::solved() const {
  assert(search_p);
  return search_p->solved();
}

bool Collector::optimal() const {
  assert(search_p);
  return search_p->optimal();
}

bool Collector::done() const {
  assert(search_p);
  return search_p->done();
}

void Collector::protect(index_type atom) {
  pre.insert(atom);
}

void Collector::insert(index_type act) {
  pre.insert(instance.actions[act].pre);
  nec_del.insert(instance.actions[act].del);
  pos_del.insert(instance.actions[act].del);
}

void Collector::advance(NTYPE delta) {
}

void Collector::end() {
}

END_HSPS_NAMESPACE
