
#include "sas_heuristic.h"
#include "graph.h"

BEGIN_HSPS_NAMESPACE

SASHeuristic::~SASHeuristic()
{
  // done
}

void SASHeuristic::set_trace_level(int level)
{
  trace_level = level;
}

NTYPE SASHeuristic::eval(const partial_state& s)
{
  Heuristic::eval_count += 1;
  return 0;
}

void SASHeuristic::write_eval
(const partial_state& s, std::ostream& st, char* p, bool e)
{
  if (p) st << p << " ";
  st << eval(s);
  if (e) st << std::endl;
}

void SASHeuristic::store(const partial_state& s, NTYPE v)
{
  std::cerr << "error: no implementation of SASHeuristic::store"
	    << std::endl;
  exit(255);
}

NTYPE SASHeuristic::eval_to_bound(const partial_state& s, NTYPE bound)
{
  return eval(s);
}

NTYPE SASCostACF::operator()(index_type a) const
{
  return instance.actions[a].cost;
}

void MaxH::set_trace_level(int level)
{
  trace_level = level;
  for (index_type k = 0; k < components.length(); k++)
    components[k]->set_trace_level(level);
}

index_type MaxH::new_component(SASHeuristic* h)
{
  components.append(h);
  return (components.length() - 1);
}

NTYPE MaxH::eval(const partial_state& s)
{
  Heuristic::eval_count += 1;
  NTYPE v_max = 0;
  for (index_type k = 0; k < components.length(); k++) {
    if (trace_level > 2) {
      std::cerr << "MaxH: evaluating component " << k << "..." << std::endl;
    }
    NTYPE v_comp = components[k]->eval(s);
    if (trace_level > 2) {
      std::cerr << "MaxH: estimate " << k << " = " << v_comp << std::endl;
    }
    v_max = MAX(v_max, v_comp);
  }
  return v_max;
}

NTYPE MaxH::min_component(const partial_state& s)
{
  NTYPE v_min = POS_INF;
  for (index_type k = 0; k < components.length(); k++) {
    NTYPE v_comp = components[k]->eval(s);
    v_min = MIN(v_min, v_comp);
  }
  return v_min;
}

void MaxH::component_values(const partial_state& s, cost_vec& vals)
{
  vals.assign_value(ZERO, components.length());
  for (index_type k = 0; k < components.length(); k++)
    vals[k] = components[k]->eval(s);
}

NTYPE MaxH::eval_to_bound(const partial_state& s, NTYPE bound)
{
  Heuristic::eval_count += 1;
  NTYPE v_max = 0;
  for (index_type k = 0; k < components.length(); k++) {
    if (trace_level > 2) {
      std::cerr << "MaxH: evaluating component " << k << "..." << std::endl;
    }
    NTYPE v_comp = components[k]->eval(s);
    if (trace_level > 2) {
      std::cerr << "MaxH: estimate " << k << " = " << v_comp << std::endl;
    }
    v_max = MAX(v_max, v_comp);
    if (v_max > bound) return v_max;
  }
  return v_max;
}

NTYPE AddH::eval(const partial_state& s)
{
  Heuristic::eval_count += 1;
  NTYPE v_add = 0;
  for (index_type k = 0; k < components.length(); k++) {
    if (trace_level > 2) {
      std::cerr << "AddH: evaluating component " << k << "..." << std::endl;
    }
    NTYPE v_comp = components[k]->eval(s);
    if (trace_level > 2) {
      std::cerr << "AddH: estimate " << k << " = " << v_comp << std::endl;
    }
    v_add += v_comp;
  }
  return v_add;
}

NTYPE AddH::eval_to_bound(const partial_state& s, NTYPE bound)
{
  Heuristic::eval_count += 1;
  NTYPE v_add = 0;
  for (index_type k = 0; k < components.length(); k++) {
    if (trace_level > 2) {
      std::cerr << "AddH: evaluating component " << k << "..." << std::endl;
    }
    NTYPE v_comp = components[k]->eval(s);
    if (trace_level > 2) {
      std::cerr << "AddH: estimate " << k << " = " << v_comp << std::endl;
    }
    v_add += v_comp;
    if (v_add > bound) return v_add;
  }
  return v_add;
}

