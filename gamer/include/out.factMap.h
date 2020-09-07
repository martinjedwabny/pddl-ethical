
#ifndef _FACT_MAP_
#define _FACT_MAP_

#include <vector>
#include <list>
#include <string>
#include <map>
#include <algorithm>
#include <util.tools.h>
#include <bdd.h>
#include <fdd.h>

using namespace std;

class Domain;          // forward class declarations
class Action;
class Group;
class Operator;
class BitArray;
class State;
class Arithmetics;

class FactMap {

  int noFacts;            // the number of grounded fluents
  int noActions;          // the number of grounded actions          
  int encodingSize;       // the encoding size of a state
  int actIndex;           // index variable set for bdd exploration

  Domain& domain;         // handle especially for debug proposes 

  bdd 
    preVariables,         // pre-image variables for bdd repr. & exploration 
    effVariables,         // corresponding image variables 
    actVariables,         // action variables for non-deterministic planning
    origVariables;         // original variable set for BDD export
  bdd 
    *initialBdd, *goalBdd;  // bdd repr. of initial and goal state
  bddPair                 // for replacing variable set back and forth
      *preEff, *effPre,
      *origPre, *origEff,
      *preOrig, *effOrig; 
  int* pre;               // pre image indices to allow replacement in bdd
  int* eff;               // corresponding eff imabe indices
  int* orig;
  bdd* fdd;

  bdd objective;          // objective function in BDD representation
  int objectiveIndex;     // index of objective function 
  int maxObjective;       // maximum int in objective function
  int minObjective;       // minimum int in objective function

  vector<bdd*> preAddFactBdd;  // bdds for propositional add atoms in pre-image
  vector<bdd*> effAddFactBdd;  // bdds for propositional add atoms in pre-image
  vector<bdd*> preDelFactBdd;  // bdds for propositional del atoms in image
  vector<bdd*> effDelFactBdd;  // bdds for propositional del atoms in image

  vector<vector<bdd*> > preIthBdd;  // bdds for num 
  vector<vector<bdd*> > effIthBdd; // bdds for num 

  Arithmetics* bddArithmetics; // links to arithmetic operations on bdds

  vector <vector <bdd> > hold; 
  vector <vector <pair <string,bdd> > > prefhold; 
  vector <vector <bdd> > with; 
  vector <vector <pair <string,bdd> > > prefwith; 

  vector<bdd> holdTable;
  map<string,bdd> bddTable;
  bdd prefVariables;
  int maxViolated;
  int fullGroupCount;

  BitArray* tvector;      // temporal bitvector 
  BitArray* derived;      // state bitvector mask (derived 1, others 0)
  vector<string> action_name;   // vector of actions for output of solution
  vector<string> variable_name; // vector of fact
  vector<int> initial_state;    // fact indices for initial state
  vector<int> initial_numstate;    // fact num indices for initial state
  vector<int> initial_numvalue;    // fact num indices for initial state
  vector<int> goal_state;       // fact indices for goal state
  vector<Operator*> operators;  // set of grounded operators
  vector<Group* > groups;       // partition of facts into groups
  //  vector<BitArray* > bitTable;  // vector of bitvectors

  int* group;        // vector of group indices for given atoms
  int* position;    // vector of position indices for given atoms
  bool inherentSequential;     // denotes inherent sequential problems
  bool timed;                  // denotes timed domain
  bool maximize;                  // denotes maximized metric
  bool durative;                  // denotes durative actions

 public:

  FactMap(int fCount,          // constructor called with instantiated  
	  Domain& dom); 
  ~FactMap();                  
  void initOperatorTable(vector<Action *> actions);  // init grounded operator
  void initPreference();
    
  // member functions for the above variables

  void initOptimizer();
  int getTimed() { return timed; } // domain contains timed initial facts

  int getIndex() { return objectiveIndex; }
  bdd getObjective() { return objective; }
  int getMaxObjective() { return maxObjective; }
  int getMinObjective() { return minObjective; }

