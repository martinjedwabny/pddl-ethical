
#include "cost_table.h"
#include "preprocess.h"
#include "plans.h"
#include "idao.h"
#include "forward.h"
#include "bfs.h"
#include "collector.h"
#include "graph.h"
#include "enumerators.h"

BEGIN_HSPS_NAMESPACE

bool CostNode::eval_set_mark = false;
#ifdef EVAL_EXTRA_STATS
count_type CostNode::eval_rec_count = 0;
#endif

bool CostTable::strong_conflict_detection = true;
bool CostTable::ultra_weak_conflict_detection = false;
bool CostTable::boost_new_entries = true;
count_type CostTable::n_entries_boosted = 0;
count_type CostTable::n_entries_solved = 0;
count_type CostTable::n_entries_discarded = 0;
count_type CostTable::n_entries_created = 0;
count_type CostTable::n_boost_searches = 0;
count_type CostTable::n_boost_searches_with_cd = 0;

CostNode::CostNode(index_type size)
  : _size(size),
    _depth(0),
    _first(0),
    _store(0),
    _default(0),
    _max(POS_INF),
    _prev(0)
{
  _store = new Value[_size];
  for (index_type k = 0; k < _size; k++) {
    _store[k].val = _default;
    _store[k].next = 0;
  }
}

CostNode::CostNode(index_type size, NTYPE v_default)
  : _size(size),
    _depth(0),
    _first(0),
    _store(0),
    _default(v_default),
    _max(POS_INF),
    _prev(0)
{
  _store = new Value[_size];
  for (index_type k = 0; k < _size; k++) {
    _store[k].val = _default;
    _store[k].next = 0;
  }
}

CostNode::CostNode(CostNode* p, index_type f)
  : _size(p->_size),
    _depth(p->_depth + 1),
    _first(f),
    _store(0),
    _default(p->_default),
    _max(POS_INF),
    _prev(p)
{
  _store = new Value[_size];
  for (index_type k = 0; k < _size; k++) {
    _store[k].val = _default;
    _store[k].next = 0;
  }
}

CostNode::~CostNode()
{
  for (index_type k = 0; k < _size; k++)
    if (_store[k].next) delete _store[k].next;
  delete [] _store;
}

void CostNode::clear()
{
  for (index_type k = 0; k < _size; k++) {
    if (_store[k].next) delete _store[k].next;
    _store[k].clear(_default);
  }
  _max = POS_INF;
}

CostNode::Value* CostNode::find(const index_set& s)
{
  Value* v = 0;
  CostNode* n = this;
  for (index_type k = 0; k < s.length(); k++) {
    if (!n) return 0;
    v = n->val_p(s[k]);
    n = v->next;
  }
  return v;
}

CostNode::Value* CostNode::find(const bool_vec& s)
{
  Value* v = 0;
  CostNode* n = this;
  for (index_type k = 0; k < _size; k++) if (s[k]) {
    if (!n) return 0;
    v = n->val_p(k);
    n = v->next;
  }
  return v;
}

CostNode::Value& CostNode::insert(const index_set& s)
{
  assert(s.length() > 0);
  CostNode* n = this;
  for (index_type k = 0; k < s.length() - 1; k++) {
    n = n->next_p(s[k]);
  }
  return n->val(s[s.length() - 1]);
}

CostNode::Value& CostNode::insert(const bool_vec& s)
{
  index_type last = no_such_index;
  for (index_type k = _size; (k > 0) && (last == no_such_index); k--)
    if (s[k - 1]) last = k - 1;
  assert(last != no_such_index);
  CostNode* n = this;
  for (index_type k = 0; k < last; k++) if (s[k]) {
    n = n->next_p(k);
  }
  return n->val(last);
}

void CostNode::store(index_type p, NTYPE v, bool opt)
{
  val(p) = Value(v, opt);
}

void CostNode::store(index_type p, NTYPE v)
{
  val(p) = v;
}

void CostNode::set_max(NTYPE v)
{
  if (v > _max) {
    _max = v;
    if (_prev) _prev->set_max(v);
  }
}

void CostNode::store(const index_set& s, NTYPE v, bool opt)
{
  if (s.length() > 0) {
    CostNode* n = this;
    for (index_type i = 0; i < s.length() - 1; i++) n = n->next_p(s[i]);
    n->store(s[s.length() - 1], v, opt);
  }
}

void CostNode::store(const bool_vec& s, NTYPE v, bool opt)
{
  CostNode* p = 0;
  CostNode* n = this;
  index_type l = no_such_index;
  for (index_type k = 0; k < _size; k++) if (s[k]) {
    p = n;
    n = n->next_p(k);
    l = k;
  }
  if (p) p->store(l, v, opt);
}

void CostNode::store(const index_set& s, NTYPE v)
{
  if (s.length() > 0) {
    CostNode* n = this;
    for (index_type i = 0; i < s.length() - 1; i++) n = n->next_p(s[i]);
    n->store(s[s.length() - 1], v);
  }
}

void CostNode::store(const bool_vec& s, NTYPE v)
{
  CostNode* p = 0;
  CostNode* n = this;
  index_type l = no_such_index;
  for (index_type k = 0; k < _size; k++) if (s[k]) {
    p = n;
    n = n->next_p(k);
    l = k;
  }
  if (p) p->store(l, v);
}

NTYPE CostNode::eval(const index_set& s)
{
  return eval(s, 0);
}

NTYPE CostNode::eval(const bool_vec& s)
{
  return eval(s, 0);
}

NTYPE CostNode::eval_to_bound(const index_set& s, NTYPE bound)
{
  return eval_to_bound(s, 0, bound);
}

NTYPE CostNode::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  return eval_to_bound(s, 0, bound);
}

NTYPE CostNode::eval(const index_set& s, index_type i)
{
  Heuristic::eval_count += 1;
  NTYPE v_new = 0;
  for (index_type k = i; k < s.length(); k++) {
    if (INFINITE(val(s[k]).val)) return POS_INF;
    v_new = MAX(v_new, val(s[k]).val);
    if (val(s[k]).next) {
#ifdef EVAL_EXTRA_STATS
      eval_rec_count += 1;
#endif
      v_new = MAX(v_new, val(s[k]).next->eval(s, k + 1));
    }
    val(s[k]).mark = true;
  }
  return v_new;
}

NTYPE CostNode::eval(const bool_vec& s, index_type i)
{
  Heuristic::eval_count += 1;
  NTYPE v_new = 0;
  for (index_type k = i; k < _size; k++) if (s[k]) {
    if (INFINITE(val(k).val)) return POS_INF;
    v_new = MAX(v_new, val(k).val);
    if (val(k).next) {
#ifdef EVAL_EXTRA_STATS
      eval_rec_count += 1;
#endif
      v_new = MAX(v_new, val(k).next->eval(s, k + 1));
    }
    val(k).mark = true;
  }
  return v_new;
}

NTYPE CostNode::eval_to_bound(const index_set& s, index_type i, NTYPE bound)
{
  NTYPE v = 0;
  for (index_type k = i; k < s.length(); k++) {
    v = MAX(v, val(s[k]).val);
    val(s[k]).mark = true;
    if (v > bound) return v;
    if (val(s[k]).next)
      v = MAX(v, val(s[k]).next->eval_to_bound(s, k + 1, bound));
  }
  return v;
}

NTYPE CostNode::eval_to_bound(const bool_vec& s, index_type i, NTYPE bound)
{
  NTYPE v = 0;
  for (index_type k = i; k < _size; k++) if (s[k]) {
    v = MAX(v, val(k).val);
    val(k).mark = true;
    if (v > bound) return v;
    if (val(k).next)
      v = MAX(v, val(k).next->eval_to_bound(s, k + 1, bound));
  }
  return v;
}

NTYPE CostNode::incremental_eval(const bool_vec& s, index_type i, index_type i_new)
{
  Heuristic::eval_count += 1;
  NTYPE v = val(i_new).val;
  if (val(i_new).next) {
#ifdef EVAL_EXTRA_STATS
    eval_rec_count += 1;
#endif
    v = MAX(v, val(i_new).next->eval(s, i_new + 1));
  }
  for (index_type k = i; k < i_new; k++) if (s[k]) {
    if (val(k).next) {
#ifdef EVAL_EXTRA_STATS
      eval_rec_count += 1;
#endif
      v = MAX(v, val(k).next->incremental_eval(s, k + 1, i_new));
    }
  }
  return v;
}

NTYPE CostNode::incremental_eval(const bool_vec& s, index_type i_new)
{
  return incremental_eval(s, 0, i_new);
}

NTYPE CostNode::incremental_eval(const index_set& s, index_type i_new)
{
  index_set s_new(s);
  s_new.insert(i_new);
  return eval(s_new);
}

NTYPE CostNode::eval_min(const index_set& s)
{
#ifdef PRINT_DEBUG
  std::cerr << this << ": " << s << std::endl;
#endif

  NTYPE v_min = POS_INF;
  CostNode* n = this;
  index_type l = no_such_index;
  for (index_type k = 0; (k < s.length()) && (n != 0); k++) {
#ifdef PRINT_DEBUG
    std::cerr << "k = " << k
	      << ", s[k] = " << s[k]
	      << std::endl;
#endif
    if (s[k] >= _first) {
      if (l == no_such_index) l = s[k];
      Value& v = n->val(s[k]);
#ifdef PRINT_DEBUG
      std::cerr << "n = " << n
		<< ", val = " << v.val
		<< ", next = " << v.next
		<< std::endl;
#endif
      if (k == (s.length() - 1)) v_min = v.val;
      n = v.next;
    }
  }

  // v_min = value of {atoms on path to this node} U s
  // l = smallest index greater than _first that is in s
#ifdef PRINT_DEBUG
  std::cerr << "v_min = " << v_min << ", l = " << l << std::endl;
#endif
  if (l != no_such_index) {
    for (index_type k = _first; k <= l; k++) {
      CostNode* n1 = val(k).next;
      if (n1) {
	NTYPE v1 = n1->eval_min(s);
	v_min = MIN(v_min, v1);
      }
    }
  }
  else { // s is a subset of {atoms on path to this node}
    for (index_type k = _first; k < _size; k++) {
      v_min = MIN(v_min, val(k).val);
      CostNode* n1 = val(k).next;
      if (n1) {
	NTYPE v1 = n1->eval_min(s);
	v_min = MIN(v_min, v1);
      }
    }
  }

  return v_min;
}

void CostNode::compute_max()
{
  NTYPE v = 0;
  for (index_type k = _first; k < _size; k++) {
    v = MAX(v, _store[k].val);
    if (_store[k].next) {
      _store[k].next->compute_max();
      v = MAX(v, _store[k].next->_max);
    }
  }
  _max = v;
}

void CostNode::clear_marks()
{
  for (index_type k = _first; k < _size; k++) {
    _store[k].mark = false;
    if (_store[k].next) _store[k].next->clear_marks();
  }
}

void CostNode::write(std::ostream& s, index_set q) const
{
  index_type l = q.length();
  q.inc_length();
  for (index_type k = _first; k < _size; k++) {
    q[l] = k;
    if (true || (_store[k].val != _default) || (_store[k].opt)) {
#ifdef PRINT_DEBUG
      s << "(" << this << ") ";
#endif
      s << q << ": " << _store[k] << std::endl;
    }
    if (_store[k].next) _store[k].next->write(s, q);
  }
  q.dec_length();
}

void CostNode::write_pddl(std::ostream& s, Instance& ins, index_set q) const
{
  assert(ins.n_atoms() >= _size);
  index_type l = q.length();
  q.inc_length();
  for (index_type k = _first; k < _size; k++) {
    q[l] = k;
    if ((_store[k].val != _default) || (_store[k].opt)) {
      s << std::endl << '(';
      for (index_type i = 0; i < q.length(); i++) {
	if (i > 0) s << ' ';
	ins.atoms[q[i]].name->write(s, Name::NC_PDDL);
      }
      s << ')';
      if (_store[k].opt) s << " :opt";
      if (INFINITE(_store[k].val)) s << " :inf";
      else s << ' ' << _store[k].val;
    }
    if (_store[k].next) _store[k].next->write_pddl(s, ins, q);
  }
  q.dec_length();
}

