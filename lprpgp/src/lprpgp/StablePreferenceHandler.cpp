/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "PreferenceHandler.h"
#include "NumericAnalysis.h"

#include <iostream>
#include <sstream>
using std::cerr;
using std::ostringstream;
using std::endl;

const char * positionName[7] = {"satisfied", "unsatisfied", "triggered", "unreachable", "eternally satisfied", "satisfied, but goal seen already and still holds", "satisfied, but if goal seen again, becomes unsatisfied"};

namespace Planner {
    

bool canBeSatisfied(const AutomatonPosition & p)
{
    return (p != unreachable);
}

bool isSatisfied(const AutomatonPosition & p)
{
    return (p == satisfied || p == eternallysatisfied || p == seenoncealreadyandstillholds || p == seenoncealready);
}
    
    
bool PreferenceHandler::preferenceDebug = false;

vector<PreferenceFSA*> PreferenceHandler::automata;
vector<AutomatonPosition> PreferenceHandler::initialAutomataPositions;
vector<vector<pair<int, bool> > > PreferenceHandler::mappingFromFactsToPreferences;
vector<vector<pair<int, bool> > > PreferenceHandler::mappingFromNumericFactsToPreferences;

void PreferenceHandler::buildAutomata()
{
 
    mappingFromFactsToPreferences.resize(RPGBuilder::getLiteralCount());
    mappingFromNumericFactsToPreferences.resize(RPGBuilder::getNumericPrecs().size());
    
    
    vector<RPGBuilder::Preference> & prefs = RPGBuilder::getPreferences();
    const uint prefCount = prefs.size();
    
    if (!prefCount) return;
    
    LiteralSet initialState;
    vector<double> initialFluents;    
    RPGBuilder::getInitialState(initialState, initialFluents);
    
    MinimalState initMS;
    initMS.second = initialFluents;
    
    {
        set<int> & isLocal = initMS.first;
        LiteralSet::iterator isItr = initialState.begin();
        const LiteralSet::iterator isEnd = initialState.end();
        
        for (; isItr != isEnd; ++isItr) {
            isLocal.insert((*isItr)->getID());
        }
    }
    
    
    
    automata.resize(prefCount, (PreferenceFSA*) 0);
    initialAutomataPositions.resize(prefCount, unsatisfied);
    initMS.preferenceStatus.resize(prefCount);
    
    
    for (uint p = 0; p < prefCount; ++p) {
        assert(initMS.preferenceStatus.size() > p);
        switch (prefs[p].cons) {
            case VAL::E_ALWAYS:
            {
                automata[p] = new AlwaysFSA(p, &(prefs[p]), initialAutomataPositions[p]);
                initMS.preferenceStatus[p] = initialAutomataPositions[p];
                automata[p]->update(&initMS);
                initialAutomataPositions[p] = initMS.preferenceStatus[p];
                break;
            }
            case VAL::E_ATEND:
            {
                automata[p] = new AtEndFSA(p, &(prefs[p]), initialAutomataPositions[p]);
                initMS.preferenceStatus[p] = initialAutomataPositions[p];
                automata[p]->update(&initMS);
                initialAutomataPositions[p] = initMS.preferenceStatus[p];
                break;
            }
            case VAL::E_SOMETIME:
            {
                automata[p] = new SometimeFSA(p, &(prefs[p]), initialAutomataPositions[p]);
                initMS.preferenceStatus[p] = initialAutomataPositions[p];
                automata[p]->update(&initMS);
                initialAutomataPositions[p] = initMS.preferenceStatus[p];
                break;
            }
            case VAL::E_ATMOSTONCE:
            {
                automata[p] = new AtMostOnceFSA(p, &(prefs[p]), initialAutomataPositions[p]);
                initMS.preferenceStatus[p] = initialAutomataPositions[p];
                automata[p]->update(&initMS);
                initialAutomataPositions[p] = initMS.preferenceStatus[p];
                break;
            }
            case VAL::E_SOMETIMEAFTER:
            {
                automata[p] = new SometimeAfterFSA(p, &(prefs[p]), initialAutomataPositions[p]);
                initMS.preferenceStatus[p] = initialAutomataPositions[p];
                automata[p]->update(&initMS);
                initialAutomataPositions[p] = initMS.preferenceStatus[p];
                break;
            }                           
            case VAL::E_SOMETIMEBEFORE:
            {
                automata[p] = new SometimeBeforeFSA(p, &(prefs[p]), initialAutomataPositions[p]);
                initMS.preferenceStatus[p] = initialAutomataPositions[p];
                automata[p]->update(&initMS);
                initialAutomataPositions[p] = initMS.preferenceStatus[p];
                break;
            }                                                                                            
            default:
            {
                cerr << "Preference type " << prefs[p].cons << " is currently unhandled\n";
                exit(1);
            }
        }
    }
    
    for (uint p = 0; p < prefCount; ++p) {
        automata[p]->populateFactToPreferenceMappings();
    }
    
}

void PreferenceFSA::populateFactToPreferenceMappings()
{
    assert(initialUnsatisfiedGoalPreconditions == -1);
    assert(initialUnsatisfiedTriggerPreconditions == -1);
    
    cout << "Populating lookup tables for the dependencies of preference " << pref->name << " (" << prefIdx << ")\n";
    
    for (int pass = 0; pass < 2; ++pass) {
        
        
        int & toIncrement = (pass ? initialUnsatisfiedTriggerPreconditions : initialUnsatisfiedGoalPreconditions);
        
        toIncrement = 0;
        
        {
            const list<Literal*> & relevantLiterals = (pass ? pref->trigger : pref->goal);
            
            list<Literal*>::const_iterator factItr = relevantLiterals.begin();
            const list<Literal*>::const_iterator factItrEnd = relevantLiterals.end();
            for (; factItr != factItrEnd; ++factItr) {
                PreferenceHandler::mappingFromFactsToPreferences[(*factItr)->getID()].push_back(make_pair(prefIdx, (pass == 1)));
                ++toIncrement;
            }
            
        }
        {
            
            const list<pair<int,int> > & relevantNumerics = (pass ? pref->triggerRPGNum : pref->goalRPGNum);
            
            {
                cout << "Preference " << pref->name << " (" << prefIdx << ") has " << relevantNumerics.size() << " numeric precondition pairs relevant to its ";
                if (pass) {
                    cout << "trigger\n";
                } else {
                    cout << "goal\n";
                }
            }
            
            list<pair<int,int> >::const_iterator numItr = relevantNumerics.begin();
            const list<pair<int,int> >::const_iterator numEnd = relevantNumerics.end();
            
            
            for (int cidx = 0; numItr != numEnd; ++numItr, ++cidx) {

                for (int pass = 0; pass < 2; ++pass) {
                    const int preIdx = (pass ? numItr->second : numItr->first);
                    if (preIdx < 0) continue;
                    PreferenceHandler::mappingFromNumericFactsToPreferences[preIdx].push_back(make_pair(prefIdx, (pass == 1)));
                    {
                        cout << (RPGBuilder::getNumericPrecs()[preIdx]) << " is relevant to preference " << pref->name << " (" << prefIdx << ")\n";
                    }
                    ++toIncrement;
                }
            }
        }
    }
    
}


PreferenceFSA::PreferenceFSA(const int & i, RPGBuilder::Preference * const p)
: prefIdx(i), pref(p), initialUnsatisfiedTriggerPreconditions(-1), initialUnsatisfiedGoalPreconditions(-1), triggerType(0), goalType(0)
{
    ostringstream c;
    c << "Switch " << p->name;
    switchVarName = c.str();
    
    if (!p->goal.empty()) goalType |= 1;
    if (!p->trigger.empty()) triggerType |= 1;
    
    if (!p->goalRPGNum.empty()) goalType |= 2;
    if (!p->triggerRPGNum.empty()) triggerType |=2;
}

/// common code

namespace PrefCommon {
  
    pair<int,int> extendLPForFactAtEnd(MinimalState * state,MILPRPG * model,
                             const string & switchVarNameIn,
                             const int & prefIdx,RPGBuilder::Preference * const pref,
                             int columnIn=-1,int trigger=-1)
    {    
        if (pref->cost <= 0.0) {
            if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for constraint " << prefIdx << ": has zero cost\n";
            return make_pair(-1,-1);
        }
        
        if (pref->goalRPGNum.empty()) {
            if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for constraint " << prefIdx << ": has no numeric parts\n";
            return make_pair(-1,-1);
        }
        
        const int toReturn = (columnIn == -1 ? model->giveMeAColumn("is-violated " + pref->name) : columnIn);
        
        int newRow = -1;
        const int varCount = RPGBuilder::getPNECount();    
        
        const bool appendindex = (pref->goalRPGNum.size() > 1);
        
        list<pair<int,int> >::const_iterator numItr = pref->goalRPGNum.begin();
        const list<pair<int,int> >::const_iterator numEnd = pref->goalRPGNum.end();
        
        
        for (int cidx = 0; numItr != numEnd; ++numItr, ++cidx) {

            for (int pass = 0; pass < 2; ++pass) {
                const int preIdx = (pass ? numItr->second : numItr->first);
                if (preIdx < 0) continue;
                
                string switchVarName;
                
                if (appendindex) {
                    ostringstream nameIt;
                    nameIt << switchVarNameIn;
                    nameIt << "_" << cidx;
                    
                    if (pass) {
                        nameIt << "_B";
                    }
                    
                    switchVarName = nameIt.str();
                } else {
                    if (pass) {
                        switchVarName = switchVarNameIn + "_B";
                    } else {
                        switchVarName = switchVarNameIn;
                    }
                }
                
                
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
                
                vector<pair<int,double> > entries;
                
                
                assert(currPre.op == VAL::E_GREATEQ);

                entries.push_back(make_pair(toReturn, BIG));
                
                int vi = 1;
                
                list<pair<int,double> > printTmp;
                
                if (currPre.LHSVariable < varCount) {                                
                    entries.push_back(make_pair(model->getColumnForVar(currPre.LHSVariable), currPre.LHSConstant));        
                    if (entries[vi].first >= 0) {
                        if (PreferenceHandler::preferenceDebug) {
                            printTmp.push_back(make_pair(currPre.LHSVariable, currPre.LHSConstant));
                        }
                        ++vi;
                    }
                } else if (currPre.LHSVariable < (2 * varCount)) {
                    entries.push_back(make_pair(model->getColumnForVar(currPre.LHSVariable - varCount), -currPre.LHSConstant));        
                    if (entries[vi].first >= 0) {
                        if (PreferenceHandler::preferenceDebug) {
                            printTmp.push_back(make_pair(currPre.LHSVariable - varCount, -currPre.LHSConstant));
                        }                        
                        ++vi;
                    }
                } else {
                    const RPGBuilder::ArtificialVariable & p = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                    for (int ps = 0; ps < p.size; ++ps) {
                        pair<int, double> addition;
                        if (p.fluents[ps] < varCount) {
                            addition = make_pair(model->getColumnForVar(p.fluents[ps]), p.weights[ps]);
                        } else {
                            addition = make_pair(model->getColumnForVar(p.fluents[ps] - varCount), -p.weights[ps]);
                        }
                        if (addition.first >= 0) {
                            if (PreferenceHandler::preferenceDebug) {
                                if (p.fluents[ps] < varCount) {
                                    printTmp.push_back(make_pair(p.fluents[ps], p.weights[ps]));
                                } else {
                                    printTmp.push_back(make_pair(p.fluents[ps] - varCount, -p.weights[ps]));
                                }
                            }
                            
                            entries.push_back(addition);
                            ++vi;
                        }
                    }
                }

                double rhc = currPre.RHSConstant;

                if (trigger != -1) {
                    rhc -= BIG;
                    entries.push_back(make_pair(trigger, -BIG));
                }

                if (vi >= 2) {
                    if (PreferenceHandler::preferenceDebug) {
                        cout << "Adding switch to enforce that:\n";
                        list<pair<int,double> >::const_iterator ptItr = printTmp.begin();
                        const list<pair<int,double> >::const_iterator ptEnd = printTmp.end();
                        for (int vip = 1; ptItr != ptEnd; ++ptItr, ++vip) {
                            if (vip >= 2) {
                                cout << "  + ";                                
                            } else {
                                cout << "    ";
                            }
                            cout << ptItr->second << " * " << *(RPGBuilder::getPNE(ptItr->first)) << " (column " << entries[vip].first << ")\n";
                        }
                        cout << " >= " << currPre.RHSConstant << "\n";
                    }
                     
                    
                    newRow = model->addSwitchB(switchVarName, entries, rhc, false);
                }
            }
        }
        
        return make_pair(toReturn,newRow);
        
    };

