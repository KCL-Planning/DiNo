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
//#include <values.h>

#define MULTIPL8(x) ((x+7) & ~0x0007)
#define MULTIPL32(x) ((x+31) & ~0x001f)
#define BYTES(x) ((x)>0 ? ((x)>8?32:8) : 0)

/********************
  scalarset extension : static variables for this module
  ********************/

static int scalarset_type_int = 0;	/* integer for scalarset type;
					   different scalarsets get a different integer value */

extern int enumval;
//
// static int scalarset_const_int = 5; /* integer for scalarset ids;
//                                     different ids get a different integer value;
//                                     0 is used for undefined value */

/********************
  class decl
  ********************/
typedecl *decl::gettype(void) const
{
  Error.Error("Internal: decl::gettype();");
  return errortype;
}

decl::decl()
  :
  TNode(), name(NULL), declared(FALSE)
{
  pddlattrs.pddlname = NULL;
}

decl::decl(char *name)
  :  TNode(), name(name), mu_name(NULL), declared(FALSE)
{
  pddlattrs.pddlname = NULL;
  mu_name = tsprintf("mu_%s", name);
}

designator *decl::getdesignator(ste * origin) const
{
  Error.Error("Internal--invalid call to decl::getdesignator()");
  return error_designator;
}

/********************
  class typedecl
  ********************/

typedecl *typedecl::origin = 0;

typedecl::typedecl()
  :
  decl(), scalarsetlist(NULL), structure(NoScalarset),
  bitsalloc(0),
  already_generated_permute_function(FALSE),
  already_generated_simple_canon_function(FALSE),
  already_generated_canonicalize_function(FALSE),
  already_generated_simple_limit_function(FALSE),
  already_generated_array_limit_function(FALSE),
  already_generated_limit_function(FALSE),
  already_generated_multisetlimit_function(FALSE)
{
  static int theTNum = 0;
  tNum = theTNum++;
  /* Add this typedecl into linked list */
  next = origin;
  origin = this;
};

typedecl::typedecl(char *name)
  :decl(name),
   scalarsetlist(NULL),
   structure(NoScalarset),
   bitsalloc(0), already_generated_permute_function(FALSE)
{
  static int theTNum = 0;
  tNum = theTNum++;
  /* Add this typedecl into linked list */
  next = origin;
  origin = this;
  mu_name = tsprintf("mu_%s%d", name, tNum);
};

typedecl *typedecl::getindextype() const	// for arraytypedecl
{
  Error.Error("Getting indextype from nonarray.\n");
  return errortype;
};

typedecl *typedecl::getelementtype() const	// for arraytypedecl
{
  Error.Error("Getting elementtype from nonarray.\n");
  return errortype;
};


/* AP: class realtypedecl */
realtypedecl::realtypedecl(int ac, int ex)
  :  typedecl(), accuracy(ac), exponent_value(ex)	//IM: fixed, exponent_value instead of exponent
{
  this->setexponentdigits();	//IM: sets this->exponent
  numbits = (int) ceil(this->getsize() / 2.0) * 8;
  bitsalloc = numbits;

  mu_type = "mu__real";
}

realtypedecl::realtypedecl(expr * ac, expr * ex)
  :  typedecl()
{
  Error.CondError(!type_equal(ac->gettype(), inttype),
                  "Only integer accuracy allowed.");
  Error.CondError(!type_equal(ex->gettype(), inttype),
                  "Only integer exponent allowed.");
  Error.CondError(!ac->hasvalue(), "Accuracy must be constant.");
  Error.CondError(!ex->hasvalue(), "Exponent must be constant.");
  Error.CondError((ac->getvalue() < 1) || (ac->getvalue() > DBL_DIG),
                  "Accuracy must be >= 1 and <= %d.", DBL_DIG);
  Error.CondError((ex->getvalue() < 1) || (ex->getvalue() > DBL_MAX_10_EXP),	//IM: fixed, now we have directly the value
                  "Exponent max value must be between 1 and %d.",
                  DBL_MAX_10_EXP);

  this->accuracy = ac->getvalue();
  this->exponent_value = ex->getvalue();
  this->setexponentdigits();	//IM: sets this->exponent

  this->numbits = (int) ceil(this->getsize() / 2.0) * 8;
  this->bitsalloc = this->numbits;

  mu_type = "mu__real";
}

