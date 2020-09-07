
#include "pdb.h"
#include "enumerators.h"
#include "idao.h"
#include "bb.h"
// #include "pattern_collection.h"

BEGIN_HSPS_NAMESPACE

#ifdef _DO_NOT_COMPILE_

void test_new_pattern_collection_class
(const SASInstance &sas_instance,
 const abstraction_heuristic_vec &abst_h_vec);

#endif

// #define TRACE_PRINT_LOTS
// #define CHECK_MDD_EVAL

#define ASH_SOLVE_USE_BFS
// #define ASH_SOLVE_USE_IDA
#define ASH_USE_EQC

// #define USE_MEAN_VALUE
#define MEAN_VALUE_INFINITY I_TO_N(0)  // ignore infinite values
// #define MEAN_VALUE_INFINITY POS_INF // let them dominate
///#define MEAN_VALUE_INFINITY I_TO_N(1000)  // use "a very large constant"


// Note: when USE_SAMPLED_MEAN_VALUE, USE_WEIGHTED_SCORE must also be
// enabled and USE_CI_PRUNING must be disabled!

// #define USE_SAMPLED_MEAN_VALUE
// #define USE_WEIGHTED_SCORE
#define USE_CI_PRUNING

// #define USE_LARGER_EXTENSIONS

StateAbstraction::StateAbstraction
(SASInstance& ins, const index_set& vars)
  : instance(ins),
    variable_set(vars),
    reduce_map(no_such_index, ins.n_variables())
{
  for (index_type k = 0; k < variable_set.length(); k++) {
    assert(variable_set[k] < instance.n_variables());
    reduce_map[variable_set[k]] = k;
  }
}

StateAbstraction::~StateAbstraction()
{
  // done
}

index_type StateAbstraction::size() const
{
  CompleteStatePerfectHashFunction h(instance, variable_set);
  if (h.overflow()) {
    return no_such_index;
  }
  else {
    return h.n_values();
  }
}

bool StateAbstraction::additive(const index_set& v) const
{
  if (variable_set.count_common(v) > 0) return false;
  for (index_type i = 0; i < variable_set.length(); i++)
    for (index_type j = 0; j < v.length(); j++)
      if (instance.interference_graph.adjacent(variable_set[i], v[j]))
	return false;
  return true;
}

partial_state StateAbstraction::apply(const partial_state& s) const
{
  partial_state a(s, reduce_map);
  return a;
}

bool StateAbstraction::equals
(const partial_state& s0, const partial_state& s1) const
{
  return (s0.equals_restricted(s1, variables()));
}

bool StateAbstraction::implies
(const partial_state& s, const partial_state& c) const
{
  return s.implies_restricted(c, variables());
}

bool StateAbstraction::consistent
(const partial_state& s0, const partial_state& s1) const
{
  partial_state a0(s0, reduce_map);
  partial_state a1(s1, reduce_map);
  a0.insert(a1);
  return a0.consistent();
}

void StateAbstraction::assign
(partial_state& s, const partial_state& c) const
{
  s.assign_restricted(c, variables());
}

bool StateAbstraction::consistent
(const partial_state& s, MDDNode* sinc, Heuristic* inc) const
{
  assert(sinc);
  partial_state s0(s, variables());
#ifdef CHECK_MDD_EVAL
  assert(inc);
  index_set a0;
  instance.make_atom_set(s0, a0);
  assert(((bool)INFINITE(inc->eval(a0))) == sinc->lookup(s0));
#endif
  return !(sinc->lookup(s0));
}

count_type StateAbstraction::n_walks = 0;
count_type StateAbstraction::n_cut_walks = 0;

index_type StateAbstraction::random_walk
(partial_state& s, index_type l, MDDNode* sinc, Heuristic* inc, RNG& rng)
{
  n_walks += 1;
  partial_state s0(s, variables());
  partial_state_vec history;
  partial_state_vec succ;
  partial_state s1;
  index_type n = 0;
  while (l > 0) {
    // std::cerr << "l = " << l << ", s0 = " << s0 << std::endl;
    succ.clear();
    for (index_type k = 0; k < instance.n_actions(); k++)
      if (instance.actions[k].post.defines_any(variables()) &&
	  implies(s0, instance.actions[k].pre) &&
	  implies(s0, instance.actions[k].prv)) {
	s1.assign_copy(s0);
	if (sinc) {
	  s1.assign(instance.actions[k].post);
#ifdef CHECK_MDD_EVAL
	  assert(inc);
	  index_set a1;
	  instance.make_atom_set(s1, a1);
	  assert(((bool)INFINITE(inc->eval(a1))) == sinc->lookup(s1));
#endif
	  if (!sinc->lookup(s1)) {
	    s1.restrict_to(variables());
	    index_type i = succ.first(s1);
	    if (i == no_such_index)
	      succ.append(s1);
	  }
	}
	else {
	  s1.assign_restricted(instance.actions[k].post, variables());
	  index_type i = succ.first(s1);
	  if (i == no_such_index)
	    succ.append(s1);
	}
      }
    index_type b = succ.length();
    if (history.length() > 0) b += 1;
    // std::cerr << "|succ| = " << succ.length() << " |history| = " << history.length() << std::endl;
    if (b == 0) {
      s.assign(s0);
      n_cut_walks += 1;
      assert(s0 == s);
      return n;
    }
    index_type i = rng.random_in_range(b + 1);
    // std::cerr << "i = " << i << std::endl;
    if (i < succ.length()) {
      history.append(s0);
      s0 = succ[i];
    }
    else if ((i == succ.length()) && (history.length() > 0)) {
      s0 = history[history.length() - 1];
      history.dec_length();
    }
    l -= 1;
    n += 1;
  }
  s = s0;
  return n;
}

index_type StateAbstraction::walk
(partial_state& s, const index_vec w, MDDNode* sinc, Heuristic* inc)
{
  partial_state s0(s, variables());
  index_type l = 0;
  std::cerr << " - abstract initial state: ";
  instance.write_partial_state(std::cerr, s0);
  std::cerr << std::endl;
  while (l < w.length()) {
    assert(w[l] < instance.n_actions());
    SASInstance::Action& act = instance.actions[w[l]];
    std::cerr << l << "th action is " << act.name << "..." << std::endl;
    if (implies(s0, act.pre) &&	implies(s0, act.prv)) {
      std::cerr << " - abstract pre- and prevail-conditions ok" << std::endl;
      partial_state s1(s0);
      assign(s1, act.post);
      std::cerr << " - abstract successor state: ";
      instance.write_partial_state(std::cerr, s1);
      std::cerr << std::endl;
      if (sinc) {
#ifdef CHECK_MDD_EVAL
	assert(inc);
	index_set a1;
	instance.make_atom_set(s1, a1);
	assert(((bool)INFINITE(inc->eval(a1))) == sinc->lookup(s1));
#endif
	if (sinc->lookup(s1)) {
	  std::cerr << "walk failed: abstract successor state inconsistent!"
		    << std::endl;
	  return l;
	}
      }
      s0 = s1;
      l += 1;
    }
    else {
      std::cerr << "walk failed: pre-/prevail-conds not satisfied!"
		<< std::endl;
      return l;
    }
  }
  std::cerr << "walk complete, abstract final state: ";
  instance.write_partial_state(std::cerr, s0);
  std::cerr << std::endl;
  s = s0;
  return l;
}

void StateAbstraction::build_explicit_graph
(MDDNode* sinc, index_set*& al_g, index_type& n_g)
{
  CompleteStatePerfectHashFunction h(instance, variables());
  assert(!h.overflow());
  n_g = h.n_values();
  al_g = new index_set[n_g];
  partial_state s0;
  partial_state s1;
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].post.defines_any(variables())) {
      partial_state s_pre(instance.actions[k].pre);
      s_pre.assign(instance.actions[k].prv);
      PartialStateEnumerator e_pre(variables(), instance.signature, s_pre);
      bool more = e_pre.first();
      while (more) {
	bool app = true;
	if (sinc) {
	  s0.assign_copy(e_pre.current_state());
	  s0.assign(s_pre);
	  if (sinc->lookup(s0)) app = false;
	}
	if (app) {
	  index_type i0 = h.index(e_pre.current_state());
	  s1.assign_copy(e_pre.current_state());
	  s1.assign_restricted(instance.actions[k].post, variables());
	  index_type i1 = h.index(s1);
	  al_g[i0].insert(i1);
	}
	more = e_pre.next();
      }
    }
}

index_type StateAbstraction::random_walk
(partial_state& s, index_type l, index_set* al_g, index_type n_g, RNG& rng)
{
  CompleteStatePerfectHashFunction h(instance, variables());
  assert(!h.overflow());
  n_walks += 1;
  index_type s0 = h.index(s);
  index_vec history(no_such_index, l);
  history.set_length(0);
  index_type n = 0;
  while (l > 0) {
    index_type b = al_g[s0].length();
    if (history.length() > 0) b += 1;
    if (b == 0) {
      h.state(s0, s);
      n_cut_walks += 1;
      return n;
    }
    index_type i = rng.random_in_range(b + 1);
    if (i < al_g[s0].length()) {
      history.append(s0);
      s0 = al_g[s0][i];
    }
    else if ((i == al_g[s0].length()) && (history.length() > 0)) {
      s0 = history[history.length() - 1];
      history.dec_length();
    }
    l -= 1;
    n += 1;
  }
  h.state(s0, s);
  return n;
}

void StateAbstraction::write_graph
(std::ostream& s, const partial_state& g, MDDNode* sinc, Heuristic* inc)
{
  CompleteStatePerfectHashFunction h(instance, variable_set);
  if (h.overflow()) {
    std::cerr << "error in StateAbstraction::write_graph:"
	      << " abstract state space over ";
    instance.write_variable_set(std::cerr, variable_set);
    std::cerr << " is too large"
	      << std::endl;
    exit(255);
  }

  s << "digraph ABSTRACT_STATE_SPACE {" << std::endl;
  s << "rankdir=LR;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;

  PartialStateEnumerator se(variable_set, instance.signature);
  bool more = se.first();
  while (more) {
    if ((sinc == 0) || consistent(se.current_state(), sinc, inc)) {
      s << "S" << h.index(se.current_state())
	<< " [shape=box,label=\"#" << h.index(se.current_state()) << ": ";
      instance.write_partial_state(s, se.current_state());
      s << "\"";
      if (implies(se.current_state(), g)) {
	s << ",style=bold";
      }
      s << "];" << std::endl;
    }
    more = se.next();
  }

  more = se.first();
  while (more) {
    if ((sinc == 0) || consistent(se.current_state(), sinc, inc)) {
      for (index_type k = 0; k < instance.n_actions(); k++) {
	SASInstance::Action& act = instance.actions[k];
	if (act.post.defines_any(variable_set) &&
	    implies(se.current_state(), act.pre) &&
	    implies(se.current_state(), act.prv)) {
	  partial_state res(se.current_state());
	  assign(res, act.post);
	  if ((sinc == 0) || consistent(res, sinc, inc)) {
	    index_type i_res = h.index(res);
	    s << "S" << h.index(se.current_state())
	      << " -> S" << h.index(res)
	      << " [label=\"" << act.name
	      << "\"";
	    s << "];" << std::endl;
	  }
	}
      }
    }
    more = se.next();
  }

  s << "}" << std::endl;
}

NTYPE AbstractionHeuristic::max()
{
  NTYPE v_max = 0;
  PartialStateEnumerator se(variables(), instance.signature);
  bool more = se.first();
  while (more) {
    v_max = MAX(eval(se.current_state()), v_max);
    more = se.next();
  }
  return v_max;
}

NTYPE AbstractionHeuristic::mean()
{
  NTYPE v_sum = 0;
  index_type n = 0;
  PartialStateEnumerator se(variables(), instance.signature);
  bool more = se.first();
  while (more) {
    NTYPE v = eval(se.current_state());
    if (INFINITE(v)) {
      v_sum += MEAN_VALUE_INFINITY;
    }
    else {
      v_sum += v;
    }
    n += 1;
    more = se.next();
  }
  NTYPE v_mean = (v_sum / n);
  return v_mean;
}

index_pair AbstractionHeuristic::compare(SASHeuristic& h)
{
  index_type n_less = 0;
  index_type n_greater = 0;
  PartialStateEnumerator se(variables(), instance.signature);
  bool more = se.first();
  while (more) {
    NTYPE v_this = eval(se.current_state());
    NTYPE v_h = h.eval(se.current_state());
    if (v_h < v_this) {
      if (trace_level > 2) {
	instance.write_partial_state(std::cerr, se.current_state());
	std::cerr << ": " << v_this << " > " << v_h << std::endl;
      }
      n_greater += 1;
    }
    else if (v_h > v_this) {
      if (trace_level > 2) {
	instance.write_partial_state(std::cerr, se.current_state());
	std::cerr << ": " << v_this << " < " << v_h << std::endl;
      }
      n_less += 1;
    }
    more = se.next();
  }
  return index_pair(n_less, n_greater);
}

