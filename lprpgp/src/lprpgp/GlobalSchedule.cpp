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

#include "GlobalSchedule.h"


#include "RPGBuilder.h"
#include <assert.h>

#include <iostream>

#include <float.h>

using std::cout;
using std::ifstream;

#ifdef ENABLE_DEBUGGING_HOOKS

#include <FlexLexer.h>

#include <fstream>
using std::ifstream;

#include <ptree.h>
#include <instantiation.h>
using namespace Inst;

namespace VAL
{
    extern yyFlexLexer* yfl;
};

extern int yyparse();
extern int yydebug;

using namespace VAL;

#endif


namespace Planner {

int GlobalSchedule::globalVerbosity = 0;

bool GlobalSchedule::penaliseHead = true;
int GlobalSchedule::minFlawScheduling = 0;
bool GlobalSchedule::alternativeGP = true;
int GlobalSchedule::approximatePenalties = 2;
bool GlobalSchedule::checkOpenStartsOnly = false;

CandidateSchedule * GlobalSchedule::currentSchedule = 0;
ScheduleManipulator * GlobalSchedule::currentManipulator = 0;
ScheduleEvaluator * GlobalSchedule::currentEvaluator = 0;

map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> GlobalSchedule::sanityCheckDebug;

set<Literal*, LiteralLT> GlobalSchedule::topLevelGoals;

class KeptDetails {

public:

	const double preservedDFromZero;
	const double preservedDToZero;
	list<pair<ScheduleNode*, pair<double, double> > > * const keptAfter;
	list<pair<ScheduleNode*, pair<double, double> > > * const keptBefore;

	KeptDetails(const double & a, const double & b) : preservedDFromZero(a), preservedDToZero(b), keptAfter(0), keptBefore(0) {};
	KeptDetails(const double & a, const double & b, const list<pair<ScheduleNode*, pair<double, double> > > & aft, const list<pair<ScheduleNode*, pair<double, double> > > & before) : preservedDFromZero(a), preservedDToZero(b), keptAfter(new list<pair<ScheduleNode*, pair<double, double> > >(aft)), keptBefore(new list<pair<ScheduleNode*, pair<double, double> > >(before)) {};
	~KeptDetails() {
		delete keptAfter;
		delete keptBefore;
	};
};

class ScheduleNodeImpl {

private:
	static const double EPSILON;
	instantiatedOp* op;
	set<int> subproblem;
	double duration;

	bool start;
	bool TIL;
	int tilIDVar;
	list<pair<ScheduleNode*, pair<double, double> > > after;
	list<pair<ScheduleNode*, pair<double, double> > > before;
	bool cachedEffects;
	bool cachedPrecInv;
	list<Literal*> cachedAddEffects;
	list<Literal*> cachedDeleteEffects;
	list<Literal*> cachedPreconditions;
	list<Literal*> cachedInvariants;
	
	list<RPGBuilder::NumericPrecondition> cachedNumericPreconditions;
	list<RPGBuilder::NumericPrecondition> cachedNumericInvariants;
	list<RPGBuilder::NumericEffect> cachedNumericEffects;

	ScheduleNode* partner; // for end nodes - the start, and vice versa

	double dFromZero;
	double dToZero;

	void cacheEffects() {
		cachedEffects = true;
		assert(!TIL);
		RPGBuilder::getEffects(op, start, cachedAddEffects, cachedDeleteEffects, cachedNumericEffects);
	};

	void cachePreconditions() {
		cachedPrecInv = true;
		//cout << "Cached preconditions for " << this << "\n";
		assert(!TIL);
		RPGBuilder::getPrecInv(op, start, cachedPreconditions, cachedInvariants, cachedNumericPreconditions, cachedNumericInvariants);
	};

	ScheduleNode * newPtr;
	ScheduleNode * oldPtr;

	KeptDetails * kept;
public:
	
	ScheduleNodeImpl(instantiatedOp* opIn, const int & spIn, const bool & sIn) : op(opIn), subproblem(), start(sIn), TIL(false), tilIDVar(-1), cachedEffects(false), cachedPrecInv(false), partner(0), dFromZero(DBL_MAX), dToZero(-DBL_MAX), newPtr(0), oldPtr(0), kept(0) {
		subproblem.insert(spIn);	
		duration = (sIn ? RPGBuilder::getOpDuration(opIn) : EPSILON);
		//cout << "NodeImpl created " << this << " with cachedPrecInv = " << cachedPrecInv << "\n";
		assert(op);
	};

	ScheduleNodeImpl(ScheduleNodeImpl & s) : op(s.op), subproblem(s.subproblem), duration(s.duration), start(s.start), TIL(s.TIL), tilIDVar(s.tilIDVar), cachedEffects(s.cachedEffects), cachedPrecInv(s.cachedPrecInv),cachedAddEffects(s.cachedAddEffects),cachedDeleteEffects(s.cachedDeleteEffects),cachedPreconditions(s.cachedPreconditions),cachedInvariants(s.cachedInvariants),cachedNumericPreconditions(s.cachedNumericPreconditions), cachedNumericInvariants(s.cachedNumericInvariants), cachedNumericEffects(s.cachedNumericEffects), partner(0), dFromZero(s.dFromZero), dToZero(s.dToZero), newPtr(0), oldPtr(0), kept(0) {
		//cout << "NodeImpl created " << this << " with cachedPrecInv = " << cachedPrecInv << "\n";
		assert(TIL || op);
	};


	ScheduleNodeImpl(list<Literal*> & adds, list<Literal*> & dels, const double & tilDelay, const int & idIn) : op(0), duration(EPSILON), start(false), TIL(true), tilIDVar(idIn), cachedEffects(true), cachedPrecInv(true),cachedAddEffects(adds),cachedDeleteEffects(dels), partner(0), dFromZero(tilDelay), dToZero(-tilDelay), newPtr(0), oldPtr(0), kept(0) {
		//cout << "NodeImpl created " << this << " with cachedPrecInv = " << cachedPrecInv << "\n";
		//
	};

	~ScheduleNodeImpl() {
		if (kept) delete kept;
	}

	void scheduleAfter(ScheduleNode * s, const double & min, const double & max) {
		after.push_back(pair<ScheduleNode*, pair<double, double> >(s,pair<double,double>(min,max)));
		if (!s) assert(min > 0.0);
	};
	void scheduleAfter(ScheduleNode * s, const pair<double, double> & w) {
		after.push_back(pair<ScheduleNode*, pair<double, double> >(s,w));
		if (!s) assert(w.first > 0.0);
	};

	void scheduleBefore(ScheduleNode * s, const double & min, const double & max) { before.push_back(pair<ScheduleNode*, pair<double, double> >(s,pair<double,double>(min,max))); };
	void scheduleBefore(ScheduleNode * s, const pair<double, double> & w) { before.push_back(pair<ScheduleNode*, pair<double, double> >(s,w)); };
	
	list<pair<ScheduleNode*, pair<double, double> > > & getSuccessors() { return before; };
	list<pair<ScheduleNode*, pair<double, double> > > & getPredecessors() { return after; };

	void addSubProblem(const int & s) { subproblem.insert(s); };
	void removeSubProblem(const int & s) { subproblem.erase(s); };	
	set<int> & getSubProblem() { return subproblem; };
	bool forSubProblem(const int & s) const { return (subproblem.find(s) != subproblem.end()); };
	const double & getDuration() const { return duration; };
	const bool & isStart() const { return start; };
	const bool & tilNode() const { return TIL; };
	const int & tilID() const { return tilIDVar; };

	const double & distFromZeroConst() const {return dFromZero; };
	const double & distToZeroConst() const {return dToZero; };

	double & distFromZero() {return dFromZero; };
	double & distToZero() {return dToZero; };

	instantiatedOp* getOp() const { return op; };
	
	list<Literal*> & getPreconditions() {
		if (!cachedPrecInv) cachePreconditions();
		return cachedPreconditions;
	}

	list<Literal*> & getInvariants() {
		if (!cachedPrecInv) cachePreconditions();
		return cachedInvariants;
	}


	list<Literal*> & getAddEffects() {
		if (!cachedEffects) cacheEffects();
		return cachedAddEffects;
	}

	list<Literal*> & getDeleteEffects() {
		if (!cachedEffects) cacheEffects();
		return cachedDeleteEffects;
	}

	list<RPGBuilder::NumericPrecondition> & getNumericPreconditions() {
		if (!cachedPrecInv) cachePreconditions();
		return cachedNumericPreconditions;
	}

	list<RPGBuilder::NumericPrecondition> & getNumericInvariants() {
		if (!cachedPrecInv) cachePreconditions();
		return cachedNumericInvariants;
	}

	list<RPGBuilder::NumericEffect> & getNumericEffects() {
		if (!cachedEffects) cacheEffects();
		return cachedNumericEffects;
	}

	ScheduleNode * getPartner() {
		return partner;
	};

	void setPartner(ScheduleNode * p) {
		partner = p;
	};


	ScheduleNode* & toOld() {
		return oldPtr;
	}

	ScheduleNode* & toNew() {
		return newPtr;
	};

	void preserve(const bool & keepEdges) {
		if (kept) return;
		if (keepEdges) {
			kept = new KeptDetails(dFromZero, dToZero, after, before);
		} else {
			kept = new KeptDetails(dFromZero, dToZero);
		}
	}

