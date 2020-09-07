
#ifndef CONFIG_H
#define CONFIG_H

// Select numerical representation:
// Define exactly one of NTYPE_FLOAT and NTYPE_RATIONAL.
// #define NTYPE_FLOAT
#define NTYPE_RATIONAL

// Enable/disable checking for numerical overflow on certain
// rational arithmetic operations (this can be quite expensive).
// #define RATIONAL_ARITHMETIC_CHECK_OVERFLOW

// Enable/disable optimisation of some rational arithmetics (mainly +/-)
// in the special case when both arguments are integers. This can improve
// speed of some calculations (e.g., h^m) very much for problems with only
// unit cost/duration.
// #define RATIONAL_ARITHMETIC_CHECK_INTEGER_SPECIAL_CASE

// Define the "safe" precision (max size of divisor) for rational
// arithmetic. Precision limiting must be done explicitly (by calling
// the rational::round method) and this is currently only done in
// certain PDB constructions. What is "safe" depends on how many
// operations are done between calls to rational::round.
#define SAFE_RATIONAL_PRECISION (LONG_MAX/16)

// Enable/disable additional safety/sanity checks.
// #define CHECK_VECTOR_INDEX
// #define CHECK_TABLE_INDEX
// #define CHECK_HEURISTIC_CONSISTENCY
// #define CHECK_INCREMENTAL_EVAL

// Enable higher trace level in some parts of the code (disabling leads
// to a number of "if (trace_level > X)" statements being removed at
// compile time, which is slightly more efficient than checking them at
// runtime).
// #define ENABLE_HIGHER_TRACE_LEVEL

// Assuming unit cost enables a small optimisation to be made when
// heuristic boosting with strong conflict detection is used, but
// limits the planners to optimize only number of actions or number
// of steps.
// #define ASSUME_UNIT_COST

// Enable/disable some optimisations in temporal regression.
// #define APPLY_D_MIN_TRICK
// #define APPLY_NCW_NOOP_TRICK

// Apply path-max inequality (used in temporal and sequential regression
// planning).
// #define APPLY_PATH_MAX

// EPSILON defines the "so-small-I-don't-care"-value. It's used only for
// certain PDB construction strategies, and changing it probably makes no
// difference ever.
#define EPSILON ((double)0.001)

// Enable recording of multiple paths in the construction of regression PDBs.
// This is required by some pattern selection methods (AddIncRegPDB with
// check_all_paths=true).
#define ENABLE_PDB_MULTI_TRACE

// Enable recording of extra statistics from different parts of the planners:
// heuristic evaluation, AH heuristic, search space and process (page faults
// and swaps).
// #define EVAL_EXTRA_STATS
// #define AH_EXTRA_STATS
// #define SEARCH_EXTRA_STATS
// #define PROC_EXTRA_STATS

// Running print of extra statistics ("one line per sample"-style).
// #define PRINT_EXTRA_STATS

// Trace print arithmetic operations involving rationals.
// #define TRACE_PRINT_RATIONAL_ARITHMETIC

// What the name says...
// #define TRACE_PRINT_LOTS


// Specify how to obtain memory usage.
// Define AT MOST ONE of the following. If none is defined, reported peak
// memory use will always equal zero.

// Use rusage.maxrss (works on cygwin, seems to compile on other systems
// but reports only zero).
// #define RSS_FROM_RUSAGE_MAX

// Use procfs/psinfo (works on (some version of) solaris).
// #define RSS_FROM_PROCFS_PSINFO

// Use procfs/stat (works on (some versions of) linux).
// #define RSS_FROM_PROCFS_STAT

// Use mallinfo.arena (measures heap space only, thus tends to underestimate).
// #define RSS_FROM_MALLINFO

// Unless PERIODIC_RESOURCE_CHECK is defined, periodic resource usage checks
// are only enabled if a memory limit is set. Note that if periodic checks
// are not enabled, peak stack size may be somewhat off.
// #define PERIODIC_RESOURCE_CHECK

// Interval between periodic resource usage checks, in seconds:
#define PERIODIC_CHECK_INTERVAL 2.0

// Perform very frequent stack size checks: this may cause some loss of
// performance, but ensures that the reported peak stack size is (more or
// less) correct.
// #define HI_FREQ_STACK_CHECK

// Specify behaviour of second ctrl-C:
// Normal behaviour is to trap SIGINT and set a flag, causing the program to
// perform a "nice exit" (printing stats and so on) at the next "checkpoint".
// In some situations, however, it may take a very long time to reach the
// next such checkpoint. Defining the directive below causes the program to
// do an immediate exit the second time it receives a SIGINT (note that
// calling exit inside a signal handler can sometime cause the program to
// crash, but that's also a way to make it stop...)
#define EXIT_ON_SECOND_INTERRUPT

// Namespacing

// #define USE_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

#define BEGIN_HSPS_NAMESPACE namespace hsps {
#define END_HSPS_NAMESPACE   }
#define HSPS hsps

#else

#define BEGIN_HSPS_NAMESPACE
#define END_HSPS_NAMESPACE
#define HSPS

#endif

// Compatibility defines

// Some systems it seems do not have the strndup function.
#include <string.h>

// On some systems it is necessary to include assert.h explicitly, on some
// it is not - just make sure we have it.
#ifndef assert
#include <assert.h>
#endif

// This shaves off another 10% or so in my experiments.
#ifdef __GNUC__
#define ALIGN __attribute__((aligned))
#else
#define ALIGN
#endif

#endif