void AbstractionHeuristic::write_graph
(std::ostream& s, const partial_state& g, const index_set& e,
 bool mark_increasing, bool mark_decreasing)
{
  CompleteStatePerfectHashFunction h(instance, variable_set);
  if (h.overflow()) {
    std::cerr << "error in AbstractionHeuristic::write_graph:"
	      << " abstract state space over ";
    instance.write_variable_set(std::cerr, variable_set);
    std::cerr << " is too large"
	      << std::endl;
    exit(255);
  }
  s << "digraph ABSTRACT_STATE_SPACE {" << std::endl;
  s << "rankdir=LR;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;

  PartialStateEnumerator se(variable_set, instance.signature);
  NTYPE c_max = 0;
  bool more = se.first();
  while (more) {
    NTYPE v = eval(se.current_state());
    if (FINITE(v)) c_max = MAX(c_max, v);
    more = se.next();
  }

  NTYPE l = 0;
  while (l <= c_max) {
    s << "{ rank = same;" << std::endl;
    s << "C" << l << " [shape=plaintext,label=\"" << l << "\"];" << std::endl;
    more = se.first();
    while (more) {
      NTYPE v = eval(se.current_state());
      if (((l - 1) < v) && (v <= l)) {
	bool inc_state = false;
	if (inc) {
	  index_set atms;
	  instance.map_to_atom_set(se.current_state(), atms);
	  if (INFINITE(inc->eval(atms))) inc_state = true;
	}
	s << "S" << h.index(se.current_state())
	  << " [shape=box,label=\"";
	instance.write_partial_state(s, se.current_state());
	s << "\"";
	if (inc_state) {
	  s << ",style=filled";
	}
	if (implies(se.current_state(), g)) {
	  s << ",style=bold";
	}
	s << "];" << std::endl;
      }
      more = se.next();
    }
    s << "}" << std::endl;
    l += 1;
  }

  s << "{ rank = same;" << std::endl;
  s << "CINF [shape=plaintext,label=\"INF\"];" << std::endl;
  more = se.first();
  while (more) {
    NTYPE v = eval(se.current_state());
    if (INFINITE(v)) {
      bool inc_state = false;
      if (inc) {
	index_set atms;
	instance.map_to_atom_set(se.current_state(), atms);
	if (INFINITE(inc->eval(atms))) inc_state = true;
      }
      s << "S" << h.index(se.current_state())
	<< " [shape=box,label=\"";
      instance.write_partial_state(s, se.current_state());
      s << "\"";
      if (inc_state) {
	s << ",style=filled";
      }
      if (implies(se.current_state(), g)) {
	s << ",style=bold";
      }
      s << "];" << std::endl;
    }
    more = se.next();
  }
  s << "}" << std::endl;

  l = 1;
  while (l <= c_max) {
    s << "C" << l - 1 << " -> C" << l << " [style=invis];" << std::endl;
    l += 1;
  }
  s << "C" << l - 1 << " -> CINF [style=invis];" << std::endl;

  more = se.first();
  while (more) {
    for (index_type k = 0; k < instance.n_actions(); k++) {
      SASInstance::Action& act = instance.actions[k];
      if (act.post.defines_any(variable_set) &&
	  implies(se.current_state(), act.pre) &&
	  implies(se.current_state(), act.prv)) {
	bool inc_trans = false;
	if (inc) {
	  index_set atms;
	  instance.map_to_atom_set(se.current_state(), atms);
	  instance.map_to_atom_set(act.pre, atms);
	  instance.map_to_atom_set(act.prv, atms);
	  if (INFINITE(inc->eval(atms))) inc_trans = true;
	}
	partial_state res(se.current_state());
	assign(res, act.post);
	index_type i_res = h.index(res);
	s << "S" << h.index(se.current_state())
	  << " -> S" << h.index(res)
	  << " [label=\"" << act.name
	  << ":" << cost(act.index)
	  << "\"";
	if (inc_trans) {
	  s << ",style=dashed";
	}
	else if (mark_increasing) {
	  if (eval(res) == (eval(se.current_state()) + cost(act.index)))
	    s << ",style=bold";
	}
	else if (mark_decreasing) {
	  if (eval(res) == (eval(se.current_state()) - cost(act.index)))
	    s << ",style=bold";
	}
	else {
	  if (e.contains(act.index))
	    s << ",style=bold";
	}
	s << "];" << std::endl;
      }
    }
    more = se.next();
  }

  s << "}" << std::endl;
}

void AbstractionHeuristic::write(std::ostream& s)
{
  PartialStateEnumerator se(variable_set, instance.signature);
  bool more = se.first();
  while (more) {
    s << "H(";
    instance.write_partial_state(s, se.current_state());
    s << ") = " << eval(se.current_state())
      << std::endl;
    more = se.next();
  }
}

RegressionPDB::RegressionPDB
(SASInstance& ins,
 const index_set& v,
 const ACF& c,
 MDDNode* sinc,
 Heuristic* inc,
 Statistics& s)
  : AbstractionHeuristic(ins, v, c, sinc, inc),
    h(ins, v),
    val(0),
    trace(0),
    stats(s)
{
  assert(!h.overflow());
  val = new NTYPE[h.n_values() + 2];
}

RegressionPDB::~RegressionPDB()
{
  delete [] val;
  if (trace) delete [] trace;
}

index_type RegressionPDBSize::apply
(const index_vec& sig, const index_set& set)
{
  RegressionPDBSize f(sig);
  return f(set);
}

index_type RegressionPDBSize::operator()(const index_set& set) const
{
  index_type s = 1;
  for (index_type k = 0; k < set.length(); k++) {
    if ((no_such_index / (signature[set[k]] + 1)) < s) return no_such_index;
    s = (s * (signature[set[k]] + 1));
  }
  return s;
}

index_cost_pair RegressionPDB::partial_state_val
(partial_state& p)
{
  index_type v_undef = p.first_undefined_variable(variable_set);
  if (v_undef == no_such_index) {
    return index_cost_pair(h.index(p), val[h.index(p)]);
  }
  else {
    index_cost_pair min = index_cost_pair(no_such_index, POS_INF);
    for (index_type k = 0; k < instance.variables[v_undef].n_values(); k++) {
      p.assign(v_undef, k);
      index_cost_pair v_k = partial_state_val(p);
      if (v_k.second < min.second) {
	min = v_k;
      }
    }
    p.undefine(v_undef);
    return min;
  }
}

NTYPE RegressionPDB::compute_partial_states
(partial_state& p, index_set& undef_set)
{
  index_type v_undef = p.first_undefined_variable(undef_set);
  if (v_undef == no_such_index) {
    if (p.first_undefined_variable(variable_set) == no_such_index) {
      return val[h.index(p)];
    }
    else {
      index_cost_pair min_val = partial_state_val(p);
      val[h.index(p)] = min_val.second;
      if (trace) {
	trace[h.index(p)].insert(index_pair(min_val.first, no_such_index));
      }
      return min_val.second;
    }
  }
  else {
    undef_set.subtract(v_undef);
    NTYPE min_val = POS_INF;
    for (index_type k = 0; k < instance.variables[v_undef].n_values(); k++) {
      p.assign(v_undef, k);
      NTYPE k_val = compute_partial_states(p, undef_set);
      min_val = MIN(min_val, k_val);
      if (stats.break_signal_raised()) return 0;
    }
    p.undefine(v_undef);
    compute_partial_states(p, undef_set);
    undef_set.insert(v_undef);
    return min_val;
  }
}

class action_cost_increasing : public index_vec::order {
  const ACF& cost;
 public:
  action_cost_increasing(const ACF& c) : cost(c) { };
  virtual bool operator()(const index_type& a0, const index_type& a1) const;
};

bool action_cost_increasing::operator()
(const index_type& a0, const index_type& a1) const
{
  return (cost(a0) < cost(a1));
}

void RegressionPDB::select_and_sort_relevant_actions(index_vec& acts)
{
  action_cost_increasing order_op(cost);
  acts.clear();
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].post.defines_any(variable_set))
      acts.insert_ordered(k, order_op);
}

void RegressionPDB::compute()
{
  stats.start();
  // init DB: set initial state to 0 and everything else to +INF
  for (index_type k = 0; k < h.n_values(); k++) val[k] = POS_INF;
  partial_state s0(instance.init_state, variable_set);
  val[h.index(s0)] = 0;

  if (trace) {
    for (index_type k = 0; k < h.n_values(); k++)
      trace[k].clear();
  }

  // compute list of relevant actions, sorted by increasing cost
  index_vec acts(no_such_index, 0);
  select_and_sort_relevant_actions(acts);

  // init the open list
  OpenQueue open;
  open.push(OpenState(s0, 0));

  // main forward exploration loop
  while (!open.empty() && !stats.break_signal_raised()) {
    for (index_type k = 0; k < acts.length(); k++) {
      SASInstance::Action& act = instance.actions[acts[k]];
      if (implies(open.top().state, act.pre) &&
	  implies(open.top().state, act.prv)) {
	NTYPE c_pre = open.top().cost;
	if (sinc) {
	  partial_state pre(open.top().state);
	  pre.assign(act.pre);
	  pre.assign(act.prv);
#ifdef CHECK_MDD_EVAL
	  assert(inc);
	  index_set atms;
	  instance.map_to_atom_set(open.top().state, atms);
	  instance.map_to_atom_set(act.pre, atms);
	  instance.map_to_atom_set(act.prv, atms);
	  assert(((bool)INFINITE(inc->eval(atms))) == sinc->lookup(pre));
#endif
	  if (sinc->lookup(pre))
	    c_pre = POS_INF;
	}
	if (FINITE(c_pre)) {
	  partial_state res(open.top().state);
	  assign(res, act.post);
	  NTYPE c_res = c_pre + cost(acts[k]);
	  index_type i_res = h.index(res);
	  if (c_res < val[i_res]) {
	    val[i_res] = c_res;
	    if (trace) {
	      trace[i_res].assign_singleton
		(index_pair(h.index(open.top().state), acts[k]));
	    }
	    open.push(OpenState(res, c_res));
	  }
	  else if ((c_res == val[i_res]) && trace) {
	    trace[i_res].insert
	      (index_pair(h.index(open.top().state), acts[k]));
	  }
	}
      }
    }
    open.pop();
  }

  // fill in values of partial states (forward exploration covers only
  // complete (w.r.t. variable_set) states)
  if (!stats.break_signal_raised()) {
    partial_state u;
    index_set     u_set(variable_set);
    NTYPE v = compute_partial_states(u, u_set);
  }
  stats.stop();
}

NTYPE RegressionPDB::eval(const partial_state& s)
{
  if (!s.consistent()) return POS_INF;
#ifdef ENABLE_HIGHER_TRACE_LEVEL
  if ((trace_level > 2) && (trace != 0)) {
    write_eval(s, std::cerr, "RPDB:", true);
  }
#endif
  return val[h.index(s)];
}

void RegressionPDB::enable_trace()
{
  if (trace == 0) {
    trace = new pair_set[h.n_values() + 2];
  }
}

void RegressionPDB::check_trace()
{
  assert(trace);
  ActionSequenceSet p0;
  ActionSequenceSet p1;
  for (index_type i = 0; i < h.n_values(); i++) {
    partial_state s;
    h.state(i, s);
    p0.clear();
    p1.clear();
    abstract_plan(s, p0, no_such_index);
    abstract_plan_from_trace(s, p1, no_such_index);
  }
}

void RegressionPDB::write_eval
(const partial_state& s, std::ostream& t, char* p, bool e)
{
  if (p) t << p << " ";
//   if (trace) {
//     index_type i = h.index(s);
//     t << "[" << i << "] = " << val[i];
//     while (trace[i].first != no_such_index) {
//       t << " <- ";
//       if (trace[i].second != no_such_index) {
// 	t << instance.actions[trace[i].second].name << " - ";
//       }
//       t << "[" << trace[i].first << "] = " << val[trace[i].first];
//       i = trace[i].first;
//     }
//   }
//   else {
    index_type i = h.index(s);
    t << "[" << i << "] = " << val[i];
//   }
  if (e) t << std::endl;
}

void RegressionPDB::write(std::ostream& s)
{
  for (index_type i = 0; i < h.n_values(); i++) {
    partial_state si;
    h.state(i, si);
    s << "H(";
    instance.write_partial_state(s, si);
    s << ") = " << eval(si) << " ; #" << h.index(si);
    if (trace) {
      index_type j = h.index(si);
      assert(j == i);
      s << " {";
      for (index_type k = 0; k < trace[j].length(); k++) {
	if (k > 0) s << ", ";
	if (trace[j][k].second != no_such_index)
	  s << instance.actions[trace[j][k].second].name;
	else
	  s << "-";
	s << "/" << trace[j][k].first;
      }
      s << "}";
    }
    s << std::endl;
  }
}

