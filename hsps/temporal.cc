
#include "temporal.h"

BEGIN_HSPS_NAMESPACE

bool TemporalRegState::implicit_noop = true;
bool TemporalRegState::subgoal_ordering = true;
bool TemporalRegState::strict_right_shift = false;

// #define TRACE_PRINT_LOTS
// #define CHECK_INCREMENTAL_EVAL

TemporalRegState::TemporalRegState
(Instance& i, Heuristic& h)
  : AtomSet(i),
    heuristic(h),
    acts(index_cost_pair(no_such_index, 0), 0),
    res(0),
    delta(0),
    est(0),
    final(true)
{
  // done
}

TemporalRegState::TemporalRegState
(Instance& i, Heuristic& h, RegressionResourceState* r)
  : AtomSet(i),
    heuristic(h),
    acts(index_cost_pair(no_such_index, 0), 0),
    res(r),
    delta(0),
    est(0),
    final(true)
{
  // done
}

TemporalRegState::TemporalRegState
(Instance& i, Heuristic& h, const index_set& g)
  : AtomSet(i, g),
    heuristic(h),
    acts(index_cost_pair(no_such_index, 0), 0),
    res(0),
    delta(0),
    est(0),
    final(false)
{
  est = heuristic.eval(set);
  final = is_init_all();
}

TemporalRegState::TemporalRegState
(Instance& i, Heuristic& h, const bool_vec& g)
  : AtomSet(i, g),
    heuristic(h),
    acts(index_cost_pair(no_such_index, 0), 0),
    res(0),
    delta(0),
    est(0),
    final(false)
{
  est = heuristic.eval(set);
  final = is_init_all();
}

TemporalRegState::TemporalRegState
(Instance& i, Heuristic& h, const index_set& g, RegressionResourceState* r)
  : AtomSet(i, g),
    heuristic(h),
    acts(index_cost_pair(no_such_index, 0), 0),
    res(r),
    delta(0),
    est(0),
    final(false)
{
  est = heuristic.eval(set);
  final = is_init_all();
}

TemporalRegState::TemporalRegState
(Instance& i, Heuristic& h, const bool_vec& g, RegressionResourceState* r)
  : AtomSet(i, g),
    heuristic(h),
    acts(index_cost_pair(no_such_index, 0), 0),
    res(r),
    delta(0),
    est(0),
    final(false)
{
  est = heuristic.eval(set);
  final = is_init_all();
}

TemporalRegState::TemporalRegState
(const TemporalRegState& s)
  : AtomSet(s),
    State(s),
    heuristic(s.heuristic),
    acts(s.acts),
    res(0),
    delta(s.delta),
    est(s.est),
    final(s.final),
    starts(s.starts),
    noops(s.noops)
{
  if (s.res) res = s.res->copy();
}

TemporalRegState::TemporalRegState
(Instance& i, Heuristic& h, const index_cost_vec& se_acts,
 const index_vec& se_noops, RegressionResourceState* r)
  : AtomSet(i),
    heuristic(h),
    acts(index_cost_pair(no_such_index, 0), 0),
    res(0),
    delta(0),
    est(0),
    final(true)
{
  // add goals for selected noops
  for (index_type k = 0; k < se_noops.length(); k++) {
    set[se_noops[k]] = true;
    noops.insert(se_noops[k]);
  }

  if (r) {
    // copy the resource state
    res = r->copy();
    // and note the simultaneous requirements of all actions in se_acts
    amt_vec cr(0, instance.n_resources());
    for (index_type i = 0; i < instance.n_resources(); i++) {
      cr[i] = 0;
      for (index_type k = 0; k < se_acts.length(); k++)
	cr[i] += instance.actions[se_acts[k].first].req(i);
    }
    res->reserve_as_required(cr);
  }

  if (se_acts.length() > 0) {
    // delta is the smallest delta among selected actions
    delta = se_acts[0].second;
    index_type k = 0;

    // add goals for precs of selected actions starting at delta
    while ((k < se_acts.length()) && (se_acts[k].second <= delta)) {
      Instance::Action& act = instance.actions[se_acts[k].first];
      for (index_type i = 0; i < act.pre.length(); i++)	set[act.pre[i]] = true;
      starts.insert(act.index);
      // deduct resources consumed by outgoing actions (since these will no
      // longer be in the set of current actions, and thus not counted when
      // simultaneous resource requirements are checked)
      if (res) res->apply(act);
      k += 1;
    }

    // set size (# of goals)
    count();

    // and add remaining actions to acts
    while (k < se_acts.length()) {
      acts.append(index_cost_pair(se_acts[k].first, se_acts[k].second - delta));
      k += 1;
    }
  }

  // compute estimated cost and final flag
  index_set s;
  for (index_type k = acts.length(); k > 0; k--) {
    Instance::Action& act = instance.actions[acts[k - 1].first];
    for (index_type i = 0; i < act.pre.length(); i++) {
      if (!instance.atoms[act.pre[i]].init) final = false;
#ifdef SUPPORT_VOLATILE_ATOMS
      // if this action has a volatile precondition and its delta
      // value is not maximal in the state, the state can not be
      // final (since that would imply persisting the precondition
      // from the initial world state)
      if ((acts[k].second < acts[acts.length() - 1]) &&
	  instance.atoms[act.pre[i]].volatile)
	final = false;
#endif
      s.insert(act.pre[i]);
    }
#ifdef APPLY_NCW_NOOP_TRICK
    // if goal atom i is n.c.w. with action act, it can only be noop:ed
    // up to the point where act begins
    for (index_type i = 0; i < instance.n_atoms(); i++) if (set[i]) {
      if (!s.contains(i) && act.ncw_atms.contains(i)) s.insert(i);
    }
#endif
    est = MAX(heuristic.eval(s) + acts[k - 1].second, est);
  }
  for (index_type i = 0; i < instance.n_atoms(); i++) if (set[i]) {
    if (!instance.atoms[i].init) final = false;
    s.insert(i);
  }
  est = MAX(heuristic.eval(s), est);

  // extra check of consumable resources (at this point, resource effects
  // of selected actions have been _applied_ to res).
  if (res) {
    if (!res->sufficient_consumable(s)) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "constructed successor state " << *this
		<< " has insufficient resources"
		<< std::endl;
#endif
      est = POS_INF;
      final = false;
    }
  }

  // if the state is final, move delta to cover all actions
  if (final && (se_acts.length() > 0)) {
//     std::cerr << "final state: " << *this << std::endl;
//     std::cerr << "delta cost (before adjustment) = " << delta << std::endl;
//     std::cerr << "selected actions (" << se_acts.length() << "): ";
//     write_index_cost_vec(se_acts);
//     std::cerr << std::endl;
    NTYPE max_d = se_acts[se_acts.length() - 1].second;
    delta = max_d;
    for (index_type i = 0; i < acts.length(); i++) {
      starts.insert(acts[i].first);
      if (res) res->apply(instance.actions[acts[i].first]);
    }
    for (index_type i = 0; i < instance.n_atoms(); i++) if (set[i])
      noops.insert(i);
    est = 0;
//     std::cerr << "state (after adjustment): " << *this << std::endl;
//     std::cerr << "delta cost (after adjustment) = " << delta << std::endl;
  }

