
#ifndef PDDL_BASE_H
#define PDDL_BASE_H

#include "config.h"
#include "index_type.h"
#include "ptr_table.h"
#include "string_table.h"
#include "problem.h"
#include "search.h"
#include "heuristic.h"
#include "htn.h"
#include "soft.h"

BEGIN_HSPS_NAMESPACE

class PDDL_Name;

class PDDL_Base {
 public:
  static bool     use_default_function_value;
  static NTYPE    default_function_value;
  static bool     use_strict_borrow_definition;
  static bool     use_extended_borrow_definition;
  static bool     compact_resource_effects;
  static bool     compile_away_disjunctive_preconditions;
  static bool     check_precondition_consistency;
  static bool     compile_away_conditional_effects;
  static bool     compile_away_plan_constraints;
  static bool     compile_away_object_functions;
  static bool     compile_for_validator;
  static bool     create_all_atoms;
  static bool     create_all_actions;
  static bool     number_multiple_action_instances;
  static bool     exclude_all_dkel_items;
  static string_set excluded_dkel_tags;
  static string_set required_dkel_tags;
  static bool     strict_set_export;
  static bool     best_effort;
  static bool     write_PDDL31;
  static bool     write_warnings;
  static bool     write_info;
  static bool     write_trace;
  static bool     trace_print_context;
  static bool     name_instance_by_problem_file;
  static const char* instance_name_prefix;

  enum symbol_class {
    sym_object,
    sym_typename,
    sym_predicate,
    sym_object_function,
    sym_function,
    sym_action,
    sym_variable,
    sym_misc,
    sym_preference,
    sym_set,
    sym_meta_variable
  };

  enum metric_class {
    metric_none,
    metric_makespan,
    metric_minimize,
    metric_maximize
  };

  enum mode_keyword {
    md_none,
    md_start,
    md_end,
    md_all,
    md_init,
    md_pos_goal,
    md_neg_goal
  };

  enum partial_value { p_false, p_true, p_unknown };

  enum expression_class {
    exp_fun,
    exp_list,
    exp_const,
    exp_add,
    exp_sub,
    exp_mul,
    exp_div,
    exp_time,
    exp_preference
  };

  enum relation_type {
    rel_equal,
    rel_greater,
    rel_greater_equal,
    rel_less,
    rel_less_equal
  };

  enum formula_class { fc_false,
		       fc_true,
		       fc_atom,
		       fc_equality,
		       fc_negation,
		       fc_conjunction,
		       fc_disjunction,
		       fc_equivalence,
		       fc_implication,
		       fc_universal,
		       fc_existential,
		       fc_list
  };

  struct Symbol;
  struct TypeSymbol;
  struct Atom;
  struct FTerm;
  struct FChangeAtom;
  struct SetOf;
  struct ListExpression;
  struct ActionSymbol;
  struct Reference;
  struct IrrelevantItem;

  typedef lvector<TypeSymbol*> type_vec;
  typedef lvector<Atom*> atom_vec;
  typedef lvector<FChangeAtom*> ch_atom_vec;
  typedef lvector<SetOf*> atom_set_vec;
  typedef lvector<Reference*> ref_vec;
  typedef lvector<IrrelevantItem*> irrelevant_vec;

  struct TypeSet : public type_vec {
    TypeSet() : type_vec(0, 0) { };
    TypeSet(const TypeSet& s) : type_vec(s) { };
    TypeSet(TypeSymbol* t) : type_vec(t, 1) { };

    index_type n_elements() const;
    Symbol*    get_element(index_type n);

    bool is_object() const;
    bool subtype_or_equal(const TypeSet& s) const;
    bool subtype_or_equal(TypeSymbol* t) const;

    void print(std::ostream& s) const;
    void write_type(std::ostream& s) const;
  };

  struct Symbol {
    symbol_class sym_class;
    char*        print_name;
    TypeSet      sym_types;
    bool         defined_in_problem;
    bool         visible;
    Symbol(symbol_class c, char* n)
      : sym_class(c), print_name(n), defined_in_problem(false), visible(true) { };
    Symbol(char* n)
      : sym_class(sym_object), print_name(n), defined_in_problem(false), visible(true) { };
    void print(std::ostream& s) const;
  };


  typedef zero_init_pair<Symbol*> symbol_pair;
  typedef lvector<Symbol*> symbol_vec;
  typedef svector<Symbol*> symbol_set;
  typedef lvector<symbol_pair> symbol_pair_vec;
  static bool extend_substitution(Symbol* out, Symbol* in, symbol_pair_vec& u);
  static bool print_substitution(std::ostream& s, const symbol_pair_vec& u);
  static bool print_inequality(std::ostream& s, const symbol_pair_vec& neq);
  static bool substitution_violates_inequality(const symbol_pair_vec& neq,
					       const symbol_pair_vec& u);

  static index_type find_matching_atom(Atom* a, atom_vec& v);
  static index_type find_matching_atom(Atom* a, mode_keyword m, atom_vec& v);
  static index_type find_matching_fluent_atom
    (FChangeAtom* a, mode_keyword m, ch_atom_vec& v);


  struct TypeSymbol : public Symbol {
    symbol_vec  elements;
    bool        is_base_type;
    TypeSymbol(char* n) : Symbol(sym_typename, n), elements(0, 0),
	 is_base_type(false) { };
    void add_element(Symbol* e);
    bool is_object() const { return (strcmp(print_name, "object") == 0); };
    bool subtype_or_equal(TypeSymbol* t) const;
    bool subtype_or_equal(const TypeSet& t) const;
    void print(std::ostream& s) const;
  };

  struct VariableSymbol : public Symbol {
    Symbol*  value;
    FTerm*   binding;
    VariableSymbol(char* n) : Symbol(sym_variable, n), value(0), binding(0) {};
    // return true iff types permit equality between this variable and s
    bool equality_type_check(Symbol* s);
    void print(std::ostream& s);
  };

  typedef lvector<VariableSymbol*> variable_vec;

  struct ParamSymbol : public Symbol {
    variable_vec param;
    ParamSymbol(symbol_class c, char* n) : Symbol(c, n), param(0, 0) { };
  };