void RegressionPDB::abstract_plan_from_trace
(index_type i, index_vec& p, PlanSet& ps, index_type& n, index_type max)
{
  assert(trace);
  assert(i < h.n_values());
  NTYPE v = val[i];
  if (INFINITE(v)) return;
  if (v == 0) {
    Plan* np = ps.new_plan();
    for (index_type k = 0; k < p.length(); k++) {
      np->insert(p[k]);
      np->advance(1);
    }
    np->end();
    n += 1;
    return;
  }
  for (index_type k = 0; k < trace[i].length(); k++) {
    if (trace[i][k].second != no_such_index)
      p.append(trace[i][k].second);
    abstract_plan_from_trace(trace[i][k].first, p, ps, n, max);
    if (trace[i][k].second != no_such_index)
      p.dec_length();
    if (n >= max) return;
  }
}

void RegressionPDB::abstract_plan_from_trace
(const partial_state& s, PlanSet& ps, index_type max)
{
  assert(trace);
  index_vec p(no_such_index, 0);
  index_type n = 0;
  index_type i = h.index(s);
  abstract_plan_from_trace(i, p, ps, n, max);
}

void RegressionPDB::abstract_plan
(const partial_state& s,
 index_vec& p, PlanSet& ps,
 index_type& n, index_type max)
{
  NTYPE v = val[h.index(s)];
//   std::cerr << "s0 = " << h.index(s) << ": ";
//   instance.write_partial_state(std::cerr, s);
//   std::cerr << ", val = " << v << std::endl;
  if (INFINITE(v)) return;
  if (v == 0) {
    Plan* np = ps.new_plan();
    for (index_type k = 1; k <= p.length(); k++) {
      np->insert(p[p.length() - k]);
      np->advance(1);
    }
    np->end();
    n += 1;
    return;
  }
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (s.consistent_with(instance.actions[k].post) &&
	(s.count_common(instance.actions[k].post) > 0) &&
	s.consistent_with(instance.actions[k].prv)) {
//       std::cerr << "testing action " << k << "." << instance.actions[k].name
//  		<< " with cost " << cost(k) << std::endl;
      partial_state s1(s);
      s1.subtract(instance.actions[k].post);
      assign(s1, instance.actions[k].pre);
      assign(s1, instance.actions[k].prv);
      PartialStateEnumerator
	pre_se(variable_set, instance.signature, s1);
//       std::cerr << "pre-state constraints: ";
//       instance.write_partial_state(std::cerr, s1);
      bool more = pre_se.first();
      while (more) {
	NTYPE v1 = val[h.index(pre_se.current_state())];
//  	std::cerr << "possible pre-state: ";
//  	instance.write_partial_state(std::cerr, pre_se.current_state());
//  	std::cerr << " with value " << v1 << std::endl;
	if (v == (v1 + cost(k))) {
	  p.append(k);
	  abstract_plan(pre_se.current_state(), p, ps, n, max);
	  p.dec_length();
	  if (n >= max) return;
	}
	more = pre_se.next();
      }
    }
}

void RegressionPDB::abstract_plan
(const partial_state& s, PlanSet& ps, index_type max)
{
  assert(max > 0);
  NTYPE v = val[h.index(s)];
  index_vec p(no_such_index, 0);
  index_type n = 0;
  PartialStateEnumerator se(variable_set, instance.signature, s);
  bool more = se.first();
  while (more) {
    NTYPE vc = val[h.index(se.current_state())];
    if (vc == v) {
      abstract_plan(se.current_state(), p, ps, n, max);
    }
    if (n >= max) return;
    more = se.next();
  }

  if ((n == 0) && FINITE(v)) {
    std::cerr << "error: no plan found for ";
    instance.write_partial_state(std::cerr, s);
    std::cerr << " with cost = " << v << std::endl;
    if (trace) {
      ActionSequenceSet ps1;
      abstract_plan_from_trace(s, ps1, no_such_index);
      std::cerr << ps1.length() << " plans found by trace" << std::endl;
      PrintSASActions ps_out(instance, std::cerr);
      ps1.output(ps_out);
    }
    exit(255);
  }
}

ProgressionPDB::ProgressionPDB
(SASInstance& ins,
 const index_set& v,
 const ACF& c,
 MDDNode* sinc,
 Heuristic* inc,
 Statistics& s)
  : AbstractionHeuristic(ins, v, c, sinc, inc),
    h(ins, v),
    val(0),
    stats(s)
{
  assert(!h.overflow());
  val = new NTYPE[h.n_values() + 2];
}

ProgressionPDB::~ProgressionPDB()
{
  delete [] val;
}

index_type ProgressionPDBSize::apply
(const index_vec& sig, const index_set& set)
{
  ProgressionPDBSize f(sig);
  return f(set);
}

index_type ProgressionPDBSize::operator()(const index_set& set) const
{
  index_type s = 1;
  for (index_type k = 0; k < set.length(); k++) {
    if ((no_such_index / signature[set[k]]) < s) return no_such_index;
    s = (s * signature[set[k]]);
  }
  return s;
}

void ProgressionPDB::compute()
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "computing progression PDB: variables = ";
  instance.write_variable_set(std::cerr, variables());
  std::cerr << ", goal state = ";
  instance.write_partial_state(std::cerr, instance.goal_state);
  std::cerr << std::endl;
  if (sinc) {
    std::cerr << "MDD = ";
    sinc->write_graph(std::cerr);
  }
  else {
    std::cerr << "MDD = 0" << std::endl;
  }
#endif

  stats.start();

  // clear the queue
  while (!open.empty()) open.pop();

  // init DB: set states consistent with goal state to 0 and
  // everything else to +INF, and at same time init open list
  for (index_type k = 0; k < h.n_values(); k++) val[k] = POS_INF;
  PartialStateEnumerator se(variable_set,
			    instance.signature,
			    instance.goal_state);
  bool more = se.first();
  while (more) {
    if (sinc) {
#ifdef CHECK_MDD_EVAL
      assert(inc);
      index_set atms;
      instance.map_to_atom_set(se.current_state(), atms);
      if (((bool)INFINITE(inc->eval(atms))) !=
	  sinc->lookup(se.current_state())) {
	std::cerr << "failed CHECK_MDD_EVAL: s = "
		  << se.current_state() << " = ";
	instance.write_partial_state(std::cerr, se.current_state());
	std::cerr << ", atom set = " << atms
		  << ", lookup = " << sinc->lookup(se.current_state())
		  << ", h.eval = " << inc->eval(atms)
		  << std::endl;
	inc->write_eval(atms, std::cerr, "h.eval: ", true);
	std::cerr << "MDD:" << std::endl;
	sinc->write_graph(std::cerr);
	exit(255);
      }
#endif
      if (!sinc->lookup(se.current_state())) {
	val[h.index(se.current_state())] = 0;
	open.push(OpenState(se.current_state(), 0));
      }
    }
    else {
      val[h.index(se.current_state())] = 0;
      open.push(OpenState(se.current_state(), 0));
    }
    more = se.next();
  }

#ifdef TRACE_PRINT_LOTS
  std::cerr << "initial PDB (before backward pass):" << std::endl;
  write(std::cerr);
#endif

  // compute list of relevant actions, sorted by increasing cost
  acts.clear();
  action_cost_increasing order_op(cost);
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].post.defines_any(variable_set))
      acts.insert_ordered(k, order_op);

  // main exploration loop
  while (!open.empty() && !stats.break_signal_raised()) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "|open| = " << open.size() << ", top = ";
    instance.write_partial_state(std::cerr, open.top().state);
    std::cerr << std::endl;
#endif
    for (index_type k = 0; k < acts.length(); k++) {
      SASInstance::Action& act = instance.actions[acts[k]];
      if (open.top().state.consistent_with(act.post) &&
	  open.top().state.consistent_with(act.prv)) {
#ifdef TRACE_PRINT_LOTS
 	std::cerr << "- action " << act.name << " is consistent"
 	          << std::endl;
#endif
	partial_state s_pre(open.top().state);
	s_pre.assign(act.pre);
	s_pre.assign(act.prv);
	PartialStateEnumerator pre_se(act.u_pre, instance.signature);
	bool more = pre_se.first();
	while (more) {
	  partial_state s1(s_pre);
	  s1.assign(pre_se.current_state());
	  NTYPE c_res = open.top().cost + cost(acts[k]);
	  if (sinc) {
#ifdef CHECK_MDD_EVAL
	    assert(inc);
	    index_set atms;
	    instance.map_to_atom_set(s1, atms);
	    if (((bool)INFINITE(inc->eval(atms))) != sinc->lookup(s1)) {
	      std::cerr << "error: CHECK_MDD_EVAL failed" << std::endl;
	      std::cerr << "s1 = " << s1 << " = ";
	      instance.write_partial_state(std::cerr, s1);
	      std::cerr << std::endl;
	      std::cerr << "atom set = " << atms << std::endl;
	      std::cerr << "lookup(s1) = " << sinc->lookup(s1) << std::endl;
	      std::cerr << "inc.eval(atms) = " << inc->eval(atms) << std::endl;
	      exit(255);
	    }
#endif
	    if (sinc->lookup(s1)) c_res = POS_INF;
	  }
	  index_type i_res = h.index(s1);
	  if (c_res < val[i_res]) {
	    val[i_res] = c_res;
	    open.push(OpenState(s1, variables(), c_res));
	  }
	  more = pre_se.next();
	}
      }
    }
    open.pop();
  }

  stats.stop();
}

void ProgressionPDB::expand_next_open()
{
  assert(!open.empty()); // open queue must not be empty
  for (index_type k = 0; k < acts.length(); k++) {
    SASInstance::Action& act = instance.actions[acts[k]];
    if (open.top().state.consistent_with(act.post) &&
	open.top().state.consistent_with(act.prv)) {
      partial_state s_pre(open.top().state);
      s_pre.assign(act.pre);
      s_pre.assign(act.prv);
      PartialStateEnumerator pre_se(act.u_pre, instance.signature);
      bool more = pre_se.first();
      while (more) {
	partial_state s1(s_pre);
	s1.assign(pre_se.current_state());
	NTYPE c_res = open.top().cost + cost(acts[k]);
	if (sinc) {
	  if (sinc->lookup(s1)) c_res = POS_INF;
	}
	index_type i_res = h.index(s1);
	if (c_res < val[i_res]) {
	  val[i_res] = c_res;
	  open.push(OpenState(s1, variables(), c_res));
	}
	more = pre_se.next();
      }
    }
  }
  open.pop();
}

void ProgressionPDB::initialize()
{
  stats.start();

  // init DB: set states consistent with goal state to 0 and
  // everything else to +INF, and at same time init open list
  for (index_type k = 0; k < h.n_values(); k++) val[k] = POS_INF;
  PartialStateEnumerator se(variables(),
			    instance.signature,
			    instance.goal_state);
  bool more = se.first();
  while (more) {
    if (sinc) {
      if (!sinc->lookup(se.current_state())) {
	val[h.index(se.current_state())] = 0;
	open.push(OpenState(se.current_state(), 0));
      }
    }
    else {
      val[h.index(se.current_state())] = 0;
      open.push(OpenState(se.current_state(), 0));
    }
    more = se.next();
  }

  // compute list of relevant actions, sorted by increasing cost
  acts.clear();
  action_cost_increasing order_op(cost);
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].post.defines_any(variable_set))
      acts.insert_ordered(k, order_op);

  // clear the queue
  while (!open.empty()) open.pop();

  stats.stop();
}

void ProgressionPDB::compute_to(const partial_state& s)
{
  stats.start();
  index_type i_s = h.index(s);
  while (INFINITE(val[i_s]) && !open.empty() && !stats.break_signal_raised()) {
    expand_next_open();
  }
  stats.stop();
}

void ProgressionPDB::compute_rest()
{
  stats.start();
  while (!open.empty() && !stats.break_signal_raised()) {
    expand_next_open();
  }
  stats.stop();
}

void ProgressionPDB::initialize2
(MDDNode* sinc, Heuristic* inc, bool_vec& valid, index_type*& queue_write)
{
  PartialStateEnumerator e(variable_set, instance.signature);
  bool more = e.first();
  while (more) {
    index_type i = h.index(e.current_state());
    val[i] = POS_INF;
    if (sinc) {
      if (!consistent(e.current_state(), sinc, inc))
	valid[i] = false;
    }
    if (valid[i] && e.current_state().implies_restricted(instance.goal_state, variable_set)) {
      val[i] = 0;
      *queue_write++ = i;
    }
    more = e.next();
  }
}

