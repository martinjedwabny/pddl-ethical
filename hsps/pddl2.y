%name PDDL_Parser
%define ERROR log_error
%define ERROR_BODY = 0
%define ERROR_VERBOSE 1
%define LEX next_token
%define LEX_BODY = 0
%define DEBUG 1

%define INHERIT : public HSPS::PDDL_Base
%define CONSTRUCTOR_PARAM HSPS::StringTable& t
%define CONSTRUCTOR_INIT : HSPS::PDDL_Base(t), error_flag(false), \
  current_param(0, 0), stored_n_param(0, 0), current_atom(0), \
  current_atom_stack(0, 0), current_context(0), stored_context(0, 0), \
  current_item(0), current_goal(0, 0), current_preference_name(0), \
  current_entry(0), current_plan_file(0), n_plans_in_current_file(0)
%define MEMBERS \
public: \
virtual std::ostream& at_position(std::ostream& s) = 0; \
virtual char*         current_file() = 0; \
bool                  error_flag; \
private: \
HSPS::PDDL_Base::variable_vec current_param; \
HSPS::index_vec               stored_n_param; \
HSPS::PDDL_Base::TypeSet      current_type_set; \
HSPS::index_type              last_n_functions; \
HSPS::PDDL_Base::AtomBase*    current_atom; \
HSPS::PDDL_Base::atom_base_vec current_atom_stack; \
HSPS::PDDL_Base::Context*     current_context; \
HSPS::lvector<Context*> stored_context; \
HSPS::PDDL_Base::DKEL_Item*   current_item; \
HSPS::lvector<ConjunctiveGoal*> current_goal; \
HSPS::PDDL_Base::Symbol*      current_preference_name; \
HSPS::PDDL_Base::HTableEntry* current_entry; \
char* current_plan_file; \
HSPS::index_type n_plans_in_current_file;

%header{
#include <stdlib.h>
#include "base.h"
#include <sstream>
%}

%union {
  HSPS::StringTable::Cell*         sym;
  HSPS::PDDL_Base::Expression*     exp;
  HSPS::PDDL_Base::ListExpression* lst;
  HSPS::PDDL_Base::Relation*       rel;
  HSPS::PDDL_Base::Goal*           goal;
  HSPS::PDDL_Base::Formula*        ff;
  HSPS::PDDL_Base::mode_keyword    tkw;
  HSPS::PDDL_Base::relation_type   rkw;
  HSPS::PDDL_Base::set_constraint_keyword sckw;
  NNTYPE rval;
  int    ival;
  char*  sval;
}

%token TK_OPEN TK_CLOSE TK_OPEN_SQ TK_CLOSE_SQ TK_GREATER TK_GREATEQ
       TK_LESS TK_LESSEQ TK_COLON TK_HASHT TK_EQ

%left TK_HYPHEN TK_PLUS
%left TK_MUL TK_DIV
%left TK_UMINUS

%token <sym>  TK_NEW_SYMBOL TK_OBJ_SYMBOL TK_TYPE_SYMBOL TK_PRED_SYMBOL
              TK_OBJFUN_SYMBOL TK_FUN_SYMBOL TK_VAR_SYMBOL TK_ACTION_SYMBOL
              TK_MISC_SYMBOL TK_KEYWORD TK_NEW_VAR_SYMBOL
              TK_PREFERENCE_SYMBOL TK_SET_SYMBOL
%token <rval> TK_FLOAT
%token <ival> TK_INT
%token <sval> TK_STRING

%token KW_REQS KW_CONSTANTS KW_PREDS KW_FUNS KW_TYPES KW_DEFINE KW_DOMAIN
       KW_ACTION KW_PROCESS KW_EVENT KW_ACTION KW_ARGS KW_PRE KW_COND
       KW_AT_START KW_AT_END KW_OVER_ALL KW_EFFECT KW_INVARIANT KW_DURATION
       KW_AND KW_OR KW_EXISTS KW_FORALL KW_IMPLY KW_NOT KW_WHEN KW_EITHER
       KW_PROBLEM KW_FORDOMAIN KW_OBJECTS KW_INIT KW_GOAL KW_LENGTH KW_SERIAL
       KW_PARALLEL KW_METRIC KW_MINIMIZE KW_MAXIMIZE KW_DURATION_VAR
       KW_TOTAL_TIME KW_INCREASE KW_DECREASE KW_SCALE_UP KW_SCALE_DOWN
       KW_ASSIGN KW_INVARIANT KW_TAG KW_NAME KW_VARS KW_SET_CONSTRAINT KW_SETOF
       KW_AT_LEAST_N KW_AT_MOST_N KW_EXACTLY_N KW_CONTEXT KW_FORMULA
       KW_IRRELEVANT KW_PLAN KW_HEURISTIC KW_OPT KW_INF KW_FACT KW_SET
       KW_EXPANSION KW_TASKS KW_PREFERENCE KW_VIOLATED KW_WITHIN
       KW_ASSOC KW_CONSTRAINTS KW_ALWAYS KW_SOMETIME KW_AT_MOST_ONCE
       KW_SOMETIME_BEFORE KW_SOMETIME_AFTER KW_ALWAYS_WITHIN KW_IFF KW_FALSE
       KW_TRUE KW_NUMBER KW_UNDEFINED

%type <rval> numeric_value
%type <exp>  d_expression d_sum d_product d_function metric_expression
%type <rel>  numeric_condition
%type <lst>  d_argument_list
%type <sym>  action_symbol any_symbol atom_argument flat_atom_argument functional_atom_argument object_assignment_value
%type <tkw>  timing_keyword
%type <rkw>  relation_keyword
%type <sckw> set_constraint_type
%type <goal>  new_goal new_single_goal
%type <ff> fol_formula fol_formula_list

%start pddl_declarations

%%

pddl_declarations:
pddl_declarations pddl_domain
| pddl_declarations pddl_problem
| pddl_declarations pddl_plan
| pddl_declarations ipc_plan
| pddl_declarations heuristic_table
| /* empty */
;

pddl_domain:
TK_OPEN KW_DEFINE domain_name domain_elements TK_CLOSE
;

domain_elements:
domain_requires domain_elements
| domain_types domain_elements
| domain_constants domain_elements
| domain_predicates domain_elements
| domain_functions domain_elements
| domain_structure domain_elements
| reference_set domain_elements
| goal_spec domain_elements
| /* empty */
;

domain_name:
TK_OPEN KW_DOMAIN any_symbol TK_CLOSE
{
  domain_name = $3->text;
  if (current_file()) domain_file = strdup(current_file());
}
;

any_symbol:
TK_NEW_SYMBOL { $$ = $1; }
| TK_OBJ_SYMBOL { $$ = $1; }
| TK_TYPE_SYMBOL { $$ = $1; }
| TK_PRED_SYMBOL { $$ = $1; }
| TK_FUN_SYMBOL { $$ = $1; }
| TK_VAR_SYMBOL { $$ = $1; }
| TK_ACTION_SYMBOL { $$ = $1; }
| TK_MISC_SYMBOL { $$ = $1; }
| TK_PREFERENCE_SYMBOL { $$ = $1; }
;

action_symbol:
TK_NEW_SYMBOL { $$ = $1; }
| TK_ACTION_SYMBOL { $$ = $1; }
;

// requirement declarations

domain_requires:
TK_OPEN KW_REQS require_list TK_CLOSE
;

require_list:
require_list KW_TYPES
| require_list KW_CONSTRAINTS
| require_list TK_KEYWORD
| /* empty */
;

// predicate declarations

domain_predicates:
TK_OPEN KW_PREDS predicate_list TK_CLOSE
;

predicate_list:
predicate_decl predicate_list
| predicate_decl
;

predicate_decl:
TK_OPEN TK_NEW_SYMBOL
{
  current_param.clear()
}
typed_param_list TK_CLOSE
{
  PredicateSymbol* p = new PredicateSymbol($2->text);
  p->param = current_param;
  dom_predicates.append(p);
  clear_context(current_param);
  $2->val = p;
}
;

typed_param_list:
typed_param_list typed_param_sym_list TK_HYPHEN TK_TYPE_SYMBOL
{
  set_variable_type(current_param, (TypeSymbol*)$4->val);
}
| typed_param_list typed_param_sym_list TK_HYPHEN TK_OPEN KW_EITHER
{
  current_type_set.clear();
}
non_empty_type_name_list TK_CLOSE
{
  set_variable_type(current_param, current_type_set);
}
| typed_param_list typed_param_sym_list
{
  set_variable_type(current_param, dom_top_type);
}
| /* empty */
;

