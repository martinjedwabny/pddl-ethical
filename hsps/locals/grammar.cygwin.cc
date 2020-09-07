#define YY_PDDL_Parser_h_included

/*  A Bison++ parser, made from pddl2.y  */

 /* with Bison++ version bison++ Version 1.21-8, adapted from GNU bison by coetmeur@icdc.fr
  */


#line 1 "/lib/bison.cc"
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* HEADER SECTION */
#if defined( _MSDOS ) || defined(MSDOS) || defined(__MSDOS__) 
#define __MSDOS_AND_ALIKE
#endif
#if defined(_WINDOWS) && defined(_MSC_VER)
#define __HAVE_NO_ALLOCA
#define __MSDOS_AND_ALIKE
#endif

#ifndef alloca
#if defined( __GNUC__)
#define alloca __builtin_alloca

#elif (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc)  || defined (__sgi)
#include <alloca.h>

#elif defined (__MSDOS_AND_ALIKE)
#include <malloc.h>
#ifndef __TURBOC__
/* MS C runtime lib */
#define alloca _alloca
#endif

#elif defined(_AIX)
#include <malloc.h>
#pragma alloca

#elif defined(__hpux)
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */

#endif /* not _AIX  not MSDOS, or __TURBOC__ or _AIX, not sparc.  */
#endif /* alloca not defined.  */
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif
#ifdef __cplusplus
#ifndef YY_USE_CLASS
#define YY_USE_CLASS
#endif
#else
#ifndef __STDC__
#define const
#endif
#endif
#include <stdio.h>
#define YYBISON 1  

/* #line 73 "/lib/bison.cc" */
#line 85 "grammar.cc"
#define YY_PDDL_Parser_ERROR  log_error
#define YY_PDDL_Parser_ERROR_BODY  = 0
#define YY_PDDL_Parser_ERROR_VERBOSE  1
#define YY_PDDL_Parser_LEX  next_token
#define YY_PDDL_Parser_LEX_BODY  = 0
#define YY_PDDL_Parser_DEBUG  1
#define YY_PDDL_Parser_INHERIT  : public PDDL_Base
#define YY_PDDL_Parser_CONSTRUCTOR_PARAM  StringTable& t
#define YY_PDDL_Parser_CONSTRUCTOR_INIT  : PDDL_Base(t), error_flag(false), \
  current_param(0), current_n_param(0), stored_n_param(0), \
  current_atom(0), current_context(0), stored_context(0), \
  current_item(0), current_goal(0, 0), current_preference_name(0), current_entry(0)
#define YY_PDDL_Parser_MEMBERS  \
public: \
virtual std::ostream& at_position(std::ostream& s) = 0; \
virtual char*         current_file() = 0; \
bool    error_flag; \
private: \
variable_vec current_param; \
index_type   current_n_param; \
index_type   stored_n_param; \
AtomBase*    current_atom; \
Context*     current_context; \
Context*     stored_context; \
DKEL_Item*   current_item; \
lvector<ConjunctiveGoal*,16> current_goal; \
Symbol*      current_preference_name; \
HTableEntry* current_entry; \
char*        current_plan_file;
#line 33 "pddl2.y"

#include <stdlib.h>
#include "base.h"

#line 38 "pddl2.y"
typedef union {
  StringTable::Cell*         sym;
  PDDL_Base::Expression*     exp;
  PDDL_Base::ListExpression* lst;
  PDDL_Base::Relation*       rel;
  PDDL_Base::Goal*           goal;
  PDDL_Base::mode_keyword    tkw;
  PDDL_Base::relation_type   rkw;
  PDDL_Base::set_constraint_keyword sckw;
  NNTYPE                     rval;
  int                        ival;
  char*                      sval;
} yy_PDDL_Parser_stype;
#define YY_PDDL_Parser_STYPE yy_PDDL_Parser_stype

#line 73 "/lib/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_PDDL_Parser_BISON 1
#ifndef YY_PDDL_Parser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_PDDL_Parser_COMPATIBILITY 1
#else
#define  YY_PDDL_Parser_COMPATIBILITY 0
#endif
#endif

#if YY_PDDL_Parser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_PDDL_Parser_LTYPE
#define YY_PDDL_Parser_LTYPE YYLTYPE
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_PDDL_Parser_STYPE 
#define YY_PDDL_Parser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_PDDL_Parser_DEBUG
#define  YY_PDDL_Parser_DEBUG YYDEBUG
#endif
#endif
#ifdef YY_PDDL_Parser_STYPE
#ifndef yystype
#define yystype YY_PDDL_Parser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_PDDL_Parser_USE_GOTO
#define YY_PDDL_Parser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_PDDL_Parser_USE_GOTO
#define YY_PDDL_Parser_USE_GOTO 0
#endif

#ifndef YY_PDDL_Parser_PURE

/* #line 117 "/lib/bison.cc" */
#line 183 "grammar.cc"

#line 117 "/lib/bison.cc"
/*  YY_PDDL_Parser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 121 "/lib/bison.cc" */
#line 192 "grammar.cc"

#line 121 "/lib/bison.cc"
/* prefix */
#ifndef YY_PDDL_Parser_DEBUG

/* #line 123 "/lib/bison.cc" */
#line 199 "grammar.cc"

#line 123 "/lib/bison.cc"
/* YY_PDDL_Parser_DEBUG */
#endif


#ifndef YY_PDDL_Parser_LSP_NEEDED

/* #line 128 "/lib/bison.cc" */
#line 209 "grammar.cc"

#line 128 "/lib/bison.cc"
 /* YY_PDDL_Parser_LSP_NEEDED*/
#endif



/* DEFAULT LTYPE*/
#ifdef YY_PDDL_Parser_LSP_NEEDED
#ifndef YY_PDDL_Parser_LTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YY_PDDL_Parser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
      /* We used to use `unsigned long' as YY_PDDL_Parser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

#ifndef YY_PDDL_Parser_STYPE
#define YY_PDDL_Parser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_PDDL_Parser_PARSE
#define YY_PDDL_Parser_PARSE yyparse
#endif
#ifndef YY_PDDL_Parser_LEX
#define YY_PDDL_Parser_LEX yylex
#endif
#ifndef YY_PDDL_Parser_LVAL
#define YY_PDDL_Parser_LVAL yylval
#endif
#ifndef YY_PDDL_Parser_LLOC
#define YY_PDDL_Parser_LLOC yylloc
#endif
#ifndef YY_PDDL_Parser_CHAR
#define YY_PDDL_Parser_CHAR yychar
#endif
#ifndef YY_PDDL_Parser_NERRS
#define YY_PDDL_Parser_NERRS yynerrs
#endif
#ifndef YY_PDDL_Parser_DEBUG_FLAG
#define YY_PDDL_Parser_DEBUG_FLAG yydebug
#endif
#ifndef YY_PDDL_Parser_ERROR
#define YY_PDDL_Parser_ERROR yyerror
#endif
#ifndef YY_PDDL_Parser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_PDDL_Parser_PARSE_PARAM
#ifndef YY_PDDL_Parser_PARSE_PARAM_DEF
#define YY_PDDL_Parser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_PDDL_Parser_PARSE_PARAM
#define YY_PDDL_Parser_PARSE_PARAM void
#endif
#endif
#if YY_PDDL_Parser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_PDDL_Parser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_PDDL_Parser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_PDDL_Parser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_PDDL_Parser_PURE
#ifndef YYPURE
#define YYPURE YY_PDDL_Parser_PURE
#endif
#endif
#ifdef YY_PDDL_Parser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_PDDL_Parser_DEBUG 
#endif
#endif
#ifndef YY_PDDL_Parser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_PDDL_Parser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_PDDL_Parser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_PDDL_Parser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif
#endif
#ifndef YY_USE_CLASS
/* TOKEN C */

/* #line 236 "/lib/bison.cc" */
#line 322 "grammar.cc"
#define	TK_OPEN	258
#define	TK_CLOSE	259
#define	TK_OPEN_SQ	260
#define	TK_CLOSE_SQ	261
#define	TK_GREATER	262
#define	TK_GREATEQ	263
#define	TK_LESS	264
#define	TK_LESSEQ	265
#define	TK_COLON	266
#define	TK_HASHT	267
#define	TK_EQ	268
#define	TK_HYPHEN	269
#define	TK_PLUS	270
#define	TK_MUL	271
#define	TK_DIV	272
#define	TK_UMINUS	273
#define	TK_NEW_SYMBOL	274
#define	TK_OBJ_SYMBOL	275
#define	TK_TYPE_SYMBOL	276
#define	TK_PRED_SYMBOL	277
#define	TK_FUN_SYMBOL	278
#define	TK_VAR_SYMBOL	279
#define	TK_ACTION_SYMBOL	280
#define	TK_MISC_SYMBOL	281
#define	TK_KEYWORD	282
#define	TK_NEW_VAR_SYMBOL	283
#define	TK_PREFERENCE_SYMBOL	284
#define	TK_SET_SYMBOL	285
#define	TK_FLOAT	286
#define	TK_INT	287
#define	TK_STRING	288
#define	KW_REQS	289
#define	KW_CONSTANTS	290
#define	KW_PREDS	291
#define	KW_FUNS	292
#define	KW_TYPES	293
#define	KW_DEFINE	294
#define	KW_DOMAIN	295
#define	KW_ACTION	296
#define	KW_PROCESS	297
#define	KW_EVENT	298
#define	KW_ARGS	299
#define	KW_PRE	300
#define	KW_COND	301
#define	KW_START_PRE	302
#define	KW_END_PRE	303
#define	KW_AT_START	304
#define	KW_AT_END	305
#define	KW_OVER_ALL	306
#define	KW_EFFECT	307
#define	KW_INITIAL_EFFECT	308
#define	KW_FINAL_EFFECT	309
#define	KW_INVARIANT	310
#define	KW_DURATION	311
#define	KW_AND	312
#define	KW_OR	313
#define	KW_EXISTS	314
#define	KW_FORALL	315
#define	KW_IMPLY	316
#define	KW_NOT	317
#define	KW_WHEN	318
#define	KW_EITHER	319
#define	KW_PROBLEM	320
#define	KW_FORDOMAIN	321
#define	KW_OBJECTS	322
#define	KW_INIT	323
#define	KW_GOAL	324
#define	KW_LENGTH	325
#define	KW_SERIAL	326
#define	KW_PARALLEL	327
#define	KW_METRIC	328
#define	KW_MINIMIZE	329
#define	KW_MAXIMIZE	330
#define	KW_DURATION_VAR	331
#define	KW_TOTAL_TIME	332
#define	KW_INCREASE	333
#define	KW_DECREASE	334
#define	KW_SCALE_UP	335
#define	KW_SCALE_DOWN	336
#define	KW_ASSIGN	337
#define	KW_TAG	338
#define	KW_NAME	339
#define	KW_VARS	340
#define	KW_SET_CONSTRAINT	341
#define	KW_SETOF	342
#define	KW_AT_LEAST_N	343
#define	KW_AT_MOST_N	344
#define	KW_EXACTLY_N	345
#define	KW_CONTEXT	346
#define	KW_IRRELEVANT	347
#define	KW_PLAN	348
#define	KW_HEURISTIC	349
#define	KW_OPT	350
#define	KW_INF	351
#define	KW_FACT	352
#define	KW_SET	353
#define	KW_EXPANSION	354
#define	KW_TASKS	355
#define	KW_TYPEOF	356
#define	KW_PREFERENCE	357
#define	KW_VIOLATED	358
#define	KW_WITHIN	359
#define	KW_ASSOC	360
#define	KW_CONSTRAINTS	361
#define	KW_ALWAYS	362
#define	KW_SOMETIME	363
#define	KW_AT_MOST_ONCE	364
#define	KW_SOMETIME_BEFORE	365
#define	KW_SOMETIME_AFTER	366
#define	KW_ALWAYS_WITHIN	367


#line 236 "/lib/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
#ifndef YY_PDDL_Parser_CLASS
#define YY_PDDL_Parser_CLASS PDDL_Parser
#endif
#ifndef YY_PDDL_Parser_INHERIT
#define YY_PDDL_Parser_INHERIT
#endif
#ifndef YY_PDDL_Parser_MEMBERS
#define YY_PDDL_Parser_MEMBERS 
#endif
#ifndef YY_PDDL_Parser_LEX_BODY
#define YY_PDDL_Parser_LEX_BODY  
#endif
#ifndef YY_PDDL_Parser_ERROR_BODY
#define YY_PDDL_Parser_ERROR_BODY  
#endif
#ifndef YY_PDDL_Parser_CONSTRUCTOR_PARAM
#define YY_PDDL_Parser_CONSTRUCTOR_PARAM
#endif
#ifndef YY_PDDL_Parser_CONSTRUCTOR_CODE
#define YY_PDDL_Parser_CONSTRUCTOR_CODE
#endif
#ifndef YY_PDDL_Parser_CONSTRUCTOR_INIT
#define YY_PDDL_Parser_CONSTRUCTOR_INIT
#endif
/* choose between enum and const */
#ifndef YY_PDDL_Parser_USE_CONST_TOKEN
#define YY_PDDL_Parser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */ 
#endif
#if YY_PDDL_Parser_USE_CONST_TOKEN != 0
#ifndef YY_PDDL_Parser_ENUM_TOKEN
#define YY_PDDL_Parser_ENUM_TOKEN yy_PDDL_Parser_enum_token
#endif
#endif

