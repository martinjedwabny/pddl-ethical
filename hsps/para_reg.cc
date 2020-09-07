
#include "para_reg.h"
#include "temporal.h"

BEGIN_HSPS_NAMESPACE

void ParaRegState::sort_to(index_vec& s) {
  s.clear();
  goal_cost_decreasing o(heuristic);
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (set[k])
      s.insert_ordered(k, o);
}

bool ParaRegState::applicable
(Instance::Atom& atom, Instance::Action& trie,
 bool_vec& p_set, bool_vec& x_set)
{
  for (index_type i = 0; (i < trie.del.length()); i++)
    if (p_set[trie.del[i]] || set[trie.del[i]]) return false;
  for (index_type i = 0; (i < trie.pre.length()); i++)
    if (x_set[trie.pre[i]]) return false;
  return true;
}

NTYPE ParaRegState::regress
(index_vec& g_set, index_type p,
 bool_vec& r_set, bool_vec& p_set, bool_vec& x_set,
 NTYPE new_est, Search& s, NTYPE bound)
{
  if (p < g_set.length()) {
    while ((p < g_set.length()) && !r_set[g_set[p]]) p += 1;
  }

  if (p >= g_set.length()) {
    if (acts.length() == 0) return POS_INF;
    else {
      count();
      eval();
#ifdef EXTRA_WARNINGS
      if (est != new_est) {
	std::cerr << "warning: " << std::endl;
	std::cerr << "new_est = " << new_est
		  << ", eval() = " << heuristic.eval(set)
		  << std::endl;
      }
#endif
#ifdef SEARCH_EXTRA_STATS
      n_succ_accepted += 1;
      rminc_count += 1;
      rminc_size += g_set.length();
      rminc_succ_size += atom_set_size();
      rminc_succ_size_ratio += (atom_set_size()/(double)g_set.length());
#endif
      NTYPE c_new = 1 + s.new_state(*this, bound - 1);
      if (s.solved()) {
	if (size > max_set_size_encountered) max_set_size_encountered = size;
      }
      return c_new;
    }
  }

#ifdef TRACE_PRINT_LOTS
  std::cerr << "regress(" << bound << "): " << p << " in ";
  instance.write_atom_set(std::cerr, g_set);
  std::cerr << std::endl;
#endif

  Instance::Atom& atom = instance.atoms[g_set[p]];
  NTYPE c_min = POS_INF;

  // if current goal atom is already in the set (new state), there is no
  // reason to choose any establisher other than a noop
  if (set[atom.index]) {
    // noops.append(atom.index);
#ifdef TRACE_PRINT_LOTS
    std::cerr << "regress(" << bound << "," << p
	      << "): forced NOOP, cost = 1 + "
	      << new_est << std::endl;
#endif
    NTYPE c_new = regress(g_set, p + 1, r_set, p_set, x_set, new_est,
			  s, bound);
    // noops.dec_length();
    return c_new;
  }

  // if not, we must try all possible establishers
  else {

    // first, try a NOOP
#ifdef SEARCH_EXTRA_STATS
    trie_count += 1;
    tries_applicable += 1;
#endif
    NTYPE noop_est = MAX(new_est, heuristic.incremental_eval(set, atom.index));
#ifdef TRACE_PRINT_LOTS
    std::cerr << "regress(" << bound << "," << p << "): NOOP, cost = 1 + "
	      << noop_est << std::endl;
#endif
    if (FINITE(noop_est) && (1 + noop_est <= bound)) {
#ifdef SEARCH_EXTRA_STATS
      tries_within_bound += 1;
#endif
      set[atom.index] = true;
      noops.append(atom.index);
      NTYPE c_new = regress(g_set, p + 1, r_set, p_set, x_set, noop_est,
			    s, bound);
      if (s.done()) return c_new;
      else c_min = MIN(c_min, c_new);
      noops.dec_length();
    }
    else {
#ifdef SEARCH_EXTRA_STATS
      n_succ_discarded += 1;
#endif
      c_min = MIN(c_min, 1 + noop_est);
    }
    set[atom.index] = false;

    // then try all actions adding the goal atom
    for (index_type k = 0; k < atom.add_by.length(); k++) if (instance.actions[atom.add_by[k]].sel) {
      Instance::Action& trie = instance.actions[atom.add_by[k]];
#ifdef SEARCH_EXTRA_STATS
      trie_count += 1;
#endif

      // check applicability
      bool is_app = applicable(atom, trie, p_set, x_set);

      if (is_app) {
#ifdef SEARCH_EXTRA_STATS
	tries_applicable += 1;
#endif
	// compute cost, starting with max of the cost over what is already
	// in the set (new_est) and the preconditions of the action...
	//NTYPE trie_est = MAX(new_est, heuristic.action_pre_cost(trie.index));
	NTYPE trie_est = MAX(new_est, heuristic.eval(trie.pre));

	// then the cost of action preconditions combined with atoms already
	// in the set
	bool reset_pre[trie.pre.length()];
	index_type pre_i = 0;
	while (FINITE(trie_est) && (1 + trie_est <= bound) &&
	       (pre_i < trie.pre.length())) {
	  if (!set[trie.pre[pre_i]]) {
	    trie_est = MAX(trie_est,
			   heuristic.incremental_eval(set, trie.pre[pre_i]));
	    set[trie.pre[pre_i]] = true;
	    reset_pre[pre_i] = true;
	  }
	  else reset_pre[pre_i] = false;
	  pre_i += 1;
	}

	// now, preconditions up to the pre_i:th have been added to the set,
	// remember that!

#ifdef TRACE_PRINT_LOTS
	std::cerr << "regress(" << bound << "," << p << "): " << trie.name
		  << ", cost = 1 + " << trie_est << std::endl;
#endif

	if (FINITE(trie_est) && (1 + trie_est <= bound)) {
#ifdef SEARCH_EXTRA_STATS
	  tries_within_bound += 1;
#endif

	  // apply action effects and recurse
	  bool_vec reset_add(false, trie.add.length());
	  for (index_type i = 0; i < trie.add.length(); i++) {
	    if (r_set[trie.add[i]]) {
	      r_set[trie.add[i]] = false;
	      reset_add[i] = true;
	    }
	    else reset_add[i] = false;
	  }

	  bool_vec reset_del(false, trie.del.length());
	  for (index_type i = 0; i < trie.del.length(); i++) {
	    if (!x_set[trie.del[i]]) {
	      x_set[trie.del[i]] = true;
	      reset_del[i] = true;
	    }
	    else reset_del[i] = false;
	  }

	  acts.append(trie.index);
	  NTYPE c_new = regress(g_set, p + 1, r_set, p_set, x_set, trie_est,
				s, bound);

	  // reset action effects
	  for (index_type i = 0; i < trie.add.length(); i++)
	    if (reset_add[i]) r_set[trie.add[i]] = true;
	  for (index_type i = 0; i < trie.del.length(); i++)
	    if (reset_del[i]) x_set[trie.del[i]] = false;
	  acts.dec_length();

	  if (s.done()) return c_new;
	  else c_min = MIN(c_min, c_new);
	}
	else {
#ifdef SEARCH_EXTRA_STATS
	  n_succ_discarded += 1;
#endif
	  c_min = MIN(c_min, 1 + trie_est);
	}

	// reset preconditions, up to the pre_i:th
	for (index_type i = 0; i < pre_i; i++)
	  if (reset_pre[i]) set[trie.pre[i]] = false;
      }
    }
  }

#ifdef TRACE_PRINT_LOTS
  std::cerr << "regress(" << bound << "," << p << "): c_min = "
	    << c_min << std::endl;
#endif
  return c_min;
}

