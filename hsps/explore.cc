
#include "explore.h"

BEGIN_HSPS_NAMESPACE

TreeStatistics::TreeStatistics()
  : n_min_nodes(0),
    n_max_nodes(0),
    n_expanded(0),
    n_branches(0),
    n_dead_ends(0),
    node_limit(count_type_max)
{
  // done
}

void TreeStatistics::reset()
{
  n_min_nodes = 0;
  n_max_nodes = 0;
  n_expanded = 0;
  n_branches = 0;
  n_dead_ends = 0;
}

void TreeStatistics::write(std::ostream& s) const
{
  s << n_min_nodes + n_max_nodes << " nodes ("
    << n_min_nodes << " min, " << n_max_nodes << " max, "
    << n_expanded << " expanded)"
    << std::endl;
  s << "average branching factor: "
    << n_branches/(double)n_expanded
    << " (" << n_dead_ends << " dead ends)"
    << std::endl;
}

index_type Tree::next_id = 0;

Tree::Tree(State& r, Statistics& s, TreeStatistics& t)
  : id(next_id++),
    root(r.copy()),
    successors(0, 0),
    expanded(false),
    x_solved(r.is_final()),
    x_est_cost(r.est_cost()),
    stats(s),
    data(t)
{
  if (root->is_max()) {
    data.n_max_nodes += 1;
  }
  else {
    data.n_min_nodes += 1;
  }
}

Tree::~Tree()
{
  for (index_type k = 0; k < successors.length(); k++)
    if (successors[k]) delete successors[k];
  delete root;
}

void Tree::build(index_type d)
{
  if (stats.break_signal_raised() || data.limit_reached()) return;
  if (root->is_final()) return;
  expand();
  if (d > 0) {
    for (index_type k = 0; k < successors.length(); k++) {
      successors[k]->build(d - 1);
      if (stats.break_signal_raised() || data.limit_reached()) return;
    }
  }
  update();
}

void Tree::build(NTYPE b)
{
  if (stats.break_signal_raised() || data.limit_reached()) return;
  if (root->is_final()) return;
  if (root->est_cost() + root->delta_cost() <= b) {
    expand();
    for (index_type k = 0; k < successors.length(); k++) {
      successors[k]->build(b - root->delta_cost());
      if (stats.break_signal_raised() || data.limit_reached()) return;
    }
  }
  update();
}

void Tree::update()
{
  if (!expanded) return;
  if (root->is_max()) {
    x_solved = true;
    x_est_cost = 0;
    for (index_type k = 0; k < successors.length(); k++) {
      x_solved = (x_solved && successors[k]->x_solved);
      x_est_cost = MAX(x_est_cost, successors[k]->x_est_cost +
		       successors[k]->root->delta_cost());
    }
  }
  else {
    x_solved = false;
    x_est_cost = POS_INF;
    for (index_type k = 0; k < successors.length(); k++) {
      x_solved = (x_solved || successors[k]->x_solved);
      x_est_cost = MIN(x_est_cost, successors[k]->x_est_cost +
		       successors[k]->root->delta_cost());
    }
  }
}

void Tree::expand()
{
  if (expanded) return;
  stats.expand_node(*root);
  root->expand(*this, POS_INF);
  expanded = true;

  data.n_expanded += 1;
  data.n_branches += successors.length();
  if (successors.length() == 0) data.n_dead_ends += 1;
}

NTYPE Tree::new_state(State& s, NTYPE bound)
{
  stats.create_node(s);
  successors.append(new Tree(s, stats, data));
  return s.est_cost();
}

bool Tree::solved() const
{
  return false;
}

bool Tree::optimal() const
{
  return false;
}

bool Tree::done() const
{
  return (stats.break_signal_raised() || data.limit_reached());
}

void Tree::write_dot
(std::ostream& s, bool solution_only, bool skip_solution_tree)
{
  s << "digraph ExTree {" << std::endl
    << "rankdir=TB;" << std::endl
    << "node [width=0,height=0];" << std::endl;

  branch_vec current_level(0, 0);
  branch_vec next_level(0, 0);
  if (x_solved || !solution_only) {
    current_level.append(this);
  }
  while (current_level.length() > 0) {
    write_dot_level(s, current_level, next_level, solution_only,
		    skip_solution_tree);
    current_level.assign_copy(next_level);
    next_level.clear();
  }

  s << "}" << std::endl;
}

void Tree::write_dot_level
(std::ostream& s, branch_vec& level, branch_vec& next, bool solution_only,
 bool skip_solution_tree)
{
  s << "{ rank = same;" << std::endl;
  for (index_type k = 0; k < level.length(); k++) {
    s << "N" << level[k]->id << " [";
    if (level[k]->root->is_max()) {
      s << "shape=box,";
    }
    else {
      s << "shape=ellipse,";
    }
    if (level[k]->x_solved) {
      s << "style=bold,";
    }
    else if (!level[k]->expanded) {
      s << "style=dashed,";
    }
    s << "label=\"";
    level[k]->root->write(s);
    if (level[k]->expanded) {
      s << ": " << level[k]->x_est_cost
	<< " (" << level[k]->root->est_cost()
	<< ")";
    }
    else {
      s << ": " << level[k]->root->est_cost();
    }
    s << "\"];" << std::endl;
  }
  s << "}" << std::endl;
  for (index_type k = 0; k < level.length(); k++) {
    if (!skip_solution_tree || !level[k]->x_solved) {
      bool non_solution_succ = false;
      for (index_type i = 0; i < level[k]->successors.length(); i++) {
	if (level[k]->successors[i]->x_solved || !solution_only) {
	  s << "N" << level[k]->id << " -> "
	    << "N" << level[k]->successors[i]->id
	    << " [label=\"" << level[k]->successors[i]->root->delta_cost()
	    << "\"";
	  if (level[k]->x_est_cost ==
	      (level[k]->successors[i]->x_est_cost +
	       level[k]->successors[i]->root->delta_cost()))
	    s << ",style=bold";
	  s << "];" << std::endl;
	  next.append(level[k]->successors[i]);
	}
	if (!level[k]->successors[i]->x_solved) non_solution_succ = true;
      }
    }
  }
}

END_HSPS_NAMESPACE
