
#include "htn.h"

BEGIN_HSPS_NAMESPACE

bool HTNInstance::write_HTN = true;

HTNInstance::HTNInstance()
  : tasks(Task(), 0), goal_tasks(step(), 0)
{
  // done
}

HTNInstance::HTNInstance(Name* n)
  : Instance(n), tasks(Task(), 0), goal_tasks(step(), 0)
{
  // done
}

HTNInstance::HTNInstance(const Instance& ins)
  : Instance(ins), tasks(Task(), 0), goal_tasks(step(), 0)
{
  // done
}

HTNInstance::HTNInstance(const HTNInstance& ins)
  : Instance(ins), tasks(ins.tasks), goal_tasks(ins.goal_tasks)
{
  // done
}

HTNInstance::Task& HTNInstance::new_task(Name* name)
{
  Task& tk = tasks.append();
  tk.index = tasks.length() - 1;
  tk.name = name;
  tk.exp.clear();
  if (trace_level > 2) {
    std::cerr << "task " << tk.index << "." << tk.name
	      << " created" << std::endl;
  }
  return tk;
}

void HTNInstance::remove_actions(const bool_vec& set, index_vec& map)
{
  Instance::remove_actions(set, map);
  // use map to re-index method task lists...
}

void HTNInstance::set_goal_tasks(const step_vec& g)
{
  goal_tasks = g;
}

void HTNInstance::task_names(name_vec& names) const
{
  names.clear();
  for (index_type k = 0; k < n_tasks(); k++)
    names.append(tasks[k].name);
}

void HTNInstance::write_step_sequence
(std::ostream& s, const step_vec& p) const
{
  for (index_type k = 0; k < p.length(); k++) {
    if (k > 0) s << ';';
    if (p[k].abstract) {
      assert(p[k].index < n_tasks());
      s << tasks[p[k].index].name;
    }
    else {
      assert(p[k].index < n_actions());
      s << actions[p[k].index].name;
    }
  }
}

void HTNInstance::write_domain(std::ostream& s) const
{
  if (write_HTN) {
    Instance::write_domain(s);

    for (index_type k = 0; k < n_tasks(); k++)
      if (tasks[k].exp.length() > 0) {
	s << " (:action ";
	tasks[k].name->write(s, Name::NC_INSTANCE);
	s << std::endl;
	for (index_type i = 0; i < tasks[k].exp.length(); i++) {
	  s << "  (:expansion";
	  if (tasks[k].exp[i].pre.length() > 0) {
	    s << " :precondition";
	    if (tasks[k].exp[i].pre.length() > 1)
	      s << " (and";
	    for (index_type l = 0; l < tasks[k].exp[i].pre.length(); l++) {
	      s << "(";
	      atoms[tasks[k].exp[i].pre[l]].name->write(s, Name::NC_INSTANCE);
	      s << ")";
	    }
	    if (tasks[k].exp[i].pre.length() > 1)
	      s << ")";
	  }
	  s << " :tasks (";
	  for (index_type l = 0; l < tasks[k].exp[i].steps.length(); l++) {
	    if (l > 0) s << " ";
	    s << "(";
	    if (tasks[k].exp[i].steps[l].abstract) {
	      tasks[tasks[k].exp[i].steps[l].index].name->write(s, Name::NC_INSTANCE);
	    }
	    else {
	      actions[tasks[k].exp[i].steps[l].index].name->write(s, Name::NC_INSTANCE);
	    }
	    s << ")";
	  }
	  s << "))" << std::endl;
	}
      }

    s << ")" << std::endl;
  }
  else {
    Instance::write_domain(s);
  }
}

void HTNInstance::write_problem_goal(std::ostream& s) const
{
  if (write_HTN) {
    index_type goal_atom_count = 0;
    for (index_type k = 0; (k < n_atoms()) && (goal_atom_count < 2); k++)
      if (atoms[k].goal) goal_atom_count += 1;
    if ((goal_atom_count + goal_tasks.length()) > 0) {
      s << " (:goal";
      if ((goal_atom_count > 1) ||
	  ((goal_atom_count > 0) && (goal_tasks.length() > 0)))
	s << " (and";
      for (index_type k = 0; k < n_atoms(); k++) if (atoms[k].goal) {
	s << " (";
	atoms[k].name->write(s, Name::NC_INSTANCE);
	s << ")";
      }
      if (goal_tasks.length() > 0) {
	if (goal_atom_count > 0) s << " ";
	s << ":tasks (";
	for (index_type k = 0; k < goal_tasks.length(); k++) {
	  if (k > 0) s << " ";
	  s << "(";
	  if (goal_tasks[k].abstract) {
	    assert(goal_tasks[k].index < n_tasks());
	    tasks[goal_tasks[k].index].name->write(s, Name::NC_INSTANCE);
	  }
	  else {
	    assert(goal_tasks[k].index < n_actions());
	    actions[goal_tasks[k].index].name->write(s, Name::NC_INSTANCE);
	  }
	  s << ")";
	}
	s << ")" << std::endl;
      }
      if ((goal_atom_count > 1) ||
	  ((goal_atom_count > 0) && (goal_tasks.length() > 0)))
	s << " )";
      s << ")" << std::endl;
    }
  }
  else {
    Instance::write_problem_goal(s);
  }
}

