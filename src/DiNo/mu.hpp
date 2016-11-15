/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#ifndef __MU_H__
#define __MU_H__

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define DINO_VERSION "DiNo Release 1.1"
#define MURPHI_VERSION "Universal Planner Murphi Release 3.0"

#ifndef __DATE__		// To accomodate DEC's CC
#define __DATE__ ""
#endif

#define QUOTED(arg) #arg
#define QUOTED_VALUE(arg) QUOTED(arg)

#define MAX_NO_BITS 1024	// maximum number of bits in a single state
#define MAX_NO_RULES 1024	// maximum number of rules in the description
#define BUFFER_SIZE 1024	// maximum size of temporary buffer in byte
#define MAX_SCALARSET_PERM 600000	// maximum number of permutations for explicit storage

#define MAXSCOPES 64		/* number of saved scopes. */
#define SYMTABSIZE 211		/* should be a prime. */

/* boolean constants, kept for compatibility */
#define TRUE true
#define FALSE false

#define RULE_INDEX_TYPE unsigned long //MUST BE same as in upm_prolog.hpp

#ifndef COMPILER_NAME
#define COMPILER_NAME g++
#endif
#ifndef COMPILER_DEFINES
#define COMPILER_DEFINES -DNO_RUN_TIME_CHECKING -DCATCH_DIV -DSYSCONF_RDCL
#endif
#ifndef COMPILER_SWITCHES
#define COMPILER_SWITCHES -m32 -O4
#endif
#ifndef PDDL_PARSER_NAME
#define PDDL_PARSER_NAME pddl2upm
#endif

#define DISCRETIZATION_CONST_NAME T
#define DEF_DISCRETIZATION 0.1

#define CLOCK_VAR_NAME TIME

/*---------------------------------------------------------------------------*/
/* Include file headers                                                      */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>
#include "y.tab.h"

/*---------------------------------------------------------------------------*/
/* class TNode                                                               */
/*---------------------------------------------------------------------------*/
class TNode
{
 public:
  TNode()
  {
  };
  virtual const char *generate_code(void)
  {
    return NULL;
  };
};

class decl;
class typedecl;
class expr;
class designator;
class stmt;
class rule;

/*---------------------------------------------------------------------------*/
/* class Error_handler                                                       */
/*---------------------------------------------------------------------------*/
class Error_handler		/* error-reporting mechanism. */
{
  int numerrors;
  int numwarnings;

 private:
  void vError(const char *fmt, va_list argp);
  void vWarning(const char *fmt, va_list argp);

 public:
  // initializer
  Error_handler(void);

  // These next five each take printf-like arguments.
  void Error(const char *fmt, ...);
  // print out message and record an error.
  void FatalError(const char *fmt, ...);
  // print out message and quit.
  bool CondError(const bool test, const char *fmt, ...);
  // if test is true, print strings and record error. Return test.
  void Warning(const char *fmt, ...);
  // print a warning.
  bool CondWarning(const bool test, const char *fmt, ...);
  // if test is true, print strings and record warning. Return test.

  // supporting routines
  int getnumerrors(void)
  {
    return numerrors;
  };
  int getnumwarnings(void)
  {
    return numwarnings;
  };

};

/*---------------------------------------------------------------------------*/
/* class argclass                                                            */
/*---------------------------------------------------------------------------*/
class argclass
{
  int argc;
  char **argv;

 public:
  // variables
  char *m_filename;		// name of the input model file
  char *pddl_domain_filename;		// name of the input domain file
  char *pddl_problem_filename;		// name of the input problem file
  bool print_license;		// whether to print license
  bool help;			// whether to print option
  bool no_compression;		// whether not to bitpack the state vector
  bool hash_compression;	// whether not to hashcompact states in hash tbl
  //UPMURPHI_BEGIN
  bool UPMurphi_planner;     // planner
  bool UPMurphi_disk;		// disk extensions
  bool keep_source;		// keep source after compilation
  bool compile_source;		// compile executable
  bool compile_pddl;		// compile pddl
  bool force_recompile;		// force recompilation
  bool dynamic_debug;		// create debug-mode (step-execution) planner
  bool variable_weight;		//use extensions for state-dependant rule weights
  bool variable_duration;	//use extensions for state-dependant rule durations
  bool pddl_parser_prompt;  //pass --prompt to pddl parser
  bool warnings;  			//show warnings
  double pddl_parser_set_1,pddl_parser_set_2,pddl_parser_set_3;     //pass --custom X Y Z to pddl parser
  //UPMURPHI_END
  const bool checking;		/* runtime checking? */
  bool remove_deadrule;		// suppress code generation for dead rules
  int symmetry_algorithm_number;	// symmetry algorithm number

