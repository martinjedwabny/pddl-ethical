#define YY_PDDL_Parser_h_included

/*  A Bison++ parser, made from pddl2.y  */

 /* with Bison++ version bison++ Version 1.21-8, adapted from GNU bison by coetmeur@icdc.fr
  */


#line 1 "/usr/local/lib/bison.cc"
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

/* #line 73 "/usr/local/lib/bison.cc" */
#line 85 "grammar.cc"
#define YY_PDDL_Parser_ERROR  log_error
#define YY_PDDL_Parser_ERROR_BODY  = 0
#define YY_PDDL_Parser_ERROR_VERBOSE  1
#define YY_PDDL_Parser_LEX  next_token
#define YY_PDDL_Parser_LEX_BODY  = 0
#define YY_PDDL_Parser_DEBUG  1
#define YY_PDDL_Parser_INHERIT  : public HSPS::PDDL_Base
#define YY_PDDL_Parser_CONSTRUCTOR_PARAM  HSPS::StringTable& t
#define YY_PDDL_Parser_CONSTRUCTOR_INIT  : HSPS::PDDL_Base(t), error_flag(false), \
  current_param(0, 0), stored_n_param(0, 0), current_atom(0), \
  current_atom_stack(0, 0), current_context(0), stored_context(0, 0), \
  current_item(0), current_goal(0, 0), current_preference_name(0), \
  current_entry(0), current_plan_file(0), n_plans_in_current_file(0)
#define YY_PDDL_Parser_MEMBERS  \
public: \
virtual std::ostream& at_position(std::ostream& s) = 0; \
virtual char*         current_file() = 0; \
bool                  error_flag; \
private: \
HSPS::PDDL_Base::variable_vec current_param; \
HSPS::index_vec               stored_n_param; \
HSPS::PDDL_Base::TypeSet      current_type_set; \
HSPS::index_type              last_n_functions; \
HSPS::PDDL_Base::AtomBase*    current_atom; \
HSPS::PDDL_Base::atom_base_vec current_atom_stack; \
HSPS::PDDL_Base::Context*     current_context; \
HSPS::lvector<Context*> stored_context; \
HSPS::PDDL_Base::DKEL_Item*   current_item; \
HSPS::lvector<ConjunctiveGoal*> current_goal; \
HSPS::PDDL_Base::Symbol*      current_preference_name; \
HSPS::PDDL_Base::HTableEntry* current_entry; \
char* current_plan_file; \
HSPS::index_type n_plans_in_current_file;
#line 37 "pddl2.y"

#include <stdlib.h>
#include "base.h"
#include <sstream>

#line 43 "pddl2.y"
typedef union {
  HSPS::StringTable::Cell*         sym;
  HSPS::PDDL_Base::Expression*     exp;
  HSPS::PDDL_Base::ListExpression* lst;
  HSPS::PDDL_Base::Relation*       rel;
  HSPS::PDDL_Base::Goal*           goal;
  HSPS::PDDL_Base::Formula*        ff;
  HSPS::PDDL_Base::mode_keyword    tkw;
  HSPS::PDDL_Base::relation_type   rkw;
  HSPS::PDDL_Base::set_constraint_keyword sckw;
  NNTYPE rval;
  int    ival;
  char*  sval;
} yy_PDDL_Parser_stype;
#define YY_PDDL_Parser_STYPE yy_PDDL_Parser_stype

#line 73 "/usr/local/lib/bison.cc"
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

/* #line 117 "/usr/local/lib/bison.cc" */
#line 189 "grammar.cc"

#line 117 "/usr/local/lib/bison.cc"
/*  YY_PDDL_Parser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 121 "/usr/local/lib/bison.cc" */
#line 198 "grammar.cc"

#line 121 "/usr/local/lib/bison.cc"
/* prefix */
#ifndef YY_PDDL_Parser_DEBUG

/* #line 123 "/usr/local/lib/bison.cc" */
#line 205 "grammar.cc"

#line 123 "/usr/local/lib/bison.cc"
/* YY_PDDL_Parser_DEBUG */
#endif


#ifndef YY_PDDL_Parser_LSP_NEEDED

/* #line 128 "/usr/local/lib/bison.cc" */
#line 215 "grammar.cc"

#line 128 "/usr/local/lib/bison.cc"
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

/* #line 236 "/usr/local/lib/bison.cc" */
#line 328 "grammar.cc"
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
#define	TK_OBJFUN_SYMBOL	278
#define	TK_FUN_SYMBOL	279
#define	TK_VAR_SYMBOL	280
#define	TK_ACTION_SYMBOL	281
#define	TK_MISC_SYMBOL	282
#define	TK_KEYWORD	283
#define	TK_NEW_VAR_SYMBOL	284
#define	TK_PREFERENCE_SYMBOL	285
#define	TK_SET_SYMBOL	286
#define	TK_FLOAT	287
#define	TK_INT	288
#define	TK_STRING	289
#define	KW_REQS	290
#define	KW_CONSTANTS	291
#define	KW_PREDS	292
#define	KW_FUNS	293
#define	KW_TYPES	294
#define	KW_DEFINE	295
#define	KW_DOMAIN	296
#define	KW_ACTION	297
#define	KW_PROCESS	298
#define	KW_EVENT	299
#define	KW_ARGS	300
#define	KW_PRE	301
#define	KW_COND	302
#define	KW_AT_START	303
#define	KW_AT_END	304
#define	KW_OVER_ALL	305
#define	KW_EFFECT	306
#define	KW_INVARIANT	307
#define	KW_DURATION	308
#define	KW_AND	309
#define	KW_OR	310
#define	KW_EXISTS	311
#define	KW_FORALL	312
#define	KW_IMPLY	313
#define	KW_NOT	314
#define	KW_WHEN	315
#define	KW_EITHER	316
#define	KW_PROBLEM	317
#define	KW_FORDOMAIN	318
#define	KW_OBJECTS	319
#define	KW_INIT	320
#define	KW_GOAL	321
#define	KW_LENGTH	322
#define	KW_SERIAL	323
#define	KW_PARALLEL	324
#define	KW_METRIC	325
#define	KW_MINIMIZE	326
#define	KW_MAXIMIZE	327
#define	KW_DURATION_VAR	328
#define	KW_TOTAL_TIME	329
#define	KW_INCREASE	330
#define	KW_DECREASE	331
#define	KW_SCALE_UP	332
#define	KW_SCALE_DOWN	333
#define	KW_ASSIGN	334
#define	KW_TAG	335
#define	KW_NAME	336
#define	KW_VARS	337
#define	KW_SET_CONSTRAINT	338
#define	KW_SETOF	339
#define	KW_AT_LEAST_N	340
#define	KW_AT_MOST_N	341
#define	KW_EXACTLY_N	342
#define	KW_CONTEXT	343
#define	KW_FORMULA	344
#define	KW_IRRELEVANT	345
#define	KW_PLAN	346
#define	KW_HEURISTIC	347
#define	KW_OPT	348
#define	KW_INF	349
#define	KW_FACT	350
#define	KW_SET	351
#define	KW_EXPANSION	352
#define	KW_TASKS	353
#define	KW_PREFERENCE	354
#define	KW_VIOLATED	355
#define	KW_WITHIN	356
#define	KW_ASSOC	357
#define	KW_CONSTRAINTS	358
#define	KW_ALWAYS	359
#define	KW_SOMETIME	360
#define	KW_AT_MOST_ONCE	361
#define	KW_SOMETIME_BEFORE	362
#define	KW_SOMETIME_AFTER	363
#define	KW_ALWAYS_WITHIN	364
#define	KW_IFF	365
#define	KW_FALSE	366
#define	KW_TRUE	367
#define	KW_NUMBER	368
#define	KW_UNDEFINED	369


#line 236 "/usr/local/lib/bison.cc"
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

/* #line 280 "/usr/local/lib/bison.cc" */
#line 490 "grammar.cc"
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
static const int TK_OBJFUN_SYMBOL;
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
static const int KW_AT_START;
static const int KW_AT_END;
static const int KW_OVER_ALL;
static const int KW_EFFECT;
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
static const int KW_FORMULA;
static const int KW_IRRELEVANT;
static const int KW_PLAN;
static const int KW_HEURISTIC;
static const int KW_OPT;
static const int KW_INF;
static const int KW_FACT;
static const int KW_SET;
static const int KW_EXPANSION;
static const int KW_TASKS;
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
static const int KW_IFF;
static const int KW_FALSE;
static const int KW_TRUE;
static const int KW_NUMBER;
static const int KW_UNDEFINED;


#line 280 "/usr/local/lib/bison.cc"
 /* decl const */
#else
enum YY_PDDL_Parser_ENUM_TOKEN { YY_PDDL_Parser_NULL_TOKEN=0

/* #line 283 "/usr/local/lib/bison.cc" */
#line 611 "grammar.cc"
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
	,TK_OBJFUN_SYMBOL=278
	,TK_FUN_SYMBOL=279
	,TK_VAR_SYMBOL=280
	,TK_ACTION_SYMBOL=281
	,TK_MISC_SYMBOL=282
	,TK_KEYWORD=283
	,TK_NEW_VAR_SYMBOL=284
	,TK_PREFERENCE_SYMBOL=285
	,TK_SET_SYMBOL=286
	,TK_FLOAT=287
	,TK_INT=288
	,TK_STRING=289
	,KW_REQS=290
	,KW_CONSTANTS=291
	,KW_PREDS=292
	,KW_FUNS=293
	,KW_TYPES=294
	,KW_DEFINE=295
	,KW_DOMAIN=296
	,KW_ACTION=297
	,KW_PROCESS=298
	,KW_EVENT=299
	,KW_ARGS=300
	,KW_PRE=301
	,KW_COND=302
	,KW_AT_START=303
	,KW_AT_END=304
	,KW_OVER_ALL=305
	,KW_EFFECT=306
	,KW_INVARIANT=307
	,KW_DURATION=308
	,KW_AND=309
	,KW_OR=310
	,KW_EXISTS=311
	,KW_FORALL=312
	,KW_IMPLY=313
	,KW_NOT=314
	,KW_WHEN=315
	,KW_EITHER=316
	,KW_PROBLEM=317
	,KW_FORDOMAIN=318
	,KW_OBJECTS=319
	,KW_INIT=320
	,KW_GOAL=321
	,KW_LENGTH=322
	,KW_SERIAL=323
	,KW_PARALLEL=324
	,KW_METRIC=325
	,KW_MINIMIZE=326
	,KW_MAXIMIZE=327
	,KW_DURATION_VAR=328
	,KW_TOTAL_TIME=329
	,KW_INCREASE=330
	,KW_DECREASE=331
	,KW_SCALE_UP=332
	,KW_SCALE_DOWN=333
	,KW_ASSIGN=334
	,KW_TAG=335
	,KW_NAME=336
	,KW_VARS=337
	,KW_SET_CONSTRAINT=338
	,KW_SETOF=339
	,KW_AT_LEAST_N=340
	,KW_AT_MOST_N=341
	,KW_EXACTLY_N=342
	,KW_CONTEXT=343
	,KW_FORMULA=344
	,KW_IRRELEVANT=345
	,KW_PLAN=346
	,KW_HEURISTIC=347
	,KW_OPT=348
	,KW_INF=349
	,KW_FACT=350
	,KW_SET=351
	,KW_EXPANSION=352
	,KW_TASKS=353
	,KW_PREFERENCE=354
	,KW_VIOLATED=355
	,KW_WITHIN=356
	,KW_ASSOC=357
	,KW_CONSTRAINTS=358
	,KW_ALWAYS=359
	,KW_SOMETIME=360
	,KW_AT_MOST_ONCE=361
	,KW_SOMETIME_BEFORE=362
	,KW_SOMETIME_AFTER=363
	,KW_ALWAYS_WITHIN=364
	,KW_IFF=365
	,KW_FALSE=366
	,KW_TRUE=367
	,KW_NUMBER=368
	,KW_UNDEFINED=369


#line 283 "/usr/local/lib/bison.cc"
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

/* #line 314 "/usr/local/lib/bison.cc" */
#line 760 "grammar.cc"
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
const int YY_PDDL_Parser_CLASS::TK_OBJFUN_SYMBOL=278;
const int YY_PDDL_Parser_CLASS::TK_FUN_SYMBOL=279;
const int YY_PDDL_Parser_CLASS::TK_VAR_SYMBOL=280;
const int YY_PDDL_Parser_CLASS::TK_ACTION_SYMBOL=281;
const int YY_PDDL_Parser_CLASS::TK_MISC_SYMBOL=282;
const int YY_PDDL_Parser_CLASS::TK_KEYWORD=283;
const int YY_PDDL_Parser_CLASS::TK_NEW_VAR_SYMBOL=284;
const int YY_PDDL_Parser_CLASS::TK_PREFERENCE_SYMBOL=285;
const int YY_PDDL_Parser_CLASS::TK_SET_SYMBOL=286;
const int YY_PDDL_Parser_CLASS::TK_FLOAT=287;
const int YY_PDDL_Parser_CLASS::TK_INT=288;
const int YY_PDDL_Parser_CLASS::TK_STRING=289;
const int YY_PDDL_Parser_CLASS::KW_REQS=290;
const int YY_PDDL_Parser_CLASS::KW_CONSTANTS=291;
const int YY_PDDL_Parser_CLASS::KW_PREDS=292;
const int YY_PDDL_Parser_CLASS::KW_FUNS=293;
const int YY_PDDL_Parser_CLASS::KW_TYPES=294;
const int YY_PDDL_Parser_CLASS::KW_DEFINE=295;
const int YY_PDDL_Parser_CLASS::KW_DOMAIN=296;
const int YY_PDDL_Parser_CLASS::KW_ACTION=297;
const int YY_PDDL_Parser_CLASS::KW_PROCESS=298;
const int YY_PDDL_Parser_CLASS::KW_EVENT=299;
const int YY_PDDL_Parser_CLASS::KW_ARGS=300;
const int YY_PDDL_Parser_CLASS::KW_PRE=301;
const int YY_PDDL_Parser_CLASS::KW_COND=302;
const int YY_PDDL_Parser_CLASS::KW_AT_START=303;
const int YY_PDDL_Parser_CLASS::KW_AT_END=304;
const int YY_PDDL_Parser_CLASS::KW_OVER_ALL=305;
const int YY_PDDL_Parser_CLASS::KW_EFFECT=306;
const int YY_PDDL_Parser_CLASS::KW_INVARIANT=307;
const int YY_PDDL_Parser_CLASS::KW_DURATION=308;
const int YY_PDDL_Parser_CLASS::KW_AND=309;
const int YY_PDDL_Parser_CLASS::KW_OR=310;
const int YY_PDDL_Parser_CLASS::KW_EXISTS=311;
const int YY_PDDL_Parser_CLASS::KW_FORALL=312;
const int YY_PDDL_Parser_CLASS::KW_IMPLY=313;
const int YY_PDDL_Parser_CLASS::KW_NOT=314;
const int YY_PDDL_Parser_CLASS::KW_WHEN=315;
const int YY_PDDL_Parser_CLASS::KW_EITHER=316;
const int YY_PDDL_Parser_CLASS::KW_PROBLEM=317;
const int YY_PDDL_Parser_CLASS::KW_FORDOMAIN=318;
const int YY_PDDL_Parser_CLASS::KW_OBJECTS=319;
const int YY_PDDL_Parser_CLASS::KW_INIT=320;
const int YY_PDDL_Parser_CLASS::KW_GOAL=321;
const int YY_PDDL_Parser_CLASS::KW_LENGTH=322;
const int YY_PDDL_Parser_CLASS::KW_SERIAL=323;
const int YY_PDDL_Parser_CLASS::KW_PARALLEL=324;
const int YY_PDDL_Parser_CLASS::KW_METRIC=325;
const int YY_PDDL_Parser_CLASS::KW_MINIMIZE=326;
const int YY_PDDL_Parser_CLASS::KW_MAXIMIZE=327;
const int YY_PDDL_Parser_CLASS::KW_DURATION_VAR=328;
const int YY_PDDL_Parser_CLASS::KW_TOTAL_TIME=329;
const int YY_PDDL_Parser_CLASS::KW_INCREASE=330;
const int YY_PDDL_Parser_CLASS::KW_DECREASE=331;
const int YY_PDDL_Parser_CLASS::KW_SCALE_UP=332;
const int YY_PDDL_Parser_CLASS::KW_SCALE_DOWN=333;
const int YY_PDDL_Parser_CLASS::KW_ASSIGN=334;
const int YY_PDDL_Parser_CLASS::KW_TAG=335;
const int YY_PDDL_Parser_CLASS::KW_NAME=336;
const int YY_PDDL_Parser_CLASS::KW_VARS=337;
const int YY_PDDL_Parser_CLASS::KW_SET_CONSTRAINT=338;
const int YY_PDDL_Parser_CLASS::KW_SETOF=339;
const int YY_PDDL_Parser_CLASS::KW_AT_LEAST_N=340;
const int YY_PDDL_Parser_CLASS::KW_AT_MOST_N=341;
const int YY_PDDL_Parser_CLASS::KW_EXACTLY_N=342;
const int YY_PDDL_Parser_CLASS::KW_CONTEXT=343;
const int YY_PDDL_Parser_CLASS::KW_FORMULA=344;
const int YY_PDDL_Parser_CLASS::KW_IRRELEVANT=345;
const int YY_PDDL_Parser_CLASS::KW_PLAN=346;
const int YY_PDDL_Parser_CLASS::KW_HEURISTIC=347;
const int YY_PDDL_Parser_CLASS::KW_OPT=348;
const int YY_PDDL_Parser_CLASS::KW_INF=349;
const int YY_PDDL_Parser_CLASS::KW_FACT=350;
const int YY_PDDL_Parser_CLASS::KW_SET=351;
const int YY_PDDL_Parser_CLASS::KW_EXPANSION=352;
const int YY_PDDL_Parser_CLASS::KW_TASKS=353;
const int YY_PDDL_Parser_CLASS::KW_PREFERENCE=354;
const int YY_PDDL_Parser_CLASS::KW_VIOLATED=355;
const int YY_PDDL_Parser_CLASS::KW_WITHIN=356;
const int YY_PDDL_Parser_CLASS::KW_ASSOC=357;
const int YY_PDDL_Parser_CLASS::KW_CONSTRAINTS=358;
const int YY_PDDL_Parser_CLASS::KW_ALWAYS=359;
const int YY_PDDL_Parser_CLASS::KW_SOMETIME=360;
const int YY_PDDL_Parser_CLASS::KW_AT_MOST_ONCE=361;
const int YY_PDDL_Parser_CLASS::KW_SOMETIME_BEFORE=362;
const int YY_PDDL_Parser_CLASS::KW_SOMETIME_AFTER=363;
const int YY_PDDL_Parser_CLASS::KW_ALWAYS_WITHIN=364;
const int YY_PDDL_Parser_CLASS::KW_IFF=365;
const int YY_PDDL_Parser_CLASS::KW_FALSE=366;
const int YY_PDDL_Parser_CLASS::KW_TRUE=367;
const int YY_PDDL_Parser_CLASS::KW_NUMBER=368;
const int YY_PDDL_Parser_CLASS::KW_UNDEFINED=369;


#line 314 "/usr/local/lib/bison.cc"
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

/* #line 325 "/usr/local/lib/bison.cc" */
#line 889 "grammar.cc"


#define	YYFINAL		1160
#define	YYFLAG		-32768
#define	YYNTBASE	115

#define YYTRANSLATE(x) ((unsigned)(x) <= 369 ? yytranslate[x] : 368)

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
   106,   107,   108,   109,   110,   111,   112,   113,   114
};

