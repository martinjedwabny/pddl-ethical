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

#include "landmarksanalysis.h"
#include "RPGBuilder.h"

#include <list>
#include <map>
#include <algorithm>
using std::list;
using std::pair;
using std::set_intersection;
using std::set_difference;
using std::endl;

#include "MILPRPG.h"


namespace Planner {
  
vector<int> LandmarksAnalysis::individualLandmarkFacts;
vector<int> LandmarksAnalysis::landmarkFacts;
vector<int> LandmarksAnalysis::factToLandmarkIndex;
vector<vector<vector<int> > > LandmarksAnalysis::disjunctiveLandmarks;

bool LandmarksAnalysis::disjunctiveZhuAndGivan = false;
bool LandmarksAnalysis::disjunctiveRegressionLandmarks = true;

void LandmarksAnalysis::addFactToState(MinimalState* theState, const int & fID)
{
    static int currLID;
    
    currLID = factToLandmarkIndex[fID];
    if (currLID != -1) {
        theState->landmarkStatus[currLID] = true;
        //cout << "Landmark fact ID " << currLID << " corresponding to " << *(RPGBuilder::getLiteral(fID)) << " now been met\n";
    }
}

void LandmarksAnalysis::performSimpleGoalRegressionLandmarksAnalysis(const LiteralSet & initialState, const vector<double> & initialFluents,
                                                                     set<int> & landmarksFound, list<list<set<int> > > & disjunctiveLandmarksFound)
{

    const vector<list<Literal*> > & allPres = RPGBuilder::getProcessedStartPreconditions();
    
    
    set<int> seen;
    list<pair<int,bool> > toVisit;
    
    {
        const list<Literal*> & goals = RPGBuilder::getLiteralGoals();
        list<Literal*>::const_iterator gItr = goals.begin();
        const list<Literal*>::const_iterator gEnd = goals.end();
        
        for (; gItr != gEnd; ++gItr) {
            if (initialState.find(*gItr) != initialState.end()) continue;            
            const int fID = (*gItr)->getStateID();
            seen.insert(fID);
            toVisit.push_back(make_pair(fID, true));            
        }
        
    }
    
    while (!toVisit.empty()) {
        const int fID = toVisit.front().first;
        const bool isGoalFact = toVisit.front().second;
        toVisit.pop_front();
        
        
        
        
        set<int> unionPres;
        
        list<set<int> > disjunctive;
        
        bool haveAnEmptyDLOption = false;
        
        bool firstTime = true;
        
        {
            const list<pair<int, VAL::time_spec> > & achievers = RPGBuilder::getEffectsToActions()[fID];
            set<int> isectPres;
        
            //cout << "Working backwards from " << *(RPGBuilder::getLiteral(fID)) << " - has " << achievers.size() << " achievers\n";
            
            list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
            const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();
            
            
            for (; accItr != accEnd; ++accItr) {
                
                set<int> pres;
                {
                    const list<Literal*> & preList = allPres[accItr->first];
                    list<Literal*>::const_iterator pItr = preList.begin();
                    const list<Literal*>::const_iterator pEnd = preList.end();
                    for (; pItr != pEnd; ++pItr) {
                        if (initialState.find(*pItr) == initialState.end()) {
                            pres.insert((*pItr)->getStateID());
                        }
                    }
                }
                
                disjunctive.push_back(pres);
                
                haveAnEmptyDLOption = (haveAnEmptyDLOption || pres.empty());
                
                if (firstTime) {
                    isectPres = pres;
                    unionPres = pres;
                    firstTime = false;
                } else {
                    unionPres.insert(pres.begin(), pres.end());
                    
                    set<int> result;
                    set_intersection(pres.begin(), pres.end(), isectPres.begin(), isectPres.end(), insert_iterator<set<int> >(result, result.begin()));
                    result.swap(isectPres);
                }
                
            }

                
            set<int> novel;
            set_difference(isectPres.begin(), isectPres.end(), seen.begin(), seen.end(), insert_iterator<set<int> >(novel, novel.begin()));
            
            seen.insert(novel.begin(), novel.end());
            {
                set<int>::const_iterator nItr = novel.begin();
                const set<int>::const_iterator nEnd = novel.end();
                
                for (; nItr != nEnd; ++nItr) {
                    toVisit.push_back(make_pair(*nItr,false));
                }
            }   
            
            /*if (!novel.empty()) {
                set<int>::const_iterator nItr = novel.begin();
                const set<int>::const_iterator nEnd = novel.end();
                for (; nItr != nEnd; ++nItr) {
                    cout << "All achievers require " << *(RPGBuilder::getLiteral(*nItr)) << " as a precondition\n";
                }
            }*/
        }
        
        if (!isGoalFact) {
            landmarksFound.insert(fID);
        }

        map<int, pair<set<int>,bool> > landmarksForFact;
        
        bool pushed = false;
        
        bool firstDLGroup = true;
        set<int> factsThatMustPrecedeDLGroup;
        
        if (!haveAnEmptyDLOption && disjunctiveRegressionLandmarks) {
            list<set<int> >::const_iterator dlItr = disjunctive.begin();
            const list<set<int> >::const_iterator dlEnd = disjunctive.end();
            
            for (; dlItr != dlEnd; ++dlItr) {
                
                set<int> factsThatMustPrecedeThisDLOption;
                
                set<int>::const_iterator upItr = dlItr->begin();
                const set<int>::const_iterator upEnd = dlItr->end();
                
                for (; upItr != upEnd; ++upItr) {
                    
                    pair<map<int, pair<set<int>,bool> >::iterator,bool> insPair = landmarksForFact.insert(make_pair(*upItr, make_pair(set<int>(),false)));
                    
                    if (insPair.second) {
                    
                        const list<pair<int, VAL::time_spec> > & achievers = RPGBuilder::getEffectsToActions()[*upItr];
                        
                        bool firstTime = true;
                        
                        list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
                        const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();
                        
                        for (; accItr != accEnd; ++accItr) {
                            

                            set<int> pres;
                            {
                                const list<Literal*> & preList = allPres[accItr->first];
                                list<Literal*>::const_iterator pItr = preList.begin();
                                const list<Literal*>::const_iterator pEnd = preList.end();
                                for (; pItr != pEnd; ++pItr) {
                                    if (initialState.find(*pItr) == initialState.end()) {
                                        pres.insert((*pItr)->getStateID());
                                    }
                                }
                            }
                            
                            if (firstTime) {
                                insPair.first->second.first.swap(pres);
                                firstTime = false;
                            } else {
                                set<int> result;
                                set_intersection(pres.begin(), pres.end(), insPair.first->second.first.begin(), insPair.first->second.first.end(), insert_iterator<set<int> >(result, result.begin()));
                                result.swap(insPair.first->second.first);
                                
                                if (insPair.first->second.first.empty()) break;
                            }
                        }                        
                    }
                    
                    factsThatMustPrecedeThisDLOption.insert(insPair.first->second.first.begin(), insPair.first->second.first.end());
                }
                
                if (firstDLGroup) {
                    factsThatMustPrecedeDLGroup = factsThatMustPrecedeThisDLOption;
                    firstDLGroup = false;
                } else {
                    set<int> result;
                    set_intersection(factsThatMustPrecedeThisDLOption.begin(), factsThatMustPrecedeThisDLOption.end(),
                                     factsThatMustPrecedeDLGroup.begin(), factsThatMustPrecedeDLGroup.end(),
                                     insert_iterator<set<int> >(result, result.begin()));
                    result.swap(factsThatMustPrecedeDLGroup);
                }
                
                {
                    if (!pushed) {
                        disjunctiveLandmarksFound.push_back(list<set<int> >());
                        pushed = true;
                    }
                    disjunctiveLandmarksFound.back().push_back(*dlItr);
                    disjunctiveLandmarksFound.back().back().insert(factsThatMustPrecedeThisDLOption.begin(), factsThatMustPrecedeThisDLOption.end());
                }
            }
            

        }
        
        {
            set<int> novel;
            set_difference(factsThatMustPrecedeDLGroup.begin(), factsThatMustPrecedeDLGroup.end(), seen.begin(), seen.end(), insert_iterator<set<int> >(novel, novel.begin()));
            
            seen.insert(novel.begin(), novel.end());
            {
                set<int>::const_iterator nItr = novel.begin();
                const set<int>::const_iterator nEnd = novel.end();
                
                for (; nItr != nEnd; ++nItr) {
                    toVisit.push_back(make_pair(*nItr,false));
                }
            }
        }
        
        
    }
    
    
    
}

/** Flag which causes checks at several steps that the disjunctive landmark sets listed contain all the disjunctive landmarks too. */
static bool expensiveSanityChecking = false;


/** Class describing the landmarks that must be satisfied to attain a given fact (used in the Zhu & Givan Analysis). */
struct LandmarkForFact {
  
