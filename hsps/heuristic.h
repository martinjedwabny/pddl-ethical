#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "config.h"
#include "problem.h"
#include "stats.h"

BEGIN_HSPS_NAMESPACE

class Heuristic {
 protected:
  Instance& instance;
  int trace_level;

 public:
  static count_type eval_count;
  static int default_trace_level;

  Heuristic(Instance& ins)
    : instance(ins), trace_level(default_trace_level) { };
  virtual ~Heuristic();

  virtual void set_trace_level(int level);

  virtual NTYPE eval(const index_set& s) = 0;
  virtual NTYPE eval(const bool_vec& s) = 0;

  // default implementation: just write the value
  virtual void write_eval(const index_set& s,
			  ::std::ostream& st,
			  char* p = 0,
			  bool e = true);
  virtual void write_eval(const bool_vec& s,
			  ::std::ostream& st,
			  char* p = 0,
			  bool e = true);

  // default implementation: call eval(a.pre)
  virtual NTYPE eval_precondition(const Instance::Action& a);

  // default implementation: call basic eval(s)
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);
  virtual NTYPE eval_to_bound(const index_set& s, NTYPE bound);
  virtual NTYPE eval_to_bound(const bool_vec& s, NTYPE bound);

  // default implementation: do nothing
  virtual void store(const index_set& s, NTYPE v, bool opt);
  virtual void store(const bool_vec& s, NTYPE v, bool opt);

  // utility methods (implemented using the interface (virtual) methods above
  NTYPE eval(index_type atom);
};

class ZeroHeuristic : public Heuristic {
 public:
  ZeroHeuristic(Instance& ins) : Heuristic(ins) { };
  virtual ~ZeroHeuristic() { };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
};

class EvalActionCache : public Heuristic {
  Heuristic& base_h;
  cost_vec   cache;
 public:
  EvalActionCache(Instance& ins, Heuristic& h);
  virtual ~EvalActionCache() { };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
  virtual NTYPE eval_precondition(const Instance::Action& a);
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);
  virtual NTYPE eval_to_bound(const index_set& s, NTYPE bound);
  virtual NTYPE eval_to_bound(const bool_vec& s, NTYPE bound);
};

class RegressionInvariantCheck : public Heuristic {
  Heuristic& base_h;
  bool       verified_invariants_only;
 public:
  RegressionInvariantCheck(Instance& ins, Heuristic& h, bool v)
    : Heuristic(ins), base_h(h), verified_invariants_only(v) { };
  virtual ~RegressionInvariantCheck() { };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);
  virtual NTYPE eval_to_bound(const index_set& s, NTYPE bound);
  virtual NTYPE eval_to_bound(const bool_vec& s, NTYPE bound);
};

class ForwardReachabilityCheck : public Heuristic {
  index_set goals;
  bool_vec  r;
  bool_vec  f;
  bool_vec  d;

  NTYPE compute();

 public:
  ForwardReachabilityCheck(Instance& i, const index_set& g);
  virtual ~ForwardReachabilityCheck();

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
};

class Combine2ByMax : public Heuristic {
  Heuristic& h0;
  Heuristic& h1;
 public:
  Combine2ByMax(Instance& ins, Heuristic& _h0, Heuristic& _h1)
    : Heuristic(ins), h0(_h0), h1(_h1) { };
  virtual ~Combine2ByMax() { };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);
  virtual NTYPE eval_to_bound(const index_set& s, NTYPE bound);
  virtual NTYPE eval_to_bound(const bool_vec& s, NTYPE bound);
};

class CombineNByMax : public Heuristic {
  lvector<Heuristic*> h_vec;
 public:
  CombineNByMax(Instance& ins)
    : Heuristic(ins), h_vec((Heuristic*)0, 0) { };
  virtual ~CombineNByMax() { };

  void add(Heuristic* h) { h_vec.append(h); };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);
  virtual NTYPE eval_to_bound(const index_set& s, NTYPE bound);
  virtual NTYPE eval_to_bound(const bool_vec& s, NTYPE bound);
};

class CombineNBySum : public Heuristic {
  lvector<Heuristic*> h_vec;
 public:
  CombineNBySum(Instance& ins)
    : Heuristic(ins), h_vec((Heuristic*)0, 0) { };
  virtual ~CombineNBySum() { };

  void add(Heuristic* h) { h_vec.append(h); };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);
  virtual NTYPE eval_to_bound(const index_set& s, NTYPE bound);
  virtual NTYPE eval_to_bound(const bool_vec& s, NTYPE bound);
};

class Combine2ByRandomChoice : public Heuristic {
  Heuristic& h0;
  Heuristic& h1;
  NTYPE   alpha;
  RNG&      rng;
 public:
  Combine2ByRandomChoice(Instance& ins,
			 Heuristic& _h0,
			 Heuristic& _h1,
			 NTYPE _alpha,
			 RNG& _rng)
    : Heuristic(ins), h0(_h0), h1(_h1), alpha(_alpha), rng(_rng) { };
  virtual ~Combine2ByRandomChoice() { };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
};

class RoundUp : public Heuristic {
  Heuristic& h;
 public:
  RoundUp(Instance& ins, Heuristic& h0) : Heuristic(ins), h(h0) { };
  virtual ~RoundUp() { };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);
  virtual NTYPE eval_to_bound(const index_set& s, NTYPE bound);
  virtual NTYPE eval_to_bound(const bool_vec& s, NTYPE bound);
};