class YY_PDDL_Parser_CLASS YY_PDDL_Parser_INHERIT
{
public: 
#if YY_PDDL_Parser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 280 "/lib/bison.cc" */
#line 482 "grammar.cc"
static const int TK_OPEN;
static const int TK_CLOSE;
static const int TK_OPEN_SQ;
static const int TK_CLOSE_SQ;
static const int TK_GREATER;
static const int TK_GREATEQ;
static const int TK_LESS;
static const int TK_LESSEQ;
static const int TK_COLON;
static const int TK_HASHT;
static const int TK_EQ;
static const int TK_HYPHEN;
static const int TK_PLUS;
static const int TK_MUL;
static const int TK_DIV;
static const int TK_UMINUS;
static const int TK_NEW_SYMBOL;
static const int TK_OBJ_SYMBOL;
static const int TK_TYPE_SYMBOL;
static const int TK_PRED_SYMBOL;
static const int TK_FUN_SYMBOL;
static const int TK_VAR_SYMBOL;
static const int TK_ACTION_SYMBOL;
static const int TK_MISC_SYMBOL;
static const int TK_KEYWORD;
static const int TK_NEW_VAR_SYMBOL;
static const int TK_PREFERENCE_SYMBOL;
static const int TK_SET_SYMBOL;
static const int TK_FLOAT;
static const int TK_INT;
static const int TK_STRING;
static const int KW_REQS;
static const int KW_CONSTANTS;
static const int KW_PREDS;
static const int KW_FUNS;
static const int KW_TYPES;
static const int KW_DEFINE;
static const int KW_DOMAIN;
static const int KW_ACTION;
static const int KW_PROCESS;
static const int KW_EVENT;
static const int KW_ARGS;
static const int KW_PRE;
static const int KW_COND;
static const int KW_START_PRE;
static const int KW_END_PRE;
static const int KW_AT_START;
static const int KW_AT_END;
static const int KW_OVER_ALL;
static const int KW_EFFECT;
static const int KW_INITIAL_EFFECT;
static const int KW_FINAL_EFFECT;
static const int KW_INVARIANT;
static const int KW_DURATION;
static const int KW_AND;
static const int KW_OR;
static const int KW_EXISTS;
static const int KW_FORALL;
static const int KW_IMPLY;
static const int KW_NOT;
static const int KW_WHEN;
static const int KW_EITHER;
static const int KW_PROBLEM;
static const int KW_FORDOMAIN;
static const int KW_OBJECTS;
static const int KW_INIT;
static const int KW_GOAL;
static const int KW_LENGTH;
static const int KW_SERIAL;
static const int KW_PARALLEL;
static const int KW_METRIC;
static const int KW_MINIMIZE;
static const int KW_MAXIMIZE;
static const int KW_DURATION_VAR;
static const int KW_TOTAL_TIME;
static const int KW_INCREASE;
static const int KW_DECREASE;
static const int KW_SCALE_UP;
static const int KW_SCALE_DOWN;
static const int KW_ASSIGN;
static const int KW_TAG;
static const int KW_NAME;
static const int KW_VARS;
static const int KW_SET_CONSTRAINT;
static const int KW_SETOF;
static const int KW_AT_LEAST_N;
static const int KW_AT_MOST_N;
static const int KW_EXACTLY_N;
static const int KW_CONTEXT;
static const int KW_IRRELEVANT;
static const int KW_PLAN;
static const int KW_HEURISTIC;
static const int KW_OPT;
static const int KW_INF;
static const int KW_FACT;
static const int KW_SET;
static const int KW_EXPANSION;
static const int KW_TASKS;
static const int KW_TYPEOF;
static const int KW_PREFERENCE;
static const int KW_VIOLATED;
static const int KW_WITHIN;
static const int KW_ASSOC;
static const int KW_CONSTRAINTS;
static const int KW_ALWAYS;
static const int KW_SOMETIME;
static const int KW_AT_MOST_ONCE;
static const int KW_SOMETIME_BEFORE;
static const int KW_SOMETIME_AFTER;
static const int KW_ALWAYS_WITHIN;


#line 280 "/lib/bison.cc"
 /* decl const */
#else
enum YY_PDDL_Parser_ENUM_TOKEN { YY_PDDL_Parser_NULL_TOKEN=0

/* #line 283 "/lib/bison.cc" */
#line 601 "grammar.cc"
	,TK_OPEN=258
	,TK_CLOSE=259
	,TK_OPEN_SQ=260
	,TK_CLOSE_SQ=261
	,TK_GREATER=262
	,TK_GREATEQ=263
	,TK_LESS=264
	,TK_LESSEQ=265
	,TK_COLON=266
	,TK_HASHT=267
	,TK_EQ=268
	,TK_HYPHEN=269
	,TK_PLUS=270
	,TK_MUL=271
	,TK_DIV=272
	,TK_UMINUS=273
	,TK_NEW_SYMBOL=274
	,TK_OBJ_SYMBOL=275
	,TK_TYPE_SYMBOL=276
	,TK_PRED_SYMBOL=277
	,TK_FUN_SYMBOL=278
	,TK_VAR_SYMBOL=279
	,TK_ACTION_SYMBOL=280
	,TK_MISC_SYMBOL=281
	,TK_KEYWORD=282
	,TK_NEW_VAR_SYMBOL=283
	,TK_PREFERENCE_SYMBOL=284
	,TK_SET_SYMBOL=285
	,TK_FLOAT=286
	,TK_INT=287
	,TK_STRING=288
	,KW_REQS=289
	,KW_CONSTANTS=290
	,KW_PREDS=291
	,KW_FUNS=292
	,KW_TYPES=293
	,KW_DEFINE=294
	,KW_DOMAIN=295
	,KW_ACTION=296
	,KW_PROCESS=297
	,KW_EVENT=298
	,KW_ARGS=299
	,KW_PRE=300
	,KW_COND=301
	,KW_START_PRE=302
	,KW_END_PRE=303
	,KW_AT_START=304
	,KW_AT_END=305
	,KW_OVER_ALL=306
	,KW_EFFECT=307
	,KW_INITIAL_EFFECT=308
	,KW_FINAL_EFFECT=309
	,KW_INVARIANT=310
	,KW_DURATION=311
	,KW_AND=312
	,KW_OR=313
	,KW_EXISTS=314
	,KW_FORALL=315
	,KW_IMPLY=316
	,KW_NOT=317
	,KW_WHEN=318
	,KW_EITHER=319
	,KW_PROBLEM=320
	,KW_FORDOMAIN=321
	,KW_OBJECTS=322
	,KW_INIT=323
	,KW_GOAL=324
	,KW_LENGTH=325
	,KW_SERIAL=326
	,KW_PARALLEL=327
	,KW_METRIC=328
	,KW_MINIMIZE=329
	,KW_MAXIMIZE=330
	,KW_DURATION_VAR=331
	,KW_TOTAL_TIME=332
	,KW_INCREASE=333
	,KW_DECREASE=334
	,KW_SCALE_UP=335
	,KW_SCALE_DOWN=336
	,KW_ASSIGN=337
	,KW_TAG=338
	,KW_NAME=339
	,KW_VARS=340
	,KW_SET_CONSTRAINT=341
	,KW_SETOF=342
	,KW_AT_LEAST_N=343
	,KW_AT_MOST_N=344
	,KW_EXACTLY_N=345
	,KW_CONTEXT=346
	,KW_IRRELEVANT=347
	,KW_PLAN=348
	,KW_HEURISTIC=349
	,KW_OPT=350
	,KW_INF=351
	,KW_FACT=352
	,KW_SET=353
	,KW_EXPANSION=354
	,KW_TASKS=355
	,KW_TYPEOF=356
	,KW_PREFERENCE=357
	,KW_VIOLATED=358
	,KW_WITHIN=359
	,KW_ASSOC=360
	,KW_CONSTRAINTS=361
	,KW_ALWAYS=362
	,KW_SOMETIME=363
	,KW_AT_MOST_ONCE=364
	,KW_SOMETIME_BEFORE=365
	,KW_SOMETIME_AFTER=366
	,KW_ALWAYS_WITHIN=367


#line 283 "/lib/bison.cc"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_PDDL_Parser_PARSE (YY_PDDL_Parser_PARSE_PARAM);
 virtual void YY_PDDL_Parser_ERROR(char *msg) YY_PDDL_Parser_ERROR_BODY;
#ifdef YY_PDDL_Parser_PURE
#ifdef YY_PDDL_Parser_LSP_NEEDED
 virtual int  YY_PDDL_Parser_LEX (YY_PDDL_Parser_STYPE *YY_PDDL_Parser_LVAL,YY_PDDL_Parser_LTYPE *YY_PDDL_Parser_LLOC) YY_PDDL_Parser_LEX_BODY;
#else
 virtual int  YY_PDDL_Parser_LEX (YY_PDDL_Parser_STYPE *YY_PDDL_Parser_LVAL) YY_PDDL_Parser_LEX_BODY;
#endif
#else
 virtual int YY_PDDL_Parser_LEX() YY_PDDL_Parser_LEX_BODY;
 YY_PDDL_Parser_STYPE YY_PDDL_Parser_LVAL;
#ifdef YY_PDDL_Parser_LSP_NEEDED
 YY_PDDL_Parser_LTYPE YY_PDDL_Parser_LLOC;
#endif
 int   YY_PDDL_Parser_NERRS;
 int    YY_PDDL_Parser_CHAR;
#endif
#if YY_PDDL_Parser_DEBUG != 0
 int YY_PDDL_Parser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_PDDL_Parser_CLASS(YY_PDDL_Parser_CONSTRUCTOR_PARAM);
public:
 YY_PDDL_Parser_MEMBERS 
};
/* other declare folow */
#if YY_PDDL_Parser_USE_CONST_TOKEN != 0

/* #line 314 "/lib/bison.cc" */
#line 748 "grammar.cc"
const int YY_PDDL_Parser_CLASS::TK_OPEN=258;
const int YY_PDDL_Parser_CLASS::TK_CLOSE=259;
const int YY_PDDL_Parser_CLASS::TK_OPEN_SQ=260;
const int YY_PDDL_Parser_CLASS::TK_CLOSE_SQ=261;
const int YY_PDDL_Parser_CLASS::TK_GREATER=262;
const int YY_PDDL_Parser_CLASS::TK_GREATEQ=263;
const int YY_PDDL_Parser_CLASS::TK_LESS=264;
const int YY_PDDL_Parser_CLASS::TK_LESSEQ=265;
const int YY_PDDL_Parser_CLASS::TK_COLON=266;
const int YY_PDDL_Parser_CLASS::TK_HASHT=267;
const int YY_PDDL_Parser_CLASS::TK_EQ=268;
const int YY_PDDL_Parser_CLASS::TK_HYPHEN=269;
const int YY_PDDL_Parser_CLASS::TK_PLUS=270;
const int YY_PDDL_Parser_CLASS::TK_MUL=271;
const int YY_PDDL_Parser_CLASS::TK_DIV=272;
const int YY_PDDL_Parser_CLASS::TK_UMINUS=273;
const int YY_PDDL_Parser_CLASS::TK_NEW_SYMBOL=274;
const int YY_PDDL_Parser_CLASS::TK_OBJ_SYMBOL=275;
const int YY_PDDL_Parser_CLASS::TK_TYPE_SYMBOL=276;
const int YY_PDDL_Parser_CLASS::TK_PRED_SYMBOL=277;
const int YY_PDDL_Parser_CLASS::TK_FUN_SYMBOL=278;
const int YY_PDDL_Parser_CLASS::TK_VAR_SYMBOL=279;
const int YY_PDDL_Parser_CLASS::TK_ACTION_SYMBOL=280;
const int YY_PDDL_Parser_CLASS::TK_MISC_SYMBOL=281;
const int YY_PDDL_Parser_CLASS::TK_KEYWORD=282;
const int YY_PDDL_Parser_CLASS::TK_NEW_VAR_SYMBOL=283;
const int YY_PDDL_Parser_CLASS::TK_PREFERENCE_SYMBOL=284;
const int YY_PDDL_Parser_CLASS::TK_SET_SYMBOL=285;
const int YY_PDDL_Parser_CLASS::TK_FLOAT=286;
const int YY_PDDL_Parser_CLASS::TK_INT=287;
const int YY_PDDL_Parser_CLASS::TK_STRING=288;
const int YY_PDDL_Parser_CLASS::KW_REQS=289;
const int YY_PDDL_Parser_CLASS::KW_CONSTANTS=290;
const int YY_PDDL_Parser_CLASS::KW_PREDS=291;
const int YY_PDDL_Parser_CLASS::KW_FUNS=292;
const int YY_PDDL_Parser_CLASS::KW_TYPES=293;
const int YY_PDDL_Parser_CLASS::KW_DEFINE=294;
const int YY_PDDL_Parser_CLASS::KW_DOMAIN=295;
const int YY_PDDL_Parser_CLASS::KW_ACTION=296;
const int YY_PDDL_Parser_CLASS::KW_PROCESS=297;
const int YY_PDDL_Parser_CLASS::KW_EVENT=298;
const int YY_PDDL_Parser_CLASS::KW_ARGS=299;
const int YY_PDDL_Parser_CLASS::KW_PRE=300;
const int YY_PDDL_Parser_CLASS::KW_COND=301;
const int YY_PDDL_Parser_CLASS::KW_START_PRE=302;
const int YY_PDDL_Parser_CLASS::KW_END_PRE=303;
const int YY_PDDL_Parser_CLASS::KW_AT_START=304;
const int YY_PDDL_Parser_CLASS::KW_AT_END=305;
const int YY_PDDL_Parser_CLASS::KW_OVER_ALL=306;
const int YY_PDDL_Parser_CLASS::KW_EFFECT=307;
const int YY_PDDL_Parser_CLASS::KW_INITIAL_EFFECT=308;
const int YY_PDDL_Parser_CLASS::KW_FINAL_EFFECT=309;
const int YY_PDDL_Parser_CLASS::KW_INVARIANT=310;
const int YY_PDDL_Parser_CLASS::KW_DURATION=311;
const int YY_PDDL_Parser_CLASS::KW_AND=312;
const int YY_PDDL_Parser_CLASS::KW_OR=313;
const int YY_PDDL_Parser_CLASS::KW_EXISTS=314;
const int YY_PDDL_Parser_CLASS::KW_FORALL=315;
const int YY_PDDL_Parser_CLASS::KW_IMPLY=316;
const int YY_PDDL_Parser_CLASS::KW_NOT=317;
const int YY_PDDL_Parser_CLASS::KW_WHEN=318;
const int YY_PDDL_Parser_CLASS::KW_EITHER=319;
const int YY_PDDL_Parser_CLASS::KW_PROBLEM=320;
const int YY_PDDL_Parser_CLASS::KW_FORDOMAIN=321;
const int YY_PDDL_Parser_CLASS::KW_OBJECTS=322;
const int YY_PDDL_Parser_CLASS::KW_INIT=323;
const int YY_PDDL_Parser_CLASS::KW_GOAL=324;
const int YY_PDDL_Parser_CLASS::KW_LENGTH=325;
const int YY_PDDL_Parser_CLASS::KW_SERIAL=326;
const int YY_PDDL_Parser_CLASS::KW_PARALLEL=327;
const int YY_PDDL_Parser_CLASS::KW_METRIC=328;
const int YY_PDDL_Parser_CLASS::KW_MINIMIZE=329;
const int YY_PDDL_Parser_CLASS::KW_MAXIMIZE=330;
const int YY_PDDL_Parser_CLASS::KW_DURATION_VAR=331;
const int YY_PDDL_Parser_CLASS::KW_TOTAL_TIME=332;
const int YY_PDDL_Parser_CLASS::KW_INCREASE=333;
const int YY_PDDL_Parser_CLASS::KW_DECREASE=334;
const int YY_PDDL_Parser_CLASS::KW_SCALE_UP=335;
const int YY_PDDL_Parser_CLASS::KW_SCALE_DOWN=336;
const int YY_PDDL_Parser_CLASS::KW_ASSIGN=337;
const int YY_PDDL_Parser_CLASS::KW_TAG=338;
const int YY_PDDL_Parser_CLASS::KW_NAME=339;
const int YY_PDDL_Parser_CLASS::KW_VARS=340;
const int YY_PDDL_Parser_CLASS::KW_SET_CONSTRAINT=341;
const int YY_PDDL_Parser_CLASS::KW_SETOF=342;
const int YY_PDDL_Parser_CLASS::KW_AT_LEAST_N=343;
const int YY_PDDL_Parser_CLASS::KW_AT_MOST_N=344;
const int YY_PDDL_Parser_CLASS::KW_EXACTLY_N=345;
const int YY_PDDL_Parser_CLASS::KW_CONTEXT=346;
const int YY_PDDL_Parser_CLASS::KW_IRRELEVANT=347;
const int YY_PDDL_Parser_CLASS::KW_PLAN=348;
const int YY_PDDL_Parser_CLASS::KW_HEURISTIC=349;
const int YY_PDDL_Parser_CLASS::KW_OPT=350;
const int YY_PDDL_Parser_CLASS::KW_INF=351;
const int YY_PDDL_Parser_CLASS::KW_FACT=352;
const int YY_PDDL_Parser_CLASS::KW_SET=353;
const int YY_PDDL_Parser_CLASS::KW_EXPANSION=354;
const int YY_PDDL_Parser_CLASS::KW_TASKS=355;
const int YY_PDDL_Parser_CLASS::KW_TYPEOF=356;
const int YY_PDDL_Parser_CLASS::KW_PREFERENCE=357;
const int YY_PDDL_Parser_CLASS::KW_VIOLATED=358;
const int YY_PDDL_Parser_CLASS::KW_WITHIN=359;
const int YY_PDDL_Parser_CLASS::KW_ASSOC=360;
const int YY_PDDL_Parser_CLASS::KW_CONSTRAINTS=361;
const int YY_PDDL_Parser_CLASS::KW_ALWAYS=362;
const int YY_PDDL_Parser_CLASS::KW_SOMETIME=363;
const int YY_PDDL_Parser_CLASS::KW_AT_MOST_ONCE=364;
const int YY_PDDL_Parser_CLASS::KW_SOMETIME_BEFORE=365;
const int YY_PDDL_Parser_CLASS::KW_SOMETIME_AFTER=366;
const int YY_PDDL_Parser_CLASS::KW_ALWAYS_WITHIN=367;


#line 314 "/lib/bison.cc"
 /* const YY_PDDL_Parser_CLASS::token */
#endif
/*apres const  */
YY_PDDL_Parser_CLASS::YY_PDDL_Parser_CLASS(YY_PDDL_Parser_CONSTRUCTOR_PARAM) YY_PDDL_Parser_CONSTRUCTOR_INIT
{
#if YY_PDDL_Parser_DEBUG != 0
YY_PDDL_Parser_DEBUG_FLAG=0;
#endif
YY_PDDL_Parser_CONSTRUCTOR_CODE;
};
#endif

/* #line 325 "/lib/bison.cc" */
#line 875 "grammar.cc"


#define	YYFINAL		983
#define	YYFLAG		-32768
#define	YYNTBASE	113

#define YYTRANSLATE(x) ((unsigned)(x) <= 367 ? yytranslate[x] : 322)

static const char yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
    86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
    96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
   106,   107,   108,   109,   110,   111,   112
};

#if YY_PDDL_Parser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     6,     9,    12,    15,    16,    22,    28,    31,
    34,    37,    40,    43,    46,    49,    52,    53,    58,    60,
    62,    64,    66,    68,    70,    72,    74,    76,    78,    80,
    82,    84,    89,    94,    97,   100,   103,   104,   109,   114,
   117,   119,   120,   126,   130,   135,   138,   139,   142,   145,
   147,   149,   154,   159,   162,   164,   165,   171,   175,   180,
   185,   190,   193,   194,   197,   200,   201,   206,   211,   216,
   219,   220,   223,   225,   227,   229,   231,   232,   239,   244,
   245,   252,   256,   260,   264,   268,   272,   275,   279,   280,
   281,   287,   288,   294,   296,   301,   304,   306,   308,   310,
   312,   314,   316,   318,   320,   321,   327,   333,   334,   343,
   352,   353,   362,   371,   372,   384,   396,   399,   402,   403,
   409,   418,   420,   422,   424,   426,   428,   433,   439,   445,
   451,   457,   461,   463,   465,   469,   474,   476,   481,   483,
   486,   489,   490,   491,   492,   504,   505,   506,   516,   517,
   518,   533,   534,   535,   548,   553,   555,   558,   564,   565,
   571,   572,   581,   582,   591,   592,   604,   606,   611,   614,
   616,   617,   618,   628,   629,   636,   637,   638,   651,   652,
   661,   663,   665,   671,   673,   675,   680,   682,   684,   685,
   691,   692,   701,   702,   711,   712,   724,   725,   735,   736,
   746,   747,   757,   758,   771,   772,   785,   786,   799,   801,
   806,   808,   811,   813,   815,   817,   823,   825,   831,   833,
   835,   841,   843,   845,   846,   857,   860,   861,   862,   868,
   874,   880,   885,   891,   894,   897,   900,   903,   906,   909,
   912,   915,   918,   919,   924,   927,   930,   931,   932,   938,
   939,   949,   955,   960,   968,   973,   981,   983,   989,   992,
   993,   995,   996,  1002,  1005,  1006,  1007,  1013,  1014,  1023,
  1025,  1030,  1035,  1040,  1046,  1052,  1058,  1065,  1071,  1076,
  1078,  1080,  1082,  1088,  1094,  1096,  1098,  1100,  1101,  1102,
  1115,  1120,  1121,  1127,  1132,  1134,  1136,  1137,  1145,  1146,
  1154,  1155,  1156,  1169,  1174,  1177,  1180,  1183,  1186,  1187,
  1190,  1193,  1196,  1197,  1203,  1206,  1212,  1217,  1219,  1220,
  1224,  1231,  1235,  1242,  1246,  1253,  1254,  1257,  1263,  1264,
  1267,  1273,  1276,  1282,  1283,  1286,  1288,  1290,  1292,  1294,
  1296,  1298,  1299,  1305,  1306,  1315,  1321,  1322,  1331,  1332,
  1344,  1345,  1357,  1366,  1367,  1376,  1377,  1389,  1390,  1402,
  1403,  1418,  1424,  1426,  1428,  1430,  1433,  1436,  1437,  1438,
  1444,  1445,  1454,  1455,  1456,  1457,  1473,  1474,  1480,  1485,
  1488,  1491,  1494,  1495,  1498,  1499,  1507,  1509,  1511,  1514,
  1516,  1517,  1526,  1530,  1531,  1534,  1536,  1537,  1543,  1548,
  1551,  1552,  1553,  1559,  1562,  1564,  1567,  1569,  1571,  1573,
  1574,  1580,  1581,  1590,  1591,  1600,  1603,  1606,  1607,  1610,
  1613,  1614,  1615,  1621,  1622,  1623
};

