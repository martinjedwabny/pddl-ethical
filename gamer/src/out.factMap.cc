#include <map>
#include <iostream>      
#include <fstream>      
#include <vector>
#include <stdlib.h>
#include <stdio.h>


using namespace std;

#include <data.fact.h>
#include <data.predicate.h>
#include <data.domain.h>
#include <data.action.h>
#include <data.instantiation.h>
#include <data.formula.h>

#include <util.options.h>
#include <util.tools.h>
#include <util.bitarray.h>

#include <bdd.bfs.h>

#include <out.factMap.h>
#include <out.operator.h>
#include <out.group.h>

#include <single.search.h>

#include <bdd.h>
#include <bdd.arithmetics.h>


// #define INITIAL_NODES 96000000
// #define MAX_INCREASE  19000000

// for 16 GB exploration
//
// #define INITIAL_NODES 32000000
// #define MAX_INCREASE  128000000

void theGbcHandler(int pre, bddGbcStat *s) { // avoid too much output
  static int nodeNum = options.getBddInit();
  // if(!pre && !options.competition()) {
    if(nodeNum != s->nodes) {
      cout << '[' << s->nodes << ']' << flush;
      nodeNum = s->nodes;
    } else {
      cout << '*' << flush;
    }

}


bdd* FactMap::getPreLessFactBdd(int i, int v) { 
    bdd rBdd = bddfalse;
    for (int j=0; j < v; j++)
	rBdd |= *(preIthBdd[i][j]);
    return new bdd (rBdd);
}

bdd* FactMap::getPreLessEqualFactBdd(int i, int v) { 
    bdd rBdd = bddfalse;
    for (int j=0; j <= v; j++)
	rBdd |= *preIthBdd[i][j];
    return new bdd(rBdd);
}
bdd* FactMap::getPreGreaterFactBdd(int i, int v) { 
//    cout << "pre greater fact bdd called i=" << i << " v=" << v << endl;
    bdd rBdd = bddfalse;
    for (int j=v+1; j < options.maxNumber(); j++)
	rBdd |= *preIthBdd[i][j];
    return new bdd (rBdd);
}

bdd* FactMap::getPreGreaterEqualFactBdd(int i, int v) { 
//    cout << "pre greater equal fact bdd called i=" << i << " v=" << v << endl;
    bdd rBdd = bddfalse;
    for (int j=v; j < options.maxNumber(); j++)
	rBdd |= *preIthBdd[i][j];
    return new bdd (rBdd);
}

bdd* FactMap::getPreEqualFactBdd(int i, int v) { 
//    cout << "pre equal fact bdd called i=" << i << " v=" << v << endl;
    return new bdd(*preIthBdd[i][v]); 
}

bdd* FactMap::getEffDecFactBdd(int i, int v) {
//    cout << "eff dec fact bdd called i=" << i << " v=" << v << endl;
    bdd rBdd = bddfalse;
    for (int j=v; j < options.maxNumber(); j++) {
//        cout << "from " << j << " to " << j-v << endl;
	rBdd |= ((*preIthBdd[i][j]) & (*effIthBdd[i][j-v]));
	// if (j==v) bdd_printtable(rBdd);
    }
    return new bdd(rBdd); 
}

bdd* FactMap::getEffIncFactBdd(int i, int v) { 
//    cout << "eff inc fact bdd called i=" << i << " v=" << v << endl;
    bdd rBdd = bddfalse;
    for (int j=0; j < options.maxNumber()-v; j++)
	rBdd |= ((*preIthBdd[i][j]) & (*effIthBdd[i][j+v]));
    return new bdd(rBdd);    
}

bdd* FactMap::getEffAssFactBdd(int i, int v) {
//    cout << "eff ass fact bdd called i=" << i << " v=" << v << endl;
    return new bdd(*effIthBdd[i][v]);  
}


   
bdd FactMap::getHeurVariables() { return bddArithmetics->getPreHeurVar(); }
int FactMap::getPreHeurIndex() { return bddArithmetics->getPreHeurIndex(); }
int FactMap::getEffHeurIndex() { return bddArithmetics->getEffHeurIndex(); }

