#ifndef SAS_H
#define SAS_H

#include "config.h"
#include "problem.h"
#include "graph.h"
#include "heuristic.h"
#include "rng.h"
#ifndef NTYPE_RATIONAL
#include "rational.h"
#endif
#include "plans.h"

BEGIN_HSPS_NAMESPACE

class partial_state : public pair_set
{
 public:
  partial_state() : pair_set() { };
  partial_state(const partial_state& s) : pair_set(s) { };
  partial_state(const partial_state& s, const index_set& res);
  partial_state(const partial_state& s, const index_vec& map);

  bool defines(index_type var) const;
  bool defines_any(const index_set& set) const;
  bool defines_any_not_in(const index_set& set) const;
  bool defines_all(const index_set& set) const;
  index_type value_of(index_type var) const;
  bool consistent() const;
  bool consistent(index_type var) const;
  index_type first_inconsistent_variable() const;
  index_type first_undefined_variable(const index_set& s) const;

  void assign(index_type var, index_type val);
  void assign(const partial_state& s);
  void undefine(index_type var);
  void restrict_to(const index_set& vars);

  void defined_set(index_set& set) const;
  void value_set(index_type var, index_set& set) const;
  void conflict_set(const partial_state& s, index_set& set) const;
  bool implies(const partial_state& s) const;
  bool consistent_with(const partial_state& s) const;

  bool implies_reduced(const partial_state& s, const index_vec& map) const;
  void assign_reduced(const partial_state& s, const index_vec& map);
  bool equals_restricted(const partial_state& s, const index_set& res) const;
  bool implies_restricted(const partial_state& s, const index_set& res) const;
  void assign_restricted(const partial_state& s, const index_set& res);
};

typedef lvector<partial_state> partial_state_vec;
typedef svector<partial_state> partial_state_set;
typedef matrix<partial_state> partial_state_matrix;

class SASInstance;

class StateHashFunction {
 protected:
  index_vec    base;

 public:
  StateHashFunction()
    : base(0, 0) { };
  StateHashFunction(SASInstance& ins)
    { init(ins); };
  StateHashFunction(SASInstance& ins, const index_set& vars)
    { init(ins, vars); };
  ~StateHashFunction() { };

  // construct non-abstracting hash function (considering all variables)
  void init(SASInstance& ins);

  // construct abstracting hash function (considering only vars)
  void init(SASInstance& ins, const index_set& vars);

  // index function
  // note: input is always state in the original (i.e., non-abstract) space!
  index_type index(const partial_state& s) const;

  // define application operator for STL compliance
  index_type operator()(const partial_state& s0) const
    { return index(s0); };
};

class SASInstance {
  friend class Simplifier;

  static const Name* val_nil;
  static const Name* val_false;
  static const Name* val_true;
  static const Name* act_default_name;

  partial_state*  atom_map;
  index_set*      atom_vars;
  index_type      n_atoms;

 public:
  static bool write_symbolic_names;
  static bool write_variable_sources;
  static bool write_variable_relevance;
  static bool write_action_cost;
  static bool write_action_sources;
  static bool write_info_in_domain;

  // control iff weak_determined_check does additional check for
  // strict variables determination (defaults to true)
  static bool additional_strictness_check;

  struct Variable {
    const Name* name;
    index_type  index;
    index_set   s_index;
    name_vec    domain;
    index_type  default_val;
    bool_vec    relevant;

    // note: enabling values contains values this variables may be
    // required to achieve (due to pre/prevail conditions and goal)
    // - this is NOT the same as the requestable value set defined
    // by Jonsson & Bäckström.
    // accidental values are the values that this variable "could
    // happen to have/get" - this comprises the initial value and the
    // post condition values of any non-unary actions
    index_set   enabling_values;
    index_set   accidental_values;
    index_set   set_by;
    // cost_matrix          dtcost;
    // partial_state_matrix nsc;
    // partial_state_matrix nse;

    Variable() : name(0), index(0), domain(), default_val(no_such_index),
	 relevant(true, 0) { };

    Variable& operator=(const Variable& v) {
      name = v.name;
      index = v.index;
      s_index = v.s_index;
      domain = v.domain;
      default_val = v.default_val;
      relevant = v.relevant;
      enabling_values = v.enabling_values;
      accidental_values = v.accidental_values;
      set_by = v.set_by;
      // dtcost = v.dtcost;
      // nsc = v.nsc;
      // nse = v.nse;
      return *this;
    };

