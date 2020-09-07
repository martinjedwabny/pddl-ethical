#ifndef STATS_H
#define STATS_H

#include "config.h"
#include "numeric_type.h"
#include "search.h"

BEGIN_HSPS_NAMESPACE

#ifdef SEARCH_EXTRA_STATS
extern count_type rmaxx_count;
extern count_type rmaxx_size;
extern count_type rmaxx_succ;
extern count_type rmaxc_count;
extern count_type rminc_count;
extern count_type rminc_size;
extern count_type rminc_succ_size;
extern double rminc_succ_size_ratio;
extern count_type rminx_count;
extern count_type rminx_size;
extern count_type rminx_succ;
extern count_type trie_count;
extern count_type tries_applicable;
extern count_type tries_within_bound;
#endif

extern const double D_INF;

class Stopwatch {
  double start_t;
  double current_t;
  double total_t;

  static const double TIME_OUT_TOLERANCE = 1.1;

  static bool interrupt_signal_trapped;
  static volatile bool interrupt_signal_raised;
  static bool alarm_signal_trapped;
  static volatile bool alarm_signal_raised;

  bool   interrupt_enabled;
  bool   time_out_enabled;
  double time_out_t;
  bool   memory_limit_enabled;
  unsigned long memory_limit;
  bool   stack_limit_enabled;
  unsigned long stack_limit;

  static void alarm_handler(int sig);
  static void interrupt_handler(int sig);
  static void check_stack();

  void set_interrupt();
  void clear_interrupt();
  void set_alarm(double t);
  void clear_alarm();
  void check_signals();

  static unsigned long peak_mem;
  static unsigned long peak_size;
  static unsigned long peak_stack;
  static unsigned long init_stack;

 protected:
  bool interrupt_flag;
  bool time_out_flag;
  bool out_of_memory_flag;
  bool out_of_stack_flag;
  bool error_flag;
  count_type running;

  bool terminate_on_interrupt;
  bool terminate_on_time_out;
  bool terminate_on_out_of_memory;
  bool terminate_on_out_of_stack;
  bool terminate_on_error;

 public:
  static const long FLAG_INTERRUPTED = 1;
  static const long FLAG_TIME_OUT = 2;
  static const long FLAG_OUT_OF_MEMORY = 4;
  static const long FLAG_OUT_OF_STACK = 8;
  static const long FLAG_ERROR = 16;

#ifdef PROC_EXTRA_STATS
  static unsigned long page_fault_count;
  static unsigned long swap_count;
#endif

  Stopwatch();
  ~Stopwatch();

  static double seconds();

  void enable_interrupt(bool terminate);
  void disable_interrupt();
  void enable_time_out(double t, bool terminate);
  void disable_time_out();
  void enable_memory_limit(unsigned long l, bool terminate);
  void disable_memory_limit();
  void enable_stack_limit(unsigned long l, bool terminate);
  void disable_stack_limit();

  void set_terminate_flags(bool on_interrupt,
			   bool on_time_out,
			   bool on_out_of_memory,
			   bool on_out_of_stack,
			   bool on_error);

  bool interrupt_raised();
  bool time_out_raised();
  bool out_of_memory_raised();
  bool out_of_stack_raised();
  bool error_raised();
  double remaining();
  bool break_signal_raised();

  virtual void interrupt();
  virtual void time_out();
  virtual void out_of_memory();
  virtual void out_of_stack();
  virtual void error();

  virtual void start();
  virtual void stop();
  virtual void reset();
  count_type run_level() { return running; };

  void add(Stopwatch& s);
  void add_total(Stopwatch& s);

  double time();
  double total_time();
  unsigned long peak_memory() const;
  unsigned long peak_total_size() const;
  unsigned long peak_stack_size() const;
  unsigned long flags();
  void print(::std::ostream& s);
};

class Statistics : public Stopwatch {
  count_type min_nodes_created;
  count_type max_nodes_created;
  count_type min_nodes_expanded;
  count_type max_nodes_expanded;
  count_type iterations_started;
  count_type iterations_finished;
  count_type total_min_nodes_created;
  count_type total_max_nodes_created;
  count_type total_min_nodes_expanded;
  count_type total_max_nodes_expanded;
  count_type total_iterations_started;
  count_type total_iterations_finished;
  index_type max_depth;
  NTYPE      max_lb;
  count_type nodes_to_prove_lb;

 public:
  static bool long_print_format;
  static bool running_print_max;

  Statistics()
    : min_nodes_created(0), max_nodes_created(0),
    min_nodes_expanded(0), max_nodes_expanded(0),
    iterations_started(0), iterations_finished(0),
    total_min_nodes_created(0), total_max_nodes_created(0),
    total_min_nodes_expanded(0), total_max_nodes_expanded(0),
    total_iterations_started(0), total_iterations_finished(0),
    max_depth(0), max_lb(0), nodes_to_prove_lb(0)
    { };

  void create_node(State& s);
  void expand_node(State& s);
  void current_lower_bound(NTYPE b);
  void begin_iteration();
  void end_iteration();

  virtual void start();
  virtual void stop();
  virtual void reset();

  virtual void time_out();
  virtual void interrupt();
  virtual void out_of_memory();
  virtual void out_of_stack();

  void add(Statistics& s);
  void add_nodes(Statistics& s);
  void add_total(Statistics& s);
  void add_nodes_total(Statistics& s);

  double branching_factor() const {
    return ((min_nodes_created + max_nodes_created)/
	    ((double)(min_nodes_expanded + max_nodes_expanded)));
  };

  count_type nodes() const {
    return (min_nodes_expanded + max_nodes_expanded);
  };

  count_type total_nodes() const {
    if (running)
      return (total_min_nodes_expanded + total_max_nodes_expanded +
	      min_nodes_expanded + max_nodes_expanded);
    else
      return (total_min_nodes_expanded + total_max_nodes_expanded);
  };

  count_type total_min_nodes() const {
    if (running) return (total_min_nodes_expanded + min_nodes_expanded);
    else return total_min_nodes_expanded;
  };

  count_type total_max_nodes() const {
    if (running) return (total_max_nodes_expanded + max_nodes_expanded);
    else return total_max_nodes_expanded;
  };

  NTYPE max_lower_bound() const {
    return max_lb;
  };

  count_type nodes_at_max_lower_bound() const {
    return nodes_to_prove_lb;
  };

  count_type iterations() {
    return iterations_started;
  };

  count_type complete_iterations() {
    return iterations_finished;
  };

  count_type total_iterations() {
    if (running) return iterations_started + total_iterations_started;
    else return total_iterations_started;
  };

  count_type total_complete_iterations() {
    if (running) return iterations_finished + total_iterations_finished;
    else return total_iterations_finished;
  };

  void print_brief(::std::ostream& s, const char* p = 0);
  void print(::std::ostream& s, const char* p = 0);
  void print_total(::std::ostream& s, const char* p = 0);
};

inline ::std::ostream& operator<<(::std::ostream& s, Stopwatch& t) {
  return s << t.total_time();
}

inline ::std::ostream& operator<<(::std::ostream& s, Statistics& t) {
  if (Statistics::long_print_format) {
    t.print_total(s);
    return s;
  }
  else {
    return s << t.total_nodes() << " nodes, "
	     << t.total_time() << " seconds, "
	     << t.peak_memory() << "k heap, "
	     << t.peak_stack_size() << "k stack";
  }
}

END_HSPS_NAMESPACE

#endif
