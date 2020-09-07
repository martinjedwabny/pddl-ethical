
#include <map>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;

#include <lisp.entity.h>
#include <util.tools.h>
#include <util.options.h>
#include <data.symbolicFact.h>
#include <data.formula.h>
#include <data.predicate.h>
#include <data.fact.h>
#include <data.action.h>
#include <data.domain.h>

#include <bdd.arithmetics.h>
#include <bdd.h>
#include <fdd.h>

typedef map<string, string> TypeMap;


Formula::Formula(Action *a, map<string, int> &funcTable,
         Domain &d, LispEntity &le)
  : action(*a), domain(d) {

  Value = "";
  Operator = "";
  Condition = "";
  sFact = (SymbolicFact *) 0;
  sString = (LispEntity *) 0;
  Left = (Formula *) 0;
  Right = (Formula *) 0;

  //  cout << "         parsing " << le.toString() << endl;
  if (le.isString()) {
    if(options.debug(Options::PARSING))
      cout << "         Constant " << le.toString() << endl;
    Value = le.getString();
  }
  else {
    vector<LispEntity> &vec = le.getList();
  
    if (vec[0].isOperator()) {
	if(options.debug(Options::PARSING))
	    cout << "         Operator " << le.toString() << endl;
	Operator = vec[0].getString();
	Left = new Formula(a,funcTable,d,vec[1]);
	Right = new Formula(a,funcTable,d,vec[2]);      
    }
    else {
	//      if (a == NULL && le.isList() && le.getList().size() == 1)
	
	if(options.debug(Options::PARSING))
	    cout << "         SymbolicFact " << le.toString() << endl;
	sFact = new SymbolicFact(a,funcTable,d,le);
	// sString = new LispEntity(le.getList());
    }
  }
}


Formula::Formula(map<string, int> &funcTable, Domain &d, LispEntity &le)
  : action(* new Action(d)), domain(d) {

  Value = "";
  Condition = "";
  Operator = "";
  sFact = (SymbolicFact *) 0;
  sString = (LispEntity *) 0;
  Left = (Formula *) 0;
  Right = (Formula *) 0;

  if (le.isString()) {
    if(options.debug(Options::PARSING))
      cout << "         Constant " << le.toString() << endl;
    Value = le.getString();
  }
  else {
    vector<LispEntity> &vec = le.getList();
    if (vec[0].isBoolean()) {
      if(options.debug(Options::PARSING))
	cout << "         Boolean " << le.toString() << endl;
      Condition = vec[0].getString(); 
      if (Condition == "not") {
	Left = new Formula(funcTable,d,vec[1]);
      }
      if (Condition == "and") {
	Left = new Formula(funcTable,d,vec[1]);
	if (vec.size() > 3) {
	  //	  cout << "splitting wide AND nodes " << endl;
	  LispEntity nop("and");
	  vector<LispEntity> nvec;
	  nvec.push_back(nop);
	  for(int i=2;i<vec.size();i++)
	    nvec.push_back(vec[i]);
	  LispEntity p(nvec);
	  Right = new Formula(funcTable,d,p);
	}
	else {
	  if (vec.size() == 3)
	    Right = new Formula(funcTable,d,vec[2]);
	}
      }
      if (Condition == "imply") {
	Left = new Formula(funcTable,d,vec[1]);
	if (vec.size() > 3) {
	  //	  cout << "splitting wide IMPLY nodes " << endl;
	  LispEntity nop("imply");
	  vector<LispEntity> nvec;
	  nvec.push_back(nop);
	  for(int i=2;i<vec.size();i++)
	    nvec.push_back(vec[i]);
	  LispEntity p(nvec);
	  Right = new Formula(funcTable,d,p);
	}
	else {
	  if (vec.size() == 3)
	    Right = new Formula(funcTable,d,vec[2]);
	}
      }
      if (Condition == "or") {
	Left = new Formula(funcTable,d,vec[1]);
	if (vec.size() > 3) {
	  // cout << "splitting wide or nodes " << endl;
	  LispEntity nop("or");
	  vector<LispEntity> nvec;
	  nvec.push_back(nop);
	  for(int i=2;i<vec.size();i++)
	    nvec.push_back(vec[i]);
	  LispEntity p(nvec);
	  Right = new Formula(funcTable,d,p);
	}
        else {
	  if (vec.size() == 3) 
	  Right = new Formula(funcTable,d,vec[2]);
	}
      }
    }
    else {
	string name = le.getList()[0].getString();
	Predicate* predicate = d.lookupPredicate(name);
	if (!predicate) { 
	    cout << "WARNING: predicate " << name 
		 << " eliminated to false" << endl;
	    sString = new LispEntity("false");
	}
	else {
	    sFact = new SymbolicFact(&action,funcTable,d,le);
	    sString = new LispEntity(le.getList());
	    
	    if(options.debug(Options::PARSING))
		cout << "         SymbolicString " 
		     << sString->toString() << endl;
	}
    }
  }
}


