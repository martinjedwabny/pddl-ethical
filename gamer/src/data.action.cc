
using namespace std;

#include <lisp.entity.h>
#include <util.tools.h>
#include <util.options.h>
#include <util.bitarray.h>
#include <data.action.h>
#include <data.formula.h>
#include <data.domain.h>
#include <data.symbolicFact.h>
#include <data.instantiation.h>

#include <algorithm>  // sort
using namespace std;

Action::Action(Domain &d): domain(d) {}
  
Action::Action(Domain &d, LispEntity &le):  
    domain(d), label(NORMAL), durative(false), duration(0) { // duration(1)

  if(le.isString()
     || le.getList().size() == 0
     || le.getList()[0].isList()
     || (le.getList()[0].getString() != ":action"
	 && le.getList()[0].getString() != ":durative-action"
	 && le.getList()[0].getString() != ":derived"))
    ::error("action specification expected");
 
  vector<LispEntity> &vec = le.getList();

  if(le.getList()[0].getString() != ":derived" &
     (vec.size() == 1 || vec[1].isList()))
    ::error("invalid action: no name given");

  if(le.getList()[0].getString() != ":derived")
    name = vec[1].getString();
  else
    name = vec[1].getList()[0].getString();

  if(options.debug(Options::PARSING))
    if(le.getList()[0].getString() != ":derived")
      cout << "    action " << ". " << name << endl;
    else
      cout << "    derived predicate " << ". " << name << endl;


  map<string, int> parameters;
  parameterCount = 0;
  derived = false;
  durative = false;
  int i = 2;

  vector<pair <int,LispEntity> > preAddList;
  vector<pair <int,LispEntity> > preDelList;
  vector<pair <int,LispEntity> > effAddList;
  vector<pair <int,LispEntity> > effDelList; 

  vector<pair <int,LispEntity> > leNumList; 
  vector<pair <int,LispEntity> > geNumList; 
  vector<pair <int,LispEntity> > lNumList; 
  vector<pair <int,LispEntity> > gNumList; 
  vector<pair <int,LispEntity> > eNumList; 


  vector<pair <int,LispEntity> > decNumList; 
  vector<pair <int,LispEntity> > incNumList; 
  vector<pair <int,LispEntity> > assNumList; 

  vector<vector<pair <int,LispEntity> > > cPreAddList;
  vector<vector<pair <int,LispEntity> > > cPreDelList;
  vector<vector<pair <int,LispEntity> > > cEffAddList;
  vector<vector<pair <int,LispEntity> > > cEffDelList;


  if (vec[0].getString() == ":durative-action") { 

    durative = true;
    if(vec[i].getString() != ":duration")
    ::error("duration missing: " + name);
    else
      i += 2;
    if(vec[i].getString() != ":condition")
    ::error("condition missing: " + name);
    else
      i += 2;
    if(vec[i].getString() != ":effect")
      ::error("effect missing: " + name);
    else
      i += 2;
    if(i < vec.size())
    ::error("invalid durative action specification: " + name);

    LispEntity &d = vec[3];    
    LispEntity &p = vec[5];    
    LispEntity &e = vec[7];

    const string types [] = {"none","start","all","end"};

    for (int k=1; k<4; k++) {  // at start, over all, at end

      if(options.debug(Options::PARSING))
	cout << "      Parsing conditions at time " << types[k] << endl;

      vector<LispEntity> ld = d.getList();   // duration
      duration = ld[2].eval();
      vector<LispEntity> lp = p.getAndList();   // (pre)conditions

      for (int j=0; j<lp.size();j++) {          // for each condition
	vector<LispEntity>& l = lp[j].getList();
	if (l.size() != 3)
	  ::error("condition size in durative action invalid");
	if (l[0].getString() != "over" &&
	    l[0].getString() != "at")
	  ::error("condition type in durative action invalid");
	string type = l[1].getString();
	if (type == types[k]) {
	  if(options.debug(Options::PARSING))
	    cout << "        Temporal operator " << type
		 << " condition " << l[2].toString() << endl;
	  if (l[2].getList()[0].getString() == "not")
	    preDelList.push_back(make_pair(k,l[2].getList()[1]));
	  else {
	    preAddList.push_back(make_pair(k,l[2]));
	  }
	}
      }

      vector<LispEntity> le = e.getAndList();   // effects

      for (int j=0; j<le.size();j++) {          // for each effect
	vector<LispEntity>& l = le[j].getList();
	if (l.size() != 3)
	  ::error("condition size in durative action invalid");
	if (l[0].getString() != "over" &&
	    l[0].getString() != "at")
	  ::error("condition type in durative action invalid");
	string type = l[1].getString();
	if (type == types[k]) {
	  if(options.debug(Options::PARSING))
	    cout << "        Temporal operator " << type
		 << " condition " << l[2].toString() << endl;
	  if (l[2].getList()[0].getString() == "not")
	    effDelList.push_back(make_pair(k,l[2].getList()[1]));
	  else {
	    effAddList.push_back(make_pair(k,l[2]));
	  }
	}
      }
    }
  }

  if (vec[0].getString() == ":action") {

    if(i + 3 < vec.size() && vec[i+2].getString() == ":effect") {

	vector<LispEntity> le = vec[i + 3].getAndList();
	for(int j = 0; j < le.size(); j++) {
	    vector<LispEntity> l = le[j].getList();
	    if (l[0].getString() == "increase") {
//		cout << " increase detected " << endl;
		if (l[1].toString() == "(total-cost)") {
//		    cout << " total cost increase " << l[2].toString() << endl;
		    duration = l[2].eval();
//		    cout << " evaluated " << duration << endl;
		}
		else {
		    int amount = (int) l[2].eval();	
		    if (amount != l[2].eval()) {
			cout << "WARNING: increasers fractional, working on integer approximation" << endl;
			::error("increasers fractional");
		    }
		    if (amount < 0 && amount >= options.maxNumber()) 
			::error(::toString(amount) + " negative, or value >= " 
				+ ::toString(options.maxNumber()) + ", please scale");
//		    cout << " variable " << l[1].toString() 
//			 << " += " << amount << endl;
		    incNumList.push_back(make_pair(amount,l[1]));
		}
	    }
	    if (l[0].getString() == "decrease") {
//		cout << " decrease detected " << endl;
		int amount = (int) l[2].eval();
		if (amount != l[2].eval()) ::error("decreasers fractional");
		if (amount < 0 && amount >= options.maxNumber()) 
		    ::error(::toString(amount) + " negative, or value >= " 
			    + ::toString(options.maxNumber()) + ", please scale");
//		cout << " variable " << l[1].toString() 
//		     << " -= " << amount << endl;
		decNumList.push_back(make_pair(amount,l[1]));
	    }
	    if (l[0].getString() == "assign") {
		cout << " assign detected " << endl;
		int amount = (int) l[2].eval();
		if (amount != l[2].eval()) ::error("assigners fractional");
		if (amount < 0 && amount >= options.maxNumber()) 
		    ::error(::toString(amount) + " negative, or value >= " 
			    + ::toString(options.maxNumber()) + ", please scale");
		cout << " variable " << l[1].toString() 
		     << " = " << amount << endl;
		assNumList.push_back(make_pair(amount,l[1]));
	    }
	}
    }




    if(i + 1 < vec.size() && vec[i].getString() == ":precondition") {

      vector<LispEntity> le = vec[i + 1].getAndList();
      preAddList.reserve(le.size());
      preDelList.reserve(le.size());
      // prefList.reserve(le.size());
      for(int j = 0; j < le.size(); j++) {
	vector<LispEntity> l = le[j].getList();

//	cout << l[0].getString() << endl;

	if (l[0].getString() == ">=") {
	    cout << ">= detected " << endl;
	    int amount = (int) l[2].eval();	
	    if (amount != l[2].eval()) ::error("comparison fractional");
	    if (amount < 0 && amount >= options.maxNumber()) 
		::error(::toString(amount) + " negative, or value >= " 
			+ ::toString(options.maxNumber()) + ", please scale");
//	    cout << " variable " << l[1].toString() 
//		 << amount << endl;
	    geNumList.push_back(make_pair(amount,l[1]));
	    continue;
	}

	if (l[0].getString() == "<=") {
	    cout << "<= detected " << endl;
	    int amount = (int) l[2].eval();	
	    if (amount != l[2].eval()) ::error("comparison fractional");
	    if (amount < 0 && amount >= options.maxNumber()) 
		::error(::toString(amount) + " negative, or value <= " 
			+ ::toString(options.maxNumber()) + ", please scale");
	    cout << " variable " << l[1].toString() 
		 << amount << endl;
	    leNumList.push_back(make_pair(amount,l[1]));
	    continue;
	}

	if (l[0].getString() == "=") {
	    cout << "= detected " << endl;
	    int amount = (int) l[2].eval();	
	    if (amount != l[2].eval()) ::error("comparison fractional");
	    if (amount < 0 && amount >= options.maxNumber()) 
		::error(::toString(amount) + " negative, or value = " 
			+ ::toString(options.maxNumber()) + ", please scale");
//	    cout << " variable " << l[1].toString() 
//		 << amount << endl;
	    eNumList.push_back(make_pair(amount,l[1]));
	    continue;
	}


	if (l[0].getString() == ">") {
	    cout << "> detected " << endl;
	    int amount = (int) l[2].eval();	
	    if (amount != l[2].eval()) ::error("comparison fractional");
	    if (amount < 0 && amount >= options.maxNumber()) 
		::error(::toString(amount) + " negative, or value > " 
			+ ::toString(options.maxNumber()) + ", please scale");
//	    cout << " variable " << l[1].toString() 
//		 << amount << endl;
	    gNumList.push_back(make_pair(amount,l[1]));
	    continue;
	}

	if (l[0].getString() == "<") {
	    cout << "< detected " << endl;
	    int amount = (int) l[2].eval();	
	    if (amount != l[2].eval()) ::error("comparison fractional");
	    if (amount < 0 && amount >= options.maxNumber()) 
		::error(::toString(amount) + " negative, or value < " 
			+ ::toString(options.maxNumber()) + ", please scale");
//	    cout << " variable " << l[1].toString() 
//		 << amount << endl;
	    lNumList.push_back(make_pair(amount,l[1]));
	    continue;
	}


	if (l[0].getString() == "preference") {
	    cout << " preference precondition found " << endl;
	    exit(1);
	  string prefname = l[1].toString();

	  cout << "- preference precondition string:" << endl;
	  cout << "\t" << le[j].toString() << endl;
	  cout << "- preference name:" << prefname << endl;

	  LispEntity expression = l[2];
	  cout << "- preference expression:" 
	       << expression.toString() << endl;	    
	    

	  Formula* form = new Formula(domain.getPredicateTable(),domain,expression);;
	  pref[prefname] = make_pair(0,form);


	}
	else {
	    if(l.size() == 2 && l[0].getString() == "not")
		preDelList.push_back(make_pair(0,l[1]));
	    else
		preAddList.push_back(make_pair(0,le[j]));
	}
      }
      i += 2;
    }


    if(i + 1 < vec.size() && vec[i].getString() == ":effect") {

      vector<LispEntity> le = vec[i + 1].getAndList();
      effAddList.reserve(le.size());
      effDelList.reserve(le.size());


      for(int j = 0; j < le.size(); j++) {
	vector<LispEntity> l = le[j].getList();

	if (l[0].getString() == "increase" || l[0].getString() == "decrease" ||
	    l[0].getString() == "assign") {
	    continue; 
	}

	if (l[0].getString() == "when") {

	    vector<LispEntity> cpre = l[1].getAndList();
	    
	    vector<pair <int,LispEntity> > wPreAdd;
	    vector<pair <int,LispEntity> > wPreDel;
	    vector<pair <int,LispEntity> > wEffAdd;
	    vector<pair <int,LispEntity> > wEffDel; 


	    for (int w=0;w<cpre.size();w++) {
		cout << "   cpre :"  << cpre[w].toString() << endl;
		if(cpre[w].isList() && 
		   cpre[w].getList().size() == 2 && 
		   cpre[w].getList()[0].getString() == "not")
		    wPreDel.push_back(make_pair(0,cpre[w].getList()[1]));
	        else 
		    wPreAdd.push_back(make_pair(0,cpre[w]));

	    }
	    cPreAddList.push_back(wPreAdd);
	    cPreDelList.push_back(wPreDel);

	    vector<LispEntity> ceffect = l[2].getAndList();

	    for (int w=0;w<ceffect.size();w++) {
		cout << "   ceffect :"  << ceffect[w].toString() << endl;
		if(ceffect[w].isList() && 
		   ceffect[w].getList().size() == 2 && 
		   ceffect[w].getList()[0].getString() == "not")
		    wEffDel.push_back(make_pair(0,ceffect[w].getList()[1]));
	        else 
		    wEffAdd.push_back(make_pair(0,ceffect[w]));
	    }
	    cEffAddList.push_back(wEffAdd);
	    cEffDelList.push_back(wEffDel);
	}
	else {
	    if(l.size() == 2 && l[0].getString() == "not")
		effDelList.push_back(make_pair(0,l[1]));
	    else
		effAddList.push_back(make_pair(0,le[j]));
	}
      }
      i += 2;
    }
    
    if(i < vec.size())
	::error("invalid action parameter: " + vec[i].getString()); 
}
  
  if (vec[0].getString() == ":derived") {
    derived = true;
    if(vec.size() < 1)
      ::error("rule missing: " + name);
    if(vec.size() < 2)
      ::error("rule incomplete, derived predicate: " + name);
    
    if(options.verbose(Options::PARSING)) {
      cout << "   deriviation " << vec[1].toString() << endl;
      cout << "   condition " << vec[2].toString() << endl;
    }
    effAddList.push_back(make_pair(0,vec[1]));
    
    vector<LispEntity> le = vec[2].getAndList();
    preAddList.reserve(le.size());
    for(int j = 0; j < le.size(); j++)
      preAddList.push_back(make_pair(0,le[j]));
  }

  for(int i = 0; i < preAddList.size(); i++)
    preAdd.push_back
      (make_pair(preAddList[i].first,
		 new SymbolicFact(this,parameters,d,preAddList[i].second)));
  for(int i = 0; i < preDelList.size(); i++)
    preDel.push_back
      (make_pair(preDelList[i].first,
		 new SymbolicFact(this,parameters,d,preDelList[i].second)));
  for(int i = 0; i < effAddList.size(); i++)
    effAdd.push_back
      (make_pair(effAddList[i].first,
		 new SymbolicFact(this,parameters,d,effAddList[i].second)));
  // cout << "comment for relaxed plan" << endl;
  for(int i = 0; i < effDelList.size(); i++)
    effDel.push_back
      (make_pair(effDelList[i].first,
		 new SymbolicFact(this,parameters,d,effDelList[i].second)));

// whens 


  for(int i = 0; i < geNumList.size(); i++)
    geNum.push_back
      (make_pair(geNumList[i].first,
		 new SymbolicFact(this,parameters,d,geNumList[i].second)));

  for(int i = 0; i < leNumList.size(); i++)
    leNum.push_back
      (make_pair(leNumList[i].first,
		 new SymbolicFact(this,parameters,d,leNumList[i].second)));

  for(int i = 0; i < gNumList.size(); i++)
    gNum.push_back
      (make_pair(gNumList[i].first,
		 new SymbolicFact(this,parameters,d,gNumList[i].second)));

  for(int i = 0; i < lNumList.size(); i++)
    lNum.push_back
      (make_pair(lNumList[i].first,
		 new SymbolicFact(this,parameters,d,lNumList[i].second)));

  for(int i = 0; i < eNumList.size(); i++)
    eNum.push_back
      (make_pair(eNumList[i].first,
		 new SymbolicFact(this,parameters,d,eNumList[i].second)));


  for(int i = 0; i < assNumList.size(); i++)
    assNum.push_back
      (make_pair(assNumList[i].first,
		 new SymbolicFact(this,parameters,d,assNumList[i].second)));

  for(int i = 0; i < decNumList.size(); i++)
    decNum.push_back
      (make_pair(decNumList[i].first,
		 new SymbolicFact(this,parameters,d,decNumList[i].second)));

  for(int i = 0; i < incNumList.size(); i++)
    incNum.push_back
      (make_pair(incNumList[i].first,
		 new SymbolicFact(this,parameters,d,incNumList[i].second)));


  for(int l = 0; l < cPreAddList.size(); l++) {
      vector<pair <int,SymbolicFact*> > cPreAddl;
      vector<pair <int,SymbolicFact*> > cPreDell;
      vector<pair <int,SymbolicFact*> > cEffAddl;
      vector<pair <int,SymbolicFact*> > cEffDell; 
      
      for(int i = 0; i < cPreAddList[l].size(); i++)
	  cPreAddl.push_back  
	      (make_pair(cPreAddList[l][i].first,
			 new SymbolicFact(this,parameters,d,cPreAddList[l][i].second)));
      for(int i = 0; i < cPreDelList[l].size(); i++)
	  cPreDell.push_back
	      (make_pair(cPreDelList[l][i].first,
			 new SymbolicFact(this,parameters,d,cPreDelList[l][i].second)));
      for(int i = 0; i < cEffAddList[l].size(); i++)
	  cEffAddl.push_back
	      (make_pair(cEffAddList[l][i].first,
			 new SymbolicFact(this,parameters,d,cEffAddList[l][i].second)));
      for(int i = 0; i < cEffDelList[l].size(); i++)
	  cEffDell.push_back
	      (make_pair(cEffDelList[l][i].first,
			 new SymbolicFact(this,parameters,d,cEffDelList[l][i].second)));

      cPreAdd.push_back(cPreAddl);
      cPreDel.push_back(cPreDell);
      cEffAdd.push_back(cEffAddl);
      cEffDel.push_back(cEffDell);

  }

  parameterCount = parameters.size();
}  

