
#include "parser.h"
#include "preprocess.h"
#include "hypergraph.h"
#include "cost_table.h"
#include "enumerators.h"

#include "temporal.h"
#include "idao.h"

#include <sstream>
#include <fstream>

BEGIN_HSPS_NAMESPACE

typedef swapable_pair<NTYPE> npair;
typedef lvector<npair> np_vec;

struct sd {
  index_type atom;
  lvector<npair> dv;
};

typedef lvector<sd> sd_vec;

class IFC : public np_vec::order {
public:
  virtual bool operator()(const npair& v0, const npair& v1) const;
};

bool IFC::operator()(const npair& v0, const npair& v1) const
{
  return (v0.first < v1.first);
}

void add_soft_deadline
(index_type a, NTYPE at, NTYPE val, sd_vec& sdv)
{
  for (index_type k = 0; k < sdv.length(); k++)
    if (sdv[k].atom == a) {
      sdv[k].dv.insert_ordered(npair(at, val), IFC());
      return;
    }
  sd& new_sd = sdv.append();
  new_sd.atom = a;
  new_sd.dv.append(npair(at, val));
}

StringTable symbols(50, lowercase_map);
Parser* reader = new Parser(symbols);

int main(int argc, char *argv[]) {
  bool     opt_preprocess = true;
  bool     opt_test = true;
  bool     opt_load = true;
  bool     opt_pddl = false;
  bool     opt_verbose = false;
  index_type opt_min_d = 1;
  index_type opt_max_d = 2;
  unsigned int random_seed = 0;

  PDDL_Base::write_warnings = false;
  PDDL_Base::compile_away_plan_constraints = false;

  Instance::write_PDDL2 = false;
  Instance::write_PDDL3 = false;
  Instance::write_metric = false;
  Instance::write_time = false;

  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      Instance::default_trace_level = atoi(argv[++k]);
      opt_verbose = true;
    }
    else if (strcmp(argv[k],"-no-prep") == 0) {
      opt_preprocess = false;
    }
    else if ((strcmp(argv[k],"-catc") == 0) && (k < argc - 1)) {
      HSPS::PDDL_Name::catc = *argv[++k];
    }
    else if ((strcmp(argv[k],"-d-min") == 0) && (k < argc - 1)) {
      opt_min_d = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-d-max") == 0) && (k < argc - 1)) {
      opt_max_d = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-no-test") == 0) {
      opt_test = false;
    }
    else if (strcmp(argv[k],"-pddl") == 0) {
      opt_pddl = true;
    }
    else if (strcmp(argv[k],"-load") == 0) {
      opt_load = true;
    }
    else if (strcmp(argv[k],"-no-load") == 0) {
      opt_load = false;
    }
    else if (strcmp(argv[k],"-no-dkel") == 0) {
      HSPS::Instance::write_DKEL = false;
    }
    else if (strcmp(argv[k],"-no-negation") == 0) {
      HSPS::Instance::write_negation = false;
    }
    else if (((strcmp(argv[k],"-rnd") == 0) ||
	      (strcmp(argv[k],"-r") == 0)) &&
	     (k < argc - 1)) {
      random_seed = atoi(argv[++k]);
    }
    else if (*argv[k] != '-') {
      reader->read(argv[k], false);
    }
  }

  Statistics stats;
  stats.enable_interrupt(true);
  Instance instance;
  // sd_vec   sds;
  index_vec   sd_atom;
  cost_vec    sd_time;
  cost_vec    sd_val;
  PDDL_Base::symbol_vec sd_name;
  index_set   sd_atoms;

  reader->post_process();

  stats.start();
  reader->instantiate(instance);
  index_set hard_goals;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (instance.atoms[k].goal)
      hard_goals.insert(k);

  for (index_type k = 0; k < reader->dom_preferences.length(); k++) {
    if (reader->dom_preferences[k]->goal->g_class == PDDL_Base::goal_within) {
      PDDL_Base::DeadlineGoal* dg =
	(PDDL_Base::DeadlineGoal*)reader->dom_preferences[k]->goal;
      if (dg->goal->is_propositional() &&
	  dg->goal->is_state() &&
	  dg->goal->is_singular()) {
	index_set s;
	dg->goal->instantiate(instance, s, 0, 0);
	assert(s.length() == 1);
	NTYPE v = reader->dom_preferences[k]->value(reader->metric_type,
						    reader->metric);
	// add_soft_deadline(s[0], dg->at, v, sds);
	sd_atom.append(s[0]);
	sd_atoms.insert(s[0]);
	sd_time.append(dg->at);
	sd_val.append(v);
	sd_name.append(reader->dom_preferences[k]->name);
      }
      else {
	std::cerr << "can't handle goal: ";
	dg->print(std::cerr);
	std::cerr << std::endl;
      }
    }
    else {
      std::cerr << "preference ";
      reader->dom_preferences[k]->print(std::cerr);
      std::cerr << " is not a deadline goal" << std::endl;
    }
  }

  for (index_type k = 0; k < sd_atoms.length(); k++) {
    index_type h = instance.create_history_atom(sd_atoms[k]);
    for (index_type i = 0; i < sd_atom.length(); i++)
      if (sd_atom[i] == sd_atoms[k])
	sd_atom[i] = h;
  }

  hypergraph h(sd_atom.length());

  Preprocessor prep(instance, stats);

  if (opt_preprocess) {
    prep.preprocess();
    for (index_type k = 0; k < sd_atom.length(); k++)
      sd_atom[k] = prep.atom_map[sd_atom[k]];
    instance.remap_set(sd_atoms, prep.atom_map);
    instance.remap_set(hard_goals, prep.atom_map);
  }
  else {
    instance.cross_reference();
  }
  instance.set_goal(hard_goals);

  stats.stop();
  std::cerr << "instantiation and preprocessing finished in "
	    << stats.time() << " seconds" << std::endl;

  NTYPE v_sum = 0;

  std::cerr << sd_atom.length() << " soft deadline goals:" << std::endl;
  for (index_type k = 0; k < sd_atom.length(); k++) {
    std::cerr << k << "." << (sd_name[k] ? sd_name[k]->print_name : "?")
	      << ": " << instance.atoms[sd_atom[k]].name << ": " << sd_time[k]
	      << " (" << PRINT_NTYPE(sd_val[k]) << ")"
	      << std::endl;
    v_sum += sd_val[k];
  }
  std::cerr << "sum value = " << PRINT_NTYPE(v_sum) << std::endl;
  std::cerr << instance.n_atoms() << " atoms and "
	    << instance.n_actions() << " actions"
	    << std::endl;

