
#include "rng.h"
#include <cmath>
#include <iostream>
#include <unistd.h>
#include <time.h>

BEGIN_HSPS_NAMESPACE

const double PI = 2 * std::acos(0.0);

void RNG::seed_with_pid()
{
  seed(getpid());
}

void RNG::seed_with_time()
{
  seed(time(0));
}

unsigned long RNG::random_in_range
(unsigned long range)
{
  unsigned long v = random();
  // std::cerr << "random_in_range(" << range << ") = " << v << " mod " << range << " = " << (v % range) << std::endl;
  return (v % range);
}

unsigned long RNG::random_in_range
(unsigned long range, unsigned long except)
{
  if (except >= range) {
    return random_in_range(range);
  }
  if (range <= 1) {
    ::std::cerr << "error: empty range in random_in_range("
	      << range << ", " << except << ")" << ::std::endl;
    exit(255);
  }
  unsigned long r = (random() % (range - 1));
  if (r < except)
    return r;
  else
    return (r + 1);
}

double RNG::random_double(unsigned long div)
{
  return ((random() % div)/(double)div);
}

double RNG::normal_sample(double mean, double var)
{
  double x2pi = random_double(max()) * 2.0 * PI;
  double g2rad = std::sqrt(-2.0 * std::log(1.0 - random_double(max())));
  return (std::cos(x2pi) * g2rad * sqrt(var)) + mean;
}

unsigned long RNG::binomial_sample(unsigned long n, double p)
{
  double s = round(normal_sample(n * p, n * p * (1 - p)));
  if (s < 0) return 0;
  else if (s > n) return n;
  else return (unsigned long)s;
}

index_type RNG::select_one_of(const bool_vec& selectable)
{
  index_type n = selectable.count(true);
  if (n == 0) return no_such_index;
  index_type m = random_in_range(n);
  for (index_type k = 0; k < selectable.length(); k++) if (selectable[k]) {
    if (m == 0) return k;
    m -= 1;
  }
  assert(0); // should always return before reaching this point
}

index_type RNG::select_one_of(const index_vec& selectable)
{
  if (selectable.empty()) return no_such_index;
  index_type m = random_in_range(selectable.length());
  return selectable[m];
}

void RNG::select_fixed_set
(index_set& s, index_type m, index_type n)
{
  assert(n >= m);
  s.clear();
  while (m > 0) {
    index_type i = random_in_range(n - s.length());
    index_type k = 0;
    while ((i > 0) || s.contains(k)) {
      if (s.contains(k)) k += 1;
      else {
	k += 1;
	i -= 1;
      }
    }
    assert(k < n);
    assert(!s.contains(k));
    s.insert(k);
    m -= 1;
  }
}

void RNG::select_variable_set
(index_set& s, index_type m, index_type n)
{
  assert(n > 0);
  s.clear();
  for (index_type k = 0; k < n; k++)
    if (random_in_range(n) < m) s.insert(k);
}

void RNG::select_non_empty_variable_set
(index_set& s, index_type m, index_type n)
{
  assert(n > 0);
  assert(m > 0);
  s.clear();
  index_type i = random_in_range(n);
  s.insert(i);
  if (m > 1) {
    for (index_type k = 0; k < n; k++)
      if ((k != i) && (random_in_range(n - 1) < (m - 1))) s.insert(k);
  }
}

void LC_RNG::seed(unsigned long s)
{
  x = s;
}

unsigned long LC_RNG::seed_value()
{
  return x;
}

unsigned long LC_RNG::random()
{
  x = ((a * x) + b) % mod;
  return x;
}

unsigned long LC_RNG::max()
{
  return mod - 1;
}

END_HSPS_NAMESPACE