MaxAddH::MaxAddH
(const sas_heuristic_vec& base_h, const index_set_vec& groups)
  : base(0), val(0), n_base(0), n_add(0), add(0), n_max(0),
    extra(0), n_extra(0)
{
  index_set_vec nsg;
  index_set   uncov;
  uncov.fill(base_h.length());
  for (index_type k = 0; k < groups.length(); k++) {
    if (groups[k].length() > 1) {
      nsg.append(groups[k]);
      uncov.subtract(groups[k]);
    }
  }

  n_base = base_h.length();
  base = new SASHeuristic*[n_base];
  for (index_type k = 0; k < n_base; k++) base[k] = base_h[k];
  val = new NTYPE[n_base];
  n_max = nsg.length();
  if (n_max > 0) {
    n_add = new index_type[n_max];
    add = new index_type*[n_max];
    for (index_type k = 0; k < n_max; k++) {
      n_add[k] = groups[k].length();
      add[k] = new index_type[n_add[k]];
      for (index_type i = 0; i < n_add[k]; i++) add[k][i] = groups[k][i];
    }
  }
  if (uncov.length() > 0) {
    n_extra = uncov.length();
    extra = new index_type[n_extra];
    for (index_type k = 0; k < uncov.length(); k++)
      extra[k] = uncov[k];
  }
  if (trace_level > 0) {
    std::cerr << "MaxAddH: " << n_base << " components in "
	      << n_max << " additive groups and " << n_extra
	      << " single" << std::endl;
  }
}

NTYPE MaxAddH::eval(const partial_state& s)
{
  for (index_type k = 0; k < n_base; k++)
    val[k] = base[k]->eval(s);
  NTYPE v_max = 0;
  for (index_type k = 0; k < n_max; k++) {
    NTYPE v_add = 0;
    for (index_type i = 0; i < n_add[k]; i++)
      v_add += val[add[k][i]];
    v_max = MAX(v_max, v_add);
  }
  for (index_type k = 0; k < n_extra; k++) {
    v_max = MAX(v_max, val[extra[k]]);
  }
  return v_max;
}

void FwdSASHAdapter::set_trace_level(int level)
{
  trace_level = level;
  sas_heuristic.set_trace_level(level);
}

NTYPE FwdSASHAdapter::eval(const index_set& s)
{
  partial_state sas_s;
  sas_instance.make_complete_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval(sas_s);
}

NTYPE FwdSASHAdapter::eval(const bool_vec& s)
{
  partial_state sas_s;
  sas_instance.make_complete_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval(sas_s);
}

void FwdSASHAdapter::write_eval
(const index_set& s, std::ostream& st, char* p, bool e)
{
  partial_state sas_s;
  sas_instance.make_complete_state(s, sas_s);
  if (p) st << p << " ";
  instance.write_atom_set(st, s);
  st << " -> ";
  sas_instance.write_partial_state(st, sas_s);
  st << " ";
  sas_heuristic.write_eval(sas_s, st, 0, e);
}

void FwdSASHAdapter::write_eval
(const bool_vec& s, std::ostream& st, char* p, bool e)
{
  partial_state sas_s;
  sas_instance.make_complete_state(s, sas_s);
  if (p) st << p << " ";
  instance.write_atom_set(st, s);
  st << " -> ";
  sas_instance.write_partial_state(st, sas_s);
  st << " ";
  sas_heuristic.write_eval(sas_s, st, 0, e);
}

NTYPE FwdSASHAdapter::incremental_eval
(const index_set& s, index_type i_new)
{
  partial_state sas_s;
  sas_instance.make_complete_state(s, sas_s);
  sas_instance.map_to_partial_state(i_new, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " + " << instance.atoms[i_new].name << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval(sas_s);
}

NTYPE FwdSASHAdapter::incremental_eval
(const bool_vec& s, index_type i_new)
{
  partial_state sas_s;
  sas_instance.make_complete_state(s, sas_s);
  sas_instance.map_to_partial_state(i_new, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " + " << instance.atoms[i_new].name << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval(sas_s);
}

NTYPE FwdSASHAdapter::eval_to_bound(const index_set& s, NTYPE bound)
{
  partial_state sas_s;
  sas_instance.make_complete_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval_to_bound(sas_s, bound);
}

NTYPE FwdSASHAdapter::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  partial_state sas_s;
  sas_instance.make_complete_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval_to_bound(sas_s, bound);
}

void FwdSASHAdapter::store(const index_set& s, NTYPE val, bool opt)
{
  partial_state sas_s;
  sas_instance.make_complete_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "storing ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << " = " << val << std::endl;
  }
  sas_heuristic.store(sas_s, val);
}