NTYPE ParaRegState::delta_cost() {
  if (acts.length() == 0) return 0;
  else return 1;
}

NTYPE ParaRegState::expand(Search& s, NTYPE bound) {
  index_vec g_set(no_such_index, 0);
  bool_vec  r_set; // (*this);
  bool_vec  x_set(false, instance.n_atoms());

  sort_to(g_set);
  copy_to(r_set);
  for (index_type k = 0; k < instance.n_atoms(); k++) x_set[k] = false;

  ParaRegState* new_s = new_succ();
#ifdef SEARCH_EXTRA_STATS
  new_s->n_succ_accepted = 0;
  new_s->n_succ_discarded = 0;
#endif
  NTYPE c_new = new_s->regress(g_set, 0, r_set, set, x_set, new_s->est,
			       s, bound);
#ifdef SEARCH_EXTRA_STATS
  rminx_count += 1;
  rminx_size += atom_set_size();
  rminx_succ += new_s->n_succ_accepted + new_s->n_succ_discarded;
#endif

  delete new_s;
  return c_new;
}

void ParaRegState::insert(Plan& p) {
  NTYPE dmax = 0;
  for (index_type k = 0; k < acts.length(); k++) {
    p.insert(acts[k]);
    dmax = MAX(dmax, instance.actions[acts[k]].dur);
  }
  for (index_type k = 0; k < noops.length(); k++)
    p.protect(noops[k]);
  if (acts.length() > 0) p.advance(dmax);
}

