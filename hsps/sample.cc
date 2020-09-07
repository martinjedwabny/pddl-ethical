
#include "problem.h"
#include "preprocess.h"
#include "parser.h"
#include "random.h"
#include "cost_table.h"
#include "pdb.h"
#include "pdb_construction.h"

// #define RIB_RANDOM_SEED 31337
// #define RIB_RANDOM_SEED 87213
// #define RIB_RANDOM_SEED 19755
#define RIB_RANDOM_SEED 25681
// #define RIB_RANDOM_SEED 57631
#define RIB_N_SWAPS 10000
#define SAMPLE_H2
#define SAMPLE_FWD_1PDB
#define SAMPLE_BWD_1PDB
#define SAMPLE_FWD_IP_PDB
#define SAMPLE_BWD_IP_PDB
#define SAMPLE_IP_PDB_ALL
// #define PRINT_FWD_IP_DISTRIBUTION
// #define PRINT_BWD_IP_DISTRIBUTION
#define SAMPLE_FWD_WIB_PDB
#define SAMPLE_BWD_WIB_PDB
// #define SAMPLE_FWD_RIB_PDB
// #define SAMPLE_BWD_RIB_PDB
#define BIN_SPANNING false
#define BIN_COLLAPSE true
#define MAXIMAL_ADDITIVE false

// #define USE_PLAIN_INC
// #define SAMPLE_COMBINED_BWD_WIB_PDB

int main(int argc, char *argv[]) {
  StringTable symbols(50, lowercase_map);

  bool        opt_cost = false;
  bool        opt_preprocess = true;
  bool        opt_rm_irrelevant = false;
  bool        opt_find_invariants = true;
  bool        opt_verify_invariants = true;

  index_type  opt_pdb_size = 20000;
  bool        opt_sas_min = false;

  double      time_limit = 0;
  count_type  n_samples = 1;
  count_type  forward_steps = 1000;
  count_type  reverse_steps = 0;
  unsigned long random_seed = 0;
  int         verbose_level = 1;

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

    // sampling options
    else if ((strcmp(argv[k],"-s") == 0) && (k < argc - 1)) {
      forward_steps = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-rev") == 0) && (k < argc - 1)) {
      reverse_steps = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-n") == 0) && (k < argc - 1)) {
      n_samples = atoi(argv[++k]);
    }
    else if (((strcmp(argv[k],"-rnd") == 0) || (strcmp(argv[k],"-r") == 0)) &&
	     (k < argc - 1)) {
      random_seed = atoi(argv[++k]);
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
      opt_find_invariants = true;
    }
    else if (strcmp(argv[k],"-verify") == 0) {
      opt_verify_invariants = true;
    }
    else if (strcmp(argv[k],"-no-verify") == 0) {
      opt_verify_invariants = true;
    }

    // misc. options
    else if (strcmp(argv[k],"-min") == 0) {
      opt_sas_min = true;
    }
    else if ((strcmp(argv[k],"-t") == 0) && (k < argc - 1)) {
      time_limit = atof(argv[++k]);
    }

    // input file
    else if (*argv[k] != '-') {
      reader->read(argv[k], false);
    }
  }

  Heuristic::default_trace_level = verbose_level - 1;
  Instance::default_trace_level = verbose_level - 1;
  Preprocessor::default_trace_level = verbose_level - 2;
  if (verbose_level <= 0) PDDL_Base::write_warnings = false;
  if (verbose_level > 1) PDDL_Base::write_info = true;

  Instance    instance;
  Statistics  stats;
  Preprocessor prep(instance, stats);

  stats.enable_interrupt(true);
  if (time_limit > 0) stats.enable_time_out(time_limit, false);

  std::cerr << "instantiating..." << std::endl;
  reader->instantiate(instance);
  if (opt_preprocess) {
    std::cerr << "preprocessing..." << std::endl;
    prep.preprocess();
  }
  else {
    std::cerr << "cross referencing..." << std::endl;
    instance.cross_reference();
  }
  if (opt_find_invariants) {
    prep.bfs_find_invariants();
  }
  if (opt_verify_invariants) {
    instance.verify_invariants();
  }

  LC_RNG random_source;
  if (random_seed > 0) random_source.seed(random_seed);
  RandomWalk generator(instance);

  LC_RNG rib_rng;
  Statistics stats_h_2;
