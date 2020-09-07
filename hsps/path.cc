
#include "path.h"
#include "preprocess.h"

BEGIN_HSPS_NAMESPACE

// #define TRACE_PRINT_LOTS

Segmentation::Segmentation()
{
  // done
}

Segmentation::Segmentation(index_type l, index_type n)
{
  init(l, n);
}

void Segmentation::init(index_type l, index_type n)
{
  l_seq = l;
  n_seg = n;
  assert(l_seq > 0);
  assert(n_seg > 0);
  cn.init(l + n + 1);
  // initialise constraint graph: sequence elements obey sequence
  for (index_type k = 0; k + 1 < l_seq; k++)
    cn.add_edge(index_seq(k), index_seq(k + 1));
  // initialise constraint graph: segment starts precedes ends
  for (index_type k = 0; k < n_seg; k++)
    cn.add_edge(index_sg_start(k), index_sg_end(k));
  // initialise constraint graph: sequence contained in segmentation
  cn.add_edge(index_sg_start(0), index_seq(0));
  cn.add_edge(index_seq(l_seq - 1), index_sg_end(n_seg - 1));
  cn.transitive_closure();
}

bool Segmentation::can_be_in(index_type p, index_type s) const
{
  if (cn.adjacent(index_seq(p), index_sg_start(s))) return false;
  if (cn.adjacent(index_sg_end(s), index_seq(p))) return false;
  return true;
}

bool Segmentation::can_be_before(index_type p, index_type s) const
{
  if (cn.adjacent(index_sg_start(s), index_seq(p)))
    return false;
  else
    return true;
}

bool Segmentation::can_be_after(index_type p, index_type s) const
{
  if (cn.adjacent(index_seq(p), index_sg_end(s)))
    return false;
  else
    return true;
}

bool Segmentation::enforce_before(index_type p, index_type s, pair_set& u)
{
  if (cn.adjacent(index_sg_start(s), index_seq(p))) return false;
  cn.add_edge_to_transitive_closure(index_seq(p), index_sg_start(s), u);
  return true;
}

bool Segmentation::enforce_after(index_type p, index_type s, pair_set& u)
{
  if (cn.adjacent(index_seq(p), index_sg_end(s))) return false;
  cn.add_edge_to_transitive_closure(index_sg_end(s), index_seq(p), u);
  return true;
}

bool Segmentation::enforce_lies_during
(index_type p0, index_type p1, index_type s0, index_type s1, pair_set& u)
{
  assert(p0 <= p1);
  assert(p1 < l_seq);
  assert(s0 <= s1);
  assert(s1 < n_seg);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "segmentation: trying to enforce [" << p0 << "," << p1
	    << "] during [" << s0 << "," << s1 << "]:" << std::endl;
  std::cerr << "              sequence indices: p0 = "
	    << index_seq(p0) << ", p1 = " << index_seq(p1)
	    << std::endl;
  std::cerr << "              segment indices: start(s0) = "
	    << index_sg_start(s0) << ", end(s1) = " << index_sg_end(s1)
	    << std::endl;
  std::cerr << "              constraint graph = " << cn << std::endl;
#endif
  // add sg_start(s0) -> p0, if consistent
  if (cn.adjacent(index_seq(p0), index_sg_start(s0))) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "failed due to constraint p0 -> start(s0)" << std::endl;
#endif
    return false;
  }
  u.clear();
  cn.add_edge_to_transitive_closure(index_sg_start(s0), index_seq(p0), u);
  // add p1 -> sg_end(s1), if consistent
  if (cn.adjacent(index_sg_end(s1), index_seq(p1))) {
    cn.remove_edges(u);
#ifdef TRACE_PRINT_LOTS
    std::cerr << "failed due to constraint end(s1) -> p1" << std::endl;
#endif
    return false;
  }
  cn.add_edge_to_transitive_closure(index_seq(p1), index_sg_end(s1), u);
  return true;
}

void Segmentation::undo(const pair_set& u)
{
  cn.remove_edges(u);
}

