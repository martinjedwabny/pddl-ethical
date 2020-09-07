
#include "pop.h"

BEGIN_HSPS_NAMESPACE

SafePOP::SafePOP(Instance& i)
  : instance(i), trace_level(Instance::default_trace_level)
{
  // empty POP
}

SafePOP::SafePOP(Instance& i, const Schedule::step_vec& s_steps)
  : instance(i), trace_level(Instance::default_trace_level)
{
  construct(s_steps, true, false);
}

SafePOP::~SafePOP()
{
  // done
}

void SafePOP::construct
(const Schedule::step_vec& s_steps, bool deorder, bool assume_sequential)
{
  // create init/goal actions and steps
  Instance::Action& init_act = instance.new_action(new StringName("init"));
  Instance::Action& goal_act = instance.new_action(new StringName("goal"));
  init_act.pre.clear();
  init_act.add.assign_copy(instance.init_atoms);
  init_act.del.clear();
  init_act.dmin = 0;
  init_act.dmax = 0;
  init_act.dur = 0;
  init_act.cost = 0;
  goal_act.pre.assign_copy(instance.goal_atoms);
  goal_act.add.clear();
  goal_act.del.clear();
  goal_act.dmin = 0;
  goal_act.dmax = 0;
  goal_act.dur = 0;
  goal_act.cost = 0;

  steps.clear();
  index_type next_t = 0;
  index_type t_init = next_t++;
  steps.append(step(steps.length(), init_act.index, t_init, t_init));
  index_type t_goal = next_t++;
  steps.append(step(steps.length(), goal_act.index, t_goal, t_goal));

  // insert schedule steps
  for (index_type k = 0; k < s_steps.length(); k++)
    steps.append(step(steps.length(), s_steps[k].act, next_t++, next_t++));

  tcn.init(next_t);
  // constrain all steps to start after init
  for (index_type k = 0; k < steps.length(); k++)
    tcn.set_min(steps[INIT_STEP].t_end, steps[k].t_start, 0);
  // constrain all steps to end before goal
  for (index_type k = 0; k < steps.length(); k++)
    tcn.set_min(steps[k].t_end, steps[GOAL_STEP].t_start, 0);
  // constrain start/end times
  for (index_type k = 0; k < steps.length(); k++) {
    tcn.set_min(steps[k].t_start, steps[k].t_end, 0);
  }

  if (deorder) {
    // extract necessary temporal constraints (PRF algorithm)
    for (index_type i = 0; i < s_steps.length(); i++) {
      Instance::Action& ai = instance.actions[s_steps[i].act];
      for (index_type j = 0; j < s_steps.length(); j++) {
	bool prec = (assume_sequential ?
		     (s_steps[i].at < s_steps[j].at) :
		     ((s_steps[i].at + ai.dur) <= s_steps[j].at));
	if (prec) {
	  if (trace_level > 0) {
	    std::cerr << "step " << i << "."
		      << instance.actions[s_steps[i].act].name
		      << " < step " << j << "."
		      << instance.actions[s_steps[j].act].name
		      << std::endl;
	  }
	  if (!instance.commutative(s_steps[i].act, s_steps[j].act) ||
	      !instance.lock_compatible(s_steps[i].act, s_steps[j].act)) {
	    if (trace_level > 0) {
	      std::cerr << "step " << i << "."
			<< instance.actions[s_steps[i].act].name
			<< " # step " << j << "."
			<< instance.actions[s_steps[j].act].name
			<< " (set T" << steps[i+2].t_end
			<< " -> T" << steps[j+2].t_start << " > 0)"
			<< std::endl;
	    }
	    tcn.set_min(steps[i+2].t_end, steps[j+2].t_start, 0);
	  }
	}
      }
    }
  }

  else {
    // insert all plan orderings into tcn
    for (index_type i = 0; i < s_steps.length(); i++) {
      Instance::Action& ai = instance.actions[s_steps[i].act];
      for (index_type j = 0; j < s_steps.length(); j++) {
	bool prec = (assume_sequential ?
		     (s_steps[i].at < s_steps[j].at) :
		     ((s_steps[i].at + ai.dur) <= s_steps[j].at));
	if (prec) {
	  tcn.set_min(steps[i+2].t_end, steps[j+2].t_start, 0);
	}
      }
    }
  }

  tcn.compute_minimal();
  // done!
}

void SafePOP::find_safe_causal_links()
{
  links.clear();
  for (index_type i = 0; i < steps.length(); i++) {
    Instance::Action& ai = instance.actions[steps[i].act];
    for (index_type k = 0; k < ai.pre.length(); k++) {
      bool found = false;
      for (index_type j = 0; j < steps.length(); j++) if (j != i) {
	Instance::Action& aj = instance.actions[steps[j].act];
	// step j is a possible establisher for pre[k] of step i iff
	// action a_j adds pre[k], or requires & locks pre[k], and
	// step i can not start earlier than step j finishes
	if ((aj.add.contains(ai.pre[k]) ||
	     (aj.pre.contains(ai.pre[k]) && aj.lck.contains(ai.pre[k]))) &&
	    (tcn.min_distance(steps[j].t_end, steps[i].t_start) >= 0)) {
	  bool threat = false;
	  for (index_type l = 0; (l < steps.length()) && !threat; l++)
	    if ((l != i) && (l != j)) {
	      Instance::Action& al = instance.actions[steps[l].act];
	      // step l is a potential threat to link pre[k]:j->i iff
	      // action a_l deletes pre[k], and
	      // step l can finish after start of j and start before end of i
	      if ((al.del.contains(ai.pre[k]) ||
		   al.lck.contains(ai.pre[k])) &&
		  (tcn.min_distance(steps[l].t_end, steps[j].t_start) < 0) &&
		  (tcn.min_distance(steps[i].t_end, steps[l].t_start) < 0))
		threat = true;
	    }
	  if (!threat) {
	    links.insert(causal_link(ai.pre[k], j, i));
	    found = true;
	  }
	}
      }
      if (!found) {
	std::cerr << "warning: no unthreatened causal link for "
		  << instance.atoms[ai.pre[k]].name
		  << " of step #" << i << " (" << ai.name << ")"
		  << " - plan may be invalid"
		  << std::endl;
      }
    }
  }
}

