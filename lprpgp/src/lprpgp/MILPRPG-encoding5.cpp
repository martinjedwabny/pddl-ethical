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

using std::ostringstream;
using std::string;

namespace Planner {

int MILPRPG::varCount;
vector<bool> MILPRPG::fluentAction;
vector<bool> MILPRPG::everAssigned;
vector<list<MILPRPG::ActionEffectUponVariable> > MILPRPG::preprocessedData;
int MILPRPG::actCount;
int MILPRPG::ractCount;

lprec * MILPRPG::lp;
vector<MILPRPG::VarData> MILPRPG::minMaxVars;
vector<MILPRPG::ActData> MILPRPG::layerActionVariables;
bool * MILPRPG::appearedYet = 0;
bool MILPRPG::donePreprocessing = false;


vector<int> MILPRPG::localID;


MILPRPG::MILPRPG(MinimalState * f, const vector<double> & layerZero) : layerCount(0), startingState(f) {

	if (layerZero.empty()) {
		varCount = 0;
		return;
	}

	if (!donePreprocessing) {

		varCount = layerZero.size();
		everAssigned = vector<bool>(varCount);
		for (int v = 0; v < varCount; ++v) everAssigned[v] = false;

		vector<RPGBuilder::RPGNumericEffect> & numericEffs = RPGBuilder::getNumericEffs();
		vector<list<int> > & actEffs = RPGBuilder::getActionsToRPGNumericStartEffects();

		ractCount = actEffs.size();
		fluentAction = vector<bool>(ractCount);
		localID = vector<int>(ractCount);
		preprocessedData = vector<list<ActionEffectUponVariable> >(ractCount);

		appearedYet = new bool[ractCount];

		assert(appearedYet);

		int faCount = 0;
		int eaCount = 0;

		for (int i = 0; i < ractCount; ++i) {
			fluentAction[i] = false;
			
			const int howMany = RPGBuilder::howManyTimes(i, *startingState);

			if (howMany > 0) {

				list<int>::iterator effItr = actEffs[i].begin();
				const list<int>::iterator effEnd = actEffs[i].end();
	
				list<ActionEffectUponVariable> & into = preprocessedData[i] = list<ActionEffectUponVariable>();
	
				for (; effItr != effEnd; ++effItr) {
					RPGBuilder::RPGNumericEffect & currEff = numericEffs[*effItr];
					const int mods = currEff.fluentIndex;				
					double quickEff = currEff.evaluate(layerZero);
					into.push_back(ActionEffectUponVariable(mods,quickEff, currEff.isAssignment));
					if (currEff.isAssignment) {
						if (!everAssigned[mods]) ++eaCount;
						everAssigned[mods] = true;
					}
					if (RPGBuilder::getDominanceConstraints()[mods] != E_METRICTRACKING) {
						if (!fluentAction[i]) {
							localID[i] = faCount;
							++faCount;
						}
						fluentAction[i] = true;
					}
				}
			}
		}


	
		

		lp = make_lp(varCount + eaCount,varCount + faCount);
	
		set_verbose(lp,3);

		{
			static REAL maximise[1];
			maximise[0] = 1.0;
			static int optVar[1];		
			optVar[0] = 1;
	
			set_maxim(lp);
			set_obj_fnex(lp, 1, maximise, optVar);
		}

		minMaxVars = vector<VarData>(varCount);
	
		eaCount = 0;
		for (int i = 0; i < varCount; ++i) {
			minMaxVars[i].Vvar = i + 1;
			minMaxVars[i].consRow = i + 1;
			set_mat(lp, i+1, i+1, 1.0);
			set_rh(lp, i+1, 0.0);
			set_constr_type(lp, i+1, EQ);

			if (everAssigned[i]) {
				++eaCount;
				minMaxVars[i].assignRow = varCount + eaCount;
				set_constr_type(lp, varCount + eaCount, LE);
				set_rh(lp, varCount + eaCount, 0.0);
			}
		}
		actCount = faCount;
		layerActionVariables = vector<ActData>(faCount);
		faCount = 0;
		for (int i = 0; i < ractCount; ++i) {
			if (fluentAction[i]) {
				ActData & newActData = layerActionVariables[faCount];
				const int newCol = ++faCount;
//						assert(colCount == get_Ncolumns(lp));
				newActData.Avar = newCol;
				set_bounds(lp,newCol,0,0);

				list<ActionEffectUponVariable>::iterator effItr = preprocessedData[i].begin();
				const list<ActionEffectUponVariable>::iterator effEnd = preprocessedData[i].end();

				for (; effItr != effEnd; ++effItr) {
//							cout << "Have an action at column " << newCol << " which gives us " << effItr->change << " of " << *(RPGBuilder::getPNE(effItr->var)) <<" - " << *(RPGBuilder::getInstantiatedOp(i)) << "\n";
					set_mat(lp, minMaxVars[effItr->var].consRow, newCol, -(effItr->change));
//							cout << "Row " << minMaxVars[effItr->var].consRow << " column " << newCol << " = " << -(effItr->change) << "\n";
					if (effItr->isAssignment) {
						set_mat(lp, minMaxVars[effItr->var].assignRow, newCol, 1.0);
					}
				}
			}

		}

		donePreprocessing = true;

	}


/*	for (int i = 1; i <= varCount; ++i) {

		ostringstream namestream;
		namestream << *(RPGBuilder::getPNE(i - 1));
		namestream << "[0.000]";
		string asString = namestream.str();
		set_col_name(lp, i, const_cast<char*>(asString.c_str()));
	}
*/



	for (int i = 0; i < varCount; ++i) {
		set_rh(lp, i+1, layerZero[i]);
		minMaxVars[i].lastMin = layerZero[i];
		minMaxVars[i].lastMax = layerZero[i];
		if (everAssigned[i]) {
			set_rh(lp, minMaxVars[i].assignRow, 0);
		}
	}

	const int loopLim = actCount + varCount + 1;
	for (int i = varCount + 1; i < loopLim; ++i) {
		set_upbo(lp, i, 0);
	}

	assert(appearedYet);
	memset(appearedYet, 0, ractCount * sizeof(bool));
//	print_lp(lp);
	assert(appearedYet);
};



MILPRPG::~MILPRPG() {

//	delete_lp(lp);

};

void MILPRPG::addApplicableActions(vector<int> * const propPrec, vector<int> * const numPrec) {
	if (!varCount) return;
	assert(donePreprocessing);
	assert(appearedYet);

	const bool debugAdd = true;

	++layerCount;

	{
		for (int i = 0; i < ractCount; ++i) {
			if (!appearedYet[i] && fluentAction[i] && !((*propPrec)[i]) && !((*numPrec)[i])) {
				appearedYet[i] = true;
				ActData & newActData = layerActionVariables[localID[i]];

				if (RPGBuilder::isInteresting(i, startingState->first, startingState->startedActions)) {
					
					const int howMany = RPGBuilder::howManyTimes(i, *startingState);
					if (debugAdd) cout << "Now have fluent-modifying action: " << *(RPGBuilder::getInstantiatedOp(i)) << "\n";
					if (howMany > 0) {
						if (howMany != INT_MAX) {
							newActData.kShot = howMany;
						} else {
							newActData.kShot = -1;
						}
					} else {
						newActData.kShot = 0;
					}
				} else {
					newActData.kShot = 0;
				}
			}
		}
	}					

	{

		for (int a = 0; a < ractCount; ++a) {
			if (appearedYet[a]) {
				ActData & currData = layerActionVariables[localID[a]];
				int & colNo = currData.Avar;
				int & kShot = currData.kShot;
				int currUpBo = get_upbo(lp, colNo);
				if (currUpBo < kShot || kShot == -1) {
					++currUpBo;
					set_upbo(lp, colNo, currUpBo);
//				cout << "Upper bound on column " << colNo << " now " << currUpBo << "\n";
				}
			}
		}
	}

	for (int v = 0; v < varCount; ++v) {
		if (everAssigned[v]) {
			set_rh(lp, minMaxVars[v].assignRow, layerCount);
		}
	}
	
};

void MILPRPG::fillMinMaxFluentTable(vector<double> & toFill) {

	if (!varCount) return;
	cout << "|";
	cout.flush();
//	static char buf[256];
	static REAL maximise[1];
	maximise[0] = 1.0;
	static int optVar[1];
//	map<double, vector<VarData>, EpsilonComp>::iterator fItr = minMaxVars.find(ts);
//	assert(fItr != minMaxVars.end());

	// bear in mind caching results to maximise/minimise each var, and use those to seed past layers


	for (int i = 0; i < varCount; ++i) {
		if (RPGBuilder::getDominanceConstraints()[i] != E_METRICTRACKING) {
//		cout << "Maximising column " << minMaxVars[i].Vvar << "  " << *(RPGBuilder::getPNE(i)) << "   ";
		
		optVar[0] = minMaxVars[i].Vvar;
		
//		cout << "Objective function: maximise " << maximise[0] << " * var " << optVar[0] << "\n";
		set_maxim(lp);
//		maximise[0] = -1.0;
		set_obj_fnex(lp, 1, maximise, optVar);

//		REAL oldlowbo = get_lowbo(lp, minMaxVars[i].Vvar);
//		REAL oldupbo = get_upbo(lp, minMaxVars[i].Vvar);

//		set_lowbo(lp, minMaxVars[i].Vvar, minMaxVars[i].lastMax);
		solve(lp);
	 	const double mv = get_objective(lp);
		toFill[i] = mv;
		minMaxVars[i].lastMax = mv;
//		set_upbo(lp, optVar[0], mv);
//		cout << "Result = " << toFill[i] << "\n";
//		print_solution(lp,varCount);
//		print_lp(lp);
//		cout << "Minimising " << *(RPGBuilder::getPNE(i)) << "\n";

		set_minim(lp);
//		maximise[0] = 1.0;
		set_obj_fnex(lp, 1, maximise, optVar);
//		set_bounds(lp, minMaxVars[i].Vvar, 0, minMaxVars[i].lastMin);
		solve(lp);
		const double mvTwo = get_objective(lp);
		toFill[i + varCount] = -mvTwo;
		minMaxVars[i].lastMin = mvTwo;
//		set_lowbo(lp, optVar[0], mvTwo);
//		cout << "Minimum Result = " << -(toFill[i + varCount]) << "\n";
//		set_bounds(lp, minMaxVars[i].Vvar, oldlowbo, oldupbo);

		cout << *(RPGBuilder::getPNE(i)) << " in " << "[ " << mvTwo << "," << mv << "]\n";
		} else {
			toFill[i] = 0.0;
			toFill[i + varCount] = 0.0;
		}
	}

}

};
