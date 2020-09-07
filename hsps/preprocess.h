#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "config.h"
#include "problem.h"
#include "cost_table.h"
#include "graph.h"

BEGIN_HSPS_NAMESPACE

class Preprocessor {
 public:
  Instance&   instance;
  index_vec   atom_map;
  index_vec   action_map;
  index_vec   inv_map;

 protected:
  Statistics& stats;
  CostTable*  inc_relation;
  graph*      inc_graph;

  bool make_invariant(index_type init_atom,
		      index_set& inv_set,
		      bool& exact,
		      index_set& branch_set);
  void analyze_branch_set(const index_set& branch_set,
			  const index_set& inv_set,
			  index_set& effective);
  void dfs_find_invariants(index_type init_atom,
			   const index_set& base_set,
			   const index_set& branch_set,
			   index_type branch_depth,
			   index_type max_branch_depth);

  void extend_LsC1_relevance(const index_set& check,
			     const graph& lmg,
			     const bool_vec& p_act,
			     const bool_vec& p_add,
			     bool_vec& rel_atms,
			     bool_vec& rel_acts);

 public:
  static int   default_trace_level;
  int          trace_level;

  Preprocessor(Instance& ins, Statistics& s);
  ~Preprocessor();

  // Methods that do NOT change the instance:

  // computes reachable atoms and actions
  void compute_reachability(bool_vec& reachable_atoms,
			    bool_vec& reachable_actions,
			    bool opt_H2 = false);

  // computes statically true atoms, uses action reachability
  void compute_static_atoms(const bool_vec& reachable_actions,
			    bool_vec& static_atoms);

  // compute recursively relevant atoms/actions
  void compute_relevance(const index_set& check,
			 bool_vec& r_atm,
			 bool_vec& r_act);
  void compute_relevance(const index_set& check,
			 index_type d_check,
			 index_vec& d_atm,
			 index_vec& d_act);

  void between(index_type p,
	       index_type q,
	       const graph& lmg,
	       bool_vec& b_atm,
	       bool_vec& b_act);

  void compute_L1C1_relevance(const index_set& check,
			      index_type d_check,
			      const graph& lmg,
			      index_vec& d_atm,
			      index_vec& d_act);
  void compute_L1C1_relevance(const index_set& check,
			      const graph& lmg,
			      bool_vec& rel_atms,
			      bool_vec& rel_acts);

  void compute_LsC1_relevance(const index_set& check,
			      const graph& lmg,
			      bool_vec& rel_atms,
			      bool_vec& rel_acts);

  void compute_relaxed_relevance(const index_set& g,
				 index_vec& path,
				 const index_set& pre,
				 bool_vec& rel_acts);

  // Compute actions relevant for achieving atom p within cost estimate
  // according to heuristic h. The ACF should be the same as used to
  // compute h.
  void strictly_relevant_actions(index_type p,
				 Heuristic& h,
				 const ACF& f,
				 bool_vec& rel);

  // Internal method (used by strictly_relevant_actions above)
  void strictly_relevant_actions(index_type p,
				 NTYPE c,
				 Heuristic& h,
				 const ACF& f,
				 bool_vec& rel);

  // Compute actions and atoms relevant for achieving atom p (resp. set s)
  // within cost c, according to heuristic h. The ACF should be the same as
  // used to compute h.
  void relevant_at_cost(index_type p,
			NTYPE c,
			Heuristic& h,
			const ACF& f,
			bool_vec& r_atoms,
			bool_vec& r_actions);
  void relevant_at_cost(const index_set& s,
			NTYPE c,
			Heuristic& h,
			const ACF& f,
			bool_vec& r_atoms,
			bool_vec& r_actions);

  // returns a pointer to the inconsistency relation and inconsistency graph,
  // respectively, computing or recomputing them if necessary
  CostTable* inconsistency();
  graph*     inconsistency_graph();

  // check consistency of a pair of atoms, resp. a set of atoms and an atom,
  // using the specified heuristic
  bool inconsistent(index_type p, index_type q, Heuristic& inc);
  bool consistent(index_type p, index_type q, Heuristic& inc);
  bool inconsistent(const index_set& s, index_type p, Heuristic& inc);
  bool consistent(const index_set& s, index_type p, Heuristic& inc);