  struct PredicateSymbol : public ParamSymbol {
    bool         pos_pre;
    bool         neg_pre;
    bool         added;
    bool         deleted;
    bool         locked;
    bool         modded;
    ptr_table    init;
    ptr_table    pos_goal;
    ptr_table    neg_goal;
    ptr_table    pos_prop;
    ptr_table    neg_prop;
    irrelevant_vec irr_ins;
    PredicateSymbol(char* n) : ParamSymbol(sym_predicate, n),
	 pos_pre(false), neg_pre(false), added(false), deleted(false),
	 locked(false),	modded(false), irr_ins(0, 0) { };
    bool is_static() const { return !modded; };
    bool is_equality() const { return (strcmp(print_name, "=") == 0); };
    // note: PredicateSymbol::instantiate is called only if create_all_atoms
    void instantiate(Instance& ins);
    void initialise_missing(const symbol_vec& p,
			    atom_vec* created,
			    index_type i = 0);
    void write_prototype(std::ostream& s);
    void print(std::ostream& s);
  };

  struct ObjectFunctionSymbol : public ParamSymbol {
    bool  modded; // true iff any action effect modifies the function
    ptr_table  init;
    ObjectFunctionSymbol(char* n)
      : ParamSymbol(sym_object_function, n), modded(false) { };
    bool is_static() const { return !modded; };
    void print(std::ostream& s);
  };

  struct FunctionSymbol : public ParamSymbol {
    bool  modified; // true iff any action effect modifies the function
    bool  integral; // true iff all init values and effects are integers
    bool  linear; // true iff all effects are inc./dec. by constant
    bool  increased;
    bool  decreased;
    bool  assigned;
    bool  borrowed;
    ptr_table  init;
    FunctionSymbol(char* n) : ParamSymbol(sym_function, n),
	 modified(false), integral(true), linear(true), increased(false),
	 decreased(false), assigned(false), borrowed(false) { };
    bool is_static() const
    { return !modified; };
    bool is_reusable() const
    { return borrowed && !increased && !decreased && !assigned; };
    bool is_consumable() const
    { return !borrowed && !increased && decreased && !assigned; };
    interval eval_init_bounds(ptr_table* p, index_type i, ListExpression* r);
    void print(std::ostream& s);
  };

  struct AtomBase {
    symbol_vec   param;
    mode_keyword at;
    NTYPE        at_time;
    AtomBase() : param(0, 0), at(md_none), at_time(0) { };
    AtomBase(mode_keyword t) : param(0, 0), at(t), at_time(0) { };
    AtomBase(AtomBase* b);
    bool equals(AtomBase& b);
    void free_variables(variable_vec& v);
    bool occurs(Symbol* s);
    void fill_in_args(AtomBase* b);
    void collect_bound_variables(variable_vec& v);
    void insert(ptr_table& t);

    static bool print_bindings;
    void print(std::ostream& s) const;
    void print_instance(std::ostream& s);
  };

  typedef lvector<AtomBase*> atom_base_vec;

  struct Atom : AtomBase {
    PredicateSymbol* pred;
    Atom(PredicateSymbol* p) : pred(p) { };
    Atom(PredicateSymbol* p, mode_keyword t) : AtomBase(t), pred(p) { };
    Atom(PredicateSymbol* p, variable_vec& a, bool as_value)
      : pred(p)
    {
      param.set_length(a.length());
      for (index_type k = 0; k < a.length(); k++)
	param[k] = (as_value ? a[k]->value : a[k]);
    };
    Atom(PredicateSymbol* p, Symbol* a0)
      : pred(p)
    {
      param.append(a0);
    };
    Atom(Atom* a) : AtomBase(a), pred(a->pred) { };
    bool equals(Atom& a);
    bool is_static() const {
      return ((at == md_init) ||
	      (at == md_pos_goal) ||
	      (at == md_neg_goal) ||
	      pred->is_static());
    };
    bool check();
    Instance::Atom* find_prop(Instance& ins, bool neg, bool create);
    void build(Instance& ins, bool neg, index_type p);
    Atom* instantiate_partially();
    partial_value partial_eval(ptr_table* r, index_type p);
    partial_value partial_eval();
    bool initial_value();

    // instance of: true iff EVERY instance of this is an instance of a/u
    // AND EVERY instance of a/u is an instance of this
    bool instance_of(Atom* a, symbol_pair_vec& u);
    bool instance_of(Atom* a, const symbol_pair_vec& neq, symbol_pair_vec& u);

    // unify: true iff this/u == a/u (i.e., this and a have SOME instance
    // in common)
    bool unify(Atom* a, symbol_pair_vec& u);
    bool unify(Atom* a, const symbol_pair_vec& neq, symbol_pair_vec& u);

    void print(std::ostream& s, bool neg) const;
    void print(std::ostream& s) const { print(s, false); };
  };

  struct CAtom : public Atom {
    symbol_pair_vec neq;
    CAtom(Atom* a) : Atom(a), neq(symbol_pair(0, 0), 0) { };
    CAtom(CAtom* s) : Atom(s), neq(s->neq) { };
    CAtom(const Atom* a, symbol_pair_vec& u);
    CAtom(const Atom* a, symbol_pair_vec& n, symbol_pair_vec& u);

    bool instance_of(Atom* a, symbol_pair_vec& u);
    bool instance_of(Atom* a, const symbol_pair_vec& neq, symbol_pair_vec& u);
    bool instance_of(CAtom* a, symbol_pair_vec& u);
    bool unify(Atom* a, symbol_pair_vec& u);
    bool unify(CAtom* a, symbol_pair_vec& u);

    void print(std::ostream& s);
  };

  typedef lvector<CAtom*> catom_vec;

  struct FTerm : AtomBase {
    ObjectFunctionSymbol* fun;
    FTerm(ObjectFunctionSymbol* f) : fun(f) { };
    bool equals(FTerm& a);
    void print(std::ostream& s) const;
  };

  struct OInitAtom : AtomBase {
    ObjectFunctionSymbol* fun;
    Symbol*               val;
    OInitAtom(ObjectFunctionSymbol* f) : fun(f), val(0) { };
    void print(std::ostream& s);
  };

  typedef lvector<OInitAtom*> obj_init_atom_vec;

  struct FInitAtom : AtomBase {
    FunctionSymbol* fun;
    NTYPE           val;
    Instance::resource_ref res;
    FInitAtom(FunctionSymbol* f) : fun(f), val(0) { };
    FInitAtom(FChangeAtom* a);
    bool is_static() const { return fun->is_static(); };
    void print(std::ostream& s);
  };

  typedef lvector<FInitAtom*> fun_init_atom_vec;

