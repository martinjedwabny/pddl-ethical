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

#ifndef __GLOBALSCHEDULE
#define __GLOBALSCHEDULE

#include <list>
using std::list;

#include <vector>
using std::vector;

#include "instantiation.h"

#include "ptree.h"

#include "RPGBuilder.h"

#include "FFSolver.h"

using namespace Inst;

namespace Planner {



class ScheduleNodeImpl;

class ScheduleNode;

struct ScheduleNodeLT {
	bool operator()(const ScheduleNode* const & a, const ScheduleNode* const & b) const {
		return (a < b);
	}
};

class ScheduleNode {

private:

	ScheduleNodeImpl * const impl;

public:
	
	ScheduleNode(instantiatedOp* opIn, const int & spIn, const bool & sIn);
	ScheduleNode(list<Literal*> & addEffs, list<Literal*> & delEffs, const double & d, const int & tID);
	ScheduleNode(ScheduleNode & s);
	~ScheduleNode();

	void scheduleAfter(ScheduleNode * s, const double & min, const double & max);
	void scheduleAfter(ScheduleNode * s, const pair<double, double> & w);
	void scheduleBefore(ScheduleNode * s, const double & min, const double & max);
	void scheduleBefore(ScheduleNode * s, const pair<double, double> & w);
	
	list<pair<ScheduleNode*, pair<double, double> > > & getSuccessors();
	list<pair<ScheduleNode*, pair<double, double> > > & getPredecessors();

	void addSubProblem(const int & s);
	void removeSubProblem(const int & s);
	set<int> & getSubProblem();
	bool forSubProblem(const int & s) const;

	const double & getDuration() const;
	const bool & isStart() const;
	const bool & tilNode() const;
	const int & tilID() const;

	const double & distFromZeroConst() const;
	const double & distToZeroConst() const;

	double & distFromZero();
	double & distToZero();

	instantiatedOp* getOp() const;
	
	list<Literal*> & getPreconditions();

	list<Literal*> & getInvariants();


	list<Literal*> & getAddEffects();

	list<Literal*> & getDeleteEffects();

	list<RPGBuilder::NumericPrecondition> & getNumericPreconditions();
	list<RPGBuilder::NumericPrecondition> & getNumericInvariants();
	list<RPGBuilder::NumericEffect> & getNumericEffects();

	ScheduleNode* getPartner();
	void setPartner(ScheduleNode*);	

	set<ScheduleNode*, ScheduleNodeLT> NEW;
	set<ScheduleNode*, ScheduleNodeLT> UBP;
	set<ScheduleNode*, ScheduleNodeLT> LBP;

	ScheduleNode* & toOld();
	ScheduleNode* & toNew();

	void preserve(const bool & keepEdges);
	void rollback();

};



class TimelinePoint {

public:
	LiteralSet state;
	vector<double> fluents;
	list<ScheduleNode*>::iterator after;
	bool dummy;
	double timestamp;

	TimelinePoint(LiteralSet & sIn, vector<double> & vIn) : state(sIn), fluents(vIn), after(0), dummy(true), timestamp(0.0) {};
	TimelinePoint(LiteralSet & sIn, vector<double> & vIn, list<ScheduleNode*>::iterator & aIn) : state(sIn), fluents(vIn), after(aIn), dummy(false), timestamp(0.0) {};
	TimelinePoint(TimelinePoint & p) : state(p.state), fluents(p.fluents), after(p.after), dummy(p.dummy), timestamp(p.timestamp) {};
	TimelinePoint(TimelinePoint & p, list<Literal*> & add, list<Literal*> & del);

	instantiatedOp* afterOp();

};

class CandidateSchedule {

private:

	list<pair<ScheduleNode*, pair<double, double> > > edgesFromZero;

	//void enforceOrder();
	double consider(map<Literal*, set<ScheduleNode*, ScheduleNodeLT>, LiteralLT> & activeDeletes, list<ScheduleNode*>::iterator from, list<ScheduleNode*>::iterator to, const int & sp);


	void addEdgeFromZero(ScheduleNode* s, const double & min, const double & max) {
		edgesFromZero.push_back(pair<ScheduleNode*, pair<double, double> >(s, pair<double, double>(min, max)));
	}