/********************
  class enumtypedecl
  ********************/
enumtypedecl::enumtypedecl(int l, int r)
  :typedecl(), left(l), right(r)
{
//   shift = (left > 1 ? left-1 : 0);
//   this->left -= shift;
//   this->right -= shift;

  // 0 is used for undefined value --> 0. lb, ... . ub
  numbits = CeilLog2(right - left + 2);
  if (!args->no_compression) {
    bitsalloc = numbits;
  } else {
    if (numbits > 31)
      Error.Error("Internal error, range is too large");
    bitsalloc = BYTES(numbits);
    if (left < 0 || right > 254 || numbits > 8)
      bitsalloc = 32;
  }
  mu_type = (left < 0 || right > 254 || numbits > 8 ?
             "mu__long" : "mu__byte");
}

/********************
  class subrangetypedecl
  ********************/
subrangetypedecl::subrangetypedecl(int left, int right, typedecl * parent)
  :typedecl(), left(left), right(right), parent(parent)
{
  // 0 is used for undefined value --> 0. lb, ... . ub
  numbits = CeilLog2(right - left + 2);
  if (!args->no_compression) {
    bitsalloc = numbits;
  } else {
    if (numbits > 31)
      Error.Error("Internal error, range is too large");
    bitsalloc = BYTES(numbits);
    if (left < 0 || right > 254 || numbits > 8)
      bitsalloc = 32;
  }
  mu_type = (left < 0 || right > 254 || numbits > 8 ?
             "mu__long" : "mu__byte");
}

subrangetypedecl::subrangetypedecl(expr * left, expr * right)
  :  typedecl()
{
  if (Error.CondError(!type_equal(left->gettype(), inttype),
                      "Only integer subranges allowed.") ||
      Error.CondError(!type_equal(right->gettype(), inttype),
                      "Only integer subranges allowed.") ||
      Error.CondError(!left->hasvalue(),
                      "Subrange bounds must be constants.") ||
      Error.CondError(!right->hasvalue(),
                      "Subrange bounds must be constants.") ||
      Error.CondError(right->getvalue() < left->getvalue(),
                      "Upper bound of subrange less than lower bound.")
     ) {
    this->left = 0;
    this->right = 1;
    this->numbits = 1;
    if (!args->no_compression) {
      this->bitsalloc = this->numbits;
    } else {
      this->bitsalloc = BYTES(this->numbits);
    }
    this->parent = inttype;
  } else {
    this->left = left->getvalue();
    this->right = right->getvalue();

    // 0 is used for undefined value --> 0. lb, ... . ub
    this->numbits = CeilLog2(right->getvalue() - left->getvalue() + 2);
    if (this->numbits > 31)
      Error.Error("Internal error, range is too large");
    if (!args->no_compression) {
      this->bitsalloc = this->numbits;
    } else {
      this->bitsalloc = BYTES(this->numbits);
      if (this->left < 0 || this->right > 254 || numbits > 8)
        this->bitsalloc = 32;
    }
    this->parent = left->gettype();
    // more general than inttype, though not yet necessary.
  }
  mu_type = (left->getvalue() < 0 || right->getvalue() > 254
             || numbits > 8 ? "mu__long" : "mu__byte");
}

/********************
  class arraytypedecl
  ********************/
arraytypedecl::arraytypedecl(bool interleaved,
                             typedecl * indextype, typedecl * elementtype)
  :
  typedecl(), interleaved(interleaved), indextype(indextype),
  elementtype(elementtype)
{
  Error.CondError(!indextype->issimple(),
                  "Array index type must be a simple type.");
  if (elementtype->name == NULL)
    symtab->declare_global(ltable.enter(tsprintf("_type_%d", new_int()
                                                )
                                       ), elementtype);
  numbits = indextype->getsize() * elementtype->getbitsalloc();
  bitsalloc = numbits;

  // classify array type according to scalarset involvement
  if (indextype->getstructure() == typedecl::NoScalarset)
    structure = elementtype->getstructure();
  else if (indextype->getstructure() == typedecl::ScalarsetVariable)
    if (elementtype->getstructure() == typedecl::NoScalarset)
      structure = typedecl::ScalarsetArrayOfFree;
    else if (elementtype->getstructure() == typedecl::ScalarsetVariable)
      structure = typedecl::ScalarsetArrayOfScalarset;
    else
      structure = typedecl::Complex;
  else
    Error.Error("Complex type as index to array.");

}