bool Segmentation::extract(pair_vec& s)
{
  index_vec s0;
  if (!cn.top_sort(s0)) {
    std::cerr << "error: constraint graph could not be topologically sorted!"
	      << std::endl;
    std::cerr << "cn = " << cn << std::endl
	      << "s0 = " << s0 << std::endl
	      << "l = " << l_seq << ", n = " << n_seg
	      << std::endl;
    return false;
  }
  s.clear();
  index_type c_seg = no_such_index;
  index_type s_first = no_such_index;
  index_type s_last = no_such_index;
  for (index_type k = 0; k < s0.length(); k++) {
    index_type i = sg_start(s0[k]);
    index_type j = seq_element(s0[k]);
    assert((k > 0) || (i != no_such_index));
    if (i != no_such_index) {
      if (c_seg != no_such_index)
	s.append(index_pair(s_first, s_last));
      c_seg = i;
      s_first = no_such_index;
      s_last = no_such_index;
    }
    else if (j != no_such_index) {
      assert(c_seg != no_such_index);
      if (s_first == no_such_index)
	s_first = j;
      assert(s_first <= j);
      assert((s_last == no_such_index) || (s_last <= j));
      s_last = j;
    }
    else {
      // the only node that is not a segment start or a sequence element
      // is the node corresponding to the last segment end, which is the
      // highest-numbered node
      assert(s0[k] == (cn.size() - 1));
      s.append(index_pair(s_first, s_last));
    }
  }
  return true;
}

PathSet::PathSet(SASInstance& i)
  : trace_level(Preprocessor::default_trace_level), instance(i)
{
  init();
}

void PathSet::add_start_to_variable(index_type var, index_type val)
{
  assert(var < instance.n_variables());
  assert(val < instance.variables[var].n_values());
  if (vpg[var].node_has_label(val)) {
    vpg[var].node_label(val).first = true;
  }
  else {
    vpg[var].node_label(val) = swapable_pair<bool>(true, false);
  }
}

void PathSet::add_stop_to_variable(index_type var, index_type val)
{
  assert(var < instance.n_variables());
  assert(val < instance.variables[var].n_values());
  if (vpg[var].node_has_label(val)) {
    vpg[var].node_label(val).second = true;
  }
  else {
    vpg[var].node_label(val) = swapable_pair<bool>(false, true);
  }
}

bool PathSet::is_start(index_type var, index_type val) const
{
  assert(var < instance.n_variables());
  assert(val < instance.variables[var].n_values());
  if (vpg[var].node_has_label(val))
    if (vpg[var].node_label(val).first)
      return true;
  return false;
}

bool PathSet::is_stop(index_type var, index_type val) const
{
  assert(var < instance.n_variables());
  assert(val < instance.variables[var].n_values());
  if (vpg[var].node_has_label(val))
    if (vpg[var].node_label(val).second)
      return true;
  return false;
}

bool PathSet::connected
(index_type var, index_type v_start, index_type v_stop) const
{
  assert(var < instance.n_variables());
  assert(v_start < instance.variables[var].n_values());
  assert(v_stop < instance.variables[var].n_values());
  return (vpg[var].reachable(v_start, v_stop));
}

bool PathSet::possible
(index_type var, index_type v_start, index_type v_stop) const
{
  assert(var < instance.n_variables());
  assert(v_start < instance.variables[var].n_values());
  assert(v_stop < instance.variables[var].n_values());
  return (vrg[var].adjacent(v_start, v_stop));
}

bool PathSet::next_missing_connection
(index_type& var, index_type& v_start, index_type& v_stop) const
{
  index_type fv = (var == no_such_index ? 0 : var);
  index_type fv0 = (var == no_such_index ? 0 : v_start);
  index_type fv1 = (var == no_such_index ? 0 : v_stop + 1);
  for (index_type k = fv; k < instance.n_variables(); k++)
    for (index_type i = fv0; i < instance.variables[k].n_values(); i++)
      for (index_type j = fv1; j < instance.variables[k].n_values(); j++)
	if ((i != j) &&
	    is_start(k, i) &&
	    is_stop(k, j) &&
	    !connected(k, i, j) &&
	    possible(k, i, j)) {
	  var = k;
	  v_start = i;
	  v_stop = j;
	  return true;
	}
  return false;
}

