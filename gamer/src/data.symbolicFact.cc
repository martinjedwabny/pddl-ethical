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
//  Module:     mips\src\data.symbolicFact.cc
//  Authors:    S.Edelkamp, M.Helmert
// 
// ***********************************************************

using namespace std;

#include <lisp.entity.h>
#include <util.tools.h>
#include <data.symbolicFact.h>
#include <data.domain.h>
#include <data.action.h>
#include <data.predicate.h>

SymbolicFact::SymbolicFact(Action *a, map<string, int> &parTable,
                           Domain &d, LispEntity &le)
      : action(a) {
  vector<LispEntity> &vec = le.getList();
  if(vec.size() == 0)
    ::error("invalid symbolic fact: no predicate given");

  string name = vec[0].getString();
  //   if (name == "=") 
  //   name = "~";

  predicate = d.lookupPredicate(name);
  if (!predicate)
    ::error("unknown predicate " + name);
  arguments.reserve(vec.size() - 1);
}

SymbolicFact::SymbolicFact(const SymbolicFact &copy) : 
  action(copy.action),
  predicate(copy.predicate) {
}

SymbolicFact::SymbolicFact(Action *a, Domain &d, 
               pair<string, vector<int> > &description)
      : action(a), arguments(description.second) {
  predicate = d.lookupPredicate(description.first);
}
                                                                               
SymbolicFact::~SymbolicFact() {
}

string SymbolicFact::toString() {
  string back;
  back += predicate->getName();
  return back;
}

int SymbolicFact::instantiateFact() {
  return predicate->getFactLowerBound();
}


