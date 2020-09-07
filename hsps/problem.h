#ifndef PROBLEM_H
#define PROBLEM_H

#include "config.h"
#include "index_type.h"
#include "numeric_type.h"
#include "name.h"
#include "graph.h"

BEGIN_HSPS_NAMESPACE

class Heuristic;

struct rule {
  index_set  antecedent;
  index_type consequent;

  rule() : antecedent(EMPTYSET), consequent(no_such_index) { };
  rule(index_type c) : antecedent(EMPTYSET), consequent(c) { };
  rule(const index_set& a, index_type c) : antecedent(a), consequent(c) { };
  rule(const rule& r) : antecedent(r.antecedent), consequent(r.consequent) { };

  rule& operator=(const rule& r) {
    antecedent = r.antecedent;
    consequent = r.consequent;
    return *this;
  };

  bool operator==(const rule& r) const {
    return ((antecedent == r.antecedent) && (consequent == r.consequent));
  };

  bool operator!=(const rule& r) const {
    return (!(*this == r));
  };

  bool operator<(const rule& r) const {
    return ((consequent < r.consequent) ||
	    ((consequent == r.consequent) && (antecedent < r.antecedent)));
  };

  bool operator>(const rule& r) const {
    return ((consequent > r.consequent) ||
	    ((consequent == r.consequent) && (antecedent > r.antecedent)));
  };
};

class rule_set : public svector<rule> {
 public:
  index_type find_rule(index_type c) const;

  void compute_dependency_graph(index_type n, index_graph& g) const;

  void remove(const bool_vec& set, index_vec& map);
  void remove(const bool_vec& set, index_graph& g);
  void remove(const bool_vec& set);

  void make_acyclic(index_graph& g);
  void make_post_unique(index_graph& g);
};

typedef svector<const char*> string_set;

class Instance {
  bool xrf; // x-reference flag

 public:
  static bool write_negation;
  static bool write_DKEL;
  static bool write_PDDL2;
  static bool write_time;
  static bool write_PDDL3;
  static bool write_metric;
  static bool write_extra;
  static bool write_resource_constraints_at_start;
  static bool always_write_parameters;
  static bool always_write_requirements;
  static bool always_write_precondition;
  static bool always_write_effect;
  static bool always_write_conjunction;

  static bool write_atom_set_with_symbolic_names;
  static bool write_action_set_with_symbolic_names;

  static const char* goal_atom_name;
  static const char* goal_action_name;
  static NTYPE       goal_action_cost;
  static const char* pc_name;
  static index_type  pc_count;

  struct Atom {
    const Name* name;
    index_type index;
    index_type neg;
    bool       init;
    NTYPE      init_t;
    bool       goal;
    NTYPE      goal_t;
#ifdef SUPPORT_VOLATILE_ATOMS
    bool       volatile;
#endif
    bool       irrelevant;
    void*      src;
    // secondary information (available if cross-ref'd)
    index_vec  req_by;
    index_vec  add_by;
    index_vec  del_by;
    Atom() : name(0), index(no_such_index), neg(no_such_index),
	 init(false), init_t(0), goal(false), goal_t(POS_INF),
#ifdef SUPPORT_VOLATILE_ATOMS
	 volatile(false),
#endif
	 irrelevant(false), src(0), req_by(no_such_index, 0),
	 add_by(no_such_index, 0), del_by(no_such_index, 0) { };
    Atom(const Name* n) : name(n), index(0), neg(no_such_index),
	 init(false), init_t(0), goal(false), goal_t(POS_INF),
#ifdef SUPPORT_VOLATILE_ATOMS
	 volatile(false),
#endif
	 irrelevant(false), src(0), req_by(no_such_index, 0),
	 add_by(no_such_index, 0), del_by(no_such_index, 0) { };
    Atom(const Name* n, index_type i) : name(n), index(i),
	 neg(no_such_index), init(false), init_t(0), goal(false),
	 goal_t(POS_INF),
#ifdef SUPPORT_VOLATILE_ATOMS
	 volatile(false),
#endif
	 irrelevant(false), req_by(no_such_index, 0),
	 add_by(no_such_index, 0), del_by(no_such_index, 0) { };

