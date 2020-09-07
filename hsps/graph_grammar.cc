
#include "graph_grammar.h"
#include <sstream>

// #define TRACE_PRINT_GRAPH_DERIVATION

BEGIN_HSPS_NAMESPACE

bool GraphGrammarRule::applicable(const index_graph& g, index_type n) const
{
  assert(n < g.size());
  if (!g.node_has_label(n))
    return false;
  return (g.node_label(n) == head);
}

void GraphGrammarRule::applicable(const index_graph& g, bool_vec& nn) const
{
  nn.assign_value(false, g.size());
  for (index_type k = 0; k < g.size(); k++)
    nn[k] = applicable(g, k);
}

bool TerminalRule::matches(const index_graph& g, index_type n) const
{
  if (!g.node_has_label(n))
    return false;
  if (g.node_label(n) != terminal)
    return false;

  if ((g.in_degree(n) > 0) && !c_in)
    return false;
  if ((g.out_degree(n) > 0) && !c_out)
    return false;
  return true;
}

void TerminalRule::apply(index_graph& g, index_type n) const
{
  assert(n < g.size());
  assert(g.node_has_label(n));
  assert(g.node_label(n) == head);

  // change node label
  g.node_label(n) = terminal;

  // edge redirection not implemented - all edges remain
}

std::pair<bool,bool> NonTerminalRule::matches
(const index_graph& g, const partial_match& m1, const partial_match& m2) const
{
#ifdef TRACE_PRINT_LOTS
  std::cerr << "checking rule " << *this << " against "
 	    << m1 << " + " << m2 << "...";
#endif

  // check that non-terminal labels match
  if ((m1.first != n1) || (m2.first != n2)) {
    // if they don't, they may match the other way around
    if ((m2.first == n1) && (m1.first == n2)) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << " reversing: ";
#endif
      std::pair<bool,bool> ok = matches(g, m2, m1);
      return std::pair<bool,bool>(ok.first, true);
    }
    else {
#ifdef TRACE_PRINT_LOTS
      std::cerr << " failed (wrong non-terminals)" << std::endl;
#endif
      return std::pair<bool,bool>(false, false);
    }
  }

  // check that matched parts of the graph are disjoint
  if (m1.second.have_common_element(m2.second)) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << " failed (not disjoint)" << std::endl;
#endif
    return std::pair<bool,bool>(false, false);
  }

  // check that edges between the two match sets conform with the rule
  bool have_12 = g.adjacent(m1.second, m2.second);
  bool have_21 = g.adjacent(m2.second, m1.second);
  if (have_12 && !e_12) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << " failed (bad int. edge 1->2)" << std::endl;
#endif
    return std::pair<bool,bool>(false, false);
  }
  if (have_21 && !e_21) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << " failed (bad int. edge 2->1)" << std::endl;
#endif
    return std::pair<bool,bool>(false, false);
  }
  if (e_12 && !have_12) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << " failed (miss. int. edge 1->2)" << std::endl;
#endif
    return std::pair<bool,bool>(false, false);
  }
  if (e_21 && !have_21) {
#ifdef TRACE_PRINT_LOTS
    std::cerr << " failed (miss. int. edge 2->1)" << std::endl;
#endif
    return std::pair<bool,bool>(false, false);
  }

  // check contexts
  for (index_type i = 0; i < g.size(); i++)
    if (!m1.second.contains(i) && !m2.second.contains(i)) {
      if (g.adjacent(i, m1.second) && !c_in_1)
	return std::pair<bool,bool>(false, false);
      else if (g.adjacent(m1.second, i) && !c_out_1)
	return std::pair<bool,bool>(false, false);
      else if (g.adjacent(i, m2.second) && !c_in_2)
	return std::pair<bool,bool>(false, false);
      else if (g.adjacent(m2.second, i) && !c_out_2)
	return std::pair<bool,bool>(false, false);
    }

#ifdef TRACE_PRINT_LOTS
  std::cerr << " ok!" << std::endl;
#endif
  return std::pair<bool,bool>(true, false);
}

#define EDGE_DIR_CONST 512

void NonTerminalRule::apply(index_graph& g, index_type n) const
{
  assert(n < g.size());
  assert(g.node_has_label(n));
  assert(g.node_label(n) == head);

  index_set pred_n(g.predecessors(n));
  index_set succ_n(g.successors(n));
  g.remove_edges_incident_on(n);

  // add new node and set node labels
  index_type i1 = n;
  index_type i2 = g.add_node();

  g.node_label(i1) = n1;
  g.node_label(i2) = n2;
  if (e_12)
    g.add_edge(i1, i2, EDGE_DIR_CONST);
  if (e_21)
    g.add_edge(i2, i1, EDGE_DIR_CONST);

  for (index_type i = 0; i < pred_n.length(); i++) {
    if (c_in_1) g.add_edge(pred_n[i], i1, EDGE_DIR_CONST);
    if (c_in_2) g.add_edge(pred_n[i], i2, EDGE_DIR_CONST);
  }

  for (index_type i = 0; i < succ_n.length(); i++) {
    if (c_out_1) g.add_edge(i1, succ_n[i], EDGE_DIR_CONST);
    if (c_out_2) g.add_edge(i2, succ_n[i], EDGE_DIR_CONST);
  }
}

GraphParser::GraphParser()
{
  // done
}

GraphParser::~GraphParser()
{
  // done
}