    /** Whether the fact has been reached yet. */
    bool factReached;

    /** Whether the fact can be reached by an achiever with no landmark predecessors. */
    bool landmarkFreeOption;
    
    /** Definite landmarks prior to this fact. */
    set<int> conjunctiveLandmarks;
    
    /** Disjunctive landmarks prior to this fact.
     *
     *  Each pair denotes a disjunctive option, and consists of:
     *  - first, a conjunct of landmarks if this option is to be used
     *  - second, a conjunct of landmarks if this option is to be used, less those in <code>conjunctiveLandmarks</code>.
     */
    map<set<int>, set<int> > disjunctiveLandmarks;   
        
    /** Default constructor: fact is not yet reached. */
    LandmarkForFact()
        : factReached(false), landmarkFreeOption(false) {
    }
    
    /** Mark fact as reached, with no predecessor landmarks. */
    void reachedWithNoPreconditions() {
        factReached = true;
        landmarkFreeOption = true;
    }    

    /** Internal function - remove common (conjunctive) landmarks from disjunctive landmark sets. */
    void stripConjunctiveFromDisjunctive() {
        const bool emptyConjunction = conjunctiveLandmarks.empty();
        
        map<set<int>, set<int> >::iterator sItr = disjunctiveLandmarks.begin();
        const map<set<int>, set<int> >::iterator sEnd = disjunctiveLandmarks.end();
        
        for (; sItr != sEnd; ++sItr) {
            if (emptyConjunction) {
                sItr->second = sItr->first;
            } else {
                sItr->second.clear();
                
                std::set_difference(sItr->first.begin(), sItr->first.end(),
                                    conjunctiveLandmarks.begin(), conjunctiveLandmarks.end(),
                                    insert_iterator<set<int> >(sItr->second, sItr->second.begin()));
                                    
                if (expensiveSanityChecking) {
                    set<int> diff;
                    
                    std::set_difference(conjunctiveLandmarks.begin(), conjunctiveLandmarks.end(),
                                        sItr->first.begin(), sItr->first.end(),
                                        insert_iterator<set<int> >(diff, diff.begin()));
                    
                    assert(diff.empty());
                }
            }
        }                
    }
    
