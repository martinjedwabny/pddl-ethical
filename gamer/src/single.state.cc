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
//  Module:     mips\src\single.state.cc
//  Authors:    S.Edelkamp, M.Helmert
// 
// ***********************************************************

#include <util.options.h>
#include <util.tools.h>
#include <util.bitarray.h>

#include <single.state.h>

#include <out.factMap.h>

/** constructor of propositional state encoded as a bitvector, 
    state includes link to predecessor, generating path length 
    executed action and heursitic estimate
 */

State::State(vector<int>& facts, FactMap& fMap): factMap(& fMap), 
  pred(0), g(0), h(0), time(0), action(0)
  //, dist(100000), brother(0) 
{
  bitvector = new BitArray(fMap.getNoFacts());
  for(int i = 0; i < facts.size(); i++) 
    bitvector->set(facts[i]);
}

/** dummy constructor 
 */

State::State(BitArray& init, FactMap& fMap): factMap(& fMap),
    pred(0), g(0), h(0), action(0), time(0)
  // , dist(100000), brother(0)  
{
  bitvector = new BitArray(init);
}
/** copy constructor
 */

State::State(const State& copy): factMap(copy.factMap), 
				 pred(copy.pred), g(copy.g), time(copy.time),
				 h(copy.h), action(copy.action)
				 //				 ,dist(copy.dist), brother(copy.brother)				 
{
  bitvector = new BitArray(* copy.bitvector);
}

/** another dummy constructor 
*/

State::State(FactMap& fMap): factMap(& fMap), time(0),
			     pred(0), g(0), h(0), action(0) 
  //, dist(100000), brother(0) 
{
  bitvector = new BitArray(fMap.getNoFacts());
}

/** destructor 
*/

State::~State() { delete bitvector; }

/** compute hash value in table of size max 
 */

int State::hash(int max) {
  return bitvector->hash(max);
}

/** project fact set to active groups, needed for pattern database
    construction, complexity O(factsize) 
*/

void State::project(int* group, bool* isActive) {
  for (int i = 0; i < size(); i++) {
    if (!isActive[group[i]]) {
      bitvector->clear(i);
    }
  }
}

/** enlarging state representation that is smaller than given 
    abstraction, complexity O(factsize) */

void State::enlarge(int* group, bool* isActive) {
  for (int i = 0; i < size(); i++) {
    if (!isActive[group[i]]) {
      bitvector->set(i);
    }
  }
}

/** return size of bitvector representation */

int State::size() { 
  return bitvector->size(); 
}

/** output for visualization in the vega-domain frontend
 */

string State::visString() {
  string back;
  for (int i = 0; i < size(); i++) {
    if (bitvector->get(i)) {
      back += "(" + factMap->getFactName(i) + ")\n";
    }
  }
  return back;
}

/** output for debug on std. I/O
 */

string State::toString() { 
  string back;
  back += "Propositional search information: \n (= g ";
  back += ::toString(g);
  back += ") (= h ";
  back += ::toString(h);
  back += ") (= a ";
  back += factMap->getActionName(action);
  back += ")\n";

  back += "Propositional State information: \n";
  for (int i = 0; i < size(); i++) {
    if (bitvector->get(i)) {
    back += " (" + factMap->getFactName(i) + ")\n";
    // back += factMap->getFactName(i) + " ";
    }
  }
  return back;
}

/** abbrevation to bitvector access
 */

bool State::get(int fact) { return bitvector->get(fact);} 