void CostNode::write(std::ostream& s) const
{
  index_set q;
  write(s, q);
}

void CostNode::write_pddl(std::ostream& s, Instance& ins) const
{
  index_set q;
  s << "(:heuristic";
  write_pddl(s, ins, q);
  s << ")" << std::endl;
}

CostTable::CostTable(Instance& i, Statistics& s)
  : Heuristic(i),
    CostNode(i.n_atoms()),
    stats(s),
    pre_cost(0),
    per_cost(0)
{
  // done
}

CostTable::CostTable(Instance& i, Statistics& s, NTYPE v_default)
  : Heuristic(i),
    CostNode(i.n_atoms(), v_default),
    stats(s),
    pre_cost(0),
    per_cost(0)
{
  // done
}

CostTable::~CostTable()
{
  if (pre_cost) {
    delete pre_cost;
    pre_cost = 0;
  }
  if (per_cost) {
    delete per_cost;
    per_cost = 0;
  }
}

CostTable::Entry* CostTable::Entry::first()
{
  Entry* e = this;
  while (e->prev) e = e->prev;
  return e;
}

CostTable::Entry* CostTable::Entry::move_down()
{
  if (next) if (val.val > next->val.val) {
    Entry* p = next;
    while ((val.val > p->val.val) && p->next) p = p->next;
    if (p->val.val <= val.val) {
      unlink();
      place_after(p);
    }
    else {
      unlink();
      place_before(p);
    }
  }
  return first();
}

CostTable::Entry* CostTable::Entry::move_up()
{
  if (prev) if (val.val < prev->val.val) {
    Entry* p = prev;
    while ((val.val < p->val.val) && p->prev) p = p->prev;
    if (p->val.val <= val.val) {
      unlink();
      place_after(p);
    }
    else {
      unlink();
      place_before(p);
    }
  }
  return first();
}

void CostTable::Entry::unlink()
{
  if (prev) prev->next = next;
  if (next) next->prev = prev;
  prev = 0;
  next = 0;
}

void CostTable::Entry::place_before(Entry* p)
{
  prev = p->prev;
  if (p->prev) p->prev->next = this;
  next = p;
  p->prev = this;
}

void CostTable::Entry::place_after(Entry* p)
{
  next = p->next;
  if (p->next) p->next->prev = this;
  p->next = this;
  prev = p;
}

void CostTable::Entry::delete_list()
{
  Entry* l = this;
  while (l) {
    Entry* e = l;
    l = l->next;
    delete e;
  }
}

count_type CostTable::Entry::list_length()
{
  count_type n = 0;
  for (Entry* e = this; e; e = e->next) n += 1;
  return n;
}

CostTable::Entry* CostTable::insert_entry
(CostTable::Entry* e, CostTable::Entry* l)
{
  if (!l) {
    return e;
  }
  if (e->val.val < l->val.val) {
    e->next = l;
    l->prev = e;
    return e;
  }
  Entry* p = l;
  while ((p->val.val <= e->val.val) && p->next) {
    if (p->set == e->set) return l;
    p = p->next;
  }
  if (p->val.val <= e->val.val) {
    e->place_after(p);
  }
  else {
    e->place_before(p);
  }
  return l;
}

CostTable::Entry* CostTable::prepend_entry
(CostTable::Entry* e, CostTable::Entry* l)
{
  if (!l) {
    return e;
  }
  e->next = l;
  l->prev = e;
  return e;
}

CostTable::Entry* CostTable::remove_entry
(CostTable::Entry* e, CostTable::Entry* l)
{
  if (e == l) {
    if (e->next) e->next->prev = 0;
    Entry* r = e->next;
    e->next = 0;
    e->prev = 0;
    return r;
  }
  else {
    if (e->next) e->next->prev = e->prev;
    if (e->prev) e->prev->next = e->next;
    e->next = 0;
    e->prev = 0;
    return l;
  }
}

CostTable::Entry* CostTable::copy_list(Entry* l)
{
  Entry* r = 0;
  while (l) {
    Entry* e = new Entry(l->set, l->val);
    r = insert_entry(e, r);
    l = l->next;
  }
  return r;
}

CostTable::Entry* CostTable::find_entry(index_set& s, Entry* l)
{
  for (Entry* e = l; e; e = e->next)
    if (e->set == s) return e;
  return 0;
}

count_type CostTable::write_list(std::ostream& s, Entry* l)
{
  count_type n = 0;
  for (Entry* e = l; e; e = e->next) {
    instance.write_atom_set(s, e->set);
    s << ": " << e->val << " (" << eval(e->set) << ")" << std::endl;
    n += 1;
  }
  return n;
}

void CostTable::store_list(Entry* l)
{
  for (Entry* e = l; e; e = e->next)
    store(e->set, e->val.val, e->val.opt);
}

CostTable::Entry* CostTable::atoms()
{
  Entry* l = 0;
  index_set s;
  s.set_length(1);
  for (index_type i = 0; i < instance.n_atoms(); i++) {
    s[0] = i;
    Entry* e = new Entry(s, val(i));
    l = insert_entry(e, l);
  }
  return l;
}

CostTable::Entry* CostTable::pairs()
{
  Entry* l = atoms();
  index_set s;
  s.set_length(2);
  for (index_type i = 0; i < instance.n_atoms(); i++) {
    for (index_type j = i + 1; j < instance.n_atoms(); j++) {
      CostNode::Value v = next(i).val(j);
      if (!v.opt && FINITE(v.val)) {
	s[0] = i;
	s[1] = j;
	Entry* e = new Entry(s, v);
	l = insert_entry(e, l);
      }
    }
  }
  return l;
}

bool CostTable::interesting(const index_set& s)
{
  NTYPE v = eval(s);
  for (index_type k = 0; k < s.length(); k++) {
    index_set sub_s(s);
    sub_s.remove(k);
    if (eval(sub_s) >= v) return false;
  }
  return true;
}

CostTable::Entry* CostTable::entries
(CostNode* n, index_set s, Entry* l, const index_set* filter,
 bool ign_zero, bool ign_opt, bool ign_inf, bool req_mark, bool sort)
{
  index_type i = s.length();
  s.inc_length();
  for (index_type k = n->first(); k < instance.n_atoms(); k++) {
    s[i] = k;
    bool accept = true;
    if (accept && req_mark) {
      if (!n->val(k).mark) accept = false;
    }
    if (accept && (filter != 0)) {
      if (!filter->contains(k)) accept = false;
    }
    if (accept && ign_zero) {
      if (n->val(k).val == 0) accept = false;
    }
    if (accept && ign_opt) {
      if (n->val(k).opt) accept = false;
    }
    if (accept && ign_inf) {
      if (INFINITE(n->val(k).val)) accept = false;
    }
    if (accept) {
      if (sort) {
	l = insert_entry(new Entry(s, n->val(k)), l);
      }
      else {
	l = prepend_entry(new Entry(s, n->val(k)), l);
      }
    }
    if (n->val(k).next)
      l = entries(n->val(k).next, s, l, filter, ign_zero, ign_opt, ign_inf,
		  req_mark, sort);
  }
  s.dec_length();
  return l;
}

count_type CostTable::count_entries
(CostNode* n, index_set s, bool ign_zero, bool ign_opt, bool ign_inf)
{
  count_type l = 0;
  index_type i = s.length();
  s.inc_length();
  for (index_type k = n->first(); k < instance.n_atoms(); k++) {
    s[i] = k;
    if (((n->val(k).val > 0) || !ign_zero) &&
	(!n->val(k).opt || !ign_opt) &&
	(FINITE(n->val(k).val) || !ign_inf))
    {
      l += 1;
    }
    if (n->val(k).next)
      l += count_entries(n->val(k).next, s, ign_zero, ign_opt, ign_inf);
  }
  s.dec_length();
  return l;
}

CostTable::Entry* CostTable::entries()
{
  index_set s;
  return entries(this, s, 0, 0, false, false, false, false, true);
}

CostTable::Entry* CostTable::entries
(bool ign_zero, bool ign_opt, bool ign_inf, bool req_mark)
{
  index_set s;
  return entries(this, s, 0, 0, ign_zero, ign_opt, ign_inf, req_mark, true);
}

CostTable::Entry* CostTable::unsorted_entries
(bool ign_zero, bool ign_opt, bool ign_inf, bool req_mark)
{
  index_set s;
  return entries(this, s, 0, 0, ign_zero, ign_opt, ign_inf, req_mark, false);
}

CostTable::Entry* CostTable::boostable_entries()
{
  index_set s;
  return entries(this, s, 0, 0, true, true, true, false, true);
}

CostTable::Entry* CostTable::boostable_entries(const index_set& f)
{
  index_set s;
  return entries(this, s, 0, &f, true, true, true, false, true);
}

CostTable::Entry* CostTable::boost_cd_entries()
{
  index_set s;
  return entries(this, s, 0, 0, true, false, true, false, true);
}

CostTable::Entry* CostTable::marked_entries(bool ign_nb)
{
  index_set s;
  return entries(this, s, 0, 0, ign_nb, ign_nb, ign_nb, true, true);
}

CostTable::Entry* CostTable::entries(const index_set& f)
{
  index_set s;
  return entries(this, s, 0, &f, false, false, false, false, true);
}

count_type CostTable::count_entries(bool ign_zero, bool ign_opt, bool ign_inf)
{
  index_set s;
  return count_entries(this, s, ign_zero, ign_opt, ign_inf);
}

void CostTable::fill(index_set& s, index_type d)
{
  index_type l = s.length();
  s.inc_length();
  for (index_type k = (l > 0 ? s[l-1] + 1 : 0); k < instance.n_atoms(); k++) {
    s[l] = k;
    store(s, eval(s));
    if (d > s.length()) fill(s, d);
  }
  s.dec_length();
}

void CostTable::fill(index_type to_depth)
{
  index_set s;
  fill(s, to_depth);
}

void CostTable::clear()
{
  CostNode::clear();
  if (pre_cost) {
    delete pre_cost;
    pre_cost = 0;
  }
  if (per_cost) {
    delete per_cost;
    per_cost = 0;
  }
}

NTYPE CostTable::eval(index_type i)
{
  return val(i).val;
}

NTYPE CostTable::eval(index_type i, index_type j)
{
  if (i == j) {
    return val(i).val;
  }
  else {
    sort2(i, j);
    NTYPE v = MAX(val(i).val, val(j).val);
    if (val(i).next) {
      v = MAX(v, next(i).val(j).val);
    }
    return v;
  }
}

NTYPE CostTable::eval(index_type i, index_type j, index_type k)
{
  if (i == j) {
    return eval(i, k);
  }
  else if (i == k) {
    return eval(i, j);
  }
  else if (j == k) {
    return eval(i, j);
  }
  else {
    sort3(i, j, k);
    NTYPE v = MAX(MAX(val(i).val, val(j).val), val(k).val);
    if (val(i).next) {
      v = MAX(v, MAX(next(i).val(j).val, next(i).val(k).val));
      if (next(i).val(j).next) {
	v = MAX(v, next(i).next(j).val(k).val);
      }
    }
    if (val(j).next) {
      v = MAX(v, next(j).val(k).val);
    }
    return v;
  }
}

NTYPE CostTable::action_pre_cost(index_type i)
{
  if (pre_cost) return pre_cost[i];
  else return eval(instance.actions[i].pre);
}

NTYPE CostTable::action_per_cost(index_type i)
{
  if (per_cost) return per_cost[i];
  else {
    index_set per_set(instance.actions[i].pre);
    per_set.subtract(instance.actions[i].del);
    return eval(per_set);
  }
}

bool CostTable::update(index_type i, NTYPE v, bool_vec& f)
{
#ifdef ENABLE_HIGHER_TRACE_LEVEL
  if (trace_level > 2) {
    std::cerr << "update: " << i << "." << instance.atoms[i].name
	      << " = " << v << " (" << val(i) << ")" << std::endl;
  }
#endif
  if (val(i) > v) {
    val(i) = CostNode::Value(v, false);
    for (index_type k = 0; k < instance.atoms[i].req_by.length(); k++)
      if (instance.actions[instance.atoms[i].req_by[k]].sel)
	f[instance.atoms[i].req_by[k]] = true;
    return true;
  }
  return false;
}

