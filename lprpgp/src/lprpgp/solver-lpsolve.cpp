#include "solver-lpsolve.h"

#include <utility>
#include <iostream>
#include <cassert>

using std::cout;
using std::cerr;
using std::endl;

int external_solve(lprec* const lp)
{
    return solve(lp);
}

using std::make_pair;

double * MILPSolverLPSolve::scratchW = 0;
int * MILPSolverLPSolve::scratchI = 0;
int MILPSolverLPSolve::scratchSize = 0;

void MILPSolverLPSolve::transferToScratch(const vector<pair<int,double> > & entries)
{
    const int entSize = entries.size();
    if (entSize > scratchSize) {
        delete [] scratchW;
        delete [] scratchI;
        
        scratchSize = entSize + 16;
        scratchW = new double[scratchSize];
        scratchI = new int[scratchSize];
    }
    
    for (int i = 0; i < entSize; ++i) {
        scratchI[i] = entries[i].first + 1;
        scratchW[i] = entries[i].second;
    }
    
}
        

MILPSolverLPSolve::MILPSolverLPSolve()
{    
    lp = make_lp(0,0);
    hasIntegerVariables = false;
    solvedYet = false;
    tmpSolution = 0;
}

MILPSolverLPSolve::MILPSolverLPSolve(const MILPSolverLPSolve & c)
{
    assert(!c.solvedYet);
    lp = copy_lp(c.lp);
    hasIntegerVariables = c.hasIntegerVariables;
    solvedYet = false;
    tmpSolution = 0;
}


MILPSolverLPSolve::~MILPSolverLPSolve()
{
    delete_lp(lp); 
    delete [] tmpSolution;
}

MILPSolver * MILPSolverLPSolve::clone()
{
    return new MILPSolverLPSolve(*this);
}

double MILPSolverLPSolve::getInfinity()
{
    return 1.0e30;
}

int MILPSolverLPSolve::getNumCols()
{
    return get_Norig_columns(lp);
}

int MILPSolverLPSolve::getNumRows()
{
    return get_Norig_rows(lp);
}

void MILPSolverLPSolve::setColName(const int & var, const string & asString)
{
    static int vcopy;
    static string scopy;
    
    vcopy = var;
    scopy = asString;
    
    set_col_name(lp, var + 1, const_cast<char*>(scopy.c_str()));
    
}

string MILPSolverLPSolve::getColName(const int & var)
{
    return string(get_col_name(lp, var + 1));    
}

void MILPSolverLPSolve::setRowName(const int & cons, const string & asString)
{
    static int vcopy;
    static string scopy;
    
    vcopy = cons;
    scopy = asString;
    
    set_row_name(lp, cons + 1, const_cast<char*>(scopy.c_str()));

}

string MILPSolverLPSolve::getRowName(const int & cons)
{
    return string(get_row_name(lp, cons + 1));
}

/*
void MILPSolverLPSolve::setInteger(const int & var)
{
    lp->setInteger(var);
}
*/

double MILPSolverLPSolve::getColUpper(const int & var)
{
    const int Nrows = get_Norig_rows(lp);
    const int RI = get_lp_index(lp, var + 1 + Nrows);
    if (RI) {        
        return get_upbo(lp, RI);
    } else {
        return get_var_primalresult(lp, var + 1 + Nrows);
    }
}

void MILPSolverLPSolve::setColUpper(const int & var, const double & b)
{
    const int Nrows = get_Norig_rows(lp);
    const int RI = get_lp_index(lp, var + 1 + Nrows);
    if (RI) {
        set_upbo(lp, RI, b);
    }
   
}

double MILPSolverLPSolve::getRowUpper(const int & c)
{
    REAL rh = get_rh(lp, c + 1);
    if (rh == getInfinity()) {
        return getInfinity();
    }
    REAL rhRange = get_rh_range(lp, c + 1);
    if (rhRange == getInfinity()) {
        return getInfinity();
    }
    return rh + rhRange;
}

void MILPSolverLPSolve::setRowUpper(const int & c, const double & b)
{
    if (b == getInfinity()) {
        set_rh_range(lp, c + 1, b);
        return;
    }
    lp->set_rh_range(lp, c + 1, b - get_rh(lp, c + 1));
}

