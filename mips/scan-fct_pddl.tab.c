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
#define yyparse fct_pddlparse
#define yylex   fct_pddllex
#define yyerror fct_pddlerror
#define yylval  fct_pddllval
#define yychar  fct_pddlchar
#define yydebug fct_pddldebug
#define yynerrs fct_pddlnerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DEFINE_TOK = 258,
     PROBLEM_TOK = 259,
     SITUATION_TOK = 260,
     BSITUATION_TOK = 261,
     OBJECTS_TOK = 262,
     BDOMAIN_TOK = 263,
     INIT_TOK = 264,
     GOAL_TOK = 265,
     METRIC_TOK = 266,
     AND_TOK = 267,
     NOT_TOK = 268,
     NAME = 269,
     VARIABLE = 270,
     NUM = 271,
     LE_TOK = 272,
     LEQ_TOK = 273,
     EQ_TOK = 274,
     GEQ_TOK = 275,
     GE_TOK = 276,
     MINUS_TOK = 277,
     AD_TOK = 278,
     MU_TOK = 279,
     DI_TOK = 280,
     FORALL_TOK = 281,
     IMPLY_TOK = 282,
     OR_TOK = 283,
     EXISTS_TOK = 284,
     EITHER_TOK = 285,
     OPEN_PAREN = 286,
     CLOSE_PAREN = 287,
     AT_TOK = 288,
     PREFERENCE_TOK = 289,
     END_TOK = 290,
     ALWAYS_TOK = 291,
     SOMETIME_TOK = 292,
     WITHIN_TOK = 293,
     AT_MOST_ONCE_TOK = 294,
     SOMETIME_AFTER_TOK = 295,
     SOMETIME_BEFORE_TOK = 296,
     ALWAYS_WITHIN_TOK = 297,
     HOLD_DURING_TOK = 298,
     HOLD_AFTER_TOK = 299,
     CONSTRAINTS_TOK = 300,
     IMPLIES_TOK = 301
   };
#endif
/* Tokens.  */
#define DEFINE_TOK 258
#define PROBLEM_TOK 259
#define SITUATION_TOK 260
#define BSITUATION_TOK 261
#define OBJECTS_TOK 262
#define BDOMAIN_TOK 263
#define INIT_TOK 264
#define GOAL_TOK 265
#define METRIC_TOK 266
#define AND_TOK 267
#define NOT_TOK 268
#define NAME 269
#define VARIABLE 270
#define NUM 271
#define LE_TOK 272
#define LEQ_TOK 273
#define EQ_TOK 274
#define GEQ_TOK 275
#define GE_TOK 276
#define MINUS_TOK 277
#define AD_TOK 278
#define MU_TOK 279
#define DI_TOK 280
#define FORALL_TOK 281
#define IMPLY_TOK 282
#define OR_TOK 283
#define EXISTS_TOK 284
#define EITHER_TOK 285
#define OPEN_PAREN 286
#define CLOSE_PAREN 287
#define AT_TOK 288
#define PREFERENCE_TOK 289
#define END_TOK 290
#define ALWAYS_TOK 291
#define SOMETIME_TOK 292
#define WITHIN_TOK 293
#define AT_MOST_ONCE_TOK 294
#define SOMETIME_AFTER_TOK 295
#define SOMETIME_BEFORE_TOK 296
#define ALWAYS_WITHIN_TOK 297
#define HOLD_DURING_TOK 298
#define HOLD_AFTER_TOK 299
#define CONSTRAINTS_TOK 300
#define IMPLIES_TOK 301




/* Copy the first part of user declarations.  */
#line 24 "scan-fct_pddl.y"

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
#define DEFINE_EXPECTED            0
#define PROBLEM_EXPECTED           1
#define PROBNAME_EXPECTED          2
#define LBRACKET_EXPECTED          3
#define RBRACKET_EXPECTED          4
#define DOMDEFS_EXPECTED           5
#define REQUIREM_EXPECTED          6
#define TYPEDLIST_EXPECTED         7
#define DOMEXT_EXPECTED            8
#define DOMEXTNAME_EXPECTED        9
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
#define SITUATION_EXPECTED        20
#define SITNAME_EXPECTED          21
#define BDOMAIN_EXPECTED          22
#define BADDOMAIN                 23
#define INIFACTS                  24
#define GOALDEF                   25
#define ADLGOAL                   26
#define CONSTRAINTSDEF            27
#endif


static char * serrmsg[] = {
  "'define' expected",
  "'problem' expected",
  "problem name expected",
  "'(' expected",
  "')' expected",
  "additional domain definitions expected",
  "requirements (e.g. ':strips') expected",
  "typed list of <%s> expected",
  "domain extension expected",
  "domain to be extented expected",
  "type definition expected",
  "list of constants expected",
  "predicate definition expected",
  "<name> expected",
  "<variable> expected",
  "action functor expected",
  "atomic formula expected",
  "effect definition expected",
  "negated atomic formula expected",
  "requirement %s not supported by this IPP version",  
  "'situation' expected",
  "situation name expected",
  "':domain' expected",
  "this problem needs another domain file",
  "initial facts definition expected",
  "goal definition expected",
  "first order logic expression expected",
  NULL
};


void fcterr( int errno_, char *par ); 


