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

#ifndef __RPGBUILDER
#define __RPGBUILDER

#include <vector>
#include <list>
#include <set>
#include <map>

using std::vector;
using std::list;
using std::set;
using std::map;

#include "instantiation.h"
#include "ptree.h"

#include <assert.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "globals.h"

using namespace Inst;

namespace Planner {


class SubproblemRPG;
class StartEvent;

struct EpsilonComp {

	bool operator()(const double & a, const double & b) const {
		if (fabs(b-a) < 0.0005) return false;
		return (a < b);
	}

};

class MinimalState;
class PreferenceData;

struct ActionFluentModification {

	int act;
	VAL::time_spec ts;
	bool openEnd;
	double change;
	double howManyTimes;
	bool assignment;

	ActionFluentModification* next;
	
	ActionFluentModification(const int & a, const VAL::time_spec & startOrEnd, const bool & oe, const double & incr, const double & shots, const bool & ass)
		: act(a), ts(startOrEnd), openEnd(oe), change(incr), howManyTimes(shots), assignment(ass), next(0) {};

};

struct ActionAndHowManyTimes {
    instantiatedOp* first;
    VAL::time_spec second;
    double howManyTimes;
    
    ActionAndHowManyTimes()
        : first(0), howManyTimes(0.0) {
    }
    
    ActionAndHowManyTimes(instantiatedOp * f, const VAL::time_spec & s, const double & t)
        : first(f), second(s), howManyTimes(t) {
    }
};

enum dominance_constraint { E_NODOMINANCE, E_IRRELEVANT, E_METRIC, E_SMALLERISBETTER, E_BIGGERISBETTER};

enum cost_propagation { E_NOCOSTS = 0, E_MAXCOST = 1, E_SUMCOST = 2 };

class LiteralAnalysis;
class NNFUtils;
class PreferenceHandler;
class PreferenceFSA;

class RPGBuilder {

public:
	enum math_op { NE_ADD, NE_SUBTRACT, NE_MULTIPLY, NE_DIVIDE, NE_CONSTANT, NE_FLUENT, NE_VIOLATION};

	struct Operand {
		math_op numericOp;
		int fluentValue;
		double constantValue;
		string isviolated;
		Operand(const math_op & o) : numericOp(o), fluentValue(-1), constantValue(NAN) {};
		Operand(const int & f) : numericOp(NE_FLUENT), fluentValue(f), constantValue(NAN) {};
		Operand(const double & c) : numericOp(NE_CONSTANT), fluentValue(-1), constantValue(c) {};
		Operand(const string & s) : numericOp(NE_VIOLATION), fluentValue(-1), constantValue(NAN), isviolated(s) {};
	};

	static double calculateRHS(const list<Operand> & formula, vector<double> & fluents);

	class NumericEffect {
	
	public:
		
		int fluentIndex;
		VAL::assign_op op; // how to update given fluent index (add to it, assign to it, etc.)
		list<Operand> formula; // formula in postfix notation
		/** @brief If <code>false</code>, the formula refers to undefined functions. */
        bool valid;
		NumericEffect(const VAL::assign_op & opIn, const int & fIn, VAL::expression * formulaIn, VAL::FastEnvironment * f,VAL::TypeChecker * t = 0);
		double applyEffect(vector<double> & fluents) const;
		void display(ostream & o) const;
	};

	class NumericPrecondition {
	
	public:
		VAL::comparison_op op; // how to compare given fluent index to calculated RHS
		list<Operand> LHSformula; // formula for LHS in postfix notation
		list<Operand> RHSformula; // formula for RHS postfix notation
        /** @brief If <code>false</code>, the formula refers to undefined functions. */
        bool valid;
                        
		NumericPrecondition(const VAL::comparison_op & opIn, VAL::expression * LHSformulaIn, VAL::expression * RHSformulaIn, VAL::FastEnvironment * f,VAL::TypeChecker * t = 0);
		bool isSatisfied(vector<double> & fluents) const;
		void display(ostream & o) const;

		double evaluateRHS(vector<double> & fluentTable) const;
	};

	class ArtificialVariable {

	public:
		int ID;
		int size;
		vector<double> weights;
		vector<int> fluents;
		double constant;

		double maxNeed;

		ArtificialVariable() : ID(-1), size(0), constant(0.0), maxNeed(-DBL_MAX) {};
		ArtificialVariable(const int & id, const int & s, const vector<double> & w, const vector<int> & f, const double & d, const double & maxIn) : ID(id), size(s), weights(w), fluents(f), constant(d), maxNeed(maxIn) {};

		double evaluate(const vector<double> & fluentTable) const {
			double toReturn = constant;
			for (int i = 0; i < size; ++i) {
                toReturn += weights[i] * fluentTable[fluents[i]];
            }
			return toReturn;
		};
		
		double evaluateWCalculate(const vector<double> & fluentTable, const int & pneCount);

		bool operator <(const ArtificialVariable & v) const;
		void display(ostream & o) const;

		void updateMax(const double & m) {
			if (m > maxNeed) maxNeed = m;
		}
	};
    class NoDuplicatePair {
        
    protected:
        
        list<Literal*> * first;
        LiteralSet * second;
        
    public:
        
        NoDuplicatePair()
            : first((list<Literal*>*)0), second((LiteralSet*)0)
        {
        }
                                    
        NoDuplicatePair(list<Literal*> * const listIn, LiteralSet* const setIn)
            : first(listIn), second(setIn)
        {
        }
                                
        void push_back(Literal* const toAdd) {
            if (second->insert(toAdd).second) {
                first->push_back(toAdd);
            }
        }        
                                                                                    
        Literal* back() const {
            return first->back();            
        }
                                                                                                    
        bool operator!() const {
            return (!first);
        }
                                                                                                                    
        template<typename T>
        void insert(T itr, const T & itrEnd)
        {
            for (; itr != itrEnd; ++itr) {
                push_back(*itr);
            }
        };
    };
    
	class RPGNumericPrecondition {

	public:
		int ID;

		int LHSVariable;
		double LHSConstant;

		VAL::comparison_op op;

		int RHSVariable;
		double RHSConstant;

