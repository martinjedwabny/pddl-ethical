#ifndef COLLECTOR_H
#define COLLECTOR_H

#include "config.h"
#include "search.h"
#include "problem.h"

BEGIN_HSPS_NAMESPACE

class Collector : public State, public Search, public Plan {
  Instance& instance;
  State& state;
  bool   sp;
  index_set  pre;
  index_set  nec_del;
  index_set  pos_del;
  index_type max;

  Search* search_p;
  bool    first;
 public:
  Collector(Instance& i, State& s);
  virtual ~Collector() { };

  const index_set& deleted() const { return nec_del; };
  const index_set& nec_deleted() const { return nec_del; };
  const index_set& pos_deleted() const { return pos_del; };
  index_type max_size() const { return max; };

  virtual State* predecessor();
  virtual void set_predecessor(State* p);
  virtual NTYPE delta_cost();
  virtual NTYPE est_cost();
  virtual bool  is_final();
  virtual bool  is_max();
  virtual int compare(const State& s);
  virtual index_type hash();
  virtual State* copy();
  virtual void insert(Plan& p);
  virtual void insert_path(Plan& p);
  virtual void write(std::ostream& s);
  virtual void write_plan(std::ostream& s);
  virtual NTYPE expand(Search& s, NTYPE bound);
  virtual void store(NTYPE cost, bool opt);
  virtual void reevaluate();

  virtual NTYPE new_state(State& s, NTYPE bound);
  virtual bool solved() const;
  virtual bool optimal() const;
  virtual bool done() const;

  virtual void protect(index_type atom);
  virtual void insert(index_type act);
  virtual void advance(NTYPE delta);
  virtual void end();
};

END_HSPS_NAMESPACE

#endif