  struct Expression;
  typedef lvector<Expression*> exp_vec;

  struct Expression {
    expression_class exp_class;
    Expression(expression_class c) : exp_class(c) { };
    static bool print_nary;
    bool is_static();
    bool is_constant();
    bool is_integral();
    NTYPE eval_static();
    bool eval_partial(NTYPE& val);
    interval eval_bounds();
    NTYPE eval_init();
    NTYPE eval_delta(ch_atom_vec& incs, ch_atom_vec& decs);
    NTYPE eval_delta(Symbol* preference, NTYPE p_value, NTYPE d_value);
    Expression* copy();
    Expression* simplify();
    void collect_constants(exp_vec& c);
    NTYPE integrify();
    void substitute_for_time(Expression* e);
    Expression* substitute_for_preference(Symbol* n, Expression* e);
    bool equals(Expression* e);
    void print_sum(std::ostream& s, bool grnd);
    void print_product(std::ostream& s, bool grnd);
    void print(std::ostream& s, bool grnd);
  };

  struct ListExpression : public Expression {
    Symbol* sym;
    ListExpression* rest;
    ListExpression(Symbol* s, ListExpression* r) :
      Expression(exp_list), sym(s), rest(r) { };
    bool match(AtomBase* atom);
  };

  struct FunctionExpression : public Expression {
    FunctionSymbol* fun;
    ListExpression* args;
    FunctionExpression(FunctionSymbol* f, ListExpression* a) :
      Expression(exp_fun), fun(f), args(a) { };
    bool match(FChangeAtom* atom);
    NTYPE eval_delta(ch_atom_vec& incs, ch_atom_vec& decs);
    FChangeAtom* make_atom_base();
  };

  struct ConstantExpression : public Expression {
    NTYPE val;
    ConstantExpression(NTYPE v) :
      Expression(exp_const), val(v) { };
  };

  struct TimeExpression : public Expression {
    Expression* time_exp;
    TimeExpression() :
      Expression(exp_time), time_exp(0) { };
    TimeExpression(Expression* e) :
      Expression(exp_time), time_exp(e) { };
  };

  struct BinaryExpression : public Expression {
    Expression* first;
    Expression* second;
    BinaryExpression(expression_class c, Expression* e1, Expression* e2) :
      Expression(c), first(e1), second(e2) { };
  };

  struct PreferenceExpression : public Expression {
    Symbol* name;
    PreferenceExpression(Symbol* n) :
      Expression(exp_preference), name(n) { };
  };

  struct Relation {
    relation_type rel;
    mode_keyword  at;
    Expression* first;
    Expression* second;
    Relation(relation_type r, Expression* e1, Expression* e2) :
      rel(r), at(md_none), first(e1), second(e2) { };
    Relation(relation_type r, mode_keyword m, Expression* e1, Expression* e2) :
      rel(r), at(m), first(e1), second(e2) { };

    Expression* match_gteq_constant(FChangeAtom* atom);
    FunctionExpression* match_lteq_fun(FChangeAtom* atom);
    bool is_static();
    partial_value partial_eval();
    void print(std::ostream& s, bool grnd);
  };

  typedef lvector<Relation*> relation_vec;

  struct FChangeAtom : AtomBase {
    FunctionSymbol* fun;
    Expression*     val;
    FChangeAtom(FunctionSymbol* f) :
      fun(f), val(0) { };
    FChangeAtom(FunctionSymbol* f, mode_keyword t) :
      AtomBase(t), fun(f), val(0) { };
    FChangeAtom(FChangeAtom* a, NTYPE v) :
      AtomBase(a), fun(a->fun), val(new ConstantExpression(v)) { };
    bool equals(FChangeAtom& a);
    bool fluent_equals(FChangeAtom& a);
    bool fluent_and_mode_equals(FChangeAtom& a);
    FChangeAtom* find_fluent_equals(ch_atom_vec& vec);
    Instance::Resource* find_resource(Instance& ins);
    void print(std::ostream& s);
  };

  struct Formula {
    formula_class fc;
    Formula(formula_class c) : fc(c) { };

    // replace 1st by 2nd
    void rename_variables_1(PDDL_Base::symbol_pair_vec& sub);
    // replace 2nd by 1st
    void rename_variables_2(PDDL_Base::symbol_pair_vec& sub);
    // replace 1st by 2nd
    void rename_predicates_1(PDDL_Base::symbol_pair_vec& sub);
    // replace 2nd by 1st
    void rename_predicates_2(PDDL_Base::symbol_pair_vec& sub);

    Formula* simplify();
    void write_otter(std::ostream& s) const;
    void print(std::ostream& s) const;
    void untype(PDDL_Base* base);
  };

  typedef lvector<Formula*> formula_vec;

  struct AFormula : public Formula, public Atom {
    AFormula(Atom* a) :
      Formula(fc_atom), Atom(a) { };
    AFormula(PredicateSymbol* p) :
      Formula(fc_atom), Atom(p) { };
    AFormula(PredicateSymbol* p, variable_vec& a) :
      Formula(fc_atom), Atom(p, a, false) { };

    void print(std::ostream& s) const;
    void write_otter(std::ostream& s) const;
  };

  struct EqFormula : public Formula {
    Symbol* t1;
    Symbol* t2;
    EqFormula(Symbol* s1, Symbol* s2) :
      Formula(fc_equality), t1(s1), t2(s2) { };
    void write_otter(std::ostream& s) const;
  };

  struct NFormula : public Formula {
    Formula* f;
    NFormula(Formula* g) : Formula(fc_negation), f(g) { };
  };

  struct BFormula : public Formula {
    Formula* f1;
    Formula* f2;
    BFormula(formula_class c, Formula* g1, Formula* g2) :
      Formula(c), f1(g1), f2(g2) { };
  };

  struct CFormula : public Formula {
    formula_vec parts;
    CFormula(formula_class c) : Formula(c), parts(0, 0) { };
    CFormula(formula_class c, Formula* f1, Formula* f2) :
      Formula(c), parts(0, 0)
    {
      parts.append(f1);
      parts.append(f2);
    };
    void add(Formula* f) { parts.append(f); };
    void add(formula_vec& f) { parts.append(f); };
    void write_otter(std::ostream& s) const;
  };

