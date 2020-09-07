
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

BddBfs::BddBfs(FactMap& fMap): Search(fMap),  
			       within(factMap.getWithin()),
			       prefWithin(factMap.getPrefWithin()),
			       hold(factMap.getHold()),
			       prefHold (factMap.getPrefHold()) {
  preVariables = factMap.getPreVariables();
  effVariables = factMap.getEffVariables();
  actVariables = factMap.getActVariables();
  preEff = factMap.getPreEff();
  effPre = factMap.getEffPre();
  
  objective = factMap.getObjective();
  index = factMap.getIndex();

  if(!options.silent(Options::TRANSITION))
    cout << "Reading arithmetic relations..." << endl;
  Arithmetics* arithmetics;
  if (options.strategy() == Options::PATTERNCONSTRUCT) {
      preOrig = factMap.getPreOrig();
      effOrig = factMap.getEffOrig();
      origPre = factMap.getOrigPre();
      origEff = factMap.getOrigEff();
  }
  else {
      arithmetics = factMap.getArithmetics();
  }    
 
  if (options.strategy() == Options::BRANCHANDBOUND ||
      options.strategy() == Options::BDDA ||
      options.strategy() == Options::PUREBDDA) {

    allVariables = arithmetics->getAllVars(); 
    preHeurIndex = arithmetics->getPreHeurIndex();
    effHeurIndex = arithmetics->getEffHeurIndex();
    preWeightIndex = arithmetics->getPreWeightIndex();
    preMeritIndex = arithmetics->getPreMeritIndex();
    effMeritIndex = arithmetics->getEffMeritIndex();  

    preHeurVariables = arithmetics->getPreHeurVar();
    effHeurVariables = arithmetics->getEffHeurVar();
    preWeightVariables = arithmetics->getPreWeightVar();
    preMeritVariables = arithmetics->getPreMeritVar();
    effMeritVariables = arithmetics->getEffMeritVar();
  }
  buildTransition();

}


void BddBfs::buildTransition() {
  Timer t;
  vector<Operator*> operators = factMap.getOperators();
  if(!options.silent(Options::TRANSITION))
    cout << "Building transition relations..." << endl;
  bdd frame[factMap.groupFullCount()];
  trans = bddfalse;

  int transCount = 0;
  int maxdur = (1 << options.maxLength()) - 1;

  for (int j=0;j<maxdur;j++) {
      vector<bdd> transition;
      transi.push_back(transition);
  }


  for (int o=0; o < operators.size(); o++) {
      transCount++;
  }



  int transNumber = transCount; // / options.transitions();

  if(options.verbose(Options::TRANSITION)) {
    cout << " Number of operators  = " << operators.size() << endl;
    cout << " Number of ordinary actions = " << transCount << endl;
    //  cout << " Number of requested splits = " << options.transitions() << endl;
    cout << " Number of ordinary action subrelations = " << transNumber << endl;
  }
  transCount = 0;

  if(!options.silent(Options::TRANSITION))
    cout << "Construction phase, building BDD for each action..." << endl;

  for (int o=0; o < operators.size(); o++) {

	      bool ismin = true;
	      for (int oj	=0; oj < operators.size(); oj++) {
		      if (operators[oj]->equals(*operators[o]) && 
		          operators[oj]->duration < operators[o]->duration) {
            //cout << " found match " << endl;
            //cout << operators[o]->toString() << endl;      
            //cout << operators[oj]->toString() << endl;      
            ismin = false;
		      }
	      }
	      if (ismin == false)
					continue;

      //  cout << operators[o]->toString() << endl;      
      for(int i = 0; i < factMap.groupFullCount(); i++)
	  frame[i] = bddfalse;
      if(!options.silent(Options::TRANSITION))
	  cout << "." << flush;
      operators[o]->buildBdd(frame);
  }



  if(!options.silent(Options::TRANSITION))
    cout << "Construction phase, combining BDDs..." << endl;

  for (int o=0; o < operators.size(); o++) {
      bdd currBdd = operators[o]->getBdd();
      if (options.strategy() == Options::DETERMINISTIC ||
	  options.strategy() == Options::DIJKSTRA ||
	  options.strategy() == Options::PATTERNCONSTRUCT ||
	  options.strategy() == Options::BRANCHANDBOUND ||
	  options.strategy() == Options::PUREBDDA ||
	  options.strategy() == Options::BDDA)
	  currBdd = bdd_exist(currBdd,actVariables);
      
      trans |= currBdd;
      transCount++;
      int duration;
      
      if (1) { // transCount % transNumber == transNumber - 1) {

//	  cout << operators[o]->toString() << endl;
//	  cout << operators[o]->durative << endl;
//	  cout << operators[o]->duration << endl;

//	  if (operators[o]->durative) {
	      if (options.strategy() == Options::DETERMINISTIC) 
		  duration = 1;
	      else
		  duration = operators[o]->duration;
//	  }
//	  else 
//              duration = 1;

	  //if (operators[o]->mark < 0) 
	  //    flopi.push_back(trans);
	  //else {

	      bool ismin = true;
	      for (int oj	=0; oj < operators.size(); oj++) {
		      if (operators[oj]->equals(*operators[o]) && 
		          operators[oj]->duration < operators[o]->duration) {
            //cout << " found match " << endl;
            //cout << operators[o]->toString() << endl;      
            //cout << operators[oj]->toString() << endl;      
            ismin = false;
		      }
	      }
	      if (ismin) 
         	transi[duration].push_back(trans);
	      //} 	  
	  trans = bddfalse;

      }

      // trans |= currBdd;
    }
  if (! (trans == bddfalse)) {
      cout << " last transition not empty, should not happen " << endl;
      exit(1);
      // transi[1].push_back(trans);
  }
  
  applicable = bdd_exist(trans,effVariables);

   
  for (int o=0;o<maxdur;o++) {
    topi.push_back(bddfalse);
    topi.push_back(bddfalse);
  }

  for (int o=0;o<maxdur;o++)
      if (transi[o].size() >0) {
	  maxduration = o;
      }

  if(options.verbose(Options::TRANSITION)) {
    cout << endl << "Transition relation sizes: ";

      for (int o=0;o<maxdur;o++)
	  if (transi[o].size() >0) {
	      cout << o << ":" << transi[o].size() << ",";
	  }
/*
    for (int j=0; j< transi.size();j++) {
	for (int i=0; i< transi[j].size();i++)
	    cout << "(" << j << "," << bdd_nodecount(transi[j][i]) << "),";
    }
    cout << endl << "Zero-cost transition relation sizes: ";
    for (int i=0; i< flopi.size();i++)
      cout << bdd_nodecount(flopi[i]) << " ";
*/
    cout << endl;
  }

  // ------------------------------------------------------


}