void GraphParser::define_non_terminal
(index_type label, const std::string& name)
{
  assert(!non_terminal_labels.contains(label));
  non_terminal_labels.append(label);
  non_terminal_names.append(name);
  nt_rules.append(EMPTYSET);
  t_rules.append(EMPTYSET);
  assert(non_terminal_labels.length() == non_terminal_names.length());
  assert(non_terminal_labels.length() == nt_rules.length());
  assert(non_terminal_labels.length() == t_rules.length());
}

index_type GraphParser::new_non_terminal
(const std::string& basename)
{
  std::string n;
  index_type c = 1;
  bool ok = false;
  while (!ok) {
    std::ostringstream s;
    s << basename << "IN" << c++;
    n = s.str();
    if (non_terminal_names.first(n) == no_such_index)
      ok = true;
  }
  index_type l = ((index_vec_util::max(non_terminal_labels, 0) / index_graph::STYLE_MAX) + 1) * index_graph::STYLE_MAX;
  assert(!non_terminal_labels.contains(l));
  define_non_terminal(l, n);
  return l;
}

void GraphParser::add_rule_1
(index_type h, index_type t, bool ci, bool co, const std::string& name)
{
  index_type nh_p = non_terminal_labels.first(h);
  assert(nh_p != no_such_index);

  TerminalRule r;
  r.name = name;
  r.head = h;
  r.terminal = t;
  r.c_in = ci;
  r.c_out = co;
  classers.append(r);
  t_rules[nh_p].insert(classers.length() - 1);
}

void GraphParser::add_rule_2
(index_type h, index_type n1, index_type n2,
 bool e12, bool e21,
 bool ci1, bool co1, bool ci2, bool co2,
 const std::string& name)
{
  index_type nh_p = non_terminal_labels.first(h);
  assert(nh_p != no_such_index);
  index_type n1_p = non_terminal_labels.first(n1);
  assert(n1_p != no_such_index);
  index_type n2_p = non_terminal_labels.first(n1);
  assert(n2_p != no_such_index);

  NonTerminalRule r;
  r.name = name;
  r.head = h;
  r.n1 = n1;
  r.n2 = n2;
  r.e_12 = e12;
  r.e_21 = e21;
  r.c_in_1 = ci1;
  r.c_out_1 = co1;
  r.c_in_2 = ci2;
  r.c_out_2 = co2;
  rules.append(r);
  nt_rules[nh_p].insert(rules.length() - 1);
}

void GraphParser::add_rule_3
(index_type h, index_type n1, index_type n2, index_type n3,
 bool e12, bool e21, bool e13, bool e31, bool e23, bool e32,
 bool ci1, bool co1, bool ci2, bool co2, bool ci3, bool co3,
 const std::string& name)
{
  index_type tmp = new_non_terminal(name);
  im_labels.insert(tmp);
  add_rule_2(h, n1, tmp, e12 || e13, e21 || e31,
	     ci1, co1, ci2 || ci3, co2 || co3, name);
  add_rule_2(tmp, n2, n3, e23, e32, ci2 || e12, co2 || e21,
	     ci3 || e13, co3 || e31, name + "b");
  im_rules.insert(rules.length() - 1);
}

void GraphParser::add_rule_4
(index_type h,
 index_type n1, index_type n2, index_type n3, index_type n4,
 bool e12, bool e21, bool e13, bool e31, bool e14, bool e41,
 bool e23, bool e32, bool e24, bool e42, bool e34, bool e43,
 bool ci1, bool co1, bool ci2, bool co2, bool ci3, bool co3,
 bool ci4, bool co4, const std::string& name)
{
  index_type tmp1 = new_non_terminal(name);
  index_type tmp2 = new_non_terminal(name);
  im_labels.insert(tmp1);
  im_labels.insert(tmp2);
  add_rule_2(h, n1, tmp1, e12 || e13 || e14, e21 || e31 || e41,
	     ci1, co1, ci2 || ci3 || ci4, co2 || co3 || co4, name);
  add_rule_2(tmp1, n2, tmp2, e23 || e24, e32 || e42, ci2 || e12, co2 || e21,
	     ci3 || ci4 || e13 || e14 || e24,
	     co3 || co4 || e31 || e41 || e42, name + "b");
  im_rules.insert(rules.length() - 1);
  add_rule_2(tmp2, n3, n4, e34, e43,
	     ci3 || e13 || e23, co3 || e31 || e32,
	     ci4 || e14 || e24, co4 || e41 || e42, name + "c");
  im_rules.insert(rules.length() - 1);
}

void GraphParser::write_rules(std::ostream& s)
{
  s << "non-terminal rules:" << std::endl;
  for (index_type k = 0; k < rules.length(); k++) {
    s << "(#" << k << ") ";
    write_rule(s, rules[k]);
    s << std::endl;
  }
  s << "terminal rules:" << std::endl;
  for (index_type k = 0; k < classers.length(); k++) {
    s << "(#" << k << ") ";
    write_rule(s, classers[k]);
    s << std::endl;
  }
//   s << "digraph rules {" << std::endl;
//   for (index_type k = 0; k < rules.length(); k++) {
//     s << "subgraph cluster" << k << " {" << std::endl;
//     s << "label=\"" << head << "\";" << std::endl;
//     index_graph g2(2);
//     g2.node_label(0) = n1;
//     g2.node_label(1) = n2;
//     if (e_12)
//       g2.add_edge(0, 1, EDGE_DIR_CONST);
//     if (e_21)
//       g2.add_edge(1, 0, EDGE_DIR_CONST);
//     s << "}" << std::endl;
//     for (index_type i = 0; i < c_in_1.length(); i++) {
//       s << "r" << k << "_c_in_1_" << i << " [];";
//     }
//   }
//   s << "}" << std::endl;
}

