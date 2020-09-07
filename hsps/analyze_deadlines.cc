
#include "problem.h"
#include "exec.h"
#include "parser.h"
#include "cost_table.h"
#include "enumerators.h"
#include "temporal.h"
#include "idao.h"
#include "ida.h"
#include "plans.h"

#include <fstream>
#include <sstream>

#define SCORE_UNIT 10

BEGIN_HSPS_NAMESPACE

StringTable symbols(50, lowercase_map);
Parser* reader = new Parser(symbols);

void improve(const index_set s,
	     CostTable& h,
	     index_type h_m,
	     NTYPE ub,
	     index_type m_limit,
	     double time_limit,
	     Instance& instance,
	     CostTable& sols,
	     Statistics& stats)
{
  Statistics search_stats;
  if (time_limit > 0) search_stats.enable_time_out(time_limit, false);
  index_type m = h_m + 1;
  NTYPE est_cost = h.eval(s);
  NTYPE new_cost = est_cost;
  while (m <= m_limit) {
    State* apx_root = new ApxTemporalRegState(instance, h, s, 0, m);
    ApxResult res;
    HashTable solved_tab(10001);
    IDAO search(search_stats, res, &solved_tab);
    search.set_store_cost(true);
    std::cerr << m << "-relaxed search, root = " << *apx_root
	      << ", est. cost = " << apx_root->est_cost()
	      << " (cost bound = " << ub
	      << ", time limit = " << time_limit << ")"
	      << std::endl;
    new_cost = search.start(*apx_root);
    if (!search.solved()) {
      std::cerr << m << "-relaxed solution not found (search interrupted)"
		<< std::endl;
      stats.add(search_stats);
      return;
    }
    std::cerr << "new cost = " << new_cost << std::endl;
    if (new_cost > ub) {
      std::cerr << "new cost exceeds upper bound " << ub << std::endl;
      stats.add(search_stats);
      return;
    }
    if (res.min_solution()) {
      std::cerr << m << "-relaxed solution is min" << std::endl;
      NTYPE best_cost = sols.eval_min(s);
      if (new_cost < best_cost) {
	std::cerr << "storing optimal solution cost of ";
	instance.write_atom_set(std::cerr, s);
	std::cerr << " = " << PRINT_NTYPE(new_cost)
		  << " (prev. best = " << PRINT_NTYPE(best_cost) << ")"
		  << std::endl;
	sols.store(s, new_cost, true);
      }
      stats.add(search_stats);
      return;
    }
    est_cost = h.eval(s);
    m += 1;
  }
  std::cerr << "m-limit reached" << std::endl;
  if (new_cost <= est_cost) {
    std::cerr << "new cost == prev. est. cost, checking for solution..."
	      << std::endl;
    State* search_root = new TemporalRSRegState(instance, h, s, 0);
    Result result;
    HashTable* tt = new HashTable(31337);
    IDA search(search_stats, result, tt);
    search.set_iteration_limit(1);
    new_cost = search.start(*search_root);
    if (search.solved()) {
      NTYPE best_cost = sols.eval_min(s);
      if (new_cost < best_cost) {
	std::cerr << "storing optimal solution cost of ";
	instance.write_atom_set(std::cerr, s);
	std::cerr << " = " << PRINT_NTYPE(new_cost)
		  << " (prev. best = " << PRINT_NTYPE(best_cost) << ")"
		  << std::endl;
	sols.store(s, new_cost, true);
      }
    }
  }
  stats.add(search_stats);
}

