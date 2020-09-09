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

#include "solver.h"

#include "PreferenceHandler.h"
#include "landmarksanalysis.h"
#include "NumericAnalysis.h"

using std::ostringstream;
using std::string;
using std::endl;
using std::cerr;

void breakHere()
{
}

LPAndBounds::~LPAndBounds()
{
    delete lp;
}



namespace Planner
{

vector<bool> MILPRPG::fluentAction;
vector<bool> MILPRPG::everAssigned;
vector<bool> MILPRPG::isInTheGoal;
vector<uint> MILPRPG::everInAResidual;
vector<MILPRPG::ResidualPrecondition> residualData;

vector<list<MILPRPG::ActionEffectUponVariable> > MILPRPG::preprocessedData;
vector<list<int> > MILPRPG::preprocessedResiduals;
vector<MILPRPG::ResidualPrecondition> MILPRPG::residualData;

int MILPRPG::actCount;
int MILPRPG::initialMatrixRows;
int MILPRPG::initialMatrixCols;
int MILPRPG::residualColumnCount;
int MILPRPG::bootstrapPropCount;

int MILPRPG::integerLevel = 2;
int MILPRPG::secondaryIntegerLevel = 0;

double MILPRPG::layerFactor = 1.1;

vector<int> MILPRPG::goesToRealAction;
vector<double> MILPRPG::preferEarlier;

vector<double> MILPRPG::maxNeeded;
vector<double> MILPRPG::minNeeded;
vector<MILPRPG::GoalConstraint> MILPRPG::goalConstraints;
vector<vector<int> > MILPRPG::propositionalGoalConstraints;
vector<vector<int> > MILPRPG::propositionalGoalNotEncoded;
vector<list<int> > MILPRPG::preprocessedBootstrapPreconditions;
vector<list<int> > MILPRPG::preprocessedBootstrapEffects;

vector<int> MILPRPG::propositionalGoals;
vector<MILPRPG::GoalBound> MILPRPG::goalVarBounds;

bool MILPRPG::addNumGoalConjunct = true;
bool MILPRPG::addProps = true;
bool MILPRPG::addLandmarks = true;
bool MILPRPG::useLocalLandmarks = false;
bool MILPRPG::integerDisjunctiveLandmarksEncoding = true;
bool MILPRPG::alternativeObjective = false;
int MILPRPG::useParamILS = 1;
bool MILPRPG::recogniseBootstrappingPropositions = true;
bool MILPRPG::ensureAllPropositionalPreconditionsAreMet = false;
bool MILPRPG::residualEverything = false;
bool MILPRPG::useSecondaryObjective = false;
bool MILPRPG::neverUsePresolving = false;


struct BootstrapData {

    list<int> addedByAction;
    list<int> neededByAction;

    bool consistent;

    BootstrapData()
            : consistent(true) {
    }