#if YY_PDDL_Parser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     6,     9,    12,    15,    16,    22,    25,    28,
    31,    34,    37,    40,    43,    46,    47,    52,    54,    56,
    58,    60,    62,    64,    66,    68,    70,    72,    74,    79,
    82,    85,    88,    89,    94,    97,    99,   100,   106,   111,
   112,   121,   124,   125,   128,   131,   133,   135,   138,   140,
   141,   147,   148,   154,   155,   161,   162,   171,   173,   174,
   177,   179,   180,   186,   187,   193,   198,   203,   206,   207,
   210,   213,   214,   219,   224,   229,   232,   233,   236,   238,
   241,   243,   245,   247,   249,   250,   257,   258,   265,   269,
   273,   277,   281,   285,   288,   292,   293,   294,   300,   301,
   307,   309,   314,   317,   319,   321,   323,   325,   327,   329,
   331,   333,   335,   337,   338,   344,   350,   351,   360,   369,
   370,   379,   388,   389,   401,   413,   416,   417,   420,   421,
   423,   425,   426,   432,   434,   436,   442,   451,   453,   455,
   457,   459,   461,   466,   472,   478,   484,   490,   494,   496,
   498,   502,   507,   509,   511,   514,   516,   519,   524,   526,
   529,   532,   533,   534,   535,   547,   548,   549,   559,   560,
   567,   568,   569,   584,   585,   586,   599,   600,   610,   616,
   618,   623,   625,   628,   630,   635,   637,   638,   644,   645,
   654,   660,   669,   670,   676,   677,   678,   679,   689,   698,
   703,   705,   708,   710,   711,   717,   723,   724,   733,   742,
   744,   749,   752,   754,   755,   756,   766,   767,   774,   775,
   776,   789,   790,   799,   801,   803,   809,   811,   816,   818,
   820,   825,   828,   830,   832,   834,   835,   841,   842,   852,
   853,   862,   864,   866,   867,   876,   877,   889,   890,   900,
   901,   911,   912,   922,   923,   936,   937,   950,   951,   964,
   966,   971,   973,   976,   978,   980,   982,   988,   990,   996,
   998,  1000,  1006,  1008,  1010,  1011,  1022,  1025,  1026,  1027,
  1033,  1039,  1044,  1050,  1053,  1056,  1059,  1062,  1065,  1068,
  1071,  1074,  1077,  1078,  1083,  1086,  1089,  1092,  1093,  1094,
  1100,  1101,  1111,  1112,  1122,  1128,  1133,  1141,  1146,  1154,
  1156,  1162,  1165,  1166,  1168,  1169,  1175,  1176,  1182,  1185,
  1186,  1187,  1193,  1194,  1203,  1209,  1211,  1216,  1221,  1226,
  1232,  1238,  1244,  1251,  1257,  1259,  1261,  1263,  1269,  1275,
  1277,  1281,  1283,  1285,  1286,  1292,  1293,  1302,  1306,  1308,
  1310,  1311,  1317,  1323,  1328,  1333,  1338,  1344,  1350,  1351,
  1360,  1361,  1370,  1373,  1375,  1376,  1382,  1384,  1386,  1387,
  1395,  1396,  1404,  1405,  1411,  1412,  1421,  1425,  1428,  1431,
  1434,  1437,  1438,  1441,  1444,  1447,  1448,  1454,  1457,  1463,
  1468,  1470,  1471,  1475,  1482,  1486,  1493,  1497,  1504,  1505,
  1508,  1514,  1515,  1518,  1524,  1527,  1533,  1534,  1537,  1539,
  1541,  1543,  1545,  1547,  1549,  1550,  1556,  1557,  1566,  1567,
  1576,  1582,  1583,  1592,  1593,  1605,  1606,  1618,  1619,  1631,
  1640,  1641,  1650,  1651,  1663,  1664,  1676,  1677,  1692,  1697,
  1702,  1704,  1706,  1708,  1711,  1714,  1715,  1716,  1722,  1723,
  1732,  1733,  1734,  1735,  1751,  1752,  1758,  1761,  1764,  1767,
  1768,  1771,  1772,  1780,  1782,  1784,  1787,  1789,  1790,  1799,
  1803,  1804,  1807,  1809,  1810,  1816,  1821,  1824,  1825,  1826,
  1832,  1835,  1837,  1840,  1842,  1844,  1846,  1847,  1853,  1854,
  1863,  1864,  1873,  1876,  1879,  1880,  1883,  1886,  1887,  1888,
  1894,  1896,  1897,  1905,  1906,  1912,  1913
};

static const short yyrhs[] = {   115,
   116,     0,   115,   248,     0,   115,   332,     0,   115,   338,
     0,   115,   346,     0,     0,     3,    40,   118,   117,     4,
     0,   121,   117,     0,   140,   117,     0,   144,   117,     0,
   123,   117,     0,   131,   117,     0,   147,   117,     0,   357,
   117,     0,   259,   117,     0,     0,     3,    41,   119,     4,
     0,    19,     0,    20,     0,    21,     0,    22,     0,    24,
     0,    25,     0,    26,     0,    27,     0,    30,     0,    19,
     0,    26,     0,     3,    35,   122,     4,     0,   122,    39,
     0,   122,   103,     0,   122,    28,     0,     0,     3,    37,
   124,     4,     0,   125,   124,     0,   125,     0,     0,     3,
    19,   126,   127,     4,     0,   127,   129,    14,    21,     0,
     0,   127,   129,    14,     3,    61,   128,   130,     4,     0,
   127,   129,     0,     0,   129,    29,     0,   129,    25,     0,
    29,     0,    25,     0,   130,    21,     0,    21,     0,     0,
     3,    38,   132,   133,     4,     0,     0,   137,    14,   113,
   134,   133,     0,     0,   137,    14,    21,   135,   133,     0,
     0,   137,    14,     3,    61,   130,     4,   136,   133,     0,
   137,     0,     0,   138,   137,     0,   138,     0,     0,     3,
    19,   139,   127,     4,     0,     0,     3,    39,   141,   142,
     4,     0,   142,   143,    14,    21,     0,   142,   143,    14,
    19,     0,   142,   143,     0,     0,   143,    21,     0,   143,
    19,     0,     0,     3,    36,   145,     4,     0,     3,    64,
   145,     4,     0,   145,   146,    14,    21,     0,   145,   146,
     0,     0,   146,    19,     0,    19,     0,   146,    20,     0,
    20,     0,   148,     0,   274,     0,   283,     0,     0,     3,
    42,   120,   149,   150,     4,     0,     0,   150,    45,     3,
   151,   127,     4,     0,   150,    96,   152,     0,   150,    51,
   206,     0,   150,    46,   155,     0,   150,    47,   155,     0,
   150,    53,   235,     0,   150,   243,     0,   150,   102,    34,
     0,     0,     0,     3,    19,   153,   165,     4,     0,     0,
     3,    31,   154,   165,     4,     0,   157,     0,     3,    54,
   156,     4,     0,   156,   157,     0,   157,     0,   159,     0,
   162,     0,   178,     0,   196,     0,   198,     0,   171,     0,
    48,     0,    49,     0,    50,     0,     0,     3,    22,   160,
   166,     4,     0,     3,    13,   170,   170,     4,     0,     0,
     3,   158,     3,    22,   161,   166,     4,     4,     0,     3,
   158,     3,    13,   170,   170,     4,     4,     0,     0,     3,
    59,     3,    22,   163,   166,     4,     4,     0,     3,    59,
     3,    13,   170,   170,     4,     4,     0,     0,     3,   158,
     3,    59,     3,    22,   164,   166,     4,     4,     4,     0,
     3,   158,     3,    59,     3,    13,   170,   170,     4,     4,
     4,     0,   165,   167,     0,     0,   166,   170,     0,     0,
    25,     0,    20,     0,     0,     3,    23,   169,   166,     4,
     0,   167,     0,   168,     0,     3,   172,   173,   173,     4,
     0,     3,   158,     3,   172,   173,   173,     4,     4,     0,
     7,     0,     8,     0,     9,     0,    10,     0,    13,     0,
     3,    14,   173,     4,     0,     3,    15,   173,   174,     4,
     0,     3,    14,   173,   173,     4,     0,     3,    16,   173,
   175,     4,     0,     3,    17,   173,   173,     4,     0,   173,
    17,   173,     0,    33,     0,    32,     0,     3,    74,     4,
     0,     3,   100,    30,     4,     0,   176,     0,   173,     0,
   173,   174,     0,   173,     0,   173,   175,     0,     3,    24,
   177,     4,     0,    24,     0,    25,   177,     0,    20,   177,
     0,     0,     0,     0,     3,    84,   179,   300,   303,     3,
    22,   180,   166,     4,     4,     0,     0,     0,     3,    57,
     3,   181,   127,     4,   182,   189,     4,     0,     0,     3,
    58,   183,   192,   190,     4,     0,     0,     0,     3,   158,
     3,    84,   184,   300,   303,     3,    22,   185,   166,     4,
     4,     4,     0,     0,     0,     3,   158,     3,    57,     3,
   186,   127,     4,   187,   189,     4,     4,     0,     0,     3,
   158,     3,    58,   188,   192,   190,     4,     4,     0,     3,
    58,   192,   190,     4,     0,   190,     0,     3,    54,   191,
     4,     0,   193,     0,   193,   191,     0,   193,     0,     3,
    54,   305,     4,     0,   306,     0,     0,     3,    22,   194,
   166,     4,     0,     0,     3,    59,     3,    22,   195,   166,
     4,     4,     0,     3,    13,   170,   170,     4,     0,     3,
    59,     3,    13,   170,   170,     4,     4,     0,     0,     3,
    55,   197,   202,     4,     0,     0,     0,     0,     3,    56,
     3,   199,   127,     4,   200,   201,     4,     0,     3,    54,
   305,     3,    55,   202,     4,     4,     0,     3,    55,   202,
     4,     0,   203,     0,   203,   202,     0,   203,     0,     0,
     3,    22,   204,   166,     4,     0,     3,    13,   170,   170,
     4,     0,     0,     3,    59,     3,    22,   205,   166,     4,
     4,     0,     3,    59,     3,    13,   170,   170,     4,     4,
     0,   208,     0,     3,    54,   207,     4,     0,   208,   207,
     0,   208,     0,     0,     0,     3,    57,     3,   209,   127,
     4,   210,   215,     4,     0,     0,     3,    60,   211,   217,
   216,     4,     0,     0,     0,     3,   158,     3,    57,     3,
   212,   127,     4,   213,   215,     4,     4,     0,     0,     3,
   158,     3,    60,   214,   217,   216,     4,     0,   219,     0,
   228,     0,     3,    60,   217,   216,     4,     0,   216,     0,
     3,    54,   218,     4,     0,   219,     0,   306,     0,     3,
    54,   305,     4,     0,   219,   218,     0,   219,     0,   220,
     0,   225,     0,     0,     3,    22,   221,   166,     4,     0,
     0,     3,    79,     3,    23,   222,   166,     4,   224,     4,
     0,     0,     3,   158,     3,    22,   223,   166,     4,     4,
     0,   170,     0,   114,     0,     0,     3,    59,     3,    22,
   226,   166,     4,     4,     0,     0,     3,   158,     3,    59,
     3,    22,   227,   166,     4,     4,     4,     0,     0,     3,
    75,     3,    24,   229,   166,     4,   173,     4,     0,     0,
     3,    76,     3,    24,   230,   166,     4,   173,     4,     0,
     0,     3,    79,     3,    24,   231,   166,     4,   173,     4,
     0,     0,     3,   158,     3,    75,     3,    24,   232,   166,
     4,   173,     4,     4,     0,     0,     3,   158,     3,    76,
     3,    24,   233,   166,     4,   173,     4,     4,     0,     0,
     3,   158,     3,    79,     3,    24,   234,   166,     4,   173,
     4,     4,     0,   237,     0,     3,    54,   236,     4,     0,
   237,     0,   237,   236,     0,   238,     0,   239,     0,   241,
     0,     3,    13,    73,   173,     4,     0,   173,     0,     3,
   240,    73,   173,     4,     0,     9,     0,    10,     0,     3,
   242,    73,   173,     4,     0,     7,     0,     8,     0,     0,
     3,    97,   244,   301,   302,    98,     3,   245,     4,     4,
     0,   246,   245,     0,     0,     0,     3,    26,   247,   166,
     4,     0,     3,    40,   249,   250,     4,     0,     3,    62,
   119,     4,     0,   250,     3,    63,   119,     4,     0,   250,
   121,     0,   250,   144,     0,   250,   251,     0,   250,   259,
     0,   250,   269,     0,   250,   272,     0,   250,   290,     0,
   250,   283,     0,   250,   357,     0,     0,     3,    65,   252,
     4,     0,   252,   257,     0,   252,   255,     0,   252,   253,
     0,     0,     0,     3,    22,   254,   166,     4,     0,     0,
     3,    13,     3,    23,   256,   166,     4,    20,     4,     0,
     0,     3,    13,     3,    24,   258,   166,     4,   273,     4,
     0,     3,    13,    24,   273,     4,     0,     3,    66,   260,
     4,     0,     3,    66,     3,    54,   261,     4,     4,     0,
     3,   103,   260,     4,     0,     3,   103,     3,    54,   261,
     4,     4,     0,   266,     0,     3,    99,    19,   262,     4,
     0,   260,   261,     0,     0,   266,     0,     0,     3,    54,
   263,   265,     4,     0,     0,     3,    55,   264,   265,     4,
     0,   266,   265,     0,     0,     0,     3,    22,   267,   166,
     4,     0,     0,     3,    59,     3,    22,   268,   166,     4,
     4,     0,     3,    13,   170,   170,     4,     0,   171,     0,
     3,   104,   262,     4,     0,     3,   105,   262,     4,     0,
     3,   106,   262,     4,     0,     3,   107,   262,   262,     4,
     0,     3,   108,   262,   262,     4,     0,     3,   101,   273,
   262,     4,     0,     3,   109,   273,   262,   262,     4,     0,
     3,    70,   270,   271,     4,     0,    71,     0,    72,     0,
   173,     0,     3,    67,    68,    33,     4,     0,     3,    67,
    69,    33,     4,     0,    33,     0,    33,    17,    33,     0,
    32,     0,    94,     0,     0,     3,    52,   275,   294,   276,
     0,     0,    83,   277,     3,   323,    33,   324,     4,     4,
     0,    89,   278,     4,     0,   111,     0,   112,     0,     0,
     3,    22,   279,   165,     4,     0,     3,    13,   167,   167,
     4,     0,     3,    59,   278,     4,     0,     3,    54,   282,
     4,     0,     3,    55,   282,     4,     0,     3,    58,   278,
   278,     4,     0,     3,   110,   278,   278,     4,     0,     0,
     3,    57,     3,   280,   127,     4,   278,     4,     0,     0,
     3,    56,     3,   281,   127,     4,   278,     4,     0,   282,
   278,     0,   278,     0,     0,     3,    90,   284,   294,   285,
     0,   286,     0,   288,     0,     0,    42,     3,    26,   287,
   166,     4,     4,     0,     0,    95,     3,    22,   289,   166,
     4,     4,     0,     0,     3,    52,   291,   294,   292,     0,
     0,    83,   293,     3,   323,    33,   324,     4,     4,     0,
    89,   278,     4,     0,   294,   295,     0,   294,   296,     0,
   294,   297,     0,   294,   299,     0,     0,    80,   119,     0,
    81,    19,     0,    81,    27,     0,     0,    82,     3,   298,
   127,     4,     0,    88,   306,     0,    88,     3,    54,   305,
     4,     0,    82,     3,   127,     4,     0,   300,     0,     0,
    88,   306,   304,     0,    88,     3,    54,   305,     4,   304,
     0,    46,   306,   303,     0,    46,     3,    54,   305,     4,
   303,     0,    47,   306,   303,     0,    47,     3,    54,   305,
     4,   303,     0,     0,    88,   306,     0,    88,     3,    54,
   305,     4,     0,     0,    46,   306,     0,    46,     3,    54,
   305,     4,     0,    47,   306,     0,    47,     3,    54,   305,
     4,     0,     0,   305,   306,     0,   306,     0,   307,     0,
   311,     0,   316,     0,   319,     0,   322,     0,     0,     3,
    22,   308,   166,     4,     0,     0,     3,   158,     3,    22,
   309,   166,     4,     4,     0,     0,     3,    65,     3,    22,
   310,   166,     4,     4,     0,     3,    13,   170,   170,     4,
     0,     0,     3,    59,     3,    22,   312,   166,     4,     4,
     0,     0,     3,   158,     3,    59,     3,    22,   313,   166,
     4,     4,     4,     0,     0,     3,    65,     3,    59,     3,
    22,   314,   166,     4,     4,     4,     0,     0,     3,    59,
     3,    65,     3,    22,   315,   166,     4,     4,     4,     0,
     3,    59,     3,    13,   170,   170,     4,     4,     0,     0,
     3,    66,     3,    22,   317,   166,     4,     4,     0,     0,
     3,    66,     3,    59,     3,    22,   318,   166,     4,     4,
     4,     0,     0,     3,    59,     3,    66,     3,    22,   320,
   166,     4,     4,     4,     0,     0,     3,    59,     3,    66,
     3,    59,     3,    22,   321,   166,     4,     4,     4,     4,
     0,     3,    21,    25,     4,     0,     3,    21,   168,     4,
     0,    85,     0,    86,     0,    87,     0,   324,   325,     0,
   324,   328,     0,     0,     0,     3,    22,   326,   166,     4,
     0,     0,     3,    59,     3,    22,   327,   166,     4,     4,
     0,     0,     0,     0,     3,    84,    82,     3,   329,   127,
     4,   330,   303,     3,    22,   331,   166,     4,     4,     0,
     0,     3,    91,   333,   334,     4,     0,   335,   334,     0,
    93,   334,     0,   336,   334,     0,     0,    81,   119,     0,
     0,   273,    11,     3,    26,   337,   166,     4,     0,   339,
     0,   343,     0,   340,   339,     0,   340,     0,     0,   273,
    11,     3,    26,   341,   166,     4,   342,     0,     5,   273,
     6,     0,     0,   344,   343,     0,   344,     0,     0,     3,
    26,   345,   166,     4,     0,     3,    92,   347,     4,     0,
   347,   348,     0,     0,     0,     3,   349,   351,     4,   350,
     0,    93,   273,     0,   273,     0,   351,   352,     0,   352,
     0,   353,     0,   355,     0,     0,     3,    22,   354,   166,
     4,     0,     0,     3,    59,     3,    22,   356,   166,     4,
     4,     0,     0,     3,    96,   358,   359,   301,   303,   360,
     4,     0,    81,    19,     0,    81,    27,     0,     0,   360,
   363,     0,   360,   361,     0,     0,     0,     3,   119,   362,
   166,     4,     0,   119,     0,     0,     3,    84,   364,   300,
   303,   365,     4,     0,     0,     3,   119,   366,   166,     4,
     0,     0,     3,    59,     3,   119,   367,   166,     4,     4,
     0
};

#endif

