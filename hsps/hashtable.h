#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "config.h"
#include "search.h"
#include "stats.h"

BEGIN_HSPS_NAMESPACE

class HashTable {
 public:
  struct Entry {
    State* state;
    index_type depth;
    NTYPE  cost;
  };

 private:
  index_type size;
  index_type nocc;
  Entry*     tab;

  count_type hits;
  count_type cols;
  count_type miss;

 public:
  HashTable(index_type s);
  ~HashTable();

  void inc_occ_count() { nocc += 1; };

  Entry& operator[](index_type i);
  index_type index(State& s);
  Entry* find(State& s);
  void insert(State& s, NTYPE v, index_type d);
  void insert(State& s, NTYPE v);
  void clear();

  double hit_ratio();
  double HCF();
  double TUF();
};

END_HSPS_NAMESPACE

#endif
