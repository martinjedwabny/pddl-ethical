#include "solver-cpx.h"

#include <sstream>

#define IL_STD 1

#include <ilcplex/ilocplex.h>

ILOSTLBEGIN

MILPSolver * getNewSolver()
{
    MILPSolver * const toReturn = new MILPSolverCPX();
    return toReturn;
}

const double LPinfinity = IloInfinity;

extern bool cpxparametersprovided;

extern vector<pair<int, int> > intParams;
extern vector<pair<int, int> > boolParams;
extern vector<pair<int,double> > numParams;
string argvi;

void addIntParam(const int & id, char * const val)
{
    
    istringstream cnv(val);
    
    int i;
    
    #ifndef NDEBUG
    const bool cnvSuccess = cnv >> i;
    if (!cnvSuccess) {
        cerr << "Fatal error: cannot interpret " << val << " as an integer value for " << argvi << endl;
        assert(cnvSuccess);
    }
    #else
    cnv >> i;
    #endif
    
    intParams.push_back(make_pair(id, i));
    
}

void addBoolParam(const int & id, char * const val)
{
    
    if (string(val) == "yes" || string(val) == "1") {
        boolParams.push_back(make_pair(id,1));
    } else {
        if (string(val) != "no" && string(val) != "0") {
		std::cout << "Value given for parameter " << id << " was " << val << std::endl;
	}	
        assert(string(val) == "no" || string(val) == "0");
        boolParams.push_back(make_pair(id,0));
    }
        
}

void addNumParam(const int & id, char * const val)
{
    
    istringstream cnv(val);
    
    double i;
    
    #ifndef NDEBUG
    assert(cnv >> i);
    #else
    cnv >> i;
    #endif
    
    numParams.push_back(make_pair(id, i));
    
}


