#include "solver-cpx.h"
#include "MILPRPG.h"

extern bool cpxparametersprovided;

extern vector<pair<int, int> > intParams;
extern vector<pair<int, int> > boolParams;
extern vector<pair<int,double> > numParams;


#include <sstream>

#define IL_STD 1

#include <ilcplex/ilocplex.h>

ILOSTLBEGIN



void setILS(IloCplex * cplex)
{
    if (!Planner::MILPRPG::useParamILS) return;
 
     if (cpxparametersprovided) {
    
        {
            const int pCount = intParams.size();
            for (int p = 0; p < pCount; ++p) {
                cplex->setParam((IloCplex::IntParam) intParams[p].first, intParams[p].second);
            }
        }
        
        {
            const int pCount = boolParams.size();
            for (int p = 0; p < pCount; ++p) {
                cplex->setParam((IloCplex::BoolParam) boolParams[p].first, boolParams[p].second);
            }
        }
        
        {
            const int pCount = numParams.size();
            for (int p = 0; p < pCount; ++p) {
                cplex->setParam((IloCplex::NumParam) numParams[p].first, numParams[p].second);
            }
        }
        
        return;
    }

    if (Planner::MILPRPG::useParamILS == 4) {
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARCROSSALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARMAXCOR, -1);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_BARGROWTH, 1e+12);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARORDER, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARSTARTALG, 1);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_MEMORYEMPHASIS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPEMPHASIS, 0);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_NUMERICALEMPHASIS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FEASOPTMODE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_LPMETHOD, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CLIQUES, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_COVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DISJCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FLOWCOVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_GUBCOVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_IMPLBD, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MCFCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIRCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FLOWPATHS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_ZEROHALFCUTS, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_PROBETIME, 1e+75);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGCUTLIM, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CUTPASS, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_CUTSFACTOR, 4);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACCAND, 200);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACPASS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STRONGCANDLIM, 10);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STRONGITLIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SUBMIPNODELIM, 500);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPORDTYPE, 0);        
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_BTTOL, 0.9999);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BBINTERVAL, 7);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BRDIR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DIVETYPE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NODEFILEIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FPHEUR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_HEURFREQ, 0);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_LBHEUR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NODESEL, 1);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_MIPORDIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PRESLVND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PROBE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_RINSHEUR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPSEARCH, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STARTALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SUBALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_VARSEL, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NETFIND, 2);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NETPPRIIND, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_EPPER, 1e-6);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BNDSTRENIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_COEREDIND, 2);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DEPIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PREDUAL, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGFILL, 10);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_PRELINEAR, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PREPASS, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REDUCE, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_RELAXPREIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REPEATPRESOLVE, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SYMMETRY, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SCAIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SIFTALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CRAIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DPRIIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PERLIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SINGLIM, 10);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_PERIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PPRIIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PRICELIM, 16);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REINV, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_EPMRK, 0.01);
    } else if (Planner::MILPRPG::useParamILS == 3) {
    
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_ADVIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARCOLNZ, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_BAREPCOMP, 1e-08);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARCROSSALG, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARMAXCOR, -1);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_BARGROWTH, 1e+8);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARORDER, 2);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_BARQCPEPCOMP, 1e-07);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARSTARTALG, 4);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_MEMORYEMPHASIS, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPEMPHASIS, 2);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_NUMERICALEMPHASIS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FEASOPTMODE, 4);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_EPRELAX, 1e-06);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_LPMETHOD, 5);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CLIQUES, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_COVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DISJCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FLOWCOVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_GUBCOVERS, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_IMPLBD, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIRCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FLOWPATHS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGCUTLIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CUTPASS, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_CUTSFACTOR, 2);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACCAND, 800);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACPASS, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_POLISHTIME, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_PROBETIME, 2);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REPAIRTRIES, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STRONGCANDLIM, 5);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STRONGITLIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SUBMIPNODELIM, 2000);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPORDTYPE, 3);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_BTTOL, 0.99);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BBINTERVAL, 4);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BRDIR, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DIVETYPE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NODEFILEIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_HEURFREQ, 40);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_LBHEUR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NODESEL, 0);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_MIPORDIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PRESLVND, 2);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PROBE, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_RINSHEUR, 20);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STARTALG, 6);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SUBALG, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_VARSEL, 4);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NETFIND, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NETPPRIIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BNDSTRENIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_COEREDIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DEPIND, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PREDUAL, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGFILL, 20);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PREPASS, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PREIND, 0);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_QPMAKEPSDIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REDUCE, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_RELAXPREIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REPEATPRESOLVE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SYMMETRY, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_QPMETHOD, 4);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SCAIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SIFTALG, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CRAIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DPRIIND, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PERLIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SINGLIM, 2);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_PERIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PPRIIND, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PRICELIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REINV, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_EPINT, 1e-06);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_EPMRK, 0.01);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_EPOPT, 1e-06);
    } else if (Planner::MILPRPG::useParamILS == 2) {
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARCROSSALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARMAXCOR, -1);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_BARGROWTH, 1e+12);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARORDER, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARSTARTALG, 1);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_MEMORYEMPHASIS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPEMPHASIS, 0);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_NUMERICALEMPHASIS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FEASOPTMODE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_LPMETHOD, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CLIQUES, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_COVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DISJCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FLOWCOVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_GUBCOVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_IMPLBD, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MCFCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIRCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FLOWPATHS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_ZEROHALFCUTS, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_PROBETIME, 1e+75);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGCUTLIM, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CUTPASS, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_CUTSFACTOR, 4);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACCAND, 200);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACPASS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STRONGCANDLIM, 10);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STRONGITLIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SUBMIPNODELIM, 500);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPORDTYPE, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_BTTOL, 0.9999);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BBINTERVAL, 7);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BRDIR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DIVETYPE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NODEFILEIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FPHEUR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_HEURFREQ, 0);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_LBHEUR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NODESEL, 1);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_MIPORDIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PRESLVND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PROBE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_RINSHEUR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPSEARCH, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STARTALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SUBALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_VARSEL, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NETFIND, 2);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NETPPRIIND, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_EPPER, 1e-6);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BNDSTRENIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_COEREDIND, 2);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DEPIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PREDUAL, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGFILL, 10);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_PRELINEAR, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PREPASS, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REDUCE, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_RELAXPREIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REPEATPRESOLVE, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SYMMETRY, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SCAIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SIFTALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CRAIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DPRIIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PERLIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SINGLIM, 10);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_PERIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PPRIIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PRICELIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REINV, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_EPMRK, 0.01);        
    } else {
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARCROSSALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARMAXCOR, -1);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_BARGROWTH, 1e+12);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARORDER, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BARSTARTALG, 4);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_MEMORYEMPHASIS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPEMPHASIS, 0);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_NUMERICALEMPHASIS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FEASOPTMODE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_LPMETHOD, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CLIQUES, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_COVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DISJCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FLOWCOVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_GUBCOVERS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_IMPLBD, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MCFCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIRCUTS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FLOWPATHS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_ZEROHALFCUTS, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_PROBETIME, 1e+75);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGCUTLIM, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CUTPASS, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_CUTSFACTOR, 4);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACCAND, 200);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FRACPASS, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STRONGCANDLIM, 10);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STRONGITLIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SUBMIPNODELIM, 500);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPORDTYPE, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_BTTOL, 0.9999);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BBINTERVAL, 7);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BRDIR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DIVETYPE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NODEFILEIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_FPHEUR, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_HEURFREQ, 0);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_LBHEUR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NODESEL, 0);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_MIPORDIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PRESLVND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PROBE, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_RINSHEUR, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_MIPSEARCH, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_STARTALG, 2);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SUBALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_VARSEL, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NETFIND, 2);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_NETPPRIIND, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_EPPER, 1e-6);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_BNDSTRENIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_COEREDIND, 2);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DEPIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PREDUAL, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_AGGFILL, 10);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_PRELINEAR, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PREPASS, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REDUCE, 3);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_RELAXPREIND, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REPEATPRESOLVE, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SYMMETRY, -1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SCAIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SIFTALG, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_CRAIND, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_DPRIIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PERLIM, 1);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_SINGLIM, 10);
        cplex->setParam((IloCplex::BoolParam) CPX_PARAM_PERIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PPRIIND, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_PRICELIM, 0);
        cplex->setParam((IloCplex::IntParam) CPX_PARAM_REINV, 0);
        cplex->setParam((IloCplex::NumParam) CPX_PARAM_EPMRK, 0.01);
    }
    
}

