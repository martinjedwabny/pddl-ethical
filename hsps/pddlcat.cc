
#include "parser.h"
#include "reduce.h"
#include "random.h"
#include "pop.h"
#include "plans.h"
#include "pdb_construction.h"
#include "simplify.h"
#include "enumerators.h"
#include "logic.h"
#include "mdd.h"
#include "ida.h"
#include "path.h"

#include <fstream>
#include <sstream>

HSPS::StringTable symbols(50, HSPS::lowercase_map);

void print_help(const char* argv0) {
  std::cout << argv0 << " [command] [option*] <file>*" << std::endl;

  std::cout << std::endl << "domain/problem commands:" << std::endl;
  std::cout << " -echo:      write domain and problem (default)"
	    << std::endl;
  std::cout << " -ins:       instantiate (and write instantiated output)"
	    << std::endl;
  std::cout << " -prep:      preprocess (implies instantiation)"
	    << std::endl;
  std::cout << " -rel:       compute irrelevant atoms (implies -prep unless -no-prep)"
	    << std::endl;
  std::cout << " -verify:    verify invariants/remove unverified (implies -prep unless -no-prep)"
	    << std::endl;
  std::cout << " -find:      find at-most-1/exactly-1 invariants (implies -verify unless -no-verify)"
	    << std::endl;
  std::cout << " -landmark:  find/write landmark atoms (implies -prep unless -no-prep)"
	    << std::endl;
  std::cout << " -redop:     compute reduced action set (implies -prep unless -no-prep)"
	    << std::endl;
  std::cout << " -sas:       convert to SAS domain/problem (implies -prep unless -no-prep)"
	    << std::endl;
  std::cout << " -decide <V>: create decision problem (n.b. > V; n.b. >= V if -decide-eq; only if -soft)"
	    << std::endl;

  std::cout << std::endl << "domain/problem transformations:" << std::endl;
  std::cout << " -untype     remove :types (replace by predicates)"
	    << std::endl;
  std::cout << " -maximize   convert :metric to 'maximize'"
	    << std::endl;
  std::cout << " -minimize   convert :metric to 'minimize'"
	    << std::endl;
  std::cout << " -compile    compile away certain domain/problem features"
	    << std::endl;
  std::cout << " -simplify   -echo: simplify :metric expression"
	    << std::endl
	    << "             -sas: simplify instantiated problem"
	    << std::endl;

  std::cout << std::endl << "plan commands:" << std::endl;
  std::cout << " -val:       validate input plan(s)"
	    << std::endl;
  std::cout << " -diff[1,2]: \"diff\" input plans (output as PDDL3 constraints)"
	    << std::endl;
  std::cout << " -pop:       convert input plan(s) to partial order"
	    << std::endl;
  std::cout << " -gantt:     write Gantt charts for input plan(s)"
	    << std::endl;

  std::cout << std::endl << "input options:" << std::endl;
  std::cout << " -soft       recognize preferences (limited functionality)"
	    << std::endl;
  std::cout << " -require <tag>  remove DKEL items without :tag <tag>"
	    << std::endl;
  std::cout << " -exclude <tag>  remove DKEL items with :tag <tag>"
	    << std::endl;
  std::cout << " -exclude all  remove all DKEL items"
	    << std::endl;

  std::cout << std::endl << "preprocessing options (instantiated problem only):" << std::endl;
  std::cout << " -remove     remove irrelevant atoms (implies -rel unless -no-rel)"
	    << std::endl;
  std::cout << " -reduce     reduce action set (implies -prep unless -no-prep)"
	    << std::endl;
  std::cout << " -extend     extend goal with implied atoms"
	    << std::endl
	    << "             (implies -prep unless -no-prep; depends on invariants)"
	    << std::endl;
  std::cout << " -reverse    reverse instance (implies -prep unless -no-prep)"
	    << std::endl;
  std::cout << " -round-up   round durations up to nearest integer"
	    << std::endl;
  std::cout << " -round-down round durations down to nearest integer"
	    << std::endl;
  std::cout << " -round      round durations to nearest integer"
	    << std::endl;
  std::cout << " -unit       assign unit duration and cost to all actions"
	    << std::endl;
  std::cout << " -use-strict-borrow    use strict definition when identifying resource use"
	    << std::endl;
  std::cout << " -use-extended-borrow  use extended definition when identifying resource use"
	    << std::endl;
  std::cout << " -compose <N>  create composite resources of size N"
	    << std::endl;

  std::cout << std::endl << "RedOp options:" << std::endl;
  std::cout << " -bound <N>  set bound for replacement search (default = 3)"
	    << std::endl;
  std::cout << " -bound inf  unbounded replacement search"
	    << std::endl;
  std::cout << " -rep        output :replaceable items only"
	    << std::endl;

  std::cout << std::endl << "SAS options:" << std::endl;
  std::cout << " -sas-min    minimize action set (implies -sas)"
	    << std::endl;
  std::cout << " -sas-goal   reduce SAS instance to variables with goal values"
	    << std::endl;
  std::cout << " -sas-extend extend STRIPS-to-SAS map by computed implications"
	    << std::endl;
  std::cout << " -sas-id     find/write independent and determining variable sets"
	    << std::endl;
  std::cout << " -strips     convert SAS instance back to STRIPS"
	    << std::endl;
  std::cout << " -sas-cost   write action costs"
	    << std::endl;
  std::cout << " -sas-info   compute/write extra info"
	    << std::endl;

  std::cout << std::endl << "plan options:" << std::endl;
  std::cout << " -plan <K>   select input plan #K (default: process all)"
	    << std::endl;
  std::cout << " -seq        treat input plan(s) as sequential"
	    << std::endl;
  std::cout << " -pop-dmin   enforce minimum duration constraints in POP"
	    << std::endl;
  std::cout << " -pop-dmax   enforce maximum duration constraints in POP"
	    << std::endl;

  std::cout << std::endl << "print options:" << std::endl;
  std::cout << " -print|-p   write domain/problem in PDDL/DKEL/SAS"
	    << std::endl;
  std::cout << " -add|-a     write only added info in PDDL/DKEL"
	    << std::endl;
  std::cout << " -count|-c   write summary (# of atoms/actions/...) only"
	    << std::endl;
  std::cout << " -dump |-d   write domain and problem in debug format"
	    << std::endl;
  std::cout << " -no-domain  don't write domain (only problem)"
	    << std::endl;
  std::cout << " -no-problem don't write problem (only domain)"
	    << std::endl;
  std::cout << " -no-extra   don't write extra elements (plan, heuristic table, etc.)"
	    << std::endl;

  std::cout << std::endl << "PDDL options:" << std::endl;
  std::cout << " -no-dkel    don't write DKEL elements" << std::endl;
  std::cout << " -no-metric  don't write problem :metric" << std::endl;
  std::cout << " -no-pddl2   don't write PDDL2 elements" << std::endl;
  std::cout << " -no-pddl3   don't write PDDL3 elements" << std::endl;
  std::cout << " -pddl1      don't write any PDDL2 or PDDL3 elements"
	    << std::endl;
  std::cout << " -no-negation  write positive preconditons/goal (instantiated problem only)"
	    << std::endl;
  std::cout << " -write-nary write n-ary '+' and '*'"
	    << std::endl;
  std::cout << " -write-parameters write :parameters even when empty"
	    << std::endl;
  std::cout << " -write-requirements write :requirements in domain"
	    << std::endl;

  std::cout << std::endl << "misc. options:" << std::endl;
  std::cout << " -no-init    ignore initial atoms (-landmark, -diff)"
	    << std::endl;
  std::cout << " -no-goal    ignore goal atoms (-landmark, -diff)"
	    << std::endl;
  std::cout << " -seq        assume sequential planning problem (-rel, -ins + -g, -pop)"
	    << std::endl;
  std::cout << " -time       assume temporal planning problem (-rel, -ins + -g, -pop)"
	    << std::endl;
  std::cout << " -ac         use approximate clique-finding algorithms (-sas-id, -find-inc)"
	    << std::endl;
  std::cout << " -weak       weak invariant verification only (-verify)"
	    << std::endl;
  std::cout << " -cost <C>   set cost bound"
	    << std::endl;
  std::cout << " -g          write various graphs (to files in .dot format)"
	    << std::endl;
  std::cout << " -l <N>      set graph label detail (<N> = 0 - 7)"
	    << std::endl;
  std::cout << " -rnd <x>|-r <x>  set random seed"
	    << std::endl;
  std::cout << " -v <N>      set verbose level (<N> = 0 - 5)"
	    << std::endl;
  std::cout << " -strict     warnings-as-errors"
	    << std::endl;
}

