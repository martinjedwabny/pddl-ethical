#ifndef HTN_H
#define HTN_H

#include "config.h"
#include "problem.h"
#include "search.h"
#include "atomset.h"

BEGIN_HSPS_NAMESPACE

class HTNInstance : public Instance {
 public:
  static bool write_HTN;

  struct step {
    bool       abstract;
    index_type index;

    step() : abstract(false), index(no_such_index) { };
    step(index_type i) : abstract(false), index(i) { };
    step(bool a, index_type i) : abstract(a), index(i) { };

    step& operator=(const step& s) {
      abstract = s.abstract;
      index = s.index;
      return *this;
    };

    bool operator==(const step& s) const {
      return ((abstract == s.abstract) && (index == s.index));
    };

    bool operator<(const step& s) const {
      return ((!abstract && s.abstract) ||
	      ((abstract == s.abstract) && (index < s.index)));
    };
  };

  class step_vec : public lvector<step> {
   public:
    step_vec() : lvector<step>() { };
    step_vec(const step& s) : lvector<step>(s, 0) { };
    step_vec(const step& s, index_type l) : lvector<step>(s, l) { };
    step_vec(const step_vec& s) : lvector<step>(s) { };

    bool operator<(const step_vec& s) const {
      index_type i = 0;
      while ((i < length()) && (i < s.length())) {
	if ((*this)[i] < s[i]) return true;
	else if (s[i] < (*this)[i]) return false;
	i += 1;
      }
      if (length() < s.length()) return true;
      return false;
    };
  };

  struct Method {
    index_set pre;
    step_vec  steps;

   public:
    Method() { };

    bool operator==(const Method& m) const {
      return ((pre == m.pre) && (steps == m.steps));
    };

    Method& operator=(const Method& m) {
      pre = m.pre;
      steps = m.steps;
      return *this;
    };

    bool operator<(const Method& m) const {
      return ((pre < m.pre) || ((pre == m.pre) && (steps < m.steps)));
    };
  };

  class method_set : public svector<Method> {
   public:
    method_set() : svector<Method>() { };
    method_set(const method_set& s) : svector<Method>(s) { };
  };

  struct Task {
    Name*      name;
    index_type index;
    method_set exp;

    Task() : name(0), index(0) { };
    Task(Name* n) : name(n), index(0) { };
    Task(Name* n, index_type i) : name(n), index(i) { };

    Task& operator=(const Task& m) {
      name = m.name;
      index = m.index;
      exp = m.exp;
      return *this;
    };

    bool operator==(const Task& m) {
      return (index == m.index);
    };
  };

  typedef lvector<Task> task_vec;

  task_vec tasks;
  step_vec goal_tasks;

  HTNInstance();
  HTNInstance(Name* n);
  HTNInstance(const Instance& ins);
  HTNInstance(const HTNInstance& ins);
  ~HTNInstance() { };

  // build (add to) instance
  Task& new_task(Name* name);

  // change instance
  void remove_actions(const bool_vec& set, index_vec& map);
  void set_goal_tasks(const step_vec& g);

  // access instance information
  index_type n_tasks() const { return tasks.length(); };
  void task_names(name_vec& names) const;

  // write utilities
  void write_step_sequence(std::ostream& s, const step_vec& p) const;

  virtual void write_domain(std::ostream& s) const;
  virtual void write_problem_goal(std::ostream& s) const;

  void write_abstract_plan(std::ostream& s, const step_vec& p) const;
};

class FwdSeqHTNState : public ProgressionState, public AtomSet {
  HTNInstance& instance;
  HTNInstance::step_vec rem;

 public:
  FwdSeqHTNState(HTNInstance& i);
  FwdSeqHTNState(HTNInstance& i,
		 const index_set& s,
		 const HTNInstance::step_vec& g);
  FwdSeqHTNState(const FwdSeqHTNState& s);
  virtual ~FwdSeqHTNState();

  virtual NTYPE delta_cost();
  virtual NTYPE est_cost();
  virtual bool  is_final();
  virtual bool  is_max();

  virtual NTYPE expand(Search& s, NTYPE bound);

  virtual void store(NTYPE cost, bool opt);
  virtual void reevaluate();
  virtual int compare(const State& s);
  virtual index_type hash();
  virtual State* copy();
  virtual void insert(Plan& p);

  virtual void write(std::ostream& s);
  virtual void write_plan(std::ostream& s);
};

END_HSPS_NAMESPACE

#endif