  void initHold();

  vector <vector <bdd> >& getHold() { return hold; }
  vector <vector <pair <string,bdd> > >& getPrefHold() { return prefhold; }
  vector <vector <bdd> >& getWithin() { return with; }
  vector <vector <pair <string,bdd> > >& getPrefWithin() { return prefwith; }
  
  int numAutomata();

  Group* getGroupFacts(int i) { return groups[i]; }
  int getGroup(int i) { return group[i]; }
  int groupCount() { return groups.size(); }
  int groupFullCount() { return fullGroupCount; }

  Arithmetics* getArithmetics() { return bddArithmetics; }
  int getActIndex() { return actIndex; }
  int getPreIndex(int i) { return pre[i]; }
  int getEffIndex(int i) { return eff[i]; }

  bdd* getPreAddFactBdd(int i) { return preAddFactBdd[i]; }
  bdd* getEffAddFactBdd(int i) { return effAddFactBdd[i]; }
  bdd* getPreDelFactBdd(int i) { return preDelFactBdd[i]; }
  bdd* getEffDelFactBdd(int i) { return effDelFactBdd[i]; }

  bdd* getPreLessFactBdd(int i, int v);
  bdd* getPreLessEqualFactBdd(int i, int v);
  bdd* getPreGreaterFactBdd(int i, int v);
  bdd* getPreGreaterEqualFactBdd(int i, int v);
  bdd* getPreEqualFactBdd(int i, int v);
  bdd* getEffDecFactBdd(int i, int v);
  bdd* getEffIncFactBdd(int i, int v);
  bdd* getEffAssFactBdd(int i, int v);


  bdd getInitialBdd() { return *initialBdd; }
  bdd getGoalBdd() { return *goalBdd; }
  string lookup(bdd oldState, bdd newState);  
  Operator* lookupOperator(bdd oldState, bdd newState);
  void initBdds();          // inits bdds 
  bdd getHeurVariables(); 
  int getPreHeurIndex();
  int getEffHeurIndex();
  bdd& getActVariables() { return actVariables; }
  bdd& getPreVariables() { return preVariables; }
  bdd& getEffVariables() { return effVariables; }

  bdd& getPrefVariables() { return prefVariables; }
  bdd& getFdd(int i) { return fdd[i]; }
  bddPair* getPreEff() { return preEff; }
  bddPair* getEffPre() { return effPre; } 
  bddPair* getPreOrig() { return preOrig; }
  bddPair* getEffOrig() { return effOrig; } 
  bddPair* getOrigPre() { return origPre; }
  bddPair* getOrigEff() { return origEff; } 

  bool getMaximize() { return maximize; }
  BitArray* getDerived() { return derived; }

  void setDurative() { durative = true; }  
  bool getDurative() { return durative; }

  int getNoFacts() { return noFacts; }
  int getEncodingSize() { return encodingSize; }
  Domain& getDomain() { return domain;}
  vector<Operator*>& getOperators() { return operators; }
  Operator* getOperator(int i) { return operators[i]; }
  void initGroupTable(vector<vector<int> >);
 
  vector<Group *>& getGroups() { return groups; }
  vector<int>& getInitialState() { return initial_state; }
  vector<int>& getGoalState() { return goal_state; }
  bool isGoal(int i) { 
    return find(goal_state.begin(),goal_state.end(),i) !=
	goal_state.end();}
  bool isInitial(int i) { 
    return find(initial_state.begin(),initial_state.end(),i) !=
      initial_state.end();}

  int getNoActions() { return noActions; }
  int getNoOperators() { return operators.size(); }
  bool isInherentSequential() { return inherentSequential; }

  string getProblemName(); 
  string getDomainName();
  string getFactName(int i) { return variable_name[i]; }
  string getActionName(int i) { return action_name[i]; }

  map<string,bdd>& getBddTable() { return bddTable; }
    
  // methods

  void run(Timer& t);                       // exec different algorithms
  string toString();                        // output information in string
};

#endif
