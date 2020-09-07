using namespace std;

#include <util.options.h>
#include <util.tools.h>

int getPos(const char *str, char c) {
  for(int i = 0; str[i] != 0; i++)
    if(str[i] == c)
      return i;
  return -1;
}

Options::Options() { 
  unidirectional = false; // unidirectional search
  zerocost = false;        // zero-cost operators

  onlyTransition = false; // stop after transition function building
  compression = false;    // state compression
  sequential = false;       // sequential goal finding
  maximize = false;    // state compression
  disjoint = false;       // sequential goal finding
  admiss = false;         // apply admissible heuristic
  vis = false;            // visualization output
  forward = true;            // visualization output
  ghsearch = false;       // bucketing
  algorithm = Options::BRANCHANDBOUND;     // default algorithm
  ffheuristic = true;    // apply ff-like symbolic heursitic 
  maxState = 1000000;    // single hash table size
  maxPattern = 30;       // pattern state space size
  bddPattern = false;    // symbolic pattern database
  pattern = false;       // apply pattern data base
  times = 1;             // no of abstractions
  weight = 100;          // weight of WIDA*
  precision = 1000;      // precision for relaxed scheduling
  length = 12;           // maximal solution length (2^^length)
  maxnumber = 1024*16;      // maximal number in PDDL3
  offset = 0;            // offset to trade sequentialism vs parallelism
  heur = 0;              // heuristic function splitting
  trans = 1;             // transition function splitting
  bound = 10000;         // upper bound
  resume = 0;            // resume option for pattern construction

  bddinit = 90000000;    // initial number of BDD-nodes
  bddmaxincrease = 128000000;    // maxincrease of BDD-nodes
  setAll(COMPETITION); // set output mode for competition
  noGenerations = 20;
  populSize = 5;
  // setAll(NORMAL);
}

void Options::setAll(int level) {
  for(int i = 0; i < ANZ_PHASES; i++)
    outputLevel[i] = level;
}

void Options::read(int argc, char *argv[], string &domFile, string &probFile) {
  vector<string> remaining;
  for(int i = 1; i < argc; i++) {
    const char *s = argv[i];
    //    cout << s << endl;
    if(s[0] == '-') {
      char c = s[1];
      int level = getPos("csnvd", c);
      if(level != -1) {
        if(s[2] == 0) {
          setAll(level);
        } else {
          for(const char *c = s + 2; *c != 0; c++) {
            int opt = getPos("pcmeobths", *c);
            if(opt == -1)
              error(string("invalid phase <") + *c + ">");
            outputLevel[opt] = level;
          }
        }
      } else if(getPos("?h", c) != -1) {
        ::error(usage);
      }
#ifndef GROUND
      else if(c == 'W') {
        weight = atoi(argv[++i]);
        admiss = true;
      } else if(c == 'O') {
        offset = atoi(argv[++i]);
      } else if(c == 'I') {
        bddinit = atoi(argv[++i]);
      } else if(c == 'N') {
        resume = atoi(argv[++i]);
      } else if(c == 'L') {
        length = atoi(argv[++i]);
      } else if(c == 'T') {
        trans = atoi(argv[++i]);
      } else if(c == 'U') {
        bound = atoi(argv[++i]);
      } else if(c == 'X') {
        bddinit = atoi(argv[++i]);
      } else if(c == 'Y') {
        bddmaxincrease = atoi(argv[++i]);
      } else if(c == 'M') {
        maximize = true;
      } else if(c == 'Z') {
        zerocost = true;
        unidirectional = true;
      } else if(c == 'D') {
        disjoint = true;
      } else if(c == 'H') {
        heur = atoi(argv[++i]);
      } else if(c == 'P') {
        maxPattern = atoi(argv[++i]);
	pattern = true;
	bddPattern = true;
	geneticpattern= true;
        ghsearch = true;
      } else if(c == 'B') {
        maxPattern = atoi(argv[++i]);
	bddPattern = true;
	pattern = true;
        ghsearch = true;
      } else if(c == 'G') {
        ghsearch = true;
      } else if(c == 'i') {
        vis = true;
      } else if(c == 'R') {
        forward = false;
      } else if(c == 'S') {
	  sequential = true;
	  ffheuristic = true;
	  pattern = false;
      } else if(c == 'F') {
        ffheuristic = true;
        pattern = false;
      } else if(c == 'A') {
        algorithm = atoi(argv[++i]);
      } else if(c == 'p') {
        onlyPreprocess = true;
      } else if(c == 'a') {
        notAdditive = true;
      } else if(c == 'j') {
        noGenerations = atoi(argv[++i]);
	populSize = atoi(argv[++i]);
      } else if(c == 't') {
        onlyTransition = true;
      } else if(c == '#') {
    pattern = true;
      } else if(c == 0) {
        error("missing option");
      } else {
        error(string("invalid option -") + s[1]);
      }
#endif
    } else {
      remaining.push_back(s);
    }
  }
  int s = remaining.size();
  if(s == 0) {
    probFile = "problem.pddl";
    domFile = "domain.pddl";
  } else if(s == 1) {
    probFile = remaining[0];
    domFile = "domain.pddl";
  } else if(s == 2) {
    domFile = remaining[0];
    probFile = remaining[1];
  } else {
    error("more than two files in argument list");
  }
}

void Options::changeStrategy() {
    algorithm = Options::BDDA;     // 2nd algorithm
    bddPattern = true;
    pattern = true;
    ghsearch = true;
}

void Options::error(string s) {
  ::error(s + "\nType planer -? for help.\n");
}

bool Options::allSilent() {
  for(int i = 0; i < ANZ_PHASES; i++)
    if(!silent(i))
      return false;
  return true;
}

Options options;

string Options::usage =
"\nMIPS 2.2 - the Model Checking Integrated Planning System \n"
"developed by Stefan Edelkamp\n\n"
//"thanks to Malte Helmert for cooperation in MIPS v1\n\n"
"usage: bdd-solver [<option> ...]\n"
"       bdd-solver [<option> ...] <problem file>\n"
"       bdd-solver [<option> ...] <domain file> <problem file>\n\n"
  "If no domain file is specified, domain.pddl serves as a default.\n"
  "If no problem file is specified, problem.pddl serves as a default.\n\n"
  "Options:\n\n"
  "Exploration algorithm:\n"
  "  -A 0 = Symbolic BFS \n"
  "  -A 1 = Symbolic Branch-And-Bound \n"
  "  -A 2 = Symbolic Dijkstra Search \n"
  "  -A 3 = Symbolic A* \n"
  "  -A 4 = Symbolic Pure Heuristic Search \n"
  "Heuristic Functions: \n"
  "  -H:   HSP max-atom relaxation (default)\n"
//  "  -P x: Pattern database heuristic, log number of explicit patterns \n"
  "  -B x: Pattern database heuristic, log number of symbolic patterns \n\n"
  "  -P x   : Pattern database heuristic, optimized with genetic algorithms, \n"
  "        log number of explicit patterns in initialisation \n"
  "Weightening of heuristic function \n"
  "  -W x: 1 + weight in x%   (default 100)\n\n"
  "Various: \n"
  "  -X x: Initial number of BDD nodes \n"
  "  -Y x  : max increase of BDD nodes \n"
//  "  -p  : flushing intermediate representation \n"
  "  -v  : verbose information\n"
  "  -d  : debug information\n";