#ifdef SAMPLE_FWD_1PDB
  Statistics stats_h_f1;
#endif
#ifdef SAMPLE_BWD_1PDB
  Statistics stats_h_r1;
#endif
#ifdef SAMPLE_FWD_IP_PDB
  Statistics stats_h_f2;
#endif
#ifdef SAMPLE_BWD_IP_PDB
  Statistics stats_h_r2;
#endif
#ifdef SAMPLE_FWD_WIB_PDB
  Statistics stats_h_fwib;
#endif
#ifdef SAMPLE_BWD_WIB_PDB
  Statistics stats_h_rwib;
#endif
#ifdef SAMPLE_FWD_RIB_PDB
  Statistics stats_h_frib;
#endif
#ifdef SAMPLE_BWD_RIB_PDB
  Statistics stats_h_rrib;
#endif
  count_type n_samples_generated = 0;
  stats.start();

  while ((n_samples > 0) && !stats.break_signal_raised()) {
    unsigned long id = random_source.seed_value();
    std::cerr << "generating instance " << id << ":" << forward_steps
	      << ":" << reverse_steps << "..." << std::endl;
    Instance* ins = 0;
    if (reverse_steps > 0) {
      ins = generator.next_instance(forward_steps, reverse_steps,
				    random_source);
    }
    else {
      ins = generator.next_forward_instance(forward_steps, random_source);
    }
    if (ins == 0) {
      std::cerr << "alert: instance " << id << ":" << forward_steps
		<< ":" << reverse_steps << " impossible" << std::endl;
      random_source.seed(id);
      random_source.random();
    }
    else {
      std::cerr << "initial state: ";
      ins->write_atom_set(std::cerr, ins->init_atoms);
      std::cerr << std::endl;
      std::cerr << "goal: ";
      ins->write_atom_set(std::cerr, ins->goal_atoms);
      std::cerr << std::endl;
      std::cerr << "cross referencing..." << std::endl;
      ins->cross_reference();
      std::cerr << "constructing SAS+ instance..." << std::endl;
      SASInstance* sas_ins =
	new SASInstance(*ins, opt_verify_invariants, opt_sas_min);
      SASInstance* res = sas_ins->restrict_to_goal_variables();

      Heuristic* h_inc = 0;

#ifdef SAMPLE_H2
      std::cerr << "computing H^2..." << std::endl;
      CostTable* h_2 = new CostTable(*ins, stats_h_2);
      h_2->compute_H2(UnitACF());
      NTYPE val_h_2 = h_2->eval(ins->goal_atoms);
      std::cerr << "H^2 = " << val_h_2 << std::endl;
      h_inc = h_2;
#else
      std::cerr << "computing inconsistency relation..." << std::endl;
      CostTable* h_j = new CostTable(*ins, stats_h_2);
      h_j->compute_H2(ZeroACF());
      h_inc = h_j;
#endif

#ifdef USE_PLAIN_INC
      std::cerr << "computing inconsistency relation..." << std::endl;
      CostTable* h_j = new CostTable(*ins, stats_h_2);
      h_j->compute_H2(ZeroACF());
      h_inc = h_j;
#endif

      // forward single-variable
#ifdef SAMPLE_FWD_1PDB
      std::cerr << "computing forward 1-PDB..." << std::endl;
      Max1PDB* h_f1 = new Max1PDB(*res, stats_h_f1);
      h_f1->compute_sets();
      h_f1->compute_progression_PDB(UnitACF(), h_inc);
      h_f1->compute_additive_groups(MAXIMAL_ADDITIVE);
      h_f1->make_additive_groups();
      NTYPE val_h_f1 = h_f1->eval(res->init_state);
      std::cerr << "forward 1-PDB = " << val_h_f1 << std::endl;
      delete h_f1;
#endif

      // backward single-variable
#ifdef SAMPLE_BWD_1PDB
      std::cerr << "computing backward 1-PDB..." << std::endl;
      Max1PDB* h_r1 = new Max1PDB(*sas_ins, stats_h_r1);
      h_r1->compute_sets();
      h_r1->compute_regression_PDB(UnitACF(), h_inc);
      h_r1->compute_additive_groups(MAXIMAL_ADDITIVE);
      h_r1->make_additive_groups();
      NTYPE val_h_r1 = h_r1->eval(sas_ins->goal_state);
      std::cerr << "backward 1-PDB = " << val_h_r1 << std::endl;
      delete h_r1;
#endif

      // forward independent pair
#ifdef SAMPLE_FWD_IP_PDB
      std::cerr << "computing forward I.P. PDB..." << std::endl;
      IndependentPairPDB* h_f2 = new IndependentPairPDB(*res, stats_h_f2);
      h_f2->compute_sets();
      h_f2->compute_progression_PDB(UnitACF(), h_inc);
#ifdef SAMPLE_IP_PDB_ALL
      h_f2->compute_all_additive_groups();
#else
      h_f2->compute_additive_groups(MAXIMAL_ADDITIVE);
#endif
      h_f2->make_additive_groups();
      NTYPE val_h_f2 = h_f2->eval(res->init_state);
#ifdef SAMPLE_IP_PDB_ALL
      NTYPE min_h_f2 = h_f2->min_component(res->init_state);
#endif
#ifdef SAMPLE_IP_PDB_ALL
      std::cerr << "forward all I.P. PDB = " << val_h_f2
		<< " (best) / " << min_h_f2 << " (worst)"
		<< std::endl;
#else
      std::cerr << "forward I.P. PDB = " << val_h_f2 << std::endl;
#endif
#endif

      // backward independent pair
#ifdef SAMPLE_BWD_IP_PDB
      std::cerr << "computing backward I.P. PDB..." << std::endl;
      IndependentPairPDB* h_r2 = new IndependentPairPDB(*sas_ins, stats_h_r2);
      h_r2->compute_sets();
      h_r2->compute_regression_PDB(UnitACF(), h_inc);
#ifdef SAMPLE_IP_PDB_ALL
      h_r2->compute_all_additive_groups();
#else
      h_r2->compute_additive_groups(MAXIMAL_ADDITIVE);
#endif
      h_r2->make_additive_groups();
      NTYPE val_h_r2 = h_r2->eval(sas_ins->goal_state);
#ifdef SAMPLE_IP_PDB_ALL
      NTYPE min_h_r2 = h_r2->min_component(sas_ins->goal_state);
#endif
#ifdef SAMPLE_IP_PDB_ALL
      std::cerr << "backward all I.P. PDB = " << val_h_r2
		<< " (best) / " << min_h_r2 << " (worst)"
		<< std::endl;
#else
      std::cerr << "backward I.P. PDB = " << val_h_r2 << std::endl;
#endif
#endif

      // forward W.I.B.
#ifdef SAMPLE_FWD_WIB_PDB
      std::cerr << "computing forward W.I.B. PDB..." << std::endl;
      GoalStateInterference sv_gsi_r(*res);
      InverseCGFraction sv_icg_r(*res);
      SetValueSum sv_sum_r(sv_gsi_r, sv_icg_r);
      WeightedIndependentBinPDB* h_fwib =
	new WeightedIndependentBinPDB(*res, stats_h_fwib, sv_sum_r);
      h_fwib->compute_sets(opt_pdb_size, ProgressionPDBSize(res->signature),
			   BIN_SPANNING, BIN_COLLAPSE);
      h_fwib->compute_progression_PDB(UnitACF(), h_inc);
      h_fwib->compute_additive_groups(MAXIMAL_ADDITIVE);
      //      std::cerr << "additive groups: ";
      //      h_fwib->write_additive_groups(std::cerr);
      //      std::cerr << std::endl;
      h_fwib->make_additive_groups();
      NTYPE val_h_fwib = h_fwib->eval(res->init_state);
      std::cerr << "forward W.I.B. PDB = " << val_h_fwib << std::endl;
      delete h_fwib;
#endif

      // backward W.I.B.
#ifdef SAMPLE_BWD_WIB_PDB
      std::cerr << "computing backward W.I.B. PDB..." << std::endl;
      InitialStateInterference sv_isi(*sas_ins);
      InverseCGFraction sv_icg(*sas_ins);
      SetValueSum sv_sum(sv_isi, sv_icg);
      WeightedIndependentBinPDB* h_rwib =
	new WeightedIndependentBinPDB(*sas_ins, stats_h_rwib, sv_sum);
      h_rwib->compute_sets(opt_pdb_size, RegressionPDBSize(sas_ins->signature),
			   BIN_SPANNING, BIN_COLLAPSE);
      h_rwib->compute_regression_PDB(UnitACF(), h_inc);
      h_rwib->compute_additive_groups(MAXIMAL_ADDITIVE);
      h_rwib->make_additive_groups();
      NTYPE val_h_rwib = h_rwib->eval(sas_ins->goal_state);
      std::cerr << "backward W.I.B. PDB = " << val_h_rwib << std::endl;
      delete h_rwib;
#endif

      // backward W.I.B.
#ifdef SAMPLE_COMBINED_BWD_WIB_PDB
      std::cerr << "computing backward W.I.B. PDB combined with h^2..."
		<< std::endl;
      InitialStateInterference sv_isi(*sas_ins);
      InverseCGFraction sv_icg(*sas_ins);
      SetValueSum sv_sum(sv_isi, sv_icg);
      WeightedIndependentBinPDB* h_rwibc =
	new WeightedIndependentBinPDB(*sas_ins, stats_h_rwib, sv_sum);
      h_rwibc->compute_sets(opt_pdb_size,
			    RegressionPDBSize(sas_ins->signature),
			    BIN_SPANNING, BIN_COLLAPSE);
      h_rwibc->compute_regression_PDB(UnitACF(), h_2);
      h_rwibc->compute_additive_groups(MAXIMAL_ADDITIVE);
      h_rwibc->make_additive_groups();
      NTYPE val_h_rwibc = h_rwibc->eval(sas_ins->goal_state);
      std::cerr << "backward W.I.B. PDB = " << val_h_rwibc << std::endl;
      delete h_rwibc;
#endif

      // forward random independent bin
#ifdef SAMPLE_FWD_RIB_PDB
      std::cerr << "computing forward R.I.B. PDB..." << std::endl;
      rib_rng.seed(RIB_RANDOM_SEED);
      RandomIndependentBinPDB* h_frib =
	new RandomIndependentBinPDB(*res, stats_h_frib);
      h_frib->compute_sets(opt_pdb_size, ProgressionPDBSize(res->signature),
			   BIN_SPANNING, BIN_COLLAPSE, rib_rng, RIB_N_SWAPS);
      h_frib->compute_progression_PDB(UnitACF(), h_inc);
      h_frib->compute_additive_groups(MAXIMAL_ADDITIVE);
      h_frib->make_additive_groups();
      NTYPE val_h_frib = h_frib->eval(res->init_state);
      std::cerr << "forward R.I.B. PDB = " << val_h_frib << std::endl;
      delete h_frib;
#endif

      // backward random independent bin
#ifdef SAMPLE_BWD_RIB_PDB
      std::cerr << "computing backward R.I.B. PDB..." << std::endl;
      rib_rng.seed(RIB_RANDOM_SEED);
      RandomIndependentBinPDB* h_rrib =
	new RandomIndependentBinPDB(*sas_ins, stats_h_rrib);
      h_rrib->compute_sets(opt_pdb_size, RegressionPDBSize(sas_ins->signature),
			   BIN_SPANNING, BIN_COLLAPSE, rib_rng, RIB_N_SWAPS);
      h_rrib->compute_regression_PDB(UnitACF(), h_inc);
      h_rrib->compute_additive_groups(MAXIMAL_ADDITIVE);
      h_rrib->make_additive_groups();
      NTYPE val_h_rrib = h_rrib->eval(sas_ins->goal_state);
      std::cerr << "backward R.I.B. PDB = " << val_h_rrib << std::endl;
      delete h_rrib;
#endif

      // print summary line
      std::cout << id << ":" << forward_steps << ":" << reverse_steps
#ifdef SAMPLE_H2
		<< " " << val_h_2
#endif
#ifdef SAMPLE_FWD_1PDB
		<< " " << val_h_f1
#endif
#ifdef SAMPLE_BWD_1PDB
		<< " " << val_h_r1
#endif
#ifdef SAMPLE_FWD_IP_PDB
		<< " " << val_h_f2
#ifdef SAMPLE_IP_PDB_ALL
		<< " " << min_h_f2
#endif
#endif
#ifdef SAMPLE_BWD_IP_PDB
		<< " " << val_h_r2
#ifdef SAMPLE_IP_PDB_ALL
		<< " " << min_h_r2
#endif
#endif
#ifdef SAMPLE_FWD_WIB_PDB
		<< " " << val_h_fwib
#endif
#ifdef SAMPLE_BWD_WIB_PDB
		<< " " << val_h_rwib
#endif
#ifdef SAMPLE_COMBINED_BWD_WIB_PDB
		<< " " << val_h_rwibc
#endif
#ifdef SAMPLE_FWD_RIB_PDB
		<< " " << val_h_frib
#endif
#ifdef SAMPLE_BWD_RIB_PDB
		<< " " << val_h_rrib
#endif
		<< std::endl;
      n_samples_generated += 1;
      std::cerr << n_samples_generated << " samples generated, "
		<< stats << std::endl;

#ifdef SAMPLE_FWD_IP_PDB
#ifdef PRINT_FWD_IP_DISTRIBUTION
      cost_vec vals(ZERO);
      h_f2->component_values(res->init_state, vals);
      std::cout << vals << std::endl;
#endif
      delete h_f2;
#endif

#ifdef SAMPLE_BWD_IP_PDB
#ifdef PRINT_BWD_IP_DISTRIBUTION
      h_r2->component_values(sas_ins->goal_state, vals);
      std::cout << vals << std::endl;
#endif
      delete h_r2;
#endif

      delete h_2;
      delete res;
      delete sas_ins;
      delete ins;
      n_samples -= 1;
    }
  }

  stats.stop();
  std::cout << n_samples_generated
#ifdef SAMPLE_H2
	    << " " << stats_h_2.total_time()
#endif
#ifdef SAMPLE_FWD_1PDB
	    << " " << stats_h_f1.total_time()
#endif
#ifdef SAMPLE_BWD_1PDB
	    << " " << stats_h_r1.total_time()
#endif
#ifdef SAMPLE_FWD_IP_PDB
	    << " " << stats_h_f2.total_time()
#endif
#ifdef SAMPLE_BWD_IP_PDB
	    << " " << stats_h_r2.total_time()
#endif
#ifdef SAMPLE_FWD_WIB_PDB
	    << " " << stats_h_fwib.total_time()
#endif
#ifdef SAMPLE_BWD_WIB_PDB
	    << " " << stats_h_rwib.total_time()
#endif
#ifdef SAMPLE_FWD_RIB_PDB
	    << " " << stats_h_frib.total_time()
#endif
#ifdef SAMPLE_BWD_RIB_PDB
	    << " " << stats_h_rrib.total_time()
#endif
	    << std::endl;

  return 0;
}
