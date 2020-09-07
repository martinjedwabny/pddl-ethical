// ***********************************************************
// 
//  Book:       Heuristic Search
// 
//  Authors:    S.Edelkamp, S.Schroedl
// 
//  See file README for information on using and copying 
//  this software.
// 
//  Project:    Mips - model checking integrated planning
//              system
// 
//  Module:     mips\src\bdd.arithmetics.cc
//  Authors:    S.Edelkamp, M.Helmert
// 
// ***********************************************************

#include <bdd.arithmetics.h>
#include <util.options.h>
#include <bdd.h>
#include <fdd.h>
#include <util.tools.h>

const int undefined = -1;

Arithmetics::Arithmetics():
  maxIndex(undefined), greaterIndex(undefined), formulaIndex(undefined), 
  incIndex(undefined), addIndex(undefined), mvaddIndex(undefined) {

  MAX_ADD = (1 << options.maxLength())-1;
  int sizes[5];
  for(int i=0;i<5;i++) sizes[i] = MAX_ADD;
  int index = fdd_extdomain(sizes,5);

  preHeurIndex = index;
  effHeurIndex = index+1;
  preWeightIndex = index+2;
  preMeritIndex = index+3;
  effMeritIndex = index+4;

  int pre[1], eff[1];

  pre[0] = preHeurIndex; eff[0] = effHeurIndex;
  // heurPair = bdd_newpair();
  // fdd_setpairs(heurPair,pre,eff,1);
  preHeurVar = fdd_makeset(pre,1);
  effHeurVar = fdd_makeset(eff,1);
  pre[0] = preWeightIndex; 
  preWeightVar = fdd_makeset(pre,1);
  pre[0] = preMeritIndex; eff[0] = effMeritIndex;
  // meritPair = bdd_newpair();
  // fdd_setpairs(meritPair,pre,eff,1);
  preMeritVar = fdd_makeset(pre,1);
  effMeritVar = fdd_makeset(eff,1);

  int all[4];
  all[0] = preMeritIndex; all[1] = preWeightIndex; 
  all[2] = preHeurIndex; all[3] = effHeurIndex;
  allVars = fdd_makeset(all,4);  

}

void Arithmetics::getIndices(int* pre, int* eff, vector<int>& indices, bool inverse) {
  int domainCount = indices.size();
  for(int i = 0; i < domainCount ; i++) {
    if (inverse) { // regression search
     eff[i] = indices[i];
     pre[i] = eff[i]+1;      
    }
    else {
      pre[i] = indices[i];
      eff[i] = pre[i]+1;
    }
  }
  renamePair = bdd_newpair();  
  fdd_setpairs(renamePair, eff, pre, domainCount);
  renamePair2 = bdd_newpair(); 
  fdd_setpairs(renamePair, pre, eff, domainCount);
}

int Arithmetics::Increment(bdd& inc) {
  if (incIndex != undefined) {
    inc = bddInc;
    return incIndex;
  }

  Timer t;

/*
  if(!options.silent(Options::HEURISTIC))
      cout << "Building increment function, MAX_ADD " << MAX_ADD 
	   << "..." << endl;
*/
     
  inc = bddfalse;
  int* doms = new int[2]; doms[0] = MAX_ADD; doms[1] = MAX_ADD;
  int index = fdd_extdomain(doms,2);
  for (int i=0;i<MAX_ADD-1;i++) {
    if(!options.silent(Options::HEURISTIC))
	cout << i << "." << flush;
    inc |= fdd_ithvar(index,i) & fdd_ithvar(index+1,i+1); 
  }

  bddInc = inc;

  // if(!options.silent(Options::HEURISTIC)) 
//    cout << " time: " << t.stop() << "; size: " << flush;
//    cout << bdd_nodecount(inc) << endl;
  incIndex = index;
  return index;
}

int Arithmetics::Increment(bdd& inc,int maxvalue) {
    
    /*
      if (incIndex != undefined) {
      inc = bddInc;
      return incIndex;
      }
    */

  Timer t;
//  cout << "Building increment function max-value = " << maxvalue 
//       << "..." << endl;
     
  inc = bddfalse;
  int* doms = new int[2]; doms[0] = maxvalue+3; doms[1] = maxvalue+3;
  int index = fdd_extdomain(doms,2);
  for (int i=0;i<maxvalue+1;i++) {
    if(!options.silent(Options::HEURISTIC))
      cout << "." << flush;
    inc |= fdd_ithvar(index,i) & fdd_ithvar(index+1,i+1); 
  }

  // bddInc = inc;

//  if(!options.silent(Options::HEURISTIC))
//      cout << " time: " << t.stop() << "; size: " 
//	   << bdd_nodecount(inc) << endl;
  // incIndex = index;
  return index;
}