#if YY_PDDL_Parser_DEBUG != 0
static const short yyrline[] = { 0,
   103,   105,   106,   107,   108,   109,   112,   116,   118,   119,
   120,   121,   122,   123,   124,   125,   128,   136,   138,   139,
   140,   141,   142,   143,   144,   145,   148,   150,   155,   159,
   161,   162,   163,   168,   172,   174,   177,   182,   192,   197,
   201,   205,   209,   212,   225,   232,   244,   253,   258,   265,
   270,   273,   279,   279,   309,   309,   339,   339,   343,   346,
   348,   351,   356,   368,   373,   376,   384,   397,   404,   407,
   413,   419,   424,   426,   429,   434,   438,   441,   450,   458,
   465,   476,   478,   479,   484,   489,   498,   503,   507,   508,
   509,   510,   511,   512,   513,   518,   521,   530,   534,   539,
   545,   547,   550,   552,   555,   557,   558,   559,   560,   561,
   567,   572,   576,   582,   587,   593,   601,   605,   611,   622,
   627,   633,   641,   645,   651,   662,   669,   672,   679,   682,
   690,   696,   702,   722,   727,   732,   737,   744,   749,   753,
   757,   761,   767,   772,   776,   780,   784,   788,   792,   796,
   800,   804,   808,   814,   819,   825,   830,   836,   841,   847,
   852,   856,   862,   868,   872,   887,   892,   901,   911,   915,
   920,   925,   929,   945,   950,   959,   971,   975,   983,   985,
   988,   990,   993,   995,   998,  1000,  1003,  1008,  1015,  1019,
  1026,  1035,  1072,  1077,  1085,  1086,  1091,  1100,  1112,  1114,
  1115,  1118,  1120,  1123,  1128,  1134,  1142,  1146,  1152,  1162,
  1164,  1167,  1169,  1172,  1178,  1186,  1197,  1201,  1206,  1211,
  1219,  1232,  1236,  1243,  1244,  1247,  1249,  1252,  1254,  1257,
  1259,  1262,  1264,  1267,  1269,  1272,  1278,  1288,  1292,  1316,
  1321,  1333,  1338,  1344,  1350,  1360,  1365,  1377,  1382,  1389,
  1393,  1400,  1404,  1411,  1415,  1422,  1426,  1433,  1437,  1446,
  1448,  1451,  1453,  1456,  1458,  1459,  1462,  1468,  1475,  1482,
  1484,  1487,  1494,  1496,  1499,  1509,  1532,  1534,  1537,  1542,
  1555,  1559,  1567,  1569,  1570,  1571,  1572,  1573,  1574,  1575,
  1576,  1577,  1578,  1581,  1585,  1587,  1588,  1589,  1592,  1597,
  1610,  1615,  1627,  1632,  1645,  1661,  1663,  1664,  1665,  1668,
  1673,  1681,  1683,  1686,  1691,  1695,  1701,  1705,  1713,  1721,
  1724,  1729,  1736,  1740,  1747,  1755,  1759,  1763,  1767,  1771,
  1775,  1779,  1783,  1819,  1823,  1833,  1844,  1859,  1864,  1877,
  1879,  1880,  1881,  1886,  1892,  1895,  1900,  1907,  1915,  1920,
  1924,  1928,  1933,  1937,  1941,  1946,  1951,  1955,  1959,  1963,
  1978,  1982,  1999,  2004,  2010,  2017,  2020,  2022,  2025,  2030,
  2042,  2047,  2059,  2066,  2069,  2074,  2081,  2089,  2091,  2092,
  2093,  2094,  2097,  2104,  2111,  2118,  2123,  2129,  2131,  2134,
  2144,  2146,  2149,  2151,  2152,  2153,  2154,  2155,  2156,  2159,
  2161,  2162,  2165,  2167,  2168,  2169,  2170,  2173,  2175,  2178,
  2180,  2181,  2182,  2183,  2186,  2191,  2198,  2202,  2208,  2212,
  2218,  2228,  2233,  2240,  2244,  2250,  2254,  2260,  2264,  2270,
  2280,  2285,  2291,  2295,  2303,  2308,  2314,  2318,  2326,  2333,
  2340,  2345,  2349,  2355,  2357,  2358,  2360,  2365,  2370,  2374,
  2381,  2387,  2395,  2399,  2415,  2424,  2437,  2439,  2444,  2445,
  2448,  2456,  2461,  2473,  2479,  2486,  2488,  2491,  2496,  2525,
  2527,  2530,  2532,  2535,  2540,  2570,  2574,  2576,  2579,  2584,
  2591,  2597,  2603,  2605,  2608,  2610,  2613,  2619,  2628,  2634,
  2643,  2652,  2659,  2665,  2669,  2672,  2674,  2675,  2678,  2688,
  2695,  2710,  2717,  2731,  2741,  2749,  2758
};

static const char * const yytname[] = {   "$","error","$illegal.","TK_OPEN",
"TK_CLOSE","TK_OPEN_SQ","TK_CLOSE_SQ","TK_GREATER","TK_GREATEQ","TK_LESS","TK_LESSEQ",
"TK_COLON","TK_HASHT","TK_EQ","TK_HYPHEN","TK_PLUS","TK_MUL","TK_DIV","TK_UMINUS",
"TK_NEW_SYMBOL","TK_OBJ_SYMBOL","TK_TYPE_SYMBOL","TK_PRED_SYMBOL","TK_OBJFUN_SYMBOL",
"TK_FUN_SYMBOL","TK_VAR_SYMBOL","TK_ACTION_SYMBOL","TK_MISC_SYMBOL","TK_KEYWORD",
"TK_NEW_VAR_SYMBOL","TK_PREFERENCE_SYMBOL","TK_SET_SYMBOL","TK_FLOAT","TK_INT",
"TK_STRING","KW_REQS","KW_CONSTANTS","KW_PREDS","KW_FUNS","KW_TYPES","KW_DEFINE",
"KW_DOMAIN","KW_ACTION","KW_PROCESS","KW_EVENT","KW_ARGS","KW_PRE","KW_COND",
"KW_AT_START","KW_AT_END","KW_OVER_ALL","KW_EFFECT","KW_INVARIANT","KW_DURATION",
"KW_AND","KW_OR","KW_EXISTS","KW_FORALL","KW_IMPLY","KW_NOT","KW_WHEN","KW_EITHER",
"KW_PROBLEM","KW_FORDOMAIN","KW_OBJECTS","KW_INIT","KW_GOAL","KW_LENGTH","KW_SERIAL",
"KW_PARALLEL","KW_METRIC","KW_MINIMIZE","KW_MAXIMIZE","KW_DURATION_VAR","KW_TOTAL_TIME",
"KW_INCREASE","KW_DECREASE","KW_SCALE_UP","KW_SCALE_DOWN","KW_ASSIGN","KW_TAG",
"KW_NAME","KW_VARS","KW_SET_CONSTRAINT","KW_SETOF","KW_AT_LEAST_N","KW_AT_MOST_N",
"KW_EXACTLY_N","KW_CONTEXT","KW_FORMULA","KW_IRRELEVANT","KW_PLAN","KW_HEURISTIC",
"KW_OPT","KW_INF","KW_FACT","KW_SET","KW_EXPANSION","KW_TASKS","KW_PREFERENCE",
"KW_VIOLATED","KW_WITHIN","KW_ASSOC","KW_CONSTRAINTS","KW_ALWAYS","KW_SOMETIME",
"KW_AT_MOST_ONCE","KW_SOMETIME_BEFORE","KW_SOMETIME_AFTER","KW_ALWAYS_WITHIN",
"KW_IFF","KW_FALSE","KW_TRUE","KW_NUMBER","KW_UNDEFINED","pddl_declarations",
"pddl_domain","domain_elements","domain_name","any_symbol","action_symbol","domain_requires",
"require_list","domain_predicates","predicate_list","predicate_decl","@1","typed_param_list",
"@2","typed_param_sym_list","non_empty_type_name_list","domain_functions","@3",
"function_list","@4","@5","@6","function_decl_list","function_decl","@7","domain_types",
"@8","typed_type_list","primitive_type_list","domain_constants","typed_constant_list",
"ne_constant_sym_list","domain_structure","action_declaration","@9","action_elements",
"@10","action_set_name","@11","@12","action_condition","action_condition_list",
"single_action_condition","timing_keyword","positive_atom_condition","@13","@14",
"negative_atom_condition","@15","@16","flat_atom_argument_list","atom_argument_list",
"flat_atom_argument","functional_atom_argument","@17","atom_argument","numeric_condition",
"relation_keyword","d_expression","d_sum","d_product","d_function","d_argument_list",
"set_condition","@18","@19","@20","@21","@22","@23","@24","@25","@26","@27",
"universal_condition_body","one_or_more_condition_atoms","quantified_condition_atom_list",
"one_or_more_context_atoms","quantified_condition_atom","@28","@29","disjunctive_condition",
"@30","disjunctive_set_condition","@31","@32","existential_condition_body","disjunction_list",
"disjunction_atom","@33","@34","action_effect","action_effect_list","single_action_effect",
"@35","@36","@37","@38","@39","@40","quantified_effect_body","one_or_more_atomic_effects",
"effect_conditions","atomic_effect_list","atomic_effect","positive_atom_effect",
"@41","@42","@43","object_assignment_value","negative_atom_effect","@44","@45",
"numeric_effect","@46","@47","@48","@49","@50","@51","action_duration","action_duration_list",
"action_duration_exp","action_exact_duration","action_min_duration","less_or_lesseq",
"action_max_duration","greater_or_greatereq","action_expansion","@52","task_list",
"task","@53","pddl_problem","problem_name","problem_elements","initial_state",
"init_elements","init_atom","@54","init_object_function","@55","init_function",
"@56","goal_spec","single_goal_spec","goal_spec_list","new_goal","@57","@58",
"new_goal_list","new_single_goal","@59","@60","metric_spec","metric_keyword",
"metric_expression","length_spec","numeric_value","domain_invariant","@61","domain_invariant_body",
"@62","fol_formula","@63","@64","@65","fol_formula_list","irrelevant_item","@66",
"irrelevant_item_content","irrelevant_action","@67","irrelevant_atom","@68",
"problem_invariant","@69","problem_invariant_body","@70","dkel_element_list",
"dkel_tag_spec","dkel_name_spec","dkel_vars_spec","@71","dkel_context_spec",
"req_vars_spec","opt_vars_spec","opt_context_and_precondition_spec","opt_context_spec",
"opt_precondition_spec","context_list","context_atom","positive_context_atom",
"@72","@73","@74","negative_context_atom","@75","@76","@77","@78","positive_context_goal_atom",
"@79","@80","negative_context_goal_atom","@81","@82","type_constraint_atom",
"set_constraint_type","invariant_set","invariant_atom","@83","@84","invariant_set_of_atoms",
"@85","@86","@87","pddl_plan","@88","plan_elements","plan_name","plan_step",
"@89","ipc_plan","ipc_plan_step_list","ipc_plan_step","@90","opt_step_duration",
"ipc_plan_step_seq","simple_plan_step","@91","heuristic_table","table_entry_list",
"table_entry","@92","entry_value","ne_entry_atom_list","entry_atom","pos_entry_atom",
"@93","neg_entry_atom","@94","reference_set","@95","opt_set_name","reference_list",
"reference","@96","simple_reference_set","@97","simple_reference_set_body","@98",
"@99",""
};
#endif

static const short yyr1[] = {     0,
   115,   115,   115,   115,   115,   115,   116,   117,   117,   117,
   117,   117,   117,   117,   117,   117,   118,   119,   119,   119,
   119,   119,   119,   119,   119,   119,   120,   120,   121,   122,
   122,   122,   122,   123,   124,   124,   126,   125,   127,   128,
   127,   127,   127,   129,   129,   129,   129,   130,   130,   132,
   131,   134,   133,   135,   133,   136,   133,   133,   133,   137,
   137,   139,   138,   141,   140,   142,   142,   142,   142,   143,
   143,   143,   144,   144,   145,   145,   145,   146,   146,   146,
   146,   147,   147,   147,   149,   148,   151,   150,   150,   150,
   150,   150,   150,   150,   150,   150,   153,   152,   154,   152,
   155,   155,   156,   156,   157,   157,   157,   157,   157,   157,
   158,   158,   158,   160,   159,   159,   161,   159,   159,   163,
   162,   162,   164,   162,   162,   165,   165,   166,   166,   167,
   167,   169,   168,   170,   170,   171,   171,   172,   172,   172,
   172,   172,   173,   173,   173,   173,   173,   173,   173,   173,
   173,   173,   173,   174,   174,   175,   175,   176,   176,   177,
   177,   177,   179,   180,   178,   181,   182,   178,   183,   178,
   184,   185,   178,   186,   187,   178,   188,   178,   189,   189,
   190,   190,   191,   191,   192,   192,   194,   193,   195,   193,
   193,   193,   197,   196,   198,   199,   200,   198,   201,   201,
   201,   202,   202,   204,   203,   203,   205,   203,   203,   206,
   206,   207,   207,   209,   210,   208,   211,   208,   212,   213,
   208,   214,   208,   208,   208,   215,   215,   216,   216,   217,
   217,   218,   218,   219,   219,   221,   220,   222,   220,   223,
   220,   224,   224,   226,   225,   227,   225,   229,   228,   230,
   228,   231,   228,   232,   228,   233,   228,   234,   228,   235,
   235,   236,   236,   237,   237,   237,   238,   238,   239,   240,
   240,   241,   242,   242,   244,   243,   245,   245,   247,   246,
   248,   249,   250,   250,   250,   250,   250,   250,   250,   250,
   250,   250,   250,   251,   252,   252,   252,   252,   254,   253,
   256,   255,   258,   257,   257,   259,   259,   259,   259,   260,
   260,   261,   261,   262,   263,   262,   264,   262,   265,   265,
   267,   266,   268,   266,   266,   266,   266,   266,   266,   266,
   266,   266,   266,   269,   270,   270,   271,   272,   272,   273,
   273,   273,   273,   275,   274,   277,   276,   276,   278,   278,
   279,   278,   278,   278,   278,   278,   278,   278,   280,   278,
   281,   278,   282,   282,   284,   283,   285,   285,   287,   286,
   289,   288,   291,   290,   293,   292,   292,   294,   294,   294,
   294,   294,   295,   296,   296,   298,   297,   299,   299,   300,
   301,   301,   302,   302,   302,   302,   302,   302,   302,   303,
   303,   303,   304,   304,   304,   304,   304,   305,   305,   306,
   306,   306,   306,   306,   308,   307,   309,   307,   310,   307,
   307,   312,   311,   313,   311,   314,   311,   315,   311,   311,
   317,   316,   318,   316,   320,   319,   321,   319,   322,   322,
   323,   323,   323,   324,   324,   324,   326,   325,   327,   325,
   329,   330,   331,   328,   333,   332,   334,   334,   334,   334,
   335,   337,   336,   338,   338,   339,   339,   341,   340,   342,
   342,   343,   343,   345,   344,   346,   347,   347,   349,   348,
   350,   350,   351,   351,   352,   352,   354,   353,   356,   355,
   358,   357,   359,   359,   359,   360,   360,   360,   362,   361,
   361,   364,   363,   366,   365,   367,   365
};

static const short yyr2[] = {     0,
     2,     2,     2,     2,     2,     0,     5,     2,     2,     2,
     2,     2,     2,     2,     2,     0,     4,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     4,     2,
     2,     2,     0,     4,     2,     1,     0,     5,     4,     0,
     8,     2,     0,     2,     2,     1,     1,     2,     1,     0,
     5,     0,     5,     0,     5,     0,     8,     1,     0,     2,
     1,     0,     5,     0,     5,     4,     4,     2,     0,     2,
     2,     0,     4,     4,     4,     2,     0,     2,     1,     2,
     1,     1,     1,     1,     0,     6,     0,     6,     3,     3,
     3,     3,     3,     2,     3,     0,     0,     5,     0,     5,
     1,     4,     2,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     0,     5,     5,     0,     8,     8,     0,
     8,     8,     0,    11,    11,     2,     0,     2,     0,     1,
     1,     0,     5,     1,     1,     5,     8,     1,     1,     1,
     1,     1,     4,     5,     5,     5,     5,     3,     1,     1,
     3,     4,     1,     1,     2,     1,     2,     4,     1,     2,
     2,     0,     0,     0,    11,     0,     0,     9,     0,     6,
     0,     0,    14,     0,     0,    12,     0,     9,     5,     1,
     4,     1,     2,     1,     4,     1,     0,     5,     0,     8,
     5,     8,     0,     5,     0,     0,     0,     9,     8,     4,
     1,     2,     1,     0,     5,     5,     0,     8,     8,     1,
     4,     2,     1,     0,     0,     9,     0,     6,     0,     0,
    12,     0,     8,     1,     1,     5,     1,     4,     1,     1,
     4,     2,     1,     1,     1,     0,     5,     0,     9,     0,
     8,     1,     1,     0,     8,     0,    11,     0,     9,     0,
     9,     0,     9,     0,    12,     0,    12,     0,    12,     1,
     4,     1,     2,     1,     1,     1,     5,     1,     5,     1,
     1,     5,     1,     1,     0,    10,     2,     0,     0,     5,
     5,     4,     5,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     0,     4,     2,     2,     2,     0,     0,     5,
     0,     9,     0,     9,     5,     4,     7,     4,     7,     1,
     5,     2,     0,     1,     0,     5,     0,     5,     2,     0,
     0,     5,     0,     8,     5,     1,     4,     4,     4,     5,
     5,     5,     6,     5,     1,     1,     1,     5,     5,     1,
     3,     1,     1,     0,     5,     0,     8,     3,     1,     1,
     0,     5,     5,     4,     4,     4,     5,     5,     0,     8,
     0,     8,     2,     1,     0,     5,     1,     1,     0,     7,
     0,     7,     0,     5,     0,     8,     3,     2,     2,     2,
     2,     0,     2,     2,     2,     0,     5,     2,     5,     4,
     1,     0,     3,     6,     3,     6,     3,     6,     0,     2,
     5,     0,     2,     5,     2,     5,     0,     2,     1,     1,
     1,     1,     1,     1,     0,     5,     0,     8,     0,     8,
     5,     0,     8,     0,    11,     0,    11,     0,    11,     8,
     0,     8,     0,    11,     0,    11,     0,    14,     4,     4,
     1,     1,     1,     2,     2,     0,     0,     5,     0,     8,
     0,     0,     0,    15,     0,     5,     2,     2,     2,     0,
     2,     0,     7,     1,     1,     2,     1,     0,     8,     3,
     0,     2,     1,     0,     5,     4,     2,     0,     0,     5,
     2,     1,     2,     1,     1,     1,     0,     5,     0,     8,
     0,     8,     2,     2,     0,     2,     2,     0,     0,     5,
     1,     0,     7,     0,     5,     0,     8
};

