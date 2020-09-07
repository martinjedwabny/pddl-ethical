
#include "random.h"

BEGIN_HSPS_NAMESPACE

// #define CHECK_PRECONDITION_EVALUATOR

void RandomInstanceName::write(std::ostream& s, unsigned int c) const {
  if (!context_is_problem(c)) {
    s << 'R' << n << '-' << o << '-' << p << '-' << q << '-' << g
      << '-' << (neg ? 1 : 0) << (cns ? 1 : 0) << (flp ? 1 : 0)
      << (var ? 1 : 0);
    if (v_id > 0) s << "-V" << v_id;
  }
  if (!context_is_problem(c) && !context_is_domain(c)) s << '-';
  if (!context_is_domain(c)) s << "I" << id;
}

void RandomWalkName::write(std::ostream& s, unsigned int c) const {
  base_name->write(s, c);
  write_char_escaped(s, '-', c);
  s << rnd;
  write_char_escaped(s, '_', c);
  s << forward_steps;
  write_char_escaped(s, '_', c);
  s << backward_steps;
}

const char* atom_prefix = "P";
const char* neg_atom_prefix = "not-P";
const char* action_prefix = "A";

RandomInstance::RandomInstance
(index_type n, index_type o, index_type p, index_type q, index_type g,
 bool var, bool neg, bool flp, bool cns, RNG& r)
  : Instance(),
    param_n(n),
    param_o(o),
    param_p(p),
    param_q(q),
    param_g(g),
    model_variable(var),
    model_negative(neg || flp),
    model_flip(flp),
    model_consistent(cns || !neg),
    v_id(0),
    v_values(1, 0),
    rng(r)
{
  id = r.seed_value();
  name = new RandomInstanceName(id, param_n, param_o, param_p, param_q,
				param_g, model_variable, model_negative,
				model_flip, model_consistent);
}

RandomInstance::RandomInstance
(index_type n, index_type p, index_type q, index_type g, bool var, bool neg,
 bool flp, bool cns, RNG& r)
  : Instance(),
    param_n(n),
    param_o(0),
    param_p(p),
    param_q(q),
    param_g(g),
    model_variable(var),
    model_negative(neg || flp),
    model_flip(flp),
    model_consistent(cns || !neg),
    v_id(0),
    v_values(1, 0),
    rng(r)
{
  param_o = ((long)floor(param_n*log(param_g)))+1;
  id = r.seed_value();
  name = new RandomInstanceName(id, param_n, param_o, param_p, param_q,
				param_g, model_variable, model_negative,
				model_flip, model_consistent);
}

void RandomInstance::select_fixed_atom_set
(Instance& ins, index_set& s, index_type p, RNG& r)
{
  if (s.length() + p > ins.n_atoms()) {
    std::cerr << "error: can't select " << s.length() << " + " << p
	      << " atoms from " << ins.n_atoms() << std::endl;
    exit(255);
  }
  index_type c = ins.n_atoms() - s.length();
  while (p > 0) {
    index_type i = r.random() % c;
    index_type k = 0;
    for (k = 0; i > 0; k++) {
      assert(k < ins.n_atoms());
      if (!s.contains(k)) i -= 1;
    }
    s.insert(k);
    p -= 1;
    c -= 1;
  }
}

void RandomInstance::select_variable_atom_set
(Instance& ins, index_set& s, index_type p, RNG& r)
{
  for (index_type k = 0; k < ins.n_atoms(); k++)
    if ((r.random() % ins.n_atoms()) < p) s.insert(k);
}

void RandomInstance::select_non_empty_atom_set
(Instance& ins, index_set& s, index_type p, RNG& r)
{
  for (index_type k = 0; k < ins.n_atoms(); k++)
    if ((r.random() % ins.n_atoms()) < p) s.insert(k);
  if (s.length() == 0) s.insert(r.random() % ins.n_atoms());
}

