/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse ops_pddlparse
#define yylex   ops_pddllex
#define yyerror ops_pddlerror
#define yylval  ops_pddllval
#define yychar  ops_pddlchar
#define yydebug ops_pddldebug
#define yynerrs ops_pddlnerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DEFINE_TOK = 258,
     DOMAIN_TOK = 259,
     REQUIREMENTS_TOK = 260,
     TYPES_TOK = 261,
     EITHER_TOK = 262,
     CONSTANTS_TOK = 263,
     PREDICATES_TOK = 264,
     FUNCTIONS_TOK = 265,
     ACTION_TOK = 266,
     VARS_TOK = 267,
     IMPLIES_TOK = 268,
     PRECONDITION_TOK = 269,
     PARAMETERS_TOK = 270,
     EFFECT_TOK = 271,
     AND_TOK = 272,
     NOT_TOK = 273,
     WHEN_TOK = 274,
     FORALL_TOK = 275,
     IMPLY_TOK = 276,
     OR_TOK = 277,
     EXISTS_TOK = 278,
     LE_TOK = 279,
     LEQ_TOK = 280,
     EQ_TOK = 281,
     GEQ_TOK = 282,
     GE_TOK = 283,
     MINUS_TOK = 284,
     AD_TOK = 285,
     MU_TOK = 286,
     DI_TOK = 287,
     ASSIGN_TOK = 288,
     SCALE_UP_TOK = 289,
     SCALE_DOWN_TOK = 290,
     INCREASE_TOK = 291,
     DECREASE_TOK = 292,
     NAME = 293,
     VARIABLE = 294,
     NUM = 295,
     OPEN_PAREN = 296,
     CLOSE_PAREN = 297,
     DURATIVE_ACTION_TOK = 298,
     CONDITION_TOK = 299,
     DURATION_TOK = 300,
     AT_TOK = 301,
     START_TOK = 302,
     END_TOK = 303,
     OVER_TOK = 304,
     ALL_TOK = 305,
     temp_TOK = 306,
     PREFERENCE_TOK = 307,
     CONSTRAINTS_TOK = 308,
     ALWAYS_TOK = 309,
     SOMETIME_TOK = 310,
     WITHIN_TOK = 311,
     AT_MOST_ONCE_TOK = 312,
     SOMETIME_AFTER_TOK = 313,
     SOMETIME_BEFORE_TOK = 314,
     ALWAYS_WITHIN_TOK = 315,
     HOLD_DURING_TOK = 316,
     HOLD_AFTER_TOK = 317
   };
#endif
/* Tokens.  */
#define DEFINE_TOK 258
#define DOMAIN_TOK 259
#define REQUIREMENTS_TOK 260
#define TYPES_TOK 261
#define EITHER_TOK 262
#define CONSTANTS_TOK 263
#define PREDICATES_TOK 264
#define FUNCTIONS_TOK 265
#define ACTION_TOK 266
#define VARS_TOK 267
#define IMPLIES_TOK 268
#define PRECONDITION_TOK 269
#define PARAMETERS_TOK 270
#define EFFECT_TOK 271
#define AND_TOK 272
#define NOT_TOK 273
#define WHEN_TOK 274
#define FORALL_TOK 275
#define IMPLY_TOK 276
#define OR_TOK 277
#define EXISTS_TOK 278
#define LE_TOK 279
#define LEQ_TOK 280
#define EQ_TOK 281
#define GEQ_TOK 282
#define GE_TOK 283
#define MINUS_TOK 284
#define AD_TOK 285
#define MU_TOK 286
#define DI_TOK 287
#define ASSIGN_TOK 288
#define SCALE_UP_TOK 289
#define SCALE_DOWN_TOK 290
#define INCREASE_TOK 291
#define DECREASE_TOK 292
#define NAME 293
#define VARIABLE 294
#define NUM 295
#define OPEN_PAREN 296
#define CLOSE_PAREN 297
#define DURATIVE_ACTION_TOK 298
#define CONDITION_TOK 299
#define DURATION_TOK 300
#define AT_TOK 301
#define START_TOK 302
#define END_TOK 303
#define OVER_TOK 304
#define ALL_TOK 305
#define temp_TOK 306
#define PREFERENCE_TOK 307
#define CONSTRAINTS_TOK 308
#define ALWAYS_TOK 309
#define SOMETIME_TOK 310
#define WITHIN_TOK 311
#define AT_MOST_ONCE_TOK 312
#define SOMETIME_AFTER_TOK 313
#define SOMETIME_BEFORE_TOK 314
#define ALWAYS_WITHIN_TOK 315
#define HOLD_DURING_TOK 316
#define HOLD_AFTER_TOK 317




/* Copy the first part of user declarations.  */
#line 24 "scan-ops_pddl.y"

#ifdef YYDEBUG
  extern int yydebug=1;
#endif


#include <stdio.h>
#include <string.h> 
#include "ff.h"
#include "memory.h"
#include "parse.h"


#ifndef SCAN_ERR
#define SCAN_ERR
#define DOMDEF_EXPECTED            0
#define DOMAIN_EXPECTED            1
#define DOMNAME_EXPECTED           2
#define LBRACKET_EXPECTED          3
#define RBRACKET_EXPECTED          4
#define DOMDEFS_EXPECTED           5
#define REQUIREM_EXPECTED          6
#define TYPEDLIST_EXPECTED         7
#define LITERAL_EXPECTED           8
#define PRECONDDEF_UNCORRECT       9
#define TYPEDEF_EXPECTED          10
#define CONSTLIST_EXPECTED        11
#define PREDDEF_EXPECTED          12 
#define NAME_EXPECTED             13
#define VARIABLE_EXPECTED         14
#define ACTIONFUNCTOR_EXPECTED    15
#define ATOM_FORMULA_EXPECTED     16
#define EFFECT_DEF_EXPECTED       17
#define NEG_FORMULA_EXPECTED      18
#define NOT_SUPPORTED             19
#define ACTION                    20
#define CONSTRAINTSDEF            21
#endif


#define NAME_STR "name\0"
#define VARIABLE_STR "variable\0"
#define STANDARD_TYPE "OBJECT\0"
 

static char *serrmsg[] = {
  "domain definition expected",
  "'domain' expected",
  "domain name expected",
  "'(' expected",
  "')' expected",
  "additional domain definitions expected",
  "requirements (e.g. ':STRIPS') expected",
  "typed list of <%s> expected",
  "literal expected",
  "uncorrect precondition definition",
  "type definition expected",
  "list of constants expected",
  "predicate definition expected",
  "<name> expected",
  "<variable> expected",
  "action functor expected",
  "atomic formula expected",
  "effect definition expected",
  "negated atomic formula expected",
  "requirement %s not supported by this FF version",  
  "action definition is not correct",
  NULL
};


void opserr( int errno_, char *par ); 


static int sact_err;
static char *sact_err_par = NULL;
static PlOperator *scur_op = NULL;
static Bool sis_negated = FALSE;
static Bool NotAtom=FALSE;
static PlOperator *durative_op=NULL;


int supported( char *str )

