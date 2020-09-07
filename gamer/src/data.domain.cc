#include <iostream>
#include <algorithm>                                                      

#include <lisp.parser.h>
#include <lisp.entity.h>
#include <lisp.scanner.h>

#include <data.symbolicFact.h>
#include <data.domain.h>
#include <data.action.h>
#include <data.predicate.h>
#include <data.formula.h>

#include <util.tools.h>
#include <util.options.h>
#include <out.factMap.h>

Domain::Domain(string domFile, string probFile) {
  timed = false;
  try {
   Timer t;    

    string addFile = probFile;
    addFile[addFile.length()-3] = 's';
    addFile[addFile.length()-2] = 'a';
    addFile[addFile.length()-1] = 's';

    parsingStep(domFile, probFile,addFile);

    vector<int> fluentFacts;
    for (int i=0; i < predicates.size(); i++)
      fluentFacts.push_back(i);

    for (int a=0; a < actions.size(); a++) {
      Instantiation inst(actions[a]->getName(), *this);
      inst.pre.reserve(actions[a]->getAddPreconditions().size());
      inst.preDel.reserve(actions[a]->getDelPreconditions().size());
      inst.add.reserve(actions[a]->getAddEffects().size());
      inst.del.reserve(actions[a]->getDelEffects().size());

      inst.numdec.reserve(actions[a]->getDecEffects().size());
      inst.numinc.reserve(actions[a]->getIncEffects().size());
      inst.numass.reserve(actions[a]->getAssEffects().size());

      inst.numl.reserve(actions[a]->getLPreconditions().size());
      inst.numle.reserve(actions[a]->getLePreconditions().size());
      inst.numg.reserve(actions[a]->getGPreconditions().size());
      inst.numge.reserve(actions[a]->getGePreconditions().size());
      inst.nume.reserve(actions[a]->getEPreconditions().size());
 
      // inst.pref.reserve(actions[a]->getPrefs().size());
      
      vector<pair<int,SymbolicFact *> >::iterator pos, end;
      map<string, pair<int, Formula *> >::iterator mpos, mend;
      
      for(mpos = actions[a]->getPrefs().begin(), 
	      mend = actions[a]->getPrefs().end(); mpos != mend; ++mpos)
	  inst.pref[(*mpos).first] = make_pair((*mpos).second.first,
					       (*mpos).second.second);

      for(pos = actions[a]->getAddPreconditions().begin(), 
	      end = actions[a]->getAddPreconditions().end(); pos != end; ++pos)
	  inst.pre.push_back(make_pair((*pos).first,
				       (*pos).second->instantiateFact()));
      
      for(pos = actions[a]->getDelPreconditions().begin(), 
	      end = actions[a]->getDelPreconditions().end(); pos != end; ++pos) 
	  inst.preDel.push_back(make_pair((*pos).first,
					  (*pos).second->instantiateFact()));
      
      for(pos = actions[a]->getAddEffects().begin(), 
	      end = actions[a]->getAddEffects().end(); pos != end; ++pos) 
	  inst.add.push_back(make_pair((*pos).first,
				       (*pos).second->instantiateFact()));
      
      for(pos = actions[a]->getDelEffects().begin(), 	   
	      end = actions[a]->getDelEffects().end(); pos != end; ++pos) 
	  inst.del.push_back(make_pair((*pos).first,
				       (*pos).second->instantiateFact()));


      for(pos = actions[a]->getAssEffects().begin(), 	   
	      end = actions[a]->getAssEffects().end(); pos != end; ++pos) 
	  inst.numass.push_back(make_pair((*pos).first,
					  (*pos).second->instantiateFact()));

      for(pos = actions[a]->getIncEffects().begin(), 	   
	      end = actions[a]->getIncEffects().end(); pos != end; ++pos) 
	  inst.numinc.push_back(make_pair((*pos).first,
					  (*pos).second->instantiateFact()));

      for(pos = actions[a]->getDecEffects().begin(), 	   
	      end = actions[a]->getDecEffects().end(); pos != end; ++pos) 
	  inst.numdec.push_back(make_pair((*pos).first,
					  (*pos).second->instantiateFact()));


      for(pos = actions[a]->getLPreconditions().begin(), 	   
	      end = actions[a]->getLPreconditions().end(); pos != end; ++pos) 
	  inst.numl.push_back(make_pair((*pos).first,
					(*pos).second->instantiateFact()));

      for(pos = actions[a]->getLePreconditions().begin(), 	   
	      end = actions[a]->getLePreconditions().end(); pos != end; ++pos) 
	  inst.numle.push_back(make_pair((*pos).first,
					(*pos).second->instantiateFact()));

      for(pos = actions[a]->getGPreconditions().begin(), 	   
	      end = actions[a]->getGPreconditions().end(); pos != end; ++pos) 
	  inst.numg.push_back(make_pair((*pos).first,
					(*pos).second->instantiateFact()));

      for(pos = actions[a]->getGePreconditions().begin(), 	   
	      end = actions[a]->getGePreconditions().end(); pos != end; ++pos) 
	  inst.numge.push_back(make_pair((*pos).first,
					(*pos).second->instantiateFact()));




      
      vector<vector<pair <int,SymbolicFact *> > >& cPreAdd = 
	  actions[a]->getConditionalsAddPreconditions();
      vector<vector<pair <int,SymbolicFact *> > >& cPreDel = 
	  actions[a]->getConditionalsDelPreconditions();
      vector<vector<pair <int,SymbolicFact *> > >& cEffAdd = 
	  actions[a]->getConditionalsAddEffects();
      vector<vector<pair <int,SymbolicFact *> > >& cEffDel = 
	  actions[a]->getConditionalsDelEffects();
      

      for (int i=0;i<cPreAdd.size();i++) {
	  vector<pair <int,int> > wpre, wpreDel, wadd, wdel;

	  for(pos = cPreAdd[i].begin(), 
		  end = cPreAdd[i].end(); pos != end; ++pos)
	      wpre.push_back(make_pair((*pos).first,
					    (*pos).second->instantiateFact()));
	  
	  for(pos = cPreDel[i].begin(), 
		  end = cPreDel[i].end(); pos != end; ++pos)
	      
	      wpreDel.push_back(make_pair((*pos).first,
					       (*pos).second->instantiateFact()));
      	  for(pos = cEffAdd[i].begin(), 
		  end = cEffAdd[i].end(); pos != end; ++pos)

	      wadd.push_back(make_pair((*pos).first,
					    (*pos).second->instantiateFact()));

	  for(pos = cEffDel[i].begin(), 
		  end = cEffDel[i].end(); pos != end; ++pos)

	      wdel.push_back(make_pair((*pos).first,
				       (*pos).second->instantiateFact()));

	  inst.cpre.push_back(wpre);
	  inst.cpreDel.push_back(wpreDel);
	  inst.cadd.push_back(wadd);
	  inst.cdel.push_back(wdel);
      }

      actions[a]->logInstantiation(inst);
    }  
    FactMap factMap(predicates.size(), *this);
    factMap.initOperatorTable(actions);
    factMap.initGroupTable(gr);

    if (options.symbolicPattern() ||
	options.strategy() == Options::DETERMINISTIC ||
	options.strategy() == Options::DIJKSTRA ||
	options.strategy() == Options::PATTERNCONSTRUCT ||
	options.strategy() == Options::BRANCHANDBOUND ||
	options.strategy() == Options::BDDA ||
	options.strategy() == Options::PUREBDDA) 
	factMap.initBdds();
    // cout << factMap.toString() << endl;
    factMap.run(t);

  } catch(StringException &e) {
      cerr << "Error: " << e.toString() << endl;
  }
}
  
