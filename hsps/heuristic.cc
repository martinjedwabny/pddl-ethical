
#include "heuristic.h"

BEGIN_HSPS_NAMESPACE

count_type Heuristic::eval_count = 0;
int Heuristic::default_trace_level = 1;

Heuristic::~Heuristic()
{
  // done
}

void Heuristic::set_trace_level(int level)
{
  trace_level = level;
}

void Heuristic::write_eval
(const index_set& s, ::std::ostream& st, char* p, bool e)
{
  if (p) st << p << ' ';
  st << eval(s);
  if (e) st << ::std::endl;
}

void Heuristic::write_eval
(const bool_vec& s, ::std::ostream& st, char* p, bool e)
{
  if (p) st << p << ' ';
  st << eval(s);
  if (e) st << ::std::endl;
}

NTYPE Heuristic::eval_precondition(const Instance::Action& a)
{
  return eval(a.pre);
}

NTYPE Heuristic::incremental_eval(const index_set& s, index_type i_new)
{
  index_set s_new(s);
  s_new.insert(i_new);
  return eval(s_new);
}

NTYPE Heuristic::incremental_eval(const bool_vec& s, index_type i_new)
{
  bool_vec s_new(s);
  for (index_type k = 0; k < instance.n_atoms(); k++) s_new[k] = s[k];
  s_new[i_new] = true;
  return eval(s_new);
}

NTYPE Heuristic::eval_to_bound(const index_set& s, NTYPE bound)
{
  return eval(s);
}

NTYPE Heuristic::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  return eval(s);
}

void Heuristic::store(const index_set& s, NTYPE v, bool opt)
{
  // does nothing
}

void Heuristic::store(const bool_vec& s, NTYPE v, bool opt)
{
  // does nothing
}

NTYPE Heuristic::eval(index_type atom)
{
  index_set s;
  s.assign_singleton(atom);
  return eval(s);
}

NTYPE ZeroHeuristic::eval(const index_set& s)
{
  eval_count += 1;
  return 0;
}

NTYPE ZeroHeuristic::eval(const bool_vec& s)
{
  eval_count += 1;
  return 0;
}

EvalActionCache::EvalActionCache(Instance& ins, Heuristic& h)
  : Heuristic(ins), base_h(h), cache(0, instance.n_actions())
{
  for (index_type k = 0; k < instance.n_actions(); k++)
    cache[k] = base_h.eval(instance.actions[k].pre);
}

NTYPE EvalActionCache::eval(const index_set& s)
{
  return base_h.eval(s);
}

NTYPE EvalActionCache::eval(const bool_vec& s)
{
  return base_h.eval(s);
}

NTYPE EvalActionCache::eval_precondition(const Instance::Action& a)
{
  return cache[a.index];
}

NTYPE EvalActionCache::incremental_eval(const index_set& s, index_type i_new)
{
  return base_h.incremental_eval(s, i_new);
}

NTYPE EvalActionCache::incremental_eval(const bool_vec& s, index_type i_new)
{
  return base_h.incremental_eval(s, i_new);
}

NTYPE EvalActionCache::eval_to_bound(const index_set& s, NTYPE bound)
{
  return base_h.eval_to_bound(s, bound);
}

NTYPE EvalActionCache::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  return base_h.eval_to_bound(s, bound);
}

NTYPE RegressionInvariantCheck::eval(const index_set& s)
{
  NTYPE val = base_h.eval(s);
  if (FINITE(val)) {
    for (index_type k = 0; (k < instance.n_invariants()) && FINITE(val); k++)
      if (instance.invariants[k].verified || !verified_invariants_only)
	if (!instance.eval_invariant_in_partial_state(s, instance.invariants[k])) val = POS_INF;
  }
  return val;
}

NTYPE RegressionInvariantCheck::eval(const bool_vec& s)
{
  NTYPE val = base_h.eval(s);
  if (FINITE(val)) {
    for (index_type k = 0; (k < instance.n_invariants()) && FINITE(val); k++)
      if (instance.invariants[k].verified || !verified_invariants_only)
	if (!instance.eval_invariant_in_partial_state(s, instance.invariants[k])) val = POS_INF;
  }
  return val;
}

