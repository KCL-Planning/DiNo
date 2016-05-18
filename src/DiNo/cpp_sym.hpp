/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/


#ifndef __CPP_SYM_H__
#define __CPP_SYM_H__

#define MAX_NUM_SCALARSET 512
#define MAX_NUM_GLOBALS 512
#define TRUNCATE_LIMIT_FOR_LIMIT_FUNCTION 1

class symmetryclass
{
  stelist *scalarsetlist;
  stelist *varlist;
  int num_scalarset;
  unsigned long size_of_set;
  int size[MAX_NUM_SCALARSET];
  unsigned long factorialsize[MAX_NUM_SCALARSET];

  bool no_need_for_perm;

  unsigned long factorial(int n);

  void setup();
  void generate_set_class();
  void generate_permutation_class();
  void generate_symmetry_class();

  void generate_exhaustive_fast_canonicalization();
  void generate_heuristic_fast_canonicalization();
  void generate_heuristic_small_mem_canonicalization();
  void generate_heuristic_fast_normalization();

  void generate_symmetry_code();
  void generate_match_function();

  void set_var_list(ste * globals);

 public:
  symmetryclass():scalarsetlist(NULL), varlist(NULL),
    no_need_for_perm(TRUE)
  {
  };
  void add(ste * s)
  {
    scalarsetlist = new stelist(s, scalarsetlist);
  };
  int getsize()
  {
    return size_of_set;
  }

  void generate_code(ste * globals);
  void generate_symmetry_function_decl();
};

bool is_simple_scalarset(typedecl * t);

class charlist
{
 public:
  char *string;
  charlist *next;
  typedecl *e;

  charlist(char *str, charlist * nxt):string(str), next(nxt)
  {
  } charlist(char *str, typedecl * e, charlist * nxt):string(str),
    next(nxt), e(e)
  {
  }
  virtual ~ charlist()
  {
    delete string;
    delete next;
  }

};

#endif
