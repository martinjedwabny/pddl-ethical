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

#ifndef LANDMARKSANALYSIS_H
#define LANDMARKSANALYSIS_H

#include "globals.h"

#include <vector>
#include <cassert>

using std::vector;

namespace Planner {

class MinimalState;

class LandmarksAnalysis
{

private:
    static vector<int> individualLandmarkFacts;
    static vector<int> landmarkFacts;
    static vector<int> factToLandmarkIndex;
    static vector<vector<vector<int> > > disjunctiveLandmarks;
    
    static void performSimpleGoalRegressionLandmarksAnalysis(const LiteralSet & initialFacts, const vector<double> & initialFluents,
                                                             set<int> & landmarksFound, list<list<set<int> > > & disjunctiveLandmarksFound);
    static void performZhuAndGivanLandmarksAnalysis(const LiteralSet & initialFacts, const vector<double> & initialFluents,
                                                    set<int> & landmarksFound, list<list<set<int> > > & disjunctiveLandmarksFound);
    
public:

    static bool disjunctiveZhuAndGivan;
    static bool disjunctiveRegressionLandmarks;
    static void performLandmarksAnalysis();
    static void addFactToState(MinimalState*, const int & fID);
    
    /** Perform landmarks analysis for a given relaxed planning graph.
     *
     * This function is called from the LP to find landmarks in the relaxed planning graph, by
     * regressing from the goal facts, using only the actions that have appeared in the RPG.  An
     * action <code>RPGBuilder::instantiatedOps[i]</code> is applicable if <code>propPrec[i]</code>
     * and <code>numPrec[i]</code> are both zero.
     *
     * @param factLayerZero  The state for layer zero of the planning graph.
     * @param propPrec       The number of unsatisfied propositional preconditions on each action
     * @param numPrec       The number of unsatisfied numeric preconditions on each action
     * @param mustUseNumericActions    If <code>true</code>, then only report landmarks or disjunctive landmarks
     *                                 where all the achievers have some numeric preconditions or effects, according
     *                                 to <code>MILPRPG::actionNeedsNumerics()</code>.
     * @param localIndividualLandmarks   The individual landmark facts found are added to this set
     * @param localDisjunctiveLandmarks  The disjunctive landmark facts found are added to this list.
     *                                   Each entry is a list of lists of individual landmarks,
     *                                   where all the facts in one such list must be met, for each
     *                                   entry, in order for the goals to have been reached.
     */
    static void localLandmarksAnalysis(const MinimalState & factLayerZero,
                                       vector<int> * const propPrec, vector<int> * const numPrec,
                                       const bool & mustUseNumericActions,
                                       set<int> & localIndividualLandmarks,
                                       list<list<set<int> > > & localDisjunctiveLandmarks);
    
    
    static const vector<int> & getIndividualLandmarkFacts() {
        return individualLandmarkFacts;        
    }
    
    static const vector<int> & getFactsInAnyLandmark() {
        return landmarkFacts;        
    }
    
    static const vector<vector<vector<int> > > & getDisjunctiveLandmarks() {
        return disjunctiveLandmarks;
    }

    static bool factIsALandmark(const int & fID) {
        assert(fID >= 0);
        assert((unsigned int) fID < factToLandmarkIndex.size());
        return (factToLandmarkIndex[fID] != -1);
    }
    
    static int factHasBeenSeen(const int & fID, MinimalState * theState);
};

};

#endif // LANDMARKSANALYSIS_H
