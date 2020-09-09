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
		//int Pvar;
		ActData() : Avar(-1) {};
		ActData(const int & a, const int & p) : Avar(a) {};
	};

	struct VarData {
		int Vvar;
		int SUvar;
		VarData() : Vvar(-1), SUvar(-1) {};
		VarData(const int & v, const int & s): Vvar(v), SUvar(s) {};
	};

	struct ActionEffectUponVariable {
		double change;
		bool isAssignment;
		ActionEffectUponVariable() : change(-1.0), isAssignment(false) {};
		ActionEffectUponVariable(const double & c, const bool & a) : change(c), isAssignment(a) {};
	};

private:
	
	int varCount;
	int colCount;
	lprec *lp;
	map<int, bool> applicableSoFar;
	map<double, vector<VarData>, EpsilonComp > minMaxVars;
	map<double, map<int, ActData> > layerActionVariables;
	map<int, pair<int, int> > boundedActs;
	MinimalState * startingState; 
	
	static vector<int> interestingVariables;
	static int ivCount;
	static vector<bool> fluentAction;
	static vector<map<int, ActionEffectUponVariable> > preprocessedData;
	static int actCount;
public:
	MILPRPG(MinimalState * forKShot, const vector<double> & layerZero);
	~MILPRPG();
	void addApplicableActions(vector<int> * const propPrec, vector<int> * const numPrec);
	void fillMinMaxFluentTable(vector<double> & toFill);

};

}

#endif