		RPGNumericPrecondition() : ID(-1), LHSVariable(-1), LHSConstant(0.0), op(VAL::E_GREATEQ), RHSVariable(-1), RHSConstant(0.0) {};
		RPGNumericPrecondition(const int & i, const int & lv, const double & lw, const VAL::comparison_op & o, const double & rw) : ID(i), LHSVariable(lv), LHSConstant(lw), op(o), RHSVariable(-1), RHSConstant(rw) {};

		bool isSatisfied(vector<double> & maxFluents) const {
			if (op == VAL::E_GREATER) {
                return (maxFluents[LHSVariable] > RHSConstant);
			} else {
                return (maxFluents[LHSVariable] - RHSConstant >= -0.0000001);
			}
		};

        bool canBeUnsatisfied(vector<double> & maxFluents) const {
            static const int varCount = RPGBuilder::getPNECount();
            if (op == VAL::E_GREATER) {
                return (RHSConstant - maxFluents[LHSVariable + varCount] >= -0.0000001);
            } else {
                return (RHSConstant > maxFluents[LHSVariable + varCount]);
            }
        }
        
		bool isSatisfiedWCalculate(vector<double> & maxFluents) const;
        bool canBeUnsatisfiedWCalculate(vector<double> & maxFluents) const;
		
		bool operator <(const RPGNumericPrecondition & r) const;

		void display(ostream & o) const;

	};

	class RPGNumericEffect {

	public:
		int ID;

		int fluentIndex;

		bool isAssignment; // if it's an assignmentOp - assume it's a += otherwise

		vector<double> weights;
		vector<int> variables;

		double constant;

		int size;

		RPGNumericEffect() : ID(-1), fluentIndex(-1), isAssignment(false), constant(NAN), size(-1) {};

		RPGNumericEffect(const int & idIn, const int & fluent, const bool & ass,
				 const vector<double> & weightsIn, const vector<int> & vars, const int & s,
				 const double & con) :
				 ID(idIn), fluentIndex(fluent), isAssignment(ass),
				 weights(weightsIn), variables(vars), constant(con), size(s) {};

		double evaluate(const vector<double> & maxFluents) const {
			double toReturn = constant;
			for (int i = 0; i < size; ++i) {
				const double val = maxFluents[variables[i]];
				if (val == DBL_MAX) return DBL_MAX;
				if (val == -DBL_MAX) return -DBL_MAX;
				toReturn += weights[i] * val;
			}
			return toReturn;
		};

		bool operator <(const RPGNumericEffect & e) const;

	};

	class FakeTILAction {
	
	public:
		const double duration;
		list<Literal*> addEffects;
		list<Literal*> delEffects;

		void mergeIn(LiteralSet & adds, LiteralSet & dels) {
			{
				LiteralSet::iterator lsItr = adds.begin();
				const LiteralSet::iterator lsEnd = adds.end();
	
				for (; lsItr != lsEnd; ++lsItr) {
					addEffects.push_back(*lsItr);
				}
			}	
	
			{
				LiteralSet::iterator lsItr = dels.begin();
				const LiteralSet::iterator lsEnd = dels.end();
	
				for (; lsItr != lsEnd; ++lsItr) {
					delEffects.push_back(*lsItr);
				}
			}	
		};
	
		FakeTILAction(const double & d, LiteralSet & adds, LiteralSet & dels) : duration(d) {
			mergeIn(adds, dels);
		}


	
	};

	class KShotFormula {
	
	public:
		KShotFormula() {};
		virtual int getLimit(MinimalState & s) = 0;
        virtual int getOptimisticLimit(MinimalState & s) = 0;
		virtual ~KShotFormula() {};
	};

	class UnlimitedKShotFormula : public KShotFormula {

	public:
		UnlimitedKShotFormula() : KShotFormula() {};
		virtual int getLimit(MinimalState &) { return INT_MAX; };
        virtual int getOptimisticLimit(MinimalState &) { return INT_MAX; };
	};

	class OneShotKShotFormula : public KShotFormula {

	private:
		list<int> watchedLiterals;
	public:
		OneShotKShotFormula(list<int> & toWatch) : KShotFormula(), watchedLiterals(toWatch) {};
		virtual int getLimit(MinimalState & s);
        virtual int getOptimisticLimit(MinimalState & s);
	};

	struct ShotCalculator {
	
		int variable;
		double greaterThan;
		double decreaseBy;

		ShotCalculator(const int & v, const double & g, const double & d) : variable(v), greaterThan(g), decreaseBy(d) {};
	};

	class KShotKShotFormula : public KShotFormula {

	private:
		list<ShotCalculator> formulae;
	public:
		KShotKShotFormula(list<ShotCalculator> & c) : KShotFormula(), formulae(c) {};
		virtual int getLimit(MinimalState & s);
        virtual int getOptimisticLimit(MinimalState & s);
	};

	
	class Metric {

	public:
		bool minimise;

		list<double> weights;
		list<int> variables;

		Metric(const bool & m) : minimise(m) {};

	};
	
	static Metric * theMetric;
	static set<int> metricVars;

	class Preference {

        public:            
            friend class PreferenceFSA;
            friend class PreferenceHandler;
        
            #ifndef NDEBUG
            list<Literal*> debug_goal;
            list<Literal*> debug_trigger;
            
            list<pair<int,int> > debug_goalRPGNum;
            list<pair<int,int> > debug_triggerRPGNum;
            #endif
        public:
        
		string name;

		VAL::constraint_sort cons;
        
        VAL::FastEnvironment * fe;
        
        VAL::goal * parsed_goal;
        VAL::goal * parsed_trigger;
        
        PreferenceData * d;
        
		double cost;
		bool neverTrue;

        int attachedToOperator;
        
        
        Preference() : parsed_goal(0), parsed_trigger(0), d(0), cost(0.0), neverTrue(false), attachedToOperator(-1) {};
        Preference(const string & n) : name(n), parsed_goal(0), parsed_trigger(0), d(0), cost(0.0), neverTrue(false), attachedToOperator(-1) {};

	};

    class ProtoConditionalEffect {
        
        public:
            
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
            
            bool validEffects;
    };
	
protected:

    friend class LiteralAnalysis;
    friend class NNFUtils;
    friend class PreferenceHandler;
    
	static bool RPGdebug;
	static bool problemIsNotTemporal;
//	static vector<list<pair<int, double> > > actionsToNegativeNumericEffects;
//	static vector<list<pair<int, double> > > negativeNumericEffectsToActions;