#ifdef TRACE_PRINT_LOTS
  std::cerr << "new state (" << this << "): " << *this
	    << ", delta = " << delta
	    << ", est. cost = " << est
	    << ", f = " << delta + est
	    << ", final = " << final
	    << std::endl;
#endif
}

TemporalRegState::TemporalRegState
(Instance& i, Heuristic& h, const index_set& g, const index_set& a,
 RegressionResourceState* r)
  : AtomSet(i, g),
    heuristic(h),
    acts(index_cost_pair(no_such_index, 0), 0),
    res(0),
    delta(0),
    est(0),
    final(true)
{
  for (index_type k = 0; k < a.length(); k++)
    insert_action(instance.actions[a[k]], acts);

  if (r) {
    res = r->copy();
    // note simultaneous requirements of all actions
    amt_vec cr(0, instance.n_resources());
    for (index_type i = 0; i < instance.n_resources(); i++) {
      cr[i] = 0;
      for (index_type k = 0; k < acts.length(); k++)
	cr[i] += instance.actions[acts[k].first].req(i);
    }
    res->reserve_as_required(cr);
  }

  // compute estimated cost and final flag
  index_set s;
  for (index_type k = acts.length(); k > 0; k--) {
    Instance::Action& act = instance.actions[acts[k - 1].first];
    for (index_type i = 0; i < act.pre.length(); i++) {
      if (!instance.atoms[act.pre[i]].init) final = false;
#ifdef SUPPORT_VOLATILE_ATOMS
      // if this action has a volatile precondition and its delta
      // value is not maximal in the state, the state can not be
      // final (since that would imply persisting the precondition
      // from the initial world state)
      if ((acts[k].second < acts[acts.length() - 1]) &&
	  instance.atoms[act.pre[i]].volatile)
	final = false;
#endif
      s.insert(act.pre[i]);
    }
#ifdef APPLY_NCW_NOOP_TRICK
    // if goal atom i is n.c.w. with action act, it can only be noop:ed
    // up to the point where act begins
    for (index_type i = 0; i < instance.n_atoms(); i++) if (set[i]) {
      if (!s.contains(i) && act.ncw_atms.contains(i)) s.insert(i);
    }
#endif
    est = MAX(heuristic.eval(s) + acts[k - 1].second, est);
  }
  for (index_type i = 0; i < instance.n_atoms(); i++) if (set[i]) {
    if (!instance.atoms[i].init) final = false;
    s.insert(i);
  }
  est = MAX(heuristic.eval(s), est);

  // check consumable resources
  if (res) {
    if (!res->sufficient_consumable(s)) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "constructed state " << *this
		<< " has insufficient resources"
		<< std::endl;
#endif
      est = POS_INF;
      final = false;
    }
  }

  // if the state is final, move delta to cover all actions
  if (final && (acts.length() > 0)) {
    NTYPE max_d = acts[acts.length() - 1].second;
    delta = max_d;
    for (index_type i = 0; i < acts.length(); i++) {
      starts.insert(acts[i].first);
      if (res) res->apply(instance.actions[acts[i].first]);
    }
    for (index_type i = 0; i < instance.n_atoms(); i++) if (set[i])
      noops.insert(i);
    est = 0;
  }
}

TemporalRegState::~TemporalRegState()
{
  if (res) delete res;
}

NTYPE TemporalRegState::eval
(index_cost_vec& se_acts, index_vec& se_noops, RegressionResourceState* r)
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "entering eval" << std::endl << " se_acts: ";
  write_action_vec(se_acts);
  std::cerr << std::endl << " se_noops: ";
  instance.write_atom_set(std::cerr, se_noops);
  std::cerr << std::endl;
#endif
  NTYPE val = 0;
  index_set s;
  for (index_type k = se_acts.length(); k > 0; k--) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "next action is "
 	      << instance.actions[se_acts[k - 1].first].name
 	      << ", pre = ";
    instance.write_atom_set(std::cerr,
 			    instance.actions[se_acts[k - 1].first].pre);
    std::cerr << std::endl;
#endif
    s.insert(instance.actions[se_acts[k - 1].first].pre);
#ifdef APPLY_NCW_NOOP_TRICK
    for (index_type i = 0; i < se_noops.length(); i++)
      if (instance.actions[se_acts[k - 1].first].ncw_atms.
	  contains(se_noops[i])) {
#ifdef TRACE_PRINT_LOTS
	std::cerr << "adding NCW noop "
		  << instance.atoms[se_noops[i]].name
		  << std::endl;
#endif
	s.insert(se_noops[i]);
      }
#endif
#ifdef TRACE_PRINT_LOTS
    std::cerr << "H(";
    instance.write_atom_set(std::cerr, s);
    std::cerr << ") + delta = "
	      << heuristic.eval(s) << " + "
	      << se_acts[k - 1].second << " = "
	      << heuristic.eval(s) + se_acts[k - 1].second
	      << std::endl;
#endif
    val = MAX(heuristic.eval(s) + se_acts[k - 1].second, val);
#ifdef TRACE_PRINT_LOTS
    std::cerr << "max = " << val << std::endl;
#endif
  }
  if (!se_noops.empty()) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "adding noops" << std::endl;
    s.insert(se_noops);
    std::cerr << "s = ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << std::endl;
#endif
#ifdef APPLY_D_MIN_TRICK
    NTYPE d_min = instance.min_dur;
    if (se_acts.length() > 0) d_min = MIN(d_min, se_acts[0].second);
#ifdef TRACE_PRINT_LOTS
    std::cerr << "H(s) + d_min = " << heuristic.eval(s)
	      << " + " << d_min
	      << " = " << heuristic.eval(s) + d_min
	      << std::endl;
#endif
    val = MAX(heuristic.eval(s) + d_min, val);
#else
    val = MAX(heuristic.eval(s), val);
#endif
  }
