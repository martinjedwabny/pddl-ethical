
#include "nodeset.h"

BEGIN_HSPS_NAMESPACE

Node::~Node()
{
  if (state) delete state;
}

NTYPE Node::min_delta_to(Node* n)
{
  NTYPE d = POS_INF;
  for (index_type k = 0; k < succ.length(); k++)
    if (succ[k].node == n)
      d = MIN(d, succ[k].delta);
  return d;
}

bool Node::back_path_contains(Node* n)
{
  if (n == this) return true;
  if (bp_pre) {
    return bp_pre->back_path_contains(n);
  }
  return false;
}

void Node::add_predecessor(Node* n, NTYPE d)
{
  if (all_pre == 0) all_pre = new link_vec;
  for (index_type k = 0; k < all_pre->length(); k++)
    if ((*all_pre)[k].node == n) {
      (*all_pre)[k].delta = MIN((*all_pre)[k].delta, d);
      return;
    }
  all_pre->append(Link(n, d));
}

void Node::backup_costs(node_vec& p)
{
//   std::cerr << "enter #" << id << ": exp = " << exp
//  	    << ", est = " << est << ", opt = " << opt
//  	    << std::endl;
  if (exp == 0) {
    if (state->is_final()) {
      opt = 0;
      assert(est == 0);
    }
    else {
      opt = POS_INF;
    }
    return;
  }
  NTYPE e_min = POS_INF;
  NTYPE o_min = POS_INF;
  p.append(this);
  for (index_type k = 0; k < succ.length(); k++)
    if ((succ[k].node->est + succ[k].delta) < e_min) {
      if (p.first(succ[k].node) == no_such_index) {
	succ[k].node->backup_costs(p);
	e_min = MIN(e_min, succ[k].node->est + succ[k].delta);
	o_min = MIN(o_min, succ[k].node->opt + succ[k].delta);
      }
    }
  p.dec_length();
//   std::cerr << "update #" << id
// 	    << ": est = " << MAX(MIN(e_min, est), state->est_cost())
//  	    << " (" << e_min << ", " << est << ", " << state->est_cost()
// 	    << "), opt = " << MIN(o_min, opt)
//  	    << " (" << o_min << ", " << opt << ")" << std::endl;
  est = MIN(e_min, est);
  opt = MIN(o_min, opt);
  assert(est <= opt);
  if (state->est_cost() > opt) {
    std::cerr << "ERROR: INADMISSIBLE HEURISTIC!" << std::endl;
    exit(255);
  }
}

void Node::write(std::ostream& s, const Name* p)
{
  assert(state);
  if (p) s << p << " ";
  s << "NODE: " << id
    << ' ' << state->is_max()
    << ' ' << state->is_final()
    << ' ' << acc
    << ' ' << opt
    << ' ' << est
    << ' ' << val
    << ' ' << exp
    << ' ' << succ.length();
  state->write_eval(s, " H:", false);
  s << " STATE: " << *state
    << std::endl;
}

void Node::write_short(std::ostream& s, const Name* p)
{
  assert(state);
  if (p) s << p << " ";
  s << "NODE: " << id
    << ' ' << state->is_max()
    << ' ' << state->is_final()
    << ' ' << acc
    << ' ' << opt
    << ' ' << est
    << ' ' << val
    << ' ' << exp
    << ' ' << succ.length();
  state->write_eval(s, " H:", true);
}

void Node::write_graph_node(std::ostream& s)
{
  assert(state);
  s << "N" << id << " [label=\"" << id << ": ";
  if (NodeSet::write_state_in_graph_node) {
    s << *state << " / ";
  }
  s << acc << " + " << est << " / " << opt << "\"";
  if (state->is_max()) {
    s << ",shape=box";
  }
  else {
    s << ",shape=ellipse";
  }
  if (state->is_final()) {
    s << ",style=filled";
  }
  else if (exp == 0) {
    s << ",style=dashed";
  }
  s << "];" << std::endl;
}

void Node::write_graph_edges(std::ostream& s)
{
  for (index_type k = 0; k < succ.length(); k++) {
    s << "N" << id << " -> " << " N" << succ[k].node->id << " [label=\""
      << succ[k].delta << "\"";
    if (FINITE(opt) && ((succ[k].node->opt + succ[k].delta) == opt)) {
      s << ",style=\"bold\"";
    }
    s << "];" << std::endl;
  }
}