void PathSet::missing_connections
(lvector< std::pair<index_type, index_pair> >& c)
{
  for (index_type k = 0; k < instance.n_variables(); k++)
    for (index_type i = 0; i < instance.variables[k].n_values(); i++)
      for (index_type j = 0; j < instance.variables[k].n_values(); j++)
	if ((i != j) &&
	    is_start(k, i) &&
	    is_stop(k, j) &&
	    !connected(k, i, j) &&
	    possible(k, i, j)) {
	  c.append(std::pair<index_type, index_pair>(k, index_pair(i, j)));
	}
}

void PathSet::add_path
(index_type var,
 index_type v_start,
 index_type v_stop,
 const ActionSequence& p)
{
  if (trace_level > 0) {
    std::cerr << "new path ";
    instance.write_action_sequence(std::cerr, p);
    std::cerr << " (" << instance.variables[var].name
	      << ": " << instance.variables[var].domain[v_start]
	      << " -> " << instance.variables[var].domain[v_stop]
	      << ")" << std::endl;
  }
  assert(var < instance.n_variables());
  assert(v_start < instance.variables[var].n_values());
  assert(v_stop < instance.variables[var].n_values());
  vpg[var].add_edge(v_start, v_stop);
  vpg[var].edge_label(v_start, v_stop).append(ActionSequence(p));
  for (index_type k = 0; k < instance.n_variables(); k++) if (k != var) {
    lvector< swapable_pair<index_pair> > s;
    sidepaths(k, p, s);
    for (index_type i = 0; i < s.length(); i++) {
      if (trace_level > 0) {
	if (!is_stop(k, s[i].second.first)) {
	  std::cerr << "new stop: " << instance.variables[k].name
		    << " = " << instance.variables[k].domain[s[i].second.first]
		    << std::endl;
	}
      }
      add_stop_to_variable(k, s[i].second.first);
      if (trace_level > 0) {
	if (!is_start(k, s[i].second.second)) {
	  std::cerr << "new start: " << instance.variables[k].name << " = "
		    << instance.variables[k].domain[s[i].second.second]
		    << std::endl;
	}
      }
      add_start_to_variable(k, s[i].second.second);
    }
  }
}

index_type PathSet::path_first_start
(index_type var, ActionSequence& p, index_type from)
{
  for (index_type k = from; k < p.length(); k++) {
    assert(p[k] < instance.n_actions());
    index_type val = instance.actions[p[k]].pre.value_of(var);
    if (is_start(var, val))
      return k;
  }
  return no_such_index;
}

index_type PathSet::path_last_stop
(index_type var, ActionSequence& p, index_type to)
{
  for (index_type k = to + 1; k > 0; k--) {
    assert(p[k - 1] < instance.n_actions());
    index_type val = instance.actions[p[k - 1]].post.value_of(var);
    if (is_stop(var, val))
      return k - 1;
  }
  return no_such_index;
}

void PathSet::remove_composed_paths(index_type var)
{
  for (index_type i = 0; i < instance.variables[var].n_values(); i++)
    for (index_type j = 0; j < instance.variables[var].n_values(); j++)
      if ((i != j) && vpg[var].adjacent(i, j)) {
	assert(vpg[var].edge_has_label(i, j));
	ActionSequenceSet& ps = vpg[var].edge_label(i, j);
	index_type k = 0;
	while (k < ps.length()) {
	  bool is_composed = false;
	  if (ps[k].length() > 2) {
	    index_type i_start = path_first_start(var, ps[k], 1);
	    index_type i_stop = path_last_stop(var, ps[k], ps[k].length() - 2);
	    if ((i_start != no_such_index) &&
		(i_stop != no_such_index) &&
		(i_start <= i_stop))
	      is_composed = true;
	  }
	  if (is_composed) {
	    if (trace_level > 0) {
	      std::cerr << "removing composed path ";
	      instance.write_action_sequence(std::cerr, ps[k]);
	      std::cerr << " in " << instance.variables[var].name
			<< std::endl;
	    }
	    ps.remove(k);
	  }
	  else {
	    k += 1;
	  }
	}
	if (ps.empty()) {
	  vpg[var].remove_edge(i, j);
	}
      }
}

