
#define CHECK_RATIONAL_ARITHMETIC

#define TEST_BY_COUNT_ONLY

#include "pdb_construction.h"
#include "math.h"

BEGIN_HSPS_NAMESPACE

PDBHeuristic::PDBHeuristic(SASInstance& ins, Statistics& s)
  : instance(ins), stats(s), pdbs(0, 0)
{
  // done
}

PDBHeuristic::~PDBHeuristic()
{
  for (index_type k = 0; k < pdbs.length(); k++)
    if (pdbs[k]) delete pdbs[k];
}

void PDBHeuristic::remove_subsets()
{
  for (index_type k = 0; k < sets.length(); k++)
    for (index_type i = 0; i < sets.length(); i++)
      if ((k != i) && sets[k].contains(sets[i])) sets[i].clear();
  sets.remove_empty_sets();
}

void PDBHeuristic::assign_sets(const index_set_vec& s)
{
  sets.assign_copy(s);
  discount.assign_value(EMPTYSET, sets.length());
  groups.clear();
}

index_type PDBHeuristic::new_component
(SASHeuristic* h, const index_set& v, const index_set& d)
{
  pdbs.append(h);
  index_type i = MaxH::new_component(h);
  if (sets.length() < i+1) sets.set_length(i+1);
  sets[i] = v;
  if (discount.length() < i+1) discount.set_length(i+1);
  discount[i] = d;
  return i;
}

void PDBHeuristic::write_variable_sets(std::ostream& s)
{
  for (index_type k = 0; k < sets.length(); k++) {
    if (k > 0) s << ", ";
    instance.write_variable_set(s, sets[k]);
  }
}

void PDBHeuristic::write_additive_groups(std::ostream& s)
{
  for (index_type k = 0; k < groups.length(); k++) {
    if (k > 0) s << ", ";
    s << "{";
    for (index_type i = 0; i < groups[k].length(); i++) {
      if (i > 0) s << " + ";
      instance.write_variable_set(s, sets[groups[k][i]]);
    }
    s << "}";
  }
}

void PDBHeuristic::write(std::ostream& s)
{
  for (index_type k = 0; k < n_components(); k++) {
    s << "PDB " << k << ": ";
    instance.write_variable_set(s, sets[k]);
    s << std::endl;
    AbstractionHeuristic* p = (AbstractionHeuristic*)components[k];
    p->write(s);
    s << std::endl;
  }
}

void PDBHeuristic::compute_additive_groups(bool maximal)
{
  if (maximal) {
    find_maximal_additive_groups(groups);
  }
  else {
    graph dep(sets.length());
    for (index_type i = 0; i < sets.length(); i++)
      for (index_type j = i + 1; j < sets.length(); j++)
	if (!additive(sets[i], sets[j])) dep.add_undirected_edge(i, j);
    dep.apx_independent_set_cover(groups);
  }
}

void PDBHeuristic::compute_regression_PDB
(const ACF& cost, MDDNode* sinc, Heuristic* inc)
{
  components.clear();
  for (index_type k = 0; (k < sets.length()) &&
	 !stats.break_signal_raised(); k++) {
    RegressionPDB* pdb =
      new RegressionPDB(instance, sets[k], cost, sinc, inc, stats);
    pdbs.append(pdb);
    if (trace_level > 1) {
      std::cerr << "computing PDB ";
      instance.write_variable_set(std::cerr, sets[k]);
      std::cerr << " counting all actions..." << std::endl;
    }
    pdb->compute();
    if (trace_level > 2) {
      std::cerr << "RegressionPDB " << k << ":" << std::endl;
      pdb->write(std::cerr);
    }
    new_component(pdb);
  }
}

void PDBHeuristic::compute_progression_PDB
(const ACF& cost, MDDNode* sinc, Heuristic* inc)
{
  components.clear();
  for (index_type k = 0; (k < sets.length()) &&
	 !stats.break_signal_raised(); k++) {
    ProgressionPDB* pdb =
      new ProgressionPDB(instance, sets[k], cost, sinc, inc, stats);
    pdbs.append(pdb);
    if (trace_level > 1) {
      std::cerr << "computing PDB ";
      instance.write_variable_set(std::cerr, sets[k]);
      std::cerr << " counting all actions..." << std::endl;
    }
    pdb->compute();
    if (trace_level > 2) {
      std::cerr << "ProgressionPDB " << k << ":" << std::endl;
      pdb->write(std::cerr);
    }
    new_component(pdb);
  }
}

void PDBHeuristic::make_additive_groups()
{
  sas_heuristic_vec old_comp(components);
  index_set_vec     old_sets(sets);
  components.clear();
  sets.clear();

  for (index_type k = 0; k < groups.length(); k++) {
    if (groups[k].length() > 1) {
      if (trace_level > 0) {
	std::cerr << "making additive group of " << groups[k].length()
		  << " sets: ";
	for (index_type i = 0; i < groups[k].length(); i++) {
	  if (i > 0) std::cerr << ", ";
	  instance.write_variable_set(std::cerr, old_sets[groups[k][i]]);
	}
	std::cerr << std::endl;
      }
      AddH* new_comp = new AddH();
      index_set new_set;
      for (index_type i = 0; i < groups[k].length(); i++) {
	new_comp->new_component(old_comp[groups[k][i]]);
	new_set.insert(old_sets[groups[k][i]]);
      }
      new_component(new_comp);
      sets.append(new_set);
    }
    else if (groups[k].length() == 1) {
      new_component(old_comp[groups[k][0]]);
      sets.append(old_sets[groups[k][0]]);
    }
    else {
      std::cerr << "error: empty additive group in " << groups << std::endl;
    }
  }

  if (trace_level > 1) {
    std::cerr << "New Partitioning: " << sets << std::endl;
  }
}

MaxAddH* PDBHeuristic::make_max_add()
{
  return new MaxAddH(components, groups);
}

void PDBHeuristic::make_extended_additive_groups
(const ACF& cost, MDDNode* sinc, Heuristic* inc, rational threshold)
{
  sas_heuristic_vec old_comp(components);
  index_set_vec     old_sets(sets);
  index_type        n_sets = sets.length();

  components.clear();
  sets.clear();

  for (index_type k = 0; k < groups.length(); k++) {
    assert(!groups[k].empty());
    if (stats.break_signal_raised()) return;

    index_set remaining;
    remaining.fill(n_sets);
    remaining.subtract(groups[k]);

    if (trace_level > 0) {
      std::cerr << "making additive group of " << groups[k].length()
		<< " sets: ";
      for (index_type i = 0; i < groups[k].length(); i++) {
	if (i > 0) std::cerr << ", ";
	instance.write_variable_set(std::cerr, old_sets[groups[k][i]]);
      }
      std::cerr << std::endl;
    }

    AddH* new_comp = new AddH();
    index_set new_set;
    for (index_type i = 0; i < groups[k].length(); i++) {
      new_comp->new_component(old_comp[groups[k][i]]);
      new_set.insert(old_sets[groups[k][i]]);
    }

    discount.assign_value(EMPTYSET, n_sets);
    index_set_vec rel(n_sets);
    for (index_type i = 0; i < remaining.length(); i++)
      relevant_actions(old_sets[remaining[i]], rel[remaining[i]]);
    lvector<rational> score(0, n_sets);

    for (index_type i = 0; i < remaining.length(); i++) {
      interfering_actions(new_set, old_sets[remaining[i]],
			  discount[remaining[i]]);
      index_type n_int =
	rel[remaining[i]].count_common(discount[remaining[i]]);
      score[remaining[i]] = rational(rel[remaining[i]].length() - n_int,
				     rel[remaining[i]].length());
    }
    if (trace_level > 1) {
      std::cerr << "initial scores: " << score << std::endl;
    }

    index_type best = score.arg_max();
    assert(best != no_such_index);
    while (score[best] > threshold) {
      if (trace_level > 0) {
	std::cerr << "extending group with set ";
	instance.write_variable_set(std::cerr, old_sets[best]);
	std::cerr << " (discount score = " << score[best] << ")..."
		  << std::endl;
      }
      // construct PDB with discounted ACF
      DiscountACF* d_cost =
	new DiscountACF(cost, discount[best], instance.n_actions());
      RegressionPDB* d_pdb =
	new RegressionPDB(instance, old_sets[best], *d_cost, sinc, inc, stats);
      d_pdb->compute();
      if (trace_level > 2) {
	std::cerr << "new RegressionPDB:" << std::endl;
	d_pdb->write(std::cerr);
	std::cerr << std::endl;
      }
      new_comp->new_component(d_pdb);
      new_set.insert(old_sets[best]);

      // remove best set from remaining
      remaining.subtract(best);
      score[best] = 0;

      // update discounted action sets and scores...
      for (index_type i = 0; i < remaining.length(); i++) {
	interfering_actions(old_sets[best], old_sets[remaining[i]],
			    discount[remaining[i]]);
	index_type n_int =
	  rel[remaining[i]].count_common(discount[remaining[i]]);
	score[remaining[i]] = rational(rel[remaining[i]].length() - n_int,
				       rel[remaining[i]].length());
      }
      if (trace_level > 1) {
	std::cerr << "current scores: " << score << std::endl;
      }

      if (stats.break_signal_raised()) return;
      best = score.arg_max();
      assert(best != no_such_index);
    }

    new_component(new_comp);
    sets.append(new_set);
  }

  if (trace_level > 1) {
    std::cerr << "New Partitioning: " << sets << std::endl;
  }
}

bool PDBHeuristic::additive
(const index_set& vset0, const index_set& vset1) const
{
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].post.defines_any(vset0) &&
	instance.actions[k].post.defines_any(vset1))
      return false;
  return true;
}

void PDBHeuristic::relevant_actions
(const index_set& vars, index_set& acts) const
{
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].post.defines_any(vars)) acts.insert(k);
}

void PDBHeuristic::interfering_actions
(const index_set& vset0, const index_set& vset1, index_set& acts) const
{
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].post.defines_any(vset0) &&
	instance.actions[k].post.defines_any(vset1))
      acts.insert(k);
}

bool PDBHeuristic::sum_is_dominated
(const index_set &dominatee,
 const index_set &dominator,
 const graph& g_subset) const
{
  // Criterion: An admissible sum P_1 + ... + P_m is dominated by
  // an admissible sum P'_1 + ... + P'_n iff each P_i is a subset
  // of sum P'_j -- in other words, P_i must have a neigbour in the
  // pattern inclusion graph that appears in the second sum.
  for (index_type i = 0; i < dominatee.length(); i++)
    if (!g_subset.adjacent(dominatee[i], dominator))
      return false;
  return true;
}

void PDBHeuristic::find_maximal_additive_groups
(index_set_vec& groups) const
{
  // construct additivity graph
  graph g_add(sets.length());
  for (index_type i = 0; i < sets.length(); i++)
    for (index_type j = i + 1; j < sets.length(); j++)
      if (additive(sets[i], sets[j])) g_add.add_undirected_edge(i, j);

  // construct subset graph
  graph g_subset(sets.length());
  for (index_type i = 0; i < sets.length(); i++)
    for (index_type j = 0; j < sets.length(); j++)
      if (sets[j].contains(sets[i])) g_subset.add_edge(i, j);

  groups.clear();
  g_add.all_nondominated_cliques(groups);
  // groups now represents all maximal admissible sums.
  // Now prune the collection to remove dominated elements.
  // NOTE: Use int to avoid problems when computing 0 - 1.
  for (int i = int(groups.length()) - 1; i >= 0; i--) {
    const index_set &this_sum = groups[i];
    for (index_type j = 0; j < groups.length(); j++) {
      if (i != j && sum_is_dominated(this_sum, groups[j], g_subset)) {
        index_type back = groups.length() - 1;
        groups.remove(i);
        break;
      }
    }
  }

  // OLD VERSION:
//   graph indep(sets.length());
//   for (index_type i = 0; i < sets.length(); i++)
//     for (index_type j = i + 1; j < sets.length(); j++)
//       if (additive(sets[i], sets[j])) indep.add_undirected_edge(i, j);
//
//   groups.clear();
//
//   index_set clique;
//   indep.maximal_clique(clique);
//   if (trace_level > 1) {
//     std::cerr << "first maximal additive group: " << clique << std::endl;
//   }
//   assert(!clique.empty());
//   groups.append(clique);
//
//   index_set uncovered;
//   uncovered.fill(sets.length());
//   uncovered.subtract(clique);
//   while (!uncovered.empty()) {
//     indep.maximal_clique_including(uncovered[0], clique);
//     if (trace_level > 1) {
//       std::cerr << "next maximal additive group: " << clique << std::endl;
//     }
//     assert(!clique.empty());
//     groups.append(clique);
//     uncovered.subtract(clique);
//   }
}