typed_param_sym_list:
typed_param_sym_list TK_NEW_VAR_SYMBOL
{
  $2->val = new VariableSymbol($2->text);
  current_param.append((VariableSymbol*)$2->val);
  if (trace_print_context) {
    std::cerr << "variable ";
    current_param[current_param.length() - 1]->print(std::cerr);
    std::cerr << " added to context (now "
	      << current_param.length() << " variables)"
	      << std::endl;
  }
}
| typed_param_sym_list TK_VAR_SYMBOL
{
  std::cerr << "error: variable ";
  ((VariableSymbol*)$2->val)->print(std::cerr);
  std::cerr << " redeclared" << std::endl;
  exit(255);
}
| TK_NEW_VAR_SYMBOL
{
  $1->val = new VariableSymbol($1->text);
  current_param.append((VariableSymbol*)$1->val);
  if (trace_print_context) {
    std::cerr << "variable ";
    current_param[current_param.length() - 1]->print(std::cerr);
    std::cerr << " added to context (now "
	      << current_param.length() << " variables)"
	      << std::endl;
  }
}
| TK_VAR_SYMBOL
{
  std::cerr << "error: variable ";
  ((VariableSymbol*)$1->val)->print(std::cerr);
  std::cerr << " redeclared" << std::endl;
  exit(255);
}
;

non_empty_type_name_list:
non_empty_type_name_list TK_TYPE_SYMBOL
{
  current_type_set.append((TypeSymbol*)$2->val);
}
| TK_TYPE_SYMBOL
{
  current_type_set.append((TypeSymbol*)$1->val);
}

// function declarations

domain_functions:
TK_OPEN KW_FUNS
{
  last_n_functions = dom_functions.length();
}
function_list TK_CLOSE
;

function_list:
function_decl_list TK_HYPHEN KW_NUMBER
{
  last_n_functions = dom_functions.length();
}
function_list
| function_decl_list TK_HYPHEN TK_TYPE_SYMBOL
{
  TypeSymbol* t = (TypeSymbol*)$3->val;
  for (HSPS::index_type k = last_n_functions; k < dom_functions.length(); k++){
    if (write_info) {
      std::cerr << "info: converting ";
      dom_functions[k]->print(std::cerr);
      std::cerr << " to object function with type " << t->print_name
		<< std::endl;
    }
    HSPS::PDDL_Base::ObjectFunctionSymbol* f =
      new HSPS::PDDL_Base::ObjectFunctionSymbol(dom_functions[k]->print_name);
    f->param = dom_functions[k]->param;
    f->sym_types.assign_value(t, 1);
    dom_object_functions.append(f);
    HSPS::StringTable::Cell* c =
      (HSPS::StringTable::Cell*)tab.find(f->print_name);
    if (c == 0) {
      std::cerr << "very bad error: function "
		<< dom_functions[k]->print_name
		<< " declared but not found in string table!"
		<< std::endl;
      exit(255);
    }
    c->val = f;
  }
  dom_functions.set_length(last_n_functions);
  // last_n_functions = dom_functions.length();
}
function_list
| function_decl_list TK_HYPHEN TK_OPEN KW_EITHER non_empty_type_name_list TK_CLOSE
{
  for (HSPS::index_type k = last_n_functions; k < dom_functions.length(); k++){
    if (write_info) {
      std::cerr << "info: converting ";
      dom_functions[k]->print(std::cerr);
      std::cerr << " to object function with type ";
      current_type_set.write_type(std::cerr);
      std::cerr << std::endl;
    }
    HSPS::PDDL_Base::ObjectFunctionSymbol* f =
      new HSPS::PDDL_Base::ObjectFunctionSymbol(dom_functions[k]->print_name);
    f->param = dom_functions[k]->param;
    f->sym_types.assign_copy(current_type_set);
    dom_object_functions.append(f);
    HSPS::StringTable::Cell* c =
      (HSPS::StringTable::Cell*)tab.find(f->print_name);
    if (c == 0) {
      std::cerr << "very bad error: function "
		<< dom_functions[k]->print_name
		<< " declared but not found in string table!"
		<< std::endl;
      exit(255);
    }
    c->val = f;
  }
  dom_functions.set_length(last_n_functions);
  // last_n_functions = dom_functions.length();
}
function_list
| function_decl_list
{
  last_n_functions = dom_functions.length();
}
| /* empty */
;

function_decl_list:
function_decl function_decl_list
| function_decl
;

function_decl:
TK_OPEN TK_NEW_SYMBOL
{
  current_param.clear();
}
typed_param_list TK_CLOSE
{
  FunctionSymbol* f = new FunctionSymbol($2->text);
  f->param = current_param;
  dom_functions.append(f);
  clear_context(current_param);
  $2->val = f;
}
;

// type declarations

domain_types:
TK_OPEN KW_TYPES
{
  current_type_set.clear();
}
typed_type_list TK_CLOSE
;

typed_type_list:
typed_type_list primitive_type_list TK_HYPHEN TK_TYPE_SYMBOL
{
  // set_type_type(dom_types, (TypeSymbol*)$4->val);
  for (HSPS::index_type k = 0; k < current_type_set.length(); k++)
    current_type_set[k]->sym_types.assign_value((TypeSymbol*)$4->val, 1);
  current_type_set.clear();
}
| typed_type_list primitive_type_list TK_HYPHEN TK_NEW_SYMBOL
{
  $4->val = new TypeSymbol($4->text);
  ((TypeSymbol*)$4->val)->sym_types.assign_value(dom_top_type, 1);
  // if (write_warnings)
  // std::cerr << "warning: assuming " << $4->text << " - object" << std::endl;
  // ((TypeSymbol*)$4->val)->sym_types.assign_value(dom_top_type, 1);
  // set_type_type(dom_types, (TypeSymbol*)$4->val);
  dom_types.append((TypeSymbol*)$4->val);
  for (HSPS::index_type k = 0; k < current_type_set.length(); k++)
    current_type_set[k]->sym_types.assign_value((TypeSymbol*)$4->val, 1);
  current_type_set.clear();
}
| typed_type_list primitive_type_list
{
  // set_type_type(dom_types, dom_top_type);
  for (HSPS::index_type k = 0; k < current_type_set.length(); k++)
    current_type_set[k]->sym_types.assign_value(dom_top_type, 1);
  current_type_set.clear();
}
| /* empty */
;

primitive_type_list:
primitive_type_list TK_TYPE_SYMBOL
{
  /* the type is already (implicitly) declared */
  current_type_set.append((TypeSymbol*)$2->val);
}
| primitive_type_list TK_NEW_SYMBOL
{
  $2->val = new TypeSymbol($2->text);
  dom_types.append((TypeSymbol*)$2->val);
  current_type_set.append((TypeSymbol*)$2->val);
}
| /* empty */
;

// constant declarations

domain_constants:
TK_OPEN KW_CONSTANTS typed_constant_list TK_CLOSE
| TK_OPEN KW_OBJECTS typed_constant_list TK_CLOSE
;

typed_constant_list:
typed_constant_list ne_constant_sym_list TK_HYPHEN TK_TYPE_SYMBOL
{
  set_constant_type(dom_constants, (TypeSymbol*)$4->val);
}
| typed_constant_list ne_constant_sym_list
{
  set_constant_type(dom_constants, dom_top_type);
}
| /* empty */
;

ne_constant_sym_list:
ne_constant_sym_list TK_NEW_SYMBOL
{
  $2->val = new Symbol($2->text);
  if (problem_name) {
    ((Symbol*)$2->val)->defined_in_problem = true;
  }
  dom_constants.append((Symbol*)$2->val);
}
| TK_NEW_SYMBOL
{
  $1->val = new Symbol($1->text);
  if (problem_name) {
    ((Symbol*)$1->val)->defined_in_problem = true;
  }
  dom_constants.append((Symbol*)$1->val);
}
| ne_constant_sym_list TK_OBJ_SYMBOL
{
  if (write_warnings) {
    std::cerr << "warning: redeclaration of constant " << $2->text
	      << " ignored" << std::endl;
  }
}
| TK_OBJ_SYMBOL
{
  if (write_warnings) {
    std::cerr << "warning: redeclaration of constant " << $1->text
	      << " ignored" << std::endl;
  }
}
;

// structure declarations

domain_structure:
action_declaration
| domain_invariant
| irrelevant_item
;

// structure declarations

action_declaration:
TK_OPEN KW_ACTION action_symbol
{
  dom_actions.append(new ActionSymbol($3->text));
}
action_elements TK_CLOSE
{
  // post-processing should be done on all actions after the complete
  // domain and problem have been read (calling PDDL_Base::post_process())
  clear_context(current_param);
  $3->val = dom_actions[dom_actions.length() - 1];
}
;