#ifdef TRACE_PRINT_LOTS
  std::cerr << "leaving eval, max = " << val << std::endl;
#endif
  if (r) {
    amt_vec cr(0, instance.n_resources());
    for (index_type i = 0; i < instance.n_resources(); i++) {
      cr[i] = 0;
      for (index_type k = 0; k < se_acts.length(); k++)
	cr[i] += instance.actions[se_acts[k].first].req(i);
    }
    if (!r->sufficient_consumable(s, se_acts)) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "insufficient resources: r = ";
      r->write(std::cerr);
      std::cerr << ", cr = ";
      cr.write(std::cerr, instance.n_resources());
      std::cerr << std::endl;
#endif
      return POS_INF;
    }
  }
  return val;
}

NTYPE TemporalRegState::eval_se_noop
(index_cost_vec& se_acts, index_vec& se_noops, RegressionResourceState* r,
 Instance::Atom& new_noop, NTYPE c_prev)
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "entering eval_se_noop" << std::endl << " se_acts: ";
  write_action_vec(se_acts);
  std::cerr << std::endl << " se_noops: ";
  instance.write_atom_set(std::cerr, se_noops);
  std::cerr << std::endl << " new_noop: " << new_noop.name << std::endl
 	    << " c_prev = " << c_prev << std::endl;
#endif
  index_set s;
  for (index_type k = se_acts.length(); k > 0; k--) {
    s.insert(instance.actions[se_acts[k - 1].first].pre);
  }
  for (index_type k = 0; k < se_noops.length(); k++) s.insert(se_noops[k]);
  s.insert(new_noop.index);
#ifdef APPLY_D_MIN_TRICK
  NTYPE d_min = instance.min_dur;
  if (se_acts.length() > 0) d_min = MIN(d_min, se_acts[0].second);
  NTYPE val = MAX(heuristic.eval(s) + d_min, c_prev);
#else
  NTYPE val = MAX(heuristic.eval(s), c_prev);
#endif
#ifdef TRACE_PRINT_LOTS
  std::cerr << "leaving eval_se_noop, max = " << val << std::endl;
#endif
  if (r) {
    amt_vec cr(0, instance.n_resources());
    for (index_type i = 0; i < instance.n_resources(); i++) {
      cr[i] = 0;
      for (index_type k = 0; k < se_acts.length(); k++)
	cr[i] += instance.actions[se_acts[k].first].req(i);
    }
    if (!r->sufficient_consumable(s, se_acts)) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "insufficient resources: r = ";
      r->write(std::cerr);
      std::cerr << ", cr = ";
      cr.write(std::cerr, instance.n_resources());
      std::cerr << std::endl;
#endif
      return POS_INF;
    }
  }
  return val;
}

NTYPE TemporalRegState::eval_se_act
(index_cost_vec& se_acts, index_vec& se_noops, RegressionResourceState* r,
 Instance::Action& new_act, NTYPE c_prev)
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "entering eval_se_act" << std::endl << " se_acts: ";
  write_action_vec(se_acts);
  std::cerr << std::endl << " se_noops: ";
  instance.write_atom_set(std::cerr, se_noops);
  std::cerr << std::endl << " new_act: [+" << new_act.dur << "]"
 	    << new_act.name << std::endl
 	    << " c_prev = " << c_prev << std::endl;
#endif
  NTYPE val = c_prev;
  index_set s;
#ifdef TRACE_PRINT_LOTS
  std::cerr << "collecting actions before " << new_act.dur << std::endl;
#endif
  for (index_type k = se_acts.length(); k > 0; k--)
    if (se_acts[k - 1].second > new_act.dur) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "action "
 		<< instance.actions[se_acts[k - 1].first].name
 		<< std::endl;
#endif
      s.insert(instance.actions[se_acts[k - 1].first].pre);
    }
#ifdef TRACE_PRINT_LOTS
  std::cerr << "s = ";
  instance.write_atom_set(std::cerr, s);
  std::cerr << std::endl;
  std::cerr << "inserting new action" << std::endl;
#endif
  s.insert(new_act.pre);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "s = ";
  instance.write_atom_set(std::cerr, s);
  std::cerr << std::endl;
  std::cerr << "H(s) + delta = " << heuristic.eval(s)
 	    << " + " << new_act.dur
 	    << " = " << heuristic.eval(s) + new_act.dur
 	    << std::endl;
#endif
  val = MAX(heuristic.eval(s) + new_act.dur, val);
  for (index_type k = se_acts.length(); k > 0; k--)
    if (se_acts[k - 1].second <= new_act.dur) {
      s.insert(instance.actions[se_acts[k - 1].first].pre);
      val = MAX(heuristic.eval(s) + se_acts[k - 1].second, val);
    }
  if (!se_noops.empty()) {
    s.insert(se_noops);
#ifdef APPLY_D_MIN_TRICK
    NTYPE d_min = instance.min_dur;
    if (se_acts.length() > 0) d_min = MIN(d_min, se_acts[0].second);
    val = MAX(heuristic.eval(s) + d_min, val);
#else
    val = MAX(heuristic.eval(s), val);
#endif
  }
#ifdef TRACE_PRINT_LOTS
  std::cerr << "leaving eval_se_act, max = " << val << std::endl;
#endif
  if (r) {
    amt_vec cr(0, instance.n_resources());
    for (index_type i = 0; i < instance.n_resources(); i++) {
      cr[i] = 0;
      for (index_type k = 0; k < se_acts.length(); k++)
	cr[i] += instance.actions[se_acts[k].first].req(i);
      cr[i] += new_act.req(i);
    }
    if (!r->sufficient_consumable(s, se_acts)) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "insufficient resources: r = ";
      r->write(std::cerr);
      std::cerr << ", cr = ";
      cr.write(std::cerr, instance.n_resources());
      std::cerr << std::endl;
#endif
      return POS_INF;
    }
  }
  return val;
}

void TemporalRegState::apply_path_max()
{
  if (pre) {
    if (pre->est_cost() > (delta + est)) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "pathmax: " << pre->est_cost() << " > "
		<< delta << " + " << est
		<< std::endl;
#endif
      est = (pre->est_cost() - delta);
    }
  }
}

NTYPE TemporalRegState::delta_cost()
{
  return delta;
}

NTYPE TemporalRegState::est_cost()
{
  return est;
}

bool TemporalRegState::is_final()
{
  return final;
}

index_type TemporalRegState::state_size()
{
  index_type n = atom_set_size();
  for (index_type k = 0; k < acts.length(); k++) {
    Instance::Action& a = instance.actions[acts[k].first];
    for (index_type i = 0; i < a.pre.length(); i++)
      if (!set[a.pre[i]]) n += 1;
  }
  return n;
}

