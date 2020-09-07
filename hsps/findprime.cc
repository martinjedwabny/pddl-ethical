
#include "index_type.h"

int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cerr << argv[0] << " <positive number>" << std::endl;
    exit(0);
  }

  HSPS::index_type t = atoi(argv[1]);
  if (t == 0) {
    std::cerr << "BAD USER! target number must be POSITIVE!" << std::endl;
    exit(1);
  }

  HSPS::index_vec f(0, 0);
  HSPS::factors(t, f);
  if (f.length() == 1) {
    std::cout << t << " is prime" << std::endl;
    exit(0);
  }

  HSPS::index_type d = 1;
  bool done = false;

  while (!done) {
    if (t > d) {
      HSPS::factors(t - d, f);
      if (f.length() == 1) {
	std::cerr << t - d << " is prime" << std::endl;
	done = true;
      }
    }
    if (!done) {
      HSPS::factors(t + d, f);
      if (f.length() == 1) {
	std::cerr << t + d << " is prime" << std::endl;
	done = true;
      }
    }
    d += 1;
  }
}
