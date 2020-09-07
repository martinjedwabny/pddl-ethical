
#include "parser.h"
#include "preprocess.h"
#include "sas.h"
#include "plans.h"

HSPS::Statistics  stats;
HSPS::SASInstance instance;

HSPS::index_vec gprv;
HSPS::index_set** a_pre;
HSPS::index_set** a_post;

BEGIN_HSPS_NAMESPACE

void achieve_prevail
(const bool_vec& prv_vars,
 partial_state& o_state,
 index_vec& o_plan)
{
  bool_vec todo(prv_vars);
  o_state = instance.init_state;
  o_plan.set_length(0);
  bool changed = true;
  while (changed) {
    changed = false;
    for (index_type k = 0; k<instance.n_variables(); k++) if (todo[k]) {
      index_type i_val = instance.init_state.value_of(k);
      assert(i_val != no_such_index);
      lvector< std::pair<index_type, index_pair> > reached;
      reached.append(std::pair<index_type, index_pair>
		     (i_val, index_pair(no_such_index, no_such_index)));
      index_type p = 0;
      bool hit = false;
      while ((p < reached.length()) && !hit) {
	index_type v0 = reached[p].first;
	for (index_type i = 0; (i < a_pre[k][v0].length()) && !hit; i++)
	  if (o_state.implies(instance.actions[a_pre[k][v0][i]].prv)) {
	    index_type v1 =
	      instance.actions[a_pre[k][v0][i]].post.value_of(k);
	    if (v1 == gprv[k]) {
	      hit = true;
	      index_type l = o_plan.length();
	      o_plan.insert(a_pre[k][v0][i], l);
	      while (p != no_such_index) {
		if (reached[p].second.second != no_such_index)
		  o_plan.insert(reached[p].second.second, l);
		p = reached[p].second.first;
	      }
	    }
	    else {
	      reached.append(std::pair<index_type, index_pair>
			     (v1, index_pair(p, a_pre[k][v0][i])));
	    }
	  }
      }
      if (hit) {
	o_state.assign(k, gprv[k]);
	todo[k] = false;
	changed = true;
      }
    }
  }
}


void achieve_goal
(const partial_state& i_state,
 partial_state& o_state,
 index_vec& o_plan)
{
  o_state = i_state;
  o_state.assign(instance.goal_state);
  o_plan.set_length(0);
  bool_vec todo(true, instance.goal_state.length());
  bool changed = true;
  while (changed) {
    changed = false;
    for (index_type k = 0; k < instance.goal_state.length(); k++) if (todo[k]) {
      index_type g_var = instance.goal_state[k].first;
      index_type g_val = instance.goal_state[k].second;
      lvector< std::pair<index_type, index_pair> > reached;
      reached.append(std::pair<index_type, index_pair>
		     (g_val, index_pair(no_such_index, no_such_index)));
      index_type p = 0;
      bool hit = false;
      while ((p < reached.length()) && !hit) {
	index_type v1 = reached[p].first;
	for (index_type i = 0; (i < a_post[g_var][v1].length()) && !hit; i++)
	  if (o_state.implies(instance.actions[a_post[g_var][v1][i]].prv)) {
	    index_type v0 =
	      instance.actions[a_post[g_var][v1][i]].pre.value_of(k);
	    if (v0 == i_state.value_of(g_var)) {
	      hit = true;
	      index_type l = 0;
	      o_plan.insert(a_post[g_var][v1][i], l++);
	      while (p != no_such_index) {
		if (reached[p].second.second != no_such_index)
		  o_plan.insert(reached[p].second.second, l++);
		p = reached[p].second.first;
	      }
	    }
	    else {
	      reached.append(std::pair<index_type, index_pair>
			     (v0, index_pair(p, a_post[g_var][v1][i])));
	    }
	  }
      }
      if (hit) {
	o_state.assign(g_var, i_state.value_of(g_var));
	todo[k] = false;
	changed = true;
      }
    }
  }
}

END_HSPS_NAMESPACE

