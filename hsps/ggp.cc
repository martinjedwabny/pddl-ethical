
#include "parser.h"
#include "preprocess.h"
#include "graph_grammar.h"
#include <sstream>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  HSPS::Statistics stats;

  HSPS::StringTable symbols(50, HSPS::lowercase_map);
  HSPS::Parser* reader = new HSPS::Parser(symbols);
  HSPS::GG1 gp;

  if (argc == 1) {
    gp.write_rules_graph(std::cout);
    exit(0);
  }

  HSPS::index_type n_files_read = 0;
  HSPS::index_vec numeric_args;
  for (HSPS::index_type k = 1; k < argc; k++) {
    char* endp;
    HSPS::index_type i = strtoul(argv[k], &endp, 10);
    if (endp == (argv[k] + strlen(argv[k]))) {
      numeric_args.append(i);
    }
    else {
      std::cerr << "reading " << argv[k] << "..." << std::endl;
      reader->read(argv[k], false);
      n_files_read += 1;
    }
  }

  if (numeric_args.length() < 1)
    numeric_args.append(10);
  if (numeric_args.length() < 2)
    numeric_args.append(31337);

  std::cout << "digraph Test {" << std::endl;

  HSPS::index_graph g;

  if (n_files_read == 0) {
    HSPS::LC_RNG rng;
    rng.seed(numeric_args[1]);
    gp.random_graph(HSPS::GG1::NL_A0, numeric_args[0], g, rng);
    g.write_styled_digraph(std::cout, true, "Random Graph", 0);
  }
  else {
    HSPS::Instance   instance;
    HSPS::Preprocessor prep(instance, stats);

    std::cerr << "instantiating..." << std::endl;
    reader->instantiate(instance);

    std::cerr << "preprocessing..." << std::endl;
    prep.preprocess();
    prep.compute_irrelevant_atoms();
    prep.remove_irrelevant_atoms();
    if (!instance.cross_referenced())
      instance.cross_reference();

    HSPS::name_vec  gnn;
    instance.make_graph_representation(g, gnn);

    for (HSPS::index_type i = 0; i < g.size(); i++)
      for (HSPS::index_type j = 0; j < g.size(); j++)
	if (g.adjacent(i, j)) {
	  if (g.edge_label(i, j) == (HSPS::index_graph::ES_BOLD +
				     HSPS::index_graph::ED_NONE)) {
	    std::cerr << "this problem has non-pre deletes, can not solve it"
		      << std::endl;
	    exit(1);
	  }
	  else if (g.edge_label(i, j) == (HSPS::index_graph::ES_DASHED +
					  HSPS::index_graph::ED_FORWARD)) {
	    g.edge_label(i, j) =
	      (HSPS::index_graph::ES_NORMAL + HSPS::index_graph::ED_FORWARD);
	    g.add_edge(j, i, (HSPS::index_graph::ES_NORMAL +
			      HSPS::index_graph::ED_FORWARD));
	  }
	}
    HSPS::write_styled_digraph<HSPS::name_vec>
      (std::cout, g, gnn, true, "Input Graph", 0);
  }

  std::cerr << "parsing..." << std::endl;
  HSPS::index_type root = gp.parse(g, HSPS::GG1::NL_A0);

  if (root == HSPS::no_such_index) {
    std::cerr << "parse failed, can not solve this problem"
	      << std::endl;
    std::cerr << "#matches = " << gp.matches.length() << std::endl;
    for (HSPS::index_type k = 0; k < gp.matches.length(); k++) {
      std::cerr << "(" << k << ") ";
      gp.write_entry(std::cerr, gp.matches[k]);
      std::cerr << std::endl;
    }
    exit(1);
  }

  HSPS::index_graph pt;
  gp.make_parse_tree(root, pt);
  // pt.write_digraph(std::cerr, true, true, true, false, "pt");
  HSPS::index_graph ptc(pt);
  gp.compact_parse_tree(ptc);
  // ptc.write_digraph(std::cerr, true, true, true, false, "ptc");
  HSPS::index_graph ptc_unreversed(ptc);

#ifdef PRINT_INTERMEDIATE_PARSE_TREE
  HSPS::lvector<std::string> rl("", pt.size());
  for (HSPS::index_type k = 0; k < rl.size(); k++) {
    HSPS::index_type ri = gp.matches[pt.node_label(k)].second.rule_id;
    std::ostringstream ls;
    if (pt.successors(k).length() > 0) {
      // assert(pt.successors(k).length() == 1);
      HSPS::index_type n = pt.edge_label(k, pt.successors(k)[0]);
      ls << "N" << n << ". ";
    }
    if (gp.matches[pt.node_label(k)].second.is_terminal) {
      assert(gp.matches[pt.node_label(k)].first.second.length() == 1);
      ls << gp.matches[pt.node_label(k)].first.second[0]
	 << " (" << gp.classers[ri].name << ")";
    }
    else {
      ls << gp.rules[ri].name;
    }
    rl[k] = ls.str();
  }
  pt.reverse();
  HSPS::write_labeled_digraph< HSPS::lvector<std::string> >
    (std::cout, pt, rl, false, "Parse Tree", 1000);
  rl.assign_value("", ptc.size());
#else
  HSPS::lvector<std::string> rl("", ptc.size());
#endif

  for (HSPS::index_type k = 0; k < ptc.size(); k++) {
    HSPS::index_type ri = gp.matches[ptc.node_label(k)].second.rule_id;
    std::ostringstream ls;
    if (ptc.successors(k).length() > 0) {
      assert(ptc.successors(k).length() == 1);
      HSPS::index_type n = ptc.edge_label(k, ptc.successors(k)[0]);
      ls << "N" << n << ". ";
    }
    if (gp.matches[ptc.node_label(k)].second.is_terminal) {
      assert(gp.matches[ptc.node_label(k)].first.second.length() == 1);
      ls << gp.matches[ptc.node_label(k)].first.second[0]
	 << " (" << gp.classers[ri].name << ")";
    }
    else {
      ls << gp.rules[ri].name;
    }
    rl[k] = ls.str();
  }

  ptc.reverse();
  HSPS::write_labeled_digraph< HSPS::lvector<std::string> >
    (std::cout, ptc, rl, false, "Compacted Parse Tree", 2000);

  std::cout << "}" << std::endl;

  std::cout << "/*" << std::endl;
  gp.write_decomposition(std::cout, g, ptc_unreversed);
  std::cout << "*/" << std::endl;

  return 0;
}
