
#include "resource.h"

BEGIN_HSPS_NAMESPACE

// #define TRACE_PRINT_LOTS

BasicResourceState::BasicResourceState
(Instance& i, estimator_vec& est)
  : instance(i),
    estimators(est),
    amt_consumed(0, instance.n_resources())
{
  // done?
}

BasicResourceState::BasicResourceState
(Instance& i, estimator_vec& est, const amt_vec& ac)
  : instance(i),
    estimators(est),
    amt_consumed(ac)
{
  // done
}

BasicResourceState::BasicResourceState
(const BasicResourceState& s)
  : instance(s.instance),
    estimators(s.estimators),
    amt_consumed(s.amt_consumed)
{
  // done?
}

BasicResourceState::~BasicResourceState()
{
  // done
}

// return amount available for consumption/use of resource r
NTYPE BasicResourceState::available_for_consumption(index_type r) const
{
  return available(r);
}

NTYPE BasicResourceState::available(index_type r) const
{
  return instance.resources[r].init - amt_consumed[r];
}

// check if amount available meets requirements for executing action a
bool BasicResourceState::applicable(Instance::Action& a)
{
  for (index_type r = 0; r < instance.n_resources(); r++)
    if (available(r) < a.req(r)) return false;
  return true;
}

// check if resources available for consumption are sufficient to reach goals
bool BasicResourceState::sufficient_consumable(const index_set& s)
{
  for (index_type r = 0; r < instance.n_resources(); r++) if (estimators[r]) {
    if (estimators[r]->eval(s) > available_for_consumption(r)) return false;
  }
  return true;
}

bool BasicResourceState::sufficient_consumable(const bool_vec& s)
{
  for (index_type r = 0; r < instance.n_resources(); r++) if (estimators[r]) {
    if (estimators[r]->eval(s) > available_for_consumption(r)) return false;
  }
  return true;
}

// apply action (deduct consumed resources from remaining)
void BasicResourceState::apply(Instance::Action& a)
{
  for (index_type r = 0; r < instance.n_resources(); r++)
    amt_consumed[r] += a.cons[r];
}

bool BasicResourceState::is_root()
{
  for (index_type r = 0; r < instance.n_resources(); r++)
    if (amt_consumed[r] > 0) return false;
  return true;
}

int BasicResourceState::compare(const BasicResourceState& s)
{
  return amt_consumed.compare(s.amt_consumed, instance.n_resources());
}

index_type BasicResourceState::hash()
{
  return amt_consumed.hash(instance.n_resources());
}

BasicResourceState* BasicResourceState::new_state()
{
  return new BasicResourceState(instance, estimators);
}

BasicResourceState* BasicResourceState::copy()
{
  return new BasicResourceState(*this);
}

void BasicResourceState::write(std::ostream& s)
{
  for (index_type r = 0; r < instance.n_resources(); r++) {
    if (r > 0) s << ", ";
    s << instance.resources[r].name << "=" << available(r);
  }
}


RegressionResourceState::RegressionResourceState
(Instance& i, estimator_vec& est)
  : BasicResourceState(i, est),
    max_required(0, instance.n_resources())
{
  // done
}

RegressionResourceState::RegressionResourceState
(Instance& i, estimator_vec& est, const amt_vec& ac)
  : BasicResourceState(i, est, ac),
    max_required(0, instance.n_resources())
{
  // done
}

RegressionResourceState::RegressionResourceState
(const RegressionResourceState& s)
  : BasicResourceState(s),
    max_required(s.max_required)
{
  // done
}

RegressionResourceState::~RegressionResourceState()
{
  // done
}

NTYPE RegressionResourceState::available(index_type r) const
{
  return instance.resources[r].init;
}

NTYPE RegressionResourceState::available_for_consumption(index_type r) const
{
  return MIN(instance.resources[r].init - amt_consumed[r],
	     instance.resources[r].init - max_required[r]);
}

// check if amount available meets requirements for executing action a
bool RegressionResourceState::applicable(Instance::Action& a)
{
  for (index_type r = 0; r < instance.n_resources(); r++) {
    if (available(r) < a.req(r)) return false;
    if (available_for_consumption(r) < a.cons[r]) return false;
  }
  return true;
}

// check if amount available meets requirements for executing action a
// in the presence of concurrent actions c; note that c can potentially
// contain multiple copies of actions
bool RegressionResourceState::applicable
(Instance::Action& a, const index_vec& c)
{
  amt_vec cr(0, instance.n_resources());
  amt_vec cc(0, instance.n_resources());
  for (index_type r = 0; r < instance.n_resources(); r++) {
    NTYPE cr = 0;
    NTYPE cc = 0;
    for (index_type k = 0; k < c.length(); k++) {
      cr += instance.actions[c[k]].req(r);
      cc += instance.actions[c[k]].cons[r];
    }
    if (available(r) < (cr + a.req(r))) return false;
    if (available_for_consumption(r) < (cc + a.cons[r])) return false;
  }
  return true;
}