class variable_domain_size_increasing : public index_vec::order {
  const index_vec& signature;
 public:
  variable_domain_size_increasing(const index_vec& sign) : signature(sign) { };
  virtual bool operator()(const index_type& v0, const index_type& v1) const;
};

bool variable_domain_size_increasing::operator()
(const index_type& v0, const index_type& v1) const
{
  return (signature[v0] > signature[v1]);
}

void PDBHeuristic::pack_best_fit
(const index_set& vars, index_type bin_size, const SetSizeFunction& set_size,
 index_set_vec& bins) const
{
  index_type first_bin = bins.length();

  index_vec sorted_vars(no_such_index, 0);
  variable_domain_size_increasing order_op(instance.signature);
  sorted_vars.insert_ordered(vars, order_op);

  for (index_type k = 0; k < sorted_vars.length(); k++) {
    index_type b = no_such_index;
    rational f = 0;
    for (index_type i = first_bin; i < bins.length(); i++) {
      index_set tmp(bins[i]);
      tmp.insert(sorted_vars[k]);
      index_type s = set_size(tmp);
      if (s < bin_size) {
	if (b == no_such_index) { // i == first fit
	  b = i;
	  f = rational(s, bin_size);
	}
	else if (f > rational(s, bin_size)) { // i == better fit
	  b = i;
	  f = rational(s, bin_size);
	}
      }
    }
    if (b == no_such_index) { // no fit
      bins.inc_length();
      bins[bins.length() - 1].assign_singleton(sorted_vars[k]);
    }
    else {
      bins[b].insert(sorted_vars[k]);
    }
  }
}

void PDBHeuristic::pack_first_fit
(const index_set& vars, index_type bin_size, const SetSizeFunction& set_size,
 index_set_vec& bins) const
{
  index_vec sorted_vars(no_such_index, 0);
  variable_domain_size_increasing order_op(instance.signature);
  sorted_vars.insert_ordered(vars, order_op);

  for (index_type k = 0; k < sorted_vars.length(); k++) {
    index_type b = no_such_index;
    for (index_type i = 0; (i < bins.length()) && (b == no_such_index); i++) {
      index_set tmp(bins[i]);
      tmp.insert(sorted_vars[k]);
      index_type s = set_size(tmp);
      if (s < bin_size) b = i;
    }
    if (b == no_such_index) { // no fit
      bins.inc_length();
      bins[bins.length() - 1].assign_singleton(sorted_vars[k]);
    }
    else {
      bins[b].insert(sorted_vars[k]);
    }
  }
}

MinimalPDBPacking::MinimalPDBPacking
(index_type n, const SetSizeFunction& s, index_type b)
  : RecursivePartitionEnumerator(n), set_size(s), bin_size(b)
{
  variables.fill(n);
}

MinimalPDBPacking::MinimalPDBPacking
(const index_set& v, const SetSizeFunction& s, index_type b)
  : RecursivePartitionEnumerator(v.length()),
    variables(v), set_size(s), bin_size(b)
{
  // done
}

void MinimalPDBPacking::solution()
{
  construct(variables);
  bool accept = true;
  for (index_type i = 0; (i < sets.length()) && accept; i++) {
    if (set_size(sets[i]) > bin_size) accept = false;
  }
  if (accept) done = true;
}

bool MinimalPDBPacking::pack()
{
  partition();
  return done;
}

void MinimalPDBPacking::result(index_set_vec& vsets)
{
  vsets.assign_copy(sets);
}

void MaxBinPDB::compute_sets
(index_type pdb_size, const SetSizeFunction& set_size)
{
  stats.start();
  index_set unsorted_vars;
  unsorted_vars.fill(instance.n_variables());
  sets.clear();
  pack_best_fit(unsorted_vars, pdb_size, set_size, sets);
  discount.assign_value(EMPTYSET, sets.length());
  if (trace_level > 0) {
    std::cerr << "Max-Bin Partitioning: " << sets << std::endl;
  }
  stats.stop();
}

bool MaxBinPDB::compute_sets_optimal
(index_type pdb_size, const SetSizeFunction& set_size)
{
  stats.start();
  MinimalPDBPacking mbp(instance.n_variables(), set_size, pdb_size);
  bool poss = mbp.pack();
  if (poss) {
    mbp.result(sets);
    discount.assign_value(EMPTYSET, sets.length());
  }
  if (trace_level > 0) {
    if (poss) {
      std::cerr << "Max-Bin Partitioning: " << sets << std::endl;
    }
    else {
      std::cerr << "error: no Max-Bin Partitioning found for size = "
		<< pdb_size << std::endl;
    }
  }
  stats.stop();
  return poss;
}

void RandomBinPDB::compute_sets
(index_type pdb_size, const SetSizeFunction& set_size,
 RNG& rng, index_type n_swaps)
{
  index_set vars;
  vars.fill(instance.n_variables());
  compute_sets(vars, pdb_size, set_size, rng, n_swaps);
}

void RandomBinPDB::compute_sets
(const index_set& vars, index_type pdb_size, const SetSizeFunction& set_size,
 RNG& rng, index_type n_swaps)
{
  stats.start();
  sets.clear();
  pack_best_fit(vars, pdb_size, set_size, sets);
  if (sets.length() > 1) {
    for (index_type n = 0; n < n_swaps; n++) {
      index_type b0 = rng.random_in_range(sets.length());
      index_type b1 = rng.random_in_range(sets.length(), b0);
      index_type v0 = rng.random_in_range(sets[b0].length());
      index_type v1 = rng.random_in_range(sets[b1].length());
      index_set new0(sets[b0]);
      new0.subtract(sets[b0][v0]);
      new0.insert(sets[b1][v1]);
      index_set new1(sets[b1]);
      new1.subtract(sets[b1][v1]);
      new1.insert(sets[b0][v0]);
      if ((set_size(new0) < pdb_size) && (set_size(new1) < pdb_size)) {
	sets[b0] = new0;
	sets[b1] = new1;
      }
    }
  }
  discount.assign_value(EMPTYSET, sets.length());
  if (trace_level > 0) {
    std::cerr << "Random Bin Partitioning: " << sets << std::endl;
  }
  stats.stop();
}

void RandomIndependentBinPDB::compute_sets
(index_type pdb_size, const SetSizeFunction& set_size,
 bool select_spanning_subset, bool collapse_small_bins,
 RNG& rng, index_type n_swaps)
{
  stats.start();
  if (trace_level > 0) {
    std::cerr << "computing independent variable sets..." << std::endl;
  }
  IndependentVariables iv(instance);
  iv.compute_approximate_independent_sets();
  if (select_spanning_subset) {
    iv.compute_spanning_sets();
  }
  sets.clear();
  index_set small;
  index_set ex_from_small;

  for (index_type k = 0; k < iv.n_sets(); k++) {
    if (trace_level > 0) {
      std::cerr << "processing ";
      instance.write_variable_set(std::cerr, iv.set(k));
      std::cerr << " (size = " << set_size(iv.set(k)) << ")..."
		<< std::endl;
    }
    if ((set_size(iv.set(k)) < pdb_size) && collapse_small_bins) {
      small.insert(iv.set(k));
    }
    else {
      index_set_vec bins;
      pack_best_fit(iv.set(k), pdb_size, set_size, bins);
      if (bins.length() > 1) {
	for (index_type n = 0; n < n_swaps; n++) {
	  index_type b0 = rng.random_in_range(bins.length());
	  index_type b1 = rng.random_in_range(bins.length(), b0);
	  index_type v0 = rng.random_in_range(bins[b0].length());
	  index_type v1 = rng.random_in_range(bins[b1].length());
	  index_set new0(bins[b0]);
	  new0.subtract(bins[b0][v0]);
	  new0.insert(bins[b1][v1]);
	  index_set new1(bins[b1]);
	  new1.subtract(bins[b1][v1]);
	  new1.insert(bins[b0][v0]);
	  if ((set_size(new0) < pdb_size) && (set_size(new1) < pdb_size)) {
	    bins[b0] = new0;
	    bins[b1] = new1;
	  }
	}
      }
      if (trace_level > 0) {
	std::cerr << bins.length() << " sets created:" << std::endl;
	for (index_type i = 0; i < bins.length(); i++) {
	  instance.write_variable_set(std::cerr, bins[i]);
	  std::cerr << " (size = " << set_size(bins[i]) << ")" << std::endl;
	}
      }
      for (index_type i = 0; i < bins.length(); i++)
	sets.append(bins[i]);
      ex_from_small.insert(iv.set(k));
    }
  }

  small.subtract(ex_from_small);
  if (!small.empty()) {
    if (trace_level > 0) {
      std::cerr << "collapsed small sets: ";
      instance.write_variable_set(std::cerr, small);
      std::cerr << "..." << std::endl;
    }
    index_set_vec bins;
    pack_best_fit(small, pdb_size, set_size, bins);
    if (bins.length() > 1) {
      for (index_type n = 0; n < n_swaps; n++) {
	index_type b0 = rng.random_in_range(bins.length());
	index_type b1 = rng.random_in_range(bins.length(), b0);
	index_type v0 = rng.random_in_range(bins[b0].length());
	index_type v1 = rng.random_in_range(bins[b1].length());
	index_set new0(bins[b0]);
	new0.subtract(bins[b0][v0]);
	new0.insert(bins[b1][v1]);
	index_set new1(bins[b1]);
	new1.subtract(bins[b1][v1]);
	new1.insert(bins[b0][v0]);
	if ((set_size(new0) < pdb_size) && (set_size(new1) < pdb_size)) {
	  bins[b0] = new0;
	  bins[b1] = new1;
	}
      }
    }
    if (trace_level > 0) {
      std::cerr << bins.length() << " sets created:" << std::endl;
      for (index_type i = 0; i < bins.length(); i++) {
	instance.write_variable_set(std::cerr, bins[i]);
	std::cerr << " (size = " << set_size(bins[i]) << ")" << std::endl;
      }
    }
    for (index_type i = 0; i < bins.length(); i++)
      sets.append(bins[i]);
  }
  discount.assign_value(EMPTYSET, sets.length());
  stats.stop();
}

void Max1PDB::compute_sets()
{
  sets.set_length(instance.n_variables());
  for (index_type k = 0; k < instance.n_variables(); k++)
    sets[k].assign_singleton(k);
}

void Max1PDB::compute_sets(const index_set& vars)
{
  sets.set_length(vars.length());
  for (index_type k = 0; k < vars.length(); k++) {
    assert(vars[k] < instance.n_variables());
    sets[k].assign_singleton(vars[k]);
  }
}

