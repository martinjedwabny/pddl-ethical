#ifndef SEARCH_H
#define SEARCH_H

#include "config.h"
#include "index_type.h"
#include "numeric_type.h"
#include "name.h"

BEGIN_HSPS_NAMESPACE

class Search;
class Plan;

class State {
 protected:
  State* pre;

 public:
  State() : pre(0) { };
  State(const State& s) : pre(s.pre) { };
  virtual ~State();

  // a pointer to the predecessor of this state
  virtual const State* predecessor() const;
  virtual State* predecessor();
  virtual void set_predecessor(State* p);

  // true IFF the state is "encapsulated", meaning it contains within
  // itself ALL information about the path that lead to the state, or in
  // other words, that the state is independent of the value of its
  // predecessor pointer. default is FALSE, but only because that's what
  // holds for the majority of State classes.
  virtual bool  is_encapsulated();

  // the delta cost of the state, defined as the difference in acc. cost
  // between this state and its predecessor
  virtual NTYPE delta_cost() = 0;

  // the accumulated cost of the state, defined as the sum of delta costs
  // along the path leading to the state (default implementation sums delta
  // cost along predecessor links)
  virtual NTYPE acc_cost();

  // length of the path leading to the state (default implementation counts
  // steps along predecessor links)
  virtual index_type depth() const;

  // the estimated cost of the state
  virtual NTYPE est_cost() = 0;

  // true IFF the state is final
  virtual bool  is_final() = 0;

  // true IFF the state is a max state
  virtual bool  is_max() = 0;

  // (i) if the state is a min state, expand as follows:
  // 1(a). for each successor state n s.t. n.delta_cost + n.est_cost <= bound,
  //  let c_n = n.delta_cost + s.new_state(n, bound - n.delta_cost).
  // 1(b). for each successor state n s.t. n.delta_cost + n.est_cost > bound,
  //  let c_n = n.delta_cost + n.est_cost
  // 2. return min c_n over all n generated in steps 1(a) and 1(b).
  // (ii) if the state is a max state, expand as follows:
  // 1. for each successor state n, let c_n = s.new_state(n, bound).
  // 2. return max c_n over all such n.
  // note: after each call to s.new_state, expand should call s.done() and
  // if this returns true, expand should return without generating any more
  // successor states (with value as above computed over the successors
  // generated so far)
  virtual NTYPE expand(Search& s, NTYPE bound) = 0;

  // store the state in the cost table
  virtual void store(NTYPE cost, bool opt) = 0;

  // force the state to re-evaluate its estimated cost (may be needed
  // if the contents of the cost table has changed)
  virtual void reevaluate() = 0;

  // compare the state to s: return < 0 if the state is smaller than s
  // (according to some order on states), > 0 if the state is greater than
  // s, and 0 iff they are equal.
  virtual int compare(const State& s) = 0;

  // hash value of the state
  virtual index_type hash() = 0;

  // create and return a copy of the state
  virtual State* copy() = 0;

  // insert the plan fragment associated with state into plan p, at current
  // plan time
  virtual void insert(Plan& p) = 0;

  // insert the corresponding path into plan p, in correct order (this
  // should only be applied to final states).
  virtual void insert_path(Plan& p) = 0;

  // write the state
  virtual void write(::std::ostream& s) = 0;

  // write the plan fragment associated with the state
  virtual void write_plan(::std::ostream& s) = 0;

  // write a derivation of the estimated cost
  virtual void write_eval(::std::ostream& s, char* p = 0, bool e = true);

  // operations on the path, up to and including this state
  State* copy_path();
  void delete_path();
  int compare_path(const State* s);
  void write_path(::std::ostream& s);

  // overridable default implement.
  virtual void write_path_as_graph(::std::ostream& s);
};

inline bool operator==(State& s0, State& s1) {
  return (s0.compare(s1) == 0);
}

inline bool operator<(State& s0, State& s1) {
  return (s0.compare(s1) < 0);
}

inline bool operator<=(State& s0, State& s1) {
  return (s0.compare(s1) <= 0);
}

inline bool operator>(State& s0, State& s1) {
  return (s0.compare(s1) > 0);
}

inline bool operator>=(State& s0, State& s1) {
  return (s0.compare(s1) >= 0);
}

inline ::std::ostream& operator<<(::std::ostream& s, State& state) {
  state.write(s);
  return s;
}

class ProgressionState : public State {
 public:
  ProgressionState() { };
  ProgressionState(const ProgressionState& s) : State(s) { };
  virtual ~ProgressionState() { };

  virtual void insert_path(Plan& p);
};

class RegressionState : public State {
 public:
  RegressionState() { };
  RegressionState(const RegressionState& s) : State(s) { };
  virtual ~RegressionState() { };

  virtual void insert_path(Plan& p);
};

typedef lvector<State*> state_vec;

class PlanTrait {
 public:
  PlanTrait() { };
  virtual ~PlanTrait();
  virtual const PlanTrait* cast_to(const char* class_name) const;
};

typedef lvector<PlanTrait*> plan_trait_vec;

class Plan {
 public:
  virtual ~Plan();

  // add a protection interval for the atom, starting at current time
  virtual void protect(index_type atom) = 0;

  // add action, starting at current time, to the plan
  virtual void insert(index_type act) = 0;

  // advance current time by delta
  virtual void advance(NTYPE delta) = 0;

  // end the plan
  virtual void end() = 0;

  // copy to another plan structure (default implementation: do nothing)
  virtual void output(Plan& to);

  // add annotations to the plan (default implementation: ignore)
  virtual void set_name(const Name* n);
  virtual void set_optimal(bool o);
  // note: plan assumes ownership of trait object
  virtual void add_trait(PlanTrait* t);
};

class Search {
 public:
  virtual ~Search();

  // add a new state to the search tree and return the updated estimated
  // cost of the state, expanding only within the bound.
  virtual NTYPE new_state(State& s, NTYPE bound) = 0;

  // true iff the search has encountered a solution.
  virtual bool  solved() const = 0;

  // true iff the solution has been proven optimal.
  virtual bool  optimal() const = 0;

  // true iff the search is finished.
  virtual bool  done() const = 0;
};

class NoSearch : public Search {
  bool _solved;
 public:
  NoSearch() : _solved(false) { };
  virtual ~NoSearch();

  void reset(); // reset solved to false

  virtual NTYPE new_state(State& s, NTYPE bound);
  virtual bool  solved() const;
  virtual bool  optimal() const;
  virtual bool  done() const;
};

class Transitions : public state_vec, public Search {
  State* target_state;
  NTYPE  delta_bound;
  bool   bound_is_exact;
 public:
  Transitions();
  Transitions(State* from, State* to, NTYPE db);
  virtual ~Transitions();

  void clear();
  bool find(State* from, State* to, NTYPE d, bool x);

  virtual NTYPE new_state(State& s, NTYPE bound);
  virtual bool  solved() const;
  virtual bool  optimal() const;
  virtual bool  done() const;
};

class StateFactory {
 public:
  virtual ~StateFactory();

  // create a new state
  virtual State* new_state(const index_set& s, State* pre) = 0;
  virtual State* new_state(const bool_vec& s, State* pre) = 0;
};

class PlanSet {
 public:
  virtual ~PlanSet();

  // add a new (empty) plan to the set
  virtual Plan* new_plan() = 0;

  // output plans to other set (default impl: do nothing)
  virtual void  output(PlanSet& to);
  virtual void  output(PlanSet& to, const bool_vec& s);
};

END_HSPS_NAMESPACE

#endif
