#include "solver-both.h"

#include "solver-clp.h"
#include "solver-cpx.h"

#include <sstream>

using std::ostringstream;

void readParams(char * argv[], const int & a)
{
    
}

MILPSolver * getNewSolver()
{
    MILPSolver * const toReturn = new MILPSolverBoth();
    return toReturn;
}

const double LPinfinity = COIN_DBL_MAX;



MILPSolverBoth::MILPSolverBoth()
    : solvers(4)
{
    solvers[0] = new MILPSolverCPX();
    solvers[1] = new MILPSolverCLP();
    solvers[2] = new MILPSolverCPX();
    solvers[3] = new MILPSolverCLP();
}

MILPSolverBoth::MILPSolverBoth(const MILPSolverBoth & c)
    : solvers(4)
{
    solvers[0] = c.solvers[0]->clone();
    solvers[1] = c.solvers[1]->clone();
    solvers[2] = c.solvers[2]->clone();
    solvers[3] = c.solvers[3]->clone();
}


MILPSolverBoth::~MILPSolverBoth()
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        delete solvers[s];
    }
}

MILPSolver * MILPSolverBoth::clone()
{
    return new MILPSolverBoth(*this);
}

double MILPSolverBoth::getInfinity()
{
    return COIN_DBL_MAX;
}

int MILPSolverBoth::getNumCols()
{
    const int toReturn = solvers[0]->getNumCols();
    for (unsigned int s = 1; s < solvers.size(); ++s) {
        assert(solvers[s]->getNumCols() == toReturn);
    }
    return toReturn;
}

int MILPSolverBoth::getNumRows()
{
    const int toReturn = solvers[0]->getNumRows();
    for (unsigned int s = 1; s < solvers.size(); ++s) {
        assert(solvers[s]->getNumRows() == toReturn);
    }
    return toReturn;
}

void MILPSolverBoth::setColName(const int & var, const string & asString)
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        solvers[s]->setColName(var,asString);
    }
}

string MILPSolverBoth::getColName(const int & var)
{
    return solvers[0]->getColName(var);
}

void MILPSolverBoth::setRowName(const int & cons, const string & asString)
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        solvers[s]->setRowName(cons, asString);
    }
}

string MILPSolverBoth::getRowName(const int & cons)
{
    return solvers[0]->getRowName(cons);
}

/*
void MILPSolverBoth::setInteger(const int & var)
{
    lp->setInteger(var);
}
*/

double MILPSolverBoth::getColUpper(const int & var)
{
    const double toReturn = solvers[0]->getColUpper(var);
    
    if (toReturn == solvers[0]->getInfinity()) {
        for (unsigned int s = 1; s < solvers.size(); ++s) {
            assert(solvers[s]->getColUpper(var) == solvers[s]->getInfinity());
        }
        return LPinfinity;
    } else {
        for (unsigned int s = 1; s < solvers.size(); ++s) {
            assert(fabs(solvers[s]->getColUpper(var) - toReturn) < 0.0000001);
        }
        return toReturn;
    }
}

void MILPSolverBoth::setColUpper(const int & var, const double & b)
{
    if (b == LPinfinity) {
        for (unsigned int s = 0; s < solvers.size(); ++s) {
            solvers[s]->setColUpper(var, solvers[s]->getInfinity());
        }                
    } else {
        for (unsigned int s = 0; s < solvers.size(); ++s) {
            solvers[s]->setColUpper(var, b);
        }
    }
}

double MILPSolverBoth::getRowUpper(const int & c)
{
    const double toReturn = solvers[0]->getRowUpper(c);
    
    if (toReturn == solvers[0]->getInfinity()) {
        for (unsigned int s = 1; s < solvers.size(); ++s) {
            assert(solvers[s]->getRowUpper(c) == solvers[s]->getInfinity());
        }
        return LPinfinity;
    } else {
        for (unsigned int s = 1; s < solvers.size(); ++s) {
            assert(fabs(solvers[s]->getRowUpper(c) - toReturn) < 0.0000001);
        }
        return toReturn;
    }
}

void MILPSolverBoth::setRowUpper(const int & c, const double & b)
{
    if (b == LPinfinity) {
        for (unsigned int s = 0; s < solvers.size(); ++s) {
            solvers[s]->setRowUpper(c, solvers[s]->getInfinity());
        }                
    } else {
        for (unsigned int s = 0; s < solvers.size(); ++s) {
            solvers[s]->setRowUpper(c, b);
        }
    }
}

double MILPSolverBoth::getRowLower(const int & c)
{
    const double toReturn = solvers[0]->getRowLower(c);
    
    if (toReturn == -solvers[0]->getInfinity()) {
        for (unsigned int s = 1; s < solvers.size(); ++s) {
            assert(solvers[s]->getRowLower(c) == -solvers[s]->getInfinity());
        }
        return LPinfinity;
    } else {
        for (unsigned int s = 1; s < solvers.size(); ++s) {
            assert(fabs(solvers[s]->getRowLower(c) - toReturn) < 0.0000001);
        }
        return toReturn;
    }
}

void MILPSolverBoth::setRowLower(const int & c, const double & b)
{
    if (b == -LPinfinity) {
        for (unsigned int s = 0; s < solvers.size(); ++s) {
            solvers[s]->setRowLower(c, -solvers[s]->getInfinity());
        }
    } else {
        for (unsigned int s = 0; s < solvers.size(); ++s) {
            solvers[s]->setRowLower(c, b);
        }
    }
}