void GraphParser::write_context_node
(std::ostream& s,
 index_type c_label,
 bool is_out,
 std::string rule_name,
 std::string node_in_rule,
 std::string c_name,
 index_type c_index)
{
  index_type li =
    non_terminal_labels.first(c_label);
  s << " " << rule_name << "_" << c_name << "_" << c_index;
  if (li != no_such_index) {
    s << " [shape=plaintext,label=\"" << non_terminal_names[li]
      << "\"];" << std::endl;
  }
  else {
    s << " [";
    index_graph::write_node_style(s, c_label);
    s << ",label=\"\"];" << std::endl;
  }
  if (is_out) {
    s << " " << rule_name << "_" << node_in_rule
      << " -> " << rule_name << "_" << c_name << "_" << c_index
      << " [dir=forward];"
      << std::endl;
  }
  else {
    s << " " << rule_name << "_" << c_name << "_" << c_index
      << " -> " << rule_name << "_" << node_in_rule << " [dir=forward];"
      << std::endl;
  }
}

void GraphParser::write_rules_graph(std::ostream& s)
{
  s << "digraph rules {" << std::endl;
  for (index_type k = 0; k < non_terminal_labels.length(); k++) {
    // s << "subgraph cluster" << non_terminal_names[k] << " {" << std::endl;
    for (index_type r = 0; r < nt_rules[k].length(); r++) {
      index_type n0 = non_terminal_labels.first(rules[nt_rules[k][r]].head);
      assert(n0 != no_such_index);
      index_type n1 = non_terminal_labels.first(rules[nt_rules[k][r]].n1);
      assert(n1 != no_such_index);
      index_type n2 = non_terminal_labels.first(rules[nt_rules[k][r]].n2);
      assert(n2 != no_such_index);
      s << "subgraph cluster" << rules[nt_rules[k][r]].name << " {"
	<< std::endl;
      s << "label=\"" << non_terminal_names[n0]
	<< " (" << nt_rules[k][r] << ". "
	<< rules[nt_rules[k][r]].name << ")\";" << std::endl;
      s << " " << rules[nt_rules[k][r]].name
	<< "_N1 [shape=box,style=bold,label=\""
	<< non_terminal_names[n1] << "\"];"
	<< std::endl;
      s << " " << rules[nt_rules[k][r]].name
	<< "_N2 [shape=box,style=bold,label=\""
	<< non_terminal_names[n2] << "\"];"
	<< std::endl;
      if (rules[nt_rules[k][r]].e_12)
	s << " " << rules[nt_rules[k][r]].name << "_N1 -> "
	  << rules[nt_rules[k][r]].name << "_N2 [dir=forward];"
	  << std::endl;
      if (rules[nt_rules[k][r]].e_21)
	s << " " << rules[nt_rules[k][r]].name << "_N2 -> "
	  << rules[nt_rules[k][r]].name << "_N1 [dir=forward];"
	  << std::endl;
      s << "}" << std::endl;

      if (rules[nt_rules[k][r]].c_in_1 ||
	  rules[nt_rules[k][r]].c_in_2) {
	s << rules[nt_rules[k][r]].name
	  << "_CIN [shape=plaintext,label=\"...\"];"
	  << std::endl;
	if (rules[nt_rules[k][r]].c_in_1)
	  s << rules[nt_rules[k][r]].name << "_CIN -> "
	    << rules[nt_rules[k][r]].name << "_N1 [dir=forward];"
	    << std::endl;
	if (rules[nt_rules[k][r]].c_in_2)
	  s << rules[nt_rules[k][r]].name << "_CIN -> "
	    << rules[nt_rules[k][r]].name << "_N2 [dir=forward];"
	    << std::endl;
      }
      if (rules[nt_rules[k][r]].c_out_1 ||
	  rules[nt_rules[k][r]].c_out_2) {
	s << rules[nt_rules[k][r]].name
	  << "_COUT [shape=plaintext,label=\"...\"];"
	  << std::endl;
	if (rules[nt_rules[k][r]].c_out_1)
	  s << rules[nt_rules[k][r]].name << "_N1 -> "
	    << rules[nt_rules[k][r]].name << "_COUT [dir=forward];"
	    << std::endl;
	if (rules[nt_rules[k][r]].c_out_2)
	  s << rules[nt_rules[k][r]].name << "_N2 -> "
	    << rules[nt_rules[k][r]].name << "_COUT [dir=forward];"
	    << std::endl;
      }
    }
    for (index_type r = 0; r < t_rules[k].length(); r++) {
      index_type n0 = non_terminal_labels.first(classers[t_rules[k][r]].head);
      assert(n0 != no_such_index);
      s << "subgraph cluster" << classers[t_rules[k][r]].name << " {"
	<< std::endl;
      s << "label=\"" << non_terminal_names[n0]
	<< " (" << classers[t_rules[k][r]].name << ")\";" << std::endl;
      s << " " << classers[t_rules[k][r]].name
	<< "_T [";
      index_graph::write_node_style(s, classers[t_rules[k][r]].terminal);
      s << ",label=\"\"];" << std::endl;
      s << "}" << std::endl;
      if (classers[t_rules[k][r]].c_in) {
	s << classers[t_rules[k][r]].name
	  << "_CIN [shape=plaintext,label=\"...\"];"
	  << std::endl;
	s << classers[t_rules[k][r]].name << "_CIN -> "
	  << classers[t_rules[k][r]].name << "_T [dir=forward];"
	  << std::endl;
      }
      if (classers[t_rules[k][r]].c_out) {
	s << classers[t_rules[k][r]].name
	  << "_COUT [shape=plaintext,label=\"...\"];"
	  << std::endl;
	s << classers[t_rules[k][r]].name << "_T -> "
	  << classers[t_rules[k][r]].name << "_COUT [dir=forward];"
	  << std::endl;
      }
    }
    // s << "}" << std::endl;
  }
  s << "}" << std::endl;
}

