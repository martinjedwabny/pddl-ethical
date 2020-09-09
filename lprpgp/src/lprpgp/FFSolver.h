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

#ifndef __FFSOLVER
#define __FFSOLVER

#include "Decomposition.h"
#include "ptree.h"

#include <map>
#include <list>

using std::map;
using std::list;

#ifndef NDEBUG
/** Preprocessor flag to enable debug hooks - used to detect if any of the states along a known-good trajectory pruned during search. */
#define FFSEARCHDEBUGHOOKS 1
#endif

namespace Planner {

class TimelinePoint;
class ScheduleNode;
class SearchQueueItem;

class FFEvent {

public:
	instantiatedOp* action;
	VAL::time_spec time_spec;
	double minDuration;
	double maxDuration;
	int pairWithStep;
	ScheduleNode* wait;	
	bool noop;
	bool getEffects;

	FFEvent(instantiatedOp* a, const double & dMin, const double & dMax);
	
	FFEvent(const FFEvent & f);
	FFEvent();
	FFEvent & operator=(const FFEvent & f);
	bool operator==(const FFEvent & f) const {
		if (time_spec == VAL::E_AT_END && pairWithStep != f.pairWithStep) return false;
		return (action == f.action && time_spec == f.time_spec && minDuration == f.minDuration && maxDuration == f.maxDuration && pairWithStep == f.pairWithStep && wait == f.wait && noop == f.noop && getEffects == f.getEffects);
	}

};

struct StartEvent {
	int actID;
	int stepID;
	double advancingDuration;
	double minDuration;
	double maxDuration;
	double elapsed;
	double minAdvance;
	ScheduleNode* compulsaryEnd;
	bool terminated;
	int fanIn;
	list<int> endComesBefore;

	StartEvent(const int & a, const int & s, const double & mind, const double & maxd, const double &e, ScheduleNode* const ce) : actID(a), stepID(s), advancingDuration(mind), minDuration(mind), maxDuration(maxd), elapsed(e), minAdvance(DBL_MAX), compulsaryEnd(ce), terminated(false), fanIn(0) {};

	bool operator ==(const StartEvent & e) const {
		return (actID == e.actID &&
			stepID == e.stepID &&
			fabs(minDuration - e.minDuration) < 0.0005 &&
			fabs(maxDuration - e.maxDuration) < 0.0005 &&
			fabs(elapsed - e.elapsed) < 0.0005 &&
			fabs(advancingDuration - e.advancingDuration) < 0.0005 &&
			compulsaryEnd == e.compulsaryEnd &&
			terminated == e.terminated &&
			fanIn == e.fanIn &&
			endComesBefore == e.endComesBefore);
	}
};

class ExtendedMinimalState : public MinimalState {

public:
	
	//list<StartEvent> startEventQueue;
	//map<int, list<list<StartEvent>::iterator > > entriesForAction;

	double timeStamp;
	int nextTIL;
	int planLength;
	int stepBeforeTIL;
	int tilFanIn;
	list<int> tilComesBefore;

	ExtendedMinimalState(const set<int> & f, const vector<double> & s, const map<int, int> & sa, const map<int, int> & ia, const map<int, int> & lc, const double & ts, const int & nt, const int & pl, const vector<AutomatonPosition> & ps, const vector<bool> & ls, const double & ppv, const double & sc) : MinimalState(f, s, sa, ia, lc, ps, ls, ppv, sc), timeStamp(ts), nextTIL(nt), planLength(pl), stepBeforeTIL(-1), tilFanIn(0) {};
	ExtendedMinimalState() : MinimalState(), timeStamp(0.0), nextTIL(0), planLength(0), stepBeforeTIL(-1), tilFanIn(0) {};
    ExtendedMinimalState(const MinimalState & s) : MinimalState(s), timeStamp(0.0), nextTIL(0), planLength(0), stepBeforeTIL(-1), tilFanIn(0) {};