void PathSet::find_minimal_paths
(index_type var, index_type v_start, index_type v_stop,
 ActionSequence& p, index_vec& v, bool s_pass, ActionSequenceSet& ps)
{
  if (v_start == v_stop) {
    assert(p.length() > 0);
    ps.append(p);
    return;
  }
  for (index_type k = 0; k < path_act[var].length(); k++) {
    index_type a = path_act[var][k];
    index_type v_pre = instance.actions[a].pre.value_of(var);
    assert(v_pre != no_such_index);
    if (v_pre == v_start) {
      index_type v_post = instance.actions[a].post.value_of(var);
      assert(v_post != no_such_index);
      bool ok = (!v.contains(v_post));
      bool pass_2 = s_pass;
      if (ok && (p.length() > 0) && (v_post != v_stop)) {
	if (is_start(var, v_pre))
	  pass_2 = true;
	if (pass_2 && is_stop(var, v_post))
	  ok = false;
      }
      if (ok) {
	p.append(a);
	v.append(v_post);
	find_minimal_paths(var, v_post, v_stop, p, v, pass_2, ps);
	v.dec_length();
	p.dec_length();
      }
    }
  }
}

void PathSet::find_minimal_paths
(index_type var, index_type v_start, index_type v_stop, ActionSequenceSet& ps)
{
  ps.clear();
  ActionSequence p;
  index_vec v;
  v.append(v_start);
  find_minimal_paths(var, v_start, v_stop, p, v, false, ps);
}

index_pair PathSet::core
(index_type var,
 const ActionSequence& p,
 const swapable_pair<index_pair>& s)
{
  assert(s.first.first < p.length());
  assert(s.first.second < p.length());
  assert(s.first.first <= s.first.second);
  index_type c_start = no_such_index;
  index_type c_end = no_such_index;
  for (index_type k = s.first.first; k <= s.first.second; k++) {
    index_type v_post = instance.actions[p[k]].post.value_of(var);
    if (v_post != no_such_index) {
      if (c_start == no_such_index)
	c_start = k;
      c_end = k;
    }
  }
  return index_pair(c_start, c_end);
}

void PathSet::fringes
(index_type var,
 const ActionSequence& p,
 const swapable_pair<index_pair>& s,
 index_set& f)
{
  index_pair c = core(var, p, s);
  // end of (= first position after) first fringe
  index_type ff_end =
    (c.first == no_such_index ? s.first.second + 1 : c.first);
  // start of (= last position before) second fringe
  index_type sf_start =
    (c.second == no_such_index ? ff_end : c.second);

  f.clear();
  for (index_type i = s.first.first; i < ff_end; i++)
    f.append(i);
  if (sf_start != no_such_index) {
    for (index_type i = sf_start + 1; i <= s.first.second; i++)
      f.append(i);
  }
}

void PathSet::sidepaths
(index_type var,
 const ActionSequence& p,
 lvector< swapable_pair<index_pair> >& s)
{
  index_type c_start = no_such_index;
  index_type c_end = no_such_index;
  index_type s_val = no_such_index;
  index_type c_val = no_such_index;
  index_type i = 0;
  while (i < p.length()) {
    index_type v_pre =
      instance.actions[p[i]].pre.value_of(var);
    if (v_pre == no_such_index)
      v_pre = instance.actions[p[i]].prv.value_of(var);
    index_type v_post =
      instance.actions[p[i]].post.value_of(var);
    if (v_post == no_such_index)
      v_post = v_pre;
    if ((c_val == no_such_index) && (v_pre != no_such_index)) {
      c_start = i;
      c_end = i;
      s_val = v_pre;
      c_val = v_post;
    }
    else if (v_pre != no_such_index) {
      if (v_pre != c_val) {
	s.append(swapable_pair<index_pair>
		 (index_pair(c_start, c_end), index_pair(s_val, c_val)));
	c_start = i;
	c_end = i;
	s_val = v_pre;
	c_val = v_post;
      }
      else {
	c_val = v_post;
	c_end = i;
      }
    }
    i += 1;
  }
  if (c_val != no_such_index) {
    s.append(swapable_pair<index_pair>
	     (index_pair(c_start, c_end), index_pair(s_val, c_val)));
  }
}

