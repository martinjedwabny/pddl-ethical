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
//  Module:     mips\include\data.instantiation.h
//  Authors:    S.Edelkamp, M.Helmert
// 
// Grounded action representation, attached as list to the corresponding
// action due to fact space exploration. A grounded action consists of 
// Strips part pre, add, del, extended by negated preconditions preDel, and
// numerical part 
//
// ***********************************************************

#ifndef _DATA_INSTANTIATION_H
#define _DATA_INSTANTIATION_H

#include <string>
#include <vector>
#include <map>
using namespace std;

class Domain;
class Formula;

class Instantiation {
  string name;
  int label;
  
  string 
    getOpString(int i)  const; 


 public:
  Instantiation(string action, Domain &dom);
  // generate one instantiation based on given action parameter list
  vector<pair <int,int> > pre, preDel, add, del;
  // strips atom indizes
  vector<pair <int,int> > numass, numinc, numdec;
  vector<pair <int,int> > numle, numl, numge, numg,nume;
  vector<vector<pair <int,int> > > cpre, cpreDel, cadd, cdel;
  map<string, pair <int,Formula*> > pref;

  // eliminate unneccessary instantiations
  bool Noop() const;

  string toString() const;
  // reduced string representation for solution sequence
};

#endif
