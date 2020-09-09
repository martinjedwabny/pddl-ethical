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

#ifndef __SCHEDULEMANIPULATORATSTART
#define __SCHEDULEMANIPULATORATSTART

#include "GlobalSchedule.h"

namespace Planner {

class ScheduleManipulatorAtStart : public ScheduleManipulator {

private:
	
	CandidateSchedule * withoutSP;
	list<TimelinePoint*>::iterator currentTimelinePoint;
	set<int> subgoalsNotAchieved;
	double baselinePenalty;
public:
	ScheduleManipulatorAtStart(const int & spCount) : withoutSP(0), baselinePenalty(0.0) {
		for (int i = 0; i < spCount; ++i) subgoalsNotAchieved.insert(i);
	};

	~ScheduleManipulatorAtStart() { delete withoutSP; withoutSP = 0; };

	list<FFEvent> * reset(const int & s) {
		currentSubProblem = s;
		delete withoutSP;

		list<FFEvent> * toReturn = (subgoalsNotAchieved.find(s) == subgoalsNotAchieved.end() ? new list<FFEvent>() : 0);
//		cout << "***NEW CODE***\n";
//		GlobalSchedule::currentSchedule->printEvents();
		withoutSP = new CandidateSchedule(*(GlobalSchedule::currentSchedule), s, toReturn);
//		cout << "***NEW CODE***\n";
//		withoutSP->printEvents();
//		cout << "***NEW CODE***\n";
		if (toReturn && false) {
			list<FFEvent>::iterator planItr = toReturn->begin();
			const list<FFEvent>::iterator planEnd = toReturn->end();
	
			for (int i = 0; planItr != planEnd; ++planItr, ++i) {
				if (planItr->action) {
					cout << i << ": " << *(planItr->action) << ", " << (planItr->time_spec == VAL::E_AT_START ? "start" : "end") << "\n";
				} else if (planItr->wait) {
					cout << i << ": wait for " << *(planItr->wait->getOp());
					if (planItr->getEffects) cout << ", getting its effects";
					cout << "\n";
				} else {
					cout << "Plan contains null nodes!\n";
					assert(false);
				}
			}
		}
		currentTimelinePoint = withoutSP->timeline.begin();

		//cout << "Old schedule:\n";
		GlobalSchedule::getCurrentEvaluator()->evaluateSchedule(*(GlobalSchedule::currentSchedule), 0, false); 
		//cout << "New schedule:\n";
		baselinePenalty = GlobalSchedule::getCurrentEvaluator()->evaluateSchedule(*withoutSP, 0, false); 

		return toReturn;
	};

	bool getNextTimelinePoint(list<TimelinePoint*>::iterator & t) {
		if (currentTimelinePoint != withoutSP->timeline.end()) {
			t = currentTimelinePoint;
			currentTimelinePoint = withoutSP->timeline.end();
			return true;
		}
		return false;
	}

	bool allSubgoalsAchieved() {
		return (subgoalsNotAchieved.empty());
	}

	list<TimelinePoint*>::iterator getEndTimelinePoint() {
		return withoutSP->timeline.end();
	}

	void prepareScheduleForExpansion(list<TimelinePoint*>::iterator & t, list<FFEvent> & header,const int & spID) {
		GlobalSchedule::currentEvaluator->prepareForExpansion(t, *withoutSP, header, spID);
	}
	double penaliseSubProblemSolution(list<FFEvent> & now, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > > & relaxedPlan, map<int, list<list<StartEvent>::iterator > > & compulsaryEnds, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalties, const bool & onlyPenaliseLast, double* localPenalty=0) {
		return GlobalSchedule::currentEvaluator->heuristicPenalties(now, relaxedPlan, compulsaryEnds, cycle, spID, pointsOfConflict, true, lastConflict, impliedWaits, committedPenalties, onlyPenaliseLast, localPenalty);
	};


