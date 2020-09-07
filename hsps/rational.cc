
#include <ctype.h>
#include <math.h>
#include "rational.h"

BEGIN_HSPS_NAMESPACE

const long CONVERSION_MAX_DIVISOR = 100000;

inline double floor_d(double d) {
  return floor(d);
}

long euclid(long n, long k, long& a, long& b) {
  long q = n / k;
  long r = n % k;
  /* n = q*k + r */
  if (r == 0) {
    /* 1*n + (1-q)*k = k */
    a = 1;
    b = 1-q;
    return k;
  }
  else {
    long a1, b1;
    long c = euclid(k, r, a1, b1);
    /* a1*k + b1*r = c, n - q*k = r => a1*k + b1*(n - q*k) = c */
    a = b1;
    b = a1 - (b1*q);
    return c;
  }
}

long gcd(long n, long k) {
  long q = n / k;
  long r = n % k;
  /* n = q*k + r */
  if (r == 0) return k;
  else return gcd(k, r);
}

long lcm(long n, long k) {
  long d = gcd(n, k);
  return ((n / d) * (k / d) * d);
}

unsigned long ilog(unsigned long n)
{
  unsigned long k = 0;
  while (n > 0) {
    k += 1;
    n = (n / 2);
  }
  return k;
}

long imag(long n)
{
  return (n < 0 ? n * -1 : n);
}

rational rational::dtor(double v) {
  long d = 1;
  while (((v - floor_d(v)) > 0.0) && (d < CONVERSION_MAX_DIVISOR)) {
    v *= 10.0;
    d *= 10;
  }
  return reduce(rational((long)floor_d(v), d));
}

rational rational::ator(char* s) {
  long n = 0;
  long d = 1;
  if (*s == '-') {
    d = -1;
    s += 1;
  }
  while (isdigit(*s)) {
    n = (n*10) + (*s - 48);
    s += 1;
  }
  if (*s == '.') {
    s += 1;
    while (isdigit(*s)) {
      n = (n*10) + (*s - 48);
      d *= 10;
      s += 1;
    }
  }
  return reduce(rational(n, d));
}

END_HSPS_NAMESPACE
