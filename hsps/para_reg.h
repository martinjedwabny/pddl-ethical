#ifndef PARA_REGRESSION_STATE_H
#define PARA_REGRESSION_STATE_H

#include "config.h"
#include "atomset.h"

BEGIN_HSPS_NAMESPACE

class ParaRegState : public AtomSetState, public StateFactory {
 protected:
  index_vec acts;
  index_vec noops;

  void sort_to(index_vec& s);
  virtual bool applicable(Instance::Atom& atom, Instance::Action& trie,
			  bool_vec& p_set, bool_vec& x_set);
  virtual ParaRegState* new_succ();

#ifdef SEARCH_EXTRA_STATS
  count_type n_succ_accepted;
  count_type n_succ_discarded;
#endif

 public:
  ParaRegState(Instance& i, Heuristic& h)
    : AtomSetState(i, h),
      acts(no_such_index, 0),
      noops(no_such_index, 0) { };
  ParaRegState(Instance& i, Heuristic& h, const index_set& g)
    : AtomSetState(i, h, g),
      acts(no_such_index, 0),
      noops(no_such_index, 0) { };
  ParaRegState(Instance& i, Heuristic& h, const bool_vec& g)
    : AtomSetState(i, h, g),
      acts(no_such_index, 0),
      noops(no_such_index, 0) { };
  ParaRegState(const ParaRegState& s)
    : AtomSetState(s), acts(s.acts), noops(s.noops) { };

  virtual NTYPE delta_cost();

  virtual NTYPE expand(Search& s, NTYPE bound);
  virtual NTYPE regress(index_vec& g_set, index_type p,
			bool_vec& r_set, bool_vec& p_set, bool_vec& x_set,
			NTYPE new_est, Search& s, NTYPE bound);

  virtual void insert(Plan& p);
  virtual void insert_path(Plan& p);
  virtual void write_plan(std::ostream& s);
  virtual State* new_state(const index_set& s, State* p);
  virtual State* new_state(const bool_vec& s, State* p);
  virtual State* copy();
};

class ApxParaRegState : public ParaRegState {
 protected:
  index_type limit;

  virtual ParaRegState* new_succ();

 public:
  ApxParaRegState(Instance& i, Heuristic& h, index_type l)
    : ParaRegState(i, h), limit(l) { };
  ApxParaRegState(Instance& i, Heuristic& h, const index_set& g, index_type l)
    : ParaRegState(i, h, g), limit(l) { };
  ApxParaRegState(Instance& i, Heuristic& h, const bool_vec& g, index_type l)
    : ParaRegState(i, h, g), limit(l) { };
  ApxParaRegState(const ApxParaRegState& s)
    : ParaRegState(s), limit(s.limit) { };

  virtual bool is_max();
  virtual NTYPE expand(Search& s, NTYPE bound);

  virtual State* new_state(const index_set& s, State* p);
  virtual State* new_state(const bool_vec& s, State* p);
  virtual State* copy();
};

class RndApxParaRegState : public ApxParaRegState {
 protected:
  index_type n_subsets;
  RNG& rnd_src;

  virtual ParaRegState* new_succ();

 public:
  RndApxParaRegState(Instance& i, Heuristic& h, index_type l, RNG& rs)
    : ApxParaRegState(i, h, l), n_subsets(1), rnd_src(rs) { };
  RndApxParaRegState(Instance& i, Heuristic& h, const index_set& g,
		     index_type l, RNG& rs)
    : ApxParaRegState(i, h, g, l), n_subsets(1), rnd_src(rs) { };
  RndApxParaRegState(Instance& i, Heuristic& h, const bool_vec& g,
		     index_type l, RNG& rs)
    : ApxParaRegState(i, h, g, l), n_subsets(1), rnd_src(rs) { };

  RndApxParaRegState(Instance& i, Heuristic& h,
		     index_type l, index_type n, RNG& rs)
    : ApxParaRegState(i, h, l), n_subsets(n), rnd_src(rs) { };
  RndApxParaRegState(Instance& i, Heuristic& h, const index_set& g,
		     index_type l, index_type n, RNG& rs)
    : ApxParaRegState(i, h, g, l), n_subsets(n), rnd_src(rs) { };
  RndApxParaRegState(Instance& i, Heuristic& h, const bool_vec& g,
		     index_type l, index_type n, RNG& rs)
    : ApxParaRegState(i, h, g, l), n_subsets(n), rnd_src(rs) { };

  RndApxParaRegState(const RndApxParaRegState& s)
    : ApxParaRegState(s), n_subsets(s.n_subsets), rnd_src(s.rnd_src) { };

  void set_n_subsets(index_type n) { n_subsets = n; };

  virtual NTYPE expand(Search& s, NTYPE bound);

  virtual State* new_state(const index_set& s, State* p);
  virtual State* new_state(const bool_vec& s, State* p);
  virtual State* copy();
};

class ParaRSRegState : public ParaRegState {
 protected:
  virtual bool applicable(Instance::Atom& atom, Instance::Action& trie,
			  bool_vec& p_set, bool_vec& x_set);
  virtual ParaRegState* new_succ();

 public:
  ParaRSRegState(Instance& i, Heuristic& h)
    : ParaRegState(i, h) { };
  ParaRSRegState(Instance& i, Heuristic& h, const index_set& g)
    : ParaRegState(i, h, g) { };
  ParaRSRegState(Instance& i, Heuristic& h, const bool_vec& g)
    : ParaRegState(i, h, g) { };
  ParaRSRegState(const ParaRSRegState& s)
    : ParaRegState(s) { };

  virtual State* new_state(const index_set& s, State* p);
  virtual State* new_state(const bool_vec& s, State* p);
  virtual State* copy();

  virtual int compare(const State& s);
  virtual void write(std::ostream& s);
};

END_HSPS_NAMESPACE

#endif