	void rollback() {
		assert(kept);
		if (kept->keptAfter) {
			after = *(kept->keptAfter);
		}
		if (kept->keptBefore) {
			before = *(kept->keptBefore);
		}
		dFromZero = kept->preservedDFromZero;
		dToZero = kept->preservedDToZero;
		delete kept;
		kept = 0;
	};



};

const double ScheduleNodeImpl::EPSILON = 0.001; // FIXME should be global

ScheduleNode::ScheduleNode(instantiatedOp* opIn, const int & spIn, const bool & sIn) : impl(new ScheduleNodeImpl(opIn, spIn, sIn)) {
	//cout << "Node created " << this << " with Impl = " << impl << "\n";
};

ScheduleNode::ScheduleNode(list<Literal*> & addEffs, list<Literal*> & delEffs, const double & d, const int & idIn) : impl(new ScheduleNodeImpl(addEffs, delEffs, d, idIn)) {

};

ScheduleNode::ScheduleNode(ScheduleNode & s) : impl(new ScheduleNodeImpl(*s.impl)) {
	//cout << "Node created " << this << " with Impl = " << impl << "\n";

};

ScheduleNode::~ScheduleNode() {
	delete impl; 
	//cout << "Deleting node " << this << " with Impl = " << impl << "\n";
};

void ScheduleNode::scheduleAfter(ScheduleNode * s, const double & min, const double & max) { impl->scheduleAfter(s, min, max); };
void ScheduleNode::scheduleAfter(ScheduleNode * s, const pair<double, double> & w) { impl->scheduleAfter(s, w); };
void ScheduleNode::scheduleBefore(ScheduleNode * s, const double & min, const double & max) { impl->scheduleBefore(s, min, max); };
void ScheduleNode::scheduleBefore(ScheduleNode * s, const pair<double, double> & w) { impl->scheduleBefore(s, w); };

list<pair<ScheduleNode*, pair<double, double> > > & ScheduleNode::getSuccessors() { return impl->getSuccessors(); };
list<pair<ScheduleNode*, pair<double, double> > > & ScheduleNode::getPredecessors() { return impl->getPredecessors(); };

void ScheduleNode::addSubProblem(const int & s) { impl->addSubProblem(s); };
void ScheduleNode::removeSubProblem(const int & s) { impl->removeSubProblem(s); };

set<int> & ScheduleNode::getSubProblem() { return impl->getSubProblem(); };
bool ScheduleNode::forSubProblem(const int & s) const { return impl->forSubProblem(s); };

const double & ScheduleNode::getDuration() const { return impl->getDuration(); };
const bool & ScheduleNode::isStart() const { return impl->isStart(); };
const bool & ScheduleNode::tilNode() const { return impl->tilNode(); };
const int & ScheduleNode::tilID() const { return impl->tilID(); };

const double & ScheduleNode::distFromZeroConst() const {return impl->distFromZeroConst(); };
const double & ScheduleNode::distToZeroConst() const {return impl->distToZeroConst(); };

double & ScheduleNode::distFromZero() {return impl->distFromZero(); };
double & ScheduleNode::distToZero() {return impl->distToZero(); };

instantiatedOp* ScheduleNode::getOp() const { return impl->getOp(); };

list<Literal*> & ScheduleNode::getPreconditions() { return impl->getPreconditions(); };
list<Literal*> & ScheduleNode::getInvariants() { return impl->getInvariants(); };

list<Literal*> & ScheduleNode::getAddEffects() { return impl->getAddEffects(); };
list<Literal*> & ScheduleNode::getDeleteEffects() { return impl->getDeleteEffects(); };

ScheduleNode* ScheduleNode::getPartner() { return impl->getPartner(); };
void ScheduleNode::setPartner(ScheduleNode* p) { return impl->setPartner(p); };

list<RPGBuilder::NumericPrecondition> & ScheduleNode::getNumericPreconditions() { return impl->getNumericPreconditions(); };
list<RPGBuilder::NumericPrecondition> & ScheduleNode::getNumericInvariants() { return impl->getNumericInvariants(); };
list<RPGBuilder::NumericEffect> & ScheduleNode::getNumericEffects() { return impl->getNumericEffects(); };

ScheduleNode* & ScheduleNode::toOld() { return impl->toOld(); };
ScheduleNode* & ScheduleNode::toNew() { return impl->toNew(); };

void ScheduleNode::preserve(const bool & keepEdges) {
	impl->preserve(keepEdges);
};

void ScheduleNode::rollback() {
	impl->rollback();
}


instantiatedOp* TimelinePoint::afterOp() {

	if ((*after)->isStart()) return 0;
	if (dummy) return 0;
	return (*after)->getOp();

};

CandidateSchedule::~CandidateSchedule() {

	{
		list<TimelinePoint*>::iterator tlItr = timeline.begin();
		const list<TimelinePoint*>::iterator tlEnd = timeline.end();

		for (; tlItr != tlEnd; ++tlItr) delete *tlItr;
	}

	{
		list<ScheduleNode*>::iterator evItr = events.begin();
		const list<ScheduleNode*>::iterator evEnd = events.end();

		for (; evItr != evEnd; ++evItr) delete *evItr;
	}

};

/*	struct ScheduleNodeLT {
		bool operator()(const ScheduleNode* const & a, const ScheduleNode* const & b) const {
			return (a < b);
		}
	};
*/
CandidateSchedule::CandidateSchedule(CandidateSchedule & s, const int without, list<FFEvent> * removedEvents) {

	//s.printEvents();
	
	static const double EPSILON = 0.001;
	timeline.push_back(new TimelinePoint(*(s.timeline.front())));
	timeline.back()->after = events.end();

	//map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> oldToNew;

	//list<ScheduleNode*>::iterator scItr = s.events.begin();
	//const list<ScheduleNode*>::iterator scEnd = s.events.end();

	list<TimelinePoint*>::iterator scItr = s.timeline.begin();
	list<TimelinePoint*>::iterator scEnd = s.timeline.end();

	++scItr;

	list<pair<ScheduleNode*, ScheduleNode*> > postProcess;

	//cout << s.events.size() << "events\n";

	//map<ScheduleNode*, double, ScheduleNodeLT> earliest;

	ScheduleNode* extraWait = 0;

	//ScheduleNode* prev = 0;

	int nodeCount = 1;
	map<ScheduleNode*, int, ScheduleNodeLT> nodeToIndex;
	list<ScheduleNode*> newNodes;
	tilEvents = vector<ScheduleNode*>(s.tilEvents.size());
	int tilSeen = 0;

	int reCount = 0;
	map<ScheduleNode*, int, ScheduleNodeLT> startIndices;

	for (; scItr != scEnd; ++scItr) {
		ScheduleNode* const oldNode = *((*scItr)->after);
		//cout << "-- " << *(oldNode->getOp()) << (oldNode->isStart() ? "start" : "end") << "\n";
		

		set<int> spDeps = oldNode->getSubProblem();
		const bool wasInSP = (oldNode->tilNode() || spDeps.find(without) != spDeps.end());
		spDeps.erase(without);

				
		if (!oldNode->tilNode()) {
			
			if (removedEvents) {
				if (spDeps.empty()) {
					if (oldNode->isStart()) {
						removedEvents->push_back(FFEvent(oldNode->getOp(), oldNode->getDuration(), oldNode->getDuration()));
						if (ScheduleNode* pw = oldNode->getPartner()) {
							startIndices[pw] = reCount;
						}
					} else {
						//const int sInd = startIndices[oldNode];
						//removedEvents->push_back(FFEvent(oldNode->getOp(), sInd, oldNode->getPartner()->getDuration(), oldNode->getPartner()->getDuration()));
					}
					++reCount;
					extraWait = 0;
				}
			}
		}

		if (oldNode->tilNode() || without == -1 || !spDeps.empty()) { // check for excluded subproblem

			ScheduleNode* const newNode = new ScheduleNode(*oldNode);


			if (newNode->tilNode()) {
				tilEvents[tilSeen++] = newNode;
			} else {
				if (without != -1) {
					newNode->removeSubProblem(without);
				}
			}

			nodeToIndex.insert(pair<ScheduleNode*, int>(newNode, nodeCount));
			++nodeCount;
			newNodes.push_back(newNode);

			
			
			list<pair<ScheduleNode*, pair<double, double> > > & pres = oldNode->getPredecessors();

			{

				//bool needExtra = true;

				list<pair<ScheduleNode*, pair<double, double> > >::iterator pItr = pres.begin();
				const list<pair<ScheduleNode*, pair<double, double> > >::iterator pEnd = pres.end();

				for (; pItr != pEnd; ++pItr) {
					if (!pItr->first) {
						newNode->scheduleAfter(0, pItr->second);
						addEdgeFromZero(newNode, pItr->second.first, pItr->second.second);
						//if (!prev) needExtra = false;
					} else {
						map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(pItr->first);
						if (findNewSN != oldToNew.end()) { // if node hasn't been deleted from the schedule
							ScheduleNode* const deref = findNewSN->second;
							newNode->scheduleAfter(deref, pItr->second);
							deref->scheduleBefore(newNode, pItr->second);
							//if (deref == prev) {
							//	needExtra = false;
							//}
						}
					}

					
					
				}

				/*if (needExtra) {
					if (prev) {
						newNode->scheduleAfter(prev, EPSILON, DBL_MAX);
						prev->scheduleBefore(newNode, EPSILON, DBL_MAX);
					} else {
						newNode->scheduleAfter(0, EPSILON, DBL_MAX);
						addEdgeFromZero(newNode, EPSILON, DBL_MAX);
					}
				}

				prev = newNode;*/
			}

			oldToNew.insert(pair<ScheduleNode*,ScheduleNode*>(oldNode, newNode));

			if (!newNode->tilNode() && !newNode->isStart()) {
				postProcess.push_back(pair<ScheduleNode*, ScheduleNode*>(oldNode, newNode));
			}

			if (without == -1) {
				events.push_back(newNode);
	
				TimelinePoint * const newTP = new TimelinePoint(*(timeline.back()), newNode->getAddEffects(), newNode->getDeleteEffects());
	
				newTP->after = events.end();
				--(newTP->after);
	
				timeline.push_back(newTP);
				//double thisTime;
				/*{
					map<ScheduleNode*, double, ScheduleNodeLT>::iterator feItr = earliest.find(oldNode);
					if (feItr == earliest.end()) {
						thisTime = 0.0;
					} else {
						thisTime = feItr->second;
					}
				}*/
				//newTP->timestamp = thisTime;
				//cout << "Giving TS of " << thisTime << " as opposed to old TS of " << (*scItr)->timestamp << "\n";
	
				newTP->timestamp = (*scItr)->timestamp;
			}

			
			/*{
				list<ScheduleNode*> & succs = oldNode->getSuccessors();

				list<ScheduleNode*>::iterator pItr = succs.begin();
				const list<ScheduleNode*>::iterator pEnd = succs.end();

				for (; pItr != pEnd; ++pItr) {
					ScheduleNode* const succNode = *pItr;
					const double newVal = thisTime + ((oldNode->getPartner() == succNode) ? oldNode->getDuration() : EPSILON);
					map<ScheduleNode*, double, ScheduleNodeLT>::iterator feItr = earliest.find(succNode);
					if (feItr == earliest.end()) {
						earliest[succNode] = newVal;
					} else {
						if (newVal > feItr->second) feItr->second = newVal;
					}
					
				}
			}*/

			if (removedEvents) {
				if (wasInSP) {
					if (extraWait) {
						//removedEvents->push_back(FFEvent(extraWait, false));
						++reCount;
					}
					//removedEvents->push_back(FFEvent(newNode,  true));
					++reCount;
					extraWait = 0;
				} else {
					extraWait = newNode;
				}
			}

		}


	}

	list<pair<ScheduleNode*, ScheduleNode*> >::iterator pItr = postProcess.begin();
	const list<pair<ScheduleNode*, ScheduleNode*> >::iterator pEnd = postProcess.end();

	for (; pItr != pEnd; ++pItr) {
		map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(pItr->first->getPartner());
		assert(findNewSN != oldToNew.end());
		ScheduleNode* const deref = findNewSN->second;
		pItr->second->setPartner(deref);
		deref->setPartner(pItr->second);

	}

	if (without != -1) {
		{
	
			vector<vector<double> > floydMatrix(nodeCount);
			for (int i = 0; i < nodeCount; ++i) {
				vector<double> & toFill = floydMatrix[i] = vector<double>(nodeCount);
				for (int j = 0; j < nodeCount; ++j) {
					toFill[j] = DBL_MAX;
				}
				toFill[i] = 0.0;
			}
	
			{
				list<ScheduleNode*>::iterator nnItr = newNodes.begin();
				const list<ScheduleNode*>::iterator nnEnd = newNodes.end();
	
	
				for (int xIndex = 1; nnItr != nnEnd; ++nnItr, ++xIndex) {
	
					list<pair<ScheduleNode*, pair<double, double> > > & pres = (*nnItr)->getPredecessors();
	
					list<pair<ScheduleNode*, pair<double, double> > >::iterator pItr = pres.begin();
					const list<pair<ScheduleNode*, pair<double, double> > >::iterator pEnd = pres.end();
	
					for (; pItr != pEnd; ++pItr) {
						if (!pItr->first) {
							floydMatrix[xIndex][0] = -(pItr->second.first);
							floydMatrix[0][xIndex] = (pItr->second.second);
						} else {
							const int yIndex = nodeToIndex[pItr->first];
							
							floydMatrix[xIndex][yIndex] = -(pItr->second.first);
							floydMatrix[yIndex][xIndex] = (pItr->second.second);							
						}
	
						
						
					}
				}
			}
	
			for (int k = 0; k < nodeCount; ++k) {
		
				vector<double> & fromKTo = floydMatrix[k];
	
				for (int i = 0; i < nodeCount; ++i) {
	
					vector<double> & fromITo = floydMatrix[i];
					const double distIK = fromITo[k];
					if (distIK != DBL_MAX) {
						for (int j = 0; j < nodeCount; ++j) {
	
							double newDist = fromKTo[j];
							if (newDist != DBL_MAX) {
								newDist += distIK;
								double & distIJ = fromITo[j];
								if (distIJ > newDist) {
									distIJ = newDist;
								}
							}
	
						}
					}
				}
			}
	
			
			list<pair<ScheduleNode*, double> > floydOpenList;
			
			
			{
				list<ScheduleNode*>::iterator nLookup = newNodes.begin();
				const list<ScheduleNode*>::iterator nLookupEnd = newNodes.end();
			
				for (int xIndex = 1; nLookup != nLookupEnd; ++nLookup, ++xIndex) {
			
					ScheduleNode* const currSN = *nLookup;
					
					const double min = floydMatrix[xIndex][0];
					const double max = floydMatrix[0][xIndex];
	
					currSN->distToZero() = min;
					currSN->distFromZero() = max;
	
					double currTS = -min;
			
					if (floydOpenList.empty()) {
						floydOpenList.push_back(pair<ScheduleNode*, double>(currSN, currTS));
					} else {
						list<pair<ScheduleNode*, double> >::iterator insItr = floydOpenList.begin();
						const list<pair<ScheduleNode*, double> >::iterator insEnd = floydOpenList.end();
			
						//for (; insItr != insEnd && (insItr->second <= currTS); ++insItr);
			
						for (; insItr != insEnd && ((insItr->second - currTS) < (EPSILON / 2)); ++insItr);
			
						floydOpenList.insert(insItr, pair<ScheduleNode*, double>(currSN, currTS));
					}
				}
			}
	
			list<pair<ScheduleNode*, double> >::iterator olItr = floydOpenList.begin();
			const list<pair<ScheduleNode*, double> >::iterator olEnd = floydOpenList.end();
	
			for (; olItr != olEnd; ++olItr) {
	
	
	
				ScheduleNode* const newNode = (olItr->first);
	
				events.push_back(newNode);
	
				TimelinePoint * const newTP = new TimelinePoint(*(timeline.back()), newNode->getAddEffects(), newNode->getDeleteEffects());
	
				newTP->after = events.end();
				--(newTP->after);
	
				timeline.push_back(newTP);
				//double thisTime;
				/*{
					map<ScheduleNode*, double, ScheduleNodeLT>::iterator feItr = earliest.find(oldNode);
					if (feItr == earliest.end()) {
						thisTime = 0.0;
					} else {
						thisTime = feItr->second;
					}
				}*/
				//newTP->timestamp = thisTime;
				//cout << "Giving TS of " << thisTime << " as opposed to old TS of " << (*scItr)->timestamp << "\n";
	
				newTP->timestamp = olItr->second;
	
			}
	
	
		}

	}

	//enforceOrder();

	//printEvents();
}

CandidateSchedule::CandidateSchedule(CandidateSchedule & s, map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> & newToOld, const bool skipTimeline) {

	//s.printEvents();
	
//	static const double EPSILON = 0.001;
	timeline.push_back(new TimelinePoint(*(s.timeline.front())));
	timeline.back()->after = events.end();

	list<pair<ScheduleNode*, ScheduleNode*> > postProcess;
	tilEvents = vector<ScheduleNode*>(s.tilEvents.size());
	int tilSeen = 0;

	
	if (skipTimeline) {
		const list<ScheduleNode*>::iterator scBegin = s.events.begin();
		const list<ScheduleNode*>::iterator scEnd = s.events.end();

		for (list<ScheduleNode*>::iterator scItr = scBegin; scItr != scEnd; ++scItr) {
			ScheduleNode* const oldNode = *scItr;
			ScheduleNode* const newNode = new ScheduleNode(*oldNode);
			events.push_back(newNode);

			oldToNew.insert(pair<ScheduleNode*,ScheduleNode*>(oldNode, newNode));
			newToOld.insert(pair<ScheduleNode*,ScheduleNode*>(newNode, oldNode));
	
			oldNode->toNew() = newNode;
			newNode->toOld() = oldNode;
	
			if (newNode->tilNode()) {
				tilEvents[tilSeen++] = newNode;
			} else {
			
				if (!newNode->isStart()) {
					postProcess.push_back(pair<ScheduleNode*, ScheduleNode*>(oldNode, newNode));
				}
			}



		}

		for (list<ScheduleNode*>::iterator scItr = scBegin; scItr != scEnd; ++scItr) {
			ScheduleNode* const oldNode = *scItr;
			ScheduleNode* const newNode = oldNode->toNew();

			{
				list<pair<ScheduleNode*, pair<double, double> > > & pres = oldNode->getPredecessors();
		
				{
					list<pair<ScheduleNode*, pair<double, double> > >::iterator pItr = pres.begin();
					const list<pair<ScheduleNode*, pair<double, double> > >::iterator pEnd = pres.end();
		
					for (; pItr != pEnd; ++pItr) {
						if (pItr->first) {
		//					map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(pItr->first);
		//					assert(findNewSN != oldToNew.end()); // no deletion so this shouldn't happen
		//					ScheduleNode* const deref = findNewSN->second;
							ScheduleNode* const deref = pItr->first->toNew();
							newNode->scheduleAfter(deref, pItr->second);
	//						deref->scheduleBefore(newNode, pItr->second);
							
						} else {
							newNode->scheduleAfter(0, pItr->second);
	//						addEdgeFromZero(newNode, pItr->second.first, pItr->second.second);					
						}
						
					}
				}
			}

			{

				list<pair<ScheduleNode*, pair<double, double> > > & pres = oldNode->getSuccessors();
		
				{
					list<pair<ScheduleNode*, pair<double, double> > >::iterator pItr = pres.begin();
					const list<pair<ScheduleNode*, pair<double, double> > >::iterator pEnd = pres.end();
		
					for (; pItr != pEnd; ++pItr) {
						if (pItr->first) {
		//					map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(pItr->first);
		//					assert(findNewSN != oldToNew.end()); // no deletion so this shouldn't happen
		//					ScheduleNode* const deref = findNewSN->second;
							ScheduleNode* const deref = pItr->first->toNew();
							newNode->scheduleBefore(deref, pItr->second);
	//						deref->scheduleBefore(newNode, pItr->second);
							
						} else {
							newNode->scheduleBefore(0, pItr->second);
	//						addEdgeFromZero(newNode, pItr->second.first, pItr->second.second);					
						}
						
					}
				}
			}

		}

		
		edgesFromZero = s.edgesFromZero;

		{
			list<pair<ScheduleNode*, pair<double, double> > >::iterator pItr = edgesFromZero.begin();
			const list<pair<ScheduleNode*, pair<double, double> > >::iterator pEnd = edgesFromZero.end();

			for (; pItr != pEnd; ++pItr) {
				if (pItr->first) {
					pItr->first = pItr->first->toNew();
				}
				
			}
		}

	} else {
		//map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> oldToNew;
	
		//list<ScheduleNode*>::iterator scItr = s.events.begin();
		//const list<ScheduleNode*>::iterator scEnd = s.events.end();
	
		list<TimelinePoint*>::iterator scItr = s.timeline.begin();
		list<TimelinePoint*>::iterator scEnd = s.timeline.end();
	
		++scItr;
	
	
		//cout << s.events.size() << "events\n";
	
		//map<ScheduleNode*, double, ScheduleNodeLT> earliest;
	
	
		for (; scItr != scEnd; ++scItr) {
			ScheduleNode* const oldNode = *((*scItr)->after);
			//cout << "-- " << *(oldNode->getOp()) << (oldNode->isStart() ? "start" : "end") << "\n";
			
			ScheduleNode* const newNode = new ScheduleNode(*oldNode);
	
	
			events.push_back(newNode);
			
			list<pair<ScheduleNode*, pair<double, double> > > & pres = oldNode->getPredecessors();
	
			{
				list<pair<ScheduleNode*, pair<double, double> > >::iterator pItr = pres.begin();
				const list<pair<ScheduleNode*, pair<double, double> > >::iterator pEnd = pres.end();
	
				for (; pItr != pEnd; ++pItr) {
					if (pItr->first) {
	//					map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(pItr->first);
	//					assert(findNewSN != oldToNew.end()); // no deletion so this shouldn't happen
	//					ScheduleNode* const deref = findNewSN->second;
						ScheduleNode* const deref = pItr->first->toNew();
						newNode->scheduleAfter(deref, pItr->second);
						deref->scheduleBefore(newNode, pItr->second);
						
					} else {
						newNode->scheduleAfter(0, pItr->second);
						addEdgeFromZero(newNode, pItr->second.first, pItr->second.second);					
					}
					
				}
			}
	
			oldToNew.insert(pair<ScheduleNode*,ScheduleNode*>(oldNode, newNode));
			newToOld.insert(pair<ScheduleNode*,ScheduleNode*>(newNode, oldNode));
	
			oldNode->toNew() = newNode;
			newNode->toOld() = oldNode;
	
			if (newNode->tilNode()) {
				tilEvents[tilSeen++] = newNode;
			} else {
			
				if (!newNode->isStart()) {
					postProcess.push_back(pair<ScheduleNode*, ScheduleNode*>(oldNode, newNode));
				}
			}
			
			
			TimelinePoint * const newTP = new TimelinePoint(*(timeline.back()), newNode->getAddEffects(), newNode->getDeleteEffects());
	
			newTP->after = events.end();
			--(newTP->after);
	
			timeline.push_back(newTP);
		
			newTP->timestamp = (*scItr)->timestamp;

			
			
	
	
		}
	}

	list<pair<ScheduleNode*, ScheduleNode*> >::iterator pItr = postProcess.begin();
	const list<pair<ScheduleNode*, ScheduleNode*> >::iterator pEnd = postProcess.end();

	for (; pItr != pEnd; ++pItr) {
//		map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(pItr->first->getPartner());
//		assert(findNewSN != oldToNew.end());
//		ScheduleNode* const deref = findNewSN->second;
		ScheduleNode* const deref = pItr->first->getPartner()->toNew();
		pItr->second->setPartner(deref);
		deref->setPartner(pItr->second);

	}

	if (s.m_after) {
		m_after = s.m_after->toNew();
		m_afterNew = s.m_afterNew;
	} else {
		m_after = 0;
	}
	if (s.m_afterReal) {
		m_afterReal = s.m_afterReal->toNew();
		m_afterRealNew = s.m_afterRealNew;
	} else {
		m_afterReal = 0;
	}
	
	m_cl = s.m_cl;
	m_tilInPlan = s.m_tilInPlan;

	//enforceOrder();

	//printEvents();
}

void breakHere() {

};

struct Edge {
	
	ScheduleNode* from;
	ScheduleNode* to;
	double min;
	double max;
	
	Edge(ScheduleNode* i, ScheduleNode* j, const double & a, const double & b) : from(i), to(j), min(a), max(b) {};

};

class QueueSet {

private:

	list<pair<ScheduleNode*, set<ScheduleNode*, ScheduleNodeLT>::iterator> > Q;
	set<ScheduleNode*, ScheduleNodeLT> qSet;

public:

	QueueSet() {};
	~QueueSet() {};

	void push_back(ScheduleNode* const u) {
		if (qSet.find(u) == qSet.end()) {
			Q.push_back(pair<ScheduleNode*, set<ScheduleNode*, ScheduleNodeLT>::iterator>(u, qSet.insert(u).first));
		}
	}

	bool empty() const {
		return (Q.empty());
	}

