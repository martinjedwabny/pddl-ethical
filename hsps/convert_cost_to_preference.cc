
#include "problem.h"
#include "soft.h"
#include "parser.h"

BEGIN_HSPS_NAMESPACE

StringTable symbols(50, lowercase_map);
Parser* reader = new Parser(symbols);

int main(int argc, char *argv[]) {
  const char* counter_predicate_prefix = "count-";
  const char* counter_object_prefix = "n";
  const char* counter_type_name = "count";
  const char* counter_next_predicate_name = "next-count";
  index_type  counter_max = 4;

  PDDL_Base::write_warnings = false;
  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      Instance::default_trace_level = atoi(argv[++k]);
    }
    else if (*argv[k] != '-') {
      reader->read(argv[k], false);
    }
  }

  Statistics stats;
  stats.enable_interrupt(true);
  SoftInstance instance;

  reader->post_process();

  stats.start();
  reader->instantiate(instance);
  reader->instantiate_soft(instance);
  instance.cross_reference();
  stats.stop();
  std::cerr << "instantiation finished in " << stats.time() << " seconds"
	    << std::endl;

  bool_vec have_cost(false, reader->dom_actions.length());
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].cost > 0) {
      ptr_pair* p = (ptr_pair*)instance.actions[k].src;
      PDDL_Base::ActionSymbol* a = (PDDL_Base::ActionSymbol*)p->first;
      index_type i = reader->dom_actions.first(a);
      if (i == no_such_index) {
	std::cerr << "error: operator corresponding to action "
		  << instance.actions[k].name << " not found in domain"
		  << std::endl;
	exit(255);
      }
      have_cost[i] = true;
    }

  std::cout << "(define (problem " << reader->problem_name << ")" << std::endl;
  std::cout << " (:domain " << reader->domain_name << ")" << std::endl;

  std::cout << " (:objects";
  for (index_type k = 0; k <= counter_max; k++) {
    std::cout << " " << counter_object_prefix << k;
  }
  std::cout << " - " << counter_type_name << ")" << std::endl;

  std::cout << " (:init" << std::endl;
  for (index_type k = 0; k < counter_max; k++) {
    std::cout << "  (" << counter_next_predicate_name
	      << " " << counter_object_prefix << k
	      << " " << counter_object_prefix << k+1
	      << ")" << std::endl;
  }
  for (index_type k = 0; k < reader->dom_actions.length(); k++)
    if (have_cost[k]) {
      ptr_table::cell_vec ins;
      reader->dom_actions[k]->instances.dump(ins);
      for (index_type i = 0; i < ins.length(); i++) if (ins[i]->val) {
	Instance::Action* act = *((Instance::action_ref*)(ins[i]->val));
	if (act->cost > 0) {
	  ptr_table::key_vec* args = ins[i]->key_sequence();
	  std::cout << "  (" << counter_predicate_prefix
		    << reader->dom_actions[k]->print_name;
	  for (index_type p = 1; p < args->length(); p++) {
	    std::cout << " " << ((PDDL_Base::Symbol*)((*args)[p]))->print_name;
	  }
	  std::cout << " " << counter_object_prefix << "0)" << std::endl;
	  delete args;
	}
      }
    }
  std::cout << ")" << std::endl;

  std::cout << " (:goal (and" << std::endl;
  for (index_type k = 0; k < reader->dom_actions.length(); k++) {
    ptr_table::cell_vec ins;
    reader->dom_actions[k]->instances.dump(ins);
    for (index_type i = 0; i < ins.length(); i++) if (ins[i]->val) {
      Instance::Action* act = *((Instance::action_ref*)(ins[i]->val));
      if (act->cost > 0) {
	ptr_table::key_vec* args = ins[i]->key_sequence();
	for (index_type c = 1; c <= counter_max; c++) {
	  std::cout << "  (preference a" << act->index << "n" << c
		    << " (not (" << counter_predicate_prefix
		    << reader->dom_actions[k]->print_name;
	  for (index_type p = 1; p < args->length(); p++) {
	    std::cout << " " << ((PDDL_Base::Symbol*)((*args)[p]))->print_name;
	  }
	  std::cout << " " << counter_object_prefix << c
		    << ")))" << std::endl;
	}
	delete args;
      }
    }
  }
  std::cout << "))" << std::endl;

  if (instance.n_soft() > 0) {
    std::cout << " (:metric maximize (- ";
    for (index_type k = 0; k < instance.n_soft() - 1; k++) {
      std::cout << "(+ (* " << PRINT_NTYPE(instance.soft[k].weight)
		<< " (- 1 (is-violated " << instance.soft[k].name << "))) ";
    }
    std::cout << "(* "
	      << PRINT_NTYPE(instance.soft[instance.n_soft() - 1].weight)
	      << " (- 1 (is-violated "
	      << instance.soft[instance.n_soft() - 1].name
	      << ")))";
    for (index_type k = 0; k < instance.n_soft() - 1; k++) {
      std::cout << ")";
    }
  }
  else {
    std::cout << " (:metric minimize";
  }

  index_type n = 0;
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].cost > 0) {
      std::cout << " (+ ";
      for (index_type c = 1; c < counter_max; c++) {
	std::cout << " (+ "
		  << "(* " << PRINT_NTYPE(instance.actions[k].cost * c)
		  << " (is-violated a" << k << "n" << c << "))";
      }
      std::cout << "(* " << PRINT_NTYPE(instance.actions[k].cost * counter_max)
		<< " (is-violated a" << k << "n" << counter_max << "))";
      for (index_type c = 1; c < counter_max; c++)
	std::cout << ")";
      n += 1;
    }
  std::cout << " 0";
  while (n > 0) {
    std::cout << ")";
    n--;
  }

  if (instance.n_soft() > 0) {
    std::cout << "))" << std::endl;
  }
  else {
    std::cout << ")" << std::endl;
  }

  // end problem definition
  std::cout << ")" << std::endl;
}

END_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
