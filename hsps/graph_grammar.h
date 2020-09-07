#ifndef GRAPH_GRAMMAR_H
#define GRAPH_GRAMMAR_H

#include "graph.h"

BEGIN_HSPS_NAMESPACE

struct partial_match : public std::pair<index_type, index_set> {
  partial_match() : std::pair<index_type, index_set>() { };
  partial_match(const partial_match& m) : std::pair<index_type, index_set>(m) { };
  partial_match(index_type l, index_type n)
    : std::pair<index_type, index_set>() {
    first = l;
    second.assign_singleton(n);
  };
  partial_match(index_type l, const partial_match& m1, const partial_match& m2)
    : std::pair<index_type, index_set>(l, EMPTYSET) {
    second.insert(m1.second);
    second.insert(m2.second);
  };
};

struct GraphGrammarRule {
  std::string name;
  index_type head;

  bool applicable(const index_graph& g, index_type n) const;
  void applicable(const index_graph& g, bool_vec& nn) const;
};

struct TerminalRule : public GraphGrammarRule {
  index_type terminal;
  bool c_in;
  bool c_out;

  bool matches(const index_graph& g, index_type n) const;
  void apply(index_graph& g, index_type n) const;
};

struct NonTerminalRule : public GraphGrammarRule {
  index_type n1;   // label of non-terminal node #1
  index_type n2;   // label of non-terminal node #2
  bool       e_12; // edge from #1 to #2 exists?
  bool       e_21; // edge from #2 to #1 exists?
  bool       c_in_1;
  bool       c_out_1;
  bool       c_in_2;
  bool       c_out_2;

  std::pair<bool,bool> matches(const index_graph& g,
			       const partial_match& m1,
			       const partial_match& m2) const;
  void apply(index_graph& g, index_type n) const;
};

class GraphParser {
 public:
  index_vec            non_terminal_labels;
  lvector<std::string> non_terminal_names;

  lvector<TerminalRule>    classers;
  lvector<NonTerminalRule> rules;

  index_set_vec nt_rules;
  index_set_vec t_rules;

  index_set im_labels;
  index_set im_rules;

  struct parse_tree_node {
    bool    is_terminal;
    index_type  rule_id;
    index_pair children;

    parse_tree_node()
      : is_terminal(false), rule_id(no_such_index), children(no_such_index) {};
    parse_tree_node(const parse_tree_node& n)
      : is_terminal(n.is_terminal),rule_id(n.rule_id),children(n.children) {};
    parse_tree_node(index_type r)
      : is_terminal(true), rule_id(r), children(no_such_index) { };
    parse_tree_node(index_type r, index_type c1, index_type c2)
      : is_terminal(false), rule_id(r), children(c1, c2) { };
  };

  typedef std::pair<partial_match, parse_tree_node> match_table_entry;
  lvector<match_table_entry> matches;

  GraphParser();
  ~GraphParser();

  // define grammar
  void define_non_terminal(index_type label, const std::string& name);
  index_type new_non_terminal(const std::string& basename);

  void add_rule_1(index_type h, index_type t,
			 bool ci, bool co,
			 const std::string& name);
  void add_rule_2(index_type h, index_type n1, index_type n2,
		  bool e12, bool e21,
		  bool ci1, bool co1, bool ci2, bool co2,
		  const std::string& name);
  void add_rule_3(index_type h, index_type n1, index_type n2, index_type n3,
		  bool e12, bool e21, bool e13, bool e31, bool e23, bool e32,
		  bool ci1, bool co1, bool ci2, bool co2, bool ci3, bool co3,
		  const std::string& name);
  void add_rule_4(index_type h,
		  index_type n1, index_type n2, index_type n3, index_type n4,
		  bool e12, bool e21, bool e13, bool e31, bool e14, bool e41,
		  bool e23, bool e32, bool e24, bool e42, bool e34, bool e43,
		  bool ci1, bool co1, bool ci2, bool co2, bool ci3, bool co3,
		  bool ci4, bool co4, const std::string& name);

  void write_rules(std::ostream& s);
  void write_rules_graph(std::ostream& s);
  // internal method used by write_rules_graph
  void write_context_node(std::ostream& s,
			  index_type c_label,
			  bool is_out,
			  std::string rule_name,
			  std::string node_in_rule,
			  std::string c_name,
			  index_type c_index);

  // parse a graph: returns the index in the match table of the first
  // entry found that matches the entire graph and has non-terminal
  // label 's', or no_such_index if no such match is found
  index_type parse(const index_graph& g, index_type s = no_such_index);
  void make_parse_tree(index_type p, index_graph& t);
  void compact_parse_tree(index_graph& t, const index_set& l_remove);
  void compact_parse_tree(index_graph& t);

  void write_decomposition(std::ostream& s,
			   const index_graph& g,
			   const index_graph& t);
  // internal method, called by write_decomposition
  void write_decomposition(std::ostream& s,
			   const index_graph& g,
			   const index_graph& t,
			   index_type n);

