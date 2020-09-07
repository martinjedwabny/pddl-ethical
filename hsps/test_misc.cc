
#include "problem.h"
#include "preprocess.h"
#include "parser.h"
#include "pdb.h"
#include "enumerators.h"

// #include <iostream>
// #define MK_INSTANCE
// #define MK_SAS_INSTANCE
#define NO_FIND_INVARIANTS
#define NO_VERIFY_INVARIANTS
// #define PRINT_SAS_VARIABLES

BEGIN_HSPS_NAMESPACE


void hypergraph_add_edge
(const index_set& e, index_set_vec& h_edge, index_set_vec& h_in)
{
  h_edge.append(e);
  for (index_type k = 0; k < e.length(); k++)
    h_in[e[k]].insert(h_edge.length() - 1);
}

bool hypergraph_is_independent
(const index_set& ns, const index_set_vec& h_edge, const index_set_vec& h_in)
{
  bool_vec t(false, h_edge.length());
  for (index_type k = 0; k < ns.length(); k++)
    for (index_type i = 0; i < h_in[ns[k]].length(); i++)
      if (!t[h_in[ns[k]][i]]) {
	if (ns.contains(h_edge[h_in[ns[k]][i]]))
	  return false;
	t[h_in[ns[k]][i]] = true;
      }
  return true;
}

bool hypergraph_is_independent
(const bool_vec& ns, const index_set_vec& h_edge, const index_set_vec& h_in)
{
  bool_vec t(false, h_edge.length());
  for (index_type k = 0; k < ns.length(); k++)
    if (ns[k])
      for (index_type i = 0; i < h_in[k].length(); i++)
	if (!t[h_in[k][i]]) {
	  if (ns.contains(h_edge[h_in[k][i]]))
	    return false;
	  t[h_in[k][i]] = true;
	}
  return true;
}

void hypergraph_independent_sets
(index_type n,
 const index_set_vec& h_edge,
 const index_set_vec& h_in,
 index_set_vec& is)
{
  index_set_vec cs;
  cs.append(EMPTYSET);
  bool_vec o(false, h_edge.length());
  for (index_type k = 0; k < n; k++)
    for (index_type i = 0; i < h_in[k].length(); i++) {
      index_type e = h_in[k][i];
      if (!o[e]) {
	std::cerr << "uncovered edge: " << h_edge[e] << std::endl;
	index_type m = cs.length();
	for (index_type j = 0; j < m; j++) {
	  std::cerr << " set " << cs[j] << "...";
	  if (cs[j].first_common_element(h_edge[e]) == no_such_index) {
	    std::cerr << " does not cover this edge" << std::endl;
	    for (index_type l = 1; l < h_edge[e].length(); l++) {
	      index_set x(cs[j]);
	      x.insert(h_edge[e][l]);
	      cs.append(x);
	    }
	    cs[j].insert(h_edge[e][0]);
	  }
	  else {
	    std::cerr << " already covers this edge" << std::endl;
	  }
	}
	cs.reduce_to_minimal();
	o[h_in[k][i]] = true;
      }
    }
  is.assign_value(EMPTYSET, cs.length());
  for (index_type k = 0; k < is.length(); k++) {
    is[k].fill(n);
    is[k].subtract(cs[k]);
  }
  std::cerr << "edges: " << h_edge << std::endl;
  std::cerr << "minimal covering sets: " << cs << std::endl;
  std::cerr << "maximal independent sets: " << is << std::endl;
}


int main(int argc, char *argv[]) {
  StringTable symbols(50, lowercase_map);
  int         verbose_level = 1;
  unsigned long seed = 0;

  Parser* reader = new Parser(symbols);

  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      verbose_level = atoi(argv[++k]);
    }
    else if ((strcmp(argv[k],"-r") == 0) && (k < argc - 1)) {
      seed = atoi(argv[++k]);
    }
    else if (*argv[k] != '-') {
      reader->read(argv[k], false);
    }
  }

  Heuristic::default_trace_level = verbose_level;
  Instance::default_trace_level = verbose_level;
  Preprocessor::default_trace_level = verbose_level;
  if (verbose_level <= 0) PDDL_Base::write_warnings = false;
  if (verbose_level > 1) PDDL_Base::write_info = true;

  Statistics  stats;
  stats.enable_interrupt(true);

  LC_RNG rng;
  if (seed > 0) rng.seed(seed);

