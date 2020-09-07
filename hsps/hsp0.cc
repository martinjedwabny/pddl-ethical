
#include "problem.h"
#include "preprocess.h"
#include "parser.h"
#include "cost_table.h"
#include "pdb_construction.h"
#include "seq_reg.h"
#include "para_reg.h"
#include "temporal.h"
#include "plans.h"
#include "ida.h"
#include "idao.h"
#include "bfs.h"
#include "bb.h"
#include "bfhs.h"
#include "minimax.h"
#include "explore.h"
#include "simplify.h"
#include "pop.h"

#include <sstream>
#include <fstream>

int main(int argc, char *argv[]) {
  HSPS::StringTable symbols(50, HSPS::lowercase_map);
  HSPS::index_type  opt_q_val = 0;
  NTYPE       opt_d_val = 0;
  bool        opt_round = false;
  bool        opt_round_up = false;
  bool        opt_round_down = false;
  bool        opt_unlimited = false;
  bool        opt_unit = false;
  bool        opt_H0 = false;
  bool        opt_H1 = false;
  bool        opt_H3 = false;
  bool        opt_AH = false;
  bool        opt_AH_max = true;
  bool        opt_structured = false;
  bool        opt_AHRC = false;
  NTYPE       alpha_AHRC = R_TO_N(5, 10);
  bool        opt_load_partition = false;
  bool        opt_rnd_relevance_partition = false;
  bool        opt_ia_partition = false;
  bool        opt_fpia = false;
  bool        opt_layered_partition = false;
  bool        opt_new_decomposition = false;
  bool        opt_bu_partition = false;
  bool        opt_load_h = false;
  bool        opt_load_compare = false;
  bool        opt_compare_max = false;
  bool        opt_all_pairs = false;
  bool        opt_pdb_1 = false;
  bool        opt_pdb_load = false;
  bool        opt_pdb_ai = false;
  bool        opt_pdb_ai_all = false;
  bool        opt_pdb_ind_pair = false;
  bool        opt_pdb_ind_bin = false;
  bool        opt_pdb_wbin = false;
  bool        opt_pdb_windbin = false;
  bool        opt_pdb_collapse = true;
  bool        opt_pdb_bin = false;
  bool        opt_pdb_random_bin = false;
  bool        opt_pdb_independent_random_bin = false;
  bool        opt_pdb_spanning_subset = true;
  HSPS::index_type  opt_pdb_random_bin_swaps = 10000;
  HSPS::index_type  opt_pdb_random_bin_k = 0;
  bool        opt_pdb_cg = false;
  bool        opt_pdb_int = false;
  HSPS::index_type  opt_pdb_size = 20000;
  bool        opt_pdb_add = true;
  bool        opt_pdb_ext_add = false;
  HSPS::rational    ext_add_threshold = 0;
  bool        opt_pdb_add_all = false;
  bool        opt_pdb_reduced = false;
  bool        opt_pdb_inc = true;
  bool        opt_optimal = false;
  bool        opt_maximal_add = true;
  bool        opt_sas = false;
  bool        opt_sas_min = false;
  bool        opt_sas_select = false;
  bool        opt_apply_invariants = false;
  bool        opt_eval_cache = false;
  bool        opt_sequential = false;
  bool        opt_temporal = false;
  bool        opt_resource = false;
  bool        opt_compose = false;
  HSPS::index_type  composite_resource_size = 2;
  bool        opt_R0 = false;
  bool        opt_R2 = false;
  bool        opt_RAH = false;
  bool        opt_rpdb = false;
  bool        opt_cost = false;
  bool        opt_deadline = false;
  bool        opt_apply_cuts = true;
  bool        opt_ncw = false;
  bool        opt_preprocess = true;
  bool        opt_rm_irrelevant = false;
  bool        opt_quick_find_invariants = false;
  bool        opt_find_invariants = false;
  bool        opt_negation_invariants = true;
  bool        opt_verify_invariants = false;
  bool        opt_extend_goal = false;
  bool        opt_find_all = false;
  bool        opt_all_different = false;
  bool        opt_exhaustive = false;
  bool        opt_find_n = false;
  HSPS::count_type  n_to_find = 0;
  bool        opt_post_op = false;
  bool        opt_print_plan = true;
  bool        opt_validate = false;
  bool        opt_pop = false;
  bool        opt_path = false;
  bool        opt_write_graphs = false;
  bool        opt_pddl = false;
  bool        opt_gantt = false;
  bool        opt_ipc = false;
  bool        opt_strict_ipc = false;
#ifdef NTYPE_RATIONAL
  NTYPE       epsilon = HSPS::rational(1,100);
#else
  NTYPE       epsilon = 0.001;
#endif
  bool        opt_cc = false;
  bool        opt_tt = false;
  HSPS::index_type opt_tt_size = 31337;
  bool        opt_bfs = false;
  bool        opt_bfs_px = false;
  NTYPE       px_threshold = 0;
  bool        opt_dfs = false;
  bool        opt_bb = false;
  bool        opt_lds = false;
  bool        opt_bfida = false;
  bool        opt_idao = false;
  bool        opt_explore_to_depth = false;
  bool        opt_explore_to_bound = false;
  HSPS::index_type  depth_limit = 0;
  bool        opt_print_solution_only = false;
  bool        opt_no_print_solution = false;
  bool        opt_apx = false;
  HSPS::index_type  apx_limit = 0;
  double      time_limit = 0;
  unsigned long memory_limit = 0;
  unsigned long stack_limit = 0;
  HSPS::index_type  iteration_limit = 0;
  HSPS::count_type  node_limit = 0;
  NTYPE       cost_limit = POS_INF;
  bool        opt_limit_to_length = false;
  bool        opt_save = false;
  bool        opt_look_ahead = false;
  HSPS::index_type  look_ahead_depth = 1;
  unsigned long rnd_seed = 0;
  int         verbose_level = 1;
  bool        opt_bfs_write_graph = false;
  bool        opt_bfs_write_stats = false;

  HSPS::Statistics parse_stats;
  HSPS::Parser* reader = new HSPS::Parser(symbols);

  for (int k = 1; k < argc; k++) {
    // verbose level
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      verbose_level = atoi(argv[++k]);
      if (verbose_level < 1) opt_print_plan = false;
      if (verbose_level <= 0) HSPS::PDDL_Base::write_warnings = false;
      if (verbose_level > 1) HSPS::PDDL_Base::write_info = true;
      // if (verbose_level > 1) HSPS::Statistics::running_print_max = true;
    }
    else if (strcmp(argv[k],"-no-warnings") == 0) {
      HSPS::PDDL_Base::write_warnings = false;
    }
    else if (strcmp(argv[k],"-no-info") == 0) {
      HSPS::PDDL_Base::write_info = false;
    }
    else if (strcmp(argv[k],"-name-by-file") == 0) {
      HSPS::PDDL_Base::name_instance_by_problem_file = true;
    }

    // problem input/tranformation options
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
    else if (strcmp(argv[k],"-u") == 0) {
      opt_unlimited = true;
    }
    else if (strcmp(argv[k],"-unit") == 0) {
      opt_unit = true;
    }
    else if (strcmp(argv[k],"-use-strict-borrow") == 0) {
      HSPS::PDDL_Base::use_strict_borrow_definition = true;
    }
    else if (strcmp(argv[k],"-use-extended-borrow") == 0) {
      HSPS::PDDL_Base::use_extended_borrow_definition = true;
    }
    else if (strcmp(argv[k],"-no-compile") == 0) {
      HSPS::PDDL_Base::compile_away_disjunctive_preconditions = false;
      HSPS::PDDL_Base::compile_away_conditional_effects = false;
    }
    else if (strcmp(argv[k],"-no-compact") == 0) {
      HSPS::PDDL_Base::compact_resource_effects = false;
    }
    else if (strcmp(argv[k],"-non-strict-set-export") == 0) {
      HSPS::PDDL_Base::strict_set_export = false;
    }

    // search space (problem type) selection
    else if (strcmp(argv[k],"-seq") == 0) {
      opt_sequential = true;
    }
    else if (strcmp(argv[k],"-time") == 0) {
      opt_temporal = true;
      opt_ncw = true;
    }
    else if (strcmp(argv[k],"-deadline") == 0) {
      opt_deadline = true;
      opt_temporal = true;
      opt_ncw = true;
    }
    else if (strcmp(argv[k],"-strict-right-shift") == 0) {
      HSPS::TemporalRegState::strict_right_shift = true;
    }
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
    else if (strcmp(argv[k],"-ncw") == 0) {
      opt_ncw = true;
    }
    else if (strcmp(argv[k],"-no-ncw") == 0) {
      opt_ncw = false;
    }
    else if (strcmp(argv[k],"-inv") == 0) {
      opt_apply_invariants = true;
    }
    else if (strcmp(argv[k],"-eac") == 0) {
      opt_eval_cache = true;
    }
    else if (strcmp(argv[k],"-no-implicit-noop") == 0) {
      HSPS::TemporalRegState::implicit_noop = false;
    }
    else if (strcmp(argv[k],"-no-subgoal-ordering") == 0) {
      HSPS::TemporalRegState::subgoal_ordering = false;
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
    else if (strcmp(argv[k],"-quick-find") == 0) {
      opt_quick_find_invariants = true;
    }
    else if (strcmp(argv[k],"-find") == 0) {
      opt_find_invariants = true;
    }
    else if (strcmp(argv[k],"-no-find") == 0) {
      opt_find_invariants = false;
    }
    else if (strcmp(argv[k],"-no-neg") == 0) {
      opt_negation_invariants = false;
    }
    else if (strcmp(argv[k],"-verify") == 0) {
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-no-verify") == 0) {
      opt_verify_invariants = false;
    }
    else if (strcmp(argv[k],"-extend") == 0) {
      opt_extend_goal = true;
    }
    else if (strcmp(argv[k],"-no-extend") == 0) {
      opt_extend_goal = false;
    }
    else if ((strcmp(argv[k],"-compose") == 0) && (k < argc - 1)) {
      opt_compose = true;
      composite_resource_size = atoi(argv[++k]);
    }

    // heuristic options
    else if ((strcmp(argv[k],"-l") == 0) || (strcmp(argv[k],"-load") == 0)) {
      opt_load_h = true;
    }
    else if (strcmp(argv[k],"-compare") == 0) {
      opt_load_compare = true;
    }
    else if (strcmp(argv[k],"-compare-max") == 0) {
      opt_load_compare = true;
      opt_compare_max = true;
    }
    else if (strcmp(argv[k],"-1") == 0) {
      opt_H1 = true;
    }
    else if (strcmp(argv[k],"-0") == 0) {
      opt_H0 = true;
    }
    else if (strcmp(argv[k],"-3") == 0) {
      opt_H3 = true;
    }
    else if (strcmp(argv[k],"-R0") == 0) {
      opt_R0 = true;
    }
    else if (strcmp(argv[k],"-R2") == 0) {
      opt_R2 = true;
    }
    else if (strcmp(argv[k],"-RAH") == 0) {
      opt_RAH = true;
    }
    else if (strcmp(argv[k],"-rpdb") == 0) {
      opt_rpdb = true;
      opt_sas = true;
      opt_optimal = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-AH") == 0) {
      opt_AH = true;
    }
    else if ((strcmp(argv[k],"-AHRC") == 0) && (k < argc - 1)) {
      opt_AH = true;
      opt_AHRC = true;
      opt_AH_max = false;
      alpha_AHRC = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-AHX") == 0) {
      opt_AH = true;
      opt_structured = true;
    }
    else if (strcmp(argv[k],"-no-max") == 0) {
      opt_AH_max = false;
    }
    else if (strcmp(argv[k],"-load-p") == 0) {
      opt_load_partition = true;
    }
    else if (strcmp(argv[k],"-prr") == 0) {
      opt_rnd_relevance_partition = true;
    }
    else if (strcmp(argv[k],"-pia") == 0) {
      opt_ia_partition = true;
      opt_optimal = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-fpia") == 0) {
      opt_ia_partition = true;
      opt_fpia = true;
      opt_optimal = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pbu") == 0) {
      opt_bu_partition = true;
      opt_optimal = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-cnd") == 0) {
      opt_new_decomposition = true;
      opt_optimal = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pl") == 0) {
      opt_layered_partition = true;
    }
    else if (strcmp(argv[k],"-pdb-ap") == 0) {
      opt_all_pairs = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-load") == 0) {
      opt_pdb_load = true;
    }
    else if (strcmp(argv[k],"-pdb-ai") == 0) {
      opt_pdb_ai = true;
      opt_optimal = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-ai-all") == 0) {
      opt_pdb_ai_all = true;
      opt_optimal = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-strict-ai") == 0) {
      opt_pdb_ai = true;
      opt_optimal = true;
      opt_pdb_collapse = false;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-strict-ai-all") == 0) {
      opt_pdb_ai_all = true;
      opt_optimal = true;
      opt_pdb_collapse = false;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-cg") == 0) {
      opt_pdb_cg = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-int") == 0) {
      opt_pdb_int = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-bin") == 0) {
      opt_pdb_bin = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-rbin") == 0) {
      opt_pdb_random_bin = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-rib") == 0) {
      opt_pdb_independent_random_bin = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-strict-rib") == 0) {
      opt_pdb_independent_random_bin = true;
      opt_pdb_collapse = false;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if ((strcmp(argv[k],"-rbin-k") == 0) && (k < argc - 1)) {
      opt_pdb_random_bin_k = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-rbin-swaps") == 0) && (k < argc - 1)) {
      opt_pdb_random_bin_swaps = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-pdb-no-span") == 0) {
      opt_pdb_spanning_subset = false;
    }
    else if (strcmp(argv[k],"-pdb-1") == 0) {
      opt_pdb_1 = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-ip") == 0) {
      opt_pdb_ind_pair = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-ibin") == 0) {
      opt_pdb_ind_bin = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-wbin") == 0) {
      opt_pdb_wbin = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-windbin") == 0) {
      opt_pdb_windbin = true;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-pdb-strict-windbin") == 0) {
      opt_pdb_windbin = true;
      opt_pdb_collapse = false;
      opt_find_invariants = true;
      opt_verify_invariants = true;
    }
    else if ((strcmp(argv[k],"-pdb-size") == 0) && (k < argc - 1)) {
      opt_pdb_size = atoi(argv[++k]);
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
    else if ((strcmp(argv[k],"-xadd") == 0) && (k < argc - 1)) {
      opt_pdb_add = true;
      opt_pdb_ext_add = true;
      ext_add_threshold = HSPS::rational::ator(argv[++k]);
    }
    else if (strcmp(argv[k],"-add-all") == 0) {
      opt_pdb_add = true;
      opt_pdb_add_all = true;
    }
    else if (strcmp(argv[k],"-red") == 0) {
      opt_pdb_reduced = true;
    }
    else if (strcmp(argv[k],"-inc") == 0) {
      opt_pdb_inc = true;
    }
    else if (strcmp(argv[k],"-no-inc") == 0) {
      opt_pdb_inc = false;
    }
    else if (strcmp(argv[k],"-o") == 0) {
      opt_optimal = true;
    }
    else if (strcmp(argv[k],"-non-optimal") == 0) {
      opt_optimal = false;
    }
    else if (strcmp(argv[k],"-ac") == 0) {
      opt_optimal = false;
    }
    else if ((strcmp(argv[k],"-look") == 0) && (k < argc - 1)) {
      opt_look_ahead = true;
      look_ahead_depth = atoi(argv[++k]);
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
    else if (strcmp(argv[k],"-bfs-write-stats") == 0) {
      opt_bfs_write_stats = true;
    }
    else if (strcmp(argv[k],"-bfs-write-graph") == 0) {
      opt_bfs_write_graph = true;
    }
    else if (strcmp(argv[k],"-bfs-write-graph-compact") == 0) {
      opt_bfs_write_graph = true;
      HSPS::NodeSet::write_state_in_graph_node = false;
    }
    else if (strcmp(argv[k],"-bb") == 0) {
      opt_bb = true;
      opt_find_all = true;
    }
    else if (strcmp(argv[k],"-dfs") == 0) {
      opt_dfs = true;
    }
    else if (strcmp(argv[k],"-lds") == 0) {
      opt_lds = true;
      opt_sequential = true;
      opt_bb = true;
      opt_find_all = true;
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
    else if ((strcmp(argv[k],"-tt-size") == 0) && (k < argc - 1)) {
      opt_tt_size = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-idao") == 0) {
      opt_idao = true;
    }
    else if (strcmp(argv[k],"-post") == 0) {
      opt_round_up = true;
      opt_round = false;
      opt_round_down = false;
      opt_post_op = true;
    }
    else if (strcmp(argv[k],"-all") == 0) {
      opt_find_all = true;
    }
    else if (strcmp(argv[k],"-all-different") == 0) {
      if (!opt_exhaustive) opt_find_all = true;
      opt_all_different = true;
    }
    else if (strcmp(argv[k],"-ex") == 0) {
      opt_find_all = false;
      opt_exhaustive = true;
    }
    else if ((strcmp(argv[k],"-k") == 0) && (k < argc - 1)) {
      opt_find_n = true;
      n_to_find = atoi(argv[++k]);
    }
    else if (((strcmp(argv[k],"-e") == 0) ||
	      (strcmp(argv[k],"-ed") == 0)) &&
	     (k < argc - 1)) {
      opt_explore_to_depth = true;
      depth_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-eb") == 0) && (k < argc - 1)) {
      opt_explore_to_bound = true;
      cost_limit = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-print-solution-only") == 0) {
      opt_print_solution_only = true;
    }
    else if (strcmp(argv[k],"-no-print-solution") == 0) {
      opt_no_print_solution = true;
    }
    else if ((strcmp(argv[k],"-apx") == 0) && (k < argc - 1)) {
      opt_apx = true;
      apx_limit = atoi(argv[++k]);
    }

    // limit-setting options
    else if ((strcmp(argv[k],"-t") == 0) && (k < argc - 1)) {
      time_limit = atof(argv[++k]);
    }
    else if ((strcmp(argv[k],"-y") == 0) && (k < argc - 1)) {
      memory_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-s") == 0) && (k < argc - 1)) {
      stack_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-i") == 0) && (k < argc - 1)) {
      iteration_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-c") == 0) && (k < argc - 1)) {
      cost_limit = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-ll") == 0) {
      opt_limit_to_length = true;
    }
    else if ((strcmp(argv[k],"-n") == 0) && (k < argc - 1)) {
      node_limit = atoi(argv[++k]);
    }

    // output/formatting options
    else if (strcmp(argv[k],"-val") == 0) {
      opt_validate = true;
    }
    else if (strcmp(argv[k],"-no-plan") == 0) {
      opt_print_plan = false;
    }
    else if (strcmp(argv[k],"-plan") == 0) {
      opt_print_plan = true;
    }
    else if (strcmp(argv[k],"-pop") == 0) {
      opt_print_plan = true;
      opt_pop = true;
    }
    else if (strcmp(argv[k],"-path") == 0) {
      opt_print_plan = true;
      opt_path = true;
    }
    else if (strcmp(argv[k],"-g") == 0) {
      opt_write_graphs = true;
    }
    else if (strcmp(argv[k],"-gantt") == 0) {
      opt_gantt = true;
      HSPS::Schedule::GANTT_ACTION_NAMES_ON_CHART = false;
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
      HSPS::Instance::write_atom_set_with_symbolic_names = false;
      HSPS::Instance::write_action_set_with_symbolic_names = false;
    }
    else if (strcmp(argv[k],"-print-rational") == 0) {
      HSPS::Print::decimal_time = false;
    }

    // misc. options
    else if (strcmp(argv[k],"-save") == 0) {
      opt_save = true;
    }
    else if (strcmp(argv[k],"-sas-min") == 0) {
      opt_sas_min = true;
    }
    else if (strcmp(argv[k],"-sas-select") == 0) {
      opt_sas_select = true;
    }
    else if (((strcmp(argv[k],"-rnd") == 0) ||
	      (strcmp(argv[k],"-r") == 0)) &&
	     (k < argc - 1)) {
      rnd_seed = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-exclude") == 0) && (k < argc - 1)) {
      char* tag = argv[++k];
      if (strcmp(tag, "all") == 0) {
	HSPS::PDDL_Base::exclude_all_dkel_items = true;
      }
      else {
	const HSPS::StringTable::Cell* c = symbols.find(tag);
	if (c) HSPS::PDDL_Base::excluded_dkel_tags.insert(c->text);
      }
    }
    else if ((strcmp(argv[k],"-require") == 0) && (k < argc - 1)) {
      const HSPS::StringTable::Cell* c = symbols.find(argv[++k]);
      if (c) HSPS::PDDL_Base::required_dkel_tags.insert(c->text);
    }

    // input file
    else if (*argv[k] != '-') {
      parse_stats.start();
      reader->read(argv[k], false);
      parse_stats.stop();
    }
  }

  HSPS::SearchAlgorithm::default_trace_level = verbose_level;
  HSPS::Heuristic::default_trace_level = verbose_level - 1;
  HSPS::Instance::default_trace_level = verbose_level - 1;
  HSPS::Preprocessor::default_trace_level = verbose_level - 1;

  bool opt_pdb = (opt_pdb_load || opt_pdb_bin || opt_pdb_random_bin ||
		  (opt_pdb_independent_random_bin &&
		   (opt_pdb_random_bin_k == 0)) ||
		  opt_pdb_cg || opt_pdb_int || opt_pdb_1 ||
		  opt_pdb_ind_pair || opt_pdb_ind_bin || opt_pdb_wbin ||
		  opt_pdb_windbin);
  if (opt_pdb || opt_all_pairs || opt_pdb_ai ||
      opt_pdb_ai_all || (opt_pdb_random_bin_k > 0))
    opt_sas = true;

  HSPS::Instance   instance;
  HSPS::cost_vec   saved_dur;
  HSPS::cost_vec   saved_res;
  HSPS::Statistics stats;
  NTYPE            root_est_cost = 0;
  bool             solved = false;
  bool             optimally = false;
  NTYPE            solution_cost = 0;
  HSPS::Store      store(instance);
  HSPS::Statistics look_ahead_stats;
  HSPS::HashTable* tt = 0; // so we can access table use stats at end
  HSPS::SASInstance* sas_instance = 0;
  HSPS::index_set    sas_variables;

  HSPS::LC_RNG rnd_source;
  if (rnd_seed > 0) rnd_source.seed(rnd_seed);

  stats.enable_interrupt(false);
  if (time_limit > 0) stats.enable_time_out(time_limit, false);
  if (memory_limit > 0) stats.enable_memory_limit(memory_limit, false);
  if (stack_limit > 0) stats.enable_stack_limit(stack_limit, false);

  stats.start();
  std::cerr << "instantiating..." << std::endl;
  reader->instantiate(instance);

  if (opt_resource && opt_compose && (instance.n_resources() > 1)) {
    HSPS::mSubsetEnumerator crs(instance.n_resources(),
				composite_resource_size);
    bool more = crs.first();
    while (more) {
      HSPS::index_set s;
      crs.current_set(s);
      instance.create_composite_resource(s);
      more = crs.next();
    }
  }

  HSPS::Preprocessor prep(instance, stats);
  if (opt_preprocess) {
    std::cerr << "preprocessing..." << std::endl;
    prep.preprocess();
    if (opt_rm_irrelevant) {
      prep.compute_irrelevant_atoms();
      prep.remove_irrelevant_atoms();
      if (!instance.cross_referenced()) {
	std::cerr << "re-cross referencing..." << std::endl;
	instance.cross_reference();
      }
    }
  }
  else {
    std::cerr << "cross referencing..." << std::endl;
    instance.cross_reference();
  }
  if (opt_quick_find_invariants) {
    HSPS::graph* g_inc = prep.inconsistency_graph();
    prep.find_inconsistent_set_invariants(*g_inc);
    instance.add_missing_negation_invariants();
  }
  else if (opt_find_invariants) {
    prep.bfs_find_invariants();
  }
  if (opt_negation_invariants) {
    instance.add_missing_negation_invariants();
  }
  if (opt_verify_invariants) {
    // instance.verify_invariants();
    prep.verify_invariants(*(prep.inconsistency()));
    prep.remove_unverified_invariants();
  }
  if (opt_extend_goal) {
    HSPS::index_set new_goals;
    prep.implied_atom_set(instance.goal_atoms,new_goals,*prep.inconsistency());
    std::cerr << new_goals.length() << " implied goals found" << std::endl;
    if (new_goals.length() > 0) {
      HSPS::index_set new_goal(instance.goal_atoms);
      new_goal.insert(new_goals);
      instance.set_goal(new_goal);
    }
  }

  instance.save_durations(saved_dur);
  if (opt_unit) {
    for (HSPS::index_type k = 0; k < instance.n_actions(); k++) {
      instance.actions[k].cost = 1;
      instance.actions[k].dmin = 1;
      instance.actions[k].dmax = 1;
      instance.actions[k].dur = 1;
    }
  }
  else if (opt_d_val > 0) instance.discretize_durations(opt_d_val);
  else if (opt_q_val > 0) instance.quantize_durations(opt_q_val);
  else if (opt_round_up) instance.round_durations_up();
  else if (opt_round_down) instance.round_durations_down();
  else if (opt_round) instance.round_durations();
  if (opt_unlimited) instance.assign_unlimited_resources(saved_res);
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

  if (verbose_level > 2) {
    instance.print(std::cerr);
  }

  if (opt_sas) {
    stats.start();
    std::cerr << "constructing SAS+ instance..." << std::endl;
    sas_instance =
      new HSPS::SASInstance(instance, opt_sas_select, opt_sas_min, false);
    sas_variables.fill(sas_instance->n_variables());
    if (verbose_level > 1) {
      std::cerr << "variables:" << std::endl;
      for (HSPS::index_type k = 0; k < sas_instance->n_variables(); k++) {
	std::cerr << "(" << k << ") ";
	sas_instance->write_variable(std::cerr, sas_instance->variables[k]);
	std::cerr << std::endl;
      }
      std::cerr << "initial: ";
      sas_instance->write_partial_state(std::cerr, sas_instance->init_state);
      std::cerr << std::endl << "goal: ";
      sas_instance->write_partial_state(std::cerr, sas_instance->goal_state);
      std::cerr << std::endl;
    }
    stats.stop();
  }

  if (!stats.break_signal_raised()) {
    stats.start();
    HSPS::Heuristic* cost_est = 0;
    HSPS::estimator_vec resource_est(0, instance.n_resources());

    if (opt_load_h) {
      HSPS::CostTable* cost_tab = new HSPS::CostTable(instance, stats);
      std::cerr << "loading heuristic table (" << reader->h_table.length()
		<< " entries)..." << std::endl;
      reader->export_heuristic(instance, prep.atom_map, true, *cost_tab);
      cost_est = cost_tab;
    }

    else if (opt_pdb) {
      HSPS::PDBHeuristic* h_pdb = 0;
      HSPS::RegressionPDBSize set_size(sas_instance->signature);

      if (opt_pdb_load) {
	HSPS::name_vec vnames(0, 0);
	sas_instance->variable_names(vnames);
	HSPS::index_set_vec sets;
	reader->export_sets(vnames, sets);
	h_pdb = new HSPS::PDBHeuristic(*sas_instance, stats);
	h_pdb->assign_sets(sets);
	if (verbose_level > 1) {
	  std::cerr << "PDB sets: ";
	  h_pdb->write_variable_sets(std::cerr);
	  std::cerr << std::endl;
	}
	if (opt_pdb_ext_add || opt_pdb_add)
	  h_pdb->compute_additive_groups(opt_maximal_add);
      }

      else if (opt_pdb_bin) {
	HSPS::MaxBinPDB* h_bin =
	  new HSPS::MaxBinPDB(*sas_instance, stats);
	if (opt_optimal)
	  h_bin->compute_sets_optimal(opt_pdb_size, set_size);
	else
	  h_bin->compute_sets(opt_pdb_size, set_size);
	if (verbose_level > 1) {
	  std::cerr << "max-bin sets: ";
	  h_bin->write_variable_sets(std::cerr);
	  std::cerr << std::endl;
	}
	if (opt_pdb_ext_add || opt_pdb_add)
	  h_bin->compute_additive_groups(opt_maximal_add);
	h_pdb = h_bin;
      }

      else if (opt_pdb_random_bin) {
	HSPS::RandomBinPDB* h_bin =
	  new HSPS::RandomBinPDB(*sas_instance, stats);
	h_bin->compute_sets(opt_pdb_size, set_size,
			    rnd_source, opt_pdb_random_bin_swaps);
	if (verbose_level > 1) {
	  std::cerr << "random bin sets: ";
	  h_bin->write_variable_sets(std::cerr);
	  std::cerr << std::endl;
	}
	if (opt_pdb_ext_add || opt_pdb_add)
	  h_bin->compute_additive_groups(opt_maximal_add);
	h_pdb = h_bin;
      }

      else if (opt_pdb_independent_random_bin) {
	HSPS::RandomIndependentBinPDB* h_bin =
	  new HSPS::RandomIndependentBinPDB(*sas_instance, stats);
	h_bin->compute_sets(opt_pdb_size, set_size,
			    opt_pdb_spanning_subset, opt_pdb_collapse,
			    rnd_source, opt_pdb_random_bin_swaps);
	if (verbose_level > 1) {
	  std::cerr << "random bin sets: ";
	  h_bin->write_variable_sets(std::cerr);
	  std::cerr << std::endl;
	}
	if (opt_pdb_ext_add || opt_pdb_add)
	  h_bin->compute_additive_groups(opt_maximal_add);
	h_pdb = h_bin;
      }

      else if (opt_pdb_cg) {
	HSPS::MaxCGPDB* h_cg = new HSPS::MaxCGPDB(*sas_instance, stats);
	h_cg->compute_sets(opt_pdb_size, set_size);
	if (verbose_level > 1) {
	  std::cerr << "max-cg sets: ";
	  h_cg->write_variable_sets(std::cerr);
	  std::cerr << std::endl;
	}
	if (opt_pdb_ext_add || opt_pdb_add)
	  h_cg->compute_additive_groups(opt_maximal_add);
	h_pdb = h_cg;
      }

      else if (opt_pdb_int) {
	HSPS::MaxInterferencePDB* h_int =
	  new HSPS::MaxInterferencePDB(*sas_instance, stats);
	if (opt_pdb_reduced)
	  h_int->compute_reduced_sets(opt_pdb_size, set_size);
	else
	  h_int->compute_sets(opt_pdb_size, set_size);
	if (verbose_level > 1) {
	  std::cerr << "max-interference sets: ";
	  h_int->write_variable_sets(std::cerr);
	  std::cerr << std::endl;
	}
	if (opt_pdb_ext_add || opt_pdb_add)
	  h_int->compute_additive_groups(opt_maximal_add);
	h_pdb = h_int;
      }

      else if (opt_pdb_ind_pair) {
	HSPS::IndependentPairPDB* h_pair =
	  new HSPS::IndependentPairPDB(*sas_instance, stats);
	h_pair->compute_sets();
	if (verbose_level > 1) {
	  std::cerr << "independent variable pairs: ";
	  h_pair->write_variable_sets(std::cerr);
	  std::cerr << std::endl;
	}
	if (opt_pdb_ext_add || opt_pdb_add) {
	  if (opt_pdb_add_all)
	    h_pair->compute_all_additive_groups();
	  else
	    h_pair->compute_additive_groups(opt_maximal_add);
	}
	h_pdb = h_pair;
      }

      else if (opt_pdb_ind_bin) {
	HSPS::IndependentBinPDB* h_ibin =
	  new HSPS::IndependentBinPDB(*sas_instance, stats);
	if (opt_optimal)
	  h_ibin->compute_sets_optimal(opt_pdb_size, set_size);
	else
	  h_ibin->compute_sets(opt_pdb_size, set_size);
	if (verbose_level > 1) {
	  std::cerr << "independent variable sets: ";
	  h_ibin->write_variable_sets(std::cerr);
	  std::cerr << std::endl;
	}
	if (opt_pdb_ext_add || opt_pdb_add)
	  h_ibin->compute_additive_groups(opt_maximal_add);
	h_pdb = h_ibin;
      }

// 	else if (opt_pdb_wbin) {
// 	  HSPS::SetValueFunction* svf_isi =
// 	    new HSPS::InitialStateInterference(*sas_instance);
// 	  HSPS::SetValueFunction* svf_aaf =
// 	    new HSPS::AffectingActionFraction(*sas_instance);
// 	  HSPS::SetValueFunction* svf_icg =
// 	    new HSPS::InverseCGFraction(*sas_instance);
// 	  HSPS::SetValueFunction* svf_sum =
// 	    new HSPS::SetValueSum(*svf_aaf, 2*sas_instance->n_actions(),
// 				  *(new HSPS::SetValueSum(*svf_isi, *svf_icg)), 1);
// 	  // SetValueFunction* svf_sum = new SetValueSum(*svf_isi, *svf_icg);
// 	  HSPS::WeightedBinPDB* h_wbin =
// 	    new HSPS::WeightedBinPDB(*sas_instance, stats, *svf_sum);
// 	  h_wbin->compute_sets(opt_pdb_size, set_size);
// 	  if (verbose_level > 1) {
// 	    std::cerr << "PDB variable sets: ";
// 	    h_wbin->write_variable_sets(std::cerr);
// 	    std::cerr << std::endl;
// 	  }
// 	  if (opt_pdb_ext_add || opt_pdb_add)
// 	    h_wbin->compute_additive_groups(opt_maximal_add);
// 	  h_pdb = h_wbin;
// 	}

      else if (opt_pdb_windbin) {
	// CombinedSetValue* sv_fun = new CombinedSetValue(*sas_instance);
	HSPS::InitialStateInterference* sv_isi =
	  new HSPS::InitialStateInterference(*sas_instance);
	HSPS::InverseCGFraction* sv_icg =
	  new HSPS::InverseCGFraction(*sas_instance);
	HSPS::SetValueSum* sv_sum = new HSPS::SetValueSum(*sv_isi, *sv_icg);
	HSPS::WeightedIndependentBinPDB* h_wbin =
	  new HSPS::WeightedIndependentBinPDB(*sas_instance, stats, *sv_sum);
	if (opt_optimal)
	  h_wbin->compute_sets_optimal(sas_variables,
				       opt_pdb_size, set_size,
				       opt_pdb_spanning_subset,
				       opt_pdb_collapse);
	else
	  h_wbin->compute_sets(sas_variables,
			       opt_pdb_size, set_size,
			       opt_pdb_spanning_subset,
			       opt_pdb_collapse);
	if (verbose_level > 1) {
	  std::cerr << "PDB variable sets: ";
	  h_wbin->write_variable_sets(std::cerr);
	  std::cerr << std::endl;
	}
	if (opt_pdb_ext_add || opt_pdb_add)
	  h_wbin->compute_additive_groups(opt_maximal_add);
	h_pdb = h_wbin;
      }

      else {
	HSPS::Max1PDB* h_1 = new HSPS::Max1PDB(*sas_instance, stats);
	h_1->compute_sets();
	if (opt_pdb_ext_add || opt_pdb_add)
	  h_1->compute_additive_groups(opt_maximal_add);
	h_pdb = h_1;
      }

      HSPS::ACF* cost = (opt_cost ?
			 (HSPS::ACF*)new HSPS::SASCostACF(*sas_instance) :
			 (HSPS::ACF*)new HSPS::UnitACF());
      HSPS::Heuristic* inc = (opt_pdb_inc ? prep.inconsistency() : 0);
      HSPS::MDDNode* sinc = (opt_pdb_inc ?
			     makeMDD(prep.inconsistency(),
				     sas_instance->atom_map_defined(),
				     sas_instance->atom_map_n()) : 0);
      std::cerr << "computing PDB:s..." << std::endl;
      h_pdb->compute_regression_PDB(*cost, sinc, inc);
      if (opt_save && !stats.break_signal_raised()) {
	h_pdb->write(std::cout);
	exit(0);
      }

      HSPS::SASHeuristic* h_sas = 0;

      if (opt_pdb_ext_add && !stats.break_signal_raised()) {
	std::cerr << "creating extended additive groups..." << std::endl;
	if (verbose_level > 1) {
	  std::cerr << "additive groups: ";
	  h_pdb->write_additive_groups(std::cerr);
	  std::cerr << std::endl;
	}
	h_pdb->make_extended_additive_groups
	  (*cost, sinc, inc, ext_add_threshold);
	h_sas = h_pdb;
      }
      else if (opt_pdb_add && !stats.break_signal_raised()) {
	std::cerr << "creating additive groups..." << std::endl;
	if (verbose_level > 1) {
	  std::cerr << "additive groups: ";
	  h_pdb->write_additive_groups(std::cerr);
	  std::cerr << std::endl;
	}
#ifdef ENABLE_HIGHER_TRACE_LEVEL
	h_pdb->make_additive_groups();
	h_sas = h_pdb;
#else
	HSPS::MaxAddH* h_ma = h_pdb->make_max_add();
	h_sas = h_ma;
#endif
      }
      else {
	h_sas = h_pdb;
      }

      cost_est = new HSPS::BwdSASHAdapter(instance, *sas_instance, *h_sas);

      stats.stop();
      if (!stats.break_signal_raised()) {
	std::cerr << "PDB heuristic computed in " << stats.time()
		  << " seconds" << std::endl;
      }
    }

    else if (opt_all_pairs) {
      HSPS::ACF* cost = (opt_cost ?
			 (HSPS::ACF*)new HSPS::SASCostACF(*sas_instance) :
			 (HSPS::ACF*)new HSPS::UnitACF());
      HSPS::Heuristic* inc = (opt_pdb_inc ? prep.inconsistency() : 0);
      std::cerr << "computing pair PDB:s..." << std::endl;
      HSPS::SASHeuristic* h_sas =
	HSPS::MaxMatchPDB::independent_pair_regression_heuristic
	(*sas_instance, stats, *cost, inc);
      cost_est = new HSPS::BwdSASHAdapter(instance, *sas_instance, *h_sas);
    }

    else if (opt_pdb_random_bin_k > 0) {
      HSPS::RegressionPDBSize set_size(sas_instance->signature);
      HSPS::MaxH* h_max = new HSPS::MaxH();
      for (HSPS::index_type k = 0; k < opt_pdb_random_bin_k; k++) {
	HSPS::RandomIndependentBinPDB* h_bin =
	  new HSPS::RandomIndependentBinPDB(*sas_instance, stats);
	h_bin->compute_sets(opt_pdb_size, set_size,
			    opt_pdb_spanning_subset, opt_pdb_collapse,
			    rnd_source, opt_pdb_random_bin_swaps);
	if (verbose_level > 1) {
	  std::cerr << "random bin #" << k + 1 << " sets: ";
	  h_bin->write_variable_sets(std::cerr);
	  std::cerr << std::endl;
	}
	HSPS::ACF* cost = (opt_cost ?
			   (HSPS::ACF*)new HSPS::SASCostACF(*sas_instance) :
			   (HSPS::ACF*)new HSPS::UnitACF());
	HSPS::Heuristic* inc = (opt_pdb_inc ? prep.inconsistency() : 0);
	HSPS::MDDNode* sinc = (opt_pdb_inc ?
			       makeMDD(prep.inconsistency(),
				       sas_instance->atom_map_defined(),
				       sas_instance->atom_map_n()) : 0);
	std::cerr << "computing PDB:s (#" << k+1 << ")..." << std::endl;
	h_bin->compute_regression_PDB(*cost, sinc, inc);
	if (opt_pdb_ext_add || opt_pdb_add) {
	  std::cerr << "creating additive groups (#" << k+1 << ")..."
		    << std::endl;
	  h_bin->compute_additive_groups(opt_maximal_add);
	  h_max->new_component(h_bin->make_max_add());
	}
	else {
	  h_max->new_component(h_bin);
	}
      }
      cost_est = new HSPS::BwdSASHAdapter(instance, *sas_instance, *h_max);
    }

    else if (opt_pdb_ai_all) {
      HSPS::AdditiveIncrementalRegressionPDB* h_ai =
	new HSPS::AdditiveIncrementalRegressionPDB(*sas_instance, stats);
      HSPS::ACF* cost = (opt_cost ?
			 (HSPS::ACF*)new HSPS::SASCostACF(*sas_instance) :
			 (HSPS::ACF*)new HSPS::UnitACF());
      HSPS::Heuristic* inc = (opt_pdb_inc ? prep.inconsistency() : 0);
      HSPS::MDDNode* sinc = (opt_pdb_inc ?
			     makeMDD(prep.inconsistency(),
				     sas_instance->atom_map_defined(),
				     sas_instance->atom_map_n()) : 0);
      std::cerr << "computing Additive Incremental PDB:s..." << std::endl;
      h_ai->compute_pdbs(sas_instance->goal_state, opt_pdb_size, opt_optimal,
			 opt_pdb_spanning_subset, opt_pdb_collapse, true,
			 *cost, sinc, inc, 0);
      // note: additive groups are already computed by compute_pdbs
      if (verbose_level > 1) {
	std::cerr << "PDB sets/Additive groups: ";
	h_ai->write_additive_groups(std::cerr);
	std::cerr << std::endl;
      }
      if (opt_save) {
	h_ai->write(std::cout);
	exit(0);
      }
      HSPS::MaxAddH* h_ma = h_ai->make_max_add();
      cost_est = new HSPS::BwdSASHAdapter(instance, *sas_instance, *h_ma);
    }

    else if (opt_pdb_ai) {
      HSPS::AdditiveIncrementalRegressionPDB* h_ai =
	new HSPS::AdditiveIncrementalRegressionPDB(*sas_instance, stats);
      HSPS::ACF* cost = (opt_cost ?
			 (HSPS::ACF*)new HSPS::SASCostACF(*sas_instance) :
			 (HSPS::ACF*)new HSPS::UnitACF());
      HSPS::Heuristic* inc = (opt_pdb_inc ? prep.inconsistency() : 0);
      HSPS::MDDNode* sinc = (opt_pdb_inc ?
			     makeMDD(prep.inconsistency(),
				     sas_instance->atom_map_defined(),
				     sas_instance->atom_map_n()) : 0);
      HSPS::CostTable* h_cw = new HSPS::CostTable(instance, stats);
      h_cw->compute_H1(*cost);
      std::cerr << "computing Additive Incremental PDB:s..." << std::endl;
      h_ai->compute_pdbs(sas_instance->goal_state, opt_pdb_size, opt_optimal,
			 opt_pdb_spanning_subset, opt_pdb_collapse, false,
			 *cost, sinc, inc, h_cw);
      // note: additive groups are already computed by compute_pdbs
      if (verbose_level > 1) {
	std::cerr << "PDB sets/Additive groups: ";
	h_ai->write_additive_groups(std::cerr);
	std::cerr << std::endl;
      }
      if (opt_save) {
	h_ai->write(std::cout);
	exit(0);
      }
      HSPS::MaxAddH* h_ma = h_ai->make_max_add();
      cost_est = new HSPS::BwdSASHAdapter(instance, *sas_instance, *h_ma);
    }

    else if (opt_AH) {
      std::cerr << "computing AH heuristic..." << std::endl;
      HSPS::AH* h = new HSPS::AH(instance, stats);
      if (opt_load_partition) {
	stats.start();
	HSPS::name_vec pnames(0, 0);
	HSPS::index_set_vec partition;
	reader->export_action_partitions(pnames, partition);
	if (partition.length() == 0) {
	  std::cerr << "warning: action partitioning not defined by domain"
		    << " - trying alternate load method..." << std::endl;
	  HSPS::name_vec anames(0, 0);
	  instance.action_names(anames);
	  reader->export_sets(anames, partition);
	}
	if (partition.length() == 0) {
	  std::cerr << "warning: no action partitioning loaded" << std::endl;
	}
	instance.remap_sets(partition, prep.action_map);
	if (opt_H1) {
	  if (opt_cost) {
	    h->compute_additive_H1(HSPS::CostACF(instance), partition);
	    if (opt_AH_max) h->compute_max_H1(HSPS::CostACF(instance));
	  }
	  else {
	    h->compute_additive_H1(HSPS::UnitACF(), partition);
	    if (opt_AH_max) h->compute_max_H1(HSPS::UnitACF());
	  }
	}
	else {
	  if (opt_cost) {
	    h->compute_additive_H2(HSPS::CostACF(instance), partition);
	    if (opt_AH_max) h->compute_max_H2(HSPS::CostACF(instance));
	  }
	  else {
	    h->compute_additive_H2(HSPS::UnitACF(), partition);
	    if (opt_AH_max) h->compute_max_H2(HSPS::UnitACF());
	  }
	}
	stats.stop();
      }
      else if (opt_ia_partition) {
	if (opt_cost)
	  h->compute_with_iterative_assignment(HSPS::CostACF(instance),
					       instance.goal_atoms,
					       !opt_H1,
					       opt_fpia,
					       opt_optimal);
	else
	  h->compute_with_iterative_assignment(HSPS::UnitACF(),
					       instance.goal_atoms,
					       !opt_H1,
					       opt_fpia,
					       opt_optimal);
	if (!opt_AH_max) h->disable_max_H();
      }
      else if (opt_bu_partition) {
	if (opt_cost) {
	  h->compute_bottom_up(HSPS::CostACF(instance), instance.goal_atoms);
	  if (opt_AH_max)
	    h->compute_max_H2(HSPS::UnitACF());
	}
	else {
	  h->compute_bottom_up(HSPS::UnitACF(), instance.goal_atoms);
	  if (opt_AH_max)
	    h->compute_max_H2(HSPS::UnitACF());
	}
      }
      else if (opt_new_decomposition) {
	if (opt_cost) {
	  h->compute_with_new_decomposition(HSPS::CostACF(instance),
					    instance.goal_atoms);
	  if (opt_AH_max) {
	    h->compute_max_H2(HSPS::CostACF(instance));
	  }
	}
	else {
	  h->compute_with_new_decomposition(HSPS::UnitACF(),
					    instance.goal_atoms);
	  if (opt_AH_max) {
	    h->compute_max_H2(HSPS::UnitACF());
	  }
	}
      }
      else if (opt_layered_partition) {
	if (opt_cost)
	  h->compute_with_layered_partitioning(HSPS::CostACF(instance),
					       instance.goal_atoms);
	else
	  h->compute_with_layered_partitioning(HSPS::UnitACF(),
					       instance.goal_atoms);
	if (!opt_AH_max) h->disable_max_H();
      }
      else if (opt_rnd_relevance_partition) {
	if (opt_cost)
	  h->compute_with_random_relevance_partitioning
	    (HSPS::CostACF(instance), instance.goal_atoms, rnd_source, !opt_H1);
	else
	  h->compute_with_random_relevance_partitioning
	    (HSPS::UnitACF(), instance.goal_atoms, rnd_source, !opt_H1);
	if (!opt_AH_max) h->disable_max_H();
      }
      else {
	if (opt_cost)
	  h->compute_with_relevance_partitioning(HSPS::CostACF(instance),
						 instance.goal_atoms);
	else
	  h->compute_with_relevance_partitioning(HSPS::UnitACF(),
						 instance.goal_atoms);
	if (!opt_AH_max) h->disable_max_H();
      }
      if (!stats.break_signal_raised()) {
	std::cerr << "heuristic computed in " << stats.time()
		  << " seconds" << std::endl;
      }
      if (opt_AHRC) {
	HSPS::CostTable* h_max = new HSPS::CostTable(instance, stats);
	if (opt_cost)
	  h_max->compute_H2(HSPS::UnitACF());
	else
	  h_max->compute_H2(HSPS::UnitACF());
	HSPS::Combine2ByRandomChoice* h_rc =
	  new HSPS::Combine2ByRandomChoice(instance, *h, *h_max,
					   alpha_AHRC, rnd_source);
	cost_est = h_rc;
      }
      else if (opt_structured) {
	HSPS::index_set invs;
	for (HSPS::index_type k = 0; k < instance.n_invariants(); k++)
	  if ((instance.invariants[k].lim == 1) &&
	      !instance.invariants[k].set.empty() &&
	      instance.invariants[k].exact)
	    invs.insert(k);
	if (invs.length() > 1) {
	  HSPS::CombineNByMax* h_max = new HSPS::CombineNByMax(instance);
	  for (HSPS::index_type k = 0; k < invs.length(); k++) {
	    HSPS::HX* hx =
	      new HSPS::HX(instance, *h, instance.invariants[invs[k]].set);
	    h_max->add(hx);
	  }
	  cost_est = h_max;
	}
	else if (invs.length() == 1) {
	  HSPS::HX* hx =
	    new HSPS::HX(instance, *h, instance.invariants[invs[0]].set);
	  cost_est = hx;
	}
	else {
	  cost_est = h;
	}
      }
      else if ((opt_fpia || opt_new_decomposition) && !opt_cost) {
	cost_est = new HSPS::RoundUp(instance, *h);
      }
      else {
	cost_est = h;
      }
    }

    else {
      std::cerr << "computing heuristic..." << std::endl;
      HSPS::CostTable* cost_tab = new HSPS::CostTable(instance, stats);
      if (opt_H1) {
	if (opt_temporal)
	  cost_tab->compute_H1(HSPS::MakespanACF(instance));
	else if (opt_sequential && opt_cost)
	  cost_tab->compute_H1(HSPS::CostACF(instance));
	else
	  cost_tab->compute_H1(HSPS::UnitACF());
      }
      else if (!opt_H0) {
	if (opt_temporal) {
	  cost_tab->compute_H2C(HSPS::MakespanACF(instance), opt_resource);
	}
	else if (opt_sequential) {
	  if (opt_H3) {
	    if (opt_cost)
	      cost_tab->compute_H3(HSPS::CostACF(instance));
	    else
	      cost_tab->compute_H3(HSPS::UnitACF());
	  }
	  else {
	    if (opt_cost)
	      cost_tab->compute_H2(HSPS::CostACF(instance));
	    else
	      cost_tab->compute_H2(HSPS::UnitACF());
	  }
	}
	else {
	  cost_tab->compute_H2C(HSPS::UnitACF(), false);
	}
      }
      if (!stats.break_signal_raised()) {
	std::cerr << "heuristic computed in " << stats.time()
		  << " seconds" << std::endl;
	if (opt_save) {
	  if (opt_pddl) {
	    cost_tab->write_pddl(std::cout, instance);
	  }
	  else {
	    std::cout << "heuristic table:" << std::endl;
	    cost_tab->write(std::cout);
	  }
	  if (!opt_resource) exit(0);
	}
      }
      cost_est = cost_tab;
    }

    if (opt_apply_invariants) {
      HSPS::RegressionInvariantCheck* ic =
	new HSPS::RegressionInvariantCheck(instance, *cost_est,
					   opt_verify_invariants);
      cost_est = ic;
    }

    if (opt_eval_cache) {
      HSPS::EvalActionCache* eac =
	new HSPS::EvalActionCache(instance, *cost_est);
      cost_est = eac;
    }

#ifdef APPLY_NCW_NOOP_TRICK
    if (opt_ncw && !stats.break_signal_raised()) {
      std::cerr << "computing n.c.w. sets..." << std::endl;
      stats.start();
      prep.compute_ncw_sets(*cost_est);
      stats.stop();
    }
#endif

    if (opt_resource && !opt_R0 && !stats.break_signal_raised()) {
      std::cerr << "computing resource estimators..." << std::endl;
      stats.start();
      HSPS::Heuristic* inc =
	(opt_rpdb && opt_pdb_inc ? prep.inconsistency() : 0);
      HSPS::MDDNode* sinc = (opt_rpdb && opt_pdb_inc ?
			     makeMDD(prep.inconsistency(),
				     sas_instance->atom_map_defined(),
				     sas_instance->atom_map_n()) : 0);
      for (HSPS::index_type k = 0; k < instance.n_resources(); k++) {
	if (opt_rpdb) {
	  HSPS::AdditiveIncrementalRegressionPDB* h_ai =
	    new HSPS::AdditiveIncrementalRegressionPDB(*sas_instance, stats);
	  HSPS::ResourceConsACF cost(instance, k);
	  HSPS::CostTable* h_cw = new HSPS::CostTable(instance, stats);
	  h_cw->compute_H1(cost);
	  std::cerr << "computing Additive Incremental PDB:s..." << std::endl;
	  h_ai->compute_pdbs(sas_instance->goal_state, opt_pdb_size,
			     opt_optimal, opt_pdb_spanning_subset,
			     opt_pdb_collapse, false, cost, sinc, inc, h_cw);
	  // note: additive groups are already computed by compute_pdbs
	  if (verbose_level > 1) {
	    std::cerr << "PDB sets/Additive groups: ";
	    h_ai->write_additive_groups(std::cerr);
	    std::cerr << std::endl;
	  }
	  HSPS::MaxAddH* h_ma = h_ai->make_max_add();
	  resource_est[k] =
	    new HSPS::BwdSASHAdapter(instance, *sas_instance, *h_ma);
	}
	else if (opt_RAH) {
	  HSPS::AH* rce = new HSPS::AH(instance, stats);
	  if (opt_load_partition) {
	    HSPS::name_vec pnames(0, 0);
	    HSPS::index_set_vec partition;
	    reader->export_action_partitions(pnames, partition);
	    if (partition.length() == 0) {
	      std::cerr << "warning: action partitioning not defined by domain"
			<< " - trying alternate load method..." << std::endl;
	      HSPS::name_vec anames(0, 0);
	      instance.action_names(anames);
	      reader->export_sets(anames, partition);
	    }
	    if (partition.length() == 0) {
	      std::cerr << "warning: no action partitioning loaded"
			<< std::endl;
	    }
	    instance.remap_sets(partition, prep.action_map);
	    if (opt_R2) {
	      rce->compute_additive_H2(HSPS::ResourceConsACF(instance, k), partition);
	      rce->compute_max_H2(HSPS::ResourceConsACF(instance, k));
	    }
	    else {
	      rce->compute_additive_H1(HSPS::ResourceConsACF(instance, k), partition);
	      // rce->compute_max_H1(HSPS::ResourceConsACF(instance, k));
	    }
	  }
	  else if (opt_ia_partition) {
	    rce->compute_with_iterative_assignment(HSPS::ResourceConsACF(instance, k),
						   instance.goal_atoms,
						   opt_R2,
						   opt_fpia,
						   opt_optimal);
	  }
	  else if (opt_new_decomposition) {
	    rce->compute_with_new_decomposition(HSPS::ResourceConsACF(instance, k),
						instance.goal_atoms);
	    if (opt_AH_max) {
	      rce->compute_max_H2(HSPS::ResourceConsACF(instance, k));
	    }
	  }
	  else if (opt_layered_partition) {
	    rce->compute_with_layered_partitioning(HSPS::ResourceConsACF(instance, k),
						   instance.goal_atoms);
	  }
	  else if (opt_rnd_relevance_partition) {
	    rce->compute_with_random_relevance_partitioning
	      (HSPS::ResourceConsACF(instance, k), instance.goal_atoms, rnd_source,
	       opt_R2);
	  }
	  else {
	    rce->compute_with_relevance_partitioning(HSPS::ResourceConsACF(instance, k),
						     instance.goal_atoms);
	  }
	  resource_est[k] = rce;
	}
	else if (opt_R2) {
	  HSPS::CostTable* rce = new HSPS::CostTable(instance, stats);
	  rce->compute_H2(HSPS::ResourceConsACF(instance, k));
	  if (verbose_level > 2) {
	    std::cout << "resource " << instance.resources[k].name
		      << " estimator:" << std::endl;
	    rce->write(std::cout);
	  }
	  resource_est[k] = rce;
	}
	else {
	  HSPS::CostTable* rce = new HSPS::CostTable(instance, stats);
	  rce->compute_H1(HSPS::ResourceConsACF(instance, k));
	  if (verbose_level > 2) {
	    std::cout << "resource " << instance.resources[k].name
		      << " estimator:" << std::endl;
	    rce->write(std::cout);
	  }
	  resource_est[k] = rce;
	}
      }
      stats.stop();
    }

    // #define COMPARE_AH
#ifdef COMPARE_AH
    HSPS::AH* ah = new HSPS::AH(instance, stats);
    HSPS::name_vec anames(0, 0);
    instance.action_names(anames);
    HSPS::index_set_vec partition;
    reader->export_sets(anames, partition);
    ah->compute_additive_H1(HSPS::UnitACF(), partition);
    ah->compute_max_H1(HSPS::UnitACF());
    HSPS::InvariantPostCheck* ic = new HSPS::InvariantPostCheck(instance, *ah);
    HSPS::CompareEval* ce = new HSPS::CompareEval(instance, *cost_est, *ic);
    ce->set_trace_level(0);
    cost_est = ce;
#endif

    if (opt_load_compare) {
      HSPS::CostTable* lc = new HSPS::CostTable(instance, stats);
      std::cerr << "loading heuristic table (" << reader->h_table.length()
		<< " entries)..." << std::endl;
      reader->export_heuristic(instance, prep.atom_map, true, *lc);
      HSPS::CompareEval* ce = new HSPS::CompareEval(instance, *cost_est, *lc);
      ce->set_maximal_heuristic_value(opt_compare_max);
      HSPS::CompareEval::lower = 0;
      HSPS::CompareEval::higher = 0;
      HSPS::CompareEval::equal = 0;
      ce->set_trace_level(0);
      cost_est = ce;
    }

    if (!stats.break_signal_raised()) { // after H^2 and n.c.w.
      if (opt_save && opt_resource) {
	for (HSPS::index_type k = 0; k < instance.n_resources(); k++) {
	  if (opt_pddl) {
	    std::cout << ";; resource " << instance.resources[k].name
		      << std::endl;
	    ((HSPS::CostTable*)resource_est[k])->write_pddl(std::cout, instance);
	  }
	  else {
	    std::cout << "resource " << instance.resources[k].name
		      << " estimator:" << std::endl;
	    ((HSPS::CostTable*)resource_est[k])->write(std::cout);
	  }
	}
	exit(0);
      }

      if (opt_look_ahead) {
	HSPS::StateFactory* ss = 0;
	if (opt_temporal) {
	  HSPS::RegressionResourceState* rcs =
	    (opt_resource ?
	     new HSPS::RegressionResourceState(instance, resource_est) : 0);
	  ss = new HSPS::ApxTemporalRegState(instance, *cost_est, rcs, 3);
	}
	else if (opt_sequential) {
	  HSPS::ACF* acf =
	    (opt_cost ? (HSPS::ACF*)new HSPS::CostACF(instance) : (HSPS::ACF*)new HSPS::UnitACF());
	  HSPS::RegressionResourceState* rcs =
	    (opt_resource ?
	     new HSPS::RegressionResourceState(instance, resource_est) : 0);
	  ss = new HSPS::ApxSeqRegState(instance, *cost_est, *acf, rcs, 3);
	}
	else {
	  ss = new HSPS::ApxParaRegState(instance, *cost_est, 3);
	}
	cost_est = new HSPS::LookAheadAB(instance, *ss, look_ahead_depth, look_ahead_stats);
      }

      HSPS::State* search_root = 0;

      if (opt_deadline) {
	cost_limit = 0;
	for (HSPS::index_type k = 0; k < instance.n_atoms(); k++)
	  if (instance.atoms[k].goal) {
	    if (INFINITE(instance.atoms[k].goal_t)) {
	      std::cerr << "error: goal " << instance.atoms[k].name
			<< " does not have a deadline" << std::endl;
	      exit(255);
	    }
	    if (instance.atoms[k].goal_t < 0) {
	      std::cerr << "error: goal " << instance.atoms[k].name
			<< " has a negative deadline" << std::endl;
	      exit(255);
	    }
	    cost_limit = MAX(instance.atoms[k].goal_t, cost_limit);
	  }
	HSPS::index_set g;
	HSPS::index_set d;
	for (HSPS::index_type k = 0; k < instance.n_atoms(); k++)
	  if (instance.atoms[k].goal) {
	    if (instance.atoms[k].goal_t == cost_limit) {
	      g.insert(k);
	    }
	    else {
	      HSPS::Instance::Action& a =
		instance.new_action(instance.atoms[k].name);
	      a.pre.insert(k);
	      a.sel = false;
	      a.dur = (cost_limit - instance.atoms[k].goal_t);
	      d.insert(a.index);
	    }
	  }
	assert(!g.empty());
	HSPS::RegressionResourceState* rcs =
	  (opt_resource ?
	   new HSPS::RegressionResourceState(instance, resource_est) : 0);
	search_root = new HSPS::TemporalRSRegState(instance, *cost_est, g, d, rcs);
      }
      else if (opt_temporal) {
	HSPS::RegressionResourceState* rcs =
	  (opt_resource ?
	   new HSPS::RegressionResourceState(instance, resource_est) : 0);
	if (opt_apx) {
	  search_root = new HSPS::ApxTemporalRegState
	    (instance, *cost_est, instance.goal_atoms, rcs, apx_limit);
	}
	else if (opt_apply_cuts) {
	  search_root = new HSPS::TemporalRSRegState
	    (instance, *cost_est, instance.goal_atoms, rcs);
	}
	else {
	  search_root = new HSPS::TemporalRegState
	    (instance, *cost_est, instance.goal_atoms, rcs);
	}
      }
      else if (opt_lds) {
	if (reader->n_plans() < 1) {
	  std::cerr << "error: least-discrepancy branch-and-bound search requires at least one input plan" << std::endl;
	  exit(255);
	}
	HSPS::Schedule* best = 0;
	for (HSPS::index_type k = 0; k < reader->n_plans(); k++) {
	  HSPS::Schedule* plan = new HSPS::Schedule(instance);
	  reader->export_plan(k, instance, prep.action_map, *plan);
	  if (best == 0) {
	    best = plan;
	  }
	  else if (plan->cost() < best->cost()) {
	    delete best;
	    best = plan;
	  }
	  else {
	    delete plan;
	  }
	}
	HSPS::ActionSequence* seq = new HSPS::ActionSequence();
	best->output(*seq);
	HSPS::ACF* acf =
	  (opt_cost ? (HSPS::ACF*)new HSPS::CostACF(instance) : (HSPS::ACF*)new HSPS::UnitACF());
	search_root = new HSPS::LDSeqRegState
	  (instance, *cost_est, *acf, instance.goal_atoms, *seq);
	cost_limit = best->cost();
      }
      else if (opt_sequential) {
	HSPS::ACF* acf =
	  (opt_cost ? (HSPS::ACF*)new HSPS::CostACF(instance) : (HSPS::ACF*)new HSPS::UnitACF());
	HSPS::RegressionResourceState* rcs =
	  (opt_resource ? new HSPS::RegressionResourceState(instance, resource_est) : 0);
	if (opt_apx) {
	  search_root = new HSPS::ApxSeqRegState
	    (instance, *cost_est, *acf, instance.goal_atoms, rcs, apx_limit);
	}
	else if (opt_apply_cuts) {
	  search_root = new HSPS::SeqCRegState
	    (instance, *cost_est, *acf, instance.goal_atoms, rcs);
	}
	else {
	  search_root = new HSPS::SeqRegState
	    (instance, *cost_est, *acf, instance.goal_atoms, rcs);
	}
      }
      else {
	if (opt_apply_cuts) {
	  search_root = new HSPS::ParaRSRegState
	    (instance, *cost_est, instance.goal_atoms);
	}
	else {
	  search_root = new HSPS::ParaRegState
	    (instance, *cost_est, instance.goal_atoms);
	}
      }

      std::cerr << "search root: " << *search_root
		<< " (est. cost: " << search_root->est_cost() << ")"
		<< std::endl;
      std::cerr << "estimated goal cost: "
		<< cost_est->eval(instance.goal_atoms)
		<< std::endl;

      if (opt_explore_to_depth || opt_explore_to_bound) {
	HSPS::TreeStatistics tree_stats;
	if (node_limit > 0) tree_stats.node_limit = node_limit;
	HSPS::Tree* search_tree =
	  new HSPS::Tree(*search_root, stats, tree_stats);
	stats.start();
	if (opt_explore_to_depth) {
	  for (HSPS::index_type k = 1; k <= depth_limit; k++) {
	    std::cerr << "exploring to depth " << k << "..." << std::endl;
	    search_tree->build(k);
	    std::cerr << stats << std::endl;
	  }
	}
	else if (opt_explore_to_bound) {
	  std::cerr << "exploring to bound " << cost_limit << "..."
		    << std::endl;
	  search_tree->build(cost_limit);
	  std::cerr << stats << std::endl;
	}
	stats.stop();
	std::cerr << stats << std::endl;
	tree_stats.write(std::cout);
	std::ofstream st_out("search_tree.dot");
	search_tree->write_dot(st_out, opt_print_solution_only,
			       opt_no_print_solution);
	st_out.close();
	return 0;
      }

      HSPS::SearchAlgorithm* search = 0;

      HSPS::SearchResult* result = &store;
      if (opt_bb) {
	store.set_stop_condition(HSPS::Result::stop_at_all_optimal);
	result = new HSPS::StoreMinCost(store);
      }
      else if (opt_all_different) {
	result = new HSPS::StoreDistinct(instance, store);
      }

      if (opt_find_n) {
	store.set_stop_condition(HSPS::Result::stop_at_nth);
	store.set_n_to_find(n_to_find);
      }
      else if (opt_find_all) {
	store.set_stop_condition(HSPS::Result::stop_at_all_optimal);
      }
      else if (opt_exhaustive) {
	store.set_stop_condition(HSPS::Result::stop_at_all);
      }
      else {
	store.set_stop_condition(HSPS::Result::stop_at_first);
      }

      if (opt_bfs) {
	if (opt_bfs_px) {
	  std::cerr << "using partial expansion A*..." << std::endl;
	  search = new HSPS::BFS_PX(stats, *result, px_threshold);
	}
	else {
	  std::cerr << "using A*..." << std::endl;
	  search = new HSPS::BFS(stats, *result);
	}
      }
      else if (opt_bfida) {
	std::cerr << "using BFIDA..." << std::endl;
	search = new HSPS::BFIDA(stats, *result, 10007);
      }
      else if (opt_dfs) {
	std::cerr << "using DFS (bound = " << cost_limit << ")..."
		  << std::endl;
	HSPS::DFS* df_search = new HSPS::DFS(stats, *result);
	df_search->set_cycle_check(opt_cc);
	df_search->set_upper_bound(cost_limit);
	search = df_search;
      }
      else if (opt_bb) {
	std::cerr << "using DFS branch-and-bound (initial bound = "
		  << cost_limit << ")..." << std::endl;
	HSPS::DFS_BB* bb_search = 0;
	if (opt_tt) {
	  tt = new HSPS::HashTable(opt_tt_size);
	  bb_search = new HSPS::DFS_BB(stats, *result, tt);
	}
	else {
	  bb_search = new HSPS::DFS_BB(stats, *result);
	}
	bb_search->set_cycle_check(opt_cc);
	bb_search->set_upper_bound(cost_limit);
	search = bb_search;
      }
      else if (opt_idao) {
	std::cerr << "using IDAO*..." << std::endl;
	tt = new HSPS::HashTable(opt_tt_size);
	result = new HSPS::ApxResult();
	HSPS::IDAO* idao_search = new HSPS::IDAO(stats, *result, tt);
	idao_search->set_cycle_check(opt_cc);
	search = idao_search;
      }
      else if (opt_tt) {
	std::cerr << "using IDA* with transposition table..." << std::endl;
	tt = new HSPS::HashTable(opt_tt_size);
	HSPS::IDA* ida_search = new HSPS::IDA(stats, *result, tt);
	ida_search->set_cycle_check(opt_cc);
	if (iteration_limit > 0)
	  ida_search->set_iteration_limit(iteration_limit);
	search = ida_search;
      }
      else {
	std::cerr << "using IDA*..." << std::endl;
	HSPS::IDA* ida_search = new HSPS::IDA(stats, *result);
	ida_search->set_cycle_check(opt_cc);
	if (iteration_limit > 0)
	  ida_search->set_iteration_limit(iteration_limit);
	search = ida_search;
      }

      search->set_problem_name(instance.name);

      if (opt_limit_to_length) {
	if (opt_sequential) {
	  cost_limit = reader->serial_length;
	}
	else {
	  cost_limit = reader->parallel_length;
	}
      }
      if (FINITE(cost_limit)) search->set_cost_limit(cost_limit);
      if (node_limit > 0) search->set_node_limit(node_limit);

      root_est_cost = search_root->est_cost();
      std::cerr << "searching..." << std::endl;
      solution_cost = search->start(*search_root);

      if (opt_bfs &&
	  search->solved() &&
	  (opt_find_all || opt_exhaustive) &&
	  !opt_bfs_write_graph && !opt_bfs_write_stats &&
	  !stats.break_signal_raised()) {
	HSPS::NodeSet& g = ((HSPS::BFS*)search)->state_space();
	g.mark_solved();
	store.reset();
	HSPS::node_vec& r = g.root_nodes();
	for (HSPS::index_type k = 0; k < r.length(); k++) {
	  std::cerr << "extracting solution paths from root node #"
		    << r[k]->id << "..." << std::endl;
	  HSPS::NodeSetSearchState ns(r[k]);
	  HSPS::DFS s(stats, *result);
	  if (opt_find_all) {
	    s.set_upper_bound(solution_cost);
	  }
	  else {
	    s.set_upper_bound(POS_INF);
	  }
	  s.set_cycle_check(true);
	  s.start(ns);
	}
      }

      if (!stats.break_signal_raised()) {
	std::cerr << "search complete (" << stats << ")" << std::endl;
      }

      solved = search->solved();
      optimally = search->optimal();
      if (!search->optimal())
	solution_cost = search->cost();

      if (opt_bfs && opt_bfs_write_graph) {
	HSPS::NodeSet& g = ((HSPS::BFS*)search)->state_space();
	g.mark_solved();
	g.write_graph(std::cout);
      }
      if (opt_bfs && opt_bfs_write_stats) {
	HSPS::NodeSet& g = ((HSPS::BFS*)search)->state_space();
	g.mark_solved();
	g.write_short(std::cout, instance.name);
      }

      if (solved && opt_temporal && opt_post_op) {
	std::cerr << "post-optimizing:" << std::endl;
	optimally = false;

	// restore durations
	instance.set_durations(saved_dur);

	// recompute heuristic
	std::cerr << "recomputing heuristic..." << std::endl;
	delete cost_est;
	HSPS::CostTable* cost_tab = new HSPS::CostTable(instance, stats);
	cost_tab->compute_H2C(HSPS::MakespanACF(instance), opt_resource);
	cost_est = cost_tab;

	// construct new search root and run DFS-BB
	if (!stats.break_signal_raised()) {
	  HSPS::RegressionResourceState* rcs =
	    (opt_resource?
	     new HSPS::RegressionResourceState(instance, resource_est) : 0);
	  if (opt_apply_cuts)
	    search_root =
	      new HSPS::TemporalRSRegState(instance, *cost_est,
					   instance.goal_atoms, rcs);
	  else
	    search_root =
	      new HSPS::TemporalRegState(instance, *cost_est,
					 instance.goal_atoms, rcs);
	  store.clear();
	  HSPS::StoreMinCost result(store);
	  store.set_stop_condition(HSPS::Result::stop_at_all_optimal);
	  std::cerr << "searching..." << std::endl;
	  HSPS::DFS_BB op_search(stats, result);
	  op_search.set_cycle_check(opt_cc);
	  NTYPE new_solution_cost =
	    op_search.start(*search_root, solution_cost);
	  std::cerr << "final cost: " << new_solution_cost
		    << " (improvement: "
		    << solution_cost - new_solution_cost
		    << " (abs.) "
		    << (solution_cost - new_solution_cost)/new_solution_cost
		    << " (rel.), " << stats << ")" << std::endl;
	  solution_cost = new_solution_cost;
	  optimally = op_search.optimal();
	}
      }
    }
  }

  if (opt_validate) {
    for (HSPS::index_type p = 0; p < store.n_solutions(); p++) {
      HSPS::Schedule* plan = store.plan(p);
      if (verbose_level > 0) {
	plan->write(std::cerr);
      }
      plan->set_trace_level(verbose_level);
      HSPS::ExecTrace* trace = new HSPS::ExecTrace(instance);
      HSPS::ExecErrorSet* errors = new HSPS::ExecErrorSet();
      std::cerr << "simulating plan " << p << "..." << std::endl;
      bool ok = plan->simulate(trace, errors, false);
      if (ok) {
	std::cout << "plan " << p << " ok" << std::endl;
	if (instance.n_resources() > 0) {
	  HSPS::amt_vec peak;
	  trace->peak_resource_use(peak);
	  for (HSPS::index_type k = 0; k < instance.n_resources(); k++) {
	    NTYPE avail =
	      (opt_unlimited ? saved_res[k] : instance.resources[k].init);
	    NTYPE ratio = (peak[k] / avail);
	    std::cout << instance.resources[k].name
		      << '\t' << peak[k]
		      << '\t' << avail
		      << '\t' << PRINT_NTYPE(ratio*100)
		      << std::endl;
	  }
	}
      }
      else {
	std::cerr << "plan " << p << " failed" << std::endl;
	std::cerr << "errors:";
	errors->write(std::cerr);
	std::cerr << std::endl << "execution trace:" << std::endl;
	trace->write(std::cerr);
      }
    }
  }

  if (opt_gantt) {
    for (HSPS::index_type p = 0; p < store.n_solutions(); p++) {
      HSPS::Schedule* plan = store.plan(p);
      std::ostringstream fname;
      fname << "plan" << p << ".epic";
      std::ofstream plan_out(fname.str().c_str());
      plan->writeGantt(plan_out);
      plan_out.close();
    }
  }

  if (opt_ipc) {
    std::cout << "; Time " << HSPS::Stopwatch::seconds() << std::endl;
    std::cout << "; ParsingTime " << parse_stats.total_time() << std::endl;
    if (solved) {
      if (opt_sequential) {
	if (opt_cost) {
	  std::cout << "; NrActions " << std::endl;
	  std::cout << "; MakeSpan " << std::endl;
	  std::cout << "; MetricValue " << PRINT_NTYPE(solution_cost)
		    << std::endl;
	}
	else {
	  std::cout << "; NrActions " << solution_cost << std::endl;
	  std::cout << "; MakeSpan " << std::endl;
	  std::cout << "; MetricValue " << std::endl;
	}
      }
      else {
	std::cout << "; NrActions " << std::endl;
	std::cout << "; MakeSpan " << PRINT_NTYPE(solution_cost)
		  << std::endl;
	std::cout << "; MetricValue " << std::endl;
      }
      HSPS::PrintIPC print_plan(instance, std::cout);
      if (opt_temporal) print_plan.set_epsilon(epsilon, false, opt_strict_ipc);
      store.output(print_plan);

      if (opt_path && opt_write_graphs) {
	for (HSPS::index_type p = 0; p < store.n_solutions(); p++) {
	  HSPS::Schedule* plan = store.plan(p);
	  HSPS::State* path_end_state = store.solution(p);
	  std::ostringstream fname;
	  fname << "path" << p << ".dot";
	  std::ofstream pg_out(fname.str().c_str());
	  pg_out << "digraph path" <<  p << " {" << std::endl;
	  pg_out << "node [width=0,height=0];" << std::endl
		 << "rankdir=LR;" << std::endl;
	  path_end_state->write_path_as_graph(pg_out);
	  pg_out << "}" << std::endl;
	  pg_out.close();
	}
      }
    }
    else {
      std::cout << "; Not Solved (" << stats.flags() << ")" << std::endl;
    }

    std::cout << "; (:heuristic (";
    for (HSPS::index_type k = 0; k < instance.goal_atoms.length(); k++)
      std::cout << instance.atoms[instance.goal_atoms[k]].name;
    std::cout << ") " << stats.max_lower_bound() << ")" << std::endl;
  }

  else {
    if (verbose_level > 0) {
      if (solved) {
	std::cout << "solution cost: " << solution_cost;
	if (optimally)
	  std::cout << " (optimal)";
	else
	  std::cout << " (upper bound)";
	std::cout << std::endl;
	std::cout << store.n_solutions() << " solutions";
	if (opt_all_different) {
	  std::cout << " (" << HSPS::StoreDistinct::n_discarded
		    << " equivalent solutions discarded)";
	}
	std::cout << std::endl;
      }
      else {
	std::cout << "no solution found" << std::endl;
      }
      stats.print_total(std::cout);
      stats.print(std::cout, "(search) ");
      std::cerr << "highest lower bound " << stats.max_lower_bound()
		<< " proven at " << stats.nodes_at_max_lower_bound()
		<< " nodes" << std::endl;
      if (opt_look_ahead) {
	std::cout << "look ahead searches: " << look_ahead_stats << std::endl;
      }
      double total_t = HSPS::Stopwatch::seconds();
      std::cout << total_t << " seconds total ("
		<< total_t - stats.total_time()
		<< " sec. not accounted for)" << std::endl;
      std::cout << "peak memory use: " << stats.peak_memory() << "k"
		<< ", peak stack size: " << stats.peak_stack_size() << "k"
#ifdef RSS_FROM_PSINFO
		<< ", peak size: " << stats.peak_total_size() << "k"
#endif
#ifdef PROC_EXTRA_STATS
		<< ", page faults: " << HSPS::Stopwatch::page_fault_count
		<< ", swaps: " << HSPS::Stopwatch::swap_count
#endif
		<< std::endl;
      if (opt_load_compare) {
	HSPS::count_type n = HSPS::CompareEval::lower +
	  HSPS::CompareEval::higher +
	  HSPS::CompareEval::equal;
	std::cout << "Alt. H: " << HSPS::CompareEval::lower << " lower ("
		  << (HSPS::CompareEval::lower/(double)n)*100 << "%), "
		  << HSPS::CompareEval::higher << " higher ("
		  << (HSPS::CompareEval::higher/(double)n)*100 << "%), "
		  << HSPS::CompareEval::equal << " equal ("
		  << (HSPS::CompareEval::equal/(double)n)*100 << "%)"
		  << std::endl;
      }
#ifdef AH_EXTRA_STATS
      if (opt_AH) {
	std::cout << "AH: " << HSPS::AH::Hmax_wins << " Hmax, "
		  << HSPS::AH::Hsum_wins << " Hsum, "
		  << HSPS::AH::draws << " equal ("
		  << (HSPS::AH::Hsum_wins/((double)(HSPS::AH::Hsum_wins + HSPS::AH::Hmax_wins + HSPS::AH::draws)))*100
		  << "% improved, "
		  << (HSPS::AH::draws/((double)(HSPS::AH::Hsum_wins + HSPS::AH::Hmax_wins + HSPS::AH::draws)))*100
		  << "% equal)" << std::endl;
      }
#endif
      if (opt_tt || opt_idao) {
	assert(tt);
	std::cout << "transposition table stats: TUF = " << tt->TUF()
		  << ", HCF: " << tt->HCF() << std::endl;
      }
#ifdef SEARCH_EXTRA_STATS
      std::cout << "search space extra stats:"
		<< ' ' << (HSPS::rminx_size/(double)HSPS::rminx_count)
		<< ' ' << (HSPS::rminc_succ_size_ratio/(double)HSPS::rminc_count)
		<< ' ' << (HSPS::rminx_succ/(double)HSPS::rminx_count)
		<< ' ' << (HSPS::trie_count/(double)HSPS::rminx_count)
		<< ' ' << (HSPS::trie_count == 0 ? 0 :
			   (HSPS::tries_applicable/(double)HSPS::trie_count))
		<< ' ' << (HSPS::trie_count == 0 ? 0 :
			   (HSPS::tries_within_bound/(double)HSPS::trie_count))
		<< std::endl;
#endif
    }

    else {
      if (opt_pddl) {
	std::cout << ";; stats: ";
      }
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
#ifdef PROC_EXTRA_STATS
		<< ' ' << HSPS::Stopwatch::page_fault_count
		<< ' ' << HSPS::Stopwatch::swap_count
#endif
		<< ' ' << stats.complete_iterations()
		<< ' ' << stats.nodes_at_max_lower_bound()
		<< ' ' << stats.time()
#ifdef EVAL_EXTRA_STATS
		<< ' ' << HSPS::CostNode::eval_count
		<< ' ' << HSPS::CostNode::eval_rec_count
#endif
#ifdef AH_EXTRA_STATS
		<< ' ' << HSPS::AH::Hsum_wins/((double)(HSPS::AH::Hsum_wins + HSPS::AH::Hmax_wins + HSPS::AH::draws))
		<< ' ' << HSPS::AH::draws/((double)(HSPS::AH::Hsum_wins + HSPS::AH::Hmax_wins + HSPS::AH::draws))
#endif
#ifdef SEARCH_EXTRA_STATS
		<< ' ' << (HSPS::rminx_size/(double)HSPS::rminx_count)
		<< ' ' << (HSPS::rminc_succ_size_ratio/(double)HSPS::rminc_count)
		<< ' ' << (HSPS::rminx_succ/(double)HSPS::rminx_count)
		<< ' ' << (HSPS::trie_count/(double)HSPS::rminx_count)
		<< ' ' << (HSPS::trie_count == 0 ? 0 :
			   (HSPS::tries_applicable/(double)HSPS::trie_count))
		<< ' ' << (HSPS::trie_count == 0 ? 0 :
			   (HSPS::tries_within_bound/(double)HSPS::trie_count))
#endif
		<< std::endl;
    }

    if (solved && opt_print_plan) {
      if (opt_pddl) {
	HSPS::PrintPDDL print_plan(instance, std::cout);
	store.output(print_plan);
      }
      else {
	for (HSPS::index_type p = 0; p < store.n_solutions(); p++) {
	  HSPS::Schedule* plan = store.plan(p);
	  HSPS::State* path_end_state = store.solution(p);

	  std::cout << "plan #" << p << ":" << std::endl;
	  HSPS::Print print_plan(instance, std::cout);
	  plan->output(print_plan);

	  if (opt_pop) {
	    std::cout << "plan #" << p << " as POP:" << std::endl;
	    HSPS::Schedule* plan = store.plan(p);
	    HSPS::SafePOP* pop =
	      new HSPS::SafePOP(instance, plan->plan_steps());
	    pop->enforce_min_durations();
	    pop->enforce_max_durations();
	    pop->write(std::cout);
	  }

	  if (opt_path) {
	    std::cout << "plan #" << p << " solution path:" << std::endl;
	    path_end_state->write_path(std::cout);
	    if (opt_write_graphs) {
	      std::ostringstream fname;
	      fname << "path" << p << ".dot";
	      std::ofstream pg_out(fname.str().c_str());
	      pg_out << "digraph path" <<  p << " {" << std::endl;
	      pg_out << "node [width=0,height=0];" << std::endl
		     << "rankdir=LR;" << std::endl;
	      path_end_state->write_path_as_graph(pg_out);
	      pg_out << "}" << std::endl;
	      pg_out.close();
	    }
	  }
	}
      }
    }

    if (opt_pddl) {
      std::cout << "(:heuristic (";
      for (HSPS::index_type k = 0; k < instance.goal_atoms.length(); k++)
	std::cout << instance.atoms[instance.goal_atoms[k]].name;
      std::cout << ") " << stats.max_lower_bound() << ")" << std::endl;
    }
  }

  return 0;
}