NTYPE RegressionInvariantCheck::incremental_eval
(const index_set& s, index_type i_new)
{
  NTYPE val = base_h.incremental_eval(s, i_new);
  if (FINITE(val)) {
    index_set s_new(s);
    s_new.insert(i_new);
    for (index_type k = 0; (k < instance.n_invariants()) && FINITE(val); k++)
      if (instance.invariants[k].verified || !verified_invariants_only)
	if (!instance.eval_invariant_in_partial_state(s_new, instance.invariants[k])) val = POS_INF;
  }
  return val;
}

NTYPE RegressionInvariantCheck::incremental_eval
(const bool_vec& s, index_type i_new)
{
  NTYPE val = base_h.incremental_eval(s, i_new);
  if (FINITE(val)) {
    bool_vec s_new(s);
    s_new[i_new] = true;
    for (index_type k = 0; (k < instance.n_invariants()) && FINITE(val); k++)
      if (instance.invariants[k].verified || !verified_invariants_only)
	if (!instance.eval_invariant_in_partial_state(s_new, instance.invariants[k])) val = POS_INF;
  }
  return val;
}

NTYPE RegressionInvariantCheck::eval_to_bound
(const index_set& s, NTYPE bound)
{
  NTYPE val = base_h.eval(s);
  if (val < bound) {
    for (index_type k = 0; (k < instance.n_invariants()) && FINITE(val); k++)
      if (instance.invariants[k].verified || !verified_invariants_only)
	if (!instance.eval_invariant_in_partial_state(s, instance.invariants[k])) val = POS_INF;
  }
  return val;
}

NTYPE RegressionInvariantCheck::eval_to_bound
(const bool_vec& s, NTYPE bound)
{
  NTYPE val = base_h.eval(s);
  if (val < bound) {
    for (index_type k = 0; (k < instance.n_invariants()) && FINITE(val); k++)
      if (instance.invariants[k].verified || !verified_invariants_only)
	if (!instance.eval_invariant_in_partial_state(s, instance.invariants[k])) val = POS_INF;
  }
  return val;
}

ForwardReachabilityCheck::ForwardReachabilityCheck
(Instance& i, const index_set& g)
  : Heuristic(i),
    goals(g),
    r(false, i.n_atoms()),
    f(false, i.n_actions()),
    d(false, i.n_actions())
{
  // done
}

ForwardReachabilityCheck::~ForwardReachabilityCheck()
{
  // done
}

NTYPE ForwardReachabilityCheck::compute()
{
  f.assign_value(false);
  for (index_type k = 0; k < instance.n_atoms(); k++) if (r[k]) {
    for (index_type i = 0; i < instance.atoms[k].req_by.length(); i++)
      f[instance.atoms[k].req_by[i]] = true;
  }
  for (index_type k = 0; k < instance.n_actions(); k++) d[k] = false;
  bool done = false;
  while (!done) {
    done = true;
    for (index_type k = 0; k < instance.n_actions(); k++) if (f[k] && !d[k]) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "action " << instance.actions[k].name
		<< " is flagged and not done"
		<< std::endl;
#endif
      bool app = true;
      for (index_type i = 0; (i < instance.actions[k].pre.length())&&app; i++)
	if (!r[instance.actions[k].pre[i]]) {
#ifdef TRACE_PRINT_LOTS
	  std::cerr << "action " << instance.actions[k].name
		    << " is not applicable because precondition "
		    << instance.atoms[instance.actions[k].pre[i]].name
		    << " not reached"
		    << std::endl;
#endif
	  app = false;
	}
      if (app) {
#ifdef TRACE_PRINT_LOTS
	std::cerr << "action " << instance.actions[k].name
		  << " is applicable"
		  << std::endl;
#endif
	for (index_type i = 0; i < instance.actions[k].add.length(); i++) {
	  index_type a = instance.actions[k].add[i];
	  if (!r[a]) {
#ifdef TRACE_PRINT_LOTS
	    std::cerr << "atom " << instance.atoms[a].name
		      << " reached"
		      << std::endl;
#endif
	    r[a] = true;
	    for (index_type j = 0; j < instance.atoms[a].req_by.length(); j++){
#ifdef TRACE_PRINT_LOTS
	      std::cerr << "flagging action "
			<< instance.actions[instance.atoms[a].req_by[j]].name
			<< std::endl;
#endif
	      f[instance.atoms[a].req_by[j]] = true;
	    }
	    done = false;
	  }
	}
	d[k] = true;
      }
    }
  }
  for (index_type k = 0; k < goals.length(); k++)
    if (!r[goals[k]]) return POS_INF;
  return 0;
}

