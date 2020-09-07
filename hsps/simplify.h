#ifndef SIMPLIFY_H
#define SIMPLIFY_H

#include "config.h"
#include "sas.h"
#include "preprocess.h"
#include "pdb.h"
#include "plans.h"

BEGIN_HSPS_NAMESPACE

class Simplifier {
 protected:
  SASInstance* instance;
  Statistics&  stats;

  // eliminate_determined_variables submethods
  void compute_elimination_graph(graph& determination_graph,
				 index_set& eliminable_variables);

  // reduce_domains submethods
  void compute_free_domain_transition_graph(index_type var, graph& fdtg);
  void compute_domain_equivalence(index_type var, equivalence& eq);
  partial_state reduce_condition(const partial_state& c,
				 index_vec& var_map,
				 index_matrix& val_map);

  // minimize submethods
  void mark(index_type i,
	    index_type j,
	    graph& neq,
	    std::map<index_pair, pair_set>& pending);
  bool compute_state_equivalence(const index_set& vars,
				 index_type limit,
				 equivalence& eq);
  void canonize_action(SASInstance::Action& act,
		       const partial_state& cs_pre,
		       const partial_state& cs_post);

  // compose_variables submethods
  bool build_composite_values(const partial_state& s,
			      const index_set& vars,
			      partial_state_vec& vals,
			      index_type limit);
  SASInstance::Variable&
    build_composite_variable(const index_set& vars,
			     partial_state_vec& vals,
			     SASInstance* new_instance);
  void build_actions(const SASInstance::Action& act,
		     const index_set_vec& cvar,
		     const partial_state_matrix& cval,
		     const index_set& uvar,
		     SASInstance* new_instance);
  void build_actions1(const SASInstance::Action& act,
		      const index_set_vec& cvar,
		      const partial_state_matrix& cval,
		      const index_set& uvar,
		      const index_set& pre_cond_cvar,
		      index_type next_p,
		      const index_set& post_cond_cvar,
		      const index_set& prv_cond_cvar,
		      const partial_state& c_cond,
		      partial_state& pre_cond,
		      partial_state& post_cond,
		      partial_state& prv_cond,
		      SASInstance* new_instance);
  void build_actions2(const SASInstance::Action& act,
		      const index_set_vec& cvar,
		      const partial_state_matrix& cval,
		      const index_set& uvar,
		      const index_set& pre_cond_cvar,
		      const index_set& post_cond_cvar,
		      const index_set& prv_cond_cvar,
		      index_type next_p,
		      const partial_state& c_cond,
		      const partial_state& pre_cond,
		      partial_state& post_cond,
		      partial_state& prv_cond,
		      SASInstance* new_instance);
  void build_actions3(const SASInstance::Action& act,
		      const index_set_vec& cvar,
		      const partial_state_matrix& cval,
		      const index_set& uvar,
		      const index_set& pre_cond_cvar,
		      const index_set& post_cond_cvar,
		      index_type next_p,
		      const index_set& prv_cond_cvar,
		      const partial_state& c_cond,
		      const partial_state& pre_cond,
		      partial_state& post_cond,
		      const partial_state& prv_cond,
		      SASInstance* new_instance);
  void build_condition_set(const partial_state& cond,
			   const index_set_vec& cvar,
			   const partial_state_matrix& cval,
			   const index_set& uvar,
			   partial_state_set& cond_set);
  void build_condition_set1(const partial_state& cond,
			    const index_set_vec& cvar,
			    const partial_state_matrix& cval,
			    const index_set& uvar,
			    const index_set& cond_cvar,
			    index_type next_p,
			    partial_state& new_cond,
			    partial_state_set& cond_set);

  // linear sequence stuff
  void find_linear_action_sequences(SASInstance::Variable& var,
				    index_type val,
				    const bool_vec& erv,
				    const index_set_vec& app,
				    index_vec& aseq,
				    index_vec& vseq,
				    bool_vec& visited,
				    ActionSequenceSet& seqs);
  bool compose_linear_action_sequences(SASInstance::Variable& var);
  bool compose_action_sequences(index_set& vars);

  // make a copy of ins with variables and atom map only (no actions)
  SASInstance* copy_variables(SASInstance* ins);

 public:
  static int   default_trace_level;
  int          trace_level;

  static index_type composition_limit_order;
  static index_type composition_limit_factor;

  Simplifier(SASInstance* ins, Statistics& s);
  ~Simplifier();

  void extend_atom_map(Preprocessor& prep);
  bool compose_variables(const index_set_vec& sets, const index_vec& limit);
  bool minimize(const index_set& vars, index_type limit);

  bool eliminate_strictly_determined_variables();
  bool eliminate_determined_variables();
  bool reduce_variable_domains();
  bool remove_irrelevant_variables();

  bool apply_old_WSA();
  bool apply_WSA(index_type limit, bool single_variable_only);
  bool apply_minimization(index_type limit);
  bool apply_variable_composition();
  bool apply_sequence_composition();

  bool check_WSA(const index_set& vars, index_type limit);
  bool check_WSA(index_type var);

  bool split_non_unary_actions();

  SASInstance* result() { return instance; };
  void write_elimination_graph(std::ostream& s);
};

END_HSPS_NAMESPACE

#endif