bool PathSet::segment
(index_type var,
 const ActionSequence& p1,
 const ActionSequence& p2,
 index_set& f,
 index_type i,
 index_type v_prv,
 index_type j,
 Segmentation& sg,
 pair_vec& s)
{
  while (j < p2.length()) {
    // if action j of p2 can lie in segment corresponding to f[i] of p1...
    if (sg.can_be_in(j, f[i])) {
      index_type v = instance.actions[p2[j]].prv.value_of(var);
      // if p2[j] has a prevail condition on var, different from v_prv...
      if ((v != no_such_index) && (v != v_prv)) {
	// ...then we have to put it either before or after
	if (sg.can_be_before(j, f[i])) {
	  pair_set u;
	  sg.enforce_before(j, f[i], u);
	  bool ok = segment(var, p1, p2, f, i, v_prv, j + 1, sg, s);
	  sg.undo(u);
	  if (ok) return true;
	}
	if (sg.can_be_after(j, f[i])) {
	  pair_set u;
	  sg.enforce_after(j, f[i], u);
	  bool ok = segment(var, p1, p2, f, i, v_prv, j + 1, sg, s);
	  sg.undo(u);
	  if (ok) return true;
	}
	// ...and if we couldn't do either, then there's no solution
	return false;
      }
      v = instance.actions[p2[j]].pre.value_of(var);
      // else, if p2[j] has a precondition on var...
      if (v != no_such_index) {
	// if this precondition equals v_prv, we have the option of
	// putting a whole persistent sequence during segment f[i]...
	if (v == v_prv) {
	  index_type k = j + 1;
	  while (k < p2.length()) {
	    index_type v_end = instance.actions[p2[k]].post.value_of(var);
	    if ((v_end == v_prv) && sg.can_be_in(k, f[i])) {
	      pair_set u;
	      if (sg.enforce_lies_during(j, k, f[i], f[i], u)) {
		bool ok = segment(var, p1, p2, f, i, v_prv, k + 1, sg, s);
		sg.undo(u);
		if (ok) return true;
	      }
	    }
	    k += 1;
	  }
	}
	// if the precondition did not equal v_prv, or if that option failed,
	// we have the options of putting p2[j] before or after segment f[i]
	if (sg.can_be_before(j, f[i])) {
	  pair_set u;
	  sg.enforce_before(j, f[i], u);
	  bool ok = segment(var, p1, p2, f, i, v_prv, j + 1, sg, s);
	  sg.undo(u);
	  if (ok) return true;
	}
	if (sg.can_be_after(j, f[i])) {
	  pair_set u;
	  sg.enforce_after(j, f[i], u);
	  bool ok = segment(var, p1, p2, f, i, v_prv, j + 1, sg, s);
	  sg.undo(u);
	  if (ok) return true;
	}
	// ...and if we couldn't do any of them, then there's no solution
	return false;
      }
    }
    j += 1;
  }
  // if we get here, we didn't have to branch
  if (i + 1 < f.length()) {
    index_type v_prv = instance.actions[p1[f[i + 1]]].prv.value_of(var);
    assert(v_prv != no_such_index);
    return segment(var, p1, p2, f, i + 1, v_prv, 0, sg, s);
  }
  else {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "segment: ok so far, next variable is " << var + 1
	      << std::endl;
#endif
    return segment(var + 1, p1, p2, sg, s);
  }
}

