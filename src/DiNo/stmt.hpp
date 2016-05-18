/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#ifndef __STMT_H__
#define __STMT_H__

/********************
  class stmt
  ********************/
class stmt:public TNode
{
 public:
  stmt * next;
  stmt(void)
    :next(NULL)
  {
  };
  virtual const char *generate_code();
  virtual const char *get_target(){return "ERROR";}; // WP WP WP
  virtual const char *get_src(){return "ERROR";}; // WP WP WP
  virtual const char *get_proc_code_plus(){return "ERROR";}; // WP WP WP
  virtual const char *get_proc_code_minus(){return "ERROR";}; // WP WP WP
};

/********************
  classes:
  -- assignment
  -- whilestmt
  -- ifstmt
  -- caselist
  -- switchstmt
  -- forstmt
  -- proccall
  -- clearstmt
  -- errorstmt
  -- assertstmt
  -- putstmt
  -- alias
  -- aliasstmt
  -- returnstmt
  ********************/

struct assignment:stmt {
  designator *target;
  expr *src;
  assignment(designator * target, expr * src);
  virtual const char *generate_code();
  virtual const char *get_target(); // WP WP WP
  virtual const char *get_src(); // WP WP WP
};


struct whilestmt:stmt {
  expr *test;
  stmt *body;
  whilestmt(expr * test, stmt * body);
  virtual const char *generate_code();
};

struct ifstmt:stmt {
  expr *test;
  stmt *body;
  stmt *elsecode;
  ifstmt(expr * test, stmt * body, stmt * elsecode = NULL);
  virtual const char *generate_code();
  virtual const char *get_proc_code_plus();
  virtual const char *get_proc_code_minus();
};


struct caselist {
  exprlist *values;
  stmt *body;
  caselist *next;
  caselist(exprlist * values, stmt * body, caselist * next = NULL);
  virtual const char *generate_code();
};

struct switchstmt:stmt {
  expr *switchexpr;
  caselist *cases;
  stmt *elsecode;
  switchstmt(expr * switchexpr, caselist * cases, stmt * elsecode);
  virtual const char *generate_code();
};


struct forstmt:stmt {
  ste *index;
  stmt *body;
  forstmt(ste * index, stmt * body);
  virtual const char *generate_code();

  // special for loop restriction for scalarset quantified loop
  // not yet implemented
  void check_sset_loop_restriction();
};

struct proccall:stmt {
  ste *procedure;
  exprlist *actuals;
  proccall(ste * procedure, exprlist * actuals);
  virtual const char *generate_code();
};


struct clearstmt:stmt {
  designator *target;
  clearstmt(designator * target);
  virtual const char *generate_code();
};

struct undefinestmt:stmt {
  designator *target;
  undefinestmt(designator * target);
  virtual const char *generate_code();
};

// WP
struct array_dim_stmt:stmt {
  designator *target;
   array_dim_stmt(designator * target);
  virtual const char *generate_code();
};

struct multisetaddstmt:stmt {
  designator *element;
  designator *target;
  multisetaddstmt(designator * element, designator * target);
  virtual const char *generate_code();
};

class multisettypedecl;

struct multisetremovestmt:stmt {
  static int num_multisetremove;

  bool matchingchoose;
  ste *index;
  designator *target;
  expr *criterion;
  int multisetremove_num;

  multisetremovestmt(ste * index, designator * target, expr * criterion);
  multisetremovestmt(expr * criterion, designator * target);
  virtual void generate_decl(multisettypedecl * mset);
  virtual void generate_procedure();
  virtual const char *generate_code();
};

struct errorstmt:stmt {
  char *string;
  errorstmt(char *string);
  virtual const char *generate_code();
};


struct assertstmt:errorstmt {
  expr *test;
  assertstmt(expr * test, char *string);
  virtual const char *generate_code();
};

struct putstmt:stmt {
  expr *putexpr;
  char *putstring;
  putstmt(expr * putexpr);
  putstmt(char *putstring);
  virtual const char *generate_code();
};


struct alias:public TNode {
  ste *name;
  designator *target;
  alias *next;
  alias(ste * name, designator * target, alias * next = NULL);
  virtual const char *generate_code();
};


struct aliasstmt:stmt {
  ste *aliases;
  stmt *body;
  aliasstmt(ste * aliases, stmt * body);
  virtual const char *generate_code();
};


struct returnstmt:stmt {
  expr *retexpr;
  returnstmt(expr * returnexpr = NULL);
  virtual const char *generate_code();
};

/********************
  extern variable
  ********************/
extern stmt *nullstmt;

#endif