ProgressionPDB::MatchTree ProgressionPDB::compute_match_tree()
{
  MatchTree match_tree;
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].post.defines_any(variable_set)) {
      lvector<Test> tests;
      int op_diff = 0;

      const partial_state& pre = instance.actions[k].pre;
      for (index_type i = 0; i < pre.length(); i++) {
	op_diff += h.index1(pre[i].first, pre[i].second);
      }
      
      const partial_state& prev = instance.actions[k].prv;
      const partial_state& post = instance.actions[k].post;
      index_type prev_index = 0;
      index_type post_index = 0;
      while(prev_index < prev.length() || post_index < post.length()) {
	bool use_prev;
	if (prev_index == prev.length()) {
	  use_prev = false;
	}
	else if (post_index == post.length()) {
	  use_prev = true;
	}
	else {
	  index_type prev_var = prev[prev_index].first;
	  index_type post_var = post[post_index].first;
	  assert(prev_var != post_var);
	  use_prev = prev_var < post_var;
	}
	if (use_prev) {
	  if (variable_set.contains(prev[prev_index].first)) {
	    Test t(prev[prev_index].second,
		   h[prev[prev_index].first],
		   instance.signature[prev[prev_index].first]);
	    tests.push_back(t);
	  }
	  prev_index += 1;
	}
	else {
	  if (variable_set.contains(post[post_index].first)) {
	    Test t(post[post_index].second,
		   h[post[post_index].first],
		   instance.signature[post[post_index].first]);
	    tests.push_back(t);
	    op_diff -=
	      h.index1(post[post_index].first, post[post_index].second);
	  }
	  post_index += 1;
	}
      }
#ifdef TRACE_PRINT_LOTS
      std::cerr << "action " << instance.actions[k].name
		<< ": pre = " << instance.actions[k].pre
		<< ", post = " << instance.actions[k].post
		<< ", tests = " << tests
		<< ", diff = " << op_diff
		<< std::endl;
#endif
      match_tree.insert(tests, op_diff, 0);
    }
  return match_tree;
}

ProgressionPDB::MatchTree::MatchTree()
{
  // done
}

ProgressionPDB::MatchTree::~MatchTree()
{
  for (index_type i = 0; i < branches.size(); i++)
    delete[] branches[i].children;
}

void ProgressionPDB::MatchTree::insert
(const lvector<Test>& tests, int op_diff, index_type i)
{
  if (i == tests.size()) {
    // All tests have been performed. Add operator here.
    if (operator_diffs.first(op_diff) == no_such_index)
      operator_diffs.append(op_diff);
  }
  else {
    const Test& t = tests[i];
    for (index_type b = 0; b < branches.size(); b++) {
      Branch& branch = branches[b];
      if (branch.divisor == t.divisor) {
        branch.children[t.value].insert(tests, op_diff, i + 1);
        return;
      }
    }
    // Must add new branch here.
    Branch branch;
    branch.divisor = t.divisor;
    branch.modulus = t.modulus;
    branch.children = new MatchTree[t.modulus];
    branch.children[t.value].insert(tests, op_diff, i + 1);
    branches.push_back(branch);
  }
}

void ProgressionPDB::MatchTree::write(std::ostream& s)
{
  s << operator_diffs << " (";
  for (index_type i = 0; i < branches.size(); i++) {
    const Branch& branch = branches[i];
    s << "i / " << branch.divisor << " % " << branch.modulus << " = ";
    for (index_type j = 0; j < branch.modulus; j++) {
      if (j > 0) s << "; ";
      s << j << ": ";
      branch.children[j].write(s);
    }
  }
  s << ")";
}

void ProgressionPDB::MatchTree::traverse
(NTYPE* db,
 const bool_vec& valid,
 index_type*& queue_write,
 index_type index) const
{
  for (index_type i = 0; i < branches.size(); i++) {
    const Branch& branch = branches[i];
    index_type child_no = (index / branch.divisor) % branch.modulus;
    branch.children[child_no].traverse(db, valid, queue_write, index);
  }
  for (index_type i = 0; i < operator_diffs.length(); i++) {
    index_type target = index + operator_diffs[i];
    // assert(target < size());
    if (INFINITE(db[target]) && valid[target]) {
      db[target] = db[index] + 1;
      *queue_write++ = target;
    }
  }
}

void ProgressionPDB::compute2()
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "compute2: ";
  instance.write_variable_set(std::cerr, variables());
  std::cerr << " = " << variables() << std::endl;
#endif
  bool_vec valid(true, size());
  index_type* queue = new index_type[size()];
  index_type* queue_read = queue;
  index_type* queue_write = queue;

  initialize2(sinc, inc, valid, queue_write);

#ifdef TRACE_PRINT_LOTS
  std::cerr << "after initialization:" << std::endl;
  for (index_type k = 0; k < size(); k++) {
    std::cerr << k << " -- " << val[k] << " -- " << valid[k] << std::endl;
  }
  std::cerr << "queue:";
  for (index_type* p = queue_read; p != queue_write; p++) {
    std::cerr << " " << *p;
  }
  std::cerr << std::endl;
#endif

  MatchTree match_tree = compute_match_tree();

#ifdef TRACE_PRINT_LOTS
  std::cerr << "match tree = ";
  match_tree.write(std::cerr);
  std::cerr << std::endl;
#endif

  while (queue_read != queue_write) {
    match_tree.traverse(val, valid, queue_write, *queue_read);
    queue_read += 1;
  }
  delete[] queue;
}

NTYPE ProgressionPDB::eval(const partial_state& s)
{
  // note: completeness check on s is implemented in CSA::index
  return val[h.index(s)];
}

NTYPE ProgressionPDB::max()
{
  NTYPE v = 0;
  for (index_type k = 0; k < h.n_values(); k++)
    if (FINITE(val[k])) v = MAX(v, val[k]);
  return v;
}

NTYPE ProgressionPDB::mean()
{
  NTYPE v_sum = 0;
  for (index_type k = 0; k < h.n_values(); k++) {
    if (INFINITE(val[k]))
      v_sum += MEAN_VALUE_INFINITY;
    else
      v_sum += val[k];
  }
  return (v_sum / h.n_values());
}

void ProgressionPDB::abstract_plan
(const partial_state& s,
 index_vec& p, PlanSet& ps,
 index_type& n, index_type max)
{
  NTYPE v = val[h.index(s)];
//   std::cerr << "s0 = " << h.index(s) << ": ";
//   instance.write_partial_state(std::cerr, s);
//   std::cerr << ", val = " << v << std::endl;
  assert(FINITE(v));
  if (v == 0) {
    Plan* np = ps.new_plan();
    for (index_type k = 0; k < p.length(); k++) {
      np->insert(p[k]);
      np->advance(1);
    }
    np->end();
    n += 1;
    return;
  }
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (implies(s, instance.actions[k].pre) &&
	implies(s, instance.actions[k].prv)) {
//       std::cerr << "testing action " << k << "." << instance.actions[k].name
//  		<< " with cost " << cost(k) << std::endl;
      partial_state s1(s);
      assign(s1, instance.actions[k].post);
//       std::cerr << "pre-state constraints: ";
//       instance.write_partial_state(std::cerr, s1);
      NTYPE v1 = val[h.index(s1)];
      if (v == (v1 + cost(k))) {
	p.append(k);
	abstract_plan(s1, p, ps, n, max);
	p.dec_length();
	if (n >= max) return;
      }
    }
}

void ProgressionPDB::abstract_plan
(const partial_state& s, PlanSet& ps, index_type max)
{
  assert(max > 0);
  NTYPE v = val[h.index(s)];
  if (INFINITE(v)) return;
  index_vec p(no_such_index, 0);
  index_type n = 0;
  abstract_plan(s, p, ps, n, max);
  if (n == 0) {
    std::cerr << "error: no plan found for ";
    instance.write_partial_state(std::cerr, s);
    std::cerr << " with cost = " << v << std::endl;
    exit(255);
  }
}

void ProgressionPDB::write(std::ostream& s)
{
  PartialStateEnumerator se(variable_set, instance.signature);
  bool more = se.first();
  while (more) {
    s << "H(";
    instance.write_partial_state(s, se.current_state());
    s << ") = " << val[h.index(se.current_state())]
      << " ; #" << h.index(se.current_state())
      << std::endl;
    more = se.next();
  }
}


bool ASHProgState::cut_at_solved = true;

ASHProgState::ASHProgState
(SASInstance& i,
 StateAbstraction& a,
 const index_set_vec& aa,
 const ACF& c,
 SASHeuristic& h,
 MDDNode* shi,
 Heuristic* hi,
 const partial_state& s0,
 partial_state_map& ash)
  : SASSeqProgState(i, c, h),
    sa(a),
    sa_acts(aa),
    sinc(shi),
    inc(hi),
    ash_store(ash),
    sflag(false)
{
  // state.restrict_to(sa.variables());
  state.clear();
  sa.assign(state, s0);
  reevaluate();
}

ASHProgState::ASHProgState(const ASHProgState& s)
  : SASSeqProgState(s),
    sa(s.sa),
    sa_acts(s.sa_acts),
    sinc(s.sinc),
    inc(s.inc),
    ash_store(s.ash_store),
    sflag(s.sflag)
{
  // done
}

ASHProgState::ASHProgState(ASHProgState& p, SASInstance::Action& a)
  : SASSeqProgState(p),
    sa(p.sa),
    sa_acts(p.sa_acts),
    sinc(p.sinc),
    inc(p.inc),
    ash_store(p.ash_store),
    sflag(false)
{
  sa.assign(state, a.post);
  reevaluate();
  c_delta = cost(a.index);
  set_predecessor(&p);
  act = a.index;
}

NTYPE ASHProgState::delta_cost()
{
//   if (cut_at_solved && sflag) {
//     return (c_delta + c_est);
//   }
  return c_delta;
}

NTYPE ASHProgState::est_cost()
{
//   if (cut_at_solved && sflag) {
//     return 0;
//   }
  return c_est;
}

bool ASHProgState::is_final()
{
//  if (cut_at_solved && sflag) return true;
  return sa.implies(state, instance.goal_state);
}

NTYPE ASHProgState::expand(Search& s, NTYPE bound)
{
  NTYPE c_min = POS_INF;
#ifdef TRACE_PRINT_LOTS
  std::cerr << "expanding " << state << ", ";
  instance.write_partial_state(std::cerr, state);
  std::cerr << " at bound " << bound << "..." << std::endl;
#endif
#ifdef ASH_USE_EQC
  for (index_type k = 0; k < sa_acts.length(); k++) {
    bool c_app = false;
    for (index_type j = 0; (j < sa_acts[k].length()) && !c_app; j++) {
      SASInstance::Action& act = instance.actions[sa_acts[k][j]];
#else
  for (index_type k = 0; k < instance.n_actions(); k++) {
      SASInstance::Action& act = instance.actions[k];
#endif
      bool app = true;
      if (!sa.implies(state, act.pre)) app = false;
      if (!sa.implies(state, act.prv)) app = false;
      if (app && sinc) {
	partial_state s_pre(state);
	s_pre.insert(act.pre);
	s_pre.insert(act.prv);
	if (sinc->lookup(s_pre)) app = false;
#ifdef CHECK_MDD_EVAL
	assert(inc);
	index_set a_pre;
	instance.make_atom_set(s_pre, a_pre);
	assert(((bool)INFINITE(inc->eval(a_pre))) == sinc->lookup(s_pre));
#endif
      }
      if (app) {
#ifdef ASH_USE_EQC
	c_app = true;
#endif
#ifdef TRACE_PRINT_LOTS
	std::cerr << "action " << act.name << " is applicable" << std::endl;
#endif
	ASHProgState* s_new = new ASHProgState(*this, act);
#ifdef TRACE_PRINT_LOTS
	std::cerr << "new state: " << *s_new << std::endl;
#endif
	if ((s_new->c_est + s_new->c_delta) <= bound) {
	  NTYPE c_new =
	    s.new_state(*s_new, bound - s_new->c_delta) + s_new->c_delta;
	  if (s.done()) {
#ifdef TRACE_PRINT_LOTS
	    std::cerr << "exit depth " << depth()
		      << " as search done, solution cost = "
		      << c_new << std::endl;
#endif
	    delete s_new;
	    return c_new;
	  }
	  else {
	    c_min = MIN(c_min, c_new);
	  }
	}
	else {
	  c_min = MIN(c_min, (s_new->c_est + s_new->c_delta));
	}
	delete s_new;
      }
#ifndef ASH_USE_EQC
  }
#else
    }
  }
#endif
#ifdef TRACE_PRINT_LOTS
  std::cerr << "exit depth " << depth()
	    << " with min = " << c_min
	    << " > bound = " << bound
	    << std::endl;
#endif
  return c_min;
}

