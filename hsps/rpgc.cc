
#include "problem.h"
#include "random.h"
#include "preprocess.h"
#include "cost_table.h"
#include "seq_reg.h"
#include "para_reg.h"
#include "temporal.h"
#include "stats.h"
#include "plans.h"
#include "ida.h"
#include "bfs.h"
#include <sstream>
#include <fstream>

BEGIN_HSPS_NAMESPACE

struct Record {
  bool       solvable;
  NTYPE      est_cost;
  NTYPE      sol_cost;
  Statistics stats;
  Record() : solvable(false), est_cost(0), sol_cost(0) { };
};

bool test_solvable(Instance& ins)
{
  Statistics test_stats;
  CostTable test_cost(ins, test_stats);
  test_cost.compute_H2(ZeroACF());
  NTYPE val = test_cost.eval(ins.goal_atoms);
  return FINITE(val);
}

void solve_sequential
(Instance& ins, bool use_bfs, bool use_cost, bool use_cut, int search_level,
 int h_level, unsigned int time_limit, Record& rec)
{
  rec.stats.enable_interrupt(true);
  if (time_limit > 0) rec.stats.enable_time_out(time_limit, false);

  ACF* acf = (use_cost ? (ACF*)new CostACF(ins) : (ACF*)new UnitACF());
  CostTable cost_tab(ins, rec.stats);
  if (h_level > 1) {
    cost_tab.compute_H2(*acf);
  }
  else if (h_level > 0) {
    cost_tab.compute_H1(*acf);
  }
  rec.est_cost = cost_tab.eval(ins.goal_atoms);
  Result search_result;

  if (search_level > 0) {
    if (use_bfs) {
      SeqRegState search_root(ins, cost_tab, *acf, ins.goal_atoms);
      BFS search(rec.stats, search_result, 107);
      if (search_level == 1) search.set_cost_limit(rec.est_cost);
      rec.sol_cost = search.start(search_root);
      rec.solvable = search.solved();
    }
    else if (use_cut) {
      SeqCRegState search_root(ins, cost_tab, *acf, ins.goal_atoms);
      IDA search(rec.stats, search_result);
      if (search_level == 1) search.set_cost_limit(rec.est_cost);
      search.set_cycle_check(false);
      rec.sol_cost = search.start(search_root);
      rec.solvable = search.solved();
    }
    else {
      SeqRegState search_root(ins, cost_tab, *acf, ins.goal_atoms);
      IDA search(rec.stats, search_result);
      if (search_level == 1) search.set_cost_limit(rec.est_cost);
      search.set_cycle_check(false);
      rec.sol_cost = search.start(search_root);
      rec.solvable = search.solved();
    }
  }
  else {
    rec.sol_cost = rec.est_cost;
    rec.solvable = 0;
  }
}

void solve_parallel
(Instance& ins, bool use_bfs, bool use_cut, int search_level, int h_level,
 unsigned int time_limit, Record& rec)
{
  rec.stats.enable_interrupt(true);
  if (time_limit > 0) rec.stats.enable_time_out(time_limit, false);

  CostTable cost_tab(ins, rec.stats);
  if (h_level > 1) {
    cost_tab.compute_H2C(UnitACF(), false);
  }
  else if (h_level > 0) {
    cost_tab.compute_H1(UnitACF());
  }
  rec.est_cost = cost_tab.eval(ins.goal_atoms);
  Result search_result;

  if (search_level > 0) {
    if (use_bfs) {
      ParaRegState search_root(ins, cost_tab, ins.goal_atoms);
      BFS search(rec.stats, search_result, 107);
      if (search_level == 1) search.set_cost_limit(rec.est_cost);
      rec.sol_cost = search.start(search_root);
      rec.solvable = search.solved();
    }
    else if (use_cut) {
      ParaRSRegState search_root(ins, cost_tab, ins.goal_atoms);
      IDA search(rec.stats, search_result);
      if (search_level == 1) search.set_cost_limit(rec.est_cost);
      search.set_cycle_check(false);
      rec.sol_cost = search.start(search_root);
      rec.solvable = search.solved();
    }
    else {
      ParaRegState search_root(ins, cost_tab, ins.goal_atoms);
      IDA search(rec.stats, search_result);
      if (search_level == 1) search.set_cost_limit(rec.est_cost);
      search.set_cycle_check(false);
      rec.sol_cost = search.start(search_root);
      rec.solvable = search.solved();
    }
  }
  else {
    rec.sol_cost = rec.est_cost;
    rec.solvable = 0;
  }
}

