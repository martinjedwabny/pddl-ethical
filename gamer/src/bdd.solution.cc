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

void BddBfs::solutionPrintZero(vector<vector<bdd> > &forward, 
			       vector<bdd> &backward,
			       bdd meet, int quality, Timer &globalTimer) {
  int soldepth = 0;
  meet = bdd_satone(meet);
  bdd sol, sol2;
  bdd meetForward = bdd_replace(meet, effPre);
  bdd meetBackward = bdd_replace(meet, preEff);
  list<bdd> states;


  for(int i = 0; i < forward.size(); i++) {
      for (int j=0; j < forward[i].size(); j++)
	  forward[i][j] = bdd_exist(forward[i][j],preMeritVariables);
  }

  int maxdur = 1 << (options.maxLength()) - 1;

  meet = bdd_exist(meet,preMeritVariables);

  for(int i = 0; i < backward.size(); i++) 
      backward[i] = bdd_exist(backward[i],preMeritVariables);

  int i = 0;
  int j = 0; // forward.size() - 1;

  if(options.verbose(Options::SEARCHING))
      cout << "Solution reconstruction - zero" << endl;

  while ((forward[j].size() == 0) ||
	 (forward[j].back() & meetForward) == bddfalse) {
      j++;
      //if (j == maxdur)
      //::error("goal not found");
  } 

  states.push_front(meetForward); soldepth++;

  int k=0;
//  cout << k << "," << flush;
  while ((forward[j][k] & meetForward) == bddfalse) {
      k++;
//      cout << k << "," << flush;
  }  
//  cout << ", now downchaining k: " << endl;
  while (k>0) {
      k--;
//      cout << k << "," << flush;
      sol = solutionStep(meetBackward, forward[j][k], false, 0);
      if (!(sol == bddfalse)) { 
	  states.push_front(sol); soldepth++;
      } 
      else ::error("empty solution at level " + ::toString(k));
  }
//  cout << endl;


  while (i < j) {

      if(options.verbose(Options::SEARCHING))
	  cout << " i=" << i << " j=" << j 
	       << " d=" << j-i << ", soldepth=" << soldepth << endl;
      
      // if (!(forward[j-1] == bddfalse)) { 
      
      sol = solutionStep(meetBackward, forward[i].back(), false, j-i); 
	  
      if (!(sol == bddfalse)) { 
	  if(options.verbose(Options::SEARCHING))
	      cout << "Intersection with d= " << j-i << " found"  
		   << " i=" << i << " j=" << j 
		   << " start upchaining hull of " 
		   << forward[i].size() << " elements, k=" << endl;
	  
	  int k=0;
//	  cout << k << "," << flush;
	  bdd meetPre = bdd_replace(meetBackward, effPre);

	  while ((forward[i][k] & meetPre) == bddfalse) {
	      k++;
//	      cout << k << "," << flush;
	  }
	  states.push_front(meetPre); soldepth++;
//	  cout << ", now downchaining k: " << endl;


	  while (k>0) {
	      k--;
//	      cout << k << "," << flush;
	      sol = solutionStep(meetBackward, forward[i][k], false, 0);
	      if (!(sol == bddfalse)) { 
		  states.push_front(sol); soldepth++;
	      } 
	      else ::error("empty solution at level " + ::toString(k));
	  }
	  // forward[i].clear();
	  j=i; i=-1; 
      }
      if ((sol & init) == bddfalse) 
	  i++;
      else { 
	  if(options.verbose(Options::SEARCHING))
	      cout << "initial state reached:" << soldepth << "=" 
		   << states.size() << endl;
	  break;
      }
  }
  if (quality == -1)
      outputStates(states,soldepth,globalTimer);
  else 
      outputStates(states,quality,globalTimer);
//  cout << " after outputting solution"  << endl;
}


