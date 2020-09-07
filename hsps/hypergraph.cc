
#include "hypergraph.h"

BEGIN_HSPS_NAMESPACE

hypergraph::hypergraph()
  : _size(0)
{
  // done
}

hypergraph::hypergraph(index_type n)
  : _size(n), _in(EMPTYSET, n)
{
  // done
}

hypergraph::hypergraph(const hypergraph& h)
  : _size(h._size), _edges(h._edges), _in(h._in)
{
  // done
}

hypergraph::~hypergraph()
{
  // done
}

index_type hypergraph::size() const
{
  return _size;
}

index_type hypergraph::n_edges() const
{
  return _edges.length();
}

const index_set& hypergraph::edge(index_type k) const
{
  assert(k < _edges.length());
  return _edges[k];
}

const index_set& hypergraph::adjacent_edges(index_type k) const
{
  assert(k < _size);
  return _in[k];
}

void hypergraph::add_edge(const index_set& e)
{
  _edges.append(e);
  for (index_type k = 0; k < e.length(); k++) {
    assert(e[k] < _size);
    _in[e[k]].insert(_edges.length() - 1);
  }
}

void hypergraph::add_singleton_edge(index_type n)
{
  index_set e;
  e.assign_singleton(n);
  add_edge(e);
}

void hypergraph::add_binary_edge(index_type n1, index_type n2)
{
  index_set e;
  e.insert(n1);
  e.insert(n2);
  add_edge(e);
}

bool hypergraph::is_independent(const index_set& ns) const
{
  bool_vec t(false, _edges.length());
  for (index_type k = 0; k < ns.length(); k++) {
    assert(ns[k] < _size);
    for (index_type i = 0; i < _in[ns[k]].length(); i++)
      if (!t[_in[ns[k]][i]]) {
	if (ns.contains(_edges[_in[ns[k]][i]]))
	  return false;
	t[_in[ns[k]][i]] = true;
      }
  }
  return true;
}

bool hypergraph::is_independent(const bool_vec& ns) const
{
  bool_vec t(false, _edges.length());
  for (index_type k = 0; (k < ns.length()) && (k < _size); k++)
    if (ns[k])
      for (index_type i = 0; i < _in[k].length(); i++)
	if (!t[_in[k][i]]) {
	  if (ns.contains(_edges[_in[k][i]]))
	    return false;
	  t[_in[k][i]] = true;
	}
  return true;
}

void hypergraph::independent_sets(index_set_vec& is)
{
  index_set_vec cs;
  cs.append(EMPTYSET);
  bool_vec o(false, _edges.length());
  for (index_type k = 0; k < _size; k++)
    for (index_type i = 0; i < _in[k].length(); i++) {
      index_type e = _in[k][i];
      if (!o[e]) {
	// std::cerr << "uncovered edge: " << _edges[e] << std::endl;
	index_type m = cs.length();
	for (index_type j = 0; j < m; j++) {
	  // std::cerr << " set " << cs[j] << "...";
	  if (cs[j].first_common_element(_edges[e]) == no_such_index) {
	    // std::cerr << " does not cover this edge" << std::endl;
	    for (index_type l = 1; l < _edges[e].length(); l++) {
	      index_set x(cs[j]);
	      x.insert(_edges[e][l]);
	      cs.append(x);
	    }
	    cs[j].insert(_edges[e][0]);
	  }
	  // else {
	  //  std::cerr << " already covers this edge" << std::endl;
	  // }
	}
	cs.reduce_to_minimal();
	o[_in[k][i]] = true;
      }
    }
  is.assign_value(EMPTYSET, cs.length());
  for (index_type k = 0; k < is.length(); k++) {
    is[k].fill(_size);
    is[k].subtract(cs[k]);
  }
  // std::cerr << "edges: " << _edges << std::endl;
  // std::cerr << "minimal covering sets: " << cs << std::endl;
  // std::cerr << "maximal independent sets: " << is << std::endl;
}


END_HSPS_NAMESPACE
