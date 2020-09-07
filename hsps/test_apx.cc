
#include "problem.h"
#include "preprocess.h"
#include "parser.h"
#include "cost_table.h"
#include "seq_reg.h"
#include "para_reg.h"
#include "temporal.h"
#include "plans.h"
#include "idao.h"
#include "itest.h"

BEGIN_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  bool        opt_sequential = false;
  bool        opt_temporal = false;
  bool        opt_cost = false;
  bool        opt_cc = false;
  index_type  max_apx_limit = 9;
  double      time_limit = 0;
  int         verbose_level = 1;
  bool        opt_no = false;

  StringTable symbols(50, lowercase_map);
  Parser* reader = new Parser(symbols);

  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      verbose_level = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-use-strict-borrow") == 0) {
      PDDL_Base::use_strict_borrow_definition = true;
    }
    else if (strcmp(argv[k],"-use-extended-borrow") == 0) {
      PDDL_Base::use_extended_borrow_definition = true;
    }
    else if (strcmp(argv[k],"-seq") == 0) {
      opt_sequential = true;
    }
    else if (strcmp(argv[k],"-time") == 0) {
      opt_temporal = true;
    }
    else if (strcmp(argv[k],"-cost") == 0) {
      opt_cost = true;
    }
    else if ((strcmp(argv[k],"-t") == 0) && (k < argc - 1)) {
      time_limit = atof(argv[++k]);
    }
    else if ((strcmp(argv[k],"-m") == 0) && (k < argc - 1)) {
      max_apx_limit = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-no-alpha") == 0) {
      IDAO::alpha_cut = false;
    }
    else if (strcmp(argv[k],"-max-no") == 0) {
      opt_no = true;
    }
    else if (*argv[k] != '-') reader->read(argv[k], false);
  }

  SearchAlgorithm::default_trace_level = verbose_level;
  Heuristic::default_trace_level = verbose_level;
  Instance::default_trace_level = verbose_level;
  Preprocessor::default_trace_level = verbose_level;
  if (verbose_level < 1) {
    PDDL_Base::write_warnings = false;
  }

  Instance    instance;
  Statistics  stats;
  stats.enable_interrupt(true);
  stats.start();

  std::cerr << "instantiating..." << std::endl;
  reader->instantiate(instance);
  Preprocessor prep(instance, stats);
  std::cerr << "preprocessing..." << std::endl;
  prep.preprocess();

  std::cerr << "instance " << instance.name << " built in "
	    << stats.time() << " seconds" << std::endl;
  std::cerr << instance.n_atoms() << " atoms, "
	    << instance.n_resources() << " resources ("
	    << instance.n_reusable_resources() << " reusable, "
	    << instance.n_consumable_resources() << " consumable), "
	    << instance.n_actions() << " actions" << std::endl;

  CostTable cost_tab(instance, stats);
  estimator_vec resource_cons_est(0, instance.n_resources());

  std::cerr << "computing heuristic..." << std::endl;
  if (opt_temporal) {
    cost_tab.compute_H2C(MakespanACF(instance), false);
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

  if (opt_temporal) {
    std::cerr << "computing n.c.w. sets..." << std::endl;
    prep.compute_ncw_sets(cost_tab);
  }

  bool solved = false;
  bool apx_solved = true;
  Statistics apx_stats;

  for (index_type apx_limit = 3; apx_limit <= max_apx_limit; apx_limit++) {
    NTYPE starting_est_cost = cost_tab.eval(instance.goal_atoms);
    std::cerr << "estimated cost before "
	      << apx_limit << "-approximate search: "
	      << PRINT_NTYPE(starting_est_cost) << std::endl;
    NTYPE new_est_cost = starting_est_cost;
    apx_stats.reset();
    double hr = 0;

    if (apx_solved && !solved) {
      AtomSet::max_set_size_encountered = 0;
#ifdef SEARCH_EXTRA_STATS
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

      State* apx_root = 0;
      if (opt_temporal) {
	apx_root = new ApxTemporalRegState
	  (instance, cost_tab, instance.goal_atoms, 0, apx_limit);
      }
      else if (opt_sequential) {
	ACF* acf =
	  (opt_cost ? (ACF*)new CostACF(instance) : (ACF*)new UnitACF());
	apx_root = new ApxSeqRegState
	  (instance, cost_tab, *acf, instance.goal_atoms, 0, apx_limit);
      }
      else {
	apx_root = new ApxParaRegState
	  (instance, cost_tab, instance.goal_atoms, apx_limit);
      }

      ApxResult apx_res;
      HashTable* sol_tab = new HashTable(10001);
      IDAO* apx_search = 0;

      if (opt_no) {
	apx_search = new kIDAO(apx_stats, apx_res, sol_tab);
      }
      else {
	apx_search = new IDAO(apx_stats, apx_res, sol_tab);
      }
      apx_search->set_cycle_check(opt_cc);
      apx_search->set_store_cost(true);
      if (time_limit > 0) apx_stats.enable_time_out(time_limit, false);
      new_est_cost = apx_search->start(*apx_root);
      if (apx_search->solved()) {
	if (apx_res.min_solution()) solved = true;
      }
      else {
	apx_solved = false;
      }
      hr = sol_tab->hit_ratio();

      delete apx_search;
      delete sol_tab;
      delete apx_root;
    }

    std::cout << instance.name
	      << ' ' << (solved ? 1 : 0)
	      << ' ' << apx_limit
	      << ' ' << (apx_solved ? 1 : 0)
	      << ' ' << PRINT_NTYPE(starting_est_cost)
	      << ' ' << PRINT_NTYPE(new_est_cost)
	      << ' ' << apx_stats.total_min_nodes()
	      << ' ' << apx_stats.total_max_nodes()
	      << ' ' << apx_stats.total_time()
	      << ' ' << apx_stats.peak_memory()
	      << ' ' << hr
#ifdef SEARCH_EXTRA_STATS
	      << ' ' << (rminx_size/(double)rminx_count)
	      << ' ' << (rminc_succ_size_ratio/(double)rminc_count)
	      << ' ' << (rminx_succ/(double)rminx_count)
	      << ' ' << (rmaxx_size/(double)rmaxx_count)
	      << ' ' << (rmaxx_succ/(double)rmaxx_count)
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