double MILPSolverBoth::getColLower(const int & var)
{
    const double toReturn = solvers[0]->getColLower(var);
    
    if (toReturn == -solvers[0]->getInfinity()) {
        for (unsigned int s = 1; s < solvers.size(); ++s) {
            assert(solvers[s]->getColLower(var) == -solvers[s]->getInfinity());
        }
        return LPinfinity;
    } else {
        for (unsigned int s = 1; s < solvers.size(); ++s) {
            assert(fabs(solvers[s]->getColLower(var) - toReturn) < 0.0000001);
        }
        return toReturn;
    }
}

void MILPSolverBoth::setColLower(const int & var, const double & b)
{
    if (b == -LPinfinity) {
        for (unsigned int s = 0; s < solvers.size(); ++s) {
            solvers[s]->setColLower(var, -solvers[s]->getInfinity());
        }
    } else {
        for (unsigned int s = 0; s < solvers.size(); ++s) {
            solvers[s]->setColLower(var, b);
        }
    }
}

void MILPSolverBoth::setColBounds(const int & var, const double & lb, const double & ub)
{
    setColLower(var,lb);
    setColUpper(var,ub);
}

bool MILPSolverBoth::isColumnInteger(const int & c)
{
    const bool toReturn = solvers[0]->isColumnInteger(c);
    for (unsigned int s = 1; s < solvers.size(); ++s) {
        assert(solvers[s]->isColumnInteger(c) == toReturn);
    }
    return toReturn;
}

bool MILPSolverBoth::isColumnBinary(const int & c)
{
    const bool toReturn = solvers[0]->isColumnBinary(c);
    for (unsigned int s = 1; s < solvers.size(); ++s) {
        assert(solvers[s]->isColumnBinary(c) == toReturn);
    }
    return toReturn;
}

void MILPSolverBoth::addCol(const vector<pair<int,double> > & entries, const double & lb, const double & ub, const ColumnType & type)
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        const double localLB = (lb == -LPinfinity ? -solvers[s]->getInfinity() : lb);
        const double localUB = (ub ==  LPinfinity ?  solvers[s]->getInfinity() : ub);
        solvers[s]->addCol(entries,localLB,localUB,type);        
    }
}

void MILPSolverBoth::addRow(const vector<pair<int,double> > & entries, const double & lb, const double & ub)
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        const double localLB = (lb == -LPinfinity ? -solvers[s]->getInfinity() : lb);
        const double localUB = (ub ==  LPinfinity ?  solvers[s]->getInfinity() : ub);        
        solvers[s]->addRow(entries,localLB,localUB);        
    }    
}

void MILPSolverBoth::setMaximiseObjective(const bool & maxim)
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        solvers[s]->setMaximiseObjective(maxim);
    }
}

void MILPSolverBoth::setObjective(double * const entries)
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        solvers[s]->setObjective(entries);
    }
}

void MILPSolverBoth::clearObjective()
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        solvers[s]->clearObjective();
    }
    
}
    

void MILPSolverBoth::setObjCoeff(const int & var, const double & w)
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        solvers[s]->setObjCoeff(var,w);
    }
}

void MILPSolverBoth::writeLp(const string & filename)
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        ostringstream fn;
        fn << "solver" << s << filename;
        string newfn = fn.str();
        solvers[s]->writeLp(newfn.c_str());
    }
}

bool MILPSolverBoth::solve(const bool & skipPresolve)
{
    vector<MILPSolver*> backups(2);
    backups[0] = solvers[2]->clone();
    backups[1] = solvers[3]->clone();
    
    const bool toReturn = solvers[0]->solve(skipPresolve);
    bool fail = false;
    
    for (unsigned int s = 1; s < solvers.size(); ++s) {
        if (solvers[s]->solve(skipPresolve) != toReturn) {
            std::cerr << "Solver " << s << " disagrees\n";
            fail = true;
        }
    }
    
    if (fail) {
        writeLp("failed.lp");
    }
    
    assert(!fail);
    
    delete solvers[2];
    delete solvers[3];
    
    solvers[2] = backups[0];
    solvers[3] = backups[1];
    
    return toReturn;
}

const double * MILPSolverBoth::getSolution()
{
    return solvers[0]->getSolution();
}

const double * MILPSolverBoth::getSolutionRows()
{
    return solvers[0]->getSolutionRows();
}

const double * MILPSolverBoth::getPartialSolution(const int & from, const int & to)
{
    return solvers[0]->getPartialSolution(from, to);
}

double MILPSolverBoth::getSingleSolutionVariableValue(const int & col) {
    return solvers[0]->getSingleSolutionVariableValue(col);
}

double MILPSolverBoth::getObjValue()
{
    const double toReturn = solvers[0]->getObjValue();
    
    for (unsigned int s = 1; s < solvers.size() - 2; ++s) {
        assert(fabs(solvers[s]->getObjValue() - toReturn) < 0.0000001);
    }
    
    return toReturn;
}

bool MILPSolverBoth::supportsQuadratic() const
{
    return true;
}


void MILPSolverBoth::hush()
{
    for (unsigned int s = 0; s < solvers.size(); ++s) {
        solvers[s]->hush();
    }
}

void MILPSolverBoth::getRow(const int & row, vector<pair<int,double> > & entries)
{
    solvers[0]->getRow(row, entries);
    for (unsigned int s = 1; s < solvers.size(); ++s) {
        vector<pair<int,double> > tmpEntries;
        solvers[s]->getRow(row, tmpEntries);
        
        assert(tmpEntries.size() == entries.size());
        
        map<int,double> tmpMap;
        tmpMap.insert(tmpEntries.begin(), tmpEntries.end());
        
        const int entSize = entries.size();
        
        for (int e = 0; e < entSize; ++e) {
            map<int,double>::const_iterator tItr = tmpMap.find(entries[e].first);
            assert(tItr != tmpMap.end());
            assert(fabs(tItr->second - entries[e].second) < 0.00000001);
        }
    }
}
