
#include <fstream>
#include <util.options.h>
#include <util.tools.h>
#include <util.queue.h>
#include <util.prioqueue.h>
#include <util.bitarray.h>
#include <out.factMap.h>
#include <out.operator.h>
#include <single.heuristic.h>
#include <single.search.h>
#include <single.pdb.h>
#include <bdd.pattern.h>
#include <bdd.bfs.h>
#include <bdd.arithmetics.h>
#include <bdd.h>

int BddBfs::Dijkstra(Timer& globalTimer, State*& finalState) {
    // if(!options.competition())
    cout << "  applying symbolic single source shortest path search ... " 
	 << endl;

  Timer t;

  init = factMap.getInitialBdd();
  goal = factMap.getGoalBdd();

  bdd_printtable(init);
  bdd_printtable(goal);

  bdd forwardReached = init;
  bdd backwardReached = goal;
  int forwardTime = 0;
  int backwardTime = 0;


  //negatedVariables = bdd_exist(init,normalVariables);
  //negatedVariables = bdd_replace(negatedVariables,preEff);

  map<string,bdd> bddTable = factMap.getBddTable();
  bdd prefVariables = factMap.getPrefVariables();

  int maxdur = (1 << options.maxLength()) -1;
  vector<vector<bdd> > reconstructionBdd;
  vector<bdd> forwardBdd;
  vector<bdd> backwardBdd;

  for (int i=0;i<maxdur;i++) forwardBdd.push_back(bddfalse);
  vector<bdd> dummy;
  for (int i=0;i<maxdur;i++) reconstructionBdd.push_back(dummy);
  for (int i=0;i<maxdur;i++) backwardBdd.push_back(bddfalse);

  int findex = 0;    
  int bindex = 0;    
  forwardBdd[0] |= init;
  backwardBdd[0] |= goal;


  int iteration = 0;
  bdd intersection = bdd_replace(goal,effPre) & init;
  bool forward;
      
//  while (intersection == bddfalse) {
  while (1) {

    bdd forwardFrontier = forwardBdd[findex];
    bdd backwardFrontier = backwardBdd[bindex];

    reconstructionBdd[findex].push_back(forwardFrontier);


    // forward = (forwardTime <= backwardTime);
    forward = 1;  
    
    intersection = bdd_replace(backwardFrontier,effPre) & 
	forwardFrontier;
    
    if (!(intersection == bddfalse)) {
	cout << " best goal expanded in depth " << findex << endl;
	break;
    }
    
    
    cout << "- expanding depth [" << findex << "]" << (forward ? 
							  "forward" : "backward") << endl;
    
    ++iteration;

    if(forward) {
	if (searchStep(forwardFrontier, backwardFrontier,
		       forwardReached, preVariables,
		       effPre, forwardBdd,
		       intersection, t, forwardTime,findex)) {
	    break;
	}
	if (forwardFrontier != forwardBdd[findex]) {
//	    cout << " 0-cost operators " << endl;
	}
	else {
	    findex++;
	}
    }
    else {
	if (searchStep(backwardFrontier, forwardFrontier,
		       backwardReached, effVariables,
		       preEff, backwardBdd,
		       intersection, t, backwardTime,bindex)) {
	    break;
	}
	bindex++;
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
      cout << "iterations " << iteration << "/" 
	   << reconstructionBdd.size() << endl;
      cout << "cost " << findex << endl;
      solutionPrintZero(reconstructionBdd, backwardBdd, 	    
		       intersection, findex, globalTimer);
  } 
  return -1;
}