void SafePOP::enforce_min_durations()
{
  for (index_type k = 0; k < steps.length(); k++) {
    if (trace_level > 0) {
      std::cerr << "set T" << steps[k].t_start
		<< " -> T" << steps[k].t_end
		<< " > " << instance.actions[steps[k].act].dmin
		<< std::endl;
    }
    tcn.set_min(steps[k].t_start, steps[k].t_end,
		instance.actions[steps[k].act].dmin);
  }
  tcn.compute_minimal();
}

void SafePOP::enforce_max_durations()
{
  for (index_type k = 0; k < steps.length(); k++) {
    tcn.set_max(steps[k].t_start, steps[k].t_end,
		instance.actions[steps[k].act].dmax);
  }
  tcn.compute_minimal();
}

void SafePOP::enforce_makespan(NTYPE b)
{
  tcn.set_max(steps[INIT_STEP].t_end, steps[GOAL_STEP].t_start, b);
  tcn.compute_minimal();
}

void SafePOP::write(std::ostream& s)
{
  for (index_type k = 0; k < steps.length(); k++) {
    Instance::Action& a = instance.actions[steps[k].act];
    s << "[T" << steps[k].t_start << ",T" << steps[k].t_end << "]: "
      << a.name << std::endl;
    for (index_type l = 0; l < links.length(); l++) if (links[l].to == k) {
      if (a.del.contains(links[l].atom) || 
	  a.lck.contains(links[l].atom)) {
	s << " [T" << steps[links[l].from].t_end
	  << ",T" << steps[k].t_start << "]: "
	  << instance.atoms[links[l].atom].name
	  << std::endl;
      }
      else { // link is to persistent precondition of step k
	s << " [T" << steps[links[l].from].t_end
	  << ",T" << steps[k].t_end << "]: "
	  << instance.atoms[links[l].atom].name
	  << std::endl;
      }
    }
  }
  tcn.write(s);
  s << " (makespan: "
    << tcn.min_distance(steps[INIT_STEP].t_start,
			steps[GOAL_STEP].t_start)
    << ")" << std::endl;
}

void SafePOP::write_graph(std::ostream& s)
{
  s << "digraph POP {" << std::endl;
  s << "rankdir=LR;" << std::endl;
  s << "node [width=0,height=0];" << std::endl;
  graph g;
  tcn.precedence_graph(g);
  // std::cerr << "TCN graph: " << g << std::endl;
  graph meg;
  g.strongly_connected_components();
  g.minimal_equivalent_digraph(meg);
  // std::cerr << "meg of TCN graph: " << meg << std::endl;
  graph w(tcn.length());
  for (index_type k = 0; k < steps.length(); k++) {
    if (steps[k].t_start == steps[k].t_end) {
      s << "T" << steps[k].t_start << "[label=\""
	<< instance.actions[steps[k].act].name
	<< "\"];" << std::endl;
    }
    else {
      s << "T" << steps[k].t_start << " -> T" << steps[k].t_end
	<< " [label=\"" << instance.actions[steps[k].act].name
	<< ": [" << tcn.min_distance(steps[k].t_start, steps[k].t_end)
	<< ", " << tcn.max_distance(steps[k].t_start, steps[k].t_end)
	<< "]\",style=bold];" << std::endl;
      w.add_edge(steps[k].t_start, steps[k].t_end);
    }
  }
  for (index_type l = 0; l < links.length(); l++) {
    Instance::Action& c = instance.actions[steps[links[l].to].act];
    if (c.del.contains(links[l].atom) || 
	c.lck.contains(links[l].atom)) {
      s << "T" << steps[links[l].from].t_end << " -> T"
	<< steps[links[l].to].t_start << " [label=\""
	<< instance.atoms[links[l].atom].name
	<< ": [" << tcn.min_distance(steps[links[l].from].t_end,
				     steps[links[l].to].t_start)
	<< ", " << tcn.max_distance(steps[links[l].from].t_end,
				    steps[links[l].to].t_start)
	<< "]\"];" << std::endl;
      w.add_edge(steps[links[l].from].t_end,
		 steps[links[l].to].t_start);
    }
    else {
      s << "T" << steps[links[l].from].t_end << " -> T"
	<< steps[links[l].to].t_end << " [label=\""
	<< instance.atoms[links[l].atom].name
	<< ": [" << tcn.min_distance(steps[links[l].from].t_end,
				     steps[links[l].to].t_end)
	<< ", " << tcn.max_distance(steps[links[l].from].t_end,
				    steps[links[l].to].t_end)
	<< "]\"];" << std::endl;
      w.add_edge(steps[links[l].from].t_end,
		 steps[links[l].to].t_end);
    }
  }
  for (index_type i = 0; i < tcn.length(); i++)
    for (index_type j = 0; j < tcn.length(); j++)
      if (meg.adjacent(i, j) && !w.adjacent(i, j)) {
	s << "T" << i << " -> T" << j << " [style=dashed];" << std::endl;
      }
  s << "}" << std::endl;
}

END_HSPS_NAMESPACE