#ifdef MK_INSTANCE
  stats.start();
  std::cerr << "instantiating..." << std::endl;
  Instance* instance = new Instance();
  reader->instantiate(*instance);

  Preprocessor* prep = new Preprocessor(*instance, stats);
  std::cerr << "preprocessing..." << std::endl;
  prep->preprocess();

#ifdef MK_SAS_INSTANCE
#ifndef NO_FIND_INVARIANTS
  prep->bfs_find_invariants();
#endif
#ifndef NO_VERIFY_INVARIANTS
  prep->verify_invariants(*(prep->inconsistency()));
#endif

  SASInstance* sas_p = new SASInstance(*instance, false, false);
#ifdef PRINT_SAS_VARIABLES
  std::cerr << "variables:" << std::endl;
  for (HSPS::index_type k = 0; k < sas_p->n_variables(); k++) {
    std::cerr << "#" << k << ": ";
    sas_p->write_variable(std::cerr, sas_p->variables[k]);
    std::cerr << std::endl;
  }
  std::cerr << "initial: ";
  sas_p->write_partial_state(std::cerr, sas_p->init_state);
  std::cerr << ";" << std::endl;
  std::cerr << "goal: ";
  sas_p->write_partial_state(std::cerr, sas_p->goal_state);
  std::cerr << ";" << std::endl;
#endif
  name_vec sas_names(0, 0);
  sas_p->variable_names(sas_names);
#endif

  stats.stop();
  std::cerr << "instance " << instance->name << " built in "
	    << stats.time() << " seconds" << std::endl;
  std::cerr << instance->n_atoms() << " atoms, "
	    << instance->n_resources() << " resources ("
	    << instance->n_reusable_resources() << " reusable, "
	    << instance->n_consumable_resources() << " consumable), "
	    << instance->n_actions() << " actions, "
	    << instance->n_invariants() << " invariants"
	    << std::endl;
#endif

  // TESTS BELOW

  index_set_vec h_edge;
  index_set_vec h_in(EMPTYSET, 5);

  index_set e1;
  e1.insert(0);
  e1.insert(1);
  hypergraph_add_edge(e1, h_edge, h_in);

  index_set e2;
  e2.insert(1);
  e2.insert(2);
  e2.insert(4);
  hypergraph_add_edge(e2, h_edge, h_in);

  index_set e3;
  e3.insert(2);
  e3.insert(3);
  hypergraph_add_edge(e3, h_edge, h_in);

  index_set_vec is;
  hypergraph_independent_sets(5, h_edge, h_in, is);

//   for (HSPS::index_type k = 0; k < 3; k++) {
//     graph g(10);
//     g.random_tree(2, index_type_max, rng);
//     g.randomize(5, rng);
//     g.write_digraph(std::cout, "RG");
//   }

#ifdef DO_NOT_COMPILE
  name_vec var_names(0, 0);
  sas_p->variable_names(var_names);
  index_set_vec sets;
  reader->export_sets(var_names, sets);
  assert(sets.length() > 0);
  SASCostACF cost(*sas_p);
  Heuristic* inc = prep->inconsistency();
  MDDNode* sinc =  makeMDD(prep->inconsistency(),
			   sas_p->atom_map_defined(),
			   sas_p->atom_map_n());
  for (index_type k = 0; k < sets.length(); k++) {
    ProgressionPDB* pdb =
      new ProgressionPDB(*sas_p, sets[k], cost, sinc, inc, stats);
    std::cerr << "computing PDB ";
    sas_p->write_variable_set(std::cerr, sets[k]);
    std::cerr << "..." << std::endl;
    stats.start();
    pdb->compute();
    stats.stop();
    std::cerr << "time: " << stats.time() << std::endl;
    delete pdb;
  }

  std::cerr << "total time: " << stats.total_time() << std::endl;
#endif

