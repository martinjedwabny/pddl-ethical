#ifndef PDB_CONSTRUCTION_H
#define PDB_CONSTRUCTION_H

#include "config.h"
#include "pdb.h"

BEGIN_HSPS_NAMESPACE

// #define TEST_USE_INCREMENTAL_PDB

// various PDB-based heuristics

class PDBHeuristic : public MaxH {
 protected:
  SASInstance&  instance;
  Statistics&   stats;
  sas_heuristic_vec pdbs;

  index_set_vec sets;
  index_set_vec discount;
  index_set_vec groups;

  void remove_subsets();

 public:
  PDBHeuristic(SASInstance& ins, Statistics& s);
  virtual ~PDBHeuristic();

  index_type n_variable_sets() const { return sets.length(); };
  const index_set& variable_set(index_type i) const { return sets[i]; };
  index_type n_additive_groups() const { return groups.length(); };
  const index_set& additive_group(index_type i) const { return groups[i]; };

  void write_variable_sets(std::ostream& s);
  void write_additive_groups(std::ostream& s);
  virtual void write(std::ostream& s);

  void assign_sets(const index_set_vec& s);
  index_type new_component(SASHeuristic* h)
    { return MaxH::new_component(h); };
  index_type new_component(SASHeuristic* h,
			   const index_set& v,
			   const index_set& d);

  void compute_regression_PDB(const ACF& cost, MDDNode* sinc, Heuristic* inc);
  void compute_progression_PDB(const ACF& cost, MDDNode* sinc, Heuristic* inc);
  void compute_additive_groups(bool maximal);
  void make_additive_groups();
  void make_extended_additive_groups(const ACF& cost,
				     MDDNode* sinc,
				     Heuristic* inc,
				     rational threshold);

  MaxAddH* make_max_add();

  // util functions that do NOT change the PDB heuristic
  void relevant_actions(const index_set& vars, index_set& acts) const;
  void interfering_actions(const index_set& vars0, const index_set& vars1,
			   index_set& acts) const;
  bool additive(const index_set& vset0, const index_set& vset1) const;

  // heuristic bin packing methods (implemented here for convenience
  // - used by several PDB construction strategies)
  void pack_best_fit(const index_set& vars,
		     index_type bin_size,
		     const SetSizeFunction& set_size,
		     index_set_vec& bins) const;
  void pack_first_fit(const index_set& vars,
		      index_type bin_size,
		      const SetSizeFunction& set_size,
		      index_set_vec& bins) const;

  bool sum_is_dominated(const index_set &dominatee,
			const index_set &dominator,
			const graph& g_subset) const;
  void find_maximal_additive_groups(index_set_vec& groups) const;
};

// optimal PDB bin packing
class MinimalPDBPacking : public RecursivePartitionEnumerator {
  index_set variables;
  const SetSizeFunction& set_size;
  index_type bin_size;

 protected:
  virtual void solution();

 public:
  MinimalPDBPacking(index_type n,
		    const SetSizeFunction& s,
		    index_type b);
  MinimalPDBPacking(const index_set& v,
		    const SetSizeFunction& s,
		    index_type b);
  virtual ~MinimalPDBPacking() { };

  bool pack();
  void result(index_set_vec& vss);
};

class MaxBinPDB : public PDBHeuristic {
 public:
  MaxBinPDB(SASInstance& ins, Statistics& s) : PDBHeuristic(ins, s) { };
  virtual ~MaxBinPDB() { };

  // compute_sets_optimal uses optimal bin packing, while compute_sets uses
  // heuristic packing
  bool compute_sets_optimal(index_type pdb_size,
			    const SetSizeFunction& set_size);
  void compute_sets(index_type pdb_size,
		    const SetSizeFunction& set_size);
};

class RandomBinPDB : public PDBHeuristic {
 public:
  RandomBinPDB(SASInstance& ins, Statistics& s) : PDBHeuristic(ins, s) { };
  virtual ~RandomBinPDB() { };