void ASHProgState::store(NTYPE cost, bool opt)
{
  if (opt) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "store ";
    instance.write_partial_state(std::cerr, state);
    std::cerr << " as SOLVED with cost " << cost << std::endl;
#endif
    ash_store[state] = HValue(cost, opt);
  }
  else if (cost > ash_store[state].val) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "store ";
    instance.write_partial_state(std::cerr, state);
    std::cerr << " with cost " << cost << std::endl;
#endif
    ash_store[state] = HValue(cost, opt);
  }
}

void ASHProgState::reevaluate()
{
  assert(!state.defines_any_not_in(sa.variables()));
  sflag = false;
  partial_state_map::iterator p = ash_store.find(state);
  if (p != ash_store.end()) {
    c_est = p->second.val;
    if (FINITE(c_est) && p->second.opt)
      sflag = true;
#ifdef TRACE_PRINT_LOTS
    std::cerr << "ASHProgState::reevaluate: s = " << state << " = ";
    instance.write_partial_state(std::cerr, state);
    std::cerr << ", found with c_est = " << c_est
 	      << " and sflag = " << sflag
 	      << std::endl;
#endif
    return;
  }
  c_est = heuristic.eval(state);
  if (sinc) {
#ifdef CHECK_MDD_EVAL
    assert(inc);
    index_set ax;
    instance.make_atom_set(state, ax);
    assert(((bool)INFINITE(inc->eval(ax))) == sinc->lookup(state));
#endif
    if (sinc->lookup(state)) c_est = POS_INF;
  }
}

State* ASHProgState::copy()
{
  return new ASHProgState(*this);
}

ProgressionASH::ProgressionASH
(SASInstance& ins,
 const index_set& v,
 const ACF& c,
 MDDNode* sinc,
 Heuristic* inc,
 SASHeuristic& h,
 Statistics& s)
  : AbstractionHeuristic(ins, v, c, sinc, inc),
    abs_shf(ins, v),
    val(10, abs_shf),
    h_search(h),
    stats(s)
{
#ifdef ASH_USE_EQC
#ifdef TRACE_PRINT_LOTS
  std::cerr << "constructing ASH for ";
  instance.write_variable_set(std::cerr, variables());
  std::cerr << "..." << std::endl;
#endif
  index_vec rel_acts;
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].post.defines_any(variables()))
      rel_acts.append(k);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "relevant actions: " << rel_acts << std::endl;
#endif
  equivalence eq_acts(rel_acts.length());
  for (index_type i = 0; i < rel_acts.length(); i++)
    for (index_type j = i + 1; j < rel_acts.length(); j++)
      if (equals(instance.actions[rel_acts[i]].post,
		 instance.actions[rel_acts[j]].post) &&
	  (cost(rel_acts[i]) == cost(rel_acts[j])))
	eq_acts.merge(i, j);
  sa_acts.assign_value(EMPTYSET, eq_acts.n_classes());
  index_vec eq_map;
  eq_acts.make_map(eq_map);
  for (index_type k = 0; k < rel_acts.length(); k++) {
    index_type c = eq_map[k];
    assert(c < sa_acts.length());
    sa_acts[c].insert(rel_acts[k]);
  }
#ifdef TRACE_PRINT_LOTS
  std::cerr << "action equivalence classes: " << sa_acts << std::endl;
#endif
#endif
}

ProgressionASH::~ProgressionASH()
{
  // done
}

HValue ProgressionASH::solveIDA
(const partial_state& s, NTYPE b, ActionSequenceSet* p)
{
  partial_state a(s, variables());
#ifdef TRACE_PRINT_LOTS
  std::cerr << "solving s = " << s << " -> ";
  instance.write_partial_state(std::cerr, a);
  std::cerr << " using IDA*..." << std::endl;
#endif
  ASHProgState s0(instance, *this, sa_acts, cost, h_search, sinc, inc, s, val);
  ASHProgState::cut_at_solved = false;
  Result res;
  res.set_plan_set(p);
  IDAO search(stats, res);
  search.set_cycle_check(false);
  search.set_store_cost(true);
  search.set_cost_limit(MIN(b, size()));
#ifdef TRACE_PRINT_LOTS
  search.set_trace_level(3);
#else
  search.set_trace_level(0);
#endif
  NTYPE v = search.start(s0);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "search finished: solved = "
	    << search.solved() << " cost = " << v
	    << ", plan(s): ";
  if (p) {
    PrintSASActions print(instance, std::cerr, ',', ';');
    p->output(print);
    std::cerr << std::endl;
  }
  else {
    std::cerr << "not stored" << std::endl;
  }
#endif
#ifdef TRACE_PRINT_LOTS
  if (!search.solved() && FINITE(v)) {
    std::cerr << "hit limit = " << v
	      << " (b = " << b << ", size = " << size() << "), "
	      << stats.time() << " seconds"
	      << std::endl;
  }
#endif
  return HValue(v, search.solved());
}

HValue ProgressionASH::solveBB
(const partial_state& s, NTYPE b)
{
  partial_state a(s, variables());
#ifdef TRACE_PRINT_LOTS
  std::cerr << "solving s = " << s << " -> ";
  instance.write_partial_state(std::cerr, a);
  std::cerr << " using branch-and-bound..." << std::endl;
#endif
  ASHProgState s0(instance, *this, sa_acts, cost, h_search, sinc, inc, s, val);
  ASHProgState::cut_at_solved = false;
  Result res;
  res.set_stop_condition(Result::stop_at_all_optimal);
  DFS_BB search(stats, res);
#ifdef TRACE_PRINT_LOTS
  search.set_trace_level(3);
#else
  search.set_trace_level(0);
#endif
  NTYPE v = search.start(s0);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "search finished: solved = "
	    << search.solved() << " cost = " << v
	    << std::endl;
#endif
  if (!search.solved() && FINITE(v)) {
    std::cerr << "hit limit = " << v
	      << " (b = " << b << ", size = " << size() << "), "
	      << stats.time() << " seconds"
	      << std::endl;
  }
  return HValue(v, (search.solved() && search.optimal()));
}

HValue ProgressionASH::solveBFS
(const partial_state& s, NTYPE b)
{
  partial_state a(s, variables());
#ifdef TRACE_PRINT_LOTS
  std::cerr << "solving s = " << s << " -> ";
  instance.write_partial_state(std::cerr, a);
  std::cerr << " using A*..." << std::endl;
#endif
  ASHProgState s0(instance, *this, sa_acts, cost, h_search, sinc, inc, s, val);
  ASHProgState::cut_at_solved = false;
  Result res;
  BFS search(stats, res);
  search.set_cost_limit(MIN(b, size()));
#ifdef TRACE_PRINT_LOTS
  search.set_trace_level(4);
#else
  search.set_trace_level(0);
#endif
  NTYPE v = search.start(s0);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "search finished: solved = "
	    << search.solved() << " cost = " << v
	    << std::endl;
#endif
  search.state_space().cache_pg(v);
  return HValue(v, search.solved());
}

NTYPE ProgressionASH::eval(const partial_state& s)
{
  // std::cerr << "ProgressionASH::eval..." << std::endl;
  partial_state a(s, variables());
  partial_state_map::iterator p = val.find(a);
  if (p != val.end()) {
    if (p->second.opt) {
      return p->second.val;
    }
  }
#ifdef ASH_SOLVE_USE_BFS
  HValue v = solveBFS(s, POS_INF);
#else
#ifdef ASH_SOLVE_USE_IDA
  HValue v = solveIDA(s, POS_INF, 0);
  if (!v.opt) v.val = POS_INF;
#else
  HValue v = solveBB(s, POS_INF);
#endif
#endif
  if (v.opt) {
    assert(FINITE(v.val));
    val[a] = v;
  }
  return v.val;
}

NTYPE ProgressionASH::eval_to_bound(const partial_state& s, NTYPE b)
{
  // std::cerr << "ProgressionASH::eval_to_bound..." << std::endl;
  partial_state a(s, variables());
  partial_state_map::iterator p = val.find(a);
  if (p != val.end()) {
    if (p->second.opt) {
      return p->second.val;
    }
    else if (p->second.val > b) {
      return p->second.val;
    }
  }
#ifdef ASH_SOLVE_USE_BFS
  HValue v = solveBFS(s, b);
#else
#ifdef ASH_SOLVE_USE_IDA
  HValue v = solveIDA(s, b, 0);
#else
  HValue v = solveBB(s, b);
#endif
#endif
  if (v.opt) {
    assert(FINITE(v.val));
    val[a] = v;
  }
  return v.val;
}

void ProgressionASH::abstract_plan
(const partial_state& s, PlanSet& ps, index_type max)
{
  partial_state a(s, variables());
#ifdef TRACE_PRINT_LOTS
  std::cerr << "finding abstract plans for s = " << s << " -> ";
  instance.write_partial_state(std::cerr, a);
  std::cerr << "..." << std::endl;
#endif
  ASHProgState s0(instance, *this, sa_acts, cost, h_search, sinc, inc, s, val);
  ASHProgState::cut_at_solved = false;
  Result res;
  if (max == no_such_index)
    res.set_stop_condition(Result::stop_at_all_optimal);
  else
    res.set_n_to_find(max);
  res.set_plan_set(&ps);
  IDAO search(stats, res);
  search.set_cycle_check(true);
#ifdef TRACE_PRINT_LOTS
  search.set_trace_level(3);
#else
  search.set_trace_level(0);
#endif
  NTYPE v = search.start(s0);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "search finished: solved = "
	    << search.solved() << " cost = " << v
	    << ", " << res.solution_count() << " plans found"
	    << std::endl;
#endif
}

void ProgressionASH::write(std::ostream& s)
{
  // ...
}

PDBCollection::PDBCollection
(SASInstance& i, const ACF& c, MDDNode* shi, Heuristic* hi, Statistics& s)
  : abstraction_heuristic_vec(0, 0),
    instance(i),
    cost(c),
    sinc(shi),
    inc(hi),
    stats(s),
    own(false, 0)
{
  update_flag = false;
}

PDBCollection::PDBCollection(PDBCollection& c)
  : abstraction_heuristic_vec(c),
    instance(c.instance),
    cost(c.cost),
    sinc(c.sinc),
    inc(c.inc),
    stats(c.stats),
    own(false, c.length()),
    g_add(c.g_add),
    g_subset(c.g_subset),
    canonical_h(c.canonical_h),
    update_flag(c.update_flag)
{
//   for (index_type k = 0; k < c.length(); k++)
//     append(c[k]);
//   own.assign_value(false, length());
//   update_flag = true;
}

PDBCollection::~PDBCollection()
{
  for (index_type k = 0; k < length(); k++) if (own[k]) {
    assert((*this)[k]);
    delete (*this)[k];
  }
}

AbstractionHeuristic* PDBCollection::find_PDB_with_pattern
(const index_set& set)
{
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k]->variables() == set) return (*this)[k];
  return 0;
}

index_type PDBCollection::n_patterns()
{
  return length();
}

index_type PDBCollection::total_size()
{
  index_type s = 0;
  for (index_type k = 0; k < length(); k++)
    s += (*this)[k]->size();
  return s;
}

void PDBCollection::addPDB(AbstractionHeuristic* h, bool o)
{
  assert(h);
  index_type i = length();
  append(h);
  assert((*this)[i] == h);
  own.append(o);
  assert(length() == own.length());
  g_add.add_node();
  assert(g_add.size() == length());
  for (index_type k = 0; k < i; k++)
    if ((*this)[k]->additive(*h)) g_add.add_undirected_edge(k, i);
  g_subset.add_node();
  assert(g_subset.size() == length());
  for (index_type k = 0; k < i; k++) {
    if ((*this)[k]->variables().contains(h->variables()))
      g_subset.add_edge(i, k);
    if (h->variables().contains((*this)[k]->variables()))
      g_subset.add_edge(k, i);
  }
  g_subset.add_edge(i, i);
  update_flag = true;
}

void PDBCollection::addProgressionPDB(const index_set& set, bool opt_fast)
{
  ProgressionPDB* new_pdb =
    new ProgressionPDB(instance, set, cost, sinc, inc, stats);
  if (opt_fast)
    new_pdb->compute2();
  else
    new_pdb->compute();
  if (stats.break_signal_raised()) {
    delete new_pdb;
    return;
  }
  addPDB(new_pdb, true);
}


