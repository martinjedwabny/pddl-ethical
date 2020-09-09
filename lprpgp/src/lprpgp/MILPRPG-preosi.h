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

#include "lp_lib.h"

struct LPAndBounds {

	lprec* const lp;
    bool presolved;
	vector<double> min;
	vector<bool> minReal;
	vector<double> max;
	vector<bool> maxReal;
	bool nowExtracting;
	list<int> varsToIntegral;

	LPAndBounds(lprec * l, const bool & p, const int & v) : lp(l), presolved(p), min(v), minReal(v), max(v), maxReal(v), nowExtracting(false) {};
	~LPAndBounds() { delete_lp(lp); };

};

namespace Planner {

class MinimalState;
class MILPRPG {

public:
	struct ActData {
		int Avar;
		int kShot;
		//int Pvar;
		ActData() : Avar(-1), kShot(-1) {};
		ActData(const int & a, const int & p) : Avar(a), kShot(p) {};
	};

	struct VarData {
		int Vvar;
		int consRow;
		int assignRow;
		REAL* maximisationGuess;
		REAL* minimisationGuess;
		int columnsInGuess;
		VarData() : Vvar(-1), consRow(-1), assignRow(-1), maximisationGuess(0), minimisationGuess(0), columnsInGuess(0) {};
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

        static int basiscrash;
        static int pivoting_rule;
        static int pivoting_mode;
        static int presolve;
        static int presolveloops;
        static int simplextype;
        static int bb_floorfirst;
        static int bb_rule;
        static int bb_mode;


private:
	
	int varCount;
	int colCount;
	int layerCount;
	vector<pair<LPAndBounds*, LPAndBounds*> > allLp;
	map<double, int, EpsilonComp> timestampToIndex;
	set<int> applicableSoFar;
	set<int> potentiallyHelpful;
	vector<VarData> minMaxVars;
	vector<REAL*> guessVectors;
	map<int, ActData> layerActionVariables;
	map<int, pair<int, int> > boundedActs;
	MinimalState * startingState; 
    vector<int> switchVarForPreference;
	
	static vector<bool> fluentAction;
	static vector<bool> everAssigned;
	static vector<double> maxNeeded;
	static vector<double> minNeeded;
	static vector<list<ActionEffectUponVariable> > preprocessedData;
	static int actCount;
	static vector<int> goesToRealAction;
	static vector<double> preferEarlier;
    
    struct GoalBound {
        REAL w;
        int v;
        int op;
        
        GoalBound()
        : v(-1)
        {
        };
        
        GoalBound(const REAL & wIn, const int & vIn, const int & lb)
        : w(wIn), v(vIn), op(lb)
        {
        };
    };
    
    /** Goals that can be expressed as simple bounds on variables */
    static vector<GoalBound> goalVarBounds;
    
    struct GoalConstraint {
        vector<pair<int,REAL> > lhs;
        int op;
        REAL rhs;
        
        GoalConstraint()
        {
        };
                
    };
    
    /** Goals that need a full (w.x [>=,<=] c) representation */
    static vector<GoalConstraint> goalConstraints;
    
    /** Action indices of achievers for the top-level propositional goals.  If the goal isn't true
      * in the state being evaluated, one of these must hold when checking if the goal can be reached.
      */
    static vector<vector<int> > propositionalGoalConstraints;
    
    /** Action indices of numeric-free achievers of the top-level propositional goals.  If one of these
      * is present, don't add a constraint for the proposition - we can do it without disturbing the LP.
      */
    static vector<vector<int> > propositionalGoalNotEncoded;
    
    /** Indices of facts that are propositional goal, corresponding with the propositionalGoalConstraints entries .
      * Negative entries indicate landmark fact IDs.
      */
    static vector<int> propositionalGoals;
    
    
    LPAndBounds* goalLP;
public:
    static bool addProps;
    static bool addLandmarks;
    
	MILPRPG(MinimalState * forKShot, const vector<double> & layerZero);
	~MILPRPG();
    int giveMeAColumn(const string & n);
    int getColumnForVar(const int & v);
    void addSwitch(const int & consSize, REAL * actWeight, int * colno, const REAL & rhs);
	void addApplicableActions(vector<int> * const propPrec, vector<int> * const numPrec);
	void fillMinMaxFluentTable(vector<double> & toFill);
    bool canSatisfyGoalConstraints(double & prefCost, MinimalState * forKShot, vector<int> * const propPrec, vector<int> * const numPrec, set<int> & propsSatisfied);
	void getActionsFor(const double & l, const int & i, const bool & min, const double & RHS, list<pair<int, double> > & retList);
    void getActionsForGoal(list<pair<int, double> > & retList);
	bool isHelpful(const int & a) const { return potentiallyHelpful.find(a) != potentiallyHelpful.end(); };
};

}

#endif