Action::~Action() {
}

vector<int> Action::split(string instance) {
  vector<int> parameter; 
  return parameter;
}


string Action::getfullString() {
  string back = name + "/" + ::toString(parameterCount);
  back += " (" + ::toString(label) + ")";
  if (derived)
    back += " derived predicate ";
  else {
    if (durative)
      back += " durative ";
    else
      back += " ordinary ";    
    back += " action ";
  }
  back += "\n        ";
  for(int i = 0; i < preAdd.size(); i++)
    back += " [P+@" + ::toString(preAdd[i].first) +"] " 
      + preAdd[i].second->toString();
  for(int i = 0; i < preDel.size(); i++)
    back += " [P-@" + ::toString(preDel[i].first) +"] " 
      + preDel[i].second->toString();
  for(int i = 0; i < effAdd.size(); i++)
    back += " [E+@" + ::toString(effAdd[i].first) +"] " 
      + effAdd[i].second->toString();
  for(int i = 0; i < effDel.size(); i++)
    back += " [E-@" + ::toString(effDel[i].first) +"] " 
      + effDel[i].second->toString();
  back += "\n        ";
// for (int i=0;i<whens.size();i++) 
//    back += "[when " + whens[i]->toString() +"]";
 return back;
}


string Action::toString() {
  string temp[4] = {"none","at start","over all","at end"};
  string back;
  bool durative = 
    (preAdd.size() > 0 && preAdd[0].first > 0 ||
     preDel.size() > 0 && preDel[0].first > 0 ||
     effAdd.size() > 0 && effAdd[0].first > 0 ||
     effDel.size() > 0 && effDel[0].first > 0);

  if (getDerived())
    back += "(:derived ";
  else {
    if (durative)
      back += "(:durative-";
    back += " action "  + name ;
  }
  back += "\n        ";
  if (durative) 
    back += " :condition";
  else
    back += " :precondition";

  if (preAdd.size() + preDel.size() > 0) 
    back += "  (and    \n";

  // pre add

  for(int i = 0; i < preAdd.size(); i++) 
    if (preAdd[i].first == 0)
      back += " (" + preAdd[i].second->toString() +  ") ";
    else
      back += " (" + 
	temp[preAdd[i].first] + " " +
	preAdd[i].second->toString() +  ") \n";
  // pre del

  for(int i = 0; i < preDel.size(); i++)
    if (preDel[i].first == 0)
      back += " (not (" + preDel[i].second->toString() +  ")) \n";
    else
      back += " (not (" + 
	temp[preDel[i].first] + " " +
	preDel[i].second->toString() +")) \n";


  for(int i = 0; i < lNum.size(); i++)
      back += " (< (" + ::toString(lNum[i].first) + "<-" 
	  + lNum[i].second->toString() +  ")) \n";

  for(int i = 0; i < leNum.size(); i++)
      back += " (<= (" + ::toString(leNum[i].first) + "<-" 
	  + leNum[i].second->toString() +  ")) \n";

  for(int i = 0; i < gNum.size(); i++)
      back += " (> (" + ::toString(gNum[i].first) + "<-" 
	  + gNum[i].second->toString() +  ")) \n";

  for(int i = 0; i < geNum.size(); i++)
      back += " (>= (" + ::toString(geNum[i].first) + "<-" 
	  + geNum[i].second->toString() +  ")) \n";


  for(int i = 0; i < eNum.size(); i++)
      back += " (= (" + ::toString(eNum[i].first) + "<-" 
	  + eNum[i].second->toString() +  ")) \n";


  if (preAdd.size() + preDel.size() > 0) 
    back += "  )    \n";

  back += " :effect";

  if (effAdd.size() + effDel.size() > 0) 
    back += "  (and    \n";

  for(int i = 0; i < effAdd.size(); i++)
    if (effAdd[i].first == 0)
      back += " (" + effAdd[i].second->toString() +  ") \n";
    else
      back += " (" + 
	temp[effAdd[i].first] + " " +
	effAdd[i].second->toString() +  ") \n";


  for(int i = 0; i < effDel.size(); i++)
    if (effDel[i].first == 0)
      back += " (not (" + effDel[i].second->toString() +  ")) \n";
    else
      back += " (not (" + 
	temp[effDel[i].first] + " " +
	effDel[i].second->toString() +")) ";

  for(int i = 0; i < decNum.size(); i++)
      back += " (decrease (" + ::toString(decNum[i].first) + "<-" 
	  + decNum[i].second->toString() +  ")) \n";

  for(int i = 0; i < incNum.size(); i++)
      back += " (increase (" + ::toString(incNum[i].first) + "<-" 
	  + incNum[i].second->toString() +  ")) \n";

  for(int i = 0; i < assNum.size(); i++)
      back += " (assign (" + ::toString(assNum[i].first) + "<-" 
	  + assNum[i].second->toString() +  ")) \n";

  if (effAdd.size() + effDel.size() > 0) 
    back += "  ) \n ";

// for (int i=0;i<whens.size();i++) 
//      back += "[when " + whens[i]->toString() +"]";
 return back;
}