bool PathSet::segment
(index_type v,
 const ActionSequence& p1,
 const ActionSequence& p2,
 const lvector< swapable_pair<index_pair> >& sp1,
 bool_vec& m,
 const lvector< swapable_pair<index_pair> >& sp2,
 index_type i,
 Segmentation& sg,
 pair_vec& s)
{
  if (i < sp2.length()) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "segment: mapping sidepath "
	      << i + 1 << " of " << sp2.length() << ": "
	      << sp2[i].first.first << "."
	      << instance.variables[v].domain[sp2[i].second.first]
	      << " -> "
	      << sp2[i].first.second << "."
	      << instance.variables[v].domain[sp2[i].second.second]
	      << "..." << std::endl;
#endif
    for (index_type j = 0; j < sp1.length(); j++) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "segment: candidate "
		<< j + 1 << " of " << sp1.length() << ": "
		<< sp1[j].first.first << "."
		<< instance.variables[v].domain[sp1[j].second.first]
		<< " -> "
		<< sp1[j].first.second << "."
		<< instance.variables[v].domain[sp1[j].second.second]
		<< " (mapped = " << m[j] << ")..." << std::endl;
#endif
      if (!m[j] &&
	  (sp1[j].second.first == sp2[i].second.first) &&
	  (sp1[j].second.second == sp2[i].second.second)) {
	pair_set u;
	bool ok = sg.enforce_lies_during
	  (sp2[i].first.first, sp2[i].first.second,
	   sp1[j].first.first, sp1[j].first.second, u);
	if (ok) {
	  m[j] = true;
	  ok = segment(v, p1, p2, sp1, m, sp2, i + 1, sg, s);
	  m[j] = false;
	  sg.undo(u);
	  if (ok) return true;
	}
#ifdef TRACE_PRINT_LOTS
	else {
	  std::cerr << "segment: can't place " << sp2[i].first
		    << " during " << sp1[j].first << std::endl;
	}
#endif
      }
    }
    return false;
  }
  else {
    // check that all unmapped sidepaths of p1 are circular
    for (index_type j = 0; j < sp1.length(); j++)
      if (!m[j] && (sp1[j].second.first != sp1[j].second.second)) {
#ifdef TRACE_PRINT_LOTS
	std::cerr << "segment: unmapped sidepath " << sp1[j]
		  << " is non-cyclic" << std::endl;
#endif
	return false;
      }
    // check persistence condition for all actions on the fringes of
    // sidepaths of p1
    index_set f;
    for (index_type j = 0; j < sp1.length(); j++) {
      index_set f0;
      fringes(v, p1, sp1[j], f0);
      f.insert(f0);
    }
#ifdef TRACE_PRINT_LOTS
    std::cerr << "sidepaths of P1 = ";
    instance.write_action_sequence(std::cerr, p1);
    std::cerr << " w.r.t. " << instance.variables[v].name << ": ";
    for (index_type j = 0; j < sp1.length(); j++) {
      if (j > 0) std::cerr << ", ";
      std::cerr << sp1[j].first.first << "."
		<< instance.variables[v].domain[sp1[j].second.first]
		<< " -> "
		<< sp1[j].first.second << "."
		<< instance.variables[v].domain[sp1[j].second.second];
      index_pair c = core(v, p1, sp1[j]);
      std::cerr << " (core = " << c << ")";
    }
    std::cerr << std::endl;
    std::cerr << "fringe positions: " << f << std::endl;
#endif
    if (!f.empty()) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "segment: checking persistence of "
		<< instance.variables[var].name << " at positions "
		<< f << " in P1..." << std::end;
#endif
      index_type v_prv = instance.actions[p1[f[0]]].prv.value_of(v);
      assert(v_prv != no_such_index);
      return segment(v, p1, p2, f, 0, v_prv, 0, sg, s);
    }
    else {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "segment: ok so far, next variable is "
		<< v + 1 << std::endl;
#endif
      return segment(v + 1, p1, p2, sg, s);
    }
  }
}

