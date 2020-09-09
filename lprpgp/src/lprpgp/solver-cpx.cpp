#include "solver-cpx.h"

bool cpxparametersprovided = false;

vector<pair<int, int> > intParams;
vector<pair<int, int> > boolParams;
vector<pair<int,double> > numParams;

class IloCplex;
extern void setILS(IloCplex * c);


#include <sstream>

#define IL_STD 1

#include <ilcplex/ilocplex.h>

ILOSTLBEGIN


struct MILPSolverCPX::Constraints {
      
    map<int, IloRange> data;
};

MILPSolverCPX::MILPSolverCPX()
{
    env = new IloEnv;
    envUsers = new int;    
    (*envUsers) = 1;
    
    model = new IloModel(*env);
    
    modelvar = new IloNumVarArray(*env);
    warmstart = new IloNumArray(*env);
    modelcon = new Constraints();
    
    obj = new IloObjective(*env);
    obj->setSense(IloObjective::Minimize);
    
    model->add(*obj);
    
    colCount = 0;
    rowCount = 0;
    
    
    #ifndef NDEBUG
    try {
    #endif
    cplex = new IloCplex(*model);
    #ifndef NDEBUG
    }
    catch (IloCplex::Exception e) {
        cerr << "Fatal error: " << e << endl;
        exit(1);
    }
    #endif
    
    
    setILS(cplex);
    solArray = 0;
    warmStartChangedSinceLastSolve = true;
}

MILPSolverCPX::MILPSolverCPX(MILPSolverCPX & c)
{
    static const bool debug = false;
    
    if (debug) {
        cout << "Copying...\n";
        ostringstream copyfn;
        copyfn << "copyconstructor" << *(c.envUsers) << ".lp";
        const string copyfns = copyfn.str();
        c.writeLp(copyfns.c_str());
    }
        
    env = c.env;
    envUsers = c.envUsers;
    ++(*envUsers);
    
    model = new IloModel(*env);
    
    modelvar = new IloNumVarArray(*env);
    warmstart = new IloNumArray(*env);
    modelcon = new Constraints();   
    
    obj = new IloObjective(*env);
    obj->setSense(IloObjective::Minimize);
    
    model->add(*obj);

    
    colCount = c.colCount;
    rowCount = c.rowCount;

    integervars = c.integervars;   

    map<int,bool>::const_iterator iItr = integervars.begin();
    const map<int,bool>::const_iterator iEnd = integervars.end();
        
    for (int ci = 0; ci < colCount; ++ci) {
        const double lb = (*(c.modelvar))[ci].getLB();
        const double ub = (*(c.modelvar))[ci].getUB();
        
        if (iItr != iEnd && iItr->first == ci) {
            modelvar->add(IloNumVar(*env, lb, ub, (iItr->second ? ILOBOOL : ILOINT)));
            //cout << "Column " << ci << " in the copy is an integer\n";
            ++iItr;
        } else {        
            modelvar->add(IloNumVar(*env, lb, ub));
        }
        const char * oldName = (*(c.modelvar))[ci].getName();
        (*modelvar)[ci].setName(oldName);
        (*warmstart).add((*(c.warmstart))[ci]);
    }
    
    coeffs = c.coeffs;
    
    map<int,map<int,double> >::const_iterator coItr = coeffs.begin();
    const map<int,map<int,double> >::const_iterator coEnd = coeffs.end();
    
    map<int,IloRange>::iterator dItr = c.modelcon->data.begin();
    const map<int,IloRange>::iterator dEnd = c.modelcon->data.end();
    
    for (int r = 0; dItr != dEnd; ++r, ++dItr) {
        const double lb = dItr->second.getLB();
        const double ub = dItr->second.getUB();
        
        IloRange & newRange = modelcon->data[r] = IloAdd(*model, IloRange(*env, lb,ub));
        
       
        if (dItr->second.getName()) {
            newRange.setName(dItr->second.getName());
        }
        
        if (coItr == coEnd) continue;
        if (r < coItr->first) continue;
        
        map<int,double>::const_iterator fItr = coItr->second.begin();
        const map<int,double>::const_iterator fEnd = coItr->second.end();
        
        for (; fItr != fEnd; ++fItr) {
            newRange.setLinearCoef((*modelvar)[fItr->first], fItr->second);
        }
        ++coItr;
    }
    
    
    
    cplex = new IloCplex(*model);
    setILS(cplex);
    
    solArray = 0;
    
    if (debug) {
        ostringstream copyfn;
        copyfn << "aftercopyconstructor" << *(c.envUsers) << ".lp";
        const string copyfns = copyfn.str();
        
        writeLp(copyfns.c_str());
    }
    
    warmStartChangedSinceLastSolve = true;
}