  void compute_sets(index_type pdb_size,
		    const SetSizeFunction& set_size,
		    RNG& rng, index_type n_swaps);
  void compute_sets(const index_set& vars,
		    index_type pdb_size,
		    const SetSizeFunction& set_size,
		    RNG& rng, index_type n_swaps);
};

class RandomIndependentBinPDB : public PDBHeuristic {
 public:
  RandomIndependentBinPDB(SASInstance& ins, Statistics& s)
    : PDBHeuristic(ins, s) { };
  virtual ~RandomIndependentBinPDB() { };

  void compute_sets(index_type pdb_size,
		    const SetSizeFunction& set_size,
		    bool select_spanning_subset,
		    bool collapse_small_bins,
		    RNG& rng, index_type n_swaps);
};

class Max1PDB : public PDBHeuristic {
 public:
  Max1PDB(SASInstance& ins, Statistics& s) : PDBHeuristic(ins, s) { };
  virtual ~Max1PDB() { };

  void compute_sets();
  void compute_sets(const index_set& vars);
};

class MaxCGPDB : public PDBHeuristic {
 public:
  MaxCGPDB(SASInstance& ins, Statistics& s)
    : PDBHeuristic(ins, s) { };
  virtual ~MaxCGPDB() { };

  void compute_sets(index_type pdb_size, const SetSizeFunction& set_size);
};

class MaxInterferencePDB : public PDBHeuristic {
 public:
  MaxInterferencePDB(SASInstance& ins, Statistics& s)
    : PDBHeuristic(ins, s) { };
  virtual ~MaxInterferencePDB() { };

  void compute_sets(index_type pdb_size, const SetSizeFunction& set_size);
  void compute_reduced_sets(index_type pdb_size,
			    const SetSizeFunction& set_size);
};

class IndependentPairPDB : public PDBHeuristic {
  index_set_vec independent_variable_sets;
  index_vec     first;
  index_vec     n_pairs;

  index_type pair_index(index_type set, index_type v0, index_type v1);
  void compute_all_additive_groups(index_type set,
				   bool_vec& unassigned,
				   index_set& pairs);
  bool ok_to_add(index_type s, index_type g);
  bool find_assignment(index_type g0, index_type g_end, index_type g_size,
		       index_type s0, index_type s_end,
		       bool_vec& assigned, index_type cur_g);

 public:
  IndependentPairPDB(SASInstance& ins, Statistics& s)
    : PDBHeuristic(ins, s), first(0,0), n_pairs(0,0) { };
  virtual ~IndependentPairPDB() { };

  void compute_sets();
  void compute_additive_groups(bool maximal);
  void compute_all_additive_groups();
  void compute_min_additive_groups();
};

class IndependentBinPDB : public PDBHeuristic {
 public:
  IndependentBinPDB(SASInstance& ins, Statistics& s)
    : PDBHeuristic(ins, s) { };
  virtual ~IndependentBinPDB() { };

  // compute_sets_optimal uses optimal bin packing, while compute_sets uses
  // heuristic packing
  void compute_sets_optimal(index_type pdb_size,
			    const SetSizeFunction& set_size);
  void compute_sets(index_type pdb_size,
		    const SetSizeFunction& set_size);
};

typedef MonadicFunction<index_set,rational> SetValueFunction;

class SetValueSum : public SetValueFunction {
  SetValueFunction& svf0;
  rational weight0;
  SetValueFunction& svf1;
  rational weight1;
 public:
  SetValueSum(SetValueFunction& f0, SetValueFunction& f1)
    : svf0(f0), weight0(1), svf1(f1), weight1(1) { };
  SetValueSum(SetValueFunction& f0, const rational& w0,
	      SetValueFunction& f1, const rational& w1)
    : svf0(f0), weight0(w0), svf1(f1), weight1(w1) { };
  virtual ~SetValueSum() { };

  virtual rational operator()(const index_set& set) const;
};

