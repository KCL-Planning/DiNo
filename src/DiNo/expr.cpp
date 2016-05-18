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
// #include "y.tab.h" /* for operator values. */

/********************
  class expr
  ********************/
expr::expr(const int value, typedecl * const type)
  :
  value(value), type(type), constval(TRUE), sideeffects(FALSE)
{
  rvalue = (double) value;
}

expr::expr(const double value, typedecl * const type)	// AP: new constructor
  :rvalue(value), type(type), constval(TRUE), sideeffects(FALSE)
{
  this->value = (int) value;
}

expr::expr(void)
  :value(0), type(NULL), constval(FALSE), sideeffects(FALSE)
{
}

mathexpr::mathexpr(expr * arg1, expr * arg2, int arg3)
  :expr(realtype, arg2 == NULL ? arg1->hasvalue() : arg1->hasvalue()
        && arg2->hasvalue(),
        arg2 == NULL ? arg1->has_side_effects() : arg1->has_side_effects()
        || arg2->has_side_effects()), arg1(arg1), arg2(arg2)
{
  switch (arg3) {
    case 1: {
      funtype = mylog;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = log(arg1->getrvalue());
        else
          rvalue = log(arg1->getvalue());
      }
      break;
    }
    case 2: {
      funtype = mylog10;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = log10(arg1->getrvalue());
        else
          rvalue = log10(arg1->getvalue());
      }
      break;
    }
    case 3: {
      funtype = myexp;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = exp(arg1->getrvalue());
        else
          rvalue = exp(arg1->getvalue());
      }
      break;
    }
    case 4: {
      funtype = mysin;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = sin(arg1->getrvalue());
        else
          rvalue = sin(arg1->getvalue());
      }
      break;
    }
    case 5: {
      funtype = mycos;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = cos(arg1->getrvalue());
        else
          rvalue = cos(arg1->getvalue());
      }
      break;
    }
    case 6: {
      funtype = mytan;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = tan(arg1->getrvalue());
        else
          rvalue = tan(arg1->getvalue());
      }
      break;
    }
    case 7: {
      funtype = myfabs;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = fabs(arg1->getrvalue());
        else
          rvalue = fabs(arg1->getvalue());
      }
      break;
    }
    case 8: {
      funtype = myfloor;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = floor(arg1->getrvalue());
        else
          rvalue = floor(arg1->getvalue());
      }
      break;
    }
    case 9: {
      funtype = myceil;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = ceil(arg1->getrvalue());
        else
          rvalue = ceil(arg1->getvalue());
      }
      break;
    }
    case 10: {
      funtype = mysqrt;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = sqrt(arg1->getrvalue());
        else
          rvalue = sqrt(arg1->getvalue());
      }
      break;
    }
    case 11: {
      funtype = myfmod;
      if (arg1->hasvalue() && arg2->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype)) {
          if (type_equal(arg2->gettype(), realtype))
            rvalue = fmod(arg1->getrvalue(), arg2->getrvalue());
          else
            rvalue = fmod(arg1->getrvalue(), arg2->getvalue());
        } else {
          if (type_equal(arg1->gettype(), realtype))
            rvalue = fmod(arg1->getvalue(), arg2->getrvalue());
          else
            rvalue = fmod(arg1->getvalue(), arg2->getvalue());
        }
      }
      break;
    }
    case 12: {
      funtype = mypow;
      if (arg1->hasvalue() && arg2->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype)) {
          if (type_equal(arg2->gettype(), realtype))
            rvalue = pow(arg1->getrvalue(), arg2->getrvalue());
          else
            rvalue = pow(arg1->getrvalue(), arg2->getvalue());
        } else {
          if (type_equal(arg1->gettype(), realtype))
            rvalue = pow(arg1->getvalue(), arg2->getrvalue());
          else
            rvalue = pow(arg1->getvalue(), arg2->getvalue());
        }
      }
      break;
    }
    case 13: {
      funtype = myasin;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = asin(arg1->getrvalue());
        else
          rvalue = asin(arg1->getvalue());
      }
      break;
    }
    case 14: {
      funtype = myacos;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = acos(arg1->getrvalue());
        else
          rvalue = acos(arg1->getvalue());
      }
      break;
    }
    case 15: {
      funtype = myatan;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = atan(arg1->getrvalue());
        else
          rvalue = atan(arg1->getvalue());
      }
      break;
    }
    case 16: {
      funtype = mysinh;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = sinh(arg1->getrvalue());
        else
          rvalue = sinh(arg1->getvalue());
      }
      break;
    }
    case 17: {
      funtype = mycosh;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = cosh(arg1->getrvalue());
        else
          rvalue = cosh(arg1->getvalue());
      }
      break;
    }
    case 18: {
      funtype = mytanh;
      if (arg1->hasvalue()) {
        if (type_equal(arg1->gettype(), realtype))
          rvalue = tanh(arg1->getrvalue());
        else
          rvalue = tanh(arg1->getvalue());
      }
      break;
    }
  }
}


