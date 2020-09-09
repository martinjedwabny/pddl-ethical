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

#ifndef MILPRPG_H
#define MILPRPG_H

#include "RPGBuilder.h"

#include <vector>
#include <map>
#include <list>

using std::vector;
using std::map;
using std::list;

class MILPSolver;

struct LPAndBounds {

    MILPSolver * lp;
    bool presolved;
    bool intVariables;
    vector<int> residualSwitch;
	vector<double> min;
	vector<bool> minReal;
	vector<double> max;
	vector<bool> maxReal;
	bool nowExtracting;
	list<int> varsToIntegral;

    LPAndBounds(MILPSolver * l, const bool & p, const bool & i, const vector<int> & ra, const int & v) : lp(l), presolved(p), intVariables(i), residualSwitch(ra), min(v), minReal(v), max(v), maxReal(v), nowExtracting(false) {};
	~LPAndBounds();

};

namespace Planner {
    
class MinimalState;

struct SwitchVarInfo {
  
    int hasBeenViolatedCol; // col. idx for switch var
    list<int> second; // rows for the condition it needs (for adding triggers to, if needed)
    int countingCol; // for precondition preferences: if violated, the number of times the action has been applied 
    int useInObjective;
    
    
    SwitchVarInfo()
        : hasBeenViolatedCol(-1), countingCol(-1), useInObjective(-1)
    {
    }
    
    SwitchVarInfo(const int & a, const bool taskPref=true)
        : hasBeenViolatedCol(a), countingCol(-1), useInObjective(taskPref ? hasBeenViolatedCol : -1)
    {
    }
    
    SwitchVarInfo(const int & a, const list<int> & b, const bool taskPref=true)
        : hasBeenViolatedCol(a), second(b), countingCol(-1), useInObjective(taskPref ? hasBeenViolatedCol : -1)
    {
    }
    
    void setCountColumn(const int & c)
    {
        countingCol = c;
        useInObjective = c;
    }
    
};

class MILPRPG {

public:
	struct ActData {
		int Avar;
        int switchVar;
		int kShot;
		//int Pvar;
        bool isBinary;
		ActData() : Avar(-1), switchVar(-1), kShot(-1), isBinary(false) {};
		ActData(const int & a, const int & sv, const int & p, const bool & b) : Avar(a), switchVar(sv), kShot(p), isBinary(b) {};
	};

	struct VarData {
		int Vvar;
		int consRow;
		int assignRow;
        int residualUB;
        int residualLB;
		double* maximisationGuess;
		double* minimisationGuess;
		int columnsInGuess;
		VarData() : Vvar(-1), consRow(-1), assignRow(-1), residualUB(-1), residualLB(-1), maximisationGuess(0), minimisationGuess(0), columnsInGuess(0) {};
		VarData(const int & v, const int & s): Vvar(v), consRow(s), maximisationGuess(0), minimisationGuess(0), columnsInGuess(0) {};
		~VarData() {
			delete [] maximisationGuess;
			delete [] minimisationGuess;
		}	
	};

	struct ActionEffectUponVariable {
		int var;
		double change;
		bool isAssignment;
		ActionEffectUponVariable() : var(-1), change(-1.0), isAssignment(false) {};
		ActionEffectUponVariable(const int & v, const double & c, const bool & a) : var(v), change(c), isAssignment(a) {};
	};

    /**
     * The extra numeric precondition check needed on an action, if its effects aren't enough.
     * For instance, if an action has a precondition (x >= 5) and effect (x -= 2), we need
     * to check that x >= 5 was tenable.
     */    
    struct ResidualPrecondition {
        mutable int ID;
        int var;
        double amount;
        VAL::comparison_op op;
        
        ResidualPrecondition() : ID(-1), var(-1), amount(-1.0)
        {            
        }
        
        ResidualPrecondition(const int & v, const double & c, const VAL::comparison_op & o)
        : var(v), amount(c), op(o)
        {
        }
                
        bool operator<(const ResidualPrecondition & other) const
        {
            
            if (var < other.var) return true;
            if (var > other.var) return false;
            if (var == -1 && other.var == -1) return false;
            
            if ((amount - other.amount) < -0.0001) return true;
            if ((amount - other.amount) > 0.0001) return false;
            
            return (op < other.op);
        }
    };


private:
	
	int varCount;
	int colCount;
	int layerCount;
	vector<pair<LPAndBounds*, LPAndBounds*> > allLp;
	map<double, int, EpsilonComp> timestampToIndex;
	set<int> applicableSoFar;
	set<int> potentiallyHelpful;
	vector<VarData> minMaxVars;
	vector<double*> guessVectors;
	map<int, ActData> layerActionVariables;
    vector<double> actionAppearedInLayer;
	map<int, pair<int, int> > boundedActs;
	MinimalState * startingState; 
    vector<SwitchVarInfo> switchVarForPreference;
	int switchVarCount;
    map<int, list<pair<int,int> > > preferenceTrips;
    