action_elements:
action_elements KW_ARGS TK_OPEN
{
  current_param.clear();
}
typed_param_list TK_CLOSE
{
  dom_actions[dom_actions.length() - 1]->param = current_param;
}
| action_elements KW_SET action_set_name
| action_elements KW_EFFECT action_effect
| action_elements KW_PRE action_condition
| action_elements KW_COND action_condition
| action_elements KW_DURATION action_duration
| action_elements action_expansion
| action_elements KW_ASSOC TK_STRING
{
  // std::cerr << "read assoc string: [" << $3 << "]" << std::endl;
  dom_actions[dom_actions.length() - 1]->assoc = $3;
}
| /* empty */
;

action_set_name:
TK_OPEN TK_NEW_SYMBOL
{
  SetSymbol* ssym = new SetSymbol($2->text);
  $2->val = ssym;
  partitions.append(ssym);
  SetName* s = new SetName(ssym);
  current_atom = s;
}
flat_atom_argument_list TK_CLOSE
{
  dom_actions[dom_actions.length() - 1]->part = (SetName*)current_atom;
}
| TK_OPEN TK_SET_SYMBOL
{
  SetName* s = new SetName((SetSymbol*)$2->val);
  current_atom = s;
}
flat_atom_argument_list TK_CLOSE
{
  dom_actions[dom_actions.length() - 1]->part = (SetName*)current_atom;
}
;

action_condition:
single_action_condition
| TK_OPEN KW_AND action_condition_list TK_CLOSE
;

action_condition_list:
action_condition_list single_action_condition
| single_action_condition
;

single_action_condition:
positive_atom_condition
| negative_atom_condition
| set_condition
| disjunctive_condition
| disjunctive_set_condition
| numeric_condition
{
  dom_actions[dom_actions.length() - 1]->num_pre.append($1);
}
;

timing_keyword:
KW_AT_START
{
  $$ = PDDL_Base::md_start;
}
| KW_AT_END
{
  $$ = PDDL_Base::md_end;
}
| KW_OVER_ALL
{
  $$ = PDDL_Base::md_all;
}
;

positive_atom_condition:
TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  ((Atom*)current_atom)->check();
  dom_actions[dom_actions.length() - 1]->pos_pre.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
}
| TK_OPEN TK_EQ atom_argument atom_argument TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$3->val;
  eq_atom->param[1] = (Symbol*)$4->val;
  dom_actions[dom_actions.length() - 1]->pos_pre.append(eq_atom);
}
| TK_OPEN timing_keyword TK_OPEN TK_PRED_SYMBOL /* timed case */
{
  current_atom = new Atom((PredicateSymbol*)$4->val, $2);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  dom_actions[dom_actions.length() - 1]->pos_pre.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
}
| TK_OPEN timing_keyword TK_OPEN TK_EQ atom_argument atom_argument
TK_CLOSE TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred, $2);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$5->val;
  eq_atom->param[1] = (Symbol*)$6->val;
  dom_actions[dom_actions.length() - 1]->pos_pre.append(eq_atom);
}
;

negative_atom_condition:
TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  dom_actions[dom_actions.length() - 1]->neg_pre.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
}
| TK_OPEN KW_NOT TK_OPEN TK_EQ atom_argument atom_argument TK_CLOSE TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$5->val;
  eq_atom->param[1] = (Symbol*)$6->val;
  dom_actions[dom_actions.length() - 1]->neg_pre.append(eq_atom);
}
| TK_OPEN timing_keyword TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$6->val, $2);
}
atom_argument_list TK_CLOSE TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  dom_actions[dom_actions.length() - 1]->neg_pre.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
}
| TK_OPEN timing_keyword TK_OPEN KW_NOT TK_OPEN TK_EQ
atom_argument atom_argument TK_CLOSE TK_CLOSE TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred, $2);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$7->val;
  eq_atom->param[1] = (Symbol*)$8->val;
  dom_actions[dom_actions.length() - 1]->neg_pre.append(eq_atom);
}
;

flat_atom_argument_list:
flat_atom_argument_list flat_atom_argument
{
  if (current_atom != 0) {
    current_atom->param.append((Symbol*)$2->val);
  }
}
| /* empty */
;

atom_argument_list:
atom_argument_list atom_argument
{
  if (current_atom != 0) {
    current_atom->param.append((Symbol*)$2->val);
  }
}
| /* empty */
;

flat_atom_argument:
TK_VAR_SYMBOL
{
  if ($1->val == 0) {
    log_error("undeclared variable in atom argument");
  }
  $$ = $1;
}
| TK_OBJ_SYMBOL
{
  $$ = $1;
}
;

functional_atom_argument:
TK_OPEN TK_OBJFUN_SYMBOL
{
  current_atom_stack.append(current_atom);
  current_atom = new FTerm((ObjectFunctionSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  ObjectFunctionSymbol* f = (ObjectFunctionSymbol*)$2->val;
  VariableSymbol* v =
    (VariableSymbol*)gensym(sym_variable, "?omsk", f->sym_types);
  v->binding = (FTerm*)current_atom;
  assert(current_atom_stack.length() > 0);
  current_atom = current_atom_stack[current_atom_stack.length() - 1];
  current_atom_stack.dec_length();
  HSPS::StringTable::Cell* c =
    (HSPS::StringTable::Cell*)tab.find(v->print_name);
  if (c == 0) {
    std::cerr << "very bad error: omsk symbol " << v->print_name
	      << " generated but not found in string table!"
	      << std::endl;
    exit(255);
  }
  $$ = c;
}

atom_argument:
flat_atom_argument
{
  $$ = $1;
}
| functional_atom_argument
{
  $$ = $1;
}

numeric_condition:
TK_OPEN relation_keyword d_expression d_expression TK_CLOSE
{
  $$ = new Relation($2, $3, $4);
}
| TK_OPEN timing_keyword TK_OPEN relation_keyword d_expression d_expression
  TK_CLOSE TK_CLOSE
{
  $$ = new Relation($4, $2, $5, $6);
}
;

relation_keyword:
TK_GREATER
{
  $$ = rel_greater;
}
| TK_GREATEQ
{
  $$ = rel_greater_equal;
}
| TK_LESS
{
  $$ = rel_less;
}
| TK_LESSEQ
{
  $$ = rel_less_equal;
}
| TK_EQ
{
  $$ = rel_equal;
}
;

d_expression:
TK_OPEN TK_HYPHEN d_expression TK_CLOSE %prec TK_UMINUS
{
  $$ = new BinaryExpression(exp_sub, new ConstantExpression(0), $3);
}
| TK_OPEN TK_PLUS d_expression d_sum TK_CLOSE
{
  $$ = new BinaryExpression(exp_add, $3, $4);
}
| TK_OPEN TK_HYPHEN d_expression d_expression TK_CLOSE
{
  $$ = new BinaryExpression(exp_sub, $3, $4);
}
| TK_OPEN TK_MUL d_expression d_product TK_CLOSE
{
  $$ = new BinaryExpression(exp_mul, $3, $4);
}
| TK_OPEN TK_DIV d_expression d_expression TK_CLOSE
{
  $$ = new BinaryExpression(exp_div, $3, $4);
}
| d_expression TK_DIV d_expression
{
  $$ = new BinaryExpression(exp_div, $1, $3);
}
| TK_INT
{
  $$ = new ConstantExpression($1);
}
| TK_FLOAT
{
  $$ = new ConstantExpression(NN_TO_N($1));
}
| TK_OPEN KW_TOTAL_TIME TK_CLOSE
{
  $$ = new TimeExpression();
}
| TK_OPEN KW_VIOLATED TK_PREFERENCE_SYMBOL TK_CLOSE
{
  $$ = new PreferenceExpression((Symbol*)$3->val);
}
| d_function
{
  $$ = $1;
}
;

d_sum:
d_expression
{
  $$ = $1;
}
| d_expression d_sum
{
  $$ = new BinaryExpression(exp_add, $1, $2);
}
;

d_product:
d_expression
{
  $$ = $1;
}
| d_expression d_product
{
  $$ = new BinaryExpression(exp_mul, $1, $2);
}
;

d_function:
TK_OPEN TK_FUN_SYMBOL d_argument_list TK_CLOSE
{
  $$ = new FunctionExpression((FunctionSymbol*)$2->val, $3);
}
| TK_FUN_SYMBOL
{
  $$ = new FunctionExpression((FunctionSymbol*)$1->val, 0);
}
;

d_argument_list:
TK_VAR_SYMBOL d_argument_list
{
  $$ = new ListExpression((VariableSymbol*)$1->val, $2);
}
| TK_OBJ_SYMBOL d_argument_list
{
  $$ = new ListExpression((Symbol*)$1->val, $2);
}
| /* empty */
{
  $$ = 0;
}
;

set_condition:
TK_OPEN KW_SETOF
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
}
req_vars_spec opt_context_spec TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$7->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append((Atom*)current_atom);
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
  ((Atom*)current_atom)->pred->pos_pre = true;
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
}
| TK_OPEN KW_FORALL TK_OPEN
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
}
typed_param_list TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
}
universal_condition_body TK_CLOSE
{
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
}
| TK_OPEN KW_IMPLY
{
  current_context = new SetOf();
}
one_or_more_context_atoms one_or_more_condition_atoms TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
}
| TK_OPEN timing_keyword TK_OPEN KW_SETOF /* timed case */
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
}
req_vars_spec opt_context_spec TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$9->val);
}
atom_argument_list TK_CLOSE TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append((Atom*)current_atom);
  s->set_mode($2);
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
  ((Atom*)current_atom)->pred->pos_pre = true;
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
}
| TK_OPEN timing_keyword TK_OPEN KW_FORALL TK_OPEN
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
}
typed_param_list TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
}
universal_condition_body TK_CLOSE TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  s->set_mode($2);
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
}
| TK_OPEN timing_keyword TK_OPEN KW_IMPLY
{
  current_context = new SetOf();
}
one_or_more_context_atoms one_or_more_condition_atoms TK_CLOSE TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  s->set_mode($2);
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
}
;

