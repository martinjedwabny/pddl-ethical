
#include "stats.h"
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include <alloca.h>
#ifdef RSS_FROM_PROCFS_PSINFO
#include <fcntl.h>
#include <procfs.h>
#include <sstream>
#endif
#ifdef RSS_FROM_PROCFS_STAT
#include <sstream>
#include <fstream>
#endif
#ifdef RSS_FROM_MALLINFO
#include <malloc.h>
#endif
#include <math.h>

// #define ABORT_ON_INTERRUPT

BEGIN_HSPS_NAMESPACE

#ifdef SEARCH_EXTRA_STATS
count_type rmaxx_count = 0;
count_type rmaxx_size = 0;
count_type rmaxx_succ = 0;
count_type rmaxc_count = 0;
count_type rminc_count = 0;
count_type rminc_size = 0;
count_type rminc_succ_size = 0;
double rminc_succ_size_ratio = 0.0;
count_type rminx_count = 0;
count_type rminx_size = 0;
count_type rminx_succ = 0;
count_type trie_count = 0;
count_type tries_applicable = 0;
count_type tries_within_bound = 0;
#endif

const double D_INF = ((double)(1.0/0.0));

unsigned long Stopwatch::peak_mem = 0;
unsigned long Stopwatch::peak_size = 0;
unsigned long Stopwatch::peak_stack = 0;
unsigned long Stopwatch::init_stack = 0;
#ifdef PROC_EXTRA_STATS
unsigned long Stopwatch::page_fault_count = 0;
unsigned long Stopwatch::swap_count = 0;
#endif

bool Statistics::long_print_format = false;
bool Statistics::running_print_max = false;

bool Stopwatch::interrupt_signal_trapped = false;
volatile bool Stopwatch::interrupt_signal_raised = false;
bool Stopwatch::alarm_signal_trapped = false;
volatile bool Stopwatch::alarm_signal_raised = false;

double Stopwatch::seconds()
{
  rusage u;
  getrusage(RUSAGE_SELF, &u);
#ifdef RSS_FROM_PROCFS_PSINFO
  ::std::ostringstream fn;
  fn << "/proc/" << getpid() << "/psinfo";
  int fd = open(fn.str().c_str(), O_RDONLY);
  if (fd < 0) {
    ::std::cerr << "error " << errno << " opening " << fn.str() << ::std::endl;
    exit(255);
  }
  psinfo_t psinfo_data;
  int res = read(fd, &psinfo_data, sizeof(psinfo_t));
  if (res < 0) {
    ::std::cerr << "error " << errno << " reading " << fn.str() << ::std::endl;
    exit(255);
  }
  if (res < sizeof(psinfo_t)) {
    ::std::cerr << "error: only " << res << " bytes of " << sizeof(psinfo_t)
	      << " read from " << fn.str() << ::std::endl;
    exit(255);
  }
  close(fd);
  if (psinfo_data.pr_rssize > Stopwatch::peak_mem) {
    Stopwatch::peak_mem = psinfo_data.pr_rssize;
  }
  if (psinfo_data.pr_size > Stopwatch::peak_size) {
    Stopwatch::peak_size = psinfo_data.pr_size;
  }
#endif
#ifdef RSS_FROM_PROCFS_STAT
  ::std::ostringstream fn;
  fn << "/proc/" << getpid() << "/stat";
  std::ifstream ps_in(fn.str().c_str());
  std::string item;
  for (index_type n = 0; n < 23; n++) {
    ps_in >> item;
    // std::cerr << "item #" << n << " = \"" << item << "\"" << std::endl;
  }
  ps_in.close();
  long vm = atoi(item.c_str()) / 1024;
  if (vm > Stopwatch::peak_mem) {
    Stopwatch::peak_mem = vm;
  }
  if (vm > Stopwatch::peak_size) {
    Stopwatch::peak_size = vm;
  }
#endif
#ifdef RSS_FROM_MALLINFO
  struct mallinfo mi = mallinfo();
  count_type s = (mi.arena / 1024);
  if (s > Stopwatch::peak_mem) {
    Stopwatch::peak_mem = s;
  }
#endif
#ifdef RSS_FROM_RUSAGE_MAX
  if (u.ru_maxrss > Stopwatch::peak_mem) {
    Stopwatch::peak_mem = u.ru_maxrss;
  }
#endif
  check_stack();
#ifdef PROC_EXTRA_STATS
  Stopwatch::page_fault_count = u.ru_majflt;
  Stopwatch::swap_count = u.ru_nswap;
#endif
  return (u.ru_utime.tv_sec + (u.ru_utime.tv_usec / 1000000.0));
}

