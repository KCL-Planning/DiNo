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
#include <typeinfo>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <iterator>
#include <map>

#define TIME_INFINITY 999999

// WP WP WP WP WP WP WP WP WP WP
bool ff_enabled = true;


/* NOTE:

   Beware of declarations introduced not at the beginnings
   of scopes.

   Why are expressions all handled with switches instead of
   virtual functions?
*/

/*
 * Each generate_code() member function outputs to stdout the code
 * necessary to evaluate that thing, and returns a string with
 * what is necessary to use it as a value.
 *
 * Some examples:
 *
 * The statement "A := foo" sends this to stdout:
 *   A = foo;
 * And returns the null string.
 *
 * the expression "A[i]" sends nothing to stdout and returns
 * the string "A[i]".
 *
 * A more complicated example, the real motivation for this scheme:
 * The expression "FORALL i : 0..1 DO boolarray[i] END;"
 * sends something like the following to stdout, which is what is
 * required to compute the forall:
 *
 *   mu__forall_1 = TRUE;
 *   {
 *     for(i = 0; i <= 1; i++)
 *       {
 *	 if ( !(boolarray[i]) )
 *       {
 *	 mu__forall_1 = FALSE;
 *	 break;
 *       }
 *       }
 *   }
 *
 * and then it returns "mu__forall_1", which is what\'s necessary to
 * use the value of the forall.
 *
 */

/*
 * Standard hack:
 * in many cases, we have to generate a list of things separated by commas.
 * Usually, we do this by sprintf-ing ", x" into a buffer for each x,
 * and then using (buffer + 1) as the value to use, so we skip the
 * comma at the betginning.
 */

/*
 * Debugging hint: to check whether all the parentheses and braces
 * this generating match, open the generated code in emacs in
 * c++-mode, put set the mark at one end and point at the other, and
 * do C-M-\ (indent-region) to reindent all the lines.  If the last
 * line of the generated code doesn\'t end in column 1, your braces
 * or parentheses don\'t match up somewhere.
 */

/********************
  code related to decl
  --
  -- typedecl
  -- -- enumtypedecl
  -- -- subrangetypedecl
  -- -- arraytypedecl
  -- -- recordtypedecl
  -- -- scalarsettypedecl
  -- -- uniontypedecl
  -- constdecl
  -- vardecl
  -- aliasdecl
  -- quantdecl
  -- parameter
  -- -- param
  -- -- varparam
  -- -- valparam
  -- -- constparam
  -- procdecl
  -- funcdecl
  ********************/

/********************
  code for decl
  ********************/
const char *decl::generate_decl()
{
  Error.Error
  ("Internal:  decl::generate_decl() should not have been called.");
  return "ERROR!";
}

const char *decl::generate_code()
{
  return mu_name;
}

/********************
  code for typedecl
 ********************/
const char *typedecl::generate_decl()
{
  Error.Error
  ("Internal:  typedecl::generate_code() should never have been called.");
  return "ERROR!";
}

void typedecl::generate_all_decl()
{
  if (next != NULL) {
    next->generate_all_decl();
  }
  generate_decl();
}

/* AP: code for realtypedecl */
const char *realtypedecl::generate_decl()
{
  if (!declared) {
    /* Invent a name for the object, and a "mu_name" for
       the declaration of the object in the generated code. */
    if (name == NULL) {
      name = tsprintf("_real_%d", tNum);
      mu_name = tsprintf("mu_%s", name);
    }
    /* class name */
    fprintf(codefile, "class %s: public %s\n" "{\n" " public:\n" "  inline double operator=(double val) { return %s::operator=(val); };\n" "  inline double operator=(const %s& val) { return %s::operator=((double) val); };\n" "  %s (const char *name, int os): %s(%d,%d,%d,name, os) {};\n" "  %s (void): %s(%d,%d,%d) {};\n" "  %s (double val): %s(%d,%d,%d,\"Parameter or function result.\", 0)\n" "  {\n" "    operator=(val);\n" "  };\n" "  char * Name() { return tsprintf(\"%%le\",value()); };\n", mu_name, mu_type,	/* class name */
            mu_type, mu_name, mu_type, mu_name, mu_type, accuracy, exponent_value, numbits,	//IM: fixed, exponent_value instead of exponent
            mu_name, mu_type, accuracy, exponent_value, numbits,	//IM: fixed, exponent_value instead of exponent
            mu_name, mu_type, accuracy, exponent_value, numbits	//IM: fixed, exponent_value instead of exponent
           );
    theprog->symmetry.generate_symmetry_function_decl();
    fprintf(codefile,
            "  virtual void MultisetSort() {};\n"
            "  void print_statistic() {};\n" "};\n\n");
    fprintf(codefile, "/*** end of real decl ***/\n");
    fprintf(codefile, "%s %s_undefined_var;\n\n", mu_name, mu_name);
    declared = TRUE;
  }
  /* Should never use this as a return value expression in the code gen */
  return "ERROR!";
}

/********************
  code for enumtypedecl
  Dill: I think this is bogusly ending each elt with ",", necessitating
   a "NULL" element at the end of the list.
 ********************/
void make_enum_idvalues(ste * idvalues, typedecl * thistype)
{
  if (idvalues != NULL &&
      idvalues->getvalue()->getclass() == decl::Const &&
      type_equal(idvalues->getvalue()->gettype(), thistype)) {
    make_enum_idvalues(idvalues->getnext(), thistype);
    fprintf(codefile, "\"%s\",", idvalues->getname()->getname());
  }
}

const char *enumtypedecl::generate_decl()
{
  if (!declared) {
    /* Invent a name for the object, and a "mu_name" for
       the declaration of the object in the generated code. */
    if (name == NULL) {
      name = tsprintf("_enum_%d", tNum);
      mu_name = tsprintf("mu_%s", name);
    }

    /* class name */
    fprintf(codefile,
            "class %s: public %s\n"
            "{\n"
            " public:\n"
            "  inline int operator=(int val) { return value(val); };\n"
            // Uli: return added
            "  inline int operator=(const %s& val)"
            " { return value(val.value()); };\n"
            "  static const char *values[];\n"
            "  friend ostream& operator<< (ostream& s, %s& val)\n"
            "  {\n"
            "    if (val.defined())\n"
            "      return ( s << %s::values[ int(val) - %d] );\n"
            "    else return ( s << \"Undefined\" );\n"
            "  };\n\n"
            "  %s (const char *name, int os): %s(%d, %d, %d, name, os) {};\n"
            "  %s (void): %s(%d, %d, %d) {};\n"
            "  %s (int val): %s(%d, %d, %d, \"Parameter or function result.\", 0)\n"
            "  {\n"
            "     operator=(val);\n"
            "  };\n"
            "  const char * Name() { return values[ value() -%d]; };\n"
//      "  inline int operator=(int val) { return %s::operator=(val); };\n"
//      "  inline int operator=(const %s& val) { return %s::operator=((int) val); };\n"
            , mu_name, mu_type,	/* class name */
            mu_name, mu_name,	/* type name for second arg to << */
            mu_name, left,	/* class preceding values in body of <<, and offset */
            mu_name, mu_type, left, right, numbits,	/* args for first constructor */
            mu_name, mu_type, left, right, numbits,	/* args for second constructor */
            mu_name, mu_type, left, right, numbits,	/* args for third constructor */
            left		/* name */
           );
    theprog->symmetry.generate_symmetry_function_decl();
    fprintf(codefile, "  virtual void MultisetSort() {};\n" "  void print_statistic() {};\n" "  virtual void print(FILE *target, const char *separator)\n" "  {\n" "    if (defined())\n" "    fprintf(target,\"%%s: %%s%%s\",name,values[ value() -%d],separator); \n" "    else\n" "    fprintf(target,\"%%s: Undefined%%s\",name,separator); \n" "  };\n" "};\n\n" "const char *%s::values[] = {", left,	/* print() */
            mu_name);		/* for values array near the end */

    make_enum_idvalues(idvalues, this);
    // Dill: an extra NULL in case the enum has only one element.
    //       ? Why is this necessary? Because of printing "," after last element?
    fprintf(codefile, "NULL };\n\n");
    fprintf(codefile, "/*** end of enum declaration ***/\n");
    fprintf(codefile, "%s %s_undefined_var;\n\n", mu_name, mu_name);
    declared = TRUE;
  }
  /* Should never use this as a return value expression in the code gen */
  return "ERROR!";
}

/********************
  code for subrangetypedecl
 ********************/

const char *subrangetypedecl::generate_decl()
{
  /* if decl has not already been generated... */
  if (!declared) {
    /* make up a name for a new class, and a mu_name to be output. */
    if (name == NULL) {
      name = tsprintf("_subrange_%d", tNum);
      mu_name = tsprintf("mu_%s", name);
    }

    fprintf(codefile,
            /* Indented so we can see what the output looks like: */
            /* (adjacent strings are concatenated in C++!) */
            "class %s: public %s\n"
            "{\n"
            " public:\n"
            "  inline int operator=(int val) { return %s::operator=(val); };\n"
            "  inline int operator=(const %s& val) { return %s::operator=((int) val); };\n"
            /* three initializers */
            "  %s (const char *name, int os): %s(%d, %d, %d, name, os) {};\n" "  %s (void): %s(%d, %d, %d) {};\n" "  %s (int val): %s(%d, %d, %d, \"Parameter or function result.\", 0)\n" "  {\n" "    operator=(val);\n" "  };\n" "  char * Name() { return tsprintf(\"%%d\",value()); };\n", mu_name, mu_type,	/* class name */
            mu_type,
            mu_name, mu_type, mu_name, mu_type, left, right, numbits,
            /* args for first constructor */
            mu_name, mu_type, left, right, numbits,
            /* args for second constructor */
            mu_name, mu_type, left, right, numbits);
    theprog->symmetry.generate_symmetry_function_decl();
    fprintf(codefile,
            "  virtual void MultisetSort() {};\n"
            "  void print_statistic() {};\n" "};\n\n");
    fprintf(codefile, "/*** end of subrange decl ***/\n");
    fprintf(codefile, "%s %s_undefined_var;\n\n", mu_name, mu_name);

    declared = TRUE;
  }
  return "ERROR!";
}



/********************
  code for arraytypedecl
  Dill:  HERE IS ONE OF THE EVIL TSPRINTFS!
 ********************/
int make_elt_ref_by_union(stelist * unionmembers)
{
  int base = 0;
  if (unionmembers->next != NULL)
    base = make_elt_ref_by_union(unionmembers->next);
  typedecl *t = (typedecl *) unionmembers->s->getvalue();
  fprintf(codefile,
          "    if ( ( index >= %d ) && ( index <= %d ) )\n"
          "      return array[ index - (%d) ];\n",
          t->getleft(), t->getright(), t->getleft() - base);
  return base + t->getsize();
}

void make_elt_with_scalarset(ste * idvalues, int size, int numbits)
{
  if (size > 1)
    make_elt_with_scalarset(idvalues->next, size - 1, numbits);
  fprintf(codefile,
          "if (n) array[i].set_self_ar(n,\"%s\", i * %d + os); else array[i].set_self_ar(NULL, NULL, 0); i++;\n",
          idvalues->getname()->getname(), numbits);
}

void make_elt_with_enum(ste * idvalues, int size, int numbits)
{
  if (idvalues == NULL & size == 2) {
    fprintf(codefile,
            "if (n) array[i].set_self_ar(n,\"false\", i * %d + os); else array[i].set_self_ar(NULL, NULL, 0); i++;\n",
            numbits);
    fprintf(codefile,
            "if (n) array[i].set_self_ar(n,\"true\", i * %d + os); else array[i].set_self_ar(NULL, NULL, 0); i++;\n",
            numbits);
  } else if (idvalues != NULL) {
    if (size > 1)
      make_elt_with_enum(idvalues->next, size - 1, numbits);
    fprintf(codefile,
            "if (n) array[i].set_self_ar(n,\"%s\", i * %d + os); else array[i].set_self_ar(NULL, NULL, 0); i++;\n",
            idvalues->getname()->getname(), numbits);
  } else
    Error.Error("Internal Error: make_elt_with_enum");
}

void make_elt_with_union(stelist * unionmembers, int numbits)
{
  if (unionmembers->next != NULL)
    make_elt_with_union(unionmembers->next, numbits);
  if (((typedecl *) unionmembers->s->getvalue())->gettypeclass() ==
      typedecl::Scalarset)
    make_elt_with_scalarset(((scalarsettypedecl *) unionmembers->
                             s->getvalue())->getidvalues(),
                            unionmembers->s->getvalue()->getsize(),
                            numbits);
  else if (((typedecl *) unionmembers->s->getvalue())->gettypeclass() ==
           typedecl::Enum)
    make_elt_with_enum(((enumtypedecl *) unionmembers->s->
                        getvalue())->getidvalues(),
                       unionmembers->s->getvalue()->getsize(), numbits);
  else
    Error.Error("funny element in union");
}

const char *arraytypedecl::generate_decl()
{
  if (!declared) {
    if (name == NULL) {
      name = tsprintf("_array_%d", tNum);
      mu_name = tsprintf("mu_%s", name);
    }




    /* declare class with set_self, constructor and destructor
     *
     *
     * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
     *
     * ADDED THE SUPERCLASS TO DECLARATION OF THE NEW TYPE ARRAY CLASS TO EASIER GET THE COMPLEX VARIABLES FROM THE COMPILED MODEL!
     *
     * TODO: CREATE A FIELD IN EACH OF THE NEWLY CREATED TYPES TO SHOW HOW MANY DIMENSIONS DOES THE ARRAY OF BOOLEANS HAVE!!!!!!!!!!!!!!!!!!!!!!!!
     *
     * */


    fprintf(codefile, "class %s/*:public mu_1__type_super*/\n" "{\n" " public:\n" "  %s array[ %d ]; \n#define awesome_mu_00_%s_%s 1 \n" " public:\n" "  char *name;\n" "  char longname[BUFFER_SIZE/4];\n" "  void set_self( const char *n, int os);\n" "  void set_self_2( const char *n, const char *n2, int os);\n" "  void set_self_ar( const char *n, const char *n2, int os);\n" "  %s (const char *n, int os) { set_self(n, os); };\n" "  %s ( void ) {};\n" "  virtual ~%s ();\n", mu_name,	/* class name */
            elementtype->generate_code(),	/* array elt type */
            indextype->getsize(),	/* array size */
	    elementtype->generate_code(),	/* array elt type for #define awesome*/
	    mu_name,		/* class name for #define awesome */
            mu_name,		/* name for first constructor */
            mu_name,		/* name for second constructor */
            mu_name);		/* destructor name */

    /* declare range-checked operator [] */
    fprintf(codefile,
            "  %s& operator[] (int index) /* const */\n"
            "  {\n", elementtype->generate_code());
    switch (indextype->gettypeclass()) {
      case typedecl::Enum:
      case typedecl::Range:
        fprintf(codefile, "#ifndef NO_RUN_TIME_CHECKING\n" "    if ( ( index >= %d ) && ( index <= %d ) )\n" "      return array[ index - %d ];\n" "    else {\n" "      if (index==UNDEFVAL) \n" "	Error.Error(\"Indexing to %%s using an undefined value.\", name);\n" "      else\n" "	Error.Error(\"%%d not in index range of %%s.\", index, name);\n" "      return array[0];\n" "    }\n" "#else\n" "    return array[ index - %d ];\n" "#endif\n" "  };\n", indextype->getleft(),	/* lower bound of range (2nd line of format) */
                indextype->getright(),	/* upper bound */
                indextype->getleft(),	/* index adjust (3rd line) */
                indextype->getleft());	/* index adjust (after #else) */
        break;
      case typedecl::Scalarset:
        fprintf(codefile, "#ifndef NO_RUN_TIME_CHECKING\n" "    if ( ( index >= %d ) && ( index <= %d ) )\n" "      return array[ index - %d ];\n" "    else\n" "      {\n" "	if (index==UNDEFVAL) \n" "	  Error.Error(\"Indexing to %%s using an undefined value.\", name);\n" "	else\n" "	  Error.Error(\"Funny index value %%d for %%s: %s is internally represented from %d to %d.\\n" "Internal Error in Type checking.\",index, name);\n" "	return array[0];\n" "      }\n" "#else\n" "    return array[ index - %d ];\n" "#endif\n" "  };\n", indextype->getleft(),	/* lower bound of range (2nd line of format) */
                indextype->getright(),	/* upper bound */
                indextype->getleft(),	/* index adjust (3rd line) */
                indextype->name, indextype->getright(), indextype->getleft(), indextype->getleft());	/* index adjust (after #else) */
        break;
      case typedecl::Union:
        (void)
        make_elt_ref_by_union(((uniontypedecl *)
                               indextype)->getunionmembers());
        fprintf(codefile,
                "    if (index==UNDEFVAL) \n"
                "      Error.Error(\"Indexing to %%s using an undefined value.\", name);\n"
                "    else\n"
                "      Error.Error(\"Funny index value %%d for %%s. (Internal Error in Type Checking.\",index, name);\n"
                "    return array[0];\n" "  }\n");
        break;
      case typedecl::Array:
      case typedecl::Record:
      case typedecl::Error_type:
      default:
        // the error should already be flagged.

        // On the other hand, if this error happened, it should never
        // have gotten here. Therefore, we will put an error here, because
        // redundant error-checking is never a waste. --RLM
        Error.Error("Internal: Odd value for arraydecl::elementtype;");
        break;
    }

    /* and an operator =. */
    generate_assign();
//    if (elementtype->issimple())
//      fprintf(codefile,
//      "  %s& operator= (const %s& from)\n"
//      "  {\n"
//      "    for (int i = 0; i < %d; i++)\n"
//      "      if (from.array[i].isundefined() )\n"
//      "       array[i].undefine();\n"
//      "      else\n"
//      "       array[i] = from.array[i];\n"
//      "    return *this;\n"
//      "  }\n\n",
//      mu_name,
//      mu_name,
//      indextype->getsize());
//   else
//      fprintf(codefile,
//      "  %s& operator= (const %s& from)\n"
//      "  {\n"
//      "    for (int i = 0; i < %d; i++)\n"
//      "      array[i] = from.array[i];\n"
//      "    return *this;\n"
//      "  }\n\n",
//      mu_name,
//      mu_name,
//      indextype->getsize());

    /* comparsion function */
    if (indextype->getstructure() == typedecl::ScalarsetVariable)
      fprintf(codefile,
              "friend int CompareWeight(%s& a, %s& b)\n"
              "  {\n" "    return 0;\n" "  }\n", mu_name, mu_name);
    else
      fprintf(codefile,
              "friend int CompareWeight(%s& a, %s& b)\n"
              "  {\n"
              "    int w;\n"
              "    for (int i=0; i<%d; i++) {\n"
              "      w = CompareWeight(a.array[i], b.array[i]);\n"
              "      if (w!=0) return w;\n"
              "    }\n"
              "    return 0;\n"
              "  }\n", mu_name, mu_name, indextype->getsize()
             );

    /* comparsion function */
    fprintf(codefile,
            "friend int Compare(%s& a, %s& b)\n"
            "  {\n"
            "    int w;\n"
            "    for (int i=0; i<%d; i++) {\n"
            "      w = Compare(a.array[i], b.array[i]);\n"
            "      if (w!=0) return w;\n"
            "    }\n"
            "    return 0;\n"
            "  }\n", mu_name, mu_name, indextype->getsize()
           );

    /* declare permute() */
    theprog->symmetry.generate_symmetry_function_decl();
    fprintf(codefile,
            "  virtual void MultisetSort()\n"
            "  {\n"
            "    for (int i=0; i<%d; i++)\n"
            "      array[i].MultisetSort();\n" "  }\n",
            indextype->getsize()
           );
    fprintf(codefile,
            "  void print_statistic()\n"
            "  {\n"
            "    for (int i=0; i<%d; i++)\n"
            "      array[i].print_statistic();\n"
            "  }\n", indextype->getsize()
           );

    /* standard functions:
     * clear(), undefine(), to_state() */
    fprintf(codefile,
            "  void clear() { for (int i = 0; i < %d; i++) array[i].clear(); };\n\n"
            "  void undefine() { for (int i = 0; i < %d; i++) array[i].undefine(); };\n\n"
            "  void reset() { for (int i = 0; i < %d; i++) array[i].reset(); };\n\n"
            "  void to_state(state *thestate)\n"
            "  {\n"
            "    for (int i = 0; i < %d; i++)\n"
            "      array[i].to_state(thestate);\n" "  };\n\n",
//      "  void from_state(state *thestate)\n"
//      "  {\n"
//      "    for (int i = 0; i < %d; i++)\n"
//      "      array[i].from_state(thestate);\n"
//      "  };\n\n",
            indextype->getsize(),	/* body of clear */
            indextype->getsize(),	/* body of undefine */
            indextype->getsize(),	/* body of reset */
            indextype->getsize()	/* body of to_state */
            //      indextype->getsize()  /* body of from_state */
           );






/*
    	 *  WP WP WP WP WP WP WP WP WP WP WP
    	 *
    	 * THIS CODE IS CREATED IN EVERY SINGLE NEWLY CREATED mu_1__type_X OBJECT!
    	 * (HAVEN'T FIGURED WHERE DO THE OTHER CODE GENERATION SNIPPETS ACTUALLY PUT THE CODE)
    	 * AND I'VE PUT THE CODE EVERYWHERE. AND I MEAN E-V'RY-WHEEEEEEEEEREEE! )
    	 *
    	 * */

    if (ff_enabled == true){


		/*
		 *  WP WP WP WP WP WP WP WP WP WP WP
		 *
		 * THIS CODE WILL GO THROUGH EACH mu_1_type_X OBJECT AND RETURN A VECTOR OF
		 * SINGLE mu_0_boolean VARS SO THEIR VALUES CAN EASILY BE CHECKED REGARDLESS OF
		 * THE NUMBER OF PARAMETERS IN THE PREDICATE.
		 *
		 *
		 * */

        fprintf(codefile,
		"  std::vector<mu_0_boolean*> bool_array() {\n\n"
    	"	std::vector<mu_0_boolean*> barr;\n"
    	""
		"	#ifdef awesome_mu_00_mu_0_boolean_%s\n"
    	"		for (int ix = 0; ix < (sizeof((array))/sizeof(*(array))); ix++){\n"
    	"			std::string stype = typeid(array[ix]).name();\n"
    	"			if (stype.compare(\"12mu_0_boolean\") == 0)\n"
    	"				barr.push_back(&(array[ix]));\n "
    	"		}\n"
    	"		return barr;\n\n"


        "	#elif awesome_mu_00_mu_1_TIME_type_%s\n"
//        "		std::string stype = typeid(array[0]).name();\n"
        "		return barr; \n\n"


		"	#else \n"
		""
		"		#ifdef awesome_mu_00_mu_1_real_type_%s\n"
		"			return barr;\n"
		"	   		\n\n"
		""
		"		#else \n"
		"			for (int ix = 0; ix < (sizeof((array))/sizeof(*(array))); ix++){\n"
		"				 std::vector<mu_0_boolean*> temp_b = array[ix].bool_array();\n"
		"				 barr.insert(barr.end(), temp_b.begin(), temp_b.end());\n "
		"			}\n"
		"			return barr;\n"
		"	   		\n"
		"		#endif \n"
		"	#endif \n}\n",

	    mu_name,				/* WP: CLASS NAME FOR bool_array in #ifdef mu_1_real_type */
		mu_name,				/* WP: CLASS NAME FOR bool_array in #elseif mu_1_TIME_type */
	    mu_name					/* WP: CLASS NAME FOR bool_array in #else #ifdef mu_0_boolean */
//	    indextype->getsize(),	/* body of bool_array in #ifdef */
//	    indextype->getsize(),	/* body of bool_array in #else */
//	    indextype->getsize()	/* body of bool_array in #else */
	);

    }


    if (ff_enabled == true){


		/*
		 *  WP WP WP WP WP WP WP WP WP WP WP WP
		 *
		 * THIS CODE WILL GO THROUGH EACH mu_1_type_X OBJECT AND RETURN A VECTOR OF
		 * SINGLE mu_1_real_type VARS SO THEIR VALUES CAN EASILY BE CHECKED REGARDLESS OF
		 * THE NUMBER OF PARAMETERS IN THE PREDICATE.
		 *
		 *
		 **/

        fprintf(codefile,
		"  std::vector<mu__real*> num_array() {\n\n"
    	"	std::vector<mu__real*> narr;\n"
    	""

		"	#ifdef awesome_mu_00_mu_1_real_type_%s\n"
		"	for (int ix = 0; ix < (sizeof((array))/sizeof(*(array))); ix++){\n"
		"		std::string stype = typeid(array[ix]).name();\n"
		"		if (stype.compare(\"14mu_1_real_type\") == 0)\n"
		"			narr.push_back(&(array[ix]));\n "
		"	}\n"
		"		return narr;\n"
		"	\n\n"


		"	#elif awesome_mu_00_mu_1_TIME_type_%s\n"
//		"		std::string stype = typeid(array[0]).name();\n"
		"		return narr; \n\n"


		"	#else \n"
		""
		"		#ifdef awesome_mu_00_mu_0_boolean_%s\n"
		"			return narr;\n\n"
        ""
		"		#else \n"
		"			for (int ix = 0; ix < (sizeof((array))/sizeof(*(array))); ix++){\n"
		"				 std::vector<mu__real*> temp_n = array[ix].num_array();\n"
		"				 narr.insert(narr.end(), temp_n.begin(), temp_n.end());\n "
		"			}\n"
		"			return narr;\n"
		"	   		\n\n"
		"		#endif \n"
		"	#endif \n}\n",

	    mu_name,				/* WP: CLASS NAME FOR num_array in #ifdef mu_1_real_type */
		mu_name,				/* WP: CLASS NAME FOR num_array in #elseif mu_1_TIME_type */
	    mu_name					/* WP: CLASS NAME FOR num_array in #else #ifdef mu_0_boolean */
//	    indextype->getsize(),	/* body of bool_array in #ifdef */
//	    indextype->getsize(),	/* body of bool_array in #else */
//	    indextype->getsize()	/* body of bool_array in #else */
	);

    }













    /* compact print function */
    // Uli: print() function has to be used because of Undefined value
    if (FALSE)			// (elementtype->issimple())
      fprintf(codefile,
              "  void print()\n"
              "  {\n"
              "    cout << name << \"[]: \\t\";\n"
              "    for (int i = 0; i < %d; i++)\n"
              "      cout << array[i] << '\\t';\n"
              "  cout << \"\\n\";\n" "  }\n\n", indextype->getsize());
    else
      fprintf(codefile,
              "  void print(FILE *target, const char *separator)\n"
              "  {\n"
              "    for (int i = 0; i < %d; i++)\n"
              "      array[i].print(target,separator); };\n\n", indextype->getsize());

    fprintf(codefile,
            "  void print_diff(state *prevstate, FILE *target, const char *separator)\n"
            "  {\n"
            "    for (int i = 0; i < %d; i++)\n"
            "      array[i].print_diff(prevstate,target,separator);\n"
            "  };\n", indextype->getsize());
    /*
       "  void print_diff(state *prevstate)\n"
       "  {\n"
       "    bool diff = FALSE;\n"
       "    for (int i = 0; i < %d; i++)\n"
       "      if (array[i] != array[i].get_value_from_state(prevstate))\n"
       "   diff = TRUE;\n"
       "    if (diff) {\n"
       "      cout << name << \":\\t\";\n"
       "      for (int i = 0; i < %d; i++)\n"
       "   cout << array[i] << '\\t';\n"
       "      cout << \"\\n\";\n"
       "    }\n"
       "  }\n",
       indextype->getright() - indextype->getleft() + 1,
       indextype->getright() - indextype->getleft() + 1,
       indextype->getright() - indextype->getleft() + 1);
     */

    /* end class definition */
    fprintf(codefile, "};\n\n");

    /* write the set_self function. */
    fprintf(codefile,
            "  void %s::set_self_ar( const char *n1, const char *n2, int os ) {\n"
            "    if (n1 == NULL) {set_self(NULL, 0); return;}\n"
            "    int l1 = strlen(n1), l2 = strlen(n2);\n"
            "    strcpy( longname, n1 );\n"
            "    longname[l1] = '[';\n"
            "    strcpy( longname+l1+1, n2 );\n"
            "    longname[l1+l2+1] = ']';\n"
            "    longname[l1+l2+2] = 0;\n"
            "    set_self( longname, os );\n"
            "  };\n"
            "  void %s::set_self_2( const char *n1, const char *n2, int os ) {\n"
            "    if (n1 == NULL) {set_self(NULL, 0); return;}\n"
            "    strcpy( longname, n1 );\n"
            "    strcat( longname, n2 );\n"
            "    set_self( longname, os );\n" "  };\n", mu_name, mu_name);
    switch (indextype->gettypeclass()) {
      case typedecl::Enum:
        fprintf(codefile,
                "  void %s::set_self( const char *n, int os)\n"
                "  {\n" "    int i=0;\n"
                "    name = (char *)n;\n\n", mu_name);
        make_elt_with_enum(((enumtypedecl *) indextype)->getidvalues(),
                           indextype->getsize(),
                           elementtype->getbitsalloc());
        fprintf(codefile, "  }\n");
        break;
      case typedecl::Range:
        fprintf(codefile,
                "void %s::set_self( const char *n, int os)\n"
                "{\n"
                "  char* s;\n"
                "  name = (char *)n;\n"
                "  for(int i = 0; i < %d; i++) {\n"
                /* ANOTHER EVIL TSPRINTF! */
                // Uli: I don't know who made the above comment, but it's
                //      really evil since the allocated memory is in many
                //      cases hard to free (LABEL1)
                "    array[i].set_self_ar(n, s=tsprintf(\"%%d\",i + %d), i * %d + os);\n"
                "    delete[] s;\n"
                "  }\n"
                "};\n",

                mu_name,  /* first line of format */
                indextype->getsize(),	/* for loop bound */
                indextype->getleft(),	/* 3rd arg to tsprintf */
                elementtype->getbitsalloc());	/* second arg to set_self */
        break;
      case typedecl::Scalarset:
        fprintf(codefile,
                "void %s::set_self( const char *n, int os)\n"
                "  {\n" "    int i=0;\n"
                "    name = (char *)n;\n\n", mu_name);
        make_elt_with_scalarset(((scalarsettypedecl *)
                                 indextype)->getidvalues(),
                                indextype->getsize(),
                                elementtype->getbitsalloc());
        fprintf(codefile, "}\n");
        break;
      case typedecl::Union:
        fprintf(codefile,
                "void %s::set_self( const char *n, int os)\n"
                "  {\n" "    int i=0;\n"
                "    name = (char *)n;\n\n", mu_name);
        make_elt_with_union(((uniontypedecl *) indextype)->getunionmembers(),
                            elementtype->getbitsalloc());
        fprintf(codefile, "}\n");
        break;
      case typedecl::Array:
      case typedecl::Record:
      case typedecl::Error_type:
      default:
        // the error should already be flaged.
        break;
    }

    fprintf(codefile,
            /* DELETE PROBLEM */// Uli: no delete since there in no new
            "%s::~%s()\n" "{\n"
//	    "  if (name) delete [] name;\n"
            // "  for(int i = 0; i < %d; i++)\n"
            // "    delete[ OLD_GPP(strlen(array[i].name) +1) ] array[i].name; // Should be delete[] \n"
            "}\n", mu_name, mu_name /*, indextype->getsize()*/); 			// WP WP WP WP WP COMMENTED OUT ARGUMENT NO LONGER USED

    fprintf(codefile, "/*** end array declaration ***/\n");
    fprintf(codefile, "%s %s_undefined_var;\n\n", mu_name, mu_name);
    declared = TRUE;
  }
  return "ERROR!";
}

