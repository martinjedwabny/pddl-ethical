%name PDDL_Scanner
%define LEX next_token
%define LEX_PARAM YY_PDDL_Parser_STYPE& val
%define DEBUG 1

%define MEMBERS \
private: \
HSPS::StringTable& _tab; \
bool  _reset; \
const char* _filename; \
int   _line_no; \
bool  _trace_line; \
public: \
void  open_file(const char* fname, bool trace); \
void  close_file(); \
const char* current_file() const { return _filename; }; \
int   current_line() const { return _line_no; }; \

%define CONSTRUCTOR_PARAM HSPS::StringTable& t
%define CONSTRUCTOR_INIT : _tab(t), _reset(false), _filename(0), _line_no(1), _trace_line(false)

%header{
#include "string_table.h"
#include "grammar.h"
%}

/* CHAR [a-zA-Z_] */
CHAR [a-zA-Z_]
DIGIT [0-9]
INT -?{DIGIT}*
FLOAT -?{DIGIT}+(\.{DIGIT}*)?
STRING {CHAR}+(-|\+|\*|=|\.|\&|\/|#|$|%|!|\\|\||~|{CHAR}|{DIGIT})*
WHITESPACE [ \t]+
NL \n
COMMENT ;.*$

%%

[ \t]+       ;
";"[^\n]*    ;
{NL}         { _line_no++; if (_trace_line) std::cerr << std::endl << "LINE: " << _line_no << std::endl; }

\"([^\"]|\\\")*\" {
  val.sval = strndup(yytext + 1, strlen(yytext) - 2);
  return PDDL_Parser::TK_STRING;
}

"(" {return PDDL_Parser::TK_OPEN;}
")" {return PDDL_Parser::TK_CLOSE;}
"[" {return PDDL_Parser::TK_OPEN_SQ;}
"]" {return PDDL_Parser::TK_CLOSE_SQ;}
"+" {return PDDL_Parser::TK_PLUS;}
"-" {return PDDL_Parser::TK_HYPHEN;}
"*" {return PDDL_Parser::TK_MUL;}
"/" {return PDDL_Parser::TK_DIV;}
">" {return PDDL_Parser::TK_GREATER;}
">=" {return PDDL_Parser::TK_GREATEQ;}
"<" {return PDDL_Parser::TK_LESS;}
"<=" {return PDDL_Parser::TK_LESSEQ;}
":" {return PDDL_Parser::TK_COLON;}
"#t" {return PDDL_Parser::TK_HASHT;}
"=" {return PDDL_Parser::TK_EQ;}