bool RegressionResourceState::applicable
(Instance::Action& a, const index_cost_vec& c)
{
  amt_vec cr(0, instance.n_resources());
  amt_vec cc(0, instance.n_resources());
  for (index_type r = 0; r < instance.n_resources(); r++) {
    NTYPE cr = 0;
    NTYPE cc = 0;
    for (index_type k = 0; k < c.length(); k++) {
      cr += instance.actions[c[k].first].req(r);
      cc += instance.actions[c[k].first].cons[r];
    }
    if (available(r) < (cr + a.req(r))) return false;
    if (available_for_consumption(r) < (cc + a.cons[r])) return false;
  }
  return true;
}

bool RegressionResourceState::applicable
(Instance::Action& a, const index_cost_vec& c1, const index_vec& c2)
{
  amt_vec cr(0, instance.n_resources());
  amt_vec cc(0, instance.n_resources());
  for (index_type r = 0; r < instance.n_resources(); r++) {
    NTYPE cr = 0;
    NTYPE cc = 0;
    for (index_type k = 0; k < c1.length(); k++) {
      cr += instance.actions[c1[k].first].req(r);
      cc += instance.actions[c1[k].first].cons[r];
    }
    for (index_type k = 0; k < c2.length(); k++) {
      cr += instance.actions[c2[k]].req(r);
      cc += instance.actions[c2[k]].cons[r];
    }
    if (available(r) < (cr + a.req(r))) return false;
    if (available_for_consumption(r) < (cc + a.cons[r])) return false;
  }
  return true;
}

// check if resources available for consumption are sufficient to reach goals
bool RegressionResourceState::sufficient_consumable(const index_set& s)
{
  return BasicResourceState::sufficient_consumable(s);
}

bool RegressionResourceState::sufficient_consumable(const bool_vec& s)
{
  return BasicResourceState::sufficient_consumable(s);
}

bool RegressionResourceState::sufficient_consumable
(const index_set& s, const Instance::Action& cact)
{
  for (index_type r = 0; r < instance.n_resources(); r++) if (estimators[r]) {
    NTYPE afc = MIN(instance.resources[r].init - amt_consumed[r],
		    instance.resources[r].init - MAX(cact.req(r),
						     max_required[r]));
    if (estimators[r]->eval(s) > afc) return false;
  }
  return true;
}

bool RegressionResourceState::sufficient_consumable
(const bool_vec& s, const Instance::Action& cact)
{
  for (index_type r = 0; r < instance.n_resources(); r++) if (estimators[r]) {
    NTYPE afc = MIN(instance.resources[r].init - amt_consumed[r],
		    instance.resources[r].init - MAX(cact.req(r),
						     max_required[r]));
    if (estimators[r]->eval(s) > afc) return false;
  }
  return true;
}

// bool RegressionResourceState::sufficient_consumable
// (const index_set& s, const amt_vec& creq)
// {
//   for (index_type r = 0; r < instance.n_resources(); r++) if (estimators[r]) {
//     NTYPE afc = MIN(instance.resources[r].init - amt_consumed[r],
// 		    instance.resources[r].init - MAX(creq[r],
// 						     max_required[r]));
//     if (estimators[r]->eval(s) > afc) {
// #ifdef TRACE_PRINT_LOTS
//       std::cerr << "resource " << instance.resources[r].name
// 		<< " overconsumed: avail. for cons. = min("
// 		<< instance.resources[r].init << " - " << amt_consumed[r]
// 		<< ", "<< instance.resources[r].init << " - max("
// 		<< creq[r] << ", " << max_required[r] << ")) = "
// 		<< afc << ", eval(";
//       instance.write_atom_set(std::cerr, s);
//       std::cerr << ") = " << estimators[r]->eval(s) << std::endl;
// #endif
//       return false;
//     }
//   }
//   return true;
// }

