#ifndef MINIMAX_H
#define MINIMAX_H

#include "config.h"
#include "search.h"
#include "heuristic.h"
#include "hashtable.h"
#include "stats.h"

BEGIN_HSPS_NAMESPACE

class MiniMax : public Search {
 protected:
  Statistics& stats;
  index_type depth;

 public:
  MiniMax(Statistics& s, index_type d);
  virtual ~MiniMax();

  NTYPE start(State& s);
  NTYPE start(State& s, index_type d);

  virtual NTYPE new_state(State& s, NTYPE bound);
  virtual bool solved() const;
  virtual bool optimal() const;
  virtual bool done() const;
};

class AlphaBeta : public MiniMax {
  bool  pre_is_max;
  NTYPE alpha;
  NTYPE beta;
  bool  cut;

  HashTable* tt;

 public:
  AlphaBeta(Statistics& s, index_type d);
  AlphaBeta(Statistics& s, index_type d, HashTable* ttab);
  virtual ~AlphaBeta();

  NTYPE start(State& s);
  NTYPE start(State& s, index_type d);

  virtual NTYPE new_state(State& s, NTYPE bound);
  virtual bool done() const;
};

class LookAhead : public Heuristic, public MiniMax {
  StateFactory& space;
 public:
  LookAhead(Instance& ins, StateFactory& ss, index_type d, Statistics& s)
    : Heuristic(ins), MiniMax(s, d), space(ss) { };
  virtual ~LookAhead() { };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
};

class LookAheadAB : public Heuristic, public AlphaBeta {
  StateFactory& space;
 public:
  LookAheadAB(Instance& ins, StateFactory& ss, index_type d, Statistics& s)
    : Heuristic(ins), AlphaBeta(s, d), space(ss) { };
  LookAheadAB(Instance& ins, StateFactory& ss, index_type d, HashTable* tt,
	      Statistics& s)
    : Heuristic(ins), AlphaBeta(s, d, tt), space(ss) { };
  virtual ~LookAheadAB() { };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
};

END_HSPS_NAMESPACE

#endif