static const short yyrhs[] = {   114,
   113,     0,   216,   113,     0,   288,   113,     0,   294,   113,
     0,   302,   113,     0,     0,     3,    39,   116,   115,     4,
     0,     3,    39,   116,     1,     4,     0,   120,   115,     0,
   132,   115,     0,   135,   115,     0,   122,   115,     0,   128,
   115,     0,   138,   115,     0,   313,   115,     0,   225,   115,
     0,     0,     3,    40,   117,     4,     0,    19,     0,    20,
     0,    21,     0,    22,     0,    23,     0,    24,     0,    25,
     0,    26,     0,    29,     0,    20,     0,    24,     0,    19,
     0,    25,     0,     3,    34,   121,     4,     0,     3,    34,
     1,     4,     0,   121,    38,     0,   121,   106,     0,   121,
    27,     0,     0,     3,    36,   123,     4,     0,     3,    36,
     1,     4,     0,   124,   123,     0,   124,     0,     0,     3,
    19,   125,   126,     4,     0,     3,     1,     4,     0,   126,
   127,    14,    21,     0,   126,   127,     0,     0,   127,    28,
     0,   127,    24,     0,    28,     0,    24,     0,     3,    37,
   129,     4,     0,     3,    37,     1,     4,     0,   130,   129,
     0,   130,     0,     0,     3,    19,   131,   126,     4,     0,
     3,     1,     4,     0,     3,    38,   133,     4,     0,   133,
   134,    14,    21,     0,   133,   134,    14,    19,     0,   133,
   134,     0,     0,   134,    21,     0,   134,    19,     0,     0,
     3,    35,   136,     4,     0,     3,    67,   136,     4,     0,
   136,   137,    14,    21,     0,   136,   137,     0,     0,   137,
    19,     0,    19,     0,   139,     0,   239,     0,   242,     0,
     0,     3,    41,   119,   140,   141,     4,     0,     3,    41,
     1,     4,     0,     0,   141,    44,     3,   142,   126,     4,
     0,   141,    98,   143,     0,   141,    52,   178,     0,   141,
    45,   146,     0,   141,    46,   146,     0,   141,    56,   203,
     0,   141,   211,     0,   141,   105,    33,     0,     0,     0,
     3,    19,   144,   156,     4,     0,     0,     3,    30,   145,
   156,     4,     0,   148,     0,     3,    57,   147,     4,     0,
   148,   147,     0,   148,     0,   150,     0,   153,     0,   162,
     0,   157,     0,    49,     0,    50,     0,    51,     0,     0,
     3,    22,   151,   156,     4,     0,     3,    13,   118,   118,
     4,     0,     0,     3,   149,     3,    22,   152,   156,     4,
     4,     0,     3,   149,     3,    13,   118,   118,     4,     4,
     0,     0,     3,    62,     3,    22,   154,   156,     4,     4,
     0,     3,    62,     3,    13,   118,   118,     4,     4,     0,
     0,     3,   149,     3,    62,     3,    22,   155,   156,     4,
     4,     4,     0,     3,   149,     3,    62,     3,    13,   118,
   118,     4,     4,     4,     0,   156,    24,     0,   156,    20,
     0,     0,     3,   158,   159,   159,     4,     0,     3,   149,
     3,   158,   159,   159,     4,     4,     0,     7,     0,     8,
     0,     9,     0,    10,     0,    13,     0,     3,    14,   159,
     4,     0,     3,    15,   159,   159,     4,     0,     3,    14,
   159,   159,     4,     0,     3,    16,   159,   159,     4,     0,
     3,    17,   159,   159,     4,     0,   159,    17,   159,     0,
    32,     0,    31,     0,     3,    77,     4,     0,     3,   103,
    29,     4,     0,   160,     0,     3,    23,   161,     4,     0,
    23,     0,    24,   161,     0,    20,   161,     0,     0,     0,
     0,     3,    87,   163,   258,   261,     3,    22,   164,   156,
     4,     4,     0,     0,     0,     3,    60,     3,   165,   126,
     4,   166,   171,     4,     0,     0,     0,     3,   149,     3,
    87,   167,   258,   261,     3,    22,   168,   156,     4,     4,
     4,     0,     0,     0,     3,   149,     3,    60,     3,   169,
   126,     4,   170,   171,     4,     4,     0,     3,   172,   173,
     4,     0,   173,     0,    61,   264,     0,    61,     3,    57,
   263,     4,     0,     0,     3,    22,   174,   156,     4,     0,
     0,     3,    62,     3,    22,   175,   156,     4,     4,     0,
     0,     3,   149,     3,    22,   176,   156,     4,     4,     0,
     0,     3,   149,     3,    62,     3,    22,   177,   156,     4,
     4,     4,     0,   180,     0,     3,    57,   179,     4,     0,
   180,   179,     0,   180,     0,     0,     0,     3,    60,     3,
   181,   126,     4,   182,   187,     4,     0,     0,     3,    63,
   183,   188,   189,     4,     0,     0,     0,     3,   149,     3,
    60,     3,   184,   126,     4,   185,   187,     4,     4,     0,
     0,     3,   149,     3,    63,   186,   188,   189,     4,     0,
   189,     0,   196,     0,     3,    63,   188,   189,     4,     0,
   189,     0,   264,     0,     3,    57,   263,     4,     0,   190,
     0,   193,     0,     0,     3,    22,   191,   156,     4,     0,
     0,     3,   149,     3,    22,   192,   156,     4,     4,     0,
     0,     3,    62,     3,    22,   194,   156,     4,     4,     0,
     0,     3,   149,     3,    62,     3,    22,   195,   156,     4,
     4,     4,     0,     0,     3,    78,     3,    23,   197,   156,
     4,   159,     4,     0,     0,     3,    79,     3,    23,   198,
   156,     4,   159,     4,     0,     0,     3,    82,     3,    23,
   199,   156,     4,   159,     4,     0,     0,     3,   149,     3,
    78,     3,    23,   200,   156,     4,   159,     4,     4,     0,
     0,     3,   149,     3,    79,     3,    23,   201,   156,     4,
   159,     4,     4,     0,     0,     3,   149,     3,    82,     3,
    23,   202,   156,     4,   159,     4,     4,     0,   205,     0,
     3,    57,   204,     4,     0,   205,     0,   205,   204,     0,
   206,     0,   207,     0,   209,     0,     3,    13,    76,   159,
     4,     0,   159,     0,     3,   208,    76,   159,     4,     0,
     9,     0,    10,     0,     3,   210,    76,   159,     4,     0,
     7,     0,     8,     0,     0,     3,    99,   212,   259,   260,
   100,     3,   213,     4,     4,     0,   214,   213,     0,     0,
     0,     3,    25,   215,   156,     4,     0,     3,    39,   217,
   218,     4,     0,     3,    39,   217,     1,     4,     0,     3,
    65,   117,     4,     0,   218,     3,    66,   117,     4,     0,
   218,   120,     0,   218,   135,     0,   218,   219,     0,   218,
   225,     0,   218,   234,     0,   218,   237,     0,   218,   249,
     0,   218,   242,     0,   218,   313,     0,     0,     3,    68,
   220,     4,     0,   220,   223,     0,   220,   221,     0,     0,
     0,     3,    22,   222,   156,     4,     0,     0,     3,    13,
     3,    23,   224,   156,     4,   238,     4,     0,     3,    13,
    23,   238,     4,     0,     3,    69,   226,     4,     0,     3,
    69,     3,    57,   227,     4,     4,     0,     3,   106,   226,
     4,     0,     3,   106,     3,    57,   227,     4,     4,     0,
   231,     0,     3,   102,    19,   228,     4,     0,   226,   227,
     0,     0,   231,     0,     0,     3,    57,   229,   230,     4,
     0,   231,   230,     0,     0,     0,     3,    22,   232,   156,
     4,     0,     0,     3,    62,     3,    22,   233,   156,     4,
     4,     0,   157,     0,     3,   107,   228,     4,     0,     3,
   108,   228,     4,     0,     3,   109,   228,     4,     0,     3,
   110,   228,   228,     4,     0,     3,   111,   228,   228,     4,
     0,     3,   104,   238,   228,     4,     0,     3,   112,   238,
   228,   228,     4,     0,     3,    73,   235,   236,     4,     0,
     3,    73,     1,     4,     0,    74,     0,    75,     0,   159,
     0,     3,    70,    71,    32,     4,     0,     3,    70,    72,
    32,     4,     0,    32,     0,    31,     0,    96,     0,     0,
     0,     3,    55,   240,   252,   241,    86,     3,   279,    32,
   280,     4,     4,     0,     3,    55,     1,     4,     0,     0,
     3,    92,   243,   252,   244,     0,     3,    92,     1,     4,
     0,   245,     0,   247,     0,     0,    41,     3,    25,   246,
   156,     4,     4,     0,     0,    97,     3,    22,   248,   156,
     4,     4,     0,     0,     0,     3,    55,   250,   252,   251,
    86,     3,   279,    32,   280,     4,     4,     0,     3,    55,
     1,     4,     0,   252,   253,     0,   252,   254,     0,   252,
   255,     0,   252,   257,     0,     0,    83,   117,     0,    84,
    19,     0,    84,    26,     0,     0,    85,     3,   256,   126,
     4,     0,    91,   264,     0,    91,     3,    57,   263,     4,
     0,    85,     3,   126,     4,     0,   258,     0,     0,    91,
   264,   262,     0,    91,     3,    57,   263,     4,   262,     0,
    45,   264,   261,     0,    45,     3,    57,   263,     4,   261,
     0,    46,   264,   261,     0,    46,     3,    57,   263,     4,
   261,     0,     0,    91,   264,     0,    91,     3,    57,   263,
     4,     0,     0,    45,   264,     0,    45,     3,    57,   263,
     4,     0,    46,   264,     0,    46,     3,    57,   263,     4,
     0,     0,   264,   263,     0,   264,     0,   265,     0,   268,
     0,   272,     0,   275,     0,   278,     0,     0,     3,    22,
   266,   156,     4,     0,     0,     3,    68,     3,    22,   267,
   156,     4,     4,     0,     3,    13,   118,   118,     4,     0,
     0,     3,    62,     3,    22,   269,   156,     4,     4,     0,
     0,     3,    68,     3,    62,     3,    22,   270,   156,     4,
     4,     4,     0,     0,     3,    62,     3,    68,     3,    22,
   271,   156,     4,     4,     4,     0,     3,    62,     3,    13,
   118,   118,     4,     4,     0,     0,     3,    69,     3,    22,
   273,   156,     4,     4,     0,     0,     3,    69,     3,    62,
     3,    22,   274,   156,     4,     4,     4,     0,     0,     3,
    62,     3,    69,     3,    22,   276,   156,     4,     4,     4,
     0,     0,     3,    62,     3,    69,     3,    62,     3,    22,
   277,   156,     4,     4,     4,     4,     0,     3,   101,    24,
    21,     4,     0,    88,     0,    89,     0,    90,     0,   280,
   281,     0,   280,   284,     0,     0,     0,     3,    22,   282,
   156,     4,     0,     0,     3,    62,     3,    22,   283,   156,
     4,     4,     0,     0,     0,     0,     3,    87,    85,     3,
   285,   126,     4,   286,   261,     3,    22,   287,   156,     4,
     4,     0,     0,     3,    93,   289,   290,     4,     0,     3,
    93,     1,     4,     0,   291,   290,     0,    95,   290,     0,
   292,   290,     0,     0,    84,   117,     0,     0,   238,    11,
     3,    25,   293,   156,     4,     0,   295,     0,   299,     0,
   296,   295,     0,   296,     0,     0,   238,    11,     3,    25,
   297,   156,     4,   298,     0,     5,   238,     6,     0,     0,
   300,   299,     0,   300,     0,     0,     3,    25,   301,   156,
     4,     0,     3,    94,   303,     4,     0,   303,   304,     0,
     0,     0,     3,   305,   307,     4,   306,     0,    95,   238,
     0,   238,     0,   307,   308,     0,   308,     0,   309,     0,
   311,     0,     0,     3,    22,   310,   156,     4,     0,     0,
     3,    62,     3,    22,   312,   156,     4,     4,     0,     0,
     3,    98,   314,   315,   259,   261,   316,     4,     0,    84,
    19,     0,    84,    26,     0,     0,   316,   319,     0,   316,
   317,     0,     0,     0,     3,   117,   318,   156,     4,     0,
     0,     0,     3,    87,   320,   258,   261,     3,   117,   321,
   156,     4,     4,     0
};

#endif

#if YY_PDDL_Parser_DEBUG != 0
static const short yyrline[] = { 0,
    95,    97,    98,    99,   100,   101,   104,   106,   113,   115,
   116,   117,   118,   119,   120,   121,   122,   125,   133,   135,
   136,   137,   138,   139,   140,   141,   142,   145,   147,   150,
   152,   157,   159,   166,   168,   169,   170,   175,   177,   184,
   186,   189,   194,   204,   211,   216,   220,   223,   237,   244,
   257,   268,   270,   277,   279,   282,   287,   297,   306,   310,
   315,   325,   329,   332,   337,   343,   348,   350,   353,   358,
   362,   365,   373,   384,   386,   387,   392,   397,   406,   413,
   418,   424,   425,   426,   427,   428,   429,   430,   435,   438,
   447,   451,   456,   462,   464,   467,   469,   472,   474,   475,
   476,   483,   488,   492,   498,   503,   510,   520,   524,   531,
   543,   548,   555,   565,   569,   576,   589,   600,   607,   610,
   615,   622,   627,   631,   635,   639,   645,   650,   654,   658,
   662,   666,   670,   674,   678,   682,   686,   692,   697,   703,
   708,   712,   718,   724,   728,   739,   744,   752,   758,   763,
   767,   779,   784,   792,   802,   805,   808,   810,   813,   818,
   826,   830,   839,   843,   851,   855,   866,   868,   871,   873,
   876,   882,   890,   899,   903,   910,   915,   923,   933,   937,
   945,   946,   949,   951,   954,   956,   959,   961,   964,   970,
   981,   986,   999,  1005,  1017,  1022,  1036,  1041,  1050,  1054,
  1063,  1067,  1076,  1080,  1089,  1093,  1102,  1106,  1117,  1119,
  1122,  1124,  1127,  1129,  1130,  1133,  1139,  1146,  1153,  1155,
  1158,  1165,  1167,  1170,  1180,  1200,  1202,  1205,  1210,  1224,
  1226,  1233,  1241,  1243,  1244,  1245,  1246,  1247,  1248,  1249,
  1250,  1251,  1252,  1255,  1259,  1261,  1262,  1265,  1270,  1283,
  1288,  1301,  1317,  1319,  1320,  1321,  1324,  1330,  1339,  1341,
  1344,  1349,  1353,  1361,  1370,  1373,  1378,  1384,  1388,  1394,
  1398,  1402,  1406,  1410,  1414,  1418,  1422,  1458,  1460,  1467,
  1477,  1488,  1503,  1508,  1514,  1516,  1517,  1522,  1529,  1532,
  1541,  1548,  1555,  1556,  1563,  1565,  1568,  1573,  1587,  1592,
  1606,  1614,  1617,  1625,  1632,  1634,  1635,  1636,  1637,  1640,
  1647,  1654,  1661,  1666,  1674,  1676,  1679,  1689,  1691,  1694,
  1696,  1697,  1698,  1699,  1700,  1701,  1704,  1706,  1707,  1710,
  1712,  1713,  1714,  1715,  1718,  1720,  1723,  1725,  1726,  1727,
  1728,  1731,  1736,  1742,  1746,  1752,  1763,  1768,  1774,  1778,
  1784,  1788,  1794,  1805,  1810,  1816,  1820,  1828,  1833,  1839,
  1843,  1851,  1860,  1865,  1869,  1875,  1877,  1878,  1880,  1885,
  1892,  1896,  1905,  1911,  1919,  1923,  1936,  1941,  1945,  1952,
  1954,  1958,  1959,  1962,  1969,  1974,  1985,  1991,  1998,  2000,
  2003,  2008,  2038,  2040,  2043,  2045,  2048,  2053,  2084,  2088,
  2090,  2093,  2098,  2105,  2111,  2117,  2119,  2122,  2124,  2127,
  2133,  2141,  2147,  2155,  2164,  2171,  2177,  2181,  2184,  2186,
  2187,  2190,  2200,  2208,  2215,  2224
};

static const char * const yytname[] = {   "$","error","$illegal.","TK_OPEN",
"TK_CLOSE","TK_OPEN_SQ","TK_CLOSE_SQ","TK_GREATER","TK_GREATEQ","TK_LESS","TK_LESSEQ",
"TK_COLON","TK_HASHT","TK_EQ","TK_HYPHEN","TK_PLUS","TK_MUL","TK_DIV","TK_UMINUS",
"TK_NEW_SYMBOL","TK_OBJ_SYMBOL","TK_TYPE_SYMBOL","TK_PRED_SYMBOL","TK_FUN_SYMBOL",
"TK_VAR_SYMBOL","TK_ACTION_SYMBOL","TK_MISC_SYMBOL","TK_KEYWORD","TK_NEW_VAR_SYMBOL",
"TK_PREFERENCE_SYMBOL","TK_SET_SYMBOL","TK_FLOAT","TK_INT","TK_STRING","KW_REQS",
"KW_CONSTANTS","KW_PREDS","KW_FUNS","KW_TYPES","KW_DEFINE","KW_DOMAIN","KW_ACTION",
"KW_PROCESS","KW_EVENT","KW_ARGS","KW_PRE","KW_COND","KW_START_PRE","KW_END_PRE",
"KW_AT_START","KW_AT_END","KW_OVER_ALL","KW_EFFECT","KW_INITIAL_EFFECT","KW_FINAL_EFFECT",
"KW_INVARIANT","KW_DURATION","KW_AND","KW_OR","KW_EXISTS","KW_FORALL","KW_IMPLY",
"KW_NOT","KW_WHEN","KW_EITHER","KW_PROBLEM","KW_FORDOMAIN","KW_OBJECTS","KW_INIT",
"KW_GOAL","KW_LENGTH","KW_SERIAL","KW_PARALLEL","KW_METRIC","KW_MINIMIZE","KW_MAXIMIZE",
"KW_DURATION_VAR","KW_TOTAL_TIME","KW_INCREASE","KW_DECREASE","KW_SCALE_UP",
"KW_SCALE_DOWN","KW_ASSIGN","KW_TAG","KW_NAME","KW_VARS","KW_SET_CONSTRAINT",
"KW_SETOF","KW_AT_LEAST_N","KW_AT_MOST_N","KW_EXACTLY_N","KW_CONTEXT","KW_IRRELEVANT",
"KW_PLAN","KW_HEURISTIC","KW_OPT","KW_INF","KW_FACT","KW_SET","KW_EXPANSION",
"KW_TASKS","KW_TYPEOF","KW_PREFERENCE","KW_VIOLATED","KW_WITHIN","KW_ASSOC",
"KW_CONSTRAINTS","KW_ALWAYS","KW_SOMETIME","KW_AT_MOST_ONCE","KW_SOMETIME_BEFORE",
"KW_SOMETIME_AFTER","KW_ALWAYS_WITHIN","pddl_declarations","pddl_domain","domain_elements",
"domain_name","any_symbol","arg_symbol","action_symbol","domain_requires","require_list",
"domain_predicates","predicate_list","predicate_decl","@1","typed_param_list",
"typed_param_sym_list","domain_functions","function_list","function_decl","@2",
"domain_types","typed_type_list","primitive_type_list","domain_constants","typed_constant_list",
"ne_constant_sym_list","domain_structure","action_declaration","@3","action_elements",
"@4","action_set_name","@5","@6","action_condition","action_condition_list",
"single_action_condition","timing_keyword","positive_atom_condition","@7","@8",
"negative_atom_condition","@9","@10","atom_argument_list","numeric_condition",
"relation_keyword","d_expression","d_function","d_argument_list","set_condition",
"@11","@12","@13","@14","@15","@16","@17","@18","universal_condition_body","universal_condition_antecedent",
"universal_condition_atom","@19","@20","@21","@22","action_effect","action_effect_list",
"single_action_effect","@23","@24","@25","@26","@27","@28","quantified_effect_body",
"effect_conditions","atomic_effect","positive_atom_effect","@29","@30","negative_atom_effect",
"@31","@32","numeric_effect","@33","@34","@35","@36","@37","@38","action_duration",
"action_duration_list","action_duration_exp","action_exact_duration","action_min_duration",
"less_or_lesseq","action_max_duration","greater_or_greatereq","action_expansion",
"@39","task_list","task","@40","pddl_problem","problem_name","problem_elements",
"initial_state","init_elements","init_atom","@41","init_function","@42","goal_spec",
"single_goal_spec","goal_spec_list","new_goal","@43","new_goal_list","new_single_goal",
"@44","@45","metric_spec","metric_keyword","metric_expression","length_spec",
"numeric_value","domain_invariant","@46","@47","irrelevant_item","@48","irrelevant_item_content",
"irrelevant_action","@49","irrelevant_atom","@50","problem_invariant","@51",
"@52","dkel_element_list","dkel_tag_spec","dkel_name_spec","dkel_vars_spec",
"@53","dkel_context_spec","req_vars_spec","opt_vars_spec","opt_context_and_precondition_spec",
"opt_context_spec","opt_precondition_spec","context_list","context_atom","positive_context_atom",
"@54","@55","negative_context_atom","@56","@57","@58","positive_context_goal_atom",
"@59","@60","negative_context_goal_atom","@61","@62","type_constraint_atom",
"set_constraint_type","invariant_set","invariant_atom","@63","@64","invariant_set_of_atoms",
"@65","@66","@67","pddl_plan","@68","plan_elements","plan_name","plan_step",
"@69","ipc_plan","ipc_plan_step_list","ipc_plan_step","@70","opt_step_duration",
"ipc_plan_step_seq","simple_plan_step","@71","heuristic_table","table_entry_list",
"table_entry","@72","entry_value","ne_entry_atom_list","entry_atom","pos_entry_atom",
"@73","neg_entry_atom","@74","reference_set","@75","opt_set_name","reference_list",
"reference","@76","simple_reference_set","@77","@78",""
};
#endif

static const short yyr1[] = {     0,
   113,   113,   113,   113,   113,   113,   114,   114,   115,   115,
   115,   115,   115,   115,   115,   115,   115,   116,   117,   117,
   117,   117,   117,   117,   117,   117,   117,   118,   118,   119,
   119,   120,   120,   121,   121,   121,   121,   122,   122,   123,
   123,   125,   124,   124,   126,   126,   126,   127,   127,   127,
   127,   128,   128,   129,   129,   131,   130,   130,   132,   133,
   133,   133,   133,   134,   134,   134,   135,   135,   136,   136,
   136,   137,   137,   138,   138,   138,   140,   139,   139,   142,
   141,   141,   141,   141,   141,   141,   141,   141,   141,   144,
   143,   145,   143,   146,   146,   147,   147,   148,   148,   148,
   148,   149,   149,   149,   151,   150,   150,   152,   150,   150,
   154,   153,   153,   155,   153,   153,   156,   156,   156,   157,
   157,   158,   158,   158,   158,   158,   159,   159,   159,   159,
   159,   159,   159,   159,   159,   159,   159,   160,   160,   161,
   161,   161,   163,   164,   162,   165,   166,   162,   167,   168,
   162,   169,   170,   162,   171,   171,   172,   172,   174,   173,
   175,   173,   176,   173,   177,   173,   178,   178,   179,   179,
   181,   182,   180,   183,   180,   184,   185,   180,   186,   180,
   180,   180,   187,   187,   188,   188,   189,   189,   191,   190,
   192,   190,   194,   193,   195,   193,   197,   196,   198,   196,
   199,   196,   200,   196,   201,   196,   202,   196,   203,   203,
   204,   204,   205,   205,   205,   206,   206,   207,   208,   208,
   209,   210,   210,   212,   211,   213,   213,   215,   214,   216,
   216,   217,   218,   218,   218,   218,   218,   218,   218,   218,
   218,   218,   218,   219,   220,   220,   220,   222,   221,   224,
   223,   223,   225,   225,   225,   225,   226,   226,   227,   227,
   228,   229,   228,   230,   230,   232,   231,   233,   231,   231,
   231,   231,   231,   231,   231,   231,   231,   234,   234,   235,
   235,   236,   237,   237,   238,   238,   238,   240,   241,   239,
   239,   243,   242,   242,   244,   244,   246,   245,   248,   247,
   250,   251,   249,   249,   252,   252,   252,   252,   252,   253,
   254,   254,   256,   255,   257,   257,   258,   259,   259,   260,
   260,   260,   260,   260,   260,   260,   261,   261,   261,   262,
   262,   262,   262,   262,   263,   263,   264,   264,   264,   264,
   264,   266,   265,   267,   265,   265,   269,   268,   270,   268,
   271,   268,   268,   273,   272,   274,   272,   276,   275,   277,
   275,   278,   279,   279,   279,   280,   280,   280,   282,   281,
   283,   281,   285,   286,   287,   284,   289,   288,   288,   290,
   290,   290,   290,   291,   293,   292,   294,   294,   295,   295,
   297,   296,   298,   298,   299,   299,   301,   300,   302,   303,
   303,   305,   304,   306,   306,   307,   307,   308,   308,   310,
   309,   312,   311,   314,   313,   315,   315,   315,   316,   316,
   316,   318,   317,   320,   321,   319
};