	static vector<list<pair<int, VAL::time_spec> > > preconditionsToActions;
	
	static list<pair<int, VAL::time_spec> > preconditionlessActions;

    static vector<vector<int> > actionsToStartPreferences;
    static int taskPrefCount;
    static int preconditionPrefCount;
    
	static vector<list<Literal*> > actionsToStartPreconditions;
	static vector<list<Literal*> > actionsToInvariants;
	static vector<list<Literal*> > actionsToEndPreconditions;

	static vector<list<Literal*> > actionsToStartEffects;
	static vector<list<Literal*> > actionsToStartNegativeEffects;
	static vector<list<Literal*> > actionsToEndEffects;
	static vector<list<Literal*> > actionsToEndNegativeEffects;

	static vector<list<pair<int, VAL::time_spec> > > effectsToActions;
	static vector<list<pair<int, VAL::time_spec> > > negativeEffectsToActions;

	static vector<double> actionsToDurations;
	static vector<NumericPrecondition*> durationExpressions;
    static vector<list<ProtoConditionalEffect*> > actionsToRawConditionalEffects;
    
	static vector<list<NumericPrecondition> > actionsToStartNumericPreconditions;
	static vector<list<NumericPrecondition> > actionsToNumericInvariants;
	static vector<list<NumericPrecondition> > actionsToEndNumericPreconditions;

	static vector<list<NumericEffect> > actionsToStartNumericEffects;
	static vector<list<NumericEffect> > actionsToEndNumericEffects;


	static vector<list<int> > actionsToRPGNumericStartPreconditions;
	static vector<list<int> > actionsToRPGNumericInvariants;
	static vector<list<int> > actionsToRPGNumericEndPreconditions;

	static vector<list<int> > actionsToRPGNumericStartEffects;
	static vector<list<int> > actionsToRPGNumericEndEffects;

	
//	static vector<list<pair<int, double> > > numericPreconditionsToActions;
//	static vector<list<pair<int, double> > > actionsToNumericPreconditions;	

	static vector<int> initialUnsatisfiedStartPreconditions;
	static vector<int> initialUnsatisfiedInvariants;
	static vector<int> initialUnsatisfiedEndPreconditions;

	static vector<double> achievedInLayer;
	static vector<double> achievedInLayerReset;
	static vector<pair<int, VAL::time_spec> > achievedBy;
	static vector<pair<int, VAL::time_spec> > achievedByReset;

    static vector<double> negativeAchievedInLayer;
    static vector<double> negativeAchievedInLayerReset;
    static vector<pair<int, VAL::time_spec> > negativeAchievedBy;
    static vector<pair<int, VAL::time_spec> > negativeAchievedByReset;
    	
	static vector<double> numericAchievedInLayer;
	static vector<double> numericAchievedInLayerReset;
	static vector<ActionFluentModification*> numericAchievedBy;
	static vector<ActionFluentModification*> numericAchievedByReset;

    static vector<double> negativeNumericAchievedInLayer;
    static vector<double> negativeNumericAchievedInLayerReset;
    static vector<ActionFluentModification*> negativeNumericAchievedBy;
    static vector<ActionFluentModification*> negativeNumericAchievedByReset;
    
//	static vector<int> increasedInLayer;
//	static vector<pair<int, double> > increasedBy;
//	static vector<pair<int, double> > increasedReset;

	static vector<instantiatedOp*> instantiatedOps;
	
	static vector<Literal*> literals;
	static vector<PNE*> pnes;

    static bool initialisedNumericPreTable;
	static vector<RPGNumericPrecondition> rpgNumericPreconditions;
	static vector<RPGNumericEffect> rpgNumericEffects;
	static vector<list<pair<int, VAL::time_spec> > > rpgNumericEffectsToActions;
	static vector<list<pair<int, VAL::time_spec> > > rpgNumericPreconditionsToActions;

	static vector<list<int> > variablesToRPGNumericPreconditions;

	static vector<ArtificialVariable> rpgArtificialVariables;
	static vector<list<int> > rpgVariableDependencies;

	static vector<list<int> > rpgArtificialVariablesToPreconditions;
	static vector<list<int> > rpgNegativeVariablesToPreconditions;
	static vector<list<int> > rpgPositiveVariablesToPreconditions;

	static vector<int> initialUnsatisfiedNumericStartPreconditions;
	static vector<int> initialUnsatisfiedNumericInvariants;
	static vector<int> initialUnsatisfiedNumericEndPreconditions;

	static vector<list<pair<int, VAL::time_spec> > > processedPreconditionsToActions;
	static vector<list<Literal*> > actionsToProcessedStartPreconditions;
	static vector<int> initialUnsatisfiedProcessedStartPreconditions;
	static vector<list<int> > realVariablesToRPGEffects;

	static vector<list<int> > numericSubsumes;

	static vector<list<pair<int, VAL::time_spec> > > processedRPGNumericPreconditionsToActions;

	static vector<list<NumericPrecondition> > actionsToProcessedStartNumericPreconditions;

	static list<Literal*> literalGoals;	
	static list<NumericPrecondition> numericGoals;
	static list<pair<int, int> > numericRPGGoals;

	static vector<Preference> preferences;
	static map<string,list<int> > prefNameToID;
    static map<string,int> prefNameToNumberOfTimesDefinitelyViolated;
    static double permanentPreferenceViolations;


	static vector<list<int> > actionsToProcessedStartRPGNumericPreconditions;
	static vector<int> initialUnsatisfiedProcessedStartNumericPreconditions;

	static vector<list<int> > mentionedInFluentInvariants;

	static list<FakeTILAction> timedInitialLiterals;
	static vector<FakeTILAction*> timedInitialLiteralsVector;

	static vector<KShotFormula*> kShotFormulae;
	static vector<bool> selfMutexes;
	static vector<bool> oneShotLiterals;

	static vector<double> maxNeeded;
	static vector<dominance_constraint> dominanceConstraints;
	static map<int, int> uninterestingnessCriteria;
	
    struct BuildingNumericPreconditionData {
        
        set<ArtificialVariable> artificialVariableSet;
        map<RPGNumericPrecondition, list<pair<int, VAL::time_spec> > > rpgNumericPreconditionSet;
        const int negOffset;
        const int offset;
        int precCount;
        int avCount;
        vector<double> localMaxNeed;
        
