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

int BddBfs::Bdda(Timer& globalTimer, State*& finalState) {

  int MAXF = (1 << options.maxLength()) - 1;
  

//  if(!options.competition()) 
  cout << "  applying heuristic symbolic bucket search ... " << MAXF << endl;
//  exit(1);
  Timer t;
  init = factMap.getInitialBdd(); // pre
  goal = factMap.getGoalBdd(); // eff
  bdd forwardReached = options.forwardPDB() ? init : goal;
  bdd backwardReached = options.forwardPDB() ? goal : init;
  int forwardTime = 0;
  int backwardTime = 0;
  vector< vector<bdd> > forwardBdd; // buckets
  vector<bdd> backwardBdd;
  vector<bdd> solutionBdd;
  vector<bdd> tempBdd;

  for (int j=0; j< MAXF;j++) {
    tempBdd.clear();
    solutionBdd.push_back(bddfalse);
    for (int i=0; i< MAXF;i++) 
      tempBdd.push_back(bddfalse);
    forwardBdd.push_back(tempBdd);
  }
  
  //  bdd hinit = bddfalse;

  int fmin=0;
  while (fmin < MAXF) {
      if ((((PDB *) heur)->getBdd(fmin,forwardReached)) == bddfalse)
	  fmin++;
      else {
	  if (!options.isMaximize())
	      break;
      }
  }
  
  if (fmin == MAXF) {
      cout << "problem proven unsolvable " << endl;
      exit(1);
  }
  forwardBdd[0][fmin] = options.forwardPDB() ? init : goal; 
  backwardBdd.push_back( options.forwardPDB() ? goal : init);

  int iteration = 0;
  bdd intersection = init & bdd_replace(goal, effPre);
  bool forward = 1; 
  int gmin, hmax;


  int objectiveIndex[1];
  objectiveIndex[0] = index;
  bool maximize = factMap.getMaximize();      
  int upperbound = factMap.getMaxObjective();
  int lowerbound = factMap.getMinObjective();

  bdd objectiveVariables;

  map<string,bdd> bddTable = factMap.getBddTable();
  bdd prefVariables = factMap.getPrefVariables();
  bdd isIn, pref, outcome;
  int initiallowerbound = lowerbound;
  int initialupperbound = upperbound;
  

  if (index != -1) {

      objectiveVariables = fdd_makeset(objectiveIndex,1);
      cout << "go down" << endl;

      for (int l = lowerbound;l<=upperbound;l++) {
	  bdd irel = bdd_relprod(objective,fdd_ithvar(index,l-lowerbound),
				 objectiveVariables);
	  rel.push_back(irel);
      }
      
      pref = bdd_exist(intersection,prefVariables);
      outcome = pref;
      for(map<string,bdd>::iterator pos = 
	      bddTable.begin(); pos != bddTable.end(); ++pos) {
	  cout << "evaluating preference " << pos->first << endl;
	  outcome = outcome & ((bdd &) pos->second);
//	  bdd_printtable(pos->second);
      }
      
    
  }
  else 
      outcome = intersection;

  do { // branch-and-bound loop
      if (index != -1) {
	  isin = bddfalse;
	  cout << "restricting solution interval to [" 
	       << lowerbound << "," << upperbound << "]" << endl;
	  for (int l=lowerbound;l<=upperbound;l++)
	      isin |= rel[l-factMap.getMinObjective()];
      }
      else 
	  isin = bddtrue;
      
      if (fmin==MAXF) { 
	  cout << " no fmin bucket exploration completed" << endl;
	  exit(1);
      }
      while ((outcome & isin) == bddfalse) {
	  
	  for (gmin=0;gmin<=fmin;gmin++) { // bdda algorithm
 	      hmax = fmin-gmin;
//	      cout << gmin <<"/"<< hmax << endl;
	      if (forwardBdd[gmin][hmax]  != bddfalse) {
		  
		  if(options.verbose(Options::SEARCHING))
		      cout << "- expanding bucket [" << gmin <<"," << hmax << "]" 
			   << " nodes: " << bdd_nodecount(forwardBdd[gmin][hmax]) 
/*			   << " states: " 
			   << bdd_satcountset(forwardBdd[gmin][hmax], 
					      options.forwardPDB() ? 
					      preVariables : effVariables) 
*/
<< endl;

		  bdd forwardFrontier = forwardBdd[gmin][hmax] & 
		      ! solutionBdd[gmin];
		  solutionBdd[gmin] |= forwardBdd[gmin][hmax];

		  intersection = bdd_replace(goal,effPre) & 
		      forwardBdd[gmin][hmax];
    
		  if (!(intersection == bddfalse)) {
		      cout << " best goal expanded " << endl;
		      solutionPrintBFS(solutionBdd, backwardBdd, 
				       intersection, 
				       gmin,globalTimer);

		      exit(1);
		      break;
		  }
		  


		  bdd backwardFrontier = backwardBdd.back();
		  
		  forwardBdd[gmin][hmax] = bddfalse;

		  ++iteration;
		  if(options.verbose(Options::SEARCHING))
		      cout << " Iteration " << iteration << " (forward) " 
			   << flush;
		  else if(!options.silent(Options::SEARCHING))
		      cout << '.' << flush;
		  vector<bdd> temp;

		  for (int count=0;count<MAXF;count++)
		      temp.push_back(bddfalse);
		  
		  if (searchStep
		      (forwardFrontier, backwardFrontier, 
		       forwardReached, 
		       options.forwardPDB()?preVariables:effVariables, 
		       options.forwardPDB()? effPre : preEff, 
		       temp, intersection, t, forwardTime, 0))
		      break;
		  
		  for (int d=0; d<temp.size();d++) {
		      if (temp[d] == bddfalse)
			  continue;
		      for (int dist=0; dist < MAXF; dist++) {
			  bdd lookup =
			      ((PDB *) heur)->getBdd(dist,temp[d]);
			  if (lookup == bddfalse)
			      continue;
			  if (gmin+d +dist < fmin) { 
			      cout << "Inconsistency found from " << fmin 
				   << " to " << gmin+d+dist-1 
				   << " to " << gmin << "/" << d << "/" 
				   << dist << endl;
			      exit(1);
			      fmin = gmin+d+dist-1;
			  }
			  forwardBdd[gmin+d][dist] =
			      forwardBdd[gmin+d][dist] |
			      lookup;
		      }

		  }
	      }
	  } // end-of-diagonal
	  fmin++;

	  intersection = bdd_replace(intersection,effPre);
	  pref = bdd_relprod(intersection,bddtrue,prefVariables);
	  outcome = pref;
	  outcome = bddfalse;

	  for(map<string,bdd>::iterator pos = 
		  bddTable.begin(); pos != bddTable.end(); ++pos) {
	      // cout << "..." << (string &) (pos->first) << "  :  ";
	      outcome &= ((bdd &) pos->second);
	  }
      }
      if(!options.verbose(Options::SEARCHING)
	 && !options.silent(Options::SEARCHING))
	    cout << endl;
	if (intersection == bddfalse) {
	    if(!options.competition()) {
		cout << endl << " No solution found " << iteration
		     << "; searching time: " << t.total() << endl;
	    } else {
		cout << globalTimer.total() << endl;
	    }
	} else {
	    cout << endl << " Solution found now constructing it " << iteration
		 << "; searching time: " << t.total() << endl;
	    
	    if (index != -1) {
		bdd ithval;
		if (maximize) { 
		    for (int l=upperbound;l>=lowerbound;l--) {
			cout << "...downwards searching for next lower bound= " << l 
			     << " lower bound=" << lowerbound 
			     << " upper bound=" << upperbound << endl;
			ithval = outcome & rel[l-factMap.getMinObjective()]; 
			if (!(ithval == bddfalse)) { 
			    cout << "solution-quality = " << l << endl;
			    cout << "flushing plan in file <mips-solution> " << endl;
			    solutionPrintBFS(solutionBdd, backwardBdd, 
					  bdd_relprod(ithval,bddtrue,prefVariables), 
					  l,globalTimer);
			    cout << "done " << endl;
			    lowerbound = l+1;
			    if (upperbound <= lowerbound) exit(1);
			    exit(1);
			}
		    } 
		}
		else {
		    for (int l=lowerbound;l<=upperbound;l++) {
			cout << "...upwards searching for next upper bound= " 
			     << l 
			     << " lower bound=" << lowerbound 
			     << " upper bound=" << upperbound << endl;
			ithval = outcome & rel[l-factMap.getMinObjective()];
			if (!(ithval == bddfalse)) {     
			    cout << "solution-quality = " << l << endl;
			    cout << "flushing solution " << endl;
			    solutionPrintBFS(solutionBdd, backwardBdd, 
				    bdd_relprod(ithval,bddtrue,prefVariables), 
					  l,globalTimer);
			    cout << "done " << endl;
			    upperbound = l-1;
			    if (upperbound <= lowerbound) exit(1);
			    exit(1);
			}
		    }
		}
	    }
	    else {
		solutionPrintBFS(solutionBdd, backwardBdd, intersection, -1,globalTimer);
		exit(1);
	    }
	}
  } while(1);
}


