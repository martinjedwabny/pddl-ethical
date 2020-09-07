
#include "problem.h"
#include "preprocess.h"
#include "parser.h"
#include "cost_table.h"
#include "seq_reg.h"
#include "para_reg.h"
#include "temporal.h"
#include "stats.h"
#include "plans.h"
#include "ida.h"
#include <fstream>

BEGIN_HSPS_NAMESPACE

StringTable symbols(50, lowercase_map);

int main(int argc, char *argv[]) {
  bool        opt_sequential = false;
  bool        opt_temporal = false;
  bool        opt_resource = false;
  bool        opt_cost = false;
  bool        opt_preprocess = true;
  bool        opt_rm_irrelevant = false;
  bool        opt_round = false;
  bool        opt_round_up = false;
  bool        opt_round_down = false;
  bool        opt_print_plan = true;
  bool        opt_pddl = false;
  bool        opt_cc = false;
  bool        opt_cd = false;
  bool        opt_cd_ignore = true;
  index_type  cd_limit = no_such_index;
  count_type  boost_wps_limit = count_type_max;
  bool        opt_dynamic_wps = false;
  bool        opt_b_cut = true;
  bool        opt_h1 = false;
  bool        opt_inc = false;
  bool        opt_save_h = false;
  double      time_limit = 0;
  int         verbose_level = 1;

  CostTable::strong_conflict_detection = false;
  CostTable::ultra_weak_conflict_detection = false;
  CostTable::boost_new_entries = false;

  Parser* reader = new Parser(symbols);

  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      verbose_level = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-name-by-file") == 0) {
      HSPS::PDDL_Base::name_instance_by_problem_file = true;
    }
    else if ((strcmp(argv[k],"-t") == 0) && (k < argc - 1)) {
      time_limit = atof(argv[++k]);
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
    else if (strcmp(argv[k],"-cut") == 0) {
      opt_b_cut = true;
    }
    else if (strcmp(argv[k],"-no-cut") == 0) {
      opt_b_cut = false;
    }
    else if (strcmp(argv[k],"-cc") == 0) {
      opt_cc = true;
    }
    else if (strcmp(argv[k],"-1") == 0) {
      opt_h1 = true;
    }
    else if (strcmp(argv[k],"-1+") == 0) {
      opt_h1 = true;
      opt_inc = true;
    }
    else if ((strcmp(argv[k],"-strong") == 0) || (strcmp(argv[k],"-s") == 0)) {
      CostTable::strong_conflict_detection = true;
      CostTable::ultra_weak_conflict_detection = false;
    }
    else if ((strcmp(argv[k],"-ultra") == 0) || (strcmp(argv[k],"-u") == 0)) {
      CostTable::strong_conflict_detection = false;
      CostTable::ultra_weak_conflict_detection = true;
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
    else if (strcmp(argv[k],"-no-ignore") == 0) {
      opt_cd_ignore = false;
    }
    else if ((strcmp(argv[k],"-wps") == 0) && (k < argc - 1)) {
      boost_wps_limit = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-dwps") == 0) {
      opt_dynamic_wps = true;
    }
    else if (strcmp(argv[k],"-bne") == 0) {
      CostTable::boost_new_entries = true;
    }
    else if (strcmp(argv[k],"-pddl") == 0) {
      opt_pddl = true;
    }
    else if (strcmp(argv[k],"-no-plan") == 0) {
      opt_print_plan = false;
    }
    else if (strcmp(argv[k],"-save") == 0) {
      opt_save_h = true;
    }
    else if (*argv[k] != '-')
      reader->read(argv[k], false);
  }

  SearchAlgorithm::default_trace_level = verbose_level;
  Heuristic::default_trace_level = verbose_level - 1;
  Instance::default_trace_level = verbose_level - 1;
  Preprocessor::default_trace_level = verbose_level - 1;
  if (verbose_level <= 0) {
    opt_print_plan = false;
    PDDL_Base::write_warnings = false;
  }

  Instance   instance;
  Statistics main_stats;
  NTYPE      root_est_cost = 0;
  bool       solved = false;
  NTYPE      current_est_cost = 0;

  main_stats.enable_interrupt(false);
  if (time_limit > 0) main_stats.enable_time_out(time_limit, false);
  Statistics s_stats;
  Statistics b_stats;

  main_stats.start();
  std::cerr << "instantiating..." << std::endl;
  reader->instantiate(instance);
  if (verbose_level > 0) {
    std::cout << "instance: " << instance.name << std::endl;
  }
  if (opt_round_up) instance.round_durations_up();
  else if (opt_round_down) instance.round_durations_down();
  else if (opt_round) instance.round_durations();

  Preprocessor prep(instance, main_stats);
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
  main_stats.stop();

  std::cerr << "instance: " << instance.name << " built in "
	    << main_stats.time() << " seconds" << std::endl;
  std::cerr << instance.n_atoms() << " atoms" << std::endl;
  std::cerr << instance.n_resources() << " resources ("
	    << instance.n_reusable_resources() << " reusable, "
	    << instance.n_consumable_resources() << " consumable)"
	    << std::endl;
  std::cerr << instance.n_actions() << " actions" << std::endl;

  if (!main_stats.break_signal_raised()) {
    CostTable cost_tab(instance, main_stats);
    CostNode::eval_set_mark = true;
    estimator_vec resource_cons_est(0, 0);

    std::cerr << "computing heuristic..." << std::endl;
    if (opt_h1) {
      if (opt_inc) cost_tab.compute_H2(ZeroACF());
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

    if (!main_stats.break_signal_raised()) {
      std::cerr << "heuristic computed in " << main_stats.time()
		<< " seconds" << std::endl;
      std::cerr << "estimated goal cost: "
		<< cost_tab.eval(instance.goal_atoms) << std::endl;
    }

#ifdef APPLY_NCW_NOOP_TRICK
    if (opt_temporal && !main_stats.break_signal_raised()) {
      std::cerr << "computing n.c.w. sets..." << std::endl;
      prep.compute_ncw_sets(cost_tab);
    }
#endif

    if (opt_resource && !main_stats.break_signal_raised()) {
      std::cerr << "computing resource estimators..." << std::endl;
      main_stats.start();
      for (index_type k = 0; k < instance.n_resources(); k++) {
	CostTable* rce = new CostTable(instance, main_stats);
	rce->compute_H1(ResourceConsACF(instance, k));
	resource_cons_est[k] = rce;
      }
      main_stats.stop();
    }

    if (!main_stats.break_signal_raised()) { // after H^2 and n.c.w.
      root_est_cost = cost_tab.eval(instance.goal_atoms);

      State* search_root = 0;
      StateFactory* b_root = 0;
      RegressionResourceState* srcs =
	(opt_resource ? new RegressionResourceState(instance, resource_cons_est) : 0);
      RegressionResourceState* brcs =
	(opt_resource ? new RegressionResourceState(instance, resource_cons_est) : 0);
      if (opt_temporal) {
	search_root = new TemporalRSRegState(instance, cost_tab,
					     instance.goal_atoms, srcs);
	if (opt_b_cut)
	  b_root = new TemporalRSRegState(instance, cost_tab, brcs);
	else
	  b_root = new TemporalRegState(instance, cost_tab, brcs);
      }
      else if (opt_sequential) {
	ACF* acf =
	  (opt_cost ? (ACF*)new CostACF(instance) : (ACF*)new UnitACF());
	search_root = new SeqCRegState(instance, cost_tab, *acf,
				       instance.goal_atoms, srcs);
	if (opt_b_cut)
	  b_root = new SeqCRegState(instance, cost_tab, *acf, brcs);
	else
	  b_root = new SeqRegState(instance, cost_tab, *acf, brcs);
      }
      else {
	search_root = new ParaRSRegState(instance, cost_tab,
					 instance.goal_atoms);
	if (opt_b_cut)
	  b_root = new ParaRSRegState(instance, cost_tab);
	else
	  b_root = new ParaRegState(instance, cost_tab);
      }

      Result result;
      if (opt_print_plan) {
	result.set_plan_set(new Print(instance, std::cout));
      }
      IDA search(main_stats, result);
      search.set_cycle_check(opt_cc);
      search.set_iteration_limit(1);

      if (verbose_level > 0) {
	std::cout << "root: " << *search_root << std::endl
		  << "root est. cost: " << root_est_cost << std::endl;
      }

      current_est_cost = root_est_cost;
      cost_tab.clear_marks();

      std::cerr << "searching..." << std::endl;
      current_est_cost = search.start(*search_root);
      s_stats.add(main_stats);
      count_type search_work = search.work();

      while (!search.solved() &&
	     FINITE(current_est_cost) &&
	     !main_stats.break_signal_raised()) {
	std::cerr << "current estimated cost: "
		  << current_est_cost << std::endl;

	CostTable::Entry* list = cost_tab.marked_entries(true);
	if (list) {
	  std::cerr << "boosting (" << list->list_length() << " entries";
	  if (opt_dynamic_wps) {
	    boost_wps_limit = (10*search_work)/list->list_length();
	    std::cerr << ", wps limit = " << boost_wps_limit;
	  }
	  std::cerr << ")..." << std::endl;
	  if (opt_cd) {
	    index_set cd_ignore;
	    if (opt_cd_ignore) {
	      for (index_type i = 0; i < instance.n_atoms(); i++)
		if (!cost_tab.val(i).mark) cd_ignore.insert(i);
	    }
	    list = cost_tab.boost_cd(list, *b_root, cd_limit, cd_ignore,
				     current_est_cost, EMPTYSET,
				     boost_wps_limit, !opt_dynamic_wps);
	  }
	  else {
	    list = cost_tab.boost(list, *b_root, current_est_cost, EMPTYSET,
				  boost_wps_limit, !opt_dynamic_wps);
	  }
	  if (!main_stats.break_signal_raised())
	    std::cerr << "boost completed in " << main_stats.time()
		      << " seconds" << std::endl;
	  b_stats.add(main_stats);
	  if (list) list->delete_list();
	}
	cost_tab.clear_marks();

	if (!main_stats.break_signal_raised()) {
	  std::cerr << "searching..." << std::endl;
	  search.increase_iteration_limit(1);
	  current_est_cost = search.resume(*search_root, current_est_cost);
	  s_stats.add(main_stats);
	  search_work = search.work() - search_work;
	}
      }

      solved = search.solved();

      if (opt_save_h) {
	std::ofstream h_file("h.pddl");
	cost_tab.write_pddl(h_file, instance);
	h_file.close();
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
    std::cout << "search total: " << s_stats << std::endl;
    std::cout << "boosting total: " << b_stats << std::endl;
    std::cout << CostTable::n_entries_boosted << " entries processed, "
	      << CostTable::n_entries_created << " created, "
	      << CostTable::n_entries_solved << " solved, "
	      << CostTable::n_entries_discarded << " discarded" << std::endl;
    std::cout << CostTable::n_boost_searches << " searches, "
	      << (CostTable::n_boost_searches_with_cd/
		  (double)CostTable::n_boost_searches)*100
	      << "% with conflict detection" << std::endl;
    main_stats.print_total(std::cout);
    double total_t = Stopwatch::seconds();
    std::cout << total_t << " seconds total ("
	      << total_t - main_stats.total_time()
	      << " sec. not accounted for)" << std::endl;
  }

  else {
    std::cout << instance.name
	      << ' ' << (solved ? 1 : 0)
	      << ' ' << PRINT_NTYPE(root_est_cost)
	      << ' ' << PRINT_NTYPE(current_est_cost)
	      << ' ' << main_stats.total_nodes()
	      << ' ' << main_stats.total_time()
	      << ' ' << main_stats.peak_memory()
	      << ' ' << s_stats.total_nodes()
	      << ' ' << s_stats.total_time()
	      << ' ' << b_stats.total_nodes()
	      << ' ' << b_stats.total_time()
	      << ' ' << CostTable::n_entries_boosted
	      << ' ' << CostTable::n_entries_created
	      << ' ' << CostTable::n_entries_solved
	      << ' ' << CostTable::n_entries_discarded
	      << ' ' << CostTable::n_boost_searches
	      << ' ' << CostTable::n_boost_searches_with_cd
	      << std::endl;
  }
}

END_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
