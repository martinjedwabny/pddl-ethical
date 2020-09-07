
#include "rng.h"

int main(int argc, char *argv[])
{
  HSPS::LC_RNG rng;
  HSPS::index_type n = 1;
  unsigned long m = rng.max();

  for (int k = 1; k < argc; k++) {
    if (((strcmp(argv[k],"-rnd") == 0) ||
	 (strcmp(argv[k],"-r") == 0)) &&
	(k < argc - 1)) {
      rng.seed(atoi(argv[++k]));
    }
    else if ((strcmp(argv[k],"-rnd-pid") == 0) ||
	     (strcmp(argv[k],"-rp") == 0)) {
      rng.seed_with_pid();
    }
    else if ((strcmp(argv[k],"-rnd-time") == 0) ||
	     (strcmp(argv[k],"-rt") == 0)) {
      rng.seed_with_time();
    }
    else if ((strcmp(argv[k],"-n") == 0) && (k < argc - 1)) {
      n = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-m") == 0) && (k < argc - 1)) {
      m = atoi(argv[++k]);
    }
  }

  while (n > 0) {
    unsigned long i = rng.random_in_range(m);
    std::cerr << i << std::endl;
    n -= 1;
  }
}