void RandomInstance::generate_action(Instance::Action& a)
{
  if (model_negative && model_consistent) {
    index_set p0;
    if (model_variable)
      rng.select_variable_set(p0, param_p, n_atoms() / 2);
    else
      rng.select_fixed_set(p0, param_p, n_atoms() / 2);
    for (index_type i = 0; i < p0.length(); i++)
      if ((rng.random() % 2) == 0) {
	a.pre.insert(pos_index(p0[i]));
      }
      else {
	a.pre.insert(neg_index(p0[i]));
      }
  }
  else {
    if (model_variable)
      rng.select_variable_set(a.pre, param_p, n_atoms());
    else
      rng.select_fixed_set(a.pre, param_p, n_atoms());
  }

  if (model_flip) {
    for (index_type i = 0; i < a.pre.length(); i++) {
      assert(a.pre[i] < n_atoms());
      index_type neg_pi = atoms[a.pre[i]].neg;
      assert(neg_pi != no_such_index);
      a.del.insert(a.pre[i]);
      a.add.insert(neg_pi);
    }
  }
  else if (model_negative) {
    if (model_variable)
      rng.select_non_empty_variable_set(a.add, param_q, n_atoms());
    else
      rng.select_fixed_set(a.add, param_q, n_atoms());
    for (index_type i = 0; i < a.add.length(); i++)
      a.del.insert(negate(a.add[i]));
  }
  else {
    index_set f;
    if (model_variable)
      rng.select_non_empty_variable_set(f, param_q, n_atoms());
    else
      rng.select_fixed_set(f, param_q, n_atoms());
    for (index_type i = 0; i < f.length(); i++) {
      if (rng.random() % 2)
	a.add.insert(f[i]);
      else
	a.del.insert(f[i]);
    }
  }
}

void RandomInstance::generate_domain() {
  atoms.clear();
  actions.clear();
  for (index_type k = 0; k < param_n; k++) {
    Instance::Atom& a = new_atom(new EnumName(atom_prefix, k + 1));
    if (model_negative) {
      index_type i = a.index;
      Instance::Atom& neg_a =
	new_atom(new EnumName(neg_atom_prefix, k + 1));
      atoms[i].neg = neg_a.index;
      neg_a.neg = i;
    }
  }
  for (index_type k = 0; k < param_o; k++) {
    generate_action(new_action(new EnumName(action_prefix, k + 1)));
  }
  clear_cross_reference();
}

void RandomInstance::generate_problem() {
  index_set g;
  if (model_negative && model_consistent) {
    index_set g0;
    rng.select_fixed_set(g0, param_g, param_n);
    for (index_type i = 0; i < g0.length(); i++)
      if ((rng.random() % 2) == 0)
	g.insert(pos_index(g0[i]));
      else
	g.insert(neg_index(g0[i]));
  }
  else {
    rng.select_fixed_set(g, param_g, n_atoms());
  }
  for (index_type k = 0; k < param_n; k++) {
    bool v = rng.random() % 2;
    if (model_negative) {
      atoms[pos_index(k)].init = v;
      atoms[neg_index(k)].init = !v;
    }
    else {
      if (g.contains(k)) atoms[k].init = false;
      else atoms[k].init = v;
    }
  }
  for (index_type k = 0; k < g.length(); k++)
    atoms[g[k]].goal = true;
}

void RandomInstance::set_variation_values(const cost_vec& v)
{
  v_values = v;
}

void RandomInstance::set_variation_type_1
(unsigned long min, unsigned long max, unsigned long div)
{
  assert(max > min);
  for (unsigned long i = min; i < max; i++) {
    v_values.append(R_TO_N(i,div));
  }
}

void RandomInstance::generate_variation(RNG& r)
{
  assert(v_values.length() > 1);
  v_id = r.seed_value();
  for (index_type k = 0; k < n_actions(); k++) {
    unsigned int i = (r.random() % v_values.length());
    actions[k].cost = v_values[i];
    actions[k].dur = v_values[i];
  }
  ((RandomInstanceName*)name)->set_variation_id(v_id);
}

RandomWalk2::RandomWalk2(Instance& ins, const ACF& c, RNG& r)
  : instance(ins), cost(c), rng(r), state(false, ins.n_atoms()),
    acc(0), evaluator(0), allowed(true, ins.n_actions()), app(0)
{
  for (index_type k = 0; k < instance.n_actions(); k++)
    allowed[k] = instance.actions[k].sel;
  app = new index_type[instance.n_actions()];
  reset();
  restart();
}

