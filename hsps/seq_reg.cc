
#include "seq_reg.h"

BEGIN_HSPS_NAMESPACE

SeqRegState::SeqRegState(const SeqRegState& s)
  : AtomSetState(s), cost(s.cost), act(s.act), res(0)
{
  if (s.res) res = s.res->copy();
}

SeqRegState::~SeqRegState()
{
  if (res) delete res;
}

bool SeqRegState::relevant(Instance::Action& a)
{
  bool rel = false;
  for (index_type i = 0; (i < a.add.length()) && !rel; i++)
    if (set[a.add[i]]) rel = true;
  return rel;
}

bool SeqRegState::applicable(Instance::Action& a)
{
  bool app = true;
  for (index_type i = 0; (i < a.del.length()) && app; i++)
    if (set[a.del[i]]) app = false;
  if (res) {
    if (app) {
      if (!res->applicable(a)) app = false;
    }
  }
  return app;
}

SeqRegState* SeqRegState::apply(Instance::Action& a)
{
  SeqRegState* s = (SeqRegState*)copy();
  for (index_type k = 0; k < a.add.length(); k++) {
    if (s->set[a.add[k]]) s->size -= 1;
    s->set[a.add[k]] = false;
  }
  for (index_type k = 0; k < a.pre.length(); k++) {
    if (!s->set[a.pre[k]]) s->size += 1;
    s->set[a.pre[k]] = true;
  }
  s->State::set_predecessor(this);
  s->act = a.index;
  s->eval();
#ifdef APPLY_PATH_MAX
  s->apply_path_max();
#endif
#ifdef CHECK_HEURISTIC_CONSISTENCY
  if (s->delta_cost() + s->est_cost() < est_cost()) {
    std::cerr << "inconsistent heuristic" << std::endl;
    std::cerr << "parent state: " << *this
	      << ", est. cost = " << est_cost()
	      << std::endl;
    std::cerr << "action: " << a.name
	      << ", cost = " << cost(a.index)
	      << std::endl;
    std::cerr << "child state: " << *s
	      << ", delta + est. cost = "
	      << s->delta_cost() << " + " << s->est_cost()
	      << std::endl;
    std::cerr << "re-evaluating parent state..." << std::endl;
    eval_with_trace_level(3);
    std::cerr << "re-evaluating child state..." << std::endl;
    s->eval_with_trace_level(3);
    exit(255);
  }
#endif
  if (s->res) {
    s->res->apply(a);
    if (!s->res->sufficient_consumable(s->set)) s->est = POS_INF;
  }
  return s;
}

NTYPE SeqRegState::delta_cost()
{
  if (act == no_such_index)
    return 0;
  else
    return cost(act);
}

