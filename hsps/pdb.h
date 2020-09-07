#ifndef PDB_H
#define PDB_H

#include "config.h"
#include "sas_heuristic.h"
#include "mdd.h"
#include "enumerators.h"
#include "function.h"
#include "bfs.h"
#include "random.h"

#include <queue>
#include <ext/hash_map>
#include <functional>

BEGIN_HSPS_NAMESPACE

// StateAbstraction implements a variable abstraction mapping, and some
// utility functions defined via the mapping.
class StateAbstraction {
 protected:
  SASInstance& instance;
  index_set    variable_set; // set of variables in the abstraction
  index_vec    reduce_map;   // map reducing full states to abstract states
 public:
  StateAbstraction(SASInstance& ins, const index_set& vars);
  virtual ~StateAbstraction();

  const index_set& variables() const { return variable_set; };
  const index_vec& map() const { return reduce_map; };
  index_type       size() const;

  bool additive(const index_set& v) const;
  bool additive(const StateAbstraction& a) const
    { return additive(a.variables()); };

  // apply abstraction to state s
  partial_state apply(const partial_state& s) const;

  // true iff s0 equals s1 in the abstract space
  bool equals(const partial_state& s0, const partial_state& s1) const;
  // true iff s implies c in the abstract space
  bool implies(const partial_state& s, const partial_state& c) const;
  // true iff s0 and s1 are consistent in the abstract space
  bool consistent(const partial_state& s0, const partial_state& s1) const;
  // assignment s := c, restricted to variables in the abstraction
  void assign(partial_state& s, const partial_state& c) const;

  // check if abstraction of state s is consistent according to MDD
  // (asserts sinc != 0; asserts inc != and the existence of instance atom
  // map iff CHECK_MDD_EVAL is defined)
  bool consistent(const partial_state& s, MDDNode* sinc, Heuristic* inc) const;

  static count_type n_walks;
  static count_type n_cut_walks;

  index_type random_walk(partial_state& s,
			 index_type l,
			 MDDNode* sinc,
			 Heuristic* inc,
			 RNG& rng);

  index_type random_walk(partial_state& s,
			 index_type l,
			 index_set* al_g,
			 index_type n_g,
			 RNG& rng);

  index_type walk(partial_state& s,
		  const index_vec w,
		  MDDNode* sinc,
		  Heuristic* inc);

  void build_explicit_graph(MDDNode* sinc, index_set*& al_g, index_type& n_g);

  // write DOT graph of abstract state space (with inconsistency relation)
  void write_graph(std::ostream& s, const partial_state& g)
    { write_graph(s, g, 0, 0); };
  void write_graph(std::ostream& s,
		   const partial_state& g,
		   MDDNode* sinc,
		   Heuristic* inc);
};


// Pattern Database implementations

// AbstractionHeuristic defines the common interface to all abstraction
// heuristics; it extends the basic SASHeuristic interface by methods to
// extract (optimal) abstract plans, for a given state, a method to compare
// the heuristic against another SASHeuristic, over the set of states in
// the abstraction, and some printing methods. The plan extraction method
// is pure virtual, because implementation differs between progression and
// regression heuristics. For the other methods, default implementations are
// correct, i.e., do what they're supposed to, but the methods are virtual
// to allow subclasses to implement them more efficiently (though at the
// moment there is no such override).
class AbstractionHeuristic : public StateAbstraction, public SASHeuristic
{
 protected:
  const ACF& cost;
  MDDNode* sinc;
  Heuristic* inc;

 public:
  AbstractionHeuristic(SASInstance& i, const index_set& a, const ACF& c)
    : StateAbstraction(i, a), cost(c), sinc(0), inc(0) { };
  AbstractionHeuristic(SASInstance& i,
		       const index_set& a,
		       const ACF& c,
		       MDDNode* s_inc,
		       Heuristic* h_inc)
    : StateAbstraction(i, a), cost(c), sinc(s_inc), inc(h_inc) { };
  virtual ~AbstractionHeuristic() { };

  virtual void abstract_plan
    (const partial_state& s, PlanSet& ps, index_type max) = 0;

  virtual NTYPE max();
  virtual NTYPE mean();
  virtual index_pair compare(SASHeuristic& h);

  // write DOT graph of abstract state space
  void write_graph(std::ostream& s,
		   const partial_state& g,
		   const index_set& e,
		   bool mark_increasing,
		   bool mark_decreasing);

  virtual void write(std::ostream& s);
};

// OpenState and OpenQueue are internal data structures used by the PDB
// computation methods.

struct OpenState {
  partial_state state;
  NTYPE         cost;

  OpenState(const partial_state& s, NTYPE c) :
    state(s), cost(c) { };
  OpenState(const partial_state& s, const index_set& v, NTYPE c) :
    state(s, v), cost(c) { };
  OpenState(const partial_state& s) :
    state(s), cost(0) { };
  OpenState(const OpenState& sc) :
    state(sc.state), cost(sc.cost) { };

  bool operator<(const OpenState& sc) const {
    return (cost > sc.cost);
  };
};