Domain::~Domain() {
}

string Domain::toString() {
  string back;
  back += "  Domain:         " + problemName + "\n";
  back +=  "  Problem:        " + domainName + "\n";
  return back;
}
                                                                              
Predicate* Domain::lookupPredicate(string str) {
  if(predicateTable.count(str))
    return predicates[predicateTable[str]];
  return 0;
}

void check(bool condition, string msg) {
  if(!condition)
    ::error(msg);
}


void Domain::parse(vector<LispEntity>& problem,
		   vector<LispEntity>& domain,
		   vector<LispEntity>::iterator& pos) {

  vector<LispEntity> constList;
  if(pos != domain.end()
     && pos->isList()
     && pos->getList().size() != 0
     && pos->getList()[0].isString()
     && pos->getList()[0].getString() == ":constants") {
    if(options.debug(Options::PARSING))
      cout << "  Reading constants..." << endl;
     constList = vector<LispEntity>(pos->getList().begin() + 1,pos->getList().end());
    pos++;
  }

  if(options.debug(Options::PARSING))
    for (int i=0;i<constList.size();i++)
      cout << constList[i].toString() << endl;

  if(options.debug(Options::PARSING))
    cout << "  Reading predicates..." << endl;
  check (pos != domain.end()
        && pos->isList()
        && pos->getList().size() > 0
        && pos->getList()[0].isString()
        && pos->getList()[0].getString() == ":predicates",
        "predicate specification missing or invalid");
  vector<LispEntity> predList(pos->getList().begin() + 1, pos->getList().end());
  ++pos;


  if(options.debug(Options::PARSING))
      cout << "  Reading functions..." << endl;
  vector<LispEntity> funcList;

  if (pos != domain.end()
        && pos->isList()
        && pos->getList().size() > 0
        && pos->getList()[0].isString()
      && pos->getList()[0].getString() == ":functions") {
      funcList = vector<LispEntity>(pos->getList().begin() + 1, pos->getList().end());
   ++pos;
  }
	/*
  else
      ::error("function specification missing");
	*/

  if(options.debug(Options::PARSING))
    cout << "  Reading actions..." << endl;
  vector<LispEntity> actionsList(pos, domain.end());

  // parse problem file
  if(options.debug(Options::PARSING))
    cout << "  Checking problem file header..." << endl;
  check(problem.size() >= 1
        && problem[0].isString()
        && problem[0].getString() == "define",
        "invalid problem file: define clause missing or incomplete");
  check(problem.size() >= 2
        && problem[1].isList()
        && problem[1].getList().size() == 2
        && problem[1].getList()[0].isString()
        && problem[1].getList()[1].isString()
        && problem[1].getList()[0].getString() == "problem",
        "problem name specification missing or invalid");
  problemName = problem[1].getList()[1].getString();
  check(problem.size() >= 3
        && problem[2].isList()
        && problem[2].getList().size() == 2
        && problem[2].getList()[0].isString()
        && problem[2].getList()[1].isString()
        && problem[2].getList()[0].getString() == ":domain"
        && problem[2].getList()[1].getString() == domainName,
        "domain specification in problem file is missing or does not match");

  vector<LispEntity> objList;

  if(options.debug(Options::PARSING))
    cout << "  Reading initial state..." << endl;
  check(problem.size() >= 4
        && problem[3].isList()
        && problem[3].getList().size() >= 1
        && problem[3].getList()[0].isString()
        && problem[3].getList()[0].getString() == ":init",
        "initial state specification missing or invalid");
  vector<LispEntity> initList(problem[3].getList().begin() + 1,
                              problem[3].getList().end());

  if(options.debug(Options::PARSING))
    cout << "  Reading goal state..." << endl;
  check(problem.size() >= 5
        && problem[4].isList()
        && problem[4].getList().size() >= 2
        && problem[4].getList()[0].isString()
        && problem[4].getList()[0].getString() == ":goal",
        "goal state specification missing or invalid");
  vector<LispEntity> goalList = problem[4].getList()[1].getAndList();

  if(options.debug(Options::PARSING))
    cout << "  Reading metric ..." << endl;
  if(problem.size() >= 6
        && problem[5].isList()
        && problem[5].getList().size() >= 2
        && problem[5].getList()[0].isString()
     && problem[5].getList()[0].getString() == ":metric") {

      if(options.debug(Options::PARSING))
	  cout << "    Reading metric type..." << endl;
      check (problem[5].getList()[1].getString() == "maximize" ||
	     problem[5].getList()[1].getString() == "minimize",
	     "metric neither maximized nor minimized");
      
      maximize = (problem[5].getList()[1].getString() == "maximize");
      metric = new LispEntity(problem[5].getList()[2]);
      // cout << metric->toString() << endl;
      if (metric->isList() && metric->getList().size() == 1) {
	  metric = new LispEntity(metric->getList()[0]);
	  if(options.debug(Options::PARSING))
	      cout << metric->toString() << endl;
      }
  }
  else {
  if(options.debug(Options::PARSING))
      cout << "    No or invalid metric given..." << endl;
    metric = (LispEntity *) 0;
  }

  int k=0;
  int paramcount = 0;
  factCount = 0;

  if(options.debug(Options::PARSING))
    cout << "  Parsing predicates..." << endl;

  for(int i = 0; i < predList.size(); i++) {
    Predicate *p = new Predicate(predList[i], factCount);
    check(predicateTable.count(p->getName()) == 0,
          "doubly defined predicate " + p->getName());
    predicateTable[p->getName()] = paramcount++;
    predicates.push_back(p);
    factCount = p->getFactUpperBound(0);
  }

  for(int i = 0; i < funcList.size(); i++) {
    Predicate *p = new Predicate(funcList[i], factCount);
    check(predicateTable.count(p->getName()) == 0,
          "doubly defined function " + p->getName());
    predicateTable[p->getName()] = paramcount++;
    predicates.push_back(p);
    factCount = p->getFactUpperBound(0);
  }

  if(options.debug(Options::PARSING))
    cout << "  Parsing actions / derived predicates..." << endl;
  for(int i = 0; i < actionsList.size(); i++) {
    actions.push_back(new Action(*this, actionsList[i]));
  }

  if(options.debug(Options::PARSING))
    cout << "  Parsing initial and goal state..." << endl;
  
  for(int i = 0; i < initList.size(); i++) {
      if (initList[i].isList()
	  && initList[i].getList().size() >= 2
	  ) {
	  if (initList[i].getList()[0].toString() != "=")
	      ::error ("wrong specification of numerical quantity");

	  Predicate* p = 
	      lookupPredicate(initList[i].getList()[1].coreString());
//	  if (!p) cout << initList[i].toString() << " not found" << endl;
	  int amount = (int) initList[i].getList()[2].eval();
	  if (amount != initList[i].getList()[2].eval()) {
	      cout << "WARNING:: fractional problem, working on integer approximation" << endl;
//	      ::error("fractional problem, please scale");
	  }
	  if (amount < 0 && amount >= options.maxNumber()) 
	      ::error(::toString(amount) + " negative, or value >= " 
		      + ::toString(options.maxNumber()) + ", please scale");
//	  cout << "numerical assignment of "
//	       << initList[i].getList()[1].toString() << " to "
//	       << amount  << endl;
          initNumFacts.push_back(make_pair(p->getFactLowerBound(),amount));
      }
      else {
	  Predicate* p = lookupPredicate(initList[i].coreString());
	  if (!p) cout << initList[i].toString() << " not found" << endl;
//	  cout << initList[i].toString()  << endl;
	  initFacts.push_back(p->getFactLowerBound());
      }
  }
  for(int i = 0; i < goalList.size(); i++) {
      string name = goalList[i].getList()[0].getString();

      if (name == "within") {
	  cout << "- within constraint string:" << endl;
	  cout << "\t" << goalList[i].toString() << endl;
	  
	  int to = atoi(goalList[i].getList()[1].getString().c_str());
	  LispEntity expression = goalList[i].getList()[2];	  
	  
	  cout << "--- to " << ::toString(to)
	       << " expression:" << expression.toString() << endl;

	  Formula* parsedExpr = new
	      Formula(predicateTable,*this,expression);

	  cout << "-- ParsedExpression "
	       << parsedExpr->toString() << endl;
	  // timeConstraints.push_back(goalList[i].instString());
	  Within.push_back(make_pair(to,parsedExpr));

	  continue;
      }

      if (name == "always-within") {
	  cout << "- always within constraint string:" << endl;
	  cout << "\t" << goalList[i].toString() << endl;
	  
	  int to = atoi(goalList[i].getList()[1].getString().c_str());
	  LispEntity expression = goalList[i].getList()[2];	  
	  
	  cout << "--- to " << ::toString(to)
	       << " expression:" << expression.toString() << endl;

	  Formula* parsedExpr = new
	      Formula(predicateTable,*this,expression);

	  cout << "-- ParsedExpression "
	       << parsedExpr->toString() << endl;
	  // timeConstraints.push_back(goalList[i].instString());
	  AlwaysWithin.push_back (make_pair(to,parsedExpr));

	  continue;
      }



      if (name == "hold-during") {
	  cout << "- hold-during constraint string:" << endl;
	  cout << "\t" << goalList[i].toString() << endl;
	  
	  int from = atoi(goalList[i].getList()[1].getString().c_str());
	  int to = atoi(goalList[i].getList()[2].getString().c_str());
	  LispEntity expression = goalList[i].getList()[3];	  
	  
	  cout << "--- from " << ::toString(from) 
	       << " to " << ::toString(to)
	       << " expression:" << expression.toString() << endl;

	  Formula* parsedExpr = new
	      Formula(predicateTable,*this,expression);

	  cout << "-- ParsedExpression "
	       << parsedExpr->toString() << endl;
//	  holdDuring.push_back(make_pair<>)
	  // timeConstraints.push_back(goalList[i].instString());
	  HoldDuring.push_back(make_pair(make_pair(from,to),parsedExpr));

	  continue;
      }


      if (name == "hold-after") {
	  cout << "- hold-after constraint string:" << endl;
	  cout << "\t" << goalList[i].toString() << endl;
	  
	  int from = atoi(goalList[i].getList()[1].getString().c_str());
	  LispEntity expression = goalList[i].getList()[2];	  

	  cout << "--- from " << ::toString(from) 
	       << " expression" << expression.toString() << endl;
	  
	  Formula* parsedExpr = new
	      Formula(predicateTable,*this,expression);

	  cout << "-- ParsedExpression "
	       << parsedExpr->toString() << endl;
	  
	  HoldAfter.push_back(make_pair(from,parsedExpr));
	      // holdAfter.push_back()
	  // timeConstraints.push_back(goalList[i].instString());
	  continue;
      }

      if (name == "preference") {
	  string pref = goalList[i].getList()[1].getString();
	  LispEntity expression = goalList[i].getList()[2];

/*
	  cout << "- preference string:" << endl;
	  cout << "\t" << goalList[i].toString() << endl;
	  cout << "- preference name:" << pref << endl;
	  cout << "- preference expression:" << expression.toString() << endl;
*/

//	  if (expression.toString() == "dummy") continue;

	  if (expression.isList() &&
	      (expression.getList()[0].getString() == "always-within")) {
	      cout << "- always-within constraint string:" << endl;
	      cout << "\t" << goalList[i].toString() << endl;

	      int to = atoi(expression.getList()[1].getString().c_str());
	      LispEntity subexpression = expression.getList()[2];	  
	      
	      Formula* parsedExpr = new
		  Formula(predicateTable,*this,subexpression);

	      cout << "--- to " << ::toString(to) 
		   << " expression:" << subexpression.toString() << endl;

	      cout << "-- ParsedExpression "
		   << parsedExpr->toString() << endl;
	      PrefAlwaysWithin.push_back
		  (make_pair(pref,make_pair(to,parsedExpr)));

	      //   timeConstraints.push_back(goalList[i].instString());
	      continue;
	  }

	  if (expression.isList() &&
	      (expression.getList()[0].getString() == "within")) {
	      cout << "- within constraint string:" << endl;
	      cout << "\t" << goalList[i].toString() << endl;

	      int to = atoi(expression.getList()[1].getString().c_str());
	      LispEntity subexpression = expression.getList()[2];	  
	      
	      Formula* parsedExpr = new
		  Formula(predicateTable,*this,subexpression);

	      cout << "--- to " << ::toString(to) 
		   << " expression:" << subexpression.toString() << endl;

	      cout << "-- ParsedExpression "
		   << parsedExpr->toString() << endl;
	      PrefWithin.push_back(make_pair(pref,make_pair(to,parsedExpr)));

	      //   timeConstraints.push_back(goalList[i].instString());
	      continue;
	  }

	  if (expression.isList() &&
	      (expression.getList()[0].getString() == "hold-during")) {
	      cout << "- hold-during constraint string:" << endl;
	      cout << "\t" << goalList[i].toString() << endl;

	      int from = atoi(expression.getList()[1].getString().c_str());
	      int to = atoi(expression.getList()[2].getString().c_str());
	      LispEntity subexpression = expression.getList()[3];	  
	      
	      Formula* parsedExpr = new
		  Formula(predicateTable,*this,subexpression);

	      cout << "--- from " << ::toString(from) 
		   << " to " << ::toString(to) 
		   << " expression:" << subexpression.toString() << endl;

	      cout << "-- ParsedExpression "
		   << parsedExpr->toString() << endl;
	      PrefHoldDuring.push_back(make_pair(pref,make_pair(make_pair(from,to),parsedExpr)));

	      //   timeConstraints.push_back(goalList[i].instString());
	      continue;
	  }

	  if (expression.isList() &&
	      (expression.getList()[0].getString() == "hold-after")) {
	      cout << "- hold-after constraint string:" << endl;
	      cout << "\t" << goalList[i].toString() << endl;

	      int from = atoi(expression.getList()[1].getString().c_str());
	      LispEntity subexpression = expression.getList()[2];	  
	      
	      Formula* parsedExpr = new
		  Formula(predicateTable,*this,subexpression);

	      cout << "--- from " << ::toString(from) 
		   << " expression" << subexpression.toString() << endl;
	      //   timeConstraints.push_back(goalList[i].instString());

	      cout << "-- ParsedExpression "
		   << parsedExpr->toString() << endl;
	      PrefHoldAfter.push_back(make_pair(pref,make_pair(from,parsedExpr) ));
	      continue;
	  }

	  preferenceTable[pref] = 
	      new Formula(predicateTable,*this,expression);
	  continue;
      }
 
      goalFacts.push_back(lookupPredicate(goalList[i].coreString())
			  ->getFactLowerBound());
  }

//  cout << metric->toString() << endl;


}
                                                                                       
