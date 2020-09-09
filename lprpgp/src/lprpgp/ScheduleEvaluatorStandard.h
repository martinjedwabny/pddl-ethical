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

#ifndef __SCHEDULEEVALUATORSTANDARD
#define __SCHEDULEEVALUATORSTANDARD

#include "GlobalSchedule.h"

#include <float.h>

namespace Planner {


class ScheduleEvaluatorStandard : public ScheduleEvaluator {

struct InstOpLT {
	bool operator()(const instantiatedOp* const & a, const instantiatedOp* const & b) const {
		return (a->getID() < b->getID());
	}
};

struct LitLT {
	bool operator()(const Literal* const & a, const Literal* const & b) const {
		return (a->getID() < b->getID());
	}
};



struct ExistingWait {

	ScheduleNode* existingNode;
	int comesAfter;

	ExistingWait(ScheduleNode* e, int c) : existingNode(e), comesAfter(c) {};

};


class NodeOpCount {

public:
	map<int, set<ScheduleNode*, ScheduleNodeLT> > count;
	
	NodeOpCount(const int & p, set<ScheduleNode*, ScheduleNodeLT> & c) {
		count[p] = c;
	}

	NodeOpCount() {
	}


};

private:

	vector<vector<double> > weights;

	map<int, double> actionWeights;

	double getWeight(const int & a, const int & b);

	void incrementWeight(const int & a, const int & b, const int multiplier=1);
	void incrementActionWeight(const int & act, const int multiplier=1);

	void addDeletes(map<Literal*, NodeOpCount, LitLT> & activeDeletes, ScheduleNode * toBlame, list<Literal*> & deletes);
	void addNumerics(list<pair<ScheduleNode*, vector<double> > > & fluentTracking, ScheduleNode * toBlame, list<RPGBuilder::NumericEffect> & numEffs);
	void addAdds(map<Literal*, NodeOpCount, LitLT> & activeDeletes, list<Literal*> & adds);
	void addInvariants(map<Literal*, NodeOpCount, LitLT> & activeInvariants, ScheduleNode * toBlame, list<Literal*> & invs);
	void addFluentInvariants(map<int, NodeOpCount> & activeFluentInvariants, ScheduleNode * toBlame, list<int> & invs);

	void removeInvariants(map<Literal*, NodeOpCount, LitLT> & activeInvariants, ScheduleNode * toBlame, list<Literal*> & invs);
	void removeFluentInvariants(map<int, NodeOpCount> & activeFluentInvariants, ScheduleNode * toBlame, list<int> & invs);

	void penalisePreconditions(map<Literal*, NodeOpCount, LitLT> & activeDeletes, ScheduleNode* toBlame, list<Literal*> & precs, double & penalty, double & committedPenalty, ScheduleNode* onlyPenalise, const bool & autoExempt, set<ScheduleNode*, ScheduleNodeLT> & exempt, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, list<ScheduleNode*> & pointsOfConflict, set<ScheduleNode*, ScheduleNodeLT> & pocSet, int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const bool & approximatePenalties, bool * anyPenalties=0, bool debug=false);
	void penaliseNumericPreconditions(list<pair<ScheduleNode*, vector<double> > > & fluentTracking, ScheduleNode* toBlame, list<RPGBuilder::NumericPrecondition> & precs, double & penalty, double & committedPenalty, ScheduleNode* onlyPenalise, const bool & autoExempt, set<ScheduleNode*, ScheduleNodeLT> & exempt, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, list<ScheduleNode*> & pointsOfConflict, set<ScheduleNode*, ScheduleNodeLT> & pocSet, int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const bool & approximatePenalties, bool * anyPenalties=0, bool debug=false);

	void penaliseDeleteEffects(map<Literal*, NodeOpCount, LitLT> & activeInvariants, ScheduleNode* toBlame, list<Literal*> & deletes, double & penalty, double & committedPenalty, ScheduleNode* onlyPenalise, const bool & autoExempt, set<ScheduleNode*, ScheduleNodeLT> & exempt, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, list<ScheduleNode*> & pointsOfConflict, set<ScheduleNode*, ScheduleNodeLT> & pocSet, int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const bool & approximatePenalties, bool * anyPenalties=0, bool debug=false);				  
	void penaliseNumericEffects(map<int, NodeOpCount> & activeFluentInvariants, ScheduleNode* toBlame, list<RPGBuilder::NumericEffect> & deletes, double & penalty, double & committedPenalty, ScheduleNode* onlyPenalise, const bool & autoExempt, set<ScheduleNode*, ScheduleNodeLT> & exempt, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, list<ScheduleNode*> & pointsOfConflict, set<ScheduleNode*, ScheduleNodeLT> & pocSet, int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const bool & approximatePenalties, bool * anyPenalties=0, bool debug=false);				  