":requirements" {return PDDL_Parser::KW_REQS;}
":constants" {return PDDL_Parser::KW_CONSTANTS;}
":predicates" {return PDDL_Parser::KW_PREDS;}
":functions" {return PDDL_Parser::KW_FUNS;}
":types" {return PDDL_Parser::KW_TYPES;}
"define" {return PDDL_Parser::KW_DEFINE;}
"domain" {return PDDL_Parser::KW_DOMAIN;}
":action" {return PDDL_Parser::KW_ACTION;}
":process" {return PDDL_Parser::KW_PROCESS;}
":event" {return PDDL_Parser::KW_EVENT;}
":durative-action" {return PDDL_Parser::KW_ACTION;}
":parameters" {return PDDL_Parser::KW_ARGS;}
":precondition" {return PDDL_Parser::KW_PRE;}
":condition" {return PDDL_Parser::KW_COND;}
"at"{WHITESPACE}"start" {return PDDL_Parser::KW_AT_START;}
"at"{WHITESPACE}"end" {return PDDL_Parser::KW_AT_END;}
"over"{WHITESPACE}"all" {return PDDL_Parser::KW_OVER_ALL;}
":effect" {return PDDL_Parser::KW_EFFECT;}
":invariant" {return PDDL_Parser::KW_INVARIANT;}
":duration" {return PDDL_Parser::KW_DURATION;}
":expansion" {return PDDL_Parser::KW_EXPANSION;}
":tasks" {return PDDL_Parser::KW_TASKS;}
"and" {return PDDL_Parser::KW_AND;}
"or" {return PDDL_Parser::KW_OR;}
"exists" {return PDDL_Parser::KW_EXISTS;}
"forall" {return PDDL_Parser::KW_FORALL;}
"imply" {return PDDL_Parser::KW_IMPLY;}
"iff" {return PDDL_Parser::KW_IFF;}
"not" {return PDDL_Parser::KW_NOT;}
"true" {return PDDL_Parser::KW_TRUE;}
"false" {return PDDL_Parser::KW_FALSE;}
"when" {return PDDL_Parser::KW_WHEN;}
"either" {return PDDL_Parser::KW_EITHER;}
"problem" {return PDDL_Parser::KW_PROBLEM;}
":domain" {return PDDL_Parser::KW_FORDOMAIN;}
":objects" {return PDDL_Parser::KW_OBJECTS;}
":init" {return PDDL_Parser::KW_INIT;}
":goal" {return PDDL_Parser::KW_GOAL;}
":length" {return PDDL_Parser::KW_LENGTH;}
":serial" {return PDDL_Parser::KW_SERIAL;}
":parallel" {return PDDL_Parser::KW_PARALLEL;}
":serial-length" {return PDDL_Parser::KW_SERIAL;}
":parallel-length" {return PDDL_Parser::KW_PARALLEL;}
":metric" {return PDDL_Parser::KW_METRIC;}
"minimize" {return PDDL_Parser::KW_MINIMIZE;}
"maximize" {return PDDL_Parser::KW_MAXIMIZE;}
"?duration" {return PDDL_Parser::KW_DURATION_VAR;}
"total-time" {return PDDL_Parser::KW_TOTAL_TIME;}
"increase"   {return PDDL_Parser::KW_INCREASE;}
"decrease"   {return PDDL_Parser::KW_DECREASE;}
"scale-up"   {return PDDL_Parser::KW_SCALE_UP;}
"scale-down" {return PDDL_Parser::KW_SCALE_DOWN;}
"change"     {return PDDL_Parser::KW_ASSIGN;}
"assign"     {return PDDL_Parser::KW_ASSIGN;}
"number"     {return PDDL_Parser::KW_NUMBER;}
"undefined"  {return PDDL_Parser::KW_UNDEFINED;}

":invariant" {return PDDL_Parser::KW_INVARIANT;}
":irrelevant" {return PDDL_Parser::KW_IRRELEVANT;}
":name" {return PDDL_Parser::KW_NAME;}
":tag" {return PDDL_Parser::KW_TAG;}
":type" {return PDDL_Parser::KW_TAG;}
":kind" {return PDDL_Parser::KW_TAG;}
":vars" {return PDDL_Parser::KW_VARS;}
":context" {return PDDL_Parser::KW_CONTEXT;}
":set-constraint" {return PDDL_Parser::KW_SET_CONSTRAINT;}
":formula" {return PDDL_Parser::KW_FORMULA;}
"at-least-n" {return PDDL_Parser::KW_AT_LEAST_N;}
"at-most-n" {return PDDL_Parser::KW_AT_MOST_N;}
"exactly-n" {return PDDL_Parser::KW_EXACTLY_N;}
"at-least" {return PDDL_Parser::KW_AT_LEAST_N;}
"at-most" {return PDDL_Parser::KW_AT_MOST_N;}
"exactly" {return PDDL_Parser::KW_EXACTLY_N;}
"setof" {return PDDL_Parser::KW_SETOF;}
":fact" {return PDDL_Parser::KW_FACT;}
":assoc" {return PDDL_Parser::KW_ASSOC;}

":plan" {return PDDL_Parser::KW_PLAN;}
":heuristic" {return PDDL_Parser::KW_HEURISTIC;}
":opt" {return PDDL_Parser::KW_OPT;}
":inf" {return PDDL_Parser::KW_INF;}
":set" {return PDDL_Parser::KW_SET;}

