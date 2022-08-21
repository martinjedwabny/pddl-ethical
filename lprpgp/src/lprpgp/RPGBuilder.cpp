/************************************************************************
 * Copyright 2008, 2009, Strathclyde Planning Group,
 * Department of Computer and Information Sciences,
 * University of Strathclyde, Glasgow, UK
 * http://planning.cis.strath.ac.uk/
 *
 * Maria Fox, Richard Howey and Derek Long - Code from VAL
 * Stephen Cresswell - PDDL Parser
 * Andrew Coles, Amanda Coles - Code for LPRPG
 *
 * This file is part of LPRPG.
 *
 * LPRPG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * LPRPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LPRPG.  If not, see <http://www.gnu.org/licenses/>.
 *
 ************************************************************************/

#include "RPGBuilder.h"

#include "GlobalSchedule.h"

#include "ptree.h"
#include <FlexLexer.h>
#include "instantiation.h"
#include "SimpleEval.h"
#include "DebugWriteController.h"
#include "typecheck.h"
#include "TIM.h"

#include "colours.h"

#include "FuncAnalysis.h"
#include "LiteralAnalysis.h"

//#include "graphconstruct.h"
//#include "PartialPlan.h"

#include "FFSolver.h"

#include <assert.h>

#include <algorithm>
#include "MILPRPG.h"
#include "MetricRPG.h"

#include "NNF.h"

#include "NumericAnalysis.h"
#include "PreferenceHandler.h"
#include "landmarksanalysis.h"

#include <sstream>

using namespace TIM;
using namespace Inst;
using namespace VAL;

using std::ostringstream;
using std::cerr;
using std::endl;

namespace Planner {

whereAreWe WhereAreWeNow = PARSE_UNKNOWN;

ostream & operator << (ostream & o, const whereAreWe & w) {
      switch (w) {
	    case (PARSE_PRECONDITION):
		  o << "As a precondition";
		  break;
	    case (PARSE_EFFECT):
		  o << "As an effect";
		  break;
	    case (PARSE_DURATION):
		  o << "As a duration constraint";
		  break;
	    case (PARSE_GOAL):
		  o << "In the problem goals";
		  break;
	    case (PARSE_INITIAL):
		  o << "In the initial state";
		  break;
	    case (PARSE_CONDITIONALEFFECT):
		  o << "In a conditional effect";
		  break;
	    case (PARSE_CONTINUOUSEFFECT):
		  o << "In a continuous effect";
		  break;
	    case (PARSE_METRIC):
		  o << "In the problem metric";
		  break;
	    case (PARSE_CONSTRAINTS) :
		  o << "In the problem constraints/preferences";
		  break;
	    default:
		  o << "At some point";
		  break;
      };
      return o;
};

void postmortem_duplicateInitialStateValue(const PNE * const variable, const double previous, const double now) {
    
    cerr << "A problem has been encountered, and the planner has to terminate.\n";
    cerr << "-----------------------------------------------------------------\n";
    cerr << "The problem file given defines multiple values in the initial\n";
    cerr << "state for one or more variables.  The following lines were both\n";
    cerr << "encountered:\n\n";
    cerr << "(= " << *variable << " " << previous << ")\n\n";
    cerr << "(= " << *variable << " " << now << ")\n\n";
    cerr << "Please fix your problem file and run the planner again.\n";
    exit(1);
}

void postmortem_noNestedWhens() {

	cerr << "A problem has been encountered, and the planner has to terminate.\n";
	cerr << "-----------------------------------------------------------------\n";
	cerr << "Unfortunately, at present, the planner does not supported nested (when (...\n";
	cerr << "conditional effects, but they are present in the problem you have provided.\n\n";
	cerr << "To use this planner with your problem, you will have to reformulate it to\n";
	cerr << "avoid these.  In the case of nested whens, one can rewrite:\n\n";
	cerr << "(when (x)\n";
	cerr << "         (and  (when (y) ...\n";
	cerr << "         (and  (when (z) ...\n";
	cerr << "\nas:\n";
	cerr << "(when (and (x) (y))\n";
	cerr << "           ...       )\n";
	cerr << "(when (and (x) (z))\n";
	cerr << "           ...       )\n\n";
	cerr << "Alternatively, dive into the source code yourself or contact the authors,\n";
	cerr << "who may be able to help.  Apologies, and best of luck with your task.\n";

	exit(0);
};

void postmortem_noADL() {

	cerr << "A problem has been encountered, and the planner has to terminate.\n";
	cerr << "-----------------------------------------------------------------\n";
	cerr << "Unfortunately, at present, the planner does not fully support ADL\n";
	cerr << "unless in the rules for derived predicates.  Only two aspects of\n";
	cerr << "ADL can be used in action definitions:\n";
	cerr << "- forall conditions, containing a simple conjunct of propositional and\n";
	cerr << "  numeric facts;\n";
	cerr << "- Conditional (when... ) effects, and then only with static conditions\n";
    cerr << "  that can be determined from the propositions in the initial state.\n\n";
	cerr << "To use this planner with your problem, you will have to reformulate it to\n";
	cerr << "avoid ADL.  Alternatively, if you have a particularly compelling case\n";
	cerr << "for them, please contact the authors to discuss it with them, who may be able to\n";
	cerr << "extend the planner to meet your needs.\n";

	exit(0);
};

void postmortem_noConstraints(const bool unsupportedPref, const char * n) {

	if (unsupportedPref) {
		cerr << "Warning - Unsupported Preference Type\n";
		cerr << "-------------------------------------\n";
		cerr << "Unfortunately, at present, the planner does not fully support\n";
		cerr << "PDDL 3 preferences - only 'always', 'atsometime', 'atmostonce',\n";
		cerr << "'sometime', 'sometime-after', 'sometime-before' and at-end\n";
		cerr << " preferences are supported.\n\n";
		cerr << "The planner will continue, but will quietly ignore the preference\nnamed " << n << ".\n\n";
	} else {
		cerr << "A problem has been encountered, and the planner has to terminate.\n";
		cerr << "-----------------------------------------------------------------\n";
		cerr << "Unfortunately, at present, the planner does not fully support\n";
		cerr << "PDDL 3 - hard trajectory constraints are not supported, only preferences.\n\n";
		cerr << "To use this planner with your problem, you will have to reformulate it to\n";
		cerr << "avoid these.  Alternatively, if you have a particularly compelling case\n";
		cerr << "for these features, please contact the authors to discuss it with them,\n";
		cerr << "who may be able to extend the planner to meet your needs.\n";

		exit(0);

	}
};

void postmortem_isViolatedNotExist(const string & n) {
	cerr << "A problem has been encountered, and the planner has to terminate.\n";
	cerr << "-----------------------------------------------------------------\n";
	cerr << "An error has been encountered in your metric expression.  The preference:\n\n";
	cerr << "\t" << n << "\n\n";
	cerr << "... does not exist, but the metric contains the expression:\n\n";
	cerr << "\t(is-violated " << n << ")\n\n";
	exit(0);
};

void postmortem_nonLinearCTS(const string & actName, const string & worksOutAs) {
	cerr << "A problem has been encountered, and the planner has to terminate.\n";
	cerr << "-----------------------------------------------------------------\n";
	cerr << "Unfortunately, the planner only supports continuous effects where the gradient\n";
	cerr << "is state-independent, i.e. evaluates to a single value.  In the action:\n\n";
	cerr << "\t" << actName << "\n\n";
	cerr << "... a continuous effect was encountered that amounts to:\n";
	cerr << "\t" << worksOutAs << "\n\n";
	cerr << "To use this planner with your problem, you will have to reformulate it to\n";
	cerr << "avoid these.  Alternatively, if you have an interesting application for them,\n";
	cerr << "please contact the authors to discuss it with them, who may be able to\n";
	cerr << "extend the planner to meet your needs.\n";

	exit(0);
};

void postmortem_noQuadratic(const string & theOp) {

	cerr << "A problem has been encountered, and the planner has to terminate.\n";
	cerr << "-----------------------------------------------------------------\n";
	cerr << "Unfortunately, the planner does not supported non-linear numeric conditions,\n";
	cerr << "effects, or duration constraints, but one or more of these is present in\n";
	cerr << "the problem you have provided.  Specifically, the sub-expression:\n\n";
	cerr << "\t" << theOp << "\n\n";
	cerr << "... was encountered.  To use this planner with your problem, you will have\n";
	cerr << "to reformulate it to avoid these.\n";

	exit(0);
};

void postmortem_noTimeSpecifierOnAPropPrecondition(const string & actname, const string & effect) {
	cerr << "A problem has been encountered with your domain/problem file.\n";
	cerr << "-------------------------------------------------------------\n";
	cerr << "Unfortunately, a bug has been encountered in your domain and problem file,\n";
	cerr << "and the planner has to terminate.  The durative action:\n\n";
	cerr << "\t" << actname << "\n";
	cerr << "has a propositional precondition:\n\n";
	cerr << "\t" << effect << "\n\n";
	cerr << "As it's a durative action, you need to give a time specifier - either\n";
	cerr << "(at start ...), (over all...) or (at end ...) - to indicate when the fact is\n";
	cerr << "to hold during the execution of the action.  For instance:\n";
	cerr << "\t (at start " << effect << ")\n";

	exit(0);
};


void postmortem_fixedAndNotTimeSpecifiers(const string & actname, const bool & multipleEquals) {
	cerr << "A problem has been encountered with your domain/problem file.\n";
	cerr << "-------------------------------------------------------------\n";
	cerr << "Unfortunately, a bug has been encountered in your domain and problem file,\n";
	cerr << "and the planner has to terminate.  The durative action:\n\n";
	cerr << "\t" << actname << "\n";
	if (multipleEquals) {
		cerr << "has multiple constraints of the form (= ?duration ...).  Actions with fixed durations\n";
		cerr << "can only have one constraint governing their duration.\n";
	} else {
		cerr << "has both duration constraints of the form (= ?duration ...), and those specifying\n";
		cerr << "maximum and/or minimum values.\n";
	}

	exit(0);
};

void postmortem_noTimeSpecifierOnAPropEffect(const string & actname, const string & effect) {
	cerr << "A problem has been encountered with your domain/problem file.\n";
	cerr << "-------------------------------------------------------------\n";
	cerr << "Unfortunately, a bug has been encountered in your domain and problem file,\n";
	cerr << "and the planner has to terminate.  The durative action:\n\n";
	cerr << "\t" << actname << "\n";
	cerr << "has a propositional effect:\n\n";
	cerr << "\t" << effect << "\n\n";
	cerr << "As it's a durative action, you need to give a time specifier - either\n";
	cerr << "(at start ...) or (at end ...) - to indicate when the effect is to\n";
	cerr << "occur when the action is executed, for instance:\n";
	cerr << "\t (at start " << effect << ")\n";

	exit(0);
};

void postmortem_noTimeSpecifierOnInstantNumericEffect(const string & actname, const string & effect, const string & suggested, const bool & isAssign) {
	cerr << "A problem has been encountered with your domain/problem file.\n";
	cerr << "-------------------------------------------------------------\n";
	cerr << "Unfortunately, a bug has been encountered in your domain and problem file,\n";
	cerr << "and the planner has to terminate.  The durative action:\n\n";
	cerr << "\t" << actname << "\n\n";
	cerr << "has a numeric effect:\n\n";
	cerr << "\t" << effect << "\n\n";
	cerr << "As it's a durative action, and the effect is not continuous across\n";
	cerr << "the duration of the action (due to #t not being present), you may wish\n";
	cerr << "do one of two things:\n\n";
	cerr << " (i) Give a time specifier - either (at start ...) or (at end ...) - to\n";
	cerr << "     indicate when the effect is to occur when the action is executed, e.g:\n\n";
	cerr << "\t (at start " << effect << ")\n\n";

	if (isAssign) {

	cerr << "(ii) If the effect is meant to be continuous, first note that continuous\n";
	cerr << "     effects cannot be assignments - they have to be increase or decrease\n";
	cerr << "     effects.  Once you have resolved this, include #t where desired.\n";
	cerr << "     For instance, you may have meant:\n";

	cerr << "\t" << suggested << "\n";


	} else {
	
	cerr << "(ii) If the effect is meant to be continuous, rewrite it to signify\n";
	cerr << "     this by including #t where desired.  For instance, you may have meant:\n\n";
	cerr << "\t" << suggested << "\n";

	}
	exit(0);
};

void postmortem_wrongNumberOfFluentArguments(const string & actname, const bool & haveActName, const whereAreWe & w, const string & predname, const string & lit, const int & givenArgs, const set<int> & realargs) {
	cerr << "A problem has been encountered with your domain/problem file.\n";
	cerr << "-------------------------------------------------------------\n";
	cerr << "Unfortunately, a bug has been encountered in your domain and problem file,\n";
	cerr << "and the planner has to terminate.  " << w << " ";
	if (haveActName) cerr << "within the action:\n\n\t" << actname << "\n\n";
	cerr << "the invalid functional value:\n\n";
	cerr << "\t" << lit << "\n\n";
	cerr << "is defined. '" << predname << "' cannot take " << givenArgs;
	if (givenArgs != 1) {
	      cerr << " arguments";
	} else {
	      cerr << " argument";
	}
	cerr << ", it can only take ";
	if (realargs.size() == 1) {
		const int ra = *(realargs.begin());
		cerr << ra << ".\n";
		
	} else {
		cerr << "either ";
		set<int>::iterator raItr = realargs.begin();
		const set<int>::iterator raEnd = realargs.end();
		int lastPrint = -1;
		for (; raItr != raEnd; ++raItr) {
			if (lastPrint != -1) cerr << lastPrint << ", ";
			lastPrint = *raItr;
		}
		cerr << "or " << lastPrint << ".\n";
	}
	exit(0);

}

void postmortem_wrongNumberOfPredicateArguments(const string & actname, const bool & haveActName, const whereAreWe & w, const string & predname, const string & lit, const int & givenargs, const set<int> & realargs) {
	cerr << "A problem has been encountered with your domain/problem file.\n";
	cerr << "-------------------------------------------------------------\n";
	cerr << "Unfortunately, a bug has been encountered in your domain and problem file,\n";
	cerr << "and the planner has to terminate.   " << w << " ";
	if (haveActName) cerr << "within the action:\n\n\t" << actname << "\n";
	cerr << "the invalid proposition:\n\n";
	cerr << "\t" << lit << "\n\n";
	cerr << "is defined. '" << predname << "' cannot take " << givenargs;
	if (givenargs != 1) {
	      cerr << " arguments";
	} else {
	      cerr << " argument";
	}
	cerr << ", it can only take ";
	if (realargs.size() == 1) {
		const int ra = *(realargs.begin());
		cerr << ra << ".\n";
		
	} else {
		cerr << "either ";
		set<int>::iterator raItr = realargs.begin();
		const set<int>::iterator raEnd = realargs.end();
		int lastPrint = -1;
		for (; raItr != raEnd; ++raItr) {
			if (lastPrint != -1) cerr << lastPrint << ", ";
			lastPrint = *raItr;
		}
		cerr << "or " << lastPrint << ".\n";
	}
	
	exit(0);
};


void postmortem_mathsError(const string & description, const string & help, const whereAreWe & w)
{
    cerr << "A maths error has been encountered in your domain/problem file.\n";
    cerr << "---------------------------------------------------------------\n";
    cerr << "Unfortunately, a maths error has been encountered in domain/problem,\n";
    cerr << "and the planner has to terminate.   " << w << " a\n";
    cerr << description << " occurred.\n";
    cerr << help;
    exit(0);
}

void postmortem_invalidMetric()
{
    cerr << "A problem has been encountered with your domain/problem file.\n";
    cerr << "-------------------------------------------------------------\n";
    cerr << "Unfortunately, a bug has been encountered in your problem file,\n";
    cerr << "and the planner has to terminate.  The metric is invalid: it\n";
    cerr << "refers to a function upon which no actions have an effect, but\n";
    cerr << "whose value is undefined in the initial state.\n";
    exit(0);
}

void postmortem_invalidGoal(const string & s)
{
    cerr << "A problem has been encountered with your domain/problem file.\n";
    cerr << "-------------------------------------------------------------\n";
    cerr << "Unfortunately, a bug has been encountered in your problem file,\n";
    cerr << "and the planner has to terminate.  At least one of the goals\n";
    cerr << "is invalid: it refers to a function upon which no actions have an\n";
    cerr << "effect, but whose value is undefined in the initial state.\n";
    cerr << "The goal in question is:\n\n" << s << endl;
    exit(0);
}

vector<double> SubproblemRPG::deadlineAtTime;

bool RPGBuilder::modifiedRPG = true;
bool RPGBuilder::doNotApplyActionsThatLookTooExpensive = true;
bool RPGBuilder::planAnyway = false;
cost_propagation RPGBuilder::costPropagationMethod = E_SUMCOST;
bool RPGBuilder::useTheCostsInTheLP = false;
bool RPGBuilder::switchInCheaperAchievers = false;

class ExpressionBuilder: public VisitController {

private:

	list<RPGBuilder::Operand> & formula;
	VAL::TypeChecker * tc;
	VAL::FastEnvironment * fe;
    bool valid;
    bool debug;
    
public:

    ExpressionBuilder(list<RPGBuilder::Operand> & formulaIn, VAL::FastEnvironment * f,VAL::TypeChecker * t = 0)
        : formula(formulaIn), tc(t), fe(f), debug(GlobalSchedule::globalVerbosity & 16) {            
    }


    bool buildFormula(VAL::expression * e)  __attribute__ ((warn_unused_result)) {
        if (debug) cout << "Building numeric expression\n";
        valid = true;        
        e->visit(this);
        return valid;
    }

	void visit_plus_expression(const plus_expression * s) {
        if (debug) cout << "+ term\n";        
		s->getLHS()->visit(this);
		s->getRHS()->visit(this);
		formula.push_back(RPGBuilder::Operand(RPGBuilder::NE_ADD));
	}

	void visit_minus_expression(const minus_expression * s) {
        if (debug) cout << "- term\n";       
		s->getLHS()->visit(this);
		s->getRHS()->visit(this);
		formula.push_back(RPGBuilder::Operand(RPGBuilder::NE_SUBTRACT));
	}
	void visit_mul_expression(const mul_expression * s) {
        if (debug) cout << "* term\n";        
		s->getLHS()->visit(this);
		s->getRHS()->visit(this);
		formula.push_back(RPGBuilder::Operand(RPGBuilder::NE_MULTIPLY));
	}
	void visit_div_expression(const div_expression * s) {
        if (debug) cout << "/ term\n";        
		s->getLHS()->visit(this);
		s->getRHS()->visit(this);
		formula.push_back(RPGBuilder::Operand(RPGBuilder::NE_DIVIDE));
	}

	void visit_uminus_expression(const uminus_expression * s) {
        if (debug) cout << "0- term\n";        
		formula.push_back(RPGBuilder::Operand(0.0));
		s->getExpr()->visit(this);
		formula.push_back(RPGBuilder::Operand(RPGBuilder::NE_SUBTRACT));
	}

	void visit_int_expression(const int_expression * s) {
        if (debug) cout << "int term " << s->double_value() << endl;        
		formula.push_back(RPGBuilder::Operand((double) s->double_value()));
	}
	void visit_float_expression(const float_expression * s) {
        if (debug) cout << "float term " << s->double_value() << endl;        
		formula.push_back(RPGBuilder::Operand((double) s->double_value()));
	};

	void visit_special_val_expr(const special_val_expr * v) {
		if (v->getKind() == E_HASHT) {
			formula.push_back(RPGBuilder::Operand((int) -2));
		} else if (v->getKind() == E_DURATION_VAR) {
			formula.push_back(RPGBuilder::Operand((int) -3));
		} else if (v->getKind() == E_TOTAL_TIME) {
			formula.push_back(RPGBuilder::Operand((int) -4));
		} else {
			cout << "Error parsing expression: unsupported task constant " << *v << " found\n";
		}
	};

	
	void visit_func_term(const func_term * s) {
		PNE lookupPNE(s, fe);
		validatePNE(&lookupPNE);
		//cout << "Looking up " << *lookupPNE << "\n";
		PNE * realPNE = instantiatedOp::findPNE(&lookupPNE);
        
		if (!realPNE) {
            if (debug) {
                cout << "PNE " << lookupPNE << " did not exist\n";
            }
            
            formula.push_back(RPGBuilder::Operand((double) 0.0));
            valid = false;
        } else {	
            // cout << "'" << realPNE->getHead()->getName() << "'\n";
            if (realPNE->getHead()->getName() == "fake-duration") {
                cout << "Detected fake-duration in condition, replaced with ?duration\n";
                formula.push_back(RPGBuilder::Operand((int) -3));
            } else {
                if (EFT(realPNE->getHead())->isStatic()) {
                    if (debug) {
                        cout << "PNE " << *realPNE << ", with static value " << EFT(realPNE->getHead())->getInitial(realPNE->begin(), realPNE->end()).second << std::endl; 
                    }
                    formula.push_back(RPGBuilder::Operand(EFT(realPNE->getHead())->getInitial(realPNE->begin(), realPNE->end()).second));	
                } else {
                    if (debug) {
                        cout << "PNE " << *realPNE << ", ID " << realPNE->getStateID() << std::endl;
                    }
                    formula.push_back(RPGBuilder::Operand((int) realPNE->getStateID()));
                }
            }
        }
    }

    void visit_violation_term(const violation_term * s) {
        formula.push_back(RPGBuilder::Operand(s->getName()));
    }
};

class ExpressionPrinter: public VisitController {

private:

	VAL::TypeChecker * tc;
	VAL::FastEnvironment * fe;
	ostream & o;
public:

	ExpressionPrinter(ostream & oIn, VAL::FastEnvironment * f,VAL::TypeChecker * t = 0) :
		tc(t), fe(f), o(oIn)
	{};

	void printFormula(VAL::expression * e) {
		e->visit(this);
	}

	void visit_plus_expression(const plus_expression * s) {
		o << "(+ ";
		s->getLHS()->visit(this);
		o << " ";
		s->getRHS()->visit(this);
		o << ")";
	}

	void visit_minus_expression(const minus_expression * s) {
		o << "(- ";
		s->getLHS()->visit(this);
		o << " ";
		s->getRHS()->visit(this);
		o << ")";
	}
	void visit_mul_expression(const mul_expression * s) {
		o << "(* ";
		s->getLHS()->visit(this);
		o << " ";
		s->getRHS()->visit(this);
		o << ")";
	}
	void visit_div_expression(const div_expression * s) {
		o << "(/ ";
		s->getLHS()->visit(this);
		o << " ";
		s->getRHS()->visit(this);
		o << ")";
	}

	void visit_uminus_expression(const uminus_expression * s) {
		o << "(- ";
		s->getExpr()->visit(this);
		o << ")";
	}
	void visit_int_expression(const int_expression * s) {
		o << s->double_value();
	}
	void visit_float_expression(const float_expression * s) {
		o << s->double_value();
	};

	void visit_special_val_expr(const special_val_expr * v) {
		if (v->getKind() == E_HASHT) {
			o << "#t";
		} else if (v->getKind() == E_DURATION_VAR) {
			o << "?duration";
		} else if (v->getKind() == E_TOTAL_TIME) {
			o << "total-time";
		}
	};

	
	void visit_func_term(const func_term * s) {
		PNE * const lookupPNE = new PNE(s, fe);
		//cout << "Looking up " << *lookupPNE << "\n";
		o << *lookupPNE;
	};

	void visit_violation_term(const violation_term * s) {		
		o << "(is-violated " << s->getName() << ")";
	};

};

double RPGBuilder::calculateRHS(const list<Operand> & formula, vector<double> & fluents) {

	list<double> RHS;
	
	list<Operand>::const_iterator fItr = formula.begin();
	const list<Operand>::const_iterator fEnd = formula.end();

	for (; fItr != fEnd; ++fItr) {
		const Operand & currOperand = *fItr;
		const math_op currMathOp = currOperand.numericOp;
		switch(currMathOp) {
			case RPGBuilder::NE_ADD:
				{
				const double oldFront = RHS.front(); RHS.pop_front();
				RHS.front() += oldFront;
				}
				break;
			case RPGBuilder::NE_SUBTRACT:
				{
				const double oldFront = RHS.front(); RHS.pop_front();
				RHS.front() -= oldFront;
				}
				break;
			case RPGBuilder::NE_MULTIPLY:
				{
				const double oldFront = RHS.front(); RHS.pop_front();
				RHS.front() *= oldFront;
				}
				break;
			case RPGBuilder::NE_DIVIDE:
				{
				const double oldFront = RHS.front(); RHS.pop_front();
				RHS.front() /= oldFront;
				}
				break;
			case RPGBuilder::NE_CONSTANT:
				RHS.push_front(currOperand.constantValue);
				break;
			case RPGBuilder::NE_FLUENT:
				RHS.push_front(fluents[currOperand.fluentValue]);
				break;
			case RPGBuilder::NE_VIOLATION:
				RHS.push_front(0.0);
				break;

			default:
				// this should never happen
				assert(false);
		}
	}

	return RHS.front();

};

double RPGBuilder::ArtificialVariable::evaluateWCalculate(const vector<double> & fluentTable, const int & pneCount) {
	//cout << "Evaluating AV " << ID << " of size " << size << "\n";
	double toReturn = constant;
	//cout << "Evaluating AV of size " << size << "\n";
	for (int i = 0; i < size; ++i) {
		int var = fluents[i];
		double w = weights[i];
		if (var >= pneCount) {
			var -= pneCount;
			if (w != 0.0) w = 0.0 - w;
		}
		toReturn += w * fluentTable[var];
	}
	return toReturn;
};


bool RPGBuilder::RPGNumericPrecondition::isSatisfiedWCalculate(vector<double> & maxFluents) const {
	
	const int pneCount = RPGBuilder::getPNECount();
	int var = LHSVariable;
	//cout << "Precondition based on variable " << var << "\n";
	if (var < pneCount) {
		//cout << "Precondition based on positive variable " << var << "\n";
		if (op == VAL::E_GREATER) {
			return (maxFluents[var] > RHSConstant);
		} else {
			return (maxFluents[var] >= RHSConstant);
		}
	}
	var -= pneCount;
	if (var < pneCount) {
		//cout << "Precondition based on negative variable " << var << "\n";
		const double localVal = (maxFluents[var] != 0.0 ? 0.0 - maxFluents[var] : 0.0);
		if (op == VAL::E_GREATER) {
			return (localVal > RHSConstant);
		} else {
			return (localVal >= RHSConstant);
		}
	}
	var += pneCount;
	//cout << "Precondition based on artificial variable " << var << "\n";
	ArtificialVariable & av = RPGBuilder::getArtificialVariable(var);
	const double localVal = av.evaluateWCalculate(maxFluents, pneCount);
	if (op == VAL::E_GREATER) {
		return (localVal > RHSConstant);
	} else {
		return (localVal >= RHSConstant);
	}
	
};

bool RPGBuilder::RPGNumericPrecondition::canBeUnsatisfiedWCalculate(vector<double> & maxFluents) const {
    
    const int pneCount = RPGBuilder::getPNECount();
    int var = LHSVariable;
    //cout << "Precondition based on variable " << var << "\n";
    if (var < pneCount) {
        //cout << "Precondition based on positive variable " << var << "\n";
        if (op == VAL::E_GREATER) {
            return (maxFluents[var] <= RHSConstant);
        } else {
            return (maxFluents[var] < RHSConstant);
        }
    }
    var -= pneCount;
    if (var < pneCount) {
        //cout << "Precondition based on negative variable " << var << "\n";
        const double localVal = (maxFluents[var] != 0.0 ? 0.0 - maxFluents[var] : 0.0);
        if (op == VAL::E_GREATER) {
            return (localVal <= RHSConstant);
        } else {
            return (localVal < RHSConstant);
        }
    }
    var += pneCount;
    //cout << "Precondition based on artificial variable " << var << "\n";
    ArtificialVariable & av = RPGBuilder::getArtificialVariable(var);
    const double localVal = av.evaluateWCalculate(maxFluents, pneCount);
    if (op == VAL::E_GREATER) {
        return (localVal <= RHSConstant);
    } else {
        return (localVal < RHSConstant);
    }
                
}


RPGBuilder::NumericEffect::NumericEffect(const VAL::assign_op & opIn, const int & fIn, VAL::expression * formulaIn, VAL::FastEnvironment * f,VAL::TypeChecker * t)
    : fluentIndex(fIn), op(opIn) {
    ExpressionBuilder builder(formula, f, t);
    valid = builder.buildFormula(formulaIn);
}

RPGBuilder::NumericPrecondition::NumericPrecondition(const VAL::comparison_op & opIn, VAL::expression * LHSformulaIn, VAL::expression * RHSformulaIn, VAL::FastEnvironment * f,VAL::TypeChecker * t)
    : op(opIn), valid(true)
{
    {
        ExpressionBuilder builder(LHSformula, f, t);
        valid = builder.buildFormula(LHSformulaIn);
    }
    {
        ExpressionBuilder builder(RHSformula, f, t);
        valid = (valid && builder.buildFormula(RHSformulaIn));
    }

}

double RPGBuilder::NumericEffect::applyEffect(vector<double> & fluents) const {

	const double RHS = calculateRHS(formula, fluents);

	switch(op) {
		case VAL::E_ASSIGN:
			return RHS;
			break;
		case VAL::E_INCREASE:
			return (fluents[fluentIndex] + RHS);
			break;
		case VAL::E_DECREASE:
			return (fluents[fluentIndex] - RHS);
			break;
		case VAL::E_SCALE_UP:
			return (fluents[fluentIndex] * RHS);
			break;
		case VAL::E_SCALE_DOWN:
			return (fluents[fluentIndex] / RHS);
			break;
		default:
			// this should never happen
			assert(false);
	}

	
};

bool RPGBuilder::NumericPrecondition::isSatisfied(vector<double> & fluents) const {

	const double LHS = calculateRHS(LHSformula, fluents);
	const double RHS = calculateRHS(RHSformula, fluents);

	switch(op) {
		case VAL::E_GREATER:
			return (LHS > RHS);
			break;
		case VAL::E_GREATEQ:
			return (LHS >= RHS);
			break;
		case VAL::E_LESS:
			return (LHS < RHS);
			break;
		case VAL::E_LESSEQ:
			return (LHS <= RHS);
			break;
		case VAL::E_EQUALS:
			return (LHS == RHS);
			break;
	}

	assert(false); // this should never happen
	return false;
	
};

double RPGBuilder::NumericPrecondition::evaluateRHS(vector<double> & fluentTable) const {
	return calculateRHS(RHSformula, fluentTable);
}

/*

class PrecCollector : public VisitController {
private:
	VAL::TypeChecker * tc;
	bool adding;
	const VAL::operator_ * op;
	FastEnvironment * fe;

	bool inpres;
	bool checkpos;
	int minusC;
	
public:

	set<int> preconditionLiterals;
	list<RPGBuilder::NumericPrecondition> preconditionFluents;

	PrecCollector(const VAL::operator_ * o,FastEnvironment * f,VAL::TypeChecker * t = 0) :
		tc(t), adding(true), op(o), fe(f), inpres(true), checkpos(true), minusC(-1)
	{};
	
	virtual void visit_simple_goal(simple_goal * p) 
	{
		Literal * l = new Literal(p->getProp(),fe);
		Literal * foundLit = instantiatedOp::findLiteral(l);
		
		if (foundLit) {
			const int tID = foundLit->getID();
			assert(tID >= 0);
			preconditionLiterals.insert(tID);
			//cout << "Looking up " << *l << ", got " << tID << " - " << *foundLit << "\n";
		} else {
			preconditionLiterals.insert(--minusC);
			//cout << "Looking up " << *l << ", got 0\n";
		}
		delete l;
	};

	virtual void visit_comparison(comparison * c) {
		preconditionFluents.push_back(RPGBuilder::NumericPrecondition(c->getOp(), const_cast<VAL::expression*>(c->getLHS()), const_cast<VAL::expression*>(c->getRHS()), fe, tc));
		cout << preconditionFluents.back() << "\n";
	};

	virtual void visit_qfied_goal(qfied_goal * p) 
	{p->getGoal()->visit(this);};
	virtual void visit_conj_goal(conj_goal * p) 
	{p->getGoals()->visit(this);};
	virtual void visit_disj_goal(disj_goal * p) 
	{p->getGoals()->visit(this);};
	virtual void visit_timed_goal(timed_goal * p) 
	{p->getGoal()->visit(this);};
	virtual void visit_imply_goal(imply_goal * p) 
	{
		p->getAntecedent()->visit(this);
		p->getConsequent()->visit(this);
	};
	virtual void visit_neg_goal(neg_goal * p) 
	{
		bool oldcheck = checkpos;
		checkpos = !checkpos;
		p->getGoal()->visit(this);
		checkpos = oldcheck;
	};
    virtual void visit_preference(preference * p)
    {
    	p->getGoal()->visit(this);
    };	
	virtual void visit_constraint_goal(constraint_goal *cg)
    {
            if(cg->getRequirement())
            {
                    cg->getRequirement()->visit(this);
            };
            if(cg->getTrigger())
            {
                    cg->getTrigger()->visit(this);
            };
    };

	
	virtual void visit_operator_(VAL::operator_ * p) 
	{
		inpres = true;
		checkpos = true;
		p->precondition->visit(this);
		inpres = false;
		
		//adding = true;
		//p->effects->visit(this);
	};
	virtual void visit_action(VAL::action * p)
	{
		visit_operator_(p); //static_cast<VAL::operator_*>(p));
	};
	virtual void visit_durative_action(VAL::durative_action * p) 
	{
		visit_operator_(p); //static_cast<VAL::operator_*>(p));
	};
	virtual void visit_process(VAL::process * p)
	{
		visit_operator_(p);
	};
	virtual void visit_event(VAL::event * p)
	{
		visit_operator_(p);
	};
	virtual void visit_problem(VAL::problem * p) 
	{
		p->initial_state->visit(this);
		inpres = false;
		p->the_goal->visit(this);
	};


};
*/

class ChildInfoStack {
 
    list<int> childCouldOnlyBe;

public:
    void push_back(const int & i)
    {
        childCouldOnlyBe.push_back(i);
    }
    
    void get_result(int & i)
    {
        i = childCouldOnlyBe.back();
        childCouldOnlyBe.pop_back();
    }
};

class GoalNumericCollector : public VisitController {

private:

	list<RPGBuilder::NumericPrecondition> * numericGoals;
	list<Literal*> * literalGoals;
	
	list<RPGBuilder::NumericPrecondition> * numToUse;
	list<Literal*> * litToUse;

	map<string,list<int> > & prefNameToID;
	map<string,int> & prefNameToNumberOfTimesDefinitelyViolated;
    
	VAL::TypeChecker * tc;
	bool adding;
	const VAL::operator_ * op;
	FastEnvironment * fe;

    bool checkingIfConstraintCouldOnlyBeFalse;
    ChildInfoStack childCouldOnlyBe;
    
	bool inpres;
	bool checkpos;
	bool inConstraints;
	bool inPreference;
	
public:

	list<RPGBuilder::Preference> builtPreferences;

	GoalNumericCollector(list<RPGBuilder::NumericPrecondition> * ng, list<Literal*> * g,
                         map<string,list<int> > & p, map<string,int> & pv,
                         const VAL::operator_ * o,FastEnvironment * f,VAL::TypeChecker * t = 0) :
		numericGoals(ng), literalGoals(g),numToUse(numericGoals), litToUse(literalGoals),
		prefNameToID(p), prefNameToNumberOfTimesDefinitelyViolated(pv), tc(t), adding(true), op(o), fe(f),
		checkingIfConstraintCouldOnlyBeFalse(false), inpres(true), checkpos(true), inConstraints(false), inPreference(false)
	{    
    }
	

	virtual void visit_comparison(comparison * c)
	{
        if (checkingIfConstraintCouldOnlyBeFalse) {
            childCouldOnlyBe.push_back(0);
            return;
        }
		assert(!inPreference || (!inConstraints || numericGoals != numToUse));
		numToUse->push_back(RPGBuilder::NumericPrecondition(c->getOp(), const_cast<VAL::expression*>(c->getLHS()), const_cast<VAL::expression*>(c->getRHS()), fe, tc));

        if (!numToUse->back().valid) {
            ostringstream s;
            
            {
                ExpressionPrinter p(s, fe, tc);
                p.printFormula(const_cast<VAL::expression*>(c->getLHS()));
            }
            
            switch(c->getOp()) {
                case VAL::E_GREATEQ:
                    s << " >= ";
                    break;
                case VAL::E_GREATER:
                    s << " > ";
                    break;
                case VAL::E_LESSEQ:
                    s << " <= ";
                    break;
                case VAL::E_LESS:
                    s << " < ";
                    break;
                case VAL::E_EQUALS:
                    s << " = ";
                    break;
                default:
                    s << " <unknown operator> ";
            }
            
            
            {
                ExpressionPrinter p(s, fe, tc);
                p.printFormula(const_cast<VAL::expression*>(c->getRHS()));
            }
            
            string asString = s.str();
            
            postmortem_invalidGoal(asString);
        }
        
    }

	virtual void visit_simple_goal(simple_goal * p) 
	{
        if (checkingIfConstraintCouldOnlyBeFalse) {
            Literal tmp(p->getProp(),fe);                        
                        
            if(VAL::current_analysis->pred_tab.symbol_probe("=") == EPS(p->getProp()->head)->getParent()) {
                VAL::LiteralParameterIterator<VAL::parameter_symbol_list::iterator> tmpBegin = tmp.begin();
                
                VAL::parameter_symbol * a = *tmpBegin;
                ++tmpBegin;
                VAL::parameter_symbol * b = *tmpBegin;
                
                
                childCouldOnlyBe.push_back(a != b ? -1 : 1);
                
            } else {
                validateLiteral(&tmp);
                Literal* const seeIfItExists = instantiatedOp::findLiteral(&tmp);
                if (!seeIfItExists) {
                    childCouldOnlyBe.push_back(-1);
                } else {
                    
                    const pair<bool,bool> & staticInfo = LiteralAnalysis::isStatic(seeIfItExists);
                    
                    if (staticInfo.first) {                    
                        childCouldOnlyBe.push_back(staticInfo.second ? 1 : -1);
                    } else {
                        childCouldOnlyBe.push_back(0);
                    }
                }
            }
            
            return;
        }        
        
        
		assert(!inPreference || literalGoals != litToUse);
		if(VAL::current_analysis->pred_tab.symbol_probe("=") == EPS(p->getProp()->head)->getParent()) {
			Literal tmp(p->getProp(),fe);

			//validateLiteral(&tmp);

			VAL::LiteralParameterIterator<VAL::parameter_symbol_list::iterator> tmpBegin = tmp.begin();
			
			VAL::parameter_symbol * a = *tmpBegin;
			++tmpBegin;
			VAL::parameter_symbol * b = *tmpBegin;

			if (a != b) {
				litToUse->push_back(0);
			}
			return;

		};

		Literal * l = new Literal(p->getProp(),fe);
		validateLiteral(l);
		litToUse->push_back(instantiatedOp::findLiteral(l));
		delete l;
	};
	
	virtual void visit_qfied_goal(qfied_goal * p) 
	{
		if (!checkingIfConstraintCouldOnlyBeFalse) {
            if (p->getQuantifier() == E_EXISTS) postmortem_noADL();
        }

		vector<vector<VAL::const_symbol*>::const_iterator> vals(p->getVars()->size());
		vector<vector<VAL::const_symbol*>::const_iterator> starts(p->getVars()->size());
		vector<vector<VAL::const_symbol*>::const_iterator> ends(p->getVars()->size());
        vector<VAL::var_symbol *> vars(static_cast<const id_var_symbol_table*>(p->getSymTab())->numSyms());
		fe->extend(vars.size());
		int i = 0;
		int c = 1;
		for(var_symbol_list::const_iterator pi = p->getVars()->begin();
				pi != p->getVars()->end();++pi,++i)
		{
			if(instantiatedOp::getValues().find((*pi)->type) == instantiatedOp::getValues().end()) 
			{
				instantiatedOp::getValues()[(*pi)->type] = tc->range(*pi);
			};
			vals[i] = starts[i] = instantiatedOp::getValues()[(*pi)->type].begin();
			ends[i] = instantiatedOp::getValues()[(*pi)->type].end();
			if(ends[i]==starts[i]) {				
                if (checkingIfConstraintCouldOnlyBeFalse) {
                    childCouldOnlyBe.push_back(p->getQuantifier() == E_EXISTS ? -1 : 1);
                }
				return;
			}
			(*fe)[(*pi)] = *(vals[i]);
			vars[i] = *pi;
			c *= instantiatedOp::getValues()[(*pi)->type].size();
		};

        bool allFalse = true;
        bool allTrue = true;
        bool anyTrue = false;
		--i;
        int cResult;
		while(vals[i] != ends[i])
		{
// This is inefficient because it creates a copy of the environment even if the copy is never used.
// In practice, this should not be a problem because a quantified effect presumably uses the variables
// it quantifies.
			FastEnvironment * ecpy = fe;
			fe = fe->copy();                        
            
			p->getGoal()->visit(this);
            
            if (checkingIfConstraintCouldOnlyBeFalse) {
                childCouldOnlyBe.get_result(cResult);
                switch (cResult) {
                    case -1:
                    {
                        allTrue = false;
                        break;
                    }
                    case 0:
                    {
                        allFalse = false;
                        allTrue = false;
                        break;
                    }
                    case 1:
                    {
                        allFalse = false;
                        anyTrue = true;
                        break;
                    }
                }
                
            }
			fe = ecpy;

			int x = 0;
			++vals[0];
			if(vals[0] != ends[0]) (*fe)[vars[0]] = *(vals[0]);
			while(x < i && vals[x] == ends[x])
			{
				vals[x] = starts[x];
				(*fe)[vars[x]] = *(vals[x]);
				++x;
				++vals[x];
				if(vals[x] != ends[x]) (*fe)[vars[x]] = *(vals[x]);
			};
		};

        if (checkingIfConstraintCouldOnlyBeFalse) {
            if (p->getQuantifier() == E_EXISTS) {
                if (anyTrue) {
                    childCouldOnlyBe.push_back(1);
                } else if (allFalse) {
                    childCouldOnlyBe.push_back(-1);
                } else {
                    childCouldOnlyBe.push_back(0);
                }
                
            } else {
                if (allFalse) {
                    childCouldOnlyBe.push_back(-1);
                } else if (allTrue) {
                    childCouldOnlyBe.push_back(1);
                } else {
                    childCouldOnlyBe.push_back(0);
                }
            }
        }
	};
	virtual void visit_conj_goal(conj_goal * p) 
	{
        
        if (!checkingIfConstraintCouldOnlyBeFalse) {
            p->getGoals()->visit(this);
            return;
        }
        
        if (p->getGoals()->empty()) {
            childCouldOnlyBe.push_back(1);
            return;
        }
        
        bool anyFalse = false;
        bool allTrue = true;
        
        pc_list<goal*>::const_iterator goalItr = p->getGoals()->begin();
        const pc_list<goal*>::const_iterator goalEnd = p->getGoals()->end();
        
        int cResult;
        for (; goalItr != goalEnd; ++goalItr) {
            (*goalItr)->visit(this);
            
            if (checkingIfConstraintCouldOnlyBeFalse) {
                childCouldOnlyBe.get_result(cResult);
                if (cResult == -1) {
                    anyFalse = true;
                    break;
                } else if (cResult == 0) {
                    allTrue = false;
                }
            }
            
        }
        
        if (anyFalse) {
            childCouldOnlyBe.push_back(-1);
        } else if (allTrue) {
            childCouldOnlyBe.push_back(1);
        } else {
            childCouldOnlyBe.push_back(0);
        }
        
        
    };
    
	virtual void visit_disj_goal(disj_goal * p) 
	{
        if (checkingIfConstraintCouldOnlyBeFalse) {
            
            bool anyTrue = false;
            bool allFalse = true;
            
            pc_list<goal*>::const_iterator goalItr = p->getGoals()->begin();
            const pc_list<goal*>::const_iterator goalEnd = p->getGoals()->end();
        
            int cResult;
            for (; !anyTrue && goalItr != goalEnd; ++goalItr) {
                (*goalItr)->visit(this);
                
                if (checkingIfConstraintCouldOnlyBeFalse) {
                    childCouldOnlyBe.get_result(cResult);
                    if (cResult == 1) {
                        anyTrue = true;
                        break;
                    } else if (cResult == 0) {
                        allFalse = false;
                    }                    
                }                            
            }
        
                            
            if (anyTrue) {
                childCouldOnlyBe.push_back(1);
            } else if (allFalse) {
                childCouldOnlyBe.push_back(-1);
            } else {
                childCouldOnlyBe.push_back(0);
            }
            
            return;
        }
        
        postmortem_noADL();
    
    };

	virtual void visit_imply_goal(imply_goal * p) 
	{        
        if (checkingIfConstraintCouldOnlyBeFalse) {
            p->getAntecedent()->visit(this);
            
            int acv;
            childCouldOnlyBe.get_result(acv);
            
            if (acv == -1) {
                childCouldOnlyBe.push_back(1);
                return;
            }                        

            p->getConsequent()->visit(this);
            
            int ccv;
            childCouldOnlyBe.get_result(ccv);
            
            if (acv == 1) {
                childCouldOnlyBe.push_back(ccv);                
            } else {
                if (ccv == 1) {
                    childCouldOnlyBe.push_back(1);
                } else {
                    childCouldOnlyBe.push_back(0);  
                }
            }
            
            return;
        }
        
		postmortem_noADL();
	};
	virtual void visit_neg_goal(neg_goal * p) 
	{
        if (checkingIfConstraintCouldOnlyBeFalse) {
            p->getGoal()->visit(this);
            int oldVal;
            childCouldOnlyBe.get_result(oldVal);
            oldVal *= -1;
            childCouldOnlyBe.push_back(oldVal);
            return;
        }
        
		postmortem_noADL();
	};
    virtual void visit_preference(preference * p)
    {

        /*inPreference = true;
        builtPreferences.push_back(RPGBuilder::Preference(p->getName()));

        if (!inConstraints) {
            builtPreferences.back().cons = VAL::E_ATEND;
            numToUse = &(builtPreferences.back().goalNum);
            litToUse = &(builtPreferences.back().goal);            
        }
        
        p->getGoal()->visit(this);
        
        if (!inConstraints) {
            numToUse = numericGoals;
            litToUse = literalGoals;            
        }
        
        inPreference = false;*/
        
        
        inPreference = true;        
        
        if (!inConstraints) {

            auto_ptr<FastEnvironment> forPref(fe ? fe->copy() : 0);
            
            checkingIfConstraintCouldOnlyBeFalse = true;
            p->getGoal()->visit(this);
            
            int cr;
            childCouldOnlyBe.get_result(cr);
            
            checkingIfConstraintCouldOnlyBeFalse = false;
            
            if (cr == 0) {            
                builtPreferences.push_back(RPGBuilder::Preference(p->getName()));
            
                builtPreferences.back().fe = forPref.release();
                builtPreferences.back().cons = VAL::E_ATEND;            
                builtPreferences.back().parsed_goal = p->getGoal();
            } else {
                if (cr == -1) {
                    cout << "Warning: at-end preference " << p->getName() << " will always be violated, ignoring\n";                    
                    ++(prefNameToNumberOfTimesDefinitelyViolated.insert(make_pair(p->getName(),0)).first->second);
                }
                if (cr == 1) {
                    //cout << "Have an instance of " << p->getName() << " that will always be true\n";
                    prefNameToID[p->getName()].push_back(-512);
                } else {
                    //cout << "Have an instance of " << p->getName() << " that will always be false\n";
                    prefNameToID[p->getName()].push_back(-513);
                }
            }
        } else {
            builtPreferences.push_back(RPGBuilder::Preference(p->getName()));
            p->getGoal()->visit(this);
        }
        
        inPreference = false;

    };
	virtual void visit_simple_effect(simple_effect * p) 
	{
		
	};

    virtual void visit_constraint_goal(constraint_goal *cg)
    {
        if (!inPreference) {
            postmortem_noConstraints();
        }

        auto_ptr<FastEnvironment> forPref(fe ? fe->copy() : 0);

        builtPreferences.back().cons = cg->getCons();

        if (builtPreferences.back().cons != E_ALWAYS && builtPreferences.back().cons != E_SOMETIME
             && builtPreferences.back().cons != E_ATMOSTONCE && builtPreferences.back().cons != E_ATEND
             && builtPreferences.back().cons != E_SOMETIMEAFTER && builtPreferences.back().cons != E_SOMETIMEBEFORE
             ) {
            string & prefName = builtPreferences.back().name;
            postmortem_noConstraints(true,prefName.c_str());
            
            if (prefName != "anonymous") prefNameToID[prefName].push_back(-1);
            builtPreferences.pop_back();
        } else {

            bool keepPref = true;
            
            string & prefName = builtPreferences.back().name;
            
            switch(cg->getCons()) {
                case VAL::E_ALWAYS:
                case VAL::E_SOMETIME:
                case VAL::E_ATEND:
                {
                    // For these three, if the goal is tautologous, we can prune;
                    // if the goal is contradictory, we can penalise; otherwise,
                    // keep
                    checkingIfConstraintCouldOnlyBeFalse = true;
                    cg->getRequirement()->visit(this);     
                    int cr;
                    childCouldOnlyBe.get_result(cr);
                    checkingIfConstraintCouldOnlyBeFalse = false;
                    
                    if (cr == 1) {
                        keepPref = false;
                        prefNameToID[prefName].push_back(-512);
                    } else if (cr == -1) {
                        keepPref = false;
                        prefNameToID[prefName].push_back(-513);
                        ++(prefNameToNumberOfTimesDefinitelyViolated.insert(make_pair(prefName,0)).first->second);
                    }
                    break;
                }
                case VAL::E_ATMOSTONCE:
                {
                    // if this is always true, or always false, then it's permanently satisfied:
                    // - if always true, we always have it, so only have it once 
                    // - if it's never true, we never have it, so can't have it more than once

                    checkingIfConstraintCouldOnlyBeFalse = true;
                    cg->getRequirement()->visit(this);                    
                    int cr;
                    childCouldOnlyBe.get_result(cr);
                    checkingIfConstraintCouldOnlyBeFalse = false;
                    
                    if (cr != 0) {                    
                        keepPref = false;
                        prefNameToID[prefName].push_back(-512);
                    }
                    
                    break;
                }
                case VAL::E_SOMETIMEAFTER:
                {
                    // Tautologous cases:
                    // - if the goal requirement is always true, is always satisfied
                    // - if the trigger is always false, is always satisfied
                    // Contradictory case:
                    // - if the trigger is always true, and the goal is always false
                    
                    bool alwaysTrue = false;
                    bool alwaysFalse = false;
                                        
                    checkingIfConstraintCouldOnlyBeFalse = true;
                    cg->getRequirement()->visit(this);                    
                    int cr;
                    childCouldOnlyBe.get_result(cr);
                    checkingIfConstraintCouldOnlyBeFalse = false;
                    
                    if (cr == 1) {
                        alwaysTrue = true;
                    } else {
                        checkingIfConstraintCouldOnlyBeFalse = true;
                        cg->getTrigger()->visit(this);                    
                        int tr;
                        childCouldOnlyBe.get_result(tr);                        
                        checkingIfConstraintCouldOnlyBeFalse = false;
                        
                        if (tr == -1) {
                            alwaysTrue = true;
                        } else if (tr == 1 && cr == -1) {
                            alwaysFalse = true;
                        }
                    }
                    
                    if (alwaysTrue) {
                        keepPref = false;
                        prefNameToID[prefName].push_back(-512);
                    } else if (alwaysFalse) {
                        keepPref = false;                        
                        prefNameToID[prefName].push_back(-513);
                        ++(prefNameToNumberOfTimesDefinitelyViolated.insert(make_pair(prefName,0)).first->second);
                    }
                    
                    break;
                }
                case VAL::E_SOMETIMEBEFORE:
                {
                    // Tautologous cases:
                    // - if the requirement is always true, preference is always true
                    // - if the trigger is always false, preference is always true
                    // Contradictory case:
                    // - if the requirement is always false and the trigger is always true
                    
                    bool alwaysTrue = false;
                    bool alwaysFalse = false;
                    
                    checkingIfConstraintCouldOnlyBeFalse = true;
                    cg->getRequirement()->visit(this);  
                    int cr;
                    childCouldOnlyBe.get_result(cr);
                    checkingIfConstraintCouldOnlyBeFalse = false;
                    
                    if (cr == 1) {
                        alwaysTrue = true;
                    } else {
                        checkingIfConstraintCouldOnlyBeFalse = true;
                        cg->getTrigger()->visit(this);                    
                        int tr;
                        childCouldOnlyBe.get_result(tr);
                        checkingIfConstraintCouldOnlyBeFalse = false;
                        
                        if (tr == -1) {
                            alwaysTrue = true;
                        } else if (cr == -1 && tr == 1) {
                            alwaysFalse = true;
                        }                    
                    }
                    
                    if (alwaysTrue) {
                        keepPref = false;
                        prefNameToID[prefName].push_back(-512);
                    } else if (alwaysFalse) {
                        keepPref = false;                        
                        prefNameToID[prefName].push_back(-513);
                        ++(prefNameToNumberOfTimesDefinitelyViolated.insert(make_pair(prefName,0)).first->second);
                    }
                    
                    break;
                }
                default:
                {
                    cout << "Fatal internal error: should never get to this point without having matched the preference type\n";
                    exit(1);
                }
            }
            
            /*
            if(cg->getRequirement()) {
                
                numToUse = &(builtPreferences.back().goalNum);
                litToUse = &(builtPreferences.back().goal);

                cg->getRequirement()->visit(this);
            }
            
            if(cg->getTrigger()) {
                
                numToUse = &(builtPreferences.back().triggerNum);
                litToUse = &(builtPreferences.back().trigger);

                cg->getTrigger()->visit(this);
            }*/

            if (keepPref) {
                builtPreferences.back().fe = forPref.release();
                
                builtPreferences.back().parsed_goal = cg->getRequirement();
                builtPreferences.back().parsed_trigger = cg->getTrigger();
            } else {
                builtPreferences.pop_back();
            } 
        
        }

        numToUse = numericGoals;
        litToUse = literalGoals;

    };

	
	virtual void visit_problem(VAL::problem * p) 
	{

		WhereAreWeNow = PARSE_GOAL;
		inpres = false;

        inPreference = false;
		numToUse = numericGoals;
		litToUse = literalGoals;
		p->the_goal->visit(this);
		WhereAreWeNow = PARSE_UNKNOWN;
		if (p->constraints) {
			WhereAreWeNow = PARSE_CONSTRAINTS;
			inConstraints = true;
			p->constraints->visit(this);
			inConstraints = false;	
			WhereAreWeNow = PARSE_UNKNOWN;
		}
		numToUse = numericGoals;
		litToUse = literalGoals;

	};


};


class TimedPrecEffCollector : public VisitController {
private:
	VAL::TypeChecker * tc;
	bool adding;
	const VAL::operator_ * op;
	FastEnvironment * fe;

	bool inpres;
	bool checkpos;
	bool debug;
    bool valid;
    
    bool visitingWhen;

	bool visitingDuration;    
    
    RPGBuilder::NoDuplicatePair addToListPositive;
    RPGBuilder::NoDuplicatePair addToListNegative;
    list<RPGBuilder::NumericPrecondition> * addToListNumeric;
    
    RPGBuilder::NoDuplicatePair addEffToList;
    RPGBuilder::NoDuplicatePair delEffToList;
    list<RPGBuilder::NumericEffect> * addEffToListNumeric;
    
    list<vector<RPGBuilder::NoDuplicatePair > > literalAddTos;
    list<vector<RPGBuilder::NoDuplicatePair > > literalNegativeAddTos;
    list<vector<list<RPGBuilder::NumericPrecondition>* > > numericAddTos;
    
    
public:

    list<RPGBuilder::ProtoConditionalEffect*> condEffs;
    
    list<Literal*> startPrec;
    LiteralSet startPrecSet;
    list<Literal*> inv;
    LiteralSet invSet;
    list<Literal*> endPrec;
    LiteralSet endPrecSet;
    
    list<Literal*> startNegPrec;
    LiteralSet startNegPrecSet;
    list<Literal*> negInv;
    LiteralSet negInvSet;
    list<Literal*> endNegPrec;
    LiteralSet endNegPrecSet;
    
    list<RPGBuilder::NumericPrecondition> startPrecNumeric;
    list<RPGBuilder::NumericPrecondition> invNumeric;
    list<RPGBuilder::NumericPrecondition> endPrecNumeric;
    
    list<Literal*> startAddEff;
    LiteralSet startAddEffSet;
    list<Literal*> startDelEff;
    LiteralSet startDelEffSet;
    list<RPGBuilder::NumericEffect> startNumericEff;
    
    list<Literal*> endAddEff;
    LiteralSet endAddEffSet;
    list<Literal*> endDelEff;
    LiteralSet endDelEffSet;
    list<RPGBuilder::NumericEffect> endNumericEff;
    
	RPGBuilder::NumericPrecondition * durationExpression;

    bool isDurative;
    
	static map<string,set<int> > litParamCounts;
	static map<string,set<int> > pneParamCounts;
	static bool initParamCounts;

	static instantiatedOp* toBlame;

    map<string, list<int> > & prefNameToID;

    vector<list<RPGBuilder::Preference> > builtPreferences;
        
    
    TimedPrecEffCollector(const VAL::operator_ * o, map<string, list<int> > & p, FastEnvironment * f,VAL::TypeChecker * t = 0)
        : tc(t), adding(true), op(o), fe(f), inpres(true), checkpos(true), valid(true),
          addToListNumeric(0), addEffToListNumeric(0),durationExpression(0),isDurative(false), prefNameToID(p), builtPreferences(1)
	{
		debug = (GlobalSchedule::globalVerbosity & 16);
		if (!initParamCounts) {
			doInit();
		}
        visitingWhen = false;
	};

    const bool & isValid() const {
        return valid;
    }
    
	static void doInit() {
		if (current_analysis->the_domain->predicates) {
			for(pred_decl_list::const_iterator os = current_analysis->the_domain->predicates->begin();
				os != current_analysis->the_domain->predicates->end();++os) {
				litParamCounts[(*os)->getPred()->getName()].insert((*os)->getArgs()->size()); 
			}
		}
		if (current_analysis->the_domain->functions) {
			for(func_decl_list::const_iterator os = current_analysis->the_domain->functions->begin();
				os != current_analysis->the_domain->functions->end();++os) {
				pneParamCounts[(*os)->getFunction()->getName()].insert((*os)->getArgs()->size()); 
			}
		}
		initParamCounts = true;
	}

	
	virtual void visit_simple_goal(simple_goal * p) 
	{
        RPGBuilder::NoDuplicatePair & addToList = (adding ? addToListPositive : addToListNegative);
        
        if(VAL::current_analysis->pred_tab.symbol_probe("=") == EPS(p->getProp()->head)->getParent()) {
            Literal tmp(p->getProp(),fe);
            
            VAL::LiteralParameterIterator<VAL::parameter_symbol_list::iterator> tmpBegin = tmp.begin();
            
            VAL::parameter_symbol * const a = *(tmpBegin);
            ++tmpBegin;
            VAL::parameter_symbol * const b = *(tmpBegin);
            
            if (adding) {
                if (a != b) {
                    addToList.push_back((Literal*) 0);
                }
            } else {
                if (a == b) {
                    addToList.push_back((Literal*) 0);
                }
            }
            
            return;
                        
        }
        
        
		Literal * l = new Literal(p->getProp(),fe);
		
        if (debug) {
            cout << "- Looking up " << *l << " (" << p->getProp()->head << "/" << current_analysis->pred_tab.symbol_get(p->getProp()->head->getName());
            VAL::LiteralParameterIterator<VAL::parameter_symbol_list::iterator> itr = l->begin();
            VAL::LiteralParameterIterator<VAL::parameter_symbol_list::iterator> itrEnd = l->end();
            
            for (; itr != itrEnd; ++itr) {
                cout << " " << *itr;
            }
            cout << ")\n";
        }
        
        Literal * const addLit = instantiatedOp::findLiteral(l);
        
        if (!addLit) {
            if (adding) {
                addToList.push_back(addLit);
                if (debug) cout << "\t\tNull\n";
            } else {
                addToList.push_back(addLit);
                if (debug) cout << "\t\tNull, but don't care - is a negative pre\n";
            }
        } else {            
            addToList.push_back(addLit);
            if (debug) cout << "\t\t" << *(addToList.back()) << "\n";
                 
        }
        
        delete l;
        //cout << "Got " << *(addToList->back()) << " with ID " << addToList->back()->getID() << "\n";
		//assert(addToList->back()->getID() >= 0);
	};
	virtual void visit_comparison(comparison * c) {
		if (visitingDuration) {
			assert(!durationExpression); // double durations should never ever be defined!
			durationExpression = new RPGBuilder::NumericPrecondition(c->getOp(), const_cast<VAL::expression*>(c->getLHS()), const_cast<VAL::expression*>(c->getRHS()), fe, tc);
		} else {
			addToListNumeric->push_back(RPGBuilder::NumericPrecondition(c->getOp(), const_cast<VAL::expression*>(c->getLHS()), const_cast<VAL::expression*>(c->getRHS()), fe, tc));
            if (!addToListNumeric->back().valid) {
                addToListNumeric->pop_back();
                
                RPGBuilder::NoDuplicatePair & addToList = (adding ? addToListPositive : addToListNegative);
                
                // whether it's a negative or positive precondition, we assume it's false
                addToList.push_back((Literal*) 0);                
            }
		}
	};

	virtual void visit_qfied_goal(qfied_goal * p) 
	{
        if (p->getQuantifier() == E_EXISTS) postmortem_noADL();
        
        vector<vector<VAL::const_symbol*>::const_iterator> vals(p->getVars()->size());
        vector<vector<VAL::const_symbol*>::const_iterator> starts(p->getVars()->size());
        vector<vector<VAL::const_symbol*>::const_iterator> ends(p->getVars()->size());
        vector<VAL::var_symbol *> vars(static_cast<const id_var_symbol_table*>(p->getSymTab())->numSyms());
        fe->extend(vars.size());
        int i = 0;
        int c = 1;
        for(var_symbol_list::const_iterator pi = p->getVars()->begin();
            pi != p->getVars()->end();++pi,++i)
        {
            if(instantiatedOp::getValues().find((*pi)->type) == instantiatedOp::getValues().end()) 
            {
                instantiatedOp::getValues()[(*pi)->type] = tc->range(*pi);
            };
            vals[i] = starts[i] = instantiatedOp::getValues()[(*pi)->type].begin();
            ends[i] = instantiatedOp::getValues()[(*pi)->type].end();
            if(ends[i]==starts[i]) {                
                return;
            }
            (*fe)[(*pi)] = *(vals[i]);
            vars[i] = *pi;
            c *= instantiatedOp::getValues()[(*pi)->type].size();
        };
                
        --i;
        while(vals[i] != ends[i])
        {
            // This is inefficient because it creates a copy of the environment even if the copy is never used.
            // In practice, this should not be a problem because a quantified effect presumably uses the variables
            // it quantifies.
            FastEnvironment * ecpy = fe;
            fe = fe->copy();
            p->getGoal()->visit(this);
            fe = ecpy;
            
            int x = 0;
            ++vals[0];
            if(vals[0] != ends[0]) (*fe)[vars[0]] = *(vals[0]);
            while(x < i && vals[x] == ends[x])
            {
                vals[x] = starts[x];
                (*fe)[vars[x]] = *(vals[x]);
                ++x;
                ++vals[x];
                if(vals[x] != ends[x]) (*fe)[vars[x]] = *(vals[x]);
            }
        }
                
    }
    
    virtual void visit_disj_goal_internal(const goal_list * gl)
    {
        assert(adding);
        
        const RPGBuilder::NoDuplicatePair oldAddToListPositive = addToListPositive;
        const RPGBuilder::NoDuplicatePair oldAddToListNegative = addToListNegative;
        list<RPGBuilder::NumericPrecondition> * const oldAddToListNumeric = addToListNumeric;
        
        list<Literal*> conds;
        LiteralSet condsSet;
        list<Literal*> negativeConds;
        LiteralSet negativeCondsSet;
        list<RPGBuilder::NumericPrecondition> numericConds;
        
        addToListPositive = RPGBuilder::NoDuplicatePair(&conds, &condsSet);
        addToListNegative = RPGBuilder::NoDuplicatePair(&negativeConds, &negativeCondsSet);
        addToListNumeric = &numericConds;
        
        bool tautology = false;
        
        gl->visit(this);
    
        {
            list<Literal*>::iterator cItr = conds.begin();
            const list<Literal*>::iterator cEnd = conds.end();
            
            while(cItr != cEnd) {
                if (*cItr) {
                    const pair<bool,bool> & staticInfo = LiteralAnalysis::isStatic(*cItr);
                    
                    if (staticInfo.first) {
                        if (staticInfo.second) {
                            tautology = true;
                            break;
                        } else {
                            const list<Literal*>::iterator cDel = cItr++;
                            conds.erase(cItr);                            
                        }                        
                    } else {                        
                        ++cItr;
                    }
                } else {
                    const list<Literal*>::iterator cDel = cItr++;
                    conds.erase(cItr);
                }
            }
        }
        
        if (!tautology) {
            list<Literal*>::iterator cItr = negativeConds.begin();
            const list<Literal*>::iterator cEnd = negativeConds.end();
            
            while(cItr != cEnd) {
                if (!(*cItr)) {
                    tautology = true;
                    break;
                }
                const pair<bool,bool> & staticInfo = LiteralAnalysis::isStatic(*cItr);
                if (staticInfo.first) {
                    if (!staticInfo.second) {
                        tautology = true;
                        break;
                    } else {                        
                        const list<Literal*>::iterator cDel = cItr++;
                        negativeConds.erase(cItr);                                                    
                    }
                }
                ++cItr;                
            }
        }
        
        if (!tautology) {            
            if (conds.size() + negativeConds.size() + numericConds.size() > 1) postmortem_noADL();            
        }
                                                    
        addToListPositive = oldAddToListPositive;
        addToListNegative = oldAddToListNegative;        
        addToListNumeric = oldAddToListNumeric;                
        
        addToListPositive.insert(conds.begin(), conds.end());
        addToListNegative.insert(negativeConds.begin(), negativeConds.end());
        addToListNumeric->insert(addToListNumeric->end(), numericConds.begin(), numericConds.end() );
    }
            
    
    virtual void visit_conj_goal(conj_goal * p) 
    {
        if (adding) {
            p->getGoals()->visit(this);
            return;
        }
                
        adding = false;
        visit_disj_goal_internal(p->getGoals());
        adding = true;
    }

    virtual void visit_disj_goal(disj_goal * p) 
    {
        if (adding) {
            visit_disj_goal_internal(p->getGoals());
        } else {
            adding = false;
            p->getGoals()->visit(this);
            adding = true;
            return;
        } 
    }
                    
                    
	virtual void visit_timed_goal(timed_goal * p) 
	{
        const RPGBuilder::NoDuplicatePair oldAddToListPositive = addToListPositive;
        const RPGBuilder::NoDuplicatePair oldAddToListNegative = addToListNegative;
        list<RPGBuilder::NumericPrecondition> * const oldAddToListNumeric = addToListNumeric;
        
		switch(p->getTime()) {
		case VAL::E_AT_START:
        {
			if (debug) cout << "\tAt start\n";
            addToListPositive = literalAddTos.back()[0];
            addToListNegative = literalNegativeAddTos.back()[0];
            addToListNumeric = numericAddTos.back()[0];
            
			break;
        }
		case VAL::E_AT_END:
        {
            if (debug) cout << "\tAt end\n";
            addToListPositive = literalAddTos.back()[2];
            addToListNegative = literalNegativeAddTos.back()[2];
            addToListNumeric = numericAddTos.back()[2];
            break;
            
        }
		case VAL::E_OVER_ALL:
        {
            if (debug) cout << "\tOver all\n";
            addToListPositive = literalAddTos.back()[1];
            addToListNegative = literalNegativeAddTos.back()[1];
            addToListNumeric = numericAddTos.back()[1];
            
        }
		default:
			cout << "Error, unsupported precondition time specification in action schema (not start, end, or over all)\n";
			exit(0);
			break;
		};
		p->getGoal()->visit(this);
        
        addToListPositive = oldAddToListPositive;
        addToListNegative = oldAddToListNegative;        
        addToListNumeric = oldAddToListNumeric;
        
	};
    virtual void visit_imply_goal(imply_goal * g) 
    {
            
        if (!adding) {
            adding = true;
            g->getAntecedent()->visit(this);
            adding = false;
            g->getConsequent()->visit(this);
            return;
        }
                
        const RPGBuilder::NoDuplicatePair oldAddToListPositive = addToListPositive;
        const RPGBuilder::NoDuplicatePair oldAddToListNegative = addToListNegative;
        list<RPGBuilder::NumericPrecondition> * const oldAddToListNumeric = addToListNumeric;
                            
        list<Literal*> conds;
        LiteralSet condsSet;
        list<Literal*> negativeConds;
        LiteralSet negativeCondsSet;
        list<RPGBuilder::NumericPrecondition> numericConds;
        
        addToListPositive = RPGBuilder::NoDuplicatePair(&conds, &condsSet);
        addToListNegative = RPGBuilder::NoDuplicatePair(&negativeConds, &negativeCondsSet);
        addToListNumeric = &numericConds;
                            
        literalAddTos.push_back(vector<RPGBuilder::NoDuplicatePair>(3, addToListPositive));
        literalNegativeAddTos.push_back(vector<RPGBuilder::NoDuplicatePair>(3, addToListNegative));
        numericAddTos.push_back(vector<list<RPGBuilder::NumericPrecondition> * >(3, &numericConds));
            
        adding = false;
        g->getAntecedent()->visit(this);
        adding = true;
        list<Literal*>::iterator cItr = negativeConds.begin();
        const list<Literal*>::iterator cEnd = negativeConds.end();
        
        for (; cItr != cEnd; ++cItr) {
            Literal* const currLit = *cItr;
            if (currLit) {
                const pair<bool,bool> & staticInfo = LiteralAnalysis::isStatic(currLit);
                if (staticInfo.first) {
                    if (!staticInfo.second) {
                        literalAddTos.pop_back();
                        literalNegativeAddTos.pop_back();
                        numericAddTos.pop_back();
                        addToListPositive = oldAddToListPositive;
                        addToListNegative = oldAddToListNegative;
                        addToListNumeric = oldAddToListNumeric;
                        return;
                    }
                } else {
                    break;
                }
            } else {
                literalAddTos.pop_back();
                literalNegativeAddTos.pop_back();
                numericAddTos.pop_back();
                addToListPositive = oldAddToListPositive;
                addToListNegative = oldAddToListNegative;
                addToListNumeric = oldAddToListNumeric;
                return;
            }
        }
        if (cItr != cEnd || !numericConds.empty()) postmortem_noADL();
        
        literalAddTos.pop_back();
        numericAddTos.pop_back();
        addToListPositive = oldAddToListPositive;
        addToListNegative = oldAddToListNegative;
        addToListNumeric = oldAddToListNumeric;
        g->getConsequent()->visit(this);            
    };
        
    virtual void visit_neg_goal(neg_goal * p) 
    {
        
        const bool oldAdding = adding;
        adding = !oldAdding;
        p->getGoal()->visit(this);
        adding = oldAdding;
        
    };
            
    virtual void visit_preference(preference * p)
    {
        builtPreferences[0].push_back(RPGBuilder::Preference(p->getName()));
        
        
        if (fe) {
            builtPreferences[0].back().fe = fe->copy();
        }
        builtPreferences[0].back().cons = VAL::E_ATEND;            
        builtPreferences[0].back().parsed_goal = p->getGoal();                                                
        builtPreferences[0].back().attachedToOperator = toBlame->getID();
        
    }
    
	virtual void visit_constraint_goal(constraint_goal *cg)
    {
        if(cg->getRequirement()) {
                cg->getRequirement()->visit(this);
        }
        
        if(cg->getTrigger()) {
                cg->getTrigger()->visit(this);
        }
    }

	virtual void visit_timed_effect(timed_effect * p) 
	{
        const RPGBuilder::NoDuplicatePair oldAdd = addEffToList;
        const RPGBuilder::NoDuplicatePair oldDel = delEffToList;
        list<RPGBuilder::NumericEffect> * const oldNum = addEffToListNumeric;
        
        switch(p->ts) {
            case VAL::E_AT_END:
            {
                if (debug) cout << "\tAt end\n";
                addEffToList = RPGBuilder::NoDuplicatePair(&endAddEff, &endAddEffSet);
                delEffToList = RPGBuilder::NoDuplicatePair(&endDelEff, &endDelEffSet);
                addEffToListNumeric = &endNumericEff;            
                break;
            }
            default:
            {
                if (debug) cout << "\tAt start\n";
                addEffToList = RPGBuilder::NoDuplicatePair(&startAddEff, &startAddEffSet);
                delEffToList = RPGBuilder::NoDuplicatePair(&startDelEff, &startDelEffSet);
                addEffToListNumeric = &startNumericEff;            
                break;

            }
        
        };
		p->effs->visit(this);

        addEffToList = oldAdd;
        delEffToList = oldDel;
        addEffToListNumeric = oldNum;
        
	};

	virtual void visit_simple_effect(simple_effect * p) 
	{
        Literal * l = new Literal(p->prop,fe);
        
        validateLiteral(l);
        
        if (adding) {
            addEffToList.push_back(instantiatedOp::findLiteral(l));
            if (debug) {
                if (addEffToList.back()) {
                    cout << "\t\t" << *(addEffToList.back()) << "\n";
                } else {
                    cout << "\t\tNull\n";
                }
            }
                        
            delete l;
        } else {
            delEffToList.push_back(instantiatedOp::findLiteral(l));
            if (debug) {
                if (delEffToList.back()) {
                    cout << "\t\tnot " << *(delEffToList.back()) << "\n";
                } else {
                    cout << "\t\tnot Null\n";
                }
            }                        
            delete l;
        }
    };
    
    
    virtual void visit_forall_effect(forall_effect * p) 
    {
            
        vector<vector<VAL::const_symbol*>::const_iterator> vals(p->getVarsList()->size());
        vector<vector<VAL::const_symbol*>::const_iterator> starts(p->getVarsList()->size());
        vector<vector<VAL::const_symbol*>::const_iterator> ends(p->getVarsList()->size());
        vector<VAL::var_symbol *> vars(p->getVars()->size());
        fe->extend(vars.size());
        int i = 0;
        int c = 1;
        for(var_symbol_list::const_iterator pi = p->getVarsList()->begin();
            pi != p->getVarsList()->end();++pi,++i)
        {
            if(instantiatedOp::getValues().find((*pi)->type) == instantiatedOp::getValues().end()) 
            {
                instantiatedOp::getValues()[(*pi)->type] = tc->range(*pi);
            };
            vals[i] = starts[i] = instantiatedOp::getValues()[(*pi)->type].begin();
            ends[i] = instantiatedOp::getValues()[(*pi)->type].end();
            if(ends[i]==starts[i]) return;
                        (*fe)[(*pi)] = *(vals[i]);
            vars[i] = *pi;
            c *= instantiatedOp::getValues()[(*pi)->type].size();
        };
        
        --i;
        while(vals[i] != ends[i])
        {
            FastEnvironment * ecpy = fe;
            fe = fe->copy();
            p->getEffects()->visit(this);
            fe = ecpy;
            
            int x = 0;
            ++vals[0];
            if(vals[0] != ends[0]) (*fe)[vars[0]] = *(vals[0]);
            while(x < i && vals[x] == ends[x])
            {
                vals[x] = starts[x];
                (*fe)[vars[x]] = *(vals[x]);
                ++x;
                ++vals[x];
                if(vals[x] != ends[x]) (*fe)[vars[x]] = *(vals[x]);
            };
        };
    }
            
    
	virtual void visit_effect_lists(effect_lists * p) 
	{
		p->add_effects.pc_list<simple_effect*>::visit(this);
		p->forall_effects.pc_list<forall_effect*>::visit(this);
		p->cond_effects.pc_list<cond_effect*>::visit(this);
		p->timed_effects.pc_list<timed_effect*>::visit(this);
		bool whatwas = adding;
		adding = !adding;
		p->del_effects.pc_list<simple_effect*>::visit(this);
		adding = whatwas;
		p->assign_effects.pc_list<assignment*>::visit(this);
	};

	
	virtual void visit_operator_(VAL::operator_ * p) 
	{

        if (debug) cout << "Going through preconditions\n";
        
        adding = true;
        inpres = true;
        checkpos = true;
        WhereAreWeNow = PARSE_PRECONDITION;        
        p->precondition->visit(this);        
        inpres = false;
        
        if (debug) cout << "Going through effects\n";
        
        adding = true;
        WhereAreWeNow = PARSE_EFFECT;
        p->effects->visit(this);
        WhereAreWeNow = PARSE_UNKNOWN;
	};
    
	virtual void visit_action(VAL::action * p)
	{
        isDurative = false;
        addToListPositive = RPGBuilder::NoDuplicatePair(&startPrec, &startPrecSet);
        addToListNegative = RPGBuilder::NoDuplicatePair(&startNegPrec, &startNegPrecSet);
        addToListNumeric = &startPrecNumeric;
        addEffToList = RPGBuilder::NoDuplicatePair(&startAddEff, &startAddEffSet);
        delEffToList = RPGBuilder::NoDuplicatePair(&startDelEff, &startDelEffSet);
        addEffToListNumeric = &startNumericEff;
        
        literalAddTos.push_back(vector<RPGBuilder::NoDuplicatePair>(3));
        literalNegativeAddTos.push_back(vector<RPGBuilder::NoDuplicatePair>(3));
        numericAddTos.push_back(vector<list<RPGBuilder::NumericPrecondition> * >(3, (list<RPGBuilder::NumericPrecondition>*) 0));
        
        literalAddTos.back()[0] = addToListPositive;
        literalNegativeAddTos.back()[0] = addToListNegative;
        numericAddTos.back()[0] = addToListNumeric;
        
        visitingDuration = false;
        visit_operator_(p); //static_cast<VAL::operator_*>(p));
        
        
	};
	virtual void visit_durative_action(VAL::durative_action * p) 
	{
		visit_operator_(p); //static_cast<VAL::operator_*>(p));
		visitingDuration = true;
		p->dur_constraint->visit(this);
		visitingDuration = false;
		
	};
	virtual void visit_process(VAL::process * p)
	{
		visit_operator_(p);
	};
	virtual void visit_event(VAL::event * p)
	{
		visit_operator_(p);
	};
	virtual void visit_problem(VAL::problem * p) 
	{
		p->initial_state->visit(this);
		inpres = false;
		p->the_goal->visit(this);
	};

	virtual void visit_assignment(assignment * a) {
		PNE p(a->getFTerm(),fe);
		PNE * pne = instantiatedOp::getPNE(&p);
		addEffToListNumeric->push_back(RPGBuilder::NumericEffect(a->getOp(), pne->getStateID(), const_cast<VAL::expression*>(a->getExpr()), fe, tc));
        if (!addEffToListNumeric->back().valid) {
            valid = false;
        }
	};
    
    virtual void visit_cond_effect(cond_effect * p) 
    {
        if (visitingWhen) {
            postmortem_noNestedWhens();
        }
                
        visitingWhen=true;
        
        condEffs.push_back(new RPGBuilder::ProtoConditionalEffect());
        
        RPGBuilder::ProtoConditionalEffect & ce = *(condEffs.back());
        
        literalAddTos.push_back(vector<RPGBuilder::NoDuplicatePair>(3));
        literalNegativeAddTos.push_back(vector<RPGBuilder::NoDuplicatePair>(3));        
        numericAddTos.push_back(vector<list<RPGBuilder::NumericPrecondition> * >(3));
        
        literalAddTos.back()[0] = RPGBuilder::NoDuplicatePair(&(ce.startPrec), &(ce.startPrecSet));
        literalAddTos.back()[1] = RPGBuilder::NoDuplicatePair(&(ce.inv), &(ce.invSet));
        literalAddTos.back()[2] = RPGBuilder::NoDuplicatePair(&(ce.endPrec), &(ce.endPrecSet));
        
        literalNegativeAddTos.back()[0] = RPGBuilder::NoDuplicatePair(&(ce.startNegPrec), &(ce.startNegPrecSet));
        literalNegativeAddTos.back()[1] = RPGBuilder::NoDuplicatePair(&(ce.negInv), &(ce.negInvSet));
        literalNegativeAddTos.back()[2] = RPGBuilder::NoDuplicatePair(&(ce.endNegPrec), &(ce.endNegPrecSet));
        
        numericAddTos.back()[0] = &(ce.startPrecNumeric);
        numericAddTos.back()[1] = &(ce.invNumeric);
        numericAddTos.back()[2] = &(ce.endPrecNumeric);
        
        const RPGBuilder::NoDuplicatePair oldAddToListPositive = addToListPositive;
        const RPGBuilder::NoDuplicatePair oldAddToListNegative = addToListNegative;
        list<RPGBuilder::NumericPrecondition> * const oldAddToListNumeric = addToListNumeric;
        const RPGBuilder::NoDuplicatePair oldAddEffToList = addEffToList;
        const RPGBuilder::NoDuplicatePair oldDelEffToList = delEffToList;
        list<RPGBuilder::NumericEffect> * const oldAddEffToListNumeric = addEffToListNumeric;
        
        addToListPositive = literalAddTos.back()[0];
        addToListNegative = literalNegativeAddTos.back()[0];
        addToListNumeric = &(ce.startPrecNumeric);
        addEffToList = RPGBuilder::NoDuplicatePair(&(ce.startAddEff), &(ce.startAddEffSet));
        delEffToList = RPGBuilder::NoDuplicatePair(&(ce.startDelEff), &(ce.startDelEffSet));
        addEffToListNumeric = &(ce.startNumericEff);
        
        p->getCondition()->visit(this);
        
        const bool oldValid = valid;
        
        valid = true;        
        p->getEffects()->visit(this);
        
        ce.validEffects = valid;
        
        valid = oldValid;
        
        addToListPositive = oldAddToListPositive;
        addToListNegative = oldAddToListNegative;
        addToListNumeric = oldAddToListNumeric;
        addEffToList = oldAddEffToList;
        delEffToList = oldDelEffToList;
        addEffToListNumeric = oldAddEffToListNumeric;
        
        
        visitingWhen=false;
        
        literalAddTos.pop_back();
        numericAddTos.pop_back();
        
        visitingWhen = false;
        
    }

};

instantiatedOp* TimedPrecEffCollector::toBlame = 0;
map<string,set<int> > TimedPrecEffCollector::litParamCounts;
map<string,set<int> > TimedPrecEffCollector::pneParamCounts;
bool TimedPrecEffCollector::initParamCounts = false;

void validatePNE(PNE * c) {
	const set<int> & expectedArgs = TimedPrecEffCollector::pneParamCounts[c->getHead()->getName()];
	const int givenArgs = c->getFunc()->getArgs()->size();
	if (expectedArgs.find(givenArgs) == expectedArgs.end()) {
		string actionname;		
		string predname;
		string lit;
		bool actLabel = false;

		if (TimedPrecEffCollector::toBlame) {
			ostringstream o;
			o << *(TimedPrecEffCollector::toBlame);
			actionname = o.str();
			actLabel = true;
		}

		{
			ostringstream o;
			o << c->getFunc()->getFunction()->getName();
			predname = o.str();
		}

		{
			ostringstream o;
			o << *c;
			lit = o.str();
		}

		postmortem_wrongNumberOfFluentArguments(actionname, actLabel, WhereAreWeNow, predname, lit, givenArgs, expectedArgs);
	}
			
};

void validateLiteral(Literal * l) {

	const set<int> & expectedArgs = TimedPrecEffCollector::litParamCounts[l->getProp()->head->getName()];
	const int givenArgs = l->getProp()->args->size();
	if (expectedArgs.find(givenArgs) == expectedArgs.end()) {
		string actionname;
		string predname;
		string lit;

		bool actLabel = false;

		if (TimedPrecEffCollector::toBlame) {
			ostringstream o;
			o << *(TimedPrecEffCollector::toBlame);
			actionname = o.str();
			actLabel = true;
		}

		{
			ostringstream o;
			o << l->getProp()->head->getName();
			predname = o.str();
		}

		{
			ostringstream o;
			o << *l;
			lit = o.str();
		}

		postmortem_wrongNumberOfPredicateArguments(actionname, actLabel, WhereAreWeNow, predname, lit, givenArgs, expectedArgs);
	}
}


class InitialStateCollector : public VisitController {

private:
	VAL::TypeChecker * tc;
	bool adding;
	const VAL::operator_ * op;
	FastEnvironment * fe;

	bool inpres;
	bool checkpos;
	int assignTo;
    const PNE* assignToPNE;
	bool TIL;
	
public:

	LiteralSet initialState;
	vector<double> initialFluents;
    vector<bool> initialFluentsDefined;

	LiteralSet tilAddSet;
	LiteralSet tilDeleteSet;
	map<double, RPGBuilder::FakeTILAction> timedInitialLiterals;


	InitialStateCollector(const VAL::operator_ * o,FastEnvironment * f,VAL::TypeChecker * t = 0) :
		tc(t), adding(true), op(o), fe(f), inpres(true), checkpos(true), assignTo(-1), TIL(false)
	{
        const int ifSize = instantiatedOp::howManyNonStaticPNEs();
		initialFluents.resize(ifSize,0.0);
        initialFluentsDefined.resize(ifSize,false);
	};
	
	virtual void visit_simple_goal(simple_goal * p) 
	{
		
	};
	virtual void visit_qfied_goal(qfied_goal * p) 
	{p->getGoal()->visit(this);};
	virtual void visit_conj_goal(conj_goal * p) 
	{p->getGoals()->visit(this);};
	virtual void visit_disj_goal(disj_goal * p) 
	{p->getGoals()->visit(this);};
	virtual void visit_timed_goal(timed_goal * p) 
	{p->getGoal()->visit(this);};
	virtual void visit_imply_goal(imply_goal * p) 
	{
		p->getAntecedent()->visit(this);
		p->getConsequent()->visit(this);
	};
	virtual void visit_neg_goal(neg_goal * p) 
	{
		bool oldcheck = checkpos;
		checkpos = !checkpos;
		p->getGoal()->visit(this);
		checkpos = oldcheck;
	};
    virtual void visit_preference(preference * p)
    {
    	p->getGoal()->visit(this);
    };
    
    virtual void visit_simple_effect(simple_effect * p) 
	{
		if (TIL) {
			if (adding) {
				Literal * l = new Literal(p->prop,fe);
				tilAddSet.insert(instantiatedOp::findLiteral(l));
				delete l;
			} else {
				Literal * l = new Literal(p->prop,fe);
				tilDeleteSet.insert(instantiatedOp::findLiteral(l));
				delete l;
			}
		} else {

			if (adding) {
				Literal * l = new Literal(p->prop,fe);
				initialState.insert(instantiatedOp::findLiteral(l));
				delete l;
			}
		}
	}
	
	virtual void visit_constraint_goal(constraint_goal *cg) {
        if(cg->getRequirement()) {
            cg->getRequirement()->visit(this);
        }
        
        if(cg->getTrigger()) {
            cg->getTrigger()->visit(this);
        }
    }

    virtual void visit_forall_effect(forall_effect * p) {
        if (!fe) {
            fe = new FastEnvironment(0);
        }
        
        vector<vector<VAL::const_symbol*>::const_iterator> vals(p->getVarsList()->size());
        vector<vector<VAL::const_symbol*>::const_iterator> starts(p->getVarsList()->size());
        vector<vector<VAL::const_symbol*>::const_iterator> ends(p->getVarsList()->size());
        vector<VAL::var_symbol *> vars(p->getVarsList()->size());
        fe->extend(vars.size());
        int i = 0;
        int c = 1;
        for(var_symbol_list::const_iterator pi = p->getVarsList()->begin();
            pi != p->getVarsList()->end();++pi,++i)
        {
            if(instantiatedOp::getValues().find((*pi)->type) == instantiatedOp::getValues().end()) 
            {
                instantiatedOp::getValues()[(*pi)->type] = tc->range(*pi);
            };
            vals[i] = starts[i] = instantiatedOp::getValues()[(*pi)->type].begin();
            ends[i] = instantiatedOp::getValues()[(*pi)->type].end();
            if(ends[i]==starts[i]) return;
                        (*fe)[(*pi)] = *(vals[i]);
            vars[i] = *pi;
            c *= instantiatedOp::getValues()[(*pi)->type].size();
        };
        
        --i;
        while(vals[i] != ends[i])
        {
            FastEnvironment * ecpy = fe;
            fe = fe->copy();
            p->getEffects()->visit(this);
            fe = ecpy;
            
            int x = 0;
            ++vals[0];
            if(vals[0] != ends[0]) (*fe)[vars[0]] = *(vals[0]);
            while(x < i && vals[x] == ends[x])
            {
                vals[x] = starts[x];
                (*fe)[vars[x]] = *(vals[x]);
                ++x;
                ++vals[x];
                if(vals[x] != ends[x]) (*fe)[vars[x]] = *(vals[x]);
            };
        };
        
                
	};
	virtual void visit_cond_effect(cond_effect * p) 
	{
		p->getCondition()->visit(this);
		p->getEffects()->visit(this);
	};
	virtual void visit_timed_effect(timed_effect * p) 
	{
		p->effs->visit(this);
	};
	virtual void visit_timed_initial_literal(timed_initial_literal * p)
	{
		const double time_stamp	= p->time_stamp;
		TIL = true;
		p->effs->visit(this);
		map<double, RPGBuilder::FakeTILAction>::iterator lookup = timedInitialLiterals.find(time_stamp);
		if (lookup == timedInitialLiterals.end()) {
			timedInitialLiterals.insert(pair<double, RPGBuilder::FakeTILAction>(time_stamp, RPGBuilder::FakeTILAction(time_stamp, tilAddSet, tilDeleteSet)));
		} else {
			lookup->second.mergeIn(tilAddSet, tilDeleteSet);
		}
		tilAddSet.clear();
		tilDeleteSet.clear();
		TIL = false;
	};
	virtual void visit_effect_lists(effect_lists * p) 
	{
		p->add_effects.pc_list<simple_effect*>::visit(this);
		p->forall_effects.pc_list<forall_effect*>::visit(this);
		p->cond_effects.pc_list<cond_effect*>::visit(this);
		p->timed_effects.pc_list<timed_effect*>::visit(this);
		bool whatwas = adding;
		adding = !adding;
		p->del_effects.pc_list<simple_effect*>::visit(this);
		adding = whatwas;
		p->assign_effects.pc_list<assignment*>::visit(this);
	};
	virtual void visit_operator_(VAL::operator_ * p) 
	{
		inpres = true;
		checkpos = true;
		p->precondition->visit(this);
		inpres = false;
		
		adding = true;
		p->effects->visit(this);
	};
	virtual void visit_action(VAL::action * p)
	{
		visit_operator_(p); //static_cast<VAL::operator_*>(p));
	};
	virtual void visit_durative_action(VAL::durative_action * p) 
	{
		visit_operator_(p); //static_cast<VAL::operator_*>(p));
	};
	virtual void visit_process(VAL::process * p)
	{
		visit_operator_(p);
	};
	virtual void visit_event(VAL::event * p)
	{
		visit_operator_(p);
	};
	virtual void visit_problem(VAL::problem * p) 
	{
		p->initial_state->visit(this);
		//inpres = false;
		//p->the_goal->visit(this);
	};

	virtual void visit_assignment(assignment * a) 
	{
		const func_term * ft = a->getFTerm();
		PNE * const pne = new PNE(ft,fe);
        assignToPNE = instantiatedOp::getPNE(pne);
		delete pne;
        
        assignTo = assignToPNE->getStateID();
        if (assignTo != -1) {
            a->getExpr()->visit(this);
        }
		assignTo = -1;
        assignToPNE = 0;
		
	};

	virtual void visit_plus_expression(plus_expression *) {
		cout << "Can only handle assignments of fluents to non-formulaic values in initial state\n";
		assert(false);
	};
	virtual void visit_minus_expression(minus_expression *) {
		cout << "Can only handle assignments of fluents to non-formulaic values in initial state\n";
		assert(false);
	};
	virtual void visit_mul_expression(mul_expression *) {
		cout << "Can only handle assignments of fluents to non-formulaic values in initial state\n";
		assert(false);
	};
	virtual void visit_div_expression(div_expression *) {
		cout << "Can only handle assignments of fluents to non-formulaic values in initial state\n";
		assert(false);
	};
	virtual void visit_uminus_expression(uminus_expression *) {
		cout << "Can only handle assignments of fluents to non-formulaic values in initial state\n";
		assert(false);
	};
	
    
	virtual void visit_int_expression(int_expression * e) {
		assert(assignTo != -1);
        if (initialFluentsDefined[assignTo]) {
            postmortem_duplicateInitialStateValue(assignToPNE, initialFluents[assignTo], e->double_value());
        }
		initialFluents[assignTo] = e->double_value();
        initialFluentsDefined[assignTo] = true;
	};

	virtual void visit_float_expression(float_expression * e) {
		assert(assignTo != -1);
        if (initialFluentsDefined[assignTo]) {
            postmortem_duplicateInitialStateValue(assignToPNE, initialFluents[assignTo], e->double_value());
        }
		initialFluents[assignTo] = e->double_value();
        initialFluentsDefined[assignTo] = true;
	};
};

bool RPGBuilder::RPGdebug = false;
bool RPGBuilder::problemIsNotTemporal = true;

vector<list<pair<int, VAL::time_spec> > > RPGBuilder::preconditionsToActions;
list<pair<int, VAL::time_spec> > RPGBuilder::preconditionlessActions;

vector<vector<int> > RPGBuilder::actionsToStartPreferences;

vector<list<Literal*> > RPGBuilder::actionsToStartPreconditions;
vector<list<Literal*> > RPGBuilder::actionsToInvariants;
vector<list<Literal*> > RPGBuilder::actionsToEndPreconditions;

vector<list<Literal*> > RPGBuilder::actionsToStartEffects;
vector<list<Literal*> > RPGBuilder::actionsToStartNegativeEffects;
vector<list<Literal*> > RPGBuilder::actionsToEndEffects;
vector<list<Literal*> > RPGBuilder::actionsToEndNegativeEffects;

vector<list<pair<int, VAL::time_spec> > > RPGBuilder::effectsToActions;
vector<list<pair<int, VAL::time_spec> > > RPGBuilder::negativeEffectsToActions;

vector<RPGBuilder::NumericPrecondition*> RPGBuilder::durationExpressions;
vector<list<RPGBuilder::ProtoConditionalEffect*> > RPGBuilder::actionsToRawConditionalEffects;

vector<double> RPGBuilder::actionsToDurations;

vector<list<RPGBuilder::NumericPrecondition> > RPGBuilder::actionsToStartNumericPreconditions;
vector<list<RPGBuilder::NumericPrecondition> > RPGBuilder::actionsToNumericInvariants;
vector<list<RPGBuilder::NumericPrecondition> > RPGBuilder::actionsToEndNumericPreconditions;

vector<list<RPGBuilder::NumericEffect> > RPGBuilder::actionsToStartNumericEffects;
vector<list<RPGBuilder::NumericEffect> > RPGBuilder::actionsToEndNumericEffects;


vector<int> RPGBuilder::initialUnsatisfiedStartPreconditions;
vector<int> RPGBuilder::initialUnsatisfiedInvariants;
vector<int> RPGBuilder::initialUnsatisfiedEndPreconditions;

vector<double> RPGBuilder::achievedInLayer;
vector<double> RPGBuilder::achievedInLayerReset;
vector<pair<int, VAL::time_spec> > RPGBuilder::achievedBy;
vector<pair<int, VAL::time_spec> > RPGBuilder::achievedByReset;

vector<double> RPGBuilder::negativeAchievedInLayer;
vector<double> RPGBuilder::negativeAchievedInLayerReset;
vector<pair<int, VAL::time_spec> > RPGBuilder::negativeAchievedBy;
vector<pair<int, VAL::time_spec> > RPGBuilder::negativeAchievedByReset;

vector<double> RPGBuilder::numericAchievedInLayer;
vector<double> RPGBuilder::numericAchievedInLayerReset;
vector<ActionFluentModification*> RPGBuilder::numericAchievedBy;
vector<ActionFluentModification*> RPGBuilder::numericAchievedByReset;

vector<double> RPGBuilder::negativeNumericAchievedInLayer;
vector<double> RPGBuilder::negativeNumericAchievedInLayerReset;
vector<ActionFluentModification*> RPGBuilder::negativeNumericAchievedBy;
vector<ActionFluentModification*> RPGBuilder::negativeNumericAchievedByReset;

vector<Literal*> RPGBuilder::literals;
vector<instantiatedOp*> RPGBuilder::instantiatedOps;
vector<bool> RPGBuilder::rogueActions;
vector<PNE*> RPGBuilder::pnes;

int RPGBuilder::taskPrefCount;
int RPGBuilder::preconditionPrefCount;

bool RPGBuilder::initialisedNumericPreTable = false;
vector<RPGBuilder::RPGNumericPrecondition> RPGBuilder::rpgNumericPreconditions;
vector<list<pair<int, VAL::time_spec> > > RPGBuilder::rpgNumericPreconditionsToActions;

vector<RPGBuilder::RPGNumericEffect> RPGBuilder::rpgNumericEffects;
vector<list<pair<int, VAL::time_spec> > > RPGBuilder::rpgNumericEffectsToActions;

vector<list<int> > RPGBuilder::actionsToRPGNumericStartEffects;
vector<list<int> > RPGBuilder::actionsToRPGNumericEndEffects;


vector<RPGBuilder::ArtificialVariable> RPGBuilder::rpgArtificialVariables;
vector<list<int> > RPGBuilder::rpgArtificialVariablesToPreconditions;
vector<list<int> > RPGBuilder::rpgPositiveVariablesToPreconditions;
vector<list<int> > RPGBuilder::rpgNegativeVariablesToPreconditions;

vector<list<int> > RPGBuilder::actionsToRPGNumericStartPreconditions;
vector<list<int> > RPGBuilder::actionsToRPGNumericInvariants;
vector<list<int> > RPGBuilder::actionsToRPGNumericEndPreconditions;
vector<list<int> > RPGBuilder::actionsToProcessedStartRPGNumericPreconditions;

vector<list<int> > RPGBuilder::rpgVariableDependencies;
vector<list<int> > RPGBuilder::variablesToRPGNumericPreconditions;

list<RPGBuilder::NumericPrecondition> RPGBuilder::numericGoals;
list<pair<int, int> > RPGBuilder::numericRPGGoals;
list<Literal*> RPGBuilder::literalGoals;

vector<int> RPGBuilder::initialUnsatisfiedNumericStartPreconditions;
vector<int> RPGBuilder::initialUnsatisfiedNumericInvariants;
vector<int> RPGBuilder::initialUnsatisfiedNumericEndPreconditions;


vector<list<pair<int, VAL::time_spec> > > RPGBuilder::processedPreconditionsToActions;
vector<list<Literal*> > RPGBuilder::actionsToProcessedStartPreconditions;
vector<int> RPGBuilder::initialUnsatisfiedProcessedStartPreconditions;

vector<list<pair<int, VAL::time_spec> > > RPGBuilder::processedRPGNumericPreconditionsToActions;
vector<list<RPGBuilder::NumericPrecondition> > RPGBuilder::actionsToProcessedStartNumericPreconditions;
vector<int> RPGBuilder::initialUnsatisfiedProcessedStartNumericPreconditions;

vector<list<int> > RPGBuilder::mentionedInFluentInvariants;

list<RPGBuilder::FakeTILAction> RPGBuilder::timedInitialLiterals;
vector<RPGBuilder::FakeTILAction*> RPGBuilder::timedInitialLiteralsVector;
vector<RPGBuilder::KShotFormula*> RPGBuilder::kShotFormulae;
vector<bool> RPGBuilder::selfMutexes;
vector<bool> RPGBuilder::oneShotLiterals;

vector<double> RPGBuilder::maxNeeded;
vector<dominance_constraint> RPGBuilder::dominanceConstraints;
map<int, int> RPGBuilder::uninterestingnessCriteria;

vector<list<int> > RPGBuilder::realVariablesToRPGEffects;
vector<list<int> > RPGBuilder::numericSubsumes;

RPGBuilder::Metric * RPGBuilder::theMetric = 0;
set<int> RPGBuilder::metricVars;
vector<RPGBuilder::Preference> RPGBuilder::preferences;
map<string,list<int> > RPGBuilder::prefNameToID;
map<string,int> RPGBuilder::prefNameToNumberOfTimesDefinitelyViolated;
double RPGBuilder::permanentPreferenceViolations = 0.0;

void RPGBuilder::initialise() {
	RPGdebug = (GlobalSchedule::globalVerbosity & 16);
	SimpleEvaluator::setInitialState();
	for(operator_list::const_iterator os = current_analysis->the_domain->ops->begin();
				os != current_analysis->the_domain->ops->end();++os)
	{
		if (RPGdebug) cout << (*os)->name->getName() << "\n";
		instantiatedOp::instantiate(*os,current_analysis->the_problem,*theTC);
		if (RPGdebug) cout << instantiatedOp::howMany() << " so far\n";
	};
	
    {
        int fpass = 1;
        int numBefore;
        do {
            if (RPGdebug) cout << "\nCollecting and filter, pass " << fpass << "\n";
            instantiatedOp::createAllLiterals(current_analysis->the_problem,theTC);
            if (RPGdebug && GlobalSchedule::globalVerbosity & 65536) instantiatedOp::writeAllLiterals(cout);
            
            numBefore = instantiatedOp::howMany();
            if (RPGdebug) {
                cout << "\tNumber of operators before filtering: " << instantiatedOp::howMany() << "\n";
            }
            instantiatedOp::filterOps(theTC);
                            
            if (RPGdebug) {
                cout << "\tNumber of operators after filtering: " << instantiatedOp::howMany() << "\n";
            }
            ++fpass;
        } while (instantiatedOp::howMany() < numBefore);
    }
    
    if (RPGdebug && GlobalSchedule::globalVerbosity & 65536) instantiatedOp::writeAll(cout);
        
    instantiatedOp::preventFurtherModificationOfLiterals();
    
    #ifdef ENABLE_DEBUGGING_HOOKS
    GlobalSchedule::markThatActionsInPlanHaveToBeKept();
    #endif
    
    instantiatedOp::assignStateIDsToNonStaticLiteralsAndPNEs();
    
    LiteralAnalysis::getBasicStaticInformationFromTIM();
    
	if (RPGdebug) cout << "\nCaching action-literal dependencies\n";

	const int operatorCount = instantiatedOp::howMany();
	const int literalCount = instantiatedOp::howManyNonStaticLiterals();
	const int pneCount = instantiatedOp::howManyNonStaticPNEs();

	actionsToStartEffects = vector<list<Literal*> >(operatorCount);
	actionsToStartNegativeEffects = vector<list<Literal*> >(operatorCount);
	actionsToEndEffects = vector<list<Literal*> >(operatorCount);
	actionsToEndNegativeEffects = vector<list<Literal*> >(operatorCount);

    actionsToStartPreferences.resize(operatorCount, vector<int>(0));
    
	actionsToStartPreconditions = vector<list<Literal*> >(operatorCount);
	actionsToInvariants = vector<list<Literal*> >(operatorCount);
	actionsToEndPreconditions = vector<list<Literal*> >(operatorCount);
        
	actionsToProcessedStartPreconditions = vector<list<Literal*> >(operatorCount);

    actionsToRawConditionalEffects.resize(operatorCount);
    
	actionsToStartNumericEffects = vector<list<NumericEffect> >(operatorCount);
	actionsToEndNumericEffects = vector<list<NumericEffect> >(operatorCount);

	actionsToStartNumericPreconditions = vector<list<NumericPrecondition> >(operatorCount);
	actionsToNumericInvariants = vector<list<NumericPrecondition> >(operatorCount);
	actionsToEndNumericPreconditions = vector<list<NumericPrecondition> >(operatorCount);

//	actionsToPositiveNumericEffects = vector<list<SimpleNumericEffect> >(operatorCount);
//	actionsToNegativeNumericEffects = vector<list<pair<int, double> > >(operatorCount);
	//actionsToNumericPreconditions   = vector<list<pair<int, double> > >(operatorCount);

	preconditionsToActions = vector<list<pair<int, VAL::time_spec> > >(literalCount);
	processedPreconditionsToActions = vector<list<pair<int, VAL::time_spec> > >(literalCount);
	
	effectsToActions = vector<list<pair<int, VAL::time_spec> > >(literalCount);
	negativeEffectsToActions = vector<list<pair<int, VAL::time_spec> > >(literalCount);
//	positiveNumericEffectsToActions = vector<list<pair<int, double> > >(pneCount);
//	negativeNumericEffectsToActions = vector<list<pair<int, double> > >(pneCount);
//	numericPreconditionsToActions   = vector<list<pair<int, double> > >(pneCount);

	initialUnsatisfiedStartPreconditions = vector<int>(operatorCount);
	initialUnsatisfiedInvariants = vector<int>(operatorCount);
	initialUnsatisfiedEndPreconditions = vector<int>(operatorCount);

	initialUnsatisfiedProcessedStartPreconditions = vector<int>(operatorCount);

	achievedInLayer = vector<double>(literalCount);
	achievedInLayerReset = vector<double>(literalCount, -1.0);	
    achievedBy = vector<pair<int, VAL::time_spec> >(literalCount);
    achievedByReset = vector<pair<int, VAL::time_spec> >(literalCount, pair<int, VAL::time_spec>(-1, VAL::E_AT_START));
    
    negativeAchievedInLayer = vector<double>(literalCount);
    negativeAchievedInLayerReset = vector<double>(literalCount, 0.0);    
    negativeAchievedBy = vector<pair<int, VAL::time_spec> >(literalCount);
    negativeAchievedByReset = vector<pair<int, VAL::time_spec> >(literalCount, pair<int, VAL::time_spec>(-1, VAL::E_AT_START));
    
//	increasedInLayer = vector<int>(pneCount);
//	increasedBy = vector<pair<int, double> >(pneCount);
//	increasedReset = vector<pair<int, double> >(pneCount);

	literals = vector<Literal*>(literalCount);
	instantiatedOps = vector<instantiatedOp*>(operatorCount);
	rogueActions = vector<bool>(operatorCount);
	pnes = vector<PNE*>(pneCount);

	{
		InitialStateCollector c(0,0,theTC);
		current_analysis->the_problem->visit(&c);

		const int tilCount = c.timedInitialLiterals.size();

		timedInitialLiteralsVector = vector<FakeTILAction*>(tilCount);

		map<double, FakeTILAction>::iterator tilItr = c.timedInitialLiterals.begin();
		const map<double, FakeTILAction>::iterator tilEnd = c.timedInitialLiterals.end();

		for (int i = 0; tilItr != tilEnd; ++tilItr, ++i) {
			timedInitialLiterals.push_back(tilItr->second);
			timedInitialLiteralsVector[i] = &(timedInitialLiterals.back());
		}
		
	}
	
	actionsToDurations = vector<double>(operatorCount);


//	for (int i = 0; i < pneCount; ++i) increasedReset[i] = pair<int, double>(-1, 0.0);

	{
		LiteralStore::iterator lsItr = instantiatedOp::literalsBegin();
		const LiteralStore::iterator lsEnd = instantiatedOp::literalsEnd();

		for (int i; lsItr != lsEnd; ++lsItr) {
            i = (*lsItr)->getStateID();
            if (i != -1) {
                literals[i] = *lsItr;
                //cout << "Literal " << i << " - " << *(*lsItr) << " with global ID " << (*lsItr)->getGlobalID() << "\n";
            }
		}
	}
	const bool PNEdebug = (GlobalSchedule::globalVerbosity & 16);
	{
		if (PNEdebug) cout << "PNEs in RPG instantiation:\n";
		PNEStore::iterator pneItr = instantiatedOp::pnesBegin();
		const PNEStore::iterator pneEnd = instantiatedOp::pnesEnd();
        int sID;
		for (; pneItr != pneEnd; ++pneItr) {
            sID = (*pneItr)->getStateID();
            if (sID != -1) {
                pnes[sID] = *pneItr;
            }
            if (PNEdebug) cout << *(*pneItr) << " with state ID " << sID << " and global ID " << (*pneItr)->getGlobalID() << "\n";
                                                
		}
		if (PNEdebug) cout << "PNEs in operators:\n";
	}
	

	OpStore::iterator opsItr = instantiatedOp::opsBegin();
	const OpStore::iterator opsEnd = instantiatedOp::opsEnd();

	durationExpressions = vector<NumericPrecondition*>(operatorCount);

    list<pair<int, list<Preference> > > builtPreconditionPreferences;
    preconditionPrefCount = 0;
    
	for (; opsItr != opsEnd; ++opsItr) {
		instantiatedOp * const currOp = *opsItr;
		
		const int operatorID = currOp->getID();
		instantiatedOps[operatorID] = currOp;

		rogueActions[operatorID] = false;

		TimedPrecEffCollector c(0,prefNameToID,currOp->getEnv(),theTC);
        c.toBlame = currOp;
		currOp->forOp()->visit(&c);
        c.toBlame = 0;
        
		const bool rogueDebug = false;

		if (!c.isValid()) { // checking for rogues - fictituous literals
            rogueActions[operatorID] = true;
        } else {
			{

				list<Literal*>::iterator llItr = c.startPrec.begin();
				const list<Literal*>::iterator llEnd = c.startPrec.end();

				for (; llItr != llEnd; ++llItr) {
                    if (!(*llItr)) {
						rogueActions[operatorID] = true;
						if (rogueDebug) cout << "Start Precondition " << *llItr << " doesn't exist\n";
					}
				}

			}

			{

				list<Literal*>::iterator llItr = c.inv.begin();
				const list<Literal*>::iterator llEnd = c.inv.end();

				for (; llItr != llEnd; ++llItr) {
                    if (!(*llItr)) {
						rogueActions[operatorID] = true;
						if (rogueDebug) cout << "Invariant " << *llItr << " doesn't exist\n";
					}
				}

			}

			{

				list<Literal*>::iterator llItr = c.endPrec.begin();
				const list<Literal*>::iterator llEnd = c.endPrec.end();

				for (; llItr != llEnd; ++llItr) {
                    if (!(*llItr)) {
						rogueActions[operatorID] = true;
						if (rogueDebug) cout << "End Precondition " << *llItr << " doesn't exist\n";
					}
				}

			}

			{

				list<Literal*>::iterator llItr = c.startAddEff.begin();
				const list<Literal*>::iterator llEnd = c.startAddEff.end();

				for (; llItr != llEnd; ++llItr) {
                    if (!(*llItr)) {
						rogueActions[operatorID] = true;
						if (rogueDebug) cout << "Start Add Effect " << *llItr << " doesn't exist\n";
					}
					
				}
			}

			{

				list<Literal*>::iterator llItr = c.startDelEff.begin();
				const list<Literal*>::iterator llEnd = c.startDelEff.end();

				for (; llItr != llEnd; ++llItr) {
                    if (!(*llItr)) {
						rogueActions[operatorID] = true;
						if (rogueDebug) cout << "Start Delete Effect " << *llItr << " doesn't exist\n";
					}
					
				}
			}
			

			{

				list<Literal*>::iterator llItr = c.endAddEff.begin();
				const list<Literal*>::iterator llEnd = c.endAddEff.end();

				for (; llItr != llEnd; ++llItr) {
                    if (!(*llItr)) {
						rogueActions[operatorID] = true;
						if (rogueDebug) cout << "End Add Effect " << *llItr << " doesn't exist\n";
					}
					
				}
			}

			{

				list<Literal*>::iterator llItr = c.endDelEff.begin();
				const list<Literal*>::iterator llEnd = c.endDelEff.end();

				for (; llItr != llEnd; ++llItr) {
                    if (!(*llItr)) {
						rogueActions[operatorID] = true;
						if (rogueDebug) cout << "End Del Effect " << *llItr << " doesn't exist\n";
					}
					
				}
			}

		}

		if (RPGdebug) cout << "Operator " << operatorID << " - " << *currOp << "\n";

		if (rogueActions[operatorID]) {
			if (RPGdebug) cout << "Rogue action, skipping";
			delete c.durationExpression;
			c.durationExpression = 0;
			if (rogueDebug) cout << "Operator " << operatorID << " is a rogue\n";
			{
                list<ProtoConditionalEffect*>::iterator dItr = c.condEffs.begin();
                const list<ProtoConditionalEffect*>::iterator dEnd = c.condEffs.end();
                
                for (; dItr != dEnd; ++dItr) {
                    delete *dItr;
                }
                c.condEffs.clear();
            }
			
		} else {
            
            actionsToRawConditionalEffects[operatorID] = c.condEffs;
            
            if (!c.builtPreferences[0].empty()) {
                builtPreconditionPreferences.push_back(make_pair(operatorID, c.builtPreferences[0]));
                preconditionPrefCount += c.builtPreferences[0].size();
            }
            
            initialUnsatisfiedStartPreconditions[operatorID] = c.startPrec.size();
            initialUnsatisfiedInvariants[operatorID] = c.inv.size();
            initialUnsatisfiedEndPreconditions[operatorID] = c.endPrec.size();
			{
	
				{
					list<Literal*> & currEffectsList = actionsToStartEffects[operatorID];
	
					list<Literal*>::iterator effItr = c.startAddEff.begin();
					const list<Literal*>::iterator effEnd = c.startAddEff.end();
				
					if (RPGdebug) cout << "Operator " << operatorID << " start adds:";
				
					for (; effItr != effEnd; ++effItr) {
						const int effID = (*effItr)->getStateID();
						if (RPGdebug) cout << " " << *(*effItr) << " (" << effID << ")";
						currEffectsList.push_back(*effItr);
						effectsToActions[effID].push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));
					}
					if (RPGdebug) cout << "\n";
	
				}
	
				{
					list<Literal*> & currEffectsList = actionsToStartNegativeEffects[operatorID];
	
					list<Literal*>::iterator effItr = c.startDelEff.begin();
					const list<Literal*>::iterator effEnd = c.startDelEff.end();
				
					if (RPGdebug) cout << "Operator " << operatorID << " start deletes:";
				
					for (; effItr != effEnd; ++effItr) {
						const int effID = (*effItr)->getStateID();
						if (RPGdebug) cout << " " << *(*effItr) << " (" << effID << ")";
						currEffectsList.push_back(*effItr);
						negativeEffectsToActions[effID].push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));
					}
					if (RPGdebug) cout << "\n";
	
				}
	
				{
					list<Literal*> & currEffectsList = actionsToEndEffects[operatorID];
	
					list<Literal*>::iterator effItr = c.endAddEff.begin();
					const list<Literal*>::iterator effEnd = c.endAddEff.end();
				
					if (RPGdebug) cout << "Operator " << operatorID << " end adds:";
				
					for (; effItr != effEnd; ++effItr) {
						const int effID = (*effItr)->getStateID();
						if (RPGdebug) cout << " " << *(*effItr) << " (" << effID << ")";
						currEffectsList.push_back(*effItr);
						effectsToActions[effID].push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_END));
					}
					if (RPGdebug) cout << "\n";
	
				}
	
				{
					list<Literal*> & currEffectsList = actionsToEndNegativeEffects[operatorID];
	
					list<Literal*>::iterator effItr = c.endDelEff.begin();
					const list<Literal*>::iterator effEnd = c.endDelEff.end();
				
					if (RPGdebug) cout << "Operator " << operatorID << " end deletes:";
				
					for (; effItr != effEnd; ++effItr) {
						const int effID = (*effItr)->getStateID();
						if (RPGdebug) cout << " " << *(*effItr) << " (" << effID << ")";
						currEffectsList.push_back(*effItr);
						negativeEffectsToActions[effID].push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_END));
					}
					if (RPGdebug) cout << "\n";
	
				}	


				{
					
					 

					actionsToStartNumericEffects[operatorID] = c.startNumericEff;

					if (RPGdebug) {
						cout << "Operator " << operatorID << " start numeric effects:\n";
						list<NumericEffect>::iterator effItr = c.startNumericEff.begin();
						const list<NumericEffect>::iterator effEnd = c.startNumericEff.end();
						for (; effItr != effEnd; ++effItr) {
							cout << "\t";
							effItr->display(cout);
							cout << "\n";
						}
					}
					actionsToEndNumericEffects[operatorID] = c.endNumericEff;
					if (RPGdebug) {
						cout << "Operator " << operatorID << " end numeric effects:\n";
						list<NumericEffect>::iterator effItr = c.endNumericEff.begin();
						const list<NumericEffect>::iterator effEnd = c.endNumericEff.end();
						for (; effItr != effEnd; ++effItr) {
							cout << "\t";
							effItr->display(cout);
							cout << "\n";
						}
					}
	
				}
	
	
			
			}
	
			{
				
	
				if (RPGdebug) cout << "Operator requires at start:";
	
				
				{	

					list<Literal*> & currPreconditionsList = actionsToStartPreconditions[operatorID];

					list<Literal*>::iterator precItr = c.startPrec.begin();
					const list<Literal*>::iterator precEnd = c.startPrec.end();
		
					for (; precItr != precEnd; ++precItr) {
						const int precID = (*precItr)->getStateID();
                        if (precID >= 0) {
                            if (RPGdebug) cout << " " << *(*precItr) << " (" << precID << ")";
                            currPreconditionsList.push_back(*precItr);
                            preconditionsToActions[precID].push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));
                        }
					}
		
					if (RPGdebug) cout << "\n";
		
					initialUnsatisfiedStartPreconditions[operatorID] = currPreconditionsList.size();
					
				}
                /*
				if (RPGdebug) cout << "Operator requires as an invariant:";

				{	
					list<Literal*> & currPreconditionsList = actionsToInvariants[operatorID];

					list<Literal*>::iterator precItr = c.inv.begin();
					const list<Literal*>::iterator precEnd = c.inv.end();
		
					for (; precItr != precEnd; ++precItr) {
						const int precID = (*precItr)->getID();
						if (RPGdebug) cout << " " << *(*precItr) << " (" << precID << ")";
						currPreconditionsList.push_back(*precItr);
						preconditionsToActions[precID].push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_OVER_ALL));
						
					}
		
					if (RPGdebug) cout << "\n";
		
					initialUnsatisfiedInvariants[operatorID] = currPreconditionsList.size();
				}

				if (RPGdebug) cout << "Operator requires at end:";

				{	
					list<Literal*> & currPreconditionsList = actionsToEndPreconditions[operatorID];

					list<Literal*>::iterator precItr = c.endPrec.begin();
					const list<Literal*>::iterator precEnd = c.endPrec.end();
		
					for (; precItr != precEnd; ++precItr) {
						const int precID = (*precItr)->getID();
						if (RPGdebug) cout << " " << *(*precItr) << " (" << precID << ")";
						currPreconditionsList.push_back(*precItr);
						preconditionsToActions[precID].push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_END));
						
					}
		
					if (RPGdebug) cout << "\n";
		
					initialUnsatisfiedEndPreconditions[operatorID] = currPreconditionsList.size();
					if (currPreconditionsList.empty()) {
						if (RPGdebug) cout << "Operator is preconditionless at the end\n";
						preconditionlessActions.push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_END));
					}
				}*/
	
				{
					actionsToStartNumericPreconditions[operatorID].insert(actionsToStartNumericPreconditions[operatorID].end(), c.startPrecNumeric.begin(), c.startPrecNumeric.end());
					//actionsToNumericInvariants[operatorID].insert(actionsToNumericInvariants[operatorID].end(), c.invNumeric.begin(), c.invNumeric.end());
					//actionsToEndNumericPreconditions[operatorID].insert(actionsToEndNumericPreconditions[operatorID].end(), c.endPrecNumeric.begin(), c.endPrecNumeric.end());
				}
			}
	
			/*actionsToStartPreconditions[operatorID] = c.startPrec;
			actionsToInvariants[operatorID] = c.inv;
			actionsToEndPreconditions[operatorID] = c.endPrec;
		
			actionsToStartEffects[operatorID] = c.startAddEff;
			actionsToStartNegativeEffects[operatorID] = c.startDelEff;
			actionsToEndEffects[operatorID] = c.endAddEff;
			actionsToEndNegativeEffects[operatorID] = c.endDelEff;*/

			if (RPGdebug){
				cout << "Start negative effects:\n";
				{
					list<Literal*> & pList = actionsToStartNegativeEffects[operatorID];
					list<Literal*>::iterator llItr = pList.begin();
					const list<Literal*>::iterator llEnd = pList.end();

					for (; llItr != llEnd; ++llItr) {
						cout << "\t" << *(*llItr) << " (" << (*llItr)->getStateID() << ")\n";
					}
					
				}
			}

/* now do the tRPG processing:
 - collapse invariants into start actions
   - filter out preconditions satisfied by start effects
 - recount initial unsatisfied
 - rebuild literal -> precondition map, for new starts and old ends
*/

			{ 
	
	
				list<Literal*> & newStartPrecs = actionsToProcessedStartPreconditions[operatorID] = actionsToStartPreconditions[operatorID];
				
				LiteralSet oldStartEffects;
				{
					list<Literal*>::iterator effItr = c.startAddEff.begin();
					const list<Literal*>::iterator effEnd = c.startAddEff.end();
				
					for (; effItr != effEnd; ++effItr) {
						oldStartEffects.insert(*effItr);
					}
				}
	
				{
	
					const LiteralSet::iterator notFound = oldStartEffects.end();
	
					list<Literal*>::iterator precItr = c.inv.begin();
					const list<Literal*>::iterator precEnd = c.inv.end();
	
					for (; precItr != precEnd; ++precItr) {
						if (oldStartEffects.find(*precItr) == notFound) {
							newStartPrecs.push_back(*precItr);
						}
					}
	
				}
	
				if (RPGdebug){
					cout << "Processed start preconditions:\n";
					{
						
						list<Literal*>::iterator llItr = newStartPrecs.begin();
						const list<Literal*>::iterator llEnd = newStartPrecs.end();
	
						for (; llItr != llEnd; ++llItr) {
							cout << "\t" << *(*llItr) << " (" << (*llItr)->getStateID() << ")\n";
						}
						
					}
				}

				initialUnsatisfiedProcessedStartPreconditions[operatorID] = newStartPrecs.size();
	
				if (newStartPrecs.empty()) {
					preconditionlessActions.push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));
				}

				{	
	
					list<Literal*> & currPreconditionsList = newStartPrecs;
	
					list<Literal*>::iterator precItr = currPreconditionsList.begin();
					const list<Literal*>::iterator precEnd = currPreconditionsList.end();
		
					for (; precItr != precEnd; ++precItr) {
						const int precID = (*precItr)->getStateID();
						processedPreconditionsToActions[precID].push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));		
					}
		
				}
	
				{	
	
					list<Literal*> & currPreconditionsList = actionsToEndPreconditions[operatorID];
	
					list<Literal*>::iterator precItr = currPreconditionsList.begin();
					const list<Literal*>::iterator precEnd = currPreconditionsList.end();
		
					for (; precItr != precEnd; ++precItr) {
						const int precID = (*precItr)->getStateID();
						processedPreconditionsToActions[precID].push_back(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_END));		
					}
		
				}
	
			}

		}

		durationExpressions[operatorID] = c.durationExpression;



		
	}

    LiteralAnalysis::findStaticLiterals(); // this must be done now - is used to speed up grounding preferences

    taskPrefCount = 0;
    
	{
        FastEnvironment env(0);
        
		GoalNumericCollector c(&numericGoals,&literalGoals,prefNameToID,prefNameToNumberOfTimesDefinitelyViolated,0,&env,theTC);
		current_analysis->the_problem->visit(&c);

        taskPrefCount = c.builtPreferences.size();
		preferences.resize(taskPrefCount + preconditionPrefCount);
        
        int pid = 0;
        
        {
            list<Preference>::const_iterator pItr = c.builtPreferences.begin();
            const list<Preference>::const_iterator pEnd = c.builtPreferences.end();
            
            for (; pItr != pEnd; ++pItr, ++pid) {
                preferences[pid] = *pItr;            
                prefNameToID[pItr->name].push_back(pid);
            }
        }
        
        list<pair<int, list<Preference> > >::const_iterator actpItr = builtPreconditionPreferences.begin();
        const list<pair<int, list<Preference> > >::const_iterator actpEnd = builtPreconditionPreferences.end();
        
        for (; actpItr != actpEnd; ++actpItr) {
            vector<int> & dest = actionsToStartPreferences[actpItr->first];
            dest.reserve(actpItr->second.size());
            list<Preference>::const_iterator pItr = actpItr->second.begin();
            const list<Preference>::const_iterator pEnd = actpItr->second.end();
            
            for (; pItr != pEnd; ++pItr, ++pid) {
                preferences[pid] = *pItr;            
                prefNameToID[pItr->name].push_back(pid);
                dest.push_back(pid);
            }
            
        }
	}

    PreferenceHandler::initialiseNNF();
    
    LiteralAnalysis::pruneStaticPreconditions();
        
    foldInConditionalEffects();


	buildRPGNumericPreconditions();
	buildRPGNumericEffects();
	handleNumericInvariants();

	{
		list<pair<int, VAL::time_spec> >::iterator plaItr = preconditionlessActions.begin();
		const list<pair<int, VAL::time_spec> >::iterator plaEnd = preconditionlessActions.end();

		while (plaItr != plaEnd) {
			const bool destroy = (plaItr->second == VAL::E_AT_START && initialUnsatisfiedProcessedStartNumericPreconditions[plaItr->first])
						|| (plaItr->second == VAL::E_AT_END && initialUnsatisfiedNumericEndPreconditions[plaItr->first]);

			if (destroy) {
				const list<pair<int, VAL::time_spec> >::iterator delItr = plaItr;
				++plaItr;
				preconditionlessActions.erase(delItr);
			} else {
				++plaItr;
			}

		}
		
	}

	metric_spec * const ms = current_analysis->the_problem->metric;

	if (ms) {
		theMetric = new Metric(ms->opt == E_MINIMIZE);
		list<Operand> tmpFormula;
		ExpressionBuilder builder(tmpFormula, 0, 0);
		const bool metricIsValid = builder.buildFormula(const_cast<VAL::expression*>(ms->expr));
        if (!metricIsValid) {
            postmortem_invalidMetric();
        }
		pair<list<double>, list<int> > result;
		WhereAreWeNow = PARSE_METRIC;
		RPGBuilder::makeWeightedSum(tmpFormula, result);
		WhereAreWeNow = PARSE_UNKNOWN;
		theMetric->weights = result.first;
		theMetric->variables = result.second;

		list<int>::iterator varItr = theMetric->variables.begin();
		const list<int>::iterator varEnd = theMetric->variables.end();

		list<double>::iterator wItr = theMetric->weights.begin();

		while (varItr != varEnd) {
			if (*varItr == -1) {
				const list<int>::iterator varPrev = varItr; ++varItr;
				const list<double>::iterator wPrev = wItr; ++wItr;
				
				theMetric->variables.erase(varPrev);
				theMetric->weights.erase(wPrev);
			} else if (*varItr >= 0) {
                metricVars.insert(*varItr);
                ++varItr;
                ++wItr;
            } else if (*varItr > -1024) {
                if (*varItr <= -512) {
                    
                    if (*varItr <= -528) {
                        *varItr += 16;
                        if (*wItr != 0.0) *wItr = -*wItr;
                    }
                    
                    if (*varItr == -512) {
                        // is the variable denoting preferences that are tautologous - i.e. evaluates to 0,
                        // so we do nothing
                    } else if (*varItr == -513) {                        
                        permanentPreferenceViolations = *wItr;                        
                    } else {
                        cout << "Internal error - variable " << *varItr << " isn't one of -512 or -513\n";
                        exit(1);
                    }
                    
                    const list<int>::iterator varPrev = varItr; ++varItr;
                    const list<double>::iterator wPrev = wItr; ++wItr;
                    
                    theMetric->variables.erase(varPrev);
                    theMetric->weights.erase(wPrev);
                    
                } else if (*varItr <= -16) {
                    *varItr += 16;
                    metricVars.insert(*varItr + 16);                    
                    if (*wItr != 0.0) *wItr = -*wItr;
                } else {
                    metricVars.insert(*varItr);
                }
				++varItr; ++wItr;
			} else {
                const int vtu = (*varItr <= -1048576 ? -1048576 - *varItr : -1024 - *varItr);
                const double wtu = (*wItr == 0.0 ? 0.0 : (*varItr <= -1048576 ? -*wItr : *wItr));
                
				preferences[vtu].cost = wtu;

				const list<int>::iterator varPrev = varItr; ++varItr;
				const list<double>::iterator wPrev = wItr; ++wItr;
				
				theMetric->variables.erase(varPrev);
				theMetric->weights.erase(wPrev);

			}
		}

		//metricVars.insert(theMetric->variables.begin(),theMetric->variables.end());
		
	};


	buildDurations(durationExpressions);
	findSelfMutexes();
	doSomeUsefulMetricRPGInference();
	findDominanceConstraints();
	findUninterestingnessCriteria();
	NumericAnalysis::identifyIntegralPNEs();
    NumericAnalysis::performBoundsAnalysis();
    
    NumericAnalysis::identifySimplePropositionNumberPumps(); 
    NumericAnalysis::identifySimpleReversibleNumberPumps();
    
    PreferenceHandler::flattenNNF();
    
    if (postFilter) {
        postFilterIrrelevantActions();
    }
    
    
    PreferenceHandler::buildAutomata();
    LandmarksAnalysis::performLandmarksAnalysis();
    
	if (!planAnyway) {	
		const int loopLim = rpgNumericEffects.size();
		set<int> warnAbout;
		for (int i = 0; i < loopLim; ++i) {
			if (rpgNumericEffects[i].size) {
				warnAbout.insert(rpgNumericEffects[i].fluentIndex);
			}
		}
		if (!warnAbout.empty()) {
			cout << "********************************************************************************\n";
			if (warnAbout.size() == 1) {
				cout << "Warning: The variable " << *(RPGBuilder::getPNE(*(warnAbout.begin()))) << " has a non-constant effect applied to it, thereby violating the producer--consumer behaviour needed for the heuristic.\n";
			} else {
				cout << "Warning: The following variables have non-constant effects applied to them,\nthereby violating the producer--consumer behaviour needed for the heuristic:\n";
				set<int>::iterator warnItr = warnAbout.begin();
				const set<int>::iterator warnEnd = warnAbout.end();
				for (; warnItr != warnEnd; ++warnItr) {
					cout << "\t" << *(RPGBuilder::getPNE(*warnItr)) << "\n";
				}
			}
			cout << "As such, the behaviour of LPRPG cannot be guaranteed to be sensible.\n";
			cout << "Run with -citation for details of the paper describing the LPRPG heuristic, or\n";
			cout << "with -plananyway to force LPRPG to attempt to find a solution.\n";
			cout << "********************************************************************************\n";
			exit(0);
		}
	}

};

void RPGBuilder::foldInConditionalEffects()
{
    
    const int actCount = actionsToRawConditionalEffects.size();
    
    for (int act = 0; act < actCount; ++act) {
        
        //bool printed = false;
        
        list<RPGBuilder::ProtoConditionalEffect*>::iterator ceItr = actionsToRawConditionalEffects[act].begin();
        const list<RPGBuilder::ProtoConditionalEffect*>::iterator ceEnd = actionsToRawConditionalEffects[act].end();
        for (; ceItr != ceEnd; ++ceItr) {                        
            
            if (rogueActions[act]) {
                delete *ceItr;
                continue;
            }
            
            bool isFalse = false;
            bool hasNonStatic = false;
            {
                list<Literal*>::iterator factItr = (*ceItr)->startPrec.begin();
                const list<Literal*>::iterator factEnd = (*ceItr)->startPrec.end();
                
                for (; factItr != factEnd; ++factItr) {
                    if (*factItr) {
                        hasNonStatic = true;
                    } else {
                        isFalse = true;
                    }
                }
            }
            if (isFalse) {
                delete *ceItr;
                continue;
            }
            
            if (hasNonStatic || !(*ceItr)->startPrecNumeric.empty() ) {
                postmortem_noADL();
            }
            
            actionsToStartEffects[act].insert(actionsToStartEffects[act].end(), (*ceItr)->startAddEff.begin(), (*ceItr)->startAddEff.end());
            actionsToStartNegativeEffects[act].insert(actionsToStartNegativeEffects[act].end(), (*ceItr)->startDelEff.begin(), (*ceItr)->startDelEff.end());
            actionsToStartNumericEffects[act].insert(actionsToStartNumericEffects[act].end(), (*ceItr)->startNumericEff.begin(), (*ceItr)->startNumericEff.end());

            /*
            if (!(*ceItr)->startNumericEff.empty()) {
                if (!printed) {
                    cout << "Folded numeric effects into " << *(RPGBuilder::getInstantiatedOp(act)) << "\n";
                    printed = true;
                }
            }*/
            
            for (int pass = 0; pass < 2; ++pass) {
                list<Literal*> & currList = (pass ? (*ceItr)->startAddEff : (*ceItr)->startDelEff);
                vector<list<pair<int, VAL::time_spec> > > & destVec = (pass ? effectsToActions : negativeEffectsToActions);
                {
                    list<Literal*>::const_iterator factItr = currList.begin();
                    const list<Literal*>::const_iterator factEnd = currList.end();
                    for (; factItr != factEnd; ++factItr) {
                        destVec[(*factItr)->getStateID()].push_back(make_pair(act, VAL::E_AT_START));
                    }
                }
            }
            delete *ceItr;
        }
        
        /*if (printed) {
            cout << " - Now has effects:\n";
            list<RPGBuilder::NumericEffect>::iterator pItr = actionsToStartNumericEffects[act].begin();
            const list<RPGBuilder::NumericEffect>::iterator pEnd = actionsToStartNumericEffects[act].end();
            
            for (; pItr != pEnd; ++pItr) {
                cout << "\t" << *pItr << endl;
            }
            
        }*/
        
    }
    
}


pair<bool,int> RPGBuilder::isBiggerBetter(const int & var, const int & polarity) {
   
	const int pneCount = RPGBuilder::getPNECount();

	list<int>::iterator preItr = variablesToRPGNumericPreconditions[var].begin();
	const list<int>::iterator preEnd = variablesToRPGNumericPreconditions[var].end();

	for (; preItr != preEnd; ++preItr) {
		RPGNumericPrecondition & currPre = rpgNumericPreconditions[*preItr];
		double varWeight = 0.0;
		double theConstant = 0.0;
		bool simpleConstant = true;
		if (currPre.LHSVariable < pneCount) {
			varWeight = currPre.LHSConstant;
			theConstant = currPre.RHSConstant;
		} else if (currPre.LHSVariable < (2 * pneCount)) {
			varWeight = -currPre.LHSConstant;
			theConstant = -currPre.RHSConstant;
		} else {
			ArtificialVariable & currAV = getArtificialVariable(currPre.LHSVariable);

			const int size = currAV.size;

			for (int i = 0; i < size; ++i) {
				const int lv = currAV.fluents[i];
				if (lv < pneCount) {
					if (lv == var) {
						varWeight = currAV.weights[i];
						break;
					}
				} else {
					if (lv == (var + pneCount)) {
						varWeight = -currAV.weights[i];
						break;
					}
				}
			}
			theConstant = -currAV.constant;
			if (size > 1) simpleConstant = false;
		}
		if (varWeight * polarity >= 0.0) {
			// is a v >= c constraint - bigger is simply better
		} else {
			// is a v <= c constraint, see if it's only to get more, and there's no gap
			set<int> actsToCheck;

			{
				list<pair<int,int> >::iterator ngItr = numericRPGGoals.begin();
				const list<pair<int,int> >::iterator ngEnd = numericRPGGoals.end();

				for (; ngItr != ngEnd; ++ngItr) {
					if (ngItr->first == currPre.ID) return pair<bool,int>(false,-1);
					if (ngItr->second == currPre.ID) return pair<bool,int>(false,-1);
				}
				
			}
			{
				list<pair<int, VAL::time_spec> > & currList = rpgNumericPreconditionsToActions[currPre.ID];
				list<pair<int, VAL::time_spec> >::iterator clItr = currList.begin();
				const list<pair<int, VAL::time_spec> >::iterator clEnd = currList.end();

				for (; clItr != clEnd; ++clItr) {
					actsToCheck.insert(clItr->first);
				}
			}
			set<int>::iterator actItr = actsToCheck.begin();
			const set<int>::iterator actEnd = actsToCheck.end();

			for (; actItr != actEnd; ++actItr) {
				for (int pass = 0; pass < 2; ++pass) {
					list<Literal*> & effList = (pass ? actionsToEndEffects[*actItr]: actionsToStartEffects[*actItr]);	

					if (!effList.empty()) return pair<bool,int>(false,-1);
				}

				for (int pass = 0; pass < 2; ++pass) {
					list<int> & effList = (pass ? actionsToRPGNumericEndEffects[*actItr]: actionsToRPGNumericStartEffects[*actItr]);	

					list<int>::iterator effItr = effList.begin();
					const list<int>::iterator effEnd = effList.begin();

					for (; effItr != effEnd; ++effItr) {
						RPGNumericEffect & currEff = rpgNumericEffects[*effItr];
						if (currEff.fluentIndex == var) {
							if (!simpleConstant) return pair<bool,int>(false,-1);
							map<int,double> weightedSum;
							double localConstant = currEff.constant;

							{
							      const int looplim = currEff.size;
							      for (int s = 0; s < looplim; ++s) {
								      weightedSum.insert(make_pair(currEff.variables[s], currEff.weights[s]));
							      }

							}
							if (!currEff.isAssignment) weightedSum.insert(make_pair(var,1.0));
							

							if (!weightedSum.empty()) return pair<bool,int>(false,-1);

							if (theConstant != 0.0) theConstant = -theConstant;
							if ((polarity * theConstant < polarity * localConstant)
                                                            && fabs(localConstant - theConstant) > 0.000000001) {
								return pair<bool,int>(false,-1);
							}
						} else if (dominanceConstraints[currEff.fluentIndex] != E_METRIC && dominanceConstraints[currEff.fluentIndex] != E_IRRELEVANT) {
							if (dominanceConstraints[currEff.fluentIndex] == E_BIGGERISBETTER) {
								map<int,double> weightedSum;
								double localConstant = currEff.constant;

								{
								      const int looplim = currEff.size;
								      for (int s = 0; s < looplim; ++s) {
									      weightedSum.insert(make_pair(currEff.variables[s], currEff.weights[s]));
								      }

								}
								if (!currEff.isAssignment) weightedSum.insert(make_pair(var,1.0));
								
								
								if (!weightedSum.empty() || localConstant > 0) return pair<bool,int>(false,currEff.fluentIndex);
								
							} else if (dominanceConstraints[currEff.fluentIndex] == E_SMALLERISBETTER) {
								map<int,double> weightedSum;
								double localConstant = currEff.constant;

								{
								      const int looplim = currEff.size;
								      for (int s = 0; s < looplim; ++s) {
									      weightedSum.insert(make_pair(currEff.variables[s], currEff.weights[s]));
								      }

								}
								if (!currEff.isAssignment) weightedSum.insert(make_pair(var,1.0));
								
								
								if (!weightedSum.empty() || localConstant < 0) return pair<bool,int>(false,currEff.fluentIndex);
							} else {
								return pair<bool,int>(false, currEff.fluentIndex);
							}
						}
					}
				}

			}
		}
	}
	return pair<bool,int>(true,0);
};


void RPGBuilder::findDominanceConstraints() {
	const int pneCount =  RPGBuilder::getPNECount();
	dominanceConstraints = vector<dominance_constraint>(pneCount,E_NODOMINANCE);
	
	set<int> toVisit;
	map<int,set<int> > revisit;
	for (int i = 0; i < pneCount; ++i) {
		toVisit.insert(i);
	}
	
	while (!toVisit.empty()) {
	
	set<int>::iterator tvItr = toVisit.begin();
	const set<int>::iterator tvEnd = toVisit.end();

		for (; tvItr != tvEnd; ++tvItr) {
			const int i = *tvItr;
			const int negativeI = i + pneCount;
			//PNE* const currPNE = getPNE(i);
			{ // case one - never appears as a precondition, presume it's a metric tracking quantity
				bool neverInPrecondition = true;
				const int rnpCount = rpgNumericPreconditions.size();
				for (int rnp = 0; rnp < rnpCount; ++rnp) {
					RPGNumericPrecondition & currRNP = rpgNumericPreconditions[rnp];
					if (	    currRNP.LHSVariable == i || currRNP.LHSVariable == negativeI
						||  currRNP.RHSVariable == i || currRNP.RHSVariable == negativeI) {
						neverInPrecondition = false;
						break;
					}
				}
				if (neverInPrecondition) {
					const int avCount = rpgArtificialVariables.size();
					for (int av = 0; av < avCount; ++av) {
						ArtificialVariable & currAV = rpgArtificialVariables[av];
						const int avfSize = currAV.size;
						for (int f = 0; f < avfSize; ++f) {
							const int currF = currAV.fluents[f];
							if (currF == i || currF == negativeI) {
								neverInPrecondition = false;
								break;
							}
						}
					}
				}
				if (neverInPrecondition) {
					
					if (metricVars.find(i) == metricVars.end()) {
						dominanceConstraints[i] = E_IRRELEVANT;
						//cout << "Have a search-irrelevant fluent: " << *currPNE << "\n";
					} else {
						dominanceConstraints[i] = E_METRIC;
						//cout << "Have a metric-tracking fluent: " << *currPNE << "\n";
					}
				}
			}
			if (dominanceConstraints[i] == E_NODOMINANCE) {
				pair<bool,int> bbCheck = isBiggerBetter(i, 1);
				if (bbCheck.first) {
					dominanceConstraints[i] = E_BIGGERISBETTER;
					//cout << *currPNE << " - bigger is better\n";
				} else if (bbCheck.second != -1) {
					revisit[bbCheck.second].insert(i);
				}
			}
			if (dominanceConstraints[i] == E_NODOMINANCE) {
				pair<bool,int> sbCheck = isBiggerBetter(i, -1);
				if (sbCheck.first) {
					dominanceConstraints[i] = E_SMALLERISBETTER;
					//cout << *currPNE << " - smaller is better\n";
				} else if (sbCheck.second != -1) {
					revisit[sbCheck.second].insert(i);
				}
			}		
		}

		toVisit.clear();

		map<int,set<int> >::iterator rvItr = revisit.begin();
		const map<int,set<int> >::iterator rvEnd = revisit.end();

		while (rvItr != rvEnd) {
			if (dominanceConstraints[rvItr->first] == E_IRRELEVANT || dominanceConstraints[rvItr->first] == E_METRIC) {
				toVisit.insert(rvItr->second.begin(), rvItr->second.end());
				map<int,set<int> >::iterator rvPrev = rvItr;
				++rvItr;
				revisit.erase(rvPrev);
			} else {
				++rvItr;
			}
		}
	}
};

void RPGBuilder::findUninterestingnessCriteria() {
	const int opCount = instantiatedOps.size();

	for (int i = 0; i < opCount; ++i) {
		if (!rogueActions[i]) {			
			bool allUninteresting = true;
			int criterion = -1;
			{
				list<Literal*> & effs = actionsToStartEffects[i];
				list<Literal*>::iterator effItr = effs.begin();
				const list<Literal*>::iterator effEnd = effs.end();
				
				for (; effItr != effEnd; ++effItr) {
					if (negativeEffectsToActions[(*effItr)->getStateID()].empty()) {
						criterion = (*effItr)->getStateID();
					} else {
						allUninteresting = false;
						break;
					}
				}
			}
			if (allUninteresting) {
				list<Literal*> & effs = actionsToEndEffects[i];
				list<Literal*>::iterator effItr = effs.begin();
				const list<Literal*>::iterator effEnd = effs.end();
				
				for (; effItr != effEnd; ++effItr) {
					if (negativeEffectsToActions[(*effItr)->getStateID()].empty()) {
						criterion = (*effItr)->getStateID();
					} else {
						allUninteresting = false;
						break;
					}
				}
			}
			
			if (allUninteresting) { // checked props, now onto numerics
				//if (criterion == -1) {
				//	cout << "Action " << i << " - " << *(instantiatedOps[i]) << " is propositionally uninteresting once applied: no propositional effects (" << actionsToStartEffects[i].size() << ", " << actionsToEndEffects[i].size() << ")\n";
				//} else {
				//	cout << "Action " << i << " - " << *(instantiatedOps[i]) << " is propositionally uninteresting once applied: nothing deletes " << *(literals[criterion]) << "\n";
				//}
				{
					list<int> & numEffs = actionsToRPGNumericStartEffects[i];
					
					list<int>::iterator neItr = numEffs.begin();
					const list<int>::iterator neEnd = numEffs.end();
					
					for (; neItr != neEnd; ++neItr) {
						RPGNumericEffect & currRNE = rpgNumericEffects[*neItr];
						if (dominanceConstraints[currRNE.fluentIndex] == E_IRRELEVANT || dominanceConstraints[currRNE.fluentIndex] == E_METRIC) {
							
						} else {
							allUninteresting = false;
							break;
						}
					}
				}
				if (allUninteresting) {
					list<int> & numEffs = actionsToRPGNumericEndEffects[i];
					
					list<int>::iterator neItr = numEffs.begin();
					const list<int>::iterator neEnd = numEffs.end();
					
					for (; neItr != neEnd; ++neItr) {
						RPGNumericEffect & currRNE = rpgNumericEffects[*neItr];
						if (dominanceConstraints[currRNE.fluentIndex] == E_IRRELEVANT || dominanceConstraints[currRNE.fluentIndex] == E_METRIC) {
							
						} else {
							allUninteresting = false;
							break;
						}
					}
				}
				if (allUninteresting) {
					//cout << "Action " << i << " - " << *(instantiatedOps[i]) << " is uninteresting";
					if (criterion != -1) {
						//cout << " once we have fact " << *(literals[criterion]) << "\n";
						uninterestingnessCriteria[i] = criterion;
					} else {
						//cout << " full stop - there's no reason ever to apply it\n";
						uninterestingnessCriteria[i] = -1;
					}
				}
			}
		}
	}
};

void RPGBuilder::getAllInitialLiterals(LiteralSet & initialState) {
 
    InitialStateCollector c(0,0,theTC);
    current_analysis->the_problem->visit(&c);
    
    c.initialState.swap(initialState);
    
}

void RPGBuilder::getInitialState(LiteralSet & initialState, vector<double> & initialFluents) {
    
    static LiteralSet isCache;
    static vector<double> ifCache;
    static bool cached = false;
    
    if (!cached) {
        
        InitialStateCollector c(0,0,theTC);
        current_analysis->the_problem->visit(&c);
        
        isCache.swap(c.initialState);
        ifCache.swap(c.initialFluents);
        
        LiteralSet::iterator factItr = isCache.begin();
        const LiteralSet::iterator factEnd = isCache.end();
        
        while (factItr != factEnd) {
            if (LiteralAnalysis::isStatic(*factItr).first) {
                LiteralSet::iterator factPrev = factItr;
                ++factItr;
                isCache.erase(factPrev);
            } else {
                ++factItr;
            }
        }
        
        cached = true;
    }
    
    initialState = isCache;
    initialFluents = ifCache;
        
        
        
};


SubproblemRPG* RPGBuilder::pruneRPG(list<Literal*> & goals, list<int> & goalFluents, LiteralSet & initialState, vector<double> & initialFluents) {

	// for now, don't prune the RPG

	getInitialState(initialState, initialFluents);

	return new SubproblemRPG( false, // subproblem does not own the arrays
				&actionsToStartEffects,
				&actionsToEndEffects,
				&effectsToActions,
				&actionsToStartNegativeEffects,
				&actionsToEndNegativeEffects,
				&negativeEffectsToActions,
				&preconditionsToActions,
				&actionsToStartPreconditions,
				&actionsToInvariants,
				&actionsToEndPreconditions,
				&actionsToStartNumericEffects,
				&actionsToEndNumericEffects,
				&actionsToRPGNumericStartEffects,
				&actionsToRPGNumericEndEffects,
				&actionsToRPGNumericStartPreconditions,
				&actionsToRPGNumericInvariants,
				&actionsToRPGNumericEndPreconditions,
				&actionsToProcessedStartRPGNumericPreconditions,
				&initialUnsatisfiedStartPreconditions,
				&initialUnsatisfiedInvariants,
				&initialUnsatisfiedEndPreconditions,
				&achievedInLayer,
				&achievedInLayerReset,
				&achievedBy,
				&achievedByReset,
                &negativeAchievedInLayer,
                &negativeAchievedInLayerReset,
                &negativeAchievedBy,
                &negativeAchievedByReset,                              
				&numericAchievedInLayer,
				&numericAchievedInLayerReset,
				&numericAchievedBy,
				&numericAchievedByReset,
                &negativeNumericAchievedInLayer,
                &negativeNumericAchievedInLayerReset,
                &negativeNumericAchievedBy,
                &negativeNumericAchievedByReset,                              
				&initialUnsatisfiedNumericStartPreconditions,
				&initialUnsatisfiedNumericInvariants,
				&initialUnsatisfiedNumericEndPreconditions,
				&rpgNumericPreconditions,
				&rpgNumericEffects,
				&processedPreconditionsToActions,
				&processedRPGNumericPreconditionsToActions,
				&actionsToProcessedStartPreconditions,
				&initialUnsatisfiedProcessedStartPreconditions,
				&initialUnsatisfiedProcessedStartNumericPreconditions,
				&preconditionlessActions);

};




void rpprintState(MinimalState & e) {

	cout << "Literals:";
	{
		set<int>::iterator itr = e.first.begin();
		const set<int>::iterator itrEnd = e.first.end();
	
		for (; itr != itrEnd; ++itr) {
			cout << " " << *itr;
		}
	}


	cout << "\nState Finished";

	cout << "\n";
};



ActionFluentModification* buildAFM(list<pair<int, double> > & actionSeq, list<ActionFluentModification> & scrap) {
	if (actionSeq.empty()) return 0;

	list<pair<int, double> >::iterator asItr = actionSeq.begin();
	const list<pair<int, double> >::iterator asEnd = actionSeq.end();


	scrap.push_back(ActionFluentModification(asItr->first, VAL::E_AT_START, false, 0.0, asItr->second, false));

	ActionFluentModification * const toReturn = &(scrap.back());

	ActionFluentModification * prev = toReturn;

	++asItr;

	for (; asItr != asEnd; ++asItr) {
		scrap.push_back(ActionFluentModification(asItr->first, VAL::E_AT_START, false, 0.0, asItr->second, false));
		prev->next = &(scrap.back());
		prev = prev->next;
	}

	return toReturn;
};

vector<double> SubproblemRPG::prefCosts;
bool RPGBuilder::useMetricRPG = false;
unsigned int RPGBuilder::statesEvaluated = 0;

SubproblemRPG::EvaluationInfo SubproblemRPG::getRelaxedPlan(MinimalState & theState, const double & maxPrefCostIn, const double & stateTS, const int & nextTIL, set<int> & goals, set<int> & goalFluents/*, set<int> & goalFluents*/, list<pair<int, VAL::time_spec> > & helpfulActions, list<pair<double, list<ActionAndHowManyTimes > > > & relaxedPlan, map<double, list<int> > * justApplied, double tilFrom) {

	static const double EPSILON = 0.001;
	bool evaluateDebug = GlobalSchedule::globalVerbosity & 64;
    bool prefDebug = GlobalSchedule::globalVerbosity & 32768;
    
	if (evaluateDebug) cout << "Evaluating a state\n";
    
    ++RPGBuilder::statesEvaluated;
    
    if (prefDebug) cout << "Outputting preference debugging information\n";

    if (prefDebug) {
        if (maxPrefCostIn != DBL_MAX) {
            cout << COLOUR_red << "Ceiling on final plan cost: " << maxPrefCostIn << COLOUR_default << endl;
        }
    }

	//rpprintState(theState);

    reducedCostFacts.clear();
    
	vector<int> startPreconditionCounts(*initialUnsatisfiedProcessedStartPreconditions);

	vector<int> numericStartPreconditionCounts(*initialUnsatisfiedProcessedStartNumericPreconditions);

	
	if (evaluateDebug) {
		const int loopLim = numericStartPreconditionCounts.size();
		for (int i = 0; i < loopLim; ++i) {
			cout << i << " - ";
			cout << startPreconditionCounts[i] << ", " << numericStartPreconditionCounts[i] << ",";
		}
	}
	

	const int vCount = theState.second.size();
	const int avCount = RPGBuilder::getAVCount();

	map<double, vector<double>, EpsilonComp > fluentLayers;	
	map<double, map<int, list<ActionFluentModification> >, EpsilonComp> fluentModifications;
	
	{
		vector<double> maxFluentTable(vCount * 2 + avCount);
		fluentLayers.insert(pair<double, vector<double> >(0.0, maxFluentTable));
		
	}
	vector<double> * maxFluentTable = &(fluentLayers.begin()->second);

	{
		const vector<double> & oldVec = theState.second;
		for (int i = 0; i < vCount; ++i) {
			const double ov = oldVec[i];
			(*maxFluentTable)[i] = ov;
			if (ov != 0.0) {
				(*maxFluentTable)[i + vCount] = 0.0 - ov;
			} else {
				(*maxFluentTable)[i + vCount] = 0.0;
			}
		}
	}
	
    auto_ptr<MILPRPG> milprpg(0);
    auto_ptr<MetricRPG> metricrpg(0);
    
    if (vCount) {
        if (RPGBuilder::useMetricRPG) {
            metricrpg = auto_ptr<MetricRPG>(new MetricRPG(&theState, *maxFluentTable, (RPGBuilder::costPropagationMethod == E_NOCOSTS || !RPGBuilder::useTheCostsInTheLP ? 0 : &(actionCosts))));
        } else {
            milprpg = auto_ptr<MILPRPG>(new MILPRPG(&theState, theState.second, (RPGBuilder::costPropagationMethod == E_NOCOSTS || !RPGBuilder::useTheCostsInTheLP ? 0 : &(actionCosts))));   
        }
    }

	{
		const int startLim = vCount * 2;
		const int endLim = startLim + avCount;
		for (int i = startLim; i < endLim; ++i) {
			(*maxFluentTable)[i] = RPGBuilder::getArtificialVariable(i).evaluate(*maxFluentTable);
		}
	}

		
	

	map<int, int> insistUponEnds;

	double heuristicOffset = 0;

	

	int unappearedEnds = insistUponEnds.size();

	int unsatisfiedGoals = goals.size() + goalFluents.size();

	if (evaluateDebug) {
		cout << "Aiming for number of goals satisfied: " << unsatisfiedGoals << "\n";
	}

		
	const set<int>::iterator gsEnd = goals.end();
	const set<int>::iterator gfEnd = goalFluents.end();

	{
		set<int>::iterator gsItr = goals.begin();

		for (; gsItr != gsEnd; ++gsItr) {
			if (theState.first.find(*gsItr) != theState.first.end()) {
				if (evaluateDebug) {
					cout << "\t" << *gsItr << " true in initial state\n";
				}
				--unsatisfiedGoals;
			}
		}
	}

    bool numericGoalsSatisfied = true; 
        
	{
        
		set<int>::iterator gfItr = goalFluents.begin();

		for (; gfItr != gfEnd; ++gfItr) {
			if ((*rpgNumericPreconditions)[*gfItr].isSatisfied(*maxFluentTable)) {
				if (evaluateDebug) {
					cout << "\t" << (*rpgNumericPreconditions)[*gfItr] << " true in initial state\n";
				}

				--unsatisfiedGoals;
			} else {
                //cout << "Numeric goals aren't all satisfied in the initial state\n";
                numericGoalsSatisfied = false;
            }
		}

	}

	if (evaluateDebug) {
		cout << "Goals unsatisfied in initial state: " << unsatisfiedGoals << "\n";
	}


    const bool realGoalsSatisfied = (!unsatisfiedGoals && !unappearedEnds);

    if (realGoalsSatisfied) {
        cout << "(G)"; cout.flush();
    }
    
    int fakeGoalCount = 0;

    if (!numericGoalsSatisfied && milprpg.get()) {
        if (evaluateDebug) {
            cout << "-- Adding dummy goal to denote that all numeric goals are co-satisfied\n";
        }
        ++unsatisfiedGoals;
        ++fakeGoalCount;
    }

    static bool initPrefCosts = false;
    
    
    if (!initPrefCosts) {
        initPrefCosts = true;
        
        const vector<RPGBuilder::Preference> & prefTable = RPGBuilder::getPreferences();
        const int pSize = prefTable.size();
        prefCosts.resize(pSize);
        for (int p = 0 ; p < pSize; ++p) {
            prefCosts[p] = prefTable[p].cost;
        }
    }
    

    const double currentPreferenceCost = PreferenceHandler::getCurrentCost(theState);
    const double reachablePreferenceCost = PreferenceHandler::getReachableCost(theState);
    
    if (reachablePreferenceCost < currentPreferenceCost) {
        if (evaluateDebug) {
            cout << "-- Adding dummy goal to try to reach the theoretical minimum preference violation cost (" << reachablePreferenceCost << "\n";
        }
        ++unsatisfiedGoals;
        ++fakeGoalCount;
    }
    

    if (!unsatisfiedGoals && !unappearedEnds) {
        if (evaluateDebug) cout << "Current cost of state is " << currentPreferenceCost << ", reachable cost is " << reachablePreferenceCost << ", so returning\n";
        return EvaluationInfo(0.0, currentPreferenceCost, true);
    }

    const double GCost = PreferenceHandler::getG(theState);

    const double maxPrefCost = (maxPrefCostIn != DBL_MAX ? maxPrefCostIn - GCost - 0.001 : DBL_MAX);

    if (maxPrefCost < 0.0) {
        if (evaluateDebug) cout << "Preference score too poor to bother with evaluation\n";
                return EvaluationInfo();        
    }
    
    maxPermissibleActionCost = (RPGBuilder::doNotApplyActionsThatLookTooExpensive ? maxPrefCost : DBL_MAX);
        
        
    
    if (prefDebug) {
        cout << COLOUR_red << "After deducting " << GCost << " for irreversibly broken preferences, and 0.001 to make sure it's strictly better, now must be no more than " << maxPrefCost << COLOUR_default << endl;
    }
    
    /*const double numericG = PreferenceHandler::getG(theState, 2);
    const double currentNumericPreferenceCost = PreferenceHandler::getCurrentCost(theState, 2);
    const double reachableNumericPreferenceCost = PreferenceHandler::getReachableCost(theState, 2);*/

    /*const double literalG = PreferenceHandler::getG(theState, 1);
    const double currentLiteralPreferenceCost = PreferenceHandler::getCurrentCost(theState, 1);
    const double reachableLiteralPreferenceCost = PreferenceHandler::getReachableCost(theState, 1);*/
    
    
    /*double lpGoalPrefViolation = currentNumericPreferenceCost - numericG;    
    double literalGoalPrefViolation = currentLiteralPreferenceCost - literalG;
    double totalPrefViolation = currentPreferenceCost - GCost;*/
    
    double lpGoalPrefViolation = currentPreferenceCost - GCost;
    
    if (prefDebug) {
        cout << "If we stopped here, pref violation for relaxed plan would be " << lpGoalPrefViolation << endl;
    }
    
    double rpgGoalPrefViolation = currentPreferenceCost - GCost;

	*achievedBy = (*achievedByReset);
	*achievedInLayer = (*achievedInLayerReset);
    
    *negativeAchievedBy = (*negativeAchievedByReset);
    *negativeAchievedInLayer = (*negativeAchievedInLayerReset);
    
	*numericAchievedBy = (*numericAchievedByReset);
	*numericAchievedInLayer = (*numericAchievedInLayerReset);

    *negativeNumericAchievedBy = (*negativeNumericAchievedByReset);
    *negativeNumericAchievedInLayer = (*negativeNumericAchievedInLayerReset);
    
    preconditionsToPrefs = PreferenceHandler::getPreconditionsToPrefs();
    negativePreconditionsToPrefs = PreferenceHandler::getNegativePreconditionsToPrefs();
    
    numericPreconditionsToPrefs = PreferenceHandler::getNumericPreconditionsToPrefs();
    negativeNumericPreconditionsToPrefs = PreferenceHandler::getNegativeNumericPreconditionsToPrefs();
    
    vector<vector<NNF_Flat*> > initialUnsatisfiedPreferenceConditions;
    
    PreferenceHandler::getUnsatisfiedConditionCounts(theState, initialUnsatisfiedPreferenceConditions);
    
    vector<vector<double> > preferencePartTrueAtLayer(initialUnsatisfiedPreferenceConditions.size(), vector<double>(2,DBL_MAX));
    
    unsatisfiedPreferenceConditions = &initialUnsatisfiedPreferenceConditions;
    preferencePartBecameTrueInLayer = &preferencePartTrueAtLayer;
    
    vector<list<PreferenceSetAndCost> > factPreferenceCost(achievedInLayer->size());
    
    preferenceViolationCostOfUsingFact = &factPreferenceCost;
    
    vector<set<int> > localLandmarkPreferenceViolationsBeforeFact(achievedInLayer->size());
    
    landmarkPreferenceViolationsBeforeFact = &(localLandmarkPreferenceViolationsBeforeFact);
    
    vector<ActionViolationData> actionCosts(initialUnsatisfiedStartPreconditions->size());
    preferenceViolationCostOfUsingAction = &actionCosts;
    
    vector<AutomatonPosition> optimisticPrefStatus(theState.preferenceStatus);
    optimisticStatusOfEachPreference = &optimisticPrefStatus;
    
    prefCostOfDeletingFact.clear();
    prefCostOfChangingNumberA.clear();
    
    prefCostOfAddingFact.clear();
    prefCostOfChangingNumberB.clear();
    
    PreferenceHandler::getCostsOfDeletion(theState, prefCostOfDeletingFact, prefCostOfChangingNumberA);
    PreferenceHandler::getCostsOfAdding(theState, prefCostOfAddingFact, prefCostOfChangingNumberB);
    
    preferenceWouldBeViolatedByAction.clear();
    preferencePartsToSatisfyBeforeAction.clear();
    
    factsThatAreNowAffordable.clear();
    negativeFactsThatAreNowAffordable.clear();
    
    actionPreviouslyTooExpensive.resize(initialUnsatisfiedStartPreconditions->size(), false);

	{
		set<int>::iterator stateItr = theState.first.begin();
		const set<int>::iterator stateEnd = theState.first.end();

		for (;stateItr != stateEnd; ++stateItr) {
			(*achievedInLayer)[*stateItr] = 0.0;
            (*preferenceViolationCostOfUsingFact)[*stateItr].push_back(PreferenceSetAndCost(true));            
            (*negativeAchievedInLayer)[*stateItr] = -1.0;
            if (RPGBuilder::costPropagationMethod != E_NOCOSTS) {
                propositionCosts[*stateItr] = 0.0;
            }
		}
	}

	{
		set<int>::iterator gfItr = goalFluents.begin();
		const int loopLim = rpgNumericPreconditions->size();
		for (int i = 0; i < loopLim; ++i) {
			if ((*rpgNumericPreconditions)[i].isSatisfied(*maxFluentTable)) {
				(*numericAchievedInLayer)[i] = 0.0;
				(*numericAchievedBy)[i] = 0;
                (*negativeNumericAchievedInLayer)[i] = 0.0;
			}
		}
	}
	
    setInitialNegativePreconditionsOfPreferences(theState);
	
	set<int> previousFacts;
	

	
	map<double, FactLayerEntry, EpsilonComp > factLayers;

	


	/*
	minFluentTables.push_back(minFluentTables.back())
	vector<double> * nextFluentLayerMin = &minFluentTables.back();
	maxFluentTables.push_back(maxFluentTables.back())
	vector<double> * nextFluentLayerMax = &maxFluentTables.back();
*/
	const int easSize = initialUnsatisfiedStartPreconditions->size();
	vector<double> startActionSchedule(easSize, -1.0);
	

	


	map<int, int> forbiddenStart;
	
	list<pair<set<int>, set<int> > > setsToForbid;

//	static vector<double> earliestTimeTIL(tilCount);
//	static vector<double> latestTimeTIL(tilCount);

	
	list<ActionFluentModification> scrapAFMs;

	noLongerForbidden.clear();

    map<int,double> propsSatisfied; // propositional goals that were satisfied by the LP, with the layer they were achieved by
    
	map<double, list<int>, EpsilonComp > endActionsAtTime;

    double initialLag = EPSILON;
    
	{
		if (evaluateDebug) cout << "Considering preconditionless actions\n";
		updateActionsForFactWithPenalties(-1, theState, startPreconditionCounts, numericStartPreconditionCounts, factLayers, fluentLayers, fluentModifications, 0.0, endActionsAtTime, startActionSchedule, goals, gsEnd, goalFluents, gfEnd, unsatisfiedGoals, unappearedEnds, insistUponEnds, forbiddenStart);
	}

	

    
	if (unsatisfiedGoals || unappearedEnds) {
	
		if (evaluateDebug) cout << "Considering initial propositional facts\n";
		set<int>::iterator stateItr = theState.first.begin();
		const set<int>::iterator stateEnd = theState.first.end();

		for (; stateItr != stateEnd; ++stateItr) {
			if (evaluateDebug) cout << "Updating from fact " << *stateItr << " " << *(RPGBuilder::getLiteral(*stateItr)) << "\n";
			if (updateActionsForFactWithPenalties(*stateItr, theState, startPreconditionCounts, numericStartPreconditionCounts, factLayers, fluentLayers, fluentModifications, 0.0, endActionsAtTime, startActionSchedule, goals, gsEnd, goalFluents, gfEnd, unsatisfiedGoals, unappearedEnds, insistUponEnds, forbiddenStart)) break;
            updatePreferencesForFact(theState, *stateItr, true, true, 0.0, initialLag, rpgGoalPrefViolation);
		}

			
	}

//	const bool milpDebug = true;

	if (unsatisfiedGoals || unappearedEnds) {
		if (evaluateDebug) cout << "Considering initial numeric facts\n";
		const int loopLim = rpgNumericPreconditions->size();
		
		for (int i = 0; i < loopLim; ++i) {
			if ((*rpgNumericPreconditions)[i].isSatisfied(*maxFluentTable)) {
				if (evaluateDebug) cout << "Satisfied at 0.000: " << (*rpgNumericPreconditions)[i] << "\n";
				if (evaluateDebug) cout << "Updating from numeric fact " << i << ":  " << (*rpgNumericPreconditions)[i] << "\n";
				if (updateActionsForNumericFactWithPenalties(i, theState, startPreconditionCounts, numericStartPreconditionCounts, factLayers, fluentLayers, fluentModifications, 0.0, endActionsAtTime, startActionSchedule, goals, gsEnd, goalFluents, gfEnd, unsatisfiedGoals, unappearedEnds, insistUponEnds, forbiddenStart)) break;
                updatePreferencesForFact(theState,i, false, true, 0.0, initialLag, rpgGoalPrefViolation);
			}
		}
	
	}		


    if (unsatisfiedGoals || unappearedEnds) {
        processAnyCheaperFacts(EPSILON, initialLag);                
    }
    
    set<int> prefsUnsatisfied;
    
    //cout << "Any lag shift {" << 0.0 << "," << initialLag << "} after initial layer?\n";
    const double & layerAfterFirstTS = shiftFactLayersByLag(0.0, initialLag, factLayers, fluentLayers);
    //cout << "Done shift if there was one.\n";
    
    importNowAffordableFact(factLayers, goals, unsatisfiedGoals);
    
    
    {
        const bool prefsPreviouslyRemaining = (fabs(lpGoalPrefViolation) >= 0.0001);
        
        bool alreadyAddedApplicableActions = false;
        
        if (unsatisfiedGoals == fakeGoalCount) {
            if (evaluateDebug || prefDebug) {
                cout << "All remaining goals are fake\n";
            }
            
            if (metricrpg.get()) {
                numericGoalsSatisfied = true;
            }
            
            if (milprpg.get()) {
                                
                if (evaluateDebug || prefDebug) {
                    cout << "Extending MILPRPG...";
                    cout.flush();
                }   
                const bool previouslyUnsatisfied = !numericGoalsSatisfied;
                {
                    const int actLim = startPreconditionCounts.size();
                    for (int ae = 0; ae < actLim; ++ae) {
                        assert(RPGBuilder::rogueActions[ae] || startPreconditionCounts[ae] || numericStartPreconditionCounts[ae] || startActionSchedule[ae] >= 0.0);
                    }
                }
                
                milprpg->addApplicableActions(&startPreconditionCounts, &numericStartPreconditionCounts, layerAfterFirstTS);
                alreadyAddedApplicableActions = true;
                if (evaluateDebug || prefDebug) {
                    cout << "solving...";
                    cout.flush();
                }
                numericGoalsSatisfied = milprpg->canSatisfyGoalConstraints(lpGoalPrefViolation, maxPrefCost, &theState, *optimisticStatusOfEachPreference, &startPreconditionCounts, &numericStartPreconditionCounts, propsSatisfied, prefsUnsatisfied);
                
                if (numericGoalsSatisfied) {
                    if (evaluateDebug || prefDebug) {
                        cout << "was solved successfully";
                        cout.flush();
                    }
                    
                    if (previouslyUnsatisfied) {
                        if (evaluateDebug || prefDebug) {
                            cout << ", for the first time";
                            cout.flush();
                        }
                        --fakeGoalCount;
                        --unsatisfiedGoals;
                        
                        if (!prefsPreviouslyRemaining && fabs(lpGoalPrefViolation) >= 0.0001) {
                            if (evaluateDebug || prefDebug) {
                                cout << ", but now some preferences have been violated";
                            }
                            ++fakeGoalCount;
                            ++unsatisfiedGoals;
                        }
                    }
                    if (evaluateDebug || prefDebug) {
                        cout << "\n";
                    }
                }                    
                
                if (numericGoalsSatisfied && prefsPreviouslyRemaining) {
                    if (fabs(lpGoalPrefViolation) < 0.0001) {
                        if (prefDebug) {
                            cout << "All preferences now satisfied\n";
                        }
                        --fakeGoalCount;
                        --unsatisfiedGoals;
                    }
                }
            } else {
                lpGoalPrefViolation = rpgGoalPrefViolation;
                assert(numericGoalsSatisfied);
                if (prefsPreviouslyRemaining) {
                    if (fabs(lpGoalPrefViolation) < 0.0001) {
                        if (prefDebug) {
                            cout << "All preferences now satisfied\n";
                        }
                        --fakeGoalCount;
                        --unsatisfiedGoals;
                    }
                }
            }
        }
    
    
        if ((unsatisfiedGoals || unappearedEnds) && (milprpg.get() || metricrpg.get())) {

            //cout << COLOUR_light_red << "Calling LP for fluent layer " << layerAfterFirstTS << COLOUR_default << endl;
            map<double, vector<double>, EpsilonComp >::iterator flItr = fluentLayers.insert(pair<double, vector<double> >(layerAfterFirstTS, fluentLayers[0.0])).first;
            vector<double> & newTable = flItr->second;

            if (!alreadyAddedApplicableActions) {
                {
                    const int actLim = startPreconditionCounts.size();
                    for (int ae = 0; ae < actLim; ++ae) {
                        assert(RPGBuilder::rogueActions[ae] || startPreconditionCounts[ae] || numericStartPreconditionCounts[ae] || startActionSchedule[ae] >= 0.0);
                    }
                }
                if (milprpg.get()) {
                    milprpg->addApplicableActions(&startPreconditionCounts, &numericStartPreconditionCounts, layerAfterFirstTS);
                }
                if (metricrpg.get()) {
                    metricrpg->addApplicableActions(&startPreconditionCounts, &numericStartPreconditionCounts, layerAfterFirstTS);
                }
            }
            
            if (milprpg.get()) {
                milprpg->fillMinMaxFluentTable(newTable);
            }
            if (metricrpg.get()) {
                metricrpg->fillMinMaxFluentTable(newTable);
            }

            {
                const int startLim = vCount * 2;
                const int endLim = startLim + avCount;
                for (int i = startLim; i < endLim; ++i) {
                    newTable[i] = RPGBuilder::getArtificialVariable(i).evaluate(newTable);
                }
            }

            const int loopLim = rpgNumericPreconditions->size();


            for (int i = 0; i < loopLim; ++i) {                                    
                double & currAIL = (*numericAchievedInLayer)[i];
                if (currAIL == -1.0) {
                    if ((*rpgNumericPreconditions)[i].isSatisfied(newTable)) {
                        if (evaluateDebug) cout << "Satisfied at "<< layerAfterFirstTS << ": " << (*rpgNumericPreconditions)[i] << "\n";
                        currAIL = layerAfterFirstTS;
                        if (goalFluents.find(i) != gfEnd) {
                            if (!(--unsatisfiedGoals) && !unappearedEnds) break;
                        }
                        factLayers[layerAfterFirstTS].second.push_back(i);
                    }
                }
            }            
            for (int i = 0; i < loopLim; ++i) {                                    
                double & currAIL = (*negativeNumericAchievedInLayer)[i];
                if (currAIL == -1.0) {
                    if ((*rpgNumericPreconditions)[i].canBeUnsatisfied(newTable)) {
                        if (evaluateDebug) cout << "Satisfied at "<< layerAfterFirstTS << ": ¬" << (*rpgNumericPreconditions)[i] << "\n";
                        currAIL = layerAfterFirstTS;
                        if (goalFluents.find(i) != gfEnd) {
                            if (!(--unsatisfiedGoals) && !unappearedEnds) break;
                        }
                        factLayers[layerAfterFirstTS].negativeNumerics.push_back(i);
                    }
                }
            }            
                                    
        }
    }
	
	if (evaluateDebug) {
		cout << "Unsatisfied goals: " << (unsatisfiedGoals - fakeGoalCount) << ", Unappeared ends: " << unappearedEnds << "\n";
        if (unsatisfiedGoals == fakeGoalCount) {
            if (!numericGoalsSatisfied) {
                cout << "Numeric goal conjunct cannot be satisfied\n";
            }
            const bool prefsPreviouslyRemaining = (fabs(lpGoalPrefViolation) >= 0.0001);
            if (prefsPreviouslyRemaining) {
                cout << "Reached a preference violation cost of " << lpGoalPrefViolation << ", but in theory can reach 0\n";
            }
        }
        
	}
	
    
	while ((unsatisfiedGoals || unappearedEnds) && !factLayers.empty()) {

        const bool prefsPreviouslyRemaining = (fabs(lpGoalPrefViolation) >= 0.0001);
		if (evaluateDebug) {
			cout << "Unsatisfied goals: " << (unsatisfiedGoals - fakeGoalCount) << ", Unappeared ends: " << unappearedEnds << "\n";
            if (unsatisfiedGoals == fakeGoalCount) {
                if (!numericGoalsSatisfied) {
                    cout << "Numeric goal conjunct cannot be satisfied\n";
                }
                if (prefsPreviouslyRemaining) {
                    cout << "Reached a preference violation cost of " << lpGoalPrefViolation << ", but in theory can reach 0\n";
                }
            }
			cout << "Expanding RPG forwards\n";
		}

        if (evaluateDebug) cout << "FACT LAYER AT TIME " << factLayers.begin()->first << "\n";

        const double cTime = factLayers.begin()->first;

        
        double maxLag = cTime + EPSILON;
        //cout << "At layer " << cTime << ", so default lag is " << maxLag << endl;
        {
            if (fluentLayers.find(cTime + EPSILON) == fluentLayers.end()) {
                const vector<double> & backFluents = fluentLayers.rbegin()->second;
                fluentLayers.insert(pair<double, vector<double> >(cTime + EPSILON, backFluents));
                
            }
        }

		
			


        {
            list<int>::iterator stateItr = factLayers.begin()->second.first.begin();
            const list<int>::iterator stateEnd = factLayers.begin()->second.first.end();
    
            for (; stateItr != stateEnd; ++stateItr) {
                if (evaluateDebug) cout << "Updating actions from fact " << *stateItr << " " << *(RPGBuilder::getLiteral(*stateItr)) << endl;
                if (updateActionsForFactWithPenalties(*stateItr, theState, startPreconditionCounts, numericStartPreconditionCounts, factLayers, fluentLayers, fluentModifications, cTime, endActionsAtTime, startActionSchedule, goals, gsEnd, goalFluents, gfEnd, unsatisfiedGoals, unappearedEnds, insistUponEnds, forbiddenStart)) break;
                                
                #ifdef PARANOIA
                map<int, pair<int,double> >::const_iterator nfItr = factsThatAreNowAffordable.begin();
                const map<int, pair<int,double> >::const_iterator nfEnd = factsThatAreNowAffordable.end();
                
                for (; nfItr != nfEnd; ++nfItr) {
                    assert((*achievedInLayer)[nfItr->first] < 0.0);
                }
                
                const int fCount = achievedInLayer->size();
                for (int f = 0; f < fCount; ++f) {
                    if ((*achievedInLayer)[f] > 0.0) {
                        assert((*preferenceViolationCostOfUsingFact)[f].back().atLayer >= (*achievedInLayer)[f]);
                    }
                }
                #endif
            }
                        
        }

        if (unsatisfiedGoals || unappearedEnds) {
            list<int>::iterator stateItr = factLayers.begin()->second.first.begin();
            const list<int>::iterator stateEnd = factLayers.begin()->second.first.end();
            
            for (; stateItr != stateEnd; ++stateItr) {
                if (evaluateDebug) cout << "Updating preferences from fact " << *stateItr << " " << *(RPGBuilder::getLiteral(*stateItr)) << endl;
                updatePreferencesForFact(theState, *stateItr, true, true, cTime, maxLag, rpgGoalPrefViolation);
                #ifdef PARANOIA
                map<int, pair<int,double> >::const_iterator nfItr = factsThatAreNowAffordable.begin();
                const map<int, pair<int,double> >::const_iterator nfEnd = factsThatAreNowAffordable.end();
                
                for (; nfItr != nfEnd; ++nfItr) {
                    assert((*achievedInLayer)[nfItr->first] < 0.0);
                }
                const int fCount = achievedInLayer->size();
                for (int f = 0; f < fCount; ++f) {
                    if ((*achievedInLayer)[f] > 0.0) {
                        assert((*preferenceViolationCostOfUsingFact)[f].back().atLayer >= (*achievedInLayer)[f]);
                    }
                }
                #endif
            }
        }
        
        
        if (unsatisfiedGoals || unappearedEnds) {
            list<int>::iterator stateItr = factLayers.begin()->second.negativeLiterals.begin();
            const list<int>::iterator stateEnd = factLayers.begin()->second.negativeLiterals.end();
            
            for (; stateItr != stateEnd; ++stateItr) {
                if (evaluateDebug) cout << "Updating preferences from negative fact " << *stateItr << " ¬" << *(RPGBuilder::getLiteral(*stateItr)) << endl;
                updatePreferencesForFact(theState, *stateItr, true, false, cTime, maxLag, rpgGoalPrefViolation);
            }
            const int fCount = achievedInLayer->size();
            for (int f = 0; f < fCount; ++f) {
                if ((*achievedInLayer)[f] > 0.0) {
                    assert((*preferenceViolationCostOfUsingFact)[f].back().atLayer >= (*achievedInLayer)[f]);
                }
            }
        }

        if (unsatisfiedGoals || unappearedEnds) {
            list<int>::iterator stateItr = factLayers.begin()->second.second.begin();
            const list<int>::iterator stateEnd = factLayers.begin()->second.second.end();
    
            for (; stateItr != stateEnd; ++stateItr) {
                if (evaluateDebug) cout << "Updating from numeric fact " << *stateItr << "\n";
                if (updateActionsForNumericFactWithPenalties(*stateItr, theState, startPreconditionCounts, numericStartPreconditionCounts, factLayers, fluentLayers, fluentModifications, cTime, endActionsAtTime, startActionSchedule, goals, gsEnd, goalFluents, gfEnd, unsatisfiedGoals, unappearedEnds, insistUponEnds, forbiddenStart)) break;
                updatePreferencesForFact(theState, *stateItr, false, true, cTime, maxLag, rpgGoalPrefViolation);
            }

        }

        if (unsatisfiedGoals || unappearedEnds) {
            list<int>::iterator stateItr = factLayers.begin()->second.negativeNumerics.begin();
            const list<int>::iterator stateEnd = factLayers.begin()->second.negativeNumerics.end();
            
            for (; stateItr != stateEnd; ++stateItr) {
                if (evaluateDebug) cout << "Updating from negative numeric fact ¬" << *stateItr << "\n";
                updatePreferencesForFact(theState, *stateItr, false, false, cTime, maxLag, rpgGoalPrefViolation);
            }
            
        }
        

        if (unsatisfiedGoals || unappearedEnds) {
            {
                #ifdef PARANOIA
                map<int, pair<int,double> >::const_iterator nfItr = factsThatAreNowAffordable.begin();
                const map<int, pair<int,double> >::const_iterator nfEnd = factsThatAreNowAffordable.end();
                
                for (; nfItr != nfEnd; ++nfItr) {
                    assert((*achievedInLayer)[nfItr->first] < 0.0);
                }
                #endif
            }       
            processAnyCheaperFacts(cTime + EPSILON, maxLag);
            
            {
                #ifdef PARANOIA
                map<int, pair<int,double> >::const_iterator nfItr = factsThatAreNowAffordable.begin();
                const map<int, pair<int,double> >::const_iterator nfEnd = factsThatAreNowAffordable.end();
                
                for (; nfItr != nfEnd; ++nfItr) {
                    assert((*achievedInLayer)[nfItr->first] < 0.0);
                }
                #endif
            }
            
        }

        importNowAffordableFact(factLayers, goals, unsatisfiedGoals, true);

        const double nextLayerTS = shiftFactLayersByLag(cTime, maxLag, factLayers, fluentLayers, true);

        bool alreadyAddedApplicableActions = false;
        
        if (unsatisfiedGoals == fakeGoalCount) {
            if (evaluateDebug || prefDebug) {
                cout << "All remaining goals are fake\n";
            }
            
            if (metricrpg.get()) {
                numericGoalsSatisfied = true;
            }
            
            if (milprpg.get()) {
                const bool previouslyUnsatisfied = !numericGoalsSatisfied;
                
                if (evaluateDebug || prefDebug) {
                    cout << "Extending MILPRPG...";
                    cout.flush();
                }   
                {
                    const int actLim = startPreconditionCounts.size();
                    for (int ae = 0; ae < actLim; ++ae) {
                        assert(RPGBuilder::rogueActions[ae] || startPreconditionCounts[ae] || numericStartPreconditionCounts[ae] || startActionSchedule[ae] >= 0.0);
                    }
                }
                                                    
                milprpg->addApplicableActions(&startPreconditionCounts, &numericStartPreconditionCounts, nextLayerTS);
                
                alreadyAddedApplicableActions = true;
                if (evaluateDebug || prefDebug) {
                    cout << "solving...";
                    cout.flush();
                }
                numericGoalsSatisfied = milprpg->canSatisfyGoalConstraints(lpGoalPrefViolation, maxPrefCost, &theState, *optimisticStatusOfEachPreference, &startPreconditionCounts, &numericStartPreconditionCounts, propsSatisfied, prefsUnsatisfied);
                
                if (numericGoalsSatisfied) {
                    if (evaluateDebug || prefDebug) {
                        cout << "was solved successfully";
                        cout.flush();
                    }

                    if (previouslyUnsatisfied) {
                        if (evaluateDebug || prefDebug) {
                            cout << ", for the first time";
                            cout.flush();
                        }
                        --fakeGoalCount;
                        --unsatisfiedGoals;
                        
                        if (!prefsPreviouslyRemaining && fabs(lpGoalPrefViolation) >= 0.0001) {
                            if (evaluateDebug || prefDebug) {
                                cout << ", but now some preferences have been violated";
                            }
                            ++fakeGoalCount;
                            ++unsatisfiedGoals;
                        }
                    }
                    if (evaluateDebug || prefDebug) {
                        cout << "\n";
                    }
                }                    
            
                if (numericGoalsSatisfied && prefsPreviouslyRemaining) {
                    if (fabs(lpGoalPrefViolation) < 0.0001) {
                        if (prefDebug) {
                            cout << "All preferences now satisfied\n";
                        }
                        --fakeGoalCount;
                        --unsatisfiedGoals;
                    }
                }
            } else {
                lpGoalPrefViolation = rpgGoalPrefViolation;
                assert(numericGoalsSatisfied);
                if (prefsPreviouslyRemaining) {
                    if (fabs(lpGoalPrefViolation) < 0.0001) {
                        if (prefDebug) {
                            cout << "All preferences now satisfied\n";
                        }
                        --fakeGoalCount;
                        --unsatisfiedGoals;
                    }
                }
            }
            
        }
	
        if ((unsatisfiedGoals || unappearedEnds) && (milprpg.get() || metricrpg.get())) {
    
            //cout << COLOUR_light_red << "At " << cTime << ", calling LP for fluent layer " << nextLayerTS << COLOUR_default << endl;
            
            assert(fluentLayers.find(cTime) != fluentLayers.end());
            map<double, vector<double>, EpsilonComp >::iterator flItr = fluentLayers.insert(pair<double, vector<double> >(nextLayerTS, fluentLayers[cTime])).first;
            vector<double> & newTable = flItr->second;
    
            if (!alreadyAddedApplicableActions) {
                {
                    const int actLim = startPreconditionCounts.size();
                    for (int ae = 0; ae < actLim; ++ae) {
                        assert(RPGBuilder::rogueActions[ae] || startPreconditionCounts[ae] || numericStartPreconditionCounts[ae] || startActionSchedule[ae] >= 0.0);
                    }
                }
                if (milprpg.get()) {
                    milprpg->addApplicableActions(&startPreconditionCounts, &numericStartPreconditionCounts, nextLayerTS);
                }
                if (metricrpg.get()) {
                    metricrpg->addApplicableActions(&startPreconditionCounts, &numericStartPreconditionCounts, nextLayerTS);
                }
            }
            if (milprpg.get()) {
                cout << "}"; cout.flush();  
                milprpg->fillMinMaxFluentTable(newTable);
            }
            if (metricrpg.get()) {
                metricrpg->fillMinMaxFluentTable(newTable);
            }
            
            {
                const int startLim = vCount * 2;
                const int endLim = startLim + avCount;
                for (int i = startLim; i < endLim; ++i) {
                    newTable[i] = RPGBuilder::getArtificialVariable(i).evaluate(newTable);
                }
            }

            const int loopLim = rpgNumericPreconditions->size();

    
            for (int i = 0; i < loopLim; ++i) {
                double & currAIL = (*numericAchievedInLayer)[i];
                if (currAIL == -1.0) {
                    if ((*rpgNumericPreconditions)[i].isSatisfied(newTable)) {
                        if (evaluateDebug) cout << "Satisfied at "<< nextLayerTS << ": " << (*rpgNumericPreconditions)[i] << "\n";

                        currAIL = nextLayerTS;
                        
                        if (goalFluents.find(i) != gfEnd) {
                            if (!(--unsatisfiedGoals) && !unappearedEnds) break;
                        }
                        factLayers[nextLayerTS].second.push_back(i);
                    }
                }
            }
            
    
        }

        if (evaluateDebug) cout << "Finished fact layer\n";


        factLayers.erase(factLayers.begin());
        
	}

    if (metricrpg.get() && unsatisfiedGoals <= fakeGoalCount) {
        numericGoalsSatisfied = true;
    }

	if ((unsatisfiedGoals > fakeGoalCount) || unappearedEnds || !numericGoalsSatisfied) {
		//cout << "Unsatisfied goals :(\n";
        if (evaluateDebug) {
            cout << "Dead end found in RPG\n";
            
            {
                set<int>::const_iterator gItr = goals.begin();
                const set<int>::const_iterator gEnd = goals.end();

                for (; gItr != gEnd; ++gItr) {
                    if ((*achievedInLayer)[*gItr] < 0.0) {
                        cout << "Goal " << *gItr << ", " << *(RPGBuilder::getLiteral(*gItr)) << ", was never seen. ";

                        cout << "Possible achievers: " << (*(effectsToActions))[*gItr].size() << endl;
                        list<pair<int, VAL::time_spec> >::const_iterator aItr = (*(effectsToActions))[*gItr].begin();
                        const list<pair<int, VAL::time_spec> >::const_iterator aEnd = (*(effectsToActions))[*gItr].end();

                        for (; aItr != aEnd; ++aItr) {
                            if (aItr->second == VAL::E_AT_START) {
                                if (startActionSchedule[aItr->first] == -1.0) {
                                    cout << "- Start of " << aItr->first << ", " << *(RPGBuilder::getInstantiatedOp(aItr->first)) << ", was never seen\n";
                                }
                                list<Literal*> & actPres = (*(actionsToProcessedStartPreconditions))[aItr->first];
                                list<Literal*>::const_iterator preItr = actPres.begin();
                                const list<Literal*>::const_iterator preEnd = actPres.end();
                                for (; preItr != preEnd; ++preItr) {
                                    if ((*(achievedInLayer))[(*preItr)->getStateID()] < 0.0) {
                                        cout << "   * Start precondition " << *(*preItr) << " never appeared\n";
                                    }
                                }
                            }
                        }
                    }
                }
            }
            set<int>::const_iterator gfItr = goalFluents.begin();
            const set<int>::const_iterator gfEnd = goalFluents.end();

            for (; gfItr != gfEnd; ++gfItr) {
                if ((*numericAchievedInLayer)[*gfItr] < 0.0) {
                    cout << "Goal " << *gfItr << " , " << RPGBuilder::getNumericPreTable()[*gfItr] << ", was never seen. ";

                    
                }
            }
        }
        
        
        return EvaluationInfo();
	} else {
        if (fakeGoalCount) {
            --unsatisfiedGoals;
            if (evaluateDebug) cout << "RPG found all goals and ends, but could not satisfy some preferences\n";
        } else {
            if (evaluateDebug) cout << "RPG found all goals and ends, and satisfied all preferences\n";
        }
	}

    if (lpGoalPrefViolation >  maxPrefCost) {
        
        if (prefDebug) {
            cout << COLOUR_red << "Preference violations needed to reach goal are too expensive - can only get down to " << lpGoalPrefViolation << COLOUR_default << " but aiming for " << maxPrefCost << " (state has itself violated " << GCost << " and cap on quality is " << maxPrefCostIn << ")\n";
            
            const int printCount = optimisticStatusOfEachPreference->size();
            for (int pp = 0; pp < printCount; ++pp) {
                const AutomatonPosition newPosn = (*optimisticStatusOfEachPreference)[pp];
                cout << "\tStatus of " << RPGBuilder::getPreferences()[pp].name << ":" << pp << " - " << positionName[newPosn] << endl;
            }
            
        }
        return EvaluationInfo(); 
    } else {
        if (prefDebug) {
            cout << COLOUR_red << "Got preference violations down to " << lpGoalPrefViolation << COLOUR_default << endl;
        }
    }

	map<double, RPGRegress, EpsilonComp> goalsAtLayer;

	set<int> numericsFreeFor;
	//set<int> factsFreeFor;

	double h = heuristicOffset;

//	const bool tmpDebug = true;
//	if (tmpDebug) {
//		cout << "h = " << heuristicOffset;
//		cout.flush();
//	}

//	evaluateDebug = true;
	
	{
		set<int>::iterator gsItr = goals.begin();

		for (; gsItr != gsEnd; ++gsItr) {
			const int currGoal = *gsItr;
            if (propsSatisfied.find(currGoal) != propsSatisfied.end()) continue;
			const double insLayer = (*achievedInLayer)[currGoal];
			if (insLayer > 0.0) {
                PreferenceSetAndCost & costData = (*preferenceViolationCostOfUsingFact)[currGoal].back();
				goalsAtLayer[costData.atLayer].propositionalGoals.insert(make_pair(currGoal, make_pair(costData.achiever,1.0)));
                if (evaluateDebug) cout << "Goal " << *gsItr << " to be achieved by " << costData.achiever << " in layer with TS " << costData.atLayer << "\n";
                
                assert(costData.atLayer >= (*achievedInLayer)[currGoal]);
			} else if (evaluateDebug) cout << "Goal achieved in initial state, not adding to RPG regression\n";
		}
	}


    addGoalsForPreferences(theState, goalsAtLayer, propsSatisfied, prefsUnsatisfied);

	if (!MILPRPG::addNumGoalConjunct || metricrpg.get()) {
		set<int>::iterator gfItr = goalFluents.begin();

		for (; gfItr != gfEnd; ++gfItr) {
			const int currGoal = *gfItr;
			const double insLayer = (*numericAchievedInLayer)[currGoal];
			if (insLayer > 0.0) {
				goalsAtLayer[insLayer].numericGoals.insert(pair<int, double>(currGoal, 1.0));
				if (evaluateDebug) cout << "Numeric goal to be achieved in layer with TS " << insLayer << "\n";
			} else if (evaluateDebug) cout << "Numeric goal achieved in initial state, not adding to RPG regression\n";
		}
	}
    
    if (MILPRPG::addNumGoalConjunct && milprpg.get() && !metricrpg.get()) {
        goalsAtLayer[DBL_MAX].numericGoals.insert(pair<int,double>(-1,1.0));
        if (evaluateDebug) cout << "Dummy numeric goal achievement added to RPG\n";
    }



	howManyTimesIsHelpful.clear();

	pair<int, VAL::time_spec> earliestTIL(-1,VAL::E_AT);
	
//	evaluateDebug = true;

    bool extractDebug = (evaluateDebug);

    bool carryOn = true;
    
    
    list<pair<double, list<ActionAndHowManyTimes > > > previousRelaxedPlan;
    
    while (carryOn) {
        
        carryOn = false;
    
        while (!goalsAtLayer.empty()) {

            const double currTS = goalsAtLayer.rbegin()->first;
            map<int, pair<int,double> > & currGAL = goalsAtLayer.rbegin()->second.propositionalGoals;
            map<int, double> & currNGAL = goalsAtLayer.rbegin()->second.numericGoals;
            
            bool alreadyPushed = false;

            

            {
                
                if (currTS == DBL_MAX) {
                    if ((!currGAL.empty() || !currNGAL.empty()) && !alreadyPushed) {
                        relaxedPlan.push_front(pair<double, list<ActionAndHowManyTimes> >( DBL_MAX, list<ActionAndHowManyTimes>() ));
                        alreadyPushed = true;
                    }                                
                } else {
                    if ((!currGAL.empty() || !currNGAL.empty()) && !alreadyPushed) {
                        relaxedPlan.push_front(pair<double, list<ActionAndHowManyTimes > >( currTS - EPSILON, list<ActionAndHowManyTimes >() ));
                        alreadyPushed = true;
                    }
            
                    if (extractDebug && !currGAL.empty()) cout << "Finding achievers for goals at TS " << currTS << "\n";
                
                
                    while (!currGAL.empty()) {
                        const map<int, pair<int, double> >::iterator nta = currGAL.begin();
                        const int nextToAchieve = nta->first;
                        const int notedToUse = nta->second.first;
                        const double tilR = nta->second.second;
                        currGAL.erase(nta);
                        if (extractDebug) cout << "\tGoal " << nextToAchieve << " with weight " << tilR << " - " << *(RPGBuilder::getLiteral(nextToAchieve)) << "\n";
                        
                        pair<int, VAL::time_spec> currAchievedBy;
                        
                        if (notedToUse == -1) {
                        
                            list<PreferenceSetAndCost>::reverse_iterator costItr = (*preferenceViolationCostOfUsingFact)[nextToAchieve].rbegin();
                            while ((costItr->atLayer - currTS) > 0.00001) {
                                ++costItr;
                            }
                            
                            currAchievedBy = make_pair(costItr->achiever, VAL::E_AT_START);
                        } else {
                            
                            currAchievedBy = make_pair(notedToUse, VAL::E_AT_START);
                            
                        } 
                        
                    
                        
                        if (propsSatisfied.find(nextToAchieve) == propsSatisfied.end()) {
                        
                            {	
                                assert(currAchievedBy.second == VAL::E_AT_START);
                                if (extractDebug) cout << "\t\tUsing start of " << currAchievedBy.first << ", " << *(RPGBuilder::getInstantiatedOp(currAchievedBy.first)) << endl;
                                if (extractDebug && currTS == EPSILON) cout << "\t\tIs a helpful action\n";
                                relaxedPlan.front().second.push_back(ActionAndHowManyTimes(getOp(currAchievedBy.first), VAL::E_AT_START, tilR));
                                h += tilR;
                
                                if (currTS == EPSILON) {
                                    helpfulActions.push_back(currAchievedBy);
                                    pair<double, double> & twoVals = (howManyTimesIsHelpful.insert(pair<int,pair<double, double> >(currAchievedBy.first,pair<double,double>(0.0,0.0))).first->second);
                                    twoVals.first += 1.0;
                                    twoVals.second += tilR;
                                    
                                }
                
                                {
                                    list<Literal*> & actionEffectsList = (*actionsToStartEffects)[currAchievedBy.first];
                                    list<Literal*>::iterator aelItr = actionEffectsList.begin();
                                    const list<Literal*>::iterator aelEnd = actionEffectsList.end();
                    
                                    for (; aelItr != aelEnd; ++aelItr) {
                                        map<int, pair<int, double> >::iterator cgItr = currGAL.find((*aelItr)->getStateID());
                                        if (cgItr != currGAL.end()) {
                                            if (cgItr->second.second <= tilR || true) {
                                                currGAL.erase(cgItr);
                                            } else {
                                                cgItr->second.second -= tilR;
                                            }
                                        }
                                    }
                                }
                    
                                /*{
                                    
                                    map<int, double>::iterator ngalItr = currNGAL.begin();
                                    const map<int, double>::iterator ngalEnd = currNGAL.end();
                
                                    while (ngalItr != ngalEnd) {
                                        ActionFluentModification* const afm = (*numericAchievedBy)[ngalItr->first];
                                        if (afm && afm->act == currAchievedBy.first && afm->ts == VAL::E_AT_START) {
                                            if (tilR > ngalItr->second) tilR = ngalItr->second;
                                            map<int, double>::iterator tItr = ngalItr;
                                            ++tItr;
                                            currNGAL.erase(ngalItr);
                                            ngalItr = tItr;
                                        } else {
                                            ++ngalItr;
                                        } 
                                    }
                                }*/
            
                                {
                                    list<Literal*> & actionPreconditionlist = (*actionsToProcessedStartPreconditions)[currAchievedBy.first];
                                    list<Literal*>::iterator aplItr = actionPreconditionlist.begin();
                                    const list<Literal*>::iterator aplEnd = actionPreconditionlist.end();
                                    if (extractDebug) cout << "\t\tPreconditions:\n";
                                    for (; aplItr != aplEnd; ++aplItr) {
                                        
                                        const int currPrec = (*aplItr)->getStateID();
                                        const double acIn = (*achievedInLayer)[currPrec];
                                        
                                        if (acIn > 0.0) {                                        
                                            if (extractDebug) {
                                                cout << "\t\tThinking about adding a requirement for " << currPrec << ", but seeing if already added by LP\n";
                                            }
                                            if (!LandmarksAnalysis::factIsALandmark(currPrec) || propsSatisfied.find(currPrec) == propsSatisfied.end()) {                                        
                                                
                                                
                                                list<PreferenceSetAndCost>::reverse_iterator pcostItr = (*preferenceViolationCostOfUsingFact)[currPrec].rbegin();
                                                while (pcostItr->atLayer >= currTS) {
                                                    if (extractDebug) {
                                                        cout << "\t\t\t" << pcostItr->atLayer << ", using " << pcostItr->achiever << " is too late\n";
                                                    }
                                                    ++pcostItr;
                                                }
                                                                                                                                        
                                                
                                                map<int, pair<int, double> >::iterator galItr = goalsAtLayer[pcostItr->atLayer].propositionalGoals.insert(make_pair(currPrec, make_pair(pcostItr->achiever,tilR))).first;
                                                if (galItr->second.second < tilR) galItr->second.second = tilR;
                                                if (extractDebug) cout << "\t\tAdding requirement for " << currPrec << " at time " << pcostItr->atLayer - EPSILON << " using " << pcostItr->achiever << " with weight " << galItr->second.second << "\n";
                                            }
                                        } else {
                                            if (extractDebug) cout << "\t\tPrecondition " << currPrec << " in initial state\n";
                                        }
                    
                                    }
                    
                                    if (extractDebug) cout << "\t\tPreconditions done\n";
                    
                                }
            
                                {
                                    list<int> & actionPreconditionlist = (*actionsToProcessedStartNumericPreconditions)[currAchievedBy.first];
                                    list<int>::iterator aplItr = actionPreconditionlist.begin();
                                    const list<int>::iterator aplEnd = actionPreconditionlist.end();
                    
                                    if (extractDebug) cout << "\t\tNumeric preconditions:\n";
                
                                    for (; aplItr != aplEnd; ++aplItr) {
                                        
                                        const int currPrec = (*aplItr);
                                        if (numericsFreeFor.find(currPrec) == numericsFreeFor.end()) {
                                            const double acIn = (*numericAchievedInLayer)[currPrec];
                                            if (acIn > 0.0) {
                                                map<int, double>::iterator galItr = goalsAtLayer[acIn].numericGoals.insert(pair<int, double>(currPrec, tilR)).first;
                                                if (galItr->second < tilR) galItr->second = tilR;
                                                if (extractDebug) cout << "\t\tAdding requirement for numeric precondition " << currPrec << " at time " << acIn << " with weight " << galItr->second << "\n";
                                            } else {
                                                if (extractDebug) cout << "\t\tNumeric precondition " << currPrec << " satisfied in initial state\n";
                                            }
                                        } else {
                                            if (extractDebug) cout << "\t\tNumeric precondition " << currPrec << " is free\n";
                                        }
                                    }
                    
                                    if (extractDebug) cout << "\t\tPreconditions done.\n";
                                }

                                addExtraPreconditionsForPreferences(currAchievedBy.first, currTS, tilR, propsSatisfied, goalsAtLayer);
                                
                                
                                
            
                            }
                        }
                    }

                    if (extractDebug && !currNGAL.empty()) cout << "Finding achievers for numeric goals at TS " << currTS << "\n";
                    
                }
                
                while (!currNGAL.empty()) {
                    const map<int, double>::iterator nta = currNGAL.begin();
                    const int nextToAchieve = nta->first;
                    double tilR = nta->second;
                    currNGAL.erase(nta);
                    
                    if (nextToAchieve >= 0) {
                        if (extractDebug) cout << "\tFluent goal " << nextToAchieve << "\n";
                    } else {
                        if (extractDebug) cout << "\tTop-level goals\n";
                    }

                    list<pair<int, double> > actionSeq;
                    
                    if (nextToAchieve < 0) {
                        assert(milprpg.get());
                        milprpg->getActionsForGoal(actionSeq);
                    } else {
                        if (milprpg.get()) {
                            const int v = (*rpgNumericPreconditions)[nextToAchieve].LHSVariable;
                            if (v < vCount) {
                                double RHS = (*rpgNumericPreconditions)[nextToAchieve].RHSConstant;
                                if ((*rpgNumericPreconditions)[nextToAchieve].op == VAL::E_GREATER) {
                                    RHS += 0.001;
                                }
                                milprpg->getActionsFor(currTS, v, false, RHS, actionSeq);
                            } else if (v < (2 * vCount)) {
                                double RHS = 0.0 - (*rpgNumericPreconditions)[nextToAchieve].RHSConstant;
                                if ((*rpgNumericPreconditions)[nextToAchieve].op == VAL::E_GREATER) {
                                    RHS -= 0.001;
                                }
                                milprpg->getActionsFor(currTS, v - vCount, true, RHS, actionSeq);
                            } else {
                                RPGBuilder::ArtificialVariable & av = RPGBuilder::getArtificialVariable(v);
                                for (int s = 0; s < av.size; ++s) {
                                    if (av.fluents[s] != -1) {
                                        if (av.fluents[s] < vCount) {
                                            milprpg->getActionsFor(currTS, av.fluents[s], false, -1.0, actionSeq);
                                        } else {
                                            milprpg->getActionsFor(currTS, av.fluents[s] - vCount, true, -1.0, actionSeq);
                                        }
                                    }
                                }
                            }				
                        }
                        if (metricrpg.get()) {
                            double RHS = (*rpgNumericPreconditions)[nextToAchieve].RHSConstant;
                            if ((*rpgNumericPreconditions)[nextToAchieve].op == VAL::E_GREATER) {
                                RHS += 0.001;
                            }                            
                            metricrpg->getActionsFor(currTS, (*rpgNumericPreconditions)[nextToAchieve].LHSVariable, false, RHS, actionSeq);
                        }
                    }

                    ActionFluentModification * currAchievedBy = buildAFM(actionSeq, scrapAFMs);
                    
                    if (nextToAchieve >= 0) {
                        (*numericAchievedBy)[nextToAchieve] = currAchievedBy;
                    }
                    
                    const bool debugLPPropPres = false;
                    
                   
                    while (currAchievedBy) {
                    
                        //const double kNeeded = ((currAchievedBy->howManyTimes * tilR > 1.0) ? 1.0 : currAchievedBy->howManyTimes * tilR);
                        const double kNeeded = tilR;
                        //const double kNeeded = (currAchievedBy->howManyTimes > 1.0 ? tilR : currAchievedBy->howManyTimes * tilR);
                        assert(currAchievedBy->ts == VAL::E_AT_START);
                        {
                            
                            if (extractDebug) cout << "\t\tUsing start of " << currAchievedBy->act << ", " << *(RPGBuilder::getInstantiatedOp(currAchievedBy->act)) << ", " << (tilR * currAchievedBy->howManyTimes) << " times\n";
                            if (extractDebug && currTS == EPSILON) cout << "\t\tIs a helpful action\n";
                            relaxedPlan.front().second.push_back(ActionAndHowManyTimes(getOp(currAchievedBy->act), VAL::E_AT_START, currAchievedBy->howManyTimes * tilR));
                            h += (currAchievedBy->howManyTimes * tilR);



                            if (   (milprpg.get() && milprpg->isHelpful(currAchievedBy->act))
                                || (metricrpg.get() && metricrpg->isHelpful(currAchievedBy->act))  ) {
                                helpfulActions.push_back(pair<int, VAL::time_spec>(currAchievedBy->act, VAL::E_AT_START));
                                //cout << "Extra helpful action: " << *(RPGBuilder::getInstantiatedOp(currAchievedBy->act)) << " to achieve " << (*rpgNumericPreconditions)[nextToAchieve] << " at " << currTS << "\n";

                                pair<double, double> & twoVals = (howManyTimesIsHelpful.insert(pair<int,pair<double,double> >(currAchievedBy->act,pair<double,double>(0.0,0.0))).first->second);
                                twoVals.first += currAchievedBy->howManyTimes;
                                twoVals.second += currAchievedBy->howManyTimes * tilR;
                            }
                
                            /*{
                                list<Literal*> & actionEffectsList = (*actionsToStartEffects)[currAchievedBy->act];
                                list<Literal*>::iterator aelItr = actionEffectsList.begin();
                                const list<Literal*>::iterator aelEnd = actionEffectsList.end();
                
                                for (; aelItr != aelEnd; ++aelItr) {
                                    factsFreeFor.insert((*aelItr)->getID());
                                }
                            }*/

                            /*{
                                
                                map<int, double>::iterator ngalItr = currNGAL.begin();
                                const map<int, double>::iterator ngalEnd = currNGAL.end();
            
                                while (ngalItr != ngalEnd) {
                                    ActionFluentModification* const afm = (*numericAchievedBy)[ngalItr->first];
                                    if (afm && afm->act == currAchievedBy->act && afm->ts == VAL::E_AT_START) {
                                        if (tilR > ngalItr->second) tilR = ngalItr->second;
                                        map<int, double>::iterator tItr = ngalItr;
                                        ++tItr;
                                        currNGAL.erase(ngalItr);
                                        ngalItr = tItr;
                                    } else {
                                        ++ngalItr;
                                    } 
                                }
                            }*/
        
                            {
                                list<Literal*> & actionPreconditionlist = (*actionsToProcessedStartPreconditions)[currAchievedBy->act];
                                list<Literal*>::iterator aplItr = actionPreconditionlist.begin();
                                const list<Literal*>::iterator aplEnd = actionPreconditionlist.end();
                                if (extractDebug || debugLPPropPres) cout << "\t\tPreconditions:\n";
                                for (; aplItr != aplEnd; ++aplItr) {
                                    
                                    const int currPrec = (*aplItr)->getStateID();
                                    const double acIn = (*achievedInLayer)[currPrec];
                                    
                                    if (acIn > 0.0) {
                                        if (!LandmarksAnalysis::factIsALandmark(currPrec) || propsSatisfied.find(currPrec) == propsSatisfied.end()) {                                        

                                            list<PreferenceSetAndCost>::reverse_iterator pcostItr = (*preferenceViolationCostOfUsingFact)[currPrec].rbegin();
                                            while (pcostItr->atLayer >= currTS) {
                                                ++pcostItr;
                                            }
                                            
                                            
                                            map<int, pair<int, double> >::iterator galItr = goalsAtLayer[pcostItr->atLayer].propositionalGoals.insert(make_pair(currPrec, make_pair(pcostItr->achiever, kNeeded))).first;
                                            if (galItr->second.second < kNeeded) galItr->second.second = kNeeded;
                                            if (extractDebug || debugLPPropPres) cout << "\t\tAdding requirement for " << currPrec << " " << *(RPGBuilder::getLiteral(currPrec)) << " by " << pcostItr->achiever << " at time " << pcostItr->atLayer << " with weight " << galItr->second.second << "\n";
                                        }
                                    } else {
                                        if (extractDebug || debugLPPropPres) cout << "\t\tPrecondition " << currPrec << " in initial state\n";
                                    }
                
                                }
                
                                if (metricrpg.get()) {
                                    list<int> & actionPreconditionlist = (*actionsToProcessedStartNumericPreconditions)[currAchievedBy->act];
                                    list<int>::iterator aplItr = actionPreconditionlist.begin();
                                    const list<int>::iterator aplEnd = actionPreconditionlist.end();
                    
                                    if (extractDebug) cout << "\t\tNumeric preconditions:\n";
                
                                    for (; aplItr != aplEnd; ++aplItr) {
                                        
                                        const int currPrec = (*aplItr);
                                        if (numericsFreeFor.find(currPrec) == numericsFreeFor.end()) {
                                            const double acIn = (*numericAchievedInLayer)[currPrec];
                                            if (acIn > 0.0) {
                                                map<int, double>::iterator galItr = goalsAtLayer[acIn].numericGoals.insert(pair<int, double>(currPrec, tilR)).first;
                                                if (galItr->second < tilR) galItr->second = tilR;
                                                if (extractDebug) cout << "\t\tAdding requirement for numeric precondition " << currPrec << " at time " << acIn << " with weight " << galItr->second << "\n";
                                            } else {
                                                if (extractDebug) cout << "\t\tNumeric precondition " << currPrec << " satisfied in initial state\n";
                                            }
                                        } else {
                                            if (extractDebug) cout << "\t\tNumeric precondition " << currPrec << " is free\n";
                                        }
                                    }
                    
                                }
                                if (extractDebug) cout << "\t\tPreconditions done.\n";
                                                                                                    
                            }
                            
                            {
                                addExtraPreconditionsForPreferences(currAchievedBy->act, currTS, kNeeded, propsSatisfied, goalsAtLayer);
                
                                if (extractDebug) cout << "\t\tPreference preconditions done.\n";
                            }
            
                        }
                        currAchievedBy = currAchievedBy->next;
                    }
                }

                if (extractDebug) cout << "All goals at this TS now satisfied\n";
            }
            goalsAtLayer.erase(currTS);
        }


        if (!previousRelaxedPlan.empty()) {
            list<pair<double, list<ActionAndHowManyTimes > > >::iterator oldItr = previousRelaxedPlan.begin();
            const list<pair<double, list<ActionAndHowManyTimes > > >::iterator oldEnd = previousRelaxedPlan.end();
            
            list<pair<double, list<ActionAndHowManyTimes > > >::iterator newItr = relaxedPlan.begin();
            const list<pair<double, list<ActionAndHowManyTimes > > >::iterator newEnd = relaxedPlan.end();
            
            while (oldItr != oldEnd && newItr != newEnd) {
                if ((oldItr->first - newItr->first) < -0.0005) {
                    relaxedPlan.insert(newItr, *oldItr);
                    ++oldItr;
                } else if ((newItr->first < oldItr->first) < -0.0005) {
                    ++newItr;
                } else {
                    newItr->second.insert(newItr->second.end(), oldItr->second.begin(), oldItr->second.end());
                    ++oldItr;
                    ++newItr;
                }
            }
            
            for (; oldItr != oldEnd; ++oldItr) {
                relaxedPlan.push_back(*oldItr);
            }
            
            previousRelaxedPlan.clear();
        }

        if (h > 0 && helpfulActions.empty()) {
            
            static const bool patchDebug = false;
            if (patchDebug) {
                cout << "Looping solution extraction to patch relaxed ordering\n";
            }
            // patch up actions missing due to relaxation of ordering in the LP
                
            set<int> missingPropositions;
            set<int> missingNumerics;
            
            vector<double> accruedFluents(*maxFluentTable);
            set<int> accruedFacts(theState.first);
            
            list<pair<double, list<ActionAndHowManyTimes > > >::iterator rpItr = relaxedPlan.begin();
            const list<pair<double, list<ActionAndHowManyTimes > > >::iterator rpEnd = relaxedPlan.end();
            
            int actID;
            int fID;
            for (; rpItr != rpEnd; ++rpItr) {
                list<ActionAndHowManyTimes >::iterator rlItr = rpItr->second.begin();
                const list<ActionAndHowManyTimes >::iterator rlEnd = rpItr->second.end();
                
                for (; rlItr != rlEnd; ++rlItr) {
                    actID = rlItr->first->getID();
                    
                    {
                        list<Literal*> & actionPreconditionlist = (*actionsToProcessedStartPreconditions)[actID];
                        list<Literal*>::iterator aplItr = actionPreconditionlist.begin();
                        const list<Literal*>::iterator aplEnd = actionPreconditionlist.end();
                        
                        for (; aplItr != aplEnd; ++aplItr) {
                            fID = (*aplItr)->getStateID();
                            if (accruedFacts.find(fID) == accruedFacts.end()) {
                                if (patchDebug) {
                                    cout << "- Missing precondition fact" << *(*aplItr) << endl;
                                }
                                missingPropositions.insert(fID);                                
                            }                            
                        }
                    }
                    
                    {
                        list<int> & actionPreconditionlist = (*actionsToProcessedStartNumericPreconditions)[actID];
                        list<int>::iterator aplItr = actionPreconditionlist.begin();
                        const list<int>::iterator aplEnd = actionPreconditionlist.end();
                        for (; aplItr != aplEnd; ++aplItr) {
                            fID = (*aplItr);
                            const RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPreTable()[fID];
                            
                            if (!currPre.isSatisfied(accruedFluents)) {
                                if (patchDebug) {
                                    cout << "- Missing numeric precondition fact " << fID << " - " << currPre << endl;
                                }                                
                                missingNumerics.insert(fID);
                            }
                        }                        
                    }
                    
                    {
                        list<Literal*> & actionEffectsList = (*actionsToStartEffects)[actID];
                        list<Literal*>::iterator aelItr = actionEffectsList.begin();
                        const list<Literal*>::iterator aelEnd = actionEffectsList.end();
                        
                        for (; aelItr != aelEnd; ++aelItr) {
                            accruedFacts.insert((*aelItr)->getStateID());
                        }                                                
                    }
                    
                    {
                        list<int> & actionEffectsList = (*actionsToRPGNumericStartEffects)[actID];
                        list<int>::iterator aelItr = actionEffectsList.begin();
                        const list<int>::iterator aelEnd = actionEffectsList.end();
                        
                        for (; aelItr != aelEnd; ++aelItr) {
                            RPGBuilder::RPGNumericEffect & currEff = RPGBuilder::getNumericEff()[*aelItr];
                            if (currEff.isAssignment) {
                                if (currEff.constant > 0.0) {
                                    if (accruedFluents[currEff.fluentIndex] < currEff.constant) {
                                        accruedFluents[currEff.fluentIndex] = currEff.constant;
                                    }
                                } else {
                                    if (accruedFluents[currEff.fluentIndex+vCount] < -currEff.constant) {
                                        accruedFluents[currEff.fluentIndex+vCount] = -currEff.constant;
                                    }
                                }
                            } else {
                                if (currEff.constant > 0.0) {
                                    accruedFluents[currEff.fluentIndex] += rlItr->howManyTimes * currEff.constant;
                                } else {
                                    accruedFluents[currEff.fluentIndex+vCount] -= rlItr->howManyTimes * currEff.constant;
                                }
                            }
                        }
                    }
                }
            }

            assert(!missingPropositions.empty() || !missingNumerics.empty());

            set<int>::iterator pItr = missingPropositions.begin();
            const set<int>::iterator pEnd = missingPropositions.end();
            
            for (; pItr != pEnd; ++pItr) {
                const int currGoal = *pItr;
                const double insLayer = (*achievedInLayer)[currGoal];
                assert(insLayer > 0.0);
                if (patchDebug) {
                    cout << "- Must achieve fact "<< *(RPGBuilder::getLiteral(*pItr)) << " at layer " << insLayer << endl;
                }
                goalsAtLayer[insLayer].propositionalGoals.insert(make_pair(currGoal, make_pair((*achievedBy)[*pItr].first,1.0)));                
            }
            
            set<int>::iterator nItr = missingNumerics.begin();
            const set<int>::iterator nEnd = missingNumerics.end();
            
            for (; nItr != nEnd; ++nItr) {
                if (patchDebug) {
                    cout << "- Must achieve " << RPGBuilder::getNumericPreTable()[*nItr];
                    cout.flush();
                    assert(numericAchievedInLayer->size() > *nItr);
                    assert(*nItr >= 0);
                }
                
                const double insLayer = (*numericAchievedInLayer)[*nItr];
                if (patchDebug) {
                    cout << " at layer " << insLayer << endl;
                }

                assert(insLayer > 0.0);
                goalsAtLayer[insLayer].numericGoals.insert(make_pair(*nItr, 1.0));
            }
            
            carryOn = true;
            previousRelaxedPlan.swap(relaxedPlan);
            extractDebug = (extractDebug || patchDebug);
            propsSatisfied.clear();
        }                                                        
    }
    
	
	double newGCost = GCost;
    
	if (lpGoalPrefViolation > 0.0001) {
        
        if (prefDebug) {
            cout << "Minimum violation cost to goal " << lpGoalPrefViolation << endl;
        }
        
        list<int> punreachable;
        
        const int pCount = optimisticPrefStatus.size();
        
        for (int p = 0; p < pCount; ++p) {
            if (!canBeSatisfied(theState.preferenceStatus[p])) continue;
            if (!isSatisfied(optimisticPrefStatus[p])) {
                punreachable.push_back(p);
                if (prefDebug) {
                    cout << "Preference " << RPGBuilder::getPreferences()[p].name << " (" << p << ") was never reached\n";
                    
                    NNF_Flat * const f = (*unsatisfiedPreferenceConditions)[p][0];
                    assert(!f->isSatisfied());
                    
                    const NNF_Flat::Cell * const cells = f->getCells();
                    const bool * cellIsAnd = f->cellIsAnAnd();
                    const int cellCount = f->getCellCount();
                    const int * parentIDs = f->getParentIDs();
                    
                    if (cellIsAnd[0]) {
                        bool oneWasFalse = false;
                        
                        for (int cc = 0; cc < cellCount; ++cc) {
                            if (!cells[cc].isCell()) continue;
                            if (parentIDs[cc] != 0) continue;
                            
                            if (cells[cc].lit) {
                                const int fID = cells[cc].lit->getStateID();
                                if (cells[cc].polarity) {
                                    if ((*achievedInLayer)[fID] < 0.0) {
                                        oneWasFalse = true;
                                        cout << " - Don't have supporting fact " << *(cells[cc].lit) << endl;
                                    } else {
                                        cout << " + Have supporting fact " << *(cells[cc].lit) << endl;
                                    }
                                } else {
                                    if ((*negativeAchievedInLayer)[fID] < 0.0) {
                                        oneWasFalse = true;
                                        cout << " - Don't have supporting fact ¬" << *(cells[cc].lit) << endl;
                                    } else {
                                        cout << " + Have supporting fact ¬" << *(cells[cc].lit) << endl;
                                    }
                                }
                            } else {
                                const int fID = cells[cc].num;
                                if (cells[cc].polarity) {                                    
                                    if ((*numericAchievedInLayer)[fID] < 0.0) {
                                        oneWasFalse = true;
                                        cout << " - Don't have supporting numeric " << RPGBuilder::getNumericPrecs()[fID] << endl;
                                    }
                                } else {
                                    if ((*negativeNumericAchievedInLayer)[fID] < 0.0) {
                                        oneWasFalse = true;
                                        cout << " - Don't have supporting numeric ¬" << RPGBuilder::getNumericPrecs()[fID] << endl;
                                    }
                                }
                            }
                        }
                        
                        assert(oneWasFalse);
                        
                    }
                    
                    
                }
            }
        }
        
        if (milprpg.get()) {        
            milprpg->getUnreachablePrefs(punreachable, optimisticPrefStatus);            
        }
        
        if (!punreachable.empty()) {
            const double delta = PreferenceHandler::markUnreachables(theState, punreachable);
            if (prefDebug) {
                cout << "- Turning g/h values for preference from " << newGCost << "," << lpGoalPrefViolation << " to ";
            }
            newGCost += delta;
            lpGoalPrefViolation -= delta;
            if (prefDebug) {
                cout << newGCost << "," << lpGoalPrefViolation << endl;
            }
        }
                            
    }
	
	
	list<double> haWeights;


    

	if (false|| (h > 0 && helpfulActions.empty()) || GlobalSchedule::globalVerbosity & 1048576) {
		cout << "Relaxed plan for state:\n";
		list<pair<double, list<ActionAndHowManyTimes > > >::iterator rpItr = relaxedPlan.begin();
        const list<pair<double, list<ActionAndHowManyTimes > > >::iterator rpEnd = relaxedPlan.end();

		for (; rpItr != rpEnd; ++rpItr) {
            if (rpItr->first == DBL_MAX) {
                cout << "\t last:\n";
            } else {
                cout << "\t" << rpItr->first << ":\n";
            }
            list<ActionAndHowManyTimes >::iterator rlItr = rpItr->second.begin();
            const list<ActionAndHowManyTimes >::iterator rlEnd = rpItr->second.end();

			for (; rlItr != rlEnd; ++rlItr) {
                const int aid = rlItr->first->getID();
                cout << "\t\t" << *(rlItr->first) <<  ", applicable at " << startActionSchedule[aid] << "\n";
                
                {
                    list<Literal*>::const_iterator fpItr = RPGBuilder::getProcessedStartPreconditions()[aid].begin();
                    const list<Literal*>::const_iterator fpEnd = RPGBuilder::getProcessedStartPreconditions()[aid].end();
                    
                    for (; fpItr != fpEnd; ++fpItr) {
                        cout << "\t\t\tNeeds " << *(*fpItr) << ", which appeared at layer " << (*achievedInLayer)[(*fpItr)->getStateID()] << "\n";
                    }
                }
                {
                    list<int>::const_iterator fpItr = (*actionsToProcessedStartNumericPreconditions)[aid].begin();
                    const list<int>::const_iterator fpEnd = (*actionsToProcessedStartNumericPreconditions)[aid].end();
                    
                    for (; fpItr != fpEnd; ++fpItr) {
                        cout << "\t\t\tNeeds ";
                        
                        (*rpgNumericPreconditions)[*fpItr].display(cout);
                        
                        cout << ", which appeared at layer "<< (*numericAchievedInLayer)[*fpItr] << "\n";
                    }
                }
                
			}
			

		}
	}

	{


		map<int, pair<bool, bool> > seenAlready;

		list<pair<int, VAL::time_spec> >::iterator haItr = helpfulActions.begin();
		const list<pair<int, VAL::time_spec> >::iterator haEnd = helpfulActions.end();

		while (haItr != haEnd) {
			pair<bool,bool> & insCheck = seenAlready.insert(pair<int, pair<bool, bool> >(haItr->first, pair<bool,bool>(false,false))).first->second;
			if (haItr->second == VAL::E_AT_START) {
				if (!insCheck.first) {
					insCheck.first = true;
					++haItr;
				} else {
					list<pair<int, VAL::time_spec> >::iterator dupli = haItr++;
					helpfulActions.erase(dupli);
				}
			} else {
				if (!insCheck.second) {
					insCheck.second = true;
					++haItr;
				} else {
					list<pair<int, VAL::time_spec> >::iterator dupli = haItr++;
					helpfulActions.erase(dupli);
				}
			}
		}
		


	}

    #ifndef NDEBUG
    if (h > 0) {
        if (helpfulActions.empty()) {
            cout << "\nEmpty helpful actions for state:\n";
            cout << " << " << h << ", " << lpGoalPrefViolation << " + " << newGCost << " < " << currentPreferenceCost << " >> "; cout.flush();
            assert(!helpfulActions.empty());
        }
    }
    #endif

	if (evaluateDebug || GlobalSchedule::globalVerbosity & 1048576) {

		cout << "Helpful actions:\n";

		list<pair<int, VAL::time_spec> >::iterator haItr = helpfulActions.begin();
		const list<pair<int, VAL::time_spec> >::iterator haEnd = helpfulActions.end();

		for (; haItr != haEnd; ++haItr) {
			if (haItr->second == VAL::E_AT) {
				cout << "Timed initial literal action " << haItr->first << "\n";
			} else {
				cout << *(RPGBuilder::getInstantiatedOp(haItr->first)) << ", " << ((haItr->second == VAL::E_AT_START) ? "start" : "end") << "\n";
			}
		}


	}

    if (GlobalSchedule::globalVerbosity & 1) {

        cout << " << " ;
        if (realGoalsSatisfied) {
            cout << "(G)";
        }
        cout << h << ", " << lpGoalPrefViolation << " + " << newGCost << " < " << currentPreferenceCost << " >> "; cout.flush();
        
    }

	return EvaluationInfo(h, lpGoalPrefViolation, realGoalsSatisfied);

};


void SubproblemRPG::addExtraPreconditionsForPreferences(const int & act, const double & currTS, const double & tilR,
                                                         const map<int,double> & propsSatisfied,
                                                         map<double, RPGRegress, EpsilonComp> & goalsAtLayer)
{
    static const bool extractDebug = (GlobalSchedule::globalVerbosity & 32768);
    map<int, list<pair<int,bool> > >::const_iterator extras = preferencePartsToSatisfyBeforeAction.find(act);
    if (extras != preferencePartsToSatisfyBeforeAction.end()) {
        {
            list<pair<int,bool> >::const_iterator prefPItr = extras->second.begin();
            const list<pair<int,bool> >::const_iterator prefPEnd = extras->second.end();
            for (; prefPItr != prefPEnd; ++prefPItr) {
                
                const double & trueInLayer = (*preferencePartBecameTrueInLayer)[prefPItr->first][prefPItr->second ? 1 : 0];
                
                if (trueInLayer >= currTS) {
                    if (extractDebug) cout << COLOUR_yellow << "\t\tToo early to manage to satisfy preference " << prefPItr->first << COLOUR_default << endl;
                    continue;
                } else {
                    if (extractDebug) cout << COLOUR_yellow << "\t\tPreference " << prefPItr->first << " was satisfied in layer " << trueInLayer << " - adding preconditions" << COLOUR_default << endl;
                }
                
                
                list<Literal*> extraPreconditionList;
                list<int> extraNumericPreconditionList;
                
                if (RPGBuilder::useMetricRPG) {
                    PreferenceHandler::getPreconditionsToSatisfy(extraPreconditionList, &extraNumericPreconditionList, *prefPItr, preferenceViolationCostOfUsingFact, numericAchievedInLayer);
                } else {                
                    PreferenceHandler::getPreconditionsToSatisfy(extraPreconditionList, 0, *prefPItr, preferenceViolationCostOfUsingFact, numericAchievedInLayer);
                }
                
                {
                    list<Literal*>::const_iterator currPItr = extraPreconditionList.begin();
                    const list<Literal*>::const_iterator currPEnd = extraPreconditionList.end();
                    for (; currPItr != currPEnd; ++currPItr) {
                        
                        const int currPrec = (*currPItr)->getStateID();
                        const double acIn = (*achievedInLayer)[currPrec];
                        
                        if (acIn > 0.0) {                                        
                            if (!LandmarksAnalysis::factIsALandmark(currPrec) || propsSatisfied.find(currPrec) == propsSatisfied.end()) {                                        
                                
                                list<PreferenceSetAndCost>::reverse_iterator pcostItr = (*preferenceViolationCostOfUsingFact)[currPrec].rbegin();
                                while (pcostItr->atLayer >= currTS) {
                                    ++pcostItr;
                                }
                                                                                            
                                
                                map<int, pair<int, double> >::iterator galItr = goalsAtLayer[pcostItr->atLayer].propositionalGoals.insert(make_pair(currPrec, make_pair(pcostItr->achiever,tilR))).first;
                                if (galItr->second.second < tilR) galItr->second.second = tilR;

                                if (extractDebug) cout << "\t\tAdding requirement for preference fact " << currPrec << " at time " << pcostItr->atLayer << " by " << pcostItr->achiever << " with weight " << galItr->second.second << "\n";
                            }
                        } else {
                            if (extractDebug) cout << "\t\tPreference precondition " << currPrec << " in initial state\n";
                        }
                                                                            
                    }
                }
                
                {
                    list<int>::const_iterator currNItr = extraNumericPreconditionList.begin();
                    const list<int>::const_iterator currNEnd = extraNumericPreconditionList.end();
                    
                    for (; currNItr != currNEnd; ++currNItr) {
                        const double acIn = (*numericAchievedInLayer)[*currNItr];
                        
                        if (acIn > 0.0) {                                        
                            map<int, double>::iterator galItr = goalsAtLayer[acIn].numericGoals.insert(make_pair(*currNItr, tilR)).first;
                            if (galItr->second < tilR) galItr->second = tilR;
                            
                        } else {
                            if (extractDebug) cout << "\t\tPreference precondition " << RPGBuilder::getNumericPreTable()[*currNItr] << " in initial state\n";
                        }
                    }
                }
            }
        }
    }
}

double SubproblemRPG::shiftFactLayersByLag(const double & currTS, const double & lag, map<double, FactLayerEntry, EpsilonComp > & factLayer, map<double, vector<double>, EpsilonComp > & fluentLayers, const bool excludeFirstLayer)
{
    map<double, FactLayerEntry, EpsilonComp > ofal;
    //map<double, vector<double>, EpsilonComp > oflul;
    
    #ifndef NDEBUG
        
    const uint aSize = factLayer.size();
    //const int bSize = fluentLayers.size();
    #endif
                    
    
    assert(!RPGBuilder::getPreferences().empty() || (fabs(lag - currTS - 0.001) < 0.00001));
    
    factLayer.swap(ofal);
    //fluentLayers.swap(oflul);
    
    if (!ofal.empty() && (!excludeFirstLayer || ofal.size() > 1)) {
        
        double fReal;
        
        if (excludeFirstLayer) {
            map<double, FactLayerEntry, EpsilonComp >::iterator sItr = ofal.begin();
            //cout << "{" << sItr->first;
            ++sItr;
            fReal = sItr->first;
            //cout << "," << sItr->first << "}\n";
        } else {
            fReal = ofal.begin()->first;

        }
        const double offset = lag - fReal;
                        
        if (fabs(offset) < 0.000001) {
            factLayer.swap(ofal);
        } else {

            /*if (excludeFirstLayer) {
                cout << ofal.begin()->first << ",";
            }*/
            //cout << fReal << " to " << lag << ", so shifting by " << offset << endl;
            
            assert(offset > -0.0000000001);
            
            map<double, FactLayerEntry, EpsilonComp >::const_iterator oItr = ofal.begin();
            const map<double, FactLayerEntry, EpsilonComp >::const_iterator oEnd = ofal.end();
            
            map<double, FactLayerEntry, EpsilonComp >::iterator insItr = factLayer.end();
            
            for (bool exc = excludeFirstLayer; oItr != oEnd; ++oItr) {
                if (exc) {
                    exc = false;
                    insItr = factLayer.insert(insItr, make_pair(oItr->first, oItr->second));
                } else {
                    insItr = factLayer.insert(insItr, make_pair(offset + oItr->first, oItr->second));
                }
            }
        }
    } else {
        factLayer.swap(ofal);
    }

    /*
    if (!oflul.empty() && (!excludeFirstLayer || oflul.size() > 1)) {
        
        
        double fReal;
        if (excludeFirstLayer) {
            map<double, vector<double>, EpsilonComp >::iterator sItr = oflul.begin();
            ++sItr;
            fReal = sItr->first;
        } else {
            fReal = oflul.begin()->first;
        }
        
        const double offset = lag - fReal;
        
        if (fabs(offset) < 0.00001) {
            fluentLayers.swap(oflul);
        } else {
            map<double, vector<double>, EpsilonComp >::const_iterator oItr = oflul.begin();
            const map<double, vector<double>, EpsilonComp >::const_iterator oEnd = oflul.end();
            
            map<double, vector<double>, EpsilonComp >::iterator insItr = fluentLayers.end();
            
            for (bool exc = excludeFirstLayer; oItr != oEnd; ++oItr) {
                if (exc) {
                    exc = false;
                    insItr = fluentLayers.insert(insItr, make_pair(oItr->first, oItr->second));
                } else {
                    insItr = fluentLayers.insert(insItr, make_pair(offset + oItr->first, oItr->second));
                }
            }
        }
    } else {
        fluentLayers.swap(oflul);
    }
    */
    #ifndef NDEBUG
         
    assert(aSize == factLayer.size());
    //assert(bSize == fluentLayers.size());
    #endif

    return lag;
                
}

void SubproblemRPG::processAnyCheaperFacts(const double & nlTime, double & maxLag)
{
    
    map<int,double>::const_iterator recost = reducedCostFacts.begin();
    const map<int,double>::const_iterator recostEnd = reducedCostFacts.end();
    
    set<int> actsVisited;
    list<int> actsToVisit;
    
    for (; recost != recostEnd; ++recost) {
        assert(!(*preferenceViolationCostOfUsingFact)[recost->first].empty());
        PreferenceSetAndCost & prefCost = (*preferenceViolationCostOfUsingFact)[recost->first].back();
        const int currAct = prefCost.achiever;
        
        assert(currAct >= 0);
        assert((uint) currAct < actionsToStartEffects->size());
        
        actsVisited.insert(currAct);
        ActionViolationData & newCost = (*preferenceViolationCostOfUsingAction)[currAct];
        
        list<Literal*> & addEffects = (*actionsToStartEffects)[currAct];
        
        list<Literal*>::iterator addEffItr = addEffects.begin();
        const list<Literal*>::iterator addEffEnd = addEffects.end();
        
        for (; addEffItr != addEffEnd; ++addEffItr) {
            const int currEff = (*addEffItr)->getStateID();
            visitFactThatIsNowCheaper(currAct, currEff, newCost, nlTime, actsVisited, actsToVisit);
        }                                
    }
    
    double lagT = 0.0;
    bool any = false;
    for (int lag = 0; !actsToVisit.empty(); ++lag) {
        
        list<int> oldActsToVisit;
        oldActsToVisit.swap(actsToVisit);
        lagT = nlTime + (lag * 0.001);
        
        list<int>::const_iterator oaItr = oldActsToVisit.begin();
        const list<int>::const_iterator oaEnd = oldActsToVisit.end();
        
        for (; oaItr != oaEnd; ++oaItr) {
            const int currAct = *oaItr;
            any = true;
            const ActionViolationData & newCost = (*preferenceViolationCostOfUsingAction)[currAct];
            
            list<Literal*> & addEffects = (*actionsToStartEffects)[currAct];
            
            list<Literal*>::iterator addEffItr = addEffects.begin();
            const list<Literal*>::iterator addEffEnd = addEffects.end();
            
            for (; addEffItr != addEffEnd; ++addEffItr) {
                const int currEff = (*addEffItr)->getStateID();
                visitFactThatIsNowCheaper(currAct, currEff, newCost, lagT, actsVisited, actsToVisit);                
            }
        }
    }
    lagT += 0.001;
    
    if (lagT > maxLag) {
        maxLag = lagT;
        //cout << "After updating for cheaper facts, setting lag to " << maxLag << endl;
        
    }
    
    reducedCostFacts.clear();
}

void SubproblemRPG::importNowAffordableFact(map<double, FactLayerEntry, EpsilonComp > & factLayers,
                                             const set<int> & goals, int & unsatisfiedGoals,
                                             const bool skipFirstLayer)
{
    if (factsThatAreNowAffordable.empty() && negativeFactsThatAreNowAffordable.empty()) return;
    
    static const bool debug = (GlobalSchedule::globalVerbosity & 32768);
    
    map<double, FactLayerEntry, EpsilonComp >::iterator addTo = factLayers.begin();               
    
    if (addTo == factLayers.end()) {
        addTo = factLayers.insert(make_pair(0.001, FactLayerEntry())).first;
    } else {        
        if (skipFirstLayer) {        
            ++addTo;
            if (addTo == factLayers.end()) {            
                addTo = factLayers.begin();
                const double newTS = addTo->first + 0.001;
                
                addTo = factLayers.insert(make_pair(newTS, FactLayerEntry())).first;
            }
        }
    }
    
    {
        map<int, pair<int,double> >::const_iterator nfItr = factsThatAreNowAffordable.begin();
        const map<int, pair<int,double> >::const_iterator nfEnd = factsThatAreNowAffordable.end();
        
        for (; nfItr != nfEnd; ++nfItr) {
            if (debug) {
                cout << "Adding now-affordable fact " << *(RPGBuilder::getLiteral(nfItr->first)) << " to the next action layer\n";                
                if ((*achievedInLayer)[nfItr->first] >= 0.0) {
                    cout << COLOUR_light_red << "\tWas previously achieved at layer " << (*achievedInLayer)[nfItr->first] << COLOUR_default << endl;
                }
            }
            assert((*achievedInLayer)[nfItr->first] < 0.0);
            (*achievedInLayer)[nfItr->first] = addTo->first;
            (*achievedBy)[nfItr->first] = pair<int, VAL::time_spec>(nfItr->second.first, VAL::E_AT_START);
            
            addTo->second.first.push_back(nfItr->first);
            
            ActionViolationData & costData = (*preferenceViolationCostOfUsingAction)[nfItr->second.first];
            
            list<PreferenceSetAndCost> & prefDataList = (*preferenceViolationCostOfUsingFact)[nfItr->first];
            
            prefDataList.push_back(PreferenceSetAndCost(nfItr->second.first, addTo->first,
                                                        costData.overallViolationCost, costData.overallViolations));
                                                        
            if (goals.find(nfItr->first) != goals.end()) {
                --unsatisfiedGoals;
            }
                                                                                                    
        }
    }    
    
    {
        map<int, pair<int,double> >::const_iterator nfItr = negativeFactsThatAreNowAffordable.begin();
        const map<int, pair<int,double> >::const_iterator nfEnd = negativeFactsThatAreNowAffordable.end();
        
        for (; nfItr != nfEnd; ++nfItr) {
            double & ail = (*negativeAchievedInLayer)[nfItr->first];
            
            if (ail >= 0.0) continue;
                        
                                    ail = addTo->first;
            (*negativeAchievedBy)[nfItr->first] = pair<int, VAL::time_spec>(nfItr->second.first, VAL::E_AT_START);
            
            addTo->second.negativeLiterals.push_back(nfItr->first);
            
            
        }
    }
                    
    factsThatAreNowAffordable.clear();
    negativeFactsThatAreNowAffordable.clear();
                                    
}

void SubproblemRPG::visitFactThatIsNowCheaper(const int & currAct, const int & currEff,
                                              const ActionViolationData & newCost, const double & factLayerTime,
                                              set<int> & actsVisited, list<int> & actsToVisit)
{
    
    const bool debug = (GlobalSchedule::globalVerbosity & 32768);
    
    double & firstAIL = (*achievedInLayer)[currEff];
    
    if (firstAIL == -1.0) {
        pair<int,double> & fCost = factsThatAreNowAffordable.insert(make_pair(currEff, make_pair(currAct, newCost.overallViolationCost))).first->second;
        if (fCost.second > newCost.overallViolationCost) {
            fCost.first = currAct;
            fCost.second = newCost.overallViolationCost;
        } else {
            if (fCost.first == currAct) {
                if (debug) {
                    cout << "\t\t -- Is new - putting on the list of facts to put at the next layer\n";
                }                                
            } else {
                if (debug) {
                    cout << "\t\t -- Is already on the list of facts to put at the next layer\n";
                }
            }
            
        }
        return;
    }
    
    list<PreferenceSetAndCost> & prefCost =  (*preferenceViolationCostOfUsingFact)[currEff];
    
    if (debug) {
        cout << "\t\t\tWas first achieved ";
        
        const int oaa = prefCost.back().achiever;
        if (oaa == -1) {
            cout << "in the initial state";
        } else {
            cout <<  "by ";
            cout.flush();
            cout << prefCost.back().achiever;
            cout.flush();            
            cout << *(RPGBuilder::getInstantiatedOp(prefCost.back().achiever));
        }
        cout << " in layer " << firstAIL << " with cost " << prefCost.back().cost << endl;
    }
    
    if (prefCost.back().cost > newCost.overallViolationCost) {
        
        if (firstAIL == factLayerTime) {
            (*achievedBy)[currEff] = make_pair(currAct, VAL::E_AT_START);
            prefCost.back().cost = newCost.overallViolationCost;
            prefCost.back().needsToViolate = newCost.overallViolations;
            
            if (debug) {
                cout << "\t\t\t\tNow achieved, no later, at " << factLayerTime << " with cost " << prefCost.back().cost;
                if (!prefCost.back().needsToViolate.empty()) {
                    cout << ", violating:";
                    set<int>::const_iterator vi = prefCost.back().needsToViolate.begin();
                    const set<int>::const_iterator ve = prefCost.back().needsToViolate.end();
                    
                    for (; vi != ve; ++vi) {
                        cout << " " << *vi;
                    }
                    
                }
                cout << endl;
            }
            
        } else {            
            prefCost.push_back(PreferenceSetAndCost(currAct, factLayerTime, newCost.overallViolationCost, newCost.overallViolations));
            if (debug) {
                cout << "\t\t\t\tNow achieved at " << factLayerTime << " with cost " << prefCost.back().cost;
                if (!prefCost.back().needsToViolate.empty()) {
                    cout << ", violating:";
                    set<int>::const_iterator vi = prefCost.back().needsToViolate.begin();
                    const set<int>::const_iterator ve = prefCost.back().needsToViolate.end();
                    
                    for (; vi != ve; ++vi) {
                        cout << " " << *vi;
                    }
                                        
                }
                cout << endl;
            }
        }
        
        const list<pair<int, VAL::time_spec> > & dependents = (*processedPreconditionsToActions)[currEff];
        list<pair<int, VAL::time_spec> >::const_iterator depItr = dependents.begin();
        const list<pair<int, VAL::time_spec> >::const_iterator depEnd = dependents.end();
        
        for (; depItr != depEnd; ++depItr) {
            assert(depItr->second == VAL::E_AT_START);
            if (actsVisited.find(depItr->first) != actsVisited.end()) continue;
            if (!(*preferenceViolationCostOfUsingAction)[depItr->first].canBeApplied) continue;
            if (debug) cout << "\t\t\tUsed by " << *(RPGBuilder::getInstantiatedOp(depItr->first)) << endl;
            
            set<int> precViolations;
            double newActPrecCost = 0.0;
            
            {
                list<Literal*> & actionPreconditionlist = (*actionsToProcessedStartPreconditions)[depItr->first];
                list<Literal*>::iterator aplItr = actionPreconditionlist.begin();
                const list<Literal*>::iterator aplEnd = actionPreconditionlist.end();
                
                for (; aplItr != aplEnd; ++aplItr) {
                    list<PreferenceSetAndCost> & currPreData = (*preferenceViolationCostOfUsingFact)[(*aplItr)->getStateID()];
                    set<int>::const_iterator viItr = currPreData.back().needsToViolate.begin();
                    const set<int>::const_iterator viEnd = currPreData.back().needsToViolate.end();
                    
                    for (; viItr != viEnd; ++viItr) {
                        if (precViolations.insert(*viItr).second) {
                            if (debug) cout << "\t\t\t\tViolates preference " << *viItr << " due to precondition " << *(*aplItr) << endl;
                            newActPrecCost += prefCosts[*viItr];
                        }
                    }
                }
            }
            
            {
                const vector<int> & actPrefs = RPGBuilder::getStartPreferences()[depItr->first];
                
                const int ppCount = actPrefs.size();
                
                for (int p = 0; p < ppCount; ++p) {
                    const NNF_Flat* const f = (*unsatisfiedPreferenceConditions)[actPrefs[p]][0];        
                    
                    if (f ? !f->isSatisfied() : RPGBuilder::getPreferences()[actPrefs[p]].neverTrue) {
                        if (precViolations.insert(actPrefs[p]).second) {
                            if (debug) {
                                cout << "Applying " << *(RPGBuilder::getInstantiatedOp(currAct)) << " would mean violating precondition pref " << RPGBuilder::getPreferences()[actPrefs[p]].name << ":" << actPrefs[p] << ", as it is not currently satisfied - ";
                                cout << "cost of violation = " << prefCosts[actPrefs[p]] << endl;
                            }
                            newActPrecCost += prefCosts[actPrefs[p]];
                        }
                    }
                }
            }
            
            
            if (debug) cout << "\t\t\t- Cost of action's preconditions is " << newActPrecCost << endl;
            
            ActionViolationData & prevActCost = (*preferenceViolationCostOfUsingAction)[depItr->first];
            
            set<int> extrasDueToEffects;
            double extrasDueToEffectsCost = 0.0;
            
            std::set_difference(prevActCost.effectsMeanViolating.begin(), prevActCost.effectsMeanViolating.end(),
                                precViolations.begin(), precViolations.end(),
                                insert_iterator<set<int> >(extrasDueToEffects, extrasDueToEffects.begin()));
            
            set<int>::const_iterator exeItr = extrasDueToEffects.begin();
            const set<int>::const_iterator exeEnd = extrasDueToEffects.end();
            
            for (; exeItr != exeEnd; ++exeItr) {
                extrasDueToEffectsCost += prefCosts[*exeItr];
            }
            
            if (newActPrecCost + extrasDueToEffectsCost < prevActCost.overallViolationCost) {
                prevActCost.preconditionsMeanViolating.swap(precViolations);
                prevActCost.precViolationCost = newActPrecCost;
                prevActCost.effectsMeanViolating.swap(extrasDueToEffects);
                prevActCost.effViolationCost = extrasDueToEffectsCost;
                prevActCost.overallViolationCost = newActPrecCost + extrasDueToEffectsCost;
                prevActCost.overallViolations = prevActCost.preconditionsMeanViolating;
                prevActCost.overallViolations.insert(prevActCost.effectsMeanViolating.begin(), prevActCost.effectsMeanViolating.end());

                bool goAhead = !actionPreviouslyTooExpensive[depItr->first];
                
                if (!goAhead && prevActCost.overallViolationCost <= maxPermissibleActionCost ) {
                    actionPreviouslyTooExpensive[depItr->first] = false;
                    
                    list<Literal*> & delEffects = (*actionsToStartNegativeEffects)[depItr->first];
                    
                    list<Literal*>::iterator delEffItr = delEffects.begin();
                    const list<Literal*>::iterator delEffEnd = delEffects.end();
                    
                    for (; delEffItr != delEffEnd; ++delEffItr) {
                        const int currEff = (*delEffItr)->getStateID();
                        double & currAIL = (*negativeAchievedInLayer)[currEff];
                        if (currAIL == -1.0) {
                            negativeFactsThatAreNowAffordable.insert(make_pair(currEff, make_pair(depItr->first, prevActCost.overallViolationCost)));
                        }
                    }
                    goAhead = true;
                }
                
                if (goAhead) {
                    if (actsVisited.insert(depItr->first).second) {
                        actsToVisit.push_back(depItr->first);
                    }
                }
            }
            
        }
    }

}

void SubproblemRPG::setInitialNegativePreconditionsOfPreferences(MinimalState & startState)
{
    {
        set<int>::const_iterator fItr = startState.first.begin();
        const set<int>::const_iterator fEnd = startState.first.end();
        
        for (; fItr != fEnd; ++fItr) {
            const list<LiteralCellDependency<pair<int,bool> > > & dependents = (*negativePreconditionsToPrefs)[*fItr];
            list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = dependents.begin();
            const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = dependents.end();
            
            for (; depItr != depEnd; ++depItr) {
                
                NNF_Flat* const toUpdate = (*unsatisfiedPreferenceConditions)[depItr->dest.first][depItr->dest.second ? 1 : 0];
                
                if (!toUpdate) continue;
                
                toUpdate->unsatisfy(depItr->index);
            }
        }
    }
    {
        const int npCount = numericAchievedInLayer->size();
        for (int p = 0; p < npCount; ++p) {
            if ((*numericAchievedInLayer)[p] != 0.0) {
                continue;
            }
            const list<LiteralCellDependency<pair<int,bool> > > & dependents = (*negativeNumericPreconditionsToPrefs)[p];
            list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = dependents.begin();
            const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = dependents.end();
            
            for (; depItr != depEnd; ++depItr) {
                
                NNF_Flat* const toUpdate = (*unsatisfiedPreferenceConditions)[depItr->dest.first][depItr->dest.second ? 1 : 0];
                
                if (!toUpdate) continue;
                                
                toUpdate->unsatisfy(depItr->index);
            }
            
        }
    }
}

void SubproblemRPG::updatePreferencesForFact(MinimalState & startState,
                                             const uint & fID, const bool & isALiteral, const bool & polarity,
                                             const double & factLayerTime, double & maxLag, double & rpgGoalPrefViolation)
{
    
    assert(isALiteral ? (polarity ? fID < preconditionsToPrefs->size() : fID < negativePreconditionsToPrefs->size())
                      : (polarity ? fID < numericPreconditionsToPrefs->size() : fID < negativeNumericPreconditionsToPrefs->size()));
    
    const list<LiteralCellDependency<pair<int,bool> > > & dependents
        = (isALiteral ? (polarity ? (*preconditionsToPrefs)[fID] : (*negativePreconditionsToPrefs)[fID])
                      : (polarity ? (*numericPreconditionsToPrefs)[fID] : (*negativeNumericPreconditionsToPrefs)[fID]) );

    list<pair<int,int> > nowCostFreeToAdd;
    
    set<int> actsVisited;
    list<int> actsToVisit;
    
    static const int taskPrefCount = RPGBuilder::getTaskPrefCount();
    
    list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = dependents.begin();
    const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = dependents.end();
    
    for (; depItr != depEnd; ++depItr) {
        
        NNF_Flat* const toUpdate = (*unsatisfiedPreferenceConditions)[depItr->dest.first][depItr->dest.second ? 1 : 0];
        
        if (!toUpdate) continue;
        
        if (toUpdate->isSatisfied()) continue;
        
        if (GlobalSchedule::globalVerbosity & 32768) {
            cout << "\t" << COLOUR_light_green;
            
            if (isALiteral) {
                cout << "Literal fact ";
                if (!polarity) cout << "¬";
                cout << *(RPGBuilder::getLiteral(fID)) << " ";
            } else {
                cout << "Numeric fact ";
            }
            
            cout << "is relevant to preference " << RPGBuilder::getPreferences()[depItr->dest.first].name << ":" << depItr->dest.first << COLOUR_default << endl;
        }
        
        toUpdate->satisfy(depItr->index);
        
        if (depItr->dest.first < taskPrefCount) {
        
            if (toUpdate->isSatisfied()) {
                
                if (GlobalSchedule::globalVerbosity & 32768) {
                    if (depItr->dest.second) {
                        cout << "\t\t" << COLOUR_light_red << "Preference trigger now satisfied\n" << COLOUR_default;
                    } else {
                        cout << "\t\t" << COLOUR_light_red << "Preference goal now satisfied\n" << COLOUR_default;
                    }
                }
                
                (*preferencePartBecameTrueInLayer)[depItr->dest.first][depItr->dest.second ? 1 : 0] = factLayerTime;
                
                const AutomatonPosition oldPosn = (*optimisticStatusOfEachPreference)[depItr->dest.first];
                            
                PreferenceHandler::updateCostsAndPreferenceStatus(startState, depItr->dest, *optimisticStatusOfEachPreference,
                                                                   prefCostOfAddingFact, prefCostOfChangingNumberB,
                                                                   nowCostFreeToAdd);

                const AutomatonPosition newPosn = (*optimisticStatusOfEachPreference)[depItr->dest.first];
                 
                if (GlobalSchedule::globalVerbosity & 32768) {
                    cout << "\t\t" << COLOUR_light_red << "Preference status was " << positionName[oldPosn] << ", now " << positionName[newPosn] << COLOUR_default << endl;
                }
                
                if (oldPosn == unsatisfied || oldPosn == triggered) {
                    
                    if (isSatisfied(newPosn)) {
                        rpgGoalPrefViolation -= RPGBuilder::getPreferences()[depItr->dest.first].cost;
                        if (rpgGoalPrefViolation < 0.0000001) {
                            rpgGoalPrefViolation = 0.0;
                        }
                    }
                }
                
                
            }
        } else {
            if (toUpdate->isSatisfied()) {
                if (GlobalSchedule::globalVerbosity & 32768) {
                    cout << "\t\t" << COLOUR_light_blue << "Precondition preference now satisfied, at time " << factLayerTime << "\n" << COLOUR_default;
                }
                (*preferencePartBecameTrueInLayer)[depItr->dest.first][depItr->dest.second ? 1 : 0] = factLayerTime;
                
                const int currAct = RPGBuilder::getPreferences()[depItr->dest.first].attachedToOperator;
                
                if (GlobalSchedule::globalVerbosity & 32768) {
                    cout << "Applies to action " << *(RPGBuilder::getInstantiatedOp(currAct)) << endl;
                }
                ActionViolationData & actCost = (*preferenceViolationCostOfUsingAction)[currAct]; 
                
                if (actCost.canBeApplied) {
                    const set<int>::iterator pmvItr = actCost.preconditionsMeanViolating.find(depItr->dest.first);
                    assert(pmvItr != actCost.preconditionsMeanViolating.end());
                    
                    const double costDelta = RPGBuilder::getPreferences()[depItr->dest.first].cost;
                    
                    actCost.preconditionsMeanViolating.erase(pmvItr);
                    actCost.overallViolations.erase(depItr->dest.first);
                    actCost.overallViolationCost -= costDelta;
                    actCost.precViolationCost -= costDelta;
                    
                    bool goAhead = !actionPreviouslyTooExpensive[currAct];
                    
                    if (!goAhead && actCost.overallViolationCost <= maxPermissibleActionCost) {            
                        
                        actionPreviouslyTooExpensive[currAct] = false;
                        
                        list<Literal*> & delEffects = (*actionsToStartNegativeEffects)[currAct];
                        
                        list<Literal*>::iterator delEffItr = delEffects.begin();
                        const list<Literal*>::iterator delEffEnd = delEffects.end();
                        
                        for (; delEffItr != delEffEnd; ++delEffItr) {
                            const int currEff = (*delEffItr)->getStateID();
                            double & currAIL = (*negativeAchievedInLayer)[currEff];
                            if (currAIL == -1.0) {
                                negativeFactsThatAreNowAffordable.insert(make_pair(currEff, make_pair(currAct, actCost.overallViolationCost)));
                            }
                        }
                        goAhead = true;
                    }
                    
                    if (goAhead) {
                        if (actsVisited.insert(currAct).second) {
                            if (GlobalSchedule::globalVerbosity & 32768) {
                                cout << "\t\t" << COLOUR_yellow << " - Revisiting " << *(RPGBuilder::getInstantiatedOp(currAct)) << ", as it's now " << costDelta << " cheaper\n" << COLOUR_default;
                            }
                            actsToVisit.push_back(currAct);
                        }
                    }
                }
                
                preferencePartsToSatisfyBeforeAction[currAct].push_back(depItr->dest);
            }
        }
    }


        
    
    list<pair<int, int> >::iterator cfItr = nowCostFreeToAdd.begin();
    const list<pair<int,int> >::iterator cfEnd = nowCostFreeToAdd.end();
    
    for (; cfItr != cfEnd; ++cfItr) {        
        const map<int, set<int> >::iterator reduceCostItr = preferenceWouldBeViolatedByAction.find(cfItr->first);
        if (reduceCostItr == preferenceWouldBeViolatedByAction.end()) {
            if (GlobalSchedule::globalVerbosity & 32768) {
                cout << COLOUR_light_blue "\tNo actions have costs associated with " << RPGBuilder::getPreferences()[cfItr->first].name << ":" << cfItr->first << COLOUR_default << endl;
            }
            
            continue;
        }
        
        const double costDelta = prefCosts[cfItr->first];
        
        
        set<int>::const_iterator actItr = reduceCostItr->second.begin();
        const set<int>::const_iterator actEnd = reduceCostItr->second.end();
        
        for (; actItr != actEnd; ++actItr) {
            const int currAct = *actItr;
            
            ActionViolationData & actCost = (*preferenceViolationCostOfUsingAction)[currAct];
            
            actCost.effectsMeanViolating.erase(cfItr->first);
            
            if (cfItr->second != -1) {
                preferencePartsToSatisfyBeforeAction[currAct].push_back(make_pair(cfItr->first, (cfItr->second == 1)));
            }
            
            if (actCost.canBeApplied) {
                                
                if (actCost.preconditionsMeanViolating.find(cfItr->first) == actCost.preconditionsMeanViolating.end()) {                                
                    actCost.overallViolations.erase(cfItr->first);
                    actCost.overallViolationCost -= costDelta;
                    actCost.effViolationCost -= costDelta;

                    if (GlobalSchedule::globalVerbosity & 32768) {
                        cout << "Reducing the recorded cost of " << *(RPGBuilder::getInstantiatedOp(currAct)) << " to " << actCost.overallViolationCost << endl;
                    }
                    
                    
                    bool goAhead = !actionPreviouslyTooExpensive[currAct];
                    
                    if (!goAhead) {
                        if (actCost.overallViolationCost <= maxPermissibleActionCost) {            
                        
                            actionPreviouslyTooExpensive[currAct] = false;
                            
                            if (GlobalSchedule::globalVerbosity & 32768) {
                                
                                cout << "Was previously too expensive - is now no longer too expensive\n";
                            }
                            
                            list<Literal*> & delEffects = (*actionsToStartNegativeEffects)[currAct];
                            
                            list<Literal*>::iterator delEffItr = delEffects.begin();
                            const list<Literal*>::iterator delEffEnd = delEffects.end();
                            
                            for (; delEffItr != delEffEnd; ++delEffItr) {
                                const int currEff = (*delEffItr)->getStateID();
                                double & currAIL = (*negativeAchievedInLayer)[currEff];
                                if (currAIL == -1.0) {
                                    negativeFactsThatAreNowAffordable.insert(make_pair(currEff, make_pair(currAct, actCost.overallViolationCost)));
                                }
                            }
                            
                            goAhead = true;                        
                        } else {
                            if (GlobalSchedule::globalVerbosity & 32768) {
                                cout << "Is still too expensive to apply\n";
                            }
                        }
                    }
                    
                    if (goAhead) {
                        if (actsVisited.insert(currAct).second) {
                            if (GlobalSchedule::globalVerbosity & 32768) {
                                cout << "\t\t" << COLOUR_yellow << " - Revisiting " << *(RPGBuilder::getInstantiatedOp(currAct)) << ", as it's now " << costDelta << " cheaper\n" << COLOUR_default;
                            }
                            actsToVisit.push_back(currAct);
                        }
                    }
                } else {
                    if (GlobalSchedule::globalVerbosity & 32768) {
                        cout <<  "\t\tApplying " << *(RPGBuilder::getInstantiatedOp(currAct)) << " would still violate the preference, due to its preconditions\n";
                    }
                                    
                }
            } else {
                if (GlobalSchedule::globalVerbosity & 32768) {
                    cout <<  "\t\tWhen " << *(RPGBuilder::getInstantiatedOp(currAct)) << " becomes applicable, it will no longer violate the preference\n";
                }
                
            }
            
        }
        
    }
    
    double lagT = 0.0;
    bool any = false;
    for (int lag = 0; !actsToVisit.empty(); ++lag) {
        
        list<int> oldActsToVisit;
        oldActsToVisit.swap(actsToVisit);
        lagT = factLayerTime + (lag * 0.001);
        const double lagTnext = factLayerTime + ((lag + 1) * 0.001);
        
        list<int>::const_iterator oaItr = oldActsToVisit.begin();
        const list<int>::const_iterator oaEnd = oldActsToVisit.end();
        
        for (; oaItr != oaEnd; ++oaItr) {
            const int currAct = *oaItr;
            
            if (GlobalSchedule::globalVerbosity & 32768) {
                cout << "\t" << lagT << ": " << *(RPGBuilder::getInstantiatedOp(currAct)) << " adds:\n";
            }
            
            any = true;
            const ActionViolationData & newCost = (*preferenceViolationCostOfUsingAction)[currAct];
            
            list<Literal*> & addEffects = (*actionsToStartEffects)[currAct];
            
            list<Literal*>::iterator addEffItr = addEffects.begin();
            const list<Literal*>::iterator addEffEnd = addEffects.end();
            
            for (; addEffItr != addEffEnd; ++addEffItr) {
                const int currEff = (*addEffItr)->getStateID();
                //assert((*achievedInLayer)[currEff] >= 0.0);
                if (GlobalSchedule::globalVerbosity & 32768) {
                    cout << "\t\t" << *(RPGBuilder::getLiteral(currEff)) << " - fact " << currEff << endl;
                }
                visitFactThatIsNowCheaper(currAct, currEff, newCost, lagTnext, actsVisited, actsToVisit);                
            }
        }
    }
    
    if (any) {
        lagT += 0.001;
    }
    if (lagT > maxLag) {
        maxLag = lagT;
        //cout << "After updating preferences, setting lag to " << maxLag << endl;        
    }
}


void SubproblemRPG::addPreconditionCost(ActionViolationData & costData, const int & currAct) {
    
    costData.precViolationCost = 0.0;
    costData.preconditionsMeanViolating.clear();
    
    const list<Literal*> & actionPreconditionlist = (*actionsToProcessedStartPreconditions)[currAct];
    list<Literal*>::const_iterator aplItr = actionPreconditionlist.begin();
    const list<Literal*>::const_iterator aplEnd = actionPreconditionlist.end();
    
    for (; aplItr != aplEnd; ++aplItr) {
        list<PreferenceSetAndCost> & currPreData = (*preferenceViolationCostOfUsingFact)[(*aplItr)->getStateID()];
        set<int>::const_iterator viItr = currPreData.back().needsToViolate.begin();
        const set<int>::const_iterator viEnd = currPreData.back().needsToViolate.end();
        
        for (; viItr != viEnd; ++viItr) {
            if (costData.preconditionsMeanViolating.insert(*viItr).second) {
                costData.precViolationCost += prefCosts[*viItr];
            }
        }
    }
    
    const vector<int> & actPrefs = RPGBuilder::getStartPreferences()[currAct];
    
    static int ppCount;
    
    ppCount = actPrefs.size();
    
    for (int p = 0; p < ppCount; ++p) {
        const NNF_Flat* const f = (*unsatisfiedPreferenceConditions)[actPrefs[p]][0];        
        
        if (f) {
            if (!f->isSatisfied()) {
                costData.precViolationCost += prefCosts[actPrefs[p]];
                costData.preconditionsMeanViolating.insert(actPrefs[p]);
                //cout << "Applying " << *(RPGBuilder::getInstantiatedOp(currAct)) << " would mean violating precondition pref " << RPGBuilder::getPreferences()[actPrefs[p]].name << ":" << actPrefs[p] << ", as it is not currently satisfied - ";
                //cout << "cost of violation = " << prefCosts[actPrefs[p]] << endl;
            }
        } else {
            if (RPGBuilder::getPreferences()[actPrefs[p]].neverTrue) {
                costData.precViolationCost += prefCosts[actPrefs[p]];
                costData.preconditionsMeanViolating.insert(actPrefs[p]);
            }
        }
    }
    
    
                    
}

void SubproblemRPG::addEffectCost(ActionViolationData & costData, const int & currAct) {
    
    set<int> allViols;
    
    {
        list<Literal*> & addEffects = (*actionsToStartEffects)[currAct];
        
        list<Literal*>::iterator addEffItr = addEffects.begin();
        const list<Literal*>::iterator addEffEnd = addEffects.end();
        
        for (; addEffItr != addEffEnd; ++addEffItr) {
            const int currEff = (*addEffItr)->getStateID();
            
            map<int, AddingConstraints >::const_iterator addConsItr = prefCostOfAddingFact.find(currEff);
            if (addConsItr == prefCostOfAddingFact.end()) continue;
            
            allViols.insert(addConsItr->second.addingWillViolate.begin(), addConsItr->second.addingWillViolate.end());
    
            set<int>::const_iterator vItr = addConsItr->second.addingWillViolate.begin();
            const set<int>::const_iterator vEnd = addConsItr->second.addingWillViolate.end();
            
            for (; vItr != vEnd; ++vItr) {
                preferenceWouldBeViolatedByAction[*vItr].insert(currAct);
            }
            
            if (GlobalSchedule::globalVerbosity & 32768) {
                
                
                for (; vItr != vEnd; ++vItr) {
                    cout << "Add effect " << *(*addEffItr) << " Would violate " << RPGBuilder::getPreferences()[*vItr].name << ":" << *vItr << endl;
                }
            }
        }
    }
    
    {
        list<Literal*> & delEffects = (*actionsToStartNegativeEffects)[currAct];
        list<Literal*>::iterator delEffItr = delEffects.begin();
        const list<Literal*>::iterator delEffEnd = delEffects.end();
        
        for (; delEffItr != delEffEnd; ++delEffItr) {
            const int currEff = (*delEffItr)->getStateID();
            
            map<int, set<int> >::const_iterator delConsItr = prefCostOfDeletingFact.find(currEff);
            if (delConsItr == prefCostOfDeletingFact.end()) continue;
            
            allViols.insert(delConsItr->second.begin(), delConsItr->second.end());
            
            set<int>::const_iterator vItr = delConsItr->second.begin();
            const set<int>::const_iterator vEnd = delConsItr->second.end();
            
            for (; vItr != vEnd; ++vItr) {
                preferenceWouldBeViolatedByAction[*vItr].insert(currAct);
            }
            
            if (GlobalSchedule::globalVerbosity & 32768) {
                set<int>::const_iterator vItr = delConsItr->second.begin();
                const set<int>::const_iterator vEnd = delConsItr->second.end();
                
                for (; vItr != vEnd; ++vItr) {
                    cout << "Delete effect " << *(*delEffItr) << " Would violate " << RPGBuilder::getPreferences()[*vItr].name << ":" << *vItr << endl;
                }
            }
            
        }
    }            
    
    costData.effectsMeanViolating.clear();
    
    std::set_difference(allViols.begin(), allViols.end(),
                        costData.preconditionsMeanViolating.begin(), costData.preconditionsMeanViolating.end(),
                        insert_iterator<set<int> >(costData.effectsMeanViolating, costData.effectsMeanViolating.begin()));
    
    double & toIncrease = costData.effViolationCost = 0.0;
    
    set<int>::const_iterator vItr = costData.effectsMeanViolating.begin();
    const set<int>::const_iterator vEnd = costData.effectsMeanViolating.end();
    
    for (; vItr != vEnd; ++vItr) {
        toIncrease += prefCosts[*vItr];
    }
    
    costData.overallViolationCost = costData.effViolationCost + costData.precViolationCost;
    costData.overallViolations = costData.preconditionsMeanViolating;
    costData.overallViolations.insert(costData.effectsMeanViolating.begin(), costData.effectsMeanViolating.end());
    
}

ostream & operator<<(ostream & o, const SubproblemRPG::ActionViolationData & d) {
    
    
    o << "Preconditions' violation cost: " << d.precViolationCost << " - ";
    {
        set<int>::const_iterator vItr = d.preconditionsMeanViolating.begin();
        const set<int>::const_iterator vEnd = d.preconditionsMeanViolating.end();
        for (; vItr != vEnd; ++vItr) {
            o << " " << (RPGBuilder::getPreferences()[*vItr].name) << ":" << *vItr;
        }
    }
    o << endl;
    
    o << "Effects' violation cost: " << d.effViolationCost << " - ";
    {
        set<int>::const_iterator vItr = d.effectsMeanViolating.begin();
        const set<int>::const_iterator vEnd = d.effectsMeanViolating.end();
        for (; vItr != vEnd; ++vItr) {
            o << " " << (RPGBuilder::getPreferences()[*vItr].name) << ":" << *vItr;
        }
    }
    o << endl;
    
    o << "Overall violation cost: " << d.overallViolationCost << " - ";
    {
        set<int>::const_iterator vItr = d.overallViolations.begin();
        const set<int>::const_iterator vEnd = d.overallViolations.end();
        for (; vItr != vEnd; ++vItr) {
            o << " " << (RPGBuilder::getPreferences()[*vItr].name) << ":" << *vItr;
        }
    }
    o << endl;
    
    return o;
}

bool SubproblemRPG::applyOperator(const int & currAct, const double & factLayerTime, const double & nlTime,
                                  const bool & updateDebug, vector<double> & startActionSchedule,
                                  map<double, FactLayerEntry, EpsilonComp > & factLayer,
                                  set<int> & goals, const set<int>::iterator & gsEnd,
                                  int & unsatisfiedGoals, int & unappearedEnds
                                  )
{
    if (updateDebug) cout << "\tAction " << currAct << ", "  << *RPGBuilder::getInstantiatedOp(currAct) << " is now applicable at " << factLayerTime << ", putting effects at " << nlTime << endl;
    
    if (RPGBuilder::costPropagationMethod != E_NOCOSTS) {
        
        actionCosts[currAct] = 0.0;
        
        if (RPGBuilder::costPropagationMethod == E_SUMCOST) {            
            const list<Literal*> & actionPreconditionlist = (*actionsToProcessedStartPreconditions)[currAct];
            list<Literal*>::const_iterator aplItr = actionPreconditionlist.begin();
            const list<Literal*>::const_iterator aplEnd = actionPreconditionlist.end();
            
            for (; aplItr != aplEnd; ++aplItr) {
                if (MILPRPG::recogniseBootstrappingPropositions) {
                    if (!RPGBuilder::getNegativeEffectsToActions()[(*aplItr)->getStateID()].empty()) {
                        actionCosts[currAct] += propositionCosts[(*aplItr)->getStateID()];
                    }
                } else {
                    actionCosts[currAct] += propositionCosts[(*aplItr)->getStateID()];
                }
            }
        } else {
            const list<Literal*> & actionPreconditionlist = (*actionsToProcessedStartPreconditions)[currAct];
            list<Literal*>::const_iterator aplItr = actionPreconditionlist.begin();
            const list<Literal*>::const_iterator aplEnd = actionPreconditionlist.end();
            
            double t;
            for (; aplItr != aplEnd; ++aplItr) {
                
                if (MILPRPG::recogniseBootstrappingPropositions) {
                    if (!RPGBuilder::getNegativeEffectsToActions()[(*aplItr)->getStateID()].empty()) {
                        t = propositionCosts[(*aplItr)->getStateID()];
                        if (t > actionCosts[currAct]) {
                            actionCosts[currAct] = t;
                        }
                    }
                } else {                
                    t = propositionCosts[(*aplItr)->getStateID()];
                    if (t > actionCosts[currAct]) {
                        actionCosts[currAct] = t;
                    }
                }
            }
        }
        
        actionCosts[currAct] += 1;
    }
    
    
    bool printmsg = false;

    startActionSchedule[currAct] = factLayerTime;
    
    ActionViolationData & costData = (*preferenceViolationCostOfUsingAction)[currAct];
    
    costData.canBeApplied = true;
    
    addPreconditionCost(costData, currAct);
    addEffectCost(costData, currAct);

    if (updateDebug || (GlobalSchedule::globalVerbosity & 32768) ) {
        if (costData.overallViolationCost > 0.0) {
            cout << costData << endl;
        }
    }   
    
    if (costData.overallViolationCost > maxPermissibleActionCost) {
        if (updateDebug || (GlobalSchedule::globalVerbosity & 32768) ) {
            cout << "\t\t" << *(RPGBuilder::getInstantiatedOp(currAct)) << " cannot currently be applied - too expensive ( " << costData.overallViolationCost << ")\n";
            cout << costData;
        }
        actionPreviouslyTooExpensive[currAct] = true;
        return false;
    }
    
    {
        
        list<Literal*> & addEffects = (*actionsToStartEffects)[currAct];
        
        list<Literal*>::iterator addEffItr = addEffects.begin();
        const list<Literal*>::iterator addEffEnd = addEffects.end();
        
        for (; addEffItr != addEffEnd; ++addEffItr) {
            const int currEff = (*addEffItr)->getStateID();
            double & currAIL = (*achievedInLayer)[currEff];
            list<PreferenceSetAndCost> & prefDataList = (*preferenceViolationCostOfUsingFact)[currEff];
            
            if (currAIL == -1.0) {                            
                if (updateDebug) cout << "\t\tFact " << currEff << " is new\n";
                                            currAIL = nlTime;
                (*achievedBy)[currEff] = pair<int, VAL::time_spec>(currAct, VAL::E_AT_START);
                factLayer[nlTime].first.push_back(currEff);
                
                if (RPGBuilder::costPropagationMethod != E_NOCOSTS) {
                    propositionCosts[currEff] = actionCosts[currAct];
                }
                
                if (!printmsg) {
                    //cout << COLOUR_light_blue << "\tAction " << *RPGBuilder::getInstantiatedOp(currAct) << " is now applicable at " << factLayerTime << ", putting effects at " << nlTime << ":";
                    printmsg = true;
                }
                
                //cout << " " << *RPGBuilder::getLiteral(currEff); cout.flush();
                
                prefDataList.push_back(PreferenceSetAndCost(currAct, nlTime, costData.overallViolationCost, costData.overallViolations));
                
                #ifdef PARANOIA
                assert(prefDataList.back().atLayer  == nlTime);
                #endif
                
                if (goals.find(currEff) != gsEnd) {
                    if (!(--unsatisfiedGoals) && !unappearedEnds) return true;
                }
            } else {
                if (updateDebug) cout << "\t\tFact " << currEff << " was achieved in layer " << currAIL << "\n";
                                        
                if (prefDataList.back().cost > costData.overallViolationCost) {

                    if (prefDataList.back().atLayer == nlTime) {
                        prefDataList.back() = PreferenceSetAndCost(currAct, nlTime, costData.overallViolationCost, costData.overallViolations);
                    } else {                                
                        double & a = reducedCostFacts.insert(make_pair(currEff, costData.overallViolationCost)).first->second;
                        if (a > costData.overallViolationCost) {
                            a = costData.overallViolationCost;
                        }
                    }
                } else if (    RPGBuilder::switchInCheaperAchievers
                            && RPGBuilder::costPropagationMethod != E_NOCOSTS
                            && prefDataList.back().needsToViolate == costData.overallViolations
                            && currAIL == nlTime
                            && propositionCosts[currEff] > actionCosts[currAct]) {
                    
                    if (updateDebug) cout << "\t\tFact " << currEff << " already existed, but have a cheaper achiever\n";
                    
                    if (prefDataList.size() == 1) {
                        (*achievedBy)[currEff] = pair<int, VAL::time_spec>(currAct, VAL::E_AT_START);
                    }
                    
                    prefDataList.back().achiever = currAct;
                }
            }
        }
    }

    {
        
        list<Literal*> & delEffects = (*actionsToStartNegativeEffects)[currAct];
        
        list<Literal*>::iterator delEffItr = delEffects.begin();
        const list<Literal*>::iterator delEffEnd = delEffects.end();
        
        for (; delEffItr != delEffEnd; ++delEffItr) {
            const int currEff = (*delEffItr)->getStateID();
            double & currAIL = (*negativeAchievedInLayer)[currEff];
            if (currAIL == -1.0) {                            
                if (updateDebug) cout << "\t\tNegative fact " << currEff << " is new\n";
                currAIL = nlTime;
                (*negativeAchievedBy)[currEff] = pair<int, VAL::time_spec>(currAct, VAL::E_AT_START);
                factLayer[nlTime].negativeLiterals.push_back(currEff);
            }                
        }
    }
    
    if (printmsg) {
        //cout << COLOUR_default << endl;
    }
    
    return false;
    
}


void SubproblemRPG::addGoalsForPreferences(MinimalState & startState, map<double, RPGRegress, EpsilonComp> & goalsAtLayer,
                                           const map<int, double> & propsSatisfied, const set<int> & prefsUnsatisfied)
{

    list<list<Literal*> > desired;
    list<list<int> > desiredNumeric;
    
    if (RPGBuilder::useMetricRPG) {
        PreferenceHandler::getDesiredGoals(desired, &desiredNumeric, startState, *unsatisfiedPreferenceConditions, prefsUnsatisfied, preferenceViolationCostOfUsingFact, numericAchievedInLayer);
    } else {
        PreferenceHandler::getDesiredGoals(desired, 0, startState, *unsatisfiedPreferenceConditions, prefsUnsatisfied, preferenceViolationCostOfUsingFact, numericAchievedInLayer);
    }

    {    
        list<list<Literal*> >::const_iterator dItr = desired.begin();
        const list<list<Literal*> >::const_iterator dEnd = desired.end();
        
        for (; dItr != dEnd; ++dItr) {
            
            list<Literal*>::const_iterator fItr = (*dItr).begin();
            const list<Literal*>::const_iterator fItrEnd = (*dItr).end();
            for (; fItr != fItrEnd; ++fItr) {
                const int currGoal = (*fItr)->getStateID();
                
                if (propsSatisfied.find(currGoal) != propsSatisfied.end()) continue;
                
                const double insLayer = (*achievedInLayer)[currGoal];
                if (insLayer > 0.0) {
                    PreferenceSetAndCost & costData = (*preferenceViolationCostOfUsingFact)[currGoal].back();
                    goalsAtLayer[costData.atLayer].propositionalGoals.insert(make_pair(currGoal, make_pair(costData.achiever,1.0)));
                    
                    if (GlobalSchedule::globalVerbosity & 32768) {
                        cout << "Preference goal " << currGoal << ", " << *(*fItr) << " to be achieved by " << costData.achiever << " in layer with TS " << costData.atLayer << "\n";
                    }
                }
            }
            
        }
    }
    
    if (GlobalSchedule::globalVerbosity & 32768) {
        if (RPGBuilder::useMetricRPG) {
            cout << "Number of numeric preference goals: " << desiredNumeric.size() << endl;
        }
    }
    {
        list<list<int> >::const_iterator dNumItr = desiredNumeric.begin();
        const list<list<int> >::const_iterator dNumEnd = desiredNumeric.end();
        for (; dNumItr != dNumEnd; ++dNumItr) {

            list<int>::const_iterator nItr = (*dNumItr).begin();
            const list<int>::const_iterator nEnd = (*dNumItr).end();
            for (; nItr != nEnd; ++nItr) {
                const double insLayer = (*numericAchievedInLayer)[*nItr];
                if (insLayer > 0.0) {
                    goalsAtLayer[insLayer].numericGoals.insert(make_pair(*nItr, 1.0));
                    
                    if (GlobalSchedule::globalVerbosity & 32768) {
                        cout << "Preference goal " << RPGBuilder::getNumericPreTable()[*nItr] << " to be achieved by in layer with TS " << insLayer << "\n";
                    }
                }
            }
        }
    }
    
}


bool SubproblemRPG::updateActionsForFactWithPenalties(const int & toPropagate, MinimalState & startState, vector<int> & startPreconditionCounts, vector<int> & numericStartPreconditionCounts, map<double, FactLayerEntry, EpsilonComp > & factLayer, map<double, vector<double>, EpsilonComp > & fluentLayers, map<double, map<int, list<ActionFluentModification> >, EpsilonComp> & fluentModifications, const double & factLayerTime, map<double, list<int>, EpsilonComp > & endActionsAtTime, vector<double> & startActionSchedule, set<int> & goals, const set<int>::iterator & gsEnd, set<int> & goalFluents, const set<int>::iterator & gfEnd, int & unsatisfiedGoals, int & unappearedEnds, map<int,int> & insistUponEnds, map<int, int> & forbiddenStart)
{

	static const double EPSILON = 0.001;

	const double nlTime = factLayerTime + EPSILON;
	const bool updateDebug = GlobalSchedule::globalVerbosity & 64;
	const bool preconditionless = (toPropagate < 0);
	list<pair<int, VAL::time_spec> > & dependents = ( (toPropagate == -1) ? (*preconditionlessActions) : ((toPropagate == -2) ? (noLongerForbidden) : (*processedPreconditionsToActions)[toPropagate]));

    if (toPropagate != -1) {
        //cout << COLOUR_light_green << "Updating from fact "<< *(RPGBuilder::getLiteral(toPropagate)) << " at " << factLayerTime << COLOUR_default << endl;
    }
    
	list<pair<int, VAL::time_spec> >::iterator depItr = dependents.begin();
	const list<pair<int, VAL::time_spec> >::iterator depEnd = dependents.end();

	if (updateDebug) cout << "\tAffects " << dependents.size() << " actions\n";
	
//	const vector<double> & maxFluents = fluentLayers[factLayerTime];
	
	for (; depItr != depEnd; ++depItr) {
		const int currAct = depItr->first;
		assert(depItr->second == VAL::E_AT_START);

		if (updateDebug) cout << "\tAffects " << currAct << ", " << *(RPGBuilder::getInstantiatedOp(currAct)) << "\n";

        {
			if ((preconditionless || (!(--startPreconditionCounts[currAct]) && !numericStartPreconditionCounts[currAct])) && forbiddenStart.find(currAct) == forbiddenStart.end()) {
				if (!RPGBuilder::rogueActions[currAct]) {
                    if (applyOperator(currAct, factLayerTime, nlTime, updateDebug, startActionSchedule,
                                       factLayer, goals, gsEnd, unsatisfiedGoals, unappearedEnds)) {
                        return true;
                    }
					
                    
				}
				
			} else {
				if (updateDebug) cout << "\tStart of action " << currAct << " now only has " << startPreconditionCounts[currAct] << " unsatisfied propositional preconditions and " << numericStartPreconditionCounts[currAct] << " numeric\n";
			}
		}
	}

	return false;	

};

bool SubproblemRPG::updateActionsForNumericFactWithPenalties(const int & toPropagate, MinimalState & startState, vector<int> & startPreconditionCounts, vector<int> & numericStartPreconditionCounts, map<double, FactLayerEntry, EpsilonComp > & factLayer, map<double, vector<double>, EpsilonComp > & fluentLayers, map<double, map<int, list<ActionFluentModification> >, EpsilonComp> & fluentModifications, const double & factLayerTime, map<double, list<int>, EpsilonComp > & endActionsAtTime, vector<double> & startActionSchedule, set<int> & goals, const set<int>::iterator & gsEnd, set<int> & goalFluents, const set<int>::iterator & gfEnd, int & unsatisfiedGoals, int & unappearedEnds, map<int,int> & insistUponEnds, map<int, int> & forbiddenStart) {

	static const double EPSILON = 0.001;

	const double nlTime = factLayerTime + EPSILON;
	const bool updateDebug = GlobalSchedule::globalVerbosity & 64;
	list<pair<int, VAL::time_spec> > & dependents = (*processedNumericPreconditionsToActions)[toPropagate];

	list<pair<int, VAL::time_spec> >::iterator depItr = dependents.begin();
	const list<pair<int, VAL::time_spec> >::iterator depEnd = dependents.end();

	if (updateDebug) cout << "\tAffects " << dependents.size() << " actions\n";
	
//	const vector<double> & maxFluents = fluentLayers[factLayerTime];
	
	for (; depItr != depEnd; ++depItr) {
		const int currAct = depItr->first;
		assert(depItr->second == VAL::E_AT_START);
		
		if (updateDebug) cout << "\tAffects " << currAct << ", " <<  *(RPGBuilder::getInstantiatedOp(currAct)) << "\n";
		{

			if (!(--numericStartPreconditionCounts[currAct]) && !startPreconditionCounts[currAct] && forbiddenStart.find(currAct) == forbiddenStart.end()) {
				assert(!RPGBuilder::rogueActions[currAct]);
                if (applyOperator(currAct, factLayerTime, nlTime, updateDebug, startActionSchedule,
                    factLayer, goals, gsEnd, unsatisfiedGoals, unappearedEnds)) {
                    return true;
                }
                
				
			} else {
				if (updateDebug) cout << "\tStart of action " << currAct << " now only has " << startPreconditionCounts[currAct] << " unsatisfied propositional preconditions\n";
			}
		}
	}

	return false;	

};

void SubproblemRPG::findApplicableActions(MinimalState & theState, const int & nextTIL, list<pair<int, VAL::time_spec> > & applicableActions) {


	vector<int> startPreconditionCounts(*initialUnsatisfiedProcessedStartPreconditions);
	
	list<pair<int, VAL::time_spec> > toFilter;

	{
		list<pair<int, VAL::time_spec> > & dependents = (*preconditionlessActions);

		list<pair<int, VAL::time_spec> >::iterator depItr = dependents.begin();
		const list<pair<int, VAL::time_spec> >::iterator depEnd = dependents.end();
	
		for (; depItr != depEnd; ++depItr) {
			const int currAct = depItr->first;
			if (!RPGBuilder::rogueActions[currAct]) {
				const VAL::time_spec startOrEnd = depItr->second;
				assert(startOrEnd == VAL::E_AT_START);
                {
					bool mutex = false;
					{
						list<Literal*> & dels = (*actionsToStartNegativeEffects)[currAct];
	
						list<Literal*>::iterator dItr = dels.begin();
						const list<Literal*>::iterator dEnd = dels.end();
	
						for (; dItr != dEnd; ++dItr) {
							if (theState.invariants.find((*dItr)->getStateID()) != theState.invariants.end()) {
								mutex = true;
								break;
							}
						}
					}
					if (!mutex) toFilter.push_back(pair<int, VAL::time_spec>(currAct, VAL::E_AT_START));
					
				}
			}
		}
	}

	set<int>::iterator stateItr = theState.first.begin();
	const set<int>::iterator stateEnd = theState.first.end();

	for (; stateItr != stateEnd; ++stateItr) {

		list<pair<int, VAL::time_spec> > & dependents = (*processedPreconditionsToActions)[*stateItr];

		list<pair<int, VAL::time_spec> >::iterator depItr = dependents.begin();
		const list<pair<int, VAL::time_spec> >::iterator depEnd = dependents.end();
	
		for (; depItr != depEnd; ++depItr) {
			const int currAct = depItr->first;
			if (!RPGBuilder::rogueActions[currAct]) {
				const VAL::time_spec startOrEnd = depItr->second;
				assert(startOrEnd == VAL::E_AT_START);
				{
					if (!(--startPreconditionCounts[currAct])) {
						bool mutex = false;
						{
							list<Literal*> & dels = (*actionsToStartNegativeEffects)[currAct];
	
							list<Literal*>::iterator dItr = dels.begin();
							const list<Literal*>::iterator dEnd = dels.end();
	
							for (; dItr != dEnd; ++dItr) {
								if (theState.invariants.find((*dItr)->getStateID()) != theState.invariants.end()) {
									mutex = true;
									break;
								}
							}
						}
						if (!mutex) toFilter.push_back(pair<int, VAL::time_spec>(currAct, VAL::E_AT_START));
					}
				
				}
			}
		}

	}

	list<pair<int, VAL::time_spec> >::iterator fItr = toFilter.begin();
	const list<pair<int, VAL::time_spec> >::iterator fEnd = toFilter.end();

	for (; fItr != fEnd; ++fItr) {
		/*const pair<int, VAL::time_spec> currAct = *fItr;
		list<RPGBuilder::NumericPrecondition> & currList = (*actionsToNumericPreconditions)[currAct->first];
		bool isApplicable = true;		
		list<RPGBuilder::NumericPrecondition>::iterator npItr = currList.begin();
		const list<RPGBuilder::NumericPrecondition>::iterator npEnd = currList.end();

		for (; npItr != npEnd; ++npItr) {
			if (!npItr->isSatisfied(theState.second)) {
				isApplicable = false;
				break;
			}
		}
		if (isApplicable) {
			applicableActions.push_back(currAct);
		}*/

		bool isApplicable = true;

		vector<list<int> > * const toQuery = (fItr->second == VAL::E_AT_START ? actionsToProcessedStartNumericPreconditions : actionsToNumericEndPreconditions);
		if (isApplicable) {

			list<int> & nprecs = (*toQuery)[fItr->first];

			list<int>::iterator npItr = nprecs.begin();
			const list<int>::iterator npEnd = nprecs.end();

			for (; npItr != npEnd; ++npItr) {

				if (!((*rpgNumericPreconditions)[*npItr]).isSatisfiedWCalculate(theState.second)) {
					isApplicable = false;
					break;
				}

			}

		}

		if (isApplicable) {
			vector<list<int> > * const checkEffects = (fItr->second == VAL::E_AT_START ? actionsToRPGNumericStartEffects : actionsToRPGNumericEndEffects);
	
			{
	
				list<int> & neffs = (*checkEffects)[fItr->first];
	
				list<int>::iterator neItr = neffs.begin();
				const list<int>::iterator neEnd = neffs.end();
	
				for (; neItr != neEnd; ++neItr) {
	
					if (theState.fluentInvariants.find(((*rpgNumericEffects)[*neItr]).fluentIndex) != theState.fluentInvariants.end()) {
						isApplicable = false;
						break;
					}
	
				}
	
	
			}

		}

		if (isApplicable) {
			if (fItr->second == VAL::E_AT_START) {
				if (RPGBuilder::isInteresting(fItr->first, theState.first, theState.startedActions)) {
					applicableActions.push_back(*fItr);
				}
			} else {
				if (theState.startedActions.find(fItr->first) != theState.startedActions.end()) applicableActions.push_back(*fItr);
			}
		}
	}
	
	{
	
		static list<RPGBuilder::FakeTILAction> & tilActs = RPGBuilder::getTILs();
		static const list<RPGBuilder::FakeTILAction>::iterator tilEnd = tilActs.end();
		
		list<RPGBuilder::FakeTILAction>::iterator tilItr = tilActs.begin();
		
		int i = 0;
		
		for (; i < nextTIL; ++i, ++tilItr);
		
		for (; i == nextTIL && tilItr != tilEnd; ++tilItr, ++i) {
			const map<int, int>::iterator stateEnd = theState.invariants.end();

			list<Literal*> & checkFor = tilItr->delEffects;
			list<Literal*>::iterator fItr = checkFor.begin();
			const list<Literal*>::iterator fEnd = checkFor.end();
		
			bool isApplicable = true;
		
			for (; fItr != fEnd; ++fItr) {
		
				if (theState.invariants.find((*fItr)->getStateID()) != stateEnd) {
					isApplicable = false;
					break;
				}
		
			}
			if (isApplicable) {
				applicableActions.push_back(pair<int, VAL::time_spec>(i, VAL::E_AT));
			} else {
				break;
			}
		
		}
	
	}

};

bool SubproblemRPG::testApplicability(MinimalState & theState, const int & nextTIL, const pair<int, VAL::time_spec> & actID, bool fail, bool ignoreDeletes) {

	if (actID.second == VAL::E_AT_START) {
		
		/* uncomment this to ban overlapping actions
	
		if (fail) {
			assert(theState.startedActions.find(actID.first) == theState.startedActions.end());
		} else {
			if (theState.startedActions.find(actID.first) != theState.startedActions.end()) return false;
		}

		*/

		{
			const set<int>::iterator stateEnd = theState.first.end();

			list<Literal*> & checkFor = (*actionsToProcessedStartPreconditions)[actID.first];
			list<Literal*>::iterator fItr = checkFor.begin();
			const list<Literal*>::iterator fEnd = checkFor.end();
		
			
		
			for (; fItr != fEnd; ++fItr) {
		
				if (theState.first.find((*fItr)->getStateID()) == stateEnd) {
					if (fail) {
						assert(false);
					} else {
						return false;
					}
				}
		
			}
		}

		if (!ignoreDeletes) {
			const map<int, int>::iterator stateEnd = theState.invariants.end();

			list<Literal*> & checkFor = (*actionsToStartNegativeEffects)[actID.first];
			list<Literal*>::iterator fItr = checkFor.begin();
			const list<Literal*>::iterator fEnd = checkFor.end();
		
			
		
			for (; fItr != fEnd; ++fItr) {
		
				if (theState.invariants.find((*fItr)->getStateID()) != stateEnd) {
					if (fail) {
						assert(false);
					} else {
						return false;
					}
				}
		
			}
		}

		if (!ignoreDeletes) {

			list<int> & checkFor = (*actionsToProcessedStartNumericPreconditions)[actID.first];
			list<int>::iterator fItr = checkFor.begin();
			const list<int>::iterator fEnd = checkFor.end();
		
			
		
			for (; fItr != fEnd; ++fItr) {
		
				if (!((*rpgNumericPreconditions)[*fItr]).isSatisfiedWCalculate(theState.second)) {
					if (fail) {
						assert(false);
					} else {
						return false;
					}
				}
		
			}
		}

		if (!ignoreDeletes) {
			const map<int, int>::iterator stateEnd = theState.fluentInvariants.end();

			list<RPGBuilder::NumericEffect> & checkFor = (*actionsToNumericStartEffects)[actID.first];
			list<RPGBuilder::NumericEffect>::iterator fItr = checkFor.begin();
			const list<RPGBuilder::NumericEffect>::iterator fEnd = checkFor.end();
		
			
		
			for (; fItr != fEnd; ++fItr) {
		
				if (theState.fluentInvariants.find(fItr->fluentIndex) != stateEnd) {
					if (fail) {
						assert(false);
					} else {
						return false;
					}
				}
		
			}
		}

		
	} else if (actID.second == VAL::E_AT_END) {
		if (fail) {
			assert(theState.startedActions.find(actID.first) != theState.startedActions.end());
		} else {
			if (theState.startedActions.find(actID.first) == theState.startedActions.end()) return false;
		}

		{
			const set<int>::iterator stateEnd = theState.first.end();

			list<Literal*> & checkFor = (*actionsToEndPreconditions)[actID.first];
			list<Literal*>::iterator fItr = checkFor.begin();
			const list<Literal*>::iterator fEnd = checkFor.end();
		
			
		
			for (; fItr != fEnd; ++fItr) {
		
				if (theState.first.find((*fItr)->getStateID()) == stateEnd) {
					if (fail) {
						assert(false);
					} else {
						return false;
					}
				}
		
			}
		}

		if (!ignoreDeletes) {
			set<int> canIgnore;
	
			{
				list<Literal*> & invs = (*actionsToInvariants)[actID.first];
	
				list<Literal*>::iterator dItr = invs.begin();
				const list<Literal*>::iterator dEnd = invs.end();
	
				for (; dItr != dEnd; ++dItr) {
					const int currLit = (*dItr)->getStateID();
					map<int, int>::iterator fItr = theState.invariants.find(currLit);
					if (fItr != theState.invariants.end()) {
						if (fItr->second == 1) canIgnore.insert(currLit);
					}
				}
			}
			{
				const map<int, int>::iterator stateEnd = theState.invariants.end();
	
				list<Literal*> & checkFor = (*actionsToEndNegativeEffects)[actID.first];
				list<Literal*>::iterator fItr = checkFor.begin();
				const list<Literal*>::iterator fEnd = checkFor.end();
			
				
			
				for (; fItr != fEnd; ++fItr) {
					const int currLit = (*fItr)->getStateID();
					if (theState.invariants.find(currLit) != stateEnd) {
						if (canIgnore.find(currLit) == canIgnore.end()) {
							if (fail) {
								assert(false);
							} else {
								return false;
							}
						}
					}
			
				}
			}
		}

		if (!ignoreDeletes) {

			list<int> & checkFor = (*actionsToNumericEndPreconditions)[actID.first];
			list<int>::iterator fItr = checkFor.begin();
			const list<int>::iterator fEnd = checkFor.end();
		
			
		
			for (; fItr != fEnd; ++fItr) {
		
				if (!((*rpgNumericPreconditions)[*fItr]).isSatisfiedWCalculate(theState.second)) {
					if (fail) {
						assert(false);
					} else {
						return false;
					}
				}
		
			}
		}

		if (!ignoreDeletes) {
			set<int> canIgnore;
	
			{
				list<int> & invs = RPGBuilder::getMentioned(actID.first);
	
				list<int>::iterator dItr = invs.begin();
				const list<int>::iterator dEnd = invs.end();
	
				for (; dItr != dEnd; ++dItr) {
					const int currLit = (*dItr);
					map<int, int>::iterator fItr = theState.fluentInvariants.find(currLit);
					if (fItr != theState.fluentInvariants.end()) {
						if (fItr->second == 1) canIgnore.insert(currLit);
					}
				}
			}
			{
				const map<int, int>::iterator stateEnd = theState.fluentInvariants.end();
	
				list<RPGBuilder::NumericEffect> & checkFor = (*actionsToNumericEndEffects)[actID.first];
				list<RPGBuilder::NumericEffect>::iterator fItr = checkFor.begin();
				const list<RPGBuilder::NumericEffect>::iterator fEnd = checkFor.end();
		
				for (; fItr != fEnd; ++fItr) {
					const int currLit = fItr->fluentIndex;
					if (theState.fluentInvariants.find(currLit) != stateEnd) {
						if (canIgnore.find(currLit) == canIgnore.end()) {
							if (fail) {
								assert(false);
							} else {
								return false;
							}
						}
					}
			
				}
			}
		}
		
	} else { // til action
	
		if (fail) {
			assert(actID.first >= nextTIL);
		} else {
			if (actID.first < nextTIL) return false;
		}
	
		static list<RPGBuilder::FakeTILAction> & tilActs = RPGBuilder::getTILs();
		static const list<RPGBuilder::FakeTILAction>::iterator tilEnd = tilActs.end();
		
		list<RPGBuilder::FakeTILAction>::iterator tilItr = tilActs.begin();
		
		int i = 0;
		
		for (; i < nextTIL; ++i, ++tilItr);
		
		for (; i <= actID.first; ++tilItr, ++i) {
			const map<int, int>::iterator stateEnd = theState.invariants.end();

			list<Literal*> & checkFor = tilItr->delEffects;
			list<Literal*>::iterator fItr = checkFor.begin();
			const list<Literal*>::iterator fEnd = checkFor.end();
		
			for (; fItr != fEnd; ++fItr) {
				if (fail) {
					assert(theState.invariants.find((*fItr)->getStateID()) == stateEnd);
				} else {
				
					if (theState.invariants.find((*fItr)->getStateID()) != stateEnd) {
						return false;
					}
				}
		
			}

		
		}
		
		
	}
	
	return true;
	

}

struct ITD {

	int first;
	VAL::time_spec second;
	pair<double,double> third;

	ITD(const pair<int, VAL::time_spec> & o) : first(o.first), second(o.second), third(pair<double,double>(1.0,1.0)) {};
};

void SubproblemRPG::filterApplicableActions(MinimalState & theState, const int & nextTIL, list<pair<int, VAL::time_spec> > & applicableActions, list<pair<double, double> > * hhm) {

	const bool filterDebug = false;
	if (filterDebug) cout << "Filtering applicable actions\n";

	list<ITD> toFilter;

	toFilter.insert(toFilter.end(), applicableActions.begin(), applicableActions.end());

	if (hhm) {
		list<pair<double,double> >::iterator hhmItr = hhm->begin();
		const list<pair<double,double> >::iterator hhmEnd = hhm->end();

		list<ITD>::iterator itItr = toFilter.begin();
		for (; hhmItr != hhmEnd; ++hhmItr, ++itItr) {
			itItr->third = *hhmItr;
		}
		hhm->clear();
	}

	list<ITD> toNumericFilter;

	applicableActions.clear();

	if (filterDebug) cout << "Input consists of " << toFilter.size() << " actions\n";

	list<ITD>::iterator tfItr = toFilter.begin();
	const list<ITD>::iterator tfEnd = toFilter.end();

	/*for (; fItr != fEnd; ++fItr) {
		const int currAct = *fItr;
		list<RPGBuilder::NumericPrecondition> & currList = (*actionsToNumericPreconditions)[currAct];
		bool isApplicable = true;		
		list<RPGBuilder::NumericPrecondition>::iterator npItr = currList.begin();
		const list<RPGBuilder::NumericPrecondition>::iterator npEnd = currList.end();

		for (; npItr != npEnd; ++npItr) {
			if (!npItr->isSatisfied(theState.second)) {
				isApplicable = false;
				break;
			}
		}
		if (isApplicable) {
			applicableActions.push_back(currAct);
		}
	}*/

	for (; tfItr != tfEnd; ++tfItr) {

		
		
		bool isApplicable = true;

		if (tfItr->second == VAL::E_AT_START) {
			
			//if (theState.startedActions.find(tfItr->first) == theState.startedActions.end()) {
		
			      if (filterDebug) cout << "Considering start of " << *(RPGBuilder::getInstantiatedOp(tfItr->first)) << "\n";
	      
				isApplicable = RPGBuilder::isInteresting(tfItr->first, theState.first, theState.startedActions);
	      
			      if (isApplicable) {
				      const map<int, int>::iterator stateEnd = theState.invariants.end();
	      
				      list<Literal*> & checkFor = (*actionsToStartNegativeEffects)[tfItr->first];
				      list<Literal*>::iterator fItr = checkFor.begin();
				      const list<Literal*>::iterator fEnd = checkFor.end();
			      
				      
			      
				      for (; fItr != fEnd; ++fItr) {
			      
					      if (theState.invariants.find((*fItr)->getStateID()) != stateEnd) {
						      isApplicable = false;
						      break;
					      }
			      
				      }
			      }
			      if (isApplicable) {
				      const map<int, int>::iterator stateEnd = theState.fluentInvariants.end();
	      
				      list<RPGBuilder::NumericEffect> & checkFor = (*actionsToNumericStartEffects)[tfItr->first];
				      list<RPGBuilder::NumericEffect>::iterator fItr = checkFor.begin();
				      const list<RPGBuilder::NumericEffect>::iterator fEnd = checkFor.end();

				      for (; fItr != fEnd; ++fItr) {
			      
					      if (theState.fluentInvariants.find(fItr->fluentIndex) != stateEnd) {
						      isApplicable = false;
						      break;
					      }
			      
				      }				
			      }
			//} else {
			//	isApplicable = false;
			//}
			
		} else if (tfItr->second == VAL::E_AT_END) {
			if (theState.startedActions.find(tfItr->first) != theState.startedActions.end()) {
	
				if (filterDebug) cout << "Considering end of " << *(RPGBuilder::getInstantiatedOp(tfItr->first)) << "\n";
	
				set<int> canIgnore;
		
				{
					list<Literal*> & invs = (*actionsToInvariants)[tfItr->first];
		
					list<Literal*>::iterator dItr = invs.begin();
					const list<Literal*>::iterator dEnd = invs.end();
		
					for (; dItr != dEnd; ++dItr) {
						const int currLit = (*dItr)->getStateID();
						map<int, int>::iterator fItr = theState.invariants.find(currLit);
						if (fItr != theState.invariants.end()) {
							if (fItr->second == 1) canIgnore.insert(currLit);
						}
					}
				}
				{
					const map<int, int>::iterator stateEnd = theState.invariants.end();
		
					list<Literal*> & checkFor = (*actionsToEndNegativeEffects)[tfItr->first];
					list<Literal*>::iterator fItr = checkFor.begin();
					const list<Literal*>::iterator fEnd = checkFor.end();
				
					
				
					for (; fItr != fEnd; ++fItr) {
						const int currLit = (*fItr)->getStateID();
						if (theState.invariants.find(currLit) != stateEnd) {
							if (canIgnore.find(currLit) == canIgnore.end()) {
								isApplicable = false;
								break;
							}
						}
				
					}
				}
				set<int> canIgnoreFluents;
				{
					list<int> & invs = RPGBuilder::getMentioned(tfItr->first);
		
					list<int>::iterator dItr = invs.begin();
					const list<int>::iterator dEnd = invs.end();
		
					for (; dItr != dEnd; ++dItr) {
						const int currLit = (*dItr);
						map<int, int>::iterator fItr = theState.fluentInvariants.find(currLit);
						if (fItr != theState.fluentInvariants.end()) {
							if (fItr->second == 1) canIgnoreFluents.insert(currLit);
						}
					}
				}
	
				{
					const map<int, int>::iterator stateEnd = theState.fluentInvariants.end();
		
					list<RPGBuilder::NumericEffect> & checkFor = (*actionsToNumericEndEffects)[tfItr->first];
					list<RPGBuilder::NumericEffect>::iterator fItr = checkFor.begin();
					const list<RPGBuilder::NumericEffect>::iterator fEnd = checkFor.end();
				
					
				
					for (; fItr != fEnd; ++fItr) {
						const int currLit = fItr->fluentIndex;
						if (theState.fluentInvariants.find(currLit) != stateEnd) {
							if (canIgnoreFluents.find(currLit) == canIgnoreFluents.end()) {
								isApplicable = false;
								break;
							}
						}
				
					}
				}
			}
		} else if (tfItr->second == VAL::E_AT) { // TIL action

			static bool cachedTILs = false;
			static vector<RPGBuilder::FakeTILAction*> tilActs;
			if (!cachedTILs) {
				cachedTILs = true;
				list<RPGBuilder::FakeTILAction> & rpgTILs = RPGBuilder::getTILs();
				tilActs = vector<RPGBuilder::FakeTILAction*>(rpgTILs.size());

				list<RPGBuilder::FakeTILAction>::iterator rItr = rpgTILs.begin();
				const list<RPGBuilder::FakeTILAction>::iterator rEnd = rpgTILs.end();

				for (int i = 0; rItr != rEnd; ++rItr, ++i) {
					tilActs[i] = &(*rItr);
				}
			}

			if (tfItr->first >= nextTIL) {

				bool safe = true;

				for (int i = nextTIL; safe && i <= tfItr->first; ++i) {
					const map<int, int>::iterator stateEnd = theState.invariants.end();
		
					list<Literal*> & checkFor = tilActs[i]->delEffects;
					list<Literal*>::iterator fItr = checkFor.begin();
					const list<Literal*>::iterator fEnd = checkFor.end();
				
					for (; fItr != fEnd; ++fItr) {
						if (theState.invariants.find((*fItr)->getStateID()) != stateEnd) {
							safe = false;
							break;
						}
				
					}
		
				
				}

				if (safe) {
					applicableActions.push_back(pair<int, VAL::time_spec>(tfItr->first, tfItr->second));
					if (hhm) hhm->push_back(pair<double,double>(1.0,1.0));
					// skip straight to the destination list - no numeric interactions,
					// as these are timed literals, not timed fluents
				}

			}

		} else {
			isApplicable = false;
		}

		if (isApplicable) {
			toNumericFilter.push_back(*tfItr);
		}
	}



	list<ITD>::iterator tnfItr = toNumericFilter.begin();
	const list<ITD>::iterator tnfEnd = toNumericFilter.end();

	for (; tnfItr != tnfEnd; ++tnfItr) {
		bool isApplicable = true;

		if (filterDebug) cout << "Considering numerically " << *(RPGBuilder::getInstantiatedOp(tnfItr->first)) << "\n";

		vector<list<int> > * const toQuery = (tnfItr->second == VAL::E_AT_START ? actionsToProcessedStartNumericPreconditions : actionsToNumericEndPreconditions);
		{

			list<int> & nprecs = (*toQuery)[tnfItr->first];

			list<int>::iterator npItr = nprecs.begin();
			const list<int>::iterator npEnd = nprecs.end();

			for (; npItr != npEnd; ++npItr) {

				if (!((*rpgNumericPreconditions)[*npItr]).isSatisfiedWCalculate(theState.second)) {
					isApplicable = false;
					break;
				} else {
					if (filterDebug) cout << "\t"<< (*rpgNumericPreconditions)[*npItr] << "satisfied\n";
				}

			}

		}

		if (isApplicable) {
			vector<list<int> > * const checkEffects = (tnfItr->second == VAL::E_AT_START ? actionsToRPGNumericStartEffects : actionsToRPGNumericEndEffects);
	
			{
	
				list<int> & neffs = (*checkEffects)[tnfItr->first];
	
				list<int>::iterator neItr = neffs.begin();
				const list<int>::iterator neEnd = neffs.end();
	
				for (; neItr != neEnd; ++neItr) {
	
					if (theState.fluentInvariants.find((*rpgNumericEffects)[*neItr].fluentIndex) != theState.fluentInvariants.end()) {
						isApplicable = false;
						break;
					}
	
				}
	
	
			}
		}

		if (isApplicable) {
			if (hhm) {
				if (hhm->empty()) {
					hhm->push_back(tnfItr->third);
					applicableActions.push_back(pair<int, VAL::time_spec>(tnfItr->first, tnfItr->second));
				} else {
					list<pair<int, VAL::time_spec> >::iterator aaItr = applicableActions.begin();
					const list<pair<int, VAL::time_spec> >::iterator aaEnd = applicableActions.end();
					list<pair<double,double> >::iterator hhmItr = hhm->begin();

					for (; aaItr != aaEnd && ((hhmItr->first > tnfItr->third.first) || ((hhmItr->first >= tnfItr->third.first) && (hhmItr->second >= tnfItr->third.second))); ++aaItr, ++hhmItr);
					applicableActions.insert(aaItr, pair<int, VAL::time_spec>(tnfItr->first, tnfItr->second));
					hhm->insert(hhmItr, tnfItr->third);
				}
		
			} else {
				applicableActions.push_back(pair<int, VAL::time_spec>(tnfItr->first, tnfItr->second));
			}
		}
	}
	

};


list<instantiatedOp*> * SubproblemRPG::makePlan(list<int> & steps) {

	list<instantiatedOp*> * toReturn = new list<instantiatedOp*>();

	list<int>::iterator sItr = steps.begin();
	const list<int>::iterator sEnd = steps.end();
	cout << "\n";
	for (; sItr != sEnd; ++sItr) {
		toReturn->push_back(RPGBuilder::getInstantiatedOp(*sItr));
	}

	return toReturn;
}

instantiatedOp* SubproblemRPG::getOp(const int & i) {

	return RPGBuilder::getInstantiatedOp(i);

};


list<Literal*> & SubproblemRPG::getDeleteEffects(const int & i, const VAL::time_spec & t) {
	if (t == VAL::E_AT_START) {
		return ((*actionsToStartNegativeEffects)[i]);
	} else {
		return ((*actionsToEndNegativeEffects)[i]);	
	}
}

list<Literal*> & SubproblemRPG::getAddEffects(const int & i, const VAL::time_spec & t) {
	if (t == VAL::E_AT_START) {
		return ((*actionsToStartEffects)[i]);
	} else {
		return ((*actionsToEndEffects)[i]);	
	}
};

list<Literal*> & SubproblemRPG::getPreconditions(const int & i, const VAL::time_spec & t) {
	if (t == VAL::E_AT_START) {
		return ((*actionsToStartPreconditions)[i]);
	} else {
		return ((*actionsToEndPreconditions)[i]);	
	}
};


list<RPGBuilder::NumericEffect> & SubproblemRPG::getNumericEffects(const int & i, const VAL::time_spec & t) {
	if (t == VAL::E_AT_START) {
		return ((*actionsToNumericStartEffects)[i]);
	} else {
		return ((*actionsToNumericEndEffects)[i]);	
	}

};

list<Literal*> & SubproblemRPG::getInvariants(const int & i) {
	return ((*actionsToInvariants)[i]);
}

double RPGBuilder::getOpDuration(instantiatedOp* op) {
	return actionsToDurations[op->getID()];

};

double RPGBuilder::getOpDuration(const int & op) {
	return actionsToDurations[op];

};

pair<double, bool> RPGBuilder::getOpDuration(instantiatedOp* op, vector<double> & fluents) {
	NumericPrecondition * const expr = durationExpressions[op->getID()];
	return pair<double, bool>(expr->evaluateRHS(fluents),expr->op != VAL::E_EQUALS);
};

pair<double, bool> RPGBuilder::getOpDuration(const int & a, vector<double> & fluents) {
	NumericPrecondition * const expr = durationExpressions[a];
	if (!expr) return pair<double, bool>(1.0,false);
	return pair<double, bool>(expr->evaluateRHS(fluents),expr->op != VAL::E_EQUALS);
};


void RPGBuilder::getEffects(instantiatedOp* op, const bool & start, list<Literal*> & add, list<Literal*> & del, list<NumericEffect> & numeric) {

	const int actID = op->getID();

	//cout << "getting effects for action " << actID << "\n";

	if (start) {

		{
			list<Literal*> & pList = actionsToStartEffects[actID];
			add.clear(); add.insert(add.end(), pList.begin(), pList.end());
		}
		{
			list<Literal*> & pList = actionsToStartNegativeEffects[actID];
			del.clear(); del.insert(del.end(), pList.begin(), pList.end());
		}
		{
			list<NumericEffect> & nList = actionsToStartNumericEffects[actID];
			numeric.clear(); numeric.insert(numeric.end(), nList.begin(), nList.end());
		}

	} else {

		{
			list<Literal*> & pList = actionsToEndEffects[actID];
			add.clear(); add.insert(add.end(), pList.begin(), pList.end());
		}
		{
			list<Literal*> & pList = actionsToEndNegativeEffects[actID];
			del.clear(); del.insert(del.end(), pList.begin(), pList.end());
		}
		{
			list<NumericEffect> & nList = actionsToEndNumericEffects[actID];
			numeric.clear(); numeric.insert(numeric.end(), nList.begin(), nList.end());
		}

	} 

	/*const VAL::time_spec toMatch = (start ? VAL::E_AT_START : E_AT_END);

	{
		VAL::pc_list<VAL::timed_effect *>::iterator sEffItr = op->forOp()->effects->timed_effects.begin();
		const VAL::pc_list<VAL::timed_effect *>::iterator sEffEnd = op->forOp()->effects->timed_effects.end();

		for (; sEffItr != sEffEnd; ++sEffItr) {
			if ((*sEffItr)->ts == toMatch) {

				for (int pass = 0; pass < 2; ++pass) {
					VAL::pc_list<VAL::simple_effect *> & listToUse = (pass ? (*sEffItr)->effs->del_effects : (*sEffItr)->effs->add_effects);
					list<Literal*> & listToAddTo = (pass ? del : add);
					
					VAL::pc_list<VAL::simple_effect *>::iterator lItr = listToUse.begin();
					const VAL::pc_list<VAL::simple_effect *>::iterator lEnd = listToUse.end();
				
					for (; lItr != lEnd; ++lItr) {
						Literal l((*lItr)->prop,op->getEnv());
						listToAddTo.push_back(instantiatedOp::findLiteral(&l));
					}

				}
			}
		}

	}

	if (start) { // for instantaneous actions, just have a start action which has everything
		{
			list<int>::iterator effItr = actionsToEffects[actID].begin();
			const list<int>::iterator effEnd = actionsToEffects[actID].end();
	
			for (; effItr != effEnd; ++effItr) add.push_back(literals[*effItr]);
		}
		{
			list<int>::iterator effItr = actionsToNegativeEffects[actID].begin();
			const list<int>::iterator effEnd = actionsToNegativeEffects[actID].end();
	
			for (; effItr != effEnd; ++effItr) del.push_back(literals[*effItr]);
		}
		{
			numeric.insert(numeric.end(), actionsToNumericEffects[actID].begin(), actionsToNumericEffects[actID].end());	
		}
	}*/

};

void RPGBuilder::getPrecInv(instantiatedOp* op, const bool & start, list<Literal*> & precs, list<Literal*> & inv, list<NumericPrecondition> & numericPrec, list<NumericPrecondition> & numericInv) {

	//TimedPrecCollector c(0,op->getEnv(),theTC);
	//op->forOp()->visit(&c);

	//cout << "Looking up PrecInv for op " << op->getID() << " - " << *op << "\n";

	const int opIndex = op->getID();

	list<Literal*> & invList = actionsToInvariants[opIndex];

	inv.clear(); inv.insert(inv.end(), invList.begin(), invList.end());
	{
		list<NumericPrecondition> & ninv = actionsToNumericInvariants[opIndex];
		numericInv.clear(); numericInv.insert(numericInv.end(), ninv.begin(), ninv.end());
	}
	precs.clear();
	numericPrec.clear();
	if (start) {
		list<Literal*> & pList = actionsToStartPreconditions[opIndex];
		
		
		precs.insert(precs.end(), pList.begin(), pList.end());

		list<NumericPrecondition> & npList = actionsToStartNumericPreconditions[opIndex];

		numericPrec.insert(numericPrec.end(), npList.begin(), npList.end());
	} else {
		list<Literal*> & pList = actionsToEndPreconditions[opIndex];
		precs.insert(precs.end(), pList.begin(), pList.end());

		list<NumericPrecondition> & npList = actionsToEndNumericPreconditions[opIndex];

		numericPrec.insert(numericPrec.end(), npList.begin(), npList.end());
	}
	


};
/*
void RPGBuilder::getCollapsedAction(instantiatedOp* op, list<Literal*> & pre, list<Literal*> & add, list<Literal*> & del, list<NumericPrecondition> & numericPre, list<NumericEffect> & numericEff) {


	const int actID = op->getID();
	{
		list<int>::iterator effItr = actionsToEffects[actID].begin();
		const list<int>::iterator effEnd = actionsToEffects[actID].end();
	
		for (; effItr != effEnd; ++effItr) add.push_back(literals[*effItr]);
	}

	{
		list<int>::iterator effItr = actionsToNegativeEffects[actID].begin();
		const list<int>::iterator effEnd = actionsToNegativeEffects[actID].end();

		for (; effItr != effEnd; ++effItr) del.push_back(literals[*effItr]);
	}

	{
		list<int>::iterator effItr = actionsToPreconditions[actID].begin();
		const list<int>::iterator effEnd = actionsToPreconditions[actID].end();

		for (; effItr != effEnd; ++effItr) pre.push_back(literals[*effItr]);
	}
	{
		numericPre.insert(numericPre.end(), actionsToNumericPreconditions[actID].begin(), actionsToNumericPreconditions[actID].end());
	}
	{
		numericEff.insert(numericEff.end(), actionsToNumericEffects[actID].begin(), actionsToNumericEffects[actID].end());
	}

}; 
*/
void RPGBuilder::NumericEffect::display(ostream & o) const {

	o << *(RPGBuilder::getPNE(fluentIndex)) << " ";
	switch(op) {
		
		case VAL::E_ASSIGN:
			o << "= ";
			break;
		case VAL::E_INCREASE:
			o << "+= ";
			break;
		case VAL::E_DECREASE:
			o << "-= ";
			break;
		case VAL::E_SCALE_UP:
			o << "*= ";
			break;
		case VAL::E_SCALE_DOWN:
			o << "/= ";
			break;
		default:
			break;
	};
	{
		list<Operand>::const_iterator opItr = formula.begin();
		const list<Operand>::const_iterator opEnd = formula.end();
		o << "(";
		for (; opItr != opEnd; ++opItr) {
			const Operand & currOperand = *opItr;
			const math_op currMathOp = currOperand.numericOp;
			switch(currMathOp) {
				case RPGBuilder::NE_ADD:
					{
					o << " +";
					}
					break;
				case RPGBuilder::NE_SUBTRACT:
					{
					o << " -";
					}
					break;
				case RPGBuilder::NE_MULTIPLY:
					{
					o << " *";
					}
					break;
				case RPGBuilder::NE_DIVIDE:
					{
					o << " /";
					}
					break;
				case RPGBuilder::NE_CONSTANT:
					{
					o << " " << currOperand.constantValue;
					}
					break;
				case RPGBuilder::NE_FLUENT:
					{
					o << " " << *(RPGBuilder::getPNE(currOperand.fluentValue));
					}
					break;
				default:
					// this should never happen
					assert(false);
			}
		}
		o << ")";
	}

};

void RPGBuilder::NumericPrecondition::display(ostream & o) const {

	{
		list<Operand>::const_iterator opItr = LHSformula.begin();
		const list<Operand>::const_iterator opEnd = LHSformula.end();
		o << "(";
		for (; opItr != opEnd; ++opItr) {
			const Operand & currOperand = *opItr;
			const math_op currMathOp = currOperand.numericOp;
			switch(currMathOp) {
				case RPGBuilder::NE_ADD:
					{
					o << " +";
					}
					break;
				case RPGBuilder::NE_SUBTRACT:
					{
					o << " -";
					}
					break;
				case RPGBuilder::NE_MULTIPLY:
					{
					o << " *";
					}
					break;
				case RPGBuilder::NE_DIVIDE:
					{
					o << " /";
					}
					break;
				case RPGBuilder::NE_CONSTANT:
					{
					o << " " << currOperand.constantValue;
					}
					break;
				case RPGBuilder::NE_FLUENT:
					{
					o << " " << *(RPGBuilder::getPNE(currOperand.fluentValue));
					}
					break;
				default:
					// this should never happen
					assert(false);
			}
		}
		o << ")";
	}

	switch(op) {
		
		case VAL::E_GREATER:
			o << " > ";
			break;
		case VAL::E_GREATEQ:
			o << " >= ";
			break;
		case VAL::E_LESS:
			o << " < ";
			break;
		case VAL::E_LESSEQ:
			o << " <= ";
			break;
		case VAL::E_EQUALS:
			o << " = ";
			break;
	};
	{
		list<Operand>::const_iterator opItr = RHSformula.begin();
		const list<Operand>::const_iterator opEnd = RHSformula.end();
		o << "(";
		for (; opItr != opEnd; ++opItr) {
			const Operand & currOperand = *opItr;
			const math_op currMathOp = currOperand.numericOp;
			switch(currMathOp) {
				case RPGBuilder::NE_ADD:
					{
					o << " +";
					}
					break;
				case RPGBuilder::NE_SUBTRACT:
					{
					o << " -";
					}
					break;
				case RPGBuilder::NE_MULTIPLY:
					{
					o << " *";
					}
					break;
				case RPGBuilder::NE_DIVIDE:
					{
					o << " /";
					}
					break;
				case RPGBuilder::NE_CONSTANT:
					{
					o << " " << currOperand.constantValue;
					}
					break;
				case RPGBuilder::NE_FLUENT:
					{
					o << " " << *(RPGBuilder::getPNE(currOperand.fluentValue));
					}
					break;
				default:
					// this should never happen
					assert(false);
			}
		}
		o << ")";
	}

};

ostream & operator <<(ostream & o, const RPGBuilder::NumericPrecondition & p) {
	p.display(o);
	return o;
};

ostream & operator <<(ostream & o, const RPGBuilder::NumericEffect & p) {
	p.display(o);
	return o;
};

ostream & operator <<(ostream & o, const RPGBuilder::RPGNumericPrecondition & p) {
	p.display(o);
	return o;
};

ostream & operator <<(ostream & o, const RPGBuilder::ArtificialVariable & p) {
	p.display(o);
	return o;
};

bool RPGBuilder::ArtificialVariable::operator <(const RPGBuilder::ArtificialVariable & v) const {

	if (size < v.size) return true;
	if (size > v.size) return false;

	for (int i = 0; i < size; ++i) {
		const double w1 = weights[i];
		const double w2 = v.weights[i];
		if (w1 < w2) return true;
		if (w1 > w2) return false;
	}

	for (int i = 0; i < size; ++i) {
		const int w1 = fluents[i];
		const int w2 = v.fluents[i];
		if (w1 < w2) return true;
		if (w1 > w2) return false;
	}

	if (constant < v.constant) return true;
	
	return false;
};

bool RPGBuilder::RPGNumericPrecondition::operator <(const RPGBuilder::RPGNumericPrecondition & r) const{

	if (LHSVariable < r.LHSVariable) return true;
	if (LHSVariable > r.LHSVariable) return false;

	if (LHSConstant < r.LHSConstant) return true;
	if (LHSConstant > r.LHSConstant) return false;
	
	if (op < r.op) return true;
	if (op > r.op) return false;

	if (RHSVariable < r.RHSVariable) return true;
	if (RHSVariable > r.RHSVariable) return false;

	if (RHSConstant < r.RHSConstant) return true;
	
	return false;

};

bool RPGBuilder::RPGNumericEffect::operator <(const RPGNumericEffect & e) const {
	if (fluentIndex < e.fluentIndex) return true;
	if (fluentIndex > e.fluentIndex) return false;

	if (!isAssignment && e.isAssignment) return true;
	if (isAssignment && !e.isAssignment) return false;

	if (size < e.size) return false;
	if (size > e.size) return true;

	if (constant < e.constant) return true;
	if (constant > e.constant) return false;

	for (int i = 0; i < size; ++i) {

		if (variables[i] < e.variables[i]) return true;
		if (variables[i] > e.variables[i]) return false;

		if (weights[i] < e.weights[i]) return true;
		if (weights[i] > e.weights[i]) return false;

	}

	return false;


};

void RPGBuilder::ArtificialVariable::display(ostream & o) const {


	o << "av of size " << size << ", id " << ID << " (";
	const int lim = RPGBuilder::getPNECount();

	for (int i = 0; i < size; ++i) {
		if (i) o << " + ";
		if (weights[i] != 1.0) {
			o << weights[i] << "*";
		}
		const int v = fluents[i];

		if (v < lim) {
			o << *(RPGBuilder::getPNE(v));
		} else {
			o << "-1*" << *(RPGBuilder::getPNE(v-lim));
		}
	}
	if (constant != 0.0) {
		if (size) o << " + ";
		o << constant;
	}

	o << ")";
}

void RPGBuilder::RPGNumericPrecondition::display(ostream & o) const {

	const int lim = RPGBuilder::getPNECount();

	if (LHSVariable != -1) {
		if (LHSVariable < lim) {
			if (LHSConstant != 1.0) o << LHSConstant << "*";
			o << *(RPGBuilder::getPNE(LHSVariable));
		} else if (LHSVariable < (2 * lim)) {
			if (LHSConstant != 1.0) o << LHSConstant << "*";
			o << "-1*" << *(RPGBuilder::getPNE(LHSVariable-lim));
		} else {
			o << RPGBuilder::getArtificialVariable(LHSVariable);
		}
	} else {
		o << LHSConstant;
	}

	if (op == VAL::E_GREATER) {
		o << " > ";
	} else {
		o << " >= ";	
	}

	if (RHSVariable != -1) {
		if (RHSVariable < lim) {
			if (RHSConstant != 1.0) o << RHSConstant << "*";
			o << *(RPGBuilder::getPNE(RHSVariable));
		} else if (RHSVariable < (2 * lim)) {
			if (RHSConstant != 1.0) o << RHSConstant << "*";
			o << "-1*" << *(RPGBuilder::getPNE(RHSVariable-lim));
		} else {
			o << RPGBuilder::getArtificialVariable(RHSVariable);
		}
	} else {
		o << RHSConstant;
	}

	o << " [lv=" << LHSVariable << ",lc=" << LHSConstant << ",rv=" << RHSVariable << ",rc=" << RHSConstant << "]";

};

void RPGBuilder::simplify(pair<list<double>, list<int> > & p) {

	map<int,double> simplified;

    list<double>::iterator fItr = p.first.begin();
    const list<double>::iterator fEnd = p.first.end();

    list<int>::iterator sItr = p.second.begin();
    
    for (; fItr != fEnd; ++fItr, ++sItr) {
        simplified.insert(make_pair(*sItr, 0.0)).first->second += *fItr;
    }
    
    p.first.clear();
    p.second.clear();
    
    map<int,double>::const_iterator rItr = simplified.begin();
    const map<int,double>::const_iterator rEnd = simplified.end();
    
    for (; rItr != rEnd; ++rItr) {
        p.first.push_back(rItr->second);
        p.second.push_back(rItr->first);
    }
    
	/*list<double>::iterator fItr = p.first.begin();
	const list<double>::iterator fEnd = p.first.end();
	list<double>::iterator constTerm = fEnd;

	list<int>::iterator sItr = p.second.begin();
	//const list<double>::iterator sEnd = p.second.end();

	while (fItr != fEnd) {

		if (*sItr >= 0 || *sItr < -1) {
			++sItr;
			++fItr;
		} else {
			if (constTerm == fEnd) {
				constTerm = fItr;
				++fItr;
				++sItr;
			} else {
				*constTerm += *fItr;
				list<double>::iterator fErase = fItr;
				list<int>::iterator sErase = sItr;

				++fItr;
				++sItr;
				p.first.erase(fErase);
				p.second.erase(sErase);
			}
		}

	}*/

}



void RPGBuilder::handleNumericInvariants() {

// this is all a big lie!
// only handle numeric invariants that aren't modified by start effects
// we've never found a domain where the start effects modify a fluent
// and then it appears as an invariant - it's perfectly doable, but for
// now, we'll throw an assertion failure

	const int opCount = instantiatedOps.size();
	const int rpgNumPrecCount = rpgNumericPreconditionsToActions.size();

	processedRPGNumericPreconditionsToActions = vector<list<pair<int, VAL::time_spec> > >(rpgNumPrecCount);

	actionsToProcessedStartNumericPreconditions = vector<list<NumericPrecondition> >(opCount);
	actionsToProcessedStartRPGNumericPreconditions = vector<list<int> >(opCount);
	initialUnsatisfiedProcessedStartNumericPreconditions = vector<int>(opCount);

	mentionedInFluentInvariants = vector<list<int> >(opCount);

	for (int i = 0; i < opCount; ++i) {

		set<int> startModified;
		
		{
			list<NumericEffect> & startEffs = actionsToStartNumericEffects[i];
			list<NumericEffect>::iterator seItr = startEffs.begin();
			const list<NumericEffect>::iterator seEnd = startEffs.end();
			for (; seItr != seEnd; ++seItr) {
				startModified.insert(seItr->fluentIndex);
			}
		}
		
		// now have indices of all fluents modified by start effect

		list<NumericPrecondition> & addTo = actionsToProcessedStartNumericPreconditions[i] = actionsToStartNumericPreconditions[i];
	
		{

			set<int> mentionedAsSet;

			list<NumericPrecondition> & numericInvs = actionsToNumericInvariants[i];

			list<NumericPrecondition>::iterator niItr = numericInvs.begin();
			const list<NumericPrecondition>::iterator niEnd = numericInvs.end();

			for (; niItr != niEnd; ++niItr) {
				for (int pass = 0; pass < 2; ++pass) { // two or more, use a for....
					list<Operand> & toCheck = (pass ? niItr->RHSformula : niItr->LHSformula);
					list<Operand>::iterator tcItr = toCheck.begin();
					const list<Operand>::iterator tcEnd = toCheck.end();
					
					for (; tcItr != tcEnd; ++tcItr) {
						const int currFI = tcItr->fluentValue;
						if (currFI != -1) {
							mentionedAsSet.insert(currFI);
							if (startModified.find(currFI) != startModified.end()) {
								cout << "Cannot handle the case where start effect modifies a fluent appearing as an invariant\n";
								cout << "Let us know and we'll implement it\n";
								assert(false);
							}
						}
					}
				}
				// if it gets to this point, we know it's safe to blindly add to the processed starts

				addTo.push_back(*niItr);
			}
		
			{

				list<int> & pushOnto = mentionedInFluentInvariants[i];

				set<int>::iterator masItr = mentionedAsSet.begin();
				const set<int>::iterator masEnd = mentionedAsSet.end();

				for (; masItr != masEnd; ++masItr) {
					pushOnto.push_back(*masItr);
				}

			}
		}

		list<int> & addToTwo = actionsToProcessedStartRPGNumericPreconditions[i] = actionsToRPGNumericStartPreconditions[i];

		//cout << "Before handing action " << i << " had " << addToTwo.size() << " numeric start pres\n";
		
		{

			set<int> alreadyAtStart;

			{
				list<int>::iterator liItr = actionsToRPGNumericStartPreconditions[i].begin();
				const list<int>::iterator liEnd = actionsToRPGNumericStartPreconditions[i].end();

				for (; liItr != liEnd; ++liItr) alreadyAtStart.insert(*liItr);
			}

			list<int>::iterator liItr = actionsToRPGNumericInvariants[i].begin();
			const list<int>::iterator liEnd = actionsToRPGNumericInvariants[i].end();
	
			for (; liItr != liEnd; ++liItr) {
				if (alreadyAtStart.find(*liItr) == alreadyAtStart.end()) {
					addToTwo.push_back(*liItr);
				}
			}

		}

		//cout << "After handing action " << i << " has " << addToTwo.size() << " numeric start pres\n";
		//cout << "Also has " << initialUnsatisfiedNumericEndPreconditions[i] << " numeric end pres\n";
		
		initialUnsatisfiedProcessedStartNumericPreconditions[i] = addToTwo.size();
		
	}
	
	{

		for (int i = 0; i < rpgNumPrecCount; ++i) {
			list<pair<int, VAL::time_spec> > & copyTo = processedRPGNumericPreconditionsToActions[i]= rpgNumericPreconditionsToActions[i];
			
			list<pair<int, VAL::time_spec> >::iterator ctItr = copyTo.begin();
			const list<pair<int, VAL::time_spec> >::iterator ctEnd = copyTo.end();

			for (; ctItr != ctEnd; ++ctItr) {
				if (ctItr->second == VAL::E_OVER_ALL) {
					ctItr->second = VAL::E_AT_START;
				}
			}
		}

	}

}

void RPGBuilder::makeOneSided(pair<list<double>, list<int> > & LHSvariable, pair<list<double>, list<int> > & RHSvariable, const int & negOffset) {
	//pushes variables to the LHS and constants to the RHS
	//result is an expression of the form (w.x [>,>=,<,<=,==] c )

	{					
		list<double>::iterator dlItr = LHSvariable.first.begin();
		list<int>::iterator ilItr = LHSvariable.second.begin();
		const list<double>::iterator dlEnd = LHSvariable.first.end();

		while (dlItr != dlEnd) {
			if (*dlItr < 0.0) { // for negative weights
				if (*ilItr == -1) { // push constants to RHS
					RHSvariable.first.push_back(0.0 - *dlItr);
					RHSvariable.second.push_back(-1);
					simplify(RHSvariable);
					list<double>::iterator dlErr = dlItr;
					list<int>::iterator ilErr = ilItr;
					++dlItr;
					++ilItr;
					LHSvariable.first.erase(dlErr);
					LHSvariable.second.erase(ilErr);
				} else { // keep vars here, but refer to negative instances and flip sign on weight
                    if (*ilItr < 0) {
                        if (*ilItr <= -1048576) {
                            *ilItr += (1048576 - 1024);
                        } else if (*ilItr <= -1024) {
                            *ilItr -= (1048576 - 1024);
                        } else {
                            *ilItr -= 16;
                        }
                    } else {
                        *ilItr += negOffset;
                    }
					*dlItr = 0.0 - *dlItr;
					++dlItr;
					++ilItr;
				}
			} else { // positive weights are fine
				++dlItr;
				++ilItr;
			}
		}
	}


	{ // finally, push constants to right, variables to left (sec 5.1, col 2)

		list<double>::iterator dlItr = RHSvariable.first.begin();
		list<int>::iterator ilItr = RHSvariable.second.begin();
		const list<double>::iterator dlEnd = RHSvariable.first.end();

		while (dlItr != dlEnd) {
			if (*ilItr == -1) {
				// leave it alone - it's a constant term :)
				++dlItr;
				++ilItr;
			} else {
				if (*dlItr > 0.0) {
					LHSvariable.first.push_back(*dlItr);
                    
                    if (*ilItr >= 0) {
                        LHSvariable.second.push_back(*ilItr + negOffset);
                    } else {
                        if (*ilItr <= -1048576) {
                            *ilItr += (1048576 - 1024);
                        } else if (*ilItr <= -1024) {
                            *ilItr -= (1048576 - 1024);
                        } else {
                            *ilItr -= 16;
                        }
                    }
					
				} else if (*dlItr == 0.0) {
					// a null weight is a very silly thing
					
				} else {
					LHSvariable.first.push_back(0.0 - *dlItr);
					LHSvariable.second.push_back(*ilItr);
				}

				list<double>::iterator dlErr = dlItr;
				list<int>::iterator ilErr = ilItr;
				++dlItr;
				++ilItr;
				RHSvariable.first.erase(dlErr);
				RHSvariable.second.erase(ilErr);
				
			}
		}

		simplify(RHSvariable); // why not!
		simplify(LHSvariable); // why not!

	}
	
}

void printStackPair(ostream & cout, const list<double> & w, const list<int> & v, const bool newline=true){
    
    assert(w.size() == v.size());
    
    list<double>::const_iterator ldItr = w.begin();
    list<double>::const_iterator ldEnd = w.end();

    list<int>::const_iterator liItr = v.begin();

    for (int term = 0; ldItr != ldEnd; ++ldItr, ++liItr, ++term) {
        
        if (term) cout << " + ";
        if (*liItr == -1) {
            cout << *ldItr;
        } else {
            if (*ldItr != 1.0) cout << *ldItr << "*";
            if (*liItr >= 0) {
                cout << *(RPGBuilder::getPNE(*liItr));
            } else {
                if (*liItr == -3) {
                    cout << "?duration";
                } else if (*liItr == -2) {
                    cout << "#t";
                } else if (*liItr == -19) {
                    cout << "-?duration";
                } else if (*liItr == -18) {
                    cout << "-#t";
                } else if (*liItr == -512) {
                    cout << "trueprefs";
                } else if (*liItr == -513) {
                    cout << "falseprefs";
                } else if (*liItr == -528) {
                    cout << "-trueprefs";
                } else if (*liItr == -529) {
                    cout << "-falseprefs";
                } else if (*liItr <= -1048576) {
                    cout << "-(is-violated " << RPGBuilder::getPreferences()[-1048576 - *liItr].name << ")";
                } else {
                    cout << "(is-violated " << RPGBuilder::getPreferences()[-1024 - *liItr].name << ")";
                }
            }
        }
    }
    if (newline) cout << "\n";
    
}

void RPGBuilder::makeWeightedSum(list<Operand> & formula, pair<list<double>, list<int> > & result) {
				
	const bool stackDebug = false;
	
	if (stackDebug) cout << "Making weighted sum\n";
	list<pair<list<double>, list<int> > > formulaStack;
	
	list<Operand>::iterator opItr = formula.begin();
	const list<Operand>::iterator opEnd = formula.end();
	
	for (int st = 0; opItr != opEnd; ++opItr, ++st) {
		if (stackDebug) cout << "Stack term " << st << "\n";
		const Operand & currOperand = *opItr;
		const math_op currMathOp = currOperand.numericOp;
		switch(currMathOp) {
			case RPGBuilder::NE_ADD:
            {
				pair<list<double>, list<int> > oldFront = formulaStack.front(); formulaStack.pop_front();
				if (stackDebug) {
					cout << "+ operation, two terms were previously:\n";
                    cout << "\t";
                    printStackPair(cout, oldFront.first, oldFront.second);                    
                    cout << "\t";
                    printStackPair(cout, formulaStack.front().first, formulaStack.front().second);;
				}
	
				formulaStack.front().first.insert(formulaStack.front().first.begin(), oldFront.first.begin(), oldFront.first.end());
				formulaStack.front().second.insert(formulaStack.front().second.begin(), oldFront.second.begin(), oldFront.second.end());
				if (stackDebug) {
					cout << "Result:\n\t";
                    printStackPair(cout, formulaStack.front().first, formulaStack.front().second);
				}
				simplify(formulaStack.front());
				if (stackDebug) {
					cout << "Simplified:\n\t";
                    printStackPair(cout, formulaStack.front().first, formulaStack.front().second);
													
				}
				break;
            }
			case RPGBuilder::NE_SUBTRACT:
            {
				pair<list<double>, list<int> > oldFront = formulaStack.front(); formulaStack.pop_front();
				if (stackDebug) {
					cout << "- operation, two terms were previously:\n";
                    cout << "\t";
                    printStackPair(cout, oldFront.first, oldFront.second);
                    cout << "\t";
                    printStackPair(cout, formulaStack.front().first, formulaStack.front().second);
				}
				
				list<double>::iterator negItr = oldFront.first.begin();
				const list<double>::iterator negEnd = oldFront.first.end();
				for (;negItr != negEnd; ++negItr) *negItr = -1.0 * (*negItr);
				formulaStack.front().first.insert(formulaStack.front().first.begin(), oldFront.first.begin(), oldFront.first.end());
				formulaStack.front().second.insert(formulaStack.front().second.begin(), oldFront.second.begin(), oldFront.second.end());
				simplify(formulaStack.front());
				if (stackDebug) {
					cout << "Simplified:\n\t";
                    printStackPair(cout, formulaStack.front().first, formulaStack.front().second);				
				}

				
				break;
            }
			case RPGBuilder::NE_MULTIPLY:
            { // at least one of the terms has to be entirely conflict, otherwise we have var x * var y
				pair<list<double>, list<int> > oldFront = formulaStack.front(); formulaStack.pop_front();
                if (stackDebug) {
                    cout << "* operation, two terms were previously:\n";
                    cout << "\t";
                    printStackPair(cout, oldFront.first, oldFront.second);                    
                    cout << "\t";
                    printStackPair(cout, formulaStack.front().first, formulaStack.front().second);;
                }
                            
				pair<list<double>, list<int> > oldSecondFront = formulaStack.front(); formulaStack.pop_front();
				
                
				const bool firstIsConst = (oldFront.first.size() == 1 && oldFront.second.front() == -1);
				const bool secondIsConst = (oldSecondFront.first.size() == 1 && oldSecondFront.second.front() == -1);
	
				if (firstIsConst && secondIsConst) {
					formulaStack.push_front(pair<list<double>, list<int> >());
					formulaStack.front().second.push_back(-1);
					formulaStack.front().first.push_back(oldFront.first.front() * oldSecondFront.first.front());
				} else if (firstIsConst && !secondIsConst) {
					const double constVal = oldFront.first.front();
					if (constVal == 0.0) {
						formulaStack.push_front(pair<list<double>, list<int> >());
						formulaStack.front().second.push_back(-1);
						formulaStack.front().first.push_back(0.0);						
					} else {
						list<double>::iterator negItr = oldSecondFront.first.begin();
						const list<double>::iterator negEnd = oldSecondFront.first.end();
						for (;negItr != negEnd; ++negItr) *negItr = constVal * (*negItr);
						formulaStack.push_front(oldSecondFront);
					}
				} else if (!firstIsConst && secondIsConst) {
					const double constVal = oldSecondFront.first.front();
					if (constVal == 0.0) {
						formulaStack.push_front(pair<list<double>, list<int> >());
						formulaStack.front().second.push_back(-1);
						formulaStack.front().first.push_back(0.0);						
					} else {
						list<double>::iterator negItr = oldFront.first.begin();
						const list<double>::iterator negEnd = oldFront.first.end();
						for (;negItr != negEnd; ++negItr) *negItr = constVal * (*negItr);
						formulaStack.push_front(oldFront);
					}
				} else {
					string theOp;

					{
						ostringstream o;
						o << "(";                        
                        printStackPair(o, oldFront.first, oldFront.second, false);                        
                        o << ") * (";
                        printStackPair(o, oldSecondFront.first, oldSecondFront.second, false);
                        o << ")";
						
						theOp = o.str();						
					}
					postmortem_noQuadratic(theOp);
				}
                if (stackDebug) {
                    cout << "Result:\n\t";
                    printStackPair(cout, formulaStack.front().first, formulaStack.front().second);
                }
				break;
            }
			case RPGBuilder::NE_DIVIDE:
            {
				pair<list<double>, list<int> > oldFront = formulaStack.front(); formulaStack.pop_front();
				const bool firstIsConst = (oldFront.first.size() == 1 && oldFront.second.front() == -1);
				if (!firstIsConst) {
					string theOp;

					{
						ostringstream o;
                        o << "(";                        
                        printStackPair(o, formulaStack.front().first, formulaStack.front().second, false);
                        o << ") / (";
                        printStackPair(o, oldFront.first, oldFront.second, false);                        
                        o << ")";

						theOp = o.str();
					}
					postmortem_noQuadratic(theOp);
				}

				const double constVal = oldFront.first.front();
				if (stackDebug) {
					cout << "/ operation, two terms were previously:\n";
                    cout << "\t";
                    printStackPair(cout, formulaStack.front().first, formulaStack.front().second);
					cout << "\t / constant value " << constVal << "\n";
				}
				if (constVal == 0) {
					postmortem_mathsError("division by zero error", "", WhereAreWeNow);
				}
				list<double>::iterator negItr = formulaStack.front().first.begin();
				const list<double>::iterator negEnd = formulaStack.front().first.end();
				for (;negItr != negEnd; ++negItr) *negItr = (*negItr) / constVal;
				
                break;
            }
				
			case RPGBuilder::NE_CONSTANT:
            {
                formulaStack.push_front(pair<list<double>, list<int> >());
                formulaStack.front().first.push_front(currOperand.constantValue);
                formulaStack.front().second.push_front(-1);
                break;
            }			
			case RPGBuilder::NE_FLUENT:
            {
                formulaStack.push_front(pair<list<double>, list<int> >());
                formulaStack.front().first.push_front(1.0);
                formulaStack.front().second.push_front(currOperand.fluentValue);
                break;
            }            
			case RPGBuilder::NE_VIOLATION:
            {
                map<string,list<int> >::iterator vID = prefNameToID.find(currOperand.isviolated);
                if (vID == prefNameToID.end()) {
                    postmortem_isViolatedNotExist(currOperand.isviolated);
                }
                
                formulaStack.push_front(pair<list<double>, list<int> >());
                
                if (stackDebug) {
                    cout << "Preferences with name " << currOperand.isviolated << ": " << vID->second.size() << endl;
                }
                               
                list<int>::const_iterator vidItr = vID->second.begin();
                const list<int>::const_iterator vidEnd = vID->second.end();
                
                for (; vidItr != vidEnd; ++vidItr) {
                    if (stackDebug) {
                        cout << "\t" << *vidItr;
                    }
                    if (*vidItr != -1) {
                        formulaStack.front().first.push_front(1.0);
                        formulaStack.front().second.push_front(-1024 - *vidItr);
                    } else {
                        formulaStack.front().first.push_front(0.0);
                        formulaStack.front().second.push_front(-1);
                    }
                }                
                simplify(formulaStack.front());
                if (stackDebug) {
                    cout << endl;
                    printStackPair(cout, formulaStack.front().first, formulaStack.front().second, true);
                }
                break;
            }
			default:
				// this should never happen
				assert(false);
		}

	}
    if (stackDebug) {
        printStackPair(cout, formulaStack.front().first, formulaStack.front().second, true);
    }
	result = formulaStack.front();
			
}


bool RPGBuilder::processPreconditions(RPGBuilder::BuildingNumericPreconditionData & commonData,
                                       list<NumericPrecondition> & currPreList, list<int> & destList,
                                       int & toIncrement, const int & i, const VAL::time_spec & passTimeSpec)
{
    
    bool evaluateOkay = true;
    
	const bool debugRPGNum = (GlobalSchedule::globalVerbosity & 16);

	toIncrement = 0;

	list<NumericPrecondition>::iterator cpItr = currPreList.begin();
	const list<NumericPrecondition>::iterator cpEnd = currPreList.end();

	for (; cpItr != cpEnd; ++cpItr) {
		if (debugRPGNum) cout << "Converting " << *cpItr << "\n";

		pair<list<double>, list<int> > LHSvariable;
		pair<list<double>, list<int> > RHSvariable;

		makeWeightedSum(cpItr->LHSformula, LHSvariable);

		if (debugRPGNum) {
			cout << "LHS is:\n\t";
			list<double>::iterator ldItr = LHSvariable.first.begin();
			list<double>::iterator ldEnd = LHSvariable.first.end();

			list<int>::iterator liItr = LHSvariable.second.begin();
			
			for (int term = 0; ldItr != ldEnd; ++ldItr, ++liItr, ++term) {
				if (term) cout << " + ";
				if (*liItr == -1) {
					cout << *ldItr;
				} else {
					if (*ldItr != 1.0) cout << *ldItr << "*";
					cout << *(RPGBuilder::getPNE(*liItr));
				}
			}
			cout << "\n";
		}
				
		makeWeightedSum(cpItr->RHSformula, RHSvariable);


		if (debugRPGNum) {
			cout << "RHS is:\n\t";
			list<double>::iterator ldItr = RHSvariable.first.begin();
			list<double>::iterator ldEnd = RHSvariable.first.end();

			list<int>::iterator liItr = RHSvariable.second.begin();
			
			for (int term = 0; ldItr != ldEnd; ++ldItr, ++liItr, ++term) {
				if (term) cout << " + ";
				if (*liItr == -1) {
					cout << *ldItr;
				} else {
					if (*ldItr != 1.0) cout << *ldItr << "*";
					cout << *(RPGBuilder::getPNE(*liItr));
				}
			}
			cout << "\n";
		}
				
	

		list<pair<list<double>, list<int> > > finalLHS;
		list<VAL::comparison_op> finalOp;
		list<pair<list<double>, list<int> > > finalRHS;
	
	
		switch(cpItr->op) {
			case VAL::E_GREATER:
				{
				makeOneSided(LHSvariable, RHSvariable, commonData.negOffset);
				finalLHS.push_back(LHSvariable);
				finalOp.push_back(VAL::E_GREATER);
				finalRHS.push_back(RHSvariable);
				}
				break;
			case VAL::E_GREATEQ:
				{
                makeOneSided(LHSvariable, RHSvariable, commonData.negOffset);
				finalLHS.push_back(LHSvariable);
				finalOp.push_back(VAL::E_GREATEQ);
				finalRHS.push_back(RHSvariable);
				}
				break;
			case VAL::E_LESS:
				{
                makeOneSided(RHSvariable, LHSvariable, commonData.negOffset);
				finalLHS.push_back(RHSvariable);
				finalOp.push_back(VAL::E_GREATER);
				finalRHS.push_back(LHSvariable);
				}
				break;
			case VAL::E_LESSEQ:
				{
                makeOneSided(RHSvariable, LHSvariable, commonData.negOffset);
				finalLHS.push_back(RHSvariable);
				finalOp.push_back(VAL::E_GREATEQ);
				finalRHS.push_back(LHSvariable);
				}
				break;
			case VAL::E_EQUALS:
				{
				pair<list<double>, list<int> > secondLHS(RHSvariable);
				pair<list<double>, list<int> > secondRHS(LHSvariable);
	
                makeOneSided(LHSvariable, RHSvariable, commonData.negOffset);
	
				finalLHS.push_back(LHSvariable);
				finalOp.push_back(VAL::E_GREATEQ);
				finalRHS.push_back(RHSvariable);
	
                makeOneSided(secondLHS, secondRHS, commonData.negOffset);
	
				finalLHS.push_back(secondLHS);
				finalOp.push_back(VAL::E_GREATEQ);
				finalRHS.push_back(secondRHS);	
				}
				break;
		}
		
		{
			list<pair<list<double>, list<int> > >::iterator lhsItr = finalLHS.begin();
			const list<pair<list<double>, list<int> > >::iterator lhsEnd = finalLHS.end();
			list<VAL::comparison_op>::iterator opItr = finalOp.begin();
			list<pair<list<double>, list<int> > >::iterator rhsItr = finalRHS.begin();
	
			for (; lhsItr != lhsEnd; ++lhsItr, ++opItr, ++rhsItr) {
				if (debugRPGNum) {
					cout << "Storing built precondition\n";
					{
						cout << "\t";
						list<double>::iterator ldItr = lhsItr->first.begin();
						const list<double>::iterator ldEnd = lhsItr->first.end();
	
						list<int>::iterator lvItr = lhsItr->second.begin();
						bool firstTerm = true;
						for (; ldItr != ldEnd; ++ldItr, ++lvItr) {
							if (!firstTerm) cout << " + ";
							if (*lvItr == -1) {
								cout << *ldItr;
							} else {
								cout << "(";
								if (*ldItr != 1.0) cout << *ldItr << " * ";
								int vtp = *lvItr;
                                if (vtp >= commonData.negOffset) {
									cout << "-";
                                    vtp -= commonData.negOffset;
								}
								cout << *(RPGBuilder::getPNE(vtp));
								cout << ")";
							}
							firstTerm = false;
						}
					}
					if (*opItr == VAL::E_GREATER) {
						cout << " > ";
					} else {
						cout << " >= ";
					}
					{
						list<double>::iterator ldItr = rhsItr->first.begin();
						const list<double>::iterator ldEnd = rhsItr->first.end();
	
						list<int>::iterator lvItr = rhsItr->second.begin();
						bool firstTerm = true;
						for (; ldItr != ldEnd; ++ldItr, ++lvItr) {
							if (!firstTerm) cout << " + ";
							if (*lvItr == -1) {
								cout << *ldItr;
							} else {
								cout << "(";
								if (*ldItr != 1.0) cout << *ldItr << " * ";
								int vtp = *lvItr;
                                if (vtp >= commonData.negOffset) {
									cout << "-";
                                    vtp -= commonData.negOffset;
								}
								cout << *(RPGBuilder::getPNE(vtp));
								cout << ")";
							}
							firstTerm = false;
						}
					}
					cout << "\n";
				}
				++toIncrement;
				pair<list<double>, list<int> > & currLHS = *lhsItr;
				pair<list<double>, list<int> > & currRHS = *rhsItr;
				VAL::comparison_op currOp = *opItr;
	
				int lVar = -1;
				double lConst = 0.0;
				bool lIsConst = false;
	
//				int rVar = -1;
				double rConst = 0.0;
				bool rIsConst = false;
				
				{
					int rSize = currRHS.first.size();
					if (rSize == 1) {
						if (currRHS.second.front() == -1) {
							rIsConst = true;
							rConst = currRHS.first.front();
						}
					} else if (!rSize) {
						rIsConst = true;
					}
				}
	
				assert(rIsConst);
	
				{
					int lSize = currLHS.first.size();
					if (lSize == 1) {
						if (currLHS.second.front() == -1) {
							lIsConst = true;
							lConst = currLHS.first.front();
						}
					} else if (!lSize) {
						lIsConst = true;
					}
				}
	
                pair<bool,bool> beenDecided(false,false);
                                    
	
				if (!lIsConst) {
	
				
					int lSize = currLHS.first.size();
					if (lSize > 1) {
						int a = 0;
						double constTerm = 0.0;
	
						vector<double> wVector(lSize);
						vector<int> vVector(lSize);
	
						list<double>::iterator wItr = currLHS.first.begin();
						const list<double>::iterator wEnd = currLHS.first.end();
						list<int>::iterator vItr = currLHS.second.begin();
	
						for (; wItr != wEnd; ++wItr, ++vItr) {
							if (*vItr == -1) {
								constTerm = *wItr;
							} else {
								wVector[a] = *wItr;
								vVector[a] = *vItr;
								++a;
							}
						}
                        ArtificialVariable newAV(commonData.avCount,a,wVector,vVector,constTerm,rConst);
                        pair<set<ArtificialVariable>::iterator, bool> insResult = commonData.artificialVariableSet.insert(newAV);
						if (insResult.second) {
                            ++(commonData.avCount);
						} else {
							if (debugRPGNum) {
								cout << "Existing AV: " << *(insResult.first) << "\n";
							}
							(const_cast<ArtificialVariable*>(&(*insResult.first)))->updateMax(rConst);
						}
						lVar = insResult.first->ID;
						if (debugRPGNum) {
							cout << "LHS = artificial variable " << lVar << "\n";
						}
					} else if (lSize) {
						const int vCheck = currLHS.second.front();
						if (vCheck == -1) {
							assert(false);  // see above
							lConst = currLHS.first.front();
							if (debugRPGNum) {
								cout << "LHS = constant " << lConst << "\n";
							}
						} else {
							lVar = vCheck;
							lConst = currLHS.first.front();
							if (debugRPGNum) {
								cout << "LHS =";
								if (lConst != 1.0) cout << " " << lConst << " *";
								cout << " variable "<< lVar << "\n";
							}
							assert(lConst > 0.0); // would be insane otherwise - the negative variables thing should have kicked in, and having 0*var as a function is just plain silly
							const double newMaxNeed = rConst / lConst;
                            if (newMaxNeed > commonData.localMaxNeed[lVar]) commonData.localMaxNeed[lVar] = newMaxNeed;
						}
					} else {
						//assert(false);  // see above
						if (debugRPGNum) {
							cout << "LHS is empty\n";
						}
						lConst = 0.0;
						// do nothing - side is empty, bizarrely enough
					}
				} else {
                    
                    if (currOp == VAL::E_GREATER) {
                        beenDecided = pair<bool,bool>(true, lConst > rConst);
                    } else if (currOp == VAL::E_GREATEQ) {
                        beenDecided = pair<bool,bool>(true, lConst >= rConst);
                    } else {
                        assert(currOp == VAL::E_GREATER || currOp == VAL::E_GREATEQ);
                    }
                }
	
				if (!beenDecided.first) {
					RPGNumericPrecondition newPrec(commonData.precCount, lVar, lConst, currOp, rConst);
					map<RPGNumericPrecondition, list<pair<int, VAL::time_spec> > >::iterator insResult = commonData.rpgNumericPreconditionSet.find(newPrec);
					if (insResult == commonData.rpgNumericPreconditionSet.end()) {
						if (debugRPGNum) {
							cout << "New RPGNumericPrecondition created, ID = " << newPrec.ID << "\n";
							cout << "lv = " << newPrec.LHSVariable << ", lc = " << newPrec.LHSConstant << ", rv = " << newPrec.RHSVariable << ", rc = " << newPrec.RHSConstant << "\n";
		
						}
						list<pair<int, VAL::time_spec> > tmpList;
						if (i >= 0) {
							tmpList.push_back(pair<int, VAL::time_spec>(i, passTimeSpec) );						
						}
						commonData.rpgNumericPreconditionSet.insert(pair<RPGNumericPrecondition, list<pair<int, VAL::time_spec> > >(newPrec, tmpList));
						destList.push_back(commonData.precCount);
						++(commonData.precCount);
						if (debugRPGNum) {
							cout << "Registered that precondition applies to action " << i << "\n";
							cout << "Registered that action uses precondition " << destList.back() << "\n";
							cout << "precCount now " << commonData.precCount << "\n";
							
						}
					} else {
						
						if (debugRPGNum) {
							cout << "RPGNumericPrecondition reused, ID = " << insResult->first.ID << "\n";
							cout << "lv = " << insResult->first.LHSVariable << ", lc = " << insResult->first.LHSConstant << ", rv = " << insResult->first.RHSVariable << ", rc = " << insResult->first.RHSConstant << "\n";
		
						}
						if (i >= 0) {
							insResult->second.push_back(pair<int, VAL::time_spec>(i, passTimeSpec));	
						}
						destList.push_back(insResult->first.ID);
						if (debugRPGNum) {
							cout << "Registered that precondition applies to action " << i << "\n";
							cout << "Registered that action uses precondition " << destList.back() << "\n";
						
							
						}
					}
                } else {
                    evaluateOkay = evaluateOkay && beenDecided.second;
                }
			}	

		}
	}

	if (debugRPGNum) {
		cout << "Action has " << toIncrement << " numeric preconditions\n";
	}

	return evaluateOkay;
}

void RPGBuilder::buildRPGNumericPreconditions() {

	const bool debugRPGNum = (GlobalSchedule::globalVerbosity & 16);
	
    BuildingNumericPreconditionData commonData;
    
	// pair<iterator, bool> insert(const value_type& x)
	const int opCount = instantiatedOps.size();

	rpgVariableDependencies = vector<list<int> >(commonData.offset);
	variablesToRPGNumericPreconditions = vector<list<int> >(commonData.negOffset);	

	actionsToRPGNumericStartPreconditions = vector<list<int> >(opCount);
	actionsToRPGNumericInvariants = vector<list<int> >(opCount);
	actionsToRPGNumericEndPreconditions = vector<list<int> >(opCount);

	initialUnsatisfiedNumericStartPreconditions = vector<int>(opCount);
	initialUnsatisfiedNumericInvariants = vector<int>(opCount);
	initialUnsatisfiedNumericEndPreconditions = vector<int>(opCount);

    set<int> clearLater;
    
    WhereAreWeNow = PARSE_PRECONDITION;
    
	for (int i = 0; i < opCount; ++i) {
        bool okay = true;
		for (int pass = 0; pass < 3; ++pass) {

			vector<list<NumericPrecondition> > * actionsToNumericPreconditions ;
			vector<list<int> > * actionsToRPGNumericPreconditions;
			vector<int> * initialUnsatisfiedNumericPreconditions;
			VAL::time_spec passTimeSpec;

			switch (pass) {

			case 0:
				{
					actionsToNumericPreconditions = &actionsToStartNumericPreconditions;
					actionsToRPGNumericPreconditions = &actionsToRPGNumericStartPreconditions;
					initialUnsatisfiedNumericPreconditions = &initialUnsatisfiedNumericStartPreconditions;
					passTimeSpec = VAL::E_AT_START;
				}
				break;
			case 1:
				{
					actionsToNumericPreconditions = &actionsToNumericInvariants;
					actionsToRPGNumericPreconditions = &actionsToRPGNumericInvariants;
					initialUnsatisfiedNumericPreconditions = &initialUnsatisfiedNumericInvariants;
					passTimeSpec = VAL::E_OVER_ALL;
				}
				break;
			case 2:
				{
					actionsToNumericPreconditions = &actionsToEndNumericPreconditions;
					actionsToRPGNumericPreconditions = &actionsToRPGNumericEndPreconditions;
					initialUnsatisfiedNumericPreconditions = &initialUnsatisfiedNumericEndPreconditions;
					passTimeSpec = VAL::E_AT_END;
				}
				break;
			default:
				{
					cout << "This should never happen\n";
					assert(false);
				}
			};

			if (debugRPGNum) cout << "Building RPG Numeric Preconditions for " << *(instantiatedOps[i]) << "\n";

			okay = processPreconditions(commonData,
                                       (*actionsToNumericPreconditions)[i], (*actionsToRPGNumericPreconditions)[i],
                                       (*initialUnsatisfiedNumericPreconditions)[i],
                                       i, passTimeSpec);
	
			if (!okay) {
				break;
			}
		}
		
		if (!okay) {
            clearLater.insert(i);
            
        }
		
	}

    WhereAreWeNow = PARSE_GOAL;
    

	{ // now do the goals

		if (debugRPGNum) {
			cout << "Considering numeric goals\n";
		}
		
		list<NumericPrecondition>::iterator goalNumItr = numericGoals.begin();
		const list<NumericPrecondition>::iterator goalNumEnd = numericGoals.end();

		for (; goalNumItr != goalNumEnd; ++goalNumItr) {

			list<NumericPrecondition> justOne;
			justOne.push_back(*goalNumItr);

			int unsat = 0;
			list<int> dest;
	
			const bool okay = processPreconditions(commonData, justOne, dest, unsat, -1, VAL::E_AT);
		
            if (okay) {
                const int destSize = dest.size();

                if (destSize == 2) {
                    if (debugRPGNum) cout << "Evaluated to pair " << dest.front() << ", " << dest.back() << "\n";
                    numericRPGGoals.push_back(pair<int, int>(dest.front(), dest.back()));
                } else if (destSize == 1) {
                    if (debugRPGNum) cout << "Evaluated to single goal " << dest.front() << "\n";
                    numericRPGGoals.push_back(pair<int, int>(dest.front(), -1));
                } else {
                    assert(false);
                }
            } else {
                cout << "One of the numeric goal reduces to false, problem unsolvable\n";
                exit(0);
            }
		}

	}

    
    /*
    WhereAreWeNow = PARSE_CONSTRAINTS;
    const int prefCount = preferences.size();

    if (prefCount) { // now do the preferences
    
        if (debugRPGNum) {
            cout << "Considering any numeric preferences\n";
        }
        
        for (int p = 0; p < prefCount; ++p) {
            for (int pass = 0; pass < 2; ++pass) {
                list<NumericPrecondition> & npList = (pass ? preferences[p].triggerNum : preferences[p].goalNum);
                list<pair<int,int> > & npDest = (pass ? preferences[p].triggerRPGNum : preferences[p].goalRPGNum);
    
                bool okay = true;
                
                list<NumericPrecondition>::iterator goalNumItr = npList.begin();
                const list<NumericPrecondition>::iterator goalNumEnd = npList.end();
                
                for (; goalNumItr != goalNumEnd; ++goalNumItr) {
                    
                    list<NumericPrecondition> justOne;
                    justOne.push_back(*goalNumItr);
                    
                    int unsat = 0;
                    list<int> dest;
                    
                    
                    
                    okay = processPreconditions(commonData, justOne, dest, unsat, -1, VAL::E_AT);
                                        
                    if (okay) {
                        const int destSize = dest.size();
                        
                        if (destSize == 2) {
                            if (debugRPGNum) cout << "Evaluated to pair " << dest.front() << ", " << dest.back() << "\n";
                            npDest.push_back(pair<int, int>(dest.front(), dest.back()));
                        } else if (destSize == 1) {
                            if (debugRPGNum) cout << "Evaluated to single goal " << dest.front() << "\n";
                            npDest.push_back(pair<int, int>(dest.front(), -1));
                        } else {
                            assert(false);
                        }
                    } else {
                        cout << "Warning: an erroenous numeric expression was encountered in preference " << preferences[p].name << endl;
                    }
                }
                                
            }
        }
        
    }
    
    WhereAreWeNow = PARSE_UNKNOWN;
    */
	
    PreferenceHandler::substituteRPGNumericPreconditions(commonData);


    
	
	assert((int) commonData.rpgNumericPreconditionSet.size() == commonData.precCount);
	
    
    rpgNumericPreconditions = vector<RPGNumericPrecondition>(commonData.precCount);
    rpgNumericPreconditionsToActions = vector<list<pair<int, VAL::time_spec> > >(commonData.precCount);
    
    rpgArtificialVariables = vector<ArtificialVariable>(commonData.avCount - commonData.offset);
    rpgArtificialVariablesToPreconditions = vector<list<int> >(commonData.avCount - commonData.offset);
    rpgNegativeVariablesToPreconditions = vector<list<int> >(commonData.negOffset);
    rpgPositiveVariablesToPreconditions = vector<list<int> >(commonData.negOffset);
    
	maxNeeded = vector<double>(commonData.avCount);

    for (int i = 0; i < commonData.offset; ++i) maxNeeded[i] = commonData.localMaxNeed[i];

	{
        set<ArtificialVariable>::iterator avsItr = commonData.artificialVariableSet.begin();
        const set<ArtificialVariable>::iterator avsEnd = commonData.artificialVariableSet.end();

		for (; avsItr != avsEnd; ++avsItr) {
            const int oIndex = avsItr->ID - commonData.offset;
			rpgArtificialVariables[oIndex] = *avsItr;
			maxNeeded[avsItr->ID] = avsItr->maxNeed;
			if (debugRPGNum) cout << "Storing AV with ID " << avsItr->ID << " at index " << oIndex << "\n";
			{
				const int afflim = avsItr->size;
				for (int aff = 0; aff < afflim; ++aff) {
					const int affVar = avsItr->fluents[aff];
					rpgVariableDependencies[affVar].push_back(avsItr->ID);
				}
			}
		}
	}
	
	{
        map<RPGNumericPrecondition, list<pair<int, VAL::time_spec> > >::iterator rnpItr = commonData.rpgNumericPreconditionSet.begin();
        const map<RPGNumericPrecondition, list<pair<int, VAL::time_spec> > >::iterator rnpEnd = commonData.rpgNumericPreconditionSet.end();
        
		for (; rnpItr != rnpEnd; ++rnpItr) {
			const RPGNumericPrecondition & currPrec = rnpItr->first;
			const int currID = currPrec.ID;
			rpgNumericPreconditions[currID] = currPrec;
			rpgNumericPreconditionsToActions[currID] = rnpItr->second;

			if (debugRPGNum) cout << "Precondition ID: " << currID << "\n";
			
			{
				const int var = currPrec.LHSVariable;
                if (var >= commonData.offset) {
					rpgArtificialVariablesToPreconditions[var-commonData.offset].push_back(currID);
					ArtificialVariable * const avsItr = &(rpgArtificialVariables[var-commonData.offset]);
					{
						const int afflim = avsItr->size;
						for (int aff = 0; aff < afflim; ++aff) {
							const int affVar = avsItr->fluents[aff];
							if (affVar < 0) {
							} else if (affVar < commonData.negOffset) {
								variablesToRPGNumericPreconditions[affVar].push_back(currID);
							} else {
								variablesToRPGNumericPreconditions[affVar-commonData.negOffset].push_back(currID);
							}
						}
					}
                } else if (var >= commonData.negOffset) {
					rpgNegativeVariablesToPreconditions[var-commonData.negOffset].push_back(currID);
					variablesToRPGNumericPreconditions[var-commonData.negOffset].push_back(currID);
				} else if (var != -1) {
					rpgPositiveVariablesToPreconditions[var].push_back(currID);
					variablesToRPGNumericPreconditions[var].push_back(currID);
				}
			}

/*			{
				const int var = currPrec.RHSVariable;
				if (var >= offset) {
					rpgArtificialVariablesToPreconditions[var-offset].push_back(currID);
				} else if (var >= negOffset) {
					rpgNegativeVariablesToPreconditions[var-negOffset].push_back(currID);
				} else if (var != -1) {
					rpgPositiveVariablesToPreconditions[var].push_back(currID);
				}
			}*/
			
			if (debugRPGNum) {
				cout << "Built precondition " << currPrec << "\n";
				cout << "Applies to actions:\n";
				list<pair<int, VAL::time_spec> >::iterator ataItr = rpgNumericPreconditionsToActions[currID].begin();
				const list<pair<int, VAL::time_spec> >::iterator ataEnd = rpgNumericPreconditionsToActions[currID].end();
				for (; ataItr != ataEnd; ++ataItr) {
					cout << "\t" << ataItr->first << " " << *(instantiatedOps[ataItr->first]) << " " << (ataItr->second == VAL::E_AT_START ? "start" : "end") << "\n";
				}
			}	
		}
		
	}
	
    numericAchievedInLayer = vector<double>(commonData.precCount);
    numericAchievedInLayerReset = vector<double>(commonData.precCount,-1.0);    
    numericAchievedBy = vector<ActionFluentModification*>(commonData.precCount);
    numericAchievedByReset = vector<ActionFluentModification*>(commonData.precCount,(ActionFluentModification*)0);

    negativeNumericAchievedInLayer = vector<double>(commonData.precCount);
    negativeNumericAchievedInLayerReset = vector<double>(commonData.precCount,0.0);    
    negativeNumericAchievedBy = vector<ActionFluentModification*>(commonData.precCount);
    negativeNumericAchievedByReset = vector<ActionFluentModification*>(commonData.precCount,(ActionFluentModification*)0);
    
    
    {
        set<int>::iterator clItr = clearLater.begin();
        const set<int>::iterator clEnd = clearLater.end();

        for (; clItr != clEnd; ++clItr) {

            const int i = *clItr;
            for (int pass = 0; pass < 3; ++pass) {
                VAL::time_spec passTimeSpec = VAL::E_AT;
                vector<list<int> > * actionsToRPGNumericPreconditions = 0;

                switch (pass) {

                case 0:
                    {
                        actionsToRPGNumericPreconditions = &actionsToRPGNumericStartPreconditions;
                        passTimeSpec = VAL::E_AT_START;
                    }
                    break;
                case 1:
                    {
                        actionsToRPGNumericPreconditions = &actionsToRPGNumericInvariants;
                        passTimeSpec = VAL::E_OVER_ALL;
                    }
                    break;
                case 2:
                    {
                        actionsToRPGNumericPreconditions = &actionsToRPGNumericEndPreconditions;
                        passTimeSpec = VAL::E_AT_END;
                    }
                    break;
                }

                list<int>::iterator clearItr = (*actionsToRPGNumericPreconditions)[i].begin();
                const list<int>::iterator clearEnd = (*actionsToRPGNumericPreconditions)[i].end();

                for (; clearItr != clearEnd; ++clearItr) {

                    rpgNumericPreconditionsToActions[*clearItr].remove(make_pair(i, passTimeSpec));

                }
            }
            rogueActions[i] = true;
        }
    }

    
	if (debugRPGNum) {
		cout << "All done!\n";
	}

    initialisedNumericPreTable = true;
}

void RPGBuilder::buildRPGNumericEffects() {

	const bool localDebug = (GlobalSchedule::globalVerbosity & 16);

	const int opCount = instantiatedOps.size();

	const int negOffset = pnes.size();
	
	realVariablesToRPGEffects = vector<list<int> >(negOffset * 2);
	
	actionsToRPGNumericStartEffects = vector<list<int> >(opCount);
	actionsToRPGNumericEndEffects = vector<list<int> >(opCount);
	
	map<RPGNumericEffect, list<pair<int, VAL::time_spec> > > rpgNumericEffectSet;

	
	const list<pair<int, VAL::time_spec> > emptyList;
	
	int effID = 0;
	
	for (int act = 0; act < opCount; ++act) {
	
		for (int pass = 0; pass < 2; ++pass) {	
		
			list<NumericEffect> & numEffs = (pass ? actionsToEndNumericEffects[act] : actionsToStartNumericEffects[act]);
			
			list<NumericEffect>::iterator neItr = numEffs.begin();
			const list<NumericEffect>::iterator neEnd = numEffs.end();
			
			for (; neItr != neEnd; ++neItr) {

				if (localDebug) cout << "Action " << act << " " << *(RPGBuilder::getInstantiatedOp(act)) << " has " << (pass ? "end" : "start") << " effect:\n";
			
				NumericEffect & currEffect = *neItr;
				
				pair<list<double>, list<int> > weightedSum;

				makeWeightedSum(currEffect.formula, weightedSum);
				
				if (currEffect.op == VAL::E_DECREASE) {
					list<double>::iterator dlItr = weightedSum.first.begin();
					const list<double>::iterator dlEnd = weightedSum.first.end();
					
					for (; dlItr != dlEnd; ++dlItr) {
						if (*dlItr != 0.0) *dlItr = 0.0 - *dlItr;
					}
				}
				
				{					
					list<double>::iterator dlItr = weightedSum.first.begin();
					list<int>::iterator ilItr = weightedSum.second.begin();
					const list<double>::iterator dlEnd = weightedSum.first.end();
			
					for (; dlItr != dlEnd; ++dlItr, ++ilItr) {
						if (*dlItr < 0.0) { // for negative weights
							if (*ilItr >= 0) {
								*ilItr += negOffset;
								*dlItr = 0.0 - *dlItr;
								
							}
						}

					}
				}
				
				int lSize = weightedSum.first.size();
				
				double constTerm = 0.0;
	
				int a = 0;
				
				vector<double> wVector(lSize);
				vector<int> vVector(lSize);

				list<double>::iterator wItr = weightedSum.first.begin();
				const list<double>::iterator wEnd = weightedSum.first.end();
				list<int>::iterator vItr = weightedSum.second.begin();

				for (; wItr != wEnd; ++wItr, ++vItr) {
					if (*vItr == -1) {
						constTerm = *wItr;
					} else {
						wVector[a] = *wItr;
						vVector[a] = *vItr;
						++a;
					}
				}
				
				RPGNumericEffect rpgEffect(effID, currEffect.fluentIndex, currEffect.op == VAL::E_ASSIGN, wVector, vVector, a, constTerm);
				
				pair<map<RPGNumericEffect, list<pair<int, VAL::time_spec> > >::iterator, bool> insItr
				    = rpgNumericEffectSet.insert(pair<RPGNumericEffect, list<pair<int, VAL::time_spec> > >(rpgEffect, emptyList));
				    
				if (insItr.second) {
					{					
						list<double>::iterator dlItr = weightedSum.first.begin();
						list<int>::iterator ilItr = weightedSum.second.begin();
						const list<double>::iterator dlEnd = weightedSum.first.end();
				
						for (; dlItr != dlEnd; ++dlItr, ++ilItr) {
							if (*ilItr >= 0) {
								realVariablesToRPGEffects[*ilItr].push_back(insItr.first->first.ID);
							}
	
						}
					}
					++effID;
				}
				
				insItr.first->second.push_back(pair<int, VAL::time_spec>(act, (pass ? VAL::E_AT_END : VAL::E_AT_START)));
				
				(pass ? actionsToRPGNumericEndEffects : actionsToRPGNumericStartEffects)[act].push_back(insItr.first->first.ID);		
				
				
				

								
				if (localDebug) {
					
					cout << "\t" << *(getPNE(currEffect.fluentIndex)) << " ";
					if (currEffect.op == VAL::E_ASSIGN) {
						cout << "= ";
					} else {
					 	cout << "+= ";
					}
				
					list<double>::iterator wItr = weightedSum.first.begin();
					const list<double>::iterator wEnd = weightedSum.first.end();
					list<int>::iterator vItr = weightedSum.second.begin();
					bool isFirst = true;
					for (; wItr != wEnd; ++wItr, ++vItr) {
						if (!isFirst) cout << " + ";
						if (*vItr == -1) {
							cout << *wItr;
						} else if (*vItr >= negOffset) {
							cout << "-";
							if (*wItr != 1.0) cout << "(" << *wItr << " * ";
							cout << *(getPNE(*vItr - negOffset));
							if (*wItr != 1.0) cout << ")";
						} else {
							if (*wItr != 1.0) cout << "(" << *wItr << " * ";
							cout << *(getPNE(*vItr));
							if (*wItr != 1.0) cout << ")";
						}
						isFirst = false;
					}
					cout << "\n";
					if (insItr.second) cout << "Effect hasn't been seen before anywhere\n";
				}
			}
						
		}
	
	}
	
	rpgNumericEffects = vector<RPGNumericEffect>(effID);
	rpgNumericEffectsToActions = vector<list<pair<int, VAL::time_spec> > >(effID);
	
	map<RPGNumericEffect, list<pair<int, VAL::time_spec> > >::iterator effItr = rpgNumericEffectSet.begin();
	const map<RPGNumericEffect, list<pair<int, VAL::time_spec> > >::iterator effEnd = rpgNumericEffectSet.end();
	
	for (; effItr != effEnd; ++effItr) {
		
		const RPGNumericEffect & currEff = effItr->first;
		const int currID = currEff.ID;
		
		rpgNumericEffects[currID] = effItr->first;
		rpgNumericEffectsToActions[currID] = effItr->second;
		
	}

};

void RPGBuilder::buildDurations(vector<NumericPrecondition*> & durations) {

	const bool durDebug = (GlobalSchedule::globalVerbosity & 16);

	InitialStateCollector c(0,0,theTC);

	current_analysis->the_problem->visit(&c);
	
	//initialState = c.initialState;
	//initialFluents = c.initialFluents;

	const int lim = durations.size();

	for (int i = 0; i < lim; ++i) {
		NumericPrecondition* const currExpr = durations[i];
		
		if (currExpr) {
	
			switch(currExpr->op) {
				case VAL::E_EQUALS:
					actionsToDurations[i] = currExpr->evaluateRHS(c.initialFluents);
					break;
				default:
					cout << "Cannot handle duration inequalities\n";
					assert(false);
			}	
			if (durDebug) cout << "Duration of action " << i << " = " << actionsToDurations[i] << "\n";
			problemIsNotTemporal = false;
		} else {
			actionsToDurations[i] = 1.0;
			if (durDebug) cout << "Duration of action fixed to 1.0\n";
		}

		//delete currExpr;
	}

}

void copyLiteralListToSet(list<Literal*> & theList, LiteralSet & theSet) {

	list<Literal*>::iterator tlItr = theList.begin();
	const list<Literal*>::iterator tlEnd = theList.end();

	for (; tlItr != tlEnd; ++tlItr) theSet.insert(*tlItr);

};

//class OneShotKShotFormula : public KShotFormula {
//
//private:
	//list<int> watchedLiterals;
//public:
	//OneShotKShotFormula(list<int> & toWatch) : KShotFormula(), watchedLiterals(toWatch) {};
//	virtual int getLimit(MinimalState & s);
//};

int RPGBuilder::OneShotKShotFormula::getLimit(MinimalState & s) {

	list<int>::iterator wlItr = watchedLiterals.begin();
	const list<int>::iterator wlEnd = watchedLiterals.end();

	for (; wlItr != wlEnd; ++wlItr) {
		if (s.first.find(*wlItr) == s.first.end()) return 0;
	}

	return 1;

};

int RPGBuilder::KShotKShotFormula::getLimit(MinimalState & s)
{
    
    int minShots = INT_MAX;
    
    list<ShotCalculator>::const_iterator sfItr = formulae.begin();
    const list<ShotCalculator>::const_iterator sfEnd = formulae.end();
    
    for (; sfItr != sfEnd; ++sfItr) {
        
        const int thisShots = (int)((s.second[sfItr->variable] - sfItr->greaterThan) / sfItr->decreaseBy);
        if (thisShots < minShots) minShots = thisShots;
        
    }
    
    return (minShots > 0 ? minShots : 0);
        
}



int RPGBuilder::OneShotKShotFormula::getOptimisticLimit(MinimalState &) {
    
    return 1;
    
};
    
    
int RPGBuilder::KShotKShotFormula::getOptimisticLimit(MinimalState & s) {
    
    return getLimit(s);
    
};
        

bool constOnlyCalculate(const list<RPGBuilder::Operand> & formula, vector<double> & fluents, double & cValue) {

	list<double> RHS;
	
	list<RPGBuilder::Operand>::const_iterator fItr = formula.begin();
	const list<RPGBuilder::Operand>::const_iterator fEnd = formula.end();

	for (; fItr != fEnd; ++fItr) {
		const RPGBuilder::Operand & currOperand = *fItr;
		const RPGBuilder::math_op currMathOp = currOperand.numericOp;
		switch(currMathOp) {
			case RPGBuilder::NE_ADD:
				{
				const double oldFront = RHS.front(); RHS.pop_front();
				RHS.front() += oldFront;
				}
				break;
			case RPGBuilder::NE_SUBTRACT:
				{
				const double oldFront = RHS.front(); RHS.pop_front();
				RHS.front() -= oldFront;
				}
				break;
			case RPGBuilder::NE_MULTIPLY:
				{
				const double oldFront = RHS.front(); RHS.pop_front();
				RHS.front() *= oldFront;
				}
				break;
			case RPGBuilder::NE_DIVIDE:
				{
				const double oldFront = RHS.front(); RHS.pop_front();
				RHS.front() /= oldFront;
				}
				break;
			case RPGBuilder::NE_CONSTANT:
				RHS.push_front(currOperand.constantValue);
				break;
			case RPGBuilder::NE_FLUENT:
				return false;
				break;
			default:
				// this should never happen
				assert(false);
		}
	}

	cValue = RHS.front();
	return true;

};

void RPGBuilder::findSelfMutexes() {

	const int opCount = instantiatedOps.size();
	selfMutexes = vector<bool>(opCount);

	for (int i = 0; i < opCount; ++i) {

		selfMutexes[i] = false;

		{	
			LiteralSet allPreconditions;
			LiteralSet allDeletes;

			copyLiteralListToSet(actionsToStartPreconditions[i], allPreconditions);
			copyLiteralListToSet(actionsToInvariants[i], allPreconditions);
			copyLiteralListToSet(actionsToEndPreconditions[i], allPreconditions);

			copyLiteralListToSet(actionsToStartNegativeEffects[i], allDeletes);
			copyLiteralListToSet(actionsToEndNegativeEffects[i], allDeletes);

			{
				LiteralSet::iterator apItr = allPreconditions.begin();
				const LiteralSet::iterator apEnd = allPreconditions.end();

				LiteralSet::iterator adItr = allDeletes.begin();
				const LiteralSet::iterator adEnd = allDeletes.end();

				while (apItr != apEnd && adItr != adEnd) {
					const int idOne = (*apItr)->getStateID();
					const int idTwo = (*adItr)->getStateID();
					if (idOne < idTwo) {
						++apItr;
					} else if (idTwo < idOne) {
						++adItr;
					} else {
						selfMutexes[i] = true;
						++apItr;
						++adItr;
					}
				};
			}
		}

	}

};

void RPGBuilder::kshotInferForAction(const int & i, MinimalState & refState, LiteralSet & maybeOneShotLiteral, vector<double> & initialFluents, const int & fluentCount) {


	int oldKShotLimit = INT_MAX;

	if (kShotFormulae[i]) oldKShotLimit = kShotFormulae[i]->getLimit(refState);

	if (oldKShotLimit > 1) {

		

		{ // first, let's see if there's a simple propositional reason for it being oneshot
		  // look for a delete effect not added by anything else, and required as a precond.
			
			LiteralSet allPreconditions;
			LiteralSet allDeletes;

			copyLiteralListToSet(actionsToStartPreconditions[i], allPreconditions);
			copyLiteralListToSet(actionsToInvariants[i], allPreconditions);
			copyLiteralListToSet(actionsToEndPreconditions[i], allPreconditions);

			copyLiteralListToSet(actionsToStartNegativeEffects[i], allDeletes);
			copyLiteralListToSet(actionsToEndNegativeEffects[i], allDeletes);

			set<int> theIntersection;

			{
				LiteralSet::iterator apItr = allPreconditions.begin();
				const LiteralSet::iterator apEnd = allPreconditions.end();

				LiteralSet::iterator adItr = allDeletes.begin();
				const LiteralSet::iterator adEnd = allDeletes.end();

				while (apItr != apEnd && adItr != adEnd) {
					const int idOne = (*apItr)->getStateID();
					const int idTwo = (*adItr)->getStateID();
					if (idOne < idTwo) {
						++apItr;
					} else if (idTwo < idOne) {
						++adItr;
					} else {
						theIntersection.insert(idOne);
						++apItr;
						++adItr;
					}
				};
			}

			list<int> toWatch;

			{
				set<int>::iterator adItr = theIntersection.begin();
				const set<int>::iterator adEnd = theIntersection.end();

				for (; adItr != adEnd; ++adItr) {
					const int currLitID = (*adItr);
					if (effectsToActions[currLitID].empty()) {
						toWatch.push_back(currLitID);		
					} else if (oneShotLiterals[currLitID]) {
						toWatch.push_back(currLitID);		
					} else {
//						cout << "\t" << *(getLiteral(*adItr)) << " is added by actions, and isn't one-shot\n";
					}
				}

			}

			if (!toWatch.empty()) {
				//cout << "Have shown that " << *(instantiatedOps[i]) << " is one-shot\n";
				kShotFormulae[i] = new OneShotKShotFormula(toWatch);
			}

		}

		if (!kShotFormulae[i]) {
	// next case
	// - precondition says v > c where c is constant, probably 0 in any reasonable encoding, but may as well generalise
	// - v is strictly decreasing (ask TIM)
	// - the action decreases v by some amount d, where d is constant or strictly increasing (again, ask TIM)
	// - and thus, k = a formula (v - c) / d
	// this is weaker than the previous case, as the previous determines a bound of 1 or 0 depending on logical information
	// rather than k based on numeric information

	//actionsToRPGNumericStartPreconditions = vector<list<int> >(opCount);
	//actionsToRPGNumericInvariants = vector<list<int> >(opCount);
	//actionsToRPGNumericEndPreconditions = vector<list<int> >(opCount);


			list<int> candidatePreconditions;

			{
				for (int pass = 0; pass < 3; ++pass) {
					list<int> & currNP = (pass == 0 ? actionsToRPGNumericStartPreconditions[i] : (pass == 1 ? actionsToRPGNumericInvariants[i] : actionsToRPGNumericEndPreconditions[i]));
					list<int>::iterator npItr = currNP.begin();
					const list<int>::iterator npEnd = currNP.end();
					
					for (; npItr != npEnd; ++npItr) {
						RPGNumericPrecondition & currPrec = rpgNumericPreconditions[*npItr];

						if (currPrec.RHSVariable != -1) break;

						const int LHSVariable = currPrec.LHSVariable;
						if (LHSVariable == -1) break;
						if (LHSVariable >= fluentCount) break; 
						
						candidatePreconditions.push_back(*npItr);
						
					}

					
				}


			}
			
			list<ShotCalculator> shotFormulae; // more than one, take min over

			{

                list<int>::iterator cpItr = candidatePreconditions.begin();
                const list<int>::iterator cpEnd = candidatePreconditions.end();

                for (; cpItr != cpEnd; ++cpItr) {

                    RPGNumericPrecondition & currPrec = rpgNumericPreconditions[*cpItr];

                    const double RHSConstant = currPrec.RHSConstant;
                    const int LHSVariable = currPrec.LHSVariable;

                    bool foundDecreaser = false;
                    double decreaseBy = 0.0;

                    for (int pass = 0; pass < 2; ++pass) {
                        list<NumericEffect> & currNE = (pass == 0 ? actionsToStartNumericEffects[i] : actionsToEndNumericEffects[i]);
                        list<NumericEffect>::iterator neItr = currNE.begin();
                        const list<NumericEffect>::iterator neEnd = currNE.end();

                        for (; neItr != neEnd; ++neItr) {

                            if (neItr->fluentIndex == LHSVariable) {

                                double decVal;

                                if (constOnlyCalculate(neItr->formula, initialFluents, decVal)) {
                                    if (neItr->op == VAL::E_DECREASE){
                                        if (decVal > 0) {

                                            if (!foundDecreaser) {
                                                decreaseBy = decVal;
                                                foundDecreaser = true;
                                            } else {
                                                decreaseBy += decVal; // must have a start then end dec effect
                                            }
                                        }
                                    } else if (neItr->op == VAL::E_INCREASE){
                                        if (decVal < 0) {
                                            
                                            if (!foundDecreaser) {
                                                decreaseBy = -decVal;
                                                foundDecreaser = true;
                                            } else {
                                                decreaseBy -= decVal; // must have a start then end dec effect
                                            }
                                        }
                                    }

                                }

                            }

                        }

                    }

                    if (foundDecreaser && decreaseBy > 0.000000001) {

                        PNE* const currPNE = getPNE(LHSVariable);

                        if (EFT(currPNE->getHead())->onlyGoingDown()) {
                            /*cout << "Have shown that " << *(instantiatedOps[i]) << " is k-shot\n where ";
                            if (RHSConstant) {
                                cout << "k = ceil(" << *(currPNE) << " - " << (RHSConstant - decreaseBy) << ")";
                            } else {
                                cout << "k = " << *(currPNE);
                            }
                            if (decreaseBy != 1.0) {
                                cout << " / " << decreaseBy << "\n";
                            } else {
                                cout << "\n";
                            }*/
                            shotFormulae.push_back(ShotCalculator(LHSVariable, (RHSConstant - decreaseBy), decreaseBy));

                        }

                    }

                }

            }

			if (!shotFormulae.empty()) {
				kShotFormulae[i] = new KShotKShotFormula(shotFormulae);
			}

		}

		// otherwise, if we can't prove anything useful, it's unlimited

		if (!kShotFormulae[i]) {
			kShotFormulae[i] = new UnlimitedKShotFormula();
		} else {
//			cout << "Have an analysis now - says limit is " << kShotFormulae[i]->getLimit(refState) << "\n";
		}

		if (kShotFormulae[i]->getLimit(refState) <= 1) {

			{
				list<Literal*> & currEffectsList = actionsToStartEffects[i];

				list<Literal*>::iterator effItr = currEffectsList.begin();
				const list<Literal*>::iterator effEnd = currEffectsList.end();
			
			
				for (; effItr != effEnd; ++effItr) {
//					cout << *(*effItr) << " is maybe one shot\n";
					maybeOneShotLiteral.insert(*effItr);
				}

			}


			{
				list<Literal*> & currEffectsList = actionsToEndEffects[i];

				list<Literal*>::iterator effItr = currEffectsList.begin();
				const list<Literal*>::iterator effEnd = currEffectsList.end();

				for (; effItr != effEnd; ++effItr) {
//					cout << *(*effItr) << " is maybe one shot\n";
					maybeOneShotLiteral.insert(*effItr);
				}


			}

		} else {
//			cout << " is > 1 shot, so can't do extra one-shot-literal-esque inference\n";
		}

	}



}

void RPGBuilder::doSomeUsefulMetricRPGInference() {

	const int opCount = instantiatedOps.size();
	const int litCount = literals.size();

	kShotFormulae = vector<KShotFormula*>(opCount);

	oneShotLiterals = vector<bool>(litCount);
	for (int i = 0; i < litCount; ++i) oneShotLiterals[i] = false;
	for (int i = 0; i < opCount; ++i) kShotFormulae[i] = 0;

	LiteralSet initialState;
	vector<double> initialFluents;

	getInitialState(initialState, initialFluents);

	const int fluentCount = initialFluents.size();
	
	MinimalState refState;
	{
		LiteralSet::iterator isItr = initialState.begin();
		const LiteralSet::iterator isEnd = initialState.end();
		for (;isItr != isEnd; ++isItr) {
            refState.first.insert((*isItr)->getStateID());
        }
	}
	refState.second = initialFluents;

	LiteralSet maybeOneShotLiteral;

	for (int i = 0; i < opCount; ++i) {		
		if (!rogueActions[i]) {
//			cout << "Considering whether " << *(getInstantiatedOp(i)) << " is one shot\n";
			kshotInferForAction(i, refState, maybeOneShotLiteral, initialFluents, fluentCount);
		}
	};

	set<int> revisit;

	while (!maybeOneShotLiteral.empty()) {

		revisit.clear();

		LiteralSet::iterator litItr = maybeOneShotLiteral.begin();
		const LiteralSet::iterator litEnd = maybeOneShotLiteral.end();

		for (; litItr != litEnd; ++litItr) {

			const int lID = (*litItr)->getStateID();

			list<pair<int, VAL::time_spec> > & eta = effectsToActions[lID];

			if (eta.size() == 1 && refState.first.find(lID) == refState.first.end()) { //

				const int actID = eta.front().first;
				if (kShotFormulae[actID]->getLimit(refState) <= 1) {
					oneShotLiterals[lID] = true;
					//cout << "Have shown that literal " << *(*litItr) << " is one shot: only added by " << *(getInstantiatedOp(actID)) << "\n";
					list<pair<int, VAL::time_spec> >::iterator depItr = preconditionsToActions[lID].begin();
					const list<pair<int, VAL::time_spec> >::iterator depEnd = preconditionsToActions[lID].end();

					for (; depItr != depEnd; ++depItr) {
						revisit.insert(depItr->first);
					}


				} else {
//					cout << *(*litItr) << " isn't one-shot: the single achieving action, " << *(getInstantiatedOp(actID)) << ", can be applied " << kShotFormulae[actID]->getLimit(refState) << " times\n";
				}
			} else {
//				if (eta.size() > 1) cout << *(*litItr) << " isn't one shot: is added by " << eta.size() << " actions\n";
			}
			
		}

		maybeOneShotLiteral.clear();

		set<int>::iterator riItr = revisit.begin();
		const set<int>::iterator riEnd = revisit.end();

		for (; riItr != riEnd; ++riItr) {
			if (!rogueActions[*riItr]) {
//				cout << "Revisiting " << *(getInstantiatedOp(*riItr)) << " due to one-shot literal\n";
				kshotInferForAction(*riItr, refState, maybeOneShotLiteral, initialFluents, fluentCount);
			}
		}
	}

};

bool RPGBuilder::isInteresting(const int & act, set<int> & facts, map<int, int> & started) {
	map<int, int>::iterator intrItr = uninterestingnessCriteria.find(act);
	if (intrItr == uninterestingnessCriteria.end()) return true;
	const int criterion = intrItr->second;
	if (criterion == -1) return false;
	if (facts.find(criterion) != facts.end()) return false;
	if (started.find(act) != started.end()) return false;
	
	return true;
};

void RPGBuilder::pruneIrrelevant(const int & operatorID)
{

    rogueActions[operatorID] = true;

    {
        list<Literal*> & currEffectsList = actionsToStartEffects[operatorID];

        list<Literal*>::iterator effItr = currEffectsList.begin();
        const list<Literal*>::iterator effEnd = currEffectsList.end();


        for (; effItr != effEnd; ++effItr) {
            const int effID = (*effItr)->getStateID();
            effectsToActions[effID].remove(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));
        }
        currEffectsList.clear();

    }
    

    {

        list<Literal*> & currPreconditionsList = actionsToStartPreconditions[operatorID];

        list<Literal*>::iterator precItr = currPreconditionsList.begin();
        const list<Literal*>::iterator precEnd = currPreconditionsList.end();

        for (; precItr != precEnd; ++precItr) {
            const int precID = (*precItr)->getStateID();
            preconditionsToActions[precID].remove(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));

        }
        currPreconditionsList.clear();
    }


    

    {
        list<Literal*> & newStartPrecs = actionsToProcessedStartPreconditions[operatorID];
        if (newStartPrecs.empty()) {
            preconditionlessActions.remove(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));
        }

        list<Literal*>::iterator precItr = newStartPrecs.begin();
        const list<Literal*>::iterator precEnd = newStartPrecs.end();

        for (; precItr != precEnd; ++precItr) {
            const int precID = (*precItr)->getStateID();
            processedPreconditionsToActions[precID].remove(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));
        }

        newStartPrecs.clear();

    }

    if (!actionsToRPGNumericStartPreconditions.empty()) {
        list<int> & currPreconditionsList = actionsToRPGNumericStartPreconditions[operatorID];

        list<int>::iterator precItr = currPreconditionsList.begin();
        const list<int>::iterator precEnd = currPreconditionsList.end();
        for (; precItr != precEnd; ++precItr) {
            const int precID = *precItr;
            rpgNumericPreconditionsToActions[precID].remove(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));
        }
        currPreconditionsList.clear();
    }

    
    if (!actionsToProcessedStartRPGNumericPreconditions.empty()) {
        list<int> & currPreconditionsList = actionsToProcessedStartRPGNumericPreconditions[operatorID];

        list<int>::iterator precItr = currPreconditionsList.begin();
        const list<int>::iterator precEnd = currPreconditionsList.end();
        for (; precItr != precEnd; ++precItr) {
            const int precID = *precItr;
            processedRPGNumericPreconditionsToActions[precID].remove(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));
        }

        currPreconditionsList.clear();
    }



    if (!actionsToRPGNumericStartEffects.empty()) {
        list<int> & currEffList = actionsToRPGNumericStartEffects[operatorID];

        list<int>::iterator effItr = currEffList.begin();
        const list<int>::iterator effEnd = currEffList.end();
        for (; effItr != effEnd; ++effItr) {
            rpgNumericEffectsToActions[*effItr].remove(pair<int, VAL::time_spec>(operatorID, VAL::E_AT_START));
        }

        currEffList.clear();
    }

    


}

};
