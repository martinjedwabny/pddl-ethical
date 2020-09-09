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
#include "NNF.h"
#include <TypedAnalyser.h>

#include <cstdlib>

using VAL::theTC;

#include <iostream>
#include <sstream>
using std::cerr;
using std::ostringstream;
using std::endl;

const char * positionName[7] = {"satisfied", "unsatisfied", "triggered", "unreachable", "eternally satisfied", "satisfied, but goal seen already and still holds", "satisfied, but if goal seen again, becomes unsatisfied"};

namespace Planner {

SwitchVarInfo noColRowData;
    
class PreferenceData {
    
public:
    
    enum { GoalIdx = 0, TriggerIdx = 1};
    
    friend class PreferenceHandler;
    friend class PreferenceFSA;
    
    
    vector<pair<NNFNode*, bool> > nodes;
    vector<NNF_Flat*> flattened;
    
    vector<list<int> > conditionLiterals;
    vector<list<int> > conditionNegativeLiterals;
    vector<list<int> > conditionNums;
    vector<list<int> > conditionNegativeNums;
    
    PreferenceData()
        : nodes(2, make_pair((NNFNode*)0, false)), flattened(2,(NNF_Flat*)0),
          conditionLiterals(2), conditionNegativeLiterals(2),
          conditionNums(2), conditionNegativeNums(2)
    {        
    }
                                
};


    
    
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
vector<const vector<vector<int> >* > PreferenceHandler::comparisonData;
vector<AutomatonPosition> PreferenceHandler::initialAutomataPositions;

vector<list<LiteralCellDependency<pair<int,bool> > > > PreferenceHandler::preconditionsToPrefs;
vector<list<LiteralCellDependency<pair<int,bool> > > > PreferenceHandler::negativePreconditionsToPrefs;
vector<list<LiteralCellDependency<pair<int,bool> > > > PreferenceHandler::numericPreconditionsToPrefs;
vector<list<LiteralCellDependency<pair<int,bool> > > > PreferenceHandler::negativeNumericPreconditionsToPrefs;

set<pair<int,bool> > PreferenceHandler::defaultTruePrefParts;
int PreferenceHandler::ptrTableInit = 0;
vector<int> PreferenceHandler::relevantNumericPreconditions;

void PreferenceHandler::buildAutomata()
{
 
    //mappingFromFactsToPreferences.resize(RPGBuilder::getLiteralCount());
    //mappingFromNumericFactsToPreferences.resize(RPGBuilder::getNumericPrecs().size());
    
    
    vector<RPGBuilder::Preference> & prefs = RPGBuilder::getPreferences();
    const uint prefCount = prefs.size();
    const uint taskPrefCount = RPGBuilder::getTaskPrefCount();
    
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
        
        for (int fID; isItr != isEnd; ++isItr) {
            fID = (*isItr)->getStateID();
            if (fID != -1) {
                isLocal.insert(fID);
            }
        }
    }
    
    
    
    automata.resize(prefCount, (PreferenceFSA*) 0);
    comparisonData.resize(taskPrefCount, (const vector<vector<int> >*) 0);
    
    initialAutomataPositions.resize(taskPrefCount, unsatisfied);
    initMS.preferenceStatus.resize(taskPrefCount);        
    
    for (uint p = 0; p < taskPrefCount; ++p) {
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
        comparisonData[p] = automata[p]->getComparisonData();
    }
    
    for (uint p = taskPrefCount; p < prefCount; ++p) {
        automata[p] = new PreconditionPref(p, &(prefs[p]));
    }
    
    /*for (uint p = 0; p < prefCount; ++p) {
        automata[p]->populateFactToPreferenceMappings();
    }*/
    
}

#define CDPAIR(x,y,z) cd[x][y] = z; cd[y][x] = -z;

const vector<vector<int> >* AlwaysFSA::getComparisonData()
{
    static vector<vector<int> > cd;
    static bool def = false;
    
    if (!def) {
        cd.resize(4, vector<int>(4));
        for (int i = 0; i < 4; ++i) {
            cd[i][i] = 0;
        }
        CDPAIR(satisfied,unsatisfied,-1)
        def = true;
    }
    
    return &cd;
}

const vector<vector<int> >* AtEndFSA::getComparisonData()
{
       
    static vector<vector<int> > cd;
    static bool def = false;
    
    if (!def) {
        cd.resize(4, vector<int>(4));
        for (int i = 0; i < 4; ++i) {
            cd[i][i] = 0;
        }
        CDPAIR(satisfied,unsatisfied,-1)
        CDPAIR(satisfied,unreachable,-1)
        CDPAIR(unsatisfied,unreachable,-1)
                        
        def = true;
    }
    
    return &cd;
}

const vector<vector<int> >* SometimeFSA::getComparisonData()
{
    static vector<vector<int> > cd;
    static bool def = false;
   
    if (!def) {
        cd.resize(5, vector<int>(5));

        for (int i = 0; i < 5; ++i) {
            cd[i][i] = 0;
        }
        CDPAIR(eternallysatisfied,unsatisfied,-1)
        CDPAIR(eternallysatisfied,unreachable,-1)
        CDPAIR(unsatisfied,unreachable,-1)
        
        def = true;
    }
        
    
    return &cd;
}

const vector<vector<int> >* AtMostOnceFSA::getComparisonData()
{
    static vector<vector<int> > cd;
    static bool def = false;

    if (!def) {
        cd.resize(7, vector<int>(7));
        
        for (int i = 0; i < 7; ++i) {
            cd[i][i] = 0;
        }
        
        CDPAIR(satisfied,seenoncealreadyandstillholds,-1);
        CDPAIR(satisfied,seenoncealready,-1);
        CDPAIR(satisfied,unreachable,-1);
                                     
        CDPAIR(seenoncealreadyandstillholds,seenoncealready,-1);
        CDPAIR(seenoncealreadyandstillholds,unreachable,-1);
        
        CDPAIR(seenoncealready,unreachable,-1);
                                                
        def = true;
    }
    
    
    return &cd;
}


const vector<vector<int> >* SometimeBeforeFSA::getComparisonData()
{
    static vector<vector<int> > cd;
    static bool def = false;

    if (!def) {
        
        cd.resize(5, vector<int>(5));
        
        for (int i = 0; i < 5; ++i) {
            cd[i][i] = 0;
        }
                
        CDPAIR(satisfied,unreachable,-1);
        CDPAIR(satisfied,eternallysatisfied,1);

        CDPAIR(eternallysatisfied,unreachable,-1);
        
        def = true;
    }
        
        
    return &cd;
}

const vector<vector<int> >* SometimeAfterFSA::getComparisonData()
{
    static vector<vector<int> > cd;
    static bool def = false;

    if (!def) {
        cd.resize(4, vector<int>(4));
        
        for (int i = 0; i < 4; ++i) {
            cd[i][i] = 0;
        }
                    
        CDPAIR(satisfied,triggered,-1);
        CDPAIR(satisfied,unreachable,-1);
        
        CDPAIR(triggered,unreachable,-1);
        
        def = true;
    }
                            
    return &cd;
}



/*void PreferenceFSA::populateFactToPreferenceMappings()
{
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
    
}*/


PreferenceFSA::PreferenceFSA(const int & i, RPGBuilder::Preference * const p)
: prefIdx(i), pref(p), triggerType(0), goalType(0), boilerplate(2)
{
    ostringstream c;
    c << "Switch " << p->name;
    switchVarName = c.str();
    
    PreferenceData * const d = pref->d;
    
    if (!d->conditionLiterals[0].empty() || !d->conditionNegativeLiterals[0].empty()) {
        goalType |= 1;
    }
    
    if (!d->conditionLiterals[1].empty() || !d->conditionNegativeLiterals[1].empty()) {
        triggerType |= 1;
    }
    
    if (!d->conditionNums[0].empty() || !d->conditionNegativeNums[0].empty()) {
        goalType |= 2;
    }
    
    if (!d->conditionNums[1].empty() || !d->conditionNegativeNums[1].empty()) {
        triggerType |=2;
    }
        
}

int PreferenceFSA::extraVariablesFor(const int& lookAtTrigger)
{

    assert(lookAtTrigger >= 0);
    assert(lookAtTrigger <= 1);
    
    
    PreferenceData * const d = pref->d;
    
    if (!(d->flattened[lookAtTrigger])) return 0;
    
    const int cellCount = d->flattened[lookAtTrigger]->getCellCount();
    const int * parentIDs = d->flattened[lookAtTrigger]->getParentIDs();
    const bool * isAnd = d->flattened[lookAtTrigger]->cellIsAnAnd();
    
    const NNF_Flat::Cell * cells = d->flattened[lookAtTrigger]->getCells();
    
    d->flattened[lookAtTrigger]->reset();
    

    boilerplate[lookAtTrigger].encodeCellUsingSwitchID.resize(cellCount,-1);
    boilerplate[lookAtTrigger].currentSwitchIDColumnScratch.resize(cellCount, -1);
    
    int & nextSwitch = boilerplate[lookAtTrigger].switchesNeeded = 0;
    
    map<int,pair<int,int> > parentToSwitch;
    
    int pid, cs;
    pair<int,int> ps;
    
    for (int c = 0; c < cellCount; ++c) {
        pid = parentIDs[c];
        if (cells[c].isCell()) {            
            if (pid == -1) {
                assert(c == 0);
                assert(cellCount == 1);
                boilerplate[lookAtTrigger].encodeCellUsingSwitchID[c] = 0;
                ++nextSwitch;
            } else {
                ps = parentToSwitch[pid];
                if (isAnd[pid]) {
                    boilerplate[lookAtTrigger].encodeCellUsingSwitchID[c] = ps.first;
                } else {
                    cs = nextSwitch++;
                    boilerplate[lookAtTrigger].encodeCellUsingSwitchID[c] = cs;
                    
                    boilerplate[lookAtTrigger].switchRowBounds[ps.second].first -= 1.0;
                    boilerplate[lookAtTrigger].switchRowEntries[ps.second].push_back(make_pair(cs, 1.0));                    
                }
            }
        } else {
            if (pid == -1) {
                assert(c == 0);
                boilerplate[lookAtTrigger].encodeCellUsingSwitchID[c] = 0;
                ++nextSwitch;
                cs = 0;
                
            } else {
                ps = parentToSwitch[pid];
                if (isAnd[pid]) {
                    boilerplate[lookAtTrigger].encodeCellUsingSwitchID[c] = ps.first;
                    cs = ps.first;
                } else {
                    cs = nextSwitch++;
                    boilerplate[lookAtTrigger].switchRowBounds[ps.second].first -= 1.0;
                    boilerplate[lookAtTrigger].switchRowEntries[ps.second].push_back(make_pair(cs, 1.0));
                }
            }
            if (isAnd[c]) {
                parentToSwitch[c] = make_pair(cs, -1);
            } else {
                
                if (cs == 0) {
                    boilerplate[lookAtTrigger].switchIsRoot = true;
                }
                vector<pair<int,double> > newRow(1);
                newRow[0] = make_pair(cs, 1);
                boilerplate[lookAtTrigger].switchRowEntries.push_back(newRow);
                boilerplate[lookAtTrigger].switchRowBounds.push_back(make_pair(SMALL, DBL_MAX));
                parentToSwitch[c] = make_pair(cs, boilerplate[lookAtTrigger].switchRowBounds.size() - 1);
            }
        }
    }
    
    return nextSwitch - 1;
}