universal_condition_body:
TK_OPEN KW_IMPLY one_or_more_context_atoms one_or_more_condition_atoms TK_CLOSE
| one_or_more_condition_atoms
;

one_or_more_condition_atoms:
TK_OPEN KW_AND quantified_condition_atom_list TK_CLOSE
| quantified_condition_atom
;

quantified_condition_atom_list:
quantified_condition_atom quantified_condition_atom_list
| quantified_condition_atom
;

one_or_more_context_atoms:
TK_OPEN KW_AND context_list TK_CLOSE
| context_atom
;

quantified_condition_atom:
TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
}
| TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->neg_atoms.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
}
| TK_OPEN TK_EQ atom_argument atom_argument TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$3->val;
  eq_atom->param[1] = (Symbol*)$4->val;
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append(eq_atom);
}
| TK_OPEN KW_NOT TK_OPEN TK_EQ atom_argument atom_argument TK_CLOSE TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$5->val;
  eq_atom->param[1] = (Symbol*)$6->val;
  SetOf* s = (SetOf*)current_context;
  s->neg_atoms.append(eq_atom);
}
/* timed case (timing keyword on atom inside quantification/condition)
   - this is NOT legal PDDL
| TK_OPEN timing_keyword TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val, $2);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
}
| TK_OPEN timing_keyword TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$6->val, $2);
}
atom_argument_list TK_CLOSE TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->atom = (Atom*)current_atom;
  s->neg = true;
  ((Atom*)current_atom)->pred->neg_pre = true;
}
*/
;

disjunctive_condition:
TK_OPEN KW_OR
{
  current_context = new SetOf();
}
disjunction_list TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  dom_actions[dom_actions.length() - 1]->dis_pre.append(s);
  current_context = 0;
}
;

disjunctive_set_condition:
| TK_OPEN KW_EXISTS TK_OPEN
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
}
typed_param_list TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
  dom_actions[dom_actions.length() - 1]->dis_pre.append(s);
}
existential_condition_body TK_CLOSE
{
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
}
;

existential_condition_body:
TK_OPEN KW_AND context_list TK_OPEN KW_OR disjunction_list TK_CLOSE TK_CLOSE
| TK_OPEN KW_OR disjunction_list TK_CLOSE
| disjunction_atom
;

disjunction_list:
disjunction_atom disjunction_list
| disjunction_atom
;

disjunction_atom:
TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  ((Atom*)current_atom)->check();
  ((SetOf*)current_context)->pos_atoms.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
}
| TK_OPEN TK_EQ atom_argument atom_argument TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$3->val;
  eq_atom->param[1] = (Symbol*)$4->val;
  ((SetOf*)current_context)->pos_atoms.append(eq_atom);
}
| TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  ((SetOf*)current_context)->neg_atoms.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
}
| TK_OPEN KW_NOT TK_OPEN TK_EQ atom_argument atom_argument TK_CLOSE TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$5->val;
  eq_atom->param[1] = (Symbol*)$6->val;
  ((SetOf*)current_context)->neg_atoms.append(eq_atom);
}
;

action_effect:
single_action_effect
| TK_OPEN KW_AND action_effect_list TK_CLOSE
;

action_effect_list:
single_action_effect action_effect_list
| single_action_effect
;

single_action_effect:
TK_OPEN KW_FORALL TK_OPEN
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
}
typed_param_list TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
}
quantified_effect_body TK_CLOSE
{
  dom_actions[dom_actions.length() - 1]->set_eff.append((SetOf*)current_context);
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
}
| TK_OPEN KW_WHEN
{
  current_context = new SetOf();
}
effect_conditions one_or_more_atomic_effects TK_CLOSE
{
  dom_actions[dom_actions.length() - 1]->set_eff.append((SetOf*)current_context);
  current_context = 0;
}
| TK_OPEN timing_keyword TK_OPEN KW_FORALL TK_OPEN /* timed case */
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
}
typed_param_list TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
}
quantified_effect_body TK_CLOSE TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  s->set_mode($2);
  dom_actions[dom_actions.length() - 1]->set_eff.append(s);
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
}
| TK_OPEN timing_keyword TK_OPEN KW_WHEN
{
  current_context = new SetOf();
}
effect_conditions one_or_more_atomic_effects TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  s->set_mode($2);
  dom_actions[dom_actions.length() - 1]->set_eff.append(s);
  current_context = 0;
}
| atomic_effect
| numeric_effect
;

quantified_effect_body:
TK_OPEN KW_WHEN effect_conditions one_or_more_atomic_effects TK_CLOSE
| one_or_more_atomic_effects
;

one_or_more_atomic_effects:
TK_OPEN KW_AND atomic_effect_list TK_CLOSE
| atomic_effect
;

effect_conditions:
context_atom
| TK_OPEN KW_AND context_list TK_CLOSE
;

atomic_effect_list:
atomic_effect atomic_effect_list
| atomic_effect
;

atomic_effect:
positive_atom_effect
| negative_atom_effect
;

positive_atom_effect:
TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$2->val);
  ((PredicateSymbol*)$2->val)->modded = true;
}
atom_argument_list TK_CLOSE
{
  ((Atom*)current_atom)->check();
  if (current_context != 0) {
    ((SetOf*)current_context)->pos_atoms.append((Atom*)current_atom);
  }
  else {
    dom_actions[dom_actions.length() - 1]->adds.append((Atom*)current_atom);
  }
}
| TK_OPEN KW_ASSIGN TK_OPEN TK_OBJFUN_SYMBOL
{
  current_atom = new FTerm((ObjectFunctionSymbol*)$4->val);
}
atom_argument_list TK_CLOSE object_assignment_value TK_CLOSE
{
  FTerm* ft = (FTerm*)current_atom;
  ft->fun->modded = true;
  VariableSymbol* v =
    (VariableSymbol*)gensym(sym_variable,"?omsk",ft->fun->sym_types);
  v->binding = ft;
  Atom* a = new Atom(dom_assign_pred);
  a->param.set_length(2);
  a->param[0] = v;
  a->param[1] = (Symbol*)$8->val;
  if (current_context != 0) {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: object function assignment ";
      a->print(std::cerr);
      std::cerr << " in quantified/conditional effect ignored"
		<< std::endl;
    }
    if (!best_effort) exit(1);
  }
  else {
    dom_actions[dom_actions.length() - 1]->adds.append(a);
  }
}
| TK_OPEN timing_keyword TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val, $2);
  ((PredicateSymbol*)$4->val)->modded = true;
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  if (current_context != 0) {
    ((SetOf*)current_context)->pos_atoms.append((Atom*)current_atom);
  }
  else {
    dom_actions[dom_actions.length() - 1]->adds.append((Atom*)current_atom);
  }
}
;

object_assignment_value:
atom_argument
{
  $$ = $1;
}
| KW_UNDEFINED
{
  $$ = (HSPS::StringTable::Cell*)tab.find("undefined");
  assert($$ != 0);
}

negative_atom_effect:
TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val);
  ((PredicateSymbol*)$4->val)->modded = true;
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  if (current_context != 0) {
    ((SetOf*)current_context)->neg_atoms.append((Atom*)current_atom);
  }
  else {
    dom_actions[dom_actions.length() - 1]->dels.append((Atom*)current_atom);
  }
}
| TK_OPEN timing_keyword TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$6->val, $2);
  ((PredicateSymbol*)$6->val)->modded = true;
}
atom_argument_list TK_CLOSE TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  if (current_context != 0) {
    ((SetOf*)current_context)->neg_atoms.append((Atom*)current_atom);
  }
  else {
    dom_actions[dom_actions.length() - 1]->dels.append((Atom*)current_atom);
  }
}
;

