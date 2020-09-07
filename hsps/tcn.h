#ifndef TCN_H
#define TCN_H

#include "config.h"
#include "numeric_type.h"
#include "graph.h"

BEGIN_HSPS_NAMESPACE

class STN : public cost_matrix {
  bool _minimal;
  bool _consistent;
 public:
  STN()
    : cost_matrix(POS_INF, 0, 0), _minimal(true), _consistent(true) { };
  STN(index_type size)
    : cost_matrix(POS_INF, size, size), _minimal(true), _consistent(true) { };
  STN(const STN& tcn)
    : cost_matrix(tcn),
    _minimal(tcn._minimal),
    _consistent(tcn._consistent) { };

  void init(index_type size) {
    cost_matrix::assign_value(POS_INF, size, size);
  }

  void init(const weighted_graph& g);

  void set_max(index_type t0, index_type t1, NTYPE d);
  void set_min(index_type t0, index_type t1, NTYPE d);

  void  compute_minimal();
  bool  consistent();
  NTYPE min_distance(index_type t0, index_type t1);
  NTYPE max_distance(index_type t0, index_type t1);

  bool  admits_min(index_type t0, index_type t1, NTYPE d);
  bool  admits_max(index_type t0, index_type t1, NTYPE d);
  bool  admits_in(index_type t0, index_type t1, NTYPE min, NTYPE max);

  void precedence_graph(graph& g);

  void  write(std::ostream& s);
};

END_HSPS_NAMESPACE

#endif