    Atom& operator=(const Atom& a) {
      name = a.name;
      index = a.index;
      neg = a.neg;
      init = a.init;
      init_t = a.init_t;
      goal = a.goal;
      goal_t = a.goal_t;
#ifdef SUPPORT_VOLATILE_ATOMS
      volatile = a.volatile;
#endif
      irrelevant = a.irrelevant;
      src = a.src;
      req_by = a.req_by;
      add_by = a.add_by;
      del_by = a.del_by;
      return *this;
    };

    bool operator==(const Atom& a) {
      return (index == a.index);
    };
  };

  struct Resource {
    const Name* name;
    index_type  index;
    NTYPE       init;
    void*       src;
    // secondary information (available if cross-ref'd)
    bool        consumed;
    bool        used;
    Resource() : name(0), index(no_such_index), init(0), src(0),
	 consumed(false), used(false) { };
    Resource(const Name* n) : name(n), index(no_such_index), init(0), src(0),
	 consumed(false), used(false) { };
    Resource(const Name* n, index_type i) : name(n), index(i), init(0),
	 src(0), consumed(false), used(false) { };

    Resource& operator=(const Resource& r) {
      name = r.name;
      index = r.index;
      init = r.init;
      src = r.src;
      consumed = r.consumed;
      used = r.used;
      return *this;
    };

    bool operator==(const Resource& r) {
      return (index == r.index);
    };

    bool reusable() const { return (used && !consumed); };
    bool consumable() const { return (consumed); };
  };

  struct Action {
    const Name* name;
    index_type index;
    bool       sel; // selectable
    index_set  pre;
    index_set  add;
    index_set  del;
    index_set  lck;
    amt_vec    use;
    amt_vec    cons;
    NTYPE      dur;
    NTYPE      dmin;
    NTYPE      dmax;
    NTYPE      cost;
    const char* assoc;
    void*      src;
    // secondary information (available if computed; see preprocessor)
    index_set  ncw_atms; // atoms whose establishers are not compatible with
                         // this action
    Action()
      : name(0), index(no_such_index), sel(true), pre(), add(), del(),
        lck(), use(ZERO, 0), cons(ZERO, 0), dur(1), dmin(1), dmax(1), cost(1),
	 assoc(0), src(0), ncw_atms() { };
    Action(const Name* n)
      : name(n), index(no_such_index), sel(true), pre(), add(), del(),
        lck(), use(ZERO, 0), cons(ZERO, 0), dur(1), dmin(1), dmax(1), cost(1),
	assoc(0), src(0), ncw_atms() { };

    Action& operator=(const Action& a) {
      name = a.name;
      index = a.index;
      sel = a.sel;
      pre = a.pre;
      add = a.add;
      del = a.del;
      lck = a.lck;
      use = a.use;
      cons = a.cons;
      dur = a.dur;
      dmin = a.dmin;
      dmax = a.dmax;
      cost = a.cost;
      assoc = a.assoc;
      src = a.src;
      ncw_atms = a.ncw_atms;
      return *this;
    };

    bool operator==(const Action& a) {
      return (index == a.index);
    };

    NTYPE req(index_type r) const { return use[r] + cons[r]; };

    bool e_deletes(index_type p, Heuristic* inc) const;
    bool e_deletes(const index_set& s, Heuristic* inc) const;
  };

  struct Constraint {
    const Name* name;
    index_type index;
    index_set  set;
    index_type lim;
    bool       exact;
    void*      src;
    bool       verified;
    Constraint() : name(0), index(no_such_index), set(), lim(0),
	 exact(false), src(0), verified(false) { };
    Constraint(const Name* n) : name(n), index(no_such_index),
	 set(), lim(0), exact(false), src(0), verified(false) { };
    Constraint(index_set s, index_type n, bool e) : name(0),
	 index(no_such_index), set(s), lim(n), exact(e), src(0),
	 verified(false) { };

    Constraint& operator=(const Constraint& c) {
      name = c.name;
      index = c.index;
      set = c.set;
      lim = c.lim;
      exact = c.exact;
      src = c.src;
      verified = c.verified;
      return *this;
    };

    bool operator==(const Constraint& c) {
      return ((set == c.set) && (lim == c.lim) && (exact == c.exact));
    };
  };

  typedef lvector<Atom> atom_vec;
  typedef lvector<Resource> resource_vec;
  typedef lvector<Action> action_vec;
  typedef lvector<Constraint> constraint_vec;

  typedef atom_vec::element_reference atom_ref;
  typedef resource_vec::element_reference resource_ref;
  typedef action_vec::element_reference action_ref;
  typedef constraint_vec::element_reference constraint_ref;

