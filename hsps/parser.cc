
#include <stdlib.h>
#include <stdio.h>
#include "parser.h"

BEGIN_HSPS_NAMESPACE

void Parser::read(char *name, bool trace) {
  yydebug = trace;
  scanner.open_file(name, trace);
  error_flag = false;
  int res = yyparse();
  scanner.close_file();
  if (error_flag || (res > 0)) {
    std::cerr << "res = " << res << std::endl;
    exit(255);
  }
}

int Parser::next_token() {
  return scanner.next_token(yylval);
}

void Parser::log_error(char* msg) {
  at_position(std::cerr << "error ") << msg << std::endl;
  error_flag = true;
}

std::ostream& Parser::at_position(std::ostream& s) {
  if (scanner.current_file()) {
    return s << "at " << scanner.current_file() << ":"
	     << scanner.current_line() << ": ";
  }
  else {
    return s << "at " << scanner.current_line() << ": ";
  }
}

char* Parser::current_file()
{
  return scanner.current_file();
}

END_HSPS_NAMESPACE