void BddBfs::solutionPrintBFS(vector<bdd> &forward, vector<bdd> &backward,
			      bdd meet, int quality, Timer &globalTimer) {
  int soldepth = 0;
  meet = bdd_satone(meet);
  bdd sol, sol2;
  bdd meetForward = bdd_replace(meet, effPre);
  bdd meetBackward = bdd_replace(meet, preEff);
  list<bdd> states;

  meet = bdd_exist(meet,preMeritVariables);
  for(int i = 0; i < forward.size(); i++) 
    forward[i] = bdd_exist(forward[i],preMeritVariables);
  for(int i = 0; i < backward.size(); i++) 
    backward[i] = bdd_exist(backward[i],preMeritVariables);

  states.push_front(meetForward);
  int i = 0;
  int j = 0; // forward.size() - 1;

  if(options.verbose(Options::SEARCHING))
      cout << "Solution reconstruction - bfs" << endl;

//  while ((forward[j] & meetBackward) == bddfalse) {
  while ((forward[j] & meetForward) == bddfalse) {
      j++;
  } 
  if(options.verbose(Options::SEARCHING))
      cout << " i=" << i << " j=" << j << "*" << endl;
  while (i < j) {
      if(options.verbose(Options::SEARCHING))
	  cout << " i=" << i << " j=" << j 
	       << " d=" << j-i << " soldepth=" << soldepth<< endl;
      
      // if (!(forward[j-1] == bddfalse)) { 
      
	  sol = solutionStep(meetBackward, forward[i], false, j-i); 
	  
	  if (!(sol == bddfalse)) { 
	      if(options.verbose(Options::SEARCHING))
		  cout << "Intersection found" << endl;
	      states.push_front(sol); soldepth++; j=i; i=-1; 
	  }

      if ((sol & init) == bddfalse) 
	  i++;
      else { 
	  if(options.verbose(Options::SEARCHING))
	      cout << "initial state reached:" << soldepth << endl;
	  break;
      }
  }
  i = 0;
  j = backward.size() - 1;
  while ((backward[j] & meetBackward) == bddfalse) {
      j--;
  } 

  while (i < j) {
      // cout << " i=" << i << " j=" << j << " soldepth=" << soldepth<< endl;
      //    cout << soldepth << endl;
    sol = solutionStep(meetForward, backward[i], true, j-i);
    if (sol != bddfalse) { states.push_back(sol); soldepth++; j=i; i=0; }
    if ((sol & bdd_replace(goal,effPre)) == bddfalse) 
      i++;
    else { 
      cout << "goal state reached:" << soldepth << endl;
      break;
    }
  }


//  exit(1);
  if (quality == -1)
      outputStates(states,soldepth,globalTimer);
  else 
      outputStates(states,quality,globalTimer);
}

bdd BddBfs::backwardStep(bdd current, bool forward, int d) {  
  topi.clear();
  for (int o=0;o<transi[d].size();o++) {
    topi.push_back(bddfalse);
    topi.push_back(bddfalse);
  }
  for (int o=0;o<transi[d].size();o++) {
    topi[o]=bdd_relprod(current,transi[d][o],
			forward ? preVariables : effVariables);
  }
  //    cout << t.stop() << "/" << flush;
  int o = transi[d].size();
  int i=0;
  while(o>0) { o=o>>1; i++; } 
  for (int j=0;j<i;j++) {
    for (int o=0;o<transi[d].size();o+=(2<<j)) {
      // cout << "combining " << o << " with "  << o+(1<<j);
      topi[o]= topi[o] | topi[o+(1<<j)];
      topi[o+(1<<j)] = bddfalse;

    }
  }

  bdd to = topi[0];

  if (forward) {
    to = bdd_replace(to, effPre);
    return to;
  } else {
    to = bdd_replace(to, preEff);
    return  to;
  }


}

