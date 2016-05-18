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
#include <iostream>

/********************
  class rule
  ********************/
rule::rule(void)
  :
  next(NULL)
{
}

/********************
  class simplerule and variable simplerulenum
  ********************/
RULE_INDEX_TYPE simplerulenum = 0;
simplerule::simplerule(ste * enclosures,
                       expr * condition,
                       ste * locals, stmt * body,
                       int priority,
                       rule_pddlclass pddlclass, expr * duration, expr * weight)
  :rule(), name(name), enclosures(enclosures), condition(condition),
   locals(locals), body(body), condname(NULL), rulename(NULL),
   priority(priority),pddlclass(pddlclass),duration(duration),weight(weight),pddlname(NULL)
{

  if (duration != NULL) {
    Error.CondError(!type_equal(duration->gettype(), inttype),
                    "Only integer duration allowed.");
    Error.CondError((duration->getvalue() < 0),
                    "Duration must be >= 0");
  }

  if ((pddlclass == Clock) && (duration == NULL || duration->getvalue() == 0)) {
    this->duration = new expr( 1, inttype ); //clock cannot have a zero duration
  }

  if (weight != NULL) {
    Error.CondError((weight->getvalue() < 0),
                    "Weight must be >= 0");
  }

  if (condition != NULL && body != NULL) {
    Error.CondError(condition->has_side_effects(),
                    "Rule Condition must not have side effects.");

    Error.CondError(!type_equal(condition->gettype(), booltype),
                    "Condition for rule must be a boolean expression.");
  }
  NextSimpleRule = SimpleRuleList;
  SimpleRuleList = this;

  size = CountSize(enclosures);

  // Rearrange enclosures so that the ones that are NOT
  // mentioned in the condition go first
  rearrange_enclosures();
}

simplerule *simplerule::SimpleRuleList = NULL;

int
simplerule::CountSize(ste * enclosures)
{
  int ret;
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    ret = CountSize(enclosures->getnext());
    if (enclosures->getvalue()->getclass() == decl::Quant ||
        enclosures->getvalue()->getclass() == decl::Choose)
      return ret * enclosures->getvalue()->gettype()->getsize();
    else
      return ret;
  } else
    return 1;
}


void simplerule::rearrange_enclosures()
{
  stecoll *us = condition ? condition->used_stes() : NULL;
  ste *front = NULL;
  ste *end = NULL;
  ste *rest = NULL;
  ste *ep = enclosures;

  bool is_dep = FALSE;		// Is the current ste mentioned in rule cond?
  bool is_cp = FALSE;		// Is the current ste a choose parameter?
  ste *cp = NULL;		// Choose parameter

  // Cardinality of parameters not mentioned in the condition
  indep_card = 1;
  // Lists of choose parameters
  dep_choose = indep_choose = NULL;

  if (!us)
    return;

  // Iterate over all relevant ste's in enclosures
  ep = enclosures;
  while (ep != NULL &&
         (ep->getvalue()->getclass() == decl::Quant ||
          ep->getvalue()->getclass() == decl::Alias ||
          ep->getvalue()->getclass() == decl::Choose)) {

    // Create a copy of ep
    ste *ns = new ste(ep->getname(), ep->getscope(), ep->getvalue());
    ns->setnext(NULL);

    // If it's a choose parameter, create another copy
    is_cp = (ep->getvalue()->getclass() == decl::Choose);
    if (is_cp) {
      cp = new ste(ep->getname(), ep->getscope(), ep->getvalue());
      cp->setnext(NULL);
    }

    if (us->includes(ep)) {
      // ep is mentioned in the condition
      is_dep = TRUE;

      if (ep->getvalue()->getclass() == decl::Alias) {
        stecoll *alias_stes = ep->getvalue()->getexpr()->used_stes();
        us->append(alias_stes);
      }
      // if it's a choose parameter,
      // add to the list of ``dependent'' choose parameters
      if (is_cp) {
        cp->setnext(dep_choose);
        dep_choose = cp;
      }
    }

    else {
      // ep is NOT mentioned in the condition
      is_dep = FALSE;

      // Cardinality of Alias is always 1
      if (ep->getvalue()->getclass() != decl::Alias) {
        indep_card *= ns->getvalue()->gettype()->getsize();
      }
      // if it's a choose parameter,
      // add to the list of ``independent'' choose parameters
      if (is_cp) {
        cp->setnext(indep_choose);
        indep_choose = cp;
      }
    }

    // If it's not an alias and it's mentioned in the condition,
    //    move to the front of the list;
    // otherwise add to the end of the list
    if (!front) {
      front = end = ns;
    } else {
      // if(is_dep) {
      if (is_dep && (ep->getvalue()->getclass() != decl::Alias)) {
        ns->setnext(front);
        front = ns;
      } else {
        end->setnext(ns);
        end = ns;
      }
    }

    ep = ep->getnext();
  }
  // The rest of the enclosures
  rest = ep;

  if (front) {
    enclosures = front;
    end->setnext(rest);
  }

}


/********************
  class startstate and variable startstatenum
  ********************/
int startstatenum = 0;
startstate::startstate(ste * enclosures, ste * locals, stmt * body)
  :  simplerule(enclosures, NULL, locals, body, 0, simplerule::Other, NULL, NULL)
{
}

/********************
  class invariant and variables invariantnum, has_invar
  ********************/
int invariantnum = 0;
bool has_invar = FALSE;

invariant::invariant(ste * enclosures, expr * test)
  :  simplerule(enclosures, test, NULL, NULL, 0, simplerule::Other, NULL, NULL)
{
  Error.CondError(!type_equal(test->gettype(), booltype),
                  "Invariant must be a boolean expression.");
  Error.CondError(test->has_side_effects(),
                  "Invariant must not have side effects.");
}

/********************
  class quantrule
  ********************/
quantrule::quantrule(ste * quant, rule * rules)
  :  rule(), quant(quant), rules(rules)
{
  quantdecl *q = (quantdecl *) quant->getvalue();
  if (q->left != NULL) {
    Error.CondError(!q->left->hasvalue(),
                    "Bounds for ruleset parameters must be constants.");
    Error.CondError(!q->right->hasvalue(),
                    "Bounds for ruleset parameters must be constants.");
  }
}

/********************
  class chooserule
  ********************/
chooserule::chooserule(ste * index, designator * set, rule * rules)
  :  rule(), index(index), set(set), rules(rules)
{
  Error.
  CondError(((multisetidtypedecl *) index->getvalue()->
             gettype())->getparenttype() != set->gettype(),
            "Internal Error: Parameter for Choose declared in type other than the multiset index");
  Error.CondError(set->gettype()->gettypeclass() != typedecl::MultiSet,
                  "Parameter for choose must be a multiset");
}

/********************
  class aliasrule
  ********************/
aliasrule::aliasrule(ste * aliases, rule * rules)
  :  rule(), aliases(aliases), rules(rules)
{
}

/********************
  variable declaration
  ********************/
simplerule *error_rule = NULL;

//UPMURPHI_BEGIN
/********************
  class plangoal and variables goalnum, has_goal
  ********************/
int goalnum = 0;
bool has_goal = FALSE;

plangoal::plangoal (ste * enclosures, expr * test)
  :simplerule (enclosures, test, NULL, NULL, 0, simplerule::Other, NULL, NULL)
{
  Error.CondError (!type_equal (test->gettype (), booltype),
                   "Goal must be a boolean expression.");
  Error.CondError (test->has_side_effects (),
                   "Goal must not have side effects.");
}
//UPMURPHI_END