    /** Provide another landmark path to the current fact. */
    bool provideAlternative(const LandmarkForFact & newLandmarkSet) {
        
        static const bool debug = false;
        
        if (landmarkFreeOption) {
            if (debug) {
                cout << "Currently have a landmark-free option, disregarding alternatives\n";
            }
            return false;
        }
        
        if (newLandmarkSet.landmarkFreeOption) {
            if (debug) {
                cout << "Have been provided with a landmark-free option, clearing alternatives\n";
            }
            conjunctiveLandmarks.clear();
            disjunctiveLandmarks.clear();
            landmarkFreeOption = true;
            return true;
        }
        
        if (newLandmarkSet.conjunctiveLandmarks == conjunctiveLandmarks) {
            
            if (!LandmarksAnalysis::disjunctiveZhuAndGivan) {
                return false;
            }
            
            if (debug) {
                cout << "Conjunctive landmarks given are the same as those already recorded\n";
            }
            if (newLandmarkSet.disjunctiveLandmarks.empty()) {                
                if (!disjunctiveLandmarks.empty()) {
                    disjunctiveLandmarks.insert(make_pair(newLandmarkSet.conjunctiveLandmarks, set<int>()));
                    return true;
                }
                return false;
            } else {
                const unsigned int oldSize = disjunctiveLandmarks.size();
                if (disjunctiveLandmarks.empty()) {
                    disjunctiveLandmarks.insert(make_pair(newLandmarkSet.conjunctiveLandmarks, set<int>()));
                }
                disjunctiveLandmarks.insert(newLandmarkSet.disjunctiveLandmarks.begin(),
                                            newLandmarkSet.disjunctiveLandmarks.end());
                return (oldSize < disjunctiveLandmarks.size());
            }            
        }                
        
        if (conjunctiveLandmarks.empty()) {
            if (!LandmarksAnalysis::disjunctiveZhuAndGivan) {
                return false;
            }
            
            if (debug) {
                cout << "Conjunctive landmark record is empty, adding as disjunctive\n";
            }
            
            const unsigned int oldSize = disjunctiveLandmarks.size();
            if (newLandmarkSet.disjunctiveLandmarks.empty()) {
                disjunctiveLandmarks.insert(make_pair(newLandmarkSet.conjunctiveLandmarks, newLandmarkSet.conjunctiveLandmarks));
            } else if (newLandmarkSet.conjunctiveLandmarks.empty()) {
                disjunctiveLandmarks.insert(newLandmarkSet.disjunctiveLandmarks.begin(), newLandmarkSet.disjunctiveLandmarks.end());                
            } else {
                
                map<set<int>,set<int> >::iterator insItr = disjunctiveLandmarks.end();
                
                map<set<int>,set<int> >::const_iterator dItr = newLandmarkSet.disjunctiveLandmarks.begin();
                const map<set<int>,set<int> >::const_iterator dEnd = newLandmarkSet.disjunctiveLandmarks.end();
                
                for (; dItr != dEnd; ++dItr) {
                    set<int> copied = dItr->first;
                    copied.insert(newLandmarkSet.conjunctiveLandmarks.begin(), newLandmarkSet.conjunctiveLandmarks.end());
                    insItr = disjunctiveLandmarks.insert(insItr, make_pair(copied,copied));
                }
            }
            return (oldSize < disjunctiveLandmarks.size());
        }
        
        if (debug) {
            cout << "Intersecting conjunctive preconditions\n";
        }
        const unsigned int oldConjSize = conjunctiveLandmarks.size();
        set<int> isect;
        
        std::set_intersection(conjunctiveLandmarks.begin(), conjunctiveLandmarks.end(),
                              newLandmarkSet.conjunctiveLandmarks.begin(), newLandmarkSet.conjunctiveLandmarks.end(),
                              insert_iterator<set<int> >(isect, isect.begin()));
        
        if (!LandmarksAnalysis::disjunctiveZhuAndGivan) {
            conjunctiveLandmarks.swap(isect);
            return (conjunctiveLandmarks.size() < oldConjSize);
        }
        
        
        set<int> lostFromTheOther;
        
        std::set_difference(newLandmarkSet.conjunctiveLandmarks.begin(), newLandmarkSet.conjunctiveLandmarks.end(),
                            isect.begin(), isect.end(),
                            insert_iterator<set<int> >(lostFromTheOther, lostFromTheOther.begin()));
                              
        const unsigned int oldSize = disjunctiveLandmarks.size();
          
        if (disjunctiveLandmarks.empty()) {
            disjunctiveLandmarks.insert(make_pair(conjunctiveLandmarks, conjunctiveLandmarks));            
        }

        isect.swap(conjunctiveLandmarks);
        
        if (newLandmarkSet.disjunctiveLandmarks.empty()) {
            disjunctiveLandmarks.insert(make_pair(newLandmarkSet.conjunctiveLandmarks, newLandmarkSet.conjunctiveLandmarks));
        } else if (lostFromTheOther.empty()) {
            if (debug) {
                cout << "Intersection of common conjunctive landmarks subsumes those in the other set; can copy across its disjunctive options directly\n";
            }
            disjunctiveLandmarks.insert(newLandmarkSet.disjunctiveLandmarks.begin(),
                                        newLandmarkSet.disjunctiveLandmarks.end());
        } else {

            map<set<int>,set<int> >::iterator insItr = disjunctiveLandmarks.end();
            
            map<set<int>,set<int> >::const_iterator dItr = newLandmarkSet.disjunctiveLandmarks.begin();
            const map<set<int>,set<int> >::const_iterator dEnd = newLandmarkSet.disjunctiveLandmarks.end();
            
            for (; dItr != dEnd; ++dItr) {
                set<int> copied = dItr->first;
                copied.insert(lostFromTheOther.begin(), lostFromTheOther.end());
                insItr = disjunctiveLandmarks.insert(insItr, make_pair(copied,copied));
            }
        }
        
        stripConjunctiveFromDisjunctive();
        
        return (oldSize < disjunctiveLandmarks.size());
    }
    
    void addFactToConjunct(const int & fact) {
        conjunctiveLandmarks.insert(fact);                                    
        landmarkFreeOption = false;
        
        map<set<int>,set<int> > oldDisjunctiveLandmarks;
        oldDisjunctiveLandmarks.swap(disjunctiveLandmarks);
        
        map<set<int>,set<int> >::iterator insItr = disjunctiveLandmarks.end();
        
        map<set<int>,set<int> >::const_iterator dItr = oldDisjunctiveLandmarks.begin();
        const map<set<int>,set<int> >::const_iterator dEnd = oldDisjunctiveLandmarks.end();
        
        for (; dItr != dEnd; ++dItr) {
            set<int> withExtra(dItr->first);
            withExtra.insert(fact);
            insItr = disjunctiveLandmarks.insert(insItr, make_pair(withExtra, withExtra));
            
            if (expensiveSanityChecking) {
                set<int> diff;
                
                std::set_difference(conjunctiveLandmarks.begin(), conjunctiveLandmarks.end(),
                                    insItr->first.begin(), insItr->first.end(),
                                    insert_iterator<set<int> >(diff, diff.begin()));
                                    
                assert(diff.empty());
            }
        }
        
        stripConjunctiveFromDisjunctive();
        
    }
    
    template <typename T>
    void addFactsToConjunct(const T & fBegin, const T & fEnd) {
        
        T fItr = fBegin;
        for (; fItr != fEnd; ++fItr) {
            conjunctiveLandmarks.insert(*fItr);                                    
            landmarkFreeOption = false;
        }
        
        map<set<int>,set<int> > oldDisjunctiveLandmarks;
        oldDisjunctiveLandmarks.swap(disjunctiveLandmarks);
        
        map<set<int>,set<int> >::iterator insItr = disjunctiveLandmarks.end();
        
        map<set<int>,set<int> >::const_iterator dItr = oldDisjunctiveLandmarks.begin();
        const map<set<int>,set<int> >::const_iterator dEnd = oldDisjunctiveLandmarks.end();
        
        for (; dItr != dEnd; ++dItr) {
            set<int> withExtra(dItr->first);
            withExtra.insert(conjunctiveLandmarks.begin(), conjunctiveLandmarks.end());
            insItr = disjunctiveLandmarks.insert(insItr, make_pair(withExtra, withExtra));
            
            if (expensiveSanityChecking) {
                set<int> diff;
                
                std::set_difference(conjunctiveLandmarks.begin(), conjunctiveLandmarks.end(),
                                    insItr->first.begin(), insItr->first.end(),
                                    insert_iterator<set<int> >(diff, diff.begin()));
                                    
                assert(diff.empty());
            }
        }
                
        stripConjunctiveFromDisjunctive();
        
    }
    