  struct QFormula : public Formula {
    variable_vec vars;
    Formula* f;
    QFormula(formula_class c) : Formula(c), vars(0, 0), f(0) { };
    QFormula(formula_class c, Formula* g) : Formula(c), vars(0, 0), f(g) { };
    QFormula(formula_class c, variable_vec& v, Formula* g) :
      Formula(c), vars(0, 0), f(g)
    {
      for (index_type k = 0; k < v.length(); k++) vars.append(v[k]);
    };
    void add(PDDL_Base::VariableSymbol* v) { vars.append(v); };
    void add(PDDL_Base::variable_vec& v) { vars.append(v); };
    void write_otter(std::ostream& s) const;
  };

  struct TypeConstraint {
    VariableSymbol* var;
    TypeSymbol*     typ;
    TypeConstraint(VariableSymbol* v, TypeSymbol* t) : var(v), typ(t) { };
    bool is_true();
    partial_value partial_eval();
    void print(std::ostream& s);
  };

  typedef lvector<TypeConstraint*> type_constraint_vec;

  struct Context {
    variable_vec param;
    atom_vec     pos_con;
    atom_vec     neg_con;
    type_constraint_vec type_con;

    Context() : param(0, 0), pos_con(0, 0), neg_con(0, 0), type_con(0, 0) { };
    Context(const Context* c) : param(c->param), pos_con(c->pos_con),
	 neg_con(c->neg_con), type_con(c->type_con) { };
    bool context_is_static() const;
    bool occurs_in_context(Symbol* s);
    void clear_arguments();
    void set_mode(mode_keyword m);
    bool is_true();
    partial_value partial_eval();
    void print(std::ostream& s);
    void print_assignment(std::ostream& s);
    void untype(PDDL_Base* base);
  };

  struct DKEL_Item : public Context {
    char*      item_name;
    Symbol*    name;
    string_set item_tags;
    bool       defined_in_problem;
    bool       included;
    DKEL_Item(char* name) : item_name(name), name(0), 
	 defined_in_problem(false), included(true) { };
    DKEL_Item(const DKEL_Item* i) : Context(i), item_name(i->item_name),
	 name(i->name), item_tags(i->item_tags),
	 defined_in_problem(i->defined_in_problem), included(i->included) { };
    bool item_is_included(string_set& ex_tags, string_set& req_tags);
    void print_begin(std::ostream& s);
    void print_end(std::ostream& s);
    void print(std::ostream& s);
    void write_dkel(std::ostream& s);
  };

  struct SetOf : public Context {
    atom_vec  pos_atoms;
    atom_vec  neg_atoms;
    SetOf() : pos_atoms(0, 0), neg_atoms(0, 0) { };
    SetOf(const SetOf* s)
      : Context(s), pos_atoms(s->pos_atoms), neg_atoms(s->neg_atoms)
    { };
    SetOf(Atom* a)
    { pos_atoms.append(a); };
    SetOf(Atom* a, bool n)
    { if (n) neg_atoms.append(a); else pos_atoms.append(a); };
    bool is_static() const;
    void set_mode(mode_keyword m);
    // by default, SetOf::partial_eval evaluates the atom set as a
    // conjunctive condition, i.e., as
    //  (forall <param> (imply <context> (and <atoms>)))
    // if as_disjunction is true, it is evaluated as
    //  (exists <param> (adn <context> (or <atoms>)))
    // to evaluate the context only, use Context::partial_eval.
    partial_value partial_eval(bool as_disjunction = false);
    partial_value partial_eval(index_type p, bool as_disjunction = false);
    // instantiate as set of atoms/conjunctive condition: negated atoms
    // are instantiated to negative propositions
    void instantiate_as_set(Instance& ins, index_set& s);
    // instantiate as effect: negated atoms are instantiated to positive
    // propositions and placed in s_del; for both negated and un-negated
    // atoms, the complementary proposition is created if required
    // (predicate has neg_pre flag set) and placed in opposite effect set
    void instantiate_as_effect(Instance& ins, index_set& s_add,
			       index_set& s_del);
    // instantiate as two sets of rules, separated on whether the
    // consequent atom is positive or negative
    void instantiate_conditional(Instance& ins, rule_set& s_pos,
				 rule_set& s_neg);
    SetOf* instantiate_partially();
    // compile set condition with static context into lists of positive
    // and negative atom instances
    void compile(atom_vec& pos_ins, atom_vec& neg_ins, index_type p);
    // compile set condition with non-static context into disjunction
    // (single SetOf struct, appended to vector)
    void compile_non_static(atom_set_vec& ins, index_type p);
    void print(std::ostream& s);
    void print_as_disjunction(std::ostream& s);

    // internal methods
    void build_set(Instance& ins, index_set& s, index_type p);
    void build_effect(Instance& ins, index_set& s_add, index_set& s_del,
		      index_type p);
    void build_conditional(Instance& ins, rule_set& s_pos, rule_set& s_neg,
			   index_type p);
  };

//  struct Disjunction : public Context {
//    atom_vec  pos_atoms;
//    atom_vec  neg_atoms;
//    Disjunction() : pos_atoms(0, 0), neg_atoms(0, 0) { };
//    bool is_static();
//    // note: Disjunction::partial_eval evaluates the disjunctive condition
//    // (to evaluate the context, use Context::partial_eval)
//    partial_value partial_eval(index_type p);
//    partial_value partial_eval();
//    void build(Instance& ins, index_set& s, index_type p);
//    void instantiate(Instance& ins, index_set& s);
//    Disjunction* instantiate_partially();
//    void print(std::ostream& s);
//  };

  struct QCNumericEffect : public Context {
    FChangeAtom* atom;
    QCNumericEffect() : atom(0) { };
    QCNumericEffect(FChangeAtom* a) : atom(a) { };
    void print(std::ostream& s);
  };

  typedef lvector<QCNumericEffect*> qc_numeric_effect_vec;

  enum set_constraint_keyword { sc_at_least, sc_at_most, sc_exactly };

  struct SetConstraint : public DKEL_Item {
    set_constraint_keyword sc_type;
    index_type             sc_count;
    atom_vec     pos_atoms;
    atom_vec     neg_atoms;
    atom_set_vec atom_sets;
    SetConstraint() : DKEL_Item(":invariant"), sc_type(sc_at_least),
	 sc_count(0), pos_atoms(0, 0), neg_atoms(0, 0), atom_sets(0, 0) { };
    SetConstraint(const DKEL_Item* item) : DKEL_Item(item),
	 sc_type(sc_at_least), sc_count(0), pos_atoms(0, 0), neg_atoms(0, 0),
	 atom_sets(0, 0) { };
    void build(Instance& ins, index_type p);
    void instantiate(Instance& ins);
    void print(std::ostream& s);
    void write_dkel(std::ostream& s);
    void untype(PDDL_Base* base);
  };