        BuildingNumericPreconditionData() : negOffset(pnes.size()),offset(negOffset * 2), precCount(0), avCount(offset),localMaxNeed(offset, -DBL_MAX) {};
        
    };
    
    
	static void buildRPGNumericPreconditions();
	static void buildRPGNumericEffects();
	static void simplify(pair<list<double>, list<int> > & s);
	static void makeOneSided(pair<list<double>, list<int> > & LHSvariable, pair<list<double>, list<int> > & RHSvariable, const int & negOffset);
	static void makeWeightedSum(list<Operand> & formula, pair<list<double>, list<int> > & result);
    static bool processPreconditions(RPGBuilder::BuildingNumericPreconditionData & commonData,
                                      list<NumericPrecondition> & currPreList, list<int> & destList, int & toIncrement,
                                      const int & i, const VAL::time_spec & passTimeSpec);

	static void buildDurations(vector<NumericPrecondition*> & d);
	static void handleNumericInvariants();
	static void findSelfMutexes();
	static void kshotInferForAction(const int & i, MinimalState & refState, LiteralSet & maybeOneShotLiteral, vector<double> & initialFluents, const int & fluentCount);
	static void doSomeUsefulMetricRPGInference();
	static pair<bool,int> isBiggerBetter(const int & var, const int & weight);
	static void findDominanceConstraints();
	static void findUninterestingnessCriteria();
    static void foldInConditionalEffects();
    static void postFilterIrrelevantActions();
    
    /** @brief Remove the given ground action. */
    static void pruneIrrelevant(const int & operatorID);


public:
		
	static vector<bool> rogueActions;
	static bool modifiedRPG;
    static bool useMetricRPG;
	static bool planAnyway;
    static bool doNotApplyActionsThatLookTooExpensive;
    static cost_propagation costPropagationMethod;
    static bool useTheCostsInTheLP;
    static bool switchInCheaperAchievers;
    static bool postFilter;
    
    static unsigned int statesEvaluated;
    
	static void initialise();

    static const map<string,int> & getPermanentViolationDetails()
    {
        return prefNameToNumberOfTimesDefinitelyViolated;
    }
    static const double & getPermanentViolationCost()
    {
        return permanentPreferenceViolations;
    }
    
	static bool nonTemporalProblem() { return problemIsNotTemporal; };

	static SubproblemRPG* pruneRPG(list<Literal*> & goals, list<int> & goalFluents, LiteralSet & initialState, vector<double> & initialFluents);
    
	static void getInitialState(LiteralSet & initialState, vector<double> & initialFluents);
    static void getAllInitialLiterals(LiteralSet & initialState);
	static instantiatedOp* getInstantiatedOp(const int & i) { return instantiatedOps[i]; };
	static Literal* getLiteral(const int & i) { return literals[i]; };
	static list<FakeTILAction> & getTILs() { return timedInitialLiterals; };
    
	static vector<list<pair<int, VAL::time_spec> > > & getEffectsToActions()
	{
		return effectsToActions;
	}
    
    static const vector<list<pair<int, VAL::time_spec> > > & getNegativeEffectsToActions()
    {
        return negativeEffectsToActions;
    }
    
    static const vector<vector<int> > & getStartPreferences()
    {
        return actionsToStartPreferences;
    }
    
    static const vector<list<Literal*> > & getProcessedStartPreconditions()
    {
        return actionsToProcessedStartPreconditions;
    }
    
    static const vector<list<Literal*> > & getStartAddEffects()
    {
        return actionsToStartEffects;
    }
    
    static const vector<list<Literal*> > & getStartDeleteEffects()
    {
        return actionsToStartNegativeEffects;
    }
    
    static const vector<list<pair<int, VAL::time_spec> > > & getProcessedRPGNumericPreconditionsToActions()
    {
        return processedRPGNumericPreconditionsToActions;
    }
    
    static const vector<list<pair<int, VAL::time_spec> > > & getProcessedPreconditionsToActions()
    {
        return processedPreconditionsToActions;
    }
    
    static const vector<int> & getUnsatisfiedStartPreconditionCounts()
    {
        return initialUnsatisfiedProcessedStartPreconditions;
    }
    
    static const vector<int> & getUnsatisfiedStartNumericPreconditionCounts()
    {
        return initialUnsatisfiedProcessedStartNumericPreconditions;
    }
    
    static const list<pair<int, VAL::time_spec> > & getPreconditionlessActions()
    {
        return preconditionlessActions;
    }
        
    
	static vector<FakeTILAction*> & getTILVec() { return timedInitialLiteralsVector; };

	static void getEffects(instantiatedOp* op, const bool & start, list<Literal*> & add, list<Literal*> & del, list<NumericEffect> & numeric); 
	static void getPrecInv(instantiatedOp* op, const bool & start, list<Literal*> & precs, list<Literal*> & inv, list<NumericPrecondition> & numericPrec, list<NumericPrecondition> & numericInv); 
	// static void getCollapsedAction(instantiatedOp* op, list<Literal*> & pre, list<Literal*> & add, list<Literal*> & del, list<NumericPrecondition> & numericPre, list<NumericEffect> & numericEff); 
	
	static list<int> & getMentioned(const int & i) { return mentionedInFluentInvariants[i]; };

	static double getOpDuration(instantiatedOp* op);
	static double getOpDuration(const int & op);

	static pair<double, bool> getOpDuration(instantiatedOp* op, vector<double> & fluents);
	static pair<double, bool> getOpDuration(const int & op, vector<double> & fluents);

    static const vector<RPGNumericPrecondition> & getNumericPreTable() {
        assert(initialisedNumericPreTable);
        return rpgNumericPreconditions;
    }

    static const int & getTaskPrefCount() {
        return taskPrefCount;
    }
    
    static const int & getPreconditionPrefCount() {
        return preconditionPrefCount;
    }
        
            
    static vector<RPGNumericPrecondition> & getModifiableNumericPreTable() { return rpgNumericPreconditions; };
    static vector<RPGNumericEffect> & getNumericEff() { return rpgNumericEffects; };

	static PNE* getPNE(const int & i) { return pnes[i]; };
	static int getPNECount() { return pnes.size(); };
    static int getLiteralCount() { return preconditionsToActions.size(); };
	static int getAVCount() { return rpgArtificialVariables.size(); };
	static ArtificialVariable & getArtificialVariable(const int & i) { return rpgArtificialVariables[i - (2 * getPNECount())]; };