#ifdef DO_NOT_COMPILE
  index_type d = 1;
  index_type t = (10000000/4);
  index_vec f(0, 0);

  bool done = false;
  while (!done) {
    factor(t - d, f);
    if (f.length() == 1) {
      std::cerr << t - d << " is prime" << std::endl;
      done = true;
    }
    factor(t + d, f);
    if (f.length() == 1) {
      std::cerr << t + d << " is prime" << std::endl;
      done = true;
    }
    d += 1;
  }
#endif

#ifdef DO_NOT_COMPILE
  index_set_vec sets;
  reader->export_sets(sas_names, sets);
  assert(sets.length() > 1);
  std::cerr << "set[0] = " << sets[0] << " = ";
  sas_p->write_variable_set(std::cerr, sets[0]);
  std::cerr << std::endl;
  std::cerr << "set[1] = " << sets[1] << " = ";
  sas_p->write_variable_set(std::cerr, sets[1]);
  std::cerr << std::endl;

  assert(reader->n_plans() > 0);
  ActionSequence* s = new ActionSequence();
  bool ok = reader->export_plan(0, *instance, prep->action_map, *s);
  assert(ok);

  UnitACF cost;
  Heuristic* inc = prep->inconsistency();
  MDDNode* sinc = makeMDD(prep->inconsistency(),
			  sas_p->atom_map_defined(),
			  sas_p->atom_map_n());
  ProgressionPDB* pdb =
    new ProgressionPDB(*sas_p, sets[1], cost, sinc, inc, stats);
  pdb->compute();
  std::cerr << "PDB:" << std::endl;
  pdb->write(std::cerr);

  ProgressionASH* ash =
    new ProgressionASH(*sas_p, sets[0], cost, sinc, inc, *pdb, stats);

  std::cerr << "applying walk " << *s << "..." << std::endl;
  partial_state s0(sas_p->init_state, sets[0]);
  index_type l = ash->walk(s0, *s, sinc, inc);
  assert(l >= s->length());

  std::cerr << "evaluating abstract state: ";
  sas_p->write_partial_state(std::cerr, s0);
  std::cerr << std::endl;

  std::cerr << "PDB(s0) = " << pdb->eval(s0) << std::endl;
  NTYPE v1 = ash->eval(s0);
  std::cerr << "ASH(s0) = " << v1 << std::endl;
#endif

#ifdef DO_NOT_COMPILE
  name_vec pnames(0, 0);
  index_set_vec partition;
  reader->export_action_partitions(pnames, partition);
  instance->remap_sets(partition, prep->action_map);

  UnitACF acf;
  AH* ah = new AH(*instance, stats);
  ah->compute_additive_H2(acf, partition);
  // ah->compute_with_iterative_assignment(acf, instance->goal_atoms, true, true);

  CombineNByMax* h = new CombineNByMax(*instance);
  for (index_type k = 0; k < instance->n_invariants(); k++)
    if ((instance->invariants[k].lim == 1) && instance->invariants[k].exact) {
      HX* hx = new HX(*instance, *ah, instance->invariants[k].set);
      h->add(hx);
    }

  std::cerr << "h+(goal) = " << ah->eval(instance->goal_atoms)
	    << ", hX+(goal) = " << h->eval(instance->goal_atoms)
	    << std::endl;
#endif

#ifdef DO_NOT_COMPILE
  index_type gsize = 5;
  graph g(gsize);

  std::cout.setf(std::ios_base::fixed);
  std::cout.precision(2);

  g.random_digraph_with_density(rational(1,4), rng);
  std::cout << "graph 1.1: " << g
	    << " (" << (g.n_edges()/20.0)*100.0
	    << "% edges, connected: "
	    << g.connected() << ")"
	    << std::endl;

  g.random_digraph_with_density(rational(1,4), rng);
  std::cout << "graph 1.2: " << g
	    << " (" << (g.n_edges()/20.0)*100.0
	    << "% edges, connected: "
	    << g.connected() << ")"
	    << std::endl;

  g.random_digraph_with_density(rational(1,2), rng);
  std::cout << "graph 2.1: " << g
	    << " (" << (g.n_edges()/20.0)*100.0
	    << "% edges, connected: "
	    << g.connected() << ")"
	    << std::endl;

  g.random_digraph_with_density(rational(1,2), rng);
  std::cout << "graph 2.2: " << g
	    << " (" << (g.n_edges()/20.0)*100.0
	    << "% edges, connected: "
	    << g.connected() << ")"
	    << std::endl;

  g.random_digraph_with_density(rational(3,4), rng);
  std::cout << "graph 3.1: " << g
	    << " (" << (g.n_edges()/20.0)*100.0
	    << "% edges, connected: "
	    << g.connected() << ")"
	    << std::endl;

  g.random_digraph_with_density(rational(3,4), rng);
  std::cout << "graph 3.2: " << g
	    << " (" << (g.n_edges()/20.0)*100.0
	    << "% edges, connected: "
	    << g.connected() << ")"
	    << std::endl;