    void abort() {
        consistent = false;
    }

};

#ifndef NDEBUG
const bool nameRowsAndColumns = true;
#else
const bool nameRowsAndColumns = false;
#endif


MILPRPG::MILPRPG(MinimalState * f, const vector<double> & layerZero, const vector<double> * actionCosts)
    : varCount(layerZero.size()), colCount(varCount), layerCount(0), startingState(f),
      anyIncreasers(varCount, false), anyDecreasers(varCount, false), goalLP(0),propositionalActionCosts(actionCosts)
{



    if (!varCount) return;

    static bool donePreprocessing = false;

    if (!donePreprocessing) {

        everAssigned = vector<bool>(varCount, false);
        isInTheGoal = vector<bool>(varCount, false);
        everInAResidual = vector<uint>(varCount, 0);
        maxNeeded = vector<double>(varCount, 0.0);
        minNeeded = vector<double>(varCount, DBL_MAX);
        initialMatrixRows = varCount;
        initialMatrixCols = varCount;
        residualColumnCount = 0;
        bootstrapPropCount = 0;
        int djLandmarkVarCount = 0;
        
        const vector<RPGBuilder::RPGNumericEffect> & numericEffs = RPGBuilder::getNumericEffs();
        const vector<list<int> > & actEffs = RPGBuilder::getActionsToRPGNumericStartEffects();

        const vector<RPGBuilder::RPGNumericPrecondition> & numericPrecs = RPGBuilder::getNumericPrecs();
        const vector<list<int> > & actPres = RPGBuilder::getActionsToRPGNumericStartPreconditions();

        actCount = actEffs.size();
        fluentAction = vector<bool>(actCount);
        preprocessedData = vector<list<ActionEffectUponVariable> >(actCount);
        preprocessedResiduals = vector<list<int> >(actCount);
        preprocessedBootstrapPreconditions = vector<list<int> >(actCount);
        preprocessedBootstrapEffects = vector<list<int> >(actCount);


        vector<BootstrapData> candidateBS;
        const vector<list<Literal*> > & actionPropositionalPres = RPGBuilder::getProcessedStartPreconditions();
        const vector<list<Literal*> > & actionStartAdds = RPGBuilder::getStartAddEffects();
        //const vector<list<Literal*> > & actionStartDels = RPGBuilder::getStartDeleteEffects();
        const int litCount = RPGBuilder::getLiteralCount();


        PreferenceHandler::noteWhichVariablesWeMightWantOptimisticBoundsFor(everInAResidual);

        if (recogniseBootstrappingPropositions) {
            candidateBS.resize(litCount);
            if (!ensureAllPropositionalPreconditionsAreMet) {
                for (int pp = 0; pp < litCount; ++pp) {
                    if (!RPGBuilder::getNegativeEffectsToActions()[pp].empty()) {
                        candidateBS[pp].abort();
                    }
                }
            }
        }


        const int prefCount = f->preferenceStatus.size();

        set<ResidualPrecondition> residualSet;
        int residualCount = 0;


        set<int> keepIfTheyHaveNumPres;
        
        if (addProps) {            
            const list<Literal*> & litGoals = RPGBuilder::getLiteralGoals();                
            {
                list<Literal*>::const_iterator gItr = litGoals.begin();
                const list<Literal*>::const_iterator gEnd = litGoals.end();
                for (int fID; gItr != gEnd; ++gItr) {
                    fID = (*gItr)->getStateID();
                    assert(fID != -1);
                    
                    const list<pair<int, VAL::time_spec> > & achievers = RPGBuilder::getEffectsToActions()[fID];
                    list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
                    const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();
                    
                    for (; accItr != accEnd; ++accItr) {
                        keepIfTheyHaveNumPres.insert(accItr->first);
                    }
                }
            }
        }
        if (addLandmarks) {
            const vector<int> & allLandmarkFacts = LandmarksAnalysis::getFactsInAnyLandmark();
            const int lfc = allLandmarkFacts.size();
            for (int i = 0; i < lfc; ++i) {
                const list<pair<int, VAL::time_spec> > & achievers = RPGBuilder::getEffectsToActions()[allLandmarkFacts[i]];
                list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
                const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();
                
                for (; accItr != accEnd; ++accItr) {
                    keepIfTheyHaveNumPres.insert(accItr->first);
                }
            }
        }
        

        for (int i = 0; i < actCount; ++i) {
            fluentAction[i] = ensureAllPropositionalPreconditionsAreMet;

            if (!RPGBuilder::rogueActions[i]) {

                map<int, list<ResidualPrecondition> > localNumPres;

                {
                    list<int>::const_iterator preItr = actPres[i].begin();
                    const list<int>::const_iterator preEnd = actPres[i].end();

                    for (; preItr != preEnd; ++preItr) {
                        const RPGBuilder::RPGNumericPrecondition & currPre = numericPrecs[*preItr];

                        if (currPre.LHSVariable < varCount) {
                            localNumPres[currPre.LHSVariable].push_back(ResidualPrecondition(currPre.LHSVariable, currPre.RHSConstant, currPre.op));
                            //cout << "Registered precondition: var " << currPre.LHSVariable << " >= " <<  currPre.RHSConstant << std::endl;
                            assert(localNumPres.find(currPre.LHSVariable) != localNumPres.end());

                        } else if (currPre.LHSVariable < 2 * varCount) {
                            const double flippedRHS = (currPre.RHSConstant != 0.0 ? -currPre.RHSConstant : 0.0);

                            const double geGap = 0.00001;

                            switch (currPre.op) {
                            case VAL::E_GREATEQ:
                                localNumPres[currPre.LHSVariable - varCount].push_back(ResidualPrecondition(currPre.LHSVariable - varCount, flippedRHS, VAL::E_LESSEQ));
                                break;
                            case VAL::E_GREATER:
                                localNumPres[currPre.LHSVariable - varCount].push_back(ResidualPrecondition(currPre.LHSVariable - varCount, flippedRHS - geGap, VAL::E_LESSEQ));
                                break;
                            case VAL::E_LESSEQ:
                                localNumPres[currPre.LHSVariable - varCount].push_back(ResidualPrecondition(currPre.LHSVariable - varCount, flippedRHS, VAL::E_GREATEQ));
                                break;
                            case VAL::E_LESS:
                                localNumPres[currPre.LHSVariable - varCount].push_back(ResidualPrecondition(currPre.LHSVariable - varCount, flippedRHS + geGap, VAL::E_GREATEQ));
                                break;
                            case VAL::E_EQUALS:
                                localNumPres[currPre.LHSVariable - varCount].push_back(ResidualPrecondition(currPre.LHSVariable - varCount, flippedRHS, VAL::E_EQUALS));
                                break;
                            }


                        } else {
                            const RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                            
                            if (currAV.size == 1) {
                                double newRhs = currPre.RHSConstant - currAV.constant;
                                const double geGap = 0.00001;
                                
                                int currVar = currAV.fluents[0];
                                double currW = currAV.weights[0];
                                
                                if (currVar >= varCount) {
                                    currVar -= varCount;
                                    currW = -currW;
                                }
                               
                                
                                if (currW < 0.0) {
                                    const double flippedRHS = (newRhs != 0.0 ? -newRhs : 0.0);
                                    if (currPre.op == VAL::E_GREATEQ) {
                                        localNumPres[currVar].push_back(ResidualPrecondition(currVar, flippedRHS, VAL::E_LESSEQ));
                                    } else {
                                        localNumPres[currVar].push_back(ResidualPrecondition(currVar, flippedRHS - geGap, VAL::E_LESSEQ));
                                    }
                                } else {
                                    if (currPre.op == VAL::E_GREATEQ) {
                                        localNumPres[currVar].push_back(ResidualPrecondition(currVar, newRhs, VAL::E_GREATEQ));
                                    } else {
                                        localNumPres[currVar].push_back(ResidualPrecondition(currVar, newRhs + geGap, VAL::E_GREATEQ));
                                    }
                                }
                                
                            }
                            
                            
                            // Do nothing for now
                        }

                    }
                }
                
                if (!localNumPres.empty()) {
                    if (keepIfTheyHaveNumPres.find(i) != keepIfTheyHaveNumPres.end()) {
                        fluentAction[i] = true;
                    }
                }

                {
                    list<int>::const_iterator effItr = actEffs[i].begin();
                    const list<int>::const_iterator effEnd = actEffs[i].end();

                    list<ActionEffectUponVariable> & into = preprocessedData[i] = list<ActionEffectUponVariable>();

                    map<int, list<ActionEffectUponVariable>::iterator> dupli;

                    for (; effItr != effEnd; ++effItr) {
                        const RPGBuilder::RPGNumericEffect & currEff = numericEffs[*effItr];
                        const int mods = currEff.fluentIndex;
                        double quickEff = currEff.evaluate(layerZero);
                        if (quickEff != 0.0) {
                            map<int, list<ActionEffectUponVariable>::iterator>::iterator dItr = dupli.find(mods);
                            if (dItr == dupli.end()) {
                                into.push_back(ActionEffectUponVariable(mods, quickEff, currEff.isAssignment));
                                list<ActionEffectUponVariable>::iterator backItr = into.end();
                                --backItr;
                                dupli.insert(make_pair(mods, backItr));
                                if (currEff.isAssignment) everAssigned[mods] = true;
                                if (RPGBuilder::getDominanceConstraints()[mods] != E_IRRELEVANT && RPGBuilder::getDominanceConstraints()[mods] != E_METRIC) fluentAction[i] = true;
                                //cout << *(RPGBuilder::getInstantiatedOp(i)) << " with effect upon " << *(RPGBuilder::getPNE(mods)) << " (" << mods << ") : " << quickEff << "\n";
                            } else {
                                cout << "Action with two effects on " << *(RPGBuilder::getPNE(mods)) << " - " << *(RPGBuilder::getInstantiatedOp(i)) << "\n";
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
                        cout << "Have an action at which gives us " << effItr->change << " on " << effItr->var;
                        cout.flush();
                        cout <<  " (" << *(RPGBuilder::getPNE(effItr->var)) << ") - " << *(RPGBuilder::getInstantiatedOp(i)) << "\n";
                    }
                }

                if (fluentAction[i]) {
                    map<int, double> effMap;

                    {
                        list<ActionEffectUponVariable>::iterator effItr = preprocessedData[i].begin();
                        const list<ActionEffectUponVariable>::iterator effEnd = preprocessedData[i].end();

                        for (; effItr != effEnd; ++effItr) {
                            effMap.insert(make_pair(effItr->var, effItr->change));
                        }
                    }

                    map<int, list<ResidualPrecondition> >::const_iterator lnpItr = localNumPres.begin();
                    const map<int, list<ResidualPrecondition> >::const_iterator lnpEnd = localNumPres.end();

                    for (; lnpItr != lnpEnd; ++lnpItr) {
                        map<int, double>::const_iterator emItr = effMap.find(lnpItr->first);
                        const double effOnIt = (emItr != effMap.end() ? emItr->second : 0.0);

                        list<ResidualPrecondition>::const_iterator rpItr = lnpItr->second.begin();
                        const list<ResidualPrecondition>::const_iterator rpEnd = lnpItr->second.end();

                        for (; rpItr != rpEnd; ++rpItr) {

                            if (rpItr->op == VAL::E_GREATEQ || rpItr->op == VAL::E_LESSEQ) {

                                if (residualEverything || fabs(rpItr->amount + effOnIt) > 0.00001) {

                                    pair<set<ResidualPrecondition>::iterator, bool> fr = residualSet.insert(*rpItr);

                                    if (fr.second) {
                                        fr.first->ID = residualCount++;
                                        if (rpItr->op == VAL::E_GREATEQ) {
                                            everInAResidual[lnpItr->first] |= 1;
                                            cout << "Creating precondition switch for " << *(RPGBuilder::getPNE(lnpItr->first)) << " >= " << rpItr->amount << " due to action " << *(RPGBuilder::getInstantiatedOp(i)) << std::endl;
                                        } else {
                                            everInAResidual[lnpItr->first] |= 2;
                                            cout << "Creating precondition switch for " << *(RPGBuilder::getPNE(lnpItr->first)) << " <= " << rpItr->amount << " due to action " << *(RPGBuilder::getInstantiatedOp(i)) << std::endl;
                                        }
                                    }

                                    preprocessedResiduals[i].push_back(fr.first->ID);

                                }

                            } else {
                                cerr << "Internal error: equality preconditions should have been split in preprocessing\n";
                                exit(1);
                            }

                        }

                    }

                }

                if (recogniseBootstrappingPropositions) {
                    list<Literal*>::const_iterator addEff = actionStartAdds[i].begin();
                    const list<Literal*>::const_iterator addEffEnd = actionStartAdds[i].end();

                    for (; addEff != addEffEnd; ++addEff) {
                        const int litID = (*addEff)->getStateID();
                        if (!candidateBS[litID].consistent) continue;

                        if (fluentAction[i] || ensureAllPropositionalPreconditionsAreMet) {
                            candidateBS[litID].addedByAction.push_back(i);
                        } else {
                            candidateBS[litID].abort();
                        }
                    }

                    if (fluentAction[i] || ensureAllPropositionalPreconditionsAreMet) {
                        {
                            list<Literal*>::const_iterator actPre = actionPropositionalPres[i].begin();
                            const list<Literal*>::const_iterator actPreEnd = actionPropositionalPres[i].end();
                            for (; actPre != actPreEnd; ++actPre) {
                                const int litID = (*actPre)->getStateID();
                                if (candidateBS[litID].consistent) {
                                    candidateBS[litID].neededByAction.push_back(i);
                                }
                            }
                        }
                    }
                }
            }



        }


        const int precCount = numericPrecs.size();

        for (int i = 0; i < precCount; ++i) {
            double RHS = numericPrecs[i].RHSConstant;
            const bool ge = (numericPrecs[i].op == VAL::E_GREATER);
            int lhsVar = numericPrecs[i].LHSVariable;

            if (lhsVar < varCount) {
                if (ge) RHS += 1.0;
                if (maxNeeded[lhsVar] < RHS) maxNeeded[lhsVar] = RHS;
                isInTheGoal[lhsVar] = true;
            } else if (lhsVar < (2 * varCount)) {
                lhsVar -= varCount;
                if (RHS != 0.0) RHS = 0.0 - RHS;
                if (ge) RHS -= 1.0;
                if (minNeeded[lhsVar] > RHS) minNeeded[lhsVar] = RHS;
                isInTheGoal[lhsVar] = true;
            } else {


                RPGBuilder::ArtificialVariable aVar = RPGBuilder::getArtificialVariable(lhsVar);
                bool anyNeg = false;
                bool anyPos = false;
                for (int s = 0; s < aVar.size; ++s) {
                    if (aVar.fluents[s] < varCount) {
                        isInTheGoal[aVar.fluents[s]] = true;
                        anyPos = true;
                    } else {
                        isInTheGoal[aVar.fluents[s]-varCount] = true;
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
            const list<pair<int, int> > & numGoals = RPGBuilder::getNumericRPGGoals();
            list<pair<int, int> >::const_iterator gItr = numGoals.begin();
            const list<pair<int, int> >::const_iterator gEnd = numGoals.end();
            for (; gItr != gEnd; ++gItr) {
                RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPrecs()[gItr->first];
                int currOp = (gItr->second != -1 ? 0 : (currPre.op == VAL::E_GREATEQ ? 1 : -1));
                if (currPre.LHSVariable < (2*varCount)) {
                    int varIdx = currPre.LHSVariable;
                    double vw = currPre.LHSConstant;
                    if (varIdx >= varCount) {
                        if (vw != 0.0) vw = -vw;
                        varIdx -= varCount;
                    }
                    if (vw < 0.0) {
                        currOp *= -1;
                        vw = -vw;
                    }
                    const double vb = (currPre.RHSConstant / vw);
                    goalVarBounds.push_back(GoalBound(vb, varIdx, currOp));
                } else {
                    RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                    if (currAV.size == 1) {
                        int varIdx = currAV.fluents[0];
                        double vw = currAV.weights[0];
                        if (varIdx >= varCount) {
                            if (vw != 0.0) vw = -vw;
                            varIdx -= varCount;
                        }
                        double rhs = (currPre.RHSConstant - currAV.constant);
                        if (vw < 0.0) {
                            currOp *= -1;
                            rhs *= -1;
                            vw = -vw;                            
                        }
                        const double vb = (rhs / vw);
                        goalVarBounds.push_back(GoalBound(vb, varIdx, currOp));
                                            
                    } else {                     
                        goalConstraints.push_back(GoalConstraint());
                        GoalConstraint & currCons = goalConstraints.back();
                        
                        const int avSize = currAV.size;
                        for (int ai = 0; ai < avSize; ++ai) {
                            int varIdx = currAV.fluents[ai];
                            double vw = currAV.weights[ai];
                            if (varIdx >= varCount) {
                                if (vw != 0.0) vw = -vw;
                                varIdx -= varCount;
                            }
                            currCons.lhs.push_back(make_pair(varIdx, vw));
                        }
                        currCons.op = currOp;
                        currCons.rhs = currPre.RHSConstant - currAV.constant;
                    }
                }
            }
        }

        {
            const list<Literal*> & litGoals = RPGBuilder::getLiteralGoals();
            const vector<int> & individualLandmarkFacts = LandmarksAnalysis::getIndividualLandmarkFacts();
            const vector<int> & allLandmarkFacts = LandmarksAnalysis::getFactsInAnyLandmark();
            const vector<vector<vector<int> > > & disjunctiveLandmarks = LandmarksAnalysis::getDisjunctiveLandmarks();

            const int dlCount = disjunctiveLandmarks.size();
            
            const int combinedSize = litGoals.size() + individualLandmarkFacts.size() + dlCount;


            
            propositionalGoals.resize(combinedSize);
            propositionalGoalConstraints.resize(combinedSize - dlCount);
            propositionalGoalNotEncoded.resize(combinedSize - dlCount);


            {
                list<Literal*>::const_iterator gItr = litGoals.begin();
                const list<Literal*>::const_iterator gEnd = litGoals.end();

                for (int g = 0; gItr != gEnd; ++gItr, ++g) {
                    propositionalGoals[g] = (*gItr)->getStateID();
                }
            }
            {
                const int lCount = individualLandmarkFacts.size();
                int gi = litGoals.size();
                for (int g = 0; g < lCount; ++g, ++gi) {
                    propositionalGoals[gi] = -1 - g;
                }
                
                for (int dl = 0; dl < dlCount; ++dl, ++gi) {
                    propositionalGoals[gi] = -1 - (lCount + dl);
                }
            }

            for (int g = 0; g < combinedSize - dlCount; ++g) {
                const int fID = (propositionalGoals[g] >= 0 ? propositionalGoals[g] : individualLandmarkFacts[-1-propositionalGoals[g]]);

                cout << "In LP, noting achievers for " << *(RPGBuilder::getLiteral(fID)) << endl;
                
                const list<pair<int, VAL::time_spec> > & achievers = RPGBuilder::getEffectsToActions()[fID];
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
            
            if (integerDisjunctiveLandmarksEncoding) {
                // make sure we create the space needed to store the 'do I have this' fact variables etc.
                
                // first, a disjunctive landmark fact cannot be an individual landmark fact
                // or it would have been stripped from the disjunctives - it's clearly needed
                
                djLandmarkVarCount = allLandmarkFacts.size() - individualLandmarkFacts.size();
                
                const int dCount = disjunctiveLandmarks.size();
                
                for (int d = 0; d < dCount; ++d) {
                    // don't need a variable for the group itself: we can say that the sum of the member
                    // variables is >= 1
                    
                    // but we do need one variable per disjunctive landmark group member, which can
                    // only >= 1 iff all its constituent facts have been reached (on the path
                    // to the state, or added by some other action)
                    djLandmarkVarCount += disjunctiveLandmarks[d].size();
                }
            }
        }


        residualData.resize(residualCount);

        {
            set<ResidualPrecondition>::const_iterator rpItr = residualSet.begin();
            const set<ResidualPrecondition>::const_iterator rpEnd = residualSet.end();

            for (; rpItr != rpEnd; ++rpItr) {
                residualData[rpItr->ID] = *rpItr;
            }
        }


        for (int v = 0; v < varCount; ++v) {
            if (everAssigned[v]) {
                ++initialMatrixRows;
            }
            if (everInAResidual[v] & 1) {
                ++initialMatrixRows;
                ++initialMatrixCols;
                ++residualColumnCount;
            }
            if (everInAResidual[v] & 2) {
                ++initialMatrixRows;
                ++initialMatrixCols;
                ++residualColumnCount;
            }
        }


        if (recogniseBootstrappingPropositions) {

            for (int pp = 0; pp < litCount; ++pp) {
                if (!candidateBS[pp].consistent) continue;
                if (candidateBS[pp].neededByAction.empty()) continue;
                if (candidateBS[pp].addedByAction.empty()) continue;

                //cout << "Recognised " << *(RPGBuilder::getLiteral(pp)) << " as an infrastructure proposition\n";

                {
                    list<int>::const_iterator nbItr = candidateBS[pp].neededByAction.begin();
                    const list<int>::const_iterator nbItrEnd = candidateBS[pp].neededByAction.end();
                    for (; nbItr != nbItrEnd; ++nbItr) {
                        preprocessedBootstrapPreconditions[*nbItr].push_back(pp);
                    }
                }

                {
                    list<int>::const_iterator abItr = candidateBS[pp].addedByAction.begin();
                    const list<int>::const_iterator abItrEnd = candidateBS[pp].addedByAction.end();
                    for (; abItr != abItrEnd; ++abItr) {
                        preprocessedBootstrapEffects[*abItr].push_back(pp);
                    }
                }

                ++bootstrapPropCount;

            }
        }

        {

            map<int, set<int> > extraBootstraps;
            PreferenceHandler::importTriggerGroups(extraBootstraps, bootstrapPropCount);
            map<int, set<int> >::const_iterator ebItr = extraBootstraps.begin();
            const map<int, set<int> >::const_iterator ebEnd = extraBootstraps.end();
            for (; ebItr != ebEnd; ++ebItr) {
                if (ebItr->second.empty()) continue;
                bool keep = false;

                {
                    set<int>::const_iterator nbItr = ebItr->second.begin();
                    const set<int>::const_iterator nbItrEnd = ebItr->second.end();
                    for (; nbItr != nbItrEnd; ++nbItr) {
                        if (fluentAction[*nbItr]) {
                            if (!keep) {
                                cout << "Adding group for preference " << (-1 - ebItr->first) << endl;
                            }
                            preprocessedBootstrapPreconditions[*nbItr].push_back(ebItr->first);
                            cout << "\t" << *(RPGBuilder::getInstantiatedOp(*nbItr)) << endl;
                            keep = true;
                        }
                    }
                }
                if (keep) {
                    ++bootstrapPropCount;
                }


            }

        }




        goesToRealAction = vector<int>(varCount + actCount + residualCount + residualColumnCount + bootstrapPropCount + djLandmarkVarCount + (alternativeObjective ? actCount : 0) + prefCount);
        preferEarlier = vector<double>(varCount + actCount + residualCount + residualColumnCount + bootstrapPropCount + djLandmarkVarCount + (alternativeObjective ? actCount : 0) + prefCount);



        if (residualColumnCount) {
            cout << "Created " << residualColumnCount << " for optimistic upper/lower bounds\n";
        }

        donePreprocessing = true;
    }

    if (!varCount) return;

    // LPSOLVE: lprec* const lp = make_lp(0,varCount);
    // allLp.reserve(10);
    //set_verbose(lp,3);

    MILPSolver * const lp = getNewSolver();

    {
        const int rCount = residualData.size();
        vector<int> minusOnes(rCount, -1);
        allLp.push_back(make_pair((LPAndBounds*) 0, new LPAndBounds(lp, false, false, minusOnes, varCount)));
    }

    timestampToIndex[0.0] = 0;

    const vector<pair<double,double> > & varBounds = NumericAnalysis::getBounds();
    
    const vector<pair<int, double> > emptyVector;
    for (int i = 0; i < varCount; ++i) {        
        lp->addCol(emptyVector, -lp->getInfinity(), lp->getInfinity(), layerZero[i], MILPSolver::C_REAL);
        if (varBounds[i].first > -DBL_MAX) {
            lp->setColLower(lp->getNumCols() - 1, varBounds[i].first);
            //assert(layerZero[i] >= varBounds[i].first);
        }
        
        if (varBounds[i].second < DBL_MAX) {
            lp->setColUpper(lp->getNumCols() - 1, varBounds[i].second);
            //assert(layerZero[i] <= varBounds[i].second);
        }
    }

    minMaxVars = vector<VarData>(varCount);

    for (int i = 0; i < varCount; ++i) {
        if (layerZero[i] < 0.0) lp->setColLower(i, layerZero[i]);
        minMaxVars[i].Vvar = i;
        minMaxVars[i].consRow = i;
    }

    for (int i = 0; i < varCount; ++i) {
        vector<pair<int, double> > newRow(1);
        newRow[0] = make_pair(minMaxVars[i].Vvar, 1.0);        
        lp->addRow(newRow, layerZero[i], layerZero[i]);

    }

    int nextRowIndex = varCount;

    int nextResidual = varCount;

    for (int i = 0; i < varCount; ++i) {
        for (int mask = 1; mask <= 2; ++mask) {
            if (everInAResidual[i] & mask) {
                if (mask == 1) {
                    lp->addCol(emptyVector, layerZero[i], lp->getInfinity(), layerZero[i], MILPSolver::C_REAL);
                    minMaxVars[i].residualUB = nextResidual;
                } else {
                    lp->addCol(emptyVector, -lp->getInfinity(), layerZero[i], layerZero[i], MILPSolver::C_REAL);
                    minMaxVars[i].residualLB = nextResidual;
                }

                vector<pair<int, double> > newRow(1);
                newRow[0] = make_pair(nextResidual, 1.0);

                lp->addRow(newRow, layerZero[i], layerZero[i]);

                assert(nextRowIndex == nextResidual);
                ++nextRowIndex;
                ++nextResidual;
                ++colCount;
            }
        }
    }


    for (int i = 0; i < varCount; ++i) {
        if (everAssigned[i]) {
            lp->addRow(emptyVector, -lp->getInfinity(), 1.0);
            minMaxVars[i].assignRow = nextRowIndex;
            ++nextRowIndex;
        }
    }


    if (nameRowsAndColumns) {
        for (int i = 0; i < varCount; ++i) {

            {
                ostringstream namestream;
                namestream << *(RPGBuilder::getPNE(i));
                //namestream << "var" << i;
                string asString = namestream.str();
                lp->setColName(i, asString);
            }

            {
                ostringstream namestream;
                namestream << *(RPGBuilder::getPNE(i)) << " balance";
                string asString = namestream.str();
                lp->setRowName(i, asString);
                //cout << "Bounds on " << lp->getRowName(i) << " are [" << lp->getRowLower(i) << "," << lp->getRowUpper(i) << "], and on corresponding variable, ";
                //cout << "[" << lp->getColLower(i) << "," << lp->getColUpper(i) << "]\n";
            }

            if (everAssigned[i]) {
                ostringstream namestream;
                namestream << *(RPGBuilder::getPNE(i)) << " assignment";
                string asString = namestream.str();
                lp->setRowName(minMaxVars[i].assignRow, asString);
            }

            if (everInAResidual[i] & 1) {
                {
                    ostringstream namestream;
                    namestream << *(RPGBuilder::getPNE(i)) << " UB calc";
                    string asString = namestream.str();
                    lp->setRowName(minMaxVars[i].residualUB, asString);
                }
                {
                    ostringstream namestream;
                    namestream << *(RPGBuilder::getPNE(i)) << " UB";
                    string asString = namestream.str();
                    lp->setColName(minMaxVars[i].residualUB, asString);
                }
            }

            if (everInAResidual[i] & 2) {
                {
                    ostringstream namestream;
                    namestream << *(RPGBuilder::getPNE(i)) << " LB calc";
                    string asString = namestream.str();
                    lp->setRowName(minMaxVars[i].residualLB, asString);
                }
                {
                    ostringstream namestream;
                    namestream << *(RPGBuilder::getPNE(i)) << " LB";
                    string asString = namestream.str();
                    lp->setColName(minMaxVars[i].residualLB, asString);
                }
            }
        }

    }

    /*{
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
    }*/
    {
        LPAndBounds* const oldBack = allLp.back().second;

        allLp.push_back(make_pair(oldBack, (LPAndBounds*) 0));


        PreferenceHandler::extendLP(switchVarForPreference, f, this);

        //cout << "After extension to switch variables, has " << get_Ncolumns(oldBack->lp) << " columns\n";

        allLp.pop_back();

        switchVarCount = lp->getNumCols() - initialMatrixCols;

        colCount = lp->getNumCols();

        {
            map<int, list<pair<int,int> > >::const_iterator ptItr = preferenceTrips.begin();
            const map<int, list<pair<int,int> > >::const_iterator ptEnd = preferenceTrips.end();

            for (; ptItr != ptEnd; ++ptItr) {

                const int prefID = ptItr->first;
                assert(switchVarForPreference[prefID].hasBeenViolatedCol != -1);
                assert(!switchVarForPreference[prefID].second.empty());
                {
                    vector<pair<int, double> > tmpColVec(switchVarForPreference[prefID].second.size());
                    
                    list<int>::const_iterator svItr = switchVarForPreference[prefID].second.begin();
                    const list<int>::const_iterator svEnd = switchVarForPreference[prefID].second.end();
                    
                    for (; svItr != svEnd; ++svItr) {
                        tmpColVec[0].first = *svItr;
                        tmpColVec[0].second = -BIG;
                    }
                    lp->addCol(tmpColVec, 0, 1, 0, MILPSolver::C_BOOL);
                    goesToRealAction[colCount] = -3;
                    
                    if (nameRowsAndColumns) {
                        ostringstream n;
                        n << "triggervar" << RPGBuilder::getPreferences()[ptItr->first].name;
                        lp->setColName(colCount, n.str());
                    }
                }
                const int bsCol = colCount++;

                {
                    vector<pair<int, double> > newRowVec(1);
                    newRowVec[0] = make_pair(bsCol, BIG);
                    lp->addRow(newRowVec, 0, lp->getInfinity());

                    const int bsRow = lp->getNumRows() - 1;
                    bootstrapPropColumnAndRow.insert(make_pair((-1 - prefID), make_pair(bsCol, bsRow)));
                    
                    if (nameRowsAndColumns) {
                        ostringstream n;
                        n << "acttrip" << RPGBuilder::getPreferences()[ptItr->first].name;
                        lp->setRowName(bsRow, n.str());
                    }

                }

                const int tCount = ptItr->second.size();

                if (tCount) {
                    vector<pair<int, double> > newRowVec(1 + tCount);
                    newRowVec[0] = make_pair(bsCol, tCount + 1);

                    //cout << "Preference " << ptItr->first << " would be tripped into needing action by:";

                    list<pair<int,int> >::const_iterator hItr = ptItr->second.begin();
                    const list<pair<int, int> >::const_iterator hEnd = ptItr->second.end();

                    for (int ti = 1; hItr != hEnd; ++hItr, ++ti) {
                        //cout << " " << *hItr;
                        newRowVec[ti] = make_pair(hItr->second, 1);
                    }

                    //cout << endl;

                    lp->addRow(newRowVec, tCount, lp->getInfinity());
                    if (nameRowsAndColumns) {
                        ostringstream n;
                        n << "preftrip" << RPGBuilder::getPreferences()[ptItr->first].name;
                        lp->setRowName(lp->getNumRows() - 1, n.str());
                    }
                }

            }

            allLp.push_back(make_pair(oldBack, (LPAndBounds*) 0));
            PreferenceHandler::addHelperTriggers(switchVarForPreference, f, this);
            allLp.pop_back();
        }


    }
    
    if (addProps) {
        const int bgCount = propositionalGoals.size();
        for (int g = 0; g < bgCount; ++g) {
            int fID = propositionalGoals[g];
            if (fID < 0) {
                continue;
            }
            if (f->first.find(fID) != f->first.end()) {
                factsThatWillBeConsideredAsGoals.insert(fID);
            }
        }

        if (addLandmarks) {
            const int individualLandmarkCount = LandmarksAnalysis::getIndividualLandmarkFacts().size();
            for (int g = 0; g < bgCount; ++g) {

                int fID = propositionalGoals[g];
                int disjunctiveLandmarkID = -1;
                
                if (fID >= 0) continue;
                
                                
                disjunctiveLandmarkID = -1 - fID;
                    
                if (disjunctiveLandmarkID >= individualLandmarkCount) {
                    disjunctiveLandmarkID -= individualLandmarkCount;
                } else {
                    if (f->landmarkStatus[disjunctiveLandmarkID]) {
                        continue; // landmark has already been seen
                    }
                    fID = LandmarksAnalysis::getIndividualLandmarkFacts()[disjunctiveLandmarkID];
                    disjunctiveLandmarkID = -1;
                    
                }                    
                
                if (fID >= 0) {
                    factsThatWillBeConsideredAsGoals.insert(fID);
                }
            }
        }
        
                
    }
    
    actionAppearedInLayer.resize(RPGBuilder::getProcessedStartPreconditions().size());
    
    //print_lp(lp);

    //lp->writeLp("initial.lp");
};



MILPRPG::~MILPRPG()
{

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

void MILPRPG::addApplicableActions(vector<int> * const propPrec, vector<int> * const numPrec, const double & newTS)
{

    if (!varCount) return;

    const bool debugAdd = false;

    ++layerCount;

    const double lWeight = pow(layerFactor, layerCount - 1);

    LPAndBounds* const oldBack = allLp.back().second;

    allLp.back().second = 0;
    allLp.push_back(make_pair(oldBack, (LPAndBounds*) 0));

    MILPSolver* const lp = oldBack->lp;

    list<int> & ivList = allLp.back().first->varsToIntegral;

    vector<int> & residualSwitch = allLp.back().first->residualSwitch;

    bool & presolved = allLp.back().first->presolved;

    timestampToIndex[newTS] = layerCount;

    static const vector<pair<int, double> > emptyVector;

    int actAdded = 0;

    {
        const int ailSize = propPrec->size();
        for (int i = 0; i < ailSize; ++i) {
            if (!RPGBuilder::rogueActions[i] && fluentAction[i] && !((*propPrec)[i]) && !((*numPrec)[i])) {
                if (applicableSoFar.insert(i).second && RPGBuilder::isInteresting(i, startingState->first, startingState->startedActions)) {

                    ++actAdded;

                    if (debugAdd) cout << "Adding " << *(RPGBuilder::getInstantiatedOp(i)) << " to the LP\n";
                    const int howMany = RPGBuilder::howManyTimesOptimistic(i, *startingState);
                    if (howMany > 0) {

                        if (layerCount == 1) potentiallyHelpful.insert(i);
                                                

                        ActData & newActData = layerActionVariables[i];

                        {
                            list<int>::const_iterator rpItr = preprocessedResiduals[i].begin();
                            const list<int>::const_iterator rpEnd = preprocessedResiduals[i].end();

                            for (; rpItr != rpEnd; ++rpItr) {
                                if (residualSwitch[*rpItr] == -1) {
                                    // Create switch variables for residual preconditions, so column numbers are right below
                                    
                                    #ifndef NDEBUG
                                    if (colCount != lp->getNumCols()) {
                                        cerr << "Internal error: count of number of cols in MILPRPG (" << colCount << ") should equal " << lp->getNumCols() << endl;
                                        assert(colCount == lp->getNumCols());
                                    }
                                    #endif
                                    

                                    lp->addCol(emptyVector, 0, 1, 0, MILPSolver::C_BOOL);


                                    const double big = BIG;

                                    vector<pair<int, double> > newRowVec(2);

                                    if (residualData[*rpItr].op == VAL::E_GREATEQ) {
                                        if (nameRowsAndColumns) {
                                            ostringstream cn;
                                            cn << "need_" << *(RPGBuilder::getPNE(residualData[*rpItr].var)) << "geq" << residualData[*rpItr].amount;
                                            lp->setColName(colCount, cn.str());
                                        }
                                        newRowVec[0] = make_pair(minMaxVars[residualData[*rpItr].var].residualUB, 1.0);
                                        newRowVec[1] = make_pair(colCount, -(residualData[*rpItr].amount - SMALL));
                                        lp->addRow(newRowVec, 0, lp->getInfinity());
                                        if (nameRowsAndColumns) {
                                            ostringstream rn;
                                            rn << "make_" << *(RPGBuilder::getPNE(residualData[*rpItr].var)) << "geq" << residualData[*rpItr].amount;
                                            lp->setRowName(lp->getNumRows() - 1, rn.str());
                                        }
                                    } else {
                                        if (nameRowsAndColumns) {
                                            ostringstream cn;
                                            cn << "need_" << *(RPGBuilder::getPNE(residualData[*rpItr].var)) << "leq" << residualData[*rpItr].amount;
                                            lp->setColName(colCount, cn.str());
                                        }
                                        newRowVec[0] = make_pair(minMaxVars[residualData[*rpItr].var].residualLB, 1.0);
                                        newRowVec[1] = make_pair(colCount, big - (residualData[*rpItr].amount + SMALL));
                                        lp->addRow(newRowVec, -lp->getInfinity(), big);
                                        if (nameRowsAndColumns) {
                                            ostringstream rn;
                                            rn << "make_" << *(RPGBuilder::getPNE(residualData[*rpItr].var)) << "leq" << residualData[*rpItr].amount;
                                            lp->setRowName(lp->getNumRows() - 1, rn.str());
                                        }
                                    }

                                    vector<pair<int, double> > otherRowVec(2);

                                    otherRowVec[0] = make_pair(colCount, big);

                                    lp->addRow(otherRowVec, 0.0, lp->getInfinity());

                                    goesToRealAction[colCount] = -3;

                                    ++colCount;

                                    residualSwitch[*rpItr] = lp->getNumRows() - 1;

                                    if (nameRowsAndColumns) {
                                        if (residualData[*rpItr].op == VAL::E_GREATEQ) {
                                            ostringstream rn;
                                            rn << "musthave_" << *(RPGBuilder::getPNE(residualData[*rpItr].var)) << "geq" << residualData[*rpItr].amount;
                                            lp->setRowName(lp->getNumRows() - 1, rn.str());
                                        } else {
                                            ostringstream rn;
                                            rn << "musthave_" << *(RPGBuilder::getPNE(residualData[*rpItr].var)) << "leq" << residualData[*rpItr].amount;
                                            lp->setRowName(lp->getNumRows() - 1, rn.str());
                                        }
                                    }
                                }
                            }
                        }                        

                        vector<pair<int, double> > newColVec;

                        const vector<int> & precPrefs = RPGBuilder::getStartPreferences()[i];
                        
                        if (!precPrefs.empty()) {
                            const int pps = precPrefs.size();
                            for (int pi = 0; pi < pps; ++pi) {          
                                const int currPref = precPrefs[pi];
                                
                                if (RPGBuilder::getPreferences()[currPref].neverTrue) continue;
                                
                                vector<pair<int, double> > inewColVec(switchVarForPreference[precPrefs[pi]].second.size());
                                
                                list<int>::const_iterator hrItr = switchVarForPreference[precPrefs[pi]].second.begin();
                                const list<int>::const_iterator hrEnd = switchVarForPreference[precPrefs[pi]].second.end();
                                
                                for (int rf = 0; hrItr != hrEnd; ++hrItr, ++rf) {                                
                                    inewColVec[rf] = make_pair(*hrItr, DBL_MAX);
                                }
                                
                                lp->addCol(inewColVec, 0, 1, 0, MILPSolver::C_BOOL);
                                
                                const int acttrig = colCount++;
                                
                                goesToRealAction[acttrig] = -3;
                                
                                if (nameRowsAndColumns) {
                                    ostringstream nn;
                                    nn << "used_act_" << i;
                                    lp->setColName(acttrig, nn.str());
                                }
                                
                                lp->addCol(emptyVector, 0, lp->getInfinity(), 0, MILPSolver::C_REAL);
                                
                                const int acttimes = colCount++;
                                
                                goesToRealAction[acttimes] = -3;

                                if (nameRowsAndColumns) {
                                    ostringstream nn;
                                    nn << "penalise_act_" << i;
                                    lp->setColName(acttimes, nn.str());
                                }                                
                                
                                switchVarForPreference[precPrefs[pi]].setCountColumn(acttimes);
                                
                                {
                                    
                                    // If the action is applied, we need this pref
                                    
                                    vector<pair<int, double> > inewRowVec(1);
                                    inewRowVec[0] = make_pair(acttrig, BIG);
                                    lp->addRow(inewRowVec, 0.0, lp->getInfinity());
                                    
                                    if (nameRowsAndColumns) {
                                        ostringstream nn;
                                        nn << "need_pref_" << pi;
                                        lp->setRowName(lp->getNumRows() - 1, nn.str());
                                    }   
                                    
                                    newColVec.push_back(make_pair(lp->getNumRows() - 1, -SMALL));
                                }
                                
                                {
                                    // If the pref is triggered and violated, make acttimes
                                    // be at least as big as the number of time the action is applied
                                    // (or else, let it be 0)
                                    
                                    vector<pair<int, double> > inewRowVec(2);
                                    inewRowVec[0] = make_pair(acttimes, 1.0);
                                    inewRowVec[1] = make_pair(switchVarForPreference[precPrefs[pi]].hasBeenViolatedCol, -BIG);
                                    lp->addRow(inewRowVec, -BIG, lp->getInfinity());
                                    
                                    if (nameRowsAndColumns) {
                                        ostringstream nn;
                                        nn << "num_times_violated_" << pi;
                                        lp->setRowName(lp->getNumRows() - 1, nn.str());
                                    }
                                    
                                    newColVec.push_back(make_pair(lp->getNumRows() - 1, -1));
                                }
                                                                
                            }
                                                        
                        }
                        
                        
                        
                        if (bootstrapPropCount) {

                            const double big = BIG;

                            for (int bspass = 0; bspass < 2; ++bspass) {

                                list<int>::const_iterator bootstrapItr = (bspass ? preprocessedBootstrapEffects[i] : preprocessedBootstrapPreconditions[i]).begin();
                                const list<int>::const_iterator bootstrapItrEnd = (bspass ? preprocessedBootstrapEffects[i] : preprocessedBootstrapPreconditions[i]).end();
                                for (; bootstrapItr != bootstrapItrEnd; ++bootstrapItr) {

                                    int prefID = -1;

                                    if (*bootstrapItr >= 0) { // is a real literal, rather than a trigger group
                                        if ((*startingState).first.find(*bootstrapItr) != (*startingState).first.end()) continue;
                                    } else {
                                        assert(bspass == 0);

                                        prefID = -1 - *bootstrapItr;

                                        if (switchVarForPreference[prefID].hasBeenViolatedCol == -1) continue; // if no need to check preference
                                        if (switchVarForPreference[prefID].second.empty()) continue;

                                        //cout << "\nAdding trigger var for pref " << prefID << endl;
                                    }


                                    map<int, pair<int, int> >::iterator bsr = bootstrapPropColumnAndRow.find(*bootstrapItr);
                                    if (bsr == bootstrapPropColumnAndRow.end()) {

                                        if (*bootstrapItr >= 0.0) {
                                            lp->addCol(emptyVector, 0, 1, 0, MILPSolver::C_BOOL);
                                            if (nameRowsAndColumns) {
                                                ostringstream cn;
                                                cn << "fact" << *(RPGBuilder::getLiteral(*bootstrapItr));
                                                lp->setColName(lp->getNumCols() - 1, cn.str());
                                            }
                                        } else {
                                            vector<pair<int, double> > tmpColVec(switchVarForPreference[prefID].second.size());
                                            list<int>::const_iterator svItr = switchVarForPreference[prefID].second.begin();
                                            const list<int>::const_iterator svEnd = switchVarForPreference[prefID].second.end();
                                            
                                            for (; svItr != svEnd; ++svItr) {
                                                tmpColVec[0].first = *svItr;;
                                                tmpColVec[0].second = -BIG;
                                            }
                                            
                                            lp->addCol(tmpColVec, 0, 1, 0, MILPSolver::C_BOOL);
                                            if (nameRowsAndColumns) {
                                                ostringstream cn;
                                                cn << "mht" << RPGBuilder::getPreferences()[-1 - *bootstrapItr].name;
                                                lp->setColName(lp->getNumCols() - 1, cn.str());
                                            }
                                        }

                                        goesToRealAction[colCount] = -3;

                                        const int bsCol = colCount++;


                                        vector<pair<int, double> > newRowVec(1);
                                        newRowVec[0] = make_pair(bsCol, big);
                                        lp->addRow(newRowVec, 0, lp->getInfinity());

                                        const int bsRow = lp->getNumRows() - 1;
                                        
                                        if (nameRowsAndColumns) {
                                            if (*bootstrapItr >= 0) {
                                                ostringstream rn;
                                                rn << "pre" << *(RPGBuilder::getLiteral(*bootstrapItr));
                                                lp->setRowName(bsRow, rn.str());
                                            } else {
                                                ostringstream rn;
                                                rn << "tact" << RPGBuilder::getPreferences()[-1 - *bootstrapItr].name;
                                                lp->setRowName(bsRow, rn.str());
                                            }
                                        }

                                        if (*bootstrapItr >= 0) {
                                            newRowVec[0] = make_pair(bsCol, -1.0);
                                            lp->addRow(newRowVec, 0, lp->getInfinity());
                                            if (nameRowsAndColumns) {
                                                ostringstream rn;
                                                rn << "eff" << *(RPGBuilder::getLiteral(*bootstrapItr));
                                                lp->setRowName(bsRow + 1, rn.str());
                                            }
                                        }

                                        bsr = bootstrapPropColumnAndRow.insert(make_pair(*bootstrapItr, make_pair(bsCol, bsRow))).first;
                                    }

                                    if (bspass) {
                                        newColVec.push_back(make_pair(bsr->second.second + 1, 1.0));
                                    } else {
                                        newColVec.push_back(make_pair(bsr->second.second, -0.001));
                                    }
                                }

                            }
                        }

                        const int newCol = colCount++;

                        bool assigns = false;

                        bool affectsGoalVar = false;
                        
                        {
                            newColVec.reserve(preprocessedData[i].size());

                            list<ActionEffectUponVariable>::iterator effItr = preprocessedData[i].begin();
                            const list<ActionEffectUponVariable>::iterator effEnd = preprocessedData[i].end();

                            for (; effItr != effEnd; ++effItr) {
                                if (debugAdd) {
                                    cout << "Have an action at column " << newCol << " which gives us " << effItr->change << " on " << effItr->var;
                                    cout.flush();
                                    cout <<  " (" << *(RPGBuilder::getPNE(effItr->var)) << ") - " << *(RPGBuilder::getInstantiatedOp(i)) << "\n";
                                }

                                if (effItr->change > 0) {
                                    anyIncreasers[effItr->var] = true;
                                    affectsGoalVar = affectsGoalVar || isInTheGoal[effItr->var];
                                } else if (effItr->change < 0) {
                                    anyDecreasers[effItr->var] = true;
                                    affectsGoalVar = affectsGoalVar || isInTheGoal[effItr->var];
                                }

                                newColVec.push_back(make_pair(minMaxVars[effItr->var].consRow, -(effItr->change)));
                                if (debugAdd) cout << "Row " << minMaxVars[effItr->var].consRow << " column " << newCol << " = " << -(effItr->change) << "\n";
                                if (effItr->isAssignment) {
                                    newColVec.push_back(make_pair(minMaxVars[effItr->var].assignRow, 1.0));
                                    //set_binary(lp, newCol, TRUE);
                                    newActData.kShot = 1;
                                    assigns = true;
                                }

                                if (effItr->change > 0) {
                                    const int UBC = minMaxVars[effItr->var].residualUB;
                                    if (UBC != -1) {
                                        newColVec.push_back(make_pair(UBC, -(effItr->change)));
                                    }
                                } else {
                                    assert(effItr->change < 0.0); // 0 effects should have been preprocessed out
                                    const int LBC = minMaxVars[effItr->var].residualLB;
                                    if (LBC != -1) {
                                        newColVec.push_back(make_pair(LBC, -(effItr->change)));
                                    }
                                }
                            }

                        }

                        {
                            list<int>::const_iterator rpItr = preprocessedResiduals[i].begin();
                            const list<int>::const_iterator rpEnd = preprocessedResiduals[i].end();

                            for (; rpItr != rpEnd; ++rpItr) {
                                assert(residualSwitch[*rpItr] != -1);
                                newColVec.push_back(make_pair(residualSwitch[*rpItr], -0.001));
                            }
                        }
                        

                        
                        if (!precPrefs.empty()) {
                            const int pps = precPrefs.size();
                            for (int pi = 0; pi < pps; ++pi) {          
                                const int currPref = precPrefs[pi];
                                
                                if (!RPGBuilder::getPreferences()[currPref].neverTrue) continue;
                                
                                switchVarForPreference[currPref].setCountColumn(newCol);
                            }
                        }
                                                                                        
                        if (propositionalActionCosts) {
                            preferEarlier[newCol] = (*propositionalActionCosts)[i];
                        } else {
                            preferEarlier[newCol] = lWeight;
                        }
                        newActData.Avar = newCol;

                        actionAppearedInLayer[i] = newTS;
                        
                        if (assigns) {
                            lp->addCol(newColVec, 0, 1, 0, (integerLevel >= 1 ? MILPSolver::C_BOOL : MILPSolver::C_REAL));
                            newActData.isBinary = true;
                            if (alternativeObjective) {
                                if (layerCount == 1) {
                                    newActData.switchVar = newCol;
                                    preferEarlier[newCol] = -1.0;
                                }
                            }
                        } else {
                            if (alternativeObjective) {
                                lp->addCol(newColVec, 0, 0, 0, MILPSolver::C_REAL);
                                if (layerCount == 1) {
                                    lp->addCol(emptyVector, 0, 1, 0, MILPSolver::C_BOOL);
                                    newActData.switchVar = colCount;

                                    preferEarlier[colCount] = 1.0;
                                    goesToRealAction[colCount] = -1;


                                    ++colCount;

                                    if (nameRowsAndColumns) {
                                        ostringstream actName;
                                        actName << "sv" << *(RPGBuilder::getInstantiatedOp(i));
                                        lp->setColName(newActData.switchVar, actName.str());
                                    }


                                    vector<pair<int, double> > sc(2);

                                    sc[0] = make_pair(newActData.switchVar, BIG);
                                    sc[1] = make_pair(newActData.Avar, -1);

                                    lp->addRow(sc, 0.0, lp->getInfinity());

                                    if (nameRowsAndColumns) {
                                        ostringstream actName;
                                        actName << "sc" << *(RPGBuilder::getInstantiatedOp(i));
                                        lp->setRowName(lp->getNumRows() - 1, actName.str());
                                    }
                                }
                            } else {

                                if (secondaryIntegerLevel == 1 && affectsGoalVar) {
                                    lp->addCol(newColVec, 0, 0, 0, MILPSolver::C_INT);
                                } else {
                                    switch (integerLevel) {
                                        case 0:
                                        case 1:
                                            lp->addCol(newColVec, 0, 0, 0, MILPSolver::C_REAL);
                                            break;
                                        case 2:
                                            lp->addCol(newColVec, 0, 0, 0, (layerCount == 1 ? MILPSolver::C_INT : MILPSolver::C_REAL));
                                            break;
                                        case 3:
                                        {
                                            if (layerCount == 1) {
                                                lp->addCol(newColVec, 0, 0, 0, MILPSolver::C_INT);
                                            } else {                                        
                                                bool goalAchieving = false;
                                                if (!factsThatWillBeConsideredAsGoals.empty()) {
                                                    list<Literal*>::const_iterator effItr = RPGBuilder::getStartAddEffects()[i].begin();
                                                    const list<Literal*>::const_iterator effEnd = RPGBuilder::getStartAddEffects()[i].end();
                                                    
                                                    for (; effItr != effEnd; ++effItr) {
                                                        if (factsThatWillBeConsideredAsGoals.find((*effItr)->getStateID()) != factsThatWillBeConsideredAsGoals.end()) {
                                                            goalAchieving = true;
                                                            break;
                                                        }
                                                    }
                                                }
                                                lp->addCol(newColVec, 0, 0, 0, (goalAchieving ? MILPSolver::C_INT : MILPSolver::C_REAL));
                                            }
                                            break;
                                        }
                                        case 4:
                                        {
                                            lp->addCol(newColVec, 0, 0, 0, MILPSolver::C_INT);
                                            break;
                                        }
                                        default:
                                        {
                                            cout << "Error, integer level for the LP should be in the range 0 to 4, inclusive\n";
                                            exit(1);
                                        }
                                    }
                                    
                                }
                            }
                        }
                        presolved = false;

                        if (nameRowsAndColumns) {
                            ostringstream actName;
                            actName << *(RPGBuilder::getInstantiatedOp(i));
                            lp->setColName(newCol, actName.str());
                        }
                        ivList.push_back(newCol);


                        assert(colCount == lp->getNumCols());
                        goesToRealAction[newCol] = i;
                        if (layerCount == 1) {
                            oldBack->intVariables = true;
                        }

                        if (howMany != INT_MAX) {
                            newActData.kShot = howMany;
//       if (howMany == 1) set_binary(lp, newCol, TRUE);

                        } else {
                            newActData.kShot = -1;
                        }

                    } else {
                        if (debugAdd) cout << "- Actually, not bothering - can only be applied " << howMany << " times\n";
                    }
                }
            }
        }
    }

    if (false && layerCount == 1 && actAdded) {
        cout << "Actions in layer 1: " << actAdded << "\n";
    }
    {
        map<int, ActData>::iterator lavItr = layerActionVariables.begin();
        const map<int, ActData>::iterator lavEnd = layerActionVariables.end();

        for (; lavItr != lavEnd; ++lavItr) {
            if (lavItr->second.isBinary) continue;
            int & colNo = lavItr->second.Avar;
            int & kShot = lavItr->second.kShot;
            int currUpBo = lp->getColUpper(colNo);
            if (currUpBo < kShot || kShot == -1) {
                currUpBo += 1000;
                if (kShot != -1 && currUpBo > kShot) currUpBo = kShot;
                lp->setColUpper(colNo, currUpBo);
//    cout << "Upper bound on column " << colNo << " now " << currUpBo << "\n";
            }
        }
    }

    /* for (int v = 0; v < varCount; ++v) {
      if (everAssigned[v]) {
       set_rh(lp, minMaxVars[v].assignRow, layerCount);
      }
     }
    */

    if (debugAdd) {
        cout << "Done adding new actions\n";
    }
};

bool MILPRPG::mustAchieveLocalFact(const int & fID, vector<int> * const propPrec, vector<int> * const numPrec,
                                   map<int,double> & propsSatisfied, int & needToCheck)
{
    static const bool landmarkAndGoalsDebug = false;
    
    MILPSolver * const lp = goalLP->lp;
    
    int actID;
    
    const list<pair<int, VAL::time_spec> > & achievers = RPGBuilder::getEffectsToActions()[fID];
    //cout << "Working backwards from " << *(RPGBuilder::getLiteral(fID)) << " - has " << achievers.size() << " achievers\n";
    bool canBeSatisfied = false;    
    {

        list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
        const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();
        
        for (; accItr != accEnd; ++accItr) {
            actID = accItr->first;
            if (!(*propPrec)[actID] && !(*numPrec)[actID]) {
                if (!fluentAction[actID]) {
                    if (landmarkAndGoalsDebug) {
                        cout << "Not adding " << *(RPGBuilder::getLiteral(fID)) << " to LP: can use the numeric-free achiever " << *(RPGBuilder::getInstantiatedOp(actID)) << endl;                
                    }
                    return true;
                } else {
                    canBeSatisfied = true;
                }
            }
        }
    }
    
    if (!canBeSatisfied) {
        return false;
    }
    
    vector<pair<int, double> > entries;
    entries.reserve(achievers.size());

    {
        list<pair<int, VAL::time_spec> >::const_iterator accItr = achievers.begin();
        const list<pair<int, VAL::time_spec> >::const_iterator accEnd = achievers.end();
        
        for (; accItr != accEnd; ++accItr) {
            actID = accItr->first;
            if (!(*propPrec)[actID] && !(*numPrec)[actID]) {
    
                const map<int, ActData>::const_iterator avItr = layerActionVariables.find(actID);
                
                if (avItr == layerActionVariables.end()) {
                    cerr << "Internal error adding " << *(RPGBuilder::getLiteral(fID)) << " to LP: missing action variable for " << *(RPGBuilder::getInstantiatedOp(actID)) << endl;
                    exit(1);
                }
                
                entries.push_back(make_pair(avItr->second.Avar, 1.0));

            }
        }

    }
    
    assert(!entries.empty());
    
    lp->addRow(entries, 1.0, lp->getInfinity());
    if (nameRowsAndColumns) {
        ostringstream rn;
        rn << "Achieve " << *(RPGBuilder::getLiteral(fID));
        lp->setRowName(lp->getNumRows() - 1, rn.str());
    }
    propsSatisfied.insert(make_pair(fID, DBL_MAX));
    ++needToCheck;
    
        
    return true;
}

bool MILPRPG::mustAchieveFact(const int & g, const int & fID, vector<int> * const propPrec, vector<int> * const numPrec,
                              map<int,double> & propsSatisfied, int & needToCheck)
{
    static const bool landmarkAndGoalsDebug = false;
    
    MILPSolver * const lp = goalLP->lp;
    
    const int getOutCount = propositionalGoalNotEncoded[g].size();
    bool mustAddConstraint = true;
    int actID;

    for (int ex = 0; ex < getOutCount; ++ex) {
        actID = propositionalGoalNotEncoded[g][ex];
        if (!(*propPrec)[actID] && !(*numPrec)[actID]) {
            if (landmarkAndGoalsDebug) {
                if (fID < 0) {
                    cout << "Not adding landmark " << *(RPGBuilder::getLiteral(LandmarksAnalysis::getIndividualLandmarkFacts()[-1-fID])) << " to LP: can use the numeric-free achiever " << *(RPGBuilder::getInstantiatedOp(actID)) << endl;
                } else {
                    cout << "Not adding " << *(RPGBuilder::getLiteral(fID)) << " to LP: can use the numeric-free achiever " << *(RPGBuilder::getInstantiatedOp(actID)) << endl;
                }
            }
            mustAddConstraint = false;
            break;
        }
    }

    if (mustAddConstraint) {
        const int vecSize = propositionalGoalConstraints[g].size();

        vector<pair<int, double> > entries;
        entries.reserve(vecSize);

        int finalVS = 0;
        for (int v = 0; v < vecSize; ++v) {
            actID = propositionalGoalConstraints[g][v];
            if (!(*propPrec)[actID] && !(*numPrec)[actID]) {

                const map<int, ActData>::const_iterator avItr = layerActionVariables.find(actID);
                
                if (avItr == layerActionVariables.end()) {
                    if (fID < 0) {
                        cout << "Error adding landmark " << *(RPGBuilder::getLiteral(LandmarksAnalysis::getIndividualLandmarkFacts()[-1-fID])) << " to LP: missing action variable for " << *(RPGBuilder::getInstantiatedOp(actID)) << endl;
                    } else {
                        cout << "Error adding " << *(RPGBuilder::getLiteral(fID)) << " to LP: missing action variable for " << *(RPGBuilder::getInstantiatedOp(actID)) << endl;
                    }
                }
                
                assert(avItr != layerActionVariables.end());

                entries.push_back(make_pair(avItr->second.Avar, 1.0));

                ++finalVS;
            }
        }

        if (finalVS) {
            lp->addRow(entries, 1.0, lp->getInfinity());
            if (nameRowsAndColumns) {
                ostringstream rn;
                rn << "Achieve " << *(RPGBuilder::getLiteral(fID));
                lp->setRowName(lp->getNumRows() - 1, rn.str());
            }
            propsSatisfied.insert(make_pair(fID, DBL_MAX));
            ++needToCheck;

        } else {

            return false;
        }
    }
    
    return true;
}


template <typename T, typename S>
bool addInternalIntegerDisjunctiveLandmarkEncoding(const int & disjunctiveLandmarkID, vector<vector<int> > & disjunctiveLandmarkGroupVar,
                                                   MinimalState * const forKShot, const bool & useLocalLandmarks,
                                                   const int & mCount, const T & memberBegin, const T & memberEnd,
                                                   const S & dummy,
                                                   MILPSolver * const lp, map<int,int> & lpVarToDenoteThatWeHaveAGivenFact,
                                                   const map<int, MILPRPG::ActData> & layerActionVariables,
                                                   const vector<int> * const propPrec, const vector<int> * const numPrec, int & needToCheck) {
    

    static const bool debug = false;
    
    disjunctiveLandmarkGroupVar[disjunctiveLandmarkID].resize(mCount, -1);
    
    //int lCount;
    //int lID;
    int fID;
    //int l;
    // First, check if any of the members is already satisfied
    
    {
        T memberItr = memberBegin;
        
        for (; memberItr != memberEnd; ++memberItr) {
            
            S iItr = memberItr->begin();
            const S iEnd = memberItr->end();
            
            for (; iItr != iEnd; ++iItr) {
                if (!useLocalLandmarks) {
                    if (LandmarksAnalysis::factHasBeenSeen(*iItr, forKShot) != 1) {
                        break;
                    }
                } else {
                    
                    if (forKShot->first.find(*iItr) == forKShot->first.end()) {
                        break;
                    }
                }
            }
            if (iItr == iEnd) {
                if (debug) {
                    cout << "Don't need to add disjunctive landmark " << disjunctiveLandmarkID << " - option has already been met\n";
                }
                // have found a possible means of satisfying this disjunctive landmark that has already been
                // met
                return true;
            }
        }
    }

    static const vector<pair<int,double> > emptyVector;
    
    bool numericFreeMember = false;
    bool anyPossibleTrue = false;
    
    vector<pair<int,double> > oneMemberConstraint;    
    
    oneMemberConstraint.reserve(mCount);
    
    int m = 0;
    for (T memberItr = memberBegin; memberItr != memberEnd; ++memberItr, ++m) {
        
        list<int> varsThatMustBeTrueForThisOption;
    
        bool canBeSatisfied = true;
        
        S iItr = memberItr->begin();
        const S iEnd = memberItr->end();
        
        for (; iItr != iEnd; ++iItr) {
            
            fID = *iItr;
            
            if (!useLocalLandmarks)  {
                if (LandmarksAnalysis::factHasBeenSeen(*iItr, forKShot) == 1) {
                    continue;
                }
            } else {
                if (forKShot->first.find(*iItr) == forKShot->first.end()) {
                    continue;
                }
            }
            
            map<int, int>::iterator varForThisFact = lpVarToDenoteThatWeHaveAGivenFact.insert(make_pair(fID,-1)).first;
            
            if (varForThisFact->second == -1) {
                vector<pair<int,double> > constraintForThisFact;
                
                const list<pair<int, VAL::time_spec> > & eta = RPGBuilder::getEffectsToActions()[fID];
                
                list<pair<int, VAL::time_spec> >::const_iterator etaItr = eta.begin();
                const list<pair<int, VAL::time_spec> >::const_iterator etaEnd = eta.end();
                
                constraintForThisFact.reserve(eta.size() + 1);
                
                int actID;
                bool numericFreeAchiever = false;
                
                canBeSatisfied = false;
                for (; etaItr != etaEnd; ++etaItr) {
                    actID = etaItr->first;
                    if (!(*propPrec)[actID] && !(*numPrec)[actID]) {                        
                        canBeSatisfied = true;
                        if (!MILPRPG::actionNeedsNumerics(actID)) {
                            numericFreeAchiever = true;
                            break;
                        } else {
                            const map<int, MILPRPG::ActData>::const_iterator avItr = layerActionVariables.find(actID);
                            assert(avItr != layerActionVariables.end());
                            constraintForThisFact.push_back(make_pair(avItr->second.Avar, 1.0));
                        }
                    }
                }
                
                if (numericFreeAchiever) {
                    varForThisFact->second = -2;
                } else {
                    lp->addCol(emptyVector, 0.0, 1.0, 0.0, MILPSolver::C_BOOL);
                    varForThisFact->second = lp->getNumCols() - 1;                    
                    
                    if (nameRowsAndColumns) {
                        ostringstream rName;
                        rName << "Added" << *(RPGBuilder::getLiteral(fID));
                        string asString = rName.str();
                        lp->setColName(varForThisFact->second, asString);
                        
                    }
                    
                    constraintForThisFact.push_back(make_pair(varForThisFact->second, -1.0));
                    lp->addRow(constraintForThisFact, 0.0, lp->getInfinity());
                    
                    
                    if (nameRowsAndColumns) {
                        ostringstream rName;
                        rName << "Req" << *(RPGBuilder::getLiteral(fID));
                        string asString = rName.str();
                        lp->setRowName(lp->getNumRows() - 1, asString);                        
                    }

                    constraintForThisFact.back().second = -BIG;
                    lp->addRow(constraintForThisFact, -lp->getInfinity(), 0.999);
                    
                    if (nameRowsAndColumns) {
                        ostringstream rName;
                        rName << "Have" << *(RPGBuilder::getLiteral(fID));
                        string asString = rName.str();
                        lp->setRowName(lp->getNumRows() - 1, asString);                        
                    }
                    
                }
            }
            
            if (varForThisFact->second >= 0) {
                varsThatMustBeTrueForThisOption.push_back(varForThisFact->second);
            }
        }
        
        
        if (canBeSatisfied) {
            anyPossibleTrue = true;
        }
        
        if (!varsThatMustBeTrueForThisOption.empty()) {

            // If all possible ways of satisfying this option for the disjunctive landmark have numeric side-effects,
            // then we get here
            
            lp->addCol(emptyVector, 0.0, 1.0, 0.0, MILPSolver::C_BOOL);                                  
            const int colForThisOption = lp->getNumCols() - 1;
            
            if (nameRowsAndColumns) {
                ostringstream rName;
                rName << "DL" << disjunctiveLandmarkID << "M" << m;
                string asString = rName.str();
                lp->setColName(colForThisOption, asString);                
            }
            
            oneMemberConstraint.push_back(make_pair(colForThisOption,1.0));
            
            disjunctiveLandmarkGroupVar[disjunctiveLandmarkID][m] = colForThisOption;
            
            vector<pair<int,double> > binaryConstraint(2);
            
            list<int>::const_iterator mvItr = varsThatMustBeTrueForThisOption.begin();
            const list<int>::const_iterator mvEnd = varsThatMustBeTrueForThisOption.end();
            
            for (; mvItr != mvEnd; ++mvItr) {
                
                // this member is true >= the group is true
                
                binaryConstraint[0].first = *mvItr;
                binaryConstraint[0].second = 1.0;
                
                binaryConstraint[1].first = colForThisOption;
                binaryConstraint[1].second = -1.0;
                                
                lp->addRow(binaryConstraint, 0.0, lp->getInfinity());
                if (nameRowsAndColumns) {
                    ostringstream rName;
                    rName << "DL" << disjunctiveLandmarkID << "M" << m << "v" << *mvItr;
                    string asString = rName.str();
                    lp->setRowName(lp->getNumRows() - 1, asString);
                }
            }
            
        } else {
            
            if (debug) {
                cout << "Don't need to add disjunctive landmark " << disjunctiveLandmarkID << " as member " << m << " can be satisfied without numeric side-effects\n";
            }
            numericFreeMember = true;
            break;
        }
                        
    }
    
    if (!numericFreeMember) {
        lp->addRow(oneMemberConstraint, 1.0, lp->getInfinity());
        if (nameRowsAndColumns) {
            ostringstream rName;
            rName << "DL" << disjunctiveLandmarkID;
            string asString = rName.str();
            lp->setRowName(lp->getNumRows() - 1, asString);
        }
    }
    
    return anyPossibleTrue;
    
};

bool MILPRPG::addIntegerDisjunctiveLandmarkEncoding(const int & disjunctiveLandmarkID, MinimalState * forKShot, 
                                                    vector<int> * const propPrec, vector<int> * const numPrec, int & needToCheck)
{

    const vector<vector<int> > & details = LandmarksAnalysis::getDisjunctiveLandmarks()[disjunctiveLandmarkID];
            
    const int mCount = details.size();    
    
    vector<int> dummyVec;
    const vector<int>::const_iterator dummy = dummyVec.begin();
    const vector<vector<int> >::const_iterator memberBegin = details.begin();
    const vector<vector<int> >::const_iterator memberEnd = details.end();
    const bool toPass = false;
    MILPSolver * const lp = goalLP->lp;
    
    return addInternalIntegerDisjunctiveLandmarkEncoding(disjunctiveLandmarkID, disjunctiveLandmarkGroupVar,
                                                         forKShot, toPass, mCount, memberBegin, memberEnd,
                                                         dummy, lp, lpVarToDenoteThatWeHaveAGivenFact,
                                                         layerActionVariables, propPrec, numPrec, needToCheck);
}

bool MILPRPG::addIntegerDisjunctiveLocalLandmarkEncoding(const int & disjunctiveLandmarkID, MinimalState * forKShot,
                                                        const list<set<int> > & details,
                                                        vector<int> * const propPrec, vector<int> * const numPrec,
                                                        int & needToCheck) {
    
    const int mCount = details.size();
    
    set<int> dummySet;
    
    const set<int>::const_iterator dummy = dummySet.begin();
    const list<set<int> >::const_iterator memberBegin = details.begin();
    const list<set<int> >::const_iterator memberEnd = details.end();
    const bool toPass = true;
    MILPSolver * const lp = goalLP->lp;
    
    return addInternalIntegerDisjunctiveLandmarkEncoding(disjunctiveLandmarkID, disjunctiveLandmarkGroupVar,
                                                         forKShot, toPass, mCount, memberBegin, memberEnd,
                                                         dummy, lp, lpVarToDenoteThatWeHaveAGivenFact,
                                                         layerActionVariables, propPrec, numPrec, needToCheck);
    
}

struct DLFactDetails {
    bool canBeReached;
    bool canBeReachedWithoutNumerics;
    bool addedToConstraint;
    
    DLFactDetails() 
        : canBeReached(false), canBeReachedWithoutNumerics(false), addedToConstraint(false) {
    }
};

bool MILPRPG::addLinearDisjunctiveLandmarkEncoding(const int & disjunctiveLandmarkID, MinimalState * forKShot, 
                                                   vector<int> * const propPrec, vector<int> * const numPrec, int & needToCheck)
{

    const vector<vector<int> > & details = LandmarksAnalysis::getDisjunctiveLandmarks()[disjunctiveLandmarkID];
    

    
    const int mCount = details.size();
    int lCount;
    //int lID;
    int fID;
    int l;
    // First, check if any of the members is already satisfied
    
    for (int m = 0; m < mCount; ++m) {
        lCount = details[m].size();
        
        for (l = 0; l < lCount; ++l) {
            if (LandmarksAnalysis::factHasBeenSeen(details[m][l], forKShot) != 1) {
                break;
            }
        }
        if (l == lCount) {
            // have found a possible means of satisfying this disjunctive landmark that has already been
            // met
            return true;
        }
    }

    MILPSolver * const lp = goalLP->lp;

    bool anyPossibleTrue = false;
    int minGroupSize = INT_MAX;
    
    map<int,DLFactDetails> alreadyContainsAchieversForFact;
    map<int,double> actionVarToWeight;
    
    for (int m = 0; m < mCount; ++m) {
        lCount = details[m].size();
        int groupSize = lCount;
        bool canBeSatisfied = true;
        
        for (l = 0; l < lCount; ++l) {
            fID = details[m][l];
            if (LandmarksAnalysis::factHasBeenSeen(fID, forKShot) == 1) {
                --groupSize;
                continue;
            }
            pair<map<int,DLFactDetails>::iterator,bool> kItrPair = alreadyContainsAchieversForFact.insert(make_pair(fID, DLFactDetails()));
            
            if (kItrPair.second) {
                int actID;
                const list<pair<int, VAL::time_spec> > & eta = RPGBuilder::getEffectsToActions()[fID];
                list<pair<int, VAL::time_spec> >::const_iterator etaItr = eta.begin();
                const list<pair<int, VAL::time_spec> >::const_iterator etaEnd = eta.end();
                
                for (; etaItr != etaEnd; ++etaItr) {
                    actID = etaItr->first;
                    if (!(*propPrec)[actID] && !(*numPrec)[actID]) {                                                    
                        kItrPair.first->second.canBeReached = true;
                        if (!fluentAction[actID]) {
                            kItrPair.first->second.canBeReachedWithoutNumerics = true;
                            break;
                        }
                    }
                }
            }
            
            if (!kItrPair.first->second.canBeReached) {
                canBeSatisfied = false;
                break;
            }
            if (kItrPair.first->second.canBeReachedWithoutNumerics) {
                --groupSize;
                continue;
            }
                
            
        }
        
        
        if (!canBeSatisfied) continue;
        
        anyPossibleTrue = true;
        
        if (!groupSize) return true;
        
        if (groupSize < minGroupSize) {
            minGroupSize = groupSize;
        }
        
        for (l = 0; l < lCount; ++l) {
            
            fID = details[m][l];

            if (LandmarksAnalysis::factHasBeenSeen(fID, forKShot) == 1) {
                continue;
            }
            
            map<int,DLFactDetails>::iterator fItr = alreadyContainsAchieversForFact.find(fID);
            assert(fItr != alreadyContainsAchieversForFact.end());
            
            if (!fItr->second.addedToConstraint) {
                int actID;
                const list<pair<int, VAL::time_spec> > & eta = RPGBuilder::getEffectsToActions()[fID];
                {
                    list<pair<int, VAL::time_spec> >::const_iterator etaItr = eta.begin();
                    const list<pair<int, VAL::time_spec> >::const_iterator etaEnd = eta.end();
                    
                    for (; etaItr != etaEnd; ++etaItr) {
                        actID = etaItr->first;
                        if (!(*propPrec)[actID] && !(*numPrec)[actID]) {                                                    
                            assert(fluentAction[actID]);
                            const map<int, ActData>::const_iterator avItr = layerActionVariables.find(actID);
                            assert(avItr != layerActionVariables.end());
                            
                            actionVarToWeight.insert(make_pair(avItr->second.Avar,0.0)).first->second += 1.0;
                        }
                    }
                }
            }            
        }
    }

    if (!actionVarToWeight.empty()) {
        assert(minGroupSize);
        
        vector<pair<int,double> > constraintDetails;
        constraintDetails.reserve(actionVarToWeight.size());
        constraintDetails.insert(constraintDetails.end(), actionVarToWeight.begin(), actionVarToWeight.end());
        
        lp->addRow(constraintDetails, minGroupSize, lp->getInfinity());
        
    }
    
    return anyPossibleTrue;
}



bool MILPRPG::canSatisfyGoalConstraints(double & prefCost, const double & maxPrefCost, MinimalState * forKShot,
                                        const vector<AutomatonPosition> & preferenceStatusesInRPG,
                                        vector<int> * const propPrec, vector<int> * const numPrec,
                                        map<int,double> & propsSatisfied, set<int> & prefsUnsatisfied)
{
    static const bool debug = false;

    static int iteration = 0;


    ++iteration;

    prefCost = 0;

    if (!varCount || !addNumGoalConjunct) return true;

    delete goalLP;
    goalLP = 0;
    
    prefsUnsatisfied.clear();
    lpVarToDenoteThatWeHaveAGivenFact.clear();
    disjunctiveLandmarkGroupVar.clear();
    
    const int tsvCount = RPGBuilder::getTaskPrefCount();
    const int asvCount = switchVarForPreference.size();
    
    int prefsChecked = 0;    
    
    double costOfPrefsThatMustBeViolated = 0.0;
    
    {
        for (int sv = 0; sv < tsvCount; ++sv) {
            if (!canBeSatisfied(forKShot->preferenceStatus[sv])) continue;
            if (!isSatisfied(preferenceStatusesInRPG[sv])) {
                costOfPrefsThatMustBeViolated += RPGBuilder::getPreferences()[sv].cost;
            } else {
                ++prefsChecked;
            }
        }
    }
    
    if (maxPrefCost != DBL_MAX) {
        const double leftover = maxPrefCost - costOfPrefsThatMustBeViolated;
        if (leftover <= 0.000001) {
            return false;
        }
    }
    
    
    assert(allLp.back().first->lp);

    goalLP = new LPAndBounds(allLp.back().first->lp->clone(), allLp.back().first->presolved, allLp.back().first->intVariables, allLp.back().first->residualSwitch, varCount);

    MILPSolver* const lp = goalLP->lp;
    bool & presolved = goalLP->presolved;

    if (debug) {
        cout << "Incoming LP";
        if (presolved) cout << " is presolved, and";
        cout << " has " << lp->getNumCols() << " columns and " << lp->getNumRows() << " rows";
        //if (presolved) cout << " (originally " << get_Norig_columns(lp) << " x " << get_Norig_rows(lp) << ")";
        cout << endl;

    }

    propsSatisfied.clear();

    int needToCheck = 0;

    {
        const int bgCount = goalVarBounds.size();
        for (int g = 0; g < bgCount; ++g) {
            const int RI = minMaxVars[goalVarBounds[g].v].Vvar;

            switch (goalVarBounds[g].op) {
            case -1: {
                if (debug) cout << "Adding bound: column " << RI << " " << lp->getColName(RI) << " <= " << goalVarBounds[g].w << endl;
                lp->setColUpper(RI, goalVarBounds[g].w);
                break;
            }
            case 0: {
                if (debug) cout << "Adding bound: column " << RI << " " << lp->getColName(RI)  << " == " << goalVarBounds[g].w << endl;
                lp->setColBounds(RI, goalVarBounds[g].w, goalVarBounds[g].w);
                break;
            }
            case 1: {
                if (debug) cout << "Adding bound: column " << RI << " " << lp->getColName(RI) << " >= " << goalVarBounds[g].w << endl;
                lp->setColLower(RI, goalVarBounds[g].w);
                break;
            }
            default: {
                std::cerr << "Unexpected bound type\n";
                exit(1);
            }
            }
            ++needToCheck;

        }
    }

    {
        const int bgCount = goalConstraints.size();
        for (int g = 0; g < bgCount; ++g) {
            if (debug) cout << "Adding goal constraint " << g << endl;
            const GoalConstraint & cons = goalConstraints[g];
            const int vecSize = cons.lhs.size();

            vector<pair<int, double> > entries;
            entries.reserve(vecSize);

            double finalRHS = cons.rhs;
            int finalVS = 0;
            for (int v = 0; v < vecSize; ++v) {
                const int RI = minMaxVars[cons.lhs[v].first].Vvar;
                entries.push_back(make_pair(RI, cons.lhs[v].second));
                ++finalVS;
            }

            assert(finalVS);

            switch (cons.op) {
            case -1:
                lp->addRow(entries, -lp->getInfinity(), finalRHS);
                break;
            case 0:
                lp->addRow(entries, finalRHS, finalRHS);
                break;
            case 1:
                lp->addRow(entries, finalRHS, lp->getInfinity());
                break;
            default:
                cerr << "Internal error: unknown goal constraint type, " << cons.op << endl;
                exit(1);
            }


            ++needToCheck;


        }
    }

    set<int> localIndividualLandmarks;
    list<list<set<int> > > localDisjunctiveLandmarks;
                

    const bool landmarkAndGoalsDebug = debug;

    if (addProps) {
        
        // first, deal with simple goals
        
        const int bgCount = propositionalGoals.size();
        for (int g = 0; g < bgCount; ++g) {
            int fID = propositionalGoals[g];
            if (fID < 0) {
                continue;
            }
            if (forKShot->first.find(fID) != forKShot->first.end()) {
                if (landmarkAndGoalsDebug) {
                    cout << "Not adding " << *(RPGBuilder::getLiteral(fID)) << " to LP: already true in state\n";
                }
                // goal already true in state - no need to add actions to achieve it
                continue;
            }
        
                    
            if (fID >= 0) {                
                if (!mustAchieveFact(g, fID, propPrec, numPrec, propsSatisfied, needToCheck)) {
                    return false;
                }
            }            
        }
        
        if (addLandmarks) {
            if (useLocalLandmarks) {

                LandmarksAnalysis::localLandmarksAnalysis(*forKShot, propPrec, numPrec, true,
                                                          localIndividualLandmarks,
                                                          localDisjunctiveLandmarks);
                                                          
                
                set<int>::const_iterator fItr = localIndividualLandmarks.begin();
                const set<int>::const_iterator fEnd = localIndividualLandmarks.end();
                
                for (; fItr != fEnd; ++fItr) {
                    if (!mustAchieveLocalFact(*fItr, propPrec, numPrec, propsSatisfied, needToCheck)) {
                        return false;
                    }
                }
                
                disjunctiveLandmarkGroupVar.resize(localDisjunctiveLandmarks.size());
                
                list<list<set<int> > >::const_iterator dlItr = localDisjunctiveLandmarks.begin();
                const list<list<set<int> > >::const_iterator dlEnd = localDisjunctiveLandmarks.end();
                
                for (int dlID = 0; dlItr != dlEnd; ++dlItr, ++dlID) {
                    if (!addIntegerDisjunctiveLocalLandmarkEncoding(dlID, forKShot, *dlItr, propPrec, numPrec, needToCheck)) {
                        return false;
                    }
                }
            } else {
                disjunctiveLandmarkGroupVar.resize(LandmarksAnalysis::getDisjunctiveLandmarks().size());
            
                const int individualLandmarkCount = LandmarksAnalysis::getIndividualLandmarkFacts().size();
                const int bgCount = propositionalGoals.size();
                for (int g = 0; g < bgCount; ++g) {

                    int fID = propositionalGoals[g];
                    int disjunctiveLandmarkID = -1;
                    
                    if (fID >= 0) continue;
                    
                                    
                    disjunctiveLandmarkID = -1 - fID;
                        
                    if (disjunctiveLandmarkID >= individualLandmarkCount) {
                        disjunctiveLandmarkID -= individualLandmarkCount;
                    } else {
                        if (forKShot->landmarkStatus[disjunctiveLandmarkID]) {
                            if (landmarkAndGoalsDebug) {
                                cout << "Not adding landmark " << *(RPGBuilder::getLiteral(LandmarksAnalysis::getIndividualLandmarkFacts()[-1-fID])) << " to LP: already been seen\n";
                            }
                            continue; // landmark has already been seen
                        }
                        fID = LandmarksAnalysis::getIndividualLandmarkFacts()[disjunctiveLandmarkID];
                        disjunctiveLandmarkID = -1;
                        
                    }                    
                    
                    if (fID >= 0) {
                        
                        if (!mustAchieveFact(g, fID, propPrec, numPrec, propsSatisfied, needToCheck)) {
                            return false;
                        }
                        
                    } else {
                        
                        if (landmarkAndGoalsDebug) {
                            cout << "Adding disjunctive landmark " << disjunctiveLandmarkID << endl;
                        }
                        
                        if (integerDisjunctiveLandmarksEncoding) {
                            if (!addIntegerDisjunctiveLandmarkEncoding(disjunctiveLandmarkID, forKShot, propPrec, numPrec, needToCheck)) {
                                if (landmarkAndGoalsDebug) {
                                    cout << "Disjunctive landmark cannot be reached yet, returning false\n";
                                }
                                return false;
                            }
                        } else {
                            if (!addLinearDisjunctiveLandmarkEncoding(disjunctiveLandmarkID, forKShot, propPrec, numPrec, needToCheck)) {
                                if (landmarkAndGoalsDebug) {
                                    cout << "Disjunctive landmark cannot be reached yet, returning false\n";
                                }                        
                                return false;                        
                            }
                        }
                                        
                    }
                }
            }
        }
    }

    prefsUnsatisfied.clear();

    {
        
        
        for (int sv = 0; sv < tsvCount; ++sv) {
            if (!canBeSatisfied(forKShot->preferenceStatus[sv])) continue;
            const SwitchVarInfo & svp = switchVarForPreference[sv];
            if (svp.hasBeenViolatedCol == -1) {
                // ..then the truth value of this preference is determined *entirely* in the RPG
                if (!isSatisfied(preferenceStatusesInRPG[sv])) {
                    if (PreferenceHandler::preferenceDebug) cout << "RPG only preference " << RPGBuilder::getPreferences()[sv].name << " (" << sv << ") is unsatisfied\n";
                    prefCost += RPGBuilder::getPreferences()[sv].cost;
                    prefsUnsatisfied.insert(sv);
                }
            } else {
                if (!isSatisfied(preferenceStatusesInRPG[sv])) {
                    // then the RPG is saying this doesn't even look satisfiable yet, so bound variable appropriately - i.e. force violation
                    lp->setColLower(svp.hasBeenViolatedCol, 1.0);
                    if (PreferenceHandler::preferenceDebug) cout << "Forcibly breaking preference " << RPGBuilder::getPreferences()[sv].name << " (" << sv << "), as suggested by the RPG\n";
                }
            }
        }
    }

    int maxPrefRow = -1;
    
    if (maxPrefCost != DBL_MAX) {
        const double leftover = maxPrefCost - costOfPrefsThatMustBeViolated;
        assert(leftover > 0.000001);
        
        vector<pair<int, double> > entries;
        entries.reserve(asvCount);
        
        //cout << endl;
        
        if (PreferenceHandler::preferenceDebug) {
            cout << "Setting preference objectives for:";
        }
        
        for (int sv = 0; sv < asvCount; ++sv) {
            const SwitchVarInfo & svp = switchVarForPreference[sv];
            if (svp.useInObjective == -1) continue;
            
            if (isSatisfied(preferenceStatusesInRPG[sv])) {
            
                const double pCost = RPGBuilder::getPreferences()[sv].cost;
                if (pCost <= leftover) {
                    entries.push_back(make_pair(svp.useInObjective, pCost));
                    //cout << RPGBuilder::getPreferences()[sv].cost << "." << svp.first << endl;
                    if (PreferenceHandler::preferenceDebug) {
                        cout << " " << sv;
                    }
                    
                    if (sv >= tsvCount) {
                        // if it's a precondition preference, we can only apply it as many times as we can afford
                        const int safeToApply = leftover / pCost;
                        lp->setColUpper(svp.countingCol, safeToApply);
                    }
                    
                } else {                                        
                    lp->setColUpper(svp.hasBeenViolatedCol, 0);
                    --prefsChecked;                    
                }
            }
        }
                
        if (PreferenceHandler::preferenceDebug) {
            cout << endl;
        }
                        
        if (!entries.empty()) {
            //cout << " <= " << maxPrefCost << endl;
            lp->addRow(entries, 0.0, leftover);
            maxPrefRow = lp->getNumRows() - 1;
        }
    }

    const int lpColCount = lp->getNumCols();
    
    double * const wVec = new double[lpColCount];
    
    int primaryObjectiveRow = -1;
    
    primaryObjectiveRowReset = make_pair(-1, lp->getInfinity());

    if (useSecondaryObjective && prefsChecked) {
        for (int c = 0; c < lpColCount; ++c) {
            wVec[c] = 0.0;
        }
            
        for (int sv = 0; sv < asvCount; ++sv) {
            const SwitchVarInfo & svp = switchVarForPreference[sv];
            if (svp.useInObjective == -1) continue;
            wVec[svp.useInObjective] = RPGBuilder::getPreferences()[sv].cost;                        
        }
        
        if (maxPrefRow == -1) {
            vector<pair<int,double> > newRow(prefsChecked);
            
            int pcol = 0;
            for (int sv = 0; sv < asvCount; ++sv) {
                const SwitchVarInfo & svp = switchVarForPreference[sv];
                if (svp.useInObjective == -1) continue;
                newRow[pcol] = make_pair(svp.useInObjective, RPGBuilder::getPreferences()[sv].cost);
                ++pcol;
            }
            lp->addRow(newRow, 0.0, lp->getInfinity());
            primaryObjectiveRow = lp->getNumRows() - 1;
            primaryObjectiveRowReset.second = lp->getInfinity();
            if (nameRowsAndColumns) {
                lp->setRowName(primaryObjectiveRow, "primaryobjective");
            }
                                                        
                                                                        
        } else {
            primaryObjectiveRow = maxPrefRow;
            primaryObjectiveRowReset.second = maxPrefCost - costOfPrefsThatMustBeViolated;
            if (nameRowsAndColumns) {
                lp->setRowName(primaryObjectiveRow, "primaryobjective");
            }
        }
        
        primaryObjectiveRowReset.first = primaryObjectiveRow;
        
        lp->setMaximiseObjective(false);
        lp->setObjective(wVec);
        
        lp->hush();
        
        if (!lp->solve(false || neverUsePresolving)) {
            presolved = true;
            if (debug) cout << "failed\n";
            delete [] wVec;
            return false;
        }
        
        presolved = true;
        
        const double objVal = lp->getObjValue();
        
        
        if (PreferenceHandler::preferenceDebug) {
            cout << "Preferences broken:";
        }
                        
        
        const double *  soln = 0;
        for (int sv = 0; sv < tsvCount; ++sv) {
            const int svp = switchVarForPreference[sv].useInObjective;
            if (svp == -1) continue;
            if (!soln) {
                soln = lp->getSolution();
            }
            if (debug) cout << "Fetching result for task switch var " << svp << " (for preference " << sv << ")\n";

            if (soln[svp] > 0.95) {
                prefCost += RPGBuilder::getPreferences()[sv].cost;
                prefsUnsatisfied.insert(sv);
                if (PreferenceHandler::preferenceDebug) {
                    cout << " " << sv;
                }
            }
            
        }
        
        for (int sv = tsvCount; sv < asvCount; ++sv) {
            const int svp = switchVarForPreference[sv].useInObjective;
            if (svp == -1) continue;
            if (!soln) {
                soln = lp->getSolution();
            }
            if (debug) cout << "Fetching result for precondition switch var " << svp << " (for preference " << sv << ")\n";
                                    
            if (soln[svp] > 0.001) {
                prefCost += RPGBuilder::getPreferences()[sv].cost * soln[svp];   
                if (PreferenceHandler::preferenceDebug) {
                    cout << " pre" << sv << "x" << soln[svp];
                }
            }
                                                            
        }
                        
        
        if (PreferenceHandler::preferenceDebug) {
            cout << endl;
        }
    
        const double newRHS = objVal + objVal * 1e-6;
        
        if (newRHS > 0.0) {
            lp->setRowUpper(primaryObjectiveRow, newRHS);
        } else {
            lp->setRowUpper(primaryObjectiveRow, 0.0);
        }
                                        
    }
    


//    const int origColCount = get_Norig_columns(lp);
    {
        int v = 0;
        
        for (; v < varCount + residualColumnCount; ++v) {
            wVec[v] = 0.0;
        }
        
        const double peMultiplier = (prefsChecked && (primaryObjectiveRow == -1) ? 0.0001 : 1.0);

        if (alternativeObjective) {
            int aID;
            for (; v < lpColCount; ++v) {
                aID = goesToRealAction[v];
                if (aID == -3) {
                    wVec[v] = 0.0;
                } else if (aID == -1 || preferEarlier[v] == -1.0) { // then we have a switch var
                    wVec[v] = 1000 * peMultiplier;
                } else {
                    wVec[v] = peMultiplier * preferEarlier[v];
                }
            }
        } else {
            for (; v < lpColCount; ++v) {
                if (goesToRealAction[v] == -3) {
                    wVec[v] = 0.0;
                } else {
                    wVec[v] = peMultiplier * preferEarlier[v];
                }
            }
        }



        for (int sv = 0; sv < asvCount; ++sv) {
            const SwitchVarInfo & svp = switchVarForPreference[sv];
            if (svp.useInObjective == -1) continue;
            if (primaryObjectiveRow == -1) {
                wVec[svp.useInObjective] = RPGBuilder::getPreferences()[sv].cost;
            } else {
                wVec[svp.useInObjective] = 0.0;
            }
        }
        
                
    }

    lp->setMaximiseObjective(false);
    lp->setObjective(wVec);

    delete [] wVec;


    if (debug) {
        ostringstream nm;
        nm << "goalmodel" << iteration << "-" << varCount << ".lp";
        lp->writeLp(nm.str());
    }

    if (debug) {
        cout << "Solving...";
        cout.flush();
    }

    lp->hush();

    if (!lp->solve(false || neverUsePresolving)) {
        presolved = true;
        if (debug) cout << "failed\n";
        return false;
    }

    presolved = true;

    if (debug) cout << "done\n";
    


    if (primaryObjectiveRow == -1) {
        
        if (PreferenceHandler::preferenceDebug) {
            cout << "Preferences broken:";
        }
        
        const double *  soln = 0;
        for (int sv = 0; sv < tsvCount; ++sv) {
            const int svp = switchVarForPreference[sv].useInObjective;
            if (svp == -1) continue;
            if (!soln) {
                soln = lp->getSolution();
            }
            if (debug) cout << "Fetching result for task switch var " << svp << " (for preference " << sv << ")\n";

            if (soln[svp] > 0.95) {
                prefCost += RPGBuilder::getPreferences()[sv].cost;
                prefsUnsatisfied.insert(sv);
                if (PreferenceHandler::preferenceDebug) {
                    cout << " " << sv;
                }
            }
        }
        
        for (int sv = tsvCount; sv < asvCount; ++sv) {
            const int svp = switchVarForPreference[sv].useInObjective;
            if (svp == -1) continue;
            if (!soln) {
                soln = lp->getSolution();
            }
            if (debug) cout << "Fetching result for precondition switch var " << svp << " (for preference " << sv << ")\n";

            if (soln[svp] > 0.001) {
                prefCost += RPGBuilder::getPreferences()[sv].cost * soln[svp];
                if (PreferenceHandler::preferenceDebug) {
                    cout << " pre" << sv << "x" << soln[svp];
                }
            }
        }
        
        if (PreferenceHandler::preferenceDebug) {
            cout << endl;
        }
    }
    
    /*{
        map<int,double>::iterator fItr = propsSatisfied.begin();
        const map<int,double>::iterator fEnd = propsSatisfied.end();
        
        for (; fItr != fEnd; ++fItr) {
            const list<pair<int, VAL::time_spec> > & eta = RPGBuilder::getEffectsToActions()[*fItr];

            list<pair<int, VAL::time_spec> >::const_iterator eItr = eta.begin();
            const list<pair<int, VAL::time_spec> >::const_iterator eEnd = eta.end();
            
            for (; eItr != eEnd; ++eItr) {
                if ( !(*propPrec)[eItr->first] && !(*numPrec)[eItr->first] && fluentAction[eItr->first]) {
                    const map<int, ActData>::const_iterator lavItr = layerActionVariables.find(eItr->first);
                    
                    if (   lavItr != layerActionVariables.end()
                        && lp->getSingleSolutionVariableValue(lavItr->second.Avar) > 0.05) {
                        if (actionAppearedInLayer[eItr->first] < fItr->second) {
                            fItr->second = actionAppearedInLayer[eItr->first];
                        }
                    }
                }
            }
        }
    }*/
    
    if (integerDisjunctiveLandmarksEncoding) {
        
        if (useLocalLandmarks) {
            
            list<list<set<int> > >::const_iterator dlItr = localDisjunctiveLandmarks.begin();
            const int dlCount = disjunctiveLandmarkGroupVar.size();
            
            for (int dl = 0; dl < dlCount; ++dl, ++dlItr) {
                
                const int mCount = disjunctiveLandmarkGroupVar[dl].size();
                
                int m;
                for (m = 0; m < mCount; ++m) {
                    if (disjunctiveLandmarkGroupVar[dl][m] < 0) break;
                }
                
                if (m != mCount) {
                    // didn't add actions to meet at least one option for this disjunctive landmark
                    continue;
                }
                
                list<set<int> >::const_iterator mItr = dlItr->begin();
                const list<set<int> >::const_iterator mEnd = dlItr->end();
                
                for (m = 0; mItr != mEnd, m < mCount; ++mItr, ++m) {
                    if (lp->getSingleSolutionVariableValue(disjunctiveLandmarkGroupVar[dl][m]) > 0.999) {
                        set<int>::const_iterator fItr = mItr->begin();
                        const set<int>::const_iterator fEnd = mItr->end();
                        
                        for (; fItr != fEnd; ++fItr) {
                            const map<int,int>::const_iterator fCol = lpVarToDenoteThatWeHaveAGivenFact.find(*fItr);
                            if (fCol != lpVarToDenoteThatWeHaveAGivenFact.end()) {
                                
                                //const list<pair<int, VAL::time_spec> > & eta = RPGBuilder::getEffectsToActions()[*fItr];
                                
                                double fachievedInLayer = DBL_MAX;
                                
                                /*list<pair<int, VAL::time_spec> >::const_iterator eItr = eta.begin();
                                const list<pair<int, VAL::time_spec> >::const_iterator eEnd = eta.end();
                                
                                for (; eItr != eEnd; ++eItr) {
                                    if ( !(*propPrec)[eItr->first] && !(*numPrec)[eItr->first] && fluentAction[eItr->first]) {
                                        const map<int, ActData>::const_iterator lavItr = layerActionVariables.find(eItr->first);
                                        
                                        if (   lavItr != layerActionVariables.end()
                                            && lp->getSingleSolutionVariableValue(lavItr->second.Avar) > 0.05) {
                                            
                                            if (actionAppearedInLayer[eItr->first] < fachievedInLayer) {
                                                fachievedInLayer = actionAppearedInLayer[eItr->first];
                                            }
                                        }
                                    }
                                }*/
                                                                
                                propsSatisfied.insert(make_pair(*fItr, fachievedInLayer));
                            }
                        }
                    }
                }
                
            }
            
        } else {
        
            const int dlCount = disjunctiveLandmarkGroupVar.size();
            
            for (int dl = 0; dl < dlCount; ++dl) {
                const int mCount = disjunctiveLandmarkGroupVar[dl].size();
                
                int m;
                for (m = 0; m < mCount; ++m) {
                    if (disjunctiveLandmarkGroupVar[dl][m] < 0) break;
                }
                
                if (m != mCount) {
                    // didn't add actions to meet at least one option for this disjunctive landmark
                    continue;
                }
                
                for (m = 0; m < mCount; ++m) {
                    if (lp->getSingleSolutionVariableValue(disjunctiveLandmarkGroupVar[dl][m]) > 0.999) {
                        // this option was satisfied
                        
                        const vector<int> & itsFacts = LandmarksAnalysis::getDisjunctiveLandmarks()[dl][m];
                        
                        const int fCount = itsFacts.size();
                        
                        for (int f = 0; f < fCount; ++f) {
                            const map<int,int>::const_iterator fCol = lpVarToDenoteThatWeHaveAGivenFact.find(itsFacts[f]);
                            if (fCol != lpVarToDenoteThatWeHaveAGivenFact.end()) {
                                
                                //const list<pair<int, VAL::time_spec> > & eta = RPGBuilder::getEffectsToActions()[itsFacts[f]];
                                
                                double fachievedInLayer = DBL_MAX;
                                
                                /*list<pair<int, VAL::time_spec> >::const_iterator eItr = eta.begin();
                                const list<pair<int, VAL::time_spec> >::const_iterator eEnd = eta.end();
                                
                                for (; eItr != eEnd; ++eItr) {
                                    if ( !(*propPrec)[eItr->first] && !(*numPrec)[eItr->first] && fluentAction[eItr->first]) {
                                        const map<int, ActData>::const_iterator lavItr = layerActionVariables.find(eItr->first);
                                        
                                        if (   lavItr != layerActionVariables.end()
                                            && lp->getSingleSolutionVariableValue(lavItr->second.Avar) > 0.05) {
                                            
                                            if (actionAppearedInLayer[eItr->first] < fachievedInLayer) {
                                                fachievedInLayer = actionAppearedInLayer[eItr->first];
                                            }
                                        }
                                    }
                                }*/
                                
                                propsSatisfied.insert(make_pair(itsFacts[f], fachievedInLayer));
                            }
                        }
                    }
                }
                
            }
        }
    }
    
    {
        map<int, pair<int,int> >::const_iterator bsItr = bootstrapPropColumnAndRow.begin();
        const map<int, pair<int,int> >::const_iterator bsEnd = bootstrapPropColumnAndRow.end();
        
        for (; bsItr != bsEnd; ++bsItr) {
            if (bsItr->first < 0) continue;
            if (lp->getSingleSolutionVariableValue(bsItr->second.first) > 0.99) {
                //const list<pair<int, VAL::time_spec> > & eta = RPGBuilder::getEffectsToActions()[bsItr->second.first];
                
                double fachievedInLayer = DBL_MAX;
                /*
                list<pair<int, VAL::time_spec> >::const_iterator eItr = eta.begin();
                const list<pair<int, VAL::time_spec> >::const_iterator eEnd = eta.end();
                
                for (; eItr != eEnd; ++eItr) {
                    if ( !(*propPrec)[eItr->first] && !(*numPrec)[eItr->first] && fluentAction[eItr->first]) {
                        const map<int, ActData>::const_iterator lavItr = layerActionVariables.find(eItr->first);
                        
                        if (   lavItr != layerActionVariables.end()
                            && lp->getSingleSolutionVariableValue(lavItr->second.Avar) > 0.05) {
                            
                            if (actionAppearedInLayer[eItr->first] < fachievedInLayer) {
                                fachievedInLayer = actionAppearedInLayer[eItr->first];
                            }
                        }
                    }
                }*/
                
                propsSatisfied.insert(make_pair(bsItr->first, fachievedInLayer));
            }
        }
    }
    
    if (debug) cout << "Returning that yes, goals are satisfied, and preference cost is " << prefCost << endl;
    return true;

}

void MILPRPG::fillMinMaxFluentTable(vector<double> & toFill)
{

    if (!varCount) return;

    static const bool debug = false;

    if (debug) {
        MILPSolver::debug = true;
    }
    
    MILPSolver* lp = allLp.back().first->lp;

    
    //lp->writeLp("fillmodel.lp");

    bool & presolved = allLp.back().first->presolved;
    const bool intVariables = allLp.back().first->intVariables;

    if (lp->copyBeforePresolving() || lp->warmStartingIsUnreliable()) {
        if (debug) cout << "Cloning LP before presolve\n";                
        allLp.back().second = new LPAndBounds(lp->clone(), presolved, intVariables, allLp.back().first->residualSwitch, varCount);
    }
    

    bool usedYet = false;

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
// static char buf[256];
// map<double, vector<VarData>, EpsilonComp>::iterator fItr = minMaxVars.find(ts);
// assert(fItr != minMaxVars.end());

    // bear in mind caching results to maximise/minimise each var, and use those to seed past layers


    assert(colCount == lp->getNumCols());

// print_lp(lp);


    for (int i = 0; i < varCount; ++i) {
        if (RPGBuilder::getDominanceConstraints()[i] != E_METRIC && RPGBuilder::getDominanceConstraints()[i] != E_IRRELEVANT) {

            if (toFill[i] < maxNeeded[i] && anyIncreasers[i]) {

                if (usedYet && lp->warmStartingIsUnreliable()) {
                    delete lp;
                    lp = allLp.back().first->lp = allLp.back().second->lp->clone();      
                    presolved = false;
                }
                
                usedYet = true;
                
                const int RI = minMaxVars[i].Vvar;

                lp->setMaximiseObjective(true);
                lp->setObjCoeff(RI, 1.0);

                if (debug) {
                    cout << "Maximising col " << RI << " (" << *(RPGBuilder::getPNE(i)) << ")";
                    cout.flush();
                }

                lp->hush();

                if (debug && false) {
                    ostringstream n;
                    n << "cplexmodel" << i << ".lp";
                    lp->writeLp(n.str());
                }

                if (!lp->solve(presolved || neverUsePresolving)) {
                    lp->writeLp("cplexmodel.lp");
                    allLp.back().second->lp->setMaximiseObjective(true);
                    allLp.back().second->lp->setObjCoeff(RI, 1.0);
                    cerr << "Internal error: model should always be solvable here.\n";
                    if (allLp.back().second->lp->solve(neverUsePresolving)) {
                        cerr << "A clone of an earlier model was solvable, pointing to errors in warm-starting.\n";
                    } else {
                        cerr << "A clone of an earlier model could not be solved either.\n";
                    }
                    assert(false);
                }
                presolved = true;

                const double mv = lp->getObjValue();

                toFill[i] = mv;

                if (debug) {
                    cout << " = " << mv << endl;
                }
                maxRealVec[i] = true;

                lp->setObjCoeff(RI, 0.0);
                //lp->clearObjective();

            } else {
                if (debug) {
                    cout << "Not maximising " << i << " (" << *(RPGBuilder::getPNE(i)) << ": ";
                    if (toFill[i] >= maxNeeded[i]) {
                        cout << toFill[i] << " is good enough (" << maxNeeded[i] << ")\n";
                    } else {
                        cout << "no increasers\n";
                    }
                }
            }

            maxVec[i] = toFill[i];

            if (toFill[i+varCount] < (-minNeeded[i]) && anyDecreasers[i]) {

                if (usedYet && lp->warmStartingIsUnreliable()) {
                    delete lp;
                    lp = allLp.back().first->lp = allLp.back().second->lp->clone();
                    presolved = false;
                }
                                
                usedYet = true;
                                                
                const int RI = minMaxVars[i].Vvar;

                lp->setMaximiseObjective(false);
                lp->setObjCoeff(RI, 1.0);


                if (debug) {
                    cout << "Minimising col " << RI << " (" << *(RPGBuilder::getPNE(i)) << ")";
                    cout.flush();
                }

                const bool result = lp->solve(presolved || neverUsePresolving);
                assert(result);

                presolved = true;
                const double mvTwo = lp->getObjValue();
                toFill[i + varCount] = -mvTwo;

                if (debug) {
                    cout << " = " << mvTwo << endl;
                }

                minRealVec[i] = true;

                lp->setObjCoeff(RI, 0.0);
               // lp->clearObjective();

            }
            minVec[i] = -toFill[i + varCount];

        } else {
            toFill[i] = 0.0;
            toFill[i + varCount] = 0.0;
        }
    }

    if (!lp->copyBeforePresolving() && !lp->warmStartingIsUnreliable()) {
        if (debug) cout << "Cloning LP\n";

        allLp.back().second = new LPAndBounds(lp->clone(), presolved, intVariables, allLp.back().first->residualSwitch, varCount);
    }

    if (debug) {
        cout << "Fluent table filled, returning\n";
        for (int i = 0; i < varCount; ++i) {
            cout << "\t" << toFill[i];
        }
        cout << "\n";

    }

    if (debug) {
        MILPSolver::debug = false;
    }
    

}

void MILPRPG::getUnreachablePrefs(list<int> & retList, const vector<AutomatonPosition> & preferenceStatusesInRPG)
{
    static const int tsvCount = RPGBuilder::getTaskPrefCount();
    if (!tsvCount) return;
    
    MILPSolver* const lp = goalLP->lp;
    
//    const double oldObj = lp->getObjValue();
    const double * soln = lp->getSolution();
    
    
    if (primaryObjectiveRowReset.first != -1) {
        lp->setRowUpper(primaryObjectiveRowReset.first, primaryObjectiveRowReset.second);
        
    }
    
    for (int sv = 0; sv < tsvCount; ++sv) {
        const int svp = switchVarForPreference[sv].useInObjective;
        if (svp == -1) continue;
        
        if (!isSatisfied(preferenceStatusesInRPG[sv])) {
            // preferences that the RPG sees as being unsatisfied are costed back in getRelaxedPlan()
            // - this method is only to spot extras
            continue;
        }
        
        if (soln[svp] > 0.95) {
            lp->setColUpper(svp, 0);
            if (!lp->solve(false || neverUsePresolving)) {
                retList.push_back(sv);
                //cout << "Shown " << RPGBuilder::getPreferences()[sv].name << " to be unreachable\n";
                lp->setColUpper(svp, 1);
                lp->setColLower(svp, 1);
            } else {
                //cout << "Pref " << RPGBuilder::getPreferences()[sv].name << " is reachable, but changes objective from " << oldObj << " to " << lp->getObjValue() << endl;
                lp->setColUpper(svp, 1);
            }
        }
        
    }
    
}


void MILPRPG::getActionsForGoal(list<pair<int, double> > & toReturn)
{
    if (!addProps) return;

    assert(goalLP);
    assert(goalLP->presolved);
    MILPSolver * const lp = goalLP->lp;

    const double * results = lp->getSolution();

    const int lpColCount = lp->getNumCols();

    int ra;

    //cout << "For goal - use:\n";

    for (int v = varCount + switchVarCount + residualColumnCount; v < lpColCount; ++v) {
        ra = goesToRealAction[v];
        if (ra <= -1) continue;
        double var = results[v];
        if (var > 0.001) {
            //cout << "\t" << var << " x " << *(RPGBuilder::getInstantiatedOp(ra)) << "\n";
            toReturn.push_back(pair<int, double>(ra, var));
        }
    }

}

void MILPRPG::getActionsFor(const double & TS, const int & noPPi, const bool & min, const double & RHS, list<pair<int, double> > & toReturn)
{

    // if min = false, then we want noPPI to be at least >= RHS
    // if min = true, then we want noPPI to be at least <= RHS

    static const bool debug = false;

    assert(varCount);

    LPAndBounds* const toUse = allLp[timestampToIndex[TS]].first;
    MILPSolver* const lp = toUse->lp;
    bool & presolved = toUse->presolved;

    const int PPi = noPPi;
    const int i = PPi;

    double oldlowbo = lp->getColLower(PPi);
    double oldupbo = lp->getColUpper(PPi);

    const int lpColCount = lp->getNumCols();

    if (!toUse->nowExtracting) {

        const int asvCount = switchVarForPreference.size();
        
        double* row = new double[lpColCount];
        int v = 0;
        for (; v < varCount + switchVarCount + residualColumnCount; ++v) {
            row[v] = 0.0;
        }

        bool havePrefs = false;
        
        for (int sv = 0; sv < asvCount; ++sv) {
            const SwitchVarInfo & svp = switchVarForPreference[sv];
            if (svp.useInObjective != -1) {
                havePrefs = true;
                break;
            }
        }

        int aID;
        for (int v = varCount + switchVarCount + residualColumnCount; v < lpColCount; ++v) {
            aID = goesToRealAction[v];
            if (aID == -3) {
                row[v] = 0.0;
            } else {
                row[v] = preferEarlier[v];
            }
        }
        
        for (int sv = 0; sv < asvCount; ++sv) {
            const SwitchVarInfo & svp = switchVarForPreference[sv];
            if (svp.useInObjective != -1) {
                row[svp.useInObjective] = 1000 * RPGBuilder::getPreferences()[sv].cost;
            }
        }
        

        lp->setMaximiseObjective(false);
        lp->setObjective(row);
        delete [] row;

        for (int v = 0; v < varCount; ++v) {
            if (toUse->minReal[v]) {
                lp->setColLower(v, toUse->min[v]);
            }
            if (toUse->maxReal[v]) {
                lp->setColUpper(v, toUse->max[v]);
            }
        }

        toUse->nowExtracting = true;
        if (debug) cout << "First time extracting at " << TS << ", set objective\n";
    }

    //double boundVal = (RHS == -1.0 ? (min ? toUse->min[noPPi] : toUse->max[noPPi]) : RHS);


    if (RHS != -1.0) {
        if (min) {
            if (debug) cout << "Asking to find " << *(RPGBuilder::getPNE(i)) << " <= " << RHS << " at " << TS << endl;
            lp->setColUpper(i, RHS);
        } else {
            if (debug) cout << "Asking to find " << *(RPGBuilder::getPNE(i)) << " >= " << RHS << " at " << TS << endl;
            lp->setColLower(i, RHS);
        }
    } else {
        if (min) {
            lp->setColUpper(i, toUse->min[noPPi]);
        } else {
            lp->setColLower(i, toUse->max[noPPi]);
        }
    }

    const bool result = lp->solve(presolved || neverUsePresolving);
    assert(result);

    presolved = true;

    const double* results = lp->getSolution();


    if (debug) cout << "\tObjective: " << lp->getObjValue() << "\n";

    int ra;
    double var;

    bool usingAny = false;
    
    for (int v = varCount + switchVarCount + residualColumnCount; v < lpColCount; ++v) {
        ra = goesToRealAction[v];
        if (ra <= -1) continue;

        var = results[v];
        if (debug) {
            if (fabs(var) > 0.00000000000001) {
                cout << "\tUsing " << var << " x " << *(RPGBuilder::getInstantiatedOp(goesToRealAction[v])) << "\n";                
            }
        }
        if (var > 0.001) {
            toReturn.push_back(pair<int, double>(ra, var));
            usingAny = true;
        }
    }

    if (!usingAny) {
        cerr << "Internal error: should use at least one action to achieve something not true in the state being evaluated\n";
        lp->writeLp("usingany.lp");
        exit(1);
    }
    
    lp->setColBounds(i, oldlowbo, oldupbo);
};

int MILPRPG::giveMeAColumn(const string & n)
{
    static const vector<pair<int, double> > emptyVector;

    MILPSolver * const lp = allLp.back().first->lp;
    lp->addCol(emptyVector, 0, 1, 0, MILPSolver::C_BOOL);

    const int toReturn = lp->getNumCols() - 1;

    goesToRealAction[toReturn] = -3;

    assert(colCount == toReturn);
    ++colCount;

    allLp.back().first->intVariables = true;

    if (nameRowsAndColumns) lp->setColName(toReturn, n);

    return toReturn;
}

int MILPRPG::giveMeAnIntermediateColumn(const string & n, const int & id)
{
    static const vector<pair<int, double> > emptyVector;
    
    MILPSolver * const lp = allLp.back().first->lp;
    lp->addCol(emptyVector, 0, 1, 0, MILPSolver::C_BOOL);
    
    const int toReturn = lp->getNumCols() - 1;
    
    goesToRealAction[toReturn] = -3;
    
    assert(colCount == toReturn);
    ++colCount;
    
    allLp.back().first->intVariables = true;
    
    if (nameRowsAndColumns) {
        ostringstream nid;
        nid << n << "c" << id;
        lp->setColName(toReturn, nid.str());
    }
    
    return toReturn;
}


/*pair<int, int> MILPRPG::giveMeTriggerColumns(const int& prefIdx, const int& n, const bool & secTrigger)
{
    static const vector<pair<int, double> > emptyVector;

    static const bool dumpRawRow = false;
    
    if (!n) return make_pair(-1, -1);

    MILPSolver * const lp = allLp.back().first->lp;

    const int toReturn = lp->getNumCols();

    for (int i = 0; i < n; ++i) {
        lp->addCol(emptyVector, 0.0, 1.0, 3);
        goesToRealAction[colCount] = -3;
        
        if (nameRowsAndColumns) {
            ostringstream wt;
            wt << "itc" << i << "for" << RPGBuilder::getPreferences()[prefIdx].name;
            lp->setColName(colCount, wt.str());
        }
        
        ++colCount;
    }

    vector<pair<int, double> > newRow(n + 1);

    for (int i = 0; i < n; ++i) {
        newRow[i] = make_pair(i + toReturn, -1.0);
        if (dumpRawRow) {
            if (i) cout << " +";
            cout << " -1*" << lp->getColName(i + toReturn);
        }
        //cout << "Trigger option " << i << " is column " << i + toReturn << endl;
    }

    int tCol = -1;


    if (secTrigger) {
        lp->addCol(emptyVector, 0.0, 1.0, 3);
        goesToRealAction[colCount] = -3;
        if (nameRowsAndColumns) {
            ostringstream wt;
            wt << "sectrigger" << RPGBuilder::getPreferences()[prefIdx].name;
            lp->setColName(colCount, wt.str());
        }
        tCol = colCount++;

    } else {
        map<int, pair<int, int> >::iterator tColItr = bootstrapPropColumnAndRow.find(-1 - prefIdx);
        if (tColItr == bootstrapPropColumnAndRow.end()) {

            assert(!switchVarForPreference[prefIdx].second.empty());

            vector<pair<int, double> > tmpColVec(switchVarForPreference[prefIdx].second.size());
            
            list<int>::const_iterator svItr = switchVarForPreference[prefIdx].second.begin();
            const list<int>::const_iterator svEnd = switchVarForPreference[prefIdx].second.end();
            
            for (; svItr != svEnd; ++svItr) {
                tmpColVec[0].first = *svItr;
                tmpColVec[0].second = -BIG;
                #ifndef NDEBUG
                assert(canPutHelperInRow.find(*svItr) != canPutHelperInRow.end());
                #endif
                            
            }
            lp->addCol(tmpColVec, 0, 1, 3);
            
            
            if (nameRowsAndColumns) {
                ostringstream wt;
                wt << "trigvar" << RPGBuilder::getPreferences()[prefIdx].name;
                lp->setColName(colCount, wt.str());
            }
            goesToRealAction[colCount] = -3;

            const int bsCol = colCount++;

            vector<pair<int, double> > newRowVec(1);
            newRowVec[0] = make_pair(bsCol, BIG);
            lp->addRow(newRowVec, 0, lp->getInfinity());
            const int bsRow = lp->getNumRows() - 1;
            
            if (nameRowsAndColumns) {
                ostringstream wt;
                wt << "controltrig" << RPGBuilder::getPreferences()[prefIdx].name;
                lp->setRowName(bsRow, wt.str());
            }
            
            
            tColItr = bootstrapPropColumnAndRow.insert(make_pair((-1 - prefIdx), make_pair(bsCol, bsRow))).first;

        }

        tCol = tColItr->second.first;
    }

    assert(tCol != -1);

    newRow[n] = make_pair(tCol, 1.0);

    if (dumpRawRow) {
        cout << " + " << lp->getColName(tCol);
        cout << " in [" << -(n - 1) << "," << 1 << "]\n";
    }
    //cout << "Entry " << n << " is column " << tCol << endl;

    lp->addRow(newRow, -(n - 1), 1);
    
    if (nameRowsAndColumns) {
        ostringstream wt;
        wt << "all" << n << "truefor" << RPGBuilder::getPreferences()[prefIdx].name;
        lp->setRowName(lp->getNumRows() - 1, wt.str());
    }
                

    allLp.back().first->intVariables = true;

    return make_pair(toReturn, tCol);
}*/


pair<int,int> MILPRPG::giveMeATriggerColumn(const int& prefIdx, const bool & secTrigger)
{
    static const vector<pair<int, double> > emptyVector;
    
    static const bool dumpRawRow = false;
    
    MILPSolver * const lp = allLp.back().first->lp;
    
    const int toReturn = lp->getNumCols();
    
    lp->addCol(emptyVector, 0.0, 1.0, 0.0, MILPSolver::C_BOOL);
    goesToRealAction[colCount] = -3;
        
    if (nameRowsAndColumns) {
        ostringstream wt;
        wt << "itcfor" << RPGBuilder::getPreferences()[prefIdx].name;
        lp->setColName(colCount, wt.str());
    }
                
    ++colCount;
    
    
    vector<pair<int, double> > newRow(2);
    
    newRow[0] = make_pair(toReturn, 1.0);
    if (dumpRawRow) {
        cout << " 1*" << lp->getColName(toReturn);
    }
        
    int tCol = -1;
            
            
    if (secTrigger) {
        lp->addCol(emptyVector, 0, 1, 0, MILPSolver::C_BOOL);
        goesToRealAction[colCount] = -3;
        if (nameRowsAndColumns) {
            ostringstream wt;
            wt << "sectrigger" << RPGBuilder::getPreferences()[prefIdx].name;
            lp->setColName(colCount, wt.str());
        }
        tCol = colCount++;
                        
    } else {
        map<int, pair<int, int> >::iterator tColItr = bootstrapPropColumnAndRow.find(-1 - prefIdx);
        if (tColItr == bootstrapPropColumnAndRow.end()) {
            
            assert(!switchVarForPreference[prefIdx].second.empty());
            
            vector<pair<int, double> > tmpColVec(switchVarForPreference[prefIdx].second.size());
            
            list<int>::const_iterator svItr = switchVarForPreference[prefIdx].second.begin();
            const list<int>::const_iterator svEnd = switchVarForPreference[prefIdx].second.end();
            
            for (; svItr != svEnd; ++svItr) {
                tmpColVec[0].first = *svItr;
                tmpColVec[0].second = -BIG;
                #ifndef NDEBUG
                assert(canPutHelperInRow.find(*svItr) != canPutHelperInRow.end());
                #endif
                                            
            }
            lp->addCol(tmpColVec, 0, 1, 0, MILPSolver::C_BOOL);
            
            
            if (nameRowsAndColumns) {
                ostringstream wt;
                wt << "trigvar" << RPGBuilder::getPreferences()[prefIdx].name;
                lp->setColName(colCount, wt.str());
            }
            goesToRealAction[colCount] = -3;
            
            const int bsCol = colCount++;
            
            vector<pair<int, double> > newRowVec(1);
            newRowVec[0] = make_pair(bsCol, BIG);
            lp->addRow(newRowVec, 0, lp->getInfinity());
            const int bsRow = lp->getNumRows() - 1;
            
            if (nameRowsAndColumns) {
                ostringstream wt;
                wt << "controltrig" << RPGBuilder::getPreferences()[prefIdx].name;
                lp->setRowName(bsRow, wt.str());
            }
                                                        
                                                                    
            tColItr = bootstrapPropColumnAndRow.insert(make_pair((-1 - prefIdx), make_pair(bsCol, bsRow))).first;
                                                                                
        }
                
        tCol = tColItr->second.first;
    }
            
    assert(tCol != -1);
                
    newRow[1] = make_pair(tCol, 1.0);
                
    if (dumpRawRow) {
        cout << " + " << lp->getColName(tCol);
        cout << " in [1,2]\n";
    }
    //cout << "Entry " << n << " is column " << tCol << endl;
                    
    lp->addRow(newRow, -1, 1);
                    
    if (nameRowsAndColumns) {
        ostringstream wt;
        wt << "alltruefor" << RPGBuilder::getPreferences()[prefIdx].name;
        lp->setRowName(lp->getNumRows() - 1, wt.str());
    }
                                    
                                    
    allLp.back().first->intVariables = true;
    
    return make_pair(toReturn,tCol);
}


int MILPRPG::getColumnForVar(const int & v)
{
    //lprec * const lp = allLp.back().first->lp;
    //const int Nrows = get_Norig_rows(lp);
    //return get_lp_index(lp, minMaxVars[v].Vvar + Nrows);

    return minMaxVars[v].Vvar;
}

int MILPRPG::getUBColumnForVar(const int & v)
{
    //lprec * const lp = allLp.back().first->lp;
    //const int Nrows = get_Norig_rows(lp);
    //return get_lp_index(lp, minMaxVars[v].Vvar + Nrows);

    return minMaxVars[v].residualUB;
}

int MILPRPG::getLBColumnForVar(const int & v)
{
    //lprec * const lp = allLp.back().first->lp;
    //const int Nrows = get_Norig_rows(lp);
    //return get_lp_index(lp, minMaxVars[v].Vvar + Nrows);

    return minMaxVars[v].residualLB;
}


void MILPRPG::addTrigger(const string & ID, const vector<pair<int, double> > & entries, const double & lb, const double & ub)
{
    MILPSolver * const lp = allLp.back().first->lp;

    static const bool dumpRawRow = (false || PreferenceHandler::preferenceDebug);
    
    if (nameRowsAndColumns) {
        assert(!ID.empty());
    }
    
    if (dumpRawRow) {
        cout << "TR" << lp->getNumRows() << ": ";

        for (uint vi = 0; vi < entries.size(); ++vi) {
            if (vi) cout << " + ";
            const int RI = entries[vi].first;
            cout << entries[vi].second << "*" << lp->getColName(RI);
        }
    }

    if (ub == DBL_MAX) {
        if (dumpRawRow) cout << " >= " << lb << " : " << ID << endl;
        lp->addRow(entries, lb, lp->getInfinity());
    } else if (lb == -DBL_MAX) {
        if (dumpRawRow) cout << " <= " << ub << " : " << ID << endl;
        lp->addRow(entries, -lp->getInfinity(), ub);
    } else {
        if (dumpRawRow) cout << " in [" << lb << "," << ub << "] : " << ID << endl;
        lp->addRow(entries, lb, ub);
    }
    
    lp->setRowName(lp->getNumRows() - 1, ID);
}

/*
int MILPRPG::addSwitch(const string & ID, const vector<pair<int,double> > & entries, const double & rhs)
{
    MILPSolver * const lp = allLp.back().first->lp;

    for (int vi = 0; vi < entries.size(); ++vi) {
        if (vi) cout << " + ";
        const int RI = entries[vi].first;
        cout << entries[vi].second << "*" << lp->getColName(RI);
    }

    cout << " <= " << rhs << endl;

    lp->addRow(entries, -lp->getInfinity(), rhs);

    const int toReturn = lp->getNumRows() - 1;

    if (nameRowsAndColumns) lp->setRowName(toReturn, ID);

    return toReturn;

}
*/

int MILPRPG::addSwitchB(const string & ID, const vector<pair<int, double> > & entries, const bool & positive, const double & rhs,
                        const bool &
#ifndef NDEBUG
                                     spaceForHelper
#endif
)
{
    MILPSolver * const lp = allLp.back().first->lp;

    static const bool dumpRawRow = PreferenceHandler::preferenceDebug;
    
    if (nameRowsAndColumns) {
        assert(!ID.empty());
    }
    
    #ifndef NDEBUG
    if (spaceForHelper) {
        canPutHelperInRow.insert(lp->getNumRows());
    }
    #endif
    
    if (dumpRawRow)  {
        cout << "R" << lp->getNumRows() << ": ";
        if (!positive) cout << "¬ ";

        for (uint vi = 0; vi < entries.size(); ++vi) {
            if (vi) cout << " + ";
            const int RI = entries[vi].first;
            cout << entries[vi].second << "*" << lp->getColName(RI);
        }

        cout << " >= " << rhs << " : " << ID << endl;
    }
    if (positive) {
        lp->addRow(entries, rhs, lp->getInfinity());
    } else {
        lp->addRow(entries, -lp->getInfinity(), rhs);
    }

    const int toReturn = lp->getNumRows() - 1;

    if (nameRowsAndColumns) lp->setRowName(toReturn, ID);

    return toReturn;

}


void MILPRPG::preferenceTrippedBy(const int & prefID, const list<pair<int,int> > & prefTrips)
{
    list<pair<int,int> > & addTo = preferenceTrips[prefID];
    
    addTo.insert(addTo.end(), prefTrips.begin(), prefTrips.end());    
}

void MILPRPG::negativeLiteralAffectsRow(const int & i, const int & r, const int & e)
{
    negativeLiteralToEffectOnRow[i].push_back(make_pair(r,e));
}

void MILPRPG::literalAffectsRow(const int & i, const int & r, const int & e)
{
    literalToEffectOnRow[i].push_back(make_pair(r,e));
}
    

};