bool PreferenceFSA::goalOrTriggerIsSatisfied(MinimalState * theState, const int & partIdx)
{
    
    #ifndef NDEBUG
    
    /**
     *  Truth value by assuming it's an AND clause over the specified constituents.
     *  Only makes sense for debugging purposes.  First value is whether it's
     *  defined, second value is what was decided upon.
     */
    pair<bool, bool> secondOpinion(false, false);
    
    if (false) {/// This assumes the domain does *not* use ADL - only uncomment it for debugging purposes
        secondOpinion.first = true;
        secondOpinion.second = true;
        
        {
            const list<int> & litList = pref->d->conditionLiterals[partIdx];
            
            list<int>::const_iterator lItr = litList.begin();
            const list<int>::const_iterator lEnd = litList.end();
            
            for (; lItr != lEnd; ++lItr) {
                if (theState->first.find(*lItr) == theState->first.end()) {
                    secondOpinion.second = false;
                    break;
                }
            }
        }
        
        if (secondOpinion.second) {
            const list<int> & numList = pref->d->conditionNums[partIdx];
            
            list<int>::const_iterator lItr = numList.begin();
            const list<int>::const_iterator lEnd = numList.end();
            
            for (; lItr != lEnd; ++lItr) {
                if (!RPGBuilder::getNumericPrecs()[*lItr].isSatisfiedWCalculate(theState->second)) {
                    secondOpinion.second = false;
                    break;
                }
            }
        }
    }
    
    #endif
    
    NNF_Flat * const f = pref->d->flattened[partIdx];
    if (!f) {
        if (PreferenceHandler::preferenceDebug) {
            if (partIdx) {
                cout << "Preference's trigger considered to always be ";
            } else {                
                cout << "Preference's goal considered to always be ";
            }
            if (pref->d->nodes[partIdx].second) {
                cout << " true\n";
            }  else {
                cout << " false\n";
            }  
        
        }
        #ifndef NDEBUG
        if (secondOpinion.first) {
            assert(secondOpinion.second == pref->d->nodes[partIdx].second);
        }
        #endif
        return pref->d->nodes[partIdx].second;        
    }
    
    f->reset();
    
    const NNF_Flat::Cell * const cells = f->getCells();
    const int cellCount = f->getCellCount();
    
    for (int c = 0; c < cellCount; ++c) {
        if (!(cells[c].isCell())) continue;
        
        if (cells[c].lit) {
            if (cells[c].polarity) {
                if (theState->first.find(cells[c].lit->getStateID()) != theState->first.end()) {
                    f->satisfy(c);
                }
            } else {
                if (theState->first.find(cells[c].lit->getStateID()) != theState->first.end()) {
                    f->unsatisfy(c);
                }
            }
        } else {
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[cells[c].num];
            if (cells[c].polarity) {
                if (currPre.isSatisfiedWCalculate(theState->second)) {
                    f->satisfy(c);
                }
            } else {
                if (currPre.canBeUnsatisfiedWCalculate(theState->second)) {
                    f->unsatisfy(c);
                }
            }
        }
    }
    
    const bool retVal = f->isSatisfied();
    
    #ifndef NDEBUG
    if (secondOpinion.first) {
        assert(secondOpinion.second == retVal);
    }
    #endif
    
    return retVal;
}

double cheapestRootedAt(list<Literal*> & answer, list<int> * answerNumeric,
                        const int & node, const NNF_Flat::Cell * const cells,
                        const bool * cellIsAnAnd, const int * parentIDs, const int & cellCount,
                        vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer)
{

    if (cells[node].isCell()) {
        if (cells[node].lit) {
            if (!cells[node].polarity) {
                return 0.0;
            }
            const list<PreferenceSetAndCost> & currList = (*literalCosts)[cells[node].lit->getStateID()];
            if (currList.empty()) { // then we've never seen it
                return DBL_MAX;
            } else {
                answer.push_back(cells[node].lit);
                return currList.back().cost;
            }
        } else {
            if (answerNumeric) {
                if (!cells[node].polarity) {
                    return 0.0;
                }
                if ((*numericAchievedInLayer)[cells[node].num] >= 0.0) {
                    answerNumeric->push_back(cells[node].num);
                    return 0.0;
                } else {
                    return DBL_MAX;
                }
            } else {
                return 0.0;
            }
        }
    }

    double toReturn;
    if (cellIsAnAnd[node]) {        
        toReturn = 0.0;
        for (int c = node + 1; c < cellCount; ++c) {
            if (parentIDs[c] == node) {
                const double childCost = cheapestRootedAt(answer, answerNumeric, c, cells, cellIsAnAnd, parentIDs, cellCount, literalCosts, numericAchievedInLayer);
                if (childCost == DBL_MAX) {
                    // never seen fact, so this branch is false
                    return DBL_MAX;
                }
                toReturn += childCost;
            }
        }
    } else {
        toReturn = DBL_MAX;
        list<Literal*> incumbent;
        list<int> incumbentNum;
        for (int c = node + 1; c < cellCount; ++c) {
            if (parentIDs[c] == node) {
                list<Literal*> tmp;
                list<int> tmpNum;
                const double childCost = cheapestRootedAt(tmp, (answerNumeric ? &tmpNum : (list<int>*) 0), c, cells, cellIsAnAnd, parentIDs, cellCount, literalCosts, numericAchievedInLayer);
                if (childCost != DBL_MAX) {
                    if (childCost < toReturn) {
                        toReturn = childCost;
                        tmp.swap(incumbent);
                        if (answerNumeric) {
                            tmpNum.swap(incumbentNum);
                        }
                    }
                }
            }
        }
        if (toReturn != DBL_MAX) {
            answer.insert(answer.end(), incumbent.begin(), incumbent.end());
            if (answerNumeric) {
                answerNumeric->insert(answerNumeric->end(), incumbentNum.begin(), incumbentNum.end());
            }
        }
    }
    
    return toReturn;
}

void PreferenceFSA::satisfyAtLowCost(const NNF_Flat * flattened,
                                     vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer,
                                     list<list<Literal*> > & dest, list<list<int> > * destNumeric)
{
    const NNF_Flat::Cell * const cells = flattened->getCells();
    const int cellCount = flattened->getCellCount();
    const bool * cellIsAnAnd = flattened->cellIsAnAnd();
    const int * const parentIDs = flattened->getParentIDs();
    
    dest.push_back(list<Literal*>());
    
    list<Literal*> & answer = dest.back();

    list<int> * answerNumeric = 0;
    
    if (destNumeric) {
        destNumeric->push_back(list<int>());
        answerNumeric = &(destNumeric->back());
    }
    
    cheapestRootedAt(answer, answerNumeric, 0, cells, cellIsAnAnd, parentIDs, cellCount, literalCosts, numericAchievedInLayer);
}

void PreferenceFSA::getPreconditionsToSatisfy(list<Literal*> & pres, list<int> * numericPres, const bool & theTrigger,
                                              vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer)
{
    const NNF_Flat * const flattened = pref->d->flattened[(theTrigger ? 1 : 0)];
    
    if (!flattened) return;
    
    const NNF_Flat::Cell * const cells = flattened->getCells();
    const int cellCount = flattened->getCellCount();
    const bool * cellIsAnAnd = flattened->cellIsAnAnd();
    const int * const parentIDs = flattened->getParentIDs();
    
    cheapestRootedAt(pres, numericPres, 0, cells, cellIsAnAnd, parentIDs, cellCount, literalCosts, numericAchievedInLayer);
}
                                                  

/// common code

namespace PrefCommon {

    /**
     *  Work out if the addition of one fact can trigger a preference - either then
     *  breaking a precarious at-most-once, or a sometime before, or a sometime after.
     */
    void workOutIfAddingOneThingCanTrigger(bool & addingOneThingCanTrigger,
                                           list<ConditionAndPolarity> & addingThisWouldTrigger,
                                           PreferenceData * d, const int & partIdx)
    {
        static const bool oneThingDebug = false;
        
        addingOneThingCanTrigger = true;
        
        /*const int litTrigCount = d->conditionLiterals[partIdx].size();
        const int negativeLitTrigCount = d->conditionNegativeLiterals[partIdx].size();
        const int numTrigCount = d->conditionNums[partIdx].size();
        const int negativeNumTrigCount = d->conditionNegativeNums[partIdx].size();
        
        if (litTrigCount + negativeLitTrigCount + numTrigCount + negativeNumTrigCount == 0) {
            addingOneThingCanTrigger = false;
            return;        
        }*/
        
        if (!d->flattened[partIdx]) {
            addingOneThingCanTrigger = false;
            return;
        }
        
        const int nodeCount = d->flattened[partIdx]->getInteriorNodeCount();
        
        if (nodeCount != 1) {
            addingOneThingCanTrigger = false;
            if (PreferenceHandler::preferenceDebug || oneThingDebug) {
                cout << "Cannot find a single possible trigger - NNF tree contains " << nodeCount << " interior nodes, rather than 1\n";
            }
            return;
        }
        
        const NNF_Flat::Cell * const cells = d->flattened[partIdx]->getCells();
        const int cellCount = d->flattened[partIdx]->getCellCount();
        const bool * const cellIsAnAnd = d->flattened[partIdx]->cellIsAnAnd();
        const int * const parentIDs = d->flattened[partIdx]->getParentIDs();
        
        if (cellIsAnAnd[0]) {
            if (cellCount > 2) {
                addingOneThingCanTrigger = false;
                if (PreferenceHandler::preferenceDebug || oneThingDebug) {
                    cout << "Cannot find a single possible trigger - NNF tree has an AND at the root, and " << (cellCount - 1) << " leaves\n";
                }
                return;
            }
        }
        
        if (PreferenceHandler::preferenceDebug || oneThingDebug) {
            cout << "Preference can be triggered by:\n";
        }
        
        for (int ci = 1; ci < cellCount; ++ci) {
            assert(parentIDs[ci] == 0);
            assert(cells[ci].isCell());
            
            if (cells[ci].lit) {
                if (cells[ci].polarity) {
                    if (PreferenceHandler::preferenceDebug || oneThingDebug) {
                        cout << "\t" << *(cells[ci].lit) << endl;
                    }
                    addingThisWouldTrigger.push_back(ConditionAndPolarity(cells[ci].lit->getStateID(), false, true));        
                } else {
                    if (PreferenceHandler::preferenceDebug || oneThingDebug) {
                        cout << "\t¬" << *(cells[ci].lit) << endl;
                    }                    
                    addingThisWouldTrigger.push_back(ConditionAndPolarity(cells[ci].lit->getStateID(), false, false));
                }
            } else {
                if (cells[ci].polarity) {
                    if (PreferenceHandler::preferenceDebug || oneThingDebug) {
                        cout << "\t" << RPGBuilder::getNumericPrecs()[cells[ci].num] << endl;
                    }                    
                    addingThisWouldTrigger.push_back(ConditionAndPolarity(cells[ci].num, true, true));
                } else {
                    if (PreferenceHandler::preferenceDebug || oneThingDebug) {
                        cout << "\t¬" << RPGBuilder::getNumericPrecs()[cells[ci].num] << endl;
                    }                                        
                    addingThisWouldTrigger.push_back(ConditionAndPolarity(cells[ci].num, true, false));
                }
            }
        }                        
    }
    