static int sact_err;
static char *sact_err_par = NULL;
static Bool sis_negated = FALSE;



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
#line 115 "scan-fct_pddl.y"
{

  char string[MAX_LENGTH];
  char* pstring;
  ParseExpNode *pParseExpNode;
  PlNode* pPlNode;
  FactList* pFactList;
  TokenList* pTokenList;
  TypedList* pTypedList;

}
/* Line 193 of yacc.c.  */
#line 295 "scan-fct_pddl.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 308 "scan-fct_pddl.tab.c"

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
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   327

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  47
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  113
/* YYNRULES -- Number of states.  */
#define YYNSTATES  322

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   301

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
      45,    46
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     8,    15,    20,    25,    26,
      29,    32,    35,    38,    41,    44,    49,    50,    56,    57,
      63,    64,    70,    76,    82,    88,    94,   100,   106,   108,
     114,   119,   124,   129,   135,   143,   151,   157,   162,   167,
     172,   178,   184,   190,   197,   204,   210,   216,   222,   228,
     234,   240,   242,   248,   253,   258,   263,   269,   275,   283,
     291,   292,   295,   296,   299,   301,   304,   306,   315,   321,
     329,   340,   342,   347,   352,   358,   364,   370,   376,   378,
     383,   392,   397,   403,   409,   415,   421,   427,   430,   433,
     438,   440,   445,   450,   451,   454,   456,   458,   460,   463,
     464,   470,   475,   478,   479,   485,   490,   493,   495,   497,
     502,   504,   509,   510
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      48,     0,    -1,    -1,    49,    48,    -1,    -1,    31,     3,
      50,    51,    53,    32,    -1,    31,     4,    14,    32,    -1,
      31,     8,    14,    32,    -1,    -1,    54,    53,    -1,    55,
      53,    -1,    57,    53,    -1,    59,    53,    -1,    52,    53,
      -1,    61,    53,    -1,    31,     7,    76,    32,    -1,    -1,
      31,     9,    56,    66,    32,    -1,    -1,    31,    10,    58,
      62,    32,    -1,    -1,    31,    45,    60,    63,    32,    -1,
      31,    11,    14,    69,    32,    -1,    31,    17,    68,    68,
      32,    -1,    31,    18,    68,    68,    32,    -1,    31,    19,
      68,    68,    32,    -1,    31,    20,    68,    68,    32,    -1,
      31,    21,    68,    68,    32,    -1,    71,    -1,    31,    34,
      74,    62,    32,    -1,    31,    12,    64,    32,    -1,    31,
      28,    64,    32,    -1,    31,    13,    62,    32,    -1,    31,
      27,    62,    62,    32,    -1,    31,    29,    31,    77,    32,
      62,    32,    -1,    31,    26,    31,    77,    32,    62,    32,
      -1,    31,    33,    35,    63,    32,    -1,    31,    36,    63,
      32,    -1,    31,    37,    63,    32,    -1,    31,    39,    63,
      32,    -1,    31,    38,    68,    63,    32,    -1,    31,    40,
      63,    63,    32,    -1,    31,    41,    63,    63,    32,    -1,
      31,    42,    68,    63,    63,    32,    -1,    31,    43,    68,
      68,    63,    32,    -1,    31,    44,    68,    63,    32,    -1,
      31,    17,    68,    68,    32,    -1,    31,    18,    68,    68,
      32,    -1,    31,    19,    68,    68,    32,    -1,    31,    20,
      68,    68,    32,    -1,    31,    21,    68,    68,    32,    -1,
      71,    -1,    31,    34,    74,    63,    32,    -1,    31,    12,
      65,    32,    -1,    31,    28,    65,    32,    -1,    31,    13,
      63,    32,    -1,    31,    27,    63,    63,    32,    -1,    31,
      46,    63,    63,    32,    -1,    31,    29,    31,    77,    32,
      63,    32,    -1,    31,    26,    31,    77,    32,    63,    32,
      -1,    -1,    62,    64,    -1,    -1,    63,    65,    -1,    67,
      -1,    67,    66,    -1,    79,    -1,    31,    19,    31,    14,
      81,    32,    16,    32,    -1,    31,    19,    14,    16,    32,
      -1,    31,    33,    16,    31,    73,    32,    32,    -1,    31,
      33,    16,    31,    13,    31,    73,    32,    32,    32,    -1,
      16,    -1,    31,    14,    73,    32,    -1,    31,    22,    68,
      32,    -1,    31,    23,    68,    68,    32,    -1,    31,    22,
      68,    68,    32,    -1,    31,    24,    68,    68,    32,    -1,
      31,    25,    68,    68,    32,    -1,    16,    -1,    31,    14,
      81,    32,    -1,    31,    22,    16,    31,    14,    81,    32,
      32,    -1,    31,    22,    69,    32,    -1,    31,    23,    69,
      69,    32,    -1,    31,    23,    70,    69,    32,    -1,    31,
      22,    69,    69,    32,    -1,    31,    24,    69,    69,    32,
      -1,    31,    25,    69,    69,    32,    -1,    69,    69,    -1,
      70,    69,    -1,    31,    13,    72,    32,    -1,    72,    -1,
      31,    78,    73,    32,    -1,    31,    19,    73,    32,    -1,
      -1,    74,    73,    -1,    14,    -1,    15,    -1,    14,    -1,
      14,    75,    -1,    -1,    14,    30,    75,    32,    76,    -1,
      14,    22,    14,    76,    -1,    14,    76,    -1,    -1,    15,
      30,    75,    32,    77,    -1,    15,    22,    14,    77,    -1,
      15,    77,    -1,    14,    -1,    33,    -1,    31,    13,    80,
      32,    -1,    80,    -1,    31,    78,    81,    32,    -1,    -1,
      14,    81,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   199,   199,   202,   209,   208,   224,   234,   245,   248,
     250,   252,   254,   256,   258,   264,   274,   273,   288,   287,
     301,   300,   315,   342,   350,   358,   366,   374,   382,   395,
     404,   410,   416,   422,   432,   447,   472,   481,   490,   499,
     509,   521,   531,   541,   553,   565,   576,   584,   592,   600,
     608,   616,   629,   639,   645,   651,   657,   667,   678,   693,
     714,   718,   728,   732,   742,   747,   757,   762,   779,   795,
     812,   844,   852,   861,   867,   874,   881,   888,   900,   908,
     929,   948,   954,   961,   968,   975,   982,   994,  1001,  1015,
    1021,  1030,  1037,  1050,  1054,  1065,  1071,  1081,  1088,  1101,
    1103,  1112,  1123,  1143,  1145,  1154,  1165,  1186,  1192,  1218,
    1228,  1238,  1251,  1253
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DEFINE_TOK", "PROBLEM_TOK",
  "SITUATION_TOK", "BSITUATION_TOK", "OBJECTS_TOK", "BDOMAIN_TOK",
  "INIT_TOK", "GOAL_TOK", "METRIC_TOK", "AND_TOK", "NOT_TOK", "NAME",
  "VARIABLE", "NUM", "LE_TOK", "LEQ_TOK", "EQ_TOK", "GEQ_TOK", "GE_TOK",
  "MINUS_TOK", "AD_TOK", "MU_TOK", "DI_TOK", "FORALL_TOK", "IMPLY_TOK",
  "OR_TOK", "EXISTS_TOK", "EITHER_TOK", "OPEN_PAREN", "CLOSE_PAREN",
  "AT_TOK", "PREFERENCE_TOK", "END_TOK", "ALWAYS_TOK", "SOMETIME_TOK",
  "WITHIN_TOK", "AT_MOST_ONCE_TOK", "SOMETIME_AFTER_TOK",
  "SOMETIME_BEFORE_TOK", "ALWAYS_WITHIN_TOK", "HOLD_DURING_TOK",
  "HOLD_AFTER_TOK", "CONSTRAINTS_TOK", "IMPLIES_TOK", "$accept", "file",
  "problem_definition", "@1", "problem_name", "base_domain_name",
  "problem_defs", "objects_def", "init_def", "@2", "goal_def", "@3",
  "constraints_def", "@4", "metric_def", "adl_goal_description",
  "adl_constraints_description", "adl_goal_description_star",
  "adl_constraints_description_star", "init_el_plus", "init_el", "f_exp",
  "ground_f_exp", "multiple_ground_f_exp", "literal_term",
  "atomic_formula_term", "term_star", "term", "name_plus",
  "typed_list_name", "typed_list_variable", "predicate", "literal_name",
  "atomic_formula_name", "name_star", 0
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
     295,   296,   297,   298,   299,   300,   301
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    47,    48,    48,    50,    49,    51,    52,    53,    53,
      53,    53,    53,    53,    53,    54,    56,    55,    58,    57,
      60,    59,    61,    62,    62,    62,    62,    62,    62,    62,
      62,    62,    62,    62,    62,    62,    63,    63,    63,    63,
      63,    63,    63,    63,    63,    63,    63,    63,    63,    63,
      63,    63,    63,    63,    63,    63,    63,    63,    63,    63,
      64,    64,    65,    65,    66,    66,    67,    67,    67,    67,
      67,    68,    68,    68,    68,    68,    68,    68,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    70,    70,    71,
      71,    72,    72,    73,    73,    74,    74,    75,    75,    76,
      76,    76,    76,    77,    77,    77,    77,    78,    78,    79,
      79,    80,    81,    81
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     0,     6,     4,     4,     0,     2,
       2,     2,     2,     2,     2,     4,     0,     5,     0,     5,
       0,     5,     5,     5,     5,     5,     5,     5,     1,     5,
       4,     4,     4,     5,     7,     7,     5,     4,     4,     4,
       5,     5,     5,     6,     6,     5,     5,     5,     5,     5,
       5,     1,     5,     4,     4,     4,     5,     5,     7,     7,
       0,     2,     0,     2,     1,     2,     1,     8,     5,     7,
      10,     1,     4,     4,     5,     5,     5,     5,     1,     4,
       8,     4,     5,     5,     5,     5,     5,     2,     2,     4,
       1,     4,     4,     0,     2,     1,     1,     1,     2,     0,
       5,     4,     2,     0,     5,     4,     2,     1,     1,     4,
       1,     4,     0,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     0,     2,     4,     1,     3,     0,     0,     8,
       0,     0,     8,     0,     8,     8,     8,     8,     8,     0,
      99,     0,    16,    18,     0,    20,    13,     5,     9,    10,
      11,    12,    14,     6,    99,     0,     0,     0,     0,     0,
       0,     0,     0,   102,    15,     7,     0,     0,    64,    66,
     110,     0,     0,    28,    90,    78,     0,     0,     0,     0,
      51,    99,    97,     0,     0,   107,     0,   108,   112,    17,
      65,    60,     0,     0,     0,    93,     0,     0,     0,     0,
      60,     0,   108,     0,    93,    19,   112,     0,     0,     0,
       0,    22,    62,     0,     0,     0,    93,     0,     0,     0,
       0,    62,     0,   108,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,   101,    98,    99,     0,
       0,     0,     0,     0,   112,     0,    60,     0,     0,     0,
      71,     0,     0,     0,    95,    96,     0,     0,    93,     0,
       0,   103,     0,     0,   103,     0,     0,     0,    78,     0,
       0,     0,     0,     0,    62,     0,     0,     0,     0,     0,
       0,     0,   103,     0,     0,   103,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   100,   109,
       0,   112,    93,   113,   111,    61,    30,    32,    89,    93,
       0,     0,     0,     0,     0,     0,     0,    92,    94,     0,
       0,   103,     0,     0,    31,     0,     0,    91,    79,     0,
      81,     0,    87,    88,     0,     0,    63,    53,    55,     0,
       0,     0,     0,     0,     0,     0,    54,     0,     0,     0,
      37,    38,     0,    39,     0,     0,     0,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,     0,     0,   106,     0,    33,     0,
      29,   112,    84,    82,    83,    85,    86,    46,    47,    48,
      49,    50,     0,    56,     0,    36,    52,    40,    41,    42,
       0,     0,    45,    57,     0,    93,     0,    72,    73,     0,
       0,     0,     0,   103,     0,     0,     0,     0,     0,     0,
      43,    44,     0,     0,    69,    75,    74,    76,    77,   105,
     103,    35,    34,     0,    59,    58,    67,     0,   104,    80,
       0,    70
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     7,     9,    12,    13,    14,    15,    37,
      16,    38,    17,    40,    18,   126,   154,   127,   155,    47,
      48,   132,    57,   151,    60,    54,   137,   138,    63,    35,
     202,    68,    49,    50,   125
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -130
static const yytype_int16 yypact[] =
{
       6,    35,    48,     6,  -130,  -130,  -130,    20,    50,    42,
      71,    13,    42,    30,    42,    42,    42,    42,    42,    62,
      82,    83,  -130,  -130,    87,  -130,  -130,  -130,  -130,  -130,
    -130,  -130,  -130,  -130,    33,    70,    72,    89,    90,     9,
      91,   100,   110,  -130,  -130,  -130,    31,    93,    89,  -130,
    -130,   228,    96,  -130,  -130,  -130,   117,    97,   246,   106,
    -130,    82,   110,   115,   118,  -130,    18,    77,   134,  -130,
    -130,    90,    90,    68,    68,    61,    68,    68,   119,    90,
      90,   120,  -130,    12,    12,  -130,   134,    75,     9,     9,
       9,  -130,    91,    91,    68,    68,    61,    68,    68,   125,
      91,    91,   126,   123,    12,    91,    91,    68,    91,    91,
      91,    68,    68,    68,    91,  -130,  -130,  -130,    82,    -4,
     127,   144,   147,   131,   134,   133,    90,   135,   136,   137,
    -130,   121,    68,    68,  -130,  -130,    68,   138,    12,    68,
      68,    88,    90,   139,    88,    90,   145,   148,   132,    55,
       9,     9,     9,     9,    91,   150,   151,    68,    68,    68,
      68,    68,    88,    91,   152,    88,    91,    91,   154,   155,
      91,   156,    91,    91,    91,    68,    91,    91,  -130,  -130,
     160,   134,    -1,  -130,  -130,  -130,  -130,  -130,  -130,    12,
      68,    68,    68,    68,   161,   162,   163,  -130,  -130,   166,
     168,    37,   169,   170,  -130,   172,   173,  -130,  -130,   165,
    -130,   174,   175,   176,   177,   180,  -130,  -130,  -130,   181,
     183,   184,   187,   189,   191,   193,  -130,   195,   196,   197,
    -130,  -130,   198,  -130,   199,   201,    91,    91,   211,   212,
    -130,   218,   220,   244,   245,    79,    68,    68,    68,  -130,
    -130,  -130,  -130,  -130,   238,   110,  -130,    90,  -130,    90,
    -130,   134,  -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,
    -130,  -130,    91,  -130,    91,  -130,  -130,  -130,  -130,  -130,
     249,   259,  -130,  -130,   262,    12,   261,  -130,  -130,   263,
     264,   265,   266,    88,   267,   268,   269,   270,   271,   272,
    -130,  -130,   273,   274,  -130,  -130,  -130,  -130,  -130,  -130,
      88,  -130,  -130,   275,  -130,  -130,  -130,   276,  -130,  -130,
     277,  -130
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -130,   291,  -130,  -130,  -130,  -130,   101,  -130,  -130,  -130,
    -130,  -130,  -130,  -130,  -130,   -33,   -40,   -69,   -93,   279,
    -130,    78,   -71,  -130,   -37,   -65,   -82,   -74,   -56,   -30,
    -129,    32,  -130,   102,   -83
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      59,    53,   146,   147,    43,    52,   117,   129,   164,   145,
      65,   143,   242,   134,   135,   205,   149,   150,   152,   153,
      20,    21,    22,    23,    24,    55,   134,   135,   129,    82,
     167,   116,   121,   224,    53,    53,   227,     1,     4,   128,
      56,   183,    53,    53,    64,    65,   142,    34,     5,   122,
      66,     8,   201,   156,    10,    41,   198,   185,    25,   254,
     163,   216,    27,    42,    67,   168,   169,   255,   171,   172,
     173,    55,   256,    11,   177,   134,   135,   130,   211,   212,
     213,   214,   215,    84,   130,    19,    56,   210,   178,    53,
      84,   148,   131,   123,    33,   130,    34,    36,   241,   131,
     243,    39,    44,   201,    45,    53,    56,   244,    53,   203,
     131,   288,   206,    26,    61,    28,    29,    30,    31,    32,
      46,    51,    58,   225,    62,    69,   228,   229,    85,    91,
     232,    86,   234,   235,   236,   189,   238,   239,   115,    87,
      88,    89,    90,   190,   191,   192,   193,   118,   124,   119,
     141,   144,   133,   136,   139,   140,   162,   165,   166,   179,
     180,   181,   182,   209,   309,   184,   120,   186,   187,   188,
     197,   204,   157,   158,   159,   160,   161,   207,   297,   261,
     208,   318,   217,   218,   226,   170,   230,   231,   233,   174,
     175,   176,   240,   249,   250,   251,   280,   281,   252,   294,
     253,   257,   258,   303,   259,   260,   262,   263,   264,   265,
     194,   195,   266,   267,   196,   268,   269,   199,   200,   270,
      53,   271,    53,   272,   295,   273,   296,   274,   275,   276,
     277,   278,   298,   279,   299,   219,   220,   221,   222,   223,
      71,    72,    65,   282,   283,    73,    74,    75,    76,    77,
     284,   285,   293,   237,    78,    79,    80,    81,    92,    93,
      65,    82,    83,    94,    95,    96,    97,    98,   245,   246,
     247,   248,    99,   100,   101,   102,   286,   287,   302,   103,
     104,   300,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   301,   114,   304,     6,   305,   306,   307,   308,   310,
     311,   312,   313,   314,   315,   316,   317,   319,   320,   321,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   289,   290,   291,   292,    70
};

