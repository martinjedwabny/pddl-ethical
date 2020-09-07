
#include "problem.h"
#include "reduce.h"
#include "enumerators.h"
#include "parser.h"
#include "cost_table.h"
#include "seq_reg.h"
#include "forward.h"
#include "ida.h"
#include "bfs.h"
#include "bfhs.h"
#include "simplify.h"

BEGIN_HSPS_NAMESPACE

bool        opt_H1 = false;
bool        opt_apply_invariants = false;
bool        opt_forward = false;
bool        opt_apply_cuts = true;
bool        opt_preprocess = true;
bool        opt_reduce = false;
NTYPE       reduce_bound = 3;
bool        opt_simplify = false;
bool        opt_pddl = false;
bool        opt_cc = false;
bool        opt_tt = false;
bool        opt_bfs = false;
bool        opt_bfs_px = false;
NTYPE       px_threshold = 0;
bool        opt_bfida = false;
double      time_limit = 0;
long        memory_limit = 0;
count_type  node_limit = 0;
NTYPE       cost_limit = 0;
int         verbose_level = 1;


bool simplify_instance(Instance*& instance, Reduce*& prep, Statistics& stats)
{
  std::cerr << instance->n_atoms() << " atoms and "
	    << instance->n_actions() << " actions before simplification"
	    << std::endl;
  std::cerr << "searching for invariants..." << std::endl;
  prep->bfs_find_invariants();
  instance->add_missing_negation_invariants();
  std::cerr << "verifying invariants..." << std::endl;
  prep->verify_invariants(*(prep->inconsistency()));
  std::cerr << "constructing SAS instance..." << std::endl;
  SASInstance* sas_p = new SASInstance();
  sas_p->name = instance->name;
  sas_p->construct(*instance, true, false, false);

  Simplifier s(sas_p, stats);
  bool any_chg = false;
  bool trivial = false;
  std::cerr << "trying to eliminate strictly determined variables..."
	    << std::endl;
  bool chg = s.eliminate_strictly_determined_variables();
  any_chg = (any_chg || chg);
  if (!chg) {
    std::cerr << "- no change" << std::endl;
  }
  bool done = false;
  while (!done) {
    while (!done) {
      bool chg = s.apply_WSA(2000000, false);
      any_chg = (any_chg || chg);
      if (!chg) {
	std::cerr << "- no change" << std::endl;
	done = true;
      }
      else if (s.result()->goal_state.empty()) {
	std::cerr << "- goal condition simplified to true" << std::endl;
	trivial = true;
	done = true;
      }
    }
    if (!s.result()->goal_state.empty()) {
      std::cerr << "trying to compose interfering variables..."
		<< std::endl;
      bool chg = s.apply_variable_composition();
      any_chg = (any_chg || chg);
      if (!chg) {
	std::cerr << "- no change" << std::endl;
      }
      done = !chg;
    }
    if (done && !s.result()->goal_state.empty()) {
      std::cerr << "trying to compose action sequences..."
		<< std::endl;
      bool chg = s.apply_sequence_composition();
      any_chg = (any_chg || chg);
      if (!chg) {
	std::cerr << "- no change" << std::endl;
      }
      done = !chg;
    }
  }
  sas_p = s.result();

  if (any_chg && !trivial) {
    std::cerr << "converting to STRIPS..." << std::endl;
    Instance* simplified_ins = sas_p->convert_to_STRIPS();
    // if (simplified_ins->n_atoms() < instance->n_atoms()) {
    delete prep;
    delete instance;
    instance = simplified_ins;
    prep = new Reduce(*instance, stats);
    std::cerr << "preprocessing..." << std::endl;
    prep->preprocess();
    prep->compute_irrelevant_atoms();
    prep->remove_irrelevant_atoms();
    // }
    // else {
    // delete simplified_ins;
    // }
  }

  delete sas_p;
  return trivial;
}