bool CostTable::update(index_type i, NTYPE v)
{
#ifdef ENABLE_HIGHER_TRACE_LEVEL
  if (trace_level > 2) {
    std::cerr << "update: " << i << "." << instance.atoms[i].name
	      << " = " << v << " (" << val(i) << ")" << std::endl;
  }
#endif
  if (val(i) > v) {
    val(i) = CostNode::Value(v, false);
    return true;
  }
  return false;
}

bool CostTable::update
(index_type i, index_type j, NTYPE v)
{
  if (i == j) {
    CostNode::Value& v_store = val(i);
#ifdef ENABLE_HIGHER_TRACE_LEVEL
    if (trace_level > 2) {
      std::cerr << "update: " << i << "." << instance.atoms[i].name
		<< ", " << j << "." << instance.atoms[j].name
		<< " = " << v << " (" << v_store << ")"
		<< std::endl;
    }
#endif
    if (v_store > v) {
      v_store = CostNode::Value(v, false);
      return true;
    }
    return false;
  }
  else {
    sort2(i, j);
    CostNode::Value& v_store = next(i).val(j);
#ifdef ENABLE_HIGHER_TRACE_LEVEL
    if (trace_level > 2) {
      std::cerr << "update: " << i << "." << instance.atoms[i].name
		<< ", " << j << "." << instance.atoms[j].name
		<< " = " << v << " (" << v_store << ")"
		<< std::endl;
    }
#endif
    if (v_store > v) {
      v_store = CostNode::Value(v, false);
      return true;
    }
    return false;
  }
}

bool CostTable::update
(index_type i, index_type j, index_type l, NTYPE v)
{
  if (i == j) {
    return update(i, l, v);
  }
  else if (i == l) {
    return update(i, j, v);
  }
  else if (j == l) {
    return update(i, j, v);
  }
  else {
    sort3(i, j, l);
    CostNode::Value& v_store = next(i).next(j).val(l);
#ifdef ENABLE_HIGHER_TRACE_LEVEL
    if (trace_level > 2) {
      std::cerr << "update: " << i << "." << instance.atoms[i].name
		<< ", " << j << "." << instance.atoms[j].name
		<< ", " << l << "." << instance.atoms[l].name
		<< " = " << v << " (" << v_store << ")"
		<< std::endl;
    }
#endif
    if (v_store > v) {
      v_store = CostNode::Value(v, false);
      return true;
    }
    return false;
  }
}

void CostTable::closure(const ACF& cost)
{
  bool done = false;
  while (!done) {
    done = true;
    for (index_type k = 0; k < instance.n_actions(); k++) {
      if (stats.break_signal_raised()) return;
      if (instance.actions[k].sel) {
	NTYPE c_pre = eval(instance.actions[k].pre);
	if (FINITE(c_pre)) {
	  for (index_type i = 0; i < instance.actions[k].add.length(); i++)
	    if (update(instance.actions[k].add[i], c_pre + cost(k)))
	      done = false;
	}
      }
    }
  }
}

void CostTable::closure(const ACF& cost, const index_set& nd)
{
  bool r_sel[instance.n_actions()];
  for (index_type k = 0; k < instance.n_actions(); k++) {
    r_sel[k] = instance.actions[k].sel;
    if (instance.actions[k].del.first_common_element(nd) != no_such_index)
      instance.actions[k].sel = false;
  }
  closure(cost);
  for (index_type k = 0; k < instance.n_actions(); k++)
    instance.actions[k].sel = r_sel[k];
}

bool CostTable::compatible
(index_type a, index_type b, bool opt_resources) const
{
  // check non-interference (i.e. non-strict commutativity)
  if (!instance.non_interfering(a, b)) return false;
  // check locks
  if (!instance.lock_compatible(a, b)) return false;
  // check resources
  if (opt_resources) {
    if (!instance.resource_compatible(a, b)) return false;
  }
  return true;
}

void CostTable::compute_H1(const ACF& cost)
{
  bool_vec init(instance.init_atoms, instance.n_atoms());
  compute_H1(cost, init);
}

void CostTable::compute_H1(const ACF& cost, const bool_vec& init)
{
  stats.start();
  bool_vec f(false, instance.n_actions());
  if (!pre_cost) pre_cost = new NTYPE[instance.n_actions()];

  // init pre_cost array
  for (index_type k = 0; k < instance.n_actions(); k++) {
    pre_cost[k] = POS_INF;
    if ((instance.actions[k].pre.length() == 0) && instance.actions[k].sel)
      f[k] = true;
  }

  // init table
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    if (init[k]) {
      val(k) = CostNode::Value(0, true);
      for (index_type i = 0; i < instance.atoms[k].req_by.length(); i++) {
	if (instance.actions[instance.atoms[k].req_by[i]].sel)
	  f[instance.atoms[k].req_by[i]] = true;
      }
    }
    else val(k) = CostNode::Value(POS_INF, false);
  }

  bool done = false;
  while (!done) {
    done = true;
    for (index_type k = 0; k < instance.n_actions(); k++) if (f[k]) {
      if (stats.break_signal_raised()) return;
      NTYPE c_pre = eval(instance.actions[k].pre);
      if (FINITE(c_pre) && (c_pre < pre_cost[k])) {
	for (index_type i = 0; i < instance.actions[k].add.length(); i++)
	  update(instance.actions[k].add[i], c_pre + cost(k), f);
	pre_cost[k] = c_pre;
	done = false;
      }
      f[k] = false;
    }
  }
  stats.stop();
}

void CostTable::compute_H1max(const ACF& cost, const bool_vec& init)
{
  stats.start();
  bool_vec f(false, instance.n_actions());
  if (!pre_cost) pre_cost = new NTYPE[instance.n_actions()];

  // init pre_cost array
  for (index_type k = 0; k < instance.n_actions(); k++) {
    pre_cost[k] = POS_INF;
    if ((instance.actions[k].pre.length() == 0) &&
	instance.actions[k].sel)f[k] = true;
    else f[k] = false;
  }

  // init table
  for (index_type k = 0; k < instance.n_atoms(); k++) {
    if (init[k]) {
      val(k) = CostNode::Value(0, true);
      for (index_type i = 0; i < instance.atoms[k].req_by.length(); i++) {
	if (instance.actions[instance.atoms[k].req_by[i]].sel)
	  f[instance.atoms[k].req_by[i]] = true;
      }
    }
    else val(k) = CostNode::Value(POS_INF, false);
  }

  bool done = false;
  while (!done) {
    done = true;
    for (index_type k = 0; k < instance.n_actions(); k++) if (f[k]) {
      if (stats.break_signal_raised()) return;
      NTYPE c_pre = eval(instance.actions[k].pre);
      if (FINITE(c_pre) && (c_pre < pre_cost[k])) {
	for (index_type i = 0; i < instance.actions[k].add.length(); i++)
	  update(instance.actions[k].add[i], MAX(c_pre, cost(k)), f);
	pre_cost[k] = c_pre;
	done = false;
      }
      f[k] = false;
    }
  }
  stats.stop();
}

void CostTable::compute_H2(const ACF& cost)
{
  bool_vec init(instance.init_atoms, instance.n_atoms());
  compute_H2(cost, init);
}

void CostTable::compute_H2(const ACF& cost, const bool_vec& init)
{
  if (trace_level > 2) {
    std::cerr << "computing H2..." << std::endl;
  }
  stats.start();

  // init table
  for (index_type i = 0; i < instance.n_atoms(); i++) {
    if (init[i])
      val(i) = CostNode::Value(0, true);
    else
      val(i) = CostNode::Value(POS_INF, false);
    for (index_type j = i + 1; j < instance.n_atoms(); j++) {
      if (init[i] && init[j])
	next(i).val(j) = CostNode::Value(0, true);
      else
	next(i).val(j) = CostNode::Value(POS_INF, false);
    }
  }

  bool done = false;
  while (!done) {
    if (trace_level > 2) {
      std::cerr << "begin iteration..." << std::endl;
    }
    done = true;
    for (index_type a = 0; a < instance.n_actions(); a++) {
      if (stats.break_signal_raised()) return;
      Instance::Action& act = instance.actions[a];
      NTYPE c_pre = eval(act.pre);
      if (FINITE(c_pre)) {
#ifdef ENABLE_HIGHER_TRACE_LEVEL
	if (trace_level > 2) {
          std::cerr << "updating over " << a << "." << act.name
                    << " (pre = " << act.pre << ", add = " << act.add
                    << ", del = " << act.del << ", cost = " << cost(a)
                    << ")..." << std::endl;
	}
#endif
	for (index_type i = 0; i < instance.n_atoms(); i++) {
	  // if i in act.add...
	  if (act.add.contains(i)) {
	    // update cost of i
	    if (update(i, c_pre + cost(a))) done = false;
	    for (index_type j = 0; j < act.add.length(); j++)
	      // update cost of {i,q} for every q in act.add
	      if (act.add[j] > i) // q < i have already been updated (as i)
		if (update(act.add[j], i, c_pre + cost(a))) done = false;
	  }
	  // else if i not in act.del (i.e. case act + noop(i))...
	  else if (!act.del.contains(i)) {
	    // update cost of {i,q} for every q in act.add
	    for (index_type j = 0; j < act.add.length(); j++) {
	      // compute H(act.pre U {i})
	      NTYPE c_join = MAX(c_pre, eval(i));
	      if (!act.pre.contains(i)) {
		for (index_type k = 0; k < act.pre.length(); k++)
		  c_join = MAX(eval(act.pre[k], i), c_join);
	      }
	      if (FINITE(c_join))
		if (update(act.add[j], i, c_join + cost(a))) done = false;
	    }
	  }
	} // for each atom i
      } // if FINITE(c_pre)
    } // for each action
    if (trace_level > 2) {
      std::cerr << "end iteration (done = " << done << ")" << std::endl;
    }
  }
  stats.stop();
}

void CostTable::compute_H3(const ACF& cost)
{
  stats.start();

  // init table
  for (index_type i = 0; i < instance.n_atoms(); i++) {
    if (instance.atoms[i].init)
      val(i) = CostNode::Value(0, true);
    else
      val(i) = CostNode::Value(POS_INF, false);
    for (index_type j = i + 1; j < instance.n_atoms(); j++) {
      if (instance.atoms[i].init && instance.atoms[j].init)
	next(i).val(j) = CostNode::Value(0, true);
      else
	next(i).val(j) = CostNode::Value(POS_INF, false);
      for (index_type l = j + 1; l < instance.n_atoms(); l++) {
	if (instance.atoms[i].init &&
	    instance.atoms[j].init &&
	    instance.atoms[l].init)
	  next(i).next(j).val(l) = CostNode::Value(0, true);
	else
	  next(i).next(j).val(l) = CostNode::Value(POS_INF, false);
      }
    }
  }

  bool done = false;
  while (!done) {
    done = true;
    for (index_type a = 0; a < instance.n_actions(); a++) {
      if (stats.break_signal_raised()) return;
      Instance::Action& act = instance.actions[a];
      NTYPE c_pre = eval(act.pre);
      if (FINITE(c_pre)) {
#ifdef ENABLE_HIGHER_TRACE_LEVEL
	if (trace_level > 2) {
          std::cerr << "updating over " << a << "." << act.name
                    << " (pre = " << act.pre << ", add = " << act.add
                    << ", del = " << act.del << ", cost = " << cost(a)
                    << ")..." << std::endl;
	}
#endif
	for (index_type i = 0; i < instance.n_atoms(); i++) {
	  // if i in act.add...
	  if (act.add.contains(i)) {
	    // update cost of i
	    if (update(i, c_pre + cost(a))) done = false;
	    for (index_type j = 0; j < act.add.length(); j++)
	      // update cost of {i,q} for every q in act.add
	      if (act.add[j] > i) { // q < i have already been updated (as i)
		if (update(i, act.add[j], c_pre + cost(a))) done = false;
		// update cost of {i,q,q'} for every q' in act.add
		for (index_type l = j + 1; l < act.add.length(); l++)
		  if (act.add[l] > i)
		    if (update(i, act.add[j], act.add[l], c_pre + cost(a)))
		      done = false;
	      }
	  }
	  // else if i not in act.del (i.e. case act + noop(i))...
	  else if (!act.del.contains(i)) {
	    // compute H(act.pre U {i})
	    NTYPE c_join = MAX(c_pre, eval(i));
	    if (!act.pre.contains(i)) {
	      for (index_type k = 0; k < act.pre.length(); k++)
		c_join = MAX(eval(act.pre[k], i), c_join);
	    }
	    if (FINITE(c_join)) {
	      // update cost of {i,q} and {i,q,q'} for every q,q' in act.add
	      for (index_type j = 0; j < act.add.length(); j++) {
		if (update(i, act.add[j], c_join + cost(a))) done = false;
		for (index_type l = j + 1; l < act.add.length(); l++)
		  if (update(i, act.add[j], act.add[l], c_join + cost(a)))
		    done = false;
	      }
	      // finally, check case act + noop(i) + noop(j)
	      for (index_type j = i + 1; j < instance.n_atoms(); j++)
		if (!act.del.contains(j)) {
		  // compute H(act.pre U {i,j})
		  NTYPE c_jj = MAX(c_join, eval(i, j));
		  if (!act.pre.contains(j)) {
		    for (index_type k = 0; k < act.pre.length(); k++)
		      c_jj = MAX(eval(act.pre[k], j), c_jj);
		  }
		  if (FINITE(c_jj)) {
		    // update cost of {i,j,q} for every q in act.add
		    for (index_type l = 0; l < act.add.length(); l++)
		      if (update(i, j, act.add[l], c_jj + cost(a)))
			done = false;
		  }
		}
	    } // FINITE(c_join)
	  }
	} // for each atom i
      } // if FINITE(c_pre)
    } // for each action
  }
  stats.stop();
}

