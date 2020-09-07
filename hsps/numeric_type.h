#ifndef NUMERIC_TYPE_H
#define NUMERIC_TYPE_H

#include "config.h"
#include "rational.h"

#include "index_type.h"
#include "rng.h"

#ifdef NTYPE_RATIONAL
#include <iostream>
#include <iomanip>

#define NTYPE  HSPS::rational
#define NNTYPE HSPS::rational::XR

#define N_TO_NN(x) x
#define NN_TO_N(x) HSPS::rational(x)

#define A_TO_N(x)   HSPS::rational::ator(x)
#define I_TO_N(x)   HSPS::rational(x)
#define R_TO_N(x,y) (HSPS::rational(x,y).reduce())
#define D_TO_N(x)   HSPS::rational::dtor(x)
#define N_TO_D(x)   ((x).decimal())
#define N_TO_R(x)   (x)

#define INFINITE(x) x.infinite()
#define FINITE(x)   x.finite()
const HSPS::rational POS_INF(1,0);
const HSPS::rational NEG_INF(-1,0);
const HSPS::rational ZERO(0,1);
#define INTEGRAL(x) x.integral()
#define FLOOR(x) x.floor()
#define FLOOR_TO_INT(x) (x.floor().numerator())
#define FRAC(x) x.frac()
#define ROUND_TO(x,p) rational::round_to(x, p)
#define IS_ZERO(x) x.zero()

#define MIN(x,y)    HSPS::rational::min(x,y)
#define MAX(x,y)    HSPS::rational::max(x,y)

#define HASH1(x)     ((index_type)((x).numerator() - (x).divisor()))
#define HASH2(x,y)   HASH1(((x) + 1) * ((y) + 1))

#define PRINT_NTYPE(x) std::resetiosflags(std::ios::scientific) << ((x).decimal())

#endif /* NTYPE_RATIONAL */

#ifdef NTYPE_FLOAT

#include <cmath>

#define NTYPE  float
#define NNTYPE float

#define N_TO_NN(x) x
#define NN_TO_N(x) x
#define A_TO_NN(x) atof(x)

#define A_TO_N(x)   atof(x)
#define I_TO_N(x)   ((float)(x))
#define R_TO_N(x,y) (((float)(x))/((float)(y)))
#define D_TO_N(x)   (x)
#define N_TO_D(x)   (x)
#define N_TO_R(x)   HSPS::rational::dtor(x)

#define INFINITE(x) (!std::isfinite<float>(x))
#define FINITE(x)   std::isfinite<float>(x)

/* definitions of isinf/finite from cygwin ieeefp.h (should not be
   needed anymore since isfinite is in c++ std):
#define INFINITE(x) (((*(long *)&(x) & 0x7f800000L)==0x7f800000L) && \
		    ((*(long *)&(x) & 0x007fffffL)==0000000000L))
#define FINITE(x)   (((*(long *)&(x) & 0x7f800000L)!=0x7f800000L))
*/

const double POS_INF = ((float)(1.0/0.0));
const double NEG_INF = ((float)(-1.0/0.0));
const double ZERO = ((float)0);

#define INTEGRAL(x) (floor(x) == (x))
#define FLOOR(x) floor(x)
#define FLOOR_TO_INT(x) ((long)floor(x))
#define FRAC(x)  (x - floor(x))
#define ROUND_TO(x,p) (floor((x) * (p)) / (p))
#define IS_ZERO(x) ((x) == ((float)0))

inline float MIN(float x, float y) {
  if (x < y) return x; else return y;
}

inline float MAX(float x, float y) {
  if (x > y) return x; else return y;
}

#define HASH1(x)     ((index_type)(x))
#define HASH2(x,y)   HASH1((x) * (y))

#define PRINT_NTYPE(x) x

#endif /* NTYPE_FLOAT */

BEGIN_HSPS_NAMESPACE

