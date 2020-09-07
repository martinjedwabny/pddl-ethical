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


int BddBfs::BranchandBound(Timer& globalTimer, State*& finalState) {
    if(!options.competition())
	cout << "  applying symbolic breadth-first branch and bound search ... " 
	     << endl;
    Timer t;

    if (index == -1) 
	return 
	    BFS(globalTimer,finalState);

    int objectiveIndex[1];
    objectiveIndex[0] = index;

    bdd objectiveVariables = fdd_makeset(objectiveIndex,1);

    init = factMap.getInitialBdd();
    goal = factMap.getGoalBdd();

    bdd forwardReached = init;
    bdd backwardReached = goal;
    int forwardTime = 0;
    int backwardTime = 0;
    int maxdur = 1 << (options.maxLength()) - 1;
    vector<bdd> forwardBdd(maxdur,bddfalse);
    vector<bdd> backwardBdd(1,bddfalse);

    vector<vector<bdd> > reconstructionBdd;
    vector<bdd> dummy;
    for (int i=0;i<maxdur;i++) reconstructionBdd.push_back(dummy);

    negatedVariables = bdd_exist(init,normalVariables);
    negatedVariables = bdd_replace(negatedVariables,preEff);
    
    map<string,bdd> bddTable = factMap.getBddTable();
    bdd prefVariables = factMap.getPrefVariables();

    init = bdd_relprod(init,bddtrue,prefVariables);

    int findex = 0;
    forwardBdd[0] = init;
//    reconstructionBdd[0].push_back(init);

//    forwardBdd.push_back(init);
    backwardBdd.push_back(goal);
    
    int iteration = 0;
    bdd intersection = bdd_replace(goal,effPre) & init;
    bool forward;
    
//    bdd pref = bdd_exist(intersection,prefVariables);
    bdd outcome = intersection;
    for(map<string,bdd>::iterator pos = 
	    bddTable.begin(); pos != bddTable.end(); ++pos) {
	if(options.verbose(Options::SEARCHING))
	    cout << "evaluating preference " << pos->first << endl;
	outcome = outcome & ((bdd &) pos->second);
   }

    int upperbound = factMap.getMaxObjective()+1;
    int lowerbound = factMap.getMinObjective();

    int initiallowerbound = lowerbound;
    int initialupperbound = upperbound;

    int nextcut = upperbound + maxdur;
    int oldcut = nextcut+1;

    for (int l = lowerbound;l<upperbound;l++) {
	bdd irel = bdd_relprod(objective,fdd_ithvar(index,l),
			       objectiveVariables);
	rel.push_back(irel);
    }

    isin = bddfalse;
    if(options.verbose(Options::SEARCHING))
	cout << "...restricting solution interval to [" 
	     << lowerbound << "," << upperbound-1 << "]" << endl;
    for (int l=lowerbound;l<upperbound;l++)
	isin |= rel[l];
    
    while (1) { 

	for (int k=findex-1;k>=0;k--)  // duplicate detection
	    forwardBdd[findex] = forwardBdd[findex] & !forwardBdd[k];
	    
	if(options.verbose(Options::SEARCHING))
	    cout << "findex = " << findex << " iter = " << iteration << endl;
	bdd forwardFrontier = forwardBdd[findex];
	bdd backwardFrontier = backwardBdd.back();


	do {
	    forwardFrontier = forwardBdd[findex];
            while (findex >= reconstructionBdd.size()) {
              int reconstructionSize = reconstructionBdd.size();
              for (int i = 0; i < reconstructionSize; i++)
                reconstructionBdd.push_back(dummy);
            }
	    reconstructionBdd[findex].push_back(forwardFrontier);

	    if(options.verbose(Options::SEARCHING))
		cout << " hull " << findex << " forward " << flush;
	    
	    iteration++;
	    if (searchStep0(forwardFrontier, backwardFrontier,
			   forwardReached, preVariables,
			   effPre, forwardBdd,
			   intersection, t, forwardTime, findex)) {
		
		cout << " --- complete exploration " << endl;
		exit (0);
	    }

	} while (! (forwardFrontier == forwardBdd[findex]));

	forward = 1;	

	if (lowerbound + findex > nextcut) {
	    cout << " --- lower + f > nextcut " << endl;
	    exit(0);
	}
	if (upperbound == lowerbound) {
	    cout << " --- upperbound == lowerbound " << endl;
	    exit(0);
	}
	if (forwardFrontier == bddfalse) {
	    int k = findex-maxduration;
	    if (k>=0) {
		while (k < findex && forwardBdd[k] == bddfalse) k++;
		if (k == findex) {
		    cout << " --- empty frontier " << endl;
		    exit(0);
		}
	    }
	    findex++; 
	    continue;
	}
    
	intersection = bdd_replace(backwardFrontier,effPre) & 
	    forwardFrontier;
	
	outcome = intersection; // pref;
	
	for(map<string,bdd>::iterator pos = 
		bddTable.begin(); pos != bddTable.end(); ++pos) {
//	    cout << "..." << (string &) (pos->first) << "  :  ";
		outcome = outcome & ((bdd &) pos->second);
	}
	
	outcome = outcome & isin;
	
	if (!(outcome == bddfalse)) {

	    if(options.verbose(Options::SEARCHING))
		cout << " best goal expanded in depth " << findex << endl
		     << "...now evaluating " << endl;
	    

	    int l=lowerbound;

	    if(options.verbose(Options::SEARCHING))
		cout << " ... now constructing the solution "  << endl;

				
	    while ((bdd_relprod(outcome,rel[l],preVariables) == bddfalse) && 
		   (l+findex < nextcut)) 
		l++;
	    
	    if(options.verbose(Options::SEARCHING))
		cout << "...found next cut= " << l + findex
		     << " lower bound=" << lowerbound
		     << " upper bound=" << upperbound << endl;


	    if (l + findex < nextcut) {
		nextcut = l + findex;
		if(options.verbose(Options::SEARCHING))
		    cout << " next cut improved " << l << "+" << findex 
			 << "=" << nextcut << endl;

		if (nextcut - findex < upperbound) {
		    upperbound = nextcut - findex;
		    if(options.verbose(Options::SEARCHING))
			cout << " next upperbound improved " << upperbound << endl; 
		}

		if (nextcut > oldcut) {
		    if(options.verbose(Options::SEARCHING))
			cout << ": " << globalTimer.stop() << endl;
		    exit(0);
		}
		oldcut = nextcut;

		isin = bddfalse;
		if(options.verbose(Options::SEARCHING))
		    cout << "...restricting solution interval to [" 
			 << lowerbound << "," << upperbound-1 << "]" << endl;
		for (int p=lowerbound;p<upperbound;p++)
		    isin |= rel[p];

		bdd meet = bdd_relprod(outcome,rel[l],prefVariables);

		cout << endl << "Flushing solution in file <mips-solution> "
		     << "solution-quality = " << l + findex << endl;

		solutionPrintZero(reconstructionBdd, backwardBdd, 	    
				  meet,findex, globalTimer);

	    }
	}

	
	if(options.verbose(Options::SEARCHING))
	    cout << " Depth " << findex << " forward " << flush;
	else if(!options.silent(Options::SEARCHING))
	    cout << '.' << flush;
	if(options.verbose(Options::SEARCHING))
	cout << "... now computing the image "  << endl;
	
	if (searchStep(forwardFrontier, backwardFrontier,
		       forwardReached, preVariables,
		       effPre, forwardBdd,
			   intersection, t, forwardTime, findex)) {
	    
	    cout << " --- complete exploration " << endl;
	    exit (0);
	}
	findex++;
    }
}