static const short yyr2[] = {     0,
     2,     2,     2,     2,     2,     0,     5,     5,     2,     2,
     2,     2,     2,     2,     2,     2,     0,     4,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     4,     4,     2,     2,     2,     0,     4,     4,     2,
     1,     0,     5,     3,     4,     2,     0,     2,     2,     1,
     1,     4,     4,     2,     1,     0,     5,     3,     4,     4,
     4,     2,     0,     2,     2,     0,     4,     4,     4,     2,
     0,     2,     1,     1,     1,     1,     0,     6,     4,     0,
     6,     3,     3,     3,     3,     3,     2,     3,     0,     0,
     5,     0,     5,     1,     4,     2,     1,     1,     1,     1,
     1,     1,     1,     1,     0,     5,     5,     0,     8,     8,
     0,     8,     8,     0,    11,    11,     2,     2,     0,     5,
     8,     1,     1,     1,     1,     1,     4,     5,     5,     5,
     5,     3,     1,     1,     3,     4,     1,     4,     1,     2,
     2,     0,     0,     0,    11,     0,     0,     9,     0,     0,
    14,     0,     0,    12,     4,     1,     2,     5,     0,     5,
     0,     8,     0,     8,     0,    11,     1,     4,     2,     1,
     0,     0,     9,     0,     6,     0,     0,    12,     0,     8,
     1,     1,     5,     1,     1,     4,     1,     1,     0,     5,
     0,     8,     0,     8,     0,    11,     0,     9,     0,     9,
     0,     9,     0,    12,     0,    12,     0,    12,     1,     4,
     1,     2,     1,     1,     1,     5,     1,     5,     1,     1,
     5,     1,     1,     0,    10,     2,     0,     0,     5,     5,
     5,     4,     5,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     0,     4,     2,     2,     0,     0,     5,     0,
     9,     5,     4,     7,     4,     7,     1,     5,     2,     0,
     1,     0,     5,     2,     0,     0,     5,     0,     8,     1,
     4,     4,     4,     5,     5,     5,     6,     5,     4,     1,
     1,     1,     5,     5,     1,     1,     1,     0,     0,    12,
     4,     0,     5,     4,     1,     1,     0,     7,     0,     7,
     0,     0,    12,     4,     2,     2,     2,     2,     0,     2,
     2,     2,     0,     5,     2,     5,     4,     1,     0,     3,
     6,     3,     6,     3,     6,     0,     2,     5,     0,     2,
     5,     2,     5,     0,     2,     1,     1,     1,     1,     1,
     1,     0,     5,     0,     8,     5,     0,     8,     0,    11,
     0,    11,     8,     0,     8,     0,    11,     0,    11,     0,
    14,     5,     1,     1,     1,     2,     2,     0,     0,     5,
     0,     8,     0,     0,     0,    15,     0,     5,     4,     2,
     2,     2,     0,     2,     0,     7,     1,     1,     2,     1,
     0,     8,     3,     0,     2,     1,     0,     5,     4,     2,
     0,     0,     5,     2,     1,     2,     1,     1,     1,     0,
     5,     0,     8,     0,     8,     2,     2,     0,     2,     2,
     0,     0,     5,     0,     0,    11
};

static const short yydefact[] = {     6,
     0,   286,   285,   287,     6,     6,     0,     6,     6,   387,
   390,   388,   396,     6,   397,     0,     0,   401,     1,     2,
     0,     3,     4,   389,     0,   395,     5,   119,     0,     0,
     0,     0,   383,     0,     0,     0,     0,     0,     0,     0,
     0,    17,    17,    17,    17,    17,    17,    74,    17,    75,
    76,    17,     0,     0,   379,     0,   383,     0,     0,   383,
   383,   402,   399,   400,   391,   398,   118,   117,    19,    20,
    21,    22,    23,    24,    25,    26,    27,     0,     0,     8,
     0,    71,     0,     0,    63,     0,     0,    71,     0,     0,
   414,     0,     7,     9,    12,    13,    10,    11,    14,    16,
    15,   231,     0,   230,   234,   235,   236,   237,   238,   239,
   241,   240,   242,   384,   381,     0,   378,   380,   382,     0,
   119,    18,   232,     0,     0,     0,     0,     0,     0,    41,
     0,     0,     0,    55,    66,     0,    30,    31,    77,     0,
   309,     0,     0,   270,     0,   257,     0,   309,   418,     0,
     0,     0,     0,   247,     0,     0,     0,     0,     0,   407,
   408,   409,     0,    33,    32,    36,    34,    35,    67,    73,
    70,    39,     0,    42,    38,    40,    53,     0,    56,    52,
    54,    59,    62,    79,    89,   291,   289,    68,   122,   123,
   124,   125,   126,   266,   102,   103,   104,   260,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   253,
   294,     0,     0,   319,   260,   255,     0,   309,     0,     0,
     0,     0,     0,   280,   281,     0,   385,   410,     0,     0,
   406,   394,     0,    72,    44,    47,    58,    47,     0,    65,
    64,     0,     0,     0,     0,     0,     0,   305,   306,   307,
   308,   119,     0,   260,     0,     0,     0,     0,     0,     0,
   261,     0,     0,     0,     0,     0,     0,     0,   139,   134,
   133,     0,   137,     0,     0,   293,   295,   296,   416,   417,
     0,   318,   329,     0,   304,   302,   233,     0,   244,   246,
   245,     0,     0,   279,   282,     0,   119,   119,     0,     0,
   405,   403,     0,   392,    69,     0,     0,    61,    60,     0,
    78,     0,     0,     0,     0,     0,     0,     0,    87,   310,
   311,   312,   313,     0,   315,   337,   338,   339,   340,   341,
     0,     0,   259,     0,   268,     0,     0,   262,   271,   272,
   273,     0,     0,     0,     0,     0,     0,     0,     0,   142,
     0,     0,     0,     0,     0,     0,    47,     0,   421,     0,
     0,     0,   248,   283,   284,   278,     0,     0,   412,   404,
     0,    43,    51,    50,    46,    57,   224,    80,     0,    84,
    94,    98,    99,   101,   100,    85,     0,    83,   167,   181,
   187,   188,   182,     0,   217,    86,   209,   213,   214,   215,
     0,    82,    88,    47,     0,   342,     0,     0,     0,     0,
     0,     0,   267,   254,   119,   258,   276,   265,   274,   275,
     0,     0,     0,     0,     0,     0,   142,   142,     0,   135,
     0,   132,   120,   297,   299,     0,     0,   327,     0,   256,
     0,     0,     0,   119,   386,   411,   119,   393,     0,    49,
    48,   319,    47,   126,   105,     0,     0,     0,   143,     0,
   189,     0,     0,     0,   174,     0,     0,     0,     0,   222,
   223,   219,   220,     0,     0,     0,     0,    90,    92,     0,
    28,    29,     0,   119,     0,     0,   336,     0,     0,     0,
     0,   363,   364,   365,     0,     0,     0,     0,   265,   277,
     0,   127,     0,     0,     0,     0,   141,   140,   138,   136,
   119,   119,   317,     0,     0,   415,   420,   419,     0,   250,
     0,     0,     0,    45,   326,     0,     0,   119,     0,     0,
    97,   146,     0,     0,     0,   119,     0,     0,   170,   171,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   211,
     0,     0,   119,   119,   314,     0,     0,   316,   335,     0,
   347,     0,     0,   344,     0,   354,     0,     0,   368,     0,
   263,   264,     0,   129,   128,   130,   131,     0,     0,     0,
   424,   422,     0,   119,   252,   249,     0,     0,     0,     0,
     0,    81,     0,     0,    95,    96,    47,     0,   111,   329,
   126,   108,     0,     0,   149,     0,   168,   169,    47,   193,
     0,     0,   185,   197,   199,   201,   191,     0,     0,   179,
     0,     0,     0,     0,   210,   212,     0,     0,     0,     0,
   346,   343,     0,   119,     0,     0,   119,     0,   119,     0,
   362,     0,   269,   121,     0,     0,   328,     0,   119,   368,
     0,   413,     0,   329,     0,   329,     0,   334,     0,   107,
   106,     0,     0,   119,     0,     0,   119,   152,     0,     0,
   190,     0,   119,     0,     0,     0,   119,   119,   119,   119,
   176,     0,     0,     0,     0,     0,   216,   218,   221,    91,
    93,     0,     0,   351,   358,     0,     0,   349,     0,   356,
     0,     0,   366,   367,   298,   300,   329,     0,     0,     0,
     0,   322,     0,   324,     0,     0,     0,   320,   227,   147,
     0,     0,     0,     0,     0,    47,     0,   114,   329,   172,
     0,     0,     0,   175,     0,     0,     0,     0,    47,   195,
     0,   203,   205,   207,     0,     0,   119,   119,     0,     0,
   119,     0,   119,   369,     0,     0,   290,     0,   423,     0,
     0,     0,     0,     0,     0,   330,     0,   332,     0,     0,
   227,     0,     0,     0,   144,     0,     0,     0,     0,   119,
     0,     0,     0,   186,     0,     0,     0,     0,     0,     0,
   119,     0,   119,   119,   119,   353,   348,     0,     0,   360,
   345,     0,   355,     0,   119,     0,     0,     0,   303,   251,
   329,   329,   334,     0,     0,   228,     0,   226,     0,     0,
   156,   113,   112,   119,   110,   109,   153,     0,     0,     0,
     0,     0,   184,   194,     0,     0,     0,   192,   177,     0,
   180,     0,     0,     0,     0,     0,   119,     0,     0,     0,
   371,   373,   425,   323,   325,   321,     0,     0,   119,   225,
   159,     0,     0,     0,     0,   148,     0,     0,     0,     0,
   150,     0,   173,   198,   200,   202,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   370,   119,    47,   119,
   331,   333,     0,   119,     0,   157,     0,     0,     0,     0,
     0,     0,     0,     0,   119,     0,     0,     0,     0,     0,
     0,   352,   359,     0,   350,   357,     0,     0,     0,   229,
     0,     0,   161,   163,     0,   155,   145,     0,   116,   115,
     0,     0,     0,   196,     0,     0,     0,     0,     0,   374,
     0,   160,     0,   119,   119,     0,   154,     0,   183,   178,
   204,   206,   208,     0,   372,   329,   426,   158,     0,     0,
   165,     0,   361,     0,     0,     0,   119,   151,     0,   162,
   164,     0,   375,     0,   119,     0,     0,   166,     0,   376,
     0,     0,     0
};

static const short yydefgoto[] = {    19,
     5,    41,    30,    78,   483,   139,    42,   125,    43,   129,
   130,   236,   306,   375,    44,   133,   134,   238,    45,   135,
   183,    46,   126,   171,    47,    48,   185,   242,   453,   402,
   553,   554,   380,   530,   381,   208,   382,   528,   667,   383,
   664,   780,    36,   144,   209,   395,   273,   429,   385,   534,
   824,   597,   772,   670,   905,   726,   868,   820,   865,   821,
   894,   944,   945,   967,   388,   538,   539,   609,   782,   542,
   739,   877,   683,   832,   612,   390,   391,   536,   680,   392,
   673,   791,   393,   677,   678,   679,   793,   794,   795,   396,
   549,   550,   398,   399,   476,   400,   477,   319,   452,   770,
   771,   859,     6,    31,    54,   107,   220,   290,   444,   291,
   584,    49,   254,   255,   260,   418,   498,   261,   252,   415,
   109,   226,   296,   110,     7,    50,   141,   247,    51,   148,
   276,   277,   511,   278,   512,   112,   218,   361,   187,   248,
   249,   250,   404,   251,   282,   283,   591,   359,   718,   486,
   487,   326,   484,   637,   327,   634,   751,   747,   328,   639,
   753,   329,   748,   847,   330,   495,   642,   703,   805,   888,
   704,   889,   956,   975,     8,    33,    59,    60,    61,   297,
     9,    10,    11,   121,   304,    12,    13,    28,    14,    34,
    64,   120,   302,   159,   160,   161,   298,   162,   447,    52,
   149,   214,   439,   517,   649,   518,   648,   890
};

static const short yypact[] = {    31,
    46,-32768,-32768,-32768,    31,    31,    24,    31,    31,-32768,
    -2,-32768,    84,    31,-32768,   147,    42,-32768,-32768,-32768,
   235,-32768,-32768,-32768,    25,-32768,-32768,-32768,    13,   638,
   733,   204,   192,    54,   236,   300,   968,   968,   292,   599,
   328,   334,   334,   334,   334,   334,   334,-32768,   334,-32768,
-32768,   334,   348,   457,-32768,   968,   192,   273,   375,   192,
   192,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   389,   421,-32768,
    17,-32768,   182,   597,-32768,   169,   168,-32768,   365,    74,
-32768,   434,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   721,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   443,-32768,-32768,-32768,   476,
-32768,-32768,-32768,   484,    22,    76,   492,    51,   526,   538,
   544,   226,   570,   583,   595,   604,-32768,-32768,-32768,   617,
-32768,   195,   362,-32768,   620,-32768,   625,-32768,   572,   485,
   663,   179,   968,-32768,   496,   107,   653,     5,   577,-32768,
-32768,-32768,   480,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   332,-32768,   676,-32768,-32768,-32768,-32768,   678,-32768,-32768,
-32768,-32768,   669,-32768,-32768,-32768,   695,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   686,   689,   683,
    -2,   691,   691,   691,   691,   691,    -2,   701,   181,-32768,
-32768,   351,    35,   621,   686,-32768,   757,-32768,   760,   644,
   680,   740,   779,-32768,-32768,   181,-32768,-32768,   789,    52,
-32768,   788,   793,-32768,-32768,-32768,-32768,-32768,   590,-32768,
-32768,    20,   968,   402,   817,   823,   744,-32768,-32768,-32768,
-32768,-32768,   553,   686,   831,   821,   691,   691,   636,   858,
-32768,   863,   871,   691,   691,   691,  1002,   263,-32768,-32768,
-32768,   436,-32768,   877,   883,-32768,-32768,-32768,-32768,-32768,
   896,-32768,   797,   890,-32768,   695,-32768,   245,-32768,-32768,
-32768,   903,   908,-32768,   901,   916,-32768,-32768,   917,    -2,
-32768,-32768,    -2,-32768,-32768,   205,   211,-32768,-32768,   832,
-32768,   941,   947,   947,   949,   364,   958,   893,-32768,-32768,
-32768,-32768,-32768,    29,-32768,-32768,-32768,-32768,-32768,-32768,
   960,   513,-32768,   954,-32768,   963,   967,-32768,-32768,-32768,
-32768,   969,   973,   691,   181,   181,   181,   181,   181,   545,
   978,   966,   181,   227,   951,   961,-32768,   981,-32768,   992,
   920,   193,-32768,-32768,-32768,-32768,   520,   525,-32768,-32768,
   979,-32768,-32768,-32768,   430,-32768,-32768,-32768,   563,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   703,-32768,-32768,-32768,
-32768,-32768,-32768,   233,   901,-32768,-32768,-32768,-32768,-32768,
   167,-32768,-32768,-32768,   581,-32768,  1001,  1004,  1010,  1011,
   993,   465,-32768,-32768,-32768,-32768,-32768,  1013,-32768,-32768,
  1014,   436,   697,   436,   436,   436,   545,   545,  1015,-32768,
  1016,-32768,-32768,-32768,-32768,   269,   244,-32768,   648,-32768,
  1018,   999,    -2,-32768,-32768,-32768,-32768,-32768,  1003,-32768,
-32768,   621,-32768,   581,-32768,  1020,  1022,  1023,-32768,  1024,
-32768,  1025,  1026,  1027,-32768,  1028,  1029,  1030,  1031,-32768,
-32768,-32768,-32768,   959,   377,   962,   964,-32768,-32768,   279,
-32768,-32768,   581,-32768,    34,  1032,  1001,   142,    26,    47,
  1021,-32768,-32768,-32768,  1005,   675,   700,  1035,  1013,-32768,
   251,-32768,   281,   288,   318,   321,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,  1001,   777,-32768,-32768,-32768,   465,-32768,
  1037,   707,   715,-32768,    -9,   291,   581,-32768,   708,  1039,
  1020,-32768,   247,   621,   609,-32768,   528,  1040,  1025,-32768,
  1033,  1042,  1034,  1036,  1038,   755,   181,    97,  1043,   377,
   181,   181,-32768,-32768,-32768,  1044,   801,-32768,-32768,   581,
-32768,  1046,  1047,-32768,  1048,-32768,  1049,  1050,-32768,  1052,
-32768,-32768,  1054,-32768,-32768,-32768,-32768,   812,   818,  1056,
-32768,-32768,  1041,-32768,-32768,-32768,  1058,  1060,  1061,  1062,
   946,-32768,  1063,   824,-32768,-32768,-32768,   581,-32768,   797,
   581,-32768,  1065,  1066,-32768,   825,-32768,-32768,-32768,-32768,
   259,  1067,-32768,-32768,-32768,-32768,-32768,  1068,  1069,-32768,
  1071,  1072,  1073,   340,-32768,-32768,   381,   399,   827,   835,
-32768,-32768,   581,-32768,  1055,    55,-32768,  1057,-32768,  1059,
-32768,   720,-32768,-32768,  1074,  1076,-32768,   621,-32768,-32768,
   836,-32768,   296,   797,   337,   797,   361,   687,  1079,-32768,
-32768,   358,   581,-32768,  1080,   581,-32768,-32768,   278,   621,
-32768,   398,-32768,  1001,   140,  1081,-32768,-32768,-32768,-32768,
-32768,  1064,  1042,  1070,  1075,  1077,-32768,-32768,-32768,-32768,
-32768,  1083,   837,-32768,-32768,  1085,   846,-32768,   848,-32768,
   111,  1086,-32768,-32768,-32768,-32768,   797,   849,   738,    -2,
  1001,-32768,  1001,-32768,  1001,  1088,  1089,-32768,  1091,-32768,
  1092,   854,  1082,  1093,   859,-32768,   581,-32768,   797,-32768,
   861,  1095,  1098,-32768,   867,   872,   873,   878,-32768,-32768,
  1067,-32768,-32768,-32768,  1099,  1101,-32768,-32768,  1084,  1103,
-32768,  1104,-32768,-32768,  1106,  1017,-32768,  1107,-32768,  1108,
  1109,  1110,  1111,  1112,   388,-32768,   418,-32768,  1094,  1113,
  1091,  1115,  1116,  1117,-32768,  1118,  1119,   423,   581,-32768,
  1121,  1122,  1123,-32768,   130,   181,   181,   181,  1124,   473,
-32768,  1125,-32768,-32768,-32768,-32768,-32768,   880,   885,-32768,
-32768,   886,-32768,   891,-32768,  1114,  1127,   968,-32768,-32768,
   797,   797,   687,  1001,  1001,-32768,  1128,-32768,   327,  1129,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,  1130,   897,  1120,
   459,  1131,-32768,-32768,   400,   448,   495,-32768,-32768,   899,
-32768,   904,   909,   910,  1133,  1134,-32768,  1135,  1136,   912,
-32768,-32768,-32768,-32768,-32768,-32768,  1137,  1139,-32768,-32768,
-32768,  1141,  1142,  1143,  1144,-32768,   918,  1115,  1145,  1146,
-32768,  1042,-32768,-32768,-32768,-32768,  1122,  1147,   181,   181,
   181,  1148,  1149,   921,  1150,  1151,-32768,-32768,-32768,-32768,
-32768,-32768,   923,-32768,   463,-32768,  1126,   145,   280,  1152,
  1153,  1154,  1155,  1156,-32768,  1067,  1157,  1158,   510,   522,
   534,-32768,-32768,  1159,-32768,-32768,   931,   487,   933,-32768,
   936,  1001,-32768,-32768,  1161,-32768,-32768,  1162,-32768,-32768,
   942,  1163,  1164,-32768,  1165,  1166,  1167,  1168,  1169,-32768,
  1170,-32768,  1171,-32768,-32768,  1160,-32768,  1172,-32768,-32768,
-32768,-32768,-32768,  1173,-32768,   797,-32768,-32768,   944,   945,
-32768,  1174,-32768,  1176,  1177,  1179,-32768,-32768,  1175,-32768,
-32768,   950,-32768,  1180,-32768,  1181,   955,-32768,  1182,-32768,
  1053,  1187,-32768
};