bool TemporalRegState::is_max()
{
  return false;
}

bool TemporalRegState::is_root()
{
  if (acts.length() > 0) return false;
  if (res) {
    if (!res->is_root()) return false;
  }
  return true;
}

bool goal_cost_decreasing::operator()
(const index_type& p0, const index_type& p1) const
{
  return (heuristic.eval(p0) > heuristic.eval(p1));
}

void TemporalRegState::sort_to(index_vec& s) {
  s.clear();
  goal_cost_decreasing o(heuristic);
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (set[k])
      s.insert_ordered(k, o);
}

bool TemporalRegState::applicable
(Instance::Atom& atom, Instance::Action& trie,
 index_cost_vec& se_acts, index_vec& se_noops, RegressionResourceState* r)
{
  // trie is not app if it deletes an atom in the parent nodes goal set
  // (selected noops and remaining goals are included in this set)
  for (index_type i = 0; i < trie.del.length(); i++)
    if (set[trie.del[i]]) return false;

  // trie is not app if it's incompatible with a selected actions
  for (index_type i = 0; i < se_acts.length(); i++) {
    if (!instance.non_interfering(trie.index, se_acts[i].first)) return false;
    if (!instance.lock_compatible(trie.index, se_acts[i].first)) return false;
  }

  if (r) {
    index_vec ca;
    for (index_type k = 0; k < se_acts.length(); k++)
      ca.append(se_acts[k].first);
    if (!r->applicable(trie, se_acts)) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "insufficient resources! (";
      r->write(std::cerr);
      std::cerr << ")" << std::endl;
#endif
      return false;
    }
  }

  // if all checks passed, trie is applicable
  return true;
}

index_type TemporalRegState::insert_action
(Instance::Action& trie, index_cost_vec& se_acts)
{
  se_acts.inc_length();
  index_type pos = se_acts.length() - 1;
  while (true) {
    if (pos == 0) {
      se_acts[pos] = index_cost_pair(trie.index, trie.dur);
      return pos;
    }
    else if (se_acts[pos - 1].second < trie.dur) {
      se_acts[pos] = index_cost_pair(trie.index, trie.dur);
      return pos;
    }
    else if ((se_acts[pos - 1].second == trie.dur) &&
	     (se_acts[pos - 1].first < trie.index)) {
      se_acts[pos] = index_cost_pair(trie.index, trie.dur);
      return pos;
    }
    else if ((se_acts[pos - 1].second == trie.dur) &&
	     (se_acts[pos - 1].first == trie.index)) {
      se_acts.dec_length();
      return no_such_index;
    }
    else {
      se_acts[pos] = se_acts[pos - 1];
      pos -= 1;
    }
  }
}

NTYPE TemporalRegState::regress
(index_vec& g_set, index_type p, bool_vec& r_set,
 index_cost_vec& se_acts, index_vec& se_noops, RegressionResourceState* r,
 NTYPE c_est, Search& s, NTYPE bound)
{
  if (p < g_set.length()) {
    while ((p < g_set.length()) && !r_set[g_set[p]]) p += 1;
  }

  if (p >= g_set.length()) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "regress(" << this << ", " << bound << "): se_acts =";
    for (index_type i = 0; i < se_acts.length(); i++)
      std::cerr << "[+" << se_acts[i].second << "]"
		<< instance.actions[se_acts[i].first].name;
    std::cerr << ", se_noops =";
    for (index_type i = 0; i < se_noops.length(); i++)
      std::cerr << " " << instance.atoms[se_noops[i]].name;
    std::cerr << std::endl;
#endif
    if (se_acts.length() == 0) return POS_INF;
    TemporalRegState* new_s = new_succ(se_acts, se_noops, r);
#ifdef CHECK_HEURISTIC_CONSISTENCY
    if (new_s->delta_cost() + new_s->est_cost() < est_cost()) {
      std::cerr << "inconsistent heuristic" << std::endl;
      std::cerr << "parent state: " << *this
		<< ", est. cost = " << est_cost()
		<< std::endl;
      std::cerr << "child state: " << *new_s
		<< ", delta + est. cost = "
		<< new_s->delta_cost() << " + " << new_s->est_cost()
		<< std::endl;
      exit(255);
    }
#endif
    NTYPE c_new = 0;
    if (new_s->delta_cost() + new_s->est_cost() <= bound) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "accepted: " << *new_s
		<< ", delta = " << new_s->delta_cost()
		<< ", est. cost = " << new_s->est_cost()
		<< ", final = " << new_s->is_final()
		<< std::endl;
#endif
#ifdef SEARCH_EXTRA_STATS
      n_succ_accepted += 1;
      rminc_count += 1;
      rminc_size += state_size();
      rminc_succ_size += new_s->state_size();
      rminc_succ_size_ratio += (new_s->state_size()/(double)state_size());
#ifdef PRINT_EXTRA_STATS
      std::cout << "RMINC " << depth()
		<< " " << size
		<< " " << acts.length()
		<< " " << state_size()
		<< " " << new_s->size
		<< " " << new_s->acts.length()
		<< " " << new_s->state_size()
		<< std::endl;
#endif
#endif
      c_new = new_s->delta_cost() +
	s.new_state(*new_s, bound - new_s->delta_cost());
    }
    else {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "discarded: " << *new_s
		<< ", delta = " << new_s->delta_cost()
		<< ", est. cost = " << new_s->est_cost()
		<< ", final = " << new_s->is_final()
		<< std::endl;
#endif
#ifdef SEARCH_EXTRA_STATS
      n_succ_discarded += 1;
#endif
      c_new = new_s->delta_cost() + new_s->est_cost();
    }
    delete new_s;
    return c_new;
  }

  // atom is the next goal atom for which to choose an establisher
  Instance::Atom& atom = instance.atoms[g_set[p]];
#ifdef TRACE_PRINT_LOTS
  std::cerr << "regress(" << this << "," << bound << "," << p
	    << "): next goal is " << atom.name << std::endl;
#endif

  if (implicit_noop) {
    bool imp_noop = false;
    for (index_type i = 0; (i < se_acts.length()) && !imp_noop; i++)
      if (instance.actions[se_acts[i].first].pre.contains(atom.index))
	imp_noop = true;
    if (imp_noop) {
      se_noops.append(atom.index);
      NTYPE c_new = regress(g_set, p + 1, r_set, se_acts, se_noops, r, c_est,
			    s, bound);
      se_noops.dec_length();
      return c_new;
    }
  }

  // c_min is the min cost over all establishers for atom
  NTYPE c_min = POS_INF;

  // first, if atoms is not VOLATILE, try a noop