void FwdSASHAdapter::store(const bool_vec& s, NTYPE val, bool opt)
{
  partial_state sas_s;
  sas_instance.make_complete_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "storing ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << " = " << val << std::endl;
  }
  sas_heuristic.store(sas_s, val);
}

void BwdSASHAdapter::set_trace_level(int level)
{
  trace_level = level;
  sas_heuristic.set_trace_level(level);
}

NTYPE BwdSASHAdapter::eval(const index_set& s)
{
  partial_state sas_s;
  sas_instance.make_partial_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval(sas_s);
}

NTYPE BwdSASHAdapter::eval(const bool_vec& s)
{
  partial_state sas_s;
  sas_instance.make_partial_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval(sas_s);
}

void BwdSASHAdapter::write_eval
(const index_set& s, std::ostream& st, char* p, bool e)
{
  partial_state sas_s;
  sas_instance.make_partial_state(s, sas_s);
  if (p) st << p << " ";
  instance.write_atom_set(st, s);
  st << " -> ";
  sas_instance.write_partial_state(st, sas_s);
  st << " ";
  sas_heuristic.write_eval(sas_s, st, 0, e);
}

void BwdSASHAdapter::write_eval
(const bool_vec& s, std::ostream& st, char* p, bool e)
{
  partial_state sas_s;
  sas_instance.make_partial_state(s, sas_s);
  if (p) st << p << " ";
  instance.write_atom_set(st, s);
  st << " -> ";
  sas_instance.write_partial_state(st, sas_s);
  st << " ";
  sas_heuristic.write_eval(sas_s, st, 0, e);
}

NTYPE BwdSASHAdapter::incremental_eval
(const index_set& s, index_type i_new)
{
  partial_state sas_s;
  sas_instance.make_partial_state(s, sas_s);
  sas_instance.map_to_partial_state(i_new, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " + " << instance.atoms[i_new].name << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval(sas_s);
}

NTYPE BwdSASHAdapter::incremental_eval
(const bool_vec& s, index_type i_new)
{
  partial_state sas_s;
  sas_instance.make_partial_state(s, sas_s);
  sas_instance.map_to_partial_state(i_new, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " + " << instance.atoms[i_new].name << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval(sas_s);
}

NTYPE BwdSASHAdapter::eval_to_bound(const index_set& s, NTYPE bound)
{
  partial_state sas_s;
  sas_instance.make_partial_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval_to_bound(sas_s, bound);
}

NTYPE BwdSASHAdapter::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  partial_state sas_s;
  sas_instance.make_partial_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "evaluating ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << "..." << std::endl;
  }
  return sas_heuristic.eval_to_bound(sas_s, bound);
}

void BwdSASHAdapter::store(const index_set& s, NTYPE val, bool opt)
{
  partial_state sas_s;
  sas_instance.make_partial_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "storing ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << " = " << val << std::endl;
  }
  sas_heuristic.store(sas_s, val);
}

void BwdSASHAdapter::store(const bool_vec& s, NTYPE val, bool opt)
{
  partial_state sas_s;
  sas_instance.make_partial_state(s, sas_s);
  if (trace_level > 2) {
    std::cerr << "storing ";
    instance.write_atom_set(std::cerr, s);
    std::cerr << " -> ";
    sas_instance.write_partial_state(std::cerr, sas_s);
    std::cerr << " = " << val << std::endl;
  }
  sas_heuristic.store(sas_s, val);
}

NTYPE ToSASHAdapter::eval(const partial_state& s)
{
  index_set a;
  sas_instance.make_atom_set(s, a);
  return heuristic.eval(a);
}

NTYPE ToSASHAdapter::eval_to_bound(const partial_state& s, NTYPE bound)
{
  index_set a;
  sas_instance.make_atom_set(s, a);
  return heuristic.eval_to_bound(a, bound);
}

NTYPE SASReductionAdapter::eval(const partial_state& s)
{
  partial_state r(s, reduce_map);
  return heuristic.eval(r);
}

NTYPE SASReductionAdapter::eval_to_bound(const partial_state& s, NTYPE bound)
{
  partial_state r(s, reduce_map);
  return heuristic.eval_to_bound(r, bound);
}

