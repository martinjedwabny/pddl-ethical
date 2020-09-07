#ifndef SEQ_REGRESSION_STATE_H
#define SEQ_REGRESSION_STATE_H

#include "config.h"
#include "atomset.h"
#include "resource.h"

BEGIN_HSPS_NAMESPACE

class SeqRegState : public AtomSetState, public StateFactory {
 protected:
  const ACF& cost;

  // action leading from predecessor
  index_type     act;

  // consumable resource state (NIL if not planning with resources)
  RegressionResourceState* res;

  bool relevant(Instance::Action& a);
  bool applicable(Instance::Action& a);

  // return (a copy of) the state that results from applying action a to
  // the present state (present state becomes predecessor of the new state).
  SeqRegState* apply(Instance::Action& a);

 public:
  SeqRegState(Instance& i, Heuristic& h, const ACF& c)
    : AtomSetState(i, h), cost(c), act(no_such_index), res(0) { };
  SeqRegState(Instance& i, Heuristic& h, const ACF& c, const index_set& g)
    : AtomSetState(i, h, g), cost(c), act(no_such_index), res(0) { };
  SeqRegState(Instance& i, Heuristic& h, const ACF& c, const bool_vec& g)
    : AtomSetState(i, h, g), cost(c), act(no_such_index), res(0) { };

  SeqRegState(Instance& i, Heuristic& h, const ACF& c,
	      RegressionResourceState* r)
    : AtomSetState(i, h), cost(c), act(no_such_index), res(r) { };
  SeqRegState(Instance& i, Heuristic& h, const ACF& c, const index_set& g,
	      RegressionResourceState* r)
    : AtomSetState(i, h, g), cost(c), act(no_such_index), res(r) { };
  SeqRegState(Instance& i, Heuristic& h, const ACF& c, const bool_vec& g,
	      RegressionResourceState* r)
    : AtomSetState(i, h, g), cost(c), act(no_such_index), res(r) { };

  SeqRegState(const SeqRegState& s);
  virtual ~SeqRegState();

  virtual NTYPE delta_cost();
  virtual NTYPE expand(Search& s, NTYPE bound);
  virtual void store(NTYPE cost, bool opt);
  virtual void set_predecessor(State* p);

  virtual void insert(Plan& p);
  virtual void insert_path(Plan& p);
  virtual void write_plan(std::ostream& s);
  virtual State* new_state(const index_set& s, State* p);
  virtual State* new_state(const bool_vec& s, State* p);
  virtual State* copy();

  virtual void write(std::ostream& s);
};

class ApxSeqRegState : public SeqRegState {
 protected:
  index_type limit;
 public:
  ApxSeqRegState(Instance& i, Heuristic& h, const ACF& c, index_type l)
    : SeqRegState(i, h, c), limit(l) { };
  ApxSeqRegState(Instance& i, Heuristic& h, const ACF& c, const index_set& g,
		 index_type l)
    : SeqRegState(i, h, c, g), limit(l) { };
  ApxSeqRegState(Instance& i, Heuristic& h, const ACF& c, const bool_vec& g,
		 index_type l)
    : SeqRegState(i, h, c, g), limit(l) { };

  ApxSeqRegState(Instance& i, Heuristic& h, const ACF& c,
		 RegressionResourceState* r, index_type l)
    : SeqRegState(i, h, c, r), limit(l) { };
  ApxSeqRegState(Instance& i, Heuristic& h, const ACF& c, const index_set& g,
		 RegressionResourceState* r, index_type l)
    : SeqRegState(i, h, c, g, r), limit(l) { };
  ApxSeqRegState(Instance& i, Heuristic& h, const ACF& c, const bool_vec& g,
		 RegressionResourceState* r, index_type l)
    : SeqRegState(i, h, c, g, r), limit(l) { };

  ApxSeqRegState(const ApxSeqRegState& s)
    : SeqRegState(s), limit(s.limit) { };

  virtual bool is_max();
  virtual NTYPE expand(Search& s, NTYPE bound);
  virtual State* new_state(const index_set& s, State* p);
  virtual State* new_state(const bool_vec& s, State* p);
  virtual State* copy();
};

class SeqCRegState : public SeqRegState {
 public:
  SeqCRegState(Instance& i, Heuristic& h, const ACF& c)
    : SeqRegState(i, h, c) { };
  SeqCRegState(Instance& i, Heuristic& h, const ACF& c, const index_set& g)
    : SeqRegState(i, h, c, g) { };
  SeqCRegState(Instance& i, Heuristic& h, const ACF& c, const bool_vec& g)
    : SeqRegState(i, h, c, g) { };

  SeqCRegState(Instance& i, Heuristic& h, const ACF& c,
	       RegressionResourceState* r)
    : SeqRegState(i, h, c, r) { };
  SeqCRegState(Instance& i, Heuristic& h, const ACF& c, const index_set& g,
	       RegressionResourceState* r)
    : SeqRegState(i, h, c, g, r) { };
  SeqCRegState(Instance& i, Heuristic& h, const ACF& c, const bool_vec& g,
	       RegressionResourceState* r)
    : SeqRegState(i, h, c, g, r) { };

  SeqCRegState(const SeqRegState& s)
    : SeqRegState(s) { };

  virtual NTYPE expand(Search& s, NTYPE bound);

  virtual State* new_state(const index_set& s, State* p);
  virtual State* new_state(const bool_vec& s, State* p);
  virtual State* copy();

  virtual int compare(const State& s);
  index_type hash();
  virtual void write(std::ostream& s);
};

class LDSeqRegState : public SeqRegState {
  const index_vec& plan;
  static bool copy_as_ld;
 public:
  LDSeqRegState(Instance& i, Heuristic& h, const ACF& c,
		const index_set& g, const index_vec& p)
    : SeqRegState(i, h, c, g), plan(p) { };
  LDSeqRegState(Instance& i, Heuristic& h, const ACF& c,
		const index_vec& p, const bool_vec& g)
    : SeqRegState(i, h, c, g), plan(p) { };
  LDSeqRegState(const LDSeqRegState& s);

  virtual NTYPE expand(Search& s, NTYPE bound);
  virtual State* copy();

  virtual int compare(const State& s);
  virtual void write(std::ostream& s);
};

END_HSPS_NAMESPACE

#endif