int main(int argc, char *argv[]) {
  HSPS::Parser* reader = new HSPS::Parser(symbols);
  bool    trace_parse = false;
  bool    opt_untype = false;
  bool    opt_logic = false;
  bool    opt_petrify = false;
  bool    opt_compile_ce = false;
  bool    opt_compile_constraints_1 = false;
  bool    opt_compile_constraints_2 = false;
  bool    opt_compile_preferences = false;
  bool    opt_complete_negation = false;
  bool    opt_infer_negation = false;
  bool    opt_simplify = false;
  bool    opt_minimize_compose = true;
  bool    opt_old = false;
  bool    opt_integrify = false;
  bool    opt_eliminate_determined = false;
  bool    opt_simplify_wsa_single_variable_only = false;
  HSPS::index_type simplify_wsa_analysis_limit = 10000;
  bool    opt_spanning = false;
  bool    opt_print_intermediary_instance = false;
  bool    opt_maximize = false;
  bool    opt_minimize = false;
  bool    opt_metric_to_goal = false;
  bool    opt_goal_to_action = false;
  NTYPE   metric_bound = 0;
  bool    opt_build_instance = false;
  bool    opt_load_heuristic = true;
  bool    opt_echo = true;
  bool    opt_instantiate = false;
  bool    opt_abstract = false;
  bool    opt_abstract_away = false;
  bool    opt_preprocess = false;
  bool    opt_preprocess_H2 = true;
  bool    opt_lift = false;
  bool    opt_safe = false;
  bool    opt_PR = false;
  bool    opt_delete_relax = false;
  bool    opt_remove = false;
  bool    opt_relevant = false;
  bool    opt_path_relevant = false;
  bool    opt_relaxed_relevant = false;
  NTYPE   cost_bound = 0;
  bool    opt_cost_bound = false;
  bool    opt_extend_goal = false;
  bool    opt_extend_prec = false;
  bool    opt_compose = false;
  HSPS::index_type  composite_resource_size = 2;
  bool    opt_subset = false;
  HSPS::index_type subset_size = 2;
  bool    opt_split = false;
  bool    opt_1reg = false;
  bool    opt_find_invariants = false;
  bool    opt_find_c_invariants = false;
  bool    opt_bfs_find_invariants = false;
  bool    opt_dfs_find_invariants = false;
  bool    opt_find_inc_invariants = false;
  bool    opt_negation_invariants = true;
  bool    opt_apx_clique = false;
  bool    opt_verify_invariants = false;
  bool    opt_weak_verify = false;
  bool    opt_unit = false;
  NTYPE   unit_value = 1;
  bool    opt_round = false;
  bool    opt_round_up = false;
  bool    opt_round_down = false;
  NTYPE   opt_min_duration = ZERO;
  bool    opt_unlimited = false;
  bool    opt_validate = false;
  bool    opt_validate_low_res = false;
  bool    opt_print_trace = false;
  bool    opt_random = false;
  HSPS::index_type max_length = HSPS::no_such_index;
  HSPS::index_type avg_length = HSPS::no_such_index;
  HSPS::index_type n_plans = 1;
  bool    opt_assoc = false;
  bool    opt_difference = false;
  bool    opt_difference_order = false;
  bool    opt_complex_constraints = true;
  bool    opt_randomize_score = false;
  bool    opt_analyse = false;
  bool    opt_analyse_and_filter = false;
  bool    opt_distinguishing_traits = true;
  bool    opt_schedule = false;
  bool    opt_schedule_explore_options = false;
  bool    opt_seo_xdc = false;
  bool    opt_seo_sdc = false;
  bool    opt_resequence = false;
  bool    opt_resequence_random = false;
  HSPS::index_type reseq_d_min = HSPS::no_such_index;
  HSPS::index_type reseq_d_max = HSPS::no_such_index;
  HSPS::index_type reseq_n = HSPS::no_such_index;
  bool    opt_pop = false;
  bool    opt_deorder = false;
  bool    opt_gantt = false;
  bool    opt_enforce_min_durations = false;
  bool    opt_enforce_max_durations = false;
  bool    opt_enforce_min_makespan = false;
  bool    opt_reduce = false;
  bool    opt_redop = false;
  bool    opt_replace = false;
  bool    opt_landmark = false;
  bool    opt_landmark_H2 = false;
  bool    opt_check = false;
  bool    opt_initial = true;
  bool    opt_goal = true;
  bool    opt_hierarchy = false;
  bool    opt_partition = false;
  bool    opt_implies = false;
  HSPS::index_type implies_n_premises = 0;
  bool    opt_resource = false;
  bool    opt_sas = false;
  bool    opt_build_sas_instance = false;
  bool    opt_sas_selective = false;
  bool    opt_sas_min = false;
  bool    opt_sas_safe = false;
  bool    opt_sas_extend = false;
  bool    opt_sas_info = false;
  bool    opt_sas_id = false;
  HSPS::index_type opt_sas_spanning_search_limit = HSPS::no_such_index;
  bool    opt_sas_reduce_to_goal = false;
  bool    opt_sas_to_strips = false;
  bool    opt_write_graphs = false;
  bool    opt_write_xml = false;
  bool    opt_MATLAB = false;
  bool    opt_misc = false;
  bool    opt_untimed = false;
  bool    opt_sequential = false;
  bool    opt_temporal = false;
  bool    opt_H1 = false;
  bool    opt_H2 = false;
  bool    opt_H3 = false;
  bool    opt_lp = false;
  bool    opt_pdb = false;
  bool    opt_regression = true;
  bool    opt_progression = true;
  HSPS::index_type pdb_all_of_size = HSPS::no_such_index;
  bool    opt_inconsistency = true;
  bool    opt_discount = false;
  bool    opt_reverse = false;
  HSPS::SASInstance::label_style opt_edge_labels = HSPS::SASInstance::ls_none;
  bool    opt_print = true;
  bool    opt_print_brief = false;
  bool    opt_add = false;
  bool    opt_dump = false;
  bool    opt_domain = true;
  bool    opt_problem = true;
  bool    opt_debug = false;
  bool    opt_unique_names = false;
  bool    opt_htn = false;
  bool    opt_soft = false;
  bool    opt_nvz = false;
  bool    opt_decide = false;
  bool    opt_decide_strict = true;
  NTYPE   decide_min_value = 0;
  bool    opt_value_only = false;
  HSPS::index_type n_max = HSPS::no_such_index;
  bool    opt_cnf = false;
  bool    opt_ipc = false;
  bool    opt_strict_ipc = false;
#ifdef NTYPE_RATIONAL
  NTYPE       epsilon = HSPS::rational(1,100);
#else
  NTYPE       epsilon = 0.001;
#endif
  bool    opt_pddl = false;
  NTYPE   reduce_bound = 3;
  HSPS::index_type max_branch_depth = HSPS::no_such_index;
  HSPS::index_type selected_plan = HSPS::no_such_index;
  HSPS::index_type selected_set = HSPS::no_such_index;
  bool    opt_print_help = false;
  int     n_source_files = 0;

  HSPS::SASInstance::write_variable_sources = true;
  HSPS::SASInstance::write_action_sources = true;
  HSPS::SASInstance::write_info_in_domain = false;

  HSPS::Statistics stats;
  stats.enable_interrupt(true);
  HSPS::LC_RNG rng;

  stats.start();
  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      HSPS::Instance::default_trace_level = atoi(argv[++k]);
      if (HSPS::Instance::default_trace_level <= 0)
	HSPS::PDDL_Base::write_warnings = false;
      if (HSPS::Instance::default_trace_level > 1)
	HSPS::PDDL_Base::write_info = true;
      if (HSPS::Instance::default_trace_level > 2)
	HSPS::PDDL_Base::write_trace = true;
      if (HSPS::Instance::default_trace_level > 3)
	HSPS::PDDL_Base::trace_print_context = true;
      if (HSPS::Instance::default_trace_level > 4)
	trace_parse = true;
      HSPS::Preprocessor::default_trace_level =
	HSPS::Instance::default_trace_level;
      HSPS::CostTable::default_trace_level =
	HSPS::Instance::default_trace_level - 1;
      HSPS::Simplifier::default_trace_level =
	HSPS::Instance::default_trace_level;
    }
    else if (strcmp(argv[k],"-no-warnings") == 0) {
      HSPS::PDDL_Base::write_warnings = false;
    }
    else if (strcmp(argv[k],"-no-info") == 0) {
      HSPS::PDDL_Base::write_info = false;
    }
    else if ((strcmp(argv[k],"-exclude") == 0) && (k < argc - 1)) {
      char* tag = argv[++k];
      if (strcmp(tag, "all") == 0) {
	HSPS::PDDL_Base::exclude_all_dkel_items = true;
      }
      else {
	const HSPS::StringTable::Cell* cn = symbols.find(tag);
	if (cn) {
	  HSPS::PDDL_Base::excluded_dkel_tags.insert(cn->text);
	}
      }
    }
    else if ((strcmp(argv[k],"-require") == 0) && (k < argc - 1)) {
      const HSPS::StringTable::Cell* cn = symbols.find(argv[++k]);
      if (cn) {
	HSPS::PDDL_Base::required_dkel_tags.insert(cn->text);
      }
    }
    else if (strcmp(argv[k],"-strict") == 0) {
      HSPS::PDDL_Base::best_effort = false;
    }
    else if (strcmp(argv[k],"-name-by-file") == 0) {
      HSPS::PDDL_Base::name_instance_by_problem_file = true;
    }
    else if ((strcmp(argv[k],"-name-prefix") == 0) && (k < argc - 1)) {
      HSPS::PDDL_Base::instance_name_prefix = argv[++k];
    }
    else if (strcmp(argv[k],"-echo") == 0) {
      opt_echo = true;
      opt_build_instance = false;
      opt_preprocess = false;
      opt_print = true;
    }
    else if (strcmp(argv[k],"-unit") == 0) {
      opt_unit = true;
    }
    else if ((strcmp(argv[k],"-unit-value") == 0) && (k < argc - 1)) {
      unit_value = A_TO_N(argv[++k]);
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
    else if ((strcmp(argv[k],"-min-duration") == 0) && (k < argc - 1)) {
      opt_min_duration = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-unlimited") == 0) {
      opt_unlimited = true;
    }
    else if (strcmp(argv[k],"-untype") == 0) {
      opt_untype = true;
    }
    else if (strcmp(argv[k],"-logic") == 0) {
      opt_logic = true;
    }
    else if (strcmp(argv[k],"-petrify") == 0) {
      opt_petrify = true;
      opt_build_instance = true;
      opt_preprocess = true;
      opt_infer_negation = true;
      // opt_complete_negation = true;
      opt_safe = true;
      opt_goal_to_action = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-PR") == 0) {
      opt_PR = true;
    }
    else if (strcmp(argv[k],"-compile") == 0) {
      opt_compile_ce = true;
      opt_compile_constraints_1 = true;
      opt_compile_preferences = true;
    }
    else if (strcmp(argv[k],"-compile-for-val") == 0) {
      HSPS::PDDL_Base::compile_for_validator = true;
      HSPS::Instance::always_write_parameters = true;
      HSPS::Instance::always_write_requirements = true;
      HSPS::Instance::always_write_precondition = true;
      HSPS::Instance::always_write_effect = true;
      HSPS::Instance::always_write_conjunction = true;
    }
    else if (strcmp(argv[k],"-compile-ce") == 0) {
      opt_compile_ce = true;
    }
    else if (strcmp(argv[k],"-compile-constraints") == 0) {
      opt_compile_constraints_1 = true;
    }
    else if (strcmp(argv[k],"-compile-constraints-1") == 0) {
      opt_compile_constraints_1 = true;
    }
    else if (strcmp(argv[k],"-compile-constraints-2") == 0) {
      opt_compile_constraints_2 = true;
      opt_compile_constraints_1 = false;
    }
    else if (strcmp(argv[k],"-compile-preferences") == 0) {
      opt_compile_preferences = true;
    }
    else if (strcmp(argv[k],"-compile-3") == 0) {
      opt_compile_constraints_1 = true;
      opt_compile_preferences = true;
    }
    else if (strcmp(argv[k],"-complete-negation") == 0) {
      opt_infer_negation = true;
      opt_complete_negation = true;
    }
    else if (strcmp(argv[k],"-no-inference") == 0) {
      opt_infer_negation = false;
    }
    else if (strcmp(argv[k],"-no-compile") == 0) {
      HSPS::PDDL_Base::compile_away_disjunctive_preconditions = false;
      HSPS::PDDL_Base::compile_away_conditional_effects = false;
      HSPS::PDDL_Base::compile_away_plan_constraints = false;
      HSPS::PDDL_Base::compile_away_object_functions = false;
    }
    else if (strcmp(argv[k],"-no-compact") == 0) {
      HSPS::PDDL_Base::compact_resource_effects = false;
    }
    else if (strcmp(argv[k],"-create-all-atoms") == 0) {
      HSPS::PDDL_Base::create_all_atoms = true;
    }
    else if (strcmp(argv[k],"-create-all-actions") == 0) {
      HSPS::PDDL_Base::create_all_actions = true;
      HSPS::PDDL_Base::check_precondition_consistency = false;
    }
    else if (strcmp(argv[k],"-simplify") == 0) {
      opt_simplify = true;
      opt_eliminate_determined = true;
    }
    else if (strcmp(argv[k],"-no-mc") == 0) {
      opt_minimize_compose = false;
    }
    else if (strcmp(argv[k],"-simplify-1") == 0) {
      opt_simplify = true;
      opt_eliminate_determined = true;
      opt_old = true;
    }
    else if (strcmp(argv[k],"-simplify-analysis-1-only") == 0) {
      opt_simplify_wsa_single_variable_only = true;
    }
    else if ((strcmp(argv[k],"-simplify-analysis-limit") == 0) &&
	     (k < argc - 1)) {
      simplify_wsa_analysis_limit = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-integrify") == 0) {
      opt_integrify = true;
    }
    else if (strcmp(argv[k],"-determined") == 0) {
      opt_eliminate_determined = true;
    }
    else if (strcmp(argv[k],"-no-determined") == 0) {
      opt_eliminate_determined = false;
    }
    else if (strcmp(argv[k],"-spanning") == 0) {
      opt_spanning = true;
    }
    else if (strcmp(argv[k],"-intermediary") == 0) {
      opt_print_intermediary_instance = true;
    }
    else if (strcmp(argv[k],"-write-nary") == 0) {
      HSPS::PDDL_Base::Expression::print_nary = true;
    }
    else if (strcmp(argv[k],"-maximize") == 0) {
      opt_maximize = true;
      opt_minimize = false;
    }
    else if (strcmp(argv[k],"-minimize") == 0) {
      opt_minimize = true;
      opt_maximize = false;
    }
    else if ((strcmp(argv[k],"-metric-bound") == 0) && (k < argc - 1)) {
      opt_metric_to_goal = true;
      metric_bound = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-ins") == 0) {
      opt_build_instance = true;
      opt_instantiate = true;
      opt_echo = false;
      opt_print = false;
    }
    else if (strcmp(argv[k],"-no-ins") == 0) {
      opt_build_instance = false;
    }
    else if (strcmp(argv[k],"-prep") == 0) {
      opt_build_instance = true;
      opt_instantiate = true;
      opt_echo = false;
      opt_preprocess = true;
      opt_print = false;
    }
    else if (strcmp(argv[k],"-prep-1") == 0) {
      // opt_build_instance = true;
      // opt_instantiate = true;
      // opt_echo = false;
      // opt_preprocess = true;
      // opt_print = false;
      opt_preprocess_H2 = false;
    }
    else if (strcmp(argv[k],"-no-prep") == 0) {
      opt_preprocess = false;
    }
    else if ((strcmp(argv[k],"-abs") == 0) ||
	     (strcmp(argv[k],"-abstract") == 0)) {
      opt_build_instance = true;
      opt_abstract = true;
      // opt_instantiate = true;
      opt_echo = false;
      opt_print = false;
    }
    else if (strcmp(argv[k],"-abstract-away") == 0) {
      opt_build_instance = true;
      opt_abstract_away = true;
      // opt_instantiate = true;
      opt_echo = false;
      opt_print = false;
    }
    else if (strcmp(argv[k],"-load") == 0) {
      opt_load_heuristic = true;
    }
    else if (strcmp(argv[k],"-no-load") == 0) {
      opt_load_heuristic = false;
    }
    else if ((strcmp(argv[k],"-subset") == 0) && (k < argc - 1)) {
      opt_subset = true;
      subset_size = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-1-safe") == 0) {
      opt_safe = true;
      opt_infer_negation = true;
      // opt_complete_negation = true;
      opt_build_instance = true;
      opt_preprocess = true;
      opt_instantiate = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-no-safe") == 0) {
      opt_safe = false;
    }
    else if (strcmp(argv[k],"-relax") == 0) {
      opt_delete_relax = true;
      opt_build_instance = true;
      opt_preprocess = true;
      opt_instantiate = true;
      opt_echo = false;
    }
    else if ((strcmp(argv[k],"-implies") == 0) && (k < argc - 1)) {
      opt_implies = true;
      implies_n_premises = atoi(argv[++k]);
      opt_build_instance = true;
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-landmark") == 0) {
      opt_landmark = true;
      opt_build_instance = true;
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-landmark-2") == 0) {
      opt_landmark = true;
      opt_landmark_H2 = true;
      opt_build_instance = true;
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-check") == 0) {
      opt_check = true;
    }
    else if (strcmp(argv[k],"-hierarchy") == 0) {
      opt_hierarchy = true;
      opt_build_instance = true;
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-partition") == 0) {
      opt_partition = true;
      opt_build_instance = true;
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-split") == 0) {
      opt_split = true;
      opt_build_instance = true;
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-1-reg") == 0) {
      opt_1reg = true;
      opt_build_instance = true;
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-resource") == 0) {
      opt_resource = true;
      opt_build_instance = true;
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-misc") == 0) {
      opt_misc = true;
      opt_build_instance = true;
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-cnf") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_cnf = true;
    }
    else if (strcmp(argv[k],"-no-init") == 0) {
      opt_initial = false;
    }
    else if (strcmp(argv[k],"-no-goal") == 0) {
      opt_goal = false;
    }
    else if (strcmp(argv[k],"-goal-to-action") == 0) {
      opt_goal_to_action = true;
    }
    else if (strcmp(argv[k],"-lift") == 0) {
      opt_lift = true;
      opt_echo = true;
    }
    else if (strcmp(argv[k],"-find") == 0) {
      opt_find_invariants = true;
      opt_find_c_invariants = true;
      // opt_verify_invariants = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-find-c") == 0) {
      opt_find_invariants = true;
      opt_find_c_invariants = true;
      // opt_verify_invariants = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-find-bfs") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_find_invariants = true;
      opt_bfs_find_invariants = true;
      opt_verify_invariants = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-find-dfs") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_find_invariants = true;
      opt_dfs_find_invariants = true;
      opt_verify_invariants = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-find-inc") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_find_invariants = true;
      opt_find_inc_invariants = true;
      opt_verify_invariants = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-find-neg") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_find_invariants = true;
      opt_negation_invariants = true;
      opt_verify_invariants = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-no-find-neg") == 0) {
      opt_negation_invariants = false;
    }
    else if (strcmp(argv[k],"-verify") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_verify_invariants = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-weak") == 0) {
      opt_weak_verify = true;
    }
    else if (strcmp(argv[k],"-no-verify") == 0) {
      opt_verify_invariants = false;
    }
    else if ((strcmp(argv[k],"-max-bd") == 0) && (k < argc - 1)) {
      max_branch_depth = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-relevance") == 0) ||
	     (strcmp(argv[k],"-rel") == 0)) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_relevant = true;
      opt_echo = false;
    }
    else if ((strcmp(argv[k],"-path-relevance") == 0) ||
	     (strcmp(argv[k],"-path-rel") == 0)) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_build_sas_instance = true;
      opt_path_relevant = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-relaxed-relevance") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_build_sas_instance = true;
      opt_relaxed_relevant = true;
      opt_echo = false;
    }
    else if ((strcmp(argv[k],"-no-relevance") == 0) ||
	     (strcmp(argv[k],"-no-rel") == 0)) {
      opt_relevant = false;
    }
    else if (strcmp(argv[k],"-remove") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_remove = true;
      opt_relevant = true;
      opt_echo = false;
    }
    else if ((strcmp(argv[k],"-cost") == 0) && (k < argc - 1)) {
      cost_bound = A_TO_N(argv[++k]);
    }
    else if ((strcmp(argv[k],"-bcost") == 0) && (k < argc - 1)) {
      opt_cost_bound = true;
    }
    else if (strcmp(argv[k],"-extend") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_extend_goal = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-extend-pre") == 0) {
      opt_extend_prec = true;
    }
    else if ((strcmp(argv[k],"-compose") == 0) && (k < argc - 1)) {
      opt_compose = true;
      composite_resource_size = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-val") == 0) {
      opt_build_instance = true;
      opt_preprocess = false;
      opt_validate = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-val-low-res") == 0) {
      opt_build_instance = true;
      opt_preprocess = false;
      opt_validate = true;
      opt_validate_low_res = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-print-trace") == 0) {
      opt_print_trace = true;
    }
    else if (strcmp(argv[k],"-extended-action-definition") == 0) {
      HSPS::ExecState::extended_action_definition = true;
      HSPS::PDDL_Base::compile_away_disjunctive_preconditions = false;
      HSPS::PDDL_Base::compile_away_conditional_effects = false;
      HSPS::PDDL_Base::create_all_atoms = true;
      opt_preprocess = false;
    }
    else if (strcmp(argv[k],"-assoc") == 0) {
      opt_assoc = true;
    }
    else if (strcmp(argv[k],"-diff") == 0) {
      opt_build_instance = true;
      opt_preprocess = false;
      opt_validate = true;
      opt_difference = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-diff-1") == 0) {
      opt_build_instance = true;
      opt_preprocess = false;
      opt_validate = true;
      opt_difference = true;
      opt_complex_constraints = false;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-diff-2") == 0) {
      opt_build_instance = true;
      opt_preprocess = false;
      opt_validate = true;
      opt_difference = true;
      opt_difference_order = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-rnd-score") == 0) {
      opt_randomize_score = true;
    }
    else if (strcmp(argv[k],"-pop") == 0) {
      opt_build_instance = true;
      opt_preprocess = false;
      opt_pop = true;
      opt_deorder = true;
      opt_sequential = false;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-pop-dmin") == 0) {
      opt_enforce_min_durations = true;
    }
    else if (strcmp(argv[k],"-pop-dmax") == 0) {
      opt_enforce_max_durations = true;
    }
    else if (strcmp(argv[k],"-pop-mmin") == 0) {
      opt_enforce_min_makespan = true;
    }
    else if (strcmp(argv[k],"-seq") == 0) {
      opt_sequential = true;
    }
    else if (strcmp(argv[k],"-deorder") == 0) {
      opt_deorder = true;
    }
    else if (strcmp(argv[k],"-no-deorder") == 0) {
      opt_deorder = false;
    }
    else if (strcmp(argv[k],"-time") == 0) {
      opt_temporal = true;
    }
    else if (strcmp(argv[k],"-gantt") == 0) {
      opt_build_instance = true;
      opt_preprocess = false;
      opt_gantt = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-gantt-abbrev") == 0) {
      opt_build_instance = true;
      opt_preprocess = false;
      opt_gantt = true;
      opt_echo = false;
      HSPS::Schedule::GANTT_ACTION_NAMES_ON_CHART = false;
    }
    else if ((strcmp(argv[k],"-gantt-params") == 0) && (k < argc - 4)) {
      HSPS::Schedule::GANTT_UNIT_WIDTH = atof(argv[++k]);
      HSPS::Schedule::GANTT_UNIT_HEIGHT = atof(argv[++k]);
      HSPS::Schedule::GANTT_TEXT_XOFF = atof(argv[++k]);
      HSPS::Schedule::GANTT_TEXT_YOFF = atof(argv[++k]);
    }
    else if ((strcmp(argv[k],"-plan") == 0) && (k < argc - 1)) {
      selected_plan = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-set") == 0) && (k < argc - 1)) {
      selected_set = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-random") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_random = true;
    }
    else if ((strcmp(argv[k],"-max-length") == 0) && (k < argc - 1)) {
      max_length = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-avg-length") == 0) && (k < argc - 1)) {
      avg_length = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-n-plans") == 0) && (k < argc - 1)) {
      n_plans = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-n-max") == 0) && (k < argc - 1)) {
      n_max = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-analyse") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_analyse = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-analyse-and-filter") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_analyse = true;
      opt_analyse_and_filter = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-analyze") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_analyse = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-analyze-and-filter") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_analyse = true;
      opt_analyse_and_filter = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-no-traits") == 0) {
      opt_distinguishing_traits = false;
    }
    else if (strcmp(argv[k],"-schedule") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_analyse = true;
      opt_schedule = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-schedule-explore-options") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_analyse = true;
      opt_schedule = true;
      opt_schedule_explore_options = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-schedule-explore-options-1") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_schedule = true;
      opt_schedule_explore_options = true;
      opt_seo_xdc = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-schedule-explore-options-2") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_schedule = true;
      opt_schedule_explore_options = true;
      opt_seo_xdc = true;
      opt_seo_sdc = true;
      opt_echo = false;
    }
    else if ((strcmp(argv[k],"-resequence") == 0) && (k < (argc - 2))) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_resequence = true;
      reseq_d_min = atoi(argv[++k]);
      reseq_d_max = atoi(argv[++k]);
      opt_echo = false;
    }
    else if ((strcmp(argv[k],"-resequence-random") == 0) && (k < (argc - 1))) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_resequence = false;
      opt_resequence_random = true;
      // reseq_d_min = atoi(argv[++k]);
      // reseq_d_max = atoi(argv[++k]);
      reseq_n = atoi(argv[++k]);
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-untimed") == 0) {
      opt_untimed = true;
    }
    else if (strcmp(argv[k],"-sas") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_build_sas_instance = true;
      opt_sas = true;
      opt_print = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-sas-selective") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_build_sas_instance = true;
      opt_sas = true;
      opt_sas_selective = true;
      opt_print = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-sas-min") == 0) {
      if (opt_path_relevant && !opt_sas) {
	opt_sas_min = true;
	opt_sas_safe = false;
      }
      else {
	opt_build_instance = true;
	opt_preprocess = true;
	opt_build_sas_instance = true;
	opt_sas = true;
	opt_print = true;
	opt_sas_min = true;
	opt_sas_safe = false;
	opt_echo = false;
      }
    }
    else if (strcmp(argv[k],"-sas-safe") == 0) {
      if (opt_path_relevant && !opt_sas) {
	opt_sas_safe = true;
	opt_sas_min = false;
      }
      else {
	opt_build_instance = true;
	opt_preprocess = true;
	opt_build_sas_instance = true;
	opt_sas = true;
	opt_print = true;
	opt_sas_safe = true;
	opt_sas_min = false;
	opt_echo = false;
      }
    }
    else if (strcmp(argv[k],"-sas-cost") == 0) {
      HSPS::SASInstance::write_action_cost = true;
    }
    else if (strcmp(argv[k],"-sas-extend") == 0) {
      opt_sas_extend = true;
      HSPS::SASInstance::additional_strictness_check = false;
    }
    else if (strcmp(argv[k],"-sas-id") == 0) {
      opt_sas_id = true;
    }
    else if ((strcmp(argv[k],"-sas-spanning-search-limit") == 0) && (k < argc - 1)) {
      opt_sas_spanning_search_limit = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-sas-goal") == 0) {
      opt_sas_reduce_to_goal = true;
    }
    else if (strcmp(argv[k],"-sas-no-info") == 0) {
      HSPS::SASInstance::write_info_in_domain = false;
      HSPS::SASInstance::write_action_sources = false;
      opt_sas_info = false;
    }
    else if (strcmp(argv[k],"-sas-info") == 0) {
      HSPS::SASInstance::write_info_in_domain = true;
      HSPS::SASInstance::write_action_sources = true;
      opt_sas_info = true;
    }
    else if (strcmp(argv[k],"-ac") == 0) {
      opt_apx_clique = true;
    }
    else if (strcmp(argv[k],"-strips") == 0) {
      opt_sas_to_strips = true;
      opt_print = true;
    }
    else if (strcmp(argv[k],"-g") == 0) {
      opt_write_graphs = true;
    }
    else if (strcmp(argv[k],"-dg") == 0) {
      opt_MATLAB = true;
    }
    else if (strcmp(argv[k],"-xml") == 0) {
      opt_write_xml = true;
      opt_build_instance = true;
    }
    else if (strcmp(argv[k],"-reverse") == 0) {
      opt_build_instance = true;
      opt_instantiate = true;
      opt_echo = false;
      opt_print = true;
      opt_infer_negation = false;
      opt_complete_negation = true;
      opt_extend_goal = true;
      opt_extend_prec = true;
      opt_reverse = true;
      HSPS::Instance::write_negation = false;
    }
    else if (strcmp(argv[k],"-regularise") == 0) {
      opt_build_instance = true;
      opt_instantiate = true;
      opt_echo = false;
      opt_print = true;
      opt_infer_negation = false;
      opt_complete_negation = true;
      opt_extend_goal = true;
      opt_extend_prec = true;
      // opt_reverse = true;
      HSPS::Instance::write_negation = false;
    }
    else if ((strcmp(argv[k],"-l") == 0) && (k < argc - 1)) {
      opt_edge_labels = ((HSPS::SASInstance::label_style)atoi(argv[++k]));
    }
    else if (strcmp(argv[k],"-reduce") == 0) {
      opt_preprocess = true;
      opt_reduce = true;
    }
    else if (strcmp(argv[k],"-redop") == 0) {
      opt_redop = true;
      opt_echo = false;
      opt_build_instance = true;
      opt_preprocess = true;
    }
    else if (strcmp(argv[k],"-rep") == 0) {
      opt_replace = true;
    }
    else if ((strcmp(argv[k],"-bound") == 0) && (k < argc - 1)) {
      if (strcasecmp(argv[k+1],"inf") == 0) {
	reduce_bound = POS_INF;
	k += 1;
      }
      else {
	reduce_bound = A_TO_N(argv[++k]);
      }
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
    else if ((strcmp(argv[k],"-count") == 0) || (strcmp(argv[k],"-c") == 0)) {
      opt_print = false;
    }
    else if ((strcmp(argv[k],"-print") == 0) || (strcmp(argv[k],"-p") == 0)) {
      opt_print = true;
    }
    else if ((strcmp(argv[k],"-brief") == 0) || (strcmp(argv[k],"-b") == 0)) {
      opt_print_brief = true;
    }
    else if ((strcmp(argv[k],"-add") == 0) || (strcmp(argv[k],"-a") == 0)) {
      opt_print = true;
      opt_add = true;
    }
    else if ((strcmp(argv[k],"-dump") == 0) || (strcmp(argv[k],"-d") == 0)) {
      opt_print = false;
      opt_dump = true;
    }
    else if (strcmp(argv[k],"-no-domain") == 0) {
      opt_domain = false;
    }
    else if (strcmp(argv[k],"-no-problem") == 0) {
      opt_problem = false;
    }
    else if (strcmp(argv[k],"-stderr") == 0) {
      opt_debug = true;
    }
    else if (strcmp(argv[k],"-no-extra") == 0) {
      HSPS::Instance::write_extra = false;
    }
    else if (strcmp(argv[k],"-htn") == 0) {
      opt_htn = true;
    }
    else if (strcmp(argv[k],"-soft") == 0) {
      opt_soft = true;
    }
    else if (strcmp(argv[k],"-nvz") == 0) {
      opt_nvz = true;
    }
    else if ((strcmp(argv[k],"-decide") == 0) && (k < argc - 1)) {
      opt_decide = true;
      decide_min_value = A_TO_N(argv[++k]);
      opt_build_instance = true;
      opt_preprocess = true;
      opt_soft = true;
      opt_echo = false;
    }
    else if ((strcmp(argv[k],"-decide-no-cost") == 0) && (k < argc - 1)) {
      opt_decide = true;
      opt_value_only = true;
      decide_min_value = A_TO_N(argv[++k]);
      opt_build_instance = true;
      opt_preprocess = true;
      opt_soft = true;
      opt_echo = false;
    }
    else if ((strcmp(argv[k],"-decide-eq") == 0) && (k < argc - 1)) {
      opt_decide = true;
      opt_decide_strict = false;
      decide_min_value = A_TO_N(argv[++k]);
      opt_build_instance = true;
      opt_preprocess = true;
      opt_soft = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-no-htn") == 0) {
      HSPS::HTNInstance::write_HTN = false;
      opt_htn = false;
    }
    else if (strcmp(argv[k],"-no-dkel") == 0) {
      HSPS::Instance::write_DKEL = false;
    }
    else if (strcmp(argv[k],"-no-metric") == 0) {
      HSPS::Instance::write_metric = false;
    }
    else if (strcmp(argv[k],"-no-time") == 0) {
      HSPS::Instance::write_time = false;
    }
    else if (strcmp(argv[k],"-pddl1") == 0) {
      HSPS::Instance::write_PDDL2 = false;
      HSPS::Instance::write_PDDL3 = false;
    }
    else if (strcmp(argv[k],"-no-pddl2") == 0) {
      HSPS::Instance::write_PDDL2 = false;
    }
    else if (strcmp(argv[k],"-no-pddl3") == 0) {
      HSPS::Instance::write_PDDL3 = false;
    }
    else if (strcmp(argv[k],"-no-negation") == 0) {
      HSPS::Instance::write_negation = false;
    }
    else if (strcmp(argv[k],"-unique") == 0) {
      opt_unique_names = true;
    }
    else if (strcmp(argv[k],"-ipc") == 0) {
      opt_ipc = true;
      opt_build_instance = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-strict-ipc") == 0) {
      opt_ipc = true;
      opt_strict_ipc = true;
      opt_build_instance = true;
      opt_echo = false;
    }
    else if ((strcmp(argv[k],"-epsilon") == 0) && (k < argc - 1)) {
      epsilon = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-pddl") == 0) {
      opt_pddl = true;
      opt_build_instance = true;
    }
    else if (strcmp(argv[k],"-no-traits") == 0) {
      HSPS::Schedule::write_traits = false;
    }
    else if (strcmp(argv[k],"-write-parameters") == 0) {
      HSPS::Instance::always_write_parameters = true;
    }
    else if (strcmp(argv[k],"-write-requirements") == 0) {
      HSPS::Instance::always_write_requirements = true;
    }
    else if (strcmp(argv[k],"-write-strict-PDDL") == 0) {
      HSPS::Instance::always_write_parameters = true;
      HSPS::Instance::always_write_requirements = true;
      HSPS::Instance::always_write_precondition = true;
      HSPS::Instance::always_write_effect = true;
      HSPS::Instance::always_write_conjunction = true;
    }
    else if ((strcmp(argv[k],"-catc") == 0) && (k < argc - 1)) {
      HSPS::PDDL_Name::catc = *argv[++k];
    }
    else if (strcmp(argv[k],"-nsn") == 0) {
      HSPS::Instance::write_atom_set_with_symbolic_names = false;
      HSPS::Instance::write_action_set_with_symbolic_names = false;
    }
    else if (strcmp(argv[k],"-use-strict-borrow") == 0) {
      HSPS::PDDL_Base::use_strict_borrow_definition = true;
    }
    else if (strcmp(argv[k],"-use-extended-borrow") == 0) {
      HSPS::PDDL_Base::use_extended_borrow_definition = true;
    }
    else if (strcmp(argv[k],"-non-strict-set-export") == 0) {
      HSPS::PDDL_Base::strict_set_export = false;
    }
    else if (strcmp(argv[k],"-inc") == 0) {
      opt_inconsistency = true;
    }
    else if (strcmp(argv[k],"-no-inc") == 0) {
      opt_inconsistency = false;
    }
    else if (strcmp(argv[k],"-1") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_H1 = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-2") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_H2 = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-3") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_H3 = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-lp") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_lp = true;
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-pdb") == 0) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_build_sas_instance = true;
      opt_pdb = true;
      opt_echo = false;
    }
    else if ((strcmp(argv[k],"-pdb-all-of-size") == 0) && (k < argc - 1)) {
      opt_build_instance = true;
      opt_preprocess = true;
      opt_build_sas_instance = true;
      opt_pdb = true;
      pdb_all_of_size = atoi(argv[++k]);
      opt_echo = false;
    }
    else if (strcmp(argv[k],"-no-regression") == 0) {
      opt_regression = false;
    }
    else if (strcmp(argv[k],"-no-progression") == 0) {
      opt_progression = false;
    }
    else if (strcmp(argv[k],"-discount") == 0) {
      opt_discount = true;
    }
    else if ((strcmp(argv[k],"-help") == 0) || (strcmp(argv[k],"-h") == 0)) {
      opt_print_help = true;
    }
    else if (*argv[k] != '-') {
      reader->read(argv[k], trace_parse);
      n_source_files += 1;
    }
  }

  if (opt_print_help || (n_source_files == 0)) {
    print_help(argv[0]);
    exit(0);
  }

  if (HSPS::PDDL_Base::compile_away_conditional_effects && opt_instantiate) {
    HSPS::PDDL_Base::number_multiple_action_instances = true;
  }

  stats.stop();
  std::cerr << "input read in " << stats.time() << " seconds"
	    << std::endl;

  HSPS::Instance*  instance = 0;
  HSPS::index_type initial_instance_atom_count = 0;
  HSPS::index_type initial_instance_action_count = 0;
  HSPS::cost_vec   saved_durations;
  HSPS::bool_vec   std_relevant_atoms;
  HSPS::bool_vec   std_relevant_actions;
  HSPS::bool_vec   L1C1_relevant_atoms;
  HSPS::bool_vec   L1C1_relevant_actions;
  HSPS::bool_vec   LsC1_relevant_atoms;
  HSPS::bool_vec   LsC1_relevant_actions;
  HSPS::bool_vec   path_relevant_atoms;
  HSPS::bool_vec   path_relevant_actions;
  HSPS::bool_vec   relaxed_relevant_actions;
  HSPS::Reduce*    prep = 0;
  HSPS::ScheduleSet* plans = 0;

  reader->post_process();

  if (opt_untype) {
    reader->untype();
  }

  if (opt_compile_constraints_1) {
    reader->compile_constraints_1();
  }
  else if  (opt_compile_constraints_2) {
    reader->compile_constraints_2();
  }

  if (opt_compile_ce && !opt_build_instance) {
    reader->compile_set_conditions_and_effects();
  }

  if (opt_compile_preferences && !(opt_build_instance && opt_soft)) {
    reader->compile_preferences();
  }

  if (opt_maximize) {
    if (reader->metric_type == HSPS::PDDL_Base::metric_minimize) {
      reader->metric = new HSPS::PDDL_Base::BinaryExpression
	(HSPS::PDDL_Base::exp_mul,
	 new HSPS::PDDL_Base::ConstantExpression(-1),
	 reader->metric);
      reader->metric_type = HSPS::PDDL_Base::metric_maximize;
    }
  }
  else if (opt_minimize) {
    if (reader->metric_type == HSPS::PDDL_Base::metric_maximize) {
      reader->metric = new HSPS::PDDL_Base::BinaryExpression
	(HSPS::PDDL_Base::exp_mul,
	 new HSPS::PDDL_Base::ConstantExpression(-1),
	 reader->metric);
      reader->metric_type = HSPS::PDDL_Base::metric_minimize;
    }
  }

  if (opt_simplify && !opt_build_instance) {
    if (reader->metric) {
      reader->metric = reader->metric->simplify();
      if (opt_integrify) {
	NTYPE d = reader->metric->integrify();
	std::cout << ";; divisor = " << d << std::endl;
      }
      else if (opt_round_up) {
	HSPS::PDDL_Base::exp_vec c(0, 0);
	reader->metric->collect_constants(c);
	for (HSPS::index_type k = 0; k < c.length(); k++) {
	  if (!INTEGRAL(((HSPS::PDDL_Base::ConstantExpression*)c[k])->val))
	    ((HSPS::PDDL_Base::ConstantExpression*)c[k])->val =
	      FLOOR(((HSPS::PDDL_Base::ConstantExpression*)c[k])->val) + 1;
	}
      }
      else if (opt_round_down) {
	HSPS::PDDL_Base::exp_vec c(0, 0);
	reader->metric->collect_constants(c);
	for (HSPS::index_type k = 0; k < c.length(); k++) {
	  ((HSPS::PDDL_Base::ConstantExpression*)c[k])->val =
	    FLOOR(((HSPS::PDDL_Base::ConstantExpression*)c[k])->val);
	}
      }
    }
  }

  if (opt_metric_to_goal) {
    reader->metric_to_goal(metric_bound);
  }

  if (opt_find_c_invariants) {
    HSPS::index_type n = reader->dom_sc_invariants.length();
    stats.start();
    reader->find_cc();
    stats.stop();
    std::cerr << reader->dom_sc_invariants.length() - n
	      << " c-invariants found in " << stats.time()
	      << " seconds" << std::endl;
  }

  if (opt_build_instance) {
    if (opt_htn) {
      instance = new HSPS::HTNInstance();
    }
    else if (opt_soft) {
      instance = new HSPS::SoftInstance();
    }
    else {
      instance = new HSPS::Instance();
    }
    stats.start();
    reader->instantiate(*instance);
    if (opt_htn) {
      reader->instantiateHTN(*((HSPS::HTNInstance*)instance));
    }
    else if (opt_soft) {
      reader->instantiate_soft(*((HSPS::SoftInstance*)instance));
      if (opt_nvz) ((HSPS::SoftInstance*)instance)->null_value = 0;
    }
    stats.stop();
    std::cerr << "instantiation finished in " << stats.time() << " seconds"
	      << std::endl;
    if (opt_soft && opt_compile_preferences) {
      stats.start();
      HSPS::Instance* c_ins = new HSPS::Instance(instance->name);
      ((HSPS::SoftInstance*)instance)->compile(*c_ins);
      delete instance;
      instance = c_ins;
      stats.stop();
      std::cerr << "soft goals compiled in " << stats.time() << " seconds"
		<< std::endl;
      opt_soft = false;
    }

    prep = new HSPS::Reduce(*instance, stats);

    if (opt_preprocess) {
      stats.start();
      prep->preprocess(false);
      stats.stop();
      std::cerr << "preprocessing finished in " << stats.time()
		<< " seconds" << std::endl;
    }
    else {
      instance->cross_reference();
    }

    // counts after normal preprocessing but before any other simplification
    initial_instance_atom_count = instance->n_atoms();
    initial_instance_action_count = instance->n_actions();

    if (opt_relevant) {
      stats.start();
      std_relevant_atoms.assign_value(false, instance->n_atoms());
      std_relevant_actions.assign_value(false, instance->n_actions());
      prep->compute_relevance(instance->goal_atoms,
			      std_relevant_atoms,
			      std_relevant_actions);
      stats.stop();
      std::cerr << std_relevant_atoms.count(true)
		<< " of " << instance->n_atoms()
		<< " atoms and "
		<< std_relevant_actions.count(true)
		<< " of " << instance->n_actions()
		<< " actions are standard-relevant"
		<< " (" << stats.time() << " seconds)"
		<< std::endl;
      for (HSPS::index_type k = 0; k < instance->n_atoms(); k++)
	if (!std_relevant_atoms[k]) instance->atoms[k].irrelevant = true;
    }

    if (opt_remove) {
      std::cerr << "removing " << std_relevant_atoms.count(false)
		<< " atoms and " << std_relevant_actions.count(false)
		<< " actions..." << std::endl;
      prep->remove_irrelevant_atoms();
      std_relevant_atoms.assign_value(true, instance->n_atoms());
      std_relevant_actions.assign_value(true, instance->n_actions());
    }

    // may need to re-cross reference after removing irrelevant atoms
    if (!instance->cross_referenced()) {
      std::cerr << "re-cross referencing..." << std::endl;
      instance->cross_reference();
    }

    if (opt_preprocess && opt_preprocess_H2) {
      stats.start();
      prep->preprocess(true);
      stats.stop();
      std::cerr << "second preprocessing stage finished in " << stats.time()
		<< " seconds" << std::endl;
    }

    // remap hard/soft goals after all preprocessing steps completed
    if (opt_soft) {
      ((HSPS::SoftInstance*)instance)->remap_hard_goals(prep->atom_map);
      ((HSPS::SoftInstance*)instance)->remap_soft_goals(prep->atom_map);
    }

    if (opt_relevant &&
	((std_relevant_atoms.length() != instance->n_atoms()) ||
	 (std_relevant_actions.length() != instance->n_actions()))) {
      std::cerr << "recomputing relevance after second preprocessing stage..."
		<< std::endl;
      stats.start();
      std_relevant_atoms.assign_value(false, instance->n_atoms());
      std_relevant_actions.assign_value(false, instance->n_actions());
      prep->compute_relevance(instance->goal_atoms,
			      std_relevant_atoms,
			      std_relevant_actions);
      stats.stop();
      std::cerr << std_relevant_atoms.count(true)
		<< " of " << instance->n_atoms()
		<< " atoms and "
		<< std_relevant_actions.count(true)
		<< " of " << instance->n_actions()
		<< " actions are standard-relevant"
		<< " (" << stats.time() << " seconds)"
		<< std::endl;
      if (opt_remove) {
	std::cerr << "removing " << std_relevant_atoms.count(false)
		  << " atoms and " << std_relevant_actions.count(false)
		  << " actions..." << std::endl;
	prep->remove_irrelevant_atoms();
	std_relevant_atoms.assign_value(true, instance->n_atoms());
	std_relevant_actions.assign_value(true, instance->n_actions());
      }
    }

    // may need to re-cross reference after removing irrelevant atoms
    if (!instance->cross_referenced()) {
      std::cerr << "re-cross referencing..." << std::endl;
      instance->cross_reference();
    }

    if (opt_relaxed_relevant) {
      HSPS::index_vec p;
      relaxed_relevant_actions.assign_value(false, instance->n_actions());
      prep->compute_relaxed_relevance(instance->goal_atoms,
				      p, instance->goal_atoms,
				      relaxed_relevant_actions);
    }

    if (opt_bfs_find_invariants || opt_dfs_find_invariants ||
	opt_find_inc_invariants || opt_negation_invariants) {
      HSPS::index_type init_n = instance->n_invariants();
      stats.start();
      if (opt_bfs_find_invariants) {
	prep->bfs_find_invariants();
      }
      if (opt_dfs_find_invariants) {
	prep->dfs_find_invariants(max_branch_depth);
      }
      if (opt_find_inc_invariants) {
	HSPS::graph* g_inc = prep->inconsistency_graph();
	prep->find_inconsistent_set_invariants(*g_inc);
      }
      if (opt_negation_invariants) {
	instance->add_missing_negation_invariants();
      }
      stats.stop();
      std::cerr << instance->n_invariants() - init_n
		<< " new invariants found in " << stats.time()
		<< " seconds" << std::endl;
    }

    if (opt_verify_invariants) {
      stats.start();
      if (opt_weak_verify)
	instance->verify_invariants();
      else
	prep->verify_invariants(*(prep->inconsistency()));
      stats.stop();
      HSPS::index_type n_verified = 0;
      for (HSPS::index_type k = 0; k < instance->n_invariants(); k++)
	if (instance->invariants[k].verified) n_verified += 1;
      std::cerr << n_verified << " of " << instance->n_invariants()
		<< " invariants verified in " << stats.time()
		<< " seconds" << std::endl;
      prep->remove_unverified_invariants();
      HSPS::index_type n_inv = instance->n_invariants();
      prep->remove_useless_invariants();
      std::cerr << n_inv - instance->n_invariants()
		<< " useless invariants removed" << std::endl;
    }

    if (opt_infer_negation) {
      std::cerr << "infering atom negations..." << std::endl;
      prep->find_binary_iff_invariants(opt_weak_verify);
      instance->extract_atom_negations_from_invariants();
      std::cerr << "finished in " << stats.time() << " seconds" << std::endl;
    }

    if (opt_reduce) {
      HSPS::index_type n = instance->n_actions();
      prep->reduce(reduce_bound);
      std::cerr << n - instance->n_actions()
		<< " redundant actions removed in " << stats.time()
		<< " seconds" << std::endl;
      // may need to re-cross reference after reducing
      if (!instance->cross_referenced()) {
	std::cerr << "re-cross referencing..." << std::endl;
	instance->cross_reference();
      }
    }

    if (opt_simplify) {
      HSPS::index_type n = instance->n_atoms();
      if (opt_eliminate_determined) {
	prep->eliminate_strictly_determined_atoms();
	std::cerr << (n - instance->n_atoms())
		  << " strictly determined atoms removed in "
		  << stats.time() << " seconds" << std::endl;
      }

      // may need to re-cross reference after eliminating determined atoms
      if (!instance->cross_referenced()) {
	std::cerr << "re-cross referencing..." << std::endl;
	instance->cross_reference();
      }

      if ((instance->n_atoms() < n) && opt_verify_invariants) {
	std::cerr << "re-verifying invariants following removal of "
		  << n - instance->n_atoms() << " atoms..." << std::endl;
	stats.start();
	for (HSPS::index_type k = 0; k < instance->n_invariants(); k++)
	  instance->invariants[k].verified = false;
	if (opt_weak_verify)
	  instance->verify_invariants();
	else
	  prep->verify_invariants(*(prep->inconsistency()));
	stats.stop();
	HSPS::index_type n_verified = 0;
	for (HSPS::index_type k = 0; k < instance->n_invariants(); k++)
	  if (instance->invariants[k].verified) n_verified += 1;
	std::cerr << n_verified << " of " << instance->n_invariants()
		  << " invariants verified in " << stats.time()
		  << " seconds" << std::endl;
	prep->remove_unverified_invariants();
      }
    }

    if (opt_complete_negation) {
      std::cerr << "adding atom negations..." << std::endl;
      instance->complete_atom_negations();

      // may need to re-cross reference after completing negations
      if (!instance->cross_referenced()) {
	std::cerr << "re-cross referencing..." << std::endl;
	instance->cross_reference();
      }
    }

    if (opt_extend_goal) {
      stats.start();
      HSPS::index_set new_goals;
      prep->implied_atom_set(instance->goal_atoms, new_goals,
			     *prep->inconsistency());
      stats.stop();
      std::cerr << new_goals.length() << " implied goals found in "
		<< stats.time()	<< " seconds" << std::endl;
      if (new_goals.length() > 0) {
	HSPS::index_set new_goal(instance->goal_atoms);
	new_goal.insert(new_goals);
	instance->set_goal(new_goal);
      }
    }

    if (opt_extend_prec) {
      std::cerr << "extending action preconditions..." << std::endl;
      stats.start();
      HSPS::index_set imp;
      for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	for (HSPS::index_type i = 0; i < instance->actions[k].add.length(); i++)
	  if (prep->inconsistent(instance->actions[k].pre,
				 instance->actions[k].add[i]))
	    instance->actions[k].pre.insert(instance->atoms[instance->actions[k].add[i]].neg);
	for (HSPS::index_type i = 0; i < instance->actions[k].del.length(); i++)
	  if (prep->inconsistent(instance->actions[k].pre,
				 instance->atoms[instance->actions[k].del[i]].neg))
	    instance->actions[k].pre.insert(instance->actions[k].del[i]);
      }
      stats.stop();
      std::cerr << "finished in " << stats.time() << " seconds" << std::endl;
    }

    if (opt_goal_to_action) {
      std::cerr << "converting goal to action..." << std::endl;
      stats.start();
      if (opt_petrify) {
	HSPS::Instance::goal_action_cost = epsilon;
      }
      HSPS::index_set_vec g;
      g.append(instance->goal_atoms);
      instance->set_DNF_goal(g);
      if (opt_petrify) {
	g[0].clear();
	for (HSPS::index_type k = 0; k < instance->n_atoms(); k++)
	  if (instance->atoms[k].goal)
	    g[0].insert(k);
	assert(g[0].length() == 1);
	HSPS::Instance::goal_atom_name = "FINI";
	HSPS::Instance::goal_action_name = "Finish";
	instance->set_DNF_goal(g);
      }
      if (opt_complete_negation) {
	std::cerr << "adding atom negations..." << std::endl;
	instance->complete_atom_negations();
      }
      if (!instance->cross_referenced()) {
	std::cerr << "re-cross referencing..." << std::endl;
	instance->cross_reference();
      }
      stats.stop();
      std::cerr << "finished in " << stats.time() << " seconds" << std::endl;
    }

    if (opt_safe) {
      stats.start();
      if (!opt_complete_negation) {
	std::cerr << "checking for missing atom negations..." << std::endl;
	HSPS::index_set missing;
	prep->necessary_completions_for_safeness(missing);
	std::cerr << "adding " << missing.length()
		  << " atom negations missing for 1-safe transformation...";
	if (HSPS::Instance::default_trace_level > 0) {
	  std::cerr << std::endl;
	  instance->write_atom_set(std::cerr, missing);
	  std::cerr << std::endl;
	}
	for (HSPS::index_type k = 0; k < missing.length(); k++)
	  instance->complete_atom_negation(missing[k]);
	if (!instance->cross_referenced()) {
	  std::cerr << "re-cross referencing..." << std::endl;
	  instance->cross_reference();
	}
	std::cerr << "done" << std::endl;
      }
      std::cerr << "making instance 1-safe..." << std::endl;
      HSPS::index_type n = instance->n_actions();
      prep->make_safe();
      stats.stop();
      if (instance->n_actions() < n) {
	std::cerr << "warning: ACTIONS LOST!" << std::endl;
      }
      std::cerr << instance->n_actions() - n << " more actions, finished in "
		<< stats.time() << " seconds" << std::endl;
      // may need to re-cross reference after making instance 1-safe
      if (!instance->cross_referenced()) {
	std::cerr << "re-cross referencing..." << std::endl;
	instance->cross_reference();
      }
    }

    if (opt_reverse) {
      stats.start();
      std::cerr << "reversing instance..." << std::endl;
      HSPS::Instance* rev_instance = new HSPS::Instance(instance->name);
      rev_instance->reverse_copy(*instance);
      std::cerr << "re-cross referencing..." << std::endl;
      rev_instance->cross_reference();
      delete instance;
      instance = rev_instance;
      delete prep;
      prep = new HSPS::Reduce(*instance, stats);
      stats.stop();
      std::cerr << "reversal finished in " << stats.time() << " seconds"
		<< std::endl;
    }

    if (opt_PR) {
      stats.start();
      std::cerr << "applying place replication..." << std::endl;
      HSPS::index_type n = instance->n_atoms();
      prep->apply_place_replication();
      stats.stop();
      std::cerr << instance->n_atoms() - n << " more atoms" << std::endl;
      if (!instance->cross_referenced()) {
	std::cerr << "re-cross referencing..." << std::endl;
	instance->cross_reference();
      }
      std::cerr << "finished in " << stats.time() << " seconds" << std::endl;
    }

    if (opt_delete_relax) {
      stats.start();
      HSPS::name_vec atm_names(0, 0);
      instance->atom_names(atm_names);
      HSPS::index_set_vec sets;
      reader->export_sets(atm_names, sets);
      if (selected_set != HSPS::no_such_index) {
	if (selected_set >= sets.length()) {
	  std::cerr << "error: set " << selected_set << " chosen, but only "
		    << sets.length() << " atom sets defined in input"
		    << std::endl;
	  exit(1);
	}
	instance->delete_relax(sets[selected_set]);
	if (opt_discount) {
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	    if ((instance->actions[k].add.first_common_element(sets[selected_set]) == HSPS::no_such_index) || (instance->actions[k].add.first_common_element(sets[selected_set]) == HSPS::no_such_index))
	      instance->actions[k].cost = 0;
	  }
	}
      }
      else {
	instance->delete_relax(HSPS::EMPTYSET);
      }
      stats.stop();
      std::cerr << "delete relaxed instance computed in " << stats.time()
		<< " seconds" << std::endl;
    }

    if (opt_landmark) {
      HSPS::graph prec;
      stats.start();
      prep->quick_landmark_graph(prec, opt_landmark_H2);
      stats.stop();
      std::cerr << "precedence/landmark graph computed in "
		<< stats.time() << " seconds" << std::endl;
      if (opt_check) {
	HSPS::graph pg_check;
	stats.start();
	prep->compute_landmark_graph(pg_check, opt_landmark_H2);
	stats.stop();
	std::cerr << "precedence/landmark graph computed with full tests in "
		  << stats.time() << " seconds" << std::endl;
	if (!pg_check.equals(prec)) {
	  std::cerr << "error: graphs differ!" << std::endl;
	  HSPS::index_vec_util c;
	  c.fill(instance->n_atoms());
	  HSPS::pair_set d0;
	  HSPS::pair_set d1;
	  pg_check.difference(prec, c, d0, d1);
	  std::cerr << "graph computed with reduced tests: " << prec
		    << std::endl;
	  std::cerr << "false edges (in this graph, not in other): " << d0
		    << std::endl;
	  std::cerr << "graph computed with full tests: " << pg_check
		    << std::endl;
	  std::cerr << "missing edges (in this graph, not in other): " << d1
		    << std::endl;
	  exit(1);
	}
      }

      HSPS::index_set lma;
      for (HSPS::index_type k = 0; k < instance->goal_atoms.length(); k++)
	lma.insert(prec.predecessors(instance->goal_atoms[k]));
      std::cerr << lma.length() << " of " << instance->n_atoms()
		<< " atom are goal landmarks: ";
      instance->write_atom_set(std::cerr, lma);
      std::cerr << std::endl;
      std::cerr << prec.n_edges() << " edges in landmark graph" << std::endl;
      HSPS::pair_set mte;
      prec.missing_transitive_edges(mte);
      if (mte.empty()) {
	std::cerr << "precedence graph is transitively closed"
		  << std::endl;
      }
      else {
	std::cerr << "precedence graph is NOT transitively closed!"
		  << std::endl
		  << mte.length() << " missing edges:"
		  << std::endl;
	for (HSPS::index_type k = 0; k < mte.length(); k++)
	  std::cerr << " " << instance->atoms[mte[k].first].name
		    << " -> " << instance->atoms[mte[k].second].name
		    << std::endl;
      }

      if (opt_write_graphs) {
	HSPS::index_set all_atoms;
	all_atoms.fill(instance->n_atoms());
	HSPS::bool_vec no_atoms(false, instance->n_atoms());
	HSPS::bool_vec unreachable_atoms(false, instance->n_atoms());
	HSPS::bool_vec unreachable_actions(false, instance->n_actions());
	if (!opt_preprocess) {
	  prep->compute_reachability(unreachable_atoms, unreachable_actions);
	  unreachable_atoms.complement();
	  unreachable_actions.complement();
	}
	std::ofstream prec_out("prec.dot");
	instance->write_atom_digraph(prec_out, prec, all_atoms, no_atoms,
				     unreachable_atoms,
				     "Atom Precedence Graph");
	prec_out.close();
	HSPS::graph meg;
	prec.strongly_connected_components();
	prec.minimal_equivalent_digraph(meg);
	std::ofstream prec_meg_out("prec-meg.dot");
	instance->write_atom_digraph(prec_meg_out, meg, all_atoms, no_atoms,
				     unreachable_atoms, "Atom Precedence MEG");
	prec_meg_out.close();

// 	HSPS::index_vec d_atm(HSPS::no_such_index, instance->n_atoms());
// 	HSPS::index_vec d_act(HSPS::no_such_index, instance->n_actions());
// 	prep->compute_relevance(instance->goal_atoms, 0, d_atm, d_act);
// 	for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
// 	  for (HSPS::index_type j = 0; j < instance->n_atoms(); j++)
// 	    if ((i != j) && prec.adjacent(i, j)) {
// 	      HSPS::bool_vec b_atm(false, instance->n_atoms());
// 	      HSPS::bool_vec b_act(false, instance->n_actions());
// 	      prep->between(i, j, prec, b_atm, b_act);
// 	      std::ostringstream fname;
// 	      fname << "between_";
// 	      instance->atoms[i].name->write(fname, HSPS::Name::NC_INSTANCE);
// 	      fname << "_";
// 	      instance->atoms[j].name->write(fname, HSPS::Name::NC_INSTANCE);
// 	      fname << ".dot";
// 	      std::ofstream g_out(fname.str().c_str());
// 	      prep->write_relevance_graph(g_out, d_atm, d_act, b_atm, b_act, 0);
// 	      g_out.close();
// 	    }

	HSPS::graph lmg;
	prec.subgraph(lmg, lma);
	lmg.strongly_connected_components();
	lmg.minimal_equivalent_digraph(meg);
	std::ofstream lmg_out("lmg.dot");
	instance->write_atom_digraph(lmg_out, lmg, lma, no_atoms, no_atoms,
				     "Landmark Graph");
	lmg_out.close();
	std::ofstream lmeg_out("lmeg.dot");
	instance->write_atom_digraph(lmeg_out, meg, lma, no_atoms, no_atoms,
				     "Landmark MEG");
	lmeg_out.close();
      }

      if (opt_relevant) {
	stats.start();
	L1C1_relevant_atoms.assign_value(false, instance->n_atoms());
	L1C1_relevant_actions.assign_value(false, instance->n_actions());
	prep->compute_L1C1_relevance(instance->goal_atoms, prec,
				     L1C1_relevant_atoms,
				     L1C1_relevant_actions);
	stats.stop();
	std::cerr << L1C1_relevant_atoms.count(true) << " of "
		  << instance->n_atoms() << " atoms and "
		  << L1C1_relevant_actions.count(true) << " of "
		  << instance->n_actions() << " actions are L1(C1)-relevant"
		  << " (" << stats.time() << " seconds)"
		  << std::endl;
	if (opt_write_graphs) {
	  HSPS::index_vec d_atm(HSPS::no_such_index, instance->n_atoms());
	  HSPS::index_vec d_act(HSPS::no_such_index, instance->n_actions());
	  prep->compute_L1C1_relevance(instance->goal_atoms, 0, prec, d_atm, d_act);
	  std::ofstream rg_out("rg-L1C1.dot");
	  prep->write_relevance_graph(rg_out, d_atm, d_act,
				      "L1(C1) Relevance Graph");
	  rg_out.close();
	  d_atm.assign_value(HSPS::no_such_index, instance->n_atoms());
	  d_act.assign_value(HSPS::no_such_index, instance->n_actions());
	  prep->compute_relevance(instance->goal_atoms, 0, d_atm, d_act);
	  std::ofstream erg_out("erg-L1C1.dot");
	  prep->write_relevance_graph(erg_out, d_atm, d_act,
				      L1C1_relevant_atoms,
				      L1C1_relevant_actions,
				      "Embedded L1(C1) Relevance Graph");
	  erg_out.close();
	}
	if (HSPS::Preprocessor::default_trace_level > 1) {
	  std::cerr << "L1(C1)-relevant actions:" << std::endl;
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	    if (L1C1_relevant_actions[k])
	      std::cerr << " " << instance->actions[k].name << std::endl;
	  std::cerr << "irrelevant actions:" << std::endl;
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	    if (!L1C1_relevant_actions[k])
	      std::cerr << " " << instance->actions[k].name << std::endl;
	  std::cerr << "L1(C1)-relevant atoms:" << std::endl;
	  for (HSPS::index_type k = 0; k < instance->n_atoms(); k++)
	    if (L1C1_relevant_atoms[k])
	      std::cerr << " " << k << ". " << instance->atoms[k].name
			<< std::endl;
	  prep->inconsistency();
	  std::cerr << "mutex relations between L1(C1)-relevant atoms:"
		    << std::endl;
	  for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
	    if (L1C1_relevant_atoms[i])
	      for (HSPS::index_type j = i + 1; j < instance->n_atoms(); j++)
		if (L1C1_relevant_atoms[j])
		  if (prep->inconsistent(i, j))
		    std::cerr << " " << instance->atoms[i].name
			      << " -- "  << instance->atoms[j].name
			      << std::endl;
	}
	LsC1_relevant_atoms.assign_value(false, instance->n_atoms());
	LsC1_relevant_actions.assign_value(false, instance->n_actions());
	stats.start();
	prep->compute_LsC1_relevance(instance->goal_atoms, prec,
				     LsC1_relevant_atoms,
				     LsC1_relevant_actions);
	stats.stop();
	std::cerr << LsC1_relevant_atoms.count(true) << " of "
		  << instance->n_atoms() << " atoms and "
		  << LsC1_relevant_actions.count(true) << " of "
		  << instance->n_actions() << " actions are L*(C1)-relevant"
		  << " (" << stats.time() << " seconds)"
		  << std::endl;
      }
    }

    if (opt_lift) {
      reader->lift_DKEL_Items(*instance);
    }

    if (opt_compose && (instance->n_resources() > 1)) {
      HSPS::mSubsetEnumerator crs(instance->n_resources(),
				  composite_resource_size);
      bool more = crs.first();
      while (more) {
	HSPS::index_set s;
	crs.current_set(s);
	instance->create_composite_resource(s);
	more = crs.next();
      }
    }

    plans = new HSPS::ScheduleSet(*instance);

    if (selected_plan != HSPS::no_such_index) {
      if (selected_plan < reader->n_plans()) {
	HSPS::Schedule* s = new HSPS::Schedule(*instance);
	bool ok =
	  reader->export_plan(selected_plan, *instance, prep->action_map, *s);
	if (!ok) {
	  std::cerr << "error: export plan " << selected_plan << " failed"
		    << std::endl;
	  exit(1);
	}
	plans->add_schedule(s);
      }
      else {
	std::cerr << "error: plan " << selected_plan
		  << " selected but there are only " << reader->n_plans()
		  << " plans in input"
		  << std::endl;
	exit(1);
      }
    }
    else {
      for (HSPS::index_type k = 0; k < reader->n_plans(); k++) {
	HSPS::Schedule* s = new HSPS::Schedule(*instance);
	bool ok = reader->export_plan(k, *instance, prep->action_map, *s);
	if (!ok) {
	  std::cerr << "error: export plan " << k << " failed" << std::endl;
	  exit(1);
	}
	plans->add_schedule(s);
      }
    }
    for (HSPS::index_type k = 0; k < plans->length(); k++)
      if (!(*plans)[k]->plan_name()) {
	(*plans)[k]->set_name(new HSPS::PlanName("input plan", k));
      }

    std::cerr << plans->length() << " plans read" << std::endl;

    // abstraction must be done after loading input plans
    if (opt_abstract || opt_abstract_away) {
      HSPS::index_set abs_keep;

      stats.start();
      std::cerr << "loading atom sets..." << std::endl;

      HSPS::name_vec atom_names(0, 0);
      instance->atom_names(atom_names);
      HSPS::index_set_vec sets;
      reader->export_sets(atom_names, sets);

      if (HSPS::Instance::default_trace_level > 0) {
	for (HSPS::index_type k = 0; k < sets.length(); k++) {
	  std::cerr << "set #" << k << ": ";
	  instance->write_atom_set(std::cerr, sets[k]);
	  std::cerr << std::endl;
	}
      }

      if (selected_set != HSPS::no_such_index) {
	if (selected_set >= sets.length()) {
	  std::cerr << "error: set " << selected_set << " chosen, but only "
		    << sets.length() << " atom sets defined in input"
		    << std::endl;
	  exit(1);
	}
	if (opt_abstract_away) {
	  abs_keep.fill(instance->n_atoms());
	  abs_keep.subtract(sets[selected_set]);
	}
	else {
	  abs_keep = sets[selected_set];
	}
      }
      else {
	if (opt_abstract_away) {
	  HSPS::index_set abs_away;
	  sets.union_set(abs_away);
	  abs_keep.fill(instance->n_atoms());
	  abs_keep.subtract(abs_away);
	}
	else {
	  sets.union_set(abs_keep);
	}
      }

      HSPS::index_vec abs_atom_map;
      HSPS::index_vec abs_action_map;
      std::cerr << "abstracting away "
		<< instance->n_atoms() - abs_keep.length()
		<< " atoms..."
		<< std::endl;
      HSPS::Instance* abs_instance = new HSPS::Instance(instance->name);
      abs_instance->
	abstracted_copy(*instance, abs_keep, abs_atom_map, abs_action_map);
      HSPS::ScheduleSet* abs_plans = new HSPS::ScheduleSet(*abs_instance);
      for (HSPS::index_type k = 0; k < plans->length(); k++) {
	HSPS::Plan* p = abs_plans->new_plan();
	(*plans)[k]->output(*p, abs_action_map);
      }

      delete instance;
      instance = abs_instance;
      delete prep;
      prep = new HSPS::Reduce(*instance, stats);
      delete plans;
      plans = abs_plans;

      stats.stop();
      std::cerr << "finished in " << stats.time() << " seconds" << std::endl;
    }

    // no more modifications to be done to instance, so we can save/set
    // durations, costs, etc.
    instance->save_durations(saved_durations);
    if (opt_unit) {
      for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	instance->actions[k].cost = 1;
	instance->actions[k].dmin = 1;
	instance->actions[k].dmax = 1;
	instance->actions[k].dur = 1;
      }
    }
    else if (opt_round_up) {
      instance->round_durations_up();
    }
    else if (opt_round_down) {
      instance->round_durations_down();
    }
    else if (opt_round) {
      instance->round_durations();
    }
    else if (opt_min_duration > 0) {
      for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	if (instance->actions[k].dur < opt_min_duration)
	  instance->actions[k].dur = opt_min_duration;
    }
    if (opt_unlimited) {
      HSPS::cost_vec saved_res;
      instance->assign_unlimited_resources(saved_res);
    }

    if (opt_cost_bound) {
      instance->set_cost_bound(cost_bound);
    }

    if (opt_unique_names) {
      instance->assign_unique_action_names();
    }
  } // end operations on instance

  if (opt_misc && !opt_sas) {
    for (HSPS::index_type i = 0; i < instance->n_invariants(); i++)
      for (HSPS::index_type j = i+1; j < instance->n_invariants(); j++)
	for (HSPS::index_type ii = 0; ii < instance->invariants[i].set.length(); ii++)
	  for (HSPS::index_type jj = 0; jj < instance->invariants[j].set.length(); jj++)
	    if (prep->consistent(instance->invariants[i].set[ii],
				 instance->invariants[j].set[jj]))
	      std::cerr << instance->atoms[instance->invariants[i].set[ii]].name << " & " << instance->atoms[instance->invariants[j].set[jj]].name << std::endl;
  }

  if (opt_petrify) {
    if (HSPS::Preprocessor::default_trace_level > 1) {
      std::cerr << "petrifying STRIPS instance:" << std::endl;
      instance->write_domain(std::cerr);
      instance->write_problem(std::cerr);
    }
    std::ostringstream fname;
    if (reader->problem_file) {
      char* p0 = strdup(reader->problem_file);
      char* p1 = strrchr(p0, '.');
      if (p1) *p1 = '\0';
      p1 = strrchr(p0, '/');
      if (p1) p0 = p1 + 1;
      fname << p0 << ".net";
    }
    else {
      fname << "petrify.net";
    }
    std::cerr << "writing petri net to " << fname.str() << "..." << std::endl;
    std::ofstream pn_out(fname.str().c_str());
    pn_out << "PEP" << std::endl
	   << "PetriBox" << std::endl
	   << "FORMAT_N" << std::endl
	   << "PL" << std::endl;
    for (HSPS::index_type k = 0; k < instance->n_atoms(); k++) {
      pn_out << k + 1 << "\"";
      instance->atoms[k].name->write(pn_out, HSPS::Name::NC_INSTANCE);
      pn_out << "\"J0";
      if (instance->atoms[k].init)
	pn_out << "M1";
      else
	pn_out << "M0";
      pn_out << std::endl;
    }
    pn_out << "TR" << std::endl;
    for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
      pn_out << k + instance->n_atoms() + 1 << "\"";
      instance->actions[k].name->write(pn_out, HSPS::Name::NC_INSTANCE);
      HSPS::rational rcost =
	N_TO_R(opt_temporal ? instance->actions[k].dur :
	       instance->actions[k].cost);
      pn_out << "\"J0D" << rcost.numerator() << "d" << rcost.divisor()
	     << std::endl;
    }
    pn_out << "TP" << std::endl;
    for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
      for (HSPS::index_type i = 0; i < instance->actions[k].add.length(); i++)
	pn_out << k + instance->n_atoms() + 1 << "<"
	       << instance->actions[k].add[i] + 1
	       << std::endl;
      for (HSPS::index_type i = 0; i < instance->actions[k].pre.length(); i++)
	if (!instance->actions[k].del.contains(instance->actions[k].pre[i]))
	  pn_out << k + instance->n_atoms() + 1 << "<"
		 << instance->actions[k].pre[i] + 1
		 << std::endl;
    }
    pn_out << "PT" << std::endl;
    for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
      for (HSPS::index_type i = 0; i < instance->actions[k].pre.length(); i++)
	pn_out << instance->actions[k].pre[i] + 1 << "<"
	       << k + instance->n_atoms() + 1
	       << std::endl;
    }
    pn_out.close();
    std::cerr << "done" << std::endl;
  }

  if (opt_logic) {
    HSPS::PDDL_Base::formula_vec v(0, 0);
    for (HSPS::index_type k = 0; k < reader->dom_predicates.length(); k++)
      if (!reader->dom_predicates[k]->is_static())
	make_all_ssvcs(reader, reader->dom_predicates[k], v);
    HSPS::index_type n_ssvc = v.length();
    std::cerr << n_ssvc << " simple single-valuedness constraints"
	      << std::endl;

    if (!reader->dom_base_types.empty()) {
      for (HSPS::index_type k = 0; k < reader->dom_base_types.length(); k++)
	make_all_scecs(reader, reader->dom_base_types[k], v);
    }
    else {
      make_all_scecs(reader, 0, v);
    }
    HSPS::index_type n_scec = v.length() - n_ssvc;

    std::cerr << n_scec << " simple category exclusion constraints"
	      << std::endl;

    for (HSPS::index_type k = 0; k < v.length(); k++) {
      std::cout << k + 1 << ". ";
      v[k]->print(std::cout);
      std::cout << std::endl;
      HSPS::PDDL_Base::Formula* vf = make_verification_formula(v[k], reader);
      vf = vf->simplify();
      std::cout << " verify: |- ";
      vf->print(std::cout);
      std::cout << std::endl;
    }
  }

  if (opt_implies) {
    for (HSPS::index_type k = 1; k <= implies_n_premises; k++) {
      HSPS::mSubsetEnumerator e(k, instance->n_atoms());
      bool more = e.first();
      while (more) {
	HSPS::index_set s;
	e.current_set(s);
	for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
	  if (!s.contains(i)) {
	    if (prep->implies(s, i)) {
	      instance->write_atom_set(std::cout, s);
	      std::cout << " -> " << instance->atoms[i].name
			<< std::endl;
	    }
	  }
	more = e.next();
      }
    }
  }

  if (opt_hierarchy) {
    stats.start();
    HSPS::graph c;
    HSPS::index_set_graph h;
    prep->compute_hierarchy(c, h);
    stats.stop();
    std::cerr << "hierarchy: " << h << std::endl;
    std::cerr << h.size() << "-set hierarchy computed in "
	      << stats.time() << " seconds" << std::endl;

    if (opt_write_graphs) {
      std::ofstream c_out("criticality.dot");
      instance->write_atom_digraph(c_out, c, "Atom Criticality");
      c_out.close();

      std::ofstream cl_out("criticality_labeled.dot");
      cl_out << "digraph {" << std::endl;
      cl_out << "concentrate=true;" << std::endl;
      HSPS::equivalence neg(instance->n_atoms());
      for (HSPS::index_type k = 0; k < instance->n_atoms(); k++) {
	if (instance->atoms[k].neg != HSPS::no_such_index)
	  neg.merge(k, instance->atoms[k].neg);
      }
      for (HSPS::index_type k = 0; k < instance->n_atoms(); k++)
	if (neg.canonical(k) == k)
	  cl_out << "A" << k
		 << " [label=\"" << instance->atoms[k].name << "\"];"
		 << std::endl;
      for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	for (HSPS::index_type i = 0; i < instance->actions[k].add.length(); i++) {
	  for (HSPS::index_type j = i+1; j < instance->actions[k].add.length(); j++)
	    if (!neg(instance->actions[k].add[i], instance->actions[k].add[j])) {
	      cl_out << "A" << neg.canonical(instance->actions[k].add[i])
		     << " -> A" << neg.canonical(instance->actions[k].add[j])
		     << " [label=\"" << instance->actions[k].name
		     << "\",dir=both];" << std::endl;
	    }
	  for (HSPS::index_type j = 0; j < instance->actions[k].del.length(); j++)
	    if (!neg(instance->actions[k].add[i], instance->actions[k].del[j])) {
	      cl_out << "A" << neg.canonical(instance->actions[k].add[i])
		     << " -> A" << neg.canonical(instance->actions[k].del[j])
		     << " [label=\"" << instance->actions[k].name
		     << "\",dir=both];" << std::endl;
	    }
	  for (HSPS::index_type j = 0; j < instance->actions[k].pre.length(); j++)
	    if (!neg(instance->actions[k].add[i], instance->actions[k].pre[j])) {
	      cl_out << "A" << neg.canonical(instance->actions[k].add[i])
		     << " -> A" << neg.canonical(instance->actions[k].pre[j])
		     << " [label=\"" << instance->actions[k].name
		     << "\"];" << std::endl;
	    }
	}
	for (HSPS::index_type i = 0; i < instance->actions[k].del.length(); i++) {
	  for (HSPS::index_type j = i+1; j < instance->actions[k].del.length(); j++)
	    if (!neg(instance->actions[k].del[i], instance->actions[k].del[j])) {
	      cl_out << "A" << neg.canonical(instance->actions[k].del[i])
		     << " -> A" << neg.canonical(instance->actions[k].del[j])
		     << " [label=\"" << instance->actions[k].name
		     << "\",dir=both];" << std::endl;
	    }
	  for (HSPS::index_type j = 0; j < instance->actions[k].pre.length(); j++)
	    if (!neg(instance->actions[k].del[i], instance->actions[k].pre[j])) {
	      cl_out << "A" << neg.canonical(instance->actions[k].del[i])
		     << " -> A" << neg.canonical(instance->actions[k].pre[j])
		   << " [label=\"" << instance->actions[k].name
		     << "\"];" << std::endl;
	    }
	}
      }
      cl_out << "}" << std::endl;
      cl_out.close();

      HSPS::graph meg;
      c.minimal_equivalent_digraph(meg);
      std::ofstream cmeg_out("criticality_meg.dot");
      instance->write_atom_digraph(cmeg_out, meg, "Atom Criticality MEG");
      cmeg_out.close();

      std::ofstream h_out("hierarchy.dot");
      instance->write_atom_set_digraph(h_out, h, "Abstraction Hierarchy");
      h_out.close();
    }
  }

  if (opt_subset) {
    HSPS::index_type n = reader->dom_goals.length();
    HSPS::index_type i = 1;
    for (HSPS::index_type m = 1; m <= subset_size; m++) {
      HSPS::mSubsetEnumerator sel(n, m);
      bool more = sel.first();
      while (more) {
	HSPS::index_set g;
	sel.current_set(g);
	std::ostringstream fname;
	if (reader->problem_file) {
	  char* p0 = strdup(reader->problem_file);
	  char* p1 = strrchr(p0, '.');
	  if (p1) *p1 = '\0';
	  p1 = strrchr(p0, '/');
	  if (p1) p0 = p1 + 1;
	  fname << p0 << "-s" << i << ".pddl";
	}
	else {
	  fname << "subset" << i << ".pddl";
	}
	std::cerr << "writing subset " << g << " to " << fname.str()
		  << "..." << std::endl;

	std::ofstream s_out(fname.str().c_str());
	s_out << "(define (problem "
	      << reader->problem_name << "-s" << i << ")"
	      << std::endl;
	s_out << " (:domain " << reader->domain_name << ")" << std::endl;
	reader->write_objects(s_out, true);
	reader->write_init(s_out);
	s_out << " (:goal";
	if (g.length() > 1) s_out << " (and";
	for (HSPS::index_type k = 0; k < g.length(); k++) {
	  s_out << " ";
	  reader->dom_goals[g[k]]->print(s_out);
	}
	if (g.length() > 1) s_out << ")";
	s_out << ")" << std::endl;
	reader->write_metric(s_out);
	s_out << ")" << std::endl;
	s_out.close();

	i += 1;
	more = sel.next();
      }
    }
  }

  if (opt_split) {
    std::cerr << "computing graphs..." << std::endl;
    HSPS::graph cg;
    instance->causal_graph(cg);
    HSPS::graph ucg;
    cg.induced_undirected_graph(ucg);
    ucg.strongly_connected_components();
    if (ucg.n_components() > 1) {
      std::cerr << ucg.n_components()
		<< " independent subproblems, splitting..."
		<< std::endl;
      for (HSPS::index_type k = 0; k < ucg.n_components(); k++) {
	HSPS::index_set p_atms;
	ucg.component_node_set(k, p_atms);
	HSPS::index_set p_acts;
	HSPS::index_vec p_map;
	HSPS::Instance* p_ins = new HSPS::Instance(instance->name);
	p_ins->restricted_copy(*instance, p_atms, HSPS::EMPTYSET, p_acts, p_map);
	std::ostringstream fname;
	char* b = reader->problem_file_basename();
	if (b) {
	  fname << b << "-part" << k + 1 << ".pddl";
	}
	else {
	  fname << "part" << k + 1 << ".pddl";
	}
	std::cerr << "writing subproblem " << k + 1 << " to " << fname.str()
		  << "..." << std::endl;
	std::ofstream s_out(fname.str().c_str());
	p_ins->write_domain(s_out);
	p_ins->write_problem(s_out);
	s_out.close();
	delete p_ins;
      }
    }
    else {
      std::cerr << "no independent subproblems, no split" << std::endl;
    }
  }

  if (opt_partition) {
    HSPS::index_set_graph pg;
    HSPS::index_set goal_nodes;
    instance->partitioning_graph(instance->goal_atoms, pg, goal_nodes);
    for (HSPS::index_type k = 0; k < goal_nodes.length(); k++) {
      HSPS::index_set p_acts;
      HSPS::index_vec p_map;
      HSPS::Instance* p_ins = new HSPS::Instance(instance->name);
      p_ins->restricted_copy(*instance, pg.node_label(goal_nodes[k]),
			     HSPS::EMPTYSET, p_acts, p_map);
      std::ostringstream fname;
      if (reader->problem_file) {
	char* p0 = strdup(reader->problem_file);
	char* p1 = strrchr(p0, '.');
	if (p1) *p1 = '\0';
	p1 = strrchr(p0, '/');
	if (p1) p0 = p1 + 1;
	fname << p0 << "-p" << k << ".pddl";
      }
      else {
	fname << "partition" << k << ".pddl";
      }
      std::cerr << "writing partition " << k << " to " << fname.str()
		<< "..." << std::endl;
      std::ofstream s_out(fname.str().c_str());
      p_ins->write_domain(s_out);
      p_ins->write_problem(s_out);
      s_out.close();
    }
  }

  if (opt_1reg) {
    HSPS::index_set g(instance->goal_atoms);
    HSPS::index_vec e(0, g.length());
    bool done = false;
    HSPS::index_type n = 0;
    while (!done) {
      done = true;
      char* fn = reader->enum_problem_filename("reg", ++n);
      HSPS::bool_vec x(false, instance->n_actions());
      for (HSPS::index_type k = 0; k < g.length(); k++)
	for (HSPS::index_type i = 0; i < instance->atoms[g[k]].add_by.length(); i++)
	  if (i != e[k])
	    x[instance->atoms[g[k]].add_by[i]] = true;
      std::cerr << x.count(true) << " actions excluded, writing problem #"
		<< n + 1 << " to " << fn << "..." << std::endl;
      if (opt_instantiate) {
	HSPS::Instance* ins = instance->copy();
	HSPS::index_vec m(prep->action_map);
	ins->remove_actions(x, m);
	std::ofstream s_out(fn);
	ins->write_domain(s_out);
	ins->write_problem(s_out);
	s_out.close();	
      }
      else {
	std::ofstream s_out(fn);
	reader->write_problem_begin(s_out);
	reader->write_objects(s_out, true);
	reader->write_init(s_out);
	reader->write_goal(s_out);
	reader->write_metric(s_out);
	reader->write_dkel_items(s_out, true);
	for (HSPS::index_type i = 0; i < instance->n_actions(); i++) if (x[i])
	  s_out << " (:irrelevant :tag one-step-regression :action "
		<< instance->actions[i].name << ")" << std::endl;
	reader->write_end(s_out);
	s_out.close();
      }
      HSPS::index_type j = 0;
      while (j < g.length()) {
	if (e[j] + 1 < instance->atoms[g[j]].add_by.length()) {
	  e[j] += 1;
	  for (HSPS::index_type i = 0; i < j; i++)
	    e[i] = 0;
	  done = false;
	  j = g.length();
	}
	j += 1;
      }
    }
    std::cerr << n << " 1-step regressed problems written" << std::endl;
  }

  if (opt_soft && opt_decide) {
    NTYPE e = ((HSPS::SoftInstance*)instance)->compute_epsilon();
    std::cerr << "b = " << decide_min_value
	      << ", e = " << e
	      << ", b + e = " << decide_min_value + e
	      << std::endl;
    if (opt_decide_strict) decide_min_value += e;
    HSPS::CostTable* h = new HSPS::CostTable(*instance, stats);
    HSPS::ACF* acf = (opt_value_only ?
		      (HSPS::ACF*)new HSPS::ZeroACF() :
		      (HSPS::ACF*)new HSPS::CostACF(*instance));
    if (opt_H3) {
      h->compute_H3(*acf);
    }
    else {
      h->compute_H2(*acf);
    }
    if (opt_load_heuristic) {
      reader->export_heuristic(*instance, prep->atom_map, true, *h);
    }

    stats.start();
    HSPS::index_type n = 0;
    HSPS::DecisionProblemEnumerator dpe(*((HSPS::SoftInstance*)instance), *h,
					decide_min_value);
    bool more = dpe.first();
    while (more && ((n < n_max) || (n_max == HSPS::no_such_index))) {
      std::ostringstream fname;
      if (reader->problem_file) {
	char* p0 = strdup(reader->problem_file);
	char* p1 = strrchr(p0, '.');
	if (p1) *p1 = '\0';
	p1 = strrchr(p0, '/');
	if (p1) p0 = p1 + 1;
	fname << p0 << "-d" << n << ".pddl";
      }
      else {
	fname << "decision_problem_" << n << ".pddl";
      }
      std::cerr << "writing decision problem " << n
		<< " to " << fname.str() << "...";
      std::ofstream s_out(fname.str().c_str());

      s_out << ";; " << instance->name << ", decision problem #" << n
	    << std::endl;
      s_out << ";; soft goals: ";
      ((HSPS::SoftInstance*)instance)->write_soft_goal_set
	(s_out, dpe.current_soft_goals());
      s_out << std::endl;
      s_out << ";; goal atoms: ";
      instance->write_atom_set(s_out, dpe.current_goal_atoms());
      s_out << std::endl;
      s_out << ";; value = " << dpe.current_value()
	    << ", min est. cost = " << dpe.current_min_cost()
	    << ", max NB = " << dpe.current_max_nb()
	    << std::endl;
      s_out << ";; min NB req. = " << dpe.current_min_nb()
	    << ", cost limit = " << dpe.current_max_cost()
	    << std::endl;

      if (!opt_instantiate) {
	HSPS::bool_vec sel(false, reader->dom_preferences.length());
	for (HSPS::index_type k = 0; k < dpe.current_soft_goals().length(); k++) {
	  HSPS::PDDL_Base::Preference* p =
	    (HSPS::PDDL_Base::Preference*)((HSPS::SoftInstance*)instance)->
	    soft[dpe.current_soft_goals()[k]].src;
	  HSPS::index_type i = reader->dom_preferences.first(p);
	  if (i == HSPS::no_such_index) {
	    std::cerr << "program error: no preference corresponding to "
		      << k << "th soft goal found" << std::endl;
	    std::cerr << "current set: ";
	    ((HSPS::SoftInstance*)instance)->write_soft_goal_set
	      (std::cerr, dpe.current_soft_goals());
	    std::cerr << std::endl;
	    std::cerr << "src = " << p << std::endl;
	    exit(255);
	  }
	  sel[i] = true;
	}

	HSPS::PDDL_Base::goal_vec original_goals =
	  reader->dom_goals;
	HSPS::PDDL_Base::preference_vec original_prefs =
	  reader->dom_preferences;
	HSPS::PDDL_Base::Expression* original_metric = reader->metric;
	HSPS::PDDL_Base::metric_class original_metric_type = reader->metric_type;
	if (opt_value_only) {
	  reader->metric_type = HSPS::PDDL_Base::metric_none;
	  reader->metric = 0;
	}
	else {
	  reader->metric = original_metric->copy();
	}

	reader->select_preferences(sel);

	if (!opt_value_only) {
	  reader->metric = reader->metric->simplify();
	  reader->metric_to_goal(dpe.current_max_cost());
	}

	HSPS::Instance::write_PDDL2 = true;
	HSPS::Instance::write_PDDL3 = true;
	HSPS::Instance::write_metric = true;
	reader->write_dkel_problem(s_out, false);

	if (reader->metric) delete reader->metric;
	reader->metric = original_metric;
	reader->metric_type = original_metric_type;
	reader->dom_goals = original_goals;
	reader->dom_preferences = original_prefs;
      }
      else {
	HSPS::Instance* dec = new HSPS::Instance(instance->name);
	dpe.create_decision_problem(*dec);

	HSPS::Instance::write_PDDL2 = true;
	HSPS::Instance::write_PDDL3 = false;
	HSPS::Instance::write_metric = false;
	dec->write_domain_init(s_out);
	dec->write_domain_declarations(s_out);
	dec->write_domain_actions(s_out);
	dec->write_domain_DKEL_items(s_out);
	if (HSPS::Instance::write_extra) {
	  HSPS::name_vec set_names(0, 0);
	  HSPS::index_set_vec sets;
	  reader->export_action_partitions(set_names, sets);
	  s_out << ";; action partitions" << std::endl;
	  for (HSPS::index_type k = 0; k < sets.length(); k++) {
	    instance->remap_set(sets[k], prep->action_map);
	    instance->write_domain_action_set(s_out, sets[k], set_names[k]);
	  }
	}
	s_out << ")";
	dec->write_problem(s_out);

	delete dec;
      }
      s_out.close();

      std::cerr << " (" << stats << ")" << std::endl;

      n += 1;
      more = dpe.next();
    }
    stats.stop();
    std::cerr << n << " decision problems generated in "
	      << stats.time() << " seconds" << std::endl;
  }

  if (opt_cnf && !opt_sas) {
    std::cout << "p cnf " << instance->n_atoms()
	      << " " << instance->n_actions()
	      << std::endl;
    for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
      for (HSPS::index_type i = 0; i < instance->actions[k].pre.length(); i++)
	std::cout << " " << instance->actions[k].pre[i] + 1;
      for (HSPS::index_type i = 0; i < instance->actions[k].add.length(); i++)
	std::cout << " " << instance->actions[k].add[i] + 1;
      for (HSPS::index_type i = 0; i < instance->actions[k].del.length(); i++)
	std::cout << " " << instance->actions[k].del[i] + 1;
      std::cout << " 0" << std::endl;
    }
  }

  if (opt_redop) {
    if (opt_print) {
      reader->write_problem_begin(std::cout);
      if (!opt_add) {
	reader->write_objects(std::cout, true);
	reader->write_init(std::cout);
	reader->write_goal(std::cout);
	reader->write_metric(std::cout);
	reader->write_dkel_items(std::cout, true);
      }
      if (opt_replace) {
	HSPS::Result res;
	HSPS::ActionSequenceSet imps;
	res.set_stop_condition(HSPS::Result::stop_at_all);
	res.set_plan_set(&imps);
	for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	  NTYPE c_imp = prep->implement(k, reduce_bound, res);
	  if (c_imp == 0) {
	    std::cout << ";; action " << instance->actions[k].name
		      << " is USELESS" << std::endl;
	    std::cout << " (:irrelevant :tag redop :tag useless :action "
		      << instance->actions[k].name << ")" << std::endl;
	  }
	  else if (FINITE(c_imp)) {
	    std::cout << ";; action " << instance->actions[k].name
		      << " is REDUNDANT - " << imps.length()
		      << " replacements of length " << c_imp
		      << " found" << std::endl;
	    for (HSPS::index_type m = 0; m < imps.length(); m++) {
	      std::cout << " (:replaceable" << std::endl
			<< "  :tag redop" << std::endl
			<< "  :replaced (" << instance->actions[k].name
			<< ")" << std::endl
			<< "  :replacing (";
	      const HSPS::index_vec& plan = imps[m];
	      for (HSPS::index_type p = 0; p < plan.length(); p++)
		std::cout << instance->actions[plan[p]].name;
	      std::cout << "))"
			<< " ; length = " << plan.length()
			<< std::endl;
	    }
	  }
	  else {
	    std::cout << ";; action " << instance->actions[k].name
		      << " is NOT replaceable" << std::endl;
	  }
	  imps.clear();
	}
      }
      else {
	HSPS::bool_vec deleted_actions;
	prep->reduced_action_set(reduce_bound, deleted_actions);
	for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	  if (deleted_actions[k])
	    std::cout << " (:irrelevant :tag redop :tag redundant :action "
		      << instance->actions[k].name << ")" << std::endl;
	}
      }
      reader->write_end(std::cout);
    }
    else {
      HSPS::index_pair c = prep->count_useless_and_redundant_actions(reduce_bound);
      std::cout << instance->name << ": "
		<< c.first + c.second << " redundant actions ("
		<< c.first << " useless, "
		<< c.second << " proper redundant)"
		<< std::endl;
    }
  }

  if (opt_random) {
    for (HSPS::index_type k = 0; k < n_plans; k++) {
      HSPS::Schedule* plan = (HSPS::Schedule*)plans->new_plan();
      plan->random_sequence(max_length, avg_length, false, 0, rng);
    }
  }

  if (opt_analyse) {
    std::cerr << "analyzing " << plans->length() << " plans..." << std::endl;
    stats.start();

    HSPS::cost_vec current_durations;
    if (opt_untimed) {
      instance->save_durations(current_durations);
      instance->assign_unit_durations(unit_value);
    }
    else {
      instance->set_durations(saved_durations, current_durations);
    }

    for (HSPS::index_type k = 0; k < plans->length(); k++) {
      HSPS::graph p;
      (*plans)[k]->deorder(p);
      HSPS::graph p1(p);
      p1.transitive_reduction();
      // HSPS::graph p2(p);
      // p2.transitive_closure();
      // p2.transitive_reduction();
      // HSPS::pair_set d0;
      // HSPS::pair_set d1;
      // HSPS::index_vec_util c;
      // c.fill(p1.size());
      // p1.difference(p2, c, d0, d1);
      // if (!d0.empty() || !d1.empty()) {
      // 	std::cerr << "error: transitive reductions p1 and p2 not equal"
      // 		  << std::endl;
      // 	std::cerr << "p = " << p << std::endl;
      // 	std::cerr << "p1 = " << p1 << std::endl;
      // 	std::cerr << "p2 = " << p2 << std::endl;
      // 	std::cerr << "d0 = " << d0 << std::endl;
      // 	std::cerr << "d1 = " << d1 << std::endl;
      // 	exit(1);
      // }
      p.transitive_closure();
      (*plans)[k]->add_trait(new HSPS::PlanPrecedenceRelation((*plans)[k], p));
      HSPS::index_type c = 0;
      for (HSPS::index_type i = 0; i < p.size(); i++)
	for (HSPS::index_type j = i + 1; j < p.size(); j++)
	  if (!p.adjacent(i, j) && !p.adjacent(j, i))
	    c += 2;
      HSPS::rational f1 = HSPS::rational(c, p.size());
      HSPS::rational nf1 = f1 * HSPS::rational(1, p.size() - 1);
      // HSPS::index_type m = (p.size() * (p.size() - 1)) / 2;
      // HSPS::rational f2 = (m - p.n_edges());
      // HSPS::rational nf2 = f2 * HSPS::rational(1, p.size());

      std::cerr << "plan #" << k << " (" << (*plans)[k]->plan_name() << "):"
		<< std::endl;
      std::cerr << "flex = " << f1 << std::endl;
      std::cerr << "flex (normalised) = " << nf1 << std::endl;
      // std::cerr << "flex (2) = " << f2 << std::endl;
      // std::cerr << "flex (2) normalised = " << nf2 << std::endl;

      if (opt_write_graphs) {
	std::ostringstream gf_name;
	gf_name << "input-plan-" << k << ".dot";
	std::ofstream g_out(gf_name.str().c_str());
	HSPS::name_vec a_names;
	(*plans)[k]->step_action_names(a_names);
	HSPS::write_labeled_digraph<HSPS::name_vec>
	  (g_out, p1, a_names, false, (*plans)[k]->plan_name()->to_cstring());
	g_out.close();

	HSPS::weighted_graph tp;
	(*plans)[k]->deorder(tp);
	tp.transitive_reduction();
	std::ostringstream wgf_name;
	wgf_name << "input-plan-" << k << "-timing.dot";
	std::ofstream wg_out(wgf_name.str().c_str());
	tp.write_digraph(wg_out, true, false, true, true,
			 (*plans)[k]->plan_name()->to_cstring());
	wg_out.close();

	HSPS::index_graph dg;
	HSPS::name_vec oc_names(0, 0);
	compute_plan_dependency_graph(*instance, (*plans)[k]->step_actions(),
				      p, instance->goal_atoms, dg, &oc_names);
	std::ostringstream dgf_name;
	dgf_name << "input-plan-" << k << "-dg.dot";
	std::ofstream dg_out(dgf_name.str().c_str());
	HSPS::write_labeled_digraph<HSPS::name_vec>
	  (dg_out, dg, oc_names, true, (*plans)[k]->plan_name()->to_cstring());
	dg_out.close();
      }

      if (opt_MATLAB) {
	HSPS::index_graph g;
	(*plans)[k]->base_precedence_graph(g);
	const HSPS::index_vec& a = (*plans)[k]->step_actions();
	assert(a.length() == g.size());
	for (HSPS::index_type i = 0; i < a.length(); i++)
	  g.node_label(i) = a[i];
	g.transitive_reduction();
	for (HSPS::index_type i = 0; i < g.size(); i++) {
	  HSPS::index_vec d;
	  g.distance(i, d);
	  for (HSPS::index_type j = 0; j < g.size(); j++)
	    if ((i != j) && !g.adjacent(i, j) && (d[j] != HSPS::no_such_index)) {
	      g.add_edge(i, j);
	      g.edge_label(i, j) = d[j];
	    }
	}
	g.write_MATLAB(std::cout, (*plans)[k]->plan_name()->to_cstring(), "named");
      }
    }

    if (opt_analyse_and_filter) {
      plans->reduce_plans();

      if (opt_write_graphs) {
	for (HSPS::index_type k = 0; k < plans->length(); k++) {
	  HSPS::PlanPrecedenceRelation* pp =
	    (HSPS::PlanPrecedenceRelation*)
	    (*plans)[k]->find_trait("PlanPrecedenceRelation");
	  assert(pp);
	  std::ostringstream gf_name;
	  gf_name << "reduced-plan-" << k << ".dot";
	  std::ofstream g_out(gf_name.str().c_str());
	  HSPS::name_vec a_names;
	  (*plans)[k]->step_action_names(a_names);
	  HSPS::write_labeled_digraph<HSPS::name_vec>
	    (g_out, pp->precedence_relation(), a_names, false,
	     (*plans)[k]->plan_name()->to_cstring());
	  g_out.close();
	}
      }

      plans->filter_unschedulable_plans();
      plans->filter_equivalent_plans();

      std::cerr << "found " << plans->length()
		<< " (logically) valid reduced non-equivalent plans"
		<< std::endl;
    }

    instance->set_durations(current_durations);

    stats.stop();
    std::cerr << "finished in " << stats.time() << " seconds" << std::endl;
  }

  if (opt_schedule) {
    std::cerr << "(re-)scheduling " << plans->length() << " plans..."
	      << std::endl;
    stats.start();
    HSPS::index_type n_ok = 0;
    HSPS::ScheduleSet* new_plans = new HSPS::ScheduleSet(*instance);
    for (HSPS::index_type k = 0; k < plans->length(); k++) {
      std::cerr << "(re-)scheduling " << (*plans)[k]->plan_name() << "..."
		<< std::endl;
      HSPS::PlanPrecedenceRelation* r =
	(HSPS::PlanPrecedenceRelation*)
	(*plans)[k]->find_trait("PlanPrecedenceRelation");
      assert(r);
      assert(r->precedence_relation().size() ==
	     (*plans)[k]->step_actions().length());
      HSPS::graph_vec rfps;
      std::cerr << " - computing feasible precedences... ";
      bool ok = feasible(*instance,
			 (*plans)[k]->step_actions(),
			 r->precedence_relation(),
			 &rfps);
      std::cerr << rfps.length() << " found" << std::endl;
      if (!ok) {
	assert(rfps.length() == 0);
	std::cerr << " - no feasible ordering, can't schedule plan"
		  << std::endl;
      }
      else {
	assert(rfps.length() > 0);
	n_ok += 1;
      }
      for (HSPS::index_type i = 0; i < rfps.length(); i++) {
	HSPS::index_vec_util c;
	c.fill(r->precedence_relation().size());
	HSPS::pair_set missing_edges;
	HSPS::pair_set extra_edges;
	r->precedence_relation().difference(rfps[i], c, missing_edges, extra_edges);
	if (!missing_edges.empty()) {
	  std::cerr << "error: extension of precedence graph lacks edges: "
		    << missing_edges << std::endl;
	  std::cerr << "original graph: " << r->precedence_relation()
		    << std::endl;
	  std::cerr << "extended graph: " << rfps[i] << std::endl;
	  exit(1);
	}
	std::cerr << " - extra edges in extension of precedence graph: "
		  << extra_edges << std::endl;
	HSPS::mapping m;
	HSPS::Schedule* s = (HSPS::Schedule*)new_plans->new_plan();
	ok = s->construct_minimal_makespan((*plans)[k]->step_actions(), rfps[i], m);
	if (!ok) {
	  std::cerr << "error: scheduling with feasible ordering failed"
		    << std::endl;
	  std::cerr << "actions: " << (*plans)[k]->step_actions() << std::endl;
	  std::cerr << "precedence graph: " << rfps[i] << std::endl;
	  exit(1);
	}
	s->set_name(new HSPS::PlanName((*plans)[k]->plan_name(), "rescheduling", i));
	s->add_trait(new HSPS::PlanPrecedenceRelation(s, rfps[i], m));
	std::cerr << "plan #" << k << " (" << (*plans)[k]->plan_name()
		  << ") makespan = "
		  << (*plans)[k]->makespan()
		  << ", rescheduling makespan = " << s->makespan()
		  << std::endl;
	if (opt_check) {
	  NTYPE ms = s->makespan();
	  std::cerr << " - makespan = " << ms << ", checking..." << std::endl;
	  // HSPS::PermutationEnumerator e((*plans)[k]->step_actions().length());
	  HSPS::index_vec v0(0, (*plans)[k]->step_actions().length());
	  HSPS::index_vec v1(0, (*plans)[k]->step_actions().length());
	  HSPS::CorrespondanceEnumerator e(v0, v1);
	  bool more = e.first();
	  while (more) {
	    HSPS::index_vec a_check;
	    a_check.assign_remap((*plans)[k]->step_actions(), e.current());
	    HSPS::graph p_check;
	    p_check.copy_and_rename(rfps[i], e.current());
	    HSPS::Schedule* s_check = new HSPS::Schedule(*instance);
	    HSPS::mapping m_check;
	    bool check1_ok =
	      s_check->construct_minimal_makespan(a_check, p_check, m_check);
	    assert(check1_ok);
	    if (s_check->makespan() != ms) {
	      std::cerr << "check failed: rescheduling min-makespan not equal"
			<< std::endl;
	      std::cerr << "actions:" << (*plans)[k]->step_actions()
			<< std::endl;
	      std::cerr << "prec. graph:" << rfps[i]
			<< std::endl;
	      std::cerr << "schedule:" << std::endl;
	      s->write(std::cerr);
	      std::cerr << "permutation: " << e.current() << std::endl;
	      std::cerr << "actions:" << a_check << std::endl;
	      std::cerr << "prec. graph:" << p_check << std::endl;
	      std::cerr << "schedule:" << std::endl;
	      s_check->write(std::cerr);
	      exit(1);
	    }
	    else {
	      std::cerr << " - permutation " << e.current() << " ok"
			<< std::endl;
	    }
	    delete s_check;
	    more = e.next();
	  }
	}
      }
    }
    stats.stop();
    std::cerr << n_ok << " of " << plans->length() << " plans feasible, "
	      << new_plans->length() << " schedules created ("
	      << stats.time() << " seconds)"
	      << std::endl;
    if (opt_schedule_explore_options) {
      std::cerr << "exploring more scheduling options..." << std::endl;
      HSPS::index_type n = new_plans->length();
      stats.start();
      new_plans->explore_options(opt_seo_xdc, opt_seo_sdc);
      stats.stop();
      std::cerr << new_plans->length() - n
		<< " additional schedules created ("
		<< stats.time() << " seconds)"
		<< std::endl;
    }
    delete plans;
    plans = new_plans;
    opt_untimed = false;
  }

  if (opt_resequence) {
    std::cerr << "re-sequencing " << plans->length() << " plans..."
	      << std::endl;
    stats.start();
    HSPS::index_type n = plans->length();
    for (HSPS::index_type k = 0; k < n; k++) {
      plans->sequential_variations((*plans)[k], reseq_d_min, reseq_d_max);
    }
    stats.stop();
    std::cerr << plans->length() - n << " new plans created in "
	      << stats.time() << " seconds" << std::endl;
  }

  if (opt_resequence_random) {
    std::cerr << "re-sequencing " << plans->length() << " plans..."
	      << std::endl;
    stats.start();
    HSPS::index_type n = plans->length();
    for (HSPS::index_type k = 0; k < n; k++) {
      plans->random_sequential_variations((*plans)[k],
					  reseq_d_min, reseq_d_max,
					  reseq_n, rng);
    }
    stats.stop();
    std::cerr << plans->length() - n << " new plans created in "
	      << stats.time() << " seconds" << std::endl;
  }

  if (opt_validate) {
    std::cerr << "validating " << plans->length() << " plans..."
	      << std::endl;
    HSPS::cost_vec current_durations;
    instance->save_durations(current_durations);
    if (opt_untimed) instance->assign_unit_durations(unit_value);
    if (opt_soft) {
      NTYPE max_value = NEG_INF;
      NTYPE max_nb = NEG_INF;
      for (HSPS::index_type k = 0; k < plans->length(); k++) {
	if (HSPS::Instance::default_trace_level > 0) {
	  (*plans)[k]->write(std::cerr);
	}
	HSPS::ExecTrace* trace = new HSPS::ExecTrace(*instance);
	HSPS::ExecErrorSet* errors = new HSPS::ExecErrorSet();
	(*plans)[k]->simulate(trace, errors, true);
	HSPS::SoftInstance* sinstance = (HSPS::SoftInstance*)instance;
	NTYPE value = sinstance->null_value;
	HSPS::index_type count = 0;
	if (errors->executable()) {
	  bool ok = true;
	  HSPS::ExecState* final_exec_state = trace->final_state();
	  assert(final_exec_state);
	  if (!final_exec_state->check_atoms(sinstance->hard)) {
	    ok = false;
	    if (HSPS::Instance::default_trace_level > 0) {
	      std::cerr << "plan " << (*plans)[k]->plan_name()
			<< " fails to achieve hard goals" << std::endl;
	    }
	  }
	  if (ok) {
	    if (HSPS::Instance::default_trace_level > 0) {
	      std::cerr << "plan " << (*plans)[k]->plan_name()
			<< " ok, evaluating..." << std::endl;
	    }
	    for (HSPS::index_type i = 0; (i < sinstance->n_soft()) && ok; i++) {
	      if (final_exec_state->check_atoms(sinstance->soft[i].atoms)) {
		if (HSPS::Instance::default_trace_level > 0) {
		  std::cout << ";; soft goal ";
		  if (sinstance->soft[i].name) {
		    std::cout << sinstance->soft[i].name << ": ";
		  }
		  sinstance->write_atom_set(std::cerr, sinstance->soft[i].atoms);
		  std::cout << " achieved; value +"
			    << PRINT_NTYPE(sinstance->soft[i].weight)
			    << std::endl;
		}
		value += sinstance->soft[i].weight;
		count += 1;
	      }
	      else if (HSPS::Instance::default_trace_level > 1) {
		std::cout << ";; soft goal ";
		if (sinstance->soft[i].name) {
		  std::cout << sinstance->soft[i].name << ": ";
		}
		sinstance->write_atom_set(std::cerr, sinstance->soft[i].atoms);
		std::cout << " (value +"
			  << PRINT_NTYPE(sinstance->soft[i].weight)
			  << ") NOT achieved"
			  << std::endl;
	      }
	    }
	    std::cout << ";; plan " << (*plans)[k]->plan_name()
		      << " ok: "
		      << count << "/" << sinstance->n_soft()
		      << " goals achieved"
		      << ", value = " << PRINT_NTYPE(value)
		      << ", cost = " << PRINT_NTYPE((*plans)[k]->cost())
		      << ", net benefit = "
		      << PRINT_NTYPE(value - (*plans)[k]->cost())
		      << std::endl;
	    max_value = MAX(value, max_value);
	    max_nb = MAX(value - (*plans)[k]->cost(), max_nb);
	  }
	}
	else {
	  std::cout << ";; plan " << (*plans)[k]->plan_name()
		    << " failed: " << errors->length() << " errors: ";
	  errors->write(std::cout);
	  std::cout << std::endl;
	}
      }
      std::cout << ";; " << instance->name << ": "
		<< plans->length()
		<< " plans checked (max value = " << PRINT_NTYPE(max_value)
		<< ", max net benefit = " << PRINT_NTYPE(max_nb) << ")"
		<< std::endl;
    }
    else {
      HSPS::index_type min_length = HSPS::no_such_index;
      NTYPE min_makespan = POS_INF;
      NTYPE min_cost = POS_INF;
      HSPS::bool_vec ok(true, plans->length());
      for (HSPS::index_type k = 0; k < plans->length(); k++) {
	HSPS::ExecTrace* trace = new HSPS::ExecTrace(*instance);
	HSPS::ExecErrorSet* errors = new HSPS::ExecErrorSet();
	ok[k] = (opt_validate_low_res ?
		 (*plans)[k]->simulate_low_resolution(trace, errors, true) :
		 (*plans)[k]->simulate(trace, errors, true));
	if (ok[k]) {
	  std::cout << ";; plan " << (*plans)[k]->plan_name() << " ok"
		    << std::endl;
	  if (errors->length() > 0) {
	    std::cout << ";; " << errors->length() << " warnings: ";
	    errors->write(std::cout);
	    std::cout << std::endl;
	  }
	  std::cout << ";; plan " << (*plans)[k]->plan_name()
		    << ": length = " << (*plans)[k]->length()
		    << ", makespan = " << (*plans)[k]->makespan()
		    << ", l/m ratio = "
		    << (I_TO_N((*plans)[k]->length())/(*plans)[k]->makespan())
		    << ", cost = " << (*plans)[k]->cost()
		    << std::endl;
	  if ((*plans)[k]->length() < min_length)
	    min_length = (*plans)[k]->length();
	  min_makespan = MIN((*plans)[k]->makespan(), min_makespan);
	  min_cost = MIN((*plans)[k]->cost(), min_cost);
	}
	else {
	  std::cout << ";; plan " << (*plans)[k]->plan_name() << " failed"
		    << std::endl;
	  std::cout << ";; " << errors->length() << " errors: ";
	  errors->write(std::cout);
	  std::cout << std::endl;
	}
	if (opt_print_trace) {
	  std::cout << "execution trace:" << std::endl;
	  trace->write(std::cout);
	  std::cout << "necessary trace:" << std::endl;
	  HSPS::ExecTrace* t_nec = trace->necessary_trace();
	  t_nec->write(std::cout);
	  delete t_nec;
	}
	if (opt_resource) {
	  for (HSPS::index_type k = 0; k < instance->n_resources(); k++) {
	    HSPS::ResourceProfile* p =
	      new HSPS::ResourceProfile(*instance, k, *trace);
	    if (opt_gantt) {
	      instance->resources[k].name->write(std::cout, HSPS::Name::NC_LATEX);
	      std::cout << std::endl << std::endl;
	      p->writeGantt(std::cout);
	      std::cout << std::endl;
	    }
	    else {
	      std::cout << "resource " << instance->resources[k].name
			<< " (#" << k << "):"
			<< std::endl;
	      p->write(std::cout);
	      NTYPE r = (p->peak_use() / instance->resources[k].init);
	      std::cout << "peak use = " << PRINT_NTYPE(p->peak_use())
			<< " (" << PRINT_NTYPE(r*100) << "% capacity)"
			<< std::endl;
	    }
	    delete p;
	  }
	}
	delete trace;
	delete errors;
      }
      std::cout << ";; " << plans->length() << " plans validated: "
		<< ok.count(true) << " valid, "
		<< plans->length() - ok.count(true) << " failed"
		<< std::endl;
      if (ok.count(true) > 0) {
	std::cout << ";; min length = " << min_length << std::endl
		  << ";; min makespan = " << min_makespan << std::endl
		  << ";; min cost = " << min_cost << std::endl;
      }
      ok.complement();
      plans->remove(ok);
    }
    if (opt_untimed) instance->set_durations(current_durations);
  }

  if (opt_analyse) {
    if (plans->length() > 0) {
      std::cerr << "analyzing " << plans->length() << " plans..." << std::endl;
      stats.start();
      if (opt_distinguishing_traits) {
	plans->add_distinguishing_traits_1();
	if (!opt_untimed) {
	  plans->add_distinguishing_traits_2();
	}
      }
      if (opt_soft) {
	HSPS::cost_vec val(0, 0);
	((HSPS::SoftInstance*)instance)->eval_plan_set(*plans, val);
	HSPS::cost_set vset;
	vset.assign_values(val);
	assert(vset.length() > 0);
	if (vset.length() > 1) {
	  for (HSPS::index_type k = 0; k < vset.length(); k++) {
	    HSPS::bool_vec s;
	    val.find(vset[k], s);
	    std::cerr << "NB = " << vset[k] << ":";
	    for (HSPS::index_type i = 0; i < plans->length(); i++)
	      if (s[i]) std::cerr << " plan #" << i;
	    std::cerr << std::endl;
	    HSPS::ScheduleSet* p_in = new HSPS::ScheduleSet(*plans, s);
	    s.complement();
	    HSPS::ScheduleSet* p_out = new HSPS::ScheduleSet(*plans, s);
	    HSPS::pair_set d_in;
	    HSPS::pair_set d_out;
	    p_in->separating_precedence_constraints(*p_out, d_in, d_out);
	    std::cerr << "d[s]:" << std::endl;
	    for (HSPS::index_type i = 0; i < d_in.length(); i++) {
	      HSPS::index_type a1 =
		(*p_in)[0]->step_actions()[d_in[i].first];
	      HSPS::index_type a2 =
		(*p_in)[0]->step_actions()[d_in[i].second];
	      std::cerr << "  " << instance->actions[a1].name
			<< " < " << instance->actions[a2].name
			<< std::endl;
	    }
	    std::cerr << "d[not s]:" << std::endl;
	    for (HSPS::index_type i = 0; i < d_out.length(); i++) {
	      HSPS::index_type a1 =
		(*p_out)[0]->step_actions()[d_out[i].first];
	      HSPS::index_type a2 =
		(*p_out)[0]->step_actions()[d_out[i].second];
	      std::cerr << "  " << instance->actions[a1].name
			<< " < " << instance->actions[a2].name
			<< std::endl;
	    }
	    delete p_in;
	    delete p_out;
	  }
	}
	else {
	  std::cerr << "skipping analysis as there is only one NB value ("
		    << vset[0] << ")" << std::endl;
	}
      }
      if (opt_write_graphs || opt_MATLAB) {
	for (HSPS::index_type k = 0; k < plans->length(); k++) {
	  HSPS::index_graph pxg;
	  HSPS::index_graph ptg;
	  HSPS::name_vec nn;
	  compute_plan_exec_graph(*instance, *((*plans)[k]), pxg, nn);
	  if (opt_write_graphs) {
	    std::ostringstream xgf_name;
	    xgf_name << "input-plan-" << k << "-pxg.dot";
	    std::ofstream xg_out(xgf_name.str().c_str());
	    HSPS::write_styled_digraph<HSPS::name_vec>
	      (xg_out, pxg, nn, false, (*plans)[k]->plan_name()->to_cstring());
	    xg_out.close();
	    std::ostringstream bxgf_name;
	    bxgf_name << "input-plan-" << k << "-bxg.dot";
	    std::ofstream bxg_out(bxgf_name.str().c_str());
	    pxg.write_styled_digraph(bxg_out, false,
				     (*plans)[k]->plan_name()->to_cstring());
	    bxg_out.close();
	  }
	  if (opt_MATLAB) {
	    pxg.reflect();
	    pxg.write_MATLAB(std::cout,
			     (*plans)[k]->plan_name()->to_cstring(), "pxg");
	  }
	  compute_plan_transition_graph(*instance, *((*plans)[k]), ptg, nn);
	  if (ptg.size() > 0) {
	    if (opt_write_graphs) {
	      std::ostringstream tgf_name;
	      tgf_name << "input-plan-" << k << "-ptg.dot";
	      std::ofstream tg_out(tgf_name.str().c_str());
	      HSPS::write_styled_digraph<HSPS::name_vec>
		(tg_out,ptg,nn,false,(*plans)[k]->plan_name()->to_cstring());
	      tg_out.close();
	      std::ostringstream btgf_name;
	      btgf_name << "input-plan-" << k << "-btg.dot";
	      std::ofstream btg_out(btgf_name.str().c_str());
	      ptg.write_styled_digraph(btg_out, false,
				       (*plans)[k]->plan_name()->to_cstring(),
				       HSPS::no_such_index);
	      btg_out.close();
	    }
	    if (opt_MATLAB) {
	      ptg.reflect();
	      ptg.write_MATLAB(std::cout,
			       (*plans)[k]->plan_name()->to_cstring(), "ptg");
	    }
	  }
	}
      }
      stats.stop();
      std::cerr << "finished in " << stats.time() << " seconds" << std::endl;
    }
    else {
      std::cerr << "skipping analysis as there are no plans" << std::endl;
    }
  }

  if (opt_difference) {
    std::cerr << "simulating " << plans->length() << " plans..." << std::endl;
    HSPS::lvector<HSPS::ExecTrace*> trajs(0, 0);
    for (HSPS::index_type k = 0; k < plans->length(); k++) {
      HSPS::ExecTrace* trace = new HSPS::ExecTrace(*instance);
      bool ok = (*plans)[k]->simulate(trace, 0, true);
      trajs.append(trace);
    }
    std::cerr << "analyzing " << trajs.length() << " traces..." << std::endl;

    HSPS::index_vec count_always(0, instance->n_atoms());
    HSPS::index_set atoms_always;
    for (HSPS::index_type p = 0; p < instance->n_atoms(); p++) {
      for (HSPS::index_type k = 0; k < trajs.length(); k++) {
	if (trajs[k]->test_always(p)) {
	  if (HSPS::Instance::default_trace_level > 0) {
	    std::cerr << "plan " << (*plans)[k]->plan_name()
		      << " satisfies (always "
		      << instance->atoms[p].name << ")" << std::endl;
	  }
	  count_always[p] += 1;
	}
	else if (HSPS::Instance::default_trace_level > 0) {
	  std::cerr << "plan " << (*plans)[k]->plan_name()
		    << " does NOT satisfy (always "
		    << instance->atoms[p].name << ")" << std::endl;
	}
      }
      if ((0 < count_always[p]) && (count_always[p] < trajs.length()))
	atoms_always.insert(p);
    }

    HSPS::index_vec count_sometime(0, instance->n_atoms());
    HSPS::index_set atoms_sometime;
    HSPS::index_set atoms_sometime_all;
    for (HSPS::index_type p = 0; p < instance->n_atoms(); p++)
      if (!atoms_always.contains(p)) {
	for (HSPS::index_type k = 0; k < trajs.length(); k++) {
	  if (trajs[k]->test_sometime(p)) {
	    if (HSPS::Instance::default_trace_level > 0) {
	      std::cerr << "plan " << (*plans)[k]->plan_name()
			<< " satisfies (sometime "
			<< instance->atoms[p].name << ")" << std::endl;
	    }
	    count_sometime[p] += 1;
	  }
	  else if (HSPS::Instance::default_trace_level > 0) {
	    std::cerr << "plan " << (*plans)[k]->plan_name()
		      << " does NOT satisfy (sometime "
		      << instance->atoms[p].name << ")" << std::endl;
	  }
	}
	if ((0 < count_sometime[p]) && (count_sometime[p] < trajs.length()))
	  atoms_sometime.insert(p);
	if ((count_sometime[p] == trajs.length()) &&
	    (!instance->atoms[p].init || opt_initial) &&
	    (!instance->atoms[p].goal || opt_goal))
	  atoms_sometime_all.insert(p);
      }

    HSPS::index_vec count_once(0, instance->n_atoms());
    HSPS::index_vec count_more(0, instance->n_atoms());
    HSPS::index_set atoms_once;
    for (HSPS::index_type p = 0; p < instance->n_atoms(); p++)
      if (!atoms_always.contains(p)) {
	for (HSPS::index_type k = 0; k < trajs.length(); k++) {
	  if (trajs[k]->test_at_most_once(p)) {
	    if (trajs[k]->test_sometime(p)) {
	      if (HSPS::Instance::default_trace_level > 0) {
		std::cerr << "plan " << (*plans)[k]->plan_name()
			  << " satisfies (at-most-once "
			  << instance->atoms[p].name
			  << ") and (sometime "
			  << instance->atoms[p].name
			  << ")" << std::endl;
	      }
	      count_once[p] += 1;
	    }
	    else if (HSPS::Instance::default_trace_level > 0) {
	      std::cerr << "plan " << (*plans)[k]->plan_name()
			<< " satisfies (at-most-once "
			<< instance->atoms[p].name
			<< ") but NOT (sometime "
			<< instance->atoms[p].name
			<< ")" << std::endl;
	    }
	  }
	  else {
	    if (HSPS::Instance::default_trace_level > 0) {
	      std::cerr << "plan " << (*plans)[k]->plan_name()
			<< " does NOT satisfy (at-most-once "
			<< instance->atoms[p].name << ")" << std::endl;
	    }
	    count_more[p] += 1;
	  }
	}
	if ((count_once[p] > 0) && (count_more[p] > 0))
	  atoms_once.insert(p);
      }

    atoms_sometime.subtract(atoms_once);

    HSPS::pair_vec pairs_before(HSPS::index_pair(HSPS::no_such_index, HSPS::no_such_index), 0);
    HSPS::index_vec count_before(0, 0);
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
    HSPS::pair_vec pairs_after(HSPS::index_pair(HSPS::no_such_index, HSPS::no_such_index), 0);
    HSPS::index_vec count_after(0, 0);
#endif

    if (opt_difference_order) {
      for (HSPS::index_type i = 0; i < atoms_sometime_all.length(); i++) {
	HSPS::index_type p = atoms_sometime_all[i];
	for (HSPS::index_type q = 0; q < instance->n_atoms(); q++)
	  if ((count_always[q] == 0) &&
	      (!instance->atoms[q].init || opt_initial) &&
	      (!instance->atoms[q].goal || opt_goal)) {
	    HSPS::index_type n_before = 0;
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
	    HSPS::index_type n_after = 0;
#endif
	    for (HSPS::index_type k = 0; k < trajs.length(); k++) {
	      if (trajs[k]->test_sometime_before(p, q)) n_before += 1;
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
	      if (trajs[k]->test_sometime_after(p, q)) n_after += 1;
#endif
	    }
	    if ((0 < n_before) && (n_before < trajs.length())) {
	      pairs_before.append(HSPS::index_pair(p, q));
	      count_before.append(n_before);
	    }
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
	    if ((0 < n_after) && (n_after < trajs.length())) {
	      pairs_after.append(HSPS::index_pair(p, q));
	      count_after.append(n_after);
	    }
#endif
	  }
      }
    }

    HSPS::index_type n_atom_constraints =
      (atoms_always.length() + atoms_sometime.length() + atoms_once.length());
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
    HSPS::index_type n_pair_constraints =
      (pairs_before.length() + pairs_after.length());
#else
    HSPS::index_type n_pair_constraints = pairs_before.length();
#endif
    HSPS::index_type n_constraints = n_atom_constraints + n_pair_constraints;

    HSPS::index_vec max_sat_always(0, atoms_always.length());
    HSPS::index_vec max_sat_sometime(0, atoms_sometime.length());
    HSPS::index_vec max_sat_once(0, atoms_once.length());
    HSPS::index_vec max_sat_before(0, pairs_before.length());
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
    HSPS::index_vec max_sat_after(0, pairs_after.length());
#endif

    for (HSPS::index_type k = 0; k < trajs.length(); k++) {
      HSPS::bool_vec sat_always(false, atoms_always.length());
      HSPS::bool_vec sat_sometime(false, atoms_sometime.length());
      HSPS::bool_vec sat_once(false, atoms_once.length());
      HSPS::bool_vec sat_before(false, pairs_before.length());
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
      HSPS::bool_vec sat_after(false, pairs_after.length());
#endif

      for (HSPS::index_type i = 0; i < atoms_always.length(); i++)
	sat_always[i] = trajs[k]->test_always(atoms_always[i]);
      for (HSPS::index_type i = 0; i < atoms_sometime.length(); i++)
	sat_sometime[i] = trajs[k]->test_sometime(atoms_sometime[i]);
      for (HSPS::index_type i = 0; i < atoms_once.length(); i++)
	sat_once[i] = (trajs[k]->test_sometime(atoms_once[i]) &&
		       trajs[k]->test_at_most_once(atoms_once[i]));
      for (HSPS::index_type i = 0; i < pairs_before.length(); i++)
	sat_before[i] = trajs[k]->test_sometime_before(pairs_before[i].first,
						       pairs_before[i].second);
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
      for (HSPS::index_type i = 0; i < pairs_after.length(); i++)
	sat_after[i] = trajs[k]->test_sometime_after(pairs_after[i].first,
						     pairs_after[i].second);
#endif

      HSPS::index_type n_always = sat_always.count(true);
      HSPS::index_type n_sometime = sat_sometime.count(true);
      HSPS::index_type n_once = sat_once.count(true);
      HSPS::index_type n_before = sat_before.count(true);
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
      HSPS::index_type n_after = sat_after.count(true);
      HSPS::index_type n_sat = n_always + n_sometime + n_once + n_before + n_after;
#else
      HSPS::index_type n_sat = n_always + n_sometime + n_once + n_before;
#endif

      std::cout << ";; plan " << (*plans)[k]->plan_name() << " satisfies "
		<< n_sat << " constraints ("
		<< n_always << " always, "
		<< n_sometime << " sometime, "
		<< n_once << " once, "
		<< n_before << " before, "
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
		<< n_after << " after"
#endif
		<< ")" << std::endl;

      for (HSPS::index_type i = 0; i < atoms_always.length(); i++)
	if (sat_always[i]) {
	  std::cout << ";;  (always "
		    << instance->atoms[atoms_always[i]].name
		    << ")" << std::endl;
	  if (n_sat > max_sat_always[i]) max_sat_always[i] = n_sat;
	}
      for (HSPS::index_type i = 0; i < atoms_sometime.length(); i++)
	if (sat_sometime[i]) {
	  std::cout << ";;  (sometime "
		    << instance->atoms[atoms_sometime[i]].name
		    << ")" << std::endl;
	  if (n_sat > max_sat_sometime[i]) max_sat_sometime[i] = n_sat;
	}
      for (HSPS::index_type i = 0; i < atoms_once.length(); i++)
	if (sat_once[i]) {
	  std::cout << ";;  (and (sometime "
		    << instance->atoms[atoms_once[i]].name
		    << ") (at-most-once "
		    << instance->atoms[atoms_once[i]].name
		    << "))" << std::endl;
	  if (n_sat > max_sat_once[i]) max_sat_once[i] = n_sat;
	}
      for (HSPS::index_type i = 0; i < pairs_before.length(); i++)
	if (sat_before[i]) {
	  std::cout << ";;  (sometime-before "
		    << instance->atoms[pairs_before[i].first].name << " "
		    << instance->atoms[pairs_before[i].second].name
		    << ")" << std::endl;
	  if (n_sat > max_sat_before[i]) max_sat_before[i] = n_sat;
	}
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
      for (HSPS::index_type i = 0; i < pairs_after.length(); i++)
	if (sat_after[i]) {
	  std::cout << ";;  (sometime-after "
		    << instance->atoms[pairs_before[i].first].name << " "
		    << instance->atoms[pairs_before[i].second].name
		    << ")" << std::endl;
	  if (n_sat > max_sat_after[i]) max_sat_after[i] = n_sat;
	}
#endif
    }

    if (opt_add) {
      std::cout << "(define (problem " << reader->problem_name << ")"
		<< std::endl;

      std::cout << "(:constraints (and" << std::endl;
      for (HSPS::index_type k = 0; k < atoms_always.length(); k++) {
	std::cout << "(preference A" << k << " (always "
		  << instance->atoms[atoms_always[k]].name
		  << "))" << std::endl;
      }
      for (HSPS::index_type k = 0; k < atoms_sometime.length(); k++) {
	std::cout << "(preference E" << k << " (sometime "
		  << instance->atoms[atoms_sometime[k]].name
		  << "))" << std::endl;
      }
      for (HSPS::index_type k = 0; k < atoms_once.length(); k++) {
	if (opt_complex_constraints) {
	  std::cout << "(preference O" << k << " (and (sometime "
		    << instance->atoms[atoms_once[k]].name
		    << ") (at-most-once "
		    << instance->atoms[atoms_once[k]].name
		    << ")))" << std::endl;
	}
	else {
	  std::cout << "(preference O" << k << " (at-most-once "
		    << instance->atoms[atoms_once[k]].name
		    << "))" << std::endl;
	}
      }
      HSPS::index_set selected_pair_constraints;
      if (n_pair_constraints > n_atom_constraints) {
	rng.select_variable_set(selected_pair_constraints,
				n_atom_constraints,
				n_pair_constraints);
      }
      else {
	selected_pair_constraints.fill(n_pair_constraints);
      }
      for (HSPS::index_type k = 0; k < selected_pair_constraints.length(); k++) {
	HSPS::index_type s = selected_pair_constraints[k];
	if (s < pairs_before.length()) { // select sometime-before
	  HSPS::index_pair p = pairs_before[s];
	  std::cout << "(preference SB" << s << " (sometime-before "
		    << instance->atoms[p.first].name << " "
		    << instance->atoms[p.second].name
		    << "))" << std::endl;
	}
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
	else { // select sometime-after
	  s -= pairs_before.length();
	  assert(s < pairs_after.length());
	  HSPS::index_pair p = pairs_after[s];
	  std::cout << "(preference SA" << s << " (sometime-after "
		    << instance->atoms[p.first].name << " "
		    << instance->atoms[p.second].name
		    << "))" << std::endl;
	}
#endif
      }
      std::cout << "))" << std::endl;

      HSPS::index_type n_preferences = (atoms_always.length() +
				  atoms_sometime.length() +
				  atoms_once.length() +
				  selected_pair_constraints.length());

      std::cout << "(:metric maximize";
      HSPS::index_type n = n_preferences;
      for (HSPS::index_type k = 0; k < atoms_always.length(); k++) {
	HSPS::rational w =
	  (HSPS::rational(n_constraints - max_sat_always[k], n_constraints) *
	   n_preferences);
	if (opt_randomize_score) {
	  w = (w * HSPS::rational(rng.random_in_range(101) + 50, 100));
	}
	if (n > 1) {
	  std::cout << " (+";
	  n -= 1;
	}
	std::cout << " (* " << w.decimal()
		  << " (- 1 (is-violated A" << k << ")))";
      }
      for (HSPS::index_type k = 0; k < atoms_sometime.length(); k++) {
	HSPS::rational w =
	  (HSPS::rational(n_constraints - max_sat_sometime[k], n_constraints) *
	   n_preferences);
	if (opt_randomize_score) {
	  w = (w * HSPS::rational(rng.random_in_range(101) + 50, 100));
	}
	if (n > 1) {
	  std::cout << " (+";
	  n -= 1;
	}
	std::cout << " (* " << w.decimal()
		  << " (- 1 (is-violated E" << k << ")))";
      }
      for (HSPS::index_type k = 0; k < atoms_once.length(); k++) {
	HSPS::rational w =
	  (HSPS::rational(n_constraints - max_sat_once[k], n_constraints) *
	   n_preferences);
	if (opt_randomize_score) {
	  w = (w * HSPS::rational(rng.random_in_range(101) + 50, 100));
	}
	if (n > 1) {
	  std::cout << " (+";
	  n -= 1;
	}
	std::cout << " (* " << w.decimal()
		  << " (- 1 (is-violated O" << k << ")))";
      }
      for (HSPS::index_type k = 0; k < selected_pair_constraints.length(); k++) {
	HSPS::index_type s = selected_pair_constraints[k];
	if (s < pairs_before.length()) {
	  HSPS::index_pair p = pairs_before[s];
	  HSPS::rational w =
	    (HSPS::rational(n_constraints - max_sat_before[s], n_constraints) *
	     n_preferences);
	  if (opt_randomize_score) {
	    w = (w * HSPS::rational(rng.random_in_range(101) + 50, 100));
	  }
	  if (n > 1) {
	    std::cout << " (+";
	    n -= 1;
	  }
	  std::cout << " (* " << w.decimal()
		    << " (- 1 (is-violated SB" << s << ")))";
	}
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
	else {
	  s -= pairs_before.length();
	  assert(s < pairs_after.length());
	  HSPS::index_pair p = pairs_after[s];
	  HSPS::rational w =
	    (HSPS::rational(n_constraints - max_sat_after[s], n_constraints) *
	     n_preferences);
	  if (opt_randomize_score) {
	    w = (w * HSPS::rational(rng.random_in_range(101) + 50, 100));
	  }
	  if (n > 1) {
	    std::cout << " (+";
	    n -= 1;
	  }
	  std::cout << " (* " << w.decimal()
		    << " (- 1 (is-violated SA" << s << ")))";
	}
#endif
      }
      while (n < n_preferences) {
	std::cout << ")";
	n += 1;
      }
      std::cout << ")" << std::endl;

      // end problem def
      std::cout << ")" << std::endl;
    }

    else { // normal output
      for (HSPS::index_type k = 0; k < atoms_always.length(); k++) {
	std::cout << "(always " << instance->atoms[atoms_always[k]].name
		  << ") ;; holds in " << count_always[atoms_always[k]]
		  << " of " << trajs.length() << " traces (max sat = "
		  << max_sat_always[k] << " of " << n_constraints << ")"
		  << std::endl;
      }
      for (HSPS::index_type k = 0; k < atoms_sometime.length(); k++) {
	std::cout << "(sometime " << instance->atoms[atoms_sometime[k]].name
		  << ") ;; holds in " << count_sometime[atoms_sometime[k]]
		  << " of " << trajs.length() << " traces (max sat = "
		  << max_sat_sometime[k] << " of " << n_constraints << ")"
		  << std::endl;
      }
      for (HSPS::index_type k = 0; k < atoms_once.length(); k++) {
	if (opt_complex_constraints) {
	  std::cout << "(and (sometime "
		    << instance->atoms[atoms_once[k]].name
		    << ") (at-most-once "
		    << instance->atoms[atoms_once[k]].name
		    << ")) ;; holds in " << count_once[atoms_once[k]]
		    << " of " << trajs.length() << " traces (max sat = "
		    << max_sat_once[k] << " of " << n_constraints << ")"
		    << std::endl;
	}
	else {
	  std::cout << "(at-most-once " << instance->atoms[atoms_once[k]].name
		    << ")) ;; holds in " << count_once[atoms_once[k]]
		    << " of " << trajs.length() << " traces (max sat = "
		    << max_sat_once[k] << " of " << n_constraints << ")"
		    << std::endl;
	}
      }
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
      for (HSPS::index_type k = 0; k < pairs_after.length(); k++) {
	HSPS::index_pair p = pairs_after[k];
	std::cout << "(sometime-after "
		  << instance->atoms[p.first].name << " "
		  << instance->atoms[p.second].name
		  << ") ;; holds in " << count_after[k]
		  << " of " << trajs.length() << " traces (max sat = "
		  << max_sat_after[k] << " of " << n_constraints << ")"
		  << std::endl;
      }
#endif
      for (HSPS::index_type k = 0; k < pairs_before.length(); k++) {
	HSPS::index_pair p = pairs_before[k];
	std::cout << "(sometime-before "
		  << instance->atoms[p.first].name << " "
		  << instance->atoms[p.second].name
		  << ") ;; holds in " << count_before[k]
		  << " of " << trajs.length() << " traces (max sat = "
		  << max_sat_before[k] << " of " << n_constraints << ")"
		  << std::endl;
      }
#ifdef DIFF_INCLUDE_SOMETIME_AFTER
      std::cout << ";; "
		<< atoms_always.length() << " always, "
		<< atoms_sometime.length() << " sometime, "
		<< atoms_once.length() << " once, "
		<< pairs_before.length() << " before and "
		<< pairs_after.length() << " after constraints found"
		<< std::endl;
#else
      std::cout << ";; "
		<< atoms_always.length() << " always, "
		<< atoms_sometime.length() << " sometime, "
		<< atoms_once.length() << " once and "
		<< pairs_before.length() << " before constraints found"
		<< std::endl;
#endif
    }
  }

  if (opt_build_sas_instance) {
    std::cerr << "constructing SAS instance..." << std::endl;
    stats.start();
    HSPS::SASInstance* sas_p = new HSPS::SASInstance();
    sas_p->name = instance->name;
    std::cerr << "constructing variables (" << stats.time() << ")..."
	      << std::endl;
    if (opt_sas_selective) {
      HSPS::Instance::constraint_vec invs;
      sas_p->select_invariants(*instance, invs);
      sas_p->construct_variables(*instance, invs);
    }
    else {
      sas_p->construct_variables(*instance, instance->invariants);
    }
    bool ok = true;
    std::cerr << "constructing actions (" << stats.time() << ")..."
	      << std::endl;
    if (opt_sas_min) {
      ok = sas_p->construct_minimal_actions(*instance);
    }
    else if (opt_sas_safe) {
      ok = sas_p->construct_safe_actions(*instance);
    }
    else {
      ok = sas_p->construct_actions(*instance);
    }
    std::cerr << "removing inconsistent actions (" << stats.time() << ")..."
	      << std::endl;
    sas_p->remove_inconsistent_actions();
    std::cerr << "constructing init & goal states (" << stats.time() << ")..."
	      << std::endl;
    ok = (ok && sas_p->construct_init_and_goal_state(*instance));
    if (!ok && !HSPS::PDDL_Base::best_effort) {
      std::cerr << "error: SAS construction failed"
		<< std::endl;
      sas_p->write_domain(std::cerr);
      exit(255);
    }
    std::cerr << "cross-referencing (" << stats.time() << ")..." << std::endl;
    sas_p->cross_reference();
    std::cerr << "computing graphs (" << stats.time() << ")..." << std::endl;
    sas_p->compute_graphs();
    if (opt_sas_extend) {
      std::cerr << "extending STRIPS to SAS map (" << stats.time() << ")..."
		<< std::endl;
      HSPS::Simplifier s(sas_p, stats);
      s.extend_atom_map(*prep);
      sas_p = s.result();
    }
    stats.stop();
    std::cerr << "SAS domain/problem constructed in "
	      << stats.time() << " seconds" << std::endl;

    if (opt_path_relevant) {
      std::cerr << "computing path set..." << std::endl;
      stats.start();
      HSPS::PathSet p(*sas_p);
      p.compute();
      HSPS::bool_vec sas_act;
      p.actions(sas_act);
      path_relevant_actions.assign_value(false, instance->n_actions());
      sas_p->map_action_set(sas_act, path_relevant_actions);
      stats.stop();
      std::cerr << "finished in " << stats.time() << " seconds" << std::endl;
    }

    if (opt_sas_reduce_to_goal) {
      sas_p = sas_p->reduce_to_goal_variables();
    }

    if (opt_simplify) {
      if (opt_print_intermediary_instance) {
	std::cerr << "SAS instance before simplification:" << std::endl;
	sas_p->write_domain(std::cerr);
      }
      std::cerr << "simplifying..." << std::endl;
      HSPS::Simplifier s(sas_p, stats);
//       if (opt_eliminate_determined) {
// 	std::cerr << "trying to eliminate strictly determined variables..."
// 		  << std::endl;
// 	bool chg = s.eliminate_strictly_determined_variables();
// 	if (!chg) {
// 	  std::cerr << "- no change" << std::endl;
// 	}
// 	else if (opt_print_intermediary_instance) {
// 	  std::cerr << "current instance:" << std::endl;
// 	  s.result()->write_domain(std::cerr);
// 	}
//       }
      bool done = false;
      while (!done) {
	while (!done) {
	  std::cerr << "applying weak safe abstraction..."
		    << std::endl;
	  bool chg =
	    (opt_old ? s.apply_old_WSA() : s.apply_WSA(simplify_wsa_analysis_limit, opt_simplify_wsa_single_variable_only));
	  if (!chg) {
	    std::cerr << "- no change" << std::endl;
	    done = true;
	  }
	  else if (s.result()->goal_state.empty()) {
	    std::cerr << "- goal condition simplified to true" << std::endl;
	    done = true;
	  }
	  else if (opt_print_intermediary_instance) {
	    // if (opt_sas_info) {
	    //   s.result()->compute_domain_transitions(HSPS::UnitACF());
	    // }
	    std::cerr << "current instance:" << std::endl;
	    s.result()->write_domain(std::cerr);
	  }
	}
	if (opt_old) {
	  if (!s.result()->goal_state.empty()) {
	    std::cerr << "trying to compose interfering variables..."
		      << std::endl;
	    bool chg = s.apply_variable_composition();
	    if (!chg) {
	      std::cerr << "- no change" << std::endl;
	    }
	    else if (opt_print_intermediary_instance) {
	      std::cerr << "current instance:" << std::endl;
	      s.result()->write_domain(std::cerr);
	    }
	    done = !chg;
	  }
	}
	if (done && !s.result()->goal_state.empty()) {
	  std::cerr << "trying action sequence composition..." << std::endl;
	  bool chg = s.apply_sequence_composition();
	  if (!chg) {
	    std::cerr << "- no change" << std::endl;
	  }
	  else if (opt_print_intermediary_instance) {
	    std::cerr << "current instance:" << std::endl;
	    s.result()->write_domain(std::cerr);
	  }
	  done = !chg;
	}
	if (opt_minimize_compose && !opt_old) {
	  if (done && !s.result()->goal_state.empty()) {
	    std::cerr << "trying minimization..." << std::endl;
	    bool chg = s.apply_minimization(simplify_wsa_analysis_limit);
	    if (!chg) {
	      std::cerr << "- no change" << std::endl;
	    }
	    else if (opt_print_intermediary_instance) {
	      std::cerr << "current instance:" << std::endl;
	      s.result()->write_domain(std::cerr);
	    }
	    std::cerr << "trying variable composition..." << std::endl;
	    chg = s.apply_variable_composition();
	    if (!chg) {
	      std::cerr << "- no change" << std::endl;
	    }
	    else if (opt_print_intermediary_instance) {
	      std::cerr << "current instance:" << std::endl;
	      s.result()->write_domain(std::cerr);
	    }
	    done = !chg;
	  }
	}
      }
      sas_p = s.result();
    }

    if (opt_misc) {
      for (HSPS::index_type k = 0; k < sas_p->goal_state.length(); k++) {
	HSPS::index_type g_var = sas_p->goal_state[k].first;
	HSPS::index_type g_val = sas_p->goal_state[k].second;
	HSPS::index_vec d;
	sas_p->dependency_graph.distance(g_var, d);
	std::cerr << "goal " << sas_p->variables[g_var].name
		  << " = " << sas_p->variables[g_var].domain[g_val]
		  << ":" << std::endl;
	for (HSPS::index_type i = 0; i < sas_p->n_variables(); i++) {
	  if (d[i] == HSPS::no_such_index) {
	    std::cerr << " " << sas_p->variables[i].name << ":\t-"
		      << std::endl;
	  }
	  else {
	    std::cerr << " " << sas_p->variables[i].name << ":\t" << d[i]
		      << std::endl;
	  }
	}
      }

      std::cerr << "inconsistency relation: " << std::endl;
      prep->inconsistency()->write(std::cerr);

      HSPS::MDDNode* sas_inc =
	HSPS::makeMDD(prep->inconsistency(),
		      sas_p->atom_map_defined(),
		      sas_p->atom_map_n());

      std::cerr << "MDD graph:" << std::endl;
      sas_inc->write_graph(std::cerr);
    }

    if (opt_spanning) {
      HSPS::SubsetEnumerator e(sas_p->n_variables());
      bool more = e.first();
      HSPS::index_set_vec sets;
      while (more) {
	HSPS::index_set s;
	e.current_set(s);
	if (sas_p->is_spanning(s)) {
	  sets.insert_minimal(s);
	}
	more = e.next();
      }
      std::cerr << sets.length() << " spanning sets found" << std::endl;

      for (HSPS::index_type k = 0; k < sets.length(); k++) {
	HSPS::SASInstance* sas_r = sas_p->reduce_to_spanning(sets[k]);
	if (sas_r) {
	  std::ostringstream fname;
	  if (reader->problem_file) {
	    char* p0 = strdup(reader->problem_file);
	    char* p1 = strrchr(p0, '.');
	    if (p1) *p1 = '\0';
	    p1 = strrchr(p0, '/');
	    if (p1) p0 = p1 + 1;
	    fname << p0;
	  }
	  else {
	    fname << "spanning";
	  }
	  for (HSPS::index_type i = 0; i < sets[k].length(); i++) {
	    fname << "-";
	    sas_p->variables[sets[k][i]].name->write(fname, HSPS::Name::NC_INSTANCE);
	  }
	  if (opt_sas_to_strips) {
	    fname << ".pddl";
	  }
	  else {
	    fname << ".sas";
	  }

	  std::cerr << "saving reduced problem to " << fname.str()
		    << "..." << std::endl;
	  std::ofstream r_out(fname.str().c_str());
	  if (opt_sas_to_strips) {
	    HSPS::Instance* ins;
	    if (sas_r->atom_map_defined()) {
	      ins = sas_r->reconstruct_STRIPS();
	      if (instance == 0) {
		ins = sas_r->convert_to_STRIPS();
	      }
	    }
	    else {
	      ins = sas_r->convert_to_STRIPS();
	    }
	    ins->write_domain(r_out);
	    ins->write_problem(r_out);
	  }
	  else {
	    sas_r->write_domain(r_out);
	  }
	  r_out.close();
	}
	else {
	  std::cerr << "error: set ";
	  sas_p->write_variable_set(std::cerr, sets[k]);
	  std::cerr << " identified as spanning but reduction failed"
		    << std::endl;
	  exit(255);
	}
      }
    }

    if (opt_sas_id) {
      std::cout << sas_p->n_variables() << " variables" << std::endl;
      HSPS::index_set gv;
      sas_p->goal_state.defined_set(gv);
      std::cout << gv.length() << " goal variables: ";
      sas_p->write_variable_set(std::cout, gv);
      std::cout << std::endl;
      stats.start();
      HSPS::IndependentVariableSets iv(*sas_p);
      if (opt_apx_clique) {
	iv.compute_approximate_independent_sets();
      }
      else {
	iv.compute_maximal_independent_sets();
      }
      stats.stop();
      std::cout << iv.length() << " independent sets:" << std::endl;
      for (HSPS::index_type k = 0; k < iv.length(); k++) {
	std::cout << " - ";
	sas_p->write_variable_set(std::cout, iv[k]);
	std::cout << std::endl;
      }
      std::cout << " (" << stats.time() << " seconds)" << std::endl;
      HSPS::IndependentVariableSets sps(iv);
      stats.start();
      sps.compute_spanning_sets(false, opt_sas_spanning_search_limit);
      stats.stop();
      std::cout << sps.length() << " spanning sets ("
		<< stats.time() << " seconds)" << std::endl;
      for (HSPS::index_type k = 0; k < sps.length(); k++) {
	std::cout << " - ";
	sas_p->write_variable_set(std::cout, sps[k]);
	std::cout << std::endl;
      }
      HSPS::index_set uss;
      sps.union_set(uss);
      std::cout << uss.length() << " of " << sas_p->n_variables()
		<< " variables span the state space" << std::endl;
      HSPS::IndependentVariableSets sps_pg(iv);
      stats.start();
      sps_pg.compute_spanning_sets(true, opt_sas_spanning_search_limit);
      stats.stop();
      std::cout << "preserving goal sets: " << sps_pg.length()
		<< " spanning sets (" << stats.time() << " seconds)"
		<< std::endl;
      for (HSPS::index_type k = 0; k < sps_pg.length(); k++) {
	std::cout << " - ";
	sas_p->write_variable_set(std::cout, sps_pg[k]);
	std::cout << std::endl;
      }
      sps_pg.union_set(uss);
      std::cout << uss.length() << " of " << sas_p->n_variables()
		<< " variables span the state space" << std::endl;
      for (HSPS::index_type k = 0; k < sas_p->n_variables(); k++) {
	HSPS::index_set_vec dets;
	dets.clear();
	HSPS::SASInstance::determined_check_result res =
	  sas_p->minimal_determining_sets(k, dets);
	sas_p->variables[k].name->write(std::cout, HSPS::Name::NC_INSTANCE);
	std::cout << ": " << res;
	if (dets.length() > 0) {
	  std::cout << " by ";
	  for (HSPS::index_type i = 0; i < dets.length(); i++) {
	    if (i > 0) std::cout << ", ";
	    sas_p->write_variable_set(std::cout, dets[i]);
	  }
	}
	std::cout << std::endl;
      }
    }

    if (opt_sas_to_strips) {
      delete instance;
      delete prep;

      stats.start();
      if (sas_p->atom_map_defined()) {
	std::cerr << "trying to reconstruct STRIPS instance from SAS..."
		  << std::endl;
	instance = sas_p->reconstruct_STRIPS();
	if (instance == 0) {
	  std::cerr << "reconstruction failed; converting SAS to STRIPS..."
		    << std::endl;
	  instance = sas_p->convert_to_STRIPS();
	}
      }
      else {
	std::cerr << "converting SAS to STRIPS..." << std::endl;
	instance = sas_p->convert_to_STRIPS();
      }
      std::cerr << "construction finished in " << stats.time()
		<< " seconds; " << instance->n_atoms() << " atoms, "
		<< instance->n_actions() << " actions" << std::endl;

      prep = new HSPS::Reduce(*instance, stats);
      if (opt_preprocess) {
	std::cerr << "preprocessing..." << std::endl;
	prep->preprocess();
      }
      else {
	std::cerr << "cross-referencing..." << std::endl;
	instance->cross_reference();
      }
      stats.stop();
      std::cerr << "finished in " << stats.time() << " seconds" << std::endl;

      if (opt_print) {
	if (opt_domain) instance->write_domain(std::cout);
	if (opt_problem) instance->write_problem(std::cout);
      }
      else if (opt_dump) {
	instance->print(std::cout);
      }
      else {
	NTYPE d_atm = R_TO_N(instance->n_atoms() - initial_instance_atom_count,
			     initial_instance_atom_count);
	NTYPE d_act = R_TO_N(instance->n_actions() -
			     initial_instance_action_count,
			     initial_instance_action_count);
	std::cout << instance->n_atoms() << " atoms (";
	if (d_atm > 0) std::cout << "+";
	std::cout << PRINT_NTYPE(d_atm * 100) << "%), "
		  << instance->n_actions() << " actions (";
	if (d_act > 0) std::cout << "+";
	std::cout << PRINT_NTYPE(d_act * 100) << "%)"
		  << std::endl;
      }
    }

    if (opt_pdb) {
      stats.start();
      HSPS::name_vec var_names(0, 0);
      sas_p->variable_names(var_names);
      HSPS::index_set_vec sets;
      if (pdb_all_of_size == HSPS::no_such_index) {
	reader->export_sets(var_names, sets);
	std::cerr << sets.length() << " variable sets found in input"
		  << std::endl;
      }
      else {
	HSPS::mSubsetEnumerator e(sas_p->n_variables(), pdb_all_of_size);
	e.all_sets(sets);
	std::cerr << sets.length() << " sets of " << pdb_all_of_size
		  << " variables" << std::endl;
      }
      HSPS::SASCostACF cost(*sas_p);
      HSPS::Heuristic* inc = (opt_inconsistency ? prep->inconsistency() : 0);
      HSPS::MDDNode* sinc = (opt_inconsistency ?
			     makeMDD(prep->inconsistency(),
				     sas_p->atom_map_defined(),
				     sas_p->atom_map_n()) : 0);

      if (opt_write_graphs) {
	std::ofstream g_out("mdd.dot");
	sinc->write_graph(g_out);
	g_out.close();
      }

      for (HSPS::index_type k = 0; k < sets.length(); k++) {
	if (opt_regression) {
	  HSPS::RegressionPDB* rpdb =
	    new HSPS::RegressionPDB(*sas_p, sets[k], cost, sinc, inc, stats);
	  rpdb->compute();
	  std::cerr << "regression PDB ";
	  sas_p->write_variable_set(std::cerr, sets[k]);
	  std::cerr << " computed in " << stats.time() << " seconds"
		    << std::endl;
	  std::cerr << "estimated cost of goals: "
		    << rpdb->eval(sas_p->goal_state)
		    << std::endl;
	  if (opt_dump) {
	    std::cout << "regression PDB ";
	    sas_p->write_variable_set(std::cout, sets[k]);
	    std::cout << ":" << std::endl;
	    rpdb->write(std::cout);
	  }
	  if (opt_write_graphs) {
	    std::cerr << "writing abstract state space graph..." << std::endl;
	    std::ostringstream fname;
	    fname << "rpdb" << k << ".dot";
	    std::ofstream g_out(fname.str().c_str());
	    rpdb->write_graph(g_out, sas_p->goal_state,
			      HSPS::EMPTYSET, true, false);
	    g_out.close();
	  }
	  delete rpdb;
	}

	if (opt_progression) {
	  HSPS::ProgressionPDB* ppdb =
	    new HSPS::ProgressionPDB(*sas_p, sets[k], cost, sinc, inc, stats);
	  double t0 = stats.time();
	  ppdb->compute();
	  std::cerr << "progression PDB ";
	  sas_p->write_variable_set(std::cerr, sets[k]);
	  std::cerr << " computed in " << stats.time() - t0 << " seconds"
		    << std::endl;
	  std::cerr << "estimated cost from initial state: "
		    << ppdb->eval(sas_p->init_state)
		    << std::endl;
	  if (opt_dump) {
	    std::cout << "progression PDB ";
	    sas_p->write_variable_set(std::cout, sets[k]);
	    std::cout << ":" << std::endl;
	    ppdb->write(std::cout);
	  }

	  HSPS::ProgressionPDB* ppdb2 =
	    new HSPS::ProgressionPDB(*sas_p, sets[k], cost, sinc, inc, stats);
	  t0 = stats.time();
	  ppdb2->compute2();
	  std::cerr << "new & improved PDB computed in "
		    << stats.time() - t0 << " seconds"
		    << std::endl;
	  if (opt_dump) {
	    std::cout << "new & improved progression PDB ";
	    sas_p->write_variable_set(std::cout, sets[k]);
	    std::cout << ":" << std::endl;
	    ppdb2->write(std::cout);
	  }
	  std::cerr << "comparing...";
	  if (HSPS::Instance::default_trace_level > 0) {
	    std::cerr << std::endl;
	    ppdb2->set_trace_level(3);
	  }
	  HSPS::index_pair res = ppdb2->compare(*ppdb);
	  if (HSPS::Instance::default_trace_level > 0) {
	    std::cerr << "result =";
	  }
	  std::cerr << " " << res.first << "/"
		    << ppdb->size() - (res.first + res.second) << "/"
		    << res.second << std::endl;

	  if (opt_write_graphs) {
	    std::cerr << "writing abstract state space graph..." << std::endl;
	    std::ostringstream fname;
	    fname << "ppdb" << k << ".dot";
	    std::ofstream g_out(fname.str().c_str());
	    ppdb->write_graph(g_out, sas_p->init_state,
			      HSPS::EMPTYSET, false, true);
	    g_out.close();
	  }
	  delete ppdb;
	}
      }
      stats.stop();
    }

    if (opt_sas && !opt_sas_to_strips && opt_print) {
      if (opt_print_brief) {
	std::cout << "variables:" << std::endl;
	for (HSPS::index_type k = 0; k < sas_p->n_variables(); k++) {
	  std::cout << " ";
	  sas_p->write_variable(std::cout, sas_p->variables[k]);
	  std::cout << std::endl;
	}
	std::cout << "goal: ";
	sas_p->write_partial_state(std::cout, sas_p->goal_state);
	std::cout << ";" << std::endl;
      }
      else {
	sas_p->write_domain(std::cout);
      }

      if (opt_sas_info) {
	// HSPS::partial_state x_goal;
	// sas_p->extend_nsc(sas_p->init_state, sas_p->goal_state, x_goal);
	// x_goal.subtract(sas_p->init_state);
	// std::cout << "{extended goal: ";
	// sas_p->write_partial_state(std::cout, x_goal);
	// std::cout << "}" << std::endl;

	if (sas_p->atom_map_n() == instance->n_atoms()) {
	  std::cout << "{(possibly) lost inconsistent atom pairs:";
	  for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
	    for (HSPS::index_type j = 0; j < instance->n_atoms(); j++) {
	      HSPS::index_set pair;
	      pair.insert(i);
	      pair.insert(j);
	      NTYPE val = prep->inconsistency()->eval(pair);
	      if (INFINITE(val)) {
		HSPS::partial_state s_pair;
		sas_p->map_to_partial_state(i, s_pair);
		sas_p->map_to_partial_state(j, s_pair);
		bool lost = false;
		for (HSPS::index_type k = 0; (k < sas_p->n_variables())&&!lost; k++)
		  if (s_pair.defines(k) && s_pair.consistent(k)) lost = true;
		if (lost) {
		  std::cout << std::endl << "  "
			    << instance->atoms[i].name << ", "
			    << instance->atoms[j].name << " = ";
		  sas_p->write_partial_state(std::cout, s_pair);
		}
	      }
	    }
	  std::cout << " }" << std::endl;
	}

	std::cout << "{# common affecting actions" << std::endl;
	for (HSPS::index_type k = 0; k < sas_p->n_variables(); k++)
	  sas_p->variables[k].name->write(std::cout << '\t', HSPS::Name::NC_INSTANCE);
	std::cout << std::endl << "rel.:";
	for (HSPS::index_type k = 0; k < sas_p->n_variables(); k++)
	  std::cout << '\t' << sas_p->variables[k].set_by.length();
	for (HSPS::index_type k = 0; k < sas_p->n_variables(); k++) {
	  sas_p->variables[k].name->write(std::cout << std::endl, HSPS::Name::NC_INSTANCE);
	  for (HSPS::index_type i = 0; i < sas_p->n_variables(); i++) {
	    if (i == k)
	      std::cout << "\t - ";
	    else
	      std::cout << '\t' << sas_p->variables[k].set_by.count_common(sas_p->variables[i].set_by);
	  }
	}
	std::cout << " }" << std::endl;
      }

      if (HSPS::Instance::write_extra) {
	HSPS::name_vec vnames(0, 0);
	sas_p->variable_names(vnames);
	HSPS::index_set_vec sets;
	reader->export_sets(vnames, sets);
	if (sets.length() > 0) {
	  std::cout << "{ input sets: }" << std::endl;
	  for (HSPS::index_type k = 0; k < sets.length(); k++) {
	    sas_p->write_variable_set(std::cout, sets[k]);
	    std::cout << std::endl;
	  }
	}
      }
    }

    else if (opt_sas && !opt_sas_to_strips) {
      HSPS::index_type n_goal_variables = 0;
      HSPS::index_type n_determined_variables = 0;
      for (HSPS::index_type k = 0; k < sas_p->n_variables(); k++) {
	if (sas_p->goal_state.defines(k)) n_goal_variables += 1;
	// if (sas_p->atom_map_defined()) {
	//   HSPS::index_set det;
	//   bool ok = sas_p->determining_set(k, det);
	//   if (ok) n_determined_variables += 1;
	// }
      }
      HSPS::index_type sum_preconditions = 0;
      HSPS::index_type sum_postconditions = 0;
      HSPS::index_type sum_prevailconditions = 0;
      for (HSPS::index_type k = 0; k < sas_p->n_actions(); k++) {
	sum_preconditions += sas_p->actions[k].pre.length();
	sum_postconditions += sas_p->actions[k].post.length();
	sum_prevailconditions += sas_p->actions[k].prv.length();
      }
      HSPS::bool_vec sa;
      sas_p->is_safe(sa);
      std::cout << "SAS instance " << instance->name << ": "
		<< sas_p->n_variables()	<< " variables ("
		<< n_goal_variables << " with goal value";
      // if (sas_p->atom_map_defined()) {
      // 	std::cout << ", " << n_determined_variables << " determined";
      // }
      std::cout << "), "
		<< sas_p->n_actions() << " actions (with "
		<< (sum_preconditions/(double)sas_p->n_actions())
		<< " pre-, "
		<< (sum_prevailconditions/(double)sas_p->n_actions())
		<< " prevail- and "
		<< (sum_postconditions/(double)sas_p->n_actions())
		<< " postconditions average; "
		<< sa.count(false) << " unsafe)"
		<< std::endl;
      for (HSPS::index_type k = 0; k < plans->length(); k++) {
	HSPS::SASPlanSummary summary(*sas_p);
	(*plans)[k]->output(summary);
	std::cout << instance->name << " plan #" << k;
	if ((*plans)[k]->plan_name()) {
	  std::cout << ", " << (*plans)[k]->plan_name();
	}
	std::cout << ": "
		  << (*plans)[k]->length() << " actions, "
		  << summary.average_variable_value_changes()
		  << " average value changes/variable ("
		  << summary.min_variable_value_changes()
		  << " min, "
		  << summary.max_variable_value_changes()
		  << " max), "
		  << summary.average_variable_required_values()
		  << " average required values/variable, "
		  << summary.n_secondary_goal_variables()
		  << " secondary goal variables"
		  << std::endl;
      }
      bool isP = sas_p->check_P();
      bool isU = sas_p->check_U();
      bool isB = sas_p->check_B();
      bool isS = sas_p->check_S();
      std::cout << "P: " << isP << ", U: " << isU
		<< ", B: " << isB << ", S: " << isS << std::endl;
    }

    if (opt_sas && opt_write_graphs) {
      if (!opt_analyse) {
	std::cerr << "writing domain transition graphs..." << std::endl;
	for (HSPS::index_type k = 0; k < sas_p->n_variables(); k++) {
	  std::ostringstream fname;
	  fname << "dtg" << k << ".dot";
	  std::ofstream g_out(fname.str().c_str());
	  bool wl = (opt_edge_labels == HSPS::SASInstance::ls_side_conditions_and_effects ? true : false);
	  sas_p->write_domain_transition_graph(g_out, k, opt_edge_labels, wl, false, false);
	  g_out.close();
	}

	std::cerr << "writing composite transition graph..." << std::endl;
	std::ofstream ctg_out("ctg.dot");
	sas_p->write_composite_transition_graph(ctg_out);
	ctg_out.close();

	std::cerr << "writing causal/independence graphs..." << std::endl;
	std::ofstream cg_out("cg.dot");
	sas_p->write_variable_digraph(cg_out, sas_p->causal_graph,
				      "Causal Graph", true);
	cg_out.close();
	std::ofstream dcg_out("dcg.dot");
	sas_p->write_dc_graph(dcg_out);
	dcg_out.close();
	std::ofstream tcg_out("transitive_cg.dot");
	sas_p->write_variable_digraph(tcg_out, sas_p->transitive_causal_graph,
				      "Transitive Causal Graph", true);
	tcg_out.close();
	std::ofstream idg_out("independence.dot");
	sas_p->write_variable_graph(idg_out, sas_p->independence_graph,
				    "Variable Independence");
	idg_out.close();
	std::ofstream ifg_out("interference.dot");
	sas_p->write_variable_graph(ifg_out, sas_p->interference_graph,
				    "Variable Interference");
	ifg_out.close();
	std::ofstream agg_out("agg.dot");
	sas_p->write_action_group_graph(agg_out);
	agg_out.close();

	if (sas_p->atom_map_n() == instance->n_atoms()) {
	  std::cerr << "writing variable elimination graph..." << std::endl;
	  HSPS::Simplifier s(sas_p, stats);
	  std::ofstream eg_out("eg.dot");
	  s.write_elimination_graph(eg_out);
	  eg_out.close();
	}

	std::ofstream asg_out("sequence.dot");
	sas_p->write_action_sequence_graph(asg_out);
	asg_out.close();
      }
      if (reader->n_plans() > 0) {
	std::cerr << "writing plan transition graphs..." << std::endl;
      }
      for (HSPS::index_type k = 0; k < plans->length(); k++) {
	HSPS::ActionSequence plan;
	(*plans)[k]->output(plan);
	std::ostringstream fname;
	fname << "input-plan-" << k << "-tg.dot";
	std::ofstream g_out(fname.str().c_str());
	sas_p->write_plan_transition_graph(g_out, plan, true);
	g_out.close();
      }
    }

    delete sas_p;
  }

  if ((opt_relevant ||
       opt_path_relevant ||
       opt_extend_goal ||
       opt_find_invariants ||
       opt_verify_invariants) &&
      opt_build_instance &&
      !opt_instantiate &&
      !opt_sas &&
      !opt_petrify &&
      !opt_lift) {
    if (opt_print) {
      reader->write_problem_begin(std::cout);
      if (!opt_add) {
	reader->write_objects(std::cout, true);
	reader->write_init(std::cout);
      }
    }

    if (opt_extend_goal) {
      if (opt_print) {
	std::cout << " (:goal (and";
	for (HSPS::index_type k = 0; k < instance->goal_atoms.length(); k++) {
	  std::cout << " " << instance->atoms[instance->goal_atoms[k]].name;
	}
	std::cout << "))" << std::endl;
      }
      else {
	std::cout << instance->name << " (extend): "
		  << (instance->goal_atoms.length() -
		      reader->dom_goals.length())
		  << " of " << instance->goal_atoms.length()
		  << " implied goals"
		  << std::endl;
      }
    }
    else if (opt_print && !opt_add) {
      reader->write_goal(std::cout);
    }

    if (opt_print && !opt_add) {
      if (HSPS::Instance::write_metric)
	reader->write_metric(std::cout);
      if (HSPS::Instance::write_DKEL)
	reader->write_dkel_items(std::cout, true);
    }

    if (opt_find_invariants || opt_verify_invariants) {
      if (opt_print) {
	if (HSPS::Instance::write_DKEL) {
	  for (HSPS::index_type k = 0; k < instance->n_invariants(); k++) {
	    std::cout << " (:invariant :set-constraint (";
	    if (instance->invariants[k].exact)
	      std::cout << "exactly-n 1";
	    else
	      std::cout << "at-most-n 1";
	    for (HSPS::index_type i = 0; i < instance->invariants[k].set.length(); i++)
	      std::cout << " "
			<< instance->atoms[instance->invariants[k].set[i]].name;
	    std::cout << "))" << std::endl;
	  }
	}
      }
      else {
	HSPS::index_type n_exact = 0;
	HSPS::index_type n_at_most = 0;
	for (HSPS::index_type k = 0; k < instance->n_invariants(); k++) {
	  if (instance->invariants[k].exact)
	    n_exact += 1;
	  else
	    n_at_most += 1;
	}
	std::cout << instance->name << " (find/verify): "
		  << instance->n_invariants() << " invariants ("
		  << n_exact << " exactly-n, " << n_at_most << " at-most-n)"
		  << std::endl;
      }
    }

    if (opt_relevant || opt_path_relevant) {
      if (std_relevant_atoms.length() > 0) {
	assert(std_relevant_atoms.length() == instance->n_atoms());
	assert(std_relevant_actions.length() == instance->n_actions());
	if (opt_print) {
	  for (HSPS::index_type k = 0; k < instance->n_atoms(); k++)
	    if (!std_relevant_atoms[k])
	      std::cout << " (:irrelevant :tag standard :tag L1C1 :tag L*C1 :fact "
			<< instance->atoms[k].name << ")" << std::endl;
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	    if (!std_relevant_actions[k])
	      std::cout << " (:irrelevant :tag standard :tag L1C1 :tag L*C1 :action "
			<< instance->actions[k].name << ")" << std::endl;
	}
	else {
	  std::cout << std_relevant_atoms.count(true) << " of "
		    << instance->n_atoms() << " atoms and "
		    << std_relevant_actions.count(true) << " of "
		    << instance->n_actions()
		    << " actions are (standard-)relevant" << std::endl;
	}
      }
      if (L1C1_relevant_atoms.length() > 0) {
	assert(L1C1_relevant_atoms.length() == instance->n_atoms());
	assert(L1C1_relevant_actions.length() == instance->n_actions());
	if (opt_print) {
	  for (HSPS::index_type k = 0; k < instance->n_atoms(); k++)
	    if (!L1C1_relevant_atoms[k] && LsC1_relevant_atoms[k])
	      std::cout << " (:irrelevant :tag L1C1 :fact "
			<< instance->atoms[k].name << ")" << std::endl;
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	    if (!L1C1_relevant_actions[k] && LsC1_relevant_actions[k])
	      std::cout << " (:irrelevant :tag L1C1 :action "
			<< instance->actions[k].name << ")" << std::endl;
	}
	else {
	  std::cout << L1C1_relevant_atoms.count(true) << " of "
		    << instance->n_atoms() << " atoms and "
		    << L1C1_relevant_actions.count(true) << " of "
		    << instance->n_actions()
		    << " actions are L1(C1)-relevant" << std::endl;
	}
      }
      if (LsC1_relevant_atoms.length() > 0) {
	assert(LsC1_relevant_atoms.length() == instance->n_atoms());
	assert(LsC1_relevant_actions.length() == instance->n_actions());
	if (opt_print) {
	  for (HSPS::index_type k = 0; k < instance->n_atoms(); k++)
	    if (!LsC1_relevant_atoms[k] && std_relevant_atoms[k])
	      std::cout << " (:irrelevant :tag L*C1 :tag L1C1 :fact "
			<< instance->atoms[k].name << ")" << std::endl;
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	    if (!LsC1_relevant_actions[k] && std_relevant_actions[k])
	      std::cout << " (:irrelevant :tag L*C1 :tag L1C1 :action "
			<< instance->actions[k].name << ")" << std::endl;
	}
	else {
	  std::cout << LsC1_relevant_atoms.count(true) << " of "
		    << instance->n_atoms() << " atoms and "
		    << LsC1_relevant_actions.count(true) << " of "
		    << instance->n_actions()
		    << " actions are L*(C1)-relevant" << std::endl;
	}
      }
      if (path_relevant_actions.length() > 0) {
	// assert(path_relevant_atoms.length() == instance->n_atoms());
	assert(path_relevant_actions.length() == instance->n_actions());
	if (opt_print) {
	  // for (HSPS::index_type k = 0; k < instance->n_atoms(); k++)
	  //   if (!path_relevant_atoms[k])
	  //     std::cout << " (:irrelevant :tag path :fact "
	  // 		<< instance->atoms[k].name << ")" << std::endl;
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	    if (!path_relevant_actions[k])
	      std::cout << " (:irrelevant :tag path :action "
			<< instance->actions[k].name << ")" << std::endl;
	}
	else {
	  std::cout << path_relevant_actions.count(true) << " of "
		    << instance->n_actions()
		    << " actions are path-relevant" << std::endl;
	}
      }
      if (relaxed_relevant_actions.length() > 0) {
	HSPS::index_vec p;
	assert(relaxed_relevant_actions.length() == instance->n_actions());
	if (opt_print) {
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	    if (!relaxed_relevant_actions[k])
	      std::cout << " (:irrelevant :tag relaxed :action "
			<< instance->actions[k].name << ")" << std::endl;
	}
	else {
	  std::cout << relaxed_relevant_actions.count(true) << " of "
		    << instance->n_actions()
		    << " actions are relaxed-relevant" << std::endl;
	}
      }
    }

    if (opt_print) {
      reader->write_end(std::cout);
    }
  }

  // plan output options

  if (opt_write_xml) {
    plans->writeXML(std::cout);
  }

  if (opt_pddl && !opt_instantiate) {
    for (HSPS::index_type k = 0; k < plans->length(); k++)
      (*plans)[k]->write(std::cout, HSPS::Name::NC_PDDL);
  }

  if (opt_ipc && !opt_instantiate) {
    for (HSPS::index_type k = 0; k < plans->length(); k++) {
      std::ostringstream fname;
      fname << "plan" << k << ".soln";
      std::cerr << "writing plan #" << k << " to " << fname.str() << "..."
		<< std::endl;
      std::ofstream p_out(fname.str().c_str());
      if ((*plans)[k]->plan_name()) {
	p_out << ";; " << (*plans)[k]->plan_name() << std::endl;
      }
      if (opt_sequential) {
	HSPS::PrintActions formatter(*instance, p_out);
	(*plans)[k]->output(formatter);
      }
      else {
	HSPS::PrintIPC formatter(*instance, p_out);
	formatter.set_epsilon(epsilon, false, opt_strict_ipc);
	(*plans)[k]->output(formatter);
      }
      p_out.close();
    }
  }

  if (opt_assoc) {
    for (HSPS::index_type k = 0; k < plans->length(); k++) {
      std::cout << "BEGIN ASSOC" << std::endl;
      HSPS::PrintAssoc print_assoc(*instance, std::cout);
      (*plans)[k]->output(print_assoc);
      std::cout << std::endl << "END ASSOC" << std::endl;
    }
  }

  if (opt_gantt) {
    for (HSPS::index_type k = 0; k < plans->length(); k++) {
      (*plans)[k]->writeGantt(std::cout);
    }
  }

  if (opt_pop) {
    for (HSPS::index_type k = 0; k < plans->length(); k++) {
      HSPS::SafePOP* pop = new HSPS::SafePOP(*instance);
      pop->construct((*plans)[k]->plan_steps(), opt_deorder, opt_sequential);
      if (opt_enforce_min_durations) pop->enforce_min_durations();
      if (opt_enforce_max_durations) pop->enforce_max_durations();
      pop->find_safe_causal_links();
      if (opt_enforce_min_makespan)
	pop->enforce_makespan((*plans)[k]->makespan());
      pop->write(std::cout);
      if (opt_write_graphs) {
	std::ostringstream fname;
	fname << "pop" << k << ".dot";
	std::ofstream g_out(fname.str().c_str());
	pop->write_graph(g_out);
	g_out.close();
      }
    }
    std::cout << ";; " << plans->length() << " plans deordered" << std::endl;
  }

  // instantiated output option

  if (opt_instantiate) {
    if (opt_print) {
      if (opt_domain) {
	if (opt_debug) {
	  instance->write_domain_init(std::cerr);
	  instance->write_domain_declarations(std::cerr);
	  instance->write_domain_actions(std::cerr);
	  instance->write_domain_DKEL_items(std::cerr);
	}
	else {
	  instance->write_domain_init(std::cout);
	  instance->write_domain_declarations(std::cout);
	  instance->write_domain_actions(std::cout);
	  instance->write_domain_DKEL_items(std::cout);
	} 
	if (HSPS::Instance::write_extra) {
	  HSPS::name_vec atm_names(0, 0);
	  instance->atom_names(atm_names);
	  HSPS::index_set_vec sets;
	  reader->export_sets(atm_names, sets);
	  std::cout << ";; atom sets" << std::endl;
	  for (HSPS::index_type k = 0; k < sets.length(); k++) {
	    instance->write_domain_atom_set(std::cout, sets[k]);
	  }
	  HSPS::name_vec act_names(0, 0);
	  instance->action_names(act_names);
	  sets.set_length(0);
	  reader->export_sets(act_names, sets);
	  std::cout << ";; action sets" << std::endl;
	  for (HSPS::index_type k = 0; k < sets.length(); k++) {
	    instance->write_domain_action_set(std::cout, sets[k]);
	  }
	  HSPS::name_vec set_names(0, 0);
	  sets.set_length(0);
	  reader->export_action_partitions(set_names, sets);
	  std::cout << ";; action partitions" << std::endl;
	  for (HSPS::index_type k = 0; k < sets.length(); k++) {
	    instance->remap_set(sets[k], prep->action_map);
	    instance->write_domain_action_set(std::cout, sets[k], set_names[k]);
	  }
	}
	std::cout << ")" << std::endl;
      }
      if (opt_problem) instance->write_problem(std::cout);
      if (HSPS::Instance::write_extra) {
	for (HSPS::index_type k = 0; k < plans->length(); k++) {
	  if (opt_ipc) {
	    HSPS::PrintIPC formatter(*instance, std::cout);
	    (*plans)[k]->output(formatter);
	  }
	  else {
	    (*plans)[k]->write(std::cout, HSPS::Name::NC_INSTANCE +
			       HSPS::Name::NC_PDDL);
	  }
	}
	HSPS::CostTable* input_h = new HSPS::CostTable(*instance, stats);
	reader->export_heuristic(*instance, prep->atom_map, false, *input_h);
      }
    }

    else if (opt_dump) {
      instance->print(std::cout);
    }

    else if (!opt_MATLAB) {
      NTYPE d_atm = R_TO_N(instance->n_atoms() - initial_instance_atom_count,
			   initial_instance_atom_count);
      NTYPE d_act = R_TO_N(instance->n_actions() -
			   initial_instance_action_count,
			   initial_instance_action_count);
      HSPS::index_type n_irrel = 0;
      HSPS::index_type n_init = 0;
      HSPS::index_type n_goal = 0;
      for (HSPS::index_type k = 0; k < instance->n_atoms(); k++) {
	if (instance->atoms[k].init) n_init += 1;
	if (instance->atoms[k].goal) n_goal += 1;
	if (instance->atoms[k].irrelevant) n_irrel += 1;
      }
      NTYPE sum_dur = 0;
      NTYPE sum_cost = 0;
      HSPS::index_type sum_n_pre = 0;
      HSPS::index_type sum_n_eff = 0;
      for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	sum_dur += instance->actions[k].dur;
	sum_cost += instance->actions[k].cost;
	sum_n_pre += instance->actions[k].pre.length();
	sum_n_eff += (instance->actions[k].add.length() +
		      instance->actions[k].del.length());
      }
      std::cout << instance->name << ": "
		<< instance->n_atoms() << " atoms ("
		<< n_init << " initial, " << n_goal << " goals, "
		<< n_irrel << " irrelevant, "
		<< PRINT_NTYPE(d_atm*100) << "%), ";
      if (opt_soft) {
	std::cout << ((HSPS::SoftInstance*)instance)->n_hard()
		  << " hard goals, "
		  << ((HSPS::SoftInstance*)instance)->n_soft()
		  << " soft goals, null value = "
		  << PRINT_NTYPE(((HSPS::SoftInstance*)instance)->null_value)
		  << ", ";
      }
      std::cout << instance->n_resources() << " resources ("
		<< instance->n_reusable_resources() << " used, "
		<< instance->n_consumable_resources() << " consumed), "
		<< instance->n_actions()
		<< " actions (min/max/sum/avg durations: "
		<< PRINT_NTYPE(instance->min_dur)
		<< "/" << PRINT_NTYPE(instance->max_dur)
		<< "/" << PRINT_NTYPE(sum_dur)
		<< "/" << PRINT_NTYPE(sum_dur / instance->n_actions())
		<< ", min/max/sum/avg cost: "
		<< PRINT_NTYPE(instance->min_cost)
		<< "/" << PRINT_NTYPE(instance->max_cost)
		<< "/" << PRINT_NTYPE(sum_cost)
		<< "/" << PRINT_NTYPE(sum_cost / instance->n_actions())
		<< ", avg #preconds: "
		<< (sum_n_pre/(double)instance->n_actions())
		<< ", avg #effects: "
		<< (sum_n_eff/(double)instance->n_actions())
		<< ", " << PRINT_NTYPE(d_act*100) << "%), ";
      if (opt_htn) {
	std::cout << ((HSPS::HTNInstance*)instance)->n_tasks()
		  << " tasks, ";
      }
      std::cout << instance->n_invariants() << " invariants";
      if (opt_soft) {
	std::cout << ", epsilon = "
		  << ((HSPS::SoftInstance*)instance)->compute_epsilon();
      }
      std::cout << ", " << plans->length() << " plans"
		<< std::endl;
    }

    if (opt_resource) {
      for (HSPS::index_type k = 0; k < instance->n_resources(); k++) {
	HSPS::CostTable* h_req = new HSPS::CostTable(*instance, stats);
	HSPS::bool_vec init(instance->init_atoms, instance->n_atoms());
	h_req->compute_H1max(HSPS::ResourceReqACF(*instance, k), init);
	NTYPE r_min = h_req->eval(instance->goal_atoms);
	delete h_req;
	HSPS::CostTable* h_cons = new HSPS::CostTable(*instance, stats);
	h_cons->compute_H1(HSPS::ResourceConsACF(*instance, k), init);
	NTYPE c_min = h_cons->eval(instance->goal_atoms);
	delete h_cons;
	std::cout << instance->resources[k].name
		  << ": min req. = " << r_min
		  << ", min cons. = " << c_min
		  << ", available = " << instance->resources[k].init
		  << std::endl;
      }
    }

    if (opt_write_graphs) {
      HSPS::graph rg(instance->n_atoms());
      for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	HSPS::Instance::Action& act = instance->actions[k];
	for (HSPS::index_type i = 0; i < act.pre.length(); i++)
	  for (HSPS::index_type j = 0; j < act.add.length(); j++)
	    rg.add_edge(act.pre[i], act.add[j]);
      }
      HSPS::index_set all_atoms;
      all_atoms.fill(instance->n_atoms());
      HSPS::bool_vec goal_atoms(instance->goal_atoms, instance->n_atoms());
      HSPS::bool_vec init_atoms(instance->init_atoms, instance->n_atoms());
      HSPS::bool_vec unreachable_atoms(false, instance->n_atoms());
      HSPS::bool_vec unreachable_actions(false, instance->n_actions());
      if (!opt_preprocess) {
	prep->compute_reachability(unreachable_atoms, unreachable_actions);
	unreachable_atoms.complement();
	unreachable_actions.complement();
      }
      std::ofstream rg_out("rg.dot");
      instance->write_atom_digraph(rg_out, rg, all_atoms, goal_atoms,
				   unreachable_atoms, "Atom Relevance Graph");
      rg_out.close();
      std::ofstream rgc_out("rg_compact.dot");
      rg.write_digraph(rgc_out, false, "Atom Relevance Graph");
      rgc_out.close();

      HSPS::graph rg2(instance->n_atoms() + instance->n_actions());
      for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	HSPS::Instance::Action& act = instance->actions[k];
	for (HSPS::index_type i = 0; i < act.pre.length(); i++)
	  rg2.add_edge(act.pre[i], instance->n_atoms() + k);
	for (HSPS::index_type i = 0; i < act.add.length(); i++)
	  rg2.add_edge(instance->n_atoms() + k, act.add[i]);
      }

      HSPS::index_set all_actions;
      all_actions.fill(instance->n_actions());
      goal_atoms.set_length(instance->n_atoms() + instance->n_actions());
      init_atoms.set_length(instance->n_atoms() + instance->n_actions());
      unreachable_atoms.append(unreachable_actions);
      std::ofstream aarg_out("aarg.dot");
      instance->write_atom_action_digraph(aarg_out, rg2,
					  all_atoms, all_actions,
					  goal_atoms, init_atoms,
					  unreachable_atoms,
					  "Atom/Action Relevance Graph");
      aarg_out.close();

      HSPS::graph cg;
      instance->causal_graph(cg);
      std::ofstream cg_out("strips-cg.dot");
      instance->write_atom_digraph(cg_out, cg, "Causal Graph (STRIPS)");
      cg_out.close();

      HSPS::index_set_graph pg;
      HSPS::index_set goal_nodes;
      instance->partitioning_graph(instance->goal_atoms, pg, goal_nodes);
      std::ofstream pg_out("pg.dot");
      instance->write_atom_set_graph(pg_out, pg, "Partitioning Graph");
      pg_out.close();

      HSPS::index_set sub_pg_nodes(goal_nodes);
      for (HSPS::index_type k = 0; k < goal_nodes.length(); k++)
	sub_pg_nodes.insert(pg.bidirectional(goal_nodes[k]));
      HSPS::index_set_graph sub_pg(pg, sub_pg_nodes);
      std::ofstream spg_out("spg.dot");
      instance->write_atom_set_graph(spg_out, sub_pg, "Partitioning (Sub-)Graph");
      spg_out.close();
      for (HSPS::index_type k = 0; k < goal_nodes.length(); k++) {
	sub_pg_nodes.assign_copy(pg.bidirectional(goal_nodes[k]));
	sub_pg_nodes.insert(goal_nodes[k]);
	pg.subgraph(sub_pg, sub_pg_nodes);
	std::ostringstream fname;
	fname << "spg" << k + 1 << ".dot";
	std::ofstream g_out(fname.str().c_str());
	instance->write_atom_set_graph(g_out, sub_pg, "Partitioning (Sub-)Graph");
	g_out.close();
      }

      HSPS::PreconditionEvaluator* pe =
	HSPS::PreconditionEvaluator::construct(*instance, R_TO_N(1, 2));
      std::ofstream peg_out("peg.dot");
      pe->write_graph(peg_out);
      peg_out.close();
    }

    if (opt_write_graphs || opt_MATLAB) {
      HSPS::index_graph png;
      png.init(instance->n_atoms() + instance->n_actions());
      HSPS::name_vec node_name(0, instance->n_atoms() + instance->n_actions());
      HSPS::index_type o = instance->n_atoms();
      for (HSPS::index_type k = 0; k < instance->n_atoms(); k++) {
	png.node_label(k) =
	  (HSPS::index_graph::NS_ELLIPSE +
	   (instance->atoms[k].init ? HSPS::index_graph::NS_FILLED : 0) +
	   (instance->atoms[k].goal ? HSPS::index_graph::NS_DOUBLE : 0));
	node_name[k] = instance->atoms[k].name;
      }
      for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	png.node_label(o + k) = HSPS::index_graph::NS_BOX;
	node_name[o + k] = instance->actions[k].name;
	for (HSPS::index_type i = 0; i < instance->actions[k].add.length(); i++) {
	  png.add_edge(o + k, instance->actions[k].add[i]);
	  png.edge_label(o + k, instance->actions[k].add[i]) =
	    HSPS::index_graph::ES_NORMAL + HSPS::index_graph::ED_FORWARD;
	}
	for (HSPS::index_type i = 0; i < instance->actions[k].pre.length(); i++) {
	  png.add_edge(instance->actions[k].pre[i], o + k);
	  if (instance->actions[k].del.contains(instance->actions[k].pre[i]))
	    png.edge_label(instance->actions[k].pre[i], o + k) =
	      HSPS::index_graph::ES_NORMAL + HSPS::index_graph::ED_FORWARD;
	  else
	    png.edge_label(instance->actions[k].pre[i], o + k) =
	      HSPS::index_graph::ES_DASHED + HSPS::index_graph::ED_FORWARD;
	}
	for (HSPS::index_type i = 0; i < instance->actions[k].del.length(); i++)
	  if (!instance->actions[k].pre.contains(instance->actions[k].del[i])) {
	    png.add_edge(o + k, instance->actions[k].del[i]);
	    png.edge_label(o + k, instance->actions[k].del[i]) =
	      HSPS::index_graph::ES_BOLD + HSPS::index_graph::ED_NONE;
	  }
      }
      std::ostringstream png_name;
      char* s = instance->name->to_cstring();
      if (opt_write_graphs) {
	png_name << s << "-png.dot";
	std::ofstream png_out(png_name.str().c_str());
	HSPS::write_styled_digraph<HSPS::name_vec>
	  (png_out, png, node_name, false, png_name.str().c_str());
	png_out.close();
	std::ostringstream bng_name;
	bng_name << s << "-bng.dot";
	std::ofstream bng_out(bng_name.str().c_str());
	png.write_styled_digraph(bng_out, false, bng_name.str().c_str());
	bng_out.close();
      }
      if (opt_MATLAB) {
	png.reflect();
	png.write_MATLAB(std::cout, instance->name->to_cstring(), "png");
      }
    }
  }

  // heuristic output options

  if (opt_lp) {
    HSPS::CostTable* h2 = new HSPS::CostTable(*instance, stats);
    h2->compute_H2(HSPS::CostACF(*instance));

    std::cout << "param n_atoms := " << instance->n_atoms() << ";"
	      << std::endl;
    std::cout << "param n_actions := " << instance->n_actions() << ";"
	      << std::endl;

    std::cout << "set Atoms :=";
    for (HSPS::index_type k = 0; k < instance->n_atoms(); k++)
      std::cout << " \"" << instance->atoms[k].name << "\"";
    std::cout << ";" << std::endl;

    std::cout << "set Actions :=";
    for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
      std::cout << " \"" << instance->actions[k].name << "\"";
    std::cout << " \"<INIT>\"" << ";" << std::endl;

    std::cout << "set Pairs :=";
    for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
      for (HSPS::index_type j = i; j < instance->n_atoms(); j++)
	if (prep->consistent(i, j))
	  std::cout << " (\"" << instance->atoms[i].name << "\", \""
		    << instance->atoms[j].name << "\")";
    std::cout << ";" << std::endl;

    // std::cout << "set Init :=";
    // for (HSPS::index_type k = 0; k < instance->n_atoms(); k++)
    //  if (instance->atoms[k].init)
    //	std::cout << " \"" << instance->atoms[k].name << "\"";
    // std::cout << ";" << std::endl;

    for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
      for (HSPS::index_type j = i; j < instance->n_atoms(); j++)
	if (prep->consistent(i, j)) {
	  std::cout << "set SuccMin[\"" << instance->atoms[i].name
		    << "\", \"" << instance->atoms[j].name << "\"] :=";
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	    if (!instance->actions[k].del.contains(i) &&
		!instance->actions[k].del.contains(j) &&
		(instance->actions[k].add.contains(i) ||
		 instance->actions[k].add.contains(j))) {
	      HSPS::index_set r(instance->actions[k].pre);
	      if (!instance->actions[k].add.contains(i))
		r.insert(i);
	      if (!instance->actions[k].add.contains(j))
		r.insert(j);
	      if (prep->consistent(r)) {
		std::cout << " \"" << instance->actions[k].name << "\"";
	      }
	    }
	  if (instance->atoms[i].init && instance->atoms[j].init)
	    std::cout << " \"<INIT>\"";
	  std::cout << ";" << std::endl;
	}

    for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
      for (HSPS::index_type j = i; j < instance->n_atoms(); j++)
	if (prep->consistent(i, j))
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	    if (!instance->actions[k].del.contains(i) &&
		!instance->actions[k].del.contains(j) &&
		(instance->actions[k].add.contains(i) ||
		 instance->actions[k].add.contains(j))) {
	      HSPS::index_set r(instance->actions[k].pre);
	      if (!instance->actions[k].add.contains(i))
		r.insert(i);
	      if (!instance->actions[k].add.contains(j))
		r.insert(j);
	      if (prep->consistent(r)) {
		std::cout << "set SuccMax[\"" << instance->atoms[i].name
			  << "\", \"" << instance->atoms[j].name << "\", \""
			  << instance->actions[k].name << "\"] :=";
		for (HSPS::index_type ri = 0; ri < r.length(); ri++)
		  for (HSPS::index_type rj = ri; rj < r.length(); rj++)
		    std::cout << " (\"" << instance->atoms[r[ri]].name
			      << "\", \"" << instance->atoms[r[rj]].name
			      << "\")";
		std::cout << ";" << std::endl;
	      }
	    }

    for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
      for (HSPS::index_type j = i; j < instance->n_atoms(); j++)
	if (instance->atoms[i].init && instance->atoms[j].init)
	  std::cout << "set SuccMax[\"" << instance->atoms[i].name
		    << "\", \"" << instance->atoms[j].name
		    << "\", \"<INIT>\"] := ;" << std::endl;

    std::cout << "param UBMax" << std::endl;
    for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
      for (HSPS::index_type j = i; j < instance->n_atoms(); j++)
	if (prep->consistent(i, j))
	  for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
	    if (!instance->actions[k].del.contains(i) &&
		!instance->actions[k].del.contains(j) &&
		(instance->actions[k].add.contains(i) ||
		 instance->actions[k].add.contains(j))) {
	      HSPS::index_set r(instance->actions[k].pre);
	      if (!instance->actions[k].add.contains(i))
		r.insert(i);
	      if (!instance->actions[k].add.contains(j))
		r.insert(j);
	      if (prep->consistent(r)) {
		std::cout << " [\"" << instance->atoms[i].name
			  << "\", \"" << instance->atoms[j].name << "\", \""
			  << instance->actions[k].name << "\"] := "
			  << h2->eval(r) << std::endl;
	      }
	    }
    for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
      for (HSPS::index_type j = i; j < instance->n_atoms(); j++)
	if (instance->atoms[i].init && instance->atoms[j].init)
	  std::cout << " [\"" << instance->atoms[i].name
		    << "\", \"" << instance->atoms[j].name
		    << "\", \"<INIT>\"] := 0" << std::endl;
    std::cout << " ;" << std::endl;

    std::cout << "param c :=";
    for (HSPS::index_type k = 0; k < instance->n_actions(); k++)
      std::cout << " [\"" << instance->actions[k].name << "\"]"
		<< instance->actions[k].cost;
    std::cout << " [\"<INIT>\"] 0";
    std::cout << ";" << std::endl;

    std::cout << "param wh " << std::endl;
    for (HSPS::index_type i = 0; i < instance->n_atoms(); i++)
      for (HSPS::index_type j = i; j < instance->n_atoms(); j++)
	if (prep->consistent(i, j))
	  std::cout << " [\"" << instance->atoms[i].name << "\", \""
		    << instance->atoms[j].name << "\"] := 1" << std::endl;
    std::cout << " ;" << std::endl;
  }

  if (opt_H1 || opt_H2 || opt_H3) {
    stats.start();
    HSPS::ACF* cost = 0;
    if (opt_discount) {
      HSPS::name_vec act_names(0, 0);
      instance->action_names(act_names);
      HSPS::index_set_vec sets;
      reader->export_sets(act_names, sets);
      if (sets.length() < selected_set) {
	std::cerr << "error: set " << selected_set << " chosen, but only "
		  << sets.length() << " action sets defined in input"
		  << std::endl;
	exit(1);
      }
      HSPS::ACF* c0 = new HSPS::CostACF(*instance);
      HSPS::bool_vec d(sets[selected_set], instance->n_actions());
      d.complement();
      cost = new HSPS::DiscountACF(*c0, d);
    }
    else {
      cost = new HSPS::CostACF(*instance);
    }
    HSPS::CostTable* h = new HSPS::CostTable(*instance, stats);
    if (opt_H1) {
      h->compute_H1(*cost);
    }
    else if (opt_H2) {
      h->compute_H2(*cost);
    }
    else if (opt_H3) {
      h->compute_H3(*cost);
    }
    stats.stop();
    std::cerr << "heuristic computed in " << stats.time() << " seconds"
	      << std::endl;
    std::cerr << "estimated cost of goals: " << h->eval(instance->goal_atoms)
	      << std::endl;

    if (opt_print) {
      h->write_pddl(std::cout, *instance);
    }

    if (opt_dump) {
      h->write(std::cout);
    }

    if (opt_write_graphs) {
      std::ofstream hg_out("hg.dot");
      prep->write_heuristic_graph(hg_out, *h, *cost);
      hg_out.close();

      HSPS::bool_vec r_atoms(false, instance->n_atoms());
      HSPS::bool_vec r_actions(false, instance->n_actions());
      for (HSPS::index_type k = 0; k < instance->goal_atoms.length(); k++)
	prep->relevant_at_cost(instance->goal_atoms[k],
			       h->eval(instance->goal_atoms[k]),
			       *h, *cost, r_atoms, r_actions);
      std::ofstream hgg_out("hgg.dot");
      prep->write_heuristic_graph(hgg_out, *h, *cost, r_atoms, r_actions);
      hgg_out.close();

      for (HSPS::index_type k = 0; k < instance->goal_atoms.length(); k++) {
	HSPS::index_type g = instance->goal_atoms[k];
	std::ostringstream fname;
	fname << "hg_";
	instance->atoms[g].name->write(fname, HSPS::Name::NC_INSTANCE);
	fname << ".dot";
	std::ofstream g_out(fname.str().c_str());
	r_atoms.assign_value(false, instance->n_atoms());
	r_actions.assign_value(false, instance->n_actions());
	prep->relevant_at_cost(g, h->eval(g), *h, *cost, r_atoms, r_actions);
	prep->write_heuristic_graph(g_out, *h, *cost, r_atoms, r_actions);
	g_out.close();
      }

      HSPS::graph srg(instance->n_atoms());
      for (HSPS::index_type k = 0; k < instance->n_actions(); k++) {
	HSPS::Instance::Action& act = instance->actions[k];
	NTYPE c_pre = h->eval(act.pre);
	for (HSPS::index_type i = 0; i < act.pre.length(); i++)
	  for (HSPS::index_type j = 0; j < act.add.length(); j++)
	    if (c_pre + 1 <= h->eval(act.add[j]))
	      srg.add_edge(act.pre[i], act.add[j]);
      }
      if (opt_reverse) srg.reverse();
      std::ofstream srgl_out("srg_labeled.dot");
      instance->write_atom_digraph(srgl_out, srg, "Strict Relevance");
      srgl_out.close();
      std::ofstream srg_out("srg.dot");
      srg.write_digraph(srg_out, false, "Strict Relevance");
      srg_out.close();
    }
  }

  if (opt_echo) {
    if (opt_print) {
      if (reader->domain_name && opt_domain)
	reader->write_dkel_domain(std::cout, false);
      if (reader->problem_name && opt_problem) {
	reader->write_dkel_problem(std::cout, true);
	if (HSPS::Instance::write_extra) reader->write_sets(std::cout);
	std::cout << ")" << std::endl;
      }
      if (HSPS::Instance::write_extra) {
	reader->write_plans(std::cout);
	if (reader->h_table.length() > 0)
	  reader->write_heuristic_table(std::cout);
      }
    }

    else if (opt_dump) {
      reader->print(std::cout);
    }

    else {
      std::cout << reader->dom_constants.length() << " objects of "
		<< reader->dom_types.length() << " types ("
		<< reader->dom_base_types.length() << " basic), "
		<< reader->dom_predicates.length() << " predicates, "
		<< reader->dom_actions.length() << " actions"
		<< std::endl;
      for (HSPS::index_type k = 0; k < reader->dom_predicates.length(); k++) {
	std::cout << "predicate ";
	reader->dom_predicates[k]->write_prototype(std::cout);
	std::cout << ": #init = "
		  << reader->dom_predicates[k]->init.count_values()
		  << std::endl;
      }
    }
  }

  return 0;
}
