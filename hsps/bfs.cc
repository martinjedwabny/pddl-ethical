
#include "bfs.h"

BEGIN_HSPS_NAMESPACE

bool NodeOrderForBFS::operator()(const nodep& v0, const nodep& v1) const
{
  if (v0->val < v1->val) return true;
  if ((v0->val == v1->val) && (v0->est < v1->est)) return true;
  return false;
}

NodeOrderForBFS BFS::b_op;

void BFS::update_cost(Node* n, Node* p, NTYPE d) {
  NTYPE c_new = p->acc + d;
  if (c_new < n->acc) {
//     std::cerr << "changing parent of " << n->id
// 	      << " from " << n->bp_pre->id
// 	      << " to " << p->id << "..." << std::endl;
    n->acc = c_new;
    n->val = c_new + n->est;
    n->bp_pre = p;
    n->bp_delta = d;
    if (n->pos != no_such_index) {
      queue.shift_up(n->pos);
    }
    for (index_type k = 0; k < n->succ.length(); k++)
      update_cost(n->succ[k].node, n, n->succ[k].delta);
  }
}

NTYPE BFS::new_state(State& s, NTYPE bound) {
  NTYPE s_est = s.est_cost();
  if (INFINITE(s_est)) return POS_INF;

  acc_succ += 1;

  Node* n = graph.insert_node(s);
  if (n->state) {
    assert(current_node);
    current_node->succ.insert(Link(n, s.delta_cost()));

    if (n->acc > (current_node->acc + s.delta_cost())) {
      // trace_level = 4;
      if (trace_level > 3) {
	std::cerr << "update #" << n->id << " " << s
		  << ": " << n->acc << "/" << n->est << "/" << n->val
		  << " -> "
		  << current_node->acc + s.delta_cost() << "/"
		  << n->est << "/"
		  << current_node->acc + s.delta_cost() + n->est
		  << ", #" << current_node->id << std::endl;
	Node* p = n->bp_pre;
	while (p) {
	  std::cerr << " #" << p->id
		    << ": " << p->acc << "/" << p->est << "/" << p->val
		    << " <- ";
	  p = p->bp_pre;
	}
	std::cerr << "0" << std::endl;
	p = current_node;
	while (p) {
	  std::cerr << " #" << p->id
		    << ": " << p->acc << "/" << p->est << "/" << p->val
		    << " <- ";
	  p = p->bp_pre;
	}
	std::cerr << "0" << std::endl;
      }
      delete n->state;
      n->state = s.copy();
      update_cost(n, current_node, s.delta_cost());
      // trace_level = 0;
      // exit(255);
    }
  }

  else {
    new_succ += 1;
    stats.create_node(s);
    n->state = s.copy();
    if (current_node) {
      current_node->succ.insert(Link(n, s.delta_cost()));
      n->bp_pre = current_node;
      n->bp_delta = s.delta_cost();
      n->acc = current_node->acc + s.delta_cost();
    }
    else {
      graph.make_root(n);
      n->bp_pre = 0;
      n->bp_delta = s.delta_cost();
      if (n->bp_delta > 0) {
	std::cerr << "warning: root node with non-zero delta cost"
		  << std::endl;
      }
      n->acc = s.delta_cost();
    }
    n->est = s_est;
    n->val = n->acc + n->est;
    n->exp = 0;
    queue.enqueue(n);

    if (trace_level > 3) {
      std::cerr << "new #" << n->id << " " << s
		<< ": " << n->acc << "/" << n->est << "/" << n->val;
      if (trace_level > 4) {
	std::cerr << " >> ";
	s.write_path(std::cerr);
      }
      std::cerr << std::endl;
    }
  }

  return POS_INF;
}

BFS::BFS(Statistics& s, SearchResult& r)
  : SingleSearchAlgorithm(s, r),
    graph(31337),
    queue(b_op),
    current_node(0),
    best_node_cost(0)
{
  // done
}

BFS::BFS(Statistics& s, SearchResult& r, index_type nt_size)
  : SingleSearchAlgorithm(s, r),
    graph(nt_size),
    queue(b_op),
    current_node(0),
    best_node_cost(0)
{
  // done
}

BFS::~BFS() {
  /* delete graph; */
}