void MaxCGPDB::compute_sets
(index_type pdb_size, const SetSizeFunction& set_size)
{
  stats.start();

  for (index_type k = 0; k < instance.n_variables(); k++) {
    index_set vars(instance.causal_graph.successors(k));
    vars.insert(k);
    index_type s = set_size(vars);
    if (trace_level > 1) {
      std::cerr << "CG set for variable " << k << ": "
		<< instance.causal_graph.successors(k)
		<< " (size = " << s << ")" << std::endl;
    }
    index_set_vec subsets;
    pack_best_fit(instance.causal_graph.successors(k),
		  pdb_size/(instance.variables[k].n_values() + 1),
		  set_size,
		  subsets);
    for (index_type i = 0; i < subsets.length(); i++) {
      subsets[i].insert(k);
      sets.append(subsets[i]);
      if (trace_level > 1) {
	std::cerr << "subset " << subsets[i] << " (size = "
		  << set_size(subsets[i]) << ") added" << std::endl;
      }
    }
  }
  remove_subsets();

  if (trace_level > 0) {
    std::cerr << "CG Partitioning: " << sets << std::endl;
  }
  stats.stop();
}

void MaxInterferencePDB::compute_sets
(index_type pdb_size, const SetSizeFunction& set_size)
{
  stats.start();
  sets.clear();

  for (index_type k = 0; k < instance.n_variables(); k++) {
    index_set vars(instance.causal_graph.predecessors(k));
    vars.insert(k);
    index_type s = set_size(vars);
    if (trace_level > 1) {
      std::cerr << "inverse CG set for variable " << k << ": "
		<< instance.causal_graph.predecessors(k)
		<< " (size = " << s << ")" << std::endl;
    }
    index_set_vec subsets;
    pack_best_fit(instance.causal_graph.predecessors(k),
		  pdb_size/(instance.variables[k].n_values() + 1),
		  set_size,
		  subsets);
    for (index_type i = 0; i < subsets.length(); i++) {
      subsets[i].insert(k);
      sets.append(subsets[i]);
      if (trace_level > 1) {
	std::cerr << "subset " << subsets[i] << " (size = "
		  << set_size(subsets[i]) << ") added" << std::endl;
      }
    }
  }
  remove_subsets();

  if (trace_level > 0) {
    std::cerr << "Inverse CG Partitioning: " << sets << std::endl;
  }
  discount.assign_value(EMPTYSET, sets.length());
  stats.stop();
}

void MaxInterferencePDB::compute_reduced_sets
(index_type pdb_size, const SetSizeFunction& set_size)
{
  stats.start();
  sets.clear();

  IndependentVariables iv(instance);
  iv.compute_approximate_independent_sets();
  iv.compute_spanning_sets();

  const index_set& spv(iv.spanning_variables());

  for (index_type k = 0; k < spv.length(); k++) {
    index_set vars(instance.causal_graph.predecessors(spv[k]));
    vars.intersect(spv);
    vars.insert(spv[k]);
    index_type s = set_size(vars);
    if (trace_level > 1) {
      std::cerr << "reduced inverse CG set for variable "
		<< spv[k] << ": " << vars << " (size = " << s << ")"
		<< std::endl;
    }
    index_set_vec subsets;
    pack_best_fit(vars,
		  pdb_size/(instance.variables[k].n_values() + 1),
		  set_size,
		  subsets);
    for (index_type i = 0; i < subsets.length(); i++) {
      subsets[i].insert(spv[k]);
      sets.append(subsets[i]);
      if (trace_level > 1) {
	std::cerr << "subset " << subsets[i] << " (size = "
		  << set_size(subsets[i]) << ") added" << std::endl;
      }
    }
  }
  remove_subsets();

  if (trace_level > 0) {
    std::cerr << "Inverse CG Partitioning: " << sets << std::endl;
  }
  discount.assign_value(EMPTYSET, sets.length());
  stats.stop();
}

void IndependentPairPDB::compute_sets()
{
  stats.start();
  sets.clear();
  groups.clear();

  if (trace_level > 0) {
    std::cerr << "computing independent variable sets..." << std::endl;
  }
  instance.independence_graph.maximal_clique_cover(independent_variable_sets);
  first.assign_value(0,independent_variable_sets.length());
  index_set covered;

  if (trace_level > 0) {
    std::cerr << "creating variable pairs..." << std::endl;
  }
  for (index_type k = 0; k < independent_variable_sets.length(); k++) {
    index_set& v_set = independent_variable_sets[k];
    first[k] = sets.length();
    if (!covered.contains(v_set)) {
      if (trace_level > 1) {
	std::cerr << "next set: ";
	instance.write_variable_set(std::cerr, v_set);
	std::cerr << std::endl;
      }
      covered.insert(v_set);
      for (index_type i = 0; i < instance.n_variables(); i++)
	if (!covered.contains(i))
	  if (instance.weak_determined_check(v_set, i)) {
	    if (trace_level > 1) {
	      std::cerr << "adding determined variable ";
	      instance.variables[i].name->write(std::cerr, Name::NC_INSTANCE);
	      std::cerr << " to covered set" << std::endl;
	    }
	    covered.insert(i);
	  }
      if (v_set.length() > 1) {
	n_pairs[k] = (v_set.length() * (v_set.length() - 1)) / 2;
	for (index_type i = 0; i < v_set.length(); i++)
	  for (index_type j = i + 1; j < v_set.length(); j++) {
	    index_set& s_new = sets.append();
	    s_new.clear();
	    s_new.insert(v_set[i]);
	    s_new.insert(v_set[j]);
	    if (trace_level > 1) {
	      std::cerr << "created pair ";
	      instance.write_variable_set(std::cerr, sets[sets.length() - 1]);
	      std::cerr << std::endl;
	    }
	  }
      }
      else {
	n_pairs[k] = 0;
	index_set& s_new = sets.append();
	s_new.assign_copy(v_set);
	if (trace_level > 1) {
	  std::cerr << "created singleton ";
	  instance.write_variable_set(std::cerr, sets[sets.length() - 1]);
	  std::cerr << std::endl;
	}
      }
    }
    else {
      if (trace_level > 1) {
	std::cerr << "skipping set ";
	instance.write_variable_set(std::cerr, v_set);
	std::cerr << " since it is already covered" << std::endl;
      }
      independent_variable_sets[k].clear();
    }
  }

  independent_variable_sets.remove_empty_sets();

  discount.assign_value(EMPTYSET, sets.length());
  stats.stop();
}

index_type IndependentPairPDB::pair_index
(index_type set, index_type v0, index_type v1)
{
  // sum {i=0,..,k-1} (n - (i+1))
  // = k*n - sum {i=0..k-1} i+1
  // = k*n - (k + (k*k-1)/2)
  assert(v0 != v1);
  if (v0 > v1) {
    index_type t = v0;
    v0 = v1;
    v1 = t;
  }
  index_type n = independent_variable_sets[set].length();
  index_type b = (v0*n) - (v0 + ((v0 * (v0 - 1)) / 2));
  index_type f = v1 - (v0 + 1);
//   std::cerr << "index of " << v0 << ", " << v1 << " in set " << set
// 	    << " = " << first[set] << " + " << b << " + " << f
// 	    << " = " << first[set] + b + f << std::endl;
  return first[set] + b + f;
}

void IndependentPairPDB::compute_all_additive_groups
(index_type set, bool_vec& unassigned, index_set& pairs)
{
  index_type first_v = no_such_index;
  index_type next_v = no_such_index;
  index_type n = independent_variable_sets[set].length();
  for (index_type k = 0; (k < n) && (next_v == no_such_index); k++)
    if (unassigned[k]) {
      if (first_v == no_such_index) first_v = k;
      else next_v = k;
    }
  if (next_v == no_such_index) {
    // std::cerr << " - group " << pairs << " added" << std::endl;
    groups.append(pairs);
  }
  else {
    unassigned[first_v] = false;
    for (index_type k = 0; k < n; k++) if ((k != first_v) && unassigned[k]) {
      unassigned[k] = false;
      index_type p = pair_index(set, first_v, k);
      pairs.insert(p);
      compute_all_additive_groups(set, unassigned, pairs);
      unassigned[k] = true;
      pairs.subtract(p);
    }
    unassigned[first_v] = true;
  }
}

void IndependentPairPDB::compute_all_additive_groups()
{
  for (index_type k = 0; k < independent_variable_sets.length(); k++)
    if (n_pairs[k] > 0) {
//       std::cerr << "computing groups for set "
// 		<< independent_variable_sets[k] << "..." << std::endl;
//       std::cerr << "pairs:";
//       for (index_type i = 0; i < n_pairs[k]; i++)
// 	std::cerr << " (" << first[k] + i << ")" << sets[first[k] + i];
//       std::cerr << " (" << n_pairs[k] << ")" << std::endl;
      bool_vec unassigned(true, independent_variable_sets[k].length());
      index_set pairs;
      compute_all_additive_groups(k, unassigned, pairs);
    }
}

bool IndependentPairPDB::ok_to_add(index_type s, index_type g)
{
  for (index_type i = 0; i < groups[g].length(); i++)
    if (sets[groups[g][i]].count_common(sets[s]) > 0)
      return false;
  return true;
}

bool IndependentPairPDB::find_assignment
(index_type g0, index_type g_end, index_type g_size,
 index_type s0, index_type s_end,
 bool_vec& assigned, index_type cur_g)
{
  if (groups[cur_g].length() == g_size) return true;
  for (index_type k = s0; k < s_end; k++) if (!assigned[k - s0]) {
    if (ok_to_add(k, cur_g)) {
      groups[cur_g].insert(k);
      assigned[k - s0] = true;
      index_type next_g = cur_g + 1;
      if (next_g == g_end) next_g = g0;
      if (find_assignment(g0, g_end, g_size, s0, s_end, assigned, next_g))
	return true;
      groups[cur_g].subtract(k);
      assigned[k - s0] = false;
    }
  }
  return false;
}

void IndependentPairPDB::compute_min_additive_groups()
{
  for (index_type k = 0; k < independent_variable_sets.length(); k++) {
    index_type n = independent_variable_sets[k].length();
    if (n > 3) {
      index_type n_groups = ((n % 2) == 0 ? n - 1 : n);
      index_type n_per_group = ((n % 2) == 0 ? n / 2 : (n - 1) / 2);
      index_type first_group = groups.length();
      groups.set_length(first_group + n_groups);
      for (index_type i = 0; i < n_groups; i++)
	groups[first_group + i].assign_singleton(first[k] + i);
      bool_vec assigned(false, n_pairs[k] - n_groups);
      bool ok = find_assignment(first_group,
				first_group + n_groups,
				n_per_group,
				first[k] + n_groups,
				first[k] + n_pairs[k],
				assigned,
				first_group);
      if (!ok) {
	std::cerr << "error: failed to find group assignment" << std::endl;
      }
    }
    else {
      // no groups possible - make singleton groups
      for (index_type i = 0; i < n_pairs[k]; i++) {
	index_set& g_new = groups.append();
	g_new.assign_singleton(first[k] + i);
      }
    }
  }
}