NodeQueue::NodeQueue(const node_vec::order& b)
  : node_vec(0, 0), before(b)
{
  // done
}

NodeQueue::~NodeQueue()
{
  // done
}

#define ROOT(i)        (i == 0)
#define PARENT(i)      (((i+1) >> 1) - 1)
#define LEFT_CHILD(i)  (((i+1)*2) - 1)
#define RIGHT_CHILD(i) ((i+1)*2)

void NodeQueue::check_queue()
{
  for (index_type i = 0; i < (length() / 2); i++) {
    if (LEFT_CHILD(i) < length()) {
      if (before((*this)[LEFT_CHILD(i)], (*this)[i])) {
	std::cerr << "error in queue:";
	for (index_type k = 0; k < length(); k++)
	  std::cerr << " #" << (*this)[k]->id << ":" << (*this)[k]->val;
	std::cerr << " at position " << i << " / " << LEFT_CHILD(i)
		  << std::endl;
	exit(255);
      }
    }
    if (RIGHT_CHILD(i) < length()) {
      if (before((*this)[RIGHT_CHILD(i)], (*this)[i])) {
	std::cerr << "error in queue:";
	for (index_type k = 0; k < length(); k++)
	  std::cerr << " #" << (*this)[k]->id << ":" << (*this)[k]->val;
	std::cerr << " at position " << i << " / " << RIGHT_CHILD(i)
		  << std::endl;
	exit(255);
      }
    }
  }
}

void NodeQueue::shift_up(index_type i) {
  while (!ROOT(i)) {
    if (before((*this)[i], (*this)[PARENT(i)])) {
      swap(i, PARENT(i));
      (*this)[i]->pos = i;
      (*this)[PARENT(i)]->pos = PARENT(i);
      i = PARENT(i);
    }
    else return;
  }
}

void NodeQueue::shift_down(index_type i) {
  index_type j;
  while (i < (length() / 2)) {
    assert(LEFT_CHILD(i) < length());
    if (LEFT_CHILD(i) == (length() - 1)) {
      j = LEFT_CHILD(i);
    }
    else {
      assert(RIGHT_CHILD(i) < length());
      if (before((*this)[LEFT_CHILD(i)], (*this)[RIGHT_CHILD(i)]))
	j = LEFT_CHILD(i);
      else
	j = RIGHT_CHILD(i);
    }
    if (before((*this)[j], (*this)[i])) {
      swap(i, j);
      (*this)[i]->pos = i;
      (*this)[j]->pos = j;
      i = j;
    }
    else return;
  }
}

void NodeQueue::enqueue(Node* n)
{
  append(n);
  n->pos = length() - 1;
  shift_up(length() - 1);
#ifdef CHECK_QUEUE_OPERATIONS
  std::cerr << "checking queue after ENQUEUE..." << std::endl;
  check_queue();
#endif
}

Node* NodeQueue::dequeue()
{
  if (length() == 0) {
    return 0;
  }
  Node* n = (*this)[0];
  n->pos = no_such_index;
  if (length() > 1) {
    (*this)[0] = (*this)[length() - 1];
    (*this)[0]->pos = 0;
    dec_length();
    shift_down(0);
  }
  else {
    clear();
  }
#ifdef CHECK_QUEUE_OPERATIONS
  std::cerr << "checking queue after DEQUEUE..." << std::endl;
  check_queue();
#endif
  return n;
}

Node* NodeQueue::peek()
{
#ifdef CHECK_QUEUE_OPERATIONS
  std::cerr << "checking queue before PEEK..." << std::endl;
  check_queue();
#endif
  if (length() == 0) return 0;
  return (*this)[0];
}


index_type NodeSet::next_id = 1;
bool NodeSet::write_state_in_graph_node = true;

NodeSet::~NodeSet()
{
  // can't call clear because no implement?
}

void NodeSet::back_path_to_sequence(Node* n, node_vec& ns)
{
  if (n->bp_pre) {
    back_path_to_sequence(n->bp_pre, ns);
  }
  ns.append(n);
}

