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

void
symmetryclass::generate_symmetry_function_decl()
{
  fprintf(codefile,
          "  virtual void Permute(PermSet& Perm, int i);\n"
          "  virtual void SimpleCanonicalize(PermSet& Perm);\n"
          "  virtual void Canonicalize(PermSet& Perm);\n"
          "  virtual void SimpleLimit(PermSet& Perm);\n"
          "  virtual void ArrayLimit(PermSet& Perm);\n"
          "  virtual void Limit(PermSet& Perm);\n"
          "  virtual void MultisetLimit(PermSet& Perm);\n");
}

/********************
  main procedure to generate code for symmetry
  ********************/

void symmetryclass::generate_code(ste * globals)
{
  stelist *var;

  // output comment on overall scalarset types on all highest level variables
  // not necessary, just to help you analyse your variables
  fprintf(codefile, "/*\n");
  set_var_list(globals);
  for (var = varlist; var != NULL; var = var->next)
    fprintf(codefile, "%s:%s\n",
            var->s->getname()->getname(),
            var->s->getvalue()->gettype()->structurestring()
           );
  fprintf(codefile, "*/\n");

  // classify scalarset types as useful or useless
  // it is not useful if
  //    1) it was not given an explicit name
  //    2) it is used in one of the state variables
  for (var = varlist; var != NULL; var = var->next)
    var->s->getvalue()->gettype()->setusefulness();

  // remove useless scalarset from scalarset list stored in symmetryclass
  stelist **entry = &scalarsetlist;
  stelist *member;
  for (member = scalarsetlist; member != NULL; member = member->next)
    if (!((scalarsettypedecl *) member->s->getvalue())->isuseless()) {
      *entry = member;
      entry = &(member->next);
    }
  *entry = NULL;

  // generate symmetry code
  generate_symmetry_code();
}

/********************
  generate canonicalization algorithm
  ********************/

void symmetryclass::generate_symmetry_code()
{
  fprintf(codefile,
          "\n/********************\n"
          "Code for symmetry\n" " ********************/\n");

  setup();
  generate_permutation_class();
  generate_symmetry_class();

  fprintf(codefile,
          "\n/********************\n"
          " Permute and Canonicalize function for different types\n"
          " ********************/\n");
  if (typedecl::origin != NULL)
    typedecl::origin->generate_all_symmetry_decl(*this);

  generate_match_function();

  generate_exhaustive_fast_canonicalization();
  generate_heuristic_fast_canonicalization();
  generate_heuristic_small_mem_canonicalization();
  generate_heuristic_fast_normalization();
}

/********************
  generate symmetry class
  ********************/
void symmetryclass::generate_symmetry_class()
{
  fprintf(codefile,
          "\n/********************\n"
          " Symmetry Class\n" " ********************/\n");
  fprintf(codefile,
          "class SymmetryClass\n"
          "{\n"
          "  PermSet Perm;\n"
          "  bool BestInitialized;\n"
          "  state BestPermutedState;\n"
          "\n"
          "  // utilities\n"
          "  void SetBestResult(int i, state* temp);\n"
          "  void ResetBestResult() {BestInitialized = FALSE;};\n"
          "\n"
          "public:\n"
          "  // initializer\n"
          "  SymmetryClass() : Perm(), BestInitialized(FALSE) {};\n"
          "  ~SymmetryClass() {};\n"
          "\n"
          "  void Normalize(state* s);\n"
          "\n"
          "  void Exhaustive_Fast_Canonicalize(state *s);\n"
          "  void Heuristic_Fast_Canonicalize(state *s);\n"
          "  void Heuristic_Small_Mem_Canonicalize(state *s);\n"
          "  void Heuristic_Fast_Normalize(state *s);\n"
          "\n" "  void MultisetSort(state* s);\n" "};\n" "\n");

  fprintf(codefile,
          "\n/********************\n"
          " Symmetry Class Members\n" " ********************/\n");
  fprintf(codefile, "void SymmetryClass::MultisetSort(state* s)\n" "{\n");
  for (stelist * var = varlist; var != NULL; var = var->next)
    fprintf(codefile,
            "        %s.MultisetSort();\n", var->s->getvalue()->mu_name);
  fprintf(codefile, "}\n");

  fprintf(codefile,
          "void SymmetryClass::Normalize(state* s)\n"
          "{\n"
          "  switch (args->sym_alg.mode) {\n"
          "  case argsym_alg::Exhaustive_Fast_Canonicalize:\n"
          "    Exhaustive_Fast_Canonicalize(s);\n"
          "    break;\n"
          "  case argsym_alg::Heuristic_Fast_Canonicalize:\n"
          "    Heuristic_Fast_Canonicalize(s);\n"
          "    break;\n"
          "  case argsym_alg::Heuristic_Small_Mem_Canonicalize:\n"
          "    Heuristic_Small_Mem_Canonicalize(s);\n"
          "    break;\n"
          "  case argsym_alg::Heuristic_Fast_Normalize:\n"
          "    Heuristic_Fast_Normalize(s);\n"
          "    break;\n"
          "  default:\n"
          "    Heuristic_Fast_Canonicalize(s);\n" "  }\n" "}\n");
}

