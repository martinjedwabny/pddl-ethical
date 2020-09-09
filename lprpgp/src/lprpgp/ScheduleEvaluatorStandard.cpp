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

#include "ScheduleEvaluatorStandard.h"

#include <assert.h>
#include <math.h>

namespace Planner {

ScheduleEvaluatorStandard::ScheduleEvaluatorStandard(const int & c, const double & by, const bool & lin) : weights(vector<vector<double> >(c)), globalPenalty(DBL_MAX), anyPenalties(true), increaseWeightsBy(by), linearIncrease(lin), newSchedule(0), waitSchedule(0), nsBoundaryNode(0), wsBoundaryNode(0) {
	for (int i = 0; i < c; ++i) {
		weights[i] = vector<double>(c);
		for (int j = 0; j < c; ++j) weights[i][j] = 100.0;
	}

};

double ScheduleEvaluatorStandard::getWeight(const int & a, const int & b) {
	if (a <= b) {
		return (weights[a][b]);
	} else {
		return (weights[b][a]);
	}
}

void ScheduleEvaluatorStandard::incrementWeight(const int & a, const int & b, const int multiplier) {
	if (a <= b) {
		if (linearIncrease) {
			weights[a][b] += multiplier * increaseWeightsBy;
		} else {
			weights[a][b] *= pow(increaseWeightsBy, multiplier);
		}
	} else {
		if (linearIncrease) {
			weights[b][a] += multiplier * increaseWeightsBy;
		} else {
			weights[b][a] *= pow(increaseWeightsBy, multiplier);
		}
	}

}

void ScheduleEvaluatorStandard::incrementActionWeight(const int & a, const int multiplier) {
	static const double increaseWeightsBy = 0.1;

	map<int, double>::iterator awItr = actionWeights.find(a);
	if (awItr != actionWeights.end()) {
		awItr->second += multiplier * increaseWeightsBy;
	} else {
		actionWeights[a] = multiplier * increaseWeightsBy;
	}

}



void ScheduleEvaluatorStandard::addDeletes(map<Literal*, NodeOpCount, LitLT> & activeDeletes, ScheduleNode * nodeToBlame, list<Literal*> & deletes) {

	list<Literal*>::iterator dItr = deletes.begin();
	const list<Literal*>::iterator dEnd = deletes.end();

	for (; dItr != dEnd; ++dItr) {
		Literal* const currLit = *dItr;

		NodeOpCount & newOp = activeDeletes[currLit];
		if (nodeToBlame->tilNode()) {
			newOp.count[-1].insert(nodeToBlame);
		} else {
			set<int> & tbSet = nodeToBlame->getSubProblem();
			set<int>::iterator tbItr = tbSet.begin();
			const set<int>::iterator tbEnd = tbSet.end();
	
			for (; tbItr != tbEnd; ++tbItr) {
				newOp.count[*tbItr].insert(nodeToBlame);
			}
		}
	}
};
                          
void ScheduleEvaluatorStandard::addAdds(map<Literal*, NodeOpCount, LitLT> & activeDeletes, list<Literal*> & adds) {

	list<Literal*>::iterator aItr = adds.begin();
	const list<Literal*>::iterator aEnd = adds.end();

	for (; aItr != aEnd; ++aItr) {
		Literal* const currLit = *aItr;

		map<Literal*, NodeOpCount, LitLT>::iterator fLit = activeDeletes.find(currLit);
		if (fLit != activeDeletes.end()) { // this literal has been deleted
			activeDeletes.erase(fLit); // it's now been added again, mutex is now inactive
		}
	}
};


void ScheduleEvaluatorStandard::addNumerics(list<pair<ScheduleNode*, vector<double> > > & fluentTracking, ScheduleNode * nodeToBlame, list<RPGBuilder::NumericEffect> & numEffs) {


	if (numEffs.empty()) {
		//cout << "No numeric effects\n";
		return;
	}

	pair<ScheduleNode*, vector<double> > toPush(nodeToBlame, fluentTracking.back().second);

	list<RPGBuilder::NumericEffect>::iterator aItr = numEffs.begin();
	const list<RPGBuilder::NumericEffect>::iterator aEnd = numEffs.end();


	for (; aItr != aEnd; ++aItr) {
		toPush.second[aItr->fluentIndex] = aItr->applyEffect(fluentTracking.back().second);
	}

	fluentTracking.push_back(toPush);
};


void ScheduleEvaluatorStandard::addInvariants(map<Literal*, NodeOpCount, LitLT> & activeInvariants, ScheduleNode * nodeToBlame, list<Literal*> & invs) {

	list<Literal*>::iterator iItr = invs.begin();
	const list<Literal*>::iterator iEnd = invs.end();

	for (; iItr != iEnd; ++iItr) {
		Literal* const currLit = *iItr;

		NodeOpCount & newOp = activeInvariants[currLit];
		set<int> & tbSet = nodeToBlame->getSubProblem();
		set<int>::iterator tbItr = tbSet.begin();
		const set<int>::iterator tbEnd = tbSet.end();

		for (; tbItr != tbEnd; ++tbItr) {
			newOp.count[*tbItr].insert(nodeToBlame);
		}

		

	}

};

void ScheduleEvaluatorStandard::addFluentInvariants(map<int, NodeOpCount> & activeInvariants, ScheduleNode * nodeToBlame, list<int> & invs) {

	list<int>::iterator iItr = invs.begin();
	const list<int>::iterator iEnd = invs.end();

	for (; iItr != iEnd; ++iItr) {
		const int currLit = *iItr;

		NodeOpCount & newOp = activeInvariants[currLit];
		set<int> & tbSet = nodeToBlame->getSubProblem();
		set<int>::iterator tbItr = tbSet.begin();
		const set<int>::iterator tbEnd = tbSet.end();

		for (; tbItr != tbEnd; ++tbItr) {
			newOp.count[*tbItr].insert(nodeToBlame);
		}

		

	}

};


void ScheduleEvaluatorStandard::removeInvariants(map<Literal*, NodeOpCount, LitLT> & activeInvariants, ScheduleNode * nodeToBlame, list<Literal*> & invs) {

	list<Literal*>::iterator iItr = invs.begin();
	const list<Literal*>::iterator iEnd = invs.end();

	for (; iItr != iEnd; ++iItr) {
		Literal* const currLit = *iItr;

		map<Literal*, NodeOpCount, LitLT>::iterator fLit = activeInvariants.find(currLit);

		//assert(fLit != activeInvariants.end()); // this literal is not currently an invariant, which should never EVER happen

		if (fLit != activeInvariants.end()) { // can happen in a corner case (ignoring negative consequence of RPG action choices, we clear invariants which might never have been added)

			set<int> & tbSet = nodeToBlame->getSubProblem();
			set<int>::iterator tbItr = tbSet.begin();
			const set<int>::iterator tbEnd = tbSet.end();
	
			for (; tbItr != tbEnd; ++tbItr) {
				map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator fOp = fLit->second.count.find(*tbItr);
				if (fOp != fLit->second.count.end()) {;
					fOp->second.erase(nodeToBlame->getPartner());
					if (fOp->second.empty()) {
						fLit->second.count.erase(fOp);
					}
				}
			}
			if (fLit->second.count.empty()) activeInvariants.erase(fLit);

		}

	}

};

void ScheduleEvaluatorStandard::removeFluentInvariants(map<int, NodeOpCount> & activeInvariants, ScheduleNode * nodeToBlame, list<int> & invs) {

	list<int>::iterator iItr = invs.begin();
	const list<int>::iterator iEnd = invs.end();

	for (; iItr != iEnd; ++iItr) {
		const int currLit = *iItr;

		map<int, NodeOpCount>::iterator fLit = activeInvariants.find(currLit);

		//assert(fLit != activeInvariants.end()); // this literal is not currently an invariant, which should never EVER happen

		if (fLit != activeInvariants.end()) { // can happen in a corner case (ignoring negative consequence of RPG action choices, we clear invariants which might never have been added)

			set<int> & tbSet = nodeToBlame->getSubProblem();
			set<int>::iterator tbItr = tbSet.begin();
			const set<int>::iterator tbEnd = tbSet.end();
	
			for (; tbItr != tbEnd; ++tbItr) {
				map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator fOp = fLit->second.count.find(*tbItr);
				if (fOp != fLit->second.count.end()) {;
					fOp->second.erase(nodeToBlame->getPartner());
					if (fOp->second.empty()) {
						fLit->second.count.erase(fOp);
					}
				}
			}
			if (fLit->second.count.empty()) activeInvariants.erase(fLit);

		}

	}

};


void ScheduleEvaluatorStandard::penalisePreconditions(map<Literal*, NodeOpCount, LitLT> & activeDeletes, ScheduleNode* toBlame, list<Literal*> & precs, double & penalty, double & committedPenalty, ScheduleNode* onlyPenalise, const bool & autoExempt, set<ScheduleNode*, ScheduleNodeLT> & exempt, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, list<ScheduleNode*> & pointsOfConflict, set<ScheduleNode*, ScheduleNodeLT> & pocSet, int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const bool & approximatePenalties, bool * anyPenalties, bool debug) {

	list<Literal*>::iterator pItr = precs.begin();
	const list<Literal*>::iterator pEnd = precs.end();

	for (; pItr != pEnd; ++pItr) {
		Literal * const currLit = *pItr;
		map<Literal*, NodeOpCount, LitLT>::iterator adItr = activeDeletes.find(currLit);
		if (adItr != activeDeletes.end()) {
			
			if (debug) cout << *currLit << " - been deleted\n";
			map<int, set<ScheduleNode*, ScheduleNodeLT> > & currSet = adItr->second.count;
			map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csItr = currSet.begin();
			const map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csEnd = currSet.end();

			for (; csItr != csEnd; ++csItr) {
				if (csItr->first == -1 || !approximatePenalties || csItr->first != spID) {
					set<ScheduleNode*, ScheduleNodeLT>::iterator snItr = csItr->second.begin();
					const set<ScheduleNode*, ScheduleNodeLT>::iterator snEnd = csItr->second.end();

					for (; snItr != snEnd; ++snItr) {

						if (alreadyPenalised.find(*snItr) == alreadyPenalised.end()) {
							//const int ocCount = currOC.count.size();
							//penalty += ocCount * getWeight(checkOp, op);
							
							
							alreadyPenalised.insert(*snItr);
							set<int> & tbSet = toBlame->getSubProblem();
							set<int>::iterator tbsnItr = tbSet.begin();
							const set<int>::iterator tbsnEnd = tbSet.end();
	
							if (debug) cout << "Recording conflict between nodes\n";

							for (; tbsnItr != tbsnEnd; ++tbsnItr) {

								const double extraWeight = (csItr->first == -1 ? 10000.0 : getWeight(*tbsnItr, csItr->first));
								
								if (newPenaltiesOnly) {
									if (*tbsnItr == spID && csItr->first != spID) {
										if (!onlyPenalise || onlyPenalise == toBlame) {
											if (anyPenalties) *anyPenalties = true;
											if (autoExempt) {
												committedPenalty += extraWeight;
											} else {
												penalty += extraWeight;
											}
											if (pocSet.find(*snItr) == pocSet.end()) {
												if (debug) cout << "Recording conflict with node - this SP, uses something deleted by another SP " << *snItr << "\n";
												pocSet.insert(*snItr);
												pointsOfConflict.push_back(*snItr);
												lastConflict = *snItr;
												//assert(GlobalSchedule::sanityCheckDebug.find(*snItr) != GlobalSchedule::sanityCheckDebug.end());
											}
										}
									} else if (*tbsnItr != spID && csItr->first == spID) {
	
										if (!onlyPenalise || onlyPenalise == *snItr) {
											if (anyPenalties) *anyPenalties = true;
											if (autoExempt) {
												committedPenalty += extraWeight;
											} else if (exempt.find(*snItr) == exempt.end()) {
												penalty += extraWeight;
											} else {
												committedPenalty += extraWeight;
											}
											if (debug) cout << "Recording conflict with node - this SP deletes something another appears to need " << toBlame << "\n";
											if (pocSet.find(*snItr) == pocSet.end()) {
												pocSet.insert(toBlame);
												pointsOfConflict.push_back(toBlame);
												lastConflict = toBlame;
												//assert(GlobalSchedule::sanityCheckDebug.find(toBlame) != GlobalSchedule::sanityCheckDebug.end());
											}
										}
									} else {
										if (debug) cout << "Penalty isn't new, not considering\n";
									}
								} else {
									if (debug) cout << "Storing penalty - regardless of whether it is new\n";
									if (anyPenalties) *anyPenalties = true;
									penalty += extraWeight;
									if (pocSet.find(*snItr) == pocSet.end()) {
										pocSet.insert(*snItr);
										pointsOfConflict.push_back(*snItr);
										lastConflict = *snItr;
										//assert(GlobalSchedule::sanityCheckDebug.find(*snItr) != GlobalSchedule::sanityCheckDebug.end());
									}
								}

								

							}
		
						} else {
							if (debug) cout << *currLit << " - deleted  but mutex between two nodes already penalised\n";
						}
					}
				}
			}
		} else if (debug) {
			cout << *currLit << " - never deleted\n";
		}
	}

};

void ScheduleEvaluatorStandard::penaliseNumericPreconditions(list<pair<ScheduleNode*, vector<double> > > & fluentTracking, ScheduleNode* toBlame, list<RPGBuilder::NumericPrecondition> & precs, double & penalty, double & committedPenalty, ScheduleNode* onlyPenalise, const bool & autoExempt, set<ScheduleNode*, ScheduleNodeLT> & exempt, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, list<ScheduleNode*> & pointsOfConflict, set<ScheduleNode*, ScheduleNodeLT> & pocSet, int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const bool & approximatePenalties, bool * anyPenalties, bool debug) {

	list<RPGBuilder::NumericPrecondition>::iterator pItr = precs.begin();
	const list<RPGBuilder::NumericPrecondition>::iterator pEnd = precs.end();

	if (debug && pItr == pEnd) cout << "No numeric preconditions\n";

	for (; pItr != pEnd; ++pItr) {
		RPGBuilder::NumericPrecondition & currLit = *pItr;
		if (!currLit.isSatisfied(fluentTracking.back().second)) {

			if (debug) {
				currLit.display(cout);
				cout << " - not satisfied\n";
			}
			
			list<pair<ScheduleNode*, vector<double> > >::reverse_iterator ftItr = fluentTracking.rbegin();
			const list<pair<ScheduleNode*, vector<double> > >::reverse_iterator ftEnd = fluentTracking.rend();

			list<pair<ScheduleNode*, vector<double> > >::reverse_iterator oneBefore = ftItr;
			++ftItr;

			for (; ftItr != ftEnd; ++ftItr) {
				if (currLit.isSatisfied(ftItr->second)) {
					break;
				} else {
					oneBefore = ftItr;
				}
			}
			
			//assert(oneBefore->first); // not true in initial state => invalid subplan even in isolatuion

			if (oneBefore->first) {
				
				if (!approximatePenalties || (!oneBefore->first->forSubProblem(spID) || oneBefore->first->getSubProblem().size() > 1))  {
					
					ScheduleNode* const snItr = oneBefore->first;
	
					if (alreadyPenalised.find(snItr) == alreadyPenalised.end()) {
						//const int ocCount = currOC.count.size();
						//penalty += ocCount * getWeight(checkOp, op);
						
						
						alreadyPenalised.insert(snItr);
	
						set<int> & csSet = snItr->getSubProblem();
						set<int>::iterator csItr = csSet.begin();
						const set<int>::iterator csEnd = csSet.end();
	
						set<int> & tbSet = toBlame->getSubProblem();
						const set<int>::iterator tbsnEnd = tbSet.end();
	
						for (; csItr != csEnd; ++csItr) {
							set<int>::iterator tbsnItr = tbSet.begin();
							
		
							if (debug) cout << "Recording conflict between nodes\n";
		
							for (; tbsnItr != tbsnEnd; ++tbsnItr) {
		
								const double extraWeight = getWeight(*tbsnItr, *csItr);
								
								if (newPenaltiesOnly) {
									if (*tbsnItr == spID && *csItr != spID) {
										if (!onlyPenalise || onlyPenalise == toBlame) {
											if (anyPenalties) *anyPenalties = true;
											if (autoExempt) {
												committedPenalty += extraWeight;
											} else {
												penalty += extraWeight;
											}
											if (pocSet.find(snItr) == pocSet.end()) {
												if (debug) cout << "Recording conflict with node - this SP, uses something deleted by another SP " << *csItr << "\n";
												pocSet.insert(snItr);
												pointsOfConflict.push_back(snItr);
												lastConflict = snItr;
												//assert(GlobalSchedule::sanityCheckDebug.find(*snItr) != GlobalSchedule::sanityCheckDebug.end());
											}
										}
									} else if (*tbsnItr != spID && *csItr == spID) {
		
										if (!onlyPenalise || onlyPenalise == snItr) {
											if (anyPenalties) *anyPenalties = true;
											if (autoExempt) {
												committedPenalty += extraWeight;
											} else if (exempt.find(snItr) == exempt.end()) {
												penalty += extraWeight;
											} else {
												committedPenalty += extraWeight;
											}
											if (debug) cout << "Recording conflict with node - this SP deletes something another appears to need " << toBlame << "\n";
											if (pocSet.find(snItr) == pocSet.end()) {
												pocSet.insert(toBlame);
												pointsOfConflict.push_back(toBlame);
												lastConflict = toBlame;
												//assert(GlobalSchedule::sanityCheckDebug.find(toBlame) != GlobalSchedule::sanityCheckDebug.end());
											}
										}
									} else {
										if (debug) cout << "Penalty isn't new, not considering\n";
									}
								} else {
									if (debug) cout << "Storing penalty - regardless of whether it is new\n";
									if (anyPenalties) *anyPenalties = true;
									penalty += extraWeight;
									if (pocSet.find(snItr) == pocSet.end()) {
										pocSet.insert(snItr);
										pointsOfConflict.push_back(snItr);
										lastConflict = snItr;
										//assert(GlobalSchedule::sanityCheckDebug.find(*snItr) != GlobalSchedule::sanityCheckDebug.end());
									}
								}
		
								
		
							}
						}
	
					} else {
						if (debug) {
							currLit.display(cout);
							cout << " - invalidated but mutex between two nodes already penalised\n";
						}
					}
					
					
				}
			} else {
				if (debug) {
					currLit.display(cout);
					cout << " - must be from the RPG as it's never true ever\n";
				}
			} 

		} else if (debug) {
			currLit.display(cout);
			cout << " - is satisfied\n";
		}
	}

};


void ScheduleEvaluatorStandard::penaliseGP(list<pair<ScheduleNode*, vector<double> > > & fluentTracking, map<Literal*, NodeOpCount, LitLT> & activeDeletes, double & penalty, ScheduleNode* onlyPenalise, list<ScheduleNode*> & pointsOfConflict, set<ScheduleNode*, ScheduleNodeLT> & pocSet, int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const bool & approximatePenalties, bool * anyPenalties, bool debug) {

	static bool doneInit = false;
	static vector<pair<int, Literal*> > topLevelGoals;
	static vector<pair<int, RPGBuilder::NumericPrecondition*> > topLevelFluentGoals;
	static int goalCount;
	static int fluentGoalCount;
	if (!doneInit) {
		doneInit = true;
		goalCount = 0;
		fluentGoalCount = 0;
		const int howMany = Decomposition::howMany();
		{
			for (int i = 0; i < howMany; ++i) {
				if (Decomposition::getSubproblem(i)->goals.empty()) {
					++fluentGoalCount;
					//cout << "Subproblem " << i << " is fluent\n";
				} else {
					//cout << "Subproblem " << i << " is propositional\n";
					++goalCount;
				}
			}
		}
		
		list<RPGBuilder::NumericPrecondition>::iterator gfItr = RPGBuilder::getNumericGoals().begin();
		
		topLevelGoals = vector<pair<int, Literal*> >(goalCount);
		topLevelFluentGoals = vector<pair<int, RPGBuilder::NumericPrecondition*> >(fluentGoalCount);
		
		//cout << "Have " << goalCount << " prop goals and " << fluentGoalCount << "fluent goals\n";
		
		int pg = 0;
		int fg = 0;
		
		for (int i = 0; i < fluentGoalCount; ++i) topLevelFluentGoals[i] = pair<int, RPGBuilder::NumericPrecondition*>(-1,0);
		
		
		
		for (int i = 0; i < howMany; ++i) {
			if (Decomposition::getSubproblem(i)->goals.empty()) {
				//cout << "fg " << fg << " = " << *gfItr << "\n";
				topLevelFluentGoals[fg] = pair<int, RPGBuilder::NumericPrecondition*>(i, &(*gfItr));
				++gfItr;
				++fg;
			} else {
				topLevelGoals[pg++] = pair<int, Literal*>(i, Decomposition::getSubproblem(i)->goals.front());
			}
		}
	}
	
	
	for (int pg = 0; pg < goalCount; ++pg) {
		int currSP = topLevelGoals[pg].first;
		Literal * const currLit = topLevelGoals[pg].second;
		map<Literal*, NodeOpCount, LitLT>::iterator adItr = activeDeletes.find(currLit);
		if (adItr != activeDeletes.end()) {
			map<int, set<ScheduleNode*, ScheduleNodeLT> > & currSet = adItr->second.count;
			map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csItr = currSet.begin();
			const map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csEnd = currSet.end();

			for (; csItr != csEnd; ++csItr) {
				if (!approximatePenalties || csItr->first != spID) {
					set<ScheduleNode*, ScheduleNodeLT>::iterator snItr = csItr->second.begin();
					const set<ScheduleNode*, ScheduleNodeLT>::iterator snEnd = csItr->second.end();

					for (; snItr != snEnd; ++snItr) {

						int * tbsnItr = &currSP;
						
						{

							const double extraWeight = getWeight(*tbsnItr, csItr->first);
							
							if (newPenaltiesOnly) {
								if (*tbsnItr == spID && csItr->first != spID) {
									if (!onlyPenalise || onlyPenalise == *snItr) {
										if (anyPenalties) *anyPenalties = true;
										penalty += extraWeight;
										if (pocSet.find(*snItr) == pocSet.end()) {
											if (debug) cout << "Recording conflict with node - this SP uses something deleted by another SP\n";
											pocSet.insert(*snItr);
											pointsOfConflict.push_back(*snItr);
											lastConflict = *snItr;
											//assert(GlobalSchedule::sanityCheckDebug.find(*snItr) != GlobalSchedule::sanityCheckDebug.end());
										}
									}
								} else if (*tbsnItr != spID && csItr->first == spID) {
									if (!onlyPenalise || onlyPenalise == *snItr) {
										if (anyPenalties) *anyPenalties = true;
										penalty += extraWeight;
										if (debug) cout << "Recording conflict with node - this SP deletes something another appears to need\n";
									}
									
								}
							} else {
								if (anyPenalties) *anyPenalties = true;
								penalty += extraWeight;
								if (pocSet.find(*snItr) == pocSet.end()) {
									pocSet.insert(*snItr);
									pointsOfConflict.push_back(*snItr);
									lastConflict = *snItr;
									//assert(GlobalSchedule::sanityCheckDebug.find(*snItr) != GlobalSchedule::sanityCheckDebug.end());
								}
							}

							

						}
		
					}
				}
			}
		} else if (debug) {
			cout << *currLit << " - never deleted\n";
		}
	}
	
	/*vector<double> debugVec(fluentTracking.back().second);
	const int loopLim = debugVec.size();
	for (int d = 0; d < loopLim; ++d) {
		cout << "\t" << debugVec[d] << "\n";
	}*/
	
	for (int fg = 0; fg < fluentGoalCount; ++fg) {
		//cout << "Fluent goal " << fg << ", < " << fluentGoalCount << "\n";
		int currSP = topLevelFluentGoals[fg].first;
		RPGBuilder::NumericPrecondition & currLit = *(topLevelFluentGoals[fg].second);
		
		//cout << currLit << "\n";
		
		if (!currLit.isSatisfied(fluentTracking.back().second)) {

			if (debug) {
				currLit.display(cout);
				cout << " - not satisfied\n";
			}
			
			list<pair<ScheduleNode*, vector<double> > >::reverse_iterator ftItr = fluentTracking.rbegin();
			const list<pair<ScheduleNode*, vector<double> > >::reverse_iterator ftEnd = fluentTracking.rend();

			list<pair<ScheduleNode*, vector<double> > >::reverse_iterator oneBefore = ftItr;
			++ftItr;

			for (; ftItr != ftEnd; ++ftItr) {
				if (currLit.isSatisfied(ftItr->second)) {
					break;
				} else {
					oneBefore = ftItr;
				}
			}
			
			//assert(oneBefore->first); // not true in initial state => invalid subplan even in isolatuion

			if (oneBefore->first) {
				
				if (!approximatePenalties || (!oneBefore->first->forSubProblem(spID) || oneBefore->first->getSubProblem().size() > 1))  {
					
					ScheduleNode* const snItr = oneBefore->first;
	
					
					set<int> & csSet = snItr->getSubProblem();
					set<int>::iterator csItr = csSet.begin();
					const set<int>::iterator csEnd = csSet.end();

					

					for (; csItr != csEnd; ++csItr) {
						
						int * tbsnItr = &currSP;
	
						if (debug) cout << "Recording conflict between nodes\n";
	
						{
	
							const double extraWeight = getWeight(*tbsnItr, *csItr);
							
							if (newPenaltiesOnly) {
								if (*tbsnItr == spID && *csItr != spID) {
									if (!onlyPenalise || onlyPenalise == snItr) {
										if (anyPenalties) *anyPenalties = true;
										penalty += extraWeight;
										
										if (pocSet.find(snItr) == pocSet.end()) {
											if (debug) cout << "Recording conflict with node - this SP, uses something deleted by another SP " << *csItr << "\n";
											pocSet.insert(snItr);
											pointsOfConflict.push_back(snItr);
											lastConflict = snItr;
											//assert(GlobalSchedule::sanityCheckDebug.find(*snItr) != GlobalSchedule::sanityCheckDebug.end());
										}
									}
								} else if (*tbsnItr != spID && *csItr == spID) {
	
									if (!onlyPenalise || onlyPenalise == snItr) {
										if (anyPenalties) *anyPenalties = true;
										penalty += extraWeight;
										
										if (debug) cout << "Recording conflict with node - this SP deletes something another appears to need " << snItr << "\n";
										
									}
								} else {
									if (debug) cout << "Penalty isn't new, not considering\n";
								}
							} else {
								if (debug) cout << "Storing penalty - regardless of whether it is new\n";
								if (anyPenalties) *anyPenalties = true;
								penalty += extraWeight;
								if (pocSet.find(snItr) == pocSet.end()) {
									pocSet.insert(snItr);
									pointsOfConflict.push_back(snItr);
									lastConflict = snItr;
									//assert(GlobalSchedule::sanityCheckDebug.find(*snItr) != GlobalSchedule::sanityCheckDebug.end());
								}
							}
	
						}	
	
						
					}
	
					
					
					
				}
			} else {
				if (debug) {
					currLit.display(cout);
					cout << " - must be from the RPG as it's never true ever\n";
				}
			} 

		} else if (debug) {
			currLit.display(cout);
			cout << " - is satisfied\n";
		}
	}

};


void ScheduleEvaluatorStandard::penaliseDeleteEffects(map<Literal*, NodeOpCount, LitLT> & activeInvariants, ScheduleNode* toBlame, list<Literal*> & deletes, double & penalty, double & committedPenalty, ScheduleNode* onlyPenalise, const bool & autoExempt, set<ScheduleNode*, ScheduleNodeLT> & exempt, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, list<ScheduleNode*> & pointsOfConflict, set<ScheduleNode*, ScheduleNodeLT> & pocSet, int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const bool & approximatePenalties, bool * anyPenalties, bool debug) {

	list<Literal*>::iterator dItr = deletes.begin();
	const list<Literal*>::iterator dEnd = deletes.end();

	for (; dItr != dEnd; ++dItr) {
		Literal * const currLit = *dItr;
		map<Literal*, NodeOpCount, LitLT>::iterator aiItr = activeInvariants.find(currLit);
		if (aiItr != activeInvariants.end()) {
			map<int, set<ScheduleNode*, ScheduleNodeLT> > & currSet = aiItr->second.count;
			map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csItr = currSet.begin();
			const map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csEnd = currSet.end();

			for (; csItr != csEnd; ++csItr) {
				set<ScheduleNode*, ScheduleNodeLT>::iterator snItr = csItr->second.begin();
				const set<ScheduleNode*, ScheduleNodeLT>::iterator snEnd = csItr->second.end();

				for (; snItr != snEnd; ++snItr) {

				
					if (alreadyPenalised.find(*snItr) == alreadyPenalised.end()) {
						//const int ocCount = currOC.count.size();
						//penalty += ocCount * getWeight(checkOp, op);
						
						alreadyPenalised.insert(*snItr);
						
						if (toBlame->tilNode()) {
						
							const double extraWeight = 10000.0; // nominally high value to really discourage violating TILs
							if (newPenaltiesOnly) {
								if (csItr->first == spID) {
									if (!onlyPenalise || onlyPenalise == *snItr) {
										if (anyPenalties) *anyPenalties = true;

										if (autoExempt) {
											committedPenalty += extraWeight;
										} else if (exempt.find(*snItr) == exempt.end()) {
											penalty += extraWeight;
										} else {
											committedPenalty += extraWeight;
										}

										//penalty += extraWeight;
										if (debug) cout << "Recording conflict with node " << toBlame << " - TIL deletes an invariant of this one\n";
										if (pocSet.find(toBlame) == pocSet.end()) {
											pocSet.insert(toBlame);
											pointsOfConflict.push_back(toBlame);
											lastConflict = toBlame;
											//assert(GlobalSchedule::sanityCheckDebug.find(toBlame) != GlobalSchedule::sanityCheckDebug.end());
										}
									}
								}
							} else {
								if (anyPenalties) *anyPenalties = true;
								penalty += extraWeight;
								if (pocSet.find(*snItr) == pocSet.end()) {
									pocSet.insert(*snItr);
									pointsOfConflict.push_back(*snItr);
									lastConflict = *snItr;
								}
							}
						
						} else {
							set<int> & tbSet = toBlame->getSubProblem();
							set<int>::iterator tbsnItr = tbSet.begin();
							const set<int>::iterator tbsnEnd = tbSet.end();
							
							for (; tbsnItr != tbsnEnd; ++tbsnItr) {
								if (!approximatePenalties || *tbsnItr != spID) {
									const double extraWeight = getWeight(*tbsnItr, csItr->first);
									
									if (newPenaltiesOnly) {
										if (*tbsnItr == spID && csItr->first != spID) {
											if (!onlyPenalise || onlyPenalise == toBlame) {
												if (anyPenalties) *anyPenalties = true;
												if (autoExempt) {
													committedPenalty += extraWeight;
												} else {
													penalty += extraWeight;
												}
	
												//penalty += extraWeight;
	
	
												ScheduleNode* const getPartner = (*snItr)->getPartner();
												if (getPartner) {
													if (pocSet.find(getPartner) == pocSet.end()) {
														if (debug) cout << "Recording conflict with node " << getPartner << " - this deletes an invariant of another SP\n";
														pocSet.insert(getPartner);
														pointsOfConflict.push_back(getPartner);
														lastConflict = getPartner;
														//assert(GlobalSchedule::sanityCheckDebug.find(getPartner) != GlobalSchedule::sanityCheckDebug.end());
													}
												}
											}
										} else if (*tbsnItr != spID && csItr->first == spID) {
											if (!onlyPenalise || onlyPenalise == *snItr) {
												if (anyPenalties) *anyPenalties = true;
	
												if (autoExempt) {
													committedPenalty += extraWeight;
												} else if (exempt.find(*snItr) == exempt.end()) {
													penalty += extraWeight;
												} else {
													committedPenalty += extraWeight;
												}
	
												//penalty += extraWeight;
												if (debug) cout << "Recording conflict with node " << toBlame << " - another SP deletes an invariant of this one\n";
												if (pocSet.find(toBlame) == pocSet.end()) {
													pocSet.insert(toBlame);
													pointsOfConflict.push_back(toBlame);
													lastConflict = toBlame;
													//assert(GlobalSchedule::sanityCheckDebug.find(toBlame) != GlobalSchedule::sanityCheckDebug.end());
												}
											}
										}
									} else {
										if (anyPenalties) *anyPenalties = true;
										penalty += extraWeight;
										if (pocSet.find(*snItr) == pocSet.end()) {
											pocSet.insert(*snItr);
											pointsOfConflict.push_back(*snItr);
											lastConflict = *snItr;
										}
									}
								}
								
	
							}
						}
					}
				}
			}
		}
	}


}

void ScheduleEvaluatorStandard::penaliseNumericEffects(map<int, NodeOpCount> & activeFluentInvariants, ScheduleNode* toBlame, list<RPGBuilder::NumericEffect> & numEffs, double & penalty, double & committedPenalty, ScheduleNode* onlyPenalise, const bool & autoExempt, set<ScheduleNode*, ScheduleNodeLT> & exempt, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, list<ScheduleNode*> & pointsOfConflict, set<ScheduleNode*, ScheduleNodeLT> & pocSet, int & spID, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const bool & approximatePenalties, bool * anyPenalties, bool debug) {

	list<RPGBuilder::NumericEffect>::iterator dItr = numEffs.begin();
	const list<RPGBuilder::NumericEffect>::iterator dEnd = numEffs.end();

	for (; dItr != dEnd; ++dItr) {
		const int currLit = dItr->fluentIndex;
		map<int, NodeOpCount>::iterator aiItr = activeFluentInvariants.find(currLit);
		if (aiItr != activeFluentInvariants.end()) {
			map<int, set<ScheduleNode*, ScheduleNodeLT> > & currSet = aiItr->second.count;
			map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csItr = currSet.begin();
			const map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csEnd = currSet.end();

			for (; csItr != csEnd; ++csItr) {
				set<ScheduleNode*, ScheduleNodeLT>::iterator snItr = csItr->second.begin();
				const set<ScheduleNode*, ScheduleNodeLT>::iterator snEnd = csItr->second.end();

				for (; snItr != snEnd; ++snItr) {

				
					if (alreadyPenalised.find(*snItr) == alreadyPenalised.end()) {
						//const int ocCount = currOC.count.size();
						//penalty += ocCount * getWeight(checkOp, op);
						
						alreadyPenalised.insert(*snItr);
						set<int> & tbSet = toBlame->getSubProblem();
						set<int>::iterator tbsnItr = tbSet.begin();
						const set<int>::iterator tbsnEnd = tbSet.end();
						
						for (; tbsnItr != tbsnEnd; ++tbsnItr) {
							if (!approximatePenalties || *tbsnItr != spID) {
								const double extraWeight = getWeight(*tbsnItr, csItr->first);
								
								if (newPenaltiesOnly) {
									if (*tbsnItr == spID && csItr->first != spID) {
										if (!onlyPenalise || onlyPenalise == toBlame) {
											if (anyPenalties) *anyPenalties = true;
											if (autoExempt) {
												committedPenalty += extraWeight;
											} else {
												penalty += extraWeight;
											}

											//penalty += extraWeight;


											ScheduleNode* const getPartner = (*snItr)->getPartner();
											if (getPartner) {
												if (pocSet.find(getPartner) == pocSet.end()) {
													if (debug) cout << "Recording conflict with node " << getPartner << " - this deletes a numeric invariant of another SP\n";
													pocSet.insert(getPartner);
													pointsOfConflict.push_back(getPartner);
													lastConflict = getPartner;
													//assert(GlobalSchedule::sanityCheckDebug.find(getPartner) != GlobalSchedule::sanityCheckDebug.end());
												}
											}
										}
									} else if (*tbsnItr != spID && csItr->first == spID) {
										if (!onlyPenalise || onlyPenalise == *snItr) {
											if (anyPenalties) *anyPenalties = true;

											if (autoExempt) {
												committedPenalty += extraWeight;
											} else if (exempt.find(*snItr) == exempt.end()) {
												penalty += extraWeight;
											} else {
												committedPenalty += extraWeight;
											}

											//penalty += extraWeight;
											if (debug) cout << "Recording conflict with node " << toBlame << " - another SP deletes a numeric invariant of this one\n";
											if (pocSet.find(toBlame) == pocSet.end()) {
												pocSet.insert(toBlame);
												pointsOfConflict.push_back(toBlame);
												lastConflict = toBlame;
												//assert(GlobalSchedule::sanityCheckDebug.find(toBlame) != GlobalSchedule::sanityCheckDebug.end());
											}
										}
									}
								} else {
									if (anyPenalties) *anyPenalties = true;
									penalty += extraWeight;
									if (pocSet.find(*snItr) == pocSet.end()) {
										pocSet.insert(*snItr);
										pointsOfConflict.push_back(*snItr);
										lastConflict = *snItr;
									}
								}
							}
							

						}
					}
				}
			}
		}
	}


}

void ScheduleEvaluatorStandard::prepareForExpansion(	list<TimelinePoint*>::iterator & t,
							CandidateSchedule & c,
							list<FFEvent> & header,
							const int & spID) {

//	static int counter = 0;

	static list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > > dummyRP;
	static map<int, list<list<StartEvent>::iterator > > dummyCE;
	static list<ScheduleNode*> * impliedWaits = 0;
	static const int exclude = 0;
	

	delete newSchedule;
	delete waitSchedule;

	newSchedule = 0;
	waitSchedule = 0;

	nsBoundaryNode = 0;
	wsBoundaryNode = 0;

	
	globalNSNewToOld.clear();
	globalWSNewToOld.clear();
	globalOpenStarts.clear();

	map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> & newToOld = globalNSNewToOld;

	map<int, map<int, ScheduleNode*> > * rememberOS = &globalOpenStarts;
	
	newSchedule = c.getScheduleWithActions(t, header,        dummyRP,     dummyCE,        rememberOS, spID, false, &newToOld, impliedWaits, exclude, &nsBoundaryNode, true);

	assert(newSchedule);
	
	//cout << " parent ";
	//cout.flush();


};

double ScheduleEvaluatorStandard::heuristicPenalties(	list<FFEvent> & tmpEvents,
							list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >  & relaxedPlan,
							map<int, list<list<StartEvent>::iterator > > & compulsaryEnds, 
							bool & cycle, const int & spID,
							list<ScheduleNode*> & pointsOfConflict,
							const bool & newPenaltiesOnly,
							ScheduleNode* & lastConflict,
							list<ScheduleNode*> & impliedWaits,
							double & committedPenalties,
							const bool & onlyPenaliseLast, double* localPenalty) {

	//cout << " child ";
	//cout.flush();
	list<FFEvent> ourList(tmpEvents);
	
	int exclude = newSchedule->m_cl + ourList.size();
	
	{
		list<FFEvent>::iterator olItr = ourList.begin();
		const list<FFEvent>::iterator olEnd = ourList.end();
		
		for (; olItr != olEnd; ++olItr) {
			if (olItr->wait) olItr->wait = newSchedule->oldToNew[olItr->wait];
		}
	}
	

	delete waitSchedule;
	waitSchedule = 0;
	wsBoundaryNode = 0;
	globalWSNewToOld.clear();
	map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> & newToOld = globalWSNewToOld;
	
	waitSchedule = new CandidateSchedule(*newSchedule, newToOld, true);
	
	map<int, map<int, ScheduleNode*> > rememberOS = globalOpenStarts;
	
	{
		map<int, map<int, ScheduleNode*> >::iterator roItr = rememberOS.begin();
		const map<int, map<int, ScheduleNode*> >::iterator roEnd = rememberOS.end();
		
		for (; roItr != roEnd; ++roItr) {
			map<int, ScheduleNode*>::iterator rlItr = roItr->second.begin();
			const map<int, ScheduleNode*>::iterator rlEnd = roItr->second.end();
			
			for (; rlItr != rlEnd; ++rlItr) {
				//cout << "Changing open start from " << rlItr->second << " to ";
				rlItr->second = waitSchedule->oldToNew[rlItr->second];
				//cout << rlItr->second << "\n";
			}
		}
	}
	
	if (nsBoundaryNode) wsBoundaryNode = waitSchedule->oldToNew[nsBoundaryNode];
	
	/*{
		list<FFEvent>::iterator olItr = ourList.begin();
		const list<FFEvent>::iterator olEnd = ourList.end();
		
		for (; olItr != olEnd; ++olItr) {
			if (olItr->wait) olItr->wait = waitSchedule->oldToNew[olItr->wait];
		}
	}*/
	
	if (!waitSchedule->postNewActions(ourList, relaxedPlan, compulsaryEnds, &rememberOS, newSchedule->oldToNew, spID, false, &newToOld, &wsBoundaryNode)) {
		cycle = true;
		return -1.0;
	};


	ScheduleNode* const onlyPenalise = (onlyPenaliseLast ? wsBoundaryNode : 0);

//	GlobalSchedule::sanityCheckDebug = newToOld;


	double toReturn = 0.0;
	
	
	if (!FF::ignorePenalties) {
		bool tmp = false;
		toReturn = evaluateSchedule(*waitSchedule, &tmp, false, pointsOfConflict, spID, lastConflict, newPenaltiesOnly, exclude, committedPenalties, onlyPenalise);
	
			

		if (GlobalSchedule::penaliseHead) toReturn += committedPenalties;
	
		//toReturn = committedPenalties;
	
		//delete newSchedule;
	
		list<ScheduleNode*>::iterator itr = pointsOfConflict.begin();
		const list<ScheduleNode*>::iterator itrEnd = pointsOfConflict.end();
	
		for (; itr != itrEnd; ++itr) {
			
			{
				map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator mpItr = globalWSNewToOld.find(*itr);
				assert(mpItr != globalWSNewToOld.end());
				(*itr) = mpItr->second;
			}
	
			{
				map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator mpItr = globalNSNewToOld.find(*itr);
				assert(mpItr != globalNSNewToOld.end());
				(*itr) = mpItr->second;
			}
	
		}
		
	
		if (lastConflict) {
			{
				map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator mpItr = globalWSNewToOld.find(lastConflict);
				assert(mpItr != globalWSNewToOld.end());
				lastConflict = mpItr->second;
			}
	
			{
				map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator mpItr = globalNSNewToOld.find(lastConflict);
				assert(mpItr != globalNSNewToOld.end());
				lastConflict = mpItr->second;
			}
		}

		if (localPenalty) {
			ScheduleNode* tlastConflict = 0;
			double tcommittedPenalties = 0.0;
			list<ScheduleNode*> tpointsOfConflict;

			*localPenalty = evaluateSchedule(*waitSchedule, &tmp, false, tpointsOfConflict, spID, tlastConflict, newPenaltiesOnly, exclude, tcommittedPenalties, wsBoundaryNode);
		}
	}
/*
	if (pointsOfConflict.empty()) {
		if (!c.events.empty()) {
			pointsOfConflict.push_back(c.events.front());
			pointsOfConflict.push_back(c.events.back());
		}
	} else {
		if (!c.events.empty()) {
			if (pointsOfConflict.front() != c.events.front()) pointsOfConflict.push_front(c.events.front());
			if (pointsOfConflict.back() != c.events.back()) pointsOfConflict.push_back(c.events.back());
		}
	}
*/

	
	
	return toReturn;
	
	

};

double ScheduleEvaluatorStandard::heuristicPenalties(	list<TimelinePoint*>::iterator & t,
							CandidateSchedule & c,
							list<FFEvent> & header, list<FFEvent> & nowList,
							list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >  & relaxedPlan,
							map<int, list<list<StartEvent>::iterator > > & compulsaryEnds, 
							bool & cycle, const int & spID,
							list<ScheduleNode*> & pointsOfConflict,
							const bool & newPenaltiesOnly,
							ScheduleNode* & lastConflict,
							list<ScheduleNode*> & impliedWaits,
							double & committedPenalties,
							const bool & onlyPenaliseLast, double* localPenalty) {

	static int counter = 0;

	
	list<FFEvent> makeshiftPlan(header);
	{
		list<FFEvent>::iterator nItr = nowList.begin();
		const list<FFEvent>::iterator nEnd = nowList.end();
		
		for (; nItr != nEnd; ++nItr) {
			FFEvent & now = *nItr;
			if ((now.wait && now.getEffects) || now.action) makeshiftPlan.push_back(now);
		}
		
	}
	if (false) {
		cout << "HP for plan:\n";

		list<FFEvent>::iterator planItr = makeshiftPlan.begin();
		const list<FFEvent>::iterator planEnd = makeshiftPlan.end();
	
		int i;

		for (i = 0; planItr != planEnd; ++planItr, ++i) {
			if (planItr->action) {
				cout << i << ": " << *(planItr->action) << ", " << (planItr->time_spec == VAL::E_AT_START ? "start" : "end") << "\n";
			} else if (planItr->wait && planItr->wait->tilNode()) {
				cout << i << ": TIL\n";
			} else if (planItr->wait) {
				cout << i << ": wait for " << *(planItr->wait->getOp()) << " " << (planItr->wait->isStart() ? "start" : "end");
				if (planItr->getEffects) cout << ", getting its effects";
				cout << "\n";
			} else {
				cout << i << ": null node!\n";
				assert(false);
			}
		}
		cout << "------------------------------------------------------\n";
		list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >::iterator rpItr = relaxedPlan.begin();
		const list<pair<double, list<pair<instantiatedOp*, VAL::time_spec> > > >::iterator rpEnd = relaxedPlan.end();

		for (; rpItr != rpEnd; ++rpItr) {
			list<pair<instantiatedOp*, VAL::time_spec> >::iterator rlItr = rpItr->second.begin();
			const list<pair<instantiatedOp*, VAL::time_spec> >::iterator rlEnd = rpItr->second.end();

			for (; rlItr != rlEnd; ++rlItr, ++i) {
			      cout << i << ":" << *(rlItr->first) << " " << (rlItr->second == VAL::E_AT_START ? "start" : "end") << "\n";
			}
		}

	}
	//const int exclude = (GlobalSchedule::penaliseHead ? 0 : makeshiftPlan.size());
	const int exclude = makeshiftPlan.size();
	//if (now.wait) cout << "** waiting for " << *(now.wait->getOp()) << " **\n";
	if (GlobalSchedule::globalVerbosity & 2) {
		cout << " ( " << (++counter) << " ) ";
		cout.flush();
	}

	/*{
		list<instantiatedOp*>::iterator rpItr = relaxedPlan.begin();
		const list<instantiatedOp*>::iterator rpEnd = relaxedPlan.end();

		for (; rpItr != rpEnd; ++rpItr) {
			makeshiftPlan.push_back(FFEvent(*rpItr));
		}
	}*/

/*	if (false && counter > 1460) {
		cout << "\n";
		list<FFEvent>::iterator mpItr = makeshiftPlan.begin();
		const list<FFEvent>::iterator mpEnd = makeshiftPlan.end();

		for (int i = 0; mpItr != mpEnd; ++i, ++mpItr) {
			cout << i << ": ";
			if (mpItr->action) {
				cout << *(mpItr->action);
			} else if (mpItr->wait) {
				cout << "wait for " << *(mpItr->wait->getOp());
			}
			cout << "\n";
		}
	}*/

	
	
	delete newSchedule;
	delete waitSchedule;
	
	newSchedule = 0;
	waitSchedule = 0;
	nsBoundaryNode = 0;
	wsBoundaryNode = 0;

	globalNSNewToOld.clear();
	globalWSNewToOld.clear();

	map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> & newToOld = globalNSNewToOld;

	map<int, map<int, ScheduleNode*> > * rememberOS = 0;
	
	newSchedule = c.getScheduleWithActions(t, makeshiftPlan, relaxedPlan, compulsaryEnds, rememberOS, spID, false, &newToOld, &impliedWaits, exclude, &nsBoundaryNode);
	
	ScheduleNode* const onlyPenalise = (onlyPenaliseLast ? nsBoundaryNode : 0);

//	GlobalSchedule::sanityCheckDebug = newToOld;


	if (!newSchedule) {
		cycle = true;
		return -1.0;
	}

	bool tmp = false;

	lastConflict = 0;

	double toReturn = 0.0;

	if (!FF::ignorePenalties) {
		
	
		toReturn = evaluateSchedule(*newSchedule, &tmp, false, pointsOfConflict, spID, lastConflict, newPenaltiesOnly, exclude, committedPenalties, onlyPenalise);
		
	
		if (GlobalSchedule::penaliseHead) toReturn += committedPenalties;
	
		//toReturn = committedPenalties;
	
		//delete newSchedule;
	
		list<ScheduleNode*>::iterator itr = pointsOfConflict.begin();
		const list<ScheduleNode*>::iterator itrEnd = pointsOfConflict.end();
	
		for (; itr != itrEnd; ++itr) {
			map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator mpItr = newToOld.find(*itr);
			assert(mpItr != newToOld.end());
			(*itr) = mpItr->second;
		}
		
	
		if (lastConflict) {
			map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator mpItr = newToOld.find(lastConflict);
			assert(mpItr != newToOld.end());
			lastConflict = mpItr->second;
	
		}

		if (localPenalty) {
			ScheduleNode* tlastConflict = 0;
			double tcommittedPenalties = 0.0;
			list<ScheduleNode*> tpointsOfConflict;

			*localPenalty = evaluateSchedule(*newSchedule, &tmp, false, tpointsOfConflict, spID, tlastConflict, newPenaltiesOnly, exclude, tcommittedPenalties, nsBoundaryNode);

		}
	}
/*
	if (pointsOfConflict.empty()) {
		if (!c.events.empty()) {
			pointsOfConflict.push_back(c.events.front());
			pointsOfConflict.push_back(c.events.back());
		}
	} else {
		if (!c.events.empty()) {
			if (pointsOfConflict.front() != c.events.front()) pointsOfConflict.push_front(c.events.front());
			if (pointsOfConflict.back() != c.events.back()) pointsOfConflict.push_back(c.events.back());
		}
	}
*/
	return toReturn;

};

double ScheduleEvaluatorStandard::heuristicSchedulePenalties(
							ScheduleNode* const waitFor,
							const bool & comeAfter,
							list<FFEvent> & header, list<FFEvent> & now,
							bool & cycle, const int & spID,
							list<ScheduleNode*> & pointsOfConflict,
							const bool & newPenaltiesOnly,
							ScheduleNode* & lastConflict,
							list<ScheduleNode*> & impliedWaits,
							double & committedPenalties,
							const bool & onlyPenaliseLast, double* localPenalty) {

	

	CandidateSchedule* const schedToUse = (waitSchedule ? waitSchedule : newSchedule);
	
	//const int exclude = (GlobalSchedule::penaliseHead ? 0 : makeshiftPlan.size());
	const int exclude = header.size() + now.size();

	ScheduleNode* transWait = newSchedule->oldToNew[waitFor];
	if (waitSchedule) transWait = waitSchedule->oldToNew[transWait];
	assert(transWait);

	//map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> newToOld;

	//CandidateSchedule toTest(*schedToUse, newToOld, true);
	
	//ScheduleNode* const transWaitNew = toTest.oldToNew[transWait];
	//ScheduleNode* const newBoundaryNode = toTest.oldToNew[(FF::incrementalExpansion ? wsBoundaryNode : nsBoundaryNode)];
	
	
	
	ScheduleNode* const newBoundaryNode = (waitSchedule ? wsBoundaryNode : nsBoundaryNode);
	assert(newBoundaryNode);
	
	if (!schedToUse->postConstraint(transWait, newBoundaryNode, comeAfter)) {
		cycle = true;
		schedToUse->unpostConstraint();
		return -1.0;
	}

	ScheduleNode* const onlyPenalise = (onlyPenaliseLast ? newBoundaryNode : 0);

//	GlobalSchedule::sanityCheckDebug = newToOld;


	if (!newSchedule) {
		cycle = true;
		schedToUse->unpostConstraint();
		return -1.0;
	}

	bool tmp = false;

	lastConflict = 0;

	double toReturn = 0.0;
	
	if (!FF::ignorePenalties) {
		evaluateSchedule(*schedToUse, &tmp, false, pointsOfConflict, spID, lastConflict, newPenaltiesOnly, exclude, committedPenalties, onlyPenalise);
		
	
		if (GlobalSchedule::penaliseHead) toReturn += committedPenalties;
	
		//toReturn = committedPenalties;
	
		//delete newSchedule;
	
		//map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT> & globalNTO = (FF::incrementalExpansion ? globalWSNewToOld : globalNSNewToOld);
		
		list<ScheduleNode*>::iterator itr = pointsOfConflict.begin();
		const list<ScheduleNode*>::iterator itrEnd = pointsOfConflict.end();
	
		for (; itr != itrEnd; ++itr) {
			
			if (waitSchedule) {
				map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator mpItr = globalWSNewToOld.find(*itr);
				assert(mpItr != globalWSNewToOld.end());
				(*itr) = mpItr->second;
			}
	
			{
				map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator mpItr = globalNSNewToOld.find(*itr);
				assert(mpItr != globalNSNewToOld.end());
				(*itr) = mpItr->second;
			}
	
		}
		
	
		if (lastConflict) {
			if (waitSchedule) {
				map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator mpItr = globalWSNewToOld.find(lastConflict);
				assert(mpItr != globalWSNewToOld.end());
				lastConflict = mpItr->second;
			}
	
			{
				map<ScheduleNode*, ScheduleNode*, ScheduleNodeLT>::iterator mpItr = globalNSNewToOld.find(lastConflict);
				assert(mpItr != globalNSNewToOld.end());
				lastConflict = mpItr->second;
			}
		}
	/*
		if (pointsOfConflict.empty()) {
			if (!c.events.empty()) {
				pointsOfConflict.push_back(c.events.front());
				pointsOfConflict.push_back(c.events.back());
			}
		} else {
			if (!c.events.empty()) {
				if (pointsOfConflict.front() != c.events.front()) pointsOfConflict.push_front(c.events.front());
				if (pointsOfConflict.back() != c.events.back()) pointsOfConflict.push_back(c.events.back());
			}
		}
	*/
	
	
		if (localPenalty) {
			ScheduleNode* tlastConflict = 0;
			double tcommittedPenalties = 0.0;
			list<ScheduleNode*> tpointsOfConflict;
			*localPenalty = evaluateSchedule(*schedToUse, &tmp, false, tpointsOfConflict, spID, tlastConflict, newPenaltiesOnly, exclude, tcommittedPenalties, newBoundaryNode );
	
		}
	}
	
	schedToUse->unpostConstraint();

	return toReturn;

};

double ScheduleEvaluatorStandard::heuristicPenaltiesOldSchedule(CandidateSchedule & c,
							const int & spID,
							const bool & newPenaltiesOnly) {

	static int counter = 0;

	//const int exclude = (GlobalSchedule::penaliseHead ? 0 : makeshiftPlan.size());
	const int exclude = 0;
	//if (now.wait) cout << "** waiting for " << *(now.wait->getOp()) << " **\n";
	if (GlobalSchedule::globalVerbosity & 2) {
		cout << " ( " << (++counter) << " ) ";
		cout.flush();
	}




	bool tmp = false;


	list<ScheduleNode*> pointsOfConflict;
	ScheduleNode* lastConflict = 0;


	list<ScheduleNode*> impliedWaits;

	double committedPenalties = 0.0;

	double toReturn = evaluateSchedule(c, &tmp, false, pointsOfConflict, spID, lastConflict, newPenaltiesOnly, exclude, committedPenalties, 0);
	

	if (GlobalSchedule::penaliseHead) toReturn += committedPenalties;



	return toReturn;

};

/*double ScheduleEvaluatorStandard::heuristicPenaltiesAlternative(
							list<TimelinePoint*>::iterator & t,
							CandidateSchedule & c,
							list<FFEvent> & header, FFEvent & now,
							list<instantiatedOp*> & relaxedPlan,
							bool & cycle, const int & spID) {

	cycle = false;

	map<Literal*, set<OpCount>, LitLT> activeDeletes;
	map<Literal*, set<OpCount>, LitLT> activeInvariants;

	Literal* const currSubGoal = Decomposition::getSubproblem(spID)->goals.front();



	list<ScheduleNode*> scheduledHolds;

	{

		list<FFEvent>::iterator hItr = header.begin();
		const list<FFEvent>::iterator hEnd = header.end();

		for (; hItr != hEnd; ++hItr) {
			if (hItr->wait) scheduledHolds.push_back(hItr->wait);
		}

		if (now.wait) scheduledHolds.push_back(now.wait);
	}


	list<ScheduleNode*>::iterator nextHold = scheduledHolds.begin();

	list<TimelinePoint*>::iterator exStart = c.timeline.begin();

	{ // skip through events before the initial state presented to the planner, adding active invariants and registering deletes but don't collect penalties
		
		for (; exStart != t; ++exStart) {
			if (!((*exStart)->dummy)) {
				ScheduleNode * const currSN = *((*exStart)->after);

				if (currSN->isStart()) { // start nodes register their effects and /add/ invariants
					addDeletes(activeDeletes, currSN->getOp(), currSN->getDeleteEffects());
					addAdds(activeDeletes, currSN->getOp(), currSN->getAddEffects());
					addInvariants(activeInvariants, currSN->getOp(), currSN->getInvariants());
				} else { // end nodes register their effects and /remove/ invariants
					addDeletes(activeDeletes, currSN->getOp(), currSN->getDeleteEffects());
					addAdds(activeDeletes, currSN->getOp(), currSN->getAddEffects());
					removeInvariants(activeInvariants, currSN->getOp(), currSN->getInvariants());
				}

			}
		}
	}

	if (exStart == c.timeline.begin()) ++exStart;

	double timeExisting = 0.0;// time at which next existing action is scheduled
	double timeNew = 0.0;// time at which next new action is scheduled
	
	if (exStart != c.timeline.end()) {
		timeExisting  = (*exStart)->timestamp;
		timeNew = timeExisting;
	}; 

	double offsetExisting = 0.0;

	const double EPSILON = 0.001; // FIXME this should be a global

	double penalty = 0.0; // accumulated penalty

	const list<TimelinePoint*>::iterator endExisting = c.timeline.end();

	header.push_back(now); // a hack to avoid copy & pasting - take the 'now' action as read

	list<ExistingWait> existingHolds;
	map<ScheduleNode*, int, ScheduleNodeLT> existingHoldsNodes;
	const bool ehnDebug = false;
	{ // new chunk of code, 19th Jan 2007, to add 'wait' for last achiever of top-level goal in each subproblem

		map<Literal*, pair<ExistingWait, ExistingWait>, LiteralLT> goalWaits;

		map<Literal*, pair<ScheduleNode*, ScheduleNode*>, LiteralLT> latestAchievers;

		{

			list<TimelinePoint*>::reverse_iterator rItr = c.timeline.rbegin();
			const list<TimelinePoint*>::reverse_iterator rEnd = c.timeline.rend();
			set<int> seenStart;
			set<int> seenEnd;

			for (; rItr != rEnd; ++rItr) {
				if (!(*rItr)->dummy) {
					ScheduleNode* const csn = *((*rItr)->after);
					const int subprob = csn->getSubProblem();
					if (csn->isStart() && seenStart.find(subprob) == seenStart.end()){
						seenStart.insert(subprob);
						latestAchievers[Decomposition::getSubproblem(subprob)->goals.front()].first = csn;
						if (ehnDebug) cout << "Start achiever for " << *(Decomposition::getSubproblem(subprob)->goals.front()) << " set to " << csn << "\n";
					} else if (seenEnd.find(subprob) == seenEnd.end()) {
						seenEnd.insert(subprob);
						latestAchievers.insert(pair<Literal*, pair<ScheduleNode*, ScheduleNode*> >(Decomposition::getSubproblem(subprob)->goals.front(), pair<ScheduleNode*, ScheduleNode*>(0,csn)));
						if (ehnDebug) cout << "End achiever for " << *(Decomposition::getSubproblem(subprob)->goals.front()) << " set to " << csn << "\n";
					}
				}
			}

		}



		if (ehnDebug) cout << "Achievers found for " << latestAchievers.size() << " literals\n";
		map<Literal*, pair<ScheduleNode*, ScheduleNode*>, LiteralLT>::iterator gItr = latestAchievers.begin();
		const map<Literal*, pair<ScheduleNode*, ScheduleNode*>, LiteralLT>::iterator gEnd = latestAchievers.end();

		for (; gItr != gEnd; ++gItr) {
			
			goalWaits.insert(pair<Literal*, pair<ExistingWait, ExistingWait> >(gItr->first, pair<ExistingWait, ExistingWait>(ExistingWait(gItr->second.first, -1), ExistingWait(gItr->second.second, -1))));

		}

		int i = 0;

		{
			list<FFEvent>::iterator headerItr = header.begin();
			const list<FFEvent>::iterator endNew = header.end();

			for (; headerItr != endNew; ++headerItr, ++i) {
				FFEvent & currEvent = *headerItr;
				if (currEvent.action) {
					if (ehnDebug) cout << i << ": " << *(currEvent.action) << "\n";
					list<Literal*> adds;
					list<Literal*> deletes;
					list<Literal*> precsInv;
				
					RPGBuilder::getCollapsedAction(currEvent.action, precsInv, adds, deletes); // can reason with collapsed actions: no concurrency as no actions alongside header - gap being implictly inserted into existing schedule to allow enough time before an existing action

					list<Literal*>::iterator delItr = deletes.begin();
					const list<Literal*>::iterator delEnd = deletes.end();

					for (; delItr != delEnd; ++delItr) {
						Literal* const currLit = *delItr;
						map<Literal*, pair<ExistingWait, ExistingWait>, LiteralLT>::iterator gwFind = goalWaits.find(currLit);
						if (gwFind != goalWaits.end()) {
							gwFind->second.first.comesAfter = i;
							gwFind->second.second.comesAfter = i;
						}
					}

				} else if (currEvent.wait) {
					if (ehnDebug) cout << i << ": reuse " << *(currEvent.wait->getOp()) << "\n";
				} else {
					if (ehnDebug) cout << i << ": Nothing at all\n";
				}
			}
		}

		{
			list<instantiatedOp*>::iterator headerItr = relaxedPlan.begin();
			const list<instantiatedOp*>::iterator endNew = relaxedPlan.end();

			for (; headerItr != endNew; ++headerItr, ++i) {
				if (ehnDebug) cout << i << ": " << *(*headerItr) << "\n";
				list<Literal*> adds;
				list<Literal*> deletes;
				list<Literal*> precsInv;
			
				RPGBuilder::getCollapsedAction(*headerItr, precsInv, adds, deletes); // can reason with collapsed actions: no concurrency as no actions alongside header - gap being implictly inserted into existing schedule to allow enough time before an existing action

				list<Literal*>::iterator delItr = deletes.begin();
				const list<Literal*>::iterator delEnd = deletes.end();

				for (; delItr != delEnd; ++delItr) {
					Literal* const currLit = *delItr;
					map<Literal*, pair<ExistingWait, ExistingWait>, LiteralLT>::iterator gwFind = goalWaits.find(currLit);
					if (gwFind != goalWaits.end()) {
						gwFind->second.first.comesAfter = i;
						gwFind->second.second.comesAfter = i;
					}
				}
			}
		}
		
		map<Literal*, pair<ExistingWait, ExistingWait>, LiteralLT>::iterator gwItr = goalWaits.begin();
		const map<Literal*, pair<ExistingWait, ExistingWait>, LiteralLT>::iterator gwEnd = goalWaits.end();

		for (; gwItr != gwEnd; ++gwItr) {
			const int index = gwItr->second.first.comesAfter;
			if (index != -1) {
				if (ehnDebug) cout << "Node from schedule for action " << *(gwItr->second.first.existingNode->getOp()) << " has to come after action at timestamp " << index << "\n";
				for (int pass = 0; pass < 2; ++pass) {// first then second
					ExistingWait & fos = (pass ? gwItr->second.first : gwItr->second.second);
					if (existingHolds.empty()) {
						existingHolds.push_back(fos);
					} else {
						list<ExistingWait>::iterator exItr = existingHolds.begin();
						const list<ExistingWait>::iterator exEnd = existingHolds.end();
	
						for (; exItr != exEnd && exItr->comesAfter <= index; ++exItr);
	
						existingHolds.insert(exItr, fos);					
					}
					map<ScheduleNode*, int, ScheduleNodeLT>::iterator enhItr = existingHoldsNodes.find(fos.existingNode);
					if (enhItr == existingHoldsNodes.end()) {					
						existingHoldsNodes.insert(pair<ScheduleNode*, int>(fos.existingNode, 1));
					} else {
						++(enhItr->second);
					}
				}
			}
		}
		
		if (ehnDebug) {
			map<ScheduleNode*, int, ScheduleNodeLT>::iterator ehnItr = existingHoldsNodes.begin();
			const map<ScheduleNode*, int, ScheduleNodeLT>::iterator ehnEnd = existingHoldsNodes.end();

			for (; ehnItr != ehnEnd; ++ehnItr) {
				cout << "Node from schedule for action " << *(ehnItr->first->getOp()) << " has to come after " << ehnItr->second << " things\n";
			}
		}

	}
	bool extraPop = false;

	{	// new chunk of code, 12th feb, make last action for new sp solution come after things which delete the goal
		ScheduleNode* lastDeletor = 0;
		
		{

			list<TimelinePoint*>::reverse_iterator rItr = c.timeline.rbegin();
			const list<TimelinePoint*>::reverse_iterator rEnd = c.timeline.rend();
			set<int> seenStart;
			set<int> seenEnd;

			for (; rItr != rEnd; ++rItr) {
				if (!(*rItr)->dummy) {
					ScheduleNode* const csn = *((*rItr)->after);
					list<Literal*> & csnDeletes = csn->getDeleteEffects();
					list<Literal*>::iterator csnDelItr = csnDeletes.begin();
					const list<Literal*>::iterator csnDelEnd = csnDeletes.end();

					for (; csnDelItr != csnDelEnd; ++csnDelItr) {
						if (*csnDelItr == currSubGoal) {
							lastDeletor = csn;
							break;
						}
					}
				}
				if (lastDeletor) break;
			}

		}

		if (lastDeletor) {
			if (relaxedPlan.empty()) {
				extraPop = true;
				if (now.wait) {
					scheduledHolds.pop_back();
					scheduledHolds.push_back(lastDeletor);
					scheduledHolds.push_back(now.wait);
				} else {
					scheduledHolds.push_back(lastDeletor);
				}
				header.pop_back();
				header.push_back(FFEvent(lastDeletor));
				header.push_back(now);
			} else {
				extraPop = false;
				scheduledHolds.push_back(lastDeletor);
			}
		}
	}

	int existingActionsUsed = 0;
	list<ExistingWait>::iterator ehItr = existingHolds.begin();

	{ // go through the plan header followed by the now action

		bool actionStart = true;
		list<FFEvent>::iterator headerItr = header.begin();
		const list<FFEvent>::iterator endNew = header.end();

		while(headerItr != endNew) {

			if (exStart != endExisting) { // if we still have the rest of the existing schedule to consider
				
				// three cases to consider here:
				//   i) next existing schedule node is a dummy for the initial state - always go for this first
				//  ii) next schedule node is the next forthcoming FFEvent wait - only consider adding actions from the header
				// iii) next schedule node isn't, in which case compare adjusted timestamps, putting existing actions first to maintain stability


				ScheduleNode* const nextExistingSN = *((*exStart)->after);
				if (!nextExistingSN) { // case i
					++exStart; // don't need to do anything
				} else {
					if ((nextHold != scheduledHolds.end()) && nextExistingSN == *nextHold) { // case ii
						FFEvent & currEvent = *headerItr;
						if (currEvent.action) {
							if (ehnDebug) cout << "Evaluating new action " << existingActionsUsed << " - " << *(currEvent.action) << "\n";
							list<Literal*> adds;
							list<Literal*> deletes;
							list<Literal*> precsInv;
				
							RPGBuilder::getCollapsedAction(currEvent.action, precsInv, adds, deletes); // can reason with collapsed actions: no concurrency as no actions alongside header - gap being implictly inserted into existing schedule to allow enough time before an existing action

							penalisePreconditions(activeDeletes, currEvent.action, precsInv, penalty); // only one source of penalties - using facts previously deleted
							addDeletes(activeDeletes, currEvent.action, deletes);
							addAdds(activeDeletes, currEvent.action, adds);

							timeNew += RPGBuilder::getOpDuration(currEvent.action) + EPSILON;

							++headerItr;
							++existingActionsUsed;
							while (ehItr != existingHolds.end() && ehItr->comesAfter < existingActionsUsed) {
								if (ehnDebug) cout << "Decrementing hold count for node " << *(ehItr->existingNode->getOp()) << "\n";
								--existingHoldsNodes[ehItr->existingNode];
								++ehItr;
							}	
						} else {
							if (ehnDebug) cout << "Evaluating new wait " << existingActionsUsed << "\n";
							assert(currEvent.wait); // FFEvents should always either be actions or waits
							++nextHold; // if it's a weight we can advance the iterator to the list of scheduled holds

							offsetExisting += timeNew - timeExisting;
							timeNew = timeExisting + offsetExisting + RPGBuilder::getOpDuration((*exStart)->afterOp()) + EPSILON; // put the time of the next new action to occur definitely after the waited-for action

							++headerItr;
							++existingActionsUsed;
							while (ehItr != existingHolds.end() && ehItr->comesAfter < existingActionsUsed) {
								if (ehnDebug) cout << "Decrementing hold count for node " << *(ehItr->existingNode->getOp()) << "\n";
								--existingHoldsNodes[ehItr->existingNode];
								++ehItr;
							}
						}
					} else { // case iii
						if (headerItr->wait || (timeExisting + offsetExisting) <= timeNew) { // if an existing action should be considered next
							bool addExisting = true;
							if (ehItr != existingHolds.end()) {
								map<ScheduleNode*, int, ScheduleNodeLT>::iterator enhItr = existingHoldsNodes.find(nextExistingSN);
								if (enhItr != existingHoldsNodes.end()) {
									if (enhItr->second) {
										if (ehnDebug) cout << "Waiting for new action before adding existing action " << *(nextExistingSN->getOp()) << "\n";
										addExisting = false;
										if (headerItr->wait) {
											cycle = true;
											if (ehnDebug) cout << "Aha - we have a cycle\n";
											header.pop_back(); // remove the now action from the header, caller will never know...
											return -1.0;
										}
									}
								}
								
							}
							if (addExisting) {
								if (nextExistingSN->isStart()) {
									instantiatedOp* const currOp = nextExistingSN->getOp();
									// if the next existing schedule node is an action start
									if (ehnDebug) cout << "Evaluating start of existing action " << (*currOp) << "\n";									
									penalisePreconditions(activeDeletes, currOp, nextExistingSN->getPreconditions(), penalty);
									penaliseDeleteEffects(activeInvariants, currOp, nextExistingSN->getDeleteEffects(), penalty);
									addDeletes(activeDeletes, currOp, nextExistingSN->getDeleteEffects());
									addAdds(activeDeletes, currOp, nextExistingSN->getAddEffects());
	
									penalisePreconditions(activeDeletes, currOp, nextExistingSN->getInvariants(), penalty);
									addInvariants(activeInvariants, currOp, nextExistingSN->getInvariants());
									
									++exStart;
									if (exStart != endExisting) timeExisting = (*exStart)->timestamp;
								} else {
									instantiatedOp* const currOp = nextExistingSN->getOp();
									// if the next existing schedule node is an action end
									if (ehnDebug) cout << "Evaluating end of existing action " << (*currOp) << "\n";									
									penalisePreconditions(activeDeletes, currOp, nextExistingSN->getPreconditions(), penalty);
									removeInvariants(activeInvariants, currOp, nextExistingSN->getInvariants());
									penaliseDeleteEffects(activeInvariants, currOp, nextExistingSN->getDeleteEffects(), penalty);
									addDeletes(activeDeletes, currOp, nextExistingSN->getDeleteEffects());
									addAdds(activeDeletes, currOp, nextExistingSN->getAddEffects());
	
									++exStart;
									if (exStart != endExisting) timeExisting = (*exStart)->timestamp;
								}
							} else {
								instantiatedOp* const currOp = headerItr->action; 
								if (currOp) {
									{ // if the next action's start comes next
										
										if (ehnDebug) cout << "Evaluating start of new action " << existingActionsUsed << " - " << *(currOp) << "\n";
										list<Literal*> adds;
										list<Literal*> deletes;
										list<Literal*> precs;
										list<Literal*> invs;	
						
										RPGBuilder::getEffects(currOp, true, adds, deletes);
										RPGBuilder::getPrecInv(currOp, true, precs, invs);
		
										penalisePreconditions(activeDeletes, currOp, precs, penalty);
										penaliseDeleteEffects(activeInvariants, currOp, deletes, penalty);
										addDeletes(activeDeletes, currOp, deletes);
										addAdds(activeDeletes, currOp, adds);
		
										penalisePreconditions(activeDeletes, currOp, invs, penalty);
										addInvariants(activeInvariants, currOp, invs);
		
		
										actionStart = false; // post-protocol - the next is an end
									}
									{
										
										if (ehnDebug) cout << "Evaluating end of new action " << existingActionsUsed << " - " << *(currOp) << "\n";
										list<Literal*> adds;
										list<Literal*> deletes;
										list<Literal*> precs;
										list<Literal*> invs;	
						
										RPGBuilder::getEffects(currOp, false, adds, deletes);
										RPGBuilder::getPrecInv(currOp, false, precs, invs);
		
										penalisePreconditions(activeDeletes, currOp, precs, penalty);
										removeInvariants(activeInvariants, currOp, invs);
										penaliseDeleteEffects(activeInvariants, currOp, deletes, penalty);
										addDeletes(activeDeletes, currOp, deletes);
										addAdds(activeDeletes, currOp, adds);
		
										offsetExisting += timeNew - timeExisting;
										timeNew = timeExisting + offsetExisting + RPGBuilder::getOpDuration(currOp) + EPSILON; // put the time of the next new action to occur definitely after the waited-for action
	
	
										++headerItr;		// post-protocol - advance onto the next action
										actionStart = true; 	// thus, the next thing is an action start
										
										
										++existingActionsUsed;
										while (ehItr != existingHolds.end() && ehItr->comesAfter < existingActionsUsed) {
											if (ehnDebug) cout << "Decrementing hold count for node " << *(ehItr->existingNode->getOp()) << "\n";
											--existingHoldsNodes[ehItr->existingNode];
											++ehItr;
										}
									}
								} else {
									++headerItr;		// post-protocol - advance onto the next action
									actionStart = true; 	// thus, the next thing is an action start
									
									
									++existingActionsUsed;
									while (ehItr != existingHolds.end() && ehItr->comesAfter < existingActionsUsed) {
										--existingHoldsNodes[ehItr->existingNode];
										++ehItr;
									}
								}
							}
						} else {
							if (actionStart) { // if the next action's start comes next

								instantiatedOp* const currOp = headerItr->action; 
								if (ehnDebug) cout << "Evaluating start of new action " << existingActionsUsed << " - " << *(currOp) << "\n";
								list<Literal*> adds;
								list<Literal*> deletes;
								list<Literal*> precs;
								list<Literal*> invs;	
				
								RPGBuilder::getEffects(currOp, true, adds, deletes);
								RPGBuilder::getPrecInv(currOp, true, precs, invs);

								penalisePreconditions(activeDeletes, currOp, precs, penalty);
								penaliseDeleteEffects(activeInvariants, currOp, deletes, penalty);
								addDeletes(activeDeletes, currOp, deletes);
								addAdds(activeDeletes, currOp, adds);

								penalisePreconditions(activeDeletes, currOp, invs, penalty);
								addInvariants(activeInvariants, currOp, invs);


								actionStart = false; // post-protocol - the next is an end
							} else {
								instantiatedOp* const currOp = headerItr->action; 
								if (ehnDebug) cout << "Evaluating start of new action " << existingActionsUsed << " - " << *(currOp) << "\n";
								list<Literal*> adds;
								list<Literal*> deletes;
								list<Literal*> precs;
								list<Literal*> invs;	
				
								RPGBuilder::getEffects(currOp, false, adds, deletes);
								RPGBuilder::getPrecInv(currOp, false, precs, invs);

								penalisePreconditions(activeDeletes, currOp, precs, penalty);
								removeInvariants(activeInvariants, currOp, invs);
								penaliseDeleteEffects(activeInvariants, currOp, deletes, penalty);
								addDeletes(activeDeletes, currOp, deletes);
								addAdds(activeDeletes, currOp, adds);

								++headerItr;		// post-protocol - advance onto the next action
								actionStart = true; 	// thus, the next thing is an action start

								++existingActionsUsed;
								while (ehItr != existingHolds.end() && ehItr->comesAfter < existingActionsUsed) {
									if (ehnDebug) cout << "Decrementing hold count for node " << *(ehItr->existingNode->getOp()) << "\n";
									--existingHoldsNodes[ehItr->existingNode];
									++ehItr;
								}
							}
						}
					}
				}
				
				
	
			} else { // otherwise it's fairly easy

				instantiatedOp* const op = headerItr->action;
				if (op) { // for the initial state this can be 0

					list<Literal*> adds;
					list<Literal*> deletes;
					list<Literal*> precsInv;
					
					RPGBuilder::getCollapsedAction(op, precsInv, adds, deletes);
	
					penalisePreconditions(activeDeletes, op, precsInv, penalty); // only one source of penalties - using facts previously deleted
					addDeletes(activeDeletes, op, deletes);
					addAdds(activeDeletes, op, adds);
				}
				++headerItr;
			}

		}

	}

	header.pop_back(); // remove the now action from the header, caller will never know...
	if (extraPop) header.pop_back();

	{ 
		bool actionStart = true;
		list<instantiatedOp*>::iterator headerItr = relaxedPlan.begin();
		const list<instantiatedOp*>::iterator endNew = relaxedPlan.end();

		while(headerItr != endNew) {

			if (exStart != endExisting) { // if we still have the rest of the existing schedule to consider
				
				ScheduleNode* const nextExistingSN = *((*exStart)->after);
				if (!nextExistingSN) { // case i
					++exStart; // don't need to do anything
				} else {
					if ((nextHold != scheduledHolds.end()) && nextExistingSN == *nextHold) {
						list<instantiatedOp*>::iterator nextHeaderItr = headerItr;
						++nextHeaderItr;
						if (nextHeaderItr == relaxedPlan.end()) {
							++nextHold;
							if (ehItr != existingHolds.end()) {
								map<ScheduleNode*, int, ScheduleNodeLT>::iterator enhItr = existingHoldsNodes.find(nextExistingSN);
								if (enhItr != existingHoldsNodes.end()) {
									if (enhItr->second) {
										cycle = true;
										return -1.0;
									}
								}
							}
							if (nextExistingSN->isStart()) {
								instantiatedOp* const currOp = nextExistingSN->getOp();
								if (ehnDebug) cout << "Evaluating start of existing action " << (*currOp) << "\n";
								// if the next existing schedule node is an action start
								penalisePreconditions(activeDeletes, currOp, nextExistingSN->getPreconditions(), penalty);
								penaliseDeleteEffects(activeInvariants, currOp, nextExistingSN->getDeleteEffects(), penalty);
								addDeletes(activeDeletes, currOp, nextExistingSN->getDeleteEffects());
								addAdds(activeDeletes, currOp, nextExistingSN->getAddEffects());
	
								penalisePreconditions(activeDeletes, currOp, nextExistingSN->getInvariants(), penalty);
								addInvariants(activeInvariants, currOp, nextExistingSN->getInvariants());
								
								++exStart;
								if (exStart != endExisting) timeExisting = (*exStart)->timestamp;
							} else {
								instantiatedOp* const currOp = nextExistingSN->getOp();
								if (ehnDebug) cout << "Evaluating end of existing action " << (*currOp) << "\n";
								// if the next existing schedule node is an action end
								penalisePreconditions(activeDeletes, currOp, nextExistingSN->getPreconditions(), penalty);
								removeInvariants(activeInvariants, currOp, nextExistingSN->getInvariants());
								penaliseDeleteEffects(activeInvariants, currOp, nextExistingSN->getDeleteEffects(), penalty);
								addDeletes(activeDeletes, currOp, nextExistingSN->getDeleteEffects());
								addAdds(activeDeletes, currOp, nextExistingSN->getAddEffects());
	
								++exStart;
								if (exStart != endExisting) timeExisting = (*exStart)->timestamp;
							}

						} else {
							if (actionStart) { // if the next action's start comes next
								instantiatedOp* const currOp = *headerItr;
								if (ehnDebug) cout << "Evaluating start of new action " << existingActionsUsed << " - " << *(currOp) << "\n";
								list<Literal*> adds;
								list<Literal*> deletes;
								list<Literal*> precs;
								list<Literal*> invs;	
				
								RPGBuilder::getEffects(currOp, true, adds, deletes);
								RPGBuilder::getPrecInv(currOp, true, precs, invs);
	
								penalisePreconditions(activeDeletes, currOp, precs, penalty);
								penaliseDeleteEffects(activeInvariants, currOp, deletes, penalty);
								addDeletes(activeDeletes, currOp, deletes);
								addAdds(activeDeletes, currOp, adds);
	
								penalisePreconditions(activeDeletes, currOp, invs, penalty);
								addInvariants(activeInvariants, currOp, invs);
	
	
								actionStart = false; // post-protocol - the next is an end
							} else {
								instantiatedOp* const currOp = *headerItr;
								if (ehnDebug) cout << "Evaluating end of new action " << existingActionsUsed << " - " << *(currOp) << "\n";
								list<Literal*> adds;
								list<Literal*> deletes;
								list<Literal*> precs;
								list<Literal*> invs;	
				
								RPGBuilder::getEffects(currOp, false, adds, deletes);
								RPGBuilder::getPrecInv(currOp, false, precs, invs);
	
								penalisePreconditions(activeDeletes, currOp, precs, penalty);
								removeInvariants(activeInvariants, currOp, invs);
								penaliseDeleteEffects(activeInvariants, currOp, deletes, penalty);
								addDeletes(activeDeletes, currOp, deletes);
								addAdds(activeDeletes, currOp, adds);
	
								offsetExisting += timeNew - timeExisting;
								timeNew = timeExisting + offsetExisting + RPGBuilder::getOpDuration(currOp) + EPSILON; // put the time of the next new action to occur definitely after the waited-for action
		
		
	
								++headerItr;		// post-protocol - advance onto the next action
								actionStart = true; 	// thus, the next thing is an action start
	
								++existingActionsUsed;
								while (ehItr != existingHolds.end() && ehItr->comesAfter < existingActionsUsed) {
									if (ehnDebug) cout << "Decrementing hold count for node " << *(ehItr->existingNode->getOp()) << "\n";
									--existingHoldsNodes[ehItr->existingNode];
									++ehItr;
								}
							}
						}
					} else {
						if ((timeExisting + offsetExisting) <= timeNew) { // if an existing action should be considered next
							bool addExisting = true;
							if (ehItr != existingHolds.end()) {
								map<ScheduleNode*, int, ScheduleNodeLT>::iterator enhItr = existingHoldsNodes.find(nextExistingSN);
								if (enhItr != existingHoldsNodes.end()) {
									if (enhItr->second) {
										addExisting = false;
										if (ehnDebug) cout << "Waiting for new action before adding existing action " << *(nextExistingSN->getOp()) << "\n";
									}
								}
								
							}
							if (addExisting) {
								if (nextExistingSN->isStart()) {
									instantiatedOp* const currOp = nextExistingSN->getOp();
									if (ehnDebug) cout << "Evaluating start of existing action " << (*currOp) << "\n";
									// if the next existing schedule node is an action start
									penalisePreconditions(activeDeletes, currOp, nextExistingSN->getPreconditions(), penalty);
									penaliseDeleteEffects(activeInvariants, currOp, nextExistingSN->getDeleteEffects(), penalty);
									addDeletes(activeDeletes, currOp, nextExistingSN->getDeleteEffects());
									addAdds(activeDeletes, currOp, nextExistingSN->getAddEffects());
		
									penalisePreconditions(activeDeletes, currOp, nextExistingSN->getInvariants(), penalty);
									addInvariants(activeInvariants, currOp, nextExistingSN->getInvariants());
									
									++exStart;
									if (exStart != endExisting) timeExisting = (*exStart)->timestamp;
								} else {
									instantiatedOp* const currOp = nextExistingSN->getOp();
									if (ehnDebug) cout << "Evaluating end of existing action " << (*currOp) << "\n";
									// if the next existing schedule node is an action end
									penalisePreconditions(activeDeletes, currOp, nextExistingSN->getPreconditions(), penalty);
									removeInvariants(activeInvariants, currOp, nextExistingSN->getInvariants());
									penaliseDeleteEffects(activeInvariants, currOp, nextExistingSN->getDeleteEffects(), penalty);
									addDeletes(activeDeletes, currOp, nextExistingSN->getDeleteEffects());
									addAdds(activeDeletes, currOp, nextExistingSN->getAddEffects());
		
									++exStart;
									if (exStart != endExisting) timeExisting = (*exStart)->timestamp;
								}
							} else {
								if (actionStart) { // if the next action's start comes next
									instantiatedOp* const currOp = *headerItr;
									if (ehnDebug) cout << "Evaluating start of new action " << existingActionsUsed << " - " << *(currOp) << "\n";
									list<Literal*> adds;
									list<Literal*> deletes;
									list<Literal*> precs;
									list<Literal*> invs;	
					
									RPGBuilder::getEffects(currOp, true, adds, deletes);
									RPGBuilder::getPrecInv(currOp, true, precs, invs);
		
									penalisePreconditions(activeDeletes, currOp, precs, penalty);
									penaliseDeleteEffects(activeInvariants, currOp, deletes, penalty);
									addDeletes(activeDeletes, currOp, deletes);
									addAdds(activeDeletes, currOp, adds);
		
									penalisePreconditions(activeDeletes, currOp, invs, penalty);
									addInvariants(activeInvariants, currOp, invs);
		
		
									actionStart = false; // post-protocol - the next is an end
								} else {
									instantiatedOp* const currOp = *headerItr;
									if (ehnDebug) cout << "Evaluating end of new action " << existingActionsUsed << " - " << *(currOp) << "\n";
									list<Literal*> adds;
									list<Literal*> deletes;
									list<Literal*> precs;
									list<Literal*> invs;	
					
									RPGBuilder::getEffects(currOp, false, adds, deletes);
									RPGBuilder::getPrecInv(currOp, false, precs, invs);
		
									penalisePreconditions(activeDeletes, currOp, precs, penalty);
									removeInvariants(activeInvariants, currOp, invs);
									penaliseDeleteEffects(activeInvariants, currOp, deletes, penalty);
									addDeletes(activeDeletes, currOp, deletes);
									addAdds(activeDeletes, currOp, adds);
		
									offsetExisting += timeNew - timeExisting;
									timeNew = timeExisting + offsetExisting + RPGBuilder::getOpDuration(currOp) + EPSILON; // put the time of the next new action to occur definitely after the waited-for action
			
			
		
									++headerItr;		// post-protocol - advance onto the next action
									actionStart = true; 	// thus, the next thing is an action start
		
									++existingActionsUsed;
									while (ehItr != existingHolds.end() && ehItr->comesAfter < existingActionsUsed) {
										if (ehnDebug) cout << "Decrementing hold count for node " << *(ehItr->existingNode->getOp()) << "\n";
										--existingHoldsNodes[ehItr->existingNode];
										++ehItr;
									}
								}
							}
						} else {
							if (actionStart) { // if the next action's start comes next
								instantiatedOp* const currOp = *headerItr;
								if (ehnDebug) cout << "Evaluating start of new action " << existingActionsUsed << " - " << *(currOp) << "\n";
								list<Literal*> adds;
								list<Literal*> deletes;
								list<Literal*> precs;
								list<Literal*> invs;	
				
								RPGBuilder::getEffects(currOp, true, adds, deletes);
								RPGBuilder::getPrecInv(currOp, true, precs, invs);
	
								penalisePreconditions(activeDeletes, currOp, precs, penalty);
								penaliseDeleteEffects(activeInvariants, currOp, deletes, penalty);
								addDeletes(activeDeletes, currOp, deletes);
								addAdds(activeDeletes, currOp, adds);
	
								penalisePreconditions(activeDeletes, currOp, invs, penalty);
								addInvariants(activeInvariants, currOp, invs);
	
	
								actionStart = false; // post-protocol - the next is an end
							} else {
								instantiatedOp* const currOp = *headerItr;
								if (ehnDebug) cout << "Evaluating end of new action " << existingActionsUsed << " - " << *(currOp) << "\n";
								list<Literal*> adds;
								list<Literal*> deletes;
								list<Literal*> precs;
								list<Literal*> invs;	
				
								RPGBuilder::getEffects(currOp, false, adds, deletes);
								RPGBuilder::getPrecInv(currOp, false, precs, invs);
	
								penalisePreconditions(activeDeletes, currOp, precs, penalty);
								removeInvariants(activeInvariants, currOp, invs);
								penaliseDeleteEffects(activeInvariants, currOp, deletes, penalty);
								addDeletes(activeDeletes, currOp, deletes);
								addAdds(activeDeletes, currOp, adds);
	
								offsetExisting += timeNew - timeExisting;
								timeNew = timeExisting + offsetExisting + RPGBuilder::getOpDuration(currOp) + EPSILON; // put the time of the next new action to occur definitely after the waited-for action
		
		
	
								++headerItr;		// post-protocol - advance onto the next action
								actionStart = true; 	// thus, the next thing is an action start
	
								++existingActionsUsed;
								while (ehItr != existingHolds.end() && ehItr->comesAfter < existingActionsUsed) {
									if (ehnDebug) cout << "Decrementing hold count for node " << *(ehItr->existingNode->getOp()) << "\n";
									--existingHoldsNodes[ehItr->existingNode];
									++ehItr;
								}
							}
						}
					}					
				}
				
				
	
			} else { // otherwise it's fairly easy

				instantiatedOp* const op = *headerItr;
				assert(op); // this has to hold true - a wait is not an option at this point, as there's no existing schedule to wait for

				list<Literal*> adds;
				list<Literal*> deletes;
				list<Literal*> precsInv;
				
				RPGBuilder::getCollapsedAction(op, precsInv, adds, deletes);

				penalisePreconditions(activeDeletes, op, precsInv, penalty); // only one source of penalties - using facts previously deleted
				addDeletes(activeDeletes, op, deletes);
				addAdds(activeDeletes, op, adds);

				++headerItr;
			}

		}

	}

	while (exStart != endExisting) { // if we still have the rest of the existing schedule to consider
				
		ScheduleNode* const nextExistingSN = *((*exStart)->after);
		if (!nextExistingSN) { // case i
			++exStart; // don't need to do anything
		} else {
			if (nextExistingSN->isStart()) {
				instantiatedOp* const currOp = nextExistingSN->getOp();
				if (ehnDebug) cout << "Evaluating start of existing action " << (*currOp) << "\n";
				// if the next existing schedule node is an action start
				penalisePreconditions(activeDeletes, currOp, nextExistingSN->getPreconditions(), penalty);
				penaliseDeleteEffects(activeInvariants, currOp, nextExistingSN->getDeleteEffects(), penalty);
				addDeletes(activeDeletes, currOp, nextExistingSN->getDeleteEffects());
				addAdds(activeDeletes, currOp, nextExistingSN->getAddEffects());

				penalisePreconditions(activeDeletes, currOp, nextExistingSN->getInvariants(), penalty);
				addInvariants(activeInvariants, currOp, nextExistingSN->getInvariants());
				
				++exStart;
				if (exStart != endExisting) timeExisting = (*exStart)->timestamp;
			} else {
				instantiatedOp* const currOp = nextExistingSN->getOp();
				if (ehnDebug) cout << "Evaluating end of existing action " << (*currOp) << "\n";
				// if the next existing schedule node is an action end
				penalisePreconditions(activeDeletes, currOp, nextExistingSN->getPreconditions(), penalty);
				removeInvariants(activeInvariants, currOp, nextExistingSN->getInvariants());
				penaliseDeleteEffects(activeInvariants, currOp, nextExistingSN->getDeleteEffects(), penalty);
				addDeletes(activeDeletes, currOp, nextExistingSN->getDeleteEffects());
				addAdds(activeDeletes, currOp, nextExistingSN->getAddEffects());

				++exStart;
				if (exStart != endExisting) timeExisting = (*exStart)->timestamp;
			}
			
		}
	}

	return penalty;

};
*/
double ScheduleEvaluatorStandard::evaluateSchedule(CandidateSchedule & c, bool * anyPenalties, bool debugEval, list<ScheduleNode*> & pointsOfConflict, const int & spIDglobal, ScheduleNode* & lastConflict, const bool & newPenaltiesOnly, const int & exclude, double & committedPenalty, ScheduleNode* onlyPenalise) {

	const bool approximatePenalties = (GlobalSchedule::approximatePenalties & 1);
	debugEval = debugEval || (GlobalSchedule::globalVerbosity & 256);
	static list<pair<ScheduleNode*, vector<double> > > ift;
	
	static bool initialisedIFT = false;

	if (!initialisedIFT) {

		LiteralSet doNotCare;
		vector<double> initialFluents;
		RPGBuilder::getInitialState(doNotCare, initialFluents);

		ift.push_back(pair<ScheduleNode*, vector<double> >(0, initialFluents));

		initialisedIFT = true;
	}

	int spID = spIDglobal;
	lastConflict = 0;
	if (debugEval) cout << "Evaluating schedule:\n";
	double penalty = 0.0;
	committedPenalty = 0.0;

	bool nothingBadAfterExclude = (GlobalSchedule::approximatePenalties & 4);

	map<Literal*, NodeOpCount, LitLT> activeDeletes;
	map<Literal*, NodeOpCount, LitLT> activeInvariants;
	map<int, NodeOpCount> activeFluentInvariants;
	list<pair<ScheduleNode*, vector<double> > > fluentTracking(ift);
	
	set<ScheduleNode*, ScheduleNodeLT> pocSet;
	set<ScheduleNode*, ScheduleNodeLT> exempt;

	map<ScheduleNode*, set<ScheduleNode*, ScheduleNodeLT>, ScheduleNodeLT> allAlreadyPenalised;

	map<int, int> alreadyStarted;

	int metFromSP = 0;
	{
		list<TimelinePoint*>::iterator exStart = c.timeline.begin();
		const list<TimelinePoint*>::iterator exEnd = c.timeline.end();
	
		++exStart; // skip dummy timeline point for initial state
	
		for (; exStart != exEnd; ++exStart) {
			ScheduleNode * const nextExistingSN = *((*exStart)->after);

			if (nextExistingSN->tilNode()) {
			
				if (nothingBadAfterExclude && metFromSP >= exclude) {
					addAdds(activeDeletes, nextExistingSN->getAddEffects());
				} else {
				
					set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised = allAlreadyPenalised[nextExistingSN];
					
					if (metFromSP >= exclude) {
						penaliseDeleteEffects(activeInvariants, nextExistingSN, nextExistingSN->getDeleteEffects(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penaliseDeleteEffects(activeInvariants, nextExistingSN, nextExistingSN->getDeleteEffects(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}

					addDeletes(activeDeletes, nextExistingSN, nextExistingSN->getDeleteEffects());
					addAdds(activeDeletes, nextExistingSN->getAddEffects());
				}
			
			} else if (nextExistingSN->isStart()) {
				{
					map<int, int>::iterator aspIterator = alreadyStarted.find(nextExistingSN->getOp()->getID());
					if (aspIterator == alreadyStarted.end()) {
						alreadyStarted.insert(pair<int, int>(nextExistingSN->getOp()->getID(), 1));
					} else {
						++(aspIterator->second);
					}
				}
				const bool fromSP = nextExistingSN->getSubProblem().find(spIDglobal) != nextExistingSN->getSubProblem().end();
			
				if (nothingBadAfterExclude && metFromSP >= exclude && fromSP) {


					//set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised = allAlreadyPenalised[nextExistingSN];

					//penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getPreconditions(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					addAdds(activeDeletes, nextExistingSN->getAddEffects());
					//penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getInvariants(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);

					//if (nextExistingSN->getPartner()) addInvariants(activeInvariants, nextExistingSN, nextExistingSN->getInvariants());

				} else {

					if (fromSP && metFromSP < exclude) {
						exempt.insert(nextExistingSN);
					}

					set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised = allAlreadyPenalised[nextExistingSN];
					instantiatedOp* const currOp = nextExistingSN->getOp();
					// if the next existing schedule node is an action start
					if (debugEval) {
						cout << (*exStart)->timestamp << ": start of " << *currOp << " for subproblems:";
						set<int> & tbSet = nextExistingSN->getSubProblem();
						set<int>::iterator tbsItr = tbSet.begin();
						const set<int>::iterator tbsEnd = tbSet.end();
						for (;tbsItr != tbsEnd; ++tbsItr) cout << " " << *tbsItr;
						cout << "\n";
					}
	
					if (metFromSP >= exclude) {
						penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getPreconditions(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getPreconditions(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}
	
					if (metFromSP >= exclude) {
						penaliseNumericPreconditions(fluentTracking, nextExistingSN, nextExistingSN->getNumericPreconditions(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penaliseNumericPreconditions(fluentTracking, nextExistingSN, nextExistingSN->getNumericPreconditions(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}

					if (metFromSP >= exclude) {
						penaliseDeleteEffects(activeInvariants, nextExistingSN, nextExistingSN->getDeleteEffects(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penaliseDeleteEffects(activeInvariants, nextExistingSN, nextExistingSN->getDeleteEffects(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}

					if (metFromSP >= exclude) {
						penaliseNumericEffects(activeFluentInvariants, nextExistingSN, nextExistingSN->getNumericEffects(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penaliseNumericEffects(activeFluentInvariants, nextExistingSN, nextExistingSN->getNumericEffects(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}	
					

					addDeletes(activeDeletes, nextExistingSN, nextExistingSN->getDeleteEffects());
					addAdds(activeDeletes, nextExistingSN->getAddEffects());
					addNumerics(fluentTracking, nextExistingSN, nextExistingSN->getNumericEffects());
	
					if (metFromSP >= exclude) {
						penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getInvariants(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getInvariants(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}
					
					if (metFromSP >= exclude) {
						penaliseNumericPreconditions(fluentTracking, nextExistingSN, nextExistingSN->getNumericInvariants(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penaliseNumericPreconditions(fluentTracking, nextExistingSN, nextExistingSN->getNumericInvariants(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}

					if (nextExistingSN->getPartner()) {
						addInvariants(activeInvariants, nextExistingSN, nextExistingSN->getInvariants());
						addFluentInvariants(activeFluentInvariants, nextExistingSN, RPGBuilder::getMentioned(nextExistingSN->getOp()->getID())); // lucky reuse!
					}
					
					

				}

				if (fromSP) {
					++metFromSP;
				}

				if (metFromSP >= exclude) {
					if (GlobalSchedule::penaliseHead) exempt.clear();
				}

			} else {

				if (debugEval) {	
					cout << (*exStart)->timestamp << ": end of " << *(nextExistingSN->getOp()) << " for subproblems:";
					set<int> & tbSet = nextExistingSN->getSubProblem();
					set<int>::iterator tbsItr = tbSet.begin();
					const set<int>::iterator tbsEnd = tbSet.end();
					for (;tbsItr != tbsEnd; ++tbsItr) cout << " " << *tbsItr;
					cout << "\n";
				}
				if(alreadyStarted.find(nextExistingSN->getOp()->getID()) == alreadyStarted.end()) {
					c.printEvents();
					assert(alreadyStarted.find(nextExistingSN->getOp()->getID()) != alreadyStarted.end());
				};
				if (!--(alreadyStarted[nextExistingSN->getOp()->getID()])) {

					alreadyStarted.erase(nextExistingSN->getOp()->getID());
				}
				//instantiatedOp* const currOp = nextExistingSN->getOp();
				set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised = allAlreadyPenalised[nextExistingSN->getPartner()];
				// if the next existing schedule node is an action end
				//if (debugEval) cout << (*exStart)->timestamp << ": " << *currOp << " finished\n";

				const bool fromSP = nextExistingSN->getSubProblem().find(spIDglobal) != nextExistingSN->getSubProblem().end();
			
				if (nothingBadAfterExclude && metFromSP >= exclude && fromSP) {

					//penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getPreconditions(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					removeInvariants(activeInvariants, nextExistingSN, nextExistingSN->getInvariants());
					addAdds(activeDeletes, nextExistingSN->getAddEffects());
					

				} else {

					if (fromSP && metFromSP < exclude) {
						exempt.insert(nextExistingSN);
					}

					if (metFromSP >= exclude) {
						penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getPreconditions(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getPreconditions(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}

					removeInvariants(activeInvariants, nextExistingSN, nextExistingSN->getInvariants());

					if (metFromSP >= exclude) {
						penaliseNumericPreconditions(fluentTracking, nextExistingSN, nextExistingSN->getNumericPreconditions(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penaliseNumericPreconditions(fluentTracking, nextExistingSN, nextExistingSN->getNumericPreconditions(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}

					removeFluentInvariants(activeFluentInvariants, nextExistingSN, RPGBuilder::getMentioned(nextExistingSN->getOp()->getID()));


					if (metFromSP >= exclude) {
						penaliseDeleteEffects(activeInvariants, nextExistingSN, nextExistingSN->getDeleteEffects(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penaliseDeleteEffects(activeInvariants, nextExistingSN, nextExistingSN->getDeleteEffects(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}

					if (metFromSP >= exclude) {
						penaliseNumericEffects(activeFluentInvariants, nextExistingSN, nextExistingSN->getNumericEffects(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					} else {
						penaliseNumericEffects(activeFluentInvariants, nextExistingSN, nextExistingSN->getNumericEffects(), penalty, committedPenalty, onlyPenalise, true, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);
					}

					addDeletes(activeDeletes, nextExistingSN, nextExistingSN->getDeleteEffects());
					addAdds(activeDeletes, nextExistingSN->getAddEffects());
					addNumerics(fluentTracking, nextExistingSN, nextExistingSN->getNumericEffects());

				}

				if (fromSP) {
					++metFromSP;
				}

				if (metFromSP >= exclude) {
					if (GlobalSchedule::penaliseHead) exempt.clear();
				}

			}
			
		}
	}

	if (GlobalSchedule::alternativeGP) {

		penaliseGP(fluentTracking, activeDeletes, penalty, onlyPenalise, pointsOfConflict, pocSet, spID, lastConflict, newPenaltiesOnly, approximatePenalties, anyPenalties, debugEval);

	}

	if (debugEval) {

		list<TimelinePoint*>::iterator exStart = c.timeline.begin();
		const list<TimelinePoint*>::iterator exEnd = c.timeline.end();
	
		++exStart; // skip dummy timeline point for initial state
	
		double prevTime = -1.0;
		for (; exStart != exEnd; ++exStart) {
			ScheduleNode * const nextExistingSN = *((*exStart)->after);

			if (nextExistingSN->tilNode()) {
			
				cout << (*exStart)->timestamp << ": Timed Initial Literal\n";
				cout << "\tAdds:\n";
				{
					list<Literal*> & tList = nextExistingSN->getAddEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
				cout << "\tDeletes:\n";
				{
					list<Literal*> & tList = nextExistingSN->getDeleteEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
				
			
			} else {
			
				if (nextExistingSN->isStart()) {
					instantiatedOp* const currOp = nextExistingSN->getOp();
					// if the next existing schedule node is an action start
					cout << (*exStart)->timestamp << ": " << *currOp << " start\n";
					
				} else {
					instantiatedOp* const currOp = nextExistingSN->getOp();
					cout << (*exStart)->timestamp << ": " << *currOp << " end\n";
					// if the next existing schedule node is an action end
					//if (debugEval) cout << (*exStart)->timestamp << ": " << *currOp << " finished\n";
				}
	//			assert((*exStart)->timestamp != prevTime);
				prevTime = (*exStart)->timestamp;
				cout << "\tSubproblems:";
				{
					set<int> & sps = nextExistingSN->getSubProblem();
					set<int>::iterator spsItr = sps.begin();
					const set<int>::iterator spsEnd = sps.end();
					for (; spsItr != spsEnd; ++spsItr) cout << " " << *spsItr;
					cout << "\n";
				}
				cout << "\tAdds:\n";
				{
					list<Literal*> & tList = nextExistingSN->getAddEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
				cout << "\tDeletes:\n";
				{
					list<Literal*> & tList = nextExistingSN->getDeleteEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
	
				cout << "\tPres:\n";
				{
					list<Literal*> & tList = nextExistingSN->getPreconditions();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
	
				cout << "\tInvs:\n";
				{
					list<Literal*> & tList = nextExistingSN->getInvariants();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
				
			}


		}
	}

	//if (anyPenalties && *anyPenalties) assert(!pocSet.empty());
	if (debugEval) {
		c.printEvents();
		cout << "Penalty for schedule = " << penalty << "\n";
		if (anyPenalties) cout << "Any penalties = " << *anyPenalties << "\n";
	}
	
	return penalty;

}

double ScheduleEvaluatorStandard::evaluateSchedule(CandidateSchedule & c, bool * anyPenalties, bool debugEval) {

	ScheduleNode* onlyPenalise = 0;

#define PRECONDITIONPARANOIA
	if (debugEval) cout << "Evaluating schedule:\n";
	double penalty = 0.0;

	map<Literal*, NodeOpCount, LitLT> activeDeletes;
	map<Literal*, NodeOpCount, LitLT> activeInvariants;
	map<int, NodeOpCount> activeFluentInvariants;

#ifdef PRECONDITIONPARANOIA
	static map<Literal*, set<int>, LitLT> initialState;
	static list<pair<ScheduleNode*, vector<double> > > ift;
	static vector<double> fluents;
	static bool initialised = false;

	static const int spCount = Decomposition::howMany();
	
	if (!initialised) {
		LiteralSet localLS;
		RPGBuilder::getInitialState(localLS, fluents);
		const int spCount = Decomposition::howMany();
		LiteralSet::iterator isItr = localLS.begin();
		const LiteralSet::iterator isEnd = localLS.end();

		set<int> toCopy;
		for (int i = 0; i < spCount; ++i) {
			toCopy.insert(i);
		}
		//cout << "\n";
		for (; isItr != isEnd; ++isItr) {
			initialState[*isItr] = toCopy;
			//cout << "Initial state contains " << (*isItr)->getID() << " - " << *(*isItr) << "\n";
		}
		
		ift.push_back(pair<ScheduleNode*, vector<double> >(0, fluents));

		initialised = true;
	}

	map<Literal*, set<int>, LitLT> allLiteralsEverSeen = initialState;
	list<pair<ScheduleNode*, vector<double> > > fluentTracking(ift);

	bool error = false;
	
#endif

	map<ScheduleNode*, set<ScheduleNode*, ScheduleNodeLT>, ScheduleNodeLT> allAlreadyPenalised;

	{
		list<TimelinePoint*>::iterator exStart = c.timeline.begin();
		const list<TimelinePoint*>::iterator exEnd = c.timeline.end();
	
		++exStart; // skip dummy timeline point for initial state
	
		for (; exStart != exEnd; ++exStart) {
			ScheduleNode * const nextExistingSN = *((*exStart)->after);


#ifdef PRECONDITIONPARANOIA

			set<int> & currSP = nextExistingSN->getSubProblem();

#endif

			if (nextExistingSN->tilNode()) {
				list<ScheduleNode*> pointsOfConflict;
				set<ScheduleNode*, ScheduleNodeLT> pocSet;
				set<ScheduleNode*, ScheduleNodeLT> exempt;
				double committedPenalty = 0.0;
				ScheduleNode* lastConflict;				
				int spID = -1;

				set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised = allAlreadyPenalised[nextExistingSN];
				
				penaliseDeleteEffects(activeInvariants, nextExistingSN, nextExistingSN->getDeleteEffects(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties);
				addDeletes(activeDeletes, nextExistingSN, nextExistingSN->getDeleteEffects());
				addAdds(activeDeletes, nextExistingSN->getAddEffects());

#ifdef PRECONDITIONPARANOIA
	{
		
		list<Literal*> & effs = nextExistingSN->getAddEffects();

		list<Literal*>::iterator pItr = effs.begin();
		const list<Literal*>::iterator pEnd = effs.end();

		for (; pItr != pEnd; ++pItr) {
			map<Literal*, set<int>, LitLT>::iterator ccItr = allLiteralsEverSeen.find(*pItr);
			
			if (ccItr == allLiteralsEverSeen.end()) {
				allLiteralsEverSeen[*pItr] = set<int>();
				ccItr = allLiteralsEverSeen.find(*pItr);
			} 
			
			set<int> & insertInto = ccItr->second;

			for (int i = 0; i < spCount; ++i) {
				insertInto.insert(i);
			}

		}
	}
#endif
							
			} else {

				if (nextExistingSN->isStart()) {
					set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised = allAlreadyPenalised[nextExistingSN];
					instantiatedOp* const currOp = nextExistingSN->getOp();
					// if the next existing schedule node is an action start
					if (debugEval) {
						cout << (*exStart)->timestamp << ": " << *currOp << " [1]\n";
					}
#ifdef PRECONDITIONPARANOIA
	{
		list<Literal*> & pres = nextExistingSN->getPreconditions();

		list<Literal*>::iterator pItr = pres.begin();
		const list<Literal*>::iterator pEnd = pres.end();

		for (; pItr != pEnd; ++pItr) {
			map<Literal*, set<int>, LitLT>::iterator ccItr = allLiteralsEverSeen.find(*pItr);
			if (ccItr == allLiteralsEverSeen.end()) {
				error = true;
			} else {
				set<int> & checkIn = ccItr->second;
				set<int>::iterator cspItr = currSP.begin();
				const set<int>::iterator cspEnd = currSP.end();
				for (; cspItr != cspEnd; ++cspItr) {
					if (checkIn.find(*cspItr) == checkIn.end()) {
						error = true;
						break;
					}
				}
			}
		}
	}
#endif
					list<ScheduleNode*> pointsOfConflict;
					set<ScheduleNode*, ScheduleNodeLT> pocSet;
					set<ScheduleNode*, ScheduleNodeLT> exempt;
					double committedPenalty = 0.0;
					ScheduleNode* lastConflict;
					int spID = -1;
	
					penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getPreconditions(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties, debugEval);
					penaliseNumericPreconditions(fluentTracking, nextExistingSN, nextExistingSN->getNumericPreconditions(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties, debugEval);
					penaliseDeleteEffects(activeInvariants, nextExistingSN, nextExistingSN->getDeleteEffects(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties);
					penaliseNumericEffects(activeFluentInvariants, nextExistingSN, nextExistingSN->getNumericEffects(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties);
					addDeletes(activeDeletes, nextExistingSN, nextExistingSN->getDeleteEffects());
					addAdds(activeDeletes, nextExistingSN->getAddEffects());
					addNumerics(fluentTracking, nextExistingSN, nextExistingSN->getNumericEffects());

#ifdef PRECONDITIONPARANOIA
	{
		
		list<Literal*> & effs = nextExistingSN->getAddEffects();

		list<Literal*>::iterator pItr = effs.begin();
		const list<Literal*>::iterator pEnd = effs.end();

		for (; pItr != pEnd; ++pItr) {
			map<Literal*, set<int>, LitLT>::iterator ccItr = allLiteralsEverSeen.find(*pItr);
			
			if (ccItr == allLiteralsEverSeen.end()) {
				allLiteralsEverSeen[*pItr] = set<int>();
				ccItr = allLiteralsEverSeen.find(*pItr);
			}
			
			set<int> & insertInto = ccItr->second;
			set<int>::iterator cspItr = currSP.begin();
			const set<int>::iterator cspEnd = currSP.end();
			for (; cspItr != cspEnd; ++cspItr) {
				insertInto.insert(*cspItr);
			}

		}
	}
#endif	

					penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getInvariants(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties, debugEval);
					penaliseNumericPreconditions(fluentTracking, nextExistingSN, nextExistingSN->getNumericInvariants(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties, debugEval);
					if (nextExistingSN->getPartner()) {
						addInvariants(activeInvariants, nextExistingSN, nextExistingSN->getInvariants());
						addFluentInvariants(activeFluentInvariants, nextExistingSN, RPGBuilder::getMentioned(nextExistingSN->getOp()->getID()));
					}
				} else {
					//instantiatedOp* const currOp = nextExistingSN->getOp();
					set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised = allAlreadyPenalised[nextExistingSN->getPartner()];
					// if the next existing schedule node is an action end
					//if (debugEval) cout << (*exStart)->timestamp << ": " << *currOp << " finished\n";
					if (debugEval) {
						cout << "; end of " << (*exStart)->timestamp << ": " << *(nextExistingSN->getOp()) << " [1]\n";
					}

#ifdef PRECONDITIONPARANOIA
	{
		list<Literal*> & pres = nextExistingSN->getPreconditions();

		list<Literal*>::iterator pItr = pres.begin();
		const list<Literal*>::iterator pEnd = pres.end();

		for (; pItr != pEnd; ++pItr) {
			map<Literal*, set<int>, LitLT>::iterator ccItr = allLiteralsEverSeen.find(*pItr);
			if (ccItr == allLiteralsEverSeen.end()) {
				error = true;
			} else {
				set<int> & checkIn = ccItr->second;
				set<int>::iterator cspItr = currSP.begin();
				const set<int>::iterator cspEnd = currSP.end();
				for (; cspItr != cspEnd; ++cspItr) {
					if (checkIn.find(*cspItr) == checkIn.end()) {
						error = true;
						break;
					}
				}
			}
		}
	}
#endif


					list<ScheduleNode*> pointsOfConflict;
					set<ScheduleNode*, ScheduleNodeLT> pocSet;
					set<ScheduleNode*, ScheduleNodeLT> exempt;
					double committedPenalty = 0.0;
					ScheduleNode* lastConflict;				
					int spID = -1;
	
					penalisePreconditions(activeDeletes, nextExistingSN, nextExistingSN->getPreconditions(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties, debugEval);
					removeInvariants(activeInvariants, nextExistingSN, nextExistingSN->getInvariants());
					
					penaliseNumericPreconditions(fluentTracking, nextExistingSN, nextExistingSN->getNumericPreconditions(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties, debugEval);
					removeFluentInvariants(activeFluentInvariants, nextExistingSN, RPGBuilder::getMentioned(nextExistingSN->getOp()->getID()));
	
					penaliseDeleteEffects(activeInvariants, nextExistingSN, nextExistingSN->getDeleteEffects(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties);
					penaliseNumericEffects(activeFluentInvariants, nextExistingSN, nextExistingSN->getNumericEffects(), penalty, committedPenalty, onlyPenalise, false, exempt, alreadyPenalised, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties);
	
					addDeletes(activeDeletes, nextExistingSN, nextExistingSN->getDeleteEffects());
					addAdds(activeDeletes, nextExistingSN->getAddEffects());
					addNumerics(fluentTracking, nextExistingSN, nextExistingSN->getNumericEffects());

#ifdef PRECONDITIONPARANOIA
	{
		
		list<Literal*> & effs = nextExistingSN->getAddEffects();

		list<Literal*>::iterator pItr = effs.begin();
		const list<Literal*>::iterator pEnd = effs.end();

		for (; pItr != pEnd; ++pItr) {
			map<Literal*, set<int>, LitLT>::iterator ccItr = allLiteralsEverSeen.find(*pItr);
			
			if (ccItr == allLiteralsEverSeen.end()) {
				allLiteralsEverSeen[*pItr] = set<int>();
				ccItr = allLiteralsEverSeen.find(*pItr);
			} 
			
			set<int> & insertInto = ccItr->second;

			set<int>::iterator cspItr = currSP.begin();
			const set<int>::iterator cspEnd = currSP.end();
			for (; cspItr != cspEnd; ++cspItr) {
				insertInto.insert(*cspItr);
			}

		}
	}
#endif
				}
			}
			
		}
	}


	if (GlobalSchedule::alternativeGP) {
				list<ScheduleNode*> pointsOfConflict;
				set<ScheduleNode*, ScheduleNodeLT> pocSet;
				ScheduleNode* lastConflict;				
				int spID = -1;

		penaliseGP(fluentTracking, activeDeletes, penalty, onlyPenalise, pointsOfConflict, pocSet, spID, lastConflict, false, false, anyPenalties, debugEval);
	}

#ifdef PRECONDITIONPARANOIA
	if (debugEval || error) {
		allLiteralsEverSeen = initialState;
		
		//allFluentsEverSeen = fluents;
#else
	if (debugEval) {
#endif
		list<TimelinePoint*>::iterator exStart = c.timeline.begin();
		const list<TimelinePoint*>::iterator exEnd = c.timeline.end();
	
		++exStart; // skip dummy timeline point for initial state
	
		double prevTime = -1.0;
		for (; exStart != exEnd; ++exStart) {
			ScheduleNode * const nextExistingSN = *((*exStart)->after);
#ifdef PRECONDITIONPARANOIA

			set<int> & currSP = nextExistingSN->getSubProblem();

#endif
			
			if (nextExistingSN->tilNode()) {
			
				cout << (*exStart)->timestamp << ": timed initial literal\n";
				
				cout << "\tAdds:\n";
				{
					list<Literal*> & tList = nextExistingSN->getAddEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
				cout << "\tDeletes:\n";
				{
					list<Literal*> & tList = nextExistingSN->getDeleteEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
				
				{
		
					list<Literal*> & effs = nextExistingSN->getAddEffects();
			
					list<Literal*>::iterator pItr = effs.begin();
					const list<Literal*>::iterator pEnd = effs.end();
			
					for (; pItr != pEnd; ++pItr) {
						map<Literal*, set<int>, LitLT>::iterator ccItr = allLiteralsEverSeen.find(*pItr);
						
						if (ccItr == allLiteralsEverSeen.end()) {
							allLiteralsEverSeen[*pItr] = set<int>();
							ccItr = allLiteralsEverSeen.find(*pItr);
						}
						set<int> & insertInto = ccItr->second;
			
						for (int i = 0; i < spCount; ++i) {
							insertInto.insert(i);
						}
			
					}
				}
			
			} else {
			
				if (nextExistingSN->isStart()) {
					instantiatedOp* const currOp = nextExistingSN->getOp();
					// if the next existing schedule node is an action start
					cout << (*exStart)->timestamp << ": " << *currOp << " start\n";
					
				} else {
					instantiatedOp* const currOp = nextExistingSN->getOp();
					cout << (*exStart)->timestamp << ": " << *currOp << " end\n";
					// if the next existing schedule node is an action end
					//if (debugEval) cout << (*exStart)->timestamp << ": " << *currOp << " finished\n";
				}
	//			assert((*exStart)->timestamp != prevTime);
				prevTime = (*exStart)->timestamp;
				cout << "\tSubproblems:";
				{
					set<int> & sps = nextExistingSN->getSubProblem();
					set<int>::iterator spsItr = sps.begin();
					const set<int>::iterator spsEnd = sps.end();
					for (; spsItr != spsEnd; ++spsItr) cout << " " << *spsItr;
					cout << "\n";
				}
				cout << "\tAdds:\n";
				{
					list<Literal*> & tList = nextExistingSN->getAddEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
				cout << "\tDeletes:\n";
				{
					list<Literal*> & tList = nextExistingSN->getDeleteEffects();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
	
				cout << "\tPres:\n";
				{
					list<Literal*> & tList = nextExistingSN->getPreconditions();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
	
				cout << "\tInvs:\n";
				{
					list<Literal*> & tList = nextExistingSN->getInvariants();
					list<Literal*>::iterator tlItr = tList.begin();
					const list<Literal*>::iterator tlEnd = tList.end();
	
					for (; tlItr != tlEnd; ++tlItr) {
					cout << "\t\t" << *(*tlItr) << "\n";
					}
				}
			
#ifdef PRECONDITIONPARANOIA
	{
		list<Literal*> & pres = nextExistingSN->getPreconditions();

		list<Literal*>::iterator pItr = pres.begin();
		const list<Literal*>::iterator pEnd = pres.end();

		for (; pItr != pEnd; ++pItr) {
			map<Literal*, set<int>, LitLT>::iterator ccItr = allLiteralsEverSeen.find(*pItr);
			if (ccItr == allLiteralsEverSeen.end()) {
				cout << "Cannot find literal " << *(*pItr) << " with ID " << (*pItr)->getID() << " for any subproblem\n";
				assert(false);
			} else {
				set<int> & checkIn = ccItr->second;
				set<int>::iterator cspItr = currSP.begin();
				const set<int>::iterator cspEnd = currSP.end();
				for (; cspItr != cspEnd; ++cspItr) {
					if (checkIn.find(*cspItr) == checkIn.end()) {
						cout << "Cannot find literal " << *(*pItr) << " for subproblem " << *cspItr <<"\n";
						assert(false);
					}
				}
			}
		}
	}
#endif
#ifdef PRECONDITIONPARANOIA
	{
		
		list<Literal*> & effs = nextExistingSN->getAddEffects();

		list<Literal*>::iterator pItr = effs.begin();
		const list<Literal*>::iterator pEnd = effs.end();

		for (; pItr != pEnd; ++pItr) {
			map<Literal*, set<int>, LitLT>::iterator ccItr = allLiteralsEverSeen.find(*pItr);
			
			if (ccItr == allLiteralsEverSeen.end()) {
				allLiteralsEverSeen[*pItr] = set<int>();
				ccItr = allLiteralsEverSeen.find(*pItr);
			}
			set<int> & insertInto = ccItr->second;

			set<int>::iterator cspItr = currSP.begin();
			const set<int>::iterator cspEnd = currSP.end();
			for (; cspItr != cspEnd; ++cspItr) {
				insertInto.insert(*cspItr);
			}

		}
	}
#endif	

			}
		}
	}

	if (debugEval) {
		c.printEvents();
		cout << "Penalty for schedule = " << penalty << "\n";
		if (anyPenalties) cout << "Any penalties = " << *anyPenalties << "\n";
	}
	return penalty;

};

void ScheduleEvaluatorStandard::updatePenaltiesForPreconditions(map<Literal*, NodeOpCount, LitLT> & activeDeletes, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, ScheduleNode* thisNode, list<Literal*> & precs) {


	list<Literal*>::iterator pItr = precs.begin();
	const list<Literal*>::iterator pEnd = precs.end();

	for (; pItr != pEnd; ++pItr) {
		Literal * const currLit = *pItr;
		map<Literal*, NodeOpCount, LitLT>::iterator adItr = activeDeletes.find(currLit);
		if (adItr != activeDeletes.end()) {
			map<int, set<ScheduleNode*, ScheduleNodeLT> > & currSet = adItr->second.count;
			map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csItr = currSet.begin();
			const map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csEnd = currSet.end();

			for (; csItr != csEnd; ++csItr) {
				set<ScheduleNode*, ScheduleNodeLT>::iterator snItr = csItr->second.begin();
				const set<ScheduleNode*, ScheduleNodeLT>::iterator snEnd = csItr->second.end();

				for (; snItr != snEnd; ++snItr) {

					if (alreadyPenalised.find(*snItr) == alreadyPenalised.end()) {
						//const int ocCount = currOC.count.size();
						//penalty += ocCount * getWeight(checkOp, op);
						
						
						alreadyPenalised.insert(*snItr);
						set<int> & tbSet = thisNode->getSubProblem();
						set<int>::iterator tbsnItr = tbSet.begin();
						const set<int>::iterator tbsnEnd = tbSet.end();

						for (; tbsnItr != tbsnEnd; ++tbsnItr) {

							if (*tbsnItr != csItr->first) {
								incrementWeight(*tbsnItr, csItr->first);
								incrementActionWeight(thisNode->getOp()->getID());
								incrementActionWeight((*snItr)->getOp()->getID());
							}
														

						}

						
	
					}
				}
			}
			
		} 
	}

};

void ScheduleEvaluatorStandard::updatePenaltiesForDeleteEffects(map<Literal*, NodeOpCount, LitLT> & activeInvariants, set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised, ScheduleNode* thisNode, list<Literal*> & deletes) {

	list<Literal*>::iterator dItr = deletes.begin();
	const list<Literal*>::iterator dEnd = deletes.end();

	for (; dItr != dEnd; ++dItr) {
		Literal * const currLit = *dItr;
		map<Literal*, NodeOpCount, LitLT>::iterator aiItr = activeInvariants.find(currLit);
		if (aiItr != activeInvariants.end()) {
			map<int, set<ScheduleNode*, ScheduleNodeLT> > & currSet = aiItr->second.count;
			map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csItr = currSet.begin();
			const map<int, set<ScheduleNode*, ScheduleNodeLT> >::iterator csEnd = currSet.end();

			for (; csItr != csEnd; ++csItr) {
				set<ScheduleNode*, ScheduleNodeLT>::iterator snItr = csItr->second.begin();
				const set<ScheduleNode*, ScheduleNodeLT>::iterator snEnd = csItr->second.end();

				for (; snItr != snEnd; ++snItr) {

				
					if (alreadyPenalised.find(*snItr) == alreadyPenalised.end()) {
						//const int ocCount = currOC.count.size();
						//penalty += ocCount * getWeight(checkOp, op);
						
						alreadyPenalised.insert(*snItr);
						set<int> & tbSet = thisNode->getSubProblem();
						set<int>::iterator tbsnItr = tbSet.begin();
						const set<int>::iterator tbsnEnd = tbSet.end();
						
						for (; tbsnItr != tbsnEnd; ++tbsnItr) {
							if (*tbsnItr != csItr->first) {
								incrementWeight(*tbsnItr, csItr->first);
								incrementActionWeight(thisNode->getOp()->getID());
								incrementActionWeight((*snItr)->getOp()->getID());
							}

						}
					}
				}
			}
		}
	}

	


}

void ScheduleEvaluatorStandard::updatePenalties(CandidateSchedule & c) {

	if (GlobalSchedule::globalVerbosity & 1) cout << "** Updating penalties\n";
	const bool debug = (GlobalSchedule::globalVerbosity & 8); 
	map<Literal*, NodeOpCount, LitLT> activeDeletes;
	map<Literal*, NodeOpCount, LitLT> activeInvariants;

	list<TimelinePoint*>::iterator exStart = c.timeline.begin();
	const list<TimelinePoint*>::iterator exEnd = c.timeline.end();

	++exStart; // skip dummy timeline point for initial state

	map<ScheduleNode*, set<ScheduleNode*, ScheduleNodeLT>, ScheduleNodeLT> allAlreadyPenalised;

	for (; exStart != exEnd; ++exStart) {
		ScheduleNode * const nextExistingSN = *((*exStart)->after);
		
		if (nextExistingSN->isStart()) {
			set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised = allAlreadyPenalised[nextExistingSN];
			instantiatedOp* const currOp = nextExistingSN->getOp();
			if (debug) cout << "Start node for " << *currOp << "\n";
			// if the next existing schedule node is an action start
			updatePenaltiesForPreconditions(activeDeletes, alreadyPenalised, nextExistingSN, nextExistingSN->getPreconditions());
			updatePenaltiesForDeleteEffects(activeInvariants, alreadyPenalised, nextExistingSN, nextExistingSN->getDeleteEffects());
			addDeletes(activeDeletes, nextExistingSN, nextExistingSN->getDeleteEffects());
			addAdds(activeDeletes, nextExistingSN->getAddEffects());

			updatePenaltiesForPreconditions(activeDeletes, alreadyPenalised, nextExistingSN, nextExistingSN->getInvariants());
			addInvariants(activeInvariants, nextExistingSN, nextExistingSN->getInvariants());
		} else {
			set<ScheduleNode*, ScheduleNodeLT> & alreadyPenalised = allAlreadyPenalised[nextExistingSN->getPartner()];
			instantiatedOp* const currOp = nextExistingSN->getOp();
			// if the next existing schedule node is an action end
			if (debug) cout << "End node for " << *currOp << "\n";
			updatePenaltiesForPreconditions(activeDeletes, alreadyPenalised, nextExistingSN, nextExistingSN->getPreconditions());
			removeInvariants(activeInvariants, nextExistingSN, nextExistingSN->getInvariants());
			updatePenaltiesForDeleteEffects(activeInvariants, alreadyPenalised, nextExistingSN, nextExistingSN->getDeleteEffects());
			addDeletes(activeDeletes, nextExistingSN, nextExistingSN->getDeleteEffects());
			addAdds(activeDeletes, nextExistingSN->getAddEffects());
		}
	}

	globalPenalty = evaluateSchedule(c, 0, debug);

}

void ScheduleEvaluatorStandard::keepLeastCost(pair<int, VAL::time_spec> & opA, double & layerA, const pair<int, VAL::time_spec> & opB, const double & layerB) {

	const bool debugKLC=false;

	if (layerB > layerA) return;

	//assert(fabs(layerB - layerA) < 0.0005);

	map<int, double>::iterator itrA = actionWeights.find(opA.first);
	if (itrA == actionWeights.end()) return;

	map<int, double>::iterator itrB = actionWeights.find(opB.first);
	if (itrB == actionWeights.end()) {
		if (debugKLC) cout << "Keeping lower cost achiever " << opB.first << " rather than " << opA.first << " with cost 0 rather than " << itrA->second << "\n";
		opA = opB;
		layerA = layerB;
		return;
	}
	
	if (itrB->second < itrA->second) {
		if (debugKLC) cout << "Keeping lower cost achiever " << opB.first << " rather than " << opA.first << " with cost " << itrB->second << " rather than " << itrA->second << "\n";
		opA = opB;
		layerA = layerB;
	}

};

double ScheduleEvaluatorStandard::costFrom(ScheduleNode* a, ScheduleNode * b, const int & spID) {

	double toReturn = 0.0;

	set<int> & aSet = a->getSubProblem();
	set<int> & bSet = b->getSubProblem();

	set<int>::iterator asItr = aSet.begin();
	const set<int>::iterator asEnd = aSet.end();

	for (; asItr != asEnd; ++asItr) {

		const int currA = *asItr;

		if (!(GlobalSchedule::approximatePenalties & 2) || currA != spID) {
			set<int>::iterator bsItr = bSet.begin();
			const set<int>::iterator bsEnd = bSet.end();
	
			for (; bsItr != bsEnd; ++bsItr) {
				if (currA != *bsItr) toReturn += getWeight(currA, *bsItr);
			}
		}
	}

	return toReturn;
};

}