	static list<int> & getVariableDependencies(const int & i) { return rpgVariableDependencies[i]; };
	static list<int> & affectsRPGNumericPreconditions(int i) { 

		static const int off = getPNECount();
		if (i < off) {
			return rpgPositiveVariablesToPreconditions[i];
		};
		i -= off;
		if (i < off) {
			return rpgNegativeVariablesToPreconditions[i];
		}
		i -= off;
		return rpgArtificialVariablesToPreconditions[i];

	}
	

	static int howManyTimes(const int & actID, MinimalState & e) { return kShotFormulae[actID]->getLimit(e); };
    static int howManyTimesOptimistic(const int & actID, MinimalState & e) { return kShotFormulae[actID]->getOptimisticLimit(e); };
    
	static bool isSelfMutex(const int & actID) { return selfMutexes[actID]; };
	static list<pair<int, int> > & getNumericRPGGoals() { return numericRPGGoals; };
	static list<NumericPrecondition> & getNumericGoals() { return numericGoals; };
	static list<Literal*> & getLiteralGoals() { return literalGoals; };
	static vector<dominance_constraint> & getDominanceConstraints() { return dominanceConstraints; };
	static bool isInteresting(const int & act, set<int> & facts, map<int, int> & started);

	static vector<RPGNumericEffect> & getNumericEffs() { return rpgNumericEffects; };
	static vector<RPGNumericPrecondition> & getNumericPrecs() { return rpgNumericPreconditions; };
	static vector<list<int> > & getActionsToRPGNumericStartEffects() { return actionsToRPGNumericStartEffects; };
    static const vector<list<int> > & getActionsToRPGNumericStartPreconditions() { return actionsToRPGNumericStartPreconditions; };
    static const vector<list<pair<int, VAL::time_spec> > > & getRpgNumericEffectsToActions()
    {
        return rpgNumericEffectsToActions;
    }
	static vector<Preference> & getPreferences() { return preferences; };
};

class MinimalState {

public:

	set<int> first;
	vector<double> second;
	map<int, int> startedActions;
	map<int, int> invariants;
	map<int, int> fluentInvariants;
    vector<AutomatonPosition> preferenceStatus;
    vector<bool> landmarkStatus;
    double prefPreconditionViolations;
    double cost;
    
    MinimalState(const set<int> & f, const vector<double> & s, const map<int, int> & sa, const map<int, int> & ia, const map<int, int> & lc, const vector<AutomatonPosition> & ps, const vector<bool> & ls, const double & ppv, const double & sc)
	: first(f), second(s), startedActions(sa), invariants(ia), fluentInvariants(lc),
	  preferenceStatus(ps), landmarkStatus(ls), prefPreconditionViolations(ppv), cost(sc) {};
	MinimalState() : prefPreconditionViolations(0.0) {};
	virtual ~MinimalState() {};

	virtual bool operator==(const MinimalState & o) const {
		return (first == o.first && second == o.second && startedActions == o.startedActions
                && invariants == o.invariants && fluentInvariants == o.fluentInvariants
                && preferenceStatus == o.preferenceStatus && landmarkStatus == o.landmarkStatus
                && prefPreconditionViolations == o.prefPreconditionViolations && cost == o.cost);
	}

};


class NNF_Flat;

class SubproblemRPG {

    public:
	
    struct RPGRegress {
        
        map<int, pair<int, double> > propositionalGoals;
        map<int, double> numericGoals;
        
        
    };
    
	struct FactLayerEntry {

		pair<set<int>, set<int> > * endOfJustApplied;
		list<int> first;
		list<int> second;
        
        list<int> negativeLiterals;
        list<int> negativeNumerics;
		
		FactLayerEntry() : endOfJustApplied(0) {};
	};

    
    struct ActionViolationData {
        set<int> preconditionsMeanViolating;
        double precViolationCost;
        
        set<int> effectsMeanViolating;
        double effViolationCost;
        
        set<int> overallViolations;
        double overallViolationCost;
        
        bool canBeApplied;
        
        ActionViolationData()
            : precViolationCost(-1.0), effViolationCost(-1.0), overallViolationCost(-1.0), canBeApplied(false)
        {
        }
    };
    
    private:
        
	vector<list<Literal*> > * const actionsToStartEffects;
	vector<list<Literal*> > * const actionsToEndEffects;
	vector<list<pair<int, VAL::time_spec> > > * const effectsToActions;

	vector<list<Literal*> > * const actionsToStartNegativeEffects;
	vector<list<Literal*> > * const actionsToEndNegativeEffects;
	vector<list<pair<int, VAL::time_spec> > > * const negativeEffectsToActions;

	
	vector<list<pair<int, VAL::time_spec> > > * const preconditionsToActions;
	
	vector<list<Literal*> > * const actionsToStartPreconditions;
	vector<list<Literal*> > * const actionsToInvariants;
	vector<list<Literal*> > * const actionsToEndPreconditions;

	vector<list<RPGBuilder::NumericEffect> > * const actionsToNumericStartEffects;
	vector<list<RPGBuilder::NumericEffect> > * const actionsToNumericEndEffects;
	
	vector<list<int> > * const actionsToRPGNumericStartEffects;
	vector<list<int> > * const actionsToRPGNumericEndEffects;
	
	vector<list<int> > * const actionsToNumericStartPreconditions;
	vector<list<int> > * const actionsToNumericInvariants;
	vector<list<int> > * const actionsToNumericEndPreconditions;
	vector<list<int> > * const actionsToProcessedStartNumericPreconditions;

	vector<int> * const initialUnsatisfiedStartPreconditions;
	vector<int> * const initialUnsatisfiedInvariants;
	vector<int> * const initialUnsatisfiedEndPreconditions;

	vector<double> * const achievedInLayer;
	vector<double> * const achievedInLayerReset;
	vector<pair<int, VAL::time_spec> > * const achievedBy;
	vector<pair<int, VAL::time_spec> > * const achievedByReset;

    vector<double> * const negativeAchievedInLayer;
    vector<double> * const negativeAchievedInLayerReset;
    vector<pair<int, VAL::time_spec> > * const negativeAchievedBy;
    vector<pair<int, VAL::time_spec> > * const negativeAchievedByReset;
        
