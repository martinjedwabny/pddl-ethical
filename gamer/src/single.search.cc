
#include <vector>
#include <fstream>      

using namespace std;

#include <util.options.h>
#include <util.tools.h>
#include <util.queue.h>
#include <util.bitarray.h>

#include <out.operator.h>
#include <out.factMap.h>

#include <single.state.h>
#include <single.search.h>

const int MAX_BRANCHING = 10000;

Search::Search(FactMap& fMap): 
  factMap(fMap), opindex(0), max(options.maxStates()) {

  initState = new State(fMap.getInitialState(),fMap); 
  goalState = new State(fMap.getGoalState(),fMap);
  FactArray = new int[fMap.getNoFacts()+1];

  finalState = new State(*initState);

/*
  succStates = new State*[MAX_BRANCHING];
  for(int i=0;i<MAX_BRANCHING;i++)
    succStates[i] = new State(*initState);
*/  
  oSize = factMap.getOperators().size();  
  operators = new Operator*[oSize];
  // preActions = new Operator**[fMap.getNoFacts()+1];
  // aSize = new int[fMap.getNoFacts()+1];

  int count = 0;
  for(int i = 0; i < oSize; i++) {
      if (factMap.getOperators()[i])
	  operators[count++] = factMap.getOperators()[i];
  }
  oSize = count;
/*
  BitArray* temp = new BitArray(factMap.getNoFacts());

  *initState->getVector() -= *factMap.getDerived();

  while (!(*temp == *initState->getVector())) {
    *temp = *initState->getVector();
    initState->getVector()->toIntArray(FactArray,fSize);
      
    for(int j = 0; j < oSize; j++) {
      Operator* oper = operators[j];
      if (!oper->derived) continue;
      if (oper->matchAllPreds(initState->getVector())) {
	oper->apply(initState->getVector());
      }
    }
    
    initState->getVector()->toIntArray(FactArray,fSize);
    
    for (int i=0; i<fSize; i++) {
      int fact = FactArray[i];
      int psize = aSize[fact];
      for(int o = 0; o < psize; o++) {
	Operator* oper = preActions[fact][o];
	if (!oper->derived) continue;
	if (oper->matchAllPreds(initState->getVector())) {
	  oper->apply(initState->getVector());
	}
      }
    }    
  }
*/

  heur = NULL;

}

int Search::pert(Timer& globalTimer, vector<State *> path) {
  int* start = new int[path.size()];
  Operator** ops = new Operator*[path.size()];
  for (int i=0; i<path.size()-1 ; i++) { 
    // cout << "i" << i 
    // << factMap.getOperator(path[i]->action)->toString() << endl;
    ops[path.size()-i-2] = factMap.getOperator(path[i]->action);
  }
  if (options.debug(Options::EXPLORE))
    cout << endl << "Refining solution with scheduler:" << endl;    

  for (int i=0; i< path.size()-1 ; i++) { 
    start[i] = 1;
    for (int j=0; j<i ; j++) { 
      if (ops[j]->depend(ops[i])) {
	if (start[j] + 1 > start[i]) {
	  start[i] = start[j] + 1;
	}
      }
    }
  }
  
  int maxi = 0; double max = -1;
  for (int i=0; i<path.size()-1 ; i++)  
    if (start[i] > max) {
      max = start[i];
      maxi = i;
    } 
  vector<pair<double,int> > sched;
  for (int i=0; i<path.size()-1 ; i++) 
    sched.push_back(make_pair(start[i]-1,i));
  sort(sched.begin(), sched.end());
  
  for (int i=0; i<path.size()-1; i++) {
    cout << sched[i].first+1 << ": (" 
	 << ops[sched[i].second]->toString() << ")" << endl; 
  }
  cout << "Total Time: " << globalTimer.total() << endl;
  return start[maxi];
}


int Search::pert(vector<State *> path) {
  int* start = new int[path.size()];
  Operator** ops = new Operator*[path.size()];
  for (int i=0; i<path.size()-1 ; i++) { 
    ops[path.size()-i-2] = factMap.getOperator(path[i]->action);
  }

  for (int i=0; i< path.size()-1 ; i++) { 
    start[i] = 1;
    for (int j=0; j<i ; j++) { 
      if (ops[j]->depend(ops[i])) {
	if (start[j] + 1 > start[i]) {
	  start[i] = start[j] + 1;
	}
      }
    }
  }
  
  int maxi = 0; double max = -1;
  for (int i=0; i<path.size()-1 ; i++)  
    if (start[i] > max) {
      max = start[i];
      maxi = i;
    } 

  return start[maxi];
}

