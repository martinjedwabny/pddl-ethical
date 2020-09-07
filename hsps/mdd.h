
#ifndef MDD_H
#define MDD_H

#include "config.h"
#include "cost_table.h"
#include "sas_heuristic.h"

BEGIN_HSPS_NAMESPACE

class MDDNode {
 public:
  enum mdd_node_type {var_node, val_node};
  static MDDNode* terminal;
 private:
  mdd_node_type node_type;
  lvector<MDDNode*> next;

  bool recursive_lookup(const partial_state& s, index_type i) const;
 public:
  MDDNode();
  ~MDDNode();

  bool lookup(const partial_state& s) const
    { return recursive_lookup(s, 0); };
  void insert(const partial_state& s);

  void write_graph(std::ostream& s, bool root = true) const;
};


MDDNode* makeMDD(CostNode* n,
		 const partial_state* map,
		 index_type mapl);

class ConsistencyPostCheck : public SASHeuristic {
  SASHeuristic& h_base;
  MDDNode*      sinc;
 public:
  ConsistencyPostCheck(SASHeuristic& h, MDDNode* s) :
    h_base(h), sinc(s) { assert(sinc); };
  ~ConsistencyPostCheck() { };

  virtual NTYPE eval(const partial_state& s);
  virtual NTYPE eval_to_bound(const partial_state& s, NTYPE bound);
};

END_HSPS_NAMESPACE

#endif