	vector<double> * const numericAchievedInLayer;
	vector<double> * const numericAchievedInLayerReset;
	vector<ActionFluentModification*> * const numericAchievedBy;
	vector<ActionFluentModification*> * const numericAchievedByReset;
    
    vector<double> * const negativeNumericAchievedInLayer;
    vector<double> * const negativeNumericAchievedInLayerReset;
    vector<ActionFluentModification*> * const negativeNumericAchievedBy;
    vector<ActionFluentModification*> * const negativeNumericAchievedByReset;
	
	vector<int> * const initialUnsatisfiedNumericStartPreconditions;
	vector<int> * const initialUnsatisfiedNumericInvariants;
	vector<int> * const initialUnsatisfiedNumericEndPreconditions;

	vector<RPGBuilder::RPGNumericPrecondition> * const rpgNumericPreconditions;
	vector<RPGBuilder::RPGNumericEffect> * const rpgNumericEffects;

	vector<list<pair<int, VAL::time_spec> > > * const processedPreconditionsToActions;
	vector<list<pair<int, VAL::time_spec> > > * const processedNumericPreconditionsToActions;
	
	vector<list<Literal*> > * const actionsToProcessedStartPreconditions;
	vector<int> * const initialUnsatisfiedProcessedStartPreconditions;
	vector<int> * const initialUnsatisfiedProcessedStartNumericPreconditions;

	list<pair<int, VAL::time_spec> > * const preconditionlessActions;
	list<pair<int, VAL::time_spec> > noLongerForbidden;
    
    
    /** @brief Cost of each proposition (as propagated by sum or max). */    
    vector<double> propositionCosts;
    
    /** @brief Cost of each proposition (as propagated by sum or max). */    
    vector<double> actionCosts;
    
    
    // For convenience, we keep pointers to the information provided by the PreferenceHandler class
    
    /**
     *  Pointer to the result of PreferenceHandler::getPreconditionsToPrefs();
     */
    const vector<list<LiteralCellDependency<pair<int,bool> > > >  * preconditionsToPrefs;

    /**
    *  Pointer to the result of PreferenceHandler::getNegativePreconditionsToPrefs();
    */        
    const vector<list<LiteralCellDependency<pair<int,bool> > > >  * negativePreconditionsToPrefs;
    
    /**
     *  Pointer to the result of PreferenceHandler::getNumericPreconditionsToPrefs();
     */    
    const vector<list<LiteralCellDependency<pair<int,bool> > > >  * numericPreconditionsToPrefs;

    /**
    *  Pointer to the result of PreferenceHandler::getNegativeNumericPreconditionsToPrefs();
    */    
    const vector<list<LiteralCellDependency<pair<int,bool> > > >  * negativeNumericPreconditionsToPrefs;
    
    
    /**
     *  Pointer to vector passed as a parameter to PreferenceHandler::getUnsatisfiedConditionCounts()
     *  in getRelaxedPlan().  This will be undefined outside the scope of that function.
     */    
    vector<vector<NNF_Flat*> > * unsatisfiedPreferenceConditions;
    
    /**
     *  Pointer to vector scoped to getRelaxedPlan(), containing the layer in which the goal/trigger
     *  (0/1) of the indexed preference became true.  This will be undefined outside that scope.
     */
    vector<vector<double> > * preferencePartBecameTrueInLayer;
    
    /** @brief Preference violation cost of using each fact.  Is a pointer to a vector scoped to getRelaxedPlan().
     */
    vector<list<PreferenceSetAndCost> > * preferenceViolationCostOfUsingFact;

    /** @brief Preference violations appearing as landmarks before a fact.  Is a pointer to a vector scoped to getRelaxedPlan().
    */
    vector<set<int> > * landmarkPreferenceViolationsBeforeFact;
    
    
    /** @brief Preferences violated by each action.  First part of pair: preconditions.  Second part of pair: itself.
     */
    vector<ActionViolationData> * preferenceViolationCostOfUsingAction;
    
    
    /** @brief Status of each preference. If it's unsatisfied in the RPG, it has to be unsatisfied in the LP, too.
     */
    vector<AutomatonPosition> * optimisticStatusOfEachPreference;
    
    /** @brief The cost of deleting a fact (from always constraints).  Set details which prefs would be broken.
     */
    map<int, set<int> > prefCostOfDeletingFact;
    
    /** @brief The cost of adding a fact (from sometime before/after constraints).
    */
    map<int, AddingConstraints > prefCostOfAddingFact;

    /** @brief The cost of changing a numeric value (from always constraints).
     * 
     *  The set details which prefs would be broken by a change of the key to the second map.
     */
    map<int, map<double, set<int> > > prefCostOfChangingNumberA;

    /** @brief The cost of changing a numeric value (from sometime before/after constraints).
     */
    map<int, map<double, AddingConstraints > > prefCostOfChangingNumberB;
    
    /** @brief A map from preferences to which (applicable) actions would violate them, were they to be applied.
     */
    map<int, set<int> > preferenceWouldBeViolatedByAction;
    
    /** @brief A map from actions to which preferences to satisfy before applying them.
     */
    map<int, list<pair<int,bool> > > preferencePartsToSatisfyBeforeAction;
    
	static vector<double> latestStartAllowed;
	static vector<double> latestEndAllowed;
	static vector<double> deadlineAtTime;
	static vector<double> earliestDeadlineRelevancyStart;
	static vector<double> earliestDeadlineRelevancyEnd;
    static vector<double> prefCosts;
    vector<bool> actionPreviouslyTooExpensive;
    double maxPermissibleActionCost;
    map<int, pair<int,double> > factsThatAreNowAffordable;
    map<int, pair<int,double> > negativeFactsThatAreNowAffordable;
    
    void importNowAffordableFact(map<double, FactLayerEntry, EpsilonComp> & factLayers,
                                 const set<int> & goals, int & unsatisfiedGoals,
                                 const bool skipFirstLayer=false);
	const bool deleteArrays;