void readParams(char * argv[], const int & a)
{
    cpxparametersprovided = true;
    
    for (int i = 0; i < a; i+=2) {
        argv[i] = &(argv[i][1]);
        
        argvi = string(argv[i]);
        
        if (argvi == "advance") {
            addIntParam(CPX_PARAM_ADVIND, argv[i+1]);
            continue;
        }

        if (argvi == "barrier_algorithm") {
            addIntParam(CPX_PARAM_BARALG, argv[i+1]);
            continue;
        }

        if (argvi == "barrier_colnonzeros") {
            addIntParam(CPX_PARAM_BARCOLNZ, argv[i+1]);
            continue;
        }

        if (argvi == "barrier_convergetol") {
            addNumParam(CPX_PARAM_BAREPCOMP, argv[i+1]);
            continue;
        }

        if (argvi == "barrier_crossover") {
            addIntParam(CPX_PARAM_BARCROSSALG, argv[i+1]);
            continue;
        }

        if (argvi == "barrier_limits_corrections") {
            addIntParam(CPX_PARAM_BARMAXCOR, argv[i+1]);
            continue;
        }

        if (argvi == "barrier_limits_growth") {
            addNumParam(CPX_PARAM_BARGROWTH, argv[i+1]);
            continue;
        }

        if (argvi == "barrier_ordering") {
            addIntParam(CPX_PARAM_BARORDER, argv[i+1]);
            continue;
        }

        if (argvi == "barrier_qcpconvergetol") {
            addNumParam(CPX_PARAM_BARQCPEPCOMP, argv[i+1]);
            continue;
        }

        if (argvi == "barrier_startalg") {
            addIntParam(CPX_PARAM_BARSTARTALG, argv[i+1]);
            continue;
        }

        if (argvi == "emphasis_memory") {
            addBoolParam(CPX_PARAM_MEMORYEMPHASIS, argv[i+1]);
            continue;
        }

        if (argvi == "emphasis_mip") {
            addIntParam(CPX_PARAM_MIPEMPHASIS, argv[i+1]);
            continue;
        }

        if (argvi == "emphasis_numerical") {
            addBoolParam(CPX_PARAM_NUMERICALEMPHASIS, argv[i+1]);
            continue;
        }

        if (argvi == "feasopt_mode") {
            addIntParam(CPX_PARAM_FEASOPTMODE, argv[i+1]);
            continue;
        }

        if (argvi == "feasopt_tolerance") {
            addNumParam(CPX_PARAM_EPRELAX, argv[i+1]);
            continue;
        }

        if (argvi == "lpmethod") {
            addIntParam(CPX_PARAM_LPMETHOD, argv[i+1]);
            continue;
        }

        if (argvi == "mip_cuts_cliques ") {
            addIntParam(CPX_PARAM_CLIQUES, argv[i+1]);
            continue;
        }

        if (argvi == "mip_cuts_covers") {
            addIntParam(CPX_PARAM_COVERS, argv[i+1]);
            continue;
        }

        if (argvi == "mip_cuts_disjunctive") {
            addIntParam(CPX_PARAM_DISJCUTS, argv[i+1]);
            continue;
        }

        if (argvi == "mip_cuts_flowcovers") {
            addIntParam(CPX_PARAM_FLOWCOVERS, argv[i+1]);
            continue;
        }

        if (argvi == "mip_cuts_gomory") {
            addIntParam(CPX_PARAM_FRACCUTS, argv[i+1]);
            continue;
        }

        if (argvi == "mip_cuts_gubcovers") {
            addIntParam(CPX_PARAM_GUBCOVERS, argv[i+1]);
            continue;
        }

        if (argvi == "mip_cuts_implied") {
            addIntParam(CPX_PARAM_IMPLBD, argv[i+1]);
            continue;
        }
        
        if (argvi == "mip_cuts_mcfcut") {
            addIntParam(CPX_PARAM_MCFCUTS, argv[i+1]);
            continue;
        }
                
        if (argvi == "mip_cuts_mircut") {
            addIntParam(CPX_PARAM_MIRCUTS, argv[i+1]);
            continue;
        }

        if (argvi == "mip_cuts_pathcut") {
            addIntParam(CPX_PARAM_FLOWPATHS, argv[i+1]);
            continue;
        }
        
        if (argvi == "mip_cuts_zerohalfcut") {
            addIntParam(CPX_PARAM_ZEROHALFCUTS, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limits_aggforcut") {
            addIntParam(CPX_PARAM_AGGCUTLIM, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limits_cutpasses") {
            addIntParam(CPX_PARAM_CUTPASS, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limits_cutsfactor") {
            addNumParam(CPX_PARAM_CUTSFACTOR, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limits_gomorycand") {
            addIntParam(CPX_PARAM_FRACCAND, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limits_gomorypass") {
            addIntParam(CPX_PARAM_FRACPASS, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limits_polishtime") {
            addNumParam(CPX_PARAM_POLISHTIME, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limit_probetime") {
            addNumParam(CPX_PARAM_PROBETIME, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limits_repairtries") {
            addIntParam(CPX_PARAM_REPAIRTRIES, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limits_strongcand") {
            addIntParam(CPX_PARAM_STRONGCANDLIM, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limits_strongit") {
            addIntParam(CPX_PARAM_STRONGITLIM, argv[i+1]);
            continue;
        }

        if (argvi == "mip_limits_submipnodelim") {
            addIntParam(2212, argv[i+1]);
            continue;
        }

        if (argvi == "mip_ordertype") {
            addIntParam(CPX_PARAM_MIPORDTYPE, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_backtrack") {
            addNumParam(CPX_PARAM_BTTOL, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_bbinterval") {
            addIntParam(CPX_PARAM_BBINTERVAL, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_branch") {
            addIntParam(CPX_PARAM_BRDIR, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_dive") {
            addIntParam(CPX_PARAM_DIVETYPE, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_file") {
            addIntParam(CPX_PARAM_NODEFILEIND, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_fpheur") {
            addIntParam(CPX_PARAM_FPHEUR, argv[i+1]);
            continue;
        }
        
        if (argvi == "mip_strategy_heuristicfreq") {
            addIntParam(CPX_PARAM_HEURFREQ, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_lbheur") {
            addBoolParam(CPX_PARAM_LBHEUR, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_nodeselect") {
            addIntParam(CPX_PARAM_NODESEL, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_order") {
            addBoolParam(CPX_PARAM_MIPORDIND, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_presolvenode") {
            addIntParam(CPX_PARAM_PRESLVND, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_probe") {
            addIntParam(CPX_PARAM_PROBE, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_rinsheur") {
            addIntParam(CPX_PARAM_RINSHEUR, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_search") {
            addIntParam(CPX_PARAM_MIPSEARCH, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_startalgorithm") {
            addIntParam(CPX_PARAM_STARTALG, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_subalgorithm") {
            addIntParam(CPX_PARAM_SUBALG, argv[i+1]);
            continue;
        }

        if (argvi == "mip_strategy_variableselect") {
            addIntParam(CPX_PARAM_VARSEL, argv[i+1]);
            continue;
        }

        if (argvi == "network_netfind") {
            addIntParam(CPX_PARAM_NETFIND, argv[i+1]);
            continue;
        }

        if (argvi == "network_pricing") {
            addIntParam(CPX_PARAM_NETPPRIIND, argv[i+1]);
            continue;
        }

        if (argvi == "perturbation_constant") {
            addNumParam(CPX_PARAM_EPPER, argv[i+1]);
        }

        if (argvi == "preprocessing_aggregator") {
            addIntParam(CPX_PARAM_AGGIND, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_boundstrength") {
            addIntParam(CPX_PARAM_BNDSTRENIND, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_coeffreduce") {
            addIntParam(CPX_PARAM_COEREDIND, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_dependency") {
            addIntParam(CPX_PARAM_DEPIND, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_dual") {
            addIntParam(CPX_PARAM_PREDUAL, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_fill") {
            addIntParam(CPX_PARAM_AGGFILL, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_linear") {
            addBoolParam(CPX_PARAM_PRELINEAR, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_numpass") {
            addIntParam(CPX_PARAM_PREPASS, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_presolve") {
            addBoolParam(CPX_PARAM_PREIND, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_qpmakepsd") {
            addBoolParam(CPX_PARAM_QPMAKEPSDIND, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_reduce") {
            addIntParam(CPX_PARAM_REDUCE, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_relax") {
            addIntParam(CPX_PARAM_RELAXPREIND, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_repeatpresolve") {
            addIntParam(CPX_PARAM_REPEATPRESOLVE, argv[i+1]);
            continue;
        }

        if (argvi == "preprocessing_symmetry") {
            addIntParam(CPX_PARAM_SYMMETRY, argv[i+1]);
            continue;
        }

        if (argvi == "qpmethod") {
            addIntParam(CPX_PARAM_QPMETHOD, argv[i+1]);
            continue;
        }

        if (argvi == "read_scale") {
            addIntParam(CPX_PARAM_SCAIND, argv[i+1]);
            continue;
        }

        if (argvi == "sifting_algorithm") {
            addIntParam(CPX_PARAM_SIFTALG, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_crash") {
            addIntParam(CPX_PARAM_CRAIND, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_dgradient") {
            addIntParam(CPX_PARAM_DPRIIND, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_limits_perturbation") {
            addIntParam(CPX_PARAM_PERLIM, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_limits_singularity") {
            addIntParam(CPX_PARAM_SINGLIM, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_perturbation") {
            addBoolParam(CPX_PARAM_PERIND, argv[i+1]);
            ++i;
            continue;
        }
        
        if (argvi == "simplex_perturbation_switch") {
            addBoolParam(CPX_PARAM_PERIND, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_pgradient") {
            addIntParam(CPX_PARAM_PPRIIND, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_pricing") {
            addIntParam(CPX_PARAM_PRICELIM, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_refactor") {
            addIntParam(CPX_PARAM_REINV, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_tolerances_feasibility") {
            addNumParam(CPX_PARAM_EPINT, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_tolerances_markowitz") {
            addNumParam(CPX_PARAM_EPMRK, argv[i+1]);
            continue;
        }

        if (argvi == "simplex_tolerances_optimality") {
            addNumParam(CPX_PARAM_EPOPT, argv[i+1]);
            continue;
        }

    }
}
