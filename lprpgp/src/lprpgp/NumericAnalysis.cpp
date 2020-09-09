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

#include "NumericAnalysis.h"

#include <FuncAnalysis.h>

using namespace VAL;

using std::endl;

namespace Planner {

vector<bool> NumericAnalysis::integral;
vector<pair<double,double> > NumericAnalysis::pneBounds;
bool NumericAnalysis::doneIntegralAnalysis = false;

void NumericAnalysis::identifyIntegralPNEs() {
	if (doneIntegralAnalysis) return;

	doneIntegralAnalysis = true;
	const int pneCount = RPGBuilder::getPNECount();
	integral = vector<bool>(pneCount,false);

	vector<double> initialFluents;

	{
		LiteralSet tinitialState;
		RPGBuilder::getInitialState(tinitialState, initialFluents);
	}
	set<int> revisit;
	for (int pne = 0; pne < pneCount; ++pne) revisit.insert(pne);

	map<int,set<int> > waitingForIntegral;

	set<int>::iterator rvItr = revisit.begin();
	const set<int>::iterator rvEnd = revisit.end();


	while (rvItr != rvEnd) {
		const int pne = *rvItr;
		set<int>::iterator rvPrev = rvItr;
		++rvItr;
		revisit.erase(rvPrev);
		
		if (initialFluents[pne] == (int) (initialFluents[pne])) {
			integral[pne] = true;
			static vector<RPGBuilder::RPGNumericEffect> & numEffs = RPGBuilder::getNumericEff();
			static const int loopLim = numEffs.size();
			for (int i = 0; i < loopLim; ++i) {
				if (numEffs[i].fluentIndex == pne) {
					if (numEffs[i].constant != (int) (numEffs[i].constant)) {
						integral[pne] = false;
					} else {
						const int vSize = numEffs[i].size;
						for (int v = 0; v < vSize; ++v) {
							if (numEffs[i].weights[v] != (int) (numEffs[i].weights[v])) {
								integral[pne] = false;
								break;
							}
							int localVar = numEffs[i].variables[v];
							if (localVar < 0) {
								integral[pne] = false;
								break;
							} else {
								if (localVar >= pneCount) localVar -= pneCount;
								if (!integral[localVar]) {
									waitingForIntegral[localVar].insert(pne);
									integral[pne] = false;
									break;
								}
							}
						}
					}
					if (!integral[pne]) break;
				}
			}
			if (integral[pne]) {
				//cout << *(RPGBuilder::getPNE(pne)) << " is integral\n";
				map<int, set<int> >::iterator nowRV = waitingForIntegral.find(pne);
				if (nowRV != waitingForIntegral.end()) {
					revisit.insert(nowRV->second.begin(), nowRV->second.end());
					waitingForIntegral.erase(nowRV);
					rvItr = revisit.begin();
				}
			}
		}
	}

	{

		vector<RPGBuilder::RPGNumericPrecondition> & preTable = RPGBuilder::getModifiableNumericPreTable();

		const int loopLim = preTable.size();

		for (int p = 0; p < loopLim; ++p) {
			if (preTable[p].op == VAL::E_GREATER) {
				if (preTable[p].LHSVariable < pneCount) {
					if (integral[preTable[p].LHSVariable]) {
						preTable[p].RHSConstant += 1;
						preTable[p].op = VAL::E_GREATEQ;
					}
				} else if (preTable[p].LHSVariable < 2 * pneCount) {
					if (integral[preTable[p].LHSVariable - pneCount]) {
						preTable[p].RHSConstant += 1;
						preTable[p].op = VAL::E_GREATEQ;
					}
				} else {
					RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(preTable[p].LHSVariable);

					bool allInt = true;
					for (int i = 0; i < currAV.size; ++i) {
						if (currAV.fluents[i] < pneCount) {
							if (!integral[currAV.fluents[i]]) {
								allInt = false; break;
							}
						} else {
							if (!integral[currAV.fluents[i] - pneCount]) {
								allInt = false; break;
							}
						}
					}
					if (allInt) {
						preTable[p].RHSConstant += 1;
						preTable[p].op = VAL::E_GREATEQ;
					}
				}
			}
		}

	}
}


void NumericAnalysis::performBoundsAnalysis()
{
    static bool doneBoundsAnalysis = false;    
    if (doneBoundsAnalysis) return;
    
    static const bool debug = false;
    
    doneBoundsAnalysis = true;
    
    identifyIntegralPNEs();
    
    
    const int pneCount = RPGBuilder::getPNECount();
    pneBounds = vector<pair<double,double> >(pneCount, make_pair(-DBL_MAX, DBL_MAX));
    
    const vector<list<int> > & actionEffs = RPGBuilder::getActionsToRPGNumericStartEffects();
    
    vector<double> initialFluents;
    
    {
        LiteralSet tinitialState;
        RPGBuilder::getInitialState(tinitialState, initialFluents);
    }
    set<int> revisit;
    map<int,set<int> > waitingForBounded;
    
    for (int pne = 0; pne < pneCount; ++pne) {
        PNE * realPNE = RPGBuilder::getPNE(pne);
        if (EFT(realPNE->getHead())->isStatic()) {
            const double v = EFT(realPNE->getHead())->getInitial(realPNE->begin(), realPNE->end()).second;   
            pneBounds[pne].first = v;
            pneBounds[pne].second = v;
            if (v != initialFluents[pne]) {
                std::cerr << "Internal error: initial state fluent value of " << *realPNE << " is " << initialFluents[pne] << ", but TIM reports " << v << endl;
                exit(1);
            }
        } else {                
            revisit.insert(pne);
        }
    }

    set<int>::iterator rvItr = revisit.begin();
    const set<int>::iterator rvEnd = revisit.end();

    while (rvItr != rvEnd) {
        const int pne = *rvItr;
        set<int>::iterator rvPrev = rvItr;
        ++rvItr;
        revisit.erase(rvPrev);
        
        const vector<RPGBuilder::RPGNumericEffect> & numEffs = RPGBuilder::getNumericEff();
        const vector<RPGBuilder::RPGNumericPrecondition> & numPres = RPGBuilder::getNumericPrecs();
        
        const vector<list<int> > & actNumPres = RPGBuilder::getActionsToRPGNumericStartPreconditions();
        const vector<list<pair<int, VAL::time_spec> > > & effsToActions = RPGBuilder::getRpgNumericEffectsToActions();
        
        const int loopLim = numEffs.size();
        
        pair<double,double> newBounds(initialFluents[pne], initialFluents[pne]);

        for (int i = 0; (newBounds.first != -DBL_MAX || newBounds.second != DBL_MAX) && i < loopLim; ++i) {
            if (numEffs[i].fluentIndex == pne) {
                if (numEffs[i].size) {
                    // Not suitable for LPRPG
                    pneBounds[pne] = make_pair(-DBL_MAX, DBL_MAX);
                    break;
                }
            }
        }
                
        
        for (int i = 0; (newBounds.first != -DBL_MAX || newBounds.second != DBL_MAX) && i < loopLim; ++i) {
            if (numEffs[i].fluentIndex == pne) {
                assert(!numEffs[i].size);
                
                if (debug) {
                    if (numEffs[i].isAssignment) {
                        cout << "Effect id " << i << " assigns " << *(RPGBuilder::getPNE(pne)) << " the value " << numEffs[i].constant << endl;
                    } else {
                        cout << "Effect id " << i << " increases " << *(RPGBuilder::getPNE(pne)) << " by " << numEffs[i].constant << endl;
                    }
                }
                
                list<pair<int, VAL::time_spec> >::const_iterator actItr = effsToActions[i].begin();
                const list<pair<int, VAL::time_spec> >::const_iterator actEnd = effsToActions[i].end();

                if (debug) {
                    cout << "- Applies to " << effsToActions[i].size() << " actions\n";
                }
                
                for (int pi = 1; actItr != actEnd; ++actItr, ++pi) {
                    if (RPGBuilder::rogueActions[actItr->first]) continue;
                    
                    assert(actItr->second == VAL::E_AT_START);
                    
                    pair<double,double> preBounds(-DBL_MAX, DBL_MAX);
                    

                    if (!numEffs[i].isAssignment) {
                        if (debug) {
                            cout << pi << ")  has " << actNumPres[actItr->first].size() << " numeric preconditions\n";
                        }
                        list<int>::const_iterator apItr = actNumPres[actItr->first].begin();
                        const list<int>::const_iterator apEnd = actNumPres[actItr->first].end();
                        for (int api = 1; apItr != apEnd; ++apItr, ++api) {
                            const RPGBuilder::RPGNumericPrecondition & currPre = numPres[*apItr];
                            if (currPre.LHSVariable < pneCount) {
                                if (currPre.LHSVariable == pne && preBounds.first < currPre.RHSConstant) {
                                    preBounds.first = currPre.RHSConstant;
                                    if (debug) {
                                        cout << "Pre " << api << ") has a precondition >= " << preBounds.first << endl;
                                    }
                                } else {
                                    if (debug) {
                                        cout << "Pre " << api << ") has a precondition on " << *(RPGBuilder::getPNE(currPre.LHSVariable)) << ", ignoring\n";
                                    }
                                }
                            } else if (currPre.LHSVariable < 2 * pneCount) {
                                if (currPre.LHSVariable == pneCount + pne) {
                                    const double flipped = (currPre.RHSConstant != 0.0 ? -currPre.RHSConstant : 0.0);
                                    if (preBounds.second > flipped) {
                                        preBounds.second = flipped;
                                        if (debug) {
                                            cout << "Pre " << api << ") has a precondition <= " << preBounds.first << endl;
                                        }                                    
                                    }
                                } else {
                                    if (debug) {
                                        cout << "Pre " << api << ") has a precondition <= " << *(RPGBuilder::getPNE(currPre.LHSVariable - pneCount)) << ", ignoring\n";
                                    }
                                    
                                }
                            } else {
                                const RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                                if (currAV.size == 1) {
                                    double newRHS = currPre.RHSConstant - currAV.constant;
                                    if (currAV.fluents[0] < pneCount) {
                                        if (currAV.fluents[0] == pne) {
                                            
                                            if (debug) {
                                                cout << pi << ") has a precondition >= " << newRHS << endl;
                                            }                                        
                                            
                                            if (preBounds.first < newRHS) {
                                                preBounds.first = newRHS;
                                                
                                            }
                                        } else {
                                            if (debug) {
                                                cout << "Pre " << api << ") has a precondition on " << *(RPGBuilder::getPNE(currAV.fluents[0])) << ", ignoring\n";
                                            }
                                        }
                                    } else if (currAV.fluents[0] < 2 * pneCount) {
                                        if (newRHS != 0.0) newRHS = -newRHS;
                                        if (currAV.fluents[0] == pne + pneCount) {

                                            if (debug) {
                                                cout << pi << ") has a precondition <= " << newRHS << endl;
                                            }
                                            
                                            if (preBounds.second > newRHS) {
                                                preBounds.second = newRHS;
                                            } else {
                                                if (debug) {
                                                    cout << "Pre " << api << ") has a precondition on " << *(RPGBuilder::getPNE(currAV.fluents[0]-pneCount)) << ", ignoring\n";
                                                }
                                            }
                                            
                                        }
                                    }                                
                                } else {
                                    if (debug) {
                                        cout << "Pre " << api << ") has a precondition on " << currAV.size << " variables, ignoring\n";
                                    }
                                    
                                }
                            }
                        }
                    
                        double netEffectOnPNE = 0.0;
                                        
                        list<int>::const_iterator otherEffItr = actionEffs[actItr->first].begin();
                        const list<int>::const_iterator otherEffEnd = actionEffs[actItr->first].end();
                        
                        for (; otherEffItr != otherEffEnd; ++otherEffItr) {
                            const RPGBuilder::RPGNumericEffect & actEff = numEffs[*otherEffItr];
                            
                            if (actEff.fluentIndex == pne) {
                                netEffectOnPNE += actEff.constant;
                            }
                        }
                    
                        if (netEffectOnPNE > 0.000001) {                    
                            if (preBounds.second == DBL_MAX) {
                                if (debug) cout << pi << ") " << *(RPGBuilder::getInstantiatedOp(actItr->first)) << " can increase indefinitely\n";
                                newBounds.second = DBL_MAX;
                                if (newBounds.first == -DBL_MAX) break;
                            } else {                            
                                preBounds.second += netEffectOnPNE;
                                if (debug) cout << pi << ") " << *(RPGBuilder::getInstantiatedOp(actItr->first)) << " can increase to " << preBounds.second << endl;
                                if (newBounds.second < preBounds.second) {
                                    newBounds.second = preBounds.second;
                                }
                            }
                        } else if (netEffectOnPNE < 0.000001) {
                            if (preBounds.first == -DBL_MAX) {
                                newBounds.first = -DBL_MAX;
                                if (debug) cout << pi << ") can decrease indefinitely\n";
                                if (newBounds.second == DBL_MAX) break;
                            } else {
                                preBounds.first += netEffectOnPNE;
                                if (debug) cout << pi << ") can decrease to " << preBounds.first << endl;
                                if (newBounds.first > preBounds.first) {
                                    newBounds.first = preBounds.first;
                                }
                            }
                        }
                        
                    } else {
                        
                        if (numEffs[i].constant > 0.0) {
                            if (debug) cout << pi << ") " << *(RPGBuilder::getInstantiatedOp(actItr->first)) << " can assign to " << numEffs[i].constant << endl;
                            if (newBounds.second < numEffs[i].constant) {
                                newBounds.second = numEffs[i].constant;
                            }
                        } else if (numEffs[i].constant < 0.0) {
                            if (debug) cout << pi << ") " << *(RPGBuilder::getInstantiatedOp(actItr->first)) << " can assign to " << numEffs[i].constant << endl;
                            if (newBounds.first > numEffs[i].constant) {
                                newBounds.first = numEffs[i].constant;
                            }   
                        }
                        
                    }
                }
            }
        }
        
        if (debug) {
            cout << "New bounds are [";
            if (newBounds.first == -DBL_MAX) {
                cout << "-inf,";
            } else {
                cout << newBounds.first << ",";
            } 
            if (newBounds.second == DBL_MAX) {
                cout << "inf]\n";
            } else {
                cout << newBounds.second << "]\n";
            }
        }
        
        if (newBounds.first > pneBounds[pne].first) {
            pneBounds[pne].first = newBounds.first;
        }
        
        if (newBounds.second < pneBounds[pne].second) {
            pneBounds[pne].second = newBounds.second;
        }
        
        assert(initialFluents[pne] <= pneBounds[pne].second);
        assert(initialFluents[pne] >= pneBounds[pne].first);
    }
    
    for (int pne = 0; pne < pneCount; ++pne) {
        cout << *(RPGBuilder::getPNE(pne)) << endl;
        assert(initialFluents[pne] <= pneBounds[pne].second);
        assert(initialFluents[pne] >= pneBounds[pne].first);
    }
    
    if (debug) {
       
        cout << "Bounds on variables:\n";
        
        for (int pne = 0; pne < pneCount; ++pne) {
            if (RPGBuilder::getDominanceConstraints()[pne] != E_IRRELEVANT && RPGBuilder::getDominanceConstraints()[pne] != E_METRIC) {
                cout << "\t" << *(RPGBuilder::getPNE(pne)) << " in [";
                if (pneBounds[pne].first == -DBL_MAX) {
                    cout << "-inf,";
                } else {
                    cout << pneBounds[pne].first << ",";
                }
                if (pneBounds[pne].second == DBL_MAX) {
                    cout << "inf]\n";
                } else {
                    cout << pneBounds[pne].second << "]\n";
                }
            } else {
                if (RPGBuilder::getDominanceConstraints()[pne] == E_IRRELEVANT) {
                    cout << "\t" << *(RPGBuilder::getPNE(pne)) << " - irrelevant\n";
                } else {
                    cout << "\t" << *(RPGBuilder::getPNE(pne)) << " - only used in metric\n";
                }
            }
        }
    }
    
    
}

void NumericAnalysis::getIncreaseEffects(const int& act, std::map< int, double > & localEffs, const int excludeEffect)
{
    const list<int> & numEffs = RPGBuilder::getActionsToRPGNumericStartEffects()[act];
    
    if (numEffs.empty()) return;
    
    const vector<RPGBuilder::RPGNumericEffect> & numEffTable = RPGBuilder::getNumericEff();
        
    list<int>::const_iterator neItr = numEffs.begin();
    const list<int>::const_iterator neEnd = numEffs.end();
    
    for (; neItr != neEnd; ++neItr) {
        
        if (*neItr == excludeEffect) continue;
        
        const RPGBuilder::RPGNumericEffect & currEff = numEffTable[*neItr];
        
        if (currEff.isAssignment || currEff.size) {
            continue;
        }
        if (RPGBuilder::getDominanceConstraints()[currEff.fluentIndex] == E_IRRELEVANT || RPGBuilder::getDominanceConstraints()[currEff.fluentIndex] == E_METRIC) {
            continue;
        }
        
        localEffs.insert(make_pair(currEff.fluentIndex, 0.0)).first->second += currEff.constant;
    }
    
}


map<int, map<int, double> > NumericAnalysis::resourcesExchangedForProposition;
map<int, map<int,double> > NumericAnalysis::resourceExchangeRateToIncrease;
map<int, map<int,double> > NumericAnalysis::resourceExchangeRateToDecrease;

void NumericAnalysis::identifySimplePropositionNumberPumps()
{

    static const bool debug = false;
    
    const vector<list<pair<int, VAL::time_spec> > > & presToActions = RPGBuilder::getProcessedPreconditionsToActions();
    const vector<list<pair<int, VAL::time_spec> > > & effsToActions = RPGBuilder::getEffectsToActions();    
    
    const vector<list<Literal*> > & actionDels = RPGBuilder::getStartDeleteEffects();
    const vector<list<Literal*> > & actionAdds = RPGBuilder::getStartAddEffects();
    
    const int factCount = presToActions.size();

    
    
    for (int fID = 0; fID < factCount; ++fID) {
    
        
        map<int,double> resourceConsumptionUponBeingAdded;
        
        
        {
            bool firstAdder = true;
            
            list<pair<int, VAL::time_spec> >::const_iterator addItr = effsToActions[fID].begin();
            const list<pair<int, VAL::time_spec> >::const_iterator addEnd = effsToActions[fID].end();
            
            for (; addItr != addEnd; ++addItr) {
                
                map<int,double> localEffs;                
                getIncreaseEffects(addItr->first, localEffs);

                if (firstAdder) {
                    resourceConsumptionUponBeingAdded.swap(localEffs);
                    firstAdder = false;
                } else {
                    map<int,double> isect;
                    
                    std::set_intersection(localEffs.begin(), localEffs.end(), resourceConsumptionUponBeingAdded.begin(), resourceConsumptionUponBeingAdded.end(),
                                          insert_iterator<map<int,double> >(isect, isect.begin()));
                                          
                    isect.swap(resourceConsumptionUponBeingAdded);
                }
                
                if (resourceConsumptionUponBeingAdded.empty()) {
                    break;
                }
            }
        }
        
        if (resourceConsumptionUponBeingAdded.empty()) {
            continue;
        }
        
        if (debug) {
            cout << "Have some sort of consistent resource consumption to get " << *(RPGBuilder::getLiteral(fID)) << endl;
        }
        // now we've established that all means of adding this fact have a common subset of resource behaviour
        // next, see if all the actions requiring this fact want to delete it and put the resource back to where it was
        
        {
            
            bool allNeedingItDeleteIt = true;
            
            
            list<pair<int, VAL::time_spec> >::const_iterator needItr = presToActions[fID].begin();
            const list<pair<int, VAL::time_spec> >::const_iterator needEnd = presToActions[fID].end();
            
            for (; needItr != needEnd; ++needItr) {
                bool deletesFact = false;
                
                {
                    list<Literal*>::const_iterator delItr = actionDels[needItr->first].begin();
                    const list<Literal*>::const_iterator delEnd = actionDels[needItr->first].end();
                    
                    for (; delItr != delEnd; ++delItr) {
                        if ((*delItr)->getStateID() == fID) {
                            deletesFact = true;
                            break;
                        }
                    }
                }
                
                if (deletesFact) {
                    list<Literal*>::const_iterator addItr = actionAdds[needItr->first].begin();
                    const list<Literal*>::const_iterator addEnd = actionAdds[needItr->first].end();
                    
                    for (; addItr != addEnd; ++addItr) {
                        if ((*addItr)->getStateID() == fID) {
                            deletesFact = false;
                            break;
                        }
                    }
                }
                
                if (!deletesFact) {
                    allNeedingItDeleteIt = false;
                    break;
                }
            }
            if (!allNeedingItDeleteIt) {
                // If not all the actions needing this fact delete it, we don't have a trivial
                // swap between a resource amount and the proposition
                if (debug) {
                    cout << "- But, not every action needing the fact deletes it\n";
                }
                continue;
            }
            
        }
        

        // Now we see what the effects of each deletor is.  Intersect with the assumption
        // that we precisely undo the effects we got when adding it.
                    
                                
        map<int,double> resourceConsumptionUponBeingDeleted = resourceConsumptionUponBeingAdded;
        
        {
            map<int,double>::iterator flip = resourceConsumptionUponBeingDeleted.begin();
            const map<int,double>::iterator flipEnd = resourceConsumptionUponBeingDeleted.end();
            
            for (; flip != flipEnd; ++flip) {
                flip->second *= -1.0;
            }
        }
                    
                                
        {
            list<pair<int, VAL::time_spec> >::const_iterator needItr = presToActions[fID].begin();
            const list<pair<int, VAL::time_spec> >::const_iterator needEnd = presToActions[fID].end();
            
            for (; needItr != needEnd; ++needItr) {
                map<int,double> localEffs;                
                getIncreaseEffects(needItr->first, localEffs);
                
                map<int,double> isect;
                
                std::set_intersection(localEffs.begin(), localEffs.end(), resourceConsumptionUponBeingDeleted.begin(), resourceConsumptionUponBeingDeleted.end(),
                                      insert_iterator<map<int,double> >(isect, isect.begin()));
                
                isect.swap(resourceConsumptionUponBeingDeleted);
                
                if (resourceConsumptionUponBeingDeleted.empty()) {
                    if (debug) {
                        cout << " - Resource effects of " << *(RPGBuilder::getInstantiatedOp(needItr->first)) << " are not consistent with the others\n";
                    }
                    break;
                }
            }
        }
    
        if (resourceConsumptionUponBeingDeleted.empty()) {
            continue;
        }
        
        
        if (debug) {
            cout << *(RPGBuilder::getLiteral(fID)) << " is two-exchangeable with ";
            map<int,double>::iterator flip = resourceConsumptionUponBeingDeleted.begin();
            const map<int,double>::iterator flipEnd = resourceConsumptionUponBeingDeleted.end();
            
            for (bool s = false; flip != flipEnd; ++flip, s = true) {
                if (s) cout << ", ";
                cout << flip->second << " of " << *(RPGBuilder::getPNE(flip->first));
            }
            cout << endl;
        }
        
        map<int,double> & store = resourcesExchangedForProposition[fID];
        store.swap(resourceConsumptionUponBeingDeleted);
        
    }
    
}

void NumericAnalysis::identifySimpleReversibleNumberPumps()
{
    static const bool debug = false;
    
    map<int, map<double, map<int,double> > > resourcesExchangedForResourceIncrease;
    map<int, map<double, map<int,double> > > resourcesExchangedForResourceDecrease;
        
    const vector<RPGBuilder::RPGNumericEffect> & numEffTable = RPGBuilder::getNumericEff();
    const vector<list<pair<int, VAL::time_spec> > > & effsToActions = RPGBuilder::getRpgNumericEffectsToActions();
    
    const int neCount = numEffTable.size();
    
    for (int ne = 0; ne < neCount; ++ne) {
        
        const RPGBuilder::RPGNumericEffect & currEff = numEffTable[ne];
        
        if (RPGBuilder::getDominanceConstraints()[currEff.fluentIndex] == E_IRRELEVANT || RPGBuilder::getDominanceConstraints()[currEff.fluentIndex] == E_METRIC) {
            continue;            
        }
        
        map<int,double> resourceConsumptionUponBeingAdded;
        
        
        {
            bool firstAdder = true;
            
            list<pair<int, VAL::time_spec> >::const_iterator addItr = effsToActions[ne].begin();
            const list<pair<int, VAL::time_spec> >::const_iterator addEnd = effsToActions[ne].end();
            
            for (; addItr != addEnd; ++addItr) {
                
                map<int,double> localEffs;                
                getIncreaseEffects(addItr->first, localEffs,ne);

                if (firstAdder) {
                    resourceConsumptionUponBeingAdded.swap(localEffs);
                    firstAdder = false;
                } else {
                    map<int,double> isect;
                    
                    std::set_intersection(localEffs.begin(), localEffs.end(), resourceConsumptionUponBeingAdded.begin(), resourceConsumptionUponBeingAdded.end(),
                                          insert_iterator<map<int,double> >(isect, isect.begin()));
                                          
                    isect.swap(resourceConsumptionUponBeingAdded);
                }
                
                if (resourceConsumptionUponBeingAdded.empty()) {
                    break;
                }
            }
        }
        
        if (resourceConsumptionUponBeingAdded.empty()) {
            continue;
        }
        
        if (currEff.constant > 0.0) {
            /*cout << "All the actions creating " << currEff.constant << " of " << *(RPGBuilder::getPNE(currEff.fluentIndex)) << " also do: ";
            map<int,double>::iterator flip = resourceConsumptionUponBeingAdded.begin();
            const map<int,double>::iterator flipEnd = resourceConsumptionUponBeingAdded.end();
            
            for (bool s = false; flip != flipEnd; ++flip, s = true) {
                if (s) cout << ", ";
                cout << flip->second << " of " << *(RPGBuilder::getPNE(flip->first));
            }
            cout << endl;*/
            resourcesExchangedForResourceIncrease[currEff.fluentIndex][currEff.constant].swap(resourceConsumptionUponBeingAdded);            
        } else if (currEff.constant < 0.0) {
            resourcesExchangedForResourceDecrease[currEff.fluentIndex][currEff.constant].swap(resourceConsumptionUponBeingAdded);
        }
    }
    
    map<int, pair<map<int,double>,map<int,double> > > minMaxProdToIncrease;
    map<int, pair<map<int,double>,map<int,double> > > minMaxProdToDecrease;
    
    for (int pass = 0; pass < 2; ++pass) {
        const map<int, map<double, map<int,double> > > & commonEffects = (pass ? resourcesExchangedForResourceDecrease : resourcesExchangedForResourceIncrease);
        map<int, pair<map<int,double>,map<int,double> > > & dest = (pass ? minMaxProdToDecrease : minMaxProdToIncrease);
        
        map<int, map<double, map<int,double> > >::const_iterator rItr = commonEffects.begin();
        const map<int, map<double, map<int,double> > >::const_iterator rEnd = commonEffects.end();
                
        for (; rItr != rEnd; ++rItr) {
            
            bool firstEffect = true;
            
            pair<map<int,double>,map<int,double> > & minMaxUsage = dest[rItr->first];
            
            
            map<double, map<int,double> >::const_iterator effItr = rItr->second.begin();
            const map<double, map<int,double> >::const_iterator effEnd = rItr->second.end();
            
            for (; effItr != effEnd; ++effItr) {
                if (debug) {
                    cout << "Considering effects ";
                    if (pass) {
                        cout << "decreasing ";
                    } else {
                        cout << "increasing ";                        
                    }
                    cout << *(RPGBuilder::getPNE(rItr->first)) << " by " << effItr->first << endl;
                }
                map<int,double> normalised = effItr->second;
                
                {
                    map<int,double>::iterator nItr = normalised.begin();
                    const map<int,double>::iterator nEnd = normalised.end();
                    
                    for (; nItr != nEnd; ++nItr) {
                        nItr->second /= effItr->first;
                    }

                }
                                
                if (firstEffect) {
                    minMaxUsage.first.swap(normalised);
                    minMaxUsage.second = minMaxUsage.first;
                    if (debug) {
                        cout << " - is the first such effect on this variable\n";
                    }
                    firstEffect = false;
                } else {
                    {
                        // take the max consumption of resources of this and all previous effects
                        // (in places where this doesn't consume a given resource, assume it consumes 0.0)
                        
                        map<int,double>::const_iterator nItr = normalised.begin();
                        const map<int,double>::const_iterator nEnd = normalised.end();
                        
                        map<int,double>::iterator erItr = minMaxUsage.first.begin();
                        const map<int,double>::iterator erEnd = minMaxUsage.first.begin();
                        
                        while (nItr != nEnd && erItr != erEnd) {
                            if (erItr->first < nItr->first) {
                                if (erItr->second > 0.0) {
                                    erItr->second = 0.0;                                    
                                }
                                ++erItr;
                            } else if (nItr->first < erItr->first) {
                                if (nItr->second < 0.0) {
                                    erItr = minMaxUsage.first.insert(erItr, *nItr);
                                    ++erItr;
                                }
                                ++nItr;
                            } else {
                                if (nItr->second < erItr->second) {
                                    erItr->second = nItr->second;
                                }
                            }
                        }
                        
                        if (nItr != nEnd) {
                            for (; nItr != nEnd; ++nItr) {
                                if (nItr->second < 0.0) {
                                    erItr = minMaxUsage.first.insert(erItr, *nItr);
                                }
                            }
                        } else if (erItr != erEnd) {
                            for (; erItr != erEnd; ++erItr) {
                                if (erItr->second > 0.0) {
                                    erItr->second = 0.0;                                    
                                }                                                                
                            }
                        }
                    }
                    
                    {
                        // take the max production of resources of this and all previous effects
                        // (in places where this doesn't produce a given resource, assume it produces 0.0)
                                                
                        map<int,double>::const_iterator nItr = normalised.begin();
                        const map<int,double>::const_iterator nEnd = normalised.end();
                        
                        map<int,double>::iterator erItr = minMaxUsage.second.begin();
                        const map<int,double>::iterator erEnd = minMaxUsage.second.begin();
                        
                        while (nItr != nEnd && erItr != erEnd) {
                            if (erItr->first < nItr->first) {
                                if (erItr->second < 0.0) {
                                    erItr->second = 0.0;                                    
                                }
                                ++erItr;
                            } else if (nItr->first < erItr->first) {
                                if (nItr->second > 0.0) {
                                    erItr = minMaxUsage.second.insert(erItr, *nItr);
                                    ++erItr;
                                }
                                ++nItr;
                            } else {
                                if (nItr->second > erItr->second) {
                                    erItr->second = nItr->second;
                                }
                            }
                        }
                        
                        if (nItr != nEnd) {
                            for (; nItr != nEnd; ++nItr) {
                                if (nItr->second > 0.0) {
                                    erItr = minMaxUsage.second.insert(erItr, *nItr);
                                }
                            }
                        } else if (erItr != erEnd) {
                            for (; erItr != erEnd; ++erItr) {
                                if (erItr->second < 0.0) {
                                    erItr->second = 0.0;                                    
                                }                                                                
                            }
                        }
                    }
                }
            }
            
            if (debug) {

                {
                    cout << "Left with min exchange rates of:";
                    map<int,double>::iterator flip = minMaxUsage.first.begin();
                    const map<int,double>::iterator flipEnd = minMaxUsage.first.end();
                    
                    for (bool s = false; flip != flipEnd; ++flip, s = true) {
                        if (s) cout << ", ";
                        cout << flip->second << " of " << *(RPGBuilder::getPNE(flip->first));
                    }
                    cout << endl;
                }
                                 
                {
                    cout << "Left with max exchange rates of:";
                    map<int,double>::iterator flip = minMaxUsage.second.begin();
                    const map<int,double>::iterator flipEnd = minMaxUsage.second.end();
                    
                    for (bool s = false; flip != flipEnd; ++flip, s = true) {
                        if (s) cout << ", ";
                        cout << flip->second << " of " << *(RPGBuilder::getPNE(flip->first));
                    }
                    cout << endl;
                }
                                 
            }
            
        }
    
    }

     for (int pass = 0; pass < 2; ++pass) {
         map<int, pair<map<int,double>,map<int,double> > > & src = (pass ? minMaxProdToDecrease : minMaxProdToIncrease);
         
         map<int, map<int,double> > & dest = (pass ? resourceExchangeRateToDecrease : resourceExchangeRateToIncrease);
         
         map<int, pair<map<int,double>,map<int,double> > >::const_iterator sItr = src.begin();
         const map<int, pair<map<int,double>,map<int,double> > >::const_iterator sEnd = src.end();
         
         for (; sItr != sEnd; ++sItr) {
             
             if (debug) {
                 cout << "Looking for equal min-max ";
                 if (pass) {
                     cout << " decreasing ";
                 } else {
                     cout << " increasing ";
                 }
                 cout << "effects on " << *(RPGBuilder::getPNE(sItr->first)) << endl;
             }
             map<int,double> common;
             
             map<int,double>::const_iterator lItr = sItr->second.first.begin();
             const map<int,double>::const_iterator lEnd = sItr->second.first.end();
             
             map<int,double>::const_iterator uItr = sItr->second.second.begin();
             const map<int,double>::const_iterator uEnd = sItr->second.second.end();
             
             while (lItr != lEnd && uItr != uEnd) {
                 if (lItr->first < uItr->first) {                     
                     ++lItr;
                 } else if (uItr->first < lItr->first) {
                     ++uItr;
                 } else {
                     if (fabs(lItr->second - uItr->second) < 0.00000001) {
                         common.insert(*lItr);
                     }
                    ++lItr;
                    ++uItr;
                 }
             }
             if (!common.empty()) {
                 map<int,double> & tmp = dest[sItr->first];
                 tmp.swap(common);
                 if (debug) {
                     cout << "All the actions";
                     if (pass) {
                         cout << " decreasing ";
                     } else {
                         cout << " increasing ";
                     }
                     cout << *(RPGBuilder::getPNE(sItr->first)) << " also do: ";
                     map<int,double>::iterator flip = tmp.begin();
                     const map<int,double>::iterator flipEnd = tmp.end();
                     
                     for (bool s = false; flip != flipEnd; ++flip, s = true) {
                         if (s) cout << ", ";
                         cout << flip->second << " of " << *(RPGBuilder::getPNE(flip->first));
                     }
                     cout << endl;
                 }
             }
         }         
     }
     
     
}



};