void IndependentPairPDB::compute_additive_groups(bool maximal)
{
  for (index_type k = 0; k < independent_variable_sets.length(); k++) {
    index_type n = independent_variable_sets[k].length();
    if (n > 3) {
      index_type n_groups = ((n % 2) == 0 ? n - 1 : n);
      index_type n_per_group = ((n % 2) == 0 ? n / 2 : (n - 1) / 2);
      index_type first_group = groups.length();
      bool_vec assigned(false, n_pairs[k]);
      index_type next_u = 0;
      while (next_u != no_such_index) {
	index_set& g_new = groups.append();
	g_new.assign_singleton(first[k] + next_u);
	assigned[next_u] = true;
	while (g_new.length() < n_per_group) {
	  index_type cur_in_group = g_new.length();
	  for (index_type i = 0; (i < n_pairs[k]) && (g_new.length() < n_per_group); i++)
	    if (!assigned[i] && ok_to_add(first[k] + i, groups.length())) {
	      g_new.insert(first[k] + i);
	      assigned[i] = true;
	    }
	  for (index_type i = 0; (i < n_pairs[k]) && (g_new.length() < n_per_group); i++)
	    if (assigned[i] && ok_to_add(first[k] + i, groups.length())) {
	      g_new.insert(first[k] + i);
	      assigned[i] = true;
	    }
	  if (g_new.length() == cur_in_group) {
	    std::cerr << "error: can't add more sets to group! (size = "
		      << g_new.length() << ", n/group = " << n_per_group
		      << ")" << std::endl;
	    exit(255);
	  }
	}
	next_u = no_such_index;
	for (index_type i = 0; (i<n_pairs[k])&&(next_u==no_such_index); i++)
	  if (!assigned[i]) next_u = i;
      }
      if (trace_level > 0) {
	std::cerr << groups.length() - first_group
		  << " groups created (min is " << n_groups << ")"
		  << std::endl;
      }
    }
    else {
      // no groups possible - make singleton groups
      for (index_type i = 0; i < n_pairs[k]; i++) {
	groups.append().assign_singleton(first[k] + i);
      }
      if (n_pairs[k] == 0) {
	groups.append().assign_singleton(first[k]);
      }
    }
  }
}

void IndependentBinPDB::compute_sets
(index_type pdb_size, const SetSizeFunction& set_size)
{
  stats.start();
  sets.clear();

  if (trace_level > 0) {
    std::cerr << "computing independent variable sets..." << std::endl;
  }
  index_set_vec ind_sets;
  instance.independence_graph.maximal_clique_cover(ind_sets);

  if (trace_level > 0) {
    std::cerr << "packing independent variables in bins..." << std::endl;
  }
  for (index_type k = 0; k < ind_sets.length(); k++) {
    index_set_vec bins;
    pack_best_fit(ind_sets[k], pdb_size, set_size, bins);
    for (index_type i = 0; i < bins.length(); i++) {
      if (trace_level > 1) {
	std::cerr << "creating PDB from set ";
	instance.write_variable_set(std::cerr, sets[sets.length() - 1]);
	std::cerr << " (size = " << set_size(bins[i]) << ")" << std::endl;
      }
      sets.append(bins[i]);
    }
  }

  discount.assign_value(EMPTYSET, sets.length());
  stats.stop();
}

void IndependentBinPDB::compute_sets_optimal
(index_type pdb_size, const SetSizeFunction& set_size)
{
  stats.start();
  sets.clear();

  if (trace_level > 0) {
    std::cerr << "computing independent variable sets..." << std::endl;
  }
  index_set_vec independent_sets;
  instance.independence_graph.maximal_clique_cover(independent_sets);

  if (trace_level > 0) {
    std::cerr << "packing independent variables in bins..." << std::endl;
  }
  for (index_type k = 0; k < independent_sets.length(); k++) {
    index_set_vec bins;
    MinimalPDBPacking mbp(independent_sets[k], set_size, pdb_size);
    bool poss = mbp.pack();
    if (poss) {
      mbp.result(bins);
    }
    else {
      std::cerr << "warning: MinimalPDBPacking failed for set "
		<< independent_sets[k] << " and size " << pdb_size
		<< ", resorting to heuristic packing..." << std::endl;
      pack_best_fit(independent_sets[k], pdb_size, set_size, bins);
    }
    for (index_type i = 0; i < bins.length(); i++) {
      sets.append(bins[i]);
      if (trace_level > 1) {
	std::cerr << "created set ";
	instance.write_variable_set(std::cerr, bins[i]);
	std::cerr << std::endl;
      }
    }
  }

  discount.assign_value(EMPTYSET, sets.length());
  stats.stop();
}

rational SetValueSum::operator()(const index_set& set) const
{
  rational r0 = svf0(set);
  rational r1 = svf1(set);
  rational w0 = safemul(weight0, r0);
  rational w1 = safemul(weight1, r1);
  rational res = safeadd(w0, w1);
  return res;
}

rational InitialStateInterference::operator()(const index_set& set) const
{
  rational val = 0;
  for (index_type i = 0; i < set.length(); i++)
    for (index_type j = i + 1; j < set.length(); j++) {
      rational r =
	instance.interference_ratio
	(set[i], instance.init_state.value_of(set[i]),
	 set[j], instance.init_state.value_of(set[j]));
      val = safeadd(val, r);
    }
  return val;
}

rational GoalStateInterference::operator()(const index_set& set) const
{
  rational val = 0;
  for (index_type i = 0; i < set.length(); i++)
    if (instance.goal_state.value_of(set[i]) != no_such_index)
      for (index_type j = i + 1; j < set.length(); j++)
	if (instance.goal_state.value_of(set[j]) != no_such_index) {
	  rational r =
	    instance.interference_ratio
	    (set[i], instance.goal_state.value_of(set[i]),
	     set[j], instance.goal_state.value_of(set[j]));
	  assert(r >= 0);
	  val = safeadd(val, r);
	  assert(val >= 0);
	}
  return val;
}

rational AffectingActionFraction::operator()(const index_set& set) const
{
  index_type n_aa = 0;
  for (index_type k = 0; k < instance.n_actions(); k++)
    if (instance.actions[k].post.defines_any(set)) n_aa += 1;
  return rational(1 - n_aa, instance.n_actions());
}

rational InverseCGFraction::operator()(const index_set& set) const
{
  rational val = 0;
  for (index_type i = 0; i < set.length(); i++) {
    index_type n =
      instance.causal_graph.predecessors(set[i]).length();
    if (n > 0) {
      index_type n_in =
	instance.causal_graph.predecessors(set[i]).count_common(set);
      rational d(n_in*n_in, n*n);
//       std::cerr << "n = " << n << ", n_in = " << n_in
// 		<< ", n^2 = " << n*n << ", n_in^2 = " << n_in * n_in
// 		<< ", d = " << d << std::endl;
      assert(d >= 0);
//       std::cerr << "val = " << val << ", val.round = " << val.round()
// 		<< ", d.round = " << d.round()
// 		<< ", val.round() + d.round = " << (val.round() + d.round())
// 		<< std::endl;
      val = safeadd(val, d);
      assert(val >= 0);
    }
  }
  return val;
}

rational CombinedSetValue::operator()(const index_set& set) const
{
  return safeadd(f_isi(set), f_icg(set));
}

MaxWeightedPDBPacking::MaxWeightedPDBPacking
(const index_set& v, const SetSizeFunction& s, index_type b,
 const SetValueFunction& f)
  : RecursivePartitionEnumerator(v.length()), variables(v),
    set_size(s), bin_size(b), value(f), solved(false)
{
  // done
}

void MaxWeightedPDBPacking::solution()
{
  construct(variables);
  bool accept = true;
  for (index_type i = 0; (i < sets.length()) && accept; i++) {
    if (set_size(sets[i]) > bin_size)
      accept = false;
  }
  if (!accept) return;
  rational val = 0;
  for (index_type i = 0; i < sets.length(); i++) 
    val = (val + value(sets[i]).round());
  if ((val > best_value) || !solved) {
    best.assign_copy(sets);
    best_value = val;
    solved = true;
  }
}

bool MaxWeightedPDBPacking::pack()
{
  best_value = 0;
  solved = false;
  partition();
  return solved;
}

index_type WeightedBinPDB::max_size_fit
(const index_set& vars, const SetSizeFunction& set_size, index_type capacity)
{
  index_type i_max = no_such_index;
  index_type s_max = 0;
  for (index_type k = 0; k < vars.length(); k++) {
    index_set v;
    v.assign_singleton(vars[k]);
    index_type s = set_size(v);
    if (s <= capacity) {
      if ((s > s_max) || (i_max == no_such_index)) {
	i_max = vars[k];
	s_max = s;
      }
    }
  }
  return i_max;
}

index_type WeightedBinPDB::best_value_fit
(const index_set& vars,
 const SetSizeFunction& set_size,
 index_type capacity,
 const index_set& sack)
{
  index_type i_best = no_such_index;
  rational v_best = 0;
  for (index_type k = 0; k < vars.length(); k++) {
    index_set tmp(sack);
    tmp.insert(vars[k]);
    rational v = value(tmp);
    if (set_size(tmp) <= capacity) {
      if ((v > v_best) || (i_best == no_such_index)) {
	i_best = vars[k];
	v_best = v;
      }
    }
  }
  return i_best;
}

void WeightedBinPDB::single_knapsack
(index_set& vars,
 const SetSizeFunction& set_size,
 index_type capacity,
 index_set& sack)
{
  sack.clear();
  index_type next = max_size_fit(vars, set_size, capacity);
  while (next != no_such_index) {
    vars.subtract(next);
    sack.insert(next);
    next = best_value_fit(vars, set_size, capacity, sack);
  }
}

void WeightedBinPDB::multi_knapsack
(const index_set& invars,
 const SetSizeFunction& set_size,
 index_type capacity,
 index_set_vec& sacks)
{
  index_set vars(invars);
  while (!vars.empty()) {
    index_set& s_new = sacks.append();
    single_knapsack(vars, set_size, capacity, s_new);
    if (s_new.empty()) {
      std::cerr << "error: set ";
      instance.write_variable_set(std::cerr, vars);
      std::cerr << " can not be packed with bin size "
		<< capacity << std::endl;
      exit(255);
    }
  }
}

void WeightedBinPDB::local_search_swap
(const SetSizeFunction& set_size,
 index_type pdb_size,
 index_type set0,
 index_type setn)
{
  for (index_type b0 = set0; b0 < setn; b0++) {
    rational v0 = value(sets[b0]);
    assert(v0 >= 0);
    for (index_type b1 = b0 + 1; b1 < setn; b1++) {
      rational v1 = value(sets[b1]);
      assert(v1 >= 0);
      bool done = false;
      count_type n_swaps = 0;
      while (!done && (n_swaps < MAX_SWAPS)) {
	done = true;
	for (index_type e0 = 0; (e0 < sets[b0].length()) && done; e0++)
	  for (index_type e1 = 0; (e1 < sets[b1].length()) && done; e1++) {
	    index_set new0(sets[b0]);
	    new0.subtract(sets[b0][e0]);
	    new0.insert(sets[b1][e1]);
	    if (set_size(new0) < pdb_size) {
	      index_set new1(sets[b1]);
	      new1.subtract(sets[b1][e1]);
	      new1.insert(sets[b0][e0]);
	      if (set_size(new1) < pdb_size) {
		rational new_v0 = value(new0);
		assert(new_v0 >= 0);
		rational new_v1 = value(new1);
		assert(new_v1 >= 0);
		rational imp = safeadd(safeadd(new_v0, new_v1),
				       safeadd(v0, v1) * -1);
		if (imp > 0) {
		  if (trace_level > 2) {
		    std::cerr << "swapping ";
		    instance.variables[sets[b0][e0]].name->
		      write(std::cerr, true);
		    std::cerr << " and ";
		    instance.variables[sets[b1][e1]].name->
		      write(std::cerr, true);
		    std::cerr << " for an improvement of " << imp
			      << " (" << (new_v0 + new_v1)
			      << ", " << (v0 + v1) << ")..."
			      << std::endl;
		  }
		  sets[b0] = new0;
		  sets[b1] = new1;
		  v0 = new_v0;
		  v1 = new_v1;
		  done = false;
		  n_swaps += 1;
		}
	      }
	    }
	  }
      }
    }
  }
}