RandomWalk2::~RandomWalk2()
{
  delete [] app;
}

void RandomWalk2::disallow_action(index_type a)
{
  assert(a < instance.n_actions());
  allowed[a] = false;
}

index_type RandomWalk2::n_allowed_actions()
{
  return allowed.count(true);
}

const bool_vec& RandomWalk2::current_state()
{
  return state;
}

NTYPE RandomWalk2::estimated_branching_factor()
{
  return R_TO_N(n_successors, n_steps);
}

NTYPE RandomWalk2::estimated_deadend_frequency()
{
  return R_TO_N(n_deadends, n_steps);
}

NTYPE RandomWalk2::estimated_cost_to_depth()
{
  return (sum_cost / sum_depth);
}

void RandomWalk2::restart()
{
  for (index_type k = 0; k < instance.n_atoms(); k++)
    state[k] = instance.atoms[k].init;
  acc = 0;
}

void RandomWalk2::reset()
{
  n_successors = 0;
  n_deadends = 0;
  n_steps = 0;
  sum_cost = 0;
  sum_depth = 0;
}

bool RandomWalk2::step()
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "step: input state = ";
  instance.write_atom_set(std::cerr, state);
  std::cerr << std::endl;
#endif
  n_steps += 1;
  index_type c = 0;
  if (evaluator) {
    c = evaluator->eval(state, allowed, app, c);
#ifdef CHECK_PRECONDITION_EVALUATOR
    assert(c < instance.n_actions());
    index_set s1;
    for (index_type k = 0; k < c; k++) {
      assert(app[k] < instance.n_actions());
      s1.insert(app[k]);
    }
    index_set s0;
    for (index_type k = 0; k < instance.n_actions(); k++) {
      bool a = true;
      for (index_type i = 0; (i < instance.actions[k].pre.length()) && a; i++)
	if (!state[instance.actions[k].pre[i]]) a = false;
      if (a) {
	s0.append(k);
      }
    }
    assert(s0 == s1);
#endif
  }
  else {
    for (index_type k = 0; k < instance.n_actions(); k++) if (allowed[k]) {
      bool a = true;
      for (index_type i = 0; (i < instance.actions[k].pre.length()) && a; i++)
	if (!state[instance.actions[k].pre[i]]) a = false;
      if (a) {
	app[c] = k;
	c += 1;
      }
    }
  }
  if (c == 0) {
    n_deadends += 1;
    return false;
  }
  n_successors += c;
  index_type j = rng.random_in_range(ergodic ? c + 1 : c);
  if (ergodic && (j == c)) return true;
  assert(j < c);
  assert(app[j] < instance.n_actions());
  assert(allowed[app[j]]);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "step: selected action = "
	    << instance.actions[app[j]].name
	    << std::endl;
#endif
  for (index_type i = 0; i < instance.actions[app[j]].del.length(); i++) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "step: DELETE "
	      << instance.atoms[instance.actions[app[j]].del[i]].name
	      << std::endl;
#endif
    state[instance.actions[app[j]].del[i]] = false;
  }
  for (index_type i = 0; i < instance.actions[app[j]].add.length(); i++) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "step: ADD "
	      << instance.atoms[instance.actions[app[j]].add[i]].name
	      << std::endl;
#endif
    state[instance.actions[app[j]].add[i]] = true;
  }
#ifdef TRACE_PRINT_LOTS
  std::cerr << "step: resulting state = ";
  instance.write_atom_set(std::cerr, state);
  std::cerr << std::endl;
#endif
  acc += cost(j);
  return true;
}

count_type RandomWalk2::walk(count_type steps)
{
  count_type d = 0;
  while (d < steps) {
    bool ok = step();
    if (!ok) {
      if (irreducible) {
	restart();
      }
      else {
	return d;
      }
    }
    d += 1;
  }
  return d;
}

