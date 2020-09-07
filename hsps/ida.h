#ifndef IDA_TT_H
#define IDA_TT_H

#include "config.h"
#include "search_base.h"
#include "hashtable.h"

BEGIN_HSPS_NAMESPACE

class IDA : public MultiSearchAlgorithm {
  HashTable* ttab;
  bool       cycle_check;

  NTYPE      current_bound;
  count_type iteration_limit;
  count_type iteration_count;

  static const count_type TRACE_LEVEL_2_NOTIFY = 100000;

 public:
  IDA(Statistics& s, SearchResult& r);
  IDA(Statistics& s, SearchResult& r, HashTable* t);
  virtual ~IDA();

  void set_iteration_limit(count_type i_max)  { iteration_limit = i_max; };
  void increase_iteration_limit(count_type i) { iteration_limit += i; };
  count_type get_iteration_limit()            { return iteration_limit; };
  void set_cycle_check(bool cc)               { cycle_check = cc; };

  NTYPE main(State& s, NTYPE bound);
  virtual NTYPE new_state(State& s, NTYPE bound);

  // from SearchAlgorithm
  virtual NTYPE start(State& s, NTYPE b);
  virtual NTYPE start(State& s);
  virtual NTYPE cost() const;

  // from MultiSearchAlgorithm
  virtual NTYPE resume(State& s, NTYPE b);
};

END_HSPS_NAMESPACE

#endif