bool PathSet::segment
(index_type v,
 const ActionSequence& p1,
 const ActionSequence& p2,
 Segmentation& sg,
 pair_vec& s)
{
  if (v < instance.n_variables()) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "segment: checking sidepaths w.r.t. "
	      << instance.variables[v].name
	      << "..." << std::endl;
#endif
    lvector< swapable_pair<index_pair> > sp1;
    lvector< swapable_pair<index_pair> > sp2;
    sidepaths(v, p1, sp1);
    bool_vec m(false, sp1.length());
    sidepaths(v, p2, sp2);
#ifdef TRACE_PRINT_LOTS
    std::cerr << "segment: sidepaths of P1: " << sp1 << std::endl;
    std::cerr << "segment: sidepaths of P2: " << sp2 << std::endl;
#endif
    return segment(v, p1, p2, sp1, m, sp2, 0, sg, s);
  }
  else {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "segment: extracting solution..." << std::endl;
#endif
    bool ok = sg.extract(s);
    assert(ok);
#ifdef TRACE_PRINT_LOTS
    std::cerr << "segment: solution = " << s << std::endl;
#endif
    return true;
  }
}

bool PathSet::segment
(const ActionSequence& p1,
 const ActionSequence& p2,
 pair_vec& sg)
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "segmenting path ";
  instance.write_action_sequence(std::cerr, p2);
  std::cerr << " to match path ";
  instance.write_action_sequence(std::cerr, p1);
  std::cerr << "..." << std::endl;
#endif
  Segmentation sgc(p2.length(), p1.length());
  bool ok = segment(0, p1, p2, sgc, sg);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "segment: finished, ok = " << ok << std::endl;
#endif
  return ok;
}

bool PathSet::can_replace
(const ActionSequence& p1, const ActionSequence& p2)
{
  pair_vec sg;
  bool ok = segment(p1, p2, sg);
  if ((trace_level > 0) && ok) {
    std::cerr << "path ";
    instance.write_action_sequence(std::cerr, p2);
    std::cerr << " corresponds to path ";
    instance.write_action_sequence(std::cerr, p1);
    std::cerr << " under segmentation " << sg << std::endl;
  }
  return ok;
}

void PathSet::filter_path_set
(const ActionSequenceSet& p, index_set& s)
{
  if (p.length() < 2) {
    s.fill(p.length());
    return;
  }

  graph d(p.length());
  for (index_type i = 0; i < p.length(); i++)
    for (index_type j = 0; j < p.length(); j++)
      if (i != j)
	if (can_replace(p[i], p[j]))
	  d.add_edge(j, i);

  d.strongly_connected_components();
  equivalence e;
  d.component_partitioning(e);
  index_set_graph c(d, e);

  s.clear();
  for (index_type k = 0; k < c.size(); k++)
    if (c.in_degree(k) == 0) {
      assert(c.node_has_label(k));
      assert(c.node_label(k).length() > 0);
      assert(c.node_label(k)[0] < p.length());
      index_type i = 0;
      for (index_type j = 1; j < c.node_label(k).length(); j++)
	if (p[j].length() < p[i].length())
	  i = j;
      s.insert(c.node_label(k)[i]);
    }
}

void PathSet::init()
{
  vpg.set_length(instance.n_variables());
  for (index_type k = 0; k < instance.n_variables(); k++) {
    vpg[k].init(instance.variables[k].n_values());
    index_type v_init = instance.init_state.value_of(k);
    assert(v_init != no_such_index);
    add_start_to_variable(k, v_init);
    index_type v_goal = instance.goal_state.value_of(k);
    if (v_goal != no_such_index)
      add_stop_to_variable(k, v_goal);
  }
  vrg.set_length(instance.n_variables());
  for (index_type k = 0; k < instance.n_variables(); k++)
    vrg[k].init(instance.variables[k].n_values());
  path_act.assign_value(EMPTYSET, instance.n_variables());
  bool is_safe = true;
  for (index_type k = 0; k < instance.n_actions(); k++)
    for (index_type i = 0; i < instance.actions[k].post.length(); i++) {
      index_type v = instance.actions[k].post[i].first;
      index_type v_post = instance.actions[k].post[i].second;
      index_type v_pre = instance.actions[k].pre.value_of(v);
      if (v_pre == no_such_index) {
	std::cerr << "error: action " << instance.actions[k].name
		  << " is not safe w.r.t. variable "
		  << instance.variables[v].name
		  << std::endl;
	is_safe = false;
      }
      if (v_pre != no_such_index)
	vrg[v].add_edge(v_pre, v_post);
      path_act[instance.actions[k].post[i].first].insert(k);
    }
  if (!is_safe) {
    exit(255);
  }
  for (index_type k = 0; k < instance.n_variables(); k++)
    vrg[k].transitive_closure();
}