numeric_effect:
TK_OPEN KW_INCREASE TK_OPEN TK_FUN_SYMBOL
{
  current_atom = new FChangeAtom((FunctionSymbol*)$4->val);
}
atom_argument_list TK_CLOSE d_expression TK_CLOSE
{
  ((FChangeAtom*)current_atom)->val = $8;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!$8->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->incs.append((FChangeAtom*)current_atom);
}
| TK_OPEN KW_DECREASE TK_OPEN TK_FUN_SYMBOL
{
  current_atom = new FChangeAtom((FunctionSymbol*)$4->val);
}
atom_argument_list TK_CLOSE d_expression TK_CLOSE
{
  ((FChangeAtom*)current_atom)->val = $8;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!$8->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->decs.append((FChangeAtom*)current_atom);
}
| TK_OPEN KW_ASSIGN TK_OPEN TK_FUN_SYMBOL
{
  current_atom = new FChangeAtom((FunctionSymbol*)$4->val);
}
atom_argument_list TK_CLOSE d_expression TK_CLOSE
{
  ((FChangeAtom*)current_atom)->val = $8;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!$8->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->fass.append((FChangeAtom*)current_atom);
}
| TK_OPEN timing_keyword TK_OPEN KW_INCREASE TK_OPEN TK_FUN_SYMBOL
{
  current_atom = new FChangeAtom((FunctionSymbol*)$6->val, $2);
}
atom_argument_list TK_CLOSE d_expression TK_CLOSE TK_CLOSE
{
  ((FChangeAtom*)current_atom)->val = $10;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!$10->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->incs.append((FChangeAtom*)current_atom);
}
| TK_OPEN timing_keyword TK_OPEN KW_DECREASE TK_OPEN TK_FUN_SYMBOL
{
  current_atom = new FChangeAtom((FunctionSymbol*)$6->val, $2);
}
atom_argument_list TK_CLOSE d_expression TK_CLOSE TK_CLOSE
{
  ((FChangeAtom*)current_atom)->val = $10;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!$10->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->decs.append((FChangeAtom*)current_atom);
}
| TK_OPEN timing_keyword TK_OPEN KW_ASSIGN TK_OPEN TK_FUN_SYMBOL
{
  current_atom = new FChangeAtom((FunctionSymbol*)$6->val, $2);
}
atom_argument_list TK_CLOSE d_expression TK_CLOSE TK_CLOSE
{
  ((FChangeAtom*)current_atom)->val = $10;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!$10->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->fass.append((FChangeAtom*)current_atom);
}
;

action_duration:
action_duration_exp
| TK_OPEN KW_AND action_duration_list TK_CLOSE
;

action_duration_list:
action_duration_exp
| action_duration_exp action_duration_list
;

action_duration_exp:
action_exact_duration
| action_min_duration
| action_max_duration
;

action_exact_duration:
TK_OPEN TK_EQ KW_DURATION_VAR d_expression TK_CLOSE
{
  dom_actions[dom_actions.length() - 1]->dmin = $4;
  dom_actions[dom_actions.length() - 1]->dmax = $4;
}
| d_expression
{
  dom_actions[dom_actions.length() - 1]->dmin = $1;
  dom_actions[dom_actions.length() - 1]->dmax = $1;
}
;

action_min_duration:
TK_OPEN less_or_lesseq KW_DURATION_VAR d_expression TK_CLOSE
{
  dom_actions[dom_actions.length() - 1]->dmax = $4;
}
;

less_or_lesseq:
TK_LESS
| TK_LESSEQ
;

action_max_duration:
TK_OPEN greater_or_greatereq KW_DURATION_VAR d_expression TK_CLOSE
{
  dom_actions[dom_actions.length() - 1]->dmin = $4;
}
;

greater_or_greatereq:
TK_GREATER
| TK_GREATEQ
;

action_expansion:
TK_OPEN KW_EXPANSION
{
  current_context = new SequentialTaskNet();
  stored_n_param.append(current_param.length());
  if (trace_print_context) {
    std::cerr << "pushed context (" << current_param.length() << " variables)"
	      << std::endl;
  }
}
opt_vars_spec opt_context_and_precondition_spec
KW_TASKS TK_OPEN task_list TK_CLOSE TK_CLOSE
{
  SequentialTaskNet* n = (SequentialTaskNet*)current_context;
  n->abs_act = dom_actions[dom_actions.length() - 1];
  dom_actions[dom_actions.length() - 1]->exps.append(n);
  if (trace_print_context) {
    std::cerr << "poping context from "
	      << current_param.length()
	      << " to "
	      << stored_n_param[stored_n_param.length() - 1]
	      << " variables..." << std::endl;
  }
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
}
;

task_list:
task task_list
| /* empty */
;

task:
TK_OPEN TK_ACTION_SYMBOL
{
  current_atom = new Reference((ActionSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  Reference* ref = (Reference*)current_atom;
  // ActionSymbol* act = (ActionSymbol*)$2->val;
  // act->refs[act->n_refs] = ref;
  // act->n_refs += 1;
  SequentialTaskNet* task_net = (SequentialTaskNet*)current_context;
  task_net->tasks.append(ref);
}
;

// problem definition

pddl_problem:
TK_OPEN KW_DEFINE problem_name problem_elements TK_CLOSE
;

problem_name:
TK_OPEN KW_PROBLEM any_symbol TK_CLOSE
{
  problem_name = $3->text;
  if (current_file()) problem_file = strdup(current_file());
}
;

problem_elements:
problem_elements TK_OPEN KW_FORDOMAIN any_symbol TK_CLOSE
| problem_elements domain_requires
| problem_elements domain_constants
| problem_elements initial_state
| problem_elements goal_spec
| problem_elements metric_spec
| problem_elements length_spec
| problem_elements problem_invariant
| problem_elements irrelevant_item
| problem_elements reference_set
| /* empty */
;

initial_state:
TK_OPEN KW_INIT init_elements TK_CLOSE
;

init_elements:
init_elements init_function
| init_elements init_object_function
| init_elements init_atom
| /* empty */
;

init_atom:
TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  ((Atom*)current_atom)->check();
  PredicateSymbol* p = (PredicateSymbol*)$2->val;
  if (p->param.length() != current_atom->param.length()) {
    log_error("wrong number of arguments for predicate in (:init ...");
  }
  ((Atom*)current_atom)->insert(p->init);
  current_atom->at_time = 0;
  dom_init.append((Atom*)current_atom);
}
;

init_object_function:
TK_OPEN TK_EQ TK_OPEN TK_OBJFUN_SYMBOL
{
  current_atom = new OInitAtom((ObjectFunctionSymbol*)$4->val);
}
atom_argument_list TK_CLOSE TK_OBJ_SYMBOL TK_CLOSE
{
  ObjectFunctionSymbol* f = (ObjectFunctionSymbol*)$4->val;
  if (f->param.length() != current_atom->param.length()) {
    log_error("wrong number of arguments for object function in (:init ...");
  }
  ((OInitAtom*)current_atom)->val = (Symbol*)$8->val;
  current_atom->at_time = 0;
  ((OInitAtom*)current_atom)->insert(f->init);
  dom_obj_init.append((OInitAtom*)current_atom);
}

init_function:
TK_OPEN TK_EQ TK_OPEN TK_FUN_SYMBOL
{
  current_atom = new FInitAtom((FunctionSymbol*)$4->val);
}
atom_argument_list TK_CLOSE numeric_value TK_CLOSE
{
  FunctionSymbol* f = (FunctionSymbol*)$4->val;
  if (f->param.length() != current_atom->param.length()) {
    log_error("wrong number of arguments for function in (:init ...");
  }
  ((FInitAtom*)current_atom)->val = NN_TO_N($8);
  if (!INTEGRAL(((FInitAtom*)current_atom)->val))
    ((FInitAtom*)current_atom)->fun->integral = false;
  ((FInitAtom*)current_atom)->insert(f->init);
  current_atom->at_time = 0;
  dom_fun_init.append((FInitAtom*)current_atom);
}
| TK_OPEN TK_EQ TK_FUN_SYMBOL numeric_value TK_CLOSE
{
  FunctionSymbol* f = (FunctionSymbol*)$3->val;
  current_atom = new FInitAtom((FunctionSymbol*)$3->val);
  if (f->param.length() != 0) {
    log_error("wrong number of arguments for function in (:init ...");
  }
  ((FInitAtom*)current_atom)->val = NN_TO_N($4);
  if (!INTEGRAL(((FInitAtom*)current_atom)->val))
    ((FInitAtom*)current_atom)->fun->integral = false;
  current_atom->at_time = 0;
  ((FInitAtom*)current_atom)->insert(f->init);
  dom_fun_init.append((FInitAtom*)current_atom);
}
;

goal_spec:
TK_OPEN KW_GOAL single_goal_spec TK_CLOSE
| TK_OPEN KW_GOAL TK_OPEN KW_AND goal_spec_list TK_CLOSE TK_CLOSE
| TK_OPEN KW_CONSTRAINTS single_goal_spec TK_CLOSE
| TK_OPEN KW_CONSTRAINTS TK_OPEN KW_AND goal_spec_list TK_CLOSE TK_CLOSE
;

single_goal_spec:
new_single_goal
{
  dom_goals.append($1);
}
| TK_OPEN KW_PREFERENCE TK_NEW_SYMBOL new_goal TK_CLOSE
{
  Symbol* name = new Symbol(sym_preference, $3->text);
  $3->val = name;
  dom_preferences.append(new Preference(name, $4));
}
;

goal_spec_list:
single_goal_spec goal_spec_list
| /* empty */
;

new_goal:
new_single_goal
{
  $$ = $1;
}
| TK_OPEN KW_AND
{
  current_goal.append(new ConjunctiveGoal());
}
new_goal_list TK_CLOSE
{
  assert(current_goal.length() > 0);
  $$ = current_goal[current_goal.length() - 1];
  current_goal.dec_length();
}
| TK_OPEN KW_OR
{
  current_goal.append(new DisjunctiveGoal());
}
new_goal_list TK_CLOSE
{
  assert(current_goal.length() > 0);
  $$ = current_goal[current_goal.length() - 1];
  current_goal.dec_length();
}
;

new_goal_list:
new_single_goal new_goal_list
{
  assert(current_goal.length() > 0);
  ConjunctiveGoal* cg = current_goal[current_goal.length() - 1];
  assert(cg != 0);
  cg->goals.append($1);
}
| /* empty */
;

new_single_goal:
TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  ((Atom*)current_atom)->check();
  $$ = new AtomicGoal((Atom*)current_atom, false);
  ((Atom*)current_atom)->pred->pos_pre = true;
  ((Atom*)current_atom)->insert(((Atom*)current_atom)->pred->pos_goal);
}
| TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  $$ = new AtomicGoal((Atom*)current_atom, true);
  ((Atom*)current_atom)->pred->neg_pre = true;
  ((Atom*)current_atom)->insert(((Atom*)current_atom)->pred->neg_goal);
}
| TK_OPEN TK_EQ atom_argument atom_argument TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$3->val;
  eq_atom->param[1] = (Symbol*)$4->val;
  $$ = new AtomicGoal(eq_atom, false);
}
| numeric_condition
{
  $$ = new NumericGoal($1);
}
| TK_OPEN KW_ALWAYS new_goal TK_CLOSE
{
  $$ = new SimpleSequenceGoal(goal_always, $3);
}
| TK_OPEN KW_SOMETIME new_goal TK_CLOSE
{
  $$ = new SimpleSequenceGoal(goal_sometime, $3);
}
| TK_OPEN KW_AT_MOST_ONCE new_goal TK_CLOSE
{
  $$ = new SimpleSequenceGoal(goal_at_most_once, $3);
}
| TK_OPEN KW_SOMETIME_BEFORE new_goal new_goal TK_CLOSE
{
  $$ = new TriggeredSequenceGoal(goal_sometime_before, $3, $4);
}
| TK_OPEN KW_SOMETIME_AFTER new_goal new_goal TK_CLOSE
{
  $$ = new TriggeredSequenceGoal(goal_sometime_after, $3, $4);
}
| TK_OPEN KW_WITHIN numeric_value new_goal TK_CLOSE
{
  $$ = new DeadlineGoal($3, $4);
}
| TK_OPEN KW_ALWAYS_WITHIN numeric_value new_goal new_goal TK_CLOSE
{
  $$ = new TriggeredDeadlineGoal($4, $3, $5);
}
;