	void penaliseGP(list<pair<ScheduleNode*, vector<double> > > & fluentTracking, map<Literal*, NodeOpCount, LitLT> & activeDeletes, double & penalty, ScheduleNode* onlyPenalise, list<ScheduleNode*> & pointsOfConflict, set<ScheduleNode*, ScheduleNodeLT> & pocSet, int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const bool & approximatePenalties, bool * anyPenalties=0, bool debug=false);


	void updatePenaltiesForPreconditions(map<Literal*, NodeOpCount, LitLT> & activeDeletes, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, ScheduleNode* thisNode, list<Literal*> & precs);
	void updatePenaltiesForDeleteEffects(map<Literal*, NodeOpCount, LitLT> & activeInvariants, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, ScheduleNode* thisNode, list<Literal*> & deletes);



	double globalPenalty;
	bool anyPenalties;

	double increaseWeightsBy;
	bool linearIncrease;

	CandidateSchedule* newSchedule;
	CandidateSchedule* waitSchedule;

	ScheduleNode* nsBoundaryNode;
	ScheduleNode* wsBoundaryNode;

	map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> globalNSNewToOld;
	map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> globalWSNewToOld;
	map<int, map<int, ScheduleNode*> > globalOpenStarts;

public:
	ScheduleEvaluatorStandard(const int & c, const double & by, const bool & lin);

	virtual ~ScheduleEvaluatorStandard() {};
	virtual double evaluateSchedule(CandidateSchedule & c, bool * anyPenalties, bool debug, list<ScheduleNode*> & pointsOfConflict, const int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const int & exclude, double & committedPenalty, ScheduleNode* onlyPenalise);
	virtual double evaluateSchedule(CandidateSchedule & c, bool * anyPenalties=0, bool debug=false);
	virtual void updatePenalties(CandidateSchedule & c);
	virtual void prepareForExpansion(list<TimelinePoint*>::iterator & t, CandidateSchedule & c, list<FFEvent> & header, const int & spID);
	virtual double heuristicPenalties(list<FFEvent> & now, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >  & relaxedPlan, map<int, list<list<StartEvent>::iterator > > & compulsaryEnds, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, const bool & newPenaltiesOnly, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalties, const bool & onlyPenaliseLast, double* localPenalty=0);
	virtual double heuristicPenalties(list<TimelinePoint*>::iterator & t, CandidateSchedule & c, list<FFEvent> & header, list<FFEvent> & now, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >  & relaxedPlan, map<int, list<list<StartEvent>::iterator > > & compulsaryEnds, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, const bool & newPenaltiesOnly, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalties, const bool & onlyPenaliseLast, double* localPenalty=0);
	virtual double heuristicSchedulePenalties(ScheduleNode* const waitFor, const bool & comeAfter, list<FFEvent> & header, list<FFEvent> & now, bool & cycle, const int & spID, list<ScheduleNode*> & pointsOfConflict, const bool & newPenaltiesOnly, ScheduleNode* & lastConflict, list<ScheduleNode*> & impliedWaits, double & committedPenalties, const bool & onlyPenaliseLast, double* localPenalty=0);
	virtual double heuristicPenaltiesOldSchedule(CandidateSchedule & c,const int & spID,const bool & newPenaltiesOnly);
//	virtual double heuristicPenaltiesAlternative(list<TimelinePoint*>::iterator & t, CandidateSchedule & c, list<FFEvent> & header, FFEvent & now, list<instantiatedOp*> & relaxedPlan, bool & cycle, const int & spID);

	virtual double getGlobalPenalty() { return globalPenalty; };
	virtual bool getGlobalAnyPenalties() { return anyPenalties; };
	virtual void setGlobalPenalty(const double & d) { globalPenalty = d; };
	virtual void setGlobalAnyPenalties(const bool & a) { anyPenalties = a; };

	virtual void keepLeastCost(pair<int, VAL::time_spec> & opA, double & layerA, const pair<int, VAL::time_spec> & opB, const double & layerB);

	virtual double costFrom(ScheduleNode* a, ScheduleNode * b, const int & spID);
};

}

#endif
