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

namespace Planner {
  
class PreferenceFSA {

protected:
    uint prefIdx;
    string switchVarName;
    RPGBuilder::Preference * pref;

    PreferenceFSA(const int & i, RPGBuilder::Preference * const p);

    int initialUnsatisfiedTriggerPreconditions;
    int initialUnsatisfiedGoalPreconditions;

    int triggerType;
    int goalType;
    
public:

    virtual pair<int, int> extendLP(map<int, list<int> >&, MinimalState *, MILPRPG *) = 0;
    virtual void anotherPreferenceWants(const map<int, list<int> > &, MinimalState *, MILPRPG*)
    {
    }
    virtual void addHelperTriggers(const vector<pair<int, int> >&, MinimalState *, MILPRPG *)
    {
    }
    virtual void update(MinimalState *) = 0;
    virtual double currentCost(MinimalState *, const int &) = 0;
    virtual double reachableCost(MinimalState *, const int &) = 0;
    virtual double GCost(MinimalState *, const int &) = 0;
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate)
    {
    }
    virtual void importTriggerGroups(map<int, set<int> > &, int &) {
    }
    virtual void populateFactToPreferenceMappings();
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<int> &) = 0;
    
    virtual void getCostsOfDeletion(MinimalState &, map<int, set<int> > &, map<int, map<double, set<int> > > &)
    {
    }
    virtual void getCostsOfAdding(MinimalState &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &)
    {
    }
    virtual void updateCosts(MinimalState &, const bool &, vector<AutomatonPosition> &, map<int, AddingConstraints > &,
                             map<int, map<double, AddingConstraints > > &, list<int> &)
    {
    }
    
    virtual void getDesiredGoals(list<list<Literal*> * > & , MinimalState &, const vector<vector<int> > &)
    {
    }
};

class AlwaysFSA : public PreferenceFSA {

public:
    AlwaysFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);

    virtual pair<int, int> extendLP(map<int, list<int> >&, MinimalState *, MILPRPG *);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<int> &);
    virtual void getCostsOfDeletion(MinimalState &, map<int, set<int> > &, map<int, map<double, set<int> > > &);

};

class AtEndFSA : public PreferenceFSA {


public:
    AtEndFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);

    virtual pair<int, int> extendLP(map<int, list<int> >&, MinimalState *, MILPRPG *);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<int> &);
    virtual void getDesiredGoals(list<list<Literal*> * > & desired, MinimalState & startState, const vector<vector<int> > & unsatisfiedPreferenceConditions);
    virtual void updateCosts(MinimalState &, const bool &, vector<AutomatonPosition> &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &, list<int> &);

};

class SometimeFSA : public PreferenceFSA {

public:
    SometimeFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);

    virtual pair<int, int> extendLP(map<int, list<int> >&, MinimalState *, MILPRPG *);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<int> &);
    virtual void getDesiredGoals(list<list<Literal*> * > & desired, MinimalState & startState, const vector<vector<int> > & unsatisfiedPreferenceConditions);
    virtual void updateCosts(MinimalState &, const bool &, vector<AutomatonPosition> &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &, list<int> &);

};

class AtMostOnceFSA : public PreferenceFSA {
    set<int> actionsImplyingTrigger;
    int triggerPartCount;
    bool addingOneThingCanTrigger;
    pair<int,bool> addingThisWouldTrigger;
public:
    AtMostOnceFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);
    
    virtual pair<int, int> extendLP(map<int, list<int> >&, MinimalState *, MILPRPG *);
    virtual void addHelperTriggers(const vector<pair<int, int> >&, MinimalState *, MILPRPG *);
    virtual void anotherPreferenceWants(const map<int, list<int> > &, MinimalState *, MILPRPG*);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    virtual void importTriggerGroups(map<int, set<int> > & d, int &);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<int> &);
    virtual void getCostsOfAdding(MinimalState &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &);

    
};

class SometimeBeforeFSA : public PreferenceFSA {

    set<int> actionsImplyingTrigger;
    int triggerPartCount;
    
    bool addingOneThingCanTrigger;
    pair<int,bool> addingThisWouldTrigger;
public:
    SometimeBeforeFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);

    virtual pair<int, int> extendLP(map<int, list<int> >&, MinimalState *, MILPRPG *);
    virtual void addHelperTriggers(const vector<pair<int, int> >&, MinimalState *, MILPRPG *);
    virtual void anotherPreferenceWants(const map<int, list<int> > &, MinimalState *, MILPRPG*);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    virtual void importTriggerGroups(map<int, set<int> > & d, int &);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<int> &);
    virtual void getCostsOfAdding(MinimalState &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &);
    virtual void updateCosts(MinimalState &, const bool &, vector<AutomatonPosition> &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &, list<int> &);
    
    
};

