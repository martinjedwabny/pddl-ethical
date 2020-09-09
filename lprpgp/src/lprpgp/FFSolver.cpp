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

#include "FFSolver.h"

#include "GlobalSchedule.h"

#include <float.h>
#include <sys/times.h>
#include "PreferenceHandler.h"
#include "landmarksanalysis.h"

#include "colours.h"

using std::endl;

namespace Planner {

list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > > FFcache_relaxedPlan;
list<pair<int, VAL::time_spec > > FFcache_helpfulActions;
int FFcache_h;
bool FFcache_upToDate;
bool FFheader_upToDate;
bool FFonly_one_successor;
bool FF::steepestDescent = true;

double FF::capOnPreferenceCost = DBL_MAX;
double FF::doubleU = 5.0;
bool FF::WAStar = false;
bool FF::timeWAStar = false;
bool FF::ignorePenalties = false;
bool FF::initialAdvancing = true;
int FF::disablePareto = 0;
bool FF::justWaitIncr = true;
bool FF::bestFirstSearch = true;
bool FF::helpfulActions = true;
bool FF::disableWaits = true;
bool FF::pruneMemoised = true;
bool FF::multipleHelpfuls = true;
bool FF::stagnantCommitted = true;

bool FF::firstImprover = false;

bool FF::relaxedGoalJump = false;
bool FF::neverReuseOrWait = false;
bool FF::incrementalExpansion = false;
bool FF::skipEHC = false;
bool FF::zealousEHC = true;
bool FF::startsBeforeEnds = true;
bool FF::invariantRPG = true;
bool FF::tsChecking = true;
bool FF::allowDualOpenList = true;
bool FF::useDualOpenList = false;
double FF::bestSolutionPCost = DBL_MAX;

bool FF::useWeightedSumWithPrefCost = false;
double FF::prefWeightInWeightedSum = 1.0;

#ifdef FFSEARCHDEBUGHOOKS
bool FF::actuallyPlanGivenPreviousSolution = false;

struct StepInfo {
  
    int stepID;
    instantiatedOp * reachedBy;    
    vector<AutomatonPosition> preferenceStatus;
    FF::HTrio heuristicValue;
    bool seenInSearch;
    bool lastStepInPlan;
    