void FactMap::run(Timer& t) {
  int length; 
  string output;
  Search* search;
  
  if(!options.allSilent())
    cout << endl << "Starting planner..." << endl;
  if (options.strategy() == Options::DETERMINISTIC ||
      options.strategy() == Options::DIJKSTRA ||
      options.strategy() == Options::PATTERNCONSTRUCT ||
      options.strategy() == Options::BRANCHANDBOUND ||
      options.strategy() == Options::BDDA ||
      options.strategy() == Options::PUREBDDA)
    search = new BddBfs(*this);
  
  cout << "Setup Time: " << t.total() << endl;
  
  State* searchResult = search->exploreSpace(t,length);
  if (searchResult) exit(1);
  if (options.strategy() == Options::PATTERNCONSTRUCT)
      exit(1);

  cout << endl << "Restarting planner..." << endl;
  if (options.strategy() == Options::DETERMINISTIC ||
      options.strategy() == Options::DIJKSTRA ||
      options.strategy() == Options::BRANCHANDBOUND ||
      options.strategy() == Options::BDDA ||
      options.strategy() == Options::PUREBDDA) {
      bdd_done();
      initBdds();
      search = new BddBfs(*this);
      State* searchResult = search->exploreSpace(t,length);
  }  
} 

FactMap::FactMap(int fCount, Domain& dom): 
  noFacts(fCount), domain(dom),timed(dom.isTimed()),
  maximize(dom.getMaximize()) {  

  objectiveIndex = -1;
  position = new int[noFacts];
  group = new int[noFacts];
  for(int i = 0; i < noFacts;i++) {
      group[i] = -1;
  }

  derived = new BitArray(noFacts);
  for(int i = 0; i < noFacts;i++) 
    variable_name.push_back(dom.getPredicate(i).getName()); 
  for(int i = 0; i < noFacts;i++) {
      // cout << "INITIAL" << endl;
      if (domain.isInitFact(i)) {
	  // cout << i << ".";
	  initial_state.push_back(i);
      }
      if (domain.isInitNumFact(i)) {
	  initial_numstate.push_back(i);
	  initial_numvalue.push_back(domain.initNumValue(i));
      }
  }
  // cout << endl;
  for(int i = 0; i < noFacts;i++) 
    if (domain.isGoalFact(i)) 
      goal_state.push_back(i);

}


FactMap::~FactMap() {
  bdd_done(); 
}


