
#include "problem.h"
#include "preprocess.h"
#include "parser.h"
#include "cost_table.h"
#include "seq_reg.h"
#include "para_reg.h"
#include "temporal.h"
#include "plans.h"
#include "idao.h"
#include "ida.h"
#include <fstream>

BEGIN_HSPS_NAMESPACE

#ifdef SEARCH_EXTRA_STATS
count_type b_rmaxx_count = 0;
count_type b_rmaxx_size = 0;
count_type b_rmaxx_succ = 0;
count_type b_rmaxc_count = 0;
count_type b_rminc_count = 0;
count_type b_rminc_size = 0;
count_type b_rminc_succ_size = 0;
double     b_rminc_succ_size_ratio = 0;
count_type b_rminx_count = 0;
count_type b_rminx_size = 0;
count_type b_rminx_succ = 0;
count_type b_trie_count = 0;
count_type b_tries_applicable = 0;
count_type b_tries_within_bound = 0;
#endif

int main(int argc, char *argv[])
{
  bool        opt_round = false;
  bool        opt_round_up = false;
  bool        opt_round_down = false;
  bool        opt_H1 = false;
  bool        opt_sequential = false;
  bool        opt_temporal = false;
  bool        opt_resource = false;
  bool        opt_cost = false;
  bool        opt_preprocess = true;
  bool        opt_rm_irrelevant = false;
  bool        opt_top_level_entries = false;
  bool        opt_cd = false;
  index_type  cd_limit = no_such_index;
  bool        opt_dynamic_limit = true;
  bool        opt_no_cost_limit = false;
  bool        opt_cc = false;
  bool        opt_boost_only = false;
  bool        opt_save_h = false;
  count_type  boost_wps_limit = count_type_max;
  double      time_limit = 0;
  int         verbose_level = 1;
  bool        opt_print_plan = true;
  bool        opt_pddl = false;

  CostTable::strong_conflict_detection = false;
  CostTable::ultra_weak_conflict_detection = false;
  CostTable::boost_new_entries = true;

  StringTable symbols(50, lowercase_map);
  Parser* reader = new Parser(symbols);

  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      verbose_level = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-name-by-file") == 0) {
      HSPS::PDDL_Base::name_instance_by_problem_file = true;
    }
    else if (strcmp(argv[k],"-use-strict-borrow") == 0) {
      PDDL_Base::use_strict_borrow_definition = true;
    }
    else if (strcmp(argv[k],"-use-extended-borrow") == 0) {
      PDDL_Base::use_extended_borrow_definition = true;
    }
    else if (strcmp(argv[k],"-no-compile") == 0) {
      PDDL_Base::compile_away_disjunctive_preconditions = false;
      PDDL_Base::compile_away_conditional_effects = false;
    }
    else if (strcmp(argv[k],"-no-compact") == 0) {
      PDDL_Base::compact_resource_effects = false;
    }
    else if (strcmp(argv[k],"-round") == 0) {
      opt_round = true;
      opt_round_up = false;
      opt_round_down = false;
    }
    else if (strcmp(argv[k],"-round-up") == 0) {
      opt_round_up = true;
      opt_round = false;
      opt_round_down = false;
    }
    else if (strcmp(argv[k],"-round-down") == 0) {
      opt_round_down = true;
      opt_round_up = false;
      opt_round = false;
    }
    else if (strcmp(argv[k],"-seq") == 0) {
      opt_sequential = true;
    }
    else if (strcmp(argv[k],"-time") == 0) {
      opt_temporal = true;
    }
    else if (strcmp(argv[k],"-res") == 0) {
      opt_resource = true;
    }
    else if (strcmp(argv[k],"-cost") == 0) {
      opt_cost = true;
    }
    else if (strcmp(argv[k],"-no-prep") == 0) {
      opt_preprocess = false;
    }
    else if (strcmp(argv[k],"-rm") == 0) {
      opt_rm_irrelevant = true;
    }
    else if (strcmp(argv[k],"-1") == 0) {
      opt_H1 = true;
    }
    else if (strcmp(argv[k],"-cc") == 0) {
      opt_cc = true;
    }
    else if ((strcmp(argv[k],"-t") == 0) && (k < argc - 1)) {
      time_limit = atof(argv[++k]);
    }
    else if (strcmp(argv[k],"-no-plan") == 0) {
      opt_print_plan = false;
    }
    else if (strcmp(argv[k],"-pddl") == 0) {
      opt_pddl = true;
    }
    else if (strcmp(argv[k],"-boost-only") == 0) {
      opt_boost_only = false;
    }
    else if (strcmp(argv[k],"-save") == 0) {
      opt_save_h = true;
    }
    else if (strcmp(argv[k],"-top") == 0) {
      opt_top_level_entries = true;
    }
    else if ((strcmp(argv[k],"-cd") == 0) && (k < argc - 1)) {
      opt_cd = true;
      if (strcmp(argv[k + 1], "any") == 0) {
	cd_limit = no_such_index;
	k += 1;
      }
      else {
	cd_limit = atoi(argv[++k]);
      }
    }
    else if (strcmp(argv[k],"-no-cd") == 0) {
      opt_cd = false;
    }
    else if (strcmp(argv[k],"-f") == 0) {
      opt_dynamic_limit = false;
    }
    else if (strcmp(argv[k],"-u") == 0) {
      opt_no_cost_limit = true;
    }
    else if ((strcmp(argv[k],"-wps") == 0) && (k < argc - 1)) {
      boost_wps_limit = atoi(argv[++k]);
    }
    else if (*argv[k] != '-') reader->read(argv[k], false);
  }

  SearchAlgorithm::default_trace_level = verbose_level;
  Heuristic::default_trace_level = verbose_level - 1;
  Instance::default_trace_level = verbose_level - 1;
  Preprocessor::default_trace_level = verbose_level - 1;
  if (verbose_level < 1) opt_print_plan = false;

  Instance    instance;
  Statistics  stats;
  Statistics  b_stats;

  NTYPE      root_est_cost = 0;
  NTYPE      current_est_cost = 0;
  bool       solved = false;
  NTYPE      post_boost_est_cost = 0;
  bool       boost_complete = false;

  stats.enable_interrupt(false);
  if (time_limit > 0) stats.enable_time_out(time_limit, false);

  stats.start();
  std::cerr << "instantiating..." << std::endl;
  reader->instantiate(instance);
  if (verbose_level > 0) {
    std::cout << "instance: " << instance.name << std::endl;
  }
  if (opt_round_up) instance.round_durations_up();
  else if (opt_round_down) instance.round_durations_down();
  else if (opt_round) instance.round_durations();

  Preprocessor prep(instance, stats);
  if (opt_preprocess) {
    std::cerr << "preprocessing..." << std::endl;
    prep.preprocess();
    if (opt_rm_irrelevant) {
      prep.compute_irrelevant_atoms();
      prep.remove_irrelevant_atoms();
    }
  }
  else {
    std::cerr << "cross referencing..." << std::endl;
    instance.cross_reference();
  }
  stats.stop();

  std::cerr << "instance " << instance.name << " built in "
	    << stats.time() << " seconds" << std::endl;
  std::cerr << instance.n_atoms() << " atoms, "
	    << instance.n_resources() << " resources ("
	    << instance.n_reusable_resources() << " reusable, "
	    << instance.n_consumable_resources() << " consumable), "
	    << instance.n_actions() << " actions" << std::endl;

  if (!stats.break_signal_raised()) {
    CostTable cost_tab(instance, stats);
    estimator_vec resource_cons_est(0, 0);

    std::cerr << "computing heuristic..." << std::endl;
    if (opt_H1) {
      if (opt_temporal)
	cost_tab.compute_H1(MakespanACF(instance));
      else if (opt_sequential && opt_cost)
	cost_tab.compute_H1(CostACF(instance));
      else
	cost_tab.compute_H1(UnitACF());
    }
    else if (opt_temporal) {
      cost_tab.compute_H2C(MakespanACF(instance), opt_resource);
    }
    else if (opt_sequential) {
      if (opt_cost)
	cost_tab.compute_H2(CostACF(instance));
      else
	cost_tab.compute_H2(UnitACF());
    }
    else {
      cost_tab.compute_H2C(UnitACF(), false);
    }
    if (!stats.break_signal_raised()) {
      std::cerr << "heuristic computed in " << stats.time()
		<< " seconds" << std::endl;
    }

#ifdef APPLY_NCW_NOOP_TRICK
    if (opt_temporal && !stats.break_signal_raised()) {
      std::cerr << "computing n.c.w. sets..." << std::endl;
      prep.compute_ncw_sets(cost_tab);
    }
#endif

    if (opt_resource && !stats.break_signal_raised()) {
      std::cerr << "computing resource estimators..." << std::endl;
      stats.start();
      for (index_type k = 0; k < instance.n_resources(); k++) {
	CostTable* rce = new CostTable(instance, stats);
	rce->compute_H1(ResourceConsACF(instance, k));
	resource_cons_est[k] = rce;
      }
      stats.stop();
    }

    if (!stats.break_signal_raised()) { // after H^2 and n.c.w.
      root_est_cost = cost_tab.eval(instance.goal_atoms);
      std::cerr << "estimated goal cost: " << root_est_cost << std::endl;

      StateFactory* b_root = 0;
      RegressionResourceState* rcs =
	(opt_resource ? new RegressionResourceState(instance, resource_cons_est) : 0);

      if (opt_temporal) {
	b_root = new TemporalRSRegState(instance, cost_tab, rcs);
      }
      else if (opt_sequential) {
	ACF* acf =
	  (opt_cost ? (ACF*)new CostACF(instance) : (ACF*)new UnitACF());
	b_root = new SeqCRegState(instance, cost_tab, *acf, rcs);
      }
      else {
	b_root = new ParaRSRegState(instance, cost_tab);
      }

      current_est_cost = root_est_cost;

      CostTable::Entry* list = 0;
      if (opt_top_level_entries) {
	list = cost_tab.boostable_entries(instance.goal_atoms);
      }
      else {
	list = cost_tab.boostable_entries();
      }
      if (list) {
	std::cerr << "boosting heuristic (" << list->list_length()
		  << " entries)..." << std::endl;
	if (opt_cd) {
	  if (opt_no_cost_limit) {
	    list = cost_tab.boost_cd(list, *b_root, cd_limit, EMPTYSET,
				     POS_INF, EMPTYSET,
				     boost_wps_limit, false);
	  }
	  else if (opt_dynamic_limit) {
	    list = cost_tab.boost_cd(list, *b_root, cd_limit, EMPTYSET,
				     root_est_cost, instance.goal_atoms,
				     boost_wps_limit, false);
	  }
	  else {
	    list = cost_tab.boost_cd(list, *b_root, cd_limit, EMPTYSET,
				     root_est_cost, EMPTYSET,
				     boost_wps_limit, false);
	  }
	}
	else {
	  if (opt_no_cost_limit) {
	    list = cost_tab.boost(list, *b_root, POS_INF, EMPTYSET,
				  boost_wps_limit, false);
	  }
	  else if (opt_dynamic_limit) {
	    list = cost_tab.boost(list, *b_root, root_est_cost,
				  instance.goal_atoms,
				  boost_wps_limit, false);
	  }
	  else {
	    list = cost_tab.boost(list, *b_root, root_est_cost, EMPTYSET,
				  boost_wps_limit, false);
	  }
	}
	if (list) list->delete_list();
	current_est_cost = cost_tab.eval(instance.goal_atoms);
	post_boost_est_cost = current_est_cost;
	if (!stats.break_signal_raised()) {
	  boost_complete = true;
	  std::cerr << "boost completed in " << stats.time()
		    << " seconds" << std::endl;
	  std::cerr << "current est. cost: " << current_est_cost
		    << std::endl;
	}
	b_stats.add(stats);
      }

      if (!stats.break_signal_raised() && opt_save_h) {
	std::ofstream h_file("h.pddl");
	cost_tab.write_pddl(h_file, instance);
	h_file.close();
      }

      if (!stats.break_signal_raised() && !opt_boost_only) {
#ifdef SEARCH_EXTRA_STATS
	// store stats from boosting searches
	b_rmaxx_count = rmaxx_count;
	b_rmaxx_size = rmaxx_size;
	b_rmaxx_succ = rmaxx_succ;
	b_rmaxc_count = rmaxc_count;
	b_rminc_count = rminc_count;
	b_rminc_size = rminc_size;
	b_rminc_succ_size = rminc_succ_size;
	b_rminc_succ_size_ratio = rminc_succ_size_ratio;
	b_rminx_count = rminx_count;
	b_rminx_size = rminx_size;
	b_rminx_succ = rminx_succ;
	b_trie_count = trie_count;
	b_tries_applicable = tries_applicable;
	b_tries_within_bound = tries_within_bound;
	// reset original counters
	rmaxx_count = 0;
	rmaxx_size = 0;
	rmaxx_succ = 0;
	rmaxc_count = 0;
	rminc_count = 0;
	rminc_size = 0;
	rminc_succ_size = 0;
	rminc_succ_size_ratio = 0;
	rminx_count = 0;
	rminx_size = 0;
	rminx_succ = 0;
	trie_count = 0;
	tries_applicable = 0;
	tries_within_bound = 0;
#endif
	State* search_root = 0;
	RegressionResourceState* rcs =
	  (opt_resource ? new RegressionResourceState(instance, resource_cons_est) : 0);
	if (opt_temporal) {
	  search_root = new TemporalRSRegState(instance, cost_tab,
					       instance.goal_atoms, rcs);
	}
	else if (opt_sequential) {
	  ACF* acf =
	    (opt_cost ? (ACF*)new CostACF(instance) : (ACF*)new UnitACF());
	  search_root = new SeqCRegState(instance, cost_tab, *acf,
					 instance.goal_atoms, rcs);
	}
	else {
	  search_root = new ParaRSRegState(instance, cost_tab,
					   instance.goal_atoms);
	}

	Result result;
	if (opt_print_plan) {
	  result.set_plan_set(new Print(instance, std::cout));
	}
	IDA search(stats, result);
	search.set_cycle_check(opt_cc);

	std::cerr << "searching..." << std::endl;
	current_est_cost = search.start(*search_root);
	solved = search.solved();
      }
    }
  }

  if (verbose_level > 0) {
    if (solved) {
      std::cout << "solution cost: " << current_est_cost << std::endl;
    }
    else {
      std::cout << "no solution found" << std::endl;
    }
    stats.print_total(std::cout);
    b_stats.print_total(std::cout, "(boosting) ");
    stats.print(std::cout, "(search) ");
    double total_t = Stopwatch::seconds();
    std::cout << total_t << " seconds total (" << total_t - stats.total_time()
	      << " sec. not accounted for)" << std::endl;
    std::cout << "peak memory use: " << stats.peak_memory() << "k"
#ifdef RSS_FROM_PSINFO
	      << ", peak size: " << stats.peak_total_size() << "k"
#endif
	      << std::endl;
#ifdef SEARCH_EXTRA_STATS
    std::cout << "search space extra stats (main search):"
	      << ' ' << (rminx_size/(double)rminx_count)
	      << ' ' << (rminc_succ_size_ratio/(double)rminc_count)
	      << ' ' << (rminx_succ/(double)rminx_count)
	      << ' ' << (trie_count/(double)rminx_count)
	      << ' ' << (trie_count == 0 ? 0 :
			 (tries_applicable/(double)trie_count))
	      << ' ' << (trie_count == 0 ? 0 :
			 (tries_within_bound/(double)trie_count))
	      << std::endl;
    std::cout << "search space extra stats (boosting searches):"
	      << ' ' << (b_rminx_size/(double)b_rminx_count)
	      << ' ' << (b_rminc_succ_size_ratio/(double)b_rminc_count)
	      << ' ' << (b_rminx_succ/(double)b_rminx_count)
	      << ' ' << (b_trie_count/(double)b_rminx_count)
	      << ' ' << (b_trie_count == 0 ? 0 :
			 (b_tries_applicable/(double)b_trie_count))
	      << ' ' << (b_trie_count == 0 ? 0 :
			 (b_tries_within_bound/(double)b_trie_count))
	      << std::endl;
#endif
  }

  else {
    std::cout << instance.name
	      << ' ' << (solved ? 1 : 0)
	      << ' ' << PRINT_NTYPE(root_est_cost)
	      << ' ' << PRINT_NTYPE(current_est_cost)
	      << ' ' << stats.total_nodes()
	      << ' ' << stats.total_time()
	      << ' ' << stats.peak_memory()
	      << ' ' << stats.nodes()
	      << ' ' << stats.time()
	      << ' ' << (boost_complete ? 1 : 0)
	      << ' ' << PRINT_NTYPE(post_boost_est_cost)
	      << ' ' << b_stats.total_nodes()
	      << ' ' << b_stats.total_time()
#ifdef SEARCH_EXTRA_STATS
	      << ' ' << (rminx_size/(double)rminx_count)
	      << ' ' << (rminc_succ_size_ratio/(double)rminc_count)
	      << ' ' << (rminx_succ/(double)rminx_count)
	      << ' ' << (trie_count/(double)rminx_count)
	      << ' ' << (trie_count == 0 ? 0 :
			 (tries_applicable/(double)trie_count))
	      << ' ' << (trie_count == 0 ? 0 :
			 (tries_within_bound/(double)trie_count))
#endif
#ifdef SEARCH_EXTRA_STATS
	      << ' ' << (b_rminx_size/(double)b_rminx_count)
	      << ' ' << (b_rminc_succ_size_ratio/(double)b_rminc_count)
	      << ' ' << (b_rminx_succ/(double)b_rminx_count)
	      << ' ' << (b_trie_count/(double)b_rminx_count)
	      << ' ' << (b_trie_count == 0 ? 0 :
			 (b_tries_applicable/(double)b_trie_count))
	      << ' ' << (b_trie_count == 0 ? 0 :
			 (b_tries_within_bound/(double)b_trie_count))
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
