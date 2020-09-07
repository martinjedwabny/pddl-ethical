
#ifndef PATHS_H
#define PATHS_H

#include "sas.h"
#include "plans.h"

BEGIN_HSPS_NAMESPACE

class Segmentation {
  index_type l_seq;
  index_type n_seg;
  graph      cn;

  // functions mapping between segment starts/ends / sequence elements
  // and nodes in the constraint graph
  index_type index_sg_start(index_type s) const
    { assert(s < n_seg); return (l_seq + s); };
  index_type index_sg_end(index_type s) const
    { assert(s < n_seg); return (l_seq + s + 1); };
  index_type index_seq(index_type p) const
    { assert(p < l_seq); return p; };

  index_type sg_start(index_type n) const
    { if (n >= l_seq) return (n - l_seq); else return no_such_index; };
  index_type seq_element(index_type n) const
    { if (n < l_seq) return n; else return no_such_index; };

 public:
  Segmentation();
  Segmentation(index_type l, index_type n);

  void init(index_type l, index_type n);

  bool can_be_in(index_type p, index_type s) const;
  bool can_be_before(index_type p, index_type s) const;
  bool can_be_after(index_type p, index_type s) const;

  bool enforce_before(index_type p, index_type s, pair_set& u);
  bool enforce_after(index_type p, index_type s, pair_set& u);
  bool enforce_lies_during(index_type p0, index_type p1,
			   index_type s0, index_type s1,
			   pair_set& u);
  void undo(const pair_set& u);

  bool extract(pair_vec& s);
};

class PathSet {
  typedef labeled_graph<swapable_pair<bool>,ActionSequenceSet> PathGraph;
  typedef lvector<PathGraph> PathGraphVec;

  SASInstance&  instance;
  PathGraphVec  vpg; // variable path-graphs
  graph_vec     vrg; // variable reachability graphs
  index_set_vec path_act;

  bool is_start(index_type var, index_type val) const;
  bool is_stop(index_type var, index_type val) const;
  bool connected(index_type var, index_type v_start, index_type v_stop) const;
  bool possible(index_type var, index_type v_start, index_type v_stop) const;
  bool next_missing_connection(index_type& var,
			       index_type& v_start,
			       index_type& v_stop) const;
  void missing_connections(lvector< std::pair<index_type, index_pair> >& c);

  void add_start_to_variable(index_type var, index_type val);
  void add_stop_to_variable(index_type var, index_type val);
  void add_path(index_type var,
		index_type v_start,
		index_type v_stop,
		const ActionSequence& p);

  index_type path_first_start(index_type var,
			      ActionSequence& p,
			      index_type from);
  index_type path_last_stop(index_type var,
			    ActionSequence& p,
			    index_type to);
  void remove_composed_paths(index_type var);

  void find_minimal_paths(index_type var,
			  index_type v_start,
			  index_type v_stop,
			  ActionSequence& p,
			  index_vec& v,
			  bool s_pass,
			  ActionSequenceSet& ps);
  void find_minimal_paths(index_type var,
			  index_type v_start,
			  index_type v_stop,
			  ActionSequenceSet& ps);

  void sidepaths(index_type var,
		 const ActionSequence& p,
		 lvector< swapable_pair<index_pair> >& s);
  index_pair core(index_type var,
		  const ActionSequence& p,
		  const swapable_pair<index_pair>& s);
  void fringes(index_type var,
	       const ActionSequence& p,
	       const swapable_pair<index_pair>& s,
	       index_set& f);

  bool segment(index_type v,
	       const ActionSequence& p1,
	       const ActionSequence& p2,
	       index_set& f,
	       index_type i,
	       index_type v_prv,
	       index_type j,
	       Segmentation& sg,
	       pair_vec& s);
  bool segment(index_type v,
	       const ActionSequence& p1,
	       const ActionSequence& p2,
	       const lvector< swapable_pair<index_pair> >& sp1,
	       bool_vec& m,
	       const lvector< swapable_pair<index_pair> >& sp2,
	       index_type i,
	       Segmentation& sg,
	       pair_vec& s);
  bool segment(index_type v,
	       const ActionSequence& p1,
	       const ActionSequence& p2,
	       Segmentation& sg,
	       pair_vec& s);
  bool segment(const ActionSequence& p1,
	       const ActionSequence& p2,
	       pair_vec& sg);
  bool can_replace(const ActionSequence& p1,
		   const ActionSequence& p2);

  void filter_path_set(const ActionSequenceSet& p,
		       index_set& s);

  void extend();

 public:
  int trace_level;

  PathSet(SASInstance& instance);

  void init();
  bool closed();
  void compute();

  void actions(bool_vec& acts);
};

END_HSPS_NAMESPACE

#endif