/********************
  class multisettypedecl
  ********************/
multisettypedecl::multisettypedecl(bool interleaved,
                                   expr * e, typedecl * elementtype)
  :
  typedecl(), interleaved(interleaved), elementtype(elementtype),
  msclist(NULL)
{
  Error.CondError(!e->hasvalue(),
                  "CONST declaration requires constant expression.");
  maximum_size = e->getvalue();

  if (elementtype->name == NULL)
    symtab->declare_global(ltable.enter(tsprintf("_type_%d", new_int()
                                                )
                                       ), elementtype);

  if (!args->no_compression) {
    numbits = maximum_size * elementtype->getbitsalloc()
              + maximum_size * 2;
  } else {
    numbits = maximum_size * elementtype->getbitsalloc()
              + maximum_size * 8;
  }

  bitsalloc = numbits;
  mu_type = (maximum_size > 254 ? "mu__long" : "mu__byte");

// CeilLog2(maximum_size+2);

  // classify array type according to scalarset involvement
  if (elementtype->getstructure() == typedecl::NoScalarset)
    structure = typedecl::MultisetOfFree;
  //   else if ( elementtype->getstructure() == typedecl::ScalarsetVariable )
  //  structure = typedecl::MultisetOfScalarset;
  else
    structure = typedecl::Complex;

}

/********************
  class recordtypedecl
  ********************/
recordtypedecl::recordtypedecl(bool interleaved, ste * fields)
  :  typedecl(), interleaved(interleaved), fields(fields)
{
  ste *s;
  typedecl *t;
  int sum = 0;

  for (s = fields; s != NULL; s = s->getnext()) {
    sum += (((vardecl *) (s->getvalue()))->gettype()->getbitsalloc());
    t = ((vardecl *) (s->getvalue()))->gettype();
    switch (t->getstructure()) {
      case NoScalarset:
        break;
      case ScalarsetVariable:
        if (structure == typedecl::NoScalarset)
          structure = typedecl::ScalarsetVariable;
        break;
      case ScalarsetArrayOfFree:
        if (structure == typedecl::NoScalarset
            || structure == typedecl::ScalarsetVariable)
          structure = typedecl::ScalarsetArrayOfFree;
        break;
      case MultisetOfFree:
        if (structure == typedecl::NoScalarset
            || structure == typedecl::ScalarsetVariable
            || structure == typedecl::ScalarsetArrayOfFree)
          structure = typedecl::MultisetOfFree;
        break;
      case ScalarsetArrayOfScalarset:
        if (structure != typedecl::Complex)
          structure = typedecl::ScalarsetArrayOfScalarset;
        break;
      case Complex:
        structure = typedecl::Complex;
        break;
    }
  }
  numbits = sum;
  bitsalloc = numbits;
}

/********************
  class scalarsettypedecl
  ********************/