void CostTable::compute_H2C(const ACF& cost, bool opt_resources)
{
  stats.start();

  // init table
  for (index_type i = 0; i < instance.n_atoms(); i++) {
    if (instance.atoms[i].init)
      val(i) = CostNode::Value(0, true);
    else
      val(i) = CostNode::Value(POS_INF, false);
    for (index_type j = i + 1; j < instance.n_atoms(); j++) {
      if (instance.atoms[i].init && instance.atoms[j].init)
	next(i).val(j) = CostNode::Value(0, true);
      else
	next(i).val(j) = CostNode::Value(POS_INF, false);
    }
  }

  bool done = false;
  while (!done) {
    done = true;
    for (index_type a = 0; a < instance.n_actions(); a++) {
      if (stats.break_signal_raised()) return;
      Instance::Action& act = instance.actions[a];
      NTYPE c_pre = eval(act.pre);
      if (FINITE(c_pre)) {
#ifdef ENABLE_HIGHER_TRACE_LEVEL
	if (trace_level > 2) {
	  std::cerr << "updating over " << a << "." << act.name
		    << "..." << std::endl;
	}
#endif
	for (index_type i = 0; i < instance.n_atoms(); i++) {
	  // if i in act.add...
	  if (act.add.contains(i)) {
	    // update cost of i
	    if (update(i, c_pre + cost(a))) done = false;
	    for (index_type j = 0; j < act.add.length(); j++)
	      // update cost of {i,q} for every q in act.add
	      if (act.add[j] > i) // q < i have already been updated (as i)
		if (update(act.add[j], i, c_pre + cost(a))) done = false;
	  }
	  // else if i not in act.del (i.e. case act + noop(i))...
#ifdef SUPPORT_VOLATILE_ATOMS
	  else if (!act.del.contains(i) && !instance.atoms[i].volatile) {
#else
          else if (!act.del.contains(i)) {
#endif
	    // update cost of {i,q} for every q in act.add
	    for (index_type j = 0; j < act.add.length(); j++) {
	      // compute H(act.pre U {i})
	      NTYPE c_join = MAX(c_pre, eval(i));
	      if (!act.pre.contains(i)) {
		for (index_type k = 0; k < act.pre.length(); k++)
		  c_join = MAX(eval(act.pre[k], i), c_join);
	      }
	      if (FINITE(c_join))
		if (update(act.add[j], i, c_join + cost(a))) done = false;
	    }
	  }
	}
	// update over act + act_b for every act_b compatible with act
	// (only need to update over b > a, since {a,b} and {b,a} symmetric
	for (index_type b = a + 1; b < instance.n_actions(); b++) {
	  Instance::Action& act_b = instance.actions[b];
	  if (compatible(a, b, opt_resources)) {
	    // compute H(act.pre U act_b.pre)
	    NTYPE c_b = eval(act_b.pre);
	    if (FINITE(c_b)) {
	      NTYPE c_join = MAX(c_pre, c_b);
	      for (index_type i = 0; i < act.pre.length(); i++)
		for (index_type j = 0; j < act_b.pre.length(); j++)
		  c_join = MAX(eval(act.pre[i], act_b.pre[j]), c_join);
	      // c_conc is H of act + act_b concurrently
	      NTYPE c_conc = MAX(MAX(c_pre + cost(a), c_b + cost(b)),
				 c_join + MIN(cost(a), cost(b)));
	      if (FINITE(c_conc)) {
		// only need to update pairs across act/act_b (pairs added
		// by act alone are handled above, pairs added by act_b alone
		// are updated when the outer loop reaches b)
		for (index_type i = 0; i < act.add.length(); i++)
		  for (index_type j = 0; j < act_b.add.length(); j++)
		    if (update(act.add[i], act_b.add[j], c_conc))
		      done = false;
	      } // if FINTITE(c_conc)
	    } // if FINITE(c_b)
	  }
	}
      } // if FINITE(c_pre)
    } // for each action
  }
  stats.stop();
}

void CostTable::verify_H2C
(const ACF& cost, bool opt_resources, bool opt_verbose)
{
  for (index_type i = 0; i < instance.n_atoms(); i++) {
    Instance::Atom& atom1 = instance.atoms[i];
    if (atom1.init) {
      if (opt_verbose)
	std::cerr << "atom " << atom1.name << " is initial" << std::endl;
    }
    else {
      if (opt_verbose) {
	std::cerr << "checking atom " << atom1.name << "..." << std::endl;
      }
      NTYPE c_tab = eval(atom1.index);
      NTYPE c_min = POS_INF;
      for (index_type k = 0; k < atom1.add_by.length(); k++) {
	Instance::Action& act = instance.actions[atom1.add_by[k]];
	NTYPE c_act = eval(act.pre) + cost(act.index);
	if (opt_verbose) {
	  std::cerr << act.name << ": " << c_act << std::endl;
	}
	c_min = MIN(c_act, c_min);
      }
      if (opt_verbose) {
	std::cerr << "min " << atom1.name << " = " << c_min << ", ";
	if (c_min != c_tab) {
	  std::cerr << "error: H(" << atom1.name << ") = " << c_tab << std::endl;
	}
	else {
	  std::cerr << "ok" << std::endl;
	}
      }
      else {
	if (c_min > c_tab) {
	  std::cerr << "improvement: H(" << atom1.name << ") = " << c_min
		    << " (stored: " << c_tab << ")" << std::endl;
	}
      }
    }
    for (index_type j = i+1; j < instance.n_atoms(); j++) {
      Instance::Atom& atom2 = instance.atoms[j];
      if (atom1.init && atom2.init) {
	if (opt_verbose)
	  std::cerr << "pair {" << atom1.name << ", " << atom2.name
		    << "} is initial" << std::endl;
      }
      else {
	if (opt_verbose) {
	  std::cerr << "checking pair {" << atom1.name
		    << ", " << atom2.name << "}..." << std::endl;
	}
	NTYPE c_tab = eval(atom1.index, atom2.index);
	NTYPE c_min = POS_INF;
	for (index_type k = 0; k < atom1.add_by.length(); k++) {
	  Instance::Action& act1 = instance.actions[atom1.add_by[k]];
	  if (!act1.del.contains(atom2.index)) {
	    // act1 adds both atoms
	    if (act1.add.contains(atom2.index)) {
	      NTYPE c_both = eval(act1.pre) + cost(act1.index);
	      if (opt_verbose) {
		std::cerr << act1.name << ": " << c_both << std::endl;
	      }
	      c_min = MIN(c_both, c_min);
	    }
	    else {
	      // act1 + NOOP(atom2)
	      index_set s0(act1.pre);
	      s0.insert(atom2.index);
	      NTYPE c_1n = eval(s0) + cost(act1.index);
	      if (opt_verbose) {
		std::cerr << act1.name << ", NOOP(" << atom2.name << "): "
			  << c_1n << std::endl;
	      }
	      c_min = MIN(c_1n, c_min);
	      for (index_type l = 0; l < atom2.add_by.length(); l++) {
		Instance::Action& act2 = instance.actions[atom2.add_by[l]];
		if (compatible(act1.index, act2.index, opt_resources) &&
		    !act2.add.contains(atom1.index)) {
		  index_set s(act1.pre);
		  s.insert(act2.pre);
		  NTYPE c_conc;
		  if (cost(act1.index) > cost(act2.index)) {
		    NTYPE c_1 = eval(act1.pre) + cost(act1.index);
		    NTYPE c_join = eval(s) + cost(act2.index);
		    c_conc = MAX(c_1, c_join);
		  }
		  else {
		    NTYPE c_2 = eval(act2.pre) + cost(act2.index);
		    NTYPE c_join = eval(s) + cost(act1.index);
		    c_conc = MAX(c_2, c_join);
		  }
		  if (opt_verbose) {
		    std::cerr << act1.name << ", " << act2.name << ": "
			      << c_conc << std::endl;
		  }
		  c_min = MIN(c_conc, c_min);
		}
	      }
	    }
	  }
	}
	for (index_type k = 0; k < atom2.add_by.length(); k++) {
	  Instance::Action& act = instance.actions[atom2.add_by[k]];
	  if (!act.del.contains(atom1.index) &&
	      !act.add.contains(atom1.index)) {
	    index_set s(act.pre);
	    s.insert(atom1.index);
	    NTYPE c_2n = eval(s) + cost(act.index);
	    if (opt_verbose) {
	      std::cerr << "NOOP(" << atom1.name << "), " << act.name << ": "
			<< c_2n << std::endl;
	    }
	    c_min = MIN(c_2n, c_min);
	  }
	}
	if (opt_verbose) {
	  std::cerr << "min {" << atom1.name << ", " << atom2.name << "} = "
		    << c_min << ", ";
	  if (c_min != c_tab) {
	    std::cerr << "error: H(" << atom1.name << ", " << atom2.name
		      << ") = " << c_tab << std::endl;
	  }
	  else {
	    std::cerr << "ok" << std::endl;
	  }
	}
	else {
	  if (c_min > c_tab) {
	    std::cerr << "improvement: H(" << atom1.name << "," << atom2.name
		      << ") = " << c_min << " (stored: " << c_tab << ")"
		      << std::endl;
	  }
	}
      }
    }
  }
}

CostTable::Entry* CostTable::boost_cd
(Entry* list, StateFactory& search_space,
 index_type cd_size_limit, const index_set& cd_atoms_ignore,
 NTYPE cost_limit, const index_set& cost_limit_set,
 count_type wps_limit, bool scale_wps)
{
#ifdef ROLLBACK
  Result boost_search_res;
#else
  Conflicts boost_search_res(instance);
#endif
  IDAO boost_search(stats, boost_search_res);
  boost_search.set_trace_level(trace_level - 2);
  boost_search.set_store_cost(false);

  if (trace_level > 2) {
    std::cerr << "entries in: ";
    write_list(std::cerr, list);
    std::cerr << std::endl;
  }

  stats.start();
  NTYPE current_bound = 0;
  count_type n_entries_in = (list ? list->list_length() : 0);
  count_type n_before = n_entries_created;

  while (list) {
    Entry* e = list;

    if (INFINITE(e->val.val)) {
      stats.stop();
      count_type n_entries_out = (list ? list->list_length() : 0);
      n_entries_boosted += (n_entries_in - n_entries_out);
      if (boost_new_entries)
	n_entries_boosted += (n_entries_created - n_before);
      if (trace_level > 0) {
	std::cerr << "done at INF (" << n_entries_solved << " solved, "
		  << n_entries_created << " created, "
		  << n_entries_discarded << " discarded, "
		  << (list ? list->list_length() : 0) << " in list, "
		  << stats.nodes() << " nodes, "
		  << stats.time() << " sec.)" << std::endl;
      }
      if (trace_level > 2) {
	std::cerr << "entries out: ";
	write_list(std::cerr, list);
	std::cerr << std::endl;
      }
      return list;
    }

    if (e->val.val > cost_limit) {
      cost_limit = eval(cost_limit_set);
      if (e->val.val > cost_limit) {
	stats.stop();
	count_type n_entries_out = (list ? list->list_length() : 0);
	n_entries_boosted += (n_entries_in - n_entries_out);
	if (boost_new_entries)
	  n_entries_boosted += (n_entries_created - n_before);
	if (trace_level > 0) {
	  std::cerr << "done at " << e->val.val
		    << " (" << n_entries_solved << " solved, "
		    << n_entries_created << " created, "
		    << n_entries_discarded << " discarded, "
		    << (list ? list->list_length() : 0) << " in list, "
		    << stats.nodes() << " nodes, "
		    << stats.time() << " sec.)" << std::endl;
	}
	if (trace_level > 2) {
	  std::cerr << "entries out: ";
	  write_list(std::cerr, list);
	  std::cerr << std::endl;
	}
	return list;
      }
      else if (trace_level > 0) {
	std::cerr << "limit is " << cost_limit << std::endl;
      }
    }

    NTYPE boost_limit = cost_limit;
    if (e->next) {
      boost_limit = e->next->val.val;
    }

    list = e->next;
    e->unlink();

    if (trace_level > 0) {
      if (e->val.val > current_bound) {
	std::cerr << "at " << e->val.val
		  << " (" << n_entries_solved << " solved, "
		  << n_entries_created << " created, "
		  << n_entries_discarded << " discarded, "
		  << (list ? list->list_length() : 0) << " in list, "
		  << stats.nodes() << " nodes, "
		  << stats.time() << " sec.)" << std::endl;
	current_bound = e->val.val;
      }
    }

    count_type w_lim = wps_limit;
    bool wps_break_flag = false;
    if (scale_wps) {
      if (2*(e->val.work) > w_lim) w_lim = 2*(e->val.work);
    }
    else {
      if (e->val.work > w_lim) wps_break_flag = true;
    }

    if (!wps_break_flag) {
      State* boost_state = search_space.new_state(e->set, 0);
      NTYPE c_est = boost_state->est_cost();
#ifdef ROLLBACK
      Collector cd_state(instance, *boost_state);
#endif

      NTYPE c_new = 0;
      if (e->set.length() < cd_size_limit) {
	if (trace_level > 2) {
	  std::cerr << "boosting " << *boost_state
		    << " (cost = " << c_est
		    << ", limit = " << boost_limit
		    << ", cd = "
		    << (strong_conflict_detection ? "strong" :
			(ultra_weak_conflict_detection ? "ultra weak" : "weak"))
		    << ", wps = " << w_lim
		    << ")... " << std::endl;
	}
	if (strong_conflict_detection || ultra_weak_conflict_detection) {
	  boost_search_res.set_stop_condition(Result::stop_at_all_optimal);
	}
	else {
	  boost_search_res.set_stop_condition(Result::stop_at_first);
	}
	boost_search_res.reset();
	boost_search.set_cost_limit(boost_limit);
	boost_search.set_work_limit(w_lim);
#ifdef ROLLBACK
	c_new = boost_search.start(cd_state);
#else
	c_new = boost_search.start(*boost_state);
#endif
	n_boost_searches_with_cd += 1;
      }
      else {
	if (trace_level > 2) {
	  std::cerr << "boosting " << *boost_state
		    << " (cost = " << c_est
		    << ", limit = " << boost_limit
		    << ", cd = off, wps = " << w_lim
		    << ")... " << std::endl;
	}
	boost_search_res.reset();
	boost_search_res.set_stop_condition(Result::stop_at_first);
	boost_search.set_cost_limit(boost_limit);
	boost_search.set_work_limit(w_lim);
	c_new = boost_search.start(*boost_state);
      }

      wps_break_flag = ((c_new == e->val.val) && !boost_search.solved());
      if (wps_break_flag) e->val.work = w_lim;

      e->val = CostNode::Value(c_new, boost_search.solved());
      n_boost_searches += 1;

      if (trace_level > 1) {
	if (wps_break_flag) {
	  std::cerr << *boost_state << " (cost = " << c_est
		    << ") discarded (wps = " << w_lim
		    << ", " << stats << ")" << std::endl;
	}
	else {
	  std::cerr << *boost_state << ": cost " << c_est << ", max "
		    << boost_search.get_cost_limit() << ", new " << e->val
		    << " (" << stats << ")" << std::endl;
	}
      }

      if (boost_search.solved()) {
	if (e->set.length() < cd_size_limit) {
	  list = create_new_entries(e->set,
				    (ultra_weak_conflict_detection ?
#ifdef ROLLBACK
				     cd_state.pos_deleted() :
				     cd_state.nec_deleted()),
#else
				     boost_search_res.pos_deleted() :
				     boost_search_res.nec_deleted()),
#endif
				    cd_atoms_ignore,
				    list);
	}
	delete e;
	n_entries_solved += 1;
      }
      else if (wps_break_flag) {
	n_entries_discarded += 1;
	delete e;
      }
      else {
	list = insert_entry(e, list);
      }

      delete boost_state;
    }

    // scale_wps is false && e->work > wps_limit
    else {
      n_entries_discarded += 1;
      delete e;
    }

    if (stats.break_signal_raised()) {
      stats.stop();
      count_type n_entries_out = (list ? list->list_length() : 0);
      n_entries_boosted += (n_entries_in - n_entries_out);
      if (boost_new_entries)
	n_entries_boosted += (n_entries_created - n_before);
      if (trace_level > 2) {
	std::cerr << "entries out: ";
	write_list(std::cerr, list);
	std::cerr << std::endl;
      }
      return list;
    }
  }

  stats.stop();
  count_type n_entries_out = (list ? list->list_length() : 0);
  n_entries_boosted += (n_entries_in - n_entries_out);
  if (boost_new_entries)
    n_entries_boosted += (n_entries_created - n_before);
  if (trace_level > 0) {
    std::cerr << "done (" << n_entries_solved << " solved, "
	      << n_entries_created << " created, "
	      << n_entries_discarded << " discarded, "
	      << (list ? list->list_length() : 0) << " in list, "
	      << stats.nodes() << " nodes, "
	      << stats.time() << " sec.)" << std::endl;
  }
  if (trace_level > 2) {
    std::cerr << "entries out: ";
    write_list(std::cerr, list);
    std::cerr << std::endl;
  }
  return list;
}

