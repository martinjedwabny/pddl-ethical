#ifndef SAS_HEURISTIC_H
#define SAS_HEURISTIC_H

#include "config.h"
#include "sas.h"
#include "heuristic.h"

BEGIN_HSPS_NAMESPACE

// SAS heuristic base class

class SASHeuristic {
 protected:
  int trace_level;

 public:
  SASHeuristic() : trace_level(Heuristic::default_trace_level) { };
  virtual ~SASHeuristic();

  virtual void set_trace_level(int level);

  virtual NTYPE eval(const partial_state& s);
  virtual NTYPE eval_to_bound(const partial_state& s, NTYPE bound);
  virtual void write_eval(const partial_state& s,
			  std::ostream& st,
			  char* p = 0,
			  bool e = true);
  virtual void store(const partial_state& s, NTYPE v);
};

typedef lvector<SASHeuristic*> sas_heuristic_vec;

class SASCostACF : public ACF {
  SASInstance& instance;
 public:
  SASCostACF(SASInstance& ins) : instance(ins) { };
  virtual ~SASCostACF() { };
  virtual NTYPE operator()(index_type a) const;
};


// component heuristic

class MaxH : public SASHeuristic {
 protected:
  sas_heuristic_vec components;

 public:
  MaxH() : components(0, 0) { };
  virtual ~MaxH() { };

  virtual void set_trace_level(int level);
  index_type new_component(SASHeuristic* h);
  index_type n_components() const { return components.length(); };
  SASHeuristic* component(index_type i) { return components[i]; };

  virtual NTYPE eval(const partial_state& s);
  virtual NTYPE eval_to_bound(const partial_state& s, NTYPE bound);

  NTYPE min_component(const partial_state& s);
  void component_values(const partial_state& s, cost_vec& vals);
};

class AddH : public MaxH {
 public:
  AddH() { };
  virtual ~AddH() { };

  virtual NTYPE eval(const partial_state& s);
  virtual NTYPE eval_to_bound(const partial_state& s, NTYPE bound);
};

class MaxAddH : public SASHeuristic {
  SASHeuristic** base;
  NTYPE*         val;
  index_type     n_base;
  index_type*    n_add;
  index_type**   add;
  index_type     n_max;
  index_type*    extra;
  index_type     n_extra;

 public:
  MaxAddH(const sas_heuristic_vec& base_h, const index_set_vec& groups);
  virtual ~MaxAddH() { };

  virtual NTYPE eval(const partial_state& s);
};

// adapter classes

class FwdSASHAdapter : public Heuristic {
  SASInstance&  sas_instance;
  SASHeuristic& sas_heuristic;
  SASHeuristic* owned;

 public:
  FwdSASHAdapter(Instance& ins, SASInstance& sas_ins, SASHeuristic& h)
    : Heuristic(ins), sas_instance(sas_ins), sas_heuristic(h), owned(0) { };
  virtual ~FwdSASHAdapter() { if (owned) delete owned; };

  void set_ownership(SASHeuristic* h) { owned = h; };
  SASHeuristic* get_owned() { return owned; };

  virtual void set_trace_level(int level);

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
  virtual void write_eval(const index_set& s,
			  std::ostream& st,
			  char* p = 0,
			  bool e = true);
  virtual void write_eval(const bool_vec& s,
			  std::ostream& st,
			  char* p = 0,
			  bool e = true);
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);
  virtual NTYPE eval_to_bound(const index_set& s, NTYPE bound);
  virtual NTYPE eval_to_bound(const bool_vec& s, NTYPE bound);

  virtual void store(const index_set& s, NTYPE v, bool opt);
  virtual void store(const bool_vec& s, NTYPE v, bool opt);
};

class BwdSASHAdapter : public Heuristic {
  SASInstance&  sas_instance;
  SASHeuristic& sas_heuristic;

 public:
  BwdSASHAdapter(Instance& ins, SASInstance& sas_ins, SASHeuristic& h)
    : Heuristic(ins), sas_instance(sas_ins), sas_heuristic(h) { };
  virtual ~BwdSASHAdapter() { };

  virtual void set_trace_level(int level);

  virtual NTYPE eval(const index_set& s);
  virtual NTYPE eval(const bool_vec& s);
  virtual void write_eval(const index_set& s,
			  std::ostream& st,
			  char* p = 0,
			  bool e = true);
  virtual void write_eval(const bool_vec& s,
			  std::ostream& st,
			  char* p = 0,
			  bool e = true);
  virtual NTYPE incremental_eval(const index_set& s, index_type i_new);
  virtual NTYPE incremental_eval(const bool_vec& s, index_type i_new);
  virtual NTYPE eval_to_bound(const index_set& s, NTYPE bound);
  virtual NTYPE eval_to_bound(const bool_vec& s, NTYPE bound);

  virtual void store(const index_set& s, NTYPE v, bool opt);
  virtual void store(const bool_vec& s, NTYPE v, bool opt);
};

class ToSASHAdapter : public SASHeuristic {
  SASInstance& sas_instance;
  Heuristic&   heuristic;

 public:
  ToSASHAdapter(SASInstance& sas, Heuristic& h)
    : sas_instance(sas), heuristic(h) { };
  virtual ~ToSASHAdapter() { };

  virtual NTYPE eval(const partial_state& s);
  virtual NTYPE eval_to_bound(const partial_state& s, NTYPE bound);
};

class SASReductionAdapter : public SASHeuristic {
  index_vec     reduce_map;
  SASHeuristic& heuristic;

 public:
  SASReductionAdapter(const index_vec& r, SASHeuristic& h)
    : reduce_map(r), heuristic(h) { };
  virtual ~SASReductionAdapter() { };

  virtual NTYPE eval(const partial_state& s);
  virtual NTYPE eval_to_bound(const partial_state& s, NTYPE bound);
  virtual void store(const partial_state& s, NTYPE v);
};

class SASExpansionAdapter : public SASHeuristic {
  index_vec     expand_map;
  SASHeuristic& heuristic;

 public:
  SASExpansionAdapter(const index_vec& e, SASHeuristic& h)
    : expand_map(e), heuristic(h) { };
  virtual ~SASExpansionAdapter() { };

  virtual NTYPE eval(const partial_state& s);
  virtual NTYPE eval_to_bound(const partial_state& s, NTYPE bound);
  virtual void store(const partial_state& s, NTYPE v);
};

// SAS search state

class SASSeqProgState : public ProgressionState {
 protected:
  SASInstance&  instance;
  const ACF&    cost;
  SASHeuristic& heuristic;
  partial_state state;
  NTYPE         c_est;
  index_type    act;
  NTYPE         c_delta;

  // successor constructor
  SASSeqProgState(SASSeqProgState& p, index_type a);

 public:
  SASSeqProgState(SASInstance& i,
		  const ACF& c,
		  SASHeuristic& h);
  SASSeqProgState(SASInstance& i,
		  const ACF& c,
		  SASHeuristic& h,
		  const partial_state& s0);
  SASSeqProgState(const SASSeqProgState& s);
  virtual ~SASSeqProgState();

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
  virtual void write(::std::ostream& s);
  virtual void write_plan(::std::ostream& s);
};

END_HSPS_NAMESPACE

#endif
