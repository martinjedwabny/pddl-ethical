
#include <fstream>
#include <util.options.h>
#include <util.tools.h>
#include <util.queue.h>
#include <util.prioqueue.h>
#include <util.bitarray.h>
#include <out.factMap.h>
#include <out.operator.h>
#include <single.search.h>
#include <bdd.bfs.h>
#include <bdd.arithmetics.h>
#include <bdd.h>

int BddBfs::BFS(Timer& globalTimer, State*& finalState) {
    // if(!options.competition())
    cout << "  applying symbolic BFS search ... " 
	 << endl;

  Timer t;

  init = factMap.getInitialBdd();
  goal = factMap.getGoalBdd();
  bdd forwardReached = init;
  bdd backwardReached = goal;
  int forwardTime = 0;
  int backwardTime = 0;


  //negatedVariables = bdd_exist(init,normalVariables);
  //negatedVariables = bdd_replace(negatedVariables,preEff);

  map<string,bdd> bddTable = factMap.getBddTable();
  bdd prefVariables = factMap.getPrefVariables();

  int maxdur = (1 << options.maxLength()) -1;
  vector<bdd> forwardBdd;
  vector<bdd> backwardBdd;

  for (int i=0;i<maxdur;i++) forwardBdd.push_back(bddfalse);
  for (int i=0;i<maxdur;i++) backwardBdd.push_back(bddfalse);

  int findex = 0;    
  int bindex = 0;    
  forwardBdd[0] |= init;
  backwardBdd[0] |= goal;


  int iteration = 0;
  bdd intersection = bdd_replace(goal,effPre) & init;
  bool forward;
      
 while (intersection == bddfalse) {

    bdd forwardFrontier = forwardBdd[findex];
    bdd backwardFrontier = backwardBdd[bindex];

    forward = 1; // (forwardTime <= backwardTime);

    cout << "- expanding depth [" << iteration << "]" 
	 << (forward ? "forward" : "backward") << endl;
    
    ++iteration;
    if(options.verbose(Options::SEARCHING))
	cout << " Depth " << iteration << " ("
	     << (forward ? "for" : "back") << "ward) " << flush;
    else if(!options.silent(Options::SEARCHING))
	cout << '.' << flush;
    if(forward) {
       if (searchStep(forwardFrontier, backwardFrontier,
		      forwardReached, preVariables,
		      effPre, forwardBdd,
		      intersection, t, forwardTime,findex++)) {
	   break;  
       }
    }
    else {
	if (searchStep(backwardFrontier, forwardFrontier,
		       backwardReached, effVariables,
		       preEff, backwardBdd,
		       intersection, t, backwardTime,bindex++)) {
	    break;
	}
    }
    intersection = bdd_replace(intersection,effPre);
    

    
  }
  if(!options.verbose(Options::SEARCHING)
     && !options.silent(Options::SEARCHING))
      cout << endl;
  
  if (intersection == bddfalse) {
      if(!options.competition()) {
	  cout << endl << " No solution found " << iteration
	       << "; searching time: " << t.total() << endl;
      } 
  }  
  else {
      cout << "Time: " << globalTimer.total() << endl;
      solutionPrintBFS(forwardBdd, backwardBdd, 	    
		       intersection, -1, globalTimer);
  } 
  return -1;
}