void NodeSet::set_back_path_solution_cost(Node* n, NTYPE c_sol)
{
  n->opt = c_sol;
  if (n->bp_pre) {
    set_back_path_solution_cost(n->bp_pre, c_sol + n->bp_delta);
  }
}

State* NodeSet::build_path(node_vec& ns)
{
  if (ns.length() == 0) return 0;
  assert(ns[0]->state);
  State* s = ns[0]->state->copy();
  for (index_type k = 1; k < ns.length(); k++) {
    assert(ns[k]->state);
    Transitions ts(s, ns[k]->state, ns[k]->acc - ns[k - 1]->acc);
    if (ts.length() == 0) {
      std::cerr << "error: can't make path from node sequence:" << std::endl;
      for (index_type i = 0; i < ns.length(); i++) {
	std::cerr << i << ". ";
	ns[i]->write(std::cerr);
      }
      std::cerr << "- no transition from " << k - 1 << " to " << k
		<< std::endl;
      exit(255);
    }
    s = ts[0]->copy();
  }
  return s;
}

State* NodeSet::build_path(Node* tn)
{
  node_vec ns(0, 0);
  back_path_to_sequence(tn, ns);
  return build_path(ns);
}

Node* NodeSet::insert_root_node(State& s)
{
  Node* n = insert_node(s);
  make_root(n);
  return n;
}

void NodeSet::make_root(Node* n)
{
  if (roots.first(n) == no_such_index)
    roots.append(n);
}

node_vec& NodeSet::root_nodes()
{
  return roots;
}

void NodeSet::cache_pg(NTYPE c_sol)
{
  node_vec v(0, 0);
  collect_nodes(v);

  for (index_type k = 0; k < v.length(); k++) if (v[k]->closed) {
    if (FINITE(v[k]->opt)) {
      v[k]->state->store(v[k]->opt, true);
    }
    else {
      v[k]->state->store(c_sol - v[k]->acc, false);
    }
  }
}

void NodeSet::mark_solved_apsp()
{
  node_vec v(0, 0);
  collect_nodes(v);
  index_type n = v.length();
  NTYPE** d = new NTYPE*[n];
  for (index_type i = 0; i < n; i++)
    d[i] = new NTYPE[n];

  std::cerr << "initializing..." << std::endl;
  for (index_type i = 0; i < n; i++)
    for (index_type j = 0; j < n; i++)
      d[i][j] = POS_INF;
  std::cerr << "adding links..." << std::endl;
  for (index_type k = 0; k < v.length(); k++)
    for (index_type i = 0; i < v[k]->succ.length(); i++) {
      index_type p = v.first(v[k]->succ[i].node);
      assert(p != no_such_index);
      d[k][p] = MIN(d[k][p], v[k]->succ[i].delta);
    }
  std::cerr << "computing shortest paths..." << std::endl;
  for (index_type k = 0; k < n; k++)
    for (index_type i = 0; i < n; i++)
      for (index_type j = 0; j < n; i++)
	d[i][j] = MIN(d[i][j], d[i][k] + d[k][j]);
      
  std::cerr << "initializing..." << std::endl;
  for (index_type k = 0; k < v.length(); k++)
    v[k]->opt = POS_INF;
  std::cerr << "computing min distances..." << std::endl;
  for (index_type k = 0; k < v.length(); k++) if (v[k]->state->is_final()) {
    v[k]->opt = 0;
    for (index_type i = 0; i < v.length(); i++)
      v[i]->opt = MIN(v[i]->opt, d[i][k]);
  }
}

class NodeOrderByOpt : public node_vec::order {
public:
  NodeOrderByOpt() { };
  virtual bool operator()(const nodep& v0, const nodep& v1) const;
};

bool NodeOrderByOpt::operator()(const nodep& v0, const nodep& v1) const
{
  if (v0->opt < v1->opt) return true;
  return false;
}

void NodeSet::compute_reverse_links(node_vec& v)
{
  for (index_type k = 0; k < v.length(); k++)
    for (index_type i = 0; i < v[k]->succ.length(); i++)
      v[k]->succ[i].node->add_predecessor(v[k], v[k]->succ[i].delta);
}

void NodeSet::compute_reverse_links()
{
  node_vec v(0, 0);
  collect_nodes(v);
  compute_reverse_links(v);
}

