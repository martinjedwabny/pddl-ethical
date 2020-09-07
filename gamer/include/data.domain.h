#ifndef _DATA_DOMAIN_H
#define _DATA_DOMAIN_H

#include <string>
#include <vector>
#include <map>
using namespace std;

class Action;
class Predicate;
class LispEntity;
class SymbolicFact;
class Formula;

class Domain {
  Domain(const Domain &copy); // prevent copying

  string domainName, problemName;

  vector< pair < pair <int,int>, Formula*> >  HoldDuring;
  vector<pair <int,Formula*> > HoldAfter;
  vector<pair <int,Formula*> > Within;
  vector<pair <int,Formula*> > AlwaysWithin; 

  vector<pair <string, pair <int,Formula*> > > PrefHoldAfter;
  vector<pair <string, pair < pair <int,int>, Formula*> > > PrefHoldDuring;
  vector<pair <string, pair <int,Formula*> > > PrefWithin;
  vector<pair <string, pair <int, Formula* > > > PrefAlwaysWithin; 

  vector<Predicate *> predicates;
  vector<Action *> actions;
  vector<int> initFacts, goalFacts;
  vector<pair <int,int> > initNumFacts;
  map<int, bool> pred;
  map<string, int> predicateTable;

  map<string, Formula*> preferenceTable;

  int factCount;    // total number of possible fact instantiations
  bool timed;

  LispEntity* metric;
  bool maximize;

  double getOperatorCount() const;

  void parse(vector<LispEntity>& problem, 
	     vector<LispEntity>& domain, 
	     vector<LispEntity>::iterator& pos);

  void parsingStep(string domFile, string probFile, string addFile);


public:
  Domain() {}
  Domain(string domFile, string probFile);
  ~Domain();

  bool isTimed() { return timed; }

  bool isPredicate(int i) {return pred[i];}
  void setPredicate(int i, bool val) {pred[i] = val;}
  map<string,int>& getPredicateTable() { return predicateTable; }

  map<string,Formula*>& getPreferenceTable() { return preferenceTable; }

  int getPredicateCount()           {return predicates.size();}
  Predicate &getPredicate(int idx)  {return *predicates[idx];}
  Predicate* lookupPredicate(string str); // throws "not found" exception
  Predicate* lookupTypedPredicate(string str);
  vector<vector<int> >  gr; 

  vector< pair < pair <int,int>, Formula*> >&  getHoldDuring() 
      { return HoldDuring; }
  vector<pair <int,Formula*> >& getHoldAfter()
      { return HoldAfter; }
  vector<pair <int,Formula*> >& getWithin()
      { return Within; }
  vector<pair <int, Formula* > >& getAlwaysWithin()
      { return AlwaysWithin; }
  vector<pair <string, pair <int,Formula*> > >& getPrefHoldAfter()
      { return PrefHoldAfter; }
  vector<pair <string, pair < pair <int,int>, Formula*> > >& 
      getPrefHoldDuring() { return PrefHoldDuring; }
  vector<pair <string, pair <int, Formula*> > >& getPrefWithin() 
      {return PrefWithin;}
  vector<pair <string, pair <int, Formula* > > >&
      getPrefAlwaysWithin() { return PrefAlwaysWithin; } 

  int getFactCount()               {return factCount; }
  int getActionCount()              {return actions.size();}
  Action &getAction(int idx)        {return *actions[idx];}


  LispEntity* getMetric() { return metric; }
  bool getMaximize() { return maximize; }

  int isInitFact(int i) {
    for (int j = 0; j < initFacts.size(); j++) {
      if (initFacts[j] == i) return 1;
    }
    return 0;
  }

  int isInitNumFact(int i) {
    for (int j = 0; j < initNumFacts.size(); j++) {
      if (initNumFacts[j].first == i) return 1;
    }
    return 0;
  }

  int initNumValue(int i) {
    for (int j = 0; j < initNumFacts.size(); j++) {
      if (initNumFacts[j].first == i) 
	  return initNumFacts[j].second;
    }
    return 0;
  }
  int isGoalFact(int i) {
    for (int j = 0; j < goalFacts.size(); j++) {
      if (goalFacts[j] == i) return 1;
    }
    return 0;
  }
  string getDomainName() {return domainName;}
  string getProblemName() {return problemName;}
  string toString();
};

#endif