typedef std::priority_queue<OpenState> OpenQueue;

class RegressionPDB : public AbstractionHeuristic {
  PartialStatePerfectHashFunction h;
  NTYPE*       val;
  pair_set*    trace;
  Statistics&  stats;

  index_cost_pair partial_state_val(partial_state& p);
  NTYPE compute_partial_states(partial_state& p, index_set& undef_set);
  void  select_and_sort_relevant_actions(index_vec& acts);

  void abstract_plan(const partial_state& s,
		     index_vec& p,
		     PlanSet& ps,
		     index_type& n,
		     index_type max);

  void abstract_plan_from_trace(index_type i,
				index_vec& p,
				PlanSet& ps,
				index_type& n,
				index_type max);
  void abstract_plan_from_trace(const partial_state& s,
				PlanSet& ps,
				index_type max);

 public:
  RegressionPDB(SASInstance& ins,
		const index_set& v,
		const ACF& c,
		MDDNode* sinc,
		Heuristic* inc,
		Statistics& s);
  virtual ~RegressionPDB();

  // compute PDB by forward exploration (from instance's init_state)
  void compute();

//   // compute PDB using the specifed ACF, atom inconsistency relation and
//   // atom precedence graph
//   void compute_with_precedence(const graph& pg);

  virtual NTYPE eval(const partial_state& s);
  virtual void write_eval(const partial_state& s,
			  std::ostream& t,
			  char* p = 0,
			  bool e = true);

  virtual void abstract_plan
    (const partial_state& s, PlanSet& ps, index_type max);
  virtual void write(std::ostream& s);

  void enable_trace();
  void check_trace();
};


class ProgressionPDB : public AbstractionHeuristic {
  CompleteStatePerfectHashFunction h;
  NTYPE*       val;

  // computation state (used for incremental PDB computation)
  OpenQueue    open;
  index_vec    acts;

  Statistics&  stats;

  void abstract_plan(const partial_state& s,
		     index_vec& p,
		     PlanSet& ps,
		     index_type& n,
		     index_type max);

  // single step of the exploration loop
  void expand_next_open();

 public:
  // structures used by the new & improved PDB compuation
  struct Test {
    index_type value;
    index_type divisor;
    index_type modulus;
    Test() :
      value(0), divisor(0), modulus(0) { };
    Test(index_type v, index_type d, index_type m) :
      value(v), divisor(d), modulus(m) { };
  };

  class MatchTree {
    struct Branch {
      index_type divisor ALIGN;
      index_type modulus;
      MatchTree* children;
    };
    std::vector<Branch> branches ALIGN;
    lvector<int>        operator_diffs;
   public:
    MatchTree();
    ~MatchTree();
    void traverse(NTYPE* db,
		  const bool_vec& valid,
		  index_type*& queue_write,
		  index_type index) const;
    void insert(const lvector<Test>& tests,
		int op_diff,
		index_type i);
    void write(std::ostream& s);
  };

 private:
  void initialize2(MDDNode* sinc,
		   Heuristic* inc,
		   bool_vec& valid,
		   index_type*& queue_write);
  MatchTree compute_match_tree();

 public:
  ProgressionPDB(SASInstance& ins,
		 const index_set& v,
		 const ACF& c,
		 MDDNode* sinc,
		 Heuristic* inc,
		 Statistics& s);
  virtual ~ProgressionPDB();

  // compute PDB by backward exploration (from instance's goal_state),
  void compute();

  // methods for computing the PDB partially: initialize must be called
  // first; compute_to(s) computes the PDB until state s has a (finite)
  // value; compute_rest continues until all states have values.
  void initialize();
  void compute_to(const partial_state& s);
  void compute_rest();

  // new and improved PDB computation
  void compute2();

  virtual NTYPE eval(const partial_state& s);
  virtual NTYPE max();
  virtual NTYPE mean();

  virtual void abstract_plan
    (const partial_state& s, PlanSet& ps, index_type max);
  virtual void write(std::ostream& s);
};

typedef MonadicFunction<index_set,index_type> SetSizeFunction;

class RegressionPDBSize : public SetSizeFunction {
  const index_vec& signature;
 public:
  static index_type apply(const index_vec& sig, const index_set& set);

  RegressionPDBSize(const index_vec& s) : signature(s) { };
  virtual ~RegressionPDBSize() { };
  virtual index_type operator()(const index_set& set) const;
};

class ProgressionPDBSize : public SetSizeFunction {
  const index_vec& signature;
 public:
  static index_type apply(const index_vec& sig, const index_set& set);

  ProgressionPDBSize(const index_vec& s) : signature(s) { };
  virtual ~ProgressionPDBSize() { };
  virtual index_type operator()(const index_set& set) const;
};

struct HValue {
  NTYPE val;
  bool  opt;
  HValue() : val(0), opt(false) { };
  HValue(NTYPE v, bool o) : val(v), opt(o) { };
  HValue(NTYPE v) : val(v), opt(false) { };