double MILPSolverLPSolve::getRowLower(const int & c)
{
    return get_rh(lp, c + 1);   
}

void MILPSolverLPSolve::setRowLower(const int & c, const double & b)
{
    REAL rhRange = get_rh_range(lp, c + 1); 
    REAL rh = get_rh(lp, c + 1);
    
    set_rh(lp, c + 1, b);
    set_rh_range(lp, c + 1, rhRange - (b - rh));
   
}

double MILPSolverLPSolve::getColLower(const int & var)
{
    const int Nrows = get_Norig_rows(lp);
    const int RI = get_lp_index(lp, var + 1 + Nrows);
    if (RI) {        
        return get_lowbo(lp, RI);
    } else {
        return get_var_primalresult(lp, var + 1 + Nrows);
    }
}

void MILPSolverLPSolve::setColLower(const int & var, const double & b)
{
    const int Nrows = get_Norig_rows(lp);
    const int RI = get_lp_index(lp, var + 1 + Nrows);
    if (RI) {
        //cout << "Setting lower bound on " << get_col_name(lp, RI) << " to " << b << endl;
        set_lowbo(lp, RI, b);
    } else {
        //cout << "Cannot set bounds on " << var << " - has been presolved out\n";
    }
}

void MILPSolverLPSolve::setColBounds(const int & var, const double & lb, const double & ub)
{
    const int Nrows = get_Norig_rows(lp);
    const int RI = get_lp_index(lp, var + 1 + Nrows);
    if (RI) {        
        //cout << "Setting bounds on " << get_col_name(lp, RI) << " to [" << lb << "," << ub << "]\n";
        set_bounds(lp, RI, lb, ub);
    } else {
        //cout << "Cannot set bounds on " << var << " to [" << lb << "," << ub << "] - has been presolved out\n";
    }
}

bool MILPSolverLPSolve::isColumnInteger(const int & c)
{
    return is_int(lp, c + 1);
}


void MILPSolverLPSolve::addCol(const vector<pair<int,double> > & entries, const double & lb, const double & ub, const ColumnType & type)
{
    static double emptyColW[1];
    static int emptyColI[1];
    
    if (entries.empty()) {
        emptyColW[0] = 0.0;
        emptyColI[0] = 0;
        add_columnex(lp, 0, emptyColW, emptyColI);
        set_bounds(lp, getNumCols(), lb, ub);
        if (type == C_INT) {
            set_int(lp, getNumCols(), TRUE);
            hasIntegerVariables = true;
        } else if (type == C_BOOL) {
            set_binary(lp, getNumCols(), TRUE);
            hasIntegerVariables = true;
        }
        return;
    }
    
    transferToScratch(entries);
    add_columnex(lp, entries.size(), scratchW, scratchI);
    set_bounds(lp, getNumCols(), lb, ub);
    if (type != C_REAL) {
        set_int(lp, getNumCols(), TRUE);
        hasIntegerVariables = true;
    }
}

void MILPSolverLPSolve::addRow(const vector<pair<int,double> > & entries, const double & lb, const double & ub)
{
    static double emptyRowW[1];
    static int emptyRowI[1];
    
    if (entries.empty()) {
        add_constraintex(lp, 0, emptyRowW, emptyRowI, GE, lb);
        setRowUpper(getNumRows() - 1, ub);
        return;
    }

    transferToScratch(entries);
    add_constraintex(lp, entries.size(), scratchW, scratchI, GE, lb);
    setRowUpper(getNumRows() - 1, ub);
    
}

void MILPSolverLPSolve::setMaximiseObjective(const bool & maxim)
{
    if (maxim) {
        set_maxim(lp);
    } else {
        set_minim(lp);
    }
}

void MILPSolverLPSolve::setObjective(double * const entries)
{
    const int lim = get_Norig_columns(lp);
    const int Nrows = get_Norig_rows(lp);
    int RI;    
    for (int c = 0; c < lim; ++c) {
        RI = get_lp_index(lp, Nrows + c + 1);
        if (RI) {
            set_obj(lp, RI, entries[c]);
        }
    }
    
}