  const Name*  name;
  atom_vec     atoms;
  action_vec   actions;
  resource_vec resources;
  constraint_vec invariants;

  // secondary information (available if instance cross-ref'd)
  index_set    init_atoms;
  index_set    goal_atoms;
  index_type   max_pre, max_add, max_del, max_lck,
               max_add_by, max_del_by, max_req_by;
  NTYPE        min_dur, max_dur, min_cost, max_cost;

  set_hash_function atom_set_hash;
  set_hash_function action_set_hash;

  static int   default_trace_level;
  int          trace_level;

  Instance();
  Instance(const Name* n);
  Instance(const Instance& ins);
  ~Instance() { };

  // build (add to) instance
  Atom&      new_atom(const Name* name);
  Resource&  new_resource(const Name* name);
  Action&    new_action(const Name* name);
  Action&    copy_action(index_type a);
  Constraint& new_invariant();
  Constraint& new_invariant(const Name* name);
  Constraint& new_invariant(const index_set& s, index_type l, bool e);

  // make this instance a copy of ins
  void copy(const Instance& ins);

  // return a copy of this instance
  Instance* copy() const;

  // clear this instance (remove all atoms/actions/resources/etc)
  void clear();

  // build restricted copy: the set of atoms and resources to include
  // (atms, rc) are input, the set of actions in the restricted instance
  // (acts) is infered (map maps atoms in original instance to restricted).
  void restricted_copy(const Instance& ins, const index_set& atms,
		       const index_set& rc, index_set& acts, index_vec& map);

  // build restricted copy: the set of actions to include (acts) is
  // input, the set of atoms in the restricted instance is infered
  // (map maps atoms in original instance to restricted). Resources
  // in the original instance are ignored (N.Y.I.).
  void restricted_copy(const Instance& ins, const index_set& acts,
		       index_vec& map);

  // build abstracted copy: the set of atoms to keep (atms) is input; in
  // difference to restricted_copy, abstraction merges actions that become
  // equivalent under the new restricted atom set. the atom and action maps
  // are output and map from the original instance to the abstracted
  // instance. resources in the original instance are ignored (N.Y.I.).
  void abstracted_copy(const Instance& ins, const index_set& atms,
		       index_vec& atm_map, index_vec& act_map);

  // build reversed copy (resources in the original instance are ignored)
  void reverse_copy(const Instance& ins);

  void delete_relax(const index_set& x_atms);
  void delete_relax_less(const index_set& x_atms);

  void assign_unique_action_names();

  // change (remove from) instance
  void remove_actions(const bool_vec& set, index_vec& map);
  void remove_atoms(const bool_vec& set, index_vec& map);
  void remove_invariants(const bool_vec& set, index_vec& map);
  void remap_set(index_set& set, const index_vec& map);
  void remap_sets(index_set_vec& sets, const index_vec& map);

  // change (set/add to) instance
  void set_initial(const index_set& init);
  void set_goal(const index_set& goal);
  void set_DNF_goal(const index_set_vec& goal);
  void replace_atom_by_conjunction(index_type p, const index_set& c);
  void set_cost_bound(NTYPE b);
  void create_composite_resource(const index_set& set);
  void create_total_resource();

  // extract atom negation info from binary exact invariants
  void extract_atom_negations_from_invariants();

  // ensure one/all atoms have negations (creating new atoms if needed)
  index_type complete_atom_negation(index_type a);
  void complete_atom_negations(const index_set& s);
  void complete_atom_negations();

  index_type create_history_atom(index_type a);

  // create invariants from atom negation info
  void add_all_negation_invariants();
  void add_missing_negation_invariants();

  // compile plan constraints
  index_type compile_pc_always(const index_set& f, const Name* n);
  index_type compile_pc_sometime(const index_set& f, const Name* n);
  index_type compile_pc_at_most_once(const index_set& f, const Name* n);
  index_type compile_pc_sometime_before(const index_set& f_t,
					const index_set& f_c,
					const Name* n);

  // enforce plan constraints
  void enforce_pc_always(const index_set& f, const Name* n);
  void enforce_pc_sometime(const index_set& f, const Name* n);
  void enforce_pc_at_most_once(const index_set& f, const Name* n);
  void enforce_pc_sometime_before(const index_set& f_t,
				  const index_set& f_c,
				  const Name* n);