#ifdef SUPPORT_VOLATILE_ATOMS
  if (!atom.volatile) {
#endif
#ifdef SEARCH_EXTRA_STATS
  trie_count += 1;
  tries_applicable += 1;
#endif
  NTYPE c_noop = eval_se_noop(se_acts, se_noops, r, atom, c_est);
  if (FINITE(c_noop) && (c_noop <= bound)) {
#ifdef SEARCH_EXTRA_STATS
    tries_within_bound += 1;
#endif
#ifdef TRACE_PRINT_LOTS
    std::cerr << "regress(" << this << "," << bound << "," << p
	      << "): trying NOOP" << std::endl;
#endif
    se_noops.append(atom.index);
#ifdef CHECK_INCREMENTAL_EVAL
    NTYPE c_check = eval(se_acts, se_noops, r);
    if (c_check != c_noop) {
      std::cerr << "diff (noop): " << c_check << " - " << c_noop << " = "
		<< c_check - c_noop << std::endl;
    }
#endif
    NTYPE c_new = regress(g_set, p + 1, r_set, se_acts, se_noops, r,
			  c_noop, s, bound);
#ifdef TRACE_PRINT_LOTS
    std::cerr << "regress(" << this << "," << bound << "," << p
	      << "): return from trying NOOP with c_new = " << c_new
	      << " and s.done = " << s.done()
	      << std::endl;
#endif
    se_noops.dec_length();
    if (s.done()) return c_new;
    c_min = MIN(c_min, c_new);
  }
  else {
    c_min = MIN(c_min, c_noop);
  }
#ifdef SUPPORT_VOLATILE_ATOMS
  } // if !atom.volatile
#endif

#ifdef TRACE_PRINT_LOTS
  std::cerr << "regress(" << this << "," << bound << "," << p
	    << "): actions adding " << atom.name << " are ";
  instance.write_action_set(std::cerr, atom.add_by);
  std::cerr << std::endl;
#endif

  // then try all SELECTABLE actions adding the goal atom
  for (index_type k = 0; k < atom.add_by.length(); k++) if (instance.actions[atom.add_by[k]].sel) {
    Instance::Action& trie = instance.actions[atom.add_by[k]];
#ifdef SEARCH_EXTRA_STATS
    trie_count += 1;
#endif

#ifdef TRACE_PRINT_LOTS
    std::cerr << "regress(" << this << "," << bound << "," << p
	      << "): checking " << trie.name << std::endl;
#endif

    // check applicability
    bool is_app = applicable(atom, trie, se_acts, se_noops, r);

    // redundancy check: do not select an action that adds an atom that
    // has already been selected to be established by a noop
    if (is_app) {
      if (trie.add.first_common_element(se_noops) != no_such_index)
	is_app = false;
    }

    if (is_app) {
#ifdef SEARCH_EXTRA_STATS
      tries_applicable += 1;
#endif
      // compute cost
      NTYPE c_trie = eval_se_act(se_acts, se_noops, r, trie, c_est);
#ifdef TRACE_PRINT_LOTS
      std::cerr << "regress(" << this << "," << bound << "," << p
		<< "): eval to " << c_trie << std::endl;
#endif
      if (FINITE(c_trie) && (c_trie <= bound)) {
#ifdef SEARCH_EXTRA_STATS
	tries_within_bound += 1;
#endif
#ifdef TRACE_PRINT_LOTS
	std::cerr << "regress(" << this << "," << bound << "," << p
		  << "): trying " << trie.name << std::endl;
#endif
	// insert the new trie into the selected set
	index_type trie_pos = insert_action(trie, se_acts);

#ifdef CHECK_INCREMENTAL_EVAL
	NTYPE c_check = eval(se_acts, se_noops, r);
	if (c_check != c_trie) {
	  std::cerr << "diff (action): " << c_check
		    << " - " << c_trie
		    << " = " << c_check - c_trie << std::endl;
	}
#endif

	// remove atoms added by the action from the remaining goal set
	bool reset_add[trie.add.length()];
	for (index_type i = 0; i < trie.add.length(); i++) {
	  if (r_set[trie.add[i]]) {
	    r_set[trie.add[i]] = false;
	    reset_add[i] = true;
	  }
	  else reset_add[i] = false;
	}

	NTYPE c_new = regress(g_set, p + 1, r_set, se_acts, se_noops, r,
			      c_trie, s, bound);

	// reset action effects
	for (index_type i = 0; i < trie.add.length(); i++)
	  if (reset_add[i]) r_set[trie.add[i]] = true;

	// remove trie from the selected set
	if (trie_pos != no_such_index) {
	  for (index_type i = trie_pos; i < se_acts.length() - 1; i++)
	    se_acts[i] = se_acts[i + 1];
	  se_acts.dec_length();
	}

	if (s.done())
	  return c_new;
	else
	  c_min = MIN(c_min, c_new);
      }
      else {
	c_min = MIN(c_min, c_trie);
      }
    } // if (is_app)
#ifdef TRACE_PRINT_LOTS
    else {
      std::cerr << "regress(" << this << "," << bound << "," << p
		<< "): not applicable" << std::endl;
    }
#endif
  } // for each trie...

#ifdef TRACE_PRINT_LOTS
  std::cerr << "regress(" << this << "," << bound << "): c_min = "
	    << c_min << std::endl;
#endif
  return c_min;
}

NTYPE TemporalRegState::expand(Search& s, NTYPE bound)
{
  index_vec g_set(no_such_index, 0);
  if (subgoal_ordering) {
    sort_to(g_set);
  }
  else {
    copy_to(g_set);
  }

  bool_vec r_set; // (*this);
  copy_to(r_set);

  index_vec se_noops(no_such_index, 0);
  index_cost_vec se_acts(acts);
  // RegressionResourceState* new_r = (res ? res->copy() : 0);

#ifdef TRACE_PRINT_LOTS
  std::cerr << "expand(" << this << "," << bound << "): expanding "
	    << *this << " (pre = " << predecessor() << ")" << std::endl;
#endif
#ifdef SEARCH_EXTRA_STATS
  n_succ_accepted = 0;
  n_succ_discarded = 0;
#endif
  NTYPE c_rec = regress(g_set, 0, r_set, se_acts, se_noops, res,
			0, s, bound);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "expand(" << this << "," << bound << "): new cost = "
	    << c_rec << std::endl;
#endif
  // if (new_r) delete new_r;
#ifdef SEARCH_EXTRA_STATS
  rminx_count += 1;
  rminx_size += state_size();
  rminx_succ += n_succ_accepted + n_succ_discarded;
#ifdef PRINT_EXTRA_STATS
  std::cout << "RMINX " << depth() << " "
	    << size << " " << acts.length() << " "
	    << n_succ_accepted << " " << n_succ_discarded
	    << " " << s.done()
	    << std::endl;
#endif
#endif
  return c_rec;
}

