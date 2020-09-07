#ifndef ITEST_H
#define ITEST_H

#include "config.h"
#include "search_base.h"
#include "hashtable.h"

BEGIN_HSPS_NAMESPACE

class ITest : public SearchAlgorithm {
 protected:
  HashTable& solved_tab;
  NTYPE      current_bound;
  bool       cycle_check;

  static const count_type TRACE_LEVEL_2_NOTIFY = 100000;

 public:
  ITest(Statistics& s, SearchResult& res, HashTable& tab);
  void set_cycle_check(bool cc) { cycle_check = cc; };

  virtual NTYPE start(State& s, NTYPE b);
  virtual NTYPE start(State& s);

  NTYPE main(State& s, NTYPE bound);
  virtual NTYPE new_state(State& s, NTYPE bound);

  virtual NTYPE cost() const;
  virtual bool done() const;
};

class maxTest : public Search {
  Statistics& stats;
  SearchResult& result;
  bool is_solved;
  HashTable& solved_tab;
  int   trace_level;
 public:
  maxTest(Statistics& s, SearchResult& res, HashTable& tab);
  maxTest(Statistics& s, SearchResult& res, HashTable& tab, int level);

  virtual NTYPE new_state(State& s, NTYPE bound);

  virtual bool solved() const;
  virtual bool optimal() const;
  virtual bool done() const;
};

class LDFS : public SearchAlgorithm {
  bool  in_max;
  NTYPE c_root;
 protected:
  HashTable& solved_tab;

  static const count_type TRACE_LEVEL_2_NOTIFY = 100000;

 public:
  LDFS(Statistics& s, SearchResult& res, HashTable& tab);

  virtual NTYPE start(State& s, NTYPE b);
  virtual NTYPE start(State& s);

  NTYPE main(State& s);
  virtual NTYPE new_state(State& s, NTYPE bound);

  virtual NTYPE cost() const;
  virtual bool done() const;
};

END_HSPS_NAMESPACE

#endif