  // derive a graph by applying n random rules, starting from a
  // singleton graph labelled with s
  void random_graph(index_type s, index_type n, index_graph& g, RNG& rng);

  void write_rule(std::ostream& s, const NonTerminalRule& r) const;
  void write_rule(std::ostream& s, const TerminalRule& r) const;
  void write_entry(std::ostream& s, const match_table_entry& e) const;
};

class GG1v1 : public GraphParser {
 public:
  static const index_type TL_P   = index_graph::NS_ELLIPSE;
  static const index_type TL_PG  = (index_graph::NS_ELLIPSE +
				    index_graph::NS_DOUBLE);
  static const index_type TL_PI  = (index_graph::NS_ELLIPSE +
				    index_graph::NS_FILLED);
  static const index_type TL_PIG = (index_graph::NS_ELLIPSE +
				    index_graph::NS_FILLED +
				    index_graph::NS_DOUBLE);
  static const index_type TL_A   = index_graph::NS_BOX;

  static const index_type NL_A0   = (index_graph::STYLE_MAX * 1);
  static const index_type NL_A1   = (index_graph::STYLE_MAX * 2);
  static const index_type NL_A1I  = (index_graph::STYLE_MAX * 3);
  static const index_type NL_A2   = (index_graph::STYLE_MAX * 4);
  static const index_type NL_PA1  = (index_graph::STYLE_MAX * 5);
  static const index_type NL_PA1I = (index_graph::STYLE_MAX * 6);
  static const index_type NL_P1   = (index_graph::STYLE_MAX * 7);
  static const index_type NL_P1I  = (index_graph::STYLE_MAX * 8);
  static const index_type NL_AP1  = (index_graph::STYLE_MAX * 9);
  static const index_type NL_AP1I = (index_graph::STYLE_MAX * 10);
  static const index_type NL_P2   = (index_graph::STYLE_MAX * 11);
  static const index_type NL_P2I  = (index_graph::STYLE_MAX * 12);
 
  GG1v1();
};

class GG1v2 : public GraphParser {
 public:
  static const index_type TL_P   = index_graph::NS_ELLIPSE;
  static const index_type TL_PG  = (index_graph::NS_ELLIPSE +
				    index_graph::NS_DOUBLE);
  static const index_type TL_PI  = (index_graph::NS_ELLIPSE +
				    index_graph::NS_FILLED);
  static const index_type TL_PIG = (index_graph::NS_ELLIPSE +
				    index_graph::NS_FILLED +
				    index_graph::NS_DOUBLE);
  static const index_type TL_A   = index_graph::NS_BOX;

  static const index_type NL_A0   = (index_graph::STYLE_MAX * 1);
  static const index_type NL_A1   = (index_graph::STYLE_MAX * 2);
  static const index_type NL_A2   = (index_graph::STYLE_MAX * 3);
  static const index_type NL_P1   = (index_graph::STYLE_MAX * 4);
  static const index_type NL_P1I  = (index_graph::STYLE_MAX * 5);
  static const index_type NL_P2   = (index_graph::STYLE_MAX * 6);
  static const index_type NL_P2I  = (index_graph::STYLE_MAX * 7);
 
  GG1v2();
};

class GG1 : public GraphParser {
 public:
  static const index_type TL_P   = index_graph::NS_ELLIPSE;
  static const index_type TL_PG  = (index_graph::NS_ELLIPSE +
				    index_graph::NS_DOUBLE);
  static const index_type TL_PI  = (index_graph::NS_ELLIPSE +
				    index_graph::NS_FILLED);
  static const index_type TL_PIG = (index_graph::NS_ELLIPSE +
				    index_graph::NS_FILLED +
				    index_graph::NS_DOUBLE);
  static const index_type TL_A   = index_graph::NS_BOX;

  static const index_type NL_A0   = (index_graph::STYLE_MAX * 1);
  static const index_type NL_A1   = (index_graph::STYLE_MAX * 2);
  static const index_type NL_A1I  = (index_graph::STYLE_MAX * 3);
  static const index_type NL_A2   = (index_graph::STYLE_MAX * 4);
  static const index_type NL_P1   = (index_graph::STYLE_MAX * 5);
  static const index_type NL_P1I  = (index_graph::STYLE_MAX * 6);
  static const index_type NL_P2   = (index_graph::STYLE_MAX * 7);
  static const index_type NL_P2I  = (index_graph::STYLE_MAX * 8);
  static const index_type NL_A1S1 = (index_graph::STYLE_MAX * 9);
  static const index_type NL_A1S2 = (index_graph::STYLE_MAX * 10);
 
  GG1();
};


// print operators

inline ::std::ostream& operator<<(::std::ostream& s, const partial_match& m)
{
  return s << m.first << ": " << m.second;
};

END_HSPS_NAMESPACE

#endif