CostTable::Entry* CostTable::create_new_entries
(const index_set& set, const index_set& conflict_set,
 const index_set& cd_ignore, CostTable::Entry* list)
{
  for (index_type k = 0; k < conflict_set.length(); k++) {
    if (!set.contains(conflict_set[k]) &&
	!cd_ignore.contains(conflict_set[k])) {
      index_set new_set(set);
      new_set.insert(conflict_set[k]);
#ifdef ASSUME_UNIT_COST
      NTYPE v = eval(new_set) + (strong_conflict_detection ? 1 : 0);
#else
      NTYPE v = eval(new_set);
#endif
      if (FINITE(v)) {
	CostNode::Value& new_set_val = insert(new_set);
	if (!new_set_val.opt) {
	  if (new_set_val.val == 0) n_entries_created += 1;
	  if (v > new_set_val.val) new_set_val.val = v;
	  if (boost_new_entries && (find_entry(new_set, list) == 0)) {
	    Entry* new_entry = new Entry(new_set, new_set_val);
	    if (trace_level > 2) {
	      instance.write_atom_set(std::cerr, new_set);
	      std::cerr << " new at " << new_set_val << std::endl;
	    }
	    list = insert_entry(new_entry, list);
	  }
	}
      }
    }
  }
  return list;
}

CostTable::Entry* CostTable::boost
(Entry* list, StateFactory& search_space,
 NTYPE cost_limit, const index_set& cost_limit_set,
 count_type wps_limit, bool scale_wps)
{
  Result boost_search_res;
  IDAO boost_search(stats, boost_search_res);
  boost_search.set_trace_level(trace_level - 2);
  boost_search_res.set_n_to_find(1);
  boost_search.set_store_cost(false);

  stats.start();
  NTYPE current_bound = 0;
  count_type n_entries_in = (list ? list->list_length() : 0);

  while (list) {
    Entry* e = list;

    if (INFINITE(e->val.val)) {
      stats.stop();
      count_type n_entries_out = (list ? list->list_length() : 0);
      n_entries_boosted += (n_entries_in - n_entries_out);
      if (trace_level > 0) {
	std::cerr << "done at INF (" << n_entries_solved << " solved, "
		  << n_entries_discarded << " discarded, "
		  << (list ? list->list_length() : 0) << " in list, "
		  << stats.nodes() << " nodes, "
		  << stats.time() << " sec.)" << std::endl;
      }
      return list;
    }

    if (e->val.val > cost_limit) {
      cost_limit = eval(cost_limit_set);
      if (e->val.val > cost_limit) {
	stats.stop();
	count_type n_entries_out = (list ? list->list_length() : 0);
	n_entries_boosted += (n_entries_in - n_entries_out);
	if (trace_level > 0) {
	  std::cerr << "done at " << e->val.val
		    << " (" << n_entries_solved << " solved, "
		    << n_entries_discarded << " discarded, "
		    << (list ? list->list_length() : 0) << " in list, "
		    << stats.nodes() << " nodes, "
		    << stats.time() << " sec.)" << std::endl;
	}
	return list;
      }
      else if (trace_level > 0) {
	std::cerr << "limit is " << cost_limit << std::endl;
      }
    }

    NTYPE boost_limit = cost_limit;
    if (e->next) {
      boost_limit = e->next->val.val;
    }

    list = e->next;
    e->unlink();

    if (trace_level > 0) {
      if (e->val.val > current_bound) {
	std::cerr << "at " << e->val.val
		  << " (" << n_entries_solved << " solved, "
		  << n_entries_discarded << " discarded, "
		  << (list ? list->list_length() : 0) << " in list, "
		  << stats.nodes() << " nodes, "
		  << stats.time() << " sec.)" << std::endl;
	current_bound = e->val.val;
      }
    }

    if (e->val.opt) {
      delete e;
    }
    else {
      count_type w_lim = wps_limit;
      bool wps_break_flag = false;
      if (scale_wps) {
	if (2*(e->val.work) > w_lim) w_lim = 2*(e->val.work);
      }
      else {
	if (e->val.work > w_lim) wps_break_flag = true;
      }

      if (!wps_break_flag) {
	State* boost_state = search_space.new_state(e->set, 0);
	NTYPE c_est = boost_state->est_cost();

	if (trace_level > 2) {
	  std::cerr << "boosting " << *boost_state << " at " << c_est
		    << "..." << std::endl;
	}

	boost_search.set_cost_limit(boost_limit);
	boost_search.set_work_limit(w_lim);
	NTYPE c_new = boost_search.start(*boost_state);
	wps_break_flag = ((c_new == e->val.val) && !boost_search.solved());
	e->val = CostNode::Value(c_new, boost_search.solved());
	if (wps_break_flag) e->val.work = w_lim;
	n_boost_searches += 1;

	if (trace_level > 1) {
	  if (wps_break_flag) {
	    std::cerr << *boost_state << " (cost = " << c_est
		      << ") discarded (wps = " << w_lim
		      << ", " << stats << ")" << std::endl;
	  }
	  else {
	    std::cerr << *boost_state << ": cost " << c_est << ", max "
		      << boost_search.get_cost_limit() << ", new " << e->val
		      << " (" << stats << ")" << std::endl;
	  }
	}

//  	if (!boost_search.solved()) {
//  	  std::cerr << "cost improved, running quick boost pass..." << std::endl;
//  	  quick_boost_pass(list, search_space, cost_limit, cost_limit_set);
//  	}

	if (boost_search.solved()) {
	  delete e;
	  n_entries_solved += 1;
	}
	else if (wps_break_flag) {
	  n_entries_discarded += 1;
	  delete e;
	}
	else {
	  list = insert_entry(e, list);
	}
	delete boost_state;
      }

      else {
	n_entries_discarded += 1;
	delete e;
      }

      if (stats.break_signal_raised()) {
	stats.stop();
	count_type n_entries_out = (list ? list->list_length() : 0);
	n_entries_boosted += (n_entries_in - n_entries_out);
	return list;
      }
    }
  }

  stats.stop();
  count_type n_entries_out = (list ? list->list_length() : 0);
  n_entries_boosted += (n_entries_in - n_entries_out);
  if (trace_level > 0) {
    std::cerr << "done (" << n_entries_solved << " solved, "
	      << n_entries_discarded << " discarded, "
	      << (list ? list->list_length() : 0) << " in list, "
	      << stats.nodes() << " nodes, "
	      << stats.time() << " sec.)" << std::endl;
  }
  return list;
}