//GDP: internal constants
specvalexpr::specvalexpr(char * _valtype)
  :expr(inttype, FALSE, FALSE)
{
  if (!strcmp(_valtype,"level")) {
    valtype = mytimer;
  } else {
    Error.Error("Unknown INTERNAL value requested: %s",_valtype);
  }
}

/********************
  class unaryexpr
  -- class for all unary expression
  ********************/
unaryexpr::unaryexpr(typedecl * type, int op, expr * left)
  :expr(type, left->hasvalue(), left->has_side_effects()), op(op), left(left)
{
}

unaryexpr::unaryexpr(typedecl * type, expr * left)
  :  expr(type, left->hasvalue(), left->has_side_effects()), left(left)
{
}

/********************
  class binaryexpr
  -- class for all binary expression
  ********************/
binaryexpr::binaryexpr(typedecl * type, int op, expr * left, expr * right)
  :expr(type, left->hasvalue() && right->hasvalue(),
        left->has_side_effects() || right->has_side_effects()),
   op(op), left(left), right(right)
{
}

/********************
  class boolexpr
  -- a binary expression of two boolean arguments.
  ********************/
boolexpr::boolexpr(int op, expr * left, expr * right)
  :binaryexpr(booltype, op, left, right)
{
  Error.CondError(!type_equal(left->gettype(), booltype),
                  "Arguments of &, |, -> must be boolean.");
  Error.CondError(!type_equal(right->gettype(), booltype),
                  "Arguments of &, |, -> must be boolean.");
  Error.CondWarning(sideeffects,
                    "Expressions with side effects may behave oddly in some boolean expressions.");
  if (constval) {
    switch (op) {
      case IMPLIES:
        value = !(left->getvalue()) || (right->getvalue());
        break;
      case '&':
        value = left->getvalue() && right->getvalue();
        break;
      case '|':
        value = left->getvalue() || right->getvalue();
        break;
      default:
        Error.
        Error("Internal: surprising value for op in boolexpr::boolexpr");
    }
  }
}

/********************
  class notexpr
  -- boolean not
  ********************/
notexpr::notexpr(expr * left)
  :  unaryexpr(booltype, left)
{
  Error.CondError(!type_equal(left->gettype(), booltype),
                  "Arguments of ! must be boolean.");
  Error.CondWarning(sideeffects,
                    "Expressions with side effects may behave oddly in some boolean expressions.");
  if (constval)
    value = !left->getvalue();
}

/********************
  class equalexpr
  -- A = B or A != B.
  ********************/
