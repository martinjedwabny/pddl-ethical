
#include "graph.h"

BEGIN_HSPS_NAMESPACE

graph::graph()
  : _size(0), comp()
{
  // done
}

graph::graph(index_type s)
  : _size(0), comp()
{
  init(s);
}

graph::graph(const graph& g)
  : _size(0), comp()
{
  copy(g);
}

graph::graph(const graph& g, const index_set& n)
  : _size(0), comp()
{
  g.subgraph(*this, n);
}

graph::graph(const graph& g, const equivalence& eq)
  : _size(0), comp()
{
  g.quotient(*this, eq);
}

graph::~graph()
{
  // done
}

bool graph::empty() const
{
  for (index_type i = 0; i < _size; i++)
    for (index_type j = 0; j < _size; j++)
      if (adj[i][j]) return false;
  return true;
}

bool graph::reachable(index_type n0, index_type n1) const
{
  assert(n0 < _size);
  assert(n1 < _size);
  bool_vec v(false, _size);
  reachable(n0, v);
  return v[n1];
}

void graph::reachable_set(index_type n0, index_set& s) const
{
  bool_vec v(false, _size);
  reachable(n0, v);
  v.copy_to(s);
}

index_type graph::count_reachable(index_type n0) const
{
  bool_vec v(false, _size);
  reachable(n0, v);
  return v.count(true);
}

void graph::reachable(bool_vec& v) const
{
  for (index_type k = 0; k < _size; k++)
    if (v[k])
      reachable(k, v);
}

bool graph::acyclic() const
{
  bool_vec _reach(false, _size);
  for (index_type k = 0; k < _size; k++) {
    _reach.assign_value(false, _size);
    for (index_type i = 0; i < out[k].length(); i++)
      reachable(out[k][i], _reach);
    if (_reach[k]) return false;
  }
  return true;
}

bool graph::top_sort(index_vec& s) const
{
  s.clear();
  // std::cerr << "g = " << *this << std::endl;
  bool_vec rem(true, size());
  while (rem.count(true) > 0) {
    // std::cerr << "rem = " << rem << std::endl;
    index_type n = no_such_index;
    for (index_type i = 0; (i < size()) && (n == no_such_index); i++)
      if (rem[i] && (in[i].count_common(rem) == 0))
	n = i;
    // std::cerr << "n = " << n << std::endl;
    if (n == no_such_index)
      return false;
    s.append(n);
    rem[n] = false;
  }
  return true;
}

index_type graph::max_out_degree() const
{
  assert(_size > 0);
  index_type m = out[0].length();
  for (index_type k = 1; k < _size; k++)
    if (out[k].length() > m) m = out[k].length();
  return m;
}

index_type graph::max_in_degree() const
{
  assert(_size > 0);
  index_type m = in[0].length();
  for (index_type k = 1; k < _size; k++)
    if (in[k].length() > m) m = in[k].length();
  return m;
}

index_type graph::max_bi_degree() const
{
  assert(_size > 0);
  index_type m = bi[0].length();
  for (index_type k = 1; k < _size; k++)
    if (bi[k].length() > m) m = bi[k].length();
  return m;
}

index_type graph::min_out_degree() const
{
  assert(_size > 0);
  index_type m = out[0].length();
  for (index_type k = 1; k < _size; k++)
    if (out[k].length() < m) m = out[k].length();
  return m;
}

index_type graph::min_in_degree() const
{
  assert(_size > 0);
  index_type m = in[0].length();
  for (index_type k = 1; k < _size; k++)
    if (in[k].length() < m) m = in[k].length();
  return m;
}

index_type graph::min_bi_degree() const
{
  assert(_size > 0);
  index_type m = bi[0].length();
  for (index_type k = 1; k < _size; k++)
    if (bi[k].length() < m) m = bi[k].length();
  return m;
}

index_type graph::first_root() const
{
  for (index_type k = 0; k < size(); k++)
    if (in_degree(k) == 0) return k;
  return no_such_index;
}

index_type graph::first_leaf() const
{
  for (index_type k = 0; k < size(); k++)
    if (out_degree(k) == 0) return k;
  return no_such_index;
}

void graph::fringe(const index_set& n, index_set& fn) const
{
  fn.clear();
  for (index_type k = 0; k < n.length(); k++) {
    fn.insert(successors(n[k]));
  }
  fn.subtract(n);
}

void graph::bi_fringe(const index_set& n, index_set& fn) const
{
  fn.clear();
  for (index_type k = 0; k < n.length(); k++) {
    fn.insert(bidirectional(n[k]));
  }
  fn.subtract(n);
}

void graph::distance(index_type s0, index_vec& d) const
{
  index_set s0s;
  s0s.assign_singleton(s0);
  distance(s0s, d);
}

void graph::distance(const index_set& s0, index_vec& d) const
{
  d.assign_value(no_such_index, size());
  for (index_type k = 0; k < s0.length(); k++) {
    assert(s0[k] < size());
    d[s0[k]] = 0;
  }
  bool done = false;
  while (!done) {
    done = true;
    for (index_type i = 0; i < size(); i++)
      for (index_type j = 0; j < out[i].length(); j++)
	if (d[i] != no_such_index) {
	  if ((d[i] + 1) < d[out[i][j]]) {
	    d[out[i][j]] = d[i] + 1;
	    done = false;
	  }
	}
  }
}

index_type graph::distance(index_type s0, index_type s1) const
{
  index_vec d;
  distance(s0, d);
  return d[s1];
}

pair_set& graph::bidirectional_edges(pair_set& s) const
{
  s.clear();
  for (index_type i = 0; i < _size; i++)
    for (index_type j = i+1; j < _size; j++)
      if (adj[i][j] && adj[j][i]) s.insert(index_pair(i, j));
  return s;
}

bool graph::adjacent(index_type i, const index_set& n) const
{
  for (index_type j = 0; j < n.length(); j++)
    if (adjacent(i, n[j])) return true;
  return false;
}

bool graph::adjacent(const index_set& n, index_type i) const
{
  for (index_type j = 0; j < n.length(); j++)
    if (adjacent(n[j], i)) return true;
  return false;
}

bool graph::adjacent(const index_set& n0, const index_set& n1) const
{
  for (index_type i = 0; i < n0.length(); i++)
    for (index_type j = 0; j < n1.length(); j++)
      if (adjacent(n0[i], n1[j])) return true;
  return false;
}

bool graph::bi_adjacent(index_type i, const index_set& n) const
{
  for (index_type j = 0; j < n.length(); j++)
    if (bi_adjacent(i, n[j])) return true;
  return false;
}

index_type graph::n_edges() const
{
  index_type n = 0;
  for (index_type i = 0; i < _size; i++)
    for (index_type j = 0; j < _size; j++)
      if (adj[i][j]) n += 1;
  return n;
}

pair_set& graph::edges(pair_set& s) const
{
  s.clear();
  for (index_type i = 0; i < _size; i++)
    for (index_type j = 0; j < _size; j++)
      if (adj[i][j]) s.insert(index_pair(i, j));
  return s;
}

index_type graph::n_induced_undirected_edges() const
{
  index_type n = 0;
  for (index_type i = 0; i < _size; i++)
    for (index_type j = i; j < _size; j++)
      if (adj[i][j] || adj[j][i]) n += 1;
  return n;
}

index_type graph::n_bidirectional_edges() const
{
  index_type n = 0;
  for (index_type i = 0; i < _size; i++)
    for (index_type j = i; j < _size; j++)
      if (adj[i][j] && adj[j][i]) n += 1;
  return n;
}

index_type graph::component_node(index_type i) const
{
  for (index_type k = 0; k < _size; k++)
    if (comp[k] == i) return k;
  return no_such_index;
}

index_type graph::component_size(index_type i) const
{
  index_type n = 0;
  for (index_type k = 0; k < _size; k++)
    if (comp[k] == i) n += 1;
  return n;
}

void graph::component_node_set(index_type i, index_set& set) const
{
  set.clear();
  for (index_type k = 0; k < _size; k++)
    if (comp[k] == i) set.insert(k);
}