	static list<TimelinePoint*> oldTimeline;
	static list<ScheduleNode*> oldEvents;

public:
	ScheduleNode* m_after;
	bool m_afterNew;
	ScheduleNode* m_afterReal;
	bool m_afterRealNew;
	int m_cl;
	int m_tilInPlan;

	list<ScheduleNode*> events;
	list<TimelinePoint*> timeline;

	vector<ScheduleNode*> tilEvents;

	map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> oldToNew;

	CandidateSchedule(TimelinePoint* t) : m_after(0), m_afterReal(0) {timeline.push_back(t); };
	~CandidateSchedule();
	CandidateSchedule(CandidateSchedule & s, const int without=-1, list<FFEvent> * removedEvents=0);
	CandidateSchedule(CandidateSchedule & s, map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> & newToOld, const bool skipTimeline);

	CandidateSchedule * getScheduleWithActions(list<TimelinePoint*>::iterator & after, list<FFEvent> & plan, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >  & relaxedPlan, map<int, list<list<StartEvent>::iterator > > compulsaryEnds, map<int, map<int, ScheduleNode*> > * rememberOS, const int & sp, bool debug, map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> * newToOld=0, list<ScheduleNode*> * impliedWeights=0, int onlyMFS=INT_MAX, ScheduleNode* * onlyPenalise=0, const bool skipTimeline=false);


	bool postConstraint(ScheduleNode* const a, ScheduleNode* const b, const bool & bAfterA);
	void unpostConstraint();
	bool postNewActions(list<FFEvent> & newEvents, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >  & relaxedPlan, map<int, list<list<StartEvent>::iterator > > compulsaryEnds, map<int, map<int, ScheduleNode*> > * rememberOS, map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> & reallyOldToNew, const int & sp,bool debug, map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> * newToOld=0, ScheduleNode* * onlyPenalise=0);

	void printEvents();
	void printAsPlan();

	void addTILs(list<RPGBuilder::FakeTILAction> & TILs);
};

class ScheduleManipulator {

protected:
	ScheduleManipulator() : currentSubProblem(-1) {};
	int currentSubProblem;
public: 
	virtual ~ScheduleManipulator() {};
	virtual list<FFEvent> * reset(const int &) = 0;
	virtual bool getNextTimelinePoint(list<TimelinePoint*>::iterator &) = 0;
	virtual list<TimelinePoint*>::iterator getEndTimelinePoint() = 0;
	virtual void prepareScheduleForExpansion(list<TimelinePoint*>::iterator & t, list<FFEvent> & header,const int & spID) = 0;
	virtual double penaliseSubProblemSolution(list<FFEvent> & now, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > > & relaxedPlan, map<int, list<list<StartEvent>::iterator > > & compulsaryEnds, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalty, const bool & onlyPenaliseLast, double* localPenalty=0) = 0;
	virtual double penaliseSubProblemSolution(list<TimelinePoint*>::iterator & t, list<FFEvent> & header, list<FFEvent> & now, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > > & relaxedPlan, map<int, list<list<StartEvent>::iterator > > & entriesForAction, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalty, const bool & onlyPenaliseLast, double* localPenalty=0) = 0;
	virtual void proposeActions(list<TimelinePoint*>::iterator & after, list<FFEvent> & plan, const int & subproblemID, const bool & firstTime, const bool & achievesSubGoal) = 0;
	virtual double proposeSchedulingDecision(ScheduleNode* wait, const bool & comeAfter, list<FFEvent> & header, list<FFEvent> & now, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalty, const bool & onlyPenaliseLast, double* localPenalty=0) = 0;
	virtual ScheduleNode* getTILNode(const int & i) = 0;
	virtual bool allSubgoalsAchieved() = 0;
	virtual const double & getBaselinePenalty() const = 0;


};

class ScheduleEvaluator {

protected:
	ScheduleEvaluator() {};
public:
	virtual ~ScheduleEvaluator() {};
	virtual double evaluateSchedule(CandidateSchedule & c, bool * anyPenalties=0, bool debug=false) = 0;
	virtual void updatePenalties(CandidateSchedule & c) = 0;
	virtual void prepareForExpansion(list<TimelinePoint*>::iterator & t, CandidateSchedule & c, list<FFEvent> & header, const int & spID) = 0;
	virtual double heuristicPenalties(list<FFEvent> & now, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > > & relaxedPlan, map<int, list<list<StartEvent>::iterator > > & compulsaryEnds, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, const bool & newPenaltiesOnly, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalty, const bool & onlyPenaliseLast, double* localPenalty=0) = 0;
	virtual double heuristicPenalties(list<TimelinePoint*>::iterator & t, CandidateSchedule & c, list<FFEvent> & header, list<FFEvent> & now, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > > & relaxedPlan, map<int, list<list<StartEvent>::iterator > > & compulsaryEnds, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, const bool & newPenaltiesOnly, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalty, const bool & onlyPenaliseLast, double* localPenalty=0) = 0;
	virtual double heuristicSchedulePenalties(ScheduleNode* const waitFor, const bool & comeAfter, list<FFEvent> & header, list<FFEvent> & now, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, const bool & newPenaltiesOnly, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalty, const bool & onlyPenaliseLast, double* localPenalty=0) = 0;
	virtual double heuristicPenaltiesOldSchedule(CandidateSchedule & c, const int & spID, const bool & newPenaltiesOnly) = 0;
	virtual double getGlobalPenalty() = 0;
	virtual void setGlobalPenalty(const double & d) = 0;
	virtual bool getGlobalAnyPenalties() = 0;
	virtual void setGlobalAnyPenalties(const bool & d) = 0;
	virtual void keepLeastCost(pair<int, VAL::time_spec> & opA, double & layerA, const pair<int, VAL::time_spec> & opB, const double & layerB) = 0;
	virtual double costFrom(ScheduleNode* a, ScheduleNode * b, const int & sp) = 0;
};


#ifndef NDEBUG
#define ENABLE_DEBUGGING_HOOKS 1
#endif


class GlobalSchedule {

public:

	static bool alternativeGP;
	static bool penaliseHead;
	static bool checkOpenStartsOnly;
	static int globalVerbosity;
	static int minFlawScheduling;
	static int approximatePenalties;

	static void removeEvents(const int & subProblemID);
	
	static ScheduleManipulator * currentManipulator;
	static ScheduleEvaluator * currentEvaluator;
	static CandidateSchedule * currentSchedule;

	static set<Literal*, LiteralLT> topLevelGoals;

	static void initialise();

	static void registerManipulator(ScheduleManipulator * c) {
		currentManipulator = c;
	};

	static void registerEvaluator(ScheduleEvaluator * c) {
		currentEvaluator = c;
	};

	static ScheduleManipulator * getCurrentManipulator() { return currentManipulator; };
	static ScheduleEvaluator * getCurrentEvaluator() { return currentEvaluator; };
	
	static map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> sanityCheckDebug;
    
    #ifdef ENABLE_DEBUGGING_HOOKS
    
    /** @brief A vector of which actions definitely must be kept, i.e. not pruned in preprocessing.
    * 
    *  This vector is only present for debugging purposes.  To populate the vector:
    *  - use the <code>-H</code> command line flag
    *  - provide a plan filename after the domain and problem filenames
    *  Then, the <code>instantiatedOp</code>s used in the plan will have their entries in this vector
    *  set to true.
    */
    static vector<bool> actionHasToBeKept;
    
    /** @brief An exemplar plan for the current problem, to be read in for debugging purposes.
    * 
    *  @see actionHasToBeKept
    */
    static const char * planFilename;
    
    /** @brief Read in <code>planFilename</code> and note that its actions must not be pruned in preprocessing.
    *
    *  @see actionHasToBeKept
    */
    static void markThatActionsInPlanHaveToBeKept();
    
    /** @brief Note that the action with the specified ID has been pruned, due to the given reason.
    * 
    *  This will lead to an assertion failure if the action must not be pruned.
    *
    *  @param i         The action index that has been eliminated
    *  @param synopsis  A short reason for why the action was eliminated.  This is printed if the pruning is known to be in error.
    *
    *  @see actionHasToBeKept
    */
    static void eliminatedAction(const int & i, const char * synopsis);
    #endif
};

};

#endif