/********************
  code for multisettypedecl
 ********************/
const char *multisetidtypedecl::generate_decl()
{
  return "ERROR!";
}

const char *multisettypedecl::generate_decl()
{
  if (!declared) {

    /* make up a name for a new class, and a mu_name to be output. */
    if (name == NULL) {
      name = tsprintf("_multiset_%d", tNum);
      mu_name = tsprintf("mu_%s", name);
    }

    fprintf(codefile, "/*** begin multiset declaration ***/\n");

    // declare multiest id
    fprintf(codefile, "class %s_id: public %s\n" "{\n" " public:\n" "  inline int operator=(int val) { return value(val); };\n" "  inline int operator=(const %s_id& val) {" " return value(val.value()); };\n"	// Uli: return added
            "  inline operator int() const { return value(); };\n" "  %s_id () : %s(0,%d,0) {};\n" "  %s_id (int val) : %s(0,%d,0, \"Parameter or function result.\",0) {operator=(val); };\n" "  char * Name() { return tsprintf(\"%%d\", value()); };\n" "};\n", mu_name, mu_type,	// class
            mu_name,		// operator=
            mu_name, mu_type, maximum_size - 1,	// constructor
            mu_name, mu_type, maximum_size - 1	// constructor
           );

    /* declare class with set_self, constructor and destructor */
    fprintf(codefile, "class %s\n" "{\n" " public:\n" "  %s array[ %d ];\n" "  int max_size;\n" "  int current_size;\n" " public:\n" "  mu_0_boolean valid[ %d ];\n" "  char *name;\n" "  char longname[BUFFER_SIZE/4];\n" "  void set_self( const char *n, int os);\n" "  void set_self_2( const char *n, const char *n2, int os);\n" "  void set_self_ar( const char *n, const char *n2, int os);\n" "  %s (const char *n, int os): current_size(0), max_size(0) { set_self(n, os); };\n" "  %s ( void ): current_size(0), max_size(0) {};\n" "  virtual ~%s ();\n", mu_name,	/* class name */
            elementtype->generate_code(),	/* array elt type */
            maximum_size,	/* array size */
            maximum_size,	/* array size */
            mu_name,		/* name for first constructor */
            /* maximum_size,  /* max current size */
            /* CeilLog2(maximum_size+2),  /* max current size */
            mu_name,		/* name for second constructor */
            /* maximum_size,  /* max current size */
            /* CeilLog2(maximum_size+2),  /* max current size */
            mu_name);		/* destructor name */

    /* no range-checked operator [] */
    fprintf(codefile,
            "  %s& operator[] (int index) /* const */\n"
            "  {\n"
            "    if ((index >= 0) && (index <= %d) && valid[index].value())\n"
            "      return array[ index ];\n"
            "    else {\n"
            "      Error.Error(\"Internal Error::%%d not in index range of %%s.\", index, name);\n"
            "      return array[0];\n"
            "    }\n"
            "  };\n", elementtype->generate_code(), maximum_size - 1);

    /* and an operator =. */
    generate_assign();
    /*    if (elementtype->issimple())
          fprintf(codefile,
    	  "  %s& operator= (const %s& from)\n"
    	  "  {\n"
    	  "    int i;\n"
    	  "    for (i = 0; i < %d; i++)\n"
    	  "      if (from.array[i].isundefined() )\n"
    	  "	array[i].undefine();\n"
    	  "      else\n"
    	  "	array[i] = from.array[i];\n"
    	  "    for (i = 0; i < %d; i++)\n"
    	  "      valid[i].value(from.valid[i].value());\n"
    	  "    current_size = from.get_current_size();\n"
    	  "    return *this;\n"
    	  "  }\n\n",
    	  mu_name,
    	  mu_name,
    	  maximum_size,
    	  maximum_size);
        else
          fprintf(codefile,
    	  "  %s& operator= (const %s& from)\n"
    	  "  {\n"
    	  "    for (int i = 0; i < %d; i++)\n"
    	  "      {\n"
    	  "	array[i] = from.array[i];\n"
    	  "	valid[i] = from.valid[i];\n"
    	  "      }\n"
    	  "    current_size = from.get_current_size();\n"
    	  "    return *this;\n"
    	  "  }\n\n",
    	  mu_name,
    	  mu_name,
    	  maximum_size);
    */

    /* comparsion function */
    fprintf(codefile,
            "friend int CompareWeight(%s& a, %s& b)\n"
            "  {\n"
            "    return 0;\n" "  }\n", mu_name, mu_name /*, maximum_size*/); 	// WP WP WP WP WP COMMENTED OUT ARGUMENT NO LONGER USED

    /* comparsion function */
    fprintf(codefile,
            "friend int Compare(%s& a, %s& b)\n"
            "  {\n"
            "    int w;\n"
            "    for (int i=0; i<%d; i++) {\n"
            "      w = Compare(a.array[i], b.array[i]);\n"
            "      if (w!=0) return w;\n"
            "    }\n"
            "    return 0;\n" "  }\n", mu_name, mu_name, maximum_size);

    /* declare permute() */
    theprog->symmetry.generate_symmetry_function_decl();

    /* standard functions:
     * clear(), undefine(), to_state(), from_state(). */
    fprintf(codefile,
            "  void clear() { for (int i = 0; i < %d; i++) { array[i].undefine(); valid[i].value(FALSE); } current_size = 0; };\n\n"
            "  void undefine() { for (int i = 0; i < %d; i++) { array[i].undefine(); valid[i].value(FALSE); } current_size = 0; };\n\n"
            "  void reset() { for (int i = 0; i < %d; i++) { array[i].undefine(); valid[i].value(FALSE); } current_size = 0; };\n\n"
            "  void to_state(state *thestate)\n"
            "  {\n"
            "    for (int i = 0; i < %d; i++)\n"
            "     {\n"
            "       array[i].to_state(thestate);\n"
            "       valid[i].to_state(thestate);\n" "     }\n" "  };\n\n"
//       "  void from_state(state *thestate)\n"
//       "  {\n"
//       "    int i;\n"
//       "    for (i = 0; i < %d; i++)\n"
//       "     {\n"
//       "      array[i].from_state(thestate);\n"
//       "      valid[i].from_state(thestate);\n"
//       "     }\n"
//       "  };\n\n"
            "  int get_current_size() const" "  {\n" "    int tmp = 0;\n" "    for (int i = 0; i < %d; i++)\n" "      if (valid[i].value()) tmp++;\n" "    return tmp;\n" "  };\n\n " "  void update_size()\n" "  {\n" "    current_size = 0;\n" "    for (int i = 0; i < %d; i++)\n" "      if (valid[i].value()) current_size++;\n" "    if (max_size<current_size) max_size = current_size;\n" "  };\n\n ", maximum_size,	/* body of clear */
            maximum_size,	/* body of undefine */
            maximum_size,	/* body of reset */
            maximum_size,	/* body of to_state */
//       maximum_size, /* body of from_state */
            maximum_size,	/* body of get_current_size */
            maximum_size	/* body of update_size */
           );

    fprintf(codefile, "  inline bool in(const %s_id& id)\n"
            // "  { if (current_size>id) return TRUE; else return FALSE; }\n",
            "  { return valid[(int)id].value(); }\n",	// Uli 10-98
            mu_name);

    /* compact print function */
    // Uli: print() function has to be used because of Undefined value
    if (FALSE)			// (elementtype->issimple())
      fprintf(codefile, "  void print()\n" "  {\n" "    cout << name << \"[]: \\t\";\n" "    for (int i = 0; i < %d; i++)\n" "      if (valid[i].value())\n" "	cout << array[i] << '\\t';\n" "      else\n" "	cout << '-' << '\\t';\n" "  cout << \"\\n\";\n" "  }\n\n", maximum_size	/* for */
             );
    else
      fprintf(codefile, "  void print()\n" "  {\n" "    for (int i = 0; i < %d; i++)\n" "      if (valid[i].value())\n" "	array[i].print();\n" "  };\n\n", maximum_size	/* for */
             );

    fprintf(codefile,
            "  void print_statistic()\n"
            "  {\n"
            "    cout << \"\tThe maximum size for the multiset \\\"\" \n"
            "	 << name << \"\\\" is: \" << max_size << \".\\n\"; \n"
            "  };\n");
    fprintf(codefile,
            "  void print_diff(state *prevstate, FILE *target, const char *separator)\n"
            "  {\n"
            "    bool prevvalid;\n"
            "    static state temp;\n"
            "    StateCopy(&temp, workingstate);\n"
            "    for (int i = 0; i < %d; i++)\n"
            "      {\n"
            "	StateCopy(workingstate, prevstate);\n"
            "	prevvalid = valid[i].value();\n"
            "	StateCopy(workingstate, &temp);\n"
            "	if (prevvalid && !valid[i].value())\n"
            "	  array[i].print(target,separator);\n"
            "	if (!prevvalid && valid[i].value())\n"
            "	  array[i].print(target,separator);\n"
            "	if (prevvalid && valid[i].value())\n"
            "	  array[i].print_diff(prevstate,target,separator);\n"
            "      }\n" "  };\n", maximum_size);

    fprintf(codefile,
            "  int multisetadd(const %s &element)\n"
            "  {\n"
            "    update_size();\n"
            "    if (current_size >= %d) Error.Error(\"Maximum size of MultiSet (%%s) exceeded.\",name);\n"
            "    int i;\n"
            "    for (i = 0; i < %d; i++)\n"
            "      if (!valid[i].value())\n"
            "	{\n"
            "	  array[i] = element;\n"
            "	  valid[i].value(TRUE);\n"
            "	  break;\n"
            "	}\n"
            "    current_size++;\n"
            "    return i;\n"
            "  };\n",
            elementtype->generate_code(), maximum_size, maximum_size);
    fprintf(codefile, "  void multisetremove(const %s_id &id)\n" "  {\n" "    update_size();\n" "    if (!valid[(int)id].value()) Error.Error(\"Internal Error: Illegal Multiset element selected.\");\n"	// Uli 10-98
            "    valid[(int)id].value(FALSE);\n"
            "    array[(int)id].undefine();\n" "    current_size--;\n"
            // "    id.undefine();\n"   // Uli: not necessary since valid[]
            //      is checked
            "  };\n", mu_name);

    fprintf(codefile,
            "  void MultisetSort()\n"
            "  {\n"
            "    static %s temp;\n"
            "\n"
            "    // compact\n"
            "    int i,j;\n"
            "    for (i = 0, j = 0; i < %d; i++)\n"
            "      if (valid[i].value())\n"
            "	{\n"
            "	  if (j!=i)\n"
            "	    array[j++] = array[i];\n"
            "	  else\n"
            "	    j++;\n"
            "	}\n"
            "    if (j != current_size) current_size = j;\n"
            "    for (i = j; i < %d; i++)\n"
            "      array[i].undefine();\n"
            "    for (i = 0; i < j; i++)\n"
            "      valid[i].value(TRUE);\n"
            "    for (i = j; i < %d; i++)\n"
            "      valid[i].value(FALSE);\n"
            "\n"
            "    // bubble sort\n"
            "    for (i = 0; i < current_size; i++)\n"
            "      for (j = i+1; j < current_size; j++)\n"
            "	if (Compare(array[i],array[j])>0)\n"
            "	  {\n"
            "	    temp = array[i];\n"
            "	    array[i] = array[j];\n"
            "	    array[j] = temp;\n"
            "	  }\n"
            "  }\n",
            elementtype->generate_code
            (), maximum_size, maximum_size, maximum_size);


    // declare procedures for all multisetcount
    if (msclist != NULL)
      msclist->generate_decl(this);

    // declare procedures for all multisetremove
    if (msrlist != NULL)
      msrlist->generate_decl(this);


    /* end class definition */
    fprintf(codefile, "};\n\n");

    /* write the set_self functions. */
    fprintf(codefile,
            "  void %s::set_self_ar( const char *n1, const char *n2, int os ) {\n"
            "    if (n1 == NULL) {set_self(NULL, 0); return;}\n"
            "    int l1 = strlen(n1), l2 = strlen(n2);\n"
            "    strcpy( longname, n1 );\n"
            "    longname[l1] = '[';\n"
            "    strcpy( longname+l1+1, n2 );\n"
            "    longname[l1+l2+1] = ']';\n"
            "    longname[l1+l2+2] = 0;\n"
            "    set_self( longname, os );\n"
            "  };\n"
            "  void %s::set_self_2( const char *n1, const char *n2, int os ) {\n"
            "    if (n1 == NULL) {set_self(NULL, 0); return;}\n"
            "    strcpy( longname, n1 );\n"
            "    strcat( longname, n2 );\n"
            "    set_self( longname, os );\n" "  };\n", mu_name, mu_name);
    fprintf(codefile,
            "void %s::set_self( const char *n, int os)\n"
            "{\n"
            "  int i,k;\n"
            "  name = (char *)n;\n"
            "  for(i = 0; i < %d; i++)\n"
            /* ANOTHER EVIL TSPRINTF! */
            // Uli: this might have to be changed in a similar fashion as
            //      at LABEL1
            "    if (n) array[i].set_self(tsprintf(\"%%s{%%d}\", n,i), i * %d + os); else array[i].set_self(NULL, 0);\n" "  k = os + i * %d;\n" "  for(i = 0; i < %d; i++)\n", mu_name,	/* first line of format */
            maximum_size,	/* for loop bound */
            elementtype->getbitsalloc(),	/* second arg to set_self */
            elementtype->getbitsalloc(),	/* second arg to set_self */
            maximum_size);	/* for loop bound */
    if (!args->no_compression)
      fprintf(codefile,
              "    valid[i].set_self(\"\", i * 2 + k);\n" "};\n");
    else
      fprintf(codefile,
              "    valid[i].set_self(\"\", i * 8 + k);\n" "};\n");

    fprintf(codefile,
            /* DELETE PROBLEM */// Uli: no delete since there in no new
            "%s::~%s()\n" "{\n"
//	    "  if (name) delete [] name;\n"
            // "  for(int i = 0; i < %d; i++)\n"
            // "    delete[ OLD_GPP(strlen(array[i].name) +1) ] array[i].name; // Should be delete[] \n"
            // "  delete[ OLD_GPP(strlen(current_size[i].name) +1) ] current_size.name; // Should be delete[] \n"
            "}\n", mu_name, mu_name /*, maximum_size*/);						// WP WP WP WP WP COMMENTED OUT ARGUMENT NO LONGER USED

    // declare procedures for all multisetcount
    if (msclist != NULL)
      msclist->generate_procedure();

    // declare procedures for all multisetremove
    if (msrlist != NULL)
      msrlist->generate_procedure();

    fprintf(codefile, "/*** end multiset declaration ***/\n");
    fprintf(codefile, "%s %s_undefined_var;\n\n", mu_name, mu_name);
    declared = TRUE;
  }
  return "ERROR!";
}

void multisetcountlist::generate_decl(multisettypedecl * mset)
{
  if (next != NULL)
    next->generate_decl(mset);
  mscount->generate_decl(mset);
}

void multisetremovelist::generate_decl(multisettypedecl * mset)
{
  if (next != NULL)
    next->generate_decl(mset);
  msremove->generate_decl(mset);
}

void multisetcountlist::generate_procedure()
{
  if (next != NULL)
    next->generate_procedure();
  mscount->generate_procedure();
}

void multisetremovelist::generate_procedure()
{
  if (next != NULL)
    next->generate_procedure();
  msremove->generate_procedure();
}

/********************
  code for recordtypedecl
 ********************/
const char *recordtypedecl::generate_decl()
{
  ste *f;
  if (!declared) {
    if (name == NULL) {
      name = tsprintf("_record_%d", tNum);
      mu_name = tsprintf("%s", name);
    }
    fprintf(codefile, "class %s\n" "{\n" " public:\n" "  char *name;\n" "  char longname[BUFFER_SIZE/4];\n" "  void set_self_2( const char *n, const char *n2, int os);\n" "  void set_self_ar( const char *n, const char *n2, int os);\n" "  void set_self(const char *n, int os);\n", mu_name);	/* class name */

    /* Generate members for record fields. */
    for (f = fields; f != NULL; f = f->getnext()) {
      /* f->getvalue()->generate_decl(); */
      /* Not right--it tells members the wrong names. */
      fprintf(codefile, "  %s %s;\n",
              f->getvalue()->gettype()->generate_code(),
              f->getvalue()->generate_code());
    };

    /* Two constructors and a destructor */
    fprintf(codefile, "  %s ( const char *n, int os ) { set_self(n,os); };\n" "  %s ( void ) {};\n\n" "  virtual ~%s(); \n", mu_name, mu_name, mu_name);	/* constructor and destructor names */

    /* comparsion function */
    fprintf(codefile,
            "friend int CompareWeight(%s& a, %s& b)\n"
            "  {\n" "    int w;\n", mu_name, mu_name);
    for (f = fields; f != NULL; f = f->getnext())
      fprintf(codefile,
              "    w = CompareWeight(a.%s, b.%s);\n"
              "    if (w!=0) return w;\n",
              f->getvalue()->generate_code(),
              f->getvalue()->generate_code()
             );
    fprintf(codefile, "  return 0;\n" "}\n");

    /* comparsion function */
    fprintf(codefile,
            "friend int Compare(%s& a, %s& b)\n"
            "  {\n" "    int w;\n", mu_name, mu_name);
    for (f = fields; f != NULL; f = f->getnext())
      fprintf(codefile,
              "    w = Compare(a.%s, b.%s);\n"
              "    if (w!=0) return w;\n",
              f->getvalue()->generate_code(),
              f->getvalue()->generate_code()
             );
    fprintf(codefile, "  return 0;\n" "}\n");

    /* declare permute() */
    theprog->symmetry.generate_symmetry_function_decl();

    /* comparsion function */
    fprintf(codefile, "  virtual void MultisetSort()\n" "  {\n");
    for (f = fields; f != NULL; f = f->getnext())
      fprintf(codefile,
              "    %s.MultisetSort();\n", f->getvalue()->generate_code()
             );
    fprintf(codefile, "  }\n");

    /* statistic */
    fprintf(codefile, "  void print_statistic()\n" "  {\n");
    for (f = fields; f != NULL; f = f->getnext())
      fprintf(codefile,
              "    %s.print_statistic();\n", f->getvalue()->generate_code()
             );
    fprintf(codefile, "  }\n");

    /* standard functions:
     * clear(), undefine(), print(), print_diff(), to_state(), from_state().
     * And now, operator = ()! */
    fprintf(codefile, "  void clear() {\n");
    for (f = fields; f != NULL; f = f->getnext())
      fprintf(codefile, "    %s.clear();\n",
              f->getvalue()->generate_code());
    fprintf(codefile, " };\n");

    fprintf(codefile, "  void undefine() {\n");
    for (f = fields; f != NULL; f = f->getnext())
      fprintf(codefile, "    %s.undefine();\n",
              f->getvalue()->generate_code());
    fprintf(codefile, " };\n");

    fprintf(codefile, "  void reset() {\n");
    for (f = fields; f != NULL; f = f->getnext())
      fprintf(codefile, "    %s.reset();\n",
              f->getvalue()->generate_code());
    fprintf(codefile, " };\n");

    // WP WP WP WP WP
    //fprintf(codefile, " };\n");

    fprintf(codefile, "  void print(FILE *target, const char *separator) {\n");
    for (f = fields; f != NULL; f = f->getnext())
      fprintf(codefile, "    %s.print(target,separator);\n",
              f->getvalue()->generate_code());
    fprintf(codefile, "  };\n");

    fprintf(codefile, "  void print_diff(state *prevstate, FILE *target, const char *separator) {\n");
    for (f = fields; f != NULL; f = f->getnext())
      fprintf(codefile, "    %s.print_diff(prevstate,target,separator);\n",
              f->getvalue()->generate_code());
    fprintf(codefile, "  };\n");

    fprintf(codefile, "  void to_state(state *thestate) {\n");
    for (f = fields; f != NULL; f = f->getnext())
      fprintf(codefile, "    %s.to_state(thestate);\n",
              f->getvalue()->generate_code());
    fprintf(codefile, "  };\n");

//       fprintf(codefile,"  void from_state(state *thestate) {\n");
//       for( f = fields; f != NULL; f = f->getnext() )
//   fprintf(codefile,"    %s.from_state(thestate);\n", f->getvalue()->generate_code() );
//       fprintf(codefile,"  };\n");

    /* isundefined : this should not be called in a well-formed program */
    fprintf(codefile,
            "virtual bool isundefined() { Error.Error(\"Checking undefinedness of a non-base type\"); return TRUE;}\n");

    /* ismember : this should not be called in a well-formed program */
    fprintf(codefile,
            "virtual bool ismember() { Error.Error(\"Checking membership for a non-base type\"); return TRUE;}\n");

    generate_assign();
    /*       fprintf(codefile,
    	  "  %s& operator= (const %s& from) {\n",
    	  mu_name,
    	  mu_name);
           for( f = fields; f != NULL; f = f->getnext() ) {
        if (f->getvalue()->gettype()->issimple())
          fprintf(codefile,
    	"   if ( from.%s.isundefined() )\n"
    	"     %s.undefine();\n"
    	"   else\n"
    	"     %s = from.%s;\n",
    	f->getvalue()->generate_code(),
    	f->getvalue()->generate_code(),
    	f->getvalue()->generate_code(),
    	f->getvalue()->generate_code());
        else
          fprintf(codefile,
    	"   %s = from.%s;\n",
    	f->getvalue()->generate_code(),
    	f->getvalue()->generate_code());
           }

           fprintf(codefile,"    return *this;\n");
           fprintf(codefile,"  };\n");
    */

    /* end class definition */
    fprintf(codefile, "};\n\n");

    fprintf(codefile,
            "  void %s::set_self_ar( const char *n1, const char *n2, int os ) {\n"
            "    if (n1 == NULL) {set_self(NULL, 0); return;}\n"
            "    int l1 = strlen(n1), l2 = strlen(n2);\n"
            "    strcpy( longname, n1 );\n"
            "    longname[l1] = '[';\n"
            "    strcpy( longname+l1+1, n2 );\n"
            "    longname[l1+l2+1] = ']';\n"
            "    longname[l1+l2+2] = 0;\n"
            "    set_self( longname, os );\n"
            "  };\n"
            "  void %s::set_self_2( const char *n1, const char *n2, int os ) {\n"
            "    if (n1 == NULL) {set_self(NULL, 0); return;}\n"
            "    strcpy( longname, n1 );\n"
            "    strcat( longname, n2 );\n"
            "    set_self( longname, os );\n" "  };\n", mu_name, mu_name);
    fprintf(codefile,
            "void %s::set_self(const char *n, int os)\n"
            "{\n"
            "  name = (char *)n;\n\n", mu_name);
    for (f = fields; f != NULL; f = f->getnext()) {
      fprintf(codefile,
              "  if (name) %s.set_self_2(name, \"%s\", os + %d ); else %s.set_self_2(NULL, NULL, 0);\n",
              f->getvalue()->generate_code(),
              tsprintf(".%s", f->getvalue()->name),
              ((vardecl *) f->getvalue())->getoffset(),
              f->getvalue()->generate_code());
    }
    fprintf(codefile, "}\n\n");

    /* destructor def */
    fprintf(codefile,		// Uli: no delete since there in no new
            "%s::~%s()\n{\n"
//	    "  if (name) delete [] name;\n"
            , mu_name, mu_name);

    // for(f = fields; f != NULL; f = f->getnext() ) {
    //   fprintf(codefile,"  delete[ OLD_GPP(strlen(%s.name) + 1)] %s.name; // Should be delete[] \n",
    //     f->getvalue()->generate_code(),
    //     f->getvalue()->generate_code());
    // }
    fprintf(codefile, "}\n\n");

    fprintf(codefile, "/*** end record declaration ***/\n");
    fprintf(codefile, "%s %s_undefined_var;\n\n", mu_name, mu_name);

    declared = TRUE;
  }
  return "ERROR!";
}

/********************
  code for scalarsettypedecl
  ********************/
void make_scalarset_idvalues(ste * idvalues, int size, bool named)
{
  if (size > 1)
    make_scalarset_idvalues(idvalues->next, size - 1, named);
  if (named) {
    fprintf(codefile, "\"%s\",", idvalues->getname()->getname());
  } else {
    char *c = idvalues->getname()->getname();
    while (*c != '_')
      c++;
    c++;
    fprintf(codefile, ",\"%s\"", c);
  }
}

const char *scalarsettypedecl::generate_decl()
{
  if (!declared) {
    if (name == NULL) {
      name = tsprintf("_scalarset_%d", tNum);
      mu_name = tsprintf("mu_%s", name);
    }

    /* declare class, <<, initializers, and destructor */
    fprintf(codefile, "class %s: public %s\n" "{\n" " public:\n" "  inline int operator=(int val) { return value(val); };\n" "  inline int operator=(const %s& val){" " return value(val.value());};\n" "  inline operator int() const { return value(); };\n" "  static const char *values[];\n" "  friend ostream& operator<< (ostream& s, %s& val)\n" "    {\n" "      if (val.defined())\n" "	return ( s << %s::values[ int(val) - %d ] );\n" "      else\n" "	return ( s << \"Undefined\" );\n" "    };\n\n" "  %s (const char *name, int os): %s(%d, %d, %d, name, os) {};\n" "  %s (void): %s(%d, %d, %d) {};\n" "  %s (int val): %s(%d, %d, %d, \"Parameter or function result.\", 0)\n" "    { operator=(val); };\n", mu_name, mu_type,	/* class name */
            mu_name, mu_name,	/* type name for second arg to << */
            mu_name,		/* class preceding values in body of << */
            left,		/* lower bound for scalarset value */
            mu_name, mu_type, left, right, numbits,
            /* args for first constructor */
            mu_name, mu_type, left, right, numbits,
            /* args for second constructor */
            mu_name, mu_type, left, right, numbits);
    /* args for third constructor */

    /* declare operator= and print */
    fprintf(codefile,
            "  const char * Name() { return values[ value() -%d]; };\n"
//       "  inline int operator=(int val) { return %s::operator=(val); };\n"
//       "  inline int operator=(const %s& val) { return %s::operator=((int) val); };\n"
            "  virtual void print()\n" "    {\n" "      if (defined()) cout << name << ':' << values[ value() - %d] << '\\n';\n" "      else cout << name << \":Undefined\\n\";\n" "    };\n", left, left);	/* lower bound for values array reference */

    fprintf(codefile, "  void print_statistic() {};\n");

    /* comparsion function */
    fprintf(codefile,
            "friend int CompareWeight(%s& a, %s& b)\n"
            "{\n"
            "  if (!a.defined() && b.defined())\n"
            "    return -1;\n"
            "  else if (a.defined() && !b.defined())\n"
            "    return 1;\n"
            "  else\n" "    return 0;\n" "}\n", mu_name, mu_name);

    /* declare permute() */
    theprog->symmetry.generate_symmetry_function_decl();

    /* end class definition */
    fprintf(codefile, "};\n");

    /* create the values array. */
    fprintf(codefile, "const char *%s::values[] =\n" "  { ", mu_name);
    make_scalarset_idvalues(idvalues, getsize(), named);
    fprintf(codefile, "NULL };\n\n");
    fprintf(codefile, "/*** end scalarset declaration ***/\n");
    fprintf(codefile, "%s %s_undefined_var;\n\n", mu_name, mu_name);

    declared = TRUE;
  }
  return "ERROR!";
}

/********************
  code for uniontypedecl
  ********************/
void make_union_idvalues(stelist * unionmembers)
{
  if (unionmembers->next != NULL)
    make_union_idvalues(unionmembers->next);

  if (((typedecl *) unionmembers->s->getvalue())->gettypeclass() ==
      typedecl::Scalarset)
    make_scalarset_idvalues(((scalarsettypedecl *)
                             unionmembers->s->getvalue())->getidvalues(),
                            unionmembers->s->getvalue()->getsize(), TRUE);
  else if (((typedecl *) unionmembers->s->getvalue())->gettypeclass() ==
           typedecl::Enum)
    make_enum_idvalues(((enumtypedecl *)
                        unionmembers->s->getvalue())->getidvalues(),
                       (typedecl *) unionmembers->s->getvalue());
  else
    Error.Error("funny element in union");
}