index_type graph::maximal_non_unit_component() const
{
  index_type k_max = no_such_index;
  index_type s_max = 1;
  for (index_type k = 0; k < n_components(); k++)
    if (component_size(k) > s_max) {
      k_max = k;
      s_max = component_size(k);
    }
  return k_max;
}

bool graph::equals(const graph& g) const
{
  if (g.size() != size()) return false;
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j) != g.adjacent(i, j)) return false;
  return true;
}

bool graph::equals(const graph& g, const index_vec& c) const
{
  if (g.size() != size()) return false;
  if (c.length() != g.size()) {
    ::std::cerr << "error: size of graph " << g
	      << " and permutation vector " << c
	      << " do not agree"
	      << ::std::endl;
    exit(255);
  }
  for (index_type i = 0; i < size(); i++) {
    assert(c[i] < g.size());
    for (index_type j = 0; j < size(); j++) {
      assert(c[j] < g.size());
      if (adjacent(i, j) != g.adjacent(c[i], c[j])) return false;
    }
  }
  return true;
}

void graph::difference
(const graph& g, const index_vec& c, pair_set& d0, pair_set& d1) const
{
  assert(g.size() == size());
  assert(c.length() == g.size());
  d0.clear();
  d1.clear();
  for (index_type i = 0; i < size(); i++) {
    assert(c[i] < g.size());
    for (index_type j = 0; j < size(); j++) {
      assert(c[j] < g.size());
      if (adjacent(i, j) && !g.adjacent(c[i], c[j]))
	d0.insert(index_pair(i, j));
      if (!adjacent(i, j) && g.adjacent(c[i], c[j]))
	d1.insert(index_pair(c[i], c[j]));
    }
  }
}

void graph::difference
(const graph& g, pair_set& d0, pair_set& d1) const
{
  assert(g.size() == size());
  d0.clear();
  d1.clear();
  for (index_type i = 0; i < size(); i++) {
    for (index_type j = 0; j < size(); j++) {
      if (adjacent(i, j) && !g.adjacent(i, j))
	d0.insert(index_pair(i, j));
      if (!adjacent(i, j) && g.adjacent(i, j))
	d1.insert(index_pair(i, j));
    }
  }
}

index_type graph::cardinality_of_difference(const graph& g) const
{
  assert(g.size() == size());
  index_type d = 0;
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adj[i][j] != g.adj[i][j]) d += 1;
  return d;
}

void graph::init(index_type size)
{
  _size = size;
  adj.assign_value(false, _size, _size);
  in.assign_value(EMPTYSET, _size);
  out.assign_value(EMPTYSET, _size);
  bi.assign_value(EMPTYSET, _size);
  comp.assign_value(0, _size);
  n_comp = 0;
}

void graph::copy(const graph& g)
{
  _size = g._size;
  adj.assign_copy(g.adj);
  in.assign_copy(g.in);
  out.assign_copy(g.out);
  bi.assign_copy(g.bi);
  comp.assign_copy(g.comp);
  n_comp = g.n_comp;
}

void graph::copy(const graph& g, const index_vec& map)
{
  assert(map.length() == g.size());
  index_type m = mapping::range(map, map.length());
  init(m);
  for (index_type i = 0; i < g.size(); i++)
    if (map[i] != no_such_index) {
      assert(map[i] < size());
      for (index_type j = 0; j < g.size(); j++)
	if (map[j] != no_such_index) {
	  assert(map[j] < _size);
	  if (g.adjacent(i, j))
	    add_edge(map[i], map[j]);
	}
    }
}

void graph::copy_and_rename(const graph& g, const index_vec& map)
{
  assert(map.length() == g.size());
  init(g.size());
  for (index_type i = 0; i < _size; i++) {
    assert(map[i] < _size);
    for (index_type j = 0; j < _size; j++) {
      assert(map[j] < _size);
      if (g.adjacent(i, j))
	add_edge(map[i], map[j]);
    }
  }
}

index_type graph::add_node()
{
  adj.set_size(_size + 1, _size + 1);
  for (index_type k = 0; k < _size; k++) {
    adj[k][_size] = false;
    adj[_size][k] = false;
  }
  adj[_size][_size] = false;

  in.set_length(_size + 1);
  in[_size].clear();
  out.set_length(_size + 1);
  out[_size].clear();
  bi.set_length(_size + 1);
  bi[_size].clear();

  comp.set_length(_size + 1);
  comp[_size] = 0;

  _size += 1;
  return _size - 1;
}

void graph::add_graph(const graph& g, mapping& m)
{
  index_type _new_size = _size + g.size();
  adj.set_size(_new_size, _new_size);
  in.set_length(_new_size);
  out.set_length(_new_size);
  bi.set_length(_new_size);
  comp.set_length(_new_size);

  m.assign_identity(g.size());
  for (index_type k = 0; k < g.size(); k++) {
    m[k] = _size + k;
    for (index_type i = 0; i < _new_size; i++) {
      adj[i][m[k]] = false;
      adj[m[k]][i] = false;
    }
    in[m[k]].clear();
    out[m[k]].clear();
    bi[m[k]].clear();
    comp[m[k]] = 0;
  }
  _size = _new_size;

  for (index_type i = 0; i < g.size(); i++)
    for (index_type j = 0; j < g.size(); j++)
      if (g.adjacent(i, j))
	add_edge(m[i], m[j]);
}

void graph::add_edge(index_type src, index_type dst)
{
  assert((src < _size) && (dst < _size));
  adj[src][dst] = true;
  out[src].insert(dst);
  in[dst].insert(src);
  if (adj[dst][src]) {
    bi[src].insert(dst);
    bi[dst].insert(src);
  }
}

void graph::add_edge(const index_set& srcs, index_type dst)
{
  for (index_type k = 0; k < srcs.length(); k++)
    add_edge(srcs[k], dst);
}

void graph::add_edge(index_type src, const index_set& dsts)
{
  for (index_type k = 0; k < dsts.length(); k++)
    add_edge(src, dsts[k]);
}

void graph::add_edge_to_transitive_closure
(index_type src, index_type dst, pair_set& e)
{
  if (!adj[src][dst]) {
    add_edge(src, dst);
    e.insert(index_pair(src, dst));
  }
  for (index_type i = 0; i < in[src].length(); i++) {
    if (!adj[in[src][i]][dst]) {
      e.insert(index_pair(in[src][i], dst));
    }
    for (index_type j = 0; j < out[dst].length(); j++) {
      if (!adj[in[src][i]][out[dst][j]]) {
	e.insert(index_pair(in[src][i], out[dst][j]));
      }
    }
  }
  for (index_type j = 0; j < out[dst].length(); j++) {
    if (!adj[src][out[dst][j]]) {
      e.insert(index_pair(src, out[dst][j]));
    }
  }
  for (index_type k = 0; k < e.length(); k++)
    add_edge(e[k].first, e[k].second);
}

void graph::remove_edge(index_type src, index_type dst)
{
  assert((src < _size) && (dst < _size));
  adj[src][dst] = false;
  out[src].subtract(dst);
  in[dst].subtract(src);
  if (adj[dst][src]) {
    bi[src].subtract(dst);
    bi[dst].subtract(src);
  }
}

void graph::remove_edges_from(index_type src)
{
  index_set ns(successors(src));
  for (index_type k = 0; k < ns.length(); k++)
    remove_edge(src, ns[k]);
}

void graph::remove_edges_to(index_type dst)
{
  index_set ns(predecessors(dst));
  for (index_type k = 0; k < ns.length(); k++)
    remove_edge(ns[k], dst);
}

void graph::remove_edges_incident_on(index_type n)
{
  remove_edges_from(n);
  remove_edges_to(n);
}

void graph::remove_edges(const pair_set& e)
{
  for (index_type k = 0; k < e.length(); k++)
    remove_edge(e[k].first, e[k].second);
}

void graph::add_undirected_edge(index_type n0, index_type n1)
{
  add_edge(n0, n1);
  add_edge(n1, n0);
}

void graph::remove_undirected_edge(index_type n0, index_type n1)
{
  remove_edge(n0, n1);
  remove_edge(n1, n0);
}