#endif

#ifdef DO_NOT_COMPILE
  index_set_vec sets;
  reader->export_sets(sas_names, sets);
  if (sets.length() < 1) {
    std::cerr << "error: no variable set in input" << std::endl;
    exit(0);
  }

  UnitACF cost;
  for (index_type k = 0; k < sets.length(); k++) {
    std::cerr << "computing PDB for ";
    sas_p->write_variable_set(std::cerr, sets[k]);
    std::cerr << "..." << std::endl;
    RegressionPDB* pdb = new RegressionPDB(*sas_p, sets[k], stats);
    pdb->enable_trace();
    pdb->compute(cost, prep->inconsistency());
    pdb->write_eval_trace(std::cerr, sas_p->goal_state);
  }
#endif

#ifdef DO_NOT_COMPILE
  UnitACF cost;
  index_set_vec sets;
  reader->export_sets(sas_names, sets);

  for (index_type k = 0; k < sets.length(); k++) {
    std::cerr << "testing ";
    sas_p->write_variable_set(std::cerr, sets[k]);
    std::cerr << "..." << std::endl;
    index_set r_act;
    for (index_type i = 0; i < sas_p->n_actions(); i++)
      if (sas_p->actions[i].post.defines_any(sets[k]))
	r_act.insert(sas_p->actions[i].s_index);
    std::cerr << "relevant actions: ";
    instance->write_action_set(std::cerr, r_act);
    std::cerr << std::endl;
    RegressionPDB* pdb = new RegressionPDB(*sas_p, sets[k], stats);
    pdb->compute(cost, prep->inconsistency());
    bool_vec d(r_act, instance->n_actions());
    d.complement();
    DiscountACF d_cost(cost, d);
    CostTable* hd = new CostTable(*instance, stats);
    hd->compute_H2(d_cost);
    PartialStateEnumerator s(sets[k], sas_p->signature);
    bool more = s.first();
    while (more) {
      index_set a;
      sas_p->make_atom_set(s.current_state(), a);
      sas_p->write_partial_state(std::cerr, s.current_state());
      std::cerr << " / ";
      instance->write_atom_set(std::cerr, a);
      std::cerr << ": " << pdb->eval(s.current_state())
		<< " / " << hd->eval(a)
		<< std::endl;
      more = s.next();
    }
  }
#endif

#ifdef DO_NOT_COMPILE
  UnitACF cost;
  IndependentVariables iv(*sas_p);
  iv.compute_maximal_independent_sets();
  iv.compute_spanning_sets();

  for (index_type v = 0; v < iv.set(0).length(); v++) {
    SASInstance::Variable& var = sas_p->variables[iv.set(0)[v]];
    std::cerr << "testing ";
    sas_p->write_variable(std::cerr, var);
    std::cerr << "..." << std::endl;
    index_set r_act;
    for (index_type k = 0; k < sas_p->n_actions(); k++)
      if (sas_p->actions[k].post.defines(iv.set(0)[v]))
	r_act.insert(sas_p->actions[k].s_index);
    std::cerr << "relevant actions: ";
    instance->write_action_set(std::cerr, r_act);
    std::cerr << std::endl;
    index_set v_set;
    v_set.assign_singleton(iv.set(0)[v]);
    RegressionPDB* pdb = new RegressionPDB(*sas_p, v_set, stats);
    pdb->compute(cost, prep->inconsistency());
    bool_vec d(r_act, instance->n_actions());
    d.complement();
    DiscountACF d_cost(cost, d);
    CostTable* hd = new CostTable(*instance, stats);
    hd->compute_H2(d_cost);
    for (index_type k = 0; k < var.s_index.length(); k++) {
      index_type p = var.s_index[k];
      partial_state s;
      s.assign(iv.set(0)[v], k);
      std::cerr << instance->atoms[p].name << " / ";
      sas_p->write_partial_state(std::cerr, s);
      std::cerr << ": " << hd->eval(p)
		<< " / " << pdb->eval(s)
		<< std::endl;
    }
  }
