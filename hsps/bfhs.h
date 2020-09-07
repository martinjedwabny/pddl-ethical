#ifndef BFHS_H
#define BFHS_H

#include "config.h"
#include "search_base.h"
#include "nodeset.h"

BEGIN_HSPS_NAMESPACE

class BFHS : public SearchAlgorithm {
 protected:
  index_type layer_table_size;
  HashNodeSet* previous_layer;
  HashNodeSet* current_layer;
  HashNodeSet* next_layer;
  lvector<Node*> current_open;
  lvector<Node*> next_open;
  NTYPE current_layer_cost;
  NTYPE least_over_bound;

  count_type acc_succ;
  count_type new_succ;

 public:
  BFHS(Statistics& s, SearchResult& r, index_type lt_size);
  virtual ~BFHS();

  virtual NTYPE start(State& s, NTYPE b);
  virtual NTYPE start(State& s);

  NTYPE main(State& s);
  virtual NTYPE new_state(State& s, NTYPE bound);

  virtual NTYPE cost() const;
  virtual bool done() const;
};

class BFIDA : public BFHS {
 public:
  BFIDA(Statistics& s, SearchResult& r, index_type lt_size);
  virtual ~BFIDA();

  virtual NTYPE start(State& s, NTYPE b);
  virtual NTYPE start(State& s);
};

class BFHS_XD : public BFHS {
 public:
  BFHS_XD(Statistics& s, SearchResult& r, index_type lt_size);
  virtual ~BFHS_XD();

  virtual NTYPE start(State& s, NTYPE b);
  virtual NTYPE start(State& s);
};

END_HSPS_NAMESPACE

#endif