void NodeSet::mark_solved()
{
  node_vec v(0, 0);
  // std::cerr << "collecting nodes..." << std::endl;
  collect_nodes(v);
  // std::cerr << "computing reverse links..." << std::endl;
  compute_reverse_links(v);

  NodeOrderByOpt opt_is_less;
  NodeQueue q(opt_is_less);

  // std::cerr << "initializing..." << std::endl;
  for (index_type k = 0; k < v.length(); k++) {
    if (v[k]->state->is_final()) {
      v[k]->opt = 0;
      q.enqueue(v[k]);
    }
    else {
      v[k]->opt = POS_INF;
    }
  }

  index_type nn = v.length();
  index_type i = 0;
  // index_type l = 0;

  // std::cerr << "marking..." << std::endl;
  while (!q.empty() > 0) {
    Node* n = q.dequeue();
    if (n->all_pre) {
      for (index_type k = 0; k < n->all_pre->length(); k++) {
	NTYPE d = (*(n->all_pre))[k].delta;
	if ((n->opt + d) < (*(n->all_pre))[k].node->opt) {
	  (*(n->all_pre))[k].node->opt = n->opt + d;
	  q.enqueue((*(n->all_pre))[k].node);
	}
      }
    }
    i += 1;
    // if ((i - l) > (nn / 20)) {
    // std::cerr << rational(i, nn).decimal()*100.0 << "%..." << std::endl;
    // l = i;
    // }
  }
}

void NodeSet::backup_costs()
{
  node_vec p(0, 0);
  collect_nodes(p);
  for (index_type k = 0; k < p.length(); k++)
    p[k]->opt = POS_INF;
//   std::cerr << "graph before update:" << std::endl;
//   write_graph(std::cerr);
  for (index_type k = 0; k < roots.length(); k++) {
    p.clear();
    roots[k]->backup_costs(p);
  }
}

void NodeSet::write_short(std::ostream& s, const Name* p)
{
  node_vec v(0, 0);
  collect_nodes(v);
  for (index_type k = 0; k < v.length(); k++) {
    v[k]->write_short(s, p);
  }
}

void NodeSet::write_graph(std::ostream& s)
{
  node_vec v(0, 0);
  collect_nodes(v);

  s << "digraph BFS_SEARCH_SPACE {" << std::endl;
  s << "node [width=0,height=0,shape=box];" << std::endl;
  for (index_type k = 0; k < v.length(); k++) {
    v[k]->write_graph_node(s);
  }
  for (index_type k = 0; k < v.length(); k++) {
    v[k]->write_graph_edges(s);
  }
  s << "}" << std::endl;
}

NodeSetSearchState::NodeSetSearchState(Node* n)
  : delta(0), node(n), path(0)
{
  assert(node);
  assert(node->state);
  assert(node->state->predecessor() == 0);
  assert(node->state->delta_cost() == 0);
  path = node->state->copy();
}

NodeSetSearchState::NodeSetSearchState
(NodeSetSearchState* p, Link& l, State* s)
  : delta(l.delta), node(l.node), path(s)
{
  assert(node);
  assert(node->state);
  assert(node->state->compare(*s) == 0);
  set_predecessor(p);
}

NodeSetSearchState::NodeSetSearchState(const NodeSetSearchState& s)
  : State(s), delta(s.delta), node(s.node), path(s.path->copy())
{
  // done
}

NodeSetSearchState::~NodeSetSearchState()
{
  // done - 'path' not owned by this so don't delete it
}

NTYPE NodeSetSearchState::delta_cost()
{
  return delta;
}

NTYPE NodeSetSearchState::est_cost()
{
  return node->est;
}

bool NodeSetSearchState::is_final()
{
  return node->state->is_final();
}

bool NodeSetSearchState::is_max()
{
  return node->state->is_max();
}