    SwitchVarInfo extendLPForFactAtEnd(MinimalState * state,MILPRPG * model,
                             const string & switchVarNameIn,
                             const int & prefIdx,RPGBuilder::Preference * const pref,
                             LPData & constraintData,
                             int columnIn=-1,int trigger=-1)
    {    
        if (pref->cost <= 0.0) {
            if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for constraint " << prefIdx << ": has zero cost\n";
            return noColRowData;
        }
        
        if (pref->d->conditionNums[0].empty() && pref->d->conditionNegativeNums[0].empty()) {
            if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for constraint " << prefIdx << ": has no numeric parts\n";
            return noColRowData;
        }
        
        PreferenceData * const d = pref->d;
        
        vector<int> & switchCols = constraintData.currentSwitchIDColumnScratch;
   
        assert(!switchCols.empty());
        
        SwitchVarInfo returnPair((columnIn == -1 ? model->giveMeAColumn("is-violated " + pref->name) : columnIn), prefIdx < RPGBuilder::getTaskPrefCount());
                   
        switchCols[0] = returnPair.hasBeenViolatedCol;
        
        for (int sc = 1; sc < constraintData.switchesNeeded; ++sc) {
            switchCols[sc] = model->giveMeAnIntermediateColumn(pref->name,sc);            
        }
        
        int addTriggerIfBasedOnVar = 0;
        
        {
            const int srCount = constraintData.switchRowEntries.size();
            for (int sr = 0; sr < srCount; ++sr) {
                const int ses = constraintData.switchRowEntries[sr].size();
                vector<pair<int,double> > newRow(ses);
                
                for (int e = 0; e < ses; ++e) {
                    newRow[0] = make_pair(switchCols[constraintData.switchRowEntries[sr][e].first], constraintData.switchRowEntries[sr][e].second);
                }
                
                ostringstream srn;
                srn << pref->name << "sr" << sr;
                
                double rhslb = constraintData.switchRowBounds[sr].first;
                if (trigger != -1) {
                    if (sr == 0 && constraintData.switchIsRoot) {
                        rhslb -= BIG;
                        newRow.push_back(make_pair(trigger, -BIG));
                    }
                }
                
                model->addTrigger(srn.str(), newRow, rhslb, constraintData.switchRowBounds[sr].second);
            }
        }
        
        
        const int varCount = RPGBuilder::getPNECount();    
        
        const NNF_Flat::Cell * const cells = d->flattened[0]->getCells();
        const int cellCount = d->flattened[0]->getCellCount();
        
        const bool appendindex = (cellCount > 1);
                
        for (int cidx = 0; cidx < cellCount; ++cidx) {

            if (!cells[cidx].isCell()) continue;

            string switchVarName;
            
            if (appendindex) {
                ostringstream nameIt;
                nameIt << switchVarNameIn;
                nameIt << "_" << cidx;
                
                switchVarName = nameIt.str();
            } else {
                switchVarName = switchVarNameIn;
            }

            vector<pair<int,double> > entries;                                                            
                
            const int toReturn = switchCols[constraintData.encodeCellUsingSwitchID[cidx]];                        
            
            if (cells[cidx].lit) {
                if (trigger != -1) {
                    entries.resize(2);
                    entries[0] = make_pair(toReturn, 1.0);
                    entries[1] = make_pair(trigger, -1.0);
                    returnPair.second.push_back(model->addSwitchB(switchVarName, entries, true, 0.0, false));
                    if (cells[cidx].polarity) {
                        model->literalAffectsRow(cells[cidx].lit->getStateID(), returnPair.second.back(), -1.0);
                    } else {
                        model->negativeLiteralAffectsRow(cells[cidx].lit->getStateID(), returnPair.second.back(), -1.0);
                    }
                } else {
                    entries.push_back(make_pair(toReturn, 1.0));
                    returnPair.second.push_back(model->addSwitchB(switchVarName, entries, true, 1.0, false));
                    if (cells[cidx].polarity) {                        
                        model->literalAffectsRow(cells[cidx].lit->getStateID(), returnPair.second.back(), 0.0);
                    } else {
                        model->negativeLiteralAffectsRow(cells[cidx].lit->getStateID(), returnPair.second.back(), 0.0);
                    }
                }
            } else {

                const int preIdx = cells[cidx].num;
                
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
                
                assert(currPre.op == VAL::E_GREATEQ);

                entries.push_back(make_pair(toReturn, (cells[cidx].polarity ? BIG : -BIG)));
                
                int vi = 1;
                
                list<pair<int,double> > printTmp;
                
                double rhc = currPre.RHSConstant;
                
                
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
                    rhc -= p.constant;
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

                if (trigger != -1) {
                    
                    if (constraintData.encodeCellUsingSwitchID[cidx] == addTriggerIfBasedOnVar) {
                        if (cells[cidx].polarity) {
                            rhc -= BIG;
                            entries.push_back(make_pair(trigger, -BIG));
                        } else {
                            rhc += BIG;
                            entries.push_back(make_pair(trigger, BIG));
                        }
                    }
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
                        cout << " >= " << rhc << "\n";
                    }
                     
                    
                    returnPair.second.push_back(model->addSwitchB(switchVarName, entries, cells[cidx].polarity, rhc, false));
                }
            }
        }
        