void FactMap::initHold() {
    vector< pair < pair <int,int>, Formula*> >HoldDuring = 
	domain.getHoldDuring(); 
    vector<pair <int,Formula*> > HoldAfter = domain.getHoldAfter();
    vector<pair <string, pair <int,Formula*> > > PrefHoldAfter =
	domain.getPrefHoldAfter();
    vector<pair <string, pair < pair <int,int>, Formula*> > > PrefHoldDuring =
	domain.getPrefHoldDuring();
    vector<pair <int,Formula*> > Within = 
	domain.getWithin();
    vector<pair <string, pair <int,Formula*> > > PrefWithin =
	domain.getPrefWithin();
    vector<pair <int,Formula* > > AlwaysWithin = 
	domain.getAlwaysWithin();
    vector<pair <string, pair <int, Formula*> > > PrefAlwaysWithin =
	domain.getPrefAlwaysWithin();

    with.reserve(1000); 
    prefwith.reserve(1000);
    hold.reserve(1000); 
    prefhold.reserve(1000);

    with.clear();
    prefwith.clear();
    hold.clear(); 
    prefhold.clear();

    for (int i=0;i<AlwaysWithin.size();i++) {
	cout << "******always within deadline " 
	     << ::toString(AlwaysWithin[i].first) << endl;
	Formula* form = (Formula*) (AlwaysWithin[i].second);
	form->makeBdd(preAddFactBdd);
	with[AlwaysWithin[i].first].push_back(form->getBinary());
    }
    for (int i=0;i<Within.size();i++) {
	cout << "*****within deadline " 
	     << ::toString(Within[i].first) << endl;
	Formula* form = (Formula*) (Within[i].second);
	form->makeBdd(preAddFactBdd);
	with[Within[i].first].push_back(form->getBinary());
    }
    for (int i=0;i<HoldDuring.size();i++) {
	cout << "*****from " << ::toString(HoldDuring[i].first.first) << endl;
	cout << "to " << ::toString(HoldDuring[i].first.second) << endl;
	Formula* form = (Formula*) (HoldDuring[i].second);
	form->makeBdd(preAddFactBdd);
	for (int j=HoldDuring[i].first.first; 
	     j<HoldDuring[i].first.second;j++) 
	    hold[j].push_back(form->getBinary());
    }
    for (int i=0;i<HoldAfter.size();i++) {
	cout << "*****from " << ::toString(HoldAfter[i].first) << endl;
	cout << "to " << ::toString(10000) << endl;
	Formula* form = (Formula*) (HoldAfter[i].second);
	form->makeBdd(preAddFactBdd);
	for (int j=HoldAfter[i].first+1; 
	     j<10000;j++) 
	    hold[j].push_back(form->getBinary());
    }

    for (int i=0;i<PrefWithin.size();i++) {
	cout << "preference name:" << PrefWithin[i].first << endl;
	cout << "*****within deadline " 
	     << ::toString(PrefWithin[i].second.first) << endl;
	Formula* form = (Formula*) (PrefWithin[i].second.second);
	form->makeBdd(preAddFactBdd);
	prefwith[PrefWithin[i].second.first].push_back(
	    make_pair(PrefWithin[i].first,form->getBinary()));
    }
    for (int i=0;i<PrefAlwaysWithin.size();i++) {
	cout << "preference name:" << PrefWithin[i].first << endl;
	cout << "*****prefalways within deadline " 
	     << ::toString(PrefAlwaysWithin[i].second.first) << endl;
	Formula* form = (Formula*) (PrefAlwaysWithin[i].second.second);
	form->makeBdd(preAddFactBdd);
	prefwith[PrefAlwaysWithin[i].second.first].push_back
	    (make_pair(PrefAlwaysWithin[i].first,form->getBinary()));
    }

    for (int i=0;i<PrefHoldDuring.size();i++) {
	cout << "preference name:" << PrefHoldDuring[i].first << endl;
	cout << "******from " << ::toString(PrefHoldDuring[i].second.first.first) << endl;
	cout << "to " << ::toString(PrefHoldDuring[i].second.first.second) << endl;
	Formula* form = (Formula*) (PrefHoldDuring[i].second.second);
	form->makeBdd(preAddFactBdd);
	for (int j=PrefHoldDuring[i].second.first.first; 
	     j<PrefHoldDuring[i].second.first.second;j++) 
	    prefhold[i].push_back(make_pair(PrefHoldDuring[i].first,
					    form->getBinary()));
    }
    for (int i=0;i<PrefHoldAfter.size();i++) {
	cout << "preference name:" << PrefHoldAfter[i].first << endl;
	cout << "******from " << ::toString(PrefHoldAfter[i].second.first) << endl;
	cout << "to " << ::toString(10000) << endl;
	Formula* form = (Formula*) (PrefHoldAfter[i].second.second);
	form->makeBdd(preAddFactBdd);
	for (int j=PrefHoldAfter[i].second.first; j<1000;j++) 
	    prefhold[i].push_back(make_pair(PrefHoldAfter[i].first,
					    form->getBinary()));
    }
    
}

