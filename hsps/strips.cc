
#include "problem.h"
#include "preprocess.h"
#include "parser.h"

// options are global, for simplicity

// solve subgoals in reverse order
bool reverse = false;

// max recursion depth
HSPS::index_type max_d = 10;

// max plan length
HSPS::index_type max_l = 100;

// be VERY verbose
bool verbose = false;

HSPS::index_type next_unsolved_subgoal(const HSPS::index_set& s, const HSPS::index_set& g)
{
  for (HSPS::index_type k = 0; k < g.length(); k++)
    if (!s.contains(g[(reverse ? (g.length() - 1) - k : k)]))
      return g[(reverse ? (g.length() - 1) - k : k)];
  return HSPS::no_such_index;
}

bool strips
(HSPS::index_type d,        // current recursion depth
 HSPS::Instance& instance,  // the planning problem/domain
 const HSPS::index_set& s0, // initial state (set of true atoms)
 const HSPS::index_set& g,  // goal (set of atoms)
 HSPS::index_vec& p)        // the result plan ("out" parameter)
{
  if (verbose) {
    std::cerr << d << ": STRIPS called with s0 = ";
    instance.write_atom_set(std::cerr, s0);
    std::cerr << " and g = ";
    instance.write_atom_set(std::cerr, g);
    std::cerr << std::endl;
  }

  // since we'll modify state s0, we make a copy
  HSPS::index_set s(s0);

  // make sure the output plan is initially empty
  p.clear();

  // while there are unsolved subgoals...
  while (!s.contains(g)) {
    HSPS::index_type sub_g = next_unsolved_subgoal(s, g);
    assert(sub_g != HSPS::no_such_index);

    if (verbose) {
      std::cerr << d << ": current subgoal is "
		<< instance.atoms[sub_g].name
		<< std::endl;
    }

    // set 'solved' status of current subgoal to false
    bool solved = false;

    // unless the subgoal is already satisfied in the current state s
    if (s.contains(sub_g)) {
      solved = true;
      if (verbose) {
	std::cerr << d << ": subgoal " << instance.atoms[sub_g].name
		  << " is already true in current state" << std::endl;
      }
    }

    if (d > max_d) {
      if (verbose) {
	std::cerr << d << ": max recursion depth exceeded - returning FAILURE"
		  << std::endl;
      }
      return false;
    }

    // try relevant actions until the current subgoal is solved (or we
    // run out of actions)
    for (HSPS::index_type k = 0; (k < instance.n_actions()) && !solved; k++) {
      HSPS::Instance::Action& act = instance.actions[k];

      // if act is a relevant action...
      if ((act.add.contains(sub_g) > 0) &&
	  (act.del.count_common(g) == 0)) {

	if (verbose) {
	  std::cerr << d << ": action " << act.name
		    << " is relevant for subgoal "
		    << instance.atoms[sub_g].name
		    << std::endl;
	}

	// invoke strips recursively with act's preconditions as goal...
	HSPS::index_vec sub_p(HSPS::no_such_index, 0);
	if (strips(d + 1, instance, s, act.pre, sub_p)) {
	  // if the recursive call succeeded, sub_p contains a plan for
	  // achieving the preconditions of action act: apply this plan
	  // to state s

	  if (verbose) {
	    std::cerr << d << ": plan (";
	    for (HSPS::index_type i = 0; i < sub_p.length(); i++) {
	      if (i > 0) std::cerr << ", ";
	      std::cerr << i + 1 << "." << instance.actions[sub_p[i]].name;
	    }
	    std::cerr << ") found for preconditions of action "
		      << act.name << std::endl;
	  }

	  for (HSPS::index_type i = 0; i < sub_p.length(); i++) {
	    if (!s.contains(instance.actions[sub_p[i]].pre)) {
	      std::cerr << "error: action "
			<< instance.actions[sub_p[i]].name
			<< " in subplan not applicable in state ";
	      instance.write_atom_set(std::cerr, s);
	      std::cerr << std::endl;
	      exit(1);
	    }
	    s.subtract(instance.actions[sub_p[i]].del);
	    s.insert(instance.actions[sub_p[i]].add);
	  }

	  if (verbose) {
	    std::cerr << d << ": state after applying plan = ";
	    instance.write_atom_set(std::cerr, s);
	    std::cerr << std::endl;
	  }

	  // action act should now be applicable to state s: apply it
	  if (!s.contains(act.pre)) {
	    std::cerr << "error: action " << act.name
		      << " not applicable in state ";
	    instance.write_atom_set(std::cerr, s);
	    std::cerr << " resulting from subplan" << std::endl;
	    exit(1);
	  }
	  s.subtract(act.del);
	  s.insert(act.add);

	  if (verbose) {
	    std::cerr << d << ": state after applying action "
		      << act.name << " = ";
	    instance.write_atom_set(std::cerr, s);
	    std::cerr << std::endl;
	  }

	  // append subplan and action act to result plan (p)
	  for (HSPS::index_type i = 0; i < sub_p.length(); i++) {
	    p.append(sub_p[i]);
	  }
	  p.append(act.index);

	  if (p.length() > max_l) {
	    if (verbose) {
	      std::cerr << d << ": max plan length exceeded by (";
	      for (HSPS::index_type i = 0; i < p.length(); i++) {
		if (i > 0) std::cerr << ", ";
		std::cerr << i + 1 << "." << instance.actions[p[i]].name;
	      }
	      std::cerr << ") - returning FAILURE" << std::endl;
	    }
	    return false;
	  }

	  // continue with next subgoal
	  solved = true;
	}
	else if (verbose) {
	  std::cerr << d
		    << ": failed to find plan for preconditions of action "
		    << act.name << std::endl;
	}
      }
    }

    // if no relevant action found, or no actions preconditions could be
    // achieved, return failure
    if (!solved) {
      if (verbose) {
	std::cerr << d << ": subgoal " << instance.atoms[sub_g].name
		  << " could not be solved from state ";
	instance.write_atom_set(std::cerr, s);
	std::cerr << " - returning FAILURE" << std::endl;
      }
      return false;
    }
  }

  // all subgoals solved: return success (with current plan)
  if (verbose) {
    std::cerr << d << ": all subgoals achieved - returning SUCCESS"
	      << std::endl;
  }
  return true;
}

int main(int argc, char *argv[]) {
  HSPS::StringTable symbols(50, HSPS::lowercase_map);
  HSPS::Parser* reader = new HSPS::Parser(symbols);

  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-d") == 0)  && (k < argc - 1)) {
      max_d = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-l") == 0)  && (k < argc - 1)) {
      max_l = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-r") == 0) {
      reverse = true;
    }
    else if (strcmp(argv[k],"-v") == 0) {
      verbose = true;
    }
    else if (*argv[k] != '-') {
      reader->read(argv[k], false);
    }
  }

  std::cerr << "instantiating..." << std::endl;
  HSPS::Instance instance;
  reader->instantiate(instance);
  instance.cross_reference();

  std::cerr << "invoking STRIPS planner..." << std::endl;
  HSPS::index_vec plan(HSPS::no_such_index, 0);
  bool solved = strips(1, instance,
		       instance.init_atoms,
		       instance.goal_atoms,
		       plan);
  if (solved) {
    std::cout << "(:plan" << std::endl;
    for (HSPS::index_type i = 0; i < plan.length(); i++) {
      std::cout << " " << i << " : "
		<< instance.actions[plan[i]].name
		<< std::endl;
    }
    std::cout << ")" << std::endl;
  }
  else {
    std::cout << "no plan found" << std::endl;
  }

  return 0;
}