  HValue& operator=(const HValue& v)
  { val = v.val; opt = v.opt; return *this; };
  HValue& operator=(const NTYPE& v)
  { val = v; opt = false; return *this; };
  bool operator==(const HValue& v) const
  { return ((v.val == val) && (v.opt == opt)); };
  bool operator!=(const HValue& v) const
  { return ((v.val != val) || (v.opt != opt)); };
  bool operator<(const HValue& v) const
  { return ((v.val < val) || ((v.val == val) && v.opt && !opt)); };
  bool operator>(const HValue& v) const
  { return ((v.val > val) || ((v.val == val) && !v.opt && opt)); };
};

typedef __gnu_cxx::hash_map<partial_state,
			    HValue,
			    StateHashFunction,
			    std::equal_to<partial_state> >
partial_state_map;

class ASHProgState : public SASSeqProgState {
 protected:
  StateAbstraction&  sa;
  const index_set_vec& sa_acts;
  MDDNode*           sinc;
  Heuristic*         inc;
  partial_state_map& ash_store;
  bool               sflag;

  // successor constructor
  ASHProgState(ASHProgState& p, SASInstance::Action& a);

 public:
  static bool cut_at_solved;

  ASHProgState(SASInstance& i,
	       StateAbstraction& a,
	       const index_set_vec& aa,
	       const ACF& c,
	       SASHeuristic& h,
	       MDDNode* shi,
	       Heuristic* hi,
	       const partial_state& s0,
	       partial_state_map& ash);
  ASHProgState(const ASHProgState& s);
  virtual ~ASHProgState() { };

  virtual NTYPE delta_cost();
  virtual NTYPE est_cost();
  virtual bool is_final();
  virtual NTYPE expand(Search& s, NTYPE bound);
  virtual void store(NTYPE cost, bool opt);
  virtual void reevaluate();
  virtual State* copy();
};

class ProgressionASH : public AbstractionHeuristic {
  StateHashFunction abs_shf;
  partial_state_map val;
  SASHeuristic&     h_search;
  index_set_vec     sa_acts;
  Statistics&       stats;

  HValue solveIDA(const partial_state& s, NTYPE b, ActionSequenceSet* p);
  HValue solveBB(const partial_state& s, NTYPE b);
  HValue solveBFS(const partial_state& s, NTYPE b);

 public:
  ProgressionASH(SASInstance& ins,
		 const index_set& v,
		 const ACF& c,
		 MDDNode* sinc,
		 Heuristic* inc,
		 SASHeuristic& h,
		 Statistics& s);
  virtual ~ProgressionASH();

  virtual NTYPE eval(const partial_state& s);
  virtual NTYPE eval_to_bound(const partial_state& s, NTYPE b);
  virtual void abstract_plan
    (const partial_state& s, PlanSet& ps, index_type max);
  virtual void write(std::ostream& s);
};

typedef lvector<AbstractionHeuristic*> abstraction_heuristic_vec;

class PDBCollection
: public SASHeuristic, public abstraction_heuristic_vec
{
  SASInstance& instance;
  const ACF&   cost;
  MDDNode*     sinc;
  Heuristic*   inc;

  Statistics&  stats;
  bool_vec own;

  graph g_add; // additivity relation over patterns in collection
  graph g_subset; // (non-strict) subset relation over patterns in collection

  index_set_vec canonical_h;
  bool update_flag;

  cost_vec values;

  bool sum_is_dominated(const index_set &dominatee,
			const index_set &dominator) const;

 public:
  PDBCollection(SASInstance& i,
		const ACF& c,
		MDDNode* shi,
		Heuristic* hi,
		Statistics& s);
  PDBCollection(PDBCollection& c);
  ~PDBCollection();

  AbstractionHeuristic* find_PDB_with_pattern(const index_set& set);
  index_type n_patterns();
  index_type total_size(); // in number of PDB entries

  void addPDB(AbstractionHeuristic* h, bool o);
  void addProgressionPDB(const index_set& set, bool opt_fast);

  void compute_canonical_heuristic();

  virtual NTYPE mean();
  virtual NTYPE eval(const partial_state& s);
  virtual NTYPE eval_to_bound(const partial_state& s, NTYPE b);

  void write_collection(std::ostream& s);
  void write_PDB(std::ostream& s);
};

PDBCollection* build_collection
(Instance& ins,
 const ACF& cost,
 SASInstance& s_ins,
 const ACF& s_cost,
 MDDNode* sinc,
 Heuristic* inc,
 const index_set& vars,
 index_type pdb_size_limit,
 index_type total_size_limit,
 index_type d_skip,
 index_type s_max,
 double     i_min,
 index_type n_trials,
 index_type n_samples,
 bool       opt_fast,
 double&     i_out,
 index_type& s_out,
 RNG& rng,
 Statistics& s);


inline std::ostream& operator<<(::std::ostream& s, const ProgressionPDB::Test& t)
{
  s << '<' << t.value << ',' << t.divisor << ',' << t.modulus << '>';
}

END_HSPS_NAMESPACE

#endif
