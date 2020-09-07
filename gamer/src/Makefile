# ***********************************************************
# **

####### Files
# CC = g++ -I ../include


FINALOBJECT = mips.o

PARSEROBJECTS = lisp.entity.o lisp.scanner.o lisp.parser.o

UTILOBJECTS = util.options.o util.tools.o  util.bitarray.o 

DATAOBJECTS = data.domain.o data.predicate.o \
	data.symbolicFact.o data.instantiation.o \
	data.action.o data.formula.o
	# \

INTERFACEOBJECTS = out.factMap.o out.operator.o out.group.o  

BDDOBJECTS = single.state.o single.search.o bdd.bfs.o bdd.bnb.o bdd.solution.o bdd.search.o bdd.dijkstra.o bdd.arithmetics.o

# AR = ar -cr

OBJECTS = $(INTERFACEOBJECTS) $(FINALOBJECT) $(UTILOBJECTS) $(PARSEROBJECTS) $(DATAOBJECTS) $(BDDOBJECTS) 

####### top-level targets

mips: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS)  -o ../bdd-solver -L ../lib -lbdd  

libs: $(OBJECTS) 
	$(AR) ../lib/libParser.a $(PARSEROBJECTS) $(BinDependencies) 
	$(AR) ../lib/libData.a $(DATAOBJECTS) $(BinDependencies) 
	$(AR) ../lib/libUtil.a $(UTILOBJECTS) $(BinDependencies)
	$(AR) ../lib/libInter.a $(INTERFACEOBJECTS) $(BinDependencies)
	$(AR) ../lib/libRealTime.a $(REALTIMEOBJECTS) $(BinDependencies)
	$(AR) ../lib/libSymb.a $(BDDOBJECTS) $(BinDependencies)
	$(AR) ../lib/libSingle.a $(SINGLEOBJECTS) $(BinDependencies)

depend:
	$(CC) -MM *.cc > Makefile.inc
clean:
	rm -f $(OBJECTS) *~ Makefile.inc 

backup:
	-rm mips.zip
	tar -zcf mips.tgz *.pddl src/*.cc include/*.h Makefile 

####### pattern rules

$(OBJECTS): %.o: %.cc
	$(CC) $(CFLAGS) -c $< -o $@

Makefile.inc:
	$(CC) $(CFLAGS) -MM *.cc > Makefile.inc

include Makefile.inc