void FactMap::initPreference() { 
    map<string,Formula*> preferenceTable = domain.getPreferenceTable();

//    cout << "init preference:"  << endl;

    Formula* form;
    prefVariables = bddtrue;
    for(map<string,Formula*>::iterator pos = 
	    preferenceTable.begin(); pos != preferenceTable.end(); ++pos) {
	
	
	string violatedname = "is-violated-"+(string &)(pos->first);
	string satisfiedname = "is-satisfied-"+(string &)(pos->first);

	form = (Formula*) (pos->second);

/*
	cout << (string &)(pos->first) << ":" << form->toString() << endl;
        cout << endl << "Lookup name " << endl;
*/

	Predicate* pv = domain.lookupPredicate(violatedname);
	Predicate* ps = domain.lookupPredicate(satisfiedname);

	bdd prediv = *preAddFactBdd[pv->getFactLowerBound()];
	bdd predis = *preAddFactBdd[ps->getFactLowerBound()];

//	cout << "bdds for is-violated and is-satisfied" << endl;
//	bdd_printtable(prediv); 
//	bdd_printtable(predis);

	prefVariables = prefVariables & predis; 

	if (form->toString() == "dummy") {
//	    cout << "Dummy found" << endl;
	    bddTable[(string&) pos->first] = prediv;
	}

	else {
//	cout << "make BDD" << endl;
	    form->makeBdd(preAddFactBdd);
//	cout << "BDD made " << endl;
	    
	    

	    bddTable[(string&) pos->first] = 
		(prediv | form->getBinary()) & (!prediv | !form->getBinary());
//		(predis | !form->getBinary()) & (!predis | form->getBinary());

	}
//	bdd_printtable(bddTable[(string&) pos->first]);
    }
}

int FactMap::numAutomata()  {
    int count = 0;
    for (int i=0;i<variable_name.size();i++) 
	if (variable_name[i].substr(0,5) == "sync-")
	    count++;
    if (count) count--;
    return count;
    // cout << "maxViolated" << maxViolated << endl;
    // cout << "domain.getPreferenceTable().size(); " << 
    //domain.getPreferenceTable().size() << endl;
}

void FactMap::initOptimizer() { 
  Formula* form = 
    new Formula(NULL,domain.getPredicateTable(),
		domain,*(domain.getMetric()));

//  cout << form->toString() << endl;

  if (form->toString() == "total-cost") return;

  maxViolated = form->countLeaves();
  if (options.verbose(Options::EXPLORE)) 
      cout << "Maximal is-violated constraints " << maxViolated << endl;
  maxObjective = form->maxint();
  minObjective = form->minint();
  if (options.verbose(Options::EXPLORE)) { 
      cout << "Maximal integer value " << maxObjective << endl;
      cout << "Minimal integer value " << minObjective << endl;
  }
  int sizes[1];
  sizes[0] = maxObjective - minObjective + 3;
  objectiveIndex = fdd_extdomain(sizes,1);

  bdd addBdd;
  //cout << "make addition" << endl;
  int addindex = bddArithmetics->Addition(addBdd,maxObjective-minObjective);
  //cout << "make objective" << endl;
  form->makeBdd(objectiveIndex,preAddFactBdd,addindex,addBdd,minObjective);
  //cout << "done" << endl;
  objective = form->getBinary();


//  bdd_printtable(objective);
  // exit(1);

}

string FactMap::lookup(bdd oldState, bdd newState) {
  bdd transition = oldState & bdd_replace(newState, preEff);
  
  for(int i = 0; i < operators.size(); i++)
    if((transition & operators[i]->getBdd()) != bddfalse)
      return operators[i]->toString();
}


Operator* FactMap::lookupOperator(bdd oldState, bdd newState) {
  bdd transition = oldState & bdd_replace(newState, preEff);
  
  for(int o = 0; o < operators.size(); o++) {
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
    if (!ismin)
      continue;

    if((transition & operators[o]->getBdd()) != bddfalse) {
      return operators[o];
    }
  }
}