// int make_assign_union_values(stelist * unionmembers)
// {
//   int base = 0;
//   if (unionmembers->next != NULL)
//       base = make_assign_union_values(unionmembers->next);
//
//   typedecl *t= (typedecl *) unionmembers->s->getvalue();
//   fprintf(codefile,
//      "    if ( ( val >= %d ) && ( val <= %d ) ) {\n"
//   //       "      defined = TRUE;\n"
//      "      initialized = TRUE;\n"
//      "      return value(val - %d);\n"
//      "    }\n",
//      t->getleft(),
//      t->getright(),
//      t->getleft() - base);
//   return base + t->getsize();
// }

int make_union_indexval(stelist * unionmembers)
{
  int base = 0;
  if (unionmembers->next != NULL)
    base = make_union_indexval(unionmembers->next);

  typedecl *t = (typedecl *) unionmembers->s->getvalue();
  fprintf(codefile,
          "    if ((value() >= %d) && (value() <= %d))"
          " return (value() - %d);\n",
          t->getleft(), t->getright(), t->getleft() - base);
  return base + t->getsize();
}

int make_bit_compacted_value_assign(stelist * unionmembers)
{
  int base = 0;
  if (unionmembers->next != NULL)
    base = make_bit_compacted_value_assign(unionmembers->next);

  typedecl *t = (typedecl *) unionmembers->s->getvalue();
  fprintf(codefile,
          "    if ((val >= %d) && (val <= %d))"
          " return (mu__byte::value(val-%d)+%d);\n",
          t->getleft(),
          t->getright(), t->getleft() - base, t->getleft() - base);
  return base + t->getsize();
}

int make_union_unionassign(stelist * unionmembers)
{
  int base = 0;
  if (unionmembers->next != NULL)
    base = make_union_unionassign(unionmembers->next);

  typedecl *t = (typedecl *) unionmembers->s->getvalue();
  fprintf(codefile,
          "    if (val >= %d && val <= %d) return value(val+%d);\n",
          // Uli: return added
          base, base + t->getsize() - 1, t->getleft() - base);
  return base + t->getsize();
}

int make_bit_compacted_value(stelist * unionmembers)
{
  int base = 0;
  if (unionmembers->next != NULL)
    base = make_bit_compacted_value(unionmembers->next);

  typedecl *t = (typedecl *) unionmembers->s->getvalue();
  fprintf(codefile,
          "    if (val <= %d) return val+%d;\n",
          base + t->getsize() - 1, t->getleft() - base);
  return base + t->getsize();
}

// int make_reference_union_values(stelist * unionmembers)
// {
//   int base = 0;
//   if (unionmembers->next != NULL)
//       base = make_reference_union_values(unionmembers->next);
//   typedecl *t= (typedecl *) unionmembers->s->getvalue();
//   fprintf(codefile,
//      "    if ( ( val >= %d ) && ( val <= %d ) )\n"
//      "      return val + %d;\n",
//      base,
//      base + t->getsize() - 1,
//      t->getleft() - base);
//   return base + t->getsize();
// }

const char *uniontypedecl::generate_decl()
{
  if (!declared) {
    if (name == NULL) {
      name = tsprintf("_union_%d", tNum);
      mu_name = tsprintf("mu_%s", name);
    }

    /* declare class, <<, initializers, and destructor */
    fprintf(codefile,
            "class %s: public %s\n"
            "{\n"
            " public:\n"
            "  inline int operator=(int val) { return value(val); };\n"
            // Uli: return added
            "  inline int operator=(const %s& val) {" " return value(val.value()); };\n" "  inline operator int() const { return value(); };\n" "  static const char *values[];\n" "  friend ostream& operator<< (ostream& s, %s& val)\n" "    {\n" "      if (val.defined())\n" "	return ( s << %s::values[ val.indexvalue() ] );\n" "      else\n" "	return ( s << \"Undefined\" );\n" "    };\n\n" "  // note thate lb and ub are not used if we have byte compacted state.\n" "  %s (const char *name, int os): %s(%d, %d, %d, name, os) {};\n" "  %s (void): %s(%d, %d, %d) {};\n" "  %s (int val): %s(%d, %d, %d, \"Parameter or function result.\", 0)\n" "    { operator=(val); };\n", mu_name, mu_type,	/* class name */
            mu_name,		/* operator= */
            mu_name,		/* type name for second arg to << */
            mu_name,		/* class preceding values in body of << */
            mu_name, mu_type, 0, size - 1, numbits,
            /* args for first constructor */
            mu_name, mu_type, 0, size - 1, numbits,
            /* args for second constructor */
            mu_name, mu_type, 0, size - 1, numbits);
    /* args for third constructor */

    //
    if (args->no_compression) {
      fprintf(codefile, "  int indexvalue()\n" "  {\n");
      make_union_indexval(unionmembers);
      fprintf(codefile, "  };\n");

      fprintf(codefile, "  inline int unionassign(int val)\n" "  {\n");
      make_union_unionassign(unionmembers);
      fprintf(codefile, "  };\n");
    } else {
      fprintf(codefile,
              "  int value() const\n"
              "  {\n"
              "    int val = mu__byte::value();\n"
              "    // val == -1 if value undefined\n"
              "    // we can return it since no enum/scalarsetid will have value -1\n"
              "    if (val == -1) return -1;\n");
      make_bit_compacted_value(unionmembers);
      fprintf(codefile,
              "  }\n"
              "  inline int value(int val)\n"
              "  {\n" "    if (val == -1) { undefine(); return -1; }\n");
      make_bit_compacted_value_assign(unionmembers);
      fprintf(codefile,
              "  }\n"
              "  inline int indexvalue() const\n"
              "  {\n" "    return mu__byte::value();\n" "  };\n");
      fprintf(codefile,
              "  inline int unionassign(int val)\n"
              "  {\n" "    return mu__byte::value(val);\n" "  };\n");
    }


    /* operator=() */
//       fprintf(codefile,
//      "  inline int operator=(int val)\n"
//      "  {\n");
//       make_assign_union_values(unionmembers);
//       fprintf(codefile,
//      "    Error.Error(\"Funny values being assigned to %%s \\n\", name);\n"
//      "    return 0;\n"
//      " };\n");


//      fprintf(codefile,
//      "  inline void unionassign(int val)\n"
//      "  {\n"
//          "    defined = TRUE;\n"
//      "    initialized = TRUE;\n"
//      "    value(val);\n"
//      "  }\n"
//      );

//       fprintf(codefile,
//          "  int operator=(const %s& val)\n"
//          "  {\n",
//      mu_name
//      );
//       make_assign_union_values(unionmembers);
//       fprintf(codefile,
//          "    Error.Error(\"Funny values being assigned to %%s \\n\", name);\n"
//          "    return 0;\n"
//          " };\n",
//      mu_name);

    /* operator int() */
//       fprintf(codefile,
//          "operator int() const\n  {\n"
//          "  int val = %s::operator int();\n",
//      mu_type
//      );
//       make_reference_union_values(unionmembers);
//       fprintf(codefile,
//          "    Error.Error(\"Funny values being assigned to %%s \\n\", name);\n"
//          "    return 0;\n"
//          "  };\n");

    fprintf(codefile,
            "  const char * Name() { return values[ indexvalue() ]; };\n");

    fprintf(codefile,
            "friend int CompareWeight(%s& a, %s& b)\n"
            "{\n"
            "  if (!a.defined() && b.defined())\n"
            "    return -1;\n"
            "  else if (a.defined() && !b.defined())\n"
            "    return 1;\n"
            "  else\n" "    return 0;\n" "}\n", mu_name, mu_name);

    /* declare permute() */
    theprog->symmetry.generate_symmetry_function_decl();

    /* print() */
    fprintf(codefile,
            "  virtual void print()\n"
            "    {\n"
            "      if (defined()) cout << name << ':' << "
            "values[ indexvalue() ] << '\\n';\n"
            "      else cout << name << \":Undefined\\n\";\n" "    };\n");
    fprintf(codefile, "  void print_statistic() {};\n");


    /* end class definition */
    fprintf(codefile, "};\n");

    /* create the values array. */
    fprintf(codefile, "const char *%s::values[] = {", mu_name);
    make_union_idvalues(unionmembers);
    fprintf(codefile, "NULL };\n\n");

    fprintf(codefile, "/*** end union declaration ***/\n");
    fprintf(codefile, "%s %s_undefined_var;\n\n", mu_name, mu_name);

    declared = TRUE;
  }
  return "ERROR!";
}

/********************
  code for constdecl
 ********************/
const char *constdecl::generate_decl()
{
  if (!declared) {
    if (type_equal(type, realtype))	// AP: real constant's declaration
      fprintf(codefile, "const double %s = %+le;\n", mu_name, rvalue);
    else
      fprintf(codefile, "const int %s = %d;\n", mu_name, value);
    declared = TRUE;
  }
  return "ERROR!";
}


/********************
  code for vardecl
 ********************/
const char *vardecl::generate_decl()
{
  if (!declared) {
    fprintf(codefile,
            "/*** Variable declaration ***/\n"
            "%s %s(\"%s\",%d);\n\n",
            type->generate_code(), mu_name, name, offset);
    declared = TRUE;
  }
  return "ERROR!";
}

/********************
  code for aliasdecl
 ********************/

const char *aliasdecl::generate_decl()
{
  if (!declared) {
    if (!ref->islvalue() && ref->gettype()->issimple()) {
      if (type_equal(ref->gettype(), realtype))
        fprintf(codefile,
                "  const double %s = %s;\n", mu_name,
                ref->generate_code());
      else
        fprintf(codefile,
                /* BUG: BOGUS CONST INT */
                /* is this fixed adding  ref->gettype()->issimple() */
                "  const int %s = %s;\n", mu_name, ref->generate_code());
    } else {
      fprintf(codefile, "  %s& %s = %s;\n",
              ref->gettype()->generate_code(),
              mu_name, ref->generate_code());
    }
  }
  return "ERROR!";
}

/********************
  code for choosedecl
 ********************/
const char *choosedecl::generate_decl()
/* Should never actually get called. */
{
  Error.Error
  ("Internal: choosedecl::generate_decl() should not have been called.");
  return "ERROR!";
}

/********************
  code for quantdecl
 ********************/
const char *quantdecl::generate_decl()
/* Should never actually get called. */
{
  Error.Error
  ("Internal: quantdecl::generate_decl() should not have been called.");
  return "ERROR!";
}

void quantdecl::make_for()
/* generates a _for_
 * statement that could serve as a wrapper for a FOR or FORALL.
 * However, this won\'t cut it for rulesets, which will have to
 * do it themselves. */
/* BUG:  There seem to be a lot of special cases here.  Why is the
   loop variable "mu_name"? */
{
  // fprintf(codefile,"int %s;\n",mu_name);
  /* I think it\'s better to declare the index in the loop. */
  if (left == NULL) {
    if (type->gettypeclass() == typedecl::Union) {
      stelist *t;
      typedecl *d;
      int minleft, maxright;

      // find max right and min left
      t = ((uniontypedecl *) type)->getunionmembers();
      d = (typedecl *) t->s->getvalue();
      minleft = d->getleft();
      maxright = d->getright();
      t = t->next;
      for (; t != NULL; t = t->next) {
        d = (typedecl *) t->s->getvalue();
        if (d->getleft() < minleft)
          minleft = d->getleft();
        if (d->getright() > maxright)
          maxright = d->getright();
      }

      fprintf(codefile,
              "for(int %s = %d; %s <= %d; %s++)\n"
              "  if (", mu_name, minleft, mu_name, maxright, mu_name);

      // check each element
      t = ((uniontypedecl *) type)->getunionmembers();
      d = (typedecl *) t->s->getvalue();
      fprintf(codefile,
              "( ( %s >= %d ) && ( %s <= %d ) )",
              mu_name, d->getleft(), mu_name, d->getright()
             );
      t = t->next;
      for (; t != NULL; t = t->next) {
        d = (typedecl *) t->s->getvalue();
        fprintf(codefile,
                "|| ( ( %s >= %d ) && ( %s <= %d ) )",
                mu_name, d->getleft(), mu_name, d->getright()
               );
      }
      fprintf(codefile, ") {\n");
    } else
      fprintf(codefile, "for(int %s = %d; %s <= %d; %s++) {\n", mu_name, type->getleft(),	/* change here */
              mu_name, type->getright(), mu_name);
  } else {
    /* Dill: Why is this name needed? */
    /* Ip: upbound is used as a new local variable which does not clash with other */
    char *upbound = tsprintf("mu__ub%d", new_int());
    if (type_equal(right->gettype(), realtype))
      fprintf(codefile, "double %s = %s;\n", upbound,
              right->generate_code());
    else
      fprintf(codefile, "int %s = %s;\n", upbound, right->generate_code());
    if (by > 0) {
      if (type_equal(left->gettype(), realtype))
        fprintf(codefile,
                "for (double %s = %s; %s <= %s; %s += %le) {\n",
                mu_name,
                left->generate_code(), mu_name, upbound, mu_name, byR);
      else
        fprintf(codefile,
                "for (int %s = %s; %s <= %s; %s += %d) {\n",
                mu_name,
                left->generate_code(), mu_name, upbound, mu_name, by);
    } else {
      if (type_equal(left->gettype(), realtype))
        fprintf(codefile,
                "for (double %s = %s; %s >= %s; %s += %+le) {\n",
                mu_name,
                left->generate_code(), mu_name, upbound, mu_name, byR);
      else
        fprintf(codefile,
                "for (int %s = %s; %s >= %s; %s += %d) {\n",
                mu_name,
                left->generate_code(), mu_name, upbound, mu_name, by);
    }
  }
  declared = TRUE;
}

/********************
  code for parameter
 ********************/
const char *param::generate_decl()
{
  Error.Error("Internal: param::generate_decl()");
  return "ERROR!";
}

const char *varparam::generate_decl()
{
  if (!declared) {
    fprintf(codefile, "%s& %s", type->generate_code(), mu_name);
    declared = TRUE;
  }
  return "ERROR!";
}

const char *valparam::generate_decl()
{
  if (!declared) {
    fprintf(codefile, "%s %s", type->generate_code(), mu_name);
    declared = TRUE;
  }
  return "ERROR!";
}

/* We have to do some skanky workarounds here;
 * CC won't let us declare our smart operator [] as
 * const, but if we don't, we can't use operator [] for
 * for a constant array.  So we work around this by
 * dropping the constant for a const array parameter.
 *
 * Furthermore, since a const record can include a const
 * array, we need to drop the const for records, too.
 * This is okay, since we\'ve checked in the Murphi
 * semantic-checking, and we know that this array isn\'t
 * modified, and we know that no conversion will be involved,
 * since Murphi doesn\'t allow conversion among aggregate
 * types.
 *
 * We do need the const for simple types, since otherwise,
 * we won\'t get any automatic conversion from C++.
 */
const char *constparam::generate_decl()
{
  if (!declared) {
    if (type->issimple())
      fprintf(codefile, "const %s& %s", type->generate_code(), mu_name);
    else
      fprintf(codefile, "%s& %s", type->generate_code(), mu_name);

    declared = TRUE;
  }
  return "ERROR!";
}

std::map<std::string,expr*> global_test_minus;
std::map<std::string,expr*> global_test_plus;

/********************
  code for procdecl
 ********************/
std::map< std::string, std::vector< std::pair< std::pair<std::string, std::string>, std::pair<int, int> > > > processes;
std::map< std::string, std::vector< std::pair< std::pair<std::string, std::string>, std::pair<int, int> > > > processes_plus;
std::map< std::string, std::vector< std::pair< std::pair<std::string, std::string>, std::pair<int, int> > > > processes_minus;

const char *procdecl::generate_decl()
{
  if (!declared) {
    if (!extern_def) {
      /* the declaration. */
      fprintf(codefile, "void %s(", mu_name);


      // WP WP WP
      std::vector< std::pair< std::pair<std::string, std::string>, std::pair<int, int> > > prms;

      // WP WP WP
      bool procs = false;
      if(ff_enabled){
    	  if (std::string(mu_name).find("mu_process") != std::string::npos) procs = true;
      }

//      // WP WP WP
//      if (procs)
//    	  std::cout << "\nPROCESS : " << mu_name << " : " ;

      /* formal parameter list */
      for (ste * p = params; p != NULL; p = p->getnext()) {


    	// WP WP WP
    	if (procs){
//    	  std::cout << " ===   " << p->getvalue()->gettype()->mu_name << " " << p->getvalue()->mu_name;
    	  prms.push_back(std::make_pair(std::make_pair(p->getvalue()->gettype()->mu_name, p->getvalue()->mu_name), std::make_pair(p->value->gettype()->getleft(), p->value->gettype()->getright())));
    	}

        p->getvalue()->generate_decl(); // adds parameters to the function declaration
        if (p->getnext() == NULL)
          break;
        fprintf(codefile, ", ");
      }

      // WP WP WP
      if (procs)
    	  processes.insert(std::make_pair(mu_name, prms));

      fprintf(codefile, ")\n" "{\n");


      /* the locals. */
      decls->generate_decls();

      /* the statements. */


// TODO

//      procdecl *ppdecl = (procdecl *) pbody->procedure->getvalue();
//
//      std::vector< std::pair <std::string, std::vector<string> > > plus_procs;
//
      if (procs){

    	  std::string pplus = "";
    	  std::string pminus = "";


    	  for (stmt * sttt = body; sttt != NULL; sttt = sttt->next) {

//    	  		std::cout << "\nPLUS == " << sttt->get_proc_code_plus() << std::endl;


//    		  global_test_plus.push_back(((ifstmt*)sttt)->test);
    		  std::string pp = sttt->get_proc_code_plus();

    		  sttt->get_proc_code_plus();
//    		  pplus.append(decls->generate_decls());
    		  pplus.append(pp);
    		  global_test_plus.insert(std::make_pair(pplus,(((ifstmt*)sttt)->test)));

//    	  		std::cout << "\nMINUS == " << sttt->get_proc_code_minus() << std::endl;

//    		  global_test_minus.push_back(((ifstmt*)sttt)->test);
    		  std::string pm = sttt->get_proc_code_minus();

    		  sttt->get_proc_code_minus();
//    		  pminus.append(decls->generate_decls());
    		  pminus.append(pm);
    		  global_test_minus.insert(std::make_pair(pminus,(((ifstmt*)sttt)->test)));

    	  }

    	  processes_plus.insert(std::make_pair(pplus, prms));
    	  processes_minus.insert(std::make_pair(pminus, prms));


      }





      for (stmt * s = body; s != NULL; s = s->next) {

          s->generate_code();

      }

      fprintf(codefile, "};\n");

      fprintf(codefile, "/*** end procedure declaration ***/\n\n");

      declared = TRUE;
    } else if (include_file_ext != NULL)
      fprintf(codefile, "\n#include \"%s\"\n\n", include_file_ext);
  }
  return "ERROR!";
}

/********************
  code for funcdecl
 ********************/

/* BUG: check for return everywhere? */
/* Norris: fixed by adding an error at the very end... so that user will
   know and go back and fix the function */

const char *funcdecl::generate_decl()
{
  if (!declared) {
    if (!extern_def) {
      /* the declaration. */
      fprintf(codefile, "%s %s(", returntype->generate_code(), mu_name);
      /* formal parameters */
      for (ste * p = params; p != NULL; p = p->getnext()) {
        p->getvalue()->generate_decl();
        if (p->getnext() == NULL)
          break;
        fprintf(codefile, ",");
      }
      fprintf(codefile, ")\n" "{\n");

      /* the locals. */
      decls->generate_decls();

      /* the statements. */
      for (stmt * s = body; s != NULL; s = s->next) {
        s->generate_code();
      }

      fprintf(codefile,
              "	Error.Error(\"The end of function %s reached without returning values.\");\n",
              name);

      fprintf(codefile, "};\n");

      fprintf(codefile, "/*** end function declaration ***/\n\n");

      declared = TRUE;
    } else if (include_file_ext != NULL)
      fprintf(codefile, "\n#include \"%s\"\n\n", include_file_ext);
  }
  return "ERROR!";
}

/********************
  code related to expression
  --
  -- expr
  -- boolexpr
  -- -- notexpr
  -- -- equalexpr
  -- -- compexpr
  -- arithexpr
  -- -- unexpr
  -- -- mulexpr
  -- condexpr
  -- quantexpr
  -- designator
  -- exprlist
  -- funccall
 ********************/

/********************
  code for expr
 ********************/
const char *expr::generate_code()
{
  if (constval)
    if (type_equal(type, realtype))
      return tsprintf("%le", rvalue);	// AP: value of a real constant
    else
      return tsprintf("%d", value);
  else {
    Error.Error
    ("Internal: a basic expression that wasn't a constant called expr::generate_code().");
    return "ERROR!";
  }
}

//IM: for math functions
const char *mathexpr::generate_code()
{
  if (constval)
    return tsprintf("%le", rvalue);
  switch (getfuntype()) {
    case mylog: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", log(arg1->getrvalue()));
        else
          return tsprintf("%le", log(arg1->getvalue()));
      } else
        return tsprintf("log((double)%s)", arg1->generate_code());
      break;
    }
    case mylog10: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", log10(arg1->getrvalue()));
        else
          return tsprintf("%le", log10(arg1->getvalue()));
      } else
        return tsprintf("log10((double)%s)", arg1->generate_code());
      break;
    }
    case myexp: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", exp(arg1->getrvalue()));
        else
          return tsprintf("%le", exp(arg1->getvalue()));
      } else
        return tsprintf("exp((double)%s)", arg1->generate_code());
      break;
    }
    case mysin: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", sin(arg1->getrvalue()));
        else
          return tsprintf("%le", sin(arg1->getvalue()));
      } else
        return tsprintf("sin((double)%s)", arg1->generate_code());
      break;
    }
    case mycos: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", cos(arg1->getrvalue()));
        else
          return tsprintf("%le", cos(arg1->getvalue()));
      } else
        return tsprintf("cos((double)%s)", arg1->generate_code());
      break;
    }
    case mytan: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", tan(arg1->getrvalue()));
        else
          return tsprintf("%le", tan(arg1->getvalue()));
      } else
        return tsprintf("tan((double)%s)", arg1->generate_code());
      break;
    }
    case myfabs: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", fabs(arg1->getrvalue()));
        else
          return tsprintf("%le", fabs(arg1->getvalue()));
      } else
        return tsprintf("fabs((double)%s)", arg1->generate_code());
      break;
    }
    case myfloor: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", floor(arg1->getrvalue()));
        else
          return tsprintf("%le", floor(arg1->getvalue()));
      } else
        return tsprintf("floor((double)%s)", arg1->generate_code());
      break;
    }
    case myceil: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", ceil(arg1->getrvalue()));
        else
          return tsprintf("%le", ceil(arg1->getvalue()));
      } else
        return tsprintf("ceil((double)%s)", arg1->generate_code());
      break;
    }
    case mysqrt: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", sqrt(arg1->getrvalue()));
        else
          return tsprintf("%le", sqrt(arg1->getvalue()));
      } else
        return tsprintf("sqrt((double)%s)", arg1->generate_code());
      break;
    }
    case myfmod: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype)) {
          if (type_equal(arg2->gettype(), realtype))
            return tsprintf("%le",
                            fmod(arg1->getrvalue(), arg2->getrvalue()));
          else
            return tsprintf("%le",
                            fmod(arg1->getrvalue(), arg2->getvalue()));
        } else {
          if (type_equal(arg2->gettype(), realtype))
            return tsprintf("%le",
                            fmod(arg1->getvalue(), arg2->getrvalue()));
          else
            return tsprintf("%le",
                            fmod(arg1->getvalue(), arg2->getvalue()));
        }
      } else
        return tsprintf("fmod((double)%s,(double)%s)",
                        arg1->generate_code(), arg2->generate_code());
      break;
    }
    case mypow: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype)) {
          if (type_equal(arg2->gettype(), realtype))
            return tsprintf("%le",
                            pow(arg1->getrvalue(), arg2->getrvalue()));
          else
            return tsprintf("%le",
                            pow(arg1->getrvalue(), arg2->getvalue()));
        } else {
          if (type_equal(arg2->gettype(), realtype))
            return tsprintf("%le",
                            pow(arg1->getvalue(), arg2->getrvalue()));
          else
            return tsprintf("%le",
                            pow(arg1->getvalue(), arg2->getvalue()));
        }
      } else
        return tsprintf("pow((double)%s,(double)%s)",
                        arg1->generate_code(), arg2->generate_code());
      break;
    }
    case myasin: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", asin(arg1->getrvalue()));
        else
          return tsprintf("%le", asin(arg1->getvalue()));
      } else
        return tsprintf("asin((double)%s)", arg1->generate_code());
      break;
    }
    case myacos: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", acos(arg1->getrvalue()));
        else
          return tsprintf("%le", acos(arg1->getvalue()));
      } else
        return tsprintf("acos((double)%s)", arg1->generate_code());
      break;
    }
    case myatan: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", atan(arg1->getrvalue()));
        else
          return tsprintf("%le", atan(arg1->getvalue()));
      } else
        return tsprintf("atan((double)%s)", arg1->generate_code());
      break;
    }
    case mysinh: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", sinh(arg1->getrvalue()));
        else
          return tsprintf("%le", sinh(arg1->getvalue()));
      } else
        return tsprintf("sinh((double)%s)", arg1->generate_code());
      break;
    }
    case mycosh: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", cosh(arg1->getrvalue()));
        else
          return tsprintf("%le", cosh(arg1->getvalue()));
      } else
        return tsprintf("cosh((double)%s)", arg1->generate_code());
      break;
    }
    case mytanh: {
      if (constval) {
        if (type_equal(arg1->gettype(), realtype))
          return tsprintf("%le", tanh(arg1->getrvalue()));
        else
          return tsprintf("%le", tanh(arg1->getvalue()));
      } else
        return tsprintf("tanh((double)%s)", arg1->generate_code());
      break;
    }
  }
}

//GDP: internal values
const char *specvalexpr::generate_code()
{
  switch (getspecvaltype()) {
    case mytimer:
      return tsprintf("StateSet->CurrentLevel()");
      break;
  }
}

/********************
  code for boolexpr
 ********************/
const char *boolexpr::generate_code()
{
  if (constval)
    return tsprintf("%d", value);
  else {
    int num = new_int();
    char *temp = tsprintf("mu__boolexpr%d", num);
    fprintf(codefile, "bool %s;\n", temp);

    switch (op) {
      case IMPLIES:
        fprintf(codefile, "  if (!(%s)) %s = TRUE ;\n"
                "  else {\n", left->generate_code(), temp);
        fprintf(codefile, "  %s = (%s) ; \n}\n", temp,
                right->generate_code());
        return temp;
        break;
      case '|':
        fprintf(codefile, "  if (%s) %s = TRUE ;\n"
                "  else {\n", left->generate_code(), temp);
        fprintf(codefile, "  %s = (%s) ; \n}\n", temp,
                right->generate_code());
        return temp;
        break;
      case '&':
        fprintf(codefile, "  if (!(%s)) %s = FALSE ;\n"
                "  else {\n", left->generate_code(), temp);
        fprintf(codefile, "  %s = (%s) ; \n}\n", temp,
                right->generate_code());
        return temp;
        break;
      default:
        Error.Error
        ("Internal: funky value for op in boolexpr::generate_code()");
        return "ERROR!";
        break;
    }
  }
}

/********************
  code for notexpr
 ********************/
const char *notexpr::generate_code()
{
  return tsprintf("!(%s)", left->generate_code());
}

/********************
  code for equalexpr
 ********************/
const char *equalexpr::generate_code()
{
  if (constval)
    return tsprintf("%d", value);
  else {
    switch (op) {
      case '=':
        return tsprintf("(%s) == (%s)",
                        left->generate_code(), right->generate_code());
        break;
      case NEQ:
        return tsprintf("(%s) != (%s)",
                        left->generate_code(), right->generate_code());
        break;
      default:
        Error.Error
        ("Internal: exciting value for op in equalexpr::generate_code().");
        return "ERROR!";
        break;
    }
  }
}

/********************
  code for compexpr
 ********************/
const char *compexpr::generate_code()
{
  if (constval)
    return tsprintf("%d", value);
  else {
    switch (op) {
      case '<':
        return tsprintf("(%s) < (%s)",
                        left->generate_code(), right->generate_code());
      case LEQ:
        return tsprintf("(%s) <= (%s)",
                        left->generate_code(), right->generate_code());
      case '>':
        return tsprintf("(%s) > (%s)",
                        left->generate_code(), right->generate_code());
      case GEQ:
        return tsprintf("(%s) >= (%s)",
                        left->generate_code(), right->generate_code());
      default:
        Error.Error("Internal: odd value in compexpr::generate_code()");
        return "ERROR!";
    }
  }
}

/********************
  code for arithexpr
 ********************/
const char *arithexpr::generate_code()
{
  if (constval)
    if (type_equal(type, realtype))
      return tsprintf("%le", rvalue);	// AP: value of a real arithmetic expression (+,-)
    else
      return tsprintf("%d", value);
  else {
    switch (op) {
      case '+':
        return tsprintf("(%s) + (%s)",
                        left->generate_code(), right->generate_code());
      case '-':
        return tsprintf("(%s) - (%s)",
                        left->generate_code(), right->generate_code());
      default:
        Error.Error("Internal: bad operator in arithexpr::generate_code()");
        return "ERROR!";
    }
  }
};

/********************
  code for unexpr
 ********************/
const char *unexpr::generate_code()
{
  if (constval)
    if (type_equal(type, realtype))
      return tsprintf("%le", rvalue);	// AP: value of a real unary expression
    else
      return tsprintf("%d", value);
  else {
    switch (op) {
      case '+':
        return left->generate_code();
      case '-':
        return tsprintf(" - (%s)", left->generate_code());
      default:
        Error.Error("Internal: bad operator in arithexpr::generate_code()");
        return "ERROR!";
    }
  }
};

