#ifndef EXPLORE_H
#define EXPLORE_H

#include "config.h"
#include "search.h"
#include "stats.h"

BEGIN_HSPS_NAMESPACE

struct TreeStatistics {
  count_type n_min_nodes;
  count_type n_max_nodes;
  count_type n_expanded;
  count_type n_branches;
  count_type n_dead_ends;

  count_type node_limit;

  TreeStatistics();
  bool limit_reached() { return ((n_min_nodes + n_max_nodes) >= node_limit); };
  void reset();
  void write(std::ostream& s) const;
};

class Tree : public Search {
  typedef lvector<Tree*> branch_vec;

  index_type  id;
  State*      root;
  branch_vec  successors;
  bool        expanded;
  bool        x_solved;
  NTYPE       x_est_cost;

  Statistics&     stats;
  TreeStatistics& data;

  static index_type next_id;

  void expand(); // ensure root is expanded
  void update(); // propagate solved status & cost from children

  void write_dot_level(std::ostream& s,
		       branch_vec& level,
		       branch_vec& next,
		       bool solution_only,
		       bool skip_solution_tree);

 public:
  Tree(State& r, Statistics& s, TreeStatistics& t);
  virtual ~Tree();

  void build(index_type d);
  void build(NTYPE b);
  virtual NTYPE new_state(State& s, NTYPE bound);
  virtual bool solved() const;
  virtual bool optimal() const;
  virtual bool done() const;

  void write_dot(std::ostream& s,
		 bool solution_only,
		 bool skip_solution_tree);
};

END_HSPS_NAMESPACE

#endif
