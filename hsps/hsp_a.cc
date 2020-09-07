
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
#include "minimax.h"
#include "itest.h"
#include "bb.h"
#include "bfs.h"
#include "pop.h"
#include <fstream>

BEGIN_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  index_type  opt_q_val = 0;
  NTYPE       opt_d_val = 0;
  bool        opt_round = false;
  bool        opt_round_up = false;
  bool        opt_round_down = false;
  bool        opt_H1 = false;
  bool        opt_sequential = false;
  bool        opt_temporal = false;
  bool        opt_resource = false;
  bool        opt_cost = false;
  bool        opt_preprocess = true;
  bool        opt_post_op = false;
  bool        opt_rm_irrelevant = false;
  bool        opt_cc = false;
  bool        opt_tt = false;
  index_type  opt_tt_size = 31337;
  index_type  opt_st_size = 10007;
  bool        opt_bfs = false;
  bool        opt_bfs_px = false;
  NTYPE       px_threshold = 0;
  bool        opt_apply_cuts = true;
  bool        opt_save_h = false;
  bool        opt_load_compare = false;
  bool        opt_compare_max = false;
  index_type  max_apx_limit = no_such_index;
  index_type  min_apx_limit = 0;
  double      time_limit = 0;
  index_type  iteration_limit = 0;
  count_type  node_limit = 0;
  int         verbose_level = 1;
  bool        opt_print_plan = true;
  bool        opt_pop = false;
  bool        opt_pddl = false;
  bool        opt_ipc = false;
  bool        opt_strict_ipc = false;
#ifdef NTYPE_RATIONAL
  NTYPE       epsilon = rational(1,100);
#else
  NTYPE       epsilon = 0.001;