/********************
  code for mulexpr
 ********************/
const char *mulexpr::generate_code()
{
  if (constval)
    if (type_equal(type, realtype))
      return tsprintf("%le", rvalue);	// AP: value of a real arithmetic expression (*,/)
    else
      return tsprintf("%d", value);
  else {
    switch (op) {
      case '*':
        return tsprintf("(%s) * (%s)",
                        left->generate_code(), right->generate_code());
      case '/':
        return tsprintf("(%s) / (%s)",
                        left->generate_code(), right->generate_code());
      case '%':
        return tsprintf("(%s) %% (%s)",	/* doubled % to accomodate printf. */
                        left->generate_code(), right->generate_code());
      default:
        Error.Error("Internal: bad operator in mulexpr::generate_code()");
        return "ERROR!";
    }
  }
}

/********************
  code for condexpr
 ********************/
const char *condexpr::generate_code()
{
  return tsprintf("(%s) ? (%s) : (%s)",
                  test->generate_code(),
                  left->generate_code(), right->generate_code());
}

/********************
  code for quantexpr
 ********************/

void make_quant_fors(ste * quants)
{
  if (quants != NULL && quants->getvalue()->getclass() == decl::Quant) {
    make_quant_fors(quants->getnext());
    ((quantdecl *) quants->getvalue())->make_for();
  }
}

void make_quant_closes(ste * quants)
{
  if (quants != NULL && quants->getvalue()->getclass() == decl::Quant) {
    make_quant_closes(quants->getnext());
    fprintf(codefile, "};\n");
  }
}

const char *quantexpr::generate_code()
{
  int num = new_int();
  char *temp = tsprintf("mu__quant%d", num);
  bool isforall = ((op == FORALL) ? TRUE : FALSE);

  // Uli: initialization in the declaration not allowed in this case
  fprintf(codefile, "bool %s; \n%s = %s;\n",
          temp, temp, (isforall ? "TRUE" : "FALSE"));
  fprintf(codefile, "{\n");
  make_quant_fors(parameter);
  /* so the new parameter doesn\'t conflict with others if it\'s reused. */
  fprintf(codefile,
          "if ( %s(%s) )\n", (isforall ? "!" : ""), left->generate_code());
  // Uli: a "goto" to exit the for loop seemed to cause problems on some
  //      compilers when there were local variables
  fprintf(codefile,
          "  { %s = %s; break; }\n", temp, (isforall ? "FALSE" : "TRUE"));
  make_quant_closes(parameter);
  fprintf(codefile, "};\n");

  return temp;
}


/********************
  code for designator
 ********************/
const char *designator::generate_code()
{
  switch (dclass) {
    case Base:
      return origin->getvalue()->generate_code();
    case ArrayRef:
      return tsprintf("%s[%s]",
                      left->generate_code(), arrayref->generate_code());
      break;
    case FieldRef:
      return tsprintf("%s.%s",
                      left->generate_code(), fieldref->getvalue()->mu_name);
      break;
    default:
      Error.Error
      ("Internal: Strange and mysterious values for designator::dclass.");
      return "ERROR!";
      break;
  }
}


/********************
  code for exprlist
 ********************/
const char *exprlist::generate_code(const char *name, ste * formals)
{
  exprlist *ex = this;
  if (this == NULL) {
    // exprlist_buffer_end = exprlist_buffer;
    return "ERROR!";
  } else {
    char *exprlist_buffer = new char[BUFFER_SIZE];
    char *exprlist_buffer_end = exprlist_buffer;
    for (ex = this; formals != NULL && ex != NULL;
         formals = formals->getnext(), ex = ex->next) {
      param *f = (param *) formals->getvalue();
      if (!ex->undefined) {
        if (f->gettype() != ex->e->gettype() &&
            ((ex->e->gettype()->gettypeclass() == typedecl::Range
              && ((f->gettype()->gettype() == ex->e->gettype()->gettype())
                  || (type_equal(f->gettype(), realtype)))
             )
             ||
             ((ex->e->gettype()->gettypeclass() == typedecl::Scalarset
               || ex->e->gettype()->gettypeclass() == typedecl::Enum)
              && f->gettype()->gettypeclass() == typedecl::Union)
             ||
             (ex->e->gettype()->gettypeclass() == typedecl::Union
              && (f->gettype()->gettypeclass() == typedecl::Scalarset
                  || f->gettype()->gettypeclass() == typedecl::Enum))
            )
           )
          sprintf(exprlist_buffer_end, ", (int)%s",
                  ex->e->generate_code());
        else if (f->gettype() != ex->e->gettype()
                 && (type_equal(ex->e->gettype(), realtype)
                     && f->gettype()->gettype() ==
                     ex->e->gettype()->gettype()))
          sprintf(exprlist_buffer_end, ", (double)%s",
                  ex->e->generate_code());
        else
          sprintf(exprlist_buffer_end, ", %s", ex->e->generate_code());
      } else {
        sprintf(exprlist_buffer_end, ", %s_undefined_var",
                f->gettype()->generate_code());
      }
      exprlist_buffer_end += strlen(exprlist_buffer_end);
    }
    if (strlen(exprlist_buffer) > BUFFER_SIZE)
      Error.Error("Internal: Buffer size for expression list overflow.\n"
                  "	  Please increase BUFFER_SIZE in /src/mu.h");
    return (exprlist_buffer + 2);	// + 2 to skip the leading comma and space
    /* BUG: Aargh! We can\'t delete the buffer! */
  }
}

// char *exprlist::generate_code()
// {
//   exprlist *ex = this;
//   if (this == NULL ) {
//     // exprlist_buffer_end = exprlist_buffer;
//     return "ERROR!";
//   }
//   else {
//     char *exprlist_buffer = new char[BUFFER_SIZE];
//     char *exprlist_buffer_end = exprlist_buffer;
//     for (ex = this; ex != NULL; ex = ex->next) {
//       sprintf(exprlist_buffer_end, ", %s", ex->e->generate_code() );
//       exprlist_buffer_end += strlen(exprlist_buffer_end);
//     }
//     return ( exprlist_buffer + 2); // + 2 to skip the leading comma and space
//     /* BUG: Aargh! We can\'t delete the buffer! */
//     }
// }

/********************
  code for funccall
 ********************/
/* BUG: Caution: is there any way the generated statement can end up in
   the middle of an expression? */

const char *funccall::generate_code()
{
  funcdecl *f = (funcdecl *) func->getvalue();
  return tsprintf("%s( %s )",
                  //im: for imported functions, "mu_" does not have to be prefixed to the function name
                  f->extern_def ? &(func->
                                    getvalue()->generate_code())[3] :
                  func->getvalue()->generate_code(),
                  actuals !=
                  NULL ? actuals->generate_code(func->getname()->getname(),
                      f->params) : "");
}

// char *funccall::generate_code()
// {
//   return tsprintf("%s( %s )",
//      func->getvalue()->generate_code(),
//      actuals != NULL ? actuals->generate_code() : "");
// }

/********************
  code for isundefined
  ********************/
const char *isundefined::generate_code()
{
  return tsprintf("%s.isundefined()", left->generate_code());
}

/********************
  code for ismember
  ********************/
const char *ismember::generate_code()
{
  return tsprintf("(%s>=%d && %s<=%d)",
                  left->generate_code(), t->getleft(),
                  left->generate_code(), t->getright());
}

/********************
  code for multisetcount
  ********************/
void multisetcount::generate_decl(multisettypedecl * mset)
{
  /*
    if (mset == set->gettype())
      {
        fprintf(codefile,"int multisetcount%d();\n",
  	  multisetcount_num
  	  );
      }
      */
}

void multisetcount::generate_procedure()
{
}

const char *multisetcount::generate_code()
{
  /* set->gettype()->getelementtype()->generate_code(), /* element type */
  /* set->gettype()->generate_code(), /* multiset type */
  /*  multisetcount_num,               /* procedure number */
  int num = new_int();
  char *temp = tsprintf("mu__intexpr%d", num);
  fprintf(codefile, "/*** begin multisetcount %d declaration ***/\n",
          multisetcount_num);
  fprintf(codefile, "  int %s = 0;\n", temp);

  fprintf(codefile, "  {\n" "  %s_id %s;\n" "  for (%s = 0; ; %s=%s+1)\n" "    {\n" "      if (%s.valid[(int)%s].value())\n"	// Uli 10-98
          "	{\n", set->gettype()->generate_code(),	/* multiset type */
          index->getvalue()->generate_code(),	/* index name */
          index->getvalue()->generate_code(),	/* index name */
          index->getvalue()->generate_code(),	/* index name */
          index->getvalue()->generate_code(),	/* index name */
          set->generate_code(),	/* multiset variable name */
          index->getvalue()->generate_code()	/* index name */
         );
  fprintf(codefile, "	  if ( %s ) %s++;\n" "	}\n" "      if (%s == %d-1) break;\n" "    }\n" "  }\n", filter->generate_code(),	/* bool expression */
          temp, index->getvalue()->generate_code(),	/* index name */
          ((multisettypedecl *) set->gettype())->getindexsize()	/* max size */
         );
  fprintf(codefile, "/*** end multisetcount %d declaration ***/\n",
          multisetcount_num);
  return tsprintf("%s", temp);
}

/********************
  code related to stmt
  --
  -- stmt
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

/********************
  code for stmt
 ********************/
const char *stmt::generate_code()
{
  /* There now is the null statement--
   * it is legal to call stmt::generate_code();
   * it must be the null statement, so it just does nothing.
   */
  /*  Error.Error("Internal: stmt::generate_code() should never have been called."); */
  return "ERROR!";
}

/********************
  code for assignment
 ********************/
const char *assignment::generate_code()
{
  if (src->isdesignator()
      && src->gettype()->issimple()
      && src->checkundefined())
//       && src->gettype()->getclass() != decl::Quant
//       && src->gettype()->getclass() != decl::Const
//       && src->gettype()->getclass() != decl::Type
//       && src->gettype()->gettype()->issimple() )
  {
    fprintf(codefile,
            "if (%s.isundefined())\n"
            "  %s.undefine();\n"
            "else\n"
            "  %s = %s;\n",
            src->generate_code(),
            target->generate_code(),
            target->generate_code(), src->generate_code());
  } else {
    fprintf(codefile, "%s = %s;\n",
            target->generate_code(), src->generate_code());
  }
  return "ERROR!";
}

/*
 * WP WP WP WP WP WP WP WP WP WP WP WP
 * get the effects of an action (fact names)
 */
const char *assignment::get_target()
{
	return (target->generate_code());
}

/*
 * WP WP WP WP WP WP WP WP WP WP WP WP
 * get the effects of an action (fact values)
 */
const char *assignment::get_src()
{
	return (src->generate_code());
}

/********************
  code for whilestmt
 ********************/
const char *whilestmt::generate_code()
{
  char *counter = tsprintf("mu__counter_%d", new_int());
  char *while_expr = tsprintf("mu__while_expr_%d", new_int());

  // Uli: a "goto" to exit the while loop seemed to cause problems on some
  //      compilers when there were local variables

  // set mu__while_expr<n> to the value of the expr
  fprintf(codefile,
          "{\n"
          "  bool %s;"
          "  %s = %s;\n", while_expr, while_expr, test->generate_code());

  fprintf(codefile,
          "int %s = 0;\n"
          "while (%s) {\n"
          "if ( ++%s > args->loopmax.value )\n"
          "  Error.Error(\"Too many iterations in while loop.\");\n",
          counter, while_expr, counter);

  // nest a block so that the code within the loop
  // can generate variables if it needs to.
  fprintf(codefile, "{\n");

  for (stmt * s = body; s != NULL; s = s->next)
    s->generate_code();

  fprintf(codefile, "};\n");

  // set mu__while_expr<n> to the value of the expr
  // before the C++ while checks the variable.
  fprintf(codefile, "%s = %s;\n", while_expr, test->generate_code());

  fprintf(codefile, "}\n" "};\n");

  return "ERROR!";
}

/********************
  code for ifstmt
  TODO
 ********************/
const char *ifstmt::generate_code()
{
  fprintf(codefile, "if ( %s )\n" "{\n", test->generate_code());

  for (stmt * s = body; s != NULL; s = s->next){
	  s->generate_code();
  }
  fprintf(codefile, "}\n");
  if (elsecode != NULL) {
    fprintf(codefile, "else\n" "{\n");
    for (stmt * s = elsecode; s != NULL; s = s->next)
      s->generate_code();

    fprintf(codefile, "}\n");
  }
  return "ERROR!";
}




/*
 * WP WP WP WP WP WP WP WP WP WP WP WP
 * get the effects of an action/process (fact names)
 */
std::string ifstmt::get_proc_code_plus()
{

	  std::string proc_pluses = "\n";
//	  std::string proc_pluses = "\n	if ( ";

//	  proc_pluses.append(test->generate_code());



	  proc_pluses.append("\n	{\n");

	  for (stmt * s = body; s != NULL; s = s->next){

		  std::string psrc(s->get_src());
		  std::string ptarget(s->get_target());

		  if ( ptarget.find("ERROR") == std::string::npos && psrc.find("ERROR") == std::string::npos &&
				  (psrc.find("increase") != std::string::npos ||  psrc.find("+") != std::string::npos) ){

//			  	  	  std::cout << "THIS IS THE PLUS CODE:" << ptarget << " = " << psrc << std::endl;

			  	  	  proc_pluses.append(("		" + ptarget + " = " + psrc + "; \n"));
		  }
	  }
	  proc_pluses.append("	}\n");
	  if (elsecode != NULL) {
	    proc_pluses.append("else\n{\n");
	    for (stmt * s = elsecode; s != NULL; s = s->next)
//	      s->generate_code();
	    proc_pluses.append("\n}\n");
	  }


//	  std::cout << "\n\n WHOLE PLUS PROCESSES" << proc_pluses.c_str() << std::endl;


	  return proc_pluses;
}



/*
 * WP WP WP WP WP WP WP WP WP WP WP WP
 * get the effects of an action/process (fact names)
 */
std::string ifstmt::get_proc_code_minus()
{


	  std::string proc_minuses = "\n";

//	  WP WP WP WP WP COMMENTED OUT FOR TESTING
//	  global_test_minus.push_back(test);

//	  std::string proc_minuses = "\n	if ( ";

//	  proc_minuses.append(test->generate_code());


	  proc_minuses.append("\n	{\n");

	  for (stmt * s = body; s != NULL; s = s->next){

		  std::string psrc(s->get_src());
		  std::string ptarget(s->get_target());


		  if ( ptarget.find("ERROR") == std::string::npos && psrc.find("ERROR") == std::string::npos &&
				  (psrc.find("decrease") != std::string::npos ||  psrc.find("-") != std::string::npos) ){

//			  	  	  std::cout << "THIS IS THE MINUS CODE:" << ptarget << " = " << psrc << std::endl;
			  	  	  proc_minuses.append(("		" + ptarget + " = " + psrc + "; \n"));
		  }
	  }
	  proc_minuses.append("	}\n");
	  if (elsecode != NULL) {
		  proc_minuses.append("else\n{\n");
	    for (stmt * s = elsecode; s != NULL; s = s->next)
//	      s->generate_code();
	    	proc_minuses.append("}\n");
	  }

//	  std::cout << "\n\n WHOLE MINUS PROCESSES" << proc_minuses.c_str() << std::endl;

	  return proc_minuses;
}



/********************
  code for caselist
 ********************/
const char *caselist::generate_code()
{
  for (exprlist * v = values; v != NULL; v = v->next)
    fprintf(codefile, "case %s:\n", v->e->generate_code());

  for (stmt * b = body; b != NULL; b = b->next)
    b->generate_code();
  fprintf(codefile, "break;\n");
  return "ERROR!";
}

/********************
  code for switchstmt
 ********************/
const char *switchstmt::generate_code()
{
  fprintf(codefile, "switch ((int) %s) {\n", switchexpr->generate_code());
  /* The explicit cast seems to be necessary to allow things like
   * switch arr[i]... */
  for (caselist * c = cases; c != NULL; c = c->next)
    c->generate_code();
  if (elsecode != NULL) {
    fprintf(codefile, "default:\n");

    for (stmt * b = elsecode; b != NULL; b = b->next)
      b->generate_code();

    fprintf(codefile, "break;\n");
  }
  fprintf(codefile, "}\n");
  return "ERROR!";
}

/********************
  code for forstmt
 ********************/
const char *forstmt::generate_code()
{
  fprintf(codefile, "{\n");
  make_quant_fors(index);
  for (stmt * b = body; b != NULL; b = b->next)
    b->generate_code();
  make_quant_closes(index);
  fprintf(codefile, "};\n");
  return "ERROR!";
}

/********************
  code for proccall
 ********************/
const char *proccall::generate_code()
{
  procdecl *p = (procdecl *) procedure->getvalue();
  fprintf(codefile, "%s ( %s );\n",
          //im: for imported procedures, "mu_" does not have to be prefixed to the function name
          p->extern_def ? &(procedure->
                            getvalue()->generate_code())[3] : procedure->
          getvalue()->generate_code(),
          actuals !=
          NULL ? actuals->generate_code(procedure->getname()->getname(),
                                        p->params) : "");
  return "ERROR!";
}

// char *proccall::generate_code()
// {
//   fprintf(codefile,"%s ( %s );\n",
//    procedure->getvalue()->generate_code(),
//    actuals != NULL ? actuals->generate_code() : "" );
//   return "ERROR!";
// }

/********************
  code for clearstmt
 ********************/
const char *clearstmt::generate_code()
{
  // Gotta figure this one out--
  // current best idea: give every object a clear method.
  fprintf(codefile, "%s.clear();\n", target->generate_code());
  return "ERROR!";
}

/********************
  code for undefinestmt
  ********************/
const char *undefinestmt::generate_code()
{
  fprintf(codefile, "%s.undefine();\n", target->generate_code());
  return "ERROR!";
}

/********************
  code for multisetaddstmt
  ********************/
const char *multisetaddstmt::generate_code()
{
  fprintf(codefile, "%s.multisetadd(%s);\n",
          target->generate_code(), element->generate_code());
  return "ERROR!";
}

/********************
  code for multisetremovestmt
  ********************/
void multisetremovestmt::generate_decl(multisettypedecl * mset)
{
  /*
    if (mset == target->gettype())
      {
        fprintf(codefile,"void multisetremove%d();\n",
  	  multisetremove_num
  	  );
      }
      */
}

void multisetremovestmt::generate_procedure()
{
}

const char *multisetremovestmt::generate_code()
{
  if (matchingchoose) {
    fprintf(codefile, "%s.multisetremove(%s);\n",
            target->generate_code(), criterion->generate_code());
  } else {
    int num = new_int();
    char *temp = tsprintf("mu__idexpr%d", num);
    fprintf(codefile, "/*** end multisetremove %d declaration ***/\n",
            multisetremove_num);
    fprintf(codefile, "  %s_id %s;\n", target->gettype()->generate_code(),	/* multiset type */
            temp);

    fprintf(codefile, "  %s_id %s;\n" "  for (%s = 0; ; %s=%s+1)\n" "    {\n" "      if (%s.valid[(int)%s].value())\n"	// Uli 01-99
            "	{\n", target->gettype()->generate_code(),	/* multiset type */
            index->getvalue()->generate_code(),	/* index name */
            index->getvalue()->generate_code(),	/* index name */
            index->getvalue()->generate_code(),	/* index name */
            index->getvalue()->generate_code(),	/* index name */
            target->generate_code(),	/* multiset variable name */
            index->getvalue()->generate_code()	/* index name */
           );
    fprintf(codefile, "	  if ( %s ) { %s = %s; %s.multisetremove(%s); };\n" "	}\n" "      if (%s == %d-1) break;\n" "    }\n", criterion->generate_code(),	/* bool expression */
            temp, index->getvalue()->generate_code(),	/* index name */
            target->generate_code(),	/* multiset variable name */
            temp, index->getvalue()->generate_code(),	/* index name */
            ((multisettypedecl *) target->gettype())->getindexsize()	/* max size */
           );
    fprintf(codefile, "/*** end multisetremove %d declaration ***/\n",
            multisetremove_num);
  }

  return "ERROR!";
}

/********************
  code for errorstmt
 ********************/
const char *errorstmt::generate_code()
{
  fprintf(codefile, "Error.Error(\"Error: %s\");\n", string);
  return "ERROR!";
}

/********************
  code for assertstmt
 ********************/
const char *assertstmt::generate_code()
{
  fprintf(codefile,
          "if ( !(%s) ) Error.Error(\"Assertion failed: %s\");\n",
          test->generate_code(), string);
  return "ERROR!";
}

/********************
  code for putstmt
 ********************/
const char *putstmt::generate_code()
{
  if (putexpr != NULL) {
    if (putexpr->islvalue())
      fprintf(codefile, "%s.print();\n", putexpr->generate_code());
    else
      fprintf(codefile, "cout << ( %s );\n", putexpr->generate_code());
  } else {
    fprintf(codefile, "cout << \"%s\";\n", putstring);
  }
  return "ERROR!";
}

/********************
  code for alias
 ********************/
const char *alias::generate_code()
/* not used right now. */
{
  return "ERROR!";
}

/********************
  code for aliasstmt
 ********************/
const char *aliasstmt::generate_code()
{
  fprintf(codefile, "{\n");
  aliases->generate_decls();
  for (stmt * b = body; b != NULL; b = b->next)
    b->generate_code();
  fprintf(codefile, "}\n");
  return "ERROR!";
}

/********************
  code for returnstmt
 ********************/
const char *returnstmt::generate_code()
{
  // Uli: the return expression is not converted to int. Therefore, the
  // copy constructor is used to create a temporary object for the return
  // expression. Advantage: complex types can be returned.
  // (no change in this routine)
  fprintf(codefile, "return %s;\n",
          retexpr != NULL ? retexpr->generate_code() : "");

//  if (retexpr) fprintf(codefile,"return %s;\n", retexpr->generate_code());
//  else fprintf(codefile, "return;\n");
  return "ERROR!";
}

/********************
  code related to rules
 ********************/
static char rule_param_buffer[BUFFER_SIZE];
static ste *stequants[MAXSCOPES];	/* We can\'t have more enclosing rulesets than scopes. */
static int numquants = 0;
static char namequants[BUFFER_SIZE];
static char quantactuals[BUFFER_SIZE];

/********************
  aux_generate_rule_params
  -- extract "Quant" from enclosure and generate the string for it
  -- i.e. the string to be placed in condition/rule function
  -- formal
 ********************/
// no longer used
static char *aux_generate_rule_params(ste * enclosures)
/* returns a pointer to the '\0' at the end of the string in
 * rule_param_buffer. */
{
  char *temp;
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias)) {
    temp = aux_generate_rule_params(enclosures->getnext());
    if (enclosures->getvalue()->getclass() == decl::Quant)
      sprintf(temp,
              ", const %s &%s",
              enclosures->getvalue()->gettype()->generate_code(),
              enclosures->getvalue()->generate_code());
    return (temp + strlen(temp));
  } else
    return rule_param_buffer;
}

/********************
  generate_rule_params
  -- initialize buffer "rule_param_buffer
  -- and call aux_generate_rule_params
  -- to return the appropriate string
  -- i.e. the string to be placed in condition/rule function
  -- formal
 ********************/

static char *generate_rule_params(ste * enclosures)
/* assumes that enclosures is a pointer to a list of ste\'s,
 * with ruleset parameters and aliases at the front. */
{
  int i = 0;
  for (i = 0; rule_param_buffer[i] != '\0' && i < BUFFER_SIZE; i++)
    rule_param_buffer[i] = '\0';
  aux_generate_rule_params(enclosures);
  return (rule_param_buffer + 1);	/* skip the leading comma. */
}

/********************
  generate_rule_params_assignment
 ********************/
// void generate_rule_params_assignment_union(ste *enclosures)
// {
//         stelist * t;
//         typedecl * d;
//         int thisright = enclosures->getvalue()->gettype()->getsize()-1;
//         int thisleft;
//
//         for(t=((uniontypedecl *)indextype)->getunionmembers();
//             t!= NULL; t=t->next)
//           {
//             d = (typedecl *)t->s->getvalue();
//             thisleft = thisright - d->getsize() + 1;
//
//   fprintf(codefile,
//    "    if (int_
//    "    %s_id %s = (r %% %d) + %d;\n"
//    ((multisetidtypedecl *)enclosures->getvalue()->gettype())
//    ->getparenttype()->generate_code(),
//    enclosures->getvalue()->gettype()->getleft(),
//    );
//
//
//
// //       for (i=thisleft; i<=thisright; i++)
//      for (i=d->getleft(); i<=d->getright(); i++)
//      for (e = elt; e!=NULL; e=e->next)
//        ret = new charlist(tsprintf("[%d]%s",i, e->string), e->e, ret);
//             thisright= thisleft - 1;
//           }
//
// }

void generate_rule_params_assignment(ste * enclosures)
/* assumes that enclosures is a pointer to a list of ste\'s,
 * with ruleset parameters and aliases at the front. */
{
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    generate_rule_params_assignment(enclosures->getnext());
    if (enclosures->getvalue()->gettype()->gettypeclass() ==
        typedecl::Union) {
//    if ( enclosures->getvalue()->getclass() == decl::Choose )
//      {
//      fprintf(codefile,
//          "    int int_%s =  (r %% %d);\n"
//          enclosures->getvalue()->generate_code(),
//          enclosures->getvalue()->gettype()->getsize()
//          );
//      generate_rule_params_assignment_union(enclosures);
//      fprintf(codefile,
//        "    r = r / %d;\n",
//        enclosures->getvalue()->gettype()->getsize()
//        );
//      }
      if (enclosures->getvalue()->getclass() == decl::Choose)
        fprintf(codefile,
                "    static %s_id %s;\n"
                "    %s.unionassign( r %% %d );\n"
                "    r = r / %d;\n",
                ((multisetidtypedecl *) enclosures->getvalue()->gettype())
                ->getparenttype()->generate_code(),
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->gettype()->getsize(),
                enclosures->getvalue()->gettype()->getsize()
               );
      if (enclosures->getvalue()->getclass() == decl::Quant)
        fprintf(codefile,
                "    static %s %s;\n"
                "    %s.unionassign(r %% %d);\n"
                "    r = r / %d;\n",
                enclosures->getvalue()->gettype()->generate_code(),
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->gettype()->getsize(),
                enclosures->getvalue()->gettype()->getsize()
               );
    } else {
      if (enclosures->getvalue()->getclass() == decl::Choose)
        fprintf(codefile,
                "    static %s_id %s;\n"
                "    %s.value((r %% %d) + %d);\n"
                "    r = r / %d;\n",
                ((multisetidtypedecl *) enclosures->getvalue()->gettype())
                ->getparenttype()->generate_code(),
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->gettype()->getsize(),
                enclosures->getvalue()->gettype()->getleft(),
                enclosures->getvalue()->gettype()->getsize()
               );
      if (enclosures->getvalue()->getclass() == decl::Quant)
        fprintf(codefile,
                "    static %s %s;\n"
                "    %s.value((r %% %d) + %d);\n"
                "    r = r / %d;\n",
                enclosures->getvalue()->gettype()->generate_code(),
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->gettype()->getsize(),
                enclosures->getvalue()->gettype()->getleft(),
                enclosures->getvalue()->gettype()->getsize()
               );
    }
  }
}

void generate_rule_params_simple_assignment(ste * enclosures)
/* assumes that enclosures is a pointer to a list of ste\'s,
 * with ruleset parameters and aliases at the front. */
{
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    generate_rule_params_simple_assignment(enclosures->getnext());
    if (enclosures->getvalue()->gettype()->gettypeclass() ==
        typedecl::Union) {
      if (enclosures->getvalue()->getclass() == decl::Choose)
        fprintf(codefile,
                "    %s.unionassign( r %% %d );\n"
                "    r = r / %d;\n",
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->gettype()->getsize(),
                enclosures->getvalue()->gettype()->getsize()
               );
      if (enclosures->getvalue()->getclass() == decl::Quant)
        fprintf(codefile,
                "    %s.unionassign(r %% %d);\n"
                "    r = r / %d;\n",
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->gettype()->getsize(),
                enclosures->getvalue()->gettype()->getsize()
               );
    } else {
      if (enclosures->getvalue()->getclass() == decl::Choose)
        fprintf(codefile,
                "    %s.value((r %% %d) + %d);\n"
                "    r = r / %d;\n",
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->gettype()->getsize(),
                enclosures->getvalue()->gettype()->getleft(),
                enclosures->getvalue()->gettype()->getsize()
               );
      if (enclosures->getvalue()->getclass() == decl::Quant)
        fprintf(codefile,
                "    %s.value((r %% %d) + %d);\n"
                "    r = r / %d;\n",
                enclosures->getvalue()->generate_code(),
                enclosures->getvalue()->gettype()->getsize(),
                enclosures->getvalue()->gettype()->getleft(),
                enclosures->getvalue()->gettype()->getsize()
               );
    }
  }
}

void generate_rule_params_choose(ste * enclosures)
/* assumes that enclosures is a pointer to a list of ste\'s,
 * with ruleset parameters and aliases at the front. */
{
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    generate_rule_params_choose(enclosures->getnext());
    if (enclosures->getvalue()->getclass() == decl::Choose)
      fprintf(codefile, "  if (!%s.in(%s)) { return FALSE; }\n",
              ((multisetidtypedecl *) enclosures->getvalue()->gettype())
              ->getparentname(), enclosures->getvalue()->mu_name);
  }
}

void generate_rule_params_choose_exist(ste * enclosures)
/* assumes that enclosures is a pointer to a list of ste\'s,
 * with ruleset parameters and aliases at the front. */
{
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    generate_rule_params_choose_exist(enclosures->getnext());
    if (enclosures->getvalue()->getclass() == decl::Choose)
      fprintf(codefile, "&& %s.value()<%d ",
              enclosures->getvalue()->mu_name,
              enclosures->getvalue()->gettype()->getsize()
             );
  }
}


