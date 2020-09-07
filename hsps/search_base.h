#ifndef SEARCH_BASE_H
#define SEARCH_BASE_H

#include "config.h"
#include "search.h"
#include "stats.h"
#include "name.h"

BEGIN_HSPS_NAMESPACE

class SearchResult {
 public:
  SearchResult() { };
  virtual ~SearchResult();

  // solution found
  virtual void solution(State& s, NTYPE cost) = 0;

  // there are no more solutions (search done)
  virtual void no_more_solutions(NTYPE cost) = 0;

  // return true iff more solutions are desired
  virtual bool  more() = 0;
};

class Result : public SearchResult {
 public:
  enum stop_condition { stop_at_first,
			stop_at_nth,
			stop_at_all_optimal,
			stop_at_all };

 private:
  stop_condition sc;
  count_type find_n;
  PlanSet*    plans;

  count_type n_found;
  NTYPE      min_cost;
  NTYPE      max_ex;

 public:
  Result() : sc(stop_at_first), find_n(1), plans(0), n_found(0),
    min_cost(POS_INF), max_ex(NEG_INF) { };
  Result(PlanSet* s) : sc(stop_at_first), find_n(1), plans(s), n_found(0),
    min_cost(POS_INF), max_ex(NEG_INF) { };
  virtual ~Result() { };

  void set_stop_condition(stop_condition c);
  void set_n_to_find(count_type n);
  void set_plan_set(PlanSet* s);

  count_type solution_count();
  bool       search_space_exhausted();

  void reset();

  // solution found
  virtual void solution(State& s, NTYPE cost);

  // there are no more solutions (search done)
  virtual void no_more_solutions(NTYPE cost);

  // return true iff more solutions are desired
  virtual bool  more();
};

class SearchStats : public Search {
 protected:
  Statistics& stats;

  NTYPE cost_limit;
  count_type node_limit;
  count_type node_count;
  count_type work_limit;
  count_type work_count;
  count_type zero_eval_count;

  void  start_count();
  void  stop_count();
  void  reset();

 public:
  SearchStats(Statistics& s);
  SearchStats(Statistics& s, NTYPE limit);
  virtual ~SearchStats();

  void set_cost_limit(NTYPE c_max);
  NTYPE get_cost_limit() const;
  bool cost_limit_reached() const;
  void set_node_limit(count_type n);
  count_type get_node_limit() const;
  bool node_limit_reached() const;
  void set_work_limit(count_type n);
  count_type get_work_limit() const;
  bool work_limit_reached() const;

  bool break_signal_raised() const;

  virtual NTYPE cost() const = 0;
  count_type work() const;
};

class SearchAlgorithm : public SearchStats {
  bool is_solved;
  bool is_optimal;
 protected:
  SearchResult& result;
  const Name* problem_name;
  int   trace_level;

  void set_solved(bool s, bool o);
  void set_solved(bool s); // default: o = true
  // SearchStats::reset + set_solved(false, false)
  void reset();

 public:
  static int default_trace_level;

  void set_problem_name(const Name* n);
  void set_trace_level(int level);

  SearchAlgorithm(Statistics& s, SearchResult& r);
  SearchAlgorithm(Statistics& s, SearchResult& r, NTYPE limit);
  virtual ~SearchAlgorithm();

  virtual NTYPE start(State& s, NTYPE b) = 0;
  virtual NTYPE start(State& s) = 0;

  virtual bool solved() const;
  virtual bool optimal() const;
  virtual bool done() const;
};

class SingleSearchAlgorithm : public SearchAlgorithm {
 public:
  SingleSearchAlgorithm(Statistics& s, SearchResult& r)
    : SearchAlgorithm(s, r) { };
  SingleSearchAlgorithm(Statistics& s, SearchResult& r, NTYPE limit)
    : SearchAlgorithm(s, r, limit) { };
  virtual ~SingleSearchAlgorithm() { };

  virtual NTYPE resume() = 0;
};

class MultiSearchAlgorithm : public SearchAlgorithm {
 public:
  MultiSearchAlgorithm(Statistics& s, SearchResult& r)
    : SearchAlgorithm(s, r) { };
  MultiSearchAlgorithm(Statistics& s, SearchResult& r, NTYPE limit)
    : SearchAlgorithm(s, r, limit) { };
  virtual ~MultiSearchAlgorithm() { };

  virtual NTYPE resume(State& s, NTYPE b) = 0;
  virtual NTYPE resume(State& s);
};

END_HSPS_NAMESPACE

#endif