/********************
  generate old algorithm I :
  simple exhaustive canonicalization
  ********************/

void symmetryclass::generate_match_function()	// changes by Uli
{
  stelist *var;

  fprintf(codefile,
          "\n/********************\n"
          " Auxiliary function for error trace printing\n"
          " ********************/\n");

  fprintf(codefile,
          "bool match(state* ns, StatePtr p)\n"
          "{\n"
          "  unsigned int i;\n"
          "  static PermSet Perm;\n"
          "  static state temp;\n" "  StateCopy(&temp, ns);\n");

  fprintf(codefile, "  if (args->symmetry_reduction.value)\n" "    {\n");

  // ********************
  // matching by permuting
  // ********************
  if (no_need_for_perm)
    fprintf(codefile,
            "      if (  args->sym_alg.mode == argsym_alg::Exhaustive_Fast_Canonicalize) {\n");
  else
    fprintf(codefile,
            "      if (  args->sym_alg.mode == argsym_alg::Exhaustive_Fast_Canonicalize\n"
            "         || args->sym_alg.mode == argsym_alg::Heuristic_Fast_Canonicalize) {\n");

  // matching by explicit data
  fprintf(codefile,
          "        Perm.ResetToExplicit();\n"
          "        for (i=0; i<Perm.count; i++)\n"
          "          if (Perm.In(i))\n"
          "            {\n"
          "              if (ns != workingstate)\n"
          "                  StateCopy(workingstate, ns);\n"
          "              \n");
  for (var = varlist; var != NULL; var = var->next)
    fprintf(codefile,
            "              %s.Permute(Perm,i);\n"
            "              if (args->multiset_reduction.value)\n"
            "                %s.MultisetSort();\n",
            var->s->getvalue()->mu_name, var->s->getvalue()->mu_name);
  fprintf(codefile, "            if (p.compare(workingstate)) {\n"
          // changed by Uli
          "              StateCopy(workingstate,&temp); return TRUE; }\n"
          "          }\n"
          "        StateCopy(workingstate,&temp);\n"
          "        return FALSE;\n" "      }\n");

  // matching by generating the permutation one by one
  fprintf(codefile,
          "      else {\n"
          "        Perm.ResetToSimple();\n"
          "        Perm.SimpleToOne();\n");

  // first iteration -- may be skipped?
  fprintf(codefile,
          "        if (ns != workingstate)\n"
          "          StateCopy(workingstate, ns);\n" "\n");
  for (var = varlist; var != NULL; var = var->next)
    fprintf(codefile,
            "          %s.Permute(Perm,0);\n"
            "          if (args->multiset_reduction.value)\n"
            "            %s.MultisetSort();\n",
            var->s->getvalue()->mu_name, var->s->getvalue()->mu_name);
  fprintf(codefile, "        if (p.compare(workingstate)) {\n"	// changed by Uli
          "          StateCopy(workingstate,&temp); return TRUE; }\n"
          "\n");

  // all other iterations
  fprintf(codefile,
          "        while (Perm.NextPermutation())\n"
          "          {\n"
          "            if (ns != workingstate)\n"
          "              StateCopy(workingstate, ns);\n"
          "              \n");
  for (var = varlist; var != NULL; var = var->next)
    fprintf(codefile,
            "              %s.Permute(Perm,0);\n"
            "              if (args->multiset_reduction.value)\n"
            "                %s.MultisetSort();\n",
            var->s->getvalue()->mu_name, var->s->getvalue()->mu_name);
  fprintf(codefile, "            if (p.compare(workingstate)) {\n"	// changed by Uli
          "              StateCopy(workingstate,&temp); return TRUE; }\n"
          "          }\n"
          "        StateCopy(workingstate,&temp);\n"
          "        return FALSE;\n" "      }\n");

  // end matching by permuting
  fprintf(codefile, "    }\n");

  // matching by multisesort
  fprintf(codefile,
          "  if (!args->symmetry_reduction.value\n"
          "      && args->multiset_reduction.value)\n"
          "    {\n"
          "      if (ns != workingstate)\n"
          "          StateCopy(workingstate, ns);\n");
  for (var = varlist; var != NULL; var = var->next)
    fprintf(codefile,
            "      %s.MultisetSort();\n", var->s->getvalue()->mu_name);
  fprintf(codefile, "      if (p.compare(workingstate)) {\n"	// changed by Uli
          "        StateCopy(workingstate,&temp); return TRUE; }\n"
          "      StateCopy(workingstate,&temp);\n"
          "      return FALSE;\n" "    }\n");

  // matching by direct comparsion
  fprintf(codefile, "  return (p.compare(ns));\n"	// changed by Uli
          "}\n");
}