bdd BddBfs::computePreImage(bdd& states) {
  if (options.strategy() == Options::STRONG)
    return bdd_appall(trans,states,bddop_imp,effVariables) & applicable;
  if (options.strategy() == Options::WEAK)
    return bdd_relprod(trans,states,effVariables);
}

int BddBfs::search(Timer& globalTimer, State*& finalState) {
  if (options.strategy() == Options::DETERMINISTIC)
    return BFS(globalTimer,finalState);
  if (options.strategy() == Options::DIJKSTRA)
    return Dijkstra(globalTimer,finalState);
  if (options.strategy() == Options::BRANCHANDBOUND)
    return BranchandBound(globalTimer,finalState);
  ::error("Strategy " + ::toString(options.strategy()) 
	  + " wrong for bdd exploration");
}


bool BddBfs::searchStep(bdd from, bdd& back, bdd &reach, bdd& varset,
			bddPair *rename, vector<bdd> &bddVec,
			bdd &meet, Timer &t, int &time, int index) {
  Timer step; 


  // partitioning  

  int maxdur = (1 << options.maxLength()) - 1;

  for (int d=1;d<maxdur;d++) {

      if (transi[d].size() == 0)
	  continue;

      for (int o=0;o<transi[d].size();o++) {
//      cout << "Building " << o << " relprod" << endl;
	  topi[o]=bdd_relprod(transi[d][o],from,varset);

/*

// flush option for large images
      char filename[80];
      sprintf(filename,"topi-%d",o);
      FILE* savefile = fopen(filename,"w"); 
      int sval = bdd_save(savefile,topi[o]);
      topi[o] = bddfalse;
      if (sval != 0) exit(1);
      fclose(savefile);

*/
  }

//  exit(1);



//      cout << endl << d << ":" << t.stop() << "/" << flush;
      int o = transi[d].size();
      
      int i=0;
      while(o>0) { o=o>>1; i++; } 
      for (int j=0;j<i;j++) {
	  for (int o=0;o<transi[d].size();o+=(2<<j)) {
//	cout << "combining " << o << " with "  << o+(1<<j) << endl;
	      topi[o]= topi[o] | topi[o+(1<<j)];
	      topi[o+(1<<j)] = bddfalse;
	  }
      }
  


      bdd to = topi[0];
      
      for (int o=0;o<transi[d].size();o++) {
	  topi[2*o] = bddfalse;
	  topi[2*o+1] = bddfalse;
      }

      // bdd to = bdd_relprod(trans,from,varset); 
      if(options.verbose(Options::SEARCHING))
	  cout << t.stop() << "; goal" << flush;
      meet = to & back;

      if(options.verbose(Options::SEARCHING))
	  cout << ": " << t.stop() << "; replace" << flush; 
      to = bdd_replace(to, rename);
      
      if(options.verbose(Options::SEARCHING))
	  cout << ": " << t.stop() << "; reach" << flush; 
      

      
      
      if (!(meet == bddfalse)) {
	  if(options.verbose(Options::SEARCHING))
	      cout << endl << "Intersection found with edge weight "  << d
		   << ", inserting into bucket " << index+d << endl;
      }
      bdd newbdd;
      bdd oldreach;
      if (1) 
	  // if (options.strategy() != Options::PATTERNCONSTRUCT) 
	  // !options.bucketing())
	  newbdd = to;
      else {
	  oldreach = reach;
	  newbdd = to & !reach;
	  reach = reach | newbdd;
      }

      // if (options.strategy() == Options::PATTERNCONSTRUCT) 
      if (0) //  (options.bucketing()) { 
      { 
	  if (reach == oldreach) {
	      cout << endl << "Exploration completed" << endl;
	      return true;
	  }
      }
      
      while (index + d >= bddVec.size()) {
        int vecSize = bddVec.size();
        for (int i =0; i < vecSize;i++)
          bddVec.push_back(bddfalse);
      }
      bddVec[index+d] |= newbdd; 
      time = step.stop().seconds;

      if(options.verbose(Options::SEARCHING))
	  cout << ": " << t.stop()
	       << "; " << bdd_nodecount(newbdd) << endl;      
  }
      

  return false;
}


