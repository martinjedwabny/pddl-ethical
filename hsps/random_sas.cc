
#include "sas.h"
#include "preprocess.h"
#include "cost_table.h"
#include "stats.h"
#include "base.h"
#include <sstream>
#include <fstream>
#include <math.h>

int main(int argc, char *argv[]) {
  HSPS::index_type n_var   = 5;
  HSPS::index_type n_val   = 5;
  HSPS::index_type dtg_type = 0;
  HSPS::rational   mixf(0, 1);
  bool       cg_tree = false;
  bool       cg_star = false;
  HSPS::index_type cg_star_nucleus_size = 1;
  bool       cg_coupled_layered = false;
  HSPS::rational   d_cg    = HSPS::rational(1,2);
  HSPS::rational   t_cg    = HSPS::rational(9,10);
  HSPS::index_type c_max   = 2;
  double     af      = 0.0;
  HSPS::index_type a_min   = 0;
  HSPS::index_type n_goals = 3;

  unsigned long rnd_seed = 0;
  HSPS::index_type n_instances = 1;

  bool opt_info = false;
  bool opt_filter = false;
  bool opt_print = false;
  bool opt_save = false;
  bool opt_single_file = false;
  bool opt_suggest_sets = false;
  const char* save_dir = "";

  unsigned int time_limit = 0;
  int verbose_level = 1;

  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      verbose_level = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-n") == 0) && (k < argc - 1)) {
      n_var = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-m") == 0) && (k < argc - 1)) {
      n_val = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-g") == 0) && (k < argc - 1)) {
      n_goals = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-tree-cg") == 0) {
      cg_tree = true;
    }
    else if ((strcmp(argv[k],"-star-cg") == 0) && (k < argc - 1)) {
      cg_star = true;
      cg_star_nucleus_size = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-dcg") == 0) && (k < argc - 1)) {
      d_cg = HSPS::rational::ator(argv[++k]);
    }
    else if ((strcmp(argv[k],"-tolerance") == 0) && (k < argc - 1)) {
      t_cg = HSPS::rational::ator(argv[++k]);
    }
    else if ((strcmp(argv[k],"-cs") == 0) && (k < argc - 1)) {
      c_max = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-a") == 0) && (k < argc - 1)) {
      a_min = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-af") == 0) && (k < argc - 1)) {
      af = atof(argv[++k]);
      a_min = ((HSPS::index_type)floor(n_var * n_val * log(n_goals) * af)) + 1;
    }
    else if (strcmp(argv[k],"-sc-dtg") == 0) dtg_type = 1;
    else if (strcmp(argv[k],"-uc-dtg") == 0) dtg_type = 2;
    else if (strcmp(argv[k],"-dc-dtg") == 0) dtg_type = 3;
    else if ((strcmp(argv[k],"-dtg") == 0) && (k < argc - 1)) {
      dtg_type = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-mix") == 0) && (k < argc - 1)) {
      mixf = HSPS::rational::ator(argv[++k]);
    }
    else if (((strcmp(argv[k],"-rnd") == 0) ||
	      (strcmp(argv[k],"-r") == 0)) &&
	     (k < argc - 1)) {
      rnd_seed = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-i") == 0) && (k < argc - 1)) {
      n_instances = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-info") == 0) opt_info = true;
    else if (strcmp(argv[k],"-filter") == 0) {
      opt_filter = true;
      opt_info = true;
    }
    else if (strcmp(argv[k],"-print") == 0) opt_print = true;
    else if (strcmp(argv[k],"-save") == 0) opt_save = true;
    else if (strcmp(argv[k],"-save-1") == 0) {
      opt_save = true;
      opt_single_file = true;
    }
    else if (strcmp(argv[k],"-sets") == 0) {
      opt_suggest_sets = true;
    }
    else if (strcmp(argv[k],"-save-2") == 0) {
      opt_save = true;
      opt_single_file = false;
    }
    else if ((strcmp(argv[k],"-dir") == 0) && (k < argc - 1)) {
      save_dir = argv[++k];
    }
    else if (strcmp(argv[k],"-pq-param") == 0)
      HSPS::Instance::always_write_parameters = true;
    else if (strcmp(argv[k],"-pq-req") == 0)
      HSPS::Instance::always_write_requirements = true;
  }

  HSPS::Instance::write_DKEL = true;
  HSPS::Instance::write_PDDL2 = false;

  HSPS::Statistics stats;

  HSPS::LC_RNG rng;
  if (rnd_seed > 0) rng.seed(rnd_seed);

  std::cout << "rnd " << rng.seed_value() << std::endl;

  std::ostringstream d_name;
  d_name << "RSV_" << n_var << "X" << n_val
	 << "_" << "DTG" << dtg_type;
  if (mixf > 0) {
    d_name << "M" << mixf.decimal();
  }
  if (cg_tree) {
    d_name << "_TREE";
  }
  else if (cg_star) {
    d_name << "_STAR" << cg_star_nucleus_size;
  }
  else if (cg_coupled_layered) {
    d_name << "_CL";
  }
  else {
    d_name << "_RND" << d_cg.decimal();
  }
  d_name << "_CS" << c_max;
  if (af > 0.0) {
    d_name << "_F" << af;
  }
  else if (a_min > 0) {
    d_name << "_A" << a_min;
  }

  HSPS::index_type i = 0;
  while (i < n_instances) {
    unsigned long instance_id = rng.seed_value();
    std::cerr << "generating instance " << instance_id << "..." << std::endl;

    HSPS::graph cg(n_var);
    HSPS::index_set goal_vars;

    if (cg_tree) {
      for (HSPS::index_type k = 1; k < n_var; k++) {
	HSPS::index_type i = rng.random_in_range(k);
	cg.add_edge(i, k);
      }
      goal_vars.fill(n_var);
    }
    else if (cg_star) {
      if (n_var < cg_star_nucleus_size + 2) {
	std::cerr << "error: can't build star-type cg with size "
		  << cg_star_nucleus_size << " nucleus from "
		  << n_var << " variables" << std::endl;
	exit(255);
      }
      for (HSPS::index_type i = 0; i < cg_star_nucleus_size; i++)
	for (HSPS::index_type j = i + 1; j < cg_star_nucleus_size; j++)
	  cg.add_undirected_edge(i, j);
      for (HSPS::index_type k = cg_star_nucleus_size; k < n_var; k++)
	for (HSPS::index_type i = 0; i < cg_star_nucleus_size; i++)
	  cg.add_undirected_edge(k, i);
      for (HSPS::index_type k = cg_star_nucleus_size; k < n_var; k++)
	goal_vars.insert(k);
    }
    else if (cg_coupled_layered) {
      std::cerr << "N.Y.I." << std::endl;
      exit(0);
    }
    else {
      cg.random_digraph_with_density(d_cg, rng);
      goal_vars.fill(n_var);
    }

    std::ostringstream i_name;
    i_name << "G" << n_goals << "_I" << instance_id;
    HSPS::Name* n = new HSPS::InstanceName(strdup(d_name.str().c_str()),
					   strdup(i_name.str().c_str()));
    HSPS::SASInstance* sas_ins = new HSPS::SASInstance(n);
    sas_ins->trace_level = verbose_level;
    sas_ins->construct_random_domain(n_var, n_val, dtg_type, mixf,
				     cg, c_max, t_cg, a_min, rng);
    sas_ins->construct_random_instance(n_goals, rng);

    if (verbose_level > 1) {
      sas_ins->write_domain(std::cout);
    }

    std::cerr << "converting instance to STRIPS..." << std::endl;
    HSPS::Instance* strips_ins = sas_ins->convert_to_STRIPS();
    strips_ins->cross_reference();

    if (opt_print) {
      strips_ins->write_domain(std::cout);
      strips_ins->write_problem(std::cout);
    }

    bool ok = true;

    if (opt_info) {
      HSPS::Preprocessor* prep = new HSPS::Preprocessor(*strips_ins, stats);
      prep->preprocess();
      HSPS::CostTable* h = new HSPS::CostTable(*strips_ins, stats);
      h->compute_H2(HSPS::UnitACF());
      std::cout << "info " << instance_id << ": "
		<< strips_ins->n_atoms() << " atoms, "
		<< strips_ins->n_actions() << " actions, estimated cost: "
		<< h->eval(strips_ins->goal_atoms)
		<< ", target cg density = "
		<< HSPS::rational(cg.n_edges(), n_var * (n_var - 1)).decimal()
		<< ", actual cg density = "
		<< HSPS::rational(sas_ins->causal_graph.n_edges(),
				  n_var * (n_var - 1)).decimal()
		<< std::endl;
      if (opt_filter) {
	if (INFINITE(h->eval(strips_ins->goal_atoms)))
	  ok = false;
      }
    }

    if (opt_save && ok) {
      std::string dir_name(save_dir);
      if (strlen(save_dir) > 0) {
	if (save_dir[strlen(save_dir) - 1] != '/') dir_name += "/";
      }

      std::ostringstream base_file_name;
      base_file_name << d_name.str() << "_" << i_name.str() << ".pddl";

      if (opt_single_file) {
	if (verbose_level > 0)
	  std::cerr << "writing domain/problem "
		    << base_file_name.str() << "..."
		    << std::endl;
	std::ofstream save_file(base_file_name.str().c_str());
	strips_ins->write_domain(save_file);
	strips_ins->write_problem(save_file);
	if (opt_suggest_sets) {
	  if (cg_star) {
	    save_file << "(define (problem " << i_name.str() << ")"
		      << std::endl;
	    for (HSPS::index_type k = cg_star_nucleus_size; k < n_var; k++) {
	      save_file << " (:set";
	      for (HSPS::index_type i = 0; i < cg_star_nucleus_size; i++) {
		save_file << " " << sas_ins->variables[i].name;
	      }
	      save_file << " " << sas_ins->variables[k].name
			<< ")" << std::endl;
	      save_file << " (:set " << sas_ins->variables[k].name
			<< ")" << std::endl;
	    }
	    save_file << ")" << std::endl;
	  }
	}
	save_file.close();
      }
      else {
	std::string domain_file_name =
	  dir_name + std::string("dom-") + base_file_name.str();
	if (verbose_level > 0)
	  std::cerr << "writing domain " << domain_file_name << "..."
		    << std::endl;
	std::ofstream domain_file(domain_file_name.c_str());
	strips_ins->write_domain(domain_file);
	domain_file.close();

	std::string problem_file_name =
	  dir_name + std::string("pro-") + base_file_name.str();
	if (verbose_level > 0)
	  std::cerr << "writing problem " << problem_file_name << "..."
		    << std::endl;
	std::ofstream problem_file(problem_file_name.c_str());
	strips_ins->write_problem(problem_file);
	problem_file.close();
      }
    }

    delete strips_ins;
    delete sas_ins;

    if (ok) {
      i += 1;
    }
  }

  std::cout << "next " << rng.seed_value() << std::endl;

  return 0;
}