{

  int i;
  char * sup[] = { ":STRIPS", ":NEGATION", ":EQUALITY",":TYPING", 
		   ":CONDITIONAL-EFFECTS", ":NEGATIVE-PRECONDITIONS", ":DISJUNCTIVE-PRECONDITIONS", 
		   ":EXISTENTIAL-PRECONDITIONS", ":UNIVERSAL-PRECONDITIONS", 
		   ":QUANTIFIED-PRECONDITIONS", ":ADL", ":FLUENTS", ":DURATIVE-ACTIONS",":PREFERENCES",":CONSTRAINT",":TIMED-INITIAL-LITERALS",
		   NULL };       

  for (i=0; NULL != sup[i]; i++) {
    if ( SAME == strcmp(sup[i], str) ) {
      return TRUE;
    }
  }
  
  return FALSE;

}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 133 "scan-ops_pddl.y"
{

  char string[MAX_LENGTH];
  char *pstring;
  ParseExpNode *pParseExpNode;
  PlNode *pPlNode;
  FactList *pFactList;
  TokenList *pTokenList;
  TypedList *pTypedList;

}
/* Line 193 of yacc.c.  */
#line 345 "scan-ops_pddl.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 358 "scan-ops_pddl.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   427

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  63
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  59
/* YYNRULES -- Number of rules.  */
#define YYNRULES  152
/* YYNRULES -- Number of states.  */
#define YYNSTATES  411

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   317

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     8,    14,    19,    21,    24,
      27,    30,    33,    36,    39,    42,    45,    46,    47,    56,
      57,    62,    63,    69,    70,    75,    76,    81,    82,    87,
      93,    94,   100,   101,   102,   109,   110,   117,   118,   124,
     125,   131,   132,   133,   140,   141,   142,   150,   151,   152,
     156,   157,   163,   164,   170,   171,   172,   181,   182,   187,
     188,   194,   195,   200,   201,   206,   207,   210,   216,   221,
     226,   231,   237,   243,   249,   256,   263,   269,   275,   281,
     287,   293,   299,   301,   307,   312,   317,   322,   328,   334,
     342,   350,   356,   362,   368,   374,   380,   386,   392,   398,
     400,   406,   411,   416,   421,   427,   435,   443,   444,   447,
     453,   459,   465,   471,   477,   483,   489,   495,   497,   502,
     510,   516,   517,   520,   525,   527,   529,   534,   540,   546,
     552,   558,   564,   570,   575,   577,   582,   587,   588,   591,
     593,   595,   597,   600,   602,   604,   605,   611,   616,   619,
     620,   626,   631
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      64,     0,    -1,    -1,    65,    66,    -1,    -1,    41,     3,
      68,    67,    69,    -1,    41,     4,    38,    42,    -1,    42,
      -1,    90,    69,    -1,    97,    69,    -1,    95,    69,    -1,
      84,    69,    -1,    79,    69,    -1,    86,    69,    -1,    99,
      69,    -1,    70,    69,    -1,    -1,    -1,    41,    43,    71,
      38,    72,    73,    74,    42,    -1,    -1,    15,    41,   121,
      42,    -1,    -1,    12,    41,   121,    42,   103,    -1,    -1,
      16,   110,    75,    74,    -1,    -1,    44,   108,    76,    74,
      -1,    -1,    45,    78,    77,    74,    -1,    41,    26,    51,
     113,    42,    -1,    -1,    41,     9,    81,    80,    42,    -1,
      -1,    -1,    41,    38,   121,    42,    82,    81,    -1,    -1,
      41,    46,   121,    42,    83,    81,    -1,    -1,    41,    53,
      85,   107,    42,    -1,    -1,    41,    10,    88,    87,    42,
      -1,    -1,    -1,    41,    38,   121,    42,    89,    88,    -1,
      -1,    -1,    41,     5,    91,    38,    92,    93,    42,    -1,
      -1,    -1,    38,    94,    93,    -1,    -1,    41,     6,    96,
     120,    42,    -1,    -1,    41,     8,    98,   120,    42,    -1,
      -1,    -1,    41,    11,   100,    38,   101,   102,   103,    42,
      -1,    -1,    15,    41,   121,    42,    -1,    -1,    12,    41,
     121,    42,   103,    -1,    -1,    14,   108,   104,   103,    -1,
      -1,    16,   110,   105,   103,    -1,    -1,   107,   106,    -1,
      41,    46,    48,   107,    42,    -1,    41,    54,   107,    42,
      -1,    41,    55,   107,    42,    -1,    41,    57,   107,    42,
      -1,    41,    56,   113,   107,    42,    -1,    41,    58,   107,
     107,    42,    -1,    41,    59,   107,   107,    42,    -1,    41,
      60,   113,   107,   107,    42,    -1,    41,    61,   113,   113,
     107,    42,    -1,    41,    62,   113,   107,    42,    -1,    41,
      24,   113,   113,    42,    -1,    41,    25,   113,   113,    42,
      -1,    41,    26,   113,   113,    42,    -1,    41,    27,   113,
     113,    42,    -1,    41,    28,   113,   113,    42,    -1,   114,
      -1,    41,    52,   117,   107,    42,    -1,    41,    17,   106,
      42,    -1,    41,    22,   106,    42,    -1,    41,    18,   107,
      42,    -1,    41,    21,   107,   107,    42,    -1,    41,    13,
     107,   107,    42,    -1,    41,    23,    41,   121,    42,   107,
      42,    -1,    41,    20,    41,   121,    42,   107,    42,    -1,
      41,    46,    47,   108,    42,    -1,    41,    46,    48,   108,
      42,    -1,    41,    49,    50,   108,    42,    -1,    41,    24,
     113,   113,    42,    -1,    41,    25,   113,   113,    42,    -1,
      41,    26,   113,   113,    42,    -1,    41,    27,   113,   113,
      42,    -1,    41,    28,   113,   113,    42,    -1,   114,    -1,
      41,    52,   117,   108,    42,    -1,    41,    17,   109,    42,
      -1,    41,    22,   109,    42,    -1,    41,    18,   108,    42,
      -1,    41,    21,   108,   108,    42,    -1,    41,    23,    41,
     121,    42,   108,    42,    -1,    41,    20,    41,   121,    42,
     108,    42,    -1,    -1,   108,   109,    -1,    41,    46,    47,
     110,    42,    -1,    41,    46,    48,   110,    42,    -1,    41,
      49,    50,   110,    42,    -1,    41,    33,   112,   113,    42,
      -1,    41,    34,   112,   113,    42,    -1,    41,    35,   112,
     113,    42,    -1,    41,    36,   112,   113,    42,    -1,    41,
      37,   112,   113,    42,    -1,   114,    -1,    41,    17,   111,
      42,    -1,    41,    20,    41,   121,    42,   110,    42,    -1,
      41,    19,   108,   110,    42,    -1,    -1,   110,   111,    -1,
      41,    38,   116,    42,    -1,    40,    -1,   112,    -1,    41,
      29,   113,    42,    -1,    41,    30,   113,   113,    42,    -1,
      41,    29,   113,   113,    42,    -1,    41,    31,   113,   113,
      42,    -1,    41,    32,   113,   113,    42,    -1,    41,    32,
      51,   113,    42,    -1,    41,    31,    51,   113,    42,    -1,
      41,    18,   115,    42,    -1,   115,    -1,    41,   119,   116,
      42,    -1,    41,    26,   116,    42,    -1,    -1,   117,   116,
      -1,    38,    -1,    39,    -1,    38,    -1,    38,   118,    -1,
      38,    -1,    46,    -1,    -1,    38,     7,   118,    42,   120,
      -1,    38,    29,    38,   120,    -1,    38,   120,    -1,    -1,
      39,     7,   118,    42,   121,    -1,    39,    29,    38,   121,
      -1,    39,   121,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   235,   235,   235,   246,   245,   259,   269,   271,   273,
     275,   277,   279,   281,   283,   285,   290,   295,   289,   309,
     313,   326,   329,   348,   347,   354,   353,   360,   359,   368,
     376,   375,   385,   388,   387,   413,   412,   443,   442,   457,
     456,   467,   470,   469,   499,   503,   498,   514,   518,   517,
     531,   530,   544,   543,   559,   563,   558,   577,   581,   595,
     598,   617,   616,   623,   622,   633,   637,   650,   659,   668,
     677,   687,   699,   709,   719,   731,   743,   754,   762,   770,
     778,   786,   794,   807,   816,   822,   828,   834,   844,   855,
     870,   892,   910,   927,   944,   952,   960,   968,   976,   984,
     998,  1006,  1012,  1018,  1024,  1034,  1049,  1070,  1074,  1087,
    1104,  1122,  1138,  1146,  1154,  1162,  1170,  1178,  1192,  1198,
    1213,  1233,  1237,  1249,  1262,  1270,  1275,  1281,  1288,  1295,
    1302,  1309,  1316,  1327,  1333,  1342,  1349,  1362,  1364,  1375,
    1381,  1391,  1398,  1410,  1416,  1427,  1429,  1439,  1450,  1470,
    1472,  1481,  1492
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DEFINE_TOK", "DOMAIN_TOK",
  "REQUIREMENTS_TOK", "TYPES_TOK", "EITHER_TOK", "CONSTANTS_TOK",
  "PREDICATES_TOK", "FUNCTIONS_TOK", "ACTION_TOK", "VARS_TOK",
  "IMPLIES_TOK", "PRECONDITION_TOK", "PARAMETERS_TOK", "EFFECT_TOK",
  "AND_TOK", "NOT_TOK", "WHEN_TOK", "FORALL_TOK", "IMPLY_TOK", "OR_TOK",
  "EXISTS_TOK", "LE_TOK", "LEQ_TOK", "EQ_TOK", "GEQ_TOK", "GE_TOK",
  "MINUS_TOK", "AD_TOK", "MU_TOK", "DI_TOK", "ASSIGN_TOK", "SCALE_UP_TOK",
  "SCALE_DOWN_TOK", "INCREASE_TOK", "DECREASE_TOK", "NAME", "VARIABLE",
  "NUM", "OPEN_PAREN", "CLOSE_PAREN", "DURATIVE_ACTION_TOK",
  "CONDITION_TOK", "DURATION_TOK", "AT_TOK", "START_TOK", "END_TOK",
  "OVER_TOK", "ALL_TOK", "temp_TOK", "PREFERENCE_TOK", "CONSTRAINTS_TOK",
  "ALWAYS_TOK", "SOMETIME_TOK", "WITHIN_TOK", "AT_MOST_ONCE_TOK",
  "SOMETIME_AFTER_TOK", "SOMETIME_BEFORE_TOK", "ALWAYS_WITHIN_TOK",
  "HOLD_DURING_TOK", "HOLD_AFTER_TOK", "$accept", "file", "@1",
  "domain_definition", "@2", "domain_name", "optional_domain_defs",
  "durative_action_def", "@3", "@4", "du_param_def", "du_def_body", "@5",
  "@6", "@7", "du_atom_for", "predicates_def", "@8", "predicates_list",
  "@9", "@10", "constraints_def", "@11", "functions_def", "@12",
  "functions_list", "@13", "require_def", "@14", "@15", "require_key_star",
  "@16", "types_def", "@17", "constants_def", "@18", "action_def", "@19",
  "@20", "param_def", "action_def_body", "@21", "@22",
  "adl_domain_constraints_description_star",
  "adl_domain_constraints_description", "adl_goal_description",
  "adl_goal_description_star", "adl_effect", "adl_effect_star", "f_head",
  "f_exp", "literal_term", "atomic_formula_term", "term_star", "term",
  "name_plus", "predicate", "typed_list_name", "typed_list_variable", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    63,    65,    64,    67,    66,    68,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    71,    72,    70,    73,
      73,    74,    74,    75,    74,    76,    74,    77,    74,    78,
      80,    79,    81,    82,    81,    83,    81,    85,    84,    87,
      86,    88,    89,    88,    91,    92,    90,    93,    94,    93,
      96,    95,    98,    97,   100,   101,    99,   102,   102,   103,
     103,   104,   103,   105,   103,   106,   106,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   109,   109,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   111,   111,   112,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   114,   114,   115,   115,   116,   116,   117,
     117,   118,   118,   119,   119,   120,   120,   120,   120,   121,
     121,   121,   121
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     0,     5,     4,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     0,     0,     8,     0,
       4,     0,     5,     0,     4,     0,     4,     0,     4,     5,
       0,     5,     0,     0,     6,     0,     6,     0,     5,     0,
       5,     0,     0,     6,     0,     0,     7,     0,     0,     3,
       0,     5,     0,     5,     0,     0,     8,     0,     4,     0,
       5,     0,     4,     0,     4,     0,     2,     5,     4,     4,
       4,     5,     5,     5,     6,     6,     5,     5,     5,     5,
       5,     5,     1,     5,     4,     4,     4,     5,     5,     7,
       7,     5,     5,     5,     5,     5,     5,     5,     5,     1,
       5,     4,     4,     4,     5,     7,     7,     0,     2,     5,
       5,     5,     5,     5,     5,     5,     5,     1,     4,     7,
       5,     0,     2,     4,     1,     1,     4,     5,     5,     5,
       5,     5,     5,     4,     1,     4,     4,     0,     2,     1,
       1,     1,     2,     1,     1,     0,     5,     4,     2,     0,
       5,     4,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     0,     1,     0,     3,     0,     0,     4,     0,
       0,     0,     0,     7,     5,     0,     0,     0,     0,     0,
       0,     0,     0,     6,    44,    50,    52,    32,    41,    54,
      16,    37,    15,    12,    11,    13,     8,    10,     9,    14,
       0,   145,   145,     0,    30,     0,    39,     0,     0,     0,
      45,   145,     0,     0,   149,   149,     0,   149,     0,    55,
      17,     0,     0,    82,   134,    47,     0,     0,   148,    51,
      53,   149,     0,     0,    31,     0,    40,    57,    19,     0,
      65,     0,     0,     0,    65,     0,     0,     0,   137,     0,
       0,   143,   144,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   137,    38,    48,     0,   141,     0,   145,
       0,     0,   152,    33,    35,    42,     0,    59,     0,    21,
       0,     0,    65,     0,     0,   149,     0,     0,   149,   124,
       0,   125,     0,     0,   139,   140,     0,     0,   137,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    47,    46,   142,   145,   147,     0,   149,
      32,    32,    41,   149,     0,     0,     0,     0,   149,     0,
       0,     0,     0,     0,     0,    84,    66,    86,   133,     0,
       0,    85,     0,     0,     0,     0,     0,   137,     0,     0,
       0,   136,   138,     0,     0,     0,     0,    68,    69,     0,
      70,     0,     0,     0,     0,     0,   135,    49,   146,   149,
     151,    34,    36,    43,     0,   149,     0,    61,    99,     0,
      63,   117,    56,     0,   149,    23,    25,     0,    27,    18,
      88,     0,    87,     0,     0,     0,     0,     0,     0,     0,
       0,    77,    78,    79,    80,    81,    67,    83,    71,    72,
      73,     0,     0,    76,   150,    58,     0,   107,     0,     0,
       0,   107,     0,     0,     0,   137,     0,     0,   144,     0,
       0,    59,   121,     0,     0,     0,   137,     0,     0,     0,
       0,     0,   144,     0,    59,    20,     0,    21,    21,     0,
      21,     0,     0,   126,     0,     0,     0,     0,     0,     0,
     123,    74,    75,    59,   107,     0,     0,   149,     0,     0,
     149,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      62,   121,     0,     0,     0,     0,   149,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    64,    59,    24,    26,
       0,    28,    90,    89,   128,   127,   132,   129,   131,   130,
      60,   108,   101,   103,     0,     0,   102,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   122,   118,   144,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,   104,     0,    94,    95,    96,    97,    98,
      91,    92,    93,   100,   120,     0,   112,   113,   114,   115,
     116,   109,   110,   111,    29,     0,     0,     0,   106,   105,
     119
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     5,    10,     8,    14,    15,    48,    78,
     119,   173,   287,   288,   290,   228,    16,    56,    44,   160,
     161,    17,    49,    18,    58,    46,   162,    19,    40,    65,
     106,   153,    20,    41,    21,    42,    22,    47,    77,   117,
     167,   271,   284,   121,   122,   304,   305,   321,   322,   131,
     132,    63,    64,   137,   138,   108,   103,    52,    72
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -271
static const yytype_int16 yypact[] =
{
    -271,    17,   -14,  -271,    42,  -271,   -10,    47,  -271,    27,
      -2,    29,    38,  -271,  -271,    -2,    -2,    -2,    -2,    -2,
      -2,    -2,    -2,  -271,  -271,  -271,  -271,    44,    46,  -271,
    -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,
      41,    51,    51,   -17,  -271,    79,  -271,    85,   106,   104,
    -271,    28,   107,   110,   114,   114,   113,   114,   115,  -271,
    -271,   306,   116,  -271,  -271,   108,   118,   121,  -271,  -271,
    -271,    45,   119,   123,  -271,   124,  -271,    68,   148,   104,
     104,   104,   126,   104,   104,   138,    96,    96,    62,    96,
      96,  -271,   144,    20,   104,   104,    96,   104,   104,   104,
      96,    96,    96,    20,  -271,  -271,   151,   118,   154,    51,
     118,   166,  -271,  -271,  -271,  -271,   164,    66,   171,    18,
     104,   168,   104,   172,   174,   114,   104,   175,   114,  -271,
      67,  -271,    96,    96,  -271,  -271,    96,   176,    20,    96,
      96,   104,   104,   177,   178,   104,   179,   104,   104,   104,
      96,   104,   180,   108,  -271,  -271,    51,  -271,   184,   114,
      44,    44,    46,   114,   186,   187,   188,   190,   114,   189,
     188,   187,   192,   193,   195,  -271,  -271,  -271,  -271,   202,
     203,  -271,   204,    96,    96,   -29,    13,    20,   211,   212,
     214,  -271,  -271,   215,   216,   217,   218,  -271,  -271,   219,
    -271,   220,   221,   104,   104,   227,  -271,  -271,  -271,   114,
    -271,  -271,  -271,  -271,   228,   114,   353,  -271,  -271,    92,
    -271,  -271,  -271,   236,   114,  -271,  -271,   208,  -271,  -271,
    -271,   104,  -271,   104,    74,    96,    96,    96,    96,    96,
     237,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,
    -271,   238,   239,  -271,  -271,  -271,   240,   187,   187,   242,
     187,   187,   243,    96,    96,    62,    96,    96,    59,   163,
      20,    66,   188,   245,   187,   246,    20,   249,   249,   249,
     249,   249,    95,   244,    66,  -271,   253,    18,    18,   185,
      18,   254,   255,  -271,   256,   257,   258,   259,   260,   261,
    -271,  -271,  -271,    66,   187,   262,   264,   114,   187,   266,
     114,    96,    96,    96,    96,    96,   187,   187,   187,   187,
    -271,   188,   267,   -18,   174,   188,   114,   272,    96,    96,
      96,    96,    96,   188,   188,   188,  -271,    66,  -271,  -271,
      96,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,
    -271,  -271,  -271,  -271,   269,   270,  -271,   271,   273,   274,
     275,   276,   279,   280,   283,   295,   296,  -271,  -271,  -271,
     298,   304,   305,   307,   308,   309,   311,   312,   313,   314,
    -271,   315,   187,  -271,   187,  -271,  -271,  -271,  -271,  -271,
    -271,  -271,  -271,  -271,  -271,   188,  -271,  -271,  -271,  -271,
    -271,  -271,  -271,  -271,  -271,   317,   327,   330,  -271,  -271,
    -271
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -271,  -271,  -271,  -271,  -271,  -271,   169,  -271,  -271,  -271,
    -271,  -195,  -271,  -271,  -271,  -271,  -271,  -271,   -21,  -271,
    -271,  -271,  -271,  -271,  -271,   222,  -271,  -271,  -271,  -271,
     229,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,
    -270,  -271,  -271,   -80,   194,   -77,  -254,  -157,    64,  -146,
     -64,  -110,   -78,   -97,   -88,   -92,  -271,   -32,   -55
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      73,   320,    75,   124,   127,   142,   152,   309,   276,   220,
      53,   129,   130,   225,   336,   155,   112,     3,   158,    68,
      91,    54,   236,   133,   136,   139,   140,     4,   369,    55,
     169,     7,   145,   350,   170,    66,   149,   150,   151,    12,
      13,   192,   176,    24,    25,     6,    26,    27,    28,    29,
     351,     9,   110,   129,   130,   218,   221,    67,   134,   135,
     221,   218,   171,   172,   238,    11,    51,   380,   188,   189,
     179,    23,   190,   182,   111,   193,   194,   157,   164,    50,
     165,    30,   166,   116,    71,    43,   204,    45,   217,    51,
     240,    31,   338,   339,   226,   341,   183,   184,   185,   186,
     134,   135,   129,   130,   210,   187,   316,   317,   214,   272,
     273,   274,   275,   223,   129,   130,   293,    57,   276,   234,
     235,   237,   239,    59,   208,   277,   278,   279,   280,   281,
      91,   328,   329,   330,   331,   332,   129,   130,   282,   211,
     212,   283,   333,   334,    60,    61,   105,   218,   218,    69,
     218,   218,    70,    71,   254,    74,   107,    76,   104,   109,
     256,   113,   221,   118,   218,   114,   115,   125,   370,   286,
     294,   295,   296,   297,   298,   299,   377,   378,   379,   128,
     124,   306,   319,   308,    32,    33,    34,    35,    36,    37,
      38,    39,   141,   154,   218,   324,   156,   325,   218,   311,
     312,   313,   314,   315,   159,   163,   218,   218,   218,   218,
     175,   221,   168,   318,   177,   221,   178,   181,   191,   197,
     198,   200,   206,   221,   221,   221,   209,   215,   216,   219,
     224,   355,   222,   227,   289,   229,   340,   230,   407,   363,
     364,   365,   366,    62,   231,   232,   233,   358,   359,   360,
     361,   362,   354,   241,   242,   357,   243,   244,   245,   246,
     247,   248,   249,   250,   372,   373,   374,   375,   376,   253,
     255,   371,   218,   120,   218,   123,   381,   126,   285,   300,
     301,   302,   303,   307,   310,   221,   323,   326,   143,   144,
     327,   146,   147,   148,   335,   337,   342,   343,   344,   345,
     346,   347,   348,   349,   352,   405,   353,   406,   356,   368,
     187,   382,   383,   384,   174,   385,   386,   387,   388,    79,
     180,   389,   390,    80,    81,   391,    82,    83,    84,    85,
      86,    87,    88,    89,    90,   195,   196,   392,   393,   199,
     394,   201,   202,   203,    91,   205,   395,   396,     0,   397,
     398,   399,    92,   400,   401,   402,   403,   404,    93,   408,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   409,
     257,   258,   410,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   207,     0,   213,   367,     0,     0,     0,     0,
       0,    91,     0,     0,     0,     0,     0,   251,   252,   268,
       0,     0,   269,     0,     0,   270,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   291,     0,   292
};