void generate_rule_params_choose_next(ste * enclosures, RULE_INDEX_TYPE end)
// Uli: unsigned short -> unsigned
/* assumes that enclosures is a pointer to a list of ste\'s,
 * with ruleset parameters and aliases at the front. */
{
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    generate_rule_params_choose(enclosures->getnext());
    if (enclosures->getvalue()->getclass() == decl::Choose)
      fprintf(codefile, "	    if (!%s.in(%s)) { return %lu; }\n",
              ((multisetidtypedecl *) enclosures->getvalue()->gettype())
              ->getparentname(), enclosures->getvalue()->mu_name, end);
  }
}

void generate_rule_params_choose_all_in(ste * enclosures)
/* assumes that enclosures is a pointer to a list of ste\'s,
 * with ruleset parameters and aliases at the front. */
{
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    generate_rule_params_choose_all_in(enclosures->getnext());
    if (enclosures->getvalue()->getclass() == decl::Choose)
      fprintf(codefile, "&& %s.in(%s)",
              ((multisetidtypedecl *) enclosures->getvalue()->gettype())
              ->getparentname(), enclosures->getvalue()->mu_name);
  }
}

void generate_rule_params_printf(ste * enclosures)
/* assumes that enclosures is a pointer to a list of ste\'s,
 * with ruleset parameters and aliases at the front. */
{
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    generate_rule_params_printf(enclosures->getnext());
    if (enclosures->getvalue()->getclass() == decl::Quant ||
        enclosures->getvalue()->getclass() == decl::Choose)
      fprintf(codefile, ", %s:%%s", enclosures->getvalue()->name);
  }
}

/*
bool generate_rule_params_printf_pddl(ste * enclosures, bool firstcall=true)
{
  if (enclosures != NULL && (enclosures->getvalue()->getclass() == decl::Quant || enclosures->getvalue()->getclass() == decl::Alias || enclosures->getvalue()->getclass() == decl::Choose)) {
    bool outer = !generate_rule_params_printf_pddl(enclosures->getnext(),false);
    if (enclosures->getvalue()->getclass() == decl::Quant || enclosures->getvalue()->getclass() == decl::Choose) {
		fprintf(codefile, "%c%%s%s",outer?'(':',',firstcall?")":"");
	}
	return true;
  }
  return false;
}
*/

/*
bool generate_rule_params_printf_pddl(ste * enclosures, bool firstcall=true)
{
  if (enclosures != NULL && (enclosures->getvalue()->getclass() == decl::Quant || enclosures->getvalue()->getclass() == decl::Alias || enclosures->getvalue()->getclass() == decl::Choose)) {
    bool outer = !generate_rule_params_printf_pddl(enclosures->getnext(),false);
    if (enclosures->getvalue()->getclass() == decl::Quant || enclosures->getvalue()->getclass() == decl::Choose) {
		fprintf(codefile, "%c%%s%s",outer?' ':' ',firstcall?"":"");
	}
	return true;
  }
  return false;
}
*/

void generate_rule_params_printf_pddl(ste * enclosures)
/* assumes that enclosures is a pointer to a list of ste\'s,
 * with ruleset parameters and aliases at the front. */
{
  while(enclosures != NULL &&
        (enclosures->getvalue()->getclass() == decl::Quant ||
         enclosures->getvalue()->getclass() == decl::Alias ||
         enclosures->getvalue()->getclass() == decl::Choose)) {

    if (enclosures->getvalue()->getclass() == decl::Quant ||
        enclosures->getvalue()->getclass() == decl::Choose) {
      fprintf(codefile, " %%s");
    }
    enclosures=enclosures->getnext();
  }
}

void generate_rule_params_name_pddl(ste * enclosures)
{
  while(enclosures != NULL &&
        (enclosures->getvalue()->getclass() == decl::Quant ||
         enclosures->getvalue()->getclass() == decl::Alias ||
         enclosures->getvalue()->getclass() == decl::Choose)) {

    if (enclosures->getvalue()->getclass() == decl::Quant ||
        enclosures->getvalue()->getclass() == decl::Choose) {
      fprintf(codefile,", %s.Name()", enclosures->getvalue()->generate_code());
    }
    enclosures=enclosures->getnext();
  }
}


void generate_rule_params_name(ste * enclosures)
/* assumes that enclosures is a pointer to a list of ste\'s,
 * with ruleset parameters and aliases at the front. */
{
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    generate_rule_params_name(enclosures->getnext());
    if (enclosures->getvalue()->getclass() == decl::Quant ||
        enclosures->getvalue()->getclass() == decl::Choose)
      fprintf(codefile,
              ", %s.Name()", enclosures->getvalue()->generate_code()
             );
  }
}

/********************
  generate_rule_aliases
  -- extract "Alias" from enclosure and generate the code for it
 ********************/
static const char *generate_rule_aliases(ste * enclosures)
/* generate alias declarations for the rule. */
{
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    generate_rule_aliases(enclosures->getnext());
    if (enclosures->getvalue()->getclass() == decl::Alias)
      enclosures->getvalue()->generate_decl();
    return "ERROR!";
  } else
    return "ERROR!";
}

/********************
  enroll_quants
  -- enter each "Quant" enclosure, i.e. ruleset, into
  -- the list "stequants" for easy recursion to find every
  -- instance of a ruleset
 ********************/
static void enroll_quants(ste * enclosures)
{
  if (enclosures != NULL &&
      (enclosures->getvalue()->getclass() == decl::Quant ||
       enclosures->getvalue()->getclass() == decl::Alias ||
       enclosures->getvalue()->getclass() == decl::Choose)) {
    enroll_quants(enclosures->getnext());
    if (enclosures->getvalue()->getclass() == decl::Quant ||
        enclosures->getvalue()->getclass() == decl::Choose) {
      if (numquants >= MAXSCOPES) {
        Error.FatalError
        ("Current implementation only allows %d nested rulesets.",
         MAXSCOPES);
      }
      stequants[numquants] = enclosures;
      numquants++;
    }
  } else {
    numquants = 0;
  }
}

// WP WP WP WP WP
int gcnum = 0;

/********************
  static void generate_rules_aux(...)
  -- called by generate_rules
  --
  -- generate every instance of a ruleset, by recursively
  -- going down the list of enclosure
  --
  -- done recursively because the cost of this program isn\'t that huge,
  -- and recursion is much easier for me to understand.
 ********************/
static void
generate_rules_aux(int quantindex,
                   char *namequants_end,
                   char *quantactuals_end, simplerule * therule)
{
  rulerec *rr;
  int rule_int = new_int();
  char *condname;
  char *rulename;
  if (quantindex >= numquants) {
    /* generate the code for the rule. */
    /* and enroll it into appropriate list */
    if (therule->condition != NULL) {
      condname = tsprintf("mu__condition_%d", rule_int);
      fprintf(codefile, "bool %s() // Condition for Rule \"%s%s\"\n" "{\n" "  return %s(%s );\n" "}\n\n", condname, therule->name, namequants, therule->condname, quantactuals + 1);	/* + 1 to skip the leading comma. */
    } else
      condname = NULL;
    if (therule->body != NULL) {
      rulename = tsprintf("mu__rule_%d", rule_int);
      fprintf(codefile, "void %s() // Action for Rule \"%s%s\"\n" "{\n" "  %s(%s );\n" "};\n\n", rulename, therule->name, namequants, therule->rulename, quantactuals + 1);	/* + 1 to skip the leading comma. */
    } else
      rulename = NULL;


    /**
     * WP WP WP WP WP WP WP WP WP WP WP WP
     *
     * GOAL CONDITION.
     */
    if(ff_enabled){
    	fprintf(codefile,"bool mu__goal__0%d(){ return %s(); } /* WP WP WP GOAL CONDITION CHECK */ ", gcnum, condname);
    }
    gcnum++;




    /* install it into the appropriate list. */
    rr = new rulerec(tsprintf("%s%s", therule->name, namequants),
                     condname, rulename);
    switch (therule->getclass()) {
      case rule::Simple:
        rr->next = theprog->rulelist;
        theprog->rulelist = rr;
        break;
      case rule::Startstate:	/* This function seems not to be called on start states! */
        rr->next = theprog->startstatelist;
        theprog->startstatelist = rr;
        break;
//UPMURPHI_BEGIN
      case rule::Goal:
        rr->next = theprog->goallist;
        theprog->goallist = rr;
        break;
//UPMURPHI_END
      case rule::Invar:
        rr->next = theprog->invariantlist;
        theprog->invariantlist = rr;
        break;
      default:
        Error.Error
        ("Internal: Exciting value for therule->getclass() in generate_rules_aux.");
        break;
    }
  } else {
    /* do some more recursion. */
    int i;
    char *names_end, *actuals_end;
    quantdecl *quant = (quantdecl *) stequants[quantindex]->getvalue();
    if (quant->left != NULL) {
      for (i = quant->left->getvalue(); i <= quant->right->getvalue();
           i += quant->by) {
        sprintf(namequants_end, ", %s:%d", quant->name, i);
        names_end = namequants_end + strlen(namequants_end);
        sprintf(quantactuals_end, ", %d", i);
        actuals_end = quantactuals_end + strlen(quantactuals_end);
        generate_rules_aux(quantindex + 1, names_end, actuals_end,
                           therule);
      }
    } else {
// rlm wrote :
//     for( ste *s = ((enumtypedecl *) quant->gettype())->getidvalues();
//      s != NULL;
//      s = s->getnext() )
// spark : ( s!= NULL ) is not reached properly
//   : ex. OMH.m with ruleset over enum type

      ste *s;
      stelist *t;
      typedecl *d;

      // put value or enum string to the rule name and parameter, according to type

      // Enum
      if (quant->gettype()->gettypeclass() == typedecl::Enum
          && quant->gettype() != booltype) {
        s = ((enumtypedecl *) quant->gettype())->getidvalues();
        for (i = quant->gettype()->getleft();
             i <= quant->gettype()->getright(); i++) {
          sprintf(namequants_end, ", %s:%s", quant->name,
                  s->getname()->getname());
          sprintf(quantactuals_end, ", %s",
                  s->getvalue()->generate_code());
          s = s->getnext();
          names_end = namequants_end + strlen(namequants_end);
          actuals_end = quantactuals_end + strlen(quantactuals_end);
          generate_rules_aux(quantindex + 1, names_end, actuals_end,
                             therule);
        }
      } else if (quant->gettype() == booltype) {
        for (i = 0; i <= 1; i++) {
          if (i == 0) {
            sprintf(namequants_end, ", %s:false", quant->name);
            sprintf(quantactuals_end, ", mu_false");
          } else {
            sprintf(namequants_end, ", %s:true", quant->name);
            sprintf(quantactuals_end, ", mu_true");
          }
          names_end = namequants_end + strlen(namequants_end);
          actuals_end = quantactuals_end + strlen(quantactuals_end);
          generate_rules_aux(quantindex + 1, names_end, actuals_end,
                             therule);
        }
      }
      // Scalarset
      else if (quant->gettype()->gettypeclass() == typedecl::Scalarset) {
        s = ((scalarsettypedecl *) quant->gettype())->getidvalues();
        for (i = quant->gettype()->getleft();
             i <= quant->gettype()->getright(); i++) {
          sprintf(namequants_end, ", %s:%s", quant->name,
                  s->getname()->getname());
          sprintf(quantactuals_end, ", %s",
                  s->getvalue()->generate_code());
          s = s->getnext();
          names_end = namequants_end + strlen(namequants_end);
          actuals_end = quantactuals_end + strlen(quantactuals_end);
          generate_rules_aux(quantindex + 1, names_end, actuals_end,
                             therule);
        }
      }
      // Union
      else if (quant->gettype()->gettypeclass() == typedecl::Union) {
        t = ((uniontypedecl *) quant->gettype())->getunionmembers();
        for (; t != NULL; t = t->next) {
          d = (typedecl *) t->s->getvalue();
          if (d->gettypeclass() == typedecl::Scalarset)
            s = ((scalarsettypedecl *) d)->getidvalues();
          else if (d->gettypeclass() == typedecl::Enum)
            s = ((enumtypedecl *) d)->getidvalues();
          else
            Error.Error("Funny element in union");

          for (i = d->getleft(); i <= d->getright(); i++) {
            sprintf(namequants_end, ", %s:%s", quant->name,
                    s->getname()->getname());
            sprintf(quantactuals_end, ", %s",
                    s->getvalue()->generate_code());
            s = s->getnext();
            names_end = namequants_end + strlen(namequants_end);
            actuals_end = quantactuals_end + strlen(quantactuals_end);
            generate_rules_aux(quantindex + 1, names_end, actuals_end,
                               therule);
          }
        }
      }
      // subrange
      else {
        for (i = quant->gettype()->getleft();
             i <= quant->gettype()->getright(); i++) {
          sprintf(namequants_end, ", %s:%d", quant->name, i);
          sprintf(quantactuals_end, ", %d", i);
          names_end = namequants_end + strlen(namequants_end);
          actuals_end = quantactuals_end + strlen(quantactuals_end);
          generate_rules_aux(quantindex + 1, names_end, actuals_end,
                             therule);
        }
      }

    }
  }
}



/********************
  generate_rules
  -- creates the stub procedures for a rule/startstate/invariant/fairness
  -- and enrolls them in the list of rules.
  --
  -- i.e. create instances of rule/startstate/invariant
  -- by calling the main condition function and rule function
  -- corresponding to the rules
 ********************/
static void generate_rules(ste * enclosures, simplerule * therule)
{
  int i = 0;

  // set up quants[] array of quantdecl
  enroll_quants(enclosures);

  // initialize namequants[] and quantactuals[] to all \'\0\'
  for (i = 0; namequants[i] != '\0' && i < BUFFER_SIZE; i++)
    namequants[i] = '\0';
  for (i = 0; quantactuals[i] != '\0' && i < BUFFER_SIZE; i++)
    quantactuals[i] = '\0';

  // generate rules by calling to main rule with quantifier value
  generate_rules_aux(0, namequants, quantactuals, therule);

  fprintf(codefile, "/**** end rule declaration ****/\n\n");
}

/********************
  code for simplerule
  -- rules produce some code,
  -- but also update the rules/startst/invariant list.
 ********************/
const char *rule::generate_code()
{
  return "ERROR!";
}

/********************
  code for simplerule
 ********************/