        return returnPair;
        
    };

    SwitchVarInfo extendLPForFactAtSomePoint(MinimalState * state,MILPRPG * model,
                                             const string & switchVarNameIn,
                                             const int & prefIdx,RPGBuilder::Preference * const pref,
                                             LPData & constraintData,                                                    
                                             bool lookAtGoal=true, bool spaceForHelper=false)
    {    
        static const double big = BIG;
        if (pref->cost <= 0.0) {
            if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for constraint " << prefIdx << ": has zero cost\n";
            return noColRowData;
        }
        
        const int partIdx = (lookAtGoal ? 0 : 1);
        
        if (pref->d->conditionNums[partIdx].empty() && pref->d->conditionNegativeNums[partIdx].empty()) {
            if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for constraint " << prefIdx << ": has no numeric parts\n";
            return noColRowData;
        }

        vector<int> & switchCols = constraintData.currentSwitchIDColumnScratch;
        
        SwitchVarInfo returnPair(model->giveMeAColumn("is-violated " + pref->name), prefIdx < RPGBuilder::getTaskPrefCount());
        
        switchCols[0] = returnPair.hasBeenViolatedCol;        
        
        for (int sc = 1; sc < constraintData.switchesNeeded; ++sc) {
            switchCols[sc] = model->giveMeAnIntermediateColumn(pref->name,sc);            
        }
                
        const int varCount = RPGBuilder::getPNECount();    

        {
            const int srCount = constraintData.switchRowEntries.size();
            for (int sr = 0; sr < srCount; ++sr) {
                const int ses = constraintData.switchRowEntries[sr].size();
                vector<pair<int,double> > newRow(ses);
                
                for (int e = 0; e < ses; ++e) {
                    newRow[0] = make_pair(switchCols[constraintData.switchRowEntries[sr][e].first], constraintData.switchRowEntries[sr][e].second);
                }
                                
                ostringstream srn;
                srn << pref->name;
                if (lookAtGoal) {
                    srn << "gsr" << sr;
                } else {
                    srn << "tsr" << sr;
                }
                
                const double rhslb = constraintData.switchRowBounds[sr].first;
                                                                
                model->addTrigger(srn.str(), newRow, rhslb, constraintData.switchRowBounds[sr].second);
            }
        }

        
        const NNF_Flat::Cell * const cells = pref->d->flattened[partIdx]->getCells();
        const int cellCount = pref->d->flattened[partIdx]->getCellCount();
        
        const bool appendindex = (cellCount > 1);
        
        for (int cidx = 0; cidx < cellCount; ++cidx) {
            
            if (!cells[cidx].isCell()) continue;
            
            vector<pair<int,double> > entries;                                                            
            
            const int toReturn = switchCols[constraintData.encodeCellUsingSwitchID[cidx]];                        
            
            string switchVarName;
            
            if (appendindex) {
                ostringstream nameIt;
                nameIt << switchVarNameIn;
                nameIt << "_" << cidx;
                
                switchVarName = nameIt.str();
            } else {
                switchVarName = switchVarNameIn;
            }
            
            
            if (cells[cidx].lit) {                
                entries.push_back(make_pair(toReturn, 1.0));
                returnPair.second.push_back(model->addSwitchB(switchVarName, entries, true, 1.0, false));
                if (cells[cidx].polarity) {                        
                    model->literalAffectsRow(cells[cidx].lit->getStateID(), returnPair.second.back(), 0.0);
                } else {
                    model->negativeLiteralAffectsRow(cells[cidx].lit->getStateID(), returnPair.second.back(), 0.0);
                }
                
            } else {
                
                const int preIdx = cells[cidx].num;
                
        
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
                
                assert(currPre.op == VAL::E_GREATEQ);

                if (cells[cidx].polarity) {                    
                    entries.push_back(make_pair(toReturn, big));
                } else {
                    entries.push_back(make_pair(toReturn, -big));
                }
                
                int vi = 1;
                
                double rhc = currPre.RHSConstant;
                
                
                list<pair<int,double> > printTmp;
                
                if (currPre.LHSVariable < varCount) {
                    int colidx;
                    if (cells[cidx].polarity) {
                        colidx = model->getUBColumnForVar(currPre.LHSVariable);
                    } else {
                        colidx = model->getLBColumnForVar(currPre.LHSVariable);
                    }
                    if (colidx >= 0) {
                        entries.push_back(make_pair(colidx, currPre.LHSConstant));        
                        if (PreferenceHandler::preferenceDebug) {
                            printTmp.push_back(make_pair(currPre.LHSVariable, currPre.LHSConstant));
                        }
                        ++vi;
                    }
                } else if (currPre.LHSVariable < (2 * varCount)) {
                    int colidx;
                    if (cells[cidx].polarity) {
                        colidx = model->getUBColumnForVar(currPre.LHSVariable - varCount);
                    } else {
                        colidx = model->getLBColumnForVar(currPre.LHSVariable - varCount);
                    }
                    if (colidx >= -0) {
                        entries.push_back(make_pair(colidx, -currPre.LHSConstant));        
                        if (PreferenceHandler::preferenceDebug) {
                            printTmp.push_back(make_pair(currPre.LHSVariable - varCount, -currPre.LHSConstant));
                        }                        
                        ++vi;
                    }
                } else {
                    const RPGBuilder::ArtificialVariable & p = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                    rhc -= p.constant;
                    for (int ps = 0; ps < p.size; ++ps) {
                        pair<int, double> addition;
                        if (cells[cidx].polarity) {
                            if (p.fluents[ps] < varCount) {
                                addition = make_pair(model->getUBColumnForVar(p.fluents[ps]), p.weights[ps]);
                            } else {
                                addition = make_pair(model->getLBColumnForVar(p.fluents[ps] - varCount), -p.weights[ps]);
                            }
                        } else {
                            if (p.fluents[ps] < varCount) {
                                addition = make_pair(model->getLBColumnForVar(p.fluents[ps]), p.weights[ps]);
                            } else {
                                addition = make_pair(model->getUBColumnForVar(p.fluents[ps] - varCount), -p.weights[ps]);
                            }
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
                            if (cells[cidx].polarity) {
                                cout << "  + (-BIG * helper)\n";
                            } else {
                                cout << "  + (BIG * helper)\n";
                            }
                        }
                        if (cells[cidx].polarity) {
                            cout << " >= " << rhc << " at some point\n";
                            
                        } else {
                            cout << " <= " << rhc << " at some point\n";
                            
                        }
                    }

                    if (spaceForHelper) {
                        if (cells[cidx].polarity) {                            
                            rhc -= big;
                        } else {
                            rhc += big;
                        }
                    }

                    
                    returnPair.second.push_back(model->addSwitchB(switchVarName, entries, cells[cidx].polarity, rhc, spaceForHelper));
                    
                    if (PreferenceHandler::preferenceDebug && spaceForHelper) {
                        cout << "Leaving space for trigger on row " << returnPair.second.back() << endl;
                    }
                }
            }
        }
        
        return returnPair;
        
    };

    void noteVariablesForFactAtSomePoint(vector<uint> & toUpdate, const int & prefIdx,RPGBuilder::Preference * const pref, bool inGoal=true)
    {    
        static const bool recordDebug = true;
        if (pref->cost <= 0.0) {
            if (PreferenceHandler::preferenceDebug || recordDebug) cout << "Not bothering to ask for bounds for constraint " << prefIdx << ": has zero cost\n";
            return;
        }
        
        PreferenceData * const d = pref->d;
        
        const int varCount = RPGBuilder::getPNECount();    
            
        const int partIdx = (inGoal ? 0 : 1);
        
        for (int neg = 0; neg < 2; ++neg) {
            
            list<int>::const_iterator numItr = (neg ? d->conditionNegativeNums[partIdx].begin() : d->conditionNums[partIdx].begin());
            const list<int>::const_iterator numEnd = (neg ? d->conditionNegativeNums[partIdx].end() : d->conditionNums[partIdx].end());
        
            const int ub = (neg ? 2 : 1);
            const int lb = (neg ? 1 : 2);
            
            for (; numItr != numEnd; ++numItr) {
                        
                const int preIdx = *numItr;
                assert(preIdx >= 0);
                
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[preIdx];
                
                vector<pair<int,double> > entries;
                                
                assert(currPre.op == VAL::E_GREATEQ);

                                
                if (currPre.LHSVariable < varCount) {                                
                    toUpdate[currPre.LHSVariable] |= ub;
                    if (recordDebug) {
                        cout << prefIdx << " wants an";
                        if (neg) cout << " inverted";
                        cout << " upper bound on " << *(RPGBuilder::getPNE(currPre.LHSVariable)) << endl;
                    }
                } else if (currPre.LHSVariable < (2 * varCount)) {
                    toUpdate[currPre.LHSVariable - varCount] |= lb;                    
                    if (recordDebug) {
                        cout << prefIdx << " wants a";
                        if (neg) cout << "n inverted";
                        cout << " lower bound on " << *(RPGBuilder::getPNE(currPre.LHSVariable - varCount)) << endl;
                    }
                } else {
                    const RPGBuilder::ArtificialVariable & p = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                    for (int ps = 0; ps < p.size; ++ps) {
                        pair<int, double> addition;
                        if (p.fluents[ps] < varCount) {
                            toUpdate[p.fluents[ps]] |= ub;
                            if (recordDebug) {
                                cout << prefIdx << " wants an";
                                if (neg) cout << " inverted";
                                cout << " upper bound on " << *(RPGBuilder::getPNE(p.fluents[ps])) << endl;
                            }
                        } else {
                            toUpdate[p.fluents[ps] - varCount] |= lb;
                            if (recordDebug) {
                                cout << prefIdx << " wants a";
                                if (neg) cout << "n inverted";
                                cout << " lower bound on " << *(RPGBuilder::getPNE(p.fluents[ps] - varCount)) << endl;
                            }
                        }
                    }
                }

            }
        }
        
    };

    
    void getSubsumedPresAndEffs(list<int> & presOut, list<int> & effsOut, const int & numCondID, const bool & polarity) {
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
                
                constVal /= currAV.weights[0];
                
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
        
        if (!polarity) {
            if (GE) {
                GE = false;
                constVal -= SMALL;
            } else {
                GE = true;
                constVal += SMALL;
            }
        }
                
        {
            const int loopLim = numPres.size();
           
            for (int p = 0; p < loopLim; ++p) {
                if (p == numCondID) continue;
                if (numPres[p].LHSVariable < PNECount) {
                    if (masterVar == numPres[p].LHSVariable) {
                        if (GE && numPres[p].RHSConstant >= constVal) {
                            presOut.push_back(p);
                        }
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
    
    void actionsWhichMeanWeHad(set<int> & actVariables, list<ConditionAndPolarity> & addingThisWouldTriggerList)    
    {
        list<ConditionAndPolarity>::iterator tItr = addingThisWouldTriggerList.begin();
        const list<ConditionAndPolarity>::iterator tEnd = addingThisWouldTriggerList.end();
        
        for (; tItr != tEnd; ++tItr) {
            ConditionAndPolarity & addingThisWouldTrigger = *tItr;
            if (!addingThisWouldTrigger.second) {
                
                
                const int litID = addingThisWouldTrigger.first;
                const bool polarity = addingThisWouldTrigger.polarity;
                
                if (polarity) {
                    const vector<list<pair<int, VAL::time_spec> > > & preToAction = RPGBuilder::getProcessedPreconditionsToActions();
                    
                    list<pair<int, VAL::time_spec> >::const_iterator wpItr = preToAction[litID].begin();
                    const list<pair<int, VAL::time_spec> >::const_iterator wpEnd = preToAction[litID].end();
                    
                    for (; wpItr != wpEnd; ++wpItr) {
                        assert(wpItr->second == VAL::E_AT_START);
                        actVariables.insert(wpItr->first);
                    }
                }
                
                {
                    const vector<list<pair<int, VAL::time_spec> > > & eta = (polarity ? RPGBuilder::getEffectsToActions() : RPGBuilder::getNegativeEffectsToActions());
                    
                    list<pair<int, VAL::time_spec> >::const_iterator wpItr = eta[litID].begin();
                    const list<pair<int, VAL::time_spec> >::const_iterator wpEnd = eta[litID].end();
                    
                    for (; wpItr != wpEnd; ++wpItr) {
                        assert(wpItr->second == VAL::E_AT_START);
                        actVariables.insert(wpItr->first);
                    }
                }
                if (PreferenceHandler::preferenceDebug) {
                    cout << "The following actions mean any trajectory contains ";
                    if (!polarity) cout << "¬";
                    cout << *(RPGBuilder::getLiteral(litID)) << ":\n";
                }
                            
            } else {
                                    
                const int numCondID = addingThisWouldTrigger.first;
                const bool polarity = addingThisWouldTrigger.polarity;
                
                list<int> plusRelated;
                list<int> effRelated;
                
                getSubsumedPresAndEffs(plusRelated, effRelated, numCondID, polarity);
                
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
                if (PreferenceHandler::preferenceDebug) {
                    cout << "The following actions mean any trajectory contains ";
                    if (!polarity) cout << "¬";
                    cout << (RPGBuilder::getNumericPrecs()[numCondID]) << ":\n";
                }
            }
            
            if (PreferenceHandler::preferenceDebug) {
                set<int>::const_iterator avpItr = actVariables.begin();
                const set<int>::const_iterator avpEnd= actVariables.end();
                
                for (; avpItr != avpEnd; ++avpItr) {
                    cout << "\t" << *(RPGBuilder::getInstantiatedOp(*avpItr)) << "\n";
                }
            }
        }
    }
    
    void buildTriggersOnEndValues(int col, RPGBuilder::Preference * const pref, LPData & constraintData,
                                  MILPRPG * const model, bool useGoal=false)
    {
        
        
        PreferenceData * const d = pref->d;
        
        vector<int> & switchCols = constraintData.currentSwitchIDColumnScratch;
        
        switchCols[0] = col;
        
        for (int sc = 1; sc < constraintData.switchesNeeded; ++sc) {
            switchCols[sc] = model->giveMeAnIntermediateColumn(pref->name,sc);            
        }
                
        {
            const int srCount = constraintData.switchRowEntries.size();
            for (int sr = 0; sr < srCount; ++sr) {
                const int ses = constraintData.switchRowEntries[sr].size();
                vector<pair<int,double> > newRow(ses);
                
                for (int e = 0; e < ses; ++e) {
                    newRow[0] = make_pair(switchCols[constraintData.switchRowEntries[sr][e].first], constraintData.switchRowEntries[sr][e].second);
                }
                                                
                ostringstream srn;
                srn << pref->name << "sr" << sr;
                
                double rhslb = constraintData.switchRowBounds[sr].first;
                model->addTrigger(srn.str(), newRow, rhslb, constraintData.switchRowBounds[sr].second);
            }
        }
                                        
        const int partIdx = (useGoal ? 0 : 1);
        
        const int varCount = RPGBuilder::getPNECount();    

        const NNF_Flat::Cell * const cells = d->flattened[partIdx]->getCells();
        const int cellCount = d->flattened[partIdx]->getCellCount();

        const bool appendindex = (cellCount > 1);

        for (int cidx = 0; cidx < cellCount; ++cidx) {

            if (!cells[cidx].isCell()) continue;
                                                            
        
            string switchVarName = "etrig-" + pref->name;
            
            if (appendindex) {
                ostringstream nameIt;
                nameIt << switchVarName;
                nameIt << "_" << cidx;
                
                switchVarName = nameIt.str();
            }
            
            vector<pair<int,double> > entries;

            const int toReturn = switchCols[constraintData.encodeCellUsingSwitchID[cidx]];                        
            
            
            if (cells[cidx].lit) {
                
                entries.push_back(make_pair(toReturn, 1.0));
                const int rowIdx = model->addSwitchB(switchVarName, entries, true, 1.0, false);
                                
                if (cells[cidx].polarity) {                        
                    model->literalAffectsRow(cells[cidx].lit->getStateID(), rowIdx, 0.0);
                } else {
                    model->negativeLiteralAffectsRow(cells[cidx].lit->getStateID(), rowIdx, 0.0);
                }
            } else {

                const RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[cells[cidx].num];
                
                assert(currPre.op == VAL::E_GREATEQ);

                entries.push_back(make_pair(toReturn, (cells[cidx].polarity ? BIG : -BIG)));
                
                int vi = 1;
                
                list<pair<int,double> > printTmp;
                
                double rhc = currPre.RHSConstant;
                
                
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
                    rhc -= p.constant;
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
                
                {
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
                        cout << " >= " << rhc << "\n";
                    }
                     
                    
                    model->addSwitchB(switchVarName, entries, cells[cidx].polarity, rhc, false);
                }

            }
        }
    }
    
    
    
    
};

void PreferenceFSA::pushNumericDependencies(map<int, list<pair<int, int> > > & factsNeededForCol, const int& partIdx)
{

    NNF_Flat * const flat = pref->d->flattened[partIdx];
    if (flat) return;
    
    const int cellCount = flat->getCellCount();
    const NNF_Flat::Cell * const cells = flat->getCells();
    
    for (int cidx = 0; cidx < cellCount; ++cidx) {
        if (cells[cidx].isCell()) continue;
        if (cells[cidx].lit) continue;
        if (!cells[cidx].polarity) continue;
        
        const int switchID = boilerplate[partIdx].encodeCellUsingSwitchID[cidx];
        
        factsNeededForCol[cells[cidx].num].push_back(make_pair(prefIdx, boilerplate[partIdx].currentSwitchIDColumnScratch[switchID]));
            
    }
    
}


/// always constraints                        

AlwaysFSA::AlwaysFSA(const int & idx, RPGBuilder::Preference * const p, AutomatonPosition & initPosn)
: PreferenceFSA(idx, p)
{
    assert(pref->cons == VAL::E_ALWAYS);    
    initPosn = satisfied;
}

void AlwaysFSA::update(MinimalState * state)
{
    const bool alwaysUpdateDebug = false;
    if (state->preferenceStatus[prefIdx] == unreachable) {
        return;
    }
    
    if (!goalOrTriggerIsSatisfied(state, 0)) {
        state->preferenceStatus[prefIdx] = unreachable;
        if (alwaysUpdateDebug || PreferenceHandler::preferenceDebug) cout << "Preference " << pref->name << " is now violated\n";        
        return;
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

                        
SwitchVarInfo AlwaysFSA::extendLP(map<int,list<pair<int, int> > > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
            
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for always constraint " << prefIdx << ": is already violated\n";
        return noColRowData;
    }
    
    const SwitchVarInfo toReturn(PrefCommon::extendLPForFactAtEnd(state, model, switchVarName, prefIdx, pref, boilerplate[0]));
    
    if (toReturn.hasBeenViolatedCol == -1) return toReturn;
   
    pushNumericDependencies(factNeededForCol, 0);
    
    return toReturn;
    
};

void AlwaysFSA::getUnsatisfiedConditionCounts(MinimalState &, vector<NNF_Flat*> &)
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

    NNF_Flat * const f = pref->d->flattened[0];
    if (!f) return;
    
    const bool * const cellIsAnAnd = f->cellIsAnAnd();
    
    if (!cellIsAnAnd[0]) return;
            
    const NNF_Flat::Cell * const cells = f->getCells();
    const int * parentIDs = f->getParentIDs();
    const int cellCount = f->getCellCount();
    
    int fID;
    
    for (int c = 0; c < cellCount; ++c) {
        if (!(cells[c].isCell())) continue;
        if (parentIDs[c] != 0) continue;
        
        if (cells[c].lit) {
            if (cells[c].polarity) {
                fID = cells[c].lit->getStateID();
                assert(theState.first.find(fID) != theState.first.end());
                factCost[fID].insert(prefIdx);
            }
        } else {
            if (cells[c].polarity) {
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[cells[c].num];
                
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
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (PreferenceHandler::preferenceDebug) {
            cout << prefIdx << ": update: at-end pref " << pref->name << " is unreachable, cost " << pref->cost << "\n";
            assert(!goalOrTriggerIsSatisfied(state, 0));
        }
        return;
    }
            
    state->preferenceStatus[prefIdx] = unsatisfied;
    
    if (goalOrTriggerIsSatisfied(state, 0)) {
        if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: at-end pref " << pref->name << " is currently satisfied, cost " << pref->cost << "\n";
        state->preferenceStatus[prefIdx] = satisfied;            
    } else {
        if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: at-end pref " << pref->name << " is currently unsatisfied, cost " << pref->cost << "\n";        
    }

    
      
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


SwitchVarInfo AtEndFSA::extendLP(map<int,list<pair<int, int> > > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
    
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (PreferenceHandler::preferenceDebug) cout << "Not extending LP for at-end constraint " << prefIdx << ": can never be reached\n";
        return noColRowData;
        
    }
    
    const SwitchVarInfo toReturn = PrefCommon::extendLPForFactAtEnd(state, model, switchVarName, prefIdx, pref, boilerplate[0]);
    
    if (toReturn.hasBeenViolatedCol == -1) return toReturn;
    
    pushNumericDependencies(factNeededForCol,0);
    
       
    
    return toReturn;
    
};

void AtEndFSA::getUnsatisfiedConditionCounts(MinimalState & state, vector<NNF_Flat*> & toFill)
{
    if (state.preferenceStatus[prefIdx] == unreachable) {
        // If it was unreachable before, it's still unreachable now, so don't try to get it
        return;
    }    
    if (!pref->d->flattened[0]) return;
    toFill[0] = pref->d->flattened[0];
    toFill[0]->reset();
}

void AtEndFSA::getDesiredGoals(list<list<Literal*> > & desired, list<list<int> > * desiredNumeric,
                               MinimalState & startState,
                               const vector<vector<NNF_Flat*> > & unsatisfiedPreferenceConditions,
                               vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer)
{
    if (startState.preferenceStatus[prefIdx] == satisfied || startState.preferenceStatus[prefIdx] == unreachable) return;
    if (!unsatisfiedPreferenceConditions[prefIdx][0]) return;
    if (!unsatisfiedPreferenceConditions[prefIdx][0]->isSatisfied()) return;
    
    satisfyAtLowCost(unsatisfiedPreferenceConditions[prefIdx][0], literalCosts, numericAchievedInLayer, desired, desiredNumeric);
    
}


void AtEndFSA::updateCosts(MinimalState & initialState,
                            const bool & wasTheTrigger,
                            vector<AutomatonPosition> & optimisticAutomataPositions,
                            map<int, AddingConstraints > & prefCostOfAddingFact,
                            map<int, map<double, AddingConstraints > > & numChangeCost,
                            list<pair<int,int> > & preferencesThatNowHaveNoAddCosts)
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
                assert(!goalOrTriggerIsSatisfied(state, 0));
            }
        }
        return;
    }
    
    if (goalOrTriggerIsSatisfied(state,0)) {
        if (sometimeUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime pref " << pref->name << " is now satisfied, cost " << pref->cost << "\n";
        state->preferenceStatus[prefIdx] = satisfied;        
    } else {
        if (sometimeUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime pref " << pref->name << " is currently unsatisfied, cost " << pref->cost << "\n";        
    }

      
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

SwitchVarInfo SometimeFSA::extendLP(map<int,list<pair<int, int> > > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
    static const bool sometimeExtendDebug = false;
    
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Not extending LP for sometime constraint " << prefIdx << ": can never be reached\n";
        return noColRowData;
        
    }
    if (state->preferenceStatus[prefIdx] == satisfied) {
        if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Not extending LP for sometime constraint " << prefIdx << ": already reached\n";
        return noColRowData;
        
    }
    
    if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Extending LP for sometime constraint " << prefIdx << "\n";
    
    const SwitchVarInfo toReturn = PrefCommon::extendLPForFactAtSomePoint(state, model, switchVarName, prefIdx, pref, boilerplate[0]);
    
    if (toReturn.hasBeenViolatedCol == -1) {
        if (PreferenceHandler::preferenceDebug || sometimeExtendDebug) cout << "Not really extended LP for sometime constraint " << prefIdx << "\n";
        return toReturn;
    }

    pushNumericDependencies(factNeededForCol, 0);
       
    return toReturn;
    
};

void SometimeFSA::getUnsatisfiedConditionCounts(MinimalState & state, vector<NNF_Flat*> & toFill)
{
    if (state.preferenceStatus[prefIdx] == satisfied || state.preferenceStatus[prefIdx] == unreachable) {
        // Once we've had a sometime preference, or shown it to be unreachable, we don't need to try to get it again
        return;
    }
    if (!pref->d->flattened[0]) return;
    toFill[0] = pref->d->flattened[0];
    toFill[0]->reset();
}

void SometimeFSA::getDesiredGoals(list<list<Literal*> > & desired, list<list<int> > * desiredNumeric,
                                  MinimalState & startState,
                                  const vector<vector<NNF_Flat*> > & unsatisfiedPreferenceConditions,
                                  vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer)
{
    if (startState.preferenceStatus[prefIdx] == satisfied || startState.preferenceStatus[prefIdx] == unreachable) return;
    if (!unsatisfiedPreferenceConditions[prefIdx][0]) return;
    if (!unsatisfiedPreferenceConditions[prefIdx][0]->isSatisfied()) return;
            
    satisfyAtLowCost(unsatisfiedPreferenceConditions[prefIdx][0], literalCosts, numericAchievedInLayer, desired, desiredNumeric);
    
}

void SometimeFSA::updateCosts(MinimalState & initialState,
                               const bool & wasTheTrigger,
                               vector<AutomatonPosition> & optimisticAutomataPositions,
                               map<int, AddingConstraints > & prefCostOfAddingFact,
                               map<int, map<double, AddingConstraints > > & numChangeCost,
                               list<pair<int,int> > & preferencesThatNowHaveNoAddCosts)
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
 
    PrefCommon::workOutIfAddingOneThingCanTrigger(addingOneThingCanTrigger, addingThisWouldTrigger,
                                                  pref->d, 0);

    if (addingOneThingCanTrigger) {
        PrefCommon::actionsWhichMeanWeHad(actionsImplyingTrigger, addingThisWouldTrigger);
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
    
    const bool allPresent = goalOrTriggerIsSatisfied(state, 0);
        
        
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
        if (!(goalType & flag)) return 0.0;
        if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": at-most-once pref " << pref->name << " cannot be satisfied, cost " << pref->cost << "\n";
        return pref->cost;
    }
            
    if (PreferenceHandler::preferenceDebug) cout << prefIdx << ": at-most-once pref " << pref->name << " is cost free: " << positionName[state->preferenceStatus[prefIdx]] << endl;
    return 0.0;
}

double AtMostOnceFSA::reachableCost(MinimalState * state, const int & flag)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (!(goalType & flag)) return 0.0;
        return pref->cost;
    }
                
    return 0.0;
}

