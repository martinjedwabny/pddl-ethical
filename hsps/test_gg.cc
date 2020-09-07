
#include "graph_grammar.h"
#include <sstream>

BEGIN_HSPS_NAMESPACE


const index_type TL_P   = index_graph::NS_CIRCLE;
const index_type TL_PG  = (index_graph::NS_CIRCLE +
			   index_graph::NS_DOUBLE);
const index_type TL_PI  = (index_graph::NS_CIRCLE +
			   index_graph::NS_FILLED);
const index_type TL_PIG = (index_graph::NS_CIRCLE +
			   index_graph::NS_FILLED +
			   index_graph::NS_DOUBLE);
const index_type TL_A   = index_graph::NS_BOX;

const index_type NL_A0   = (index_graph::STYLE_MAX * 1);
const index_type NL_A1   = (index_graph::STYLE_MAX * 2);
const index_type NL_A1I  = (index_graph::STYLE_MAX * 3);
const index_type NL_A2   = (index_graph::STYLE_MAX * 4);
const index_type NL_PA1  = (index_graph::STYLE_MAX * 5);
const index_type NL_PA1I = (index_graph::STYLE_MAX * 6);
const index_type NL_P1   = (index_graph::STYLE_MAX * 7);
const index_type NL_P1I  = (index_graph::STYLE_MAX * 8);
const index_type NL_P2   = (index_graph::STYLE_MAX * 9);
const index_type NL_P2I  = (index_graph::STYLE_MAX * 10);