  // initializer
  argclass(int ac, char **av);

  // supporting routines
  char *NextFile();		// ??
  bool Flag(char *arg);		// ??
  void PrintInfo(void);
  void PrintOptions(void);
  void PrintLicense(void);
};

/*---------------------------------------------------------------------------*/
/* Auxiliary functions                                                       */
/*---------------------------------------------------------------------------*/
int CeilLog2(int n);
/* number of bits required to hold n values. */

char *tsprintf(const char *fmt, ...);
/* sprintf's the arguments into dynamically allocated memory.  Returns the
 * dynamically allocated string. */

int new_int();
/* returns a different integer each time. Used for creating distinct names.*/

/*---------------------------------------------------------------------------*/
/* class lexid                                                               */
/*---------------------------------------------------------------------------*/
class lexid
{
  char *name;
  int lextype;
  struct {
    char *pddlname;
  } pddlattrs;
 public:
  lexid(const char *name, int lextype);
  char *getname() const
  {
    return name;
  };
  int getlextype() const
  {
    return lextype;
  };
  void setpddlname(const char *name);
  char *getpddlname() const
  {
    return pddlattrs.pddlname;
  };
};


struct lexlist {
  lexid *l;
  lexlist *next;
  lexlist(lexid * l, lexlist * next)
    :l(l), next(next)
  {
  }
};

/*---------------------------------------------------------------------------*/
/* Symbol table stuff.
   -- The symbol table needs to be a persistent structure; the processing
   -- stage needs to be able to find symbols to access names.  However,
   -- when we're in that stage, we'll have a pointer to each symbol
   -- we need, so finding symbols is not such a problem.
   -- The current implementation uses a hash table to find identifiers,
   -- with each bucket a linked list of symbol table entries ( ste's ).      */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* struct ste
  -- a symbol table entry.
  -- Note: since we use lexids instead of char *'s for names, we can
  -- use pointer comparison to check for equality, and if we decided to move
  -- to a different conception of type sensitivity, all we have to do
  -- is make sure that lextable::enter() maps words that are considered
  -- equivalent to the same lexid.                                           */
/*---------------------------------------------------------------------------*/
struct ste {
  friend class symboltable;

  // basic data
  lexid *name;			/* name of object. */
  int scope;			/* scope in which it's declared. */
  decl *value;			/* thing associated with name. */

  // linkage in lists
  ste *next;			/* previous thing declared. */

 public:
  // basic interface
  ste(lexid * name, int scope, decl * value);
  void setname(lexid * name)
  {
    this->name = name;
  };
  lexid *getname() const
  {
    return name;
  };
  int getscope() const
  {
    return scope;
  };
  decl *getvalue()
  {
    return value;
  };

  // list interface
  ste *getnext() const
  {
    return next;
  };
  void setnext(ste * next)
  {
    this->next = next;
  };
  ste *reverse();		/* reverse a list of ste's. */

  // search a list of ste's for one with lexid name.
  ste *search(lexid * name);

  // code generation
  // generate's declarations for all the things in a list of ste's,
  // in reverse order.
  const char *generate_decls();
};

/*---------------------------------------------------------------------------*/
/* class symboltable                                                         */
/*---------------------------------------------------------------------------*/
class symboltable
{
 private:
  // variables
  static const int globalscope;
  int scopes[MAXSCOPES];	/* stack of numbers of currently active scopes. */
  ste *scopestack[MAXSCOPES];	/* stack of top element of active scopes. */
  int offsets[MAXSCOPES];	/* Saved variable offsets. */
  int curscope;			/* integer number of scopes created. */
  int scopedepth;		/* index of highest scope in use. */


