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

vector<bool> MILPRPG::fluentAction;
vector<bool> MILPRPG::everAssigned;
vector<list<MILPRPG::ActionEffectUponVariable> > MILPRPG::preprocessedData;
int MILPRPG::actCount;


MILPRPG::MILPRPG(MinimalState * f, const vector<double> & layerZero) : varCount(layerZero.size()), colCount(varCount), layerCount(0), startingState(f) {
	
	if (!varCount) return;

	static bool donePreprocessing = false;

	if (!donePreprocessing) {

		everAssigned = vector<bool>(varCount);
		for (int v = 0; v < varCount; ++v) everAssigned[v] = false;

		vector<RPGBuilder::RPGNumericEffect> & numericEffs = RPGBuilder::getNumericEffs();
		vector<list<int> > & actEffs = RPGBuilder::getActionsToRPGNumericStartEffects();

		actCount = actEffs.size();
		fluentAction = vector<bool>(actCount);
		preprocessedData = vector<list<ActionEffectUponVariable> >(actCount);

		for (int i = 0; i < actCount; ++i) {
			fluentAction[i] = false;
			list<int>::iterator effItr = actEffs[i].begin();
			const list<int>::iterator effEnd = actEffs[i].end();

			list<ActionEffectUponVariable> & into = preprocessedData[i] = list<ActionEffectUponVariable>();

			for (; effItr != effEnd; ++effItr) {
				RPGBuilder::RPGNumericEffect & currEff = numericEffs[*effItr];
				const int mods = currEff.fluentIndex;				
				double quickEff = currEff.evaluate(layerZero);
				into.push_back(ActionEffectUponVariable(mods,quickEff, currEff.isAssignment));
				if (currEff.isAssignment) everAssigned[mods] = true;
				if (RPGBuilder::getDominanceConstraints()[mods] != E_METRICTRACKING) fluentAction[i] = true;
			}

		}

		donePreprocessing = true;
	}

	lp = make_lp(0,varCount);
	
	set_verbose(lp,3);


/*	for (int i = 1; i <= varCount; ++i) {

		ostringstream namestream;
		namestream << *(RPGBuilder::getPNE(i - 1));
		namestream << "[0.000]";
		string asString = namestream.str();
		set_col_name(lp, i, const_cast<char*>(asString.c_str()));
	}
*/
	minMaxVars = vector<VarData>(varCount);

	for (int i = 0; i < varCount; ++i) {
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

	}

	for (int i = 0; i < varCount; ++i) {
		if (everAssigned[i]) {
			add_constraintex(lp, 0, 0, 0, LE, 0);
			minMaxVars[i].assignRow = get_Nrows(lp);
		}
	}


//	print_lp(lp);

};



MILPRPG::~MILPRPG() {

	delete_lp(lp);

};

void MILPRPG::addApplicableActions(vector<int> * const propPrec, vector<int> * const numPrec) {

	const bool debugAdd = false;

	++layerCount;

	{
		const int ailSize = propPrec->size();
		for (int i = 0; i < ailSize; ++i) {
			if (fluentAction[i] && !((*propPrec)[i]) && !((*numPrec)[i])) {
				if (applicableSoFar.insert(i).second && RPGBuilder::isInteresting(i, startingState->first, startingState->startedActions)) {

					const int howMany = RPGBuilder::howManyTimes(i, *startingState);
					if (howMany > 0) {
	
						ActData & newActData = layerActionVariables[i];
						add_columnex(lp,0,0,0);
						const int newCol = ++colCount;
//						assert(colCount == get_Ncolumns(lp));
						newActData.Avar = newCol;
						set_bounds(lp,newCol,0,0);
						if (howMany != INT_MAX) {
							newActData.kShot = howMany;
//							if (howMany == 1) set_binary(lp, newCol, TRUE);
						} else {
							newActData.kShot = -1;
						}

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
				++currUpBo;
				set_upbo(lp, colNo, currUpBo);
//				cout << "Upper bound on column " << colNo << " now " << currUpBo << "\n";
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
		solve(lp);
	 	const double mv = get_objective(lp);
		toFill[i] = mv;
//		set_upbo(lp, optVar[0], mv);
//		cout << "Result = " << toFill[i] << "\n";
//		print_solution(lp,varCount);
//		print_lp(lp);
//		cout << "Minimising " << *(RPGBuilder::getPNE(i)) << "\n";

		set_minim(lp);
//		maximise[0] = 1.0;
		set_obj_fnex(lp, 1, maximise, optVar);
		solve(lp);
		const double mvTwo = get_objective(lp);
		toFill[i + varCount] = -mvTwo;
//		set_lowbo(lp, optVar[0], mvTwo);
//		cout << "Minimum Result = " << -(toFill[i + varCount]) << "\n";
		} else {
			toFill[i] = 0.0;
			toFill[i + varCount] = 0.0;
		}
	}

}

};
