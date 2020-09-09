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

#include "Decomposition.h"

#include "ptree.h"
#include <FlexLexer.h>
#include "instantiation.h"
#include "SimpleEval.h"
#include "DebugWriteController.h"
#include "typecheck.h"
#include "TIM.h"

#include "GlobalSchedule.h"

//#include "graphconstruct.h"
//#include "PartialPlan.h"

using namespace TIM;
using namespace Inst;
using namespace VAL;

namespace Planner {


vector<SubProblem* > Decomposition::subproblems;

void Decomposition::performDummyDecomposition() {


	list<Literal*> goals;
	list<int> goalFluents;
	LiteralSet initialState;
	vector<double> initialFluents;

	{
		list<Literal*>::iterator goalItr = RPGBuilder::getLiteralGoals().begin();
		list<Literal*>::iterator goalEnd = RPGBuilder::getLiteralGoals().end();
//		cout << "SubProblem 0 has goals:";
		for (; goalItr != goalEnd; ++goalItr) {
//			cout << " " << (*goalItr)->getID();
			goals.push_back(*goalItr);
			
		}
//		cout << "\n";
	}

	{
		list<pair<int, int> > & numGoals = RPGBuilder::getNumericRPGGoals();
		list<pair<int, int> >::iterator goalItr = numGoals.begin();
		list<pair<int, int> >::iterator goalEnd = numGoals.end();
		for (; goalItr != goalEnd; ++goalItr) {

			goalFluents.push_back(goalItr->first);
			if (goalItr->second != -1) goalFluents.push_back(goalItr->second);
		}
	}


	SubproblemRPG* spRPG = RPGBuilder::pruneRPG(goals, goalFluents, initialState, initialFluents);

	SubProblem* singleSP = new SubProblem(goals, goalFluents, initialState, initialFluents, spRPG);
	
	subproblems = vector<SubProblem*>(1);
	subproblems[0] = singleSP;
};

};