/********************
  generate old algorithm :
  fast exhaustive canonicalization
  ********************/

void symmetryclass::generate_exhaustive_fast_canonicalization()
{
  fprintf(codefile,
          "\n/********************\n"
          " Canonicalization by fast exhaustive generation of\n"
          " all permutations\n" " ********************/\n");

  fprintf(codefile,
          "void SymmetryClass::Exhaustive_Fast_Canonicalize(state* s)\n"
          "{\n"
          "  unsigned int i;\n"
          "  static state temp;\n" "  Perm.ResetToExplicit();\n" "\n");

  for (stelist * var = varlist; var != NULL; var = var->next)
    fprintf(codefile,
            "  StateCopy(&temp, workingstate);\n"
            "  ResetBestResult();\n"
            "  for (i=0; i<Perm.count; i++)\n"
            "    if (Perm.In(i))\n"
            "      {\n"
            "        StateCopy(workingstate, &temp);\n"
            "        %s.Permute(Perm,i);\n"
            "        if (args->multiset_reduction.value)\n"
            "          %s.MultisetSort();\n"
            "        SetBestResult(i, workingstate);\n"
            "      }\n"
            "  StateCopy(workingstate, &BestPermutedState);\n"
            "\n",
            var->s->getvalue()->mu_name, var->s->getvalue()->mu_name);
  fprintf(codefile, "};\n");
}

/********************
  generate normalization algorithm
  ********************/
