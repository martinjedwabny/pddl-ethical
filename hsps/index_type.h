#ifndef VECTOR_H
#define VECTOR_H

#include "config.h"
#include <limits.h>
#include <vector>
#include <utility>
#include <iostream>

BEGIN_HSPS_NAMESPACE

typedef unsigned int index_type;
const index_type index_type_max = (UINT_MAX - 1);
#define INDEX_TYPE_BITS 32
const index_type LARGE_PRIME = 2147483629U;
const index_type no_such_index = UINT_MAX;

typedef unsigned long count_type;
const count_type count_type_max = ULONG_MAX;

template<class T> class swapable_pair : public std::pair<T, T>
{
 public:
  swapable_pair()
    : std::pair<T, T>() { };
  swapable_pair(const T& v1, const T& v2)
    : std::pair<T, T>(v1, v2) { };
  swapable_pair(const T& v)
    : std::pair<T, T>(v, v) { };
  swapable_pair(const swapable_pair& p)
    : std::pair<T, T>(p) { };

  void swap();
};

template<class T> class comparable_pair : public swapable_pair<T>
{
 public:
  comparable_pair()
    : swapable_pair<T>() { };
  comparable_pair(const T& v1, const T& v2)
    : swapable_pair<T>(v1, v2) { };
  comparable_pair(const T& v)
    : swapable_pair<T>(v) { };
  comparable_pair(const comparable_pair& p)
    : swapable_pair<T>(p) { };

  void sort_ascending();
  void sort_descending();
};

typedef comparable_pair<index_type> index_pair;

template<class T> class zero_init_pair : public comparable_pair<T>
{
 public:
  zero_init_pair()
    : comparable_pair<T>(0) { };
  zero_init_pair(const T& v1, const T& v2)
    : comparable_pair<T>(v1, v2) { };
  zero_init_pair(const T& v)
    : comparable_pair<T>(v) { };
  zero_init_pair(const zero_init_pair& p)
    : comparable_pair<T>(p) { };
};


// forward declarations of the two representations of "set of indices"
// - they are used as arguments to some lvector methods
class index_set;
class bool_vec;

template<class T> class lvector : public std::vector<T>
{
 public:
  lvector() : std::vector<T>() { };
  lvector(const T& v, index_type l) : std::vector<T>(l, v) { };
  lvector(const lvector<T>& vec) : std::vector<T>(vec) { };
  // lvector(const T* arr, index_type n) : std::vector<T>(n) {
  //   for (index_type k = 0; k < n; k++) (*this)[k] = arr[k];
  // };

  class element_reference {
    lvector*   _vec;
    index_type _pos;
  public:
    element_reference() : _vec(0), _pos(no_such_index) { };
    element_reference(lvector& v, index_type p) : _vec(&v), _pos(p) { };
    operator T*() const {
      if (_vec == 0) return 0;
      return &((*_vec)[_pos]);
    };
  };

  class order {
   public:
    virtual bool operator()(const T& v0, const T& v1) const = 0;
  };

  index_type length() const;

#ifdef CHECK_VECTOR_INDEX
  typename std::vector<T>::reference operator[](typename std::vector<T>::size_type k) {
    assert(k < std::vector<T>::size());
    return std::vector<T>::operator[](k);
  };

  typename std::vector<T>::const_reference operator[](typename std::vector<T>::size_type k) const {
    assert(k < std::vector<T>::size());
    return std::vector<T>::operator[](k);
  };
#endif

  bool contains(const T& v) const;
  index_type first(const T& v) const;
  index_type next(const T& v, index_type i) const;
  index_type find(const T& v, bool_vec& s) const;
  index_type count(const T& v) const;
  index_type arg_max() const;
  index_type arg_min() const;
  index_type arg_first(const order& o) const;
  index_type arg_last(const order& o) const;

  index_pair first_common(const lvector<T>& vec) const;
  index_pair next_common(const lvector<T>& vec, index_pair p) const;
  void difference(const lvector& v1, lvector& d0, lvector& d1);

  bool operator==(const lvector& _vec) const;
  bool operator!=(const lvector& _vec) const;
  bool operator<(const lvector& vec) const;
  bool operator>(const lvector& vec) const;
  bool operator<=(const lvector& vec) const;
  bool operator>=(const lvector& vec) const;

  void assign_copy(const lvector& _vec);
  void assign_copy(const T* _arr, index_type n);
  void assign_value(const T& val);
  void assign_value(const T& val, index_type l);

  // note: map remaps vector indices
  void assign_remap(const lvector& vec, const lvector<index_type>& map);
  void remap(const lvector<index_type>& map);

  const lvector& operator=(const lvector& _vec);
  // const T& operator=(const T& _val);

  void set_length(index_type l);
  void set_length(index_type l, const T& v);
  void inc_length_to(index_type l);
  void inc_length_to(index_type l, const T& v);
  index_type inc_length() { return inc_length(1); };
  index_type inc_length(index_type d);
  index_type inc_length(index_type d, const T& v);
  index_type dec_length() { return dec_length(1); };
  index_type dec_length(index_type d);
  void clear();

  void append(const T& v);
  void append(const lvector& v);
  T&   append();
  void insert(const T& v, index_type p);
  index_type insert_ordered(const T& v, const order& o, index_type f = 0);
  index_type insert_ordered(const lvector& vec, const order& o);

  void remove(index_type p);
  void remove(index_type p0, index_type p1);
  void remove(const index_set& s);
  void remove(const index_set& s, lvector<index_type>& map);
  void remove(const bool_vec& s);
  void remove(const bool_vec& s, lvector<index_type>& map);

  void swap(index_type i, index_type j);
};