	inline bool updateActionsForFactWithPenalties(const int & toPropagate, MinimalState & startState, vector<int> & startPreconditionCounts, vector<int> & numericStartPreconditionCounts, map<double, FactLayerEntry, EpsilonComp > & factLayer, map<double, vector<double>, EpsilonComp > & fluentLayers, map<double, map<int, list<ActionFluentModification> >, EpsilonComp> & fluentModifications, const double & factLayerTime, map<double, list<int>, EpsilonComp > & endActionsAtTime, vector<double> & startActionSchedule, set<int> & goals, const set<int>::iterator & gsEnd, set<int> & goalFluents, const set<int>::iterator & gfEnd, int & unsatisfiedGoals, int & unappearedEnds, map<int,int> & insistUponEnds, map<int, int> & forbiddenStart);
	inline bool updateActionsForNumericFactWithPenalties(const int & toPropagate, MinimalState & startState, vector<int> & startPreconditionCounts, vector<int> & numericStartPreconditionCounts, map<double, FactLayerEntry, EpsilonComp > & factLayer, map<double, vector<double>, EpsilonComp > & fluentLayers, map<double, map<int, list<ActionFluentModification> >, EpsilonComp> & fluentModifications, const double & factLayerTime, map<double, list<int>, EpsilonComp > & endActionsAtTime, vector<double> & startActionSchedule, set<int> & goals, const set<int>::iterator & gsEnd, set<int> & goalFluents, const set<int>::iterator & gfEnd, int & unsatisfiedGoals, int & unappearedEnds, map<int,int> & insistUponEnds, map<int, int> & forbiddenStart);

    void setInitialNegativePreconditionsOfPreferences(MinimalState & startState);
    void updatePreferencesForFact(MinimalState & startState, const uint & factID, const bool & isALiteral, const bool & polarity, const double&, double & lag, double & rpgGoalPrefViolation);
    void visitFactThatIsNowCheaper(const int & currAct, const int & currEff, const ActionViolationData & newCost,
                                   const double & factLayerTime, set<int> & actsVisited, list<int> & actsToVisit);
                                   
    void addPreconditionCost(ActionViolationData & costData, const int & currAct);
    void addEffectCost(ActionViolationData & costData, const int & currAct);
    bool applyOperator(const int & currAct, const double & factLayerTime, const double & nlTime,
                        const bool & updateDebug, vector<double> & startActionSchedule,
                        map<double, FactLayerEntry, EpsilonComp > & factLayer,
                        set<int> & goals, const set<int>::iterator & gsEnd,
                        int & unsatisfiedGoals, int & unappearedEnds);

    void processAnyCheaperFacts(const double & nlTime, double & maxLag);
    void addGoalsForPreferences(MinimalState & startState, map<double, RPGRegress, EpsilonComp> & goalsAtLayer,
                                const map<int,double> & propsSatisfied, const set<int> & prefsUnsatisfied);
    
    map<int,double> reducedCostFacts;
    
    double shiftFactLayersByLag(const double & currTS, const double & lag, map<double, FactLayerEntry, EpsilonComp > & factLayer, map<double, vector<double>, EpsilonComp > & fluentLayers, const bool excludeFirstLayer=false);
    

    void addExtraPreconditionsForPreferences(const int & act, const double & currTS, const double & fraction,
                                              const map<int,double> & propsSatisfied,
                                              map<double, RPGRegress, EpsilonComp> & goalsAtLayer);
    
public:
	


    SubproblemRPG(const bool & b,
                    vector<list<Literal*> > * atse, 
                    vector<list<Literal*> > * atee, 
                    vector<list<pair<int, VAL::time_spec> > > * eta,
                    vector<list<Literal*> > * atsne, 
                    vector<list<Literal*> > * atene, 
                    vector<list<pair<int, VAL::time_spec> > > * neta,
                    vector<list<pair<int, VAL::time_spec> > > * pta, 
                    vector<list<Literal*> > * atsp,
                    vector<list<Literal*> > * ati,
                    vector<list<Literal*> > * atep,
                    vector<list<RPGBuilder::NumericEffect> > * atnuse,
                    vector<list<RPGBuilder::NumericEffect> > * atnuee,
                    vector<list<int> > * atrnuse,
                    vector<list<int> > * atrnuee,
                    vector<list<int> > * atnusp,
                    vector<list<int> > * atnui,
                    vector<list<int> > * atnuep,
                    vector<list<int> > * atpnuep,
                    vector<int> * iusp,
                    vector<int> * iuip,
                    vector<int> * iuep,
                    vector<double> * ail,
                    vector<double> * ailr,
                    vector<pair<int, VAL::time_spec> > * ab,
                    vector<pair<int, VAL::time_spec> > * abr,
                    vector<double> * negail,
                    vector<double> * negailr,
                    vector<pair<int, VAL::time_spec> > * negab,
                    vector<pair<int, VAL::time_spec> > * negabr,                    
                    vector<double> * nail,
                    vector<double> * nailr,
                    vector<ActionFluentModification*> * nab,
                    vector<ActionFluentModification*> * nabr,
                    vector<double> * negnail,
                    vector<double> * negnailr,
                    vector<ActionFluentModification*> * negnab,
                    vector<ActionFluentModification*> * negnabr,                    
                    vector<int> * iunsp,
                    vector<int> * iuni,
                    vector<int> * iunep,
                    vector<RPGBuilder::RPGNumericPrecondition> * rnp,
                    vector<RPGBuilder::RPGNumericEffect> * rne,
                    vector<list<pair<int, VAL::time_spec> > > * ppta,
                    vector<list<pair<int, VAL::time_spec> > > * nppta,
                    vector<list<Literal*> > * atpsp,
                    vector<int> * iupsp,
                    vector<int> * iupsnp,
                    list<pair<int, VAL::time_spec> > * pla)
        : actionsToStartEffects(atse),
        actionsToEndEffects(atee),
        effectsToActions(eta),
        actionsToStartNegativeEffects(atsne),
        actionsToEndNegativeEffects(atene),
        negativeEffectsToActions(neta),
        preconditionsToActions(pta),
        actionsToStartPreconditions(atsp),
        actionsToInvariants(ati),
        actionsToEndPreconditions(atep),
        actionsToNumericStartEffects(atnuse),
        actionsToNumericEndEffects(atnuee),
        actionsToRPGNumericStartEffects(atrnuse),
        actionsToRPGNumericEndEffects(atrnuee),
        actionsToNumericStartPreconditions(atnusp),
        actionsToNumericInvariants(atnui),
        actionsToNumericEndPreconditions(atnuep),
        actionsToProcessedStartNumericPreconditions(atpnuep),
        initialUnsatisfiedStartPreconditions(iusp),
        initialUnsatisfiedInvariants(iuip),
        initialUnsatisfiedEndPreconditions(iuep),
        achievedInLayer(ail),
        achievedInLayerReset(ailr),
        achievedBy(ab),
        achievedByReset(abr),
        negativeAchievedInLayer(negail),
        negativeAchievedInLayerReset(negailr),
        negativeAchievedBy(negab),
        negativeAchievedByReset(negabr),		  		                                        
        numericAchievedInLayer(nail),
        numericAchievedInLayerReset(nailr),
        numericAchievedBy(nab),
        numericAchievedByReset(nabr),
        negativeNumericAchievedInLayer(negnail),
        negativeNumericAchievedInLayerReset(negnailr),
        negativeNumericAchievedBy(negnab),
        negativeNumericAchievedByReset(negnabr),                                                                  
        initialUnsatisfiedNumericStartPreconditions(iunsp),
        initialUnsatisfiedNumericInvariants(iuni),
        initialUnsatisfiedNumericEndPreconditions(iunep),
        rpgNumericPreconditions(rnp),
        rpgNumericEffects(rne),
        processedPreconditionsToActions(ppta),
        processedNumericPreconditionsToActions(nppta),
        actionsToProcessedStartPreconditions(atpsp),
        initialUnsatisfiedProcessedStartPreconditions(iupsp),
        initialUnsatisfiedProcessedStartNumericPreconditions(iupsnp),
        preconditionlessActions(pla),
        deleteArrays(b)
    {
        
        if (RPGBuilder::costPropagationMethod != E_NOCOSTS) {
            propositionCosts.resize(achievedInLayer->size(),0.0);
            actionCosts.resize(initialUnsatisfiedStartPreconditions->size(),0.0);
        }
        
    }