int main(int argc, char *argv[]) {
  StringTable symbols(50, lowercase_map);
  Parser* reader = new Parser(symbols);

  for (int k = 1; k < argc; k++) {
    // verbose level
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      verbose_level = atoi(argv[++k]);
    }

    // problem input/tranformation options
    else if (strcmp(argv[k],"-use-strict-borrow") == 0) {
      PDDL_Base::use_strict_borrow_definition = true;
    }
    else if (strcmp(argv[k],"-use-extended-borrow") == 0) {
      PDDL_Base::use_extended_borrow_definition = true;
    }

    // search space (problem type) selection
    else if (strcmp(argv[k],"-f") == 0) {
      opt_forward = true;
    }


    // additional search space options
    else if (strcmp(argv[k],"-cut") == 0) {
      opt_apply_cuts = true;
    }
    else if (strcmp(argv[k],"-no-cut") == 0) {
      opt_apply_cuts = false;
    }
    else if (strcmp(argv[k],"-inv") == 0) {
      opt_apply_invariants = true;
    }

    // preprocessing options
    else if (strcmp(argv[k],"-prep") == 0) {
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-no-prep") == 0) {
      opt_preprocess = false;
    }
    else if ((strcmp(argv[k],"-reduce") == 0) && (k < (argc - 1))) {
      opt_reduce = true;
      reduce_bound = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-simplify") == 0) {
      opt_simplify = true;
    }

    // heuristic options
    else if (strcmp(argv[k],"-1") == 0) {
      opt_H1 = true;
    }

    // search algorithm selection & options
    else if (strcmp(argv[k],"-bfs") == 0) {
      opt_bfs = true;
      opt_apply_cuts = false;
    }
    else if ((strcmp(argv[k],"-px") == 0) && (k < argc - 1)) {
      opt_bfs_px = true;
      px_threshold = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-bfida") == 0) {
      opt_bfida = true;
      opt_apply_cuts = false;
    }
    else if (strcmp(argv[k],"-cc") == 0) {
      opt_cc = true;
    }
    else if (strcmp(argv[k],"-tt") == 0) {
      opt_tt = true;
    }

    // limit-setting options
    else if ((strcmp(argv[k],"-t") == 0) && (k < argc - 1)) {
      time_limit = atof(argv[++k]);
    }
    else if ((strcmp(argv[k],"-y") == 0) && (k < argc - 1)) {
      memory_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-c") == 0) && (k < argc - 1)) {
      cost_limit = A_TO_N(argv[++k]);
    }
    else if ((strcmp(argv[k],"-n") == 0) && (k < argc - 1)) {
      node_limit = atoi(argv[++k]);
    }

    // output/formatting options
    else if (strcmp(argv[k],"-pddl") == 0) {
      opt_pddl = true;
    }
    else if (strcmp(argv[k],"-nsn") == 0) {
      Instance::write_atom_set_with_symbolic_names = false;
      Instance::write_action_set_with_symbolic_names = false;
    }
    else if (strcmp(argv[k],"-print-rational") == 0) {
      Print::decimal_time = false;
    }

    // misc. options
    else if ((strcmp(argv[k],"-exclude") == 0) && (k < argc - 1)) {
      char* tag = argv[++k];
      if (strcmp(tag, "all") == 0) {
	PDDL_Base::exclude_all_dkel_items = true;
      }
      else {
	const StringTable::Cell* c = symbols.find(tag);
	if (c) PDDL_Base::excluded_dkel_tags.insert(c->text);
      }
    }
    else if ((strcmp(argv[k],"-require") == 0) && (k < argc - 1)) {
      const StringTable::Cell* c = symbols.find(argv[++k]);
      if (c) PDDL_Base::required_dkel_tags.insert(c->text);
    }

    // input file
    else if (*argv[k] != '-') {
      reader->read(argv[k], false);
    }
  }

  SearchAlgorithm::default_trace_level = verbose_level;
  Heuristic::default_trace_level = verbose_level - 1;
  Instance::default_trace_level = verbose_level - 1;
  Preprocessor::default_trace_level = verbose_level - 1;
  Simplifier::default_trace_level = verbose_level;
  if (verbose_level <= 0) PDDL_Base::write_warnings = false;
  if (verbose_level > 1) PDDL_Base::write_info = true;

  SASInstance::additional_strictness_check = false;

  Instance*   instance;
  Statistics  stats;
  NTYPE       root_est_cost = 0;
  bool        solved = false;
  NTYPE       solution_cost = 0;
  Result      result;
  mapping     map(0);
  mapping     inv_map(0);

  index_type  n_increments = 0;
  index_type  n_searched = 0;
  bool        original_problem_searched = false;

  stats.enable_interrupt(false);
  if (time_limit > 0) stats.enable_time_out(time_limit, false);
  if (memory_limit > 0) stats.enable_memory_limit(memory_limit, false);

  stats.start();
  std::cerr << "instantiating..." << std::endl;
  instance = new Instance();
  reader->instantiate(*instance);

  Reduce* prep = new Reduce(*instance, stats);
  if (opt_preprocess) {
    std::cerr << "preprocessing..." << std::endl;
    prep->preprocess();
    prep->compute_irrelevant_atoms();
    prep->remove_irrelevant_atoms();
  }
  else {
    std::cerr << "cross referencing..." << std::endl;
    instance->cross_reference();
  }

  if (opt_reduce) {
    std::cerr << "reducing..." << std::endl;
    prep->reduce(reduce_bound);
  }

  if (opt_simplify) {
    solved = simplify_instance(instance, prep, stats);
  }

  stats.stop();

  if (solved) {
    std::cout << "problem solved by simplification" << std::endl;
    exit(0);
  }

  std::cerr << "instance " << instance->name << " built in "
	    << stats.time() << " seconds" << std::endl;
  std::cerr << instance->n_atoms() << " atoms ("
	    << instance->goal_atoms.length() << " goals), "
	    << instance->n_actions() << " actions, "
	    << instance->n_invariants() << " invariants"
	    << std::endl;

  CostTable* original_h = new CostTable(*instance, stats);
  ACF* cost_f = 0;

  stats.start();
  if (opt_H1) {
    cost_f = new UnitACF();
    original_h->compute_H1(*cost_f);
  }
  else {
    cost_f = new UnitACF();
    original_h->compute_H2(*cost_f);
  }
  stats.stop();
  if (!stats.break_signal_raised()) {
    std::cerr << "original problem heuristic computed in "
	      << stats.time() << " seconds" << std::endl;
    root_est_cost = original_h->eval(instance->goal_atoms);
    std::cerr << "estimated goal cost: " << root_est_cost << std::endl;
  }

  NTYPE increment = 0;
  bool  done = stats.break_signal_raised();
  index_type n_relevant_actions_last = 0;

  while (!done) {
    n_increments += 1;
    index_set relevant_actions;

    stats.start();
    for (index_type k = 0; k < instance->goal_atoms.length(); k++) {
      bool_vec rel_acts(false, instance->n_actions());
      NTYPE c = original_h->eval(instance->goal_atoms[k]) + increment;
      prep->strictly_relevant_actions(instance->goal_atoms[k], c,
				      *original_h, *cost_f, rel_acts);
      rel_acts.insert_into(relevant_actions);
    }
    stats.stop();

    index_type n_rel_acts = relevant_actions.length();

    std::cerr << n_rel_acts << " of " << instance->n_actions()
	      << " actions relevant at H + " << increment
	      << std::endl;

    if ((n_rel_acts > n_relevant_actions_last) &&
	!stats.break_signal_raised()) {
      n_relevant_actions_last = n_rel_acts;
      n_searched += 1;
      Instance* sub_p = 0;
      Reduce* sub_prep = 0;
      CostTable* sub_h = 0;
      Heuristic* sub_search_h = 0;

      stats.start();
      if (n_rel_acts < instance->n_actions()) {
	sub_p = new Instance(instance->name);
	sub_p->restricted_copy(*instance, relevant_actions, map);
	Reduce* sub_prep = new Reduce(*sub_p, stats);
	sub_prep->preprocess();
	assert(map.invert(inv_map));

	if (opt_simplify) {
	  solved = simplify_instance(sub_p, sub_prep, stats);

	  if (solved) {
	    std::cout << "problem solved by simplification" << std::endl;
	    exit(0);
	  }
	}

	std::cerr << "sub-problem contains "
		  << sub_p->n_atoms() << " atoms and "
		  << sub_p->n_actions() << " actions"
		  << std::endl;

	if (opt_pddl) {
	  std::cout << ";; sub-problem" << std::endl;
	  sub_p->write_domain(std::cout);
	  sub_p->write_problem(std::cout);
	}
      }
      else {
	sub_p = instance;
	sub_prep = prep;
	original_problem_searched = true;
      }

      if (opt_forward) {
	sub_search_h =
	  new ForwardH1(*sub_p, sub_p->goal_atoms, *cost_f, stats);
      }
      else {
	if (n_rel_acts < instance->n_actions()) {
	  sub_h = new CostTable(*sub_p, stats);
	  if (opt_H1) {
	    sub_h->compute_H1(*cost_f);
	  }
	  else {
	    sub_h->compute_H2(*cost_f);
	  }
	  if (!stats.break_signal_raised()) {
	    std::cerr << "sub-problem and sub-problem heuristic computed in "
		      << stats.time() << " seconds" << std::endl;
	  }

	  sub_search_h = sub_h;
	}
	else {
	  sub_search_h = original_h;
	}
      }

      if (!stats.break_signal_raised()) {
	State* search_root = 0;

	if (opt_forward) {
	  if (opt_apply_cuts) {
	    search_root = new SeqCProgState
	      (*sub_p, *sub_search_h, *cost_f, sub_p->init_atoms, 0);
	  }
	  else {
	    search_root = new SeqProgState
	      (*sub_p, *sub_search_h, *cost_f, sub_p->init_atoms, 0);
	  }
	}
	else {
	  if (opt_apply_cuts) {
	    search_root = new SeqCRegState
	      (*sub_p, *sub_search_h, *cost_f, sub_p->goal_atoms, 0);
	  }
	  else {
	    search_root = new SeqRegState
	      (*sub_p, *sub_search_h, *cost_f, sub_p->goal_atoms, 0);
	  }
	}

	std::cerr << "search root: " << *search_root << std::endl;

	SearchAlgorithm* search = 0;
	if (opt_bfs) {
	  if (opt_bfs_px) {
	    std::cerr << "using partial expansion A*..." << std::endl;
	    search = new BFS_PX(stats, result, px_threshold);
	  }
	  else {
	    std::cerr << "using A*..." << std::endl;
	    search = new BFS(stats, result);
	  }
	}
	else if (opt_bfida) {
	  std::cerr << "using BFIDA..." << std::endl;
	  search = new BFIDA(stats, result, 10007);
	}
	else if (opt_tt) {
	  std::cerr << "using IDA* with transposition table..." << std::endl;
	  HashTable* tt = new HashTable(31337);
	  IDA* i_search = new IDA(stats, result, tt);
	  i_search->set_cycle_check(opt_cc);
	  search = i_search;
	}
	else {
	  std::cerr << "using IDA*..." << std::endl;
	  IDA* i_search = new IDA(stats, result);
	  i_search->set_cycle_check(opt_cc);
	  search = i_search;
	}
	result.set_n_to_find(1);
	if (cost_limit > 0) search->set_cost_limit(cost_limit);
	if (node_limit > 0) search->set_node_limit(node_limit);

	std::cerr << "searching..." << std::endl;
	solution_cost = search->start(*search_root);

	if (!stats.break_signal_raised()) {
	  if (search->solved()) {
	    solved = true;
	    done = true;
	  }

	  std::cerr << "search complete (solved = " << solved << ")"
		    << std::endl;
	  stats.print(std::cerr, "(sub-problem) ");
	  std::cerr << "peak memory use: " << stats.peak_memory() << "k"
#ifdef RSS_FROM_PSINFO
		    << ", peak size: " << stats.peak_total_size() << "k"
#endif
		    << std::endl;
#ifdef SEARCH_EXTRA_STATS
	  std::cerr << "search space extra stats:"
		    << ' ' << (rminx_size/(double)rminx_count)
		    << ' ' << (rminc_succ_size_ratio/(double)rminc_count)
		    << ' ' << (rminx_succ/(double)rminx_count)
		    << ' ' << (trie_count/(double)rminx_count)
		    << ' ' << (tries_applicable/(double)trie_count)
		    << ' ' << (tries_within_bound/(double)trie_count)
		    << std::endl;
	  rminx_size = 0;
	  rminx_count = 0;
	  rminc_succ_size_ratio = 0;
	  rminc_count = 0;
	  rminx_succ = 0;
	  rmaxx_size = 0;
	  rmaxx_count = 0;
	  rmaxx_succ = 0;
	  trie_count = 0;
	  tries_applicable = 0;
	  tries_within_bound = 0;
#endif
	}
	else {
	  done = true;
	}

	if (n_rel_acts < instance->n_actions()) {
	  delete sub_search_h;
	  delete sub_prep;
	  delete sub_p;
	}
      }

      stats.stop();
    }

    std::cerr << "iteration at H + " << increment
	      << " finished in " << stats.time() << " seconds"
	      << std::endl;

    if ((n_rel_acts == instance->n_actions()) ||
	stats.break_signal_raised()) {
      done = true;
    }
    if (!done) {
      increment += instance->min_cost;
    }
  }

  if (verbose_level > 0) {
    if (solved) {
      std::cout << "problem solved" << std::endl;
    }
    else {
      std::cout << "no solution found" << std::endl;
    }
    std::cerr << n_increments << " iterations, "
	      << n_searched << " sub-problems searched, "
	      << PRINT_NTYPE(R_TO_N(n_relevant_actions_last,
				    instance->n_actions())*100)
	      << "% relevant actions in last iteration"
	      << std::endl;
    stats.print_total(std::cout);
    double total_t = Stopwatch::seconds();
    std::cout << total_t << " seconds total (" << total_t - stats.total_time()
	      << " sec. not accounted for)" << std::endl;
    std::cout << "peak memory use: " << stats.peak_memory() << "k"
#ifdef RSS_FROM_PSINFO
	      << ", peak size: " << stats.peak_total_size() << "k"
#endif
	      << std::endl;
#ifdef SEARCH_EXTRA_STATS
    std::cout << "search space extra stats:"
	      << ' ' << (rminx_size/(double)rminx_count)
	      << ' ' << (rminc_succ_size_ratio/(double)rminc_count)
	      << ' ' << (rminx_succ/(double)rminx_count)
	      << ' ' << (trie_count/(double)rminx_count)
	      << ' ' << (tries_applicable/(double)trie_count)
	      << ' ' << (tries_within_bound/(double)trie_count)
	      << std::endl;
#endif
  }

  else {
    std::cout << instance->name
	      << ' ' << (solved ? 1 : 0)
	      << ' ' << PRINT_NTYPE(root_est_cost)
	      << ' ' << PRINT_NTYPE(solution_cost)
	      << ' ' << stats.total_nodes()
	      << ' ' << stats.total_time()
	      << ' ' << stats.peak_memory()
#ifdef RSS_FROM_PSINFO
	      << ' ' << stats.peak_total_size()
#endif
	      << ' ' << n_increments
	      << ' ' << n_searched
	      << ' ' << (original_problem_searched ? 1 : 0)
#ifdef EVAL_EXTRA_STATS
	      << ' ' << CostNode::eval_count
	      << ' ' << CostNode::eval_rec_count
#endif
#ifdef SEARCH_EXTRA_STATS
	      << ' ' << (rminx_size/(double)rminx_count)
	      << ' ' << (rminc_succ_size_ratio/(double)rminc_count)
	      << ' ' << (rminx_succ/(double)rminx_count)
	      << ' ' << (trie_count/(double)rminx_count)
	      << ' ' << (tries_applicable/(double)trie_count)
	      << ' ' << (tries_within_bound/(double)trie_count)
#endif
	      << std::endl;
  }

  return 0;
}

END_HSPS_NAMESPACE


#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