void CostTable::quick_boost_pass
(Entry* list, StateFactory& search_space,
 NTYPE cost_limit, const index_set& cost_limit_set)
{
  NoSearch ns;

  stats.start();
  NTYPE current_bound = 0;
  count_type n_pushed = 0;

  Entry* e = list;
  while (e) {

    if (INFINITE(e->val.val)) {
      stats.stop();
      if (trace_level > 0) {
	std::cerr << "done at INF (" << n_pushed << " pushed, "
		  << stats.time() << " sec.)" << std::endl;
      }
      return;
    }

    if (e->val.val > cost_limit) {
      cost_limit = eval(cost_limit_set);
      if (e->val.val > cost_limit) {
	stats.stop();
	if (trace_level > 0) {
	  std::cerr << "done at " << e->val.val
		    << " (" << n_pushed << " pushed, "
		    << stats.time() << " sec.)" << std::endl;
	}
	return;
      }
      else if (trace_level > 0) {
	std::cerr << "limit is " << cost_limit << std::endl;
      }
    }

    if (trace_level > 0) {
      if (e->val.val > current_bound) {
	std::cerr << "at " << e->val.val
		  << " (" << n_pushed << " pushed, "
		  << stats.time() << " sec.)" << std::endl;
	current_bound = e->val.val;
      }
    }

    if (!e->val.opt) {
      State* boost_state = search_space.new_state(e->set, 0);
      NTYPE c_est = boost_state->est_cost();
      NTYPE c_new = boost_state->expand(ns, c_est);
      if (c_new > c_est) {
	if (trace_level > 1) {
	  std::cerr << *boost_state << " pushed from "
		    << c_est << " to " << c_new << std::endl;
	}
	e->val = CostNode::Value(c_new, ns.solved());
	n_pushed += 1;
      }
      ns.reset();
      delete boost_state;
    }

    if (stats.break_signal_raised()) {
      stats.stop();
      return;
    }

    e = e->next;
  }

  stats.stop();
  if (trace_level > 0) {
    std::cerr << "done (" << n_pushed << " pushed, "
	      << stats.time() << " sec.)" << std::endl;
  }
}

void CostTable::compute_action_cost()
{
  if (!pre_cost) pre_cost = new NTYPE[instance.n_actions()];
  if (!per_cost) per_cost = new NTYPE[instance.n_actions()];
  for (index_type k = 0; k < instance.n_actions(); k++) {
    pre_cost[k] = eval(instance.actions[k].pre);
    index_set per_set(instance.actions[k].pre);
    per_set.subtract(instance.actions[k].del);
    per_cost[k] = eval(per_set);
  }
}

LazyPDB::LazyPDB(Instance& i, const index_set& a, Statistics& s)
  : Heuristic(i),
    tab(0),
    atom_set(a),
    r_instance(0),
    stats(s)
{
  r_instance = new Instance(instance.name);
  r_instance->
    restricted_copy(instance, atom_set, EMPTYSET, action_map, atom_map);
  tab = new CostNode(r_instance->n_atoms());
}

LazyPDB::~LazyPDB()
{
  delete tab;
  delete r_instance;
}

NTYPE LazyPDB::lookup(const index_set& s)
{
  CostNode::Value* v = tab->find(s);
  if (v) {
    return v->val;
  }
  NTYPE new_v = solve(s);
  tab->store(s, new_v, false);
  return new_v;
}

NTYPE LazyPDB::eval(const index_set& s)
{
  index_set r;
  for (index_type k = 0; k < s.length(); k++)
    if (atom_map[s[k]] != no_such_index) r.append(atom_map[s[k]]);
  return lookup(r);
}

NTYPE LazyPDB::eval(const bool_vec& s)
{
  index_set r;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (s[k] && (atom_map[k] != no_such_index)) r.append(atom_map[k]);
  return lookup(r);
}

NTYPE LazyPDB::incremental_eval(const index_set& s, index_type i_new)
{
  index_set r;
  for (index_type k = 0; k < s.length(); k++)
    if (atom_map[s[k]] != no_such_index) r.append(atom_map[s[k]]);
  if (atom_map[i_new] != no_such_index) r.insert(atom_map[i_new]);
  return lookup(r);
}

NTYPE LazyPDB::incremental_eval(const bool_vec& s, index_type i_new)
{
  index_set r;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (s[k] && (atom_map[k] != no_such_index)) r.append(atom_map[k]);
  if (atom_map[i_new] != no_such_index) r.insert(atom_map[i_new]);
  return lookup(r);
}

NTYPE LazyPDB::eval_to_bound(const index_set& s, NTYPE bound)
{
  return eval(s);
}

NTYPE LazyPDB::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  return eval(s);
}

void LazyPDB::store(const index_set& s, NTYPE v, bool opt)
{
  index_set r;
  for (index_type k = 0; k < s.length(); k++)
    if (atom_map[s[k]] != no_such_index) r.append(s[k]);
  tab->store(r, v, opt);
}

void LazyPDB::store(const bool_vec& s, NTYPE v, bool opt)
{
  index_set r;
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (s[k] && (atom_map[k] != no_such_index)) r.append(k);
  tab->store(r, v, opt);
}

count_type AH::Hmax_wins = 0;
count_type AH::Hsum_wins = 0;
count_type AH::draws = 0;

AH::AH(Instance& i, Statistics& s)
  : Heuristic(i), Hmax(0), H_vec(0, 0), stats(s)
{
  // done
}

AH::~AH()
{
  if (Hmax) delete Hmax;
  for (index_type k = 0; k < H_vec.length(); k++)
    if (H_vec[k]) delete H_vec[k];
}

void AH::compute_additive_H2(const ACF& cost, const index_set_vec& p)
{
  stats.start();

  for (index_type k = 0; k < p.length(); k++) if (!p[k].empty()) {
    // bool_vec dis(p[k], instance.n_actions());
    // dis.complement();
    if (trace_level > 0) {
      std::cerr << "counting " << p[k].length()
		<< " of " << instance.n_actions() << " actions..."
		<< std::endl;
      if (trace_level > 1) {
	instance.write_action_set(std::cerr, p[k]);
	std::cerr << std::endl;
      }
    }
    // DiscountACF d_cost(cost, dis);
    FracACF d_cost(cost, instance.n_actions(), 0);
    d_cost.set(p[k], 1);
    CostTable* Hd = new CostTable(instance, stats);
    Hd->compute_H2(d_cost);
    H_vec.append(Hd);
  }

  if (trace_level > 0) {
    std::cerr << H_vec.length() << " additive H2 built" << std::endl;
  }

  stats.stop();
}

void AH::compute_fractional_H2(const ACF& cost, const index_set_vec& p)
{
  stats.start();

  index_vec occ(0, instance.n_actions());
  for (index_type k = 0; k < p.length(); k++)
    for (index_type i = 0; i < p[k].length(); i++)
      occ[p[k][i]] += 1;

  for (index_type k = 0; k < p.length(); k++) if (!p[k].empty()) {
    if (trace_level > 0) {
      std::cerr << "counting " << p[k].length()
		<< " of " << instance.n_actions() << " actions..."
		<< std::endl;
      if (trace_level > 1) {
	instance.write_action_set(std::cerr, p[k]);
	std::cerr << std::endl;
      }
    }
    FracACF d_cost(cost, instance.n_actions(), 0);
    for (index_type i = 0; i < p[k].length(); i++)
      d_cost.set(p[k][i], R_TO_N(1, occ[p[k][i]]));
    CostTable* Hd = new CostTable(instance, stats);
    Hd->compute_H2(d_cost);
    H_vec.append(Hd);
  }

  if (trace_level > 0) {
    std::cerr << H_vec.length() << " additive H2 built" << std::endl;
  }

  stats.stop();
}

void AH::compute_additive_H1(const ACF& cost, const index_set_vec& p)
{
  stats.start();

  for (index_type k = 0; k < p.length(); k++) if (!p[k].empty()) {
    bool_vec dis(p[k], instance.n_actions());
    dis.complement();
    if (trace_level > 0) {
      std::cerr << "counting " << p[k].length()
		<< " of " << instance.n_actions() << " actions..."
		<< std::endl;
      if (trace_level > 1) {
	instance.write_action_set(std::cerr, p[k]);
	std::cerr << std::endl;
      }
    }
    DiscountACF d_cost(cost, dis);
    CostTable* Hd = new CostTable(instance, stats);
    Hd->compute_H1(d_cost);
    H_vec.append(Hd);
  }

  if (trace_level > 0) {
    std::cerr << H_vec.length() << " additive H1 built" << std::endl;
  }

  stats.stop();
}

void AH::compute_max_H2(const ACF& cost)
{
  if (Hmax) delete Hmax;
  Hmax = new CostTable(instance, stats);
  Hmax->compute_H2(cost);  
}

void AH::compute_max_H1(const ACF& cost)
{
  if (Hmax) delete Hmax;
  Hmax = new CostTable(instance, stats);
  Hmax->compute_H1(cost);
}

void AH::disable_max_H()
{
  if (Hmax) delete Hmax;
  Hmax = 0;
}

class decreasing_cost : public index_vec::order {
  CostTable* h;
 public:
  decreasing_cost(CostTable* _h) : h(_h) { };
  virtual bool operator()(const index_type& v0, const index_type& v1) const;
};

bool decreasing_cost::operator()
(const index_type& v0, const index_type& v1) const
{
  return (h->eval(v0) >= h->eval(v1));
}

class increasing_cost : public index_vec::order {
  CostTable* h;
 public:
  increasing_cost(CostTable* _h) : h(_h) { };
  virtual bool operator()(const index_type& v0, const index_type& v1) const;
};

bool increasing_cost::operator()
(const index_type& v0, const index_type& v1) const
{
  return (h->eval(v0) <= h->eval(v1));
}

void AH::compute_with_relevance_partitioning
(const ACF& cost, const index_set& g)
{
  stats.start();
  Hmax = new CostTable(instance, stats);
  Hmax->compute_H2(cost);

  Preprocessor* prep = new Preprocessor(instance, stats);
  bool_vec rem(true, instance.n_actions());
  index_type n_rem = instance.n_actions();

  index_vec g_sort;
  g_sort.insert_ordered(g, decreasing_cost(Hmax));

  for (index_type k = 0; (k < g_sort.length()) && (rem.count(true) > 0); k++) {
    bool_vec arg(false, instance.n_actions()); // Actions Relevant to Goal
    prep->strictly_relevant_actions(g_sort[k], Hmax->eval(g_sort[k]),
				    *Hmax, cost, arg);
    arg.intersect(rem);
    if (arg.count(true) > 0) {
      if (trace_level > 0) {
	std::cerr << "goal " << instance.atoms[g_sort[k]].name
		  << ": counting " << arg.count(true)
		  << " of " << instance.n_actions() << " actions..."
		  << std::endl;
	if (trace_level > 1) {
	  instance.write_action_set(std::cerr, arg);
	  std::cerr << std::endl;
	}
      }
      bool_vec d(arg);
      d.complement();
      DiscountACF d_cost(cost, d);
      CostTable* Hd = new CostTable(instance, stats);
      Hd->compute_H2(d_cost);
      H_vec.append(Hd);
      rem.subtract(arg);
    }
  }

  if (rem.count(true) > 0) {
    if (trace_level > 0) {
      std::cerr << "remaining: " << rem.count(true)
		<< " of " << instance.n_actions() << " actions..."
		<< std::endl;
      if (trace_level > 1) {
	instance.write_action_set(std::cerr, rem);
	std::cerr << std::endl;
      }
    }
    bool_vec d(rem);
    d.complement();
    DiscountACF d_cost(cost, d);
    CostTable* Hd = new CostTable(instance, stats);
    Hd->compute_H2(d_cost);
    H_vec.append(Hd);
  }

  if (trace_level > 0) {
    std::cerr << H_vec.length() << " additive H2 built" << std::endl;
  }

  delete prep;
  stats.stop();
}