template<class T> class auto_expanding_vector : public lvector<T>
{
  T _default;
 public:
  auto_expanding_vector() : lvector<T>() { };
  auto_expanding_vector(const T& v, index_type l)
    : lvector<T>(v, l), _default(v) { };
  auto_expanding_vector(const lvector<T>& vec)
    : lvector<T>(vec) { };
  auto_expanding_vector(const auto_expanding_vector<T>& vec)
    : lvector<T>(vec), _default(vec._default) { };

  typename std::vector<T>::reference
  operator[](typename std::vector<T>::size_type k)
  {
    this->inc_length_to(k + 1, _default);
    return lvector<T>::operator[](k);
  };

  typename std::vector<T>::const_reference
  operator[](typename std::vector<T>::size_type k) const
  {
    if (k >= std::vector<T>::size())
      return _default;
    else
      return lvector<T>::operator[](k);
  };

  void assign_value(const T& val)
  {
    _default = val;
    lvector<T>::assign_value(val);
  };

  void assign_value(const T& val, index_type l)
  {
    _default = val;
    lvector<T>::assign_value(val, l);
  };
};

typedef lvector<index_type> index_vec;
typedef lvector<index_pair> pair_vec;

class index_vec_util : public index_vec
{
 public:

  class decreasing_index_order : public index_vec::order {
  public:
    virtual bool operator()
      (const index_type& v0, const index_type& v1) const
      { return (v0 > v1); };
  };

  class increasing_index_order : public index_vec::order {
  public:
    virtual bool operator()
      (const index_type& v0, const index_type& v1) const
      { return (v0 < v1); };
  };

  class increasing_value_order : public index_vec::order {
    const index_vec& value;
  public:
    increasing_value_order(const index_vec& v) : value(v) { };
    virtual bool operator()
      (const index_type& v0, const index_type& v1) const
      {
	assert(v0 < value.length());
	assert(v1 < value.length());
	return (value[v0] < value[v1]);
      };
  };

  static class decreasing_index_order decreasing;
  static class increasing_index_order increasing;

  static void       fill(index_vec& vec, index_type max);

  static index_type min(const index_vec& vec, index_type def = no_such_index);
  static index_type max(const index_vec& vec, index_type def = no_such_index);
  static int        compare(const index_vec& v0, const index_vec& v1);
  static index_type hash(const index_vec& vec);

  void fill(index_type max);
  int  compare(const index_vec& v1) const;
  index_type hash() const;
};

void factors(index_type n, index_vec& f);

template<class T> class svector : public lvector<T>
{
 public:
  svector() : lvector<T>() { };
  svector(const svector<T>& _svec) : lvector<T>(_svec) { };
  svector(const lvector<T>& _lvec) : lvector<T>() {
    for (index_type k = 0; k < _lvec.size(); k++) insert(_lvec[k]);
  };
  // svector(const T* _arr, index_type n) : lvector<T>() {
  //   for (index_type k = 0; k < n; k++) insert(_arr[k]);
  // };

  bool contains(const T& v) const;
  bool contains(const svector& vec) const;
  bool subset(const svector& vec) const;

  index_pair first_common(const lvector<T>& vec) const;
  index_pair next_common(const lvector<T>& vec, index_pair p) const;
  index_pair first_common(const svector<T>& vec) const;
  index_pair next_common(const svector<T>& vec, index_pair p) const;
  index_type count_common(const svector& vec) const;

  void assign_singleton(const T& _val);
  void assign_values(const lvector<T>& vec);
  void insert(const T& v);
  void insert(const lvector<T>& vec);
  void intersect(const svector& vec);
  void difference(const svector& vec);
  void subtract(const svector& vec);
  void subtract(const T& v);
};

class index_set : public svector<index_type>
{
 public:
  // standard svector constructors
  index_set()
    : svector<index_type>() { };
  index_set(const index_set& _svec)
    : svector<index_type>(_svec) { };
  index_set(const lvector<index_type>& _lvec)
    : svector<index_type>(_lvec) { };
  // index_set(const index_type* _arr, index_type n)
  //   : svector<index_type>(_arr, n) { };

  // conversion from array of bool and bool_vec
  index_set(const bool* _arr, index_type n);
  index_set(const bool_vec& _vec);

  // subset constructor
  index_set(const index_set& s0, const index_set& s);
  index_set(const index_set& s0, const bool_vec& s);

  // remap constructor
  index_set(const index_set& s0, const index_vec& map);

  index_type first_common_element(const index_set& set) const;
  index_type first_common_element(const index_vec& vec) const;
  index_type first_common_element(const bool_vec vec) const;
  index_type first_common_element(const bool* vec, index_type n) const;

  index_type count_common(const index_set& set) const;
  index_type count_common(const bool_vec& set) const;

  bool have_common_element(const index_set& set) const;
  bool have_common_element(const bool_vec& set) const;

  void insert(const index_type& v);
  void insert(const index_vec& vec);
  void insert(const bool_vec& set);
  void intersect(const index_set& vec);
  void intersect(const bool_vec& set);
  void subtract(const index_vec& vec);
  void subtract(const bool_vec& set);
  void subtract(const index_type& v);

  bool* copy_to(bool* s, index_type n) const;

  void  fill(index_type to);
  // note: map here remaps elements
  void  assign_remap(const index_set& set, const index_vec& map);
  void  remap(const index_vec& map);
};

extern const index_set EMPTYSET;
typedef svector<index_pair> pair_set;

class bool_vec : public lvector<bool>
{
 public:
  // standard constructors
  bool_vec() : lvector<bool>() { };
  bool_vec(bool _val, index_type l) : lvector<bool>(_val, l) { };
  bool_vec(const bool_vec& _vec) : lvector<bool>(_vec) { };