	~SubproblemRPG() {
		if (deleteArrays) {
			assert(false);
		}
	}

    struct EvaluationInfo {
        
        double first;
        double second;
        bool goalState;
        
        EvaluationInfo()
        : first(-1.0), second(DBL_MAX), goalState(false)
        {
        }
        
        EvaluationInfo(const double & a, const double & b, const bool & c)
        : first(a), second(b), goalState(c)
        {
        }
        
    };
	EvaluationInfo getRelaxedPlan(MinimalState & theState, const double & maxPrefCost, const double & stateTS, const int & nextTIL, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActions, list<pair<double, list<ActionAndHowManyTimes > > > & relaxedPlan, map<double, list<int> > * justApplied=0, double tilFrom=0.001);

	void findApplicableActions(MinimalState & theState, const int & nextTIL, list<pair<int, VAL::time_spec> > & applicableActions);
	void filterApplicableActions(MinimalState & theState, const int & nextTIL, list<pair<int, VAL::time_spec> > & applicableActions, list<pair<double,double> > * hhm=0);
	
	bool testApplicability(MinimalState & theState, const int & nextTIL, const pair<int, VAL::time_spec> & actID, bool fail=false, bool ignoreDeletes=false);

	list<Literal*> & getDeleteEffects(const int & i, const VAL::time_spec & t);
	list<Literal*> & getAddEffects(const int & i, const VAL::time_spec & t);
	list<Literal*> & getPreconditions(const int & i, const VAL::time_spec & t);
	list<Literal*> & getInvariants(const int & i);
	
	list<RPGBuilder::NumericEffect> & getNumericEffects(const int & i, const VAL::time_spec & t);

	list<instantiatedOp*> * makePlan(list<int> & steps);

	instantiatedOp* getOp(const int & i);
	double earliestTILForAction(const int & i, const bool & isStart);

	static double & getDeadlineRelevancyStart(const int & i) {
		return earliestDeadlineRelevancyStart[i];
	}

	static double & getDeadlineRelevancyEnd(const int & i) {
		return earliestDeadlineRelevancyEnd[i];
	}

	map<int, pair<double,double> > howManyTimesIsHelpful;
};

ostream & operator <<(ostream & o, const RPGBuilder::NumericPrecondition & p);
ostream & operator <<(ostream & o, const RPGBuilder::NumericEffect & p);

ostream & operator <<(ostream & o, const RPGBuilder::RPGNumericPrecondition & p);
ostream & operator <<(ostream & o, const RPGBuilder::ArtificialVariable & p);
ostream & operator <<(ostream & o, const RPGBuilder::RPGNumericEffect & p);

enum whereAreWe { PARSE_UNKNOWN, PARSE_PRECONDITION, PARSE_EFFECT, PARSE_DURATION, PARSE_GOAL, PARSE_INITIAL, PARSE_CONDITIONALEFFECT, PARSE_CONTINUOUSEFFECT, PARSE_METRIC, PARSE_DERIVATION_RULE, PARSE_CONSTRAINTS };

extern whereAreWe WhereAreWeNow;

void validatePNE(PNE * c);
void validateLiteral(Literal * l);

void postmortem_noNestedWhens();
void postmortem_noADL();
void postmortem_duplicateInitialStateValue(const PNE * const variable, const double previous, const double now);
void postmortem_nonLinearCTS(const string & actName, const string & worksOutAs);
void postmortem_noQuadratic(const string & theOp);
void postmortem_noTimeSpecifierOnAPropPrecondition(const string & actname, const string & effect);
void postmortem_fixedAndNotTimeSpecifiers(const string & actname, const bool & multipleEquals);
void postmortem_noTimeSpecifierOnAPropEffect(const string & actname, const string & effect);
void postmortem_noTimeSpecifierOnInstantNumericEffect(const string & actname, const string & effect, const string & suggested, const bool & isAssign);
void postmortem_wrongNumberOfFluentArguments(const string & actname, const bool & haveActName, const whereAreWe & w, const string & predname, const string & lit, const int & givenArgs, const set<int> & realargs);
void postmortem_wrongNumberOfPredicateArguments(const string & actname, const bool & haveActName, const whereAreWe & w, const string & predname, const string & lit, const int & givenargs, const set<int> & realargs);
void postmortem_mathsError(const string & description, const string & help, const whereAreWe & w);
void postmortem_noConstraints(const bool unsupportedPref=false, const char * n=0);
void postmortem_isViolatedNotExist(const string & s);
void postmortem_invalidMetric();
void postmortem_invalidGoal(const string & s);

};



#endif