  struct InvariantFormula : public DKEL_Item {
    Formula* f;
    InvariantFormula(Formula* g)
      : DKEL_Item(":invariant"), f(g) { };
    InvariantFormula(const DKEL_Item* item, Formula* g)
      : DKEL_Item(item), f(g) { };
    void write_dkel(std::ostream& s);
    void untype(PDDL_Base* base);
  };

  struct Reference : public AtomBase {
    Symbol* name;
    bool    neg;
    bool    has_args;
    index_set index;
    Reference(Symbol* n) : name(n), neg(false), has_args(true) { };
    Reference(Symbol* n, bool ng, bool ha)
      : name(n), neg(ng), has_args(ha) { };
    bool match(symbol_vec& args);
    bool match(variable_vec& args);
    void* find_action();
    void find(const name_vec& names, index_set& ind);
    Reference* instantiate_partially();
    void print(std::ostream& s);
  };

  struct IrrelevantItem : public DKEL_Item {
    Reference* entity;
    IrrelevantItem() : DKEL_Item(":irrelevant"), entity(0) { };
    bool match(symbol_vec& args);
    bool match(variable_vec& args);
    IrrelevantItem* instantiate_partially();
    void print(std::ostream& s);
    void write_dkel(std::ostream& s);
  };

  struct SequentialTaskNet : public Context {
    ref_vec tasks;
    ActionSymbol* abs_act;

    SequentialTaskNet() : tasks(0, 0), abs_act(0) { };
    void build(HTNInstance& ins, HTNInstance::Task& t, index_type p);
    void instantiate(HTNInstance& ins, HTNInstance::Task& t);
    void print(std::ostream& s);
  };

  typedef lvector<SequentialTaskNet*> task_net_vec;

  struct SetSymbol : public Symbol {
    ptr_table  set_table;
    lvector<index_set*> sets;
    name_vec   names;
    SetSymbol(char* n) : Symbol(sym_set, n), sets(0, 0), names(0, 0) { };
  };

  struct SetName : public AtomBase {
    SetSymbol* sym;
    SetName(SetSymbol* s) : sym(s) { };
    index_set* find();
    SetName* instantiate_partially();
    void print(std::ostream& s);
    void print_instance(std::ostream& s);
  };

  struct ActionSymbol : public ParamSymbol {
    atom_vec     pos_pre;
    atom_vec     neg_pre;

    atom_set_vec set_pre;
    atom_set_vec dis_pre;
    // disjunction_vec dis_pre;
    relation_vec num_pre;

    // simple & quantified propositional effects
    atom_vec     adds;
    atom_vec     dels;
    atom_set_vec set_eff;

    // simple consume effects
    atom_vec     cons;

    // quantified conditional propositional effects
    atom_set_vec cond_eff;

    // temporary propositional effects
    atom_vec     locks;
    atom_vec     enables;

    // numeric effects
    ch_atom_vec  reqs;
    ch_atom_vec  incs;
    ch_atom_vec  decs;
    ch_atom_vec  fass;

    // quantified conditional numeric effects
    qc_numeric_effect_vec qc_incs;
    qc_numeric_effect_vec qc_decs;
    qc_numeric_effect_vec qc_fass;

    Expression*  dmin;
    Expression*  dmax;
    irrelevant_vec irr_ins;
    ref_vec      refs;

    task_net_vec exps;

    ptr_table    instances;
    SetName*     part;

    const char*  assoc;

    ActionSymbol(char* n) : ParamSymbol(sym_action, n), pos_pre(0, 0),
	 neg_pre(0, 0), set_pre(0, 0), dis_pre(0, 0), num_pre(0, 0),
	 adds(0, 0), dels(0, 0), set_eff(0, 0), cons(0, 0), cond_eff(0, 0),
	 locks(0, 0), enables(0, 0), reqs(0, 0), incs(0, 0), decs(0, 0),
	 fass(0, 0), qc_incs(0, 0), qc_decs(0, 0), qc_fass(0, 0), dmin(0),
	 dmax(0), irr_ins(0, 0), refs(0, 0), exps(0, 0), part(0), assoc(0) { };

    // "internal" methods
    void build(Instance& ins, index_type p, Expression* cost_exp);
    Instance::Action& build_action(Instance& ins,
				   PDDL_Name* name,
				   index_type& count,
				   const index_set& pre,
				   const index_set& add,
				   const index_set& del,
				   const index_set& lck,
				   const index_cost_vec& r_use,
				   const index_cost_vec& r_cons,
				   NTYPE dmin,
				   NTYPE dmax,
				   NTYPE dur,
				   NTYPE cost);
    void build_actions_with_dc(Instance& ins,
			       PDDL_Name* name,
			       index_type& count,
			       const index_set& pre,
			       const index_set& add,
			       const index_set& del,
			       const index_set& lck,
			       const index_cost_vec& r_use,
			       const index_cost_vec& r_cons,
			       NTYPE dmin,
			       NTYPE dmax,
			       NTYPE dur,
			       NTYPE cost,
			       const index_set_vec& dc,
			       index_vec& s,
			       index_type p);
    void build_actions_with_dc(Instance& ins,
			       PDDL_Name* name,
			       index_type& count,
			       const index_set& pre,
			       const index_set& add,
			       const index_set& del,
			       const index_set& lck,
			       const index_cost_vec& r_use,
			       const index_cost_vec& r_cons,
			       NTYPE dmin,
			       NTYPE dmax,
			       NTYPE dur,
			       NTYPE cost,
			       const index_set_vec& dc);
    void build_actions_with_dc_and_ce(Instance& ins,
				      PDDL_Name* name,
				      index_type& count,
				      const index_set& pre,
				      const index_set& add,
				      const index_set& del,
				      const index_set& lck,
				      const index_cost_vec& r_use,
				      const index_cost_vec& r_cons,
				      NTYPE dmin,
				      NTYPE dmax,
				      NTYPE dur,
				      NTYPE cost,
				      const index_set_vec& dc,
				      const rule_set& pce,
				      const rule_set& nce,
				      const bool_vec& ece);
    index_type param_index(VariableSymbol* p);
    void get_param_inequalities(symbol_pair_vec& neq);
    void set_arguments(const symbol_vec& args);
    void set_arguments(const ptr_table::key_vec& args);
    void clear_arguments();
    void build_abstract(HTNInstance& ins, index_type p);
    void* find_instance();

