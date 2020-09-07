#ifndef BFS_H
#define BFS_H

#include "config.h"
#include "search_base.h"
#include "nodeset.h"

BEGIN_HSPS_NAMESPACE

class NodeOrderForBFS : public node_vec::order {
 public:
  NodeOrderForBFS() { };
  virtual bool operator()(const nodep& v0, const nodep& v1) const;
};

class BFS : public SingleSearchAlgorithm {
  static NodeOrderForBFS b_op;

 protected:
  static const count_type TRACE_LEVEL_2_NOTIFY = 10000;

  HashNodeSet graph;
  NodeQueue   queue;
  Node* current_node;
  NTYPE best_node_cost;

  count_type acc_succ;
  count_type new_succ;

  void update_cost(Node* n, Node* p, NTYPE d);

 public:
  NodeSet& state_space() { return graph; };

  BFS(Statistics& s, SearchResult& r);
  BFS(Statistics& s, SearchResult& r, index_type nt_size);
  virtual ~BFS();

  virtual NTYPE start(State& s, NTYPE b);
  virtual NTYPE start(State& s);
  virtual NTYPE resume();

  virtual NTYPE main();
  virtual NTYPE new_state(State& s, NTYPE bound);

  virtual NTYPE cost() const;
  virtual bool done() const;
};

class BFS_PX : public BFS {
  NTYPE threshold;
 public:
  BFS_PX(Statistics& s, SearchResult& r, NTYPE thresh)
    : BFS(s, r), threshold(thresh) { };
  BFS_PX(Statistics& s, SearchResult& r, index_type nt_size, NTYPE thresh)
    : BFS(s, r, nt_size), threshold(thresh) { };
  virtual ~BFS_PX() { };

  virtual NTYPE main();
};

END_HSPS_NAMESPACE

#endif
