
#include "minimax.h"

BEGIN_HSPS_NAMESPACE

// #define TRACE_PRINT_LOTS

MiniMax::MiniMax(Statistics& s, index_type d)
  : stats(s), depth(d)
{
  // does nothing
}

MiniMax::~MiniMax()
{
  // does nothing
}

NTYPE MiniMax::start(State& s)
{
  stats.start();
  NTYPE val = new_state(s, POS_INF);
  stats.stop();
  return val;
}

NTYPE MiniMax::start(State& s, index_type d)
{
  depth = d;
  return start(s);
}

NTYPE MiniMax::new_state(State& s, NTYPE bound)
{
  stats.create_node(s);
  NTYPE val = 0;
  if ((s.depth() >= depth) || s.is_final()) {
    val = s.acc_cost() + s.est_cost();
#ifdef TRACE_PRINT_LOTS
    std::cerr << "leaf (" << s.depth() << ": " << s << "): "
	      << s.acc_cost() << " + " << s.est_cost()
	      << std::endl;
#endif
  }
  else {
    stats.expand_node(s);
    val = s.expand(*this, POS_INF);
#ifdef TRACE_PRINT_LOTS
    std::cerr << (s.is_max() ? "max" : "min")
	      << " (" << s.depth() << ": " << s << "): "
	      << val << " (" << s.acc_cost() << " + " << s.est_cost() << ")"
	      << std::endl;
#endif
  }
  return val - s.delta_cost();
}

bool MiniMax::solved() const
{
  return false;
}

bool MiniMax::optimal() const
{
  return false;
}

bool MiniMax::done() const
{
  return stats.break_signal_raised();
}


AlphaBeta::AlphaBeta(Statistics& s, index_type d)
  : MiniMax(s, d), pre_is_max(true), alpha(0), beta(POS_INF), cut(false), tt(0)
{
  // does nothing
}

AlphaBeta::AlphaBeta
(Statistics& s, index_type d, HashTable* ttab)
  : MiniMax(s, d), pre_is_max(true), alpha(0), beta(POS_INF), cut(false), tt(ttab)
{
  // does nothing
}

AlphaBeta::~AlphaBeta()
{
  // does nothing
}

NTYPE AlphaBeta::start(State& s)
{
  stats.start();
  alpha = s.est_cost();
  beta = POS_INF;
  if (alpha >= beta) return s.est_cost();
  pre_is_max = s.is_max();
  NTYPE val = s.expand(*this, POS_INF);
  stats.stop();
  return MAX(val, s.est_cost());
}

NTYPE AlphaBeta::new_state(State& s, NTYPE bound)
{
  stats.create_node(s);
  NTYPE s_val = s.acc_cost() + s.est_cost();
//   HashTable::Entry* stored = tt->find(s);
//   if (stored) {
//     if (stored->depth >= (depth - s.depth()))
//       s_val = s.acc_cost() + stored->cost;
//   }
  NTYPE val = 0;

  if ((s.depth() >= depth) || s.is_final()) {
    val = s_val;
#ifdef TRACE_PRINT_LOTS
    std::cerr << "leaf (" << s.depth() << ": " << s << "): "
	      << s.acc_cost() << " + " << s.est_cost()
	      << std::endl;
#endif
  }

  else {
    if (MAX(alpha,s_val) >= beta) {
      val = s_val;
    }
    else {
      stats.expand_node(s);
      // save local variables
      NTYPE r_alpha = alpha;
      NTYPE r_beta = beta;
      bool  r_is_max = pre_is_max;
      // set alpha and pre_is_max
      alpha = MAX(alpha, s_val);
      pre_is_max = s.is_max();
      NTYPE x_val = s.expand(*this, POS_INF);
      // restore local variables
      alpha = r_alpha;
      beta = r_beta;
      pre_is_max = r_is_max;
      cut = false;
      // calc. value of implicit max node
      val = MAX(s_val, x_val);
    }
#ifdef TRACE_PRINT_LOTS
    std::cerr << (s.is_max() ? "max" : "min")
	      << " (" << s.depth() << ": " << s << "): "
	      << val << " (" << s.acc_cost() << " + " << s.est_cost() << ")"
	      << std::endl;
#endif
  }

  if (pre_is_max) {
    alpha = MAX(alpha, val);
#ifdef TRACE_PRINT_LOTS
    std::cerr << "p. max (" << s.depth() - 1 << "): alpha = " << alpha
	      << ", beta = " << beta << std::endl;
#endif
    if (alpha >= beta) {
      cut = true;
    }
  }
  else {
    beta = MIN(beta, val);
#ifdef TRACE_PRINT_LOTS
    std::cerr << "p. min (" << s.depth() - 1 << "): alpha = " << alpha
	      << ", beta = " << beta << std::endl;
#endif
    if (beta <= alpha) {
      cut = true;
    }
  }

#ifdef POINTLESS
  if (val < s_val) {
    std::cerr << "error: value " << val
	      << " with depth " << depth - s.depth()
	      << " < static value " << s_val
	      << " for state " << s
	      << std::endl;
    exit(255);
  }
#endif
//   if (stored) {
//     if ((val - s.acc_cost()) > stored->cost)
//       stored->cost = (val - s.acc_cost());
//   }
//   else if ((val - s.acc_cost()) > s.est_cost()) {
//     tt->insert(s, val - s.acc_cost(), depth);
//   }
  return val - s.delta_cost();
}