float Search::pertDurative(Timer& globalTimer, vector<State *> path) {
  float* start = new float[path.size()];
  Operator** ops = new Operator*[path.size()];
  for (int i=0; i<path.size()-1 ; i++) { 
    // cout << "i" << i 
    // << factMap.getOperator(path[i]->action)->toString() << endl;
    ops[path.size()-i-2] = factMap.getOperator(path[i]->action);
  }
  if (options.debug(Options::EXPLORE))
    cout << endl << "Refining solution with scheduler:" << endl;    

  for (int i=0; i< path.size()-1 ; i++) { 
    start[i] = ops[i]->duration;
    for (int j=0; j<i ; j++) { 
      if (ops[j]->depend(ops[i])) {
	if (start[j] + ops[j]->duration > start[i]) {
	  start[i] = start[j] + ops[j]->duration;
	}
      }
    }
  }
  
  int maxi = 0; double max = -1;
  for (int i=0; i<path.size()-1 ; i++)  
    if (start[i] > max) {
      max = start[i];
      maxi = i;
    } 
  vector<pair<double,int> > sched;
  for (int i=0; i<path.size()-1 ; i++) 
    sched.push_back(make_pair(start[i]-ops[i]->duration,i));
  sort(sched.begin(), sched.end());
  
  for (int i=0; i<path.size()-1; i++) {
    cout << sched[i].first << ": (" 
	 << ops[sched[i].second]->toString() << ")" << endl; 
  }
  cout << "Total Time: " << globalTimer.total() << endl;
  return start[maxi];
}


float Search::pertDurative(vector<State *> path) {
  float* start = new float[path.size()];
  Operator** ops = new Operator*[path.size()];
  for (int i=0; i<path.size()-1 ; i++) { 
    ops[path.size()-i-2] = factMap.getOperator(path[i]->action);
  }

  for (int i=0; i< path.size()-1 ; i++) { 
    start[i] = ops[i]->duration;
    for (int j=0; j<i ; j++) { 
      if (ops[j]->depend(ops[i])) {
	if (start[j] + ops[j]->duration > start[i]) {
	  start[i] = start[j] + ops[j]->duration;
	}
      }
    }
  }
  
  int maxi = 0; double max = -1;
  for (int i=0; i<path.size()-1 ; i++)  
    if (start[i] > max) {
      max = start[i];
      maxi = i;
    } 

  return start[maxi];
}


State* Search::exploreSpace(Timer& globalTimer, int& length) {

  if(options.verbose(Options::EXPLORE)) 
    cout << endl << "Exploring space ..." << endl;


  expansions = generations = 0;
  int hval, i, found;
  vector<State *> path;
  State* rstate = NULL;


  Timer tim;
  cout << "Searching Time:" << flush;

  found = search(globalTimer,finalState);

  if (found) 
      return finalState;

  if (found == 0) {
    if (!options.competition())
      cout << "---No Solution found" << endl;
  }
  return 0;
/*
  else {
    while (finalState != 0) {
      if (!options.competition())
    cout << finalState->toString() << endl;
      path.push_back(finalState);
      finalState = finalState->pred;
    }
    printSolution(globalTimer,path);
  }
*/

}




void Search::printSolution(Timer& globalTimer, vector<State *> path) {

  double length;
/*
  if (!factMap.getDurative()) 
    length = pert(globalTimer,path);
  else
    length = pertDurative(globalTimer,path);
  cout << "Expansions " << expansions << endl;
  cout << "Optimized Value " << length << endl;
  if(options.verbose(Options::EXPLORE)) 
    cout << endl << "---Solved in " << expansions  
     << " parallel length " << length << endl;
*/
}

string Search::toString () {
  string back = "";
  back += "  operators:\n";
  for(int i = 0; i < oSize; i++) 
    back += operators[i]->toString();
  return back;
}
