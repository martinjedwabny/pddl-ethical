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

#ifndef PREFERENCEHANDLER_H
#define PREFERENCEHANDLER_H

#include "RPGBuilder.h"
#include "MILPRPG.h"
#include "NNF.h"

namespace Planner {
  
struct LPData {
    int switchesNeeded;
    
    bool switchIsRoot;
    vector<vector<pair<int,double> > > switchRowEntries;
    vector<pair<double,double> > switchRowBounds;
    
    vector<int> encodeCellUsingSwitchID;
    vector<int> currentSwitchIDColumnScratch;
    
    LPData()
    : switchesNeeded(0), switchIsRoot(false)
    {
    }
};

struct ConditionAndPolarity {
    
    int first;
    bool second;
    bool polarity;
    
    ConditionAndPolarity()
        : first(-1), second(false), polarity(true)
    {
    }

    ConditionAndPolarity(const int & a, const bool & b, const bool & c)
        : first(a), second(b), polarity(c)
    {
    }
};
    
class PreferenceFSA {

protected:
    
    uint prefIdx;
    string switchVarName;
    RPGBuilder::Preference * pref;

    PreferenceFSA(const int & i, RPGBuilder::Preference * const p);

    int triggerType;
    int goalType;
    
    vector<LPData> boilerplate;
    
    int extraVariablesFor(const int & lookAtTrigger);
    
    
    /**
     *  Work out whether the goal/trigger of a preference is satisfied in the
     *  given state.  Setting partIdx to 1 checks the trigger; to 0, the goal.
     */
    virtual bool goalOrTriggerIsSatisfied(MinimalState *, const int & partIdx);
    
    virtual void satisfyAtLowCost(const NNF_Flat *, vector<list<PreferenceSetAndCost> > *, vector<double> *, list<list<Literal*> > &, list<list<int> >* );
    
    virtual void pushNumericDependencies(map<int, list<pair<int,int> > > &, const int & partIdx);
public:
    virtual const vector<vector<int> >* getComparisonData() = 0;

    virtual void getPreconditionsToSatisfy(list<Literal*> & pres, list<int> * numericPres, const bool & theTrigger,
                                           vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer);
                                          
                                          
    
    virtual SwitchVarInfo extendLP(map<int, list<pair<int, int> > >&, MinimalState *, MILPRPG *) = 0;
    virtual void anotherPreferenceWants(const map<int, list<pair<int, int> > > &, MinimalState *, MILPRPG*)
    {
    }
    virtual void addHelperTriggers(const vector<SwitchVarInfo> &, MinimalState *, MILPRPG *)
    {
    }
    virtual void update(MinimalState *) = 0;
    virtual double currentCost(MinimalState *, const int &) = 0;
    virtual void currentViolations(MinimalState * s, string & v) {
        if (currentCost(s,3) > 0.0) {
            v += " " + pref->name;
        }
    }
    virtual double reachableCost(MinimalState *, const int &) = 0;
    virtual double GCost(MinimalState *, const int &) = 0;
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate)
    {
    }
    virtual void importTriggerGroups(map<int, set<int> > &, int &) = 0;
    //virtual void populateFactToPreferenceMappings();
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<NNF_Flat*> &) = 0;
    
    virtual void getCostsOfDeletion(MinimalState &, map<int, set<int> > &, map<int, map<double, set<int> > > &)
    {
    }
    virtual void getCostsOfAdding(MinimalState &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &)
    {
    }
    virtual void updateCosts(MinimalState &, const bool &, vector<AutomatonPosition> &, map<int, AddingConstraints > &,
                             map<int, map<double, AddingConstraints > > &, list<pair<int,int> > &)
    {
    }
    
    virtual void getDesiredGoals(list<list<Literal*> > & , list<list<int> > *, MinimalState &, const vector<vector<NNF_Flat*> > &, vector<list<PreferenceSetAndCost> > *, vector<double> *)
    {
    }
};

class AlwaysFSA : public PreferenceFSA {

public:
    AlwaysFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);

    
    virtual const vector<vector<int> >* getComparisonData();
    virtual SwitchVarInfo extendLP(map<int, list<pair<int, int> > >&, MinimalState *, MILPRPG *);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void importTriggerGroups(map<int, set<int> > &, int & toInc) {
        toInc += extraVariablesFor(0);
    }
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<NNF_Flat*> &);
    virtual void getCostsOfDeletion(MinimalState &, map<int, set<int> > &, map<int, map<double, set<int> > > &);

};

class AtEndFSA : public PreferenceFSA {


public:
    AtEndFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);

    virtual const vector<vector<int> >* getComparisonData();
    virtual SwitchVarInfo extendLP(map<int, list<pair<int, int> > >&, MinimalState *, MILPRPG *);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void importTriggerGroups(map<int, set<int> > &, int & toInc) {
        toInc += extraVariablesFor(0);
    }    
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<NNF_Flat*> &);
    virtual void getDesiredGoals(list<list<Literal*> > & desired, list<list<int> > * desiredNumeric, MinimalState & startState, const vector<vector<NNF_Flat*> > & unsatisfiedPreferenceConditions, vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer);
    virtual void updateCosts(MinimalState &, const bool &, vector<AutomatonPosition> &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &, list<pair<int,int> > &);

};