    StepInfo(const int & i, instantiatedOp* o, const vector<AutomatonPosition> & p, const FF::HTrio & h, const bool & l)
        : stepID(i), reachedBy(o),  preferenceStatus(p), heuristicValue(h), seenInSearch(false), lastStepInPlan(l)
    {
    }
};

#endif
    

void printState(MinimalState & e) {

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

void printState(ExtendedMinimalState & e) {

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

namespace CSBase {

	vector<bool> ignorableFluents;
	
    int compareSets(const set<int> & a, const set<int> & b) {
	
		const bool aEmpty = a.empty();
		const bool bEmpty = b.empty();
		if (aEmpty && bEmpty) return 0;
		if (aEmpty && !bEmpty) return -1;
		if (bEmpty && !aEmpty) return 1;

		set<int>::const_reverse_iterator aItr = a.rbegin();
		const set<int>::const_reverse_iterator aEnd = a.rend();

		set<int>::const_reverse_iterator bItr = b.rbegin();
		const set<int>::const_reverse_iterator bEnd = b.rend();

		for (; aItr != aEnd && bItr != bEnd; ++aItr, ++bItr) {
			const int av = *aItr;
			const int bv = *bItr;
			if (av < bv) return 1;
			if (av > bv) return -1;
		}

		if (aItr == aEnd && bItr != bEnd) return 1;
		if (aItr != aEnd && bItr == bEnd) return -1;

		return 0;
	}

	void skipTerminates(list<StartEvent>::const_reverse_iterator & itr, const list<StartEvent>::const_reverse_iterator & itrEnd) {
		while (itr != itrEnd && itr->terminated) ++itr;
	}

	int compareLists(const list<StartEvent> & a, const list<StartEvent> & b) {
	
		list<StartEvent>::const_reverse_iterator aItr = a.rbegin();
		const list<StartEvent>::const_reverse_iterator aEnd = a.rend();

		list<StartEvent>::const_reverse_iterator bItr = b.rbegin();
		const list<StartEvent>::const_reverse_iterator bEnd = b.rend();

		skipTerminates(aItr, aEnd);
		skipTerminates(bItr, bEnd);

		while (aItr != aEnd && bItr != bEnd) {
			const StartEvent& av = *aItr;
			const StartEvent& bv = *bItr;
			if (av.actID < bv.actID) return 1;
			if (av.actID > bv.actID) return -1;

//			if (av.stepID < bv.stepID) return 1;
//			if (av.stepID > bv.stepID) return -1;

			if (av.compulsaryEnd < bv.compulsaryEnd) return 1;
			if (av.compulsaryEnd > bv.compulsaryEnd) return -1;
			
			++aItr;
			++bItr;

			skipTerminates(aItr, aEnd);
			skipTerminates(bItr, bEnd);

		}

		if (aItr == aEnd && bItr != bEnd) return 1;
		if (aItr != aEnd && bItr == bEnd) return -1;

		return 0;
	}


	int compareMaps(const map<int, int> & a, const map<int, int> & b) {
	
		const bool aEmpty = a.empty();
		const bool bEmpty = b.empty();
		if (aEmpty && bEmpty) return 0;
		if (aEmpty && !bEmpty) return -1;
		if (bEmpty && !aEmpty) return 1;

		map<int, int>::const_reverse_iterator aItr = a.rbegin();
		const map<int, int>::const_reverse_iterator aEnd = a.rend();

		map<int, int>::const_reverse_iterator bItr = b.rbegin();
		const map<int, int>::const_reverse_iterator bEnd = b.rend();

		for (; aItr != aEnd && bItr != bEnd; ++aItr, ++bItr) {
			const int av = aItr->first;
			const int bv = bItr->first;
			if (av < bv) return 1;
			if (av > bv) return -1;
			const int av2 = aItr->second;
			const int bv2 = bItr->second;
			if (av2 < bv2) return 1;
			if (av2 > bv2) return -1;
		}

		if (aItr == aEnd && bItr != bEnd) return 1;
		if (aItr != aEnd && bItr == bEnd) return -1;

		return 0;
	}

	int compareVecs(const vector<double> & a, const vector<double> & b) {
		const int aSize = a.size();
		const int bSize = b.size();
		if (!aSize && !bSize) return 0;
		if (aSize < bSize) return 1;
		if (bSize > aSize) return -1;

		for (int i = 0; i < aSize; ++i) {
			if (!ignorableFluents[i]) {
				const double av = a[i];
				const double bv = b[i];
				if (av < bv) return 1;
				if (av > bv) return -1;
			}
		}

		return 0;
	}

	int compareSecondMaps(const map<int, ScheduleNode*> & a, const map<int, ScheduleNode*> & b) {
	
		const bool aEmpty = a.empty();
		const bool bEmpty = b.empty();
		if (aEmpty && bEmpty) return 0;
		if (aEmpty && !bEmpty) return -1;
		if (bEmpty && !aEmpty) return 1;

		map<int, ScheduleNode*>::const_reverse_iterator aItr = a.rbegin();
		const map<int, ScheduleNode*>::const_reverse_iterator aEnd = a.rend();

		map<int, ScheduleNode*>::const_reverse_iterator bItr = b.rbegin();
		const map<int, ScheduleNode*>::const_reverse_iterator bEnd = b.rend();

		for (; aItr != aEnd && bItr != bEnd; ++aItr, ++bItr) {
			const int av = aItr->first;
			const int bv = bItr->first;
			if (av < bv) return 1;
			if (av > bv) return -1;
			ScheduleNode* const av2 = aItr->second;
			ScheduleNode* const bv2 = bItr->second;
			if (av2 < bv2) return 1;
			if (av2 > bv2) return -1;
		}

		if (aItr == aEnd && bItr != bEnd) return 1;
		if (aItr != aEnd && bItr == bEnd) return -1;

		return 0;
	}

/*	int compareLists(const list<int> & a, const list<int> & b) {

		const bool aEmpty = a.empty();
		const bool bEmpty = b.empty();
		if (aEmpty && bEmpty) return 0;
		if (aEmpty && !bEmpty) return -1;
		if (bEmpty && !aEmpty) return 1;

		list<int>::const_iterator aItr = a.begin();
		const list<int>::const_iterator aEnd = a.end();

		list<int>::const_iterator bItr = b.begin();
		const list<int>::const_iterator bEnd = b.end();

		for (; aItr != aEnd && bItr != bEnd; ++aItr, ++bItr) {
			const int av = *aItr;
			const int bv = *bItr;
			if (av < bv) return 1;
			if (av > bv) return -1;
		}

		if (aItr == aEnd && bItr != bEnd) return 1;
		if (aItr != aEnd && bItr == bEnd) return -1;

		return 0;

	}*/

};

struct CompareStates {


	bool operator()(const ExtendedMinimalState & a, const ExtendedMinimalState & b) const {

		const int csVal = CSBase::compareSets(a.first, b.first);
		if (csVal > 0) {
			return true;
		} else if (csVal < 0) {
			return false;
		}
		
		const int cvVal = CSBase::compareVecs(a.second, b.second);
		if (cvVal > 0) {
			return true;
		} else if (cvVal < 0) {
			return false;
		}
	
		const int saVal = CSBase::compareMaps(a.startedActions, b.startedActions);

		if (saVal > 0) {
			return true;
		} else if (saVal < 0) {
			return false;
		}

/*		const int invVal = CSBase::compareMaps(a.invariants, b.invariants);

		if (invVal > 0) {
			return true;
		} else if (invVal < 0) {
			return false;
		}*/
	
		/*const int ceVal = CSBase::compareLists(a.startEventQueue, b.startEventQueue);

		if (ceVal > 0) {
			return true;
		} else if (ceVal < 0) {
			return false;
		}*/
	  
		if (a.nextTIL < b.nextTIL) return true;
		if (a.nextTIL > b.nextTIL) return false;

		return false;
	}

};


struct CompareStatesZealously {


	bool operator()(const ExtendedMinimalState & a, const ExtendedMinimalState & b) const {

		const int csVal = CSBase::compareSets(a.first, b.first);
		if (csVal > 0) {
			return true;
		} else if (csVal < 0) {
			return false;
		}
		
		const int cvVal = CSBase::compareVecs(a.second, b.second);
		if (cvVal > 0) {
			return true;
		} else if (cvVal < 0) {
			return false;
		}
	
		const int saVal = CSBase::compareMaps(a.startedActions, b.startedActions);

		if (saVal > 0) {
			return true;
		} else if (saVal < 0) {
			return false;
		}

		/*const int invVal = CSBase::compareMaps(a.invariants, b.invariants);

		if (invVal > 0) {
			return true;
		} else if (invVal < 0) {
			return false;
		}
		  */
		if (a.nextTIL < b.nextTIL) return true;
		if (a.nextTIL > b.nextTIL) return false;

		return false;
	}

};


#ifdef FFSEARCHDEBUGHOOKS
map<ExtendedMinimalState, list<StepInfo> , CompareStatesZealously> knownGoodStates;
#endif

FFEvent::FFEvent(instantiatedOp* a, const double & dMin, const double & dMax) : action(a), time_spec(VAL::E_AT_START), minDuration(dMin), maxDuration(dMax), pairWithStep(-1), wait(0), noop(false), getEffects(true) {    
	//cout << "FFEvent start\n";
};

FFEvent::FFEvent(const FFEvent & f) : action(f.action), time_spec(f.time_spec), minDuration(f.minDuration), maxDuration(f.maxDuration), pairWithStep(f.pairWithStep), wait(f.wait), noop(f.noop), getEffects(f.getEffects) {
    //cout << "FFEvent copy\n";
};
FFEvent::FFEvent() : action(0), time_spec(VAL::E_AT_START), minDuration(0.0), maxDuration(0.0), wait(0), noop(true) {
	//cout << "FFEvent default\n";
};
FFEvent & FFEvent::operator=(const FFEvent & f) {
	//cout << "FFEvent assignment op\n";
	action = f.action;
	time_spec = f.time_spec;
	minDuration = f.minDuration;
	maxDuration = f.maxDuration;
	pairWithStep = f.pairWithStep;
	wait = f.wait;
	noop = f.noop;
	getEffects = f.getEffects;
	return *this;
}

class SearchQueueItem {

public:
    
    pair<list<SearchQueueItem*>::iterator,int> positionA;
    pair<list<SearchQueueItem*>::iterator,int> positionB;
    
	ExtendedMinimalState state;
	list<FFEvent> plan;	

	list<pair<int, VAL::time_spec> > helpfulActions;
	list<pair<double, double> > helpfulHowManyTimes;	
	FF::HTrio heuristicValue;
	//list<TimelinePoint*>::iterator tlPoint;
	list<ScheduleNode*> pointsOfConflict;

	int nextTIL;
	
	SearchQueueItem() : nextTIL(0) {
        positionA.second = -1;
        positionB.second = -1;
	};


	SearchQueueItem(const ExtendedMinimalState & sIn) : state(sIn) , nextTIL(0) {
        positionA.second = -1;
        positionB.second = -1;
	};


	void printPlan() {
		if (GlobalSchedule::globalVerbosity & 2) {
			list<FFEvent>::iterator planItr = plan.begin();
			const list<FFEvent>::iterator planEnd = plan.end();
	
			for (int i = 0; planItr != planEnd; ++planItr, ++i) {
				if (planItr->action) {
					cout << i << ": " << *(planItr->action) << ", " << (planItr->time_spec == VAL::E_AT_START ? "start" : "end") << "\n";
				} else if (planItr->wait) {
					if (planItr->wait->tilNode()) {
						cout << i << ": wait for TIL node " << planItr->wait->tilID();
					} else {
						cout << i << ": wait for " << *(planItr->wait->getOp()) << " " << (planItr->wait->isStart() ? "start" : "end");
					}
					if (planItr->getEffects) cout << ", getting its effects";
					cout << "\n";
					
				} else {
					cout << i << ": null node!\n";
					assert(false);
				}
			}
		}
	}
	
	void printNewSolution()
	{
        cout << "\n\n---- New best solution ----\n";
        cout << "; Preference violation cost = " << PreferenceHandler::getCurrentCost(state) + RPGBuilder::getPermanentViolationCost()  << " , Time = ";
        {
            tms refReturn;
            times(&refReturn);              
            double secs = ((double)refReturn.tms_utime + (double)refReturn.tms_stime) / ((double) sysconf(_SC_CLK_TCK));
            
            int twodp = (int) (secs * 100.0);
            int wholesecs = twodp / 100;
            int centisecs = twodp % 100;
            
            cout << wholesecs << ".";
            if (centisecs < 10) cout << "0";
            cout << centisecs << "\n";
            cout << ";\n; Time = " << wholesecs << ".";
            if (centisecs < 10) cout << "0";
            cout << centisecs << "\n";
        }
        cout << "; States evaluated: " << RPGBuilder::statesEvaluated << endl;                
        cout << ";\n;\n";
        
        if (!RPGBuilder::getPreferences().empty()) {
            cout << "; " << PreferenceHandler::getCurrentViolations(state) << endl;
            cout << ";\n;\n";
        }
        
        list<FFEvent>::iterator planItr = plan.begin();
        const list<FFEvent>::iterator planEnd = plan.end();
                
        for (int i = 0; planItr != planEnd; ++planItr) {            
            if (planItr->time_spec == VAL::E_AT_START) {
                cout << (i * 1.001) << ": " << *(planItr->action) << " [1.000]\n";
                ++i;
            }
        }
        cout << "\n\n---------------------------\n";
    }
    
    static void printSolutionList(list<const FFEvent*> & plan, const double & cost) {
        cout << "\n\n---- New best solution ----\n";
        cout << "; Preference violation cost = " << cost + RPGBuilder::getPermanentViolationCost()  << " , Time = ";
        {
            tms refReturn;
            times(&refReturn);              
            double secs = ((double)refReturn.tms_utime + (double)refReturn.tms_stime) / ((double) sysconf(_SC_CLK_TCK));
            
            int twodp = (int) (secs * 100.0);
            int wholesecs = twodp / 100;
            int centisecs = twodp % 100;
            
            cout << wholesecs << ".";
            if (centisecs < 10) cout << "0";
            cout << centisecs << "\n";
            cout << ";\n; Time = " << wholesecs << ".";
            if (centisecs < 10) cout << "0";
            cout << centisecs << "\n";
        }
        cout << "; States evaluated: " << RPGBuilder::statesEvaluated << endl;                
        cout << ";\n;\n";
        
        if (!RPGBuilder::getPreferences().empty()) {
            cout << "; Violations not known on this code pathway: improved using AE\n";
            cout << ";\n;\n";
        }
        
        list<const FFEvent*>::iterator planItr = plan.begin();
        const list<const FFEvent*>::iterator planEnd = plan.end();
                
        for (int i = 0; planItr != planEnd; ++planItr) {            
            if ((*planItr)->time_spec == VAL::E_AT_START) {
                cout << (i * 1.001) << ": " << *((*planItr)->action) << " [1.000]\n";
                ++i;
            }
        }
        cout << "\n\n---------------------------\n";
    }

};

class SearchQueue {

private:

	map<double, list<SearchQueueItem*> > qOneA;
	map<double, list<SearchQueueItem*> > qTwoA;
    
    map<double, list<SearchQueueItem*> > qOneB;
    map<double, list<SearchQueueItem*> > qTwoB;
    
    bool debug;
    
public:

    bool seenAnyNonZeroPCosts; // public, so FF solver can set it when needed
    
    SearchQueue()
    {
        debug = false;
        seenAnyNonZeroPCosts = false;
    }

	~SearchQueue()
	{
		clearall();
	}

    bool useOtherQueue(const int & ilist)
    {
        if (!FF::useDualOpenList) {
            assert(ilist == 1);
            return false;
        }
        if (ilist == 1) {
            return (qOneA.empty() && qTwoA.empty());
        }
        if (ilist == 2) {
            return (qOneB.empty() && qTwoB.empty());
        }
        std::cerr << "Internal error - unexpected open list ID, " << ilist << endl;
        exit(1);
        return true;
    }

    void clear(const int & ilist)
    {
        for (int pass = 0; pass < 2; ++pass) {
            
            map<double, list<SearchQueueItem*> > & currMap = (ilist == 1 ? (pass ? qTwoA : qOneA) : (pass ? qTwoB : qOneB));
            
            map<double, list<SearchQueueItem*> >::iterator cmItr = currMap.begin();
            const map<double, list<SearchQueueItem*> >::iterator cmEnd = currMap.end();
            
            for (; cmItr != cmEnd; ++cmItr) {
                list<SearchQueueItem*>::iterator qItr = cmItr->second.begin();
                const list<SearchQueueItem*>::iterator qEnd = cmItr->second.end();
                
                for (; qItr != qEnd; ++qItr) {                    
                    pair<list<SearchQueueItem*>::iterator,int> & otherPosn = (ilist == 1 ? (*qItr)->positionB : (*qItr)->positionA);
                    if (otherPosn.second == -1) {
                        delete *qItr;
                    } else {
                        if (ilist == 1) {
                            (*qItr)->positionA.second = -1;
                            //cout << "Marking " << *qItr << " as no longer being in list 1\n";
                        } else {
                            (*qItr)->positionB.second = -1;
                            //cout << "Marking " << *qItr << " as no longer being in list 2\n";
                        }
                    }
                }                
            }
            currMap.clear();
                        
        }
    }
    
    
	void clearall() {
        for (int pass = 0; pass < 2; ++pass) {

            map<double, list<SearchQueueItem*> > & currMap = (pass ? qTwoA : qOneA);
            
            map<double, list<SearchQueueItem*> >::iterator cmItr = currMap.begin();
            const map<double, list<SearchQueueItem*> >::iterator cmEnd = currMap.end();

            for (; cmItr != cmEnd; ++cmItr) {
                list<SearchQueueItem*>::iterator qItr = cmItr->second.begin();
                const list<SearchQueueItem*>::iterator qEnd = cmItr->second.end();

                for (; qItr != qEnd; ++qItr) {
                    if (FF::useDualOpenList) {
                        if ((*qItr)->positionB.second == -1) {
                            delete *qItr;
                        } else {
                            (*qItr)->positionA.second = -1;
                        }
                    } else {                    
                        delete *qItr;                    
                    }
                }
            }
            currMap.clear();

        }
        if (FF::useDualOpenList) {
            for (int pass = 0; pass < 2; ++pass) {
                
                map<double, list<SearchQueueItem*> > & currMap = (pass ? qTwoB : qOneB);            
                
                map<double, list<SearchQueueItem*> >::iterator cmItr = currMap.begin();
                const map<double, list<SearchQueueItem*> >::iterator cmEnd = currMap.end();
                
                for (; cmItr != cmEnd; ++cmItr) {
                    list<SearchQueueItem*>::iterator qItr = cmItr->second.begin();
                    const list<SearchQueueItem*>::iterator qEnd = cmItr->second.end();
                    
                    for (; qItr != qEnd; ++qItr) {                    
                       delete *qItr;                    
                    }
                }
                currMap.clear();                        
            }
        }
	}

	SearchQueueItem* pop_front(const int & listID) {
        
        map<double, list<SearchQueueItem*> > & currOneMap = (listID == 1 ? qOneA : qOneB);
        map<double, list<SearchQueueItem*> > & currTwoMap = (listID == 1 ? qTwoA : qTwoB);
        
        map<double, list<SearchQueueItem*> > & otherOneMap = (listID == 2 ? qOneA : qOneB);
        map<double, list<SearchQueueItem*> > & otherTwoMap = (listID == 2 ? qTwoA : qTwoB);
        
		static int lastTime = 0;
		if (!currOneMap.empty()) {
			if (lastTime != 1) {
				lastTime = 1;
				if (GlobalSchedule::globalVerbosity & 1) {
					cout << "\n1: ";
					cout.flush();
				}
			}
			map<double, list<SearchQueueItem*> >::iterator mItr = currOneMap.begin();
			SearchQueueItem* const toReturn = mItr->second.front();
            if (debug) cout << "Pop from " << listID << " with f value " << mItr->first << " - next state is " << toReturn << endl;
			mItr->second.pop_front();            
			if (mItr->second.empty()) {
                if (debug) cout << "\tNo more states with this f-value left, removing map entry\n";
                currOneMap.erase(mItr);
            } else {
                if (debug) cout << "\tNumber of states with this f-value left; " << mItr->second.size() << endl;
            }
            
            
            
            if (FF::useDualOpenList && seenAnyNonZeroPCosts) {
                double otherH;
                
                if (listID == 1) {
                    otherH = toReturn->heuristicValue.second;
                } else {
                    otherH = toReturn->heuristicValue.first;
                    if (FF::WAStar) {
                        otherH *= FF::doubleU;
                    }
                    otherH += toReturn->plan.size() / 2;                    
                }
                
                pair<list<SearchQueueItem*>::iterator,int> & otherPosn = (listID == 1 ? toReturn->positionB : toReturn->positionA);
                
                if (otherPosn.second >= 1) {
                    if (otherPosn.second & 1) {
                        const map<double, list<SearchQueueItem*> >::iterator oItr = otherOneMap.find(otherPosn.second == 1 ? otherH : 1.0);
                        assert(oItr != otherOneMap.end());
                        if (oItr != otherOneMap.end()) {
                            #ifndef NDEBUG
                            list<SearchQueueItem*>::iterator orItr = oItr->second.begin();
                            const list<SearchQueueItem*>::iterator orEnd = oItr->second.end();
                            for (; orItr != orEnd; ++orItr) {
                                if (*orItr == toReturn) {
                                    assert(orItr == otherPosn.first);
                                    break;
                                }
                            }
                            assert(orItr != orEnd);
                            #endif
                            oItr->second.erase(otherPosn.first);
                            if (oItr->second.empty()) otherOneMap.erase(oItr);
                        }
                    } else if (otherPosn.second & 2) {
                        const map<double, list<SearchQueueItem*> >::iterator oItr = otherTwoMap.find(otherPosn.second == 2 ? otherH : 1.0);
                        assert(oItr != otherTwoMap.end());
                        if (oItr != otherTwoMap.end()) {
                            #ifndef NDEBUG
                            list<SearchQueueItem*>::iterator orItr = oItr->second.begin();
                            const list<SearchQueueItem*>::iterator orEnd = oItr->second.end();
                            for (; orItr != orEnd; ++orItr) {
                                if (*orItr == toReturn) {
                                    assert(orItr == otherPosn.first);
                                    break;
                                }
                            }
                            assert(orItr != orEnd);
                            #endif
                            oItr->second.erase(otherPosn.first);
                            if (oItr->second.empty()) otherTwoMap.erase(oItr);
                        }
                    }
                }
            }
            
			return toReturn;
		} else {
            assert(!currTwoMap.empty());
			if (lastTime != 2) {
				lastTime = 2;
				if (GlobalSchedule::globalVerbosity & 1) {
					cout << "\n2: ";
					cout.flush();
				}
			}
			map<double, list<SearchQueueItem*> >::iterator mItr = currTwoMap.begin();
			SearchQueueItem* const toReturn = mItr->second.front();
			mItr->second.pop_front();
			if (mItr->second.empty()) currTwoMap.erase(mItr);
            
            if (FF::useDualOpenList && seenAnyNonZeroPCosts) {
                double otherH;
                if (listID == 1) {
                    otherH = toReturn->heuristicValue.second;
                } else {
                    otherH = toReturn->heuristicValue.first;
                    if (FF::WAStar) {
                        otherH *= FF::doubleU;
                    }
                    otherH += toReturn->plan.size() / 2;                    
                }

                pair<list<SearchQueueItem*>::iterator,int> & otherPosn = (listID == 1 ? toReturn->positionB : toReturn->positionA);
                
                if (toReturn->positionB.second >= 1) {
                
                    if (toReturn->positionB.second & 1) {
                        const map<double, list<SearchQueueItem*> >::iterator oItr = otherOneMap.find(toReturn->positionB.second == 1 ? otherH : 1.0);
                        if (oItr != otherOneMap.end()) {
                            oItr->second.erase(otherPosn.first);                    
                            if (oItr->second.empty()) otherOneMap.erase(oItr);
                        }
                    } else if (toReturn->positionB.second & 2) {
                        const map<double, list<SearchQueueItem*> >::iterator oItr = otherTwoMap.find(toReturn->positionB.second == 2 ? otherH : 1.0);
                        if (oItr != otherTwoMap.end()) {
                            oItr->second.erase(otherPosn.first);
                            if (oItr->second.empty()) otherTwoMap.erase(oItr);
                        }
                    }
                }
            }
            
			return toReturn;
		}
	}

	SearchQueueItem * back(const int & listID) {
        if (listID == 1) {
            if (qTwoA.empty()) {
                return qOneA.rbegin()->second.back();
            } else {
                return qTwoA.rbegin()->second.back();
            }
        } else {
            assert(FF::useDualOpenList);
            if (qTwoB.empty()) {
                return qOneB.rbegin()->second.back();
            } else {
                return qTwoB.rbegin()->second.back();
            }
        }
	}

	void push_back(SearchQueueItem* p, const int category=1) {
        
        //cout << "Pushing " << p << " to the back of both lists\n";
        
        list<SearchQueueItem*> & q  = (category == 1 ? qOneA[1.0] : qTwoA[1.0]);
        q.push_back(p);
        
        p->positionA = make_pair(q.end(), category + 1024);
        --(p->positionA.first);
        
        if (FF::useDualOpenList) {
            
            if (seenAnyNonZeroPCosts || p->heuristicValue.second > 0.000001) {
            
                list<SearchQueueItem*> & q2 = (category == 1 ? qOneB[1.0] : qTwoB[1.0]);        
                q2.push_back(p);
            
                p->positionB = make_pair(q2.end(), category + 1024);
                --(p->positionB.first);            
                
                seenAnyNonZeroPCosts = true;
            }
        }

	}

	void insert(SearchQueueItem* p, const int category=1) {
        
        debug = (GlobalSchedule::globalVerbosity & 131072);
        
        for (int ilist = 1; ilist < 3; ++ilist) {
            
            if (ilist == 2) {
                if (!FF::useDualOpenList) break;
                if (!seenAnyNonZeroPCosts || p->heuristicValue.second > 0.000001) { 
                    if (debug) cout << "Not seen any non-zero P violations, not using second list for now\n";
                    break;
                }
                seenAnyNonZeroPCosts = true;
            }
            
            const FF::HTrio insHeuristic(p->heuristicValue);                                    
            double prim = (ilist == 1 ? insHeuristic.first : insHeuristic.second);
            if (FF::useWeightedSumWithPrefCost) {
                prim += insHeuristic.second * FF::prefWeightInWeightedSum;
            }
            
            if (FF::WAStar && ilist == 1) {
                prim *= FF::doubleU;
                prim += p->plan.size() / 2;                
            }
            
            if (debug) cout << "Inserting " << p << " into queue " << ilist << ":" << category << " at position " << prim << endl;
            
            list<SearchQueueItem*> & q = (ilist == 1 ? (category == 1 ? qOneA[prim] : qTwoA[prim]) : (category == 1 ? qOneB[prim] : qTwoB[prim])); 
            if (q.empty()) {
                if (debug) {
                    cout << " - No other states on the open list have this f-value\n";
                }
                q.push_back(p);
                if (ilist == 1) {
                    p->positionA = make_pair(q.end(), category);
                    --(p->positionA.first);
                } else {
                    p->positionB = make_pair(q.end(), category);
                    --(p->positionB.first);
                }
            } else {
                const double sec = (ilist == 1 ? insHeuristic.second : insHeuristic.first);
                bool inserted = false;
                list<SearchQueueItem*>::iterator qItr = q.begin();
                const list<SearchQueueItem*>::iterator qEnd = q.end();
                double bh;
                for (; qItr != qEnd; ++qItr) {
                    if (ilist == 1) {
                        bh = (*qItr)->heuristicValue.second;
                    } else {
                        bh = (*qItr)->heuristicValue.first;
                    } 
                
                    if (bh > sec) {
                        const list<SearchQueueItem*>::iterator insItr = q.insert(qItr, p);
                        if (ilist == 1) {
                            p->positionA = make_pair(insItr, category);
                        } else {
                            p->positionB = make_pair(insItr, category);
                        }
                        
                        inserted = true;
                        break;
                    }
                }
                if (!inserted) {
                    q.push_back(p);
                    if (ilist == 1) {
                        p->positionA = make_pair(q.end(), category);
                        --(p->positionA.first);
                    } else {
                        p->positionB = make_pair(q.end(), category);
                        --(p->positionB.first);
                    }
                }
            }
        }
		
	}


	bool empty() const { return (qOneA.empty() && qTwoA.empty() && qOneB.empty() && qTwoB.empty()); };
    
    double newBestPrefScore(const double & bestSolutionPCost)
    {
        //assert(FF::useDualOpenList);
        
        double toReturn = DBL_MAX;
        
        uint keptStates = 0;
        
        for (int pass = 0; pass < 2; ++pass) {

            map<double, list<SearchQueueItem*> > & currMap = (pass ? qTwoA : qOneA);
            
            map<double, list<SearchQueueItem*> >::iterator cmItr = currMap.begin();
            const map<double, list<SearchQueueItem*> >::iterator cmEnd = currMap.end();

            while (cmItr != cmEnd) {
                list<SearchQueueItem*>::iterator qItr = cmItr->second.begin();
                const list<SearchQueueItem*>::iterator qEnd = cmItr->second.end();

                list<SearchQueueItem*>::iterator qDel;
                while(qItr != qEnd) {
                    if ((*qItr)->heuristicValue.second >= bestSolutionPCost) {
                        if ((*qItr)->positionB.second == -1) {
                            delete *qItr;                 
                        } else {
                            (*qItr)->positionA.second = -1;
                        }
                        qDel = qItr++;
                        cmItr->second.erase(qDel);
                    } else {
                        if ((*qItr)->heuristicValue.first < toReturn) {
                            toReturn = (*qItr)->heuristicValue.first;
                        }
                        
                        if ((*qItr)->positionB.second == -1) {
                            ++keptStates;
                        }
                        ++qItr;
                    }
                }
                
                if (cmItr->second.empty()) {
                    const map<double, list<SearchQueueItem*> >::iterator cmDel = cmItr++;
                    currMap.erase(cmDel);
                } else {
                    ++cmItr;
                }
            }

        }
        
        if (FF::useDualOpenList) {
            for (int pass = 0; pass < 2; ++pass) {
                
                map<double, list<SearchQueueItem*> > & currMap = (pass ? qTwoB : qOneB);            
                
                map<double, list<SearchQueueItem*> >::iterator cmItr = currMap.begin();
                const map<double, list<SearchQueueItem*> >::iterator cmEnd = currMap.end();
                
                while (cmItr != cmEnd) {
                    list<SearchQueueItem*>::iterator qItr = cmItr->second.begin();
                    const list<SearchQueueItem*>::iterator qEnd = cmItr->second.end();
                    
                    list<SearchQueueItem*>::iterator qDel;
                    
                    while (qItr != qEnd) {
                        if ((*qItr)->heuristicValue.second >= bestSolutionPCost) {
                            assert((*qItr)->positionA.second == -1);
                            delete *qItr;                    
                            qDel = qItr++;
                            cmItr->second.erase(qDel);
                        } else {
                            if ((*qItr)->heuristicValue.first < toReturn) {
                                toReturn = (*qItr)->heuristicValue.first;
                            }
                            ++qItr;
                            ++keptStates;
                        }
                    }
                    
                    if (cmItr->second.empty()) {
                        const map<double, list<SearchQueueItem*> >::iterator cmDel = cmItr++;
                        currMap.erase(cmDel);
                    } else {
                        ++cmItr;
                    }
                                                
                }
                

            }
        }        
        cout << "\n -> Kept " << keptStates << " states\n";
        
        return toReturn;
    }

};


#ifdef FFSEARCHDEBUGHOOKS

void resetKnownGoodFlags()
{
    map<ExtendedMinimalState, list<StepInfo>, CompareStatesZealously>::iterator kgItr = knownGoodStates.begin();
    const map<ExtendedMinimalState, list<StepInfo>, CompareStatesZealously>::iterator kgEnd = knownGoodStates.end();
    
    for (; kgItr != kgEnd; ++kgItr) {
        list<StepInfo>::iterator siItr = kgItr->second.begin();
        const list<StepInfo>::iterator siEnd = kgItr->second.end();
        
        for (; siItr != siEnd; ++siItr) {
            siItr->seenInSearch = false;
        }        
    }
}

void keepingState(SearchQueueItem * const succ)
{
    map<ExtendedMinimalState, list<StepInfo>, CompareStatesZealously>::iterator kgItr = knownGoodStates.find(succ->state);
    if (kgItr == knownGoodStates.end()) return;
    
    list<StepInfo>::iterator siItr = kgItr->second.begin();
    const list<StepInfo>::iterator siEnd = kgItr->second.end();
    
    for (; siItr != siEnd; ++siItr) {
        if (siItr->preferenceStatus == succ->state.preferenceStatus) {
            cout << "\n- Have encountered the step reached after step " << siItr->stepID << " in the given plan\n";
            siItr->seenInSearch = true;
            return;
        }           
    }
}

void checkWhetherSuccessorReallyShouldBePruned(SearchQueueItem * const succ, const bool & betterBeLastStep)
{
    map<ExtendedMinimalState, list<StepInfo>, CompareStatesZealously>::iterator kgItr = knownGoodStates.find(succ->state);
    if (kgItr == knownGoodStates.end()) return;

    list<StepInfo>::iterator siItr = kgItr->second.begin();
    const list<StepInfo>::iterator siEnd = kgItr->second.end();
    
    for (; siItr != siEnd; ++siItr) {
        if (siItr->preferenceStatus != succ->state.preferenceStatus) continue;        
        if (siItr->seenInSearch) return;
        if (betterBeLastStep && siItr->lastStepInPlan) return;
        
        cout << "\n\n!! Error detected: want to prune the state reached after step " << siItr->stepID << " of the plan\n";
        if (siItr->reachedBy) {
            cout << "i.e. by the action " << *(siItr->reachedBy) << "\n";            
        }
        exit(1);
    }
    
}
#endif


void checkreallyIsAGoal(ExtendedMinimalState & theState, set<int> & goals, set<int> & goalFluents)
{
    const set<int>::iterator gsEnd = goals.end();
    const set<int>::iterator gfEnd = goalFluents.end();
    
    {
        set<int>::iterator gsItr = goals.begin();
        
        for (; gsItr != gsEnd; ++gsItr) {
            assert(theState.first.find(*gsItr) != theState.first.end());
            cout << "-- Goal " << (RPGBuilder::getLiteral(*gsItr)) << " is satisfied\n";
        }
    }
                
    const int vCount = RPGBuilder::getPNECount();
    const int avCount = RPGBuilder::getAVCount();
    
    vector<double> maxFluentTable(vCount * 2 + avCount);
    
    {
        const vector<double> & oldVec = theState.second;
        for (int i = 0; i < vCount; ++i) {
            const double ov = oldVec[i];
            maxFluentTable[i] = ov;
            if (ov != 0.0) {
                maxFluentTable[i + vCount] = 0.0 - ov;
            } else {
                maxFluentTable[i + vCount] = 0.0;
            }
        }
    }
                
                        
    {
        const int startLim = vCount * 2;
        const int endLim = startLim + avCount;
        for (int i = startLim; i < endLim; ++i) {
            maxFluentTable[i] = RPGBuilder::getArtificialVariable(i).evaluate(maxFluentTable);
        }
    }
                                                
                                                                                    
                                                                                            
    {
                        
        set<int>::iterator gfItr = goalFluents.begin();
        
        for (; gfItr != gfEnd; ++gfItr) {
            assert(RPGBuilder::getNumericPrecs()[*gfItr].isSatisfied(maxFluentTable));
            cout << "-- Numeric goal " << RPGBuilder::getNumericPrecs()[*gfItr] << " is satisfied\n";
        }
                                                                                                                                                            
    }
                                                                                                                        

}

FF::HTrio FF::evaluateStateWRTSchedule(SubproblemRPG* const rpg, ExtendedMinimalState & theState, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActions, list<pair<double, double> > & howMany, list<ScheduleNode*> & pointsOfConflict,list<FFEvent> & header, list<FFEvent> & now, FF::HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnant, bool considerCache, map<double, list<int> > * justApplied, double tilFrom) {

	list<pair<double, list<ActionAndHowManyTimes> > > relaxedPlan;

	//printState(theState);
	//static int oldBestH = INT_MAX;
	
	const SubproblemRPG::EvaluationInfo h(rpg->getRelaxedPlan(theState, bestSolutionPCost, theState.timeStamp, theState.nextTIL,  goals, goalFluents, helpfulActions, relaxedPlan, justApplied, tilFrom));
    {
		list<pair<int, VAL::time_spec> >::iterator haItr = helpfulActions.begin();
		const list<pair<int, VAL::time_spec> >::iterator haEnd = helpfulActions.end();
		for (; haItr != haEnd; ++haItr) {
			map<int, pair<double,double> >::iterator hmItr = rpg->howManyTimesIsHelpful.find(haItr->first);
			if (hmItr != rpg->howManyTimesIsHelpful.end()) {
				howMany.push_back(hmItr->second);
			} else {
				howMany.push_back(pair<double,double>(1.0,1.0));
			}
		}
		
	}

    
	return HTrio(h.first,h.second,h.first,h.goalState);

}

FF::HTrio FF::evaluateStateWRTScheduleLocally(list<TimelinePoint*>::iterator startPoint, SubproblemRPG* const rpg, ExtendedMinimalState & theState, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActions, list<ScheduleNode*> & pointsOfConflict, list<TimelinePoint*>::iterator tlPoint, list<FFEvent> & header, list<FFEvent> & now, FF::HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnant, bool considerCache, double & localPenalty, map<double, list<int> > * justApplied, double tilFrom) {

    exit(1);    
    return HTrio(-1.0,0.0,-1.0);
	
}

FF::HTrio FF::evaluateStateWRTScheduleLocally(list<TimelinePoint*>::iterator startPoint, SubproblemRPG* const rpg, ExtendedMinimalState & theState, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActions, list<ScheduleNode*> & pointsOfConflict, list<TimelinePoint*>::iterator tlPoint, list<FFEvent> & header, list<FFEvent> & now, ScheduleNode* const waitFor, const bool & comesAfter, FF::HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnant, bool considerCache, double & localPenalty, map<double, list<int> > * justApplied, double tilFrom) {

    exit(1);    
    return HTrio(-1.0,0.0,-1.0);
    
}

FF::HTrio FF::evaluateOldPlan(list<TimelinePoint*>::iterator startPoint, SubproblemRPG* const rpg, list<ScheduleNode*> & pointsOfConflict, list<TimelinePoint*>::iterator tlPoint, list<FFEvent> * header, FF::HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID) {

    exit(1);    
    return HTrio(-1.0,0.0,-1.0);
    
}

void FF::applyActionToState(SubproblemRPG* const rpg, ExtendedMinimalState & theState, const int & actionToApply, const VAL::time_spec & ts) {

	const bool localDebug = false;

	if (localDebug) {
		cout << "Applying action " << actionToApply << " to state:\n";
		printState(theState);
	}

	assert(!RPGBuilder::rogueActions[actionToApply]);
	assert(rpg->testApplicability(theState, theState.nextTIL, pair<int, VAL::time_spec>(actionToApply, ts), true));

    assert(ts == VAL::E_AT_START);
    
    PreferenceHandler::aboutToApply(theState, actionToApply);
    

    {
        list<Literal*> & delEffs = rpg->getDeleteEffects(actionToApply, ts);

        list<Literal*>::iterator effItr = delEffs.begin();
        const list<Literal*>::iterator effEnd = delEffs.end();
    
        for (; effItr != effEnd; ++effItr) {
            theState.first.erase((*effItr)->getStateID());
        }
    }
	
    {
        list<Literal*> & addEffs = rpg->getAddEffects(actionToApply, ts);

        list<Literal*>::iterator effItr = addEffs.begin();
        const list<Literal*>::iterator effEnd = addEffs.end();
    
        int fID;
        
        for (; effItr != effEnd; ++effItr) {
            fID = (*effItr)->getStateID();
            LandmarksAnalysis::addFactToState(&theState, fID);
            theState.first.insert(fID);
        }
        
    }
	
    {
        list<RPGBuilder::NumericEffect> & numEffs = rpg->getNumericEffects(actionToApply, ts);
        
        list<RPGBuilder::NumericEffect>::iterator effItr = numEffs.begin();
        const list<RPGBuilder::NumericEffect>::iterator effEnd = numEffs.end();

        list<pair<int, double> > updated;

        for (; effItr != effEnd; ++effItr) {
            updated.push_back(pair<int, double>(effItr->fluentIndex, effItr->applyEffect(theState.second)));
        }

        list<pair<int, double> >::iterator updItr = updated.begin();
        const list<pair<int, double> >::iterator updEnd = updated.end();

        for (; updItr != updEnd; ++updItr) {
            theState.second[updItr->first] = updItr->second;
        }
                
    }
	
    ++(theState.planLength);
	
    PreferenceHandler::update(theState);

	
}



void FF::chooseWhetherToReuseAndWait(auto_ptr<SearchQueueItem> & succ, SubproblemRPG* rpg, ExtendedMinimalState & state, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActionsExport, list<pair<double, double> > & howManyExport, list<ScheduleNode*> & pointsOfConflictExport, const pair<instantiatedOp*, VAL::time_spec> & actID, const int & howManyInTotal, list<FFEvent> & header, FF::HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnantExport)
{

	list<pair<int, VAL::time_spec> > helpfulActions;
	list<pair<double, double> > howMany;
	
	FFEvent afterWaitEvent;
	FFEvent extraEvent;
	FFEvent beforeWaitEvent;

//	pair<int, ScheduleNode*> newCEN(0, 0);
	

	map<double, list<int> > * justApplied = 0;
	map<double, list<int> > actualJustApplied;
	double tilFrom = 0.001;
	
	assert(actID.second == VAL::E_AT_START);

    extraEvent = FFEvent(actID.first, 1.0, 1.0);

	FFEvent forHeaderLaterOn(extraEvent);

	list<ScheduleNode*> pointsOfConflict;

	bool stagnant = false;

	FFcache_upToDate = false;

	list<FFEvent> nowList;
	nowList.push_back(forHeaderLaterOn);
	
	HTrio h1(evaluateStateWRTSchedule(rpg, state, goals, goalFluents, helpfulActions, howMany, pointsOfConflict, header, nowList, bestNodeLimitHeuristic, bestNodeLimitPlan, bestNodeLimitGoal, spID, stagnant, true, justApplied, tilFrom));
	
	HTrio hcurr(h1);


	helpfulActionsExport = helpfulActions;
	howManyExport = howMany;
	succ->heuristicValue = hcurr;
	succ->plan = header;

    for (int h = 0; h < howManyInTotal; ++h) {
		succ->plan.push_back(extraEvent);	
	}

	pointsOfConflictExport = pointsOfConflict;
	stagnantExport = stagnant;
	
}

void FF::justEvaluateNotReuse(auto_ptr<SearchQueueItem> & succ, SubproblemRPG* rpg, ExtendedMinimalState & state, set<int> & goals, set<int> & goalFluents, list<pair<int, VAL::time_spec> > & helpfulActionsExport, list<pair<double,double> > & howManyExport, list<ScheduleNode*> & pointsOfConflictExport, list<FFEvent> & nowList, list<FFEvent> & header, FF::HTrio & bestNodeLimitHeuristic, list<FFEvent> *& bestNodeLimitPlan, bool & bestNodeLimitGoal, const int & spID, bool & stagnant, map<double, list<int> > * justApplied, double tilFrom) {

	list<pair<int, VAL::time_spec> > helpfulActions;
	list<pair<double, double> > howMany;
	list<ScheduleNode*> poc;

	HTrio h1(evaluateStateWRTSchedule(rpg, state, goals, goalFluents, helpfulActions, howMany, poc, header, nowList, bestNodeLimitHeuristic, bestNodeLimitPlan, bestNodeLimitGoal, spID, stagnant, true, justApplied, tilFrom));
	
	HTrio hcurr(h1);
	
	helpfulActionsExport = helpfulActions;
	howManyExport = howMany;
	succ->heuristicValue = hcurr;
	succ->plan = header;
	pointsOfConflictExport = poc;
	
}

/*
void reorderNoDeletesFirst(list<pair<int, VAL::time_spec> > & applicableActions) {
    
    list<pair<int, VAL::time_spec> > tmp;
    tmp.swap(applicableActions);
    
    list<pair<int, VAL::time_spec> >::iterator itr = tmp.begin();
    const list<pair<int, VAL::time_spec> >::iterator itrEnd = tmp.end();
    
    list<pair<int, VAL::time_spec> >::iterator firstEnd = applicableActions.end();
    
    for (; itr != itrEnd; ++itr) {
        if (RPGBuilder::getStartDeleteEffects()[itr->first].empty()) {
            if (firstEnd == applicableActions.end()) {
                applicableActions.push_back(*itr);
            } else {
                applicableActions.insert(firstEnd, *itr);
            }
        } else {
            applicableActions.push_back(*itr);
            if (firstEnd == applicableActions.end()) {
                --firstEnd;
            }
        }
    }
        
            
};*/

typedef pair<int, VAL::time_spec> ActionSegment;

void reorderNoDeletesFirst(list<ActionSegment > & applicableActions, list<pair<double,double> > * aWeights)
{

    if (false) {
        cout << "Before, ordered:\n";
        list<ActionSegment>::iterator postItr = applicableActions.begin();
        const list<ActionSegment>::iterator postEnd = applicableActions.end();
        
        for (; postItr != postEnd; ++postItr) {
            if (postItr->second != VAL::E_AT) {
                cout << "\t" << *(RPGBuilder::getInstantiatedOp(postItr->first));
                if (postItr->second == VAL::E_AT_START) {
                    cout << " start";
                } else {
                    cout << " end";
                }
                cout << "\n";
            }
        }
    }
    
    list<ActionSegment > tmp;
    tmp.swap(applicableActions);

    list<pair<double,double> > tWeights;
    
    if (aWeights) {
        tWeights.swap(*aWeights);
    }

    list<set<int> > preconditionFacts;
    
    {
        
        list<ActionSegment >::iterator itr = tmp.begin();
        const list<ActionSegment >::iterator itrEnd = tmp.end();
     
        for (; itr != itrEnd; ++itr) {
            preconditionFacts.push_back(set<int>());
            
            if (itr->second == VAL::E_AT) continue;
            
            set<int> & toFill = preconditionFacts.back();
            
            const list<Literal*> & pres = RPGBuilder::getProcessedStartPreconditions()[itr->first];
                                           
            list<Literal*>::const_iterator pItr = pres.begin();
            const list<Literal*>::const_iterator pEnd = pres.end();
            
            for (; pItr != pEnd; ++pItr) {
                toFill.insert((*pItr)->getStateID());
            }
        }
    }
    
    list<int> penalties;
    
    list<set<int> >::const_iterator ownPFItr = preconditionFacts.begin();
    list<ActionSegment >::iterator itr = tmp.begin();
    const list<ActionSegment >::iterator itrEnd = tmp.end();
    
    list<pair<double,double> >::const_iterator wItr = tWeights.begin();

    for (; itr != itrEnd; ++itr, ++ownPFItr) {
        int penaltyScore = 0;
        if (itr->second != VAL::E_AT) {
            set<int> toFill;
            
            const list<Literal*> & effs = RPGBuilder::getStartDeleteEffects()[itr->first];
                                            
            list<Literal*>::const_iterator eItr = effs.begin();
            const list<Literal*>::const_iterator eEnd = effs.end();

            for (; eItr != eEnd; ++eItr) {
                toFill.insert((*eItr)->getStateID());
            }
            
            list<set<int> >::const_iterator pfItr = preconditionFacts.begin();
            const list<set<int> >::const_iterator pfEnd = preconditionFacts.end();
            
            for (; pfItr != pfEnd; ++pfItr) {
                if (pfItr == ownPFItr) {
                    continue;
                }
                set<int> overlap;
                
                std::set_intersection(pfItr->begin(), pfItr->end(), toFill.begin(), toFill.end(),
                                      insert_iterator<set<int> >(overlap, overlap.begin()));
                                      
                if (!overlap.empty()) {
                    ++penaltyScore;
                }
            }
        }
        
        list<int>::iterator penItr = penalties.begin();
        list<ActionSegment>::iterator actItr = applicableActions.begin();
        list<pair<double,double> >::iterator outputWItr;
        
        if (aWeights) {
            outputWItr = aWeights->begin();
        }
        
        for (; actItr != applicableActions.end() && penaltyScore >= *penItr; ++actItr, ++penItr) {
            if (aWeights) {
                ++outputWItr;
            }
        }
        
        penalties.insert(penItr, penaltyScore);
        applicableActions.insert(actItr, *itr);
        
        if (aWeights) {
            aWeights->insert(outputWItr, *wItr);
        }
        
        if (aWeights) {
            ++wItr;
        }
    }
    
    if (false) {
        cout << "After, ordered:\n";
        list<ActionSegment>::iterator postItr = applicableActions.begin();
        const list<ActionSegment>::iterator postEnd = applicableActions.end();

        for (; postItr != postEnd; ++postItr) {
            if (postItr->second != VAL::E_AT) {
                cout << "\t" << *(RPGBuilder::getInstantiatedOp(postItr->first));
                if (postItr->second == VAL::E_AT_START) {
                    cout << " start";
                } else {
                    cout << " end";
                }
                cout << "\n";
            }
        }
    }
};

void reorderStartsBeforeEnds(list<pair<int, VAL::time_spec> > & applicableActions) {

	list<pair<int, VAL::time_spec> > tmp;
    tmp.swap(applicableActions);
	
	list<pair<int, VAL::time_spec> >::iterator itr = tmp.begin();
	const list<pair<int, VAL::time_spec> >::iterator itrEnd = tmp.end();
	
	list<pair<int, VAL::time_spec> >::iterator firstEnd = applicableActions.end();
	
	const VAL::time_spec toMatch = (FF::startsBeforeEnds ? VAL::E_AT_START : VAL::E_AT_END);

	for (; itr != itrEnd; ++itr) {
		if (itr->second == toMatch) {
			if (firstEnd == applicableActions.end()) {
				applicableActions.push_back(*itr);
			} else {
				applicableActions.insert(firstEnd, *itr);
			}
		} else {
			applicableActions.push_back(*itr);
			if (firstEnd == applicableActions.end()) {
				--firstEnd;
			}
		}
	}
	
	
};

void FF::reorderByPrefCost(SubproblemRPG* const rpg, SearchQueueItem * const currSQI, list<pair<int, VAL::time_spec> > & applicableActions, list<pair<double,double> > & aWeights) {

    static const bool debug = false;
    list<pair<int, VAL::time_spec> > tmp;
    list<pair<double,double> > tmpW;
    list<double> prefCosts;
    tmp.swap(applicableActions);
    tmpW.swap(aWeights);
    
    assert(tmp.size() == tmpW.size());
    
    const double oldCost = PreferenceHandler::getG(currSQI->state);
    
    list<pair<int, VAL::time_spec> >::iterator itr = tmp.begin();
    list<pair<double,double> >::iterator wItr = tmpW.begin();
    const list<pair<int, VAL::time_spec> >::iterator itrEnd = tmp.end();
    
    for (; itr != itrEnd; ++itr) {
        
        if (debug) {
            cout << "Considering the impact of " << *(RPGBuilder::getInstantiatedOp(itr->first)) << " on preference costs\n";
        }
        double hmLimit = wItr->first;
        if (hmLimit < 0.1) hmLimit = 0.1;
        
        bool everApplicable = true;
        
        auto_ptr<SearchQueueItem> succ(new SearchQueueItem(currSQI->state));        
        
        int appTimes = 0;
        for (double hm = 0; hm < hmLimit; hm += 1.0) {
            
            if (!rpg->testApplicability(succ->state, succ->state.nextTIL, (*itr), false,false)) {
                if (hm == 0) everApplicable = false;
                break;
            }

            applyActionToState(rpg, succ->state, itr->first, itr->second);
            ++appTimes;
        }
        
        if (!everApplicable) {
            if (debug) cout << " -- Was never actually applicable\n";
            continue;
        } else {
            if (debug) cout << " -- Applied " << appTimes << " times\n";
        }
    
        const double prefCost = PreferenceHandler::getG(succ->state) - oldCost;
        
        if (debug) cout << " -- cost is " << prefCost << endl;
        
        if (prefCosts.empty()) {
            applicableActions.push_back(*itr);
            aWeights.push_back(*wItr);
            prefCosts.push_back(prefCost);
        } else {
            list<pair<int, VAL::time_spec> >::iterator aaItr = applicableActions.begin();
            list<pair<double,double> >::iterator awItr = aWeights.begin();
            list<double>::iterator pcItr = prefCosts.begin();
            
            for (; pcItr != prefCosts.end() && prefCost >= *pcItr; ++pcItr, ++aaItr, ++awItr) ;
            
            applicableActions.insert(aaItr, *itr);
            aWeights.insert(awItr, *wItr);
            prefCosts.insert(pcItr, prefCost);
        }
    }
        
    assert(applicableActions.size() == aWeights.size());        
    
    if (debug) {
        cout << "Final HA order:\n";
        list<pair<int, VAL::time_spec> >::iterator aaItr = applicableActions.begin();
        list<pair<double,double> >::iterator awItr = aWeights.begin();
        list<double>::iterator pcItr = prefCosts.begin();
        
        for (; aaItr != applicableActions.end(); ++aaItr, ++awItr, ++pcItr) {
            cout << *(RPGBuilder::getInstantiatedOp(aaItr->first)) << "x" << awItr->first << ", cost " << *pcItr << endl;
        }
    }
};


void reorderHelpfulFirst(list<pair<int, VAL::time_spec> > & applicableActions, list<pair<int, VAL::time_spec> > & helpfulActions) {

	list<pair<int, VAL::time_spec> > tmp;
    tmp.swap(applicableActions);	

	list<int> weights;

	const list<pair<int, VAL::time_spec> >::iterator hitrStart = helpfulActions.begin();
	const list<pair<int, VAL::time_spec> >::iterator hitrEnd = helpfulActions.end();
	
	list<pair<int, VAL::time_spec> >::iterator itr = tmp.begin();
	const list<pair<int, VAL::time_spec> >::iterator itrEnd = tmp.end();
	
	for (; itr != itrEnd; ++itr) {
		int w = 0;
		for (list<pair<int, VAL::time_spec> >::iterator hitr = hitrStart; hitr != hitrEnd; ++hitr, ++w) {
			if (itr->first == hitr->first && itr->second == hitr->second) {
				break;
			}
		}

		list<pair<int, VAL::time_spec> >::iterator aaItr = applicableActions.begin();
		const list<pair<int, VAL::time_spec> >::iterator aaEnd = applicableActions.end();
		list<int>::iterator wItr = weights.begin();

		for (; aaItr != aaEnd && (w >= *wItr); ++aaItr, ++wItr);
	
		applicableActions.insert(aaItr, *itr);
		weights.insert(wItr, w);
		
	}
	
	
};


void FF::helpfulHelper(list<pair<int, VAL::time_spec> >::iterator helpfulActsItr,
                        const list<pair<int, VAL::time_spec> >::iterator helpfulActsEnd,
                        list<pair<double,double> >::iterator hhmItr,
                        SearchQueueItem * const currSQI,
                        SubProblem * const toSolve,
                        SearchQueueItem *& succOneReturn,
                        SearchQueueItem *& succTwoReturn
                      )
{

    static set<int> backupFacts;
    static vector<double> backupFluents;
    static vector<AutomatonPosition> backupPreferenceStatus;
    static vector<bool> backupLandmarkStatus;
    
    auto_ptr<SearchQueueItem> succOne(new SearchQueueItem(currSQI->state));
    auto_ptr<SearchQueueItem> succTwo((SearchQueueItem*)0);
    

    list<FFEvent> backupPlanOne = currSQI->plan;
    list<FFEvent> backupPlanTwo;

    

    int howManyInTotalOrig = 0;
    int howManyInTotalCopy = 0;

    const double oldGCost = PreferenceHandler::getG(succOne->state);
    bool bcopy = FF::useDualOpenList;
    
    if (bcopy) {
        backupPlanTwo = currSQI->plan;
        succTwo = auto_ptr<SearchQueueItem>(new SearchQueueItem(currSQI->state));
    }
    
    for (; helpfulActsItr != helpfulActsEnd; ++helpfulActsItr, ++hhmItr) {

        if (hhmItr->first == 0.0) hhmItr->first = 0.1;

        for (double hm = 0; hm < hhmItr->first; hm += 1.0) {

            
            if (bestSolutionPCost != DBL_MAX) {
                backupFacts = succOne->state.first;
                backupFluents = succOne->state.second;
                backupPreferenceStatus = succOne->state.preferenceStatus;
                backupLandmarkStatus = succOne->state.landmarkStatus;
            }
            
            
            
            if (!toSolve->rpg->testApplicability(succOne->state, succOne->state.nextTIL, (*helpfulActsItr), false,false)) break;

            

            
            applyActionToState(toSolve->rpg, succOne->state, helpfulActsItr->first, helpfulActsItr->second);
    
            bool keepIt = true;
            
            if (bcopy || bestSolutionPCost != DBL_MAX) {
                const double newG = PreferenceHandler::getG(succOne->state);
                if (bcopy && newG > oldGCost) {
                    bcopy = false;
                }
                if (bestSolutionPCost != DBL_MAX && newG >= bestSolutionPCost) {
                    succOne->state.first.swap(backupFacts);
                    succOne->state.second.swap(backupFluents);
                    succOne->state.preferenceStatus.swap(backupPreferenceStatus);
                    succOne->state.landmarkStatus.swap(backupLandmarkStatus);                    
                    --(succOne->state.planLength);                  
                    keepIt = false;
                }
            }
                
            if (!keepIt) break;
            
            if (bcopy) {
                applyActionToState(toSolve->rpg, succTwo->state, helpfulActsItr->first, helpfulActsItr->second);
            }
            
            

            for (int sc = 0; sc < (bcopy ? 2 : 1); ++sc) {
                
                FFEvent extraEvent(toSolve->rpg->getOp(helpfulActsItr->first), 1.0, 1.0);

                list<FFEvent> & backupPlan = (sc ? backupPlanTwo : backupPlanOne);
                
                backupPlan.push_back(extraEvent);

                
                if (sc) {
                    ++howManyInTotalCopy;
                } else {
                    ++howManyInTotalOrig;
                }
            }
            
        }

    }

    if (howManyInTotalCopy == howManyInTotalOrig) {
        howManyInTotalCopy = 0;
    }

    if (howManyInTotalCopy) {
        succTwoReturn = succTwo.release();        
        succTwoReturn->plan = backupPlanTwo;
    } else {
        succTwoReturn = 0;
    }
    
    if (howManyInTotalOrig) {
        succOneReturn = succOne.release();
        succOneReturn->plan = backupPlanOne;
    } else {
        succOneReturn = 0;
    }

    if (GlobalSchedule::globalVerbosity & 1) {
        cout << "(" << howManyInTotalOrig << ")(" << howManyInTotalCopy << ")"; cout.flush();
    }
}



int FF::testExistingSolution(const list<int> & soln, const bool & primeKnownStates)
{

    #ifndef FFSEARCHDEBUGHOOKS
    assert(!primeKnownStates);
    #endif
    
    SubProblem * const toSolve = Decomposition::getSubproblem(0);
    LiteralSet startingState;
    vector<double> startingFluents;
    
    RPGBuilder::getInitialState(startingState, startingFluents);
    
    
    static bool initCSBase = false;
    
    if (!initCSBase) {
        initCSBase = true;
        vector<dominance_constraint> & dcs = RPGBuilder::getDominanceConstraints();
        const int pneCount = dcs.size();
        CSBase::ignorableFluents = vector<bool>(pneCount);
        for (int i = 0; i < pneCount; ++i) {
            CSBase::ignorableFluents[i] = (dcs[i] == E_METRIC || dcs[i] == E_IRRELEVANT);
        }
    }
    
    FFheader_upToDate = false;
    FFonly_one_successor = false;
    WAStar = false;
    set<int> goals;
    set<int> numericGoals;
    ExtendedMinimalState initialState;

    {
        list<Literal*>::iterator gsItr = toSolve->goals.begin();
        const list<Literal*>::iterator gsEnd = toSolve->goals.end();

        for (; gsItr != gsEnd; ++gsItr) {
            goals.insert((*gsItr)->getStateID());
        }

    }
    {
        list<int>::iterator gsItr = toSolve->goalFluents.begin();
        const list<int>::iterator gsEnd = toSolve->goalFluents.end();
        
        for (; gsItr != gsEnd; ++gsItr) {
            numericGoals.insert(*gsItr);
        }
    }
    {
        set<int> & isLocal = initialState.first;
        LiteralSet::iterator isItr = startingState.begin();
        const LiteralSet::iterator isEnd = startingState.end();

        for (int fID; isItr != isEnd; ++isItr) {
            fID = (*isItr)->getStateID();
            isLocal.insert(fID);
        }
    }

    {
        initialState.second = startingFluents;
        initialState.preferenceStatus = PreferenceHandler::getInitialAutomataPositions();
    }

    {
        initialState.landmarkStatus.resize(LandmarksAnalysis::getFactsInAnyLandmark().size(),false);
        initialState.prefPreconditionViolations = 0.0;
        initialState.cost = 0.0;
    }

    // used to map to list<pair<pair<HTrio, bool>, double > >
    map<ExtendedMinimalState, list<vector<AutomatonPosition> > , CompareStatesZealously> zealousVisitedStates;
    
    list<pair<ExtendedMinimalState,int> > statesSeen;
    
    statesSeen.push_back(make_pair(initialState, -1));
    
    ExtendedMinimalState currState = initialState;

    list<pair<int, VAL::time_spec> > helpfulActions;
    
    bool roomforimprovement = true;
    
    bestSolutionPCost = capOnPreferenceCost;
    
    PreferenceHandler::recordPreconditionViolations = true;
    
    {


        list<pair<double, list<ActionAndHowManyTimes > > > relaxedPlan;
        
        const SubproblemRPG::EvaluationInfo h(toSolve->rpg->getRelaxedPlan(currState, bestSolutionPCost, currState.timeStamp,
                                                                           currState.nextTIL,  goals, numericGoals, helpfulActions,
                                                                           relaxedPlan, 0, 0.001));        

                                                                           
        if (h.first == -1) {
            cout << "Planner claims that the initial state is a dead end\n";
            return 1;
        }
        
        cout << "Heuristic value of the initial state: " << h.first << endl;

        HTrio t(h.first,h.second,h.first,h.goalState);

        t.second += PreferenceHandler::getG(initialState);
        
        if (t.goalsSatisfied) {
            const double pcost = PreferenceHandler::getCurrentCost(initialState);
            
            cout << endl;
            cout << "; New best\n";
            cout << "; violation cost = " << pcost + RPGBuilder::getPermanentViolationCost() << " " << endl;
            cout << "; Violations: " << PreferenceHandler::getCurrentViolations(initialState) << endl;
            cout << ";\n\n\n";
            
            if (pcost < bestSolutionPCost) {
                cout << COLOUR_yellow << "Initial state is the new best goal state: cost " << pcost << COLOUR_default << endl;
                bestSolutionPCost = pcost;


            }
            roomforimprovement = (t.second < bestSolutionPCost);
                                                                
        }
        
        list<vector<AutomatonPosition> > tList;
        tList.push_back(initialState.preferenceStatus);
                
        if (!zealousVisitedStates.insert(make_pair(initialState, tList)).second) {
            cout << "Planner claims the initial state has been seen before\n";
            return 1;
        }

        #ifdef FFSEARCHDEBUGHOOKS
        if (primeKnownStates) {            
            knownGoodStates[initialState].push_back(StepInfo(-1,0,initialState.preferenceStatus,t,soln.empty()));
        }
        #endif
    }

    
    list<int>::const_iterator stepItr = soln.begin();
    const list<int>::const_iterator stepEnd = soln.end();
    
    for (unsigned int i = 0; stepItr != stepEnd; ++stepItr, ++i) {
        
        if (!roomforimprovement) {
            cout << "Planner would have stopped before applying step " << i << ", claiming there was no way to improve the costs of the preceding state\n";
            return 2;
        }
        cout << COLOUR_light_blue << "Applying step " << i << ": " << *(RPGBuilder::getInstantiatedOp(*stepItr)) << COLOUR_default << endl;
        applyActionToState(toSolve->rpg, currState, *stepItr, VAL::E_AT_START);

        {
            const vector<bool> & posns = currState.landmarkStatus;
            
            const vector<int> & indiv = LandmarksAnalysis::getIndividualLandmarkFacts();
            
            const int iCount = indiv.size();
            
            for (int i = 0; i < iCount; ++i) {
                if (!posns[i]) {
                    cout << "Invidual landmark fact " << *(RPGBuilder::getLiteral(indiv[i])) << " not yet seen\n";
                }
            }
            const vector<vector<vector<int> > > & dis = LandmarksAnalysis::getDisjunctiveLandmarks();
            
            const int dCount = dis.size();
            
            for (int d = 0; d < dCount; ++d) {
                const vector<vector<int> > & possibilities = dis[d];
                
                int pCount = possibilities.size();
                
                bool oneWasTrue = false;
                for (int p = 0; !oneWasTrue && p < pCount; ++p) {
                    const int gCount = possibilities[p].size();                
                    oneWasTrue = true;
                    for (int g = 0; g < gCount; ++g) {
                        if (LandmarksAnalysis::factHasBeenSeen(possibilities[p][g], &currState) != 1) {
                            oneWasTrue = false;
                            break;
                        }
                    }
                    if (oneWasTrue) {
                        cout << "Disjunctive landmark group " << d << " was satisfied with facts";
                        for (int g = 0; g < gCount; ++g) {
                            cout << " " << *(RPGBuilder::getLiteral(possibilities[p][g]));
                        }
                        cout << endl;
                    }
                }
                if (oneWasTrue) {
                    continue;
                }
                cout << "Disjunctive landmark group " << d << " was never satisfied:\n";
                
                for (int p = 0; !oneWasTrue && p < pCount; ++p) {
                    const int gCount = possibilities[p].size();                
                    for (int g = 0; g < gCount; ++g) {
                        cout << "\tGroup " << p << " entry " << g << ", " << *(RPGBuilder::getLiteral(possibilities[p][g]));
                        if (LandmarksAnalysis::factHasBeenSeen(possibilities[p][g], &currState) != 1) {
                            cout << ", was never seen\n";
                        } else {
                            cout << ", was seen\n";
                        }
                    }
                }
                
            }
            
            
        }
        
        list<pair<int, VAL::time_spec> >::const_iterator hItr = helpfulActions.begin();
        const list<pair<int, VAL::time_spec> >::const_iterator hEnd = helpfulActions.end();
        
        for (; hItr != hEnd; ++hItr) {
            if (hItr->first == *stepItr) break;
        }
        
        if (hItr == hEnd) {
            cout << COLOUR_light_red << "  Warning: this action is not a helpful action\n" << COLOUR_default;
        }
        
        helpfulActions.clear();
        
        list<pair<double, list<ActionAndHowManyTimes > > > relaxedPlan;
        
        cout << COLOUR_light_magenta << "* Evaluating state\n" << COLOUR_default;
        
        const SubproblemRPG::EvaluationInfo h(toSolve->rpg->getRelaxedPlan(currState, bestSolutionPCost, currState.timeStamp,
                                                                           currState.nextTIL,  goals, numericGoals, helpfulActions,
                                                                           relaxedPlan, 0, 0.001));        
        
        if (h.first == -1) {
            cout << COLOUR_light_red << "Planner claims that the state reached is a dead end\n" << COLOUR_default;
            
            {
                const double pcost = PreferenceHandler::getCurrentCost(currState);
                
                cout << endl;
                cout << "; New best\n";
                cout << "; violation cost = " << pcost + RPGBuilder::getPermanentViolationCost() << " " << endl;
                cout << "; Violations: " << PreferenceHandler::getCurrentViolations(currState) << endl;
                
                
                list<int>::const_iterator stepTwo = soln.begin();
                
                int ip = 0;
                for (; stepTwo != stepItr; ++stepTwo, ++ip) {
                    cout << ip << ": " << *(RPGBuilder::getInstantiatedOp(*stepTwo)) << " [1]\n";
                }
                cout << ip << ": " << *(RPGBuilder::getInstantiatedOp(*stepItr)) << " [1]\n";
                cout << endl << endl;
                            
                            
            }
            
            return 1;
        }
        
        HTrio t(h.first,h.second,h.first,h.goalState);
        
        t.second += PreferenceHandler::getG(currState);
        
        if (t.goalsSatisfied) {
                        
            const double pcost = PreferenceHandler::getCurrentCost(currState);
            
            cout << endl;
            cout << "; New best\n";
            cout << "; violation cost = " << pcost + RPGBuilder::getPermanentViolationCost() << " " << endl;
            cout << "; Violations: " << PreferenceHandler::getCurrentViolations(currState) << endl;
            
            list<int>::const_iterator stepTwo = soln.begin();
            
            int ip = 0;
            for (; stepTwo != stepItr; ++stepTwo, ++ip) {
                cout << ip << ": " << *(RPGBuilder::getInstantiatedOp(*stepTwo)) << " [1]\n";
            }
            cout << ip << ": " << *(RPGBuilder::getInstantiatedOp(*stepItr)) << " [1]\n";
            cout << endl << endl;
            
            if (pcost < bestSolutionPCost) {
                cout << COLOUR_yellow << "State reached is the new best goal state: cost " << pcost << COLOUR_default << endl;
                bestSolutionPCost = pcost;                
            }
            roomforimprovement = (t.second < bestSolutionPCost);
                        
        }
        
        list<vector<AutomatonPosition> > tList;
        tList.push_back(currState.preferenceStatus);
        
        if (!zealousVisitedStates.insert(make_pair(currState, tList)).second) {
            cout << "Planner would prune the resulting state as being equal to one already seen\n";
            list<pair<ExtendedMinimalState,int> >::iterator mItr = statesSeen.begin();
            const list<pair<ExtendedMinimalState,int> >::iterator mEnd = statesSeen.end();
            for (; mItr != mEnd; ++mItr) {
                if (mItr->first.first == currState.first
                    && mItr->first.second == currState.second) {
                    cout << "\t\tThe facts are the same as those";
                    if (mItr->second == -1) {
                        cout << " in the initial state";
                    } else {
                        cout << " after step " << mItr->second;
                    }
                    cout << endl;                
                
                    if (mItr->first.preferenceStatus == currState.preferenceStatus) {
                        cout << "\t\tThe preference statuses match too\n";
                        break;
                    } else {
                        cout << "\t\tBut, the preference statuses don't match\n";
                    }
                }
            }
            if (mItr == mEnd) {
                cout << COLOUR_light_red << "\tAs such, the pruning would be possibly incorrect\n" << COLOUR_default;               
            } else {
                cout << "\tAs such, the pruning would be okay, continuing\n";
            }
        }
        
        cout << "\tReached a state with a heuristic value of " << h.first << endl;
        
        statesSeen.push_back(make_pair(currState, i));
        
        #ifdef FFSEARCHDEBUGHOOKS
        if (primeKnownStates) {            
            knownGoodStates[currState].push_back(StepInfo(i,RPGBuilder::getInstantiatedOp(*stepItr),currState.preferenceStatus,t,(i+1 == soln.size())));
        }
        #endif
    }
    
    
    cout << "Returning 0\n";
    
    return 0;
}

int paretoCheck(const list<vector<AutomatonPosition> > & existing, const vector<AutomatonPosition> & proposed)
{
    if (!FF::disablePareto) return 0;
    static const int tpCount = proposed.size();
    
    if (!tpCount) return 0;
    if (existing.empty()) return 1;
    
    static int pStatus;
    static int p;
    static bool anyBetter;
    //static bool anyWorse;
    
    list<vector<AutomatonPosition> >::const_iterator exItr = existing.begin();
    const list<vector<AutomatonPosition> >::const_iterator exEnd = existing.end();
    
    for (; exItr != exEnd; ++exItr) {
        anyBetter = false;
        //anyWorse = false;
        for (p = 0; p < tpCount; ++p) {
            pStatus = PreferenceHandler::compareStatusOfPref(p,proposed[p], (*exItr)[p]);
            /*if (pStatus > 0) {
                anyWorse = true;
            } else */if (pStatus < 0) {
                anyBetter = true;
            }
        }
        
        if (!anyBetter) {
            // is no better than one we've already visited
            return 0;
        }
    }
    
    return FF::disablePareto;
}

list<FFEvent> * FF::solveSubproblemWRTSchedule(LiteralSet & startingState, vector<double> & startingFluents, SubProblem* const toSolve, const int nodeLimit,bool & reachedGoal, const int & spID, list<FFEvent> * oldSoln, double & oldSolutionValue, int & relaxedStepCount) {    
    
	static bool initCSBase = false;
	
	if (!initCSBase) {
		initCSBase = true;
		vector<dominance_constraint> & dcs = RPGBuilder::getDominanceConstraints();
		const int pneCount = dcs.size();
		CSBase::ignorableFluents = vector<bool>(pneCount);
		for (int i = 0; i < pneCount; ++i) {
			CSBase::ignorableFluents[i] = (dcs[i] == E_METRIC || dcs[i] == E_IRRELEVANT);
		}
	}
	
	#ifdef FFSEARCHDEBUGHOOKS
	if (actuallyPlanGivenPreviousSolution) {
        cout << COLOUR_light_blue << "Planning, comparing states reached against those along provided trajectory\n" << COLOUR_default ;
    }
	#endif
	
	const bool ffDebug = false;

	relaxedStepCount = 0;
	FFheader_upToDate = false;
	FFonly_one_successor = false;
	WAStar = false;
	set<int> goals;
	set<int> numericGoals;
	ExtendedMinimalState initialState;

	{
		list<Literal*>::iterator gsItr = toSolve->goals.begin();
		const list<Literal*>::iterator gsEnd = toSolve->goals.end();

		for (; gsItr != gsEnd; ++gsItr) {
			goals.insert((*gsItr)->getStateID());
		}

	}
	{
		list<int>::iterator gsItr = toSolve->goalFluents.begin();
		const list<int>::iterator gsEnd = toSolve->goalFluents.end();
		
		for (; gsItr != gsEnd; ++gsItr) {
			numericGoals.insert(*gsItr);
		}
	}
	{
		set<int> & isLocal = initialState.first;
		LiteralSet::iterator isItr = startingState.begin();
		const LiteralSet::iterator isEnd = startingState.end();

		for (; isItr != isEnd; ++isItr) {
			isLocal.insert((*isItr)->getStateID());
		}
	}

	{
		initialState.second = startingFluents;
        initialState.preferenceStatus = PreferenceHandler::getInitialAutomataPositions();
                
	}

    {
        initialState.landmarkStatus.resize(LandmarksAnalysis::getFactsInAnyLandmark().size(),false);
        initialState.prefPreconditionViolations = 0.0;
        initialState.cost = 0.0;
    }
    
    
    if (initialState.preferenceStatus.size() && allowDualOpenList) {
        useDualOpenList = true;
        cout << "Using dual open list for preference optimisation\n";
    }
    
	if (ffDebug) {
		cout << "Solving subproblem\n";
	}

	map<ExtendedMinimalState, list<vector<AutomatonPosition> >, CompareStates> visitedStates;
	map<ExtendedMinimalState, list<vector<AutomatonPosition> >, CompareStatesZealously> zealousVisitedStates;

	SearchQueue searchQueue;

	HTrio bestHeuristic;
	HTrio initialHeuristic;   
    
	HTrio bestNodeLimitHeuristic = HTrio(DBL_MAX, DBL_MAX, INT_MAX);
	list<FFEvent> * bestNodeLimitPlan = 0;

    double bestSolutionCost = DBL_MAX;
    bestSolutionPCost = capOnPreferenceCost;
    list<FFEvent> * bestSolution = 0;
    
    list<pair<int, VAL::time_spec> > initialHA;
    list<pair<double, double> > initialHMT;
	{
		SearchQueueItem * const initialSQI = new SearchQueueItem(initialState);
		list<FFEvent> tEvent;
		bool stagnant = false;
		FFheader_upToDate = false;
		FFonly_one_successor = true;
		bestHeuristic = evaluateStateWRTSchedule(toSolve->rpg, initialState, goals, numericGoals, initialSQI->helpfulActions, initialSQI->helpfulHowManyTimes, initialSQI->pointsOfConflict, initialSQI->plan, tEvent, bestNodeLimitHeuristic, bestNodeLimitPlan, reachedGoal, spID, stagnant);
        bestHeuristic.second += PreferenceHandler::getG(initialState);
		initialSQI->heuristicValue = bestHeuristic;
		initialHeuristic = bestHeuristic;
        initialHA = initialSQI->helpfulActions;
        initialHMT = initialSQI->helpfulHowManyTimes;   
        
        #ifdef FFSEARCHDEBUGHOOKS
        if (FF::actuallyPlanGivenPreviousSolution) {
            keepingState(initialSQI);
        }
        #endif
        
		searchQueue.push_back(initialSQI,1);
		
		relaxedStepCount = (relaxedGoalJump ? bestHeuristic.third : 0);
        
        if (initialHeuristic.goalsSatisfied) {
            cout << "\n\n; Initial state is a solution\n";
            reachedGoal = true;
            bestSolutionPCost = PreferenceHandler::getCurrentCost(initialState);
            bestSolutionCost = bestSolutionPCost;
            cout << "; Preference violation cost = " << bestSolutionPCost + RPGBuilder::getPermanentViolationCost() << " , Time = ";
            {
                tms refReturn;
                times(&refReturn);              
                double secs = ((double)refReturn.tms_utime + (double)refReturn.tms_stime) / ((double) sysconf(_SC_CLK_TCK));
                
                int twodp = (int) (secs * 100.0);
                int wholesecs = twodp / 100;
                int centisecs = twodp % 100;
                
                cout << wholesecs << ".";
                if (centisecs < 10) cout << "0";
                cout << centisecs << "\n";
                cout << ";\n; Time = " << wholesecs << ".";
                if (centisecs < 10) cout << "0";
                cout << centisecs << "\n";
            }
            
            cout << ";\n;\n";
            
            if (!RPGBuilder::getPreferences().empty()) {
                cout << "; " << PreferenceHandler::getCurrentViolations(initialState) << endl;
                cout << ";\n;\n\n";
            }
                        
            bestSolution  = new list<FFEvent>();
            if (bestSolutionPCost == 0.0) {
                return bestSolution;
            } else if (bestHeuristic.second >= bestSolutionPCost) {
                return bestSolution;
            }
        }
	}

	if (ffDebug) cout << "Initial heuristic = " << bestHeuristic.first << ", tie breaker " << bestHeuristic.second << "\n";

	if (bestHeuristic.first == -1) {
		reachedGoal = false;
		delete bestNodeLimitPlan;
		return 0;
	}

	auto_ptr<list<FFEvent> > bestPlan(new list<FFEvent>());
	{

		list<vector<AutomatonPosition> > tList;
		tList.push_back(initialState.preferenceStatus);
		
		visitedStates.insert(make_pair(initialState, tList));
		zealousVisitedStates.insert(make_pair(initialState, tList));
	}
	cout << "(" << bestHeuristic.first << ", " << bestHeuristic.second << ") "; cout.flush();


    


    
	bool firstTimeSpecial = initialAdvancing;

	if (skipEHC) searchQueue.clearall();
	
	int nextPopFrom = 1;    
    int thisPopWasFrom = -1;
        
	
	while (!searchQueue.empty()) {
		if (GlobalSchedule::globalVerbosity & 2) cout << "\n--\n";
		
		if (searchQueue.useOtherQueue(nextPopFrom)) {
            nextPopFrom = 3 - nextPopFrom;
        }
		
		auto_ptr<SearchQueueItem> currSQI(searchQueue.pop_front(nextPopFrom));

        //cout << "Visiting state at " << currSQI.get() << endl;
        
        thisPopWasFrom = nextPopFrom;
        
        if (useDualOpenList) {
            nextPopFrom = 3 - thisPopWasFrom;
        }
        
        
        if (bestSolutionPCost != DBL_MAX) {
            if (currSQI->heuristicValue.second >= bestSolutionPCost) {
                cout << "~";
                cout.flush();
                continue;
            }
        }
        
        if (GlobalSchedule::globalVerbosity & 2) {
            currSQI->printPlan();
        }
        
		bool foundBetter = false;

		
		list<pair<int, VAL::time_spec> > maybeApplicableActions;
		list<pair<double,double> > scrapList;
		list<pair<double,double> >::iterator hhmItr;

        list<pair<int, VAL::time_spec> >::iterator helpfulActsItr;
        list<pair<int, VAL::time_spec> >::iterator helpfulActsEnd;

		if (!foundBetter) {
			if (helpfulActions) {
				//cout << "(( " << currSQI->helpfulActions.size() << "))";
				toSolve->rpg->filterApplicableActions(currSQI->state, currSQI->state.nextTIL, currSQI->helpfulActions, &(currSQI->helpfulHowManyTimes));
                reorderNoDeletesFirst(currSQI->helpfulActions, &(currSQI->helpfulHowManyTimes));
                reorderByPrefCost(toSolve->rpg, currSQI.get(), currSQI->helpfulActions, currSQI->helpfulHowManyTimes);
                helpfulActsItr = currSQI->helpfulActions.begin();
                helpfulActsEnd = currSQI->helpfulActions.end();                
                hhmItr = currSQI->helpfulHowManyTimes.begin();
				if (GlobalSchedule::globalVerbosity & 1) {
                                    cout << "(( " << currSQI->helpfulActions.size() << "))";
                                    cout.flush();
                                }
				FFonly_one_successor = (currSQI->helpfulActions.size() == 1);
			} else {
				toSolve->rpg->findApplicableActions(currSQI->state, currSQI->state.nextTIL, maybeApplicableActions);
                reorderNoDeletesFirst(maybeApplicableActions, 0);
				helpfulActsItr = maybeApplicableActions.begin();
				helpfulActsEnd = maybeApplicableActions.end();
				const int lim = maybeApplicableActions.size();
				for (int i = 0; i < lim; ++i) scrapList.push_back(pair<double,double>(1.0,1.0));
				hhmItr = scrapList.begin();
				cout << "(( " << maybeApplicableActions.size() << "))";
				cout.flush();
				FFonly_one_successor = (maybeApplicableActions.size() == 1);
			}
		} else {
	                helpfulActsItr = helpfulActsEnd = currSQI->helpfulActions.end();			
		}

		FFheader_upToDate = false;
		
		
		if (FF::multipleHelpfuls) {
            
            SearchQueueItem * s1returned;
            SearchQueueItem * s2returned;
            
            helpfulHelper(helpfulActsItr, helpfulActsEnd, hhmItr, currSQI.get(), toSolve, s1returned, s2returned);

            
			auto_ptr<SearchQueueItem> succOne(s1returned);
            auto_ptr<SearchQueueItem> succTwo(s2returned);
            
            for (int sc = 0; sc < 2; ++sc) {
                if (sc ? (s2returned == 0) : (s1returned == 0)) {
                    continue;
                }
                
                assert(!sc || FF::useDualOpenList);                
                
                auto_ptr<SearchQueueItem> succ(sc ? succTwo.release() : succOne.release());
                                                
                assert(succ.get());
                
				map<ExtendedMinimalState, list<vector<AutomatonPosition> >, CompareStatesZealously>::iterator zvsItr
				  = zealousVisitedStates.insert(make_pair(succ->state, list<vector<AutomatonPosition> >())).first;
				if (zvsItr->second.empty()) {
                    #ifdef FFSEARCHDEBUGHOOKS
                    if (FF::actuallyPlanGivenPreviousSolution) {
                        keepingState(succ.get());
                    }
                    #endif
                                                                            
					list<FFEvent> tEvent;
					bool stagnant = false;
					FFheader_upToDate = false;
					FFonly_one_successor = true;

					succ->heuristicValue = evaluateStateWRTSchedule(toSolve->rpg, succ->state, goals, numericGoals, succ->helpfulActions, succ->helpfulHowManyTimes, succ->pointsOfConflict,succ->plan, tEvent, bestNodeLimitHeuristic, bestNodeLimitPlan, reachedGoal, spID, stagnant);
                    					
					if (succ->heuristicValue.first != -1.0) {
		
                        succ->heuristicValue.second += PreferenceHandler::getG(succ->state);
                        
                        bool roomforimprovement = true;
			
						if (succ->heuristicValue.goalsSatisfied) {
                            

                            const double pcost = PreferenceHandler::getCurrentCost(succ->state);
                            
                            
                            if (pcost < bestSolutionPCost) {
                                cout << "g[" << pcost << "] "; cout.flush();                                
                                reachedGoal = true;                                
                                relaxedStepCount = 0;
                                delete bestSolution;
                                succ->printNewSolution();
                                bestSolution = new list<FFEvent>(succ->plan);
                                bestSolutionCost = succ->state.cost + pcost; 
                                bestSolutionPCost = pcost;
                                if (pcost == 0.0) return bestSolution;
                                
                                
                                bestHeuristic.first = searchQueue.newBestPrefScore(bestSolutionPCost);                                                                                                                                                                                                
                            }
                            roomforimprovement = (succ->heuristicValue.second < bestSolutionPCost);
                            if (roomforimprovement) {
                                cout << "^"; cout.flush();
                            }
                        }
                        
						if (roomforimprovement) {                            
							//if (((succ->heuristicValue.first - bestHeuristic.first) < -0.5) || ((succ->heuristicValue.first-bestHeuristic.first >= 0.5) && succ->heuristicValue.second < bestHeuristic.second)) {						
                            
                            const bool firstQueueCriteria = (succ->heuristicValue.first - bestHeuristic.first < 0);
                            const bool secondQueueCriteria = (succ->heuristicValue.second - bestHeuristic.second < 0);
                            
							if (firstQueueCriteria || secondQueueCriteria) {
								if (firstQueueCriteria) {
                                    bestHeuristic.first = succ->heuristicValue.first;
                                    cout << COLOUR_light_red << "L (" << bestHeuristic.first << ")" << COLOUR_default ; cout.flush();
                                }
                                if (secondQueueCriteria) {
                                    bestHeuristic.second = succ->heuristicValue.second;
                                    cout << "P (" << bestHeuristic.second << ")" ; cout.flush();
                                }
                                
								//const int oldVerbosity = GlobalSchedule::globalVerbosity;
								//GlobalSchedule::globalVerbosity = 2;
//								succ->printPlan();
								//GlobalSchedule::globalVerbosity = oldVerbosity;
                                if (firstQueueCriteria && secondQueueCriteria) {
                                    searchQueue.clearall();
                                    searchQueue.push_back(succ.release(),1);
                                } else {
                                    if (firstQueueCriteria) searchQueue.clear(1);
                                    if (secondQueueCriteria) searchQueue.clear(2);
                                    
                                    searchQueue.push_back(succ.release(),1);
                                    if (!FF::steepestDescent) {
                                        foundBetter = true;
                                    }
                                }
							} else {
								cout << "l"; cout.flush();
								searchQueue.push_back(succ.release(),1);
							}
						} else {
                            #ifdef FFSEARCHDEBUGHOOKS
                            if (actuallyPlanGivenPreviousSolution) {
                                checkWhetherSuccessorReallyShouldBePruned(succ.get(),true);
                            }
                            #endif
                        }
					} else {
                        #ifdef FFSEARCHDEBUGHOOKS
                        if (actuallyPlanGivenPreviousSolution) {
                            checkWhetherSuccessorReallyShouldBePruned(succ.get(),false);
                        }
                        #endif
						cout << "_"; cout.flush();
						zvsItr->second.push_back(succ->state.preferenceStatus);

					}
	
				} else {
                    #ifdef FFSEARCHDEBUGHOOKS
                    if (actuallyPlanGivenPreviousSolution) {
                        checkWhetherSuccessorReallyShouldBePruned(succ.get(),false);
                    }
                    #endif
                                                            
                }
			}

		}
		
		if (foundBetter) {
            cout << "#"; cout.flush();
//			cout << "skipping actual expansion "; cout.flush();
			helpfulActsItr = helpfulActsEnd;
		}

		for (; helpfulActsItr != helpfulActsEnd; ++helpfulActsItr, ++hhmItr) {
		
            if (GlobalSchedule::globalVerbosity & 2) {
                cout << "Applying " << *(RPGBuilder::getInstantiatedOp(helpfulActsItr->first)) << endl;
            }
            
			//cout << "Arg: " << *(RPGBuilder::getInstantiatedOp(helpfulActsItr->first));
//			if (helpfulActsItr->second == VAL::E_AT_START) {
//				cout << " start\n";
//			} else {
//				cout << " end\n";
//			}
			auto_ptr<SearchQueueItem> succ(new SearchQueueItem(currSQI->state));
//			cout << "Before:\n";
//			printState(succ->state);



			int howManyInTotal = 0;
			if (hhmItr->first == 0.0) hhmItr->first = 0.1;

			for (double hm = 0; hm < hhmItr->first; hm += 1.0) {
				if (hm > 0.0) {
					if (!toSolve->rpg->testApplicability(succ->state, succ->state.nextTIL, (*helpfulActsItr), false,false)) break;
				}
                applyActionToState(toSolve->rpg, succ->state, helpfulActsItr->first, helpfulActsItr->second);
				++howManyInTotal;
			}

//			cout << "After:\n";
//			printState(succ->state);

            {


				bool visitTheState = false;
				map<ExtendedMinimalState, list<vector<AutomatonPosition> >, CompareStatesZealously>::iterator zvsItr;
				map<ExtendedMinimalState, list<vector<AutomatonPosition> >, CompareStates>::iterator vsItr;

				if (zealousEHC) {
					zvsItr = zealousVisitedStates.insert(make_pair(succ->state, list<vector<AutomatonPosition> >())).first;
					if (zvsItr->second.empty()) {
						visitTheState = true;
					} else {
						//visitTheState = fabs(succ->state.timeStamp - zvsItr->second.back().second) > 0.0005 && succ->state.timeStamp < zvsItr->second.back().second;
						visitTheState = false;
					}
				} else {
					vsItr = visitedStates.insert(make_pair(succ->state, list<vector<AutomatonPosition> >())).first;
					if (vsItr->second.empty()) {
						visitTheState = true;
					} else {                                                
						//visitTheState = fabs(succ->state.timeStamp - zvsItr->second.back().second) > 0.0005 && succ->state.timeStamp < vsItr->second.back().second;
                        //return false;
                        visitTheState = (paretoCheck(zvsItr->second, succ->state.preferenceStatus) == 1);
                        if (visitTheState) {
                            cout << "R"; cout.flush();                            
                        }
					}
				}
					
	
				if (!visitTheState) {
					cout << "s";
                    #ifdef FFSEARCHDEBUGHOOKS
                    if (actuallyPlanGivenPreviousSolution) {
                        checkWhetherSuccessorReallyShouldBePruned(succ.get(),false);
                    }
                    #endif
                                                
				} else {
					                                        
					bool stagnant = false;
					//chooseWhetherToReuse(tlPoint, succ, toSolve->rpg, succ->state, goals, numericGoals, succ->helpfulActions, succ->pointsOfConflict, currSQI->tlPoint, pair<instantiatedOp*, VAL::time_spec>(toSolve->rpg->getOp(helpfulActsItr->first), helpfulActsItr->second), currSQI->plan, bestNodeLimitHeuristic, bestNodeLimitPlan, reachedGoal, spID, stagnant);
					
					chooseWhetherToReuseAndWait(succ, toSolve->rpg, succ->state, goals, numericGoals, succ->helpfulActions, succ->helpfulHowManyTimes, succ->pointsOfConflict, pair<instantiatedOp*, VAL::time_spec>(toSolve->rpg->getOp(helpfulActsItr->first), helpfulActsItr->second), howManyInTotal, currSQI->plan, bestNodeLimitHeuristic, bestNodeLimitPlan, reachedGoal, spID, stagnant);
					
//					succ->printPlan();

					if (succ->heuristicValue.first != -1.0) {
	
                        
                        bool roomforimprovement = true;
                        
                        succ->heuristicValue.second += PreferenceHandler::getG(succ->state);
                        
                        if (succ->heuristicValue.goalsSatisfied) {
                            

                            const double pcost = PreferenceHandler::getCurrentCost(succ->state);
                            if (pcost < bestSolutionPCost) {
                                cout << "g[" << pcost << "] "; cout.flush();
                                reachedGoal = true;                                
                                relaxedStepCount = 0;
                                delete bestSolution;
                                succ->printNewSolution();
                                bestSolution = new list<FFEvent>(succ->plan);
                                bestSolutionCost = succ->state.cost + pcost;
                                bestSolutionPCost = pcost;
                                if (pcost == 0.0) return bestSolution;
                                
                                
                                bestHeuristic.first = searchQueue.newBestPrefScore(bestSolutionPCost);
                            }
                            roomforimprovement = (succ->heuristicValue.second < bestSolutionPCost);
                            if (roomforimprovement) {
                                cout << "^"; cout.flush();
                            }
                        }
                                                
                                                                        
		
						
						if (zealousEHC) {
							zvsItr->second.push_back(succ->state.preferenceStatus);
						} else {
							vsItr->second.push_back(succ->state.preferenceStatus);
						}
	
						if (!stagnant && FF::stagnantCommitted) {
							if (succ->heuristicValue.second >= bestNodeLimitHeuristic.first) stagnant = true;
						}
						
                        if (roomforimprovement) {
                   
                            if (!stagnant) {
                                //if (((succ->heuristicValue.first - bestHeuristic.first) < -0.5) || ((succ->heuristicValue.first-bestHeuristic.first >= 0.5) && succ->heuristicValue.second < bestHeuristic.second)) {						
                                   
                                #ifdef FFSEARCHDEBUGHOOKS
                                if (FF::actuallyPlanGivenPreviousSolution) {
                                    keepingState(succ.get());
                                }
                                #endif
                                    
                                bool aBetter = false;
                                bool bBetter = false;
                                if (useWeightedSumWithPrefCost
                                    ?  ((succ->heuristicValue.first + prefWeightInWeightedSum * succ->heuristicValue.second) - (bestHeuristic.first + prefWeightInWeightedSum * bestHeuristic.second) < 0)
                                    :  (succ->heuristicValue.first - bestHeuristic.first < 0) ) {
                                    aBetter = true;
                                    bestHeuristic.first = succ->heuristicValue.first;
                                    if (useWeightedSumWithPrefCost) {
                                        bestHeuristic.second = succ->heuristicValue.second;
                                    }
                                    cout << COLOUR_light_red << "b (" << (useWeightedSumWithPrefCost ? bestHeuristic.first + prefWeightInWeightedSum * bestHeuristic.second : bestHeuristic.first) << ")" << COLOUR_default; cout.flush();
                                    //const int oldVerbosity = GlobalSchedule::globalVerbosity;
                                    //GlobalSchedule::globalVerbosity = 2;
                                    succ->printPlan();
                                    //GlobalSchedule::globalVerbosity = oldVerbosity;
                                    searchQueue.clear(1);
                                } else {
                                    cout << "."; cout.flush();
                                }
                            
                                if (allowDualOpenList) {
                                    if (succ->heuristicValue.second - bestHeuristic.second < 0) {
                                        bBetter = true;
                                        bestHeuristic.second = succ->heuristicValue.second;
                                        searchQueue.clear(2);
                                    }
                                }
                                searchQueue.push_back(succ.release(),1);
                                    
                                if (thisPopWasFrom == 1) {
                                    if (aBetter && !FF::steepestDescent) {
                                        foundBetter = true;
                                        break;
                                    }
                                } else {
                                    if (bBetter && !FF::steepestDescent) {
                                        foundBetter = true;
                                        break;
                                    }
                                }
                                
                            } else {
                                cout << "!"; cout.flush();
                                #ifdef FFSEARCHDEBUGHOOKS
                                if (actuallyPlanGivenPreviousSolution) {
                                    checkWhetherSuccessorReallyShouldBePruned(succ.get(),false);
                                }
                                #endif
                                                                                
                            }
                        } else {
                            #ifdef FFSEARCHDEBUGHOOKS
                            if (actuallyPlanGivenPreviousSolution) {
                                checkWhetherSuccessorReallyShouldBePruned(succ.get(),true);
                            }
                            #endif
                                                                            
                        }
					} else {
						cout << "d"; cout.flush();
						if (zealousEHC) {
							zvsItr->second.push_back(succ->state.preferenceStatus);
						} else {
							vsItr->second.push_back(succ->state.preferenceStatus);
						}
                        #ifdef FFSEARCHDEBUGHOOKS
                        if (actuallyPlanGivenPreviousSolution) {
                            checkWhetherSuccessorReallyShouldBePruned(succ.get(),false);
                        }
                        #endif
                                                                                                
					}
				}
			}
		}

		FFonly_one_successor = false;

		assert(!foundBetter || !searchQueue.empty());
	}

	if (!bestFirstSearch) {
		cout << "\nProblem unsolvable by EHC\n";

		reachedGoal = false;
		relaxedStepCount = (relaxedGoalJump ? bestNodeLimitHeuristic.third : 0);
		return bestNodeLimitPlan;
	}


    if (bestSolutionPCost < initialHeuristic.second) {
        cout << "\nAfter EHC, converged on a solution of cost " << bestSolutionPCost << endl;
        if (bestSolution) {
            return bestSolution;
        }        
    }
    
	cout << "\nResorting to best-first search\n";
    
	WAStar = true;
	visitedStates.clear();
	zealousVisitedStates.clear();

    searchQueue.clearall();
    searchQueue.seenAnyNonZeroPCosts = false;
    
    #ifdef FFSEARCHDEBUGHOOKS
    if (FF::actuallyPlanGivenPreviousSolution) {
        resetKnownGoodFlags();
    }
    #endif
            
    
	{
		SearchQueueItem * const initialSQI = new SearchQueueItem(initialState);
		initialSQI->heuristicValue = initialHeuristic;
        initialSQI->helpfulActions = initialHA;
        initialSQI->helpfulHowManyTimes = initialHMT;

        #ifdef FFSEARCHDEBUGHOOKS
        if (FF::actuallyPlanGivenPreviousSolution) {
            keepingState(initialSQI);
        }
        #endif
                
        
		bestHeuristic = initialHeuristic;
		searchQueue.insert(initialSQI,1);
		//visitedStates.find(initialSQI->state)->second.second = true;

		list<vector<AutomatonPosition> > tList;
		tList.push_back(initialState.preferenceStatus);

		visitedStates.insert(make_pair(initialState, tList));
		zealousVisitedStates.insert(make_pair(initialState, tList));
	}


	firstTimeSpecial = initialAdvancing;

	while (!searchQueue.empty()) {

        
        if (searchQueue.useOtherQueue(nextPopFrom)) {
            nextPopFrom = 3 - nextPopFrom;
        }
        
		auto_ptr<SearchQueueItem> currSQI(searchQueue.pop_front(nextPopFrom));

        if (FF::useDualOpenList) {
            thisPopWasFrom = nextPopFrom;
            nextPopFrom = 3 - thisPopWasFrom;
        }
        
        if (bestSolutionPCost != DBL_MAX) {
            if (currSQI->heuristicValue.second >= bestSolutionPCost) {
                cout << "~";
                cout.flush();
                continue;
            }
        }
                        
        
		if (GlobalSchedule::globalVerbosity & 2) {
			cout << "\n--\n";
			cout << "Now visiting state with heuristic value of " << currSQI->heuristicValue.first << "\n";
			printState(currSQI->state);
			currSQI->printPlan();
			cout << "\n + \n";
		}

		bool foundBetter = false;

		list<pair<int, VAL::time_spec> > applicableActions;

		if (!foundBetter) toSolve->rpg->findApplicableActions(currSQI->state, currSQI->state.nextTIL, applicableActions);

        reorderNoDeletesFirst(applicableActions, 0);
		reorderHelpfulFirst(applicableActions, currSQI->helpfulActions);
        
        
		if (GlobalSchedule::globalVerbosity & 2) {
			cout << "Applicable actions are:\n";
			list<pair<int, VAL::time_spec> >::iterator helpfulActsItr = applicableActions.begin();
			const list<pair<int, VAL::time_spec> >::iterator helpfulActsEnd = applicableActions.end();
			for (; helpfulActsItr != helpfulActsEnd; ++helpfulActsItr) {
				if (helpfulActsItr->second == VAL::E_AT) {
					cout << "\t Timed initial literal " << helpfulActsItr->first << "\n";
				} else {
					cout << "\t " << *(RPGBuilder::getInstantiatedOp(helpfulActsItr->first)) << ", ";
					if (helpfulActsItr->second == VAL::E_AT_START) {
						cout << "start\n";
					} else {
						cout << "end\n";
					}
				}
			}
			cout << "\nHeuristic values are:\n";
		
		}

		FFheader_upToDate = false;
		FFonly_one_successor = (applicableActions.size() == 1);

        auto_ptr<SearchQueueItem> hsucc1;
        auto_ptr<SearchQueueItem> hsucc2;
        
        
        if (FF::multipleHelpfuls) {
            SearchQueueItem * s1returned;
            SearchQueueItem * s2returned;
            
            reorderByPrefCost(toSolve->rpg, currSQI.get(), currSQI->helpfulActions, currSQI->helpfulHowManyTimes);
            
            FF::helpfulHelper(currSQI->helpfulActions.begin(),
                              currSQI->helpfulActions.end(),
                              currSQI->helpfulHowManyTimes.begin(), currSQI.get(), toSolve, s1returned, s2returned);
            if (s2returned) {
                assert(FF::useDualOpenList);
                hsucc2 = auto_ptr<SearchQueueItem>(s2returned);
                applicableActions.push_front(make_pair(-2, VAL::E_OVER_ALL));
            }
                                                
            if (s1returned) {
                hsucc1 = auto_ptr<SearchQueueItem>(s1returned);
                applicableActions.push_front(make_pair(-1, VAL::E_OVER_ALL));
            }
            
        }
        
		list<pair<int, VAL::time_spec> >::iterator helpfulActsItr = applicableActions.begin();
		const list<pair<int, VAL::time_spec> >::iterator helpfulActsEnd = applicableActions.end();

		for (;helpfulActsItr != helpfulActsEnd; ++helpfulActsItr) {
			auto_ptr<SearchQueueItem> succ;
            
            if (helpfulActsItr->first <= -1) {
                if (helpfulActsItr->first == -1) {
                    succ = auto_ptr<SearchQueueItem>(hsucc1.release());
                } else if (helpfulActsItr->first == -2) {
                    succ = auto_ptr<SearchQueueItem>(hsucc2.release());
                }
            } else {
                succ = auto_ptr<SearchQueueItem>(new SearchQueueItem(currSQI->state));

                
                applyActionToState(toSolve->rpg, succ->state, helpfulActsItr->first, helpfulActsItr->second);
        

            }
            
            {

				if (GlobalSchedule::globalVerbosity & 2) {
                    if (helpfulActsItr->first >= 0) {
                        cout << "\tAfter applying " << *(RPGBuilder::getInstantiatedOp(helpfulActsItr->first));
                        cout << " state is:\n";
                        printState(succ->state);
                    } else {
                        cout << "\tAfter applying HA group " << -(helpfulActsItr->first);
                        cout << " state is:\n";
                        printState(succ->state);                        
                    }
				}

				int visitTheState = 0;
				map<ExtendedMinimalState, list<vector<AutomatonPosition> >, CompareStatesZealously>::iterator zvsItr
				 = zealousVisitedStates.insert(make_pair(succ->state, list<vector<AutomatonPosition> >())).first;
				map<ExtendedMinimalState, list<vector<AutomatonPosition> >, CompareStates>::iterator vsItr
                 = visitedStates.insert(make_pair(succ->state, list<vector<AutomatonPosition> >())).first;

				//if (vsItr->second.empty() || (fabs(vsItr->second.back().second - succ->state.timeStamp) > 0.0005 && (vsItr->second.back().second > succ->state.timeStamp))) {
                if (zvsItr->second.empty()) {
                    visitTheState = 1;					
				} else {
                    visitTheState = paretoCheck(zvsItr->second, succ->state.preferenceStatus);
                    
                    if (visitTheState == 0) {
                        if (GlobalSchedule::globalVerbosity & 2) {
                            cout << "\tState visited before\n";
                        } else {
                            cout << "s";
                        }
                        #ifdef FFSEARCHDEBUGHOOKS
                        if (actuallyPlanGivenPreviousSolution) {
                            checkWhetherSuccessorReallyShouldBePruned(succ.get(),false);
                        }
                        #endif
                                                                                                
                    } else {
                        if (visitTheState == 1) {
                            cout << "R"; cout.flush();                            
                        } else {
                            cout << "r"; cout.flush();                            
                        }
                    }
                    //visitTheState = 0;
								
					
				}
				
				if (visitTheState) {
                    if (GlobalSchedule::globalVerbosity & 1) {
                        if (helpfulActsItr->first == -1) {                        
                            cout << "(L)"; cout.flush();
                        }
                        if (helpfulActsItr->first == -2) {
                            cout << "(l)"; cout.flush();
                        }
                    }
                                                            
					bool stagnant = false;
                    /*					
					if (helpfulActsItr->second == VAL::E_AT) {
						list<FFEvent> tilEvents;
						for (int tn = oldTIL; tn <= helpfulActsItr->first; ++tn) {
							tilEvents.push_back(FFEvent(GlobalSchedule::getCurrentManipulator()->getTILNode(tn), true));
						}
						FFcache_upToDate = false;

						justEvaluateNotReuse(succ, toSolve->rpg, succ->state, goals, numericGoals, succ->helpfulActions, succ->helpfulHowManyTimes, succ->pointsOfConflict, tilEvents                                                                                                   , currSQI->plan, bestNodeLimitHeuristic, bestNodeLimitPlan, reachedGoal, spID, stagnant, 0);
						
						succ->plan = currSQI->plan;
						succ->plan.insert(succ->plan.end(), tilEvents.begin(), tilEvents.end());
					} else {*/
                    
                    if (helpfulActsItr->first >= 0) {
                        chooseWhetherToReuseAndWait(succ, toSolve->rpg, succ->state, goals, numericGoals, succ->helpfulActions, succ->helpfulHowManyTimes, succ->pointsOfConflict, pair<instantiatedOp*, VAL::time_spec>(toSolve->rpg->getOp(helpfulActsItr->first), helpfulActsItr->second), 1, currSQI->plan, bestNodeLimitHeuristic, bestNodeLimitPlan, reachedGoal, spID, stagnant);
                    } else {
                        list<FFEvent> tEvent;
                        succ->heuristicValue = evaluateStateWRTSchedule(toSolve->rpg, succ->state, goals, numericGoals, succ->helpfulActions, succ->helpfulHowManyTimes, succ->pointsOfConflict,succ->plan, tEvent, bestNodeLimitHeuristic, bestNodeLimitPlan, reachedGoal, spID, stagnant);
                    }
					//}
	
					//succ->printPlan();
					if (succ->heuristicValue.first != -1.0) {
		
                        bool roomforimprovement = true;
                        
                        succ->heuristicValue.second += PreferenceHandler::getG(succ->state);
                        
                        if (succ->heuristicValue.goalsSatisfied) {
                                                        
                            const double pcost = PreferenceHandler::getCurrentCost(succ->state);
                            if (pcost < bestSolutionPCost) {
                                cout << "g[" << succ->state.cost + pcost << "] "; cout.flush();
                        
                                reachedGoal = true;                                
                                relaxedStepCount = 0;
                                delete bestSolution;
                                succ->printNewSolution();
                                bestSolution = new list<FFEvent>(succ->plan);
                                bestSolutionCost = succ->state.cost + pcost;
                                bestSolutionPCost = pcost;
                                if (pcost == 0.0) return bestSolution;
                                
                                
                                bestHeuristic.first = searchQueue.newBestPrefScore(bestSolutionPCost);
                            }
                            roomforimprovement = (succ->heuristicValue.second < bestSolutionPCost);
                            
                            if (roomforimprovement) {
                                cout << "^"; cout.flush();
                            } 
                        
                        }
                                                
                        
						
						vsItr->second.push_back(succ->state.preferenceStatus);
                        zvsItr->second.push_back(succ->state.preferenceStatus);
	
						if (!stagnant && FF::stagnantCommitted) {
							if (succ->heuristicValue.second >= bestNodeLimitHeuristic.first) stagnant = true;
						}
                        if (roomforimprovement) {
                            if (!stagnant) {
                                
                                #ifdef FFSEARCHDEBUGHOOKS
                                if (FF::actuallyPlanGivenPreviousSolution) {
                                    keepingState(succ.get());
                                }
                                #endif
                                
                                if (useWeightedSumWithPrefCost
                                    ?  succ->heuristicValue.first + prefWeightInWeightedSum * succ->heuristicValue.second < bestHeuristic.first + prefWeightInWeightedSum * bestHeuristic.second
                                     : succ->heuristicValue.first < bestHeuristic.first || (succ->heuristicValue.first == bestHeuristic.first && succ->heuristicValue.second < bestHeuristic.second)) {
                                    bestHeuristic = succ->heuristicValue;
                                    if (GlobalSchedule::globalVerbosity & 2) {
                                        cout << "\t" << bestHeuristic.first << ", category " << visitTheState << " - a new best heuristic value, with plan:\n";
                                        succ->printPlan();
                                    } else {
                                        cout << COLOUR_light_red <<  "b (" << bestHeuristic.first << ")" << COLOUR_default; cout.flush();
                                    }
                                    //const int oldVerbosity = GlobalSchedule::globalVerbosity;
                                    //GlobalSchedule::globalVerbosity = 2;
                                    
                                    //GlobalSchedule::globalVerbosity = oldVerbosity;
                                    searchQueue.insert(succ.release(), visitTheState);
                                } else {
                                    if (GlobalSchedule::globalVerbosity & 2) {
                                        cout << "\t" << succ->heuristicValue.first << ", category " << visitTheState << "\n";
                                    } else {
                                        cout << "."; cout.flush();
                                    }
                                    searchQueue.insert(succ.release(), visitTheState);
                                }
                            } else {
                                cout << "!"; cout.flush();
                                #ifdef FFSEARCHDEBUGHOOKS
                                if (actuallyPlanGivenPreviousSolution) {
                                    checkWhetherSuccessorReallyShouldBePruned(succ.get(),false);
                                }
                                #endif
                            }
                        } else {
                            #ifdef FFSEARCHDEBUGHOOKS
                            if (actuallyPlanGivenPreviousSolution) {
                                checkWhetherSuccessorReallyShouldBePruned(succ.get(),true);
                            }
                            #endif
                        }
					} else {
						if (GlobalSchedule::globalVerbosity & 2) {
							cout << "\tDead end\n";
						} else {
							cout << "d"; cout.flush();
						}
						#ifdef FFSEARCHDEBUGHOOKS
                        if (actuallyPlanGivenPreviousSolution) {
                            checkWhetherSuccessorReallyShouldBePruned(succ.get(),false);
                        }
                        #endif
						
					}
				}
			}
		}


	}

    cout << "\nConverged on a solution with cost " << bestSolutionPCost << "\n";

    if (bestSolution) {
        return bestSolution;
    }
    	
	cout << "\nProblem Unsolvable\n";

    assert(searchQueue.empty());
    
	reachedGoal = false;
	relaxedStepCount = (relaxedGoalJump ? bestNodeLimitHeuristic.third : 0);
	return bestNodeLimitPlan;

};


struct ExecutableState {

	ExtendedMinimalState currState;
	list<TimelinePoint*>::iterator timelineItr;
	list<list<TimelinePoint*>::iterator >::iterator sequenceItr;

};

list<list<TimelinePoint*>::iterator > FF::findExecutablePath(list<TimelinePoint*>::iterator startPoint, SubproblemRPG* rpg, ExtendedMinimalState & startState) {
	
	const bool debugFEP = false;


	list<list<TimelinePoint*>::iterator > toReturn;

	list<TimelinePoint*>::iterator tlItr = startPoint;
	list<TimelinePoint*>::iterator timelineEnd = GlobalSchedule::currentManipulator->getEndTimelinePoint();

	if (tlItr != timelineEnd && (*tlItr)->dummy) ++tlItr;

	map<ScheduleNode*, ExecutableState, ScheduleNodeLT> correspondingIterators;
	set<ScheduleNode*, ScheduleNodeLT> blacklisted;

	ExecutableState currentState;
	currentState.currState = startState;
	currentState.currState.nextTIL = 0;
	currentState.timelineItr = tlItr;
	currentState.sequenceItr = toReturn.end();

	map<ScheduleNode*, int, ScheduleNodeLT> tilIndices;
	int tilPool = 0;

	while (currentState.timelineItr != timelineEnd) {

		ScheduleNode* const currSN = (*((*currentState.timelineItr)->after));

		if (currSN->tilNode()) { // TIL

			toReturn.push_back(currentState.timelineItr);
			currentState.sequenceItr = --(toReturn.end());
			correspondingIterators[currSN] = currentState;
			map<ScheduleNode*, int, ScheduleNodeLT>::iterator tiItr = tilIndices.find(currSN);
			int useIndex;
			if (tiItr == tilIndices.end()) {
				tilIndices[currSN] = tilPool;
				useIndex = tilPool++;
			} else {
				useIndex = tiItr->second;
			}
			
			applyActionToState(rpg, currentState.currState, useIndex, VAL::E_AT);
			++(currentState.timelineItr);

		} else {

			const VAL::time_spec tsMatch = (currSN->isStart() ? VAL::E_AT_START : VAL::E_AT_END);
			const int currActID = currSN->getOp()->getID();

			ScheduleNode* const partner = currSN->getPartner();
	
	
			if (partner) {
				if (tsMatch == VAL::E_AT_START) {
					if (debugFEP) cout << "Considering start of " << *(currSN->getOp()) << "\n";
					if (blacklisted.find(currSN) == blacklisted.end()) {
						if (rpg->testApplicability(currentState.currState, currentState.currState.nextTIL, pair<int, VAL::time_spec>(currActID, tsMatch))) {
							if (debugFEP) cout << "Start is executable\n";
							toReturn.push_back(currentState.timelineItr);
							currentState.sequenceItr = --(toReturn.end());
							correspondingIterators[currSN] = currentState;
							applyActionToState(rpg, currentState.currState, currActID, tsMatch);
						} else {
							if (debugFEP) cout << "Start is inexecutable\n";
							correspondingIterators.erase(currSN);
						}
					} else {
						if (debugFEP) cout << "Start is blacklisted\n";
			
					}
					++(currentState.timelineItr);
				} else {
					if (debugFEP) cout << "Considering end of " << *(currSN->getOp()) << "\n";
					
					if (blacklisted.find(partner) == blacklisted.end()) {
		
	
	
						map<ScheduleNode*, ExecutableState, ScheduleNodeLT>::iterator ciItr = correspondingIterators.find(partner);
						
						if (ciItr != correspondingIterators.end()) {
		
							if (debugFEP) cout << "Corresponding start was executed\n";
							if (rpg->testApplicability(currentState.currState, currentState.currState.nextTIL, pair<int, VAL::time_spec>(currActID, tsMatch))) {
								if (debugFEP) cout << "End is executable\n";
								toReturn.push_back(currentState.timelineItr);
								currentState.sequenceItr = --(toReturn.end());
								applyActionToState(rpg, currentState.currState, currActID, tsMatch);
								++(currentState.timelineItr);
							} else {
								if (debugFEP) cout << "End isn't executable\n";
								toReturn.erase(ciItr->second.sequenceItr, toReturn.end());
								blacklisted.insert(partner);
								currentState = ciItr->second;
								++(currentState.timelineItr); // skip forwards one - know this node will be blacklisted
			
							}
		
						} else {
	
							if (debugFEP) cout << "Corresponding start wasn't executed\n";
							++(currentState.timelineItr);
						}	
		
					} else {
						if (debugFEP) cout << "Corresponding start is blacklisted\n";
						++(currentState.timelineItr);
					}
		
				}
	
			} else {
				++(currentState.timelineItr);
			}
		}

	}

	{
		cout <<"\nExecutable path:\n";
		list<list<TimelinePoint*>::iterator >::iterator trItr = toReturn.begin();
		const list<list<TimelinePoint*>::iterator >::iterator trEnd = toReturn.end();

		int tn = 0;
		for (int i = 0; trItr != trEnd; ++i, ++trItr) {
			ScheduleNode* const currSN = *((*(*trItr))->after);
			if (currSN->tilNode()) {
				cout << i << ": TIL node " << tn << "\n";
				++tn;
			} else {
				cout << i << ": " << *(currSN->getOp()) << " " << (currSN->isStart() ? "start" : "end") << ", timestamp " << (*(*trItr))->timestamp << "\n";
			}
		}
	}

	return toReturn;

};

};