void BddBfs::outputStates(list<bdd>& states, int quality, Timer& globalTimer) {
  ofstream plan("mips-solution");
  ofstream splan("simple-solution");

//  plan << ";; Searching Time: " << globalTimer.total() << endl;


  if (options.strategy() == Options::DETERMINISTIC ||
      options.strategy() == Options::DIJKSTRA ||
      options.strategy() == Options::BDDA) {

      plan << "; NrActions " 
	   << states.size()-1 << endl;
      plan << "; MakeSpan " << endl; 
      plan << "; MetricValue " << endl;
      if(!options.bucketing()) 
	  plan << "; PlanningTechnique symbolic single-source search " 
	       << endl; 
      else 
	  plan << "; PlanningTechnique symbolic heuristic A* search " 
	       << endl; 

      splan << "; NrActions " 
	    << (states.size()-1) / (factMap.numAutomata()+1) << endl;
      splan << "; MakeSpan " << endl; 
      splan << "; MetricValue " << endl; 
      if(!options.bucketing()) 
	  splan << "; PlanningTechnique symbolic shortest path search " 
		<< endl;
      else 
	  splan << "; PlanningTechnique symbolic heuristic A* search " 
		<< endl;  
  }
  else {
      plan << "; NrActions " << endl; 
      plan << "; MakeSpan " << endl; 
      plan << "; MetricValue " << quality << endl;

      if(!options.bucketing()) 
	  plan << "; PlanningTechnique symbolic shortest path search " 
		<< endl;
      else 
	  plan << "; PlanningTechnique symbolic heuristic A* search " 
	       << endl;  

      splan << "; NrActions " << endl; 
      splan << "; MakeSpan " << endl; 
      splan << "; MetricValue " 
	    << quality << endl;
      if(!options.bucketing()) 
	  splan << "; PlanningTechnique symbolic branch-and-bound net-benefit search " 
		<< endl;
      else 
	  splan << "; PlanningTechnique symbolic heuristic A* search " 
	       << endl;  
  }
  plan << endl;
  splan << endl;

  int i=0;
  int j=0;

//  cout << "Total Time "<< globalTimer.stop().toString() << endl; 

//  cout << "Stack Size "<< states.size() << endl; 

  list<bdd>::iterator  pos = states.begin(),oldPos = pos,end = states.end();
  while(++pos != end) { 
//      bdd_printtable(*oldPos);
      if (factMap.lookup(*oldPos, *pos).substr(0,4) != "sync" &&
	  factMap.lookup(*oldPos, *pos).substr(0,4) != "skip"
	  ) {

	  int duration;
//	  if (factMap.lookupOperator(*oldPos, *pos)->durative)
	      duration = factMap.lookupOperator(*oldPos, *pos)->duration;
//	  else duration = 1;

	  splan << ""<< j <<": " 
					<< "(" << factMap.lookup(*oldPos, *pos) << ")" << endl;

	  j += 1;
      }

      
      int duration;
//	  if (factMap.lookupOperator(*oldPos, *pos)->durative)
      duration = factMap.lookupOperator(*oldPos, *pos)->duration;
//	  else duration = 1;
      

      
      plan << "" << i <<": " 
	   <<"(" << factMap.lookup(*oldPos, *pos) << ") [" 
	   << duration
	   << "]" << endl;
	  i += duration;

      oldPos++;
  }
  plan.close();
  plan.close();
  // exit(1);
}