void TemporalRegState::store(NTYPE cost, bool opt)
{
  if (res) {
    if (!res->is_root()) return;
  }
  if (acts.length() == 0) {
    heuristic.store(set, cost, opt);
  }
#ifndef ENSURE_CONSISTENCY_IN_TEMPORAL_RELAXED_SEARCH
  else {
    index_set atoms;
    copy_to(atoms);
    NTYPE d_max = 0;
    for (index_type k = 0; k < acts.length(); k++) {
      atoms.insert(instance.actions[acts[k].first].pre);
      d_max = MAX(acts[k].second, d_max);
    }
    if (heuristic.eval(atoms) < (cost - d_max)) {
      heuristic.store(atoms, cost - d_max, false);
    }
  }
#endif
}

void TemporalRegState::reevaluate()
{
  if (is_final()) return;

  // evaluate state
  index_set s;
  for (index_type k = acts.length(); k > 0; k--) {
    Instance::Action& act = instance.actions[acts[k - 1].first];
    for (index_type i = 0; i < act.pre.length(); i++) {
      s.insert(act.pre[i]);
    }
#ifdef APPLY_NCW_NOOP_TRICK
    // if goal atom i is n.c.w. with action act, it can only be noop:ed
    // up to the point where act begins
    for (index_type i = 0; i < instance.n_atoms(); i++) if (set[i]) {
      if (!s.contains(i) && act.ncw_atms.contains(i)) s.insert(i);
    }
#endif
    est = MAX(heuristic.eval(s) + acts[k - 1].second, est);
  }
  for (index_type i = 0; i < instance.n_atoms(); i++) if (set[i]) {
    s.insert(i);
  }
  est = MAX(heuristic.eval(s), est);

  // check consumable resources
  if (res) {
    amt_vec cr(0, instance.n_resources());
    for (index_type i = 0; i < instance.n_resources(); i++) {
      cr[i] = 0;
      for (index_type k = 0; k < acts.length(); k++)
	cr[i] += instance.actions[acts[k].first].req(i);
    }
    if (!res->sufficient_consumable(s, acts)) {
      est = POS_INF;
    }
  }
}

int TemporalRegState::compare(const State& s)
{
  const TemporalRegState& ts = (const TemporalRegState&)s;
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    if (!set[k] && ts.set[k]) return -1;
    else if (set[k] && !ts.set[k]) return 1;
  }
  if (acts.length() < ts.acts.length()) return -1;
  else if (acts.length() > ts.acts.length()) return 1;
  for (index_type k = 0; k < acts.length(); k++) {
    if (acts[k].second < ts.acts[k].second) return -1;
    else if (acts[k].second > ts.acts[k].second) return 1;
    else {
      if (acts[k].first < ts.acts[k].first) return -1;
      else if (acts[k].first > ts.acts[k].first) return 1;
    }
  }
  if (res && !ts.res) return 1;
  else if (!res && ts.res) return -1;
  else if (res && ts.res) return res->compare(*ts.res);
  else return 0;
}

index_type TemporalRegState::hash()
{
  index_type v1 = AtomSet::hash();
  if (acts.length() == 0)
    return v1;
  if (acts.length() == 1)
    return (instance.action_set_hash(acts[0].first, v1) +
	    HASH1(acts[0].second));
  index_type v2 = 0;
  index_type v3 = 0;
  for (index_type k = 0; k < acts.length() - 1; k++) {
    v2 = instance.action_set_hash(acts[k].first, v2);
    v3 += HASH2(acts[k].second, acts[k + 1].second);
  }
  v2 = instance.action_set_hash(acts[acts.length() - 1].first, v2);
  return (v1 + v2 + v3);
}

State* TemporalRegState::copy() {
  return new TemporalRegState(*this);
}

TemporalRegState* TemporalRegState::new_succ
(const index_cost_vec& se_acts, const index_vec& se_noops, RegressionResourceState* r)
{
  TemporalRegState* new_s =
    new TemporalRegState(instance, heuristic, se_acts, se_noops, r);
  new_s->set_predecessor(this);
#ifdef APPLY_PATH_MAX
  new_s->apply_path_max();
#endif
  return new_s;
}

State* TemporalRegState::new_state(const index_set& s, State* p) {
  TemporalRegState* new_s;
  new_s = new TemporalRegState(instance, heuristic, s,
			       (res ? res->new_state() : 0));
  new_s->set_predecessor(p);
  return new_s;
}

State* TemporalRegState::new_state(const bool_vec& s, State* p) {
  TemporalRegState* new_s;
  new_s = new TemporalRegState(instance, heuristic, s,
			       (res ? res->new_state() : 0));
  new_s->set_predecessor(p);
  return new_s;
}

void TemporalRegState::insert(Plan& p) {
  if (final && strict_right_shift) {
    // std::cerr << "inserting final state: " << *this << std::endl;
    NTYPE zero_delta = 0;
    for (index_type k = acts.length(); k > 0; k--) {
      // std::cerr << "inserting " << instance.actions[acts[k - 1].first].name
      //	<< " at +" << zero_delta << std::endl;
      p.insert(acts[k - 1].first);
      if (k > 1) {
	NTYPE adv_delta = (acts[k - 1].second - acts[k - 2].second);
	if (adv_delta > 0) p.advance(adv_delta);
	zero_delta += adv_delta;
      }
      else {
	NTYPE adv_delta = acts[k - 1].second;
	if (adv_delta > 0) p.advance(adv_delta);
	zero_delta += adv_delta;
      }
    }
    for (index_type k = 0; k < starts.length(); k++) {
      bool delayed = false;
      for (index_type i = 0; (i < acts.length()) && !delayed; i++)
	if (acts[i].first == starts[k]) delayed = true;
      if (!delayed) {
	// std::cerr << "inserting " << instance.actions[starts[k]].name
	//	  << " at +" << zero_delta << std::endl;
	p.insert(starts[k]);
      }
    }
    p.advance(delta - zero_delta);
  }
  else {
    for (index_type k = 0; k < starts.length(); k++)
      p.insert(starts[k]);
    for (index_type k = 0; k < noops.length(); k++)
      p.protect(noops[k]);
    p.advance(delta);
  }
}