void HTNInstance::write_abstract_plan
(std::ostream& s, const step_vec& p) const
{
  s << "(:plan";
  for (index_type k = 0; k < p.length(); k++) {
    s << std::endl << " " << k + 1 << " : ";
    if (p[k].abstract) {
      assert(p[k].index < n_tasks());
      tasks[p[k].index].name->write(s, Name::NC_INSTANCE);
    }
    else {
      assert(p[k].index < n_actions());
      actions[p[k].index].name->write(s, Name::NC_INSTANCE);
    }
  }
  s << ")" << std::endl;
}


FwdSeqHTNState::FwdSeqHTNState(HTNInstance& i)
  : AtomSet(i), instance(i)
{
  // done
}

FwdSeqHTNState::FwdSeqHTNState
(HTNInstance& i, const index_set& s, const HTNInstance::step_vec& g)
  : AtomSet(i, s), instance(i), rem(g)
{
  // done
}

FwdSeqHTNState::FwdSeqHTNState(const FwdSeqHTNState& s)
  : ProgressionState(s), AtomSet(s), instance(s.instance), rem(s.rem)
{
  // done
}

FwdSeqHTNState::~FwdSeqHTNState()
{
  // done
}

NTYPE FwdSeqHTNState::delta_cost()
{
  FwdSeqHTNState* ps = (FwdSeqHTNState*)predecessor();
  if (ps) {
    if (ps->rem.length() > 0) {
      if (!ps->rem[0].abstract) return 1;
    }
  }
  return 0;
}

NTYPE FwdSeqHTNState::est_cost()
{
  return 0;
}

bool FwdSeqHTNState::is_final()
{
  return rem.empty();
}

bool FwdSeqHTNState::is_max()
{
  return false;
}

NTYPE FwdSeqHTNState::expand(Search& s, NTYPE bound)
{
  if (rem.empty()) return POS_INF;
  if (rem[0].abstract) {
    HTNInstance::Task& tk = instance.tasks[rem[0].index];
    NTYPE c_min = POS_INF;
    for (index_type m = 0; m < tk.exp.length(); m++) {
      HTNInstance::Method& met = tk.exp[m];
      bool app = true;
      for (index_type k = 0; (k < met.pre.length()) && app; k++)
	if (!set[met.pre[k]]) app = false;
      if (app) {
	FwdSeqHTNState* new_s = new FwdSeqHTNState(instance);
	new_s->add(set);
	for (index_type i = 0; i < met.steps.length(); i++)
	  new_s->rem.append(met.steps[i]);
	for (index_type i = 1; i < rem.length(); i++)
	  new_s->rem.append(rem[i]);
	new_s->set_predecessor(this);
	if (new_s->est_cost() <= bound) {
	  NTYPE c_new = s.new_state(*new_s, bound);
	  c_min = MIN(c_min, c_new);
	}
	else {
	  c_min = MIN(c_min, new_s->est_cost());
	}
	delete new_s;
      }
    }
    return c_min;
  }
  else {
    Instance::Action& a = instance.actions[rem[0].index];
    bool app = true;
    for (index_type k = 0; (k < a.pre.length()) && app; k++)
      if (!set[a.pre[k]]) app = false;
    if (app) {
      FwdSeqHTNState* new_s = new FwdSeqHTNState(instance);
      new_s->add(set);
      new_s->add(a.add);
      new_s->del(a.del);
      for (index_type i = 1; i < rem.length(); i++)
	new_s->rem.append(rem[i]);
      new_s->set_predecessor(this);
      NTYPE c_new;
      if ((1 + new_s->est_cost()) <= bound) {
	c_new = 1 + s.new_state(*new_s, bound - 1);
      }
      else {
	c_new = (1 + new_s->est_cost());
      }
      delete new_s;
      return c_new;
    }
    else {
      return POS_INF;
    }
  }
}

void FwdSeqHTNState::store(NTYPE cost, bool opt)
{
  std::cerr << "error: can't store FwdSeqHTNState " << *this << std::endl;
  exit(255);
}

void FwdSeqHTNState::reevaluate()
{
  // does nothing (est_cost == 0)
}

int FwdSeqHTNState::compare(const State& s)
{
  const FwdSeqHTNState& p = (const FwdSeqHTNState&)s;
  if (rem < p.rem) return -1;
  else if (p.rem < rem) return 1;
  else return AtomSet::compare(p);
}

index_type FwdSeqHTNState::hash()
{
  return AtomSet::hash();
}

State* FwdSeqHTNState::copy()
{
  return new FwdSeqHTNState(*this);
}

void FwdSeqHTNState::insert(Plan& p)
{
  FwdSeqHTNState* ps = (FwdSeqHTNState*)predecessor();
  if (ps) {
    if (ps->rem.length() > 0) {
      if (!ps->rem[0].abstract) {
	p.insert(ps->rem[0].index);
      }
    }
  }
}

void FwdSeqHTNState::write(std::ostream& s)
{
  s << "(";
  instance.write_atom_set(s, set);
  s << ", ";
  instance.write_step_sequence(s, rem);
  s << ")";
}

void FwdSeqHTNState::write_plan(std::ostream& s)
{
  s << "FwdSeqHTNState::write_plan NOT IMPLEMENTED";
}

END_HSPS_NAMESPACE
