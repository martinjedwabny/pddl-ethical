
#ifndef LOGIC_H
#define LOGIC_H

#include "config.h"
#include "base.h"

BEGIN_HSPS_NAMESPACE

void new_variable_vec(PDDL_Base* base,
		      index_type n,
		      PDDL_Base::variable_vec& v);
PDDL_Base::AFormula* new_atomic_formula(PDDL_Base* base,
					PDDL_Base::PredicateSymbol* p);

// construct simple formulas
PDDL_Base::Formula* make_unifying_formula(PDDL_Base::symbol_pair_vec& u);
PDDL_Base::Formula* make_separating_formula(PDDL_Base::symbol_pair_vec& u);
PDDL_Base::Formula* make_atom_formula(PDDL_Base::Atom* atom);

// create a predicate substitution (renaming scheme) based on suffix
void create_predicate_substitution
(const char* suffix,
 lvector<PDDL_Base::PredicateSymbol*> preds,
 PDDL_Base::symbol_pair_vec& s);

// construct formulas representing actions
PDDL_Base::Formula*
action_precondition(PDDL_Base::ActionSymbol* act);

PDDL_Base::Formula*
action_adds_atom(PDDL_Base::ActionSymbol* act,
		 PDDL_Base::Atom* atom);

PDDL_Base::Formula*
action_deletes_atom(PDDL_Base::ActionSymbol* act,
		    PDDL_Base::Atom* atom);

PDDL_Base::Formula*
make_step_formula(PDDL_Base* base,
		  PDDL_Base::ActionSymbol* act,
		  lvector<PDDL_Base::PredicateSymbol*> preds,
		  PDDL_Base::symbol_pair_vec& s0,
		  PDDL_Base::symbol_pair_vec& s1);

PDDL_Base::Formula*
regress(PDDL_Base::Formula* f, PDDL_Base::ActionSymbol* act);

PDDL_Base::Formula*
make_verification_formula(PDDL_Base::Formula* f, PDDL_Base* base);

// construct candidate invariant formulas:

// - simple single-valuedness constraints
void make_all_ssvcs(PDDL_Base* base,
		    PDDL_Base::PredicateSymbol* p,
		    PDDL_Base::formula_vec& v);

void make_simple_categories(PDDL_Base* base,
			    PDDL_Base::TypeSymbol* t,
			    PDDL_Base::VariableSymbol* voft,
			    PDDL_Base::formula_vec& v);

// - binary simple category exclusion constraints
void make_all_bscecs(PDDL_Base* base,
		     PDDL_Base::TypeSymbol* p,
		     PDDL_Base::formula_vec& v);

// - general simple category exclusion constraints
void make_all_scecs(PDDL_Base* base,
		    PDDL_Base::TypeSymbol* p,
		    PDDL_Base::formula_vec& v);

END_HSPS_NAMESPACE

#endif