scalarsettypedecl::scalarsettypedecl(expr * l, int lb)
  :typedecl(), named(FALSE), lexname(NULL), useless(TRUE)
{
  if (Error.CondError(!type_equal(l->gettype(), inttype),
                      "Only scalarsets of integer size allowed.") ||
      Error.CondError(!l->hasvalue(), "Scalarset size must be constants.")
     ) {
    left = lb;
    right = lb;
    numbits = 1;
    if (!args->no_compression) {
      bitsalloc = numbits;
    } else {
      bitsalloc = BYTES(numbits);
      if (left < 0 || right > 254 || numbits > 8)
        bitsalloc = 32;
    }
    mu_type = (left < 0 || right > 254 || numbits > 8 ?
               "mu__long" : "mu__byte");
    idvalues = symtab->declare(new lexid(tsprintf("scalarset_%u_v_error",
                                         scalarset_type_int++),
                                         0), new constdecl(lb++, this));
    // it is not set as scalarset variable because it is of size 1.
    // structure = typedecl::ScalarsetVariable;
    scalarsetlist = NULL;	// to be set when declaring ID : typeExpr
  } else {
    // setting size, numbits, left and right
    // const 0 is used for undefined value --> 0. lb, ... . ub
    int size = l->getvalue();
    if (size < 1)
      Error.Error("Scalarset size must be greater than zero.");
    numbits = CeilLog2(l->getvalue() + 1);
    left = lb;
    right = left + size - 1;
    if (!args->no_compression) {
      bitsalloc = numbits;
    } else {
      if (numbits > 31)
        Error.Error("Internal error, range is too large");
      bitsalloc = BYTES(numbits);
      if (left < 0 || right > 254 || numbits > 8)
        bitsalloc = 32;
    }
    mu_type = (left < 0 || right > 254 || numbits > 8 ?
               "mu__long" : "mu__byte");

    // set id strings
    // name may be changed if it is later explicitly given a type name
    int value = left;
    for (int i = 1; i <= size; i++) {
      symtab->declare(new lexid(tsprintf("scalarset_%u_v%u",
                                         scalarset_type_int, i), 0),
                      new constdecl(value++, this));
    }
    idvalues = symtab->getscope();
    if (size > 1)		// scalarset of size 1 is treated as normal enum
      structure = typedecl::ScalarsetVariable;
    scalarsetlist = NULL;	// to be set when declaring ID : typeExpr
  }
}

void scalarsettypedecl::setupid(lexid * n)
{
  int i;
  ste *v;

  // set up type name
  setname(n);

  // rename id names
  for (i = getsize(), v = idvalues; i >= 1; i--, v = v->getnext()) {
    v->setname(new lexid(tsprintf("%s_%u", n->getname(), i), 0));
    v->getvalue()->name = v->getname()->getname();
    v->getvalue()->mu_name = tsprintf("mu_%s", v->getvalue()->name);
  }

}

/********************
  class uniontypedecl
  ********************/
uniontypedecl::uniontypedecl(stelist * unionmembers)
  :  typedecl(), unionmembers(unionmembers), useless(TRUE)
{
  stelist *s;
  typedecl *d;
  int sumsize = 0;

  // get total size
  for (s = unionmembers; s != NULL; s = s->next) {
    sumsize += ((scalarsettypedecl *) (s->s->getvalue()))->getsize();
  }
  size = sumsize;

  // 0 is used for undefined value --> 0. lb, ... . ub
  numbits = CeilLog2(size + 1);
  if (!args->no_compression) {
    bitsalloc = numbits;
  } else {
    bitsalloc = MULTIPL8(numbits);
    if (size > 254 || numbits > 8)
      bitsalloc = MULTIPL32(numbits);
  }
  mu_type = (size > 254 || numbits > 8 ? "mu__long" : "mu__byte");

  // check the existance of scalarset member with size > 1
  for (s = unionmembers; s != NULL; s = s->next) {
    d = (typedecl *) s->s->getvalue();
    if (d->gettypeclass() == typedecl::Scalarset && d->getsize() > 1)
      structure = typedecl::ScalarsetVariable;
  }

  scalarsetlist = unionmembers;

  unionwithscalarset = FALSE;
  for (stelist * member = unionmembers; member != NULL;
       member = member->next)
    if (((typedecl *) member->s->getvalue())->gettypeclass() ==
        typedecl::Scalarset)
      unionwithscalarset = TRUE;
}

/********************
  class constdecl
  ********************/
constdecl::constdecl(int value, typedecl * type)
  :decl(), value(value), type(type)
{
}

constdecl::constdecl(expr * e)
  :  decl(), type(e->gettype())
{
  Error.CondError(!e->hasvalue(),
                  "CONST declaration requires constant expression.");
  if (type_equal(e->gettype(), realtype))
    rvalue = e->getrvalue();	// AP: now, a constant may be a real one
  else
    value = e->getvalue();
}