  // construct from array + given size
  bool_vec(const bool* _arr, index_type n) : lvector<bool>(false, n) {
    for (index_type k = 0; k < n; k++) {
      if (_arr[k])
	(*this)[k] = true;
      else
	(*this)[k] = false;
    }
  };

  // conversion from index_set
  bool_vec(const index_set& set, index_type l);

  void complement();
  void insert(const bool_vec& vec);
  void insert(const index_set& set);
  void intersect(const bool_vec& vec);
  void intersect(const index_set& set);
  void subtract(const bool_vec& vec);
  void subtract(const index_set& set);
  bool subset(const bool_vec& vec) const;
  bool strict_subset(const bool_vec& vec) const;
  bool superset(const bool_vec& vec) const;
  bool strict_superset(const bool_vec& vec) const;
  bool contains(const bool& v) const;
  bool contains(const bool_vec& set) const;
  bool contains(const index_set& set) const;
  bool contains_any(const index_set& set) const;
  index_type first_common_element(const index_set& vec) const;
  index_type first_common_element(const bool_vec vec) const;
  index_type count_common(const bool_vec& vec) const;
  index_type count_common(const index_set& set) const;
  index_set& copy_to(index_set& set) const;
  index_set& insert_into(index_set& set) const;
  index_set& subtract_from(index_set& set) const;
  bool* copy_to(bool* s, index_type n) const;
  int compare(const bool_vec& vec) const;
  index_type hash() const;
};

class index_set_vec : public lvector<index_set>
{
 public:
  index_set_vec()
    : lvector<index_set>() { };
  index_set_vec(const index_set& set, index_type l)
    : lvector<index_set>(set, l) { };
  index_set_vec(index_type l)
    : lvector<index_set>(EMPTYSET, l) { };
  index_set_vec(const index_set_vec& vec)
    : lvector<index_set>(vec) { };

  class decreasing_cardinality_order : public index_set_vec::order {
  public:
    virtual bool operator()
      (const index_set& v0, const index_set& v1) const
      { return (v0.size() > v1.size()); };
  };

  decreasing_cardinality_order decreasing_cardinality;

  index_type minimum_cardinality() const;
  index_type maxmimum_cardinality() const;
  index_type selected_minimum_cardinality(const index_set& sel) const;
  index_type selected_maximum_cardinality(const index_set& sel) const;
  index_type first_minimum_cardinality_set() const;
  index_type first_maxmimum_cardinality_set() const;

  index_type first_superset(const index_set& set) const;
  index_type first_strict_superset(const index_set& set) const;
  index_type first_subset(const index_set& set) const;
  index_type first_strict_subset(const index_set& set) const;

  index_set& union_set(index_set& set) const;
  index_set& selected_union_set(const index_set& sel, index_set& set) const;
  index_set& intersection_set(index_set& set) const;

  void insert_maximal(const index_set& set);
  void insert_minimal(const index_set& set);
  void reduce_to_maximal();
  void reduce_to_minimal();

  void insert_if_not_subset(const index_set& set);
  void insert_if_not_superset(const index_set& set);
  void remove_sets_size_le(index_type l);
  void remove_empty_sets();
};

template<class T> class matrix : public lvector< lvector<T> >
{
 public:
  typedef lvector<T> row_type;

  matrix()
    : lvector<row_type>() { };
  matrix(const T& _val, index_type r, index_type c)
    : lvector<row_type>(row_type(_val, c), r) { };
  matrix(const matrix& _mat)
    : lvector<row_type>(_mat) { };

  index_type rows() const
  {
    return lvector<row_type>::length();
  };

  index_type columns() const
  {
    if (lvector<row_type>::length() == 0) return 0;
    else return (*this)[0].length();
  };
 
  void set_size(index_type r, index_type c);
  void assign_value(const T& _val);
  void assign_value(const T& _val, index_type r, index_type c);
};

class bool_matrix : public matrix<bool> {
 public:
  bool_matrix()
    : matrix<bool>() { };
  bool_matrix(const bool& v, index_type r, index_type c)
    : matrix<bool>(v, c, r) { };
  bool_matrix(const bool_matrix& m)
    : matrix<bool>(m) { };

  void complement();
  // "insert" means union (can't use name "union" since it's a keyword,
  // and insert is consistent with bool_vec/index_set terminology)
  void insert(const bool_matrix& m);
  void intersect(const bool_matrix& m);
  void subtract(const bool_matrix& m);
  // this = m0 x m1
  void multiply(const bool_matrix& m0, const bool_matrix& m1);
  void transitive_closure();
};

typedef matrix<index_type> index_matrix;

class mapping : public index_vec
{
 public:
  // construct an identity mapping of size n
  static void identity_map
    (index_type n, index_vec& map)
    { index_vec_util::fill(map, n); };

  // construct inverse mapping, if possible
  static bool invert_map
    (const index_vec& map, index_vec& inv, index_type m = 0);

  // construct a map that deletes i:th of n elements
  // (maps 0..i-1 to identity, i to nil, and i+1..n to identity - 1)
  static void delete_index_map
    (index_type n, index_type i, index_vec& map);

  // compose two mappings: cm[x] = m1[m0[x]]; clearly, this assumes
  // the domain of m1 contains the range of m0; using the same object
  // for the first input (m0) and the result (cm) is safe
  static void compose
    (const index_vec& m0, const index_vec& m1, index_vec& cm);

  // compute image under mapping of a vector of indices
  static void map_image
    (const index_vec& map, const index_vec& vec, index_vec& img);

  // compute inverse image under mapping of a single index (the
  // inverse image may be a set, since mappings can be many to one)
  static void inverse_map_image
    (const index_vec& map, index_type x, index_set& img);