void FactMap::initBdds() {
/*
  void FactMap::initBdds(vector<Action *> actions) {
  int count = 0;

  for (int a=0; a< actions.size(); a++) {
    vector<Instantiation>& instants =
      actions[a]->getInstantiations();
    count += instants.size();
  }
*/
  int maxlength = (1 << options.maxLength())-1; 

//  cout << "---" <<  endl;
  /* bdd_autoreorder(BDD_REORDER_SIFT);
  bdd_reorder_verbose(2);
  */

  bdd_init(options.getBddInit(), 1000000);
  if (options.strategy() == Options::PATTERNCONSTRUCT) {
      bdd_setvarnum(3*36*6);
  }
  else
      bdd_setvarnum(1000000);


  int maxnum = options.maxNumber(); 
  //  cout <<  " maxnum= " << maxnum << endl;
      
//  bdd_setvarnum(5*maxlength+5*(encodingSize+groups.size())+count); 
  bdd_setminfreenodes(5);      
  bdd_setmaxincrease(options.getBddMaxIncrease());
  bdd_gbc_hook(&theGbcHandler); 

  if (options.strategy() != Options::PATTERNCONSTRUCT) {
      bddArithmetics = new Arithmetics();
  }
  int indices[1];
  int index;

  int numvar = 0;
  for(int i=0; i<noFacts;i++) { 
      if (group[i] == -1 && variable_name[i] != "total-cost") {
	  numvar++;
      }
  }

  pre = new int[groups.size()+numvar];
  eff = new int[groups.size()+numvar];


  if (options.strategy() != Options::PATTERNCONSTRUCT) {
      indices[0] = 1; // count;  
      actIndex = fdd_extdomain(indices,1);
      pre[0] = actIndex;
      actVariables = fdd_makeset(pre,1);
  }
  else { // abstraction only on propositional domains
      orig = new int[groups.size()];
      for(int i = 0; i < groups.size() ; i++) {
	  indices[0] = groups[i]->size();
	  index = fdd_extdomain(indices,1);
	  orig[i] = index;      
      }
      origVariables = fdd_makeset(orig, groups.size());
      effOrig = bdd_newpair();  // eff->original
      preOrig = bdd_newpair();  // pre->original
      origEff = bdd_newpair();  // eff->original
      origPre = bdd_newpair();  // pre->original
      

  }

  for(int i = 0; i < groups.size() ; i++) {
    indices[0] = groups[i]->size();
    indices[1] = groups[i]->size();
    index = fdd_extdomain(indices,2);
    pre[i] = index;      
    eff[i] = index+1;
  }

  int grsize = groups.size();
//  cout << noFacts << endl;
  for(int i=0; i<noFacts;i++) { 
      if (group[i] == -1 && variable_name[i] != "total-cost") {
        //cout << "fact id " << i << " name " << variable_name[i] 
        //<< " group for it " << group[i] << endl;
        indices[0] = options.maxNumber(); // maxnum; 
        indices[1] = options.maxNumber(); // maxnum; 
	  index = fdd_extdomain(indices,2);
	  pre[grsize] = index;      
	  eff[grsize] = index+1;
	  grsize++;
      }
  }


//  cout << "===" <<  endl;

  if (options.strategy() == Options::PATTERNCONSTRUCT) {
      fdd_setpairs(effOrig, eff, orig, groups.size());
      fdd_setpairs(preOrig, pre, orig, groups.size());
      fdd_setpairs(origEff, orig, eff, groups.size());
      fdd_setpairs(origPre, orig, pre, groups.size());
  }
  preVariables = fdd_makeset(pre, groups.size()+numvar);
  effVariables = fdd_makeset(eff, groups.size()+numvar);



//  cout << "reached" << endl;
  effPre = bdd_newpair();  // eff->pre
  fdd_setpairs(effPre, eff, pre, groups.size()+numvar);
  preEff = bdd_newpair(); // pre->eff
  fdd_setpairs(preEff, pre, eff, groups.size()+numvar);

// cout << "---" <<  endl;

  bdd preAddBdd, effAddBdd, preDelBdd, effDelBdd;
  preAddFactBdd.reserve(noFacts); preAddFactBdd.clear();
  effAddFactBdd.reserve(noFacts); effAddFactBdd.clear();
  preDelFactBdd.reserve(noFacts); preDelFactBdd.clear();
  effDelFactBdd.reserve(noFacts); effDelFactBdd.clear();
  effIthBdd.reserve(noFacts); effIthBdd.clear();
  preIthBdd.reserve(noFacts); preIthBdd.clear();
  //  cout << "noFacts=" << noFacts << endl; exit(1);
  for(int i=0; i<noFacts;i++) {
      vector<bdd* > dummy;
      preIthBdd.push_back(dummy);
      effIthBdd.push_back(dummy);
  }


//  cout << "before construction" << endl;

  fullGroupCount = groups.size();
  for(int i=0; i<noFacts;i++) {
      if (group[i] != -1) {
	  int omitted = groups[group[i]]->omitted();
	  preAddBdd = fdd_ithvar(pre[group[i]],position[i]+omitted);
	  effAddBdd = fdd_ithvar(eff[group[i]],position[i]+omitted);
//	  cout << "[" << i << "]" << group[i] << "<-" << position[i]+omitted << " X ";
	  preDelBdd = !fdd_ithvar(pre[group[i]],position[i]+omitted);
	  
/*
  preDelBdd = bddfalse;
  for(int j=0; j<noFacts;j++) {
  if(group[j] == group[i] && j!=i)
  preDelBdd |= fdd_ithvar(pre[group[i]],position[j]+omitted);
  }
*/
	  effDelBdd = fdd_ithvar(eff[group[i]],0);
	  preAddFactBdd.push_back(new bdd(preAddBdd));
	  effAddFactBdd.push_back(new bdd(effAddBdd));
	  preDelFactBdd.push_back(new bdd(preDelBdd));
	  effDelFactBdd.push_back(new bdd(effDelBdd));
	  
      }
      else {
	  if (variable_name[i] != "total-cost") {

	    //cout << "processing " << variable_name[i] << "encoding interval [0,"<< options.maxNumber() <<"]" << endl;
	      group[i] = fullGroupCount++;
	      vector<bdd*> effIBdd;
	      vector<bdd*> preIBdd;
	      for(int j = 0; j < options.maxNumber(); j++) {
		  effIBdd.push_back(new bdd(fdd_ithvar(eff[group[i]],j)));
		  preIBdd.push_back(new bdd(fdd_ithvar(pre[group[i]],j)));
	      }
	      effIthBdd[i] = effIBdd;
	      preIthBdd[i] = preIBdd;
	  }
      }
  }

//  cout << noFacts << endl;
//  for(int i=0; i<noFacts;i++) 
//      cout << "fact id " << i << " name " << variable_name[i] << " group for it " << group[i] << endl;


  int visited[groups.size()];
  for (int i=0;i<groups.size();i++)
    visited[i] = 0;
 
  initialBdd = new bdd(bddtrue);
  for (int i=0;i<initial_state.size();i++) {
      bdd added = *preAddFactBdd[initial_state[i]];
      visited[group[initial_state[i]]] = 1;
      *initialBdd &= added;
  }

//  cout << "-groups-bdd-" <<  endl;

  for (int i=0;i<groups.size();i++) {
    if (! visited[i]) {
      bdd deleted =  fdd_ithvar(pre[i],0);
      // *preDelFactBdd[groups[i]->getFirst()];
      // bdd_printtable(deleted);
      *initialBdd &= deleted;
    }
    groups[i]->setBdd();
  }

  for (int i=0;i<initial_numstate.size();i++) {
      if (variable_name[initial_numstate[i]] != "total-cost") {
        //cout << " name " <<  variable_name[initial_numstate[i]] << " val =" 
        //<< initial_numvalue[i] << endl;

	  bdd added = *preIthBdd[initial_numstate[i]]
	      [initial_numvalue[i]];
	  *initialBdd &= added;
      }
  }


//  cout << "-goal-bdd-" <<  endl;

  goalBdd = new bdd(bddtrue);
  for (int i=0;i<goal_state.size();i++) {
    bdd added = *effAddFactBdd[goal_state[i]];
    *goalBdd &= added;
  }

// cout << "-fdd-" <<  endl;

  fdd = new bdd[fullGroupCount];
  for(int i = 0; i < fullGroupCount; i++)
    fdd[i] =  fdd_equals(getPreIndex(i), 
			 getEffIndex(i));

//  cout << "before optimizer" << endl;

  if (domain.getMetric()) {
    initOptimizer();
  }
//  cout << "after optimizer" << endl;
  initPreference();
//  cout << "after preference" << endl;
  initHold();
//  cout << "after hold" << endl;
  /* exit(1); */


}

