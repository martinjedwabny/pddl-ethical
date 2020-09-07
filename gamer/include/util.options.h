
#ifndef _OPTIONS_H
#define _OPTIONS_H

#include <string>
using namespace std;

class Options {
public:
  enum {PARSING, CONSTANT, MERGE, EXPLORE, CODING,
        BDD, TRANSITION, HEURISTIC, PATTERN, SEARCHING, ANZ_PHASES};
  enum {COMPETITION, SILENT, NORMAL, VERBOSE, DEBUG};
  enum {C3STACK,C3DUPLICATE,C3STATIC};
  enum {DETERMINISTIC, BRANCHANDBOUND, DIJKSTRA, BDDA, PUREBDDA, WEAK, STRONG, STRONGCYCLIC,ASTAR,HILLCLIMBING, IDASTAR, BESTFIRST, PATTERNCONSTRUCT};
private:
  static string usage;
  void error(string s);
  void setAll(int level);
  int outputLevel[ANZ_PHASES];
  bool onlyPreprocess;
  bool onlyTransition;
  bool predecessor;
  bool ffheuristic;
  bool admiss;
  bool compression;
  bool bddPattern;
  bool vis;
  bool ghsearch;
  bool notAdditive;
  bool maximize;
  bool disjoint;
  bool zerocost; 
  bool unidirectional;
  int sequential;
  int algorithm;
  int maxState; 
  int maxnumber; 
  int pattern; 
  int maxPattern; 
  int length;
  int precision;
  int times;
  int weight;
  int offset;
  int trans;
  int heur;
  int resume;
  int bound;
  int forward;
  int bddinit;
  int bddmaxincrease;
  bool geneticpattern;
  int noGenerations;
  int populSize;
public:
  Options();
  void read(int argc, char *argv[], string &domFile, string &probFile);

  bool forwardPDB() { return forward; }
  bool zeroCost() { return zerocost; }
  bool transition() { return !onlyPreprocess; }
  bool preprocess()     { return onlyPreprocess; }
  bool sequentialGoal()       { return sequential; }
  bool admissible()     { return admiss; }
  bool bucketing()      { return ghsearch; }
  int resuming()      { return resume; }
  bool ffHeuristic()    { return ffheuristic; }
  bool compress()       { return compression; }
  bool pure()           { return algorithm == Options::PUREBDDA; }
  bool symbolicPattern(){ return bddPattern; } 
  bool visualize()      { return vis; }

  bool isDisjoint()     { return disjoint; } 
  bool isMaximize()     { return maximize; }

  int strategy()        { return algorithm; }
  int maxStates()       { return maxState; }
  int maxNumber()       { return maxnumber; }
  int patternDB()       { return pattern; }
  int maxPatterns()     { return maxPattern; }
  int abstractions()    { return times; }
  int widaWeight()      { return weight; }
  int maxLength()       { return length; }
  int getPrecision()    { return precision; }
  int getOffset()       { return offset; }
  int getBound()           { return bound; }
  int getBddInit()           { return bddinit; }
  int getBddMaxIncrease()           { return bddmaxincrease; }
  int transitions()     { return trans; }
  int heuristics()      { return heur; }
  bool geneticp() { return geneticpattern;}
  void changeStrategy();
  
  bool competition()  {return outputLevel[0] == COMPETITION;}
  bool silent(int i)  {return outputLevel[i] <= SILENT;}
  bool verbose(int i) {return outputLevel[i] >= VERBOSE;}
  bool debug(int i)   {return outputLevel[i] >= DEBUG;}
  bool allSilent();

  int noOfGenerations() {return noGenerations;}
  int populationSize() {return populSize;}
};

extern Options options;

#endif