equalexpr::equalexpr(int op, expr * left, expr * right)
  :binaryexpr(booltype, op, left, right)
{
  bool real_left, real_right;

  real_left = type_equal(left->gettype(), realtype);
  real_right = type_equal(right->gettype(), realtype);

  Error.CondError(!type_equal(left->gettype(),
                              right->gettype()),
                  "Arguments of = or <> must have same types.");
  Error.CondError(!left->gettype()->issimple(),
                  "Arguments of = or <> must be of simple type.");
  if (constval) {
    switch (op) {
      case '=':
        switch (real_left) {
          case true: {
            if (real_right)
              value = left->getrvalue() == right->getrvalue();
            else
              value = left->getrvalue() == right->getvalue();
            break;
          }
          case false: {
            if (real_right)
              value = left->getvalue() == right->getrvalue();
            else
              value = left->getvalue() == right->getvalue();
            break;
          }
        }
        break;
      case NEQ:
        switch (real_left) {
          case true: {
            if (real_right)
              value = left->getrvalue() != right->getrvalue();
            else
              value = left->getrvalue() != right->getvalue();
            break;
          }
          case false: {
            if (real_right)
              value = left->getvalue() != right->getrvalue();
            else
              value = left->getvalue() != right->getvalue();
            break;
          }
        }
        break;
      default:
        Error.Error
        ("Internal: surprising value for op in equalexpr::equalexpr");
        break;
    }
  }
}

/********************
  class compexpr
  -- A < B, etc.
  ********************/
compexpr::compexpr(int op, expr * left, expr * right)
  :binaryexpr(booltype, op, left, right)
{
  bool real_left, real_right;

  real_left = type_equal(left->gettype(), realtype);
  real_right = type_equal(right->gettype(), realtype);

  Error.CondError(!type_equal(left->gettype(), inttype)
                  && !type_equal(left->gettype(), realtype),
                  "Arguments of <, <=, >, >= must be integral or real.");
  Error.CondError(!type_equal(right->gettype(), inttype)
                  && !type_equal(right->gettype(), realtype),
                  "Arguments of  <, <=, >, >= must be integral or real.");

  if (constval) {
    switch (op) {
      case '<':
        switch (real_left) {
          case true: {
            if (real_right)
              value = left->getrvalue() < right->getrvalue();
            else
              value = left->getrvalue() < right->getvalue();
            break;
          }
          case false: {
            if (real_right)
              value = left->getvalue() < right->getrvalue();
            else
              value = left->getvalue() < right->getvalue();
            break;
          }
        }
        break;
      case LEQ:
        switch (real_left) {
          case true: {
            if (real_right)
              value = left->getrvalue() <= right->getrvalue();
            else
              value = left->getrvalue() <= right->getvalue();
            break;
          }
          case false: {
            if (real_right)
              value = left->getvalue() <= right->getrvalue();
            else
              value = left->getvalue() <= right->getvalue();
            break;
          }
        }
        break;
      case '>':
        switch (real_left) {
          case true: {
            if (real_right)
              value = left->getrvalue() > right->getrvalue();
            else
              value = left->getrvalue() > right->getvalue();
            break;
          }
          case false: {
            if (real_right)
              value = left->getvalue() > right->getrvalue();
            else
              value = left->getvalue() > right->getvalue();
            break;
          }
        }
        break;
      case GEQ:
        switch (real_left) {
          case true: {
            if (real_right)
              value = left->getrvalue() >= right->getrvalue();
            else
              value = left->getrvalue() >= right->getvalue();
            break;
          }
          case false: {
            if (real_right)
              value = left->getvalue() >= right->getrvalue();
            else
              value = left->getvalue() >= right->getvalue();
            break;
          }
        }
        break;
      default:
        Error.Error
        ("Internal: surprising value for op in compexpr::compexpr.");
        break;
    }
  }
}

/********************
  class arithexpr
  -- an arithmetic expression.
  ********************/
