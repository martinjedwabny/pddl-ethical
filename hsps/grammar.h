#ifndef YY_PDDL_Parser_h_included
#define YY_PDDL_Parser_h_included

#line 1 "/usr/local/lib/bison.h"
/* before anything */
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
#endif
#include <stdio.h>

/* #line 14 "/usr/local/lib/bison.h" */
#line 21 "grammar.h"
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

#line 14 "/usr/local/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
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
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_PDDL_Parser_STYPE 
#define YY_PDDL_Parser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_PDDL_Parser_DEBUG
#define  YY_PDDL_Parser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
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

/* #line 63 "/usr/local/lib/bison.h" */
#line 130 "grammar.h"

#line 63 "/usr/local/lib/bison.h"
/* YY_PDDL_Parser_PURE */
#endif

/* #line 65 "/usr/local/lib/bison.h" */
#line 137 "grammar.h"

#line 65 "/usr/local/lib/bison.h"
/* prefix */
#ifndef YY_PDDL_Parser_DEBUG

/* #line 67 "/usr/local/lib/bison.h" */
#line 144 "grammar.h"

#line 67 "/usr/local/lib/bison.h"
/* YY_PDDL_Parser_DEBUG */
#endif
#ifndef YY_PDDL_Parser_LSP_NEEDED

/* #line 70 "/usr/local/lib/bison.h" */
#line 152 "grammar.h"

#line 70 "/usr/local/lib/bison.h"
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

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_PDDL_Parser_PURE
extern YY_PDDL_Parser_STYPE YY_PDDL_Parser_LVAL;
#endif


/* #line 143 "/usr/local/lib/bison.h" */
#line 230 "grammar.h"
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


#line 143 "/usr/local/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
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

/* #line 182 "/usr/local/lib/bison.h" */
#line 387 "grammar.h"
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


#line 182 "/usr/local/lib/bison.h"
 /* decl const */
#else
enum YY_PDDL_Parser_ENUM_TOKEN { YY_PDDL_Parser_NULL_TOKEN=0

/* #line 185 "/usr/local/lib/bison.h" */
#line 508 "grammar.h"
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


#line 185 "/usr/local/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_PDDL_Parser_PARSE(YY_PDDL_Parser_PARSE_PARAM);
 virtual void YY_PDDL_Parser_ERROR(char *msg) YY_PDDL_Parser_ERROR_BODY;
#ifdef YY_PDDL_Parser_PURE
#ifdef YY_PDDL_Parser_LSP_NEEDED
 virtual int  YY_PDDL_Parser_LEX(YY_PDDL_Parser_STYPE *YY_PDDL_Parser_LVAL,YY_PDDL_Parser_LTYPE *YY_PDDL_Parser_LLOC) YY_PDDL_Parser_LEX_BODY;
#else
 virtual int  YY_PDDL_Parser_LEX(YY_PDDL_Parser_STYPE *YY_PDDL_Parser_LVAL) YY_PDDL_Parser_LEX_BODY;
#endif
#else
 virtual int YY_PDDL_Parser_LEX() YY_PDDL_Parser_LEX_BODY;
 YY_PDDL_Parser_STYPE YY_PDDL_Parser_LVAL;
#ifdef YY_PDDL_Parser_LSP_NEEDED
 YY_PDDL_Parser_LTYPE YY_PDDL_Parser_LLOC;
#endif
 int YY_PDDL_Parser_NERRS;
 int YY_PDDL_Parser_CHAR;
#endif
#if YY_PDDL_Parser_DEBUG != 0
public:
 int YY_PDDL_Parser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_PDDL_Parser_CLASS(YY_PDDL_Parser_CONSTRUCTOR_PARAM);
public:
 YY_PDDL_Parser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_PDDL_Parser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_PDDL_Parser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_PDDL_Parser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_PDDL_Parser_DEBUG 
#define YYDEBUG YY_PDDL_Parser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/usr/local/lib/bison.h" */
#line 677 "grammar.h"
#endif