MILPSolverCPX::~MILPSolverCPX()
{
    delete cplex;
    delete obj;
    delete modelcon;
    delete modelvar;
    delete warmstart;
    delete model;
    
    if (!(--(*envUsers))) {
        env->end();
        delete env;
        delete envUsers;
    }
    
    delete [] solArray;
}

MILPSolver * MILPSolverCPX::clone()
{
    return new MILPSolverCPX(*this);
}

double MILPSolverCPX::getInfinity()
{    
    return IloInfinity;
}

int MILPSolverCPX::getNumCols()
{
    return colCount;
}

int MILPSolverCPX::getNumRows()
{
    return rowCount;
}

void MILPSolverCPX::setColName(const int & var, const string & asString)
{
    string nospaces = asString;
        
    const int len = asString.length();
    
    for (int l = 0; l < len; ++l) {
        if (asString[l] != ' ' && asString[l] != '(' && asString[l] != ')' && asString[l] != '-') {
        } else {
            nospaces[l] = '_';
        }
    }
    
    //std::ostringstream nn;
    //nn << "c" << var;
    
    (*modelvar)[var].setName(nospaces.c_str());
    //(*modelvar)[var].setName(nn.str().c_str());
}

string MILPSolverCPX::getColName(const int & var)
{
    const char * n = (*modelvar)[var].getName();
    if (n) {
        return string(n);
    } else {
        std::ostringstream nn;
        nn << "c" << var;
        return nn.str();
    }
}

void MILPSolverCPX::setRowName(const int & cons, const string & asString)
{
    string nospaces = asString;
    
    const int len = asString.length();
    
    for (int l = 0; l < len; ++l) {
        if (asString[l] != ' ' && asString[l] != '(' && asString[l] != ')' && asString[l] != '-') {
        } else {
            nospaces[l] = '_';
        }
    }
    
    modelcon->data[cons].setName(nospaces.c_str());
}

string MILPSolverCPX::getRowName(const int & cons)
{
    const char * n = modelcon->data[cons].getName();
    if (n) {
        return string(n);
    } else {
        std::ostringstream nn;
        nn << "row" << cons;
        return nn.str();
    }
}
/*
void MILPSolverCPX::setInteger(const int & var)
{
    model->add(IloConversion(*env, (*modelvar)[var], ILOINT));
    integervars.insert(var);
}*/

double MILPSolverCPX::getColUpper(const int & var)
{
    return (*modelvar)[var].getUB();
}

void MILPSolverCPX::setColUpper(const int & var, const double & b)
{
    (*modelvar)[var].setUB(b);    
}

double MILPSolverCPX::getColLower(const int & var)
{
    return (*modelvar)[var].getLB();
}

void MILPSolverCPX::setColLower(const int & var, const double & b)
{
    (*modelvar)[var].setLB(b);    
}

void MILPSolverCPX::setColBounds(const int & var, const double & lb, const double & ub)
{
    (*modelvar)[var].setBounds(lb,ub);
}


void MILPSolverCPX::setRowUpper(const int & r, const double & b)
{
    (modelcon->data)[r].setUB(b);
}

void MILPSolverCPX::setRowLower(const int & r, const double & b)
{
    (modelcon->data)[r].setLB(b);
}

double MILPSolverCPX::getRowUpper(const int & r)
{
    return (modelcon->data)[r].getUB();
}

double MILPSolverCPX::getRowLower(const int & r)
{
    return (modelcon->data)[r].getLB();
}

bool MILPSolverCPX::isColumnInteger(const int & col)
{
    return (integervars.find(col) != integervars.end());
}

