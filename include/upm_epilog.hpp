/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

/****************************************
  header that depend on constants
  which is generated in the middle of the compiled program:
  RULES_IN_WORLD
  ****************************************/
#include "upm_util_dep.hpp"

/****************************************
  supporting routines
  ****************************************/
#ifdef HASHC
#include "upm_hash.cpp"
#endif

#include "upm_util.cpp"
#include "upm_io.cpp"
#include "upm_sym.cpp"
#include "upm_state.cpp"
#include "upm_graph.cpp"
#include "upm_storage.cpp"
#include "upm_system.cpp"
//#include "upm_rpg.cpp"
//#include "upm_numeric_rpg2.cpp"
#include "upm_staged_rpg.cpp"
#include <iostream>
#include <string>
#include <set>
#include <list>

/****************************************
  real numbers extension
  ****************************************/
#include "upm_real.cpp"	//im

/****************************************
  main routines
  ****************************************/

#include "upm_verifier.cpp"