static const short yydefact[] = {     6,
     0,     0,   342,   340,   343,     1,     2,     0,     3,     4,
   464,   467,   465,   473,     5,   474,     0,   455,   478,     0,
     0,   466,     0,   472,   129,     0,    16,   293,   460,     0,
   341,     0,     0,     0,     0,     0,     0,    16,    16,    16,
    16,    16,    16,    82,    16,    83,    84,    16,     0,     0,
   460,     0,     0,   460,   460,   479,   476,   477,   468,     0,
   475,   131,   130,   134,   135,   128,    18,    19,    20,    21,
    22,    23,    24,    25,    26,     0,     0,    33,    77,     0,
    50,    64,     0,   344,    77,     0,   365,   491,     0,     7,
     8,    11,    12,     9,    10,    13,    15,    14,     0,   281,
   284,   285,   286,   287,   288,   289,   291,   290,   292,   461,
   458,     0,   456,   457,   459,     0,   129,   132,    17,   282,
     0,     0,     0,     0,    36,    59,    69,    27,    28,    85,
   382,     0,     0,   326,     0,   310,   382,   495,     0,     0,
   373,     0,   298,     0,     0,     0,     0,     0,   484,   485,
   486,     0,   129,    29,    32,    30,    31,    73,    79,    81,
    76,    37,    34,    35,     0,     0,    58,    61,    72,    96,
     0,    74,   138,   139,   140,   141,   142,   321,   111,   112,
   113,   313,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   306,     0,     0,   392,   313,   308,   382,
     0,     0,     0,     0,   335,   336,     0,   462,   487,     0,
     0,   483,   471,     0,     0,    78,    80,    43,    62,    51,
     0,    60,    65,    68,     0,     0,     0,     0,   346,     0,
     0,   345,   378,   379,   380,   381,     0,   129,     0,   313,
     0,     0,     0,     0,     0,     0,   314,     0,     0,     0,
     0,     0,     0,     0,   159,   150,   149,     0,   153,     0,
     0,   366,   367,   368,   493,   494,     0,   391,   402,     0,
     0,   283,     0,   294,   297,   296,   295,     0,     0,   337,
     0,   129,   129,     0,     0,   482,   480,     0,   469,   133,
    75,     0,    43,     0,    54,    52,     0,    71,    70,     0,
    86,     0,   195,   195,     0,     0,     0,     0,    94,   383,
   384,   385,   386,     0,     0,   388,   410,   411,   412,   413,
   414,     0,   349,   350,     0,     0,     0,   312,     0,   323,
     0,     0,   315,   317,   327,   328,   329,     0,     0,     0,
   142,     0,     0,     0,     0,     0,   162,     0,     0,     0,
     0,     0,     0,    43,     0,   498,     0,   375,     0,   374,
     0,   299,   338,   339,   334,     0,     0,   489,   481,     0,
    38,    47,    46,    42,     0,     0,    59,    59,    67,    66,
   275,    87,     0,    91,   101,   105,   106,   110,   107,   108,
   109,    92,     0,    90,   210,   224,   234,   235,   225,     0,
   268,    93,   260,   264,   265,   266,     0,    89,    95,    43,
     0,     0,     0,   415,     0,     0,     0,     0,     0,     0,
   351,     0,     0,     0,     0,     0,     0,     0,   348,   325,
   322,   307,   129,   311,   332,   320,   320,   330,   331,     0,
     0,     0,     0,     0,     0,   162,   162,     0,   151,     0,
   148,   136,   369,   371,     0,     0,   400,     0,   309,     0,
     0,     0,     0,   129,   463,   488,   129,   470,     0,    45,
    44,    63,    49,     0,    55,    53,   392,    43,   142,   114,
   195,   193,     0,     0,   169,     0,   163,     0,   236,     0,
     0,     0,   217,     0,     0,     0,     0,   273,   274,   270,
   271,     0,     0,     0,     0,    97,    99,     0,   441,   442,
   443,     0,     0,     0,     0,   129,     0,     0,   409,     0,
     0,     0,     0,     0,   127,   364,     0,     0,   361,   359,
     0,     0,     0,     0,     0,     0,   320,     0,   333,     0,
   143,     0,   154,     0,   156,     0,     0,   161,   160,   158,
   152,   129,   129,   390,     0,     0,   492,   501,   497,   496,
     0,   377,   301,   303,     0,     0,     0,     0,    39,    56,
    48,   399,     0,     0,   129,     0,     0,   104,     0,   196,
   166,     0,     0,     0,     0,   129,     0,     0,   213,   214,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   262,
     0,     0,   127,   127,   387,   446,     0,   439,   440,     0,
   389,   408,     0,   422,     0,     0,   419,     0,   431,     0,
   417,     0,     0,     0,   355,   363,   356,    43,    43,     0,
   354,     0,     0,   316,   319,   318,     0,   145,   155,   144,
   157,   146,   147,     0,     0,     0,   502,   499,     0,   129,
   129,   305,   300,     0,    40,    59,     0,     0,     0,     0,
    88,     0,     0,   102,   103,     0,     0,   203,    43,    43,
     0,     0,   186,     0,   120,   402,   142,   117,     0,   177,
     0,   171,     0,   211,   212,    43,   244,     0,     0,   230,
   248,   250,   238,   252,   240,     0,     0,   222,     0,     0,
     0,     0,   261,   263,     0,     0,     0,     0,     0,   421,
   416,     0,   129,     0,     0,   129,     0,   129,     0,   129,
     0,   353,   352,   126,     0,     0,   357,   358,   324,   137,
     0,     0,   401,     0,   129,   446,     0,     0,   490,     0,
    57,     0,   402,     0,   402,     0,   407,     0,   116,   115,
     0,   204,     0,   194,   202,     0,     0,     0,     0,     0,
   182,     0,   129,     0,     0,   129,   174,     0,     0,     0,
   237,     0,   129,     0,     0,     0,   229,   129,   129,   129,
   129,   129,   219,     0,     0,     0,     0,     0,   267,   269,
   272,    98,   100,     0,     0,   444,   445,     0,     0,   428,
   435,     0,     0,   426,     0,   433,     0,   424,     0,     0,
   370,   372,   402,     0,     0,     0,     0,     0,     0,   395,
     0,   397,     0,     0,     0,   393,   278,     0,   129,     0,
   197,   167,     0,     0,   187,     0,     0,   170,     0,     0,
     0,     0,     0,    43,     0,     0,   123,   402,   215,     0,
     0,     0,     0,     0,   218,     0,     0,     0,     0,     0,
    43,   246,     0,   254,   256,   258,   447,     0,     0,   347,
     0,     0,   129,   129,     0,     0,   129,     0,   129,     0,
   129,     0,     0,     0,   500,     0,     0,     0,    41,     0,
     0,     0,     0,   403,     0,   405,     0,     0,   278,     0,
     0,     0,   207,     0,     0,   185,     0,   129,     0,     0,
   184,     0,     0,     0,   164,     0,     0,     0,     0,     0,
   129,     0,     0,     0,   231,     0,     0,   233,     0,     0,
     0,     0,     0,     0,     0,     0,   129,     0,   129,   129,
   129,   129,     0,     0,   430,   423,     0,     0,   437,   420,
     0,   432,     0,   418,     0,   362,   360,     0,     0,   376,
   302,   304,   402,   402,   407,     0,     0,   279,     0,   277,
   206,   205,     0,   129,     0,     0,   201,     0,     0,   180,
     0,     0,   181,   183,     0,   189,   122,   121,   129,   119,
   118,   175,     0,     0,     0,     0,     0,     0,   227,   245,
   228,   232,     0,     0,   243,   242,     0,     0,   241,   220,
     0,   223,     0,     0,     0,     0,   449,   451,     0,     0,
   129,     0,     0,     0,     0,   504,   503,   396,   398,   394,
     0,     0,   129,   276,     0,     0,     0,     0,   198,     0,
   168,   191,   188,     0,   129,     0,     0,   178,     0,     0,
   172,     0,   216,   249,   251,   239,   253,     0,     0,     0,
     0,     0,   448,   129,    43,     0,     0,     0,     0,     0,
     0,     0,   129,   404,   406,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   129,     0,     0,
     0,     0,     0,     0,     0,     0,   429,   436,     0,   427,
   434,   425,   506,     0,   280,   209,   208,     0,   200,     0,
     0,     0,   165,     0,   125,   124,     0,     0,     0,   247,
     0,     0,     0,     0,   452,     0,   129,   505,     0,   179,
   192,   190,   176,     0,   226,   221,   255,   257,   259,   450,
   402,     0,     0,     0,     0,     0,   438,     0,     0,   173,
     0,   507,   199,   453,   129,     0,     0,   454,     0,     0
};

static const short yydefgoto[] = {     1,
     6,    37,    27,    76,   130,    38,   121,    39,   124,   125,
   218,   292,   740,   374,   474,    40,   126,   166,   378,   377,
   656,   167,   168,   293,    41,   127,   169,   224,    42,   122,
   161,    43,    44,   170,   225,   478,   408,   603,   604,   384,
   577,   385,   419,   386,   575,   766,   387,   763,   921,   624,
    33,    64,    65,   153,    66,   134,   193,   401,   544,   546,
   259,   448,   389,   584,   989,   670,   905,   582,   770,  1088,
   844,  1047,   768,   979,   980,   910,   672,   761,   908,  1045,
   390,   579,   391,   669,   904,   976,   667,   668,   829,   974,
   394,   588,   589,   686,   923,   592,   861,  1058,   785,   998,
   999,   689,   927,   777,   397,   586,   780,   782,  1007,   398,
   773,   937,   399,   778,   779,   781,   939,   940,   941,   402,
   599,   600,   404,   405,   504,   406,   505,   309,   477,   898,
   899,  1033,     7,    28,    49,   103,   202,   275,   464,   276,
   650,   277,   651,    45,   240,   241,   246,   436,   437,   536,
   247,   238,   433,   105,   207,   281,   106,    52,    46,   131,
   232,   314,   526,   525,   629,   628,   527,    47,   137,   262,
   263,   552,   264,   553,   108,   200,   360,   460,   171,   233,
   234,   235,   410,   236,   268,   269,   660,   356,   826,   518,
   519,   317,   516,   720,   716,   318,   713,   881,   877,   873,
   319,   718,   879,   320,   874,  1021,   321,   512,   709,   796,
   942,  1064,   797,  1065,  1141,  1155,     9,    29,    53,    54,
    55,   282,    10,    11,    12,   117,   289,    13,    14,    25,
    15,    30,    58,   116,   287,   148,   149,   150,   283,   151,
   467,    48,   138,   197,   458,   559,   735,   560,   734,   959,
  1073,  1127
};

static const short yypact[] = {-32768,
    55,    19,-32768,    54,-32768,-32768,-32768,    67,-32768,-32768,
-32768,    37,-32768,   139,-32768,-32768,   167,-32768,-32768,    56,
   169,-32768,   168,-32768,-32768,   107,   239,-32768,    74,   473,
-32768,   222,   620,  1314,  1314,   716,   264,   239,   239,   239,
   239,   239,   239,-32768,   239,-32768,-32768,   239,   585,  1314,
    74,   268,   282,    74,    74,-32768,-32768,-32768,-32768,   325,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   302,   324,-32768,-32768,   375,
-32768,-32768,   108,-32768,-32768,   394,-32768,-32768,   419,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   415,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   454,-32768,-32768,-32768,   481,-32768,-32768,-32768,-32768,
    29,   177,   469,   509,   375,   489,-32768,-32768,-32768,-32768,
-32768,   219,   176,-32768,   520,-32768,-32768,   449,   333,   588,
-32768,  1314,-32768,   434,   579,   580,    38,   629,-32768,-32768,
-32768,   696,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   344,-32768,-32768,-32768,   596,   639,   634,   489,   651,-32768,
  1277,-32768,-32768,-32768,-32768,-32768,   293,-32768,-32768,-32768,
-32768,   633,   657,   648,    37,   667,   667,   667,   667,   667,
    37,   673,    80,-32768,   719,   209,   597,   633,-32768,-32768,
   679,   714,   674,   690,-32768,-32768,    80,-32768,-32768,   687,
    30,-32768,   721,   702,   708,-32768,-32768,-32768,-32768,-32768,
    25,-32768,-32768,    53,    45,  1314,   388,   732,-32768,   738,
    28,-32768,-32768,-32768,-32768,-32768,   293,-32768,   688,   633,
   742,   737,   667,   667,   665,   761,-32768,   763,   775,   667,
   667,   667,   851,   217,-32768,-32768,-32768,   430,-32768,   778,
   785,-32768,-32768,-32768,-32768,-32768,   787,-32768,   717,   794,
  1281,-32768,   292,-32768,-32768,-32768,-32768,   798,   804,   800,
   807,-32768,-32768,   791,    37,-32768,-32768,    37,-32768,-32768,
-32768,    52,-32768,   760,-32768,-32768,   542,-32768,-32768,   718,
-32768,   829,   838,   838,   846,   213,   848,   819,-32768,-32768,
-32768,-32768,-32768,   862,   864,-32768,-32768,-32768,-32768,-32768,
-32768,   151,-32768,-32768,   867,   869,   705,-32768,   872,-32768,
   878,   880,-32768,-32768,-32768,-32768,-32768,   888,   895,   667,
-32768,    80,    80,    80,    80,    80,   619,   905,   919,    80,
   283,   891,   926,-32768,   947,-32768,   949,-32768,    28,-32768,
   196,-32768,-32768,-32768,-32768,   806,   863,-32768,-32768,   951,
-32768,-32768,-32768,   379,   215,   941,   489,   489,-32768,-32768,
-32768,-32768,   488,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   562,-32768,-32768,-32768,-32768,-32768,-32768,   352,
   800,-32768,-32768,-32768,-32768,-32768,   424,-32768,-32768,-32768,
   617,   293,    40,-32768,   961,   964,   969,   970,   983,   622,
-32768,    28,    28,   985,   988,    28,    28,    28,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   990,   990,-32768,-32768,   993,
   430,   397,   430,   430,   430,   619,   619,  1001,-32768,  1002,
-32768,-32768,-32768,-32768,   218,   911,-32768,  1119,-32768,   991,
  1007,   710,    37,-32768,-32768,-32768,-32768,-32768,    58,-32768,
-32768,-32768,-32768,   250,-32768,-32768,   597,-32768,   293,-32768,
  1011,-32768,  1012,  1013,-32768,  1014,-32768,  1017,-32768,  1022,
  1023,  1024,-32768,  1025,  1026,  1027,  1028,-32768,-32768,-32768,
-32768,   931,   442,   959,   960,-32768,-32768,   263,-32768,-32768,
-32768,  1018,   293,  1040,  1056,-32768,  1068,   740,-32768,    63,
   115,   141,   154,   622,-32768,-32768,    48,    50,-32768,-32768,
    28,  1057,    28,   890,   830,  1058,   990,  1059,-32768,   317,
-32768,   353,   430,  1060,   430,  1063,   371,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   961,   305,-32768,-32768,-32768,-32768,
   617,-32768,-32768,-32768,  1069,   938,   965,  1033,-32768,-32768,
-32768,    68,   308,   293,-32768,   518,   745,-32768,  1075,-32768,
-32768,  1082,   519,   597,   499,-32768,   609,  1091,  1022,-32768,
  1050,  1093,  1073,  1074,   754,   538,    80,   249,  1097,   442,
    80,    80,-32768,-32768,-32768,-32768,  1098,-32768,-32768,  1020,
-32768,-32768,   293,-32768,  1103,  1104,-32768,  1106,-32768,  1108,
-32768,  1109,  1111,   391,-32768,-32768,-32768,-32768,-32768,  1115,
-32768,  1120,  1124,-32768,-32768,-32768,  1125,-32768,-32768,-32768,
-32768,-32768,-32768,  1054,  1088,   780,-32768,-32768,  1123,-32768,
-32768,-32768,-32768,  1126,-32768,   489,  1134,  1163,  1167,  1076,
-32768,  1169,  1122,-32768,-32768,    86,  1174,  1075,-32768,-32768,
   930,  1180,-32768,   293,-32768,   717,   293,-32768,  1186,-32768,
  1200,-32768,  1128,-32768,-32768,-32768,-32768,   933,  1204,-32768,
-32768,-32768,-32768,-32768,-32768,  1208,  1217,-32768,  1223,  1237,
  1241,   381,-32768,-32768,   427,   483,   466,   479,   782,-32768,
-32768,   293,-32768,  1226,   155,-32768,  1255,-32768,  1259,-32768,
  1263,-32768,-32768,-32768,   348,   398,-32768,-32768,-32768,-32768,
  1253,  1290,-32768,   597,-32768,-32768,  1132,  1147,-32768,   941,
-32768,   953,   717,   987,   717,  1000,   783,  1260,-32768,-32768,
   293,-32768,  1297,-32768,-32768,   399,   464,   961,    71,  1309,
-32768,   293,-32768,  1311,   293,-32768,-32768,  1082,   540,   597,
-32768,   490,-32768,   961,   632,  1312,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,  1296,  1093,  1288,  1293,  1298,-32768,-32768,
-32768,-32768,-32768,    60,  1325,-32768,-32768,  1327,  1151,-32768,
-32768,  1334,  1155,-32768,  1157,-32768,  1159,-32768,    28,    28,
-32768,-32768,   717,  1161,   832,  1322,    37,   315,   961,-32768,
   961,-32768,   961,  1340,  1342,-32768,  1349,   293,-32768,   568,
-32768,-32768,   844,   293,-32768,  1351,  1352,-32768,  1363,  1165,
  1346,  1367,  1184,-32768,  1180,   293,-32768,   717,-32768,  1188,
   859,  1369,  1370,  1371,-32768,  1190,  1192,  1194,  1196,  1198,
-32768,-32768,  1204,-32768,-32768,-32768,-32768,  1372,  1294,-32768,
  1373,  1374,-32768,-32768,  1357,  1376,-32768,  1377,-32768,  1378,
-32768,  1379,  1380,  1382,-32768,  1383,  1384,  1385,-32768,   887,
   892,   894,  1021,-32768,  1034,-32768,  1360,  1386,  1349,  1387,
  1202,   293,-32768,  1389,  1390,-32768,   293,-32768,   158,  1391,
  1351,   578,  1392,  1393,-32768,  1394,  1395,   510,  1396,   293,
-32768,  1398,  1399,  1400,-32768,   500,  1401,  1369,  1388,   170,
    80,    80,    22,    80,  1402,   600,-32768,  1403,-32768,-32768,
-32768,-32768,  1381,  1405,-32768,-32768,  1221,  1225,-32768,-32768,
  1227,-32768,  1229,-32768,  1231,-32768,-32768,   881,  1406,-32768,
-32768,-32768,   717,   717,   783,   961,   961,-32768,  1408,-32768,
-32768,-32768,   293,-32768,   256,  1409,-32768,   332,  1410,-32768,
  1411,  1233,-32768,-32768,   293,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,  1412,  1413,  1235,  1397,   796,  1414,-32768,-32768,
-32768,-32768,   512,   547,-32768,-32768,  1416,   561,-32768,-32768,
  1239,-32768,  1258,  1262,  1264,  1266,-32768,-32768,  1417,  1418,
-32768,  1419,  1420,  1421,  1423,-32768,-32768,-32768,-32768,-32768,
   916,   918,-32768,-32768,  1424,  1268,   961,  1075,-32768,  1082,
-32768,-32768,-32768,   293,-32768,  1270,  1390,-32768,  1425,  1426,
-32768,  1093,-32768,-32768,-32768,-32768,-32768,  1399,  1427,    80,
    80,    80,-32768,-32768,-32768,  1428,  1429,  1272,  1430,  1431,
  1432,  1314,-32768,-32768,-32768,  1276,  1433,  1434,  1436,  1437,
  1180,  1438,  1295,  1439,  1440,  1441,  1442,-32768,  1204,  1443,
  1444,   565,   576,   603,  1299,   605,-32768,-32768,  1445,-32768,
-32768,-32768,-32768,  1301,-32768,-32768,-32768,  1055,-32768,  1446,
  1447,  1448,-32768,  1449,-32768,-32768,  1303,  1450,  1451,-32768,
  1452,  1453,  1454,  1455,-32768,  1456,-32768,-32768,  1075,-32768,
-32768,-32768,-32768,  1457,-32768,-32768,-32768,-32768,-32768,-32768,
   717,  1458,  1305,  1459,  1460,  1462,-32768,  1463,  1464,-32768,
  1461,-32768,-32768,-32768,-32768,  1307,  1465,-32768,  1466,-32768
};

static const short yypgoto[] = {-32768,
-32768,  1308,-32768,   -33,-32768,  1345,-32768,-32768,  1284,-32768,
-32768,  -290,-32768,-32768,   700,-32768,-32768,  -359,-32768,-32768,
-32768,  1302,-32768,-32768,-32768,-32768,-32768,-32768,  1422,  1404,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  1168,
-32768,  -431,  -123,-32768,-32768,-32768,-32768,-32768,-32768,   323,
  -117,  -406,  1061,-32768,  -157,  -269,  -241,  -192,   884,   928,
-32768,   498,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   428,  -663,   566,  -741,  -806,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,  -664,   572,-32768,-32768,
-32768,   889,  1175,-32768,-32768,-32768,-32768,-32768,-32768,   421,
  -676,  -762,   553,  -300,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   882,  1178,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   586,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,  1467,   360,  -154,  -149,-32768,-32768,  -411,
   -65,-32768,-32768,-32768,-32768,-32768,-32768,    10,-32768,-32768,
-32768,-32768,  -224,-32768,-32768,-32768,  1064,  1468,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  -100,-32768,
-32768,-32768,-32768,-32768,  -555,  1009,-32768,  -670,   523,  -547,
  -198,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   929,   755,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   677,-32768,
-32768,-32768,-32768,  1480,-32768,-32768,-32768,  1479,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,  1347,-32768,-32768,-32768,
-32768,  1469,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768
};