Stopwatch::Stopwatch()
  : start_t(0.0),
    current_t(0.0),
    total_t(0.0),
    interrupt_enabled(false),
    time_out_enabled(false),
    time_out_t(0.0),
    memory_limit_enabled(false),
    memory_limit(0),
    stack_limit_enabled(false),
    stack_limit(0),
    interrupt_flag(false),
    time_out_flag(false),
    out_of_memory_flag(false),
    out_of_stack_flag(false),
    error_flag(false),
    running(0),
    terminate_on_interrupt(false),
    terminate_on_time_out(false),
    terminate_on_out_of_memory(false),
    terminate_on_out_of_stack(false),
    terminate_on_error(false)
{
  // done
}

Stopwatch::~Stopwatch()
{
  // done
}

void Stopwatch::alarm_handler(int sig)
{
  alarm_signal_raised = true;
}

void Stopwatch::interrupt_handler(int sig)
{
#ifdef EXIT_ON_SECOND_INTERRUPT
  if (interrupt_signal_raised) exit(255);
#endif
  interrupt_signal_raised = true;
#ifdef ABORT_ON_INTERRUPT
  abort();
#endif
#ifdef EXIT_ON_SECOND_INTERRUPT
  signal(SIGINT, Stopwatch::interrupt_handler);
#endif
}

void Stopwatch::set_alarm(double t)
{
#ifdef PERIODIC_RESOURCE_CHECK
  if (t > PERIODIC_CHECK_INTERVAL)
    t = PERIODIC_CHECK_INTERVAL;
#else
  if ((memory_limit_enabled || stack_limit_enabled) &&
      (t > PERIODIC_CHECK_INTERVAL))
    t = PERIODIC_CHECK_INTERVAL;
#endif
  unsigned int sec = (unsigned int)floor(t);
  if (sec > 0) {
    alarm_signal_trapped = true;
    alarm_signal_raised = false;
    signal(SIGALRM, Stopwatch::alarm_handler);
    alarm(sec);
  }
  else if (time_out_enabled) {
    alarm_signal_trapped = false;
    alarm_signal_raised = false;
    time_out();
  }
}

void Stopwatch::clear_alarm()
{
  alarm_signal_trapped = false;
  alarm_signal_raised = false;
}

void Stopwatch::set_interrupt()
{
  signal(SIGINT, Stopwatch::interrupt_handler);
  interrupt_signal_trapped = true;
  interrupt_signal_raised = false;
}

void Stopwatch::clear_interrupt()
{
  signal(SIGINT, SIG_DFL);
  interrupt_signal_trapped = false;
  interrupt_signal_raised = false;
}

void Stopwatch::check_signals()
{
  if (interrupt_signal_trapped && interrupt_signal_raised) {
    signal(SIGINT, SIG_DFL);
    interrupt();
  }
  if (alarm_signal_trapped && alarm_signal_raised) {
    double r = remaining();
    if (time_out_enabled && (r < TIME_OUT_TOLERANCE)) {
      time_out();
    }
    else if (memory_limit_enabled && (peak_memory() > memory_limit)) {
      out_of_memory();
    }
    else if (stack_limit_enabled && (peak_stack_size() > stack_limit)) {
      out_of_stack();
    }
    else {
      set_alarm(r);
    }
  }
#ifdef HI_FREQ_STACK_CHECK
  check_stack();
#endif
}

void Stopwatch::check_stack()
{
  unsigned long current_stack = (unsigned long)alloca(4);
  if (init_stack == 0) {
    init_stack = current_stack;
  }
  else {
    if (current_stack < init_stack) {
      unsigned long used_stack = ((init_stack - current_stack) / 1024);
      if (used_stack > peak_stack) {
	peak_stack = used_stack;
      }
    }
    else {
      unsigned long used_stack = ((current_stack - init_stack) / 1024);
      if (used_stack > peak_stack) {
	peak_stack = used_stack;
      }
    }
  }
}