    map<int,pair<int,int> > bootstrapPropColumnAndRow;
  
    map<int,list<pair<int,int> > > literalToEffectOnRow;
    map<int,list<pair<int,int> > > negativeLiteralToEffectOnRow;
    
    pair<int,double> primaryObjectiveRowReset;
    
	static vector<bool> fluentAction;
	static vector<bool> everAssigned;
    static vector<bool> isInTheGoal;
    static vector<uint> everInAResidual;
	static vector<double> maxNeeded;
	static vector<double> minNeeded;
	static vector<list<ActionEffectUponVariable> > preprocessedData;
    static vector<list<int> > preprocessedResiduals;
    static vector<list<int> > preprocessedBootstrapPreconditions;
    static vector<list<int> > preprocessedBootstrapEffects;
    static vector<ResidualPrecondition> residualData;
    
	static int actCount;
    static int initialMatrixRows;
    static int initialMatrixCols;
    static int residualColumnCount;
    static int bootstrapPropCount;
	static vector<int> goesToRealAction;
	static vector<double> preferEarlier;
    
    struct GoalBound {
        double w;
        int v;
        int op;
        
        GoalBound()
        : v(-1)
        {
        };
        
        GoalBound(const double & wIn, const int & vIn, const int & lb)
        : w(wIn), v(vIn), op(lb)
        {
        };
    };
    
    /** Goals that can be expressed as simple bounds on variables */
    static vector<GoalBound> goalVarBounds;
    
    struct GoalConstraint {
        vector<pair<int,double> > lhs;
        int op;
        double rhs;
        
        GoalConstraint()
        {
        };
                
    };
    
    /** Goals that need a full (w.x [>=,<=] c) representation */
    static vector<GoalConstraint> goalConstraints;
    
    /** Action indices of achievers for the top-level propositional goals.
     * 
     * If the goal isn't true in the state being evaluated, one of these must hold
     * when checking if the goal can be reached.
     * 
     */
    static vector<vector<int> > propositionalGoalConstraints;
    
    /** Action indices of numeric-free achievers of the top-level propositional goals.
     * 
     * If one of these is present, don't add a constraint for the proposition - 
     * we can do it without disturbing the LP.
     * 
     */
    static vector<vector<int> > propositionalGoalNotEncoded;
    
    /** Indices of facts that are propositional goals.
     * 
     * These correspond with the propositionalGoalConstraints entries.
     * Negative entries indicate individual landmark fact, or disjunctive landmarks.
     * An entry of <code>-1 - k</code> denotes entry <code>k</code> in
     * <code>LandmarksAnalysis::getIndividualLandmarkFacts()</code>, up to an (exclusive) maximum of
     * <code>-1 - s</code>, where <code>s</code> is the size of that vector.  Beyond this point,
     * then when the integer encoding of disjunctive landmarks is not being used,
     * a negative index <code>-1 - k</code> denotes the disjunctive landmark group
     * at index <code>k - s</code> into <code>LandmarksAnalysis::getDisjunctiveLandmarks()</code>.
     */
    static vector<int> propositionalGoals;
    
    /** A mapping from a fact ID to the LP variable that denotes that it has been added. */
    map<int, int> lpVarToDenoteThatWeHaveAGivenFact;
    
    /** A mapping from each disjunctive landmark group member to the variable it takes. */
    vector<vector<int> > disjunctiveLandmarkGroupVar;
    
    /** Whether any actions in the LP can increase the given variable. */
    vector<bool> anyIncreasers;
    
    /** Whether any actions in the LP can decrease the given variable. */
    vector<bool> anyDecreasers;
    
    LPAndBounds* goalLP;

    set<int> factsThatWillBeConsideredAsGoals;
    
    bool mustAchieveFact(const int & g, const int & fID,
                         vector<int> * const propPrec, vector<int> * const numPrec,
                         map<int,double> & propsSatisfied, int & needToCheck);
    
    bool mustAchieveLocalFact(const int & fID,
                              vector<int> * const propPrec, vector<int> * const numPrec,
                              map<int,double> & propsSatisfied, int & needToCheck);

    bool addIntegerDisjunctiveLocalLandmarkEncoding(const int & disjunctiveLandmarkID, MinimalState * forKShot,
                                                    const list<set<int> > & landmarkOptions,
                                                    vector<int> * const propPrec, vector<int> * const numPrec,
                                                    int & needToCheck);
                              
