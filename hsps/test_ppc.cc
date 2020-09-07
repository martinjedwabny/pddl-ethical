
#include "parser.h"
#include "preprocess.h"
#include "exec.h"
#include "hypergraph.h"
#include "cost_table.h"
#include "enumerators.h"

#include <sstream>
#include <fstream>

BEGIN_HSPS_NAMESPACE

enum pc_type { pc_at_end,
	       pc_always,
	       pc_sometime,
	       pc_at_most_once,
	       pc_sometime_before,
	       pc_sometime_after };

struct ppc {
  const Name* name;
  NTYPE     weight;
  pc_type   pct;
  index_set s_c;
  index_set s_t;
  PDDL_Base::Preference* src;

  ppc()
    : name(0), weight(0), pct(pc_at_end), src(0) { };
  ppc(Name* n, NTYPE w, pc_type t, const index_set& a_c, const index_set& a_t, PDDL_Base::Preference* s)
    : name(n), weight(w), pct(t), s_c(a_c), s_t(a_t), src(s) { };

  ppc& operator=(const ppc& p) {
    name = p.name;
    weight = p.weight;
    pct = p.pct;
    s_c.assign_copy(p.s_c);
    s_t.assign_copy(p.s_t);
    src = p.src;
  };
};

void write_ppc(std::ostream& s, const Instance& ins, const ppc& p)
{
  if (p.name)
    s << p.name << ": ";
  switch (p.pct) {
  case pc_at_end:
    s << "at-end ";
    ins.write_atom_set(s, p.s_c);
    break;
  case pc_always:
    s << "always ";
    ins.write_atom_set(s, p.s_c);
    break;
  case pc_sometime:
    s << "sometime ";
    ins.write_atom_set(s, p.s_c);
    break;
  case pc_at_most_once:
    s << "at-most-once ";
    ins.write_atom_set(s, p.s_c);
    break;
  case pc_sometime_before:
    s << "sometime-before ";
    ins.write_atom_set(s, p.s_t);
    s << " ";
    ins.write_atom_set(s, p.s_c);
    break;
  case pc_sometime_after:
    s << "sometime-after ";
    ins.write_atom_set(s, p.s_t);
    s << " ";
    ins.write_atom_set(s, p.s_c);
    break;
  default:
    s << "ERROR!";
    exit(255);
  }
  s << " (" << p.weight << ")";
}

void enforce_ppc(const ppc& p, Instance& ins)
{
  switch (p.pct) {
  case pc_always:
    ins.enforce_pc_always(p.s_c, p.name);
    break;
  case pc_sometime:
    ins.enforce_pc_sometime(p.s_c, p.name);
    break;
  case pc_at_most_once:
    ins.enforce_pc_at_most_once(p.s_c, p.name);
    break;
  case pc_sometime_before:
    ins.enforce_pc_sometime_before(p.s_t, p.s_c, p.name);
    break;
  default:
    std::cerr << "ERROR!";
    exit(255);
  }
}

bool test_ppc(const ppc& p, ExecTrace* t)
{
  switch (p.pct) {
  case pc_always:
    if (p.s_c.length() != 1) return false;
    return t->test_always(p.s_c[0]);
  case pc_sometime:
    if (p.s_c.length() != 1) return false;
    return t->test_sometime(p.s_c[0]);
  case pc_at_most_once:
    if (p.s_c.length() != 1) return false;
    return t->test_at_most_once(p.s_c[0]);
  case pc_sometime_before:
    if (p.s_c.length() != 1) return false;
    if (p.s_t.length() != 1) return false;
    return t->test_sometime_before(p.s_t[0], p.s_c[0]);
  default:
    std::cerr << "ERROR!";
    exit(255);
  }
}

typedef lvector<ppc> ppc_vec;

void write_ppc_set(std::ostream& s, const ppc_vec& pv, const index_set& set)
{
  s << "{";
  for (index_type k = 0; k < set.length(); k++) {
    if (k > 0) s << ", ";
    s << pv[set[k]].name;
  }
  s << "}";
}

