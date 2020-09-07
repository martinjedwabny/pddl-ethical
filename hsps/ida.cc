
#include "ida.h"

BEGIN_HSPS_NAMESPACE

IDA::IDA(Statistics& s, SearchResult& r)
  : MultiSearchAlgorithm(s, r),
    ttab(0),
    cycle_check(false),
    current_bound(0),
    iteration_limit(count_type_max),
    iteration_count(0)
{
  // done
}

IDA::IDA(Statistics& s, SearchResult& r, HashTable* tt)
  : MultiSearchAlgorithm(s, r),
    ttab(tt),
    cycle_check(false),
    current_bound(0),
    iteration_limit(count_type_max),
    iteration_count(0)
{
  // done
}

IDA::~IDA()
{
  // done
}

NTYPE IDA::start(State& s, NTYPE b)
{
  if (ttab) ttab->clear();
  iteration_count = 0;
  reset();

  start_count();
  NTYPE val = main(s, b);
  stop_count();
  return val;
}

NTYPE IDA::start(State& s)
{
  stats.current_lower_bound(s.est_cost());
  return start(s, s.est_cost());
}

NTYPE IDA::resume(State& s, NTYPE b)
{
  start_count();
  NTYPE val = main(s, b);
  stop_count();
  return val;
}

NTYPE IDA::main(State& s, NTYPE bound)
{
  current_bound = bound;

  while (!solved()) {
    if (INFINITE(current_bound)) {
      if (trace_level > 1) {
	std::cerr << "break on cost = +INF" << std::endl;
      }
      return current_bound;
    }
    if (current_bound > cost_limit) {
      if (trace_level > 1) {
	std::cerr << "break on cost > " << cost_limit << std::endl;
      }
      return current_bound;
    }
    if (iteration_count >= iteration_limit) {
      if (trace_level > 1) {
	std::cerr << "break on iteration count > "
		  << iteration_limit << std::endl;
      }
      return current_bound;
    }
    if (trace_level > 0) {
      std::cerr << "bound = " << PRINT_NTYPE(current_bound) << ": ";
    }
    stats.begin_iteration();
    NTYPE new_bound = new_state(s, current_bound);
    if (break_signal_raised()) return current_bound;
    if (!solved()) stats.current_lower_bound(new_bound);
    result.no_more_solutions(current_bound);
    stats.end_iteration();
    iteration_count += 1;
    current_bound = new_bound;
    if ((trace_level > 0) && !solved()) {
      std::cerr << stats;
      if (ttab) {
	std::cerr << ", TUF: " << ttab->TUF() << ", HCF: " << ttab->HCF();
      }
      std::cerr << std::endl;
    }
  }
  return current_bound;
}

NTYPE IDA::new_state(State& s, NTYPE bound)
{
  assert(!s.is_max());
  stats.create_node(s);
  if (s.is_final()) {
    if (trace_level > 1) {
      std::cerr << "solution (cost = " << s.acc_cost()
		<< ", depth = " << s.depth() << ")" << std::endl;
      if (trace_level > 2) {
	std::cerr << "solution path:" << std::endl;
	s.write_path(std::cerr);
      }
    }
    set_solved(true);
    result.solution(s, current_bound - bound);
    return 0;
  }

  NTYPE c_est = s.est_cost();

  HashTable::Entry* entry = 0;
  if (ttab) {
    entry = ttab->find(s);
    if (entry) c_est = MAX(c_est, entry->cost);
  }

  if (c_est <= bound) {
    if (cycle_check) {
      for (State* sp = s.predecessor(); sp; sp = sp->predecessor())
	if (s.compare(*sp) == 0) return POS_INF;
    }

    stats.expand_node(s);
    if (trace_level > 2) {
      std::cerr << "expanding " << s
		<< " (" << (s.is_max() ? "max, " : "min, ")
		<< c_est << ") at bound " << bound
		<< " and depth " << s.depth()
		<< std::endl;
    }
    else if (trace_level > 1) {
      if ((stats.nodes() % TRACE_LEVEL_2_NOTIFY) == 0) {
	std::cerr << stats;
	if (ttab) {
	  std::cerr << ", TUF: " << ttab->TUF() << ", HCF: " << ttab->HCF();
	}
	std::cerr << std::endl;
      }
    }

    NTYPE val = s.expand(*this, bound);

    if (entry) {
      entry->cost = val;
    }
    else if (ttab) {
      index_type h = ttab->index(s);
      if ((*ttab)[h].state) {
	if ((*ttab)[h].depth > s.depth()) {
	  delete (*ttab)[h].state;
	  (*ttab)[h].state = 0;
	}
      }
      else {
	ttab->inc_occ_count();
      }
      if (!(*ttab)[h].state) {
	(*ttab)[h].state = s.copy();
	(*ttab)[h].depth = s.depth();
	(*ttab)[h].cost = val;
      }
    }

    return val;
  }
  else return c_est;
}

NTYPE IDA::cost() const
{
  return current_bound;
}

END_HSPS_NAMESPACE