#define	YYLAST		1518


static const short yytable[] = {   152,
   258,    77,   375,   755,   396,   764,   325,   646,   760,   192,
     8,   342,   776,   524,   280,   192,   110,   475,   476,   237,
   136,     8,   863,   136,    60,   538,   845,   294,   676,   911,
   322,   316,   154,   388,   388,   214,   195,   248,   249,   250,
   251,    62,    60,   270,    16,   295,    63,   300,   301,   578,
   322,   625,   322,   627,  1159,   371,   155,     2,    17,   209,
   568,     3,     4,   455,   514,   351,   297,   156,     3,     4,
    20,   298,   820,   299,   822,   613,   372,    21,   569,   326,
   373,   867,   254,   834,   614,   328,     3,     4,    31,   302,
   303,   304,   835,   331,   332,   305,   210,   306,   751,   271,
   338,   339,   340,   255,   911,     3,     4,   752,   201,    18,
    19,   256,   257,   657,   658,   192,   136,   623,   868,   508,
   327,   192,   285,     5,   836,   635,   128,   615,   616,   837,
     5,   157,   136,   129,   461,  1005,   617,   296,   323,   324,
   307,    23,   884,   869,   753,   665,   308,    34,     5,   441,
   442,   443,   444,   445,    50,   659,   457,   451,   323,   324,
   323,   324,   619,   420,   366,   367,    51,     5,    35,    26,
   834,    32,   421,   618,   136,   621,   801,   922,   813,   835,
   158,   919,   173,   174,   175,   176,   938,   573,   177,   396,
   440,   695,   310,    16,   244,   159,   160,   178,   462,   620,
   252,   531,   532,   533,   422,   423,   424,   425,   426,   427,
   833,   388,   622,   802,   848,   400,   837,   724,   472,   463,
   286,   554,   172,   179,   180,   181,   851,   265,   697,   182,
   343,   344,   345,   346,   183,   266,   255,   159,   160,   372,
   347,    36,   372,   373,   256,   257,   373,    59,   540,   542,
   543,   545,   547,   570,   513,   498,   499,   500,   501,   488,
   428,   502,   343,   344,   345,   346,   605,    90,   751,   497,
   571,   890,   347,   891,   184,   892,   185,   752,   112,   186,
   187,   188,   189,   190,   191,   113,   452,   372,   396,  1089,
   348,   373,  1028,  1029,   369,    60,   741,   370,  1081,   350,
   724,   724,   626,   626,   361,   119,   630,   388,   632,  1037,
  1038,   661,    62,   362,   753,   534,   349,    63,   889,   612,
   637,   574,   348,    67,    68,    69,    70,   120,    71,    72,
    73,    74,   372,   350,    75,   571,   373,   725,   726,   173,
   174,   175,   176,   342,   834,   177,   566,   118,   349,   567,
   543,   809,   545,   835,   178,   607,   638,   215,   498,   499,
   500,   501,   216,   217,   502,   343,   344,   345,   346,   350,
   537,   537,   372,  1080,   643,   347,   373,   123,   756,   757,
   179,   180,   181,   673,   789,   836,   198,   350,   647,  1040,
   837,   183,   469,   690,   723,   772,   133,   350,   610,   254,
   541,   810,   831,   470,   702,   503,   311,   471,   705,   706,
    62,   192,  1118,   350,   312,    63,   662,  1110,  1031,  1032,
   255,   139,   372,   372,   558,   348,   373,   373,   256,   257,
   790,   184,   254,   185,   644,   645,   186,   187,   188,   189,
   190,   191,   506,   350,   598,   135,   350,   612,   140,    78,
    79,   349,   488,   255,   507,   712,   146,   663,   743,   745,
   747,   256,   257,   497,  1144,   255,   141,   832,   683,   792,
  1146,   537,   565,   256,   257,    56,    57,   142,    85,   143,
    86,   144,   793,   147,   145,    62,   791,   162,   372,  1079,
    63,   165,   373,   849,   173,   174,   175,   176,    62,   350,
   479,   203,   204,    63,    87,   173,   174,   175,   176,   480,
    88,   677,   163,   992,   372,  1054,   762,    89,   373,   765,
   678,   489,   648,   194,   173,   174,   175,   176,   350,   196,
   479,   674,   737,   738,   372,   179,   180,   181,   373,   480,
   675,   481,   482,   483,   484,   485,   486,   179,   180,   181,
  1055,   928,   846,   918,   798,   679,   680,   681,   492,   695,
   379,   847,   380,   350,  1057,   179,   180,   181,  1121,   673,
   936,   487,   482,   483,   484,   485,   486,   350,   853,  1122,
   902,   350,   682,   489,   882,   883,   690,    99,   100,   903,
   985,   199,   350,   828,   696,   799,   697,   698,   803,   986,
   805,   487,   807,  1010,   839,   208,  1123,   842,  1125,   179,
   180,   181,   699,   700,   219,   490,   701,   814,   491,   350,
   492,   493,    60,    61,   372,   894,   896,   928,   373,   372,
   489,   147,   211,   373,   612,   239,   494,   495,   446,    62,
   496,    62,   220,   447,    63,   840,    63,   221,   843,   205,
   206,   854,   612,   489,   223,   850,   179,   180,   181,   242,
   856,   857,   858,   859,   860,   491,   243,   492,   493,   245,
   900,   173,   174,   175,   176,   253,   907,   177,   267,   179,
   180,   181,   272,   494,   495,   852,   178,   496,   920,   284,
   492,   612,   612,   612,   173,   174,   175,   176,    60,   213,
   177,   509,   510,   511,    60,   290,   278,    60,   431,   178,
   853,   901,   179,   180,   181,    62,   273,   274,   333,   334,
    63,    62,   279,   183,    62,   288,    63,   111,   291,    63,
   114,   115,   563,   564,   313,   179,   180,   181,  1003,  1004,
   315,  1008,   517,   611,   973,   329,   183,   576,   664,   981,
    78,    79,    80,    81,    82,   947,   948,    83,   330,   951,
   260,   953,   994,   955,   335,   185,   336,    84,   186,   187,
   188,   189,   190,   191,  1096,  1006,   693,   694,   337,    85,
   352,    86,   517,   733,   794,   795,   184,   353,   185,   354,
   982,   186,   187,   188,   189,   190,   191,   357,   226,   227,
   228,   363,   854,   995,   355,    87,   230,   364,    60,   465,
   365,    88,   368,   261,   381,  1035,   350,   489,    89,  1011,
   376,  1013,  1014,  1015,  1016,    62,   888,  1044,   824,   825,
    63,   382,   612,   612,   794,   886,   173,   174,   175,   176,
   383,   673,   177,   179,   180,   181,   517,   906,   393,   852,
   407,   178,   409,   690,   492,  1052,  1036,   173,   174,   175,
   176,   517,   925,   341,   411,    60,   466,  1092,  1093,  1094,
   429,  1046,   430,   854,   853,   432,   412,   179,   180,   181,
   612,   434,    62,   435,   413,   414,  1082,    63,   183,   517,
   963,   438,    60,   633,   517,   964,   517,   965,   439,    67,
    68,    69,    70,  1068,    71,    72,    73,    74,   449,    62,
    75,   179,   180,   181,    63,  1076,   453,   415,   517,  1074,
   517,  1075,   416,   412,  1026,   707,   708,  1083,   417,   418,
   185,   413,   414,   186,   187,   188,   189,   190,   191,  1025,
    60,   653,   412,   548,   549,   412,  1095,   454,   450,   456,
   413,   414,   459,   413,   414,  1104,   468,    62,   179,   180,
   181,   473,    63,   517,   555,   412,   520,    60,   654,   416,
  1117,   521,   522,   413,   414,   417,   418,   179,   180,   181,
   179,   180,   181,   758,    62,   523,   774,   529,   416,    63,
   530,   416,   535,   561,   417,   418,   539,   417,   418,   412,
   179,   180,   181,   597,   550,   551,   819,   413,   414,  1143,
   562,   416,   412,   576,   580,   581,   583,   417,   418,   585,
   413,   414,    60,   711,   587,   590,   591,   593,   594,   595,
   596,   601,   602,   412,   179,   180,   181,  1156,  1103,    62,
   821,   413,   414,   608,    63,   416,   412,   179,   180,   181,
   606,   417,   418,   823,   413,   414,    60,   731,   416,   609,
   631,   634,   636,   640,   417,   418,   642,   412,   179,   180,
   181,   687,   652,    62,   966,   413,   414,   666,    63,   416,
   412,   179,   180,   181,   671,   417,   418,   967,   413,   414,
    60,   732,   416,   655,   684,   688,   691,   692,   417,   418,
   703,   710,   179,   180,   181,   714,   715,    62,   717,  1129,
   719,   721,    63,   416,   722,   179,   180,   181,   727,   417,
   418,   556,   557,   728,    60,   750,   416,   729,   730,   739,
    60,   771,   417,   418,    60,   816,   742,    67,    68,    69,
    70,    62,    71,    72,    73,    74,    63,    62,    75,    60,
   817,    62,    63,    60,   872,   736,    63,    60,   876,    60,
   878,    60,   880,    60,   885,   744,    62,    60,   914,   746,
    62,    63,   749,   748,    62,    63,    62,   754,    62,    63,
    62,    63,   759,    63,    62,    63,    60,   917,   767,    63,
    60,   924,    60,   931,    60,   932,    60,   933,    60,   934,
    60,   935,   769,    62,    60,   972,   775,    62,    63,    62,
   783,    62,    63,    62,    63,    62,    63,    62,    63,   784,
    63,    62,    63,    60,  1019,   786,    63,    60,  1020,    60,
  1022,    60,  1023,    60,  1024,    60,  1043,    60,  1050,   787,
    62,    60,  1059,   788,    62,    63,    62,   800,    62,    63,
    62,    63,    62,    63,    62,    63,   811,    63,    62,    63,
    60,  1060,   827,    63,    60,  1061,    60,  1062,    60,  1063,
    60,  1078,    60,  1084,    60,  1099,   804,    62,    60,  1105,
   806,    62,    63,    62,   808,    62,    63,    62,    63,    62,
    63,    62,    63,   812,    63,    62,    63,    60,  1112,   830,
    63,    60,  1124,    60,  1128,    60,  1134,    60,  1148,    60,
  1157,   864,   838,   841,    62,   855,   865,   862,    62,    63,
    62,   866,    62,    63,    62,    63,    62,    63,   870,    63,
   871,    63,    67,    68,    69,    70,   875,    71,    72,    73,
    74,   887,   893,    75,   895,    91,    92,    93,    94,    95,
    96,   897,    97,   909,   912,    98,   226,   227,   228,   229,
   226,   227,   228,   358,   230,   231,   913,   915,   230,   359,
   916,   926,   929,   930,   943,   944,   945,   946,   949,   950,
   952,   954,   956,   957,   958,   968,   960,   961,   962,   969,
   971,   975,   978,   101,   983,   987,   988,   990,   991,   993,
   996,   997,  1017,  1000,  1001,  1009,  1012,  1018,   164,  1027,
   693,  1034,  1039,  1041,  1042,  1048,  1049,  1053,  1051,  1056,
  1066,  1067,  1069,  1070,  1071,  1072,   639,  1077,  1086,  1087,
  1091,  1097,  1098,  1100,  1101,  1102,  1106,  1107,  1108,   818,
  1109,  1111,  1113,  1114,  1115,  1116,  1119,  1120,  1126,  1130,
  1131,  1132,  1133,  1135,  1136,  1137,  1138,  1139,  1140,  1142,
  1145,  1147,  1149,  1150,  1151,  1160,  1152,  1153,  1158,   222,
   102,   392,   641,   515,  1085,   977,   984,   685,  1090,   395,
  1002,   704,  1154,   403,   970,   572,   528,  1030,   132,   649,
   815,    22,    24,     0,   212,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   104,   107,   109
};

static const short yycheck[] = {   117,
   193,    35,   293,   668,   305,   676,   231,   555,   672,   133,
     1,   253,   689,   420,   207,   139,    50,   377,   378,   177,
    86,    12,   785,    89,     3,   437,   768,     3,   584,   836,
     3,   230,     4,   303,   304,   153,   137,   187,   188,   189,
   190,    20,     3,   198,    26,    21,    25,     3,     4,   481,
     3,     4,     3,     4,     0,     4,    28,     3,    40,    22,
     3,    32,    33,   354,    25,   258,    14,    39,    32,    33,
    17,    19,   743,    21,   745,    13,    25,    11,    21,   237,
    29,    22,     3,    13,    22,   240,    32,    33,    33,    45,
    46,    47,    22,   243,   244,    51,    59,    53,    13,   200,
   250,   251,   252,    24,   911,    32,    33,    22,   142,    91,
    92,    32,    33,    46,    47,   239,   182,   524,    59,   410,
   238,   245,    93,    94,    54,   537,    19,    65,    66,    59,
    94,   103,   198,    26,   359,   114,    22,   113,   111,   112,
    96,     3,   813,    84,    59,   577,   102,    41,    94,   342,
   343,   344,   345,   346,    81,    88,   355,   350,   111,   112,
   111,   112,    22,    13,   282,   283,    93,    94,    62,     3,
    13,     3,    22,    59,   240,    22,    22,   848,   734,    22,
     4,   845,     7,     8,     9,    10,   863,   478,    13,   490,
   340,    22,   226,    26,   185,    19,    20,    22,     3,    59,
   191,   426,   427,   428,    54,    55,    56,    57,    58,    59,
   758,   481,    59,    59,   770,     3,    59,   624,     4,    24,
   211,     4,     4,    48,    49,    50,   774,    19,    59,    54,
    14,    15,    16,    17,    59,    27,    24,    19,    20,    25,
    24,     3,    25,    29,    32,    33,    29,    26,   441,   442,
   443,   444,   445,     4,   412,     7,     8,     9,    10,   383,
   110,    13,    14,    15,    16,    17,     4,     4,    13,   393,
    21,   819,    24,   821,    99,   823,   101,    22,    11,   104,
   105,   106,   107,   108,   109,     4,     4,    25,   589,  1052,
    74,    29,   963,   964,   285,     3,   656,   288,  1040,    17,
   707,   708,   527,   528,    13,     4,   531,   577,   533,    54,
    55,     4,    20,    22,    59,   433,   100,    25,     4,   518,
     4,   479,    74,    19,    20,    21,    22,     4,    24,    25,
    26,    27,    25,    17,    30,    21,    29,   628,   629,     7,
     8,     9,    10,   585,    13,    13,   464,    23,   100,   467,
   543,     4,   545,    22,    22,   513,     4,    14,     7,     8,
     9,    10,    19,    20,    13,    14,    15,    16,    17,    17,
   436,   437,    25,  1038,     4,    24,    29,     3,   669,   670,
    48,    49,    50,   582,     4,    54,    54,    17,    84,    58,
    59,    59,    14,   592,     4,   686,     3,    17,   516,     3,
     4,     4,     4,    25,   597,    54,    19,    29,   601,   602,
    20,   535,  1089,    17,    27,    25,   574,  1081,   966,   967,
    24,     3,    25,    25,   458,    74,    29,    29,    32,    33,
     4,    99,     3,   101,   552,   553,   104,   105,   106,   107,
   108,   109,    19,    17,     3,    86,    17,   646,    89,    35,
    36,   100,   576,    24,    31,   613,     3,   575,   657,   658,
   659,    32,    33,   587,  1129,    24,    52,     4,   586,     4,
  1141,   537,   463,    32,    33,     3,     4,    63,    64,    65,
    66,    67,     4,     3,    70,    20,     4,    19,    25,  1037,
    25,     3,    29,     4,     7,     8,     9,    10,    20,    17,
    13,    68,    69,    25,    90,     7,     8,     9,    10,    22,
    96,    13,     4,     4,    25,     4,   674,   103,    29,   677,
    22,    22,   556,     4,     7,     8,     9,    10,    17,    81,
    13,    13,   650,   651,    25,    48,    49,    50,    29,    22,
    22,    54,    55,    56,    57,    58,    59,    48,    49,    50,
     4,   852,    13,   844,   712,    57,    58,    59,    59,    22,
    19,    22,    21,    17,     4,    48,    49,    50,     4,   768,
   861,    84,    55,    56,    57,    58,    59,    17,    79,     4,
    13,    17,    84,    22,   809,   810,   785,     3,     4,    22,
    13,     4,    17,   751,    57,   713,    59,    60,   716,    22,
   718,    84,   720,     4,   762,    26,     4,   765,     4,    48,
    49,    50,    75,    76,    19,    54,    79,   735,    57,    17,
    59,    60,     3,     4,    25,   824,   825,   928,    29,    25,
    22,     3,     4,    29,   833,     3,    75,    76,    20,    20,
    79,    20,     4,    25,    25,   763,    25,    14,   766,    71,
    72,   775,   851,    22,     4,   773,    48,    49,    50,     3,
   778,   779,   780,   781,   782,    57,    19,    59,    60,     3,
   828,     7,     8,     9,    10,     3,   834,    13,    82,    48,
    49,    50,     4,    75,    76,    54,    22,    79,   846,     3,
    59,   890,   891,   892,     7,     8,     9,    10,     3,     4,
    13,    85,    86,    87,     3,     4,    33,     3,     4,    22,
    79,   829,    48,    49,    50,    20,     3,     4,    54,    55,
    25,    20,    33,    59,    20,     5,    25,    51,    21,    25,
    54,    55,    23,    24,     3,    48,    49,    50,   931,   932,
     3,   934,     3,     4,   902,     4,    59,     3,     4,   907,
    35,    36,    37,    38,    39,   873,   874,    42,    22,   877,
    42,   879,   920,   881,     4,   101,     4,    52,   104,   105,
   106,   107,   108,   109,  1065,   933,    23,    24,     4,    64,
     3,    66,     3,     4,     3,     4,    99,     3,   101,     3,
   908,   104,   105,   106,   107,   108,   109,     4,    80,    81,
    82,     4,   926,   921,    88,    90,    88,     4,     3,     4,
     4,    96,    22,    95,    97,   973,    17,    22,   103,   937,
    61,   939,   940,   941,   942,    20,   817,   985,    46,    47,
    25,     3,  1031,  1032,     3,     4,     7,     8,     9,    10,
     3,  1040,    13,    48,    49,    50,     3,     4,     3,    54,
     3,    22,    34,  1052,    59,    60,   974,     7,     8,     9,
    10,     3,     4,    13,     3,     3,     4,  1060,  1061,  1062,
     4,   989,     4,   997,    79,     4,    13,    48,    49,    50,
  1079,     4,    20,     4,    21,    22,  1044,    25,    59,     3,
     4,     4,     3,     4,     3,     4,     3,     4,     4,    19,
    20,    21,    22,  1021,    24,    25,    26,    27,     4,    20,
    30,    48,    49,    50,    25,  1033,    26,    54,     3,     4,
     3,     4,    59,    13,   958,   603,   604,  1045,    65,    66,
   101,    21,    22,   104,   105,   106,   107,   108,   109,    59,
     3,     4,    13,   446,   447,    13,  1064,    22,    30,     3,
    21,    22,     4,    21,    22,  1073,     6,    20,    48,    49,
    50,    21,    25,     3,    54,    13,     3,     3,     4,    59,
  1088,     3,     3,    21,    22,    65,    66,    48,    49,    50,
    48,    49,    50,    54,    20,     3,    54,     3,    59,    25,
     3,    59,     3,     3,    65,    66,     4,    65,    66,    13,
    48,    49,    50,    73,     4,     4,    54,    21,    22,  1127,
     4,    59,    13,     3,     3,     3,     3,    65,    66,     3,
    21,    22,     3,     4,     3,     3,     3,     3,     3,     3,
     3,    73,    73,    13,    48,    49,    50,  1155,  1072,    20,
    54,    21,    22,     4,    25,    59,    13,    48,    49,    50,
    33,    65,    66,    54,    21,    22,     3,     4,    59,     4,
     4,     4,     4,     4,    65,    66,     4,    13,    48,    49,
    50,    22,     4,    20,    54,    21,    22,     3,    25,    59,
    13,    48,    49,    50,     3,    65,    66,    54,    21,    22,
     3,     4,    59,    61,     4,     3,    24,    24,    65,    66,
     4,     4,    48,    49,    50,     3,     3,    20,     3,    55,
     3,     3,    25,    59,     4,    48,    49,    50,     4,    65,
    66,     3,     4,     4,     3,     4,    59,     4,     4,     4,
     3,     4,    65,    66,     3,     4,     3,    19,    20,    21,
    22,    20,    24,    25,    26,    27,    25,    20,    30,     3,
     4,    20,    25,     3,     4,    33,    25,     3,     4,     3,
     4,     3,     4,     3,     4,     3,    20,     3,     4,     3,
    20,    25,     4,    98,    20,    25,    20,     4,    20,    25,
    20,    25,     3,    25,    20,    25,     3,     4,     3,    25,
     3,     4,     3,     4,     3,     4,     3,     4,     3,     4,
     3,     4,     3,    20,     3,     4,     3,    20,    25,    20,
     3,    20,    25,    20,    25,    20,    25,    20,    25,     3,
    25,    20,    25,     3,     4,     3,    25,     3,     4,     3,
     4,     3,     4,     3,     4,     3,     4,     3,     4,     3,
    20,     3,     4,     3,    20,    25,    20,    22,    20,    25,
    20,    25,    20,    25,    20,    25,     4,    25,    20,    25,
     3,     4,     3,    25,     3,     4,     3,     4,     3,     4,
     3,     4,     3,     4,     3,     4,    22,    20,     3,     4,
    22,    20,    25,    20,    22,    20,    25,    20,    25,    20,
    25,    20,    25,     4,    25,    20,    25,     3,     4,     3,
    25,     3,     4,     3,     4,     3,     4,     3,     4,     3,
     4,    24,     4,     3,    20,     4,    24,    22,    20,    25,
    20,    24,    20,    25,    20,    25,    20,    25,     4,    25,
     4,    25,    19,    20,    21,    22,     3,    24,    25,    26,
    27,    20,     3,    30,     3,    38,    39,    40,    41,    42,
    43,     3,    45,     3,     3,    48,    80,    81,    82,    83,
    80,    81,    82,    83,    88,    89,     4,    22,    88,    89,
     4,     3,     3,     3,     3,    82,     4,     4,    22,     4,
     4,     4,     4,     4,     3,    26,     4,     4,     4,     4,
     4,     3,     3,    49,     4,     4,     4,     4,     4,     4,
     3,     3,    22,     4,     4,     4,     4,     3,   125,     4,
    23,     4,     4,     4,     4,     4,     4,     4,    22,     4,
     4,     4,     4,     4,     4,     3,   543,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     3,   740,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     3,     0,     4,     4,     4,   168,
    49,   304,   545,   413,  1047,   904,   911,   589,  1058,   305,
   928,   600,    22,   306,   899,   477,   423,   965,    85,   561,
   736,    12,    14,    -1,   148,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    49,    49,    49
};