double AtMostOnceFSA::GCost(MinimalState * state, const int & flag)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (!(goalType & flag)) return 0.0;
        return pref->cost;
    }
                    
    return 0.0;
}

void AtMostOnceFSA::noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate)
{
    
}

SwitchVarInfo AtMostOnceFSA::extendLP(map<int,list<pair<int, int> > > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
    static const bool atMostOnceExtendDebug = false;
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (atMostOnceExtendDebug || PreferenceHandler::preferenceDebug) cout << "Not extending LP for at-most-once constraint " << prefIdx << ": can never be reached\n";
        return noColRowData;
        
    }
    
    if (state->preferenceStatus[prefIdx] != seenoncealready) {
        if (atMostOnceExtendDebug || PreferenceHandler::preferenceDebug) cout << "Not extending LP for at-most-once constraint " << prefIdx << ": " << positionName[state->preferenceStatus[prefIdx]] << endl;
        return noColRowData;
        
    }
                
    if (atMostOnceExtendDebug || PreferenceHandler::preferenceDebug) {
        cout << "Extending LP for at-most-once constraint " << prefIdx << ": potential for it to be broken\n";
    }
                  
    const int toReturn = model->giveMeAColumn("is-violated " + pref->name);
    
    vector<pair<int,double> > entries(1, make_pair(toReturn, BIG));
    
    list<int> newRow;
    newRow.push_back(model->addSwitchB(switchVarName, entries, true, 0.0, true)); // Can only be triggered by actions known to cause it to be true
    
    return SwitchVarInfo(toReturn,newRow);
                            
                                
};

void AtMostOnceFSA::addHelperTriggers(const vector<SwitchVarInfo> & switchVarForPreference, MinimalState* state, MILPRPG* model)
{
    
    if (state->preferenceStatus[prefIdx] == unreachable) return;
    if (state->preferenceStatus[prefIdx] != seenoncealready) return;
        
    if (!triggerPartCount) return;
                
    const int startVar = model->giveMeATriggerColumn(prefIdx, false).first;
    
    PrefCommon::buildTriggersOnEndValues(startVar, pref, boilerplate[0], model, true);    
    
}



void AtMostOnceFSA::anotherPreferenceWants(const map<int,list<pair<int, int> > > & factNeededForCol,MinimalState * state, MILPRPG * model)
{    
    if (state->preferenceStatus[prefIdx] == unreachable) return;
    if (state->preferenceStatus[prefIdx] != seenoncealready) return;
    
    if (!addingOneThingCanTrigger) return;
    
    list<ConditionAndPolarity>::iterator tItr = addingThisWouldTrigger.begin();
    const list<ConditionAndPolarity>::iterator tEnd = addingThisWouldTrigger.end();
    
    for (; tItr != tEnd; ++tItr) {
        if (!tItr->second && tItr->polarity) {    
            map<int,list<pair<int, int> > >::const_iterator opItr = factNeededForCol.find(tItr->first);
                        
            if (opItr == factNeededForCol.end()) return;
                            
            model->preferenceTrippedBy(prefIdx, opItr->second);
        }
    }
                
};

void AtMostOnceFSA::importTriggerGroups(map<int, set<int> > & d, int & extraVars)
{
    if (!actionsImplyingTrigger.empty()) {
        d[-1 - prefIdx] = actionsImplyingTrigger;
    }
        
    triggerPartCount = extraVariablesFor(0);
    
    extraVars += triggerPartCount;
    extraVars += 4;
                
}

void AtMostOnceFSA::getUnsatisfiedConditionCounts(MinimalState &, vector<NNF_Flat*> &)
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
    
    
    list<ConditionAndPolarity>::iterator tItr = addingThisWouldTrigger.begin();
    const list<ConditionAndPolarity>::iterator tEnd = addingThisWouldTrigger.end();
    
    for (; tItr != tEnd; ++tItr) {
        if (!tItr->polarity) {
            /// TODO - Implement negative triggers
            continue;
        }
                        
        if (tItr->second) {
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[tItr->first];
            
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
            factCost[tItr->first].addingWillViolate.insert(prefIdx);
        }
    }
                                                                                                                        
}


/// sometime-after constraints