bool AlphaBeta::done() const
{
  return (cut || stats.break_signal_raised());
}

NTYPE LookAhead::eval(const index_set& s)
{
  State* root_state = space.new_state(s, 0);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "evaluating " << *root_state
	    << " (static value = " << root_state->est_cost()
	    << ") to depth " << depth << "..."
	    << std::endl;
#endif
  NTYPE val = start(*root_state);
  if (val < root_state->est_cost()) {
    std::cerr << "error: " << depth << " look ahead heuristic = "
	      << val << " < base heuristic = " << root_state->est_cost()
	      << " for state " << *root_state << std::endl;
    exit(255);
  }
  delete root_state;
  return val;
}

NTYPE LookAhead::eval(const bool_vec& s)
{
  State* root_state = space.new_state(s, 0);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "evaluating " << *root_state
	    << " (static value = " << root_state->est_cost()
	    << ") to depth " << depth << "..."
	    << std::endl;
#endif
  NTYPE val = start(*root_state);
  if (val < root_state->est_cost()) {
    std::cerr << "error: " << depth << " look ahead heuristic = "
	      << val << " < base heuristic = " << root_state->est_cost()
	      << " for state " << *root_state << std::endl;
    exit(255);
  }
  delete root_state;
  return val;
}

NTYPE LookAheadAB::eval(const index_set& s)
{
  State* root_state = space.new_state(s, 0);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "evaluating " << *root_state
	    << " (static value = " << root_state->est_cost()
	    << ") to depth " << depth << "..."
	    << std::endl;
#endif
  NTYPE val = start(*root_state);
  if (val < root_state->est_cost()) {
    std::cerr << "error: " << depth << " look ahead heuristic = "
	      << val << " < base heuristic = " << root_state->est_cost()
	      << " for state " << *root_state << std::endl;
    exit(255);
  }
  delete root_state;
  return val;
}

NTYPE LookAheadAB::eval(const bool_vec& s)
{
  State* root_state = space.new_state(s, 0);
#ifdef TRACE_PRINT_LOTS
  std::cerr << "evaluating " << *root_state
	    << " (static value = " << root_state->est_cost()
	    << ") to depth " << depth << "..."
	    << std::endl;
#endif
  NTYPE val = start(*root_state);
  if (val < root_state->est_cost()) {
    std::cerr << "error: " << depth << " look ahead heuristic = "
	      << val << " < base heuristic = " << root_state->est_cost()
	      << " for state " << *root_state << std::endl;
    exit(255);
  }
  delete root_state;
  return val;
}

END_HSPS_NAMESPACE
