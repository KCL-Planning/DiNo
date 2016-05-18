
# choice of compilers
CPLUSPLUS = g++
FLEX = flex++
#FLEX = flex-+
BYACC = bison

# choice of the options
CFLAGS = -g -O2 -ggdb -DYYMAXDEPTH=25000    # options for g++

#must end with a slash
PDDL_PARSER_DIR = bin/ 

SRCS = 	pddl+.cpp \
	parse.cpp \
	ptree.cpp
  	
OBJS = 	pddl+.o \
	parse.o \
	ptree.o
	
pddl2upm: pddl+.o parse.o ptree.o
	$(CPLUSPLUS) -g $(OBJS) -o pddl2upm  $(CFLAGS)
	mv pddl2upm ../../$(PDDL_PARSER_DIR)

pddl+.o: pddl+.cpp lex.yy.cc
	$(CPLUSPLUS) -c pddl+.cpp  $(CFLAGS)

parse.o: parse.cpp
	$(CPLUSPLUS) -c parse.cpp  $(CFLAGS)

ptree.o: ptree.cpp
	$(CPLUSPLUS) -c ptree.cpp  $(CFLAGS)

lex.yy.cc: pddl+.lex
	$(FLEX) pddl+.lex

pddl+.cpp: pddl+.yacc
	$(BYACC) pddl+.yacc -o pddl+.cpp

clean:
	rm -f  *.o

cleanall: clean
	rm -f  pddl+.cpp lex.yy.c pddl2upm

