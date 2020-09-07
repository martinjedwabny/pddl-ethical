#ifndef ENUMERATORS_H
#define ENUMERATORS_H

#include "config.h"
#include "sas.h"

BEGIN_HSPS_NAMESPACE

class IterativeEnumerator {
 public:
  IterativeEnumerator() { };
  virtual ~IterativeEnumerator();

  virtual bool first() = 0;
  virtual bool next() = 0;
};

class RecursiveEnumerator {
 protected:
  virtual bool done() = 0;

 public:
  RecursiveEnumerator() { };
  virtual ~RecursiveEnumerator();
};

class SubsetEnumerator : public IterativeEnumerator {
 protected:
  index_type n;
  bool_vec   in;
 public:
  SubsetEnumerator(index_type _n);
  virtual ~SubsetEnumerator() { };

  virtual bool first();
  virtual bool next();
  const bool_vec& current_set() const;
  void current_set(const index_set& elements, index_set& set);
  void current_set(index_set& set);
  index_type current_set_size();
  void all_sets(index_set_vec& sets);
};

class mSubsetEnumerator : public SubsetEnumerator {
 protected:
  index_type m;
 public:
  mSubsetEnumerator(index_type _n, index_type _m);
  virtual ~mSubsetEnumerator() { };

  count_type m_of_n();

  virtual bool first();
  virtual bool next();
};

class PartialStateEnumerator : public IterativeEnumerator {
 protected:
  index_set variables;
  index_vec signature;
  bool_vec  v_free;
  partial_state state;
 public:
  PartialStateEnumerator(const index_set& v, const index_vec& s);
  PartialStateEnumerator(const index_set& v, const index_vec& s, const partial_state& c);
  virtual ~PartialStateEnumerator() { };

  virtual bool first();
  virtual bool next();
  const partial_state& current_state();
};

class kAssignmentEnumerator : public IterativeEnumerator {
 protected:
  index_type n;
  index_type k;
  index_vec  a;
 public:
  kAssignmentEnumerator(index_type _n, index_type _k);
  virtual ~kAssignmentEnumerator() { };

  virtual bool first();
  virtual bool next();
  void current_assignment(index_set_vec& sets);
};

class CorrespondanceEnumerator : public IterativeEnumerator {
 protected:
  const index_vec& a;
  const index_vec& b;
  mapping   c;
  bool_vec  f;

  index_type first_free(index_type x,
			const index_vec& vec,
			const bool_vec& f_vec);
  index_type next_free(index_type x,
		       const index_vec& vec,
		       const bool_vec& f_vec,
		       index_type starting_from);
  bool find(index_type p);

 public:
  CorrespondanceEnumerator(const index_vec& v0, const index_vec& v1);
  virtual ~CorrespondanceEnumerator() { };

  virtual bool first();
  virtual bool next();
  const mapping& current() const { return c; };
};

void write_correspondance(::std::ostream& s, const index_vec& c);

// really stupid implementation of a permutation enumerator - it
// enumerates correspondances between two all-zero vectors.
class PermutationEnumerator : public IterativeEnumerator {
 protected:
  CorrespondanceEnumerator e;
 public:
  PermutationEnumerator(index_type n);
  virtual ~PermutationEnumerator() { };

  virtual bool first();
  virtual bool next();
  const mapping& current() const { return e.current(); };
};

class RecursivekPartitionEnumerator {
  index_vec ass;

 protected:
  index_type    n;
  index_type    k;
  index_set_vec sets;
  bool          done;

  void partition(index_type n, index_type k);

  void construct();
  void construct(const index_set& set);
  virtual void solution();

 public:
  RecursivekPartitionEnumerator(index_type _n, index_type _k);
  virtual ~RecursivekPartitionEnumerator();

  void partition();
};

class RecursivePartitionEnumerator : public RecursivekPartitionEnumerator {
 public:
  RecursivePartitionEnumerator(index_type _n);
  virtual ~RecursivePartitionEnumerator() { };

  void partition();
  void partition_bounded(index_type min, index_type max);
};

class CountPartitions : public RecursivePartitionEnumerator {
  index_type c;

 protected:
  virtual void solution();

 public:
  CountPartitions(index_type _n) : RecursivePartitionEnumerator(_n) { };
  virtual ~CountPartitions() { };

  index_type count();
};

class PrintPartitions : public RecursivePartitionEnumerator {
 protected:
  virtual void solution();

 public:
  PrintPartitions(index_type _n) : RecursivePartitionEnumerator(_n) { };
  virtual ~PrintPartitions() { };
};

END_HSPS_NAMESPACE

#endif