// bool RegressionResourceState::sufficient_consumable
// (const bool_vec& s, const amt_vec& creq)
// {
//   for (index_type r = 0; r < instance.n_resources(); r++) if (estimators[r]) {
//     NTYPE afc = MIN(instance.resources[r].init - amt_consumed[r],
// 		    instance.resources[r].init - MAX(creq[r],
// 						     max_required[r]));
//     if (estimators[r]->eval(s) > afc) {
// #ifdef TRACE_PRINT_LOTS
//       std::cerr << "resource " << instance.resources[r].name
// 		<< " overconsumed: avail. for cons. = min("
// 		<< instance.resources[r].init << " - " << amt_consumed[r]
// 		<< ", "<< instance.resources[r].init << " - max("
// 		<< creq[r] << ", " << max_required[r] << ")) = "
// 		<< afc << ", eval(";
//       instance.write_atom_set(std::cerr, s);
//       std::cerr << ") = " << estimators[r]->eval(s) << std::endl;
// #endif
//       return false;
//     }
//   }
//   return true;
// }

bool RegressionResourceState::sufficient_consumable
(const index_set& s, const index_cost_vec& cacts)
{
  for (index_type r = 0; r < instance.n_resources(); r++)
    if (estimators[r]) {
      NTYPE creq = 0;
      for (index_type k = 0; k < cacts.length(); k++)
	creq += instance.actions[cacts[k].first].req(r);
      // afc = available for consumption
      NTYPE afc = MIN(instance.resources[r].init - amt_consumed[r],
		      instance.resources[r].init - MAX(creq, max_required[r]));
      if (estimators[r]->eval(s) > afc) {
#ifdef TRACE_PRINT_LOTS
	std::cerr << "resource " << instance.resources[r].name
		  << " overconsumed: avail. for cons. = min("
		  << instance.resources[r].init << " - " << amt_consumed[r]
		  << ", "<< instance.resources[r].init << " - max("
		  << creq << ", " << max_required[r] << ")) = "
		  << afc << ", eval(";
	instance.write_atom_set(std::cerr, s);
	std::cerr << ") = " << estimators[r]->eval(s) << std::endl;
#endif
	return false;
      }
    }
  return true;
}

bool RegressionResourceState::sufficient_consumable
(const bool_vec& s, const index_cost_vec& cacts)
{
  for (index_type r = 0; r < instance.n_resources(); r++)
    if (estimators[r]) {
      NTYPE creq = 0;
      for (index_type k = 0; k < cacts.length(); k++)
	creq += instance.actions[cacts[k].first].req(r);
      // afc = available for consumption
      NTYPE afc = MIN(instance.resources[r].init - amt_consumed[r],
		      instance.resources[r].init - MAX(creq, max_required[r]));
      if (estimators[r]->eval(s) > afc) {
#ifdef TRACE_PRINT_LOTS
	std::cerr << "resource " << instance.resources[r].name
		  << " overconsumed: avail. for cons. = min("
		  << instance.resources[r].init << " - " << amt_consumed[r]
		  << ", "<< instance.resources[r].init << " - max("
		  << creq << ", " << max_required[r] << ")) = "
		  << afc << ", eval(";
	instance.write_atom_set(std::cerr, s);
	std::cerr << ") = " << estimators[r]->eval(s) << std::endl;
#endif
	return false;
      }
    }
  return true;
}

// apply action (deduct consumed resources from remaining)
void RegressionResourceState::apply(Instance::Action& a)
{
  for (index_type r = 0; r < instance.n_resources(); r++) {
    amt_consumed[r] += a.cons[r];
    max_required[r] = MAX(max_required[r], a.req(r));
  }
}

void RegressionResourceState::reserve_as_required(const amt_vec& req)
{
  for (index_type r = 0; r < instance.n_resources(); r++)
    max_required[r] = MAX(max_required[r], req[r]);
}

bool RegressionResourceState::is_root()
{
  for (index_type r = 0; r < instance.n_resources(); r++)
    if ((amt_consumed[r] > 0) || (max_required[r] > 0)) return false;
  return true;
}

int RegressionResourceState::compare(const RegressionResourceState& s)
{
  int i = amt_consumed.compare(s.amt_consumed, instance.n_resources());
  if (i != 0) return i;
  return max_required.compare(s.max_required, instance.n_resources());
}

index_type RegressionResourceState::hash()
{
  return (amt_consumed.hash(instance.n_resources()) +
	  max_required.hash(instance.n_resources()));
}

RegressionResourceState* RegressionResourceState::new_state()
{
  return new RegressionResourceState(instance, estimators);
}

RegressionResourceState* RegressionResourceState::copy()
{
  return new RegressionResourceState(*this);
}

void RegressionResourceState::write(std::ostream& s)
{
  for (index_type r = 0; r < instance.n_resources(); r++) {
    if (r > 0) s << ", ";
    s << instance.resources[r].name << "=" << available_for_consumption(r);
  }
}

END_HSPS_NAMESPACE