    pair<int,int> extendLPForFactAtSomePoint(MinimalState * state,MILPRPG * model,
                                             const string & switchVarName,
                                             const int & prefIdx,RPGBuilder::Preference * const pref,
                                             bool lookAtGoal=true, bool spaceForHelper=false)
    {    
        static const double big = BIG;
        if (pref->cost <= 0.0) {
            if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for constraint " << prefIdx << ": has zero cost\n";
            return make_pair(-1,-1);
        }
        
        if (lookAtGoal ? pref->goalRPGNum.empty() : pref->triggerRPGNum.empty()) {
            if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for constraint " << prefIdx << ": has no numeric parts\n";
            return make_pair(-1,-1);
        }
        
        const int toReturn = model->giveMeAColumn("is-violated " + pref->name);
        int newRow = -1;
        const int varCount = RPGBuilder::getPNECount();    
        
        list<pair<int,int> >::const_iterator numItr = (lookAtGoal ? pref->goalRPGNum.begin() : pref->triggerRPGNum.begin());
        const list<pair<int,int> >::const_iterator numEnd = (lookAtGoal ? pref->goalRPGNum.end() : pref->triggerRPGNum.end());
        
        for (; numItr != numEnd; ++numItr) {
            
            for (int pass = 0; pass < 2; ++pass) {
                const int preIdx = (pass ? numItr->second : numItr->first);
                if (preIdx < 0) continue;
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
                
                vector<pair<int,double> > entries;
                
                
                assert(currPre.op == VAL::E_GREATEQ);

                entries.push_back(make_pair(toReturn, big));
                
                int vi = 1;
                
                list<pair<int,double> > printTmp;
                
                if (currPre.LHSVariable < varCount) {                                
                    const int cidx = model->getUBColumnForVar(currPre.LHSVariable);
                    if (cidx >= 0) {
                        entries.push_back(make_pair(cidx, currPre.LHSConstant));        
                        if (PreferenceHandler::preferenceDebug) {
                            printTmp.push_back(make_pair(currPre.LHSVariable, currPre.LHSConstant));
                        }
                        ++vi;
                    }
                } else if (currPre.LHSVariable < (2 * varCount)) {
                    const int cidx = model->getUBColumnForVar(currPre.LHSVariable - varCount);
                    if (cidx >= -0) {
                        entries.push_back(make_pair(cidx, -currPre.LHSConstant));        
                        if (PreferenceHandler::preferenceDebug) {
                            printTmp.push_back(make_pair(currPre.LHSVariable - varCount, -currPre.LHSConstant));
                        }                        
                        ++vi;
                    }
                } else {
                    const RPGBuilder::ArtificialVariable & p = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                    for (int ps = 0; ps < p.size; ++ps) {
                        pair<int, double> addition;
                        if (p.fluents[ps] < varCount) {
                            addition = make_pair(model->getUBColumnForVar(p.fluents[ps]), p.weights[ps]);
                        } else {
                            addition = make_pair(model->getLBColumnForVar(p.fluents[ps] - varCount), -p.weights[ps]);
                        }
                        if (addition.first >= 0) {
                            if (PreferenceHandler::preferenceDebug) {
                                if (p.fluents[ps] < varCount) {
                                    printTmp.push_back(make_pair(p.fluents[ps], p.weights[ps]));
                                } else {
                                    printTmp.push_back(make_pair(p.fluents[ps] - varCount, -p.weights[ps]));
                                }
                            }
                            
                            entries.push_back(addition);
                            ++vi;
                        }
                    }
                }

                if (vi >= 2 || spaceForHelper) {
                    if (PreferenceHandler::preferenceDebug) {
                        cout << "Adding switch to enforce that:\n";
                        list<pair<int,double> >::const_iterator ptItr = printTmp.begin();
                        const list<pair<int,double> >::const_iterator ptEnd = printTmp.end();
                        for (int vip = 1; ptItr != ptEnd; ++ptItr, ++vip) {
                            if (vip >= 2) {
                                cout << "  + ";                                
                            } else {
                                cout << "    ";
                            }
                            cout << ptItr->second << " * " << *(RPGBuilder::getPNE(ptItr->first)) << " (column " << entries[vip].first << ")\n";
                        }
                        if (spaceForHelper) {
                            cout << "  + (-BIG * helper)\n";
                        }
                        cout << " >= " << currPre.RHSConstant << " at some point\n";
                    }

                    double rhc = currPre.RHSConstant;

                    if (spaceForHelper) {
                        rhc -= big;
                    }

                    
                    newRow = model->addSwitchB(switchVarName, entries, rhc, spaceForHelper);
                    
                    if (PreferenceHandler::preferenceDebug && spaceForHelper) {
                        cout << "Leaving space for trigger on row " << newRow << endl;
                    }
                }
            }
        }
        
        return make_pair(toReturn,newRow);
        
    };

    void noteVariablesForFactAtSomePoint(vector<uint> & toUpdate, const int & prefIdx,RPGBuilder::Preference * const pref, bool inGoal=true)
    {    
        static const bool recordDebug = true;
        if (pref->cost <= 0.0) {
            if (PreferenceHandler::preferenceDebug || recordDebug) cout << "Not bothering to ask for bounds for constraint " << prefIdx << ": has zero cost\n";
            return;
        }
        
        const int varCount = RPGBuilder::getPNECount();    
                        
        list<pair<int,int> >::const_iterator numItr = (inGoal ? pref->goalRPGNum.begin() : pref->triggerRPGNum.begin());
        const list<pair<int,int> >::const_iterator numEnd = (inGoal ? pref->goalRPGNum.end() : pref->triggerRPGNum.end());
        
        for (; numItr != numEnd; ++numItr) {
            
            for (int pass = 0; pass < 2; ++pass) {
                const int preIdx = (pass ? numItr->second : numItr->first);
                if (preIdx < 0) continue;
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
                
                vector<pair<int,double> > entries;
                                
                assert(currPre.op == VAL::E_GREATEQ);

                                
                if (currPre.LHSVariable < varCount) {                                
                    toUpdate[currPre.LHSVariable] |= 1;
                    if (recordDebug) cout << prefIdx << " wants an upper bound on " << *(RPGBuilder::getPNE(currPre.LHSVariable)) << endl;
                } else if (currPre.LHSVariable < (2 * varCount)) {
                    toUpdate[currPre.LHSVariable - varCount] |= 2;                    
                    if (recordDebug) cout << prefIdx << " wants a lower bound on " << *(RPGBuilder::getPNE(currPre.LHSVariable - varCount)) << endl;
                } else {
                    const RPGBuilder::ArtificialVariable & p = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                    for (int ps = 0; ps < p.size; ++ps) {
                        pair<int, double> addition;
                        if (p.fluents[ps] < varCount) {
                            toUpdate[p.fluents[ps]] |= 1;
                            if (recordDebug) cout << prefIdx << " wants an upper bound on " << *(RPGBuilder::getPNE(p.fluents[ps])) << endl;
                        } else {
                            toUpdate[p.fluents[ps] - varCount] |= 2;
                            if (recordDebug) cout << prefIdx << " wants an upper bound on " << *(RPGBuilder::getPNE(p.fluents[ps] - varCount)) << endl;
                        }
                    }
                }

            }
        }
        
    };

    
    void getSubsumedPresAndEffs(list<int> & presOut, list<int> & effsOut, const int & numCondID) {
        presOut.push_back(numCondID);
                        
        const vector<RPGBuilder::RPGNumericPrecondition> & numPres = RPGBuilder::getNumericPrecs();
        
        const int PNECount = RPGBuilder::getPNECount();

        int masterVar;
        bool GE;
        double constVal;
        
                
        {
            const RPGBuilder::RPGNumericPrecondition & master = numPres[numCondID];
            
            masterVar = master.LHSVariable;           
            constVal = master.RHSConstant;
        
            if (master.LHSVariable < PNECount) {
                GE = true;
            } else if (master.LHSVariable < 2 * PNECount) {
                masterVar -= PNECount;
                if (master.RHSConstant != 0.0) {
                    constVal = -master.RHSConstant;
                }
                GE = false;
            } else {
                const RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(master.LHSVariable);
                
                if (currAV.size != 1) return;
                
                masterVar = currAV.fluents[0];
                constVal -= currAV.constant;
                
                assert(fabs(currAV.weights[0] - 1.0) < 0.00001);
                
                if (masterVar < PNECount) {
                    GE = true;
                } else {
                    masterVar -= PNECount;
                    if (constVal != 0.0) {
                        constVal = -constVal;
                    }
                    GE = false;
                }
            }
        }        
        {
            const int loopLim = numPres.size();
           
            for (int p = 0; p < loopLim; ++p) {
                if (p == numCondID) continue;
                if (numPres[p].LHSVariable < PNECount) {
                    if (masterVar == numPres[p].LHSVariable && GE && numPres[p].RHSConstant >= constVal) {
                        presOut.push_back(p);
                    }
                } else if (numPres[p].RHSVariable < (2 * PNECount)) {
                    const double flipped = (numPres[p].RHSConstant != 0.0 ? -numPres[p].RHSConstant : 0.0);
                    if (masterVar == (numPres[p].LHSVariable - PNECount) && !GE && flipped <= constVal) {
                        presOut.push_back(p);
                    }
                } else {
                    const RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(numPres[p].LHSVariable);
                    
                    if (currAV.size != 1) continue;
                    
                    double newRHS = numPres[p].RHSConstant - currAV.constant;
                    
                    assert(fabs(currAV.weights[0] - 1.0) < 0.00001);
                    
                    if (currAV.fluents[0] < PNECount) {
                        if (masterVar == currAV.fluents[0] && GE && newRHS >= constVal) {
                            presOut.push_back(p);
                        }
                    } else {
                        if (newRHS != 0.0) newRHS = -newRHS;
                        
                        if (masterVar == (currAV.fluents[0] - PNECount) && !GE && newRHS <= constVal) {
                            presOut.push_back(p);
                        }
                    }
                                    
                }
            }
        }
        
        double minEffect;
        
        if (GE) {
            const double B = NumericAnalysis::getBounds()[masterVar].first;
            if (B == -DBL_MAX) return;
            minEffect = constVal - B;
        } else {
            const double B = NumericAnalysis::getBounds()[masterVar].second;
            if (B == DBL_MAX) return;
            minEffect = constVal - B;
        }
        
        {
            const vector<RPGBuilder::RPGNumericEffect> & numEffs = RPGBuilder::getNumericEffs();
            
            const int effCount = numEffs.size();
            
            for (int e = 0; e < effCount; ++e) {
                if (numEffs[e].fluentIndex != masterVar) continue;
                if (numEffs[e].size) continue;
                
                if (numEffs[e].isAssignment) {
                    if (GE) {
                        if (numEffs[e].constant >= constVal) {
                            effsOut.push_back(e);
                        }
                    } else {
                        if (numEffs[e].constant <= constVal) {
                            effsOut.push_back(e);
                        }
                    } 

                } else {                
                    if (GE) {
                        if (numEffs[e].constant >= minEffect) {
                            effsOut.push_back(e);
                        }
                    } else {
                        if (numEffs[e].constant <= minEffect) {
                            effsOut.push_back(e);
                        }                    
                    }
                }
            }
        }
        
    }
    
