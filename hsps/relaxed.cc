
#include "relaxed.h"
#include "forward.h"

BEGIN_HSPS_NAMESPACE

ForwardRelaxedDFS::ForwardRelaxedDFS
(Instance& i, Instance& r, const index_set& g, const ACF& c, Statistics& s)
  : Heuristic(i), relaxed(r), goals(g), cost(c)
{
  rfh = new ForwardH1(relaxed, goals, cost, s);
  search = new DFS_BB(s, result);
  search->set_cycle_check(true);
  search->set_trace_level(0);
}

ForwardRelaxedDFS::~ForwardRelaxedDFS()
{
  delete rfh;
  delete search;
}

NTYPE ForwardRelaxedDFS::eval(const index_set& s)
{
  bool sua_state = SeqProgState::separate_update_actions;
  SeqProgState::separate_update_actions = true;
  // std::cerr << "computing h(";
  // instance.write_atom_set(std::cerr, s);
  // search->set_trace_level(3);
  SeqCProgState* root = new SeqCProgState(relaxed, *rfh, cost, s);
  NTYPE val = search->start(*root, POS_INF);
  // std::cerr << " = " << val << std::endl;
  SeqProgState::separate_update_actions = sua_state;
  if (!search->solved()) return POS_INF;
  return val;
}

NTYPE ForwardRelaxedDFS::eval(const bool_vec& s)
{
  bool sua_state = SeqProgState::separate_update_actions;
  SeqProgState::separate_update_actions = true;
  // std::cerr << "computing h(";
  // instance.write_atom_set(std::cerr, s);
  // search->set_trace_level(3);
  SeqCProgState* root = new SeqCProgState(relaxed, *rfh, cost, s);
  NTYPE val = search->start(*root, POS_INF);
  // std::cerr << " = " << val << std::endl;
  SeqProgState::separate_update_actions = sua_state;
  if (!search->solved()) return POS_INF;
  return val;
}


ForwardRelaxedBFS::ForwardRelaxedBFS
(Instance& i, Instance& r, const index_set& g, const ACF& c, Statistics& s)
  : Heuristic(i), relaxed(r), goals(g), cost(c)
{
  // rfh = new ForwardH1(relaxed, goals, cost, s);
  rfh = new ZeroHeuristic(relaxed);
  search = new BFS(s, result, 31337);
  search->set_trace_level(0);
}

ForwardRelaxedBFS::~ForwardRelaxedBFS()
{
  delete rfh;
  delete search;
}

NTYPE ForwardRelaxedBFS::eval(const index_set& s)
{
  bool sua_state = SeqProgState::separate_update_actions;
  SeqProgState::separate_update_actions = true;
  // std::cerr << "computing h(";
  // instance.write_atom_set(std::cerr, s);
  // search->set_trace_level(3);
  SeqProgState* root = new SeqProgState(relaxed, *rfh, cost, s);
  NTYPE val = search->start(*root, POS_INF);
  // std::cerr << " = " << val << std::endl;
  SeqProgState::separate_update_actions = sua_state;
  if (!search->solved()) return POS_INF;
  return val;
}

NTYPE ForwardRelaxedBFS::eval(const bool_vec& s)
{
  bool sua_state = SeqProgState::separate_update_actions;
  SeqProgState::separate_update_actions = true;
  // std::cerr << "computing h(";
  // instance.write_atom_set(std::cerr, s);
  // search->set_trace_level(3);
  SeqProgState* root = new SeqProgState(relaxed, *rfh, cost, s);
  NTYPE val = search->start(*root, POS_INF);
  // std::cerr << " = " << val << std::endl;
  SeqProgState::separate_update_actions = sua_state;
  if (!search->solved()) return POS_INF;
  return val;
}


SumX::SumX(Instance& i, const ACF& c, Statistics& s)
  : CombineNBySum(i)
{
  equivalence r(instance.n_atoms());
  for (index_type k = 0; k < instance.n_actions(); k++) {
    for (index_type i = 0; i < instance.actions[k].add.length(); i++) {
      for (index_type j = i+1; j < instance.actions[k].add.length(); j++)
        r.merge(instance.actions[k].add[i],
		instance.actions[k].add[j]);
      for (index_type j = 0; j < instance.actions[k].del.length(); j++)
        r.merge(instance.actions[k].add[i],
		instance.actions[k].del[j]);
    }
    for (index_type i = 0; i < instance.actions[k].del.length(); i++) {
      for (index_type j = i+1; j < instance.actions[k].del.length(); j++)
        r.merge(instance.actions[k].del[i],
		instance.actions[k].del[j]);
    }
  }
  index_set_vec x;
  r.classes(x);

  for (index_type k = 0; k < x.length(); k++) {
    std::cerr << "creating heuristic for class ";
    instance.write_atom_set(std::cerr, x[k]);
    std::cerr << std::endl;

    Instance* r_ins = new Instance(instance);
    r_ins->delete_relax(x[k]);
    DiscountACF* r_cost = new DiscountACF(c, instance.n_actions());
    for (index_type i = 0; i < instance.n_actions(); i++) {
      if ((instance.actions[i].add.first_common_element(x[k]) == no_such_index) || (instance.actions[i].add.first_common_element(x[k]) == no_such_index))
	r_cost->discount(i);
    }
    ForwardRelaxedBFS* r_h =
      new ForwardRelaxedBFS(instance, *r_ins, instance.goal_atoms, *r_cost, s);
    add(r_h);
  }
}

END_HSPS_NAMESPACE