static const short yypgoto[] = {   972,
-32768,   956,-32768,   -37,  -438,-32768,  1012,-32768,-32768,   965,
-32768,-32768,  -236,-32768,-32768,   977,-32768,-32768,-32768,-32768,
-32768,  1138,   996,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   775,   600,  -441,  -376,-32768,-32768,-32768,-32768,
-32768,-32768,  -121,  -305,  -260,  -204,-32768,   347,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   258,-32768,   315,
-32768,-32768,-32768,-32768,-32768,   626,   874,-32768,-32768,-32768,
-32768,-32768,-32768,   311,  -669,  -595,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   640,   875,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   422,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,  1140,   225,   -49,  -165,-32768,   696,   -79,-32768,-32768,
-32768,-32768,-32768,-32768,   -29,-32768,-32768,-32768,  1178,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -89,-32768,
-32768,-32768,-32768,-32768,  -514,   746,-32768,  -575,   383,  -481,
  -234,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   681,   549,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   445,-32768,-32768,-32768,
-32768,  1190,-32768,-32768,-32768,  1189,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,  1045,-32768,-32768,-32768,-32768,  1183,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768
};


#define	YYLAST		1237


static const short yytable[] = {   163,
    79,   307,   460,    58,   272,   559,   345,   384,   384,   146,
   469,   325,   146,   741,   531,   527,   676,   124,   114,   600,
   -37,   295,   310,   311,   665,   165,   228,    58,     2,     3,
    58,    58,   580,     1,    21,   588,   589,   262,   263,   264,
   265,   405,    32,   -37,   556,  -377,   405,   564,   166,    15,
   406,   173,    37,   279,   -37,   406,    62,    63,   212,   167,
   280,     2,     3,   312,   313,   314,   229,   354,   566,   174,
    15,   315,  -377,  -377,   147,   316,   695,    38,   712,   169,
   714,   590,     2,     3,    16,   407,    25,   565,   593,   531,
   408,   336,   337,     4,   170,   408,   409,   410,   342,   343,
   344,   409,   410,   470,   471,   472,   473,   223,   567,   474,
   346,   347,   348,   349,  -292,   219,   696,   317,   146,   350,
   436,   633,   -37,   438,   318,  -377,     4,   168,   286,   411,
   332,   758,   754,   707,   411,   146,  -377,  -377,    17,    18,
   422,   423,   424,   425,   426,   792,   300,     4,   432,    29,
   384,   617,   460,   781,   560,   729,  -292,  -292,  -292,   663,
   469,   461,   666,   561,  -292,   284,   924,   480,   140,   136,
  -292,   258,   755,   351,   146,   367,   368,   266,   421,   217,
   224,   225,   127,   268,   128,   478,   833,   137,   195,   196,
   197,   619,   732,   138,   692,   442,   479,   756,   188,   352,
   301,   464,   906,   269,   333,   320,   925,    55,   372,   562,
   563,   270,   271,   170,   376,   443,   526,   501,   503,   504,
   505,   506,     2,     3,   721,   384,   178,   724,   373,   762,
   433,   763,   374,   764,   373,   854,   855,    35,   374,   470,
   471,   472,   473,   353,   179,   474,   346,   347,   348,   349,
  -288,  -288,  -288,  -288,   573,   350,   405,   362,  -288,   598,
    65,  -301,  -301,  -301,  -301,   406,   363,   353,   599,  -301,
   370,   405,   513,   371,   345,    56,   346,   347,   348,   349,
   406,   833,   555,   116,   574,   350,    57,     4,   779,   475,
   727,   575,   373,   496,   592,    80,   374,   353,   733,   728,
   514,   861,   373,    66,   353,   408,   374,   613,   405,   351,
   932,   409,   410,   145,   373,   674,   151,   406,   374,    67,
   408,   576,   522,    68,   577,   523,   409,   410,   195,   196,
   197,    93,   857,   858,   353,   352,    40,   353,   499,   351,
   828,   863,   624,   687,   411,   233,   627,   628,   861,   405,
   234,   102,   711,   654,   656,   658,   353,   408,   406,   411,
   662,   720,   557,   409,   410,   352,   394,   143,   189,   190,
   191,   192,   672,   405,   193,   195,   196,   197,   117,   548,
   964,   373,   406,   194,   688,   374,   269,   862,   863,   578,
   579,   274,   122,   713,   270,   271,   411,   353,   408,   269,
   405,   730,   689,   874,   409,   410,   594,   270,   271,   406,
   195,   196,   197,   521,   606,   353,   353,   715,   198,   499,
   321,   373,   408,   199,   123,   374,   827,   322,   409,   410,
   405,   629,   630,   243,   244,   245,   150,   411,   268,   406,
   943,   246,   864,   449,   814,   157,   373,   275,   613,   408,
   374,   875,   353,   450,   733,   409,   410,   451,   269,   103,
   104,   411,   651,   200,   353,   201,   270,   271,   202,   203,
   204,   205,   206,   207,   815,   405,   839,   582,   158,   408,
   461,   766,   768,   232,   406,   409,   410,   164,   411,   778,
   940,   189,   190,   191,   192,   172,   373,   193,   876,    67,
   374,   115,   790,    68,   118,   119,   194,   195,   196,   197,
   373,   353,   693,   935,   374,   697,   413,   699,   411,   922,
   464,   872,   864,   445,   408,   936,   353,   708,   446,   175,
   409,   410,    67,   195,   196,   197,    68,   937,   353,    67,
   128,   215,   722,    68,    67,   725,   199,   177,    68,   461,
   353,   731,   492,   493,   494,   735,   736,   737,   738,   189,
   190,   191,   192,   411,   427,   193,   221,   222,   428,   189,
   190,   191,   192,   180,   194,   454,   195,   196,   197,   158,
   230,   835,   836,   837,   455,   132,   200,   463,   201,   464,
   465,   202,   203,   204,   205,   206,   207,   131,   182,   132,
   481,   195,   196,   197,   482,   466,   467,   184,   308,   468,
   309,   195,   196,   197,   199,   189,   190,   191,   192,   456,
   186,   601,   457,   210,   458,   798,   799,   896,   211,   802,
   602,   804,    81,    82,    83,    84,    85,   613,    39,    86,
    40,   -17,   189,   190,   191,   192,   288,   289,   193,   459,
   515,   516,   918,    87,   200,   213,   201,   194,   829,   202,
   203,   204,   205,   206,   207,    88,   216,    89,   603,   840,
   604,   842,   843,   844,   909,   910,   911,   227,   570,   235,
   761,   237,   239,   850,   195,   196,   197,   240,   253,   241,
    90,   256,   338,   259,    67,   605,    91,   199,    68,   268,
   502,   257,   867,   267,    92,   281,   189,   190,   191,   192,
   586,   292,   193,   353,   189,   190,   191,   192,   587,   269,
   454,   194,   701,   702,   461,   884,    67,   270,   271,   455,
    68,   716,   717,    53,    67,  -243,  -243,   893,    68,   201,
   701,   760,   202,   203,   204,   205,   206,   207,   195,   196,
   197,   195,   196,   197,    81,    82,   195,   196,   197,   462,
   285,   199,   463,   287,   464,   465,   917,   457,   919,   458,
   853,   293,   921,   507,   508,   152,   617,   243,   244,   245,
   466,   467,   294,   931,   468,   246,   153,    88,   154,    89,
   155,   299,   303,   156,   459,    69,    70,    71,    72,    73,
    74,    75,    76,   201,   632,    77,   202,   203,   204,   205,
   206,   207,    90,   305,   618,   645,   619,   620,    91,   323,
    67,   646,   959,   960,    68,   324,    92,   661,   671,   331,
   690,    67,   621,   622,   334,    68,   623,    67,   691,   710,
   746,    68,   335,    67,    67,   972,    67,    68,    68,   750,
    68,   752,   759,   977,    67,    67,    67,   774,    68,    68,
    68,   339,   777,   581,   783,    67,   340,    67,    67,    68,
   786,    68,    68,    67,   341,   787,   788,    68,    67,   355,
    67,   789,    68,   845,    68,   356,    67,   358,   846,   848,
    68,    67,    67,   360,   849,    68,    68,    67,   357,    67,
   870,    68,   878,    68,    67,    67,   364,   879,    68,    68,
    67,   365,   880,   881,    68,   887,    67,   353,    67,   366,
    68,   901,    68,    67,   914,   403,   920,    68,    67,    67,
   377,    67,    68,    68,   939,    68,   941,    67,   369,   942,
    67,    68,    67,   378,    68,   948,    68,   965,   966,   379,
    67,   387,    67,   974,    68,    67,    68,   414,   979,    68,
   401,    67,   412,    67,    67,    68,   416,    68,    68,    67,
   417,   981,   419,    68,    67,   434,   420,    20,    68,    22,
    23,   430,   435,   437,   448,    27,    69,    70,    71,    72,
    73,    74,    75,    76,   431,   440,    77,    94,    95,    96,
    97,    98,    99,   485,   100,   441,   488,   101,   189,   190,
   191,   192,   489,   490,   193,   497,   491,   500,   509,   510,
   519,   520,   529,   524,   532,   533,   535,   537,   540,   541,
   543,   544,   545,   546,   547,   558,   569,   551,   571,   552,
   585,   568,   595,   607,   611,   659,   625,   631,   635,   636,
   638,   640,   982,   641,   610,   643,   614,   644,   615,   647,
   616,   652,   653,   655,   657,   105,   660,   668,   669,   675,
   681,   682,   650,   684,   685,   686,   694,   705,   698,   706,
   700,   719,   723,   142,   734,   740,   745,   749,   386,   757,
   765,   767,   742,   769,   176,   773,   776,   743,   784,   744,
   785,   807,   796,   775,   797,   800,   801,   803,   806,   808,
   181,   809,   810,   811,   812,   813,   817,   819,   816,   822,
   823,   825,   826,   830,   831,   902,   834,   838,   841,   852,
   596,   860,   866,   869,   873,   851,   882,   883,   885,   886,
   891,   871,   892,   895,   897,   898,   899,   923,   903,   904,
   908,   912,   913,   915,   916,   926,   927,   928,   929,   930,
   933,   934,   938,   946,   608,   947,   949,   950,   951,   952,
   953,   954,   955,   957,   958,   962,   963,   968,   969,   900,
   970,   961,   971,   976,   978,   980,   983,   907,   389,   626,
   397,   106,   818,   108,   572,   856,   973,   525,   709,   583,
    24,    26,     0,   231,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   111,     0,     0,     0,     0,   113
};

static const short yycheck[] = {   121,
    38,   238,   379,    33,   209,   487,   267,   313,   314,    89,
   387,   246,    92,   683,   456,   454,   612,     1,    56,   534,
     4,   226,     3,     4,   600,     4,    22,    57,    31,    32,
    60,    61,   514,     3,    11,    45,    46,   203,   204,   205,
   206,    13,     1,    27,   483,     4,    13,    22,    27,    25,
    22,     1,    40,    19,    38,    22,     3,     4,   148,    38,
    26,    31,    32,    44,    45,    46,    62,   272,    22,    19,
    25,    52,    31,    32,     1,    56,    22,    65,   654,     4,
   656,    91,    31,    32,    39,    57,     3,    62,   527,   531,
    62,   257,   258,    96,    19,    62,    68,    69,   264,   265,
   266,    68,    69,     7,     8,     9,    10,     1,    62,    13,
    14,    15,    16,    17,    41,   153,    62,    98,   198,    23,
   357,   560,   106,   358,   105,    84,    96,   106,   218,   101,
   252,   707,    22,   648,   101,   215,    95,    96,    93,    94,
   345,   346,   347,   348,   349,   741,    95,    96,   353,     3,
   456,    22,   529,   729,    13,   670,    83,    84,    85,   598,
   537,    22,   601,    22,    91,   215,    22,   404,     1,     1,
    97,   201,    62,    77,   254,   297,   298,   207,   344,     1,
    74,    75,     1,     3,     3,    19,   782,    19,    49,    50,
    51,    62,   674,    25,   633,     3,    30,    87,     4,   103,
   230,    62,   872,    23,   254,   243,    62,     4,     4,    68,
    69,    31,    32,    19,     4,    23,   453,   422,   423,   424,
   425,   426,    31,    32,   663,   531,     1,   666,    24,   711,
     4,   713,    28,   715,    24,   811,   812,     3,    28,     7,
     8,     9,    10,    17,    19,    13,    14,    15,    16,    17,
    83,    84,    85,    86,     4,    23,    13,    13,    91,    13,
    25,    83,    84,    85,    86,    22,    22,    17,    22,    91,
   300,    13,     4,   303,   535,    84,    14,    15,    16,    17,
    22,   877,     4,    11,     4,    23,    95,    96,   727,    57,
    13,     4,    24,   415,     4,     4,    28,    17,   675,    22,
    57,    22,    24,     4,    17,    62,    28,   542,    13,    77,
   906,    68,    69,    89,    24,    57,    92,    22,    28,    20,
    62,     4,   444,    24,     4,   447,    68,    69,    49,    50,
    51,     4,   814,   815,    17,   103,     3,    17,   418,    77,
   779,    62,   547,     4,   101,    14,   551,   552,    22,    13,
    19,     4,    57,   588,   589,   590,    17,    62,    22,   101,
   597,     4,   484,    68,    69,   103,     3,     3,     7,     8,
     9,    10,   609,    13,    13,    49,    50,    51,     4,     3,
   956,    24,    22,    22,     4,    28,    23,    61,    62,   511,
   512,    41,     4,    57,    31,    32,   101,    17,    62,    23,
    13,     4,     4,     4,    68,    69,   528,    31,    32,    22,
    49,    50,    51,   443,   536,    17,    17,    57,    57,   499,
    19,    24,    62,    62,     4,    28,     4,    26,    68,    69,
    13,   553,   554,    83,    84,    85,     3,   101,     3,    22,
   922,    91,   819,    14,    57,     3,    24,    97,   683,    62,
    28,     4,    17,    24,   831,    68,    69,    28,    23,     3,
     4,   101,   584,   102,    17,   104,    31,    32,   107,   108,
   109,   110,   111,   112,    57,    13,     4,   515,     3,    62,
    22,   716,   717,     4,    22,    68,    69,     4,   101,   726,
     4,     7,     8,     9,    10,     4,    24,    13,     4,    20,
    28,    57,   739,    24,    60,    61,    22,    49,    50,    51,
    24,    17,   634,     4,    28,   637,     4,   639,   101,    57,
    62,    63,   899,     4,    62,     4,    17,   649,     4,     4,
    68,    69,    20,    49,    50,    51,    24,     4,    17,    20,
     3,    57,   664,    24,    20,   667,    62,     4,    24,    22,
    17,   673,    88,    89,    90,   677,   678,   679,   680,     7,
     8,     9,    10,   101,    20,    13,    71,    72,    24,     7,
     8,     9,    10,     4,    22,    13,    49,    50,    51,     3,
     4,   786,   787,   788,    22,     3,   102,    60,   104,    62,
    63,   107,   108,   109,   110,   111,   112,     1,     4,     3,
    20,    49,    50,    51,    24,    78,    79,     4,    19,    82,
    21,    49,    50,    51,    62,     7,     8,     9,    10,    57,
     4,    13,    60,     4,    62,   747,   748,   862,     4,   751,
    22,   753,    34,    35,    36,    37,    38,   872,     1,    41,
     3,     4,     7,     8,     9,    10,     3,     4,    13,    87,
     3,     4,   889,    55,   102,    84,   104,    22,   780,   107,
   108,   109,   110,   111,   112,    67,     4,    69,    60,   791,
    62,   793,   794,   795,   879,   880,   881,    25,     4,     4,
   710,     4,    14,   805,    49,    50,    51,    19,     3,    21,
    92,     3,    57,     3,    20,    87,    98,    62,    24,     3,
     4,    19,   824,     3,   106,    85,     7,     8,     9,    10,
     4,    32,    13,    17,     7,     8,     9,    10,     4,    23,
    13,    22,     3,     4,    22,   847,    20,    31,    32,    22,
    24,    45,    46,     1,    20,     3,     4,   859,    24,   104,
     3,     4,   107,   108,   109,   110,   111,   112,    49,    50,
    51,    49,    50,    51,    34,    35,    49,    50,    51,    57,
     4,    62,    60,     4,    62,    63,   888,    60,   890,    62,
   808,    32,   894,   427,   428,    55,    22,    83,    84,    85,
    78,    79,     4,   905,    82,    91,    66,    67,    68,    69,
    70,     3,     5,    73,    87,    19,    20,    21,    22,    23,
    24,    25,    26,   104,     4,    29,   107,   108,   109,   110,
   111,   112,    92,    21,    60,     4,    62,    63,    98,     3,
    20,     4,   944,   945,    24,     3,   106,     4,     4,    86,
     4,    20,    78,    79,     4,    24,    82,    20,     4,     4,
     4,    24,    22,    20,    20,   967,    20,    24,    24,     4,
    24,     4,     4,   975,    20,    20,    20,     4,    24,    24,
    24,     4,     4,    87,     4,    20,     4,    20,    20,    24,
     4,    24,    24,    20,     4,     4,     4,    24,    20,     3,
    20,     4,    24,     4,    24,     3,    20,    91,     4,     4,
    24,    20,    20,     4,     4,    24,    24,    20,     3,    20,
     4,    24,     4,    24,    20,    20,     4,     4,    24,    24,
    20,     4,     4,     4,    24,     4,    20,    17,    20,     4,
    24,     4,    24,    20,     4,    33,     4,    24,    20,    20,
    99,    20,    24,    24,     4,    24,     4,    20,    22,     4,
    20,    24,    20,     3,    24,     4,    24,     4,     4,     3,
    20,     3,    20,     4,    24,    20,    24,     4,     4,    24,
     3,    20,     3,    20,    20,    24,     4,    24,    24,    20,
     4,     0,     4,    24,    20,    25,     4,     6,    24,     8,
     9,     4,    22,     3,     6,    14,    19,    20,    21,    22,
    23,    24,    25,    26,    29,     4,    29,    42,    43,    44,
    45,    46,    47,     3,    49,    86,     3,    52,     7,     8,
     9,    10,     3,     3,    13,     3,    24,     4,     4,     4,
     3,    23,     3,    21,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,    76,     4,    32,    76,     4,    76,
     4,    21,     4,     4,     3,   100,     4,     4,     3,     3,
     3,     3,     0,     4,    22,     4,    23,     4,    23,     4,
    23,     4,     3,     3,     3,    54,     4,     3,     3,     3,
     3,     3,    32,     3,     3,     3,    22,     4,    22,     4,
    22,     3,     3,    88,     4,    22,     4,     3,   314,     4,
     3,     3,    23,     3,   130,     4,     4,    23,     4,    23,
     3,    85,     4,    22,     4,    22,     4,     4,     3,     3,
   134,     4,     4,     4,     4,     4,     4,     3,    25,     4,
     4,     4,     4,     3,     3,   868,     4,     4,     4,     3,
   531,     4,     4,     4,     4,    22,     4,     4,     4,     4,
     4,    22,     4,     3,     3,     3,     3,    22,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     3,   539,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     3,   865,
     4,    22,     4,     4,     4,     4,     0,   877,   315,   550,
   316,    54,   771,    54,   499,   813,    22,   452,   650,   519,
    11,    13,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    54,    -1,    -1,    -1,    -1,    54
};

#line 325 "/lib/bison.cc"
 /* fattrs + tables */

/* parser code folow  */


/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: dollar marks section change
   the next  is replaced by the list of actions, each action
   as one case of the switch.  */ 