    void actionsWhichMeanWeHad(set<int> & actVariables, const list<pair<int,int> > & numConds, const list<Literal*> & litConds)
    {
        if (numConds.size() + litConds.size() > 1) {
            cout << "Cannot identify actions which mean we had this preference: too many triggers\n";
            return;
        }
        

        int numCondID = -1;
        
        if (!numConds.empty()) {
            if (numConds.front().first != -1) {
                numCondID = numConds.front().first;
            }
            if (numConds.front().second != -1) {
                if (numCondID != -1) {
                    cout << "Cannot identify actions which mean we had this preference: too many numeric triggers\n";
                    return;
                }
                numCondID = numConds.front().second;
            }
        }
        
        
        if (!litConds.empty()) {
            
            if (numCondID != -1) {
                cout << "Cannot identify actions which mean we had this preference: has literal /and/ numeric triggers\n";
                return;
            }
            
            const int litID = litConds.front()->getID();
            
            {
                const vector<list<pair<int, VAL::time_spec> > > & preToAction = RPGBuilder::getProcessedPreconditionsToActions();
                
                list<pair<int, VAL::time_spec> >::const_iterator wpItr = preToAction[litID].begin();
                const list<pair<int, VAL::time_spec> >::const_iterator wpEnd = preToAction[litID].end();
                
                for (; wpItr != wpEnd; ++wpItr) {
                    assert(wpItr->second == VAL::E_AT_START);
                    actVariables.insert(wpItr->first);
                }
            }
            
            {
                const vector<list<pair<int, VAL::time_spec> > > & eta = RPGBuilder::getEffectsToActions();
                
                list<pair<int, VAL::time_spec> >::const_iterator wpItr = eta[litID].begin();
                const list<pair<int, VAL::time_spec> >::const_iterator wpEnd = eta[litID].end();
                
                for (; wpItr != wpEnd; ++wpItr) {
                    assert(wpItr->second == VAL::E_AT_START);
                    actVariables.insert(wpItr->first);
                }
            }
            cout << "The following actions mean any trajectory contains " << *(litConds.front()) << ":\n";
                        
        } else {
                                
                   
            if (numCondID == -1) return;
            
            list<int> plusRelated;
            list<int> effRelated;
            
            getSubsumedPresAndEffs(plusRelated, effRelated, numCondID);
            
            {
                const vector<list<pair<int, VAL::time_spec> > > & numPreToAction = RPGBuilder::getProcessedRPGNumericPreconditionsToActions();
            
                list<int>::const_iterator prItr = plusRelated.begin();
                const list<int>::const_iterator prEnd = plusRelated.end();
                
                for (; prItr != prEnd; ++prItr) {
                    list<pair<int, VAL::time_spec> >::const_iterator wpItr = numPreToAction[*prItr].begin();
                    const list<pair<int, VAL::time_spec> >::const_iterator wpEnd = numPreToAction[*prItr].end();
                    
                    for (; wpItr != wpEnd; ++wpItr) {
                        assert(wpItr->second == VAL::E_AT_START);
                        actVariables.insert(wpItr->first);
                    }
                }
            }
            
            {
             
                const vector<list<pair<int, VAL::time_spec> > > & eta = RPGBuilder::getRpgNumericEffectsToActions();
                list<int>::const_iterator prItr = effRelated.begin();
                const list<int>::const_iterator prEnd = effRelated.end();
                
                for (; prItr != prEnd; ++prItr) {
                    list<pair<int, VAL::time_spec> >::const_iterator wpItr = eta[*prItr].begin();
                    const list<pair<int, VAL::time_spec> >::const_iterator wpEnd = eta[*prItr].end();
                    
                    for (; wpItr != wpEnd; ++wpItr) {
                        assert(wpItr->second == VAL::E_AT_START);
                        actVariables.insert(wpItr->first);
                    }
                }
            }
            cout << "The following actions mean any trajectory contains " << (RPGBuilder::getNumericPrecs()[numCondID]) << ":\n";
        }
        
        
        set<int>::const_iterator avpItr = actVariables.begin();
        const set<int>::const_iterator avpEnd= actVariables.end();
        
        for (; avpItr != avpEnd; ++avpItr) {
            cout << "\t" << *(RPGBuilder::getInstantiatedOp(*avpItr)) << "\n";
        }
    }
    
    void buildTriggersOnEndValues(int col, RPGBuilder::Preference * const pref, MILPRPG * const model, bool useGoal=false)
    {
        
        static const int varCount = RPGBuilder::getPNECount();
        
        list<pair<int,int> >::const_iterator tItr = (useGoal ? pref->goalRPGNum.begin() : pref->triggerRPGNum.begin());
        const list<pair<int,int> >::const_iterator tItrEnd = (useGoal ? pref->goalRPGNum.end() : pref->triggerRPGNum.end());
        
        const bool appendindex = (useGoal ? (pref->goalRPGNum.size() > 1) : (pref->triggerRPGNum.size() > 1));
        
        for (int cidx = 0; tItr != tItrEnd; ++tItr, ++cidx) {
            for (int pass = 0; pass < 2; ++pass) {
                const int pid = (pass ? tItr->second : tItr->first);
                if (pid == -1) continue;
                

                string switchVarName = "etrig-" + pref->name;
                
                if (appendindex) {
                    ostringstream nameIt;
                    nameIt << switchVarName;
                    nameIt << "_" << cidx;
                    
                    if (pass) {
                        nameIt << "_B";
                    }
                                        
                    switchVarName = nameIt.str();
                } else {
                    if (pass) {
                        switchVarName = switchVarName + "_B";
                    }
                }
                vector<pair<int,double> > newRowVec;
                
                const RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[pid];
                
                if (currPre.LHSVariable < varCount) {
                    newRowVec.resize(2);
                    newRowVec[0] = make_pair(model->getColumnForVar(currPre.LHSVariable), 1.0);
                    newRowVec[1] = make_pair(col, -BIG);
                    model->addTrigger(switchVarName, newRowVec, -DBL_MAX, currPre.RHSConstant - SMALL);
                } else if (currPre.LHSVariable < 2 * varCount) {
                    newRowVec.resize(2);
                    newRowVec[0] = make_pair(model->getColumnForVar(currPre.LHSVariable - varCount), 1.0);
                    newRowVec[1] = make_pair(col, BIG);
                    if (currPre.RHSConstant != 0.0) {
                        model->addTrigger(switchVarName, newRowVec, (-currPre.RHSConstant) + SMALL, DBL_MAX);
                    } else {
                        model->addTrigger(switchVarName, newRowVec, SMALL, DBL_MAX);
                    }                
                } else {
                    const RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                    double newRHC = currPre.RHSConstant - currAV.constant;
                    
                    newRowVec.resize(1 + currAV.size);
                    int fid;
                    for (int s = 0; s < currAV.size; ++s) {
                        fid = currAV.fluents[s];
                        if (fid < varCount) {
                            newRowVec[s] = make_pair(model->getColumnForVar(fid), currAV.weights[s]);
                        } else {
                            fid -= varCount;
                            assert(fid < varCount);                        
                            newRowVec[s] = make_pair(model->getColumnForVar(fid), -currAV.weights[s]);
                        }                 
                    }
                    newRowVec[currAV.size] = make_pair(col, -BIG);
                    model->addTrigger(switchVarName, newRowVec, -DBL_MAX, newRHC - SMALL);
                }
                
                
                ++col;
            }
        }
    }
    
    
    
    
};

/// always constraints                        

AlwaysFSA::AlwaysFSA(const int & idx, RPGBuilder::Preference * const p, AutomatonPosition & initPosn)
: PreferenceFSA(idx, p)
{
    assert(pref->cons == VAL::E_ALWAYS);    
    initPosn = satisfied;
}

void AlwaysFSA::update(MinimalState * state)
{
    const bool alwaysUpdateDebug = true;
    if (state->preferenceStatus[prefIdx] == unreachable) return;
    
    list<Literal*>::const_iterator factItr = pref->goal.begin();
    const list<Literal*>::const_iterator factEnd = pref->goal.end();
    
    for (; factItr != factEnd; ++factItr) {
        if (state->first.find((*factItr)->getID()) == state->first.end()) {
            state->preferenceStatus[prefIdx] = unreachable;
            return;
        }
    }

    list<pair<int,int> >::const_iterator numItr = pref->goalRPGNum.begin();
    const list<pair<int,int> >::const_iterator numEnd = pref->goalRPGNum.end();
    
    for (; numItr != numEnd; ++numItr) {
        bool bothMatch = true;
        for (int pass = 0; bothMatch && pass < 2; ++pass) {
            const int preIdx = (pass ? numItr->second : numItr->first);
            if (preIdx < 0) continue;
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
            bothMatch = currPre.isSatisfiedWCalculate(state->second);
        }
        if (!bothMatch) {
            state->preferenceStatus[prefIdx] = unreachable;
            if (alwaysUpdateDebug || PreferenceHandler::preferenceDebug) cout << "Preference " << pref->name << " is now violated\n";
            return;
        }
    }
      
}

double AlwaysFSA::currentCost(MinimalState * state, const int & flag)
{
    if (!(goalType & flag)) return 0.0;
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (PreferenceHandler::preferenceDebug) {
            cout << pref->name << ") cost is " << pref->cost << " because the always condition has been broken\n";
        }
        return pref->cost;
    }
    
    return 0.0;
}

double AlwaysFSA::reachableCost(MinimalState * state, const int & flag)
{
    if (!(goalType & flag)) return 0.0;
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        return pref->cost;
    }
        
    return 0.0;
}

double AlwaysFSA::GCost(MinimalState * state, const int & flag)
{
    if (!(goalType & flag)) return 0.0;
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        return pref->cost;
    }
            
    return 0.0;
}

                        
pair<int,int> AlwaysFSA::extendLP(map<int,list<int> > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
            
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for always constraint " << prefIdx << ": is already violated\n";
        return make_pair(-1,-1);
    }
    
    const pair<int,int> toReturn(PrefCommon::extendLPForFactAtEnd(state, model, switchVarName, prefIdx, pref));
    
    if (toReturn.first == -1) return toReturn;
    
    list<pair<int,int> >::const_iterator gfItr = pref->goalRPGNum.begin();
    const list<pair<int,int> >::const_iterator gfEnd = pref->goalRPGNum.end();
    
    for (; gfItr != gfEnd; ++gfItr) {
        if (gfItr->first != -1) {
            factNeededForCol[gfItr->first].push_back(prefIdx);
        }
        if (gfItr->second != -1) {
            factNeededForCol[gfItr->second].push_back(prefIdx);
        }
    }
   
    return toReturn;
    
};

void AlwaysFSA::getUnsatisfiedConditionCounts(MinimalState &, vector<int> &)
{
    // Don't actually try to satisfy always preferences - just avoid breaking them
    return;
}

void AlwaysFSA::getCostsOfDeletion(MinimalState & theState, map<int, set<int> > & factCost, map<int, map<double, set<int> > > & numChangeCost)
{
    if (theState.preferenceStatus[prefIdx] == unreachable) return;
    
    static const int varCount = RPGBuilder::getPNECount();
    
    const double pCost = pref->cost;
    
    if (pCost == 0.0) return;

    {        
        list<Literal*>::const_iterator factItr = pref->goal.begin();
        const list<Literal*>::const_iterator factEnd = pref->goal.end();
        
        int fID;
        for (; factItr != factEnd; ++factItr) {
            fID = (*factItr)->getID();
            assert(theState.first.find(fID) != theState.first.end());
            factCost[fID].insert(prefIdx);
        }
    }
            
    {
        list<pair<int,int> >::const_iterator numItr = pref->goalRPGNum.begin();
        const list<pair<int,int> >::const_iterator numEnd = pref->goalRPGNum.end();
        
        int pass, preIdx;
        
        for (; numItr != numEnd; ++numItr) {
            for (pass = 0;pass < 2; ++pass) {
                preIdx = (pass ? numItr->second : numItr->first);
                if (preIdx == -1) continue;
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
                
                if (currPre.LHSVariable < varCount) {
                    double delta = currPre.RHSConstant - theState.second[currPre.LHSVariable];
                    if (delta >= 0.0) delta = -1e-6;
                    numChangeCost[currPre.LHSVariable][delta].insert(prefIdx);
                } else if (currPre.LHSVariable < 2 * varCount) {
                    double delta = -currPre.RHSConstant - theState.second[currPre.LHSVariable - varCount];
                    if (delta <= 0.0) delta = 1e-6;
                    numChangeCost[currPre.LHSVariable - varCount][delta].insert(prefIdx);
                } else {
                    const RPGBuilder::ArtificialVariable & currAV =  RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                    if (currAV.size == 1) {
                        double rhc = currPre.RHSConstant - currAV.constant;
                        rhc /= currAV.weights[0];
                        
                        if (currAV.fluents[0] < varCount) {
                            double delta = rhc - theState.second[currAV.fluents[0]];
                            if (delta >= 0.0) delta = -1e-6;
                            numChangeCost[currAV.fluents[0]][delta].insert(prefIdx);
                        } else {
                            double delta = -rhc - theState.second[currAV.fluents[0] - varCount];
                            if (delta <= 0.0) delta = 1e-6;
                            numChangeCost[currAV.fluents[0] - varCount][delta].insert(prefIdx);
                        }
                    }
                }
            }
        }
    }
    
}

                                
/// at end constraints                        

AtEndFSA::AtEndFSA(const int & idx, RPGBuilder::Preference * const p, AutomatonPosition & initPosn)
: PreferenceFSA(idx, p)
{
    assert(pref->cons == VAL::E_ATEND);    
    initPosn = unsatisfied;
}

void AtEndFSA::update(MinimalState * state)
{
    if (state->preferenceStatus[prefIdx] == unreachable) return;
    
    state->preferenceStatus[prefIdx] = unsatisfied;
    
    list<Literal*>::const_iterator factItr = pref->goal.begin();
    const list<Literal*>::const_iterator factEnd = pref->goal.end();
    
    for (; factItr != factEnd; ++factItr) {
        if (state->first.find((*factItr)->getID()) == state->first.end()) {
            if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: at-end pref " << pref->name << " is currently unsatisfied, cost " << pref->cost << "\n";
            return;
        }
    }

    list<pair<int,int> >::const_iterator numItr = pref->goalRPGNum.begin();
    const list<pair<int,int> >::const_iterator numEnd = pref->goalRPGNum.end();
    
    for (; numItr != numEnd; ++numItr) {
        bool bothMatch = true;
        for (int pass = 0; bothMatch && pass < 2; ++pass) {
            const int preIdx = (pass ? numItr->second : numItr->first);
            if (preIdx < 0) continue;
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
            bothMatch = currPre.isSatisfiedWCalculate(state->second);
        }
        if (!bothMatch) {
            if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: at-end pref " << pref->name << " is currently unsatisfied, cost " << pref->cost << " - automaton position = " << state->preferenceStatus[prefIdx] << "\n";
            return;            
        }
    }
    
    if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: at-end pref " << pref->name << " is currently satisfied, cost " << pref->cost << "\n";
    state->preferenceStatus[prefIdx] = satisfied;
      
}

