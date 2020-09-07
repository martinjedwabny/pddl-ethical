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
//  Module:     mips\src\lisp.entity.cc
//  Authors:    S.Edelkamp, M.Helmert
// 
// ***********************************************************

#include <fstream>

using namespace std;

#include <util.tools.h>
#include <lisp.entity.h>

float LispEntity::eval() {
  if(atom)
    return atof(value.c_str());
  string oper(list[0].getString());
  if (oper == "*")
    return list[1].eval() * list[2].eval();
  if (oper == "/")
    return list[1].eval() / list[2].eval();
  if (oper == "+")
    return list[1].eval() + list[2].eval();
  if (oper == "-")
    return list[1].eval() - list[2].eval();
  ::error("operator does not exist" + list[0].getString());
}

bool LispEntity::isBoolean() {
  if(atom) {
    if (value == "and" || 
	value == "imply" || 
    value == "or" || 
    value == "not") 
      return true;
    else
      return false;
  }
  return false;
}


string LispEntity::toString() {
  if(atom)
    return value;
  string back;
  for(int i = 0; i < list.size(); i++) {
    back += (i == 0) ? '(' : ' ';
    back += list[i].toString();
  }
  return back + ')';
}

string LispEntity::coreString() {
  if(atom)
    return value;
  string back;
  for(int i = 0; i < list.size(); i++) {
    if (i != 0) back += ' ';
    back += list[i].toString();
  }
  return back;
}

string& LispEntity::getString() {
  if(!atom)
    ::error("atom required in expression " + toString());
  return value;
}

vector<LispEntity> &LispEntity::getList() {
  if(atom)
    ::error("list required in expression " + toString());
  return list;
}

vector<LispEntity> LispEntity::getAndList() {
  // returns list consisting of single entity if this is no
  // and list or returns list of entities if this is an and list
  if(!atom && list.size() > 0 && list[0].atom && list[0].value == "and")
    return vector<LispEntity>(list.begin() + 1, list.end());
  vector<LispEntity> back;
  if (!atom && list.size() == 0) return back;
  back.push_back(*this);
  return back;
}