  // inference stuff
  void compute_iff_axioms(rule_set& ax);

  // compute/clear secondary instance info
  void cross_reference();
  void clear_cross_reference();
  bool cross_referenced() const;

  // note: verifying invariants changes instance (verified flag)
  bool verify_invariant(Constraint& inv);
  void verify_invariants();

  // save/set/exchange durations
  void save_durations(cost_vec& out) const;
  void set_durations(const cost_vec& in);
  void set_durations(const cost_vec& in, cost_vec& out);

  // change durations/cost/resources
  void assign_unit_durations(NTYPE unit = I_TO_N(1));
  void discretize_durations(NTYPE interval_width);
  void quantize_durations(index_type n_intervals);
  void round_durations_up();
  void round_durations_down();
  void round_durations();

  void assign_unit_costs(cost_vec& save);
  void restore_costs(const cost_vec& saved);

  void assign_unlimited_resources(cost_vec& save);
  void restore_resources(const cost_vec& saved);

  // access instance information
  index_type n_atoms() const { return atoms.length(); };
  index_type n_resources() const { return resources.length(); };
  index_type n_reusable_resources() const;
  index_type n_consumable_resources() const;
  index_type n_actions() const { return actions.length(); };
  index_type n_invariants() const { return invariants.length(); };
  index_type n_verified_invariants() const;

  void atom_names(name_vec& names) const;
  void action_names(name_vec& names) const;

  // note: causal graph is "dependency directed" and does not have edges
  // from deleted atoms to precondition atoms (reasonable, since there
  // are no negative preconditions)
  void cochange_graph(graph& g) const;
  void causal_graph(graph& g) const;
  void partitioning_graph(const index_set& goal,
			  index_set_graph& g,
			  index_set& n_goal) const;

  // construct a "Petri net-style" graph representation of the instance;
  // nn contains node labels (atom and action names)
  void make_graph_representation(index_graph& g, name_vec& nn);

  // actions a0,a1 are non-interfering iff neither deletes a precondition
  // or add-effect of the other
  bool non_interfering(index_type a0, index_type a1) const;

  // actions a0,a1 are lock-compatible iff neither locks, deletes or requires
  // (as a precondition) an atom locked by the other
  bool lock_compatible(index_type a0, index_type a1) const;

  // actions a0,a1 are resource-compatible iff for no resource their combined
  // USE exceeds the amount available (i.e., the resource init value)
  bool resource_compatible(index_type a0, index_type a1) const;

  // actions a0,a1 are commutative if neither adds or deletes a precondition
  // or add-effect of the other
  bool commutative(const Action& a0, const Action& a1) const;
  bool commutative(index_type a0, index_type a1) const;

  // atoms p0,p1 are additive iff no action adds both atoms
  bool additive(index_type p0, index_type p1) const;

  // atoms p0,p1 are cochanged iff some action affects (adds or deletes) both
  // atoms (<-> iff exists an edge between p0 and p1 in the cochange graph)
  bool cochanged(index_type p0, index_type p1) const;

  bool eval_invariant_in_partial_state(const index_set& s,
				       const Constraint& inv);
  bool eval_invariant_in_partial_state(const bool_vec& s,
				       const Constraint& inv);
  bool eval_invariant_in_complete_state(const index_set& s,
					const Constraint& inv);
  bool eval_invariant_in_complete_state(const bool_vec& s,
					const Constraint& inv);

  void negation_atom_set(const index_set& pset, index_set& nset) const;

  // write utilities
  void write_atom_set(::std::ostream& s,
		      const index_vec& set,
		      unsigned int c = Name::NC_DEFAULT) const;
  void write_atom_set(::std::ostream& s,
		      const bool_vec& set,
		      unsigned int c = Name::NC_DEFAULT) const;
  void write_atom_sets(::std::ostream& s,
		       const index_set_vec& sets,
		       unsigned int c = Name::NC_DEFAULT) const;
  void write_action_set(::std::ostream& s,
			const index_vec& set,
			unsigned int c = Name::NC_DEFAULT) const;
  void write_action_set(::std::ostream& s,
			const bool_vec& set,
			unsigned int c = Name::NC_DEFAULT) const;
  void write_iff_axiom(::std::ostream& s, const rule& r) const;
  void write_iff_axiom_set(::std::ostream& s, const rule_set& rset) const;

