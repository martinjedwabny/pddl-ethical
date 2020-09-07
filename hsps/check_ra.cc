
// #define CHECK_RATIONAL_ARITHMETIC
// #define TRACE_PRINT_RATIONAL_ARITHMETIC

// #define VECTOR_ASSIGNMENT_BUG_TEST
// #define SUM_TEST

#include "rational.h"
#ifdef SUM_TEST
#include "stats.h"
#endif
#ifdef VECTOR_ASSIGNMENT_BUG_TEST
#include "vector.h"
#endif

int main(int argc, char *argv[]) {

  // HSPS::rational r0 = HSPS::rational(5370212, 4307205);
  // HSPS::rational r1 = HSPS::rational(11479, 160083);
  HSPS::rational r0 = HSPS::rational(0);
  HSPS::rational r1 = HSPS::rational(12, 127);

  HSPS::rational r2 = r0 + r1;
  HSPS::rational r3 = HSPS::safeadd(r0, r1);

  std::cerr << "r0 = " << r0 << " ~= " << r0.decimal() << std::endl;
  std::cerr << "r1 = " << r1 << " ~= " << r1.decimal() << std::endl;
  std::cerr << "r0 + r1 = " << r2 << " ~= " << r2.decimal() << std::endl;
  std::cerr << "safeadd(r0, r1) = " << r3 << " ~= " << r3.decimal()
	    << std::endl;

#ifdef VECTOR_ASSIGNMENT_BUG_TEST
  cost_vec v0(0, 0);
  cost_vec v1(0, 0);

  v0.assign_value(rational(7,11), 5);
  // std::cerr << "v0 = " << v0 << std::endl;
  v1.set_length(8);
  // v1.assign_value(0, 8);
  for (index_type k = 0; k < 7; k++) {
    // std::cerr << "v0[" << k << "] = " << v0.read_only(k) << std::endl;
    //     rational d = v0[k];
    //     v1[k] = d;
    v1[k] = v0[k];
    // std::cerr << "v1[" << k << "] = " << v1[k] << std::endl;
  }

  std::cerr << "v1 = " << v1 << std::endl;
#endif

#ifdef SUM_TEST
  if (argc < 2) {
    std::cerr << argv[0] << " <n>" << std::endl;
    exit(0);
  }

  long n = atoi(argv[1]);
  Stopwatch t;
  t.enable_interrupt(true);

  t.start();

  rational s = 0;
  for (long i = 1; i <= n; i++) {
    rational r(1, i);
#ifdef CHECK_RATIONAL_ARITHMETIC
    std::cerr << s << " + " << r << std::endl;
#endif
    s += r;
  }
  std::cerr << "sum i=1.." << n << " 1/i = " << s << std::endl;

  t.stop();
  std::cerr << n << " additions in " << t.time()
	    << " seconds (" << n/t.time() << " add/sec)"
	    << std::endl;
#endif

  return 0;
}