	double penaliseSubProblemSolution(list<TimelinePoint*>::iterator & t, list<FFEvent> & header,  list<FFEvent> & now, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >  & relaxedPlan, map<int, list<list<StartEvent>::iterator > > & compulsaryEnds, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalties, const bool & onlyPenaliseLast, double* localPenalty=0) {
		return GlobalSchedule::currentEvaluator->heuristicPenalties(t, *withoutSP, header, now, relaxedPlan, compulsaryEnds, cycle, spID, pointsOfConflict, true, lastConflict, impliedWaits, committedPenalties, onlyPenaliseLast, localPenalty);
	}	

	double proposeSchedulingDecision(ScheduleNode* wait, const bool & comeAfter, list<FFEvent> & header, list<FFEvent> & now, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalties, const bool & onlyPenaliseLast, double* localPenalty=0) {
		return GlobalSchedule::currentEvaluator->heuristicSchedulePenalties(wait, comeAfter, header, now, cycle, spID, pointsOfConflict, true, lastConflict, impliedWaits, committedPenalties, onlyPenaliseLast, localPenalty);
	}

	void proposeActions(list<TimelinePoint*>::iterator & after, list<FFEvent> & plan, const int & sp, const bool & firstTime, const bool & achievesSubGoal) {
		list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >  relaxedPlan;
		map<int, list<list<StartEvent>::iterator > > compulsaryEnds;
		CandidateSchedule* const newSchedule = withoutSP->getScheduleWithActions(after, plan, relaxedPlan, compulsaryEnds, 0, sp, GlobalSchedule::globalVerbosity & 4);
		
		if (!newSchedule) {
			if (firstTime) subgoalsNotAchieved.insert(sp);
			return;
		}

		bool any = false;
		const double newPenaltyValue = GlobalSchedule::currentEvaluator->evaluateSchedule(*newSchedule, &any, GlobalSchedule::globalVerbosity & 4);
		if (firstTime || (newPenaltyValue < GlobalSchedule::currentEvaluator->getGlobalPenalty()) || (!any && GlobalSchedule::currentEvaluator->getGlobalAnyPenalties())) {
			if (GlobalSchedule::globalVerbosity & 1) {
				cout << "Keeping new schedule - ";
				if (firstTime) {
					cout << "First time round\n";
				} else if (newPenaltyValue < GlobalSchedule::currentEvaluator->getGlobalPenalty()) {
					cout << "Better penalty score\n";
				} else if (!any && GlobalSchedule::currentEvaluator->getGlobalAnyPenalties()) {
					cout << "This one doesn't have penalties but the global best does\n";
				}
			}
			if (GlobalSchedule::globalVerbosity & 1) {
				if (any) {
					cout << "Penalties remain\n";
				} else {
					cout << "No penalties found\n";
				}
			}
			delete GlobalSchedule::currentSchedule;
			GlobalSchedule::currentSchedule = newSchedule;
			GlobalSchedule::currentEvaluator->setGlobalPenalty(newPenaltyValue);
			GlobalSchedule::currentEvaluator->setGlobalAnyPenalties(any);
			if (achievesSubGoal) {
				if (GlobalSchedule::globalVerbosity & 1) cout << "Subgoal " << sp << " now achieved\n";
				subgoalsNotAchieved.erase(sp);
			} else {
				if (GlobalSchedule::globalVerbosity & 1) cout << "Subgoal " << sp << " not achieved (relaxed plan goal jump)\n";
				subgoalsNotAchieved.insert(sp);
			}
			baselinePenalty = newPenaltyValue;
		} else {
			if (GlobalSchedule::globalVerbosity & 1) cout << "Keeping old schedule\n";
		}
		
	}

	const double & getBaselinePenalty() const {
		return baselinePenalty;
	}
	
	ScheduleNode* getTILNode(const int & i) {
		return withoutSP->tilEvents[i];
	};

};

}

#endif