void WeightedBinPDB::local_search_move
(const SetSizeFunction& set_size,
 index_type pdb_size,
 index_type set0,
 index_type setn)
{
  for (index_type b0 = set0; b0 < setn; b0++) {
    rational v0 = value(sets[b0]);
    for (index_type b1 = b0 + 1; b1 < setn; b1++) {
      rational v1 = value(sets[b1]);
      bool done = false;
      count_type n_swaps = 0;
      while (!done && (n_swaps < MAX_SWAPS)) {
	done = true;
	for (index_type e0 = 0; (e0 < sets[b0].length()) && done; e0++) {
	  index_set new1(sets[b1]);
	  new1.insert(sets[b0][e0]);
	  if (set_size(new1) < pdb_size) {
	    index_set new0(sets[b0]);
	    new0.subtract(sets[b0][e0]);
	    rational new_v0 = value(new0);
	    rational new_v1 = value(new1);
	    if ((new_v0 + new_v1) > (v0 + v1)) {
	      sets[b0] = new0;
	      sets[b1] = new1;
	      v0 = new_v0;
	      v1 = new_v1;
	      done = false;
	      n_swaps += 1;
	    }
	  }
	}
	for (index_type e1 = 0; (e1 < sets[b1].length()) && done; e1++) {
	  index_set new0(sets[b0]);
	  new0.insert(sets[b1][e1]);
	  if (set_size(new0) < pdb_size) {
	    index_set new1(sets[b1]);
	    new1.subtract(sets[b1][e1]);
	    rational new_v0 = value(new0);
	    rational new_v1 = value(new1);
	    if ((new_v0 + new_v1) > (v0 + v1)) {
	      sets[b0] = new0;
	      sets[b1] = new1;
	      v0 = new_v0;
	      v1 = new_v1;
	      done = false;
	      n_swaps += 1;
	    }
	  }
	}
      }
    }
  }
}

void WeightedBinPDB::compute_sets
(const index_set_vec& varsets,
 index_type pdb_size,
 const SetSizeFunction& set_size)
{
  stats.start();
  sets.clear();
  for (index_type k = 0; k < varsets.length(); k++) {
    if (trace_level > 0) {
      std::cerr << "partitioning set ";
      instance.write_variable_set(std::cerr, varsets[k]);
      std::cerr << "..." << std::endl;
    }
    index_type n_before = sets.length();
    multi_knapsack(varsets[k], set_size, pdb_size, sets);
    local_search_swap(set_size, pdb_size, n_before, sets.length());
    if (trace_level > 0) {
      std::cerr << sets.length() - n_before << " sets created:" << std::endl;
      for (index_type i = 0; i < sets.length(); i++) {
	instance.write_variable_set(std::cerr, sets[i]);
	std::cerr << " (size = " << set_size(sets[i])
		  << ", value = "	<< value(sets[i]) << ")" << std::endl;
      }
    }
  }
  discount.assign_value(EMPTYSET, sets.length());
  stats.stop();
}

void WeightedIndependentBinPDB::compute_sets
(const index_set& vars, index_type pdb_size, const SetSizeFunction& set_size,
 bool select_spanning_subset, bool collapse_small_bins)
{
  stats.start();

  if (trace_level > 0) {
    std::cerr << "computing independent variable sets..." << std::endl;
  }
  IndependentVariableSets iv(instance, vars);
  iv.compute_approximate_independent_sets();
  if (select_spanning_subset) {
    iv.compute_spanning_sets();
  }

  if (trace_level > 0) {
    std::cerr << "packing knapsacks..." << std::endl;
  }
  sets.clear();
  index_set small;
  index_set ex_from_small;

  for (index_type k = 0; k < iv.length(); k++) {
    if (trace_level > 0) {
      std::cerr << "processing ";
      instance.write_variable_set(std::cerr, iv[k]);
      std::cerr << " (size = " << set_size(iv[k]) << ")..." << std::endl;
    }
    if ((set_size(iv[k]) < pdb_size) && collapse_small_bins) {
      small.insert(iv[k]);
    }
    else {
      index_set vars(iv[k]);
      index_type n_before = sets.length();
      multi_knapsack(vars, set_size, pdb_size, sets);
      local_search_swap(set_size, pdb_size, n_before, sets.length());
      // local_search_move(set_size, pdb_size, n_before, sets.length());
      if (trace_level > 0) {
	std::cerr << sets.length() - n_before << " sets created:" << std::endl;
	for (index_type i = n_before; i < sets.length(); i++) {
	  instance.write_variable_set(std::cerr, sets[i]);
	  std::cerr << " (size = " << set_size(sets[i])
		    << ", value = " << value(sets[i]) << ")" << std::endl;
	}
      }
      ex_from_small.insert(iv[k]);
    }
  }

  small.subtract(ex_from_small);
  if (!small.empty()) {
    if (trace_level > 0) {
      std::cerr << "collapsed small sets: ";
      instance.write_variable_set(std::cerr, small);
      std::cerr << "..." << std::endl;
    }
    index_type n_before = sets.length();
    multi_knapsack(small, set_size, pdb_size, sets);
    local_search_swap(set_size, pdb_size, n_before, sets.length());
    if (trace_level > 0) {
      std::cerr << sets.length() - n_before << " sets created:" << std::endl;
      for (index_type i = n_before; i < sets.length(); i++) {
	instance.write_variable_set(std::cerr, sets[i]);
	std::cerr << " (size = " << set_size(sets[i])
		  << ", value = " << value(sets[i]) << ")" << std::endl;
      }
    }
  }

  discount.assign_value(EMPTYSET, sets.length());
  stats.stop();
}

void WeightedIndependentBinPDB::compute_sets_optimal
(const index_set& vars, index_type pdb_size, const SetSizeFunction& set_size,
 bool select_spanning_subset, bool collapse_small_bins)
{
  stats.start();

  if (trace_level > 0) {
    std::cerr << "computing independent variable sets..." << std::endl;
  }
  IndependentVariableSets iv(instance, vars);
  iv.compute_maximal_independent_sets();
  if (select_spanning_subset) {
    iv.compute_spanning_sets();
  }

  if (trace_level > 0) {
    std::cerr << "packing knapsacks..." << std::endl;
  }
  sets.clear();
  index_set small;

  for (index_type k = 0; k < iv.length(); k++) {
    if (trace_level > 0) {
      std::cerr << "processing ";
      instance.write_variable_set(std::cerr, iv[k]);
      std::cerr << " (size = " << set_size(iv[k]) << ")..." << std::endl;
    }
    if ((set_size(iv[k]) < pdb_size) && collapse_small_bins) {
      small.insert(iv[k]);
    }
    else {
      MaxWeightedPDBPacking mwp(iv[k], set_size, pdb_size, value);
      if (!mwp.pack()) {
	std::cerr << "error: set ";
	instance.write_variable_set(std::cerr, iv[k]);
	std::cerr << " can not be packed with bin size "
		  << pdb_size << std::endl;
	exit(255);
      }
      if (trace_level > 0) {
	std::cerr << mwp.best.length() << " sets created:" << std::endl;
      }
      for (index_type i = 0; i < mwp.best.length(); i++) {
	if (trace_level > 0) {
	  instance.write_variable_set(std::cerr, mwp.best[i]);
	  std::cerr << " (size = " << set_size(mwp.best[i])
		    << ", value = " << value(mwp.best[i])
		    << ")" << std::endl;
	}
	sets.append(mwp.best[i]);
      }
    }
  }

  if (!small.empty()) {
    if (trace_level > 0) {
      std::cerr << "collapsed small sets: ";
      instance.write_variable_set(std::cerr, small);
      std::cerr << "..." << std::endl;
    }
    MaxWeightedPDBPacking mwp(small, set_size, pdb_size, value);
    if (!mwp.pack()) {
      std::cerr << "error: set ";
      instance.write_variable_set(std::cerr, small);
      std::cerr << " can not be packed with bin size "
		<< pdb_size << std::endl;
      exit(255);
    }
    if (trace_level > 0) {
      std::cerr << mwp.best.length() << " sets created:" << std::endl;
    }
    for (index_type i = 0; i < mwp.best.length(); i++) {
      if (trace_level > 0) {
	instance.write_variable_set(std::cerr, mwp.best[i]);
	std::cerr << " (size = " << set_size(mwp.best[i])
		  << ", value = " << value(mwp.best[i])
		  << ")" << std::endl;
      }
      sets.append(mwp.best[i]);
    }
  }

  discount.assign_value(EMPTYSET, sets.length());
  stats.stop();
}

void AdditiveIncrementalRegressionPDB::analyze_plan
(const index_vec& plan,
 index_type c_first, index_type c_last, index_type c_cur, MDDNode* sinc,
 Heuristic* inc, Heuristic* cw, weighted_graph& rg)
{
  partial_state s(instance.init_state);
  for (index_type p = 0; p < plan.length(); p++) {
    SASInstance::Action& act = instance.actions[plan[p]];
    if (trace_level > 1) {
      std::cerr << p + 1 << ". " << act.name << std::endl;
    }
    for (index_type i = c_first; i < c_last; i++) if (i != c_cur) {
      partial_state s_i(s, sets[i]); // state s restricted to set[i]
      NTYPE c_max = 0;
      if (inc) {
	index_set a_i;
	instance.make_atom_set(s_i, a_i);
	index_set p_i;
	instance.make_atom_set(act.pre, p_i);
	instance.map_to_atom_set(act.prv, p_i);
	for (index_type j = 0; j < p_i.length(); j++)
	  if (INFINITE(inc->incremental_eval(a_i, p_i[j]))) {
	    if (trace_level > 1) {
	      std::cerr << " state restricted to ";
	      instance.write_variable_set(std::cerr, sets[i]);
	      std::cerr << " = ";
	      instance.write_partial_state(std::cerr, s_i);
	      std::cerr << " is inconsistent with atom "
			<< p_i[j] << " of prec(" << act.name << ") = ";
	      instance.write_partial_state(std::cerr, act.pre);
	      std::cerr << std::endl;
	    }
	    if (cw)
	      c_max = MAX(c_max, cw->incremental_eval(a_i, p_i[j]));
	    else
	      c_max = 1;
	  }
      }
      if (c_max < 1) {
	s_i.insert(act.pre); // s_i together with preconditions of act
	s_i.insert(act.prv);
	if (!s_i.consistent()) {
	  if (trace_level > 1) {
	    std::cerr << " state restricted to ";
	    instance.write_variable_set(std::cerr, sets[i]);
	    std::cerr << " + pre(" << act.name << ") = ";
	    instance.write_partial_state(std::cerr, s_i);
	    std::cerr << " is inconsistent" <<  std::endl;
	  }
	  c_max = 1;
	}
      }
      if (c_max > 0) {
	rg.increment_edge_weight(c_cur - c_first, i - c_first, c_max);
      }
    }
    s.assign(act.post);
  }
}