void Stopwatch::enable_interrupt(bool terminate)
{
  interrupt_enabled = true;
  terminate_on_interrupt = terminate;
}

void Stopwatch::disable_interrupt()
{
  interrupt_enabled = false;
}

void Stopwatch::enable_time_out(double t, bool terminate)
{
  time_out_enabled = true;
  time_out_t = total_time() + t;
  time_out_flag = false;
  terminate_on_time_out = terminate;
}

void Stopwatch::disable_time_out()
{
  time_out_enabled = false;
}

void Stopwatch::enable_memory_limit(unsigned long l, bool terminate)
{
  memory_limit_enabled = true;
  memory_limit = l;
  out_of_memory_flag = false;
  terminate_on_out_of_memory = terminate;
}

void Stopwatch::disable_memory_limit()
{
  memory_limit_enabled = false;
}

void Stopwatch::enable_stack_limit(unsigned long l, bool terminate)
{
  stack_limit_enabled = true;
  stack_limit = l;
  out_of_stack_flag = false;
  terminate_on_out_of_stack = terminate;
}

void Stopwatch::disable_stack_limit()
{
  stack_limit_enabled = false;
}

void Stopwatch::set_terminate_flags
(bool on_int, bool on_to, bool on_mo, bool on_so, bool on_err)
{
  terminate_on_interrupt = on_int;
  terminate_on_time_out = on_to;
  terminate_on_out_of_memory = on_mo;
  terminate_on_out_of_stack = on_so;
  terminate_on_error = on_err;
}

bool Stopwatch::interrupt_raised()
{
  if (interrupt_signal_trapped) check_signals();
  return interrupt_flag;
}

bool Stopwatch::time_out_raised()
{
  if (alarm_signal_trapped) check_signals();
  return time_out_flag;
}

bool Stopwatch::out_of_memory_raised()
{
  if (alarm_signal_trapped) check_signals();
  return (memory_limit_enabled && (peak_memory() > memory_limit));
}

bool Stopwatch::out_of_stack_raised()
{
  if (alarm_signal_trapped) check_signals();
  return (stack_limit_enabled && (peak_stack_size() > stack_limit));
}

bool Stopwatch::error_raised()
{
  return error_flag;
}

bool Stopwatch::break_signal_raised()
{
  if (interrupt_signal_trapped || alarm_signal_trapped) check_signals();
  return (interrupt_flag ||
	  time_out_flag ||
	  (memory_limit_enabled && (peak_memory() > memory_limit)) ||
	  (stack_limit_enabled && (peak_stack_size() > stack_limit)) ||
	  error_flag);
}

double Stopwatch::remaining()
{
  if (time_out_enabled) {
    return time_out_t - total_time();
  }
  else {
    seconds();
    return D_INF;
  }
}

void Stopwatch::start()
{
  if (running == 0) {
    start_t = seconds();
    if (interrupt_enabled) set_interrupt();
#ifdef PERIODIC_RESOURCE_CHECK
    set_alarm(remaining());
#else
    if (time_out_enabled || memory_limit_enabled || stack_limit_enabled)
      set_alarm(remaining());
#endif
  }
  running += 1;
}

void Stopwatch::stop()
{
  if (running == 0) return;
  running -= 1;
  if (running == 0) {
    current_t = seconds() - start_t;
    if (current_t < 0.0) current_t = 0.0;
    total_t += current_t;
    if (alarm_signal_trapped) clear_alarm();
    if (interrupt_enabled) clear_interrupt();
  }
}

void Stopwatch::reset()
{
  running = 0;
  start_t = 0.0;
  current_t = 0.0;
  total_t = 0.0;
}

void Stopwatch::time_out()
{
  if (!time_out_flag) {
    ::std::cerr << "time-out: " << total_time() << ::std::endl;
    time_out_flag = true;
  }
  if (terminate_on_time_out) exit(0);
}

void Stopwatch::out_of_memory()
{
  if (!out_of_memory_flag) {
    ::std::cerr << "memory limit exceeded: " << peak_memory() << ::std::endl;
    out_of_memory_flag = true;
  }
  if (terminate_on_out_of_memory) exit(0);
}