NTYPE ForwardReachabilityCheck::eval(const index_set& s)
{
  r.assign_value(false);
  for (index_type k = 0; k < s.length(); k++) r[s[k]] = true;
  return compute();
}

NTYPE ForwardReachabilityCheck::eval(const bool_vec& s)
{
  r.assign_copy(s);
  return compute();
}

NTYPE RoundUp::eval(const index_set& s)
{
  NTYPE v = h.eval(s);
  if (FRAC(v) > 0)
    return FLOOR(v) + 1;
  else
    return v;
}

NTYPE RoundUp::eval(const bool_vec& s)
{
  NTYPE v = h.eval(s);
  if (FRAC(v) > 0)
    return FLOOR(v) + 1;
  else
    return v;
}

NTYPE RoundUp::incremental_eval(const index_set& s, index_type i_new)
{
  NTYPE v = h.incremental_eval(s, i_new);
  if (FRAC(v) > 0)
    return FLOOR(v) + 1;
  else
    return v;
}

NTYPE RoundUp::incremental_eval(const bool_vec& s, index_type i_new)
{
  NTYPE v = h.incremental_eval(s, i_new);
  if (FRAC(v) > 0)
    return FLOOR(v) + 1;
  else
    return v;
}

NTYPE RoundUp::eval_to_bound(const index_set& s, NTYPE bound)
{
  NTYPE v = h.eval_to_bound(s, bound);
  if (FRAC(v) > 0)
    return FLOOR(v) + 1;
  else
    return v;
}

NTYPE RoundUp::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  NTYPE v = h.eval_to_bound(s, bound);
  if (FRAC(v) > 0)
    return FLOOR(v) + 1;
  else
    return v;
}

NTYPE Combine2ByMax::eval(const index_set& s)
{
  NTYPE v0 = h0.eval(s);
  NTYPE v1 = h1.eval(s);
  if (trace_level > 1) {
    if (v0 != v1) {
      ::std::cerr << "Combine2ByMax: ";
      instance.write_atom_set(::std::cerr, s);
      ::std::cerr << ", v0 = " << v0 << ", v1 = " << v1 << ::std::endl;
    }
  }
  return MAX(v0, v1);
}

NTYPE Combine2ByMax::eval(const bool_vec& s)
{
  NTYPE v0 = h0.eval(s);
  NTYPE v1 = h1.eval(s);
  if (trace_level > 1) {
    if (v0 != v1) {
      ::std::cerr << "Combine2ByMax: ";
      instance.write_atom_set(::std::cerr, s);
      ::std::cerr << ", v0 = " << v0 << ", v1 = " << v1 << ::std::endl;
    }
  }
  return MAX(v0, v1);
}

NTYPE Combine2ByMax::incremental_eval(const index_set& s, index_type i_new)
{
  NTYPE v0 = h0.incremental_eval(s, i_new);
  NTYPE v1 = h1.incremental_eval(s, i_new);
  return MAX(v0, v1);
}

NTYPE Combine2ByMax::incremental_eval(const bool_vec& s, index_type i_new)
{
  NTYPE v0 = h0.incremental_eval(s, i_new);
  NTYPE v1 = h1.incremental_eval(s, i_new);
  return MAX(v0, v1);
}

NTYPE Combine2ByMax::eval_to_bound(const index_set& s, NTYPE bound)
{
  NTYPE v0 = h0.eval_to_bound(s, bound);
  if (v0 > bound) return v0;
  NTYPE v1 = h1.eval_to_bound(s, bound);
  return MAX(v0, v1);
}