void symmetryclass::generate_heuristic_fast_normalization()
{
  stelist *var;
  bool ToOne;

  fprintf(codefile,
          "\n/********************\n"
          " Normalization by fast simple variable canonicalization,\n"
          " fast simple scalarset array canonicalization,\n"
          " fast restriction on permutation set with simple scalarset array of scalarset,\n"
          " and for all other variables, pick any remaining permutation\n"
          " ********************/\n");

  fprintf(codefile,
          "void SymmetryClass::Heuristic_Fast_Normalize(state* s)\n"
          "{\n"
          "  int i;\n"
          "  static state temp;\n" "\n" "  Perm.ResetToSimple();\n" "\n");

  // simple variable
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetVariable()
        && (var->s->getvalue()->gettype()->getstructure() ==
            typedecl::ScalarsetArrayOfFree
            || var->s->getvalue()->gettype()->getstructure() ==
            typedecl::ScalarsetVariable
            || var->s->getvalue()->gettype()->getstructure() ==
            typedecl::MultisetOfFree))
      fprintf(codefile, "  %s.SimpleCanonicalize(Perm);\n" "\n",
              var->s->getvalue()->mu_name);


  // simple scalarset array
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetArrayOfFree()
        && (var->s->getvalue()->gettype()->getstructure() ==
            typedecl::ScalarsetArrayOfFree
            || var->s->getvalue()->gettype()->getstructure() ==
            typedecl::MultisetOfFree))
      fprintf(codefile, "  %s.Canonicalize(Perm);\n" "\n",
              var->s->getvalue()->mu_name);

  // multiset
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->getstructure() ==
        typedecl::MultisetOfFree)
      fprintf(codefile, "  %s.MultisetSort();\n" "\n",
              var->s->getvalue()->mu_name);

  // if they are inside more complicated structure, only do a limit
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetVariable()
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.SimpleLimit(Perm);\n" "  }\n" "\n",
              var->s->getvalue()->mu_name);
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetArrayOfFree()
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.ArrayLimit(Perm);\n" "  }\n" "\n",
              var->s->getvalue()->mu_name);

  // refine Permutation set by checking graph structure of scalarset array of Scalarset
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetArrayOfS())
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.Limit(Perm);\n"
              "  }\n" "\n", var->s->getvalue()->mu_name);

  // checking if we need to change simple to one
  ToOne = FALSE;
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->getstructure() !=
        typedecl::NoScalarset
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      ToOne = TRUE;
  if (ToOne)
    fprintf(codefile, "  Perm.SimpleToOne();\n" "\n");

  // others
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->getstructure() !=
        typedecl::NoScalarset
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      fprintf(codefile,
              "  %s.Permute(Perm,0);\n"
              "  if (args->multiset_reduction.value)\n"
              "    %s.MultisetSort();\n" "\n", var->s->getvalue()->mu_name,
              var->s->getvalue()->mu_name
			  /*,var->s->getvalue()->mu_name,var->s->getvalue()->mu_name*/ 		// WP WP WP WP WP COMMENTED OUT ARGUMENT NO LONGER USED
			  );

  fprintf(codefile, "};\n");
}

/********************
  generate new algorithm IV :
  fast canonicalization for simple variable and simple scalarset array
  and also fast restriction in permutation set with simple scalarset array of scalarset
  and fast restriction for multiset
  ********************/