  void write_atom_digraph(::std::ostream& s,
			  const graph& g,
			  const index_set& atomset,
			  const bool_vec& mark_shaded,
			  const bool_vec& mark_dashed,
			  const char* label) const;
  void write_atom_digraph(::std::ostream& s,
			  const graph& g,
			  const char* label) const;
  void write_atom_action_digraph(::std::ostream& s,
				 const graph& g,
				 const index_set& atomset,
				 const index_set& actionset,
				 const bool_vec& mark_shaded,
				 const bool_vec& mark_bold,
				 const bool_vec& mark_dashed,
				 const char* label) const;
  void write_atom_set_digraph(::std::ostream& s,
			      const index_set_graph& g,
			      const char* label) const;
  void write_atom_set_graph(::std::ostream& s,
			    const index_set_graph& g,
			    const char* label) const;
  void write_axiom_dependency_graph(::std::ostream& s,
				    const index_graph& g,
				    const char* label) const;

  virtual void write_PDDL_action
    (::std::ostream& s, const Action& act) const;
  virtual void write_DKEL_invariant_item
    (::std::ostream& s, const Constraint& inv, string_set& tags) const;
  virtual void write_DKEL_irrelevant_atom_item
    (::std::ostream& s, const Atom& atm, string_set& tags) const;
  virtual void write_DKEL_irrelevant_action_item
    (::std::ostream& s, const Action& act, string_set& tags) const;
  virtual void write_domain_atom_set
    (::std::ostream& s, const index_set& set) const;
  virtual void write_domain_action_set
    (::std::ostream& s, const index_set& set) const;
  virtual void write_domain_action_set
    (::std::ostream& s, const index_set& set, const Name* name) const;

  virtual void write_domain(::std::ostream& s) const;
  virtual void write_domain_init(::std::ostream& s) const;
  virtual void write_domain_declarations(::std::ostream& s) const;
  virtual void write_domain_actions(::std::ostream& s) const;
  virtual void write_domain_DKEL_items(::std::ostream& s) const;
  virtual void write_problem(::std::ostream& s) const;
  virtual void write_problem_init(::std::ostream& s) const;
  virtual void write_problem_goal(::std::ostream& s) const;
  virtual void write_problem_metric(::std::ostream& s) const;

  void print_atom(::std::ostream& s, const Atom& a) const;
  void print_resource(::std::ostream& s, const Resource& r) const;
  void print_action(::std::ostream& s, const Action& a) const;
  void print_invariant(::std::ostream& s, const Constraint& c) const;
  virtual void print(::std::ostream& s) const;

 private:
  // create atom representing negation of atom a; note that this method
  // does not clear cross-reference information, which is no longer valid
  void create_atom_negation(index_type a);
};

typedef lvector<Instance*> instance_vec;

class PreconditionEvaluator {
  enum eval_node_type { positive_leaf,
			undecided_leaf,
			no_test,
			test_invariant,
			test_atom };
  Instance& instance;
  eval_node_type node_type;
  index_type i_test;
  lvector<PreconditionEvaluator*> next;
  PreconditionEvaluator* prev;
  index_type n_positive;
  index_set acts;

  static void construct(Instance& ins,
			PreconditionEvaluator* p,
			bool_vec& s,
			bool_vec& ua,
			index_type n_ua,
			index_type n_pos,
			bool_vec& rem_invs,
			bool_vec& rem_atoms,
			NTYPE T);

 public:
  PreconditionEvaluator(Instance& ins);
  ~PreconditionEvaluator();

  static PreconditionEvaluator* construct(Instance& ins, NTYPE T);

  PreconditionEvaluator* node(const bool_vec& s);
  index_type eval(const bool_vec& s, // in: state
		  const bool_vec& a, // in: allowed actions
		  index_type* app,   // out: indices of applicable actions
		  index_type c);     // in/out: #in app
  void write_graph(std::ostream& s, bool root = true);
};


inline ::std::ostream& operator<<(::std::ostream& s, const rule& r)
{
  return s << r.antecedent << "->" << r.consequent;
};

inline ::std::ostream& operator<<(::std::ostream& s, const rule_set& r)
{
  s << '{';
  for (index_type k = 0; k < r.length(); k++) {
    if (k > 0) s << ',';
    s << r[k];
  }
  s << '}';
  return s;
};

END_HSPS_NAMESPACE

#endif