#line 325 "/usr/local/lib/bison.cc"
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


/* #line 811 "/usr/local/lib/bison.cc" */
#line 2518 "grammar.cc"

  switch (yyn) {

case 17:
#line 130 "pddl2.y"
{
  domain_name = yyvsp[-1].sym->text;
  if (current_file()) domain_file = strdup(current_file());
;
    break;}
case 18:
#line 137 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 19:
#line 138 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 20:
#line 139 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 21:
#line 140 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 22:
#line 141 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 23:
#line 142 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 24:
#line 143 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 25:
#line 144 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 26:
#line 145 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 27:
#line 149 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 28:
#line 150 "pddl2.y"
{ yyval.sym = yyvsp[0].sym; ;
    break;}
case 37:
#line 179 "pddl2.y"
{
  current_param.clear()
;
    break;}
case 38:
#line 183 "pddl2.y"
{
  PredicateSymbol* p = new PredicateSymbol(yyvsp[-3].sym->text);
  p->param = current_param;
  dom_predicates.append(p);
  clear_context(current_param);
  yyvsp[-3].sym->val = p;
;
    break;}
case 39:
#line 194 "pddl2.y"
{
  set_variable_type(current_param, (TypeSymbol*)yyvsp[0].sym->val);
;
    break;}
case 40:
#line 198 "pddl2.y"
{
  current_type_set.clear();
;
    break;}
case 41:
#line 202 "pddl2.y"
{
  set_variable_type(current_param, current_type_set);
;
    break;}
case 42:
#line 206 "pddl2.y"
{
  set_variable_type(current_param, dom_top_type);
;
    break;}
case 44:
#line 214 "pddl2.y"
{
  yyvsp[0].sym->val = new VariableSymbol(yyvsp[0].sym->text);
  current_param.append((VariableSymbol*)yyvsp[0].sym->val);
  if (trace_print_context) {
    std::cerr << "variable ";
    current_param[current_param.length() - 1]->print(std::cerr);
    std::cerr << " added to context (now "
	      << current_param.length() << " variables)"
	      << std::endl;
  }
;
    break;}
case 45:
#line 226 "pddl2.y"
{
  std::cerr << "error: variable ";
  ((VariableSymbol*)yyvsp[0].sym->val)->print(std::cerr);
  std::cerr << " redeclared" << std::endl;
  exit(255);
;
    break;}
case 46:
#line 233 "pddl2.y"
{
  yyvsp[0].sym->val = new VariableSymbol(yyvsp[0].sym->text);
  current_param.append((VariableSymbol*)yyvsp[0].sym->val);
  if (trace_print_context) {
    std::cerr << "variable ";
    current_param[current_param.length() - 1]->print(std::cerr);
    std::cerr << " added to context (now "
	      << current_param.length() << " variables)"
	      << std::endl;
  }
;
    break;}
case 47:
#line 245 "pddl2.y"
{
  std::cerr << "error: variable ";
  ((VariableSymbol*)yyvsp[0].sym->val)->print(std::cerr);
  std::cerr << " redeclared" << std::endl;
  exit(255);
;
    break;}
case 48:
#line 255 "pddl2.y"
{
  current_type_set.append((TypeSymbol*)yyvsp[0].sym->val);
;
    break;}
case 49:
#line 259 "pddl2.y"
{
  current_type_set.append((TypeSymbol*)yyvsp[0].sym->val);
;
    break;}
case 50:
#line 267 "pddl2.y"
{
  last_n_functions = dom_functions.length();
;
    break;}
case 52:
#line 275 "pddl2.y"
{
  last_n_functions = dom_functions.length();
;
    break;}
case 54:
#line 280 "pddl2.y"
{
  TypeSymbol* t = (TypeSymbol*)yyvsp[0].sym->val;
  for (HSPS::index_type k = last_n_functions; k < dom_functions.length(); k++){
    if (write_info) {
      std::cerr << "info: converting ";
      dom_functions[k]->print(std::cerr);
      std::cerr << " to object function with type " << t->print_name
		<< std::endl;
    }
    HSPS::PDDL_Base::ObjectFunctionSymbol* f =
      new HSPS::PDDL_Base::ObjectFunctionSymbol(dom_functions[k]->print_name);
    f->param = dom_functions[k]->param;
    f->sym_types.assign_value(t, 1);
    dom_object_functions.append(f);
    HSPS::StringTable::Cell* c =
      (HSPS::StringTable::Cell*)tab.find(f->print_name);
    if (c == 0) {
      std::cerr << "very bad error: function "
		<< dom_functions[k]->print_name
		<< " declared but not found in string table!"
		<< std::endl;
      exit(255);
    }
    c->val = f;
  }
  dom_functions.set_length(last_n_functions);
  // last_n_functions = dom_functions.length();
;
    break;}
case 56:
#line 310 "pddl2.y"
{
  for (HSPS::index_type k = last_n_functions; k < dom_functions.length(); k++){
    if (write_info) {
      std::cerr << "info: converting ";
      dom_functions[k]->print(std::cerr);
      std::cerr << " to object function with type ";
      current_type_set.write_type(std::cerr);
      std::cerr << std::endl;
    }
    HSPS::PDDL_Base::ObjectFunctionSymbol* f =
      new HSPS::PDDL_Base::ObjectFunctionSymbol(dom_functions[k]->print_name);
    f->param = dom_functions[k]->param;
    f->sym_types.assign_copy(current_type_set);
    dom_object_functions.append(f);
    HSPS::StringTable::Cell* c =
      (HSPS::StringTable::Cell*)tab.find(f->print_name);
    if (c == 0) {
      std::cerr << "very bad error: function "
		<< dom_functions[k]->print_name
		<< " declared but not found in string table!"
		<< std::endl;
      exit(255);
    }
    c->val = f;
  }
  dom_functions.set_length(last_n_functions);
  // last_n_functions = dom_functions.length();
;
    break;}
case 58:
#line 340 "pddl2.y"
{
  last_n_functions = dom_functions.length();
;
    break;}
case 62:
#line 353 "pddl2.y"
{
  current_param.clear();
;
    break;}
case 63:
#line 357 "pddl2.y"
{
  FunctionSymbol* f = new FunctionSymbol(yyvsp[-3].sym->text);
  f->param = current_param;
  dom_functions.append(f);
  clear_context(current_param);
  yyvsp[-3].sym->val = f;
;
    break;}
case 64:
#line 370 "pddl2.y"
{
  current_type_set.clear();
;
    break;}
case 66:
#line 378 "pddl2.y"
{
  // set_type_type(dom_types, (TypeSymbol*)$4->val);
  for (HSPS::index_type k = 0; k < current_type_set.length(); k++)
    current_type_set[k]->sym_types.assign_value((TypeSymbol*)yyvsp[0].sym->val, 1);
  current_type_set.clear();
;
    break;}
case 67:
#line 385 "pddl2.y"
{
  yyvsp[0].sym->val = new TypeSymbol(yyvsp[0].sym->text);
  ((TypeSymbol*)yyvsp[0].sym->val)->sym_types.assign_value(dom_top_type, 1);
  // if (write_warnings)
  // std::cerr << "warning: assuming " << $4->text << " - object" << std::endl;
  // ((TypeSymbol*)$4->val)->sym_types.assign_value(dom_top_type, 1);
  // set_type_type(dom_types, (TypeSymbol*)$4->val);
  dom_types.append((TypeSymbol*)yyvsp[0].sym->val);
  for (HSPS::index_type k = 0; k < current_type_set.length(); k++)
    current_type_set[k]->sym_types.assign_value((TypeSymbol*)yyvsp[0].sym->val, 1);
  current_type_set.clear();
;
    break;}
case 68:
#line 398 "pddl2.y"
{
  // set_type_type(dom_types, dom_top_type);
  for (HSPS::index_type k = 0; k < current_type_set.length(); k++)
    current_type_set[k]->sym_types.assign_value(dom_top_type, 1);
  current_type_set.clear();
;
    break;}
case 70:
#line 409 "pddl2.y"
{
  /* the type is already (implicitly) declared */
  current_type_set.append((TypeSymbol*)yyvsp[0].sym->val);
;
    break;}
case 71:
#line 414 "pddl2.y"
{
  yyvsp[0].sym->val = new TypeSymbol(yyvsp[0].sym->text);
  dom_types.append((TypeSymbol*)yyvsp[0].sym->val);
  current_type_set.append((TypeSymbol*)yyvsp[0].sym->val);
;
    break;}
case 75:
#line 431 "pddl2.y"
{
  set_constant_type(dom_constants, (TypeSymbol*)yyvsp[0].sym->val);
;
    break;}
case 76:
#line 435 "pddl2.y"
{
  set_constant_type(dom_constants, dom_top_type);
;
    break;}
case 78:
#line 443 "pddl2.y"
{
  yyvsp[0].sym->val = new Symbol(yyvsp[0].sym->text);
  if (problem_name) {
    ((Symbol*)yyvsp[0].sym->val)->defined_in_problem = true;
  }
  dom_constants.append((Symbol*)yyvsp[0].sym->val);
;
    break;}
case 79:
#line 451 "pddl2.y"
{
  yyvsp[0].sym->val = new Symbol(yyvsp[0].sym->text);
  if (problem_name) {
    ((Symbol*)yyvsp[0].sym->val)->defined_in_problem = true;
  }
  dom_constants.append((Symbol*)yyvsp[0].sym->val);
;
    break;}
case 80:
#line 459 "pddl2.y"
{
  if (write_warnings) {
    std::cerr << "warning: redeclaration of constant " << yyvsp[0].sym->text
	      << " ignored" << std::endl;
  }
;
    break;}
case 81:
#line 466 "pddl2.y"
{
  if (write_warnings) {
    std::cerr << "warning: redeclaration of constant " << yyvsp[0].sym->text
	      << " ignored" << std::endl;
  }
;
    break;}
case 85:
#line 486 "pddl2.y"
{
  dom_actions.append(new ActionSymbol(yyvsp[0].sym->text));
;
    break;}
case 86:
#line 490 "pddl2.y"
{
  // post-processing should be done on all actions after the complete
  // domain and problem have been read (calling PDDL_Base::post_process())
  clear_context(current_param);
  yyvsp[-3].sym->val = dom_actions[dom_actions.length() - 1];
;
    break;}
case 87:
#line 500 "pddl2.y"
{
  current_param.clear();
;
    break;}
case 88:
#line 504 "pddl2.y"
{
  dom_actions[dom_actions.length() - 1]->param = current_param;
;
    break;}
case 95:
#line 514 "pddl2.y"
{
  // std::cerr << "read assoc string: [" << $3 << "]" << std::endl;
  dom_actions[dom_actions.length() - 1]->assoc = yyvsp[0].sval;
;
    break;}
case 97:
#line 523 "pddl2.y"
{
  SetSymbol* ssym = new SetSymbol(yyvsp[0].sym->text);
  yyvsp[0].sym->val = ssym;
  partitions.append(ssym);
  SetName* s = new SetName(ssym);
  current_atom = s;
;
    break;}
case 98:
#line 531 "pddl2.y"
{
  dom_actions[dom_actions.length() - 1]->part = (SetName*)current_atom;
;
    break;}
case 99:
#line 535 "pddl2.y"
{
  SetName* s = new SetName((SetSymbol*)yyvsp[0].sym->val);
  current_atom = s;
;
    break;}
case 100:
#line 540 "pddl2.y"
{
  dom_actions[dom_actions.length() - 1]->part = (SetName*)current_atom;
;
    break;}
case 110:
#line 562 "pddl2.y"
{
  dom_actions[dom_actions.length() - 1]->num_pre.append(yyvsp[0].rel);
;
    break;}
case 111:
#line 569 "pddl2.y"
{
  yyval.tkw = PDDL_Base::md_start;
;
    break;}
case 112:
#line 573 "pddl2.y"
{
  yyval.tkw = PDDL_Base::md_end;
;
    break;}
case 113:
#line 577 "pddl2.y"
{
  yyval.tkw = PDDL_Base::md_all;
;
    break;}
case 114:
#line 584 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 115:
#line 588 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  dom_actions[dom_actions.length() - 1]->pos_pre.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 116:
#line 594 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-2].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-1].sym->val;
  dom_actions[dom_actions.length() - 1]->pos_pre.append(eq_atom);
;
    break;}
case 117:
#line 602 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, yyvsp[-2].tkw);
;
    break;}
case 118:
#line 606 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  dom_actions[dom_actions.length() - 1]->pos_pre.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 119:
#line 613 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred, yyvsp[-6].tkw);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-3].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-2].sym->val;
  dom_actions[dom_actions.length() - 1]->pos_pre.append(eq_atom);
;
    break;}
case 120:
#line 624 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 121:
#line 628 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  dom_actions[dom_actions.length() - 1]->neg_pre.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 122:
#line 634 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-3].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-2].sym->val;
  dom_actions[dom_actions.length() - 1]->neg_pre.append(eq_atom);
;
    break;}
case 123:
#line 642 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
;
    break;}
case 124:
#line 646 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  dom_actions[dom_actions.length() - 1]->neg_pre.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 125:
#line 653 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred, yyvsp[-9].tkw);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-4].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-3].sym->val;
  dom_actions[dom_actions.length() - 1]->neg_pre.append(eq_atom);
;
    break;}
case 126:
#line 664 "pddl2.y"
{
  if (current_atom != 0) {
    current_atom->param.append((Symbol*)yyvsp[0].sym->val);
  }
;
    break;}
case 128:
#line 674 "pddl2.y"
{
  if (current_atom != 0) {
    current_atom->param.append((Symbol*)yyvsp[0].sym->val);
  }
;
    break;}
case 130:
#line 684 "pddl2.y"
{
  if (yyvsp[0].sym->val == 0) {
    log_error("undeclared variable in atom argument");
  }
  yyval.sym = yyvsp[0].sym;
;
    break;}
case 131:
#line 691 "pddl2.y"
{
  yyval.sym = yyvsp[0].sym;
;
    break;}
case 132:
#line 698 "pddl2.y"
{
  current_atom_stack.append(current_atom);
  current_atom = new FTerm((ObjectFunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 133:
#line 703 "pddl2.y"
{
  ObjectFunctionSymbol* f = (ObjectFunctionSymbol*)yyvsp[-3].sym->val;
  VariableSymbol* v =
    (VariableSymbol*)gensym(sym_variable, "?omsk", f->sym_types);
  v->binding = (FTerm*)current_atom;
  assert(current_atom_stack.length() > 0);
  current_atom = current_atom_stack[current_atom_stack.length() - 1];
  current_atom_stack.dec_length();
  HSPS::StringTable::Cell* c =
    (HSPS::StringTable::Cell*)tab.find(v->print_name);
  if (c == 0) {
    std::cerr << "very bad error: omsk symbol " << v->print_name
	      << " generated but not found in string table!"
	      << std::endl;
    exit(255);
  }
  yyval.sym = c;
;
    break;}
case 134:
#line 724 "pddl2.y"
{
  yyval.sym = yyvsp[0].sym;
;
    break;}
case 135:
#line 728 "pddl2.y"
{
  yyval.sym = yyvsp[0].sym;
;
    break;}
case 136:
#line 734 "pddl2.y"
{
  yyval.rel = new Relation(yyvsp[-3].rkw, yyvsp[-2].exp, yyvsp[-1].exp);
;
    break;}
case 137:
#line 739 "pddl2.y"
{
  yyval.rel = new Relation(yyvsp[-4].rkw, yyvsp[-6].tkw, yyvsp[-3].exp, yyvsp[-2].exp);
;
    break;}
case 138:
#line 746 "pddl2.y"
{
  yyval.rkw = rel_greater;
;
    break;}
case 139:
#line 750 "pddl2.y"
{
  yyval.rkw = rel_greater_equal;
;
    break;}
case 140:
#line 754 "pddl2.y"
{
  yyval.rkw = rel_less;
;
    break;}
case 141:
#line 758 "pddl2.y"
{
  yyval.rkw = rel_less_equal;
;
    break;}
case 142:
#line 762 "pddl2.y"
{
  yyval.rkw = rel_equal;
;
    break;}
case 143:
#line 769 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_sub, new ConstantExpression(0), yyvsp[-1].exp);
;
    break;}
case 144:
#line 773 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_add, yyvsp[-2].exp, yyvsp[-1].exp);
;
    break;}
case 145:
#line 777 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_sub, yyvsp[-2].exp, yyvsp[-1].exp);
;
    break;}
case 146:
#line 781 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_mul, yyvsp[-2].exp, yyvsp[-1].exp);
;
    break;}
case 147:
#line 785 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_div, yyvsp[-2].exp, yyvsp[-1].exp);
;
    break;}