void symmetryclass::generate_heuristic_fast_canonicalization()
{
  stelist *var;
  bool ToExplicit;

  fprintf(codefile,
          "\n/********************\n"
          " Canonicalization by fast simple variable canonicalization,\n"
          " fast simple scalarset array canonicalization,\n"
          " fast restriction on permutation set with simple scalarset array of scalarset,\n"
          " and fast exhaustive generation of\n"
          " all permutations for other variables\n"
          " ********************/\n");

  fprintf(codefile,
          "void SymmetryClass::Heuristic_Fast_Canonicalize(state* s)\n"
          "{\n"
          "  int i;\n"
          "  static state temp;\n" "\n" "  Perm.ResetToSimple();\n" "\n");

  // simple variable
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetVariable()
        && (var->s->getvalue()->gettype()->getstructure() ==
            typedecl::ScalarsetArrayOfFree
            || var->s->getvalue()->gettype()->getstructure() ==
            typedecl::ScalarsetVariable
            || var->s->getvalue()->gettype()->getstructure() ==
            typedecl::MultisetOfFree))
      fprintf(codefile, "  %s.SimpleCanonicalize(Perm);\n" "\n",
              var->s->getvalue()->mu_name);

  // simple scalarset array
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetArrayOfFree()
        && (var->s->getvalue()->gettype()->getstructure() ==
            typedecl::ScalarsetArrayOfFree
            || var->s->getvalue()->gettype()->getstructure() ==
            typedecl::MultisetOfFree))
      fprintf(codefile, "  %s.Canonicalize(Perm);\n" "\n",
              var->s->getvalue()->mu_name);

  // multiset
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->getstructure() ==
        typedecl::MultisetOfFree)
      fprintf(codefile, "  %s.MultisetSort();\n" "\n",
              var->s->getvalue()->mu_name);

  // if they are inside more complicated structure, only do a limit
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetVariable()
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.SimpleLimit(Perm);\n" "  }\n" "\n",
              var->s->getvalue()->mu_name);
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetArrayOfFree()
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.ArrayLimit(Perm);\n" "  }\n" "\n",
              var->s->getvalue()->mu_name);

  // refine Permutation set by checking graph structure of multiset array of Scalarset
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasMultisetOfScalarset())
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.MultisetLimit(Perm);\n"
              "  }\n" "\n", var->s->getvalue()->mu_name);

  // refine Permutation set by checking graph structure of scalarset array of Scalarset
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetArrayOfS())
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.Limit(Perm);\n"
              "  }\n" "\n", var->s->getvalue()->mu_name);

  // checking if we need to change simple to explicit
  if (!no_need_for_perm)
    fprintf(codefile, "  Perm.SimpleToExplicit();\n" "\n");

  // handle all other cases by explicit/exhaustive enumeration
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->getstructure() !=
        typedecl::NoScalarset
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      fprintf(codefile,
              "  StateCopy(&temp, workingstate);\n"
              "  ResetBestResult();\n" "  for (i=0; i<Perm.count; i++)\n"
              "    if (Perm.In(i))\n" "      {\n"
              "        StateCopy(workingstate, &temp);\n"
              "        %s.Permute(Perm,i);\n"
              "        if (args->multiset_reduction.value)\n"
              "          %s.MultisetSort();\n"
              "        SetBestResult(i, workingstate);\n" "      }\n"
              "  StateCopy(workingstate, &BestPermutedState);\n" "\n",
              var->s->getvalue()->mu_name, var->s->getvalue()->mu_name);

  fprintf(codefile, "};\n");
}

/********************
  generate new algorithm V :
  fast canonicalization for simple variable and simple scalarset array
  and also fast restriction in permutation set with simple scalarset array of scalarset
  and fast restriction for multiset
  -- use less local memory
  ********************/
