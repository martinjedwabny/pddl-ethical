
#include "idao.h"

BEGIN_HSPS_NAMESPACE

bool IDAO::alpha_cut = true;

IDAO::IDAO(Statistics& s, SearchResult& res)
  : MultiSearchAlgorithm(s, res),
    solved_tab(0),
    current_bound(0),
    iteration_count(0),
    iteration_limit(count_type_max),
    cycle_check(false),
    store_cost(false)
{
  // done
}

IDAO::IDAO(Statistics& s, SearchResult& res, HashTable* tab)
  : MultiSearchAlgorithm(s, res),
    solved_tab(tab),
    current_bound(0),
    iteration_count(0),
    iteration_limit(count_type_max),
    cycle_check(false),
    store_cost(false)
{
  // done
}

NTYPE IDAO::start(State& s, NTYPE b)
{
  iteration_count = 0;
  if (solved_tab) solved_tab->clear();
  reset();

  start_count();
  NTYPE val = main(s, b);
  stop_count();
  return val;
}

NTYPE IDAO::start(State& s)
{
  return start(s, s.est_cost());
}

NTYPE IDAO::resume(State& s, NTYPE b)
{
  start_count();
  NTYPE val = main(s, b);
  stop_count();
  return val;
}

NTYPE IDAO::main(State& s, NTYPE bound) {
  current_bound = bound;

  while (!solved()) {
    if (INFINITE(current_bound)) {
      if (trace_level > 1) {
	std::cerr << "break on cost = +INF" << std::endl;
      }
      return current_bound;
    }
    if (current_bound > get_cost_limit()) {
      if (trace_level > 1) {
	std::cerr << "break on cost > " << cost_limit << std::endl;
      }
      return current_bound;
    }
    if (iteration_count >= iteration_limit) {
      if (trace_level > 1) {
	std::cerr << "break on iteration count > " << iteration_limit
		  << std::endl;
      }
      return current_bound;
    }
    if (trace_level > 1) {
      std::cerr << "bound = " << PRINT_NTYPE(current_bound) << std::endl;
    }
    stats.begin_iteration();
    NTYPE new_bound = new_state(s, current_bound);
    if (break_signal_raised()) return current_bound;
    if (!solved()) stats.current_lower_bound(new_bound);
    result.no_more_solutions(current_bound);
    stats.end_iteration();
    iteration_count += 1;
    current_bound = new_bound;
    if ((trace_level > 0) && !solved()) std::cerr << stats << std::endl;
  }
  return current_bound;
}

NTYPE IDAO::new_state(State& s, NTYPE bound) {
  stats.create_node(s);

  if (s.is_final()) {
    if (trace_level > 2) {
      std::cerr << "solution " << s
		<< " (" << (s.is_max() ? "max, " : "min, ")
		<< s.est_cost() << ")" << " at bound " << bound
		<< " and depth " << s.depth()
		<< ": acc cost = " << s.acc_cost()
		<< std::endl;
    }
    set_solved(true);
    result.solution(s, current_bound - bound);
    return 0;
  }

  // check if node is in solved table
  if (solved_tab) {
    HashTable::Entry* entry = solved_tab->find(s);
    if (entry) {
      if (entry->cost <= bound) {
	set_solved(true);
	return entry->cost;
      }
    }
  }

  if (s.est_cost() <= bound) {
    if (cycle_check) {
      for (State* sp = s.predecessor(); sp; sp = sp->predecessor())
	if (s.compare(*sp) == 0) return POS_INF;
    }
    stats.expand_node(s);
    if (trace_level > 2) {
      std::cerr << "expanding " << s
		<< " (" << (s.is_max() ? "max, " : "min, ")
		<< s.est_cost() << ")" << " at bound " << bound
		<< " and depth " << s.depth()
		<< std::endl;
    }
    else if (trace_level > 1) {
      if ((stats.nodes() % TRACE_LEVEL_2_NOTIFY) == 0)
	std::cerr << stats << std::endl;
    }
    if (s.is_max()) {
      return new_max_state(s, bound);
    }
    else {
      NTYPE val = s.expand(*this, bound);
      if (store_cost && (val > s.est_cost())) {
	if (trace_level > 2) {
	  std::cerr << "storing " << s << " with cost " << val << std::endl;
	}
	s.store(val, solved());
      }
      if (solved()) {
	if (solved_tab)	solved_tab->insert(s, val);
      }
      return val;
    }
  }
  else return s.est_cost();
}

NTYPE IDAO::new_max_state(State& s, NTYPE bound)
{
  maxIDAO sub_search(stats, result, solved_tab, s.depth(), trace_level);
  sub_search.set_store_cost(store_cost);
  NTYPE val = s.expand(sub_search, bound);
  if (sub_search.solved()) {
    if (solved_tab) solved_tab->insert(s, val);
    set_solved(true);
  }
  return val;
}

