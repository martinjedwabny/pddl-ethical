
#include "atomset.h"

BEGIN_HSPS_NAMESPACE

index_type AtomSet::max_set_size_encountered = 0;

index_type AtomSet::count() {
  size = 0;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (set[k]) size += 1;
  return size;
}

void AtomSet::add(const index_set& s)
{
  for (index_type k = 0; k < s.length(); k++) {
    if (!set[s[k]]) size += 1;
    set[s[k]] = true;
  }
}

void AtomSet::add(const bool_vec& s)
{
  for (index_type k = 0; k < instance.n_atoms(); k++) if (s[k]) {
    if (!set[k]) size += 1;
    set[k] = true;
  }
}

void AtomSet::del(const index_set& s)
{
  for (index_type k = 0; k < s.length(); k++) {
    if (set[s[k]]) size -= 1;
    set[s[k]] = false;
  }
}

void AtomSet::del(const bool_vec& s)
{
  for (index_type k = 0; k < instance.n_atoms(); k++) if (s[k]) {
    if (set[k]) size -= 1;
    set[k] = false;
  }
}

bool AtomSet::is_init_all()
{
  for (index_type k = 0; k < instance.n_atoms(); k++) if (set[k])
    if (!instance.atoms[k].init) return false;
  return true;
}

bool AtomSet::is_init_some()
{
  for (index_type k = 0; k < instance.n_atoms(); k++) if (set[k])
    if (!instance.atoms[k].init) return true;
  return false;
}

bool AtomSet::is_goal_all()
{
  for (index_type k = 0; k < instance.n_atoms(); k++) if (set[k])
    if (!instance.atoms[k].goal) return false;
  return true;
}

bool AtomSet::is_goal_some()
{
  for (index_type k = 0; k < instance.n_atoms(); k++) if (set[k])
    if (!instance.atoms[k].goal) return true;
  return false;
}

AtomSet::AtomSet(Instance& i)
  : instance(i),
    set(false, instance.n_atoms()),
    size(0)
{
  // done
}

AtomSet::AtomSet(Instance& i, const index_set& g)
  : instance(i),
    set(false, instance.n_atoms()),
    size(g.length())
{
  set.insert(g);
}

AtomSet::AtomSet(Instance& i, const bool_vec& g)
  : instance(i),
    set(g),
    size(0)
{
  size = set.count(true);
}

AtomSet::AtomSet(const AtomSet& s)
  : instance(s.instance),
    set(s.set),
    size(s.size)
{
  // done
}

AtomSet::~AtomSet()
{
  // done
}

index_vec& AtomSet::copy_to(index_vec& s)
{
  s.set_length(0);
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (set[k]) s.append(k);
  return s;
}

bool_vec& AtomSet::copy_to(bool_vec& s)
{
  s = set;
  return s;
}

NTYPE AtomSet::split
(index_set g, // indices of atoms (goals) in the set
 index_type p, // next "undecided" atom (pointer into g)
 index_type n_out, // # of atoms removed
 index_type limit, // subset size (max # of atoms in out set)
 StateFactory& f,
 State* sp,
 Search& s, // search & bound to call with each subset
 NTYPE bound)
{
  // if # of atoms remaining in set is below limit, output the subset
  if ((g.length() - n_out) <= limit) {
#ifdef SEARCH_EXTRA_STATS
    n_succ_created += 1;
#endif
    State* sub_s = f.new_state(set, sp);
    NTYPE c_sub = s.new_state(*sub_s, bound);
    delete sub_s;
    return c_sub;
  }

  // if # of undecided atoms (in g, beyond p) is greater than # of atoms
  // that need to be switched out (|g| - limit) - n_out, branch on next
  // atom in g
  else if ((g.length() - p) > ((g.length() - limit) - n_out)) {
    // keep atom g[p] in set: g[p] becomes decided, n_out remains the same
    NTYPE c_in = split(g, p + 1, n_out, limit, f, sp, s, bound);

    // if the in-branch was unsolved (this implies c_in > bound),
    // or if a break has occurred, return
    if (s.done()) return c_in;

    // take atom g[p] out of set: g[p] becomes decided, n_out increases by 1
    set[g[p]] = false;
    NTYPE c_out = split(g, p + 1, n_out + 1, limit, f, sp, s, bound);
    // restore set
    set[g[p]] = true;

    // return max cost
    return MAX(c_in, c_out);
  }

  // otherwise, # of atoms decided-in equals limit, so all undecided atoms
  // must be out
  else {
#ifdef SEARCH_EXTRA_STATS
    n_succ_created += 1;
#endif
    for (index_type k = p; k < g.length(); k++) set[g[k]] = false;
    State* sub_s = f.new_state(set, sp);
    NTYPE c_sub = s.new_state(*sub_s, bound);
    for (index_type k = p; k < g.length(); k++) set[g[k]] = true;
    delete sub_s;
    return c_sub;
  }
}