void symmetryclass::generate_heuristic_small_mem_canonicalization()
{
  stelist *var;
  bool ToExplicit;

  fprintf(codefile,
          "\n/********************\n"
          " Canonicalization by fast simple variable canonicalization,\n"
          " fast simple scalarset array canonicalization,\n"
          " fast restriction on permutation set with simple scalarset array of scalarset,\n"
          " and fast exhaustive generation of\n"
          " all permutations for other variables\n"
          " and use less local memory\n" " ********************/\n");

  fprintf(codefile,
          "void SymmetryClass::Heuristic_Small_Mem_Canonicalize(state* s)\n"
          "{\n"
          "  unsigned long cycle;\n"
          "  static state temp;\n" "\n" "  Perm.ResetToSimple();\n" "\n");

  // simple variable
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetVariable()
        && (var->s->getvalue()->gettype()->getstructure() ==
            typedecl::ScalarsetArrayOfFree
            || var->s->getvalue()->gettype()->getstructure() ==
            typedecl::ScalarsetVariable
            || var->s->getvalue()->gettype()->getstructure() ==
            typedecl::MultisetOfFree))
      fprintf(codefile, "  %s.SimpleCanonicalize(Perm);\n" "\n",
              var->s->getvalue()->mu_name);

  // simple scalarset array
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetArrayOfFree()
        && (var->s->getvalue()->gettype()->getstructure() ==
            typedecl::ScalarsetArrayOfFree
            || var->s->getvalue()->gettype()->getstructure() ==
            typedecl::MultisetOfFree))
      fprintf(codefile, "  %s.Canonicalize(Perm);\n" "\n",
              var->s->getvalue()->mu_name);

  // multiset
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->getstructure() ==
        typedecl::MultisetOfFree)
      fprintf(codefile, "  %s.MultisetSort();\n" "\n",
              var->s->getvalue()->mu_name);

  // if they are inside more complicated structure, only do a limit
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetVariable()
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.SimpleLimit(Perm);\n" "  }\n" "\n",
              var->s->getvalue()->mu_name);
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetArrayOfFree()
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.ArrayLimit(Perm);\n" "  }\n" "\n",
              var->s->getvalue()->mu_name);

  // refine Permutation set by checking graph structure of multiset array of Scalarset
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasMultisetOfScalarset())
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.MultisetLimit(Perm);\n"
              "  }\n" "\n", var->s->getvalue()->mu_name);

  // refine Permutation set by checking graph structure of scalarset array of Scalarset
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->HasScalarsetArrayOfS())
      fprintf(codefile,
              "  if (Perm.MoreThanOneRemain()) {\n"
              "    %s.Limit(Perm);\n"
              "  }\n" "\n", var->s->getvalue()->mu_name);

  // checking if we need to change simple to explicit
  ToExplicit = FALSE;
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->getstructure() !=
        typedecl::NoScalarset
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      ToExplicit = TRUE;
  if (ToExplicit)
    fprintf(codefile,
            "  Perm.SimpleToOne();\n"
            "\n"
            "  StateCopy(&temp, workingstate);\n"
            "  ResetBestResult();\n");

  // handle all other cases by explicit/exhaustive enumeration
  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->getstructure() !=
        typedecl::NoScalarset
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      fprintf(codefile,
              "  %s.Permute(Perm,0);\n"
              "  if (args->multiset_reduction.value)\n"
              "    %s.MultisetSort();\n", var->s->getvalue()->mu_name,
              var->s->getvalue()->mu_name);

  if (!no_need_for_perm)
    fprintf(codefile,
            "  BestPermutedState = *workingstate;\n"
            "  BestInitialized = TRUE;\n"
            "\n"
            "  cycle=1;\n"
            "  while (Perm.NextPermutation())\n"
            "    {\n"
            "      if (args->perm_limit.value != 0\n"
            "          && cycle++ >= args->perm_limit.value) break;\n"
            "      StateCopy(workingstate, &temp);\n");

  for (var = varlist; var != NULL; var = var->next)
    if (var->s->getvalue()->gettype()->getstructure() !=
        typedecl::NoScalarset
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetVariable
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::ScalarsetArrayOfFree
        && var->s->getvalue()->gettype()->getstructure() !=
        typedecl::MultisetOfFree)
      fprintf(codefile,
              "      %s.Permute(Perm,0);\n"
              "      if (args->multiset_reduction.value)\n"
              "        %s.MultisetSort();\n", var->s->getvalue()->mu_name,
              var->s->getvalue()->mu_name);

  if (!no_need_for_perm)
    fprintf(codefile,
            "      switch (StateCmp(workingstate, &BestPermutedState)) {\n"
            "      case -1:\n"
            "        BestPermutedState = *workingstate;\n"
            "        break;\n"
            "      case 1:\n"
            "        break;\n"
            "      case 0:\n"
            "        break;\n"
            "      default:\n"
            "        Error.Error(\"funny return value from StateCmp\");\n"
            "      }\n"
            "    }\n"
            "  StateCopy(workingstate, &BestPermutedState);\n" "\n");
  fprintf(codefile, "};\n");
}