//   if (opt_verbose) {
//     instance.write_domain(std::cerr);
//     instance.write_problem(std::cerr);
//   }

  CostTable lb(instance, stats);
  lb.compute_H2C(MakespanACF(instance), false);

  bool done = false;
  Statistics apx_stats;
  for (index_type d = 3; d <= opt_max_d; d++) {
    NTYPE starting_est_cost = lb.eval(sd_atoms);
    std::cerr << "starting " << d << "-approximate search..." << std::endl;
    NTYPE new_est_cost = starting_est_cost;
    apx_stats.reset();
    double hr = 0;
    ApxTemporalRegState* apx_root =
      new ApxTemporalRegState(instance, lb, sd_atoms, 0, d);
    ApxResult apx_res;
    HashTable sol_tab(10007);
    IDAO apx_search(apx_stats, apx_res, &sol_tab);
    apx_search.set_cycle_check(false);
    apx_search.set_store_cost(true);
    // if (time_limit > 0) apx_stats.enable_time_out(time_limit, false);
    new_est_cost = apx_search.start(*apx_root);
    if (apx_search.solved() && apx_res.min_solution()) {
      std::cerr << "min-solution found!" << std::endl;
      done = true;
    }
    std::cerr << "finished in " << apx_stats.time() << " seconds"
	      << std::endl;
    delete apx_root;
  }

  if (opt_pddl) {
    lb.write_pddl(std::cerr, instance);
  }

  for (index_type d = opt_min_d; d <= opt_max_d; d++) {
    mSubsetEnumerator e(sd_atom.length(), d);
    bool more = e.first();
    while (more) {
      if (h.is_independent(e.current_set())) {
	index_set s_sd;
	index_set s_atms;
	NTYPE t_ld = 0;
	for (index_type i = 0; i < sd_atom.length(); i++)
	  if (e.current_set()[i]) {
	    s_sd.insert(i);
	    s_atms.insert(sd_atom[i]);
	    t_ld = MAX(t_ld, sd_time[i]);
	  }
	NTYPE t_min = lb.eval(s_atms);
	if (t_min > t_ld) {
	  std::cerr << "deadlines";
	  for (index_type i = 0; i < sd_atom.length(); i++)
	    if (e.current_set()[i]) {
	      std::cerr << " " << i << "."
			<< (sd_name[i] ? sd_name[i]->print_name : "?")
			<< ":" << instance.atoms[sd_atom[i]].name
			<< ":" << sd_time[i];
	    }
	  std::cerr << " CAN NOT ALL be met (t_ld = " << t_ld
		    << ", t_min = " << t_min << ")" << std::endl;
	  h.add_edge(s_sd);
	}
      }
      more = e.next();
    }
  }

  index_set_vec mcs;
  h.independent_sets(mcs);
  cost_vec set_value(0, mcs.length());
  cost_vec set_penalty(0, mcs.length());
  std::cerr << mcs.length() << " maximal consistent sets" << std::endl;
  for (index_type k = 0; k < mcs.length(); k++) {
    if (opt_verbose) std::cerr << "{";
    for (index_type i = 0; i < mcs[k].length(); i++) {
      if (opt_verbose) {
	if (i > 0) std::cerr << ", ";
	std::cerr << instance.atoms[sd_atom[mcs[k][i]]].name
		  << ":" << sd_time[mcs[k][i]];
      }
      set_value[k] += sd_val[mcs[k][i]];
    }
    set_penalty[k] = v_sum - set_value[k];
    if (opt_verbose) 
      std::cerr << "}: value = " << PRINT_NTYPE(set_value[k])
		<< ", penalty = " << PRINT_NTYPE(set_penalty[k])
		<< std::endl;
  }
  NTYPE v_max = cost_vec_util::max(set_value);
  NTYPE p_min = cost_vec_util::min(set_penalty);
  std::cerr << "max value = " << PRINT_NTYPE(v_max) << std::endl;
  std::cerr << "min penalty = " << PRINT_NTYPE(p_min) << std::endl;

  return 0;
}

END_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