    index_type n_values() const { return domain.length(); };
    bool is_binary() const { return (s_index.length() == 1); };
  };

  struct Action {
    const Name*   name;
    index_type    index;
    index_set     s_index;
    partial_state pre;
    partial_state prv;
    partial_state post;
    NTYPE         cost;

    index_set     u_pre;

    Action() : name(0), index(0), cost(1) { };

    Action& operator=(const Action& a) {
      name = a.name;
      index = a.index;
      s_index = a.s_index;
      pre = a.pre;
      prv = a.prv;
      post = a.post;
      cost = a.cost;
      u_pre = a.u_pre;
      return *this;
    };
  };

  typedef lvector<Variable> variable_vec;
  typedef lvector<Action> action_vec;

  const Name*  name;
  variable_vec variables;
  index_vec    signature;
  StateHashFunction state_hash_function;
  graph        causal_graph;
  graph        dependency_graph;
  graph        transitive_causal_graph;
  graph        interference_graph;
  graph        independence_graph;
  action_vec   actions;

  partial_state init_state;
  partial_state goal_state;

  // variable/action index maps: only valid if this instance was constructed
  // as an abstraction of a SAS instance; reduce_map maps from the original
  // (super)instance to this, expand_map from this back to the original
  // instance; for actions, only the reduce_map is defined, because the
  // mapping is many-to-one.
  index_vec    variable_reduce_map;
  index_vec    variable_expand_map;
  index_vec    action_reduce_map;

  int          trace_level;

  // construct empty SAS instance
  SASInstance();
  // construct empty SAS instance with name
  SASInstance(const Name* n);
  // construct SAS instance from STRIPS instance
  SASInstance(const Instance& ins, bool selective, bool minimal, bool safe);
  // construct reduced (abstracted) SAS instance from SAS instance
  SASInstance(const SASInstance& ins, const index_set& vars);
  // copy
  SASInstance(const SASInstance& ins);
  ~SASInstance();

  Variable& new_variable();
  Variable& new_variable(const Name* name);
  Action& new_action(const Name* name);
  Action& new_action(const Action& act);

  index_type n_variables() const { return variables.length(); };
  index_type n_actions() const { return actions.length(); };

  void variable_names(name_vec& names) const;
  void action_names(name_vec& names) const;

  // construct SAS domain/problem from STRIPS instance
  void select_invariants(const Instance& ins,
			 Instance::constraint_vec& invs);
  void construct_variables(const Instance& ins,
			   const Instance::constraint_vec& invs);
  Variable& binary_variable(const Instance& ins, index_type atom);
  bool construct_actions(const Instance& ins);
  bool construct_minimal_actions(const Instance& ins);
  bool construct_safe_actions(const Instance& ins);
  bool construct_init_and_goal_state(const Instance& ins);
  void remove_inconsistent_actions();

  void cross_reference();
  void compute_graphs();
  void compute_extended_causal_graph(graph& g, Heuristic& inc);
  // void compute_domain_transitions(const ACF& cost);
  void compute_DTG(index_type v, graph& g);

  void remove_variables(const bool_vec& set, index_vec& map);
  void remove_actions(const bool_vec& set, index_vec& map);

  // void compute_nsce_cg(const partial_state& init,
  // 		       const partial_state& goal,
  // 		       weighted_graph& g);
  // void optimal_path_sce(index_type var,
  // 			index_type init_val,
  // 			index_type goal_val,
  // 			partial_state& nec_sc,
  // 			partial_state& nec_se,
  // 			partial_state& pos_sc,
  // 			partial_state& pos_se);

  // all construction steps wrapped in one (including removal of inconsistent
  // actions and cross referencing)
  bool construct(const Instance& ins, bool selective, bool minimal, bool safe);

  // returns a pointer to the instance atom map, zero if map is not defined
  const partial_state* atom_map_defined() const;
  // returns number of atoms for which the instances atom map is valid (zero
  // if it has no map at all)
  index_type atom_map_n() const;

  // construct partial/complete states from STRIPS atom sets
  // beware: these only work if the instance has a valid atom map!
  void make_partial_state(const bool_vec& s, partial_state& ss) const;
  void make_partial_state(const index_set& s, partial_state& ss) const;
  void make_complete_state(const bool_vec& s, partial_state& ss) const;
  void make_complete_state(const index_set& s, partial_state& ss) const;

