
#include "tcn.h"

BEGIN_HSPS_NAMESPACE

void STN::init(const weighted_graph& g)
{
  init(g.size());
  for (index_type i = 0; i < g.size(); i++)
    for (index_type j = 0; j < g.size(); j++)
      if (g.adjacent(i, j))
	set_min(i, j, g.weight(i, j));
  compute_minimal();
}

void STN::set_max(index_type t0, index_type t1, NTYPE d)
{
  assert(t0 < length());
  assert(t1 < length());
  if (d < (*this)[t0][t1]) {
    (*this)[t0][t1] = d;
    _minimal = false;
  }
}

void STN::set_min(index_type t0, index_type t1, NTYPE d)
{
  assert(t0 < length());
  assert(t1 < length());
  if (d > (-1 * ((*this)[t1][t0]))) {
    (*this)[t1][t0] = (-1 * d);
    _minimal = false;
  }
}

bool STN::consistent() {
  if (!_minimal) compute_minimal();
  return _consistent;
}

void STN::compute_minimal() {
  for (index_type k = 0; k < length(); k++) {
    for (index_type i = 0; i < length(); i++)
      for (index_type j = 0; j < length(); j++) {
	NTYPE d = (*this)[i][k] + (*this)[k][j];
	if (d < (*this)[i][j]) (*this)[i][j] = d;
      }
  }
  _minimal = true;
  _consistent = true;
  for (index_type k = 0; k < length(); k++)
    if ((*this)[k][k] < ZERO) _consistent = false;
}

NTYPE STN::min_distance(index_type t0, index_type t1)
{
  assert(t0 < length());
  assert(t1 < length());
  if (!_minimal) compute_minimal();
  return (-1 * ((*this)[t1][t0]));
}

NTYPE STN::max_distance(index_type t0, index_type t1)
{
  assert(t0 < length());
  assert(t1 < length());
  if (!_minimal) compute_minimal();
  return (*this)[t0][t1];
}

bool STN::admits_max(index_type t0, index_type t1, NTYPE d)
{
  assert(t0 < length());
  assert(t1 < length());
  return (min_distance(t0, t1) < d);
}

bool STN::admits_min(index_type t0, index_type t1, NTYPE d)
{
  assert(t0 < length());
  assert(t1 < length());
  NTYPE dmax = max_distance(t0,t1);
  return (FINITE(d) && ((d < dmax) || !FINITE(dmax)));
}

bool STN::admits_in(index_type t0, index_type t1, NTYPE min, NTYPE max)
{
  assert(t0 < length());
  assert(t1 < length());
  NTYPE dmin = min_distance(t0, t1);
  NTYPE dmax = max_distance(t0, t1);
  return (FINITE(min) && ((min <= dmax) || !FINITE(dmax)) &&
	  FINITE(dmin) && ((dmin <= max) || !FINITE(max)));
}

void STN::precedence_graph(graph& g)
{
  g.init(length());
  for (index_type i = 0; i < length(); i++)
    for (index_type j = 0; j < length(); j++)
      if ((i != j) && (min_distance(i, j) >= 0)) g.add_edge(i, j);
}

void STN::write(std::ostream& s)
{
  s << "{";
  bool first = true;
  for (index_type i = 0; i < length(); i++)
    for (index_type j = i + 1; j < length(); j++) {
      NTYPE dmin = min_distance(i, j);
      NTYPE dmax = max_distance(i, j);
      if (FINITE(dmin) || FINITE(dmax)) {
	if (!first) s << ", "; else first = false;
	if (INFINITE(dmin)) {
	  if (dmax < 0) {
	    s << -1*dmax << " <= " << "T" << i << " - " << "T" << j;
	  }
	  else {
	    s << "T" << j << " - " << "T" << i << " <= " << dmax;
	  }
	}
	else if (INFINITE(dmax)) {
	  if (dmin < 0) {
	    s << "T" << i << " - " << "T" << j << " <= " << -1*dmin;
	  }
	  else {
	    s << dmin << " <= " << "T" << j << " - " << "T" << i;
	  }
	}
	else if (dmin == dmax) {
	  if (dmin < 0) {
	    s << "T" << i << " - " << "T" << j << " == " << -1*dmin;
	  }
	  else {
	    s << "T" << j << " - " << "T" << i << " == " << dmin;
	  }
	}
	else {
	  s << dmin << " <= "
	    << "T" << j << " - " << "T" << i
	    << " <= " << dmax;
	}
      }
    }
  s << "}";
}

END_HSPS_NAMESPACE