void AH::compute_with_random_relevance_partitioning
(const ACF& cost, const index_set& g, RNG& rnd, bool use_H2)
{
  stats.start();

  Hmax = new CostTable(instance, stats);
  if (use_H2) {
    Hmax->compute_H2(cost);
  }
  else {
    Hmax->compute_H1(cost);
  }

  index_set_vec p; // action partitions
  p.assign_value(EMPTYSET, g.length());

  Preprocessor* prep = new Preprocessor(instance, stats);

  index_set_vec r; // relevant actions
  r.assign_value(EMPTYSET, g.length());

  NTYPE offset = 0;
  bool_vec rem(true, instance.n_actions());
  bool conv = false;

  while (!conv && (rem.count(true) > 0)) {
    conv = true;
    if (trace_level > 0) {
      std::cerr << "checking relevance at H() + " << offset << "..."
		<< std::endl;
    }

    for (index_type k = 0; k < g.length(); k++) {
      bool_vec rel(false, instance.n_actions());
      prep->strictly_relevant_actions(g[k], Hmax->eval(g[k]) + offset,
				      *Hmax, cost, rel);
      rel.copy_to(r[k]);
      if (trace_level > 0) {
	std::cerr << r[k].length() << " actions relevant to "
		  << instance.atoms[g[k]].name << " at H() + " << offset
		  << std::endl;
      }
    }

    for (index_type k = 0; k < instance.n_actions(); k++) if (rem[k]) {
      index_set s;
      for (index_type i = 0; i < g.length(); i++)
	if (r[i].contains(k)) s.insert(i);
      if (s.length() == 1) {
	p[s[0]].insert(k);
	if (trace_level > 0) {
	  std::cerr << "action " << instance.actions[k].name
		    << " relevant to single goal: assigned to "
		    << instance.atoms[g[s[0]]].name << " (" << s[0] << ")"
		    << std::endl;
	}
	rem[k] = false;
	conv = false;
      }
      else if (s.length() > 1) {
	index_type l = (rnd.random() % s.length());
	p[s[l]].insert(k);
	if (trace_level > 0) {
	  std::cerr << "action " << instance.actions[k].name
		    << " relevant to " << s.length() << " goals: assigned to "
		    << instance.atoms[g[s[l]]].name << " (" << s[l] << ")"
		    << std::endl;
	}
	rem[k] = false;
	conv = false;
      }
    }

    offset += 1;
    if (trace_level > 0) {
      std::cerr << rem.count(true) << " actions remaining" << std::endl;
    }
  }

//   for (index_type k = 0; k < instance.n_actions(); k++) if (rem[k]) {
//     index_type l = (rnd.random() % g.length());
//     p[l].insert(k);
//     std::cerr << "action " << instance.actions[k].name
// 	      << " not relevant to any goal: assigned to "
// 	      << instance.atoms[g[l]].name << " (" << l << ")"
// 	      << std::endl;
//   }

  if (use_H2) {
    compute_additive_H2(cost, p);
  }
  else {
    compute_additive_H1(cost, p);
  }

  delete prep;
  stats.stop();
}

void AH::compute_with_iterative_assignment
(const ACF& cost, const index_set& g,
 bool use_H2, bool fractional, bool optimal)
{
  stats.start();

  std::cerr << "computing H1..." << std::endl;
  Hmax = new CostTable(instance, stats);
  Hmax->compute_H1(cost);

  index_set_vec p; // action partition
  p.assign_value(EMPTYSET, g.length());
  bool_vec rem(true, instance.n_actions());

  index_set v;
  for (index_type k = 0; k < instance.n_invariants(); k++)
    if ((instance.invariants[k].lim == 1) && instance.invariants[k].exact)
      v.insert(k);
  if (v.empty()) {
    std::cerr << "warning: no invariants usable for action partitioning"
	      << std::endl;
  }
  else {
    std::cerr << v.length() << " variables in problem..." << std::endl;

    std::cerr << "associating actions to variables..." << std::endl;
    index_set_vec a(v.length());
    for (index_type k = 0; k < instance.n_actions(); k++) {
      index_set e(instance.actions[k].add);
      e.insert(instance.actions[k].del);
      for (index_type i = 0; i < v.length(); i++)
	if (instance.invariants[v[i]].set.count_common(e) > 0)
	  a[i].insert(k);
    }

    std::cerr << "building graph of conflicts..." << std::endl;
    graph c(v.length());
    for (index_type i = 0; i < v.length(); i++)
      for (index_type j = i+1; j < v.length(); j++)
	if (a[i].count_common(a[j]))
	  c.add_undirected_edge(i, j);

    index_set s;
    if (optimal) {
      std::cerr << "searching for maximal conflict-free set..." << std::endl;
      c.complement();
      c.maximal_clique(s);
    }
    else {
      std::cerr << "searching for conflict-free set..." << std::endl;
      c.apx_independent_set(s);
    }
    std::cerr << s.length() << " variables in maximal additive set"
	      << std::endl;

    for (index_type k = 0; k < s.length(); k++) {
      if (trace_level > 0) {
	std::cerr << "classifying action set " << s[k]
		  << " (" << a[s[k]].length() << " actions)..."
		  << std::endl;
      }
      cost_vec l(0, g.length());
      for (index_type i = 0; i < g.length(); i++) {
	DiscountACF d_cost(cost, instance.n_actions());
	d_cost.discount(a[s[k]]);
	CostTable* Hd = new CostTable(instance, stats);
	Hd->compute_H1(d_cost);
	l[i] = (Hmax->eval(g[i]) - Hd->eval(g[i]));
	if (trace_level > 0) {
	  std::cerr << "loss for goal " << instance.atoms[g[i]].name
		    << " = " << l[i] << std::endl;
	}
	delete Hd;
      }
      if (fractional) {
	for (index_type i = 0; i < g.length(); i++)
	  if (l[i] > 0) {
	    if (trace_level > 0) {
	      std::cerr << "action set " << s[k] << " assigned to goal "
			<< instance.atoms[g[i]].name << std::endl;
	    }
	    p[i].insert(a[s[k]]);
	  }
      }
      else {
	index_type mi = l.arg_max();
	assert(mi != no_such_index);
	index_type mn = p[mi].length();
	for (index_type i = mi + 1; i < g.length(); i++)
	  if (l[i] == l[mi])
	    if (p[i].length() < mn) {
	      mi = i;
	      mn = p[i].length();
	    }
	assert(mi != no_such_index);
	if (trace_level > 0) {
	  std::cerr << "action set " << s[k] << " assigned to goal "
		    << instance.atoms[g[mi]].name << std::endl;
	}
	p[mi].insert(a[s[k]]);
      }
      rem.subtract(a[s[k]]);
    }
  }

  index_set_vec p_new;
  p_new.assign_value(EMPTYSET, g.length());
  bool done = (rem.count(true) == 0);
  while (!done) {
    std::cerr << rem.count(true) << " actions remaining..." << std::endl;
    done = true;
    for (index_type k = 0; k < instance.n_actions(); k++) if (rem[k]) {
      bool assigned = false;
      for (index_type i = 0; (i < g.length()) && !assigned; i++) {
	DiscountACF d_cost(cost, instance.n_actions());
	for (index_type j = 0; j < g.length(); j++)
	  if (j != i) d_cost.discount(p[j]);
	d_cost.discount(k);
	CostTable* Hd = new CostTable(instance, stats);
	Hd->compute_H1(d_cost);
	if (Hd->eval(g[i]) < Hmax->eval(g[i])) {
	  p_new[i].insert(k);
	  rem[k] = false;
	  if (!fractional)
	    assigned = true;
	  done = false;
	}
	delete Hd;
      }
    }
    for (index_type i = 0; i < g.length(); i++) {
      p[i].insert(p_new[i]);
      p_new[i].clear();
    }
  }

  if (use_H2) {
    if (fractional) {
      compute_fractional_H2(cost, p);
    }
    else {
      compute_additive_H2(cost, p);
    }
    compute_max_H2(cost);
  }
  else {
    compute_additive_H1(cost, p);
  }
  stats.stop();
}


void AH::compute_with_new_decomposition
(const ACF& cost, const index_set& g)
{
  stats.start();

  index_set_graph cg;
  bool_vec  a(true, instance.n_atoms());
  for (index_type k = 0; k < instance.n_invariants(); k++)
    if (instance.invariants[k].lim == 1) {
      cg.add_node();
      cg.node_label(cg.size() - 1).assign_copy(instance.invariants[k].set);
      a.subtract(instance.invariants[k].set);
    }
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (a[k]) {
      cg.add_node();
      cg.node_label(cg.size() - 1).assign_singleton(k);
    }
  std::cerr << cg.size() << " state variables" << std::endl;

  for (index_type k = 0; k < instance.n_actions(); k++) {
    index_set ps;
    for (index_type i = 0; i < cg.size(); i++)
      if (instance.actions[k].pre.have_common_element(cg.node_label(i)))
	ps.insert(i);
    for (index_type i = 0; i < cg.size(); i++)
      if (instance.actions[k].add.have_common_element(cg.node_label(i)))
	cg.add_edge(ps, i);
  }

  // cg.write_digraph(std::cerr, "CG");

  index_set_graph ccg(cg);
  ccg.merge_labels_downwards();

  // ccg.write_digraph(std::cerr, "CCG");

  index_set gv;
  for (index_type k = 0; k < cg.size(); k++)
    if (g.have_common_element(cg.node_label(k)))
      gv.insert(k);

  // std::cerr << "gv = " << gv << std::endl;

  index_set_vec p(EMPTYSET, gv.length());
  for (index_type k = 0; k < instance.n_actions(); k++) {
    //bool_vec aff(false, gv.length());
    //for (index_type i = 0; i < gv.length(); i++)
    //  if (instance.actions[k].add.have_common_element(cg.node_label(gv[i])))
    //aff[i] = true;
    //if (aff.count(true)) {
    //  for (index_type i = 0; i < gv.length(); i++)
    //    if (aff[i]) p[i].insert(k);
    //}
    //else {
    for (index_type i = 0; i < gv.length(); i++)
      if (instance.actions[k].add.have_common_element(ccg.node_label(gv[i])))
	p[i].insert(k);
    //}
  }

  if (trace_level > 0) {
    for (index_type k = 0; k < gv.length(); k++) {
      std::cerr << "goal variable #" << k + 1 << ": ";
      instance.write_atom_set(std::cerr, cg.node_label(gv[k]));
      std::cerr << std::endl << " actions: ";
      instance.write_action_set(std::cerr, p[k]);
      std::cerr << std::endl;
    }
  }

  compute_fractional_H2(cost, p);
  stats.stop();
}