Formula::~Formula() {
  // cout << "destructor called" << endl;
  if (Left) { delete Left; }
  if (Right) { delete Right; }
}


LispEntity Formula::toLisp() {

  if (Value != "") 
    return LispEntity(Value);

  if (sString) {
      return LispEntity(sString->getList()); 
  }


  vector<LispEntity> back;


  if (Operator != "")
    back.push_back(LispEntity(Operator));




  if (Left) {
    LispEntity lret = Left->toLisp();
    back.push_back(lret);
  }
  if (Right) {
    LispEntity rret = Right->toLisp();
    back.push_back(rret);
  }
  return LispEntity(back);
}

/*

void Formula::sat(int bound) {
  int id;
  if (sString != NULL) { // for goal condition
    cout << sString->toString() << endl;
    vector<LispEntity> funcList = sString->getList();
    Function &func = *domain.lookupPredicate(funcList[0].getString());
     cout << " found " << func.toString() << endl;
    int objCount = domain.getObjectCount();
    id = 0;
    for(int i = 1; i < funcList.size(); i++) {
      id *= objCount;
      id += domain.lookupObject(funcList[i].getString()).toInt();
    }
    id += func.getFactLowerBound();

    if (domain.isConstant(Fact(id).toInt())) {
      return new Tree(domain.getValue(id).eval(), domain);
    } 
    else {
      return new Tree(Fact(id).toInt(), domain);   
    }
  }
  if (Value != "") {
    Function* func = domain.lookupFunction(Value);
    if (!func) {
      //      cout << Value << " not found" << endl;
      return new Tree (atof(Value.c_str()),domain);
    }
    id = func->getFactLowerBound();      
    if (domain.isConstant(id)) {
      return new Tree(domain.getValue(id).eval(), domain);
    } 
    else {
      return new Tree(id, domain);   
    }
    
  }
  else {
    Tree *l = 0; 
    Tree *r = 0;
    if (Left)
      l = Left->instantiate();
    if (Right)
      r = Right->instantiate();
    int oper = -1;

    if (Operator == "+") oper = 0;
    if (Operator == "-") oper = 1;
    if (Operator == "/") oper = 2; 
    if (Operator == "*") oper = 3; 

    if (oper == -1) 
      ::error("Wrong Operator "+ Operator +" in Formulae");
    return new Tree(oper,l,r,domain);
  }
    
}

*/

string Formula::sat(int upperBound) {
//  cout << "sat called" << endl;
  string back;
  if (Value != "") { 
    back += Value;
    return back;
  }
  if (sFact) {
      int id = sFact->instantiateFact();
      back += sFact->toString(); 
//      bdd added = *effAddFactBdd[id];
      back += ::toString(id);
  }
  if (sString)
    back += sString->toString(); 
  if (Left)
    back += Left->sat(upperBound)+ " ";
  if (Operator != "")
    back += Operator + " ";
  if (Right)
    back += Right->sat(upperBound)+ " ";
  return back;
}