// goal_task_list:
// KW_TASKS TK_OPEN
// {
//   current_context = new SequentialTaskNet();
//   stored_n_param = current_n_param;
//   if (trace_print_context) {
//     std::cerr << "pushed context (" << current_n_param << " variables)"
// 	      << std::endl;
//   }
// }
// task_list TK_CLOSE
// {
//   SequentialTaskNet* n = (SequentialTaskNet*)current_context;
//   for (HSPS::index_type k = 0; k < n->n_tasks; k++)
//     goal_tasks[k] = n->tasks[k];
//   n_goal_tasks = n->n_tasks;
//   delete n;
//   if (trace_print_context) {
//     std::cerr << "poping context from "
// 	      << current_n_param
// 	      << " to "
// 	      << stored_n_param
// 	      << " variables..." << std::endl;
//   }
//   clear_context(current_param, stored_n_param, current_n_param);
//   current_n_param = stored_n_param;
//   current_context = 0;
// }
// ;

metric_spec:
TK_OPEN KW_METRIC metric_keyword metric_expression TK_CLOSE
;

metric_keyword:
KW_MINIMIZE
{
  if (metric_type != metric_none) {
    if (write_warnings) {
      std::cerr << "warning: multiple :metric expressions - overwriting previous definition" << std::endl;
    }
  }
  metric_type = metric_minimize;
}
| KW_MAXIMIZE
{
  if (metric_type != metric_none) {
    if (write_warnings) {
      std::cerr << "warning: multiple :metric expressions - overwriting previous definition" << std::endl;
    }
  }
  metric_type = metric_maximize;
}
;

metric_expression:
d_expression
{
  if ($1->exp_class == exp_time) {
    metric = 0;
    metric_type = metric_makespan;
    $$ = 0;
  }
  else {
    metric = $1;
    $$ = $1;
  }
}
;

length_spec:
TK_OPEN KW_LENGTH KW_SERIAL TK_INT TK_CLOSE
{
  serial_length = I_TO_N($4);
}
| TK_OPEN KW_LENGTH KW_PARALLEL TK_INT TK_CLOSE
{
  parallel_length = I_TO_N($4);
}
;

/*
numeric_value:
TK_INT { $$ = N_TO_NN(I_TO_N($1)); }
| TK_FLOAT { $$ = $1; }
| KW_INF { $$ = POS_INF; }
*/

numeric_value:
TK_INT { $$ = N_TO_NN($1); }
| TK_INT TK_DIV TK_INT { $$ = N_TO_NN(R_TO_N($1,$3)); }
| TK_FLOAT { $$ = $1; }
| KW_INF { $$ = POS_INF; }


// DKEL invariants

domain_invariant:
TK_OPEN KW_INVARIANT
{
  current_item = new DKEL_Item(":invariant");
  current_context = current_item;
}
dkel_element_list domain_invariant_body
;

domain_invariant_body:
KW_SET_CONSTRAINT
{
  dom_sc_invariants.append(new SetConstraint(current_item));
}
TK_OPEN set_constraint_type TK_INT invariant_set TK_CLOSE TK_CLOSE
{
  dom_sc_invariants[dom_sc_invariants.length() - 1]->sc_type = $4;
  dom_sc_invariants[dom_sc_invariants.length() - 1]->sc_count = $5;
  clear_context(current_param);
  current_context = 0;
}
| KW_FORMULA fol_formula TK_CLOSE
{
  dom_f_invariants.append(new InvariantFormula(current_item, $2));
  clear_context(current_param);
  current_context = 0;
}
;

fol_formula:
KW_FALSE
{
  $$ = new Formula(fc_false);
}
| KW_TRUE
{
  $$ = new Formula(fc_true);
}
| TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$2->val);
}
flat_atom_argument_list TK_CLOSE
{
  ((Atom*)current_atom)->check();
  $$ = new AFormula((Atom*)current_atom);
}
| TK_OPEN TK_EQ flat_atom_argument flat_atom_argument TK_CLOSE
{
  $$ = new EqFormula((Symbol*)$3->val, (Symbol*)$4->val);
}
| TK_OPEN KW_NOT fol_formula TK_CLOSE
{
  $$ = new NFormula($3);
}
| TK_OPEN KW_AND fol_formula_list TK_CLOSE
{
  $$ = $3;
  $$->fc = fc_conjunction;
}
| TK_OPEN KW_OR fol_formula_list TK_CLOSE
{
  $$ = $3;
  $$->fc = fc_conjunction;
}
| TK_OPEN KW_IMPLY fol_formula fol_formula TK_CLOSE
{
  $$ = new BFormula(fc_implication, $3, $4);
}
| TK_OPEN KW_IFF fol_formula fol_formula TK_CLOSE
{
  $$ = new BFormula(fc_equivalence, $3, $4);
}
| TK_OPEN KW_FORALL TK_OPEN
{
  stored_n_param.append(current_param.length());
}
typed_param_list TK_CLOSE fol_formula TK_CLOSE
{
  QFormula* qf = new QFormula(fc_universal, $7);
  assert(stored_n_param.length() > 0);
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    qf->vars.append(current_param[k]);
  }
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  $$ = qf;
}
| TK_OPEN KW_EXISTS TK_OPEN
{
  stored_n_param.append(current_param.length());
}
typed_param_list TK_CLOSE fol_formula TK_CLOSE
{
  QFormula* qf = new QFormula(fc_existential, $7);
  assert(stored_n_param.length() > 0);
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    qf->vars.append(current_param[k]);
  }
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  $$ = qf;
}
;