bool append_ppc
(PDDL_Base* b,
 PDDL_Base::Preference* p,
 PDDL_Base::Goal* g1,
 PDDL_Base::Goal* g2,
 Instance& ins,
 pc_type t,
 index_type k,
 ppc_vec& ppcv)
{
  PDDL_Base::atom_vec a(0, 0);
  index_set s1;
  if (b->goal_to_atom_vec(g1, a)) {
    Name* n = (p->name ?
	       (Name*)new StringName(p->name->print_name) :
	       (Name*)new EnumName("ppc", k));
    NTYPE w = p->value(b->metric_type, b->metric);
    b->instantiate_atom_set(ins, a, s1);
    for (index_type i = 0; i < s1.length(); i++)
      ins.atoms[s1[i]].goal = true;
    if (g2) {
      PDDL_Base::atom_vec a2(0, 0);
      index_set s2;
      if (b->goal_to_atom_vec(g2, a2)) {
	b->instantiate_atom_set(ins, a2, s2);
	for (index_type i = 0; i < s2.length(); i++)
	  ins.atoms[s2[i]].goal = true;
	ppcv.append(ppc(n, w, t, s1, s2, p));
      }
      else {
	std::cerr << "error: can't handle goal ";
	p->print(std::cerr);
	std::cerr << std::endl;
	return false;
      }
    }
    else {
      ppcv.append(ppc(n, w, t, s1, EMPTYSET, p));
    }
    return true;
  }
  else {
    std::cerr << "error: can't handle goal ";
    p->print(std::cerr);
    std::cerr << std::endl;
    return false;
  }
}

StringTable symbols(50, lowercase_map);
Parser* reader = new Parser(symbols);