double AtEndFSA::currentCost(MinimalState * state, const int & flag)
{
    if (!(goalType & flag)) return 0.0;
    
    if (!(state->preferenceStatus[prefIdx] == satisfied)) {
        if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": at-end pref " << pref->name << " is currently unsatisfied, cost " << pref->cost << "\n";
        return pref->cost;
    }
    
    if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": at-end pref " << pref->name << " is currently satisfied\n";
    return 0.0;
}

double AtEndFSA::reachableCost(MinimalState * state, const int & flag)
{
    if (!(goalType & flag)) return 0.0;
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        return pref->cost;
    }
        
    return 0.0;
}

double AtEndFSA::GCost(MinimalState * state, const int & flag)
{
    if (!(goalType & flag)) return 0.0;
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        return pref->cost;
    }
            
    return 0.0;
}


pair<int,int> AtEndFSA::extendLP(map<int,list<int> > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
    
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for at-end constraint " << prefIdx << ": can never be reached\n";
        return make_pair(-1,-1);
        
    }
    
    const pair<int,int> toReturn = PrefCommon::extendLPForFactAtEnd(state, model, switchVarName, prefIdx, pref);
    
    if (toReturn.first == -1) return toReturn;
    
    list<pair<int,int> >::const_iterator gfItr = pref->goalRPGNum.begin();
    const list<pair<int,int> >::const_iterator gfEnd = pref->goalRPGNum.end();
    
    for (; gfItr != gfEnd; ++gfItr) {
        if (gfItr->first != -1) {
            factNeededForCol[gfItr->first].push_back(prefIdx);
        }
        if (gfItr->second != -1) {
            factNeededForCol[gfItr->second].push_back(prefIdx);
        }
    }
       
       
    
    return toReturn;
    
};

void AtEndFSA::getUnsatisfiedConditionCounts(MinimalState & state, vector<int> & toFill)
{
    if (state.preferenceStatus[prefIdx] == unreachable) {
        // If it was unreachable before, it's still unreachable now, so don't try to get it
        return;
    }
    toFill[0] = initialUnsatisfiedGoalPreconditions;
}

void AtEndFSA::getDesiredGoals(list<list<Literal*> * > & desired, MinimalState & startState, const vector<vector<int> > & unsatisfiedPreferenceConditions)
{
    if (startState.preferenceStatus[prefIdx] == satisfied || startState.preferenceStatus[prefIdx] == unreachable) return;
    if (unsatisfiedPreferenceConditions[prefIdx][0]) return;
    
    desired.push_back(&(pref->goal));
}


void AtEndFSA::updateCosts(MinimalState & initialState,
                            const bool & wasTheTrigger,
                            vector<AutomatonPosition> & optimisticAutomataPositions,
                            map<int, AddingConstraints > & prefCostOfAddingFact,
                            map<int, map<double, AddingConstraints > > & numChangeCost,
                            list<int> & preferencesThatNowHaveNoAddCosts)
{
                                      
    if (wasTheTrigger) return;
    
    if (PreferenceHandler::preferenceDebug) {
        cout << "\t\t- At end preference " << pref->name << ", index " << prefIdx << " is satisfied\n";
    }
    
    optimisticAutomataPositions[prefIdx] = satisfied;
    
}

/// sometime constraints

SometimeFSA::SometimeFSA(const int & idx, RPGBuilder::Preference * const p, AutomatonPosition & initPosn)
: PreferenceFSA(idx, p)
{
    assert(pref->cons == VAL::E_SOMETIME);    
    initPosn = unsatisfied;
}

void SometimeFSA::update(MinimalState * state)
{
    const bool sometimeUpdateDebug = false;
    if (state->preferenceStatus[prefIdx] == satisfied || state->preferenceStatus[prefIdx] == unreachable) {
        if (sometimeUpdateDebug || PreferenceHandler::preferenceDebug) {
            if (state->preferenceStatus[prefIdx] == satisfied) {
                cout << prefIdx << ": update: sometime pref " << pref->name << " has already been satisfied, cost " << pref->cost << "\n";
            } else {
                cout << prefIdx << ": update: sometime pref " << pref->name << " cannot be satisfied from here, cost " << pref->cost << "\n";
            }
        }
        return;
    }
    
    list<Literal*>::const_iterator factItr = pref->goal.begin();
    const list<Literal*>::const_iterator factEnd = pref->goal.end();
    
    for (; factItr != factEnd; ++factItr) {
        if (state->first.find((*factItr)->getID()) == state->first.end()) {
            if (sometimeUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime pref " << pref->name << " is currently unsatisfied, cost " << pref->cost << "\n";
            return;
        }
    }

    list<pair<int,int> >::const_iterator numItr = pref->goalRPGNum.begin();
    const list<pair<int,int> >::const_iterator numEnd = pref->goalRPGNum.end();
    
    for (; numItr != numEnd; ++numItr) {
        bool bothMatch = true;
        for (int pass = 0; bothMatch && pass < 2; ++pass) {
            const int preIdx = (pass ? numItr->second : numItr->first);
            if (preIdx < 0) continue;
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
            bothMatch = currPre.isSatisfiedWCalculate(state->second);
        }
        if (!bothMatch) {
            if (sometimeUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime pref " << pref->name << " is currently unsatisfied, cost " << pref->cost << " - automaton position = " << state->preferenceStatus[prefIdx] << "\n";
            return;            
        }
    }
    
    if (sometimeUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime pref " << pref->name << " is now satisfied, cost " << pref->cost << "\n";
    state->preferenceStatus[prefIdx] = satisfied;
      
}

double SometimeFSA::currentCost(MinimalState * state, const int & flag)
{
    if (!(goalType & flag)) return 0.0;
    
    if (!(state->preferenceStatus[prefIdx] == satisfied)) {
        if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": sometime pref " << pref->name << " is currently unsatisfied, cost " << pref->cost << "\n";
        return pref->cost;
    }
    
    if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": sometime pref " << pref->name << " is satisfied\n";
    return 0.0;
}

double SometimeFSA::reachableCost(MinimalState * state, const int & flag)
{
    if (!(goalType & flag)) return 0.0;
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        return pref->cost;
    }
        
    return 0.0;
}

double SometimeFSA::GCost(MinimalState * state, const int & flag)
{
    if (!(goalType & flag)) return 0.0;
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        return pref->cost;
    }
            
    return 0.0;
}

void SometimeFSA::noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate)
{
    PrefCommon::noteVariablesForFactAtSomePoint(toUpdate, prefIdx, pref);        
}

pair<int,int> SometimeFSA::extendLP(map<int,list<int> > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
    static const bool sometimeExtendDebug = false;
    
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Not extending LP for sometime constraint " << prefIdx << ": can never be reached\n";
        return make_pair(-1,-1);
        
    }
    if (state->preferenceStatus[prefIdx] == satisfied) {
        if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Not extending LP for sometime constraint " << prefIdx << ": already reached\n";
        return make_pair(-1,-1);
        
    }
    
    if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Extending LP for sometime constraint " << prefIdx << "\n";
    
    const pair<int,int> toReturn = PrefCommon::extendLPForFactAtSomePoint(state, model, switchVarName, prefIdx, pref);
    
    if (toReturn.first == -1) {
        if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Not really extended LP for sometime constraint " << prefIdx << "\n";
        return toReturn;
    }
    
    list<pair<int,int> >::const_iterator gfItr = pref->goalRPGNum.begin();
    const list<pair<int,int> >::const_iterator gfEnd = pref->goalRPGNum.end();
    
    bool pAny = false;
    
    for (; gfItr != gfEnd; ++gfItr) {
        if (gfItr->first != -1) {
            pAny = true;
            if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Needs " << RPGBuilder::getNumericPrecs()[gfItr->first] << endl;
            factNeededForCol[gfItr->first].push_back(prefIdx);
        }
        if (gfItr->second != -1) {
            if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Needs " << RPGBuilder::getNumericPrecs()[gfItr->second] << endl;
            pAny = true;
            factNeededForCol[gfItr->second].push_back(prefIdx);
        }
    }
    
    if (!pAny) {
        if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Didn't need any numeric facts\n";
    }
       
    return toReturn;
    
};

void SometimeFSA::getUnsatisfiedConditionCounts(MinimalState & state, vector<int> & toFill)
{
    if (state.preferenceStatus[prefIdx] == satisfied || state.preferenceStatus[prefIdx] == unreachable) {
        // Once we've had a sometime preference, or shown it to be unreachable, we don't need to try to get it again
        return;
    }
    
    toFill[0] = initialUnsatisfiedGoalPreconditions;
}

void SometimeFSA::getDesiredGoals(list<list<Literal*> * > & desired, MinimalState & startState, const vector<vector<int> > & unsatisfiedPreferenceConditions)
{
    if (startState.preferenceStatus[prefIdx] == satisfied || startState.preferenceStatus[prefIdx] == unreachable) return;
    if (unsatisfiedPreferenceConditions[prefIdx][0]) return;
            
    desired.push_back(&(pref->goal));
}

void SometimeFSA::updateCosts(MinimalState & initialState,
                               const bool & wasTheTrigger,
                               vector<AutomatonPosition> & optimisticAutomataPositions,
                               map<int, AddingConstraints > & prefCostOfAddingFact,
                               map<int, map<double, AddingConstraints > > & numChangeCost,
                               list<int> & preferencesThatNowHaveNoAddCosts)
{
   
    if (wasTheTrigger) return;
       
    optimisticAutomataPositions[prefIdx] = satisfied;
   
}

/// at-most-once constraints

AtMostOnceFSA::AtMostOnceFSA(const int & idx, RPGBuilder::Preference * const p, AutomatonPosition & initPosn)
: PreferenceFSA(idx, p), triggerPartCount(0)
{
    assert(pref->cons == VAL::E_ATMOSTONCE);
    initPosn = satisfied;
    
    PrefCommon::actionsWhichMeanWeHad(actionsImplyingTrigger, pref->goalRPGNum, pref->goal);
    
    addingOneThingCanTrigger = true;
    addingThisWouldTrigger = make_pair(-1,true);
    
    const int litTrigCount = pref->goal.size();
    
    if (litTrigCount > 1) {
        addingOneThingCanTrigger = false;
        return;
    }
            
    if (litTrigCount) {
        addingThisWouldTrigger = make_pair(pref->goal.front()->getID(), false);
    }
                                
    const int numTrigCount = pref->goalRPGNum.size();
    
    if (numTrigCount > 1) {
        addingOneThingCanTrigger = false;
        return;
    }
                                                            
    if (numTrigCount) {
        const pair<int,int> & numPair = pref->goalRPGNum.front();
        for (int pass = 0; pass < 2; ++pass) {
            const int preIdx = (pass ? numPair.second : numPair.first);
            if (preIdx == -1) continue;
            if (addingThisWouldTrigger.first != -1) {
                addingOneThingCanTrigger = false;
                return;
            }
            addingThisWouldTrigger = make_pair(preIdx, true);
        }
    }
                                                                                                
    if (addingThisWouldTrigger.first == -1) {
        addingOneThingCanTrigger = false;
    }
}