void TemporalRegState::insert_path(Plan& p)
{
  insert(p);
  if (predecessor()) {
    predecessor()->insert_path(p);
  }
}

void TemporalRegState::write_plan(std::ostream& s)
{
  // s << "[+" << delta << "]";
  for (index_type k = 0; k < starts.length(); k++) {
    if (k > 0) s << ", ";
    s << instance.actions[starts[k]].name;
  }
  for (index_type k = 0; k < noops.length(); k++) {
    if ((starts.length() > 0) || (k > 0)) s << ", ";
    s << "noop:" << instance.atoms[noops[k]].name;
  }
}

void TemporalRegState::write(std::ostream& s) {
  s << "[+" << delta << "](";
  instance.write_atom_set(s, set);
  for (index_type k = 0; k < acts.length(); k++) {
    s << ", [+" << acts[k].second << "]"
      << instance.actions[acts[k].first].name;
  }
  if (res) {
    s << ", ";
    res->write(s);
  }
  s << ", starts: ";
  instance.write_action_set(s, starts);
  s << ", noops: ";
  instance.write_atom_set(s, noops);
  s << ")";
}

void TemporalRegState::write_path_as_graph(std::ostream& s)
{
  TemporalRegState* p = (TemporalRegState*)predecessor();
  if (p) {
    p->write_path_as_graph(s);
  }
  if (size > 0) {
    s << "{ rank=same;" << std::endl;
    for (index_type k = 0; k < instance.n_atoms(); k++) if (set[k]) {
      s << "S" << depth() << "A" << k
	<< " [shape=\"ellipse\",label=\"[" << depth() << "]"
	<< instance.atoms[k].name << "\"];"
	<< std::endl;
    }
    s << "}" << std::endl;
  }
  if (p) {
    for (index_type k = 0; k < starts.length(); k++) {
      s << "S" << depth() << "S" << starts[k]
	<< " [shape=\"box\",label=\"[" << depth() << "]start:"
	<< instance.actions[starts[k]].name << " \"];"
	<< std::endl;
      for (index_type i = 0; i < instance.actions[starts[k]].pre.length(); i++)
	s << "S" << depth() << "A" << instance.actions[starts[k]].pre[i]
	  << " -> "
	  << "S" << depth() << "S" << starts[k]
	  << ";" << std::endl;
      if (instance.actions[starts[k]].dur <= delta) {
	s << "S" << depth() << "F" << starts[k]
	  << " [shape=\"box\",label=\"[" << depth() << "]finish:"
	  << instance.actions[starts[k]].name << " \"];"
	  << std::endl;
	s << "S" << depth() << "S" << starts[k] << " -> "
	  << "S" << depth() << "F" << starts[k] << ";"
	  << std::endl;
	for (index_type i = 0; i < instance.actions[starts[k]].add.length(); i++) if (p->set[instance.actions[starts[k]].add[i]])
	  s << "S" << depth() << "F" << starts[k]
	    << " -> "
	    << "S" << p->depth() << "A" << instance.actions[starts[k]].add[i]
	    << ";" << std::endl;
      }
      else {
	TemporalRegState* q = p;
	while (q) {
	  index_type iq = no_such_index;
	  for (index_type i = 0; (i < q->acts.length()) && (iq == no_such_index); i++) if (q->acts[i].first == starts[k]) iq = i;
	  if (iq != no_such_index) {
	    if ((q->acts[iq].second + q->delta) == instance.actions[starts[k]].dur) {
	      s << "S" << depth() << "S" << starts[k] << " -> "
		<< "S" << q->depth() << "F" << starts[k] << ";"
		<< std::endl;
	      q = 0;
	    }
	    else {
	      q = (TemporalRegState*)q->predecessor();
	    }
	  }
	  else {
	    q = (TemporalRegState*)q->predecessor();
	  }
	}
      }
    }
    for (index_type k = 0; k < acts.length(); k++) {
      if ((acts[k].second + delta) == instance.actions[acts[k].first].dur) {
	s << "S" << depth() << "F" << acts[k].first
	  << " [shape=\"box\",label=\"[" << depth() << "]finish:"
	  << instance.actions[acts[k].first].name
	  << " \"];"
	  << std::endl;
	for (index_type i = 0; i < instance.actions[acts[k].first].add.length(); i++) if (p->set[instance.actions[acts[k].first].add[i]])
	  s << "S" << depth() << "F" << acts[k].first
	    << " -> "
	    << "S" << p->depth() << "A" << instance.actions[acts[k].first].add[i]
	    << ";" << std::endl;
      }
    }
    for (index_type k = 0; k < noops.length(); k++) {
      s << "S" << depth() << "A" << noops[k]
	<< " -> " << "S" << p->depth() << "A" << noops[k]
	<< " [style=\"dashed\"];"
	<< std::endl;
    }
  }
}

void TemporalRegState::write_action_vec(const index_cost_vec& av)
{
  for (index_type k = 0; k < av.length(); k++) {
    if (k > 0) std::cerr << ", ";
    std::cerr << "[" << av[k].second << "]"
	      << instance.actions[av[k].first].name;
  }
}

