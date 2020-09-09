#include "RPGBuilder.h"
#include "GlobalSchedule.h"
#include "NumericAnalysis.h"
#include "PreferenceHandler.h"
#include "LiteralAnalysis.h"

using std::endl;

namespace Planner {
 
bool RPGBuilder::postFilter = true;

#ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO    
struct GoalOrPreference {
    
    pair<int,bool> goalIDAndIfNumeric;
    pair<int,bool> prefAndTriggerOrGoal;
    
    GoalOrPreference() : goalIDAndIfNumeric(-1,true), prefAndTriggerOrGoal(-1,true) {
        goalIDAndIfNumeric.first = -1;
        prefAndTriggerOrGoal.first = -1;
    }
    
    inline void setGoal(const int & g, const bool & n) {
        goalIDAndIfNumeric.first = g;
        goalIDAndIfNumeric.second = n;
    }
    
    inline void setpref(const int & g, const bool & n) {
        prefAndTriggerOrGoal.first = g;
        prefAndTriggerOrGoal.second = n;
    }
    
    bool operator <(const GoalOrPreference & other) const {
        if (goalIDAndIfNumeric.first < other.goalIDAndIfNumeric.first) return true;
        if (goalIDAndIfNumeric.first > other.goalIDAndIfNumeric.first) return false;
        
        if (!goalIDAndIfNumeric.second && other.goalIDAndIfNumeric.second) return true;
        if (goalIDAndIfNumeric.second && !other.goalIDAndIfNumeric.second) return false;
        
        if (prefAndTriggerOrGoal.first < other.prefAndTriggerOrGoal.first) return true;
        if (prefAndTriggerOrGoal.first > other.prefAndTriggerOrGoal.first) return false;
        
        if (!prefAndTriggerOrGoal.second && other.prefAndTriggerOrGoal.second) return true;
            
        return false;
    }
    
    void write(ostream & o) const {
        if (goalIDAndIfNumeric.first != -1) {
            if (goalIDAndIfNumeric.second) {
                cout << "Numeric goal " << goalIDAndIfNumeric.first;
            } else {
                cout << "Goal " << goalIDAndIfNumeric.first;
            }
        }
        if (prefAndTriggerOrGoal.first != -1) {
            if (prefAndTriggerOrGoal.second) {
                cout << "Trigger of pref " << prefAndTriggerOrGoal.first << " (" << RPGBuilder::getPreferences()[prefAndTriggerOrGoal.first].name << ")";
            } else {
                cout << "Requirement of pref " << prefAndTriggerOrGoal.first << " (" << RPGBuilder::getPreferences()[prefAndTriggerOrGoal.first].name << ")";
            }
        }
    }
};
    
ostream & operator <<(ostream & o, const set<GoalOrPreference> & s) {
    set<GoalOrPreference>::const_iterator sItr = s.begin();
    const set<GoalOrPreference>::const_iterator sEnd = s.end();
    
    if (sItr == sEnd) {
        cout << "{}";
        return o;
    }
    o << "{";
    sItr->write(o);
    ++sItr;
    
    for (; sItr != sEnd; ++sItr) {
        o << ", ";
        sItr->write(o);
    }
    o << "}";
    return o;
}

#endif
struct NextRegress {

    /** @brief  A literal fact, and whether it is a positive precondition. */    
    pair<int,bool> fact;
    
    /** @brief  A numeric state variable, and if increasors are wanted. */
    pair<int,bool> fluent;
    
    #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO         
    /** @brief  The latest time at which the fact/variable is pertinent. */
    double ts;
    
    /** @brief  The goals to which the fact/variable is relevant. */
    set<GoalOrPreference> forGoal;
    #endif

    
    #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO     
    NextRegress(const int & a, const bool & p, const int & b, const bool & bb, const double & c) : fact(a,p), fluent(b,bb), ts(c) {};
    NextRegress(const int & a, const bool & p,
                const int & b, const bool & bb,
                const double & c, const set<GoalOrPreference> & d)
        : fact(a,p), fluent(b,bb), ts(c) , forGoal(d) {
    }
    #else
    NextRegress(const int & a, const bool & p, const int & b, const bool & bb) : fact(a,p), fluent(b,bb) {};
    #endif                  
        
    bool operator <(const NextRegress & o) const {
        #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO             
        if (ts < o.ts) return true;
        if (ts > o.ts) return false;        
        #endif

        if (fact.first < o.fact.first) return true;
        if (fact.first > o.fact.first) return false;
        
        if (!fact.second && o.fact.second) return true;
        if (fact.second && !o.fact.second) return false;

        if (fluent.first < o.fluent.first) return true;
        if (fluent.first > o.fluent.first) return false;

        if (!fluent.second && o.fluent.second) return true;
        
        return false;
    }

};

#ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
typedef map<GoalOrPreference, double> RelevanceRecord;
static const RelevanceRecord relevanceDefault;
#else
typedef bool RelevanceRecord;
static RelevanceRecord relevanceDefault = false;
#endif

class CommonRegressionData
{

private:

    const int fluentCount;

    vector<RelevanceRecord> positiveLiteralSeenForGoalAtTime;
    vector<RelevanceRecord> negativeLiteralSeenForGoalAtTime;
    vector<RelevanceRecord> fluentSeenGreaterForGoalAtTime;
    vector<RelevanceRecord> fluentSeenLessForGoalAtTime;
    vector<RelevanceRecord> opAppliedForGoalAtTime;
    vector<RelevanceRecord> dpAppliedForGoalAtTime;
    vector<map<int, set<VAL::time_spec> > > fluentIncreasedBy;
    vector<map<int, set<VAL::time_spec> > > fluentDecreasedBy;

    set<NextRegress> expansionQueue;

