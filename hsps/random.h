#ifndef RANDOM_H
#define RANDOM_H

#include "config.h"
#include "problem.h"
#include "preprocess.h"
#include "rng.h"

#include <math.h>

BEGIN_HSPS_NAMESPACE

class RandomInstanceName : public Name {
  unsigned long id;
  unsigned long v_id;
  index_type n;
  index_type o;
  index_type p;
  index_type q;
  index_type g;
  bool var;
  bool neg;
  bool flp;
  bool cns;

 public:
  RandomInstanceName(unsigned long _id, index_type _n, index_type _o,
		     index_type _p, index_type _q, index_type _g, bool _var,
		     bool _neg, bool _flp, bool _cns)
    : id(_id), v_id(0), n(_n), o(_o), p(_p), q(_q), g(_g), var(_var),
    neg(_neg), flp(_flp), cns(_cns) { };
  RandomInstanceName(unsigned long _id, unsigned long _v_id, index_type _n,
		     index_type _o, index_type _p, index_type _q,
		     index_type _g, bool _var, bool _neg, bool _flp, bool _cns)
    : id(_id), v_id(_v_id), n(_n), o(_o), p(_p), q(_q), g(_g), var(_var),
    neg(_neg), flp(_flp), cns(_cns) { };
  virtual ~RandomInstanceName() { };
  virtual void write(std::ostream& s, unsigned int c) const;
  void    set_variation_id(unsigned int _v_id) { v_id = _v_id; };
};

class RandomWalkName : public Name {
  const Name* base_name;
  unsigned long rnd;
  count_type forward_steps;
  count_type backward_steps;
 public:
  RandomWalkName(const Name* n, unsigned long r, count_type f, count_type b)
    : base_name(n), rnd(r), forward_steps(f), backward_steps(b) { };
  virtual ~RandomWalkName() { };
  virtual void write(std::ostream& s, unsigned int c) const;
};

extern const char* atom_prefix;
extern const char* neg_atom_prefix;
extern const char* action_prefix;

class RandomInstance : public Instance {
  unsigned long id;
  index_type param_n;
  index_type param_o;
  index_type param_p;
  index_type param_q;
  index_type param_g;
  bool model_variable;
  bool model_negative;
  bool model_flip;
  bool model_consistent;
  unsigned long v_id;
  cost_vec   v_values;

  RNG& rng;

  index_type pos_index(index_type i) { return 2*i; };
  index_type neg_index(index_type i) { return (2*i)+1; };
  index_type negate(index_type i) { return ((i % 2) ? i - 1 : i + 1); };

  void generate_action(Instance::Action& a);

 public:
  RandomInstance(index_type n, index_type o, index_type p, index_type q,
		 index_type g, bool var, bool neg, bool flp, bool cns, RNG& r);
  RandomInstance(index_type n, index_type p, index_type q,
		 index_type g, bool var, bool neg, bool flp, bool cns, RNG& r);

  static void select_fixed_atom_set(Instance& ins, index_set& s,
				    index_type p, RNG& r);
  static void select_variable_atom_set(Instance& ins, index_set& s,
				       index_type p, RNG& r);
  static void select_non_empty_atom_set(Instance& ins, index_set& s,
					index_type p, RNG& r);

  unsigned int instance_id() { return id; };
  unsigned int variation_id() { return v_id; };

  void generate_domain();
  void generate_problem();

  void set_variation_values(const cost_vec& v);
  void set_variation_type_1(unsigned long min,
			    unsigned long max,
			    unsigned long div);
  void generate_variation(RNG& r);
};

class RandomWalk2 {
  Instance&  instance;
  const ACF& cost;
  RNG&       rng;

  bool        irreducible;
  bool        ergodic;

  bool_vec    state;
  NTYPE       acc;

  PreconditionEvaluator* evaluator;
  bool_vec    allowed;
  index_type* app;

  // estimations
  count_type n_successors;
  count_type n_deadends;

  NTYPE      sum_cost;
  count_type sum_depth;

  count_type n_steps;
  count_type n_walks;

  bool step();

 public:
  RandomWalk2(Instance& ins, const ACF& c, RNG& r);
  ~RandomWalk2();

  void set_parameters(bool i, bool e)
    { irreducible = i; ergodic = e; };
  void set_evaluator(PreconditionEvaluator* e)
    { evaluator = e; };

  void disallow_action(index_type a);
  index_type n_allowed_actions();

  const bool_vec& current_state();
  NTYPE estimated_branching_factor();
  NTYPE estimated_deadend_frequency();
  NTYPE estimated_cost_to_depth();

  // restart the walk from the initial state
  void restart();

  count_type walk(count_type steps);
  index_type sample_exp(index_type avg_depth, index_type max_depth);
  index_type sample_bin(index_type avg_depth, index_type max_depth);

  // forget all collected statistics
  void reset();
};

class RandomWalk {
  Instance& instance;

  void forward_applicable_actions(const index_set& state, index_set& acts);
  void reverse_applicable_actions(const index_set& state, Heuristic& inc,
				  index_set& acts);

 public:
  RandomWalk(Instance& ins) : instance(ins) { };
  ~RandomWalk() { };

  bool walk_forward(count_type forward_steps, index_set& state, RNG& rng);
  bool walk_reverse(count_type reverse_steps, index_set& state, RNG& rng,
		    Heuristic& inc);

  Instance* next_forward_instance(count_type forward_steps, RNG& rng);
  Instance* next_instance(count_type forward_steps,
			  count_type reverse_steps,
			  RNG& rng);

};

END_HSPS_NAMESPACE

#endif