    // "public" methods
    bool is_abstract();
    void post_process();
    void instantiate(Instance& ins, Expression* cost_exp);
    void instantiate_abstract_1(HTNInstance& ins);
    void instantiate_abstract_2(HTNInstance& ins);
    void write_prototype(std::ostream& s);
    void print(std::ostream& s);
    void untype(PDDL_Base* base);
  };

  struct InputPlanStep {
    Reference* act;
    NTYPE start_time;
    InputPlanStep() : act(0), start_time(0) { };
    InputPlanStep(Reference* a, NTYPE t) : act(a), start_time(t) { };
  };

  struct InputPlan {
    Symbol* name;
    bool    is_opt;
    lvector<InputPlanStep*> steps;
    InputPlan() : name(0), is_opt(false), steps(0, 0) { };

    bool export_to_instance(Instance& ins, const index_vec& map, Plan& p);
    void print(std::ostream& s);
  };

  struct HTableEntry {
    atom_vec   atoms;
    bool_vec   neg;
    NTYPE      cost;
    bool       opt;
    HTableEntry() : atoms(0, 0), neg(false, 0), cost(0), opt(false) { };
  };

  struct SimpleReferenceSet : public Context {
    Reference* ref;
    SimpleReferenceSet(Reference* r) : ref(r) { };
    bool build(const name_vec& names, index_set& set, index_type p);
    void print(std::ostream& s);
  };

  typedef lvector<SimpleReferenceSet*> refs_vec;

  struct ReferenceSet : public Context {
    refs_vec refs;
    Symbol*  name;

    ReferenceSet() : refs(0, 0), name(0) { };
    void add(SimpleReferenceSet* ref) { refs.append(ref); };
    void build(const name_vec& names, index_set_vec& sets, index_type p);
    void print(std::ostream& s);
  };

  struct CPG {
    atom_vec atoms;
    index_vec atom_first_arg;
    bool_vec neg;
    symbol_vec args;
    type_vec arg_types;

    CPG() : atoms(0, 0), neg(false, 0), args(0, 0), arg_types(0, 0) { };
    CPG(CPG& g, index_vec& s);
    void make_key(ptr_table::key_vec& key);
    void make_typed_key(ptr_table::key_vec& key);
    void make_parameters(variable_vec& params);
    ListExpression* make_argument_list(index_type first);
    void add_asserting_effects(ActionSymbol* act,
			       PredicateSymbol* p,
			       bool p_val,
			       PredicateSymbol* g,
			       FunctionSymbol* f,
			       Expression* f_val,
			       bool strict);
    void add_destroying_effects(ActionSymbol* act,
				ParamSymbol* pf,
				PredicateSymbol* p,
				bool p_val,
				PredicateSymbol* g,
				FunctionSymbol* f,
				Expression* f_val,
				bool strict);
    bool initial_value();

    // internals
    void add_asserting_effects(index_type c_atom,
			       ActionSymbol* act,
			       index_type c_eff,
			       bool_vec& sat,
			       PredicateSymbol* p,
			       bool p_val,
			       PredicateSymbol* g,
			       FunctionSymbol* f,
			       Expression* f_val,
			       symbol_pair_vec& eq,
			       symbol_pair_vec& neq,
			       bool strict);
    void add_propositional_effect(ActionSymbol* act,
				  PredicateSymbol* p, bool p_val,
				  index_type c_atom, Atom* a_eff,
				  bool strict);
    void add_fluent_effect(ActionSymbol* act,
			   FunctionSymbol* f, Expression* f_val,
			   index_type c_atom, Atom* a_eff);
    void add_effect_conditions(Context* e, ParamSymbol* pf, bool_vec& sat,
			       symbol_vec& subs, symbol_pair_vec& eq,
			       symbol_pair_vec& neq);
  };

  enum goal_class { goal_pos_atom, goal_neg_atom, goal_relation, goal_task,
		    goal_conjunction, goal_disjunction, goal_forall,
		    goal_exists, goal_always, goal_sometime,
		    goal_at_most_once, goal_within, goal_always_within,
		    goal_sometime_before, goal_sometime_after };

  struct Goal {
    goal_class g_class;
    Goal(goal_class c) : g_class(c) { };
    bool is_state();
    bool is_propositional();
    bool is_singular();
    bool makeCPG(CPG& g);
    bool makeCPG(CPG& g, index_vec& s);
    void instantiate(Instance& ins, NTYPE deadline);
    void instantiate(Instance& ins, index_set& set, Symbol* p, index_type i);
    void print(std::ostream& s);
  };

  typedef lvector<Goal*> goal_vec;

  struct AtomicGoal : public Goal {
    Atom* atom;
    AtomicGoal(Atom* a, bool neg)
      : Goal(neg ? goal_neg_atom : goal_pos_atom), atom(a) { };
    void print(std::ostream& s);
  };

  struct NumericGoal : public Goal {
    Relation* rel;
    NumericGoal(Relation* r) : Goal(goal_relation), rel(r) { };
    void print(std::ostream& s);
  };

  struct TaskGoal : public Goal {
    Reference* task;
    TaskGoal(Reference* r) : Goal(goal_task), task(r) { };
    void print(std::ostream& s);
  };

  struct ConjunctiveGoal : public Goal {
    goal_vec goals;
    ConjunctiveGoal() : Goal(goal_conjunction), goals(0, 0) { };
    ConjunctiveGoal(goal_class gc) : Goal(gc), goals(0, 0) { };
    void print(std::ostream& s);
  };

  struct DisjunctiveGoal : public ConjunctiveGoal {
    DisjunctiveGoal() : ConjunctiveGoal(goal_disjunction) { };
    void print(std::ostream& s);
  };

  struct QuantifiedGoal : public Goal, public Context {
    Goal* goal;
    QuantifiedGoal() : Goal(goal_forall), goal(0) { };
    QuantifiedGoal(goal_class gc) : Goal(gc), goal(0) { };
    void print(std::ostream& s);
  };

  struct SimpleSequenceGoal : public Goal {
    Goal* constraint;
    SimpleSequenceGoal(goal_class c, Goal* g) : Goal(c), constraint(g) { };
    void print(std::ostream& s);
  };

