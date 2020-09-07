#ifndef NODESET_H
#define NODESET_H

#include "config.h"
#include "index_type.h"
#include "search.h"
#include "stats.h"
#include "name.h"

BEGIN_HSPS_NAMESPACE

class Node;
typedef Node* nodep;
typedef lvector<nodep> node_vec;

struct Link {
  Node* node;
  NTYPE delta;

  Link() : node(0), delta(0) { };
  Link(Node* s, NTYPE d) : node(s), delta(d) { };

  Link& operator=(const Link& l);
  bool operator==(const Link& l);
  bool operator<(const Link& l);
  bool operator>(const Link& l);
  bool operator<=(const Link& l);
  bool operator>=(const Link& l);
};

typedef svector<Link> link_vec;

class Node {
 public:
  index_type id;
  State*     state;
  link_vec   succ;
  bool       closed;

  NTYPE      acc;
  NTYPE      est;
  NTYPE      val;
  NTYPE      opt;
  index_type pos;
  count_type exp;

  Node*   bp_pre;
  NTYPE bp_delta;
  link_vec* all_pre;

  Node() :
    id(0), state(0), closed(false), acc(0), est(0), val(0), opt(POS_INF),
    pos(no_such_index), exp(0), bp_pre(0), bp_delta(0), all_pre(0) { };
  ~Node();

  NTYPE min_delta_to(Node* n);
  bool solved() { return FINITE(opt); };
  bool back_path_contains(Node* n);
  void add_predecessor(Node* n, NTYPE d);

  void backup_costs(node_vec& p);

  void write(std::ostream& s, const Name* p = 0);
  void write_short(std::ostream& s, const Name* p = 0);

  void write_graph_node(std::ostream& s);
  void write_graph_edges(std::ostream& s);
};

class NodeQueue : public node_vec {
  const node_vec::order& before;
 public:
  NodeQueue(const node_vec::order& b);
  ~NodeQueue();

  void check_queue();

  Node* peek();
  void enqueue(Node*);
  Node* dequeue();

  // these should be private, but BFS uses them when updating costs/paths
  void shift_up(index_type i);
  void shift_down(index_type i);
};

class NodeSet {
 protected:
  static index_type next_id;
  node_vec roots;

 public:
  static bool write_state_in_graph_node;

  virtual ~NodeSet();

  virtual Node* insert_node(State& s) = 0;
  virtual Node* find_node(State& s) = 0;
  virtual void  clear() = 0;
  virtual void  collect_nodes(node_vec& ns) = 0;

  Node*  insert_root_node(State& s);
  void   make_root(Node* n);
  node_vec& root_nodes();

  void   compute_reverse_links(node_vec& v);
  void   compute_reverse_links();
  void   mark_solved_apsp();
  void   mark_solved();
  void   backup_costs();

  void   set_back_path_solution_cost(Node* n, NTYPE c_sol);
  void   cache_pg(NTYPE c_sol);

  void   back_path_to_sequence(Node* n, node_vec& ns);
  State* build_path(node_vec& ns);
  State* build_path(Node* tn);

  void  write_short(std::ostream& s, const Name* p = 0);
  void  write_graph(std::ostream& s);
};

class NodeSetSearchState : public State {
  Node*  node;
  NTYPE  delta;
  State* path;
 public:
  NodeSetSearchState(Node* n);
  NodeSetSearchState(NodeSetSearchState* p, Link& l, State* s);
  NodeSetSearchState(const NodeSetSearchState& s);
  virtual ~NodeSetSearchState();

  virtual NTYPE delta_cost();
  virtual NTYPE est_cost();
  virtual bool  is_final();
  virtual bool  is_max();
  virtual NTYPE expand(Search& s, NTYPE bound);
  virtual void store(NTYPE cost, bool opt);
  virtual void reevaluate();
  virtual int compare(const State& s);
  virtual index_type hash();
  virtual State* copy();
  virtual void insert(Plan& p);
  virtual void insert_path(Plan& p);
  virtual void write(std::ostream& s);
  virtual void write_plan(std::ostream& s);
};

class TreeNodeSet : public Node, public NodeSet {
  TreeNodeSet* left;
  TreeNodeSet* right;
 public:
  TreeNodeSet();
  virtual ~TreeNodeSet();

  virtual Node* insert_node(State& s);
  virtual Node* find_node(State& s);
  virtual void  clear();

  virtual void collect_nodes(node_vec& ns);
};

class HashNodeSet : public NodeSet {
  index_type    size;
  TreeNodeSet** tab;
 public:
  HashNodeSet(index_type s);
  virtual ~HashNodeSet();

  virtual Node* insert_node(State& s);
  virtual Node* find_node(State& s);
  virtual void  clear();

  virtual void collect_nodes(node_vec& ns);
};

// inlines

inline Link& Link::operator=(const Link& l)
{
  node = l.node;
  delta = l.delta;
  return *this;
}

inline bool Link::operator==(const Link& l)
{
  return ((node == l.node) && (delta == l.delta));
}

inline bool Link::operator<(const Link& l)
{
  return ((node < l.node) || ((node == l.node) && (delta < l.delta)));
}

inline bool Link::operator>(const Link& l)
{
  return ((node > l.node) || ((node == l.node) && (delta > l.delta)));
}

inline bool Link::operator<=(const Link& l)
{
  return ((node <= l.node) || ((node == l.node) && (delta <= l.delta)));
}

inline bool Link::operator>=(const Link& l)
{
  return ((node >= l.node) || ((node == l.node) && (delta >= l.delta)));
}

END_HSPS_NAMESPACE

#endif