NTYPE Combine2ByMax::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  NTYPE v0 = h0.eval_to_bound(s, bound);
  if (v0 > bound) return v0;
  NTYPE v1 = h1.eval_to_bound(s, bound);
  return MAX(v0, v1);
}

NTYPE CombineNByMax::eval(const index_set& s)
{
  NTYPE v_max = 0;
  for (index_type k = 0; k < h_vec.length(); k++)
    v_max = MAX(v_max, h_vec[k]->eval(s));
  return v_max;
}

NTYPE CombineNByMax::eval(const bool_vec& s)
{
  NTYPE v_max = 0;
  for (index_type k = 0; k < h_vec.length(); k++)
    v_max = MAX(v_max, h_vec[k]->eval(s));
  return v_max;
}

NTYPE CombineNByMax::incremental_eval(const index_set& s, index_type i_new)
{
  NTYPE v_max = 0;
  for (index_type k = 0; k < h_vec.length(); k++)
    v_max = MAX(v_max, h_vec[k]->incremental_eval(s, i_new));
  return v_max;
}

NTYPE CombineNByMax::incremental_eval(const bool_vec& s, index_type i_new)
{
  NTYPE v_max = 0;
  for (index_type k = 0; k < h_vec.length(); k++)
    v_max = MAX(v_max, h_vec[k]->incremental_eval(s, i_new));
  return v_max;
}

NTYPE CombineNByMax::eval_to_bound(const index_set& s, NTYPE bound)
{
  NTYPE v_max = 0;
  for (index_type k = 0; k < h_vec.length(); k++) {
    v_max = MAX(v_max, h_vec[k]->eval(s));
    if (v_max > bound) return v_max;
  }
  return v_max;
}

NTYPE CombineNByMax::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  NTYPE v_max = 0;
  for (index_type k = 0; k < h_vec.length(); k++) {
    v_max = MAX(v_max, h_vec[k]->eval(s));
    if (v_max > bound) return v_max;
  }
  return v_max;
}

NTYPE CombineNBySum::eval(const index_set& s)
{
  NTYPE v_sum = 0;
  for (index_type k = 0; k < h_vec.length(); k++)
    v_sum += h_vec[k]->eval(s);
  return v_sum;
}

NTYPE CombineNBySum::eval(const bool_vec& s)
{
  NTYPE v_sum = 0;
  for (index_type k = 0; k < h_vec.length(); k++)
    v_sum += h_vec[k]->eval(s);
  return v_sum;
}

NTYPE CombineNBySum::incremental_eval(const index_set& s, index_type i_new)
{
  NTYPE v_sum = 0;
  for (index_type k = 0; k < h_vec.length(); k++)
    v_sum += h_vec[k]->incremental_eval(s, i_new);
  return v_sum;
}

NTYPE CombineNBySum::incremental_eval(const bool_vec& s, index_type i_new)
{
  NTYPE v_sum = 0;
  for (index_type k = 0; k < h_vec.length(); k++)
    v_sum += h_vec[k]->incremental_eval(s, i_new);
  return v_sum;
}

NTYPE CombineNBySum::eval_to_bound(const index_set& s, NTYPE bound)
{
  NTYPE v_sum = 0;
  for (index_type k = 0; k < h_vec.length(); k++) {
    v_sum += h_vec[k]->eval(s);
    if (v_sum > bound) return v_sum;
  }
  return v_sum;
}

NTYPE CombineNBySum::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  NTYPE v_sum = 0;
  for (index_type k = 0; k < h_vec.length(); k++) {
    v_sum += h_vec[k]->eval(s);
    if (v_sum > bound) return v_sum;
  }
  return v_sum;
}


NTYPE Combine2ByRandomChoice::eval(const index_set& s)
{
  unsigned long i = rng.random_in_range(alpha.divisor());
  if (i < alpha.numerator())
    return h0.eval(s);
  else
    return h1.eval(s);
}