const char *simplerule::generate_code()
{
  fprintf(codefile,
          "/******************** RuleBase%lu ********************/\n"
          "class RuleBase%lu\n" "{\n" "public:\n", rulenumber, rulenumber);

  // priority function, added by Uli
  fprintf(codefile,
          "  int Priority()\n" "  {\n" "    return %d;\n" "  }\n",
          priority);

  // generate Name(r)
  fprintf(codefile, "  char * Name(RULE_INDEX_TYPE r)\n" "  {\n");
  generate_rule_params_assignment(enclosures);
  fprintf(codefile, "    return tsprintf(\"%s", name);
  generate_rule_params_printf(enclosures);
  fprintf(codefile, "\"");
  generate_rule_params_name(enclosures);
  fprintf(codefile, ");\n" "  }\n");

  // generate Condition(r)
  fprintf(codefile, "  bool Condition(RULE_INDEX_TYPE r)\n" "  {\n");
  generate_rule_params_assignment(enclosures);
  generate_rule_params_choose(enclosures);
  generate_rule_aliases(enclosures);

  fprintf(codefile, "    return %s;\n", condition->generate_code());
  fprintf(codefile, "  }\n" "\n");








  /*
     * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
     *
     * returns grounded boolean preconditions!!!
     *
     */

if (ff_enabled == true){
    // generate array of numeric preconditions(r)
    fprintf(codefile, "  std::vector<mu_0_boolean*> bool_precond_array(RULE_INDEX_TYPE r)\n" "  {\n"
  		  ""
  		  "    std::vector<mu_0_boolean*> preconds;\n");
    generate_rule_params_assignment(enclosures);  // RESPONSIBLE FOR CREATING THE RIGHT MU_BLOCKS FOR THE PRECONDITION!
    generate_rule_params_choose(enclosures);
    generate_rule_aliases(enclosures);

    fprintf(codefile, "\n");

    std::set<std::string> precondset;

    expr* lc = condition;

//    std::cout << "\n-----------------------------------------------------------\n\nBOOLEAN PRECONDITIONS: \n\n" << std::endl;


    while (lc != NULL) {

    			std::string rrr(lc->generate_code_right());
    			std::string lll(lc->generate_code_left());
    			std::string all(lc->generate_code());

//    			std::cout << "ALL : " << all << std::endl;
//    			std::cout << "LEFT : " << lll << std::endl;
//    			std::cout << "RIGHT : " << rrr << std::endl;

//    			std::cout << "\nSHAMALAMA DING DONG\n" << std::endl;
    			std::string lstype;

    			if (lc->gettype()->name != NULL){
    				lstype = lc->gettype()->name;
//    				std::cout << "TYPE: " << lstype << std::endl;
    			}
    			else {
//    				std::cout << "\nSHAMALAMA DING DONG TYPE IS NULL\n" << std::endl;
    				lc = lc->get_left();
    				break;
    			}
//    			std::cout << "\nSHAMALAMA DING DONG\n" << std::endl;

//    			std::cout << "TYPE: " << lstype << std::endl;


    			if (lstype.compare("boolean") == 0 && all.find("mu__boolexpr") == std::string::npos && all.find("!") == std::string::npos ){


    				if (all.find("<") == std::string::npos && all.find(">") == std::string::npos &&
    						all.find("mu_true") == std::string::npos && all.find("==") == std::string::npos &&
    						all.find("+") == std::string::npos && all.find("-") == std::string::npos) {

//    					std::cout << "--- top inserting boolean: " << all << std::endl;

    					precondset.insert(all);
    					break;
    				}
//    				std::cout << "--- top NOT inserting boolean: " << all << std::endl;
    			}
    			else if (lstype.compare("boolean") == 0 && all.find("mu__boolexpr") != std::string::npos && all.find("!") == std::string::npos) {

    				lstype = lc->get_right()->gettype()->name;

    				if (lstype.compare("boolean") == 0 && rrr.find("mu__boolexpr") == std::string::npos && rrr.find("!") == std::string::npos) {

    					if(rrr.find("<") == std::string::npos && rrr.find(">") == std::string::npos &&
    							rrr.find("==") == std::string::npos && all.find("mu_true") == std::string::npos &&
        						all.find("+") == std::string::npos && all.find("-") == std::string::npos) {

//    						std::cout << "--- right inserting boolean expression: " << rrr << std::endl;

//    						std::cout << "RIGHT LEFT: " << (lc->get_right()->generate_code_left()) << std::endl;
//    						std::cout << "RIGHT RIGHT: " << (lc->get_right()->generate_code_right()) << std::endl;

    						precondset.insert(lc->get_right()->generate_code());
    					}
    				}

    				else {
//    					std::cout << "--- right NOT inserting boolean! : " << rrr << std::endl;
    				}
    			}

    			else {
//    				std::cout << "--- NOT inserting anything boolean: " << all << std::endl;
    			}

    			lc = lc->get_left();
//    			std::cout << "============================\n\n" << std::endl;
    		}


        fprintf(codefile, "\n");



    std::set<std::string>::iterator it;
    for (it=precondset.begin(); it!=precondset.end(); ++it){

  		  fprintf(codefile, " 		if (std::string(typeid(%s).name()).compare(\"12mu_0_boolean\") == 0)\n"
  				  	  	  	"			preconds.push_back(&(%s)); \n", it->c_str(), it->c_str() );
    }


    fprintf(codefile, "\n    return preconds;\n");
    fprintf(codefile, "  }\n" "\n");






  /*
   * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
   *
   * returns grounded numeric preconditions as a map in the format <variable*, <condition value, condition operator index> >
   *
   *
   */

  // generate array of numeric preconditions(r)
  fprintf(codefile, "  std::map<mu__real*, std::pair<double, int> > num_precond_array(RULE_INDEX_TYPE r)\n" "  {\n"
		  ""
		  "    std::map<mu__real*, std::pair<double, int> > preconds;\n");
  generate_rule_params_assignment(enclosures);  // RESPONSIBLE FOR CREATING THE RIGHT MU_BLOCKS FOR THE PRECONDITION!
  generate_rule_params_choose(enclosures);
  generate_rule_aliases(enclosures);

  fprintf(codefile, "\n");

  std::map<std::string, std::pair<std::string, int> > precondmap;

  expr* lc2 = condition;


//  std::cout << "\n-----------------------------------------------------------\n\nNUMERIC PRECONDITIONS: \n\n" << std::endl;


  while (lc2 != NULL) {

  			std::string rrr(lc2->generate_code_right());
  			std::string lll(lc2->generate_code_left());
  			std::string all(lc2->generate_code());

//  			std::cout << "ALL : " << all << std::endl;
//  			std::cout << "LEFT : " << lll << std::endl;
//  			std::cout << "RIGHT : " << rrr << std::endl;

  			std::string lstype(lc2->gettype()->name);

//  			std::cout << "\n\nNUMERIC SHAMALAMA DING DONG\n\n" << std::endl;

//  			std::cout << "TYPE: " << lstype << std::endl;


  			if (lstype.compare("boolean") == 0 && all.find("mu__boolexpr") == std::string::npos && all.find("!") == std::string::npos){

  				if (all.find("==") != std::string::npos) {

//  					std::cout << "--- top inserting EQ COMPARISON boolean expression: " << all << std::endl;

  					precondmap.insert(std::make_pair(lll, std::make_pair(rrr,0)));
  					break;
  				}
  				else if (all.find("<=") != std::string::npos) {

//  					std::cout << "--- top inserting LTE COMPARISON boolean expression: " << all << std::endl;

  					precondmap.insert(std::make_pair(lll, std::make_pair(rrr, 2)));
  					break;
  				}
  				else if (all.find(">=") != std::string::npos) {

//  					std::cout << "--- top inserting GTE COMPARISON boolean expression: " << all << std::endl;

  					precondmap.insert(std::make_pair(lll, std::make_pair(rrr, 1)));
  					break;
  				}
  				else if (all.find("<") != std::string::npos) {

//  					std::cout << "--- top inserting LT COMPARISON boolean expression: " << all << std::endl;

  					precondmap.insert(std::make_pair(lll, std::make_pair(rrr, 4)));
  					break;
  				}
  				else if (all.find(">") != std::string::npos) {

//  					std::cout << "--- top inserting GT COMPARISON boolean expression: " << all << std::endl;

  					precondmap.insert(std::make_pair(lll, std::make_pair(rrr, 3)));
  					break;
  				}

  			}
  			else if (lstype.compare("boolean") == 0 && all.find("mu__boolexpr") != std::string::npos && all.find("!") == std::string::npos) {

  				lstype = lc2->get_right()->gettype()->name;

  				if (lstype.compare("boolean") == 0 && rrr.find("mu__boolexpr") == std::string::npos && rrr.find("!") == std::string::npos) {


  	  				if (rrr.find("==") != std::string::npos) {

//  	  					std::cout << "--- right inserting EQ COMPARISON boolean expression: " << rrr << std::endl;

  	  					precondmap.insert(std::make_pair(lc2->get_right()->generate_code_left(), std::make_pair(lc2->get_right()->generate_code_right(),0)));
  	  				}
  					else if (rrr.find("<=") != std::string::npos) {

//  	  					std::cout << "--- right inserting LTE COMPARISON boolean expression: " << rrr << std::endl;

  	  					precondmap.insert(std::make_pair(lc2->get_right()->generate_code_left(), std::make_pair(lc2->get_right()->generate_code_right(),2)));
  	  				}
  	  				else if (rrr.find(">=") != std::string::npos) {

//  	  					std::cout << "--- right inserting GTE COMPARISON boolean expression: " << rrr << std::endl;

  	  					precondmap.insert(std::make_pair(lc2->get_right()->generate_code_left(), std::make_pair(lc2->get_right()->generate_code_right(),1)));
  	  				}
  	  				else if (rrr.find("<") != std::string::npos) {

//  	  					std::cout << "--- right inserting LT COMPARISON boolean expression: " << rrr << std::endl;

  	  					precondmap.insert(std::make_pair(lc2->get_right()->generate_code_left(), std::make_pair(lc2->get_right()->generate_code_right(),4)));
  	  				}
  	  				else if (rrr.find(">") != std::string::npos) {

//  	  					std::cout << "--- right inserting GT COMPARISON boolean expression: " << rrr << std::endl;

  	  					precondmap.insert(std::make_pair(lc2->get_right()->generate_code_left(), std::make_pair(lc2->get_right()->generate_code_right(),3)));
  	  				}

  				}

  				else {
//  					std::cout << "--- right NOT inserting! : " << rrr << std::endl;
  				}
  			}

  			else {
//  				std::cout << "--- NOT inserting anything: " << all << std::endl;
  			}

  			lc2 = lc2->get_left();
//  			std::cout << "============================\n\n" << std::endl;
  		}


      fprintf(codefile, "\n");



  std::map<std::string, std::pair<std::string, int> >::iterator it33;
  for (it33=precondmap.begin(); it33!=precondmap.end(); ++it33){

		  fprintf(codefile, " 	if (std::string(typeid(%s).name()).compare(\"14mu_1_real_type\") == 0)\n"
				  	  	  	"			preconds.insert(std::make_pair(&(%s), std::make_pair(%s, %d))); \n", it33->first.c_str(), it33->first.c_str(), it33->second.first.c_str(), it33->second.second  );
  }


  fprintf(codefile, "\n    return preconds;\n");
  fprintf(codefile, "  }\n" "\n");






  /*
   * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
   *
   * returns all precondition variables of an action!!! (returns the fact pointers (not whole expressions) as mu__any* )
   *
   * TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
   */

  // generate array of boolean preconditions(r)
  fprintf(codefile, "  std::vector<mu__any*> all_precond_array(RULE_INDEX_TYPE r)\n" "  {\n"
		  ""
		  "    std::vector<mu__any*> preconds;\n");
  generate_rule_params_assignment(enclosures);  // RESPONSIBLE FOR CREATING THE RIGHT MU_BLOCKS FOR THE PRECONDITION!
  generate_rule_params_choose(enclosures);
  generate_rule_aliases(enclosures);


  std::map<std::string, std::pair<std::string, int> >::iterator it66;
  for (it66=precondmap.begin(); it66!=precondmap.end(); ++it66){

		  fprintf(codefile, " 	if (std::string(typeid(%s).name()).compare(\"14mu_1_real_type\") == 0)\n"
				  	  	  	"			preconds.push_back(&(%s)); \n", it66->first.c_str(), it66->first.c_str());
  }


  std::set<std::string>::iterator it99;
  for (it99=precondset.begin(); it99!=precondset.end(); ++it99){

		  fprintf(codefile, " 		if (std::string(typeid(%s).name()).compare(\"12mu_0_boolean\") == 0)\n"
				  	  	  	"			preconds.push_back(&(%s)); \n", it99->c_str(), it99->c_str());
  }






  fprintf(codefile, "\n    return preconds;\n");
  fprintf(codefile, "  }\n" "\n");

}









/*
   * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
   *
   * returns grounded boolean preconditions to check for interference set of mu_bool* and 0/1 (false/true)
   *
   */

  // generate set of bool preconditions(r)
  fprintf(codefile, "  std::set<std::pair<mu_0_boolean*, int> > precond_bool_interference(RULE_INDEX_TYPE r)\n" "  {\n"
		  ""
		  "    std::set<std::pair<mu_0_boolean*, int> > interference_preconds;\n");
  generate_rule_params_assignment(enclosures);  // RESPONSIBLE FOR CREATING THE RIGHT MU_BLOCKS FOR THE PRECONDITION!
  generate_rule_params_choose(enclosures);
  generate_rule_aliases(enclosures);

  fprintf(codefile, "\n");

  std::set<std::string> interf_precondset;

  expr* lc = condition;

//  std::cout << "\n-----------------------------------------------------------\n\nBOOLEAN INTERFERENCE PRECONDITIONS: \n\n" << std::endl;


  while (lc != NULL) {

  			std::string rrr(lc->generate_code_right());
  			std::string lll(lc->generate_code_left());
  			std::string all(lc->generate_code());

//  			std::cout << "ALL : " << all << std::endl;
//  			std::cout << "LEFT : " << lll << std::endl;
//  			std::cout << "RIGHT : " << rrr << std::endl;

//  			std::cout << "\nSHAMALAMA DING DONG\n" << std::endl;
  			std::string lstype;

  			if (lc->gettype()->name != NULL){
  				lstype = lc->gettype()->name;
//  				std::cout << "TYPE: " << lstype << std::endl;
  			}
  			else {
//  				std::cout << "\nSHAMALAMA DING DONG TYPE IS NULL\n" << std::endl;
  				lc = lc->get_left();
  				break;
  			}
//  			std::cout << "\nSHAMALAMA DING DONG\n" << std::endl;

//  			std::cout << "TYPE: " << lstype << std::endl;


  			if (lstype.compare("boolean") == 0 && all.find("mu__boolexpr") == std::string::npos /*&& all.find("!") == std::string::npos */){


  				if (all.find("<") == std::string::npos && all.find(">") == std::string::npos &&
  						all.find("mu_true") == std::string::npos && all.find("==") == std::string::npos &&
  						all.find("+") == std::string::npos && all.find("-") == std::string::npos) {

//  					std::cout << "--- top inserting boolean: " << all << std::endl;

  					interf_precondset.insert(all);
  					break;
  				}
//  				std::cout << "--- top NOT inserting boolean: " << all << std::endl;
  			}
  			else if (lstype.compare("boolean") == 0 && all.find("mu__boolexpr") != std::string::npos /*&& all.find("!") == std::string::npos*/) {

  				lstype = lc->get_right()->gettype()->name;

  				if (lstype.compare("boolean") == 0 && rrr.find("mu__boolexpr") == std::string::npos /*&& rrr.find("!") == std::string::npos*/) {

  					if(rrr.find("<") == std::string::npos && rrr.find(">") == std::string::npos &&
  							rrr.find("==") == std::string::npos && all.find("mu_true") == std::string::npos &&
      						all.find("+") == std::string::npos && all.find("-") == std::string::npos) {

//  						std::cout << "--- right inserting boolean expression: " << rrr << std::endl;

//  						std::cout << "RIGHT LEFT: " << (lc->get_right()->generate_code_left()) << std::endl;
//  						std::cout << "RIGHT RIGHT: " << (lc->get_right()->generate_code_right()) << std::endl;

  						interf_precondset.insert(lc->get_right()->generate_code());
  					}
  				}

  				else {
//  					std::cout << "--- right NOT inserting boolean! : " << rrr << std::endl;
  				}
  			}

  			else {
//  				std::cout << "--- NOT inserting anything boolean: " << all << std::endl;
  			}

  			lc = lc->get_left();
//  			std::cout << "============================\n\n" << std::endl;
  		}


      fprintf(codefile, "\n");



  std::set<std::string>::iterator it;
  for (it=interf_precondset.begin(); it!=interf_precondset.end(); ++it){

	  if (it->find("!") == std::string::npos){

		  fprintf(codefile, " 		if (std::string(typeid(%s).name()).compare(\"12mu_0_boolean\") == 0)\n"
				  	  	  	"			interference_preconds.insert(std::make_pair(&(%s), 1)); \n", it->c_str(), it->c_str() );
	  }
	  else {
//		  int len_it = it->length()-3;
//		  std::cout << "FULL LEN: " << it->length() << " != LEN-1: " << len_it << std::endl;
		  std::string sub_inf = it->substr(2,it->length()-3);
//		  std::cout << "SUB_INF: " << sub_inf << std::endl;
		  fprintf(codefile, " 		if (std::string(typeid(%s).name()).compare(\"12mu_0_boolean\") == 0)\n"
		  				  	  	  	"			interference_preconds.insert(std::make_pair(&(%s), 0)); \n", sub_inf.c_str(), sub_inf.c_str() );
	  }
  }


  fprintf(codefile, "\n    return interference_preconds;\n");
  fprintf(codefile, "  }\n" "\n");
















    /*
       * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
       *
       * returns temporal preconditions!!!
       *
       */

      if (ff_enabled == true){
      // generate array of numeric preconditions(r)
      fprintf(codefile, "  std::pair<double, double> temporal_constraints(RULE_INDEX_TYPE r)\n" "  {\n"
    		  ""
    		  "    std::pair<double, double> temporal_cons;\n");
      generate_rule_params_assignment(enclosures);  // RESPONSIBLE FOR CREATING THE RIGHT MU_BLOCKS FOR THE PRECONDITION!
      generate_rule_params_choose(enclosures);
      generate_rule_aliases(enclosures);

      fprintf(codefile, "\n");

      std::map<std::string, std::pair<std::string, std::string> > cons;

      expr* lc3 = condition;

//      std::cout << "\n-----------------------------------------------------------\n\nTEMPORAL CONSTRAINTS: \n\n" << std::endl;


      while (lc3 != NULL) {

        			std::string rrr(lc3->generate_code_right());
        			std::string lll(lc3->generate_code_left());
        			std::string all(lc3->generate_code());

//        			std::cout << "ALL : " << all << std::endl;
//        			std::cout << "LEFT : " << lll << std::endl;
//        			std::cout << "RIGHT : " << rrr << std::endl;

        			std::string lstype(lc3->gettype()->name);

//        			std::cout << "\n\nTEMPORAL SHAMALAMA DING DONG\n\n" << std::endl;

//        			std::cout << "TYPE: " << lstype << std::endl;


        			if (lstype.compare("boolean") == 0 && all.find("mu__boolexpr") == std::string::npos && all.find("!") == std::string::npos
        					&& all.find("mu_true") == std::string::npos && all.find("mu_false") == std::string::npos && all.find("_clock") != std::string::npos){

        				if (all.find("==") != std::string::npos) {

//        					std::cout << "--- top inserting EQ COMPARISON boolean expression: " << all << std::endl;

        					cons.insert(std::make_pair(lll, std::make_pair(rrr,rrr)));
        					break;
        				}
        				else if (all.find("<=") != std::string::npos) {

//        					std::cout << "--- top inserting LTE COMPARISON boolean expression: " << all << std::endl;

        					cons.insert(std::make_pair(lll, std::make_pair("mu_T", rrr)));
        					break;
        				}
        				else if (all.find(">=") != std::string::npos) {

//        					std::cout << "--- top inserting GTE COMPARISON boolean expression: " << all << std::endl;

        					cons.insert(std::make_pair(lll, std::make_pair(rrr, "TIME_INFINITY")));
        					break;
        				}
        				else if (all.find("<") != std::string::npos) {

//        					std::cout << "--- top inserting LT COMPARISON boolean expression: " << all << std::endl;

        					cons.insert(std::make_pair(lll, std::make_pair("mu_T", (rrr + " - mu_T"))));
        					break;
        				}
        				else if (all.find(">") != std::string::npos) {

//        					std::cout << "--- top inserting GT COMPARISON boolean expression: " << all << std::endl;

        					cons.insert(std::make_pair(lll, std::make_pair((rrr + " + mu_T"), "TIME_INFINITY")));
        					break;
        				}

        			}
        			else if (lstype.compare("boolean") == 0 && all.find("mu__boolexpr") != std::string::npos && all.find("!") == std::string::npos) {

        				lstype = lc3->get_right()->gettype()->name;

        				if (lstype.compare("boolean") == 0 && rrr.find("mu__boolexpr") == std::string::npos && rrr.find("!") == std::string::npos
        						&& rrr.find("mu_true") == std::string::npos && rrr.find("mu_false") == std::string::npos && rrr.find("_clock") != std::string::npos) {


        	  				if (rrr.find("==") != std::string::npos) {

//        	  					std::cout << "--- right inserting EQ COMPARISON boolean expression: " << rrr << std::endl;

        	  					cons.insert(std::make_pair(lc3->get_right()->generate_code_left(), std::make_pair(lc3->get_right()->generate_code_right(),lc3->get_right()->generate_code_right())));
        	  				}
        					else if (rrr.find("<=") != std::string::npos) {

//        	  					std::cout << "--- right inserting LTE COMPARISON boolean expression: " << rrr << std::endl;

        	  					cons.insert(std::make_pair(lc3->get_right()->generate_code_left(), std::make_pair("mu_T", lc3->get_right()->generate_code_right())));
        	  				}
        	  				else if (rrr.find(">=") != std::string::npos) {

//        	  					std::cout << "--- right inserting GTE COMPARISON boolean expression: " << rrr << std::endl;

        	  					cons.insert(std::make_pair(lc3->get_right()->generate_code_left(), std::make_pair(lc3->get_right()->generate_code_right(), "TIME_INFINITY")));
        	  				}
        	  				else if (rrr.find("<") != std::string::npos) {

//        	  					std::cout << "--- right inserting LT COMPARISON boolean expression: " << rrr << std::endl;

        	  					cons.insert(std::make_pair(lc3->get_right()->generate_code_left(), std::make_pair("mu_T", (lc3->get_right()->generate_code_right() + std::string(" - mu_T")))));
        	  				}
        	  				else if (rrr.find(">") != std::string::npos) {

//        	  					std::cout << "--- right inserting GT COMPARISON boolean expression: " << rrr << std::endl;

        	  					cons.insert(std::make_pair(lc3->get_right()->generate_code_left(), std::make_pair((lc3->get_right()->generate_code_right() + std::string(" + mu_T")), "TIME_INFINITY")));
        	  				}

        				}

        				else {
//        					std::cout << "--- right NOT inserting! : " << rrr << std::endl;
        				}
        			}

        			else {
//        				std::cout << "--- NOT inserting anything: " << all << std::endl;
        			}

        			lc3 = lc3->get_left();
//        			std::cout << "============================\n\n" << std::endl;
        		}


            fprintf(codefile, "\n");



        std::map<std::string, std::pair<std::string, std::string> >::iterator it44;
        for (it44=cons.begin(); it44!=cons.end(); ++it44){

      		  fprintf(codefile, " 	if (std::string(typeid(%s).name()).compare(\"14mu_1_real_type\") == 0)\n"
      				  	  	  	"			temporal_cons = std::make_pair(%s, %s); \n",
      				  	  	  	it44->first.c_str(), it44->second.first.c_str(), it44->second.second.c_str()  );
        }


        fprintf(codefile, "\n    return temporal_cons;\n");
        fprintf(codefile, "  }\n" "\n");


   }







  /*
   * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
   *
   * returns grounded numeric effects of an action!!! (returns the fact pointers (not whole expressions) )
   *
   * TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
   */

  if (ff_enabled == true){

  // generate array of effects(r)
  fprintf(codefile, "  std::vector<mu__real*> effects_num_array(RULE_INDEX_TYPE r)\n" "  {\n"
		  ""
		  "    std::vector<mu__real*> effs;\n");

  fprintf(codefile, "\n");

  generate_rule_params_assignment(enclosures);
  generate_rule_aliases(enclosures);
  locals->generate_decls();

  fprintf(codefile, "\n");

  // gets the m_0_boolean* grounded fact
      for (stmt * b = body; b != NULL; b = b->next) {
    	  std::string src_str(b->get_src());
    	  if (b->get_target() != "ERROR" && b->get_src() != "ERROR" && (src_str.compare("mu_true")!=0 && src_str.compare("mu_false")!=0)){
    		  fprintf(codefile, "    effs.push_back(&(%s));  // %s \n", b->get_target(), b->get_src());
    	  }
      }

//      for (stmt * b = body; b != NULL; b = b->next) b->generate_code();

  fprintf(codefile, "\n    return effs;\n");
  fprintf(codefile, "  }\n" "\n");

  }







  /*
   * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
   *
   * returns grounded boolean add effects of an action!!!
   *
   * TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
   */

  if (ff_enabled == true){

  // generate array of add effects(r)
  fprintf(codefile, "  std::vector<mu_0_boolean*> effects_add_bool_array(RULE_INDEX_TYPE r)\n" "  {\n"
		  ""
		  "    std::vector<mu_0_boolean*> aeffs;\n");

  fprintf(codefile, "\n");

  generate_rule_params_assignment(enclosures);
  generate_rule_aliases(enclosures);
  locals->generate_decls();

  fprintf(codefile, "\n");

  // gets the m_0_boolean* grounded fact
      for (stmt * b = body; b != NULL; b = b->next) {
    	  std::string src_str(b->get_src());
    	  if ((b->get_target() != "ERROR") && (b->get_src() != "ERROR") && (src_str.compare("mu_true")==0)){
    		  fprintf(codefile, "    aeffs.push_back(&(%s)); //  %s \n", b->get_target(), b->get_src());
    	  }
      }

//      for (stmt * b = body; b != NULL; b = b->next) b->generate_code();

  fprintf(codefile, "\n    return aeffs;\n");
  fprintf(codefile, "  }\n" "\n");

  }


  /*
    * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
    *
    * returns a set of grounded boolean action effects with their values (0==false, 1==true)
    *
    * TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
    */

   if (ff_enabled == true){

   // generate set of bool effects(r)
   fprintf(codefile, "  std::set<std::pair<mu_0_boolean*, int> > effects_bool_interference(RULE_INDEX_TYPE r)\n" "  {\n"
 		  ""
 		  "    std::set<std::pair<mu_0_boolean*,int> > inter_effs;\n");

   fprintf(codefile, "\n");

   generate_rule_params_assignment(enclosures);
   generate_rule_aliases(enclosures);
   locals->generate_decls();

   fprintf(codefile, "\n");

   // gets the m_0_boolean* grounded fact
       for (stmt * b = body; b != NULL; b = b->next) {
     	  std::string src_str(b->get_src());
     	  if ((b->get_target() != "ERROR") && (b->get_src() != "ERROR") && (src_str.compare("mu_true")==0)){
     		  fprintf(codefile, "    inter_effs.insert(std::make_pair(&(%s), 1)); //  %s \n", b->get_target(), b->get_src());
     	  }
     	  else if ((b->get_target() != "ERROR") && (b->get_src() != "ERROR") && (src_str.compare("mu_false")==0)){
     		  fprintf(codefile, "    inter_effs.insert(std::make_pair(&(%s), 0)); //  %s \n", b->get_target(), b->get_src());
     	  }
       }

 //      for (stmt * b = body; b != NULL; b = b->next) b->generate_code();

   fprintf(codefile, "\n    return inter_effs;\n");
   fprintf(codefile, "  }\n" "\n");

   }




/*
 *  WP WP WP WP WP WP WP WP
 *
 *  generate array of mu_any* effect variables(r) (add booleans and all numeric)
 *
 */
  if (ff_enabled == true){

  fprintf(codefile, "  std::vector<mu__any*> effects_all_array(RULE_INDEX_TYPE r)\n" "  {\n"
		  ""
		  "    std::vector<mu__any*> aeffs;\n");
  generate_rule_params_assignment(enclosures);
  generate_rule_params_choose(enclosures);
  generate_rule_aliases(enclosures);



  // gets the m_0_boolean* grounded fact
        for (stmt * b = body; b != NULL; b = b->next) {
      	  std::string src_str(b->get_src());
      	  if ((b->get_target() != "ERROR") && (b->get_src() != "ERROR") && (src_str.compare("mu_false")!=0)){
      		  fprintf(codefile, "    aeffs.push_back(&(%s)); //  %s \n", b->get_target(), b->get_src());
      	  }
        }




//  fprintf(codefile, "\n"
//		  ""
//		  "		std::vector<mu_0_boolean*> bls = effects_add_bool_array(r);\n"
//		  "		std::vector<mu_1_real_type*> nms = effects_num_array(r);\n"
//		  "");
//
//  fprintf(codefile, " 	for( int i = 0; i < bls.size(); i++){\n"
//		  	  	  	"		\n"
//		  	  	  	"		aeffs.push_back( bls.at(i)); \n"
//		  	  	  	" 	}\n\n" );
//
//  fprintf(codefile, " 	for( int i = 0; i < nms.size(); i++){\n"
//		  	  	  	"		\n"
//		  	  	  	"		aeffs.push_back( nms.at(i)); \n"
//		  	  	  	" 	}\n\n" );



  fprintf(codefile, "\n    return aeffs;\n");
  fprintf(codefile, "  }\n" "\n");

  }







  // generate NextRule(r)
  fprintf(codefile,
          "  void NextRule(RULE_INDEX_TYPE & what_rule)\n"
          "  {\n"
          "    RULE_INDEX_TYPE r = what_rule - %lu;\n", maxrulenumber - getsize()
         );
  generate_rule_params_assignment(enclosures);
  fprintf(codefile, "    while (what_rule < %lu ", maxrulenumber);
  generate_rule_params_choose_exist(enclosures);
  fprintf(codefile, ")\n      {\n");

  // Vitaly's hacks

  // Check if ``dependent'' choose parameters are there
  fprintf(codefile, "	if ( ( TRUE ");
  generate_rule_params_choose_all_in(dep_choose);
  fprintf(codefile, " ) ) {\n");

  // Check condition
  generate_rule_aliases(enclosures);
  fprintf(codefile, "	      if (%s) {\n", condition->generate_code());

  // Check is ``independent'' choose parameters are there
  fprintf(codefile, "		if ( ( TRUE ");
  generate_rule_params_choose_all_in(indep_choose);
  fprintf(codefile, " ) )\n");
  fprintf(codefile, "		  return;\n");
  fprintf(codefile, "		else\n");
  fprintf(codefile, "		  what_rule++;\n");
  fprintf(codefile, "	      }\n");

  fprintf(codefile, "	      else\n");
  fprintf(codefile, "		what_rule += %d;\n", indep_card);
  fprintf(codefile, "	}\n");

  fprintf(codefile, "	else\n");
  fprintf(codefile, "	  what_rule += %d;\n", indep_card);

  // End of Vitaly's hacks


  fprintf(codefile, "    r = what_rule - %lu;\n",
          maxrulenumber - getsize());

  generate_rule_params_simple_assignment(enclosures);
  fprintf(codefile, "    }\n" "  }\n\n");

  // generate Code(r)
  fprintf(codefile, "  void Code(RULE_INDEX_TYPE r)\n" "  {\n");
  generate_rule_params_assignment(enclosures);
  generate_rule_aliases(enclosures);
  locals->generate_decls();

  /*
    //if we have a clock procedure, call it before the rule code and as many times as rule duration, so...
    if (duration!=NULL && theprog->clock_procedure != NULL) {
  	fprintf(codefile, " for(int clock_ticks=1; clock_ticks<=Duration(r); ++clock_ticks) %s();\n",theprog->clock_procedure->getvalue()->mu_name);
    }
  */
  for (stmt * b = body; b != NULL; b = b->next) b->generate_code();



  fprintf(codefile, "  };\n" "\n");




/*
   * WP WP WP WP WP WP WP WP WP WP WP WP
   *
   * executes only the add effects of an action (FF)
   *
   * TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
   */

  if (ff_enabled == true){

  // generate execute code responsible for add effects
    fprintf(codefile, "  void Code_ff(RULE_INDEX_TYPE r)\n" "  {\n\n");
    fprintf(codefile, "\n");

    generate_rule_params_assignment(enclosures);
    generate_rule_aliases(enclosures);
    locals->generate_decls();

    fprintf(codefile, "\n");

    // gets the m_0_boolean* grounded fact
    for (stmt * b = body; b != NULL; b = b->next) {
  	  std::string src_str(b->get_src());
  	  if ((b->get_target() != "ERROR") && (b->get_src() != "ERROR") && src_str.compare("mu_true") == 0){
//  		  fprintf(codefile, "    %s = %s; \n", b->get_target(), b->get_src());
  		  	  b->generate_code();
  	  }
    }

    fprintf(codefile, "\n\n  }\n" "\n");

  }



  /*
   * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
   *
   * executes only the additive numeric effects of an action (metricFF)
   *
   * TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
   */

  if (ff_enabled == true){

  // generate execute code responsible for add effects
    fprintf(codefile, "  void Code_numeric_ff_plus(RULE_INDEX_TYPE r)\n" "  {\n\n");
    fprintf(codefile, "\n");

    generate_rule_params_assignment(enclosures);
    generate_rule_aliases(enclosures);
    locals->generate_decls();

    fprintf(codefile, "\n");

    // gets the m_0_boolean* grounded fact
    for (stmt * b = body; b != NULL; b = b->next) {
  	  std::string src_str(b->get_src());
  	  std::string trg_str(b->get_target());

  	  if (src_str.find("mu_event_check") != std::string::npos || src_str.find("mu_apply_continuous_change") != std::string::npos ||
  			trg_str.find("mu_event_check") != std::string::npos || trg_str.find("mu_apply_continuous_change") != std::string::npos){
  		  b->generate_code();
  		  continue;
  	  }


  	  if ((b->get_target() != "ERROR") && (b->get_src() != "ERROR") && src_str.compare("mu_false")!= 0
  			  && (src_str.find("+") != std::string::npos || src_str.find("increase") != std::string::npos ||
  					  src_str.find("mu_true") != std::string::npos) ){

//  		  if (trg_str.find("_clock") != std::string::npos){
////  		  fprintf(codefile, "    %s = %s; \n", b->get_target(), b->get_src());
//  		  	  continue;
//  		  }

  		  b->generate_code();
  	  }
    }

    fprintf(codefile, "\n\n  }\n" "\n");

  }



  /*
   * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
   *
   * executes only the subtractive numeric effects of an action (metricFF)
   *
   * TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
   */

  if (ff_enabled == true){

  // generate execute code responsible for add effects
    fprintf(codefile, "  void Code_numeric_ff_minus(RULE_INDEX_TYPE r)\n" "  {\n\n");
    fprintf(codefile, "\n");

    generate_rule_params_assignment(enclosures);
    generate_rule_aliases(enclosures);
    locals->generate_decls();

    fprintf(codefile, "\n");

    // gets the m_0_boolean* grounded fact
    for (stmt * b = body; b != NULL; b = b->next) {
  	  std::string src_str(b->get_src());
  	  std::string trg_str(b->get_target());


  	  if (src_str.find("mu_event_check") != std::string::npos || src_str.find("mu_apply_continuous_change") != std::string::npos ||
  			trg_str.find("mu_event_check") != std::string::npos || trg_str.find("mu_apply_continuous_change") != std::string::npos){
  		  b->generate_code();
  		  continue;
  	  }


  	  if ((b->get_target() != "ERROR") && (b->get_src() != "ERROR") && src_str.compare("mu_false")!= 0
  			   && (src_str.find("-") != std::string::npos || src_str.find("decrease") != std::string::npos || src_str.find("mu_true") != std::string::npos)){
////  		  fprintf(codefile, "    %s = %s; \n", b->get_target(), b->get_src());

//  		  if (trg_str.find("_clock") != std::string::npos){
//  		  	  continue;
//  		  }

  		  	  b->generate_code();
  	  }
    }

    fprintf(codefile, "\n\n  }\n" "\n");

  }


  // WP WP WP WP WP WP WP WP
  if (ff_enabled == true){

  // generate execute code responsible for getting started clocks
    fprintf(codefile, "  mu_0_boolean* get_rule_clock_started(RULE_INDEX_TYPE r)\n" "  {\n\n");
    fprintf(codefile, "\n");

    generate_rule_params_assignment(enclosures);
    generate_rule_aliases(enclosures);
    locals->generate_decls();

    fprintf(codefile, "\n");

    // gets the m_0_boolean* grounded fact
    for (stmt * b = body; b != NULL; b = b->next) {
  	  std::string src_str(b->get_src());
  	  std::string trg_str(b->get_target());


	  if (trg_str.find("clock_started") != std::string::npos && src_str.find("mu_true") != std::string::npos){
			fprintf(codefile, "return (&(%s)); \n", b->get_target());
	  }

  	}

    fprintf(codefile, "\n\n  }\n" "\n");

  }


  // WP WP WP WP WP WP WP
  if (ff_enabled == true){

  // generate execute code responsible for getting all clocks
    fprintf(codefile, "std::map<mu_0_boolean*, mu__real*> get_clocks(RULE_INDEX_TYPE r)\n" "  {\n\n");
    fprintf(codefile, "\n std::map<mu_0_boolean*, mu__real*> pr; \n");

    std::string cl_st;
    std::string cl_d;

    generate_rule_params_assignment(enclosures);
    generate_rule_aliases(enclosures);
    locals->generate_decls();

    fprintf(codefile, "\n");

//    std::cout << "\n\nTEMPORAL PRECONDITIONS ====================================================" << std::endl;

    // gets the m_0_boolean* grounded fact
    for (stmt * b = body; b != NULL; b = b->next) {
  	  std::string src_str(b->get_src());
  	  std::string trg_str(b->get_target());


	  if (trg_str.find("clock_started") != std::string::npos && (src_str.find("mu_true") != std::string::npos || src_str.find("mu_false") != std::string::npos)){
			cl_st = b->get_target();
	  }
	  else if (trg_str.find("_clock") != std::string::npos && src_str.find("mu_true") == std::string::npos && src_str.find("mu_false") == std::string::npos){
			cl_d = b->get_target();
	  }

  	}

    if(cl_st.length() > 0 && cl_d.length() > 0){

//    	std::cout << cl_st << " : " << cl_d << "\n\n" <<  std::endl;

    	fprintf(codefile, "pr.insert(std::make_pair(&(%s), &(%s))); \n", cl_st.c_str(), cl_d.c_str());
    }

    	fprintf(codefile, "return pr; \n");
    	fprintf(codefile, "\n\n  }\n" "\n");


  } else {

	    fprintf(codefile, "std::map<mu_0_boolean*, mu__real*> get_clocks(RULE_INDEX_TYPE r)\n" "  {\n\n");
	    fprintf(codefile, "\n std::map<mu_0_boolean*, mu__real*> pr; \n");

    	fprintf(codefile, "return pr; \n");
    	fprintf(codefile, "\n\n  }\n" "\n");
  }





  // generate Duration(r)
  fprintf(codefile, "  int Duration(RULE_INDEX_TYPE r)\n" "  {\n");
  generate_rule_params_assignment(enclosures);
  generate_rule_params_choose(enclosures);
  generate_rule_aliases(enclosures);

  if (duration!=NULL) {
    fprintf(codefile, "    return %s;\n", duration->generate_code());
  } else {
    fprintf(codefile, "    return 0;\n");
  }
  fprintf(codefile, "  }\n" "\n");

  // generate Weight(r)
  fprintf(codefile, "  int Weight(RULE_INDEX_TYPE r)\n" "  {\n");
  generate_rule_params_assignment(enclosures);
  generate_rule_params_choose(enclosures);
  generate_rule_aliases(enclosures);

  if (weight!=NULL) {
    fprintf(codefile, "    return %s;\n", weight->generate_code());
  } else if (duration!=NULL) {
    fprintf(codefile, "    return Duration(r);\n");
  } else {
    fprintf(codefile, "    return 0;\n");
  }
  fprintf(codefile, "  }\n" "\n");

  // generate PDDLName(r)
  fprintf(codefile, "   char * PDDLName(RULE_INDEX_TYPE r)\n" "  {\n");
  generate_rule_params_assignment(enclosures);
//fprintf(codefile, "    return tsprintf(\"%s", get_pddlname());
  fprintf(codefile, "    return tsprintf(\"(%s", get_pddlname());
  generate_rule_params_printf_pddl(enclosures);
//fprintf(codefile, "\"");
  fprintf(codefile, ")\"");
  generate_rule_params_name_pddl(enclosures);
  fprintf(codefile, ");\n" "  }\n");


  // generate PDDLClass(r)
  fprintf(codefile, "   RuleManager::rule_pddlclass PDDLClass(RULE_INDEX_TYPE r)\n" "  {\n");
  switch(pddlclass) {
    case Event:
      fprintf(codefile, "    return RuleManager::Event;\n");
      break;
    case DurativeStart:
      fprintf(codefile, "    return RuleManager::DurativeStart;\n");
      break;
    case DurativeEnd:
      fprintf(codefile, "    return RuleManager::DurativeEnd;\n");
      break;
    case Clock:
      fprintf(codefile, "    return RuleManager::Clock;\n");
      break;
    case Action:
      fprintf(codefile, "    return RuleManager::Action;\n");
      break;
    default:
      fprintf(codefile, "    return RuleManager::Other;\n");
      break;
  }
  fprintf(codefile, "  };\n" "\n");


  // end declaration
  fprintf(codefile, "};\n");

  return "ERROR!";
}

/********************
  code for startstate
 ********************/
const char *startstate::generate_code()
{
  fprintf(codefile,
          "/******************** StartStateBase%lu ********************/\n"
          "class StartStateBase%lu\n"
          "{\n" "public:\n", rulenumber, rulenumber);

  // generate Name(r)
  fprintf(codefile, "  char * Name(unsigned short r)\n" "  {\n");
  generate_rule_params_assignment(enclosures);
  fprintf(codefile, "    return tsprintf(\"%s", name);
  generate_rule_params_printf(enclosures);
  fprintf(codefile, "\"");
  generate_rule_params_name(enclosures);
  fprintf(codefile, ");\n" "  }\n");

  // generate Code(r)
  fprintf(codefile, "  void Code(unsigned short r)\n" "  {\n");
  generate_rule_params_assignment(enclosures);
  generate_rule_aliases(enclosures);
  locals->generate_decls();
  for (stmt * b = body; b != NULL; b = b->next)
    b->generate_code();
  fprintf(codefile, "  };\n" "\n");

  // end declaration
  fprintf(codefile, "};\n");

  return "ERROR!";
}

//UPMURPHI_BEGIN
/********************
 code for Goal (similar to invariant construct)
 ********************/