int Arithmetics::Greater(bdd& greater) {
  if (greaterIndex != undefined) {
    greater = bddGreater;
    return greaterIndex;
  }

  Timer t;
  if(!options.silent(Options::HEURISTIC))
    cout << "Building greater function..." << endl;

  int* doms = new int[3];   
  for(int i=0;i<3;i++) doms[i] = MAX_ADD;
  int index = fdd_extdomain(doms,3);
  int a=index, b=index+1, temp=index+2;

  int p1[3]; int p2[3];
  p1[0]=addIndex; p1[1]=addIndex+1; p1[2]=addIndex+2; 
  p2[0]=b; p2[1]=temp; p2[2]=a; 
  bddPair* exchange = bdd_newpair();
  fdd_setpairs(exchange,p1,p2,3);  
  greater = bdd_replace(bddAdd,exchange);

  p2[0] = temp;
  bdd variable = fdd_makeset(p2,1);

  bddGreater = greater = bdd_exist(greater,variable);

  if(!options.silent(Options::HEURISTIC))
    cout << " time: " << t.stop() << "; size: " << bdd_nodecount(greater) << endl;
  greaterIndex = index;
  return index;
}

int Arithmetics::Greater(bdd& greater,int maxvalue) {
    /*
      if (greaterIndex != undefined) {
      greater = bddGreater;
      return greaterIndex;
      }
    */

  Timer t;
//  cout << "Building greater function..." << endl;

  int* doms = new int[3];   
  for(int i=0;i<3;i++) doms[i] = maxvalue+3;
  int index = fdd_extdomain(doms,3);
  int a=index, b=index+1, temp=index+2;

  int p1[3]; int p2[3];
  p1[0]=mvaddIndex; p1[1]=mvaddIndex+1; p1[2]=mvaddIndex+2; 
  p2[0]=b; p2[1]=temp; p2[2]=a; 
  bddPair* exchange = bdd_newpair();
  fdd_setpairs(exchange,p1,p2,3);  
  greater = bdd_replace(mvbddAdd,exchange);

  p2[0] = temp;
  bdd variable = fdd_makeset(p2,1);

  // bddGreater = 
  greater = bdd_exist(greater,variable);

  if(!options.silent(Options::HEURISTIC))  
      cout << " time: " << t.stop() << "; size: " 
	   << bdd_nodecount(greater) << endl;
//  greaterIndex = index;
  return index;
}

int Arithmetics::Max(bdd& max) {
  if (maxIndex != undefined) {
    max = bddMax;
    return maxIndex;
  }

  int* doms = new int[2];   
  for(int i=0;i<2;i++) doms[i] = MAX_ADD;
  int index = fdd_extdomain(doms,2);
  int a=index, b=index+1;
  
  bdd greater;
  int gIndex = Greater(greater);
  int c=gIndex, d=gIndex+1;

  int p1[2]; int p2[2];
  p1[0]=c; p1[1]=d; 
  p2[0]=a; p2[1]=b;
  bddPair* exchange = bdd_newpair();

  fdd_setpairs(exchange,p1,p2,2);  
  greater = bdd_replace(greater,exchange);
  
  bddMax = max = 
    greater & fdd_equals(a,c) | 
    !greater & fdd_equals(b,c);
  
  maxIndex = index;
  return index;
}

int Arithmetics::Addition(bdd& add,int maxvalue) {
  if (mvaddIndex != undefined) {
    add = mvbddAdd;
    return mvaddIndex;
  }

  Timer t;
  
  bdd inc;
  int* p1 = new int[2];
  int* p2 = new int[2];
  bddPair* exchange = bdd_newpair();
  int incindex = Increment(inc,maxvalue);

//  cout << "Building addition function..." << endl;

  int* doms = new int[5];   
  for(int i=0;i<5;i++) doms[i] = maxvalue+3;
  int index = fdd_extdomain(doms,5);
  int a=index, b=index+1, c=index+2, bTemp=index+3, cTemp=index+4;
  
  p1[0]=incindex; p1[1]=incindex+1; 
  p2[0]=cTemp; p2[1]=c;
  fdd_setpairs(exchange,p1,p2,2);
  bdd cInc = bdd_replace(inc,exchange);

  p2[0]=bTemp; p2[1]=b;
  fdd_setpairs(exchange,p1,p2,2);
  bdd bInc = bdd_replace(inc,exchange);

  bdd End = fdd_equals(a,c) & fdd_ithvar(b,0);
  bdd Reach = End, From = End, To, New;

  p1[0]=b; p1[1]=c; p2[0]=bTemp; p2[1]=cTemp;
  fdd_setpairs(exchange,p1,p2,2);
  bdd variables = fdd_makeset(p2,2);

  do { 
    if(!options.silent(Options::HEURISTIC))
      cout << "." << flush;
    To = bdd_replace(From,exchange);
    To = bdd_relprod(cInc & bInc,To,variables);
    From = New = To & !Reach;
    Reach = Reach | New;
  } while (!(New == bddfalse));   

  add  = mvbddAdd  = Reach;
  if(!options.silent(Options::HEURISTIC))
      cout << " time: " << t.stop() << "; size: " 
	   << bdd_nodecount(add) << endl;

  mvaddIndex = index;
  return index;
}



