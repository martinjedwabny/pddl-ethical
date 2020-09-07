#ifndef RESOURCE_STATE_H
#define RESOURCE_STATE_H

#include "config.h"
#include "problem.h"
#include "heuristic.h"

BEGIN_HSPS_NAMESPACE

typedef lvector<Heuristic*> estimator_vec;

class BasicResourceState {
 protected:
  Instance& instance;
  estimator_vec& estimators;
  amt_vec   amt_consumed;

 public:
  BasicResourceState(Instance& i, estimator_vec& est);
  BasicResourceState(Instance& i, estimator_vec& est, const amt_vec& ac);
  BasicResourceState(const BasicResourceState& s);
  virtual ~BasicResourceState();

  // return amount available/available for consumption of resource r
  NTYPE available(index_type r) const;
  NTYPE available_for_consumption(index_type r) const;

  // check if amount available meets requirements for executing action a
  bool applicable(Instance::Action& a);

  // check if resources available for consumption are sufficient to reach goals
  bool sufficient_consumable(const index_set& s);
  bool sufficient_consumable(const bool_vec& s);

  // apply action (deduct consumed resources from remaining) - NON-UNDOABLE!
  void apply(Instance::Action& a);

  bool is_root();
  int compare(const BasicResourceState& s);
  index_type hash();

  BasicResourceState* new_state();
  BasicResourceState* copy();

  void write(std::ostream& s);
};

class RegressionResourceState : public BasicResourceState {
 protected:
  amt_vec max_required;

 public:
  RegressionResourceState(Instance& i, estimator_vec& est);
  RegressionResourceState(Instance& i, estimator_vec& est, const amt_vec& ac);
  RegressionResourceState(const RegressionResourceState& s);
  ~RegressionResourceState();

  NTYPE available(index_type r) const;
  NTYPE available_for_consumption(index_type r) const;

  // check if amount available meets requirements for executing action a
  bool applicable(Instance::Action& a);
  bool applicable(Instance::Action& a, const index_vec& c);
  bool applicable(Instance::Action& a, const index_cost_vec& c);
  bool applicable(Instance::Action& a,
		  const index_cost_vec& c1,
		  const index_vec& c2);

  bool sufficient_consumable(const index_set& s);
  bool sufficient_consumable(const bool_vec& s);
  bool sufficient_consumable(const index_set& s, const Instance::Action& cact);
  bool sufficient_consumable(const bool_vec& s, const Instance::Action& cact);
  // bool sufficient_consumable(const index_set& s, const amt_vec& creq);
  // bool sufficient_consumable(const bool_vec& s, const amt_vec& creq);
  bool sufficient_consumable(const index_set& s, const index_cost_vec& cacts);
  bool sufficient_consumable(const bool_vec& s, const index_cost_vec& cacts);

  // NON-UNDOABLE!
  void apply(Instance::Action& a);
  // record requirements of simultaneuos application of a set of actions
  void reserve_as_required(const amt_vec& req);

  bool is_root();
  int compare(const RegressionResourceState& s);
  index_type hash();

  RegressionResourceState* new_state();
  RegressionResourceState* copy();

  void write(std::ostream& s);
};

END_HSPS_NAMESPACE

#endif