bool TemporalRSRegState::applicable
(Instance::Atom& atom, Instance::Action& trie,
 index_cost_vec& se_acts, index_vec& se_noops, RegressionResourceState* r)
{
  if (!TemporalRegState::applicable(atom, trie, se_acts, se_noops, r))
    return false;
  bool was_noop = noops.contains(atom.index);
  for (index_type i = 0; (i < starts.length()) && was_noop; i++)
    if (instance.actions[starts[i]].pre.contains(atom.index)) was_noop = false;
  // if the atom was established by a noop...
  if (was_noop) {
    index_set s_pre(noops);
    s_pre.subtract(atom.index);
    // ...but we could have used this action instead...
    for (index_type i = 0; i < starts.length(); i++) {
      // check that trie is compatible with all actions starting in this
      // state (since they were in se_acts when this state was created)
      // NOTE: At this point, we check that actions are COMMUTATIVE, rather
      // than non-interfering (commutativity is a stronger requirement; i.e.,
      // this weakens the pruning power of the cut). Using the stronger cut
      // the planner fails to find solutions in some problems (example:
      // airport/temporal/p03). Not clear if this is an implementation issue
      // or if the stronger cut is also in principle unsafe.
      // if (!instance.non_interfering(starts[i], trie.index)) return true;
      if (!instance.commutative(starts[i], trie.index)) return true;
      if (!instance.lock_compatible(starts[i], trie.index)) return true;
      s_pre.insert(instance.actions[starts[i]].pre);
    }
    // need to check resource use also
    if (r) {
      index_vec ca;
      for (index_type k = 0; k < se_acts.length(); k++)
	ca.append(se_acts[k].first);
      for (index_type k = 0; k < starts.length(); k++)
	ca.append(starts[k]);
      if (!r->applicable(trie, se_acts, starts)) return true;
    }
#ifdef CHECK_FOR_BAD_CUTS
    // calculate estimated cost of predecessor state with trie among
    // starting actions, and compare to estimated cost of inserting trie
    // at this point: if the former is greater (which it should never be)
    // we can't cut
    s_pre.insert(trie.pre);
    NTYPE c_then = (heuristic.eval(s_pre) + delta_cost());
    NTYPE c_now = eval_se_act(se_acts, se_noops, r, trie, est_cost());
    if (c_then > c_now) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "BAD CUT!" << std::endl;
      std::cerr << "action: " << trie.name
		<< " (" << trie.index << ")" << std::endl;
      std::cerr << "atom: " << atom.name
		<< " (" << atom.index << ")" << std::endl;
      std::cerr << "pre (" << this << "): " << *this << std::endl;
      std::cerr << "s_pre = ";
      instance.write_atom_set(std::cerr, s_pre);
      std::cerr << std::endl;
      std::cerr << "H(s_pre) + delta = ";
      heuristic.write_eval(s_pre, std::cerr, 0, false);
      std::cerr << " + " << delta_cost() << " = " << c_then << std::endl;
      std::cerr << "eval_se_act(trie) = " << c_now << std::endl;
#endif
      return true;
    }
#endif
    // ...then cut
#ifdef TRACE_PRINT_LOTS
    std::cerr << "right-shift cut!" << std::endl;
    std::cerr << "action: " << trie.name
	      << " (" << trie.index << ")" << std::endl;
    std::cerr << "atom: " << atom.name
	      << " (" << atom.index << ")" << std::endl;
    std::cerr << "pre (" << this << "): " << *this << std::endl;
#endif
    return false;
  }
  else {
    return true;
  }
}

int TemporalRSRegState::compare(const State& s)
{
  int d = TemporalRegState::compare(s);
  if (d == 0) {
    const TemporalRSRegState& trss = (const TemporalRSRegState&)s;
    if (starts.length() < trss.starts.length()) return -1;
    else if (starts.length() > trss.starts.length()) return 1;
    for (index_type k = 0; k < starts.length(); k++) {
      if (starts[k] < trss.starts[k]) return -1;
      else if (starts[k] > trss.starts[k]) return 1;
    }
    if (noops.length() < trss.noops.length()) return -1;
    else if (noops.length() > trss.noops.length()) return 1;
    for (index_type k = 0; k < noops.length(); k++) {
      if (noops[k] < trss.noops[k]) return -1;
      else if (noops[k] > trss.noops[k]) return 1;
    }
    return 0;
  }
  else {
    return d;
  }
}

index_type TemporalRSRegState::hash()
{
  index_type v0 = TemporalRegState::hash();
  index_type v1 = instance.action_set_hash(starts);
  index_type v2 = instance.atom_set_hash(noops);
  return (v0 + v1 + v2);
}

State* TemporalRSRegState::copy() {
  return new TemporalRSRegState(*this);
}

TemporalRegState* TemporalRSRegState::new_succ
(const index_cost_vec& se_acts,
 const index_vec& se_noops,
 RegressionResourceState* r)
{
  TemporalRSRegState* new_s =
    new TemporalRSRegState(instance, heuristic, se_acts, se_noops, r);
  new_s->set_predecessor(this);
#ifdef APPLY_PATH_MAX
  new_s->apply_path_max();
#endif
  return new_s;
}

State* TemporalRSRegState::new_state(const index_set& s, State* p)
{
  TemporalRSRegState* new_s =
    new TemporalRSRegState(instance, heuristic, s,
			   (res ? res->new_state() : 0));
  new_s->set_predecessor(p);
  return new_s;
}

State* TemporalRSRegState::new_state(const bool_vec& s, State* p)
{
  TemporalRSRegState* new_s =
    new TemporalRSRegState(instance, heuristic, s,
			   (res ? res->new_state() : 0));
  new_s->set_predecessor(p);
  return new_s;
}

TemporalRegState* ApxTemporalRegState::new_succ
(const index_cost_vec& se_acts,
 const index_vec& se_noops,
 RegressionResourceState* r)
{
  ApxTemporalRegState* new_s =
    new ApxTemporalRegState(instance, heuristic, se_acts, se_noops, r, limit);
  new_s->set_predecessor(this);
#ifdef APPLY_PATH_MAX
  new_s->apply_path_max();
#endif
  return new_s;
}

bool ApxTemporalRegState::is_max()
{
  AtomSet atoms(*this);
  for (index_type k = 0; k < acts.length(); k++)
    atoms.add(instance.actions[acts[k].first].pre);
  return (atoms.atom_set_size() > limit);
}

NTYPE ApxTemporalRegState::expand(Search& s, NTYPE bound)
{
  AtomSet atoms(*this);
  for (index_type k = 0; k < acts.length(); k++)
    atoms.add(instance.actions[acts[k].first].pre);
  if (atoms.atom_set_size() > limit) {
    NTYPE c_max = atoms.split(limit, *this, this, s, bound);
    if (s.solved())
      if (atoms.atom_set_size() > max_set_size_encountered)
	max_set_size_encountered = atoms.atom_set_size();
    return MAX(c_max, est);
  }
  else {
    NTYPE c_rec = TemporalRegState::expand(s, bound);
    return c_rec;
  }
}

State* ApxTemporalRegState::new_state(const index_set& s, State* p)
{
  ApxTemporalRegState* new_s =
    new ApxTemporalRegState(instance, heuristic, s,
			    (res ? res->new_state() : 0),
			    limit);
  new_s->set_predecessor(p);
  return new_s;
}

State* ApxTemporalRegState::new_state(const bool_vec& s, State* p)
{
  ApxTemporalRegState* new_s =
    new ApxTemporalRegState(instance, heuristic, s,
			    (res ? res->new_state() : 0),
			    limit);
  new_s->set_predecessor(p);
  return new_s;
}

State* ApxTemporalRegState::copy()
{
  return new ApxTemporalRegState(*this);
}

END_HSPS_NAMESPACE