class SometimeFSA : public PreferenceFSA {

public:
    SometimeFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);

    virtual const vector<vector<int> >* getComparisonData();
    virtual SwitchVarInfo extendLP(map<int, list<pair<int, int> > >&, MinimalState *, MILPRPG *);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void importTriggerGroups(map<int, set<int> > &, int & toInc) {
        toInc += extraVariablesFor(0);
    }    
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<NNF_Flat*> &);
    virtual void getDesiredGoals(list<list<Literal*> > & desired, list<list<int> > * desiredNumeric, MinimalState & startState, const vector<vector<NNF_Flat*> > & unsatisfiedPreferenceConditions, vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer);
    virtual void updateCosts(MinimalState &, const bool &, vector<AutomatonPosition> &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &, list<pair<int,int> > &);

};

class AtMostOnceFSA : public PreferenceFSA {
    set<int> actionsImplyingTrigger;
    int triggerPartCount;
    bool addingOneThingCanTrigger;
    list<ConditionAndPolarity> addingThisWouldTrigger;
public:
    AtMostOnceFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);
    
    virtual const vector<vector<int> >* getComparisonData();
    virtual SwitchVarInfo extendLP(map<int, list<pair<int, int> > >&, MinimalState *, MILPRPG *);
    virtual void addHelperTriggers(const vector<SwitchVarInfo>&, MinimalState *, MILPRPG *);
    virtual void anotherPreferenceWants(const map<int, list<pair<int, int> > > &, MinimalState *, MILPRPG*);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    virtual void importTriggerGroups(map<int, set<int> > & d, int &);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<NNF_Flat*> &);
    virtual void getCostsOfAdding(MinimalState &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &);

    
};

class SometimeBeforeFSA : public PreferenceFSA {

    set<int> actionsImplyingTrigger;
    int triggerPartCount;
    
    bool addingOneThingCanTrigger;
    list<ConditionAndPolarity> addingThisWouldTrigger;
public:
    SometimeBeforeFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);

    virtual const vector<vector<int> >* getComparisonData();
    virtual SwitchVarInfo extendLP(map<int, list<pair<int, int> > >&, MinimalState *, MILPRPG *);
    virtual void addHelperTriggers(const vector<SwitchVarInfo> &, MinimalState *, MILPRPG *);
    virtual void anotherPreferenceWants(const map<int, list<pair<int, int> > > &, MinimalState *, MILPRPG*);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    virtual void importTriggerGroups(map<int, set<int> > & d, int &);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<NNF_Flat*> &);
    virtual void getCostsOfAdding(MinimalState &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &);
    virtual void updateCosts(MinimalState &, const bool &, vector<AutomatonPosition> &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &, list<pair<int,int> > &);
    
    
};

class SometimeAfterFSA : public PreferenceFSA {
    set<int> actionsImplyingTrigger;
    int triggerPartCount;
    bool addingOneThingCanTrigger;
    list<ConditionAndPolarity> addingThisWouldTrigger;
public:
    SometimeAfterFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);

    virtual const vector<vector<int> >* getComparisonData();
    virtual SwitchVarInfo extendLP(map<int, list<pair<int, int> > >&, MinimalState *, MILPRPG *);
    virtual void addHelperTriggers(const vector<SwitchVarInfo>&, MinimalState *, MILPRPG *);
    virtual void anotherPreferenceWants(const map<int, list<pair<int, int> > > &, MinimalState *, MILPRPG*);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    virtual void importTriggerGroups(map<int, set<int> > & d, int &);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<NNF_Flat*> &);
    virtual void getCostsOfAdding(MinimalState &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &);
    virtual void updateCosts(MinimalState &, const bool &, vector<AutomatonPosition> &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &, list<pair<int,int> > &);
    virtual void getDesiredGoals(list<list<Literal*> > & desired, list<list<int> > * desiredNumeric, MinimalState & startState, const vector<vector<NNF_Flat*> > & unsatisfiedPreferenceConditions, vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer);

};

class PreconditionPref : public PreferenceFSA {
    
public:
    PreconditionPref(const int & i, RPGBuilder::Preference * const p);
        
    virtual const vector<vector<int> >* getComparisonData()
    {
        return 0;
    }
    virtual SwitchVarInfo extendLP(map<int, list<pair<int, int> > >&, MinimalState *, MILPRPG *);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual void currentViolations(MinimalState *, string &)
    {
    }
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    virtual void importTriggerGroups(map<int, set<int> > & d, int &);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<NNF_Flat*> &);
    
    
};


class PreferenceHandler {
protected:
    friend class PreferenceFSA;
    static vector<const vector<vector<int> >* > comparisonData;
    static vector<PreferenceFSA*> automata;
    static vector<AutomatonPosition> initialAutomataPositions;
    
