########################################
#   Please make sure that you have set the 
#   appropriate path for "flex", "byacc" and c++ compiler.
#
#   options:
#   1) "make" or "make dino"  
#      --> compile dino
#   2) "make clean"
#      --> remove all intermediate files
#   3) "make cleanall"
#      --> remove all intermediate files, including byacc and flex ones.
#
########################################

# choice of compilers
CPLUSPLUS = g++
FLEX = flex
BYACC = byacc

# compiler options
CFLAGS = -g -O2 -DYYMAXDEPTH=25000       # options for g++

#must end with a slash
BIN_DIR = bin/ 
INCLUDE_DIR = include/

#model compilation settings
COMPILER_DEFINES = -DNO_RUN_TIME_CHECKING -DCATCH_DIV -DSYSCONF_RDCL -D_FILE_OFFSET_BITS=64 
COMPILER_SWITCHES = -m32 -w -O4
PDDL_PARSER_NAME = pddl2upm
DINO_DEFINES = -DCOMPILER_NAME=$(CPLUSPLUS) -DCOMPILER_DEFINES="$(COMPILER_DEFINES)" -DCOMPILER_SWITCHES="$(COMPILER_SWITCHES)" -DPDDL_PARSER_NAME=$(PDDL_PARSER_NAME) 

#uncomment and write the correct validator path to enable automatic plan validation
VAL_DIR:=$(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))
VAL_PATHNAME = $(VAL_DIR)/../VAL-master/validate
VAL_DEFINES = -DVAL_PATHNAME="$(VAL_PATHNAME)"

OBJS = \
	lex.yy.o \
	y.tab.o \
	cpp_code.o \
	cpp_code_as.o \
  	cpp_sym.o \
  	cpp_sym_aux.o \
  	cpp_sym_decl.o \
	decl.o \
	error.o \
	expr.o \
	lextable.o \
	mu.o \
	parse.o \
	rule.o \
	stmt.o \
	symtab.o \
	util.o

dino: $(OBJS)
	$(CPLUSPLUS) -g -o dino $(CFLAGS) $(OBJS)
	mv dino ../../$(BIN_DIR)

mu.o: mu.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) $(DINO_DEFINES) $(VAL_DEFINES) mu.cpp

cpp_code.o: cpp_code.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -g -c $(CFLAGS) $(DINO_DEFINES) $(VAL_DEFINES) cpp_code.cpp

cpp_code_as.o: cpp_code_as.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) cpp_code_as.cpp

cpp_sym.o: cpp_sym.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) cpp_sym.cpp

cpp_sym_aux.o: cpp_sym_aux.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) cpp_sym_aux.cpp

cpp_sym_decl.o: cpp_sym_decl.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) cpp_sym_decl.cpp

decl.o: decl.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) decl.cpp

error.o: error.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) error.cpp

expr.o: expr.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) expr.cpp

lextable.o: lextable.cpp mu.hpp lextable.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) lextable.cpp

rule.o: rule.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) rule.cpp

stmt.o: stmt.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) stmt.cpp

symtab.o: symtab.cpp mu.hpp lextable.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) symtab.cpp

util.o: util.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) util.cpp

parse.o: parse.cpp mu.hpp y.tab.h
	$(CPLUSPLUS) -c $(CFLAGS) parse.cpp

lex.yy.o: lex.yy.c mu.hpp lextable.hpp
	$(CPLUSPLUS) -c $(CFLAGS) lex.yy.c

lex.yy.c: mu.l mu.hpp y.tab.h
	$(FLEX) mu.l

y.tab.o: y.tab.c mu.hpp y.tab.h
	$(CPLUSPLUS) -c -DYYDEBUG=0 -DYYMAXDEPTH=25000 $(CFLAGS) y.tab.c

y.tab.c + y.tab.h: mu.y
	$(BYACC) -dt mu.y

clean:
	rm -f  *.o

cleanall: clean
	rm -f  y.* lex.yy.c dino

