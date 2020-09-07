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
//  Module:     mips\include\single.search.h
//  Authors:    S.Edelkamp, M.Helmert
// 
// Basic search class for the different search algorithm implementations.
//
// ***********************************************************

#ifndef _SEARCH_H
#define _SEARCH_H

#include <vector>
#include <list>

using namespace std;

#include <single.state.h>
#include <util.queue.h>

class FactMap;
class Operator;
class Timer;
class Heuristic;

class Search {

 protected:
    // enum {TIMEOUT,BACKTRACK,OVERFLOW,FALSE,TRUE}; // return values 

  Operator** operators;   // set of operators in the space
  int oSize;              // upper bound on the previous
  Operator*** preActions; // precomputed set of operators wrt to preconditions
  int* aSize;             // maintains the respective upper bounds
           
  int factCount;          
  int expansions;         // counts number of expansions for statistics
  int generations;        // counts number of generations for statistics

  State** open;            // not used 
  State* initState;        // initial state to start implicit exploration
  State* goalState;        // goal state to terminate implicit exploration
  Heuristic* heur;         // the heuristic estimate 

  State* finalState;       // temporary state variable
  State** succStates;      // array of successor states
  int opindex;             // index to traverse successor array

  FactMap& factMap;        // handle to planning problem interface

  int* FactArray;         // transferred array from bitvector
  int fSize;              // number of elements in the array
  int max;                // maximum number of states in exploration (deprec.)

  Operator* getOperator(State* from, State* to); 
  // computing operator given two states, necessary if executed action
  // is not stored together with the state
  virtual int search(Timer & t, State*& finalState) = 0;
  // abstract search procedure to be implemented 
  virtual void printSolution(Timer& time, vector<State *> path);
  // output the solution sequence, virtual for real time exploration
  int visPath();
  // includes state information in output for vega visualizer
 public:

  State* exploreSpace(Timer& globalTimer, int& length);
  int pert(Timer& timer,vector<State *> path);
  int pert(vector<State *> path);
  float pertDurative(Timer& timer,vector<State *> path);
  float pertDurative(vector<State *> path);
  Search(FactMap& fMap); 
  // initializes data structure, most importingly, actions are
  // additionally bucketed according to propositional precondition
  // for fast access
  string toString ();    // output name of execution engine
};
  

#endif