void ParaRegState::insert_path(Plan& p)
{
  insert(p);
  if (predecessor()) {
    predecessor()->insert_path(p);
  }
}

void ParaRegState::write_plan(std::ostream& s)
{
  for (index_type k = 0; k < acts.length(); k++) {
    if (k > 0) s << ", ";
    s << instance.actions[acts[k]].name;
  }
  for (index_type k = 0; k < noops.length(); k++) {
    if ((acts.length() > 0) || (k > 0)) s << ", ";
    s << "noop:" << instance.atoms[noops[k]].name;
  }
}

State* ParaRegState::copy() {
  return new ParaRegState(*this);
}

ParaRegState* ParaRegState::new_succ() {
  ParaRegState* new_s = new ParaRegState(instance, heuristic);
  new_s->set_predecessor(this);
  return new_s;
}

State* ParaRegState::new_state(const index_set& s, State* p) {
  ParaRegState* new_s = new ParaRegState(instance, heuristic, s);
  new_s->set_predecessor(p);
  return new_s;
}

State* ParaRegState::new_state(const bool_vec& s, State* p) {
  ParaRegState* new_s = new ParaRegState(instance, heuristic, s);
  new_s->set_predecessor(p);
  return new_s;
}

bool ApxParaRegState::is_max() {
  return (size > limit);
}

NTYPE ApxParaRegState::expand(Search& s, NTYPE bound) {
  if (size > limit) {
    return split(limit, *this, this, s, bound);
  }
  else {
    NTYPE c_rec = ParaRegState::expand(s, bound);
    return c_rec;
  }
}

ParaRegState* ApxParaRegState::new_succ()
{
  ApxParaRegState* new_s = new ApxParaRegState(instance, heuristic, limit);
  new_s->set_predecessor(this);
  return new_s;
}

State* ApxParaRegState::new_state(const index_set& s, State* p)
{
  ApxParaRegState* new_s =
    new ApxParaRegState(instance, heuristic, s, limit);
  new_s->set_predecessor(p);
  return new_s;
}

State* ApxParaRegState::new_state(const bool_vec& s, State* p)
{
  ApxParaRegState* new_s =
    new ApxParaRegState(instance, heuristic, s, limit);
  new_s->set_predecessor(p);
  return new_s;
}

State* ApxParaRegState::copy()
{
  return new ApxParaRegState(*this);
}


NTYPE RndApxParaRegState::expand(Search& s, NTYPE bound)
{
  if (size > limit) {
    RndApxParaRegState sub_s(*this);
    sub_s.set_predecessor(this);
    sub_s.acts.set_length(0);
    sub_s.noops.set_length(0);
    NTYPE c_max = est;
    for (index_type k = 0; k < n_subsets; k++) {
      NTYPE c_sub = sub_s.split_random(limit, *this, this, s, bound, rnd_src);
      c_max = MAX(c_max, c_sub);
    }
    return c_max;
  }
  else {
    NTYPE c_rec = ParaRegState::expand(s, bound);
    return c_rec;
  }
}

ParaRegState* RndApxParaRegState::new_succ()
{
  RndApxParaRegState* new_s =
    new RndApxParaRegState(instance, heuristic, limit, n_subsets, rnd_src);
  new_s->set_predecessor(this);
  return new_s;
}