  // check consistency of a pair of atoms, a set of atoms, or a set of atoms
  // and an atom, using the preprocessors inconsistency relation
  bool inconsistent(index_type p, index_type q);
  bool consistent(index_type p, index_type q);
  bool inconsistent(const index_set& s);
  bool consistent(const index_set& s);
  bool inconsistent(const index_set& s, index_type p);
  bool consistent(const index_set& s, index_type p);
  bool inconsistent(const index_set& s0, const index_set& s1);
  bool consistent(const index_set& s0, const index_set& s1);

  // check entailment (depends on invariants and inconsistency relation)
  bool implies(index_type p, index_type q, Heuristic& inc);
  bool implies(const index_set& s, index_type q, Heuristic& inc);
  bool implies(index_type p, index_type q);
  bool implies(const index_set& s, index_type q);

  // iteratively compute set of atoms implied by atom set s
  void implied_atom_set(const index_set& s, index_set& is, Heuristic& inc);

  // check if atom p is a landmark (instance is unsolvable iff p can
  // not be made true).
  bool landmark_test(const index_set& init,
		     const index_set& goal,
		     index_type p,
		     bool opt_H2 = false);
  void compute_landmarks(const index_set& init,
			 const index_set& goal,
			 index_set& landmark_atoms);
  void compute_landmarks(bool exclude_goal,
			 bool exclude_init,
			 bool_vec& landmark_atoms);
  void compute_landmarks(bool exclude_goal,
			 bool exclude_init,
			 index_set& landmark_atoms);

  void quick_landmark_graph(graph& g, bool opt_H2 = false);
  void compute_landmark_graph(graph& g, bool opt_H2 = false);

  // compute an abstraction hierarchy for the instance using Knoblock's
  // algorithm (second method returns also the "atom criticality" graph)
  void compute_hierarchy(index_set_graph& g);
  void compute_hierarchy(graph& g0, index_set_graph& g);

  // find atoms that need to be completed with negations for transformation
  // to 1-safe instance to be possible
  void necessary_completions_for_safeness(index_set& atoms_missing_negation);

  // Methods that ADD (information) TO the instance:

  // marks atoms not relevant to goals in instance
  void compute_irrelevant_atoms();

  // synthesize invariants and add to instance
  void dfs_find_invariants(index_type max_branch_depth);
  void bfs_find_invariants();

  void find_inconsistent_set_invariants(graph& inc);
  void find_maximal_inconsistent_set_invariants(graph& inc);
  void find_binary_iff_invariants(bool opt_weak_verify);

  // verify invariant(s) and strengthen/weaken exactness
  bool verify_invariant(Instance::Constraint& inv, Heuristic& inc);
  bool verify_invariants(Heuristic& inc);

  // compute ncw sets with/without the help of an inconsistency relation
  void compute_ncw_sets(Heuristic& inc);
  void compute_ncw_sets();

  // Methods that REMOVE things from, or otherwise CHANGE the instance:

  // perform standard preprocessing
  // (remove static atoms and unreachable actions, re-cross-ref)
  void preprocess(bool opt_H2 = false);

  // remove atoms marked irrelevant from instance (re-cross-ref)
  void remove_irrelevant_atoms();

  // remove invariants not marked as verified
  void remove_unverified_invariants();

  // remove actions with no effects and invariants with <= limit atoms
  void remove_useless_actions();
  void remove_useless_invariants();

  // remove preconditions entailed by other preconditions (check uses
  // implies method above); minimality of the remaining set of preconditions
  // is not guaranteed; for some reason, this is not safe (in combination
  // with something else, maybe...)
  void remove_redundant_preconditions();

  // remove atoms equivalent to EXISTING conjunctions of other atoms
  void eliminate_strictly_determined_atoms();

  // make actions of the instance 1-safe; this requires the instance to
  // have complete atom negations
  void make_safe();

  void apply_place_replication();

  // Visualization utility methods

  // write heuristic graph for some/all atoms/actions
  void write_heuristic_graph(std::ostream& s,
			     Heuristic& h,
			     const ACF& cost,
			     const bool_vec& atoms,
			     const bool_vec& actions);

  void write_heuristic_graph(std::ostream& s,
			     Heuristic& h,
			     const ACF& cost);

  void write_relevance_graph(std::ostream& s,
			     const index_vec& d_atm,
			     const index_vec& d_act,
			     const char* label);

  void write_relevance_graph(std::ostream& s,
			     const index_vec& d_atm,
			     const index_vec& d_act,
			     const bool_vec& mark_atm,
			     const bool_vec& mark_act,
			     const char* label);

  // Methods under development...
  bool find_inverse_actions(const Instance::Action& act, Heuristic& inc,
			    index_set& inverses);
};

END_HSPS_NAMESPACE

#endif
