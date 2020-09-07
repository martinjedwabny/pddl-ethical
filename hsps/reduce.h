#ifndef REDUCE_H
#define REDUCE_H

#include "config.h"
#include "problem.h"
#include "preprocess.h"
#include "search_base.h"

BEGIN_HSPS_NAMESPACE

class Reduce : public Preprocessor {
 public:
  bool_vec   deleted;
  index_type n_redundant;
  index_type n_useless;

  Reduce(Instance& ins, Statistics& s)
    : Preprocessor(ins, s), deleted(false, ins.n_actions()) { };
  ~Reduce() { };

  NTYPE implement(index_type act, NTYPE bound, Result& imps);
  index_pair count_useless_and_redundant_actions(NTYPE bound);
  void reduced_action_set(NTYPE bound, bool_vec& deleted);
  void reduce(NTYPE bound);
};

END_HSPS_NAMESPACE

#endif