void graph::remove_node(index_type n)
{
  assert(n < _size);
  graph g(*this);
  init(g.size() - 1);
  for (index_type i = 0; i < g.size(); i++)
    for (index_type j = 0; j < g.size(); j++)
      if (g.adjacent(i, j) && (i != n) && (j != n)) {
	index_type s = (i > n ? i - 1 : i);
	index_type d = (j > n ? j - 1 : j);
	add_edge(s, d);
      }
}

void graph::clear_edges()
{
  adj.assign_value(false);
  in.assign_value(EMPTYSET);
  out.assign_value(EMPTYSET);
  bi.assign_value(EMPTYSET);
}

void graph::recalculate()
{
  for (index_type i = 0; i < _size; i++) {
    in[i].clear();
    out[i].clear();
  }
  for (index_type i = 0; i < _size; i++)
    for (index_type j = 0; j < _size; j++)
      if (adj[i][j]) {
	out[i].insert(j);
	in[j].insert(i);
      }
  for (index_type k = 0; k < _size; k++) {
    bi[k].assign_copy(out[k]);
    bi[k].intersect(in[k]);
  }
  comp.set_length(_size);
  comp.assign_value(0);
  n_comp = 0;
}

void graph::complement()
{
  for (index_type i = 0; i < _size; i++) {
    for (index_type j = 0; j < _size; j++)
      adj[i][j] = !adj[i][j];
    adj[i][i] = false;
  }
  recalculate();
}

void graph::complement_with_loops()
{
  for (index_type i = 0; i < _size; i++) {
    for (index_type j = 0; j < _size; j++)
      adj[i][j] = !adj[i][j];
  }
  recalculate();
}

void graph::remove_loops()
{
  for (index_type i = 0; i < _size; i++) if (adj[i][i]) {
    adj[i][i] = false;
    out[i].subtract(i);
    in[i].subtract(i);
    bi[i].subtract(i);
  }
}

void graph::reverse()
{
  for (index_type i = 0; i < _size; i++)
    for (index_type j = i+1; j < _size; j++) {
      if (adj[i][j] && !adj[j][i]) {
	adj[i][j] = false;
	adj[j][i] = true;
      }
      else if (!adj[i][j] && adj[j][i]) {
	adj[i][j] = true;
	adj[j][i] = false;
      }
    }
  recalculate();
}

void graph::transitive_closure()
{
  for (index_type k = 0; k < _size; k++)
    for (index_type i = 0; i < _size; i++)
      if (adj[i][k])
	for (index_type j = 0; j < _size; j++)
	  if (adj[k][j]) adj[i][j] = true;
  recalculate();
}

void graph::missing_transitive_edges(pair_set& e) const
{
  e.clear();
  for (index_type i = 0; i < _size; i++) {
    bool_vec v(false, _size);
    reachable(i, v);
    for (index_type j = 0; j < _size; j++)
      if ((i != j) && v[j] && (!adj[i][j]))
	e.insert(index_pair(i, j));
  }
}

void graph::transitive_reduction()
{
  bool_matrix m2(adj);
  m2.transitive_closure();
  bool_matrix m3;
  m3.multiply(adj, m2);
  adj.subtract(m3);
  recalculate();
}

void graph::intersect(const graph& g)
{
  if (g._size != _size) {
    ::std::cerr << "error: can't intersect " << *this << " of size " << _size
	      << " with graph " << g << " of size" << g._size << ::std::endl;
    exit(255);
  }
  for (index_type i = 0; i < _size; i++)
    for (index_type j = 0; j < _size; j++)
      if (!g.adj[i][j]) adj[i][j] = false;
  recalculate();
}

graph& graph::subgraph(graph& sg, const index_set& nodes) const
{
  sg.init(nodes.length());
  for (index_type i = 0; i < nodes.length(); i++)
    for (index_type j = 0; j < nodes.length(); j++)
      if (adj[nodes[i]][nodes[j]]) sg.add_edge(i, j);
  return sg;
}

graph& graph::edge_subgraph(graph& sg, const index_set& nodes) const
{
  sg.init(size());
  for (index_type i = 0; i < nodes.length(); i++)
    for (index_type j = 0; j < nodes.length(); j++)
      if (adj[nodes[i]][nodes[j]]) sg.add_edge(nodes[i], nodes[j]);
  return sg;
}

graph& graph::component_tree(graph& cg) const
{
  cg.init(n_comp);
  for (index_type i = 0; i < _size; i++)
    for (index_type j = 0; j < _size; j++)
      if (adj[i][j] && (comp[i] != comp[j])) cg.add_edge(comp[i], comp[j]);
  return cg;
}

equivalence& graph::component_partitioning(equivalence& eq) const
{
  eq.extend(_size);
  for (index_type i = 0; i < _size; i++)
    for (index_type j = i + 1; j < _size; j++)
      if ((comp[i] == comp[j])) eq.merge(i, j);
  return eq;
}

equivalence& graph::induced_partitioning(equivalence& eq) const
{
  eq.clear();
  eq.extend(_size);
  for (index_type i = 0; i < _size; i++)
    for (index_type j = 0; j < _size; j++)
      if (adj[i][j]) eq.merge(i, j);
  return eq;
}

graph& graph::induced_undirected_graph(graph& g) const
{
  g.copy(*this);
  for (index_type i = 0; i < g._size; i++)
    for (index_type j = 0; j < g._size; j++)
      if (g.adj[i][j] && !g.adj[j][i]) g.add_edge(j, i);
  return g;
}

graph& graph::minimal_equivalent_digraph(graph& g) const
{
  graph cg;
  component_tree(cg);
  cg.transitive_reduction();
  g.init(size());
  for (index_type k = 0; k < n_components(); k++) {
    index_set c;
    component_node_set(k, c);
    if (c.length() > 1) {
      for (index_type i = 0; i < (c.length() - 1); i++)
	g.add_edge(c[i], c[i+1]);
      g.add_edge(c[c.length() - 1], c[0]);
    }
  }
  for (index_type i = 0; i < cg.size(); i++)
    for (index_type j = 0; j < cg.size(); j++)
      if (cg.adjacent(i, j)) {
	index_type ci = component_node(i);
	index_type cj = component_node(j);
	assert(ci != no_such_index);
	assert(cj != no_such_index);
	g.add_edge(ci, cj);
      }
  return g;
}

graph& graph::minimal_distance_graph(graph& g, const index_set& s0) const
{
  index_vec d;
  distance(s0, d);
  g.init(size());
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j) && (d[i] != no_such_index) && (d[j] == (d[i] + 1)))
	g.add_edge(i, j);
  return g;
}

graph& graph::quotient(graph& g, const equivalence& eq) const
{
  g.init(eq.n_classes());
  index_vec m;
  eq.make_map(m);
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j) && !eq(i, j)) {
	assert(m[i] < eq.n_classes());
	assert(m[j] < eq.n_classes());
	g.add_edge(m[i], m[j]);
      }
}

bool graph::is_clique(const index_set& nodes) const
{
  for (index_type i = 0; i < nodes.length(); i++)
    for (index_type j = i + 1; j < nodes.length(); j++)
      if (!adj[nodes[i]][nodes[j]] || !adj[nodes[j]][nodes[i]])
	return false;
  return true;
}

bool graph::is_independent(const index_set& nodes) const
{
  for (index_type i = 0; i < nodes.length(); i++)
    for (index_type j = i + 1; j < nodes.length(); j++)
      if (adj[nodes[i]][nodes[j]] || adj[nodes[j]][nodes[i]])
	return false;
  return true;
}

bool graph::is_independent_range(index_type l, index_type u) const
{
  for (index_type i = l; i <= u; i++)
    for (index_type j = i + 1; j <= u; j++)
      if (adjacent(i, j) || adjacent(j, i)) return false;
  return true;
}

void graph::write_node_set(::std::ostream& s) const
{
  s << '{';
  for (index_type i = 0; i < _size; i++) {
    if (i > 0) s << ',';
    s << i << "[" << comp[i] << "]";
  }
  s << '}';
}

void graph::write_edge_set(::std::ostream& s) const
{
  s << '{';
  bool first = true;
  for (index_type i = 0; i < _size; i++)
    for (index_type j = 0; j < _size; j++)
      if (adj[i][j]) {
	if (!first) s << ',';
	first = false;
	s << i << "->" << j;
      }
  s << '}';
}

