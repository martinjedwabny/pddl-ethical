#ifndef RELAXED_PROBLEM_H
#define RELAXED_PROBLEM_H

#include "config.h"
#include "cost_table.h"
#include "bb.h"
#include "bfs.h"

BEGIN_HSPS_NAMESPACE

class ForwardRelaxedDFS : public Heuristic {
  Instance&  relaxed;
  index_set  goals;
  const ACF& cost;
  Heuristic* rfh;
  Result     result;
  DFS_BB*    search;

 public:
  ForwardRelaxedDFS(Instance& i, Instance& r, const index_set& g,
		    const ACF& c, Statistics& s);
  virtual ~ForwardRelaxedDFS();

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
};


class ForwardRelaxedBFS : public Heuristic {
  Instance&  relaxed;
  index_set  goals;
  const ACF& cost;
  Heuristic* rfh;
  Result     result;
  BFS*       search;

 public:
  ForwardRelaxedBFS(Instance& i, Instance& r, const index_set& g,
		    const ACF& c, Statistics& s);
  virtual ~ForwardRelaxedBFS();

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
};

class SumX : public CombineNBySum {
 public:
  SumX(Instance& i, const ACF& c, Statistics& s);
};

END_HSPS_NAMESPACE

#endif
