#ifndef IDAO_H
#define IDAO_H

#include "config.h"
#include "search_base.h"
#include "hashtable.h"

BEGIN_HSPS_NAMESPACE

class IDAO : public MultiSearchAlgorithm {
 protected:
  HashTable* solved_tab;
  NTYPE      current_bound;
  count_type iteration_count;
  count_type iteration_limit;
  bool       cycle_check;
  bool       store_cost;

  static const count_type TRACE_LEVEL_2_NOTIFY = 100000;

 public:
  static bool alpha_cut;

  IDAO(Statistics& s, SearchResult& res);
  IDAO(Statistics& s, SearchResult& res, HashTable* tab);

  void set_iteration_limit(count_type i_max)  { iteration_limit = i_max; };
  void increase_iteration_limit(count_type i) { iteration_limit += i; };
  count_type get_iteration_limit()            { return iteration_limit; };
  count_type iterations()                     { return iteration_count; };
  void set_cycle_check(bool cc)               { cycle_check = cc; };
  void set_store_cost(bool on)                { store_cost = on; };

  NTYPE main(State& s, NTYPE b);
  virtual NTYPE new_state(State& s, NTYPE bound);
  virtual NTYPE new_max_state(State& s, NTYPE bound);

  // from SearchAlgorithm
  virtual NTYPE start(State& s, NTYPE b);
  virtual NTYPE start(State& s);
  virtual NTYPE cost() const;

  // from MultiSearchAlgorithm
  virtual NTYPE resume(State& s, NTYPE b);
};

class maxIDAO : public Search {
 protected:
  Statistics& stats;
  SearchResult& result;
  bool is_solved;
  HashTable* solved_tab;
  bool store_cost;
  index_type depth;
  int  trace_level;

 public:
  maxIDAO(Statistics& s, SearchResult& res, HashTable* tab, index_type d);
  maxIDAO(Statistics& s, SearchResult& res, HashTable* tab, index_type d,
	  int level);
  void set_store_cost(bool on) { store_cost = on; };

  virtual NTYPE new_state(State& s, NTYPE bound);

  virtual bool solved() const;
  virtual bool optimal() const;
  virtual bool done() const;
};

class estimated_cost : public state_vec::order {
 public:
  estimated_cost() { };
  virtual ~estimated_cost() { };
  virtual bool operator()(const State*& v0, const State*& v1) const;
};

class kIDAO : public IDAO {
  count_type max_k;
 public:
  kIDAO(Statistics& s, SearchResult& res, HashTable* tab)
    : IDAO(s, res, tab), max_k(count_type_max) { };
  kIDAO(count_type k, Statistics& s, SearchResult& res, HashTable* tab)
    : IDAO(s, res, tab), max_k(k) { };
  virtual NTYPE new_max_state(State& s, NTYPE bound);
};

class maxkIDAO : public maxIDAO {
 protected:
  state_vec succ;
  count_type max_k;

 public:
  maxkIDAO(count_type k, Statistics& s, SearchResult& res, HashTable* tab,
	   index_type d)
    : maxIDAO(s, res, tab, d), succ(0, 0), max_k(k) { };
  maxkIDAO(count_type k, Statistics& s, SearchResult& res, HashTable* tab,
	   index_type d, int l)
    : maxIDAO(s, res, tab, d, l), succ(0, 0), max_k(k) { };
  virtual ~maxkIDAO();

  virtual NTYPE new_state(State& s, NTYPE bound);
  NTYPE expand(NTYPE bound);
};

END_HSPS_NAMESPACE

#endif
