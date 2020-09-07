
#include "itest.h"

BEGIN_HSPS_NAMESPACE

ITest::ITest(Statistics& s, SearchResult& res, HashTable& tab)
  : SearchAlgorithm(s, res),
    solved_tab(tab),
    current_bound(0),
    cycle_check(false)
{
  // done
}

NTYPE ITest::start(State& s, NTYPE b)
{
  reset();
  solved_tab.clear();

  start_count();
  NTYPE val = main(s, b);
  stop_count();
  return val;
}

NTYPE ITest::start(State& s)
{
  return start(s, s.est_cost());
}

NTYPE ITest::main(State& s, NTYPE bound) {
  current_bound = bound;

  while (!solved() && (current_bound < cost_limit)) {
    if (INFINITE(current_bound)) {
      if (trace_level > 1) {
	std::cerr << "break on cost = +INF" << std::endl;
      }
      return current_bound;
    }
    if (current_bound > get_cost_limit()) {
      if (trace_level > 1) {
	std::cerr << "break on cost > " << cost_limit << std::endl;
      }
      return current_bound;
    }
    if (trace_level > 1) {
      std::cerr << "bound = " << current_bound << std::endl;
    }
    NTYPE new_bound = new_state(s, current_bound);
    if (break_signal_raised()) return current_bound;
    current_bound = new_bound;
    if ((trace_level > 0) && !solved()) std::cerr << stats << std::endl;
  }
  return current_bound;
}

NTYPE ITest::new_state(State& s, NTYPE bound) {
  stats.create_node(s);

  // check if the node is final
  if (s.is_final()) {
    set_solved(true);
    result.solution(s, current_bound - bound);
    return 0;
  }

  // check if node is in solved table
  HashTable::Entry* entry = solved_tab.find(s);
  if (entry) {
    if (entry->cost <= bound) {
      set_solved(true);
      return entry->cost;
    }
  }

  // check for cycle
  if (cycle_check) {
    for (State* sp = s.predecessor(); sp; sp = sp->predecessor())
      if (s.compare(*sp) == 0) return POS_INF;
  }

  // if none of the above, node will be expanded
  stats.expand_node(s);
  if (trace_level > 2) {
    std::cerr << "expanding " << s
	      << " (" << (s.is_max() ? "max, " : "min, ")
	      << s.est_cost() << ")" << " at " << bound
	      << std::endl;
  }
  else if (trace_level > 1) {
    if ((stats.nodes() % TRACE_LEVEL_2_NOTIFY) == 0)
      std::cerr << stats << std::endl;
  }

  if (s.is_max()) {
    maxTest sub_search(stats, result, solved_tab, trace_level);
    NTYPE val = s.expand(sub_search, bound);
    if (sub_search.solved()) {
      solved_tab.insert(s, val);
      set_solved(true);
    }
    return val;
  }
  else {
    NTYPE val = s.expand(*this, bound);
    if (solved()) solved_tab.insert(s, val);
    if (!solved()) s.store(val, false);
    return val;
  }
}

NTYPE ITest::cost() const {
  return current_bound;
}

bool ITest::done() const {
  return (solved() || break_signal_raised());
}

maxTest::maxTest(Statistics& s, SearchResult& res, HashTable& tab)
  : stats(s),
    result(res),
    is_solved(true),
    solved_tab(tab),
    trace_level(0)
{
  // done
}

maxTest::maxTest(Statistics& s, SearchResult& res, HashTable& tab, int level)
  : stats(s),
    result(res),
    is_solved(true),
    solved_tab(tab),
    trace_level(level)
{
  // done
}

NTYPE maxTest::new_state(State& s, NTYPE bound) {
  if (trace_level > 2) {
    std::cerr << "sub state: " << s << std::endl;
  }

  assert(!s.is_max());

  ITest search(stats, result, solved_tab);
  if (trace_level > 3)
    search.set_trace_level(trace_level);
  else
    search.set_trace_level(0);
  NTYPE val = search.new_state(s, bound);
  is_solved = is_solved && search.solved();

  if (trace_level > 2) {
    std::cerr << "sub state (" << s << "): cost = " << val
	      << ", solved = " << (search.solved() ? 1 : 0)
	      << std::endl;
  }

  return val;
}

bool maxTest::solved() const {
  return is_solved;
}

bool maxTest::optimal() const {
  return false;
}

bool maxTest::done() const {
  return (!is_solved || stats.break_signal_raised());
}

LDFS::LDFS(Statistics& s, SearchResult& res, HashTable& tab)
  : SearchAlgorithm(s, res),
    solved_tab(tab)
{
  // done
}

NTYPE LDFS::main(State& s)
{
  in_max = s.is_max();
  c_root = s.est_cost();
  while (!solved() && !stats.break_signal_raised()) {
    if (trace_level > 1) {
      std::cerr << "root est. cost = " << c_root << std::endl;
    }
    stats.begin_iteration();
    new_state(s, s.est_cost());
    stats.end_iteration();
    if ((trace_level > 0) && !solved()) std::cerr << stats << std::endl;
    s.reevaluate();
    c_root = s.est_cost();
  }
  return s.est_cost();
}

NTYPE LDFS::start(State& s, NTYPE b)
{
  return start(s);
}

NTYPE LDFS::start(State& s)
{
  reset();
  solved_tab.clear();
  start_count();
  NTYPE val = main(s);
  stop_count();
  return val;
}

NTYPE LDFS::new_state(State& s, NTYPE bound)
{
  stats.create_node(s);

  // check if the node is final
  if (s.is_final()) {
    set_solved(true);
    result.solution(s, s.acc_cost());
    return 0;
  }

  // check if node is in solved table
  HashTable::Entry* entry = solved_tab.find(s);
  if (entry) {
    set_solved(true);
    return entry->cost;
  }

  stats.expand_node(s);
  if (trace_level > 2) {
    std::cerr << "expanding " << s
	      << " (" << (s.is_max() ? "max, " : "min, ")
	      << s.est_cost() << ")" << std::endl;
  }

  bool pred_in_max = in_max;
  NTYPE val = 0;
  if (s.is_max()) {
    in_max = true;
    set_solved(true);
    val = s.expand(*this, s.est_cost());
    if (solved()) {
      solved_tab.insert(s, val);
    }
  }
  else {
    in_max = false;
    set_solved(false);
    val = s.expand(*this, s.est_cost());
    if (solved()) {
      solved_tab.insert(s, val);
    }
    else {
      if (trace_level > 2) {
	std::cerr << "storing " << s
		  << " (" << (s.is_max() ? "max, " : "min, ")
		  << s.est_cost() << ")" << " = " << val
		  << std::endl;
      }
      s.store(val, false);
    }
  }
  in_max = pred_in_max;
  return val;
}

NTYPE LDFS::cost() const
{
  return c_root;
}

bool LDFS::done() const
{
  if (stats.break_signal_raised()) return true;
  if (in_max) {
    return !solved();
  }
  else {
    return solved();
  }
}

END_HSPS_NAMESPACE
