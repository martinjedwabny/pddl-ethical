#ifndef GLOBALS_H
#define GLOBALS_H

#include <instantiation.h>

using Inst::Literal;

#include <set>
#include <list>
#include <utility>
#include <string>

using std::set;
using std::list;
using std::pair;
using std::string;

enum AutomatonPosition { satisfied = 0, unsatisfied = 1, triggered = 2, unreachable = 3, eternallysatisfied = 4, seenoncealreadyandstillholds = 5, seenoncealready = 6};

extern const char * positionName[7];

#define BIG 100000.0
#define SMALL 0.001


struct AddingConstraints {
    
    set<int> addingWillViolate;    
    list<pair<int,bool> > extraGoalsToAvoidViolations;
    
};

template<typename T>
class LiteralCellDependency {
    
public:
    T dest;
    int index;
    bool init;
    LiteralCellDependency(const T & d, const int & i) : dest(d), index(i), init(true) {};
    LiteralCellDependency() : init(false) {};
        
};


struct PreferenceSetAndCost {
    set<int> needsToViolate;
    double cost;
    
    int achiever;
    double atLayer;
    
    PreferenceSetAndCost(const bool satisfied=false)
        : cost(0.0), achiever(-1), atLayer(satisfied ? 0.0 : -1.0)
    {
    }
                                
    PreferenceSetAndCost(const int & currAct, const double & factLayerTime,
                         const double & ncnum, const set<int> & ncset)
        : needsToViolate(ncset), cost(ncnum), achiever(currAct), atLayer(factLayerTime)
    {
    }
};
    
struct LiteralLT {
    
    bool operator()(const Literal* const & a, const Literal* const & b) const  {
        if (a && b) {
            return (a->getGlobalID() < b->getGlobalID());
        } else {
            if (!a && b) return true;
            return false;
        }
    }

};

typedef set<Literal*, LiteralLT> LiteralSet;


#endif

