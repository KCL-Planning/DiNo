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

/********************
  variable declaration
  ********************/
typedecl *switchtype = NULL;

/********************
  initializers for
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
  -- return stmt
  ********************/

assignment::assignment(designator * target, expr * src)
  :  target(target), src(src)
{
  Error.CondError(!type_equal(target->gettype(), src->gettype())
                  && (!type_equal(target->gettype(), realtype)/* ||
		      type_equal(src->gettype(), booltype)*/),
                  "Type of target of assignment doesn't match type of source.");
  Error.CondError(!target->islvalue(),
                  "Target of assignment is not a variable.");
}

whilestmt::whilestmt(expr * test, stmt * body)
  :  test(test), body(body)
{
  Error.CondError(!type_equal(test->gettype(), booltype),
                  "Test of while loop must be boolean.");
}

ifstmt::ifstmt(expr * test, stmt * body, stmt * elsecode)
  :  test(test), body(body), elsecode(elsecode)
{
  Error.CondError(!type_equal(test->gettype(), booltype),
                  "Test of if statement must be boolean.");
}

caselist::caselist(exprlist * values, stmt * body, caselist * next)
  :  values(values), body(body), next(next)
{
  exprlist *v = values;
  for (; v != NULL; v = v->next) {
    Error.CondError(!v->e->hasvalue(),
                    "Cases in switch statement must be constants.");
    Error.CondError(!type_equal(v->e->gettype(), switchtype),
                    "Type of case does not match type of switch expression.");
  }
}


switchstmt::switchstmt(expr * switchexpr, caselist * cases,
                       stmt * elsecode)
  :  switchexpr(switchexpr), cases(cases), elsecode(elsecode)
{
}


forstmt::forstmt(ste * index, stmt * body)
  :  index(index), body(body)
{
  quantdecl *q = (quantdecl *) index->getvalue();

  if (q->gettype()->gettypeclass() == typedecl::Scalarset
      || q->gettype()->gettypeclass() == typedecl::Union)
    check_sset_loop_restriction();
}

void forstmt::check_sset_loop_restriction()
{
  // if there is no statement in the loop, it satisfied the restriction.
  if (body == NULL)
    return;

  Error.Warning("Scalarset is used in loop index.\n\
\tPlease make sure that the iterations are independent.");

  return;

  /*
    quantdecl *q = (quantdecl *) index->getvalue();
    analysis_rec *vi, *vj;
    int i, j;
    bool violated = FALSE;

    // check for body statements
    // slow implementation --> n^2 rather than n
    if (q->left == NULL)
      {
        // i.e. < for id: type Do >
        for (i = q->type->getleft(); i<= q->type->getright(); i++)
  	for (j = q->type->getleft(); j<= q->type->getright(); j++)
  	  if (i!=j)
  	    {
  	      index->setParaValue(i);
  	      vi = body->analyse_var_access();
  	      index->setParaValue(j);
  	      vj = body->analyse_var_access();
  	      if (conflict(vi,vj)) violated = TRUE;
  	    }
      }
    else
      {
        // i.e. < for id = left to right by amount Do >
        for (i = q->left->getvalue(); i<= q->right->getvalue(); i += q->by )
  	for (j = q->left->getvalue(); j<= q->right->getvalue(); j += q->by )
  	  if (i!=j)
  	    {
  	      index->setParaValue(i);
  	      vi = body->analyse_var_access();
  	      index->setParaValue(j);
  	      vj = body->analyse_var_access();
  	      if (conflict(vi,vj)) violated = TRUE;
  	    }
      }
    Error.CondError(violated, "The for statement with scalarset breaks symmetry.");
  */

}

proccall::proccall(ste * procedure, exprlist * actuals)
  :  procedure(procedure), actuals(actuals)
{
  if (procedure != 0) {
    if (!Error.CondError(procedure->getvalue()->getclass() != decl::Proc,
                         "%s is not a procedure.",
                         procedure->getname()->getname())) {
      procdecl *p = (procdecl *) procedure->getvalue();
      matchparams(procedure->getname()->getname(), p->params, actuals);
    }
  }
}

clearstmt::clearstmt(designator * target)
  :  target(target)
{
  Error.CondError(!target->islvalue(),
                  "Target of CLEAR statement must be a variable.");
  Error.CondError(target->gettype()->HasScalarsetLeaf(),
                  "Target of CLEAR statement must not contain a scalarset variable.");
}

undefinestmt::undefinestmt(designator * target)
  :  target(target)
{
  Error.CondError(!target->islvalue(),
                  "Target of UNDEFINE statement must be a variable.");
}

multisetaddstmt::multisetaddstmt(designator * element, designator * target)
  :  element(element), target(target)
{
  Error.CondError(element->gettype() !=
                  target->gettype()->getelementtype(),
                  "Multisetadd statement -- 1st argument must be of elementtype of the 2nd argument.");
}

multisetremovestmt::multisetremovestmt(expr * criterion,
                                       designator * target)
  :  index(NULL), target(target), criterion(criterion)
{
  if (criterion->isdesignator()
      && criterion->gettype()->gettypeclass() == typedecl::MultiSetID) {
    Error.
    CondError(((multisetidtypedecl *) criterion->
               gettype())->getparenttype() != target->gettype(),
              "Multisetremove statement -- 2nd argument does not match 3rd argument");
    matchingchoose = TRUE;
  } else
    Error.Error("Internal Error: cannot parse MultiSetRemove.");
}

multisetremovestmt::multisetremovestmt(ste * index, designator * target,
                                       expr * criterion)
  :  index(index), target(target), criterion(criterion)
{
  Error.CondError(criterion->gettype() != booltype,
                  "Multisetremove statement -- 3rd argument must be a boolean expression.");
  matchingchoose = FALSE;
  multisetremove_num = num_multisetremove;
  num_multisetremove++;
  ((multisettypedecl *) target->gettype())->addremove(this);
}

int multisetremovestmt::num_multisetremove = 0;

errorstmt::errorstmt(char *string)
  :string(string != NULL ?
          string : tsprintf("%s, line %d.", gFileName, gLineNum))
{
}

assertstmt::assertstmt(expr * test, char *string)
  :errorstmt(string != NULL ?
             string : tsprintf("%s, line %d.", gFileName, gLineNum)),
   test(test)
{
  Error.CondError(!type_equal(test->gettype(), booltype),
                  "ASSERT condition must be a boolean expression.");
}


putstmt::putstmt(expr * putexpr)
  :  putexpr(putexpr), putstring(NULL)
{
}


putstmt::putstmt(char *putstring)
  :putexpr(NULL), putstring(putstring)
{
}


alias::alias(ste * name, designator * target, alias * next)
  :  name(name), target(target), next(next)
{
}


aliasstmt::aliasstmt(ste * aliases, stmt * body)
  :  aliases(aliases), body(body)
{
}


returnstmt::returnstmt(expr * returnexpr)
  :  retexpr(returnexpr)
{
// if (returnexpr==NULL)
// printf("returnstmt::returnstmt called with null.\n");
// else printf("returnstmt::returnstmt called with something.\n");
}

/********************
  variable declaration
  ********************/
stmt *nullstmt = NULL;