void Stopwatch::out_of_stack()
{
  if (!out_of_stack_flag) {
    ::std::cerr << "stack limit exceeded: " << peak_stack_size() << ::std::endl;
    out_of_stack_flag = true;
  }
  if (terminate_on_out_of_stack) exit(0);
}

void Stopwatch::interrupt()
{
  if (!interrupt_flag) {
    ::std::cerr << "interrupt: " << total_time() << ::std::endl;
    interrupt_flag = true;
  }
  if (terminate_on_interrupt) exit(0);
}

void Stopwatch::error()
{
  if (!error_flag) {
    // ::std::cerr << "error!" << ::std::endl;
    error_flag = true;
  }
  if (terminate_on_error) exit(0);
}

void Stopwatch::add(Stopwatch& s)
{
  total_t += s.time();
}

void Stopwatch::add_total(Stopwatch& s)
{
  total_t += s.total_time();
}

double Stopwatch::time()
{
  if (running > 0) {
    current_t = seconds() - start_t;
    if (current_t < 0.0) current_t = 0.0;
  }
  return current_t;
}

double Stopwatch::total_time()
{
  if (running > 0) return total_t + time();
  else return total_t;
}

unsigned long Stopwatch::peak_memory() const
{
  return peak_mem;
}

unsigned long Stopwatch::peak_total_size() const
{
#if (defined(RSS_FROM_PROCFS_PSINFO) || defined(RSS_FROM_PROCFS_STAT))
  return peak_size;
#else
  return peak_mem + peak_stack;
#endif
}

unsigned long Stopwatch::peak_stack_size() const
{
  return peak_stack;
}

unsigned long Stopwatch::flags()
{
  return ((interrupt_raised() ? FLAG_INTERRUPTED : 0) +
	  (time_out_raised() ? FLAG_TIME_OUT : 0) +
	  (out_of_memory_raised() ? FLAG_OUT_OF_MEMORY : 0) +
	  (out_of_stack_raised() ? FLAG_OUT_OF_STACK : 0) +
	  (error_raised() ? FLAG_ERROR : 0));
}

void Statistics::create_node(State& s)
{
  if (s.is_max()) max_nodes_created += 1;
  else min_nodes_created += 1;
  index_type d = s.depth();
  if (d > max_depth) {
    max_depth = d;
    if (running_print_max) {
      ::std::cerr << "max depth: " << d << " (" << *this << ")" << ::std::endl;
    }
  }
}

void Statistics::expand_node(State& s)
{
  if (s.is_max()) max_nodes_expanded += 1;
  else min_nodes_expanded += 1;
}

void Statistics::current_lower_bound(NTYPE b)
{
  if (b > max_lb) {
    max_lb = b;
    nodes_to_prove_lb = nodes();
#ifdef PRINT_EVOLUTION
    ::std::cerr << "EVO " << max_lb
	      << " " << nodes()
	      << " " << time()
	      << ::std::endl;
#endif
  }
}

void Statistics::begin_iteration()
{
  iterations_started += 1;
}

void Statistics::end_iteration()
{
  iterations_finished += 1;
}

void Statistics::start()
{
  if (running == 0) {
    min_nodes_created = 0;
    max_nodes_created = 0;
    min_nodes_expanded = 0;
    max_nodes_expanded = 0;
    iterations_started = 0;
    iterations_finished = 0;
  }
  Stopwatch::start();
}

void Statistics::stop()
{
  Stopwatch::stop();
  if (running == 0) {
    total_min_nodes_created += min_nodes_created;
    total_max_nodes_created += max_nodes_created;
    total_min_nodes_expanded += min_nodes_expanded;
    total_max_nodes_expanded += max_nodes_expanded;
    total_iterations_started += iterations_started;
    total_iterations_finished += iterations_finished;
  }
}

void Statistics::reset()
{
  Stopwatch::reset();
  min_nodes_created = 0;
  max_nodes_created = 0;
  min_nodes_expanded = 0;
  max_nodes_expanded = 0;
  iterations_started = 0;
  iterations_finished = 0;
  total_min_nodes_created = 0;
  total_max_nodes_created = 0;
  total_min_nodes_expanded = 0;
  total_max_nodes_expanded = 0;
  total_iterations_started = 0;
  total_iterations_finished = 0;
  max_depth = 0;
  max_lb = 0;
  nodes_to_prove_lb = 0;
}