void SASReductionAdapter::store(const partial_state& s, NTYPE v)
{
  partial_state r(s, reduce_map);
  heuristic.store(r, v);
}


NTYPE SASExpansionAdapter::eval(const partial_state& s)
{
  partial_state x(s, expand_map);
  return heuristic.eval(x);
}

NTYPE SASExpansionAdapter::eval_to_bound(const partial_state& s, NTYPE bound)
{
  partial_state x(s, expand_map);
  return heuristic.eval_to_bound(x, bound);
}

void SASExpansionAdapter::store(const partial_state& s, NTYPE v)
{
  partial_state x(s, expand_map);
  heuristic.store(x, v);
}

SASSeqProgState::SASSeqProgState
(SASInstance& i,
 const ACF& c,
 SASHeuristic& h)
  : instance(i),
    cost(c),
    heuristic(h),
    c_est(0),
    act(no_such_index),
    c_delta(0)
{
  // done
}

SASSeqProgState::SASSeqProgState
(SASInstance& i,
 const ACF& c,
 SASHeuristic& h,
 const partial_state& s0)
  : instance(i),
    cost(c),
    heuristic(h),
    state(s0),
    c_est(0),
    act(no_such_index),
    c_delta(0)
{
  reevaluate();
}

SASSeqProgState::SASSeqProgState(const SASSeqProgState& ssps)
  : instance(ssps.instance),
    cost(ssps.cost),
    heuristic(ssps.heuristic),
    state(ssps.state),
    c_est(ssps.c_est),
    act(ssps.act),
    c_delta(ssps.c_delta)
{
  // done
}

SASSeqProgState::SASSeqProgState(SASSeqProgState& p, index_type a)
  : instance(p.instance),
    cost(p.cost),
    heuristic(p.heuristic),
    state(p.state),
    c_est(0),
    act(a),
    c_delta(0)
{
  state.assign(instance.actions[a].post);
  c_est = heuristic.eval(state);
  c_delta = cost(act);
  set_predecessor(&p);
}

SASSeqProgState::~SASSeqProgState()
{
  // done
}

NTYPE SASSeqProgState::delta_cost()
{
  return c_delta;
}

NTYPE SASSeqProgState::est_cost()
{
  return c_est;
}

bool  SASSeqProgState::is_final()
{
  return state.implies(instance.goal_state);
}

bool  SASSeqProgState::is_max()
{
  return false;
}

NTYPE SASSeqProgState::expand(Search& s, NTYPE bound)
{
  NTYPE c_min = POS_INF;
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (state.implies(instance.actions[k].pre) &&
	state.implies(instance.actions[k].prv)) {
      SASSeqProgState* s_new = new SASSeqProgState(*this, k);
      if ((s_new->est_cost() + s_new->delta_cost()) <= bound) {
	NTYPE c_new = s.new_state(*s_new, bound - s_new->delta_cost()) +
	  s_new->delta_cost();
	if (s.done()) {
	  delete s_new;
	  return c_new;
	}
	else {
	  c_min = MIN(c_min, c_new);
	}
      }
      else {
	c_min = MIN(c_min, (s_new->est_cost() + s_new->delta_cost()));
      }
      delete s_new;
    }
  return c_min;
}

void SASSeqProgState::store(NTYPE val, bool opt)
{
  heuristic.store(state, val);
}

void SASSeqProgState::reevaluate()
{
  c_est = heuristic.eval(state);
}

int SASSeqProgState::compare(const State& s)
{
  SASSeqProgState& ssps = (SASSeqProgState&)s;
  if (state < ssps.state) {
    return -1;
  }
  else if (state > ssps.state) {
    return 1;
  }
  else {
    return 0;
  }
}

index_type SASSeqProgState::hash()
{
  return instance.state_hash_function.index(state);
}

State* SASSeqProgState::copy()
{
  return new SASSeqProgState(*this);
}

void SASSeqProgState::insert(Plan& p)
{
  if (act != no_such_index) {
    p.insert(act);
    p.advance(1);
  }
}

void SASSeqProgState::write(::std::ostream& s)
{
  instance.write_partial_state(s, state);
  s << " (delta = " << delta_cost() << ", est = " << est_cost() << ")";
}

void SASSeqProgState::write_plan(::std::ostream& s)
{
  if (act != no_such_index) {
    s << instance.actions[act].name;
  }
}

END_HSPS_NAMESPACE