static const yytype_int16 yycheck[] =
{
      55,   271,    57,    81,    84,    93,   103,   261,    26,   166,
      42,    40,    41,   170,   284,   107,    71,     0,   110,    51,
      38,    38,    51,    87,    88,    89,    90,    41,    46,    46,
      12,    41,    96,   303,    16,     7,   100,   101,   102,    41,
      42,   138,   122,     5,     6,     3,     8,     9,    10,    11,
     304,     4,     7,    40,    41,   165,   166,    29,    38,    39,
     170,   171,    44,    45,    51,    38,    38,   337,   132,   133,
     125,    42,   136,   128,    29,   139,   140,   109,    12,    38,
      14,    43,    16,    15,    39,    41,   150,    41,   165,    38,
     187,    53,   287,   288,   171,   290,    29,    30,    31,    32,
      38,    39,    40,    41,   159,    38,    47,    48,   163,    17,
      18,    19,    20,   168,    40,    41,    42,    38,    26,   183,
     184,   185,   186,    38,   156,    33,    34,    35,    36,    37,
      38,   277,   278,   279,   280,   281,    40,    41,    46,   160,
     161,    49,    47,    48,    38,    41,    38,   257,   258,    42,
     260,   261,    42,    39,   209,    42,    38,    42,    42,    38,
     215,    42,   272,    15,   274,    42,    42,    41,   325,   224,
     234,   235,   236,   237,   238,   239,   333,   334,   335,    41,
     258,   258,   270,   260,    15,    16,    17,    18,    19,    20,
      21,    22,    48,    42,   304,   273,    42,   274,   308,   263,
     264,   265,   266,   267,    38,    41,   316,   317,   318,   319,
      42,   321,    41,    50,    42,   325,    42,    42,    42,    42,
      42,    42,    42,   333,   334,   335,    42,    41,    41,    41,
      41,   308,    42,    41,    26,    42,    51,    42,   395,   316,
     317,   318,   319,    49,    42,    42,    42,   311,   312,   313,
     314,   315,   307,    42,    42,   310,    42,    42,    42,    42,
      42,    42,    42,    42,   328,   329,   330,   331,   332,    42,
      42,   326,   382,    79,   384,    81,   340,    83,    42,    42,
      42,    42,    42,    41,    41,   395,    41,    41,    94,    95,
      41,    97,    98,    99,    50,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,   382,    42,   384,    42,    42,
      38,    42,    42,    42,   120,    42,    42,    42,    42,    13,
     126,    42,    42,    17,    18,    42,    20,    21,    22,    23,
      24,    25,    26,    27,    28,   141,   142,    42,    42,   145,
      42,   147,   148,   149,    38,   151,    42,    42,    -1,    42,
      42,    42,    46,    42,    42,    42,    42,    42,    52,    42,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    42,
      17,    18,    42,    20,    21,    22,    23,    24,    25,    26,
      27,    28,   153,    -1,   162,   321,    -1,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,   203,   204,    46,
      -1,    -1,    49,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   231,    -1,   233
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    64,    65,     0,    41,    66,     3,    41,    68,     4,
      67,    38,    41,    42,    69,    70,    79,    84,    86,    90,
      95,    97,    99,    42,     5,     6,     8,     9,    10,    11,
      43,    53,    69,    69,    69,    69,    69,    69,    69,    69,
      91,    96,    98,    41,    81,    41,    88,   100,    71,    85,
      38,    38,   120,   120,    38,    46,    80,    38,    87,    38,
      38,    41,   107,   114,   115,    92,     7,    29,   120,    42,
      42,    39,   121,   121,    42,   121,    42,   101,    72,    13,
      17,    18,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    38,    46,    52,    54,    55,    56,    57,    58,    59,
      60,    61,    62,   119,    42,    38,    93,    38,   118,    38,
       7,    29,   121,    42,    42,    42,    15,   102,    15,    73,
     107,   106,   107,   107,   115,    41,   107,   106,    41,    40,
      41,   112,   113,   113,    38,    39,   113,   116,   117,   113,
     113,    48,   117,   107,   107,   113,   107,   107,   107,   113,
     113,   113,   116,    94,    42,   118,    42,   120,   118,    38,
      82,    83,    89,    41,    12,    14,    16,   103,    41,    12,
      16,    44,    45,    74,   107,    42,   106,    42,    42,   121,
     107,    42,   121,    29,    30,    31,    32,    38,   113,   113,
     113,    42,   116,   113,   113,   107,   107,    42,    42,   107,
      42,   107,   107,   107,   113,   107,    42,    93,   120,    42,
     121,    81,    81,    88,   121,    41,    41,   108,   114,    41,
     110,   114,    42,   121,    41,   110,   108,    41,    78,    42,
      42,    42,    42,    42,   113,   113,    51,   113,    51,   113,
     116,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,   107,   107,    42,   121,    42,   121,    17,    18,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    46,    49,
      52,   104,    17,    18,    19,    20,    26,    33,    34,    35,
      36,    37,    46,    49,   105,    42,   121,    75,    76,    26,
      77,   107,   107,    42,   113,   113,   113,   113,   113,   113,
      42,    42,    42,    42,   108,   109,   108,    41,   108,   109,
      41,   113,   113,   113,   113,   113,    47,    48,    50,   117,
     103,   110,   111,    41,   115,   108,    41,    41,   112,   112,
     112,   112,   112,    47,    48,    50,   103,    42,    74,    74,
      51,    74,    42,    42,    42,    42,    42,    42,    42,    42,
     103,   109,    42,    42,   121,   108,    42,   121,   113,   113,
     113,   113,   113,   108,   108,   108,   108,   111,    42,    46,
     110,   121,   113,   113,   113,   113,   113,   110,   110,   110,
     103,   113,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,   108,   108,   110,    42,    42,
      42
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 235 "scan-ops_pddl.y"
    { 
  opserr( DOMDEF_EXPECTED, NULL ); 
;}
    break;

  case 4:
#line 246 "scan-ops_pddl.y"
    { 
;}
    break;

  case 5:
#line 249 "scan-ops_pddl.y"
    {
  if ( gcmd_line.display_info >= 1 ) {
    printf("\ndomain '%s' defined\n", gdomain_name);
  }
;}
    break;

  case 6:
#line 260 "scan-ops_pddl.y"
    { 
  gdomain_name = new_Token( strlen((yyvsp[(3) - (4)].string))+1 );
  strcpy( gdomain_name, (yyvsp[(3) - (4)].string));
;}
    break;

  case 16:
#line 290 "scan-ops_pddl.y"
    {
   isDurative=TRUE;
  /* opserr( ACTION, NULL );*/
;}
    break;

  case 17:
#line 295 "scan-ops_pddl.y"
    {
   
   durative_op=new_PlOperator( (yyvsp[(4) - (4)].string) ); 
;}
    break;

  case 18:
#line 300 "scan-ops_pddl.y"
    {
      durative_op->next = gloaded_ops;
	  gloaded_ops=durative_op;
	  
;}
    break;

  case 19:
#line 309 "scan-ops_pddl.y"
    { 
  durative_op->params = NULL; 
;}
    break;

  case 20:
#line 314 "scan-ops_pddl.y"
    {
  TypedList *tl;
  durative_op->parse_params = (yyvsp[(3) - (4)].pTypedList);
  for (tl = durative_op->parse_params; tl; tl = tl->next) {
    /* to be able to distinguish params from :VARS 
     */
    durative_op->number_of_real_params++;
  }
;}
    break;

  case 22:
#line 330 "scan-ops_pddl.y"
    {
  TypedList *tl = NULL;

  /* add vars as parameters 
   */
  if ( durative_op->parse_params ) {
    for( tl = durative_op->parse_params; tl->next; tl = tl->next ) {
      /* empty, get to the end of list 
       */
    }
    tl->next = (yyvsp[(3) - (5)].pTypedList);
    tl = tl->next;
  } else {
    durative_op->parse_params = (yyvsp[(3) - (5)].pTypedList);
  }
;}
    break;

  case 23:
#line 348 "scan-ops_pddl.y"
    { 
  durative_op->effects = (yyvsp[(2) - (2)].pPlNode); 
;}
    break;

  case 25:
#line 354 "scan-ops_pddl.y"
    { 
  durative_op->preconds = (yyvsp[(2) - (2)].pPlNode); 
;}
    break;

  case 27:
#line 360 "scan-ops_pddl.y"
    {
durative_op->duration = (yyvsp[(2) - (2)].pTokenList);

;}
    break;

  case 29:
#line 369 "scan-ops_pddl.y"
    {
   (yyval.pTokenList)=(yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 30:
#line 376 "scan-ops_pddl.y"
    {
;}
    break;

  case 31:
#line 379 "scan-ops_pddl.y"
    { 
;}
    break;

  case 32:
#line 385 "scan-ops_pddl.y"
    {;}
    break;

  case 33:
#line 388 "scan-ops_pddl.y"
    {

  TypedListList *tll;

  if ( gparse_predicates ) {
    tll = gparse_predicates;
    while ( tll->next ) {
      tll = tll->next;
    }
    tll->next = new_TypedListList();
    tll = tll->next;
  } else {
    tll = new_TypedListList();
    gparse_predicates = tll;
  }

  tll->predicate = new_Token( strlen( (yyvsp[(2) - (4)].string) ) + 1);
  strcpy( tll->predicate, (yyvsp[(2) - (4)].string) );

  tll->args = (yyvsp[(3) - (4)].pTypedList);

;}
    break;

  case 35:
#line 413 "scan-ops_pddl.y"
    {

  TypedListList *tll;

  if ( gparse_predicates ) {
    tll = gparse_predicates;
    while ( tll->next ) {
      tll = tll->next;
    }
    tll->next = new_TypedListList();
    tll = tll->next;
  } else {
    tll = new_TypedListList();
    gparse_predicates = tll;
  }

  tll->predicate = new_Token( strlen( "at" ) + 1);
  strcpy( tll->predicate, "at" );

  tll->args = (yyvsp[(3) - (4)].pTypedList);

;}
    break;

  case 37:
#line 443 "scan-ops_pddl.y"
    { 
  fcterr( CONSTRAINTSDEF, NULL ); 
;}
    break;

  case 38:
#line 447 "scan-ops_pddl.y"
    {
  (yyvsp[(4) - (5)].pPlNode)->next = gorig_domain_constraints_facts;
  gorig_domain_constraints_facts = (yyvsp[(4) - (5)].pPlNode);
  isConstraints = TRUE;
;}
    break;

  case 39:
#line 457 "scan-ops_pddl.y"
    {
 isfluents=TRUE;
;}
    break;

  case 40:
#line 461 "scan-ops_pddl.y"
    { 
;}
    break;

  case 41:
#line 467 "scan-ops_pddl.y"
    {;}
    break;

  case 42:
#line 470 "scan-ops_pddl.y"
    {

  TypedListList *tll;

  if ( gparse_functions ) {
    tll = gparse_functions;
    while ( tll->next ) {
      tll = tll->next;
    }
    tll->next = new_TypedListList();
    tll = tll->next;
  } else {
    tll = new_TypedListList();
    gparse_functions = tll;
  }

  tll->predicate = new_Token( strlen( (yyvsp[(2) - (4)].string) ) + 1);
  strcpy( tll->predicate, (yyvsp[(2) - (4)].string) );

  tll->args = (yyvsp[(3) - (4)].pTypedList);

;}
    break;

  case 44:
#line 499 "scan-ops_pddl.y"
    { 
  opserr( REQUIREM_EXPECTED, NULL ); 
;}
    break;

  case 45:
#line 503 "scan-ops_pddl.y"
    { 
  if ( !supported( (yyvsp[(4) - (4)].string) ) ) {
    opserr( NOT_SUPPORTED, (yyvsp[(4) - (4)].string) );
    yyerror();
  }
;}
    break;

  case 48:
#line 518 "scan-ops_pddl.y"
    { 
  if ( !supported( (yyvsp[(1) - (1)].string) ) ) {
    opserr( NOT_SUPPORTED, (yyvsp[(1) - (1)].string) );
    yyerror();
  }
;}
    break;

  case 50:
#line 531 "scan-ops_pddl.y"
    { 
  opserr( TYPEDEF_EXPECTED, NULL ); 
;}
    break;

  case 51:
#line 535 "scan-ops_pddl.y"
    {
  gparse_types = (yyvsp[(4) - (5)].pTypedList);
;}
    break;

  case 52:
#line 544 "scan-ops_pddl.y"
    { 
  opserr( CONSTLIST_EXPECTED, NULL ); 
;}
    break;

  case 53:
#line 548 "scan-ops_pddl.y"
    {
  gparse_constants = (yyvsp[(4) - (5)].pTypedList);
;}
    break;

  case 54:
#line 559 "scan-ops_pddl.y"
    { 
  opserr( ACTION, NULL ); 
;}
    break;

  case 55:
#line 563 "scan-ops_pddl.y"
    { 
  scur_op = new_PlOperator( (yyvsp[(4) - (4)].string) );
;}
    break;

  case 56:
#line 567 "scan-ops_pddl.y"
    {
  scur_op->next = gloaded_ops;
  gloaded_ops = scur_op; 
;}
    break;

  case 57:
#line 577 "scan-ops_pddl.y"
    { 
  scur_op->params = NULL; 
;}
    break;

  case 58:
#line 582 "scan-ops_pddl.y"
    {
  TypedList *tl;
  scur_op->parse_params = (yyvsp[(3) - (4)].pTypedList);
  for (tl = scur_op->parse_params; tl; tl = tl->next) {
    /* to be able to distinguish params from :VARS 
     */
    scur_op->number_of_real_params++;
  }
;}
    break;

  case 60:
#line 599 "scan-ops_pddl.y"
    {
  TypedList *tl = NULL;

  /* add vars as parameters 
   */
  if ( scur_op->parse_params ) {
    for( tl = scur_op->parse_params; tl->next; tl = tl->next ) {
      /* empty, get to the end of list 
       */
    }
    tl->next = (yyvsp[(3) - (5)].pTypedList);
    tl = tl->next;
  } else {
    scur_op->parse_params = (yyvsp[(3) - (5)].pTypedList);
  }
;}
    break;

  case 61:
#line 617 "scan-ops_pddl.y"
    { 
  scur_op->preconds = (yyvsp[(2) - (2)].pPlNode); 
;}
    break;

  case 63:
#line 623 "scan-ops_pddl.y"
    { 
  scur_op->effects = (yyvsp[(2) - (2)].pPlNode); 
;}
    break;

  case 65:
#line 633 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = NULL;
;}
    break;

  case 66:
#line 638 "scan-ops_pddl.y"
    {
  (yyvsp[(1) - (2)].pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
  (yyval.pPlNode) = (yyvsp[(1) - (2)].pPlNode);
;}
    break;

  case 67:
#line 651 "scan-ops_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=AEND;
    (yyval.pPlNode)->sons = (yyvsp[(4) - (5)].pPlNode);
   
;}
    break;

  case 68:
#line 660 "scan-ops_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=ALWAYS;
    (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
   
;}
    break;

  case 69:
#line 669 "scan-ops_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=SOMETIME;
    (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
   
;}
    break;

  case 70:
#line 678 "scan-ops_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=AT_MOST_ONCE;
    (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
   
;}
    break;

  case 71:
#line 688 "scan-ops_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=WITHIN;
    (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
    
    (yyval.pPlNode)->sons = (yyvsp[(4) - (5)].pPlNode);
    
   
;}
    break;

  case 72:
#line 700 "scan-ops_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=SOMETIME_AFTER;
    (yyvsp[(3) - (5)].pPlNode)->next = (yyvsp[(4) - (5)].pPlNode);
    (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
   
;}
    break;

  case 73:
#line 710 "scan-ops_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=SOMETIME_BEFORE;
    (yyvsp[(3) - (5)].pPlNode)->next = (yyvsp[(4) - (5)].pPlNode);
    (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
   
;}
    break;

  case 74:
#line 720 "scan-ops_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=ALWAYS_WITHIN;
    (yyval.pPlNode)->lh = (yyvsp[(3) - (6)].pParseExpNode);
    
    (yyvsp[(4) - (6)].pPlNode)->next = (yyvsp[(5) - (6)].pPlNode);
    (yyval.pPlNode)->sons = (yyvsp[(4) - (6)].pPlNode);
   
;}
    break;

  case 75:
#line 732 "scan-ops_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=HOLD_DURING;
    (yyval.pPlNode)->lh = (yyvsp[(3) - (6)].pParseExpNode);
    (yyval.pPlNode)->rh = (yyvsp[(4) - (6)].pParseExpNode);
   
    (yyval.pPlNode)->sons = (yyvsp[(5) - (6)].pPlNode);
   
;}
    break;

  case 76:
#line 744 "scan-ops_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=HOLD_AFTER;
    (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
    
    (yyval.pPlNode)->sons = (yyvsp[(4) - (5)].pPlNode);
   
;}
    break;

  case 77:
#line 755 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = LE;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 78:
#line 763 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = LEQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 79:
#line 771 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = EQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 80:
#line 779 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = GEQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 81:
#line 787 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = GE;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 82:
#line 795 "scan-ops_pddl.y"
    { 
  if ( sis_negated ) {
    (yyval.pPlNode) = new_PlNode(NOT);
    (yyval.pPlNode)->sons = new_PlNode(ATOM);
    (yyval.pPlNode)->sons->atom = (yyvsp[(1) - (1)].pTokenList);
    sis_negated = FALSE;
  } else {
    (yyval.pPlNode) = new_PlNode(ATOM);
    (yyval.pPlNode)->atom = (yyvsp[(1) - (1)].pTokenList);
  }
;}
    break;

  case 83:
#line 808 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(pref);
  (yyval.pPlNode)->sons = (yyvsp[(4) - (5)].pPlNode);
  (yyval.pPlNode)->sons->name = new_Token( strlen((yyvsp[(3) - (5)].pstring))+1 );
  strcpy((yyval.pPlNode)->sons->name,(yyvsp[(3) - (5)].pstring));
  isConstraintsPreference = TRUE;