void Statistics::time_out()
{
  if (!time_out_flag) {
    ::std::cerr << "time-out: " << *this << ::std::endl;
    time_out_flag = true;
  }
  if (terminate_on_time_out) exit(0);
}

void Statistics::interrupt()
{
  if (!interrupt_flag) {
    ::std::cerr << "interrupt: " << *this << ::std::endl;
    interrupt_flag = true;
  }
  if (terminate_on_interrupt) exit(0);
}

void Statistics::out_of_memory()
{
  if (!out_of_memory_flag) {
    ::std::cerr << "memory limit exceeded: " << *this << ::std::endl;
    out_of_memory_flag = true;
  }
  if (terminate_on_out_of_memory) exit(0);
}

void Statistics::out_of_stack()
{
  if (!out_of_stack_flag) {
    ::std::cerr << "stack limit exceeded: " << *this << ::std::endl;
    out_of_stack_flag = true;
  }
  if (terminate_on_out_of_stack) exit(0);
}

void Statistics::add(Statistics& s)
{
  Stopwatch::add(s);
  add_nodes(s);
  total_iterations_started += s.iterations_started;
  total_iterations_finished += s.iterations_finished;
}

void Statistics::add_total(Statistics& s)
{
  Stopwatch::add_total(s);
  add_nodes_total(s);
  total_iterations_started += s.total_iterations_started;
  total_iterations_finished += s.total_iterations_finished;
}

void Statistics::add_nodes(Statistics& s)
{
  total_min_nodes_created += s.min_nodes_created;
  total_max_nodes_created += s.max_nodes_created;
  total_min_nodes_expanded += s.min_nodes_expanded;
  total_max_nodes_expanded += s.max_nodes_expanded;
}

void Statistics::add_nodes_total(Statistics& s)
{
  total_min_nodes_created += s.total_min_nodes_created;
  total_max_nodes_created += s.total_max_nodes_created;
  total_min_nodes_expanded += s.total_min_nodes_expanded;
  total_max_nodes_expanded += s.total_max_nodes_expanded;
}

void Statistics::print_brief(::std::ostream& s, const char* p)
{
  if (p) s << p;
  s << time() << " sec, "
    << min_nodes_created << "/" << max_nodes_created << " n.c., "
    << min_nodes_expanded << "/" << max_nodes_expanded << " n.x., "
    << nodes()/time() << " nodes/sec." << ::std::endl;
}

void Statistics::print(::std::ostream& s, const char* p)
{
  if (p) s << p;
  s << "time: " << time() << " seconds" << ::std::endl;
  if (p) s << p;
  s << "nodes created: " << min_nodes_created << " min / "
    << max_nodes_created << " max" << ::std::endl;
  if (p) s << p;
  s << "nodes expanded: " << min_nodes_expanded << " min / "
    << max_nodes_expanded << " max ("
    << nodes()/time() << " nodes/sec.)" << ::std::endl;
}

void Statistics::print_total(::std::ostream& s, const char* p)
{
  if (p) s << p;
  s << "total time: " << total_time() << " seconds" << ::std::endl;
  if (running > 0) {
    if (p) s << p;
    s << "total nodes created: "
      << total_min_nodes_created + min_nodes_created << " min / "
      << total_max_nodes_created + max_nodes_created << " max" << ::std::endl;
    if (p) s << p;
    s << "total nodes expanded: "
      << total_min_nodes_expanded + min_nodes_expanded << " min / "
      << total_max_nodes_expanded + max_nodes_expanded << " max ("
      << total_nodes()/total_time() << " nodes/sec.)" << ::std::endl;
  }
  else {
    if (p) s << p;
    s << "total nodes created: " << total_min_nodes_created << " min / "
      << total_max_nodes_created << " max" << ::std::endl;
    if (p) s << p;
    s << "total nodes expanded: " << total_min_nodes_expanded << " min / "
      << total_max_nodes_expanded << " max ("
      << total_nodes()/total_time() << " nodes/sec.)" << ::std::endl;
  }
}

END_HSPS_NAMESPACE
