#ifndef POP_H
#define POP_H

#include "config.h"
#include "exec.h"
#include "tcn.h"

BEGIN_HSPS_NAMESPACE

class SafePOP {
 public:
  int trace_level;

  struct step {
    index_type index;
    index_type act;
    index_type t_start;
    index_type t_end;

    step() : index(no_such_index), act(no_such_index),
	 t_start(no_such_index), t_end(no_such_index) {
    };
    step(index_type i, index_type a, index_type t0, index_type t1)
      : index(i), act(a), t_start(t0), t_end(t1) {
    };

    step& operator=(const step& s) {
      index = s.index;
      act = s.act;
      t_start = s.t_start;
      t_end = s.t_end;
      return *this;
    };

    bool operator==(const step& s) {
      return ((index == s.index) &&
	      (act == s.act) &&
	      (t_start == s.t_start) &&
	      (t_end == s.t_end));
    };
  };

  typedef lvector<step> step_vec;

  struct causal_link {
    index_type atom; // atom index
    index_type from; // step index of establisher
    index_type to;   // step index of consumer

    causal_link()
      : atom(no_such_index), from(no_such_index), to(no_such_index) {
    };
    causal_link(index_type p, index_type e, index_type c)
      : atom(p), from(e), to(c) {
    };
    causal_link(const causal_link& l)
      : atom(l.atom), from(l.from), to(l.to) {
    };

    causal_link& operator=(const causal_link& l) {
      atom = l.atom;
      from = l.from;
      to = l.to;
      return *this;
    };

    bool operator==(const causal_link& l) const {
      return ((atom == l.atom) && (from == l.from) && (to == l.to));
    };

    bool operator!=(const causal_link& l) const {
      return (!(*this == l));
    };

    bool operator<(const causal_link& l) const {
      return ((to < l.to) ||
	      ((to == l.to) && (atom < l.atom)) ||
	      ((to == l.to) && (atom == l.atom) && (from < l.from)));
    };

    bool operator>(const causal_link& l) const {
      return ((to > l.to) ||
	      ((to == l.to) && (atom > l.atom)) ||
	      ((to == l.to) && (atom == l.atom) && (from > l.from)));
    };
  };

  typedef svector<causal_link> link_set;

  Instance& instance;
  step_vec  steps;
  static const index_type INIT_STEP = 0;
  static const index_type GOAL_STEP = 1;
  link_set  links;
  STN       tcn;

  SafePOP(Instance& i);
  SafePOP(Instance& i, const Schedule::step_vec& s_steps);
  ~SafePOP();

  void construct(const Schedule::step_vec& s_steps,
		 bool deorder,
		 bool assume_sequential);
  void find_safe_causal_links();
  void enforce_min_durations();
  void enforce_max_durations();
  void enforce_makespan(NTYPE b);

  void write(std::ostream& s);
  void write_graph(std::ostream& s);
};

END_HSPS_NAMESPACE

#endif
