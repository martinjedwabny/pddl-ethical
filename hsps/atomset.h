#ifndef ATOM_SET_H
#define ATOM_SET_H

#include "config.h"
#include "search.h"
#include "problem.h"
#include "heuristic.h"
#include "stats.h"
#include "rng.h"

BEGIN_HSPS_NAMESPACE

// AtomSet is a base class for (almost) all states that consist of (or
// contain) a set of atoms. Mainly implements the split/split_random
// methods.

class AtomSet {
 protected:
  Instance&  instance;
  bool_vec   set;
  index_type size;

  index_type count();

 private:
  // internal split method (recursive)
  NTYPE split(index_set g, index_type p, index_type n_out, index_type limit,
	      StateFactory& f, State* sp, Search& s, NTYPE bound);
  NTYPE split_random(index_set g, index_type p, index_type n_out,
		     index_type limit, StateFactory& f, State* sp,
		     Search& s, NTYPE bound, RNG& r);

#ifdef SEARCH_EXTRA_STATS
  count_type n_succ_created;
#endif

 public:
  static index_type max_set_size_encountered;

  AtomSet(Instance& i);
  AtomSet(Instance& i, const index_set& g);
  AtomSet(Instance& i, const bool_vec& g);
  AtomSet(const AtomSet& s);
  ~AtomSet();

  index_type atom_set_size() { return size; };

  void add(const index_set& s);
  void add(const bool_vec& s);
  void del(const index_set& s);
  void del(const bool_vec& s);

  bool is_init_all();
  bool is_init_some();
  bool is_goal_all();
  bool is_goal_some();

  index_vec& copy_to(index_vec& s);
  bool_vec&  copy_to(bool_vec& s);
  NTYPE split(index_type limit, StateFactory& f, State* sp,
	      Search& s, NTYPE bound);
  NTYPE split_random(index_type limit, StateFactory& f, State* sp,
		     Search& s, NTYPE bound, RNG& r);
  int compare(const AtomSet& s);
  index_type hash();
};

// AtomSetState is a base class for all STRIPS regression states.

class AtomSetState : public AtomSet, public State {
 protected:
  Heuristic& heuristic;

  // estimated cost
  NTYPE      est;

  // recompute est cost
  NTYPE      eval();
  NTYPE      eval(NTYPE b_est);
  NTYPE      eval_with_trace_level(int level);
  void       apply_path_max();

 public:
  AtomSetState(Instance& i, Heuristic& h);
  AtomSetState(Instance& i, Heuristic& h, const index_set& g);
  AtomSetState(Instance& i, Heuristic& h, const bool_vec& g);
  AtomSetState(const AtomSetState& s);
  virtual ~AtomSetState();

  index_type atom_set_size() { return size; };

  virtual NTYPE est_cost();
  virtual bool is_final();
  virtual bool is_max();
  virtual void store(NTYPE cost, bool opt);
  virtual void reevaluate();

  virtual int compare(const State& s);
  virtual index_type hash();
  virtual void write(std::ostream& s);
  virtual void write_eval(std::ostream& s, char* p = 0, bool e = true);
};

END_HSPS_NAMESPACE

#endif
