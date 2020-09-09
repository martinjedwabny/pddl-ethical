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

#ifndef MILPRPG_H
#define MILPRPG_H

#include "RPGBuilder.h"

#include <vector>
#include <map>

using std::vector;
using std::map;

#include "lp_lib.h"

namespace Planner {

class MinimalState;
class MILPRPG {

public:
	struct ActData {
		int Avar;
		int kShot;
		//int Pvar;
		ActData() : Avar(-1), kShot(-1) {};
		ActData(const int & a, const int & p) : Avar(a), kShot(p) {};
	};

	struct VarData {
		int Vvar;
		int consRow;
		int assignRow;
		double lastMin;
		double lastMax;
		VarData() : Vvar(-1), consRow(-1), assignRow(-1) {};
		VarData(const int & v, const int & s): Vvar(v), consRow(s) {};
	};

	struct ActionEffectUponVariable {
		int var;
		double change;
		bool isAssignment;
		ActionEffectUponVariable() : var(-1), change(-1.0), isAssignment(false) {};
		ActionEffectUponVariable(const int & v, const double & c, const bool & a) : var(v), change(c), isAssignment(a) {};
	};

private:
	
	int layerCount;

	MinimalState * startingState; 

	static int varCount;
	static lprec *lp;
	static vector<VarData> minMaxVars;
	static vector<ActData> layerActionVariables;
	static bool * appearedYet;
	static bool donePreprocessing;
	static vector<bool> fluentAction;
	static vector<int> localID;
	static vector<bool> everAssigned;
	static vector<list<ActionEffectUponVariable> > preprocessedData;
	static int ractCount;
	static int actCount;
public:
	MILPRPG(MinimalState * forKShot, const vector<double> & layerZero);
	~MILPRPG();
	void addApplicableActions(vector<int> * const propPrec, vector<int> * const numPrec);
	void fillMinMaxFluentTable(vector<double> & toFill);

};

}

#endif