fol_formula_list:
fol_formula_list fol_formula
{
  ((CFormula*)$$)->add($2);
}
| fol_formula
{
  $$ = new CFormula(fc_list);
}
;

irrelevant_item:
TK_OPEN KW_IRRELEVANT
{
  current_item = new IrrelevantItem();
  if (problem_name) current_item->defined_in_problem = true;
  current_context = current_item;
}
dkel_element_list irrelevant_item_content
;

irrelevant_item_content:
irrelevant_action
| irrelevant_atom
;

irrelevant_action:
KW_ACTION TK_OPEN TK_ACTION_SYMBOL
{
  current_atom = new Reference((ActionSymbol*)$3->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  IrrelevantItem* item = (IrrelevantItem*)current_item;
  item->entity = (Reference*)current_atom;
  dom_irrelevant.append(item);
  ActionSymbol* act = (ActionSymbol*)$3->val;
  act->irr_ins.append(item);
  clear_context(current_param);
  current_context = 0;
}
;

irrelevant_atom:
KW_FACT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Reference((PredicateSymbol*)$3->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  IrrelevantItem* item = (IrrelevantItem*)current_item;
  item->entity = (Reference*)current_atom;
  dom_irrelevant.append(item);
  PredicateSymbol* pred = (PredicateSymbol*)$3->val;
  pred->irr_ins.append(item);
  clear_context(current_param);
  current_context = 0;
}
;

problem_invariant:
TK_OPEN KW_INVARIANT
{
  current_item = new DKEL_Item(":invariant");
  current_item->defined_in_problem = true;
  current_context = current_item;
}
dkel_element_list problem_invariant_body
;

problem_invariant_body:
KW_SET_CONSTRAINT 
{
  dom_sc_invariants.append(new SetConstraint(current_item));
}
TK_OPEN set_constraint_type TK_INT invariant_set TK_CLOSE TK_CLOSE
{
  dom_sc_invariants[dom_sc_invariants.length() - 1]->sc_type = $4;
  dom_sc_invariants[dom_sc_invariants.length() - 1]->sc_count = $5;
  clear_context(current_param);
  current_context = 0;
}
| KW_FORMULA fol_formula TK_CLOSE
{
  dom_f_invariants.append(new InvariantFormula(current_item, $2));
  clear_context(current_param);
  current_context = 0;
}
;

dkel_element_list:
dkel_element_list dkel_tag_spec
| dkel_element_list dkel_name_spec
| dkel_element_list dkel_vars_spec
| dkel_element_list dkel_context_spec
| /* empty */
;

dkel_tag_spec:
KW_TAG any_symbol
{
  current_item->item_tags.insert($2->text);
}
;

dkel_name_spec:
KW_NAME TK_NEW_SYMBOL
{
  $2->val = new Symbol(sym_misc, $2->text);
  current_item->name = (Symbol*)$2->val;
  current_item->item_tags.insert($2->text);
}
| KW_NAME TK_MISC_SYMBOL
{
  current_item->name = (Symbol*)$2->val;
  current_item->item_tags.insert($2->text);
}
;

dkel_vars_spec:
KW_VARS TK_OPEN
{
  current_param.clear();
}
typed_param_list TK_CLOSE
{
  current_context->param = current_param;
}
;

dkel_context_spec:
KW_CONTEXT context_atom
| KW_CONTEXT TK_OPEN KW_AND context_list TK_CLOSE
;

req_vars_spec:
KW_VARS TK_OPEN typed_param_list TK_CLOSE
{
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    current_context->param.append(current_param[k]);
  }
}
;

opt_vars_spec:
req_vars_spec
| /* empty */
;

opt_context_and_precondition_spec:
KW_CONTEXT context_atom opt_precondition_spec
| KW_CONTEXT TK_OPEN KW_AND context_list TK_CLOSE opt_precondition_spec
| KW_PRE context_atom opt_context_spec
| KW_PRE TK_OPEN KW_AND context_list TK_CLOSE opt_context_spec
| KW_COND context_atom opt_context_spec
| KW_COND TK_OPEN KW_AND context_list TK_CLOSE opt_context_spec
| /* empty */
;

opt_context_spec:
KW_CONTEXT context_atom
| KW_CONTEXT TK_OPEN KW_AND context_list TK_CLOSE
| /* empty */
;

opt_precondition_spec:
KW_PRE context_atom
| KW_PRE TK_OPEN KW_AND context_list TK_CLOSE
| KW_COND context_atom
| KW_COND TK_OPEN KW_AND context_list TK_CLOSE
| /* empty */
;

context_list:
context_list context_atom
| context_atom
;

context_atom:
positive_context_atom
| negative_context_atom
| positive_context_goal_atom
| negative_context_goal_atom
| type_constraint_atom
;

positive_context_atom:
TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->pos_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
}
/* timed case - note: timing info on context atoms IS IGNORED */
| TK_OPEN timing_keyword TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->pos_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
}
| TK_OPEN KW_INIT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val, md_init);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->pos_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
}
| TK_OPEN TK_EQ atom_argument atom_argument TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$3->val;
  eq_atom->param[1] = (Symbol*)$4->val;
  current_context->pos_con.append(eq_atom);
}
;

negative_context_atom:
TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
}
/* timed case - note: timing info on context atoms IS IGNORED */
| TK_OPEN timing_keyword TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$6->val);
}
atom_argument_list TK_CLOSE TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
}
| TK_OPEN KW_INIT TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$6->val, md_init);
}
atom_argument_list TK_CLOSE TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
}
| TK_OPEN KW_NOT TK_OPEN KW_INIT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$6->val, md_init);
}
atom_argument_list TK_CLOSE TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
}
| TK_OPEN KW_NOT TK_OPEN TK_EQ atom_argument atom_argument TK_CLOSE TK_CLOSE
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)$5->val;
  eq_atom->param[1] = (Symbol*)$6->val;
  current_context->neg_con.append(eq_atom);
}
;

positive_context_goal_atom:
TK_OPEN KW_GOAL TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val, md_pos_goal);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->pos_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
}
| TK_OPEN KW_GOAL TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$6->val, md_neg_goal);
}
atom_argument_list TK_CLOSE TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->pos_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
}
;

negative_context_goal_atom:
TK_OPEN KW_NOT TK_OPEN KW_GOAL TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$6->val, md_pos_goal);
}
atom_argument_list TK_CLOSE TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
}
| TK_OPEN KW_NOT TK_OPEN KW_GOAL TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$8->val, md_neg_goal);
}
atom_argument_list TK_CLOSE TK_CLOSE TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
}
;

type_constraint_atom:
TK_OPEN TK_TYPE_SYMBOL TK_VAR_SYMBOL TK_CLOSE
{
  TypeConstraint* c =
    new TypeConstraint((VariableSymbol*)$3->val, (TypeSymbol*)$2->val);
  current_context->type_con.append(c);
}
| TK_OPEN TK_TYPE_SYMBOL functional_atom_argument TK_CLOSE
{
  TypeConstraint* c =
    new TypeConstraint((VariableSymbol*)$3->val, (TypeSymbol*)$2->val);
  current_context->type_con.append(c);
}

set_constraint_type:
KW_AT_LEAST_N
{
  $$ = sc_at_least;
}
| KW_AT_MOST_N
{
  $$ = sc_at_most;
}
| KW_EXACTLY_N
{
  $$ = sc_exactly;
}
;

invariant_set:
invariant_set invariant_atom
| invariant_set invariant_set_of_atoms
| /* empty */

invariant_atom:
TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  ((Atom*)current_atom)->check();
  dom_sc_invariants[dom_sc_invariants.length()-1]->pos_atoms.append((Atom*)current_atom);
}
| TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$4->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  dom_sc_invariants[dom_sc_invariants.length()-1]->neg_atoms.append((Atom*)current_atom);
}
;