index_type GraphParser::parse
(const index_graph& g, index_type s)
{
  for (index_type k = 0; k < g.size(); k++) {
    // find all terminal rules ("classers") that produce node label g[k]:
    for (index_type i = 0; i < classers.length(); i++)
      if (classers[i].matches(g, k)) {
	matches.append(match_table_entry(partial_match(classers[i].head, k),
					 parse_tree_node(i)));
      }
  }

  index_type p = 0;
  while (p < matches.length()) {
    for (index_type q = 0; q < p; q++) {
#ifdef TRACE_PRINT_LOTS
      std::cerr << "matching " << p << " = " << matches[p].first
		<< " and " << q << " = " << matches[q].first
		<< std::endl;
#endif
      for (index_type i = 0; i < rules.length(); i++) {
	std::pair<bool,bool> ok =
	  rules[i].matches(g, matches[p].first, matches[q].first);
	if (ok.first) {
	  partial_match pm(rules[i].head, matches[p].first, matches[q].first);
	  parse_tree_node pn(i, (ok.second ? q : p), (ok.second ? p : q));
	  matches.append(match_table_entry(pm, pn));
#ifdef TRACE_PRINT_LOTS
	  std::cerr << "rule " << rules[i] << " works, created entry "
		    << matches.length() - 1 << " = ";
	  write_entry(std::cerr, matches[matches.length() - 1]);
	  std::cerr << std::endl;
#endif
	  if (matches[matches.length() - 1].first.first == s) {
	    if (matches[matches.length() - 1].first.second.length() == g.size()) {
#ifdef TRACE_PRINT_LOTS
	      std::cerr << "successful match of entire graph!" << std::endl;
#endif
	      return matches.length() - 1;
	    }
	  }
	}
      }
    }
    p += 1;
  }

  return no_such_index;
}

void GraphParser::make_parse_tree
(index_type p, index_graph& t)
{
  assert(p < matches.length());
  index_vec q;
  q.append(p);
  index_type i = 0;
  while (i < q.length()) {
    if (!matches[q[i]].second.is_terminal) {
      assert(matches[q[i]].second.children.first != no_such_index);
      q.append(matches[q[i]].second.children.first); 
      assert(matches[q[i]].second.children.second != no_such_index);
      q.append(matches[q[i]].second.children.second);
    }
    i += 1;
  }
  t.init(q.length());
  for (index_type k = 0; k < q.length(); k++) {
    t.node_label(k) = q[k];
    if (!matches[q[k]].second.is_terminal) {
      index_type p1 = q.first(matches[q[k]].second.children.first);
      index_type p2 = q.first(matches[q[k]].second.children.second);
      assert(p1 != no_such_index);
      assert(p2 != no_such_index);
      t.add_edge(p1, k, 1);
      t.add_edge(p2, k, 2);
    }
  }
}

void GraphParser::compact_parse_tree
(index_graph& t, const index_set& l_remove)
{
  index_graph tmp(t);
  index_set n_keep;
  n_keep.fill(t.size());
  for (index_type k = 0; k < tmp.size(); k++) {
    index_type p = tmp.node_label(k);
    if (l_remove.contains(matches[p].first.first)) {
      // std::cerr << "removing node " << k << " with label "
      //	<< matches[p].first.first << std::endl;
      for (index_type i = 0; i < tmp.predecessors(k).length(); i++) {
	index_type n = tmp.edge_label(tmp.predecessors(k)[i], k);
	for (index_type j = 0; j < tmp.successors(k).length(); j++)
	  tmp.add_edge(tmp.predecessors(k)[i], tmp.successors(k)[j],
		       n + tmp.edge_label(k, tmp.successors(k)[j]) - 1);
      }
      n_keep.subtract(k);
    }
  }
  tmp.subgraph(t, n_keep);
}

void GraphParser::compact_parse_tree(index_graph& t)
{
  compact_parse_tree(t, im_labels);
}

void GraphParser::write_decomposition
(std::ostream& s, const index_graph& g, const index_graph& t, index_type n)
{
  index_type p = t.node_label(n);
  assert(p < matches.length());
  index_type l = matches[p].first.first;
  index_type li = non_terminal_labels.first(l);
  assert(li != no_such_index);
  index_type ri = matches[p].second.rule_id;
  s << "subgraph cluster" << p << " {" << std::endl;
  s << "label=\"" << non_terminal_names[li] << " (";
  if (matches[p].second.is_terminal)
    s << classers[ri].name;
  else
    s << rules[ri].name;
  s << ")\";" << std::endl;
  if (t.predecessors(n).empty()) {
    // assert(matches[p].second.is_terminal);
    assert(matches[p].first.second.length() == 1);
    index_type gn = matches[p].first.second[0];
    s << "GN" << gn << " [";
    index_graph::write_node_style(s, g.node_label(gn));
    s << ",label=\"" << gn << "\"];" << std::endl;
  }
  else {
    for (index_type i = 0; i < t.predecessors(n).length(); i++)
      write_decomposition(s, g, t, t.predecessors(n)[i]);
  }
  s << "}" << std::endl;
}