class InitialStateInterference : public SetValueFunction {
  SASInstance& instance;
 public:
  InitialStateInterference(SASInstance& ins) : instance(ins) { };
  virtual ~InitialStateInterference() { };

  virtual rational operator()(const index_set& set) const;
};

class GoalStateInterference : public SetValueFunction {
  SASInstance& instance;
 public:
  GoalStateInterference(SASInstance& ins) : instance(ins) { };
  virtual ~GoalStateInterference() { };

  virtual rational operator()(const index_set& set) const;
};

class AffectingActionFraction : public SetValueFunction {
  SASInstance& instance;
 public:
  AffectingActionFraction(SASInstance& ins) : instance(ins) { };
  virtual ~AffectingActionFraction() { };

  virtual rational operator()(const index_set& set) const;
};

class InverseCGFraction : public SetValueFunction {
  SASInstance& instance;
 public:
  InverseCGFraction(SASInstance& ins) : instance(ins) { };
  virtual ~InverseCGFraction() { };

  virtual rational operator()(const index_set& set) const;
};

class CombinedSetValue : public SetValueFunction {
  InitialStateInterference f_isi;
  InverseCGFraction f_icg;
 public:
  CombinedSetValue(SASInstance& ins) : f_isi(ins), f_icg(ins) { };
  virtual ~CombinedSetValue() { };

  virtual rational operator()(const index_set& set) const;
};

class MaxWeightedPDBPacking
: public RecursivePartitionEnumerator
{
  index_set     variables;
  const SetSizeFunction& set_size;
  index_type    bin_size;
  const SetValueFunction& value;
  bool          solved;

 protected:
  virtual void solution();

 public:
  index_set_vec best;
  rational best_value;

  MaxWeightedPDBPacking(const index_set& v,
			const SetSizeFunction& s,
			index_type b,
			const SetValueFunction& f);
  virtual ~MaxWeightedPDBPacking() { };

  bool pack();
};

#define MAX_SWAPS 100 // the idiot-safety limit for local searches...

class WeightedBinPDB : public PDBHeuristic {
 protected:
  const SetValueFunction& value;

  index_type max_size_fit(const index_set& vars,
			  const SetSizeFunction& set_size,
			  index_type capacity);
  index_type best_value_fit(const index_set& vars,
			    const SetSizeFunction& set_size,
			    index_type capacity,
			    const index_set& sack);
  void single_knapsack(index_set& vars,
		       const SetSizeFunction& set_size,
		       index_type capacity,
		       index_set& sack);
  void multi_knapsack(const index_set& vars,
		      const SetSizeFunction& set_size,
		      index_type capacity,
		      index_set_vec& sacks);
  void local_search_swap(const SetSizeFunction& set_size,
			 index_type pdb_size,
			 index_type set0,
			 index_type setn);
  void local_search_move(const SetSizeFunction& set_size,
			 index_type pdb_size,
			 index_type set0,
			 index_type setn);

 public:
  WeightedBinPDB
    (SASInstance& ins, Statistics& s, const SetValueFunction& v)
    : PDBHeuristic(ins, s), value(v) { };
  virtual ~WeightedBinPDB() { };

  void compute_sets(const index_set_vec& varsets,
		    index_type pdb_size,
		    const SetSizeFunction& set_size);
};

class WeightedIndependentBinPDB : public WeightedBinPDB {
 public:
  WeightedIndependentBinPDB
    (SASInstance& ins, Statistics& s, const SetValueFunction& v)
    : WeightedBinPDB(ins, s, v) { };
  virtual ~WeightedIndependentBinPDB() { };

  void compute_sets(const index_set& vars,
		    index_type pdb_size,
		    const SetSizeFunction& set_size,
		    bool select_spanning_subset,
		    bool collapse_small_bins);
  void compute_sets_optimal(const index_set& vars,
			    index_type pdb_size,
			    const SetSizeFunction& set_size,
			    bool select_spanning_subset,
			    bool collapse_small_bins);
};

