#include "solver-lpsolve.h"

MILPSolver * getNewSolver()
{
    MILPSolver * const toReturn = new MILPSolverLPSolve();
    return toReturn;
}

const double LPinfinity = 1.0e30;

void readParams(char * argv[], const int & a)
{
    
}