invariant_set_of_atoms:
TK_OPEN KW_SETOF KW_VARS TK_OPEN
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
}
typed_param_list TK_CLOSE
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
}
opt_context_spec TK_OPEN TK_PRED_SYMBOL
{
  current_atom = new Atom((PredicateSymbol*)$11->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append((Atom*)current_atom);
  dom_sc_invariants[dom_sc_invariants.length()-1]->atom_sets.append(s);
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
}
;

pddl_plan:
TK_OPEN KW_PLAN
{
  input_plans.append(new InputPlan());
  if (current_plan_file != current_file()) {
    n_plans_in_current_file = 0;
    current_plan_file = current_file();
  }
}
plan_elements TK_CLOSE
{
  if (input_plans[input_plans.length() - 1]->name == 0)
    if (current_plan_file) {
      std::ostringstream pn;
      pn << current_plan_file << ":" << n_plans_in_current_file;
      Symbol* plan_file_name = new Symbol(sym_misc, strdup(pn.str().c_str()));
      input_plans[input_plans.length() - 1]->name = plan_file_name;
    }
  n_plans_in_current_file += 1;
}
;

plan_elements:
plan_name plan_elements
| KW_OPT plan_elements
{
  assert(input_plans.length() > 0);
  input_plans[input_plans.length() - 1]->is_opt = true;
}
| plan_step plan_elements
| /* empty */
;

plan_name:
KW_NAME any_symbol
{
  assert(input_plans.length() > 0);
  input_plans[input_plans.length() - 1]->name = new Symbol($2->text);
}
;

plan_step:
numeric_value TK_COLON TK_OPEN TK_ACTION_SYMBOL
{
  current_atom = new Reference((ActionSymbol*)$4->val);
}
atom_argument_list TK_CLOSE
{
  Reference* ref = (Reference*)current_atom;
  ActionSymbol* act = (ActionSymbol*)$4->val;
  act->refs.append(ref);
  assert(input_plans.length() > 0);
  input_plans[input_plans.length() - 1]->
    steps.append(new InputPlanStep(ref, $1));
  clear_context(current_param);
}
;

ipc_plan:
ipc_plan_step_list
{
  // input_plans.append(0);
  current_plan_file = 0;
}
| ipc_plan_step_seq
{
  // input_plans.append(0);
  current_plan_file = 0;
}
;

ipc_plan_step_list:
ipc_plan_step ipc_plan_step_list
| ipc_plan_step
;

ipc_plan_step:
numeric_value TK_COLON TK_OPEN TK_ACTION_SYMBOL
{
  current_atom = new Reference((ActionSymbol*)$4->val);
}
atom_argument_list TK_CLOSE opt_step_duration
{
  Reference* ref = (Reference*)current_atom;
  ActionSymbol* act = (ActionSymbol*)$4->val;
  act->refs.append(ref);
  if (input_plans.length() == 0) {
    at_position(std::cerr) << "beginning of new plan" << std::endl;
    input_plans.append(new InputPlan());
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length() - 1]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  else if (current_file() != current_plan_file) {
    at_position(std::cerr) << "beginning of new plan (new file)" << std::endl;
    input_plans.append(new InputPlan());
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length() - 1]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  input_plans[input_plans.length() - 1]->
    steps.append(new InputPlanStep(ref, $1));
  clear_context(current_param);
}
;

opt_step_duration:
TK_OPEN_SQ numeric_value TK_CLOSE_SQ
| /* empty */
;

ipc_plan_step_seq:
simple_plan_step ipc_plan_step_seq
| simple_plan_step
;

simple_plan_step:
TK_OPEN TK_ACTION_SYMBOL
{
  current_atom = new Reference((ActionSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  Reference* ref = (Reference*)current_atom;
  ActionSymbol* act = (ActionSymbol*)$2->val;
  act->refs.append(ref);
  if (input_plans.length() == 0) {
    at_position(std::cerr) << "beginning of new plan" << std::endl;
    input_plans.append(new InputPlan());
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length() - 1]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  else if (current_file() != current_plan_file) {
    at_position(std::cerr) << "beginning of new plan (new file)" << std::endl;
    input_plans.append(new InputPlan());
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length() - 1]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  HSPS::index_type n = input_plans[input_plans.length() - 1]->steps.length();
  input_plans[input_plans.length() - 1]->
    steps.append(new InputPlanStep(ref, n));
  clear_context(current_param);
}
;

heuristic_table:
TK_OPEN KW_HEURISTIC table_entry_list TK_CLOSE
;

table_entry_list:
table_entry_list table_entry
| /* empty */
;

table_entry:
TK_OPEN
{
  current_entry = new HTableEntry();
}
ne_entry_atom_list TK_CLOSE entry_value
{
  h_table.append(current_entry);
  current_entry = 0;
}
;

entry_value:
KW_OPT numeric_value
{
  current_entry->cost = $2;
  current_entry->opt = true;
}
| numeric_value
{
  current_entry->cost = $1;
}
;

ne_entry_atom_list:
ne_entry_atom_list entry_atom
| entry_atom
;

entry_atom:
pos_entry_atom
| neg_entry_atom
;

pos_entry_atom:
TK_OPEN TK_PRED_SYMBOL
{
  assert(current_entry);
  current_atom = new Atom((PredicateSymbol*)$2->val);
}
atom_argument_list TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_entry->atoms.append((Atom*)current_atom);
  current_entry->neg.append(false);
  assert(current_entry->atoms.length() == current_entry->neg.length());
}
;

neg_entry_atom:
TK_OPEN KW_NOT TK_OPEN TK_PRED_SYMBOL
{
  assert(current_entry);
  current_atom = new Atom((PredicateSymbol*)$4->val);
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  ((Atom*)current_atom)->check();
  current_entry->atoms.append((Atom*)current_atom);
  current_entry->neg.append(true);
  assert(current_entry->atoms.length() == current_entry->neg.length());
}
;

reference_set:
TK_OPEN KW_SET
{
  ReferenceSet* set = new ReferenceSet();
  current_context = set;
  stored_n_param.assign_value(0, 1);
  current_param.clear();
  input_sets.append(set);
}
opt_set_name opt_vars_spec opt_context_spec reference_list TK_CLOSE
{
  clear_context(current_param);
  current_context = 0;
}
;

opt_set_name:
KW_NAME TK_NEW_SYMBOL
{
  $2->val = new Symbol(sym_misc, $2->text);
  ((ReferenceSet*)current_context)->name = (Symbol*)$2->val;
}
| KW_NAME TK_MISC_SYMBOL
{
  ((ReferenceSet*)current_context)->name = (Symbol*)$2->val;
}
| /* empty */
;

reference_list:
reference_list simple_reference_set
| reference_list reference
| /* empty */
;

reference:
TK_OPEN any_symbol
{
  if ($2->val) {
    current_atom = new Reference((Symbol*)$2->val, false, true);
  }
  else {
    current_atom = new Reference(new Symbol(sym_misc, $2->text), false, true);
  }
}
atom_argument_list TK_CLOSE
{
  assert(input_sets.length() > 0);
  assert(input_sets[input_sets.length() - 1] != 0);
  input_sets[input_sets.length() - 1]->
    add(new SimpleReferenceSet((Reference*)current_atom));
}
| any_symbol
{
  assert(input_sets.length() > 0);
  assert(input_sets[input_sets.length() - 1] != 0);
  if ($1->val) {
    input_sets[input_sets.length() - 1]->add
      (new SimpleReferenceSet(new Reference((Symbol*)$1->val, false, false)));
  }
  else {
    input_sets[input_sets.length() - 1]->add
      (new SimpleReferenceSet(new Reference(new Symbol(sym_misc, $1->text), false, false)));
  }
}
;

simple_reference_set:
TK_OPEN KW_SETOF
{
  stored_n_param.append(current_param.length());
  stored_context.append(current_context);
  current_context = new SimpleReferenceSet(0);
}
req_vars_spec opt_context_spec simple_reference_set_body TK_CLOSE
{
  assert(stored_n_param.length() > 0);
  assert(stored_context.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = stored_context[stored_context.length() - 1];
  stored_context.dec_length();
}
;

simple_reference_set_body:
TK_OPEN any_symbol
{
  if ($2->val) {
    current_atom = new Reference((Symbol*)$2->val, false, true);
  }
  else {
    current_atom = new Reference(new Symbol(sym_misc, $2->text), false, true);
  }
}
atom_argument_list TK_CLOSE
{
  SimpleReferenceSet* s = (SimpleReferenceSet*)current_context;
  s->ref = (Reference*)current_atom;
  assert(input_sets.length() > 0);
  assert(input_sets[input_sets.length() - 1] != 0);
  input_sets[input_sets.length() - 1]->add(s);
}
| TK_OPEN KW_NOT TK_OPEN any_symbol
{
  if ($4->val) {
    current_atom = new Reference((Symbol*)$4->val, true, true);
  }
  else {
    current_atom = new Reference(new Symbol(sym_misc, $4->text), true, true);
  }
}
atom_argument_list TK_CLOSE TK_CLOSE
{
  SimpleReferenceSet* s = (SimpleReferenceSet*)current_context;
  s->ref = (Reference*)current_atom;
  assert(input_sets.length() > 0);
  assert(input_sets[input_sets.length() - 1] != 0);
  input_sets[input_sets.length() - 1]->add(s);
}
;

%%