bool PDBCollection::sum_is_dominated
(const index_set &dominatee, const index_set &dominator) const
{
  // Criterion: An admissible sum P_1 + ... + P_m is dominated by
  // an admissible sum P'_1 + ... + P'_n iff each P_i is a subset
  // of sum P'_j -- in other words, P_i must have a neigbour in the
  // pattern inclusion graph that appears in the second sum.
  for (index_type i = 0; i < dominatee.length(); i++)
    if (!g_subset.adjacent(dominatee[i], dominator))
      return false;
  return true;
}

void PDBCollection::compute_canonical_heuristic()
{
  update_flag = false;

  canonical_h.clear();
  g_add.all_nondominated_cliques(canonical_h);
  if (canonical_h.empty()) return;
  // canonical_h now represents all maximal admissible sums.
  // Now prune the collection to remove dominated elements.

#ifdef TRACE_PRINT_LOTS
  // std::cerr << "*** before dominance pruning ***" << std::endl;
  // std::cerr << canonical_h << std::endl;
  // write_collection(std::cerr);
#endif
  // NOTE: Use int to avoid problems when computing 0 - 1.
  for (int i = int(canonical_h.length()) - 1; i >= 0; i--) {
    const index_set& this_sum = canonical_h[i];
    for (index_type j = 0; j < canonical_h.length(); j++) {
      if ((i != ((int)j)) && sum_is_dominated(this_sum, canonical_h[j])) {
        index_type back = canonical_h.length() - 1;
#ifdef TRACE_PRINT_LOTS
        std::cerr << "  => pruning      " << i << ": " << canonical_h[i];
        // write_pattern_sum(std::cerr, canonical_h[i]);
        std::cerr << std::endl << "     dominated by " << j << ": " << canonical_h[j];
        // write_pattern_sum(std::cerr, canonical_h[j]);
	std::cerr << std::endl << " from " << canonical_h
		  << std::endl << " g_subset = " << g_subset
		  << std::endl;
#endif
        canonical_h.remove(i);
        break;
      }
    }
  }
#ifdef TRACE_PRINT_LOTS
  // std::cerr << "*** after dominance pruning ***" << std::endl;
  // std::cerr << canonical_h << std::endl;
  // write_collection(std::cerr);
#endif
}

NTYPE PDBCollection::mean()
{
  if (update_flag) compute_canonical_heuristic();
  values.set_length(length());
  for (index_type k = 0; k < length(); k++)
    values[k] = (*this)[k]->mean();
  NTYPE v_max = NEG_INF;
  for (index_type k = 0; k < canonical_h.length(); k++) {
    NTYPE v_sum = 0;
    for (index_type i = 0; i < canonical_h[k].length(); i++)
      v_sum += values[canonical_h[k][i]];
    v_max = MAX(v_max, v_sum);
  }
  return v_max;
}

NTYPE PDBCollection::eval(const partial_state& s)
{
  if (update_flag) compute_canonical_heuristic();
  values.set_length(length());
  for (index_type k = 0; k < length(); k++)
    values[k] = (*this)[k]->eval(s);
  // std::cerr << "eval: values = " << values << std::endl;
  NTYPE v_max = NEG_INF;
  for (index_type k = 0; k < canonical_h.length(); k++) {
    NTYPE v_sum = 0;
    for (index_type i = 0; i < canonical_h[k].length(); i++)
      v_sum += values[canonical_h[k][i]];
    v_max = MAX(v_max, v_sum);
  }
  return v_max;
}

NTYPE PDBCollection::eval_to_bound(const partial_state& s, NTYPE b)
{
  if (update_flag) compute_canonical_heuristic();
  values.set_length(length());
  for (index_type k = 0; k < length(); k++) {
    values[k] = (*this)[k]->eval_to_bound(s, b);
    if (values[k] >= b) return values[k];
  }
  // std::cerr << "eval_to_bound: values = " << values << std::endl;
  NTYPE v_max = NEG_INF;
  for (index_type k = 0; k < canonical_h.length(); k++) {
    NTYPE v_sum = 0;
    for (index_type i = 0; i < canonical_h[k].length(); i++) {
      v_sum += values[canonical_h[k][i]];
      if (v_sum >= b) return v_sum;
    }
    v_max = MAX(v_max, v_sum);
  }
  return v_max;
}

void PDBCollection::write_collection(std::ostream& s)
{
  if (update_flag) compute_canonical_heuristic();
  s << "max(";
  for (index_type k = 0; k < canonical_h.length(); k++) {
    if (k > 0) s << ", ";
    for (index_type i = 0; i < canonical_h[k].length(); i++) {
      if (i > 0) s << " + ";
      instance.write_variable_set(s, (*this)[canonical_h[k][i]]->variables());
    }
  }
  s << ")";
}

void PDBCollection::write_PDB(std::ostream& s)
{
  for (index_type k = 0; k < length(); k++) {
    s << "#" << k << ": ";
    instance.write_variable_set(s, (*this)[k]->variables());
    s << std::endl;
    (*this)[k]->write(s);
  }
}

struct extension {
  index_type pattern;
  index_set  v_new;
  PDBCollection* h_ash;
  PDBCollection* col;
  index_type n_positive;
  index_type n_samples;
  double     c_upper;
  double     c_lower;
  double     wscore;
  double     sq_wscore;
  lvector<double> wscores;
  index_type s_count;
  double     p_upper;
  double     p_lower;

  extension()
    : pattern(no_such_index),
      h_ash(0),
      col(0),
      n_positive(0),
      n_samples(0),
      c_upper(0),
      c_lower(0),
      wscore(0),
      sq_wscore(0),
      s_count(0),
      p_upper(0),
      p_lower(0)
  { };

  void delete_own() {
    if (h_ash)
      delete h_ash;
    h_ash = 0;
    if (col)
      delete col;
    col = 0;
  };

  double counting_score() const {
    return (n_positive / (double)n_samples);
  };

  double weighted_score() const {
    return (wscore / (double)s_count);
  };

#ifdef USE_WEIGHTED_SCORE
  double score() const {
    return weighted_score();
  };

  double score_upper_bound() const {
    return p_upper;
  };

  double score_lower_bound() const {
    return p_lower;
  };
#else
  double score() const {
    return counting_score();
  };

  double score_upper_bound() const {
    return c_upper;
  };

  double score_lower_bound() const {
    return c_lower;
  };
#endif
};

std::ostream& operator<<(::std::ostream& s, const extension& ext)
{
  s << "<<" << ext.pattern << "," << ext.v_new << ">: "
    << ext.weighted_score()
#ifndef USE_MEAN_VALUE
    << ", " << ext.n_samples
    << ", [" << ext.p_upper << ", " << ext.p_lower << "]"
    << ", " << ext.counting_score()
    << ", [" << ext.c_upper << ", " << ext.c_lower << "]"
#endif
    << ">";
}

typedef lvector<extension> extension_vec;

class decreasing_score : public extension_vec::order {
public:
  virtual bool operator()(const extension& e0, const extension& e1) const {
    return (e0.score() > e1.score());
  };
};

void init_extension_vec
(Instance& ins,
 const ACF& cost,
 SASInstance& s_ins,
 const ACF& s_cost,
 MDDNode* sinc,
 Heuristic* inc,
 const index_set& vars,
 graph& cg,
 index_type pdb_size_limit,
 index_type total_size_limit,
 bool opt_fast,
 PDBCollection& col,
 extension_vec& extensions,
 index_type& s_out,
 Statistics& stats)
{
  for (index_type k = 0; k < col.length(); k++)
    for (index_type i = 0; i < vars.length(); i++) {
      bool go = true;
      if (col[k]->variables().contains(vars[i])) {
// 	std::cerr << "not considering add "
// 		  << s_ins.variables[vars[i]].name << " to ";
// 	s_ins.write_variable_set(std::cerr, col[k]->variables());
// 	std::cerr << " because variable already in pattern"
// 		  << std::endl;
	go = false;
      }
      if (go && !cg.adjacent(col[k]->variables(), vars[i])) {
// 	std::cerr << "not considering add "
// 		  << s_ins.variables[vars[i]].name << " to ";
// 	s_ins.write_variable_set(std::cerr, col[k]->variables());
// 	std::cerr << " because no direct connection in causal graph"
// 		  << std::endl;
	go = false;
      }
      if (go && (col[k]->variables().length() == 1)) {
	if (s_ins.goal_state.defines(vars[i]) &&
	    (vars[i] < col[k]->variables()[0])) {
// 	  std::cerr << "not considering add "
// 		    << s_ins.variables[vars[i]].name << " to ";
// 	  s_ins.write_variable_set(std::cerr, col[k]->variables());
// 	  std::cerr << " because of symmetric option"
// 		    << std::endl;
	  go = false;
	}
      }
      if (go) {
	index_set v_ext(col[k]->variables());
	v_ext.insert(vars[i]);
	index_type s_ext =
	  ProgressionPDBSize::apply(s_ins.signature, v_ext);
	if (col.find_PDB_with_pattern(v_ext)) {
// 	  std::cerr << "not considering add "
// 		    << s_ins.variables[vars[i]].name << " to ";
// 	  s_ins.write_variable_set(std::cerr, col[k]->variables());
// 	  std::cerr << " because pattern already in collection"
// 		    << std::endl;
	  go = false;
	}
	if ((s_ext > pdb_size_limit) ||
	    ((col.total_size() + s_ext) > total_size_limit)) {
// 	  std::cerr << "not considering add "
// 		    << s_ins.variables[vars[i]].name << " to ";
// 	  s_ins.write_variable_set(std::cerr, col[k]->variables());
// 	  std::cerr << " because resulting PDB/collection exceeds size limit"
// 		    << std::endl;
	  if ((s_out == no_such_index) || (s_ext < s_out))
	    s_out = s_ext;
	  go = false;
	}
	if (go) {
	  extension ext;
	  ext.pattern = k;
	  ext.v_new.assign_singleton(vars[i]);
#ifdef USE_MEAN_VALUE
	  ProgressionPDB* new_pdb =
	    new ProgressionPDB(s_ins, v_ext, s_cost, sinc, inc, stats);
	  new_pdb->compute2();
	  ext.v_mean = new_pdb->mean();
	  delete new_pdb;
	  ext.h_ash = 0;
	  ext.col = 0;
#else
	  if (opt_fast) {
	    ext.h_ash = 0;
	    ProgressionPDB* new_pdb =
	      new ProgressionPDB(s_ins, v_ext, s_cost, sinc, inc, stats);
	    new_pdb->compute2();
	    ext.col = new PDBCollection(col);
	    ext.col->addPDB(new_pdb, true);
	  }
	  else {
	    ext.h_ash = new PDBCollection(s_ins, s_cost, sinc, inc, stats);
	    for (index_type j = 0; j < col.length(); j++)
	      if (v_ext.contains(col[j]->variables()))
		ext.h_ash->addPDB(col[j], false);
	    assert(ext.h_ash->length() > 0);
	    ProgressionASH* new_pdb =
	      new ProgressionASH(s_ins, v_ext, s_cost, sinc, inc, *(ext.h_ash),
				 stats);
	    ext.col = new PDBCollection(col);
	    ext.col->addPDB(new_pdb, true);
	  }
#endif
	  extensions.append(ext);
	}
      }
      if (stats.break_signal_raised()) return;
    }
}


void init_extension_vec_2
(Instance& ins,
 const ACF& cost,
 SASInstance& s_ins,
 const ACF& s_cost,
 MDDNode* sinc,
 Heuristic* inc,
 const index_set& vars,
 graph& cg,
 index_type m_ext,
 index_type pdb_size_limit,
 index_type total_size_limit,
 PDBCollection& col,
 extension_vec& extensions,
 index_type& s_out,
 Statistics& stats)
{
  for (index_type k = 0; k < col.length(); k++) {
    index_set cand;
    for (index_type i = 0; i < vars.length(); i++) {
      bool go = true;
      if (col[k]->variables().contains(vars[i]))
	go = false;
      if (go && !cg.adjacent(col[k]->variables(), vars[i]))
	go = false;
      if (go)
	cand.insert(vars[i]);
    }
    if (cand.length() > m_ext) {
      mSubsetEnumerator e(cand.size(), m_ext);
      bool more = e.first();
      while (more) {
	index_set v_ext;
	e.current_set(v_ext);
	v_ext.insert(col[k]->variables());
	bool go = true;
	index_type s_ext =
	  ProgressionPDBSize::apply(s_ins.signature, v_ext);
	if (col.find_PDB_with_pattern(v_ext))
	  go = false;
	if ((s_ext > pdb_size_limit) ||
	    ((col.total_size() + s_ext) > total_size_limit)) {
	  if ((s_out == no_such_index) || (s_ext < s_out))
	    s_out = s_ext;
	  go = false;
	}
	if (go) {
	  extension ext;
	  ext.pattern = k;
	  e.current_set(ext.v_new);
	  ext.h_ash = new PDBCollection(s_ins, s_cost, sinc, inc, stats);
	  for (index_type j = 0; j < col.length(); j++)
	    if (v_ext.contains(col[j]->variables()))
	      ext.h_ash->addPDB(col[j], false);
	  assert(ext.h_ash->length() > 0);
	  ProgressionASH* new_pdb =
	    new ProgressionASH(s_ins, v_ext, s_cost, sinc, inc,
			       *(ext.h_ash), stats);
	  ext.col = new PDBCollection(col);
 	  ext.col->addPDB(new_pdb, true);
	  extensions.append(ext);
	}
	if (stats.break_signal_raised()) return;
	more = e.next();
      }
    }
  }
}

