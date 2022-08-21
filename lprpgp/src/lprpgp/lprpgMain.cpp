/************************************************************************
 * Copyright 2008, 2009, Strathclyde Planning Group,
 * Department of Computer and Information Sciences,
 * University of Strathclyde, Glasgow, UK
 * http://planning.cis.strath.ac.uk/
 *
 * Maria Fox, Richard Howey and Derek Long - Code from VAL
 * Stephen Cresswell - PDDL Parser
 * Andrew Coles, Amanda Coles - Code for LPRPG
 *
 * This file is part of LPRPG.
 *
 * LPRPG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * LPRPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LPRPG.  If not, see <http://www.gnu.org/licenses/>.
 *
 ************************************************************************/

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ptree.h>
#include <assert.h>
#include <FlexLexer.h>
#include "instantiation.h"
#include "SimpleEval.h"
#include "DebugWriteController.h"
#include "typecheck.h"
#include "TIM.h"
#include "FuncAnalysis.h"
#include <unistd.h>

//#include "graphconstruct.h"
#include "RPGBuilder.h"
//#include "PartialPlan.h"
#include "Decomposition.h"
#include "FFSolver.h"
#include "GlobalSchedule.h"
#include "PreferenceHandler.h"

#include <sys/times.h>

#include "MILPRPG.h"
#include "solver.h"

using std::ifstream;
using std::cerr;

using namespace TIM;
using namespace Inst;
using namespace VAL;
using namespace Planner;

namespace VAL {
bool ContinueAnyway;
bool ErrorReport;
bool InvariantWarnings;
bool LaTeX;
//bool LaTeXRecord;
bool makespanDefault;
};

void usage()
{
        cout << "LPRPG\n";
        cout << "By releasing this code we imply no warranty as to its reliability\n";
        cout << "and its use is entirely at your own risk.\n\n";
        cout << "Usage: lprpg [OPTIONS] domainfile problemfile\n\n";
        cout << "Options are: \n\n";
	cout << "\t" << "-citation" << "\t" << "Display citation to relevant conference paper (ICAPS, 2008);\n";
	cout << "\t" << "-plananyway" << "\t" << "Force planning, even on problems with non-producer-consumer numerics;\n";
        cout << "\t" << "-e" << "\t" << "Use standard EHC instead of steepest descent;\n";
	cout << "\t" << "-E" << "\t" << "Skip HC - go straight to best-first search;\n";
    cout << "\t" << "-K" << "\t" << "Apply action elimination to each new best solution found;\n";
    cout << "\t" << "-G" << "\t" << "Don't put the numeric goal conjunct formula into the LP;\n";
    cout << "\t" << "-P" << "\t" << "Don't put the propositional goal formula in the LP;\n";    
    cout << "\t" << "-L" << "\t" << "Don't put propositional landmarks in the LP;\n";    
    cout << "\t" << "-I<n>" << "\t" << "Control use of parameters learnt by ParamILS:\n";
    cout << "\t\t\t\t0 - do not set parameters\n";
    cout << "\t\t\t\t1-4 - use a hardwired parameter set (default: 1)\n";
    cout << "\t" << "-w<n>" << "\t" << "Control the extent to which integers are used in the LP (from 0 to 4);\n";
    cout << "\t" << "-f<n>" << "\t" << "Set the factor used to generate the geometric series for weighting actions in the LP;\n";
    cout << "\t" << "-F[max|sum]" << "\t" << "Use hmax/hadd cost propagation in the RPG;\n";
    cout << "\t" << "-r" << "\t" << "Only force the LP to wholly satisfy infrastructure numeric preconditions, rather than wholly satisfying all preconditions;\n";
    cout << "\t" << "-x" << "\t" << "Never use presolving in the LP;\n";
    cout << "\t" << "-B" << "\t" << "Do not recognise infrastructure propositional preconditions in the LP;\n";
    cout << "\t" << "-p" << "\t" << "Add all propositional preconditions and effects to the LP;\n";
    cout << "\t" << "-U" << "\t" << "Disable LP warm-starting based on sensible initial column values;\n";    
	cout << "\t" << "-h" << "\t" << "Disable helpful-action pruning.\n\n";
    
};

