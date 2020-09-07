
#include "mdd.h"

BEGIN_HSPS_NAMESPACE

MDDNode* MDDNode::terminal = (MDDNode*)0xFFFFFFFF;

static lvector<const MDDNode*> p_stack(0, 0);
static index_vec i_stack(no_such_index, 0);

MDDNode::MDDNode()
  : node_type(var_node), next(0, 0)
{
  // done
}

MDDNode::~MDDNode()
{
  for (index_type i = 0; i < next.length(); i++)
    if ((next[i] != 0) && (next[i] != terminal))
      delete next[i];
}

bool MDDNode::recursive_lookup(const partial_state& s, index_type i) const
{
  assert(node_type == var_node);
  while (i < s.length()) {
    index_type var = s[i].first;
    index_type val = s[i].second;
    // std::cerr << "lookup VAR @ " << this
    //	      << ": s = " << s
    //	      << ", i = " << i
    //	      << ", next = " << next
    //	      << std::endl;
    if (next.length() <= var) return false;
    else if (next[var] == terminal) return true;
    else if (next[var]) {
      assert(next[var]->node_type == val_node);
      // std::cerr << "lookup VAL @ " << next[var]
      // 	<< ", next = " << next[var]->next
      // 	<< std::endl;
      if (next[var]->next.length() > val) {
	if (next[var]->next[val] == terminal) return true;
	else if (next[var]->next[val] && ((i + 1) < s.length())) {
	  bool hit = next[var]->next[val]->recursive_lookup(s, i + 1);
	  if (hit) return true;
	}
      }
    }
    i += 1;
  }
  return false;
}

// bool MDDNode::lookup(const partial_state& s) const
// {
//   assert(p_stack.length() == 0);
//   assert(i_stack.length() == 0);
//   p_stack.append(this);
//   i_stack.append(0);
//   while (p_stack.length() > 0) {
//     const MDDNode* p = p_stack[p_stack.length() - 1];
//     p_stack.dec_length();
//     index_type i = i_stack[i_stack.length() - 1];
//     i_stack.dec_length();
//     while (i < s.length()) {
//       index_type var = s[i].first;
//       index_type val = s[i].second;
//       if (p->next[var] == terminal) {
// 	p_stack.clear();
// 	i_stack.clear();
// 	return true;
//       }
//       else if (p->next[var]) {
// 	if (p->next[var]->next[val] == terminal) {
// 	  p_stack.clear();
// 	  i_stack.clear();
// 	  return true;
// 	}
// 	else if (p->next[var]->next[val] && ((i + 1) < s.length())) {
// 	  i_stack.append(i + 1);
// 	  p_stack.append(p);
// 	  p = next[var]->next[val];
// 	  i = i + 1;
// 	}
// 	else i += 1;
//       }
//       else i += 1;
//     }
//   }
//   return false;
// }

void MDDNode::insert(const partial_state& s)
{
  assert(s.length() > 0);
  MDDNode* p = this;
  for (index_type i = 0; i < s.length(); i++) {
    assert(p->node_type == var_node);
    index_type var = s[i].first;
    index_type val = s[i].second;
    p->next.inc_length_to(var + 1, 0);
    if (p->next[var] == terminal) return;
    if (p->next[var] == 0) {
      p->next[var] = new MDDNode();
      p->next[var]->node_type = val_node;
    }
    assert(p->next[var]->node_type == val_node);
    p->next[var]->next.inc_length_to(val + 1, 0);
    if (p->next[var]->next[val] == terminal) return;
    if (i == (s.length() - 1)) {
      if (p->next[var]->next[val] != 0) {
	delete p->next[var]->next[val];
      }
      p->next[var]->next[val] = terminal;
    }
    else {
      if (p->next[var]->next[val] == 0) {
	p->next[var]->next[val] = new MDDNode();
      }
      p = p->next[var]->next[val];
    }
  }
}

void MDDNode::write_graph(std::ostream& s, bool root) const
{
  if (root) {
    s << "digraph MDD {" << std::endl;
  }
  if (node_type == var_node) {
    s << "N" << this << " [shape=box,label=";
    if (root)
      s << "\"root\"];" << std::endl;
    else
      s << "\"\"];" << std::endl;
  }
  else {
    s << "N" << this << " [shape=circle,label=";
    if (root)
      s << "\"root\"];" << std::endl;
    else
      s << "\"\"];" << std::endl;
  }
  for (index_type i = 0; i < next.length(); i++)
    if ((next[i] != 0) && (next[i] != terminal)) {
      next[i]->write_graph(s, false);
    }
  for (index_type i = 0; i < next.length(); i++)
    if (next[i] == terminal) {
      s << "T" << this << "_" << i << " [shape=point];" << std::endl;
      s << "N" << this << " -> T" << this << "_" << i
	<< "[label=\"" << i << "\"];" << std::endl;
    }
    else if (next[i] != 0) {
      s << "N" << this << " -> N" << next[i]
	<< " [label=\"" << i << "\"];" << std::endl;
    }
  if (root) {
    s << "}" << std::endl;
  }
}

void insert_states
(index_vec& p,
 index_type i,
 partial_state& s,
 MDDNode* m,
 const partial_state* map,
 index_type mapl)
{
  assert(i < p.length());
  assert(p[i] < mapl);
  for (index_type j = 0; j < map[p[i]].length(); j++)
    if (!s.defines(map[p[i]][j].first)) {
      partial_state s1(s);
      s1.insert(map[p[i]][j]);
      if ((i + 1) < p.length()) {
	insert_states(p, i + 1, s1, m, map, mapl);
      }
      else {
#ifdef TRACE_PRINT_LOTS
	std::cerr << "insert_states: inserting " << s1 << std::endl;
#endif
	m->insert(s1);
#ifdef TRACE_PRINT_LOTS
	std::cerr << "insert_states: new graph = " << std::endl;
	m->write_graph(std::cerr);
#endif
      }
    }
}

void insert_states
(CostNode* n,
 index_vec& p,
 MDDNode* m,
 const partial_state* map,
 index_type mapl)
{
  assert(n != 0);
  for (index_type i = n->first(); i < n->size(); i++) {
    p.append(i);
    const CostNode::Value& v = n->val(i);
    if (v.val == POS_INF) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "insert_states: found " << p << std::endl;
#endif
      partial_state s;
      insert_states(p, 0, s, m, map, mapl);
    }
    else if (v.next != 0) {
      insert_states(v.next, p, m, map, mapl);
    }
    p.dec_length();
  }
}

MDDNode* makeMDD(CostNode* n, const partial_state* map, index_type mapl)
{
  MDDNode* root = new MDDNode();
  index_vec p(no_such_index, 0);
  insert_states(n, p, root, map, mapl);
  return root;
}

NTYPE ConsistencyPostCheck::eval
(const partial_state& s)
{
  NTYPE v = h_base.eval(s);
  if (FINITE(v)) {
    if (sinc->lookup(s)) return POS_INF;
  }
  return v;
}

NTYPE ConsistencyPostCheck::eval_to_bound
(const partial_state& s, NTYPE bound)
{
  NTYPE v = h_base.eval_to_bound(s, bound);
  if (v >= bound) return v;
  if (FINITE(v)) {
    if (sinc->lookup(s)) return POS_INF;
  }
  return v;
}

END_HSPS_NAMESPACE