case 148:
#line 789 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_div, yyvsp[-2].exp, yyvsp[0].exp);
;
    break;}
case 149:
#line 793 "pddl2.y"
{
  yyval.exp = new ConstantExpression(yyvsp[0].ival);
;
    break;}
case 150:
#line 797 "pddl2.y"
{
  yyval.exp = new ConstantExpression(NN_TO_N(yyvsp[0].rval));
;
    break;}
case 151:
#line 801 "pddl2.y"
{
  yyval.exp = new TimeExpression();
;
    break;}
case 152:
#line 805 "pddl2.y"
{
  yyval.exp = new PreferenceExpression((Symbol*)yyvsp[-1].sym->val);
;
    break;}
case 153:
#line 809 "pddl2.y"
{
  yyval.exp = yyvsp[0].exp;
;
    break;}
case 154:
#line 816 "pddl2.y"
{
  yyval.exp = yyvsp[0].exp;
;
    break;}
case 155:
#line 820 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_add, yyvsp[-1].exp, yyvsp[0].exp);
;
    break;}
case 156:
#line 827 "pddl2.y"
{
  yyval.exp = yyvsp[0].exp;
;
    break;}
case 157:
#line 831 "pddl2.y"
{
  yyval.exp = new BinaryExpression(exp_mul, yyvsp[-1].exp, yyvsp[0].exp);
;
    break;}
case 158:
#line 838 "pddl2.y"
{
  yyval.exp = new FunctionExpression((FunctionSymbol*)yyvsp[-2].sym->val, yyvsp[-1].lst);
;
    break;}
case 159:
#line 842 "pddl2.y"
{
  yyval.exp = new FunctionExpression((FunctionSymbol*)yyvsp[0].sym->val, 0);
;
    break;}
case 160:
#line 849 "pddl2.y"
{
  yyval.lst = new ListExpression((VariableSymbol*)yyvsp[-1].sym->val, yyvsp[0].lst);
;
    break;}
case 161:
#line 853 "pddl2.y"
{
  yyval.lst = new ListExpression((Symbol*)yyvsp[-1].sym->val, yyvsp[0].lst);
;
    break;}
case 162:
#line 857 "pddl2.y"
{
  yyval.lst = 0;
;
    break;}
case 163:
#line 864 "pddl2.y"
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
;
    break;}
case 164:
#line 869 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 165:
#line 873 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append((Atom*)current_atom);
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
  ((Atom*)current_atom)->pred->pos_pre = true;
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
;
    break;}
case 166:
#line 888 "pddl2.y"
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
;
    break;}
case 167:
#line 893 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
;
    break;}
case 168:
#line 902 "pddl2.y"
{
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
;
    break;}
case 169:
#line 912 "pddl2.y"
{
  current_context = new SetOf();
;
    break;}
case 170:
#line 916 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
;
    break;}
case 171:
#line 921 "pddl2.y"
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
;
    break;}
case 172:
#line 926 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 173:
#line 930 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append((Atom*)current_atom);
  s->set_mode(yyvsp[-12].tkw);
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
  ((Atom*)current_atom)->pred->pos_pre = true;
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
;
    break;}
case 174:
#line 946 "pddl2.y"
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
;
    break;}
case 175:
#line 951 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
;
    break;}
case 176:
#line 960 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->set_mode(yyvsp[-10].tkw);
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
;
    break;}
case 177:
#line 972 "pddl2.y"
{
  current_context = new SetOf();
;
    break;}
case 178:
#line 976 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->set_mode(yyvsp[-7].tkw);
  dom_actions[dom_actions.length() - 1]->set_pre.append(s);
;
    break;}
case 187:
#line 1005 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 188:
#line 1009 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 189:
#line 1016 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 190:
#line 1020 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->neg_atoms.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 191:
#line 1027 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-2].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-1].sym->val;
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append(eq_atom);
;
    break;}
case 192:
#line 1036 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-3].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-2].sym->val;
  SetOf* s = (SetOf*)current_context;
  s->neg_atoms.append(eq_atom);
;
    break;}
case 193:
#line 1074 "pddl2.y"
{
  current_context = new SetOf();
;
    break;}
case 194:
#line 1078 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  dom_actions[dom_actions.length() - 1]->dis_pre.append(s);
  current_context = 0;
;
    break;}
case 196:
#line 1087 "pddl2.y"
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
;
    break;}
case 197:
#line 1092 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
  dom_actions[dom_actions.length() - 1]->dis_pre.append(s);
;
    break;}
case 198:
#line 1101 "pddl2.y"
{
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
;
    break;}
case 204:
#line 1125 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 205:
#line 1129 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  ((SetOf*)current_context)->pos_atoms.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 206:
#line 1135 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-2].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-1].sym->val;
  ((SetOf*)current_context)->pos_atoms.append(eq_atom);
;
    break;}
case 207:
#line 1143 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 208:
#line 1147 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  ((SetOf*)current_context)->neg_atoms.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 209:
#line 1153 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-3].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-2].sym->val;
  ((SetOf*)current_context)->neg_atoms.append(eq_atom);
;
    break;}
case 214:
#line 1174 "pddl2.y"
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
;
    break;}
case 215:
#line 1179 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
;
    break;}
case 216:
#line 1187 "pddl2.y"
{
  dom_actions[dom_actions.length() - 1]->set_eff.append((SetOf*)current_context);
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
;
    break;}
case 217:
#line 1198 "pddl2.y"
{
  current_context = new SetOf();
;
    break;}
case 218:
#line 1202 "pddl2.y"
{
  dom_actions[dom_actions.length() - 1]->set_eff.append((SetOf*)current_context);
  current_context = 0;
;
    break;}
case 219:
#line 1207 "pddl2.y"
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
;
    break;}
case 220:
#line 1212 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
;
    break;}
case 221:
#line 1220 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->set_mode(yyvsp[-10].tkw);
  dom_actions[dom_actions.length() - 1]->set_eff.append(s);
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
;
    break;}
case 222:
#line 1233 "pddl2.y"
{
  current_context = new SetOf();
;
    break;}
case 223:
#line 1237 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  s->set_mode(yyvsp[-6].tkw);
  dom_actions[dom_actions.length() - 1]->set_eff.append(s);
  current_context = 0;
;
    break;}
case 236:
#line 1274 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
  ((PredicateSymbol*)yyvsp[0].sym->val)->modded = true;
;
    break;}
case 237:
#line 1279 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  if (current_context != 0) {
    ((SetOf*)current_context)->pos_atoms.append((Atom*)current_atom);
  }
  else {
    dom_actions[dom_actions.length() - 1]->adds.append((Atom*)current_atom);
  }
;
    break;}
case 238:
#line 1289 "pddl2.y"
{
  current_atom = new FTerm((ObjectFunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 239:
#line 1293 "pddl2.y"
{
  FTerm* ft = (FTerm*)current_atom;
  ft->fun->modded = true;
  VariableSymbol* v =
    (VariableSymbol*)gensym(sym_variable,"?omsk",ft->fun->sym_types);
  v->binding = ft;
  Atom* a = new Atom(dom_assign_pred);
  a->param.set_length(2);
  a->param[0] = v;
  a->param[1] = (Symbol*)yyvsp[-1].sym->val;
  if (current_context != 0) {
    if (write_warnings || !best_effort) {
      std::cerr << "warning: object function assignment ";
      a->print(std::cerr);
      std::cerr << " in quantified/conditional effect ignored"
		<< std::endl;
    }
    if (!best_effort) exit(1);
  }
  else {
    dom_actions[dom_actions.length() - 1]->adds.append(a);
  }
;
    break;}
case 240:
#line 1317 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, yyvsp[-2].tkw);
  ((PredicateSymbol*)yyvsp[0].sym->val)->modded = true;
;
    break;}
case 241:
#line 1322 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  if (current_context != 0) {
    ((SetOf*)current_context)->pos_atoms.append((Atom*)current_atom);
  }
  else {
    dom_actions[dom_actions.length() - 1]->adds.append((Atom*)current_atom);
  }
;
    break;}
case 242:
#line 1335 "pddl2.y"
{
  yyval.sym = yyvsp[0].sym;
;
    break;}
case 243:
#line 1339 "pddl2.y"
{
  yyval.sym = (HSPS::StringTable::Cell*)tab.find("undefined");
  assert(yyval.sym != 0);
;
    break;}
case 244:
#line 1346 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
  ((PredicateSymbol*)yyvsp[0].sym->val)->modded = true;
;
    break;}
case 245:
#line 1351 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  if (current_context != 0) {
    ((SetOf*)current_context)->neg_atoms.append((Atom*)current_atom);
  }
  else {
    dom_actions[dom_actions.length() - 1]->dels.append((Atom*)current_atom);
  }
;
    break;}
case 246:
#line 1361 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
  ((PredicateSymbol*)yyvsp[0].sym->val)->modded = true;
;
    break;}
case 247:
#line 1366 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  if (current_context != 0) {
    ((SetOf*)current_context)->neg_atoms.append((Atom*)current_atom);
  }
  else {
    dom_actions[dom_actions.length() - 1]->dels.append((Atom*)current_atom);
  }
;
    break;}
case 248:
#line 1379 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 249:
#line 1383 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-1].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-1].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->incs.append((FChangeAtom*)current_atom);
;
    break;}
case 250:
#line 1390 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 251:
#line 1394 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-1].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-1].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->decs.append((FChangeAtom*)current_atom);
;
    break;}
case 252:
#line 1401 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 253:
#line 1405 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-1].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-1].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->fass.append((FChangeAtom*)current_atom);
;
    break;}
case 254:
#line 1412 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
;
    break;}
case 255:
#line 1416 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-2].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-2].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->incs.append((FChangeAtom*)current_atom);
;
    break;}
case 256:
#line 1423 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
;
    break;}
case 257:
#line 1427 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-2].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-2].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->decs.append((FChangeAtom*)current_atom);
;
    break;}
case 258:
#line 1434 "pddl2.y"
{
  current_atom = new FChangeAtom((FunctionSymbol*)yyvsp[0].sym->val, yyvsp[-4].tkw);
;
    break;}
case 259:
#line 1438 "pddl2.y"
{
  ((FChangeAtom*)current_atom)->val = yyvsp[-2].exp;
  ((FChangeAtom*)current_atom)->fun->modified = true;
  if (!yyvsp[-2].exp->is_integral()) ((FChangeAtom*)current_atom)->fun->integral = false;
  dom_actions[dom_actions.length() - 1]->fass.append((FChangeAtom*)current_atom);
;
    break;}
case 267:
#line 1464 "pddl2.y"
{
  dom_actions[dom_actions.length() - 1]->dmin = yyvsp[-1].exp;
  dom_actions[dom_actions.length() - 1]->dmax = yyvsp[-1].exp;
;
    break;}
case 268:
#line 1469 "pddl2.y"
{
  dom_actions[dom_actions.length() - 1]->dmin = yyvsp[0].exp;
  dom_actions[dom_actions.length() - 1]->dmax = yyvsp[0].exp;
;
    break;}
case 269:
#line 1477 "pddl2.y"
{
  dom_actions[dom_actions.length() - 1]->dmax = yyvsp[-1].exp;
;
    break;}
case 272:
#line 1489 "pddl2.y"
{
  dom_actions[dom_actions.length() - 1]->dmin = yyvsp[-1].exp;
;
    break;}
case 275:
#line 1501 "pddl2.y"
{
  current_context = new SequentialTaskNet();
  stored_n_param.append(current_param.length());
  if (trace_print_context) {
    std::cerr << "pushed context (" << current_param.length() << " variables)"
	      << std::endl;
  }
;
    break;}
case 276:
#line 1511 "pddl2.y"
{
  SequentialTaskNet* n = (SequentialTaskNet*)current_context;
  n->abs_act = dom_actions[dom_actions.length() - 1];
  dom_actions[dom_actions.length() - 1]->exps.append(n);
  if (trace_print_context) {
    std::cerr << "poping context from "
	      << current_param.length()
	      << " to "
	      << stored_n_param[stored_n_param.length() - 1]
	      << " variables..." << std::endl;
  }
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
;
    break;}
