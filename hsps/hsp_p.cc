
#include "problem.h"
#include "preprocess.h"
#include "enumerators.h"
#include "parser.h"
#include "cost_table.h"
#include "seq_reg.h"
#include "plans.h"
#include "ida.h"
#include "bfs.h"
#include "bfhs.h"
#include "soft.h"

BEGIN_HSPS_NAMESPACE

int main(int argc, char *argv[]) {
  StringTable symbols(50, lowercase_map);
  bool        opt_H1 = false;
  bool        opt_AH = false;
  bool        opt_load_partition = false;
  bool        opt_find_invariants = false;
  bool        opt_apply_invariants = false;
  bool        opt_resource = false;
  bool        opt_compose_resources = false;
  index_type  composite_resource_size = 2;
  bool        opt_R2 = false;
  bool        opt_apply_cuts = true;
  bool        opt_preprocess = true;
  bool        opt_rm_irrelevant = false;
  bool        opt_print_plan = true;
  bool        opt_pddl = false;
  bool        opt_ipc = false;
  bool        opt_cc = false;
  bool        opt_tt = false;
  bool        opt_bfs = false;
  bool        opt_bfs_px = false;
  NTYPE       px_threshold = 0;
  bool        opt_bfida = false;
  bool        opt_optimal = true;
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
    else if ((strcmp(argv[k],"-print-options-max") == 0) && (k < argc - 1)) {
      MaxValueSearch::print_options_max = atoi(argv[++k]);
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
    else if (strcmp(argv[k],"-rm") == 0) {
      opt_rm_irrelevant = true;
    }
    else if (strcmp(argv[k],"-find") == 0) {
      opt_find_invariants = true;
    }
    else if (strcmp(argv[k],"-no-find") == 0) {
      opt_find_invariants = false;
    }
    else if ((strcmp(argv[k],"-compose") == 0) && (k < argc - 1)) {
      opt_compose_resources = true;
      composite_resource_size = atoi(argv[++k]);
    }

    // heuristic options
    else if (strcmp(argv[k],"-1") == 0) {
      opt_H1 = true;
    }
    else if (strcmp(argv[k],"-AH") == 0) {
      opt_AH = true;
      opt_find_invariants = true;
    }
    else if (strcmp(argv[k],"-load") == 0) {
      opt_load_partition = true;
    }
    else if (strcmp(argv[k],"-R2") == 0) {
      opt_R2 = true;
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
    else if (strcmp(argv[k],"-no-plan") == 0) {
      opt_print_plan = false;
    }
    else if (strcmp(argv[k],"-pddl") == 0) {
      opt_print_plan = true;
      opt_pddl = true;
    }
    else if (strcmp(argv[k],"-ipc") == 0) {
      opt_print_plan = true;
      opt_ipc = true;
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
    else if (strcmp(argv[k],"-o") == 0) {
      opt_optimal = true;
    }
    else if (strcmp(argv[k],"-non-optimal") == 0) {
      opt_optimal = false;
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
  Preprocessor::default_trace_level = verbose_level - 1;
  if (verbose_level < 1) opt_print_plan = false;
  if (verbose_level <= 0) PDDL_Base::write_warnings = false;
  if (verbose_level > 1) PDDL_Base::write_info = true;

  SoftInstance instance;
  Statistics  stats;
  Store       result(instance);

  stats.enable_interrupt(false);
  if (time_limit > 0) stats.enable_time_out(time_limit, false);
  if (memory_limit > 0) stats.enable_memory_limit(memory_limit, false);

  stats.start();
  std::cerr << "instantiating..." << std::endl;
  reader->instantiate(instance);
  reader->instantiate_soft(instance);

  if (opt_resource && opt_compose_resources && (instance.n_resources() > 1)) {
    mSubsetEnumerator crs(instance.n_resources(), composite_resource_size);
    bool more = crs.first();
    while (more) {
      index_set s;
      crs.current_set(s);
      instance.create_composite_resource(s);
      more = crs.next();
    }
  }

  Preprocessor prep(instance, stats);
  if (opt_preprocess) {
    std::cerr << "preprocessing..." << std::endl;
    prep.preprocess();
    if (opt_rm_irrelevant) {
      prep.compute_irrelevant_atoms();
      prep.remove_irrelevant_atoms();
    }
    instance.remap_hard_goals(prep.atom_map);
    instance.remap_soft_goals(prep.atom_map);
  }
  else {
    std::cerr << "cross referencing..." << std::endl;
    instance.cross_reference();
  }
  if (opt_find_invariants) {
    prep.bfs_find_invariants();
  }
  instance.verify_invariants();
  stats.stop();

  std::cerr << "instance " << instance.name << " built in "
	    << stats.time() << " seconds" << std::endl;
  std::cerr << instance.n_atoms() << " atoms, "
	    << instance.n_hard() << " hard goals, "
	    << instance.n_soft() << " soft goals, "
	    << instance.n_resources() << " resources ("
	    << instance.n_reusable_resources() << " reusable, "
	    << instance.n_consumable_resources() << " consumable), "
	    << instance.n_actions() << " actions, "
	    << instance.n_invariants() << " invariants"
	    << std::endl;

  Heuristic* h = 0;
  CostACF f(instance);

  stats.start();
  if (opt_AH) {
    std::cerr << "computing AH heuristic..." << std::endl;
    AH* ah = new AH(instance, stats);
    if (opt_load_partition) {
      stats.start();
      name_vec pnames(0, 0);
      index_set_vec partition;
      reader->export_action_partitions(pnames, partition);
      instance.remap_sets(partition, prep.action_map);
      if (opt_H1) {
	ah->compute_additive_H1(CostACF(instance), partition);
      }
      else {
	ah->compute_additive_H2(CostACF(instance), partition);
      }
      stats.stop();
    }
    else {
      ah->compute_with_iterative_assignment(CostACF(instance),
					    instance.goal_atoms,
					    !opt_H1,
					    false,
					    opt_optimal);
      ah->disable_max_H();
    }
    h = ah;
  }
  else if (opt_H1) {
    CostTable* h1 = new CostTable(instance, stats);
    h1->compute_H1(f);
    h = h1;
  }
  else {
    CostTable* h2 = new CostTable(instance, stats);
    h2->compute_H2(f);
    h = h2;
  }
  stats.stop();
  if (!stats.break_signal_raised()) {
    std::cerr << "heuristic computed in " << stats.time()
	      << " seconds" << std::endl;
  }

  MaxNetBenefit mnb(instance, f, stats, result, *h);
  mnb.init();
  index_type init_n = mnb.n_options();
  NTYPE init_best = mnb.best_option_estimated_value();
  std::cerr << "initial best value estimate: " << init_best << std::endl;
  std::cerr << "searching..." << std::endl;
  NTYPE current_best = mnb.main();

  if (opt_ipc) {
    std::cout << "; Time " << Stopwatch::seconds() << std::endl;
    std::cout << "; ParsingTime " << parse_stats.total_time() << std::endl;
    if (mnb.solved()) {
      std::cout << "; MetricValue " << PRINT_NTYPE(current_best)
		<< std::endl;
      PrintIPC print_plan(instance, std::cout);
      result.output(print_plan);
    }
    else {
      std::cout << "; Not Solved" << std::endl;
    }
  }

  else if (opt_pddl) {
    if (mnb.solved()) {
      PrintPDDL print_plan(instance, std::cout);
      result.output(print_plan);
    }
  }

  else if (verbose_level > 0) {
    if (mnb.solved()) {
      std::cout << "solution value: " << current_best << std::endl;
      if (opt_print_plan) {
	Print print_plan(instance, std::cout);
	result.output(print_plan);
      }
    }
    else {
      std::cout << "no solution found" << std::endl;
    }
    stats.print_total(std::cout);
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
	      << ' ' << (mnb.solved() ? 1 : 0)
	      << ' ' << PRINT_NTYPE(init_best)
	      << ' ' << PRINT_NTYPE(current_best)
	      << ' ' << stats.total_nodes()
	      << ' ' << stats.total_time()
	      << ' ' << stats.peak_memory()
#ifdef RSS_FROM_PSINFO
	      << ' ' << stats.peak_total_size()
#endif
	      << ' ' << stats.time()
	      << ' ' << init_n
	      << ' ' << mnb.n_options()
	      << ' ' << mnb.best_option_size()
	      << ' ' << stats.flags()
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