void GraphParser::write_decomposition
(std::ostream& s, const index_graph& g, const index_graph& t)
{
  index_type n = t.first_leaf();
  assert(n != no_such_index);
  s << "digraph decomposition {" << std::endl;
  write_decomposition(s, g, t, n);
  for (index_type i = 0; i < g.size(); i++)
    for (index_type j = 0; j < g.size(); j++)
      if (g.adjacent(i, j))
	s << "GN" << i << " -> GN" << j << " [dir=forward];" << std::endl;
  s << "}" << std::endl;
}

void GraphParser::random_graph
(index_type s, index_type n_steps, index_graph& g, RNG& rng)
{
  index_set prio_nt;
  for (index_type k = 0; k < non_terminal_labels.length(); k++)
    if (t_rules[k].empty())
      prio_nt.insert(non_terminal_labels[k]);

  g.init(1);
  g.node_label(0) = s;
  index_type l0 = non_terminal_labels.first(s);
  assert(l0 != no_such_index);

  pair_vec app;
  bool_vec prio_app;

  for (index_type k = 0; k < rules.length(); k++)
    if (rules[k].applicable(g, 0))
      app.append(index_pair(k, 0));
  prio_app.assign_value(prio_nt.contains(s), app.length());

#ifdef TRACE_PRINT_GRAPH_DERIVATION
  std::cerr << "digraph random_graph {" << std::endl;
  lvector<std::string> ls("", 1);
  ls[0] = non_terminal_names[l0];
  write_labeled_digraph< lvector<std::string> >
    (std::cerr, g, ls, true, "cluster0", 100);
#endif

  index_type stepc = 0;
  while ((stepc < n_steps) || (prio_app.count(true) > 0))  {
    assert(app.length() > 0);
    index_type sel;
    if (prio_app.count(true) > 0) {
      sel = rng.select_one_of(prio_app);
    }
    else {
      sel = rng.random_in_range(app.length());
    }
    index_type r = app[sel].first;
    index_type n = app[sel].second;
    rules[r].apply(g, n);
    bool_vec c(false, app.length());
    for (index_type k = 0; k < app.length(); k++)
      if (app[k].second == n) c[k] = true;
    app.remove(c);
    prio_app.remove(c);
    assert(app.length() == prio_app.length());
    for (index_type k = 0; k < rules.length(); k++) {
      if (rules[k].applicable(g, n)) {
	app.append(index_pair(k, n));
	prio_app.append(prio_nt.contains(g.node_label(n)));
      }
      if (rules[k].applicable(g, g.size() - 1)) {
	app.append(index_pair(k, g.size() - 1));
	prio_app.append(prio_nt.contains(g.node_label(g.size() - 1)));
      }
    }

    stepc += 1;

#ifdef TRACE_PRINT_GRAPH_DERIVATION
    ls.set_length(g.size());
    for (index_type k = 0; k < g.size(); k++) {
      index_type lk = non_terminal_labels.first(g.node_label(k));
      assert(lk != no_such_index);
      ls[k] = non_terminal_names[lk];
    }
    std::cerr << "subgraph cluster" << (stepc + 1) * 100 << " {"
	      << std::endl;
    std::cerr << "label=\"" << rules[r].name << " @ " << n << "\";"
	      << std::endl;
    write_labeled_digraph< lvector<std::string> >
      (std::cerr, g, ls, true, 0, (stepc + 1) * 100);
    std::cerr << "}" << std::endl;
#endif
  }

  for (index_type i = 0; i < g.size(); i++) {
    app.clear();
    for (index_type k = 0; k < classers.length(); k++)
      if (classers[k].applicable(g, i))
	app.append(index_pair(k, i));
    assert(app.length() > 0);
    index_type sel = rng.random_in_range(app.length());
    index_type r = app[sel].first;
    index_type n = app[sel].second;
    classers[r].apply(g, n);
  }

#ifdef TRACE_PRINT_GRAPH_DERIVATION
  std::cerr << "}" << std::endl;
#endif
}

void GraphParser::write_rule
(std::ostream& s, const NonTerminalRule& r) const
{
  index_type nh = non_terminal_labels.first(r.head);
  assert(nh != no_such_index);
  index_type n1 = non_terminal_labels.first(r.n1);
  assert(n1 != no_such_index);
  index_type n2 = non_terminal_labels.first(r.n2);
  assert(n2 != no_such_index);
  s << r.name << ": " << non_terminal_names[nh]
    << " => (" << non_terminal_names[n1] << " ";
  if (r.e_21) s << "<";
  s << "-";
  if (r.e_12) s << ">";
  s << " " << non_terminal_names[n2];
  if (r.c_in_1) s << ", ->1";
  if (r.c_out_1) s << ", 1->";
  if (r.c_in_2) s << ", ->2";
  if (r.c_out_2) s << ", 2->";
  s << ")";
}

void GraphParser::write_rule
(std::ostream& s, const TerminalRule& r) const
{
  index_type nh = non_terminal_labels.first(r.head);
  assert(nh != no_such_index);
  s << r.name << ": " << non_terminal_names[nh]
    << " => (" << r.terminal;
  if (r.c_in) s << ", ->1";
  if (r.c_out) s << ", 1->";
  s << ")";
}

void GraphParser::write_entry
(std::ostream& s, const match_table_entry& e) const
{
  index_type n = non_terminal_labels.first(e.first.first);
  assert(n != no_such_index);
  s << non_terminal_names[n] << ": " << e.first.second;
  if (e.second.is_terminal) {
    s << " (by terminal rule " << classers[e.second.rule_id].name << ")";
  }
  else {
    s << " (by rule " << rules[e.second.rule_id].name
      << " from " << e.second.children << ")";
  }
}

