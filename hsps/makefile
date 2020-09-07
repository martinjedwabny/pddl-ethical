
include makedefs

.SUFFIXES : .cc $(SUFFIXES)

.cc.o:
	$(CCC) $(CFLAGS) -c $*.cc

.c.o:
	$(CC) $(CFLAGS) -c $*.c

%.d: %.cc
	$(CCDEP) $(CFLAGS) $< > $@

LIBGENSRCS = grammar.cc scanner.cc
LIBNGSRCS = rational.cc index_type.cc name.cc ptr_table.cc char_map.cc \
 string_table.cc numeric_type.cc graph.cc hypergraph.cc enumerators.cc \
 rng.cc problem.cc search.cc heuristic.cc stats.cc exec.cc plans.cc \
 preprocess.cc cost_table.cc atomset.cc resource.cc seq_reg.cc para_reg.cc \
 temporal.cc forward.cc search_base.cc hashtable.cc ida.cc idao.cc \
 minimax.cc itest.cc nodeset.cc bfs.cc bb.cc bfhs.cc collector.cc base.cc \
 parser.cc reduce.cc random.cc explore.cc sas.cc sas_heuristic.cc pdb.cc \
 pdb_construction.cc simplify.cc tcn.cc pop.cc htn.cc soft.cc relaxed.cc \
 logic.cc mdd.cc path.cc
LIBSRCS = $(LIBNGSRCS) $(LIBGENSRCS)
SUPSRCS = graph_grammar.cc
GENSRCS = pddl2.y pddl2.lex
PRGSRCS = pddlcat.cc hsp0.cc hsp_f.cc hsp_a.cc hsp_b.cc hsp_c.cc d_hsp.cc \
 rpgc.cc hsp_p.cc
EMPRGSRCS = hsp_e.cc hsp_h.cc test_apx.cc test_misc.cc strips.cc \
 convert_cost_to_preference.cc create_nb_problems.cc test_nbb.cc \
 test_ppc.cc test_sd.cc analyze_deadlines.cc random_sas.cc \
 roverMSP_to_TSP.cc findprime.cc rnd.cc sas_us_plan.cc
NMPRGSRCS = check_ra.cc sample.cc test_gg.cc ggp.cc
EXTRASRCS = locals/config.h function.h chopshop.c
DOCS = hsps.html
UTILS = locals/fake-flex++ locals/fake-bison++ locals/scanner.*.h \
 locals/scanner.*.cc locals/grammar.*.h locals/grammar.*.cc \
 plan-to-xhtml.xsl

all: libhsps.a $(PRGSRCS:.cc=$(EXEEXT))
extra: $(EMPRGSRCS:.cc=$(EXEEXT))

grammar.h: pddl2.y base.h
	$(BISON) -v -d -o grammar.cc pddl2.y
grammar.cc: pddl2.y base.h
	$(BISON) -v -d -o grammar.cc pddl2.y
scanner.cc: pddl2.lex grammar.h
	$(FLEX) -i -hscanner.h -oscanner.cc pddl2.lex
scanner.h: pddl2.lex grammar.h
	$(FLEX) -i -hscanner.h -oscanner.cc pddl2.lex


libhsps.a: $(LIBSRCS:.cc=.o)
	ar cr libhsps.a $(LIBSRCS:.cc=.o)
	ranlib libhsps.a

pddlcat$(EXEEXT): pddlcat.o libhsps.a
	$(LINK) $(LFLAGS) pddlcat.o -lhsps $(LLIBS) -o pddlcat$(EXEEXT)

hsp0$(EXEEXT): hsp0.o libhsps.a
	$(LINK) $(LFLAGS) hsp0.o -lhsps $(LLIBS) -o hsp0$(EXEEXT)

hsp_a$(EXEEXT): hsp_a.o libhsps.a
	$(LINK) $(LFLAGS) hsp_a.o -lhsps $(LLIBS) -o hsp_a$(EXEEXT)

hsp_b$(EXEEXT): hsp_b.o libhsps.a
	$(LINK) $(LFLAGS) hsp_b.o -lhsps $(LLIBS) -o hsp_b$(EXEEXT)

hsp_c$(EXEEXT): hsp_c.o libhsps.a
	$(LINK) $(LFLAGS) hsp_c.o -lhsps $(LLIBS) -o hsp_c$(EXEEXT)