const char *plangoal::generate_code ()
{
  int rule_int = new_int ();
  condname = tsprintf ("mu__goal_%d", rule_int);

  /* generate the parameters to the function for the goal. */
  /* e.g.  "int pid, int mid" as in "mu__goal_1(int pid, int mid)" */
  char *params = generate_rule_params (enclosures);

  /* generate the condition function. */
  /* which is called by the different instances of the invariant */
  fprintf (codefile, "int %s(%s) // Goal \"%s\"\n",
           condname, params, name);
  fprintf (codefile, "{\n");
  generate_rule_aliases (enclosures);
  fprintf (codefile, "return %s;\n", condition->generate_code ());
  fprintf (codefile, "};\n\n");




/* WP WP WP WP WP WP WP WP WP WP WP
   *
   * GET A SET OF BOOLEAN GOAL CONDITIONS FOR THE PROBLEM
   */

  //TODO GET THIS SHIT WORKING PROPERLY -> ONLY PROPOSITIONAL FACTS AS GOAL CONDITIONS FOR FF

  if (ff_enabled){

	  fprintf(codefile, "  std::set<mu_0_boolean*> get_bool_goal_conditions()\n" "  {\n"
					  ""
					  "    std::set<mu_0_boolean*> bool_goal_conds;\n");


		std::set<std::string> boolgoalcondset;
		std::map<std::string, std::pair<std::string, int> > numgoalcondmap;

		expr* lc = condition;


//		std::cout << "/n\n----------------GOAL CONDITIONS ----------------\n\n\n" << std::endl;


		while (lc != NULL) {

			std::string rrr(lc->generate_code_right());
			std::string lll(lc->generate_code_left());
			std::string all(lc->generate_code());

//			std::cout << "ALL : " << all << std::endl;
//			std::cout << "LEFT : " << lll << std::endl;
//			std::cout << "RIGHT : " << rrr << std::endl;

			std::string lstype(lc->gettype()->name);

//			std::cout << "TYPE: " << lstype << std::endl;


			if (lstype.compare("boolean") == 0 && all.find("mu__boolexpr") == std::string::npos && all.find("!") == std::string::npos){


				if (all.find("<") == std::string::npos && all.find(">") == std::string::npos && all.find("==") == std::string::npos) {

					boolgoalcondset.insert(all);
//					std::cout << "--- GOAL top BOOL inserting: " << all << std::endl;
					break;
				}
				else if (rrr.find("true") == std::string::npos && rrr.find("false") == std::string::npos){


	  				if (all.find("==") != std::string::npos) {

//	  					std::cout << "--- GOAL top inserting EQ COMPARISON  expression: " << all << std::endl;

	  					numgoalcondmap.insert(std::make_pair(lll, std::make_pair(rrr,0)));
	  					break;
	  				}
	  				else if (all.find("<=") != std::string::npos) {

//	  					std::cout << "--- GOAL top inserting LTE COMPARISON  expression: " << all << std::endl;

	  					numgoalcondmap.insert(std::make_pair(lll, std::make_pair(rrr, 2)));
	  					break;
	  				}
	  				else if (all.find(">=") != std::string::npos) {

//	  					std::cout << "--- GOAL top inserting GTE COMPARISON  expression: " << all << std::endl;

	  					numgoalcondmap.insert(std::make_pair(lll, std::make_pair(rrr, 1)));
	  					break;
	  				}
	  				else if (all.find("<") != std::string::npos) {

//	  					std::cout << "--- GOAL top inserting LT COMPARISON  expression: " << all << std::endl;

	  					numgoalcondmap.insert(std::make_pair(lll, std::make_pair(rrr, 4)));
	  					break;
	  				}
	  				else if (all.find(">") != std::string::npos) {

//	  					std::cout << "--- GOAL top inserting GT COMPARISON  expression: " << all << std::endl;

	  					numgoalcondmap.insert(std::make_pair(lll, std::make_pair(rrr, 3)));
	  					break;
	  				}




//					std::cout << "--- top inserting COMPARISON boolean expression: " << all << std::endl;
////
//					numgoalcondmap.insert(std::make_pair(lll, rrr));
//					std::cout << "TOP LEFT: " << (lc->generate_code_left()) << std::endl;
//					std::cout << "TOP RIGHT: " << (lc->generate_code_right()) << std::endl;
//
//					break;
				}
			}
			else if (lstype.compare("boolean") == 0 && all.find("mu__boolexpr") != std::string::npos && all.find("!") == std::string::npos && all.find("!") == std::string::npos) {

				lstype = lc->get_right()->gettype()->name;

				if (lstype.compare("boolean") == 0 && rrr.find("mu__boolexpr") == std::string::npos && rrr.find("!") == std::string::npos ) {
					if(rrr.find("<") == std::string::npos && rrr.find(">") == std::string::npos && rrr.find("==") == std::string::npos ) {
						boolgoalcondset.insert(rrr);
//						std::cout << "--- GOAL right BOOL inserting: " << rrr << std::endl;
					}
					else if (rrr.find("true") == std::string::npos && rrr.find("false") == std::string::npos){

						if (rrr.find("==") != std::string::npos) {

//							std::cout << "--- GOAL right inserting EQ COMPARISON  expression: " << rrr << std::endl;

							numgoalcondmap.insert(std::make_pair(lc->get_right()->generate_code_left(), std::make_pair(lc->get_right()->generate_code_right(),0)));
						}
						else if (rrr.find("<=") != std::string::npos) {

//							std::cout << "--- GOAL right inserting LTE COMPARISON  expression: " << rrr << std::endl;

							numgoalcondmap.insert(std::make_pair(lc->get_right()->generate_code_left(), std::make_pair(lc->get_right()->generate_code_right(),2)));
						}
						else if (rrr.find(">=") != std::string::npos) {

//							std::cout << "--- GOAL right inserting GTE COMPARISON  expression: " << rrr << std::endl;

							numgoalcondmap.insert(std::make_pair(lc->get_right()->generate_code_left(), std::make_pair(lc->get_right()->generate_code_right(),1)));
						}
						else if (rrr.find("<") != std::string::npos) {

//							std::cout << "--- GOAL right inserting LT COMPARISON  expression: " << rrr << std::endl;

							numgoalcondmap.insert(std::make_pair(lc->get_right()->generate_code_left(), std::make_pair(lc->get_right()->generate_code_right(),4)));
						}
						else if (rrr.find(">") != std::string::npos) {

//							std::cout << "--- GOAL right inserting GT COMPARISON  expression: " << rrr << std::endl;

							numgoalcondmap.insert(std::make_pair(lc->get_right()->generate_code_left(), std::make_pair(lc->get_right()->generate_code_right(),3)));
						}









//						std::cout << "--- right inserting COMPARISON boolean expression: " << rrr << std::endl;
//
////						std::cout << "RIGHT LEFT: " << (lc->get_right()->generate_code_left()) << std::endl;
////						std::cout << "RIGHT RIGHT: " << (lc->get_right()->generate_code_right()) << std::endl;
//
//						numgoalcondmap.insert(std::make_pair(lc->get_right()->generate_code_left(), lc->get_right()->generate_code_right()));
					}
				}

				else {
//					std::cout << "--- right NOT inserting! : " << rrr << std::endl;
				}
			}

			else {
//				std::cout << "--- NOT inserting anything: " << all << std::endl;
			}

			lc = lc->get_left();
//			std::cout << "============================\n\n" << std::endl;
		}

//		std::cout << "=============================\n\n\n" << std::endl;

			fprintf(codefile, "\n");

			std::set<std::string>::iterator it;
			for (it=boolgoalcondset.begin(); it!=boolgoalcondset.end(); ++it){

			  if ((*it).find("mu__boolexpr")==std::string::npos) {

				  fprintf(codefile, " if (std::string(typeid(%s).name()).compare(\"12mu_0_boolean\") == 0)\n"
						  	  	  	"		bool_goal_conds.insert(&(%s)); \n", it->c_str(), it->c_str() );

			  }
			}


			fprintf(codefile, "\n    return bool_goal_conds;\n");
			fprintf(codefile, "  }\n" "\n");






			/*
			 * WP WP WP WP WP GOAL NUMERIC CONDITIONS
			 *
			 *
			 */

		  fprintf(codefile, "  std::map<mu__real*, std::pair<double, int> > get_numeric_goal_conditions()\n" "  {\n"
						  ""
						  "    std::map<mu__real*, std::pair<double, int> > numeric_goal_conds;\n");

			std::map<std::string, std::pair<std::string, int> >::iterator it2;
			for (it2=numgoalcondmap.begin(); it2!=numgoalcondmap.end(); ++it2){



				  fprintf(codefile, " if (std::string(typeid(%s).name()).compare(\"14mu_1_real_type\") == 0)\n"
						  	  	  	"		numeric_goal_conds.insert(std::make_pair(&(%s), std::make_pair(%s, %d))); \n",
						  	  	  	it2->first.c_str(), it2->first.c_str(), it2->second.first.c_str(), it2->second.second );


			}


			fprintf(codefile, "\n    return numeric_goal_conds;\n");
			fprintf(codefile, "  }\n" "\n");





  }










  /* there is no rule function as it is a invariant */
  /* may be added later */

  /* generate different instances of the rule according the
     ruleset parameter */
  /* currently syntax enforce a single instance only */
  generate_rules (enclosures, this);
  return "ERROR!";
}
//UPMURPHI_END

/********************
  code for invariant
 ********************/
const char *invariant::generate_code()
{
  int rule_int = new_int();
  condname = tsprintf("mu__invariant_%d", rule_int);

  /* generate the parameters to the function for the invariant. */
  /* e.g.  "int pid, int mid" as in "mu__invariant_1(int pid, int mid)" */
  char *params = generate_rule_params(enclosures);

  /* generate the condition function. */
  /* which is called by the different instances of the invariant */
  fprintf(codefile, "int %s(%s) // Invariant \"%s\"\n",
          condname, params, name);
  fprintf(codefile, "{\n");
  generate_rule_aliases(enclosures);
  fprintf(codefile, "return %s;\n", condition->generate_code());
  fprintf(codefile, "};\n\n");

  /* there is no rule function as it is a invariant */
  /* may be added later */

  /* generate different instances of the rule according the
     ruleset parameter */
  /* currently syntax enforce a single instance only */
  generate_rules(enclosures, this);
  return "ERROR!";
}

/********************
  code for quantrule
 ********************/
const char *quantrule::generate_code()
{
  // generate each rule within the quantifier
  for (rule * r = rules; r != NULL; r = r->next)
    r->generate_code();
  return "ERROR!";
}

/********************
  code for chooserule
 ********************/
const char *chooserule::generate_code()
{
  // generate each rule within the quantifier
  for (rule * r = rules; r != NULL; r = r->next)
    r->generate_code();
  return "ERROR!";
}

/********************
  code for aliasrule
 ********************/
const char *aliasrule::generate_code()
{
  // generate each rule within the alias
  for (rule * r = rules; r != NULL; r = r->next)
    r->generate_code();
  return "ERROR!";
}

/******************************
  From here on is all functions that 'globally' affect the world.
 ******************************/

/* process a list of ste\'s in reverse order. */
const char *ste::generate_decls()
{
  if (this != NULL) {
    if (next != NULL && next->scope == scope)
      next->generate_decls();
    value->generate_decl();
  }
  return "ERROR!";
}

// char *ste::generate_alias_decls()
// {
//   if ( this != NULL ) {
//       value->generate_decl();
//       if ( next != NULL && next->scope == scope )
//  next->generate_decls();
//     }
//   return "ERROR!";
// }

typedef void (*varproc) (vardecl * var);


void map_vars(ste * globals, varproc proc)
/* Calls proc for each variable in the list of declarations. */
{
  ste *next;
  if (globals != NULL) {
    next = globals->getnext();
    if (next != NULL && next->getscope() == globals->getscope())
      map_vars(next, proc);
    if (globals->getvalue()->getclass() == decl::Var) {
      (*proc) ((vardecl *) globals->getvalue());
    }
  }
}


/* The world::print() function prints out the state of every variable.
 * We have to make our own from the variable declarations. */
void make_print_world_aux(vardecl * var)
{
  fprintf(codefile, "  %s.print(target, separator);\n", var->generate_code());
}

void make_print_pddlworld_aux(vardecl * var)
{
  if (var->pddlattrs.pddlname)
    fprintf(codefile, "  %s.print(target, separator);\n", var->generate_code());
}

void make_print_world(ste * globals)
/* done recursively. */
{

	//WP WP WP WP get_f_val
	fprintf(codefile,
	"//WP WP WP WP WP\n"
	"double world_class::get_f_val()\n"
	"{\n"
	"  double f_val = mu_f_n.value();\n"
	"  return f_val;\n"
	"}\n\n");


	if(ff_enabled){
		fprintf(codefile,
		"//WP WP WP WP WP\n"
		"void world_class::fire_processes()\n"
		"{\n");
		std::map< std::string, std::vector< std::pair< std::pair<std::string, std::string>, std::pair<int, int> > > >::iterator ip;
		for (ip = processes.begin(); ip != processes.end(); ++ip){

	//		std::cout << "\nLOOOOPING: \n" << (*ip).first << " ( ";
	//		for (int ii = 0; ii < (*ip).second.size(); ii++){
	//			std::cout << (*ip).second.at(ii).first.first << " " << (*ip).second.at(ii).first.second <<
	//					"( " << (*ip).second.at(ii).second.first << "," << (*ip).second.at(ii).second.second << "), ";
	//
	//		}
	//		std::cout << " ) " << std::endl;

	//		std::cout << "\nLOOOOPING: \n" << (*ip).first << " ( ";

			for (int ii = 0; ii < (*ip).second.size(); ii++){

				fprintf(codefile, "		for(int %s = %d; %s <= %d; %s++)\n",
						(*ip).second.at(ii).first.second.c_str(), (*ip).second.at(ii).second.first,
						(*ip).second.at(ii).first.second.c_str(), (*ip).second.at(ii).second.second,
						(*ip).second.at(ii).first.second.c_str() );

			}

			fprintf(codefile, "			%s(", (*ip).first.c_str() );

			for (int j = 0; j < (*ip).second.size(); j++){
				fprintf(codefile, "%s", (*ip).second.at(j).first.second.c_str());
				if (j != (*ip).second.size()-1) fprintf(codefile, ", ");
			}

			fprintf(codefile, ");\n\n");
		}


		fprintf(codefile, "}\n\n");
	}



	if(ff_enabled){
		// TODO
		fprintf(codefile,
		"//WP WP WP WP WP\n"
		"void world_class::fire_processes_plus()\n"
		"{\n");
		std::map< std::string, std::vector< std::pair< std::pair<std::string, std::string>, std::pair<int, int> > > >::iterator ip_plus;

		for (ip_plus = processes_plus.begin(); ip_plus != processes_plus.end(); ++ip_plus){

	//		std::cout << "\nLOOOOPING: \n" << (*ip).first << " ( ";
	//		for (int ii = 0; ii < (*ip).second.size(); ii++){
	//			std::cout << (*ip).second.at(ii).first.first << " " << (*ip).second.at(ii).first.second <<
	//					"( " << (*ip).second.at(ii).second.first << "," << (*ip).second.at(ii).second.second << "), ";
	//
	//		}
	//		std::cout << " ) " << std::endl;

	//		std::cout << "\nLOOOOPING: \n" << (*ip).first << " ( ";

			if ((*ip_plus).first.size() >= 1){

				for (int ii = 0; ii < (*ip_plus).second.size(); ii++){

					fprintf(codefile, "for(int %s = %d; %s <= %d; %s++)\n",
							(*ip_plus).second.at(ii).first.second.c_str(), (*ip_plus).second.at(ii).second.first,
							(*ip_plus).second.at(ii).first.second.c_str(), (*ip_plus).second.at(ii).second.second,
							(*ip_plus).second.at(ii).first.second.c_str() );

				}

				fprintf(codefile, "{\n\n");

				fprintf(codefile, "\n\n if (%s) \n", global_test_plus[(*ip_plus).first]->generate_code());// WP WP WP WP WP

				fprintf(codefile, "%s", (*ip_plus).first.c_str() );

				fprintf(codefile, "\n\n}\n");

	//			fprintf(codefile, "			%s_plus(", (*ip_plus).first.c_str() );
	//
	//			for (int j = 0; j < (*ip_plus).second.size(); j++){
	//				fprintf(codefile, "%s", (*ip_plus).second.at(j).first.second.c_str());
	//				if (j != (*ip_plus).second.size()-1) fprintf(codefile, ", ");
	//			}
	//
	//			fprintf(codefile, ");\n\n");
			}
		}



		fprintf(codefile, "}\n\n");
	}


	if(ff_enabled){
		fprintf(codefile,
		"//WP WP WP WP WP\n"
		"void world_class::fire_processes_minus()\n"
		"{\n");
		std::map< std::string, std::vector< std::pair< std::pair<std::string, std::string>, std::pair<int, int> > > >::iterator ip_minus;
		for (ip_minus = processes_minus.begin(); ip_minus != processes_minus.end(); ++ip_minus){

	//		std::cout << "\nLOOOOPING: \n" << (*ip).first << " ( ";
	//		for (int ii = 0; ii < (*ip).second.size(); ii++){
	//			std::cout << (*ip).second.at(ii).first.first << " " << (*ip).second.at(ii).first.second <<
	//					"( " << (*ip).second.at(ii).second.first << "," << (*ip).second.at(ii).second.second << "), ";
	//
	//		}
	//		std::cout << " ) " << std::endl;

	//		std::cout << "\nLOOOOPING: \n" << (*ip).first << " ( ";

			if ((*ip_minus).first.size() >= 1){

				for (int ii = 0; ii < (*ip_minus).second.size(); ii++){

					fprintf(codefile, "for(int %s = %d; %s <= %d; %s++)\n",
							(*ip_minus).second.at(ii).first.second.c_str(), (*ip_minus).second.at(ii).second.first,
							(*ip_minus).second.at(ii).first.second.c_str(), (*ip_minus).second.at(ii).second.second,
							(*ip_minus).second.at(ii).first.second.c_str() );

				}


				fprintf(codefile, "{\n\n");

				fprintf(codefile, "\n\n if (%s) \n", global_test_minus[(*ip_minus).first]->generate_code());// WP WP WP WP WP


				fprintf(codefile, "%s", (*ip_minus).first.c_str() );

				fprintf(codefile, "\n\n}\n");


	//			fprintf(codefile, "			%s_minus(", (*ip_minus).first.c_str() );
	//
	//			for (int j = 0; j < (*ip_minus).second.size(); j++){
	//				fprintf(codefile, "%s", (*ip_minus).second.at(j).first.second.c_str());
	//				if (j != (*ip_minus).second.size()-1) fprintf(codefile, ", ");
	//			}
	//
	//			fprintf(codefile, ");\n\n");
			}
		}


		fprintf(codefile, "}\n\n");

	}






	//WP WP WP WP set_f_val
	fprintf(codefile,
	"//WP WP WP WP WP\n"
	"void world_class::set_f_val()\n"
	"{\n"
	"  double f_val = mu_g_n.value() + mu_h_n.value();\n"
	"  mu_f_n.value(f_val);\n"
	"}\n\n");

	//WP WP WP WP get_h_val
	fprintf(codefile,
	"//WP WP WP WP WP\n"
	"double world_class::get_h_val()\n"
	"{\n"
	"  double h_val = mu_h_n.value();\n"
	"  return h_val;\n"
	"}\n\n");


	// WP WP WP WP set_h_val with or without FF RPG
	if (ff_enabled == true){

	//WP WP WP WP set_h_val using FF-RPG or numeric RPG
	fprintf(codefile,
	"//WP WP WP WP WP\n"
	"void world_class::set_h_val()\n"
	"{\n"
	"  //	NON-HEURISTIC SEARCH\n"
	"  // double h_val = 0; \n\n"


	"  //	FF RPG\n"
	"  //upm_rpg::getInstance().clear_all();\n"
	"  //double h_val = upm_rpg::getInstance().compute_rpg();\n\n\n"


	"  //	NUMERIC RPG\n"
	"  //upm_numeric_rpg::getInstance().clear_all();\n"
	"  //double h_val = upm_numeric_rpg::getInstance().compute_rpg();\n\n"

	"  //	TEMPORAL RPG\n"
	"  upm_staged_rpg::getInstance().clear_all();\n"
	"  double h_val = upm_staged_rpg::getInstance().compute_rpg();\n\n"

	"  mu_h_n.value(h_val);\n"

	"}\n\n");

	//WP WP WP WP set_h_val with parameters
	fprintf(codefile,
	"//WP WP WP WP WP\n"
	"void world_class::set_h_val(int hp)\n"
	"{\n"
	"  double h_val = hp; \n"
	"  mu_h_n.value(h_val);\n"
	"}\n\n");

	} else {

	//WP WP WP WP set_h_val
	fprintf(codefile,
	"//WP WP WP WP WP\n"
	"void world_class::set_h_val()\n"
	"{\n"
	"  double h_val = 0; \n"
	"  mu_h_n.value(h_val);\n"
	"}\n\n");

	//WP WP WP WP set_h_val with parameters
	fprintf(codefile,
	"//WP WP WP WP WP\n"
	"void world_class::set_h_val(int hp)\n"
	"{\n"
	"  double h_val = hp; \n"
	"  mu_h_n.value(h_val);\n"
	"}\n\n");

	}


	//WP WP WP WP get_g_val
	fprintf(codefile,
	"//WP WP WP WP WP\n"
	"double world_class::get_g_val()\n"
	"{\n"
	"  double g_val = mu_g_n.value();\n"
	"  return g_val;\n"
	"}\n\n");

	//WP WP WP WP set_g_val
	fprintf(codefile,
	"//WP WP WP WP WP\n"
	"void world_class::set_g_val(double g_val)\n"
	"{\n"
	"  mu_g_n.value(g_val);\n"
	"}\n\n");




  fprintf(codefile,
          "void world_class::print(FILE *target, const char *separator)\n"
          "{\n"
          "  static int num_calls = 0; /* to ward off recursive calls. */\n"
          "  if ( num_calls == 0 ) {\n" "    num_calls++;\n");
  map_vars(globals, &make_print_world_aux);
  fprintf(codefile, "    num_calls--;\n" "}\n}\n");
}


void make_get_pddlclock()
{
  if (theprog->clock_var_name!=NULL) {
    fprintf(codefile,
            "double world_class::get_clock_value()\n"
            "{\n"
            "  return %s.value();\n"
            "}\n",theprog->clock_var_name);
  }
}

void make_print_pddlworld(ste * globals)
{
  fprintf(codefile,
          "void world_class::pddlprint(FILE *target, const char *separator)\n"
          "{\n"
          "  static int num_calls = 0; /* to ward off recursive calls. */\n"
          "  if ( num_calls == 0 ) {\n" "    num_calls++;\n");
  map_vars(globals, &make_print_pddlworld_aux);
  fprintf(codefile, "    num_calls--;\n" "}\n}\n");
}

/* The world::print_statistic() function prints out certain statistic of some variables.
 * We have to make our own from the variable declarations. */
void make_print_statistic_aux(vardecl * var)
{
  fprintf(codefile, "  %s.print_statistic();\n", var->generate_code());
}

void make_print_statistic(ste * globals)
/* done recursively. */
{
  fprintf(codefile,
          "void world_class::print_statistic()\n"
          "{\n"
          "  static int num_calls = 0; /* to ward off recursive calls. */\n"
          "  if ( num_calls == 0 ) {\n" "    num_calls++;\n");
  map_vars(globals, &make_print_statistic_aux);
  fprintf(codefile, "    num_calls--;\n" "}\n}\n");
}

/* the world::clear() function clears every variable. */
void make_clear_aux(vardecl * var)
{
  fprintf(codefile, "  %s.clear();\n", var->generate_code());
}

void make_clear(ste * globals)
{
  fprintf(codefile, "void world_class::clear()\n" "{\n");
  map_vars(globals, &make_clear_aux);
  fprintf(codefile, "}\n");
}

/* the world::undefine() function resets every variable to an undefined state. */
void make_undefine_aux(vardecl * var)
{
  fprintf(codefile, "  %s.undefine();\n", var->generate_code());
}

void make_undefine(ste * globals)
{
  fprintf(codefile, "void world_class::undefine()\n" "{\n");
  map_vars(globals, &make_undefine_aux);
  fprintf(codefile, "}\n");
}

/* WP WP WP WP WP: */
void make_get_mu_bools_aux(vardecl * var)
{
	std::string b_type("mu_0_boolean");
	if (var->gettype()->generate_code() == b_type)
	{
		fprintf(codefile, "      awesome.push_back(&(%s));\n", var->generate_code() );
	}
}

/* WP WP WP WP WP: TODO */
void make_get_mu_bools(ste * globals)
{
  fprintf(codefile, "std::vector<mu_0_boolean*> world_class::get_mu_bools()\n"
		  "{\n"
		  	  "	  std::vector<mu_0_boolean*> awesome;" "\n\n");
  map_vars(globals, &make_get_mu_bools_aux);
  fprintf(codefile, "    return awesome; \n}\n");
}


/* WP WP WP WP WP: */
void make_get_mu_nums_aux(vardecl * var)
{
	std::string n_type("mu_1_real_type");
	if (var->gettype()->generate_code() == n_type)
	{
		fprintf(codefile, "      awesome.push_back(&(%s));\n", var->generate_code() );
	}
}

/* WP WP WP WP WP: TODO */
void make_get_mu_nums(ste * globals)
{
  fprintf(codefile, "std::vector<mu__real*> world_class::get_mu_nums()\n"
		  "{\n"
		  	  "	  std::vector<mu__real*> awesome;" "\n\n");
  map_vars(globals, &make_get_mu_nums_aux);
  fprintf(codefile, "    return awesome; \n}\n");
}


/* WP WP WP WP WP: */
void make_get_mu_num_arrays_aux(vardecl * var)
{
	std::string ba_type("mu_1__type");
	std::string s_comp(var->gettype()->generate_code());
	if (s_comp.find(ba_type) != std::string::npos)
	{
		fprintf(codefile, "      interm = %s.num_array();\n"
				"		if (interm.size() > 0) var_arrays.insert(var_arrays.end(), interm.begin(), interm.end());\n", var->generate_code() );
	}
}

/* WP WP WP WP WP: TODO */
void make_get_mu_num_arrays(ste * globals)
{
  fprintf(codefile, "std::vector<mu__real*> world_class::get_mu_num_arrays()\n"
		  "{\n"
		  	  "	  std::vector<mu__real*> var_arrays;\n"
		  	  "   std::vector<mu__real*> interm;\n"
		      "\n");
  map_vars(globals, &make_get_mu_num_arrays_aux);
  fprintf(codefile, "    return var_arrays; \n}\n");
}


/* WP WP WP WP WP: */
void make_get_mu_bool_arrays_aux(vardecl * var)
{
	std::string ba_type("mu_1__type");
	std::string s_comp(var->gettype()->generate_code());
	if (s_comp.find(ba_type) != std::string::npos)
	{
		fprintf(codefile, "      interm = %s.bool_array();\n"
				"		if (interm.size() > 0) var_arrays.insert(var_arrays.end(), interm.begin(), interm.end());\n", var->generate_code() );
	}
}

/* WP WP WP WP WP: TODO */
void make_get_mu_bool_arrays(ste * globals)
{
  fprintf(codefile, "std::vector<mu_0_boolean*> world_class::get_mu_bool_arrays()\n"
		  "{\n"
		  	  "	  std::vector<mu_0_boolean*> var_arrays;\n"
		  	  "   std::vector<mu_0_boolean*> interm;\n"
		      "\n");
  map_vars(globals, &make_get_mu_bool_arrays_aux);
  fprintf(codefile, "    return var_arrays; \n}\n");
}



/* the world::reset() function resets every variable to an resetd state. */
void make_reset_aux(vardecl * var)
{
  fprintf(codefile, "  %s.reset();\n", var->generate_code());
}

void make_reset(ste * globals)
{
  fprintf(codefile, "void world_class::reset()\n" "{\n");
  map_vars(globals, &make_reset_aux);
  fprintf(codefile, "}\n");
}

/* the world::getstate() function returns a state
 * encoding the state of the world. */
void make_getstate_aux(vardecl * var)
{
  fprintf(codefile, "  %s.to_state( newstate );\n", var->generate_code());
}

void make_getstate(ste * globals)
{
  fprintf(codefile, "void world_class::to_state(state *newstate)\n{\n");
  // fprintf(codefile,"state *newstate = new state;\n");
  /* C++ doesn\'t know what a state looks like yet. Feh. */
  map_vars(globals, &make_getstate_aux);
  // fprintf(codefile,"return newstate;\n");
  fprintf(codefile, "}\n");
}

/* the world::setstate() function sets the world to the state recorded
 * in thestate. */
void make_setstate_aux(vardecl * var)
{
//  fprintf(codefile,"  %s.from_state(thestate);\n",
//    var->generate_code() );
}

void make_setstate(ste * globals)
{
  fprintf(codefile, "void world_class::setstate(state *thestate)\n{\n");
  map_vars(globals, &make_setstate_aux);
  fprintf(codefile, "}\n");
}


void make_print_diff_aux(vardecl * var)
{
  fprintf(codefile, "    %s.print_diff(prevstate,target,separator);\n",
          var->generate_code());
}

void make_print_diff(ste * globals)
{
  fprintf(codefile,
          "void world_class::print_diff(state *prevstate, FILE *target, const char *separator)\n"
          "{\n" "  if ( prevstate != NULL )\n" "  {\n");
  map_vars(globals, &make_print_diff_aux);
  fprintf(codefile, "  }\n");
  fprintf(codefile, "  else\n" "print(target,separator);\n");
  fprintf(codefile, "}\n");
}


void make_world(ste * globals)
/* generate the world state. */
{
  make_clear(globals);
  make_undefine(globals);
  make_reset(globals);
  if (ff_enabled == true) make_get_mu_bools(globals); /* WP: */
  if (ff_enabled == true) make_get_mu_bool_arrays(globals); /* WP: */
  if (ff_enabled == true) make_get_mu_nums(globals); /* WP: */
  if (ff_enabled == true) make_get_mu_num_arrays(globals); /* WP: */
  make_print_world(globals);
  make_print_pddlworld(globals);
  make_get_pddlclock();
  make_print_statistic(globals);
  make_print_diff(globals);
  make_getstate(globals);
  make_setstate(globals);
}


/* Generate the global lists of rules.
 * since we can initialize arrays with a list
 * that ends in a comma, we do so here. */

int rulerec::print_rules()
{
  rulerec *r = this;
  int i = 0;
  if (r != NULL) {
    for (; r != NULL; r = r->next) {
      fprintf(codefile, "{\"%s\", ", r->rulename);
      if (r->conditionname == NULL)
        fprintf(codefile, "NULL, ");
      else
        fprintf(codefile, "&%s, ", r->conditionname);
      if (r->bodyname == NULL)
        fprintf(codefile, "NULL, ");
      else
        fprintf(codefile, "&%s, ", r->bodyname);
      fprintf(codefile, "},\n");
      i++;
    }
  } else {
    fprintf(codefile, "{ NULL, NULL, NULL, FALSE }");
  }
  return i;
}

int generate_invariants()
{
  int i, r;
  simplerule *sr;

  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Invar) {
      sr->rulenumber = r++;
      sr->generate_code();
    }
  }
  return r;
}