void AtMostOnceFSA::update(MinimalState * state)
{
    const bool atMostOnceUpdateDebug = false;
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (atMostOnceUpdateDebug || PreferenceHandler::preferenceDebug) {
            cout << prefIdx << ": update: at-most-once pref " << pref->name << " cannot be satisfied from here, cost " << pref->cost << "\n";
        }
        return;
    }
    
    bool allPresent = true;
        
    {
        
        
        
        list<Literal*>::const_iterator factItr = pref->goal.begin();
        const list<Literal*>::const_iterator factEnd = pref->goal.end();
        
        for (; factItr != factEnd; ++factItr) {
            if (state->first.find((*factItr)->getID()) == state->first.end()) {
                if (atMostOnceUpdateDebug || PreferenceHandler::preferenceDebug) {
                }
                allPresent = false;                                                
                break;
            }
        }
            
        if (allPresent) {
            
            list<pair<int,int> >::const_iterator numItr = pref->triggerRPGNum.begin();
            const list<pair<int,int> >::const_iterator numEnd = pref->triggerRPGNum.end();
            
            for (; numItr != numEnd; ++numItr) {
                bool bothMatch = true;
                for (int pass = 0; bothMatch && pass < 2; ++pass) {
                    const int preIdx = (pass ? numItr->second : numItr->first);
                    if (preIdx < 0) {
                        continue;
                    }
                    RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
                    bothMatch = currPre.isSatisfiedWCalculate(state->second);
                }
                if (!bothMatch) {
                    allPresent = false;
                    break;
                }
            }
        }
    }
        
    switch (state->preferenceStatus[prefIdx]) {
        case satisfied:
        {
            if (allPresent) {
                if (atMostOnceUpdateDebug || PreferenceHandler::preferenceDebug) {
                    cout << prefIdx << ": update: at-most-once pref " << pref->name << " has been seen once\n";
                }
                state->preferenceStatus[prefIdx] = seenoncealreadyandstillholds;
                return;
            } else {
                if (atMostOnceUpdateDebug || PreferenceHandler::preferenceDebug) {
                    cout << prefIdx << ": update: at-most-once pref " << pref->name << " still never been seen\n";
                }
                return;
            }           
        }
        case seenoncealreadyandstillholds:
        {
            if (allPresent) {
                if (atMostOnceUpdateDebug || PreferenceHandler::preferenceDebug) {
                    cout << prefIdx << ": update: at-most-once pref " << pref->name << " has been seen once, and still holds\n";
                }
                return;
            } else {
                if (atMostOnceUpdateDebug || PreferenceHandler::preferenceDebug) {
                    cout << prefIdx << ": update: at-most-once pref " << pref->name << " has been seen once, but doesn't currently hold\n";
                }
                state->preferenceStatus[prefIdx] = seenoncealready;
                return;
            }

        }
        case seenoncealready:
        {
            if (allPresent) {
                if (atMostOnceUpdateDebug || PreferenceHandler::preferenceDebug) {
                    cout << prefIdx << ": update: at-most-once pref " << pref->name << " has been violated - now seen twice - cost = " << pref->cost << endl;
                }
                state->preferenceStatus[prefIdx] = unreachable;
                return;
            } else {
                if (atMostOnceUpdateDebug || PreferenceHandler::preferenceDebug) {
                    cout << prefIdx << ": update: at-most-once pref " << pref->name << " has been seen once, and still doesn't currently hold\n";
                }
                return;
            }

        }
        default:
        {
            cerr << "Internal error - should not have preference status " << positionName[state->preferenceStatus[prefIdx]] << " for the at-most-once preference " << pref->name << endl;
            exit(1);
        }
    }
}

double AtMostOnceFSA::currentCost(MinimalState * state, const int & flag)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (!(triggerType & flag)) return 0.0;
        if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": at-most-once pref " << pref->name << " cannot be satisfied, cost " << pref->cost << "\n";
        return pref->cost;
    }
            
    if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": at-most-once pref " << pref->name << " is cost free: " << positionName[state->preferenceStatus[prefIdx]] << endl;
    return 0.0;
}

double AtMostOnceFSA::reachableCost(MinimalState * state, const int & flag)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (!(triggerType & flag)) return 0.0;
        return pref->cost;
    }
                
    return 0.0;
}

double AtMostOnceFSA::GCost(MinimalState * state, const int & flag)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (!(triggerType & flag)) return 0.0;
        return pref->cost;
    }
                    
    return 0.0;
}

void AtMostOnceFSA::noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate)
{
    
}

pair<int,int> AtMostOnceFSA::extendLP(map<int,list<int> > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
    static const bool atMostOnceExtendDebug = false;
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (atMostOnceExtendDebug || PreferenceHandler::preferenceDebug) cout << "Not extending LP for at-most-once constraint " << prefIdx << ": can never be reached\n";
                return make_pair(-1,-1);
        
    }
    
    if (state->preferenceStatus[prefIdx] != seenoncealready) {
        if (atMostOnceExtendDebug || PreferenceHandler::preferenceDebug) cout << "Not extending LP for at-most-once constraint " << prefIdx << ": " << positionName[state->preferenceStatus[prefIdx]] << endl;
                return make_pair(-1,-1);
        
    }
                
    if (atMostOnceExtendDebug || PreferenceHandler::preferenceDebug) {
        cout << "Extending LP for at-most-once constraint " << prefIdx << ": potential for it to be broken\n";
    }
                  
    const int toReturn = model->giveMeAColumn("is-violated " + pref->name);
    
    vector<pair<int,double> > entries(1, make_pair(toReturn, BIG));
    
    const int newRow = model->addSwitchB(switchVarName, entries, 0.0, true); // Can only be triggered by actions known to cause it to be true
    
    return make_pair(toReturn,newRow);
                            
                                
};

void AtMostOnceFSA::addHelperTriggers(const vector<pair<int,int> > & switchVarForPreference, MinimalState* state, MILPRPG* model)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) return;
    if (state->preferenceStatus[prefIdx] != seenoncealready) return;
        
    if (!triggerPartCount) return;
                
    const int startVar = model->giveMeTriggerColumns(prefIdx, triggerPartCount, false).first;
    
    PrefCommon::buildTriggersOnEndValues(startVar, pref, model, true);    
    
}



void AtMostOnceFSA::anotherPreferenceWants(const map<int,list<int> > & factNeededForCol,MinimalState * state, MILPRPG * model)
{    
    if (state->preferenceStatus[prefIdx] == unreachable) return;
    if (state->preferenceStatus[prefIdx] != seenoncealready) return;
    
    if (!pref->goal.empty()) return;
            
    if (pref->goalRPGNum.size() > 1) return;
                        
    const list<pair<int,int> >::const_iterator numItr = pref->triggerRPGNum.begin();
            
    int thePre = numItr->first;
    
    if (numItr->second != -1) {
        if (thePre != -1) return;
        thePre = numItr->second;
    } else {
        if (thePre == -1) return;
    }
    
            
    map<int,list<int> >::const_iterator opItr = factNeededForCol.find(thePre);
                
    if (opItr == factNeededForCol.end()) return;
                    
    model->preferenceTrippedBy(prefIdx, opItr->second);
                
};

void AtMostOnceFSA::importTriggerGroups(map<int, set<int> > & d, int & extraVars)
{
    if (!actionsImplyingTrigger.empty()) {
        d[-1 - prefIdx] = actionsImplyingTrigger;
    }
        
    triggerPartCount = 0;
    
    list<pair<int,int> >::const_iterator tItr = pref->triggerRPGNum.begin();
    const list<pair<int,int> >::const_iterator tItrEnd = pref->triggerRPGNum.end();
    for (; tItr != tItrEnd; ++tItr) {
        if (tItr->first != -1) ++triggerPartCount;
        if (tItr->second != -1) ++triggerPartCount;
    }
    extraVars += triggerPartCount;
    extraVars += 2;
                
}

void AtMostOnceFSA::getUnsatisfiedConditionCounts(MinimalState & state, vector<int> & toFill)
{
    // Don't actually try to satisfy at-most-once preferences - just avoid breaking them
}

void AtMostOnceFSA::getCostsOfAdding(MinimalState & theState, map<int, AddingConstraints > & factCost, map<int, map<double, AddingConstraints > > & numChangeCost)
{
    if (theState.preferenceStatus[prefIdx] != seenoncealready) return;
    if (!addingOneThingCanTrigger) return;
                    
    static const int varCount = RPGBuilder::getPNECount();
        
    const double pCost = pref->cost;
    
    if (pCost == 0.0) return;
            
                
                                                    
    if (addingThisWouldTrigger.second) {
        RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[addingThisWouldTrigger.first];
        
        if (currPre.LHSVariable < varCount) {
            double delta = currPre.RHSConstant - theState.second[currPre.LHSVariable];
            if (delta >= 0.0) delta = -1e-6;
                        numChangeCost[currPre.LHSVariable][delta].addingWillViolate.insert(prefIdx);
        } else if (currPre.LHSVariable < 2 * varCount) {
            double delta = -currPre.RHSConstant - theState.second[currPre.LHSVariable - varCount];
            if (delta <= 0.0) delta = 1e-6;
                        numChangeCost[currPre.LHSVariable - varCount][delta].addingWillViolate.insert(prefIdx);
        } else {
            const RPGBuilder::ArtificialVariable & currAV =  RPGBuilder::getArtificialVariable(currPre.LHSVariable);
            if (currAV.size == 1) {
                double rhc = currPre.RHSConstant - currAV.constant;
                rhc /= currAV.weights[0];
                
                if (currAV.fluents[0] < varCount) {
                    double delta = rhc - theState.second[currAV.fluents[0]];
                    if (delta >= 0.0) delta = -1e-6;
                                         numChangeCost[currAV.fluents[0]][delta].addingWillViolate.insert(prefIdx);
                } else {
                    double delta = -rhc - theState.second[currAV.fluents[0] - varCount];
                    if (delta <= 0.0) delta = 1e-6;
                                        numChangeCost[currAV.fluents[0] - varCount][delta].addingWillViolate.insert(prefIdx);
                }
            }
        }        
    } else {
        factCost[addingThisWouldTrigger.first].addingWillViolate.insert(prefIdx);
    }
                                                                                                                        
}


/// sometime-after constraints

SometimeAfterFSA::SometimeAfterFSA(const int & idx, RPGBuilder::Preference * const p, AutomatonPosition & initPosn)
: PreferenceFSA(idx, p), triggerPartCount(0)
{
    assert(pref->cons == VAL::E_SOMETIMEAFTER);
    initPosn = satisfied;
    
    PrefCommon::actionsWhichMeanWeHad(actionsImplyingTrigger, pref->triggerRPGNum, pref->trigger);
    addingOneThingCanTrigger = true;
    addingThisWouldTrigger = make_pair(-1,true);
    
    const int litTrigCount = pref->trigger.size();
    
    if (litTrigCount > 1) {
        addingOneThingCanTrigger = false;
        return;
    }
    
    if (litTrigCount) {
        addingThisWouldTrigger = make_pair(pref->trigger.front()->getID(), false);
    }
    
    const int numTrigCount = pref->triggerRPGNum.size();
    
    if (numTrigCount > 1) {
        addingOneThingCanTrigger = false;
        return;
    }
    
    if (numTrigCount) {
        const pair<int,int> & numPair = pref->triggerRPGNum.front();
        for (int pass = 0; pass < 2; ++pass) {
            const int preIdx = (pass ? numPair.second : numPair.first);
            if (preIdx == -1) continue;
            if (addingThisWouldTrigger.first != -1) {
                addingOneThingCanTrigger = false;
                return;
            }
            addingThisWouldTrigger = make_pair(preIdx, true);
        }
    }
    
    if (addingThisWouldTrigger.first == -1) {
        addingOneThingCanTrigger = false;
    }
}

void SometimeAfterFSA::update(MinimalState * state)
{
    const bool sometimeAfterUpdateDebug = false;
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) {
            cout << prefIdx << ": update: sometime-after pref " << pref->name << " cannot be satisfied from here, cost " << pref->cost << "\n";
        }
        return;
    }

    if (state->preferenceStatus[prefIdx] == satisfied) {
        bool allPresent = true;
        
        list<Literal*>::const_iterator factItr = pref->trigger.begin();
        const list<Literal*>::const_iterator factEnd = pref->trigger.end();
        
        for (; factItr != factEnd; ++factItr) {
            if (state->first.find((*factItr)->getID()) == state->first.end()) {
                if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) {
                    cout << prefIdx << ": update: sometime-after pref " << pref->name << " won't trigger\n";
                }
                allPresent = false;
                break;
            }
        }
        
        if (allPresent) {
            list<pair<int,int> >::const_iterator numItr = pref->triggerRPGNum.begin();
            const list<pair<int,int> >::const_iterator numEnd = pref->triggerRPGNum.end();
            
            for (; numItr != numEnd; ++numItr) {
                bool bothMatch = true;
                for (int pass = 0; bothMatch && pass < 2; ++pass) {
                    const int preIdx = (pass ? numItr->second : numItr->first);
                    
                    if (preIdx < 0) continue;
                    //cout << "Working out (with calculations) if pre " << preIdx << " is satisfied\n";
                    assert((uint) preIdx < RPGBuilder::getNumericPrecs().size());                    
                    RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];                                    
                    //cout << currPre << endl;
                    
                    bothMatch = currPre.isSatisfiedWCalculate(state->second);
                }
                if (!bothMatch) {
                    if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime-after pref " << pref->name << " won't trigger\n";
                    allPresent = false;
                    break;
                }
            }
        }
        
        if (allPresent) {
            if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) {
                cout << prefIdx << ": update: sometime-after pref " << pref->name << " has been triggered\n";
            }
            state->preferenceStatus[prefIdx] = triggered;
        }
                    
    }
    
    if (state->preferenceStatus[prefIdx] != triggered) {
        if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) {
            cout << prefIdx << ": update: sometime-after pref hasn't been triggered\n";
        }
        return;
    }

    if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) {
        cout << prefIdx << ": update: seeing if goal for sometime-after is true\n";
    }

    list<Literal*>::const_iterator factItr = pref->goal.begin();
    const list<Literal*>::const_iterator factEnd = pref->goal.end();
    
    for (; factItr != factEnd; ++factItr) {
        if (state->first.find((*factItr)->getID()) == state->first.end()) {
            if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) {
                cout << prefIdx << ": update: goal of sometime-after pref " << pref->name << " not met\n";
            }
            return;
        }
    }


    list<pair<int,int> >::const_iterator numItr = pref->goalRPGNum.begin();
    const list<pair<int,int> >::const_iterator numEnd = pref->goalRPGNum.end();
    
    for (; numItr != numEnd; ++numItr) {
        bool bothMatch = true;
        for (int pass = 0; bothMatch && pass < 2; ++pass) {
            const int preIdx = (pass ? numItr->second : numItr->first);
            if (preIdx < 0) continue;
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
            bothMatch = currPre.isSatisfiedWCalculate(state->second);
            if (!bothMatch) {
                if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) {
                    cout << "  - Don't have " << currPre << endl;
                }
            }
        }
        if (!bothMatch) {
            if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: goal of sometime-after pref " << pref->name << " not met\n";
            return;            
        }
    }
    
    if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime-after pref " << pref->name << " is now satisfied, cost " << pref->cost << "\n";
    
    state->preferenceStatus[prefIdx] = satisfied;
      
}

