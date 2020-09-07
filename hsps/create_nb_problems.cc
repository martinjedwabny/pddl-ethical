
#include "problem.h"
#include "exec.h"
#include "parser.h"

#include <fstream>
#include <sstream>

BEGIN_HSPS_NAMESPACE

StringTable symbols(50, lowercase_map);
Parser* reader = new Parser(symbols);

int main(int argc, char *argv[]) {
  count_type generate_n = 0;
  bool       opt_interference_only = false;
  bool       opt_synergy_only = false;
  bool       opt_both_only = false;
  index_type opt_min_goals = 1;
  bool       opt_write_graphs = false;

  unsigned int random_seed = 0;

  PDDL_Base::write_warnings = false;
  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      Instance::default_trace_level = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-n") == 0) && (k < argc - 1)) {
      generate_n = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-min-goals") == 0) && (k < argc - 1)) {
      opt_min_goals = atoi(argv[++k]);
    }
    else if (strcmp(argv[k],"-interference-only") == 0) {
      opt_interference_only = true;
    }
    else if (strcmp(argv[k],"-synergy-only") == 0) {
      opt_synergy_only = true;
    }
    else if (strcmp(argv[k],"-synint-only") == 0) {
      opt_both_only = true;
    }
    else if (strcmp(argv[k],"-g") == 0) {
      opt_write_graphs = true;
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

  reader->post_process();

  stats.start();
  reader->instantiate(instance);
  instance.cross_reference();
  stats.stop();
  std::cerr << "instantiation finished in " << stats.time() << " seconds"
	    << std::endl;

  std::cerr << "analyzing " << reader->n_plans() << " plans..." << std::endl;
  index_vec goal_map;
  mapping::invert_map(instance.goal_atoms, goal_map);
  cost_matrix joint_cost(POS_INF,
			 instance.goal_atoms.length(),
			 instance.goal_atoms.length());
  for (index_type k = 0; k < reader->n_plans(); k++) {
    Schedule* plan = new Schedule(instance);
    reader->export_plan(k, instance, *plan);
    index_set atoms;
    bool ok = plan->simulate(atoms);
    if (ok) {
      atoms.intersect(instance.goal_atoms);
      std::cerr << "plan " << k << " achieves ";
      instance.write_atom_set(std::cerr, atoms);
      std::cerr << " at cost " << plan->cost() << std::endl;
      for (index_type i = 0; i < atoms.length(); i++)
	for (index_type j = 0; j < atoms.length(); j++) {
	  assert(goal_map[atoms[i]] != no_such_index);
	  assert(goal_map[atoms[j]] != no_such_index);
	  joint_cost[goal_map[atoms[i]]][goal_map[atoms[j]]] =
	    MIN(joint_cost[goal_map[atoms[i]]][goal_map[atoms[j]]],
		plan->cost());
	}
    }
    else {
      std::cerr << "plan " << k << " is not executable" << std::endl;
    }
    delete plan;
  }

  index_vec n_syn(0, instance.goal_atoms.length());
  cost_vec sum_syn(0, instance.goal_atoms.length());
  index_vec n_int(0, instance.goal_atoms.length());
  cost_vec sum_int(0, instance.goal_atoms.length());

  index_type n_goals_with_syn = 0;
  index_type n_goals_with_int = 0;
  index_type n_goals_with_both = 0;

  for (index_type i = 0; i < instance.goal_atoms.length(); i++) {
    std::cerr << instance.atoms[instance.goal_atoms[i]].name
	      << ": " << joint_cost[i][i] << std::endl;
    for (index_type j = 0; j < instance.goal_atoms.length(); j++)
      if (i != j) {
	if (joint_cost[i][j] > (joint_cost[i][i] + joint_cost[j][j])) {
	  std::cerr << " & " << instance.atoms[instance.goal_atoms[j]].name
		    << ": +" << (joint_cost[i][j] -
				 (joint_cost[i][i] + joint_cost[j][j]))
		    << std::endl;
	  n_int[i] += 1;
	  sum_int[i] +=
	    (joint_cost[i][j] - (joint_cost[i][i] + joint_cost[j][j]));
	}
	else if (joint_cost[i][j] < (joint_cost[i][i] + joint_cost[j][j])) {
	  std::cerr << " & " << instance.atoms[instance.goal_atoms[j]].name
		    << ": -" << ((joint_cost[i][i] + joint_cost[j][j]) -
				 joint_cost[i][j])
		    << std::endl;
	  n_syn[i] += 1;
	  sum_syn[i] +=
	    ((joint_cost[i][i] + joint_cost[j][j]) - joint_cost[i][j]);
	}
      }
    std::cerr << " total/average synergy: "
	      << sum_syn[i] << " / " << sum_syn[i]/n_syn[i]
	      << std::endl;
    std::cerr << " total/average interference: "
	      << sum_int[i] << " / " << sum_int[i]/n_int[i]
	      << std::endl;
    if (n_syn[i] > 0) n_goals_with_syn += 1;
    if (n_int[i] > 0) n_goals_with_int += 1;
    if ((n_syn[i] > 0) && (n_int[i] > 0)) n_goals_with_both += 1;
  }
  std::cerr << n_goals_with_syn << " goals in synergy relations"
	    << std::endl
	    << n_goals_with_int << " goals in interference relations"
	    << std::endl
	    << n_goals_with_both
	    << " goals in synergy and interference relations"
	    << std::endl;

  if (opt_write_graphs) {
    std::ofstream g_out("gcg.dot");
    g_out << "graph GoalCostGraph {" << std::endl
	  << "overlap=false;" << std::endl
	  << "splines=true;" << std::endl
	  << "sep=0.1;" << std::endl
	  << "node [width=0,height=0];" << std::endl;
    for (index_type i = 0; i < instance.goal_atoms.length(); i++) {
      g_out << "G" << i << " [label=\""
	    << instance.atoms[instance.goal_atoms[i]].name
	    << ": " << PRINT_NTYPE(joint_cost[i][i])
	    << "\"];" << std::endl;
    }
    for (index_type i = 0; i < instance.goal_atoms.length(); i++) {
      for (index_type j = i + 1; j < instance.goal_atoms.length(); j++) {
	if (joint_cost[i][j] > (joint_cost[i][i] + joint_cost[j][j])) {
	  g_out << "G" << i << " -- G" << j
		<< " [label=\"" << PRINT_NTYPE(joint_cost[i][j])
		<< " (+ "
		<< PRINT_NTYPE(joint_cost[i][j] -
			       (joint_cost[i][i] + joint_cost[j][j]))
		<< ")\"];" << std::endl;
	}
	else if (joint_cost[i][j] < (joint_cost[i][i] + joint_cost[j][j])) {
	  g_out << "G" << i << " -- G" << j
		<< " [label=\"" << PRINT_NTYPE(joint_cost[i][j])
		<< " (- "
		<< PRINT_NTYPE((joint_cost[i][i] + joint_cost[j][j]) -
			       joint_cost[i][j])
		<< ")\",style=dashed];" << std::endl;
	}
      }
    }
    g_out << "}" << std::endl;
    g_out.close();
  }

  LC_RNG rng;
  if (random_seed > 0) rng.seed(random_seed);
  index_set selected_goals;

  if (opt_interference_only) {
    graph g_syn(instance.goal_atoms.length());
    for (index_type i = 0; i < instance.goal_atoms.length(); i++)
      for (index_type j = 0; j < instance.goal_atoms.length(); j++)
	if ((i != j) && (joint_cost[i][j] < (joint_cost[i][i] + joint_cost[j][j])))
	  g_syn.add_undirected_edge(i, j);
    g_syn.apx_independent_set(selected_goals);
  }
  else if (opt_synergy_only) {
    graph g_int(instance.goal_atoms.length());
    for (index_type i = 0; i < instance.goal_atoms.length(); i++)
      for (index_type j = 0; j < instance.goal_atoms.length(); j++)
	if ((i != j) && (joint_cost[i][j] > (joint_cost[i][i] + joint_cost[j][j])))
	  g_int.add_undirected_edge(i, j);
    g_int.apx_independent_set(selected_goals);
  }
  else if (opt_both_only) {
    for (index_type i = 0; i < instance.goal_atoms.length(); i++)
      if ((n_syn[i] > 0) && (n_int[i] > 0)) selected_goals.insert(i);
  }
  else {
    selected_goals.fill(instance.goal_atoms.length());
  }

  index_set unreachable_goals;
  for (index_type i = 0; i < selected_goals.length(); i++)
    if (INFINITE(joint_cost[selected_goals[i]][selected_goals[i]]))
      unreachable_goals.insert(selected_goals[i]);
  std::cerr << unreachable_goals.length() << " unreachable goals:";
  for (index_type i = 0; i < instance.goal_atoms.length(); i++) {
    if (unreachable_goals.contains(i))
      std::cerr << " " << instance.atoms[instance.goal_atoms[i]].name;
  }
  std::cerr << std::endl;
  selected_goals.subtract(unreachable_goals);

  std::cerr << selected_goals.length() << " selected goals:";
  for (index_type i = 0; i < instance.goal_atoms.length(); i++) {
    if (selected_goals.contains(i))
      std::cerr << " " << instance.atoms[instance.goal_atoms[i]].name;
  }
  std::cerr << std::endl;

  if (selected_goals.length() < opt_min_goals) {
    std::cerr << "only " << selected_goals.length()
	      << " goals selected, " << opt_min_goals
	      << " required" << std::endl;
  }
  else {
    // recompute synergy/intereference based on selected goals only
    n_syn.assign_value(0, selected_goals.length());
    sum_syn.assign_value(0, selected_goals.length());
    n_int.assign_value(0, selected_goals.length());
    sum_int.assign_value(0, selected_goals.length());

    for (index_type i = 0; i < selected_goals.length(); i++)
      for (index_type j = 0; j < selected_goals.length(); j++)
	if (i != j) {
	  NTYPE c_joint = joint_cost[selected_goals[i]][selected_goals[j]];
	  NTYPE c_sum = (joint_cost[selected_goals[i]][selected_goals[i]] +
			 joint_cost[selected_goals[j]][selected_goals[j]]);
	  if (c_joint > c_sum) {
	    n_int[i] += 1;
	    sum_int[i] += (c_joint - c_sum);
	  }
	  else if (c_joint < c_sum) {
	    n_syn[i] += 1;
	    sum_syn[i] += (c_sum - c_joint);
	  }
	}
  }

  while ((selected_goals.length() >= opt_min_goals) && (generate_n > 0)) {
    cost_matrix goal_value(0, selected_goals.length(),
			   selected_goals.length());
    for (index_type i = 0; i < selected_goals.length(); i++) {
      NTYPE c_unit = joint_cost[selected_goals[i]][selected_goals[i]];
      if (n_syn[i] > 0) {
	goal_value[i][i] =
	  ROUND_TO(c_unit - (R_TO_N(rng.random_in_range(51), 100) *
			     (sum_syn[i]/n_syn[i])), 10);
      }
      else if (n_int[i] > 0) {
	goal_value[i][i] =
	  ROUND_TO(c_unit + (R_TO_N(rng.random_in_range(51), 100) *
			     (sum_int[i]/n_int[i])), 10);
      }
      else {
	goal_value[i][i] =
	  ROUND_TO(R_TO_N(rng.random_in_range(201), 100) * c_unit, 10);
      }
    }

    for (index_type i = 0; i < selected_goals.length(); i++)
      if ((n_syn[i] > 0) && (n_int[i] > 0)) {
	for (index_type j = i+1; j < selected_goals.length(); j++) {
	  NTYPE v = (goal_value[i][i] + goal_value[j][j]);
	  if (joint_cost[selected_goals[i]][selected_goals[j]] >
	      (joint_cost[selected_goals[i]][selected_goals[i]] +
	       joint_cost[selected_goals[j]][selected_goals[j]])) {
	    if ((v - joint_cost[selected_goals[i]][selected_goals[j]]) < 0) {
	      goal_value[i][j] =
		ROUND_TO(R_TO_N(rng.random_in_range(201), 100) *
			 joint_cost[selected_goals[i]][selected_goals[j]] - v,
			 10);
	    }
	  }
	}
      }

    NTYPE min_value = 0;
    for (index_type i = 0; i < selected_goals.length(); i++) {
      min_value = MAX(goal_value[i][i] - joint_cost[selected_goals[i]][selected_goals[i]], min_value);
      for (index_type j = i + 1; j < selected_goals.length(); j++) {
	min_value = MAX((goal_value[i][i] +
			 goal_value[j][j] +
			 goal_value[i][j]) -
			joint_cost[selected_goals[i]][selected_goals[j]],
			min_value);
      }
    }

    std::cerr << "variant " << generate_n << " has a minimum value of "
	      << min_value << std::endl;

    if (min_value > 0) {
      std::ostringstream fname;
      if (reader->problem_file) {
	char* p0 = strdup(reader->problem_file);
	char* p1 = strrchr(p0, '.');
	if (p1) *p1 = '\0';
	p1 = strrchr(p0, '/');
	if (p1) p0 = p1 + 1;
	fname << p0 << "-v" << generate_n << ".pddl";
      }
      else {
	fname << "variant" << generate_n << ".pddl";
      }
      std::cerr << "writing variant " << generate_n << " to " << fname.str()
		<< "..." << std::endl;

      cost_vec pref(0, 0);

      std::ofstream p_out(fname.str().c_str());
      p_out << "(define (problem "
	    << reader->problem_name << "-" << generate_n << ")"
	    << " ;; min value = " << min_value
	    << std::endl;
      p_out << " (:domain " << reader->domain_name << ")" << std::endl;
      reader->write_objects(p_out, true);
      reader->write_init(p_out);
      p_out << " (:goal (and";
      for (index_type i = 0; i < selected_goals.length(); i++) {
	p_out << " (preference g" << pref.length() << " "
	      << instance.atoms[instance.goal_atoms[selected_goals[i]]].name
	      << ")";
	pref.append(goal_value[i][i]);
	for (index_type j = i + 1; j < selected_goals.length(); j++)
	  if (goal_value[i][j] > 0) {
	    p_out << " (preference g" << pref.length() << " (and "
		  << instance.atoms[instance.goal_atoms[selected_goals[i]]].name
		  << " "
		  << instance.atoms[instance.goal_atoms[selected_goals[j]]].name
		  << "))";
	    pref.append(goal_value[i][j]);
	  }
      }
      p_out << "))" << std::endl;
      p_out << " (:metric maximize (-";
      for (index_type k = 0; k < pref.length(); k++) {
	if (k + 1 < pref.length()) p_out << " (+";
	p_out << " (* " << PRINT_NTYPE(pref[k])
	      << " (- 1 (is-violated g" << k << ")))";
      }
      for (index_type k = 1; k < pref.length(); k++) p_out << ")";
      reader->metric->print(p_out, false);
      p_out << "))" << std::endl;
      p_out << ")" << std::endl;
      p_out.close();
    }

    generate_n -= 1;
  }
}

END_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
