
#include "htn.h"
#include "parser.h"
#include "enumerators.h"
#include "plans.h"
#include "ida.h"
#include "bfs.h"
#include "bfhs.h"
#include "pop.h"

BEGIN_HSPS_NAMESPACE

int main(int argc, char *argv[]) {
  StringTable symbols(50, lowercase_map);
  index_type  opt_q_val = 0;
  NTYPE       opt_d_val = 0;
  bool        opt_resource = false;
  bool        opt_cost = false;
  bool        opt_apply_cuts = true;
  bool        opt_preprocess = true;
  bool        opt_find_all = false;
  bool        opt_print_plan = true;
  bool        opt_pop = false;
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

  Statistics parse_stats;
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
    else if (strcmp(argv[k],"-res") == 0) {
      opt_resource = true;
    }
    else if (strcmp(argv[k],"-cost") == 0) {
      opt_cost = true;
    }

    // additional search space options
    else if (strcmp(argv[k],"-cut") == 0) {
      opt_apply_cuts = true;
    }
    else if (strcmp(argv[k],"-no-cut") == 0) {
      opt_apply_cuts = false;
    }

    // preprocessing options
    else if (strcmp(argv[k],"-prep") == 0) {
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-no-prep") == 0) {
      opt_preprocess = false;
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
    else if (strcmp(argv[k],"-all") == 0) {
      opt_find_all = true;
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
    else if (strcmp(argv[k],"-pop") == 0) {
      opt_pop = true;
    }
    else if (strcmp(argv[k],"-no-plan") == 0) {
      opt_print_plan = false;
    }
    else if (strcmp(argv[k],"-pddl") == 0) {
      opt_print_plan = true;
      opt_pddl = true;
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
      parse_stats.start();
      reader->read(argv[k], false);
      parse_stats.stop();
    }
  }

  SearchAlgorithm::default_trace_level = verbose_level;
  Heuristic::default_trace_level = verbose_level - 1;
  Instance::default_trace_level = verbose_level - 1;
  // Preprocessor::default_trace_level = verbose_level - 1;
  if (verbose_level < 1) opt_print_plan = false;
  if (verbose_level <= 0) PDDL_Base::write_warnings = false;
  if (verbose_level > 1) PDDL_Base::write_info = true;

  HTNInstance instance;
  Statistics  stats;
  NTYPE       root_est_cost = 0;
  bool        solved = false;
  NTYPE       solution_cost = 0;
  Store       result(instance);

  stats.enable_interrupt(false);
  if (time_limit > 0) stats.enable_time_out(time_limit, false);
  if (memory_limit > 0) stats.enable_memory_limit(memory_limit, false);

  stats.start();
  std::cerr << "instantiating..." << std::endl;
  reader->instantiate(instance);
  reader->instantiateHTN(instance);
  std::cerr << "cross referencing..." << std::endl;
  instance.cross_reference();
  stats.stop();

  std::cerr << "instance " << instance.name << " built in "
	    << stats.time() << " seconds" << std::endl;
  std::cerr << instance.n_atoms() << " atoms ("
	    << instance.goal_atoms.length() << " goals), "
	    << instance.n_resources() << " resources ("
	    << instance.n_reusable_resources() << " reusable, "
	    << instance.n_consumable_resources() << " consumable), "
	    << instance.n_actions() << " actions, "
	    << instance.n_tasks() << " abstract tasks, "
	    << instance.n_invariants() << " invariants"
	    << std::endl;

  if (!stats.break_signal_raised()) { // after H^2 and n.c.w.
    State* search_root =
      new FwdSeqHTNState(instance, instance.init_atoms, instance.goal_tasks);
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

    if (opt_find_all) {
      result.set_stop_condition(Result::stop_at_all_optimal);
    }
    else {
      result.set_stop_condition(Result::stop_at_first);
    }
    if (cost_limit > 0) search->set_cost_limit(cost_limit);
    if (node_limit > 0) search->set_node_limit(node_limit);

    root_est_cost = search_root->est_cost();
    std::cerr << "searching..." << std::endl;
    solution_cost = search->start(*search_root);

    if (!stats.break_signal_raised()) {
      std::cerr << "search complete (" << stats << ")" << std::endl;
    }
    solved = search->solved();
  }

  if (opt_pop) {
    for (index_type p = 0; p < result.n_solutions(); p++) {
      Schedule* plan = result.plan(p);
      SafePOP* pop = new SafePOP(instance, plan->plan_steps());
      pop->enforce_min_durations();
      pop->enforce_max_durations();
      pop->find_safe_causal_links();
      pop->write(std::cout);
    }
  }

  else if (opt_pddl) {
    if (solved) {
      PrintPDDL print_plan(instance, std::cout);
      result.output(print_plan);
    }
  }

  else if (verbose_level > 0) {
    if (solved) {
      std::cout << "solution cost: " << solution_cost << std::endl;
      if (opt_print_plan) {
	Print print_plan(instance, std::cout);
	result.output(print_plan);
      }
    }
    else {
      std::cout << "no solution found" << std::endl;
    }
    stats.print(std::cout, "(search) ");
    double total_t = Stopwatch::seconds();
    std::cout << total_t << " seconds total (" << total_t - stats.total_time()
	      << " sec. not accounted for)" << std::endl;
    std::cout << "peak memory use: " << stats.peak_memory() << "k"
#ifdef RSS_FROM_PSINFO
	      << ", peak size: " << stats.peak_total_size() << "k"
#endif
	      << std::endl;
  }

  else {
    std::cout << instance.name
	      << ' ' << (solved ? 1 : 0)
	      << ' ' << PRINT_NTYPE(root_est_cost)
	      << ' ' << PRINT_NTYPE(solution_cost)
	      << ' ' << stats.total_nodes()
	      << ' ' << stats.total_time()
	      << ' ' << stats.peak_memory()
#ifdef RSS_FROM_PSINFO
	      << ' ' << stats.peak_total_size()
#endif
	      << ' ' << stats.time()
#ifdef EVAL_EXTRA_STATS
	      << ' ' << CostNode::eval_count
	      << ' ' << CostNode::eval_rec_count
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
