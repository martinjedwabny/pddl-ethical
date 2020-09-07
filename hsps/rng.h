#ifndef RNG_H
#define RNG_H

#include "config.h"
#include "index_type.h"

BEGIN_HSPS_NAMESPACE

class RNG {
 public:
  virtual ~RNG() { };

  virtual void seed(unsigned long s) = 0;
  virtual unsigned long seed_value() = 0;
  virtual unsigned long random() = 0;

  void seed_with_pid();
  void seed_with_time();

  unsigned long random_in_range(unsigned long range);
  unsigned long random_in_range(unsigned long range, unsigned long except);

  double random_double(unsigned long div);
  double normal_sample(double mean, double var);
  unsigned long binomial_sample(unsigned long n, double p);

  index_type select_one_of(const bool_vec& sel);
  index_type select_one_of(const index_vec& sel);

  // select exactly m of n distinct elements
  void select_fixed_set(index_set& s, index_type m, index_type n);
  // select on average m of n distinct elements (min 0, max n)
  void select_variable_set(index_set& s, index_type m, index_type n);
  // select on average m of n distinct elements (min 1, max n)
  void select_non_empty_variable_set(index_set& s, index_type m, index_type n);

  virtual unsigned long max() = 0;
};

class LC_RNG : public RNG {
  unsigned long a;
  unsigned long b;
  unsigned long mod;
  unsigned long x;
 public:
  LC_RNG()
    : a(23), b(0), mod(100000001), x(100000001 - 1) { };
  LC_RNG(unsigned long _a, unsigned long _b, unsigned long m)
    : a(_a), b(_b), mod(m), x(m - 1) { };
  LC_RNG(unsigned long s)
    : a(23), b(0), mod(100000001), x(s) { };
  LC_RNG(unsigned long _a, unsigned long _b, unsigned long m, unsigned long s)
    : a(_a), b(_b), mod(m), x(s) { };
  virtual ~LC_RNG() { };

  virtual void seed(unsigned long s);
  virtual unsigned long seed_value();
  virtual unsigned long random();
  virtual unsigned long max();
};

END_HSPS_NAMESPACE

#endif