int readAndTestPlan(const char *, const bool &);

/*static bool handle_binary(char * argva,char * val,int & var,const char * toMatch,const int & bitmask)
{

	if (!strcmp(argva,toMatch)) {
		if (!strcmp(val,"yes")) {
			var = var | bitmask;
		} else {
			var = var & ~bitmask;
		}
		return true;
	} else {
		return false;
	}

};*/

/*
static void paramILS(char * argv[], int & a)
{

	static int & basiscrash = MILPRPG::basiscrash;
	static int & pivoting_rule = MILPRPG::pivoting_rule;
	static int & pivoting_mode = MILPRPG::pivoting_mode;
	static int & presolve = MILPRPG::presolve;
	static int & presolveloops = MILPRPG::presolveloops;
	static int & simplextype = MILPRPG::simplextype;
	static int & bb_floorfirst = MILPRPG::bb_floorfirst;
	static int & bb_rule = MILPRPG::bb_rule;
	static int & bb_mode = MILPRPG::bb_mode;


	char* val = argv[a + 1];
	argv[a] = argv[a] + 2; // begins with '-H'

	if (!strcmp(argv[a],"basis_crash")) {
		if (!strcmp(val,"CRASH_NONE")) {
			basiscrash = CRASH_NONE;
		} else if (!strcmp(val,"CRASH_MOSTFEASIBLE")) {
			basiscrash = CRASH_MOSTFEASIBLE;
		} else if (!strcmp(val,"CRASH_LEASTDEGENERATE")) {
			basiscrash = CRASH_LEASTDEGENERATE;
		} else {
			cout << "Invalid basis_crash option: " << val << "\n";
			exit(1);
		}
	} else if (!strcmp(argv[a],"pivoting")) {
		if (!strcmp(val,"PRICER_FIRSTINDEX")) {
			pivoting_rule = PRICER_FIRSTINDEX;
		} else if (!strcmp(val,"PRICER_DANTZIG")) {
			pivoting_rule = PRICER_DANTZIG;
		} else if (!strcmp(val,"PRICER_DEVEX")) {
			pivoting_rule = PRICER_DEVEX;
		} else if (!strcmp(val,"PRICER_STEEPESTEDGE")) {
			pivoting_rule = PRICER_STEEPESTEDGE;
		} else {
			cout << "Invalid pivoting rule option: " << val << "\n";
			exit(1);
		}
	} else if (!strcmp(argv[a], "pivoting_primalfallback")) {
		if (!strcmp(val, "yes")) {
			pivoting_mode = pivoting_mode | PRICE_PRIMALFALLBACK;
		} else {
			pivoting_mode = pivoting_mode & ~PRICE_PRIMALFALLBACK;
		}
	} else if (!strcmp(argv[a], "pivoting_pricemultiple")) {
		if (!strcmp(val, "yes")) {
			pivoting_mode = pivoting_mode | PRICE_MULTIPLE;
		} else {
			pivoting_mode = pivoting_mode & ~PRICE_MULTIPLE;
		}
	} else if (!strcmp(argv[a], "pivoting_pricepartial")) {
		if (!strcmp(val, "yes")) {
			pivoting_mode = pivoting_mode | PRICE_PARTIAL;
		} else {
			pivoting_mode = pivoting_mode & ~PRICE_PARTIAL;
		}
	} else if (!strcmp(argv[a], "pivoting_priceadaptive")) {
		if (!strcmp(val, "yes")) {
			pivoting_mode = pivoting_mode | PRICE_ADAPTIVE;
		} else {
			pivoting_mode = pivoting_mode & ~PRICE_ADAPTIVE;
		}
	} else if (!strcmp(argv[a], "pivoting_pricerandomize")) {
		if (!strcmp(val, "yes")) {
			pivoting_mode = pivoting_mode | PRICE_RANDOMIZE;
		} else {
			pivoting_mode = pivoting_mode & ~PRICE_RANDOMIZE;
		}
	} else if (!strcmp(argv[a], "pivoting_priceautopartial")) {
		if (!strcmp(val, "yes")) {
			pivoting_mode = pivoting_mode | PRICE_AUTOPARTIAL;
		} else {
			pivoting_mode = pivoting_mode & ~PRICE_AUTOPARTIAL;
		}
	} else if (!strcmp(argv[a], "pivoting_priceloopleft")) {
		if (!strcmp(val, "yes")) {
			pivoting_mode = pivoting_mode | PRICE_LOOPLEFT;
			pivoting_mode = pivoting_mode & ~PRICE_LOOPALTERNATE;
		} else if (!strcmp(val, "no")) {
			pivoting_mode = pivoting_mode & ~PRICE_LOOPLEFT;
			pivoting_mode = pivoting_mode & ~PRICE_LOOPALTERNATE;
		} else {
			pivoting_mode = pivoting_mode & ~PRICE_LOOPLEFT;
			pivoting_mode = pivoting_mode | PRICE_LOOPALTERNATE;
		}
	} else if (!strcmp(argv[a], "pivoting_harristwopass")) {
		if (!strcmp(val, "yes")) {
			pivoting_mode = pivoting_mode | PRICE_HARRISTWOPASS;
		} else {
			pivoting_mode = pivoting_mode & ~PRICE_HARRISTWOPASS;
		}
	} else if (!strcmp(argv[a], "pivoting_truenorminit")) {
		if (!strcmp(val, "yes")) {
			pivoting_mode = pivoting_mode | PRICE_TRUENORMINIT;
		} else {
			pivoting_mode = pivoting_mode & ~PRICE_TRUENORMINIT;
		}
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_ROWS",PRESOLVE_ROWS)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_COLS",PRESOLVE_COLS)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_LINDEP",PRESOLVE_LINDEP)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_SOS",PRESOLVE_SOS)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_REDUCEMIP", PRESOLVE_REDUCEMIP)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_KNAPSACK", PRESOLVE_KNAPSACK)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_ELIMEQ2", PRESOLVE_ELIMEQ2)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_IMPLIEDFREE", PRESOLVE_IMPLIEDFREE)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_REDUCEGCD", PRESOLVE_REDUCEGCD)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_PROBEFIX", PRESOLVE_PROBEFIX)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_PROBEREDUCE", PRESOLVE_PROBEREDUCE)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_ROWDOMINATE", PRESOLVE_ROWDOMINATE)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_COLDOMINATE", PRESOLVE_COLDOMINATE)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_MERGEROWS", PRESOLVE_MERGEROWS)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_IMPLIEDSLK", PRESOLVE_IMPLIEDSLK)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_COLFIXDUAL", PRESOLVE_COLFIXDUAL)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_BOUNDS", PRESOLVE_BOUNDS)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_DUALS", PRESOLVE_DUALS)) {
	} else if (handle_binary(argv[a],val,presolve,"presolve_PRESOLVE_SENSDUALS", PRESOLVE_SENSDUALS)) {
	} else if (!strcmp(argv[a],"presolveloops")) {
		presolveloops = atoi(val);
	} else if (!strcmp(argv[a], "simplextype")) {
		if (!strcmp(val, "SIMPLEX_PRIMAL_PRIMAL")) {
			simplextype = SIMPLEX_PRIMAL_PRIMAL;
		} else if (!strcmp(val, "SIMPLEX_DUAL_PRIMAL")) {
			simplextype = SIMPLEX_DUAL_PRIMAL;
		} else if (!strcmp(val, "SIMPLEX_PRIMAL_DUAL")) {
			simplextype = SIMPLEX_PRIMAL_DUAL;
		} else if (!strcmp(val, "SIMPLEX_DUAL_DUAL")) {
			simplextype = SIMPLEX_DUAL_DUAL;
		}
	} else if (!strcmp(argv[a], "bb_floorfirst")) {
		if (!strcmp(val, "BRANCH_CEILING")) {
			bb_floorfirst = BRANCH_CEILING;
		} else if (!strcmp(val, "BRANCH_FLOOR")) {
			bb_floorfirst = BRANCH_FLOOR;
		} else if (!strcmp(val, "BRANCH_AUTOMATIC")) {
			bb_floorfirst = BRANCH_AUTOMATIC;
		}
	} else if (!strcmp(argv[a], "bb_rule")) {
		if (!strcmp(val, "NODE_FIRSTSELECT")) {
			bb_rule = NODE_FIRSTSELECT;
		} else if (!strcmp(val, "NODE_GAPSELECT")) {
			bb_rule = NODE_GAPSELECT;
		} else if (!strcmp(val, "NODE_RANGESELECT")) {
			bb_rule = NODE_RANGESELECT;
		} else if (!strcmp(val, "NODE_FRACTIONSELECT")) {
			bb_rule = NODE_FRACTIONSELECT;
		} else if (!strcmp(val, "NODE_PSEUDOCOSTSELECT")) {
			bb_rule = NODE_PSEUDOCOSTSELECT;
		} else if (!strcmp(val, "NODE_PSEUDONONINTSELECT")) {
			bb_rule = NODE_PSEUDONONINTSELECT;
		} else if (!strcmp(val, "NODE_PSEUDORATIOSELECT")) {
			bb_rule = NODE_PSEUDORATIOSELECT;
		}
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_WEIGHTREVERSEMODE", NODE_WEIGHTREVERSEMODE)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_BRANCHREVERSEMODE", NODE_BRANCHREVERSEMODE)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_GREEDYMODE",        NODE_GREEDYMODE)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_PSEUDOCOSTMODE",    NODE_PSEUDOCOSTMODE)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_DEPTHFIRSTMODE",    NODE_DEPTHFIRSTMODE)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_RANDOMIZEMODE",     NODE_RANDOMIZEMODE)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_DYNAMICMODE",       NODE_DYNAMICMODE)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_RESTARTMODE",       NODE_RESTARTMODE)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_BREADTHFIRSTMODE",  NODE_BREADTHFIRSTMODE)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_AUTOORDER",         NODE_AUTOORDER)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_RCOSTFIXING",       NODE_RCOSTFIXING)) {
	} else if (handle_binary(argv[a],val,bb_mode,"bbrule_NODE_STRONGINIT",        NODE_STRONGINIT)) {
	} else {
		cout << "Unknown command line option " << argv[a] << "=" << val << "\n";
		exit(1);
	}	
}
*/

