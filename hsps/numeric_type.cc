
#include "numeric_type.h"

BEGIN_HSPS_NAMESPACE

cost_vec_util::decreasing_cost_order cost_vec_util::decreasing;
cost_vec_util::increasing_cost_order cost_vec_util::increasing;

NTYPE cost_vec_util::max(const cost_vec& v)
{
  index_type i = v.arg_max();
  return (i == no_such_index ? NEG_INF : v[i]);
}

NTYPE cost_vec_util::min(const cost_vec& v)
{
  index_type i = v.arg_min();
  return (i == no_such_index ? NEG_INF : v[i]);
}

END_HSPS_NAMESPACE