class AdditiveIncrementalRegressionPDB : public PDBHeuristic {
  void analyze_plan(const index_vec& plan,
		    index_type c_first,
		    index_type c_last,
		    index_type c_cur,
		    MDDNode* sinc,
		    Heuristic* inc,
		    Heuristic* cw,
		    weighted_graph& rg);
  void partition(const partial_state& goal,
		 index_type pdb_size,
		 const index_set& set,
		 bool check_all_abstract_plans,
		 const ACF& cost,
		 MDDNode* sinc,
		 Heuristic* inc,
		 Heuristic* cw);

 public:
  AdditiveIncrementalRegressionPDB(SASInstance& ins, Statistics& s)
    : PDBHeuristic(ins, s) { };
  virtual ~AdditiveIncrementalRegressionPDB() { };

  void compute_pdbs(const partial_state& goal,
		    index_type pdb_size,
		    bool optimal_additive_sets,
		    bool select_spanning_subset,
		    bool collapse_small_sets,
		    bool check_all_abstract_plans,
		    const ACF& cost,
		    MDDNode* sinc,
		    Heuristic* inc,
		    Heuristic* cw);
};

class IncrementalProgressionPDB : public PDBHeuristic {

  NTYPE test1(SASHeuristic* h_base,
#ifdef TEST_USE_INCREMENTAL_PDB
	      ProgressionPDB* h_new
#else
	      ProgressionASH* h_new
#endif
	      );

  NTYPE test3(SASHeuristic* h_base,
	      NTYPE v_max_base,
#ifdef TEST_USE_INCREMENTAL_PDB
	      ProgressionPDB* h_new,
#else
	      ProgressionASH* h_new,
#endif
	      NTYPE v_max_new,
	      MDDNode* sinc,
	      Heuristic* inc,
	      index_type n_samples,
	      NTYPE& dd_val,
	      RNG& rng);

 public:
  IncrementalProgressionPDB(SASInstance& ins, Statistics& s)
    : PDBHeuristic(ins, s) { };
  virtual ~IncrementalProgressionPDB() { };


  NTYPE test_merge3(ProgressionPDB* pdb1,
		    ProgressionPDB* pdb2,
		    const ACF& cost,
		    MDDNode* sinc,
		    Heuristic* inc,
		    index_type n_samples,
		    NTYPE& dd_val,
		    RNG& rng);

  NTYPE test_grow3(ProgressionPDB* pdb1,
		   index_type v_new,
		   const ACF& cost,
		   MDDNode* sinc,
		   Heuristic* inc,
		   index_type n_samples,
		   NTYPE& dd_val,
		   RNG& rng);

  void compute_sets(const index_set& vars,
		    index_type pdb_size,
		    index_type set_size,
		    const ACF& cost,
		    MDDNode* sinc,
		    Heuristic* inc,
		    index_type n_best,
		    index_type n_samples,
		    RNG& rng);
};

class MaxMatchPDB : public SASHeuristic {
  SASInstance& instance;
  Statistics& stats;
  index_set variables;
  sas_heuristic_vec variable_pdb;
  lvector<sas_heuristic_vec> pair_pdb;
  weighted_graph variable_graph;
  bool_vec       matched;

 public:
  MaxMatchPDB(SASInstance& ins, const index_set& avs, Statistics& s);
  virtual ~MaxMatchPDB();

  static MaxH* independent_pair_regression_heuristic
    (SASInstance& ins, Statistics& s, ACF& cost, Heuristic* inc);
  static MaxH* independent_pair_progression_heuristic
    (SASInstance& ins, Statistics& s, ACF& cost, Heuristic* inc);

  void compute_regression_PDB(const ACF& cost, Heuristic* inc);
  void compute_progression_PDB(const ACF& cost, Heuristic* inc);

  virtual NTYPE eval(const partial_state& s);
};


// new design
//
// class SimplePDBConstruction {
//  public:
//   virtual const index_set_vec&
//     compute(const index_set& v, const SetSizeFunction& s, index_type b) = 0;
// };

END_HSPS_NAMESPACE

#endif