void BddBfs::solutionPrintBnB(vector<bdd> &forward, vector<bdd> &backward,
			      bdd meet, int quality, Timer &globalTimer) {

  int soldepth = 0;
  meet = bdd_satone(meet);
  bdd sol, sol2;
  bdd meetForward = bdd_replace(meet, effPre);
  bdd meetBackward = bdd_replace(meet, preEff);
  list<bdd> states;

  meet = bdd_exist(meet,preMeritVariables);
  for(int i = 0; i < forward.size(); i++) 
    forward[i] = bdd_exist(forward[i],preMeritVariables);
  for(int i = 0; i < backward.size(); i++) 
    backward[i] = bdd_exist(backward[i],preMeritVariables);

  states.push_front(meetForward);
  int i = 0;
  int j = forward.size() - 1;
  

  while ((forward[j] & meetBackward) == bddfalse) {
      j--;
  } 

  while (i < j) {
      if(options.verbose(Options::SEARCHING))
	  cout << " i=" << i << " j=" << j << " soldepth=" << soldepth<< endl;
      

      if (!(forward[j-1] == bddfalse)) { // eliminated pred
	  
	  sol = solutionStep(meetBackward, forward[i], false, 1); 
	  
	  if (!(sol == bddfalse)) { 
	      cout << "intersection found !!! " << endl;
	      exit(1);
	      states.push_front(sol); soldepth++; j--; i=j-2; 
	  }
      }

/*

      else {
	  cout << "Decompressing automaton-reduced frontier list" << endl;

	  int steps = 0;
	  while (forward[j-1] == bddfalse) {
	      meetBackward = backwardStep(meetBackward, false, 1);
	      states.push_front(bdd_replace(meetBackward, effPre)); 
	      soldepth++; j--; steps++;
	  }

	  sol = solutionStep(meetBackward, forward[j-1], false, 1 ); 
	  bdd forward = sol;
	  int k=0;
	  for(list<bdd>::iterator pos = 
		  states.begin(); pos != states.end(); ++pos) {
	      k++;
	      if (k==steps) break;
	      sol2 = solutionStep(forward, bdd_replace(*pos,preEff), true, 1); 
	      *pos = bdd_replace(sol2,effPre);
//	      bdd_printtable(*pos);
	  }
	  
	  if (sol != bddfalse) { 
	      cout << "INSERT" << endl;
	      states.push_front(sol); soldepth++; j--; i=j-2; 

	  }
      }

*/

    if ((sol & init) == bddfalse) 
      i++;
    else { 
	if(options.verbose(Options::SEARCHING))
	    cout << "initial state reached:" << soldepth << endl;
      break;
    }
  }
  i = 0;
  j = backward.size() - 1;
  while (i < j) {
      if(options.verbose(Options::SEARCHING))
	  cout << " i=" << i << " j=" << j << " soldepth=" << soldepth<< endl;
    sol = solutionStep(meetForward, backward[i], true, 2);
    if (sol != bddfalse) { states.push_back(sol); soldepth++; j--; i=j-2; }
    if ((sol & bdd_replace(goal,effPre)) == bddfalse) 
      i++;
    else { 
      if(options.verbose(Options::SEARCHING))
	  cout << "goal state reached:" << soldepth << endl;
      break;
    }
  }


//  exit(1);
  if (quality == -1)
      outputStates(states,soldepth,globalTimer);
  else 
      outputStates(states,quality,globalTimer);
}



bdd BddBfs::solutionStep(bdd &current, bdd next, bool forward, int d) {  

  for (int o=0;o<transi[d].size();o++) {
    topi[o]=bdd_relprod(current,transi[d][o],
			forward ? preVariables : effVariables);
  }
  //    cout << t.stop() << "/" << flush;
  int o = transi[d].size();
  int i=0;
  while(o>0) { o=o>>1; i++; } 
  for (int j=0;j<i;j++) {
    for (int o=0;o<transi[d].size();o+=(2<<j)) {
      // cout << "combining " << o << " with "  << o+(1<<j);
      topi[o]= topi[o] | topi[o+(1<<j)];
      topi[o+(1<<j)] = bddfalse;
    }
  }

  bdd to = topi[0];
  
  for (int o=0;o<transi[d].size();o++) {
      topi[2*o] = bddfalse;
      topi[2*o+1] = bddfalse;
  }

// end of image backward
  
//  cout << bdd_nodecount(next) << "§" << bdd_nodecount(to) << "§" << endl;
//  bdd_printtable(next);
//  bdd_printtable(to);

  bdd newCurr = next & to;
  // cout << bdd_nodecount(newCurr) << "§§" << endl;
  //    bdd_relprod(current, trans, forward ? preVariables : effVariables);
  newCurr = bdd_satone(newCurr); // select one
  if (newCurr == bddfalse) {
//      cout << "no match" << endl;
      return newCurr;
  }
  current = newCurr;

  if (forward) {
    current = bdd_replace(current, effPre);
    bdd state = current;
    return state;
  } else {
    bdd state = current;
    current = bdd_replace(current, preEff);
    return  state;
  }
}