void graph::write_compact(::std::ostream& s) const
{
  s << '(';
  write_node_set(s);
  s << ',';
  write_edge_set(s);
  s << '}';
}

void graph::write_undirected_edge_set(::std::ostream& s) const
{
  s << '{';
  bool first = true;
  for (index_type i = 0; i < _size; i++)
    for (index_type j = i+1; j < _size; j++)
      if (adj[i][j]) {
	if (!first) s << ',';
	first = false;
	s << i << "-" << j;
      }
  s << '}';
}

void graph::write_adjacency_lists(::std::ostream& s) const
{
  for (index_type k = 0; k < _size; k++) {
    s << "node " << k << " (component " << comp[k] << "):" << ::std::endl;
    s << " in (" << in[k].length() << "):";
    for (index_type i = 0; i < _size; i++)
      if (adj[i][k]) s << " " << i;
    s << ::std::endl << " out (" << out[k].length() << "):"; 
    for (index_type i = 0; i < _size; i++)
      if (adj[k][i]) s << " " << i;
    s << ::std::endl;
  }
}

void graph::randomize(count_type n, RNG& rnd)
{
  assert(_size > 1);
  for (index_type s = 0; s < n; s++) {
    index_type i = rnd.random_in_range(_size);
    index_type j = rnd.random_in_range(_size);
    if (i != j) {
      if (adj[i][j]) {
	adj[i][j] = false;
      }
      else {
	adj[i][j] = true;
      }
    }
  }
  recalculate();
}

void graph::randomize_connected(count_type n, RNG& rnd)
{
  assert(_size > 1);
  for (index_type s = 0; s < n; s++) {
    index_type i = rnd.random_in_range(_size);
    index_type j = rnd.random_in_range(_size, i);
    if (adj[i][j]) {
      adj[i][j] = false;
      if (!connected()) adj[i][j] = true;
    }
    else {
      adj[i][j] = true;
    }
  }
  recalculate();
}

void graph::randomize_strongly_connected(count_type n, RNG& rnd)
{
  assert(_size > 1);
  strongly_connected_components();
  index_type c = n_components();
  for (index_type s = 0; s < n; s++) {
    index_type i = rnd.random_in_range(_size);
    index_type j = rnd.random_in_range(_size, i);
    if (adj[i][j]) {
      adj[i][j] = false;
      strongly_connected_components();
      if (n_components() > c)
	adj[i][j] = true;
    }
    else {
      adj[i][j] = true;
    }
  }
  recalculate();
}

void graph::random_digraph(count_type n, RNG& rnd)
{
  clear_edges();
  randomize(n, rnd);
}

void graph::random_connected_digraph(count_type n, RNG& rnd)
{
  assert(_size > 1);
  clear_edges();
  for (index_type i = 0; i < _size - 1; i++) {
    adj[i][i+1] = true;
  }
  randomize_connected(n, rnd);
}

void graph::random_strongly_connected_digraph(count_type n, RNG& rnd)
{
  assert(_size > 1);
  clear_edges();
  for (index_type i = 0; i < _size - 1; i++) {
    adj[i][i+1] = true;
  }
  adj[_size - 1][0] = true;
  randomize_strongly_connected(n, rnd);
}

void graph::random_digraph_with_density(rational density, RNG& rnd)
{
  clear_edges();
  // assert(_size > 1);
  index_set e;
  index_type n = (_size * (_size - 1));
  index_type m = rational::floor(density*n).numerator();
  rnd.select_fixed_set(e, m, n);
  for (index_type k = 0; k < e.length(); k++) {
    index_type s = e[k] / (_size - 1);
    index_type t = e[k] % (_size - 1);
    if (t >= s) t += 1;
    add_edge(s, t);
  }
}

void graph::random_tree(RNG& rnd)
{
  clear_edges();
  for (index_type k = 1; k < size(); k++) {
    index_type i = rnd.random_in_range(k);
    add_edge(i, k);
  }
}

void graph::random_tree(index_type b, index_type d, RNG& rnd)
{
  clear_edges();
  for (index_type k = 1; k < size(); k++) {
    // std::cerr << "k = " << k << ", g = " << *this << std::endl;
    index_set c;
    for (index_type i = 0; i < k; i++) {
      // std::cerr << i << ": "
      // << out_degree(i) << ", " << distance(0, i)
      // << std::endl;
      if ((out_degree(i) < b) && (distance(0, i) < d))
	c.insert(i);
    }
    // std::cerr << "c = " << c << std::endl;
    assert(!c.empty());
    index_type j = rnd.select_one_of(c);
    add_edge(j, k);
  }
}

void graph::max_clique
(index_set& sel, index_type next, index_set& clique) const
{
  if (next >= _size) {
    if (sel.length() > clique.length()) clique = sel;
  }
  else if (sel.contains(next)) {
    max_clique(sel, next + 1, clique);
  }
  else {
    if (bi[next].contains(sel)) {
      sel.insert(next);
      max_clique(sel, next + 1, clique);
      sel.subtract(next);
    }
    max_clique(sel, next + 1, clique);
  }
}

void graph::maximal_clique(index_set& clique) const
{
  assert(_size > 0);
  clique.clear();
  index_set sel;
  max_clique(sel, 0, clique);
}

void graph::maximal_clique_including
(index_type node, index_set& clique) const
{
  assert((0 <= node) && (node < _size) && (_size > 0));
  clique.clear();
  index_set sel;
  sel.assign_singleton(node);
  max_clique(sel, 0, clique);
}

void graph::maximal_clique_cover(index_set_vec& sets) const
{
  sets.clear();

  index_set clique;
  // max_clique(clique);
  // assert(!clique.empty());
  // sets.append(clique);

  index_set uncovered;
  uncovered.fill(_size);
  // uncovered.subtract(clique);
  while (!uncovered.empty()) {
    maximal_clique_including(uncovered[0], clique);
    assert(!clique.empty());
    sets.append(clique);
    uncovered.subtract(clique);
  }
}

void graph::all_max_cliques
(index_set& sel, index_type next, index_set_vec& cliques) const
{
  if (next >= _size) {
    if (sel.length() > cliques[0].length()) {
      cliques[0] = sel;
      cliques.set_length(1);
    }
    else if (sel.length() == cliques[0].length()) {
      cliques.append(sel);
    }
  }
  else if (sel.contains(next)) {
    all_max_cliques(sel, next + 1, cliques);
  }
  else {
    if (bi[next].contains(sel)) {
      sel.insert(next);
      all_max_cliques(sel, next + 1, cliques);
      sel.subtract(next);
    }
    all_max_cliques(sel, next + 1, cliques);
  }
}

void graph::all_maximal_cliques(index_set_vec& cliques) const
{
  assert(_size > 0);
  cliques.assign_value(EMPTYSET, 1);
  index_set sel;
  all_max_cliques(sel, 0, cliques);
}

void graph::all_maximal_cliques_including
(index_type node, index_set_vec& cliques) const
{
  assert(_size > 0);
  cliques.assign_value(EMPTYSET, 1);
  index_set sel;
  sel.assign_singleton(node);
  all_max_cliques(sel, 0, cliques);
}

void graph::scc_first_dfs
(index_type n, bool_vec& visited, index_vec& num) const
{
  visited[n] = true;
  for (index_type k = 0; k < _size; k++)
    if (adj[n][k] && !visited[k])
      scc_first_dfs(k, visited, num);
  num.append(n);
}

void graph::scc_second_dfs
(index_type n, bool_vec& visited, index_type c_id)
{
  visited[n] = true;
  comp[n] = c_id;
  for (index_type k = 0; k < _size; k++)
    if (adj[k][n] && !visited[k])
      scc_second_dfs(k, visited, c_id);
}

void graph::undirected_dfs
(index_type n, bool_vec& visited) const
{
  visited[n] = true;
  for (index_type k = 0; k < _size; k++)
    if ((adj[n][k] || adj[k][n]) && !visited[k])
      undirected_dfs(k, visited);
}

