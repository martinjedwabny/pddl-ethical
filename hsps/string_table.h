#ifndef STRING_TABLE_H
#define STRING_TABLE_H

#include "config.h"
#include "index_type.h"
#include "char_map.h"

BEGIN_HSPS_NAMESPACE

typedef lvector<char*> string_vec;
typedef lvector<void*> element_vec;

inline ::std::ostream& operator<<(::std::ostream& s, const string_vec& vec) {
  s << '[';
  for (index_type k = 0; k < vec.length(); k++) {
    if (k > 0) s << ',';
    s << '"' << vec[k] << '"';
  }
  s << ']';
  return s;
}

class StringTable {
 public:
  struct Cell {
    index_type bin;
    char*      text;
    void*      val;
    Cell*      next;
    Cell(const char* s, char_map& map, index_type b, Cell* n)
      : bin(b), text(map.strdup(s)), val(0), next(n) { };
    Cell(const char* s, index_type len, char_map& map, index_type b, Cell* n)
      : bin(b), text(map.strdup(s, len)), val(0), next(n) { };
    Cell(const char* s, char_map& map, void* v, index_type b, Cell* n)
      : bin(b), text(map.strdup(s)), val(v), next(n) { };
    Cell(const char* s, index_type len, char_map& map, void* v, index_type b, Cell* n)
      : bin(b), text(map.strdup(s, len)), val(v), next(n) { };
    ~Cell() { delete text; if (next) delete next; };
  };

 private:
  index_type n_bin;
  char_map&  map;
  Cell**     table;
  index_type n_entries;

 public:
  StringTable(index_type b, char_map& cm);
  ~StringTable();
  StringTable::Cell* inserta(const char* str);
  StringTable::Cell* inserta(const char* str, index_type len);
  StringTable::Cell* gensym(const char* str);
  char* insert(const char* str);
  char* insert(const char* str, index_type len);
  char* set(const char* str, void* val);
  char* set(const char* str, index_type len, void* val);
  char* set(const char* str)
    { return set(str, (void*)0); };
  char* set(const char* str, index_type len)
    { return set(str, len, (void*)0); };
  const StringTable::Cell* find(const char* str) const;
  const StringTable::Cell* find(const char* str, index_type len) const;
  void* find_val(const char* str) const;
  void* find_val(const char* str, index_type len) const;
  const StringTable::Cell* first() const;
  const StringTable::Cell* next(const StringTable::Cell* c) const;
  string_vec* keys();
  element_vec* values();
  index_type table_bins() { return n_bin; };
  index_type table_entries() { return n_entries; };
  char_map&  table_char_map() { return map; };
};

END_HSPS_NAMESPACE

#endif