void AdditiveIncrementalRegressionPDB::partition
(const partial_state& goal, index_type pdb_size, const index_set& set,
 bool check_all_abstract_plans, const ACF& cost, MDDNode* sinc, Heuristic* inc,
 Heuristic* cw)
{
  assert(!set.empty());
  if (trace_level > 0) {
    std::cerr << "partitioning ";
    instance.write_variable_set(std::cerr, set);
    std::cerr << " (size = "
	      << RegressionPDBSize::apply(instance.signature, set)
	      << ") w.r.t. goal ";
    instance.write_partial_state(std::cerr, goal);
    std::cerr << std::endl;
  }

  index_type b = n_components();
  for (index_type k = 0; k < set.length(); k++) {
    index_set v;
    v.assign_singleton(set[k]);
    new_component(0, v, EMPTYSET);
  }

  index_type done = false;
  weighted_graph rg(n_components() - b);
  while (!done && !stats.break_signal_raised()) {
    // 1. compute PDBs (with traces) for sets in the current partition, AND
    // 2. analyse traces and create "relationship graph"
    for (index_type k = b; k < n_components(); k++) if (components[k] == 0) {
      if (trace_level > 0) {
	std::cerr << "computing new PDB ";
	instance.write_variable_set(std::cerr, sets[k]);
	std::cerr << "..." << std::endl;
      }
      RegressionPDB* pdb =
	new RegressionPDB(instance, sets[k], cost, sinc, inc, stats);
      // pdb->enable_trace();
      pdb->compute();
      NTYPE c = pdb->eval(goal);
      if (c > 0) {
	if (trace_level > 0) {
	  std::cerr << "PDB ";
	  instance.write_variable_set(std::cerr, sets[k]);
	  std::cerr << " yields cost " << c << ", checking plan/plans..."
		    << std::endl;
	}
	// pdb->write(std::cerr);
	// pdb->check_trace();
	ActionSequenceSet plans;
	if (check_all_abstract_plans)
	  pdb->abstract_plan(goal, plans, index_type_max);
	else
	  pdb->abstract_plan(goal, plans, 1);
	assert(plans.length() > 0);
	for (index_type p = 0; p < plans.length(); p++) {
	  if (trace_level > 1) {
	    std::cerr << "plan " << p << ": " << plans[p] << std::endl;
	  }
	  analyze_plan(plans[p], b, n_components(), k, sinc, inc, cw, rg);
	}
      }
      else if (trace_level > 1) {
	std::cerr << "PDB ";
	instance.write_variable_set(std::cerr, sets[k]);
	std::cerr << " yields cost 0" << std::endl;
      }
      components[k] = pdb;
    }

    if (trace_level > 1) {
      std::cerr << "current sets:" << std::endl;
      for (index_type k = b; k < n_components(); k++) {
	std::cerr << " ";
	instance.write_variable_set(std::cerr, sets[k]);
	std::cerr << std::endl;
	for (index_type i = b; i < n_components(); i++)
	  if (rg.adjacent(k - b, i - b)) {
	    std::cerr << " -> ";
	    instance.write_variable_set(std::cerr, sets[i]);
	    std::cerr << " (" << rg.weight(k - b, i - b) << ")" << std::endl;
	  }
      }
    }

    // 3. combine sets as indicated by relationship graph...
    done = true;
    weighted_graph rg_copy(rg);
    while (done && !rg_copy.empty()) {
      index_pair e = rg_copy.max_weight_edge();
      if (e != index_pair(no_such_index)) {
	rg_copy.remove_edge(e.first, e.second);
	e.sort_ascending();
	index_set new_set(sets[e.first + b]);
	new_set.insert(sets[e.second + b]);
	if (RegressionPDBSize::apply(instance.signature, new_set)<=pdb_size) {
	  if (trace_level > 1) {
	    std::cerr << "merging ";
	    instance.write_variable_set(std::cerr, sets[e.first + b]);
	    std::cerr << " and ";
	    instance.write_variable_set(std::cerr, sets[e.second + b]);
	    std::cerr << std::endl;
	  }
	  sets[e.first + b].insert(sets[e.second + b]);
	  sets.remove(e.second + b);
	  if (trace_level > 2) {
	    std::cerr << "1st PDB to be deleted: " << std::endl;
	    ((RegressionPDB*)components[e.first + b])->write(std::cerr);
	  }
	  delete components[e.first + b];
	  components[e.first + b] = 0;
	  if (trace_level > 2) {
	    std::cerr << "2nd PDB to be deleted: " << std::endl;
	    ((RegressionPDB*)components[e.second + b])->write(std::cerr);
	  }
	  delete components[e.second + b];
	  components.remove(e.second + b);
	  rg.remove_node(e.second);
	  for (index_type i = 0; i < rg.size(); i++) {
	    if (rg.adjacent(e.first,i)) rg.remove_edge(e.first, i);
	    if (rg.adjacent(i, e.first)) rg.remove_edge(i, e.first);
	  }
	  done = false;
	}
	e = rg.max_weight_edge();
      }
      else {
	assert(rg_copy.empty());
      }
    }
  }
}

void AdditiveIncrementalRegressionPDB::compute_pdbs
(const partial_state& goal, index_type pdb_size,
 bool optimal_additive_sets, bool select_spanning_subset,
 bool collapse_small_sets, bool check_all_abstract_plans,
 const ACF& cost, MDDNode* sinc, Heuristic* inc, Heuristic* cw)
{
  stats.start();

  components.clear();
  sets.clear();
  discount.clear();
  groups.clear();

  if (trace_level > 0) {
    std::cerr << "computing independent variable sets (";
    if (optimal_additive_sets)
      std::cerr << "optimal";
    else
      std::cerr << "non-optimal";
    std::cerr << ")..." << std::endl;
  }
  IndependentVariables iv(instance);
  if (optimal_additive_sets) {
    iv.compute_maximal_independent_sets();
  }
  else {
    iv.compute_approximate_independent_sets();
  }
  if (select_spanning_subset) {
    iv.compute_spanning_sets();
  }
  if (stats.break_signal_raised()) return;

  partial_state x_goal;
  instance.extend_determined(goal, x_goal);

  if (trace_level > 0) {
    std::cerr << "building PDB sets..." << std::endl;
  }
  index_set small;

  for (index_type k = 0; k < iv.n_sets(); k++) {
    if (RegressionPDBSize::apply(instance.signature, iv.set(k)) < pdb_size) {
      if (collapse_small_sets) {
	small.insert(iv.set(k));
      }
      else {
	RegressionPDB* pdb =
	  new RegressionPDB(instance, iv.set(k), cost, sinc, inc, stats);
	pdb->compute();
	index_type c = new_component(pdb, iv.set(k), EMPTYSET);
	groups.append().assign_singleton(c);
      }
    }
    else {
      index_type b = n_components();
      partition(x_goal, pdb_size, iv.set(k), check_all_abstract_plans, cost,
		sinc, inc, cw);
      index_set& g_new = groups.append();
      g_new.clear();
      for (index_type k = b; k < n_components(); k++)
	g_new.insert(k);
    }
    if (stats.break_signal_raised()) return;
  }

  if (!small.empty()) {
    std::cerr << "processing collapsed small sets..." << std::endl;
    if (RegressionPDBSize::apply(instance.signature, small) < pdb_size) {
      RegressionPDB* pdb =
	new RegressionPDB(instance, small, cost, sinc, inc, stats);
      pdb->compute();
      index_type c = new_component(pdb, small, EMPTYSET);
      groups.append().assign_singleton(c);
    }
    else {
      index_type b = n_components();
      partition(x_goal, pdb_size, small, check_all_abstract_plans, cost,
		sinc, inc, cw);
      for (index_type k = b; k < n_components(); k++) {
	groups.append().assign_singleton(k);
      }
    }
  }

  stats.stop();
}

void write_variable_set_label
(std::ostream& to, SASInstance& ins, const index_set& set)
{
  assert(!set.empty());
  for (index_type k = 0; k < set.length(); k++) {
    if (k > 0) to << "_";
    ins.variables[set[k]].name->write(to, Name::NC_INSTANCE);
  }
}