"within" {return PDDL_Parser::KW_WITHIN;}
"preference" {return PDDL_Parser::KW_PREFERENCE;}
"is-violated" {return PDDL_Parser::KW_VIOLATED;}

":constraints" { return PDDL_Parser::KW_CONSTRAINTS; }
"always" { return PDDL_Parser::KW_ALWAYS; }
"sometime" { return PDDL_Parser::KW_SOMETIME; }
"at-most-once" { return PDDL_Parser::KW_AT_MOST_ONCE; }
"sometime-before" { return PDDL_Parser::KW_SOMETIME_BEFORE; }
"sometime-after" { return PDDL_Parser::KW_SOMETIME_AFTER; }
"always-within" { return PDDL_Parser::KW_ALWAYS_WITHIN; }

\?{STRING} {
  val.sym = _tab.inserta(yytext);
  if (val.sym->val == 0) return PDDL_Parser::TK_NEW_VAR_SYMBOL;
  if (((HSPS::PDDL_Base::Symbol*)val.sym->val)->sym_class == HSPS::PDDL_Base::sym_variable)
    return PDDL_Parser::TK_VAR_SYMBOL;
  return PDDL_Parser::TK_NEW_VAR_SYMBOL;
}

\:{STRING} {
  val.sym = _tab.inserta(yytext);
  return PDDL_Parser::TK_KEYWORD;
}

{STRING} {
  val.sym = _tab.inserta(yytext);
  if (val.sym->val == 0) return PDDL_Parser::TK_NEW_SYMBOL;
  else {
    if (yy_flex_debug) {
      HSPS::PDDL_Base::Symbol* s = (HSPS::PDDL_Base::Symbol*)val.sym->val;
      std::cerr << "symbol " << s->print_name
		<< " has class " << s->sym_class
		<< std::endl;
    }
    switch (((HSPS::PDDL_Base::Symbol*)val.sym->val)->sym_class) {
    case HSPS::PDDL_Base::sym_object:
      return PDDL_Parser::TK_OBJ_SYMBOL;
    case HSPS::PDDL_Base::sym_typename:
      return PDDL_Parser::TK_TYPE_SYMBOL;
    case HSPS::PDDL_Base::sym_predicate:
      return PDDL_Parser::TK_PRED_SYMBOL;
    case HSPS::PDDL_Base::sym_object_function:
      return PDDL_Parser::TK_OBJFUN_SYMBOL;
    case HSPS::PDDL_Base::sym_function:
      return PDDL_Parser::TK_FUN_SYMBOL;
    case HSPS::PDDL_Base::sym_action:
      return PDDL_Parser::TK_ACTION_SYMBOL;
    case HSPS::PDDL_Base::sym_preference:
      return PDDL_Parser::TK_PREFERENCE_SYMBOL;
    case HSPS::PDDL_Base::sym_set:
      return PDDL_Parser::TK_SET_SYMBOL;
    default:
      return PDDL_Parser::TK_MISC_SYMBOL;
    }
  }
}

{INT}   val.ival = atoi(yytext); return PDDL_Parser::TK_INT;
{FLOAT} val.rval = N_TO_NN(A_TO_N(yytext)); return PDDL_Parser::TK_FLOAT;

%% 

int yywrap() {
  return 1;
}

void PDDL_Scanner::open_file(const char* name, bool trace) {
  yy_flex_debug = trace;
  yyin = fopen(name, "r");
  if (!yyin) {
    std::cerr << "error: can't open " << name << std::endl;
    exit(255);
  }
  _filename = name;
  if (_reset) yy_init_buffer(YY_PDDL_Scanner_CURRENT_BUFFER, yyin);
  _reset = true;
  _line_no = 1;
  _trace_line = trace;
}

void PDDL_Scanner::close_file() {
  if (_filename != 0) {
    fclose(yyin);
    _filename = 0;
  }
}