    bool addIntegerDisjunctiveLandmarkEncoding(const int & disjunctiveLandmarkID, MinimalState * forKShot,
                                               vector<int> * const propPrec, vector<int> * const numPrec,
                                               int & needToCheck);

    bool addLinearDisjunctiveLandmarkEncoding(const int & disjunctiveLandmarkID, MinimalState * forKShot,
                                              vector<int> * const propPrec, vector<int> * const numPrec,
                                              int & needToCheck);
    
    
    /** A pointer to the costs to use for each action, or 0 if costs are managed locally.
     * 
     * If sum/max action costing is used, this is a pointer to the cost for each action,
     * as passed to the constructor.  Otherwise, 0 is passed to the constructor, indicating
     * that the costs for actions should be managed locally, using <code>layerFactor</code>.
     */
    const vector<double> * propositionalActionCosts;
    
public:
    
    /** The factor for the geometric series used to weight actions.
     * 
     * If sum/max action costing is not used, the weight given to each action variable 
     * in the objective (when satisfying a precondition or the goals) is set to this
     * variable, raised to the power of the action layer in which it first appeared.
     */ 
    static double layerFactor;
    
    
    
    static bool addProps;
    static bool addLandmarks;
    static bool useLocalLandmarks;
    static bool addNumGoalConjunct;
    static bool alternativeObjective;
    static int useParamILS;
    
    /** @brief The extent to which integer variables should be used in the LP.
     *
     * - 0 if none at all (giving a pure LP)
     * - 1 to make assignment actions integer (binary)
     * - 2 if to make potentially helpful actions integer
     * - 3 if to make potentially helpful actions and propositional goal-achieving actions integer
     * - 4 if to make all actions integer.
     */
    static int integerLevel;
    
    /** @brief Another flag to control the extent to which integer variables should be used in the LP.
    *
    * - 0 if no additional variables should be made integral
    * - 0 if goal-variable-affecting actions should be made integer
    */
    static int secondaryIntegerLevel;
    static bool recogniseBootstrappingPropositions;
    static bool ensureAllPropositionalPreconditionsAreMet;
    static bool residualEverything;
    static bool useSecondaryObjective;
    static bool integerDisjunctiveLandmarksEncoding;
    
    /** @brief  If <code>true</code>, presolving is never used when solving the LPs. */
    static bool neverUsePresolving;
    
    #ifndef NDEBUG
    set<int> canPutHelperInRow;
    #endif
    
	MILPRPG(MinimalState * forKShot, const vector<double> & layerZero, const vector<double> * const actionCosts);
	~MILPRPG();
    int giveMeAColumn(const string & n);
    int giveMeAnIntermediateColumn(const string &, const int&);
    void negativeLiteralAffectsRow(const int & i, const int & r, const int & e);
    void literalAffectsRow(const int & i, const int & r, const int & e);
    //pair<int,int> giveMeTriggerColumns(const int & prefIdx, const int & n, const bool & makeSecondaryTriggerCol);
    pair<int,int> giveMeATriggerColumn(const int & prefIdx, const bool & makeSecondaryTriggerCol);
    int getColumnForVar(const int & v);
    int getLBColumnForVar(const int & v);
    int getUBColumnForVar(const int & v);
    //int addSwitch(const string & ID, const vector<pair<int,double> > & entries, const double & rhs);
    int addSwitchB(const string & ID, const vector<pair<int,double> > & entries, const bool & positive, const double & rhs, const bool & spaceForHelper);
    void addTrigger(const string & ID, const vector<pair<int,double> > & entries, const double & lb, const double & ub);
	void addApplicableActions(vector<int> * const propPrec, vector<int> * const numPrec, const double & newTS);
	void fillMinMaxFluentTable(vector<double> & toFill);    
    bool canSatisfyGoalConstraints(double & prefCost, const double & maxPrefCost, MinimalState * forKShot, const vector<AutomatonPosition> & preferenceStatusesInRPG, vector<int> * const propPrec, vector<int> * const numPrec, map<int,double> & propsSatisfied, set<int> & prefsSatisfied);
	void getActionsFor(const double & l, const int & i, const bool & min, const double & RHS, list<pair<int, double> > & retList);
    void getActionsForGoal(list<pair<int, double> > & retList);
    void getUnreachablePrefs(list<int> & retList, const vector<AutomatonPosition> & preferenceStatusesInRPG);
	bool isHelpful(const int & a) const { return potentiallyHelpful.find(a) != potentiallyHelpful.end(); };
    void preferenceTrippedBy(const int &, const list<pair<int,int> > &);
    static bool actionNeedsNumerics(const int & a) {
        return fluentAction[a];
    }
};

}

#endif