    /** Create a landmark record, based on a given fact and the landmarks before it.
      *
      * This function is used when taking the union of the landmarks needed to reach an
      * action.  The first precondition fact, and its landmark record, are passed to this
      * constructor.  If the fact isn't -1 (i.e. if it wasn't known to be true in the initial
      * state) then it added as a conjunctive landmark.
      * 
      * @param fact           A precondition fact which is a landmark (or -1 if fact is true initially)
      * @param itsConditions  The landmarks needed to reach one of the action's preconditions
      */
    LandmarkForFact(const int & fact, const LandmarkForFact & itsConditions)
        : factReached(true), landmarkFreeOption(itsConditions.landmarkFreeOption),
          conjunctiveLandmarks(itsConditions.conjunctiveLandmarks),
          disjunctiveLandmarks(itsConditions.disjunctiveLandmarks) {
          
          if (fact != -1) {
              addFactToConjunct(fact);
          } else {
              if (expensiveSanityChecking) {
                  map<set<int>,set<int> >::const_iterator dItr = disjunctiveLandmarks.begin();
                  const map<set<int>,set<int> >::const_iterator dEnd = disjunctiveLandmarks.end();
                  
                  for (; dItr != dEnd; ++dItr) {
                      set<int> diff;
                      
                      std::set_difference(conjunctiveLandmarks.begin(), conjunctiveLandmarks.end(),
                                          dItr->first.begin(), dItr->first.end(),
                                          insert_iterator<set<int> >(diff, diff.begin()));
                                          
                      assert(diff.empty());
                  }
              }
          }
    }
    
