#ifndef NAME_H
#define NAME_H

#include "config.h"
#include "index_type.h"
#include <string>
#include <iostream>

BEGIN_HSPS_NAMESPACE

class Name {
 public:
  static const unsigned int NC_DEFAULT = 0;
  static const unsigned int NC_INSTANCE = 1;
  static const unsigned int NC_DOMAIN = 2;
  static const unsigned int NC_PROBLEM = 4;
  static const unsigned int NC_PLAN = 8;
  static const unsigned int NC_ESCAPE = 16 + 32;
  static const unsigned int NC_PDDL = 16;
  static const unsigned int NC_XML = 32;
  static const unsigned int NC_LATEX = 16 + 32;
  static const unsigned int NC_IPC = 64;

  static bool context_is_instance(unsigned int c)
    { return ((c & NC_INSTANCE) == NC_INSTANCE); };
  static bool context_is_domain(unsigned int c)
    { return ((c & NC_DOMAIN) == NC_DOMAIN); };
  static bool context_is_problem(unsigned int c)
    { return ((c & NC_PROBLEM) == NC_PROBLEM); };
  static bool context_is_plan(unsigned int c)
    { return ((c & NC_PLAN) == NC_PLAN); };
  static bool escape_for_pddl(unsigned int c)
    { return ((c & NC_ESCAPE) == NC_PDDL); };
  static bool escape_for_xml(unsigned int c)
    { return ((c & NC_ESCAPE) == NC_XML); };
  static bool escape_for_latex(unsigned int c)
    { return ((c & NC_ESCAPE) == NC_LATEX); };
  static bool conform_to_IPC(unsigned int c)
    { return ((c & NC_IPC) == NC_IPC); };

  virtual ~Name();
  virtual void write(::std::ostream& s, unsigned int c = NC_DEFAULT) const = 0;
  virtual const Name* cast_to(const char* cname) const;

  ::std::string to_string(unsigned int c = NC_DEFAULT) const;
  char*       to_cstring(unsigned int c = NC_DEFAULT) const;
  bool equals(const Name* name) const;

  void write_char_escaped(::std::ostream& s,
			  char ch,
			  unsigned int c) const;
  void write_string_escaped(::std::ostream& s,
			    ::std::string& str,
			    unsigned int c) const;
  void write_string_escaped(::std::ostream& s,
			    const char* str,
			    unsigned int c) const;
};

typedef lvector<const Name*>  name_vec;

inline ::std::ostream& operator<<(::std::ostream& s, const Name& n) {
  n.write(s, false);
  return s;
}

inline ::std::ostream& operator<<(::std::ostream& s, const Name* n) {
  n->write(s, false);
  return s;
}

inline void Name::write_char_escaped
(::std::ostream& s, char ch, unsigned int c) const
{
  if (escape_for_latex(c)) {
    if (ch == '_') s << '\\' << '_';
    else if (ch == '%') s << '\\' << '%';
    else if (ch == '#') s << '\\' << '#';
    else s << ch;
  }
  else if (escape_for_xml(c)) {
    if (ch == '<') s << "&lt;";
    else if (ch == '>') s << "&gt;";
    else if (ch == '&') s << "&amp;";
    else if (ch == '"') s << "&quot;";
    else s << ch;
  }
  else if (escape_for_pddl(c)) {
    if (ch == ' ') s << '_';
    else s << ch;
  }
  else {
    s << ch;
  }
}

inline void Name::write_string_escaped
(::std::ostream& s, const char* str, unsigned int c) const
{
  while (*str) {
    write_char_escaped(s, *str, c);
    str++;
  }
}

inline void Name::write_string_escaped
(::std::ostream& s, ::std::string& str, unsigned int c) const
{
  for (index_type i = 0; i < str.length(); i++)
    write_char_escaped(s, str[i], c);
}

class NameWithContext : public Name {
  const Name*  name;
  unsigned int c_on;
  unsigned int c_off;
 public:
  NameWithContext(const Name* n, unsigned int on, unsigned int off)
    : name(n), c_on(on), c_off(off) { };
  virtual ~NameWithContext() { };
  virtual void write(::std::ostream& s, unsigned int c = NC_DEFAULT) const;
};

class StringName : public Name {
  const char* _string;
  bool        _own;
 public:
  StringName(const char* s, bool c = false)
    : _string(c ? strdup(s) : s), _own(c) { };
  virtual ~StringName() { if (_own) delete (char*)_string; };
  virtual void write(::std::ostream& s, unsigned int c = NC_DEFAULT) const;
};

class ConcatenatedName : public Name {
  name_vec elements;
  char     catc;
 public:
  ConcatenatedName() : elements((Name*)0, 0), catc('+') { };
  ConcatenatedName(const Name* n) : elements(n, 1), catc('+') { };
  ConcatenatedName(char c) : elements((Name*)0, 0), catc(c) { };
  ConcatenatedName(const Name* n1, const Name* n2, char c)
    : elements((Name*)0, 0), catc(c)
  {
    elements.append(n1);
    elements.append(n2);
  };
  virtual ~ConcatenatedName() { };
  virtual void write(::std::ostream& s, unsigned int c = NC_DEFAULT) const;
  void append(const Name* n) { elements.append(n); };
};

class ModName : public Name {
  const char* _mod;
  const Name* _name;
 public:
  ModName(const Name* n, const char* m) : _mod(m), _name(n) { };
  virtual ~ModName() { };
  virtual void write(::std::ostream& s, unsigned int c = NC_DEFAULT) const;
};

class CopyName : public Name {
  const Name* _name;
  index_type  _num;
 public:
  CopyName(const Name* n) : _name(n), _num(no_such_index) { };
  CopyName(const Name* n, index_type m) : _name(n), _num(m) { };
  virtual ~CopyName() { };
  virtual void write(::std::ostream& s, unsigned int c = NC_DEFAULT) const;
};

class NameAtIndex : public Name {
  const Name* _name;
  index_type  _index;
 public:
  NameAtIndex(const Name* n, index_type i) : _name(n), _index(i) { };
  virtual ~NameAtIndex() { };
  virtual void write(::std::ostream& s, unsigned int c = NC_DEFAULT) const;
};

class EnumName : public Name {
  const char* prefix;
  index_type  index;
 public:
  EnumName(const char* p, index_type i) : prefix(p), index(i) { };
  virtual ~EnumName() { };
  virtual void write(::std::ostream& s, unsigned int c = NC_DEFAULT) const;
};

END_HSPS_NAMESPACE

#endif
