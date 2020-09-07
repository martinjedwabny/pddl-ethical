#ifndef PARSER_H

#include "config.h"
#include "scanner.h"
#include "grammar.h"

BEGIN_HSPS_NAMESPACE

class Parser : public PDDL_Parser {
  PDDL_Scanner scanner;
 public:
  Parser(StringTable& t) : PDDL_Parser(t), scanner(t) { };

  void read(char *name, bool trace);

  virtual int next_token();
  virtual void log_error(char* msg);
  virtual std::ostream& at_position(std::ostream& s);
  virtual char* current_file();
};

END_HSPS_NAMESPACE

#endif