bool BddBfs::searchStep0(bdd from, bdd& back, bdd &reach, bdd& varset,
			bddPair *rename, vector<bdd> &bddVec,
			bdd &meet, Timer &t, int &time, int index) {
  Timer step; 


  // partitioning  

  int maxdur = (1 << options.maxLength()) - 1;

  for (int d=0;d<1;d++) {

      if (transi[d].size() == 0)
	  continue;

      for (int o=0;o<transi[d].size();o++) {
//      cout << "Building " << o << " relprod" << endl;
	  topi[o]=bdd_relprod(transi[d][o],from,varset);

/*

// flush option for large images
      char filename[80];
      sprintf(filename,"topi-%d",o);
      FILE* savefile = fopen(filename,"w"); 
      int sval = bdd_save(savefile,topi[o]);
      topi[o] = bddfalse;
      if (sval != 0) exit(1);
      fclose(savefile);

*/
  }

//  exit(1);



//      cout << endl << d << ":" << t.stop() << "/" << flush;
      int o = transi[d].size();
      
      int i=0;
      while(o>0) { o=o>>1; i++; } 
      for (int j=0;j<i;j++) {
	  for (int o=0;o<transi[d].size();o+=(2<<j)) {
//	cout << "combining " << o << " with "  << o+(1<<j) << endl;
	      topi[o]= topi[o] | topi[o+(1<<j)];
	      topi[o+(1<<j)] = bddfalse;
	  }
      }
  


      bdd to = topi[0];
      
      for (int o=0;o<transi[d].size();o++) {
	  topi[2*o] = bddfalse;
	  topi[2*o+1] = bddfalse;
      }

      // bdd to = bdd_relprod(trans,from,varset); 
      if(options.verbose(Options::SEARCHING))
	  cout << t.stop() << "; goal" << flush;
      meet = to & back;

      if(options.verbose(Options::SEARCHING))
	  cout << ": " << t.stop() << "; replace" << flush; 
      to = bdd_replace(to, rename);
      
      if(options.verbose(Options::SEARCHING))
	  cout << ": " << t.stop() << "; reach" << flush; 
      

      
      
      if (!(meet == bddfalse)) {
	 if(options.verbose(Options::SEARCHING))
	  cout << endl << "Intersection found with edge weight "  << d
	       << ", inserting into bucket " << index+d << endl;
      }
      bdd newbdd;
      bdd oldreach;
      if (1) 
	  // if (options.strategy() != Options::PATTERNCONSTRUCT) 
	  // !options.bucketing())
	  newbdd = to;
      else {
	  oldreach = reach;
	  newbdd = to & !reach;
	  reach = reach | newbdd;
      }

      // if (options.strategy() == Options::PATTERNCONSTRUCT) 
      if (0) //  (options.bucketing()) { 
      { 
	  if (reach == oldreach) {
	      cout << endl << "Exploration completed" << endl;
	      return true;
	  }
      }
      
      while (index + d >= bddVec.size()) {
        int vecSize = bddVec.size();
        for (int i =0; i < vecSize;i++)
          bddVec.push_back(bddfalse);
      }
      bddVec[index+d] |= newbdd; 
      time = step.stop().seconds;

      if(options.verbose(Options::SEARCHING))
	  cout << ": " << t.stop()
	       << "; " << bdd_nodecount(newbdd) << endl;      
  }
      

  return false;
}