#ifndef USE_MEAN_VALUE

void eval_extension
(Instance& ins,
 const ACF& cost,
 SASInstance& s_ins,
 const ACF& s_cost,
 const index_set& vars,
 PDBCollection& col0,
 PDBCollection& col1,
 index_type n_samples,
 index_type avg_depth,
 index_type max_depth,
 NTYPE c_est,
 NTYPE b_est,
 RandomWalk2& rw,
 index_type& score,
 double& weighted_score,
 double& sq_weighted_score,
 Statistics& stats)
{
#ifndef USE_SAMPLED_MEAN_VALUE
  FwdSASHAdapter h0(ins, s_ins, col0);
#endif
  FwdSASHAdapter h1(ins, s_ins, col1);

  score = 0;
  weighted_score = 0;
  sq_weighted_score = 0;

  for (index_type i = 0; i < n_samples; i++) {
    if (stats.break_signal_raised()) return;
    // std::cerr << "sample: ";
    index_type d = rw.sample_bin(avg_depth, max_depth);
    // std::cerr << d << " -- ";
    // index_set s(rw.current_state(), ins.n_atoms());
    // std::cerr << s << " -- ";
#ifdef USE_SAMPLED_MEAN_VALUE
    NTYPE v1 = h1.eval(rw.current_state());
    if (INFINITE(v1)) {
      weighted_score += N_TO_D(MEAN_VALUE_INFINITY);
      sq_weighted_score += pow(N_TO_D(MEAN_VALUE_INFINITY), 2);
    }
    else {
      weighted_score += N_TO_D(v1);
      sq_weighted_score += pow(N_TO_D(v1), 2);
    }
#else
    NTYPE v0 = h0.eval(rw.current_state());
    // std::cerr << v0 << " -- ";
    NTYPE v_plus;
    if (FINITE(v0)) {
      v_plus = (h1.eval_to_bound(rw.current_state(), v0 + 1) - v0);
    }
    else {
      v_plus = 0;
    }
    // std::cerr << (v_plus > 0) << std::endl;
    if (v_plus > 0) {
      score += 1;
      double x = pow(N_TO_D(b_est), N_TO_D(MAX(c_est - v0, 0)));
      weighted_score += x;
      sq_weighted_score += (x * x);
    }
#endif
  }
}

double z_value[120] =
//     1      2      3      4      5      6      7      8      9     10
  {12.71, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228,
//    11     12     13     14     15     16     17     18     19     20
   2.201, 2.179, 2.160, 2.145, 2.131, 2.120, 2.110, 2.101, 2.093, 2.086,
//    21     22     23     24     25     26     27     28     29     30
   2.080, 2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042,
//    31     32     33     34     35     36     37     38     39     40
   2.040, 2.038, 2.036, 2.034, 2.032, 2.030, 2.028, 2.026, 2.024, 2.021,
//    41     42     43     44     45     46     47     48     49     50
   2.019, 2.018, 2.017, 2.016, 2.015, 2.014, 2.013, 2.012, 2.011, 2.009,
//    51     52     53     54     55     56     57     58     59     60
   2.009, 2.008, 2.007, 2.006, 2.005, 2.004, 2.003, 2.002, 2.001, 2.000,
//    61     62     63     64     65     66     67     68     69     70
   1.999, 1.999, 1.998, 1.998, 1.997, 1.997, 1.996, 1.996, 1.995, 1.995,
//    71     72     73     74     75     76     77     78     79     80
   1.994, 1.994, 1.993, 1.993, 1.992, 1.992, 1.991, 1.991, 1.990, 1.990,
//    81     82     83     84     85     86     87     88     89     90
   1.990, 1.990, 1.989, 1.989, 1.989, 1.988, 1.988, 1.988, 1.987, 1.987,
//    91     92     93     94     95     96     97     98     99    100
   1.987, 1.986, 1.986, 1.986, 1.985, 1.985, 1.985, 1.984, 1.984, 1.984,
//   101    102    103    104    105    106    107    108    109    110
   1.984, 1.984, 1.984, 1.983, 1.983, 1.983, 1.983, 1.983, 1.982, 1.982,
//   111    112    113    114    115    116    117    118    119    120
   1.982, 1.982, 1.982, 1.981, 1.981, 1.981, 1.981, 1.981, 1.980, 1.980};

comparable_pair<double> calculate_normal_ci
(double sum_v, double ssq_v, index_type n)
{
  assert(n > 1);
  double mean = sum_v / ((double) n);
  // sum (xi - mean)^2 = sum xi^2 + n * mean^2 - 2*mean * sum xi
  double ssq_d = ssq_v + (n * pow(mean, 2)) - (2 * mean * sum_v);
  double s2 = ssq_d / (double)(n - 1);
  double t = sqrt(s2) / sqrt((double)(n - 1));
  // get value for n - 1; -2 because table is zero indexed
  double z (n > 120 ? 1.96 : z_value[n - 2]);
  return comparable_pair<double>(mean + (z * t), mean - (z * t));
}

void eval_extension_vec
(Instance& ins,
 const ACF& cost,
 SASInstance& s_ins,
 const ACF& s_cost,
 const index_set& vars,
 PDBCollection& col,
 extension_vec& exts,
 index_type n_samples,
 index_type avg_depth,
 index_type max_depth,
 NTYPE c_est,
 NTYPE b_est,
 RandomWalk2& rw,
 Statistics& stats)
{
  index_type c_score = 0;
  double w_score = 0;
  double sq_w_score = 0;
  for (index_type k = 0; k < exts.length(); k++) {
    // std::cerr << "evaluating " << exts[k] << "..." << std::endl;
    eval_extension(ins, cost, s_ins, s_cost, vars, col, *(exts[k].col),
		   n_samples, avg_depth, max_depth, c_est, b_est, rw,
		   c_score, w_score, sq_w_score, stats);
    exts[k].n_positive += c_score;
    exts[k].n_samples += n_samples;

    // calculate confidence interval from n_positive and n_samples:
    // for 95% ci, use z = 1.96; for 98% ci, use z = 2.33
    double z = 1.96;

    // Wilson's formula:
    double mean = exts[k].counting_score();
    double a = ((1 / (2 * (double)exts[k].n_samples)) * z * z);
    double b = ((1 / (4 * (double)exts[k].n_samples)) * z * z);
    double c = (((mean * (1 - mean)) + b) / ((double)exts[k].n_samples));
    double d = ((1 / (double)exts[k].n_samples) * z * z);
    exts[k].c_upper = (mean + a + (z * sqrt(c))) / d;
    exts[k].c_lower = (mean + a - (z * sqrt(c))) / d;

#ifdef USE_WEIGHTED_SCORE
    exts[k].wscore += (w_score / n_samples);
    exts[k].wscores.append(w_score / n_samples);
    exts[k].sq_wscore += pow(w_score / n_samples, 2);
    exts[k].s_count += 1;

    if (exts[k].s_count > 1) {
      comparable_pair<double> ci =
	calculate_normal_ci(exts[k].wscore, exts[k].sq_wscore,
			    exts[k].s_count);
      exts[k].p_upper = ci.first;
      exts[k].p_lower = ci.second;
    }
    else {
      exts[k].p_upper = D_INF;
      exts[k].p_lower = -1*D_INF;
    }
#endif
  }
}

void prune_extension_vec
(extension_vec& exts, double i_min, double& i_out)
{
// std::cerr << "extensions before pruning: " << exts << std::endl;
  index_type i = 0;
  while (i < exts.length()) {
    bool is_dom = (exts[i].c_upper < i_min);
    for (index_type j = 0; (j < exts.length()) && !is_dom; j++)
      if (exts[j].score_lower_bound() > exts[i].score_upper_bound()) {
// 	std::cerr << "ext. <" << exts[j].pattern << "," << exts[j].v_new
// 		  << "> with p = " << exts[j].n_positive << "/"
// 		  << exts[j].n_samples << " [" << exts[j].p_upper
// 		  << ", " << exts[j].p_lower
// 		  << "] dominates "
// 		  << "ext. <" << exts[i].pattern << "," << exts[i].v_new
// 		  << "> with p = " << exts[i].n_positive << "/"
// 		  << exts[i].n_samples << " [" << exts[i].p_upper
// 		  << ", " << exts[i].p_lower << "]"
// 		  << std::endl;
	is_dom = true;
      }
    if (is_dom) {
      if (exts[i].counting_score() > i_out)
	i_out = exts[i].counting_score();
      exts[i].delete_own();
      exts.remove(i);
    }
    else {
      i += 1;
    }
  }
}

#endif // !USE_MEAN_VALUE

void clear_extension_vec
(extension_vec& exts)
{
  for (index_type k = 0; k < exts.length(); k++)
    exts[k].delete_own();
  exts.clear();
}

