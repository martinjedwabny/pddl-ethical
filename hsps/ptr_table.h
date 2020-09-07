#ifndef PTR_TABLE_H
#define PTR_TABLE_H

#include "config.h"
#include "index_type.h"

BEGIN_HSPS_NAMESPACE

typedef zero_init_pair<void*> ptr_pair;

class ptr_table {
  void* _key;
  ptr_table* _left;
  ptr_table* _right;
  ptr_table* _up;
  ptr_table* _prev;
  ptr_table* _next;
  index_type _count;

 public:
  void* val;

 private:
  ptr_table(void* k, ptr_table* u, ptr_table* p)
    : _key(k), _left(0), _right(0), _up(u), _prev(p), _next(0),
    _count(1), val(0) { };

 public:

  typedef lvector<void*> key_vec;
  typedef lvector<void*> value_vec;
  typedef lvector<ptr_table*> cell_vec;

  ptr_table()
    : _key(0), _left(0), _right(0), _up(0), _prev(0), _next(0),
    _count(0), val(0) { };
  ~ptr_table();

  void*      key() { return _key; };
  // note: enum_key_first/next enumerate key values only at *this*
  // position in the key sequence (not all keys/key sequences)
  ptr_table* enum_key_first();
  ptr_table* enum_key_next();

  void       key_sequence(key_vec&);
  key_vec*   key_sequence();
  ptr_table* root();

  ptr_table* next() { return _next; };

  ptr_table* insert(void* k);
  ptr_table* insert_next(void* k);
  ptr_table* insert(void** k, index_type l);
  ptr_table* insert(key_vec& k);
  ptr_table* find(void* k);
  ptr_table* find_next(void* k);
  ptr_table* find(void** k, index_type l);
  ptr_table* find(key_vec& k);
  bool   contains(void* k);
  bool   contains(void** k, index_type l);
  bool   contains(key_vec& k);
  void   set(void* k, void* v);
  void   set(void** k, index_type l, void* v);
  void   set(key_vec& k, void* v);
  void*  find_val(void** k, index_type l);
  void*  find_val(key_vec& k);

  void*& operator[](void* k);
  void*& operator[](key_vec& k);

  void dump(cell_vec& vec);
  index_type count_keys();
  void dump_keys(key_vec& vec);
  key_vec* keys();
  index_type count_values();
  void dump_values(value_vec& vec);
  value_vec* values();
};


// inlines

inline bool ptr_table::contains(void* k) {
  return (find(k) != 0);
}

inline bool ptr_table::contains(void** k, index_type l) {
  return (find(k) != 0);
}

inline bool ptr_table::contains(key_vec& k) {
  return (find(k) != 0);
}

inline void ptr_table::set(void* k, void* v) {
  insert(k)->val = v;
}

inline void ptr_table::set(void** k, index_type l, void* v) {
  insert(k)->val = v;
}

inline void ptr_table::set(key_vec& k, void* v) {
  insert(k)->val = v;
}

inline void*& ptr_table::operator[](void* k) {
  ptr_table* a = insert(k);
  return a->val;
}

inline void*& ptr_table::operator[](key_vec& k) {
  ptr_table* a = insert(k);
  return a->val;
}

inline void* ptr_table::find_val(void** k, index_type l) {
  ptr_table* a = find(k, l);
  if (a) return a->val;
  else return 0;
}

inline void* ptr_table::find_val(key_vec& k) {
  ptr_table* a = find(k);
  if (a) return a->val;
  else return 0;
}

END_HSPS_NAMESPACE

#endif
