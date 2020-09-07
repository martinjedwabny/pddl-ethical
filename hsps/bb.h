#ifndef BRANCH_AND_BOUND_H
#define BRANCH_AND_BOUND_H

#include "config.h"
#include "search_base.h"
#include "hashtable.h"

BEGIN_HSPS_NAMESPACE

class DFS : public SearchAlgorithm {
 protected:
  bool       cycle_check;
  bool       store_cost;
  NTYPE      upper_bound;

  HashTable* ttab;

  static const count_type TRACE_LEVEL_2_NOTIFY = 100000;

  virtual bool within_upper_bound(NTYPE c_acc, NTYPE c_est);
  virtual void update_upper_bound(NTYPE c);

 public:
  DFS(Statistics& s, SearchResult& r);
  DFS(Statistics& s, SearchResult& r, HashTable* tt);
  virtual ~DFS();

  void set_cycle_check(bool cc) { cycle_check = cc; };
  void set_store_cost(bool on) { store_cost = on; };
  void set_upper_bound(NTYPE ub) { upper_bound = ub; };

  virtual NTYPE start(State& s, NTYPE b);
  virtual NTYPE start(State& s);
  virtual NTYPE new_state(State& s, NTYPE bound);

  virtual NTYPE cost() const;
};

class DFS_BB : public DFS {
 protected:

  virtual bool within_upper_bound(NTYPE c_acc, NTYPE c_est);
  virtual void update_upper_bound(NTYPE c);

 public:
  DFS_BB(Statistics& s, SearchResult& r);
  DFS_BB(Statistics& s, SearchResult& r, HashTable* tt);
  virtual ~DFS_BB();
};

END_HSPS_NAMESPACE

#endif