    #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO    
    void requestVisitHandler(const int & literal, const bool & positive,
                             const int & fluent, const bool & biggerIsBetter,
                             const double & time, set<GoalOrPreference> forGoal) {

        if (time < 0.0) return;

        RelevanceRecord & bestSeen = (literal != -1
                                        ? (positive ? positiveLiteralSeenForGoalAtTime[literal] : negativeLiteralSeenForGoalAtTime[literal])
                                        : (biggerIsBetter ? fluentSeenGreaterForGoalAtTime[fluent] : fluentSeenLessForGoalAtTime[fluent])
                                       );

        set<GoalOrPreference>::iterator whittle = forGoal.begin();

        while (whittle != forGoal.end()) {
            double & oldTime = bestSeen.insert(make_pair(*whittle, -1.0)).first->second;
            if (oldTime < time) {
                oldTime = time;
                ++whittle;
            } else {
                set<GoalOrPreference>::iterator toDel = whittle++;
                forGoal.erase(toDel);
            }
        }

        if (forGoal.empty()) return;

        NextRegress newNR(literal, positive, fluent, biggerIsBetter, time, forGoal);
        pair<set<NextRegress>::iterator, bool> ins = expansionQueue.insert(newNR);
        if (!ins.second) {
            const_cast<set<GoalOrPreference>* >(&(ins.first->forGoal))->insert(forGoal.begin(), forGoal.end());
        } else {
            /*if (GlobalSchedule::globalVerbosity & 16) {
                if (literal != -1) {
                    cout << "- Actually queued visiting " << *(RPGBuilder::getLiteral(literal)) << endl;
                } else {
                    if (biggerIsBetter) {
                        cout << "- Actually queued looking for larger values of ";
                    } else {
                        cout << "- Actually queued looking for smaller values of ";
                    }
                    cout << *(RPGBuilder::getPNE(fluent)) << endl;
                }
            }*/
        }
    }
    #else
    void requestVisitHandler(const int & literal, const bool & positive,
                             const int & fluent, const bool & biggerIsBetter) {

        vector<RelevanceRecord> & currRec = (literal != -1
                                              ? (positive ? positiveLiteralSeenForGoalAtTime : negativeLiteralSeenForGoalAtTime)
                                              : (biggerIsBetter ? fluentSeenGreaterForGoalAtTime : fluentSeenLessForGoalAtTime));
        
        const int vecID = (literal != -1 ? literal : fluent);
        
        if (currRec[vecID]) {
            return;
        }
        
        NextRegress newNR(literal, positive, fluent, biggerIsBetter);
        expansionQueue.insert(newNR);
        
        currRec[vecID] = true;
        
        if (GlobalSchedule::globalVerbosity & 16) {
            if (literal != -1) {
                cout << "- Actually queued visiting " << *(RPGBuilder::getLiteral(literal)) << endl;
            } else {
                if (biggerIsBetter) {
                    cout << "- Actually queued looking for larger values of ";
                } else {
                    cout << "- Actually queued looking for smaller values of ";
                }
                cout << *(RPGBuilder::getPNE(fluent)) << endl;
            }
        }
        
    }
    #endif

public:

    CommonRegressionData(const int & lc, const int  & fc, const int & ac, const int & dc)
            : fluentCount(fc),
            positiveLiteralSeenForGoalAtTime(lc, relevanceDefault), negativeLiteralSeenForGoalAtTime(lc, relevanceDefault),
            fluentSeenGreaterForGoalAtTime(fc, relevanceDefault), fluentSeenLessForGoalAtTime(fc, relevanceDefault),
            opAppliedForGoalAtTime(ac, relevanceDefault), dpAppliedForGoalAtTime(dc, relevanceDefault),
            fluentIncreasedBy(fc), fluentDecreasedBy(fc) {


        const vector<list<pair<int, VAL::time_spec> > > & rpgNumericEffectsToActions = RPGBuilder::getRpgNumericEffectsToActions();
        const vector<RPGBuilder::RPGNumericEffect> & rpgNumericEffects = RPGBuilder::getNumericEffs();
        
        {
            vector<list<pair<int, VAL::time_spec> > >::const_iterator numEffItr = rpgNumericEffectsToActions.begin();
            const vector<list<pair<int, VAL::time_spec> > >::const_iterator numEffEnd = rpgNumericEffectsToActions.end();

            for (int i = 0; numEffItr != numEffEnd; ++numEffItr, ++i) {

                const RPGBuilder::RPGNumericEffect & currEff = rpgNumericEffects[i];
                
                const int affFluent = rpgNumericEffects[i].fluentIndex;

                bool proceed = false;

                if (RPGBuilder::getDominanceConstraints()[affFluent] == E_NODOMINANCE) {

                    proceed = true;

                } else if (RPGBuilder::getDominanceConstraints()[affFluent] == E_BIGGERISBETTER) {

                    proceed = (currEff.isAssignment || (currEff.constant > 0) || (currEff.size > 0));

                } else if (RPGBuilder::getDominanceConstraints()[affFluent] == E_SMALLERISBETTER) {

                    proceed = (currEff.isAssignment || (currEff.constant < 0) || (currEff.size > 0));

                }

                if (proceed) {
                    for (int pass = 0; pass < 2; ++pass) {
                    
                        if (!currEff.isAssignment && currEff.size == 0) {
                            if (pass) {
                                if (currEff.constant >= 0.0) continue;
                            } else {
                                if (currEff.constant <= 0.0) continue;
                            }
                        }
                            
                        map<int, set<VAL::time_spec> > & destSet = (pass ? fluentDecreasedBy[affFluent] : fluentIncreasedBy[affFluent]);

                        const list<pair<int, VAL::time_spec> > & achievers = *numEffItr;
                        list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
                        const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();
                        for (; accItr != accEnd; ++accItr) {
                            destSet[accItr->first].insert(accItr->second);
                        }
                    }
                } else {
                    //cout << "Effect " << i << " is not interesting in its own right\n";
                }
            }
        }
        
    };