int generate_startstates()
{
  int i, r;
  simplerule *sr;

  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Startstate) {
      sr->rulenumber = r++;
      sr->generate_code();
    }
  }
  r = 0;
  fprintf(codefile, "class StartStateGenerator\n" "{\n");
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Startstate) {
      fprintf(codefile, "  StartStateBase%d S%d;\n", r, r);
      r++;
    }
  }
  fprintf(codefile, "public:\n");

  // generate Code(r)
  fprintf(codefile, "void Code(unsigned short r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Startstate) {
      if (i != 0)
        fprintf(codefile,
                "  if (r>=%d && r<=%d) { S%d.Code(r-%d); return; }\n",
                i, i + sr->getsize() - 1, r, i);
      else
        fprintf(codefile,
                "  if (r<=%d) { S%d.Code(r-%d); return; }\n",
                i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile, "}\n");

  // generate Name(r)
  fprintf(codefile, "char * Name(unsigned short r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Startstate) {
      if (i != 0)
        fprintf(codefile,
                "  if (r>=%d && r<=%d) return S%d.Name(r-%d);\n",
                i, i + sr->getsize() - 1, r, i);
      else
        fprintf(codefile,
                "  if (r<=%d) return S%d.Name(r-%d);\n",
                i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile, "  return NULL;\n");	// Uli: return added
  fprintf(codefile, "}\n");
  fprintf(codefile, "};\n");
  return i;
}

//UPMURPHI_BEGIN
/* generate_Goal() */
int
generate_Goals ()
{
  int i, r;
  simplerule *sr;

  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL; sr = sr->NextSimpleRule) {
    if (sr->getclass () == rule::Goal) {
      sr->rulenumber = r++;
      fprintf(codefile, "\n// WP WP WP GOAL\n");
      sr->generate_code ();
      fprintf(codefile, "\n// WP WP WP GOAL\n");
    }
  }
  return r;
}
//UPMURPHI_END

RULE_INDEX_TYPE generate_ruleset()
{
  RULE_INDEX_TYPE i, r, max_r;
  simplerule *sr;

  r = 0;
  max_r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      sr->rulenumber = r++;
      max_r += sr->getsize();
      sr->maxrulenumber = max_r;
      sr->generate_code();
    }
  }

  r = 0;
  fprintf(codefile, "class NextStateGenerator\n" "{\n");
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      fprintf(codefile, "  RuleBase%lu R%lu;\n", r, r);
      r++;
    }
  }
  fprintf(codefile, "public:\n");

  // generate SetNextEnabledRule(r)
  i = 0;
  r = 0;

  fprintf(codefile,
          "void SetNextEnabledRule(RULE_INDEX_TYPE & what_rule)\n"
          "{\n" "  category = CONDITION;\n");
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {

      if (i != 0)
        fprintf(codefile,
                "  if (what_rule>=%lu && what_rule<%lu)\n"
                "    { R%lu.NextRule(what_rule);\n"
                "      if (what_rule<%lu) return; }\n",
                i, i + sr->getsize(), r, i + sr->getsize());
      else
        fprintf(codefile,
                "  if (what_rule<%lu)\n"
                "    { R%lu.NextRule(what_rule);\n"
                "      if (what_rule<%lu) return; }\n",
                i + sr->getsize(), r, i + sr->getsize());
      r++;
      i += sr->getsize();

    }
  }
  fprintf(codefile, "}\n");

  // generate Condition(r)
  fprintf(codefile,
          "bool Condition(RULE_INDEX_TYPE r)\n" "{\n"
          "  category = CONDITION;\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      if (i != 0)
        fprintf(codefile,
                "  if (r>=%lu && r<=%lu) return R%lu.Condition(r-%lu);\n",
                i, i + sr->getsize() - 1, r, i);
      else
        fprintf(codefile,
                "  if (r<=%lu) return R%lu.Condition(r-%lu);\n",
                i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile,
          "Error.Notrace(\"Internal: NextStateGenerator -- checking condition for nonexisting rule.\");\n"
          "}\n");





/*
   * WP WP WP WP WP WP WP WP WP
   */
  // generate boolean precond_array(r)

  if (ff_enabled == true){
    fprintf(codefile,
  	  "std::vector<mu_0_boolean*> bool_precond_array(RULE_INDEX_TYPE r)\n" "{\n"
  	  "  category = CONDITION;\n");
    i = 0;
    r = 0;
    for (sr = simplerule::SimpleRuleList; sr != NULL;
         sr = sr->NextSimpleRule) {
      if (sr->getclass() == rule::Simple && sr != error_rule) {
        if (i != 0)
  	fprintf(codefile,
  		"  if (r>=%lu && r<=%lu) return R%lu.bool_precond_array(r-%lu);\n",
  		i, i + sr->getsize() - 1, r, i);
        else
  	fprintf(codefile,
  		"  if (r<=%lu) return R%lu.bool_precond_array(r-%lu);\n",
  		i + sr->getsize() - 1, r, i);
        r++;
        i += sr->getsize();
      }
    }
    fprintf(codefile,
  	  "Error.Notrace(\"Internal: NextStateGenerator -- checking preconditions for nonexisting rule.\");\n"
  	  "}\n");
  }



  /*
   * WP WP WP WP WP WP WP WP WP
   */
  // generate numeric precond_array(r)

  if (ff_enabled == true){
    fprintf(codefile,
  	  "std::map<mu__real*, std::pair<double,int> > num_precond_array(RULE_INDEX_TYPE r)\n" "{\n"
  	  "  category = CONDITION;\n");
    i = 0;
    r = 0;
    for (sr = simplerule::SimpleRuleList; sr != NULL;
         sr = sr->NextSimpleRule) {
      if (sr->getclass() == rule::Simple && sr != error_rule) {
        if (i != 0)
  	fprintf(codefile,
  		"  if (r>=%lu && r<=%lu) return R%lu.num_precond_array(r-%lu);\n",
  		i, i + sr->getsize() - 1, r, i);
        else
  	fprintf(codefile,
  		"  if (r<=%lu) return R%lu.num_precond_array(r-%lu);\n",
  		i + sr->getsize() - 1, r, i);
        r++;
        i += sr->getsize();
      }
    }
    fprintf(codefile,
  	  "Error.Notrace(\"Internal: NextStateGenerator -- checking preconditions for nonexisting rule.\");\n"
  	  "}\n");
  }




  /*
   * WP WP WP WP WP WP WP WP WP
   */
  // generate boolean precond_array(r)

  if (ff_enabled == true){
    fprintf(codefile,
  	  "std::vector<mu__any*> all_precond_array(RULE_INDEX_TYPE r)\n" "{\n"
  	  "  category = CONDITION;\n");
    i = 0;
    r = 0;
    for (sr = simplerule::SimpleRuleList; sr != NULL;
         sr = sr->NextSimpleRule) {
      if (sr->getclass() == rule::Simple && sr != error_rule) {
        if (i != 0)
  	fprintf(codefile,
  		"  if (r>=%lu && r<=%lu) return R%lu.all_precond_array(r-%lu);\n",
  		i, i + sr->getsize() - 1, r, i);
        else
  	fprintf(codefile,
  		"  if (r<=%lu) return R%lu.all_precond_array(r-%lu);\n",
  		i + sr->getsize() - 1, r, i);
        r++;
        i += sr->getsize();
      }
    }
    fprintf(codefile,
  	  "Error.Notrace(\"Internal: NextStateGenerator -- checking preconditions for nonexisting rule.\");\n"
  	  "}\n");
  }



  /*
     * WP WP WP WP WP WP WP WP WP
     */
    // generate boolean precond_array(r)

      fprintf(codefile,
    	  "std::set<std::pair<mu_0_boolean*, int> > precond_bool_interference(RULE_INDEX_TYPE r)\n" "{\n"
    	  "  category = CONDITION;\n");
      i = 0;
      r = 0;
      for (sr = simplerule::SimpleRuleList; sr != NULL;
           sr = sr->NextSimpleRule) {
        if (sr->getclass() == rule::Simple && sr != error_rule) {
          if (i != 0)
    	fprintf(codefile,
    		"  if (r>=%lu && r<=%lu) return R%lu.precond_bool_interference(r-%lu);\n",
    		i, i + sr->getsize() - 1, r, i);
          else
    	fprintf(codefile,
    		"  if (r<=%lu) return R%lu.precond_bool_interference(r-%lu);\n",
    		i + sr->getsize() - 1, r, i);
          r++;
          i += sr->getsize();
        }
      }
      fprintf(codefile,
    	  "Error.Notrace(\"Internal: NextStateGenerator -- checking preconditions for nonexisting rule.\");\n"
    	  "}\n");



  /*
   * WP WP WP WP WP WP WP WP WP
   */
  // generate temporal constraints array(r)

  if (ff_enabled == true){
    fprintf(codefile,
  	  "std::pair<double, double> temporal_constraints(RULE_INDEX_TYPE r)\n" "{\n"
  	  "  category = CONDITION;\n");
    i = 0;
    r = 0;
    for (sr = simplerule::SimpleRuleList; sr != NULL;
         sr = sr->NextSimpleRule) {
      if (sr->getclass() == rule::Simple && sr != error_rule) {
        if (i != 0)
  	fprintf(codefile,
  		"  if (r>=%lu && r<=%lu) return R%lu.temporal_constraints(r-%lu);\n",
  		i, i + sr->getsize() - 1, r, i);
        else
  	fprintf(codefile,
  		"  if (r<=%lu) return R%lu.temporal_constraints(r-%lu);\n",
  		i + sr->getsize() - 1, r, i);
        r++;
        i += sr->getsize();
      }
    }
    fprintf(codefile,
  	  "Error.Notrace(\"Internal: NextStateGenerator -- checking preconditions for nonexisting rule.\");\n"
  	  "}\n");
  }


      /*
       * WP WP WP WP WP WP WP WP WP
       */
      // generate effects_bool_interference(r)

  if (ff_enabled == true){
        fprintf(codefile,
      	  "std::set<std::pair<mu_0_boolean*, int> > effects_bool_interference(RULE_INDEX_TYPE r)\n" "{\n");
        i = 0;
        r = 0;
        for (sr = simplerule::SimpleRuleList; sr != NULL;
             sr = sr->NextSimpleRule) {
          if (sr->getclass() == rule::Simple && sr != error_rule) {
            if (i != 0)
      	fprintf(codefile,
      		"  if (r>=%lu && r<=%lu) return R%lu.effects_bool_interference(r-%lu);\n",
      		i, i + sr->getsize() - 1, r, i);
            else
      	fprintf(codefile,
      		"  if (r<=%lu) return R%lu.effects_bool_interference(r-%lu);\n",
      		i + sr->getsize() - 1, r, i);
            r++;
            i += sr->getsize();
          }
        }
        fprintf(codefile,
      	  "Error.Notrace(\"Internal: NextStateGenerator -- checking add effects for nonexisting rule.\");\n"
      	  "}\n");
  }

  /*
   * WP WP WP WP WP WP WP WP WP
   */
  // generate effects_add_bool_array(r)

if (ff_enabled == true){
    fprintf(codefile,
  	  "std::vector<mu_0_boolean*> effects_add_bool_array(RULE_INDEX_TYPE r)\n" "{\n");
    i = 0;
    r = 0;
    for (sr = simplerule::SimpleRuleList; sr != NULL;
         sr = sr->NextSimpleRule) {
      if (sr->getclass() == rule::Simple && sr != error_rule) {
        if (i != 0)
  	fprintf(codefile,
  		"  if (r>=%lu && r<=%lu) return R%lu.effects_add_bool_array(r-%lu);\n",
  		i, i + sr->getsize() - 1, r, i);
        else
  	fprintf(codefile,
  		"  if (r<=%lu) return R%lu.effects_add_bool_array(r-%lu);\n",
  		i + sr->getsize() - 1, r, i);
        r++;
        i += sr->getsize();
      }
    }
    fprintf(codefile,
  	  "Error.Notrace(\"Internal: NextStateGenerator -- checking add effects for nonexisting rule.\");\n"
  	  "}\n");
}

  /*
   * WP WP WP WP WP WP WP WP WP
   */
  // generate effects_num_array(r)

if (ff_enabled == true){
    fprintf(codefile,
  	  "std::vector<mu__real*> effects_num_array(RULE_INDEX_TYPE r)\n" "{\n");
    i = 0;
    r = 0;
    for (sr = simplerule::SimpleRuleList; sr != NULL;
         sr = sr->NextSimpleRule) {
      if (sr->getclass() == rule::Simple && sr != error_rule) {
        if (i != 0)
  	fprintf(codefile,
  		"  if (r>=%lu && r<=%lu) return R%lu.effects_num_array(r-%lu);\n",
  		i, i + sr->getsize() - 1, r, i);
        else
  	fprintf(codefile,
  		"  if (r<=%lu) return R%lu.effects_num_array(r-%lu);\n",
  		i + sr->getsize() - 1, r, i);
        r++;
        i += sr->getsize();
      }
    }
    fprintf(codefile,
  	  "Error.Notrace(\"Internal: NextStateGenerator -- checking add effects for nonexisting rule.\");\n"
  	  "}\n");
}


/*
 * WP WP WP WP WP WP WP WP WP
 */
// generate effects_all_array(r)

if (ff_enabled == true){
  fprintf(codefile,
	  "std::vector<mu__any*> effects_all_array(RULE_INDEX_TYPE r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      if (i != 0)
	fprintf(codefile,
		"  if (r>=%lu && r<=%lu) return R%lu.effects_all_array(r-%lu);\n",
		i, i + sr->getsize() - 1, r, i);
      else
	fprintf(codefile,
		"  if (r<=%lu) return R%lu.effects_all_array(r-%lu);\n",
		i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile,
	  "Error.Notrace(\"Internal: NextStateGenerator -- checking add effects for nonexisting rule.\");\n"
	  "}\n");
}






  // generate Code(r)
  fprintf(codefile, "void Code(RULE_INDEX_TYPE r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      if (i != 0)
        fprintf(codefile,
                "  if (r>=%lu && r<=%lu) { R%lu.Code(r-%lu); return; } \n",
                i, i + sr->getsize() - 1, r, i);
      else
        fprintf(codefile,
                "  if (r<=%lu) { R%lu.Code(r-%lu); return; } \n",
                i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile, "}\n");






  /*
   * WP WP WP WP WP WP WP WP WP
   */
  // generate Code FF(r)

 if (ff_enabled){
  fprintf(codefile, "void Code_ff(RULE_INDEX_TYPE r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      if (i != 0)
	fprintf(codefile,
		"  if (r>=%lu && r<=%lu) { R%lu.Code_ff(r-%lu); return; } \n",
		i, i + sr->getsize() - 1, r, i);
      else
	fprintf(codefile,
		"  if (r<=%lu) { R%lu.Code_ff(r-%lu); return; } \n",
		i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile, "}\n");

 }



 /*
  * WP WP WP WP WP WP WP WP WP
  */
 // generate Code numeric FF(r) +

if (ff_enabled){
 fprintf(codefile, "void Code_numeric_ff_plus(RULE_INDEX_TYPE r)\n" "{\n");
 i = 0;
 r = 0;
 for (sr = simplerule::SimpleRuleList; sr != NULL;
      sr = sr->NextSimpleRule) {
   if (sr->getclass() == rule::Simple && sr != error_rule) {
     if (i != 0)
	fprintf(codefile,
		"  if (r>=%lu && r<=%lu) { R%lu.Code_numeric_ff_plus(r-%lu); return; } \n",
		i, i + sr->getsize() - 1, r, i);
     else
	fprintf(codefile,
		"  if (r<=%lu) { R%lu.Code_numeric_ff_plus(r-%lu); return; } \n",
		i + sr->getsize() - 1, r, i);
     r++;
     i += sr->getsize();
   }
 }
 fprintf(codefile, "}\n");

}



/*
 * WP WP WP WP WP WP WP WP WP
 */
// generate Code numeric FF(r) -

if (ff_enabled){
fprintf(codefile, "void Code_numeric_ff_minus(RULE_INDEX_TYPE r)\n" "{\n");
i = 0;
r = 0;
for (sr = simplerule::SimpleRuleList; sr != NULL;
     sr = sr->NextSimpleRule) {
  if (sr->getclass() == rule::Simple && sr != error_rule) {
    if (i != 0)
	fprintf(codefile,
		"  if (r>=%lu && r<=%lu) { R%lu.Code_numeric_ff_minus(r-%lu); return; } \n",
		i, i + sr->getsize() - 1, r, i);
    else
	fprintf(codefile,
		"  if (r<=%lu) { R%lu.Code_numeric_ff_minus(r-%lu); return; } \n",
		i + sr->getsize() - 1, r, i);
    r++;
    i += sr->getsize();
  }
}
fprintf(codefile, "}\n");

}


// WP WP WP WP WP WP WP WP
if (ff_enabled){
fprintf(codefile, "mu_0_boolean* get_rule_clock_started(RULE_INDEX_TYPE r)\n" "{\n");
i = 0;
r = 0;
for (sr = simplerule::SimpleRuleList; sr != NULL;
     sr = sr->NextSimpleRule) {
  if (sr->getclass() == rule::Simple && sr != error_rule) {
    if (i != 0)
	fprintf(codefile,
		"  if (r>=%lu && r<=%lu) { return R%lu.get_rule_clock_started(r-%lu); } \n",
		i, i + sr->getsize() - 1, r, i);
    else
	fprintf(codefile,
		"  if (r<=%lu) { return R%lu.get_rule_clock_started(r-%lu); } \n",
		i + sr->getsize() - 1, r, i);
    r++;
    i += sr->getsize();
  }
}
fprintf(codefile, "}\n");

}

// WP WP WP WP WP WP WP
//if (ff_enabled){
fprintf(codefile, "std::map<mu_0_boolean*, mu__real*> get_clocks(RULE_INDEX_TYPE r)\n" "{\n");
i = 0;
r = 0;
for (sr = simplerule::SimpleRuleList; sr != NULL;
     sr = sr->NextSimpleRule) {
  if (sr->getclass() == rule::Simple && sr != error_rule) {
    if (i != 0)
	fprintf(codefile,
		"  if (r>=%lu && r<=%lu) { return R%lu.get_clocks(r-%lu); } \n",
		i, i + sr->getsize() - 1, r, i);
    else
	fprintf(codefile,
		"  if (r<=%lu) { return R%lu.get_clocks(r-%lu); } \n",
		i + sr->getsize() - 1, r, i);
    r++;
    i += sr->getsize();
  }
}
fprintf(codefile, "}\n");

//}






  // generate Priority, added by Uli
  fprintf(codefile, "int Priority(RULE_INDEX_TYPE r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      if (i != 0)
        fprintf(codefile,
                "  if (r>=%lu && r<=%lu) { return R%lu.Priority(); } \n",
                i, i + sr->getsize() - 1, r);
      else
        fprintf(codefile,
                "  if (r<=%lu) { return R%lu.Priority(); } \n",
                i + sr->getsize() - 1, r);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile, "}\n");

  // generate Name(r)
  fprintf(codefile, "char * Name(RULE_INDEX_TYPE r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      if (i != 0)
        fprintf(codefile,
                "  if (r>=%lu && r<=%lu) return R%lu.Name(r-%lu);\n",
                i, i + sr->getsize() - 1, r, i);
      else
        fprintf(codefile,
                "  if (r<=%lu) return R%lu.Name(r-%lu);\n",
                i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile, "  return NULL;\n");	// added by Uli
  fprintf(codefile, "}\n");

  // generate Duration(r)
  fprintf(codefile,
          "int Duration(RULE_INDEX_TYPE r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      if (i != 0)
        fprintf(codefile,
                "  if (r>=%lu && r<=%lu) return R%lu.Duration(r-%lu);\n",
                i, i + sr->getsize() - 1, r, i);
      else
        fprintf(codefile,
                "  if (r<=%lu) return R%lu.Duration(r-%lu);\n",
                i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile,
          "Error.Notrace(\"Internal: NextStateGenerator -- querying duration for nonexisting rule.\");\n"
          "}\n");

// generate Weight(r)
  fprintf(codefile,
          "int Weight(RULE_INDEX_TYPE r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      if (i != 0)
        fprintf(codefile,
                "  if (r>=%lu && r<=%lu) return R%lu.Weight(r-%lu);\n",
                i, i + sr->getsize() - 1, r, i);
      else
        fprintf(codefile,
                "  if (r<=%lu) return R%lu.Weight(r-%lu);\n",
                i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile,
          "Error.Notrace(\"Internal: NextStateGenerator -- querying duration for nonexisting rule.\");\n"
          "}\n");


// generate PDDLName(r)
  fprintf(codefile, " char * PDDLName(RULE_INDEX_TYPE r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      if (i != 0)
        fprintf(codefile,
                "  if (r>=%lu && r<=%lu) return R%lu.PDDLName(r-%lu);\n",
                i, i + sr->getsize() - 1, r, i);
      else
        fprintf(codefile,
                "  if (r<=%lu) return R%lu.PDDLName(r-%lu);\n",
                i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile, "  return NULL;\n");
  fprintf(codefile, "}\n");

  // generate PDDLClass(r)
  fprintf(codefile,
          "RuleManager::rule_pddlclass PDDLClass(RULE_INDEX_TYPE r)\n" "{\n");
  i = 0;
  r = 0;
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Simple && sr != error_rule) {
      if (i != 0)
        fprintf(codefile,
                "  if (r>=%lu && r<=%lu) return R%lu.PDDLClass(r-%lu);\n",
                i, i + sr->getsize() - 1, r, i);
      else
        fprintf(codefile,
                "  if (r<=%lu) return R%lu.PDDLClass(r-%lu);\n",
                i + sr->getsize() - 1, r, i);
      r++;
      i += sr->getsize();
    }
  }
  fprintf(codefile,
          "Error.Notrace(\"Internal: NextStateGenerator -- querying PDDL class for nonexisting rule.\");\n"
          "}\n");

  fprintf(codefile, "};\n");
  return i;
}

/******************************
  The main program
 ******************************/

const char *program::generate_code()
{
  RULE_INDEX_TYPE count;
  int messagecount=0;

  // header
  fprintf(codefile, "/******************************\n");
  fprintf(codefile, "  Program \"%s\" compiled by \"%s\"\n",
          get_mfilename(), DINO_VERSION);
  fprintf(codefile, "\n");
  fprintf(codefile, "  DiNo Last Compiled date: \"%s\"\n", __DATE__);
  fprintf(codefile, " ******************************/\n");
  fprintf(codefile, "\n");

  // record the date of the compiler.
  fprintf(codefile, "/********************\n  Parameter\n");
  fprintf(codefile, " ********************/\n");

  fprintf(codefile, "#define DINO_VERSION \"%s\"\n", DINO_VERSION);

  fprintf(codefile, "#define MURPHI_DATE \"%s\"\n", __DATE__);
  fprintf(codefile, "#define PROTOCOL_NAME \"");	// added by Uli
  for (int i = 0, l = strlen(get_mfilename())-2; i < l; i++)
    fputc(get_mfilename()[i], codefile);
  fputs("\"\n", codefile);
  //gdp
  if (domain_name!=NULL) fprintf(codefile, "#define DOMAIN_NAME \"%s\"\n", domain_name);
  if (get_pddldomainfilename()!=NULL) fprintf(codefile, "#define DOMAIN_FILENAME \"%s\"\n", get_pddldomainfilename());
  else if (domain_filename!=NULL) fprintf(codefile, "#define DOMAIN_FILENAME \"%s\"\n", domain_filename);
  if (problem_name!=NULL) fprintf(codefile, "#define PROBLEM_NAME \"%s\"\n", problem_name);
  if (get_pddlproblemfilename()!=NULL) fprintf(codefile, "#define PROBLEM_FILENAME \"%s\"\n", get_pddlproblemfilename());
  else if (problem_filename!=NULL) fprintf(codefile, "#define PROBLEM_FILENAME \"%s\"\n", problem_filename);
  fprintf(codefile, "#define DISCRETIZATION %lf\n", discretization);
#if defined(VAL_PATHNAME)
  fprintf(codefile, "#define VAL_PATHNAME \"%s\"\n", QUOTED_VALUE(VAL_PATHNAME));
#endif  
  fprintf(codefile, "#define BITS_IN_WORLD %d\n", bits_in_world);
  if (args->no_compression)
    fprintf(codefile, "#define ALIGN\n");
  if (args->hash_compression)
    fprintf(codefile, "#define HASHC\n");
//UPMURPHI_BEGIN
  if (args->UPMurphi_planner)
    fprintf(codefile, "#define UPMURPHI_PLANNER\n");
  if (args->UPMurphi_disk)
    fprintf(codefile, "#define UPMURPHI_DISK_EXTENSIONS\n");
  if (args->dynamic_debug)
    fprintf(codefile, "#define DYNDBG\n");
  if (args->variable_weight)
    fprintf(codefile, "#define VARIABLE_WEIGHT\n");
  if (args->variable_duration)
    fprintf(codefile, "#define VARIABLE_DURATION\n");
  if (theprog->clock_var_name!=NULL)
    fprintf(codefile, "#define HAS_CLOCK\n");
  fprintf(codefile, "const char * const modelmessages[] = { ");
  while(messagelist!=NULL) {
    fprintf(codefile, "\"%s\"%c",messagelist->message,(messagelist->next!=NULL?',':' '));
    messagelist = messagelist->next;
    messagecount++;
  }
  fprintf(codefile, "};\n");
  fprintf(codefile, "const int modelmessagecount = %d;\n",messagecount);


//UPMURPHI_END
  fprintf(codefile, "\n");

  // include prolog
  fprintf(codefile, "/********************\n  Include\n");
  fprintf(codefile, " ********************/\n");
  fprintf(codefile, "#include \"upm_prolog.hpp\"\n");

  /* generate dependent stuff. */
  fprintf(codefile, "\n/********************\n  Decl declaration\n");
  fprintf(codefile, " ********************/\n\n");
  // typedecl declaration -- added to fixed a bug
  if (typedecl::origin != NULL)
    typedecl::origin->generate_all_decl();
  // globals->generate_decls() gets done by procedures->generate_decls
  procedures->generate_decls();
  fprintf(codefile, "\n\n\n");

  // generate the world
  fprintf(codefile, "\n/********************\n  The world\n");
  fprintf(codefile, " ********************/\n");
  make_world(globals);
  fprintf(codefile, "\n");

  // generate the codes for each rule
  fprintf(codefile, "\n/********************\n  Rule declarations\n");
  fprintf(codefile, " ********************/\n");

//   for ( rule *r = rules;
//  r != NULL;
//  r = r->next )
//     r->generate_code();
//   fprintf(codefile,"\n\n\n");
//
//   /* generate the lists of rules, startstate, and invariants. */
//   // generate the lists of rules
//   fprintf(codefile,"\n/********************\n  Rule records\n");
//   fprintf(codefile," ********************/\n");
//   fprintf(codefile, "const rulerec rules[] = {\n");
//   count = rulelist->print_rules();
//   fprintf(codefile,"};\n");

  count = generate_ruleset();
  fprintf(codefile, "const RULE_INDEX_TYPE numrules = %lu;\n", count);

  fprintf(codefile, "\n/********************\n  parameter\n");
  fprintf(codefile, " ********************/\n");
  fprintf(codefile, "#define RULES_IN_WORLD %lu\n", count);
  fprintf(codefile, "\n");

  // generate the lists of startstate
  fprintf(codefile, "\n/********************\n  Startstate records\n");
  fprintf(codefile, " ********************/\n");

//  fprintf(codefile, "const rulerec startstates[] = {\n");
//  count = startstatelist->print_rules();
//  fprintf(codefile,"};\n");
  count = generate_startstates();
  fprintf(codefile, "const rulerec startstates[] = {\n");
  simplerule *sr;		/* IM: this is to have probabilities on startstates */
  for (sr = simplerule::SimpleRuleList; sr != NULL;
       sr = sr->NextSimpleRule) {
    if (sr->getclass() == rule::Startstate) {
      fprintf(codefile, "{ NULL, NULL, NULL, FALSE},\n");
    }

  }
  fprintf(codefile, "};\n");
  fprintf(codefile,
          "unsigned short StartStateManager::numstartstates = %lu;\n",			// WP WP WP WP WP CHANGED %d TO %lu TO REFLECT THE CORRECT VARIABLE TYPE
          count);

//UPMURPHI_BEGIN
  // generate the lists of goals
  //fprintf(codefile, "\n#ifndef UPMURPHI_PLANNER\n");
  fprintf(codefile,"\n/********************\n  Goal records\n");
  fprintf(codefile," ********************/\n");
  (void) generate_Goals();
  fprintf (codefile, "const rulerec goals[] = {\n");
  count = goallist->print_rules ();
  fprintf (codefile, "};\n");
  fprintf (codefile, "const unsigned short numgoals = %lu;\n", count);			// WP WP WP WP WP CHANGED %d TO %lu TO REFLECT THE CORRECT VARIABLE TYPE
  //fprintf(codefile, "\n#endif\n");
//UPMURPHI_END


//UPMURPHI_BEGIN
  // generate the lists of goals
  //fprintf(codefile, "\n#ifndef UPMURPHI_PLANNER\n");
  fprintf(codefile,"\n/********************\n  Metric related stuff\n");
  fprintf(codefile," ********************/\n");
  fprintf (codefile, "const short metric = %d;\n", theprog->metric);
  //fprintf(codefile, "\n#endif\n");
//UPMURPHI_END



  // generate the lists of invariants
  fprintf(codefile, "\n/********************\n  Invariant records\n");
  fprintf(codefile, " ********************/\n");
  (void) generate_invariants();
  fprintf(codefile, "const rulerec invariants[] = {\n");
  count = invariantlist->print_rules();
  fprintf(codefile, "};\n");
  fprintf(codefile, "const unsigned short numinvariants = %lu;\n", count);		// WP WP WP WP WP CHANGED %d TO %lu TO REFLECT THE CORRECT VARIABLE TYPE


  // generate normalization/canonicalization function for scalarset
  fprintf(codefile,
          "\n/********************\n  Normal/Canonicalization for scalarset\n");
  fprintf(codefile, " ********************/\n");
  symmetry.generate_code(globals);

  // include prolog
  fprintf(codefile, "\n/********************\n  Include\n");
  fprintf(codefile, " ********************/\n");

  /* I\'ve decided to just #include it, so that I can foist off the problem
   * of finding the file to be included on the c++ compiler. */
  /* But the filename still should not be hardcoded. */
  fprintf(codefile, "#include \"upm_epilog.hpp\"\n");

  return "ERROR!";
}