SometimeAfterFSA::SometimeAfterFSA(const int & idx, RPGBuilder::Preference * const p, AutomatonPosition & initPosn)
: PreferenceFSA(idx, p), triggerPartCount(0)
{
    assert(pref->cons == VAL::E_SOMETIMEAFTER);
    initPosn = satisfied;
    
    PrefCommon::workOutIfAddingOneThingCanTrigger(addingOneThingCanTrigger, addingThisWouldTrigger, pref->d, 1);

    if (addingOneThingCanTrigger) {
        PrefCommon::actionsWhichMeanWeHad(actionsImplyingTrigger, addingThisWouldTrigger);
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
        const bool allPresent = goalOrTriggerIsSatisfied(state, 1);
        
        if (allPresent) {
            if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) {
                cout << prefIdx << ": update: sometime-after pref " << pref->name << " has been triggered\n";
            }
            state->preferenceStatus[prefIdx] = triggered;
        } else {
            if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) {
                cout << prefIdx << ": update: sometime-after pref " << pref->name << " won't trigger\n";
            }
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

    const bool allPresent = goalOrTriggerIsSatisfied(state, 0);
    
    if (allPresent) {
        if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime-after pref " << pref->name << " is now satisfied, cost " << pref->cost << "\n";       
        state->preferenceStatus[prefIdx] = satisfied;
    } else {
        if (sometimeAfterUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime-after pref " << pref->name << " remains unsatisfied, cost " << pref->cost << "\n";       
    }
      
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

SwitchVarInfo SometimeAfterFSA::extendLP(map<int,list<pair<int, int> > > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
    static const bool sometimeAfterExtendDebug = false;
    
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (PreferenceHandler::preferenceDebug || sometimeAfterExtendDebug) cout << "Not extending LP for sometime-after constraint " << prefIdx << ": can never be reached\n";
        return noColRowData;
        
    }
    if (state->preferenceStatus[prefIdx] == triggered) {
        if (PreferenceHandler::preferenceDebug || sometimeAfterExtendDebug) cout << "Extending LP for triggered sometime-after constraint " << prefIdx << endl;
        
        //const bool debugWas = PreferenceHandler::preferenceDebug;
        if (sometimeAfterExtendDebug) PreferenceHandler::preferenceDebug = true;
        const SwitchVarInfo toReturn = PrefCommon::extendLPForFactAtSomePoint(state, model, switchVarName, prefIdx, pref, boilerplate[0]);
        //PreferenceHandler::preferenceDebug = debugWas;

        pushNumericDependencies(factNeededForCol,0);
        
           
        return toReturn;
    }
    
    assert(state->preferenceStatus[prefIdx] == satisfied);
    
    const bool oldDebug = PreferenceHandler::preferenceDebug;
    PreferenceHandler::preferenceDebug = sometimeAfterExtendDebug;
    
    
    if (PreferenceHandler::preferenceDebug || sometimeAfterExtendDebug) cout << "Extending LP for sometime-after constraint " << prefIdx << endl;
    
    const SwitchVarInfo toReturn = PrefCommon::extendLPForFactAtSomePoint(state, model, switchVarName, prefIdx, pref, boilerplate[0], true, true);
    PreferenceHandler::preferenceDebug = oldDebug;
    return toReturn;
};

void SometimeAfterFSA::addHelperTriggers(const vector<SwitchVarInfo> & switchVarForPreference, MinimalState* state, MILPRPG* model)
{
    
    if (!triggerPartCount) return;
    if (state->preferenceStatus[prefIdx] == unreachable) return;
        
    //static const int varCount = RPGBuilder::getPNECount();
    
    const pair<int,int> startVar = model->giveMeATriggerColumn(prefIdx, true);
    
    PrefCommon::buildTriggersOnEndValues(startVar.first, pref, boilerplate[1], model);
    
    PrefCommon::extendLPForFactAtEnd(state,model,"fae" + pref->name ,prefIdx,pref,boilerplate[0],switchVarForPreference[prefIdx].hasBeenViolatedCol,startVar.second);
                                       
        
}


void SometimeAfterFSA::anotherPreferenceWants(const map<int,list<pair<int, int> > > & factNeededForCol,MinimalState * state, MILPRPG * model)
{    
    if (state->preferenceStatus[prefIdx] != satisfied) return;

    if (!pref->d->conditionLiterals[1].empty()
        || !pref->d->conditionNegativeLiterals[1].empty()
        || !pref->d->conditionNegativeNums[1].empty() ) return;        
    
    if (pref->d->conditionNums[1].size() != 1) return;
                
    const int thePre = pref->d->conditionNums[1].front();
    
    map<int,list<pair<int, int> > >::const_iterator opItr = factNeededForCol.find(thePre);
    
    if (opItr == factNeededForCol.end()) return;
    
    model->preferenceTrippedBy(prefIdx, opItr->second);
            
};

void SometimeAfterFSA::importTriggerGroups(map<int, set<int> > & d, int & extraVars)
{
    if (!actionsImplyingTrigger.empty()) {
        d[-1 - prefIdx] = actionsImplyingTrigger;
    }

    extraVars += 2*extraVariablesFor(0);

    triggerPartCount = extraVariablesFor(1);
    
    if (triggerPartCount) {
        extraVars += 2*triggerPartCount;
        extraVars += 4;
    }
    
}

void SometimeAfterFSA::getUnsatisfiedConditionCounts(MinimalState & state, vector<NNF_Flat*> & toFill)
{
    if (state.preferenceStatus[prefIdx] == unreachable) {
        // If we can never satisfy the preference, don't bother trying
        return;
    }
    if (pref->d->flattened[0]) {
        toFill[0] = pref->d->flattened[0];
        toFill[0]->reset();
    }
    
    if (pref->d->flattened[1]) {
        toFill[1] = pref->d->flattened[1];        
        toFill[1]->reset();
    }
}

void SometimeAfterFSA::getCostsOfAdding(MinimalState & theState, map<int, AddingConstraints > & factCost, map<int, map<double, AddingConstraints > > & numChangeCost)
{
    if (theState.preferenceStatus[prefIdx] == unreachable || theState.preferenceStatus[prefIdx] == triggered) return;
    if (!addingOneThingCanTrigger) return;
    
    const double pCost = pref->cost;
    
    if (pCost == 0.0) return;
        
    const bool allPresent = goalOrTriggerIsSatisfied(&theState, 0);
    
    if (allPresent) {
        /*if (addingThisWouldTrigger.second) {
        RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[addingThisWouldTrigger.first];
        cout << "No fear from " << pref->name << " of satisfying " << currPre << " - is already satisfied\n";
    }*/
        return;
    }
    
    static const int varCount = RPGBuilder::getPNECount();
    
    list<ConditionAndPolarity>::iterator tItr = addingThisWouldTrigger.begin();
    const list<ConditionAndPolarity>::iterator tEnd = addingThisWouldTrigger.end();
    
    for (; tItr != tEnd; ++tItr) {
        if (!tItr->polarity) {
            /// TODO - Implement negative triggers
            continue;
        }
        
        
        
        if (tItr->second) {
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[tItr->first];
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
            factCost[tItr->first].addingWillViolate.insert(prefIdx);
        }
    }
}

void SometimeAfterFSA::updateCosts(MinimalState & initialState,
                                   const bool & wasTheTrigger,
                                   vector<AutomatonPosition> & optimisticAutomataPositions,
                                   map<int, AddingConstraints > & prefCostOfAddingFact,
                                   map<int, map<double, AddingConstraints > > & numChangeCost,
                                   list<pair<int,int> > & preferencesThatNowHaveNoAddCosts)
{
    
    if (wasTheTrigger) return;
    if (optimisticAutomataPositions[prefIdx] == unreachable) return;
    
    if (optimisticAutomataPositions[prefIdx] == triggered) {
        optimisticAutomataPositions[prefIdx] = satisfied;
        return;
    }
    
    if (!addingOneThingCanTrigger) return;
    
    static const int varCount = RPGBuilder::getPNECount();
    
    list<ConditionAndPolarity>::iterator tItr = addingThisWouldTrigger.begin();
    const list<ConditionAndPolarity>::iterator tEnd = addingThisWouldTrigger.end();
    
    for (; tItr != tEnd; ++tItr) {
    
        if (!tItr->polarity) {
            /// TODO - Implement negative triggers
            return;
        }
        if (tItr->first == -1) return;
        
        //cout << "Now no longer have to fear seeing the trigger of " << pref->name << " - the goal is reachable\n";
        
        
        if (tItr->second) {
            
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[tItr->first];
            
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
            map<int,AddingConstraints >::iterator fcItr = prefCostOfAddingFact.find(tItr->first);
            if (fcItr != prefCostOfAddingFact.end()) {
            
                fcItr->second.addingWillViolate.erase(prefIdx);
                fcItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));             
            }
        }
        
    }

    preferencesThatNowHaveNoAddCosts.push_back(make_pair(prefIdx,-1));
        
}


void SometimeAfterFSA::getDesiredGoals(list<list<Literal*> > & desired, list<list<int> > * desiredNumeric,
                                       MinimalState & startState,
                                       const vector<vector<NNF_Flat*> > & unsatisfiedPreferenceConditions,
                                       vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer)
{
    if (startState.preferenceStatus[prefIdx] != triggered) return;
    if (!unsatisfiedPreferenceConditions[prefIdx][0]) return;
    if (!unsatisfiedPreferenceConditions[prefIdx][0]->isSatisfied()) return;
                    
    satisfyAtLowCost(unsatisfiedPreferenceConditions[prefIdx][0], literalCosts, numericAchievedInLayer, desired, desiredNumeric);
    
}

/// Sometime-before constraints