 public:
  // initializer
  symboltable();

  // supportine routines
  int getscopedepth() const
  {
    return scopedepth;
  };
  ste *find(lexid * name) const; //gdp made public
  /* return the ste corresponding to name
   in current table, or NULL if none. */
  ste *lookup(lexid * name);
  /* find ste corresponding to name in current scope. */
  /* if not there, report error and declare as error_decl. */
  ste *declare(lexid * name, decl * value);
  /* associate lexid with object value. If already declared in scope,
   * report error and do nothing. */
  ste *declare_global(lexid * name, decl * value);
  /* declare in global scope. */
  int pushscope();
  /* push a new scope and return its number. */
  ste *popscope(bool cut = TRUE);
  /* pop a scope, returning the list of ste's popped.
   * IF cut is true, clear the last link in the returned scope. */
  ste *getscope() const;
  /* return to a pointer to the linked list of ste's
   * at the top of the stack. */
  ste *dupscope() const;
  /* return a copy of the ste's in the top scope. */
  ste *topste() const
  {
    return scopestack[scopedepth];
  };
  int topscope() const
  {
    return scopes[scopedepth];
  };
  int getcurscope() const
  {
    return curscope;
  };
};

/*---------------------------------------------------------------------------*/
/* Extern variables                                                          */
/*---------------------------------------------------------------------------*/
extern TNode error_obj;		/* created for error handling. */
extern Error_handler Error;
extern argclass *args;
extern char *gFileName;		/* file support. */
extern FILE *codefile;		/* name of the file currently being compiled. */
extern symboltable *symtab;

struct stelist {
  ste *s;
  stelist *next;
 public:
  stelist(ste * s, stelist * next)
    :s(s), next(next)
  {
  }
};

class stecoll
{
  stelist *first;
  stelist *last;
 public:
  stecoll(ste * s)
  {
    first = last = new stelist(s, NULL);
  } stecoll()
  {
    first = last = NULL;
  }

  void append(stecoll * sc)
  {
    if (last) {
      // This collection is not empty
      if (sc->last) {
        // sc is not empty
        last->next = sc->first;
        last = sc->last;
      }
    } else {
      // This collection is empty
      first = sc->first;
      last = sc->last;
    }
    delete sc;
  }

  bool includes(ste * s)
  {
    stelist *p = first;
    while (p) {
      if (p->s == s)
        return TRUE;
      p = p->next;
    }
    return FALSE;
  }

  void output()
  {
    stelist *p = first;
    while (p) {
      printf("%s ", p->s->getname()->getname());
      p = p->next;
    }
    printf("\n");
  }
};

/*---------------------------------------------------------------------------*/
/* Include file headers (must be included after the above ones)              */
/*---------------------------------------------------------------------------*/
#include "cpp_sym.hpp"
#include "lextable.hpp"
#include "expr.hpp"
#include "decl.hpp"
#include "stmt.hpp"
#include "rule.hpp"

bool matchparams(char *name, ste * formals, exprlist * actuals);
bool type_equal(typedecl * a, typedecl * b);
/* whether two types should be considered compatible. */


/*---------------------------------------------------------------------------*/
/* GDP struct messagerec                                                             */
/*---------------------------------------------------------------------------*/
struct messagerec {
  const char *message;
  messagerec *next;
  messagerec(const char *_message, messagerec *_next) :message(_message), next(_next)
  {
  };
};


/*---------------------------------------------------------------------------*/
/* struct rulerec                                                            */
/*---------------------------------------------------------------------------*/
struct rulerec {
  const char *rulename;
  const char *conditionname;
  const char *bodyname;
  const char *probbound;
  rulerec *next;
  rulerec(const char *rulename, const char *conditionname,
          const char *bodyname)
    :rulename(rulename), conditionname(conditionname), bodyname(bodyname),
     next(NULL), probbound(NULL)
  {
  };
  int print_rules();		/* print out a list of rulerecs. */
};

