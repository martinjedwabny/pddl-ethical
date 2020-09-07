#ifndef CHAR_MAP_H
#define CHAR_MAP_H

#include "config.h"
#include "index_type.h"

BEGIN_HSPS_NAMESPACE

class char_map {
 public:
  static const unsigned int _CHAR_COUNT = 256;
 private:
  char _map[_CHAR_COUNT];
 public:
  char_map();
  char_map(const char in[_CHAR_COUNT]);

  void identify(const char _c0, const char _c1);

  char operator[](const char _in) const
    { return _map[(unsigned char)_in]; };

  void  apply(char* s) const;
  void  apply(char* s, index_type len) const;
  int   strcmp(const char *s0, const char *s1) const;
  int   strcmp(const char *s0, const char *s1, index_type len) const;
  const char* strchr(const char* s, char c) const;
  const char* strchr(const char* s, index_type len, char c) const;
  char* strcpy(const char *s0, char *s1) const;
  char* strcpy(const char *s0, index_type len, char *s1) const;
  char* strdup(const char *s0) const;
  char* strdup(const char *s0, index_type len) const;
  index_type hash(const char *s) const;
  index_type hash(const char *s, index_type len) const;
};

extern const char lowercase[char_map::_CHAR_COUNT];
// extern const char uppercase[char_map::_CHAR_COUNT];
extern char_map lowercase_map;
// extern char_map uppercase_map;

END_HSPS_NAMESPACE

#endif
