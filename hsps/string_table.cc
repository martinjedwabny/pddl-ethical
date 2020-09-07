
#include "string_table.h"
#include <sstream>

BEGIN_HSPS_NAMESPACE

StringTable::StringTable(index_type b, char_map& cm)
  : n_bin(b), map(cm), table(0), n_entries(0)
{
  table = new StringTable::Cell*[n_bin];
  for (index_type k = 0; k < n_bin; k++) table[k] = 0;
}

StringTable::~StringTable() {
  for (index_type k = 0; k < n_bin; k++)
    if (table[k]) delete table[k];
  delete [] table;
}

StringTable::Cell* StringTable::gensym(const char* str)
{
  Cell* sc = inserta(str);
  index_type i = 0;
  while (sc->val) {
    ::std::ostringstream s;
    s << str << i++;
    sc = inserta(s.str().c_str());
  }
  return sc;
}

StringTable::Cell* StringTable::inserta(const char* str) {
  index_type l = map.hash(str) % n_bin;
  StringTable::Cell **sc = &(table[l]);
  while (1) {
    if (!*sc) {
      *sc = new StringTable::Cell(str, map, 0, l, 0);
      n_entries += 1;
      return *sc;
    }
    else {
      int d = map.strcmp((*sc)->text, str);
      if (d == 0) {
	return *sc;
      }
      else if (d < 0) sc = &((*sc)->next);
      else {
	*sc = new StringTable::Cell(str, map, 0, l, *sc);
	n_entries += 1;
	return *sc;
      }
    }
  }
}

StringTable::Cell* StringTable::inserta(const char* str, index_type len) {
  index_type l = map.hash(str, len) % n_bin;
  StringTable::Cell **sc = &(table[l]);
  while (1) {
    if (!*sc) {
      *sc = new StringTable::Cell(str, map, 0, l, 0);
      n_entries += 1;
      return *sc;
    }
    else {
      int d = map.strcmp((*sc)->text, str, len);
      if (d == 0) {
	return *sc;
      }
      else if (d < 0) sc = &((*sc)->next);
      else {
	*sc = new StringTable::Cell(str, len, map, 0, l, *sc);
	n_entries += 1;
	return *sc;
      }
    }
  }
}

char* StringTable::insert(const char* str) {
  StringTable::Cell *sc = inserta(str);
  return sc->text;
}

char* StringTable::insert(const char* str, index_type len) {
  StringTable::Cell *sc = inserta(str, len);
  return sc->text;
}

char* StringTable::set(const char* str, void* val) {
  StringTable::Cell *sc = inserta(str);
  sc->val = val;
  return sc->text;
}

char* StringTable::set(const char* str, index_type len, void* val) {
  StringTable::Cell *sc = inserta(str, len);
  sc->val = val;
  return sc->text;
}

const StringTable::Cell* StringTable::find(const char* str) const {
  index_type l = map.hash(str) % n_bin;
  StringTable::Cell **sc = &(table[l]);
  while (1) {
    if (!*sc) {
      return 0;
    }
    else {
      int d = map.strcmp((*sc)->text, str);
      if (d == 0) return *sc;
      else if (d < 0) sc = &((*sc)->next);
      else return 0;
    }
  }
}

const StringTable::Cell* StringTable::find(const char* str,
					   index_type len) const {
  index_type l = map.hash(str, len) % n_bin;
  StringTable::Cell **sc = &(table[l]);
  while (1) {
    if (!*sc) {
      return 0;
    }
    else {
      int d = map.strcmp((*sc)->text, str, len);
      if (d == 0) return *sc;
      else if (d < 0) sc = &((*sc)->next);
      else return 0;
    }
  }
}

void* StringTable::find_val(const char* str) const {
  const StringTable::Cell* sc = find(str);
  if (sc) return sc->val;
  else return (void*)0;
}

void* StringTable::find_val(const char* str, index_type len) const {
  const StringTable::Cell* sc = find(str, len);
  if (sc) return sc->val;
  else return (void*)0;
}

const StringTable::Cell* StringTable::first() const {
  for (index_type k = 0; k < n_bin; k++)
    if (table[k]) return table[k];
  return 0;
}

const StringTable::Cell* StringTable::next(const StringTable::Cell* c) const {
  if (c->next) return c->next;
  for (index_type k = c->bin + 1; k < n_bin; k++)
    if (table[k]) return table[k];
  return 0;
}

string_vec* StringTable::keys() {
  string_vec* vec = new string_vec();
  for (const StringTable::Cell* sc = first(); sc; sc = next(sc))
    vec->append((char*)sc->text);
  return vec;
}

element_vec* StringTable::values() {
  element_vec* vec = new element_vec();
  for (const StringTable::Cell* sc = first(); sc; sc = next(sc))
    if (sc->val) vec->append((void*)sc->val);
  return vec;
}

END_HSPS_NAMESPACE
