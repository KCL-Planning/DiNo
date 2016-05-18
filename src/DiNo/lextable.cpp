/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#include "mu.hpp"
#include <string.h>
#include <ctype.h>

/********************
  Hash table
  -- Mostly copied from the hash table used in CS143, Spring '92. rlm
  -- A simple and mediocre hash table implementation.
  -- This will go into an infinite loop when the hash table fills up.
  ********************/

/********************
  Constant
  -- constant for int lextable::hash(char *str) const
  ********************/
#define M1		71
#define M2		578393
#define M3		358723

/********************
  class lexid
  -- from mu.h
  ********************/
lexid::lexid(const char *name, int lextype)
{
  pddlattrs.pddlname = NULL;
  if (name != NULL) {
    this->name = new char[strlen(name) + 1];	// +1 for the '\0'.
    strcpy(this->name, name);
  } else
    this->name = NULL;
  this->lextype = lextype;
}

void lexid::setpddlname(const char *name)
{
  if (name != NULL) {
    this->pddlattrs.pddlname = new char[strlen(name) + 1];
    strcpy(this->pddlattrs.pddlname, name);
  }

}

/********************
  utilities
  ********************/
char *my_strtolower(const char *str)
{
  char *ret;

  if (str != NULL) {
    ret = new char[strlen(str) + 1];
    strcpy(ret, str);
    for (int i = 0; i < strlen(ret); i++)
      ret[i] = tolower(ret[i]);
  } else
    ret = NULL;
  return ret;
}

/********************
  class lextable
  ********************/
int lextable::hash(const char *str) const
/* This returns an integer hash value for a string. */
{
  char c;
  int h = 0;
  while ((c = *str++) != '\0')
    h = (h * M1 + (int) c);
  return (h);
}

int lextable::rehash(int h) const
/* If hash location is already full, this computes a new hash address to
   try. */
{
  return (h + 1);
}

bool lextable::reserved(const char *str)
{
  // in order to allow at least and at most one entry
  // of reserved words in the main lex table,
  // the lower case version is considered not reserved here,
  // so that the same enter routine can be used in the
  // main lex table for initializing keywords.

  int h;
  char *lowerstr;

  lowerstr = my_strtolower(str);
  h = hash(lowerstr);

  while (1) {
    int i = (h & RESERVETABLESIZE - 1);
    lexid *entry = rtable[i];
    if (entry == NULL) {
      return (FALSE);
    } else if (strcmp(entry->getname(), lowerstr) == 0) {
      return (TRUE);
    } else {
      /* try again */
      h = rehash(h);
    }
  }
}

lexid *lextable::enter_reserved(const char *str, int lextype)
{
  int h;
  char *lowerstr;

  lowerstr = my_strtolower(str);
  h = hash(lowerstr);

  while (1) {
    int i = (h & RESERVETABLESIZE - 1);
    lexid *entry = rtable[i];
    if (entry == NULL) {
      /* enter it and return */
      rtable[i] = entry = new lexid(lowerstr, lextype);
      return (entry);
    } else if (strcmp(entry->getname(), lowerstr) == 0) {
      return (entry);
    } else {
      /* try again */
      h = rehash(h);
    }
  }
}

lexid *lextable::enter(const char *str, int lextype)
{
  int h = hash(str);
  while (1) {
    int i = (h & LEXTABLESIZE - 1);
    lexid *entry = table[i];
    if (entry == NULL) {
      // there is no entry in the lex table
      // but is it a reserved word in a different case context?
      if (reserved(str)) {
        // yes, it is.  get the entry in the reserved words table
        return enter_reserved(str);
      } else {
        /* no, it is not.  Enter it and return */
        table[i] = entry = new lexid(str, lextype);
        return (entry);
      }
    } else if (strcmp(entry->getname(), str) == 0) {
      return (entry);
    } else {
      /* try again */
      h = rehash(h);
    }
  }
}