void graph::reachable(index_type n, bool_vec& visited) const
{
  visited[n] = true;
  for (index_type k = 0; k < _size; k++)
    if (adj[n][k] && !visited[k])
      reachable(k, visited);
}

void graph::strongly_connected_components()
{
  index_vec num(no_such_index, 0);
  bool_vec visited(false, _size);
  visited.set_length(_size);

  for (index_type k = 0; k < _size; k++) {
    if (!visited[k]) {
      scc_first_dfs(k, visited, num);
    }
  }
  assert(num.length() == _size);

  visited.assign_value(false);
  n_comp = 0;
  for (index_type k = num.length(); k > 0; k--) if (!visited[num[k - 1]]) {
    scc_second_dfs(num[k - 1], visited, n_comp);
    n_comp += 1;
  }
}

void graph::ramsey(const index_set& nodes, index_set& I, index_set& C) const
{
  I.clear();
  C.clear();

  if (nodes.empty()) return;

  index_type v = nodes[0];
  index_set n_v(nodes);
  n_v.intersect(bi[v]);
  n_v.subtract(v);
  ramsey(n_v, I, C);
  C.insert(v);

  index_set I2;
  index_set C2;
  n_v.assign_copy(nodes);
  n_v.subtract(out[v]);
  n_v.subtract(in[v]);
  n_v.subtract(v);
  ramsey(n_v, I2, C2);
  I2.insert(v);

  if (C2.length() > C.length()) C.assign_copy(C2);
  if (I2.length() > I.length()) I.assign_copy(I2);
}

void graph::apx_independent_set(const index_set& nodes, index_set& set) const
{
  set.clear();

  index_set n(nodes);
  index_set nextI;
  index_set nextC;

  while (!n.empty()) {
    ramsey(n, nextI, nextC);
    if (nextI.length() > set.length()) set.assign_copy(nextI);
    n.subtract(nextC);
  }
}

void graph::apx_independent_set(index_set& set) const
{
  index_set nodes;
  nodes.fill(_size);
  apx_independent_set(nodes, set);
}

void graph::apx_independent_set_including
(index_type node, index_set& set) const
{
  assert((0 <= node) && (node < _size) && (_size > 0));
  set.clear();

  index_set nodes;
  nodes.fill(_size);
  nodes.subtract(in[node]);
  nodes.subtract(out[node]);
  nodes.subtract(node);
  index_set nextI;
  index_set nextC;

  while (!nodes.empty()) {
    ramsey(nodes, nextI, nextC);
    if (nextI.length() > set.length()) set.assign_copy(nextI);
    nodes.subtract(nextC);
  }

  set.insert(node);
}

void graph::apx_independent_set_cover(index_set_vec& sets) const
{
  sets.clear();

  index_set I;
  apx_independent_set(I);
  assert(!I.empty());
  sets.append(I);

  index_set uncovered;
  uncovered.fill(_size);
  uncovered.subtract(I);
  while (!uncovered.empty()) {
    apx_independent_set_including(uncovered[0], I);
    assert(!I.empty());
    sets.append(I);
    uncovered.subtract(I);
  }
}

void graph::all_nondominated_cliques(index_set_vec &cliques) const
{
  /*
    Find all maximal (wrt set inclusion) cliques of the graph.
    The cliques are reported in no particular order.

    Implements the algorithm by Tomita, Tanaka and Takahashi
    ("The Worst-Case Time Complexity for Generating All Maximal
    Cliques"), which is optimal in the sense that it runs in
    O(3^(n/3)), which is about O(1.44^n), for an n-vertex graph, and
    there are graphs which have that many maximal cliques.

    Give or take a (low-order?) polynomial factor for inefficiencies
    in basic operations such as computing the set of neighbours
    from a certain set.
  */

  assert(cliques.empty());
  if (_size > 0) {
    for (index_type i = 0; i < _size; i++)
      assert(!adjacent(i, i)); // Self-loops confuse the algorithm.
    index_set current_clique;
    index_set candidates;
    candidates.fill(_size);
    all_nondominated_cliques_aux(cliques, current_clique, candidates, 1);
  }
}

void graph::all_cliques_geq
(index_type k, index_set_vec& cliques) const
{
  cliques.clear();
  if (_size > 0) {
    for (index_type i = 0; i < _size; i++)
      assert(!adjacent(i, i)); // Self-loops confuse the algorithm.
    index_set current_clique;
    index_set candidates;
    candidates.fill(_size);
    all_nondominated_cliques_aux(cliques, current_clique, candidates, k);
  }
}

void graph::all_nondominated_cliques_aux
(index_set_vec& cliques,
 index_set& current_clique,
 const index_set& candidates,
 index_type min) const
{
  if (candidates.empty()) {
    cliques.append(current_clique);
  }
  else {
    // Find vertex with maximal number of successors in candidates.
    index_type best_vertex = no_such_index;
    int max_degree = 0;
    for (index_type i = 0; i < candidates.length(); i++) {
      index_type vertex = candidates[i];
      index_set neighbors = successors(vertex);
      neighbors.intersect(candidates);
      if (i == 0 || neighbors.length() > max_degree) {
        max_degree = neighbors.length();
        best_vertex = vertex;
      }
    }
    assert(best_vertex != no_such_index);
    // Iterate over candidates that are *not* adjacent to best_vertex.
    index_set chosen_set = candidates;
    chosen_set.subtract(successors(best_vertex));
    for (index_type i = 0; i < chosen_set.length(); i++) {
      index_type chosen = chosen_set[i];
      // Call recursively with chosen added to the current clique
      // and candidates reduced to chosen's neighbours.
      index_set new_candidates = candidates;
      new_candidates.intersect(successors(chosen));
      if ((current_clique.length() + new_candidates.length() + 1) >= min) {
	current_clique.insert(chosen);
	all_nondominated_cliques_aux(cliques,
				     current_clique,
				     new_candidates,
				     min);
	current_clique.subtract(chosen);
      }
    }
  }
}

bool graph::connected() const
{
  if (_size == 0) return true;
  bool_vec visited(false, _size);
  undirected_dfs(0, visited);
  return (visited.count(true) == _size);
}

bool graph::strongly_connected() const
{
  if (_size == 0) return true;
  bool_vec visited(false, _size);
  index_vec num(no_such_index, 0);
  scc_first_dfs(0, visited, num);
  return (visited.count(true) == _size);
}

void graph::write_digraph
(::std::ostream& s, bool with_node_indices, const char* name) const
{
  s << "digraph \"" << name << "\"" << "{" << ::std::endl;
  if (with_node_indices) {
    s << "node [shape=circle,width=0.5,height=0.5];" << ::std::endl;
    for (index_type k = 0; k < size(); k++)
      s << "\t" << k << " [label=\"" << k << "\"];" << std::endl;
  }
  else {
    s << "node [shape=point];" << ::std::endl;
  }
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j))
	s << "\t" << i << " -> " << j << ";" << ::std::endl;
  s << "}" << ::std::endl;
}

void graph::write_component_labeled_digraph
(::std::ostream& s, const char* name) const
{
  write_labeled_digraph<index_vec>(s, *this, comp, false, name, no_such_index);
}

void graph::write_graph_correspondance
(::std::ostream& s,
 const graph& g,
 const index_vec& c,
 const char* name) const
{
  assert(g.size() == size());
  assert(c.length() == g.size());

  s << "digraph \"" << name << "\"" << ::std::endl << "{" << ::std::endl;
  s << "node [width=0,height=0];" << ::std::endl;

  s << "subgraph cluster0 {" << ::std::endl;
  for (index_type k = 0; k < size(); k++) {
    s << "\tG0_" << k << " [label=\"" << k << "\"]"
      << ::std::endl;
  }
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j)) {
	if (!g.adjacent(c[i], c[j])) {
	  s << "\tG0_" << i << " -> G0_" << j << " [style=bold];" << ::std::endl;
	}
	else {
	  s << "\tG0_" << i << " -> G0_" << j << ";" << ::std::endl;
	}
      }
  s << "}" << ::std::endl;

  s << "subgraph cluster1 {" << ::std::endl;
  for (index_type k = 0; k < g.size(); k++) {
    s << "\tG1_" << k << " [label=\"" << k << "\"]"
      << ::std::endl;
  }
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (g.adjacent(c[i], c[j])) {
	if (!adjacent(i, j)) {
	  s << "\tG1_" << i << " -> G1_" << j << " [style=bold];" << ::std::endl;
	}
	else {
	  s << "\tG1_" << i << " -> G1_" << j << ";" << ::std::endl;
	}
      }
  s << "}" << ::std::endl;

  for (index_type k = 0; k < size(); k++) {
    s << "G0_" << k << " -> G1_" << c[k] << " [style=dashed,dir=none];"
      << ::std::endl;
  }

  s << "}" << ::std::endl;
}