NTYPE BFS::main() {
  while ((!solved() || result.more()) && !queue.empty()) {
    if (break_signal_raised()) return best_node_cost;
    current_node = queue.peek();
    if (current_node->val > best_node_cost) {
      if (!solved()) stats.current_lower_bound(best_node_cost);
      result.no_more_solutions(best_node_cost);
      if (solved() && !result.more()) return best_node_cost;
      best_node_cost = current_node->val;
      if (trace_level > 0) {
	std::cerr << "f = " << current_node->val
		  << ", q = " << queue.length()
		  << ", " << stats << std::endl;
      }
    }
    if (best_node_cost > cost_limit) {
      return best_node_cost;
    }
    current_node = queue.dequeue();
    assert(!current_node->state->is_max());
    if (current_node->state->is_final()) {
      if (trace_level > 0) {
	std::cerr << "solution (cost = " << current_node->acc
		  << " (" << current_node->state->acc_cost()
		  << "), depth = " << current_node->state->depth() << ")"
		  << std::endl;
      }
      set_solved(true);
      graph.set_back_path_solution_cost(current_node, current_node->acc);
      if (current_node->state->is_encapsulated()) {
	result.solution(*(current_node->state),
			current_node->state->acc_cost());
      }
      else {
	State* f_state = graph.build_path(current_node);
	result.solution(*f_state, f_state->acc_cost());
	f_state->delete_path();
      }
    }
    else {
      assert(current_node->exp == 0); // node should only ever be expanded once
      stats.expand_node(*(current_node->state));
      if (trace_level > 2) {
	std::cerr << "expanding #" << current_node->id
		  << " " << *(current_node->state)
		  << ": " << current_node->acc
		  << "/" << current_node->est
		  << "/" << current_node->val;
	if (current_node->bp_pre) {
	  std::cerr << ", pre = #" << current_node->bp_pre->id;
	}
	std::cerr << std::endl;
      }
      else if (trace_level > 1) {
	if ((stats.nodes() % TRACE_LEVEL_2_NOTIFY) == 0)
	  std::cerr << stats << std::endl;
      }
      acc_succ = 0;
      new_succ = 0;
      index_type l = queue.length();
      current_node->state->expand(*this, POS_INF);
      current_node->exp += 1;
      if (!done()) current_node->closed = true;
      assert(new_succ == (queue.length() - l));
      if (trace_level > 3) {
	std::cerr << "done expanding #" << current_node->id
		  << " (" << acc_succ << " acc. / " << new_succ
		  << " new succs.), closed = " << current_node->closed
		  << std::endl;
      }
    }
    current_node = 0;
  }
  if (queue.empty()) {
    result.no_more_solutions(POS_INF);
    if (!solved()) best_node_cost = POS_INF;
  }
  return best_node_cost;
}

NTYPE BFS::start(State& s, NTYPE b)
{
  return start(s);
}

NTYPE BFS::start(State& s)
{
  reset();
  graph.clear();
  queue.set_length(0);
  best_node_cost = 0;

  start_count();
  current_node = 0;
  new_state(s, POS_INF);

  NTYPE val = main();
  stop_count();
  return val;
}

NTYPE BFS::resume() {
  start_count();
  NTYPE val = main();
  stop_count();
  return val;
}

NTYPE BFS::cost() const {
  return best_node_cost;
}

bool BFS::done() const {
  return ((solved() && !result.more()) || break_signal_raised());
}

NTYPE BFS_PX::main() {
  while ((!solved() || result.more()) && !queue.empty()) {
    if (break_signal_raised()) return best_node_cost;
    current_node = queue.peek();
    if (current_node->val > best_node_cost) {
      if (!solved()) stats.current_lower_bound(current_node->val);
      result.no_more_solutions(best_node_cost);
      if (solved() && !result.more()) return best_node_cost;
      best_node_cost = current_node->val;
      if (trace_level > 0) {
	std::cerr << "f = " << current_node->val
		  << ", q = " << queue.length()
		  << ", " << stats << std::endl;
      }
    }
    if (best_node_cost > cost_limit) {
      return best_node_cost;
    }
    current_node = queue.dequeue();
    assert(!current_node->state->is_max());
    if (current_node->state->is_final()) {
      set_solved(true);
      if (current_node->state->is_encapsulated()) {
	result.solution(*(current_node->state),
			current_node->state->acc_cost());
      }
      else {
	State* f_state = graph.build_path(current_node);
	result.solution(*f_state, f_state->acc_cost());
	f_state->delete_path();
      }
    }
    else {
      stats.expand_node(*(current_node->state));
      if (trace_level > 2) {
	if (current_node->exp == 0)
	  std::cerr << "expanding ";
	else
	  std::cerr << "re-expanding ";
	std::cerr << *(current_node->state) << ": "
		  << current_node->acc << "/"
		  << current_node->est << "/"
		  << current_node->val
		  << " with bound " << current_node->val + threshold
		  << std::endl;
	acc_succ = 0;
	new_succ = 0;
      }
      else if (trace_level > 1) {
	if ((stats.nodes() % TRACE_LEVEL_2_NOTIFY) == 0)
	  std::cerr << stats << std::endl;
      }
      NTYPE new_val =
	current_node->state->expand(*this, current_node->val + threshold);
      if (trace_level > 2) {
	std::cerr << "new value is " << new_val << ", " << acc_succ
		  << " successors accepted, " << new_succ << " are new"
		  << std::endl;
      }
      if (FINITE(new_val)) {
	current_node->val = new_val;
	queue.enqueue(current_node);
      }
      current_node->exp += 1;
    }
    current_node = 0;
  }
  if (queue.empty()) {
    result.no_more_solutions(POS_INF);
    if (!solved()) best_node_cost = POS_INF;
  }
  return best_node_cost;
}

END_HSPS_NAMESPACE