arithexpr::arithexpr(int op, expr * left, expr * right)
  :binaryexpr(inttype, op, left, right)
{
  bool real_left, real_right;

  real_left = type_equal(left->gettype(), realtype);
  real_right = type_equal(right->gettype(), realtype);

  if (real_left || real_right)
    type = realtype;

  Error.CondError(!type_equal(left->gettype(), inttype)
                  && !type_equal(left->gettype(), realtype),
                  "Arguments of %c must be integral or real.", op);
  Error.CondError(!type_equal(right->gettype(), inttype)
                  && !type_equal(right->gettype(), realtype),
                  "Arguments of %c must be integral or real.", op);
  if (constval) {
    switch (op) {
      case '+':
        switch (real_left) {
          case true: {
            if (real_right)
              rvalue = left->getrvalue() + right->getrvalue();
            else
              rvalue = left->getrvalue() + right->getvalue();
            break;
          }
          case false: {
            if (real_right)
              rvalue = left->getvalue() + right->getrvalue();
            else
              value = left->getvalue() + right->getvalue();
            break;
          }
        }
        break;
      case '-':
        switch (real_left) {
          case true: {
            if (real_right)
              rvalue = left->getrvalue() - right->getrvalue();
            else
              rvalue = left->getrvalue() - right->getvalue();
            break;
          }
          case false: {
            if (real_right)
              rvalue = left->getvalue() - right->getrvalue();
            else
              value = left->getvalue() - right->getvalue();
            break;
          }
        }
        break;
      default:
        /* Commented out to allow for mulexprs. */
        // Error.Error("Internal: surprising value for op in arithexpr::arithexpr.");
        break;
    }
  }
}

/********************
  class unexpr
  -- an unary arithmetic expression.
  -- +expr or -expr.
  ********************/
unexpr::unexpr(int op, expr * left)
  :unaryexpr(inttype, op, left)
{
  bool real_left;

  real_left = type_equal(left->gettype(), realtype);

  if (real_left)
    type = realtype;

  Error.CondError(!type_equal(left->gettype(), inttype)
                  && !type_equal(left->gettype(), realtype),
                  "Arguments of %c must be integral or real.", op);

  if (constval) {
    switch (op) {
      case '+':
        switch (real_left) {
          case true: {
            rvalue = left->getrvalue();
            break;
          }
          case false: {
            value = left->getvalue();
            break;
          }
        }
        break;
      case '-':
        switch (real_left) {
          case true: {
            rvalue = -left->getrvalue();
            break;
          }
          case false: {
            value = -left->getvalue();
            break;
          }
        }
        break;
      default:
        Error.Error("Internal: surprising value for op in unexpr::unexpr.");
        break;
    }
  }
}

/********************
  class mulexpr
  -- a multiplication or division.
  ********************/
mulexpr::mulexpr(int op, expr * left, expr * right)
  :arithexpr(op, left, right)
{
  bool real_left, real_right;

  real_left = type_equal(left->gettype(), realtype);
  real_right = type_equal(right->gettype(), realtype);

  if (real_left || real_right)
    type = realtype;

  if (constval) {
    switch (op) {
      case '*':
        switch (real_left) {
          case true: {
            if (real_right)
              rvalue = left->getrvalue() * right->getrvalue();
            else
              rvalue = left->getrvalue() * right->getvalue();
            break;
          }
          case false: {
            if (real_right)
              rvalue = left->getvalue() * right->getrvalue();
            else
              value = left->getvalue() * right->getvalue();
            break;
          }
        }
        break;
      case '/':
        switch (real_left) {
          case true: {
            if (real_right)
              rvalue = left->getrvalue() / right->getrvalue();
            else
              rvalue = left->getrvalue() / right->getvalue();
            break;
          }
          case false: {
            if (real_right)
              rvalue = left->getvalue() / right->getrvalue();
            else
              value = left->getvalue() / right->getvalue();
            break;
          }
        }
        break;
      case '%':			/* not allowed as of 9/92, but just in case. */
        if (real_left || real_right)
          Error.Error("Illegal use of floating point in module operation.");
        value = left->getvalue() % right->getvalue();
        break;
      default:
        Error.
        Error("Internal: surprising value for op in mulexpr::mulexpr.");
        break;
    }
  }
}