  // compute inverse image under mapping of a set of indices
  static void inverse_map_image
    (const index_vec& map, const index_set& x, index_set& img);

  // compute the range, i.e. the highest index in the map image + 1, or
  // equivalently, the number of distinct indices that can appear in the
  // map image, corresponding to domain 0..d - 1; map must be defined up
  // to d - 1 (i.e. map.length() >= d).
  static index_type range(const index_vec& map, index_type d);

  mapping()
    : index_vec() { };
  mapping(index_type n) // identity map constructor
    : index_vec() { identity_map(n, *this); };
  // constructor for two kinds of map: if out == true, construct a
  // deleted index ("all but i") map; else construct an "all to i" map
  mapping(index_type n, index_type i, bool out) : index_vec() {
    if (out) delete_index_map(n, i, *this); else assign_value(i, n);
  };
  mapping(const mapping& map)
    : index_vec(map) { };

  // assign identity mapping of size n to this
  void assign_identity(index_type n)
    { identity_map(n, *this); };

  // apply mapping to a single index
  index_type operator()(index_type x) const
    { assert(x < size()); return (*this)[x]; };

  // apply mapping to a vector of indices
  index_vec operator()(const index_vec& vec) const
    { index_vec res; map_image(*this, vec, res); return res; };

  // apply inverse mapping to a single index
  index_vec& inverse(index_type x, index_set& res) const
    { inverse_map_image(*this, x, res); return res; };

  // apply inverse mapping to a set of indices
  index_vec& inverse(const index_set& x, index_set& res) const
    { inverse_map_image(*this, x, res); return res; };

  // assign inverse of this mapping to rmap
  bool invert(index_vec& rmap) const
    { return invert_map(*this, rmap); };

  // assign inverse of this mapping to this
  bool invert()
    { index_vec tmp(*this); return invert_map(tmp, *this); };

  index_type range() const
    { return range(*this, length()); };
};

class sparse_mapping : public pair_vec
{
 public:
  static void dense_to_sparse(const index_vec& dm, pair_vec sm);
  static void sparse_to_dense(const pair_vec& sm, index_vec dm);

  static index_type map_image
    (const pair_vec& map, index_type x);
  static void map_image
    (const pair_vec& map, const index_vec& vec, index_vec& img);
  static void inverse_map_image
    (const pair_vec& map, index_type x, index_set& img);
  static void inverse_map_image
    (const pair_vec& map, const index_set& x, index_set& img);

  sparse_mapping()
    : pair_vec() { };
  sparse_mapping(const pair_vec& m)
    : pair_vec(m) { };
  sparse_mapping(const index_vec& m)
    : pair_vec() { dense_to_sparse(m, *this); };

  index_type operator()(index_type x) const
    { return map_image(*this, x); };
  index_vec operator()(const index_vec& vec) const
    { index_vec res; map_image(*this, vec, res); return res; };
  index_set& inverse(index_type x, index_set& res) const
    { inverse_map_image(*this, x, res); return res; };
  index_set& inverse(const index_set& x, index_set& res) const
    { inverse_map_image(*this, x, res); return res; };
};


class equivalence : public index_vec
{
 public:
  equivalence()
    : index_vec() { };
  equivalence(index_type n)
    : index_vec(no_such_index, n) { index_vec_util::fill(*this, n); };
  equivalence(const equivalence& eq)
    : index_vec(eq) { };

  bool operator()(index_type a, index_type b) const;
  index_type canonical(index_type a) const;

  void extend(index_type a);
  void merge(index_type a, index_type b);
  void merge(const equivalence& eq);
  void merge(const index_set& set);
  void reset();
  void reset(index_type n);

  void canonical_set(index_set& set) const;
  void canonical_elements(index_set& set) const;
  void class_elements(index_type rep, index_set& set) const;
  index_type n_class_elements(index_type rep) const;
  void classes(index_set_vec& sets) const;
  void make_map(index_vec& map) const;
  index_type n_classes() const;
  index_type n_squeezed() const;
};


class set_hash_function : index_vec
{
 public:
  set_hash_function(index_type n)
    : index_vec() { init(n); };

  void init(index_type n);

  index_type operator()(index_type& i, index_type v) const;

  index_type operator()(const index_set& set) const;
  index_type operator()(const bool_vec& set) const;
  index_type operator()(const bool* set, index_type n) const;
};


template<class T, class N> struct weighted
{
  T value;
  N weight;

  weighted() : weight(0) { };
  weighted(const T& v) : value(v), weight(0) { };
  weighted(const T& v, const N& w) : value(v), weight(w) { };
  weighted(const weighted& w) : value(w.value), weight(w.weight) { };
  ~weighted() { };

  weighted& operator=(const T& v)
  {
    value = v;
    weight = 0;
    return *this;
  };

  weighted& operator=(const weighted& w)
  {
    value = w.value;
    weight = w.weight;
    return *this;
  };

  bool operator==(const weighted& w) const
  {
    return (value == w.value);
  };

  bool operator!=(const weighted& w) const
  {
    return (value != w.value);
  };

  bool operator<(const weighted& w) const
  {
    return (value < w.value);
  };

  bool operator<=(const weighted& w) const
  {
    return (value <= w.value);
  };

  bool operator>(const weighted& w) const
  {
    return (value > w.value);
  };

  bool operator>=(const weighted& w) const
  {
    return (value >= w.value);
  };
};