case 279:
#line 1539 "pddl2.y"
{
  current_atom = new Reference((ActionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 280:
#line 1543 "pddl2.y"
{
  Reference* ref = (Reference*)current_atom;
  // ActionSymbol* act = (ActionSymbol*)$2->val;
  // act->refs[act->n_refs] = ref;
  // act->n_refs += 1;
  SequentialTaskNet* task_net = (SequentialTaskNet*)current_context;
  task_net->tasks.append(ref);
;
    break;}
case 282:
#line 1561 "pddl2.y"
{
  problem_name = yyvsp[-1].sym->text;
  if (current_file()) problem_file = strdup(current_file());
;
    break;}
case 299:
#line 1594 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 300:
#line 1598 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  PredicateSymbol* p = (PredicateSymbol*)yyvsp[-3].sym->val;
  if (p->param.length() != current_atom->param.length()) {
    log_error("wrong number of arguments for predicate in (:init ...");
  }
  ((Atom*)current_atom)->insert(p->init);
  current_atom->at_time = 0;
  dom_init.append((Atom*)current_atom);
;
    break;}
case 301:
#line 1612 "pddl2.y"
{
  current_atom = new OInitAtom((ObjectFunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 302:
#line 1616 "pddl2.y"
{
  ObjectFunctionSymbol* f = (ObjectFunctionSymbol*)yyvsp[-5].sym->val;
  if (f->param.length() != current_atom->param.length()) {
    log_error("wrong number of arguments for object function in (:init ...");
  }
  ((OInitAtom*)current_atom)->val = (Symbol*)yyvsp[-1].sym->val;
  current_atom->at_time = 0;
  ((OInitAtom*)current_atom)->insert(f->init);
  dom_obj_init.append((OInitAtom*)current_atom);
;
    break;}
case 303:
#line 1629 "pddl2.y"
{
  current_atom = new FInitAtom((FunctionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 304:
#line 1633 "pddl2.y"
{
  FunctionSymbol* f = (FunctionSymbol*)yyvsp[-5].sym->val;
  if (f->param.length() != current_atom->param.length()) {
    log_error("wrong number of arguments for function in (:init ...");
  }
  ((FInitAtom*)current_atom)->val = NN_TO_N(yyvsp[-1].rval);
  if (!INTEGRAL(((FInitAtom*)current_atom)->val))
    ((FInitAtom*)current_atom)->fun->integral = false;
  ((FInitAtom*)current_atom)->insert(f->init);
  current_atom->at_time = 0;
  dom_fun_init.append((FInitAtom*)current_atom);
;
    break;}
case 305:
#line 1646 "pddl2.y"
{
  FunctionSymbol* f = (FunctionSymbol*)yyvsp[-2].sym->val;
  current_atom = new FInitAtom((FunctionSymbol*)yyvsp[-2].sym->val);
  if (f->param.length() != 0) {
    log_error("wrong number of arguments for function in (:init ...");
  }
  ((FInitAtom*)current_atom)->val = NN_TO_N(yyvsp[-1].rval);
  if (!INTEGRAL(((FInitAtom*)current_atom)->val))
    ((FInitAtom*)current_atom)->fun->integral = false;
  current_atom->at_time = 0;
  ((FInitAtom*)current_atom)->insert(f->init);
  dom_fun_init.append((FInitAtom*)current_atom);
;
    break;}
case 310:
#line 1670 "pddl2.y"
{
  dom_goals.append(yyvsp[0].goal);
;
    break;}
case 311:
#line 1674 "pddl2.y"
{
  Symbol* name = new Symbol(sym_preference, yyvsp[-2].sym->text);
  yyvsp[-2].sym->val = name;
  dom_preferences.append(new Preference(name, yyvsp[-1].goal));
;
    break;}
case 314:
#line 1688 "pddl2.y"
{
  yyval.goal = yyvsp[0].goal;
;
    break;}
case 315:
#line 1692 "pddl2.y"
{
  current_goal.append(new ConjunctiveGoal());
;
    break;}
case 316:
#line 1696 "pddl2.y"
{
  assert(current_goal.length() > 0);
  yyval.goal = current_goal[current_goal.length() - 1];
  current_goal.dec_length();
;
    break;}
case 317:
#line 1702 "pddl2.y"
{
  current_goal.append(new DisjunctiveGoal());
;
    break;}
case 318:
#line 1706 "pddl2.y"
{
  assert(current_goal.length() > 0);
  yyval.goal = current_goal[current_goal.length() - 1];
  current_goal.dec_length();
;
    break;}
case 319:
#line 1715 "pddl2.y"
{
  assert(current_goal.length() > 0);
  ConjunctiveGoal* cg = current_goal[current_goal.length() - 1];
  assert(cg != 0);
  cg->goals.append(yyvsp[-1].goal);
;
    break;}
case 321:
#line 1726 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 322:
#line 1730 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  yyval.goal = new AtomicGoal((Atom*)current_atom, false);
  ((Atom*)current_atom)->pred->pos_pre = true;
  ((Atom*)current_atom)->insert(((Atom*)current_atom)->pred->pos_goal);
;
    break;}
case 323:
#line 1737 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 324:
#line 1741 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  yyval.goal = new AtomicGoal((Atom*)current_atom, true);
  ((Atom*)current_atom)->pred->neg_pre = true;
  ((Atom*)current_atom)->insert(((Atom*)current_atom)->pred->neg_goal);
;
    break;}
case 325:
#line 1748 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-2].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-1].sym->val;
  yyval.goal = new AtomicGoal(eq_atom, false);
;
    break;}
case 326:
#line 1756 "pddl2.y"
{
  yyval.goal = new NumericGoal(yyvsp[0].rel);
;
    break;}
case 327:
#line 1760 "pddl2.y"
{
  yyval.goal = new SimpleSequenceGoal(goal_always, yyvsp[-1].goal);
;
    break;}
case 328:
#line 1764 "pddl2.y"
{
  yyval.goal = new SimpleSequenceGoal(goal_sometime, yyvsp[-1].goal);
;
    break;}
case 329:
#line 1768 "pddl2.y"
{
  yyval.goal = new SimpleSequenceGoal(goal_at_most_once, yyvsp[-1].goal);
;
    break;}
case 330:
#line 1772 "pddl2.y"
{
  yyval.goal = new TriggeredSequenceGoal(goal_sometime_before, yyvsp[-2].goal, yyvsp[-1].goal);
;
    break;}
case 331:
#line 1776 "pddl2.y"
{
  yyval.goal = new TriggeredSequenceGoal(goal_sometime_after, yyvsp[-2].goal, yyvsp[-1].goal);
;
    break;}
case 332:
#line 1780 "pddl2.y"
{
  yyval.goal = new DeadlineGoal(yyvsp[-2].rval, yyvsp[-1].goal);
;
    break;}
case 333:
#line 1784 "pddl2.y"
{
  yyval.goal = new TriggeredDeadlineGoal(yyvsp[-2].goal, yyvsp[-3].rval, yyvsp[-1].goal);
;
    break;}
case 335:
#line 1825 "pddl2.y"
{
  if (metric_type != metric_none) {
    if (write_warnings) {
      std::cerr << "warning: multiple :metric expressions - overwriting previous definition" << std::endl;
    }
  }
  metric_type = metric_minimize;
;
    break;}
case 336:
#line 1834 "pddl2.y"
{
  if (metric_type != metric_none) {
    if (write_warnings) {
      std::cerr << "warning: multiple :metric expressions - overwriting previous definition" << std::endl;
    }
  }
  metric_type = metric_maximize;
;
    break;}
case 337:
#line 1846 "pddl2.y"
{
  if (yyvsp[0].exp->exp_class == exp_time) {
    metric = 0;
    metric_type = metric_makespan;
    yyval.exp = 0;
  }
  else {
    metric = yyvsp[0].exp;
    yyval.exp = yyvsp[0].exp;
  }
;
    break;}
case 338:
#line 1861 "pddl2.y"
{
  serial_length = I_TO_N(yyvsp[-1].ival);
;
    break;}
case 339:
#line 1865 "pddl2.y"
{
  parallel_length = I_TO_N(yyvsp[-1].ival);
;
    break;}
case 340:
#line 1878 "pddl2.y"
{ yyval.rval = N_TO_NN(yyvsp[0].ival); ;
    break;}
case 341:
#line 1879 "pddl2.y"
{ yyval.rval = N_TO_NN(R_TO_N(yyvsp[-2].ival,yyvsp[0].ival)); ;
    break;}
case 342:
#line 1880 "pddl2.y"
{ yyval.rval = yyvsp[0].rval; ;
    break;}
case 343:
#line 1881 "pddl2.y"
{ yyval.rval = POS_INF; ;
    break;}
case 344:
#line 1888 "pddl2.y"
{
  current_item = new DKEL_Item(":invariant");
  current_context = current_item;
;
    break;}
case 346:
#line 1897 "pddl2.y"
{
  dom_sc_invariants.append(new SetConstraint(current_item));
;
    break;}
case 347:
#line 1901 "pddl2.y"
{
  dom_sc_invariants[dom_sc_invariants.length() - 1]->sc_type = yyvsp[-4].sckw;
  dom_sc_invariants[dom_sc_invariants.length() - 1]->sc_count = yyvsp[-3].ival;
  clear_context(current_param);
  current_context = 0;
;
    break;}
case 348:
#line 1908 "pddl2.y"
{
  dom_f_invariants.append(new InvariantFormula(current_item, yyvsp[-1].ff));
  clear_context(current_param);
  current_context = 0;
;
    break;}
case 349:
#line 1917 "pddl2.y"
{
  yyval.ff = new Formula(fc_false);
;
    break;}
case 350:
#line 1921 "pddl2.y"
{
  yyval.ff = new Formula(fc_true);
;
    break;}
case 351:
#line 1925 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 352:
#line 1929 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  yyval.ff = new AFormula((Atom*)current_atom);
;
    break;}
case 353:
#line 1934 "pddl2.y"
{
  yyval.ff = new EqFormula((Symbol*)yyvsp[-2].sym->val, (Symbol*)yyvsp[-1].sym->val);
;
    break;}
case 354:
#line 1938 "pddl2.y"
{
  yyval.ff = new NFormula(yyvsp[-1].ff);
;
    break;}
case 355:
#line 1942 "pddl2.y"
{
  yyval.ff = yyvsp[-1].ff;
  yyval.ff->fc = fc_conjunction;
;
    break;}
case 356:
#line 1947 "pddl2.y"
{
  yyval.ff = yyvsp[-1].ff;
  yyval.ff->fc = fc_conjunction;
;
    break;}
case 357:
#line 1952 "pddl2.y"
{
  yyval.ff = new BFormula(fc_implication, yyvsp[-2].ff, yyvsp[-1].ff);
;
    break;}
case 358:
#line 1956 "pddl2.y"
{
  yyval.ff = new BFormula(fc_equivalence, yyvsp[-2].ff, yyvsp[-1].ff);
;
    break;}
case 359:
#line 1960 "pddl2.y"
{
  stored_n_param.append(current_param.length());
;
    break;}
case 360:
#line 1964 "pddl2.y"
{
  QFormula* qf = new QFormula(fc_universal, yyvsp[-1].ff);
  assert(stored_n_param.length() > 0);
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    qf->vars.append(current_param[k]);
  }
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  yyval.ff = qf;
;
    break;}
case 361:
#line 1979 "pddl2.y"
{
  stored_n_param.append(current_param.length());
;
    break;}
case 362:
#line 1983 "pddl2.y"
{
  QFormula* qf = new QFormula(fc_existential, yyvsp[-1].ff);
  assert(stored_n_param.length() > 0);
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    qf->vars.append(current_param[k]);
  }
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  yyval.ff = qf;
;
    break;}
case 363:
#line 2001 "pddl2.y"
{
  ((CFormula*)yyval.ff)->add(yyvsp[0].ff);
;
    break;}
case 364:
#line 2005 "pddl2.y"
{
  yyval.ff = new CFormula(fc_list);
;
    break;}
case 365:
#line 2012 "pddl2.y"
{
  current_item = new IrrelevantItem();
  if (problem_name) current_item->defined_in_problem = true;
  current_context = current_item;
;
    break;}
case 369:
#line 2027 "pddl2.y"
{
  current_atom = new Reference((ActionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 370:
#line 2031 "pddl2.y"
{
  IrrelevantItem* item = (IrrelevantItem*)current_item;
  item->entity = (Reference*)current_atom;
  dom_irrelevant.append(item);
  ActionSymbol* act = (ActionSymbol*)yyvsp[-4].sym->val;
  act->irr_ins.append(item);
  clear_context(current_param);
  current_context = 0;
;
    break;}
case 371:
#line 2044 "pddl2.y"
{
  current_atom = new Reference((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 372:
#line 2048 "pddl2.y"
{
  IrrelevantItem* item = (IrrelevantItem*)current_item;
  item->entity = (Reference*)current_atom;
  dom_irrelevant.append(item);
  PredicateSymbol* pred = (PredicateSymbol*)yyvsp[-4].sym->val;
  pred->irr_ins.append(item);
  clear_context(current_param);
  current_context = 0;
;
    break;}
case 373:
#line 2061 "pddl2.y"
{
  current_item = new DKEL_Item(":invariant");
  current_item->defined_in_problem = true;
  current_context = current_item;
;
    break;}
case 375:
#line 2071 "pddl2.y"
{
  dom_sc_invariants.append(new SetConstraint(current_item));
;
    break;}
case 376:
#line 2075 "pddl2.y"
{
  dom_sc_invariants[dom_sc_invariants.length() - 1]->sc_type = yyvsp[-4].sckw;
  dom_sc_invariants[dom_sc_invariants.length() - 1]->sc_count = yyvsp[-3].ival;
  clear_context(current_param);
  current_context = 0;
;
    break;}
case 377:
#line 2082 "pddl2.y"
{
  dom_f_invariants.append(new InvariantFormula(current_item, yyvsp[-1].ff));
  clear_context(current_param);
  current_context = 0;
;
    break;}
case 383:
#line 2099 "pddl2.y"
{
  current_item->item_tags.insert(yyvsp[0].sym->text);
;
    break;}
case 384:
#line 2106 "pddl2.y"
{
  yyvsp[0].sym->val = new Symbol(sym_misc, yyvsp[0].sym->text);
  current_item->name = (Symbol*)yyvsp[0].sym->val;
  current_item->item_tags.insert(yyvsp[0].sym->text);
;
    break;}
case 385:
#line 2112 "pddl2.y"
{
  current_item->name = (Symbol*)yyvsp[0].sym->val;
  current_item->item_tags.insert(yyvsp[0].sym->text);
;
    break;}
case 386:
#line 2120 "pddl2.y"
{
  current_param.clear();
;
    break;}
case 387:
#line 2124 "pddl2.y"
{
  current_context->param = current_param;
;
    break;}
case 390:
#line 2136 "pddl2.y"
{
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    current_context->param.append(current_param[k]);
  }
;
    break;}
case 415:
#line 2188 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 416:
#line 2192 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->pos_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 417:
#line 2199 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 418:
#line 2203 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->pos_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 419:
#line 2209 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_init);
;
    break;}
case 420:
#line 2213 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->pos_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 421:
#line 2219 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-2].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-1].sym->val;
  current_context->pos_con.append(eq_atom);
;
    break;}
case 422:
#line 2230 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 423:
#line 2234 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 424:
#line 2241 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 425:
#line 2245 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 426:
#line 2251 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_init);
;
    break;}
case 427:
#line 2255 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 428:
#line 2261 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_init);
;
    break;}
case 429:
#line 2265 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 430:
#line 2271 "pddl2.y"
{
  Atom* eq_atom = new Atom(dom_eq_pred);
  eq_atom->param.set_length(2);
  eq_atom->param[0] = (Symbol*)yyvsp[-3].sym->val;
  eq_atom->param[1] = (Symbol*)yyvsp[-2].sym->val;
  current_context->neg_con.append(eq_atom);
;
    break;}
case 431:
#line 2282 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_pos_goal);
;
    break;}
case 432:
#line 2286 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->pos_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 433:
#line 2292 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_neg_goal);
;
    break;}
case 434:
#line 2296 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->pos_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->pos_pre = true;
;
    break;}
case 435:
#line 2305 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_pos_goal);
;
    break;}
case 436:
#line 2309 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 437:
#line 2315 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val, md_neg_goal);
;
    break;}
case 438:
#line 2319 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_context->neg_con.append((Atom*)current_atom);
  ((Atom*)current_atom)->pred->neg_pre = true;
;
    break;}
case 439:
#line 2328 "pddl2.y"
{
  TypeConstraint* c =
    new TypeConstraint((VariableSymbol*)yyvsp[-1].sym->val, (TypeSymbol*)yyvsp[-2].sym->val);
  current_context->type_con.append(c);
;
    break;}
case 440:
#line 2334 "pddl2.y"
{
  TypeConstraint* c =
    new TypeConstraint((VariableSymbol*)yyvsp[-1].sym->val, (TypeSymbol*)yyvsp[-2].sym->val);
  current_context->type_con.append(c);
;
    break;}
case 441:
#line 2342 "pddl2.y"
{
  yyval.sckw = sc_at_least;
;
    break;}
case 442:
#line 2346 "pddl2.y"
{
  yyval.sckw = sc_at_most;
;
    break;}
case 443:
#line 2350 "pddl2.y"
{
  yyval.sckw = sc_exactly;
;
    break;}
case 447:
#line 2362 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 448:
#line 2366 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  dom_sc_invariants[dom_sc_invariants.length()-1]->pos_atoms.append((Atom*)current_atom);
;
    break;}
case 449:
#line 2371 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 450:
#line 2375 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  dom_sc_invariants[dom_sc_invariants.length()-1]->neg_atoms.append((Atom*)current_atom);
;
    break;}
case 451:
#line 2383 "pddl2.y"
{
  stored_n_param.append(current_param.length());
  current_context = new SetOf();
;
    break;}
case 452:
#line 2388 "pddl2.y"
{
  SetOf* s = (SetOf*)current_context;
  for (HSPS::index_type k = stored_n_param[stored_n_param.length() - 1];
       k < current_param.length(); k++) {
    s->param.append(current_param[k]);
  }
;
    break;}
case 453:
#line 2396 "pddl2.y"
{
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 454:
#line 2400 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  SetOf* s = (SetOf*)current_context;
  s->pos_atoms.append((Atom*)current_atom);
  dom_sc_invariants[dom_sc_invariants.length()-1]->atom_sets.append(s);
  assert(stored_n_param.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = 0;
;
    break;}
case 455:
#line 2417 "pddl2.y"
{
  input_plans.append(new InputPlan());
  if (current_plan_file != current_file()) {
    n_plans_in_current_file = 0;
    current_plan_file = current_file();
  }
;
    break;}
case 456:
#line 2425 "pddl2.y"
{
  if (input_plans[input_plans.length() - 1]->name == 0)
    if (current_plan_file) {
      std::ostringstream pn;
      pn << current_plan_file << ":" << n_plans_in_current_file;
      Symbol* plan_file_name = new Symbol(sym_misc, strdup(pn.str().c_str()));
      input_plans[input_plans.length() - 1]->name = plan_file_name;
    }
  n_plans_in_current_file += 1;
;
    break;}
case 458:
#line 2440 "pddl2.y"
{
  assert(input_plans.length() > 0);
  input_plans[input_plans.length() - 1]->is_opt = true;
;
    break;}
case 461:
#line 2450 "pddl2.y"
{
  assert(input_plans.length() > 0);
  input_plans[input_plans.length() - 1]->name = new Symbol(yyvsp[0].sym->text);
;
    break;}
case 462:
#line 2458 "pddl2.y"
{
  current_atom = new Reference((ActionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 463:
#line 2462 "pddl2.y"
{
  Reference* ref = (Reference*)current_atom;
  ActionSymbol* act = (ActionSymbol*)yyvsp[-3].sym->val;
  act->refs.append(ref);
  assert(input_plans.length() > 0);
  input_plans[input_plans.length() - 1]->
    steps.append(new InputPlanStep(ref, yyvsp[-6].rval));
  clear_context(current_param);
;
    break;}
case 464:
#line 2475 "pddl2.y"
{
  // input_plans.append(0);
  current_plan_file = 0;
;
    break;}
case 465:
#line 2480 "pddl2.y"
{
  // input_plans.append(0);
  current_plan_file = 0;
;
    break;}
case 468:
#line 2493 "pddl2.y"
{
  current_atom = new Reference((ActionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 469:
#line 2497 "pddl2.y"
{
  Reference* ref = (Reference*)current_atom;
  ActionSymbol* act = (ActionSymbol*)yyvsp[-4].sym->val;
  act->refs.append(ref);
  if (input_plans.length() == 0) {
    at_position(std::cerr) << "beginning of new plan" << std::endl;
    input_plans.append(new InputPlan());
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length() - 1]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  else if (current_file() != current_plan_file) {
    at_position(std::cerr) << "beginning of new plan (new file)" << std::endl;
    input_plans.append(new InputPlan());
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length() - 1]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  input_plans[input_plans.length() - 1]->
    steps.append(new InputPlanStep(ref, yyvsp[-7].rval));
  clear_context(current_param);
;
    break;}
case 474:
#line 2537 "pddl2.y"
{
  current_atom = new Reference((ActionSymbol*)yyvsp[0].sym->val);
;
    break;}
case 475:
#line 2541 "pddl2.y"
{
  Reference* ref = (Reference*)current_atom;
  ActionSymbol* act = (ActionSymbol*)yyvsp[-3].sym->val;
  act->refs.append(ref);
  if (input_plans.length() == 0) {
    at_position(std::cerr) << "beginning of new plan" << std::endl;
    input_plans.append(new InputPlan());
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length() - 1]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  else if (current_file() != current_plan_file) {
    at_position(std::cerr) << "beginning of new plan (new file)" << std::endl;
    input_plans.append(new InputPlan());
    if (current_file()) {
      Symbol* plan_file_name = new Symbol(sym_misc, current_file());
      input_plans[input_plans.length() - 1]->name = plan_file_name;
    }
    current_plan_file = current_file();
  }
  HSPS::index_type n = input_plans[input_plans.length() - 1]->steps.length();
  input_plans[input_plans.length() - 1]->
    steps.append(new InputPlanStep(ref, n));
  clear_context(current_param);
;
    break;}
case 479:
#line 2581 "pddl2.y"
{
  current_entry = new HTableEntry();
;
    break;}
case 480:
#line 2585 "pddl2.y"
{
  h_table.append(current_entry);
  current_entry = 0;
;
    break;}
case 481:
#line 2593 "pddl2.y"
{
  current_entry->cost = yyvsp[0].rval;
  current_entry->opt = true;
;
    break;}
case 482:
#line 2598 "pddl2.y"
{
  current_entry->cost = yyvsp[0].rval;
;
    break;}
case 487:
#line 2615 "pddl2.y"
{
  assert(current_entry);
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 488:
#line 2620 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_entry->atoms.append((Atom*)current_atom);
  current_entry->neg.append(false);
  assert(current_entry->atoms.length() == current_entry->neg.length());
;
    break;}
case 489:
#line 2630 "pddl2.y"
{
  assert(current_entry);
  current_atom = new Atom((PredicateSymbol*)yyvsp[0].sym->val);
;
    break;}
case 490:
#line 2635 "pddl2.y"
{
  ((Atom*)current_atom)->check();
  current_entry->atoms.append((Atom*)current_atom);
  current_entry->neg.append(true);
  assert(current_entry->atoms.length() == current_entry->neg.length());
;
    break;}
case 491:
#line 2645 "pddl2.y"
{
  ReferenceSet* set = new ReferenceSet();
  current_context = set;
  stored_n_param.assign_value(0, 1);
  current_param.clear();
  input_sets.append(set);
;
    break;}
case 492:
#line 2653 "pddl2.y"
{
  clear_context(current_param);
  current_context = 0;
;
    break;}
case 493:
#line 2661 "pddl2.y"
{
  yyvsp[0].sym->val = new Symbol(sym_misc, yyvsp[0].sym->text);
  ((ReferenceSet*)current_context)->name = (Symbol*)yyvsp[0].sym->val;
;
    break;}
case 494:
#line 2666 "pddl2.y"
{
  ((ReferenceSet*)current_context)->name = (Symbol*)yyvsp[0].sym->val;
;
    break;}
case 499:
#line 2680 "pddl2.y"
{
  if (yyvsp[0].sym->val) {
    current_atom = new Reference((Symbol*)yyvsp[0].sym->val, false, true);
  }
  else {
    current_atom = new Reference(new Symbol(sym_misc, yyvsp[0].sym->text), false, true);
  }
;
    break;}
case 500:
#line 2689 "pddl2.y"
{
  assert(input_sets.length() > 0);
  assert(input_sets[input_sets.length() - 1] != 0);
  input_sets[input_sets.length() - 1]->
    add(new SimpleReferenceSet((Reference*)current_atom));
;
    break;}
case 501:
#line 2696 "pddl2.y"
{
  assert(input_sets.length() > 0);
  assert(input_sets[input_sets.length() - 1] != 0);
  if (yyvsp[0].sym->val) {
    input_sets[input_sets.length() - 1]->add
      (new SimpleReferenceSet(new Reference((Symbol*)yyvsp[0].sym->val, false, false)));
  }
  else {
    input_sets[input_sets.length() - 1]->add
      (new SimpleReferenceSet(new Reference(new Symbol(sym_misc, yyvsp[0].sym->text), false, false)));
  }
;
    break;}
case 502:
#line 2712 "pddl2.y"
{
  stored_n_param.append(current_param.length());
  stored_context.append(current_context);
  current_context = new SimpleReferenceSet(0);
;
    break;}
case 503:
#line 2718 "pddl2.y"
{
  assert(stored_n_param.length() > 0);
  assert(stored_context.length() > 0);
  clear_context(current_param,
		stored_n_param[stored_n_param.length() - 1],
		current_param.length());
  current_param.set_length(stored_n_param[stored_n_param.length() - 1]);
  stored_n_param.dec_length();
  current_context = stored_context[stored_context.length() - 1];
  stored_context.dec_length();
;
    break;}
case 504:
#line 2733 "pddl2.y"
{
  if (yyvsp[0].sym->val) {
    current_atom = new Reference((Symbol*)yyvsp[0].sym->val, false, true);
  }
  else {
    current_atom = new Reference(new Symbol(sym_misc, yyvsp[0].sym->text), false, true);
  }
;
    break;}
case 505:
#line 2742 "pddl2.y"
{
  SimpleReferenceSet* s = (SimpleReferenceSet*)current_context;
  s->ref = (Reference*)current_atom;
  assert(input_sets.length() > 0);
  assert(input_sets[input_sets.length() - 1] != 0);
  input_sets[input_sets.length() - 1]->add(s);
;
    break;}
case 506:
#line 2750 "pddl2.y"
{
  if (yyvsp[0].sym->val) {
    current_atom = new Reference((Symbol*)yyvsp[0].sym->val, true, true);
  }
  else {
    current_atom = new Reference(new Symbol(sym_misc, yyvsp[0].sym->text), true, true);
  }
;
    break;}
case 507:
#line 2759 "pddl2.y"
{
  SimpleReferenceSet* s = (SimpleReferenceSet*)current_context;
  s->ref = (Reference*)current_atom;
  assert(input_sets.length() > 0);
  assert(input_sets[input_sets.length() - 1] != 0);
  input_sets[input_sets.length() - 1]->add(s);
;
    break;}
}

#line 811 "/usr/local/lib/bison.cc"
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

/* #line 1010 "/usr/local/lib/bison.cc" */
#line 5321 "grammar.cc"
#line 2768 "pddl2.y"