lextable::lextable()
{
  int i;			/* loop index. */
  for (i = 0; i < LEXTABLESIZE; i++)
    table[i] = NULL;

  // table[] is case sensitive and rtable[] is case insensitive
  // before entering new entry, the string is checked again
  // the reserve table in lower case.  Therefore, all reserved words
  // are case insensitive and everything else are case sensitive
  enter_reserved("end", END);
  enter_reserved("program", PROGRAM);
  // enter_reserved("process",PROCESS);
  enter_reserved("procedure", PROCEDURE);
  enter_reserved("endprocedure", ENDPROCEDURE);
  enter_reserved("function", FUNCTION);
  enter_reserved("endfunction", ENDFUNCTION);
  enter_reserved("rule", RULE);
  enter_reserved("endrule", ENDRULE);
  enter_reserved("ruleset", RULESET);
  enter_reserved("endruleset", ENDRULESET);
  enter_reserved("alias", ALIAS);
  enter_reserved("endalias", ENDALIAS);
  enter_reserved("if", IF);
  enter_reserved("then", THEN);
  enter_reserved("elsif", ELSIF);
  enter_reserved("else", ELSE);
  enter_reserved("endif", ENDIF);
  enter_reserved("switch", SWITCH);
  enter_reserved("case", CASE);
  enter_reserved("endswitch", ENDSWITCH);
  enter_reserved("for", FOR);
  enter_reserved("forall", FORALL);
  enter_reserved("exists", EXISTS);
  enter_reserved("in", IN);
  enter_reserved("do", DO);
  enter_reserved("endfor", ENDFOR);
  enter_reserved("endforall", ENDFORALL);
  enter_reserved("endexists", ENDEXISTS);
  enter_reserved("while", WHILE);
  enter_reserved("endwhile", ENDWHILE);
  enter_reserved("return", RETURN);
  enter_reserved("to", TO);
  enter_reserved("begin", bEGIN);
  enter_reserved("by", BY);
  enter_reserved("clear", CLEAR);
  enter_reserved("error", ERROR);
  enter_reserved("assert", ASSERT);
  enter_reserved("put", PUT);
  enter_reserved("const", CONST);
  enter_reserved("type", TYPE);
  enter_reserved("var", VAR);
  enter_reserved("enum", ENUM);
  enter_reserved("interleaved", INTERLEAVED);
  enter_reserved("record", RECORD);
  enter_reserved("array", ARRAY);
  enter_reserved("of", OF);
  enter_reserved("endrecord", ENDRECORD);
  enter_reserved("startstate", STARTSTATE);
  enter_reserved("endstartstate", ENDSTARTSTATE);
//UPMURPHI_BEGIN
  enter_reserved("goal",GOAL);
  enter_reserved("event",EVENT);
  enter_reserved("action",ACTION);
  enter_reserved("durative_start",DURATIVE_ACTION_START);
  enter_reserved("durative_end",DURATIVE_ACTION_END);
  enter_reserved("clock",CLOCK);
  enter_reserved("duration",DURATION);
  enter_reserved("weight",WEIGHT);
  enter_reserved("pddlname",PDDLNAME);
  enter_reserved("metric",METRICS);
  enter_reserved("minimize",MINIMIZE);
  enter_reserved("maximize",MAXIMIZE);
  enter_reserved("domain",DOMAIN_NAME);
  enter_reserved("problem",PROBLEM_NAME);
  enter_reserved("file",PATH);
  enter_reserved("message",MESSAGE);
  enter_reserved("INTERNAL",INTERNAL);
//UPMURPHI_END
  enter_reserved("invariant", INVARIANT);
  enter_reserved("traceuntil", TRACEUNTIL);

  /* scalarset */
  enter_reserved("scalarset", SCALARSET);
  enter_reserved("ismember", ISMEMBER);

  /* undefined */
  enter_reserved("undefine", UNDEFINE);
  enter_reserved("isundefined", ISUNDEFINED);
  enter_reserved("undefined", UNDEFINED);

  /* general union */
  enter_reserved("union", UNION);

  /* multiset */
  enter_reserved("multiset", MULTISET);
  enter_reserved("multisetremove", MULTISETREMOVE);
  enter_reserved("multisetremovepred", MULTISETREMOVEPRED);
  enter_reserved("multisetadd", MULTISETADD);
  enter_reserved("multisetcount", MULTISETCOUNT);
  enter_reserved("choose", CHOOSE);
  enter_reserved("endchoose", ENDCHOOSE);

  /* first definitions. */
  enter_reserved("boolean", ID);
  enter_reserved("true", ID);
  enter_reserved("false", ID);

  /* AP: added reserved words REAL and NEVERCLAIM *//* IM: NEVERCLAIM deleted */
  enter_reserved("real", REAL);
  enter_reserved("log", LOG);
  enter_reserved("log10", LOG10);
  enter_reserved("exp", EXP);
  enter_reserved("sin", SIN);
  enter_reserved("cos", COS);
  enter_reserved("tan", TAN);
  enter_reserved("fabs", FABS);
  enter_reserved("floor", FLOOR);
  enter_reserved("ceil", CEIL);
  enter_reserved("sqrt", SQRT);
  enter_reserved("fmod", FMOD);
  enter_reserved("pow", POW);
  enter_reserved("asin", ASIN);
  enter_reserved("acos", ACOS);
  enter_reserved("atan", ATAN);
  enter_reserved("sinh", SINH);
  enter_reserved("cosh", COSH);
  enter_reserved("tanh", TANH);

  /* IM: other two reserved words */
  enter_reserved("externfun", EXTERNFUN);
  enter_reserved("externproc", EXTERNPROC);
}

/********************
  variable declarations
  ********************/
lextable ltable;

