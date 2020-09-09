#include "solver-clp.h"

MILPSolver * getNewSolver()
{
    MILPSolver * const toReturn = new MILPSolverCLP();
    return toReturn;
}

const double LPinfinity = COIN_DBL_MAX;

void readParams(char * argv[], const int & a)
{
    
}