#if YY_PDDL_Parser_USE_GOTO != 0
/* 
 SUPRESSION OF GOTO : on some C++ compiler (sun c++)
  the goto is strictly forbidden if any constructor/destructor
  is used in the whole function (very stupid isn't it ?)
 so goto are to be replaced with a 'while/switch/case construct'
 here are the macro to keep some apparent compatibility
*/
#define YYGOTO(lb) {yy_gotostate=lb;continue;}
#define YYBEGINGOTO  enum yy_labels yy_gotostate=yygotostart; \
                     for(;;) switch(yy_gotostate) { case yygotostart: {
#define YYLABEL(lb) } case lb: {
#define YYENDGOTO } } 
#define YYBEGINDECLARELABEL enum yy_labels {yygotostart
#define YYDECLARELABEL(lb) ,lb
#define YYENDDECLARELABEL  };
#else
/* macro to keep goto */
#define YYGOTO(lb) goto lb
#define YYBEGINGOTO 
#define YYLABEL(lb) lb:
#define YYENDGOTO
#define YYBEGINDECLARELABEL 
#define YYDECLARELABEL(lb)
#define YYENDDECLARELABEL 
#endif
/* LABEL DECLARATION */
YYBEGINDECLARELABEL
  YYDECLARELABEL(yynewstate)
  YYDECLARELABEL(yybackup)
/* YYDECLARELABEL(yyresume) */
  YYDECLARELABEL(yydefault)
  YYDECLARELABEL(yyreduce)
  YYDECLARELABEL(yyerrlab)   /* here on detecting error */
  YYDECLARELABEL(yyerrlab1)   /* here on error raised explicitly by an action */
  YYDECLARELABEL(yyerrdefault)  /* current state does not do anything special for the error token. */
  YYDECLARELABEL(yyerrpop)   /* pop the current state because it cannot handle the error token */
  YYDECLARELABEL(yyerrhandle)  
YYENDDECLARELABEL
/* ALLOCA SIMULATION */
/* __HAVE_NO_ALLOCA */
#ifdef __HAVE_NO_ALLOCA
int __alloca_free_ptr(char *ptr,char *ref)
{if(ptr!=ref) free(ptr);
 return 0;}

#define __ALLOCA_alloca(size) malloc(size)
#define __ALLOCA_free(ptr,ref) __alloca_free_ptr((char *)ptr,(char *)ref)

#ifdef YY_PDDL_Parser_LSP_NEEDED
#define __ALLOCA_return(num) \
            return( __ALLOCA_free(yyss,yyssa)+\
		    __ALLOCA_free(yyvs,yyvsa)+\
		    __ALLOCA_free(yyls,yylsa)+\
		   (num))
#else
#define __ALLOCA_return(num) \
            return( __ALLOCA_free(yyss,yyssa)+\
		    __ALLOCA_free(yyvs,yyvsa)+\
		   (num))
#endif
#else
#define __ALLOCA_return(num) return(num)
#define __ALLOCA_alloca(size) alloca(size)
#define __ALLOCA_free(ptr,ref) 
#endif

/* ENDALLOCA SIMULATION */

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (YY_PDDL_Parser_CHAR = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        __ALLOCA_return(0)
#define YYABORT         __ALLOCA_return(1)
#define YYERROR         YYGOTO(yyerrlab1)
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL          YYGOTO(yyerrlab)
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do                                                              \
  if (YY_PDDL_Parser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_PDDL_Parser_CHAR = (token), YY_PDDL_Parser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_PDDL_Parser_CHAR);                                \
      YYPOPSTACK;                                               \
      YYGOTO(yybackup);                                            \
    }                                                           \
  else                                                          \
    { YY_PDDL_Parser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_PDDL_Parser_PURE
/* UNPURE */
#define YYLEX           YY_PDDL_Parser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_PDDL_Parser_CHAR;                      /*  the lookahead symbol        */
YY_PDDL_Parser_STYPE      YY_PDDL_Parser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_PDDL_Parser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_PDDL_Parser_LSP_NEEDED
YY_PDDL_Parser_LTYPE YY_PDDL_Parser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_PDDL_Parser_LSP_NEEDED
#define YYLEX           YY_PDDL_Parser_LEX(&YY_PDDL_Parser_LVAL, &YY_PDDL_Parser_LLOC)
#else
#define YYLEX           YY_PDDL_Parser_LEX(&YY_PDDL_Parser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_PDDL_Parser_DEBUG != 0
int YY_PDDL_Parser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif
#endif



/*  YYINITDEPTH indicates the initial size of the parser's stacks       */

#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif


#if __GNUC__ > 1                /* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM,TO,COUNT)       __builtin_memcpy(TO,FROM,COUNT)
#else                           /* not GNU C or C++ */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */

#ifdef __cplusplus
static void __yy_bcopy (char *from, char *to, int count)
#else
#ifdef __STDC__
static void __yy_bcopy (char *from, char *to, int count)
#else
static void __yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
#endif
#endif
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}
#endif

int
#ifdef YY_USE_CLASS
 YY_PDDL_Parser_CLASS::
#endif
     YY_PDDL_Parser_PARSE(YY_PDDL_Parser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_PDDL_Parser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_PDDL_Parser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_PDDL_Parser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_PDDL_Parser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_PDDL_Parser_LSP_NEEDED
  YY_PDDL_Parser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_PDDL_Parser_LTYPE *yyls = yylsa;
  YY_PDDL_Parser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_PDDL_Parser_PURE
  int YY_PDDL_Parser_CHAR;
  YY_PDDL_Parser_STYPE YY_PDDL_Parser_LVAL;
  int YY_PDDL_Parser_NERRS;
#ifdef YY_PDDL_Parser_LSP_NEEDED
  YY_PDDL_Parser_LTYPE YY_PDDL_Parser_LLOC;
#endif
#endif

  YY_PDDL_Parser_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;
/* start loop, in which YYGOTO may be used. */
YYBEGINGOTO

#if YY_PDDL_Parser_DEBUG != 0
  if (YY_PDDL_Parser_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif
  yystate = 0;
  yyerrstatus = 0;
  YY_PDDL_Parser_NERRS = 0;
  YY_PDDL_Parser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_PDDL_Parser_LSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
YYLABEL(yynewstate)

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YY_PDDL_Parser_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_PDDL_Parser_LSP_NEEDED
      YY_PDDL_Parser_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_PDDL_Parser_LSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YY_PDDL_Parser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_PDDL_Parser_ERROR("parser stack overflow");
	  __ALLOCA_return(2);
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) __ALLOCA_alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      __ALLOCA_free(yyss1,yyssa);
      yyvs = (YY_PDDL_Parser_STYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
      __ALLOCA_free(yyvs1,yyvsa);
#ifdef YY_PDDL_Parser_LSP_NEEDED
      yyls = (YY_PDDL_Parser_LTYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      __ALLOCA_free(yyls1,yylsa);
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_PDDL_Parser_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_PDDL_Parser_DEBUG != 0
      if (YY_PDDL_Parser_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_PDDL_Parser_DEBUG != 0
  if (YY_PDDL_Parser_DEBUG_FLAG)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  YYGOTO(yybackup);
YYLABEL(yybackup)

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* YYLABEL(yyresume) */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    YYGOTO(yydefault);

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (YY_PDDL_Parser_CHAR == YYEMPTY)
    {
#if YY_PDDL_Parser_DEBUG != 0
      if (YY_PDDL_Parser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_PDDL_Parser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_PDDL_Parser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_PDDL_Parser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_PDDL_Parser_DEBUG != 0
      if (YY_PDDL_Parser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_PDDL_Parser_CHAR);

#if YY_PDDL_Parser_DEBUG != 0
      if (YY_PDDL_Parser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_PDDL_Parser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_PDDL_Parser_CHAR, YY_PDDL_Parser_LVAL);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    YYGOTO(yydefault);

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	YYGOTO(yyerrlab);
      yyn = -yyn;
      YYGOTO(yyreduce);
    }
  else if (yyn == 0)
    YYGOTO(yyerrlab);

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YY_PDDL_Parser_DEBUG != 0
  if (YY_PDDL_Parser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_PDDL_Parser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_PDDL_Parser_CHAR != YYEOF)
    YY_PDDL_Parser_CHAR = YYEMPTY;

  *++yyvsp = YY_PDDL_Parser_LVAL;
#ifdef YY_PDDL_Parser_LSP_NEEDED
  *++yylsp = YY_PDDL_Parser_LLOC;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  YYGOTO(yynewstate);

/* Do the default action for the current state.  */
YYLABEL(yydefault)

  yyn = yydefact[yystate];
  if (yyn == 0)
    YYGOTO(yyerrlab);

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
YYLABEL(yyreduce)
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YY_PDDL_Parser_DEBUG != 0
  if (YY_PDDL_Parser_DEBUG_FLAG)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


/* #line 811 "/lib/bison.cc" */
#line 2334 "grammar.cc"

  switch (yyn) {

case 8:
#line 107 "pddl2.y"
{
  log_error("syntax error in domain");
  yyerrok;
;
    break;}
case 18:
#line 127 "pddl2.y"
{
  domain_name = yyvsp[-1].sym->text;
  if (current_file()) domain_file = strdup(current_file());
;
    break;}
case 19:
#line 134 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 20:
#line 135 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 21:
#line 136 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 22:
#line 137 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 23:
#line 138 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 24:
#line 139 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 25:
#line 140 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 26:
#line 141 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 27:
#line 142 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 28:
#line 146 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 29:
#line 147 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 30:
#line 151 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 31:
#line 152 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 33:
#line 160 "pddl2.y"
{
  log_error("syntax error in requirements declaration.");
  yyerrok; 
;
    break;}
case 39:
#line 178 "pddl2.y"
{
  log_error("syntax error in (:predicates ...)");
  yyerrok;
;
    break;}
case 42:
#line 191 "pddl2.y"
{
  current_n_param = 0;
;
    break;}
case 43:
#line 195 "pddl2.y"
{
  dom_predicates[n_predicates] = new PredicateSymbol(yyvsp[-3].sym->text);
  for (index_type k = 0; k < current_n_param; k++)
    dom_predicates[n_predicates]->param[k] = current_param[k];
  dom_predicates[n_predicates]->n_param = current_n_param;
  clear_context(current_param, current_n_param);
  yyvsp[-3].sym->val = dom_predicates[n_predicates];
  n_predicates += 1;
;
    break;}
case 44:
#line 205 "pddl2.y"
{
  log_error("syntax error in predicate declaration");
  yyerrok;
;
    break;}
case 45:
#line 213 "pddl2.y"
{
  set_variable_type(current_param, current_n_param, (TypeSymbol*)yyvsp[0].sym->val);
;
    break;}
case 46:
#line 217 "pddl2.y"
{
  set_variable_type(current_param, current_n_param, dom_top_type);
;
    break;}
case 48:
#line 225 "pddl2.y"
{
  yyvsp[0].sym->val = new VariableSymbol(yyvsp[0].sym->text);
  current_param[current_n_param] = (VariableSymbol*)yyvsp[0].sym->val;
  current_n_param += 1;
  if (trace_print_context) {
    std::cerr << "variable ";
    current_param[current_n_param - 1]->print(std::cerr);
    std::cerr << " added to context (now "
	      << current_n_param << " variables)"
	      << std::endl;
  }
;
    break;}
case 49:
#line 238 "pddl2.y"
{
  std::cerr << "error: variable ";
  ((VariableSymbol*)yyvsp[0].sym->val)->print(std::cerr);
  std::cerr << " redeclared" << std::endl;
  exit(255);
;
    break;}
case 50:
#line 245 "pddl2.y"
{
  yyvsp[0].sym->val = new VariableSymbol(yyvsp[0].sym->text);
  current_param[current_n_param] = (VariableSymbol*)yyvsp[0].sym->val;
  current_n_param += 1;
  if (trace_print_context) {
    std::cerr << "variable ";
    current_param[current_n_param - 1]->print(std::cerr);
    std::cerr << " added to context (now "
	      << current_n_param << " variables)"
	      << std::endl;
  }
;
    break;}
case 51:
#line 258 "pddl2.y"
{
  std::cerr << "error: variable ";
  ((VariableSymbol*)yyvsp[0].sym->val)->print(std::cerr);
  std::cerr << " redeclared" << std::endl;
  exit(255);
;
    break;}
case 53:
#line 271 "pddl2.y"
{
  log_error("syntax error in (:functions ...)");
  yyerrok;
;
    break;}
case 56:
#line 284 "pddl2.y"
{
  current_n_param = 0;
;
    break;}
case 57:
#line 288 "pddl2.y"
{
  dom_functions[n_functions] = new FunctionSymbol(yyvsp[-3].sym->text);
  for (index_type k = 0; k < current_n_param; k++)
    dom_functions[n_functions]->param[k] = current_param[k];
  dom_functions[n_functions]->n_param = current_n_param;
  clear_context(current_param, current_n_param);
  yyvsp[-3].sym->val = dom_functions[n_functions];
  n_functions += 1;
;
    break;}
case 58:
#line 298 "pddl2.y"
{
  log_error("syntax error in function declaration");
  yyerrok;
;
    break;}
case 60:
#line 312 "pddl2.y"
{
  set_type_type(dom_types, n_types, (TypeSymbol*)yyvsp[0].sym->val);
;
    break;}
case 61:
#line 316 "pddl2.y"
{
  yyvsp[0].sym->val = new TypeSymbol(yyvsp[0].sym->text);
  if (write_warnings)
    std::cerr << "warning: assuming " << yyvsp[0].sym->text << " - object" << std::endl;
  ((TypeSymbol*)yyvsp[0].sym->val)->sym_type = dom_top_type;
  set_type_type(dom_types, n_types, (TypeSymbol*)yyvsp[0].sym->val);
  dom_types[n_types] = (TypeSymbol*)yyvsp[0].sym->val;
  n_types += 1;
;
    break;}
case 62:
#line 326 "pddl2.y"
{
  set_type_type(dom_types, n_types, dom_top_type);
;
    break;}
case 64:
#line 334 "pddl2.y"
{
  /* the type is already (implicitly) declared */
;
    break;}
case 65:
#line 338 "pddl2.y"
{
  yyvsp[0].sym->val = new TypeSymbol(yyvsp[0].sym->text);
  dom_types[n_types] = (TypeSymbol*)yyvsp[0].sym->val;
  n_types += 1;
;
    break;}
case 69:
#line 355 "pddl2.y"
{
  set_constant_type(dom_constants, n_constants, (TypeSymbol*)yyvsp[0].sym->val);
;
    break;}
case 70:
#line 359 "pddl2.y"
{
  set_constant_type(dom_constants, n_constants, dom_top_type);
;
    break;}
case 72:
#line 367 "pddl2.y"
{
  yyvsp[0].sym->val = new Symbol(yyvsp[0].sym->text);
  dom_constants[n_constants] = (Symbol*)yyvsp[0].sym->val;
  if (problem_name) dom_constants[n_constants]->defined_in_problem = true;
  n_constants += 1;
;
    break;}
case 73:
#line 374 "pddl2.y"
{
  yyvsp[0].sym->val = new Symbol(yyvsp[0].sym->text);
  dom_constants[n_constants] = (Symbol*)yyvsp[0].sym->val;
  if (problem_name) dom_constants[n_constants]->defined_in_problem = true;
  n_constants += 1;
;
    break;}
case 77:
#line 394 "pddl2.y"
{
  dom_actions[n_actions] = new ActionSymbol(yyvsp[0].sym->text);
;
    break;}
case 78:
#line 398 "pddl2.y"
{
  // post-processing should actually be done on all actions after the complete
  // domain and problem have been read (calling PDDL_Base::post_process())
  dom_actions[n_actions]->post_process();
  clear_context(current_param, current_n_param);
  yyvsp[-3].sym->val = dom_actions[n_actions];
  n_actions += 1;
;
    break;}
case 79:
#line 407 "pddl2.y"
{
  log_error("syntax error in action declaration");
  yyerrok; 
;
    break;}
case 80:
#line 415 "pddl2.y"
{
  current_n_param = 0;
;
    break;}
case 81:
#line 419 "pddl2.y"
{
  for (index_type k = 0; k < current_n_param; k++)
    dom_actions[n_actions]->param[k] = current_param[k];
  dom_actions[n_actions]->n_param = current_n_param;
;
    break;}
case 88:
#line 431 "pddl2.y"
{
  // std::cerr << "read assoc string: [" << $3 << "]" << std::endl;
  dom_actions[n_actions]->assoc = yyvsp[0].sval;
;
    break;}
case 90:
#line 440 "pddl2.y"
{
  SetSymbol* ssym = new SetSymbol(yyvsp[0].sym->text);
  yyvsp[0].sym->val = ssym;
  partitions.append(ssym);
  SetName* s = new SetName(ssym);
  current_atom = s;
;
    break;}
case 91:
#line 448 "pddl2.y"
{
  dom_actions[n_actions]->part = (SetName*)current_atom;
;
    break;}
case 92:
#line 452 "pddl2.y"
{
  SetName* s = new SetName((SetSymbol*)yyvsp[0].sym->val);
  current_atom = s;
;
    break;}
case 93:
#line 457 "pddl2.y"
{
  dom_actions[n_actions]->part = (SetName*)current_atom;
;
    break;}
case 101:
#line 477 "pddl2.y"
{
  dom_actions[n_actions]->num_pre[dom_actions[n_actions]->n_num_pre] = yyvsp[0].rel;
  dom_actions[n_actions]->n_num_pre += 1;
;
    break;}
case 102:
#line 485 "pddl2.y"
{
  yyval.tkw = PDDL_Base::md_start;
;
    break;}
case 103:
#line 489 "pddl2.y"
{
  yyval.tkw = PDDL_Base::md_end;
;
    break;}
case 104:
#line 493 "pddl2.y"
{
  yyval.tkw = PDDL_Base::md_all;
;
    break;}
case 105:
#line 500 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 106:
#line 504 "pddl2.y"
{
  dom_actions[n_actions]->pos_pre[dom_actions[n_actions]->n_pos_pre] =
    (Atom*)current_atom;
  dom_actions[n_actions]->n_pos_pre += 1;
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 107:
#line 511 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param[0] = (Symbol*)yyvsp[-2].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-1].sym->val;
  eq_atom->n_param = 2;
  dom_actions[n_actions]->pos_pre[dom_actions[n_actions]->n_pos_pre] =
    eq_atom;
  dom_actions[n_actions]->n_pos_pre += 1;
;
    break;}
case 108:
#line 521 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, yyvsp[-2].tkw);
;
    break;}
case 109:
#line 525 "pddl2.y"
{
  dom_actions[n_actions]->pos_pre[dom_actions[n_actions]->n_pos_pre] =
    (Atom*)current_atom;
  dom_actions[n_actions]->n_pos_pre += 1;
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 110:
#line 532 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred, yyvsp[-6].tkw);
  eq_atom->param[0] = (Symbol*)yyvsp[-3].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-2].sym->val;
  eq_atom->n_param = 2;
  dom_actions[n_actions]->pos_pre[dom_actions[n_actions]->n_pos_pre] =
    eq_atom;
  dom_actions[n_actions]->n_pos_pre += 1;
;
    break;}
case 111:
#line 545 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 112:
#line 549 "pddl2.y"
{
  dom_actions[n_actions]->neg_pre[dom_actions[n_actions]->n_neg_pre] =
    (Atom*)current_atom;
  dom_actions[n_actions]->n_neg_pre += 1;
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 113:
#line 556 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param[0] = (Symbol*)yyvsp[-3].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-2].sym->val;
  eq_atom->n_param = 2;
  dom_actions[n_actions]->neg_pre[dom_actions[n_actions]->n_neg_pre] =
    eq_atom;
  dom_actions[n_actions]->n_neg_pre += 1;
;
    break;}
case 114:
#line 566 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
;
    break;}
case 115:
#line 570 "pddl2.y"
{
  dom_actions[n_actions]->neg_pre[dom_actions[n_actions]->n_neg_pre] =
    (Atom*)current_atom;
  dom_actions[n_actions]->n_neg_pre += 1;
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 116:
#line 578 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred, yyvsp[-9].tkw);
  eq_atom->param[0] = (Symbol*)yyvsp[-4].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-3].sym->val;
  eq_atom->n_param = 2;
  dom_actions[n_actions]->neg_pre[dom_actions[n_actions]->n_neg_pre] =
    eq_atom;
  dom_actions[n_actions]->n_neg_pre += 1;
;
    break;}
case 117:
#line 591 "pddl2.y"
{
  if (yyvsp[0].sym->val == 0) {
    log_error("undeclared variable in atom args list");
  }
  else if (current_atom != 0) {
    current_atom->param[current_atom->n_param] = (VariableSymbol*)yyvsp[0].sym->val;
    current_atom->n_param += 1;
  }
;
    break;}
case 118:
#line 601 "pddl2.y"
{
  if (current_atom != 0) {
    current_atom->param[current_atom->n_param] = (Symbol*)yyvsp[0].sym->val;
    current_atom->n_param += 1;  
  }
;
    break;}
case 120:
#line 612 "pddl2.y"
{
  yyval.rel = new Relation(yyvsp[-3].rkw, yyvsp[-2].exp, yyvsp[-1].exp);
;
    break;}
case 121:
#line 617 "pddl2.y"
{
  yyval.rel = new Relation(yyvsp[-4].rkw, yyvsp[-6].tkw, yyvsp[-3].exp, yyvsp[-2].exp);
;
    break;}
case 122:
#line 624 "pddl2.y"
{
  yyval.rkw = rel_greater;
;
    break;}
case 123:
#line 628 "pddl2.y"
{
  yyval.rkw = rel_greater_equal;
;
    break;}
case 124:
#line 632 "pddl2.y"
{
  yyval.rkw = rel_less;
;
    break;}
case 125:
#line 636 "pddl2.y"
{
  yyval.rkw = rel_less_equal;
;
    break;}
case 126:
#line 640 "pddl2.y"
{
  yyval.rkw = rel_equal;
;
    break;}
case 127:
#line 647 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_sub, new ConstantExpression(0), yyvsp[-1].exp);
;
    break;}
case 128:
#line 651 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_add, yyvsp[-2].exp, yyvsp[-1].exp);
;
    break;}
case 129:
#line 655 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_sub, yyvsp[-2].exp, yyvsp[-1].exp);
;
    break;}
case 130:
#line 659 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_mul, yyvsp[-2].exp, yyvsp[-1].exp);
;
    break;}
