
#include "problem.h"
#include "preprocess.h"
#include "parser.h"
#include "pdb_construction.h"
#include "forward.h"
#include "temporal.h"
#include "plans.h"
#include "ida.h"
#include "bfs.h"
#include "bfhs.h"
#include "bb.h"
#include "pop.h"
#include "relaxed.h"

BEGIN_HSPS_NAMESPACE

int main(int argc, char *argv[]) {
  StringTable symbols(50, lowercase_map);
  bool        opt_forward_rc = false;
  bool        opt_forward_H1 = false;
  bool        opt_forward_H2 = false;
  bool        opt_forward_sumx = false;
  bool        opt_reverse_H2 = false;
  bool        opt_reverse_AH2 = false;
  bool        opt_compare_h = false;
  bool        opt_H0 = false;
  bool        opt_HAlmost0 = false;
  bool        opt_pdb = true;
  bool        opt_pdb_load = false;
  bool        opt_ipdb = false;
  bool        opt_fast_pdb = false;
  index_type  opt_repeat = 1;
  bool        opt_pdb_incremental = false;
  bool        opt_pdb_bin = false;
  bool        opt_pdb_random_bin = false;
  bool        opt_pdb_random_independent_bin = false;
  bool        opt_pdb_spanning = true;
  index_type  opt_pdb_span_search_limit = no_such_index;
  index_type  opt_pdb_random_bin_swaps = 10000;
  bool        opt_pdb_ip = false;
  bool        opt_pdb_weighted_independent_bin = false;
  bool        opt_pdb_collapse = true;
  index_type  opt_pdb_size = 1000000;
  index_type  opt_total_size = 10000000;
  index_type  opt_pdb_set_size = index_type_max;
  bool        opt_pdb_add = true;
  bool        opt_pdb_ext_add = false;
  bool        opt_pdb_add_all = false;
  rational    ext_add_threshold = 0;
  bool        opt_pdb_inc = true;
  bool        opt_maximal_add = true;
  bool        opt_sas_min = false;
  bool        opt_sas_safe = false;
  bool        opt_sas_select = false;
  bool        opt_apx_clique = false;
  bool        opt_extended = false;
  bool        opt_relaxed = false;
  bool        opt_resource = false;
  bool        opt_res_ipdb = false;
  bool        opt_compose_resources = false;
  index_type  composite_resource_size = 2;
  bool        opt_R2 = false;
  bool        opt_cost = false;
  bool        opt_zero = false;
  bool        opt_apply_cuts = true;
  bool        opt_preprocess = true;
  bool        opt_preprocess_2 = true;
  bool        opt_rm_irrelevant = false;
  bool        opt_find_invariants = true;
  bool        opt_quick_find_invariants = false;
  bool        opt_verify_invariants = true;
  bool        opt_extend_goal = true;
  bool        opt_find_all = false;
  bool        opt_all_different = false;
  bool        opt_exhaustive = false;
  bool        opt_print_plan = true;
  bool        opt_schedule = false;
  bool        opt_post_op = false;
  bool        opt_validate = false;
  bool        opt_pddl = false;
  bool        opt_ipc = false;
  bool        opt_strict_ipc = false;
#ifdef NTYPE_RATIONAL
  NTYPE       epsilon = rational(1,100);
#else
  NTYPE       epsilon = 0.001;
#endif

  index_type  ipdb_param_d_skip = 1;
  index_type  ipdb_param_s_max = index_type_max;
  double      ipdb_param_i_min = 0;
  index_type  ipdb_param_n_trials = 10;
  index_type  ipdb_param_n_samples = 100;

  bool        opt_save = false;
  bool        opt_cc = false;
  bool        opt_tt = false;
  index_type  opt_tt_size = 31337;
  bool        opt_bfs = false;
  bool        opt_bfs_px = false;
  NTYPE       px_threshold = 0;
  bool        opt_bb = false;
  bool        opt_dfs = false;
  bool        opt_bfida = false;
  bool        opt_bfxd = false;
  bool        opt_test = false;
  double      time_limit = 0;
  long        memory_limit = 0;
  index_type  iteration_limit = 0;
  count_type  node_limit = 0;
  NTYPE       cost_limit = 0;
  int         verbose_level = 1;
  bool        opt_bfs_write_graph = false;
  bool        opt_bfs_write_stats = false;

  Statistics parse_stats;
  Parser* reader = new Parser(symbols);

  LC_RNG rng;

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
    else if (strcmp(argv[k],"-no-compile") == 0) {
      PDDL_Base::create_all_atoms = true;
      PDDL_Base::compile_away_disjunctive_preconditions = false;
      PDDL_Base::compile_away_conditional_effects = false;
    }
    else if (strcmp(argv[k],"-no-compact") == 0) {
      PDDL_Base::compact_resource_effects = false;
    }
    else if (strcmp(argv[k],"-test") == 0) {
      opt_test = true;
    }

    // search space (problem type) selection
    else if (strcmp(argv[k],"-x") == 0) {
      opt_extended = true;
    }
    else if (strcmp(argv[k],"-relax") == 0) {
      opt_relaxed = true;
    }
    else if (strcmp(argv[k],"-res") == 0) {
      opt_resource = true;
    }
    else if (strcmp(argv[k],"-res-ipdb") == 0) {
      opt_res_ipdb = true;
    }
    else if ((strcmp(argv[k],"-compose") == 0) && (k < argc - 1)) {
      opt_compose_resources = true;
      composite_resource_size = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-cost") == 0) {
      opt_cost = true;
    }
    else if (strcmp(argv[k],"-zero") == 0) {
      opt_zero = true;
    }
    else if (strcmp(argv[k],"-sua") == 0) {
      SeqProgState::separate_update_actions = true;
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
    else if (strcmp(argv[k],"-prep-1") == 0) {
      opt_preprocess = true;
      opt_preprocess_2 = false;
    }
    else if (strcmp(argv[k],"-no-prep") == 0) {
      opt_preprocess = false;
    }
    else if (strcmp(argv[k],"-extend") == 0) {
      opt_extend_goal = true;
    }
    else if (strcmp(argv[k],"-no-extend") == 0) {
      opt_extend_goal = false;
    }
    else if (strcmp(argv[k],"-rm") == 0) {
      opt_rm_irrelevant = true;
    }
    else if (strcmp(argv[k],"-find") == 0) {
      opt_find_invariants = true;
    }
    else if (strcmp(argv[k],"-quick-find") == 0) {
      opt_quick_find_invariants = true;
    }
    else if (strcmp(argv[k],"-no-find") == 0) {
      opt_find_invariants = false;
      opt_quick_find_invariants = false;
    }
    else if (strcmp(argv[k],"-verify") == 0) {
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-no-verify") == 0) {
      opt_verify_invariants = false;
    }

    // heuristic options
    else if (strcmp(argv[k],"-r") == 0) {
      opt_reverse_H2 = true;
      opt_pdb = false;
      opt_find_invariants = false;
      opt_verify_invariants = false;
    }
    else if (strcmp(argv[k],"-rAH") == 0) {
      opt_reverse_AH2 = true;
      opt_pdb = false;
      opt_find_invariants = false;
      opt_verify_invariants = false;
    }
    else if (strcmp(argv[k],"-f") == 0) {
      opt_forward_rc = true;
      opt_pdb = false;
      opt_find_invariants = false;
      opt_verify_invariants = false;
    }
    else if (strcmp(argv[k],"-f1") == 0) {
      opt_forward_H1 = true;
      opt_pdb = false;
      opt_find_invariants = false;
      opt_verify_invariants = false;
    }
    else if (strcmp(argv[k],"-f2") == 0) {
      opt_forward_H2 = true;
      opt_pdb = false;
      opt_find_invariants = false;
      opt_verify_invariants = false;
    }
    else if (strcmp(argv[k],"-fx") == 0) {
      opt_forward_sumx = true;
      opt_pdb = false;
      opt_find_invariants = false;
      opt_verify_invariants = false;
    }
    else if (strcmp(argv[k],"-0") == 0) {
      opt_H0 = true;
      opt_pdb = false;
      opt_find_invariants = false;
      opt_verify_invariants = false;
    }
    else if (strcmp(argv[k],"-almost-blind") == 0) {
      opt_HAlmost0 = true;
      opt_pdb = false;
      opt_find_invariants = false;
      opt_verify_invariants = false;
    }
    else if (strcmp(argv[k],"-compare") == 0) {
      opt_compare_h = true;
    }
    else if (strcmp(argv[k],"-pdb-load") == 0) {
      opt_pdb_load = true;
      opt_pdb = false; // AARGH! (only to test use of PDBCollection instead
                       // of old PDBHeuristic...)
    }
    else if (strcmp(argv[k],"-pdb-fast") == 0) {
      opt_fast_pdb = true;
    }
    else if (strcmp(argv[k],"-ipdb") == 0) {
      opt_ipdb = true;
      opt_pdb = false; // looks stupid, but it is correct...
      opt_extend_goal = false;
      ipdb_param_d_skip = 1;
      ipdb_param_i_min = 0.01;
      ipdb_param_n_trials = 10;
      ipdb_param_n_samples = 100;
    }
    else if ((strcmp(argv[k],"-ipdb-param") == 0) && (k < argc - 5)) {
      // opt_ipdb = true;
      // opt_pdb = false; // looks stupid, but it is correct...
      // opt_extend_goal = false;
      ipdb_param_d_skip = atoi(argv[++k]);
      ipdb_param_s_max = atoi(argv[++k]);
      ipdb_param_i_min = atof(argv[++k]);
      ipdb_param_n_trials = atoi(argv[++k]);
      ipdb_param_n_samples = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-repeat") == 0) && (k < argc - 1)) {
      opt_repeat = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-pdb-i") == 0) {
      opt_pdb_incremental = true;
      opt_extend_goal = false;
      ipdb_param_d_skip = 1;
      ipdb_param_n_samples = 25;
    }
    else if ((strcmp(argv[k],"-pdb-i-param") == 0) && (k < argc - 2)) {
      opt_pdb_incremental = true;
      opt_extend_goal = false;
      ipdb_param_d_skip = atoi(argv[++k]);
      ipdb_param_n_samples = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-pdb-bin") == 0) {
      opt_pdb_bin = true;
    }
    else if (strcmp(argv[k],"-pdb-rbin") == 0) {
      opt_pdb_random_bin = true;
    }
    else if (strcmp(argv[k],"-pdb-rib") == 0) {
      opt_pdb_random_independent_bin = true;
    }
    else if (strcmp(argv[k],"-pdb-strict-rib") == 0) {
      opt_pdb_random_independent_bin = true;
      opt_pdb_collapse = false;
    }
    else if ((strcmp(argv[k],"-rbin-swaps") == 0) && (k < argc - 1)) {
      opt_pdb_random_bin_swaps = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-pdb-no-span") == 0) {
      opt_pdb_spanning = false;
    }
    else if ((strcmp(argv[k],"-pdb-span-search-limit") == 0) && (k < argc - 1)) {
      opt_pdb_span_search_limit = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-pdb-ip") == 0) {
      opt_pdb_ip = true;
    }
    else if (strcmp(argv[k],"-pdb-windbin") == 0) {
      opt_pdb_weighted_independent_bin = true;
    }
    else if (strcmp(argv[k],"-pdb-strict-windbin") == 0) {
      opt_pdb_weighted_independent_bin = true;
      opt_pdb_collapse = false;
    }
    else if ((strcmp(argv[k],"-pdb-size") == 0) && (k < argc - 1)) {
      opt_pdb_size = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-pdb-total-size") == 0) && (k < argc - 1)) {
      opt_total_size = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-pdb-set-size") == 0) && (k < argc - 1)) {
      opt_pdb_set_size = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-add") == 0) {
      opt_pdb_add = true;
    }
    else if (strcmp(argv[k],"-no-add") == 0) {
      opt_pdb_add = false;
    }
    else if (strcmp(argv[k],"-apx-add") == 0) {
      opt_pdb_add = true;
      opt_maximal_add = false;
    }
    else if (strcmp(argv[k],"-add-all") == 0) {
      opt_pdb_add = true;
      opt_pdb_add_all = true;
    }
    else if ((strcmp(argv[k],"-xadd") == 0) && (k < argc - 1)) {
      opt_pdb_add = true;
      opt_pdb_ext_add = true;
      ext_add_threshold = rational::ator(argv[++k]);
    }
    else if (strcmp(argv[k],"-inc") == 0) {
      opt_pdb_inc = true;
    }
    else if (strcmp(argv[k],"-no-inc") == 0) {
      opt_pdb_inc = false;
    }
    else if (strcmp(argv[k],"-R2") == 0) {
      opt_R2 = true;
    }

    // search algorithm selection & options
    else if (strcmp(argv[k],"-bfs") == 0) {
      opt_bfs = true;
      opt_apply_cuts = false;
    }
    else if (strcmp(argv[k],"-bfida") == 0) {
      opt_bfida = true;
      opt_apply_cuts = false;
    }
    else if (strcmp(argv[k],"-bfxd") == 0) {
      opt_bfxd = true;
      opt_apply_cuts = false;
    }
    else if ((strcmp(argv[k],"-px") == 0) && (k < argc - 1)) {
      opt_bfs_px = true;
      px_threshold = A_TO_N(argv[++k]);
      opt_apply_cuts = false;
    }
    else if (strcmp(argv[k],"-bb") == 0) {
      opt_bb = true;
    }
    else if (strcmp(argv[k],"-dfs") == 0) {
      opt_dfs = true;
    }
    else if (strcmp(argv[k],"-cc") == 0) {
      opt_cc = true;
    }
    else if (strcmp(argv[k],"-tt") == 0) {
      opt_tt = true;
    }
    else if ((strcmp(argv[k],"-tt-size") == 0) && (k < argc - 1)) {
      opt_tt_size = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-all") == 0) {
      opt_find_all = true;
    }
    else if (strcmp(argv[k],"-all-different") == 0) {
      opt_find_all = true;
      opt_all_different = true;
    }
    else if (strcmp(argv[k],"-ex") == 0) {
      opt_find_all = false;
      opt_exhaustive = true;
    }

    // limit-setting options
    else if ((strcmp(argv[k],"-t") == 0) && (k < argc - 1)) {
      time_limit = atof(argv[++k]);
    }
    else if ((strcmp(argv[k],"-y") == 0) && (k < argc - 1)) {
      memory_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-i") == 0) && (k < argc - 1)) {
      iteration_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-c") == 0) && (k < argc - 1)) {
      cost_limit = A_TO_N(argv[++k]);
    }
    else if ((strcmp(argv[k],"-n") == 0) && (k < argc - 1)) {
      node_limit = atoi(argv[++k]);
    }

    // output/formatting options
    else if (strcmp(argv[k],"-val") == 0) {
      opt_validate = true;
    }
    else if (strcmp(argv[k],"-schedule") == 0) {
      opt_schedule = true;
    }
    else if (strcmp(argv[k],"-post") == 0) {
      opt_schedule = true;
      opt_post_op = true;
    }
    else if (strcmp(argv[k],"-no-plan") == 0) {
      opt_print_plan = false;
    }
    else if (strcmp(argv[k],"-pddl") == 0) {
      opt_print_plan = true;
      opt_pddl = true;
      opt_ipc = false;
    }
    else if (strcmp(argv[k],"-ipc") == 0) {
      opt_print_plan = true;
      opt_ipc = true;
      opt_pddl = false;
    }
    else if (strcmp(argv[k],"-strict-ipc") == 0) {
      opt_print_plan = true;
      opt_ipc = true;
      opt_strict_ipc = true;
      opt_pddl = false;
    }
    else if ((strcmp(argv[k],"-epsilon") == 0) && (k < argc - 1)) {
      epsilon = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-nsn") == 0) {
      Instance::write_atom_set_with_symbolic_names = false;
      Instance::write_action_set_with_symbolic_names = false;
    }

    // misc. options
    else if (strcmp(argv[k],"-save") == 0) {
      opt_save = true;
    }
    else if (strcmp(argv[k],"-sas-select") == 0) {
      opt_sas_select = true;
    }
    else if (strcmp(argv[k],"-sas-min") == 0) {
      opt_sas_min = true;
    }
    else if (strcmp(argv[k],"-sas-safe") == 0) {
      opt_sas_safe = true;
    }
    else if (strcmp(argv[k],"-ac") == 0) {
      opt_apx_clique = true;
    }
    else if (((strcmp(argv[k],"-rnd") == 0) ||
	      (strcmp(argv[k],"-r") == 0)) &&
	     (k < argc - 1)) {
      rng.seed(atoi(argv[++k]));
    }
    else if ((strcmp(argv[k],"-rnd-pid") == 0) ||
	     (strcmp(argv[k],"-rp") == 0)) {
      rng.seed_with_pid();
    }
    else if ((strcmp(argv[k],"-rnd-time") == 0) ||
	     (strcmp(argv[k],"-rt") == 0)) {
      rng.seed_with_time();
    }
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
    else if (strcmp(argv[k],"-bfs-write-stats") == 0) {
      opt_bfs_write_stats = true;
    }
    else if (strcmp(argv[k],"-bfs-write-graph") == 0) {
      opt_bfs_write_graph = true;
    }

    // input file
    else if (*argv[k] != '-') {
      parse_stats.start();
      reader->read(argv[k], false);
      parse_stats.stop();
    }
  }

  SearchAlgorithm::default_trace_level = verbose_level;
  Heuristic::default_trace_level = verbose_level;
  Instance::default_trace_level = verbose_level - 1;
  Preprocessor::default_trace_level = verbose_level - 1;
  if (verbose_level < 1) opt_print_plan = false;
  if (verbose_level <= 0) PDDL_Base::write_warnings = false;
  if (verbose_level > 1) PDDL_Base::write_info = true;

  Instance    instance;
  cost_vec    saved_dur;
  cost_vec    saved_res;
  Statistics  stats;
  count_type  pre_search_nodes = 0;
  Preprocessor prep(instance, stats);
  NTYPE       root_est_cost = 0;
  bool        solved = false;
  bool        optimally = false;
  NTYPE       solution_cost = 0;
  Store       store(instance);
  HSPS::HashTable* tt = 0; // so we can access table use stats at end

  stats.enable_interrupt(false);
  if (time_limit > 0) stats.enable_time_out(time_limit, false);
  if (memory_limit > 0) stats.enable_memory_limit(memory_limit, false);

  stats.start();
  std::cerr << "instantiating..." << std::endl;
  reader->instantiate(instance);
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
  if (opt_preprocess) {
    std::cerr << "preprocessing..." << std::endl;
    prep.preprocess(opt_preprocess_2 && !opt_rm_irrelevant);
    if (opt_rm_irrelevant) {
      prep.compute_irrelevant_atoms();
      prep.remove_irrelevant_atoms();
      if (opt_preprocess_2)
	prep.preprocess(true);
    }
    if (!instance.cross_referenced()) {
      std::cerr << "re-cross referencing..." << std::endl;
      instance.cross_reference();
    }
  }
  else {
    std::cerr << "cross referencing..." << std::endl;
    instance.cross_reference();
  }
#ifdef COMMENTED_OUT
  if (opt_schedule) {
    bool_vec n_act(false, instance.n_actions());
    for (index_type k = 0; k < instance.n_actions(); k++)
      for (index_type r = 0; r < instance.n_reusable(); r++)
	if (instance.actions[k].use[r] > instance.reusables[r].init)
	  n_act[k] = true;
    if (n_act.count(true) > 0) {
      index_vec n_map;
      std::cerr << "removing " << n_act.count(true)
		<< " non-executable actions..."
		<< std::endl;
      instance.remove_actions(n_act, n_map);
      instance.clear_cross_reference();
      instance.cross_reference();
    }
  }
#endif
  if (opt_quick_find_invariants) {
    graph* g_inc = prep.inconsistency_graph();
    prep.find_inconsistent_set_invariants(*g_inc);
    instance.add_missing_negation_invariants();
  }
  else if (opt_find_invariants) {
    prep.bfs_find_invariants();
  }
  if (opt_verify_invariants) {
    prep.verify_invariants(*(prep.inconsistency()));
    prep.remove_unverified_invariants();
  }
  index_set original_goal_atoms(instance.goal_atoms);
  if (opt_extend_goal) {
    index_set new_goals;
    prep.implied_atom_set(instance.goal_atoms,new_goals,*prep.inconsistency());
    std::cerr << new_goals.length() << " implied goals found" << std::endl;
    if (new_goals.length() > 0) {
      index_set new_goal(instance.goal_atoms);
      new_goal.insert(new_goals);
      instance.set_goal(new_goal);
    }
  }
  stats.stop();

  std::cerr << "instance " << instance.name << " built in "
	    << stats.time() << " seconds" << std::endl;
  std::cerr << instance.n_atoms() << " atoms ("
	    << instance.goal_atoms.length() << " goals), "
	    << instance.n_resources() << " resources ("
	    << instance.n_reusable_resources() << " reusable, "
	    << instance.n_consumable_resources() << " consumable), "
	    << instance.n_actions() << " actions, "
	    << instance.n_invariants() << " invariants"
	    << std::endl;

  Heuristic* cost_est = 0;
  estimator_vec resource_est(0, 0);

  if (!stats.break_signal_raised()) { // instance construction finished
    stats.start();

    if (opt_H0) {
      cost_est = new ZeroHeuristic(instance);
    }

    else if (opt_HAlmost0) {
      cost_est = new FwdUnitHeuristic(instance);
    }

    else if (opt_forward_rc) {
      ForwardReachabilityCheck* fh =
	new ForwardReachabilityCheck(instance, instance.goal_atoms);
      cost_est = fh;
    }

    else if (opt_forward_H1) {
      ACF* cost = (opt_cost ?
		   (ACF*)new CostACF(instance) :
		   (opt_zero ? (ACF*)new ZeroACF() :
		    (ACF*)new UnitACF()));
      ForwardH1* fh =
	new ForwardH1(instance, instance.goal_atoms, *cost, stats);
      cost_est = fh;
    }

    else if (opt_forward_H2) {
      ACF* cost = (opt_cost ?
		   (ACF*)new CostACF(instance) :
		   (opt_zero ? (ACF*)new ZeroACF() :
		    (ACF*)new UnitACF()));
      ForwardH2* fh =
	new ForwardH2(instance, instance.goal_atoms, *cost, stats);
      cost_est = fh;
    }

    else if (opt_forward_sumx) {
      ACF* cost = (opt_cost ?
		   (ACF*)new CostACF(instance) :
		   (opt_zero ? (ACF*)new ZeroACF() :
		    (ACF*)new UnitACF()));
      SumX* h = new SumX(instance, *cost, stats);
      cost_est = h;
    }

    else if (opt_reverse_H2 || opt_reverse_AH2) {
      std::cerr << "constructing reversed instance..." << std::endl;
      Instance* c_instance = instance.copy();
      c_instance->complete_atom_negations();
      c_instance->cross_reference();
      pair_vec pn;
      for (index_type k = 0; k < c_instance->n_atoms(); k++)
	if (k < c_instance->atoms[k].neg)
	  pn.append(index_pair(k, c_instance->atoms[k].neg));
      assert((pn.length() * 2) == c_instance->n_atoms());
      mapping bm(c_instance->n_atoms());
      for (index_type k = instance.n_atoms(); k < c_instance->n_atoms(); k++)
	bm[k] = no_such_index;
      AtomMapAdapter c_inc(*c_instance, bm, *(prep.inconsistency()));
      for (index_type i = 0; i < pn.length(); i++) {
	if (INFINITE(c_inc.incremental_eval(c_instance->goal_atoms, pn[i].first))) {
	  c_instance->atoms[pn[i].second].goal = true;
	  c_instance->goal_atoms.insert(pn[i].second);
	}
	else if (INFINITE(c_inc.incremental_eval(c_instance->goal_atoms, pn[i].second))) {
	  c_instance->atoms[pn[i].first].goal = true;
	  c_instance->goal_atoms.insert(pn[i].first);
	}
      }
      for (index_type k = 0; k < c_instance->n_actions(); k++) {
	for (HSPS::index_type i = 0; i < c_instance->actions[k].add.length(); i++)
	  if (INFINITE(c_inc.incremental_eval(c_instance->actions[k].pre,
					      c_instance->actions[k].add[i])))
	    c_instance->actions[k].pre.insert(c_instance->atoms[c_instance->actions[k].add[i]].neg);
	for (HSPS::index_type i = 0; i < c_instance->actions[k].del.length(); i++)
	  if (INFINITE(c_inc.incremental_eval(c_instance->actions[k].pre,
					      c_instance->atoms[c_instance->actions[k].del[i]].neg)))
	    c_instance->actions[k].pre.insert(c_instance->actions[k].del[i]);
      }

      Instance* r_instance = new Instance(instance.name);
      r_instance->reverse_copy(*c_instance);
      r_instance->cross_reference();
      std::cerr << "reversed instance: "
		<< r_instance->n_atoms() << " atoms ("
		<< r_instance->goal_atoms.length() << " goals), "
		<< r_instance->n_actions() << " actions, "
		<< std::endl;
      delete c_instance;
      std::cerr << "computing heuristic..." << std::endl;
      ACF* cost = (opt_cost ?
		   (ACF*)new CostACF(*r_instance) :
		   (opt_zero ? (ACF*)new ZeroACF() :
		    (ACF*)new UnitACF()));
      if (opt_reverse_AH2) {
	Preprocessor r_prep(*r_instance, stats);
	r_prep.bfs_find_invariants();
	r_instance->verify_invariants();
	AH* h = new AH(*r_instance, stats);
	h->compute_with_iterative_assignment(*cost, r_instance->goal_atoms,
					     true, false, false);
	CompleteNegationAdapter* rh =
	  new CompleteNegationAdapter(instance, pn, *h);
	cost_est = rh;
      }
      else {
	CostTable* h = new CostTable(*r_instance, stats);
	h->compute_H2(*cost);
	CompleteNegationAdapter* rh =
	  new CompleteNegationAdapter(instance, pn, *h);
	cost_est = rh;
      }
    }

    else if (opt_pdb_load) {
      std::cerr << "constructing SAS+ instance..." << std::endl;
      SASInstance* sas_instance =
	new SASInstance(instance, opt_sas_select, opt_sas_min, opt_sas_safe);
      index_set goal_variables;
      sas_instance->goal_state.defined_set(goal_variables);
      std::cerr << goal_variables.length() << " of "
		<< sas_instance->n_variables()
		<< " variables with goal value"	<< std::endl;
      // std::cerr << "SAS instance:" << std::endl;
      // sas_instance->write_domain(std::cerr);
      ACF* cost = (opt_cost ?
		   (ACF*)new SASCostACF(*sas_instance) :
		   (opt_zero ? (ACF*)new ZeroACF() :
		    (ACF*)new UnitACF()));
      Heuristic* inc = (opt_pdb_inc ? prep.inconsistency() : 0);
      MDDNode* sinc = (opt_pdb_inc ? makeMDD(prep.inconsistency(),
					     sas_instance->atom_map_defined(),
					     sas_instance->atom_map_n()) : 0);
      name_vec vnames(0, 0);
      sas_instance->variable_names(vnames);
      index_set_vec sets;
      reader->export_sets(vnames, sets);
      PDBCollection* h_col =
	new PDBCollection(*sas_instance, *cost, sinc, inc, stats);
      for (index_type k = 0; k < sets.length(); k++)
	h_col->addProgressionPDB(sets[k], opt_fast_pdb);
      std::cerr << h_col->n_patterns() << " PDBs, total size = "
		<< h_col->total_size() << std::endl;
      if (opt_save) {
	h_col->write_collection(std::cout);
	h_col->write_PDB(std::cout);
	exit(0);
      }
      cost_est = new FwdSASHAdapter(instance, *sas_instance, *h_col);
    }

    else if (opt_ipdb) {
      std::cerr << "constructing SAS+ instance..." << std::endl;
      SASInstance* sas_instance =
	new SASInstance(instance, opt_sas_select, opt_sas_min, opt_sas_safe);
      index_set goal_variables;
      sas_instance->goal_state.defined_set(goal_variables);
      std::cerr << stats.time() << " seconds, "
		<< goal_variables.length() << " of "
		<< sas_instance->n_variables()
		<< " variables with goal value"	<< std::endl;
      // std::cerr << "SAS instance:" << std::endl;
      // sas_instance->write_domain(std::cerr);
      ACF* cost = (opt_cost ?
		   (ACF*)new SASCostACF(*sas_instance) :
		   (opt_zero ? (ACF*)new ZeroACF() :
		    (ACF*)new UnitACF()));
      Heuristic* inc = (opt_pdb_inc ? prep.inconsistency() : 0);
      MDDNode* sinc = (opt_pdb_inc ? makeMDD(prep.inconsistency(),
					     sas_instance->atom_map_defined(),
					     sas_instance->atom_map_n()) : 0);
      index_set vars;
      if (opt_pdb_spanning) {
	std::cerr << "searching for independent sets..." << std::endl;
	IndependentVariableSets iv(*sas_instance);
	if (opt_apx_clique) {
	  iv.compute_approximate_independent_sets();
	}
	else {
	  iv.compute_maximal_independent_sets();
	}
	std::cerr << stats.time() << "seconds, "
		  << iv.length() << " independent sets found" << std::endl;
	std::cerr << "searching for spanning sets..."
		  << std::endl;
	iv.compute_spanning_sets(true, opt_pdb_span_search_limit);
	iv.union_set(vars);
	std::cerr << stats.time() << "seconds, "
		  << iv.length() << " spanning sets, "
		  << vars.length() << " of " << sas_instance->n_variables()
		  << " variables span the state space"
		  << std::endl;
      }
      else {
	vars.fill(sas_instance->n_variables());
      }
      UnitACF dummy;
      std::cerr << "variables: ";
      sas_instance->write_variable_set(std::cerr, vars);
      std::cerr << std::endl << "goal: ";
      partial_state sg(sas_instance->goal_state, vars);
      sas_instance->write_partial_state(std::cerr, sg);
      std::cerr << std::endl;
      std::cerr << "searching for patterns..." << std::endl;
      double i_out = -1*D_INF;
      index_type s_out = no_such_index;
      if (opt_repeat > 1) {
	MaxH* h_max = new MaxH();
	for (index_type k = 0; k < opt_repeat; k++) {
	  std::cerr << "collection #" << k + 1 << "..." << std::endl;
	  PDBCollection* h_col =
	    build_collection(instance, dummy, *sas_instance, *cost, sinc, inc,
			     vars, opt_pdb_size, opt_total_size,
			     ipdb_param_d_skip, ipdb_param_s_max,
			     ipdb_param_i_min,
			     ipdb_param_n_trials, ipdb_param_n_samples,
			     opt_fast_pdb, i_out, s_out, rng, stats);
	  h_max->new_component(h_col);
	}
	cost_est = new FwdSASHAdapter(instance, *sas_instance, *h_max);
      }
      else {
	PDBCollection* h_col =
	  build_collection(instance, dummy, *sas_instance, *cost, sinc, inc,
			   vars, opt_pdb_size, opt_total_size,
			   ipdb_param_d_skip, ipdb_param_s_max,
			   ipdb_param_i_min,
			   ipdb_param_n_trials, ipdb_param_n_samples,
			   opt_fast_pdb, i_out, s_out, rng, stats);
	std::cerr << "built PDB collection: " << h_col->n_patterns()
		  << " patterns, total size = " << h_col->total_size()
		  << ", best skipped extension score = " << i_out
		  << ", size of smallest too large PDB = " << s_out
		  << std::endl;
	cost_est = new FwdSASHAdapter(instance, *sas_instance, *h_col);
      }
    }

    if (opt_pdb || opt_compare_h) {
      std::cerr << "constructing SAS+ instance..." << std::endl;
      SASInstance* sas_instance =
	new SASInstance(instance, opt_sas_select, opt_sas_min, opt_sas_safe);
      index_set goal_variables;
      sas_instance->goal_state.defined_set(goal_variables);
      std::cerr << goal_variables.length() << " of "
		<< sas_instance->n_variables()
		<< " variables with goal value"	<< std::endl;

      ProgressionPDBSize set_size_fcn(sas_instance->signature);
      PDBHeuristic* h_pdb = 0;
      ACF* cost = (opt_cost ?
		   (ACF*)new SASCostACF(*sas_instance) :
		   (opt_zero ? (ACF*)new ZeroACF() :
		    (ACF*)new UnitACF()));
      Heuristic* inc = (opt_pdb_inc ? prep.inconsistency() : 0);
      MDDNode* sinc = (opt_pdb_inc ? makeMDD(prep.inconsistency(),
					     sas_instance->atom_map_defined(),
					     sas_instance->atom_map_n()) : 0);
      SASHeuristic* h_sas = 0;

      if (opt_pdb_incremental) {
	index_set vars;
	if (opt_pdb_spanning) {
	  std::cerr << "searching for independent sets..." << std::endl;
	  IndependentVariableSets iv(*sas_instance);
	  if (opt_apx_clique) {
	    iv.compute_approximate_independent_sets();
	  }
	  else {
	    iv.compute_maximal_independent_sets();
	  }
	  std::cerr << iv.length() << " independent sets found" << std::endl;
	  std::cerr << "searching for spanning sets..."
		    << std::endl;
	  iv.compute_spanning_sets(true, opt_pdb_span_search_limit);
	  iv.union_set(vars);
	  std::cerr << iv.length() << " spanning sets: "
		    << vars.length() << " of " << sas_instance->n_variables()
		    << " variables span the state space"
		    << std::endl;
	}
	else {
	  vars.fill(sas_instance->n_variables());
	}
	std::cerr << "computing PDB:s..." << std::endl;
	IncrementalProgressionPDB* h_inc =
	  new IncrementalProgressionPDB(*sas_instance, stats);
	h_inc->compute_sets(vars, opt_pdb_size, opt_pdb_set_size, *cost,
			    sinc, inc, ipdb_param_d_skip,
			    ipdb_param_n_samples, rng);
// 	std::cerr << "time: " << stats.time() << std::endl;
// 	std::cerr << StateAbstraction::n_walks << " random walks, "
// 		  << StateAbstraction::n_cut_walks << " cut short"
// 		  << std::endl;
// 	exit(0);
	if (!stats.break_signal_raised())
	  h_inc->compute_progression_PDB(*cost, sinc, inc);
	if ((opt_pdb_ext_add || opt_pdb_add) &&
	    !stats.break_signal_raised())
	  h_inc->compute_additive_groups(opt_maximal_add);
	h_pdb = h_inc;
	if (opt_save && !stats.break_signal_raised()) {
	  h_pdb->write(std::cout);
	  exit(0);
	}
      }

      else {
	if (opt_pdb_load) {
	  name_vec vnames(0, 0);
	  sas_instance->variable_names(vnames);
	  index_set_vec sets;
	  reader->export_sets(vnames, sets);
	  h_pdb = new PDBHeuristic(*sas_instance, stats);
	  h_pdb->assign_sets(sets);
	  if (verbose_level > 1) {
	    std::cerr << "PDB sets: ";
	    h_pdb->write_variable_sets(std::cerr);
	    std::cerr << std::endl;
	  }
	  if (opt_pdb_ext_add || opt_pdb_add)
	    h_pdb->compute_additive_groups(opt_maximal_add);
	}

	else if (opt_pdb_random_bin) {
	  RandomBinPDB* h_bin = new RandomBinPDB(*sas_instance, stats);
	  h_bin->compute_sets(goal_variables, opt_pdb_size, set_size_fcn,
			      rng, opt_pdb_random_bin_swaps);
	  if (verbose_level > 0) {
	    std::cerr << "random bin sets: ";
	    h_bin->write_variable_sets(std::cerr);
	    std::cerr << std::endl;
	  }
	  if (opt_pdb_ext_add || opt_pdb_add)
	    h_bin->compute_additive_groups(opt_maximal_add);
	  h_pdb = h_bin;
	}

	else if (opt_pdb_random_independent_bin) {
	  RandomIndependentBinPDB* h_bin =
	    new RandomIndependentBinPDB(*sas_instance, stats);
	  h_bin->compute_sets(opt_pdb_size, set_size_fcn,
			      opt_pdb_spanning, opt_pdb_collapse,
			      rng, opt_pdb_random_bin_swaps);
	  if (verbose_level > 0) {
	    std::cerr << "random bin sets: ";
	    h_bin->write_variable_sets(std::cerr);
	    std::cerr << std::endl;
	  }
	  if (opt_pdb_ext_add || opt_pdb_add)
	    h_bin->compute_additive_groups(opt_maximal_add);
	  h_pdb = h_bin;
	}

	else if (opt_pdb_weighted_independent_bin) {
	  std::cerr << "searching for independent sets..." << std::endl;
	  IndependentVariableSets iv(*sas_instance, goal_variables);
	  if (opt_apx_clique) {
	    iv.compute_approximate_independent_sets();
	  }
	  else {
	    iv.compute_maximal_independent_sets();
	  }
	  std::cerr << iv.length() << " independent sets found" << std::endl;
	  if (opt_pdb_spanning) {
	    std::cerr << "searching for spanning sets..." << std::endl;
	    iv.compute_spanning_sets(false, opt_pdb_span_search_limit);
	    index_set uss;
	    iv.union_set(uss);
	    std::cerr << iv.length() << " spanning sets: "
		      << uss.length() << " of " << goal_variables.length()
		      << " (goal) variables included"
		      << std::endl;
	  }
	  if (opt_pdb_collapse) {
	    index_set small;
	    bool_vec  tbr(false, iv.length());
	    for (index_type k = 0; k < iv.length(); k++)
	      if (set_size_fcn(iv[k]) <= opt_pdb_size) {
		small.insert(iv[k]);
		tbr[k] = true;
	      }
	    if (tbr.count(true) > 1) {
	      std::cerr << "collapsing " << tbr.count(true)
			<< " small sets..." << std::endl;
	      iv.remove(tbr);
	      iv.append(small);
	    }
	  }
	  GoalStateInterference* sv_gsi =
	    new GoalStateInterference(*sas_instance);
	  InverseCGFraction* sv_icg = new InverseCGFraction(*sas_instance);
	  SetValueSum* sv_sum = new SetValueSum(*sv_gsi, *sv_icg);
	  WeightedBinPDB* h_wib =
	    new WeightedBinPDB(*sas_instance, stats, *sv_sum);
	  h_wib->compute_sets(iv, opt_pdb_size, set_size_fcn);
	  if (verbose_level > 0) {
	    std::cerr << "weighted independent bin sets: ";
	    h_wib->write_variable_sets(std::cerr);
	    std::cerr << std::endl;
	  }
	  if (opt_pdb_ext_add || opt_pdb_add)
	    h_wib->compute_additive_groups(opt_maximal_add);
	  h_pdb = h_wib;
	}

	else {
	  Max1PDB* h_1 = new Max1PDB(*sas_instance, stats);
	  h_1->compute_sets(goal_variables);
	  if (opt_pdb_ext_add || opt_pdb_add)
	    h_1->compute_additive_groups(opt_maximal_add);
	  h_pdb = h_1;
	}

	std::cerr << "computing PDB:s..." << std::endl;
	h_pdb->compute_progression_PDB(*cost, sinc, inc);
	if (opt_save && !stats.break_signal_raised()) {
	  // h_pdb->write(std::cout);
	  for (index_type k = 0; k < h_pdb->n_components(); k++) {
	    std::cout << "PDB #" << k << " ";
	    sas_instance->write_variable_set(std::cout, h_pdb->variable_set(k));
	    std::cout << std::endl;
	    ((AbstractionHeuristic*)h_pdb->component(k))->write(std::cout);
	    std::cout << std::endl;
	    ((AbstractionHeuristic*)h_pdb->component(k))->
	      write_graph(std::cout, sas_instance->goal_state, EMPTYSET, false, true);
	    std::cout << std::endl;
	  }
	  exit(0);
	}
      }

      if (opt_pdb_add && !stats.break_signal_raised()) {
	std::cerr << "creating additive groups..." << std::endl;
	if (verbose_level > 0) {
	  std::cerr << "additive groups: ";
	  h_pdb->write_additive_groups(std::cerr);
	  std::cerr << std::endl;
	}
	MaxAddH* h_ma = h_pdb->make_max_add();
	h_sas = h_ma;
      }
      else {
	h_sas = h_pdb;
      }

      if (opt_compare_h) {
	Heuristic* h_alt = new FwdSASHAdapter(instance, *sas_instance, *h_sas);
	cost_est = new CompareEval(instance, *cost_est, *h_alt);
      }
      else {
	cost_est = new FwdSASHAdapter(instance, *sas_instance, *h_sas);
      }
    }

    if (opt_resource && opt_res_ipdb && !stats.break_signal_raised()) {
      std::cerr << "computing resource estimators..." << std::endl;
      resource_est.assign_value(0, instance.n_resources());
      stats.start();

      std::cerr << "constructing SAS+ instance..." << std::endl;
      SASInstance* sas_instance =
	new SASInstance(instance, opt_sas_select, opt_sas_min, opt_sas_safe);
      index_set goal_variables;
      sas_instance->goal_state.defined_set(goal_variables);
      std::cerr << stats.time() << " seconds, "
		<< goal_variables.length() << " of "
		<< sas_instance->n_variables()
		<< " variables with goal value"	<< std::endl;
      Heuristic* inc = (opt_pdb_inc ? prep.inconsistency() : 0);
      MDDNode* sinc = (opt_pdb_inc ?
		       makeMDD(prep.inconsistency(),
			       sas_instance->atom_map_defined(),
			       sas_instance->atom_map_n()) : 0);
      index_set vars;
      if (opt_pdb_spanning) {
	std::cerr << "searching for independent sets..." << std::endl;
	IndependentVariableSets iv(*sas_instance);
	if (opt_apx_clique) {
	  iv.compute_approximate_independent_sets();
	}
	else {
	  iv.compute_maximal_independent_sets();
	}
	std::cerr << stats.time() << "seconds, "
		  << iv.length() << " independent sets found" << std::endl;
	std::cerr << "searching for spanning sets..."
		  << std::endl;
	iv.compute_spanning_sets(true, opt_pdb_span_search_limit);
	iv.union_set(vars);
	std::cerr << stats.time() << "seconds, "
		  << iv.length() << " spanning sets, "
		  << vars.length() << " of " << sas_instance->n_variables()
		  << " variables span the state space"
		  << std::endl;
      }
      else {
	vars.fill(sas_instance->n_variables());
      }
      std::cerr << "variables: ";
      sas_instance->write_variable_set(std::cerr, vars);
      std::cerr << std::endl << "goal: ";
      partial_state sg(sas_instance->goal_state, vars);
      sas_instance->write_partial_state(std::cerr, sg);
      std::cerr << std::endl;

      for (index_type k = 0; k < instance.n_resources(); k++) {
	std::cerr << "computing PDB collection for resource " << k
		  << ": " << instance.resources[k].name << "..."
		  << std::endl;
	ResourceConsACF* rce = new ResourceConsACF(instance, k);
	std::cerr << "searching for patterns..." << std::endl;
	double i_out = -1*D_INF;
	index_type s_out = no_such_index;
	PDBCollection* h_col =
	  build_collection(instance, *rce, *sas_instance, *rce, sinc, inc,
			   vars, opt_pdb_size, opt_total_size,
			   ipdb_param_d_skip, ipdb_param_s_max,
			   ipdb_param_i_min,
			   ipdb_param_n_trials, ipdb_param_n_samples,
			   opt_fast_pdb, i_out, s_out, rng, stats);
	std::cerr << "built PDB collection: " << h_col->n_patterns()
		  << " patterns, total size = " << h_col->total_size()
		  << ", best skipped extension score = " << i_out
		  << ", size of smallest too large PDB = " << s_out
		  << std::endl;
	resource_est[k] = new FwdSASHAdapter(instance, *sas_instance, *h_col);
      }
      stats.stop();
    }

    stats.stop();
    if (!stats.break_signal_raised()) {
      std::cerr << "heuristic computed in " << stats.time()
		<< " seconds (" << stats << ")" << std::endl;
    }

    pre_search_nodes = stats.total_nodes();

    if (!stats.break_signal_raised()) { // heuristic finished
      State* search_root = 0;

      if (opt_extended) {
	ACF* acf =
	  (opt_cost ? (ACF*)new CostACF(instance) : (ACF*)new UnitACF());
	search_root = new ExtendedSeqProgState(instance, *cost_est, *acf,
					       instance.init_atoms);
      }
      else if (opt_relaxed) {
	ACF* acf =
	  (opt_cost ? (ACF*)new CostACF(instance) : (ACF*)new UnitACF());
	index_set x_atoms;
	for (index_type k = 0; k < instance.n_invariants(); k++)
	  if ((instance.invariants[k].set.length() > 1) &&
	      (instance.invariants[k].lim == 1) &&
	      instance.invariants[k].exact)
	    x_atoms.insert(instance.invariants[k].set);
	search_root = new RelaxedSeqProgState(instance, x_atoms,
					      *cost_est, *acf,
					      instance.init_atoms);
      }
      else {
	ACF* acf =
	  (opt_cost ? (ACF*)new CostACF(instance) : (ACF*)new UnitACF());
	BasicResourceState* rcs =
	  (opt_resource ? new BasicResourceState(instance, resource_est) : 0);
	if (opt_apply_cuts)
	  search_root = new SeqCProgState(instance, *cost_est, *acf,
					  instance.init_atoms, rcs);
	else
	  search_root = new SeqProgState(instance, *cost_est, *acf,
					 instance.init_atoms, rcs);
      }

      std::cerr << "search root: " << *search_root << std::endl;
      std::cerr << "estimated goal cost: "
		<< cost_est->eval(instance.init_atoms)
		<< std::endl;

      SearchAlgorithm* search = 0;
      SearchResult* result = &store;
      if (opt_bb) {
	store.set_stop_condition(Result::stop_at_all_optimal);
	result = new StoreMinCost(store);
      }
      else if (opt_all_different) {
	result = new StoreDistinct(instance, store);
      }

      if (opt_find_all) {
	store.set_stop_condition(Result::stop_at_all_optimal);
      }
      else if (opt_exhaustive) {
	store.set_stop_condition(Result::stop_at_all);
      }
      else {
	store.set_stop_condition(Result::stop_at_first);
      }

      if (opt_bfs) {
	if (opt_bfs_px) {
	  std::cerr << "using partial expansion A*..." << std::endl;
	  search = new BFS_PX(stats, *result, px_threshold);
	}
	else {
	  std::cerr << "using A*..." << std::endl;
	  search = new BFS(stats, *result);
	}
      }
      else if (opt_bfida) {
	std::cerr << "using BFIDA..." << std::endl;
	search = new BFIDA(stats, *result, 10007);
      }
      else if (opt_bfxd) {
	std::cerr << "using BFHS with Exponential Deepening..." << std::endl;
	search = new BFHS_XD(stats, *result, 10007);
      }
      else if (opt_dfs) {
	std::cerr << "exhaustive DFS to " << cost_limit << "..." << std::endl;
	DFS* df_search = new DFS(stats, *result);
	df_search->set_cycle_check(opt_cc);
	df_search->set_upper_bound(cost_limit);
	search = df_search;
      }
      else if (opt_bb) {
	std::cerr << "using DFS branch-and-bound..." << std::endl;
	DFS_BB* bb_search = 0;
	if (opt_tt) {
	  tt = new HashTable(opt_tt_size);
	  bb_search = new DFS_BB(stats, *result, tt);
	}
	else {
	  bb_search = new DFS_BB(stats, *result);
	}
	bb_search->set_cycle_check(opt_cc);
	bb_search->set_upper_bound(cost_limit);
	search = bb_search;
      }
      else if (opt_tt) {
	std::cerr << "using IDA* with transposition table..." << std::endl;
	tt = new HashTable(opt_tt_size);
	IDA* i_search = new IDA(stats, *result, tt);
	i_search->set_cycle_check(opt_cc);
	if (iteration_limit > 0)
	  i_search->set_iteration_limit(iteration_limit);
	search = i_search;
      }
      else {
	std::cerr << "using IDA*..." << std::endl;
	IDA* i_search = new IDA(stats, *result);
	i_search->set_cycle_check(opt_cc);
	if (iteration_limit > 0)
	  i_search->set_iteration_limit(iteration_limit);
	search = i_search;
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
      optimally = search->optimal();
      if (!search->optimal())
	solution_cost = search->cost();

      if (opt_bfs && opt_bfs_write_graph) {
	NodeSet& g = ((BFS*)search)->state_space();
	g.write_graph(std::cout);
      }
      if (opt_bfs && opt_bfs_write_stats) {
	NodeSet& g = ((BFS*)search)->state_space();
	g.write_short(std::cout, instance.name);
      }
    }
  }

  NTYPE min_makespan = POS_INF;

  if (solved && opt_schedule) {
    for (index_type p = 0; p < store.n_solutions(); p++) {
      Schedule* plan = store.plan(p);
      std::cerr << "processing plan " << p << "..." << std::endl;
      plan->write(std::cerr);
      graph s_prec;
      plan->deorder(s_prec);
      index_vec s_acts(plan->step_actions());
      std::cerr << "actions:";
      for (index_type k = 0; k < s_acts.length(); k++)
	std::cerr << " " << k << ":" << instance.actions[s_acts[k]].name;
      std::cerr << std::endl;
      std::cerr << "precedence graph: " << s_prec << std::endl;
      plan->schedule(s_acts, s_prec);
      plan->write(std::cerr);
      min_makespan = MIN(min_makespan, plan->makespan());
      optimally = false;
    }
  }

  if (solved && opt_post_op) {
    std::cerr << "post-optimizing:" << std::endl;

    // compute makespan heuristic
    std::cerr << "computing makespan heuristic..." << std::endl;
    CostTable* cost_tab = new CostTable(instance, stats);
    cost_tab->compute_H2C(MakespanACF(instance), opt_resource);
    std::cerr << "lower bound = " << cost_tab->eval(instance.goal_atoms)
	      << ", upper bound = " << min_makespan
	      << std::endl;

    if (opt_resource && !stats.break_signal_raised()) {
      std::cerr << "computing resource estimators..." << std::endl;
      stats.start();
      for (index_type k = 0; k < instance.n_resources(); k++) {
	CostTable* rce = new CostTable(instance, stats);
	if (opt_R2) {
	  rce->compute_H2(ResourceConsACF(instance, k));
	}
	else {
	  rce->compute_H1(ResourceConsACF(instance, k));
	}
	if (verbose_level > 2) {
	  std::cout << "resource " << instance.resources[k].name
		    << " estimator:" << std::endl;
	  rce->write(std::cout);
	}
	resource_est[k] = rce;
      }
      stats.stop();
    }

    // construct new search root and run DFS-BB
    if (!stats.break_signal_raised()) {
      RegressionResourceState* rcs =
	(opt_resource? new RegressionResourceState(instance, resource_est) : 0);
      TemporalRSRegState* search_root =
	new TemporalRSRegState(instance, *cost_tab, instance.goal_atoms, rcs);
      std::cerr << "searching..." << std::endl;
      store.clear();
      store.set_stop_condition(Result::stop_at_all_optimal);
      StoreMinCost result(store);
      HashTable* tt = new HashTable(opt_tt_size);
      DFS_BB op_search(stats, result, tt);
      op_search.set_cycle_check(opt_cc);
      op_search.start(*search_root, min_makespan);
      std::cerr << "final cost: " << op_search.cost()
		<< " (improvement: "
		<< solution_cost - op_search.cost()
		<< " (abs.) "
		<< (solution_cost - op_search.cost())/op_search.cost()
		<< " (rel.), " << stats << ")" << std::endl;
      solution_cost = op_search.cost();
      optimally = op_search.optimal();
    }
  }

  if (solved && opt_validate) {
    for (index_type p = 0; p < store.n_solutions(); p++) {
      Schedule* plan = store.plan(p);
      ExecTrace trace(instance);
      std::cerr << "simulating plan " << p << "..." << std::endl;
      bool ok = plan->simulate(&trace);
      if (ok) {
	std::cout << "plan " << p << " ok" << std::endl;
	if (instance.n_resources() > 0) {
	  amt_vec peak;
	  trace.peak_resource_use(peak);
	  for (index_type k = 0; k < instance.n_resources(); k++) {
	    NTYPE ratio = (peak[k] / instance.resources[k].init);
	    std::cout << instance.resources[k].name
		      << '\t' << peak[k]
		      << '\t' << instance.resources[k].init
		      << '\t' << PRINT_NTYPE(ratio*100)
		      << std::endl;
	  }
	}
      }
      else {
	std::cerr << "plan " << p << " failed" << std::endl;
      }
    }
  }

  else if (opt_ipc) {
    std::cout << "; Time " << Stopwatch::seconds() << std::endl;
    std::cout << "; ParsingTime " << parse_stats.total_time() << std::endl;
    if (solved) {
      if (opt_cost) {
	std::cout << "; NrActions " << std::endl;
	std::cout << "; MakeSpan " << std::endl;
	std::cout << "; MetricValue " << solution_cost << std::endl;
      }
      else {
	std::cout << "; NrActions " << solution_cost << std::endl;
	std::cout << "; MakeSpan " << std::endl;
	std::cout << "; MetricValue " << std::endl;
      }
      PrintIPC print_plan(instance, std::cout);
      store.output(print_plan);
    }
    else {
      std::cout << "; Not Solved (" << stats.flags() << ")" << std::endl;
    }
  }

  else if (opt_pddl) {
    if (solved) {
      PrintPDDL print_plan(instance, std::cout);
      store.output(print_plan);
    }
    else if (!stats.break_signal_raised()) {
      std::cout << "(:heuristic (";
      for (index_type k = 0; k < original_goal_atoms.length(); k++)
	std::cout << instance.atoms[original_goal_atoms[k]].name;
      std::cout << ") :opt :inf)" << std::endl;
    }
  }

  else if (verbose_level > 0) {
    if (solved) {
      std::cout << "solution cost: " << solution_cost;
      if (optimally)
	std::cout << " (optimal)";
      else
	std::cout << " (upper bound)";
      std::cout << std::endl;
      if (opt_print_plan) {
	Print print_plan(instance, std::cout);
	store.output(print_plan);
      }
      std::cout << store.n_solutions() << " solutions";
      if (opt_all_different) {
	std::cout << " (" << StoreDistinct::n_discarded
		  << " equivalent solutions discarded)";
      }
      std::cout << std::endl;
    }
    else {
      std::cout << "no solution found" << std::endl;
    }
    stats.print_total(std::cout);
    stats.print(std::cout, "(search) ");
    std::cout << "in search: " << stats.total_nodes() - pre_search_nodes
	      << " nodes, " << stats.time() << " seconds" << std::endl;
    std::cerr << "highest lower bound " << stats.max_lower_bound()
	      << " proven at " << stats.nodes_at_max_lower_bound()
	      << " nodes" << std::endl;
    double total_t = Stopwatch::seconds();
    std::cout << total_t << " seconds total (" << total_t - stats.total_time()
	      << " sec. not accounted for)" << std::endl;
    std::cout << "peak memory use: " << stats.peak_memory() << "k"
#ifdef RSS_FROM_PSINFO
	      << ", peak size: " << stats.peak_total_size() << "k"
#endif
	      << std::endl;
    if (opt_tt && (tt != 0)) {
      std::cout << "transposition table stats: TUF = " << tt->TUF()
		<< ", HCF: " << tt->HCF() << std::endl;
    }
#ifdef SEARCH_EXTRA_STATS
    std::cout << "search space extra stats:"
	      << ' ' << (rminx_size/(double)rminx_count)
	      << ' ' << (rminc_succ_size_ratio/(double)rminc_count)
	      << ' ' << (rminx_succ/(double)rminx_count)
	      << ' ' << (tries_within_bound/(double)trie_count)
	      << std::endl;
#endif
  }

  else {
    std::cout << instance.name
	      << ' ' << (solved ? 1 : 0)
	      << ' ' << (optimally ? 1 : 0)
	      << ' ' << PRINT_NTYPE(root_est_cost)
	      << ' ' << PRINT_NTYPE(solution_cost)
	      << ' ' << stats.total_nodes()
	      << ' ' << stats.total_time()
	      << ' ' << stats.peak_memory()
	      << ' ' << stats.peak_stack_size()
#ifdef RSS_FROM_PSINFO
	      << ' ' << stats.peak_total_size()
#endif
	      << ' ' << stats.complete_iterations()
	      << ' ' << stats.nodes_at_max_lower_bound()
	      << ' ' << stats.time()
#ifdef SEARCH_EXTRA_STATS
	      << ' ' << (rminx_size/(double)rminx_count)
	      << ' ' << (rminc_succ_size_ratio/(double)rminc_count)
	      << ' ' << (rminx_succ/(double)rminx_count)
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