void FactMap::initGroupTable(vector<vector<int> > groupsInt) {
  encodingSize = 0;

  for (int i=0;i<groupsInt.size();i++) {
    int bitsize = 0;
    int groupsize = groupsInt[i].size()-1;
    // cout << "(" << groupsize << ")";
    while (groupsize) { 
      groupsize /=2; 
      bitsize +=1; 
    }
    // cout << "+" << bitsize;
    encodingSize += bitsize;
  }


  for (int i=0;i<groupsInt.size();i++) {
    bool omitted = false;
    for (int j=0;j<groupsInt[i].size();j++) {
      if (groupsInt[i][j] == -1) {
	omitted = true;
	groupsInt[i].pop_back();
      }
      else {
	group[groupsInt[i][j]] = i;
	position[groupsInt[i][j]] = j;
      }
    }
    groups.push_back(new Group(i,groupsInt[i],omitted,*this)); 
    // cout << groups[i]->toString() << endl;
  }

  Operator* oper;
  Group *g1,*g2;

  int osize = getNoOperators();

  bool causalgraph[groups.size()][groups.size()];

  for (int i=0;i<groups.size();i++) {
      g1 = groups[i];
      for (int j=0;j<groups.size();j++) {
	  if (i==j) continue;
	  g2 = groups[j];
	  for(int o=0;o<osize;o++) {
	      oper = operators[o];

	      bool isIn = false;
	      for (int a=0; a < oper->addsize; a++) {
		  if ( group[oper->add[a]] == i ) {
		      isIn = true;
		      break;
		  }
	      }

	      for (int d=0; d < oper->delsize; d++) {
		  if ( group[oper->del[d]] == i ) {
		      isIn = true;
		      break;
		  }
	      }


	      for (int p=0; p < oper->presize; p++) {
		  if ( group[oper->pre[p]] == i ) {
		      isIn = true;
		      break;
		  }
	      }
	      
	      if (!isIn) continue;

	      isIn = false;

	      for (int a=0; a < oper->addsize; a++) {
		  if ( group[oper->add[a]] == j ) {
		      isIn = true;
		      break;
		  }
	      }
	      
	      for (int d=0; d < oper->delsize; d++) {
		  if ( group[oper->del[d]] == j ) {
		      isIn = true;
		      break;
		  }
	      }

	      if (!isIn) continue;

	      if (options.verbose(Options::EXPLORE)) 
		  cout << " found causal relationship between group " << i 
		       << " and group " << j << endl;
	      causalgraph[i][j] = true;
	      break;
	  }
      }
  }
}

