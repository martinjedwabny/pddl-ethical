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
//  Module:     mips\src\out.group.cc
//  Authors:    S.Edelkamp, M.Helmert
// 
// ***********************************************************

#include <util.options.h>
#include <util.tools.h>
#include <util.bitarray.h>

#include <out.factMap.h>
#include <out.group.h>
#include <out.operator.h>


#include <bdd.h>

Group::Group(int nr, vector<int>& facts, bool omitted,
	     FactMap& fMap): factMap(fMap), number(nr), none(omitted) {

  factsize = facts.size();
  fact = new int [factsize];
  for (int i=0;i<factsize;i++) 
    fact[i] = facts[i];
}


Group::Group(const Group& copy): 
  number(copy.number), factMap(copy.factMap), none(copy.none) {

  factsize = copy.factsize;
  fact = new int[factsize];  
  for(int i = 0; i < factsize; i++) 
    fact[i] = copy.fact[i];
}

void Group::setBdd() {
  groupPreBdd = bddtrue;
  groupEffBdd = bddtrue;
  int pre[1], eff[1];
  pre[0] = factMap.getPreIndex(factMap.getGroup(fact[0]));
  eff[0] = factMap.getEffIndex(factMap.getGroup(fact[0]));
  groupPreBdd = fdd_makeset(pre,1);
  groupEffBdd = fdd_makeset(eff,1);
}

string Group::toString() {
  string back;
  for (int i=0;i<factsize;i++) 
    back += "    (" + factMap.getFactName(fact[i]) + ")\n";
  if (none) back += "    none\n";
  return back;
}