	ScheduleNode* pop_front() {

		ScheduleNode* const toReturn = Q.front().first;
		qSet.erase(Q.front().second);
		Q.pop_front();

		return toReturn;
	
	}
	
};

bool Propagation(Edge & e, list<pair<ScheduleNode*, pair<double, double> > > & edgesFromZero) {

	const bool bfDebug = false;
	if (bfDebug) cout << "Propagating\n";
	static list<pair<ScheduleNode*, pair<double, double> > > negativeEdgesFromZero;
	QueueSet Q;

	Q.push_back(e.from);
	Q.push_back(e.to);

	set<ScheduleNode*, ScheduleNodeLT> LB;
	set<ScheduleNode*, ScheduleNodeLT> UB;
	
	LB.insert(e.from); UB.insert(e.from);
	LB.insert(e.to); UB.insert(e.to);
	
	set<ScheduleNode*, ScheduleNodeLT> NEWfromZero;
	set<ScheduleNode*, ScheduleNodeLT> UBPfromZero;
	set<ScheduleNode*, ScheduleNodeLT> LBPfromZero;

	if (e.from) {
		e.from->NEW.insert(e.to);
	} else {
		NEWfromZero.insert(e.to);
	}

	if (e.to) {
		e.to->NEW.insert(e.from);
	} else {
		NEWfromZero.insert(e.from);
	}

	{
		bool foundAlready = false;
		{
			if (e.from) {
				list<pair<ScheduleNode*, pair<double, double> > > & succs = e.from->getSuccessors();

				list<pair<ScheduleNode*, pair<double, double> > >::iterator sItr = succs.begin();
				const list<pair<ScheduleNode*, pair<double, double> > >::iterator sEnd = succs.end();

				for (; sItr != sEnd; ++sItr) {
					if (sItr->first == e.to) {
						if (e.min > sItr->second.first) sItr->second.first = e.min;
						if (e.max < sItr->second.second) sItr->second.second = e.max;
						foundAlready = true;
						break;
					}
				}
			} else {
				list<pair<ScheduleNode*, pair<double, double> > >::iterator sItr = edgesFromZero.begin();
				const list<pair<ScheduleNode*, pair<double, double> > >::iterator sEnd = edgesFromZero.end();

				for (; sItr != sEnd; ++sItr) {
					if (sItr->first == e.to) {
						if (e.min > sItr->second.first) sItr->second.first = e.min;
						if (e.max < sItr->second.second) sItr->second.second = e.max;
						foundAlready = true;
						break;
					}
				}
			}

			if (foundAlready) {

				list<pair<ScheduleNode*, pair<double, double> > > & succs = e.to->getPredecessors();

				list<pair<ScheduleNode*, pair<double, double> > >::iterator sItr = succs.begin();
				const list<pair<ScheduleNode*, pair<double, double> > >::iterator sEnd = succs.end();

				for (; sItr != sEnd; ++sItr) {
					if (sItr->first == e.to) {
						if (e.min > sItr->second.first) sItr->second.first = e.min;
						if (e.max < sItr->second.second) sItr->second.second = e.max;
						foundAlready = true;
						break;
					}
				}

			} else {

				if (e.from) {
					e.from->scheduleBefore(e.to, e.min, e.max);
				} else {
					edgesFromZero.push_back(pair<ScheduleNode*, pair<double, double> >(e.to, pair<double, double>(e.min, e.max)));
				}
				e.to->scheduleAfter(e.from, e.min, e.max);

			}
		}
	}
	

	while (!Q.empty()) {

		ScheduleNode* const u = Q.pop_front();

		if (bfDebug) {
			if (u) {
				cout << "Next in queue: " << *(u->getOp()) << ", " << (u->isStart() ? "start" : "end") << "\n";
			} else {
				cout << "Next in queue: time 0\n";
			} 
		}

		if (UB.find(u) != UB.end()) {

			const double d0u = (u ? u->distFromZero() : 0.0);

			if (d0u != DBL_MAX) {

				if (bfDebug) {
					if (u) {
						cout << "Edges out of " << *(u->getOp()) << ", " << (u->isStart() ? "start" : "end") << "\n";
					} else {
						cout << "Edges out of time zero\n";
					}
				}
				for (int pass = 0; pass < 2; ++pass) {
					if (bfDebug) {
						if (pass) {
							cout << "Edges to predecessors: ";
						} else {
							cout << "Edges to successors: ";
						}
					}
	
					list<pair<ScheduleNode*, pair<double, double> > > & edgesOut = (pass ? (u ? u->getPredecessors() : negativeEdgesFromZero) : (u ? u->getSuccessors() : edgesFromZero) );
					
					if (bfDebug) cout << edgesOut.size() << "\n";
	
					list<pair<ScheduleNode*, pair<double, double> > >::iterator eoItr = edgesOut.begin();
					const list<pair<ScheduleNode*, pair<double, double> > >::iterator eoEnd = edgesOut.end();
	
					for (; eoItr != eoEnd; ++eoItr) {
	
						ScheduleNode* const v = eoItr->first;
						if (v) {
							const double w = (pass ? (-1.0 * eoItr->second.first) : eoItr->second.second);
							
							if (w != DBL_MAX) {
								if (bfDebug) cout << "one of weight " << w << "\n";

								const double d0uplusWuv = d0u + w;
								double & d0v = v->distFromZero();
								if (d0uplusWuv < d0v) {
									d0v = d0uplusWuv;
									if (d0v + v->distToZero() < 0.0) {
										return false;
									} else {
										if ((u ? (u->NEW.find(v) != u->NEW.end()) : (NEWfromZero.find(v) != NEWfromZero.end()))) {
											set<ScheduleNode*, ScheduleNodeLT> & UBPset = (u ? u->UBP : UBPfromZero);
											if (UBPset.find(v) != UBPset.end()) {
												return false;
											} else {
												UBPset.insert(v);
											}
										}
									}
									UB.insert(v);
									Q.push_back(v);
									
								}
								
							} else {
								if (bfDebug) cout << "one of weight inf\n";
							}
						} else {
							if (bfDebug) cout << "One back to time zero, but we can't reduce d00\n";
						}
	
					}
				}
			}
		}

		if (LB.find(u) != LB.end()) {

			const double du0 = (u ? u->distToZero() : 0.0);

			if (du0 != DBL_MAX) {
				if (bfDebug) {
					if (u) {
						cout << "Edges into " << *(u->getOp()) << ", " << (u->isStart() ? "start" : "end") << "\n";
					} else {
						cout << "Edges into time zero\n";
					}
				}
				for (int pass = 0; pass < 2; ++pass) {
					if (bfDebug) {
						if (pass) {
							cout << "Edges to predecessors: ";
						} else {
							cout << "Edges to successors: ";
						}
					}
					list<pair<ScheduleNode*, pair<double, double> > > & edgesIn = (pass ? (u ? u->getSuccessors() : edgesFromZero) : (u ? u->getPredecessors() : negativeEdgesFromZero) );
					
					if (bfDebug) cout << edgesIn.size() << "\n";
	
					list<pair<ScheduleNode*, pair<double, double> > >::iterator eiItr = edgesIn.begin();
					const list<pair<ScheduleNode*, pair<double, double> > >::iterator eiEnd = edgesIn.end();
	
					for (; eiItr != eiEnd; ++eiItr) {
	
						ScheduleNode* const v = eiItr->first;
						if (v) {
							
							const double w = (pass ? (-1.0 * eiItr->second.first) : eiItr->second.second);
		
							if (w != DBL_MAX) {
								if (bfDebug) cout << "one of weight " << w << "\n";
								const double du0plusWvu = du0 + w;
								double & dv0 = v->distToZero();
	
								if (du0plusWvu < dv0) {
									dv0 = du0plusWvu;
									
									if (v->distFromZero() + dv0 < 0) {
										return false;
									} else { // figure 2, lines 20a to 20f
										if ((v ? (v->NEW.find(u) != v->NEW.end()) : (NEWfromZero.find(u) != NEWfromZero.end()))) {
											set<ScheduleNode*, ScheduleNodeLT> & LBPset = (v ? v->LBP : LBPfromZero);
											if (LBPset.find(u) != LBPset.end()) {
												return false;
											} else {
												LBPset.insert(u);
											}
										}
									}

									LB.insert(v);
									Q.push_back(v);
									
	
								}
							} else {
								if (bfDebug) cout << "one of weight inf\n";
							}
							
						} else {
							if (bfDebug) cout << "One back to time zero, but we can't reduce d00\n";
						
						}
	
					}
				}
			}
		}

		LB.erase(u);
		UB.erase(u);

	}

	if (e.from) {
		e.from->NEW.clear();
		e.from->LBP.clear();
		e.from->UBP.clear();
	}
	
	if (e.to) {
		e.to->NEW.clear();
		e.to->LBP.clear();
		e.to->UBP.clear();
	}




	return true;
}


CandidateSchedule * CandidateSchedule::getScheduleWithActions(list<TimelinePoint*>::iterator & after, list<FFEvent> & plan, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >  & relaxedPlan, map<int, list<list<StartEvent>::iterator > > compulsaryEnds, map<int, map<int, ScheduleNode*> > * rememberOS, const int & sp, bool debugMerging, map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> * newToOld, list<ScheduleNode*> * impliedWaits, int onlyMFS, ScheduleNode* * onlyPenalise, const bool skipTimeline) {
	
	if (debugMerging) {
		cout << "\nMerging actions into schedule\n";
		printEvents();
	}
	if (GlobalSchedule::minFlawScheduling == 0) {
		onlyMFS = 0;
	} else if (GlobalSchedule::minFlawScheduling == 2) {
		onlyMFS = INT_MAX;
	}
	
	static const double EPSILON = 0.001;
	const bool debugNewMerge = false;
	CandidateSchedule * toReturn = new CandidateSchedule(new TimelinePoint(*(timeline.front())));
	toReturn->timeline.back()->after = toReturn->events.end();

	map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> & oldToNew = toReturn->oldToNew;
//	map<ScheduleNode*, int, ScheduleNodeLT> fanIn;

	//map<ScheduleNode*, double, ScheduleNodeLT> earliest;

	const int spCount = Decomposition::howMany();
	
	vector<ScheduleNode*> lastForSPVec(spCount);
	ScheduleNode* lastThatDeletesCurrGoal = (ScheduleNode*) 0;

	map<Literal*, ScheduleNode*, LiteralLT> lastForSP;
	map<Literal*, ScheduleNode*, LiteralLT> lastDeletors;

	for (int i = 0; i < spCount; ++i) {
		lastForSPVec[i] = (ScheduleNode*) 0;
	}
	
	int targetNodeCount = 0;

	const bool debugMinFlawScheduling = false;

	Literal* const newTopLevelGoal = (Decomposition::getSubproblem(sp)->goals.empty() ? 0 : Decomposition::getSubproblem(sp)->goals.front());

	list<ScheduleNode*> oldTotalOrdering;
	map<ScheduleNode*, int, ScheduleNodeLT> oldTOIndices;

	map<ScheduleNode*, int, ScheduleNodeLT> newNodeIndices;

	map<ScheduleNode*, map<ScheduleNode*, double, ScheduleNodeLT>, ScheduleNodeLT > stnConstraints;

	list<ScheduleNode*> garbage;

	int nodeIndexCounter = 0;

	//ScheduleNode* graphRootNode = 0;
	int tilSeen = 0;
	toReturn->tilEvents = vector<ScheduleNode*>(tilEvents.size());

	{
		list<pair<ScheduleNode*, ScheduleNode*> > postProcess;

		list<ScheduleNode*>::iterator scItr = events.begin();
		const list<ScheduleNode*>::iterator scEnd = events.end();
	
	
		for (int i = 0; scItr != scEnd; ++scItr, ++i, ++nodeIndexCounter) {
			
			ScheduleNode* const oldNode = *scItr;
	
			ScheduleNode* const newNode = new ScheduleNode(*oldNode);
			garbage.push_back(newNode);
			if (newToOld) {
				(*newToOld)[newNode] = oldNode;
				if (debugMerging) cout << "New node " << newNode << " maps to old node " << oldNode << "\n";
			}

			++targetNodeCount;

			oldTotalOrdering.push_back(newNode);
			oldTOIndices[newNode] = i;

			/*if (i == 0) {
				graphRootNode = newNode;
			}*/

			if (debugMinFlawScheduling) {
				cout << "Old node " << newNode << " with index " << i << "\n";
			}
			if (debugMerging && false) cout << "Copying across node for " << (oldNode->isStart() ? "start" : "end") << " of " << *(oldNode->getOp()) << "\n";

			//toReturn->events.push_back(newNode);

			list<pair<ScheduleNode*, pair<double, double> > > & pres = oldNode->getPredecessors();
	
			{
				list<pair<ScheduleNode*, pair<double, double> > >::iterator pItr = pres.begin();
				const list<pair<ScheduleNode*, pair<double, double> > >::iterator pEnd = pres.end();
	
					
				if (debugMerging) cout << "Has " << pres.size() << " predecessors\n";
				for (; pItr != pEnd; ++pItr) {
					if (pItr->first) {
						map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(pItr->first);
						assert(findNewSN != oldToNew.end()); // this really shouldn't happen - no nodes are being removed
						
						ScheduleNode* const deref = findNewSN->second;
						newNode->scheduleAfter(deref, pItr->second);
						deref->scheduleBefore(newNode, pItr->second);
						
					} else {
						newNode->scheduleAfter(0, pItr->second);
						//cout << "Scheduling old node " << i << " to be after zero\n";
						toReturn->addEdgeFromZero(newNode, pItr->second.first, pItr->second.second);
					}
				}
				
			}
	
			oldToNew.insert(pair<ScheduleNode*,ScheduleNode*>(oldNode, newNode));

			if (newNode->tilNode()) {
			
				toReturn->tilEvents[tilSeen++] = newNode;
			
			} else {
				if (!newNode->isStart()) {
					postProcess.push_back(pair<ScheduleNode*, ScheduleNode*>(oldNode, newNode));
				} else {
					set<int> & sp = newNode->getSubProblem();
					set<int>::iterator spItr = sp.begin();
					const set<int>::iterator spEnd = sp.end();
					for (; spItr != spEnd; ++spItr) {
						lastForSPVec[*spItr] = newNode;
					}
				}
			}
			
			if (newTopLevelGoal) {
				
				list<Literal*> & deletes = newNode->getDeleteEffects();				
				
				list<Literal*>::iterator delItr = deletes.begin();
				const list<Literal*>::iterator delEnd = deletes.end();

				for (; delItr != delEnd; ++delItr) {
					Literal* const currLit = *delItr;
					//cout << "Node deletes " << *currLit << "\n";
					if (currLit->getStateID() == newTopLevelGoal->getStateID()) {
						lastThatDeletesCurrGoal = newNode;
						break;
					}
					
				}
			
			}
			
		}
		
		list<pair<ScheduleNode*, ScheduleNode*> >::iterator pItr = postProcess.begin();
		const list<pair<ScheduleNode*, ScheduleNode*> >::iterator pEnd = postProcess.end();

		for (; pItr != pEnd; ++pItr) {
			map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(pItr->first->getPartner());
			assert(findNewSN != oldToNew.end());
			ScheduleNode* const deref = findNewSN->second;
			pItr->second->setPartner(deref);
			deref->setPartner(pItr->second);

			/*const double actDur = deref->getDuration();

			if (debugMerging) cout << "Setting duration of action to " << actDur << "\n";

			stnConstraints[deref][pItr->second] = actDur; // separation of at most duration
			stnConstraints[pItr->second][deref] = -actDur; // separation of at least duration*/
		}
	}
	for (int i = 0; i < spCount; ++i) {
		if (lastForSPVec[i]) {
			if (debugNewMerge) cout << "Last node for subproblem " << i << " is " << *(lastForSPVec[i]->getOp()) << "\n";
			if (Decomposition::getSubproblem(i)->goals.empty()) {
				// do nothing....
			} else {
				lastForSP[Decomposition::getSubproblem(i)->goals.front()] = lastForSPVec[i];
				if (debugNewMerge) cout << "Associated with goal literal " << *(Decomposition::getSubproblem(i)->goals.front()) << "\n";
			}
		}
	}


	ScheduleNode* afterNode = 0;
	if (after != timeline.end()) {
		list<ScheduleNode*>::iterator const lookFor = (*after)->after;
		if (lookFor != events.end()) {
			map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(*lookFor);
			assert(findNewSN != oldToNew.end()); // this really shouldn't happen - no nodes are being removed						
			afterNode = findNewSN->second;
			afterNode->addSubProblem(sp);
			if (debugMerging) cout << "Planning in subproblem started after node " << afterNode << "\n";
		}
	}
	
	list<ScheduleNode*> planNodesToPokeAround;
	ScheduleNode* noEarlierThan = 0;

	const int considerUpTo = onlyMFS;

	//bool subproblemSolutionRooted = false;

	list<Edge> newEdges;
	int tilInPlan = 0;
	{
		

		ScheduleNode* prevTO = 0;
		ScheduleNode* prevReal = 0;

		bool prevTOnew = false;
		bool prevRealNew = false;

		map<int, map<int, ScheduleNode*> > localOpenStarts;
		map<int, map<int, ScheduleNode*> > & openStarts = (rememberOS ? *rememberOS : localOpenStarts);

		if (false) {
			cout << "Adding from plan:\n";

			list<FFEvent>::iterator planItr = plan.begin();
			const list<FFEvent>::iterator planEnd = plan.end();
		
			for (int i = 0; planItr != planEnd; ++planItr, ++i) {
				if (planItr->action) {
					cout << i << ": " << *(planItr->action) << ", " << (planItr->time_spec == VAL::E_AT_START ? "start" : "end") << "\n";
				} else if (planItr->wait) {
					cout << i << ": wait for " << *(planItr->wait->getOp()) << " " << (planItr->wait->isStart() ? "start" : "end");
					if (planItr->getEffects) cout << ", getting its effects";
					cout << "\n";
				} else {
					cout << i << ": null node!\n";
					assert(false);
				}
			}

		}
		int cl = 0;
		{
			
	
			list<FFEvent>::iterator npItr = plan.begin();
			const list<FFEvent>::iterator npEnd = plan.end();
	
			
	
			for (; npItr != npEnd; ++npItr, ++cl) {
		
				if (npItr->action) {
					if (npItr->time_spec == VAL::E_AT_START) {
						if (debugMerging || debugMinFlawScheduling) cout << "New: start of " << *(npItr->action) << "\n";
						ScheduleNode* const startNode = new ScheduleNode(npItr->action, sp, true);
						garbage.push_back(startNode);
						startNode->distFromZero() = DBL_MAX;
						startNode->distToZero() = -EPSILON;

						{
							startNode->scheduleAfter(0, EPSILON, DBL_MAX);
							toReturn->addEdgeFromZero(startNode, EPSILON, DBL_MAX);
						}

						if (onlyPenalise) *onlyPenalise = startNode;
			
						/*planNodesToPokeAround.push_back(startNode);
						if (debugMinFlawScheduling) {
							cout << "Put " << startNode << " on queue to be min-flaw scheduled\n";
						}*/
						//fanIn[startNode] = 0;
						//earliest[startNode] = 0.0;
						++targetNodeCount;
						
						newNodeIndices[startNode] = nodeIndexCounter++;	
	
						if (afterNode) {
							if (debugMerging) cout << "Start put after afterNode " << afterNode << "\n";
							newEdges.push_back(Edge(afterNode, startNode, EPSILON, DBL_MAX));
							afterNode = 0;
						}
		
						if (prevTO) {
							if (debugMerging) cout << "Start put after prevTO " << prevTO << "\n";
							if (debugMinFlawScheduling) cout << prevTO << " before " << startNode << "\n";
							newEdges.push_back(Edge(prevTO, startNode, EPSILON, DBL_MAX));
							//++(fanIn[startNode]);
							//stnConstraints[startNode][prevTO] = -EPSILON;
						}
		
						if (prevReal && prevReal != prevTO) {
							newEdges.push_back(Edge(prevReal, startNode, EPSILON, DBL_MAX));
						}
				
		
						{
							
							list<Literal*> & deletes = startNode->getDeleteEffects();				
							
							list<Literal*>::iterator delItr = deletes.begin();
							const list<Literal*>::iterator delEnd = deletes.end();
		
							for (; delItr != delEnd; ++delItr) {
								Literal* const currLit = *delItr;
								//cout << "New node deletes " << *currLit << "\n";
								map<Literal*, ScheduleNode*, LiteralLT>::iterator gwFind = lastForSP.find(currLit);
								if (gwFind != lastForSP.end()) {
									//cout << "Last deletor for " << *currLit << " is new node " << *(startNode->getOp()) << "\n";
									lastDeletors[currLit] = startNode;
								}
							}
						
						}
	
						prevTO = startNode;
						prevReal = startNode;

						prevTOnew = true;
						prevRealNew = true;
						
						openStarts[npItr->action->getID()][cl] = startNode;

						assert(openStarts[npItr->action->getID()][cl] == startNode);
						//cout << "Start of action " << npItr->action->getID() << " at step " << cl << " maps to " << startNode << "\n";
	
					} else {
						if (debugMerging || debugMinFlawScheduling) cout << "New: end of " << *(npItr->action) << "\n";
						ScheduleNode* const endNode = new ScheduleNode(npItr->action, sp, false);
						garbage.push_back(endNode);
						endNode->distFromZero() = DBL_MAX;
						endNode->distToZero() = -EPSILON;

						{
							endNode->scheduleAfter(0, EPSILON, DBL_MAX);
							toReturn->addEdgeFromZero(endNode, EPSILON, DBL_MAX);
						}

						if (onlyPenalise) *onlyPenalise = endNode;

						ScheduleNode* startNode = 0;
						{
							map<int, map<int, ScheduleNode*> >::iterator osItr = openStarts.find(npItr->action->getID());
							if (osItr != openStarts.end()) {
								//cout << cl << " - looking for start of " << npItr->action->getID() << " at step " << npItr->pairWithStep;
								startNode = osItr->second[npItr->pairWithStep];
								//cout << " - maps to " << startNode << "\n";
							} else {
								assert(osItr != openStarts.end());
							}
						}
						if (debugMinFlawScheduling) cout << "Corresponding end node: " << endNode << "\n";
						//earliest[endNode] = 0.0;
						++targetNodeCount;
		
						newNodeIndices[endNode] = nodeIndexCounter++;
	
						if (afterNode) {
							if (debugMerging) cout << "End put after afterNode " << afterNode << "\n";
							newEdges.push_back(Edge(afterNode, endNode, EPSILON, DBL_MAX));							

							afterNode = 0;
							//++(fanIn[endNode]);
							//stnConstraints[endNode][afterNode] = -EPSILON;
						}
		
						if (prevTO && prevTO != startNode) {
							if (debugMerging) cout << "End put after prevTO " << prevTO << "\n";
							if (debugMinFlawScheduling) cout << prevTO << " before " << endNode << "\n";
							newEdges.push_back(Edge(prevTO, endNode, EPSILON, DBL_MAX));
						}
	
						if (prevReal && prevReal != prevTO && prevReal != startNode) {
							newEdges.push_back(Edge(prevReal, endNode, EPSILON, DBL_MAX));
						}

						startNode->setPartner(endNode);
						endNode->setPartner(startNode);
		
						//endNode->scheduleAfter(startNode);
						//startNode->scheduleBefore(endNode);
		
						const double actDur = startNode->getDuration();

						newEdges.push_back(Edge(startNode, endNode, actDur, actDur));


						if (debugMinFlawScheduling) cout << startNode << " before " << endNode << "\n";
		
		
						prevTO = endNode;
						prevTOnew = true;

						prevReal = endNode;
						prevRealNew = true;

						{
							list<Literal*> & deletes = endNode->getDeleteEffects();				
							
							list<Literal*>::iterator delItr = deletes.begin();
							const list<Literal*>::iterator delEnd = deletes.end();
		
							for (; delItr != delEnd; ++delItr) {
								Literal* const currLit = *delItr;
								//cout << "New node deletes " << *currLit << "\n";
								map<Literal*, ScheduleNode*, LiteralLT>::iterator gwFind = lastForSP.find(currLit);
								if (gwFind != lastForSP.end()) {
									lastDeletors[currLit] = endNode;
									//cout << "Last deletor for " << *currLit << " is new node " << *(endNode->getOp()) << "\n";
								}
							}
						}
					}
				} else if (npItr->wait) {
				
					if (cl < considerUpTo) --onlyMFS;
					map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(npItr->wait);
					assert(findNewSN != oldToNew.end());
					ScheduleNode* const deref = findNewSN->second;

					if (deref->tilNode()) {
						const int thisSee = deref->tilID() + 1;
						if (thisSee > tilInPlan) tilInPlan = thisSee;
						//cout << cl << " - TIL\n";
					}				

					if (onlyPenalise) *onlyPenalise = deref;
					if (planNodesToPokeAround.empty()) {
						noEarlierThan = deref;
	
						if (debugMinFlawScheduling) {
							cout << "Earliest possible existing schedule point for m.f.s. set to " << noEarlierThan;
							cout << ",\nwhich has index " << oldTOIndices[noEarlierThan] << "\n";
	
						}
					}
					
					if (npItr->getEffects) {
						if (!deref->tilNode()) {
							deref->addSubProblem(sp);
							deref->getPartner()->addSubProblem(sp);
						}
						
						if (prevReal && prevReal != prevTO) {
							if (prevRealNew) {
								newEdges.push_back(Edge(prevReal, deref, EPSILON, DBL_MAX));
							} else {
								bool explicitLink = false;
								{
									list<pair<ScheduleNode*, pair<double, double> > > & succs = prevReal->getSuccessors();
									list<pair<ScheduleNode*, pair<double, double> > >::iterator oItr = succs.begin();
									const list<pair<ScheduleNode*, pair<double, double> > >::iterator oEnd = succs.end();
	
									for (; oItr != oEnd; ++oItr) {
	
										if (oItr->first == deref) {
											explicitLink = true;
											break;
										}
	
									}
	
								}
								{
									list<pair<ScheduleNode*, pair<double, double> > > & succs = deref->getSuccessors();
									list<pair<ScheduleNode*, pair<double, double> > >::iterator oItr = succs.begin();
									const list<pair<ScheduleNode*, pair<double, double> > >::iterator oEnd = succs.end();
	
									for (; oItr != oEnd; ++oItr) {
	
										if (oItr->first == prevReal) {
											if (debugMerging) cout << "Conflict: already have A-before-B, now trying to say B-before-A\n";
											list<ScheduleNode*>::iterator fiItr = garbage.begin();
											const list<ScheduleNode*>::iterator fiEnd = garbage.end();
											for (; fiItr != fiEnd; ++fiItr) {
												delete *fiItr;
											}
											delete toReturn;
											toReturn = 0;
											return toReturn;
										}
	
									}
								}
								
								if (!explicitLink) {
									newEdges.push_back(Edge(prevReal, deref, EPSILON, DBL_MAX));
								}
							}
							
						}
						prevReal = deref;
						prevRealNew = false;
					}
	
					if (prevTO) {
						
						if (prevTOnew) {
							newEdges.push_back(Edge(prevTO, deref, EPSILON, DBL_MAX));
						} else {
							bool explicitLink = false;
							{
								list<pair<ScheduleNode*, pair<double, double> > > & succs = prevTO->getSuccessors();
								list<pair<ScheduleNode*, pair<double, double> > >::iterator oItr = succs.begin();
								const list<pair<ScheduleNode*, pair<double, double> > >::iterator oEnd = succs.end();

								for (; oItr != oEnd; ++oItr) {

									if (oItr->first == deref) {
										explicitLink = true;
										break;
									}

								}

							}
							{
								list<pair<ScheduleNode*, pair<double, double> > > & succs = deref->getSuccessors();
								list<pair<ScheduleNode*, pair<double, double> > >::iterator oItr = succs.begin();
								const list<pair<ScheduleNode*, pair<double, double> > >::iterator oEnd = succs.end();

								for (; oItr != oEnd; ++oItr) {

									if (oItr->first == prevTO) { // was deref, but I think that's wrong
										if (debugMerging) cout << "Conflict: already have A-before-B, now trying to say B-before-A\n";
										list<ScheduleNode*>::iterator fiItr = garbage.begin();
										const list<ScheduleNode*>::iterator fiEnd = garbage.end();
										for (; fiItr != fiEnd; ++fiItr) {
											delete *fiItr;
										}
										delete toReturn;
										toReturn = 0;
										return toReturn;
									}

								}
							}
							
							if (!explicitLink) {
								newEdges.push_back(Edge(prevTO, deref, EPSILON, DBL_MAX));
							}

							
		
							if (debugMerging) cout << "Wait: old " << *(prevTO->getOp()) << (prevTO->isStart() ? " (start)" : " (end)") << " before new " << *(deref->getOp()) << (deref->isStart() ? " (start)" : " (end)")  << "\n";	
						} 
					} else {
						if (debugMerging) cout << "Wait: old " << *(deref->getOp()) << (deref->isStart() ? " (start)" : " (end)")  << "\n";
					}



					prevTO = deref;
					prevTOnew = false;
//					graphRootNode = 0;
	
				}
		
			}
		}
		
		toReturn->m_after = prevTO;
		toReturn->m_afterNew = prevTOnew;
		toReturn->m_afterReal = prevReal;
		toReturn->m_afterRealNew = prevRealNew;
		toReturn->m_cl = cl;
		toReturn->m_tilInPlan = tilInPlan;
		
		{
			const bool rpgPrint = false;

			if (rpgPrint || debugMerging) cout << "Merging in from RPG - \n";

			list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >::iterator rpItr = relaxedPlan.begin();
			const list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >::iterator rpEnd = relaxedPlan.end();

			list<ScheduleNode*> aList;
			//list<ScheduleNode*> bList;
			list<ScheduleNode*> * prevLayer = &aList;
			//list<ScheduleNode*> * thisLayer = &bList;
			if (prevTO) {
				prevLayer->push_back(prevTO);
			} else {
				prevLayer->push_back(0);
			}
			//if (graphRootNode) prevLayer->push_back(graphRootNode);
			//double prevTS = -EPSILON;

			double specialOffset = 0.0;

			for (; rpItr != rpEnd; ++rpItr) {

				double thisTS = rpItr->first + specialOffset;

				if (rpgPrint || debugMerging) cout << "\tlayer " << thisTS << "\n";
				
				list<pair<instantiatedOp*, VAL::time_spec> >::iterator layerItr = rpItr->second.begin();
				const list<pair<instantiatedOp*, VAL::time_spec> >::iterator layerEnd = rpItr->second.end();

				list<ScheduleNode*>::iterator plStart = prevLayer->begin();
				const list<ScheduleNode*>::iterator plEnd = prevLayer->end();
//				const int fanInTmp = prevLayer->size();

				for (; layerItr != layerEnd; ++layerItr) {
					
					if (layerItr->second == VAL::E_AT_START) {

						if (!GlobalSchedule::checkOpenStartsOnly) {
							ScheduleNode* const newNode = new ScheduleNode(layerItr->first, sp, true);
				
							if (rpgPrint || debugMerging) cout << "\t\tstart of " << *(newNode->getOp()) << "\n";
	
							garbage.push_back(newNode);
							newNode->distFromZero() = DBL_MAX;
							newNode->distToZero() = -EPSILON;
	
							{
								newNode->scheduleAfter(0, EPSILON, DBL_MAX);
								toReturn->addEdgeFromZero(newNode, EPSILON, DBL_MAX);
							}
							++targetNodeCount;
	
							for (list<ScheduleNode*>::iterator plItr = plStart; plItr != plEnd; ++plItr) {
						
								newEdges.push_back(Edge(*plItr, newNode, thisTS+EPSILON, DBL_MAX));
								if (debugMerging) cout << "\t\tputting between " << thisTS+EPSILON << " and infinity after " << (*plItr ? " previous node" : " time zero") << "\n";
								
							}						
	
	/*						if (prevLayer->empty()) {
								//cout << "Special case, dealing with it\n";
							
								prevLayer->push_back(newNode);
								specialOffset += EPSILON;
								thisTS = rpItr->first + specialOffset;
								plStart = prevLayer->begin();
							}
	*/
							//fanIn[newNode] = fanInTmp;
							//if (!fanInTmp) openList.push_back(pair<ScheduleNode*, double>(newNode,0.0));
		
							newNodeIndices[newNode] = nodeIndexCounter++;
	
						}

					} else {
					
						map<int, list<list<StartEvent>::iterator > >::iterator ceCheck = compulsaryEnds.find(layerItr->first->getID());

						if (ceCheck == compulsaryEnds.end()) {
							if (!GlobalSchedule::checkOpenStartsOnly) {
								ScheduleNode* const newNode = new ScheduleNode(layerItr->first, sp, false);
								if (rpgPrint || debugMerging) cout << "\t\tend of " << *(newNode->getOp()) << "\n";
								garbage.push_back(newNode);
								newNode->distFromZero() = DBL_MAX;
								newNode->distToZero() = -EPSILON;
		
								{
									newNode->scheduleAfter(0, EPSILON, DBL_MAX);
									toReturn->addEdgeFromZero(newNode, EPSILON, DBL_MAX);
								}
								++targetNodeCount;
								for (list<ScheduleNode*>::iterator plItr = plStart; plItr != plEnd; ++plItr) {
							
									newEdges.push_back(Edge(*plItr, newNode, thisTS+EPSILON, DBL_MAX));
									if (debugMerging) cout << "\t\tputting between " << thisTS+EPSILON << " and infinity after " << (*plItr ? " previous node" : " time zero") << "\n";
									
								}
							}
						} else {

							StartEvent & pairWith = *(ceCheck->second.front());
							ceCheck->second.pop_front();

							if (ceCheck->second.empty()) compulsaryEnds.erase(ceCheck);

							if (!pairWith.compulsaryEnd) {
	
								//int thisFanIn = fanInTmp;
	
								ScheduleNode* const newNode = new ScheduleNode(layerItr->first, sp, false);
								if (rpgPrint || debugMerging) cout << "\t\tend of " << *(newNode->getOp()) << "\n";
								garbage.push_back(newNode);
								newNode->distFromZero() = DBL_MAX;
								newNode->distToZero() = -EPSILON;
		
								{
									newNode->scheduleAfter(0, EPSILON, DBL_MAX);
									toReturn->addEdgeFromZero(newNode, EPSILON, DBL_MAX);
								}
								++targetNodeCount;
								for (list<ScheduleNode*>::iterator plItr = plStart; plItr != plEnd; ++plItr) {
							
									newEdges.push_back(Edge(*plItr, newNode, thisTS+EPSILON, DBL_MAX));
									if (debugMerging) cout << "\t\tputting between " << thisTS+EPSILON << " and infinity after " << (*plItr ? " previous node" : " time zero") << "\n";
									
								}

								map<int, map<int, ScheduleNode*> >::iterator cosItr = openStarts.find(layerItr->first->getID());
								
								if (cosItr != openStarts.end()) {
									//++thisFanIn;
									ScheduleNode* const cc = cosItr->second[pairWith.stepID];
									//cout << "Pairing end of " << layerItr->first->getID() << " with start at step " << pairWith.stepID << "\n";
									newEdges.push_back(Edge(cc, newNode, EPSILON, RPGBuilder::getOpDuration(layerItr->first->getID())));
									if (debugMerging) cout << "\t\tputting at duration after its start\n";
									
									newNode->setPartner(cc);
									cc->setPartner(newNode);
								}
	
								//fanIn[newNode] = thisFanIn;
								//if (!thisFanIn) openList.push_back(pair<ScheduleNode*, double>(newNode,0.0));
	
								newNodeIndices[newNode] = nodeIndexCounter++;
	
							} else {
	
								ScheduleNode* const newNode = oldToNew[pairWith.compulsaryEnd];
								ScheduleNode* const partner = newNode->getPartner();
	
								if (rpgPrint || debugMerging) cout << "\t\tend of " << *(newNode->getOp()) << ", compulsary pairing\n";
								
								for (list<ScheduleNode*>::iterator plItr = plStart; plItr != plEnd; ++plItr) {
									if (*plItr != partner) {
										newEdges.push_back(Edge(*plItr, newNode, thisTS+EPSILON, DBL_MAX));
										if (debugMerging) cout << "\t\tputting between " << thisTS+EPSILON << " and infinity after " << (*plItr ? " previous node" : " time zero") << "\n";
									} else {
										if (debugMerging) cout << "\t\tdon't need to extra partner temporal constraint, one already exists\n";
									}
								}
								
		
								
	
							}
						}
					
					}
					
				}


			}

		}

		if (lastThatDeletesCurrGoal && prevTO) {
			lastForSP[newTopLevelGoal] = prevTO;
			lastDeletors.insert(pair<Literal*, ScheduleNode*>(newTopLevelGoal, lastThatDeletesCurrGoal));
			if (debugMerging) {
				cout << "Recorded that old node " << *(lastThatDeletesCurrGoal->getOp()) << " has to come before " << *(prevTO->getOp()) << "\n";
			}
		}
		if (prevTO) {
			for (int i = tilInPlan; i < tilSeen; ++i) {
				newEdges.push_back(Edge(prevTO, toReturn->tilEvents[i], EPSILON, DBL_MAX));
			}
		}


	}	


	if (!nodeIndexCounter) { // global schedule is empty
		return toReturn;
	}



#ifdef FLOYD_REGRESSION

// Regression-test Cesta & Oddi incremental Bellman-Ford against good old-fashioned Floyd Walshall
// Obviously, only do this if debugging....

	map<ScheduleNode*, double> fwNodeTimestamps; // to check later against BF results
	bool graphIsCyclical = false; // gets set to true if f.w. finds negative cycles
	int firstBadEdge = -1;
	bool filledTS = false;

	const bool debugFloyd = false; // the irony of bugs in bug-checking code
	{

		
		const int matrixSize = garbage.size() + 1;

		vector<vector<double> > floydMatrix(matrixSize);
		for (int i = 0; i < matrixSize; ++i) {
			floydMatrix[i] = vector<double>(matrixSize);
			for (int j = 0; j < matrixSize; ++j) {
				floydMatrix[i][j] = DBL_MAX;
			}
			floydMatrix[i][i] = 0.0;
		}

		{
			list<ScheduleNode*>::iterator fiItr = garbage.begin();
			const list<ScheduleNode*>::iterator fiEnd = garbage.end();
			for (; fiItr != fiEnd; ++fiItr) {
	
				ScheduleNode* const thisNode = *fiItr;
	
				int xIndex;
		
				{
					map<ScheduleNode*, int, ScheduleNodeLT>::iterator oLookup = oldTOIndices.find(thisNode);
					if (oLookup != oldTOIndices.end()) {
						xIndex = oLookup->second;
					} else {
						xIndex = newNodeIndices[thisNode];
					}
					++xIndex;
				}
	
				{
	
					list<pair<ScheduleNode*, pair<double, double> > > & pres = thisNode->getPredecessors();
				
					list<pair<ScheduleNode*, pair<double, double> > >::iterator edgeItr = pres.begin();
					const list<pair<ScheduleNode*, pair<double, double> > >::iterator edgeEnd = pres.end();
		
					for (; edgeItr != edgeEnd; ++edgeItr) {
	
						ScheduleNode* const otherNode = edgeItr->first;
						const double min = edgeItr->second.first;
						const double max = edgeItr->second.second;
	
						int yIndex;
	
						if (otherNode == 0) { // special case for edges to zero
							yIndex = 0;
						} else {
							map<ScheduleNode*, int, ScheduleNodeLT>::iterator oLookup = oldTOIndices.find(otherNode);
							if (oLookup != oldTOIndices.end()) {
								yIndex = oLookup->second;
							} else {
								yIndex = newNodeIndices[otherNode];
							}
							++yIndex;
						}
	
						
						floydMatrix[xIndex][yIndex] = -min;
						floydMatrix[yIndex][xIndex] = max;

						if (debugFloyd && !yIndex) {
							cout << "Floyd previous edge from " << xIndex << " to zero of cost " << -min << "\n";
						}
			
						if (debugFloyd && !xIndex) {
							cout << "Floyd previous edge from " << yIndex << " to zero of cost " << max << "\n";
						}

					}
	
	
				}
	
	
	
			}
		}

		if(debugFloyd) {
			cout << "STN before f.w. for old edges\n";
			for (int i = 0; i < matrixSize; ++i) {
				for (int j = 0; j < matrixSize; ++j) {
					const double v = floydMatrix[i][j];
					if (v == DBL_MAX) {
						cout << "\tinf";
					} else {
						cout << "\t" << v;
					}
				}
				cout << "\n";
			}
		}

		if (false) {

			list<Edge>::iterator elItr = newEdges.begin();
			const list<Edge>::iterator elEnd = newEdges.end();
	
			for (int ei = 0; elItr != elEnd; ++elItr, ++ei) {
	
				int xIndex;
		
				if (elItr->from) {
					map<ScheduleNode*, int, ScheduleNodeLT>::iterator oLookup = oldTOIndices.find(elItr->from);
					if (oLookup != oldTOIndices.end()) {
						xIndex = oLookup->second;
					} else {
						xIndex = newNodeIndices[elItr->from];
					}
					++xIndex;
				} else {
					xIndex = 0;
				}
	
				int yIndex;
		
				if (elItr->to) {
					map<ScheduleNode*, int, ScheduleNodeLT>::iterator oLookup = oldTOIndices.find(elItr->to);
					if (oLookup != oldTOIndices.end()) {
						yIndex = oLookup->second;
					} else {
						yIndex = newNodeIndices[elItr->to];
					}
					++yIndex;
				} else {
					yIndex = 0;
				}
	
				if (floydMatrix[xIndex][yIndex] != DBL_MAX && floydMatrix[xIndex][yIndex] < 0.00) {
					graphIsCyclical = true;
					if (debugFloyd) {
						cout << "Graph is trivially cyclical:\n\told edge says " << xIndex << " -> " << yIndex << " has weight " << floydMatrix[xIndex][yIndex] << "\n";
						cout << "New edge says edge has weight " << elItr->max << "\n";
						cout << "Edge goes from ";
						if (xIndex) {
							cout << *(elItr->from->getOp()) << (elItr->from->isStart() ? " start" : " end") << " to ";
						} else {
							cout << "time zero to ";
						}
						if (yIndex) {
							cout << *(elItr->to->getOp()) << (elItr->to->isStart() ? " start" : " end") << "\n";
						} else {
							cout << "time zero\n";
						}
					}
					if (firstBadEdge == -1 || ei < firstBadEdge) firstBadEdge = ei;
					if (debugFloyd) cout << "First bad edge has index " << ei << "\n";
					break;
				}
				if (floydMatrix[yIndex][xIndex] != DBL_MAX && floydMatrix[yIndex][xIndex] > 0.00) {
					graphIsCyclical = true;
					if (debugFloyd) {
						cout << "Graph is trivially cyclical:\n\told edge says " << xIndex << " -> " << yIndex << " has weight " << floydMatrix[yIndex][xIndex] << "\n";
						cout << "New edge says edge has weight " << 0.0 - elItr->max << "\n";
						cout << "Edge goes from ";
						if (xIndex) {
							cout << *(elItr->from->getOp()) << (elItr->from->isStart() ? " start" : " end") << " to ";
						} else {
							cout << "time zero to ";
						}
						if (yIndex) {
							cout << *(elItr->to->getOp()) << (elItr->to->isStart() ? " start" : " end") << "\n";
						} else {
							cout << "time zero\n";
						}
					}
					if (firstBadEdge == -1 || ei < firstBadEdge) firstBadEdge = ei;
					if (debugFloyd) cout << "First bad edge has index " << ei << "\n";
					break;
				}
	
			}

		}

		vector<vector<double> > testMatrix(floydMatrix);
				
		{
			for (int k = 0; k < matrixSize; ++k) {
			
				vector<double> & fromKTo = testMatrix[k];
		
				for (int i = 0; i < matrixSize; ++i) {
		
					vector<double> & fromITo = testMatrix[i];
					const double distIK = fromITo[k];
					if (distIK != DBL_MAX) {
						for (int j = 0; j < matrixSize; ++j) {
		
							double newDist = fromKTo[j];
							if (newDist != DBL_MAX) {
								newDist += distIK;
								double & distIJ = fromITo[j];
								if (distIJ > newDist) {
									distIJ = newDist;
								}
							}
		
						}
					}
				}
			}
		}

		if(debugFloyd) {

			cout << "STN after f.w. for old edges\n";
			for (int i = 0; i < matrixSize; ++i) {
				for (int j = 0; j < matrixSize; ++j) {
					const double v = testMatrix[i][j];
					if (v == DBL_MAX) {
						cout << "\tinf";
					} else {
						cout << "\t" << v;
					}
				}
				cout << "\n";
			}
	
		}

		list<Edge>::iterator elItr = newEdges.begin();
		const list<Edge>::iterator elEnd = newEdges.end();

		for (int ei = 0; elItr != elEnd; ++elItr, ++ei) {

			testMatrix = floydMatrix;

			int xIndex;
	
			if (elItr->from) {
				map<ScheduleNode*, int, ScheduleNodeLT>::iterator oLookup = oldTOIndices.find(elItr->from);
				if (oLookup != oldTOIndices.end()) {
					xIndex = oLookup->second;
				} else {
					xIndex = newNodeIndices[elItr->from];
				}
				++xIndex;
			} else {
				xIndex = 0;
			}

			int yIndex;
	
			if (elItr->to) {
				map<ScheduleNode*, int, ScheduleNodeLT>::iterator oLookup = oldTOIndices.find(elItr->to);
				if (oLookup != oldTOIndices.end()) {
					yIndex = oLookup->second;
				} else {
					yIndex = newNodeIndices[elItr->to];
				}
				++yIndex;
			} else {
				yIndex = 0;
			}

			if (floydMatrix[xIndex][yIndex] != DBL_MAX && floydMatrix[xIndex][yIndex] < 0.00) {
				graphIsCyclical = true;
				if (debugFloyd) {
					cout << "Graph is trivially cyclical:\n\told edge says " << xIndex << " -> " << yIndex << " has weight " << floydMatrix[xIndex][yIndex] << "\n";
					cout << "New edge says edge has weight " << elItr->max << "\n";
				}
				if (firstBadEdge == -1 || ei < firstBadEdge) firstBadEdge = ei;
				if (debugFloyd) cout << "First bad edge has index " << ei << "\n";
				break;
			}
			if (floydMatrix[yIndex][xIndex] != DBL_MAX && floydMatrix[yIndex][xIndex] > 0.00) {
				graphIsCyclical = true;
				if (debugFloyd) {
					cout << "Graph is trivially cyclical:\n\told edge says " << xIndex << " -> " << yIndex << " has weight " << floydMatrix[yIndex][xIndex] << "\n";
					cout << "New edge says edge has weight " << 0.0 - elItr->max << "\n";
				}
				if (firstBadEdge == -1 || ei < firstBadEdge) firstBadEdge = ei;
				if (debugFloyd) cout << "First bad edge has index " << ei << "\n";
				break;
			}
	
			floydMatrix[xIndex][yIndex] = (elItr->max);
			floydMatrix[yIndex][xIndex] = -(elItr->min);

			testMatrix[xIndex][yIndex] = (elItr->max);
			testMatrix[yIndex][xIndex] = -(elItr->min);

			if (debugFloyd) {

				if (!xIndex) {
					cout << "Floyd new edge from something to zero of cost " << -(elItr->min) << "\n";
				} else if (!yIndex) {
					cout << "Floyd new edge from something to zero of cost " << elItr->max << "\n";
				} else {
					cout << "Floyd new edge from " << *(elItr->from->getOp()) << (elItr->from->isStart() ? " start" : " end");
					cout << " to " << *(elItr->to->getOp()) << (elItr->to->isStart() ? " start" : " end");
					cout << " [" << elItr->min << ", " << elItr->max << "]\n";
					cout << "Index " << xIndex << " to " << yIndex << "\n";
				}
			}

			if (!graphIsCyclical) {
				for (int k = 0; k < matrixSize; ++k) {
			
					vector<double> & fromKTo = testMatrix[k];
			
					for (int i = 0; i < matrixSize; ++i) {
			
						vector<double> & fromITo = testMatrix[i];
						const double distIK = fromITo[k];
						if (distIK != DBL_MAX) {
							for (int j = 0; j < matrixSize; ++j) {
			
								double newDist = fromKTo[j];
								if (newDist != DBL_MAX) {
									newDist += distIK;
									double & distIJ = fromITo[j];
									if (distIJ > newDist) {
										distIJ = newDist;
									}
								}
			
							}
						}
					}
				}
				{
					for (int c = 0; c < matrixSize; ++c) {
						if (testMatrix[c][c] < 0.0) {
							graphIsCyclical = true;
							if (firstBadEdge == -1 || ei < firstBadEdge) firstBadEdge = ei;
							if (debugFloyd) cout << "First bad edge has index " << ei << "\n";
						}
					}
					
				}		

				if(debugFloyd) {
		
					cout << "STN after f.w. for edge\n";
					for (int i = 0; i < matrixSize; ++i) {
						for (int j = 0; j < matrixSize; ++j) {
							const double v = testMatrix[i][j];
							if (v == DBL_MAX) {
								cout << "\tinf";
							} else {
								cout << "\t" << v;
							}
						}
						cout << "\n";
					}
			
				}

				if (graphIsCyclical) break;
			}
			
		}

	
		if (!graphIsCyclical) {
			for (int k = 0; k < matrixSize; ++k) {
	
				vector<double> & fromKTo = floydMatrix[k];
	
				for (int i = 0; i < matrixSize; ++i) {
	
					vector<double> & fromITo = floydMatrix[i];
					const double distIK = fromITo[k];
					if (distIK != DBL_MAX) {
						for (int j = 0; j < matrixSize; ++j) {
	
							double newDist = fromKTo[j];
							if (newDist != DBL_MAX) {
								newDist += distIK;
								double & distIJ = fromITo[j];
								if (distIJ > newDist) {
									distIJ = newDist;
								}
							}
	
						}
					}
				}
			}
	
			if(debugFloyd) {
		
				cout << "STN after f.w. for all edges\n";
				for (int i = 0; i < matrixSize; ++i) {
					for (int j = 0; j < matrixSize; ++j) {
						const double v = floydMatrix[i][j];
						if (v == DBL_MAX) {
							cout << "\tinf";
						} else {
							cout << "\t" << v;
						}
					}
					cout << "\n";
				}
		
			}
	
			
	
			{
				for (int c = 0; c < matrixSize; ++c) {
					assert(floydMatrix[c][c] >= 0.0);
				}
			}

			
			list<ScheduleNode*>::iterator fiItr = garbage.begin();
			const list<ScheduleNode*>::iterator fiEnd = garbage.end();
			for (; fiItr != fiEnd; ++fiItr) {
		
				ScheduleNode* const thisNode = *fiItr;
		
				int xIndex;
		
				{
					map<ScheduleNode*, int, ScheduleNodeLT>::iterator oLookup = oldTOIndices.find(thisNode);
					if (oLookup != oldTOIndices.end()) {
						xIndex = oLookup->second;
					} else {
						xIndex = newNodeIndices[thisNode];
					}
					++xIndex;
				}
				const double fmEntry = floydMatrix[xIndex][0];

				if (fmEntry != 0.0) {
					fwNodeTimestamps[thisNode] = -fmEntry;
				} else {
					fwNodeTimestamps[thisNode] = fmEntry;
				}
			}
			
			filledTS = true;
			
		}
		
	}
	

#endif

	{ // this is where the Bellman-Ford bit goes
		if (debugMerging) {

			cout << "Points of each node before:\n";
			for (int pass = 0; pass < 2; ++pass) {
	
				map<ScheduleNode*, int, ScheduleNodeLT>::iterator nLookup  = (pass ? newNodeIndices.begin() : oldTOIndices.begin());
				const map<ScheduleNode*, int, ScheduleNodeLT>::iterator nLookupEnd = (pass ? newNodeIndices.end() : oldTOIndices.end());
	
				for (; nLookup != nLookupEnd; ++nLookup) {
	
					ScheduleNode* const currSN = nLookup->first;
					cout << "\t" << (currSN->isStart() ? "start" : "end") << " of " << *(currSN->getOp()) << ": ";
					cout << "[" << -(currSN->distToZero()) << ",";
					const double dt = currSN->distFromZero();
					if (dt == DBL_MAX) {
						cout << "inf]\n";
					} else {
						cout << dt << "]\n";
					}
					
				}
	
			}

		}

		list<Edge>::iterator elItr = newEdges.begin();
		const list<Edge>::iterator elEnd = newEdges.end();

		for (int ei = 0; elItr != elEnd; ++elItr, ++ei) {
			if (debugMerging) {
				cout << "Considering edge of from ";
				if (elItr->from) {
					cout << (elItr->from->isStart() ? "start" : "end");
					cout << " of " << *(elItr->from->getOp());
				} else {
					cout << "time zero";
				}
				cout << " to ";
				if (elItr->to) {
					cout << (elItr->to->isStart() ? "start" : "end");
					cout << " of " << *(elItr->to->getOp());
				} else {
					cout << "time zero";
				}
				cout << ", length [" << elItr->min << ",";
				if (elItr->max == DBL_MAX) {
					cout << "inf]\n";
				} else {
					cout << elItr->max << "]\n";
				}
			}
			if (!Propagation(*elItr, toReturn->edgesFromZero)) {
				//cout << "Edge of index " << ei << " wasn't accepted\n";
				//cout << *(elItr->from->getOp()) << " " << (elItr->from->isStart() ? "start" : "end") << " -> " << *(elItr->to->getOp()) << " " << (elItr->to->isStart() ? "start" : "end") << " [" << elItr->min << ", " << elItr->max << "]\n";
#ifdef FLOYD_REGRESSION
				assert(graphIsCyclical);
				assert(ei == firstBadEdge);
				cout << "Floyd-Walshall agrees\n";
				
#endif
				list<ScheduleNode*>::iterator fiItr = garbage.begin();
				const list<ScheduleNode*>::iterator fiEnd = garbage.end();
				for (; fiItr != fiEnd; ++fiItr) {
					delete *fiItr;
				}
				delete toReturn;
				toReturn = 0;
				return toReturn;
			}
		}

		if (debugMerging) {

			cout << "Points of each node after:\n";
			for (int pass = 0; pass < 2; ++pass) {
	
				map<ScheduleNode*, int, ScheduleNodeLT>::iterator nLookup  = (pass ? newNodeIndices.begin() : oldTOIndices.begin());
				const map<ScheduleNode*, int, ScheduleNodeLT>::iterator nLookupEnd = (pass ? newNodeIndices.end() : oldTOIndices.end());
	
				for (; nLookup != nLookupEnd; ++nLookup) {
	
					ScheduleNode* const currSN = nLookup->first;
					cout << "\t" << (currSN->isStart() ? "start" : "end") << " of " << *(currSN->getOp()) << ": ";
					cout << "[" << -(currSN->distToZero()) << ",";
					const double dt = currSN->distFromZero();
					if (dt == DBL_MAX) {
						cout << "inf]\n";
					} else {
						cout << dt << "]\n";
					}
					
				}
	
			}

		}
	}	


#ifdef FLOYD_REGRESSION
	if (graphIsCyclical) {
		cout << "Floyd-Walshall thinks this graph is cyclical - bellman-ford does not!\n";
		assert(false);
	}
	
#endif



	/*if (!GlobalSchedule::alternativeGP) {
		map<Literal*, ScheduleNode*, LiteralLT>::iterator ldItr = lastDeletors.begin();
		const map<Literal*, ScheduleNode*, LiteralLT>::iterator ldEnd = lastDeletors.end();

		for (; ldItr != ldEnd; ++ldItr) {
			ScheduleNode* const firstNode = ldItr->second;
			ScheduleNode* const secondNode = lastForSP[ldItr->first];

			if (debugMerging) {
				cout << "Node " << *(firstNode->getOp()) << " has to come before old node " << *(secondNode->getOp()) << "\n";
			}
			if (debugMinFlawScheduling) {
				cout << "Deletors dictate " << firstNode << " before " << secondNode << "\n";
			}
			secondNode->scheduleAfter(firstNode);
			firstNode->scheduleBefore(secondNode);
			if (1 == ++(fanIn[secondNode])) {
				if (debugMerging) {
					cout << "Removing node for " << *(secondNode->getOp()) << " from open list\n";
					cout << "Open list contained:\n";
					list<pair<ScheduleNode*, double> >::iterator olItr = openList.begin();
					const list<pair<ScheduleNode*, double> >::iterator olEnd = openList.end();

					for (; olItr != olEnd; ++olItr) {
						cout << "\t" << *(olItr->first->getOp()) << "\n";
					}
				}
				openList.remove(pair<ScheduleNode*, double>(secondNode,0.0));
				if (debugMerging) {
					
					cout << "Open list now contains:\n";
					list<pair<ScheduleNode*, double> >::iterator olItr = openList.begin();
					const list<pair<ScheduleNode*, double> >::iterator olEnd = openList.end();

					for (; olItr != olEnd; ++olItr) {
						cout << "\t" << *(olItr->first->getOp()) << "\n";
					}
				}
			}
		}

		

	}

	if (GlobalSchedule::minFlawScheduling) {
		if (debugMinFlawScheduling) {
			cout << "Performing m.f.s\n";
		}
		map<Literal*, set<ScheduleNode*, ScheduleNodeLT>, LiteralLT> activeDeletes;
		//map<Literal*, set<ScheduleNode*, ScheduleNodeLT>, LiteralLT> activeInvariants;

		list<ScheduleNode*>::iterator totalOrderItr = oldTotalOrdering.begin();
		const list<ScheduleNode*>::iterator totalOrderEnd = oldTotalOrdering.end();
		if (noEarlierThan) {
			if (debugMinFlawScheduling) cout << "Have initial no earlier than, executing:\n";
			for (; totalOrderItr != totalOrderEnd && *totalOrderItr != noEarlierThan; ++totalOrderItr) {
				if (debugMinFlawScheduling) cout << *totalOrderItr << "\n";
				{
					list<Literal*> & tList = (*totalOrderItr)->getDeleteEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();

					for (; tlItr != tlEnd; ++tlItr) {
						activeDeletes[*tlItr].insert(*totalOrderItr);
					}
				}
				{
					list<Literal*> & tList = (*totalOrderItr)->getAddEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();

					for (; tlItr != tlEnd; ++tlItr) {
						activeDeletes.erase(*tlItr);
					}
				}
			};
			if (debugMinFlawScheduling) cout << noEarlierThan << "\n";
			{
				list<Literal*> & tList = noEarlierThan->getDeleteEffects();
				list<Literal*>::iterator tlItr = tList.begin();
				const list<Literal*>::iterator tlEnd = tList.end();

				for (; tlItr != tlEnd; ++tlItr) {
					activeDeletes[*tlItr].insert(noEarlierThan);
				}
			}
			{
				list<Literal*> & tList = noEarlierThan->getAddEffects();
				list<Literal*>::iterator tlItr = tList.begin();
				const list<Literal*>::iterator tlEnd = tList.end();

				for (; tlItr != tlEnd; ++tlItr) {
					activeDeletes.erase(*tlItr);
				}
			}
		}
		list<ScheduleNode*>::iterator ntpItr = planNodesToPokeAround.begin();
		const list<ScheduleNode*>::iterator ntpEnd = planNodesToPokeAround.end();

		for (int cl = 0; cl < onlyMFS && ntpItr != ntpEnd; ++ntpItr) {

			if (debugMinFlawScheduling) cout << "Scheduling " << *ntpItr << "\n";

			{
				ScheduleNode* skipTo = noEarlierThan;

				int neIndex = (noEarlierThan ? oldTOIndices[noEarlierThan] : -1);

				{
					list<ScheduleNode*> & pre = (*ntpItr)->getPredecessors();
					
					{	
						list<ScheduleNode*>::iterator pItr = pre.begin();
						const list<ScheduleNode*>::iterator pEnd = pre.end();

						for (; pItr != pEnd; ++pItr) {
							set<int> & tSet = (*pItr)->getSubProblem();
							if (tSet.find(sp) == tSet.end() || tSet.size() > 1) {
								const int ti = oldTOIndices[*pItr];
								if (ti > neIndex) {
									neIndex = ti;
									skipTo = *pItr;
								}
							}
						}
					}
					
				}

				if (skipTo != noEarlierThan) {

					if (debugMinFlawScheduling) cout << "Executing existing schedule:\n";

					noEarlierThan = skipTo;
					for (; totalOrderItr != totalOrderEnd && *totalOrderItr != noEarlierThan; ++totalOrderItr) {
						if (debugMinFlawScheduling) cout << *totalOrderItr << "\n";
						{
							list<Literal*> & tList = (*totalOrderItr)->getDeleteEffects();
							list<Literal*>::iterator tlItr = tList.begin();
							const list<Literal*>::iterator tlEnd = tList.end();
		
							for (; tlItr != tlEnd; ++tlItr) {
								activeDeletes[*tlItr].insert(*totalOrderItr);
							}
						}
						{
							list<Literal*> & tList = (*totalOrderItr)->getAddEffects();
							list<Literal*>::iterator tlItr = tList.begin();
							const list<Literal*>::iterator tlEnd = tList.end();
		
							for (; tlItr != tlEnd; ++tlItr) {
								activeDeletes.erase(*tlItr);
							}
						}
					}
					if (debugMinFlawScheduling) cout << noEarlierThan << "\n";
					{
						list<Literal*> & tList = noEarlierThan->getDeleteEffects();
						list<Literal*>::iterator tlItr = tList.begin();
						const list<Literal*>::iterator tlEnd = tList.end();
		
						for (; tlItr != tlEnd; ++tlItr) {
							activeDeletes[*tlItr].insert(noEarlierThan);
						}
					}
					{
						list<Literal*> & tList = noEarlierThan->getAddEffects();
						list<Literal*>::iterator tlItr = tList.begin();
						const list<Literal*>::iterator tlEnd = tList.end();
		
						for (; tlItr != tlEnd; ++tlItr) {
							activeDeletes.erase(*tlItr);
						}
					}

				}

				

			}

			ScheduleNode * oldNE = noEarlierThan;

			ScheduleNode* cannotComeAfter = 0;
			
			{
				if (debugMinFlawScheduling) cout << "Finding latest point cut-off\n";
				list<ScheduleNode*>::iterator restItr = ntpItr;
				
				for (; restItr != ntpEnd; ++restItr) {
					for (int pass = 0; pass < 2; ++pass) {
						list<ScheduleNode*> & pre = (pass ? (*restItr)->getPartner() : (*restItr))->getSuccessors();
						
						{	
							list<ScheduleNode*>::iterator pItr = pre.begin();
							const list<ScheduleNode*>::iterator pEnd = pre.end();
	
							for (; pItr != pEnd; ++pItr) {
								set<int> & tSet = (*pItr)->getSubProblem();
								if (tSet.find(sp) == tSet.end() || tSet.size() > 1) {
									cannotComeAfter = *pItr;
									if (debugMinFlawScheduling) {
										cout << "On pass " << pass << " - successor node " << *pItr << "\n";
									}
									if (oldTOIndices[cannotComeAfter] <= oldTOIndices[noEarlierThan]) {
										if (debugMinFlawScheduling) cout << "m.f.s spotted a cycle\n";
										
										delete toReturn;
										toReturn = 0;

										map<ScheduleNode*, int, ScheduleNodeLT>::iterator fanItr = fanIn.begin();
										const map<ScheduleNode*, int, ScheduleNodeLT>::iterator fanEnd = fanIn.end();
										for (; fanItr != fanEnd; ++fanItr) {
											delete fanItr->first;
										}

										return toReturn;
									}
									break;
								}
							}
						}
						if (cannotComeAfter) break;
					}
					if (cannotComeAfter) break;
				}
				
			}
			
			
			list<ScheduleNode*>::iterator ccaItr = totalOrderEnd;

			if (cannotComeAfter) {
				list<ScheduleNode*>::iterator wrtTO = totalOrderItr;	
				for (; wrtTO != totalOrderEnd; ++wrtTO) {
					if ((*wrtTO) == cannotComeAfter) {
						ccaItr = wrtTO;
						break;
					}
				}
				if (debugMinFlawScheduling) {
					cout << "Found matching iterator point - " << *ccaItr << "\n";
				}
			} else {
				if (debugMinFlawScheduling) cout << "Okay until the end\n";
			}

			
			

			double bestCost = DBL_MAX;
			ScheduleNode* bestOption = 0;
			list<ScheduleNode*>::iterator bestOptionIterator = totalOrderItr;

			if (!noEarlierThan) {
				
				map<Literal*, set<ScheduleNode*, ScheduleNodeLT>, LiteralLT> localAD(activeDeletes);
				{
					list<Literal*> & tList = (*ntpItr)->getDeleteEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();

					for (; tlItr != tlEnd; ++tlItr) {
						localAD[*tlItr].insert(*totalOrderItr);
					}
				}
				{
					list<Literal*> & tList = (*ntpItr)->getAddEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();

					for (; tlItr != tlEnd; ++tlItr) {
						localAD.erase(*tlItr);
					}
				}
				bestCost = consider(localAD, totalOrderItr, totalOrderEnd, sp);
				if (debugMinFlawScheduling) {
					cout << "Scheduling option - cost " << bestCost << "\n";
					cout << "Now the best cost\n";
				}
				
			}
			
			list<ScheduleNode*>::iterator wrtTO = totalOrderItr;

			map<Literal*, set<ScheduleNode*, ScheduleNodeLT>, LiteralLT> outerAD(activeDeletes);

			for (; bestCost > 0.0 && wrtTO != ccaItr; ++wrtTO) {

				
				if (*wrtTO != noEarlierThan) {
					{
						list<Literal*> & tList = (*wrtTO)->getDeleteEffects();
						list<Literal*>::iterator tlItr = tList.begin();
						const list<Literal*>::iterator tlEnd = tList.end();
	
						for (; tlItr != tlEnd; ++tlItr) {
							outerAD[*tlItr].insert((*wrtTO));
						}
					}
					{
						list<Literal*> & tList = (*wrtTO)->getAddEffects();
						list<Literal*>::iterator tlItr = tList.begin();
						const list<Literal*>::iterator tlEnd = tList.end();
	
						for (; tlItr != tlEnd; ++tlItr) {
							outerAD.erase(*tlItr);
						}
					}
				}

				double newCost = 0.0;

				{
					list<Literal*> & pres = (*ntpItr)->getPreconditions();
					list<Literal*>::iterator pItr = pres.begin();
					const list<Literal*>::iterator pEnd = pres.end();
					
					set<ScheduleNode*, ScheduleNodeLT> alreadyPenalised;
		
					for (; pItr != pEnd; ++pItr) {
						map<Literal*, set<ScheduleNode*, ScheduleNodeLT>, LiteralLT>::iterator fItr = outerAD.find(*pItr);
						if (fItr != outerAD.end()) {
							set<ScheduleNode*, ScheduleNodeLT>::iterator sItr = fItr->second.begin();
							const set<ScheduleNode*, ScheduleNodeLT>::iterator sEnd = fItr->second.end();
							for (; sItr != sEnd; ++sItr) {
								if (alreadyPenalised.find(*sItr) == alreadyPenalised.end()) {
									newCost += GlobalSchedule::currentEvaluator->costFrom(*sItr, *ntpItr, sp);
									alreadyPenalised.insert(*sItr);
								}
							}
						}
					}

				}

				map<Literal*, set<ScheduleNode*, ScheduleNodeLT>, LiteralLT> localAD(outerAD);

				{
					list<Literal*> & tList = (*ntpItr)->getDeleteEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();

					for (; tlItr != tlEnd; ++tlItr) {
						localAD[*tlItr].insert((*ntpItr));
					}
				}
				{
					list<Literal*> & tList = (*ntpItr)->getAddEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();

					for (; tlItr != tlEnd; ++tlItr) {
						localAD.erase(*tlItr);
					}
				}

				list<ScheduleNode*>::iterator restStart = wrtTO;
				++restStart;

				newCost += consider(localAD, restStart, totalOrderEnd, sp);
				if (debugMinFlawScheduling) {
					cout << "Scheduling option - cost " << newCost << "\n";
				}
				if (newCost < bestCost) {
					if (debugMinFlawScheduling) {
						cout << "Now the best cost\n";
					}
					bestCost = newCost;
					bestOption = *wrtTO;
					bestOptionIterator = wrtTO;
					++bestOptionIterator;
				}
				
			}

			if (bestOption) {
				(*ntpItr)->scheduleAfter(bestOption);
				bestOption->scheduleBefore(*ntpItr);
				if (1 == ++(fanIn[*ntpItr])) {
					openList.remove(pair<ScheduleNode*, double>(*ntpItr,0.0));
				}
				
				if (bestOptionIterator != totalOrderEnd) {

					(*ntpItr)->scheduleBefore(*bestOptionIterator);
					(*bestOptionIterator)->scheduleAfter(*ntpItr);
					if (1 == ++(fanIn[*bestOptionIterator])) {
						openList.remove(pair<ScheduleNode*, double>(*bestOptionIterator,0.0));
					}

				}
						
			} else {
				if (totalOrderItr != totalOrderEnd) {
					(*totalOrderItr)->scheduleAfter(*ntpItr);
					(*ntpItr)->scheduleBefore(*totalOrderItr);
					if (1 == ++(fanIn[*totalOrderItr])) {
						openList.remove(pair<ScheduleNode*, double>(*totalOrderItr,0.0));
					}
				}
			}

			noEarlierThan = bestOption;

			

			if (noEarlierThan != oldNE) {
				for (; totalOrderItr != totalOrderEnd && *totalOrderItr != noEarlierThan; ++totalOrderItr) {
					{
						list<Literal*> & tList = (*totalOrderItr)->getDeleteEffects();
						list<Literal*>::iterator tlItr = tList.begin();
						const list<Literal*>::iterator tlEnd = tList.end();
	
						for (; tlItr != tlEnd; ++tlItr) {
							activeDeletes[*tlItr].insert(*totalOrderItr);
						}
					}
					{
						list<Literal*> & tList = (*totalOrderItr)->getAddEffects();
						list<Literal*>::iterator tlItr = tList.begin();
						const list<Literal*>::iterator tlEnd = tList.end();
	
						for (; tlItr != tlEnd; ++tlItr) {
							activeDeletes.erase(*tlItr);
						}
					}
				}
				{
					list<Literal*> & tList = noEarlierThan->getDeleteEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
						activeDeletes[*tlItr].insert(noEarlierThan);
					}
				}
				{
					list<Literal*> & tList = noEarlierThan->getAddEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
						activeDeletes.erase(*tlItr);
					}
				}
			}

			{
				{
					list<Literal*> & tList = (*ntpItr)->getDeleteEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();

					for (; tlItr != tlEnd; ++tlItr) {
						activeDeletes[*tlItr].insert((*ntpItr));
					}
				}
				{
					list<Literal*> & tList = (*ntpItr)->getAddEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();

					for (; tlItr != tlEnd; ++tlItr) {
						activeDeletes.erase(*tlItr);
					}
				}
			}


			{
				ScheduleNode* skipTo = noEarlierThan;

				int neIndex = (noEarlierThan ? oldTOIndices[noEarlierThan] : -1);

				{
					list<ScheduleNode*> & pre = (*ntpItr)->getPartner()->getPredecessors();
					
					{	
						list<ScheduleNode*>::iterator pItr = pre.begin();
						const list<ScheduleNode*>::iterator pEnd = pre.end();

						for (; pItr != pEnd; ++pItr) {
							set<int> & tSet = (*pItr)->getSubProblem();
							if (tSet.find(sp) == tSet.end() || tSet.size() > 1) {
								const int ti = oldTOIndices[*pItr];
								if (ti > neIndex) {
									neIndex = ti;
									skipTo = *pItr;
								}
							}
						}
					}
					
				}

				if (skipTo != noEarlierThan) {


					noEarlierThan = skipTo;
					for (; totalOrderItr != totalOrderEnd && *totalOrderItr != noEarlierThan; ++totalOrderItr) {
						{
							list<Literal*> & tList = (*totalOrderItr)->getDeleteEffects();
							list<Literal*>::iterator tlItr = tList.begin();
							const list<Literal*>::iterator tlEnd = tList.end();
		
							for (; tlItr != tlEnd; ++tlItr) {
								activeDeletes[*tlItr].insert(*totalOrderItr);
							}
						}
						{
							list<Literal*> & tList = (*totalOrderItr)->getAddEffects();
							list<Literal*>::iterator tlItr = tList.begin();
							const list<Literal*>::iterator tlEnd = tList.end();
		
							for (; tlItr != tlEnd; ++tlItr) {
								activeDeletes.erase(*tlItr);
							}
						}
					}
					{
						list<Literal*> & tList = noEarlierThan->getDeleteEffects();
						list<Literal*>::iterator tlItr = tList.begin();
						const list<Literal*>::iterator tlEnd = tList.end();
		
						for (; tlItr != tlEnd; ++tlItr) {
							activeDeletes[*tlItr].insert(noEarlierThan);
						}
					}
					{
						list<Literal*> & tList = noEarlierThan->getAddEffects();
						list<Literal*>::iterator tlItr = tList.begin();
						const list<Literal*>::iterator tlEnd = tList.end();
		
						for (; tlItr != tlEnd; ++tlItr) {
							activeDeletes.erase(*tlItr);
						}
					}

				}

				

			}
		}
	}*/

	if (skipTimeline) {

		list<ScheduleNode*>::iterator nLookup = garbage.begin();
		const list<ScheduleNode*>::iterator nLookupEnd = garbage.end();
	
		for (; nLookup != nLookupEnd; ++nLookup) {
			toReturn->events.push_back(*nLookup);
		}

	} else {

		list<pair<ScheduleNode*, double> > openList;
	
		{ // new STN bit
			list<ScheduleNode*>::iterator nLookup = garbage.begin();
			const list<ScheduleNode*>::iterator nLookupEnd = garbage.end();
	
			for (; nLookup != nLookupEnd; ++nLookup) {
	
				ScheduleNode* const currSN = *nLookup;
				double currTS = currSN->distToZero();
				if (currTS < 0) currTS = 0.0 - currTS;
	
	
				if (openList.empty()) {
					openList.push_back(pair<ScheduleNode*, double>(currSN, currTS));
				} else {
					list<pair<ScheduleNode*, double> >::iterator insItr = openList.begin();
					const list<pair<ScheduleNode*, double> >::iterator insEnd = openList.end();
	
					//for (; insItr != insEnd && (insItr->second <= currTS); ++insItr);
					for (; insItr != insEnd && ((insItr->second - currTS) < (EPSILON / 2)); ++insItr);
	
					openList.insert(insItr, pair<ScheduleNode*, double>(currSN, currTS));
				}
			}
	
	
		}
	
	#ifdef FLOYD_REGRESSION
	
		list<pair<ScheduleNode*, double> > floydOpenList;
	
	
		{
			list<ScheduleNode*>::iterator nLookup = garbage.begin();
			const list<ScheduleNode*>::iterator nLookupEnd = garbage.end();
	
			for (; nLookup != nLookupEnd; ++nLookup) {
	
				ScheduleNode* const currSN = *nLookup;
				double currTS = fwNodeTimestamps[currSN];
	
	
				if (floydOpenList.empty()) {
					floydOpenList.push_back(pair<ScheduleNode*, double>(currSN, currTS));
				} else {
					list<pair<ScheduleNode*, double> >::iterator insItr = floydOpenList.begin();
					const list<pair<ScheduleNode*, double> >::iterator insEnd = floydOpenList.end();
	
					//for (; insItr != insEnd && (insItr->second <= currTS); ++insItr);
	
					for (; insItr != insEnd && ((insItr->second - currTS) < (EPSILON / 2)); ++insItr);
	
					floydOpenList.insert(insItr, pair<ScheduleNode*, double>(currSN, currTS));
				}
			}
		}
	
		bool failedTest = false;
	
		{
	
			list<pair<ScheduleNode*, double> >::iterator oneItr = openList.begin();
			const list<pair<ScheduleNode*, double> >::iterator oneEnd = openList.end();
	
			list<pair<ScheduleNode*, double> >::iterator twoItr = floydOpenList.begin();
	
			for (int i = 0; oneItr != oneEnd; ++oneItr, ++twoItr, ++i) {
	
				if (oneItr->first != twoItr->first) {
	
					cout << "Nodes at " << i << " differ\n";
					failedTest = true;
				}
	
				if (fabs(oneItr->second - twoItr->second) >= 0.0005) {
					cout << "Timestamps at " << i << " differ: " << oneItr->second << " vs " << twoItr->second << "\n";
					failedTest = true;
				}
	
			}
	
		}
		if (failedTest) {
			cout << "Open lists differ - Bellmann Ford gives:\n";
	
			{
				list<pair<ScheduleNode*, double> >::iterator olItr = openList.begin();
				const list<pair<ScheduleNode*, double> >::iterator olEnd = openList.end();
	
				for (int i = 0; olItr != olEnd; ++olItr, ++i) {
					cout << "\t" << i << ":" << olItr->second << " - " << *(olItr->first->getOp()) << ", " << (olItr->first->isStart() ? "start" : "end") << "\n";
				}
			}
	
			cout << "Floyd Walshall gives:\n";
	
			{
				list<pair<ScheduleNode*, double> >::iterator olItr = floydOpenList.begin();
				const list<pair<ScheduleNode*, double> >::iterator olEnd = floydOpenList.end();
	
				for (int i = 0; olItr != olEnd; ++olItr, ++i) {
					cout << "\t" << i << ":" << olItr->second << " - " << *(olItr->first->getOp()) << ", " << (olItr->first->isStart() ? "start" : "end") << "\n";
				}
			}
			
			assert(false);
	
		}
	
	#endif
	
		int nodesVisited = 0;	
		while (!openList.empty()) {
			++nodesVisited;
			ScheduleNode * const currNode = openList.front().first;
			const double currTS = openList.front().second;
			if (debugNewMerge || debugMerging) cout << "Visiting node for " << *(currNode->getOp()) << "\n";
	
			openList.pop_front();
	
			toReturn->events.push_back(currNode);
	
			TimelinePoint * const newTP = new TimelinePoint(*(timeline.back()), currNode->getAddEffects(), currNode->getDeleteEffects());
	
			newTP->after = toReturn->events.end();
			--(newTP->after);
	
			newTP->timestamp = currTS;
	
			toReturn->timeline.push_back(newTP);
	
	
			
		}
	
	//	if (toReturn) toReturn->enforceOrder();

	}
	if (debugMerging && toReturn) toReturn->printEvents();
	return toReturn;

}

double CandidateSchedule::consider(map<Literal*, set<ScheduleNode*, ScheduleNodeLT>, LiteralLT> & activeDeletes, list<ScheduleNode*>::iterator from, list<ScheduleNode*>::iterator to, const int & spID) {

	double accruedCost = 0.0;

	for (; from != to; ++from) {
		{
			list<Literal*> & pres = (*from)->getPreconditions();
			list<Literal*>::iterator pItr = pres.begin();
			const list<Literal*>::iterator pEnd = pres.end();
			
			set<ScheduleNode*, ScheduleNodeLT> alreadyPenalised;

			for (; pItr != pEnd; ++pItr) {
				map<Literal*, set<ScheduleNode*, ScheduleNodeLT>, LiteralLT>::iterator fItr = activeDeletes.find(*pItr);
				if (fItr != activeDeletes.end()) {
					set<ScheduleNode*, ScheduleNodeLT>::iterator sItr = fItr->second.begin();
					const set<ScheduleNode*, ScheduleNodeLT>::iterator sEnd = fItr->second.end();
					for (; sItr != sEnd; ++sItr) {
						if (alreadyPenalised.find(*sItr) == alreadyPenalised.end()) {
							accruedCost += GlobalSchedule::currentEvaluator->costFrom(*sItr, *from, spID);
							alreadyPenalised.insert(*sItr);
						}
					}
				}
			}
		}
		{
			list<Literal*> & effs = (*from)->getAddEffects();
			list<Literal*>::iterator eItr = effs.begin();
			const list<Literal*>::iterator eEnd = effs.end();
		
			for (; eItr != eEnd; ++eItr) {
				activeDeletes.erase(*eItr);
			}
			
		}
		if (activeDeletes.empty()) break;
	}

	return accruedCost;


};


TimelinePoint::TimelinePoint(TimelinePoint & p, list<Literal*> & add, list<Literal*> & del) : state(p.state), fluents(p.fluents), dummy(false), timestamp(p.timestamp) {

	{
		list<Literal*>::iterator effItr = add.begin();
		const list<Literal*>::iterator effEnd = add.end();

		for (; effItr != effEnd; ++effItr) {
			state.erase(*effItr);
		}
	}

	{
		list<Literal*>::iterator effItr = del.begin();
		const list<Literal*>::iterator effEnd = del.end();

		for (; effItr != effEnd; ++effItr) {
			state.insert(*effItr);
		}
	}


}

void GlobalSchedule::initialise() {

	LiteralSet initialState;
	vector<double> initialFluents;

	RPGBuilder::getInitialState(initialState, initialFluents);	

	currentSchedule = new CandidateSchedule(new TimelinePoint(initialState, initialFluents));

	currentSchedule->addTILs(RPGBuilder::getTILs());

};

void CandidateSchedule::printEvents() {

	list<ScheduleNode*>::iterator eItr = events.begin();
	const list<ScheduleNode*>::iterator eEnd = events.end();
	cout << "\n**\n";
	for (int i = 0; eItr != eEnd; ++i, ++eItr) {
		if ((*eItr)->tilNode()) {
			cout << i << " - timed initial literal\n";
		} else {
			cout << i <<" - " << *((*eItr)->getOp()) << " " << ((*eItr)->isStart() ? "start" : "end") << " ( ";
			set<int> sps = (*eItr)->getSubProblem();
			set<int>::iterator spsItr = sps.begin();
			const set<int>::iterator spsEnd = sps.end();
	
			for (; spsItr != spsEnd; ++spsItr) {
				cout << *spsItr << " ";
			}
			cout << ")";
		}
		cout << " [" << -((*eItr)->distToZero()) << ", ";
		if ((*eItr)->distFromZero() == DBL_MAX) {
			cout << "inf]\n";
		} else {
			cout  << (*eItr)->distFromZero() << "]\n";
		}
		assert((*eItr)->tilNode() || (*eItr)->isStart() || (*eItr)->getPredecessors().size());
	}

	cout << "\n**\n";
};

/*
void CandidateSchedule::enforceOrder() {

	list<ScheduleNode*> allEarlierSNs;
	{
		list<ScheduleNode*>::iterator eItr = events.begin();
		const list<ScheduleNode*>::iterator eEnd = events.end();

		for (; eItr != eEnd; ++eItr) {
			(*eItr)->getPredecessors().clear();
			(*eItr)->getSuccessors().clear();
		}

	}

	list<TimelinePoint*>::iterator tlItr = timeline.begin();
	const list<TimelinePoint*>::iterator tlEnd = timeline.end();

	++tlItr; // skip dummy for initial state

	for (; tlItr != tlEnd; ++tlItr) {
	
		ScheduleNode* const nextSN = *((*tlItr)->after);

		list<ScheduleNode*>::iterator eItr = allEarlierSNs.begin();
		const list<ScheduleNode*>::iterator eEnd = allEarlierSNs.end();

		for (; eItr != eEnd; ++eItr) {
			
			ScheduleNode* const prevSN = *eItr;

			prevSN->scheduleBefore(nextSN);
			nextSN->scheduleAfter(prevSN);
		}

		allEarlierSNs.push_back(nextSN);

	}


};
*/


void CandidateSchedule::printAsPlan() {

	list<TimelinePoint*>::iterator tlItr = timeline.begin();
	const list<TimelinePoint*>::iterator tlEnd = timeline.end();

	++tlItr; // skip dummy for initial state

	for (; tlItr != tlEnd; ++tlItr) {
		ScheduleNode* const nextSN = *((*tlItr)->after);
		if (nextSN->isStart()) {
			cout << (*tlItr)->timestamp << ": " << *(nextSN->getOp()) << " [" << nextSN->getDuration() << "]\n";
		}
	}

};

bool CandidateSchedule::postConstraint(ScheduleNode* const a, ScheduleNode* const b, const bool & bAfterA) {


	static const double EPSILON = 0.001;
#define PC_FLOYD_DEBUG
#ifdef PC_FLOYD_DEBUG
	
	const static bool debugFloyd = false;

	bool graphIsCyclical = false;

	map<ScheduleNode*, int, ScheduleNodeLT> nodeIndices;

	const int nodeCount = events.size() + 1;

	vector<vector<double> > floydMatrix(nodeCount);

	{

		for (int i = 0; i < nodeCount; ++i) {
			vector<double> & toFill = floydMatrix[i] = vector<double>(nodeCount);
			for (int j = 0; j < nodeCount; ++j) {
				toFill[j] = DBL_MAX;
			}
			toFill[i] = 0.0;
		}

	}

	
	
	{
		list<ScheduleNode*>::iterator fiItr = events.begin();
		const list<ScheduleNode*>::iterator fiEnd = events.end();
		for (int xIndex = 1; fiItr != fiEnd; ++fiItr, ++xIndex) {

			ScheduleNode* const thisNode = *fiItr;

			nodeIndices[thisNode] = xIndex;

			{

				list<pair<ScheduleNode*, pair<double, double> > > & pres = thisNode->getPredecessors();
			
				list<pair<ScheduleNode*, pair<double, double> > >::iterator edgeItr = pres.begin();
				const list<pair<ScheduleNode*, pair<double, double> > >::iterator edgeEnd = pres.end();
	
				for (; edgeItr != edgeEnd; ++edgeItr) {

					ScheduleNode* const otherNode = edgeItr->first;
					const double min = edgeItr->second.first;
					const double max = edgeItr->second.second;

					int yIndex;

					if (otherNode == 0) { // special case for edges to zero
						yIndex = 0;
					} else {
						map<ScheduleNode*, int, ScheduleNodeLT>::iterator oLookup = nodeIndices.find(otherNode);
						yIndex = oLookup->second;
					}

					
					floydMatrix[xIndex][yIndex] = -min;
					floydMatrix[yIndex][xIndex] = max;

					if (debugFloyd && !yIndex) {
						cout << "Floyd previous edge from " << xIndex << " to zero of cost " << -min << "\n";
					}
		
					if (debugFloyd && !xIndex) {
						cout << "Floyd previous edge from " << yIndex << " to zero of cost " << max << "\n";
					}

				}


			}

		}
	}

	{

		Edge tmpEdge = (bAfterA ? Edge(a, b, EPSILON, DBL_MAX) : Edge(b, a, EPSILON, DBL_MAX));
		Edge* const elItr = &tmpEdge;


		int xIndex;

		if (elItr->from) {
			map<ScheduleNode*, int, ScheduleNodeLT>::iterator oLookup = nodeIndices.find(elItr->from);
			xIndex = oLookup->second;
		} else {
			xIndex = 0;
		}

		int yIndex;

		if (elItr->to) {
			map<ScheduleNode*, int, ScheduleNodeLT>::iterator oLookup = nodeIndices.find(elItr->to);
			yIndex = oLookup->second;
		} else {
			yIndex = 0;
		}

		if (floydMatrix[xIndex][yIndex] != DBL_MAX && floydMatrix[xIndex][yIndex] < 0.00) {
			graphIsCyclical = true;
			if (debugFloyd) {
				cout << "Graph is trivially cyclical:\n\told edge says " << xIndex << " -> " << yIndex << " has weight " << floydMatrix[xIndex][yIndex] << "\n";
				cout << "New edge says edge has weight " << elItr->max << "\n";
			}
			
		}
		if (floydMatrix[yIndex][xIndex] != DBL_MAX && floydMatrix[yIndex][xIndex] > 0.00) {
			graphIsCyclical = true;
			if (debugFloyd) {
				cout << "Graph is trivially cyclical:\n\told edge says " << xIndex << " -> " << yIndex << " has weight " << floydMatrix[yIndex][xIndex] << "\n";
				cout << "New edge says edge has weight " << 0.0 - elItr->max << "\n";
			}
			
		}

		if (!graphIsCyclical) {

			floydMatrix[xIndex][yIndex] = (elItr->max);
			floydMatrix[yIndex][xIndex] = -(elItr->min);
	
	
			if (debugFloyd) {
	
				if (!xIndex) {
					cout << "Floyd new edge from something to zero of cost " << -(elItr->min) << "\n";
				} else if (!yIndex) {
					cout << "Floyd new edge from something to zero of cost " << elItr->max << "\n";
				} else {
					cout << "Floyd new edge from " << *(elItr->from->getOp()) << (elItr->from->isStart() ? " start" : " end");
					cout << " to " << *(elItr->to->getOp()) << (elItr->to->isStart() ? " start" : " end");
					cout << " [" << elItr->min << ", " << elItr->max << "]\n";
					cout << "Index " << xIndex << " to " << yIndex << "\n";
				}
			}

		
			for (int k = 0; k < nodeCount; ++k) {
		
				vector<double> & fromKTo = floydMatrix[k];
		
				for (int i = 0; i < nodeCount; ++i) {
		
					vector<double> & fromITo = floydMatrix[i];
					const double distIK = fromITo[k];
					if (distIK != DBL_MAX) {
						for (int j = 0; j < nodeCount; ++j) {
		
							double newDist = fromKTo[j];
							if (newDist != DBL_MAX) {
								newDist += distIK;
								double & distIJ = fromITo[j];
								if (distIJ > newDist) {
									distIJ = newDist;
								}
							}
		
						}
					}
				}
			}
			{
				for (int c = 0; c < nodeCount; ++c) {
					if (floydMatrix[c][c] < 0.0) {
						graphIsCyclical = true;
						break;
					}
				}
				
			}		

			if(debugFloyd) {
	
				cout << "STN after f.w. for edge\n";
				for (int i = 0; i < nodeCount; ++i) {
					for (int j = 0; j < nodeCount; ++j) {
						const double v = floydMatrix[i][j];
						if (v == DBL_MAX) {
							cout << "\tinf";
						} else {
							cout << "\t" << v;
						}
					}
					cout << "\n";
				}
		
			}
		}
			


	}

	

#endif

	{
		oldTimeline.clear();
		a->preserve(true);
		b->preserve(true);


		list<ScheduleNode*>::iterator evItr = events.begin();
		const list<ScheduleNode*>::iterator evEnd = events.end();

		for (; evItr != evEnd; ++evItr) {
			(*evItr)->preserve(false);
		}

	}

	//cout << "Posting constraint\n";

	if (bAfterA) {
		Edge newEdge(a, b, EPSILON, DBL_MAX);
		if (!Propagation(newEdge, edgesFromZero)) {
#ifdef PC_FLOYD_DEBUG
			assert(graphIsCyclical);
#endif
			return false;
		}
	} else {
		Edge newEdge(b, a, EPSILON, DBL_MAX);
		if (!Propagation(newEdge, edgesFromZero)) {
#ifdef PC_FLOYD_DEBUG
			assert(graphIsCyclical);
#endif
			return false;
		}
	}
#ifdef PC_FLOYD_DEBUG
	assert(!graphIsCyclical);
#endif
	//cout << "Constraint accepted\n";

	oldEvents = events;
	events.clear();

	list<pair<ScheduleNode*, double> > openList;

	{ // new STN bit
		list<ScheduleNode*>::iterator nLookup = oldEvents.begin();
		const list<ScheduleNode*>::iterator nLookupEnd = oldEvents.end();

		for (; nLookup != nLookupEnd; ++nLookup) {

			ScheduleNode* const currSN = *nLookup;
			double currTS = currSN->distToZero();

#ifdef PC_FLOYD_DEBUG
			const double floydTS = floydMatrix[nodeIndices[currSN]][0];
			if (fabs(currTS-floydTS) > 0.0005) {
				cout << "Floyd gives TS " << floydTS << "\n";
				cout << "BF    gives TS " << currTS << "\n";
				assert(fabs(currTS-floydTS) < 0.0005);
			}
#endif
			if (currTS < 0) currTS = 0.0 - currTS;



			if (openList.empty()) {
				openList.push_back(pair<ScheduleNode*, double>(currSN, currTS));
			} else {
				list<pair<ScheduleNode*, double> >::iterator insItr = openList.begin();
				const list<pair<ScheduleNode*, double> >::iterator insEnd = openList.end();

				//for (; insItr != insEnd && (insItr->second <= currTS); ++insItr);
				for (; insItr != insEnd && ((insItr->second - currTS) < (EPSILON / 2)); ++insItr);

				openList.insert(insItr, pair<ScheduleNode*, double>(currSN, currTS));
			}
		}


	}

	{
		oldTimeline = timeline;
		timeline.clear();
		timeline.push_back(new TimelinePoint(*(oldTimeline.front())));
		timeline.back()->after = events.end();
	}


	while (!openList.empty()) {
		
		ScheduleNode * const currNode = openList.front().first;
		const double currTS = openList.front().second;
		
		openList.pop_front();

		events.push_back(currNode);

		TimelinePoint * const newTP = new TimelinePoint(*(timeline.back()), currNode->getAddEffects(), currNode->getDeleteEffects());

		newTP->after = events.end();
		--(newTP->after);

		newTP->timestamp = currTS;

		timeline.push_back(newTP);


		
	}

	return true;

};

list<ScheduleNode*> CandidateSchedule::oldEvents;
list<TimelinePoint*> CandidateSchedule::oldTimeline;

void CandidateSchedule::unpostConstraint() {

	if (!oldTimeline.empty()) {
				
		list<TimelinePoint*>::iterator tlItr = timeline.begin();
		const list<TimelinePoint*>::iterator tlEnd = timeline.end();

		for (; tlItr != tlEnd; ++tlItr) delete *tlItr;
	
		
		timeline = oldTimeline;
		oldTimeline.clear();
	}

	events = oldEvents;
	oldEvents.clear();

	list<ScheduleNode*>::iterator eItr = events.begin();
	const list<ScheduleNode*>::iterator eEnd = events.end();

	for (; eItr != eEnd; ++eItr) {
		(*eItr)->rollback();
	}
	
}


bool CandidateSchedule::postNewActions(list<FFEvent> & newEvents, list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >  & relaxedPlan, map<int, list<list<StartEvent>::iterator > > compulsaryEnds, map<int, map<int, ScheduleNode*> > * rememberOS, map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> & reallyOldToNew, const int & sp,bool debug, map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> * newToOld, ScheduleNode* * onlyPenalise) {

	const bool debugMerging = false;
	const bool debugMinFlawScheduling = false;
	static const double EPSILON = 0.001;
	ScheduleNode* prevTO = m_after;
	bool prevTOnew = m_afterNew;
	
	ScheduleNode* prevReal = m_afterReal;
	bool prevRealNew = m_afterRealNew;
	
	list<ScheduleNode*> garbage;
	
	map<int, map<int, ScheduleNode*> > openStarts(*rememberOS);
	
	list<Edge> newEdges;
	
	int tilInPlan = m_tilInPlan;
	
	list<FFEvent>::iterator neItr = newEvents.begin();
	const list<FFEvent>::iterator neEnd = newEvents.end();
	
	for (; neItr != neEnd; ++neItr) {
		FFEvent & newEvent = *neItr;
		if (newEvent.action) {
			if (newEvent.time_spec == VAL::E_AT_START) {
				if (debugMerging || debugMinFlawScheduling) cout << "New: start of " << *(newEvent.action) << "\n";
				ScheduleNode* const startNode = new ScheduleNode(newEvent.action, sp, true);
				garbage.push_back(startNode);
				startNode->distFromZero() = DBL_MAX;
				startNode->distToZero() = -EPSILON;
	
				{
					startNode->scheduleAfter(0, EPSILON, DBL_MAX);
					addEdgeFromZero(startNode, EPSILON, DBL_MAX);
				}
	
				if (onlyPenalise) *onlyPenalise = startNode;
	
				/*planNodesToPokeAround.push_back(startNode);
				if (debugMinFlawScheduling) {
					cout << "Put " << startNode << " on queue to be min-flaw scheduled\n";
				}*/
				//fanIn[startNode] = 0;
				//earliest[startNode] = 0.0;
				//++targetNodeCount;
				
				//newNodeIndices[startNode] = nodeIndexCounter++;	
	
				/*if (afterNode) {
					if (debugMerging) cout << "Start put after afterNode " << afterNode << "\n";
					newEdges.push_back(Edge(afterNode, startNode, EPSILON, DBL_MAX));
					afterNode = 0;
				}*/
	
				if (prevTO) {
					if (debugMerging) cout << "Start put after prevTO " << prevTO << "\n";
					if (debugMinFlawScheduling) cout << prevTO << " before " << startNode << "\n";
					newEdges.push_back(Edge(prevTO, startNode, EPSILON, DBL_MAX));
					//++(fanIn[startNode]);
					//stnConstraints[startNode][prevTO] = -EPSILON;
				}
	
				if (prevReal && prevReal != prevTO) {
					newEdges.push_back(Edge(prevReal, startNode, EPSILON, DBL_MAX));
				}
		
	
				/*{
					
					list<Literal*> & deletes = startNode->getDeleteEffects();				
					
					list<Literal*>::iterator delItr = deletes.begin();
					const list<Literal*>::iterator delEnd = deletes.end();
	
					for (; delItr != delEnd; ++delItr) {
						Literal* const currLit = *delItr;
						//cout << "New node deletes " << *currLit << "\n";
						map<Literal*, ScheduleNode*, LiteralLT>::iterator gwFind = lastForSP.find(currLit);
						if (gwFind != lastForSP.end()) {
							//cout << "Last deletor for " << *currLit << " is new node " << *(startNode->getOp()) << "\n";
							lastDeletors[currLit] = startNode;
						}
					}
				
				}*/
	
				prevTO = startNode;
				prevReal = startNode;
	
				prevTOnew = true;
				prevRealNew = true;
				
				openStarts[newEvent.action->getID()][m_cl] = startNode;
	
				assert(openStarts[newEvent.action->getID()][m_cl] == startNode);
				//cout << "Start of action " << newEvent.action->getID() << " at step " << cl << " maps to " << startNode << "\n";
	
			} else {
				if (debugMerging || debugMinFlawScheduling) cout << "New: end of " << *(newEvent.action) << "\n";
				ScheduleNode* const endNode = new ScheduleNode(newEvent.action, sp, false);
				garbage.push_back(endNode);
				endNode->distFromZero() = DBL_MAX;
				endNode->distToZero() = -EPSILON;
	
				{
					endNode->scheduleAfter(0, EPSILON, DBL_MAX);
					addEdgeFromZero(endNode, EPSILON, DBL_MAX);
				}
	
				if (onlyPenalise) *onlyPenalise = endNode;
	
				ScheduleNode* startNode = 0;
				{
					map<int, map<int, ScheduleNode*> >::iterator osItr = openStarts.find(newEvent.action->getID());
					if (osItr != openStarts.end()) {
						//cout << "Looking for start of " << newEvent.action->getID() << " at step " << newEvent.pairWithStep;
						startNode = osItr->second[newEvent.pairWithStep];
						//cout << " - maps to " << startNode << "\n";
					} else {
						assert(osItr != openStarts.end());
					}
				}
				if (debugMinFlawScheduling) cout << "Corresponding end node: " << endNode << "\n";
				//earliest[endNode] = 0.0;
	//			++targetNodeCount;
	
	//			newNodeIndices[endNode] = nodeIndexCounter++;
	
	/*			if (afterNode) {
					if (debugMerging) cout << "End put after afterNode " << afterNode << "\n";
					newEdges.push_back(Edge(afterNode, endNode, EPSILON, DBL_MAX));							
	
					afterNode = 0;
					//++(fanIn[endNode]);
					//stnConstraints[endNode][afterNode] = -EPSILON;
				}
	*/
				if (prevTO && prevTO != startNode) {
					if (debugMerging) cout << "End put after prevTO " << prevTO << "\n";
					if (debugMinFlawScheduling) cout << prevTO << " before " << endNode << "\n";
					newEdges.push_back(Edge(prevTO, endNode, EPSILON, DBL_MAX));
				}
	
				if (prevReal && prevReal != prevTO && prevReal != startNode) {
					newEdges.push_back(Edge(prevReal, endNode, EPSILON, DBL_MAX));
				}
	
				startNode->setPartner(endNode);
				endNode->setPartner(startNode);
	
				//endNode->scheduleAfter(startNode);
				//startNode->scheduleBefore(endNode);
	
				const double actDur = startNode->getDuration();
	
				newEdges.push_back(Edge(startNode, endNode, actDur, actDur));
	
	
				if (debugMinFlawScheduling) cout << startNode << " before " << endNode << "\n";
	
	
				prevTO = endNode;
				prevTOnew = true;
	
				prevReal = endNode;
				prevRealNew = true;
	
				/*{
					list<Literal*> & deletes = endNode->getDeleteEffects();				
					
					list<Literal*>::iterator delItr = deletes.begin();
					const list<Literal*>::iterator delEnd = deletes.end();
	
					for (; delItr != delEnd; ++delItr) {
						Literal* const currLit = *delItr;
						//cout << "New node deletes " << *currLit << "\n";
						map<Literal*, ScheduleNode*, LiteralLT>::iterator gwFind = lastForSP.find(currLit);
						if (gwFind != lastForSP.end()) {
							lastDeletors[currLit] = endNode;
							//cout << "Last deletor for " << *currLit << " is new node " << *(endNode->getOp()) << "\n";
						}
					}
				}*/
			}
		} else if (newEvent.wait) {
		
	//		if (cl < considerUpTo) --onlyMFS;
			map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator findNewSN = oldToNew.find(newEvent.wait);
			assert(findNewSN != oldToNew.end());
			ScheduleNode* const deref = findNewSN->second;
	
			if (deref->tilNode()) {
				const int thisSee = deref->tilID() + 1;
				if (thisSee > tilInPlan) tilInPlan = thisSee;
			}				
	
			if (onlyPenalise) *onlyPenalise = deref;
			/*if (planNodesToPokeAround.empty()) {
				noEarlierThan = deref;
	
				if (debugMinFlawScheduling) {
					cout << "Earliest possible existing schedule point for m.f.s. set to " << noEarlierThan;
					cout << ",\nwhich has index " << oldTOIndices[noEarlierThan] << "\n";
	
				}
			}*/
			
			if (newEvent.getEffects) {
				if (!deref->tilNode()) {
					deref->addSubProblem(sp);
					deref->getPartner()->addSubProblem(sp);
				}
				
				if (prevReal && prevReal != prevTO) {
					if (prevRealNew) {
						newEdges.push_back(Edge(prevReal, deref, EPSILON, DBL_MAX));
					} else {
						bool explicitLink = false;
						{
							list<pair<ScheduleNode*, pair<double, double> > > & succs = prevReal->getSuccessors();
							list<pair<ScheduleNode*, pair<double, double> > >::iterator oItr = succs.begin();
							const list<pair<ScheduleNode*, pair<double, double> > >::iterator oEnd = succs.end();
	
							for (; oItr != oEnd; ++oItr) {
	
								if (oItr->first == deref) {
									explicitLink = true;
									break;
								}
	
							}
	
						}
						{
							list<pair<ScheduleNode*, pair<double, double> > > & succs = deref->getSuccessors();
							list<pair<ScheduleNode*, pair<double, double> > >::iterator oItr = succs.begin();
							const list<pair<ScheduleNode*, pair<double, double> > >::iterator oEnd = succs.end();
	
							for (; oItr != oEnd; ++oItr) {
	
								if (oItr->first == prevReal) {
									if (debugMerging) cout << "Conflict: already have A-before-B, now trying to say B-before-A\n";
									list<ScheduleNode*>::iterator fiItr = garbage.begin();
									const list<ScheduleNode*>::iterator fiEnd = garbage.end();
									for (; fiItr != fiEnd; ++fiItr) {
										delete *fiItr;
									}
									return false;
								}
	
							}
						}
						
						if (!explicitLink) {
							newEdges.push_back(Edge(prevReal, deref, EPSILON, DBL_MAX));
						}
					}
					
				}
				prevReal = deref;
			}
	
			if (prevTO) {
				
				if (prevTOnew) {
					newEdges.push_back(Edge(prevTO, deref, EPSILON, DBL_MAX));
				} else {
					bool explicitLink = false;
					{
						list<pair<ScheduleNode*, pair<double, double> > > & succs = prevTO->getSuccessors();
						list<pair<ScheduleNode*, pair<double, double> > >::iterator oItr = succs.begin();
						const list<pair<ScheduleNode*, pair<double, double> > >::iterator oEnd = succs.end();
	
						for (; oItr != oEnd; ++oItr) {
	
							if (oItr->first == deref) {
								explicitLink = true;
								break;
							}
	
						}
	
					}
					{
						list<pair<ScheduleNode*, pair<double, double> > > & succs = deref->getSuccessors();
						list<pair<ScheduleNode*, pair<double, double> > >::iterator oItr = succs.begin();
						const list<pair<ScheduleNode*, pair<double, double> > >::iterator oEnd = succs.end();
	
						for (; oItr != oEnd; ++oItr) {
	
							if (oItr->first == prevTO) { // was deref, but I think that's wrong
								if (debugMerging) cout << "Conflict: already have A-before-B, now trying to say B-before-A\n";
								list<ScheduleNode*>::iterator fiItr = garbage.begin();
								const list<ScheduleNode*>::iterator fiEnd = garbage.end();
								for (; fiItr != fiEnd; ++fiItr) {
									delete *fiItr;
								}
								return false;
							}
	
						}
					}
					
					if (!explicitLink) {
						newEdges.push_back(Edge(prevTO, deref, EPSILON, DBL_MAX));
					}
	
					
	
					if (debugMerging) cout << "Wait: old " << *(prevTO->getOp()) << (prevTO->isStart() ? " (start)" : " (end)") << " before new " << *(deref->getOp()) << (deref->isStart() ? " (start)" : " (end)")  << "\n";	
				} 
			} else {
				if (debugMerging) cout << "Wait: old " << *(deref->getOp()) << (deref->isStart() ? " (start)" : " (end)")  << "\n";
			}
	
	
	
			prevTO = deref;
			prevTOnew = false;
	//					graphRootNode = 0;
	
		}
	}
		
	{
		const bool rpgPrint = false;

		if (rpgPrint || debugMerging) cout << "Merging in from RPG - \n";

		list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >::iterator rpItr = relaxedPlan.begin();
		const list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >::iterator rpEnd = relaxedPlan.end();

		list<ScheduleNode*> aList;
		//list<ScheduleNode*> bList;
		list<ScheduleNode*> * prevLayer = &aList;
		//list<ScheduleNode*> * thisLayer = &bList;
		if (prevTO) {
			prevLayer->push_back(prevTO);
		} else {
			prevLayer->push_back(0);
		}
		//if (graphRootNode) prevLayer->push_back(graphRootNode);
		//double prevTS = -EPSILON;

		double specialOffset = 0.0;

		for (; rpItr != rpEnd; ++rpItr) {

			double thisTS = rpItr->first + specialOffset;

			if (rpgPrint || debugMerging) cout << "\tlayer " << thisTS << "\n";
			
			list<pair<instantiatedOp*, VAL::time_spec> >::iterator layerItr = rpItr->second.begin();
			const list<pair<instantiatedOp*, VAL::time_spec> >::iterator layerEnd = rpItr->second.end();

			list<ScheduleNode*>::iterator plStart = prevLayer->begin();
			const list<ScheduleNode*>::iterator plEnd = prevLayer->end();
//				const int fanInTmp = prevLayer->size();

			for (; layerItr != layerEnd; ++layerItr) {
				
				if (layerItr->second == VAL::E_AT_START) {

					if (!GlobalSchedule::checkOpenStartsOnly) {
						ScheduleNode* const newNode = new ScheduleNode(layerItr->first, sp, true);
		
						if (rpgPrint || debugMerging) cout << "\t\tstart of " << *(newNode->getOp()) << "\n";
	
						garbage.push_back(newNode);
						newNode->distFromZero() = DBL_MAX;
						newNode->distToZero() = -EPSILON;
	
						{
							newNode->scheduleAfter(0, EPSILON, DBL_MAX);
							addEdgeFromZero(newNode, EPSILON, DBL_MAX);
						}
	//					++targetNodeCount;
	
						for (list<ScheduleNode*>::iterator plItr = plStart; plItr != plEnd; ++plItr) {
					
							newEdges.push_back(Edge(*plItr, newNode, thisTS+EPSILON, DBL_MAX));
							if (debugMerging) cout << "\t\tputting between " << thisTS+EPSILON << " and infinity after " << (*plItr ? " previous node" : " time zero") << "\n";
							
						}						
	
	/*						if (prevLayer->empty()) {
							//cout << "Special case, dealing with it\n";
						
							prevLayer->push_back(newNode);
							specialOffset += EPSILON;
							thisTS = rpItr->first + specialOffset;
							plStart = prevLayer->begin();
						}
	*/
						//fanIn[newNode] = fanInTmp;
						//if (!fanInTmp) openList.push_back(pair<ScheduleNode*, double>(newNode,0.0));
	
						//newNodeIndices[newNode] = nodeIndexCounter++;

					}
					

				} else {
				

				
					map<int, list<list<StartEvent>::iterator > >::iterator ceCheck = compulsaryEnds.find(layerItr->first->getID());

					if (ceCheck == compulsaryEnds.end()) {
						if (!GlobalSchedule::checkOpenStartsOnly) {
							ScheduleNode* const newNode = new ScheduleNode(layerItr->first, sp, false);
							if (rpgPrint || debugMerging) cout << "\t\tend of " << *(newNode->getOp()) << "\n";
							garbage.push_back(newNode);
							newNode->distFromZero() = DBL_MAX;
							newNode->distToZero() = -EPSILON;
	
							{
								newNode->scheduleAfter(0, EPSILON, DBL_MAX);
								addEdgeFromZero(newNode, EPSILON, DBL_MAX);
							}
							//++targetNodeCount;
							for (list<ScheduleNode*>::iterator plItr = plStart; plItr != plEnd; ++plItr) {
						
								newEdges.push_back(Edge(*plItr, newNode, thisTS+EPSILON, DBL_MAX));
								if (debugMerging) cout << "\t\tputting between " << thisTS+EPSILON << " and infinity after " << (*plItr ? " previous node" : " time zero") << "\n";
								
							}
						}
					} else {

						StartEvent & pairWith = *(ceCheck->second.front());
						ceCheck->second.pop_front();

						if (ceCheck->second.empty()) compulsaryEnds.erase(ceCheck);

						if (!pairWith.compulsaryEnd) {

							//int thisFanIn = fanInTmp;

							ScheduleNode* const newNode = new ScheduleNode(layerItr->first, sp, false);
							if (rpgPrint || debugMerging) cout << "\t\tend of " << *(newNode->getOp()) << "\n";
							garbage.push_back(newNode);
							newNode->distFromZero() = DBL_MAX;
							newNode->distToZero() = -EPSILON;
	
							{
								newNode->scheduleAfter(0, EPSILON, DBL_MAX);
								addEdgeFromZero(newNode, EPSILON, DBL_MAX);
							}
							//++targetNodeCount;
							for (list<ScheduleNode*>::iterator plItr = plStart; plItr != plEnd; ++plItr) {
						
								newEdges.push_back(Edge(*plItr, newNode, thisTS+EPSILON, DBL_MAX));
								if (debugMerging) cout << "\t\tputting between " << thisTS+EPSILON << " and infinity after " << (*plItr ? " previous node" : " time zero") << "\n";
								
							}

							map<int, map<int, ScheduleNode*> >::iterator cosItr = openStarts.find(layerItr->first->getID());
							
							if (cosItr != openStarts.end()) {
								//++thisFanIn;
								ScheduleNode* const cc = cosItr->second[pairWith.stepID];
								newEdges.push_back(Edge(cc, newNode, EPSILON, RPGBuilder::getOpDuration(layerItr->first->getID())));
								if (debugMerging) cout << "\t\tputting at duration after its start\n";
								
								newNode->setPartner(cc);
								cc->setPartner(newNode);
							}

							//fanIn[newNode] = thisFanIn;
							//if (!thisFanIn) openList.push_back(pair<ScheduleNode*, double>(newNode,0.0));

							//newNodeIndices[newNode] = nodeIndexCounter++;

						} else {

							ScheduleNode* const newNode = oldToNew[reallyOldToNew[pairWith.compulsaryEnd]];
							ScheduleNode* const partner = newNode->getPartner();

							if (rpgPrint || debugMerging) cout << "\t\tend of " << *(newNode->getOp()) << ", compulsary pairing\n";
							
							for (list<ScheduleNode*>::iterator plItr = plStart; plItr != plEnd; ++plItr) {
								if (*plItr != partner) {
									newEdges.push_back(Edge(*plItr, newNode, thisTS+EPSILON, DBL_MAX));
									if (debugMerging) cout << "\t\tputting between " << thisTS+EPSILON << " and infinity after " << (*plItr ? " previous node" : " time zero") << "\n";
								} else {
									if (debugMerging) cout << "\t\tdon't need to extra partner temporal constraint, one already exists\n";
								}
							}
							
	
							

						}
					}
				
				}
				
			}


		}

	}
	
	if (prevTO) {
		const int tilSeen = tilEvents.size();
		for (int i = tilInPlan; i < tilSeen; ++i) {
			newEdges.push_back(Edge(prevTO, tilEvents[i], EPSILON, DBL_MAX));
		}
	}
	
	
	{
		list<Edge>::iterator elItr = newEdges.begin();
		const list<Edge>::iterator elEnd = newEdges.end();

		for (int ei = 0; elItr != elEnd; ++elItr, ++ei) {
			
			if (!Propagation(*elItr, edgesFromZero)) {
				//cout << "Edge of index " << ei << " wasn't accepted\n";
				//cout << *(elItr->from->getOp()) << " " << (elItr->from->isStart() ? "start" : "end") << " -> " << *(elItr->to->getOp()) << " " << (elItr->to->isStart() ? "start" : "end") << " [" << elItr->min << ", " << elItr->max << "]\n";

				list<ScheduleNode*>::iterator fiItr = garbage.begin();
				const list<ScheduleNode*>::iterator fiEnd = garbage.end();
				for (; fiItr != fiEnd; ++fiItr) {
					delete *fiItr;
				}
				return false;
				
			}
		}

		
	}
	{
		list<ScheduleNode*>::iterator fiItr = garbage.begin();
		const list<ScheduleNode*>::iterator fiEnd = garbage.end();
		for (; fiItr != fiEnd; ++fiItr) {
			events.push_back(*fiItr);
		}
	}
	garbage = events;
	events.clear();

	list<pair<ScheduleNode*, double> > openList;

	{ // new STN bit
		list<ScheduleNode*>::iterator nLookup = garbage.begin();
		const list<ScheduleNode*>::iterator nLookupEnd = garbage.end();

		for (; nLookup != nLookupEnd; ++nLookup) {

			ScheduleNode* const currSN = *nLookup;
			double currTS = currSN->distToZero();

			if (currTS < 0) currTS = 0.0 - currTS;



			if (openList.empty()) {
				openList.push_back(pair<ScheduleNode*, double>(currSN, currTS));
			} else {
				list<pair<ScheduleNode*, double> >::iterator insItr = openList.begin();
				const list<pair<ScheduleNode*, double> >::iterator insEnd = openList.end();

				//for (; insItr != insEnd && (insItr->second <= currTS); ++insItr);
				for (; insItr != insEnd && ((insItr->second - currTS) < (EPSILON / 2)); ++insItr);

				openList.insert(insItr, pair<ScheduleNode*, double>(currSN, currTS));
			}
		}


	}

	{

		if (!timeline.empty()) {
			list<TimelinePoint*>::iterator tlItr = timeline.begin();
			const list<TimelinePoint*>::iterator tlEnd = timeline.end();
	
			++tlItr;

			for (; tlItr != tlEnd; ++tlItr) delete *tlItr;

			tlItr = timeline.begin();
			++tlItr;
			if (tlItr != tlEnd) timeline.erase(tlItr, tlEnd);
		}

	}

	while (!openList.empty()) {
		
		ScheduleNode * const currNode = openList.front().first;
		const double currTS = openList.front().second;
		
		openList.pop_front();

		events.push_back(currNode);

		TimelinePoint * const newTP = new TimelinePoint(*(timeline.back()), currNode->getAddEffects(), currNode->getDeleteEffects());

		newTP->after = events.end();
		--(newTP->after);

		newTP->timestamp = currTS;

		timeline.push_back(newTP);


		
	}

	return true;

};

void CandidateSchedule::addTILs(list<RPGBuilder::FakeTILAction> & TILs) {

	tilEvents = vector<ScheduleNode*>(TILs.size());
	
	list<RPGBuilder::FakeTILAction>::iterator tilItr = TILs.begin();
	const list<RPGBuilder::FakeTILAction>::iterator tilEnd = TILs.end();

	for (int i = 0; tilItr != tilEnd; ++tilItr, ++i) {

		ScheduleNode* const tilNode = new ScheduleNode(tilItr->addEffects, tilItr->delEffects, tilItr->duration, i);
		tilNode->scheduleAfter(0, tilItr->duration, tilItr->duration);
		addEdgeFromZero(tilNode, tilItr->duration, tilItr->duration);
		events.push_back(tilNode);
	
		TimelinePoint * const newTP = new TimelinePoint(*(timeline.back()), tilNode->getAddEffects(), tilNode->getDeleteEffects());

		newTP->after = events.end();
		--(newTP->after);

		timeline.push_back(newTP);
		

		newTP->timestamp = tilItr->duration;

		tilEvents[i] = tilNode;

	}

};

#ifdef ENABLE_DEBUGGING_HOOKS

vector<bool> GlobalSchedule::actionHasToBeKept;
const char * GlobalSchedule::planFilename = 0;

void GlobalSchedule::markThatActionsInPlanHaveToBeKept()
{    
    actionHasToBeKept.resize(instantiatedOp::howMany(), false);
    
    if (!planFilename) return;
        
    ifstream * const current_in_stream = new ifstream(planFilename);
    if (!current_in_stream->good()) {
        cout << "Exiting: could not open plan file " << planFilename << "\n";
        exit(1);
    }
    
    VAL::yfl = new yyFlexLexer(current_in_stream, &cout);
    yyparse();
    
    VAL::plan * const the_plan = dynamic_cast<VAL::plan*>(top_thing);
    
    delete VAL::yfl;
    delete current_in_stream;
    
    
    
    if (!the_plan) {
        cout << "Exiting: failed to load plan " << planFilename << "\n";
        exit(1);
    };
    
    if (!theTC->typecheckPlan(the_plan)) {
        cout << "Exiting: error when type-checking plan " << planFilename << "\n";
        exit(1);
    }
        
    pc_list<plan_step*>::const_iterator planItr = the_plan->begin();
    const pc_list<plan_step*>::const_iterator planEnd = the_plan->end();
    
    for (int idebug = 0, i = 0; planItr != planEnd; ++planItr, ++i, ++idebug) {
        plan_step* const currStep = *planItr;
        
        instantiatedOp * const currOp = instantiatedOp::findInstOp(currStep->op_sym, currStep->params->begin(), currStep->params->end());
        if (!currOp) {
            cout << "Exiting: step " << idebug << " in the input plan uses an action that the instantiation code did not create.\n";
            cout << "Use VALfiles/testing/checkplanstepsexist to find out why this is the case\n";
            exit(1);
        }
        const int ID = currOp->getID();
        
        actionHasToBeKept[ID] = true;
        
        cout << "Marking that " << *currOp << " (" << ID << ") must not be eliminated by the preprocessor\n";
    }
            
}

void GlobalSchedule::eliminatedAction(const int & i, const char * synopsis)
{
    if (!planFilename) return;
        
    if (actionHasToBeKept[i]) {
        cout << "Error in preprocessor.  Pruned operator " << i << ", but should not have done.  Reason given was:\n";
        cout << synopsis << std::endl;
        exit(1);
    } else {
        cout << "Eliminated " << i << ": " << synopsis << std::endl;
    }
}

#endif

}