/*---------------------------------------------------------------------------*/
/* struct program                                                            */
/*---------------------------------------------------------------------------*/
struct program:TNode {
  enum plan_metric {minimize=-1, maximize=1, dontcare=0};
  int bits_in_world;
  ste *globals;
  ste *procedures;
  rule *rules;
  rulerec *rulelist;
  rulerec *startstatelist;
//UPMURPHI_BEGIN
  rulerec *goallist;
  plan_metric metric;
  expr *metric_expression;
  ste *clock_procedure;
  char *domain_name, *problem_name, *domain_filename, *problem_filename;
  messagerec *messagelist;
  double discretization;
  char *clock_var_name;
//UPMURPHI_END
  rulerec *invariantlist;
  symmetryclass symmetry;

  program(void)
    :globals(NULL), procedures(NULL), goallist(NULL), //UPMURPHI_BEGIN_END
     rules(NULL), rulelist(NULL), startstatelist(NULL),
     invariantlist(NULL),metric(minimize),metric_expression(NULL),clock_procedure(NULL),  //UPMURPHI_BEGIN_END
     domain_name(NULL), problem_name(NULL), domain_filename(NULL), problem_filename(NULL),  //UPMURPHI_BEGIN_END
     discretization(DEF_DISCRETIZATION),messagelist(NULL),clock_var_name(NULL) //UPMURPHI_BEGIN_END
  { }

  void make_state(int bits_in_world);
  virtual const char *generate_code();
};

/*---------------------------------------------------------------------------*/
/* Extern variables                                                          */
/*---------------------------------------------------------------------------*/
extern program *theprog;

/*---------------------------------------------------------------------------*/
/* Extern functions                                                          */
/*---------------------------------------------------------------------------*/
char *get_mfilename();
char *get_pddldomainfilename();
char *get_pddlproblemfilename();

/*---------------------------------------------------------------------------*/
/* Declarations for parser and lexer                                         */
/*---------------------------------------------------------------------------*/
#define IDLEN 256		/* longest length of an identifier. */
extern int gLineNum;		/* current line number in file. */
extern FILE *yyin;		/* the file from which yylex() reads. */
int yylex(void);		/* the lexer, provided by 'flex mu.l' */
void yyerror(const char *s);	/* stuff for yacc. yacc's yyerror. */
extern int offset;		/* offset for variables in current scope. */
extern int yyparse(void);	/* shouldn't this get declared in y.tab.h? */

/*---------------------------------------------------------------------------*/
/* Declaration for Yacc's YYSTYPE, left here for compatibility               */
/* Old comment:               */
/*  -- Yacc's YYSTYPE, done here instead of in a %union in yacc so that we
    -- don't get redefinitions when y.tab.c #include's this which #include's
    -- y_tab.h.                                                              */
/*---------------------------------------------------------------------------*/
union YYSTYPE {			/* sloppy, sloppy, sloppy. Bleah. */
  TNode *node;
  expr *expr_p;			// an expression.
  designator *desig_p;		// a designator (l-value).
  decl *decl_p;			// a declaration.
  typedecl *typedecl_p;		// a type declaration.
  ste *ste_p;			// a record field list or formal parameter list.
  stmt *stmt_p;			// a statement or sequence of statements.
  rule *rule_p;			// a sequence of rules.
  ifstmt *elsifs;		// a sequence of elsif clauses.
  caselist *cases;		// a sequence of cases of a switch statement.
  alias *aliases;		// a sequence of alias bindings.
  int integer;			// an integer constant.
  double real;			// AP: a real constant
  bool boolean;			// a boolean value--used with INTERLEAVED.
  lexid *lex;			// a lexeme, most usefully an ID.
  lexlist *lexlist_p;		// a list of lexemes.
  char *string;			// a string value.
  exprlist *exprlist_p;		// a list of expressions.
  simplerule::rule_pddlclass pddlclass; // rule pddl class
};

typedef union YYSTYPE YYSTYPE;

extern YYSTYPE yylval;

#endif