d_hsp$(EXEEXT): d_hsp.o libhsps.a
	$(LINK) $(LFLAGS) d_hsp.o -lhsps $(LLIBS) -o d_hsp$(EXEEXT)

hsp_f$(EXEEXT): hsp_f.o libhsps.a
	$(LINK) $(LFLAGS) hsp_f.o -lhsps $(LLIBS) -o hsp_f$(EXEEXT)

hsp_h$(EXEEXT): hsp_h.o libhsps.a
	$(LINK) $(LFLAGS) hsp_h.o -lhsps $(LLIBS) -o hsp_h$(EXEEXT)

hsp_e$(EXEEXT): hsp_e.o libhsps.a
	$(LINK) $(LFLAGS) hsp_e.o -lhsps $(LLIBS) -o hsp_e$(EXEEXT)

hsp_g$(EXEEXT): hsp_g.o libhsps.a
	$(LINK) $(LFLAGS) hsp_g.o -lhsps $(LLIBS) -o hsp_g$(EXEEXT)

hsp_p$(EXEEXT): hsp_p.o libhsps.a
	$(LINK) $(LFLAGS) hsp_p.o -lhsps $(LLIBS) -o hsp_p$(EXEEXT)

rpgc$(EXEEXT): rpgc.o libhsps.a
	$(LINK) $(LFLAGS) rpgc.o -lhsps $(LLIBS) -o rpgc$(EXEEXT)

chopshop$(EXEEXT): chopshop.o
	$(LINK) $(LFLAGS) chopshop.o $(LLIBS) -o chopshop$(EXEEXT)

create_nb_problems$(EXEEXT): create_nb_problems.o libhsps.a
	$(LINK) $(LFLAGS) create_nb_problems.o -lhsps $(LLIBS) -o create_nb_problems$(EXEEXT)

test_nbb$(EXEEXT): test_nbb.o libhsps.a
	$(LINK) $(LFLAGS) test_nbb.o -lhsps $(LLIBS) -o test_nbb$(EXEEXT)

test_ppc$(EXEEXT): test_ppc.o libhsps.a
	$(LINK) $(LFLAGS) test_ppc.o -lhsps $(LLIBS) -o test_ppc$(EXEEXT)

test_sd$(EXEEXT): test_sd.o libhsps.a
	$(LINK) $(LFLAGS) test_sd.o -lhsps $(LLIBS) -o test_sd$(EXEEXT)

analyze_deadlines$(EXEEXT): analyze_deadlines.o libhsps.a
	$(LINK) $(LFLAGS) analyze_deadlines.o -lhsps $(LLIBS) -o analyze_deadlines$(EXEEXT)

convert_cost_to_preference$(EXEEXT): convert_cost_to_preference.o libhsps.a
	$(LINK) $(LFLAGS) convert_cost_to_preference.o -lhsps $(LLIBS) -o convert_cost_to_preference$(EXEEXT)

random_sas$(EXEEXT): random_sas.o libhsps.a
	$(LINK) $(LFLAGS) random_sas.o -lhsps $(LLIBS) -o random_sas$(EXEEXT)

roverMSP_to_TSP$(EXEEXT): roverMSP_to_TSP.o libhsps.a
	$(LINK) $(LFLAGS) roverMSP_to_TSP.o -lhsps $(LLIBS) -o roverMSP_to_TSP$(EXEEXT)

sas_us_plan$(EXEEXT): sas_us_plan.o libhsps.a
	$(LINK) $(LFLAGS) sas_us_plan.o -lhsps $(LLIBS) -o sas_us_plan$(EXEEXT)

# create sample_current_config.cc by copying if necessary
sample_current_config.cc: sample.cc
	cp -f sample.cc sample_current_config.cc

sample$(EXEEXT): sample_current_config.o libhsps.a
	$(LINK) $(LFLAGS) sample_current_config.o -lhsps $(LLIBS) -o sample$(EXEEXT)

# misc. test programs

check_ra$(EXEEXT): check_ra.o rational.o
	$(LINK) $(LFLAGS) check_ra.o rational.o $(LLIBS) -o check_ra$(EXEEXT)