SometimeBeforeFSA::SometimeBeforeFSA(const int & idx, RPGBuilder::Preference * const p, AutomatonPosition & initPosn)
: PreferenceFSA(idx, p), triggerPartCount(0)
{
    assert(pref->cons == VAL::E_SOMETIMEBEFORE);
    initPosn = satisfied;
    
    PrefCommon::workOutIfAddingOneThingCanTrigger(addingOneThingCanTrigger, addingThisWouldTrigger, pref->d, 1);
    
    if (addingOneThingCanTrigger) {
        PrefCommon::actionsWhichMeanWeHad(actionsImplyingTrigger, addingThisWouldTrigger);
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

        const bool allPresent = goalOrTriggerIsSatisfied(state, 1);
                                
        if (allPresent) {            
            if (sometimeBeforeUpdateDebug || PreferenceHandler::preferenceDebug) {
                cout << prefIdx << ": update: sometime-before pref " << pref->name << " is now broken forever, cost " << pref->cost << "\n";
            }                        
            state->preferenceStatus[prefIdx] = unreachable;
            return;
        }

        
    }
    
    const bool allPresent = goalOrTriggerIsSatisfied(state, 0);
    
    if (allPresent) {
        if (sometimeBeforeUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime-before pref " << pref->name << " has triggered, eternally satisfied\n";
        state->preferenceStatus[prefIdx] = eternallysatisfied;
    } else {
        if (sometimeBeforeUpdateDebug || PreferenceHandler::preferenceDebug) cout << prefIdx << ": update: sometime-before pref " << pref->name << " still just satisfied\n";
    }
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

SwitchVarInfo SometimeBeforeFSA::extendLP(map<int,list<pair<int, int> > > & factNeededForCol,MinimalState * state,MILPRPG * model)
{    
    static const bool sometimeBeforeExtendDebug = false;
    assert(state->preferenceStatus.size() > prefIdx);
    
    if (state->preferenceStatus[prefIdx] == unreachable) {
        if (sometimeBeforeExtendDebug || PreferenceHandler::preferenceDebug) cout << "Not extending LP for sometime-before constraint " << prefIdx << ": can never be reached\n";
        return noColRowData;
        
    }
    if (state->preferenceStatus[prefIdx] == eternallysatisfied) {
        if (sometimeBeforeExtendDebug || PreferenceHandler::preferenceDebug) cout << "Not extending LP for sometime-before constraint " << prefIdx << ": already reached\n";
        return noColRowData;
        
    }
    assert(state->preferenceStatus[prefIdx] == satisfied);
            
    if (sometimeBeforeExtendDebug || PreferenceHandler::preferenceDebug) {
        cout << "Extending LP for sometime-before constraint " << prefIdx << ": potential for it to be broken\n";
    }
    
    return PrefCommon::extendLPForFactAtSomePoint(state, model, switchVarName, prefIdx, pref, boilerplate[1], false, true);
};

void SometimeBeforeFSA::addHelperTriggers(const vector<SwitchVarInfo> & switchVarForPreference, MinimalState* state, MILPRPG* model)
{

    if (state->preferenceStatus[prefIdx] != satisfied) return;
    
    if (!triggerPartCount) return;
    
    //static const int varCount = RPGBuilder::getPNECount();
    
    const int startVar = model->giveMeATriggerColumn(prefIdx, false).first;
    
    PrefCommon::buildTriggersOnEndValues(startVar, pref, boilerplate[1], model);
    
        
}


void SometimeBeforeFSA::anotherPreferenceWants(const map<int,list<pair<int, int> > > & factNeededForCol,MinimalState * state, MILPRPG * model)
{    
    if (state->preferenceStatus[prefIdx] != satisfied) return;

    if (!addingOneThingCanTrigger) return;
    
    list<ConditionAndPolarity>::iterator tItr = addingThisWouldTrigger.begin();
    const list<ConditionAndPolarity>::iterator tEnd = addingThisWouldTrigger.end();
    
    for (; tItr != tEnd; ++tItr) {
        
        if (!tItr->polarity) {
            /// TODO - Implement negative triggers
            continue;
        }

        if (tItr->second) continue;
        
        map<int,list<pair<int, int> > >::const_iterator opItr = factNeededForCol.find(tItr->first);
        
        if (opItr == factNeededForCol.end()) return;
        
        model->preferenceTrippedBy(prefIdx, opItr->second);
    }
};

void SometimeBeforeFSA::importTriggerGroups(map<int, set<int> > & d, int & extraVars)
{
    if (!actionsImplyingTrigger.empty()) {
        d[-1 - prefIdx] = actionsImplyingTrigger;
    }
    
    extraVars += extraVariablesFor(0);
    
    triggerPartCount = extraVariablesFor(1);
    
    extraVars += triggerPartCount;
    extraVars += 4;
    
}

void SometimeBeforeFSA::getUnsatisfiedConditionCounts(MinimalState & state, vector<NNF_Flat*> & toFill)
{
    if (state.preferenceStatus[prefIdx] == unreachable || state.preferenceStatus[prefIdx] == eternallysatisfied) {
        // If we can never satisfy/unsatisfy the preference, don't bother trying
        return;
    }
    
    if (pref->d->flattened[0]) {        
        toFill[0] = pref->d->flattened[0];
        toFill[0]->reset();
    }
    
    if (pref->d->flattened[1]) {        
        toFill[1] = pref->d->flattened[1];        
        toFill[1]->reset();
    }
}

void SometimeBeforeFSA::getCostsOfAdding(MinimalState & theState, map<int, AddingConstraints > & factCost, map<int, map<double, AddingConstraints > > & numChangeCost)
{
    if (theState.preferenceStatus[prefIdx] != satisfied) return;
    if (!addingOneThingCanTrigger) return;
                
    static const int varCount = RPGBuilder::getPNECount();
        
    const double pCost = pref->cost;
    
    if (pCost == 0.0) return;
        
    list<ConditionAndPolarity>::iterator tItr = addingThisWouldTrigger.begin();
    const list<ConditionAndPolarity>::iterator tEnd = addingThisWouldTrigger.end();
    
    for (; tItr != tEnd; ++tItr) {
    
        if (!tItr->polarity) {
            /// TODO - Implement negative triggers
            continue;
        }
            
                                    
        if (tItr->second) {
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[tItr->first];
            
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
            //cout << "Adding " << *(RPGBuilder::getLiteral(addingThisWouldTrigger.first)) << " would kill " << pref->name << endl;
            factCost[tItr->first].addingWillViolate.insert(prefIdx);
        }
    }                                                       
}

void SometimeBeforeFSA::updateCosts(MinimalState & initialState,
                                    const bool & wasTheTrigger,
                                    vector<AutomatonPosition> & optimisticAutomataPositions,
                                    map<int, AddingConstraints > & prefCostOfAddingFact,
                                    map<int, map<double, AddingConstraints > > & numChangeCost,
                                    list<pair<int,int> > & preferencesThatNowHaveNoAddCosts)
{
                                       
    if (wasTheTrigger) return;
    if (optimisticAutomataPositions[prefIdx] == unreachable || optimisticAutomataPositions[prefIdx] == eternallysatisfied) return;
                                               
    optimisticAutomataPositions[prefIdx] = eternallysatisfied;
    
                                                       
    if (!addingOneThingCanTrigger) return;
                    
    
    static const int varCount = RPGBuilder::getPNECount();
    
    list<ConditionAndPolarity>::iterator tItr = addingThisWouldTrigger.begin();
    const list<ConditionAndPolarity>::iterator tEnd = addingThisWouldTrigger.end();
    
    for (; tItr != tEnd; ++tItr) {    
        if (!tItr->polarity) {
            /// TODO - Implement negative triggers
            continue;
        }
        if (tItr->second) {
            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[tItr->first];

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
            //cout << "Now, adding " << *(RPGBuilder::getLiteral(addingThisWouldTrigger.first)) << " won't kill " << pref->name << endl;
            
            map<int, AddingConstraints >::iterator fcItr = prefCostOfAddingFact.find(tItr->first);
            assert(fcItr != prefCostOfAddingFact.end());
           
            fcItr->second.addingWillViolate.erase(prefIdx);
            fcItr->second.extraGoalsToAvoidViolations.push_back(make_pair(prefIdx, wasTheTrigger));
                           
        }
    }
       
    preferencesThatNowHaveNoAddCosts.push_back(make_pair(prefIdx,0));
                                                                   
}
                                   

/// Precondition preferences

bool PreferenceHandler::recordPreconditionViolations = false;
    
map<int,int> PreferenceHandler::preconditionViolations;

PreconditionPref::PreconditionPref(const int & idx, RPGBuilder::Preference * const p)
: PreferenceFSA(idx, p)
{
    p->neverTrue = false;
    
    if (!p->d->flattened[0]) {
        p->neverTrue = !(p->d->nodes[0].second);
    }
}

void PreconditionPref::update(MinimalState * state)
{
    if (!goalOrTriggerIsSatisfied(state, 0)) {
        if (PreferenceHandler::preferenceDebug) cout << "Precondition preference " << pref->name << " not satisfied, cost " << pref->cost << endl;
        state->prefPreconditionViolations += pref->cost;
        if (PreferenceHandler::recordPreconditionViolations) {
            ++(PreferenceHandler::preconditionViolations.insert(make_pair(prefIdx,0)).first->second);
        }
    } else {
        if (PreferenceHandler::preferenceDebug) cout << "Precondition preference " << pref->name << " not satisfied, cost " << pref->cost << endl;
    }
}

double PreconditionPref::currentCost(MinimalState * state, const int & flag)
{
    exit(1);
    return DBL_MAX;    
}

double PreconditionPref::reachableCost(MinimalState * state, const int & flag)
{   
    exit(1);
    return DBL_MAX;
    
}

double PreconditionPref::GCost(MinimalState * state, const int & flag)
{    
    exit(1);
    return DBL_MAX;
}

void PreconditionPref::noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate)
{
    PrefCommon::noteVariablesForFactAtSomePoint(toUpdate, prefIdx, pref, true);
}

SwitchVarInfo PreconditionPref::extendLP(map<int,list<pair<int, int> > > & factNeededForCol,MinimalState * state,MILPRPG * model)
{        
    
    if (pref->neverTrue) {
        return noColRowData;
    } else {
        return PrefCommon::extendLPForFactAtSomePoint(state, model, switchVarName, prefIdx, pref, boilerplate[0], true, true);
    }
};

void PreconditionPref::importTriggerGroups(map<int, set<int> > & d, int & extraVars)
{
    if (!pref->neverTrue) {
        extraVars += extraVariablesFor(0);    
        extraVars += 2;
    }
    
}

void PreconditionPref::getUnsatisfiedConditionCounts(MinimalState & state, vector<NNF_Flat*> & toFill)
{
    if (pref->d->flattened[0]) {        
        toFill[0] = pref->d->flattened[0];
        toFill[0]->reset();
    }
}




/// Simple preference handler functions

void PreferenceHandler::extendLP(vector<SwitchVarInfo> & switchVars, MinimalState * state,MILPRPG * model)
{
    
    map<int,list<pair<int,int> > > factNeededForCol;
    
    const int pCount = automata.size();
    switchVars.resize(pCount,noColRowData);
    
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

void PreferenceHandler::addHelperTriggers(const vector<SwitchVarInfo> & switchVarForPreference, MinimalState* state, MILPRPG* model)
{
    const int tpCount = RPGBuilder::getTaskPrefCount();

    for (int p = 0; p < tpCount; ++p) {
        automata[p]->addHelperTriggers(switchVarForPreference, state, model);
    }
}

void PreferenceHandler::update(MinimalState & state)
{
    if (PreferenceHandler::preferenceDebug) cout << "Updating preference automata following action application\n";
    const int pCount = RPGBuilder::getTaskPrefCount();
    
    for (int p = 0; p < pCount; ++p) {
        if (PreferenceHandler::preferenceDebug) cout << "p)"; cout.flush();
        automata[p]->update(&state);
    }
}

double PreferenceHandler::getCurrentCost(MinimalState & state, const int flag)
{
    
    double toReturn = state.prefPreconditionViolations;
    
    const int pCount = RPGBuilder::getTaskPrefCount();
    
    for (int p = 0; p < pCount; ++p) {
        toReturn += automata[p]->currentCost(&state, flag);
    }
    
    return toReturn;
}

string PreferenceHandler::getCurrentViolations(MinimalState & state)
{
    
    string toReturn = "Violations:";
    
    const int pCount = RPGBuilder::getTaskPrefCount();
    
    for (int p = 0; p < pCount; ++p) {
        automata[p]->currentViolations(&state, toReturn);
    }
 
    {
     
        const map<string,int> & permanents = RPGBuilder::getPermanentViolationDetails();
        map<string,int>::const_iterator pItr = permanents.begin();
        const map<string,int>::const_iterator pEnd = permanents.end();
        
        for (; pItr != pEnd; ++pItr) {
            for (int i = 0; i < pItr->second; ++i) {
                toReturn += (" " + pItr->first);
            }
        }
        
    }
    
    if (recordPreconditionViolations) {
        map<int,int>::const_iterator rvItr = preconditionViolations.begin();    
        const map<int,int>::const_iterator rvEnd = preconditionViolations.end();
        
        for (; rvItr != rvEnd; ++rvItr) {
            const string pname = " " + RPGBuilder::getPreferences()[rvItr->first].name;
            
            for (int i = 0; i < rvItr->second; ++i) {
                toReturn += pname;
            }
        }
    }
    return toReturn;
}


double PreferenceHandler::getReachableCost(MinimalState & state, const int flag)
{
    
    double toReturn = state.prefPreconditionViolations;
    
    const int pCount = RPGBuilder::getTaskPrefCount();
    
    for (int p = 0; p < pCount; ++p) {
        toReturn += automata[p]->reachableCost(&state, flag);
    }
        
    return toReturn;
}

double PreferenceHandler::getG(MinimalState & state, const int flag)
{
    
    double toReturn = state.prefPreconditionViolations;
    
    const int pCount = RPGBuilder::getTaskPrefCount();
    
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
    
    const int tpCount = RPGBuilder::getTaskPrefCount();
    list<int>::const_iterator tmItr = toMark.begin();
    const list<int>::const_iterator tmEnd = toMark.end();
    
    for (; tmItr != tmEnd; ++tmItr) {        
        if (*tmItr < tpCount) {
            if (theState.preferenceStatus[*tmItr] == unreachable) continue;
            assert(automata[*tmItr]->GCost(&theState,3) == 0.0);
            
            theState.preferenceStatus[*tmItr] = unreachable;
            
            toReturn += automata[*tmItr]->GCost(&theState,3);
        }
    }
    
    return toReturn;
}

void PreferenceHandler::getUnsatisfiedConditionCounts(MinimalState & theState, vector<vector<NNF_Flat*> > & toFill)
{
    const int pCount = automata.size();
    toFill.resize(pCount, vector<NNF_Flat*>(2,(NNF_Flat*)0) );
    
    for (int p = 0; p < pCount; ++p) {
        automata[p]->getUnsatisfiedConditionCounts(theState, toFill[p]);
    }
}

void PreferenceHandler::getCostsOfDeletion(MinimalState & theState, map<int, set<int> > & prefCostOfDeletingFact, map<int, map<double, set<int> > > & prefCostOfChangingNumber)
{
    const int tpCount = RPGBuilder::getTaskPrefCount();
    for (int p = 0; p < tpCount; ++p) {
        automata[p]->getCostsOfDeletion(theState, prefCostOfDeletingFact, prefCostOfChangingNumber);
    }
}

void PreferenceHandler::getCostsOfAdding(MinimalState & theState, map<int, AddingConstraints > & prefCostOfDeletingFact, map<int, map<double, AddingConstraints > > & prefCostOfChangingNumberB)
{
    const int tpCount = RPGBuilder::getTaskPrefCount();
    
    for (int p = 0; p < tpCount; ++p) {
        automata[p]->getCostsOfAdding(theState, prefCostOfDeletingFact, prefCostOfChangingNumberB);
    }
}

void PreferenceHandler::updateCostsAndPreferenceStatus(MinimalState & initialState, const pair<int,bool> & whatHasBeenSatisfied,
                                                       vector<AutomatonPosition> & optimisticAutomataPositions,
                                                       map<int, AddingConstraints > & prefCostOfAddingFact,
                                                       map<int, map<double, AddingConstraints > > & prefCostOfChangingNumberB,
                                                       list<pair<int,int> > & preferencesThatNowHaveNoAddCosts)
{
    static const int tpCount = RPGBuilder::getTaskPrefCount();
    if (whatHasBeenSatisfied.first >= tpCount) return;
    
    automata[whatHasBeenSatisfied.first]->updateCosts(initialState, whatHasBeenSatisfied.second, optimisticAutomataPositions, prefCostOfAddingFact, prefCostOfChangingNumberB, preferencesThatNowHaveNoAddCosts);
    if (preferenceDebug) {
        cout << "\t\tAfter that, optimistic preference status is " << positionName[optimisticAutomataPositions[whatHasBeenSatisfied.first]] << endl;
    }
}

void PreferenceHandler::getDesiredGoals(list<list<Literal*> > & desired, list<list<int> > * desiredNumeric,
                                        MinimalState & startState,
                                        const vector<vector<NNF_Flat*> > & unsatisfiedPreferenceConditions, const set<int> & prefsUnsatisfied,
                                        vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer)
{
    const int tpCount = RPGBuilder::getTaskPrefCount();
    
    set<int>::const_iterator puItr = prefsUnsatisfied.begin();
    const set<int>::const_iterator puEnd = prefsUnsatisfied.end();
    
    int p = 0;
    
    while (p < tpCount && puItr != puEnd) {
        if (p < *puItr) {
            automata[p]->getDesiredGoals(desired, desiredNumeric, startState, unsatisfiedPreferenceConditions, literalCosts, numericAchievedInLayer);
            ++p;
        } else {
            ++p;
            ++puItr;
        }
    }
    
    for (; p < tpCount; ++p) {
        automata[p]->getDesiredGoals(desired, desiredNumeric, startState, unsatisfiedPreferenceConditions, literalCosts, numericAchievedInLayer);
    }
    
}

/// PreferenceHandler 2.0 - now with ADL etc.

void PreferenceHandler::initialiseNNF()
{
    
    WhereAreWeNow = PARSE_CONSTRAINTS;
    
    vector<RPGBuilder::Preference> & prefs = RPGBuilder::getPreferences();
    const unsigned int prefCount = prefs.size();     
    
    
    for (unsigned int p = 0; p < prefCount; ++p) {
        prefs[p].d = new PreferenceData();
        for (int part = 0; part < 2; ++part) {
            VAL::goal * const lookAt = (part ? prefs[p].parsed_trigger : prefs[p].parsed_goal);
            if (lookAt == 0) {
                continue;
            }
            if (PreferenceHandler::preferenceDebug) {
                cout << "Building NNF for " << prefs[p].name << ":" << p << " part " << part << "\n";
            }
            prefs[p].d->nodes[part] = NNFUtils::buildNNF(theTC,prefs[p].fe,lookAt);
            if (prefs[p].d->nodes[part].first) {
                if (PreferenceHandler::preferenceDebug) {
                    cout << "NNF " << p << " for " << prefs[p].name << ":" << p << " part " << part << "\n";
                    prefs[p].d->nodes[part].first->write(cout);
                }
            } else {                
                if (PreferenceHandler::preferenceDebug) {
                    cout << "NNF " << p << " for " << prefs[p].name << " evaluates to ";
                    if (prefs[p].d->nodes[part].second) {
                        cout << "true\n";
                    } else {
                        cout << "false\n";
                    } 
                }
                
            }
        }
    }
    
    WhereAreWeNow = PARSE_UNKNOWN;
}

void PreferenceHandler::substituteRPGNumericPreconditions(RPGBuilder::BuildingNumericPreconditionData & commonData) {

    WhereAreWeNow = PARSE_CONSTRAINTS;

    vector<RPGBuilder::Preference> & prefs = RPGBuilder::getPreferences();
    const unsigned int prefCount = prefs.size();     
    
    
    for (unsigned int i = 0; i < prefCount; ++i) {
        for (int part = 0; part < 2; ++part) {
            NNFNode * const oldRoot = prefs[i].d->nodes[part].first;
            if (oldRoot) {
                prefs[i].d->nodes[part] = NNFUtils::substituteRPGNumerics(oldRoot,commonData);
            }
        }
    }

    WhereAreWeNow = PARSE_UNKNOWN;
};

void PreferenceHandler::pruneStaticLiterals(vector<pair<bool,bool> > & staticLiterals) {
    
    vector<RPGBuilder::Preference> & prefs = RPGBuilder::getPreferences();
    const unsigned int prefCount = prefs.size();     
    
    for (int unsigned i = 0; i < prefCount; ++i) {
        PreferenceData * const d = prefs[i].d;
        assert(d);
        for (unsigned int part = 0; part < 2; ++part) {
            assert(part < d->nodes.size());
            NNFNode * const oldRoot = d->nodes[part].first;
            if (oldRoot) {
                prefs[i].d->nodes[part] = NNFUtils::pruneStaticLiterals(oldRoot,staticLiterals);
            }
        }
    }
    
        
};


void PreferenceHandler::flattenNNF() {
    
    vector<RPGBuilder::Preference> & prefs = RPGBuilder::getPreferences();
    const unsigned int prefCount = prefs.size();     
    
    for (unsigned int i = 0; i < prefCount; ++i) {
        for (int part = 0; part < 2; ++part) {
            NNFNode * & oldRoot = prefs[i].d->nodes[part].first;
            if (oldRoot) {
                oldRoot = NNFUtils::simplifyNNF(oldRoot);            
                if (oldRoot) {
                    prefs[i].d->flattened[part] = NNFUtils::flattenNNF(oldRoot);
                    if (preferenceDebug) {
                        cout << "NNF for " << prefs[i].name << ":" << i << " part " << part << ":\n";
                        cout << *(prefs[i].d->flattened[part]);
                    }
                } else {
                    cout << "NNF simplified to " << oldRoot << endl;
                }
            }
        }
    }
    initPTRTable();
    
    
    #ifndef NDEBUG
    
    
    for (unsigned int i = 0; i < prefCount; ++i) {
        for (int part = 0; part < 2; ++part) {            
            list<Literal*> & dest = (part ? prefs[i].debug_trigger : prefs[i].debug_goal);
            list<pair<int,int> > & destNum = (part ? prefs[i].debug_triggerRPGNum : prefs[i].debug_goalRPGNum);
            list<int>::const_iterator litItr = prefs[i].d->conditionLiterals[part].begin();
            const list<int>::const_iterator litEnd = prefs[i].d->conditionLiterals[part].end();
            
            for (; litItr != litEnd; ++litItr) {
                dest.push_back(RPGBuilder::getLiteral(*litItr));
            }
            
            list<int>::const_iterator numItr = prefs[i].d->conditionNums[part].begin();
            const list<int>::const_iterator numEnd = prefs[i].d->conditionNums[part].end();
            
            for (; numItr != numEnd; ++numItr) {
                destNum.push_back(make_pair(*numItr,-1));
            }
            

            
        }
    }
    
    #endif
    
};

bool PreferenceHandler::couldBeBeneficialToOppose(const int & p, const bool & wasTheTrigger)
{
    const uint taskPrefCount = RPGBuilder::getTaskPrefCount();
    
    if (p >= taskPrefCount) {
        // is never a good idea to violate precondition preferences
        return false;
    }
        
    vector<RPGBuilder::Preference> & prefs = RPGBuilder::getPreferences();
    
    switch (prefs[p].cons) {
        case VAL::E_ATMOSTONCE: {
            // at most onces are best avoided
            return true;
        }
        case VAL::E_SOMETIMEAFTER:
        case VAL::E_SOMETIMEBEFORE: {
           
            
            // it can be beneficial to avoid the trigger of a sometime after to avoid having to reach the requirement,
            // or to avoid the trigger of a sometime before to avoid violating the preference if the requirement has not yet been seen
            return wasTheTrigger;
            //return true; // this should be wrong
        }                           
        default: {
            return false;
            // return true;// this should be wrong
        }
    }
    
}

bool PreferenceHandler::couldBeBeneficialToSupport(const int & p, const bool & wasTheTrigger)
{
    const uint taskPrefCount = RPGBuilder::getTaskPrefCount();
    
    if (p >= taskPrefCount) {
        // is never a good idea to violate precondition preferences, so always support them
        return true;
    }
        
    vector<RPGBuilder::Preference> & prefs = RPGBuilder::getPreferences();
    
    switch (prefs[p].cons) {
        case VAL::E_ATMOSTONCE: {
            // at most onces are best avoided
            return false;
            //return true; // this should be wrong
        }
        case VAL::E_SOMETIMEAFTER:
        case VAL::E_SOMETIMEBEFORE: {
           
            // support the requirement of sometime afters or sometime before, but not the triggers
            return !wasTheTrigger;
            //return true; // this should be wrong
        }                           
        default: {
            // anything else - always, at end, sometime, etc.  need to be supported
            return true;
        }
    }
    
}

void PreferenceHandler::initPTRTable()
{
    if (ptrTableInit >= 2) return;

    const unsigned int litCount = RPGBuilder::getLiteralCount();
    const unsigned int numPreCount = RPGBuilder::getNumericPreTable().size();
    
    preconditionsToPrefs.resize(litCount);
    negativePreconditionsToPrefs.resize(litCount);
    
    numericPreconditionsToPrefs.resize(numPreCount);
    negativeNumericPreconditionsToPrefs.resize(numPreCount);
            
    
    vector<RPGBuilder::Preference> & prefs = RPGBuilder::getPreferences();
    const unsigned int prefCount = prefs.size();     
    
    for (unsigned int i = 0; i < prefCount; ++i) {
        PreferenceData* const d = prefs[i].d;
        for (int part = 0; part < 2; ++part) {
            const pair<int,bool> currReference(i, part == 1);
            
            NNFNode * const oldRoot = d->nodes[part].first;
            
            if (oldRoot) {
                NNFUtils::buildLiteralsToPreconditions<pair<int,bool> >(d->flattened[part],
                    preconditionsToPrefs,negativePreconditionsToPrefs,
                    numericPreconditionsToPrefs, negativeNumericPreconditionsToPrefs,
                    d->conditionLiterals[part], d->conditionNegativeLiterals[part],
                    d->conditionNums[part], d->conditionNegativeNums[part],
                    currReference);

                d->flattened[part]->reset();
                if (d->flattened[part]->isSatisfied()) defaultTruePrefParts.insert(currReference);
            }
        }
    }
    
    for (unsigned int i = 0; i < numPreCount; ++i) {
        if (!numericPreconditionsToPrefs[i].empty() || !negativeNumericPreconditionsToPrefs[i].empty()) {
            relevantNumericPreconditions.push_back(i);
        }
    }

    if (preferenceDebug) {
        for (unsigned int l = 0; l < litCount; ++l) {
            for (int isneg = 0; isneg < 2; ++isneg) {                
                list<LiteralCellDependency<pair<int,bool> > >::iterator depItr = (isneg ? negativePreconditionsToPrefs[l].begin() : preconditionsToPrefs[l].begin());
                const list<LiteralCellDependency<pair<int,bool> > >::iterator depEnd = (isneg ? negativePreconditionsToPrefs[l].end() : preconditionsToPrefs[l].end());

                for (int p = 0 ; depItr != depEnd; ++depItr,++p) {
                    if (!p) {
                        if (isneg) cout << "¬";
                        cout << *(RPGBuilder::getLiteral(l)) << " is used in:\n";
                    }
                    if (depItr->dest.second) {
                        cout << "\tTrigger of preference ";
                    } else {
                        cout << "\tGoal of preference ";
                    }                
                    cout << prefs[depItr->dest.first].name;
                    cout << ":" << depItr->dest.first << " @ " << depItr->index << "\n";
                }
            }
        }
    }

    ptrTableInit = 2;
};

void PreferenceHandler::getPreconditionsToSatisfy(list<Literal*> & pres, list<int> * numPres, const pair<int,bool> & satisfied,
                                                  vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer)
{
    automata[satisfied.first]->getPreconditionsToSatisfy(pres, numPres, satisfied.second, literalCosts, numericAchievedInLayer);
}
                                      

void PreferenceHandler::aboutToApply(MinimalState & theState, const int & actID)
{
    
    const vector<int> & prePrefs = RPGBuilder::getStartPreferences()[actID];
    
    const int ppCount = prePrefs.size();
    
    if (preferenceDebug) {
        if (ppCount) {
            cout << "Checking the " << ppCount << " precondition preferences of " << *(RPGBuilder::getInstantiatedOp(actID)) << endl;
        } else {
            cout << "No precondition preferences attached to " << *(RPGBuilder::getInstantiatedOp(actID)) << endl;
        }
    }
    
    for (int p = 0; p < ppCount; ++p) {
        automata[prePrefs[p]]->update(&theState);
    }
    
}
                                          

};