class SometimeAfterFSA : public PreferenceFSA {
    set<int> actionsImplyingTrigger;
    int triggerPartCount;
    bool addingOneThingCanTrigger;
    pair<int,bool> addingThisWouldTrigger;
public:
    SometimeAfterFSA(const int & i, RPGBuilder::Preference * const p, AutomatonPosition & initPosn);

    virtual pair<int, int> extendLP(map<int, list<int> >&, MinimalState *, MILPRPG *);
    virtual void addHelperTriggers(const vector<pair<int, int> >&, MinimalState *, MILPRPG *);
    virtual void anotherPreferenceWants(const map<int, list<int> > &, MinimalState *, MILPRPG*);
    virtual void update(MinimalState *);
    virtual double currentCost(MinimalState *, const int &);
    virtual double reachableCost(MinimalState *, const int &);
    virtual double GCost(MinimalState *, const int &);
    virtual void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    virtual void importTriggerGroups(map<int, set<int> > & d, int &);
    virtual void getUnsatisfiedConditionCounts(MinimalState &, vector<int> &);
    virtual void getCostsOfAdding(MinimalState &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &);
    virtual void updateCosts(MinimalState &, const bool &, vector<AutomatonPosition> &, map<int, AddingConstraints > &, map<int, map<double, AddingConstraints > > &, list<int> &);
    virtual void getDesiredGoals(list<list<Literal*> * > & desired, MinimalState & startState, const vector<vector<int> > & unsatisfiedPreferenceConditions);

};


class PreferenceHandler {
protected:
    friend class PreferenceFSA;
    static vector<PreferenceFSA*> automata;
    static vector<AutomatonPosition> initialAutomataPositions;
    static vector<vector<pair<int, bool> > > mappingFromFactsToPreferences;
    static vector<vector<pair<int, bool> > > mappingFromNumericFactsToPreferences;

public:

    static bool preferenceDebug;
    static void noteWhichVariablesWeMightWantOptimisticBoundsFor(vector<uint> & toUpdate);
    static void buildAutomata();
    static const vector<AutomatonPosition> & getInitialAutomataPositions()
    {
        return initialAutomataPositions;
    }

    static void update(MinimalState & theState);
    static double getCurrentCost(MinimalState & theState, const int flag=3);
    static double getReachableCost(MinimalState & theState, const int flag=3);
    static double getG(MinimalState & theState, const int flag=3);
    

    // Functions for integrating preferences into the RPG

    static const vector<vector<pair<int, bool> > > * getMappingFromFactsToPreferences()
    {
        return &(mappingFromFactsToPreferences);
    }

    static const vector<vector<pair<int, bool> > > * getMappingFromNumericFactsToPreferences()
    {
        return &(mappingFromNumericFactsToPreferences);
    }

    static void getUnsatisfiedConditionCounts(MinimalState &, vector<vector<int> > &);

    static void getCostsOfDeletion(MinimalState &, map<int, set<int> > & prefCostOfDeletingFact, map<int, map<double, set<int> > > & prefCostOfChangingNumberA);
    static void getCostsOfAdding(MinimalState &, map<int, AddingConstraints > & prefCostOfAddingFact, map<int, map<double, AddingConstraints > > & prefCostOfChangingNumberB);
    
    static void updateCostsAndPreferenceStatus(MinimalState &,
                                               const pair<int,bool> & whatHasBeenSatisfied,
                                               vector<AutomatonPosition> & optimisticAutomataPositions,
                                               map<int, AddingConstraints > & prefCostOfAddingFact,
                                               map<int, map<double, AddingConstraints > > & prefCostOfChangingNumberB,
                                               list<int> & preferencesThatNowHaveNoAddCosts);
                                               
                                               
    
    // Functions for integrating preferences into the LP
    static void extendLP(vector<pair<int, int> > &, MinimalState *, MILPRPG *);
    static void addHelperTriggers(const vector<pair<int, int> > &, MinimalState *, MILPRPG *);
    static void importTriggerGroups(map<int, set<int> > &, int & extraVars);
    static double markUnreachables(MinimalState &, const list<int> &);
    
    static void getDesiredGoals(list<list<Literal*> * > & desired, MinimalState & startState, const vector<vector<int> > & unsatisfiedPreferenceConditions, const set<int> & prefsUnsatisfied);

};

extern bool canBeSatisfied(const AutomatonPosition & p);
extern bool isSatisfied(const AutomatonPosition & p);

};

#endif // PREFERENCEHANDLER_H