void Formula::makeBdd(vector<bdd*> v) { 

  if (sFact) {
//                     cout << "-- fact bdd " ;
      int id = sFact->instantiateFact();      
                     //      cout << "makebdd proposition found " << id 
//	   << " max " << v.size() << endl;
      Binary = *v[id];
//      bdd_printtable(Binary);
      return;
  } 
  if (Condition != "") {
  //      cout << "makebdd condition " << Condition << " found " << endl;
      if (Condition == "not") {
	  if (!Left) exit(1);
	  Left->makeBdd(v); 
	  Binary = ! Left->Binary;
//	  bdd_printtable(Binary); 
      }
      if (Condition == "and") {
	  Left->makeBdd(v); 
	  Right->makeBdd(v); 
	  Binary = Left->Binary & Right->Binary; 
//	  bdd_printtable(Binary);
      }
      if (Condition == "imply") {
	  if (!Left || !Right) exit(1);
	  Left->makeBdd(v); 
	  Right->makeBdd(v); 
	  Binary = (!Left->Binary) | Right->Binary; 
//	  bdd_printtable(Binary);
      }
      if (Condition == "or") {
	  if (!Left || !Right) exit(1);
	  Left->makeBdd(v); 
	  Right->makeBdd(v); 
	  Binary = Left->Binary | Right->Binary; 
//	  bdd_printtable(Binary);
      }
      return;
  }
  if (sString->toString() == "false") {
                     cout << "-- false bdd " ;
	  Binary = bddfalse;
	  return;
  }

  ::error("Wrong Formulae");
}


                     
void Formula::makeBdd
                     (int index, vector<bdd*> v, int addIndex, 
                     bdd addBdd,int mint) { 
                     /* assumes linear expression */
  if (Value != "") {
//      cout << "makebdd value found " << atoi(Value.c_str())- mint << endl;
      Binary = fdd_ithvar(index,atoi(Value.c_str()) - mint);
//      bdd_printtable(Binary); 
      return;
  }
  if (sFact) {
      int id = sFact->instantiateFact();      
      //      cout << "makebdd proposition found " << sFact->toString() << id << endl;
      Binary = *v[id];
//      bdd_printtable(Binary);
      return;
  } 
  if (Operator != "") {
                     //      cout << "makebdd operator found " << endl;
      if (Operator == "+") {
//	  cout << "makebdd going left (+)" << endl;
	  Left->makeBdd(index,v,addIndex,addBdd,mint); 
//	  cout << "makebdd going right (+)" << endl;
	  Right->makeBdd (index,v,addIndex,addBdd,mint);
//	  cout << "makebdd back from recursion (+)" << endl;
	  bddPair* exchange = bdd_newpair();

	  int p1[1];
	  int p2[1];
	  p1[0]=index; 
	  p2[0]=addIndex; 
	  fdd_setpairs(exchange,p1,p2,1);
	  bdd leftBdd = bdd_replace(Left->Binary,exchange);
//	  cout << "leftbdd " << endl;
//	  bdd_printtable(leftBdd);

	  p2[0]=addIndex+1; 
	  fdd_setpairs(exchange,p1,p2,1);
	  bdd rightBdd = bdd_replace(Right->Binary,exchange);
//	  cout << "rightbdd " << endl;
//	  bdd_printtable(rightBdd);
//	  cout << "leftbdd & rightbdd " << endl;
//	  bdd_printtable(leftBdd & rightBdd);

	  int varray[2];
	  varray[0] = addIndex;
	  varray[1] = addIndex+1;
	  bdd aVariables = fdd_makeset(varray,2);
//	  cout << "aVariables " << endl;
//	  bdd_printtable(aVariables);
	  bdd result = bdd_relprod(leftBdd & rightBdd, addBdd, aVariables);
//	  cout << "result " << endl;
//	  bdd_printtable(result);

	  p1[0]=addIndex+2; 
	  p2[0]=index; 
	  fdd_setpairs(exchange,p1,p2,1);
	  Binary = bdd_replace(result,exchange);
//	  	  bdd_printtable(Binary);
      }
/*
      if (Operator == "-") 
      if (Operator == "/") 
*/ 
      if (Operator == "*") {
//	  cout << "makebdd going left (*)" << endl;
	  Left->makeBdd(index,v,addIndex,addBdd,mint); 
//	  cout << "makebdd going right (*)" << endl;
	  Right->makeBdd (index,v,addIndex,addBdd,mint);
//	  cout << "makebdd back from recursion (*)" << endl;
	  if (Right->Value != "") 
	      Binary = 
		  Left->Binary & Right->Binary |
		  !Left->Binary & fdd_ithvar(index,0);
	  else
	      Binary = 
		  Left->Binary & Right->Binary |
		  fdd_ithvar(index,0) & !Right->Binary;
//	  bdd_printtable(Binary);
      }

      return;
  }
  ::error("Wrong Formulae");
}