static const yytype_int16 yycheck[] =
{
      40,    38,    84,    86,    34,    38,    62,    72,   101,    83,
      14,    80,    13,    14,    15,   144,    87,    88,    89,    90,
       7,     8,     9,    10,    11,    16,    14,    15,    93,    33,
     104,    61,    14,   162,    71,    72,   165,    31,     3,    72,
      31,   124,    79,    80,    13,    14,    79,    14,     0,    31,
      19,    31,    15,    93,     4,    22,   138,   126,    45,    22,
     100,   154,    32,    30,    33,   105,   106,    30,   108,   109,
     110,    16,   201,    31,   114,    14,    15,    16,   149,   150,
     151,   152,   153,    51,    16,    14,    31,    32,   118,   126,
      58,    16,    31,    16,    32,    16,    14,    14,   181,    31,
     182,    14,    32,    15,    32,   142,    31,   189,   145,   142,
      31,    32,   145,    12,    14,    14,    15,    16,    17,    18,
      31,    31,    31,   163,    14,    32,   166,   167,    32,    32,
     170,    14,   172,   173,   174,    14,   176,   177,    32,    22,
      23,    24,    25,    22,    23,    24,    25,    32,    14,    31,
      31,    31,    74,    75,    76,    77,    31,    31,    35,    32,
      16,    14,    31,    31,   293,    32,    64,    32,    32,    32,
      32,    32,    94,    95,    96,    97,    98,    32,   261,    14,
      32,   310,    32,    32,    32,   107,    32,    32,    32,   111,
     112,   113,    32,    32,    32,    32,   236,   237,    32,   255,
      32,    32,    32,   285,    32,    32,    32,    32,    32,    32,
     132,   133,    32,    32,   136,    32,    32,   139,   140,    32,
     257,    32,   259,    32,   257,    32,   259,    32,    32,    32,
      32,    32,   272,    32,   274,   157,   158,   159,   160,   161,
      12,    13,    14,    32,    32,    17,    18,    19,    20,    21,
      32,    31,    14,   175,    26,    27,    28,    29,    12,    13,
      14,    33,    34,    17,    18,    19,    20,    21,   190,   191,
     192,   193,    26,    27,    28,    29,    32,    32,    16,    33,
      34,    32,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    32,    46,    32,     3,    32,    32,    32,    32,    32,
      32,    32,    32,    32,    32,    32,    32,    32,    32,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   245,   246,   247,   248,    48
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    31,    48,    49,     3,     0,    48,    50,    31,    51,
       4,    31,    52,    53,    54,    55,    57,    59,    61,    14,
       7,     8,     9,    10,    11,    45,    53,    32,    53,    53,
      53,    53,    53,    32,    14,    76,    14,    56,    58,    14,
      60,    22,    30,    76,    32,    32,    31,    66,    67,    79,
      80,    31,    62,    71,    72,    16,    31,    69,    31,    63,
      71,    14,    14,    75,    13,    14,    19,    33,    78,    32,
      66,    12,    13,    17,    18,    19,    20,    21,    26,    27,
      28,    29,    33,    34,    78,    32,    14,    22,    23,    24,
      25,    32,    12,    13,    17,    18,    19,    20,    21,    26,
      27,    28,    29,    33,    34,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    46,    32,    76,    75,    32,    31,
      80,    14,    31,    16,    14,    81,    62,    64,    62,    72,
      16,    31,    68,    68,    14,    15,    68,    73,    74,    68,
      68,    31,    62,    64,    31,    74,    73,    81,    16,    69,
      69,    70,    69,    69,    63,    65,    63,    68,    68,    68,
      68,    68,    31,    63,    65,    31,    35,    74,    63,    63,
      68,    63,    63,    63,    68,    68,    68,    63,    76,    32,
      16,    14,    31,    81,    32,    64,    32,    32,    32,    14,
      22,    23,    24,    25,    68,    68,    68,    32,    73,    68,
      68,    15,    77,    62,    32,    77,    62,    32,    32,    31,
      32,    69,    69,    69,    69,    69,    65,    32,    32,    68,
      68,    68,    68,    68,    77,    63,    32,    77,    63,    63,
      32,    32,    63,    32,    63,    63,    63,    68,    63,    63,
      32,    81,    13,    73,    73,    68,    68,    68,    68,    32,
      32,    32,    32,    32,    22,    30,    77,    32,    32,    32,
      32,    14,    32,    32,    32,    32,    32,    32,    32,    32,
      32,    32,    32,    32,    32,    32,    32,    32,    32,    32,
      63,    63,    32,    32,    32,    31,    32,    32,    32,    68,
      68,    68,    68,    14,    75,    62,    62,    81,    63,    63,
      32,    32,    16,    73,    32,    32,    32,    32,    32,    77,
      32,    32,    32,    32,    32,    32,    32,    32,    77,    32,
      32,    32
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
        case 4:
#line 209 "scan-fct_pddl.y"
    { 
  fcterr( PROBNAME_EXPECTED, NULL ); 
;}
    break;

  case 5:
#line 213 "scan-fct_pddl.y"
    {  
  gproblem_name = (yyvsp[(4) - (6)].pstring);
  if ( gcmd_line.display_info >= 1 ) {
    printf("\nproblem '%s' defined\n", gproblem_name);
  }
;}
    break;

  case 6:
#line 225 "scan-fct_pddl.y"
    { 
  (yyval.pstring) = new_Token( strlen((yyvsp[(3) - (4)].string))+1 );
  strcpy((yyval.pstring), (yyvsp[(3) - (4)].string));
;}
    break;

  case 7:
#line 235 "scan-fct_pddl.y"
    { 
  if ( SAME != strcmp((yyvsp[(3) - (4)].string), gdomain_name) ) {
    fcterr( BADDOMAIN, NULL );
    yyerror();
  }
;}
    break;

  case 15:
#line 265 "scan-fct_pddl.y"
    { 
  gparse_objects = (yyvsp[(3) - (4)].pTypedList);
;}
    break;

  case 16:
#line 274 "scan-fct_pddl.y"
    {
  fcterr( INIFACTS, NULL ); 
;}
    break;

  case 17:
#line 278 "scan-fct_pddl.y"
    {
  gorig_initial_facts = new_PlNode(AND);
  gorig_initial_facts->sons = (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 18:
#line 288 "scan-fct_pddl.y"
    { 
  fcterr( GOALDEF, NULL ); 
;}
    break;

  case 19:
#line 292 "scan-fct_pddl.y"
    {
  (yyvsp[(4) - (5)].pPlNode)->next = gorig_goal_facts;
  gorig_goal_facts = (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 20:
#line 301 "scan-fct_pddl.y"
    { 
  fcterr( CONSTRAINTSDEF, NULL ); 
;}
    break;

  case 21:
#line 305 "scan-fct_pddl.y"
    {
  (yyvsp[(4) - (5)].pPlNode)->next = gorig_constraints_facts;
  gorig_constraints_facts = (yyvsp[(4) - (5)].pPlNode);
  isConstraints = TRUE;
;}
    break;

  case 22:
#line 316 "scan-fct_pddl.y"
    {

  if ( gparse_metric != NULL ) {
    printf("\n\ndouble metric specification!\n\n");
    exit( 1 );
  }

  gparse_optimization = new_Token(strlen((yyvsp[(3) - (5)].string))+1);
  sprintf(gparse_optimization, "%s", (yyvsp[(3) - (5)].string));
  
  gparse_metric = (yyvsp[(4) - (5)].pParseExpNode);

;}
    break;

  case 23:
#line 343 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = LE;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 24:
#line 351 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = LEQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 25:
#line 359 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = EQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 26:
#line 367 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = GEQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 27:
#line 375 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = GE;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 28:
#line 383 "scan-fct_pddl.y"
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

  case 29:
#line 396 "scan-fct_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(pref);
  (yyval.pPlNode)->sons = (yyvsp[(4) - (5)].pPlNode);
  (yyval.pPlNode)->sons->name = new_Token( strlen((yyvsp[(3) - (5)].pstring))+1 );
  strcpy((yyval.pPlNode)->sons->name,exchange((yyvsp[(3) - (5)].pstring)));
  isPreference = TRUE;
;}
    break;

  case 30:
#line 405 "scan-fct_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 31:
#line 411 "scan-fct_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 32:
#line 417 "scan-fct_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(NOT);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 33:
#line 423 "scan-fct_pddl.y"
    { 
  PlNode *np = new_PlNode(NOT);
  np->sons = (yyvsp[(3) - (5)].pPlNode);
  np->next = (yyvsp[(4) - (5)].pPlNode);

  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = np;
;}
    break;

  case 34:
#line 435 "scan-fct_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(EX);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

   (yyval.pPlNode) = pln;
   pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 35:
#line 450 "scan-fct_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(ALL);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);


   (yyval.pPlNode) = pln;
   pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 36:
#line 473 "scan-fct_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=AEND;
    (yyval.pPlNode)->sons = (yyvsp[(4) - (5)].pPlNode);
   
;}
    break;

  case 37:
#line 482 "scan-fct_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=ALWAYS;
    (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
   
;}
    break;

  case 38:
#line 491 "scan-fct_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=SOMETIME;
    (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
   
;}
    break;

  case 39:
#line 500 "scan-fct_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=AT_MOST_ONCE;
    (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
   
;}
    break;

  case 40:
#line 510 "scan-fct_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=WITHIN;
    (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
    
    (yyval.pPlNode)->sons = (yyvsp[(4) - (5)].pPlNode);
    
   
;}
    break;

  case 41:
#line 522 "scan-fct_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=SOMETIME_AFTER;
    (yyvsp[(3) - (5)].pPlNode)->next = (yyvsp[(4) - (5)].pPlNode);
    (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
   
;}
    break;

  case 42:
#line 532 "scan-fct_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=SOMETIME_BEFORE;
    (yyvsp[(3) - (5)].pPlNode)->next = (yyvsp[(4) - (5)].pPlNode);
    (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
   
;}
    break;

  case 43:
#line 542 "scan-fct_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=ALWAYS_WITHIN;
    (yyval.pPlNode)->lh = (yyvsp[(3) - (6)].pParseExpNode);
    
    (yyvsp[(4) - (6)].pPlNode)->next = (yyvsp[(5) - (6)].pPlNode);
    (yyval.pPlNode)->sons = (yyvsp[(4) - (6)].pPlNode);
   
;}
    break;

  case 44:
#line 554 "scan-fct_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=HOLD_DURING;
    (yyval.pPlNode)->lh = (yyvsp[(3) - (6)].pParseExpNode);
    (yyval.pPlNode)->rh = (yyvsp[(4) - (6)].pParseExpNode);
   
    (yyval.pPlNode)->sons = (yyvsp[(5) - (6)].pPlNode);
   
;}
    break;

  case 45:
#line 566 "scan-fct_pddl.y"
    {
 
    (yyval.pPlNode)= new_PlNode(TEMPORALOP);
    (yyval.pPlNode)->temporal_op=HOLD_AFTER;
    (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
    
    (yyval.pPlNode)->sons = (yyvsp[(4) - (5)].pPlNode);
   
;}
    break;

  case 46:
#line 577 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = LE;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 47:
#line 585 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = LEQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 48:
#line 593 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = EQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 49:
#line 601 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = GEQ;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 50:
#line 609 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(COMP);
  (yyval.pPlNode)->comp = GE;
  (yyval.pPlNode)->lh = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pPlNode)->rh = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 51:
#line 617 "scan-fct_pddl.y"
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

  case 52:
#line 630 "scan-fct_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(pref);
  (yyval.pPlNode)->sons = (yyvsp[(4) - (5)].pPlNode);
  /*$$->sons->Pref = TRUE;*/
  (yyval.pPlNode)->sons->name = new_Token( strlen((yyvsp[(3) - (5)].pstring))+1 );
  strcpy((yyval.pPlNode)->sons->name,(yyvsp[(3) - (5)].pstring));
  isConstraintsPreference = TRUE;
;}
    break;

  case 53:
#line 640 "scan-fct_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 54:
#line 646 "scan-fct_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 55:
#line 652 "scan-fct_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(NOT);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 56:
#line 658 "scan-fct_pddl.y"
    { 
  PlNode *np = new_PlNode(NOT);
  np->sons = (yyvsp[(3) - (5)].pPlNode);
  np->next = (yyvsp[(4) - (5)].pPlNode);

  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = np;
;}
    break;

  case 57:
#line 668 "scan-fct_pddl.y"
    { 
  PlNode *np = new_PlNode(NOT);
  np->sons = (yyvsp[(3) - (5)].pPlNode);
  np->next = (yyvsp[(4) - (5)].pPlNode);

  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = np;
;}
    break;

  case 58:
#line 681 "scan-fct_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(EX);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

   (yyval.pPlNode) = pln;
   pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 59:
#line 696 "scan-fct_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(ALL);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

   (yyval.pPlNode) = pln;
   pln->sons = (yyvsp[(6) - (7)].pPlNode);


;}
    break;

  case 60:
#line 714 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = NULL;
;}
    break;

  case 61:
#line 719 "scan-fct_pddl.y"
    {
  (yyvsp[(1) - (2)].pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
  (yyval.pPlNode) = (yyvsp[(1) - (2)].pPlNode);
;}
    break;

  case 62:
#line 728 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = NULL;
;}
    break;

  case 63:
#line 733 "scan-fct_pddl.y"
    {
  (yyvsp[(1) - (2)].pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
  (yyval.pPlNode) = (yyvsp[(1) - (2)].pPlNode);
;}
    break;

  case 64:
#line 743 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = (yyvsp[(1) - (1)].pPlNode);
;}
    break;

  case 65:
#line 748 "scan-fct_pddl.y"
    {
   (yyval.pPlNode) = (yyvsp[(1) - (2)].pPlNode);
   (yyval.pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
;}
    break;

  case 66:
#line 758 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = (yyvsp[(1) - (1)].pPlNode);
;}
    break;

  case 67:
#line 763 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode( COMP );
  (yyval.pPlNode)->comp = EQ;

  (yyval.pPlNode)->lh = new_ParseExpNode( FHEAD );
  (yyval.pPlNode)->lh->atom = new_TokenList();
  (yyval.pPlNode)->lh->atom->item = new_Token( strlen((yyvsp[(4) - (8)].string))+1 );
  strcpy( (yyval.pPlNode)->lh->atom->item, (yyvsp[(4) - (8)].string) );
  (yyval.pPlNode)->lh->atom->next = (yyvsp[(5) - (8)].pTokenList);

  (yyval.pPlNode)->rh = new_ParseExpNode( NUMBER );
  (yyval.pPlNode)->rh->atom = new_TokenList();
  (yyval.pPlNode)->rh->atom->item = new_Token( strlen((yyvsp[(7) - (8)].string))+1 );
  strcpy( (yyval.pPlNode)->rh->atom->item, (yyvsp[(7) - (8)].string) );