bool PathSet::closed()
{
  index_type var = no_such_index;
  index_type v_start = no_such_index;
  index_type v_stop = no_such_index;
  return next_missing_connection(var, v_start, v_stop);
}

void PathSet::compute()
{
  index_type n = 0;
  bool done = false;
  while (!done) {
    done = true;
    n += 1;
    lvector< std::pair<index_type, index_pair> > missing;
    missing_connections(missing);
    std::cerr << "iteration " << n
	      << ": " << missing.length() << " connections missing"
	      << std::endl;
    for (index_type k = 0; k < missing.length(); k++) {
      index_type var = missing[k].first;
      index_type v_start = missing[k].second.first;
      index_type v_stop = missing[k].second.second;
      if (trace_level > 0) {
	std::cerr << "missing connection: "
		  << instance.variables[var].domain[v_start]
		  << " -> " << instance.variables[var].domain[v_stop]
		  << " in " << instance.variables[var].name
		  << std::endl;
      }
      ActionSequenceSet new_path_set;
      find_minimal_paths(var, v_start, v_stop, new_path_set);
      if (new_path_set.length() > 0) {
#ifdef TRACE_PRINT_LOTS
	std::cerr << new_path_set.length() << " minimal paths found:"
		  << std::endl;
	for (index_type p = 0; p < new_path_set.length(); p++) {
	  std::cerr << p << ". ";
	  instance.write_action_sequence(std::cerr, new_path_set[p]);
	  std::cerr << std::endl;
	  for (index_type i = 0; i < instance.n_variables(); i++)
	    if (i != var) {
	      lvector< swapable_pair<index_pair> > sp;
	      sidepaths(i, new_path_set[p], sp);
	      if (sp.length() > 0) {
		std::cerr << " sidepaths in "
			  << instance.variables[i].name << ": ";
		for (index_type j = 0; j < sp.length(); j++) {
		  if (j > 0) std::cerr << ", ";
		  std::cerr << sp[j].first.first << "."
			    << instance.variables[i].domain[sp[j].second.first]
			    << " -> "
			    << sp[j].first.second << "."
			  << instance.variables[i].domain[sp[j].second.second];
		}
		std::cerr << std::endl;
	      }
	    }
	}
#endif
	index_set f;
	filter_path_set(new_path_set, f);
#ifdef TRACE_PRINT_LOTS
	std::cerr << "selected path set: " << f << std::endl;
#endif
	for (index_type k = 0; k < f.length(); k++) {
	  assert(f[k] < new_path_set.length());
	  add_path(var, v_start, v_stop, new_path_set[f[k]]);
	}
	done = false;
      }
    }
    if (!done) {
      for (index_type k = 0; k < instance.n_variables(); k++)
	remove_composed_paths(k);
    }
  }
}

void PathSet::actions(bool_vec& acts)
{
  acts.assign_value(false, instance.n_actions());
  for (index_type k = 0; k < instance.n_variables(); k++)
    for (index_type i = 0; i < instance.variables[k].n_values(); i++)
      for (index_type j = 0; j < instance.variables[k].n_values(); j++)
	if (vpg[k].adjacent(i, j) &&  vpg[k].edge_has_label(i, j)) {
	  ActionSequenceSet& ps = vpg[k].edge_label(i, j);
	  for (index_type n = 0; n < ps.length(); n++)
	    for (index_type l = 0; l < ps[n].length(); l++)
	      acts[ps[n][l]] = true;
	}
}

END_HSPS_NAMESPACE