/********************
  class quantexpr
  -- a quantified expression.
  ********************/
quantexpr::quantexpr(int op, ste * parameter, expr * left)
  :expr(booltype, FALSE, left->has_side_effects()),
   parameter(parameter), op(op), left(left)
{
  Error.CondError(!type_equal(left->gettype(), booltype),
                  "Quantified subexpressions must be boolean.");
  Error.CondWarning(sideeffects,
                    "Expressions with side effects in quantified expressions may behave oddly.");

}

/********************
  class condexpr
  -- a ?: expression.  We need to have a subclass for this
  -- for the extra arg.
  ********************/
condexpr::condexpr(expr * test, expr * left, expr * right)
  :
  expr(left->gettype(),
      FALSE,
      test->has_side_effects() ||
      left->has_side_effects() ||
      right->has_side_effects()), test(test), left(left), right(right)
{
  Error.CondError(!type_equal(test->gettype(), booltype),
                  "First argument of ?: must be boolean.");
  Error.CondError(!type_equal(left->gettype(),
                              right->gettype()),
                  "Second and third arguments of ?: must have same type.");
}

/********************
  class designator
  -- represented as a left-linear tree.
  ********************/
designator::designator(ste * origin,
                       typedecl * type,
                       bool islvalue,
                       bool isconst, bool maybeundefined, int val)
  :expr(val, type),
   origin(origin),
   lvalue(islvalue), left(NULL), dclass(Base), maybeundefined(maybeundefined)
{
  constval = isconst;
  sideeffects = 0;
}

designator::designator(ste * origin,
                       typedecl * type,
                       bool islvalue,
                       bool isconst, bool maybeundefined, double val)
  :expr(val, type),
   origin(origin),
   lvalue(islvalue), left(NULL), dclass(Base), maybeundefined(maybeundefined)
{
  constval = isconst;
  sideeffects = 0;

}

designator::designator(designator * left, expr * ar)
  :
  expr(NULL, FALSE,
      left->has_side_effects() || ar->has_side_effects()),
  left(left), arrayref(ar), lvalue(left->lvalue), dclass(ArrayRef)
{
  typedecl *t = left->gettype();
  if (Error.CondError(t->gettypeclass() != typedecl::Array
                      && t->gettypeclass() != typedecl::MultiSet,
                      "Not an array/multiset type.")) {
    type = errortype;
  } else if (t->gettypeclass() == typedecl::Array) {
    if (Error.CondError(!type_equal(((arraytypedecl *) t)->getindextype(),
                                    ar->gettype()),
                        "Wrong index type for array reference.")) {
      type = errortype;
    } else {
      type = ((arraytypedecl *) t)->getelementtype();
    }
  } else {
    if (Error.CondError(!ar->isdesignator(), "B")
        ||
        Error.CondError(ar->gettype()->gettypeclass() !=
                        typedecl::MultiSetID, "A")
        || Error.CondError(!type_equal
                           ((multisettypedecl *) t,
                            ((multisetidtypedecl *) ar->
                             gettype())->getparenttype()),
                           "Wrong index type for MultiSet reference.")) {
      type = errortype;
    } else {
      type = ((multisettypedecl *) t)->getelementtype();
    }
  }
}

designator::designator(designator * left, lexid * fr)
  :
  expr(NULL, FALSE,
      left->has_side_effects()),
  left(left), lvalue(left->lvalue), dclass(FieldRef)
{
  typedecl *t = left->gettype();
  if (!Error.CondError(t->gettypeclass() != typedecl::Record,
                       "Not a record type.")) {
    ste *field = ((recordtypedecl *) t)->getfields()->search(fr);
    if (Error.CondError(field == NULL,
                        "No field with name %s.", fr->getname())) {
      type = errortype;
    } else {
      fieldref = field;
      type = field->getvalue()->gettype();
    }
  } else {
    type = errortype;
  }
}