;}
    break;

  case 84:
#line 817 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 85:
#line 823 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 86:
#line 829 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(NOT);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 87:
#line 835 "scan-ops_pddl.y"
    { 
  PlNode *np = new_PlNode(NOT);
  np->sons = (yyvsp[(3) - (5)].pPlNode);
  np->next = (yyvsp[(4) - (5)].pPlNode);

  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = np;
;}
    break;

  case 88:
#line 845 "scan-ops_pddl.y"
    { 
  PlNode *np = new_PlNode(NOT);
  np->sons = (yyvsp[(3) - (5)].pPlNode);
  np->next = (yyvsp[(4) - (5)].pPlNode);

  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = np;
;}
    break;

  case 89:
#line 858 "scan-ops_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(EX);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

     (yyval.pPlNode) = pln;
     pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 90:
#line 873 "scan-ops_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(ALL);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

    (yyval.pPlNode) = pln;
    pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 91:
#line 893 "scan-ops_pddl.y"
    {
 if(NotAtom)
    {
    (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
    (yyvsp[(4) - (5)].pPlNode)->atom_t=1;
    (yyvsp[(4) - (5)].pPlNode)->sons->atom_t=1;
    
    }
    else
    {
    (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
    (yyvsp[(4) - (5)].pPlNode)->atom_t=1;
    }
    NotAtom=FALSE;
    
;}
    break;

  case 92:
#line 911 "scan-ops_pddl.y"
    {
 if(NotAtom)
    {
    (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
    (yyvsp[(4) - (5)].pPlNode)->atom_t=3;
    (yyvsp[(4) - (5)].pPlNode)->sons->atom_t=3;
    
    }
    else
    {
    (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
    (yyvsp[(4) - (5)].pPlNode)->atom_t=3;
    }
    NotAtom=FALSE; 
;}
    break;

  case 93:
#line 928 "scan-ops_pddl.y"
    {
    if(NotAtom)
    {
    (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
    (yyvsp[(4) - (5)].pPlNode)->atom_t=2;
    (yyvsp[(4) - (5)].pPlNode)->sons->atom_t=2;
    }
    else
    {
    (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
    (yyvsp[(4) - (5)].pPlNode)->atom_t=2;
    }
    NotAtom=FALSE;
    
;}
    break;

  case 94:
#line 945 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = LE;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 95:
#line 953 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = LEQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 96:
#line 961 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = EQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 97:
#line 969 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = GEQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 98:
#line 977 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = GE;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 99:
#line 985 "scan-ops_pddl.y"
    { 
  if ( sis_negated ) {
    (yyval.pPlNode) = new_PlNode(NOT);
    (yyval.pPlNode)->sons = new_PlNode(ATOM);
    (yyval.pPlNode)->sons->atom = (yyvsp[(1) - (1)].pTokenList);
    sis_negated = FALSE;
    NotAtom=TRUE;
  } else {
    (yyval.pPlNode) = new_PlNode(ATOM);
    (yyval.pPlNode)->atom = (yyvsp[(1) - (1)].pTokenList);
  }
;}
    break;

  case 100:
#line 999 "scan-ops_pddl.y"
    { 
   (yyval.pPlNode) = (yyvsp[(4) - (5)].pPlNode);
   (yyval.pPlNode)->name = new_Token( strlen((yyvsp[(3) - (5)].pstring))+1 );
   strcpy((yyval.pPlNode)->name,exchange((yyvsp[(3) - (5)].pstring)));
   isDomainPreference = TRUE;
;}
    break;

  case 101:
#line 1007 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 102:
#line 1013 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 103:
#line 1019 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(NOT);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 104:
#line 1025 "scan-ops_pddl.y"
    { 
  PlNode *np = new_PlNode(NOT);
  np->sons = (yyvsp[(3) - (5)].pPlNode);
  np->next = (yyvsp[(4) - (5)].pPlNode);

  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = np;
;}
    break;

  case 105:
#line 1037 "scan-ops_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(EX);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

  (yyval.pPlNode) = pln;
  pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 106:
#line 1052 "scan-ops_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(ALL);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

  (yyval.pPlNode) = pln;
  pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 107:
#line 1070 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = NULL;
;}
    break;

  case 108:
#line 1075 "scan-ops_pddl.y"
    {
  (yyvsp[(1) - (2)].pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
  (yyval.pPlNode) = (yyvsp[(1) - (2)].pPlNode);
;}
    break;

  case 109:
#line 1088 "scan-ops_pddl.y"
    {
   if(NotAtom)
    {
    (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
    (yyvsp[(4) - (5)].pPlNode)->atom_t=1;
    (yyvsp[(4) - (5)].pPlNode)->sons->atom_t=1;
    
    }
    else
    {
    (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
    (yyvsp[(4) - (5)].pPlNode)->atom_t=1;
    }
    NotAtom=FALSE;
;}
    break;

  case 110:
#line 1105 "scan-ops_pddl.y"
    {
    if(NotAtom)
    {
    (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
    (yyvsp[(4) - (5)].pPlNode)->atom_t=3;
    (yyvsp[(4) - (5)].pPlNode)->sons->atom_t=3;
    
    }
    else
    {
    
    (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
    (yyvsp[(4) - (5)].pPlNode)->atom_t=3;
    }
    NotAtom=FALSE;
;}
    break;

  case 111:
#line 1123 "scan-ops_pddl.y"
    {
    if(NotAtom)
     {
      (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
      (yyvsp[(4) - (5)].pPlNode)->atom_t=2;
      (yyvsp[(4) - (5)].pPlNode)->sons->atom_t=2;
     }
    else
     {
     (yyval.pPlNode)= (yyvsp[(4) - (5)].pPlNode);
     (yyvsp[(4) - (5)].pPlNode)->atom_t=2;
     }
    NotAtom=FALSE;
;}
    break;

  case 112:
#line 1139 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode( NEF );
  (yyval.pPlNode)->neft = ASSIGN;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 113:
#line 1147 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode( NEF );
  (yyval.pPlNode)->neft = SCALE_UP;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 114:
#line 1155 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode( NEF );
  (yyval.pPlNode)->neft = SCALE_DOWN;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 115:
#line 1163 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode( NEF );
  (yyval.pPlNode)->neft = INCREASE;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 116:
#line 1171 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode( NEF );
  (yyval.pPlNode)->neft = DECREASE;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 117:
#line 1179 "scan-ops_pddl.y"
    { 
  if ( sis_negated ) {
    (yyval.pPlNode) = new_PlNode(NOT);
    (yyval.pPlNode)->sons = new_PlNode(ATOM);
    (yyval.pPlNode)->sons->atom = (yyvsp[(1) - (1)].pTokenList);
    sis_negated = FALSE;
    NotAtom=TRUE;
  } else {
    (yyval.pPlNode) = new_PlNode(ATOM);
    (yyval.pPlNode)->atom = (yyvsp[(1) - (1)].pTokenList);
  }
;}
    break;

  case 118:
#line 1193 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 119:
#line 1201 "scan-ops_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(ALL);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

  (yyval.pPlNode) = pln;
  pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 120:
#line 1214 "scan-ops_pddl.y"
    {
  /* This will be conditional effects in FF representation, but here
   * a formula like (WHEN p q) will be saved as:
   *  [WHEN]
   *  [sons]
   *   /  \
   * [p]  [q]
   * That means, the first son is p, and the second one is q. 
   */
  (yyval.pPlNode) = new_PlNode(WHEN);
  (yyvsp[(3) - (5)].pPlNode)->next = (yyvsp[(4) - (5)].pPlNode);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
;}
    break;

  case 121:
#line 1233 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = NULL; 
;}
    break;

  case 122:
#line 1238 "scan-ops_pddl.y"
    {
  (yyvsp[(1) - (2)].pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
  (yyval.pPlNode) = (yyvsp[(1) - (2)].pPlNode);
;}
    break;

  case 123:
#line 1250 "scan-ops_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( FHEAD );
  (yyval.pParseExpNode)->atom = new_TokenList();
  (yyval.pParseExpNode)->atom->item = new_Token( strlen((yyvsp[(2) - (4)].string))+1 );
  strcpy( (yyval.pParseExpNode)->atom->item, (yyvsp[(2) - (4)].string) );
  (yyval.pParseExpNode)->atom->next = (yyvsp[(3) - (4)].pTokenList);
;}
    break;

  case 124:
#line 1263 "scan-ops_pddl.y"
    { 
  (yyval.pParseExpNode) = new_ParseExpNode( NUMBER );
  (yyval.pParseExpNode)->atom = new_TokenList();
  (yyval.pParseExpNode)->atom->item = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pParseExpNode)->atom->item, (yyvsp[(1) - (1)].string) );
;}
    break;

  case 125:
#line 1271 "scan-ops_pddl.y"
    {
  (yyval.pParseExpNode) = (yyvsp[(1) - (1)].pParseExpNode);
;}
    break;

  case 126:
#line 1276 "scan-ops_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( MINUS );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (4)].pParseExpNode);
;}
    break;

  case 127:
#line 1282 "scan-ops_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( AD );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 128:
#line 1289 "scan-ops_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( SU );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 129:
#line 1296 "scan-ops_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( MU );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 130:
#line 1303 "scan-ops_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( DI );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 131:
#line 1310 "scan-ops_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( DI );
  (yyval.pParseExpNode)->leftson = durative_op->duration;
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 132:
#line 1317 "scan-ops_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( MU );
  (yyval.pParseExpNode)->leftson = durative_op->duration;
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 133:
#line 1328 "scan-ops_pddl.y"
    { 
  (yyval.pTokenList) = (yyvsp[(3) - (4)].pTokenList);
  sis_negated = TRUE;
;}
    break;

  case 134:
#line 1334 "scan-ops_pddl.y"
    {
  (yyval.pTokenList) = (yyvsp[(1) - (1)].pTokenList);
;}
    break;

  case 135:
#line 1343 "scan-ops_pddl.y"
    { 
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = (yyvsp[(2) - (4)].pstring);
  (yyval.pTokenList)->next = (yyvsp[(3) - (4)].pTokenList);
;}
    break;

  case 136:
#line 1350 "scan-ops_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = new_Token( 5 );
  (yyval.pTokenList)->item = "=";
  (yyval.pTokenList)->next = (yyvsp[(3) - (4)].pTokenList);
;}
    break;

  case 137:
#line 1362 "scan-ops_pddl.y"
    { (yyval.pTokenList) = NULL; ;}
    break;

  case 138:
#line 1365 "scan-ops_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = (yyvsp[(1) - (2)].pstring);
  (yyval.pTokenList)->next = (yyvsp[(2) - (2)].pTokenList);
;}
    break;

  case 139:
#line 1376 "scan-ops_pddl.y"
    { 
  (yyval.pstring) = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pstring), (yyvsp[(1) - (1)].string) );
;}
    break;

  case 140:
#line 1382 "scan-ops_pddl.y"
    { 
  (yyval.pstring) = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pstring), (yyvsp[(1) - (1)].string) );
;}
    break;

  case 141:
#line 1392 "scan-ops_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pTokenList)->item, (yyvsp[(1) - (1)].string) );
;}
    break;

  case 142:
#line 1399 "scan-ops_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = new_Token( strlen((yyvsp[(1) - (2)].string))+1 );
  strcpy( (yyval.pTokenList)->item, (yyvsp[(1) - (2)].string) );
  (yyval.pTokenList)->next = (yyvsp[(2) - (2)].pTokenList);
;}
    break;

  case 143:
#line 1411 "scan-ops_pddl.y"
    { 
  (yyval.pstring) = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pstring), (yyvsp[(1) - (1)].string) );
;}
    break;

  case 144:
#line 1417 "scan-ops_pddl.y"
    { 
  (yyval.pstring) = new_Token( strlen("at")+1 );
  strcpy( (yyval.pstring), "at" );
;}
    break;

  case 145:
#line 1427 "scan-ops_pddl.y"
    { (yyval.pTypedList) = NULL; ;}
    break;

  case 146:
#line 1430 "scan-ops_pddl.y"
    { 

  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (5)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (5)].string) );
  (yyval.pTypedList)->type = (yyvsp[(3) - (5)].pTokenList);
  (yyval.pTypedList)->next = (yyvsp[(5) - (5)].pTypedList);
;}
    break;

  case 147:
#line 1440 "scan-ops_pddl.y"
    {
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (4)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (4)].string) );
  (yyval.pTypedList)->type = new_TokenList();
  (yyval.pTypedList)->type->item = new_Token( strlen((yyvsp[(3) - (4)].string))+1 );
  strcpy( (yyval.pTypedList)->type->item, (yyvsp[(3) - (4)].string) );
  (yyval.pTypedList)->next = (yyvsp[(4) - (4)].pTypedList);