NTYPE NodeSetSearchState::expand(Search& s, NTYPE bound)
{
  NTYPE v = POS_INF;
  for (index_type k = 0; k < node->succ.length(); k++) {
    if ((node->succ[k].delta + node->succ[k].node->opt) <= bound) {
      Transitions* ts =
	new Transitions(path, node->succ[k].node->state, node->succ[k].delta);
      if (ts->length() == 0) {
	std::cerr << "error: no transition found from node #"
		  << node->id
		  << " (" << node->state << ") to successor node #"
		  << node->succ[k].node->id
		  << " (" << node->succ[k].node->state << ") with delta = "
		  << node->succ[k].delta
		  << std::endl;
	exit(255);
      }
      for (index_type i = 0; i < ts->length(); i++) {
	NodeSetSearchState* new_ns =
	  new NodeSetSearchState(this, node->succ[k], (*ts)[i]);
	v = MIN(s.new_state(*new_ns, bound), v);
	delete new_ns;
	if (s.done()) return v;
      }
      delete ts;
    }
  }
  return v;
}

void  NodeSetSearchState::store(NTYPE cost, bool opt)
{
  node->state->store(cost, opt);
}

void NodeSetSearchState::reevaluate()
{
  node->state->reevaluate();
  node->est = node->state->est_cost();
}

int NodeSetSearchState::compare(const State& s)
{
  const NodeSetSearchState& ns = (const NodeSetSearchState&)s;
  if (node->id < ns.node->id) {
    return -1;
  }
  else if (node->id > ns.node->id) {
    return 1;
  }
  return 0;
}

index_type NodeSetSearchState::hash()
{
  return node->id;
}

State* NodeSetSearchState::copy()
{
  return new NodeSetSearchState(*this);
}

void NodeSetSearchState::insert(Plan& p)
{
  path->insert(p);
}

void NodeSetSearchState::insert_path(Plan& p)
{
  path->insert_path(p);
}

void NodeSetSearchState::write(std::ostream& s)
{
  node->write_short(s);
}

void NodeSetSearchState::write_plan(std::ostream& s)
{
  path->write_plan(s);
}


TreeNodeSet::TreeNodeSet()
  : left(0), right(0)
{
  id = next_id++;
}

TreeNodeSet::~TreeNodeSet()
{
  clear();
}

Node* TreeNodeSet::insert_node(State& s)
{
  if (!state) return this;
  int d = state->compare(s);
  if (d < 0) {
    if (!right) right = new TreeNodeSet();
    return right->insert_node(s);
  }
  else if (d > 0) {
    if (!left) left = new TreeNodeSet();
    return left->insert_node(s);
  }
  else return this;
}

Node* TreeNodeSet::find_node(State& s)
{
  if (!state) return 0;
  int d = state->compare(s);
  if (d < 0) {
    if (!right) return 0;
    else return right->find_node(s);
  }
  else if (d > 0) {
    if (!left) return 0;
    else return left->find_node(s);
  }
  else return this;
}

void TreeNodeSet::clear()
{
  if (state) {
    delete state;
    state = 0;
  }
  if (left) {
    delete left;
    left = 0;
  }
  if (right) {
    delete right;
    right = 0;
  }
}

void TreeNodeSet::collect_nodes(node_vec& ns)
{
  ns.append(this);
  if (left) {
    left->collect_nodes(ns);
  }
  if (right) {
    right->collect_nodes(ns);
  }
}

HashNodeSet::HashNodeSet(index_type s)
  : size(s), tab(0)
{
  tab = new TreeNodeSet*[size];
  for (index_type k = 0; k < size; k++) tab[k] = 0;
}

HashNodeSet::~HashNodeSet() {
  clear();
  delete tab;
}

Node* HashNodeSet::insert_node(State& s)
{
  index_type i = (s.hash() % size);
  if (tab[i]) {
    return tab[i]->insert_node(s);
  }
  else {
    tab[i] = new TreeNodeSet();
    return tab[i];
  }
}

Node* HashNodeSet::find_node(State& s)
{
  index_type i = (s.hash() % size);
  if (tab[i]) {
    return tab[i]->find_node(s);
  }
  else {
    return 0;
  }
}

void HashNodeSet::clear()
{
  for (index_type k = 0; k < size; k++) if (tab[k]) {
    delete tab[k];
    tab[k] = 0;
  }
}

void HashNodeSet::collect_nodes(node_vec& ns)
{
  for (index_type k = 0; k < size; k++) {
    if (tab[k]) tab[k]->collect_nodes(ns);
  }
}

END_HSPS_NAMESPACE