PDBCollection* build_collection
(Instance& ins,
 const ACF& cost,
 SASInstance& s_ins,
 const ACF& s_cost,
 MDDNode* sinc,
 Heuristic* inc,
 const index_set& vars,
 index_type pdb_size_limit,
 index_type total_size_limit,
 index_type d_skip,
 index_type s_max,
 double     i_min,
 index_type n_trials,
 index_type n_samples,
 bool       opt_fast,
 double&    i_out,
 index_type& s_out,
 RNG& rng,
 Statistics& stats)
{
  PDBCollection* col = new PDBCollection(s_ins, s_cost, sinc, inc, stats);
  for (index_type k = 0; k < vars.length(); k++) {
    if (s_ins.goal_state.defines(vars[k])) {
      index_set g;
      g.assign_singleton(vars[k]);
      std::cerr << "adding PDB for variable "
		<< s_ins.variables[vars[k]].name
		<< " with " << s_ins.variables[vars[k]].n_values()
		<< " values...";
      col->addProgressionPDB(g, opt_fast);
      std::cerr << "done" << std::endl;
    }
  }

  std::cerr << "initial collection: ";
  col->write_collection(std::cerr);
  std::cerr << std::endl
	    << "initial h = " << col->eval(s_ins.init_state)
	    << std::endl;

  graph xcg;
  if (sinc) {
    assert(inc);
    s_ins.compute_extended_causal_graph(xcg, *inc);
  }
  else {
    xcg.copy(s_ins.causal_graph);
  }

  RandomWalk2 rw(ins, cost, rng);
  rw.set_parameters(true, true);

  PreconditionEvaluator* pe =
    PreconditionEvaluator::construct(ins, R_TO_N(1, 2));
  rw.set_evaluator(pe);

  index_set p_atoms;
  index_set n_atoms;
  for (index_type k = 0; k < col->length(); k++) {
    PartialStateEnumerator e((*col)[k]->variables(), s_ins.signature);
    bool more = e.first();
    while (more) {
      if (INFINITE((*col)[k]->eval(e.current_state()))) {
	s_ins.make_dual_atom_sets(e.current_state(), p_atoms, n_atoms);
	std::cerr << "state ";
	s_ins.write_partial_state(std::cerr, e.current_state());
	std::cerr << " is a deadlock (atoms ";
	ins.write_atom_set(std::cerr, p_atoms);
	std::cerr << " and none of ";
	ins.write_atom_set(std::cerr, n_atoms);
	std::cerr << ")" << std::endl;
	for (index_type i = 0; i < ins.n_actions(); i++)
	  if (ins.actions[i].add.contains(p_atoms) &&
	      ins.actions[i].e_deletes(n_atoms, inc)) {
	    std::cerr << "action " << ins.actions[i].name
		      << " causes deadlock!"
		      << std::endl;
	    rw.disallow_action(i);
	  }
      }
      more = e.next();
    }
  }
  std::cerr << rw.n_allowed_actions() << " allowed actions" << std::endl;

  NTYPE b_est = 1;
  NTYPE c_to_d = s_cost.avg_cost(s_ins.n_actions());
  NTYPE c_est = col->eval(s_ins.init_state);
  NTYPE d_est = (c_est / c_to_d);
  assert(d_est >= 0);
  index_type avg_depth = (2 * FLOOR_TO_INT(d_est)) + 1;
  if (avg_depth < 20) avg_depth = 20;
  index_type max_depth = avg_depth * 4;
  NTYPE c_max = col->eval(s_ins.init_state) * 4;
  std::cerr << "initial estimates: cost = " << c_est
	    << ", cost/depth = " << c_to_d
	    << ", depth = " << d_est
	    << " (rw E(depth) = " << avg_depth
	    << ", max depth = " << max_depth << ")"
	    << std::endl;

  // perform some random walks to initialize estimates of branching
  // factor and cost-to-depth
  for (index_type k = 0; k < 2 * n_samples; k++)
    rw.sample_bin(avg_depth, max_depth);

#ifdef USE_SAMPLED_MEAN_VALUE
  // estimate mean value of current collection
  index_type c_score = 0;
  double w_score = 0;
  double sq_w_score = 0;
  eval_extension(ins, cost, s_ins, s_cost, vars, *col, *col,
		 n_samples, avg_depth, max_depth, c_est, b_est, rw,
		 c_score, w_score, sq_w_score, stats);
  double current_mean_value = (w_score / n_samples);
#endif

  extension_vec exts;
  extension_vec best;
  decreasing_score so;
  index_type l = 0;
  bool done = false;

  while (!done && (l < s_max)) {
    done = true;

    std::cerr << l << ". current pattern collection: ";
    col->write_collection(std::cerr);
    std::cerr << ", total size = " << col->total_size()
	      << " (" << stats << ")"
	      << std::endl;

    // update estimates
    b_est = rw.estimated_branching_factor();
    // NOTE: we do not update c_to_d ratio, because estimation is broken!
    // c_to_d = rw.estimated_cost_to_depth();
    c_est = col->eval(s_ins.init_state);
    d_est = (c_est / c_to_d);
    avg_depth = (2 * FLOOR_TO_INT(d_est)) + 1;
    max_depth = avg_depth * 4;
    if (avg_depth < 20) avg_depth = 20;
    c_max = col->eval(s_ins.init_state) * 4;
    std::cerr << "current estimates: cost = " << c_est
	      << ", cost/depth = " << c_to_d
	      << " (" << rw.estimated_cost_to_depth() << ")"
	      << ", depth = " << d_est
	      << " (rw E(depth) = " << avg_depth
	      << ", max depth = " << max_depth
	      << ", bf = " << b_est
	      << ", df = " << rw.estimated_deadend_frequency()
#ifdef USE_SAMPLED_MEAN_VALUE
	      << ", mean value = " << current_mean_value
#endif
	      << std::endl;

    // evaluate possible extensions
    exts.clear();
    // std::cerr << "initializing..." << std::endl;
    s_out = no_such_index;
    i_out = -1*D_INF;
    init_extension_vec(ins, cost, s_ins, s_cost, sinc, inc, vars, xcg,
		       pdb_size_limit, total_size_limit, opt_fast, *col,
		       exts, s_out, stats);
    if (stats.break_signal_raised()) return col;
#ifndef USE_MEAN_VALUE
    index_type t = 0;
    while ((exts.length() > d_skip) && (t < n_trials)) {
      std::cerr << "trial #" << t << ", " << exts.length()
		<< " extensions" << std::endl;
      // std::cerr << "evaluating..." << std::endl;
      eval_extension_vec(ins, cost, s_ins, s_cost, vars, *col, exts,
			 n_samples, avg_depth, max_depth, c_max,
			 rw.estimated_branching_factor(), rw, stats);
      if (stats.break_signal_raised()) return col;
#ifdef USE_CI_PRUNING
      // std::cerr << "pruning..." << std::endl;
      prune_extension_vec(exts, i_min, i_out);
      if (stats.break_signal_raised()) return col;
#endif
      t += 1;
    }
#endif

#ifdef USE_LARGER_EXTENSIONS
    bool any_good = false;
    for (index_type k = 0; (k < exts.length()) && !any_good; k++)
      if (exts[k].counting_score() > i_min)
	any_good = true;
    if (!any_good) {
      std::cerr << "no useful 1-variable extensions, trying 2..."
		<< std::endl;
      init_extension_vec_2(ins, cost, s_ins, s_cost, sinc, inc, vars, xcg, 2,
			   pdb_size_limit, total_size_limit, *col, exts, s_out,
			   stats);
      if (stats.break_signal_raised()) return col;
      index_type t = 0;
      while ((exts.length() > d_skip) && (t < n_trials)) {
	std::cerr << "trial #" << t << ", " << exts.length()
		  << " extensions" << std::endl;
	// std::cerr << "evaluating..." << std::endl;
	eval_extension_vec(ins, cost, s_ins, s_cost, vars, *col, exts,
			   n_samples, avg_depth, max_depth, c_max,
			   rw.estimated_branching_factor(), rw, stats);
#ifdef USE_CI_PRUNING
	// std::cerr << "pruning..." << std::endl;
	prune_extension_vec(exts, i_min, i_out);
#endif
	if (stats.break_signal_raised()) return col;
	t += 1;
      }
    }
#endif

    best.clear();
    for (index_type k = 0; k < exts.length(); k++) {
#ifdef USE_SAMPLED_MEAN_VALUE
      if ((exts[k].score() - current_mean_value) > i_min)
#else
      if (exts[k].counting_score() > i_min)
#endif
	best.insert_ordered(exts[k], so);
      else
#ifdef USE_SAMPLED_MEAN_VALUE
	if ((exts[k].score() - current_mean_value) > i_out)
#else
	if (exts[k].counting_score() > i_out)
#endif
	  i_out = exts[k].counting_score();
    }

    std::cerr << "best options:" << std::endl;
    for (index_type i = 0; i < best.length(); i++) {
      index_type p = best[i].pattern;
      index_set& v = best[i].v_new;
      std::cerr << " add ";
      s_ins.write_variable_set(std::cerr, v);
      std::cerr << " to ";
      s_ins.write_variable_set(std::cerr, (*col)[p]->variables());
      std::cerr << " (" << best[i].counting_score()
		<< ", " << best[i].weighted_score() << ")" << std::endl;
    }
    std::cerr << "skipped options: best improvement = " << i_out
	      << ", min size = " << s_out << std::endl;

    // pick one or more...
    index_type n = 0;
    while ((n < d_skip) && (n < best.length())) {
      index_type p = best[n].pattern;
      index_set& v = best[n].v_new;
      index_set  p_ext((*col)[p]->variables());
      p_ext.insert(v);
      if (col->find_PDB_with_pattern(p_ext) != 0) {
	std::cerr << "BAD! new pattern ";
	s_ins.write_variable_set(std::cerr, p_ext);
	std::cerr << " already in collection ";
	col->write_collection(std::cerr);
	std::cerr << std::endl;
	exit(255);
      }
      std::cerr << "adding new pattern ";
      s_ins.write_variable_set(std::cerr, p_ext);
      std::cerr << "...";
      double t0 = stats.time();
      col->addProgressionPDB(p_ext, opt_fast);
      std::cerr << " PDB computed in " << stats.time() - t0
		<< " seconds" << std::endl;
      if (stats.break_signal_raised()) return col;
      done = false;
      n += 1;
    }

#ifdef USE_SAMPLED_MEAN_VALUE
    if (!done && (best.length() > 0)) {
      current_mean_value = best[0].score();
    }
#endif

//     if (done) {
//       index_type n = remove_dominated();
//       if (n > 0) done = false;
//     }
    l += 1;
  }

  return col;
}

// void examine_extensions
// (Instance& ins,
//  const ACF& cost,
//  SASInstance& s_ins,
//  const ACF& s_cost,
//  MDDNode* sinc,
//  Heuristic* inc,
//  const index_set& vars,
//  graph& cg,
//  index_type total_size_limit,
//  PDBCollection& col,
//  index_type n_best,
//  index_type n_samples_to_eval,
//  index_type avg_sample_depth,
//  RandomWalk2& rw,
//  weighted_pair_vec& best,
//  Statistics& stats)
// {
//   best.clear();
//   for (index_type k = 0; k < col.length(); k++)
//     for (index_type i = 0; i < vars.length(); i++) {
//       bool go = true;
//       if (col[k]->variables().contains(vars[i])) {
// // 	std::cerr << "not considering add "
// // 		  << s_ins.variables[vars[i]].name << " to ";
// // 	s_ins.write_variable_set(std::cerr, col[k]->variables());
// // 	std::cerr << " because variable already in pattern"
// // 		  << std::endl;
// 	go = false;
//       }
//       if (go && !cg.adjacent(col[k]->variables(), vars[i])) {
// // 	std::cerr << "not considering add "
// // 		  << s_ins.variables[vars[i]].name << " to ";
// // 	s_ins.write_variable_set(std::cerr, col[k]->variables());
// // 	std::cerr << " because no direct connection in causal graph"
// // 		  << std::endl;
// 	go = false;
//       }
//       if (go && (col[k]->variables().length() == 1)) {
// 	if (s_ins.goal_state.defines(vars[i]) &&
// 	    (vars[i] < col[k]->variables()[0])) {
// // 	  std::cerr << "not considering add "
// // 		    << s_ins.variables[vars[i]].name << " to ";
// // 	  s_ins.write_variable_set(std::cerr, col[k]->variables());
// // 	  std::cerr << " because of symmetric option"
// // 		    << std::endl;
// 	  go = false;
// 	}
//       }
//       if (go) {
// 	index_set v_ext(col[k]->variables());
// 	v_ext.insert(vars[i]);
// 	index_type s_ext =
// 	  ProgressionPDBSize::apply(s_ins.signature, v_ext);
// 	if (col.find_PDB_with_pattern(v_ext)) {
// // 	  std::cerr << "not considering add "
// // 		    << s_ins.variables[vars[i]].name << " to ";
// // 	  s_ins.write_variable_set(std::cerr, col[k]->variables());
// // 	  std::cerr << " because pattern already in collection"
// // 		    << std::endl;
// 	  go = false;
// 	}
// 	if ((col.total_size() + s_ext) > total_size_limit) {
// // 	  std::cerr << "not considering add "
// // 		    << s_ins.variables[vars[i]].name << " to ";
// // 	  s_ins.write_variable_set(std::cerr, col[k]->variables());
// // 	  std::cerr << " because resulting collection exceeds size limit"
// // 		    << std::endl;
// 	  go = false;
// 	}
// 	if (go) {
// 	  PDBCollection* h_ash =
// 	    new PDBCollection(s_ins, s_cost, sinc, inc, stats);
// 	  for (index_type j = 0; j < col.length(); j++)
// 	    if (v_ext.contains(col[j]->variables()))
// 	      h_ash->addPDB(col[j], false);
// 	  assert(h_ash->length() > 0);
// 	  ProgressionASH* new_pdb =
// 	    new ProgressionASH(s_ins, v_ext, s_cost, sinc, inc, *h_ash, stats);
// 	  PDBCollection* new_col = new PDBCollection(col);
//  	  new_col->addPDB(new_pdb, true);
// 	  index_type n_ext;
// 	  NTYPE mv_ext;
// 	  eval_extension(ins, cost, s_ins, s_cost, vars, col, *new_col,
// 			 n_samples_to_eval, avg_sample_depth, rw,
// 			 n_ext, mv_ext, stats);
// 	  delete h_ash;
// 	  delete new_col;
// // 	  std::cerr << "evaluated add " << s_ins.variables[vars[i]].name
// // 		    << " to ";
// // 	  s_ins.write_variable_set(std::cerr, col[k]->variables());
// // 	  std::cerr << ": " << n_ext << ", " << mv_ext
// // 		    << ", " << (n_ext > 0 ? n_ext * (1 / mv_ext) : 0)
// // 		    << std::endl;
// 	  if (n_ext > 0) {
// 	    NTYPE val = n_ext;
// 	    // NTYPE val = (n_ext * (1 / mv_ext));
// 	    best.insert_ordered(weighted<index_pair>(index_pair(k, i), val),
// 				best.decreasing);
// 	    if (best.length() > n_best) best.set_length(n_best);
// 	  }
// 	}
//       }
//     }
// }

#ifdef _DO_NOT_COMPILE_

void test_new_pattern_collection_class(
  const SASInstance &sas_instance,
  const abstraction_heuristic_vec &abst_h_vec)
{
  std::cerr << std::endl
            << "We interrupt this broadcast for the following message:"
            << std::endl;
  PatternCollection collection(sas_instance);
  for (index_type i = 0; i < abst_h_vec.length(); i++)
    collection.add_pattern(abst_h_vec[i]->variables());
  std::cerr << "pattern collection:" << std::endl;
  collection.write(std::cerr);
  std::cerr << "canonical heuristic function:" << std::endl;
  collection.write_canonical_heuristic(std::cerr);
  std::cerr << "Thank you for your attention and goodbye."
	    << std::endl << std::endl;
}

#endif

END_HSPS_NAMESPACE
