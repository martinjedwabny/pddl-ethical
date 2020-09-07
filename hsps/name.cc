
#include "name.h"
#include <sstream>

BEGIN_HSPS_NAMESPACE

Name::~Name()
{
  // done
}

const Name* Name::cast_to(const char* cname) const
{
  return 0;
}

::std::string Name::to_string(unsigned int c) const
{
  ::std::ostringstream s;
  write(s, c);
  return s.str();
}

char* Name::to_cstring(unsigned int c) const
{
  ::std::string s = to_string(c);
  return strdup(s.c_str());
}

bool Name::equals(const Name* name) const
{
  return (to_string() == name->to_string());
}

void NameWithContext::write(::std::ostream& s, unsigned int c) const
{
  name->write(s, (c & ~c_off) | c_on);
}

void StringName::write(::std::ostream& s, unsigned int c) const
{
  write_string_escaped(s, _string, c);
}

void ConcatenatedName::write(::std::ostream& s, unsigned int c) const
{
  for (index_type k = 0; k < elements.length(); k++) {
    if (k > 0) {
      write_char_escaped(s, catc, c);
    }
    elements[k]->write(s, c);
  }
}

void EnumName::write(::std::ostream& s, unsigned int c) const
{
  if (!context_is_instance(c)) s << '(';
  write_string_escaped(s, prefix, c);
  s << index;
  if (!context_is_instance(c)) s << ')';
}

void ModName::write(::std::ostream& s, unsigned int c) const
{
  if (context_is_instance(c)) {
    write_string_escaped(s, _mod, c);
    write_char_escaped(s, '_', c);
    _name->write(s, c);
  }
  else {
    write_char_escaped(s, '(', c);
    write_string_escaped(s, _mod, c);
    write_char_escaped(s, ' ', c);
    _name->write(s, c);
    write_char_escaped(s, ')', c);
  }
}

void CopyName::write(::std::ostream& s, unsigned int c) const
{
  write_string_escaped(s, "copy", c);
  if (_num != no_such_index) {
    if (context_is_instance(c)) {
      write_char_escaped(s, '_', c);
      s << _num;
      write_char_escaped(s, '_', c);
    }
    else {
      write_string_escaped(s, " #", c);
      s << _num;
      write_string_escaped(s, " of ", c);
    }
  }
  else {
    if (context_is_instance(c)) {
      write_char_escaped(s, '_', c);
    }
    else {
      write_string_escaped(s, " of ", c);
    }
  }
  _name->write(s, c);
}

void NameAtIndex::write(::std::ostream& s, unsigned int c) const
{
  _name->write(s, c);
  if (_index == no_such_index) {
    if (context_is_instance(c)) {
      write_string_escaped(s, "_at_no_index", c);
    }
    else {
      write_string_escaped(s, " at <no index>", c);
    }
  }
  else {
    if (context_is_instance(c)) {
      write_string_escaped(s, "_at_", c);
    }
    else {
      write_string_escaped(s, " at ", c);
    }
    s << _index;
  }
}

END_HSPS_NAMESPACE