;}
    break;

  case 148:
#line 1451 "scan-ops_pddl.y"
    {
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (2)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (2)].string) );
  if ( (yyvsp[(2) - (2)].pTypedList) ) {/* another element (already typed) is following */
    (yyval.pTypedList)->type = copy_TokenList( (yyvsp[(2) - (2)].pTypedList)->type );
  } else {/* no further element - it must be an untyped list */
    (yyval.pTypedList)->type = new_TokenList();
    (yyval.pTypedList)->type->item = new_Token( strlen(STANDARD_TYPE)+1 );
    strcpy( (yyval.pTypedList)->type->item, STANDARD_TYPE );
  }
  (yyval.pTypedList)->next = (yyvsp[(2) - (2)].pTypedList);
;}
    break;

  case 149:
#line 1470 "scan-ops_pddl.y"
    { (yyval.pTypedList) = NULL; ;}
    break;

  case 150:
#line 1473 "scan-ops_pddl.y"
    { 
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (5)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (5)].string) );
  (yyval.pTypedList)->type = (yyvsp[(3) - (5)].pTokenList);
  (yyval.pTypedList)->next = (yyvsp[(5) - (5)].pTypedList);
;}
    break;

  case 151:
#line 1482 "scan-ops_pddl.y"
    {
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (4)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (4)].string) );
  (yyval.pTypedList)->type = new_TokenList();
  (yyval.pTypedList)->type->item = new_Token( strlen((yyvsp[(3) - (4)].string))+1 );
  strcpy( (yyval.pTypedList)->type->item, (yyvsp[(3) - (4)].string) );
  (yyval.pTypedList)->next = (yyvsp[(4) - (4)].pTypedList);