template<class T, class N> class weighted_vec
: public lvector< weighted<T, N> >
{
 public:

  class decreasing_weight_order : public lvector< weighted<T,N> >::order {
  public:
    virtual bool operator()
      (const weighted<T,N>& v0, const weighted<T,N>& v1) const
      { return (v0.weight > v1.weight); };
  };

  class increasing_weight_order : public lvector< weighted<T,N> >::order {
  public:
    virtual bool operator()
      (const weighted<T,N>& v0, const weighted<T,N>& v1) const
      { return (v0.weight < v1.weight); };
  };

  static class decreasing_weight_order decreasing;
  static class increasing_weight_order increasing;

  void insert_increasing(const weighted<T,N>& v);
  void insert_decreasing(const weighted<T,N>& v);
  void insert_increasing(const T& v, const N& w);
  void insert_decreasing(const T& v, const N& w);
};

template<class T, class N> class weighted_set
: public svector< weighted<T,N> >
{
 public:
  void insert(const T& v, const N& w);
  void insert(const T& v);

  index_type arg_max();
  index_type arg_min();
};


// inlines

template<class T>
bool lvector<T>::operator==(const lvector& _vec) const
{
  if (lvector<T>::size() != _vec.size()) return false;
  for (index_type k = 0; k < lvector<T>::size(); k++)
    if (!((*this)[k] == _vec[k])) return false;
  return true;
}

template<class T>
bool lvector<T>::operator!=(const lvector& _vec) const
{
  if (*this == _vec) return false;
  else return true;
}

template<class T>
bool lvector<T>::operator<(const lvector& vec) const
{
  if (lvector<T>::size() < vec.size()) return true;
  else if (lvector<T>::size() > vec.size()) return false;
  else {
    for (index_type k = 0; k < lvector<T>::size(); k++) {
      if ((*this)[k] < vec[k]) return true;
      else if ((*this)[k] > vec[k]) return false;
    }
    return false;
  }
}

template<class T>
bool lvector<T>::operator<=(const lvector& vec) const
{
  if (lvector<T>::size() < vec.size()) return true;
  else if (lvector<T>::size() > vec.size()) return false;
  else {
    for (index_type k = 0; k < lvector<T>::size(); k++) {
      if ((*this)[k] < vec[k]) return true;
      else if ((*this)[k] > vec[k]) return false;
    }
    return true;
  }
}

template<class T>
bool lvector<T>::operator>(const lvector& vec) const
{
  if (lvector<T>::size() < vec.size()) return false;
  else if (lvector<T>::size() > vec.size()) return true;
  else {
    for (index_type k = 0; k < lvector<T>::size(); k++) {
      if ((*this)[k] < vec[k]) return false;
      else if ((*this)[k] > vec[k]) return true;
    }
    return false;
  }
}

template<class T>
bool lvector<T>::operator>=(const lvector& vec) const
{
  if (lvector<T>::size() < vec.size()) return false;
  else if (lvector<T>::size() > vec.size()) return true;
  else {
    for (index_type k = 0; k < lvector<T>::size(); k++) {
      if ((*this)[k] < vec[k]) return false;
      else if ((*this)[k] > vec[k]) return true;
    }
    return true;
  }
}

template<class T>
bool lvector<T>::contains(const T& v) const
{
  for (index_type k = 0; k < lvector<T>::size(); k++)
    if ((*this)[k] == v) return true;
  return false;
}

template<class T>
index_type lvector<T>::first(const T& v) const
{
  for (index_type k = 0; k < lvector<T>::size(); k++)
    if ((*this)[k] == v) return k;
  return no_such_index;
}

template<class T>
index_type lvector<T>::next(const T& v, index_type p) const
{
  for (index_type k = p + 1; k < lvector<T>::size(); k++)
    if ((*this)[k] == v) return k;
  return no_such_index;
}

template<class T>
index_type lvector<T>::find(const T& v, bool_vec& s) const
{
  index_type n = 0;
  s.assign_value(false, lvector<T>::size());
  for (index_type k = 0; k < lvector<T>::size(); k++)
    if ((*this)[k] == v) {
      s[k] = true;
      n += 1;
    }
  return n;
}

template<class T>
index_type lvector<T>::count(const T& v) const
{
  index_type c = 0;
  for (index_type k = 0; k < lvector<T>::size(); k++)
    if ((*this)[k] == v) c += 1;
  return c;
}

template<class T>
index_type lvector<T>::length() const
{
  return std::vector<T>::size();
}

template<class T>
index_type lvector<T>::arg_max() const
{
  if (lvector<T>::empty()) return no_such_index;
  index_type m = 0;
  for (index_type k = 1; k < lvector<T>::size(); k++)
    if ((*this)[k] > (*this)[m]) m = k;
  return m;
}

template<class T>
index_type lvector<T>::arg_min() const
{
  if (lvector<T>::empty()) return no_such_index;
  index_type m = 0;
  for (index_type k = 1; k < lvector<T>::size(); k++)
    if ((*this)[k] < (*this)[m]) m = k;
  return m;
}

template<class T>
index_type lvector<T>::arg_first(const order& o) const
{
  if (lvector<T>::empty()) return no_such_index;
  index_type m = 0;
  for (index_type k = 1; k < lvector<T>::size(); k++)
    if (o((*this)[k], (*this)[m])) m = k;
  return m;
}

template<class T>
index_type lvector<T>::arg_last(const order& o) const
{
  if (lvector<T>::empty()) return no_such_index;
  index_type m = 0;
  for (index_type k = 1; k < lvector<T>::size(); k++)
    if (o((*this)[m], (*this)[k])) m = k;
  return m;
}

template<class T>
index_pair lvector<T>::first_common(const lvector<T>& vec) const
{
  for (index_type i = 0; i < lvector<T>::size(); i++) {
    for (index_type j = 0; j < vec.size(); j++)
      if ((*this)[i] == vec[j]) return index_pair(i, j);
  }
  return index_pair(no_such_index, no_such_index);
}