class HX : public Heuristic {
  Heuristic& h0;
  index_set  X;
 public:
  HX(Instance& ins, Heuristic& h, const index_set& x)
    : Heuristic(ins), h0(h), X(x) { };
  HX(Instance& ins, Heuristic& h)
    : Heuristic(ins), h0(h) { };
  virtual ~HX() { };

  void setX(const index_set& x) { X = x; };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);

  virtual void write_eval(const index_set& s,
			  ::std::ostream& st,
			  char* p = 0,
			  bool e = true);
  virtual void write_eval(const bool_vec& s,
			  ::std::ostream& st,
			  char* p = 0,
			  bool e = true);
};

class AtomMapAdapter : public Heuristic {
  index_vec  map;
  Heuristic& base_h;
 public:
  AtomMapAdapter(Instance& i, const index_vec& m, Heuristic& h)
    : Heuristic(i), map(m), base_h(h)
  { assert(map.length() == instance.n_atoms()); };
  virtual ~AtomMapAdapter() { };

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);
};

class CompleteNegationAdapter : public Heuristic {
  Heuristic& h_base;
  pair_vec   pn_map;
  bool_vec   sc;

 public:
  CompleteNegationAdapter(Instance& ins, const pair_vec& p, Heuristic& h);
  virtual ~CompleteNegationAdapter();

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
};

class CompareEval : public Heuristic {
  Heuristic& base_h;
  Heuristic& alt_h;
  bool       max_h_val;
 public:
  static count_type lower;
  static count_type equal;
  static count_type higher;

  CompareEval(Instance& i, Heuristic& h0, Heuristic& h1)
    : Heuristic(i), base_h(h0), alt_h(h1) { }
  virtual ~CompareEval() { };

  void set_maximal_heuristic_value(bool on) { max_h_val = on; };
  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
};

class ACF {
 public:
  virtual ~ACF() { };
  virtual NTYPE operator()(index_type a) const = 0;
  virtual NTYPE min_cost(index_type n) const;
  virtual NTYPE max_cost(index_type n) const;
  virtual NTYPE avg_cost(index_type n) const;
};

class UnitACF : public ACF {
 public:
  UnitACF() { };
  virtual ~UnitACF() { };
  virtual NTYPE operator()(index_type a) const;
  virtual NTYPE min_cost(index_type n) const;
  virtual NTYPE max_cost(index_type n) const;
  virtual NTYPE avg_cost(index_type n) const;
};

class ZeroACF : public ACF {
 public:
  ZeroACF() { };
  virtual ~ZeroACF() { };
  virtual NTYPE operator()(index_type a) const;
  virtual NTYPE min_cost(index_type n) const;
  virtual NTYPE max_cost(index_type n) const;
  virtual NTYPE avg_cost(index_type n) const;
};

class CostACF : public ACF {
  Instance& instance;
 public:
  CostACF(Instance& i) : instance(i) { };
  virtual ~CostACF() { };
  virtual NTYPE operator()(index_type a) const;
};

class FracACF : public ACF {
  const ACF& baseACF;
  cost_vec   df;
 public:
  FracACF(const ACF& b, index_type l);
  FracACF(const ACF& b, index_type l, NTYPE f);
  virtual ~FracACF();
  void set(index_type a, NTYPE f);
  void set(const index_set& d, NTYPE f);
  virtual NTYPE operator()(index_type a) const;
};

class DiscountACF : public ACF {
  const ACF& baseACF;
  bool_vec   discounted;
 public:
  DiscountACF(const ACF& b, index_type l)
    : baseACF(b), discounted(false, l) { };
  DiscountACF(const ACF& b, const index_set& d, index_type l)
    : baseACF(b), discounted(d, l) { };
  DiscountACF(const ACF& b, const bool_vec& d)
    : baseACF(b), discounted(d) { };
  virtual ~DiscountACF() { };
  void discount(index_type a) { discounted[a] = true; };
  void discount(const index_set& d) { discounted.insert(d); };
  void discount(const bool_vec& d) { discounted.insert(d); };
  void count(index_type a) { discounted[a] = false; };
  void count(const index_set& d) { discounted.subtract(d); };
  void count(const bool_vec& d) { discounted.subtract(d); };
  const bool_vec& discounted_actions() { return discounted; };
  virtual NTYPE operator()(index_type a) const;
};

class MakespanACF : public ACF {
  Instance& instance;
 public:
  MakespanACF(Instance& i) : instance(i) { };
  virtual ~MakespanACF() { };
  virtual NTYPE operator()(index_type a) const;
};

class ResourceConsACF : public ACF {
  Instance&  instance;
  index_type resource_id;
 public:
  ResourceConsACF(Instance& i, index_type r);
  virtual ~ResourceConsACF() { };
  virtual NTYPE operator()(index_type a) const;
};

class ResourceReqACF : public ACF {
  Instance&  instance;
  index_type resource_id;
 public:
  ResourceReqACF(Instance& i, index_type r);
  virtual ~ResourceReqACF() { };
  virtual NTYPE operator()(index_type a) const;
};

END_HSPS_NAMESPACE

#endif