;}
    break;

  case 152:
#line 1493 "scan-ops_pddl.y"
    {
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (2)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (2)].string) );
  if ( (yyvsp[(2) - (2)].pTypedList) ) {/* another element (already typed) is following */
    (yyval.pTypedList)->type = copy_TokenList( (yyvsp[(2) - (2)].pTypedList)->type );
  } else {/* no further element - it must be an untyped list */
    (yyval.pTypedList)->type = new_TokenList();
    (yyval.pTypedList)->type->item = new_Token( strlen(STANDARD_TYPE)+1 );
    strcpy( (yyval.pTypedList)->type->item, STANDARD_TYPE );
  }
  (yyval.pTypedList)->next = (yyvsp[(2) - (2)].pTypedList);
;}
    break;


/* Line 1267 of yacc.c.  */
#line 3264 "scan-ops_pddl.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 1510 "scan-ops_pddl.y"

#include "lex.ops_pddl.c"


/**********************************************************************
 * Functions
 **********************************************************************/

/* 
 * call	bison -pops -bscan-ops scan-ops.y
 */


  
void opserr( int errno_, char *par )

{

/*   sact_err = errno_; */

/*   if ( sact_err_par ) { */
/*     free(sact_err_par); */
/*   } */
/*   if ( par ) { */
/*     sact_err_par = new_Token(strlen(par)+1); */
/*     strcpy(sact_err_par, par); */
/*   } else { */
/*     sact_err_par = NULL; */
/*   } */

}


int yyerror( char *msg )

{

  fflush(stdout);
  fprintf(stderr, "\n%s: syntax error in line %d, '%s':\n", 
	  gact_filename, lineno, ops_pddltext);

  if ( NULL != sact_err_par ) {
    fprintf(stderr, "%s %s\n", serrmsg[sact_err], sact_err_par);
  } else {
    fprintf(stderr, "%s\n", serrmsg[sact_err]);
  }

  exit( 1 );

}



void load_ops_file( char *filename )

{

  FILE * fp;/* pointer to input files */
  char tmp[MAX_LENGTH] = "";

  /* open operator file 
   */
  if( ( fp = fopen( filename, "r" ) ) == NULL ) {
    sprintf(tmp, "\nff: can't find operator file: %s\n\n", filename );
    perror(tmp);
    exit( 1 );
  }

  gact_filename = filename;
  lineno = 1; 
  ops_pddlin = fp;

  yyparse();

  fclose( fp );/* and close file again */

}