int main(int argc, char *argv[]) {
  bool     opt_preprocess = true;
  bool     opt_test = true;
  bool     opt_max = true;
  bool     opt_H2 = true;
  bool     opt_H3 = false;
  bool     opt_split = false;
  bool     opt_precheck = false;
  bool     opt_load = true;
  bool     opt_pddl = true;
  bool     opt_save_test = false;
  bool     opt_save_max = false;
  bool     opt_save_ext = false;
  bool     opt_save_compiled = false;
  NTYPE    opt_slack = 0;
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
    else if (strcmp(argv[k],"-3") == 0) {
      opt_H3 = true;
    }
    else if (strcmp(argv[k],"-1") == 0) {
      opt_H2 = false;
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
    else if (strcmp(argv[k],"-no-max") == 0) {
      opt_max = false;
    }
    else if (strcmp(argv[k],"-no-pddl") == 0) {
      opt_pddl = false;
    }
    else if (strcmp(argv[k],"-load") == 0) {
      opt_load = true;
    }
    else if (strcmp(argv[k],"-no-load") == 0) {
      opt_load = false;
    }
    else if (strcmp(argv[k],"-split") == 0) {
      opt_split = true;
    }
    else if (strcmp(argv[k],"-precheck") == 0) {
      opt_precheck = true;
    }
    else if ((strcmp(argv[k],"-save-test") == 0) ||
	     (strcmp(argv[k],"-save-tests") == 0)) {
      opt_save_test = true;
    }
    else if ((strcmp(argv[k],"-save-ext") == 0) ||
	     (strcmp(argv[k],"-save-exts") == 0)) {
      opt_save_ext = true;
    }
    else if (strcmp(argv[k],"-save-max") == 0) {
      opt_save_max = true;
    }
    else if (strcmp(argv[k],"-save-compiled") == 0) {
      opt_save_compiled = true;
    }
    else if ((strcmp(argv[k],"-slack") == 0) && (k < argc - 1)) {
      opt_slack = A_TO_N(argv[++k]);
    }
    else if (strcmp(argv[k],"-max-slack") == 0) {
      opt_slack = POS_INF;
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
  ppc_vec  ppcs;

  reader->post_process();

  stats.start();
  reader->instantiate(instance);
  index_set hard_goals;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (instance.atoms[k].goal)
      hard_goals.insert(k);

  bool ok = true;
  for (index_type k = 0; k < reader->dom_preferences.length(); k++) {
    if (reader->dom_preferences[k]->goal->g_class == PDDL_Base::goal_always) {
      if (!append_ppc(reader,
		      reader->dom_preferences[k],
		      ((PDDL_Base::SimpleSequenceGoal*)reader->dom_preferences[k]->goal)->constraint,
		      0,
		      instance,
		      pc_always,
		      k,
		      ppcs))
	ok = false;
    }
    else if (reader->dom_preferences[k]->goal->g_class == PDDL_Base::goal_sometime) {
      if (!append_ppc(reader,
		      reader->dom_preferences[k],
		      ((PDDL_Base::SimpleSequenceGoal*)reader->dom_preferences[k]->goal)->constraint,
		      0,
		      instance,
		      pc_sometime,
		      k,
		      ppcs))
	ok = false;
    }
    else if (reader->dom_preferences[k]->goal->g_class == PDDL_Base::goal_at_most_once) {
      if (!append_ppc(reader,
		      reader->dom_preferences[k],
		      ((PDDL_Base::SimpleSequenceGoal*)reader->dom_preferences[k]->goal)->constraint,
		      0,
		      instance,
		      pc_at_most_once,
		      k,
		      ppcs))
	ok = false;
    }
    else if (reader->dom_preferences[k]->goal->g_class == PDDL_Base::goal_sometime_before) {
      if (!append_ppc(reader,
		      reader->dom_preferences[k],
		      ((PDDL_Base::TriggeredSequenceGoal*)reader->dom_preferences[k]->goal)->constraint,
		      ((PDDL_Base::TriggeredSequenceGoal*)reader->dom_preferences[k]->goal)->trigger,
		      instance,
		      pc_sometime_before,
		      k,
		      ppcs))
	ok = false;
    }
    else {
      if (!append_ppc(reader,
		      reader->dom_preferences[k],
		      reader->dom_preferences[k]->goal,
		      0,
		      instance,
		      pc_always,
		      k,
		      ppcs))
	ok = false;
    }
  }

  if (!ok) exit(255);

  Preprocessor prep(instance, stats);

  if (opt_preprocess) {
    prep.preprocess();
    for (index_type k = 0; k < ppcs.length(); k++) {
      instance.remap_set(ppcs[k].s_c, prep.atom_map);
      instance.remap_set(ppcs[k].s_t, prep.atom_map);
    }
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
  for (index_type k = 0; k < ppcs.length(); k++)
    v_sum += ppcs[k].weight;
  NTYPE v_max = v_sum;
  NTYPE p_min = 0;

  std::cerr << ppcs.length() << " preferences over plan constraints"
	    << std::endl;
  std::cerr << "sum penalty = " << v_sum << std::endl;
  std::cerr << instance.n_atoms() << " atoms and "
	    << instance.n_actions() << " actions"
	    << std::endl;

  if (opt_verbose)
    for (index_type k = 0; k < ppcs.length(); k++) {
      std::cerr << k << ". ";
      write_ppc(std::cerr, instance, ppcs[k]);
      std::cerr << std::endl;
    }

  // hypergraph over ppcs
  hypergraph h_inc(ppcs.length());

  if (opt_load) {
    PDDL_Base::write_warnings = false;
    name_vec ppc_name(0, ppcs.length());
    for (index_type k = 0; k < ppcs.length(); k++)
      ppc_name[k] = ppcs[k].name;
    index_set_vec h_load;
    reader->export_sets(ppc_name, h_load);
    std::cerr << h_load.length() << " inconsistent sets found in input"
	      << std::endl;
    for (index_type k = 0; k < h_load.length(); k++)
      if (h_inc.is_independent(h_load[k]))
	h_inc.add_edge(h_load[k]);
  }

  // maximal known-to-be-satisfiable sets of ppcs
  index_set_vec max_sat;

  if (reader->n_plans() > 0) {
    std::cerr << "analyzing " << reader->n_plans() << " plans..." << std::endl;
    NTYPE v_max_plan = 0;

    for (index_type k = 0; k < reader->n_plans(); k++) {
      Schedule* plan = new Schedule(instance);
      reader->export_plan(k, instance, prep.action_map, *plan);
      ExecTrace* trace = new ExecTrace(instance);
      ExecErrorSet* errors = new ExecErrorSet();
      bool ok = plan->simulate(trace, errors, false);
      if (!ok) {
	std::cerr << "plan " << k << " not executable: ";
	errors->write(std::cerr);
	std::cerr << std::endl;
      }
      else {
	index_set sat;
	NTYPE val = 0;
	for (index_type i = 0; i < ppcs.length(); i++)
	  if (test_ppc(ppcs[i], trace)) {
	    std::cerr << "plan #" << k << " satisfies #" << i << ": ";
	    write_ppc(std::cerr, instance, ppcs[i]);
	    std::cerr << std::endl;
	    sat.insert(i);
	    val += ppcs[i].weight;
	  }
	if (val > v_max_plan) v_max_plan = val;
	max_sat.insert_maximal(sat);
      }
      delete errors;
      delete trace;
      delete plan;
    }

    std::cerr << "maximal satisfied sets of constraints:" << std::endl;
    for (index_type k = 0; k < max_sat.length(); k++) {
      std::cerr << k + 1 << ": ";
      write_ppc_set(std::cerr, ppcs, max_sat[k]);
      std::cerr << std::endl;
    }
    std::cerr << "max plan value = " << PRINT_NTYPE(v_max_plan)
	      << std::endl
	      << "min plan penalty = " << PRINT_NTYPE(v_sum - v_max_plan)
	      << std::endl;
  }

  // test-related stuff

  index_set ce;
  index_set ca;

  count_type n_tests = 0;

  if (opt_split) {
    for (index_type k = 0; k < ppcs.length(); k++) {
      if (ppcs[k].pct == pc_sometime)
	ce.insert(k);
      else
	ca.insert(k);
    }
    std::cerr << ce.length() << " type-E constraints: " << ce << std::endl;
    std::cerr << ca.length() << " type-A constraints: " << ca << std::endl;
  }
  else {
    ca.fill(ppcs.length());
  }

  if (opt_test) {
    stats.start();

    if (opt_precheck) {
      std::cerr << "pre-checking..." << std::endl;
      CostTable* h0 = new CostTable(instance, stats);
      if (opt_H3) {
	h0->compute_H3(ZeroACF());
      }
      else if (opt_H2) {
	h0->compute_H2(ZeroACF());
      }
      else {
	h0->compute_H1(ZeroACF());
      }
      for (index_type i = 0; i < ppcs.length(); i++)
	if (ppcs[i].pct == pc_always)
	  for (index_type j = 0; j < ppcs.length(); j++)
	    if (ppcs[j].pct == pc_sometime) {
	      index_set s(ppcs[i].s_c);
	      s.insert(ppcs[j].s_c);
	      if (INFINITE(h0->eval(s))) {
		index_set sel;
		sel.insert(i);
		sel.insert(j);
		std::cerr << "set " << sel << " = ";
		write_ppc_set(std::cerr, ppcs, sel);
		std::cerr << " is unachievable" << std::endl;
		h_inc.add_edge(sel);
	      }
	    }
      delete h0;
      n_tests += 1;
    }

    if (ce.length() > 0) {
      std::cerr << "checking type-E constraints..." << std::endl;
      Instance* test_ins = instance.copy();
      index_vec ace(no_such_index, ce.length());
      for (index_type k = 0; k < ce.length(); k++) {
	ace[k] =
	  test_ins->compile_pc_sometime(ppcs[ce[k]].s_c, ppcs[ce[k]].name);
	test_ins->atoms[ace[k]].goal = false;
      }
      test_ins->cross_reference();
      CostTable* h0 = new CostTable(*test_ins, stats);
      if (opt_H3) {
	h0->compute_H3(ZeroACF());
      }
      else if (opt_H2) {
	h0->compute_H2(ZeroACF());
      }
      else {
	h0->compute_H1(ZeroACF());
      }
      for (index_type k = 0; k < ce.length(); k++) {
	if (opt_verbose) {
	  std::cerr << "checking " << ce[k] << "..." << std::endl;
	}
	if (INFINITE(h0->incremental_eval(test_ins->goal_atoms, ace[k]))) {
	  std::cerr << "constraint " << ppcs[ce[k]].name
		    << " is unachievable" << std::endl;
	  h_inc.add_singleton_edge(ce[k]);
	}
	else if ((opt_H2 || opt_H3) && (opt_max_d > 1)) {
	  index_set g1(test_ins->goal_atoms);
	  g1.insert(ace[k]);
	  for (index_type i = k + 1; i < ce.length(); i++) {
	    if (opt_verbose) {
	      std::cerr << "checking " << ce[k] << " & " << ce[i] << "..."
			<< std::endl;
	    }
	    if (INFINITE(h0->incremental_eval(g1, ace[i]))) {
	      std::cerr << "constraints " << ppcs[ce[k]].name
			<< " and " << ppcs[ce[i]].name
			<< " are jointly unachievable" << std::endl;
	      h_inc.add_binary_edge(ce[k], ce[i]);
	    }
	  }
	}
      }
      delete h0;
      delete test_ins;
      n_tests += 1;
    }

    index_type d = opt_min_d;
    while ((d <= opt_max_d) && (d < ca.length())) {
      std::cerr << "checking " << d << "-consistency..." << std::endl;
      mSubsetEnumerator e(ca.length(), d);
      bool more = e.first();
      while (more) {
	assert(e.current_set_size() == d);
	index_set sel;
	e.current_set(ca, sel);
	if (h_inc.is_independent(sel)) {
	  if (opt_verbose)
	    std::cerr << "testing " << sel << " ("
		      << h_inc.n_edges() << " inc. sets, "
		      << n_tests << " tests, "
		      << stats.time() << " sec., "
		      << stats.peak_memory()
		      << "k)..." << std::endl;
	  Instance* test_ins = instance.copy();
	  for (index_type i = 0; i < sel.length(); i++)
	    enforce_ppc(ppcs[sel[i]], *test_ins);
	  index_vec ace;
	  if (ce.length() > 0) {
	    ace.assign_value(no_such_index, ce.length());
	    for (index_type k = 0; k < ce.length(); k++) {
	      ace[k] = test_ins->compile_pc_sometime(ppcs[ce[k]].s_c, ppcs[ce[k]].name);
	      test_ins->atoms[ace[k]].goal = false;
	    }
	  }
	  // Preprocessor* test_prep = new Preprocessor(*test_ins, stats);
	  // test_prep->preprocess();
	  test_ins->cross_reference();
	  CostTable* h0 = new CostTable(*test_ins, stats);
	  if (opt_H3) {
	    h0->compute_H3(ZeroACF());
	  }
	  else if (opt_H2) {
	    h0->compute_H2(ZeroACF());
	  }
	  else {
	    h0->compute_H1(ZeroACF());
	  }
	  if (INFINITE(h0->eval(test_ins->goal_atoms))) {
	    std::cerr << "set " << sel << " = ";
	    write_ppc_set(std::cerr, ppcs, sel);
	    std::cerr << " is unachievable" << std::endl;
	    h_inc.add_edge(sel);
	  }
	  else {
	    if (opt_save_test) {
	      std::ostringstream fname;
	      char* b = reader->problem_file_basename();
	      if (b) fname << b << "-";
	      fname << "test" << n_tests + 1 << "-compiled.pddl";
	      std::ofstream s_out(fname.str().c_str());
	      s_out << ";; " << instance.name << ", test problem #"
		    << (n_tests + 1) << std::endl;
	      write_ppc_set(s_out << ";; selected constraints: ", ppcs, sel);
	      test_ins->write_domain_init(s_out);
	      test_ins->write_domain_declarations(s_out);
	      test_ins->write_domain_actions(s_out);
	      test_ins->write_domain_DKEL_items(s_out);
	      if (HSPS::Instance::write_extra) {
		name_vec set_names(0, 0);
		index_set_vec sets;
		reader->export_action_partitions(set_names, sets);
		s_out << ";; action partitions" << std::endl;
		for (index_type k = 0; k < sets.length(); k++) {
		  instance.remap_set(sets[k], prep.action_map);
		  instance.write_domain_action_set(s_out, sets[k], set_names[k]);
		}
	      }
	      s_out << ")";
	      test_ins->write_problem(s_out);
	    }
	    if (ce.length() > 0) {
	      for (index_type k = 0; k < ce.length(); k++) {
		if (opt_verbose) {
		  std::cerr << "checking " << sel << " & " << ce[k] << "..."
			    << std::endl;
		}
		if (INFINITE(h0->incremental_eval(test_ins->goal_atoms, ace[k]))) {
		  index_set sel1(sel);
		  sel1.insert(ce[k]);
		  if (h_inc.is_independent(sel1)) {
		    std::cerr << "set " << sel1 << " = ";
		    write_ppc_set(std::cerr, ppcs, sel1);
		    std::cerr << " is unachievable" << std::endl;
		    h_inc.add_edge(sel1);
		  }
		}
		else if ((opt_H2 || opt_H3) && (opt_max_d > 1)) {
		  index_set g1(test_ins->goal_atoms);
		  g1.insert(ace[k]);
		  for (index_type i = k + 1; i < ce.length(); i++) {
		    if (opt_verbose) {
		      std::cerr << "checking " << sel << " & " << ce[k]
				<< " & " << ce[i] << "..." << std::endl;
		    }
		    if (INFINITE(h0->incremental_eval(g1, ace[i]))) {
		      index_set sel2(sel);
		      sel2.insert(ce[k]);
		      sel2.insert(ce[i]);
		      if (h_inc.is_independent(sel2)) {
			std::cerr << "set " << sel2 << " = ";
			write_ppc_set(std::cerr, ppcs, sel2);
			std::cerr << " is unachievable" << std::endl;
			h_inc.add_edge(sel2);
		      }
		    }
		  }
		}
	      }
	    }
	  }
	  delete h0;
	  delete test_ins;
	  n_tests += 1;
	}
	more = e.next();
      }
      std::cerr << h_inc.n_edges() << " inconsistent sets found ("
		<< n_tests << " tests, " << stats << ")" << std::endl;
      d += 1;
    }

    stats.stop();
  } // if (opt_test) ...

  if (opt_max) {
    std::cerr << "computing maximal consistent sets..." << std::endl;

    index_set_vec mcs;
    h_inc.independent_sets(mcs);
    cost_vec set_value(0, mcs.length());
    cost_vec set_penalty(0, mcs.length());
    std::cerr << mcs.length() << " maximal consistent sets" << std::endl;
    for (index_type k = 0; k < mcs.length(); k++) {
      if (opt_verbose) std::cerr << "{";
      for (index_type i = 0; i < mcs[k].length(); i++) {
	if (opt_verbose) {
	  if (i > 0) std::cerr << ", ";
	  std::cerr << ppcs[mcs[k][i]].name;
	}
	set_value[k] += ppcs[mcs[k][i]].weight;
      }
      set_penalty[k] = v_sum - set_value[k];
      if (opt_verbose) 
	std::cerr << "}: value = " << PRINT_NTYPE(set_value[k])
		  << ", penalty = " << PRINT_NTYPE(set_penalty[k])
		  << std::endl;
    }

    v_max = cost_vec_util::max(set_value);
    p_min = cost_vec_util::min(set_penalty);

    std::cerr << "max value = " << PRINT_NTYPE(v_max) << std::endl;
    std::cerr << "min penalty = " << PRINT_NTYPE(p_min) << std::endl;

    if (opt_save_max) {
      for (index_type k = 0; k < mcs.length(); k++)
	if (set_value[k] >= (v_max - opt_slack)) {
	  std::ostringstream fname;
	  char* b = reader->problem_file_basename();
	  if (b) fname << b << "-";
	  fname << "max" << k + 1;
	  if (opt_save_compiled) fname << "-compiled";
	  fname << ".pddl";
	  std::ofstream s_out(fname.str().c_str());

	  s_out << ";; " << instance.name << ", test problem #"
		<< (k + 1) << std::endl;
	  s_out << ";; selected constraints: {";
	  for (index_type i = 0; i < mcs[k].length(); i++) {
	    if (i > 0) s_out << ", ";
	    s_out << ppcs[mcs[k][i]].name;
	  }
	  s_out << "}" << std::endl;
	  s_out << ";; value = " << PRINT_NTYPE(set_value[k])
		<< ", penalty = " << PRINT_NTYPE(set_penalty[k])
		<< std::endl;

	  if (opt_save_compiled) {
	    Instance* test_ins = instance.copy();
	    for (index_type i = 0; i < mcs[k].length(); i++)
	      enforce_ppc(ppcs[mcs[k][i]], *test_ins);
	    test_ins->write_domain_init(s_out);
	    test_ins->write_domain_declarations(s_out);
	    test_ins->write_domain_actions(s_out);
	    test_ins->write_domain_DKEL_items(s_out);
	    if (HSPS::Instance::write_extra) {
	      name_vec set_names(0, 0);
	      index_set_vec sets;
	      reader->export_action_partitions(set_names, sets);
	      s_out << ";; action partitions" << std::endl;
	      for (index_type k = 0; k < sets.length(); k++) {
		instance.remap_set(sets[k], prep.action_map);
		instance.write_domain_action_set(s_out, sets[k], set_names[k]);
	      }
	    }
	    s_out << ")";
	    test_ins->write_problem(s_out);
	    delete test_ins;
	  }
	  else {
	    reader->write_problem_begin(s_out);
	    reader->write_objects(s_out, true);
	    reader->write_init(s_out);
	    s_out << " (:goal (and";
	    for (index_type i = 0; i < reader->dom_goals.length(); i++) {
	      if (reader->dom_goals[i]->is_state() &&
		  reader->dom_goals[i]->is_propositional()) {
		s_out << " ";
		reader->dom_goals[i]->print(s_out);
	      }
	    }
	    s_out << "))" << std::endl << " (:constraints (and";
	    for (index_type i = 0; i < mcs[k].length(); i++) {
	      s_out << " ";
	      ppcs[mcs[k][i]].src->goal->print(s_out);
	    }
	    s_out << "))" << std::endl << ")" << std::endl;
	  }
	  s_out.close();
	}
    }
  }

  if (opt_save_ext && (max_sat.length() > 0)) {
    index_set_vec exts;
    for (index_type k = 0; k < max_sat.length(); k++) {
      bool_vec in(max_sat[k], ppcs.length());
      for (index_type i = 0; i < ppcs.length(); i++) if (!in[i]) {
	in[i] = true;
	if (h_inc.is_independent(in)) {
	  exts.insert_maximal(index_set(in));
	}
	in[i] = false;
      }
    }

    std::cerr << exts.length() << " consistent 1-step extensions..."
	      << std::endl;
    for (index_type k = 0; k < exts.length(); k++) {
      std::ostringstream fname;
      char* b = reader->problem_file_basename();
      if (b) fname << b << "-";
      fname << "ext" << k + 1;
      if (opt_save_compiled) fname << "-compiled";
      fname << ".pddl";
      std::ofstream s_out(fname.str().c_str());

      s_out << ";; " << instance.name << ", consistent extension #"
	    << (k + 1) << std::endl;
      s_out << ";; selected constraints: {";
      NTYPE val = 0;
      for (index_type i = 0; i < exts[k].length(); i++) {
	if (i > 0) s_out << ", ";
	s_out << ppcs[exts[k][i]].name;
	val += ppcs[exts[k][i]].weight;
      }
      s_out << "}" << std::endl;
      s_out << ";; value = " << PRINT_NTYPE(val)
	    << ", penalty = " << PRINT_NTYPE(v_sum - val)
	    << std::endl;

      if (opt_save_compiled) {
	Instance* test_ins = instance.copy();
	for (index_type i = 0; i < exts[k].length(); i++)
	  enforce_ppc(ppcs[exts[k][i]], *test_ins);
	test_ins->write_domain_init(s_out);
	test_ins->write_domain_declarations(s_out);
	test_ins->write_domain_actions(s_out);
	test_ins->write_domain_DKEL_items(s_out);
	if (HSPS::Instance::write_extra) {
	  name_vec set_names(0, 0);
	  index_set_vec sets;
	  reader->export_action_partitions(set_names, sets);
	  s_out << ";; action partitions" << std::endl;
	  for (index_type k = 0; k < sets.length(); k++) {
	    instance.remap_set(sets[k], prep.action_map);
	    instance.write_domain_action_set(s_out, sets[k], set_names[k]);
	  }
	}
	s_out << ")";
	test_ins->write_problem(s_out);
	delete test_ins;
      }
      else {
	reader->write_problem_begin(s_out);
	reader->write_objects(s_out, true);
	reader->write_init(s_out);
	s_out << " (:goal (and";
	for (index_type i = 0; i < reader->dom_goals.length(); i++) {
	  if (reader->dom_goals[i]->is_state() &&
	      reader->dom_goals[i]->is_propositional()) {
	    s_out << " ";
	    reader->dom_goals[i]->print(s_out);
	  }
	}
	s_out << "))" << std::endl << " (:constraints (and";
	for (index_type i = 0; i < exts[k].length(); i++) {
	  s_out << " ";
	  ppcs[exts[k][i]].src->goal->print(s_out);
	}
	s_out << "))" << std::endl << ")" << std::endl;
      }
      s_out.close();
    }
  }

  if (opt_pddl) {
    std::cout << ";; " << n_tests << " tests, " << stats << std::endl;
    std::cout << ";; max value = " << PRINT_NTYPE(v_max) << std::endl;
    std::cout << ";; min penalty = " << PRINT_NTYPE(p_min) << std::endl;

    std::cout << "(define (problem " << reader->problem_name << ")"
	      << std::endl
	      << " ;; inconsistent preference sets:" << std::endl;
    for (index_type k = 0; k < h_inc.n_edges(); k++) {
      std::cout << " (:set";
      for (index_type i = 0; i < h_inc.edge(k).length(); i++)
	std::cout << " " << ppcs[h_inc.edge(k)[i]].name;
      std::cout << ")" << std::endl;
    }
    std::cout << ")" << std::endl;
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