GG1v1::GG1v1()
{
  define_non_terminal(NL_A0, "A0");
  define_non_terminal(NL_A1, "A1");
  define_non_terminal(NL_A1I, "A1I");
  define_non_terminal(NL_A2, "A2");
  define_non_terminal(NL_PA1, "PA1");
  define_non_terminal(NL_PA1I, "PA1I");
  define_non_terminal(NL_P1, "P1");
  define_non_terminal(NL_P1I, "P1I");
  define_non_terminal(NL_AP1, "AP1");
  define_non_terminal(NL_AP1I, "AP1I");
  define_non_terminal(NL_P2, "P2");
  define_non_terminal(NL_P2I, "P2I");

  add_rule_1(NL_A0, TL_A, TL_P, TL_P, "tA0_A");
  add_rule_1(NL_A1, TL_A, TL_P, TL_P, "tA1_A");
  add_rule_1(NL_A2, TL_A, TL_P, TL_P, "tA2_A");

  add_rule_1(NL_P1, TL_P, TL_A, TL_A, "tP1_P");
  add_rule_1(NL_P1, TL_PG, TL_A, TL_A, "tP1_PG");
  add_rule_1(NL_P2, TL_P, TL_A, TL_A, "tP2_P");
  add_rule_1(NL_P2, TL_PG, TL_A, TL_A, "tP2_PG");
  add_rule_1(NL_P1I, TL_P, TL_A, TL_A, "tP1I_P");
  add_rule_1(NL_P1I, TL_PG, TL_A, TL_A, "tP1I_PG");
  add_rule_1(NL_P2I, TL_P, TL_A, TL_A, "tP2I_P");
  add_rule_1(NL_P2I, TL_PG, TL_A, TL_A, "tP2I_PG");
  add_rule_1(NL_P1I, TL_PI, TL_A, TL_A, "tP1I_PI");
  add_rule_1(NL_P1I, TL_PIG, TL_A, TL_A, "tP1I_PIG");
  add_rule_1(NL_P2I, TL_PI, TL_A, TL_A, "tP2I_PI");
  add_rule_1(NL_P2I, TL_PIG, TL_A, TL_A, "tP2I_PIG");

  // A0 rules:
  add_rule_2(NL_A0, NL_A1, NL_P1I, true, true,
	     false, false, false, false, "A0Synch1");
  add_rule_2(NL_A0, NL_A1I, NL_P1, true, true,
	     false, false, false, false, "A0Synch2");

  // A1/A1I rules:
  add_rule_2(NL_A1, NL_A2, NL_P1I, true, true,
	     true, true, false, false, "A1Synch");
  add_rule_2(NL_A1, NL_A1, NL_PA1, true, false,
	     true, false, false, true, "A1Seq");
  add_rule_2(NL_A1I, NL_A1, NL_PA1I, true, false,
	     true, false, false, true, "A1ISeq");

  // PA1/PA1I rules:
  add_rule_2(NL_PA1, NL_P1, NL_A1, true, false,
	     true, false, false, true, "PA1Seq");
  add_rule_2(NL_PA1I, NL_P1I, NL_A1, true, false,
	     true, false, false, true, "PA1ISeq1");
  add_rule_2(NL_PA1I, NL_P1, NL_A1I, true, false,
	     true, false, false, true, "PA1ISeq2");

  // P1/P1I rules:
  add_rule_2(NL_P1, NL_P2, NL_A1, true, true,
	     true, true, false, false, "P1Branch");
  add_rule_2(NL_P1, NL_P1, NL_AP1, true, false,
	     true, false, false, true, "P1Seq");
  add_rule_2(NL_P1I, NL_P2I, NL_A1, true, true,
	     true, true, false, false, "P1IBranch1");
  add_rule_2(NL_P1I, NL_P2, NL_A1I, true, true,
	     true, true, false, false, "P1IBranch2");
  add_rule_2(NL_P1I, NL_P1I, NL_AP1, true, false,
	     true, false, false, true, "P1ISeq1");
  add_rule_2(NL_P1I, NL_P1, NL_AP1I, true, false,
	     true, false, false, true, "P1ISeq2");

  // AP1/AP1I rules:
  add_rule_2(NL_AP1, NL_A1, NL_P1, true, false,
	     true, false, false, true, "AP1Seq");
  add_rule_2(NL_AP1I, NL_A1I, NL_P1, true, false,
	     true, false, false, true, "AP1ISeq1");
  add_rule_2(NL_AP1I, NL_A1, NL_P1I, true, false,
	     true, false, false, true, "AP1ISeq2");
}