double SometimeAfterFSA::currentCost(MinimalState * state, const int & flag)
{
    
    if (state->preferenceStatus[prefIdx] == triggered || state->preferenceStatus[prefIdx] == unreachable) {
        if (!(goalType & flag)) return 0.0;
        if (PreferenceHandler::preferenceDebug) {
            if (state->preferenceStatus[prefIdx] == triggered) {
                cout << prefIdx << ": sometime-after pref " << pref->name << " is currently unsatisfied, cost " << pref->cost << "\n";
            } else {
                cout << prefIdx << ": sometime-after pref " << pref->name << " cannot be reached, cost " << pref->cost << "\n";
            }
        }
        return pref->cost;
    }
    
    if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": sometime-after pref " << pref->name << " is satisfied\n";
    return 0.0;
}

double SometimeAfterFSA::reachableCost(MinimalState * state, const int & flag)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (!(goalType & flag)) return 0.0;
        return pref->cost;
    }
        
    return 0.0;
}

double SometimeAfterFSA::GCost(MinimalState * state, const int & flag)
{    
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (!(goalType & flag)) return 0.0;
        return pref->cost;
    }
            
    return 0.0;
}

void SometimeAfterFSA::noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate)
{
    PrefCommon::noteVariablesForFactAtSomePoint(toUpdate, prefIdx, pref);
}

pair<int,int> SometimeAfterFSA::extendLP(map<int,list<int> > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
    static const bool sometimeAfterExtendDebug = false;
    
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (PreferenceHandler::preferenceDebug || sometimeAfterExtendDebug) cout << "Not extending LP for sometime-after constraint " << prefIdx << ": can never be reached\n";
        return make_pair(-1,-1);
        
    }
    if (state->preferenceStatus[prefIdx] == triggered) {
        if (PreferenceHandler::preferenceDebug || sometimeAfterExtendDebug) cout << "Extending LP for triggered sometime-after constraint " << prefIdx << endl;
        
        //const bool debugWas = PreferenceHandler::preferenceDebug;
        if (sometimeAfterExtendDebug) PreferenceHandler::preferenceDebug = true;
        const pair<int,int> toReturn = PrefCommon::extendLPForFactAtSomePoint(state, model, switchVarName, prefIdx, pref);
        //PreferenceHandler::preferenceDebug = debugWas;
        
        list<pair<int,int> >::const_iterator gfItr = pref->goalRPGNum.begin();
        const list<pair<int,int> >::const_iterator gfEnd = pref->goalRPGNum.end();
        
        for (; gfItr != gfEnd; ++gfItr) {
            if (gfItr->first != -1) {
                factNeededForCol[gfItr->first].push_back(prefIdx);
            }
            if (gfItr->second != -1) {
                factNeededForCol[gfItr->second].push_back(prefIdx);
            }
        }
           
           
        return toReturn;
    }
    
    assert(state->preferenceStatus[prefIdx] == satisfied);
    
    const bool oldDebug = PreferenceHandler::preferenceDebug;
    PreferenceHandler::preferenceDebug = sometimeAfterExtendDebug;
    
    
    if (PreferenceHandler::preferenceDebug || sometimeAfterExtendDebug) cout << "Extending LP for sometime-after constraint " << prefIdx << endl;
    
    const pair<int,int> toReturn = PrefCommon::extendLPForFactAtSomePoint(state, model, switchVarName, prefIdx, pref, true, true);
    PreferenceHandler::preferenceDebug = oldDebug;
    return toReturn;
};

void SometimeAfterFSA::addHelperTriggers(const vector<pair<int,int> > & switchVarForPreference, MinimalState* state, MILPRPG* model)
{
    
    if (!triggerPartCount) return;
    if (state->preferenceStatus[prefIdx] == unreachable) return;
        
    //static const int varCount = RPGBuilder::getPNECount();
    
    const pair<int,int> startVar = model->giveMeTriggerColumns(prefIdx, triggerPartCount, true);
    
    PrefCommon::buildTriggersOnEndValues(startVar.first, pref, model);
    
    PrefCommon::extendLPForFactAtEnd(state,model,"fae" + pref->name ,prefIdx,pref,switchVarForPreference[prefIdx].first,startVar.second);
                                       
        
}


void SometimeAfterFSA::anotherPreferenceWants(const map<int,list<int> > & factNeededForCol,MinimalState * state, MILPRPG * model)
{    
    if (state->preferenceStatus[prefIdx] != satisfied) return;

    if (!pref->trigger.empty()) return;    
    if (pref->triggerRPGNum.size() > 1) return;
        
    const list<pair<int,int> >::const_iterator numItr = pref->triggerRPGNum.begin();
    
    int thePre = numItr->first;
    
    if (numItr->second != -1) {
        if (thePre != -1) return;
        thePre = numItr->second;
    } else {
        if (thePre == -1) return;
    }
    
    map<int,list<int> >::const_iterator opItr = factNeededForCol.find(thePre);
    
    if (opItr == factNeededForCol.end()) return;
    
    model->preferenceTrippedBy(prefIdx, opItr->second);
            
};

void SometimeAfterFSA::importTriggerGroups(map<int, set<int> > & d, int & extraVars)
{
    if (!actionsImplyingTrigger.empty()) {
        d[-1 - prefIdx] = actionsImplyingTrigger;
    }

    triggerPartCount = 0;

    list<pair<int,int> >::const_iterator tItr = pref->triggerRPGNum.begin();
    const list<pair<int,int> >::const_iterator tItrEnd = pref->triggerRPGNum.end();
    for (; tItr != tItrEnd; ++tItr) {
        if (tItr->first != -1) ++triggerPartCount;
        if (tItr->second != -1) ++triggerPartCount;
    }
    
    if (triggerPartCount) {
        extraVars += triggerPartCount;
        extraVars += 2;
    }
    
}

void SometimeAfterFSA::getUnsatisfiedConditionCounts(MinimalState & state, vector<int> & toFill)
{
    if (state.preferenceStatus[prefIdx] == unreachable) {
        // If we can never satisfy the preference, don't bother trying
        return;
    }
        
    toFill[0] = initialUnsatisfiedGoalPreconditions;
    toFill[1] = initialUnsatisfiedTriggerPreconditions;
}

void SometimeAfterFSA::getCostsOfAdding(MinimalState & theState, map<int, AddingConstraints > & factCost, map<int, map<double, AddingConstraints > > & numChangeCost)
{
    if (theState.preferenceStatus[prefIdx] == unreachable || theState.preferenceStatus[prefIdx] == triggered) return;
    if (!addingOneThingCanTrigger) return;
    
    static const int varCount = RPGBuilder::getPNECount();
    
    const double pCost = pref->cost;
    
    if (pCost == 0.0) return;
    
    bool allPresent = true;
        
    {        
        list<Literal*>::const_iterator factItr = pref->goal.begin();
        const list<Literal*>::const_iterator factEnd = pref->goal.end();
        
        int fID;
        for (; factItr != factEnd; ++factItr) {
            fID = (*factItr)->getID();
            if (theState.first.find(fID) == theState.first.end()) {
                allPresent = false;
                break;
            }
        }
    }
                
    if (allPresent) {
        list<pair<int,int> >::const_iterator numItr = pref->goalRPGNum.begin();
        const list<pair<int,int> >::const_iterator numEnd = pref->goalRPGNum.end();
        
        int pass, preIdx;
        
        for (; numItr != numEnd; ++numItr) {
            bool bothMatch = true;
            for (pass = 0;pass < 2; ++pass) {
                preIdx = (pass ? numItr->second : numItr->first);
                if (preIdx == -1) continue;
                
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
                
                if (!currPre.isSatisfiedWCalculate(theState.second)) {
                    bothMatch = false;
                    break;
                }
            }
            if (!bothMatch) {
                allPresent = false;
                break;
            }
        }
    }
    
    if (allPresent) {
        /*if (addingThisWouldTrigger.second) {
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[addingThisWouldTrigger.first];
            cout << "No fear from " << pref->name << " of satisfying " << currPre << " - is already satisfied\n";
        }*/
        return;
    }
    
    if (addingThisWouldTrigger.second) {
        RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[addingThisWouldTrigger.first];
        //cout << "Recording fear from " << pref->name << " of satisfying " << currPre << endl;
        
        if (currPre.LHSVariable < varCount) {
            double delta = currPre.RHSConstant - theState.second[currPre.LHSVariable];
            if (delta >= 0.0) delta = -1e-6;
            numChangeCost[currPre.LHSVariable][delta].addingWillViolate.insert(prefIdx);
        } else if (currPre.LHSVariable < 2 * varCount) {
            double delta = -currPre.RHSConstant - theState.second[currPre.LHSVariable - varCount];
            if (delta <= 0.0) delta = 1e-6;
            numChangeCost[currPre.LHSVariable - varCount][delta].addingWillViolate.insert(prefIdx);
        } else {
            const RPGBuilder::ArtificialVariable & currAV =  RPGBuilder::getArtificialVariable(currPre.LHSVariable);
            if (currAV.size == 1) {
                double rhc = currPre.RHSConstant - currAV.constant;
                rhc /= currAV.weights[0];
                
                if (currAV.fluents[0] < varCount) {
                    double delta = rhc - theState.second[currAV.fluents[0]];
                    if (delta >= 0.0) delta = -1e-6;
                    numChangeCost[currAV.fluents[0]][delta].addingWillViolate.insert(prefIdx);
                } else {
                    double delta = -rhc - theState.second[currAV.fluents[0] - varCount];
                    if (delta <= 0.0) delta = 1e-6;
                    numChangeCost[currAV.fluents[0] - varCount][delta].addingWillViolate.insert(prefIdx);
                }
            }
        }        
    } else {
        factCost[addingThisWouldTrigger.first].addingWillViolate.insert(prefIdx);
    }
                        
}