#endif

#ifdef DO_NOT_COMPILE
  UnitACF cost;
  CostTable* h2 = new CostTable(*instance, stats);
  h2->compute_H2(cost);

  // additive h with strictly relevant + 1
  NTYPE w = h2->eval(instance->goal_atoms);
  for (index_type k = 0; k < instance->goal_atoms.length(); k++) {
    index_type g = instance->goal_atoms[k];
    std::cerr << "checking goal " << instance->atoms[g].name
	      << " (H2 cost = " << h2->eval(g) << ")..." << std::endl;
    NTYPE epsilon = 0;
    while ((h2->eval(g) + epsilon) <= w) {
      bool_vec rel(false, instance->n_actions());
      prep->strictly_relevant_actions(g, h2->eval(g)+epsilon, *h2, cost, rel);
      std::cerr << rel.count(true) << " of " << instance->n_actions()
 		<< " relevant at epsilon = " << epsilon
 		<< std::endl;
      epsilon = (epsilon + 1);
    }
  }
#endif

#ifdef DO_NOT_COMPILE
  // minmal additive h
  for (index_type k = 0; k < instance->goal_atoms.length(); k++) {
    index_type g = instance->goal_atoms[k];
    std::cerr << "checking goal " << instance->atoms[g].name
	      << "..." << std::endl;
    bool_vec must_rem(true, instance->n_actions());
    for (index_type i = 0; i < instance->n_actions(); i++) {
      bool_vec dis(false, instance->n_actions());
      dis[i] = true;
      DiscountACF d_cost(cost, dis);
      CostTable* hd = new CostTable(*instance, stats);
      hd->compute_H1(d_cost);
      if (h1->eval(g) == hd->eval(g)) {
	must_rem[i] = false;
      }
    }
    std::cerr << "necessary actions: ";
    instance->write_action_set(std::cerr, must_rem);
    std::cerr << " (" << must_rem.count(true) << " of "
	      << instance->n_actions() << ")" << std::endl;
    bool_vec undecided(must_rem);
    undecided.complement();
    index_set u;
    undecided.copy_to(u);
    std::cerr << "there are " << undecided.count(true) << " / "
	      << u.length() << " undecided actions" << std::endl;
    SubsetEnumerator e(u.length());
    bool more = e.first();
    index_type min_size = u.length();
    index_type min_count = 1;
    while (more) {
      // std::cerr << "current set size = " << e.current_set_size() << std::endl;
      index_set s;
      e.current_set(u, s);
      bool_vec dis(s, instance->n_actions());
      DiscountACF d_cost(cost, dis);
      CostTable* hd = new CostTable(*instance, stats);
      hd->compute_H1(d_cost);
      if (h1->eval(g) == hd->eval(g)) {
	dis.complement();
	if (dis.count(true) < min_size) {
	  min_size = dis.count(true);
	  min_count = 1;
	  std::cerr << "smaller set found: ";
	  instance->write_action_set(std::cerr, dis);
	  std::cerr << " (" << dis.count(true) << " of "
		    << instance->n_actions() << ")" << std::endl;
	}
	else if (dis.count(true) == min_size) {
	  min_count += 1;
	  std::cerr << "another set found: ";
	  instance->write_action_set(std::cerr, dis);
	  std::cerr << " (" << dis.count(true) << " of "
		    << instance->n_actions() << ")" << std::endl;
	}
      } 
      more = e.next();
    }
    std::cerr << min_count << " sets of size " << min_size << " found"
	      << std::endl;
  }
#endif