void Domain::parsingStep(string domFile, string probFile, string addFile) {
  Timer t;

  LispParser parser;
  if(options.debug(Options::PARSING))
    cout << "  Reading domain file..." << endl;
  vector<LispEntity> domain = parser.parseFile(domFile).getList();
  if(options.debug(Options::PARSING))
    cout << "  Reading problem file..." << endl;
  vector<LispEntity> problem = parser.parseFile(probFile).getList();
  if(options.debug(Options::PARSING))
    cout << "  Reading SAS+ encoding file..." << endl;

  
  // parse domain file
  if(options.debug(Options::PARSING))
    cout << endl << "  Checking domain file header..." << endl;
  check(domain.size() >= 2
        && domain[0].isString()
        && domain[0].getString() == "define",
        "invalid domain file: define clause missing or incomplete");
  check(domain[1].isList()
        && domain[1].getList().size() == 2
        && domain[1].getList()[0].isString()
        && domain[1].getList()[1].isString()
        && domain[1].getList()[0].getString() == "domain",
        "domain name specification missing or invalid");
  domainName = domain[1].getList()[1].getString();
  vector<LispEntity>::iterator  pos = problem.begin() + 3;
  
  pos = domain.begin() + 2;

  if(pos != domain.end()
     && pos->isList()
     && pos->getList().size() != 0
     && pos->getList()[0].isString()
     && pos->getList()[0].getString() == ":requirements") {
    vector<LispEntity> require = pos->getList();
    for(int i = 1; i < require.size(); i++) {
      string s = require[i].getString();
    }
    ++pos;
  }
  
  parse(problem,domain,pos);

  vector<LispEntity> sas = parser.parseFile(addFile).getList();
  if(options.debug(Options::PARSING))
    cout << endl << "  Checking SAS+ header..." << endl;
  check (sas[0].getString() == ":partition","not suitable SAS+ encoding");
  for(int i=1;i<sas.size();i++) {
    vector<int> group;
    for(int j=1;j<sas[i].getList().size();j++) {
      Predicate* p = lookupPredicate(sas[i].getList()[j].coreString());
      if (!p) {
	check (sas[i].getList()[j].coreString() == "none-of-those",
	       "non-initialized predicate in SAS+ encoding" + 
	       sas[i].getList()[j].coreString()); 
	group.push_back(-1);
      }
      else 
	group.push_back(p->getFactLowerBound());
    }
    gr.push_back(group);
  }

  for(int i=0;i<gr.size();i++) {
      if (gr[i].size() == 0 || gr[i].size() > 2 || gr[i][1] != -1) 
	  continue;
      string p1s = predicates[gr[i][0]]->toString(); 
      for(int j=0;j<i;j++) {
	  if (gr[j].size() == 0 || gr[j].size() > 2 || gr[j][1] != -1) 
	      continue;
	  string p2s = predicates[gr[j][0]]->toString(); 
//	  cout << "p1s="  << p1s << " " << " p2s=" << p2s << endl; 
	  if (p2s == "not-"+p1s || p1s == "not-"+p2s) {

//	      cout << "moving group " << i << " to group " << j << endl;

	      gr[j].pop_back(); // remove none-of-those
	      for (int k=0;k<gr[i].size()-1;k++) {
		  gr[j].push_back(gr[i][k]);
	      }
	      gr[i].clear();
	      break;
	  }	      
      }
  }


  for (int i=0;i<gr.size();i++) {
      if (gr[i].size() == 0) continue;

      string p1s = predicates[gr[i][0]]->toString(); 
      if (p1s.substr(0,4) == "dead" ||  p1s.substr(0,11) == "is-violated") 
	  continue;

      int k = gr[i][gr[i].size()-1] == -1 ? 
	  gr[i].size()-2 :
	  gr[i].size()-1;
      p1s = predicates[gr[i][k]]->toString(); 
 
      for (int j=0;j<i;j++) {
	  if (gr[j].size() <= 1) continue;
//	  if (gr[j].size() <= 2) continue;

	  string p2s = predicates[gr[j][0]]->toString(); 
	  if (p2s.substr(0,4) == "dead" ||  p2s.substr(0,11) == "is-violated") 
	      continue;

	  int l = gr[j][gr[j].size()-1] == -1 ? 
	      gr[j].size()-2 :
	      gr[j].size()-1;
	  p2s = predicates[gr[j][l]]->toString(); 
	  if (p2s.substr(0,4) == "dead" ||  p2s.substr(0,11) == "is-violated") 
	      continue;

	  int stop1 = p1s.find_first_of(".",0);
	  int stop2 = p2s.find_first_of(".",0);
	  
	  if (stop1 == -1 || stop2 == -1) continue;



	  if (p1s.substr(stop1,p1s.length()) <
	      p2s.substr(stop2,p2s.length())) {
	      vector<int> temp = gr[i];
	      gr[i] = gr[j];
	      gr[j] = temp;
	  }
      }
  }

  if(options.verbose(Options::PARSING))
    cout << "Parsing time: " << t.total() << endl;
}