int main(int argc,char * argv[])
{

	int argcount = 1;

	FF::ignorePenalties = true;
	FF::initialAdvancing = false;
	FF::disableWaits = true;
	FF::neverReuseOrWait = true;
	FF::steepestDescent = false;
	GlobalSchedule::checkOpenStartsOnly = true;
        FF::incrementalExpansion = false;
        FF::justWaitIncr = true;
	FF::invariantRPG = false;
	FF::timeWAStar = false;
    MILPRPG::residualEverything = true;
    
    
    while(argcount < argc && argv[argcount][0] == '-') {

		string remainder(&(argv[argcount][1]));
		if (remainder == "citation") {
			cout << "@CONFERENCE{lprpgicaps08,\n";
			cout << "\tauthor = \"A. I. Coles and M. Fox and D. Long and A. J. Smith\",\n";
			cout << "\ttitle = \"A Hybrid Relaxed Planning Graph-LP Heuristic for Numeric Planning Domains\",\n";
			cout << "\tbooktitle = \"Proceedings of the Eighteenth International Conference on Automated Planning and Scheduling (ICAPS 08)\",\n";
			cout << "\tyear = \"2008\",\n";
			cout << "\tmonth = \"September\",\n";
			cout << "}\n";
		} else if (remainder == "plananyway") {
			RPGBuilder::planAnyway = true;
		} else {

	
		switch(argv[argcount][1]) {
            case '1':
                FF::allowDualOpenList = false;
                break;
            case '2': {
                RPGBuilder::switchInCheaperAchievers = false;
                break;
            }
            case 'a':
                FF::initialAdvancing = false;
                break;
            case 'b':
                FF::bestFirstSearch = false;
                break;
            case 'e':
                FF::steepestDescent = true;
                break;
            case 'E':
                FF::skipEHC = true;
                break;
            case 'c':
                FF::disablePareto = 2;
                break;
            case 'C':
                FF::disablePareto = 1;
                break;
            case 'f':
            {
                if(argv[argcount][2] == 0){
                    usage();
                    exit(1);
                }
                MILPRPG::layerFactor = atof(&(argv[argcount][2]));                    
                break;
            }                
            case 'F':
            {
                if(argv[argcount][2] == 0){
                    usage();
                    exit(1);
                }
                
                if (!(strcmp(&(argv[argcount][2]), "sum"))) {
                    RPGBuilder::costPropagationMethod = E_SUMCOST;
                } else if (!(strcmp(&(argv[argcount][2]), "max"))) {
                    RPGBuilder::costPropagationMethod = E_MAXCOST;
                } else {
                    cout << "Unknown cost propagation method '" << &(argv[argcount][2]) << "', should be sum or max.\n";
                    usage();
                    exit(1);
                }
                RPGBuilder::useTheCostsInTheLP = true;
                                
                break;
            }
            case 'g': {
                {
                    if(argv[argcount][2] != 0){
                        MILPRPG::secondaryIntegerLevel = atoi(&(argv[argcount][2]));
                    } else {                    
                        MILPRPG::secondaryIntegerLevel = 1;
                    }
                }
                break;
            }
            case 'h':
                FF::helpfulActions = false;
                break;
            case 'i':
                FF::firstImprover = true;
                break;
            case 'D':
                PreferenceHandler::preferenceDebug = true;
                break;
            case 'P':
                MILPRPG::addProps = false;
                break;
            case 'L':
                MILPRPG::addLandmarks = false;
                break;
            case 'l':
                MILPRPG::useLocalLandmarks = true;
                break;
            case 'G':
                MILPRPG::addNumGoalConjunct = false;
                break;
            case 'A':
                MILPRPG::alternativeObjective = true;
                break;
            case 'B':
                MILPRPG::recogniseBootstrappingPropositions = false;
                break;
            case 'j':
                FF::relaxedGoalJump = false;
                break;
            case 'm': {
                RPGBuilder::useMetricRPG = true;
                MILPRPG::addNumGoalConjunct = false;
                break;
            }
            case 'M':
                FF::multipleHelpfuls = false;
                break;
            case 'n':
                FF::ignorePenalties = false;
                break;
            case 'O':
                FF::startsBeforeEnds = false;
                break;
            case 'p':
                MILPRPG::ensureAllPropositionalPreconditionsAreMet = true;
                break;
            case 'r':
                MILPRPG::residualEverything = false;
                break;
            case 'R':
                MILPRPG::residualEverything = true;
                break;
            case 'S':
                MILPRPG::useSecondaryObjective = true;
                break;
            case 's':
                RPGBuilder::doNotApplyActionsThatLookTooExpensive = false;
                break;
            case 'T':
                FF::tsChecking = false;
                FF::invariantRPG = false;
                break;
            case 'U': {
                useLPWarmStart = false;
                break;
            }
            case 'v':
                {
                    if(argv[argcount][2] != 0){
                        GlobalSchedule::globalVerbosity = atoi(&(argv[argcount][2]));
                    } else {
                        GlobalSchedule::globalVerbosity = 1;
                    }
                }
                break;
            case 'V':
                {
                    if(argv[argcount][2] == 0){
                        usage();
                        exit(1);
                    }
                    FF::capOnPreferenceCost = atof(&(argv[argcount][2]));
                    break;
                }
            case 'w':
            {
                if(argv[argcount][2] == 0){
                    usage();
                    exit(1);
                }
                MILPRPG::integerLevel= atoi(&(argv[argcount][2]));
                break;
            }
            case 'W':
                FF::doubleU = atof(&(argv[argcount][2]));
                break;
            case 'x':
            {
                MILPRPG::neverUsePresolving = true;                    
                break;
            }                
            case 'X': {
                RPGBuilder::postFilter = false;
                break;
            }
            case 'I':
                {
                    if(argv[argcount][2] != 0){
                        MILPRPG::useParamILS = atoi(&(argv[argcount][2]));
                    } else {                    
                        MILPRPG::useParamILS = 1;
                    }
                }
                break;
            case 'Y': {
                FF::allowDualOpenList = false;
                FF::useWeightedSumWithPrefCost = true;                
                if(argv[argcount][2] != 0){
                    FF::prefWeightInWeightedSum = atof(&(argv[argcount][2]));
                }
                break;
            }
            case 'H':
                {
                    readParams(argv, argcount);
                    //paramILS(argv, argcount);
                    ++argcount;
                }
                break;
            #ifdef FFSEARCHDEBUGHOOKS
            case 'z':
                {
                    FF::actuallyPlanGivenPreviousSolution = true;
                    break;
                }
            #endif
		default:
			cout << "Unrecognised command-line switch '" << argv[argcount][1] << "'\n";
			usage();
			exit(0);
	
		};
		}
		++argcount;
	};

	if (argcount + 2 > argc) {
		usage();
		exit(0);
	}

	performTIMAnalysis(&argv[argcount]);

    argcount += 2;
            
    const char * solutionFile = 0;
    
    if (argcount < argc) {
        if (argv[argcount][0] != '-') {
            solutionFile = argv[argcount];
            ++argcount;
        }
        
        #ifndef NDEBUG
        GlobalSchedule::planFilename = solutionFile;
        #endif
    }
    
    if (argcount < argc) {
        readParams(&argv[argcount], argc - argcount);
    }

	cout << std::setprecision(3) << std::fixed;

	RPGBuilder::initialise();

	if (RPGBuilder::getTILVec().size() > 0) {
		cout << "Timed initial literals detected, using extended temporal RPG analysis\n";
		FF::invariantRPG = true;
		FF::timeWAStar = true;
	}

	Decomposition::performDummyDecomposition();
	GlobalSchedule::initialise();
	const int spCount = Decomposition::howMany();
	bool carryOn = true;
	
    #ifndef FFSEARCHDEBUGHOOKS    
    if (solutionFile) {
        return readAndTestPlan(solutionFile, false);
    }
    #else
    if (solutionFile) {
        if (FF::actuallyPlanGivenPreviousSolution) {
            const int rv = readAndTestPlan(solutionFile, true);
            if (rv) return rv;
        } else {
            return readAndTestPlan(solutionFile, false);
        }
    }
    #endif

	while (carryOn) {
	
		carryOn = false;
//		bool reachesAllGoals = true;
		
		for (int i = 0; i < spCount; ++i) {
			if (GlobalSchedule::globalVerbosity & 1) cout << "Planning for subproblem " << i << "\n";
			//list<FFEvent> * const oldSoln = GlobalSchedule::getCurrentManipulator()->reset(i);
			//double toBeat = (oldSoln ? GlobalSchedule::currentEvaluator->heuristicPenaltiesOldSchedule(*GlobalSchedule::currentSchedule, i, true) : 0.0);
            
            list<FFEvent> * const oldSoln = 0;
            double toBeat = 0.0;
			bool solved = false;
            list<FFEvent> * spSoln;
			{
				
                bool reachesGoals;
                int relaxedStepCount = 0;
                
                LiteralSet initialState;
                vector<double> initialFluents;
                
                RPGBuilder::getInitialState(initialState, initialFluents);
                
                spSoln = FF::solveSubproblemWRTSchedule(initialState, initialFluents, Decomposition::getSubproblem(i), INT_MAX, reachesGoals, i, oldSoln, toBeat, relaxedStepCount);
                if (spSoln) {
                    solved = true;
                }
				
			}
			if (solved) {
				tms refReturn;
                		times(&refReturn);				
				cout << ";;;; Solution Found\n";
                cout << "; States evaluated: " << RPGBuilder::statesEvaluated  << "\n;\n";
                
                double secs = ((double)refReturn.tms_utime + (double)refReturn.tms_stime) / ((double) sysconf(_SC_CLK_TCK));

				int twodp = (int) (secs * 100.0);
				int wholesecs = twodp / 100;
				int centisecs = twodp % 100;
		
				cout << "; Time " << wholesecs << ".";
				if (centisecs < 10) cout << "0";
				cout << centisecs << "\n";
				
                cout << ";\n";
                list<FFEvent>::iterator planItr = spSoln->begin();
                const list<FFEvent>::iterator planEnd = spSoln->end();
                
                for (int i = 0; planItr != planEnd; ++planItr) {            
                    if (planItr->time_spec == VAL::E_AT_START) {
                        cout << (i * 1.001) << ": " << *(planItr->action) << " [1.000]\n";
                        ++i;
                    }
                }
				return 0;
			} else {
				cout << ";; Problem unsolvable!\n";
				tms refReturn;
                		times(&refReturn);				
				cout << ";;;; Solution Found\n";
                		double secs = ((double)refReturn.tms_utime + (double)refReturn.tms_stime) / ((double) sysconf(_SC_CLK_TCK));

				int twodp = (int) (secs * 100.0);
				int wholesecs = twodp / 100;
				int centisecs = twodp % 100;
		
				cout << "; Time " << wholesecs << ".";
				if (centisecs < 10) cout << "0";
				cout << centisecs << "\n";
				return 1;
			}
	
		} 
		
	}
	
	tms refReturn;
	times(&refReturn);				
	cout << ";;;; Solution Found\n";
	double secs = ((double)refReturn.tms_utime + (double)refReturn.tms_stime) / ((double) sysconf(_SC_CLK_TCK));

	int twodp = (int) (secs * 100.0);
	int wholesecs = twodp / 100;
	int centisecs = twodp % 100;

	cout << "; Time " << wholesecs << ".";
	if (centisecs < 10) cout << "0";
	cout << centisecs << "\n";

	return 0;
}

