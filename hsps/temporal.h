#ifndef TEMPORAL_REGRESSION_STATE_H
#define TEMPORAL_REGRESSION_STATE_H

#include "config.h"
#include "search.h"
#include "problem.h"
#include "heuristic.h"
#include "atomset.h"
#include "resource.h"

BEGIN_HSPS_NAMESPACE

class TemporalRegState : public AtomSet, public State, public StateFactory {
 protected:
  Heuristic& heuristic;

  // fixed actions
  index_cost_vec acts;

  // consumable resource state
  RegressionResourceState* res;

  // delta & estimated cost
  NTYPE      delta;
  NTYPE      est;
  bool       final;

  // set of actions starting in this state
  index_set  starts;
  // set of noops starting in this state
  index_set  noops;

#ifdef SEARCH_EXTRA_STATS
  count_type n_succ_accepted;
  count_type n_succ_discarded;
#endif

  void sort_to(index_vec& s);

  NTYPE eval(index_cost_vec& se_acts,
	     index_vec& se_noops,
	     RegressionResourceState* r);
  NTYPE eval_se_noop(index_cost_vec& se_acts,
		     index_vec& se_noops,
		     RegressionResourceState* r,
		     Instance::Atom& new_noop,
		     NTYPE prev_est);
  NTYPE eval_se_act(index_cost_vec& se_acts,
		    index_vec& se_noops,
		    RegressionResourceState* r,
		    Instance::Action& new_act,
		    NTYPE prev_est);
  index_type insert_action(Instance::Action& trie, index_cost_vec& se_acts);

  virtual bool applicable(Instance::Atom& atom,
			  Instance::Action& trie,
			  index_cost_vec& se_acts,
			  index_vec& se_noops,
			  RegressionResourceState* r);
  virtual TemporalRegState* new_succ(const index_cost_vec& se_acts,
				     const index_vec& se_noops,
				     RegressionResourceState* r);
  NTYPE regress(index_vec& g_set, index_type p, bool_vec& r_set,
		index_cost_vec& se_acts, index_vec& se_noops,
		RegressionResourceState* r,
		NTYPE c_est, Search& s, NTYPE bound);

  void apply_path_max();
  void write_action_vec(const index_cost_vec& av);

  TemporalRegState(Instance& i, Heuristic& h,
		   const index_cost_vec& se_acts, const index_vec& se_noops,
		   RegressionResourceState* r);

 public:
  static bool implicit_noop;
  static bool subgoal_ordering;
  static bool strict_right_shift;

  TemporalRegState(Instance& i, Heuristic& h);
  TemporalRegState(Instance& i, Heuristic& h,
		   RegressionResourceState* r);
  TemporalRegState(Instance& i, Heuristic& h, const index_set& g);
  TemporalRegState(Instance& i, Heuristic& h, const bool_vec& g);
  TemporalRegState(Instance& i, Heuristic& h, const bool_vec& g,
		   RegressionResourceState* r);
  TemporalRegState(Instance& i, Heuristic& h, const index_set& g,
		   RegressionResourceState* r);
  TemporalRegState(const TemporalRegState& s);
  TemporalRegState(Instance& i, Heuristic& h,
		   const index_set& g, const index_set& a,
		   RegressionResourceState* r);
  virtual ~TemporalRegState();

  virtual NTYPE delta_cost();
  virtual NTYPE est_cost();
  virtual bool  is_final();
  index_type    state_size();
  virtual bool  is_max();
          bool  is_root();

  virtual NTYPE expand(Search& s, NTYPE bound);
  virtual void store(NTYPE cost, bool opt);
  virtual void reevaluate();

  virtual int compare(const State& s);
  virtual index_type hash();

  virtual State* new_state(const index_set& s, State* p);
  virtual State* new_state(const bool_vec& s, State* p);
  virtual State* copy();

  virtual void insert(Plan& p);
  virtual void insert_path(Plan& p);
  virtual void write_plan(std::ostream& s);
  virtual void write(std::ostream& s);
  virtual void write_path_as_graph(std::ostream& s);
};