bool MILPSolverCPX::isColumnBinary(const int & col)
{
    map<int,bool>::const_iterator cItr = integervars.find(col);
    if (cItr == integervars.end()) return false;
    
    return (cItr->second);
}


void MILPSolverCPX::addCol(const vector<pair<int,double> > & entries, const double & lb, const double & ub, const double & start, const ColumnType & type)
{
    static const bool debug = false;
    
    if (debug) {
        cout << "Adding column to LP: ";
    }
    
    if (type == C_BOOL) {
        (*modelvar).add(IloNumVar(*env, lb, ub, ILOBOOL));
        //cout << "Adding column " << colCount << " as binary\n";
        integervars.insert(make_pair(colCount, true));
    } else if (type == C_INT) {
        (*modelvar).add(IloNumVar(*env, lb, ub, ILOINT));
        //cout << "Adding column " << colCount << " as an integer\n";
        integervars.insert(make_pair(colCount, false));
    } else {
        (*modelvar).add(IloNumVar(*env, lb, ub));
    }
    
    (*warmstart).add(IloNum(start));
        
    const int entCount = entries.size();
    for (int i = 0; i < entCount; ++i) {
        IloRange & target = modelcon->data[entries[i].first];
        target.setLinearCoef((*modelvar)[colCount], entries[i].second);
        coeffs[entries[i].first][colCount] = entries[i].second;
        if (debug) {
            if (i) cout << " + ";
            cout << entries[i].second << "*" << getRowName(entries[i].first);
        }

    }
    
            
    if (debug) {
        if (lb == -IloInfinity) {
            cout << " <= " << ub << "\n";
        } else if (ub == IloInfinity) {
            cout << " >= " << lb << "\n";
        } else if (ub == lb) {
            cout << " == " << ub << "\n";
        } else {
            cout << " in [" << lb << "," << ub << "]\n";            
        }
    }
                
    ++colCount;
    
    warmStartChangedSinceLastSolve = true;
}

void MILPSolverCPX::addRow(const vector<pair<int,double> > & entries, const double & lb, const double & ub)
{
    static const bool debug = false;
    
    if (debug) cout << "Adding row to LP: ";
    
    IloRange & newRange = modelcon->data[rowCount] = IloAdd(*model, IloRange(*env, lb,ub));
    
    map<int,double> & dest = coeffs[rowCount];
    
    const int entCount = entries.size();
    for (int i = 0; i < entCount; ++i) {
        newRange.setLinearCoef((*modelvar)[entries[i].first], entries[i].second);
        dest[entries[i].first] = entries[i].second;
        
        if (debug) {
            if (i) cout << " + ";
            cout << entries[i].second << "*" << entries[i].first;
        }
       
    }
    
    if (debug) {
        if (lb == -IloInfinity) {
            cout << " <= " << ub << "\n";
        } else if (ub == IloInfinity) {
            cout << " >= " << lb << "\n";
        } else if (ub == lb) {
            cout << " == " << ub << "\n";
        } else {
            cout << " in [" << lb << "," << ub << "]\n";            
        }
    }
    ++rowCount;    
    
    warmStartChangedSinceLastSolve = true;
    
}

void MILPSolverCPX::setMaximiseObjective(const bool & maxim)
{
    if (maxim) {
        obj->setSense(IloObjective::Maximize);
    } else {
        obj->setSense(IloObjective::Minimize);
    }
}


void MILPSolverCPX::setObjective(double * const entries)
{    
    for (int i = 0; i < colCount; ++i) {
        obj->setLinearCoef((*modelvar)[i], entries[i]);
    }
}


void MILPSolverCPX::setObjCoeff(const int & var, const double & w)
{
    obj->setLinearCoef((*modelvar)[var], w);
}

void MILPSolverCPX::clearObjective()
{
    const IloObjective::Sense direction = obj->getSense();
    
    model->remove(*obj);
    delete obj;
    
    obj = new IloObjective(*env);
    obj->setSense(direction);
    
    model->add(*obj);
}


void MILPSolverCPX::writeLp(const string & filename)
{
    try {
        cplex->exportModel(filename.c_str());
    }
    catch (IloCplex::Exception e) {
        std::cerr << "Error writing " << filename << std::endl;
        std::cerr << e << std::endl;
        exit(1);
    }
}