State* RndApxParaRegState::new_state(const index_set& s, State* p)
{
  RndApxParaRegState* new_s =
    new RndApxParaRegState(instance, heuristic, s, limit, n_subsets, rnd_src);
  new_s->set_predecessor(p);
  return new_s;
}

State* RndApxParaRegState::new_state(const bool_vec& s, State* p)
{
  RndApxParaRegState* new_s =
    new RndApxParaRegState(instance, heuristic, s, limit, n_subsets, rnd_src);
  new_s->set_predecessor(p);
  return new_s;
}

State* RndApxParaRegState::copy()
{
  return new RndApxParaRegState(*this);
}

bool ParaRSRegState::applicable
(Instance::Atom& atom, Instance::Action& trie,
 bool_vec& p_set, bool_vec& x_set)
{
  if (!ParaRegState::applicable(atom, trie, p_set, x_set)) return false;
  ParaRSRegState* rs_pre = (ParaRSRegState*)predecessor();
  assert(rs_pre);  // must have predecessor
  bool was_noop = false;
  bool was_com = true;
  for (index_type i = 0; (i < rs_pre->noops.length()) && was_com; i++) {
    if (rs_pre->noops[i] == atom.index) was_noop = true;
    else if (trie.del.contains(rs_pre->noops[i])) was_com = false;
  }
  if (was_noop && was_com) {
    for (index_type i = 0; (i < rs_pre->acts.length()) && was_com; i++) {
      if (!instance.commutative(rs_pre->acts[i], trie.index))
	was_com = false;
      if (!instance.lock_compatible(rs_pre->acts[i], trie.index))
	was_com = false;
    }
    if (was_com) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "right-shift cut!" << std::endl;
      std::cerr << "action: " << trie.name
		<< " (" << trie.index << ")" << std::endl;
      std::cerr << "atom: " << atom.name
		<< " (" << atom.index << ")" << std::endl;
      std::cerr << "pre: " << *pre << std::endl;
#endif
      return false;
    }
  }
  return true;
}

State* ParaRSRegState::copy() {
  return new ParaRSRegState(*this);
}

ParaRegState* ParaRSRegState::new_succ() {
  ParaRSRegState* new_s = new ParaRSRegState(instance, heuristic);
  new_s->set_predecessor(this);
  return new_s;
}

State* ParaRSRegState::new_state(const index_set& s, State* p) {
  ParaRSRegState* new_s = new ParaRSRegState(instance, heuristic, s);
  new_s->set_predecessor(p);
  return new_s;
}

State* ParaRSRegState::new_state(const bool_vec& s, State* p) {
  ParaRSRegState* new_s = new ParaRSRegState(instance, heuristic, s);
  new_s->set_predecessor(p);
  return new_s;
}

int ParaRSRegState::compare(const State& s) {
  const ParaRSRegState& rss = (const ParaRSRegState&)s;
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    if (!set[k] && rss.set[k]) return -1;
    else if (set[k] && !rss.set[k]) return 1;
  }
  if (noops.length() < rss.noops.length()) return -1;
  else if (noops.length() > rss.noops.length()) return 1;
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    if (noops.contains(k)) {
      if (!rss.noops.contains(k)) return 1;
    }
    else {
      if (rss.noops.contains(k)) return -1;
    }
  }
  if (acts.length() < rss.acts.length()) return -1;
  else if (acts.length() > rss.acts.length()) return 1;
  for (index_type k = 0; k < instance.n_actions(); k++) {
    if (acts.contains(k)) {
      if (!rss.acts.contains(k)) return 1;
    }
    else {
      if (rss.acts.contains(k)) return -1;
    }
  }
  return 0;
}

void ParaRSRegState::write(std::ostream& s) {
  s << "{";
  for (index_type k = 0; k < acts.length(); k++) {
    if (k > 0) s << ", ";
    s << instance.actions[acts[k]].name;
  }
  for (index_type k = 0; k < noops.length(); k++) {
    if ((k > 0) || (acts.length() > 0)) s << ", ";
    s << "noop" << instance.atoms[noops[k]].name;
  }
  s << "} <- ";
  instance.write_atom_set(s, set);
}

END_HSPS_NAMESPACE