ste *designator::getbase(void) const
{
  designator *d = (designator *) this;
  while (d->left != NULL)
    d = d->left;
  return d->origin;
}

stecoll *designator::used_stes() const
{
  stecoll *l = left ? left->used_stes() : new stecoll;
  stecoll *o = origin ? new stecoll(origin) : new stecoll;
  stecoll *a = arrayref ? arrayref->used_stes() : new stecoll;
  // NOTE:  fieldref is ignored

  l->append(o);
  l->append(a);

  return l;
}


/********************
  class exprlist
  ********************/
exprlist::exprlist(expr * e, exprlist * next)
  :
  e(e), undefined(FALSE), next(next)
{
}

exprlist::exprlist(bool b, exprlist * next)
  :  e(NULL), undefined(b), next(next)
{
  Error.CondError(b == FALSE, "Internal Error::exprlist()");
}

exprlist *exprlist::reverse(void)
{
  exprlist *in = this, *out = NULL, *temp = NULL;
  while (in != NULL) {
    temp = in;
    in = in->next;
    temp->next = out;
    out = temp;
  }
  return out;
}

/********************
  class funccall
  ********************/
funccall::funccall(ste * func, exprlist * actuals)
  :  expr(NULL, FALSE, FALSE), func(func), actuals(actuals)
{
  if (func != 0) {
    if (Error.CondError(func->getvalue()->getclass() != decl::Func,
                        "%s is not a function.",
                        func->getname()->getname())) {
      type = errortype;
    } else {
      funcdecl *f = (funcdecl *) func->getvalue();
      if (f != NULL) {		/* IM: this now happens only when it is not an extern call */
        matchparams(func->getname()->getname(), f->params, actuals);
        type = f->returntype;
        sideeffects = f->has_side_effects();
      } else {
        sideeffects = false;
      }
    }
  }
}

/********************
  class isundefined
  ********************/
isundefined::isundefined(expr * left)
  :  unaryexpr(booltype, left)
{
  if (constval)
    value = FALSE;
  Error.CondError(!left->gettype()->issimple(),
                  "The designator in ISUNDEFINED statement must be a simple variable.");
}

/********************
  class ismember
  ********************/
ismember::ismember(expr * left, typedecl * type)
  :  unaryexpr(booltype, left)
{
  if (type->gettypeclass() != typedecl::Scalarset
      && type->gettypeclass() != typedecl::Enum) {
    Error.Error
    ("The type in ISMEMBER statement must be a scalarset/enum type.");
    t = errortype;
  } else if (type->gettypeclass() == typedecl::Scalarset
             && !((scalarsettypedecl *) type)->isnamed()) {
    Error.Warning("The anonymous scalarset type in ISMEMBER is useless.");
    t = errortype;
  } else {
    t = type;
  }
}

/********************
  class multisetcount
  ********************/
multisetcount::multisetcount(ste * index, designator * set, expr * filter)
  :  expr(inttype, FALSE, FALSE), index(index), set(set), filter(filter)
{
  Error.CondError(set->gettype()->gettypeclass() != typedecl::MultiSet,
                  "2nd Argument of MultiSetCount must be a MultiSet.");
  Error.CondError(!type_equal(filter->gettype(), booltype),
                  "3rd Argument of MultiSetCount must be boolean.");
  multisetcount_num = num_multisetcount;
  num_multisetcount++;
  ((multisettypedecl *) set->gettype())->addcount(this);
}

int multisetcount::num_multisetcount = 0;

/********************
  Global objects.
  ********************/
expr *error_expr = NULL;
expr *true_expr = NULL;
expr *false_expr = NULL;
designator *error_designator = NULL;