class TemporalRSRegState : public TemporalRegState {
 protected:
  virtual bool applicable(Instance::Atom& atom, Instance::Action& trie,
			  index_cost_vec& se_acts, index_vec& se_noops,
			  RegressionResourceState* r);
  virtual TemporalRegState* new_succ(const index_cost_vec& se_acts,
				     const index_vec& se_noops,
				     RegressionResourceState* r);

  TemporalRSRegState(Instance& i, Heuristic& h,
		     const index_cost_vec& se_acts, const index_vec& se_noops,
		     RegressionResourceState* r)
    : TemporalRegState(i, h, se_acts, se_noops, r) { };

 public:
  TemporalRSRegState(Instance& i, Heuristic& h)
    : TemporalRegState(i, h) { };
  TemporalRSRegState(Instance& i, Heuristic& h,
		     RegressionResourceState* r)
    : TemporalRegState(i, h, r) { };
  TemporalRSRegState(Instance& i, Heuristic& h, const index_set& g)
    : TemporalRegState(i, h, g) { };
  TemporalRSRegState(Instance& i, Heuristic& h, const bool_vec& g)
    : TemporalRegState(i, h, g) { };
  TemporalRSRegState(Instance& i, Heuristic& h, const index_set& g,
		     RegressionResourceState* r)
    : TemporalRegState(i, h, g, r) { };
  TemporalRSRegState(Instance& i, Heuristic& h, const bool_vec& g,
		     RegressionResourceState* r)
    : TemporalRegState(i, h, g, r) { };
  TemporalRSRegState(const TemporalRegState& s)
    : TemporalRegState(s) { };
  TemporalRSRegState(Instance& i, Heuristic& h,
		     const index_set& g, const index_set& a,
		     RegressionResourceState* r)
    : TemporalRegState(i, h, g, a, r) { };

  virtual int compare(const State& s);
  virtual index_type hash();
  virtual State* new_state(const index_set& s, State* p);
  virtual State* new_state(const bool_vec& s, State* p);
  virtual State* copy();
};


class ApxTemporalRegState : public TemporalRegState {
  index_type limit;
 protected:
  virtual TemporalRegState* new_succ(const index_cost_vec& se_acts,
				     const index_vec& se_noops,
				     RegressionResourceState* r);
 public:
  ApxTemporalRegState(Instance& i, Heuristic& h, index_type l)
    : TemporalRegState(i, h), limit(l) { };
  ApxTemporalRegState(Instance& i, Heuristic& h,
		      RegressionResourceState* r, index_type l)
    : TemporalRegState(i, h, r), limit(l) { };
  ApxTemporalRegState(Instance& i, Heuristic& h, const index_set& g,
		      index_type l)
    : TemporalRegState(i, h, g), limit(l) { };
  ApxTemporalRegState(Instance& i, Heuristic& h, const bool_vec& g,
		      index_type l)
    : TemporalRegState(i, h, g), limit(l) { };
  ApxTemporalRegState(Instance& i, Heuristic& h, const index_set& g,
		      RegressionResourceState* r, index_type l)
    : TemporalRegState(i, h, g, r), limit(l) { };
  ApxTemporalRegState(Instance& i, Heuristic& h, const bool_vec& g,
		      RegressionResourceState* r, index_type l)
    : TemporalRegState(i, h, g, r), limit(l) { };
  ApxTemporalRegState(const ApxTemporalRegState& s)
    : TemporalRegState(s), limit(s.limit) { };
  ApxTemporalRegState(Instance& i, Heuristic& h,
		      const index_cost_vec& se_acts, const index_vec& se_noops,
		      RegressionResourceState* r, index_type l)
    : TemporalRegState(i, h, se_acts, se_noops, r), limit(l) { };

  virtual bool  is_max();
  virtual NTYPE expand(Search& s, NTYPE bound);
  virtual State* new_state(const index_set& s, State* p);
  virtual State* new_state(const bool_vec& s, State* p);
  virtual State* copy();
};


class goal_cost_decreasing : public index_vec::order {
  Heuristic& heuristic;
public:
  goal_cost_decreasing(Heuristic& h) : heuristic(h) { };
  virtual bool operator()(const index_type& p0, const index_type& p1) const;
};

END_HSPS_NAMESPACE

#endif
