#ifndef PLANS_H
#define PLANS_H

#include "config.h"
#include "problem.h"
#include "search_base.h"
#include "exec.h"

BEGIN_HSPS_NAMESPACE

class ActionSequence : public index_vec, public Plan {
 public:
  ActionSequence() : index_vec(no_such_index, 0) { };
  ActionSequence(const index_vec& vec) : index_vec(vec) { };
  ActionSequence(const ActionSequence& seq) : index_vec(seq) { };
  virtual ~ActionSequence() { };
  virtual void protect(index_type atom);
  virtual void insert(index_type act);
  virtual void advance(NTYPE delta);
  virtual void end();
  virtual void output(Plan& to);
};

class ActionSequenceSet : public lvector<ActionSequence>, public PlanSet {
 public:
  ActionSequenceSet() : lvector<ActionSequence>() { };
  virtual ~ActionSequenceSet() { };

  virtual Plan* new_plan();
  virtual void  output(PlanSet& to);
};

class CountActions
: public lvector<count_type>, public Plan, public PlanSet
{
 public:
  CountActions() : lvector<count_type>(0, 0) { };
  CountActions(index_type n) : lvector<count_type>(0, n) { };
  virtual ~CountActions() { };

  void reset() { assign_value(0, length()); };
  void reset(index_type n) { assign_value(0, n); };
  count_type sum();

  virtual Plan* new_plan();
  virtual void protect(index_type atom);
  virtual void insert(index_type act);
  virtual void advance(NTYPE delta);
  virtual void end();
};

class ApxResult : public Result {
  bool min_sol;
  index_type sol_depth;
 public:
  ApxResult() : min_sol(false), sol_depth(0) { };
  virtual ~ApxResult() { };

  bool min_solution() { return min_sol; };
  index_type solution_depth() { return sol_depth; };

  virtual void solution(State& s, NTYPE cost);
  virtual bool  more();
};

class Print : public Plan, public PlanSet {
  Instance&  instance;
  ::std::ostream&   to;

  NTYPE      current_t;
  amt_vec    peak_use;
  amt_vec    res_cons;
  count_type n_actions;

  count_type n_plans;
  NTYPE      sum_cost;

 public:
  static bool print_noops;
  static bool decimal_time;

  Print(Instance& i, ::std::ostream& s);
  virtual ~Print();

  virtual Plan* new_plan();
  virtual void protect(index_type atom);
  virtual void insert(index_type act);
  virtual void advance(NTYPE delta);
  virtual void end();
};

class PrintActions : public Plan, public PlanSet {
 protected:
  Instance& instance;
  ::std::ostream&   to;
  char    action_sep;
  bool  first_action;
  char      plan_sep;
  bool    first_plan;

 public:
  PrintActions(Instance& i, ::std::ostream& s);
  PrintActions(Instance& i, ::std::ostream& s, char as);
  PrintActions(Instance& i, ::std::ostream& s, char as, char ps);
  virtual ~PrintActions();

  virtual Plan* new_plan();
  virtual void protect(index_type atom);
  virtual void insert(index_type act);
  virtual void advance(NTYPE delta);
  virtual void end();
};

class PrintActionSetNames : public PrintActions {
 public:
  PrintActionSetNames(Instance& i, ::std::ostream& s)
    : PrintActions(i, s) { };
  PrintActionSetNames(Instance& i, ::std::ostream& s, char p)
    : PrintActions(i, s, p) { };
  virtual ~PrintActionSetNames() { };

  virtual void insert(index_type act);
};

class PrintPDDL : public Plan, public PlanSet {
  Instance&  instance;
  ::std::ostream& to;
  NTYPE    start_t;

 public:
  PrintPDDL(Instance& i, ::std::ostream& s);
  virtual ~PrintPDDL();

  virtual Plan* new_plan();

  virtual void protect(index_type atom);
  virtual void insert(index_type act);
  virtual void advance(NTYPE delta);
  virtual void end();
};

class PrintIPC : public Plan, public PlanSet {
  Instance&  instance;
  ::std::ostream& to;
  bool   temporal;
  NTYPE  epsilon;
  bool   included_epsilon;
  bool   strict_separation;

  NTYPE  start_t;
  bool   occ_current_t;

 public:
  PrintIPC(Instance& i, ::std::ostream& s);
  virtual ~PrintIPC();

  void set_epsilon(NTYPE e, bool included, bool strict);

  virtual Plan* new_plan();

  virtual void protect(index_type atom);
  virtual void insert(index_type act);
  virtual void advance(NTYPE delta);
  virtual void end();
};

class PrintAssoc : public Plan {
  Instance&  instance;
  ::std::ostream& to;

 public:
  PrintAssoc(Instance& i, ::std::ostream& s);
  virtual ~PrintAssoc();

  virtual void protect(index_type atom);
  virtual void insert(index_type act);
  virtual void advance(NTYPE delta);
  virtual void end();
};

class Store : public Result, public ScheduleSet {
  state_vec solutions;

 public:
  Store(Instance& i);
  virtual ~Store();

  void reset();
  void clear();

  virtual void solution(State& s, NTYPE cost);

  index_type n_solutions() { return solutions.length(); };
  State*     solution(index_type k);
  index_type n_plans() { return length(); };
  Schedule*  plan(index_type k);

  virtual void output(PlanSet& to);
  virtual void output(Result& to);
};

class StoreFilter : public SearchResult {
 protected:
  Store& store;
 public:
  StoreFilter(Store& s) : store(s) { };
  virtual ~StoreFilter() { };

  virtual void solution(State& s, NTYPE cost) = 0;
  virtual void no_more_solutions(NTYPE cost);
  virtual bool more();
};

class StoreMinCost : public StoreFilter {
 public:
  StoreMinCost(Store& s) : StoreFilter(s) { };
  virtual ~StoreMinCost() { };

  virtual void solution(State& s, NTYPE cost);
};

class StoreDistinct : public StoreFilter {
  Instance&  instance;
 public:
  static count_type n_discarded;

  StoreDistinct(Instance& i, Store& s) : StoreFilter(s), instance(i) { };
  virtual ~StoreDistinct() { };

  virtual void solution(State& s, NTYPE cost);
};

class Conflicts : public Result {
  Instance&  instance;
  index_set  nec_del;
  index_set  pos_del;
  amt_vec    min_peak;
  amt_vec    max_peak;

  bool       first;
  bool       need_to_clear;

 public:
  Conflicts(Instance& ins);
  virtual ~Conflicts();

  const index_set& nec_deleted() const { return nec_del; };
  const index_set& pos_deleted() const { return pos_del; };
  const amt_vec&   min_peak_resource_use() const { return min_peak; };
  const amt_vec&   max_peak_resource_use() const { return max_peak; };

  void clear();
  virtual void solution(State& s, NTYPE cost);
  virtual void no_more_solutions();
};

END_HSPS_NAMESPACE

#endif