NTYPE Combine2ByRandomChoice::eval(const bool_vec& s)
{
  unsigned long i = rng.random_in_range(alpha.divisor());
  if (i < alpha.numerator())
    return h0.eval(s);
  else
    return h1.eval(s);
}

NTYPE HX::eval(const index_set& s)
{
  assert(!X.empty());
  index_type c = s.first_common_element(X);
  if (c == no_such_index) {
    bool_vec sx(s, instance.n_atoms());
    NTYPE v = POS_INF;
    for (index_type k = 0; k < X.length(); k++) {
      assert(!sx[X[k]]);
      sx[X[k]] = true;
      v = MIN(v, h0.eval(sx));
      sx[X[k]] = false;
    }
    return v;
  }
  else {
    return h0.eval(s);
  }
}

NTYPE HX::eval(const bool_vec& s)
{
  assert(!X.empty());
  index_type c = X.first_common_element(s);
  if (c == no_such_index) {
    bool_vec sx(s);
    NTYPE v = POS_INF;
    for (index_type k = 0; k < X.length(); k++) {
      assert(!sx[X[k]]);
      sx[X[k]] = true;
      v = MIN(v, h0.eval(sx));
      sx[X[k]] = false;
    }
    return v;
  }
  else {
    return h0.eval(s);
  }
}

NTYPE HX::incremental_eval(const index_set& s, index_type i_new)
{
  assert(!X.empty());
  if (X.contains(i_new)) {
    return h0.incremental_eval(s, i_new);
  }
  else {
    index_type c = s.first_common_element(X);
    if (c == no_such_index) {
      bool_vec sx(s, instance.n_atoms());
      sx[i_new] = true;
      NTYPE v = POS_INF;
      for (index_type k = 0; k < X.length(); k++) {
	assert(!sx[X[k]]);
	sx[X[k]] = true;
	v = MIN(v, h0.eval(sx));
	sx[X[k]] = false;
      }
      return v;
    }
    else {
      return h0.incremental_eval(s, i_new);
    }
  }
}

NTYPE HX::incremental_eval(const bool_vec& s, index_type i_new)
{
  if (X.contains(i_new)) {
    return h0.incremental_eval(s, i_new);
  }
  else {
    assert(!X.empty());
    index_type c = X.first_common_element(s);
    if (c == no_such_index) {
      bool_vec sx(s);
      bool i_new_set = sx[i_new];
      sx[i_new] = true;
      NTYPE v = POS_INF;
      for (index_type k = 0; k < X.length(); k++) {
	assert(!sx[X[k]]);
	sx[X[k]] = true;
	v = MIN(v, h0.eval(sx));
	sx[X[k]] = false;
      }
      sx[i_new] = i_new_set;
      return v;
    }
    else {
      return h0.incremental_eval(s, i_new);
    }
  }
}

void HX::write_eval(const index_set& s, std::ostream& st, char* p, bool e)
{
  assert(!X.empty());
  if (p) st << p << ' ';
  index_type c = s.first_common_element(X);
  if (c == no_such_index) {
    st << "max {";
    bool_vec sx(s, instance.n_atoms());
    NTYPE v = POS_INF;
    for (index_type k = 0; k < X.length(); k++) {
      assert(!sx[X[k]]);
      if (k > 0) st << ", ";
      st << "X=" << instance.atoms[X[k]].name << ": ";
      sx[X[k]] = true;
      h0.write_eval(s, st, 0, false);
      v = MIN(v, h0.eval(sx));
      sx[X[k]] = false;
    }
    st << "} = " << v;
  }
  else {
    st << "X=" << instance.atoms[c].name << ": ";
    h0.write_eval(s, st, 0, false);
  }
  if (e) st << ::std::endl;
}

void HX::write_eval(const bool_vec& s, std::ostream& st, char* p, bool e)
{
  assert(!X.empty());
  if (p) st << p << ' ';
  index_type c = X.first_common_element(s);
  if (c == no_such_index) {
    st << "max {";
    bool_vec sx(s);
    NTYPE v = POS_INF;
    for (index_type k = 0; k < X.length(); k++) {
      assert(!sx[X[k]]);
      if (k > 0) st << ", ";
      st << "X=" << instance.atoms[X[k]].name << ": ";
      sx[X[k]] = true;
      h0.write_eval(s, st, 0, false);
      v = MIN(v, h0.eval(sx));
      sx[X[k]] = false;
    }
    st << "} = " << v;
  }
  else {
    st << "X=" << instance.atoms[c].name << ": ";
    h0.write_eval(s, st, 0, false);
  }
  if (e) st << ::std::endl;
}