#ifdef DO_NOT_COMPILE
  // greedy additive h
  for (index_type k = 0; k < instance->goal_atoms.length(); k++) {
    index_type g = instance->goal_atoms[k];
    std::cerr << "checking goal " << instance->atoms[g].name
	      << " (cost = " << h1->eval(g) << ")..." << std::endl;
    bool_vec rem(true, instance->n_actions());
    bool_vec dis(false, instance->n_actions());
    for (index_type i = 0; i < instance->n_actions(); i++) {
      dis[i] = true;
      DiscountACF d_cost(cost, dis);
      CostTable* hd = new CostTable(*instance, stats);
      hd->compute_H1(d_cost);
      if (hd->eval(g) < h1->eval(g)) {
	dis[i] = false;
// 	std::cerr << "action " << instance->actions[i].name
// 		  << " can not be removed (discounted cost = "
// 		  << hd->eval(g) << ")" << std::endl;
      }
      else {
// 	std::cerr << "action " << instance->actions[i].name
// 		  << " removed" << std::endl;
	rem[i] = false;
      }
      delete hd;
    }
    std::cerr << "greedy set of actions: ";
    instance->write_action_set(std::cerr, rem);
    std::cerr << " (" << rem.count(true) << " of "
	      << instance->n_actions() << ")" << std::endl;
  }
#endif

#ifdef DO_NOT_COMPILE
  stats.start();
  graph pg(instance->n_atoms());
  prep->compute_landmark_graph(pg);
  stats.stop();
  std::cerr << "landmark graph computed in " << stats.time()
	    << " seconds" << std::endl;

  index_set lm_atoms;
  prep->compute_landmarks(false, true, lm_atoms);
  std::cerr << "landmark atoms: ";
  instance->write_atom_set(std::cerr, lm_atoms);
  std::cerr << " (" << lm_atoms.length() << " of "
	    << instance->n_atoms() << ")" << std::endl;
  Instance* lm_instance = new Instance(instance->name);
  index_set r_acts;
  index_vec r_atoms;
  lm_instance->restricted_copy(*instance, lm_atoms, EMPTYSET, EMPTYSET,
			       r_acts, r_atoms);
  lm_instance->cross_reference();
  std::cerr << "verifying restricted invariants..." << std::endl;
  Heuristic* h_inc =
    new AtomMapAdapter(*lm_instance, lm_atoms, *(prep->inconsistency()));
  Preprocessor* lm_prep = new Preprocessor(*lm_instance, stats);
  lm_prep->verify_invariants(*h_inc);
  // lm_instance->write_problem(std::cerr);

  std::cerr << "creating SAS instance..." << std::endl;
  SASInstance* sas_lm = new SASInstance(*lm_instance, false, false);
  std::cerr << "variables:" << std::endl;
  for (index_type k = 0; k < sas_lm->n_variables(); k++) {
    std::cerr << "(" << k << ") ";
    sas_lm->write_variable(std::cerr, sas_lm->variables[k]);
    std::cerr << std::endl;
  }
  std::cerr << "initial: ";
  sas_lm->write_partial_state(std::cerr, sas_lm->init_state);
  std::cerr << std::endl << "goal: ";
  sas_lm->write_partial_state(std::cerr, sas_lm->goal_state);
  std::cerr << std::endl;

  index_set all;
  all.fill(sas_lm->n_variables());
  graph pg_lm(pg, lm_atoms);

  std::cerr << "computing standard PDB..." << std::endl;
  RegressionPDB* std_pdb = new RegressionPDB(*sas_lm, all, stats);
  std_pdb->compute(UnitACF(), h_inc);
  std::cerr << "finished in " << stats.time() << " seconds" << std::endl;

  std::cerr << "computing PDB with precedence constraints..." << std::endl;
  RegressionPDB* prec_pdb = new RegressionPDB(*sas_lm, all, stats);
  prec_pdb->compute_with_precedence(UnitACF(), h_inc, pg_lm);
  std::cerr << "finished in " << stats.time() << " seconds" << std::endl;

  std::cerr << "comparing..." << std::endl;
  prec_pdb->set_trace_level(3);
  NTYPE diff = prec_pdb->compare(*std_pdb);
  std::cerr << "result = " << diff << std::endl;
#endif

  return 0;
}

END_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
