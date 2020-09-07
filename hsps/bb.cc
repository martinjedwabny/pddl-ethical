
#include "bb.h"

BEGIN_HSPS_NAMESPACE

DFS::DFS(Statistics& s, SearchResult& r)
  : SearchAlgorithm(s, r),
    cycle_check(false),
    store_cost(false),
    upper_bound(POS_INF),
    ttab(0)
{
  // done
}

DFS::DFS(Statistics& s, SearchResult& r, HashTable* tt)
  : SearchAlgorithm(s, r),
    cycle_check(false),
    store_cost(false),
    upper_bound(POS_INF),
    ttab(tt)
{
  // done
}

DFS::~DFS()
{
  // done
}

bool DFS::within_upper_bound(NTYPE c_acc, NTYPE c_est)
{
  return ((c_acc + c_est) <= upper_bound);
}

void DFS::update_upper_bound(NTYPE c)
{
  // ignore - plain DFS uses a fixed upper bound
}

NTYPE DFS::start(State& s, NTYPE b)
{
  set_upper_bound(b);
  return start(s);
}

NTYPE DFS::start(State& s)
{
  reset();
  if (ttab) {
    ttab->clear();
  }
  start_count();
  NTYPE val = new_state(s, upper_bound);
  stop_count();
  if (solved())
    set_solved(true, true);
  result.no_more_solutions(upper_bound);
  return val;
}

NTYPE DFS::new_state(State& s, NTYPE bound) {
  assert(!s.is_max());
  stats.create_node(s);

  if (s.is_final()) {
    assert(s.acc_cost() <= upper_bound);
    if (trace_level > 1) {
      std::cerr << "solution (cost: " << s.acc_cost() << ", " << stats << ")"
		<< std::endl;
    }
    set_solved(true, false);
    update_upper_bound(s.acc_cost());
    result.solution(s, s.acc_cost());
    return 0;
  }

  NTYPE c_acc = s.acc_cost();
  NTYPE c_est = s.est_cost();

  HashTable::Entry* entry = 0;
  if (ttab) {
    entry = ttab->find(s);
    if (entry) c_est = MAX(c_est, entry->cost);
  }

  // bound should not be tighter than upper_bound
  assert(c_acc + bound >= upper_bound);

  if (within_upper_bound(c_acc, c_est)) {
    if (cycle_check) {
      for (State* sp = s.predecessor(); sp; sp = sp->predecessor())
	if (s.compare(*sp) == 0) return POS_INF;
    }

    stats.expand_node(s);
    if (trace_level > 2) {
      std::cerr << "expanding " << s
		<< " (" << (s.is_max() ? "max, " : "min, ")
		<< c_acc << " + " << c_est << ") at bound "
		<< upper_bound << " and depth " << s.depth()
		<< " (done = " << done() << ")"
		<< std::endl;
    }
    else if (trace_level > 1) {
      if ((stats.nodes() % TRACE_LEVEL_2_NOTIFY) == 0)
	std::cerr << stats << std::endl;
    }

    NTYPE val = s.expand(*this, upper_bound - c_acc);

    if (store_cost) {
      if (val > c_est) {
	s.store(val, false);
      }
    }

    if (ttab) {
      if (entry) {
	entry->cost = val;
      }
      else {
	index_type h = ttab->index(s);
	if (h != no_such_index) {
	  if ((*ttab)[h].state) {
	    if ((*ttab)[h].depth > s.depth()) {
	      delete (*ttab)[h].state;
	      (*ttab)[h].state = 0;
	    }
	  }
	  if (!(*ttab)[h].state) {
	    (*ttab)[h].state = s.copy();
	    (*ttab)[h].depth = s.depth();
	    (*ttab)[h].cost = val;
	  }
	}
      }
    }

    return val;
  }
  else {
    return c_est;
  }
}

NTYPE DFS::cost() const
{
  return upper_bound;
}

DFS_BB::DFS_BB(Statistics& s, SearchResult& r)
  : DFS(s, r)
{
  // done
}

DFS_BB::DFS_BB(Statistics& s, SearchResult& r, HashTable* tt)
  : DFS(s, r, tt)
{
  // done
}

DFS_BB::~DFS_BB()
{
  // done
}

bool DFS_BB::within_upper_bound(NTYPE c_acc, NTYPE c_est)
{
  return (solved() ?
	  (c_acc + c_est < upper_bound) :
	  (c_acc + c_est <= upper_bound));
}

void DFS_BB::update_upper_bound(NTYPE c)
{
  upper_bound = c;
}

END_HSPS_NAMESPACE