NTYPE AtomMapAdapter::eval(const index_set& s)
{
  index_set xs;
  for (index_type k = 0; k < s.length(); k++)
    if (map[s[k]] != no_such_index) xs.insert(map[s[k]]);
  return base_h.eval(xs);
}

NTYPE AtomMapAdapter::eval(const bool_vec& s)
{
  index_set xs;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (s[k] && (map[k] != no_such_index)) xs.insert(map[k]);
  return base_h.eval(xs);
}

NTYPE AtomMapAdapter::incremental_eval(const index_set& s, index_type i_new)
{
  index_set xs;
  for (index_type k = 0; k < s.length(); k++)
    if (map[s[k]] != no_such_index) xs.insert(map[s[k]]);
  if (map[i_new] != no_such_index) 
    return base_h.incremental_eval(xs, map[i_new]);
  else
    return base_h.eval(xs);
}

NTYPE AtomMapAdapter::incremental_eval(const bool_vec& s, index_type i_new)
{
  index_set xs;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (s[k] && (map[k] != no_such_index)) xs.insert(map[k]);
  if (map[i_new] != no_such_index) 
    return base_h.incremental_eval(xs, map[i_new]);
  else
    return base_h.eval(xs);
}

count_type CompareEval::lower = 0;
count_type CompareEval::equal = 0;
count_type CompareEval::higher = 0;

NTYPE CompareEval::eval(const index_set& s)
{
  NTYPE base_val = base_h.eval(s);
  NTYPE alt_val = alt_h.eval(s);
  if (alt_val < base_val) {
    lower += 1;
  }
  else if (alt_val > base_val) {
    higher += 1;
  }
  else {
    equal += 1;
  }
#ifdef PRINT_COMPARE_EVAL
  ::std::cout << "COMPARE ";
  instance.write_atom_set(::std::cout, s);
  ::std::cout << " : " << base_val << " / " << alt_val << ::std::endl;
#endif
  if ((trace_level > 0) && (alt_val != base_val)) {
    base_h.set_trace_level(trace_level);
    ::std::cerr << "evaluating base heuristic..." << ::std::endl;
    base_h.eval(s);
    base_h.set_trace_level(0);
    alt_h.set_trace_level(trace_level);
    ::std::cerr << "evaluating alternative heuristic..." << ::std::endl;
    alt_h.eval(s);
    alt_h.set_trace_level(0);
  }
  if (max_h_val) {
    return MAX(base_val, alt_val);
  }
  else {
    return base_val;
  }
}

NTYPE CompareEval::eval(const bool_vec& s)
{
  NTYPE base_val = base_h.eval(s);
  NTYPE alt_val = alt_h.eval(s);
  if (alt_val < base_val) {
    lower += 1;
  }
  else if (alt_val > base_val) {
    higher += 1;
  }
  else {
    equal += 1;
  }
#ifdef PRINT_COMPARE_EVAL
  ::std::cout << "COMPARE ";
  instance.write_atom_set(::std::cout, s);
  ::std::cout << " : " << base_val << " / " << alt_val << ::std::endl;
#endif
  if ((trace_level > 0) && (alt_val != base_val)) {
    base_h.set_trace_level(trace_level);
    ::std::cerr << "evaluating base heuristic..." << ::std::endl;
    base_h.eval(s);
    base_h.set_trace_level(0);
    alt_h.set_trace_level(trace_level);
    ::std::cerr << "evaluating alternative heuristic..." << ::std::endl;
    alt_h.eval(s);
    alt_h.set_trace_level(0);
  }
  if (max_h_val) {
    return MAX(base_val, alt_val);
  }
  else {
    return base_val;
  }
}