void SometimeAfterFSA::updateCosts(MinimalState & initialState,
                                   const bool & wasTheTrigger,
                                   vector<AutomatonPosition> & optimisticAutomataPositions,
                                   map<int, AddingConstraints > & prefCostOfAddingFact,
                                   map<int, map<double, AddingConstraints > > & numChangeCost,
                                   list<int> & preferencesThatNowHaveNoAddCosts)
{
    
    if (wasTheTrigger) return;
    if (optimisticAutomataPositions[prefIdx] == unreachable) return;
    
    if (optimisticAutomataPositions[prefIdx] == triggered) {
        optimisticAutomataPositions[prefIdx] = satisfied;
        return;
    }
    
    if (!addingOneThingCanTrigger) return;
    if (addingThisWouldTrigger.first == -1) return;
    
    //cout << "Now no longer have to fear seeing the trigger of " << pref->name << " - the goal is reachable\n";
    
    static const int varCount = RPGBuilder::getPNECount();
    if (addingThisWouldTrigger.second) {
        
        RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[addingThisWouldTrigger.first];
        
        //cout << "Removing fear of satisfying " << currPre << endl;
        
        if (currPre.LHSVariable < varCount) {
            map<int, map<double, AddingConstraints > >::iterator varItr = numChangeCost.find(currPre.LHSVariable);
            if(varItr != numChangeCost.end()) {
                double delta = currPre.RHSConstant - initialState.second[currPre.LHSVariable];
                if (delta >= 0.0) delta = -1e-6;
                
                map<double, AddingConstraints >::iterator dItr = varItr->second.find(delta);
                if (dItr != varItr->second.end()) {
                
                    dItr->second.addingWillViolate.erase(prefIdx);
                    dItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));
                }
            }
        } else if (currPre.LHSVariable < 2 * varCount) {
            double delta = -currPre.RHSConstant - initialState.second[currPre.LHSVariable - varCount];
            if (delta <= 0.0) delta = 1e-6;
            
            map<int, map<double, AddingConstraints > >::iterator varItr = numChangeCost.find(currPre.LHSVariable - varCount);
            if (varItr != numChangeCost.end()) {
            
                map<double, AddingConstraints >::iterator dItr = varItr->second.find(delta);
                if (dItr != varItr->second.end()) {
                
                    dItr->second.addingWillViolate.erase(prefIdx);
                    dItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));
                }
            }
        } else {
            const RPGBuilder::ArtificialVariable & currAV =  RPGBuilder::getArtificialVariable(currPre.LHSVariable);
            if (currAV.size == 1) {
                double rhc = currPre.RHSConstant - currAV.constant;
                rhc /= currAV.weights[0];
                
                if (currAV.fluents[0] < varCount) {
                    double delta = rhc - initialState.second[currAV.fluents[0]];
                    if (delta >= 0.0) delta = -1e-6;
                    
                    map<int, map<double, AddingConstraints > >::iterator varItr = numChangeCost.find(currAV.fluents[0]);
                    if (varItr != numChangeCost.end()) {
                    
                        map<double, AddingConstraints >::iterator dItr = varItr->second.find(delta);
                        if (dItr != varItr->second.end()) {
                        
                            dItr->second.addingWillViolate.erase(prefIdx);
                            dItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));
                        }
                    }
                   
                    
                } else {
                    double delta = -rhc - initialState.second[currAV.fluents[0] - varCount];
                    if (delta <= 0.0) delta = 1e-6;
                    
                    map<int, map<double, AddingConstraints > >::iterator varItr = numChangeCost.find(currAV.fluents[0] - varCount);
                    if (varItr != numChangeCost.end()) {
                    
                        map<double, AddingConstraints >::iterator dItr = varItr->second.find(delta);
                        if (dItr != varItr->second.end()) {
                        
                            dItr->second.addingWillViolate.erase(prefIdx);
                            dItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));
                        }
                    }
                }
            }
        }        
    } else {
        map<int,AddingConstraints >::iterator fcItr = prefCostOfAddingFact.find(addingThisWouldTrigger.first);
        if (fcItr != prefCostOfAddingFact.end()) {
        
            fcItr->second.addingWillViolate.erase(prefIdx);
            fcItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));             
        }
    }
    
    preferencesThatNowHaveNoAddCosts.push_back(prefIdx);

}


void SometimeAfterFSA::getDesiredGoals(list<list<Literal*> * > & desired, MinimalState & startState, const vector<vector<int> > & unsatisfiedPreferenceConditions)
{
    if (startState.preferenceStatus[prefIdx] != triggered) return;
    if (unsatisfiedPreferenceConditions[prefIdx][0]) return;
                    
    desired.push_back(&(pref->goal));
}

/// Sometime-before constraints

SometimeBeforeFSA::SometimeBeforeFSA(const int & idx, RPGBuilder::Preference * const p, AutomatonPosition & initPosn)
: PreferenceFSA(idx, p), triggerPartCount(0)
{
    assert(pref->cons == VAL::E_SOMETIMEBEFORE);
    initPosn = satisfied;
    
    PrefCommon::actionsWhichMeanWeHad(actionsImplyingTrigger, pref->triggerRPGNum, pref->trigger);
    
    addingOneThingCanTrigger = true;
    addingThisWouldTrigger = make_pair(-1,true);
    
    const int litTrigCount = pref->trigger.size();
    
    if (litTrigCount > 1) {
        addingOneThingCanTrigger = false;
        return;
    }
        
    if (litTrigCount) {
        addingThisWouldTrigger = make_pair(pref->trigger.front()->getID(), false);
    }
                
    const int numTrigCount = pref->triggerRPGNum.size();
    
    if (numTrigCount > 1) {
        addingOneThingCanTrigger = false;
        return;
    }
                        
    if (numTrigCount) {
        const pair<int,int> & numPair = pref->triggerRPGNum.front();
        for (int pass = 0; pass < 2; ++pass) {
            const int preIdx = (pass ? numPair.second : numPair.first);
            if (preIdx == -1) continue;
            if (addingThisWouldTrigger.first != -1) {
                addingOneThingCanTrigger = false;
                return;
            }
            addingThisWouldTrigger = make_pair(preIdx, true);
        }
    }
                                
    if (addingThisWouldTrigger.first == -1) {
        addingOneThingCanTrigger = false;
    }
}

void SometimeBeforeFSA::update(MinimalState * state)
{
    const bool sometimeBeforeUpdateDebug = false;
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (sometimeBeforeUpdateDebug || PreferenceHandler::preferenceDebug) {
            cout << prefIdx << ": update: sometime-before pref " << pref->name << " cannot be satisfied from here, cost " << pref->cost << "\n";
        }
        return;
    }

    if (state->preferenceStatus[prefIdx] == eternallysatisfied) {
        if (sometimeBeforeUpdateDebug || PreferenceHandler::preferenceDebug) {
            cout << prefIdx << ": update: sometime-before pref " << pref->name << " cannot be unsatisfied from here, cost " << pref->cost << "\n";
        }
        return;
    }
    
    
    {

        bool allPresent = true;
        
        list<Literal*>::const_iterator factItr = pref->trigger.begin();
        const list<Literal*>::const_iterator factEnd = pref->trigger.end();
        
        for (; factItr != factEnd; ++factItr) {
            if (state->first.find((*factItr)->getID()) == state->first.end()) {
                if (sometimeBeforeUpdateDebug || PreferenceHandler::preferenceDebug) {
                }
                allPresent = false;
                break;                
            }
        }

        if (allPresent) {
                    
            list<pair<int,int> >::const_iterator numItr = pref->triggerRPGNum.begin();
            const list<pair<int,int> >::const_iterator numEnd = pref->triggerRPGNum.end();
            
            for (; numItr != numEnd; ++numItr) {
                bool bothMatch = true;
                for (int pass = 0; bothMatch && pass < 2; ++pass) {
                    const int preIdx = (pass ? numItr->second : numItr->first);
                    if (preIdx < 0) continue;
                                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
                    bothMatch = currPre.isSatisfiedWCalculate(state->second);
                }
                if (!bothMatch) {
                    allPresent = false;
                    break;
                }
            }
        }
                        
        if (allPresent) {
            
            if (sometimeBeforeUpdateDebug || PreferenceHandler::preferenceDebug) {
                cout << prefIdx << ": update: sometime-before pref " << pref->name << " is now broken forever, cost " << pref->cost << "\n";
            }
                        
            state->preferenceStatus[prefIdx] = unreachable;
            return;
        }

        
    }
    
    list<Literal*>::const_iterator factItr = pref->goal.begin();
    const list<Literal*>::const_iterator factEnd = pref->goal.end();
    
    for (; factItr != factEnd; ++factItr) {
        if (state->first.find((*factItr)->getID()) == state->first.end()) {
            if (sometimeBeforeUpdateDebug || PreferenceHandler::preferenceDebug) {
                cout << prefIdx << ": update: sometime-before pref " << pref->name << " won't trigger - don't have " << *(*factItr) << "\n";
            }
            return;
        }
    }    
                    
    list<pair<int,int> >::const_iterator numItr = pref->goalRPGNum.begin();
    const list<pair<int,int> >::const_iterator numEnd = pref->goalRPGNum.end();
    
    for (; numItr != numEnd; ++numItr) {
        bool bothMatch = true;
        int sIdx = -1;
        for (int pass = 0; bothMatch && pass < 2; ++pass) {
            const int preIdx = (pass ? numItr->second : numItr->first);
            if (preIdx < 0) continue;
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
            bothMatch = currPre.isSatisfiedWCalculate(state->second);
            sIdx = preIdx;
        }
        if (!bothMatch) {
            if (sometimeBeforeUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime-before pref " << pref->name << " won't trigger: don't have " << RPGBuilder::getNumericPrecs()[sIdx] << endl;
            return;
        }
    }
                                    
    if (sometimeBeforeUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime-before pref " << pref->name << " has triggered, eternally satisfied\n";

    
    state->preferenceStatus[prefIdx] = eternallysatisfied;
}

double SometimeBeforeFSA::currentCost(MinimalState * state, const int & flag)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (!(triggerType & flag)) return 0.0;
        if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": sometime-before pref " << pref->name << " cannot be satisfied, cost " << pref->cost << "\n";
        return pref->cost;
    }
        
    if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": sometime-before pref " << pref->name << " is satisfied\n";
    return 0.0;
}

double SometimeBeforeFSA::reachableCost(MinimalState * state, const int & flag)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (!(triggerType & flag)) return 0.0;
        return pref->cost;
    }
            
    return 0.0;
}

double SometimeBeforeFSA::GCost(MinimalState * state, const int & flag)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (!(triggerType & flag)) return 0.0;
        return pref->cost;
    }
                
    return 0.0;
}

void SometimeBeforeFSA::noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate)
{
    PrefCommon::noteVariablesForFactAtSomePoint(toUpdate, prefIdx, pref, false);
}

pair<int,int> SometimeBeforeFSA::extendLP(map<int,list<int> > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
    static const bool sometimeBeforeExtendDebug = false;
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (sometimeBeforeExtendDebug || PreferenceHandler::preferenceDebug) cout << "Not extending LP for sometime-before constraint " << prefIdx << ": can never be reached\n";
        return make_pair(-1,-1);
        
    }
    if (state->preferenceStatus[prefIdx] == eternallysatisfied) {
        if (sometimeBeforeExtendDebug || PreferenceHandler::preferenceDebug) cout << "Not extending LP for sometime-before constraint " << prefIdx << ": already reached\n";
        return make_pair(-1,-1);
        
    }
    assert(state->preferenceStatus[prefIdx] == satisfied);
            
    if (sometimeBeforeExtendDebug || PreferenceHandler::preferenceDebug) {
        cout << "Extending LP for sometime-before constraint " << prefIdx << ": potential for it to be broken\n";
    }
    
    const pair<int,int> toReturn = PrefCommon::extendLPForFactAtSomePoint(state, model, switchVarName, prefIdx, pref, false, true);
    return toReturn;
            
};

void SometimeBeforeFSA::addHelperTriggers(const vector<pair<int,int> > & switchVarForPreference, MinimalState* state, MILPRPG* model)
{

    if (state->preferenceStatus[prefIdx] != satisfied) return;
    
    if (!triggerPartCount) return;
    
    //static const int varCount = RPGBuilder::getPNECount();
    
    const int startVar = model->giveMeTriggerColumns(prefIdx, triggerPartCount, false).first;
    
    PrefCommon::buildTriggersOnEndValues(startVar, pref, model);
    
        
}


void SometimeBeforeFSA::anotherPreferenceWants(const map<int,list<int> > & factNeededForCol,MinimalState * state, MILPRPG * model)
{    
    if (state->preferenceStatus[prefIdx] != satisfied) return;

    if (!pref->trigger.empty()) return;
    
    if (pref->triggerRPGNum.size() > 1) return;
        
    const list<pair<int,int> >::const_iterator numItr = pref->triggerRPGNum.begin();
    
    int thePre = numItr->first;
    
    if (numItr->second != -1) {
        if (thePre != -1) return;
        thePre = numItr->second;
    } else {
        if (thePre == -1) return;
    }
    
    map<int,list<int> >::const_iterator opItr = factNeededForCol.find(thePre);
    
    if (opItr == factNeededForCol.end()) return;
    
    model->preferenceTrippedBy(prefIdx, opItr->second);
            
};

void SometimeBeforeFSA::importTriggerGroups(map<int, set<int> > & d, int & extraVars)
{
    if (!actionsImplyingTrigger.empty()) {
        d[-1 - prefIdx] = actionsImplyingTrigger;
    }
    
    triggerPartCount = 0;
    
    list<pair<int,int> >::const_iterator tItr = pref->triggerRPGNum.begin();
    const list<pair<int,int> >::const_iterator tItrEnd = pref->triggerRPGNum.end();
    for (; tItr != tItrEnd; ++tItr) {
        if (tItr->first != -1) ++triggerPartCount;
        if (tItr->second != -1) ++triggerPartCount;
    }
    extraVars += triggerPartCount;
    extraVars += 2;
    
}

void SometimeBeforeFSA::getUnsatisfiedConditionCounts(MinimalState & state, vector<int> & toFill)
{
    if (state.preferenceStatus[prefIdx] == unreachable || state.preferenceStatus[prefIdx] == eternallysatisfied) {
        // If we can never satisfy/unsatisfy the preference, don't bother trying
        return;
    }
            
    toFill[0] = initialUnsatisfiedGoalPreconditions;
    toFill[1] = initialUnsatisfiedTriggerPreconditions;
}

