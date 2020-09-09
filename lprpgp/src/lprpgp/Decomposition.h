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

#ifndef __DECOMPOSITION
#define __DECOMPOSITION

#include <list>
using std::list;

#include <vector>
using std::vector;

#include "RPGBuilder.h"

namespace Planner {

class SubProblem {

public:
	list<Literal*> goals;
	list<int> goalFluents;

	LiteralSet initialState;
	vector<double> initialFluents;

	SubproblemRPG* rpg;

	SubProblem(	const list<Literal*> & goalsIn, const list<int> & gfIn,
			const LiteralSet & isIn, const vector<double> & ifIn,
			SubproblemRPG* rpgIn) : goals(goalsIn), goalFluents(gfIn), initialState(isIn), initialFluents(ifIn), rpg(rpgIn) {};


	~SubProblem() {
		delete rpg;
	}

};

class Decomposition {

private:

	static vector<SubProblem* > subproblems;

public:

	static void performDummyDecomposition();
	static int howMany() { return subproblems.size();};
	static SubProblem* getSubproblem(const int & i) { return subproblems[i]; };

};

};

#endif