    /** Take the union of the recorded landmarks with those for another fact.
      * 
      * This function is used when taking the union of the landmarks needed to reach an
      * action.  After creating a <code>LandmarkForFact</code> object for the first precondition,
      * this function is used to take the union of that and the other landmarks for the other
      * precondition facts.  If the fact isn't -1 (i.e. if it wasn't known to be true in the initial
      * state) then it added as a conjunctive landmark, along with the landmarks given to reach it.
      * 
      * @param fact           If not <code>-1</code>, a precondition fact which becomes a landmark
      * @param itsConditions  The landmarks needed to reach one of the action's preconditions
     */
    void unionWith(const int & otherFact, const LandmarkForFact & other) {
        
        if (otherFact != -1) {
            conjunctiveLandmarks.insert(otherFact);
            landmarkFreeOption = false;
        } else {
            if (expensiveSanityChecking) {
                map<set<int>,set<int> >::const_iterator dItr = disjunctiveLandmarks.begin();
                const map<set<int>,set<int> >::const_iterator dEnd = disjunctiveLandmarks.end();
                
                for (; dItr != dEnd; ++dItr) {
                    set<int> diff;
                    
                    std::set_difference(conjunctiveLandmarks.begin(), conjunctiveLandmarks.end(),
                                        dItr->first.begin(), dItr->first.end(),
                                        insert_iterator<set<int> >(diff, diff.begin()));
                                        
                    assert(diff.empty());
                }
            }
        }

        disjunctiveLandmarks.insert(other.disjunctiveLandmarks.begin(),
                                    other.disjunctiveLandmarks.end());
                                    
        addFactsToConjunct(other.conjunctiveLandmarks.begin(), other.conjunctiveLandmarks.end());
        
                                    
    }
};

ostream & operator <<(ostream & o, const LandmarkForFact & l) {
    
    if (!l.factReached) {
        o << "Fact not reached\n";
        return o;
    }
    
    if (l.landmarkFreeOption) {
        o << "No landmarks prior to this fact\n";
        return o;
    }
    
    if (!l.conjunctiveLandmarks.empty()) {
        o << "Must have all of: ";
        
        set<int>::const_iterator lItr = l.conjunctiveLandmarks.begin();
        const set<int>::const_iterator lEnd = l.conjunctiveLandmarks.end();
        
        for (bool second=false; lItr != lEnd; ++lItr, second=true) {
            if (second) {
                o << ", ";
            }
            o << *(RPGBuilder::getLiteral(*lItr));
        }
        o << endl;
    }
    if (!l.disjunctiveLandmarks.empty()) {
        if (l.conjunctiveLandmarks.empty()) {
            o << "Must have one of: ";
        } else {
            o << "...and one of: [";
        }
        
        map<set<int>, set<int> >::const_iterator sItr = l.disjunctiveLandmarks.begin();
        const map<set<int>, set<int> >::const_iterator sEnd = l.disjunctiveLandmarks.end();
        
        for (; sItr != sEnd; ++sItr) {
            o << " {";
            
            set<int>::const_iterator lItr = sItr->second.begin();
            const set<int>::const_iterator lEnd = sItr->second.end();
            
            for (bool second=false; lItr != lEnd; ++lItr, second=true) {
                if (second) {
                    o << ", ";
                }
                o << *(RPGBuilder::getLiteral(*lItr));
            }
            o << "}";
        }
        
        o << " ]\n";
    }
    
    return o;
}

void LandmarksAnalysis::performZhuAndGivanLandmarksAnalysis(const LiteralSet & initialState, const vector<double> & initialFluents,
                                                            set<int> & landmarksFound, list<list<set<int> > > & disjunctiveLandmarksFound)
{

    static const bool debug = false;
    
    
    vector<int> preconditionCounts(RPGBuilder::getUnsatisfiedStartPreconditionCounts());
    vector<int> numericPreconditionCounts(RPGBuilder::getUnsatisfiedStartNumericPreconditionCounts());
    
    // Landmarks needed to reach a given fact.  If the bool is false, the fact hasn't been reached yet.
    vector<LandmarkForFact> landmarksToReachFact(RPGBuilder::getLiteralCount());

    set<int> factLayer;
    set<int> factsWithFewerLandmarks;
    
    {
        // Any fact true initially has no landmarks to be able to reach it.
        
        LiteralSet::const_iterator fItr = initialState.begin();
        const LiteralSet::const_iterator fEnd = initialState.end();
        
        for (; fItr != fEnd; ++fItr) {
            landmarksToReachFact[(*fItr)->getStateID()].reachedWithNoPreconditions();
            factLayer.insert((*fItr)->getStateID());
        }
    }
    
    {
        const list<pair<int, VAL::time_spec> > & preconditionlessActions = RPGBuilder::getPreconditionlessActions();
        
        list<pair<int, VAL::time_spec> >::const_iterator aItr = preconditionlessActions.begin();
        const list<pair<int, VAL::time_spec> >::const_iterator aEnd = preconditionlessActions.end();
        
        for (; aItr != aEnd; ++aItr) {
            if (aItr->second != VAL::E_AT_START) continue;
            
            const list<Literal*> & added = RPGBuilder::getStartAddEffects()[aItr->first];
            
            list<Literal*>::const_iterator fItr = added.begin();
            const list<Literal*>::const_iterator fEnd = added.end();
            
            for (; fItr != fEnd; ++fItr) {
                landmarksToReachFact[(*fItr)->getStateID()].reachedWithNoPreconditions();
                factLayer.insert((*fItr)->getStateID());
            }
        }
        
    }
    
    {
        const vector<list<pair<int, VAL::time_spec> > > & rpgNumericToAction = RPGBuilder::getProcessedRPGNumericPreconditionsToActions();
        
        const int lim = rpgNumericToAction.size();
        
        for (int pre = 0; pre < lim; ++pre) {
            list<pair<int, VAL::time_spec> >::const_iterator aItr = rpgNumericToAction[pre].begin();
            const list<pair<int, VAL::time_spec> >::const_iterator aEnd = rpgNumericToAction[pre].end();
            
            for (; aItr != aEnd; ++aItr) {
                if (aItr->second != VAL::E_AT_START) continue;
                            
                if (!(--(numericPreconditionCounts[aItr->first])) && !preconditionCounts[aItr->first]) { 
                    const list<Literal*> & added = RPGBuilder::getStartAddEffects()[aItr->first];
                    
                    list<Literal*>::const_iterator fItr = added.begin();
                    const list<Literal*>::const_iterator fEnd = added.end();
                    
                    for (; fItr != fEnd; ++fItr) {
                        landmarksToReachFact[(*fItr)->getStateID()].reachedWithNoPreconditions();
                        factLayer.insert((*fItr)->getStateID());
                    }
                }
            }
        }
    }
    
    const vector<list<pair<int, VAL::time_spec> > > & presToActions = RPGBuilder::getProcessedPreconditionsToActions();
    
    const vector<list<Literal*> > & actionPreconditions = RPGBuilder::getProcessedStartPreconditions();
    
    // Now, factLayer contains all the facts true initially, or reached by (propositionally) preconditionless actions
    
    while (!factLayer.empty() || !factsWithFewerLandmarks.empty()) {
        
        if (debug) {
            cout << ">>> Next layer\n";
        }
        
        set<int> previousFactLayer;
        set<int> previousFactsWithFewerLandmarks;
        
        previousFactLayer.swap(factLayer);
        previousFactsWithFewerLandmarks.swap(factsWithFewerLandmarks);
        
        for (int pass = 0; pass < 2; ++pass) {
            
            const set<int> & loopOver = (pass ? previousFactsWithFewerLandmarks : previousFactLayer);
            
            if (debug && !loopOver.empty()) {
                if (pass) {
                    cout << "Facts that now need fewer landmarks:\n";
                } else {
                    cout << "Facts that are true:\n";
                }
            }
            
            set<int>::const_iterator factLayerItr = loopOver.begin();
            const set<int>::const_iterator factLayerEnd = loopOver.end();
            
            for (; factLayerItr != factLayerEnd; ++factLayerItr) {
                                
                if (debug) {
                    cout << "- " << *(RPGBuilder::getLiteral(*factLayerItr)) << endl;
                }
                
                list<pair<int, VAL::time_spec> >::const_iterator actItr = presToActions[*factLayerItr].begin();
                const list<pair<int, VAL::time_spec> >::const_iterator actEnd = presToActions[*factLayerItr].end();
                
                for (; actItr != actEnd; ++actItr) {
                    assert(actItr->second == VAL::E_AT_START);
                                
                    if (!pass) {
                        if (--(preconditionCounts[actItr->first])) {
                            continue;
                        }
                    }
                    
                    if (debug) {
                        cout << " * Can apply " << *(RPGBuilder::getInstantiatedOp(actItr->first)) << endl;
                    }
                    
                    LandmarkForFact landmarksOfThisAction;
                    bool setIsDefined = false;
                    
                    const list<Literal*> & added = RPGBuilder::getStartAddEffects()[actItr->first];
                    
                    list<Literal*>::const_iterator addItr = added.begin();
                    const list<Literal*>::const_iterator addEnd = added.end();
                    
                    for (; addItr != addEnd; ++addItr) {
                        
                        const int fID = (*addItr)->getStateID();
                        
                        if (landmarksToReachFact[fID].landmarkFreeOption) {
                            // no point considering disjunctive landmarks preceding facts that can be reached
                            // with an option requiring none at all
                            continue;
                        }
                        
                        // define precondition set of action here so it's only done if new facts or non-empty-landmark-set facts
                        // are reached
                        
                        if (!setIsDefined) {
                            list<Literal*>::const_iterator pItr = actionPreconditions[actItr->first].begin();
                            const list<Literal*>::const_iterator pEnd = actionPreconditions[actItr->first].end();

                            if (pItr == pEnd) {
                                if (debug) {
                                    cout << "When defining landmarks to reach " << *(RPGBuilder::getInstantiatedOp(actItr->first)) << ", it has no preconditions\n";
                                }
                                landmarksOfThisAction.reachedWithNoPreconditions();
                            } else {
                                bool anyNonInitial = false;
                                if (initialState.find(*pItr) == initialState.end() && landmarksFound.find((*pItr)->getStateID()) == landmarksFound.end()) {                                
                                    landmarksOfThisAction = LandmarkForFact((*pItr)->getStateID(), landmarksToReachFact[(*pItr)->getStateID()]);                                    
                                    anyNonInitial = true;
                                } else {
                                    landmarksOfThisAction = LandmarkForFact(-1, landmarksToReachFact[(*pItr)->getStateID()]);
                                }
                                
                                for (; pItr != pEnd; ++pItr) {
                                    if (initialState.find(*pItr) == initialState.end() && landmarksFound.find((*pItr)->getStateID()) == landmarksFound.end()) {
                                        landmarksOfThisAction.unionWith((*pItr)->getStateID(), landmarksToReachFact[(*pItr)->getStateID()]);
                                        anyNonInitial = true;
                                    } else {
                                        landmarksOfThisAction.unionWith(-1, landmarksToReachFact[(*pItr)->getStateID()]);
                                    }
                                }
                                if (debug) {
                                    if (anyNonInitial) {
                                        cout << "Not all preconditions of " << *(RPGBuilder::getInstantiatedOp(actItr->first)) << " are true initially\n";
                                    } else {
                                        cout << "All preconditions of " << *(RPGBuilder::getInstantiatedOp(actItr->first)) << " are true initially\n";
                                    }
                                }
                            }
                            setIsDefined = true;
                        }
                        
                        if (landmarksToReachFact[fID].factReached) {
                            
                            if (debug) {
                                cout << "Revisiting landmarks to reach " << *(RPGBuilder::getLiteral(fID)) << ", used to be:\n";
                                cout << landmarksToReachFact[fID];
                            }

                            
                            if (landmarksToReachFact[fID].provideAlternative(landmarksOfThisAction)) {
                                factsWithFewerLandmarks.insert(fID);
                                if (debug) {
                                    cout << "Now have fewer landmarks to reach " << *(RPGBuilder::getLiteral(fID)) << "\n";
                                    cout << "Merged in:\n";
                                    cout << landmarksOfThisAction;
                                    cout << "Now must have:\n";
                                    cout << landmarksToReachFact[fID];
                                }
                            }
                                                        
                        } else {
                            landmarksToReachFact[fID] = landmarksOfThisAction;
                            factLayer.insert(fID);                  
                            if (debug) {
                                cout << "First time to have reached " << *(RPGBuilder::getLiteral(fID)) << "\n";
                                cout << landmarksToReachFact[fID];
                            }
                        }
                        
                    }
                    
                }
            }
        }
    }
    
    {
        const list<Literal*> & goals = RPGBuilder::getLiteralGoals();
        
        {
            // First, get the non-disjunctive landmarks
            list<Literal*>::const_iterator gItr = goals.begin();
            const list<Literal*>::const_iterator gEnd = goals.end();
            
            for (; gItr != gEnd; ++gItr) {
                const int fID = (*gItr)->getStateID();
                if (!landmarksToReachFact[fID].factReached) {
                    cout << "Zhu & Givan Landmarks Analysis has shown problem to be unsolvable: cannot reach goal " << *(*gItr) << endl;
                    exit(0);
                }
                
                cout << "Zhu & Givan Landmarks before " << *(*gItr) << ":\n";
                cout << landmarksToReachFact[fID];
                
                landmarksFound.insert(landmarksToReachFact[fID].conjunctiveLandmarks.begin(), landmarksToReachFact[fID].conjunctiveLandmarks.end());            
            }
        }
        
        {
            list<Literal*>::const_iterator gItr = goals.begin();
            const list<Literal*>::const_iterator gEnd = goals.end();
            
            for (; gItr != gEnd; ++gItr) {
                const int fID = (*gItr)->getStateID();
            
                bool pushed = false;
                bool shouldPop = false;
                
                map<set<int>, set<int> >::const_iterator dItr = landmarksToReachFact[fID].disjunctiveLandmarks.begin();
                const map<set<int>, set<int> >::const_iterator dEnd = landmarksToReachFact[fID].disjunctiveLandmarks.end();
                
                for (; dItr != dEnd; ++dItr) {
                    if (dItr->second.empty()) {
                        shouldPop = pushed;
                        continue;
                    }
                    
                    if (!pushed) {
                        disjunctiveLandmarksFound.push_back(list<set<int> >());
                        pushed = true;
                    }
                    disjunctiveLandmarksFound.back().push_back(dItr->second);
                }
                
                if (shouldPop) {
                    disjunctiveLandmarksFound.pop_back();
                }
            }
        }
    }
}

    
bool isASubsetOf(const set<int> & a, const set<int> & b) {

    if (a.size() > b.size()) return false;
    
    set<int>::const_iterator aItr = a.begin();
    const set<int>::const_iterator aEnd = a.end();

    set<int>::const_iterator bItr = b.begin();
    const set<int>::const_iterator bEnd = b.end();
    
    while (aItr != aEnd && bItr != bEnd) {
        if (*aItr < *bItr) {
            return false;
        } else if (*bItr < *aItr) {
            ++bItr;
        } else {
            ++aItr;
            ++bItr;
        }
    }
    
    return (aItr == aEnd);

}

void LandmarksAnalysis::performLandmarksAnalysis()
{
    LiteralSet initialState;
    vector<double> initialFluents;
    
    RPGBuilder::getInitialState(initialState,initialFluents);
    
    factToLandmarkIndex.resize(RPGBuilder::getLiteralCount(), -1);
    
    set<int> seen;
    list<list<set<int> > > disjunctiveLandmarksFound;
    
    performSimpleGoalRegressionLandmarksAnalysis(initialState, initialFluents, seen, disjunctiveLandmarksFound);
    performZhuAndGivanLandmarksAnalysis(initialState, initialFluents, seen, disjunctiveLandmarksFound);

    {
        const list<Literal*> & goals = RPGBuilder::getLiteralGoals();
        list<Literal*>::const_iterator gItr = goals.begin();
        const list<Literal*>::const_iterator gEnd = goals.end();
        
        for (; gItr != gEnd; ++gItr) {
            seen.erase((*gItr)->getStateID());
        }
    }
    
    const int lCount = seen.size();
    
    individualLandmarkFacts.resize(lCount);
    
    set<int>::const_iterator sItr = seen.begin();
    
    for (int ll = 0; ll < lCount; ++ll, ++sItr) {        
        factToLandmarkIndex[*sItr] = ll;
        individualLandmarkFacts[ll] = *sItr;
        cout << *(RPGBuilder::getLiteral(*sItr)) << " is individual landmark fact " << ll << endl;
    }
    
    list<set<set<int> > > tmpDisjunctiveLandmarks;
    
    {
        list<list<set<int> > >::const_iterator dlItr = disjunctiveLandmarksFound.begin();
        const list<list<set<int> > >::const_iterator dlEnd = disjunctiveLandmarksFound.end();
        
        for (; dlItr != dlEnd; ++dlItr) {
            
            tmpDisjunctiveLandmarks.push_back(set<set<int> >());
            
            set<set<int> > & newGroup = tmpDisjunctiveLandmarks.back();
            
            list<set<int> >::const_iterator lItr = dlItr->begin();
            const list<set<int> >::const_iterator lEnd = dlItr->end();
            
            for (; lItr != lEnd; ++lItr) {
                set<int> residual;
                
                std::set_difference(lItr->begin(), lItr->end(), seen.begin(), seen.end(),
                                    insert_iterator<set<int> >(residual, residual.end()));
                
                if (residual.empty()) {
                    newGroup.clear();
                    break;
                }
                
                if (newGroup.find(residual) != newGroup.end()) continue;
                
                bool insertIt = true;
                
                set<set<int> >::iterator ngItr = newGroup.begin();
                const set<set<int> >::iterator ngEnd = newGroup.end();
                
                while (ngItr != ngEnd) {
                    if (isASubsetOf(*ngItr, residual)) {
                        insertIt = false;
                        break;
                    }
                    if (isASubsetOf(residual, *ngItr)) {
                        const set<set<int> >::iterator ngDel = ngItr++;
                        newGroup.erase(ngDel);
                    } else {
                        ++ngItr;
                    }
                }
                
                if (insertIt) {
                    newGroup.insert(residual);
                }
            }
            
            if (newGroup.empty()) {
                tmpDisjunctiveLandmarks.pop_back();
            }
        }
    }

    
    set<int> extraFacts;
    
    {
        disjunctiveLandmarks.resize(tmpDisjunctiveLandmarks.size());
        
        list<set<set<int> > >::const_iterator dlItr = tmpDisjunctiveLandmarks.begin();
        const list<set<set<int> > >::const_iterator dlEnd = tmpDisjunctiveLandmarks.end();
        
        for (int d = 0; dlItr != dlEnd; ++dlItr, ++d) {
            
            disjunctiveLandmarks[d].resize(dlItr->size());
            
            set<set<int> >::const_iterator lItr = dlItr->begin();
            const set<set<int> >::const_iterator lEnd = dlItr->end();
            
            for (int m = 0; lItr != lEnd; ++lItr, ++m) {
                extraFacts.insert(lItr->begin(), lItr->end());
                disjunctiveLandmarks[d][m].insert(disjunctiveLandmarks[d][m].end(),
                                                  lItr->begin(), lItr->end());
            }
        }

    }
    
    const int extraCount = extraFacts.size();

    landmarkFacts.resize(extraCount + lCount);
    
    for (int l = 0; l < lCount; ++l) {
        landmarkFacts[l] = individualLandmarkFacts[l];
    }
    
    set<int>::const_iterator efItr = extraFacts.begin();
    const set<int>::const_iterator efEnd = extraFacts.end();
    
    for (int l = lCount; efItr != efEnd; ++efItr, ++l) {
        landmarkFacts[l] = *efItr;
        assert(factToLandmarkIndex[*efItr] == -1);
        factToLandmarkIndex[*efItr] = l;
        cout << *(RPGBuilder::getLiteral(*efItr)) << " is landmark fact " << l << endl;
    }
    
        
    const int dCount = disjunctiveLandmarks.size();
    
    cout << "Number of individual landmark facts: " << individualLandmarkFacts.size() << endl;
    cout << "Number of disjunctive landmark groups: " << dCount << endl;
    
    for (int d = 0; d < dCount; ++d) {
        cout << "Disjunctive landmark group " << d << ":";
        const int mCount = disjunctiveLandmarks[d].size();
        
        for (int m = 0; m < mCount; ++m) {
            cout << " {";
            const int tCount = disjunctiveLandmarks[d][m].size();
            for (int t = 0; t < tCount; ++t) {
                if (t) {
                    cout << ", ";
                }
                cout << *(RPGBuilder::getLiteral(disjunctiveLandmarks[d][m][t]));
            }
            cout << "}";
        }
        cout << endl;
    }
    
}


void LandmarksAnalysis::localLandmarksAnalysis(const MinimalState & factLayerZero,
                                               vector<int> * const propPrec, vector<int> * const numPrec,
                                               const bool & mustUseNumericActions,
                                               set<int> & localIndividualLandmarks,
                                               list<list<set<int> > > & localDisjunctiveLandmarks)
{

    const vector<list<Literal*> > & allPres = RPGBuilder::getProcessedStartPreconditions();
    
    
    set<int> seen;
    list<pair<int,bool> > toVisit;
    
    {
        const list<Literal*> & goals = RPGBuilder::getLiteralGoals();
        list<Literal*>::const_iterator gItr = goals.begin();
        const list<Literal*>::const_iterator gEnd = goals.end();
        
        int localfID;
        for (; gItr != gEnd; ++gItr) {
            
            localfID = (*gItr)->getStateID();
            if (factLayerZero.first.find(localfID) != factLayerZero.first.end()) continue;                        
            seen.insert(localfID);
            toVisit.push_back(make_pair(localfID, true));            
        }
                
    } 
    
    while (!toVisit.empty()) {
        const int fID = toVisit.front().first;
        const bool isGoalFact = toVisit.front().second;
        toVisit.pop_front();
        
        
        
        
        list<set<int> > disjunctive;
        set<int> unionPres;
        
        
        bool skipThisFact = false;
        bool haveAnEmptyDLOption = false;
        {
            bool firstTime = true;
            
            const list<pair<int, VAL::time_spec> > & achievers = RPGBuilder::getEffectsToActions()[fID];
            set<int> isectPres;
        
            //cout << "Working backwards from " << *(RPGBuilder::getLiteral(fID)) << " - has " << achievers.size() << " achievers\n";
            
            list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
            const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();
            
            
            for (; accItr != accEnd; ++accItr) {
                
                if ((*propPrec)[accItr->first] || (*numPrec)[accItr->first]) {
                    // achiever has unsatisfied preconditions, ignore it
                    continue;
                }
                
                if (mustUseNumericActions) {
                    if (!MILPRPG::actionNeedsNumerics(accItr->first)) {
                        // then we have an achiever for this fact that has no need for numerics
                        // and hence we do not include it in the LP
                        skipThisFact = true;
                        break;
                        
                    }
                }
                
                set<int> pres;
                {
                    const list<Literal*> & preList = allPres[accItr->first];
                    list<Literal*>::const_iterator pItr = preList.begin();
                    const list<Literal*>::const_iterator pEnd = preList.end();
                    
                    int localfID;
                    for (; pItr != pEnd; ++pItr) {
                        localfID = (*pItr)->getStateID();
                        if (factLayerZero.first.find(localfID) == factLayerZero.first.end()) {
                            pres.insert(localfID);
                        }
                    }
                }
                
                disjunctive.push_back(pres);
                
                haveAnEmptyDLOption = (haveAnEmptyDLOption || pres.empty());
                
                if (firstTime) {
                    isectPres = pres;
                    unionPres = pres;
                    firstTime = false;
                } else {
                    unionPres.insert(pres.begin(), pres.end());
                    set<int> result;
                    set_intersection(pres.begin(), pres.end(), isectPres.begin(), isectPres.end(), insert_iterator<set<int> >(result, result.begin()));
                    result.swap(isectPres);
                }
                
            }

                
            set<int> novel;
            set_difference(isectPres.begin(), isectPres.end(), seen.begin(), seen.end(), insert_iterator<set<int> >(novel, novel.begin()));
            
            seen.insert(novel.begin(), novel.end());
            
            {
                set<int>::const_iterator nItr = novel.begin();
                const set<int>::const_iterator nEnd = novel.end();
                
                for (; nItr != nEnd; ++nItr) {
                    toVisit.push_back(make_pair(*nItr,false));
                }
            }
            
            /*if (!novel.empty()) {
                set<int>::const_iterator nItr = novel.begin();
                const set<int>::const_iterator nEnd = novel.end();
                for (; nItr != nEnd; ++nItr) {
                    cout << "All achievers require " << *(RPGBuilder::getLiteral(*nItr)) << " as a precondition\n";
                }
            }*/
        }
        
        if (skipThisFact) {
            continue;
        }
        
        if (!isGoalFact) {
            localIndividualLandmarks.insert(fID);
        }

        map<int, pair<set<int>,bool> > landmarksForFact;
        
        bool pushed = false;
        bool doPop = false;
        
        bool firstDLGroup = true;
        set<int> factsThatMustPrecedeDLGroup;
        
        if (!haveAnEmptyDLOption) {
            list<set<int> >::const_iterator dlItr = disjunctive.begin();
            const list<set<int> >::const_iterator dlEnd = disjunctive.end();
            
            for (; dlItr != dlEnd; ++dlItr) {
                
                bool allNumericFree = true;
                set<int> factsThatMustPrecedeThisDLOption;
                
                set<int>::const_iterator upItr = dlItr->begin();
                const set<int>::const_iterator upEnd = dlItr->end();
                
                for (; upItr != upEnd; ++upItr) {
                    
                    pair<map<int, pair<set<int>,bool> >::iterator,bool> insPair = landmarksForFact.insert(make_pair(*upItr, make_pair(set<int>(),false)));
                    
                    if (insPair.second) {
                    
                        const list<pair<int, VAL::time_spec> > & achievers = RPGBuilder::getEffectsToActions()[*upItr];
                        
                        bool firstTime = true;
                        
                        list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
                        const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();
                        
                        for (; accItr != accEnd; ++accItr) {
                            
                            if ((*propPrec)[accItr->first] || (*numPrec)[accItr->first]) {
                                // achiever has unsatisfied preconditions, ignore it
                                continue;
                            }
                            
                            if (mustUseNumericActions) {
                                if (!MILPRPG::actionNeedsNumerics(accItr->first)) {
                                    // then we have an achiever for this fact that has no need for numerics
                                    // and hence we do not include it in the LP
                                    insPair.first->second.second = true;
                                    break;                                                                                    
                                }
                            }
                            
                            set<int> pres;
                            {
                                const list<Literal*> & preList = allPres[accItr->first];
                                list<Literal*>::const_iterator pItr = preList.begin();
                                const list<Literal*>::const_iterator pEnd = preList.end();
                                
                                int localfID;
                                for (; pItr != pEnd; ++pItr) {
                                    localfID = (*pItr)->getStateID();
                                    if (factLayerZero.first.find(localfID) == factLayerZero.first.end()) {
                                        pres.insert(localfID);
                                    }
                                }
                            }
                            
                            if (firstTime) {
                                insPair.first->second.first.swap(pres);
                                firstTime = false;
                            } else {
                                set<int> result;
                                set_intersection(pres.begin(), pres.end(), insPair.first->second.first.begin(), insPair.first->second.first.end(), insert_iterator<set<int> >(result, result.begin()));
                                result.swap(insPair.first->second.first);
                                
                                if (insPair.first->second.first.empty()) break;
                            }
                        }                        
                    }
                    
                    factsThatMustPrecedeThisDLOption.insert(insPair.first->second.first.begin(), insPair.first->second.first.end());
                    allNumericFree = (allNumericFree && insPair.first->second.second);
                }
                
                if (firstDLGroup) {
                    factsThatMustPrecedeDLGroup = factsThatMustPrecedeThisDLOption;
                    firstDLGroup = false;
                } else {
                    set<int> result;
                    set_intersection(factsThatMustPrecedeThisDLOption.begin(), factsThatMustPrecedeThisDLOption.end(),
                                     factsThatMustPrecedeDLGroup.begin(), factsThatMustPrecedeDLGroup.end(),
                                     insert_iterator<set<int> >(result, result.begin()));
                    result.swap(factsThatMustPrecedeDLGroup);
                }
                
                if (allNumericFree) {
                    doPop = pushed;
                } else if (!doPop) {
                    if (!pushed) {
                        localDisjunctiveLandmarks.push_back(list<set<int> >());
                        pushed = true;
                    }
                    localDisjunctiveLandmarks.back().push_back(*dlItr);
                    localDisjunctiveLandmarks.back().back().insert(factsThatMustPrecedeThisDLOption.begin(), factsThatMustPrecedeThisDLOption.end());
                }
            }
            
            if (doPop) {
                
                // we don't report disjunctive landmarks if one of them can be satisfied entirely by actions with no numeric side effects,
                // as that will lead to no numeric actions being introduced into the LP
                localDisjunctiveLandmarks.pop_back();
            }
        }
        
        {
            set<int> novel;
            set_difference(factsThatMustPrecedeDLGroup.begin(), factsThatMustPrecedeDLGroup.end(), seen.begin(), seen.end(), insert_iterator<set<int> >(novel, novel.begin()));
            
            seen.insert(novel.begin(), novel.end());
            {
                set<int>::const_iterator nItr = novel.begin();
                const set<int>::const_iterator nEnd = novel.end();
                
                for (; nItr != nEnd; ++nItr) {
                    toVisit.push_back(make_pair(*nItr,false));
                }
            }
        }
        
        
    }
    
    /*
    if (!localIndividualLandmarks.empty()) {
        cout << "Local landmarks:";
        set<int>::const_iterator llItr = localIndividualLandmarks.begin();
        const set<int>::const_iterator llEnd = localIndividualLandmarks.end();
        
        for (; llItr != llEnd; ++llItr) {
            cout << " " << *(RPGBuilder::getLiteral(*llItr));
        }
        cout << endl;
    }
    
    if (!localDisjunctiveLandmarks.empty()) {
        cout << "Local disjunctive landmarks:\n";
        list<list<set<int> > >::const_iterator dlItr = localDisjunctiveLandmarks.begin();
        const list<list<set<int> > >::const_iterator dlEnd = localDisjunctiveLandmarks.end();
        
        for (int dlID = 0; dlItr != dlEnd; ++dlItr, ++dlID) {
            cout << dlID << ": one of";
            list<set<int> >::const_iterator mItr = dlItr->begin();
            const list<set<int> >::const_iterator mEnd = dlItr->end();
            
            for (; mItr != mEnd; ++mItr) {
                cout << " {";
                
                set<int>::const_iterator fItr = mItr->begin();
                const set<int>::const_iterator fEnd = mItr->end();
                
                for (; fItr != fEnd; ++fItr) {
                    cout << " " << *(RPGBuilder::getLiteral(*fItr));
                }
                cout << " }";
            }
            cout << endl;
            
        }
    }*/
}

int LandmarksAnalysis::factHasBeenSeen(const int& fID, MinimalState* theState)
{
    const int lID = factToLandmarkIndex[fID];
    
    if (lID == -1) return -1;
    return (theState->landmarkStatus[lID]);
}

    
    
    
};