template<class T>
index_pair lvector<T>::next_common(const lvector<T>& vec, index_pair p) const
{
  index_type i = p.first;
  index_type j = p.second + 1;
  while (j < vec.size()) {
    if ((*this)[i] == vec[j])
      return index_pair(i, j);
    j += 1;
  }
  i += 1;
  while (i < lvector<T>::size()) {
    j = 0;
    while (j < vec.size()) {
      if ((*this)[i] == vec[j])
	return index_pair(i, j);
      j += 1;
    }
    i += 1;
  }
  return index_pair(no_such_index, no_such_index);
}

template<class T>
void lvector<T>::difference
(const lvector& v1, lvector& d0, lvector& d1)
{
  d0.assign_copy(*this);
  d1.assign_copy(v1);
  index_type i0 = 0;
  while (i0 < d0.size()) {
    index_type i1 = d1.first(d0[i0]);
    if (i1 != no_such_index) {
      d0.remove(i0);
      d1.remove(i1);
    }
    else {
      i0 += 1;
    }
  }
}

template<class T>
void lvector<T>::assign_copy(const lvector& _vec)
{
  std::vector<T>::resize(_vec.size());
  for (index_type k = 0; k < _vec.size(); k++)
    (*this)[k] = _vec[k];
}

template<class T>
void lvector<T>::assign_copy(const T* _arr, index_type n)
{
  std::vector<T>::resize(n);
  for (index_type k = 0; k < n; k++)
    (*this)[k] = _arr[k];
}

template<class T>
void lvector<T>::assign_value(const T& _val)
{
  for (index_type k = 0; k < lvector<T>::size(); k++)
    (*this)[k] = _val;
}

template<class T>
void lvector<T>::assign_value(const T& _val, index_type l)
{
  std::vector<T>::resize(l);
  for (index_type k = 0; k < lvector<T>::size(); k++)
    (*this)[k] = _val;
}

template<class T>
void lvector<T>::assign_remap(const lvector<T>& vec, const index_vec& map)
{
  assert(map.length() == vec.length());
  index_type m = 0;
  for (index_type k = 0; k < vec.length(); k++)
    if (map[k] != no_such_index)
      if (map[k] > m) m = map[k];
  set_length(m + 1);
  for (index_type k = 0; k < vec.length(); k++)
    if (map[k] != no_such_index)
      (*this)[map[k]] = vec[k];
}

template<class T>
void lvector<T>::remap(const index_vec& map)
{
  lvector v0(*this);
  assign_remap(v0, map);
}

template<class T>
const lvector<T>& lvector<T>::operator=(const lvector<T>& _vec)
{
  assign_copy(_vec);
  return _vec;
}

// template<class T>
// const T& lvector<T>::operator=(const T& _val)
// {
//   assign_value(_val);
//   return _val;
// }

template<class T>
void lvector<T>::set_length(index_type l)
{
  std::vector<T>::resize(l);
}

template<class T>
void lvector<T>::set_length(index_type l, const T& v)
{
  std::vector<T>::resize(l, v);
}

template<class T>
void lvector<T>::inc_length_to(index_type l)
{
  if (std::vector<T>::size() < l)
    std::vector<T>::resize(l);
}

template<class T>
void lvector<T>::inc_length_to(index_type l, const T& v)
{
  if (std::vector<T>::size() < l)
    std::vector<T>::resize(l, v);
}

template<class T>
index_type lvector<T>::inc_length(index_type d)
{
  std::vector<T>::resize(std::vector<T>::size() + d);
  return std::vector<T>::size();
}

template<class T>
index_type lvector<T>::inc_length(index_type d, const T& v)
{
  std::vector<T>::resize(std::vector<T>::size() + d, v);
  return std::vector<T>::size();
}

template<class T>
index_type lvector<T>::dec_length(index_type d)
{
  assert(std::vector<T>::size() >= d);
  std::vector<T>::resize(std::vector<T>::size() - d);
  return std::vector<T>::size();
}

template<class T>
void lvector<T>::clear()
{
  std::vector<T>::clear();
}

template<class T>
void lvector<T>::append(const T& v)
{
  std::vector<T>::push_back(v);
}

template<class T>
void lvector<T>::append(const lvector<T>& v)
{
  for (index_type k = 0; k < v.size(); k++) append(v[k]);
}

template<class T>
T& lvector<T>::append()
{
  T v;
  std::vector<T>::push_back(v);
  // std::vector<T>::resize(std::vector<T>::size() + 1);
  return (*this)[std::vector<T>::size() - 1];
}

template<class T>
void lvector<T>::insert(const T& v, index_type p)
{
  if (p < lvector<T>::size()) {
    std::vector<T>::insert(std::vector<T>::begin() + p, v);
  }
  else {
    std::vector<T>::resize(p + 1);
    (*this)[p] = v;
  }
}

template<class T>
index_type lvector<T>::insert_ordered(const T& v, const order& o, index_type f)
{
  assert(f <= lvector<T>::size());
  for (index_type k = f; k < lvector<T>::size(); k++) {
    if (o(v, (*this)[k])) {
      insert(v, k);
      return k;
    }
  }
  append(v);
  return (lvector<T>::size() - 1);
}

template<class T>
index_type lvector<T>::insert_ordered(const lvector& vec, const order& o)
{
  if (vec.empty()) return no_such_index;
  index_type p0 = insert_ordered(vec[0], o);
  for (index_type k = 1; k < vec.size(); k++) {
    index_type p1 = insert_ordered(vec[k], o);
    if (p1 < p0) p0 = p1;
  }
  return p0;
}

template<class T>
void lvector<T>::remove(index_type p)
{
  if (p < lvector<T>::size())
    std::vector<T>::erase(std::vector<T>::begin() + p);
}