  // slightly internal methods (note: map_to_partial_state, in difference
  // to make_partial_state, does not clear the state)
  void map_to_partial_state(const bool_vec& s, partial_state& ss) const;
  void map_to_partial_state(const index_set& s, partial_state& ss) const;
  void map_to_partial_state(index_type p, partial_state& ss) const;
  void make_state_complete(partial_state& s) const;

  void make_atom_set(const partial_state& s, index_set& set) const;
  void map_to_atom_set(index_type var, index_type val, index_set& set) const;
  void map_to_atom_set(const partial_state& s, index_set& set) const;

  void make_dual_atom_sets(const partial_state& s,
			   index_set& p_set, index_set& n_set) const;
  void map_to_dual_atom_sets(const partial_state& s,
			     index_set& p_set, index_set& n_set) const;

  void variable_atom_set(const index_set& vars, index_set& set) const;
  void atom_variable_set(const index_set& atoms, index_set& set) const;

  void map_action_set(const index_set& sas_acts, index_set& strips_acts) const;
  void map_action_set(const bool_vec& sas_acts, bool_vec& strips_acts) const;
  index_type corresponding_action(index_type strips_act) const;

  // construct a STRIPS instance from the SAS instance
  Instance* convert_to_STRIPS();
  Instance* reconstruct_STRIPS();

  // construct reduced (abstracted) SAS instance
  void construct(const SASInstance& ins, const index_set& vars);
  void construct_atom_map(const SASInstance& ins, const index_set& vars);
  SASInstance* reduce(const index_set& vars) const;
  SASInstance* reduce_to_goal_variables() const;
  SASInstance* copy() const;

  // construct random SAS instance
  void construct_random_domain(index_type n_var,
			       index_type n_val,
			       index_type dtg_type,
			       rational mixf,
			       const graph& cg,
			       index_type c_max,
			       rational t_cg,
			       index_type a_min,
			       RNG& rnd);
  void construct_random_instance(index_type n_goals,
				 const index_set& goal_vars,
				 RNG& rnd);
  void construct_random_instance(index_type n_goals,
				 RNG& rnd);

  void random_partial_state(partial_state& s,
			    index_type n_avg,
			    RNG& rng) const;

  // void extend_nsc(const partial_state& init,
  // 		  const partial_state& goal,
  // 		  partial_state& x_goal);

  // determined variable checking and replacement
  enum determined_check_result
    { determined_no,
      determined_yes,
      determined_goal_preserving,
      determined_strict };

  determined_check_result weak_determined_check
    (const index_set& set, index_type var) const;
  determined_check_result determining_set
    (index_type var, index_set& set) const;
  determined_check_result minimal_determining_sets
    (index_type var, index_set_vec& sets) const;
  determined_check_result smallest_determining_set
    (index_type var, index_set& set) const;
  void extend_determined(const partial_state& s, partial_state& x) const;

  void replace_determined_variable_in_condition(index_type var,
						const index_set& det,
						const partial_state& cond,
						partial_state_vec& repl) const;

  // replace_determined_variable returns false if the goal could not be
  // replaced (replacement goal is either inconsistent or disjunctive)
  bool replace_determined_variable(index_type var, const index_set& det);

  bool is_spanning(const index_set& set) const;
  SASInstance* reduce_to_spanning(const index_set& set) const;

  // true iff the determined extensions of s0 and s1 are consistent
  bool extended_consistent(const partial_state& s0,
			   const partial_state& s1) const;

  // check various restrictions
  bool check_P() const;
  bool check_U() const;
  bool check_B() const;
  bool check_S() const;

  // some miscellaneous methods

  bool commutative(const Action& a0, const Action& a1);
  bool commutative(index_type a0, index_type a1);

  // true iff variable sets v0 and v1 are additive (not cochanged)
  bool additive(const index_set& v0, const index_set& v1) const;

  void relevant_actions(index_type v, index_set& acts);
  rational interference_ratio(index_type var0, index_type val0,
			      index_type var1, index_type val1);

  // true iff a0 "contributes to" the achievement of a1's pre- & prevail-cond
  bool contributes_to(const Action& a0,
		      const Action& a1,
		      partial_state& i) const;

  // check safenes of all actions
  bool is_safe(bool_vec& sa);

  // construct from a sequence of actions the corresponding sequence
  // of partial states
  void action_to_partial_state_sequence(const index_vec& plan,
					partial_state_vec& psv);