void MILPSolverLPSolve::clearObjective()
{
    set_obj_fnex(lp, 0, (double*)0, (int*)0);
}
    

void MILPSolverLPSolve::setObjCoeff(const int & var, const double & w)
{
    const int Nrows = get_Norig_rows(lp);
    const int RI = get_lp_index(lp, Nrows + var + 1);
    
    if (RI) {
        set_obj(lp, RI, w);
    } else {
        cout << "Warning: cannot change objective coefficient of " << var << " as it has been removed\n";
    }
}

void MILPSolverLPSolve::writeLp(const string & filename)
{
    write_lp(lp, const_cast<char*>(filename.c_str()));
}

bool MILPSolverLPSolve::solve(const bool & skipPresolve)
{
    if (skipPresolve) {
        set_presolve(lp, 0, get_presolveloops(lp));
    } else {
        set_presolve(lp, 3, get_presolveloops(lp));
    }
    
    /*{
        int rule = get_bb_rule(lp);
        if (rule & NODE_RCOSTFIXING) {
            rule -= NODE_RCOSTFIXING;
            set_bb_rule(lp, rule);
        }        
    }*/
    solvedYet = true;        

    const int solveVal = external_solve(lp);
    return (solveVal == 0 || solveVal == 9);
    
}

const double * MILPSolverLPSolve::getSolution()
{
    
    delete [] tmpSolution;

    const int Ncols = get_Norig_columns(lp);
    const int Nrows = get_Norig_rows(lp);
    
    tmpSolution = new double[Ncols];
    
    REAL* results = new REAL[Ncols];    
    get_variables(lp, results);
    
    
    int RI;
    for (int c = 0; c < Ncols; ++c) {
        RI = get_lp_index(lp, Nrows + c + 1);
        if (RI) {
            tmpSolution[c] = results[RI - 1];
        } else {
            tmpSolution[c] = get_var_primalresult(lp, Nrows + c + 1);
        }
        
    }
    
    
    delete [] results;
    
    return tmpSolution;
}

const double * MILPSolverLPSolve::getSolutionRows()
{
    delete [] tmpSolution;
    
    const int Nrows = get_Norig_rows(lp);
    
    tmpSolution = new double[Nrows];

    for (int r = 0; r < Nrows; ++r) {
        tmpSolution[r] = get_var_primalresult(lp, r + 1);
    }
    
    return tmpSolution;
}

const double * MILPSolverLPSolve::getPartialSolution(const int & from, const int & to)
{
    delete [] tmpSolution;
    
    const int lim = to - from;
    tmpSolution = new double[lim];
    
    const int Ncols = get_Norig_columns(lp);
    const int offset = get_Norig_rows(lp) + 1 + from;

    REAL* results = new REAL[Ncols];    
    get_variables(lp, results);
    
    int RI;
    for (int c = 0; c < lim; ++c) {
        RI = get_lp_index(lp, from + 1 + c);
        if (RI) {
            tmpSolution[c] = results[RI - 1];
        } else {
            tmpSolution[c] = get_var_primalresult(lp, offset + c);
        }
    }
    
    delete [] results;
    
    return tmpSolution;
}

double MILPSolverLPSolve::getSingleSolutionVariableValue(const int & col)
{
    const double * tmp = getPartialSolution(col, col+1);
    return tmp[0];
}


double MILPSolverLPSolve::getObjValue()
{
    return get_objective(lp);
}

bool MILPSolverLPSolve::supportsQuadratic() const
{
    return true;
}


void MILPSolverLPSolve::hush()
{
    set_verbose(lp,3);
}

void MILPSolverLPSolve::getRow(const int & row, vector<pair<int,double> > & entries)
{
    
    const int colCount = get_Norig_columns(lp);
    REAL * const coeffs = new REAL[colCount];
    int * const vars = new int[colCount];
    
    const int rowSize = get_rowex(lp, row+1, coeffs, vars);
    
    entries.resize(rowSize);
    
    for (int e = 0; e < rowSize; ++e) {
        entries[e] = make_pair(vars[e] - 1, coeffs[e]);
    }
    
}