    #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
    void requestLiteralVisit(const int & literal, const bool & polarity, const double & time, const set<GoalOrPreference> & forGoal) {
        requestVisitHandler(literal, polarity, -1, false, time, forGoal);
    };

    void requestLiteralVisit(const int & literal, const bool & polarity,  const double & time, const GoalOrPreference & forGoal) {
        set<GoalOrPreference> tmp; tmp.insert(forGoal);
        requestLiteralVisit(literal, polarity, time, tmp);
    };

    void requestFluentVisit(const int & fluent, const bool & biggerIsBetter, const double & time, const set<GoalOrPreference> & forGoal) {
        requestVisitHandler(-1, true, fluent, biggerIsBetter, time, forGoal);
    };

    void requestFluentVisit(const int & fluent, const bool & biggerIsBetter, const double & time, const GoalOrPreference & forGoal) {
        set<GoalOrPreference> tmp; tmp.insert(forGoal);
        requestFluentVisit(fluent, biggerIsBetter, time, tmp);
    };

    void requestNumericPreconditionVisit(const int & preID, const bool & positive, const GoalOrPreference & forGoal) {
        set<GoalOrPreference> tmp; tmp.insert(forGoal);
        requestNumericPreconditionVisit(preID, positive, tmp);
    }
    #else
    void requestLiteralVisit(const int & literal, const bool & polarity) {
        requestVisitHandler(literal, polarity, -1, false);
    };
    void requestFluentVisit(const int & fluent, const bool & biggerIsBetter) {
        requestVisitHandler(-1, true, fluent, biggerIsBetter);
    };    
    #endif
    
    void requestNumericPreconditionVisit(const int & preID, const bool & positive
    #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                                         , const set<GoalOrPreference> & forGoal
    #endif
    ) {
        const RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPreTable()[preID];
        int fID = currPre.LHSVariable;
        if (fID < 0) {
            
        } else if (fID < fluentCount) {
            if (GlobalSchedule::globalVerbosity & 16) {
                if (positive) {
                    cout << "Queueing increasors of " << *(RPGBuilder::getPNE(fID)) << endl;
                } else {
                    cout << "Queueing decreasors of " << *(RPGBuilder::getPNE(fID)) << endl;
                }                                                
                //cout << "Relevant to " << forGoal << endl;
            }
                                    
            requestFluentVisit(fID, positive
            #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                              , DBL_MAX, forGoal
            #endif
                              );
        } else if (fID < (2 * fluentCount)) {
            fID -= fluentCount;
            assert(fID < fluentCount);
            if (GlobalSchedule::globalVerbosity & 16) {
                if (positive) {
                    cout << "Queueing decreasors of " << *(RPGBuilder::getPNE(fID)) << endl;
                } else {
                    cout << "Queueing increasors of " << *(RPGBuilder::getPNE(fID)) << endl;
                }
                //cout << "Relevant to " << forGoal << endl;
            }
                                    
            requestFluentVisit(fID, !positive
            #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                               , DBL_MAX, forGoal
            #endif
                              );
        } else {
            const RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(fID);
            const int size = currAV.size;
            for (int j = 0; j < size; ++j) {
                int afID = currAV.fluents[j];
                
                if (afID < fluentCount) {
                    if (GlobalSchedule::globalVerbosity & 16) {
                        if (positive) {
                            cout << "Queuing increasors of " << *(RPGBuilder::getPNE(afID)) << endl;
                        } else {
                            cout << "Queuing decreasors of " << *(RPGBuilder::getPNE(afID)) << endl;
                        }
                        //cout << "Relevant to " << forGoal << endl;
                    }
                                                    
                    requestFluentVisit(afID, positive
                    #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                                       , DBL_MAX, forGoal
                    #endif                                       
                                      );
                } else {
                    afID -= fluentCount;
                    
                    assert(afID < fluentCount);
                    if (GlobalSchedule::globalVerbosity & 16) {
                        if (positive) {
                            cout << "Queuing decreasors of " << *(RPGBuilder::getPNE(afID)) << endl;
                        } else {
                            cout << "Queuing increasors of " << *(RPGBuilder::getPNE(afID)) << endl;
                        } 
                        //cout << "Relevant to " << forGoal << endl;
                    }
                                                    
                    requestFluentVisit(afID, !positive
                    #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                                       , DBL_MAX, forGoal
                    #endif
                                      );
                }
            }
        }
    }
    
    typedef set<NextRegress>::iterator iterator;

    iterator begin() {
        return expansionQueue.begin();
    };
    const iterator end() {
        return expansionQueue.end();
    };
    void erase(const iterator & i) {
        expansionQueue.erase(i);
    };

    bool empty() const {
        return expansionQueue.empty();
    };

    size_t size() const {
        return expansionQueue.size();
    };
    map<int, set<VAL::time_spec> > & relevantToFluent(const int & currVar, const bool & biggerIsBetter) {        
        return (biggerIsBetter ? fluentIncreasedBy[currVar] : fluentDecreasedBy[currVar]);
    }