;}
    break;

  case 68:
#line 780 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode( COMP );
  (yyval.pPlNode)->comp = EQ;

  (yyval.pPlNode)->lh = new_ParseExpNode( FHEAD );
  (yyval.pPlNode)->lh->atom = new_TokenList();
  (yyval.pPlNode)->lh->atom->item = new_Token( strlen((yyvsp[(3) - (5)].string))+1 );
  strcpy( (yyval.pPlNode)->lh->atom->item, (yyvsp[(3) - (5)].string) );

  (yyval.pPlNode)->rh = new_ParseExpNode( NUMBER );
  (yyval.pPlNode)->rh->atom = new_TokenList();
  (yyval.pPlNode)->rh->atom->item = new_Token( strlen((yyvsp[(4) - (5)].string))+1 );
  strcpy( (yyval.pPlNode)->rh->atom->item, (yyvsp[(4) - (5)].string) );
;}
    break;

  case 69:
#line 796 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode( AT );
  (yyval.pPlNode)->atom = new_TokenList();
  
  (yyval.pPlNode)->sons=new_PlNode(ATOM);
  (yyval.pPlNode)->sons->atom=new_TokenList();
  (yyval.pPlNode)->sons->atom->item = new_Token( strlen((yyvsp[(3) - (7)].string))+1 );
  strcpy( (yyval.pPlNode)->sons->atom->item, (yyvsp[(3) - (7)].string) );
  
  (yyval.pPlNode)->sons->atom->next= (yyvsp[(5) - (7)].pTokenList);
  
  isTempTimeWindows=TRUE;

    