	ExtendedMinimalState(const ExtendedMinimalState & e) : MinimalState(e.first, e.second, e.startedActions, e.invariants, e.fluentInvariants, e.preferenceStatus, e.landmarkStatus, e.prefPreconditionViolations, e.cost), /*startEventQueue(e.startEventQueue),*/ timeStamp(e.timeStamp), nextTIL(e.nextTIL), planLength(e.planLength), stepBeforeTIL(e.stepBeforeTIL), tilFanIn(e.tilFanIn), tilComesBefore(e.tilComesBefore)  {

    /*	list<StartEvent>::iterator bqItr = startEventQueue.begin();
		const list<StartEvent>::iterator bqEnd = startEventQueue.end();

		for (; bqItr != bqEnd; ++bqItr) {
			entriesForAction[bqItr->actID].push_back(bqItr);
		}*/

	}

	ExtendedMinimalState & operator=(const ExtendedMinimalState & e) {

		first = e.first;
		second = e.second;
		startedActions = e.startedActions;
		invariants = e.invariants;
		fluentInvariants = e.fluentInvariants;
        preferenceStatus = e.preferenceStatus;
        landmarkStatus = e.landmarkStatus;
        prefPreconditionViolations = e.prefPreconditionViolations;
        cost = e.cost;
		//startEventQueue = e.startEventQueue;
		timeStamp = e.timeStamp;
		nextTIL = e.nextTIL;
		planLength = e.planLength;
		stepBeforeTIL = e.stepBeforeTIL;
		tilFanIn = e.tilFanIn;
		tilComesBefore = e.tilComesBefore;
		/*entriesForAction.clear();
		list<StartEvent>::iterator bqItr = startEventQueue.begin();
		const list<StartEvent>::iterator bqEnd = startEventQueue.end();

		for (; bqItr != bqEnd; ++bqItr) {
			entriesForAction[bqItr->actID].push_back(bqItr);
		}*/


		return *this;
	}

	virtual ~ExtendedMinimalState() {};


    virtual bool operator==(const ExtendedMinimalState & o) const {
        return (nextTIL == o.nextTIL && first == o.first && second == o.second && preferenceStatus == o.preferenceStatus && landmarkStatus == o.landmarkStatus && prefPreconditionViolations == o.prefPreconditionViolations && cost == o.cost
                && startedActions == o.startedActions && invariants == o.invariants && fluentInvariants == o.fluentInvariants && stepBeforeTIL == o.stepBeforeTIL && tilFanIn == o.tilFanIn && tilComesBefore == o.tilComesBefore /*&& queueEqual(startEventQueue, o.startEventQueue)*/ && fabs(timeStamp - o.timeStamp) < 0.0005);
    }


};

class SearchAroundPlan;

class FF {

public:
	
	class HTrio {
	
	public:
	
		double first;
		double second;
		int third;
        bool goalsSatisfied;
	
		HTrio() : goalsSatisfied(false) {};
		HTrio(const double & a, const double & b, const int & c, bool gs=false) : first(a), second(b), third(c), goalsSatisfied(gs) {};
		HTrio(const HTrio & h) : first(h.first), second(h.second), third(h.third), goalsSatisfied(h.goalsSatisfied) {};
		
		HTrio & operator =(const HTrio & h) {
			first = h.first;
			second = h.second;
			third = h.third;
            goalsSatisfied = h.goalsSatisfied;
			return *this;
		}
	
	};

protected:

    friend class SearchAroundPlan;

	static HTrio evaluateStateWRTSchedule(SubproblemRPG* const rpg, ExtendedMinimalState & theState, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActions, list<pair<double,double> > & howMany, list<ScheduleNode*> & pointsOfConflict, list<FFEvent> & header, list<FFEvent> & now, HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnant, bool considerCache=false, map<double, list<int> > * justApplied=0, double tilFrom=0.001);
	static HTrio evaluateStateWRTScheduleLocally(list<TimelinePoint*>::iterator startPoint, SubproblemRPG* const rpg, ExtendedMinimalState & theState, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActions, list<ScheduleNode*> & pointsOfConflict, list<TimelinePoint*>::iterator t, list<FFEvent> & header, list<FFEvent> & now, HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnant, bool considerCache, double & localPenalty, map<double, list<int> > * justApplied=0, double tilFrom=0.001);
	static HTrio evaluateStateWRTScheduleLocally(list<TimelinePoint*>::iterator startPoint, SubproblemRPG* const rpg, ExtendedMinimalState & theState, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActions, list<ScheduleNode*> & pointsOfConflict, list<TimelinePoint*>::iterator t, list<FFEvent> & header, list<FFEvent> & now, ScheduleNode* const waitFor, const bool & comesAfter, HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnant, bool considerCache, double & localPenalty, map<double, list<int> > * justApplied=0, double tilFrom=0.001);
	static HTrio evaluateOldPlan(list<TimelinePoint*>::iterator startPoint, SubproblemRPG* const rpg, list<ScheduleNode*> & pointsOfConflict, list<TimelinePoint*>::iterator t, list<FFEvent> * header, HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID);