template<class T>
void lvector<T>::remove(index_type p0, index_type p1)
{
  assert(p0 < p1);
  if (p1 < lvector<T>::size())
    std::vector<T>::erase(std::vector<T>::begin() + p0,
			  std::vector<T>::begin() + p1);
  else
    std::vector<T>::erase(std::vector<T>::begin() + p0,
			  std::vector<T>::end());
}

template<class T>
void lvector<T>::remove(const bool_vec& s, index_vec& map)
{
  assert(s.size() >= lvector<T>::size());
  index_type scan_p = 0;
  index_type put_p = 0;
  index_vec rm_map(no_such_index, lvector<T>::size());
  while (scan_p < lvector<T>::size()) {
    if (!s[scan_p]) {
      if (put_p < scan_p) {
	(*this)[put_p] = (*this)[scan_p];
      }
      rm_map[scan_p] = put_p;
      put_p += 1;
    }
    else {
      rm_map[scan_p] = no_such_index;
    }
    scan_p += 1;
  }
  std::vector<T>::resize(put_p);

  for (index_type k = 0; k < map.size(); k++)
    if (map[k] != no_such_index) {
      assert(map[k] < rm_map.size());
      map[k] = rm_map[map[k]];
    }
}

template<class T>
void lvector<T>::remove(const bool_vec& s)
{
  assert(s.size() >= lvector<T>::size());
  index_type scan_p = 0;
  index_type put_p = 0;
  while (scan_p < lvector<T>::size()) {
    if (!s[scan_p]) {
      if (put_p < scan_p) {
	(*this)[put_p] = (*this)[scan_p];
      }
      put_p += 1;
    }
    scan_p += 1;
  }
  std::vector<T>::resize(put_p);
}

template<class T>
void lvector<T>::remove(const index_set& s)
{
  bool_vec s1(s, lvector<T>::size());
  remove(s1);
}

template<class T>
void remove(const index_set& s, index_vec& map)
{
  bool_vec s1(s, std::vector<T>::size());
  lvector<T>::remove(s1, map);
}

template<class T>
void lvector<T>::swap(index_type i, index_type j)
{
  T tmp = (*this)[i];
  (*this)[i] = (*this)[j];
  (*this)[j] = tmp;
}

template<class T>
void svector<T>::assign_singleton(const T& _val)
{
  lvector<T>::set_length(1);
  (*this)[0] = _val;
}

template<class T>
void svector<T>::assign_values(const lvector<T>& vec)
{
  lvector<T>::clear();
  for (index_type k = 0; k < vec.size(); k++)
    insert(vec[k]);
}

template<class T>
void svector<T>::insert(const T& v) {
  index_type i = 0;
  bool seeking = (i < std::vector<T>::size());
  while (seeking) {
    if ((*this)[i] < v) {
      i += 1;
      if (i >= std::vector<T>::size())
	seeking = false;
    }
    else {
      seeking = false;
    }
  }
  if (i < lvector<T>::size()) {
    if ((*this)[i] == v)
      return;
    else
      lvector<T>::insert(v, i);
  }
  else {
    lvector<T>::append(v);
  }
}

template<class T>
void svector<T>::insert(const lvector<T>& vec)
{
  for (index_type k = 0; k < vec.size(); k++) insert(vec[k]);
}

template<class T>
bool svector<T>::contains(const T& v) const
{
  index_type i = 0;
  while ((i < lvector<T>::size()) &&
	 ((*this)[i] < v)) i += 1;
  if (i < lvector<T>::size())
    if ((*this)[i] == v) return true;
  return false;
}

template<class T>
bool svector<T>::contains(const svector& vec) const
{
  index_type v_i = 0;
  index_type i = 0;
  while (v_i < vec.size()) {
    if (i >= lvector<T>::size()) return false;
    if ((*this)[i] == vec[v_i]) {
      v_i += 1;
      i += 1;
    }
    else if ((*this)[i] > vec[v_i]) {
      return false;
    }
    else {
      while ((i < lvector<T>::size()) && ((*this)[i] < vec[v_i]))
	i += 1;
    }
  }
  return true;
}

template<class T>
bool svector<T>::subset(const svector& vec) const
{
  return vec.contains(*this);
}

template<class T>
void svector<T>::intersect(const svector& vec)
{
  index_type i = 0;
  while (i < lvector<T>::size()) {
    if (!vec.contains((*this)[i]))
      lvector<T>::remove(i);
    else
      i += 1;
  }
}

template<class T>
void svector<T>::difference(const svector& vec)
{
  svector d(vec);
  d.subtract(*this);
  subtract(vec);
  insert(d);
}

template<class T>
index_pair svector<T>::first_common(const lvector<T>& vec) const
{
  return lvector<T>::first_common(vec);
}

template<class T>
index_pair svector<T>::next_common(const lvector<T>& vec, index_pair p) const
{
  return lvector<T>::next_common(vec, p);
}

template<class T>
index_pair svector<T>::first_common(const svector<T>& vec) const
{
  index_type i = 0;
  index_type j = 0;
  while ((i < svector<T>::size()) && (j < vec.size())) {
    if ((*this)[i] == vec[j])
      return index_pair(i, j);
    else if
      ((*this)[i] < vec[j]) i += 1;
    else
      j += 1;
  }
  return index_pair(no_such_index, no_such_index);
}

template<class T>
index_pair svector<T>::next_common(const svector<T>& vec, index_pair p) const
{
  index_type i = p.first + 1;
  index_type j = p.second + 1;
  while ((i < svector<T>::size()) && (j < vec.size())) {
    if ((*this)[i] == vec[j])
      return index_pair(i, j);
    else if ((*this)[i] < vec[j])
      i += 1;
    else
      j += 1;
  }
  return index_pair(no_such_index, no_such_index);
}