  struct TriggeredSequenceGoal : public Goal {
    Goal* trigger;
    Goal* constraint;
    TriggeredSequenceGoal(goal_class c, Goal* t, Goal* g)
      : Goal(c), trigger(t), constraint(g) { };
    void print(std::ostream& s);
  };

  struct DeadlineGoal : public Goal {
    Goal* goal;
    NTYPE at;
    DeadlineGoal(NTYPE t, Goal* g) : Goal(goal_within), goal(g), at(t) { };
    void print(std::ostream& s);
  };

  struct TriggeredDeadlineGoal : public Goal {
    Goal* trigger;
    Goal* goal;
    NTYPE delay;
    TriggeredDeadlineGoal(Goal* t, NTYPE d, Goal* g)
      : Goal(goal_always_within), trigger(t), delay(d), goal(g) { };
    void print(std::ostream& s);
  };

  struct Preference {
    Symbol* name;
    Goal*   goal;

    Preference() : name(0), goal(0) { };
    Preference(Symbol* n) : name(n), goal(0) { };
    Preference(Symbol* n, Goal* g) : name(n), goal(g) { };
    bool is_state() { return goal->is_state(); };
    bool is_propositional() { return goal->is_propositional(); };
    NTYPE value(metric_class metric_type, Expression* m);
    void instantiate(SoftInstance& ins,
		     metric_class metric_type,
		     Expression* m);
    void print(std::ostream& s);
  };

  typedef lvector<Preference*> preference_vec;

  char* domain_name;
  char* problem_name;
  char* domain_file;
  char* problem_file;
  bool  ready_to_instantiate;

  char* problem_file_basename();
  char* enum_problem_filename(const char* s, index_type i);

  StringTable& tab;

  type_vec    dom_types;
  type_vec    dom_base_types;
  TypeSymbol* dom_top_type;

  symbol_vec  dom_constants;

  PredicateSymbol* dom_eq_pred;
  PredicateSymbol* dom_assign_pred;
  Symbol*          dom_undefined_obj;
  static PredicateSymbol* current_eq_predicate;

  lvector<PredicateSymbol*> dom_predicates;
  lvector<ObjectFunctionSymbol*>  dom_object_functions;
  lvector<FunctionSymbol*>  dom_functions;
  lvector<ActionSymbol*>    dom_actions;

  lvector<SetConstraint*>   dom_sc_invariants;
  lvector<InvariantFormula*> dom_f_invariants;
  lvector<IrrelevantItem*>  dom_irrelevant;

  atom_vec      dom_init;
  obj_init_atom_vec dom_obj_init;
  fun_init_atom_vec dom_fun_init;
  goal_vec      dom_goals;
  preference_vec dom_preferences;
  ref_vec       goal_tasks;

  metric_class  metric_type;
  Expression*   metric;

  NTYPE         serial_length;
  NTYPE         parallel_length;

  lvector<InputPlan*>    input_plans;
  lvector<HTableEntry*>  h_table;
  lvector<ReferenceSet*> input_sets;
  lvector<SetSymbol*>    partitions;

  static ActionSymbol* src_action_symbol(ptr_pair* p);

  // functions for extracting specific (named) information from
  // the domain and problem
  TypeSymbol*      find_type(const char* name);
  PredicateSymbol* find_predicate(const char* name);
  FunctionSymbol*  find_function(const char* name);
  bool  find_initial_fact(const char* pname, const symbol_vec& arg);
  NTYPE find_function_value(const char* fname, const symbol_vec& arg);
  index_type find_element_satisfying(const symbol_vec& elements,
				     const char* pname,
				     symbol_vec& arg,
				     index_type element_arg_p);
  void find_elements_satisfying(const symbol_vec& elements,
				const char* pname,
				symbol_vec& arg,
				index_type element_arg_p,
				index_set& sats);
  Atom* goal_to_atom(Goal* g);
  bool  goal_to_atom_vec(Goal* g, atom_vec& av);

  PDDL_Base(StringTable& t);
  ~PDDL_Base();

  void set_variable_type(variable_vec& vec, TypeSymbol* t);
  void set_variable_type(variable_vec& vec, const TypeSet& t);
  void set_type_type(type_vec& vec, TypeSymbol* t);
  void set_constant_type(symbol_vec& vec, TypeSymbol* t);
  void clear_context(variable_vec& vec);
  void clear_context(variable_vec& vec, index_type n_min, index_type n_max);

  // note: first argument of merge receives the result!
  bool merge_type_vectors(type_vec& v0, type_vec& v1);
  void make_parameters(type_vec& t, const char* prefix, variable_vec& v);

  Atom* make_atom_from_prop(ptr_pair& src, bool& neg);

  Symbol* gensym(symbol_class c, const char* p, const TypeSet& t);
  Symbol* gensym(symbol_class c, const char* p, TypeSymbol* t);
  Symbol* gensym_i(symbol_class c, const char* p, index_type i, TypeSymbol* t);
  Symbol* gensym_s(symbol_class c, const char* p, const Symbol* s, TypeSymbol* t);
  Symbol* gensym_n(symbol_class c, const char* p, const Name* n, TypeSymbol* t);
  void new_variable_substitution(Atom* a,
				 symbol_pair_vec& u,
				 symbol_pair_vec& new_u);
  Atom* new_meta_atom(PredicateSymbol* p);
  CAtom* new_CAtom(Atom* a, symbol_pair_vec& u);
  CAtom* new_CAtom(Atom* a, symbol_pair_vec& n, symbol_pair_vec& u);

  void extend_cc(CAtom* l, catom_vec& m, catom_vec& nm,
		 lvector< swapable_pair<catom_vec> >& x,
		 index_type d);
  void find_cc();

  void post_process();
  void instantiate(Instance& ins);
  // important: instantiateHTN should only be called after instantiate!
  void instantiateHTN(HTNInstance& ins);
  // important: instantiate_soft should only be called after instantiate!
  void instantiate_soft(SoftInstance& ins);
  void instantiate_atom_set(Instance& ins, atom_vec& a, index_set& s);
  index_type n_plans() const { return input_plans.length(); };
  bool export_plan(index_type i, Instance& ins, const index_vec& map, Plan& p);
  bool export_plan(index_type i, Instance& ins, Plan& p);
  void export_heuristic(Instance& ins,
			const index_vec& map,
			bool opt_maximize,
			Heuristic& h);
  void export_sets(const name_vec& names, index_set_vec& sets);
  void export_action_partitions(name_vec& names, index_set_vec& sets);