case 131:
#line 663 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_div, yyvsp[-2].exp, yyvsp[-1].exp);
;
    break;}
case 132:
#line 667 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_div, yyvsp[-2].exp, yyvsp[0].exp);
;
    break;}
case 133:
#line 671 "pddl2.y"
{
  yyval.exp = new ConstantExpression(yyvsp[0].ival);
;
    break;}
case 134:
#line 675 "pddl2.y"
{
  yyval.exp = new ConstantExpression(NN_TO_N(yyvsp[0].rval));
;
    break;}
case 135:
#line 679 "pddl2.y"
{
  yyval.exp = new TimeExpression();
;
    break;}
case 136:
#line 683 "pddl2.y"
{
  yyval.exp = new PreferenceExpression((Symbol*)yyvsp[-1].sym->val);
;
    break;}
case 137:
#line 687 "pddl2.y"
{
  yyval.exp = yyvsp[0].exp;
;
    break;}
case 138:
#line 694 "pddl2.y"
{
  yyval.exp = new FunctionExpression((FunctionSymbol*)yyvsp[-2].sym->val, yyvsp[-1].lst);
;
    break;}
case 139:
#line 698 "pddl2.y"
{
  yyval.exp = new FunctionExpression((FunctionSymbol*)yyvsp[0].sym->val, 0);
;
    break;}
case 140:
#line 705 "pddl2.y"
{
  yyval.lst = new ListExpression((VariableSymbol*)yyvsp[-1].sym->val, yyvsp[0].lst);
;
    break;}
case 141:
#line 709 "pddl2.y"
{
  yyval.lst = new ListExpression((Symbol*)yyvsp[-1].sym->val, yyvsp[0].lst);
;
    break;}
case 142:
#line 713 "pddl2.y"
{
  yyval.lst = 0;
;
    break;}
case 143:
#line 720 "pddl2.y"
{
  stored_n_param = current_n_param;
  current_context = new SetOf();
;
    break;}
case 144:
#line 725 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 145:
#line 729 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->atom = (Atom*)current_atom;
  dom_actions[n_actions]->set_pre[dom_actions[n_actions]->n_set_pre] = s;
  dom_actions[n_actions]->n_set_pre += 1;
  ((Atom*)current_atom)->pred->pos_pre = true;
  clear_context(current_param, stored_n_param, current_n_param);
  current_n_param = stored_n_param;
  current_context = 0;
;
    break;}
case 146:
#line 740 "pddl2.y"
{
  stored_n_param = current_n_param;
  current_context = new SetOf();
;
    break;}
case 147:
#line 745 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (index_type k = stored_n_param; k < current_n_param; k++) {
    s->param[s->n_param] = current_param[k];
    s->n_param += 1;
  }
;
    break;}
case 148:
#line 753 "pddl2.y"
{
  clear_context(current_param, stored_n_param, current_n_param);
  current_n_param = stored_n_param;
  current_context = 0;
;
    break;}
case 149:
#line 759 "pddl2.y"
{
  stored_n_param = current_n_param;
  current_context = new SetOf();
;
    break;}
case 150:
#line 764 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 151:
#line 768 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->atom = (Atom*)current_atom;
  s->atom->at = yyvsp[-12].tkw;
  dom_actions[n_actions]->set_pre[dom_actions[n_actions]->n_set_pre] = s;
  dom_actions[n_actions]->n_set_pre += 1;
  ((Atom*)current_atom)->pred->pos_pre = true;
  clear_context(current_param, stored_n_param, current_n_param);
  current_n_param = stored_n_param;
  current_context = 0;
;
    break;}
case 152:
#line 780 "pddl2.y"
{
  stored_n_param = current_n_param;
  current_context = new SetOf();
;
    break;}
case 153:
#line 785 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (index_type k = stored_n_param; k < current_n_param; k++) {
    s->param[s->n_param] = current_param[k];
    s->n_param += 1;
  }
;
    break;}
case 154:
#line 793 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->atom->at = yyvsp[-10].tkw;
  clear_context(current_param, stored_n_param, current_n_param);
  current_n_param = stored_n_param;
  current_context = 0;
;
    break;}
case 159:
#line 815 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 160:
#line 819 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->atom = (Atom*)current_atom;
  dom_actions[n_actions]->set_pre[dom_actions[n_actions]->n_set_pre] = s;
  dom_actions[n_actions]->n_set_pre += 1;
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 161:
#line 827 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 162:
#line 831 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->atom = (Atom*)current_atom;
  s->neg = true;
  dom_actions[n_actions]->set_pre[dom_actions[n_actions]->n_set_pre] = s;
  dom_actions[n_actions]->n_set_pre += 1;
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 163:
#line 840 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, yyvsp[-2].tkw);
;
    break;}
case 164:
#line 844 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->atom = (Atom*)current_atom;
  dom_actions[n_actions]->set_pre[dom_actions[n_actions]->n_set_pre] = s;
  dom_actions[n_actions]->n_set_pre += 1;
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 165:
#line 852 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
;
    break;}
case 166:
#line 856 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->atom = (Atom*)current_atom;
  s->neg = true;
  dom_actions[n_actions]->set_pre[dom_actions[n_actions]->n_set_pre] = s;
  dom_actions[n_actions]->n_set_pre += 1;
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 171:
#line 878 "pddl2.y"
{
  stored_n_param = current_n_param;
  current_context = new SetOf();
;
    break;}
case 172:
#line 883 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (index_type k = stored_n_param; k < current_n_param; k++) {
    s->param[s->n_param] = current_param[k];
    s->n_param += 1;
  }
;
    break;}
case 173:
#line 891 "pddl2.y"
{
  dom_actions[n_actions]->set_eff[dom_actions[n_actions]->n_set_eff] =
    (SetOf*)current_context;
  dom_actions[n_actions]->n_set_eff += 1;
  current_context = 0;
  clear_context(current_param, stored_n_param, current_n_param);
  current_n_param = stored_n_param;
;
    break;}
case 174:
#line 900 "pddl2.y"
{
  current_context = new SetOf();
;
    break;}
case 175:
#line 904 "pddl2.y"
{
  dom_actions[n_actions]->set_eff[dom_actions[n_actions]->n_set_eff] =
    (SetOf*)current_context;
  dom_actions[n_actions]->n_set_eff += 1;
  current_context = 0;
;
    break;}
case 176:
#line 911 "pddl2.y"
{
  stored_n_param = current_n_param;
  current_context = new SetOf();
;
    break;}
case 177:
#line 916 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (index_type k = stored_n_param; k < current_n_param; k++) {
    s->param[s->n_param] = current_param[k];
    s->n_param += 1;
  }
;
    break;}
case 178:
#line 924 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->atom->at = yyvsp[-10].tkw;
  dom_actions[n_actions]->set_eff[dom_actions[n_actions]->n_set_eff] = s;
  dom_actions[n_actions]->n_set_eff += 1;
  current_context = 0;
  clear_context(current_param, stored_n_param, current_n_param);
  current_n_param = stored_n_param;
;
    break;}
case 179:
#line 934 "pddl2.y"
{
  current_context = new SetOf();
;
    break;}
case 180:
#line 938 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->atom->at = yyvsp[-6].tkw;
  dom_actions[n_actions]->set_eff[dom_actions[n_actions]->n_set_eff] = s;
  dom_actions[n_actions]->n_set_eff += 1;
  current_context = 0;
;
    break;}
case 189:
#line 966 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
  ((PredicateSymbol*)yyvsp[0].sym->val)->modded = true;
;
    break;}
case 190:
#line 971 "pddl2.y"
{
  if (current_context != 0) {
    ((SetOf*)current_context)->atom = (Atom*)current_atom;
  }
  else {
    dom_actions[n_actions]->adds[dom_actions[n_actions]->n_adds] =
      (Atom*)current_atom;
    dom_actions[n_actions]->n_adds += 1;
  }
;
    break;}
case 191:
#line 982 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, yyvsp[-2].tkw);
  ((PredicateSymbol*)yyvsp[0].sym->val)->modded = true;
;
    break;}
case 192:
#line 987 "pddl2.y"
{
  if (current_context != 0) {
    ((SetOf*)current_context)->atom = (Atom*)current_atom;
  }
  else {
    dom_actions[n_actions]->adds[dom_actions[n_actions]->n_adds] =
      (Atom*)current_atom;
    dom_actions[n_actions]->n_adds += 1;
  }
;
    break;}
case 193:
#line 1001 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
  ((PredicateSymbol*)yyvsp[0].sym->val)->modded = true;
;
    break;}
case 194:
#line 1006 "pddl2.y"
{
  if (current_context != 0) {
    ((SetOf*)current_context)->atom = (Atom*)current_atom;
    ((SetOf*)current_context)->neg = true;
  }
  else {
    dom_actions[n_actions]->dels[dom_actions[n_actions]->n_dels] =
      (Atom*)current_atom;
    dom_actions[n_actions]->n_dels += 1;
  }
;
    break;}
case 195:
#line 1018 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
  ((PredicateSymbol*)yyvsp[0].sym->val)->modded = true;
;
    break;}
case 196:
#line 1023 "pddl2.y"
{
  if (current_context != 0) {
    ((SetOf*)current_context)->atom = (Atom*)current_atom;
    ((SetOf*)current_context)->neg = true;
  }
  else {
    dom_actions[n_actions]->dels[dom_actions[n_actions]->n_dels] =
      (Atom*)current_atom;
    dom_actions[n_actions]->n_dels += 1;
  }
;
    break;}
case 197:
#line 1038 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 198:
#line 1042 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-1].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-1].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[n_actions]->incs[dom_actions[n_actions]->n_incs] =
    (FChangeAtom*)current_atom;
  dom_actions[n_actions]->n_incs += 1;
;
    break;}
case 199:
#line 1051 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 200:
#line 1055 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-1].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-1].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[n_actions]->decs[dom_actions[n_actions]->n_decs] =
    (FChangeAtom*)current_atom;
  dom_actions[n_actions]->n_decs += 1;
;
    break;}
case 201:
#line 1064 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 202:
#line 1068 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-1].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-1].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[n_actions]->fass[dom_actions[n_actions]->n_fass] =
    (FChangeAtom*)current_atom;
  dom_actions[n_actions]->n_fass += 1;
;
    break;}
case 203:
#line 1077 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
;
    break;}
case 204:
#line 1081 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-2].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-2].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[n_actions]->incs[dom_actions[n_actions]->n_incs] =
    (FChangeAtom*)current_atom;
  dom_actions[n_actions]->n_incs += 1;
;
    break;}
case 205:
#line 1090 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
;
    break;}
case 206:
#line 1094 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-2].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-2].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[n_actions]->decs[dom_actions[n_actions]->n_decs] =
    (FChangeAtom*)current_atom;
  dom_actions[n_actions]->n_decs += 1;
;
    break;}
case 207:
#line 1103 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
;
    break;}
case 208:
#line 1107 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-2].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-2].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[n_actions]->fass[dom_actions[n_actions]->n_fass] =
    (FChangeAtom*)current_atom;
  dom_actions[n_actions]->n_fass += 1;
;
    break;}
case 216:
#line 1135 "pddl2.y"
{
  dom_actions[n_actions]->dmin = yyvsp[-1].exp;
  dom_actions[n_actions]->dmax = yyvsp[-1].exp;
;
    break;}
case 217:
#line 1140 "pddl2.y"
{
  dom_actions[n_actions]->dmin = yyvsp[0].exp;
  dom_actions[n_actions]->dmax = yyvsp[0].exp;
;
    break;}
case 218:
#line 1148 "pddl2.y"
{
  dom_actions[n_actions]->dmax = yyvsp[-1].exp;
;
    break;}
case 221:
#line 1160 "pddl2.y"
{
  dom_actions[n_actions]->dmin = yyvsp[-1].exp;
;
    break;}
case 224:
#line 1172 "pddl2.y"
{
  current_context = new SequentialTaskNet();
  stored_n_param = current_n_param;
  if (trace_print_context) {
    std::cerr << "pushed context (" << current_n_param << " variables)"
	      << std::endl;
  }
;
    break;}
case 225:
#line 1182 "pddl2.y"
{
  SequentialTaskNet* n = (SequentialTaskNet*)current_context;
  n->abs_act = dom_actions[n_actions];
  dom_actions[n_actions]->exps[dom_actions[n_actions]->n_exps] = n;
  dom_actions[n_actions]->n_exps += 1;
  if (trace_print_context) {
    std::cerr << "poping context from "
	      << current_n_param
	      << " to "
	      << stored_n_param
	      << " variables..." << std::endl;
  }
  clear_context(current_param, stored_n_param, current_n_param);
  current_n_param = stored_n_param;
  current_context = 0;
;
    break;}