void index_graph::reverse()
{
  for (index_type i = 0; i < size(); i++)
    for (index_type j = i+1; j < size(); j++) {
      if (adjacent(i, j) && !adjacent(j, i)) {
	index_type l = (edge_has_label(i, j) ? edge_label(i, j) : 0);
	remove_edge(i, j);
	if ((l & EDGE_DIR) == ED_FORWARD)
	  l += (ED_BACK - ED_FORWARD);
	else if ((l & EDGE_DIR) == ED_BACK)
	  l -= (ED_BACK - ED_FORWARD);
	add_edge(j, i, l);
      }
      else if (!adjacent(i, j) && adjacent(j, i)) {
	index_type l = (edge_has_label(j, i) ? edge_label(j, i) : 0);
	remove_edge(j, i);
	if ((l & EDGE_DIR) == ED_FORWARD)
	  l += (ED_BACK - ED_FORWARD);
	else if ((l & EDGE_DIR) == ED_BACK)
	  l -= (ED_BACK - ED_FORWARD);
	add_edge(i, j, l);
      }
    }
}

void index_graph::reflect()
{
  for (index_type i = 0; i < size(); i++)
    for (index_type j = i + 1; j < size(); j++)
      if (adjacent(i, j)) {
	assert(!adjacent(j, i));
	add_edge(j, i);
	index_type l = (edge_has_label(i, j) ? edge_label(i, j) : 0);
	edge_label(j, i) = l;
	if ((l & EDGE_DIR) == ED_FORWARD)
	  edge_label(j, i) += (ED_BACK - ED_FORWARD);
	else if ((l & EDGE_DIR) == ED_BACK)
	  edge_label(j, i) -= (ED_BACK - ED_FORWARD);
      }
      else if (adjacent(j, i)) {
	add_edge(i, j);
	index_type l = (edge_has_label(j, i) ? edge_label(j, i) : 0);
	edge_label(i, j) = l;
	if ((l & EDGE_DIR) == ED_FORWARD)
	  edge_label(i, j) += (ED_BACK - ED_FORWARD);
	else if ((l & EDGE_DIR) == ED_BACK)
	  edge_label(i, j) -= (ED_BACK - ED_FORWARD);
      }
}

void index_graph::write_node_style
(std::ostream& s, index_type l)
{
  s << "shape=";
  if ((l & NODE_SHAPE) == NS_ELLIPSE)
    s << "ellipse";
  else if ((l & NODE_SHAPE) == NS_BOX)
    s << "box";
  else if ((l & NODE_SHAPE) == NS_POINT)
    s << "point";
  else if ((l & NODE_SHAPE) == NS_DIAMOND)
    s << "diamond";
  else if ((l & NODE_SHAPE) == NS_HEXAGON)
    s << "hexagon";
  else if ((l & NODE_SHAPE) == NS_OCTAGON)
    s << "octagon";
  else if ((l & NODE_SHAPE) == NS_PLAINTEXT)
    s << "plaintext";
  else
    s << "circle";
  if ((l & NODE_STYLE) == NS_FILLED)
    s << ",style=filled";
  else if ((l & NODE_STYLE) == NS_BOLD)
    s << ",style=bold";
  else if ((l & NODE_STYLE) == NS_DASHED)
    s << ",style=dashed";
  else if ((l & NODE_STYLE) == NS_DOTTED)
    s << ",style=dotted";
  if ((l & NS_DOUBLE) == NS_DOUBLE)
    s << ",peripheries=2";
}

void index_graph::write_edge_style
(std::ostream& s, index_type l)
{
  if ((l & EDGE_DIR) == ED_FORWARD)
    s << "dir=forward";
  else if ((l & EDGE_DIR) == ED_BACK)
    s << "dir=back";
  else if ((l & EDGE_DIR) == ED_BOTH)
    s << "dir=both";
  else
    s << "dir=none";
  if ((l & EDGE_STYLE) == ES_BOLD)
    s << ",style=bold";
  else if ((l & EDGE_STYLE) == ES_DASHED)
    s << ",style=dashed";
  else if ((l & EDGE_STYLE) == ES_DOTTED)
    s << ",style=dotted";
}

void index_graph::write_styled_digraph
(std::ostream& s,
 bool with_node_indices,
 const char* name,
 index_type c_id) const
{
  if (c_id != no_such_index) {
    s << "subgraph cluster" << c_id << " {" << std::endl;
    s << "node [width=0.5,height=0.5];" << ::std::endl;
  }
  else if (name) {
    s << "digraph \"" << name << "\" {" << std::endl;
    s << "node [width=0.5,height=0.5];" << ::std::endl;
  }
  for (index_type k = 0; k < size(); k++) {
    s << "\t" << k + (c_id != no_such_index ? c_id : 0) << " [";
    write_node_style(s, node_has_label(k) ? node_label(k) : 0);
    if (with_node_indices)
      s << ",label=\"" << k << "\"];" << std::endl;
    else
      s << ",label=\"\"];" << std::endl;
  }
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j)) {
	s << "\t" << i + (c_id != no_such_index ? c_id : 0)
	  << " -> " << j + (c_id != no_such_index ? c_id : 0) << " [";
	write_edge_style(s, edge_has_label(i, j) ? edge_label(i, j) : 0);
	s << "];" << std::endl;
      }
  if ((c_id != no_such_index) || (name != 0)) {
    s << "}" << ::std::endl;
  }
}

void index_graph::write_matrix
(std::ostream& s) const
{
  for (index_type i = 0; i < size(); i++) {
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j))
	s << ' ' << (edge_has_label(i, j) ? edge_label(i, j) : 0) + 1;
      else
	s << ' ' << 0;
    s << ' ' << (node_has_label(i) ? node_label(i) : 0) << std::endl;
  }
}

void index_graph::write_MATLAB
(std::ostream& s, const char* n, const char* t) const
{
  s << "defgraph('";
  if (n)
    s << n;
  else
    s << "NONAME";
  s << "', '";
  if (t)
    s << t;
  else
    s << "NOTYPE";
  s << "',[";
  for (index_type i = 0; i < size(); i++) {
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j))
	s << ' ' << (edge_has_label(i, j) ? edge_label(i, j) : 0) + 1;
      else
	s << ' ' << 0;
    if (i + 1 < size()) s << ";";
  }
  s << "], [";
  for (index_type i = 0; i < size(); i++) {
    s << ' ' << (node_has_label(i) ? node_label(i) : 0);
    if (i + 1 < size()) s << ";";
  }
  s << "]);" << std::endl;
}


// weighted_graph methods

weighted_graph& weighted_graph::quotient
(weighted_graph& g, const equivalence& eq) const
{
  g.init(eq.n_classes());
  index_vec m;
  eq.make_map(m);
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j) && !eq(i, j))
	g.increment_edge_weight(m[i], m[j], weight(i, j));
}

NTYPE weighted_graph::apx_weighted_independent_set_1(index_set& set) const
{
  set.clear();
  NTYPE w_best = NEG_INF;

  NTYPE w_max = max_node_weight();
  index_type m = ilog(size());
  for (index_type k = 1; k <= m; k++) {
    NTYPE w_lb = (w_max / (1 << k));
    assert(w_lb > 0);
    NTYPE w_ub = (w_max / (1 << (k - 1)));

    index_set nodes;
    for (index_type i = 0; i < size(); i++)
      if ((w_lb < weight(i)) && (weight(i) <= w_ub))
	nodes.insert(i);
    index_set maxI;
    apx_independent_set(nodes, maxI);
    NTYPE w = 0;
    for (index_type i = 0; i < maxI.length(); i++)
      w += weight(maxI[i]);
    if (w > w_best) {
      set.assign_copy(maxI);
      w_best = w;
    }
  }
  return w_best;
}