    void regressThroughAction(const int & actID, const VAL::time_spec & ts, const NextRegress & inAidOf) {

        static const bool debug = false;

        if (ts == VAL::E_AT) return;

        /*if (ts == VAL::E_AT_START) {
            if (TemporalAnalysis::getActionTSBounds()[actID][0].first > inAidOf.ts) return;
        } else {
            if (TemporalAnalysis::getActionTSBounds()[actID][1].first > inAidOf.ts) return;
        }*/

        /*{
            double atTime = inAidOf.ts;
            if (ts == VAL::E_AT_END && atTime != DBL_MAX) atTime -= getOpMinDuration(actID, -1);

            pair<double, double> & tsBounds = TemporalAnalysis::getActionTSBounds()[actID][0];

            if (atTime < tsBounds.first) return;
            if (atTime > tsBounds.second) atTime = tsBounds.second;

            set<int>::iterator fgItr = forGoal.begin();
            const set<int>::iterator fgEnd = forGoal.end();

            while (fgItr != fgEnd) {
                double & insAt = opAppliedForGoalAtTime[actID].insert(make_pair(*fgItr, -DBL_MAX)).first->second;
                if (insAt >= atTime) {
                    set<int>::iterator fgDel = fgItr++;
                    forGoal.erase(fgDel);
                } else {
                    insAt = atTime;
                    ++fgItr;
                }
            }
        }*/

        #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
        const set<GoalOrPreference> & forGoal = inAidOf.forGoal;

        if (forGoal.empty()) return;
        
        {
            set<GoalOrPreference>::iterator fgItr = forGoal.begin();
            const set<GoalOrPreference>::iterator fgEnd = forGoal.end();
            
            for (; fgItr != fgEnd; ++fgItr) {
                opAppliedForGoalAtTime[actID].insert(make_pair(*fgItr, -DBL_MAX));
            }
                
        }
        double atTime = inAidOf.ts;
        #else        
        opAppliedForGoalAtTime[actID] = true;        
        const double atTime = DBL_MAX;
        #endif
        
        const vector<list<int> > & actNumPres = RPGBuilder::getActionsToRPGNumericStartPreconditions();
        
        assert(actID >= 0);
        assert(actID < (int) actNumPres.size());
        
        const list<Literal*> & currList = RPGBuilder::getProcessedStartPreconditions()[actID];
        const list<int> & currNumList = actNumPres[actID];
        
        
        if (debug) {
            cout << "Adding pres for start of " << *(RPGBuilder::getInstantiatedOp(actID)) << " at time ";
            if (atTime == DBL_MAX) {
                cout << "infinity";
            } else {
                cout << atTime;
            }
            
            cout << ": propositional (" << currList.size() << "), numeric (" << currNumList.size() << ")\n";
        }
        
        list<Literal*>::const_iterator preItr = currList.begin();
        const list<Literal*>::const_iterator preEnd = currList.end();

        for (; preItr != preEnd; ++preItr) {
            const int litID = (*preItr)->getStateID();
            #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
            requestLiteralVisit(litID, true, atTime, forGoal);
            #else
            requestLiteralVisit(litID, true);
            #endif
        }
        
        list<int>::const_iterator numPreItr = currNumList.begin();
        const list<int>::const_iterator numPreEnd = currNumList.end();

        for (; numPreItr != numPreEnd; ++numPreItr) {
            int fID = RPGBuilder::getNumericPreTable()[*numPreItr].LHSVariable;
            if (fID < 0) {

            } else if (fID < fluentCount) {
                if (debug) {
                    cout << " - Preconditions want large values of " << *(RPGBuilder::getPNE(fID)) << endl;
                }
                #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                requestFluentVisit(fID, true, atTime, forGoal);
                #else
                requestFluentVisit(fID, true);
                #endif
            } else if (fID < (2 * fluentCount)) {
                fID -= fluentCount;
                assert(fID < fluentCount);
                if (debug) {
                    cout << " - Preconditions want large values of " << *(RPGBuilder::getPNE(fID)) << endl;
                }
                #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                requestFluentVisit(fID, false, atTime, forGoal);
                #else
                requestFluentVisit(fID, false);
                #endif
            } else {
                const RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(fID);

                const int size = currAV.size;
                for (int i = 0; i < size; ++i) {
                    int afID = currAV.fluents[i];

                    if (afID < fluentCount) {
                        if (debug) {
                            cout << " - Preconditions want large values of " << *(RPGBuilder::getPNE(afID)) << endl;
                        }
                        #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                        requestFluentVisit(afID, true, atTime, forGoal);
                        #else
                        requestFluentVisit(afID, true);
                        #endif
                    } else {
                        afID -= fluentCount;
                        if (debug) {
                            cout << " - Preconditions want large values of " << *(RPGBuilder::getPNE(afID)) << endl;
                        }
                        #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                        requestFluentVisit(afID, false, atTime, forGoal);
                        #else
                        requestFluentVisit(afID, false);
                        #endif
                    }
                }
            }
        }
    }

    /*  void regressThroughDerivationRule(const int & ruleID, const NextRegress & inAidOf) {

        set<int> forGoal(inAidOf.forGoal);

        {
        const double atTime = inAidOf.ts;

        if (atTime < 0.0) return;

        set<int>::iterator fgItr = forGoal.begin();
        const set<int>::iterator fgEnd = forGoal.end();

        while (fgItr != fgEnd) {
        double & insAt = dpAppliedForGoalAtTime[ruleID].insert(make_pair(*fgItr,-DBL_MAX)).first->second;
        if (insAt >= atTime) {
        set<int>::iterator fgDel = fgItr++;
        forGoal.erase(fgDel);
    } else {
        insAt = atTime;
        ++fgItr;
    }
    }
    }

        if (forGoal.empty()) return;

        {
        list<int> & currList = DerivedPredicatesEngine::getPreLiterals(ruleID);

        list<int>::iterator preItr = currList.begin();
        const list<int>::iterator preEnd = currList.end();

        for (; preItr != preEnd; ++preItr) {
        const int litID = (*preItr);
        requestLiteralVisit(litID, inAidOf.ts, forGoal);
    }
    }
        {
        list<int> * currList = &(DerivedPredicatesEngine::getPreNums(ruleID));
        list<int>::iterator preItr = currList->begin();
        const list<int>::iterator preEnd = currList->end();

        for (; preItr != preEnd; ++preItr) {
        int fID = rpgNumericPreconditions[*preItr].LHSVariable;
        if (fID < 0) {

    } else if (fID < fluentCount) {
        assert(fID < fluentCount);
        requestFluentVisit(fID, inAidOf.ts, forGoal);
    } else if (fID < (2 * fluentCount)) {
        fID -= fluentCount;
        assert(fID < fluentCount);
        requestFluentVisit(fID, inAidOf.ts, inAidOf.forGoal);
    } else {
        ArtificialVariable & currAV = getArtificialVariable(fID);

        const int size = currAV.size;
        for (int i = 0; i < size; ++i) {
        int afID = currAV.fluents[i];

        if (afID >= fluentCount) afID -= fluentCount;

        assert(afID < fluentCount);
        requestFluentVisit(afID, inAidOf.ts, forGoal);
    }
    }
    }
    }
    }
    */
    bool opNeverApplied(const int & i) const {
        #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
        return (opAppliedForGoalAtTime[i].empty());
        #else
        return (!opAppliedForGoalAtTime[i]);
        #endif
    };