void IncrementalProgressionPDB::compute_sets
(const index_set& vars, index_type pdb_size, index_type set_size,
 const ACF& cost,  MDDNode* sinc, Heuristic* inc,
 index_type n_best, index_type n_samples, RNG& rng)
{
  stats.start();
  if (trace_level > 0) {
    std::cerr << "building PDB sets..." << std::endl;
  }

  components.clear();
  sets.clear();
  discount.clear();
  groups.clear();

  index_set_vec dd_sets;

  graph xcg;
  if (sinc) {
    assert(inc);
    instance.compute_extended_causal_graph(xcg, *inc);
#ifdef TRACE_PRINT_LOTS
    std::cerr << "causal graph:" << std::endl;
    instance.write_variable_digraph(std::cerr, instance.causal_graph, "Causal Graph", true);
    std::cerr << "extended causal graph:" << std::endl;
    instance.write_variable_digraph(std::cerr, xcg, "Extended Causal Graph", true);
#endif
  }
  else {
    xcg.copy(instance.causal_graph);
  }

  // note: g_vars is a set of indices into the vars set
  index_set g_vars;
  for (index_type k = 0; k < vars.length(); k++)
    if (instance.goal_state.defines(vars[k]))
      g_vars.insert(k);
  if (trace_level > 1) {
    std::cerr << g_vars.length() << " goal variables:";
    for (index_type k = 0; k < g_vars.length(); k++)
      std::cerr << " " << instance.variables[vars[g_vars[k]]].name;
    std::cerr << std::endl;
  }
  else if (trace_level > 0) {
    std::cerr << g_vars.length() << " goal variables" << std::endl;
  }
  lvector<ProgressionPDB*> g_pdb(0, g_vars.length());
  for (index_type k = 0; k < g_vars.length(); k++) {
    index_set vset;
    vset.assign_singleton(vars[g_vars[k]]);
    ProgressionPDB* pdb =
      new ProgressionPDB(instance, vset, cost, sinc, inc, stats);
    // std::cerr << "computing PDB for ";
    // instance.write_variable_set(std::cerr, pdb->variables());
    // std::cerr << std::endl;
    pdb->compute();
    if (stats.break_signal_raised()) return;
    // std::cerr << "done (" << stats.time() << " seconds)" << std::endl;
    g_pdb[k] = pdb;
  }
  for (index_type k = 0; k < g_vars.length(); k++) {
    lvector<ProgressionPDB*> w(0, 0);
    index_set vset;
    vset.assign_singleton(vars[g_vars[k]]);
    sets.append(vset);
    w.append(g_pdb[k]);
    index_type n = 0;
    while (n < w.length()) {
      assert(w[n]);
      if (w[n]->variables().length() < set_size) {
#ifdef TRACE_PRINT_LOTS
	std::cerr << "PDB for ";
	instance.write_variable_set(std::cerr, w[n]->variables());
	std::cerr << ":" << std::endl;
	w[n]->write(std::cerr);
	w[n]->write_graph(std::cerr, instance.goal_state);
#endif
	if (trace_level > 1) {
	  std::cerr << "examining extensions of ";
	  instance.write_variable_set(std::cerr, w[n]->variables());
	  std::cerr << " (goal " << k + 1 << " of " << g_vars.length()
		    << ", " << w.length() - n << " on queue)..."
		    << std::endl;
	  std::cerr << "GRAPH (NODE): ";
	  write_variable_set_label(std::cerr, instance, w[n]->variables());
	  std::cerr << " [label=\"";
	  instance.write_variable_set(std::cerr, w[n]->variables());
	  std::cerr << " (" << k + 1 << "/" << g_vars.length()
		    << ", " << w.length() << ")\"];" << std::endl;
	}
	weighted_vec<index_type,NTYPE> ext;
	weighted_vec<index_type,NTYPE> dd_ext;
	for (index_type i = 0; i < vars.length(); i++)
	  if (!w[n]->variables().contains(vars[i]) &&
	      xcg.adjacent(w[n]->variables(), vars[i])) {
	    vset = w[n]->variables();
	    vset.insert(vars[i]);
	    index_type sset =
	      ProgressionPDBSize::apply(instance.signature, vset);
	    if ((sets.first(vset) == no_such_index) &&
		(sset <= pdb_size)) {
#ifdef TRACE_PRINT_LOTS
	      std::cerr << "abstract instance/state space for ";
	      instance.write_variable_set(std::cerr, vset);
	      std::cerr << ":" << std::endl;
	      SASInstance* ai = new SASInstance(instance, vset);
	      ai->write_domain(std::cerr);
	      delete ai;
	      StateAbstraction sa(instance, vset);
	      sa.write_graph(std::cerr, instance.goal_state, sinc, inc);
#endif
	      index_type g_i = g_vars.first(i);
	      NTYPE dd_val;
	      NTYPE v_plus;
	      if (g_i != no_such_index) {
		assert(g_pdb[g_i]->variables().length() == 1);
		assert(g_pdb[g_i]->variables()[0] == vars[i]);
		v_plus = test_merge3(w[n], g_pdb[g_i], cost, sinc, inc,
				     n_samples, dd_val, rng);
	      }
	      else {
		v_plus = test_grow3(w[n], vars[i], cost, sinc, inc, n_samples,
				    dd_val, rng);
	      }
	      if (INFINITE(v_plus)) {
		std::cerr << "pattern ";
		instance.write_variable_set(std::cerr, vset);
		std::cerr << " detects deadlock in initial state!"
			  << std::endl;
		sets.clear();
		sets.append(vset);
		return;
	      }
	      if (stats.break_signal_raised()) return;
	      if (v_plus > 0) {
		ext.insert_decreasing(vars[i], v_plus);
	      }
	      if (dd_val > 0) {
		dd_ext.insert_decreasing(vars[i], dd_val);
	      }
	    }
	    else if (trace_level > 1) {
	      std::cerr << "set ";
	      instance.write_variable_set(std::cerr, vset);
	      std::cerr << " already in collection or PDB too large"
			<< std::endl;
	      if (sets.first(vset) != no_such_index) {
		std::cerr << "GRAPH (EDGE): ";
		write_variable_set_label(std::cerr, instance,
					 w[n]->variables());
		std::cerr << " -> ";
		write_variable_set_label(std::cerr, instance, vset);
		std::cerr << " [style=dashed];" << std::endl;
	      }
	    }
	  }
	  else if (trace_level > 1) {
	    if (w[n]->variables().contains(vars[i])) {
	      std::cerr << "not considering variable "
			<< instance.variables[vars[i]].name
			<< " which is already in the pattern"
			<< std::endl;
	    }
	    else if (!xcg.adjacent(w[n]->variables(), vars[i])) {
	      std::cerr << "not considering variable "
			<< instance.variables[vars[i]].name
			<< " because no (direct) connection to pattern"
			<< std::endl;
	    }
	    else {
	      assert(0);
	    }
	  }
	if (trace_level > 1) {
	  std::cerr << "extension values: " << ext
		    << std::endl
		    << "dd values: " << dd_ext
		    << std::endl;
	}
	NTYPE p_best;
	index_type n_down = 0;
	index_set taken;
	for (index_type i = 0; i < ext.length(); i++) {
	  if (i > 0) {
	    if (ext[i].weight < p_best) n_down += 1;
	  }
	  if (n_down < n_best) {
	    p_best = ext[i].weight;
	    taken.insert(ext[i].value);
	    vset = w[n]->variables();
	    vset.insert(ext[i].value);
	    if (trace_level > 1) {
	      std::cerr << n_down + 1 << "th maximal value "
			<< ext[i].weight << " achieved by extending with "
			<< instance.variables[ext[i].value].name
			<< std::endl;
	    }
	    index_type sset =
	      ProgressionPDBSize::apply(instance.signature, vset);
	    if (sset < pdb_size) {
	      if (trace_level > 1) {
		std::cerr << "GRAPH (EDGE): ";
		write_variable_set_label(std::cerr, instance, w[n]->variables());
		std::cerr << " -> ";
		write_variable_set_label(std::cerr, instance, vset);
		std::cerr << " [label=\""
			  << instance.variables[ext[i].value].name
			  << ": " << ext[i].weight << "\"];" << std::endl;
	      }
	      ProgressionPDB* pdb =
		new ProgressionPDB(instance, vset, cost, sinc, inc, stats);
	      pdb->compute();
	      if (stats.break_signal_raised()) return;
	      w.append(pdb);
	      sets.append(vset);
	    }
	    else if (trace_level > 1) {
	      std::cerr << "PDB-size of ";
	      instance.write_variable_set(std::cerr, vset);
	      std::cerr << " = " << sset << " exceeds size limit"
			<< std::endl;
	    }
	  }
	}

	for (index_type i = 0; i < dd_ext.length(); i++) {
	  if (!taken.contains(dd_ext[i].value)) {
	    vset = w[n]->variables();
	    vset.insert(dd_ext[i].value);
	    index_type sset =
	      ProgressionPDBSize::apply(instance.signature, vset);
	    if (sset < pdb_size) {
	      if (trace_level > 1) {
		std::cerr << "adding pattern ";
		instance.write_variable_set(std::cerr, vset);
		std::cerr << " as potentially useful deadlock detector"
			  << std::endl;
		std::cerr << "GRAPH (NODE): ";
		write_variable_set_label(std::cerr, instance, vset);
		std::cerr << " [style=bold,label=\"";
		instance.write_variable_set(std::cerr, vset);
		std::cerr << " (DD)\"];" << std::endl;
		std::cerr << "GRAPH (EDGE): ";
		write_variable_set_label(std::cerr, instance,
					 w[n]->variables());
		std::cerr << " -> ";
		write_variable_set_label(std::cerr, instance, vset);
		std::cerr << " [style=bold,label=\""
			  << instance.variables[dd_ext[i].value].name
			  << "\"];" << std::endl;
	      }
	      dd_sets.append(vset);
	    }
	  }
	}
      }
      else if (trace_level > 1) {
	std::cerr << "set ";
	instance.write_variable_set(std::cerr, vset);
	std::cerr << " already has maximal size" << std::endl;
	std::cerr << "GRAPH (NODE): ";
	write_variable_set_label(std::cerr, instance, w[n]->variables());
	std::cerr << " [style=dashed,label=\"";
	instance.write_variable_set(std::cerr, w[n]->variables());
	std::cerr << " (" << k + 1 << "/" << g_vars.length()
		  << ", " << w.length() << ")\"];" << std::endl;
      }
      if (n > 0) {
	delete w[n];
      }
      n += 1;
    }
  }

  if (trace_level > 1) {
    std::cerr << "initial PDB set collection:" << std::endl;
    for (index_type k = 0; k < sets.length(); k++) {
      std::cerr << "#" << k << ": ";
      instance.write_variable_set(std::cerr, sets[k]);
      std::cerr << std::endl;
    }
    std::cerr << "initial DD set collection:" << std::endl;
    for (index_type k = 0; k < dd_sets.length(); k++) {
      std::cerr << "#" << k << ": ";
      instance.write_variable_set(std::cerr, dd_sets[k]);
      std::cerr << std::endl;
    }
  }
  else if (trace_level > 0) {
    std::cerr << sets.length() << " PDB sets and "
	      << dd_sets.length() << " DD sets, filtering..." << std::endl;
  }

  graph g(sets.length());
  for (index_type i = 0; i < sets.length(); i++)
    for (index_type j = i + 1; j < sets.length(); j++) {
      if (!instance.additive(sets[i], sets[j]))
	g.add_undirected_edge(i, j);
    }

  bool_vec dom(false, sets.length());
  for (index_type i = 0; i < sets.length(); i++)
    for (index_type j = 0; j < sets.length(); j++)
      if ((i != j) && sets[i].contains(sets[j])) {
	index_set d(g.bidirectional(i));
	d.difference(g.bidirectional(j));
	assert(d.length() >= 2);
	if (d.length() == 2) {
	  assert(d.contains(i));
	  assert(d.contains(j));
	  dom[j] = true;
	}
      }
  if (trace_level > 1) {
    for (index_type k = 0; k < sets.length(); k++) if (dom[k]) {
      std::cerr << "GRAPH (NODE): ";
      write_variable_set_label(std::cerr, instance, sets[k]);
      std::cerr << " [style=filled];" << std::endl;
    }
  }
  sets.remove(dom);

  dom.assign_value(false, dd_sets.length());
  for (index_type k = 0; k < dd_sets.length(); k++) {
    for (index_type i = 0; (i < sets.length()) && !dom[k]; i++)
      if (sets[i].contains(dd_sets[k])) dom[k] = true;
    for (index_type i = 0; (i < dd_sets.length()) && !dom[k]; i++)
      if ((i != k) && dd_sets[i].contains(dd_sets[k])) dom[k] = true;
  }
  if (trace_level > 1) {
    for (index_type k = 0; k < dd_sets.length(); k++) if (dom[k]) {
      std::cerr << "GRAPH (NODE): ";
      write_variable_set_label(std::cerr, instance, dd_sets[k]);
      std::cerr << " [style=filled];" << std::endl;
    }
  }
  dd_sets.remove(dom);

  if (trace_level > 0) {
    std::cerr << "final PDB set collection:" << std::endl;
    for (index_type k = 0; k < sets.length(); k++) {
      std::cerr << "#" << k << ": ";
      instance.write_variable_set(std::cerr, sets[k]);
      std::cerr << std::endl;
    }
    std::cerr << "final DD set collection:" << std::endl;
    for (index_type k = 0; k < dd_sets.length(); k++) {
      std::cerr << "#" << k << ": ";
      instance.write_variable_set(std::cerr, dd_sets[k]);
      std::cerr << std::endl;
    }
  }
  else if (trace_level > 0) {
    std::cerr << "final collection: " << sets.length() << " PDB sets and "
	      << dd_sets.length() << " DD sets" << std::endl;
  }

  for (index_type k = 0; k < dd_sets.length(); k++)
    sets.append(dd_sets[k]);

  stats.stop();
}

NTYPE IncrementalProgressionPDB::test1
(SASHeuristic* h_base,
#ifdef TEST_USE_INCREMENTAL_PDB
 ProgressionPDB* h_new
#else
 ProgressionASH* h_new
#endif
 )
{
#ifdef TRACE_PRINT_LOTS
  partial_state s0(instance.init_state, h_new->variables());
  std::cerr << "test1: abstract init state = ";
  instance.write_partial_state(std::cerr, s0);
  std::cerr << std::endl;
#endif
  NTYPE v_base = h_base->eval(instance.init_state);
  if (FINITE(v_base)) {
#ifdef TEST_USE_INCREMENTAL_PDB
    h_new->compute_to(instance.init_state);
#endif
#ifdef TEST_BY_COUNT_ONLY
    NTYPE v_new = h_new->eval_to_bound(instance.init_state, v_base + 1);
    return ((v_new - v_base) > 0 ? 1 : 0);
#else
    NTYPE v_new = h_new->eval(instance.init_state);
    return (v_new - v_base);
#endif
  }
  else {
    return 0;
  }
}