void SometimeBeforeFSA::getCostsOfAdding(MinimalState & theState, map<int, AddingConstraints > & factCost, map<int, map<double, AddingConstraints > > & numChangeCost)
{
    if (theState.preferenceStatus[prefIdx] != satisfied) return;
    if (!addingOneThingCanTrigger) return;
            
    static const int varCount = RPGBuilder::getPNECount();
        
    const double pCost = pref->cost;
    
    if (pCost == 0.0) return;
        
    
                                    
    if (addingThisWouldTrigger.second) {
        RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[addingThisWouldTrigger.first];
        
        if (currPre.LHSVariable < varCount) {
            double delta = currPre.RHSConstant - theState.second[currPre.LHSVariable];
            if (delta >= 0.0) delta = -1e-6;
            numChangeCost[currPre.LHSVariable][delta].addingWillViolate.insert(prefIdx);
        } else if (currPre.LHSVariable < 2 * varCount) {
            double delta = -currPre.RHSConstant - theState.second[currPre.LHSVariable - varCount];
            if (delta <= 0.0) delta = 1e-6;
            numChangeCost[currPre.LHSVariable - varCount][delta].addingWillViolate.insert(prefIdx);
        } else {
            const RPGBuilder::ArtificialVariable & currAV =  RPGBuilder::getArtificialVariable(currPre.LHSVariable);
            if (currAV.size == 1) {
                double rhc = currPre.RHSConstant - currAV.constant;
                rhc /= currAV.weights[0];
                
                if (currAV.fluents[0] < varCount) {
                    double delta = rhc - theState.second[currAV.fluents[0]];
                    if (delta >= 0.0) delta = -1e-6;
                     numChangeCost[currAV.fluents[0]][delta].addingWillViolate.insert(prefIdx);
                } else {
                    double delta = -rhc - theState.second[currAV.fluents[0] - varCount];
                    if (delta <= 0.0) delta = 1e-6;
                    numChangeCost[currAV.fluents[0] - varCount][delta].addingWillViolate.insert(prefIdx);
                }
            }
        }        
    } else {
        factCost[addingThisWouldTrigger.first].addingWillViolate.insert(prefIdx);
    }
                                                                
}

void SometimeBeforeFSA::updateCosts(MinimalState & initialState,
                                    const bool & wasTheTrigger,
                                    vector<AutomatonPosition> & optimisticAutomataPositions,
                                    map<int, AddingConstraints > & prefCostOfAddingFact,
                                    map<int, map<double, AddingConstraints > > & numChangeCost,
                                    list<int> & preferencesThatNowHaveNoAddCosts)
{
                                       
    if (wasTheTrigger) return;
    if (optimisticAutomataPositions[prefIdx] == unreachable || optimisticAutomataPositions[prefIdx] == eternallysatisfied) return;
                                               
    optimisticAutomataPositions[prefIdx] = eternallysatisfied;
    
                                                       
    if (!addingOneThingCanTrigger) return;
                                                           
                                                           
    static const int varCount = RPGBuilder::getPNECount();
    if (addingThisWouldTrigger.second) {
        RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[addingThisWouldTrigger.first];

        if (currPre.LHSVariable < varCount) {
            map<int, map<double, AddingConstraints > >::iterator varItr = numChangeCost.find(currPre.LHSVariable);
            assert(varItr != numChangeCost.end());
            double delta = currPre.RHSConstant - initialState.second[currPre.LHSVariable];
            if (delta >= 0.0) delta = -1e-6;
                       
            map<double, AddingConstraints >::iterator dItr = varItr->second.find(delta);
            assert(dItr != varItr->second.end());

            dItr->second.addingWillViolate.erase(prefIdx);
            dItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));

                       
        } else if (currPre.LHSVariable < 2 * varCount) {
            double delta = -currPre.RHSConstant - initialState.second[currPre.LHSVariable - varCount];
            if (delta <= 0.0) delta = 1e-6;
                       
            map<int, map<double, AddingConstraints > >::iterator varItr = numChangeCost.find(currPre.LHSVariable - varCount);
            assert(varItr != numChangeCost.end());
           
            map<double, AddingConstraints >::iterator dItr = varItr->second.find(delta);
            assert(dItr != varItr->second.end());
           
            dItr->second.addingWillViolate.erase(prefIdx);
            dItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));
           
        } else {
            const RPGBuilder::ArtificialVariable & currAV =  RPGBuilder::getArtificialVariable(currPre.LHSVariable);
            if (currAV.size == 1) {
                double rhc = currPre.RHSConstant - currAV.constant;
                rhc /= currAV.weights[0];

                if (currAV.fluents[0] < varCount) {
                    double delta = rhc - initialState.second[currAV.fluents[0]];
                    if (delta >= 0.0) delta = -1e-6;
                                       
                    map<int, map<double, AddingConstraints  > >::iterator varItr = numChangeCost.find(currAV.fluents[0]);
                    assert(varItr != numChangeCost.end());

                    map<double, AddingConstraints >::iterator dItr = varItr->second.find(delta);
                    assert(dItr != varItr->second.end());

                    dItr->second.addingWillViolate.erase(prefIdx);
                    dItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));
                                       
                                                           
                } else {
                    double delta = -rhc - initialState.second[currAV.fluents[0] - varCount];
                    if (delta <= 0.0) delta = 1e-6;
                                       
                    map<int, map<double, AddingConstraints > >::iterator varItr = numChangeCost.find(currAV.fluents[0] - varCount);
                    assert(varItr != numChangeCost.end());

                    map<double, AddingConstraints >::iterator dItr = varItr->second.find(delta);
                    assert(dItr != varItr->second.end());

                    dItr->second.addingWillViolate.erase(prefIdx);
                    dItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));                    
                }
            }
        }        
    } else {
        map<int, AddingConstraints >::iterator fcItr = prefCostOfAddingFact.find(addingThisWouldTrigger.first);
        assert(fcItr != prefCostOfAddingFact.end());
       
        fcItr->second.addingWillViolate.erase(prefIdx);
        fcItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));
                       
    }
       
    preferencesThatNowHaveNoAddCosts.push_back(prefIdx);
                                                                   
}
                                   


/// Simple preference handler functions

void PreferenceHandler::extendLP(vector<pair<int,int> > & switchVars, MinimalState * state,MILPRPG * model)
{
    
    map<int,list<int> > factNeededForCol;
    
    const int pCount = automata.size();
    switchVars.resize(pCount,make_pair(-1,-1));
    
    for (int p = 0; p < pCount; ++p) {
        switchVars[p] = automata[p]->extendLP(factNeededForCol,state,model);        
    }
    
    /*{
        map<int,list<int> >::const_iterator fncItr = factNeededForCol.begin();
        const map<int,list<int> >::const_iterator fncEnd = factNeededForCol.end();
        
        for (; fncItr != fncEnd; ++fncItr) {
            cout << RPGBuilder::getNumericPrecs()[fncItr->first] << " is needed by at least pref " << fncItr->second.front() << endl;
        }
    }*/
    
    for (int p = 0; p < pCount; ++p) {
        automata[p]->anotherPreferenceWants(factNeededForCol,state,model);
    }
            
}

void PreferenceHandler::addHelperTriggers(const vector<pair<int,int> > & switchVarForPreference, MinimalState* state, MILPRPG* model)
{
    const int pCount = automata.size();

    for (int p = 0; p < pCount; ++p) {
        automata[p]->addHelperTriggers(switchVarForPreference, state, model);
    }
}

void PreferenceHandler::update(MinimalState & state)
{
    if (PreferenceHandler::preferenceDebug) cout << "Updating preference automata following action application\n";
    const int pCount = automata.size();
    
    for (int p = 0; p < pCount; ++p) {
        if (PreferenceHandler::preferenceDebug) cout << "p)"; cout.flush();
        automata[p]->update(&state);
    }
}

double PreferenceHandler::getCurrentCost(MinimalState & state, const int flag)
{
    
    double toReturn = 0.0;
    
    const int pCount = automata.size();
    
    for (int p = 0; p < pCount; ++p) {
        toReturn += automata[p]->currentCost(&state, flag);
    }
    
    return toReturn;
}

double PreferenceHandler::getReachableCost(MinimalState & state, const int flag)
{
    
    double toReturn = 0.0;
    
    const int pCount = automata.size();
    
    for (int p = 0; p < pCount; ++p) {
        toReturn += automata[p]->reachableCost(&state, flag);
    }
        
    return toReturn;
}

double PreferenceHandler::getG(MinimalState & state, const int flag)
{
    
    double toReturn = 0.0;
    
    const int pCount = automata.size();
    
    for (int p = 0; p < pCount; ++p) {
        toReturn += automata[p]->GCost(&state, flag);
    }
            
    return toReturn;
}

void PreferenceHandler::noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate)
{
    const int pCount = automata.size();
    
    for (int p = 0; p < pCount; ++p) {
        cout << "Noting bounds needed for " << p << endl;
        automata[p]->noteWhichVariablesWeMightWantOptimisticBoundsFor(toUpdate);        
    }
        
}

void PreferenceHandler::importTriggerGroups(map<int, set<int> > & d, int & extraVars)
{
    const int pCount = automata.size();
    
    for (int p = 0; p < pCount; ++p) {
        automata[p]->importTriggerGroups(d,extraVars);
    }
}

double PreferenceHandler::markUnreachables(MinimalState & theState , const list<int> & toMark)
{
    double toReturn = 0.0;
    
    list<int>::const_iterator tmItr = toMark.begin();
    const list<int>::const_iterator tmEnd = toMark.end();
    
    for (; tmItr != tmEnd; ++tmItr) {
        if (theState.preferenceStatus[*tmItr] == unreachable) continue;
        assert(automata[*tmItr]->GCost(&theState,3) == 0.0);
        
        theState.preferenceStatus[*tmItr] = unreachable;
        
        toReturn += automata[*tmItr]->GCost(&theState,3);
    }
    
    return toReturn;
}

void PreferenceHandler::getUnsatisfiedConditionCounts(MinimalState & theState, vector<vector<int> > & toFill)
{
    const int pCount = automata.size();
    toFill.resize(pCount, vector<int>(2,-1) );
    
    for (int p = 0; p < pCount; ++p) {
        automata[p]->getUnsatisfiedConditionCounts(theState, toFill[p]);
    }
}

void PreferenceHandler::getCostsOfDeletion(MinimalState & theState, map<int, set<int> > & prefCostOfDeletingFact, map<int, map<double, set<int> > > & prefCostOfChangingNumber)
{
    const int pCount = automata.size();
    for (int p = 0; p < pCount; ++p) {
        automata[p]->getCostsOfDeletion(theState, prefCostOfDeletingFact, prefCostOfChangingNumber);
    }
}

void PreferenceHandler::getCostsOfAdding(MinimalState & theState, map<int, AddingConstraints > & prefCostOfDeletingFact, map<int, map<double, AddingConstraints > > & prefCostOfChangingNumberB)
{
    const int pCount = automata.size();
    for (int p = 0; p < pCount; ++p) {
        automata[p]->getCostsOfAdding(theState, prefCostOfDeletingFact, prefCostOfChangingNumberB);
    }
}

void PreferenceHandler::updateCostsAndPreferenceStatus(MinimalState & initialState, const pair<int,bool> & whatHasBeenSatisfied,
                                                       vector<AutomatonPosition> & optimisticAutomataPositions,
                                                       map<int, AddingConstraints > & prefCostOfAddingFact,
                                                       map<int, map<double, AddingConstraints > > & prefCostOfChangingNumberB,
                                                       list<int> & preferencesThatNowHaveNoAddCosts)
{
    automata[whatHasBeenSatisfied.first]->updateCosts(initialState, whatHasBeenSatisfied.second, optimisticAutomataPositions, prefCostOfAddingFact, prefCostOfChangingNumberB, preferencesThatNowHaveNoAddCosts);
    if (preferenceDebug) {
        cout << "\t\tAfter that, optimistic preference status is " << positionName[optimisticAutomataPositions[whatHasBeenSatisfied.first]] << endl;
    }
}

void PreferenceHandler::getDesiredGoals(list<list<Literal*> * > & desired, MinimalState & startState, const vector<vector<int> > & unsatisfiedPreferenceConditions, const set<int> & prefsUnsatisfied)
{
    const int pCount = automata.size();
    
    set<int>::const_iterator puItr = prefsUnsatisfied.begin();
    const set<int>::const_iterator puEnd = prefsUnsatisfied.end();
    
    int p = 0;
    
    while (p < pCount && puItr != puEnd) {
        if (p < *puItr) {
            automata[p]->getDesiredGoals(desired, startState, unsatisfiedPreferenceConditions);
            ++p;
        } else {
            ++p;
            ++puItr;
        }
    }
    
    for (; p < pCount; ++p) {
        automata[p]->getDesiredGoals(desired, startState, unsatisfiedPreferenceConditions);
    }
    
}

};