NTYPE IDAO::cost() const {
  return current_bound;
}

maxIDAO::maxIDAO
(Statistics& s, SearchResult& res, HashTable* tab, index_type d)
  : stats(s),
    result(res),
    is_solved(true),
    solved_tab(tab),
    store_cost(true),
    depth(d),
    trace_level(0)
{
  // done
}

maxIDAO::maxIDAO
(Statistics& s, SearchResult& res, HashTable* tab, index_type d, int level)
  : stats(s),
    result(res),
    is_solved(true),
    solved_tab(tab),
    store_cost(true),
    depth(d),
    trace_level(level)
{
  // done
}

NTYPE maxIDAO::new_state(State& s, NTYPE bound) {
  if (trace_level > 2) {
    std::cerr << "sub state (" << s.depth() << "): " << s << std::endl;
  }

  assert(!s.is_max());

  IDAO search(stats, result, solved_tab);
  search.set_cost_limit(bound);
  search.set_store_cost(store_cost);
  if (trace_level > 3)
    search.set_trace_level(trace_level);
  else
    search.set_trace_level(0);

  // don't do start() because this clears the solved table
  // NTYPE val = search.start(s);
  NTYPE val = search.main(s, s.est_cost());
  is_solved = is_solved && search.solved();

  if (trace_level > 2) {
    std::cerr << "sub state (" << s.depth() << "): " << s
	      << ", cost = " << val
	      << ", solved = " << (search.solved() ? 1 : 0)
	      << std::endl;
  }

  return val;
}

bool maxIDAO::solved() const {
  return is_solved;
}

bool maxIDAO::optimal() const {
  return is_solved;
}

bool maxIDAO::done() const {
  return ((!is_solved && IDAO::alpha_cut) || stats.break_signal_raised());
}

bool estimated_cost::operator()(const State*& v0, const State*& v1) const
{
  assert(v0);
  assert(v1);
  State* s0 = (State*)v0;
  State* s1 = (State*)v1;
  std::cerr << "test: " << s0->est_cost() << " > " << s1->est_cost()
	    << "?" << std::endl;
  return (s0->est_cost() > s1->est_cost());
}

NTYPE kIDAO::new_max_state(State& s, NTYPE bound)
{
  maxkIDAO sub_search(max_k, stats, result, solved_tab,
		      s.depth(), trace_level);
  s.expand(sub_search, bound);
  sub_search.set_store_cost(store_cost);
  NTYPE val = sub_search.expand(bound);
  val = MAX(val, s.est_cost());
  if (sub_search.solved()) {
    if (solved_tab) solved_tab->insert(s, val);
    set_solved(true);
  }
  return val;
}

maxkIDAO::~maxkIDAO()
{
  for (index_type k = 0; k < succ.length(); k++)
    delete succ[k];
}

NTYPE maxkIDAO::new_state(State& s, NTYPE bound)
{
  // estimated_cost eco;
  // succ.insert_ordered(s.copy(), eco);
  for (index_type k = 0; k < succ.length(); k++) {
    if (succ[k]->est_cost() < s.est_cost()) {
      for (index_type i = succ.length(); i > k; i--)
	succ[i] = succ[i - 1];
      succ.inc_length();
      succ[k] = s.copy();
      return s.est_cost();
    }
  }
  succ.append(s.copy());
  return s.est_cost();
}

NTYPE maxkIDAO::expand(NTYPE bound)
{
  if (trace_level > 2) {
    std::cerr << succ.length() << " substates (" << depth << ")";
    for (index_type k = 0; k < succ.length(); k++)
      std::cerr << " " << succ[k]->est_cost();
    std::cerr << ", bound = " << bound;
    std::cerr << std::endl;
  }

//   std::cerr << "expanding " << max_k << " of " << succ.length()
// 	    << " successors..." << std::endl;

  NTYPE v_max = 0;
  for (index_type k = 0; (k < succ.length()) && (k < max_k) && !done(); k++) {
    State* s = succ[k];

    if (trace_level > 2) {
      std::cerr << "sub state (" << s->depth() << "): " << *s << std::endl;
    }
    assert(!s->is_max());

    kIDAO search(max_k, stats, result, solved_tab);
    search.set_cost_limit(bound);
    search.set_store_cost(store_cost);
    if (trace_level > 3)
      search.set_trace_level(trace_level);
    else
      search.set_trace_level(0);

    NTYPE val = search.main(*s, s->est_cost());
    is_solved = is_solved && search.solved();

    if (trace_level > 2) {
      std::cerr << "sub state (" << s->depth() << "): " << *s
		<< ", cost = " << val
		<< ", solved = " << (search.solved() ? 1 : 0)
		<< std::endl;
    }
    v_max = MAX(val, v_max);
  }

  return v_max;
}

END_HSPS_NAMESPACE