int Formula::maxint() {
  string back;
  if (Value != "") 
    return atoi(Value.c_str());
  if (sFact) {
    return 1;
  } 

  if (Operator != "") {

    if (Operator == "+") 
	return (Left->maxint() + Right->maxint ());
                     //    if (Operator == "-")
                     //	return (Left->maxint() - Right->maxint ());
                     //    if (Operator == "/") 
                     //	return (Left->maxint() / Right->maxint ());
    if (Operator == "*") {
       int leftmax = Left->maxint();
       int rightmax = Right->maxint ();
       int leftmin = Left->minint();
       int rightmin = Right->minint ();
 
       int maxmult = leftmax * rightmax;
       if (leftmax * rightmin > maxmult) maxmult = leftmax * rightmin;
       if (leftmin * rightmax > maxmult) maxmult = leftmin * rightmax;
       if (leftmin * rightmin > maxmult) maxmult = leftmin * rightmin;

       return maxmult;
    }
    ::error("Wrong Formulae");
  }
}

int Formula::minint() {
  string back;
  if (Value != "") 
    return atoi(Value.c_str());
  if (sFact) {
    return 0;
  } 
  if (Operator != "") {

    if (Operator == "+") 
	return (Left->minint() + Right->minint ());
                                         
//    if (Operator == "-")
//	return (Left->minint(maximize) - Right->maxint (!maximize));
//    if (Operator == "/") 
//	return (Left->maxint(maximize) / Right->maxint (!maximize));

    if (Operator == "*") {
       int leftmax = Left->maxint();
       int rightmax = Right->maxint ();
       int leftmin = Left->minint();
       int rightmin = Right->minint ();

       int minmult = leftmax * rightmax;
       if (leftmax * rightmin < minmult) minmult = leftmax * rightmin;
       if (leftmin * rightmax < minmult) minmult = leftmin * rightmax;
       if (leftmin * rightmin < minmult) minmult = leftmin * rightmin;

       return minmult;
    } 

    ::error("Wrong Formulae");
  }
}

void Formula::printBdd() {
  if (Value != "") 
      ; // bdd_printtable(Binary);
  if (sFact)
      ; // bdd_printtable(Binary);
  if (Operator != "") {
      cout << Operator + " " << endl;
      // bdd_printtable(Binary);
  }
  if (Left)
    Left->printBdd();
  if (Right)
   Right->printBdd();
}

int Formula::countLeaves() {
  if (Value != "") 
    return 0;
  if (sFact) {
      //cout << sFact->toString() << " " <<
      //sFact->toString().substr(0,11) << endl;
      if (sFact->toString().substr(0,11) == "is-violated" ||
          sFact->toString().substr(0,12) == "is-satisfied")
	  return 1;
      return 0;
  }
  return 
      Left->countLeaves() + 
      Right->countLeaves();
}

string Formula::toString() {
  string back;
  if (Value != "") 
    back += Value;
  if (Condition != "")
     back += Condition + " ";

  if(sString && sString->toString() == "false") {
     back += "false";
  }

  if (sFact)
    back += sFact->toString()+ " "; 
  if (Left)
    back += Left->toString()+ " ";
  if (Operator != "")
    back += Operator + " ";
  if (Right)
    back += Right->toString()+ " ";
  return back;
}