inline NTYPE random_numeric
(NTYPE min, NTYPE max, unsigned long prec, RNG& rng)
{
  NTYPE d = (max - min);
  NTYPE s = (d / prec);
  unsigned long r = rng.random_in_range(prec + 1);
  return ((r*s) + min);
}

class amt_vec : public auto_expanding_vector<NTYPE> {
 public:
  amt_vec()
    : auto_expanding_vector<NTYPE>() { };
  amt_vec(const NTYPE& v, index_type l)
    : auto_expanding_vector<NTYPE>(v, l) { };
  amt_vec(const amt_vec& vec)
    : auto_expanding_vector<NTYPE>(vec) { };

  int compare(const amt_vec& vec, index_type n);
  //  1 if this dominates vec
  // -1 if vec dominates this
  //  0 if incomparable
  int dcompare(const amt_vec& vec, index_type n);
  index_type hash(index_type n);
  void write(std::ostream& s, index_type n);
};

inline int amt_vec::compare(const amt_vec& vec, index_type n)
{
  for (index_type k = 0; k < n; k++) {
    if ((*this)[k] < vec[k]) return -1;
    else if ((*this)[k] > vec[k]) return 1;
  }
  return 0;
}

inline int amt_vec::dcompare(const amt_vec& vec, index_type n)
{
  bool this_less_than_vec = false;
  bool vec_less_than_this = false;
  for (index_type k = 0; k < n; k++) {
    if ((*this)[k] < vec[k]) this_less_than_vec = true;
    else if ((*this)[k] > vec[k]) vec_less_than_this = true;
  }
  if (this_less_than_vec && !vec_less_than_this) return -1;
  else if (!this_less_than_vec && vec_less_than_this) return 1;
  else return 0;
}

inline index_type amt_vec::hash(index_type n)
{
  if (n == 0) return 0;
  if (n == 1) return HASH1((*this)[0]);
  index_type h = 0;
  for (index_type k = 0; k < n - 1; k++) {
    h += HASH2((*this)[k], (*this)[k + 1]);
  }
  return h;
}

inline void amt_vec::write(std::ostream& s, index_type n)
{
  s << '[';
  for (index_type k = 0; k < n; k++) {
    if (k > 0) s << ',';
    s << PRINT_NTYPE((*this)[k]);
  }
  s << ']';
}

typedef lvector<NTYPE> cost_vec;
typedef svector<NTYPE> cost_set;
typedef matrix<NTYPE> cost_matrix;

class cost_vec_util : public cost_vec
{
 public:

  class decreasing_cost_order : public cost_vec::order {
  public:
    virtual bool operator()
      (const NTYPE& v0, const NTYPE& v1) const
      { return (v0 > v1); };
  };

  class increasing_cost_order : public cost_vec::order {
  public:
    virtual bool operator()
      (const NTYPE& v0, const NTYPE& v1) const
      { return (v0 < v1); };
  };

  static class decreasing_cost_order decreasing;
  static class increasing_cost_order increasing;

  static NTYPE max(const cost_vec& v);
  static NTYPE min(const cost_vec& v);

  NTYPE max() const { return max(*this); };
  NTYPE min() const { return min(*this); };
};


struct interval : public comparable_pair<NTYPE> {
  interval(const NTYPE& v1, const NTYPE& v2) :
    comparable_pair<NTYPE>(v1, v2) { };
  interval(const NTYPE& v) :
    comparable_pair<NTYPE>(v) { };
  interval(const interval& p) :
    comparable_pair<NTYPE>(p) { };
  interval() :
    comparable_pair<NTYPE>(NEG_INF, POS_INF) { };
};

typedef std::pair<index_type, NTYPE> index_cost_pair;
typedef lvector<index_cost_pair> index_cost_vec;

inline std::ostream& operator<<(std::ostream& s, const index_cost_pair& p)
{
  s << '(' << p.first << ',' << p.second << ')';
}

inline std::ostream& operator<<(std::ostream& s, const interval& i)
{
  s << '[' << i.first << ',' << i.second << ']';
}

END_HSPS_NAMESPACE

#endif