/********************
  class vardecl
  ********************/
vardecl::vardecl(typedecl * type)
  :  decl(), type(type)
{
  if (type->name == NULL)
    symtab->declare_global(ltable.enter(tsprintf("_type_%d",
                                        new_int())), type);
  offset =::offset;
  ::offset += type->getbitsalloc();
  if (args->no_compression) {
    if (type->getbitsalloc() % 8 != 0)
      Error.Error("Internal error, byte aligned allocation failed.");
  }
}


/********************
  class aliasdecl
  ********************/
aliasdecl::aliasdecl(expr * ref)
  :  decl(), ref(ref)
{
}

/********************
  class choosedecl
  ********************/
choosedecl::choosedecl(typedecl * type)
  :  decl(), type(type)
{
  Error.CondError(type->gettypeclass() != typedecl::MultiSetID,
                  "'Choose ID: ...' must use a multiset variable.");
}

/********************
  class quantdecl
  ********************/
quantdecl::quantdecl(typedecl * type)
  :  decl(), type(type), left(0), right(0), by(0)
{
  Error.CondError(!type->issimple(),
                  "Type of 'FOR ID: type ...' must be a simple type.");
}

quantdecl::quantdecl(expr * left, expr * right, int by)
  :decl(), type(inttype), left(left), right(right), by(by), byR(by)
{
  Error.CondError(!type_equal(left->gettype(), inttype)
                  && !type_equal(left->gettype(), realtype),
                  "Bounds of <expr> TO <expr> must be either integers or reals.");
  Error.CondError(!type_equal(right->gettype(), inttype)
                  && !type_equal(left->gettype(), realtype),
                  "Bounds of <expr> TO <expr> must be either integers or reals.");
}

quantdecl::quantdecl(expr * left, expr * right, double byR)
  :decl(),
   type(inttype), left(left), right(right), by((int) ceil(byR)), byR(byR)
{
  Error.CondError(!type_equal(left->gettype(), inttype)
                  && !type_equal(left->gettype(), realtype),
                  "Bounds of <expr> TO <expr> must be either integers or reals.");
  Error.CondError(!type_equal(right->gettype(), inttype)
                  && !type_equal(left->gettype(), realtype),
                  "Bounds of <expr> TO <expr> must be either integers or reals.");
}

/********************
  class param
  ********************/
param::param(typedecl * type)
  :  vardecl(type)
{
}

/********************
  class valparam
  ********************/
valparam::valparam(typedecl * type)
  :  param(type)
{
}

/********************
  class varparam
  ********************/
varparam::varparam(typedecl * type)
  :  param(type)
{
}

/********************
  class constparam
  ********************/
constparam::constparam(typedecl * type)
  :  param(type)
{
}

/********************
  class procdecl
  ********************/
procdecl::procdecl(ste * params, ste * decls, stmt * body)
  :  decl(), params(params), decls(decls), body(body), extern_def(FALSE), include_file_ext(NULL)	// im: added extern_def and include_file_ext
{
}

procdecl::procdecl()
  :  decl(), params(NULL), decls(NULL), body(NULL), extern_def(FALSE), include_file_ext(NULL)	// im: added extern_def and include_file_ext
{
}

/********************
  class funcdecl
  ********************/
funcdecl::funcdecl(ste * params,
                   ste * decls,
                   stmt * body, typedecl * returntype, bool sideeffects)
  :
  procdecl(params, decls, body), returntype(returntype),
  sideeffects(sideeffects)
{
}

funcdecl::funcdecl()
  :  procdecl(), returntype(NULL), sideeffects(FALSE)
{
}

/********************
  class error_decl
  ********************/
typedecl *error_decl::gettype() const
{
  return errortype;
}

designator *error_decl::getdesignator(ste * origin) const
{
  return error_designator;
}

/********************
  Global objects.
  ********************/
typedecl *booltype = NULL;
typedecl *inttype = NULL;
typedecl *realtype = NULL;	// AP: realtype's initialization
errortypedecl *errortype = NULL;
typedecl *voidtype = NULL;
param *errorparam = NULL;
error_decl *errordecl = NULL;