void solve_temporal
(Instance& ins, bool use_bfs, bool use_cut, int search_level, int h_level,
 unsigned int time_limit, Record& rec)
{
  rec.stats.enable_interrupt(true);
  if (time_limit > 0) rec.stats.enable_time_out(time_limit, false);

  CostTable cost_tab(ins, rec.stats);
  if (h_level > 1) {
    cost_tab.compute_H2C(MakespanACF(ins), false);
  }
  else if (h_level > 0) {
    cost_tab.compute_H1(MakespanACF(ins));
  }
  Preprocessor prep(ins, rec.stats);
  prep.compute_ncw_sets(cost_tab);
  rec.est_cost = cost_tab.eval(ins.goal_atoms);
  Result search_result;

  if (search_level > 0) {
    if (use_bfs) {
      TemporalRegState search_root(ins, cost_tab, ins.goal_atoms);
      BFS search(rec.stats, search_result, 107);
      if (search_level == 1) search.set_cost_limit(rec.est_cost);
      rec.sol_cost = search.start(search_root);
      rec.solvable = search.solved();
    }
    else if (use_cut) {
      TemporalRSRegState search_root(ins, cost_tab, ins.goal_atoms);
      IDA search(rec.stats, search_result);
      if (search_level == 1) search.set_cost_limit(rec.est_cost);
      search.set_cycle_check(false);
      rec.sol_cost = search.start(search_root);
      rec.solvable = search.solved();
    }
    else {
      TemporalRegState search_root(ins, cost_tab, ins.goal_atoms);
      IDA search(rec.stats, search_result);
      if (search_level == 1) search.set_cost_limit(rec.est_cost);
      search.set_cycle_check(false);
      rec.sol_cost = search.start(search_root);
      rec.solvable = search.solved();
    }
  }
  else {
    rec.sol_cost = rec.est_cost;
    rec.solvable = 0;
  }
}

void check_heuristic(index_type instance_id, Instance& ins)
{
  Statistics stats;
  CostTable::Entry* list = 0;

  CostTable tab_h1(ins, stats);
  tab_h1.compute_H1(UnitACF());
  list = tab_h1.entries();
  bool h1_ok = true;
  for (CostTable::Entry* e = list; e && h1_ok; e = e->next)
    if (e->val.val != tab_h1.eval(e->set)) h1_ok = false;
  list->delete_list();

  CostTable tab_h2(ins, stats);
  tab_h2.compute_H2(UnitACF());
  list = tab_h2.entries();
  bool h2_ok = true;
  for (CostTable::Entry* e = list; e && h2_ok; e = e->next)
    if (e->val.val != tab_h2.eval(e->set)) h2_ok = false;
  list->delete_list();

  CostTable tab_h2c(ins, stats);
  tab_h2c.compute_H2C(UnitACF(), false);
  list = tab_h2c.entries();
  bool h2c_ok = true;
  for (CostTable::Entry* e = list; e && h2c_ok; e = e->next) {
    if (e->val.val != tab_h2c.eval(e->set)) {
      std::cerr << "error: ";
      ins.write_atom_set(std::cerr, e->set);
      std::cerr << " stored cost = " << e->val
		<< ", eval = " << tab_h2c.eval(e->set)
		<< std::endl;
      h2c_ok = false;
    }
  }
  list->delete_list();

  std::cout << "check " << instance_id
	    << " : " << (h1_ok ? 1 : 0)
	    << " " << (h2_ok ? 1 : 0)
	    << " " << (h2c_ok ? 1 : 0)
	    << " " << stats.total_time()
	    << std::endl;
}