NTYPE SeqRegState::expand(Search& s, NTYPE bound)
{
  NTYPE c_min = POS_INF;
#ifdef SEARCH_EXTRA_STATS
  count_type n_succ = 0;
#endif
  for (index_type k = 0; k < instance.n_actions(); k++) {
    Instance::Action& a = instance.actions[k];
    if (a.sel) {
      if (relevant(a)) {
#ifdef TRACE_PRINT_LOTS
	std::cerr << depth() << ". action " << instance.actions[k].name
		  << " is relevant" << std::endl;
#endif
	if (applicable(a)) {
#ifdef TRACE_PRINT_LOTS
	  std::cerr << depth() << ". action " << instance.actions[k].name
		    << " is applicable" << std::endl;
#endif
	  NTYPE act_est = heuristic.eval_precondition(a);
#ifdef TRACE_PRINT_LOTS
	  std::cerr << depth() << ". est. prec. cost = "
		    << cost(a.index) << " + " << act_est << std::endl;
#endif
	  if (res) {
	    if (!res->sufficient_consumable(a.pre, a)) act_est = POS_INF;
#ifdef TRACE_PRINT_LOTS
	    std::cerr << depth() << ". est. prec. cost after resource check = "
		      << cost(a.index) << " + " << act_est << std::endl;
#endif
	  }
	  if (FINITE(act_est) && (cost(a.index) + act_est <= bound)) {
	    SeqRegState* new_s = apply(a);
#ifdef TRACE_PRINT_LOTS
	    std::cerr << depth() << ". successor state: " << *new_s
		      << std::endl;
#endif
#ifdef SEARCH_EXTRA_STATS
	    rminc_count += 1;
	    rminc_size += atom_set_size();
	    rminc_succ_size += new_s->atom_set_size();
	    rminc_succ_size_ratio +=
	      (new_s->atom_set_size()/(double)atom_set_size());
	    n_succ += 1;
#endif
	    if (FINITE(new_s->est) && (cost(a.index) + new_s->est <= bound)) {
#ifdef TRACE_PRINT_LOTS
	      std::cerr << depth() << ". successor state accepted"
			<< std::endl;
#endif
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
	    else c_min = MIN(c_min, cost(a.index) + new_s->est);
	    delete new_s;
	  }
	  else c_min = MIN(c_min, cost(a.index) + act_est);
	}
      }
    }
  }
#ifdef SEARCH_EXTRA_STATS
  rminx_count += 1;
  rminx_size += atom_set_size();
  rminx_succ += n_succ;
#endif
  return c_min;
}

void SeqRegState::store(NTYPE cost, bool opt)
{
  if (res) {
    if (!res->is_root()) return;
  }
  heuristic.store(set, cost, opt);
}

void SeqRegState::set_predecessor(State* p)
{
  if (p == 0) {
    act = no_such_index;
    pre = 0;
    return;
  }
  SeqRegState* s = (SeqRegState*)p;
  for (index_type k = 0; k < instance.n_actions(); k++) {
    Instance::Action& a = instance.actions[k];
    if (a.sel && s->relevant(a) && s->applicable(a)) {
      SeqRegState* new_s = s->apply(a);
      int d = compare(*new_s);
      delete new_s;
      if (d == 0) {
	act = k;
	pre = s;
	return;
      }
    }
  }
  std::cerr << "error in SeqRegState::set_predecessor: state " << *this
	    << " can not be a successor of " << *s << std::endl;
  assert(0);
}

void SeqRegState::insert(Plan& p)
{
  if (act != no_such_index) {
    p.insert(act);
    for (index_type k = 0; k < instance.n_atoms(); k++)
      if (set[k] && !instance.actions[act].add.contains(k))
	p.protect(k);
    p.advance(instance.actions[act].dur);
  }
}

void SeqRegState::write_plan(std::ostream& s)
{
  if (act != no_such_index) {
    s << instance.actions[act].name;
  }
}

void SeqRegState::insert_path(Plan& p)
{
  insert(p);
  if (predecessor()) {
    predecessor()->insert_path(p);
  }
}

State* SeqRegState::copy()
{
  return new SeqRegState(*this);
}

State* SeqRegState::new_state(const index_set& s, State* p)
{
  SeqRegState* new_s =
    (res ? new SeqRegState(instance, heuristic, cost, s, res->new_state())
         : new SeqRegState(instance, heuristic, cost, s));
  new_s->State::set_predecessor(p);
  return new_s;
}

State* SeqRegState::new_state(const bool_vec& s, State* p)
{
  SeqRegState* new_s =
    (res ? new SeqRegState(instance, heuristic, cost, s, res->new_state())
         : new SeqRegState(instance, heuristic, cost, s));
  new_s->State::set_predecessor(p);
  return new_s;
}

void SeqRegState::write(std::ostream& s)
{
  instance.write_atom_set(s, set);
  if (res) {
    s << " (";
    res->write(s);
    s << ")";
  }
}

bool ApxSeqRegState::is_max()
{
  return (size > limit);
}

NTYPE ApxSeqRegState::expand(Search& s, NTYPE bound)
{
  if (size > limit) {
    return split(limit, *this, this, s, bound);
  }
  else {
    NTYPE c_rec = SeqRegState::expand(s, bound);
    return c_rec;
  }
}

State* ApxSeqRegState::copy()
{
  return new ApxSeqRegState(*this);
}

State* ApxSeqRegState::new_state(const index_set& s, State* p)
{
  ApxSeqRegState* new_s =
    (res ? new ApxSeqRegState(instance, heuristic, cost, s, res, limit)
         : new ApxSeqRegState(instance, heuristic, cost, s, limit));
  new_s->State::set_predecessor(p);
  return new_s;
}

State* ApxSeqRegState::new_state(const bool_vec& s, State* p)
{
  ApxSeqRegState* new_s =
    (res ? new ApxSeqRegState(instance, heuristic, cost, s, res, limit)
         : new ApxSeqRegState(instance, heuristic, cost, s, limit));
  new_s->State::set_predecessor(p);
  return new_s;
}

NTYPE SeqCRegState::expand(Search& s, NTYPE bound)
{
  NTYPE c_min = POS_INF;
  for (index_type k = 0; k < instance.n_actions(); k++) {
    Instance::Action& a = instance.actions[k];
    if (a.sel) {
      if (relevant(a)) {
	bool app = applicable(a);
	if (app) {
	  if ((act != no_such_index) && (act < a.index) &&
	      instance.commutative(act, a.index)) app = false;
	}
	if (app) {
	  NTYPE act_est = heuristic.eval_precondition(a);
	  if (res) {
	    if (!res->sufficient_consumable(a.pre, a)) act_est = POS_INF;
	  }
	  if (FINITE(act_est) && (cost(a.index) + act_est <= bound)) {
	    SeqCRegState* new_s = (SeqCRegState*)apply(a);
	    if (FINITE(new_s->est) && (cost(a.index) + new_s->est <= bound)) {
	      NTYPE c_new = (cost(a.index) +
			     s.new_state(*new_s, bound - cost(a.index)));
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
	  else c_min = MIN(c_min, cost(a.index) + act_est);
	}
      }
    }
  }
  return c_min;
}

State* SeqCRegState::copy()
{
  return new SeqCRegState(*this);
}

State* SeqCRegState::new_state(const index_set& s, State* p)
{
  SeqCRegState* new_s =
    (res ? new SeqCRegState(instance, heuristic, cost, s, res)
         : new SeqCRegState(instance, heuristic, cost, s));
  new_s->State::set_predecessor(p);
  return new_s;
}

State* SeqCRegState::new_state(const bool_vec& s, State* p)
{
  SeqCRegState* new_s =
    (res ? new SeqCRegState(instance, heuristic, cost, s, res)
         : new SeqCRegState(instance, heuristic, cost, s));
  new_s->State::set_predecessor(p);
  return new_s;
}

int SeqCRegState::compare(const State& s)
{
  if (act < ((SeqCRegState&)s).act) return -1;
  else if (act > ((SeqCRegState&)s).act) return 1;
  else return AtomSetState::compare(s);
}

index_type SeqCRegState::hash()
{
  return (AtomSetState::hash() + act);
}

void SeqCRegState::write(std::ostream& s)
{
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

bool LDSeqRegState::copy_as_ld = true;

LDSeqRegState::LDSeqRegState(const LDSeqRegState& s)
  : SeqRegState(s), plan(s.plan)
{
  // done
}

NTYPE LDSeqRegState::expand(Search& s, NTYPE bound)
{
  NTYPE c_min = POS_INF;
  index_type ra = no_such_index;
  index_type d = depth();
  if (d < plan.length()) {
    ra = plan[plan.length() - (d + 1)];
    Instance::Action& a = instance.actions[ra];
    if (a.sel && applicable(a)) {
      NTYPE act_est = heuristic.eval_precondition(a);
      if (res) {
	if (!res->sufficient_consumable(a.pre, a)) act_est = POS_INF;
      }
      if (FINITE(act_est) && (cost(a.index) + act_est <= bound)) {
	copy_as_ld = true;
	LDSeqRegState* new_s = (LDSeqRegState*)apply(a);
	if (FINITE(new_s->est) && (cost(a.index) + new_s->est <= bound)) {
	  NTYPE c_new = (cost(a.index) +
			 s.new_state(*new_s, bound - cost(a.index)));
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
      else {
	c_min = MIN(c_min, cost(a.index) + act_est);
      }
    }
  }
  std::cerr << "exploring deviations at depth " << d << "..." << std::endl;
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].sel && (k != ra)) {
      Instance::Action& a = instance.actions[k];
      if (a.sel && relevant(a)) {
	bool app = applicable(a);
	if (app) {
	  if ((act != no_such_index) && (act < a.index) &&
	      instance.commutative(act, a.index)) app = false;
	}
	if (app) {
	  NTYPE act_est = heuristic.eval_precondition(a);
	  if (res) {
	    if (!res->sufficient_consumable(a.pre, a)) act_est = POS_INF;
	  }
	  if (FINITE(act_est) && (cost(a.index) + act_est <= bound)) {
	    copy_as_ld = false;
	    SeqCRegState* new_s = (SeqCRegState*)apply(a);
	    copy_as_ld = true;
	    if (FINITE(new_s->est_cost()) &&
		(cost(a.index) + new_s->est_cost() <= bound)) {
	      NTYPE c_new = (cost(a.index) +
			     s.new_state(*new_s, bound - cost(a.index)));
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
	      c_min = MIN(c_min, cost(a.index) + new_s->est_cost());
	    }
	    delete new_s;
	  }
	  else c_min = MIN(c_min, cost(a.index) + act_est);
	}
      }
    }
  return c_min;
}

State* LDSeqRegState::copy()
{
  if (copy_as_ld) {
    return new LDSeqRegState(*this);
  }
  else {
    return new SeqCRegState(*this);
  }
}

int LDSeqRegState::compare(const State& s)
{
  return SeqRegState::compare(s);
}

void LDSeqRegState::write(std::ostream& s)
{
  s << "(" << depth() << ": ";
  if (depth() < plan.length()) {
    s << instance.actions[plan[plan.length() - (depth() + 1)]].name;
  }
  else {
    s << "-";
  }
  s << ") ";
  SeqRegState::write(s);
}

END_HSPS_NAMESPACE
