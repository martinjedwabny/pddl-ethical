
#include "search_base.h"
#include "heuristic.h"

BEGIN_HSPS_NAMESPACE

SearchResult::~SearchResult()
{
  // done
}

count_type Result::solution_count()
{
  return n_found;
}

bool Result::search_space_exhausted()
{
  return (max_ex == POS_INF);
}

void Result::reset()
{
  n_found = 0;
  min_cost = POS_INF;
  max_ex = NEG_INF;
}

void Result::set_stop_condition(stop_condition c)
{
  sc = c;
}

void Result::set_n_to_find(count_type n)
{
  find_n = n;
  sc = stop_at_nth;
}

void Result::set_plan_set(PlanSet* s)
{
  plans = s;
}

void Result::solution(State& s, NTYPE cost)
{
  // count the solution
  n_found += 1;
  min_cost = MIN(min_cost, cost);

  if (plans) {
    Plan* p = plans->new_plan();
    if (p) {
      s.insert_path(*p);
      p->end();
    }
  }
}

void Result::no_more_solutions(NTYPE cost)
{
  max_ex = MAX(max_ex, cost);
}

bool Result::more()
{
  switch (sc) {
  case stop_at_first:
    return false;
  case stop_at_nth:
    return (n_found < find_n);
  case stop_at_all_optimal:
    return (max_ex < min_cost);
  case stop_at_all:
    return true;
  default:
    assert(0);
  }
}

SearchStats::SearchStats(Statistics& s)
  : stats(s),
    cost_limit(POS_INF),
    node_limit(count_type_max),
    work_limit(count_type_max),
    zero_eval_count(0)
{
  // done
}

SearchStats::SearchStats(Statistics& s, NTYPE limit)
  : stats(s),
    cost_limit(limit),
    node_limit(count_type_max),
    work_limit(count_type_max),
    zero_eval_count(0)
{
  // done
}

SearchStats::~SearchStats()
{
  // done
}

void SearchStats::set_cost_limit(NTYPE c_max)
{
  cost_limit = c_max;
}

NTYPE SearchStats::get_cost_limit() const
{
  return cost_limit;
}

bool SearchStats::cost_limit_reached() const
{
  return (cost() > cost_limit);
}

void SearchStats::set_node_limit(count_type n)
{
  node_limit = n;
}

count_type SearchStats::get_node_limit() const
{
  return node_limit;
}

bool SearchStats::node_limit_reached() const
{
  return (stats.nodes() + node_count > node_limit);
}

void SearchStats::set_work_limit(count_type n)
{
  work_limit = n;
}

count_type SearchStats::get_work_limit() const
{
  return work_limit;
}

bool SearchStats::work_limit_reached() const
{
  return (work() > work_limit);
}

count_type SearchStats::work() const
{
  return (Heuristic::eval_count - zero_eval_count) + work_count;
}

void SearchStats::reset()
{
  zero_eval_count = Heuristic::eval_count;
  work_count = 0;
  node_count = 0;
}

void SearchStats::start_count()
{
  if (stats.run_level() == 0) {
    zero_eval_count = Heuristic::eval_count;
  }
  stats.start();
}

void SearchStats::stop_count()
{
  stats.stop();
  if (stats.run_level() == 0) {
    work_count += (Heuristic::eval_count - zero_eval_count);
    node_count += stats.nodes();
  }
}

bool SearchStats::break_signal_raised() const
{
  return (stats.break_signal_raised() ||
	  node_limit_reached() ||
	  work_limit_reached());
}


int SearchAlgorithm::default_trace_level = 0;

SearchAlgorithm::SearchAlgorithm(Statistics& s, SearchResult& r)
  : SearchStats(s),
    result(r),
    is_solved(false),
    trace_level(default_trace_level)
{
  // done
}

SearchAlgorithm::SearchAlgorithm(Statistics& s, SearchResult& r, NTYPE limit)
  : SearchStats(s, limit),
    result(r),
    is_solved(false),
    trace_level(default_trace_level)
{
  // done
}

SearchAlgorithm::~SearchAlgorithm()
{
  // done
}

void SearchAlgorithm::set_problem_name(const Name* n)
{
  problem_name = n;
}

void SearchAlgorithm::set_trace_level(int level)
{
  trace_level = level;
}

void SearchAlgorithm::reset()
{
  SearchStats::reset();
  set_solved(false, false);
}

bool SearchAlgorithm::solved() const
{
  return is_solved;
}

bool SearchAlgorithm::optimal() const
{
  return (is_solved && is_optimal);
}

bool SearchAlgorithm::done() const
{
  return ((solved() && !result.more()) || break_signal_raised());
}

void SearchAlgorithm::set_solved(bool s, bool o)
{
  is_solved = s;
  is_optimal = o;
}

void SearchAlgorithm::set_solved(bool s)
{
  is_solved = s;
  is_optimal = true;
}

NTYPE MultiSearchAlgorithm::resume(State& s)
{
  s.reevaluate();
  return resume(s, s.est_cost());
}

END_HSPS_NAMESPACE