CompleteNegationAdapter::CompleteNegationAdapter
(Instance& ins, const pair_vec& p, Heuristic& h)
  : Heuristic(ins), h_base(h), pn_map(p), sc(false, p.length() * 2)
{
  // done
}

CompleteNegationAdapter::~CompleteNegationAdapter()
{
  // done
}

NTYPE CompleteNegationAdapter::eval(const index_set& s)
{
  bool_vec sv(s, instance.n_atoms());
  return eval(sv);
}

NTYPE CompleteNegationAdapter::eval(const bool_vec& s)
{
  for (index_type k = 0; k < pn_map.length(); k++) {
    sc[pn_map[k].first] = s[pn_map[k].first];
    sc[pn_map[k].second] = !s[pn_map[k].first];
  }
  return h_base.eval(sc);
}

NTYPE ACF::min_cost(index_type n) const
{
  NTYPE c_min = POS_INF;
  for (index_type k = 0; k < n; k++)
    c_min = MIN(c_min, (*this)(k));
  return c_min;
}

NTYPE ACF::max_cost(index_type n) const
{
  NTYPE c_max = NEG_INF;
  for (index_type k = 0; k < n; k++)
    c_max = MAX(c_max, (*this)(k));
  return c_max;
}

NTYPE ACF::avg_cost(index_type n) const
{
  // std::cerr << "computing average cost..." << std::endl;
  NTYPE c_sum = 0;
  for (index_type k = 0; k < n; k++) {
    NTYPE c_k = (*this)(k);
    // std::cerr << "cost(" << k << ") = " << c_k << std::endl;
    c_sum += c_k;
  }
  // std::cerr << "returning " << c_sum << " / " << n << " = "
  //	    << (c_sum / n) << std::endl;
  return c_sum / n;
}

NTYPE UnitACF::operator()(index_type a) const
{
  return 1;
}

NTYPE UnitACF::min_cost(index_type n) const
{
  return 1;
}

NTYPE UnitACF::max_cost(index_type n) const
{
  return 1;
}

NTYPE UnitACF::avg_cost(index_type n) const
{
  return 1;
}

NTYPE ZeroACF::operator()(index_type a) const
{
  return 0;
}

NTYPE ZeroACF::min_cost(index_type n) const
{
  return 0;
}

NTYPE ZeroACF::max_cost(index_type n) const
{
  return 0;
}

NTYPE ZeroACF::avg_cost(index_type n) const
{
  return 0;
}

NTYPE CostACF::operator()(index_type a) const
{
  return instance.actions[a].cost;
}

FracACF::FracACF(const ACF& b, index_type l)
  : baseACF(b), df(1, l)
{
  // done
}

FracACF::FracACF(const ACF& b, index_type l, NTYPE f)
  : baseACF(b), df(f, l)
{
  // done
}

FracACF::~FracACF()
{
  // done
}

void FracACF::set(index_type a, NTYPE f)
{
  assert(a < df.length());
  df[a] = f;
}

void FracACF::set(const index_set& d, NTYPE f)
{
  for (index_type k = 0; k < d.length(); k++) {
    assert(d[k] < df.length());
    df[d[k]] = f;
  }
}

NTYPE FracACF::operator()(index_type a) const
{
  assert(a < df.length());
  return (baseACF(a) * df[a]);
}

NTYPE DiscountACF::operator()(index_type a) const
{
  if (discounted[a]) return 0;
  else return baseACF(a);
}

NTYPE MakespanACF::operator()(index_type a) const
{
  return instance.actions[a].dur;
}

ResourceConsACF::ResourceConsACF(Instance& i, index_type r)
  : instance(i), resource_id(r)
{
  assert(r < instance.n_resources());
}

NTYPE ResourceConsACF::operator()(index_type a) const
{
  return instance.actions[a].cons[resource_id];
}

ResourceReqACF::ResourceReqACF(Instance& i, index_type r)
  : instance(i), resource_id(r)
{
  assert(r < instance.n_resources());
}

NTYPE ResourceReqACF::operator()(index_type a) const
{
  return instance.actions[a].req(resource_id);
}

END_HSPS_NAMESPACE