index_type RandomWalk2::sample_exp(index_type avg_depth, index_type max_depth)
{
  restart();
  index_type d = 0;
  index_type l = 0;
  while (true) {
    index_type j = rng.random_in_range(avg_depth);
    if ((j == 0) || (l > max_depth)) {
      sum_cost += acc;
      sum_depth += d;
      n_walks += 1;
      return d;
    }
    bool ok = step();
    d += 1;
    if (!ok) {
      if (irreducible) {
	restart();
	d = 0;
      }
      else {
	sum_cost += acc;
	sum_depth += d;
	n_walks += 1;
	return d;
      }
    }
    l += 1;
  }
}

index_type RandomWalk2::sample_bin
(index_type avg_depth, index_type max_depth)
{
  restart();
  index_type d = 0;
  double p = ((double)avg_depth/(double)max_depth);
  index_type l = rng.binomial_sample(max_depth, p);
  while (l > 0) {
    bool ok = step();
    d += 1;
    if (!ok) {
      if (irreducible) {
	restart();
	d = 0;
      }
      else {
	return d;
      }
    }
    l -= 1;
  }
  return d;
}

void RandomWalk::forward_applicable_actions
(const index_set& state, index_set& acts)
{
  acts.clear();
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (state.contains(instance.actions[k].pre)) acts.insert(k);
}

void RandomWalk::reverse_applicable_actions
(const index_set& state, Heuristic& inc, index_set& acts)
{
  acts.clear();
  for (index_type k = 0; k < instance.n_actions(); k++)
    if ((state.count_common(instance.actions[k].del) == 0) &&
	(state.count_common(instance.actions[k].add) > 0)) {
      index_set s(state);
      s.subtract(instance.actions[k].add);
      s.insert(instance.actions[k].pre);
      bool ok = true;
      if (INFINITE(inc.eval(s))) ok = false;
      for (index_type i = 0; (i < instance.n_invariants()) && ok; i++)
	if (!instance.eval_invariant_in_partial_state(s, instance.invariants[i])) ok = false;
      if (ok) acts.insert(k);
    }
}

bool RandomWalk::walk_forward(count_type n, index_set& state, RNG& rng)
{
  while (n > 0) {
    index_set app;
    forward_applicable_actions(state, app);
    if (app.empty()) return false;
    index_type i = rng.random_in_range(app.length());
    state.subtract(instance.actions[app[i]].del);
    state.insert(instance.actions[app[i]].add);
    n -= 1;
  }
  return true;
}

bool RandomWalk::walk_reverse
(count_type n, index_set& state, RNG& rng, Heuristic& inc)
{
  while (n > 0) {
    index_set app;
    reverse_applicable_actions(state, inc, app);
    if (app.empty()) return false;
    index_type i = rng.random_in_range(app.length());
    state.subtract(instance.actions[app[i]].add);
    state.insert(instance.actions[app[i]].pre);
    n -= 1;
  }
  return true;
}

Instance* RandomWalk::next_forward_instance
(count_type forward_steps, RNG& rng)
{
  unsigned long rnd = rng.seed_value();
  index_set s(instance.init_atoms);
  if (walk_forward(forward_steps, s, rng)) {
    Instance* new_instance = new Instance(instance);
    new_instance->set_initial(s);
    new_instance->name =
      new RandomWalkName(new_instance->name, rnd, forward_steps, 0);
    return new_instance;
  }
  else {
    return 0;
  }
}

Instance* RandomWalk::next_instance
(count_type forward_steps, count_type reverse_steps, RNG& rng)
{
  unsigned long rnd = rng.seed_value();
  Instance* new_instance = next_forward_instance(forward_steps, rng);
  if (new_instance == 0) return 0;
  Statistics stats;
  CostTable* inc = new CostTable(*new_instance, stats);
  inc->compute_H2(ZeroACF());
  index_set g(new_instance->goal_atoms);
  bool ok = walk_reverse(reverse_steps, g, rng, *inc);
  delete inc;
  if (ok) {
    new_instance->set_goal(g);
    new_instance->name = new RandomWalkName(new_instance->name, rnd,
					    forward_steps, reverse_steps);
    return new_instance;
  }
  else {
    delete new_instance;
    return 0;
  }
}

END_HSPS_NAMESPACE