#endif
  bool        opt_no = false;
  count_type  max_k = 0;
  bool        opt_iterate_on_k = false;
  count_type  max_k_limit = 0;
  bool        opt_mm = false;
  bool        opt_ab = false;
  bool        opt_itest = false;
  bool        opt_ldfs = false;
  index_type  mm_depth = 0;
  bool        opt_apx_only = false;
  bool        opt_dynamic_limit = true;
  bool        opt_apx_store = true;

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
    else if ((strcmp(argv[k],"-q") == 0) && (k < argc - 1)) {
      opt_q_val = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-d") == 0) && (k < argc - 1)) {
      opt_d_val = A_TO_N(argv[++k]);
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
    else if (strcmp(argv[k],"-strict-right-shift") == 0) {
      TemporalRegState::strict_right_shift = true;
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
    else if (strcmp(argv[k],"-post") == 0) {
      opt_round_up = true;
      opt_round = false;
      opt_round_down = false;
      opt_post_op = true;
    }
    else if (strcmp(argv[k],"-rm") == 0) {
      opt_rm_irrelevant = true;
    }
    else if (strcmp(argv[k],"-1") == 0) {
      opt_H1 = true;
    }
    else if (strcmp(argv[k],"-tt") == 0) {
      opt_tt = true;
    }
    else if ((strcmp(argv[k],"-tt-size") == 0) && (k < argc - 1)) {
      opt_tt_size = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-st-size") == 0) && (k < argc - 1)) {
      opt_st_size = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-cc") == 0) {
      opt_cc = true;
    }
    else if (strcmp(argv[k],"-bfs") == 0) {
      opt_bfs = true;
      opt_apply_cuts = false;
    }
    else if ((strcmp(argv[k],"-px") == 0) && (k < argc - 1)) {
      opt_bfs_px = true;
      px_threshold = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-cut") == 0) {
      opt_apply_cuts = true;
    }
    else if (strcmp(argv[k],"-no-cut") == 0) {
      opt_apply_cuts = false;
    }
    else if ((strcmp(argv[k],"-t") == 0) && (k < argc - 1)) {
      time_limit = atof(argv[++k]);
    }
    else if ((strcmp(argv[k],"-i") == 0) && (k < argc - 1)) {
      iteration_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-n") == 0) && (k < argc - 1)) {
      node_limit = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-no-plan") == 0) {
      opt_print_plan = false;
    }
    else if (strcmp(argv[k],"-pop") == 0) {
      opt_print_plan = true;
      opt_pop = true;
    }
    else if (strcmp(argv[k],"-pddl") == 0) {
      opt_print_plan = true;
      opt_pddl = true;
    }
    else if (strcmp(argv[k],"-ipc") == 0) {
      opt_print_plan = true;
      opt_ipc = true;
    }
    else if (strcmp(argv[k],"-strict-ipc") == 0) {
      opt_print_plan = true;
      opt_ipc = true;
      opt_strict_ipc = true;
    }
    else if ((strcmp(argv[k],"-epsilon") == 0) && (k < argc - 1)) {
      epsilon = A_TO_N(argv[++k]);
    }
    else if ((strcmp(argv[k],"-m") == 0) && (k < argc - 1)) {
      max_apx_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-l") == 0) && (k < argc - 1)) {
      min_apx_limit = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-f") == 0) {
      opt_dynamic_limit = false;
    }
    else if (strcmp(argv[k],"-no-alpha") == 0) {
      IDAO::alpha_cut = false;
    }
    else if (strcmp(argv[k],"-max-no") == 0) {
      opt_no = true;
    }
    else if ((strcmp(argv[k],"-ik") == 0) && (k < argc - 1)) {
      max_k = 1;
      opt_iterate_on_k = true;
      max_k_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-max-k") == 0) && (k < argc - 1)) {
      max_k = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-no-store") == 0) {
      opt_apx_store = false;
    }
    else if ((strcmp(argv[k],"-mm") == 0) && (k < argc - 1)) {
      opt_mm = true;
      mm_depth = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-ab") == 0) && (k < argc - 1)) {
      opt_ab = true;
      mm_depth = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-itest") == 0) {
      opt_itest = true;
    }
    else if (strcmp(argv[k],"-ldfs") == 0) {
      opt_ldfs = true;
    }
    else if (strcmp(argv[k],"-apx-only") == 0) {
      opt_apx_only = true;
    }
    else if (strcmp(argv[k],"-save") == 0) {
      opt_save_h = true;
    }
    else if (strcmp(argv[k],"-compare") == 0) {
      opt_load_compare = true;
    }
    else if (strcmp(argv[k],"-compare-max") == 0) {
      opt_load_compare = true;
      opt_compare_max = true;
    }
    else if (*argv[k] != '-') reader->read(argv[k], false);
  }

  SearchAlgorithm::default_trace_level = verbose_level;
  Heuristic::default_trace_level = verbose_level - 1;
  Instance::default_trace_level = verbose_level - 1;
  Preprocessor::default_trace_level = verbose_level - 1;
  if (verbose_level < 1) {
    opt_print_plan = false;
    PDDL_Base::write_warnings = false;
  }

  Instance    instance;
  cost_vec    saved_dur;
  Statistics  stats;
  Statistics  a_stats;
  double      solved_tab_hit_ratio = 0;
  Store       result(instance);

  NTYPE      root_est_cost = 0;
  bool       solved = false;
  NTYPE      current_est_cost = 0;
  index_type current_apx_limit = 0;
  bool       apx_converged = false;
  NTYPE      post_apx_est_cost = 0;
  NTYPE      pre_op_cost = 0;

  stats.enable_interrupt(false);
  if (time_limit > 0) stats.enable_time_out(time_limit, false);

  stats.start();
  std::cerr << "instantiating..." << std::endl;
  reader->instantiate(instance);

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

  instance.save_durations(saved_dur);
  if (opt_d_val > 0) instance.discretize_durations(opt_d_val);
  else if (opt_q_val > 0) instance.quantize_durations(opt_q_val);
  else if (opt_round_up) instance.round_durations_up();
  else if (opt_round_down) instance.round_durations_down();
  else if (opt_round) instance.round_durations();

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

      current_est_cost = root_est_cost;
      current_apx_limit = (opt_H1 ? 2 : 3);
      if (min_apx_limit > current_apx_limit) current_apx_limit = min_apx_limit;
      apx_converged = (current_apx_limit > max_apx_limit);

      while (!apx_converged && !stats.break_signal_raised()) {
	std::cerr << "current est. cost: " << current_est_cost << std::endl;
	if (max_k > 0) {
	  std::cerr << "computing recursive (" << current_apx_limit
		    << "," << max_k << ")-approximation..." << std::endl;
	}
	else {
	  std::cerr << "computing recursive " << current_apx_limit
		    << "-approximation..." << std::endl;
	}
	AtomSet::max_set_size_encountered = 0;

	State* apx_root = 0;
	RegressionResourceState* rcs =
	  (opt_resource ? new RegressionResourceState(instance, resource_cons_est) : 0);
	if (opt_temporal) {
	  apx_root = new ApxTemporalRegState(instance, cost_tab,
					     instance.goal_atoms, rcs,
					     current_apx_limit);
	}
	else if (opt_sequential) {
	  ACF* acf =
	    (opt_cost ? (ACF*)new CostACF(instance) : (ACF*)new UnitACF());
	  apx_root = new ApxSeqRegState(instance, cost_tab, *acf,
					instance.goal_atoms, rcs,
					current_apx_limit);
	}
	else {
	  apx_root = new ApxParaRegState(instance, cost_tab,
					 instance.goal_atoms,
					 current_apx_limit);
	}

	NTYPE new_cost = 0;

	if (opt_mm) {
	  stats.start();
	  for (index_type d = 1; d <= mm_depth; d++) {
	    std::cerr << "depth = " << d << ": ";
	    MiniMax mm_search(stats, d);
	    new_cost = mm_search.new_state(*apx_root, POS_INF);
	    std::cerr << "val = " << new_cost << " (" << stats << ")"
		      << std::endl;
	  }
	  stats.stop();
	  std::cerr << "minimax search complete in " << stats.time()
		    << " seconds" << std::endl;
	}

	else if (opt_itest) {
	  ApxResult apx_res;
	  HashTable sol_tab(opt_st_size);
	  ITest apx_search(stats, apx_res, sol_tab);
	  apx_search.set_cycle_check(opt_cc);
	  new_cost = apx_search.start(*apx_root);

	  if (!stats.break_signal_raised()) {
	    if (apx_search.solved()) {
	      std::cerr << current_apx_limit << "-apxroximate solution: "
			<< new_cost << " (max depth = "
			<< apx_res.solution_depth()
			<< ", TUF: " << sol_tab.TUF()
			<< ", HCF: " << sol_tab.HCF()
			<< ", " << stats << ")" << std::endl;
	      if (apx_res.min_solution()) {
		std::cerr << current_apx_limit
			  << "-apxroximate solution is min"
			  << std::endl;
		apx_converged = true;
	      }
	    }
	    else {
	      std::cerr << "no " << current_apx_limit
			<< "-apxroximate solution! (" << stats << ")"
			<< std::endl;
	      exit(0);
	    }
	  }
	  solved_tab_hit_ratio = sol_tab.hit_ratio();
	  a_stats.add(stats);
	}

	else if (opt_ldfs) {
	  ApxResult apx_res;
	  HashTable sol_tab(opt_st_size);
	  LDFS apx_search(stats, apx_res, sol_tab);
	  new_cost = apx_search.start(*apx_root);

	  if (!stats.break_signal_raised()) {
	    if (apx_search.solved()) {
	      std::cerr << current_apx_limit << "-apxroximate solution: "
			<< new_cost << " (max depth = "
			<< apx_res.solution_depth()
			<< ", TUF: " << sol_tab.TUF()
			<< ", HCF: " << sol_tab.HCF()
			<< ", " << stats << ")" << std::endl;
	      if (apx_res.min_solution()) {
		std::cerr << current_apx_limit
			  << "-apxroximate solution is min"
			  << std::endl;
		apx_converged = true;
	      }
	    }
	    else {
	      std::cerr << "no " << current_apx_limit
			<< "-apxroximate solution! (" << stats << ")"
			<< std::endl;
	      exit(0);
	    }
	  }
	  solved_tab_hit_ratio = sol_tab.hit_ratio();
	  a_stats.add(stats);
	}

	else {
	  ApxResult apx_res;
	  HashTable sol_tab(opt_st_size);
	  IDAO* apx_search = 0;
	  if (max_k > 0) {
	    apx_search = new kIDAO(max_k, stats, apx_res, &sol_tab);
	  }
	  else if (opt_no) {
	    apx_search = new kIDAO(stats, apx_res, &sol_tab);
	  }
	  else {
	    apx_search = new IDAO(stats, apx_res, &sol_tab);
	  }
	  apx_search->set_cycle_check(opt_cc);
	  apx_search->set_store_cost(opt_apx_store);
	  if (node_limit > 0) {
	    apx_search->set_node_limit(node_limit);
	  }
	  if (iteration_limit > 0) {
	    apx_search->set_iteration_limit(iteration_limit);
	  }
	  new_cost = apx_search->start(*apx_root);

	  if (!stats.break_signal_raised()) {
	    if (apx_search->solved()) {
	      std::cerr << current_apx_limit << "-apxroximate solution: "
			<< new_cost << " (max depth = "
			<< apx_res.solution_depth()
			<< ", TUF: " << sol_tab.TUF()
			<< ", HCF: " << sol_tab.HCF()
			<< ", " << stats << ")" << std::endl;
	      if (apx_res.min_solution()) {
		std::cerr << current_apx_limit
			  << "-apxroximate solution is min"
			  << std::endl;
		apx_converged = true;
	      }
	    }
	    else if (apx_search->break_signal_raised()) {
	      std::cerr << "node/work limit reached, no "
			<< current_apx_limit
			<< "-apxroximate solution found ("
			<< stats << ")"
			<< std::endl;
	      apx_converged = true;
	    }
	    else if (apx_search->iterations() >= iteration_limit) {
	      std::cerr << "no " << current_apx_limit
			<< "-apxroximate solution in "
			<< apx_search->iterations() << " iterations ("
			<< stats << ")"
			<< std::endl;
	      apx_converged = true;
	    }
	    else {
	      std::cerr << "no " << current_apx_limit
			<< "-apxroximate solution! (" << stats << ")"
			<< std::endl;
	      exit(0);
	    }
	  }
// 	  std::cerr << "TUF: " << sol_tab.TUF()
// 		    << ", HCF: " << sol_tab.HCF()
// 		    << std::endl;
	  delete apx_search;
	  solved_tab_hit_ratio = sol_tab.hit_ratio();
	  a_stats.add(stats);
	}

	if (!stats.break_signal_raised()) {
	  if ((new_cost <= current_est_cost) &&
	      (current_apx_limit >= min_apx_limit)) {
	    std::cerr << "no cost improvement" << std::endl;
	    if (opt_iterate_on_k) {
	      max_k += 1;
	      if (max_k > max_k_limit) {
		apx_converged = true;
	      }
	      else {
		min_apx_limit = current_apx_limit + 1;
		current_apx_limit = (opt_H1 ? 2 : 3) - 1;
	      }
	    }
	    else if (opt_dynamic_limit) apx_converged = true;
	  }
	  if (current_apx_limit >= max_apx_limit) {
	    std::cerr << "approximation limit reached" << std::endl;
	    if (opt_iterate_on_k) {
	      max_k += 1;
	      if (max_k > max_k_limit) {
		apx_converged = true;
	      }
	      else {
		min_apx_limit = current_apx_limit + 1;
		current_apx_limit = (opt_H1 ? 2 : 3);
	      }
	    }
	    else {
	      apx_converged = true;
	    }
	  }
	  if (!apx_converged) current_apx_limit += 1;
	}

	current_est_cost = MAX(current_est_cost, new_cost);
	delete apx_root;
      }

      post_apx_est_cost = current_est_cost;
      if (opt_save_h) {
	std::ofstream h_file("h.pddl");
	cost_tab.write_pddl(h_file, instance);
	h_file.close();
      }

      Heuristic* cost_est = &cost_tab;
      if (opt_load_compare) {
	CostTable* lc = new CostTable(instance, stats);
	std::cerr << "loading heuristic table (" << reader->h_table.length()
		  << " entries)..." << std::endl;
	reader->export_heuristic(instance, prep.atom_map, true, *lc);
	CompareEval* ce = new CompareEval(instance, cost_tab, *lc);
	ce->set_maximal_heuristic_value(opt_compare_max);
	CompareEval::lower = 0;
	CompareEval::higher = 0;
	CompareEval::equal = 0;
	ce->set_trace_level(0);
	cost_est = ce;
      }

      // after apx search
      if (FINITE(current_est_cost) &&
	  !opt_apx_only &&
	  !stats.break_signal_raised()) {

	State* search_root = 0;
	RegressionResourceState* rcs =
	  (opt_resource ? new RegressionResourceState(instance, resource_cons_est) : 0);
	if (opt_temporal) {
	  if (opt_apply_cuts) {
	    search_root = new TemporalRSRegState(instance, *cost_est,
						 instance.goal_atoms, rcs);
	  }
	  else {
	    search_root = new TemporalRegState(instance, *cost_est,
					       instance.goal_atoms, rcs);
	  }
	}
	else if (opt_sequential) {
	  ACF* acf =
	    (opt_cost ? (ACF*)new CostACF(instance) : (ACF*)new UnitACF());
	  if (opt_apply_cuts) {
	    search_root = new SeqCRegState(instance, *cost_est, *acf,
					   instance.goal_atoms, rcs);
	  }
	  else {
	    search_root = new SeqRegState(instance, *cost_est, *acf,
					  instance.goal_atoms, rcs);
	  }
	}
	else {
	  if (opt_apply_cuts) {
	    search_root = new ParaRSRegState(instance, *cost_est,
					     instance.goal_atoms);
	  }
	  else {
	    search_root = new ParaRegState(instance, *cost_est,
					   instance.goal_atoms);
	  }
	}

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
	else if (opt_tt) {
	  std::cerr << "using IDA* with transposition table..." << std::endl;
	  HashTable* tt = new HashTable(opt_tt_size);
	  search = new IDA(stats, result, tt);
	  ((IDA*)search)->set_cycle_check(opt_cc);
	}
	else {
	  std::cerr << "using IDA*..." << std::endl;
	  search = new IDA(stats, result);
	  ((IDA*)search)->set_cycle_check(opt_cc);
	}

	std::cerr << "searching..." << std::endl;
	current_est_cost = search->start(*search_root);
	if (!stats.break_signal_raised()) {
	  std::cerr << "search complete (" << stats << ")" << std::endl;
	}
	solved = search->solved();

	if (solved && opt_temporal && opt_post_op) {
	  std::cerr << "post-optimizing:" << std::endl;
	  pre_op_cost = current_est_cost;

	  // restore durations
	  instance.set_durations(saved_dur);

	  // recompute heuristic
	  std::cerr << "recomputing heuristic..." << std::endl;
	  cost_tab.clear();
	  cost_tab.compute_H2C(MakespanACF(instance), opt_resource);

	  // construct new search root and run DFS-BB
	  if (!stats.break_signal_raised()) {
	    RegressionResourceState* rcs =
	      (opt_resource? new RegressionResourceState(instance, resource_cons_est):0);
	    search_root = new TemporalRSRegState(instance, cost_tab,
						 instance.goal_atoms, rcs);
	    result.set_stop_condition(Result::stop_at_all_optimal);
	    StoreMinCost filter(result);
	    std::cerr << "searching..." << std::endl;
	    DFS_BB op_search(stats, filter);
	    op_search.set_cycle_check(opt_cc);
	    op_search.start(*search_root, current_est_cost);
	    if (!stats.break_signal_raised()) {
	      std::cerr << "final optimal cost: " << op_search.cost()
			<< " (improvement: "
			<< current_est_cost - op_search.cost()
			<< " (abs.) "
			<< ((current_est_cost - op_search.cost())/
			    op_search.cost())
			<< " (rel.), " << stats << ")" << std::endl;
	      current_est_cost = op_search.cost();
	    }
	    else {
	      solved = false;
	    }
	  }
	  else {
	    solved = false;
	  }
	}
	// end post_op

      }
    }
  }

  if (opt_pop) {
    for (index_type p = 0; p < result.n_solutions(); p++) {
      Schedule* plan = result.plan(p);
      SafePOP* pop = new SafePOP(instance, plan->plan_steps());
      pop->enforce_min_durations();
      pop->enforce_max_durations();
      pop->write(std::cout);
    }
  }

  else if (opt_ipc) {
    std::cout << "; Time " << Stopwatch::seconds() << std::endl;
    // std::cout << "; ParsingTime " << parse_stats.total_time() << std::endl;
    if (solved) {
      if (opt_sequential) {
	std::cout << "; NrActions " << current_est_cost << std::endl;
	std::cout << "; MakeSpan " << std::endl;
	std::cout << "; MetricValue " << std::endl;
      }
      else {
	std::cout << "; NrActions " << std::endl;
	std::cout << "; MakeSpan " << PRINT_NTYPE(current_est_cost)
		  << std::endl;
	if (opt_temporal && opt_post_op) {
	  std::cout << "; UpperBound " << pre_op_cost << std::endl;
	}
	std::cout << "; MetricValue " << std::endl;
      }
      PrintIPC print_plan(instance, std::cout);
      if (opt_temporal) print_plan.set_epsilon(epsilon, false, opt_strict_ipc);
      result.output(print_plan);
    }
    else {
      std::cout << "; Not Solved" << std::endl;
      if (opt_temporal && opt_post_op && (pre_op_cost > 0)) {
	std::cout << "; UpperBound " << pre_op_cost << std::endl;
      }
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
      std::cout << "solution cost: " << current_est_cost << std::endl;
      if (opt_print_plan) {
	Print print_plan(instance, std::cout);
	result.output(print_plan);
      }
    }
    else {
      std::cout << "no solution found" << std::endl;
    }
    stats.print_total(std::cout);
    double total_t = Stopwatch::seconds();
    std::cout << total_t << " seconds total (" << total_t - stats.total_time()
	      << " sec. not accounted for)" << std::endl;
    if (opt_load_compare) {
      count_type n =
	CompareEval::lower + CompareEval::higher + CompareEval::equal;
      std::cout << "Alt. H: " << CompareEval::lower << " lower ("
		<< (CompareEval::lower/(double)n)*100 << "%), "
		<< CompareEval::higher << " higher ("
		<< (CompareEval::higher/(double)n)*100 << "%), "
		<< CompareEval::equal << " equal ("
		<< (CompareEval::equal/(double)n)*100 << "%)"
		<< std::endl;
    }
#ifdef SEARCH_EXTRA_STATS
    std::cout << "search space extra stats:"
	      << ' ' << (rminx_size/(double)rminx_count)
	      << ' ' << (rminc_succ_size_ratio/(double)rminc_count)
	      << ' ' << (rminx_succ/(double)rminx_count)
	      << ' ' << (rmaxx_size/(double)rmaxx_count)
	      << ' ' << (rmaxx_succ/(double)rmaxx_count)
	      << ' ' << (trie_count/(double)rminx_count)
	      << ' ' << (trie_count == 0 ? 0 :
			 (tries_applicable/(double)trie_count))
	      << ' ' << (trie_count == 0 ? 0 :
			 (tries_within_bound/(double)trie_count))
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
	      << ' ' << stats.complete_iterations()
	      << ' ' << stats.time()
	      << ' ' << (apx_converged ? 1 : 0)
	      << ' ' << current_apx_limit
	      << ' ' << PRINT_NTYPE(post_apx_est_cost)
	      << ' ' << a_stats.total_min_nodes()
	      << ' ' << a_stats.total_max_nodes()
	      << ' ' << a_stats.total_complete_iterations()
	      << ' ' << solved_tab_hit_ratio
	      << ' ' << a_stats.total_time()
#ifdef EVAL_EXTRA_STATS
	      << ' ' << CostNode::eval_count
	      << ' ' << CostNode::eval_rec_count
#endif
#ifdef SEARCH_EXTRA_STATS
	      << ' ' << (rminx_size/(double)rminx_count)
	      << ' ' << (rminc_succ_size_ratio/(double)rminc_count)
	      << ' ' << (rminx_succ/(double)rminx_count)
	      << ' ' << (rmaxx_size/(double)rmaxx_count)
	      << ' ' << (rmaxx_succ/(double)rmaxx_count)
	      << ' ' << (trie_count/(double)rminx_count)
	      << ' ' << (trie_count == 0 ? 0 :
			 (tries_applicable/(double)trie_count))
	      << ' ' << (trie_count == 0 ? 0 :
			 (tries_within_bound/(double)trie_count))
#endif
	      << std::endl;
  }

  return 0;
}

END_HSPS_NAMESPACE

// this is quite possibly the ugliest hack I've ever perpetrated...

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