NTYPE weighted_graph::apx_weighted_independent_set_2(index_set& set) const
{
  // sort nodes in order of increasing weighted degree
  weighted_vec<index_type,NTYPE> sorted;
  for (index_type k = 0; k < size(); k++) {
    NTYPE wd = (weight(bidirectional(k)) / weight(k));
    sorted.insert_increasing(k, wd);
  }

  set.clear();
  bool_vec rem(true, size());
  NTYPE w = 0;
  for (index_type k = 0; k < size(); k++) {
    index_type i = sorted[k].value;
    if (rem[i] && (weight(i) > 0)) {
      set.insert(i);
      rem.subtract(bidirectional(i));
      w += weight(i);
    }
  }
  return w;
}

NTYPE weighted_graph::apx_weighted_independent_set(index_set& set) const
{
  NTYPE v1 = apx_weighted_independent_set_1(set);
  NTYPE v2 = apx_weighted_independent_set_2(set);
  return MAX(v1, v2);
}

void weighted_graph::add_edge(index_type src, index_type dst)
{
  graph::add_edge(src, dst);
}

void weighted_graph::add_edge(index_type src, index_type dst, NTYPE w)
{
  graph::add_edge(src, dst);
  set_weight(src, dst, w);
}

void weighted_graph::increment_edge_weight
(index_type src, index_type dst, NTYPE w)
{
  if (!adjacent(src, dst)) graph::add_edge(src, dst);
  set_weight(src, dst, weight(src, dst) + w);
}

void weighted_graph::add_undirected_edge(index_type n0, index_type n1)
{
  graph::add_undirected_edge(n0, n1);
}

void weighted_graph::add_undirected_edge
(index_type n0, index_type n1, NTYPE w)
{
  graph::add_undirected_edge(n0, n1);
  set_weight(n0, n1, w);
  set_weight(n1, n0, w);
}

NTYPE weighted_graph::weight(index_type n) const
{
  if (node_has_label(n)) {
    return node_label(n);
  }
  else {
    return 0;
  }
}

NTYPE weighted_graph::weight(const index_set& ns) const
{
  NTYPE sw = 0;
  for (index_type k = 0; k < ns.length(); k++) {
    assert(ns[k] < size());
    sw += weight(ns[k]);
  }
  return sw;
}

NTYPE weighted_graph::weight(index_type n0, index_type n1) const
{
  if (edge_has_label(n0, n1)) {
    return edge_label(n0, n1);
  }
  else {
    return 0;
  }
}

NTYPE weighted_graph::max_node_weight() const
{
  NTYPE w_max = NEG_INF;
  for (index_type k = 0; k < size(); k++)
    if (weight(k) > w_max) w_max = weight(k);
  return w_max;
}

void weighted_graph::set_weight(index_type n, NTYPE w)
{
  node_label(n) = w;
}

void weighted_graph::set_weight(index_type n0, index_type n1, NTYPE w)
{
  edge_label(n0, n1) = w;
}

void weighted_graph::transitive_closure()
{
  for (index_type k = 0; k < size(); k++)
    for (index_type i = 0; i < size(); i++)
      for (index_type j = 0; j < size(); j++)
	if (adjacent(i, k) && adjacent(k, j)) {
	  if (!adjacent(i, j)) {
	    add_edge(i, j, weight(i, k) + weight(k, j));
	  }
	  else if ((weight(i, k) + weight(k, j)) < weight(i, j)) {
	    set_weight(i, j, weight(i, k) + weight(k, j));
	  }
	}
  recalculate();
}

NTYPE weighted_graph::critical_path(cost_vec& s)
{
  cost_vec e(POS_INF, size());
  bool done = false;
  while (!done) {
    done = true;
    for (index_type k = 0; k < size(); k++) {
      NTYPE p = 0;
      for (index_type i = 0; i < predecessors(k).length(); i++)
	p = MAX(p, e[predecessors(k)[i]] + weight(predecessors(k)[i], k));
      if ((p + weight(k)) < e[k]) {
	e[k] = p + weight(k);
	done = false;
      }
    }
  }
  s.set_length(size());
  for (index_type k = 0; k < size(); k++)
    s[k] = (e[k] - weight(k));
  return cost_vec_util::max(e);
}

index_pair weighted_graph::max_weight_edge() const
{
  NTYPE w_max = NEG_INF;
  index_type n0 = no_such_index;
  index_type n1 = no_such_index;
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i,j) && ((n0 == no_such_index) || (weight(i, j) > w_max))) {
	n0 = i;
	n1 = j;
	w_max = weight(i, j);
      }
  return index_pair(n0, n1);
}

void weighted_graph::min_and_max_edges
(const index_set& nodes,
 pair_set& e_min, NTYPE& w_min,
 pair_set& e_max, NTYPE& w_max) const
{
  e_min.clear();
  w_min = POS_INF;
  e_max.clear();
  w_max = NEG_INF;
  for (index_type i = 0; i < nodes.length(); i++)
    for (index_type j = 0; j < nodes.length(); j++)
      if ((i != j) && adjacent(nodes[i], nodes[j])) {
	if (weight(nodes[i], nodes[j]) < w_min) {
	  w_min = weight(nodes[i], nodes[j]);
	  e_min.clear();
	  e_min.insert(index_pair(nodes[i], nodes[j]));
	}
	else if (weight(nodes[i], nodes[j]) == w_min) {
	  e_min.insert(index_pair(nodes[i], nodes[j]));
	}
	if (weight(nodes[i], nodes[j]) > w_max) {
	  w_max = weight(nodes[i], nodes[j]);
	  e_max.clear();
	  e_max.insert(index_pair(nodes[i], nodes[j]));
	}
	else if (weight(nodes[i], nodes[j]) == w_max) {
	  e_max.insert(index_pair(nodes[i], nodes[j]));
	}
      }
}

void weighted_graph::write_node_set(::std::ostream& s) const
{
  s << '{';
  for (index_type i = 0; i < size(); i++) {
    if (i > 0) s << ',';
    s << i << "[" << weight(i) << "]";
  }
  s << '}';
}

void weighted_graph::write_edge_set(::std::ostream& s) const
{
  s << '{';
  bool first = true;
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j)) {
	if (!first) s << ',';
	first = false;
	if (edge_has_label(i, j)) {
	  s << i << "-[" << weight(i, j) << "]->" << j;
	}
	else {
	  s << i << "->" << j;
	}
      }
  s << '}';
}

void weighted_graph::write_compact(::std::ostream& s) const
{
  s << '(';
  write_node_set(s);
  s << ',';
  write_edge_set(s);
  s << '}';
}

void weighted_graph::write_matrix(::std::ostream& s) const
{
  s << '[';
  for (index_type i = 0; i < size(); i++) {
    if (i > 0) s << ' ';
    s << '[';
    for (index_type j = 0; j < size(); j++) {
      if (j > 0) s << ',';
      if (adjacent(i, j)) {
	if (edge_has_label(i, j)) {
	  s << PRINT_NTYPE(edge_label(i, j));
	}
	else {
	  s << "?";
	}
      }
      else {
	s << "INF";
      }
    }
    s << ']';
    if (i + 1 < size()) {
      s << ',' << '\n';
    }
    else {
      s << ']' << '\n';
    }
  }
}

