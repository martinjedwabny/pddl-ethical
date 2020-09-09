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

#include "MILPRPG.h"
#include "RPGBuilder.h"

#include <sstream>
#include <string>
#include "PreferenceHandler.h"
#include "landmarksanalysis.h"

using std::ostringstream;
using std::string;
using std::endl;

namespace Planner {

vector<bool> MILPRPG::fluentAction;
vector<bool> MILPRPG::everAssigned;
vector<list<MILPRPG::ActionEffectUponVariable> > MILPRPG::preprocessedData;
int MILPRPG::actCount;
vector<int> MILPRPG::goesToRealAction;
vector<double> MILPRPG::preferEarlier;

vector<double> MILPRPG::maxNeeded;
vector<double> MILPRPG::minNeeded;
vector<MILPRPG::GoalConstraint> MILPRPG::goalConstraints;
vector<vector<int> > MILPRPG::propositionalGoalConstraints;
vector<vector<int> > MILPRPG::propositionalGoalNotEncoded;
vector<int> MILPRPG::propositionalGoals;
vector<MILPRPG::GoalBound> MILPRPG::goalVarBounds;
    
bool MILPRPG::addProps = true;
bool MILPRPG::addLandmarks = true;
        

        int MILPRPG::basiscrash = CRASH_NONE;
        int MILPRPG::pivoting_rule = PRICER_DEVEX;
        int MILPRPG::pivoting_mode = PRICE_ADAPTIVE;
        int MILPRPG::presolve = 3;
//        int MILPRPG::presolve = PRESOLVE_NONE;
        int MILPRPG::presolveloops = -1;
        int MILPRPG::simplextype = SIMPLEX_DUAL_PRIMAL;
        int MILPRPG::bb_floorfirst = BRANCH_AUTOMATIC;
        int MILPRPG::bb_rule = NODE_PSEUDONONINTSELECT;
        int MILPRPG::bb_mode = + NODE_GREEDYMODE + NODE_DYNAMICMODE + NODE_RCOSTFIXING;


MILPRPG::MILPRPG(MinimalState * f, const vector<double> & layerZero)
: varCount(layerZero.size()), colCount(varCount), layerCount(0), startingState(f), goalLP(0)
{
	


	if (!varCount) return;

	static bool donePreprocessing = false;

	if (!donePreprocessing) {

		everAssigned = vector<bool>(varCount);
		maxNeeded = vector<double>(varCount);
		minNeeded = vector<double>(varCount);
		for (int v = 0; v < varCount; ++v) everAssigned[v] = false;
		for (int v = 0; v < varCount; ++v) {
			maxNeeded[v] = 0.0;
			//if (layerZero[v] > 0.0) maxNeeded[v] = 0.0;
		}
		for (int v = 0; v < varCount; ++v) {
			minNeeded[v] = DBL_MAX;
			//if (layerZero[v] < DBL_MAX) minNeeded[v] = layerZero[v];
		}

		vector<RPGBuilder::RPGNumericEffect> & numericEffs = RPGBuilder::getNumericEffs();
		vector<list<int> > & actEffs = RPGBuilder::getActionsToRPGNumericStartEffects();

		actCount = actEffs.size();
		fluentAction = vector<bool>(actCount);
		preprocessedData = vector<list<ActionEffectUponVariable> >(actCount);

		goesToRealAction = vector<int>(varCount + actCount + 1);

		preferEarlier = vector<double>(varCount + actCount + 1);

		for (int i = 0; i < actCount; ++i) {
			fluentAction[i] = false;

			if (!RPGBuilder::rogueActions[i]) {
				list<int>::iterator effItr = actEffs[i].begin();
				const list<int>::iterator effEnd = actEffs[i].end();
	
				list<ActionEffectUponVariable> & into = preprocessedData[i] = list<ActionEffectUponVariable>();
	
				map<int, list<ActionEffectUponVariable>::iterator> dupli;

				for (; effItr != effEnd; ++effItr) {
					RPGBuilder::RPGNumericEffect & currEff = numericEffs[*effItr];
					const int mods = currEff.fluentIndex;				
					double quickEff = currEff.evaluate(layerZero);
					if (quickEff != 0.0) {
						map<int, list<ActionEffectUponVariable>::iterator>::iterator dItr = dupli.find(mods);
						if (dItr == dupli.end()) {
							into.push_back(ActionEffectUponVariable(mods,quickEff, currEff.isAssignment));
							list<ActionEffectUponVariable>::iterator backItr = into.end();
							--backItr;
							dupli.insert(make_pair(mods, backItr));
							if (currEff.isAssignment) everAssigned[mods] = true;
							if (RPGBuilder::getDominanceConstraints()[mods] != E_IRRELEVANT && RPGBuilder::getDominanceConstraints()[mods] != E_METRIC) fluentAction[i] = true;
							//cout << *(RPGBuilder::getInstantiatedOp(i)) << " with effect upon " << *(RPGBuilder::getPNE(mods)) << " (" << mods << ") : " << quickEff << "\n";
						} else {
							cout << "Action with two effects on " << *(RPGBuilder::getPNE(mods)) <<" - " << *(RPGBuilder::getInstantiatedOp(i)) << "\n";
							dItr->second->change += quickEff;
							if (fabs(dItr->second->change) < 0.000001) {
								into.erase(dItr->second);
								dupli.erase(dItr);
							}
						}
					}
				}
			}

			if (false) {
				list<ActionEffectUponVariable>::iterator effItr = preprocessedData[i].begin();
				const list<ActionEffectUponVariable>::iterator effEnd = preprocessedData[i].end();
	
				for (; effItr != effEnd; ++effItr) {
					cout << "Have an action at which gives us " << effItr->change << " on " << effItr->var; cout.flush(); cout <<  " (" << *(RPGBuilder::getPNE(effItr->var)) <<") - " << *(RPGBuilder::getInstantiatedOp(i)) << "\n";
				}
			}

		}


		vector<RPGBuilder::RPGNumericPrecondition> & numericPrecs = RPGBuilder::getNumericPrecs();

		const int precCount = numericPrecs.size();

		for (int i = 0; i < precCount; ++i) {
			double RHS = numericPrecs[i].RHSConstant;
			const bool ge = (numericPrecs[i].op == VAL::E_GREATER);
			int lhsVar = numericPrecs[i].LHSVariable;
			
			if (lhsVar < varCount) {
				if (ge) RHS += 1.0;
				if (maxNeeded[lhsVar] < RHS) maxNeeded[lhsVar] = RHS;
			} else if (lhsVar < (2 * varCount)) {
				lhsVar -= varCount;
				if (RHS != 0.0) RHS = 0.0 - RHS;
				if (ge) RHS -= 1.0;
				if (minNeeded[lhsVar] > RHS) minNeeded[lhsVar] = RHS;
			} else {
				

				RPGBuilder::ArtificialVariable aVar = RPGBuilder::getArtificialVariable(lhsVar);
				bool anyNeg = false;
				bool anyPos = false;
				for (int s = 0; s < aVar.size; ++s) {
					if (aVar.fluents[s] < varCount) {
						anyPos = true;
					} else {
						anyNeg = true;
					}
				}
				if (!anyNeg) {
					if (ge) RHS += 1.0;
					RHS -= aVar.constant;
					for (int s = 0; s < aVar.size; ++s) {
						if (maxNeeded[aVar.fluents[s]] < RHS) maxNeeded[aVar.fluents[s]] = RHS;
					}
				} else {
					for (int s = 0; s < aVar.size; ++s) {
						if (aVar.fluents[s] < varCount) maxNeeded[aVar.fluents[s]] = DBL_MAX;
					}
				}

				if (!anyPos) {
					RHS -= aVar.constant;
					if (RHS != 0.0) RHS = 0.0 - RHS;
					if (ge) RHS = RHS - 1.0;
					if (RHS < 0.0) RHS = 0.0;
					for (int s = 0; s < aVar.size; ++s) {
						if (minNeeded[aVar.fluents[s] - varCount] > RHS) minNeeded[aVar.fluents[s] - varCount] = RHS;
					}
				} else {
					for (int s = 0; s < aVar.size; ++s) {
						if (aVar.fluents[s] >= varCount) minNeeded[aVar.fluents[s] - varCount] = 0.0;
					}
				}


			}

			
		}

        {
            const list<pair<int,int> > & numGoals = RPGBuilder::getNumericRPGGoals();
            list<pair<int,int> >::const_iterator gItr = numGoals.begin();
            const list<pair<int,int> >::const_iterator gEnd = numGoals.end();
            for (; gItr != gEnd; ++gItr) {
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[gItr->first];
                const int currOp = (gItr->second != -1 ? EQ : (currPre.op == VAL::E_GREATEQ ? GE : LE));
                if (currPre.LHSVariable < (2*varCount)) {
                    int varIdx = currPre.LHSVariable;
                    REAL vw = currPre.LHSConstant;
                    if (varIdx >=varCount) {
                        if (vw != 0.0) vw = -vw;
                        varIdx -= varCount;
                    }
                    const REAL vb = (currPre.RHSConstant / vw);
                    goalVarBounds.push_back(GoalBound(vb,varIdx,currOp));
                } else {
                    goalConstraints.push_back(GoalConstraint());
                    GoalConstraint & currCons = goalConstraints.back();
                    
                    RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                    const int avSize = currAV.size;
                    for (int ai = 0; ai < avSize; ++ai) {
                        int varIdx = currAV.fluents[ai];
                        REAL vw = currAV.weights[ai];
                        if (varIdx >=varCount) {
                            if (vw != 0.0) vw = -vw;
                            varIdx -= varCount;
                        }
                        currCons.lhs.push_back(make_pair(varIdx,vw));
                    }
                    currCons.op = currOp;
                    currCons.rhs = currPre.RHSConstant;
                }
            }
        }
        
        {
            const list<Literal*> & litGoals = RPGBuilder::getLiteralGoals();
            const vector<int> & landmarkFacts = LandmarksAnalysis::getLandmarkFacts();

            const int combinedSize = litGoals.size() + landmarkFacts.size();
            
            propositionalGoals.resize(combinedSize);
            propositionalGoalConstraints.resize(combinedSize);
            propositionalGoalNotEncoded.resize(combinedSize);
            
            
            {
                list<Literal*>::const_iterator gItr = litGoals.begin();
                const list<Literal*>::const_iterator gEnd = litGoals.end();
                            
                for (int g = 0; gItr != gEnd; ++gItr, ++g) {
                    propositionalGoals[g] = (*gItr)->getID();
                }
            }
            {
                const int lCount = landmarkFacts.size();
                int gi = litGoals.size();
                for (int g = 0; g < lCount; ++g, ++gi) {
                    propositionalGoals[gi] = -1 - g;
                }
            }
            
            for (int g = 0; g < combinedSize; ++g) {
                const int fID = (propositionalGoals[g] >= 0 ? propositionalGoals[g] : landmarkFacts[-1-propositionalGoals[g]]);
                
                const list<pair<int, VAL::time_spec> > & achievers = RPGBuilder::getEffectsToActions(fID);
                list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
                const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();
                
                for (; accItr != accEnd; ++accItr) {
                    if (fluentAction[accItr->first]) {
                        propositionalGoalConstraints[g].push_back(accItr->first);
                    } else {
                        propositionalGoalNotEncoded[g].push_back(accItr->first);
                    }
                }
                
            }
        }

		donePreprocessing = true;
	}

	if (!varCount) return;

	lprec* const lp = make_lp(0,varCount);

       

	allLp.reserve(10);
	
	allLp.push_back(make_pair((LPAndBounds*) 0, new LPAndBounds(lp,false,varCount)));
	timestampToIndex[0.0] = 0;

	set_verbose(lp,3);


	for (int i = 1; i <= varCount; ++i) {

		ostringstream namestream;
		namestream << *(RPGBuilder::getPNE(i - 1));
		namestream << "[0.000]";
		string asString = namestream.str();
		set_col_name(lp, i, const_cast<char*>(asString.c_str()));
	}

	minMaxVars = vector<VarData>(varCount);

	for (int i = 0; i < varCount; ++i) {
		if (layerZero[i] < 0.0) set_lowbo(lp, i+1, layerZero[i]);
		minMaxVars[i].Vvar = i + 1;
	}

	{
		static REAL maximise[1];
		maximise[0] = 1.0;
		static int optVar[1];		
		optVar[0] = 1;

		set_maxim(lp);
		set_obj_fnex(lp, 1, maximise, optVar);
	}

	for (int i = 0; i < varCount; ++i) {
		static REAL* actWeight = new REAL[1];
		static int* colno = new int[1];
		
		actWeight[0] = 1.0;
		colno[0] = minMaxVars[i].Vvar;

		minMaxVars[i].consRow = i + 1;
		add_constraintex(lp, 1, actWeight, colno, EQ, layerZero[i]);

		ostringstream namestream;
		namestream << *(RPGBuilder::getPNE(i));
		namestream << " balance constraint (row " << minMaxVars[i].consRow << ")";
		string asString = namestream.str();
		set_row_name(lp, minMaxVars[i].consRow, const_cast<char*>(asString.c_str()));

	}

	for (int i = 0; i < varCount; ++i) {
		if (everAssigned[i]) {
			add_constraintex(lp, 0, 0, 0, LE, 1);
			minMaxVars[i].assignRow = get_Nrows(lp);

			ostringstream namestream;
			namestream << *(RPGBuilder::getPNE(i));
			namestream << " assignment constraint";
			string asString = namestream.str();
			set_row_name(lp, minMaxVars[i].assignRow, const_cast<char*>(asString.c_str()));

		}
	}

	{
		REAL* const startingPoint = new REAL[varCount + 1];
		startingPoint[0] = 0;
		for (int i = 0; i < varCount; ++i) {
			startingPoint[i + 1] = layerZero[i];
		}
		for (int i = 0; i < varCount; ++i) {
			minMaxVars[i].minimisationGuess = new REAL[varCount + 1];
			memcpy(minMaxVars[i].minimisationGuess, startingPoint, (varCount + 1) * sizeof(REAL));
			
			minMaxVars[i].maximisationGuess = new REAL[varCount + 1];
			memcpy(minMaxVars[i].maximisationGuess, startingPoint, (varCount + 1) * sizeof(REAL));

			minMaxVars[i].columnsInGuess = varCount + 1;
		}
		delete [] startingPoint;
	}
    {
        LPAndBounds* const oldBack = allLp.back().second;
        
        allLp.push_back(make_pair(oldBack, (LPAndBounds*) 0));
        
        
        PreferenceHandler::extendLP(switchVarForPreference,f,this);
        
        //cout << "After extension to switch variables, has " << get_Ncolumns(oldBack->lp) << " columns\n";
                
        allLp.pop_back();
    }
	//print_lp(lp);

};



MILPRPG::~MILPRPG() {

	if (varCount) {
		vector<pair<LPAndBounds*, LPAndBounds*> >::iterator lpItr = allLp.begin();
		const vector<pair<LPAndBounds*, LPAndBounds*> >::iterator lpEnd = allLp.end();

		for (; lpItr != lpEnd; ++lpItr) {
			delete lpItr->first;
			delete lpItr->second;
		}
		
		delete goalLP;
	}
};

void MILPRPG::addApplicableActions(vector<int> * const propPrec, vector<int> * const numPrec) {

	if (!varCount) return;

	const bool debugAdd = false;

	++layerCount;

	const double lWeight = pow(1.1,layerCount);

	LPAndBounds* const oldBack = allLp.back().second;

	allLp.back().second = 0;
	allLp.push_back(make_pair(oldBack, (LPAndBounds*) 0));

	lprec* const lp = oldBack->lp;

	list<int> & ivList = allLp.back().first->varsToIntegral;

	timestampToIndex[layerCount * 0.001] = layerCount;

	{
		const int ailSize = propPrec->size();
		for (int i = 0; i < ailSize; ++i) {
			if (!RPGBuilder::rogueActions[i] && fluentAction[i] && !((*propPrec)[i]) && !((*numPrec)[i])) {
				if (applicableSoFar.insert(i).second && RPGBuilder::isInteresting(i, startingState->first, startingState->startedActions)) {

					const int howMany = RPGBuilder::howManyTimes(i, *startingState);
					if (howMany > 0) {
	
						if (layerCount == 1) potentiallyHelpful.insert(i);

						ActData & newActData = layerActionVariables[i];
						add_columnex(lp,0,0,0);
						const int newCol = ++colCount;
						ivList.push_back(newCol);
						preferEarlier[newCol] = lWeight;
						assert(colCount == get_Ncolumns(lp));
						newActData.Avar = newCol;
						goesToRealAction[newCol] = i;
						set_bounds(lp,newCol,0,0);
						if (layerCount == 1) set_int(lp, newCol, TRUE);
						if (howMany != INT_MAX) {
							newActData.kShot = howMany;
//							if (howMany == 1) set_binary(lp, newCol, TRUE);

						

						} else {
							newActData.kShot = -1;
						}

						list<ActionEffectUponVariable>::iterator effItr = preprocessedData[i].begin();
						const list<ActionEffectUponVariable>::iterator effEnd = preprocessedData[i].end();

						for (; effItr != effEnd; ++effItr) {
							if (debugAdd) {
								cout << "Have an action at column " << newCol << " which gives us " << effItr->change << " on " << effItr->var; cout.flush(); cout <<  " (" << *(RPGBuilder::getPNE(effItr->var)) <<") - " << *(RPGBuilder::getInstantiatedOp(i)) << "\n";
							}
							set_mat(lp, minMaxVars[effItr->var].consRow, newCol, -(effItr->change));
							if (debugAdd) cout << "Row " << minMaxVars[effItr->var].consRow << " column " << newCol << " = " << -(effItr->change) << "\n";
							if (effItr->isAssignment) {
								set_mat(lp, minMaxVars[effItr->var].assignRow, newCol, 1.0);
								//set_binary(lp, newCol, TRUE);
								newActData.kShot = 1;
							}
						}
					}
				}
			}
		}
	}					

	{
		map<int, ActData>::iterator lavItr = layerActionVariables.begin();
		const map<int, ActData>::iterator lavEnd = layerActionVariables.end();

		for (; lavItr != lavEnd; ++lavItr) {
			int & colNo = lavItr->second.Avar;
			int & kShot = lavItr->second.kShot;
			int currUpBo = get_upbo(lp, colNo);
			if (currUpBo < kShot || kShot == -1) {
				currUpBo += 10;
				if (kShot != -1 && currUpBo > kShot) currUpBo = kShot;
				set_upbo(lp, colNo, currUpBo);
//				cout << "Upper bound on column " << colNo << " now " << currUpBo << "\n";
			}
		}
	}

/*	for (int v = 0; v < varCount; ++v) {
		if (everAssigned[v]) {
			set_rh(lp, minMaxVars[v].assignRow, layerCount);
		}
	}
*/	
};

bool MILPRPG::canSatisfyGoalConstraints(double & prefCost, MinimalState * forKShot, vector<int> * const propPrec, vector<int> * const numPrec, set<int> & propsSatisfied)
{
    static const bool debug = false;
    
    prefCost = 0;
    
    if (!varCount || !addProps) return true;
    
    delete goalLP;
           
    assert(allLp.back().first->lp);
    
    goalLP = new LPAndBounds(copy_lp(allLp.back().first->lp), allLp.back().first->presolved, varCount);    
    
    lprec* const lp = goalLP->lp;
    bool & presolved = goalLP->presolved;
    
    if (debug) {
        cout << "Incoming LP";
        if (presolved) cout << " is presolved, and";
        cout << " has " << get_Ncolumns(lp) << " columns and " << get_Nrows(lp) << " rows";
        if (presolved) cout << " (originally " << get_Norig_columns(lp) << " x " << get_Norig_rows(lp) << ")";
        cout << endl;
        
    }
    
    propsSatisfied.clear();
    
    const int Nrows = get_Norig_rows(lp);    
    
    int needToCheck = 0;
    
    {
        const int bgCount = goalVarBounds.size();
        for (int g = 0; g < bgCount; ++g) {
            const int RI = (presolved ? get_lp_index(lp, minMaxVars[goalVarBounds[g].v].Vvar + Nrows) : minMaxVars[goalVarBounds[g].v].Vvar);
            if (RI) {            
                switch(goalVarBounds[g].op) {
                    case LE:
                    {
                        if (debug) cout << "Adding bound: column " << RI << " <= " << goalVarBounds[g].w << endl;
                        set_upbo(lp, RI, goalVarBounds[g].w);
                        break;
                    }
                    case EQ:
                    {
                        if (debug) cout << "Adding bound: column " << RI << " == " << goalVarBounds[g].w << endl;
                        set_bounds(lp, RI, goalVarBounds[g].w, goalVarBounds[g].w);
                        break;
                    }
                    case GE:
                    {
                        if (debug) cout << "Adding bound: column " << RI << " >= " << goalVarBounds[g].w << endl;
                        set_lowbo(lp, RI, goalVarBounds[g].w);
                        break;
                    }   
                    default:
                    {
                        std::cerr << "Unexpected bound type\n";
                        exit(1);
                    }
                }
                ++needToCheck;
            } else {
                const double fixedVal = get_var_primalresult(lp, minMaxVars[goalVarBounds[g].v].Vvar + Nrows);
                switch(goalVarBounds[g].op) {
                    case LE:
                    {
                        if (fixedVal > goalVarBounds[g].w) return false;
                        break;
                    }
                    case EQ:
                    {
                        if (fabs(fixedVal - goalVarBounds[g].w) > 0.0001) return false;
                        break;
                    }
                    case GE:
                    {
                        if (fixedVal < goalVarBounds[g].w) return false;
                        break;
                    }   
                    default:
                    {
                        std::cerr << "Unexpected bound type\n";
                        exit(1);
                    }
                }
            }
        }
    }
    
    {
        const int bgCount = goalConstraints.size();
        for (int g = 0; g < bgCount; ++g) {
            if (debug) cout << "Adding goal constraint " << g << endl;
            const GoalConstraint & cons = goalConstraints[g];
            const int vecSize = cons.lhs.size();
            REAL * const wVec = new REAL[vecSize];
            int * const vVec = new int[vecSize];
            REAL finalRHS = cons.rhs;
            int finalVS = 0;
            for(int v = 0; v < vecSize; ++v) {
                const int RI = (presolved ? get_lp_index(lp, minMaxVars[cons.lhs[v].first].Vvar + Nrows)
                                          : minMaxVars[cons.lhs[v].first].Vvar);
                
                if (RI) {
                    vVec[finalVS] = RI;
                    wVec[finalVS] = cons.lhs[v].second;
                    ++finalVS;
                } else {
                    const double fixedVal = get_var_primalresult(lp, minMaxVars[cons.lhs[v].first].Vvar + Nrows);
                    finalRHS -= (cons.lhs[v].second * fixedVal);
                }
            }
            
            if (finalVS) {
                add_constraintex(lp, finalVS, wVec, vVec, cons.op, finalRHS);
                ++needToCheck;
                delete [] wVec;
                delete [] vVec;                
            } else {
                delete [] wVec;
                delete [] vVec;
                switch(goalVarBounds[g].op) {
                    case LE:
                    {
                        if (0 > finalRHS) return false;
                        break;
                    }
                    case EQ:
                    {
                        if (fabs(finalRHS) > 0.0001) return false;
                        break;
                    }
                    case GE:
                    {
                        if (0 < finalRHS) return false;
                        break;
                    }   
                    default:
                    {
                        std::cerr << "Unexpected bound type\n";
                        exit(1);
                    }
                }
            } 
        }
    }
    
    {
        const int bgCount = propositionalGoalConstraints.size();
        for (int g = 0; g < bgCount; ++g) {
            
            int fID = propositionalGoals[g];
            
            if (fID < 0) {
                
                if (forKShot->landmarkStatus[-1-fID]) continue; // landmark has already been seen
                fID = LandmarksAnalysis::getLandmarkFacts()[-1-fID];
                
            } else {
                
                if (!addLandmarks || forKShot->first.find(fID) != forKShot->first.end()) {
                    // goal already true in state - no need to add actions to achieve it
                    continue;
                }
            }
            
            const int getOutCount = propositionalGoalNotEncoded[g].size();
            bool mustAddConstraint = true;
            int actID;
            
            for (int ex = 0; ex < getOutCount; ++ex) {
                actID = propositionalGoalNotEncoded[g][ex];
                if (!(*propPrec)[actID] && !(*numPrec)[actID]) {
                    mustAddConstraint = false;
                    break;
                }
            }
            
            if (mustAddConstraint) {
                const int vecSize = propositionalGoalConstraints[g].size();
                REAL * const wVec = new REAL[vecSize];
                int * const vVec = new int[vecSize];
                int finalVS = 0;
                for(int v = 0; v < vecSize; ++v) {
                    actID = propositionalGoalConstraints[g][v];
                    if (!(*propPrec)[actID] && !(*numPrec)[actID]) {
                        
                        const map<int,ActData>::const_iterator avItr = layerActionVariables.find(actID);
                        assert(avItr != layerActionVariables.end());
                        
                        wVec[finalVS] = 1.0;
                        vVec[finalVS] = (presolved ? get_lp_index(lp, avItr->second.Avar + Nrows ) : avItr->second.Avar);

                        ++finalVS;
                    }
                }
            
                if (finalVS) {
                    add_constraintex(lp, finalVS, wVec, vVec, GE, 1.0);
                    ostringstream rn;
                    rn << "Achieve " << *(RPGBuilder::getLiteral(fID));
                    set_row_name(lp, get_Nrows(lp), const_cast<char*>(rn.str().c_str()) );    
                    propsSatisfied.insert(fID);
                    ++needToCheck;
                    delete [] wVec;
                    delete [] vVec;   
                } else {
                    delete [] wVec;
                    delete [] vVec;
                    return false;
                }
            }
        }
    }
    
    const int svCount = switchVarForPreference.size();
    
    const int origColCount = get_Norig_columns(lp);
    const int lpColCount = get_Ncolumns(lp);

    REAL * const wVec = new REAL[svCount + lpColCount];
    int * const vVec = new int[svCount + lpColCount];    
    int finalSize = 0;
    
                    
    for (int sv = 0; sv < svCount; ++sv) {
        const int svp = switchVarForPreference[sv];
        if (svp == -1) continue;       
                                
        const int RI = (presolved ? get_lp_index(lp, svp + Nrows) : svp);
        if (RI) {
            assert(RI <= get_Ncolumns(lp));
            vVec[finalSize] = RI;
            wVec[finalSize] = RPGBuilder::getPreferences()[sv].cost;
            ++finalSize;
            if (debug) {
                cout << "Adding ";
                if (presolved) cout << "post-presolve ";
                cout << "term to objective: " << RPGBuilder::getPreferences()[sv].cost << " * " << RI;
                if (presolved) cout << " (previously " << svp << ")";
                cout << endl;
            }
        } else {
            if (debug) cout << "Switch var " << svp << " for preference " << sv << " was presolved out\n";
            /*const double fixedVal = get_var_primalresult(lp, switchVarForPreference[sv] + Nrows);
            if (fixedVal >= 0.9999) {
                prefCost += RPGBuilder::getPreferences()[sv].cost;
            }*/
        }
    }
    
    const double peMultiplier = (finalSize ? 0.0001 : 1.0);
    
    for (int v = varCount; v <= origColCount; ++v) {
        const int RI = (presolved ? get_lp_index(lp, v + Nrows) : v);
        if (RI) {
            vVec[finalSize] = RI;
            wVec[finalSize] = peMultiplier * preferEarlier[v];
            ++finalSize;
        }
    }
    if (debug) cout << "Passing objective to lpsolve\n";
    
    //print_lp(lp);    
    set_minim(lp);
    set_obj_fnex(lp, finalSize, wVec, vVec);
    
    if (debug) {
        cout << "Solving...";
        cout.flush();
    }
    
    const int sRes = solve(lp);
    
    goalLP->presolved = true;
    
    if (sRes != 0 && sRes != 9) {
        if (debug) cout << "failed\n";
        return false;
    }
    
        
    if (debug) cout << "done\n";
    
    for (int sv = 0; sv < svCount; ++sv) {
        const int svp = switchVarForPreference[sv];
        if (svp == -1) continue;
        if (debug) cout << "Fetching result for switch var " << svp << " (for preference " << sv << ")\n";
        if (get_var_primalresult(lp, svp + Nrows) > 0.95) {
            prefCost += RPGBuilder::getPreferences()[sv].cost;
        }
    }
                
    if (debug) cout << "Returning that yes, goals are satisfied, and preference cost is " << prefCost << endl;
    return true;
    
}

void MILPRPG::fillMinMaxFluentTable(vector<double> & toFill) {

	if (!varCount) return;

	lprec* const lp = allLp.back().first->lp;

    bool & presolved = allLp.back().first->presolved;
    
	allLp.back().second = new LPAndBounds(copy_lp(lp), presolved, varCount);

	vector<double> & maxVec = allLp.back().first->max;
	vector<double> & minVec = allLp.back().first->min;
	vector<bool> & maxRealVec = allLp.back().first->maxReal;
	vector<bool> & minRealVec = allLp.back().first->minReal;

	for (int i = 0; i < varCount; ++i) {
		maxRealVec[i] = false;
		minRealVec[i] = false;
	}
	cout << "|";
	cout.flush();
//	static char buf[256];
	static REAL maximise[1];
	maximise[0] = 1.0;
	static int optVar[1];
//	map<double, vector<VarData>, EpsilonComp>::iterator fItr = minMaxVars.find(ts);
//	assert(fItr != minMaxVars.end());

	// bear in mind caching results to maximise/minimise each var, and use those to seed past layers

	const int rowCount = get_Nrows(lp);

	assert(colCount == get_Ncolumns(lp));

//	print_lp(lp);
	const int Nrows = get_Norig_rows(lp);

	set_basiscrash(lp, basiscrash);
        set_pivoting(lp, pivoting_rule | pivoting_mode);
        if (presolveloops == -1) {
                set_presolve(lp, presolve, get_presolveloops(lp));
        } else {
                set_presolve(lp, presolve, presolveloops);
        }
        set_simplextype(lp, simplextype);
        set_bb_floorfirst(lp, bb_floorfirst);
        set_bb_rule(lp, bb_rule + bb_mode);

	for (int i = 0; i < varCount; ++i) {
		if (RPGBuilder::getDominanceConstraints()[i] != E_METRIC && RPGBuilder::getDominanceConstraints()[i] != E_IRRELEVANT) {

			if (toFill[i] < maxNeeded[i]) {

                const int RI = (presolved ? get_lp_index(lp, minMaxVars[i].Vvar + Nrows) : minMaxVars[i].Vvar);
				if (RI) {
					optVar[0] = RI;

	//				cout << "Maximising column " << minMaxVars[i].Vvar << "  " << *(RPGBuilder::getPNE(i)) << "   ";
			
	//				REAL* startingPoint = new REAL[colCount + 1];
			//		int * basisVector = new int[rowCount + colCount + 1];
	//				for (int t = 0; t <= colCount; ++t) startingPoint[t] = 0.0;
			
			//		cout << (colCount + 1) - (minMaxVars[i].columnsInGuess) << " columns added since last change, ";
			//		cout.flush();
			
	//				memcpy(startingPoint, minMaxVars[i].maximisationGuess, (minMaxVars[i].columnsInGuess) * sizeof(REAL));
					
			
					set_maxim(lp);
			//		maximise[0] = -1.0;
					set_obj_fnex(lp, 1, maximise, optVar);
					//print_lp(lp);
			//		if (guess_basis(lp, startingPoint, basisVector)) {
			//			set_basis(lp, basisVector, TRUE);
			//		} else {
			//			cout << "For some reason, the max basis for " << minMaxVars[i].Vvar << " is invalid...\n";
			//		}
					
			//		cout << "Objective function: maximise " << maximise[0] << " * var " << optVar[0] << "\n";
			
			
			//		cout << "maximising"; cout.flush();		
					solve(lp);
                    presolved = true;
			
			//		print_solution(lp, varCount);
	//				get_variables(lp, &(startingPoint[1]));

					const double mv = get_objective(lp);
					//const double mv = startingPoint[optVar[0]];
					toFill[i] = mv;
			
	//				cout << mv << "\n";
		
					maxRealVec[i] = true;
	//				delete [] minMaxVars[i].maximisationGuess;
	//				minMaxVars[i].maximisationGuess = startingPoint;
	//				delete [] startingPoint;


				} else {
					toFill[i] = get_var_primalresult(lp, minMaxVars[i].Vvar + Nrows);
					maxRealVec[i] = true;
				}

			}

			maxVec[i] = toFill[i];

			if (toFill[i+varCount] < (-minNeeded[i])) {

                const int RI = (presolved ? get_lp_index(lp, minMaxVars[i].Vvar + Nrows) : minMaxVars[i].Vvar);
				if (RI) {
					optVar[0] = RI;

			//		set_upbo(lp, optVar[0], mv);
			//		cout << "Result = " << toFill[i] << "\n";
			//		print_solution(lp,varCount);
			//		print_lp(lp);
			//		cout << "Minimising " << *(RPGBuilder::getPNE(i)) << "\n";
			
			//		delete [] basisVector;
			
			//		basisVector = new int[rowCount + colCount + 1];
			
	//				REAL* startingPoint = new REAL[colCount + 1];
	//				for (int t = 0 ; t <= colCount; ++t) startingPoint[t] = 0.0;
			
					//memcpy(startingPoint, minMaxVars[i].minimisationGuess, (minMaxVars[i].columnsInGuess) * sizeof(REAL));
			
					set_minim(lp);
			//		maximise[0] = 1.0;
					set_obj_fnex(lp, 1, maximise, optVar);
			
			//		if (guess_basis(lp, startingPoint, basisVector)) {
			//			set_basis(lp, basisVector, TRUE);
			//		} else {
			//			cout << "For some reason, the min basis for " << minMaxVars[i].Vvar << " is invalid...\n";
			//		}
			
			
			
			//		cout << ", minimising"; cout.flush();
					solve(lp);
                    presolved = true;
					const double mvTwo = get_objective(lp);
	//				get_variables(lp, &(startingPoint[1]));

					//const double mvTwo = startingPoint[optVar[0]];
					toFill[i + varCount] = -mvTwo;

					minRealVec[i] = true;
	//				delete [] minMaxVars[i].minimisationGuess;
	//				minMaxVars[i].minimisationGuess = startingPoint;
	//				delete [] startingPoint;
				} else {
					toFill[i + varCount] = -get_var_primalresult(lp, minMaxVars[i].Vvar + Nrows);
					minRealVec[i] = true;
				}

			}
			minVec[i] = -toFill[i + varCount];
				
		/*
		//		delete [] basisVector;
		
		//		set_lowbo(lp, optVar[0], mvTwo);
		//		cout << "Minimum Result = " << -(toFill[i + varCount]) << "\n";
		
				// sanity checking time
		
				if (false && minMaxVars[i].Vvar == 8) {
					cout << "Solution from past maxim:";
						
					for (int v = 0; v <= colCount; ++v) {
						cout << "\t" << minMaxVars[i].maximisationGuess[v];
					}
					cout << "\n";
			
					print_lp(lp);
					
				}
		
				set_maxim(lp);
		//		maximise[0] = -1.0;
				set_obj_fnex(lp, 1, maximise, optVar);
		
				default_basis(lp);
				
				cout << ", sanityC"; cout.flush();
		
				solve(lp);
		
				set_minim(lp);
		//		maximise[0] = -1.0;
				set_obj_fnex(lp, 1, maximise, optVar);
		
				default_basis(lp);
				
				cout << ", sanityc"; cout.flush();
		
				solve(lp);
		
		
				basisVector = new int[rowCount + colCount + 1];
		
				set_maxim(lp);
		//		maximise[0] = -1.0;
				set_obj_fnex(lp, 1, maximise, optVar);
		
				if (guess_basis(lp, minMaxVars[i].maximisationGuess, basisVector)) {
					set_basis(lp, basisVector, TRUE);
				} else {
					cout << ", ish"; cout.flush();
				}
		
		
		
				
				cout << ", sanity+"; cout.flush();
		
				solve(lp);
		
				delete [] basisVector;
		
				basisVector = new int[rowCount + colCount + 1];
		
				set_minim(lp);
		//		maximise[0] = -1.0;
				set_obj_fnex(lp, 1, maximise, optVar);
		
		
		
				if (guess_basis(lp, minMaxVars[i].minimisationGuess, basisVector)) {
					set_basis(lp, basisVector, TRUE);
				} else {
					cout << "ish"; cout.flush();
				}
		
		
				
				cout << ", sanity-\n"; cout.flush();
		
				solve(lp);
		
				delete [] basisVector;
		*/
		} else {
			toFill[i] = 0.0;
			toFill[i + varCount] = 0.0;
		}
	}

}

void MILPRPG::getActionsForGoal(list<pair<int, double> > & toReturn)
{
    if (!addProps) return;
    
    assert(goalLP);
    assert(goalLP->presolved);
    lprec * const lp = goalLP->lp;
    
    const int Nrows = get_Norig_rows(lp);
    const int origColCount = get_Norig_columns(lp);
    const int lpColCount = get_Ncolumns(lp);
    
    REAL* results = new REAL[lpColCount];
    
    get_variables(lp, results);
    
    for (int v = varCount; v < origColCount; ++v) {
        const int RI = get_lp_index(lp, v + 1 + Nrows);
        if (RI) {
            REAL var = results[RI - 1];
            if (var > 0.001) toReturn.push_back(pair<int,double>(goesToRealAction[v+1], var));
        } else {
            REAL var = get_var_primalresult(lp, v + 1 + Nrows);
            if (var > 0.001) toReturn.push_back(pair<int,double>(goesToRealAction[v+1], var));
        }
    }
    
        
    delete [] results;
            
}

void MILPRPG::getActionsFor(const double & TS, const int & noPPi, const bool & min, const double & RHS, list<pair<int, double> > & toReturn) {

	assert(varCount);

	LPAndBounds* const toUse = allLp[timestampToIndex[TS]].first;
	lprec* const lp = toUse->lp;
    bool & presolved = toUse->presolved;
    
	const int Nrows = get_Norig_rows(lp);
	const int PPi = (presolved ? get_lp_index(lp, noPPi + 1 + Nrows) : noPPi + 1);

	if (!PPi) {
		return;
	}

	const int i = PPi - 1;

/*	cout << "MILP for var " << i;
	if (min) {
		cout << " <=";
	} else {
		cout << " >=";
	}
*/
	REAL oldlowbo = get_lowbo(lp, i + 1);
	REAL oldupbo = get_upbo(lp, i + 1);

	double boundVal = (RHS == -1.0 ? (min ? toUse->min[noPPi] : toUse->max[noPPi]) : RHS);

	
	const int origColCount = get_Norig_columns(lp);
	const int lpColCount = get_Ncolumns(lp);

	if (!toUse->nowExtracting) {

		REAL* row = new REAL[1 + lpColCount];
		row[0] = 0.0;
		for (int v = 0; v < varCount; ++v) {
			const int RI = (presolved ? get_lp_index(lp, v + 1 + Nrows) : v + 1);
			if (RI) {
				row[RI] = 0.0;
			}
		}

		for (int v = varCount; v <= origColCount; ++v) {
			const int RI = (presolved ? get_lp_index(lp, v + Nrows) : v);
			if (RI) {
				row[RI] = preferEarlier[v];
			}
		}
	
		set_minim(lp);
		set_obj_fn(lp, row);
		delete [] row;

		for (int v = 0; v < varCount; ++v) {
			if (toUse->minReal[v]) {
				const int RI = (presolved ? get_lp_index(lp, v + 1 + Nrows) : v + 1);
				if (RI) set_lowbo(lp, RI, toUse->min[v]);
			}
			if (toUse->maxReal[v]) {
				const int RI = (presolved ? get_lp_index(lp, v + 1 + Nrows) : v + 1);
				if (RI) set_upbo(lp, RI, toUse->max[v]);
			}
		}
//
//		list<int>::iterator ivItr = toUse->varsToIntegral.begin();
//		const list<int>::iterator ivEnd = toUse->varsToIntegral.end();
//
//		for (; ivItr != ivEnd; ++ivItr) {
//			if (!(is_binary(lp, *ivItr))) set_int(lp, *ivItr, TRUE);
//		}

		toUse->nowExtracting = true;
	}


	set_bounds(lp, i+1, boundVal, boundVal);
	solve(lp);
    presolved = true;

	REAL* results = new REAL[lpColCount];

	get_variables(lp, results);

	for (int v = varCount; v < origColCount; ++v) {
		const int RI = get_lp_index(lp, v + 1 + Nrows);
		if (RI) {
			REAL var = results[RI - 1];
			if (var > 0.001) toReturn.push_back(pair<int,double>(goesToRealAction[v+1], var));
		} else {
			REAL var = get_var_primalresult(lp, v + 1 + Nrows);
			if (var > 0.001) toReturn.push_back(pair<int,double>(goesToRealAction[v+1], var));
		}
	}

	
	delete [] results;

	set_bounds(lp, i+1, oldlowbo, oldupbo);
};

int MILPRPG::giveMeAColumn(const string & n)
{
    lprec * const lp = allLp.back().first->lp;
    add_columnex(lp,0,0,0);    
    const int newCol = ++colCount;
    set_binary(lp, newCol, TRUE);
    set_col_name(lp,newCol,const_cast<char*>(n.c_str()));
    return newCol;
}

int MILPRPG::getColumnForVar(const int & v)
{
    //lprec * const lp = allLp.back().first->lp;
    //const int Nrows = get_Norig_rows(lp);
    //return get_lp_index(lp, minMaxVars[v].Vvar + Nrows);
    
    return minMaxVars[v].Vvar;
}

void MILPRPG::addSwitch(const int & consSize, REAL * actWeight, int * colno, const REAL & rhs)
{
    lprec * const lp = allLp.back().first->lp;
 
    for (int vi = 0; vi < consSize; ++vi) {
        if (vi) cout << " + ";
        const int RI = colno[vi];
        cout << actWeight[vi] << "*" << get_col_name(lp,RI);
    }
    
    cout << " <= " << rhs << endl;
    
    add_constraintex(lp, consSize, actWeight, colno, LE, rhs);    
    set_row_name(lp, get_Nrows(lp), const_cast<char*>("Switch constraint") );    
        
}

};