NTYPE AtomSet::split
(index_type limit, StateFactory& f, State* sp, Search& s, NTYPE bound)
{
#ifdef SEARCH_EXTRA_STATS
    n_succ_created = 0;
#endif
  index_set g;
  copy_to(g);
  NTYPE c = split(g, 0, 0, limit, f, sp, s, bound);
#ifdef SEARCH_EXTRA_STATS
  rmaxx_count += 1;
  rmaxx_size += size;
  rmaxx_succ += n_succ_created;
#ifdef PRINT_EXTRA_STATS
  std::cout << "RMAXX " << size
	    << " " << n_succ_created
	    << " " << s.done()
	    << std::endl;
#endif
#endif
  return c;
}

// random split

NTYPE AtomSet::split_random
(index_set g, // indices of atoms (goals) in the set
 index_type p, // next "undecided" atom (pointer into g)
 index_type n_out, // # of atoms removed
 index_type limit, // subset size (max # of atoms in out set)
 StateFactory& f,
 State* sp,
 Search& s, // search & bound to call with each subset
 NTYPE bound,
 RNG& r)
{
  // if # of atoms remaining in set is below limit, output the subset
  if ((g.length() - n_out) <= limit) {
    State* sub_s = f.new_state(set, sp);
    NTYPE c_sub = s.new_state(*sub_s, bound);
    delete sub_s;
    return c_sub;
  }

  // if # of undecided atoms (in g, beyond p) is greater than # of atoms
  // that need to be switched out (|g| - limit) - n_out, randomly choose
  // if next atom in g goes in or out
  else if ((g.length() - p) > ((g.length() - limit) - n_out)) {
    if (r.random() % 2) {
      // keep atom g[p] in set: g[p] becomes decided, n_out remains the same
      return split_random(g, p + 1, n_out, limit, f, sp, s, bound, r);
    }
    else {
      // take atom g[p] out of set: g[p] becomes decided, n_out increases by 1
      set[g[p]] = false;
      NTYPE c_out = split_random(g, p + 1, n_out + 1, limit, f, sp, s, bound, r);

      // restore before returning
      set[g[p]] = true;
      return c_out;
    }
  }

  // otherwise, # of atoms decided-in equals limit, so all undecided atoms
  // must be out
  else {
    for (index_type k = p; k < g.length(); k++) set[g[k]] = false;
    State* sub_s = f.new_state(set, sp);
    NTYPE c_sub = s.new_state(*sub_s, bound);
    for (index_type k = p; k < g.length(); k++) set[g[k]] = true;
    delete sub_s;
    return c_sub;
  }
}

NTYPE AtomSet::split_random
(index_type limit, StateFactory& f, State* sp, Search& s, NTYPE bound, RNG& r)
{
  index_set g;
  copy_to(g);
  return split_random(g, 0, 0, limit, f, sp, s, bound, r);
}

int AtomSet::compare(const AtomSet& s)
{
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    if (!set[k] && s.set[k]) return -1;
    else if (set[k] && !s.set[k]) return 1;
  }
  return 0;
}

index_type AtomSet::hash()
{
  index_type val = instance.atom_set_hash(set);
  return val;
}

AtomSetState::AtomSetState(Instance& i, Heuristic& h)
  : AtomSet(i),
    heuristic(h),
    est(0)
{
  // done
}

AtomSetState::AtomSetState(Instance& i, Heuristic& h, const index_set& g)
  : AtomSet(i, g),
    heuristic(h),
    est(0)
{
  reevaluate();
}

AtomSetState::AtomSetState(Instance& i, Heuristic& h, const bool_vec& g)
  : AtomSet(i, g),
    heuristic(h),
    est(0)
{
  reevaluate();
}

AtomSetState::AtomSetState(const AtomSetState& s)
  : AtomSet(s),
    State(s),
    heuristic(s.heuristic),
    est(s.est)
{
  // done
}

AtomSetState::~AtomSetState()
{
  // does nothing
}

NTYPE AtomSetState::eval()
{
  est = heuristic.eval(set);
  return est;
}

NTYPE AtomSetState::eval(NTYPE bound)
{
  return heuristic.eval_to_bound(set, bound);
}

NTYPE AtomSetState::eval_with_trace_level(int level)
{
  heuristic.set_trace_level(level);
  est = heuristic.eval(set);
  heuristic.set_trace_level(0);
  return est;
}

void AtomSetState::apply_path_max()
{
  if (pre) {
    if (pre->est_cost() > (delta_cost() + est)) {
      est = (pre->est_cost() - delta_cost());
    }
  }
}

NTYPE AtomSetState::est_cost()
{
  return est;
}

bool AtomSetState::is_final()
{
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (set[k] && !instance.atoms[k].init) return false;
  if (size > max_set_size_encountered) max_set_size_encountered = size;
  return true;
}

bool AtomSetState::is_max()
{
  return false;
}

void AtomSetState::store(NTYPE cost, bool opt)
{
  heuristic.store(set, cost, opt);
}

void AtomSetState::reevaluate()
{
  est = heuristic.eval(set);
}

int AtomSetState::compare(const State& s)
{
  const AtomSetState& as = (const AtomSetState&)s;
  return AtomSet::compare(as);
}

index_type AtomSetState::hash()
{
  index_type val = instance.atom_set_hash(set);
  return val;
}

void AtomSetState::write(std::ostream& s)
{
  instance.write_atom_set(s, set);
}

void AtomSetState::write_eval(std::ostream& s, char* p, bool e)
{
  heuristic.write_eval(set, s, p, e);
}

END_HSPS_NAMESPACE