  // write methods
  void write_partial_state(std::ostream& s, const partial_state& v) const;
  void write_partial_state_sequence(std::ostream& s,
				    const partial_state_vec& v) const;
  void write_variable(std::ostream& s, const Variable& var) const;
  void write_variable_info(std::ostream& s, const Variable& var) const;
  void write_action(std::ostream& s, const Action& act) const;
  void write_action_info(std::ostream& s, const Action& act) const;
  void write_domain(std::ostream& s) const;

  void write_variable_set(std::ostream& s, const index_set& set) const;
  void write_value_set(std::ostream& s, index_type var, const index_set& set) const;
  void write_action_set(std::ostream& s, const index_set& set) const;
  void write_action_sequence(std::ostream& s, const index_vec& seq) const;

  enum label_style {
    ls_none,
    ls_action_index_set,
    ls_action_name_set,
    ls_nec_side_constraints,
    ls_nec_side_constraints_and_effects,
    ls_related_variables,
    ls_action_name,
    ls_prevail_conditions,
    ls_side_conditions_and_effects
  };

  void write_domain_transition_graph(std::ostream& s,
				     index_type var,
				     label_style ls,
				     bool with_loops,
				     bool with_act_nodes,
				     bool as_subgraph) const;
  void write_composite_transition_graph(std::ostream& s) const;
  void write_dc_graph(std::ostream& s) const;
  void write_action_group_graph(std::ostream& s) const;
  void write_variable_digraph
    (std::ostream& s, const graph& g, const char* label, bool opt_reverse) const;
  void write_weighted_variable_digraph
    (std::ostream& s, const weighted_graph& g, const char* label,
     index_type n_weight_classes, index_type weight_class_delta) const;
  void write_variable_graph
    (std::ostream& s, const graph& g, const char* label) const;
  void write_weighted_variable_graph
    (std::ostream& s, const weighted_graph& g, const char* label) const;

  void write_action_sequence_graph(std::ostream& s) const;
  void write_plan_transition_graph
    (std::ostream& s, const index_vec& p, bool lockstep) const;

 protected:
  // util methods used by the various domain constructions
  index_type find_action_with_post(const partial_state& post, NTYPE cost);
  index_type find_action_with_cond(const partial_state& pre,
				   const partial_state& prv,
				   const partial_state& post);
  void construct_condition(const Instance& ins,
			   const index_set& acts,
			   partial_state& cond);
  void construct_final_action_condition(Action& act);
  index_type find_split_variable(const partial_state& cond,
				 const partial_state& post,
				 index_set& values);
  void restrict_source_set(const Instance& ins,
			   const index_set& set,
			   index_set& r_set,
			   index_type var,
			   index_type val);
  void new_action_with_replacement_condition(const SASInstance::Action& act,
					     const partial_state& c,
					     index_type var,
					     index_type val);

  void make_random_action(index_type v,
			  index_type t,
			  pair_set* trans,
			  bool_vec* t_free,
			  index_type& n_free,
			  const graph& cg,
			  index_type c_max,
			  bool prefer_free,
			  RNG& rnd);

  // computes side constraints/effects of action act w.r.t. variable var
  partial_state side_constraints(index_type act, index_type var);
  partial_state side_effects(index_type act, index_type var);

  // void path_sce(index_type var,
  // 		index_type init_val,
  // 		index_type goal_val,
  // 		NTYPE bound,
  // 		const partial_state& sc,
  // 		const partial_state& se,
  // 		bool& solved,
  // 		partial_state& nec_sc,
  // 		partial_state& nec_se,
  // 		partial_state& pos_sc,
  // 		partial_state& pos_se);
};

inline std::ostream& operator<<(std::ostream& s, const SASInstance::determined_check_result& r)
{
  if (r == SASInstance::determined_strict) {
    s << "strictly determined";
  }
  else if (r == SASInstance::determined_goal_preserving) {
    s << "determined (goal-preserving)";
  }
  else if (r == SASInstance::determined_yes) {
    s << "determined (not goal-preserving)";
  }
  else {
    s << "not determined";
  }
}

class PartialStatePerfectHashFunction : public index_vec {
 protected:
  index_type   size;
  bool overflow_flag;

  // non-initializing constructor (for use by subclass constructors only)
  PartialStatePerfectHashFunction(SASInstance& ins);