    #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
    double latestUsefulPoint(const int & op) {
        double toReturn = 0.0;
        map<GoalOrPreference, double>::iterator fgItr = opAppliedForGoalAtTime[op].begin();
        const map<GoalOrPreference, double>::iterator fgEnd = opAppliedForGoalAtTime[op].end();

        for (; fgItr != fgEnd; ++fgItr) {
            const double & currT = fgItr->second;
            if (currT == DBL_MAX) return DBL_MAX;
            if (currT > toReturn) toReturn = currT;
        }

        return toReturn;
    }
    #endif
};


void RPGBuilder::postFilterIrrelevantActions()
{

    const int opCount = instantiatedOps.size();
    const int dpCount = 0; // DerivedPredicatesEngine::getRuleCount();
    const int litCount = literals.size();
    const int fluentCount = pnes.size();

    CommonRegressionData data(litCount, fluentCount, opCount, dpCount);

//  interestingActionUntilWeHaveLiteral = vector<set<int> >(opCount);
//  interestingActionUntilWeHaveNumeric = vector<set<int> >(opCount);

    LiteralSet tinitialState;
    vector<double> tinitialFluents;
    
    {        
        RPGBuilder::getInitialState(tinitialState, tinitialFluents);
    }
    

    #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
    {
        GoalOrPreference scratch;
        
        list<Literal*>::iterator afItr = literalGoals.begin();
        const list<Literal*>::iterator afEnd = literalGoals.end();

        for (int i = 0; afItr != afEnd; ++afItr, ++i) {
            if (LiteralAnalysis::isStatic(*afItr).first) {
                continue;
            }
            
            if (GlobalSchedule::globalVerbosity & 16) {
                cout << "Queuing top-level goal " << *(*afItr) << endl;   
            }
            scratch.setGoal(i,false);
            data.requestLiteralVisit((*afItr)->getID(), true, DBL_MAX, scratch);
        }
    }


    {
        GoalOrPreference scratch;
        
        list<pair<int, int> >::iterator afItr = numericRPGGoals.begin();
        const list<pair<int, int> >::iterator afEnd = numericRPGGoals.end();

        for (int i = 0; afItr != afEnd; ++afItr, ++i) {
            for (int pass = 0 ; pass < 2; ++pass) {
                const int local = (pass ? afItr->second : afItr->first);
                if (local != -1) {
                    scratch.setGoal(i,true);
                    data.requestNumericPreconditionVisit(local, true, scratch);                    
                }
            }
        }
    }
    #else
    {
        list<Literal*>::iterator afItr = literalGoals.begin();
        const list<Literal*>::iterator afEnd = literalGoals.end();

        for (int i = 0; afItr != afEnd; ++afItr, ++i) {
            if (LiteralAnalysis::isStatic(*afItr).first) {
                continue;
            }
            
            if (GlobalSchedule::globalVerbosity & 16) {
                cout << "Queuing top-level goal " << *(*afItr) << endl;   
            }
            data.requestLiteralVisit((*afItr)->getStateID(), true);
        }
    }
    {
        list<pair<int, int> >::iterator afItr = numericRPGGoals.begin();
        const list<pair<int, int> >::iterator afEnd = numericRPGGoals.end();
        
        for (int i = 0; afItr != afEnd; ++afItr, ++i) {
            for (int pass = 0 ; pass < 2; ++pass) {
                const int local = (pass ? afItr->second : afItr->first);
                if (local != -1) {
                    data.requestNumericPreconditionVisit(local, true);                    
                }
            }
        }
    }
    #endif
    
    {
        for (int pass = 0; pass < 2; ++pass) {
                        
            const vector<list<LiteralCellDependency<pair<int,bool> > > > * propositionsInPreferences
             = (pass == 1 ? PreferenceHandler::getNegativePreconditionsToPrefs() : PreferenceHandler::getPreconditionsToPrefs());
             
            const vector<list<LiteralCellDependency<pair<int,bool> > > > * oppositePropositionsInPreferences
             = (pass == 0 ? PreferenceHandler::getNegativePreconditionsToPrefs() : PreferenceHandler::getPreconditionsToPrefs());
             
            const int pSize = propositionsInPreferences->size();
            
            for (int p = 0; p < pSize; ++p) {
                if (LiteralAnalysis::isStatic(RPGBuilder::getLiteral(p)).first) {
                    continue;
                }
                
                if ((*propositionsInPreferences)[p].empty() && (*oppositePropositionsInPreferences)[p].empty()) continue;
                
                #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                set<GoalOrPreference> relevant;
                set<GoalOrPreference>::iterator relIns = relevant.begin();
                
                {
                    list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = (*propositionsInPreferences)[p].begin();
                    const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = (*propositionsInPreferences)[p].end();
                    
                    for (; depItr != depEnd; ++depItr) {
                        if (PreferenceHandler::couldBeBeneficialToSupport(depItr->dest.first, depItr->dest.second)) {
                            // the fact is relevant to preconditions that stand to benefit from it becoming true (or false if pass = 1)
                            // due to that causing a part of their NNF to be satisfied
                            GoalOrPreference tmp;
                            tmp.setpref(depItr->dest.first, depItr->dest.second);
                            relIns = relevant.insert(relIns, tmp);                    
                        }
                    }
                }
                
                {
                    list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = (*oppositePropositionsInPreferences)[p].begin();
                    const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = (*oppositePropositionsInPreferences)[p].end();
                    
                    for (; depItr != depEnd; ++depItr) {
                        if (PreferenceHandler::couldBeBeneficialToOppose(depItr->dest.first, depItr->dest.second)) {
                            // the fact is relevant to preconditions that stand to benefit from it becoming true (or false if pass = 1)
                            // due to that causing a part of their NNF to become further away from satisfied
                            GoalOrPreference tmp;
                            tmp.setpref(depItr->dest.first, depItr->dest.second);
                            relIns = relevant.insert(relIns, tmp);                    
                        }
                    }
                }
                
                if (relevant.empty()) continue;
                
                data.requestLiteralVisit(p, (pass == 0), DBL_MAX, relevant);                                                    
                #else
                bool relevant = false;
                {
                    list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = (*propositionsInPreferences)[p].begin();
                    const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = (*propositionsInPreferences)[p].end();
                    
                    for (; depItr != depEnd; ++depItr) {
                        if (PreferenceHandler::couldBeBeneficialToSupport(depItr->dest.first, depItr->dest.second)) {
                            // the fact is relevant to preconditions that stand to benefit from it becoming true (or false if pass = 1)
                            // due to that causing a part of their NNF to be satisfied
                            relevant = true;
                            break;
                        }
                    }
                }
                
                if (!relevant) {
                    list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = (*oppositePropositionsInPreferences)[p].begin();
                    const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = (*oppositePropositionsInPreferences)[p].end();
                    
                    for (; depItr != depEnd; ++depItr) {
                        if (PreferenceHandler::couldBeBeneficialToOppose(depItr->dest.first, depItr->dest.second)) {
                            // the fact is relevant to preconditions that stand to benefit from it becoming true (or false if pass = 1)
                            // due to that causing a part of their NNF to become further away from satisfied
                            relevant = true;
                            break;
                        }
                    }
                }
                if (relevant) {
                    data.requestLiteralVisit(p, (pass == 0));
                }
                #endif
            }
        }
        
        for (int pass = 0; pass < 2; ++pass) {
            
            if (GlobalSchedule::globalVerbosity & 16) {
                if (!pass) {
                    cout << "Considering effort towards the satisfaction of the numeric preconditions in preferences\n";
                } else {
                    cout << "Considering effort towards the unsatisfaction of the numeric preconditions in preferences\n";
                }
            }
            
            const vector<list<LiteralCellDependency<pair<int,bool> > > > * numericsInPreferences
             = (pass == 0 ? PreferenceHandler::getNumericPreconditionsToPrefs() : PreferenceHandler::getNegativeNumericPreconditionsToPrefs());
             
            const vector<list<LiteralCellDependency<pair<int,bool> > > > * oppositeNumericsInPreferences
             = (pass == 1 ? PreferenceHandler::getNumericPreconditionsToPrefs() : PreferenceHandler::getNegativeNumericPreconditionsToPrefs());
                          
            const int pSize = numericsInPreferences->size();
             
            for (int p = 0; p < pSize; ++p) {

                if ((*numericsInPreferences)[p].empty() && (*oppositeNumericsInPreferences)[p].empty()) continue;

                if (GlobalSchedule::globalVerbosity & 16) {
                    cout << "* " << (RPGBuilder::getNumericPreTable()[p]) << endl;
                }

                #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                set<GoalOrPreference> relevant;
                set<GoalOrPreference>::iterator relIns = relevant.begin();

                {
                    list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = (*numericsInPreferences)[p].begin();
                    const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = (*numericsInPreferences)[p].end();

                    if (GlobalSchedule::globalVerbosity & 16) {
                        if (depItr != depEnd) {
                            if (pass) {
                                cout << "- Unsatisfying the precondition would support the satisfaction of parts of preferences\n";
                            } else {
                                cout << "- Satisfying the precondition would support the satisfaction of parts of preferences\n";
                            }
                        } else {
                            if (pass) {
                                cout << " - Doesn't appear as a negative precondition of a preference\n";
                            } else {
                                cout << " - Doesn't appear as a positive precondition of a preference\n";
                            }
                        }
                    }
                    
                    for (; depItr != depEnd; ++depItr) {
                        if (PreferenceHandler::couldBeBeneficialToSupport(depItr->dest.first, depItr->dest.second)) {
                            GoalOrPreference tmp;
                            tmp.setpref(depItr->dest.first, depItr->dest.second);
                            relIns = relevant.insert(relIns, tmp);
                        }
                    }
                }

                {
                    list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = (*oppositeNumericsInPreferences)[p].begin();
                    const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = (*oppositeNumericsInPreferences)[p].end();
                    
                    if (GlobalSchedule::globalVerbosity & 16) {
                        if (depItr != depEnd) {
                            if (pass) {
                                cout << "- Unsatisfying the precondition would oppose the satisfaction of parts of preferences\n";
                            } else {
                                cout << "- Satisfying the precondition would oppose the satisfaction of parts of preferences\n";
                            }
                        } else {
                            if (pass) {
                                cout << " - Doesn't appear as a positive precondition of a preference\n";
                            } else {
                                cout << " - Doesn't appear as a negative precondition of a preference\n";
                            }
                        }
                    }
                    
                    for (; depItr != depEnd; ++depItr) {
                        if (PreferenceHandler::couldBeBeneficialToOppose(depItr->dest.first, depItr->dest.second)) {
                            GoalOrPreference tmp;
                            tmp.setpref(depItr->dest.first, depItr->dest.second);
                            relIns = relevant.insert(relIns, tmp);
                        }
                    }
                }
                
                if (relevant.empty()) continue;
                
                data.requestNumericPreconditionVisit(p, (pass == 0), relevant);
                #else
                
                bool relevant = false;
                {
                    list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = (*numericsInPreferences)[p].begin();
                    const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = (*numericsInPreferences)[p].end();

                    for (; depItr != depEnd; ++depItr) {
                        if (PreferenceHandler::couldBeBeneficialToSupport(depItr->dest.first, depItr->dest.second)) {
                            relevant = true;
                            break;
                        }
                    }
                }

                if (!relevant) {
                    list<LiteralCellDependency<pair<int,bool> > >::const_iterator depItr = (*oppositeNumericsInPreferences)[p].begin();
                    const list<LiteralCellDependency<pair<int,bool> > >::const_iterator depEnd = (*oppositeNumericsInPreferences)[p].end();
                    for (; depItr != depEnd; ++depItr) {
                        if (PreferenceHandler::couldBeBeneficialToOppose(depItr->dest.first, depItr->dest.second)) {
                            relevant = true;
                            break;
                        }
                    }
                }
                
                if (relevant) {
                    data.requestNumericPreconditionVisit(p, (pass == 0));
                }
                #endif
            }
        }
        
    }

    /*vector<double> initialFluents;

    {
        LiteralSet tinitialState;
        RPGBuilder::getInitialState(tinitialState, initialFluents);
    }
    
    const int pneCount = RPGBuilder::getPNECount();*/
    
    int sinceLastDot = 0;
    
    while (!data.empty()) {

        if (++sinceLastDot == 50) {
            cout << "."; cout.flush();
            sinceLastDot = 0;
        }
        
        if (GlobalSchedule::globalVerbosity & 16) {
            cout << data.size() << " queue entries remaining\n";
        }
        const CommonRegressionData::iterator dItr = data.begin();

        NextRegress currPair = *dItr;

        data.erase(dItr);


        if (currPair.fact.first != -1) {
            const int currLit = currPair.fact.first;

            if (GlobalSchedule::globalVerbosity & 16) {
                cout << "Finding achievers for ";
                if (!currPair.fact.second) {
                    cout << "¬";
                }                
                cout << *(getLiteral(currLit));
                #ifdef NOTEWHICHGOALSACTIONSARERELEVANTTO
                cout << " at time ";
                if (currPair.ts == DBL_MAX) {
                    cout << "infinity - ";
                } else {
                    cout << currPair.ts << "\n";
                }
                #endif
                if (currPair.fact.second) {
                    cout << effectsToActions[currLit].size() << " available\n";
                } else {
                    cout << negativeEffectsToActions[currLit].size() << " available\n";
                }
            }

            const list<pair<int, VAL::time_spec> > & achievers = (currPair.fact.second ? effectsToActions[currLit] : negativeEffectsToActions[currLit]);
            //set<int> & ruleAchievers = DerivedPredicatesEngine::rulesWillDerive(currLit);

            list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
            const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();

            for (; accItr != accEnd; ++accItr) {
                assert(!RPGBuilder::rogueActions[accItr->first]);
                data.regressThroughAction(accItr->first, accItr->second, currPair);
            }
            /*
                        set<int>::iterator raItr = ruleAchievers.begin();
                        const set<int>::iterator raEnd = ruleAchievers.end();

                        for (; raItr != raEnd; ++raItr) {
                        data.regressThroughDerivationRule(*raItr, currPair);
                    }
            */
        } else if (currPair.fluent.first != -1) {
            const int currVar = currPair.fluent.first;

            if (GlobalSchedule::globalVerbosity & 16) {
                if (currPair.fluent.second) {
                    cout << "Finding increasors for " << *(getPNE(currVar)) << endl;
                } else {
                    cout << "Finding decreasors for " << *(getPNE(currVar)) << endl;
                }
            }
            const map<int, map<int,double> > & factResourceCosts = NumericAnalysis::getResourcesExchangedForProposition();
            //const map<int, map<int,double> > & incrResourceCosts = NumericAnalysis::getResourcesExchangedForIncrease();
            //const map<int, map<int,double> > & decrResourceCosts = NumericAnalysis::getResourcesExchangedForDecrease();
            
            map<int, set<VAL::time_spec> >::iterator faItr = data.relevantToFluent(currVar, currPair.fluent.second).begin();
            const map<int, set<VAL::time_spec> >::iterator faEnd = data.relevantToFluent(currVar, currPair.fluent.second).end();

            for (; faItr != faEnd; ++faItr) {
                
                {
                    const list<Literal*> & startPres = getProcessedStartPreconditions()[faItr->first];
                    
                    list<Literal*>::const_iterator pItr = startPres.begin();
                    const list<Literal*>::const_iterator pEnd = startPres.end();
                    
                    for (; pItr != pEnd; ++pItr) {
                        if (tinitialState.find(*pItr) != tinitialState.end()) {
                            continue;
                        }
                        const map<int, map<int,double> >::const_iterator cItr = factResourceCosts.find((*pItr)->getStateID());
                        if (cItr != factResourceCosts.end()) {
                            const map<int,double>::const_iterator vItr = cItr->second.find(currVar);
                            if (vItr != cItr->second.end()) {
                                if (currPair.fluent.second) {
                                    if (vItr->second > 0.0) {
                                        break;
                                    }
                                } else {
                                    if (vItr->second < 0.0) {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (pItr != pEnd) {
                        // cannot use this action to support this fluent level, as it has a precondition fact
                        // that can only be obtained in an conversely unhelpful manner
                        continue;
                    }
                }
                
                /*{
                    const list<int> & startNumPres =  getActionsToRPGNumericStartPreconditions()[faItr->first];
                    
                    list<int>::const_iterator npItr = startNumPres.begin();
                    const list<int>::const_iterator npEnd = startNumPres.end();
                    
                    for (; npItr != npEnd; ++npItr) {
                        const RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPreTable()[*npItr];
                        
                        if (currPre.isSatisfiedWCalculate(initialFluents)) {
                            continue;
                        }
                        
                        int thisVar = currPre.LHSVariable;
                        bool needIncrease = true;
                        
                        if (thisVar < pneCount) {                            
                        } else if (thisVar < 2 * pneCount) {
                            thisVar -= pneCount;
                            needIncrease = false;
                        } else {
                            const RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(thisVar);
                            if (currAV.size != 1) {
                                continue;
                            }
                            if (currAV.fluents[0] < pneCount) {
                                thisVar = currAV.fluents[0];
                            } else {
                                thisVar = currAV.fluents[0] - pneCount;
                                needIncrease = false;
                            }
                        }
                        
                        const map<int, map<int,double> > & costMap = (needIncrease ? incrResourceCosts : decrResourceCosts);
                        
                        const map<int, map<int,double> >::const_iterator costItr = costMap.find(thisVar);
                        
                        if (costItr != costMap.end()) {
                            const map<int,double>::const_iterator costOfThisResource = costItr->second.find(currPair.fluent.first);
                            if (costOfThisResource != costItr->second.end()) {
                                if (currPair.fluent.second) {
                                    if (costOfThisResource->second < 0.0) {
                                        // if we're here, then we had to decrease this resource in order to be able to increase it again
                                        if (GlobalSchedule::globalVerbosity & 16) {
                                            cout << "Cannot use " << *(getInstantiatedOp(faItr->first)) << " as a support\n";
                                        }
                                        break;
                                    }
                                } else {
                                    if (costOfThisResource->second > 0.0) {
                                        // if we're here, then we had to increase this resource in order to be able to decrease it again
                                        if (GlobalSchedule::globalVerbosity & 16) {
                                            cout << "Cannot use " << *(getInstantiatedOp(faItr->first)) << " as a support\n";
                                        }
                                        break;
                                    }
                                }
                            }                                
                        }                                                                                                                        
                    }
                    if (npItr != npEnd) {
                        // cannot use this action to support this fluent level, as it has a precondition fact
                        // that can only be obtained in an conversely unhelpful manner
                        continue;
                    }
                                                        
                }*/
                
                set<VAL::time_spec>::iterator tsItr = faItr->second.begin();
                const set<VAL::time_spec>::iterator tsEnd = faItr->second.end();

                for (; tsItr != tsEnd; ++tsItr) {
                    data.regressThroughAction(faItr->first, *tsItr, currPair);
                }
                

            }

        } else {
            cout << "For some reason, the goal of achieving precisely nothing has worked its way onto the regression filter stack\n";
            assert(false);
        }
    }

    cout << endl;
    
    for (int i = 0; i < opCount; ++i) {
        if (!rogueActions[i] && data.opNeverApplied(i)) {
            
            #ifdef ENABLE_DEBUGGING_HOOKS
            {                
                GlobalSchedule::eliminatedAction(i, "Can never be usefully applied on any path to the goal");
            }
            #endif

            
            if (GlobalSchedule::globalVerbosity & 16) cout << *(instantiatedOps[i]) << " is irrelevant\n";
            pruneIrrelevant(i);

        }
    }


    /*for (int i = 0; i < opCount; ++i) {
    if (!rogueActions[i]) {
    interestingActionUntilWeHaveLiteral[i] = opApplied[i];
    {
    set<int>::iterator ggItr = interestingActionUntilWeHaveLiteral[i].begin();
    const set<int>::iterator ggEnd = interestingActionUntilWeHaveLiteral[i].end();
    bool killAfter = false;
    while (ggItr != ggEnd) {
    if (*ggItr < 0) {
    interestingActionUntilWeHaveNumeric[i].insert(-1 - *ggItr);
    set<int>::iterator ggPrev = ggItr;
    ++ggItr;
    interestingActionUntilWeHaveLiteral[i].erase(ggPrev);
    // sort this out later
    killAfter = true;
    } else {
    assert(*ggItr < (int) goalLiteralsSafe.size());
    if (!goalLiteralsSafe[*ggItr]) {
    killAfter = true;
    }
    ++ggItr;
    }
    }
    if (killAfter) {
    interestingActionUntilWeHaveLiteral[i].clear();
    interestingActionUntilWeHaveNumeric[i].clear();
    } else if (false) {
    cout << *(instantiatedOps[i]) << " is uninteresting once goals";
    ggItr = interestingActionUntilWeHaveLiteral[i].begin();
    for (; ggItr != ggEnd; ++ggItr) cout << " " << *ggItr;
    cout << " are achieved\n";

    }
    }

    {
    set<int>::iterator ggItr = interestingActionUntilWeHaveNumeric[i].begin();
    const set<int>::iterator ggEnd = interestingActionUntilWeHaveNumeric[i].end();
    bool killAfter = false;
    while (ggItr != ggEnd) {
    if (!goalNumericsSafe[*ggItr]) {
    killAfter = true;
    break;
    }
    ++ggItr;

    }
    if (killAfter) {
    interestingActionUntilWeHaveLiteral[i].clear();
    interestingActionUntilWeHaveNumeric[i].clear();
    }
    }
    }
    }*/

};

};