bool MILPSolverCPX::solve(const bool & skipPresolve)
{
    if (skipPresolve) {
        cplex->setParam(IloCplex::PreInd, 0);
    } else {
        cplex->setParam(IloCplex::PreInd, 1);
    }
    
    if (useLPWarmStart && warmStartChangedSinceLastSolve) {
        if (!integervars.empty()) {
            try {
                cplex->addMIPStart(*modelvar, *warmstart);
            }
            catch (IloCplex::Exception e) {
                cerr << "Error warm-starting CPLEX: ";
                e.print(cerr);
                exit(1);        
            }
        }
        warmStartChangedSinceLastSolve = false;
    }
    
    //cout << "Solving problem with " << cplex->getNcols() << " columns and " << cplex->getNrows() << " rows\n";
    cout << "xxxCUTxxx\n";
    bool toReturn = false;
    try {
        toReturn = cplex->solve() == IloTrue;
    }
    catch (IloCplex::Exception e) {
        cerr << "Error calling CPLEX: ";
        e.print(cerr);
        exit(1);        
    }
    cout << "yyyCUTyyy\n";
    return toReturn;
}

const double * MILPSolverCPX::getSolution()
{
    delete [] solArray;
    solArray = new double[colCount];
    
    for (int i = 0; i < colCount; ++i) {
        try {
            IloNum v = cplex->getValue((*modelvar)[i]);
            solArray[i] = v;
        }
        catch (IloAlgorithm::NotExtractedException e) {
            solArray[i] = (*modelvar)[i].getLB();
        }
    }
    
    return solArray;
}

const double * MILPSolverCPX::getSolutionRows()
{
    delete [] solArray;
    solArray = new double[rowCount];
    
    for (int i = 0; i < rowCount; ++i) {
        try {
            IloNum v = cplex->getValue((modelcon->data)[i]);
            solArray[i] = v;
        }
        catch (IloAlgorithm::NotExtractedException e) {
            solArray[i] = (modelcon->data)[i].getLB();
        }
    }
        
    return solArray;
}

const double * MILPSolverCPX::getPartialSolution(const int & from, const int & to)
{
    delete [] solArray;
    
    if (from == to) {
        solArray = 0;       
    } else {
        solArray = new double[to - from];
        
        for (int i = from; i < to; ++i) {
            try {
                IloNum v = cplex->getValue((*modelvar)[i]);
                solArray[i-from] = v;
            }
            catch (IloAlgorithm::NotExtractedException e) {
                solArray[i-from] = (*modelvar)[i].getLB();
            }
        }
    }
        
    return solArray;
}
    
double MILPSolverCPX::getSingleSolutionVariableValue(const int & col)
{
    try {
        IloNum v = cplex->getValue((*modelvar)[col]);
        return v;
    }
    catch (IloAlgorithm::NotExtractedException e) {
        return (*modelvar)[col].getLB();
    }
}

double MILPSolverCPX::getObjValue()
{
    return cplex->getObjValue();
}

void MILPSolverCPX::getRow(const int & i, vector<pair<int,double> > & entries)
{
    const int colCount = getNumCols();
    
    IloRange & currRow = (modelcon->data)[i];
    
    IloNumExprArg ne = currRow.getExpr();
    
    for(IloExpr::LinearIterator itr = static_cast<IloExpr>(ne).getLinearIterator(); itr.ok(); ++itr) {
        const int colID = itr.getVar().getId();
        for (int c = 0; c < colCount; ++c) {
            if ((*modelvar)[c].getId() == colID) {
                entries.push_back(make_pair(c, itr.getCoef()));
                break;
            }
        }        
    }    
    
}

bool MILPSolverCPX::supportsQuadratic() const
{
    return true;
}


void MILPSolverCPX::hush()
{
    
    env->setOut(env->getNullStream());
    env->setWarning(env->getNullStream());
    env->setError(env->getNullStream());
    cplex->setOut(env->getNullStream());
    cplex->setWarning(env->getNullStream());
    cplex->setError(env->getNullStream());
}