int main(int argc, char *argv[]) {
  index_type max_goal_set_size = 2;
  index_type h_level = 1;
  index_type improve_m_limit = 3;
  double     improve_time_limit = 0;
  NTYPE      tolerance = 0;
  count_type generate_n = 0;
  NTYPE      density = R_TO_N(1, 4);
  bool       opt_solvable_only = true;
  bool       opt_write_graphs = false;
  bool       opt_mutex_edges = false;
  bool       opt_hard_deadlines = false;
  bool       opt_aw_constraints = true;
  bool       opt_filter_related = false;
  bool       opt_filter_non_zero = false;
  bool       opt_filter_non_init_phi = false;
  bool       opt_filter_non_goal_psi = false;
  char*      opt_filter_req_pred = 0;
  bool       opt_aw_max = false;
  index_type n_aw_max = 0;

  unsigned int random_seed = 0;

  PDDL_Base::write_warnings = false;
  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      Instance::default_trace_level = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-n") == 0) && (k < argc - 1)) {
      generate_n = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-d") == 0) && (k < argc - 1)) {
      density = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-possible") == 0) {
      opt_solvable_only = false;
    }
    else if (strcmp(argv[k],"-hard") == 0) {
      opt_hard_deadlines = true;
    }
    else if (strcmp(argv[k],"-aw-related") == 0) {
      opt_filter_related = true;
    }
    else if (strcmp(argv[k],"-aw-non-zero") == 0) {
      opt_filter_non_zero = true;
    }
    else if (strcmp(argv[k],"-aw-no-init") == 0) {
      opt_filter_non_init_phi = true;
    }
    else if (strcmp(argv[k],"-aw-no-goal") == 0) {
      opt_filter_non_goal_psi = true;
    }
    else if (strcmp(argv[k],"-no-aw") == 0) {
      opt_aw_constraints = false;
    }
    else if ((strcmp(argv[k],"-aw-max") == 0) && (k < argc - 1)) {
      opt_aw_max = true;
      n_aw_max = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-aw-predicate") == 0) && (k < argc - 1)) {
      opt_filter_req_pred = argv[++k];
    }
    else if ((strcmp(argv[k],"-m") == 0) && (k < argc - 1)) {
      max_goal_set_size = atoi(argv[++k]);
      improve_m_limit = max_goal_set_size + 1;
    }
    else if ((strcmp(argv[k],"-h") == 0) && (k < argc - 1)) {
      h_level = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-imp-m") == 0) && (k < argc - 1)) {
      improve_m_limit = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-imp-t") == 0) && (k < argc - 1)) {
      improve_time_limit = atof(argv[++k]);
    }
    else if ((strcmp(argv[k],"-tolerance") == 0) && (k < argc - 1)) {
      tolerance = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-g") == 0) {
      opt_write_graphs = true;
    }
    else if (strcmp(argv[k],"-u") == 0) {
      opt_mutex_edges = true;
    }
    else if (strcmp(argv[k],"-nsn") == 0) {
      Instance::write_atom_set_with_symbolic_names = false;
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

  LC_RNG rng;
  if (random_seed > 0) rng.seed(random_seed);

  reader->post_process();

  stats.start();
  reader->instantiate(instance);
  instance.cross_reference();
  stats.stop();
  std::cerr << "instantiation finished in " << stats.time() << " seconds"
	    << std::endl;

  CostTable lb(instance, stats);
  CostTable ub(instance, stats, POS_INF);

  if (h_level >= 2) {
    lb.compute_H2C(MakespanACF(instance), true);
  }
  else if (h_level == 1) {
    lb.compute_H1(MakespanACF(instance));
  }
  std::cerr << "heuristic computed in " << stats.time() << " seconds"
	    << std::endl;

  index_type n_goals = instance.goal_atoms.length();
  index_type n_plans = reader->n_plans();

  NTYPE reached[n_goals][n_plans];
  for (index_type g = 0; g < n_goals; g++)
    for (index_type p = 0; p < n_plans; p++)
      reached[g][p] = POS_INF;
  bool_vec accepted(false, n_plans);

  std::cerr << "analyzing " << n_plans << " plans..." << std::endl;
  stats.start();
  for (index_type k = 0; k < n_plans; k++) {
    Schedule plan(instance);
    reader->export_plan(k, instance, plan);

    ExecState cs(instance, instance.init_atoms);
    ExecErrorSet* errs = new ExecErrorSet();
    index_type s = 0;
    NTYPE t = 0;
    NTYPE c = 0;
    bool ok = true;
    while ((s < plan.plan_steps().length()) && ok) {
      NTYPE dt = plan.plan_steps()[s].at - t;
      Instance::Action& act = instance.actions[plan.plan_steps()[s].act];
      cs.advance(dt, errs);
      if (!cs.applicable(act, errs, s)) {
	std::cerr << "plan " << k << " failed at step " << s << std::endl;
	std::cerr << "state: ";
	cs.write(std::cerr);
	std::cerr << "errors: ";
	errs->write(std::cerr);
	std::cerr << std::endl;
	ok = false;
      }
      cs.apply(act, errs, s);
      t = plan.plan_steps()[s].at;
      c += act.cost;
      index_set ach;
      cs.check_atoms(instance.goal_atoms, ach);
      for (index_type i = 0; i < n_goals; i++)
	if (ach.contains(instance.goal_atoms[i])) {
	  reached[i][k] = MIN(t, reached[i][k]);
	}
      if (!ach.empty()) {
	NTYPE v_min = ub.eval_min(ach);
	std::cerr << "goals ";
	instance.write_atom_set(std::cerr, ach);
	std::cerr << " achieved at " << PRINT_NTYPE(t)
		  << " (known min = " << PRINT_NTYPE(v_min) << ")"
		  << std::endl;
	if (t < v_min) {
	  ub.store(ach, t);
	}
      }
      s += 1;
    }
    if (ok) {
      cs.finish(errs);
      index_set ach;
      cs.check_atoms(instance.goal_atoms, ach);
      for (index_type i = 0; i < n_goals; i++)
	if (ach.contains(instance.goal_atoms[i])) {
	  reached[i][k] = MIN(plan.makespan(), reached[i][k]);
	}
      if (!ach.empty()) {
	NTYPE v_min = ub.eval_min(ach);
	std::cerr << "goals ";
	instance.write_atom_set(std::cerr, ach);
	std::cerr << " achieved at " << PRINT_NTYPE(plan.makespan())
		  << " (known min = " << PRINT_NTYPE(v_min) << ")"
		  << std::endl;
	if (plan.makespan() < v_min) {
	  ub.store(ach, plan.makespan());
	}
	if (plan.plan_is_optimal()) {
	  NTYPE v_max = lb.eval(ach);
	  if (plan.makespan() > v_max) {
	    lb.store(ach, plan.makespan(), true);
	  }
	}
      }
      if (ach == instance.goal_atoms) {
	std::cerr << "plan " << k << " finished with all goals achieved"
		  << std::endl;
	accepted[k] = true;
      }
      else {
	accepted[k] = false;
      }
    }
    else {
      accepted[k] = false;
    }

    if (accepted[k]) {
      std::cerr << "reached[" << k << "] =";
      for (index_type i = 0; i < n_goals; i++)
	std::cerr << " " << reached[i][k];
      std::cerr << std::endl;
    }
    else {
      std::cerr << "plan " << k << " failed" << std::endl;
    }
  }
  stats.stop();
  std::cerr << "analysis finished in " << stats.time() << " seconds"
	    << std::endl;

  if (opt_hard_deadlines) {
    bool_matrix dominates(false, n_plans, n_plans);
    bool_vec dominated(false, n_plans);
    for (index_type i = 0; i < n_plans; i++) if (accepted[i]) {
      for (index_type j = 0; j < n_plans; j++) if ((i != j) && accepted[j]) {
	bool dom = true;
	for (index_type k = 0; (k < n_goals) && dom; k++)
	  if ((reached[k][j] + tolerance) < reached[k][i])
	    dom = false;
	dominates[i][j] = dom;
      }
    }

    for (index_type j = 0; j < n_plans; j++)
      for (index_type i = 0; i < n_plans; i++)
	if ((i != j) && dominates[i][j])
	  if (!dominates[j][i] || (j > i))
	    dominated[j] = true;

    bool_vec candidate(dominated);
    candidate.complement();
    candidate.intersect(accepted);

    std::cout << candidate.count(true) << " of " << n_plans
	      << " candidate plans (accepted & non-dominated)"
	      << std::endl;
    for (index_type k = 0; k < n_plans; k++) if (candidate[k]) {
      Schedule plan(instance);
      reader->export_plan(k, instance, plan);

      bool_matrix aw_c;
      cost_matrix aw_t;
      index_type  n_aw = 0;
      index_type  n_pass = 0;
      index_set   aw_sel;

      if (opt_aw_constraints) {
	std::cerr << "extracting additional constraints..." << std::endl;
	ExecTrace* trace = new ExecTrace(instance);
	bool ok = plan.simulate(trace);
	if (!ok) {
	  std::cerr << "program error: plan " << k
		    << " accepted but now failed!"
		    << std::endl;
	  exit(255);
	}
	trace->extract_always_within(aw_c, aw_t);
	for (index_type i = 0; i < instance.n_atoms(); i++)
	  for (index_type j = 0; j < instance.n_atoms(); j++)
	    if ((i != j) && aw_c[i][j]) {
	      bool pass = true;
	      if (opt_filter_non_init_phi) {
		if (instance.atoms[i].init) pass = false;
	      }
	      if (pass && opt_filter_non_goal_psi) {
		if (instance.atoms[j].goal) pass = false;
	      }
	      if (pass && opt_filter_non_zero) {
		if (aw_t[i][j] == 0) pass = false;
	      }
	      if (pass && (opt_filter_req_pred != 0)) {
		pass = false;
		PDDL_Name* n = (PDDL_Name*)instance.atoms[i].name->cast_to("PDDL_Name");
		if (n) {
		  if (strcmp(n->symbol()->print_name, opt_filter_req_pred) == 0)
		    pass = true;
		}
		if (!pass) {
		  PDDL_Name* n = (PDDL_Name*)instance.atoms[i].name->cast_to("PDDL_Name");
		  if (n) {
		    if (strcmp(n->symbol()->print_name, opt_filter_req_pred) == 0)
		      pass = true;
		  }
		}
	      }
	      if (pass && opt_filter_related) {
		pass = false;
		PDDL_Name* ni = (PDDL_Name*)instance.atoms[i].name->cast_to("PDDL_Name");
		PDDL_Name* nj = (PDDL_Name*)instance.atoms[j].name->cast_to("PDDL_Name");
		if (ni && nj) {
		  for (index_type ai = 0; ai < ni->argc(); ai++)
		    for (index_type aj = 0; aj < nj->argc(); aj++)
		      if (ni->args()[ai] == nj->args()[aj]) pass = true;
		}
	      }
	      if (!pass) {
		aw_c[i][j] = false;
	      }
	      else {
		n_pass += 1;
	      }
	      n_aw += 1;
	    }
	std::cerr << n_aw << " of "
		  << (instance.n_atoms() * (instance.n_atoms() - 1))
		  << " constraints (non-trivially) satisfied, "
		  << n_pass << " passed filter"
		  << std::endl;
	if (opt_aw_max && (n_pass > n_aw_max)) {
	  aw_sel.clear();
	  rng.select_fixed_set(aw_sel, n_aw_max, n_pass);
	  std::cerr << n_aw_max << " of " << n_pass
		    << " constraints selected"
		    << std::endl;
	}
      }

      std::cout << ";; plan " << k;
      if (plan.plan_name()) std::cout << " (" << plan.plan_name() << ")";
      std::cout << std::endl;

      std::cout << " (:constraints (and" << std::endl;
      for (index_type i = 0; i < n_goals; i++) {
	std::cout << "  (within "
#ifdef NTYPE_RATIONAL
		  << PRINT_NTYPE((reached[i][k] + tolerance).floor_to(tolerance.divisor()))
#else
		  << PRINT_NTYPE(FLOOR(reached[i][k] + tolerance))
#endif
		  << " " << instance.atoms[instance.goal_atoms[i]].name
		  << ")" << std::endl;
      }

      if (opt_aw_constraints) {
	index_type n = 0;
	for (index_type i = 0; i < instance.n_atoms(); i++)
	  for (index_type j = 0; j < instance.n_atoms(); j++)
	    if ((i != j) && aw_c[i][j]) {
	      bool sel = true;
	      if (opt_aw_max && (n_pass > n_aw_max)) {
		if (!aw_sel.contains(n)) sel = false;
	      }
	      if (sel) {
		std::cout << "  (always-within "
#ifdef NTYPE_RATIONAL
			  << PRINT_NTYPE((aw_t[i][j] + tolerance).floor_to(tolerance.divisor()))
#else
			  << PRINT_NTYPE(FLOOR(aw_t[i][j] + tolerance))
#endif
			  << " " << instance.atoms[i].name
			  << " " << instance.atoms[j].name
			  << ")" << std::endl;
	      }
	      n += 1;
	    }
	std::cout << ";; " << n_aw << " of "
		  << (instance.n_atoms() * (instance.n_atoms() - 1))
		  << " constraints (non-trivially) satisfied, "
		  << n_pass << " passed filter";
	if (opt_aw_max && (n_pass > n_aw_max)) {
	  std::cout << ", " << n_aw_max << " randomly selected";
	}
	std::cout << std::endl;
      }
      std::cout << "))" << std::endl;

    } // for each candidate plan ...
    exit(0);
  }

  for (index_type m = 1; m <= max_goal_set_size; m++) {
    mSubsetEnumerator se(instance.goal_atoms.length(), m);
    bool more = se.first();
    while (more) {
      index_set s;
      se.current_set(instance.goal_atoms, s);
      NTYPE ubs = ub.eval_min(s);
      for (index_type k = 0; k < instance.goal_atoms.length(); k++)
	if (!s.contains(instance.goal_atoms[k])) {
	  index_set ss(s);
	  ss.insert(instance.goal_atoms[k]);
	  NTYPE lbss = lb.eval(ss);
	  if (lbss < ubs) {
	    improve(ss, lb, h_level,
		    ubs, improve_m_limit, improve_time_limit,
		    instance, ub, stats);
	    lbss = lb.eval(ss);
	  }
	}
      more = se.next();
    }
  }

//   std::cerr << "upper bounds table:" << std::endl;
//   ub.write(std::cerr);
//   std::cerr << "lower bounds table:" << std::endl;
//   lb.write(std::cerr);

  std::cout << "goal set bounds:" << std::endl;
  cost_set breakpoints;
  SubsetEnumerator se(instance.goal_atoms.length());
  bool more = se.first();
  while (more) {
    index_set s;
    se.current_set(instance.goal_atoms, s);
    if (!s.empty()) {
      NTYPE lbs = lb.eval(s);
      NTYPE ubs = ub.eval_min(s);
      if (FINITE(lbs)) breakpoints.insert(lbs);
      if (FINITE(ubs)) breakpoints.insert(ubs);
      NTYPE lbx = POS_INF;
      NTYPE ubx = 0;
      for (index_type k = 0; k < instance.goal_atoms.length(); k++)
	if (!s.contains(instance.goal_atoms[k])) {
	  index_set ss(s);
	  ss.insert(instance.goal_atoms[k]);
	  NTYPE lbss = lb.eval(ss);
	  lbx = MIN(lbx, lbss);
	  ubx = MAX(ubx, lbss);
	}
      instance.write_atom_set(std::cout, s);
      std::cout << ": [" << PRINT_NTYPE(lbs)
		<< ", " << PRINT_NTYPE(ubs)
		<< "] / " << PRINT_NTYPE(lbx)
		<< " / " << PRINT_NTYPE(ubx)
		<< std::endl;
    }
    more = se.next();
  }

  index_set_vec sets;
  index_type max_set = no_such_index;
  cost_vec p0(POS_INF, 0);
  cost_vec p1(POS_INF, 0);
  bool_vec r0;
  bool_vec r1;

  more = se.first();
  while (more) {
    index_set s;
    se.current_set(instance.goal_atoms, s);
    if (!s.empty()) {
      sets.append(s);
      if (s.length() == instance.goal_atoms.length())
	max_set = sets.length() - 1;
      p0[sets.length() - 1] = lb.eval(s);
      p1[sets.length() - 1] = ub.eval_min(s);
    }
    more = se.next();
  }

  r0.assign_value(true, sets.length());
  r1.assign_value(true, sets.length());
  for (index_type k = 0; k < sets.length(); k++) {
    for (index_type i = 0; i < sets.length(); i++)
      if ((i != k) && sets[i].contains(sets[k])) {
	if (p0[i] <= p0[k]) r0[k] = false;
	if (p1[i] <= p1[k]) r1[k] = false;
      }
    if (p1[k] <= (p0[k] + tolerance)) r0[k] = false;
  }

  breakpoints.clear();
  for (index_type k = 0; k < sets.length(); k++) {
    if (r0[k]) breakpoints.insert(p0[k]);
    if (r1[k]) breakpoints.insert(p1[k]);
  }

  std::cout << "breakpoints:" << std::endl;
  for (index_type k = 0; k < breakpoints.length(); k++) {
    std::cout << "at " << PRINT_NTYPE(breakpoints[k]) << std::endl;
    for (index_type i = 0; i < sets.length(); i++) {
      if (r1[i] && (p1[i] == breakpoints[k])) {
	std::cout << " - ";
	instance.write_atom_set(std::cout, sets[i]);
	std::cout << " becomes CERTAIN"
		  << std::endl;
      }
      if (r0[i] && (p0[i] == breakpoints[k])) {
	std::cout << " - ";
	instance.write_atom_set(std::cout, sets[i]);
	std::cout << " becomes POSSIBLE"
		  << std::endl;
      }
    }
  }

  if (opt_write_graphs) {
    std::ostringstream fname;
    if (reader->problem_file) {
      char* p0 = strdup(reader->problem_file);
      char* p1 = strrchr(p0, '.');
      if (p1) *p1 = '\0';
      p1 = strrchr(p0, '/');
      if (p1) p0 = p1 + 1;
      fname << p0 << "-timeline.dot";
    }
    else {
      fname << "timeline.dot";
    }

    std::cerr << "writing timeline graph to " << fname.str()
	      << "..." << std::endl;

    std::ofstream g_out(fname.str().c_str());
    g_out << "digraph Timeline {" << std::endl
	  << "rankdir=LR;" << std::endl;
    // g_out << "ranksep=0.5;" << std::endl;
    g_out << "node [shape=box,width=0,height=0];" << std::endl;

    for (index_type k = 0; k < breakpoints.length(); k++) {
      g_out << "{ rank = same;" << std::endl;
      g_out << "T" << k << " [shape=plaintext,label=\""
	    << PRINT_NTYPE(breakpoints[k]) << "\"];" << std::endl;
      for (index_type i = 0; i < sets.length(); i++) {
	if (r1[i] && (p1[i] == breakpoints[k])) {
	  g_out << "A" << i << " [label=\"";
	  for (index_type j = 0; j < sets[i].length(); j++) {
	    if (j > 0) g_out << "\\n";
	    g_out << instance.atoms[sets[i][j]].name;
	  }
	  g_out << "\"];" << std::endl;
	}
	if (r0[i] && (p0[i] == breakpoints[k])) {
	  g_out << "B" << i << " [style=dashed,label=\"";
	  for (index_type j = 0; j < sets[i].length(); j++) {
	    if (j > 0) g_out << "\\n";
	    g_out << instance.atoms[sets[i][j]].name;
	  }
	  g_out << "\"];" << std::endl;
	}
      }
      g_out << "}" << std::endl;
      if (k > 0) {
	g_out << "T" << k - 1 << " -> T" << k << " [style=invis];"
	      << std::endl;
      }
    }
    for (index_type i = 0; i < sets.length(); i++) {
      for (index_type j = 0; j < sets.length(); j++)
	if ((i != j) && sets[j].contains(sets[i])) {
	  if (r0[i] && r0[j]) {
	    g_out << "B" << i << " -> B" << j << ";" << std::endl;
	  }
	  if (r1[i] && r1[j]) {
	    g_out << "A" << i << " -> A" << j << ";" << std::endl;
	  }
	  if (r0[i] && !r1[i] && !r0[j] && r1[j]) {
	    g_out << "B" << i << " -> A" << j << ";" << std::endl;
	  }
	}
      if (r0[i] && r1[i]) {
	g_out << "B" << i << " -> A" << i << ";" << std::endl;
      }
    }
    g_out << "}" << std::endl; // end of Graph { ...
    g_out.close();
  }

  while (generate_n > 0) {
    index_set s0;
    index_set s1;
    r1.copy_to(s1);
    s1.subtract(max_set);
    index_type n_to_choose_from = s1.length();
    if (!opt_solvable_only) {
      r0.copy_to(s0);
      s0.subtract(max_set);
      n_to_choose_from += s0.length();
    }
    index_type c = FLOOR_TO_INT(n_to_choose_from * density);
    if (c < 1) c = 1;
    index_cost_vec deadlines;
    index_set s;
    rng.select_non_empty_variable_set(s, c, n_to_choose_from);
    for (index_type k = 0; k < s.length(); k++) {
      index_type p = (s[k] < s1.length() ? s1[s[k]] : s0[s[k] - s1.length()]);
      NTYPE t = (s[k] < s1.length() ? p1[p] : p0[p]);
      index_type i = breakpoints.first(t);
      assert(i < breakpoints.length());
      index_type j = i + 1;
      bool done = (j >= breakpoints.length());
      while (!done) {
	if ((breakpoints[j] - breakpoints[i]) > tolerance) {
	  done = true;
	}
	else {
	  j += 1;
	  if (j >= breakpoints.length()) done = true;
	}
      }
      if (j < breakpoints.length()) {
	t = (breakpoints[i] + breakpoints[j]) / 2;
      }
      else {
	t = (breakpoints[i] + tolerance);
      }
      for (index_type l = 0; l < sets[p].length(); l++)
	if (deadlines.first(index_cost_pair(sets[p][l], t)) == no_such_index)
	  deadlines.append(index_cost_pair(sets[p][l], t));
    }

    cost_vec score;
    score.assign_value(1, deadlines.length());
    for (index_type k = 0; k < deadlines.length(); k++) {
      for (index_type i = 0; i < deadlines.length(); i++) {
	if (deadlines[i].first != deadlines[k].first) {
	  index_set c;
	  c.insert(deadlines[i].first);
	  c.insert(deadlines[k].first);
	  if (lb.eval(c) > MAX(deadlines[i].second, deadlines[k].second))
	    score[k] += 1;
	}
      }
    }

    cost_vec rnd_score;
    rnd_score.assign_value(0, deadlines.length());
    for (index_type k = 0; k < deadlines.length(); k++) {
      rnd_score[k] = (SCORE_UNIT *
		      score[k] *
		      R_TO_N(rng.random_in_range(100) + 50, 100));
    }

    std::cout << "suggested deadlines:" << std::endl;
    for (index_type k = 0; k < deadlines.length(); k++) {
      std::cout << "[" << PRINT_NTYPE(deadlines[k].second) << "] "
		<< instance.atoms[deadlines[k].first].name
		<< ", score = " << score[k]
		<< " (" << rnd_score[k] << ")"
		<< std::endl;
    }

    // print suggested deadlines in PDDL3 syntax
    std::cout << " (:constraints (and";
    for (index_type k = 0; k < deadlines.length(); k++)
      std::cout << std::endl
		<< "   (preference d" << k + 1 << " (within "
		<< PRINT_NTYPE(deadlines[k].second)
		<< " "
		<< instance.atoms[deadlines[k].first].name
		<< "))";
    std::cout << "))" << std::endl;
    std::cout << " (:metric maximize";
    for (index_type k = 0; k < deadlines.length() - 1; k++)
      std::cout << " (+ (* " << PRINT_NTYPE(rnd_score[k])
		<< " (- 1 (is-violated d" << k + 1
		<< ")))";
    std::cout << " (* " << PRINT_NTYPE(rnd_score[deadlines.length() - 1])
	      << " (- 1 (is-violated d" << deadlines.length()
	      << ")))";
    for (index_type k = 0; k < deadlines.length() - 1; k++)
      std::cout << ")";
    std::cout << ")" << std::endl;

    generate_n -= 1;
  }

  std::cout << stats << std::endl;
}

END_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