 public:
  PartialStatePerfectHashFunction(SASInstance& ins, const index_set& vars);
  ~PartialStatePerfectHashFunction() { };

  index_type n_values() const { return size; };
  bool overflow() const { return overflow_flag; };

  // index function and its inverse
  // note: input/out are states in the original (i.e., non-abstract) space!
  index_type index(const partial_state& s) const;
  void state(index_type i, partial_state& s) const;

  index_type index1(index_type var, index_type val) const;

  // define application operator for STL compliance
  index_type operator()(const partial_state& s0) const
    { return index(s0); };
};

class CompleteStatePerfectHashFunction
: public PartialStatePerfectHashFunction
{
 protected:
  index_type n_vars;

 public:
  CompleteStatePerfectHashFunction(SASInstance& ins, const index_set& vars);
  ~CompleteStatePerfectHashFunction() { };

  index_type n_values() const { return size; };
  bool overflow() const { return overflow_flag; };

  // index function and its inverse
  // note: input/out are states in the original (i.e., non-abstract) space!
  index_type index(const partial_state& s) const;
  void state(index_type i, partial_state& s) const;

  index_type index1(index_type var, index_type val) const;

  // define application operator for STL compliance
  index_type operator()(const partial_state& s0) const
    { return index(s0); };
};


class IndependentVariableSets : public index_set_vec {
  SASInstance&  instance;

  void compute_extended_set(const index_set& s,
			    const index_set& u,
			    bool preseve_goals,
			    index_set& x);
  void find_spanning_sets
    (const index_set& cs, // current sets
     const index_set& cv, // variables in U{cs}
     const index_set_vec& dets, // ind. sets extended with det. variables
     const index_set& av, // set of all variables
     index_set& bs,       // best solution so far
     index_type& bs_nv,   // #variables in U{bs}
     bool preseve_goals,
     index_type& n_bt,    // #backtracks so far
     index_type max_bt);  // backtrack limit (no_such_index == unlimited)

 public:
  IndependentVariableSets(SASInstance& ins);
  IndependentVariableSets(SASInstance& ins, const index_set& vars);
  IndependentVariableSets(const IndependentVariableSets& ivs);
  ~IndependentVariableSets();

  void compute_maximal_independent_sets();
  void compute_approximate_independent_sets();
  void compute_spanning_sets(bool preseve_goal_sets = false,
			     index_type max_bt = no_such_index);
};

class IndependentVariables {
  SASInstance&  instance;
  index_set_vec independent;
  index_set_vec determined;
  graph         covers;
  graph         covers_scc;
  index_set     spanning_sets;
  index_set     spanning_vars;

  bool          have_independent;
  bool          have_spanning;

  void compute_determined();

 public:
  IndependentVariables(SASInstance& ins)
    : instance(ins), have_independent(false), have_spanning(false) { };
  ~IndependentVariables() { };

  index_type n_sets();
  const index_set& set(index_type i);
  const index_set& spanning_variables();

  void compute_maximal_independent_sets();
  void compute_approximate_independent_sets();
  void compute_spanning_sets();

  SASInstance* reduced_instance();
};

class PrintSASActions : public Plan, public PlanSet {
 protected:
  SASInstance& instance;
  ::std::ostream&   to;
  char    action_sep;
  bool  first_action;
  char      plan_sep;
  bool    first_plan;

 public:
  PrintSASActions(SASInstance& i, ::std::ostream& s);
  PrintSASActions(SASInstance& i, ::std::ostream& s, char as);
  PrintSASActions(SASInstance& i, ::std::ostream& s, char as, char ps);
  virtual ~PrintSASActions();

  virtual Plan* new_plan();
  virtual void protect(index_type atom);
  virtual void insert(index_type act);
  virtual void advance(NTYPE delta);
  virtual void end();
};

class SASPlanSummary : public Plan {
  SASInstance& instance;
  index_vec n_value_changes;
  index_vec n_required_values;
  bool need_to_clear;
 public:
  SASPlanSummary(SASInstance& i);
  virtual ~SASPlanSummary() { };

  virtual void protect(index_type atom);
  virtual void insert(index_type act);
  virtual void advance(NTYPE delta);
  virtual void end();

  void clear();
  double average_variable_value_changes();
  index_type min_variable_value_changes();
  index_type max_variable_value_changes();
  double average_variable_required_values();
  index_type n_secondary_goal_variables();
};

END_HSPS_NAMESPACE

#endif