GG1v2::GG1v2()
{
  define_non_terminal(NL_A0, "A0");
  define_non_terminal(NL_A1, "A1");
  define_non_terminal(NL_A2, "A2");
  define_non_terminal(NL_P1, "P1");
  define_non_terminal(NL_P1I, "P1I");
  define_non_terminal(NL_P2, "P2");
  define_non_terminal(NL_P2I, "P2I");

  add_rule_1(NL_A0, TL_A, TL_P, TL_P, "tA0_A");
  add_rule_1(NL_A1, TL_A, TL_P, TL_P, "tA1_A");
  add_rule_1(NL_A2, TL_A, TL_P, TL_P, "tA2_A");

  add_rule_1(NL_P1, TL_P, TL_A, TL_A, "tP1_P");
  add_rule_1(NL_P1, TL_PG, TL_A, TL_A, "tP1_PG");
  add_rule_1(NL_P2, TL_P, TL_A, TL_A, "tP2_P");
  add_rule_1(NL_P2, TL_PG, TL_A, TL_A, "tP2_PG");
  add_rule_1(NL_P1I, TL_P, TL_A, TL_A, "tP1I_P");
  add_rule_1(NL_P1I, TL_PG, TL_A, TL_A, "tP1I_PG");
  add_rule_1(NL_P2I, TL_P, TL_A, TL_A, "tP2I_P");
  add_rule_1(NL_P2I, TL_PG, TL_A, TL_A, "tP2I_PG");
  add_rule_1(NL_P1I, TL_PI, TL_A, TL_A, "tP1I_PI");
  add_rule_1(NL_P1I, TL_PIG, TL_A, TL_A, "tP1I_PIG");
  add_rule_1(NL_P2I, TL_PI, TL_A, TL_A, "tP2I_PI");
  add_rule_1(NL_P2I, TL_PIG, TL_A, TL_A, "tP2I_PIG");

  // A0 rules:
  add_rule_2(NL_A0, NL_A1, NL_P1I, true, true,
	     false, false, false, false, "A0SyncLoop");
  add_rule_3(NL_A0, NL_P1I, NL_A1, NL_P1,
	     true, false, false, false, true, false,
	     false, false, false, false, false, false,
	     "A0SyncChain");

  // A1 rules:
  add_rule_2(NL_A1, NL_A2, NL_P1I, true, true,
	     true, true, false, false, "A1SyncLoop");
  add_rule_3(NL_A1, NL_P1I, NL_A2, NL_P1,
	     true, false, false, false, true, false,
	     false, false, true, true, false, false,
	     "A1SyncChain");
//   add_rule_3(NL_A1, NL_A1, NL_P1, NL_A1,
// 	     true, false, false, false, true, false,
// 	     true, false, false, false, false, true,
// 	     "A1Seq");

  // P1/P1I branching rules:
  add_rule_4(NL_P1, NL_P2, NL_A1, NL_P1, NL_A1,
	     true, false, false, false, false, true, true, false,
	     false, false, true, false,
	     true, true, false, false, false, false, false, false,
	     "P1BranchLoop");
  add_rule_3(NL_P1, NL_P2, NL_A1, NL_P1,
	     true, false, false, false, true, false,
	     true, true, false, false, false, false,
	     "P1BranchOut");
  add_rule_4(NL_P1I, NL_P2I, NL_A1, NL_P1, NL_A1,
	     true, false, false, false, false, true, true, false,
	     false, false, true, false,
	     true, true, false, false, false, false, false, false,
	     "P1IBranchLoop1");
  add_rule_4(NL_P1I, NL_P2, NL_A1, NL_P1I, NL_A1,
	     true, false, false, false, false, true, true, false,
	     false, false, true, false,
	     true, true, false, false, false, false, false, false,
	     "P1IBranchLoop2");
  add_rule_3(NL_P1I, NL_P2I, NL_A1, NL_P1,
	     true, false, false, false, true, false,
	     true, true, false, false, false, false,
	     "P1IBranchOut");
  add_rule_3(NL_P1I, NL_P2, NL_A1, NL_P1I,
	     false, true, false, false, false, true,
	     true, true, false, false, false, false,
	     "P1IBranchIn");

  // P1/P1I sequence rules
  add_rule_3(NL_P1, NL_P1, NL_A1, NL_P1,
	     true, false, false, false, true, false,
	     true, false, false, false, false, true,
	     "P1Sequence");
  add_rule_3(NL_P1I, NL_P1I, NL_A1, NL_P1,
	     true, false, false, false, true, false,
	     true, false, false, false, false, true,
	     "P1ISequence1");
  add_rule_3(NL_P1I, NL_P1, NL_A1, NL_P1I,
	     true, false, false, false, true, false,
	     true, false, false, false, false, true,
	     "P1ISequence2");

//   // choice and action sequence rules
//   add_rule_4(NL_P1, NL_P1, NL_A1, NL_A1, NL_P1,
// 	     true, false, true, false, false, false,
// 	     false, false, true, false, true, false,
// 	     true, false, false, false, false, false, false, true,
// 	     "P1Choice");
//   add_rule_4(NL_P1I, NL_P1I, NL_A1, NL_A1, NL_P1,
// 	     true, false, true, false, false, false,
// 	     false, false, true, false, true, false,
// 	     true, false, false, false, false, false, false, true,
// 	     "P1Choice");
//   add_rule_4(NL_P1I, NL_P1, NL_A1, NL_A1, NL_P1I,
// 	     true, false, true, false, false, false,
// 	     false, false, true, false, true, false,
// 	     true, false, false, false, false, false, false, true,
// 	     "P1Choice");
//   add_rule_3(NL_A1, NL_A1, NL_P1, NL_A1,
// 	     true, false, false, false, true, false,
// 	     true, false, false, false, false, true,
// 	     "A1Sequence");

}