	static void applyActionToState(SubproblemRPG* const rpg, ExtendedMinimalState & theState, const int & theAction, const VAL::time_spec & ts);

    static void reorderByPrefCost(SubproblemRPG* const rpg, SearchQueueItem * const currSQI, list<pair<int, VAL::time_spec> > & applicableActions, list<pair<double,double> > & aWeights);
    
	static void chooseWhetherToReuse(const list<TimelinePoint*>::iterator & startPoint, auto_ptr<SearchQueueItem> & succ, SubproblemRPG* rpg, ExtendedMinimalState & state, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActionsExport, list<ScheduleNode*> & pocExport, list<TimelinePoint*>::iterator tlPoint, const pair<instantiatedOp*, VAL::time_spec> & actID, list<FFEvent> & header, HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnant);
	static void chooseWhetherToReuseAndWait(auto_ptr<SearchQueueItem> & succ, SubproblemRPG* rpg, ExtendedMinimalState & state, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActionsExport, list<pair<double,double> > & howManyExport, list<ScheduleNode*> & pocExport, const pair<instantiatedOp*, VAL::time_spec> & actID, const int & howManyInTotal, list<FFEvent> & header, HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnant);
	static void justEvaluateNotReuse(auto_ptr<SearchQueueItem> & succ, SubproblemRPG* rpg, ExtendedMinimalState & state, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActionsExport, list<pair<double,double> > & howManyExport, list<ScheduleNode*> & pocExport, list<FFEvent> & extraEvents, list<FFEvent> & header, HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnant, map<double, list<int> > * justApplied, double tilFrom=0.001);
	static list<list<TimelinePoint*>::iterator > findExecutablePath(list<TimelinePoint*>::iterator startPoint, SubproblemRPG* rpg, ExtendedMinimalState & startState);



	
    static void helpfulHelper(list<pair<int, VAL::time_spec> >::iterator helpfulActsItr,
                                  const list<pair<int, VAL::time_spec> >::iterator helpfulActsEnd,
                                  list<pair<double,double> >::iterator hhmItr,
                                  SearchQueueItem * const currSQI,
                                  SubProblem * const toSolve,
                                  SearchQueueItem *& succOneReturn,
                                  SearchQueueItem *& succTwoReturn);
                                                            
    
    static double bestSolutionPCost;

public:
	static bool ignorePenalties;
	static bool steepestDescent;
	static bool initialAdvancing;
	static bool bestFirstSearch;
	static bool helpfulActions;
	static bool disableWaits;
	static bool pruneMemoised;
	static bool stagnantCommitted;
	static bool firstImprover;
	static bool relaxedGoalJump;
	static bool neverReuseOrWait;
	static bool incrementalExpansion;
	static bool skipEHC;
	static bool zealousEHC;
	static bool startsBeforeEnds;
	static bool invariantRPG;
	static bool tsChecking;
	static bool timeWAStar;
	static bool WAStar;
	static bool justWaitIncr;
	static double doubleU;
    static bool allowDualOpenList;
    static bool useDualOpenList;
    static bool multipleHelpfuls;
    static int disablePareto;
    static double capOnPreferenceCost;
    static bool useWeightedSumWithPrefCost;
    static double prefWeightInWeightedSum;
	//static list<instantiatedOp*> * solveSubproblem(LiteralSet & startingState, vector<pair<PNE*, double> > & startingFluents, SubProblem* const s);
	static list<FFEvent> * solveSubproblemWRTSchedule(LiteralSet & startingState, vector<double> & startingFluents, SubProblem* const s,const int nodeLimit,bool & reachedGoal, const int & spID, list<FFEvent> * oldSoln, double & oldSolutionValue, int & relaxedStepCount);
    static int testExistingSolution(const list<int> & soln, const bool & primeKnownStates);	
    
    #ifdef FFSEARCHDEBUGHOOKS
    static bool actuallyPlanGivenPreviousSolution;
    #endif
    
};


};

#endif