void FactMap::initOperatorTable(vector<Action *> actions) {
  int c = 0;
  durative = false; 
  timed =  false; 
  vector<Operator*> whens;
  vector<pair <int,int> > pre, add, del, pdel;
  vector<pair <int,int> > numinc, numass, numdec, numle, numl, 
      numge, numg, nume;
  vector<vector<pair <int,int> > > cpre, cadd, cdel, cpdel;

  // cout << "init operator table " << endl;

  for(int a = 0; a < actions.size(); a++) {
//      cout <<  actions[a]->toString() << endl;
    durative |= actions[a]->getDurative();
    vector<Instantiation>& instants =
      actions[a]->getInstantiations();
    for(int k = 0; k < instants.size(); k++) {
      Instantiation& inst = instants[k];

      pre.clear(); pdel.clear(); add.clear(); del.clear();
      numass.clear(); numdec.clear(); numinc.clear(); 
      numle.clear(); numl.clear(); numge.clear(); numg.clear(); nume.clear();
      cpre.clear(); cpdel.clear(); cadd.clear(); cdel.clear();

      for(int i = 0; i < inst.numass.size(); i++) 
	numass.push_back(make_pair(inst.numass[i].first,inst.numass[i].second));
      for(int i = 0; i < inst.numle.size(); i++) 
	numle.push_back(make_pair(inst.numle[i].first,inst.numle[i].second));
      for(int i = 0; i < inst.numl.size(); i++) 
	numl.push_back(make_pair(inst.numl[i].first,inst.numl[i].second));  
      for(int i = 0; i < inst.numge.size(); i++) 
	numge.push_back(make_pair(inst.numge[i].first,inst.numge[i].second));
      for(int i = 0; i < inst.numg.size(); i++) 
	numg.push_back(make_pair(inst.numg[i].first,inst.numg[i].second));  
      for(int i = 0; i < inst.nume.size(); i++) 
	nume.push_back(make_pair(inst.nume[i].first,inst.nume[i].second));  
      for(int i = 0; i < inst.numdec.size(); i++) 
	numdec.push_back(make_pair(inst.numdec[i].first,inst.numdec[i].second));      
      for(int i = 0; i < inst.numinc.size(); i++) {
	numinc.push_back(make_pair(inst.numinc[i].first,inst.numinc[i].second));
      }
      

      for(int i = 0; i < inst.pre.size(); i++) 
	pre.push_back(make_pair(inst.pre[i].first,inst.pre[i].second));
      for(int i = 0; i < inst.preDel.size(); i++) 
	pdel.push_back(make_pair(inst.preDel[i].first,inst.preDel[i].second));
      for(int i = 0; i < inst.add.size(); i++) 
	add.push_back(make_pair(inst.add[i].first,inst.add[i].second));
      for(int i = 0; i < inst.del.size(); i++) 
	del.push_back(make_pair(inst.del[i].first,inst.del[i].second)); 

      string name = actions[a]->getName();
      action_name.push_back(name);
      
      for(int w = 0; w < inst.cpre.size(); w++) {
	  vector<pair <int,int> > wpre, wadd, wdel, wpdel;
	  for(int i = 0; i < inst.cpre[w].size(); i++) 
	      wpre.push_back(make_pair(inst.cpre[w][i].first,
				       inst.cpre[w][i].second));
	  for(int i = 0; i < inst.cpreDel[w].size(); i++) 
	      wpdel.push_back(make_pair(inst.cpreDel[w][i].first,
					inst.cpreDel[w][i].second));
	  for(int i = 0; i < inst.cadd[w].size(); i++) 
	      wadd.push_back(make_pair(inst.cadd[w][i].first,
				       inst.cadd[w][i].second));
	  for(int i = 0; i < inst.cdel[w].size(); i++) 
	      wdel.push_back(make_pair(inst.cdel[w][i].first,
				       inst.cdel[w][i].second)); 
	  cpre.push_back(wpre);
	  cpdel.push_back(wpdel);
	  cadd.push_back(wadd);
	  cdel.push_back(wdel);
      }



      Operator* Oper = new Operator(c,actions[a]->getDerived(),
				    actions[a]->getDuration(),
				    actions[a]->getDurative(),
				    pre,pdel,add,del,
				    numl,numle,numg,numge,nume,
				    numass,numinc,numdec,
				    cpre,cpdel,cadd,cdel,
				    *this);

//      cout << Oper->toString() << endl;
      operators.push_back(Oper);
      c++;
    }
  }
  for(int i = 0; i < operators.size(); i++) 
    if (operators[i]->derived) 
      for(int j = 0; j< operators[i]->addsize; j++)
	derived->set(operators[i]->add[j]);

  Operator *operi, *operj;
  int osize = getNoOperators();
  inherentSequential = true;

  /*

  for(int i=0;i<osize;i++) {
    operi = operators[i];
    for(int j=0;j<osize;j++) {
      operj = operators[j];
      if (! operi->depend(operj)) 
	inherentSequential = false;
    }
  }

  */

  if (options.verbose(Options::EXPLORE)) 
    if (inherentSequential)
      cout << "  problem is inherently sequential" << endl;
    else
      cout << "  problem is not inherently sequential" << endl;


}

string FactMap::getProblemName() { return domain.getProblemName(); } 
string FactMap::getDomainName() { return domain.getDomainName(); } 

string FactMap::toString() {
  string back;
  back += "Number of encoded Facts:" + ::toString(noFacts) + "\n";
  for(int i = 0; i < noFacts;i++) {
    back += "    Fact[" + ::toString(i) + "] = ";
    back += " " + variable_name[i] + "\n"; 
  }
  for(int i = 0; i < operators.size(); i++) {
    back += "    Operator[" + ::toString(i) + "] = ";
    back += operators[i]->toString() + "\n";
  }
  return back + "\n";
}