    static vector<list<LiteralCellDependency<pair<int,bool> > > > preconditionsToPrefs;
    static vector<list<LiteralCellDependency<pair<int,bool> > > > negativePreconditionsToPrefs;
    static vector<list<LiteralCellDependency<pair<int,bool> > > > numericPreconditionsToPrefs;
    static vector<list<LiteralCellDependency<pair<int,bool> > > > negativeNumericPreconditionsToPrefs;
    
    static vector<vector<pair<int, bool> > > mappingFromFactsToPreferences;
    static vector<vector<pair<int, bool> > > mappingFromNumericFactsToPreferences;
    
    static set<pair<int,bool> > defaultTruePrefParts;
    static vector<int> relevantNumericPreconditions;
    
    static void initPTRTable();    
    static int ptrTableInit;
    
public:

    static bool preferenceDebug;
    static bool recordPreconditionViolations;
    
    static map<int,int> preconditionViolations;
    
    static void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    static void buildAutomata();
    static const vector<AutomatonPosition> & getInitialAutomataPositions()
    {
        return initialAutomataPositions;
    }

    static void aboutToApply(MinimalState & theState, const int & actID);
    static void update(MinimalState & theState);
    static double getCurrentCost(MinimalState & theState, const int flag=3);
    static string getCurrentViolations(MinimalState & theState);
    static double getReachableCost(MinimalState & theState, const int flag=3);
    static double getG(MinimalState & theState, const int flag=3);
    

    static const vector<list<LiteralCellDependency<pair<int,bool> > > > * getPreconditionsToPrefs()
    {
        return &preconditionsToPrefs;
    }
    
    static const vector<list<LiteralCellDependency<pair<int,bool> > > > * getNegativePreconditionsToPrefs()
    {
        return &negativePreconditionsToPrefs;
    }
    
    static const vector<list<LiteralCellDependency<pair<int,bool> > > > * getNumericPreconditionsToPrefs()
    {
        return &numericPreconditionsToPrefs;
    }
    
    static const vector<list<LiteralCellDependency<pair<int,bool> > > > * getNegativeNumericPreconditionsToPrefs()
    {
        return &negativeNumericPreconditionsToPrefs;
    }
    

    static void getUnsatisfiedConditionCounts(MinimalState &, vector<vector<NNF_Flat*> > &);

    static void getCostsOfDeletion(MinimalState &, map<int, set<int> > & prefCostOfDeletingFact, map<int, map<double, set<int> > > & prefCostOfChangingNumberA);
    static void getCostsOfAdding(MinimalState &, map<int, AddingConstraints > & prefCostOfAddingFact, map<int, map<double, AddingConstraints > > & prefCostOfChangingNumberB);
    
    static void updateCostsAndPreferenceStatus(MinimalState &,
                                               const pair<int,bool> & whatHasBeenSatisfied,
                                               vector<AutomatonPosition> & optimisticAutomataPositions,
                                               map<int, AddingConstraints > & prefCostOfAddingFact,
                                               map<int, map<double, AddingConstraints > > & prefCostOfChangingNumberB,
                                               list<pair<int,int> > & preferencesThatNowHaveNoAddCosts);
                                               
                                               
    
    // Functions for integrating preferences into the LP
    static void extendLP(vector<SwitchVarInfo> &, MinimalState *, MILPRPG *);
    static void addHelperTriggers(const vector<SwitchVarInfo> &, MinimalState *, MILPRPG *);
    static void importTriggerGroups(map<int, set<int> > &, int & extraVars);
    static double markUnreachables(MinimalState &, const list<int> &);
    
    static void getDesiredGoals(list<list<Literal*> > & desired, list<list<int> > * desiredNumeric,
                                MinimalState & startState,
                                const vector<vector<NNF_Flat*> > & unsatisfiedPreferenceConditions,const set<int> & prefsUnsatisfied,
                                vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer);

    static void getPreconditionsToSatisfy(list<Literal*> & pres, list<int> * numPres, const pair<int,bool> & satisfied,
                                          vector<list<PreferenceSetAndCost> > * literalCosts, vector<double> * numericAchievedInLayer);
                                                            
                                                            
    static void initialiseNNF();
    static void flattenNNF();    
    static bool couldBeBeneficialToOppose(const int & pref, const bool & wasTheTrigger);
    static bool couldBeBeneficialToSupport(const int & pref, const bool & wasTheTrigger);
    static void pruneStaticLiterals(vector<pair<bool,bool> > & staticLiterals);    
    
    static void substituteRPGNumericPreconditions(RPGBuilder::BuildingNumericPreconditionData & commonData);
    static inline int compareStatusOfPref(const int & prefIdx, const AutomatonPosition & a, const AutomatonPosition & b) {
        return ((*comparisonData[prefIdx])[(int) a][(int) b]);
    }
    
};

extern bool canBeSatisfied(const AutomatonPosition & p);
extern bool isSatisfied(const AutomatonPosition & p);

};

#endif // PREFERENCEHANDLER_H