int main(int argc, char *argv[]) {
  bool        opt_sas_min = false;
  bool        opt_sas_safe = false;
  bool        opt_sas_select = false;
  bool        opt_preprocess = true;
  bool        opt_preprocess_2 = true;
  bool        opt_rm_irrelevant = false;
  bool        opt_find_invariants = true;
  bool        opt_quick_find_invariants = false;
  bool        opt_verify_invariants = true;
  bool        opt_print_plan = true;
  bool        opt_schedule = false;
  bool        opt_pddl = false;
  double      time_limit = 0;
  long        memory_limit = 0;
  int         verbose_level = 1;

  HSPS::StringTable symbols(50, HSPS::lowercase_map);
  HSPS::Parser* reader = new HSPS::Parser(symbols);

  for (int k = 1; k < argc; k++) {
    // verbose level
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      verbose_level = atoi(argv[++k]);
    }

    else if (strcmp(argv[k],"-no-compile") == 0) {
      HSPS::PDDL_Base::create_all_atoms = true;
      HSPS::PDDL_Base::compile_away_disjunctive_preconditions = false;
      HSPS::PDDL_Base::compile_away_conditional_effects = false;
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
    else if (strcmp(argv[k],"-remove") == 0) {
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

    // limit-setting options
    else if ((strcmp(argv[k],"-t") == 0) && (k < argc - 1)) {
      time_limit = atof(argv[++k]);
    }
    else if ((strcmp(argv[k],"-y") == 0) && (k < argc - 1)) {
      memory_limit = atoi(argv[++k]);
    }

    // output/formatting options
    else if (strcmp(argv[k],"-schedule") == 0) {
      opt_schedule = true;
    }
    else if (strcmp(argv[k],"-no-plan") == 0) {
      opt_print_plan = false;
    }
    else if (strcmp(argv[k],"-pddl") == 0) {
      opt_print_plan = true;
      opt_pddl = true;
    }
    else if (strcmp(argv[k],"-ipc") == 0) {
      opt_print_plan = true;
      opt_pddl = false;
    }
    else if (strcmp(argv[k],"-nsn") == 0) {
      HSPS::Instance::write_atom_set_with_symbolic_names = false;
      HSPS::Instance::write_action_set_with_symbolic_names = false;
    }

    // misc. options
    else if (strcmp(argv[k],"-sas-select") == 0) {
      opt_sas_select = true;
    }
    else if (strcmp(argv[k],"-sas-min") == 0) {
      opt_sas_min = true;
    }
    else if (strcmp(argv[k],"-sas-safe") == 0) {
      opt_sas_safe = true;
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
      reader->read(argv[k], false);
    }
  }

  HSPS::SearchAlgorithm::default_trace_level = verbose_level;
  HSPS::Heuristic::default_trace_level = verbose_level;
  HSPS::Instance::default_trace_level = verbose_level - 1;
  HSPS::Preprocessor::default_trace_level = verbose_level - 1;
  if (verbose_level < 1) opt_print_plan = false;
  if (verbose_level <= 0) HSPS::PDDL_Base::write_warnings = false;
  if (verbose_level > 1) HSPS::PDDL_Base::write_info = true;

  HSPS::Instance    p_instance;
  HSPS::Preprocessor prep(p_instance, stats);
  HSPS::Store       store(p_instance);

  stats.enable_interrupt(false);
  if (time_limit > 0) stats.enable_time_out(time_limit, false);
  if (memory_limit > 0) stats.enable_memory_limit(memory_limit, false);

  stats.start();
  std::cerr << "instantiating..." << std::endl;
  reader->instantiate(p_instance);
  if (opt_preprocess) {
    std::cerr << "preprocessing..." << std::endl;
    prep.preprocess(opt_preprocess_2 && !opt_rm_irrelevant);
    if (opt_rm_irrelevant) {
      prep.compute_irrelevant_atoms();
      prep.remove_irrelevant_atoms();
      if (opt_preprocess_2)
	prep.preprocess(true);
    }
    if (!p_instance.cross_referenced()) {
      std::cerr << "re-cross referencing..." << std::endl;
      p_instance.cross_reference();
    }
  }
  else {
    std::cerr << "cross referencing..." << std::endl;
    p_instance.cross_reference();
  }
  if (opt_quick_find_invariants) {
    HSPS::graph* g_inc = prep.inconsistency_graph();
    prep.find_inconsistent_set_invariants(*g_inc);
    p_instance.add_missing_negation_invariants();
  }
  else if (opt_find_invariants) {
    prep.bfs_find_invariants();
  }
  if (opt_verify_invariants) {
    prep.verify_invariants(*(prep.inconsistency()));
    prep.remove_unverified_invariants();
  }

  std::cerr << "constructing SAS+ instance..." << std::endl;
  instance.construct(p_instance, opt_sas_select, opt_sas_min, opt_sas_safe);
  HSPS::index_set goal_variables;
  instance.goal_state.defined_set(goal_variables);
  std::cerr << "SAS instance " << instance.name
	    << " built in " << stats.time() << " seconds" << std::endl;
  std::cerr << instance.n_variables() << " variables ("
	    << goal_variables.length() << " with goal value), "
	    << instance.n_actions() << " actions" << std::endl;
  stats.stop();

  stats.start();
  std::cerr << "checking restrictions..." << std::endl;
  bool isU = instance.check_U();
  bool isS = instance.check_S();
  if (!isU) {
    std::cerr << "error: problem is not unary - this algorithm can't solve it"
	      << std::endl;
    exit(1);
  }
  if (!isS) {
    std::cerr << "error: problem is not single-valued - this algorithm can't solve it"
	      << std::endl;
    exit(1);
  }
  bool isP = instance.check_P();
  if (isP) {
    std::cerr << "note: problem is post-unique" << std::endl;
  }

  gprv.assign_value(HSPS::no_such_index, instance.n_variables());
  a_pre = new HSPS::index_set*[instance.n_variables()];
  a_post = new HSPS::index_set*[instance.n_variables()];
  for (HSPS::index_type k = 0; k < instance.n_variables(); k++) {
    a_pre[k] = new HSPS::index_set[instance.variables[k].n_values()];
    a_post[k] = new HSPS::index_set[instance.variables[k].n_values()];
  }
  for (HSPS::index_type k = 0; k < instance.n_actions(); k++) {
    for (HSPS::index_type i = 0; i < instance.actions[k].pre.length(); i++)
      a_pre[instance.actions[k].pre[i].first][instance.actions[k].pre[i].second].insert(k);
    for (HSPS::index_type i = 0; i < instance.actions[k].post.length(); i++)
      a_post[instance.actions[k].post[i].first][instance.actions[k].post[i].second].insert(k);
  }

  HSPS::bool_vec prv_variables(false, instance.n_variables());

  for (HSPS::index_type k = 0; k < instance.n_actions(); k++)
    for (HSPS::index_type i = 0; i < instance.actions[k].prv.length(); i++) {
      HSPS::index_type var = instance.actions[k].prv[i].first;
      HSPS::index_type val = instance.actions[k].prv[i].second;
      if (gprv[var] == HSPS::no_such_index) {
	gprv[var] = val;
	prv_variables[var] = true;
      }
      else
	assert(gprv[var] == val);
    }

  bool done = false;
  bool solved = false;
  HSPS::ActionSequence plan;

  while (!done) {
    HSPS::partial_state  state1;
    HSPS::achieve_prevail(prv_variables, state1, plan);

    HSPS::partial_state  state2;
    HSPS::ActionSequence beta;
    HSPS::achieve_goal(state1, state2, beta);

    if (state1.implies(state2)) {
      plan.append(beta);
      solved = true;
      done = true;
    }
    else {
      HSPS::index_set cset;
      state1.conflict_set(state2, cset);
      HSPS::bool_vec non_prv(prv_variables);
      non_prv.complement();
      if (cset.have_common_element(non_prv)) {
	done = true; // no solution
      }
      else {
	prv_variables.subtract(cset);
      }
    }
  }
  stats.stop();

  if (opt_pddl) {
    if (solved) {
      HSPS::PrintPDDL print_plan(p_instance, std::cout);
      plan.output(print_plan);
    }
    else if (!stats.break_signal_raised()) {
      std::cout << ";; no solution" << std::endl;
    }
  }

  else {
    std::cout << "; Time " << HSPS::Stopwatch::seconds() << std::endl;
    if (solved) {
      std::cout << "; NrActions " << plan.length() << std::endl;
      std::cout << "; MakeSpan " << std::endl;
      std::cout << "; MetricValue " << std::endl;
      HSPS::PrintIPC print_plan(p_instance, std::cout);
      plan.output(print_plan);
    }
    else {
      std::cout << "; Not Solved (" << stats.flags() << ")" << std::endl;
    }
  }

  return 0;
}