int main(int argc, char *argv[]) {
  index_type par_n = 20;
  index_type par_o = 0;
  index_type par_p = 3;
  index_type par_q = 3;
  index_type par_g = 0;
  bool opt_random_g = false;
  index_type g_min = 0;
  index_type g_max = 0;
  bool opt_fixed_o = false;
  float density = 1.0;
  bool mod_v = false;
  bool mod_n = false;
  bool mod_f = false;
  bool mod_c = false;

  unsigned long rnd_seed = 0;
  index_type n_instances = 1;

  cost_vec v_values(1, 0);
  unsigned long vrnd_seed = 0;
  index_type n_variations = 0;

  bool opt_print = false;
  bool opt_save = false;
  bool opt_single_file = false;
  bool opt_prep = true;
  bool opt_print_before_prep = false;
  bool opt_info = false;
  bool opt_test = false;
  bool opt_solve_seq = false;
  bool opt_solve_par = false;
  bool opt_solve_temporal = false;
  bool opt_cost = false;
  bool opt_bfs = false;
  bool opt_cut = true;
  int  search_level = 2;
  int  h_level = 2;
  bool opt_check_h = false;
  const char* save_dir = "";

  unsigned int time_limit = 0;
  int verbose_level = 1;

  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-t") == 0) && (k < argc - 1)) {
      time_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      verbose_level = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-n") == 0) && (k < argc - 1)) {
      par_n = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-o") == 0) && (k < argc - 1)) {
      par_o = atoi(argv[++k]);
      opt_fixed_o = true;
    }
    else if ((strcmp(argv[k],"-d") == 0) && (k < argc - 1)) {
      density = atof(argv[++k]);
      opt_fixed_o = false;
    }
    else if ((strcmp(argv[k],"-p") == 0) && (k < argc - 1)) {
      par_p = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-q") == 0) && (k < argc - 1)) {
      par_q = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-g") == 0) && (k < argc - 1)) {
      par_g = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-rnd-g") == 0) && (k < argc - 2)) {
      g_min = atoi(argv[++k]);
      g_max = atoi(argv[++k]);
      opt_random_g = true;
    }
    else if ((strcmp(argv[k],"-m") == 0) && (k < argc - 5)) {
      par_n = atoi(argv[++k]);
      par_p = atoi(argv[++k]);
      par_q = atoi(argv[++k]);
      par_g = atoi(argv[++k]);
      int f = atoi(argv[++k]);
      if ((f % 10) > 0) mod_v = true;
      if (((f / 10) % 10) > 0) mod_f = true;
      if (((f / 100) % 10) > 0) mod_c = true;
      if (((f / 1000) % 10) > 0) mod_n = true;
    }
    else if ((strcmp(argv[k],"-cfg") == 0) && (k < argc - 6)) {
      par_n = atoi(argv[++k]);
      par_o = atoi(argv[++k]);
      par_p = atoi(argv[++k]);
      par_q = atoi(argv[++k]);
      par_g = atoi(argv[++k]);
      int f = atoi(argv[++k]);
      if ((f % 10) > 0) mod_v = true;
      if (((f / 10) % 10) > 0) mod_f = true;
      if (((f / 100) % 10) > 0) mod_c = true;
      if (((f / 1000) % 10) > 0) mod_n = true;
      opt_fixed_o = true;
    }
    else if (strcmp(argv[k],"-var") == 0) mod_v = true;
    else if (strcmp(argv[k],"-neg") == 0) mod_n = true;
    else if (strcmp(argv[k],"-flip") == 0) {
      mod_f = true;
      mod_n = true;
    }
    else if (strcmp(argv[k],"-cons") == 0) {
      mod_c = true;
    }
    else if (((strcmp(argv[k],"-rnd") == 0) ||
	      (strcmp(argv[k],"-r") == 0)) &&
	     (k < argc - 1)) {
      rnd_seed = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-rnd2") == 0) && (k < argc - 1)) {
      vrnd_seed = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-i") == 0) && (k < argc - 1)) {
      n_instances = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-j") == 0) && (k < argc - 1)) {
      n_variations = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-val") == 0) && (k < argc - 1)) {
      index_type n_val = atoi(argv[++k]);
      if (k + n_val > argc) {
	std::cerr << "error: -val " << n_val << ", not enough values"
		  << std::endl;
      }
      while (n_val > 0) {
	v_values.append(A_TO_N(argv[++k]));
	n_val -= 1;
      }
    }
    else if (strcmp(argv[k],"-test") == 0) {
      opt_test = true;
    }
    else if (strcmp(argv[k],"-info") == 0) {
      opt_info = true;
      opt_print = false;
    }
    else if (strcmp(argv[k],"-seq") == 0) {
      opt_solve_seq = true;
      opt_print = false;
    }
    else if (strcmp(argv[k],"-par") == 0) {
      opt_solve_par = true;
      opt_print = false;
    }
    else if (strcmp(argv[k],"-time") == 0) {
      opt_solve_temporal = true;
      opt_print = false;
    }
    else if (strcmp(argv[k],"-check") == 0) {
      opt_check_h = true;
      opt_print = false;
    }
    else if (strcmp(argv[k],"-prep") == 0) opt_prep = true;
    else if (strcmp(argv[k],"-no-prep") == 0) opt_prep = false;
    else if (strcmp(argv[k],"-cut") == 0) opt_cut = true;
    else if (strcmp(argv[k],"-no-cut") == 0) opt_cut = false;
    else if (strcmp(argv[k],"-cost") == 0) opt_cost = true;
    else if (strcmp(argv[k],"-bfs") == 0) opt_bfs = true;
    else if ((strcmp(argv[k],"-s") == 0) && (k < argc - 1)) {
      search_level = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-h") == 0) && (k < argc - 1)) {
      h_level = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-print") == 0) opt_print = true;
    else if (strcmp(argv[k],"-print-original") == 0)
      opt_print_before_prep = true;
    else if (strcmp(argv[k],"-save") == 0) opt_save = true;
    else if (strcmp(argv[k],"-save-1") == 0) {
      opt_save = true;
      opt_single_file = true;
    }
    else if (strcmp(argv[k],"-save-2") == 0) {
      opt_save = true;
      opt_single_file = false;
    }
    else if ((strcmp(argv[k],"-dir") == 0) && (k < argc - 1)) {
      save_dir = argv[++k];
    }
    else if (strcmp(argv[k],"-no-dkel") == 0) Instance::write_DKEL = false;
    else if (strcmp(argv[k],"-no-neg") == 0) Instance::write_negation = false;
    else if (strcmp(argv[k],"-pddl1") == 0) Instance::write_PDDL2 = false;
    else if (strcmp(argv[k],"-write-parameters") == 0)
      Instance::always_write_parameters = true;
    else if (strcmp(argv[k],"-write-requirements") == 0)
      Instance::always_write_requirements = true;
  }

  if ((par_g == 0) && !opt_random_g) {
    par_g = ((long) floor(par_n * 0.15));
    if (par_g < 2) par_g = 2;
  }
  if (!opt_fixed_o) {
    par_o = ((long) floor(par_n * log(par_g) * density)) + 1;
  }

  if (!opt_random_g) {
    if (verbose_level > 0) {
      std::cerr << par_n << " atoms, "
		<< par_o << " actions, "
		<< par_g << " goals"
		<< std::endl;
    }
    std::cout << "cfg " << par_n
	      << " " << par_o
	      << " " << par_p
	      << " " << par_q
	      << " " << par_g
	      << " " << ((mod_n || mod_f) ? 1 : 0)
	      << ((mod_c || !mod_n) ? 1 : 0)
	      << (mod_f ? 1 : 0)
	      << (mod_v ? 1 : 0)
	      << std::endl;
  }

  if (v_values.length() > 0) {
    std::cout << "val " << v_values.length();
    for (index_type k = 0; k < v_values.length(); k++)
      std::cout << " " << v_values[k];
    std::cout << std::endl;
  }

  LC_RNG rng;
  if (rnd_seed > 0) rng.seed(rnd_seed);
  LC_RNG vrng;
  if (vrnd_seed > 0) {
    vrng.seed(vrnd_seed);
  }
  else {
    vrnd_seed = vrng.seed_value();
  }

  std::cout << "rnd " << rng.seed_value() << std::endl;
  std::cout << "rnd2 " << vrng.seed_value() << std::endl;

  SearchAlgorithm::default_trace_level = verbose_level - 1;
  Preprocessor::default_trace_level = verbose_level - 1;
  CostTable::default_trace_level = verbose_level - 2;
  Instance::default_trace_level = verbose_level - 2;

  index_type i = 0;
  while (i < n_instances) {
    if (opt_random_g) {
      index_type g_range = (g_max - g_min) + 1;
      par_g = (rng.random() % g_range) + g_min;
    }
    if (!opt_fixed_o) {
      par_o = ((long) floor(par_n * log(par_g) * density)) + 1;
    }

    unsigned long instance_id = rng.seed_value();

    if (verbose_level > 0) {
      std::cerr << "generating instance " << instance_id;
      if (opt_random_g) {
	std::cerr << " (" << par_n << " atoms, " << par_o << " actions, "
		  << par_g << " goals)";
      }
      std::cerr << "..." << std::endl;
    }

    RandomInstance instance(par_n, par_o, par_p, par_q, par_g,
			    mod_v, mod_n, mod_f, mod_c, rng);
    instance.generate_domain();
    instance.generate_problem();
    instance.cross_reference();

    bool test_ok = true;
    if (opt_test) {
      test_ok = test_solvable(instance);
    }

    if (test_ok) {
      if (opt_random_g) {
	std::cout << "cfg " << par_n
		  << " " << par_o
		  << " " << par_p
		  << " " << par_q
		  << " " << par_g
		  << " " << (mod_n ? 1 : 0) << (mod_v ? 1 : 0)
		  << std::endl;
      }

      instance.set_variation_values(v_values);
      vrng.seed(vrnd_seed);

      for (index_type j = 0; j < (n_variations == 0 ? 1 : n_variations); j++) {
	if (n_variations > 0) {
	  if (verbose_level > 0) {
	    std::cerr << "generating variation " << vrng.seed_value()
		      << "..." << std::endl;
	  }
	  instance.generate_variation(vrng);
	}

	Instance m_instance(instance);
	
	m_instance.cross_reference();
	if (opt_prep && opt_print_before_prep) {
	  std::cout << ";; instance before preprocessing" << std::endl;
	  m_instance.write_domain(std::cout);
	  m_instance.write_problem(std::cout);
	}

	if (opt_prep) {
	  Statistics prep_stats;
	  Preprocessor prep(m_instance, prep_stats);
	  prep.preprocess();
	}

	if (opt_print) {
	  if (opt_prep)
	    std::cout << ";; instance AFTER preprocessing" << std::endl;
	  m_instance.write_domain(std::cout);
	  m_instance.write_problem(std::cout);
	}

	if (opt_save) {
	  std::string dir_name(save_dir);
	  if (strlen(save_dir) > 0) {
	    if (save_dir[strlen(save_dir) - 1] != '/') dir_name += "/";
	  }

	  std::ostringstream base_file_name;
	  base_file_name << m_instance.name << ".pddl";

	  if (opt_single_file) {
	    if (verbose_level > 0)
	      std::cerr << "writing domain/problem "
			<< base_file_name.str() << "..."
			<< std::endl;
	    std::ofstream save_file(base_file_name.str().c_str());
	    m_instance.write_domain(save_file);
	    m_instance.write_problem(save_file);
	    save_file.close();
	  }
	  else {
	    std::string domain_file_name =
	      dir_name + std::string("dom-") + base_file_name.str();
	    if (verbose_level > 0)
	      std::cerr << "writing domain " << domain_file_name << "..."
			<< std::endl;
	    std::ofstream domain_file(domain_file_name.c_str());
	    m_instance.write_domain(domain_file);
	    domain_file.close();

	    std::string problem_file_name =
	      dir_name + std::string("pro-") + base_file_name.str();
	    if (verbose_level > 0)
	      std::cerr << "writing problem " << problem_file_name << "..."
			<< std::endl;
	    std::ofstream problem_file(problem_file_name.c_str());
	    m_instance.write_problem(problem_file);
	    problem_file.close();
	  }
	}

	if (opt_info) {
	  std::cout << "info " << instance_id;
	  if (instance.variation_id() > 0)
	    std::cout << "/" << instance.variation_id();
	  std::cout << " : " << m_instance.n_atoms() << " "
		    << m_instance.n_actions();
	  if (n_variations > 0) {
#ifdef NTYPE_RATIONAL
	    long v_gcd = 0;
	    if (m_instance.n_actions() > 0) {
	      v_gcd = m_instance.actions[0].cost.numerator();
	      for (index_type k = 1; k < m_instance.n_actions(); k++)
		v_gcd = gcd(v_gcd, m_instance.actions[k].cost.numerator());
	    }
	    std::cout << " " << v_gcd;
#endif
	    double sum_v = 0;
	    double sum_sq_v = 0;
	    double sum_n = 0;
	    for (index_type k = 0; k < m_instance.n_actions(); k++) {
#ifdef NTYPE_RATIONAL
	      double v = m_instance.actions[k].cost.decimal();
#else
	      double v = m_instance.actions[k].cost;
#endif
	      sum_v += v;
	      sum_sq_v += (v * v);
	      sum_n += 1;
	    }
	    double v_avg = sum_v/sum_n;
	    double v_dev = sqrt(sum_sq_v + sum_n*v_avg*v_avg - 2*v_avg*sum_v);
	    double sum_r = 0;
	    double sum_sq_r = 0;
	    sum_n = 0;
	    for (index_type k1 = 0; k1 < m_instance.n_actions(); k1++)
	      for (index_type k2 = k1 + 1; k2 < m_instance.n_actions(); k2++) {
#ifdef NTYPE_RATIONAL
		double v_max =
		  rational::max(m_instance.actions[k1].cost,
				m_instance.actions[k2].cost).decimal();
		double v_min =
		  rational::min(m_instance.actions[k1].cost,
				m_instance.actions[k2].cost).decimal();
#else
		double v_max = MAX(m_instance.actions[k1].cost,
				   m_instance.actions[k2].cost);
		double v_min = MIN(m_instance.actions[k1].cost,
				   m_instance.actions[k2].cost);
#endif
		double r = v_max/v_min;
		sum_r += r;
		sum_sq_r += (r * r);
		sum_n += 1;
	      }
	    double r_avg = sum_r/sum_n;
	    double r_dev = sqrt(sum_sq_r + sum_n*r_avg*r_avg - 2*r_avg*sum_r);
	    std::cout << " " << v_avg << " " << v_dev
		      << " " << r_avg << " " << r_dev;
	  }
	  std::cout << std::endl;
	}

	if (opt_check_h) {
	  check_heuristic(instance_id, m_instance);
	}

	if (opt_solve_seq) {
	  if (verbose_level > 0)
	    std::cerr << "solving the sequential version..." << std::endl;
	  Record solution;
	  solve_sequential(m_instance, opt_bfs, opt_cost, opt_cut,
			   search_level, h_level, time_limit, solution);
	  std::cout << "seq " << instance_id;
	  if (instance.variation_id() > 0)
	    std::cout << "/" << instance.variation_id();
	  std::cout << " : " << m_instance.n_atoms()
		    << " " << m_instance.n_actions()
		    << " " << (solution.solvable ? '1' : '0')
		    << " " << solution.est_cost << " " << solution.sol_cost
		    << " " << solution.stats.nodes()
		    << " " << solution.stats.time()
		    << " " << solution.stats.complete_iterations()
		    << " " << solution.stats.total_time()
		    << " " << solution.stats.peak_memory()
		    << std::endl;
	}

	if (opt_solve_par) {
	  if (verbose_level > 0)
	    std::cerr << "solving the parallel version..." << std::endl;
	  Record solution;
	  solve_parallel(m_instance, opt_bfs, opt_cut, search_level, h_level,
			 time_limit, solution);
	  std::cout << "par " << instance_id;
	  if (instance.variation_id() > 0)
	    std::cout << "/" << instance.variation_id();
	  std::cout << " : " << m_instance.n_atoms()
		    << " " << m_instance.n_actions()
		    << " " << (solution.solvable ? '1' : '0')
		    << " " << solution.est_cost << " " << solution.sol_cost
		    << " " << solution.stats.nodes()
		    << " " << solution.stats.time()
		    << " " << solution.stats.complete_iterations()
		    << " " << solution.stats.total_time()
		    << " " << solution.stats.peak_memory()
		    << std::endl;
	}

	if (opt_solve_temporal) {
	  if (verbose_level > 0)
	    std::cerr << "solving the temporal version..." << std::endl;
	  Record solution;
	  solve_temporal(m_instance, opt_bfs, opt_cut, search_level, h_level,
			 time_limit, solution);
	  std::cout << "time " << instance_id;
	  if (instance.variation_id() > 0)
	    std::cout << "/" << instance.variation_id();
	  std::cout << " : " << m_instance.n_atoms()
		    << " " << m_instance.n_actions()
		    << " " << (solution.solvable ? '1' : '0')
		    << " " << solution.est_cost << " " << solution.sol_cost
		    << " " << solution.stats.nodes()
		    << " " << solution.stats.time()
		    << " " << solution.stats.complete_iterations()
		    << " " << solution.stats.total_time()
		    << " " << solution.stats.peak_memory()
		    << std::endl;
	}
      }

      i += 1;
    }
    else {
      if (verbose_level > 0)
	std::cerr << "instance " << instance_id << " failed solvability test"
		  << std::endl;
    }
  }

  std::cout << "next " << rng.seed_value() << std::endl;

  return 0;
}

END_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