  void lift_DKEL_Items(const Instance& ins);

  PredicateSymbol* find_type_predicate(Symbol* type_sym);
  void untype();

  void compile_preferences();
  void compile_constraints_1();
  void compile_constraints_2();
  void select_preferences(const bool_vec& sel);
  void metric_to_goal(NTYPE bound);
  void compile_set_conditions_and_effects();

  void compile_object_functions();

  // internal submethods
  Expression* replace_violations_1(Expression* exp,
				   CPG* cpg[],
				   FunctionSymbol* f_violated[]);
  Goal* compile_constraint_1(Goal* g, index_type i, const Symbol* n,
			     symbol_vec& aut_states, index_type n_ra);
  AtomicGoal* make_automaton_type_a(CPG& f, index_type i, const Symbol* n,
				    symbol_vec& aut_state, index_type n_ra);
  AtomicGoal* make_automaton_type_e(CPG& f, index_type i, const Symbol* n,
				    symbol_vec& aut_state, index_type n_ra);
  AtomicGoal* make_automaton_type_o(CPG& f, index_type i, const Symbol* n,
				    symbol_vec& aut_state, index_type n_ra);
  AtomicGoal* make_automaton_type_sb(CPG& f, CPG& g,
				     index_type i, const Symbol* n,
				     symbol_vec& aut_state, index_type n_ra);
  AtomicGoal* make_automaton_type_sa(CPG& f, CPG& g,
				     index_type i, const Symbol* n,
				     symbol_vec& aut_state, index_type n_ra);
  void make_automaton_transition(Symbol* s_from,
				 Symbol* s_to,
				 bool is_accept,
				 CPG* f, bool neg_f,
				 CPG* g, bool neg_g,
				 PredicateSymbol* p_state,
				 PredicateSymbol* p_accept,
				 PredicateSymbol* p_synch);
  void add_precondition_formula(ActionSymbol* a, CPG* f, bool is_neg);

  Goal* compile_constraint_2(Goal* g, const Symbol* n);
  Goal* compile_always_constraint(SimpleSequenceGoal* g, const Symbol* n);
  Goal* compile_sometime_constraint(SimpleSequenceGoal* g, const Symbol* n);
  Goal* compile_at_most_once_constraint(SimpleSequenceGoal* g,
					const Symbol* n);
  Goal* compile_sometime_before_constraint(TriggeredSequenceGoal* g,
					   const Symbol* n);

  void compile_set_conditions_and_effects(ActionSymbol* act);
  void compile_set_conditions_and_effects(ActionSymbol* act,
					  variable_vec& i_param,
					  variable_vec& d_param,
					  index_type p);

  Atom* make_binding_atom(VariableSymbol* v);
  void compile_object_functions(ActionSymbol* act, Symbol* undefined_value);
  void compile_object_functions_for_validator(ActionSymbol* act);
  Goal* compile_object_functions(Goal* g);

  // write parts in PDDL(1,2,3)/DKEL
  void write_declarations(std::ostream& s);
  void write_action(std::ostream& s, ActionSymbol* act);
  void write_set_precondition(std::ostream& s, SetOf* set);
  void write_disjunctive_set_precondition(std::ostream& s, SetOf* set);
  void write_set_effect(std::ostream& s, SetOf* set);
  void write_QCN_effect(std::ostream& s, const char* effect_type, QCNumericEffect* qcn);
  void write_objects(std::ostream& s, bool defined_in_problem);
  void write_init(std::ostream& s);
  void write_goal(std::ostream& s);
  void write_metric(std::ostream& s);
  void write_dkel_items(std::ostream& s, bool defined_in_problem);

  // write domain/problem in PDDL(1,2,3)/DKEL
  void write_domain_begin(std::ostream& s);
  void write_problem_begin(std::ostream& s);
  void write_end(std::ostream& s);
  void write_dkel_domain(std::ostream& s, bool leave_open);
  void write_dkel_problem(std::ostream& s, bool leave_open);

  // write extras in reader-friendly syntax
  void write_plans(std::ostream& s);
  void write_heuristic_table(std::ostream& s);
  void write_sets(std::ostream& s);

  // print (dump)
  void print(std::ostream& s);
};

class InstanceName : public Name {
  char* domain_name;
  char* problem_name;
 public:
  InstanceName(char* d, char* p) : domain_name(d), problem_name(p) { };
  virtual ~InstanceName() { };
  virtual void write(std::ostream& s, unsigned int c) const;
  virtual const Name* cast_to(const char* cname) const;
};

class PDDL_Name : public Name {
 protected:
  bool neg;
  PDDL_Base::Symbol* sym;
  PDDL_Base::symbol_vec arg;
  bool     vis;
  bool_vec avis;
 public:
  static char catc;

  PDDL_Name(PDDL_Base::Symbol* s)
    : neg(false), sym(s), arg(0, 0), vis(true), avis(true, 0) { };
  PDDL_Name(PDDL_Base::Symbol* s, bool n)
    : neg(n), sym(s), arg(0, 0), vis(true), avis(true, 0) { };
  PDDL_Name(PDDL_Base::Symbol* s, PDDL_Base::symbol_vec a, bool n)
    : neg(n), sym(s), arg(a), vis(true), avis(true, a.length()) { };
  PDDL_Name(PDDL_Base::Symbol* s, PDDL_Base::variable_vec a, bool n);
  void add(PDDL_Base::Symbol* s);
  void add(PDDL_Base::Symbol* s, bool v);
  PDDL_Base::Symbol* symbol() { return sym; };
  PDDL_Base::symbol_vec& args() { return arg; };
  index_type argc() { return arg.length(); };
  bool is_neg() { return  neg; };
  virtual ~PDDL_Name() { };
  virtual void write(std::ostream& s, unsigned int c) const;
  virtual const Name* cast_to(const char* cname) const;
};

class Numbered_PDDL_Name : public PDDL_Name {
  index_type copy;
 public:
  Numbered_PDDL_Name(PDDL_Base::Symbol* sym, index_type c)
    : PDDL_Name(sym), copy(c) { };
  Numbered_PDDL_Name(PDDL_Name* n, index_type c);
  virtual ~Numbered_PDDL_Name() { };
  virtual void write(std::ostream& s, unsigned int c) const;
};

END_HSPS_NAMESPACE

#endif