int Arithmetics::Addition(bdd& add) {
  if (addIndex != undefined) {
    add = bddAdd;
    return addIndex;
  }

  Timer t;
  
  bdd inc;
  int* p1 = new int[2];
  int* p2 = new int[2];
  bddPair* exchange = bdd_newpair();
  int incindex = Increment(inc);

  if(!options.silent(Options::HEURISTIC))
    cout << "Building addition function..." << endl;

  int* doms = new int[5];   
  for(int i=0;i<5;i++) doms[i] = MAX_ADD;
  int index = fdd_extdomain(doms,5);
  int a=index, b=index+1, c=index+2, bTemp=index+3, cTemp=index+4;
  
  p1[0]=incindex; p1[1]=incindex+1; 
  p2[0]=cTemp; p2[1]=c;
  fdd_setpairs(exchange,p1,p2,2);
  bdd cInc = bdd_replace(inc,exchange);

  p2[0]=bTemp; p2[1]=b;
  fdd_setpairs(exchange,p1,p2,2);
  bdd bInc = bdd_replace(inc,exchange);

  bdd End = fdd_equals(a,c) & fdd_ithvar(b,0);
  bdd Reach = End, From = End, To, New;

  p1[0]=b; p1[1]=c; p2[0]=bTemp; p2[1]=cTemp;
  fdd_setpairs(exchange,p1,p2,2);
  bdd variables = fdd_makeset(p2,2);

  do { 
    if(!options.silent(Options::HEURISTIC))
      cout << "." << flush;
    To = bdd_replace(From,exchange);
    To = bdd_relprod(cInc & bInc,To,variables);
    From = New = To & !Reach;
    Reach = Reach | New;
  } while (!(New == bddfalse));   

  add = bddAdd = Reach;
//  if(!options.silent(Options::HEURISTIC))
//    cout << " time: " << t.stop() << "; size: " << bdd_nodecount(bddAdd) << endl;

  addIndex = index;
  return index;
}

bdd Arithmetics::getPreHeuristic() {
  return bddHeuristic;
}


bdd Arithmetics::getEffHeuristic() {
  bddPair* exchange = bdd_newpair();
  int p1[1], p2[1];
  p1[0]= preHeurIndex; p2[0]= effHeurIndex;
  fdd_setpairs(exchange,p1,p2,1);
  return bdd_replace(bddHeuristic,exchange);
}


bdd Arithmetics::buildFormula() {
  bdd addition;
  Addition(addition);
  bdd max;
  Max(max);
    
  bddPair* exchange = bdd_newpair();
  int p1[5]; int p2[5];
  bdd form;
  int index = Formula(form);
  p1[0]=index; 
  p1[1]=index+1; 
  p1[2]=index+2; 
  p1[3]=index+3; 
  p1[4]=index+4; 
  p2[0]=preHeurIndex; 
  p2[1]=effHeurIndex; 
  p2[2]=preWeightIndex; 
  p2[3]=preMeritIndex; 
  p2[4]=effMeritIndex;  
  fdd_setpairs(exchange,p1,p2,5);
  return bdd_replace(form,exchange);
}

int Arithmetics::Formula(bdd& formula) {
  if (formulaIndex != undefined) {
    formula = bddFormula; 
    return formulaIndex;
  }

  bddPair* exchange = bdd_newpair();
  int* p1 = new int[5];
  int* p2 = new int[5];

  int* doms = new int[7];   
  for(int i=0;i<7;i++) doms[i] = MAX_ADD;
  int index = fdd_extdomain(doms,7);
  int t1=index, t2=index+1, 
    a=index+2, b=index+3, c=index+4, 
    d=index+5, e=index+6;
   
  if(!options.silent(Options::HEURISTIC)) {
    cout << "Building formula function..." << endl;
    cout << " f(v)=f(u)-h(u)+h(v)+w(u,v) " << endl;
  }

  p1[0]=addIndex; p1[1]=addIndex+1; p1[2]=addIndex+2; 
  p2[0]=t1; p2[1]=a; p2[2]=d; 
  fdd_setpairs(exchange,p1,p2,3);
  bdd R1 = bdd_replace(bddAdd,exchange);

  p2[0]=t1; p2[1]=c; p2[2]=t2; 
  fdd_setpairs(exchange,p1,p2,3);
  bdd R2 = bdd_replace(bddAdd,exchange);

  p2[0]=t2; p2[1]=b; p2[2]=e; 
  fdd_setpairs(exchange,p1,p2,3);
  bdd R3 = bdd_replace(bddAdd,exchange);

  p2[0]=t1; p2[1]=t2;
  bdd variables = fdd_makeset(p2,2);
  
  bddFormula = formula = bdd_relprod(R1 & R2,R3,variables);

  formulaIndex = index + 2;
  return index + 2;
}