NTYPE weighted_graph::maximal_matching(weighted_graph& matching)
{
  index_type ni = no_such_index;
  index_type nj = no_such_index;
  for (index_type i = 0; (i < size()) && (ni == no_such_index); i++)
    for (index_type j = i + 1; (j < size()) && (nj == no_such_index); j++)
      if (adjacent(i, j) && adjacent(j, i)) {
	ni = i;
	nj = j;
      }
  // if there exists an edge...
  if ((ni != no_such_index) && (nj != no_such_index)) {
    // try with edge ni->nj in (recurse on subgraph without nodes ni, nj)...
    index_set s;
    s.fill(size());
    s.subtract(ni);
    s.subtract(nj);
    weighted_graph* g1 = new weighted_graph(*this, s);
    weighted_graph* m1 = new weighted_graph();
    NTYPE v1 = weight(ni, nj) + g1->maximal_matching(*m1);
    delete g1;

    // try with edge ni->nj out...
    weighted_graph* g2 = new weighted_graph(*this);
    g2->remove_edge(ni, nj);
    NTYPE v2 = g2->maximal_matching(matching);
    delete g2;

    if (v1 > v2) {
      // construct the matching with ni->nj (if v2 >= v1, just return
      // the matching returned by second recursive call)
      matching.init(size());
      for (index_type i = 0; i < m1->size(); i++)
	for (index_type j = i + 1; j < m1->size(); j++)
	  if (m1->adjacent(i, j) && m1->adjacent(j, i))
	    matching.add_undirected_edge(s[i], s[j], weight(s[i], s[j]));
      matching.add_undirected_edge(ni, nj, weight(ni, nj));
    }
    delete m1;
    return MAX(v1, v2);
  }
  else {
    matching.init(size());
    return 0;
  }
}

NTYPE weighted_graph::apx_matching(bool_vec& matched)
{
  bool_vec rem(true, size());
  matched.assign_value(false, size());
  NTYPE val[2] = {0,0};
  index_type i = 0;
  index_type v = rem.first(true);
  while (v != no_such_index) {
    rem[v] = false;
    bool done = false;
    while (!done) {
      NTYPE w_max = NEG_INF;
      index_type v_next = no_such_index;
      for (index_type k = 0; k < bidirectional(v).length(); k++)
	if (rem[bidirectional(v)[k]] &&
	    (weight(v, bidirectional(v)[k]) > w_max)) {
	  w_max = weight(v, bidirectional(v)[k]);
	  v_next = bidirectional(v)[k];
	}
      if (v_next != no_such_index) {
	val[i] += w_max;
	i = ((i + 1) % 2);
	matched[v] = true;
	matched[v_next] = true;
	rem[v_next] = false;
	v = v_next;
      }
      else {
	done = true;
      }
    }
    v = rem.first(true);
  }
  return MAX(val[0], val[1]);
}


// index_set_graph methods

index_set_graph::index_set_graph
(const graph& g, const equivalence& eq)
  : labeled_graph<index_set,index_set>(eq.n_classes())
{
  index_vec m;
  eq.make_map(m);
  assert(m.length() == g.size());
  for (index_type i = 0; i < g.size(); i++)
    for (index_type j = 0; j < g.size(); j++)
      if (g.adjacent(i, j) && !eq(i, j))
	add_edge(m[i], m[j]);
  index_set ce;
  eq.canonical_elements(ce);
  assert(ce.length() == size());
  for (index_type i = 0; i < size(); i++)
    eq.class_elements(ce[i], node_label(i));
}

index_set_graph::index_set_graph
(const index_set_graph& g, const equivalence& eq)
{
  g.quotient(*this, eq);
}

index_set_graph& index_set_graph::quotient
(index_set_graph& g, const equivalence& eq) const
{
  graph::quotient(g, eq);
  g.clear_node_labels();
  g.clear_edge_labels();
  index_vec m;
  eq.make_map(m);
  for (index_type i = 0; i < g.size(); i++)
    g.node_label(i) = EMPTYSET;
  for (index_type i = 0; i < size(); i++)
    g.node_label(m[i]).insert(node_label(i));
}

index_set_graph& index_set_graph::union_reachable(index_set_graph& g) const
{
  g.copy(*this);
  for (index_type k = 0; k < size(); k++) {
    index_set n;
    g.reachable_set(k, n);
    for (index_type i = 0; i < n.length(); i++)
      g.node_label(k).insert(node_label(n[i]));
  }
}

void index_set_graph::merge_labels(const index_set& ns)
{
  if (ns.empty()) return;
  assert(ns[0] < size());
  for (index_type i = 1; i < ns.length(); i++) {
    assert(ns[i] < size());
    node_label(ns[0]).insert(node_label(ns[i]));
  }
  for (index_type i = 1; i < ns.length(); i++) {
    node_label(ns[i]).assign_copy(node_label(ns[0]));
  }
}

void index_set_graph::merge_labels_upwards()
{
  strongly_connected_components();
  equivalence eq;
  component_partitioning(eq);
  // cdag: DAG of strongly connected components of *this
  index_set_graph cdag(*((graph*)this), eq);
  // 1. union node labels among nodes in same component
  for (index_type k = 0; k < cdag.size(); k++) {
    assert(!cdag.node_label(k).empty());
    merge_labels(cdag.node_label(k));
  }
  // 2. propagate upwards...
  while (!cdag.empty()) {
    // cdag.write_digraph(std::cerr, "CDAG");
    index_type l = cdag.first_leaf();
    // std::cerr << "l = " << l << std::endl;
    assert(l != no_such_index);
    for (index_type i = 0; i < cdag.predecessors(l).length(); i++) {
      index_type n_i = cdag.predecessors(l)[i];
      assert(n_i < size());
      for (index_type j = 0; j < cdag.node_label(n_i).length(); j++) {
	index_type n_j = cdag.node_label(n_i)[j];
	assert(n_j < size());
	assert(!cdag.node_label(l).empty());
	assert(cdag.node_label(l)[0] < size());
	node_label(n_j).insert(node_label(cdag.node_label(l)[0]));
      }
    }
    cdag.remove_node(l);
  }
}

void index_set_graph::merge_labels_downwards()
{
  strongly_connected_components();
  equivalence eq;
  component_partitioning(eq);
  // cdag: DAG of strongly connected components of *this
  index_set_graph cdag(*((graph*)this), eq);
  // 1. union node labels among nodes in same component
  for (index_type k = 0; k < cdag.size(); k++) {
    assert(!cdag.node_label(k).empty());
    merge_labels(cdag.node_label(k));
  }
  // 2. propagate downwards...
  while (!cdag.empty()) {
    index_type l = cdag.first_root();
    assert(l != no_such_index);
    for (index_type i = 0; i < cdag.successors(l).length(); i++) {
      index_type n_i = cdag.successors(l)[i];
      for (index_type j = 0; j < cdag.node_label(n_i).length(); j++) {
	index_type n_j = cdag.node_label(n_i)[j];
	node_label(n_j).insert(node_label(cdag.node_label(l)[0]));
      }
    }
    cdag.remove_node(l);
  }
}

index_set_graph& index_set_graph::subgraph_set_size_gt
(index_set_graph& g, index_type l)
{
  index_set r;
  for (index_type k = 0; k < size(); k++)
    if (node_has_label(k)) {
      if (node_label(k).length() > l) r.insert(k);
    }
  subgraph(g, r);
  return g;
}

void index_set_graph::write_edge_set(::std::ostream& s) const
{
  s << '{';
  bool first = true;
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j)) {
	if (!first) s << ',';
	first = false;
	s << i << ":";
	if (node_has_label(i))
	  s << node_label(i);
	else
	  s << "[]";
	s << "->" << j << ":";
	if (node_has_label(j))
	  s << node_label(j);
	else
	  s << "[]";
      }
  s << '}';
}

void index_set_graph::write_digraph(::std::ostream& s, const char* name) const
{
  if (strncmp(name, "cluster", 7) == 0)
    s << "subgraph";
  else
    s << "digraph";
  s << " \"" << name << "\"" << ::std::endl << "{" << ::std::endl;
  for (index_type k = 0; k < size(); k++) {
    s << "\t" << k << " [label=\"" << k << ":{";
    if (node_has_label(k)) {
      for (index_type i = 0; i < node_label(k).length(); i++) {
	if (i > 0) s << ",";
	s << node_label(k)[i];
      }
    }
    s << "}\"];" << ::std::endl;
  }
  for (index_type i = 0; i < size(); i++)
    for (index_type j = 0; j < size(); j++)
      if (adjacent(i, j))
	s << "\t" << i << " -> " << j << ";" << ::std::endl;
  s << "}" << ::std::endl;
}

END_HSPS_NAMESPACE