case 228:
#line 1207 "pddl2.y"
{
  current_atom = new Reference((ActionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 229:
#line 1211 "pddl2.y"
{
  Reference* ref = (Reference*)current_atom;
  // ActionSymbol* act = (ActionSymbol*)$2->val;
  // act->refs[act->n_refs] = ref;
  // act->n_refs += 1;
  SequentialTaskNet* task_net = (SequentialTaskNet*)current_context;
  task_net->tasks[task_net->n_tasks] = ref;
  task_net->n_tasks += 1;
;
    break;}
case 231:
#line 1227 "pddl2.y"
{
  log_error("syntax error in problem definition.");
  yyerrok;
;
    break;}
case 232:
#line 1235 "pddl2.y"
{
  problem_name = yyvsp[-1].sym->text;
  if (current_file()) problem_file = strdup(current_file());
;
    break;}
case 248:
#line 1267 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 249:
#line 1271 "pddl2.y"
{
  PredicateSymbol* p = (PredicateSymbol*)yyvsp[-3].sym->val;
  if (p->n_param != current_atom->n_param) {
    log_error("wrong number of arguments for predicate in (:init ...");
  }
  insert_atom(p->init, (Atom*)current_atom);
  dom_init[n_init] = (Atom*)current_atom;
  dom_init[n_init]->at_time = 0;
  n_init += 1;
;
    break;}
case 250:
#line 1285 "pddl2.y"
{
  current_atom = new FInitAtom((FunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 251:
#line 1289 "pddl2.y"
{
  FunctionSymbol* f = (FunctionSymbol*)yyvsp[-5].sym->val;
  if (f->n_param != current_atom->n_param) {
    log_error("wrong number of arguments for function in (:init ...");
  }
  ((FInitAtom*)current_atom)->val = NN_TO_N(yyvsp[-1].rval);
  if (!INTEGRAL(((FInitAtom*)current_atom)->val))
    ((FInitAtom*)current_atom)->fun->integral = false;
  insert_atom(f->init, (FInitAtom*)current_atom);
  dom_fun_init[n_fun_init] = (FInitAtom*)current_atom;
  n_fun_init += 1;
;
    break;}
case 252:
#line 1302 "pddl2.y"
{
  FunctionSymbol* f = (FunctionSymbol*)yyvsp[-2].sym->val;
  current_atom = new FInitAtom((FunctionSymbol*)yyvsp[-2].sym->val);
  if (f->n_param != 0) {
    log_error("wrong number of arguments for function in (:init ...");
  }
  ((FInitAtom*)current_atom)->val = NN_TO_N(yyvsp[-1].rval);
  if (!INTEGRAL(((FInitAtom*)current_atom)->val))
    ((FInitAtom*)current_atom)->fun->integral = false;
  insert_atom(f->init, (FInitAtom*)current_atom);
  dom_fun_init[n_fun_init] = (FInitAtom*)current_atom;
  n_fun_init += 1;
;
    break;}
case 257:
#line 1326 "pddl2.y"
{
  dom_goals[n_goals] = yyvsp[0].goal;
  n_goals += 1;
;
    break;}
case 258:
#line 1331 "pddl2.y"
{
  Symbol* name = new Symbol(sym_preference, yyvsp[-2].sym->text);
  yyvsp[-2].sym->val = name;
  dom_preferences[n_preferences] = new Preference(name, yyvsp[-1].goal);
  n_preferences += 1;
;
    break;}
case 261:
#line 1346 "pddl2.y"
{
  yyval.goal = yyvsp[0].goal;
;
    break;}
case 262:
#line 1350 "pddl2.y"
{
  current_goal.append(new ConjunctiveGoal());
;
    break;}
case 263:
#line 1354 "pddl2.y"
{
  assert(current_goal.length() > 0);
  yyval.goal = current_goal[current_goal.length() - 1];
  current_goal.dec_length();
;
    break;}
case 264:
#line 1363 "pddl2.y"
{
  assert(current_goal.length() > 0);
  ConjunctiveGoal* cg = current_goal[current_goal.length() - 1];
  assert(cg != 0);
  cg->goals[cg->n_goals] = yyvsp[-1].goal;
  cg->n_goals += 1;
;
    break;}
case 266:
#line 1375 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 267:
#line 1379 "pddl2.y"
{
  yyval.goal = new AtomicGoal((Atom*)current_atom, false);
  ((Atom*)current_atom)->pred->pos_pre = true;
  insert_atom(((Atom*)current_atom)->pred->pos_goal, (Atom*)current_atom);
;
    break;}
case 268:
#line 1385 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 269:
#line 1389 "pddl2.y"
{
  yyval.goal = new AtomicGoal((Atom*)current_atom, true);
  ((Atom*)current_atom)->pred->neg_pre = true;
  insert_atom(((Atom*)current_atom)->pred->neg_goal, (Atom*)current_atom);
;
    break;}
case 270:
#line 1395 "pddl2.y"
{
  yyval.goal = new NumericGoal(yyvsp[0].rel);
;
    break;}
case 271:
#line 1399 "pddl2.y"
{
  yyval.goal = new SimpleSequenceGoal(goal_always, yyvsp[-1].goal);
;
    break;}
case 272:
#line 1403 "pddl2.y"
{
  yyval.goal = new SimpleSequenceGoal(goal_sometime, yyvsp[-1].goal);
;
    break;}
case 273:
#line 1407 "pddl2.y"
{
  yyval.goal = new SimpleSequenceGoal(goal_at_most_once, yyvsp[-1].goal);
;
    break;}
case 274:
#line 1411 "pddl2.y"
{
  yyval.goal = new TriggeredSequenceGoal(goal_sometime_before, yyvsp[-2].goal, yyvsp[-1].goal);
;
    break;}
case 275:
#line 1415 "pddl2.y"
{
  yyval.goal = new TriggeredSequenceGoal(goal_sometime_after, yyvsp[-2].goal, yyvsp[-1].goal);
;
    break;}
case 276:
#line 1419 "pddl2.y"
{
  yyval.goal = new DeadlineGoal(yyvsp[-2].rval, yyvsp[-1].goal);
;
    break;}
case 277:
#line 1423 "pddl2.y"
{
  yyval.goal = new TriggeredDeadlineGoal(yyvsp[-2].goal, yyvsp[-3].rval, yyvsp[-1].goal);
;
    break;}
case 279:
#line 1461 "pddl2.y"
{
  log_error("syntax error in metric declaration");
  yyerrok; 
;
    break;}
case 280:
#line 1469 "pddl2.y"
{
  if (original_metric_type != metric_none) {
    if (write_warnings) {
      std::cerr << "warning: multiple :metric expressions - overwriting previous definition" << std::endl;
    }
  }
  original_metric_type = metric_minimize;
;
    break;}
case 281:
#line 1478 "pddl2.y"
{
  if (original_metric_type != metric_none) {
    if (write_warnings) {
      std::cerr << "warning: multiple :metric expressions - overwriting previous definition" << std::endl;
    }
  }
  original_metric_type = metric_maximize;
;
    break;}
case 282:
#line 1490 "pddl2.y"
{
  if (yyvsp[0].exp->exp_class == exp_time) {
    original_metric = 0;
    original_metric_type = metric_makespan;
    yyval.exp = 0;
  }
  else {
    original_metric = yyvsp[0].exp;
    yyval.exp = yyvsp[0].exp;
  }
;
    break;}
case 283:
#line 1505 "pddl2.y"
{
  serial_length = I_TO_N(yyvsp[-1].ival);
;
    break;}
case 284:
#line 1509 "pddl2.y"
{
  parallel_length = I_TO_N(yyvsp[-1].ival);
;
    break;}
case 285:
#line 1515 "pddl2.y"
{ yyval.rval = N_TO_NN(yyvsp[0].ival); ;
    break;}
case 286:
#line 1516 "pddl2.y"
{ yyval.rval = yyvsp[0].rval; ;
    break;}
case 287:
#line 1517 "pddl2.y"
{ yyval.rval = POS_INF; ;
    break;}
case 288:
#line 1524 "pddl2.y"
{
  current_item = new SetConstraint();
  current_context = current_item;
;
    break;}
case 289:
#line 1529 "pddl2.y"
{
  dom_invariants[n_invariants] = (SetConstraint*)current_item;
;
    break;}
case 290:
#line 1534 "pddl2.y"
{
  dom_invariants[n_invariants]->sc_type = yyvsp[-4].sckw;
  dom_invariants[n_invariants]->sc_count = yyvsp[-3].ival;
  n_invariants += 1;
  clear_context(current_param, current_n_param);
  current_context = 0;
;
    break;}
case 291:
#line 1542 "pddl2.y"
{
  log_error("syntax error in invariant declaration");
  yyerrok;
;
    break;}
case 292:
#line 1550 "pddl2.y"
{
  current_item = new IrrelevantItem();
  if (problem_name) current_item->defined_in_problem = true;
  current_context = current_item;
;
    break;}
case 294:
#line 1557 "pddl2.y"
{
  log_error("syntax error in irrelevant declaration");
  yyerrok;
;
    break;}
case 297:
#line 1570 "pddl2.y"
{
  current_atom = new Reference((ActionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 298:
#line 1574 "pddl2.y"
{
  IrrelevantItem* item = (IrrelevantItem*)current_item;
  item->entity = (Reference*)current_atom;
  dom_irrelevant[n_irrelevant] = item;
  n_irrelevant += 1;
  ActionSymbol* act = (ActionSymbol*)yyvsp[-4].sym->val;
  act->irr_ins[act->n_irr_ins] = item;
  act->n_irr_ins += 1;
  clear_context(current_param, current_n_param);
  current_context = 0;
;
    break;}
case 299:
#line 1589 "pddl2.y"
{
  current_atom = new Reference((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 300:
#line 1593 "pddl2.y"
{
  IrrelevantItem* item = (IrrelevantItem*)current_item;
  item->entity = (Reference*)current_atom;
  dom_irrelevant[n_irrelevant] = item;
  n_irrelevant += 1;
  PredicateSymbol* pred = (PredicateSymbol*)yyvsp[-4].sym->val;
  pred->irr_ins[pred->n_irr_ins] = item;
  pred->n_irr_ins += 1;
  current_context = 0;
  clear_context(current_param, current_n_param);
;
    break;}
case 301:
#line 1608 "pddl2.y"
{
  current_item = new SetConstraint();
  current_item->defined_in_problem = true;
  current_context = current_item;
;
    break;}
case 302:
#line 1614 "pddl2.y"
{
  dom_invariants[n_invariants] = (SetConstraint*)current_item;
;
    break;}
case 303:
#line 1619 "pddl2.y"
{
  dom_invariants[n_invariants]->sc_type = yyvsp[-4].sckw;
  dom_invariants[n_invariants]->sc_count = yyvsp[-3].ival;
  n_invariants += 1;
  current_context = 0;
;
    break;}
case 304:
#line 1626 "pddl2.y"
{
  log_error("syntax error in invariant declaration");
  yyerrok;
;
    break;}
case 310:
#line 1642 "pddl2.y"
{
  current_item->item_tags.insert(yyvsp[0].sym->text);
;
    break;}
case 311:
#line 1649 "pddl2.y"
{
  yyvsp[0].sym->val = new Symbol(sym_misc, yyvsp[0].sym->text);
  current_item->name = (Symbol*)yyvsp[0].sym->val;
  current_item->item_tags.insert(yyvsp[0].sym->text);
;
    break;}
case 312:
#line 1655 "pddl2.y"
{
  current_item->name = (Symbol*)yyvsp[0].sym->val;
  current_item->item_tags.insert(yyvsp[0].sym->text);
;
    break;}
case 313:
#line 1663 "pddl2.y"
{
  current_n_param = 0;
;
    break;}
case 314:
#line 1667 "pddl2.y"
{
  for (index_type k = 0; k < current_n_param; k++)
    current_context->param[k] = current_param[k];
  current_context->n_param = current_n_param;
;
    break;}
case 317:
#line 1681 "pddl2.y"
{
  for (index_type k = stored_n_param; k < current_n_param; k++) {
    current_context->param[current_context->n_param] = current_param[k];
    current_context->n_param += 1;
  }
;
    break;}
case 342:
#line 1733 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 343:
#line 1737 "pddl2.y"
{
  current_context->pos_con[current_context->n_pos_con] = (Atom*)current_atom;
  current_context->n_pos_con += 1;
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 344:
#line 1743 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_init);
;
    break;}
case 345:
#line 1747 "pddl2.y"
{
  current_context->pos_con[current_context->n_pos_con] = (Atom*)current_atom;
  current_context->n_pos_con += 1;
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 346:
#line 1753 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param[0] = (Symbol*)yyvsp[-2].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-1].sym->val;
  eq_atom->n_param = 2;
  current_context->pos_con[current_context->n_pos_con] = eq_atom;
  current_context->n_pos_con += 1;
;
    break;}
case 347:
#line 1765 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 348:
#line 1769 "pddl2.y"
{
  current_context->neg_con[current_context->n_neg_con] = (Atom*)current_atom;
  current_context->n_neg_con += 1;
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 349:
#line 1775 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_init);
;
    break;}
case 350:
#line 1779 "pddl2.y"
{
  current_context->neg_con[current_context->n_neg_con] = (Atom*)current_atom;
  current_context->n_neg_con += 1;
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 351:
#line 1785 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_init);
;
    break;}
case 352:
#line 1789 "pddl2.y"
{
  current_context->neg_con[current_context->n_neg_con] = (Atom*)current_atom;
  current_context->n_neg_con += 1;
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 353:
#line 1795 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param[0] = (Symbol*)yyvsp[-3].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-2].sym->val;
  eq_atom->n_param = 2;
  current_context->neg_con[current_context->n_neg_con] = eq_atom;
  current_context->n_neg_con += 1;
;
    break;}
case 354:
#line 1807 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_pos_goal);
;
    break;}
case 355:
#line 1811 "pddl2.y"
{
  current_context->pos_con[current_context->n_pos_con] = (Atom*)current_atom;
  current_context->n_pos_con += 1;
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 356:
#line 1817 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_neg_goal);
;
    break;}
case 357:
#line 1821 "pddl2.y"
{
  current_context->pos_con[current_context->n_pos_con] = (Atom*)current_atom;
  current_context->n_pos_con += 1;
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 358:
#line 1830 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_pos_goal);
;
    break;}
case 359:
#line 1834 "pddl2.y"
{
  current_context->neg_con[current_context->n_neg_con] = (Atom*)current_atom;
  current_context->n_neg_con += 1;
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 360:
#line 1840 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_neg_goal);
;
    break;}
case 361:
#line 1844 "pddl2.y"
{
  current_context->neg_con[current_context->n_neg_con] = (Atom*)current_atom;
  current_context->n_neg_con += 1;
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 362:
#line 1853 "pddl2.y"
{
  TypeConstraint* c =
    new TypeConstraint((VariableSymbol*)yyvsp[-2].sym->val, (TypeSymbol*)yyvsp[-1].sym->val);
  current_context->type_con[current_context->n_type_con] = c;
  current_context->n_type_con += 1;
;
    break;}
case 363:
#line 1862 "pddl2.y"
{
  yyval.sckw = sc_at_least;
;
    break;}
case 364:
#line 1866 "pddl2.y"
{
  yyval.sckw = sc_at_most;
;
    break;}
case 365:
#line 1870 "pddl2.y"
{
  yyval.sckw = sc_exactly;
;
    break;}
case 369:
#line 1882 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 370:
#line 1886 "pddl2.y"
{
  dom_invariants[n_invariants]->
    pos_atoms[dom_invariants[n_invariants]->n_pos_atoms]
    = (Atom*)current_atom;
  dom_invariants[n_invariants]->n_pos_atoms += 1;
;
    break;}
case 371:
#line 1893 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 372:
#line 1897 "pddl2.y"
{
  dom_invariants[n_invariants]->
    neg_atoms[dom_invariants[n_invariants]->n_neg_atoms]
    = (Atom*)current_atom;
  dom_invariants[n_invariants]->n_neg_atoms += 1;
;
    break;}
case 373:
#line 1907 "pddl2.y"
{
  stored_n_param = current_n_param;
  current_context = new SetOf();
;
    break;}
case 374:
#line 1912 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (index_type k = stored_n_param; k < current_n_param; k++) {
    s->param[s->n_param] = current_param[k];
    s->n_param += 1;
  }
;
    break;}
case 375:
#line 1920 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 376:
#line 1924 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->atom = (Atom*)current_atom;
  dom_invariants[n_invariants]->
    atom_sets[dom_invariants[n_invariants]->n_atom_sets] = s;
  dom_invariants[n_invariants]->n_atom_sets += 1;
  clear_context(current_param, stored_n_param, current_n_param);
  current_n_param = stored_n_param;
  current_context = 0;
;
    break;}
case 377:
#line 1938 "pddl2.y"
{
  input_plans[input_plans.length()] = new InputPlan();
;
    break;}
case 378:
#line 1942 "pddl2.y"
{
  input_plans.inc_length();
;
    break;}
case 379:
#line 1946 "pddl2.y"
{
  log_error("syntax error in plan");
  yyerrok;
;
    break;}
case 381:
#line 1955 "pddl2.y"
{
  input_plans[input_plans.length()]->is_opt = true;
;
    break;}
case 384:
#line 1964 "pddl2.y"
{
  input_plans[input_plans.length()]->name = new Symbol(yyvsp[0].sym->text);
;
    break;}
case 385:
#line 1971 "pddl2.y"
{
  current_atom = new Reference((ActionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 386:
#line 1975 "pddl2.y"
{
  Reference* ref = (Reference*)current_atom;
  ActionSymbol* act = (ActionSymbol*)yyvsp[-3].sym->val;
  act->refs[act->n_refs] = ref;
  act->n_refs += 1;
  input_plans[input_plans.length()]->steps.append(new InputPlanStep(ref, yyvsp[-6].rval));
  clear_context(current_param, current_n_param);
;
    break;}
case 387:
#line 1987 "pddl2.y"
{
  input_plans.inc_length();
  current_plan_file = 0;
;
    break;}
case 388:
#line 1992 "pddl2.y"
{
  input_plans.inc_length();
  current_plan_file = 0;
;
    break;}
case 391:
#line 2005 "pddl2.y"
{
  current_atom = new Reference((ActionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 392:
#line 2009 "pddl2.y"
{
  Reference* ref = (Reference*)current_atom;
  ActionSymbol* act = (ActionSymbol*)yyvsp[-4].sym->val;
  act->refs[act->n_refs] = ref;
  act->n_refs += 1;
  if (input_plans[input_plans.length()] == 0) {
    at_position(std::cerr) << "beginning of new plan" << std::endl;
    input_plans[input_plans.length()] = new InputPlan();
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length()]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  else if (current_file() != current_plan_file) {
    at_position(std::cerr) << "beginning of new plan (new file)" << std::endl;
    input_plans.inc_length();
    input_plans[input_plans.length()] = new InputPlan();
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length()]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  input_plans[input_plans.length()]->steps.append(new InputPlanStep(ref, yyvsp[-7].rval));
  clear_context(current_param, current_n_param);
;
    break;}
case 397:
#line 2050 "pddl2.y"
{
  current_atom = new Reference((ActionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 398:
#line 2054 "pddl2.y"
{
  Reference* ref = (Reference*)current_atom;
  ActionSymbol* act = (ActionSymbol*)yyvsp[-3].sym->val;
  act->refs[act->n_refs] = ref;
  act->n_refs += 1;
  if (input_plans[input_plans.length()] == 0) {
    at_position(std::cerr) << "beginning of new plan" << std::endl;
    input_plans[input_plans.length()] = new InputPlan();
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length()]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  else if (current_file() != current_plan_file) {
    at_position(std::cerr) << "beginning of new plan (new file)" << std::endl;
    input_plans.inc_length();
    input_plans[input_plans.length()] = new InputPlan();
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length()]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  index_type n = input_plans[input_plans.length()]->steps.length();
  input_plans[input_plans.length()]->steps.append(new InputPlanStep(ref, n));
  clear_context(current_param, current_n_param);
;
    break;}
case 402:
#line 2095 "pddl2.y"
{
  current_entry = new HTableEntry();
;
    break;}
case 403:
#line 2099 "pddl2.y"
{
  h_table.append(current_entry);
  current_entry = 0;
;
    break;}
case 404:
#line 2107 "pddl2.y"
{
  current_entry->cost = yyvsp[0].rval;
  current_entry->opt = true;
;
    break;}
case 405:
#line 2112 "pddl2.y"
{
  current_entry->cost = yyvsp[0].rval;
;
    break;}
case 410:
#line 2129 "pddl2.y"
{
  assert(current_entry);
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 411:
#line 2134 "pddl2.y"
{
  current_entry->atoms[current_entry->n_atoms] = (Atom*)current_atom;
  current_entry->neg[current_entry->n_atoms] = false;
  current_entry->n_atoms += 1;
;
    break;}
case 412:
#line 2143 "pddl2.y"
{
  assert(current_entry);
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 413:
#line 2148 "pddl2.y"
{
  current_entry->atoms[current_entry->n_atoms] = (Atom*)current_atom;
  current_entry->neg[current_entry->n_atoms] = true;
  current_entry->n_atoms += 1;
;
    break;}
case 414:
#line 2157 "pddl2.y"
{
  ReferenceSet* set = new ReferenceSet();
  current_context = set;
  stored_n_param = 0;
  current_n_param = 0;
  input_sets.append(set);
;
    break;}
case 415:
#line 2165 "pddl2.y"
{
  current_context = 0;
  clear_context(current_param, current_n_param);
;
    break;}
case 416:
#line 2173 "pddl2.y"
{
  yyvsp[0].sym->val = new Symbol(sym_misc, yyvsp[0].sym->text);
  ((ReferenceSet*)current_context)->name = (Symbol*)yyvsp[0].sym->val;
;
    break;}
case 417:
#line 2178 "pddl2.y"
{
  ((ReferenceSet*)current_context)->name = (Symbol*)yyvsp[0].sym->val;
;
    break;}
case 422:
#line 2192 "pddl2.y"
{
  if (yyvsp[0].sym->val) {
    current_atom = new Reference((Symbol*)yyvsp[0].sym->val);
  }
  else {
    current_atom = new Reference(new Symbol(sym_misc, yyvsp[0].sym->text));
  }
;
    break;}
case 423:
#line 2201 "pddl2.y"
{
  assert(input_sets.length() > 0);
  assert(input_sets[input_sets.length() - 1] != 0);
  input_sets[input_sets.length() - 1]->
    add(new SimpleReferenceSet((Reference*)current_atom));
;
    break;}
case 424:
#line 2210 "pddl2.y"
{
  stored_n_param = current_n_param;
  stored_context = current_context;
  current_context = new SimpleReferenceSet(0);
;
    break;}
case 425:
#line 2216 "pddl2.y"
{
  if (yyvsp[0].sym->val) {
    current_atom = new Reference((Symbol*)yyvsp[0].sym->val);
  }
  else {
    current_atom = new Reference(new Symbol(sym_misc, yyvsp[0].sym->text));
  }
;
    break;}
case 426:
#line 2225 "pddl2.y"
{
  SimpleReferenceSet* s = (SimpleReferenceSet*)current_context;
  s->ref = (Reference*)current_atom;
  assert(input_sets.length() > 0);
  assert(input_sets[input_sets.length() - 1] != 0);
  input_sets[input_sets.length() - 1]->add(s);
  clear_context(current_param, stored_n_param, current_n_param);
  current_n_param = stored_n_param;
  current_context = stored_context;
;
    break;}
}

#line 811 "/lib/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_PDDL_Parser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_PDDL_Parser_DEBUG != 0
  if (YY_PDDL_Parser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_PDDL_Parser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_PDDL_Parser_LLOC.first_line;
      yylsp->first_column = YY_PDDL_Parser_LLOC.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  YYGOTO(yynewstate);

YYLABEL(yyerrlab)   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++YY_PDDL_Parser_NERRS;

#ifdef YY_PDDL_Parser_ERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      YY_PDDL_Parser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_PDDL_Parser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_PDDL_Parser_ERROR_VERBOSE */
	YY_PDDL_Parser_ERROR("parse error");
    }

  YYGOTO(yyerrlab1);
YYLABEL(yyerrlab1)   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_PDDL_Parser_CHAR == YYEOF)
	YYABORT;

#if YY_PDDL_Parser_DEBUG != 0
      if (YY_PDDL_Parser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_PDDL_Parser_CHAR, yytname[yychar1]);
#endif

      YY_PDDL_Parser_CHAR = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;              /* Each real token shifted decrements this */

  YYGOTO(yyerrhandle);

YYLABEL(yyerrdefault)  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) YYGOTO(yydefault);
#endif

YYLABEL(yyerrpop)   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YY_PDDL_Parser_LSP_NEEDED
  yylsp--;
#endif

#if YY_PDDL_Parser_DEBUG != 0
  if (YY_PDDL_Parser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

YYLABEL(yyerrhandle)

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    YYGOTO(yyerrdefault);

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    YYGOTO(yyerrdefault);

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	YYGOTO(yyerrpop);
      yyn = -yyn;
      YYGOTO(yyreduce);
    }
  else if (yyn == 0)
    YYGOTO(yyerrpop);

  if (yyn == YYFINAL)
    YYACCEPT;

#if YY_PDDL_Parser_DEBUG != 0
  if (YY_PDDL_Parser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_PDDL_Parser_LVAL;
#ifdef YY_PDDL_Parser_LSP_NEEDED
  *++yylsp = YY_PDDL_Parser_LLOC;
#endif

  yystate = yyn;
  YYGOTO(yynewstate);
/* end loop, in which YYGOTO may be used. */
  YYENDGOTO
}

/* END */

/* #line 1010 "/lib/bison.cc" */
#line 4622 "grammar.cc"
#line 2236 "pddl2.y"