NTYPE IncrementalProgressionPDB::test3
(SASHeuristic* h_base,
 NTYPE v_max_base,
#ifdef TEST_USE_INCREMENTAL_PDB
 ProgressionPDB* h_new,
#else
 ProgressionASH* h_new,
#endif
 NTYPE v_max_new,
 MDDNode* sinc,
 Heuristic* inc,
 index_type n_samples,
 NTYPE& dd_val,
 RNG& rng)
{
  index_type dd_count = 0;
  index_type dd_base = 1;
  index_type n = 1;
  index_type t = 1;
  NTYPE v_plus = test1(h_base, h_new);
  if (INFINITE(v_plus)) {
    if (trace_level > 2) {
      std::cerr << "test3: infinite value " << v_plus << " returned by test1"
		<< std::endl;
    }
    return POS_INF;
  }
#ifdef TRACE_PRINT_LOTS
  std::cerr << "test3: sample #0: v_plus = " << v_plus << std::endl;
#endif
  // index_type d_max = (index_type)(sqrt(h_new->size()));
  // index_type d_max = h_new->size();
//   std::cerr << "COMP: " << d_max
// 	    << ", " << FLOOR_TO_INT(v_max_base)
// 	    << ", " << FLOOR_TO_INT(v_max_new)
// 	    << std::endl;
  index_type d_max = FLOOR_TO_INT(v_max_new);
#ifdef BUILD_GRAPH_FOR_RANDOM_WALK
  index_set* al_g = 0;
  index_type n_g = 0;
  h_new->build_explicit_graph(sinc, al_g, n_g);
#endif
  while ((n < n_samples) && (t < 2*n_samples)) {
    partial_state s1(instance.init_state);
#ifdef BUILD_GRAPH_FOR_RANDOM_WALK
    index_type l = h_new->random_walk(s1, d_max, al_g, n_g, rng);
#else
    index_type l = h_new->random_walk(s1, d_max, sinc, inc, rng);
#endif
    if (stats.break_signal_raised()) return 0;
    if (l >= d_max) {
      NTYPE v_base = h_base->eval(s1);
#ifdef TRACE_PRINT_LOTS
      std::cerr << "test3: sample #" << n << " (" << l << " steps): ";
      instance.write_partial_state(std::cerr, s1);
      std::cerr << ", v_base = " << v_base
		<< std::endl;
#endif
      if (FINITE(v_base)) {
	dd_base += 1;
#ifdef TEST_USE_INCREMENTAL_PDB
	h_new->compute_to(s1);
#endif
	NTYPE v_new = h_new->eval(s1);
	if (stats.break_signal_raised()) return 0;
#ifdef TRACE_PRINT_LOTS
	std::cerr << "test3: v_new = " << v_new
		  << ", v_plus = " << (v_new - v_base)
		  << std::endl;
#endif
#ifdef TEST_BY_COUNT_ONLY
	v_plus += ((v_new - v_base) > 0 ? 1 : 0);
	n += 1;
#else
	if (INFINITE(v_new)) {
	  // store the deadlock in sas inc., to avoid "detecting" it again
	  if (sinc) sinc->insert(s1);
	  dd_count += 1;
	}
	else {
	  v_plus += (v_new - v_base);
	  n += 1;
	}
#endif
      }
      else {
	n += 1;
      }
    }
    t += 1;
  }
#ifdef BUILD_GRAPH_FOR_RANDOM_WALK
  delete [] al_g;
#endif
#ifdef TRACE_PRINT_LOTS
  std::cerr << "test3: final v_plus = "
	    << v_plus << " / " << n << " = "
	    << (v_plus / n)
	    << std::endl;
#endif
  dd_val = R_TO_N(dd_count, dd_base);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "test3: dd_val = "
	    << dd_count << " / " << dd_base << " = "
	    << dd_val
	    << std::endl;
#endif
  return (v_plus / n);
}

NTYPE IncrementalProgressionPDB::test_merge3
(ProgressionPDB* pdb1,
 ProgressionPDB* pdb2,
 const ACF& cost,
 MDDNode* sinc,
 Heuristic* inc,
 index_type n_samples,
 NTYPE& dd_val,
 RNG& rng)
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "merge3: evaluating merge of ";
  instance.write_variable_set(std::cerr, pdb1->variables());
  std::cerr << " and ";
  instance.write_variable_set(std::cerr, pdb2->variables());
  std::cerr << "..." << std::endl;
#endif
  NTYPE v_plus = 0;
  if (pdb1->additive(*pdb2)) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "merge3: PDB ";
    instance.write_variable_set(std::cerr, pdb1->variables());
    std::cerr << " and PDB ";
    instance.write_variable_set(std::cerr, pdb2->variables());
    std::cerr << " are additive: comparing with sum..." << std::endl;
#endif
    index_set s(pdb1->variables());
    s.insert(pdb2->variables());
    // v_max_{base,cmb} are ESTIMATES! NOT VALID UPPER BOUNDS!
    NTYPE v_max_base = (pdb1->max() + pdb2->max());
    NTYPE v_max_cmb = (pdb1->max() * pdb2->max());
    AddH* h_sum = new AddH();
    h_sum->new_component(pdb1);
    h_sum->new_component(pdb2);
#ifdef TEST_USE_INCREMENTAL_PDB
    ProgressionPDB* h_cmb =
      new ProgressionPDB(instance, s, cost, sinc, inc, stats);
    h_cmb->initialize();
#else
    ProgressionASH* h_cmb =
      new ProgressionASH(instance, s, cost, sinc, inc, *h_sum, stats);
#endif
    v_plus = test3(h_sum, v_max_base, h_cmb, v_max_cmb, sinc, inc,
		   n_samples, dd_val, rng);
    delete h_cmb;
    delete h_sum;
  }
  else {
#ifdef TRACE_PRINT_LOTS
    std::cerr << "merge3: PDB ";
    instance.write_variable_set(std::cerr, pdb1->variables());
    std::cerr << " and PDB ";
    instance.write_variable_set(std::cerr, pdb2->variables());
    std::cerr << " are NOT additive: comparing with max..." << std::endl;
#endif
    index_set s(pdb1->variables());
    s.insert(pdb2->variables());
    // v_max_{base,cmb} are ESTIMATES! NOT VALID UPPER BOUNDS!
    NTYPE v_max_base = MAX(pdb1->max(), pdb2->max());
    NTYPE v_max_cmb = (pdb1->max() * pdb2->max());
    MaxH* h_max = new MaxH();
    h_max->new_component(pdb1);
    h_max->new_component(pdb2);
#ifdef TEST_USE_INCREMENTAL_PDB
    ProgressionPDB* h_cmb =
      new ProgressionPDB(instance, s, cost, sinc, inc, stats);
    h_cmb->initialize();
#else
    ProgressionASH* h_cmb =
      new ProgressionASH(instance, s, cost, sinc, inc, *h_max, stats);
#endif
    v_plus = test3(h_max, v_max_base, h_cmb, v_max_cmb, sinc, inc,
		   n_samples, dd_val, rng);
    delete h_cmb;
    delete h_max;
  }
#ifdef TRACE_PRINT_LOTS
  std::cerr << "merge3: value = " << v_plus << std::endl;
#endif
  return v_plus;
}

NTYPE IncrementalProgressionPDB::test_grow3
(ProgressionPDB* pdb,
 index_type v_new,
 const ACF& cost,
 MDDNode* sinc,
 Heuristic* inc,
 index_type n_samples,
 NTYPE& dd_val,
 RNG& rng)
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "grow3: evaluating extension of ";
  instance.write_variable_set(std::cerr, pdb->variables());
  std::cerr << " with non-goal variable "
	    << instance.variables[v_new].name
	    << "..." << std::endl;
#endif
  index_set s(pdb->variables());
  s.insert(v_new);
  // v_max_{base,cmb} are ESTIMATES! NOT VALID UPPER BOUNDS!
  NTYPE v_max_base = pdb->max();
  NTYPE v_max_cmb = pdb->max() * (instance.variables[v_new].n_values() / 2);
#ifdef TEST_USE_INCREMENTAL_PDB
  ProgressionPDB* h_cmb =
    new ProgressionPDB(instance, s, cost, sinc, inc, stats);
  h_cmb->initialize();
#else
  ProgressionASH* h_cmb =
    new ProgressionASH(instance, s, cost, sinc, inc, *pdb, stats);
#endif
  NTYPE v_plus = test3(pdb, v_max_base, h_cmb, v_max_cmb, sinc, inc,
		       n_samples, dd_val, rng);
  delete h_cmb;
#ifdef TRACE_PRINT_LOTS
  std::cerr << "grow3: value = " << v_plus << std::endl;
#endif
  return v_plus;
}

MaxMatchPDB::MaxMatchPDB
(SASInstance& ins, const index_set& avs, Statistics& s)
  : instance(ins),
    stats(s),
    variables(avs),
    variable_pdb(0, 0),
    pair_pdb(sas_heuristic_vec(0, 0), 0),
    variable_graph(avs.length()),
    matched(false, avs.length())
{
  // done ?
}

MaxMatchPDB::~MaxMatchPDB()
{
  // delete some PDBs...
}

void MaxMatchPDB::compute_regression_PDB(const ACF& cost, Heuristic* inc)
{
  variable_pdb.set_length(variables.length());
  for (index_type i = 0; i < variables.length(); i++) {
    index_set v_set;
    v_set.assign_singleton(variables[i]);
    RegressionPDB* pdb =
      new RegressionPDB(instance, v_set, cost, 0, inc, stats);
    pdb->compute();
    variable_pdb[i] = pdb;
  }
  pair_pdb.set_length(variables.length());
  for (index_type i = 0; i < variables.length(); i++) {
    pair_pdb[i].set_length(variables.length());
    for (index_type j = i + 1; j < variables.length(); j++) {
      index_set v_set;
      v_set.assign_singleton(variables[i]);
      v_set.insert(variables[j]);
      RegressionPDB* pdb =
	new RegressionPDB(instance, v_set, cost, 0, inc, stats);
      pdb->compute();
      pair_pdb[i][j] = pdb;
    }
  }
}

void MaxMatchPDB::compute_progression_PDB(const ACF& cost, Heuristic* inc)
{
  variable_pdb.set_length(variables.length());
  for (index_type i = 0; i < variables.length(); i++) {
    index_set v_set;
    v_set.assign_singleton(variables[i]);
    ProgressionPDB* pdb =
      new ProgressionPDB(instance, v_set, cost, 0, inc, stats);
    pdb->compute();
    variable_pdb[i] = pdb;
  }
  pair_pdb.set_length(variables.length());
  for (index_type i = 0; i < variables.length(); i++) {
    pair_pdb[i].set_length(variables.length());
    for (index_type j = i + 1; j < variables.length(); j++) {
      index_set v_set;
      v_set.assign_singleton(variables[i]);
      v_set.insert(variables[j]);
      ProgressionPDB* pdb =
	new ProgressionPDB(instance, v_set, cost, 0, inc, stats);
      pdb->compute();
      pair_pdb[i][j] = pdb;
    }
  }
}

NTYPE MaxMatchPDB::eval(const partial_state& s)
{
  for (index_type i = 0; i < variables.length(); i++)
    for (index_type j = i + 1; j < variables.length(); j++) {
      assert(pair_pdb[i][j]);
      NTYPE v = pair_pdb[i][j]->eval(s);
      variable_graph.set_weight(i, j, v);
      variable_graph.set_weight(j, i, v);
    }
  NTYPE m = variable_graph.apx_matching(matched);
  for (index_type k = 0; k < variables.length(); k++)
    if (!matched[k]) m += variable_pdb[k]->eval(s);
  return m;
}

MaxH* MaxMatchPDB::independent_pair_regression_heuristic
(SASInstance& ins, Statistics& s, ACF& cost, Heuristic* inc)
{
  MaxH* max_h = new MaxH();
  IndependentVariables iv(ins);
  iv.compute_approximate_independent_sets();
  for (index_type k = 0; k < iv.n_sets(); k++) {
    if (iv.set(k).length() > 1) {
      MaxMatchPDB* mm = new MaxMatchPDB(ins, iv.set(k), s);
      mm->compute_regression_PDB(cost, inc);
      max_h->new_component(mm);
    }
  }
  return max_h;
}

MaxH* MaxMatchPDB::independent_pair_progression_heuristic
(SASInstance& ins, Statistics& s, ACF& cost, Heuristic* inc)
{
  MaxH* max_h = new MaxH();
  IndependentVariables iv(ins);
  iv.compute_approximate_independent_sets();
  for (index_type k = 0; k < iv.n_sets(); k++) {
    if (iv.set(k).length() > 1) {
      MaxMatchPDB* mm = new MaxMatchPDB(ins, iv.set(k), s);
      mm->compute_progression_PDB(cost, inc);
      max_h->new_component(mm);
    }
  }
  return max_h;
}

END_HSPS_NAMESPACE