extern int yyparse();
extern int yydebug;

namespace VAL {
    extern yyFlexLexer* yfl;
};

int readAndTestPlan(const char * filename, const bool & prime) {
    
    ifstream * const current_in_stream = new ifstream(filename);
    if (!current_in_stream->good())
    {
        cout << "Exiting: could not open plan file " << filename << "\n";
        exit(1);
    }
        
    VAL::yfl = new yyFlexLexer(current_in_stream,&cout);
    yyparse();
    
    VAL::plan * const the_plan = dynamic_cast<VAL::plan*>(top_thing);
    
    delete VAL::yfl;
    delete current_in_stream;
    
    
    
    if(!the_plan)
    {
        cout << "Exiting: failed to load plan " << filename << "\n";
        exit(1);
    };
    
    if (!theTC->typecheckPlan(the_plan)) {
        cout << "Exiting: error when type-checking plan " << filename << "\n";
        exit(1);
    }
    
    list<int> planSteps;
    
    pc_list<plan_step*>::const_iterator planItr = the_plan->begin();
    const pc_list<plan_step*>::const_iterator planEnd = the_plan->end();
    
    for (int idebug = 0, i=0; planItr != planEnd; ++planItr, ++i, ++idebug) {
        plan_step* const currStep = *planItr;
        
        instantiatedOp * const currOp = instantiatedOp::findInstOp(currStep->op_sym, currStep->params->begin(), currStep->params->end());
        
        if (!currOp) {
            //instantiatedOp * const debugOp = instantiatedOp::getInstOp(currStep->op_sym, currStep->params->begin(), currStep->params->end());
            cout << "Exiting: step " << idebug << " in the input plan uses the action ";
            cout << "(" << currStep->op_sym->getName();
            
            const_symbol_list::const_iterator cslItr = currStep->params->begin();
            const const_symbol_list::const_iterator cslEnd = currStep->params->end();
            for (; cslItr != cslEnd; ++cslItr) {
                cout << " " << (*cslItr)->getName();
            }
            cout << "), which has not been instantiated.\n";
            return 1;
        }
        const int ID = currOp->getID();
        if (RPGBuilder::rogueActions[ID]) {            
            cout << "Exiting: step " << idebug << " in the input plan uses the action " << *(currOp) << ", which has been pruned on the basis that it's never applicable\n";
            return 1;
        }
        planSteps.push_back(ID);
    }
    
    cout << "All necessary plan steps have been instantiated\n";
    
    return FF::testExistingSolution(planSteps,prime);
   
}