test_misc$(EXEEXT): test_misc.o libhsps.a
	$(LINK) $(LFLAGS) test_misc.o -lhsps $(LLIBS) -o test_misc$(EXEEXT)

test_apx$(EXEEXT): test_apx.o libhsps.a
	$(LINK) $(LFLAGS) test_apx.o -lhsps $(LLIBS) -o test_apx$(EXEEXT)

strips$(EXEEXT): strips.o libhsps.a
	$(LINK) $(LFLAGS) strips.o -lhsps $(LLIBS) -o strips$(EXEEXT)

findprime$(EXEEXT): findprime.o libhsps.a
	$(LINK) $(LFLAGS) findprime.o -lhsps $(LLIBS) -o findprime$(EXEEXT)

rnd$(EXEEXT): rnd.o libhsps.a
	$(LINK) $(LFLAGS) rnd.o -lhsps $(LLIBS) -o rnd$(EXEEXT)

ggp$(EXEEXT): ggp.o graph_grammar.o libhsps.a
	$(LINK) $(LFLAGS) ggp.o graph_grammar.o -lhsps $(LLIBS) -o ggp$(EXEEXT)

test_gg$(EXEEXT): test_gg.o graph_grammar.o libhsps.a
	$(LINK) $(LFLAGS) test_gg.o graph_grammar.o -lhsps $(LLIBS) -o test_gg$(EXEEXT)

# admin targets

deps: grammar.cc scanner.cc
	rm -f makedeps
	$(CCC) -MM $(LIBSRCS) $(PRGSRCS) $(EMPRGSRCS) $(NMPRGSRCS) > makedeps

clean:
	rm -f *.d *.o *.a $(PRGSRCS:.cc=$(EXEEXT)) $(EMPRGSRCS:.cc=$(EXEEXT)) $(NMPRGSRCS:.cc=$(EXEEXT)) chopshop$(EXEEXT)

distclean:
	make clean
	(cd pddl3sim; make clean)
	(cd hspIAO; make clean)

remake: clean
	rm -f grammar.* scanner.*

arc:
	rm -f hsps.tar hsps.tar.gz
	tar cvf hsps.tar $(LIBSRCS:.cc=.h) $(LIBSRCS) $(PRGSRCS) $(EMPRGSRCS) \
 $(NMPRGSRCS) $(EXTRASRCS) $(GENSRCS) makefile
	gzip hsps.tar

unarc: hsps.tar.gz
	gzip -d hsps.tar.gz
	tar xvf hsps.tar
	rm -f hsps.tar

dist:
	tar cvf hsps.tar $(LIBSRCS:.cc=.h) $(LIBSRCS) $(PRGSRCS) $(EMPRGSRCS) \
 $(NMPRGSRCS) $(EXTRASRCS) $(GENSRCS) $(DOCS) locals/makedefs $(UTILS) \
 makefile
	gzip hsps.tar


# experimental stuff

lgrammar.h: lisp.y lisp.h
	$(BISON) -v -d -o lgrammar.cc lisp.y
lgrammar.cc: lisp.y lisp.h
	$(BISON) -v -d -o lgrammar.cc lisp.y
lscanner.h: lisp.lex lgrammar.h
	$(FLEX) -i -hlscanner.h -olscanner.cc lisp.lex
lscanner.cc: lisp.lex lgrammar.h
	$(FLEX) -i -hlscanner.h -olscanner.cc lisp.lex

lscanner.o: lscanner.cc lscanner.h lisp.h input.h
lgrammar.o: lgrammar.cc lgrammar.h lisp.h input.h
lisp.o: lisp.cc lisp.h input.h
input.o: input.cc input.h
new_test.o: new_test.cc lscanner.h lgrammar.h lisp.h input.h
new_test: new_test.o input.o lisp.o lgrammar.o lscanner.o
	$(LINK) $(LFLAGS) new_test.o lscanner.o lgrammar.o lisp.o input.o -lhsps $(LLIBS) -o new_test$(EXEEXT)

-include $(EMPRGSRCS:%.cc=%.d)
-include $(NMPRGSRCS:%.cc=%.d)
-include $(PRGSRCS:%.cc=%.d)
-include $(LIBNGSRCS:%.cc=%.d)
-include $(LIBGENSRCS:%.cc=%.d)