void AH::compute_bottom_up
(const ACF& cost, const index_set& g)
{
  stats.start();

  index_set_graph cg;
  bool_vec  rem(true, instance.n_atoms());
  for (index_type k = 0; k < instance.n_invariants(); k++)
    if (instance.invariants[k].lim == 1) {
      cg.add_node();
      cg.node_label(cg.size() - 1).assign_copy(instance.invariants[k].set);
      rem.subtract(instance.invariants[k].set);
    }
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (rem[k]) {
      cg.add_node();
      cg.node_label(cg.size() - 1).assign_singleton(k);
    }
  std::cerr << cg.size() << " state variables" << std::endl;

  for (index_type k = 0; k < instance.n_actions(); k++) {
    index_set ps;
    for (index_type i = 0; i < cg.size(); i++)
      if (instance.actions[k].pre.have_common_element(cg.node_label(i)))
	ps.insert(i);
    for (index_type i = 0; i < cg.size(); i++)
      if (instance.actions[k].add.have_common_element(cg.node_label(i)))
	cg.add_edge(ps, i);
  }

  index_set_vec p_atms(EMPTYSET, 0);

  rem.assign_value(true, instance.n_atoms());
  for (index_type k = 0; k < cg.size(); k++)
    if (cg.out_degree(k) == 0) {
      p_atms.append(cg.node_label(k));
      rem.subtract(cg.node_label(k));
    }
  for (index_type k = 0; k < instance.n_atoms(); k++)
    if (rem[k])
      p_atms.append().assign_singleton(k);

  std::cerr << p_atms.length() << " initial partitions" << std::endl;

  index_set_vec p_acts(EMPTYSET, p_atms.length());

  for (index_type k = 0; k < p_atms.length(); k++)
    for (index_type i = 0; i < p_atms[k].length(); i++)
      p_acts[k].insert(instance.atoms[p_atms[k][i]].add_by);

  compute_fractional_H2(cost, p_acts);
  stats.stop();
}


void AH::compute_with_layered_partitioning
(const ACF& cost, const index_set& g)
{
  stats.start();

  std::cerr << "computing H2..." << std::endl;
  Hmax = new CostTable(instance, stats);
  Hmax->compute_H2(cost);

  index_set_vec p; // action partition
  bool_vec rem(true, instance.n_actions());

  std::cerr << "computing layered action partition..." << std::endl;
  compute_layered_action_partition(2, g, p, no_such_index, rem);

  std::cerr << "computing additive H2 ("
	    << p.length() << " components)..."
	    << std::endl;
  compute_additive_H2(cost, p);
  stats.stop();
}

void AH::max_cost_mset
(index_type m, const index_set& g, index_set& s)
{
  assert(Hmax); // Hmax must be computed/set

  NTYPE v_max = NEG_INF;
  mSubsetEnumerator se(g.length(), m);
  bool more = se.first();
  while (more) {
    index_set cs;
    se.current_set(g, cs);
    if (Hmax->eval(cs) > v_max) {
      v_max = Hmax->eval(cs);
      s.assign_copy(cs);
    }
    more = se.next();
  }
}

void AH::goal_relevant_remaining_actions
(const index_set& g, index_set& a, bool_vec& rem)
{
  a.clear();
  for (index_type k = 0; k < instance.n_actions(); k++) if (rem[k]) {
    Instance::Action& act = instance.actions[k];
    if ((act.add.count_common(g) > 0) && (act.del.count_common(g) == 0))
      a.insert(k);
  }
}

void AH::compute_layered_action_partition
(index_type m, const index_set& g, index_set_vec& p, index_type pg,
 bool_vec& rem)
{
  // split g into size m partitions
  std::cerr << "splitting " << g << "..." << std::endl;
  index_set g0(g);
  index_set_vec sub_g;
  while (!g0.empty()) {
    index_set next_p;
    max_cost_mset(m, g0, next_p);
    sub_g.append(next_p);
    g0.subtract(next_p);
  }
  std::cerr << "result: " << sub_g << std::endl;
  // assign immediately relevant actions to each goal partition
  index_vec sub_p(no_such_index, sub_g.length());
  index_set_vec new_g(sub_g.length());
  for (index_type k = 0; k < sub_g.length(); k++) {
    index_set rra;
    goal_relevant_remaining_actions(sub_g[k], rra, rem);
    std::cerr << "goal subset " << k << " = " << sub_g[k]
	      << ", rra = " << rra << std::endl;
    if (!rra.empty()) {
      // if pg != no_such_index, assign rra to p[pg]
      if (pg != no_such_index) {
	p[pg].insert(rra);
	sub_p[k] = pg;
	pg = no_such_index;
      }
      // otherwise, create a new partition
      else {
	p.inc_length();
	p[p.length() - 1].insert(rra);
	sub_p[k] = p.length() - 1;
      }
      // remove rra's from rem, and add precs to new_g
      for (index_type i = 0; i < rra.length(); i++) {
	rem[rra[i]] = false;
	new_g[k].insert(instance.actions[rra[i]].pre);
      }
    }
  }
  std::cerr << "action partitions: " << p << std::endl;
  // recurse with collected preconditions of assigned actions
  std::cerr << "new_g = " << new_g << std::endl;
  for (index_type k = 0; k < sub_g.length(); k++) if (!new_g[k].empty()) {
    compute_layered_action_partition(m, new_g[k], p, sub_p[k], rem);
  }
}

NTYPE AH::eval(const index_set& s)
{
  NTYPE v_max = (Hmax ? Hmax->eval(s) : 0);
  NTYPE v_sum = 0;
  if (trace_level > 2) {
    std::cerr << "AH: v_max = " << v_max << ", v_sum = ";
  }
  for (index_type k = 0; k < H_vec.length(); k++) {
    assert(H_vec[k]);
    v_sum += H_vec[k]->eval(s);
    if (trace_level > 2) {
      std::cerr << " + " << H_vec[k]->eval(s);
    }
  }
  if (trace_level > 2) {
    std::cerr << " = " << v_sum << std::endl;
  }
#ifdef PRINT_EXTRA_STATS
  std::cerr << "AH: " << v_max << " " << v_sum << std::endl;
#endif
#ifdef AH_EXTRA_STATS
  if (v_sum > v_max) {
    Hsum_wins += 1;
    return v_sum;
  }
  else if (v_max > v_sum) {
    Hmax_wins += 1;
    return v_max;
  }
  else {
    draws += 1;
    return v_max;
  }
#else
  return MAX(v_max, v_sum);
#endif
}

NTYPE AH::eval(const bool_vec& s)
{
  NTYPE v_max = (Hmax ? Hmax->eval(s) : 0);
  NTYPE v_sum = 0;
  if (trace_level > 2) {
    std::cerr << "AH: v_max = " << v_max << ", v_sum = ";
  }
  for (index_type k = 0; k < H_vec.length(); k++) {
    assert(H_vec[k]);
    v_sum += H_vec[k]->eval(s);
    if (trace_level > 2) {
      std::cerr << " + " << H_vec[k]->eval(s);
    }
  }
  if (trace_level > 2) {
    std::cerr << " = " << v_sum << std::endl;
  }
#ifdef PRINT_EXTRA_STATS
  std::cerr << "AH: " << v_max << " " << v_sum << std::endl;
#endif
#ifdef AH_EXTRA_STATS
  if (v_sum > v_max) {
    Hsum_wins += 1;
    return v_sum;
  }
  else if (v_max > v_sum) {
    Hmax_wins += 1;
    return v_max;
  }
  else {
    draws += 1;
    return v_max;
  }
#else
  return MAX(v_max, v_sum);
#endif
}

void AH::write_eval(const index_set& s, std::ostream& st, char* p, bool e)
{
  NTYPE v_max = (Hmax ? Hmax->eval(s) : 0);
  NTYPE v_sum = 0;
  if (p) st << p << " (";
  for (index_type k = 0; k < H_vec.length(); k++) {
    assert(H_vec[k]);
    v_sum += H_vec[k]->eval(s);
    if (k > 0) st << " ";
    st << H_vec[k]->eval(s);
  }
  st << ") " << v_sum << " " << v_max;
  if (e) st << std::endl;
}

void AH::write_eval(const bool_vec& s, std::ostream& st, char* p, bool e)
{
  NTYPE v_max = (Hmax ? Hmax->eval(s) : 0);
  NTYPE v_sum = 0;
  if (p) st << p << " (";
  for (index_type k = 0; k < H_vec.length(); k++) {
    assert(H_vec[k]);
    v_sum += H_vec[k]->eval(s);
    if (k > 0) st << " ";
    st << H_vec[k]->eval(s);
  }
  st << ") " << v_sum << " " << v_max;
  if (e) st << std::endl;
}

NTYPE AH::incremental_eval(const index_set& s, index_type i_new)
{
  NTYPE v_max = (Hmax ? Hmax->incremental_eval(s, i_new) : 0);
  NTYPE v_sum = 0;
  for (index_type k = 0; k < H_vec.length(); k++) {
    assert(H_vec[k]);
    v_sum += H_vec[k]->incremental_eval(s, i_new);
  }
#ifdef AH_EXTRA_STATS
  if (v_sum > v_max) {
    Hsum_wins += 1;
    return v_sum;
  }
  else if (v_max > v_sum) {
    Hmax_wins += 1;
    return v_max;
  }
  else {
    draws += 1;
    return v_max;
  }
#else
  return MAX(v_max, v_sum);
#endif
}

NTYPE AH::incremental_eval(const bool_vec& s, index_type i_new)
{
  NTYPE v_max = (Hmax ? Hmax->incremental_eval(s, i_new) : 0);
  NTYPE v_sum = 0;
  for (index_type k = 0; k < H_vec.length(); k++) {
    assert(H_vec[k]);
    v_sum += H_vec[k]->incremental_eval(s, i_new);
  }
#ifdef AH_EXTRA_STATS
  if (v_sum > v_max) {
    Hsum_wins += 1;
    return v_sum;
  }
  else if (v_max > v_sum) {
    Hmax_wins += 1;
    return v_max;
  }
  else {
    draws += 1;
    return v_max;
  }
#else
  return MAX(v_max, v_sum);
#endif
}

NTYPE AH::eval_to_bound(const index_set& s, NTYPE bound)
{
  NTYPE v_max = (Hmax ? Hmax->eval_to_bound(s, bound) : 0);
  NTYPE v_sum = 0;
  for (index_type k = 0; (k < H_vec.length()) && (v_sum < bound); k++) {
    assert(H_vec[k]);
    v_sum += H_vec[k]->eval(s);
  }
#ifdef AH_EXTRA_STATS
  if (v_sum > v_max) {
    Hsum_wins += 1;
    return v_sum;
  }
  else if (v_max > v_sum) {
    Hmax_wins += 1;
    return v_max;
  }
  else {
    draws += 1;
    return v_max;
  }
#else
  return MAX(v_max, v_sum);
#endif
}

NTYPE AH::eval_to_bound(const bool_vec& s, NTYPE bound)
{
  NTYPE v_max = (Hmax ? Hmax->eval_to_bound(s, bound) : 0);
  NTYPE v_sum = 0;
  for (index_type k = 0; (k < H_vec.length()) && (v_sum < bound); k++) {
    assert(H_vec[k]);
    v_sum += H_vec[k]->eval(s);
  }
#ifdef AH_EXTRA_STATS
  if (v_sum > v_max) {
    Hsum_wins += 1;
    return v_sum;
  }
  else if (v_max > v_sum) {
    Hmax_wins += 1;
    return v_max;
  }
  else {
    draws += 1;
    return v_max;
  }
#else
  return MAX(v_max, v_sum);
#endif
}

void AH::store(const index_set& s, NTYPE v, bool opt)
{
  if (Hmax) Hmax->store(s, v, opt);
}

void AH::store(const bool_vec& s, NTYPE v, bool opt)
{
  if (Hmax) Hmax->store(s, v, opt);
}

ForwardH1::ForwardH1
(Instance& i, const index_set& g, const ACF& c, Statistics& s)
  : Heuristic(i), goals(g), cost(c), table(0)
{
  table = new CostTable(instance, s);
}

ForwardH1::~ForwardH1()
{
  delete table;
}

NTYPE ForwardH1::eval(const index_set& s)
{
  bool_vec s1(s, instance.n_atoms());
  table->compute_H1(cost, s1);
  return table->eval(goals);
}

NTYPE ForwardH1::eval(const bool_vec& s)
{
  table->compute_H1(cost, s);
  return table->eval(goals);
}

ForwardH2::ForwardH2
(Instance& i, const index_set& g, const ACF& c, Statistics& s)
  : Heuristic(i), goals(g), cost(c), table(0)
{
  table = new CostTable(instance, s);
}

ForwardH2::~ForwardH2()
{
  delete table;
}

NTYPE ForwardH2::eval(const index_set& s)
{
  bool_vec s1(s, instance.n_atoms());
  table->compute_H2(cost, s1);
  return table->eval(goals);
}

NTYPE ForwardH2::eval(const bool_vec& s)
{
  table->compute_H2(cost, s);
  return table->eval(goals);
}

END_HSPS_NAMESPACE