GG1::GG1()
{
  define_non_terminal(NL_A0, "A0");
  define_non_terminal(NL_A1, "A1");
  define_non_terminal(NL_A1I, "A1I");
  define_non_terminal(NL_A1S1, "A1S1");
  define_non_terminal(NL_A1S2, "A1S2");
  define_non_terminal(NL_A2, "A2");
  define_non_terminal(NL_P1, "P1");
  define_non_terminal(NL_P1I, "P1I");
  define_non_terminal(NL_P2, "P2");
  define_non_terminal(NL_P2I, "P2I");

  add_rule_1(NL_A1, TL_A, TL_P, TL_P, "tA1_A");
  add_rule_1(NL_A1S1, TL_A, TL_P, TL_P, "tA1S1_A");
  add_rule_1(NL_A1S2, TL_A, TL_P, TL_P, "tA1S2_A");
  add_rule_1(NL_A2, TL_A, TL_P, TL_P, "tA2_A");

  add_rule_1(NL_P1, TL_P, TL_A, TL_A, "tP1_P");
  add_rule_1(NL_P1, TL_PG, TL_A, TL_A, "tP1_PG");
  add_rule_1(NL_P2, TL_P, TL_A, TL_A, "tP2_P");
  add_rule_1(NL_P2, TL_PG, TL_A, TL_A, "tP2_PG");
  add_rule_1(NL_P1I, TL_P, TL_A, TL_A, "tP1I_P");
  add_rule_1(NL_P1I, TL_PG, TL_A, TL_A, "tP1I_PG");
  add_rule_1(NL_P2I, TL_P, TL_A, TL_A, "tP2I_P");
  add_rule_1(NL_P2I, TL_PG, TL_A, TL_A, "tP2I_PG");
  add_rule_1(NL_P1I, TL_PI, TL_A, TL_A, "tP1I_PI");
  add_rule_1(NL_P1I, TL_PIG, TL_A, TL_A, "tP1I_PIG");
  add_rule_1(NL_P2I, TL_PI, TL_A, TL_A, "tP2I_PI");
  add_rule_1(NL_P2I, TL_PIG, TL_A, TL_A, "tP2I_PIG");

  // A0 rules:
  add_rule_2(NL_A0, NL_A1, NL_P1I, true, true,
	     false, false, false, false, "A0SyncLoop");
  add_rule_3(NL_A0, NL_P1I, NL_A1, NL_P1,
	     true, false, false, false, true, false,
	     false, false, false, false, false, false,
	     "A0SyncChain");

  // A1 rules:
  add_rule_2(NL_A1, NL_A2, NL_P1I, true, true,
	     true, true, false, false, "A1SyncLoop");
  add_rule_2(NL_A1S1, NL_A2, NL_P1I, true, true,
	     true, true, false, false, "A1SyncLoop");
  add_rule_3(NL_A1, NL_P1I, NL_A2, NL_P1,
	     true, false, false, false, true, false,
	     false, false, true, true, false, false,
	     "A1SyncChain");
  add_rule_3(NL_A1S1, NL_P1I, NL_A2, NL_P1,
	     true, false, false, false, true, false,
	     false, false, true, true, false, false,
	     "A1S1SyncChain");
  add_rule_3(NL_A1S2, NL_P2I, NL_A2, NL_P2,
	     true, false, false, false, true, false,
	     false, false, true, true, false, false,
	     "A1S2SyncChain");
  add_rule_2(NL_A1, NL_A1S1, NL_A1S2, false, false,
	     true, true, true, true, "A1Choice");

  // P1/P1I branching rules:
  add_rule_2(NL_P1, NL_P2, NL_A1, true, true,
	     true, true, false, false, "P1BranchLoop");
  add_rule_3(NL_P1, NL_P2, NL_A1, NL_P1,
	     true, false, false, false, true, false,
	     true, true, false, false, false, false,
	     "P1BranchOut");
  add_rule_2(NL_P1I, NL_P2I, NL_A1, true, true,
	     true, true, false, false, "P1IBranchLoop1");
  add_rule_2(NL_P1I, NL_P2, NL_A1I, true, true,
	     true, true, false, false, "P1IBranchLoop2");
  add_rule_3(NL_P1I, NL_P2I, NL_A1, NL_P1,
	     true, false, false, false, true, false,
	     true, true, false, false, false, false,
	     "P1IBranchOut");
  add_rule_3(NL_P1I, NL_P2, NL_A1, NL_P1I,
	     false, true, false, false, false, true,
	     true, true, false, false, false, false,
	     "P1IBranchIn");

  // sequence rules
  add_rule_3(NL_P1, NL_P1, NL_A1, NL_P1,
	     true, false, false, false, true, false,
	     true, false, false, false, false, true,
	     "P1Sequence");
  add_rule_3(NL_P1I, NL_P1I, NL_A1, NL_P1,
	     true, false, false, false, true, false,
	     true, false, false, false, false, true,
	     "P1ISequence1");
  add_rule_3(NL_P1I, NL_P1, NL_A1, NL_P1I,
	     true, false, false, false, true, false,
	     true, false, false, false, false, true,
	     "P1ISequence2");
  add_rule_3(NL_A1, NL_A1, NL_P1, NL_A1,
 	     true, false, false, false, true, false,
 	     true, false, false, false, false, true,
 	     "A1Sequence");
  add_rule_3(NL_A1I, NL_A1, NL_P1I, NL_A1,
 	     true, false, false, false, true, false,
 	     true, false, false, false, false, true,
 	     "A1ISequence");
  add_rule_3(NL_A1S1, NL_A2, NL_P1, NL_A2,
 	     true, false, false, false, true, false,
 	     true, false, false, false, false, true,
 	     "A1S1Sequence");
  add_rule_3(NL_A1S2, NL_A2, NL_P2, NL_A2,
 	     true, false, false, false, true, false,
 	     true, false, false, false, false, true,
 	     "A1S2Sequence");

}

END_HSPS_NAMESPACE
