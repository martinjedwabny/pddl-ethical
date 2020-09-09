#include "MetricRPG.h"

#include "colours.h"

using std::endl;

namespace Planner {
  
    
bool MetricRPG::doneInitialisation = false;

vector<map<int,double> > MetricRPG::increaseEffectsOfEachAction;
vector<map<int,double> > MetricRPG::assignmentEffectsOfEachAction;

vector<double> MetricRPG::maxNeeded;
vector<double> MetricRPG::minNeeded;

MetricRPG::~MetricRPG()
{
    vector<LayerData*>::const_iterator ldItr = fluentLayers.begin();
    const vector<LayerData*>::const_iterator ldEnd = fluentLayers.end();
    
    for (; ldItr != ldEnd; ++ldItr) {
        delete *ldItr;
    }
    
}


    
MetricRPG::MetricRPG(MinimalState * forKShot, const vector<double> & layerZero, const vector<double> * const actionCosts)
    : layerCount(0), startingState(forKShot), propositionalActionCosts(actionCosts)
{
    

    fluentLayers.reserve(10);
    fluentLayers.push_back(new LayerData(layerZero, 2 * RPGBuilder::getPNECount()));
    
    
    if (!doneInitialisation) {
        
        static const int pneCount = RPGBuilder::getPNECount();
        doneInitialisation = true;
        
        
        const vector<RPGBuilder::RPGNumericEffect> & numEffs = RPGBuilder::getNumericEffs();
        const vector<list<int> > & actionEffects = RPGBuilder::getActionsToRPGNumericStartEffects();
        
        const int actCount = actionEffects.size();
        increaseEffectsOfEachAction.resize(actCount);
        assignmentEffectsOfEachAction.resize(actCount);
        
        for (int a = 0; a < actCount; ++a) {
            
            if (RPGBuilder::rogueActions[a]) {
                continue;
            }
            
            {
                list<int>::const_iterator effItr = actionEffects[a].begin();
                const list<int>::const_iterator effEnd = actionEffects[a].end();
                
                for (; effItr != effEnd; ++effItr) {
                    const RPGBuilder::RPGNumericEffect & currEff = numEffs[*effItr];
                    
                    if (currEff.isAssignment) {
                        assignmentEffectsOfEachAction[a][currEff.fluentIndex] = currEff.constant;
                    } else {
                        increaseEffectsOfEachAction[a].insert(make_pair(currEff.fluentIndex, 0.0)).first->second += currEff.constant;
                    }
                }            
            }
            
            map<int,double> reprocessEffects;
            reprocessEffects.swap(increaseEffectsOfEachAction[a]);
            map<int,double>::iterator insItr = increaseEffectsOfEachAction[a].end();
            
            map<int,double>::const_iterator effItr = reprocessEffects.begin();
            const map<int,double>::const_iterator effEnd = reprocessEffects.end();
            
            for (; effItr != effEnd; ++effItr) {
                if (effItr->second > 0.0) {
                    insItr = increaseEffectsOfEachAction[a].insert(insItr, *effItr);
                } else if (effItr->second < 0.0) {
                    insItr = increaseEffectsOfEachAction[a].insert(insItr, make_pair(effItr->first + pneCount, -effItr->second));
                }
            }
        }
        
        const vector<RPGBuilder::RPGNumericPrecondition> & numericPrecs = RPGBuilder::getNumericPrecs();
                
        const int precCount = numericPrecs.size();

        minNeeded.resize(pneCount, DBL_MAX);
        maxNeeded.resize(pneCount, -DBL_MAX);
        
        for (int i = 0; i < precCount; ++i) {
            double RHS = numericPrecs[i].RHSConstant;
            const bool ge = (numericPrecs[i].op == VAL::E_GREATER);
            int lhsVar = numericPrecs[i].LHSVariable;

            if (lhsVar < pneCount) {
                if (ge) RHS += 0.001;
                if (maxNeeded[lhsVar] < RHS) maxNeeded[lhsVar] = RHS;
            } else if (lhsVar < (2 * pneCount)) {
                lhsVar -= pneCount;
                if (RHS != 0.0) RHS = 0.0 - RHS;
                if (ge) RHS -= 0.001;
                if (minNeeded[lhsVar] > RHS) minNeeded[lhsVar] = RHS;
            } else {


                RPGBuilder::ArtificialVariable aVar = RPGBuilder::getArtificialVariable(lhsVar);
                bool anyNeg = false;
                bool anyPos = false;
                for (int s = 0; s < aVar.size; ++s) {
                    if (aVar.fluents[s] < pneCount) {
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
                        if (aVar.fluents[s] < pneCount) maxNeeded[aVar.fluents[s]] = DBL_MAX;
                    }
                }

                if (!anyPos) {
                    RHS -= aVar.constant;
                    if (RHS != 0.0) RHS = 0.0 - RHS;
                    if (ge) RHS = RHS - 1.0;
                    if (RHS < 0.0) RHS = 0.0;
                    for (int s = 0; s < aVar.size; ++s) {
                        if (minNeeded[aVar.fluents[s] - pneCount] > RHS) minNeeded[aVar.fluents[s] - pneCount] = RHS;
                    }
                } else {
                    for (int s = 0; s < aVar.size; ++s) {
                        if (aVar.fluents[s] >= pneCount) minNeeded[aVar.fluents[s] - pneCount] = 0.0;
                    }
                }


            }


        }
    }
    
    actionHasAppeared.resize(increaseEffectsOfEachAction.size(), false);
}


void MetricRPG::addApplicableActions(vector<int> * const propPrec, vector<int> * const numPrec, const double & newTS)
{
    static const int pneCount = RPGBuilder::getPNECount();
    
    ++layerCount;
    timestampToIndex[newTS] = layerCount;
    
    const LayerData * const prevLayer = fluentLayers.back();
        
    LayerData * const currLayer = new LayerData(*prevLayer, true);
    
    fluentLayers.push_back(currLayer);
    
    const int ailSize = propPrec->size();
    for (int i = 0; i < ailSize; ++i) {
        if (!RPGBuilder::rogueActions[i] && !actionHasAppeared[i] && !((*propPrec)[i]) && !((*numPrec)[i])) {
            actionHasAppeared[i] = true;
            
            if (layerCount == 1) {
                potentiallyHelpful.insert(i);
            }
            
            
            {                
                map<int,double>::const_iterator assItr = assignmentEffectsOfEachAction[i].begin();
                const map<int,double>::const_iterator assEnd = assignmentEffectsOfEachAction[i].end();
                
                for (; assItr != assEnd; ++assItr) {
                    
                    if (assItr->second > prevLayer->fluentValues[assItr->first]) {
                        pair<int,double> & toUpdatePositive = currLayer->biggestAssignmentAction[assItr->first];
                        if (toUpdatePositive.first == -1 || assItr->second > toUpdatePositive.second) {
                            toUpdatePositive.first = i;
                            toUpdatePositive.second = assItr->second;
                        }
                    }
                    
                    if (-assItr->second > prevLayer->fluentValues[assItr->first + pneCount]) {
                        pair<int,double> & toUpdateNegative = currLayer->biggestAssignmentAction[assItr->first + pneCount];
                        if (toUpdateNegative.first == -1 || -assItr->second > toUpdateNegative.second) {
                            toUpdateNegative.first = i;
                            toUpdateNegative.second = -assItr->second;
                        }
                    }
                }
            }
            
            {
                map<int,double>::const_iterator incItr = increaseEffectsOfEachAction[i].begin();
                const map<int,double>::const_iterator incEnd = increaseEffectsOfEachAction[i].end();
                
                for (; incItr != incEnd; ++incItr) {                    
                    /*
                    if (incItr->first >= RPGBuilder::getPNECount()) {
                        cout << COLOUR_light_green << "Increasers of -" << *(RPGBuilder::getPNE(incItr->first - RPGBuilder::getPNECount())) << " in RPG\n" << COLOUR_default;
                    } else {
                        cout << COLOUR_light_green << "Increasers of " << *(RPGBuilder::getPNE(incItr->first)) << " in RPG\n" << COLOUR_default;
                    } 
                    */
                    currLayer->increasersAvailable[incItr->first].push_back(i);                    
                }                        
            }
        }
    }
    
}

void MetricRPG::fillMinMaxFluentTable(vector<double> & toFill)
{

    static const bool debug = false;
    
    LayerData * const currLayer = fluentLayers.back();
    
    static const int pneCount = RPGBuilder::getPNECount();
    
    for (int v = 0; v < pneCount; ++v) {
        if (RPGBuilder::getDominanceConstraints()[v] != E_METRIC && RPGBuilder::getDominanceConstraints()[v] != E_IRRELEVANT) {
            
            if (debug) {
                cout << "Bounding fluent " << *(RPGBuilder::getPNE(v)) << ", from initially " << startingState->second[v] << ":";
                cout.flush();
            }
            if (toFill[v] < maxNeeded[v]) {                
                double accumulatedIncrease = currLayer->fluentValues[v];
                list<int>::const_iterator incItr = currLayer->increasersAvailable[v].begin();
                const list<int>::const_iterator incEnd = currLayer->increasersAvailable[v].end();
                
                for (; incItr != incEnd; ++incItr) {
                    accumulatedIncrease += increaseEffectsOfEachAction[*incItr][v] * 1000;
                }
                
                if (currLayer->biggestAssignmentAction[v].first == -1) {
                    currLayer->fluentValues[v] = accumulatedIncrease;
                } else {
                    if (currLayer->biggestAssignmentAction[v].second >= accumulatedIncrease) {
                        currLayer->fluentValues[v] = currLayer->biggestAssignmentAction[v].second;
                        currLayer->assignmentWasBetter[v] = true;
                    } else {
                        currLayer->fluentValues[v] = accumulatedIncrease;
                    }
                }
                toFill[v] = currLayer->fluentValues[v];
                if (debug) {
                    cout << " upper = " << toFill[v];
                    cout.flush();
                }
            }

            if (-toFill[v+pneCount] > minNeeded[v]) {                
                double accumulatedIncrease = currLayer->fluentValues[v + pneCount];
                list<int>::const_iterator incItr = currLayer->increasersAvailable[v + pneCount].begin();
                const list<int>::const_iterator incEnd = currLayer->increasersAvailable[v + pneCount].end();
                
                if (debug) {
                    cout << " decreasors: " << currLayer->increasersAvailable[v + pneCount].size() << ", so";
                }
                
                for (; incItr != incEnd; ++incItr) {
                    accumulatedIncrease += increaseEffectsOfEachAction[*incItr][v + pneCount];
                }
                                
                if (currLayer->biggestAssignmentAction[v + pneCount].first == -1) {
                    currLayer->fluentValues[v + pneCount] = accumulatedIncrease;
                } else {
                    if (currLayer->biggestAssignmentAction[v + pneCount].second >= accumulatedIncrease) {
                        currLayer->fluentValues[v + pneCount] = currLayer->biggestAssignmentAction[v].second;
                        currLayer->assignmentWasBetter[v + pneCount] = true;
                    } else {
                        currLayer->fluentValues[v + pneCount] = accumulatedIncrease;
                    }
                }
                toFill[v + pneCount] = currLayer->fluentValues[v + pneCount];
                if (debug) {
                    cout << " lower = " << -toFill[v + pneCount];
                    cout.flush();
                }
            }
            if (debug) {
                cout << endl;
            }
        } else {
            toFill[v] = 0.0;
            toFill[v + pneCount] = 0.0;
        }
    }
    
}

void MetricRPG::getActionsFor(const double & layer,
                              const int & var, const bool & min, const double & RHS,
                              list<pair<int, double> > & retList)
{
    
    static const bool debug = false;
    
    static const int pneCount = RPGBuilder::getPNECount();
    
    const RPGBuilder::ArtificialVariable * currAV = 0;
    
    double internalRHS = (min ? (RHS != 0.0 ? -RHS : 0) : RHS);
    
    if (var >= 2 * pneCount) {
        currAV = &(RPGBuilder::getArtificialVariable(var));
        
        if (debug) {
            if (min) {
                cout << "Asking for " << *currAV << " <= " << RHS << endl;
            } else {
                cout << "Asking for " << *currAV << " >= " << RHS << endl;
            }
        }
    } else if (debug) {
        if (var < pneCount) {
            cout << "Asking for " << *(RPGBuilder::getPNE(var)) << " >= " << internalRHS << endl;
        } else {
            cout << "Asking for -" << *(RPGBuilder::getPNE(var-pneCount)) << " >= " << internalRHS << endl;
        }
    }
        
    int layerIndex = timestampToIndex[layer];
    if (!layerIndex) {
        // is true in the state being evaluated
        return;        
    }
    
    if (debug) {
        cout << "Starting at layer " << layerIndex << endl;
    }       
    
    while (true) {
        --layerIndex;
        
        LayerData* currLayer = fluentLayers[layerIndex];
        
        
        double valueAtPreviousLayer;
        
        while (true) {
            if (currAV) {
                valueAtPreviousLayer = currAV->evaluate(currLayer->fluentValues);
            } else {
                if (min) {
                    valueAtPreviousLayer = currLayer->fluentValues[var + pneCount];
                } else {
                    valueAtPreviousLayer = currLayer->fluentValues[var];
                }
            }
            
            if (debug) {
                cout << "Value in layer " << layerIndex << " = " << valueAtPreviousLayer << endl;
            }
            if (valueAtPreviousLayer < internalRHS) {
                if (debug) {
                    cout << " - Is insufficient to satisfy the precondition\n";
                }
                // is insufficient to satisfy the pre
                break;
            }
            
            if (layerIndex == 0) {
                if (debug) {
                    cout << " - Is sufficient, and in layer zero - done\n";
                }                
                // is satisfied in the state being evaluated
                return;
            }
            --layerIndex;
            currLayer = fluentLayers[layerIndex];
            
        };
        
        ++layerIndex;
        
        currLayer = fluentLayers[layerIndex];

        if (debug) {
            cout << "Earliest layer with sufficient is therefore layer " << layerIndex << " where = ";
            if (currAV) {
                cout << currAV->evaluate(currLayer->fluentValues) << endl;
            } else {
                if (min) {
                    cout << currLayer->fluentValues[var + pneCount] << endl;
                } else {
                    cout << currLayer->fluentValues[var] << endl;
                }
            }         
        }
        
        
        // At this point, layer index and currLayer are the earliest layer in which the
        // precondition holds; and valueAtPreviousLayer holds the previous value
        // of the variable (i.e. that which can be pushed back and satisfied earlier.
        
        if (currAV) {
            
            bool wasSufficient = false;
            int localV;
            for (int s = 0; !wasSufficient && s < currAV->size; ++s) {
                localV = currAV->fluents[s];
                if (debug) {
                    cout << "Considering contribution from ";
                    if (localV < pneCount) {
                        cout << *(RPGBuilder::getPNE(localV)) << endl;
                    } else {
                        cout << "-" << *(RPGBuilder::getPNE(localV - pneCount)) << endl;
                    } 
                
                }
                
                if (currLayer->assignmentWasBetter[localV]) {
                    if (debug) {
                        cout << "Best value was from assignment: deducting " << currAV->weights[s] * currLayer->biggestAssignmentAction[localV].second << " from the residual\n";
                    }
                    internalRHS -= currAV->weights[s] * currLayer->biggestAssignmentAction[localV].second;
                    retList.push_back(make_pair(currLayer->biggestAssignmentAction[localV].first, 1.0));
                    wasSufficient = (valueAtPreviousLayer >= internalRHS);
                } else {
                    if (debug) {
                        cout << "Collecting increasors\n";
                    }
                    wasSufficient = currLayer->getActionsToReach(localV, internalRHS, valueAtPreviousLayer, currAV->weights[s], retList);
                    if (debug) {
                        cout << "Residual is now " << internalRHS;
                        if (wasSufficient) {
                            cout << " - can be pushed back a layer\n";
                        } else {
                            cout << " - cannot yet be pushed back a layer\n";
                        }
                    }
                }
            }
            
            assert(wasSufficient);
            
        } else {
            
            if (currLayer->assignmentWasBetter[var]) {
                retList.push_back(make_pair(currLayer->biggestAssignmentAction[var].first, 1.0));
                return;
            } else {
                
                #ifndef NDEBUG
                const bool wasSufficient =
                #endif                
                currLayer->getActionsToReach(var, internalRHS, valueAtPreviousLayer, 1.0, retList);
                
                assert(wasSufficient);
                
            }
            
        } 
    
        
    }
}

bool MetricRPG::LayerData::getActionsToReach(const int & var,
                                             double & currentLevel, const double & targetLevel,
                                             const double & weight,
                                             list<pair<int,double> > & retList)
{
    static const bool debug = false;
    assert(!assignmentWasBetter[var]);
    
    if (debug) {
        cout << increasersAvailable[var].size() << " increasors available\n";
    }
    list<int>::const_iterator incItr = increasersAvailable[var].begin();
    const list<int>::const_iterator incEnd = increasersAvailable[var].end();
    
    for (; incItr != incEnd; ++incItr) {
        const double effectOnVar = weight * MetricRPG::increaseEffectsOfEachAction[*incItr][var];
        if (debug) {
            cout << " - Action " << *incItr << " with weighted effect " << effectOnVar << endl;
        }
        const double numNeeded = (currentLevel - targetLevel) / effectOnVar;
        if (numNeeded < 1000) {
            const int asInt = ceil(numNeeded);
            currentLevel -= asInt * effectOnVar;
            retList.push_back(make_pair(*incItr, (double) asInt));
            return true;
        } else {
            retList.push_back(make_pair(*incItr, 1000.0));
            currentLevel -= 1000 * effectOnVar;
        }
    }
    
    return false;
}



};