int main(int argc, char *argv[])
{
  GraphParser gp;
  gp.define_non_terminal(NL_A0, "A0");
  gp.define_non_terminal(NL_A1, "A1");
  gp.define_non_terminal(NL_A1I, "A1I");
  gp.define_non_terminal(NL_A2, "A2");
  gp.define_non_terminal(NL_PA1, "PA1");
  gp.define_non_terminal(NL_PA1I, "PA1I");
  gp.define_non_terminal(NL_P1, "P1");
  gp.define_non_terminal(NL_P1I, "P1I");
  gp.define_non_terminal(NL_P2, "P2");
  gp.define_non_terminal(NL_P2I, "P2I");

  gp.add_terminal_rule(NL_A0, TL_A, TL_P, TL_P, "tA0_A");
  gp.add_terminal_rule(NL_A1, TL_A, TL_P, TL_P, "tA1_A");
  gp.add_terminal_rule(NL_A2, TL_A, TL_P, TL_P, "tA2_A");

  gp.add_terminal_rule(NL_P1, TL_P, TL_A, TL_A, "tP1_P");
  gp.add_terminal_rule(NL_P1, TL_PG, TL_A, TL_A, "tP1_PG");
  gp.add_terminal_rule(NL_P2, TL_P, TL_A, TL_A, "tP2_P");
  gp.add_terminal_rule(NL_P2, TL_PG, TL_A, TL_A, "tP2_PG");
  gp.add_terminal_rule(NL_P1I, TL_P, TL_A, TL_A, "tP1I_P");
  gp.add_terminal_rule(NL_P1I, TL_PG, TL_A, TL_A, "tP1I_PG");
  gp.add_terminal_rule(NL_P2I, TL_P, TL_A, TL_A, "tP2I_P");
  gp.add_terminal_rule(NL_P2I, TL_PG, TL_A, TL_A, "tP2I_PG");
  gp.add_terminal_rule(NL_P1I, TL_PI, TL_A, TL_A, "tP1I_PI");
  gp.add_terminal_rule(NL_P1I, TL_PIG, TL_A, TL_A, "tP1I_PIG");
  gp.add_terminal_rule(NL_P2I, TL_PI, TL_A, TL_A, "tP2I_PI");
  gp.add_terminal_rule(NL_P2I, TL_PIG, TL_A, TL_A, "tP2I_PIG");

  // A0 rules:
  gp.add_non_terminal_rule(NL_A0, NL_A1, NL_P1I, true, true,
			   no_such_index, no_such_index,
			   no_such_index, no_such_index, "A0Synch1");
  gp.add_non_terminal_rule(NL_A0, NL_A1I, NL_P1, true, true,
			   no_such_index, no_such_index,
			   no_such_index, no_such_index, "A0Synch2");

  // A1/A1I rules:
  gp.add_non_terminal_rule(NL_A1, NL_A2, NL_P1I, true, true,
			   TL_P, TL_P,
			   no_such_index, no_such_index, "A1Synch");
  gp.add_non_terminal_rule(NL_A1, NL_A1, NL_PA1, true, false,
			   TL_P, no_such_index,
			   no_such_index, TL_P, "A1Seq");
  gp.add_non_terminal_rule(NL_A1I, NL_A1, NL_PA1I, true, false,
			   TL_P, no_such_index,
			   no_such_index, TL_P, "A1ISeq");

  // PA1/PA1I rules:
  gp.add_non_terminal_rule(NL_PA1, NL_P1, NL_A1, true, false,
			   TL_A, no_such_index,
			   no_such_index, TL_P, "PA1Seq");
  gp.add_non_terminal_rule(NL_PA1I, NL_P1I, NL_A1, true, false,
			   TL_A, no_such_index,
			   no_such_index, TL_P, "PA1ISeq1");
  gp.add_non_terminal_rule(NL_PA1I, NL_P1, NL_A1I, true, false,
			   TL_A, no_such_index,
			   no_such_index, TL_P, "PA1ISeq2");

  // P1/P1I rules:
  gp.add_non_terminal_rule(NL_P1, NL_P2, NL_A1, true, true,
			   TL_A, TL_A,
			   no_such_index, no_such_index, "P1Branch");
  gp.add_non_terminal_rule(NL_P1I, NL_P2I, NL_A1, true, true,
			   TL_A, TL_A,
			   no_such_index, no_such_index, "P1IBranch1");
  gp.add_non_terminal_rule(NL_P1I, NL_P2, NL_A1I, true, true,
			   TL_A, TL_A,
			   no_such_index, no_such_index, "P1IBranch2");

  sparse_mapping lc_in;
  lc_in.append(index_pair(TL_PG, TL_P));
  lc_in.append(index_pair(TL_PI, TL_P));
  lc_in.append(index_pair(TL_PIG, TL_P));
  lc_in.append(index_pair(NL_A0, TL_A));
  lc_in.append(index_pair(NL_A1, TL_A));
  lc_in.append(index_pair(NL_A1I, TL_A));
  lc_in.append(index_pair(NL_A2, TL_A));
  lc_in.append(index_pair(NL_PA1, TL_P));
  lc_in.append(index_pair(NL_PA1I, TL_P));
  lc_in.append(index_pair(NL_P1, TL_P));
  lc_in.append(index_pair(NL_P1I, TL_P));
  lc_in.append(index_pair(NL_P2, TL_P));
  lc_in.append(index_pair(NL_P2I, TL_P));
  sparse_mapping lc_out;
  lc_out.append(index_pair(TL_PG, TL_P));
  lc_out.append(index_pair(TL_PI, TL_P));
  lc_out.append(index_pair(TL_PIG, TL_P));
  lc_out.append(index_pair(NL_A0, TL_A));
  lc_out.append(index_pair(NL_A1, TL_A));
  lc_out.append(index_pair(NL_A1I, TL_A));
  lc_out.append(index_pair(NL_A2, TL_A));
  lc_out.append(index_pair(NL_PA1, TL_A));
  lc_out.append(index_pair(NL_PA1I, TL_A));
  lc_out.append(index_pair(NL_P1, TL_P));
  lc_out.append(index_pair(NL_P1I, TL_P));
  lc_out.append(index_pair(NL_P2, TL_P));
  lc_out.append(index_pair(NL_P2I, TL_P));
  gp.extend_context(lc_in, lc_out);

  gp.write_rules(std::cerr);
  // gp.write_rules_graph(std::cerr);

  LC_RNG rng;
  if (argc > 2)
    rng.seed(atoi(argv[2]));

  index_type n = 10;
  if (argc > 1)
    n = atoi(argv[1]);

  index_graph g;
  gp.random_graph(NL_A0, n, g, rng);

  std::cout << "digraph {" << std::endl;
  g.write_styled_digraph(std::cout, true, "cluster", 0);

  index_type root = gp.parse(g, NL_A0);
  if (root != no_such_index) {
    std::cerr << "parse successful" << std::endl;
    index_graph pt;
    gp.make_parse_tree(root, pt);
    lvector<std::string> rl("", pt.size());
    for (index_type k = 0; k < rl.size(); k++) {
      index_type ri = gp.matches[pt.node_label(k)].second.rule_id;
      if (gp.matches[pt.node_label(k)].second.is_terminal) {
	std::ostringstream pt_lk;
	pt_lk << gp.classers[ri].name
	      << " (" << gp.matches[pt.node_label(k)].first.second << ")";
	rl[k] = pt_lk.str();
      }
      else {
	rl[k] = gp.rules[ri].name;
      }
    }
    write_labeled_digraph< lvector<std::string> >(std::cout, pt, rl, false, "Parse Tree", 1000);
//     index_set lrm;
//     lrm.insert(NL_PA1);
//     lrm.insert(NL_PA1I);
//     gp.compact_parse_tree(pt, lrm);
//     rl.assign_value("", pt.size());
//     for (index_type k = 0; k < rl.size(); k++) {
//       index_type ri = gp.matches[pt.node_label(k)].second.rule_id;
//       if (gp.matches[pt.node_label(k)].second.is_terminal) {
// 	std::ostringstream pt_lk;
// 	pt_lk << gp.classers[ri].name
// 	      << " (" << gp.matches[pt.node_label(k)].first.second << ")";
// 	rl[k] = pt_lk.str();
//       }
//       else {
// 	rl[k] = gp.rules[ri].name;
//       }
//     }
//     write_labeled_digraph< lvector<std::string> >(std::cout, pt, rl, false, "Compacted Parse Tree", 2000);
  }
  else {
    std::cerr << "parse failed" << std::endl;
    for (index_type k = 0; k < gp.matches.length(); k++) {
      std::cerr << "(" << k << ") ";
      gp.write_entry(std::cerr, gp.matches[k]);
      std::cerr << std::endl;
    }
  }
  std::cout << "}" << std::endl;

  return 0;
}

END_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