template<class T>
index_type svector<T>::count_common(const svector& vec) const
{
  index_type i = 0;
  index_type j = 0;
  index_type c = 0;
  while ((i < svector<T>::size()) && (j < vec.size())) {
    if ((*this)[i] == vec[j]) {
      c += 1;
      i += 1;
      j += 1;
    }
    else if ((*this)[i] < vec[j])
      i += 1;
    else
      j += 1;
  }
  return c;
}

template<class T>
void svector<T>::subtract(const svector& vec)
{
  index_type i = 0;
  while (i < lvector<T>::size()) {
    if (vec.contains((*this)[i]))
      lvector<T>::remove(i);
    else
      i += 1;
  }
}

template<class T>
void svector<T>::subtract(const T& v)
{
  index_type i = 0;
  while (i < lvector<T>::size()) {
    if ((*this)[i] == v) {
      lvector<T>::remove(i);
      return;
    }
    else {
      i += 1;
    }
  }
}

template<class T>
void matrix<T>::set_size(index_type r, index_type c)
{
  lvector<row_type>::set_length(r);
  for (index_type k = 0; k < lvector<row_type>::size(); k++)
    (*this)[k].set_length(c);
}

template<class T>
void matrix<T>::assign_value(const T& _val)
{
  for (index_type k = 0; k < lvector<row_type>::size(); k++)
    (*this)[k].assign_value(_val);
}

template<class T>
void matrix<T>::assign_value(const T& _val, index_type r, index_type c)
{
  lvector<row_type>::set_length(r);
  for (index_type k = 0; k < lvector<row_type>::size(); k++)
    (*this)[k].assign_value(_val, c);
}

template<class T>
void swapable_pair<T>::swap()
{
  T tmp = this->first;
  this->first = this->second;
  this->second = tmp;
}

template<class T>
void comparable_pair<T>::sort_ascending()
{
  if (this->first > this->second) swapable_pair<T>::swap();
}

template<class T>
void comparable_pair<T>::sort_descending()
{
  if (this->first < this->second) swapable_pair<T>::swap();
}

template<class T, class N>
class weighted_vec<T,N>::decreasing_weight_order
  weighted_vec<T,N>::decreasing;

template<class T, class N>
class weighted_vec<T,N>::increasing_weight_order
  weighted_vec<T,N>::increasing;

template<class T, class N>
void weighted_vec<T,N>::insert_increasing(const weighted<T,N>& v)
{
  this->insert_ordered(v, increasing);
}

template<class T, class N>
void weighted_vec<T,N>::insert_decreasing(const weighted<T,N>& v)
{
  this->insert_ordered(v, decreasing);
}

template<class T, class N>
void weighted_vec<T,N>::insert_increasing(const T& v, const N& w)
{
  this->insert_ordered(weighted<T,N>(v, w), increasing);
}

template<class T, class N>
void weighted_vec<T,N>::insert_decreasing(const T& v, const N& w)
{
  this->insert_ordered(weighted<T,N>(v, w), decreasing);
}

template<class T, class N>
void weighted_set<T,N>::insert(const T& v, const N& w)
{
  index_type p = svector< weighted<T,N> >::first(v);
  if (p == no_such_index) {
    svector< weighted<T,N> >::insert(weighted<T,N>(v, w));
  }
  else {
    (*this)[p].weight += w;
  }
}

template<class T, class N>
void weighted_set<T,N>::insert(const T& v)
{
  index_type p = svector< weighted<T,N> >::first(v);
  if (p == no_such_index) {
    svector< weighted<T,N> >::insert(weighted<T,N>(v, 1));
  }
  else {
    (*this)[p].weight += 1;
  }
}

template<class T, class N>
index_type weighted_set<T,N>::arg_max()
{
  if (weighted_set<T,N>::empty()) return no_such_index;
  index_type m = 0;
  for (index_type k = 1; k < weighted_set<T,N>::size(); k++)
    if ((*this)[k].weight > (*this)[m].weight) m = k;
  return m;
}

template<class T, class N>
index_type weighted_set<T,N>::arg_min()
{
  if (weighted_set<T,N>::empty()) return no_such_index;
  index_type m = 0;
  for (index_type k = 1; k < weighted_set<T,N>::size(); k++)
    if ((*this)[k].weight < (*this)[m].weight) m = k;
  return m;
}

template<class T>
inline std::ostream& operator<<(std::ostream& s, const swapable_pair<T>& p)
{
  return s << '(' << p.first << ',' << p.second << ')';
}

template<class T>
::std::ostream& operator<<(::std::ostream& s, const lvector<T>& _vec)
{
  s << '[';
  for (index_type k = 0; k < _vec.size(); k++) {
    if (k > 0) s << ',';
    s << _vec[k];
  }
  s << ']';
  return s;
}

inline std::ostream& operator<<(std::ostream& s, const mapping& m)
{
  s << '{';
  for (index_type k = 0; k < m.length(); k++) {
    if (k > 0) s << ',';
    s << k << '-' << '>';
    if (m[k] == no_such_index)
      s << '_';
    else
      s << m[k];
  }
  return s << '}';
}

inline std::ostream& operator<<(std::ostream& s, const equivalence& eq)
{
  s << '{';
  bool first = true;
  for (index_type k = 0; k < eq.length(); k++) {
    index_type c = eq.canonical(k);
    if (!first) {
      s << ',';
    }
    else {
      first = false;
    }
    s << k << '=' << c;
  }
  return s << '}';
}

template<class T, class N>
std::ostream& operator<<(::std::ostream& s, const weighted<T,N>& w)
{
  s << '<' << w.value << ':' << w.weight << '>';
}

END_HSPS_NAMESPACE

#endif
