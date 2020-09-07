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
//  Module:     mips\src\data.instantiation.cc
//  Authors:    S.Edelkamp, M.Helmert
// 
// ***********************************************************

#include <algorithm>  // sort

using namespace std;

#include <lisp.entity.h>
#include <util.tools.h>
#include <util.options.h>
#include <data.instantiation.h>
#include <data.domain.h>
#include <data.fact.h>


Instantiation::Instantiation(string action,
                 Domain &dom) {
  name = action;
}


string Instantiation::toString() const {
  string back = name;

  for(int w = 0; w < cpre.size(); w++) {
      back += " \ncond-pre: ";
      for(int i = 0; i < cpre[w].size(); i++) {
	  back += "      ";
	  back +=  "(" + ::toString(cpre[w][i].second)+ ") ";
	  back += "\n";
      }
      back += " cond-pdel: ";
      for(int i = 0; i < cpreDel[w].size(); i++) {
	  back += "      ";
	  back +=  "(" + ::toString(cpreDel[w][i].second)+ ") ";
	  back += "\n";
      }
      back += " cond-add: ";
      for(int i = 0; i < cadd[w].size(); i++) {
	  back += "      ";
	  back +=  "(" + ::toString(cadd[w][i].second)+ ") ";
	  back += "\n";
      }
      back += " cond-del: ";
      for(int i = 0; i < cdel[w].size(); i++) {
	  back += "      ";
	  back +=  "(" + ::toString(cdel[w][i].second)+ ") ";
	  back += "\n";
      }
  }

  return back;
}

bool Instantiation::Noop() const {
  //  cout << "noop called" << endl;
  if (add.size() != del.size()) return false;
  if (add.size() == 0 && del.size() == 0) return false;
  for (int i=0; i < add.size();i++) {
    if (add[i].second != del[i].second) return false;
  }
  if (numass.size() != 0) return false;
  if (numdec.size() != 0) return false;
  if (numinc.size() != 0) return false;
  return true;
}