;}
    break;

  case 70:
#line 813 "scan-fct_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode( AT );
  (yyval.pPlNode)->atom = new_TokenList();

  (yyval.pPlNode)->sons=new_PlNode(ATOM); 
  (yyval.pPlNode)->sons->atom=new_TokenList();
  (yyval.pPlNode)->sons->atom->item = new_Token( strlen((yyvsp[(3) - (10)].string))+1 );
  strcpy( (yyval.pPlNode)->sons->atom->item, (yyvsp[(3) - (10)].string) );
  
  (yyval.pPlNode)->sons->atom->next=new_TokenList();
  (yyval.pPlNode)->sons->atom->next->item = new_Token( strlen("not")+1 );
  strcpy( (yyval.pPlNode)->sons->atom->next->item, "not" );
  
  (yyval.pPlNode)->sons->atom->next->next= (yyvsp[(7) - (10)].pTokenList) 

    
;}
    break;

  case 71:
#line 845 "scan-fct_pddl.y"
    { 
  (yyval.pParseExpNode) = new_ParseExpNode( NUMBER );
  (yyval.pParseExpNode)->atom = new_TokenList();
  (yyval.pParseExpNode)->atom->item = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pParseExpNode)->atom->item, (yyvsp[(1) - (1)].string) );
;}
    break;

  case 72:
#line 853 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( FHEAD );
  (yyval.pParseExpNode)->atom = new_TokenList();
  (yyval.pParseExpNode)->atom->item = new_Token( strlen((yyvsp[(2) - (4)].string))+1 );
  strcpy( (yyval.pParseExpNode)->atom->item, (yyvsp[(2) - (4)].string) );
  (yyval.pParseExpNode)->atom->next = (yyvsp[(3) - (4)].pTokenList);
;}
    break;

  case 73:
#line 862 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( MINUS );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (4)].pParseExpNode);
;}
    break;

  case 74:
#line 868 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( AD );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 75:
#line 875 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( SU );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 76:
#line 882 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( MU );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 77:
#line 889 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( DI );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 78:
#line 901 "scan-fct_pddl.y"
    { 
  (yyval.pParseExpNode) = new_ParseExpNode( NUMBER );
  (yyval.pParseExpNode)->atom = new_TokenList();
  (yyval.pParseExpNode)->atom->item = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pParseExpNode)->atom->item, (yyvsp[(1) - (1)].string) );
;}
    break;

  case 79:
#line 909 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( FHEAD );
  (yyval.pParseExpNode)->atom = new_TokenList();
  TokenList *tl;
  tl = (yyvsp[(3) - (4)].pTokenList);

  if(strcmp((yyvsp[(2) - (4)].string), "IS-VIOLATED") == 0){
    (yyval.pParseExpNode)->atom->item = new_Token(strlen((yyvsp[(2) - (4)].string))+strlen(tl->item)+2);
      sprintf((yyval.pParseExpNode)->atom->item, "%s-%s", (yyvsp[(2) - (4)].string), tl->item);
  }
  else{
  (yyval.pParseExpNode)->atom->item = new_Token( strlen((yyvsp[(2) - (4)].string))+1 );
  strcpy( (yyval.pParseExpNode)->atom->item, (yyvsp[(2) - (4)].string) );
  (yyval.pParseExpNode)->atom->next = (yyvsp[(3) - (4)].pTokenList);
  }

  

;}
    break;

  case 80:
#line 931 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( FHEAD );
  (yyval.pParseExpNode)->atom = new_TokenList();
  TokenList *tl;
  tl = (yyvsp[(6) - (8)].pTokenList);
  
  if(strcmp((yyvsp[(5) - (8)].string), "IS-VIOLATED") == 0){
      (yyval.pParseExpNode)->atom->item = new_Token(strlen((yyvsp[(5) - (8)].string))+strlen(tl->item)+2);
      sprintf( (yyval.pParseExpNode)->atom->item, "IS-SATISFIED-%s",tl->item );
  }
  else{
      (yyval.pParseExpNode)->atom->item = new_Token(strlen((yyvsp[(5) - (8)].string))+1);
      strcpy( (yyval.pParseExpNode)->atom->item, (yyvsp[(5) - (8)].string) );
      (yyval.pParseExpNode)->atom->next = (yyvsp[(6) - (8)].pTokenList);
  }
;}
    break;

  case 81:
#line 949 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( MINUS );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (4)].pParseExpNode);
;}
    break;

  case 82:
#line 955 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( AD );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 83:
#line 962 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( AD );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 84:
#line 969 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( SU );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 85:
#line 976 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( MU );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 86:
#line 983 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( DI );
  (yyval.pParseExpNode)->leftson = (yyvsp[(3) - (5)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(4) - (5)].pParseExpNode);
;}
    break;

  case 87:
#line 995 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( AD );
  (yyval.pParseExpNode)->leftson = (yyvsp[(1) - (2)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(2) - (2)].pParseExpNode);
;}
    break;

  case 88:
#line 1002 "scan-fct_pddl.y"
    {
  (yyval.pParseExpNode) = new_ParseExpNode( AD );
  (yyval.pParseExpNode)->leftson = (yyvsp[(1) - (2)].pParseExpNode);
  (yyval.pParseExpNode)->rightson = (yyvsp[(2) - (2)].pParseExpNode);
;}
    break;

  case 89:
#line 1016 "scan-fct_pddl.y"
    { 
  (yyval.pTokenList) = (yyvsp[(3) - (4)].pTokenList);
  sis_negated = TRUE;
;}
    break;

  case 90:
#line 1022 "scan-fct_pddl.y"
    {
  (yyval.pTokenList) = (yyvsp[(1) - (1)].pTokenList);
;}
    break;

  case 91:
#line 1031 "scan-fct_pddl.y"
    { 
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = (yyvsp[(2) - (4)].pstring);
  (yyval.pTokenList)->next = (yyvsp[(3) - (4)].pTokenList);
;}
    break;

  case 92:
#line 1038 "scan-fct_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = new_Token( 5 );
  (yyval.pTokenList)->item = "=";
  (yyval.pTokenList)->next = (yyvsp[(3) - (4)].pTokenList);
;}
    break;

  case 93:
#line 1050 "scan-fct_pddl.y"
    {
  (yyval.pTokenList) = NULL;
;}
    break;

  case 94:
#line 1055 "scan-fct_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = (yyvsp[(1) - (2)].pstring);
  (yyval.pTokenList)->next = (yyvsp[(2) - (2)].pTokenList);
;}
    break;

  case 95:
#line 1066 "scan-fct_pddl.y"
    { 
  (yyval.pstring) = new_Token(strlen((yyvsp[(1) - (1)].string)) + 1);
  strcpy((yyval.pstring), (yyvsp[(1) - (1)].string));
;}
    break;

  case 96:
#line 1072 "scan-fct_pddl.y"
    { 
  (yyval.pstring) = new_Token(strlen((yyvsp[(1) - (1)].string)) + 1);
  strcpy((yyval.pstring), (yyvsp[(1) - (1)].string));
;}
    break;

  case 97:
#line 1082 "scan-fct_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = new_Token(strlen((yyvsp[(1) - (1)].string)) + 1);
  strcpy((yyval.pTokenList)->item, (yyvsp[(1) - (1)].string));
;}
    break;

  case 98:
#line 1089 "scan-fct_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = new_Token(strlen((yyvsp[(1) - (2)].string)) + 1);
  strcpy((yyval.pTokenList)->item, (yyvsp[(1) - (2)].string));
  (yyval.pTokenList)->next = (yyvsp[(2) - (2)].pTokenList);
;}
    break;

  case 99:
#line 1101 "scan-fct_pddl.y"
    { (yyval.pTypedList) = NULL; ;}
    break;

  case 100:
#line 1104 "scan-fct_pddl.y"
    { 
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (5)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (5)].string) );
  (yyval.pTypedList)->type = (yyvsp[(3) - (5)].pTokenList);
  (yyval.pTypedList)->next = (yyvsp[(5) - (5)].pTypedList);
;}
    break;

  case 101:
#line 1113 "scan-fct_pddl.y"
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

  case 102:
#line 1124 "scan-fct_pddl.y"
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

  case 103:
#line 1143 "scan-fct_pddl.y"
    { (yyval.pTypedList) = NULL; ;}
    break;

  case 104:
#line 1146 "scan-fct_pddl.y"
    { 
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (5)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (5)].string) );
  (yyval.pTypedList)->type = (yyvsp[(3) - (5)].pTokenList);
  (yyval.pTypedList)->next = (yyvsp[(5) - (5)].pTypedList);
;}
    break;

  case 105:
#line 1155 "scan-fct_pddl.y"
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

  case 106:
#line 1166 "scan-fct_pddl.y"
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

  case 107:
#line 1187 "scan-fct_pddl.y"
    { 
  (yyval.pstring) = new_Token(strlen((yyvsp[(1) - (1)].string)) + 1);
  strcpy((yyval.pstring), (yyvsp[(1) - (1)].string));
;}
    break;

  case 108:
#line 1193 "scan-fct_pddl.y"
    { 
  (yyval.pstring) = new_Token(strlen("at") + 1);
  strcpy((yyval.pstring), "at");
;}
    break;

  case 109:
#line 1219 "scan-fct_pddl.y"
    { 
  PlNode *tmp;

  tmp = new_PlNode(ATOM);
  tmp->atom = (yyvsp[(3) - (4)].pTokenList);
  (yyval.pPlNode) = new_PlNode(NOT);
  (yyval.pPlNode)->sons = tmp;
;}
    break;

  case 110:
#line 1229 "scan-fct_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(ATOM);
  (yyval.pPlNode)->atom = (yyvsp[(1) - (1)].pTokenList);
;}
    break;

  case 111:
#line 1239 "scan-fct_pddl.y"
    { 
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = (yyvsp[(2) - (4)].pstring);
  (yyval.pTokenList)->next = (yyvsp[(3) - (4)].pTokenList);
;}
    break;

  case 112:
#line 1251 "scan-fct_pddl.y"
    { (yyval.pTokenList) = NULL; ;}
    break;

  case 113:
#line 1254 "scan-fct_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = new_Token(strlen((yyvsp[(1) - (2)].string)) + 1);
  strcpy((yyval.pTokenList)->item, (yyvsp[(1) - (2)].string));
  (yyval.pTokenList)->next = (yyvsp[(2) - (2)].pTokenList);
;}
    break;


/* Line 1267 of yacc.c.  */
#line 2856 "scan-fct_pddl.tab.c"
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


#line 1263 "scan-fct_pddl.y"



#include "lex.fct_pddl.c"


/**********************************************************************
 * Functions
 **********************************************************************/


/* 
 * call	bison -pfct -bscan-fct scan-fct.y
 */
void fcterr( int errno_, char *par ) {

/*   sact_err = errno_; */

/*   if ( sact_err_par ) { */
/*     free( sact_err_par ); */
/*   } */
/*   if ( par ) { */
/*     sact_err_par = new_Token( strlen(par)+1 ); */
/*     strcpy( sact_err_par, par); */
/*   } else { */
/*     sact_err_par = NULL; */
/*   } */

}



int yyerror( char *msg )

{
  fflush( stdout );
  fprintf(stderr,"\n%s: syntax error in line %d, '%s':\n", 
	  gact_filename, lineno, fct_pddltext );

  if ( sact_err_par ) {
    fprintf(stderr, "%s%s\n", serrmsg[sact_err], sact_err_par );
  } else {
    fprintf(stderr,"%s\n", serrmsg[sact_err] );
  }

  exit( 1 );

}



void load_fct_file( char *filename ) 

{

  FILE *fp;/* pointer to input files */
  char tmp[MAX_LENGTH] = "";

  /* open fact file 
   */
  if( ( fp = fopen( filename, "r" ) ) == NULL ) {
    sprintf(tmp, "\nff: can't find fact file: %s\n\n", filename );
    perror(tmp);
    exit ( 1 );
  }

  gact_filename = filename;
  lineno = 1; 
  fct_pddlin = fp;

  yyparse();

  fclose( fp );/* and close file again */

}


