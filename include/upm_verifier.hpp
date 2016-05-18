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
  Include library
 ****************************************/
#ifndef MU_VERIFIER_H
#define MU_VERIFIER_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
//#include <values.h> /* for BITSPERBYTE and BITS() */
// Uli: values.h is obsolete, replaced by limits.h
#include <limits.h>
#include <new>    /* for new_handler stuff. */
#include <signal.h> /* To trap division by zero. */
#include <assert.h>
#include <float.h>  // AP: included for FP information
#include <math.h>	//IM: for floating point functions
#include <regex.h> // standard library for regular expression manipulation

using namespace std;

/****************************************   // added by Uli
  C Objects
 ****************************************/

extern "C" int remove(const char *filename);

// GDP, ET : Needed for Red Hat 7.1
#ifndef SYSCONF_RDCL
extern "C" long sysconf(int name);
#endif


/****************************************
  General Constants
 ****************************************/

#define FALSE 0
#define TRUE 1
#define mu_false 0
#define mu_true 1
#define BIT_BLOCK unsigned char /* type of an element for a bit vector. */

// Uli: replace the BITS() macro from values.h
#ifndef BITS
#define BITS(type) ((int)sizeof (type) * CHAR_BIT)
#endif

//typedef char bool;
typedef void (*proc) (void);
typedef bool (*boolfunc) (void);


struct rulerec {
  char *name;
  boolfunc condition;
  proc code;
  bool unfair;
};

#ifndef ALIGN
typedef struct {
  int longoffset;   /* offset in the state vector, on long boundary */
  unsigned int mask1, mask2;  /* masks for fast access */
  int shift1, shift2;         /* shift values for fast access */
} position;
#endif

/****************************************
  For Liveness
 ****************************************/
enum space { PRE, LEFT, RIGHT };
enum live_type { E, AE, EA, U, AIE, AIU };
struct liverec {
  char *name;
  boolfunc condition_pre;
  boolfunc condition_left;
  boolfunc condition_right;
  live_type livetype;
};


/****************************************
  class declaration
 ****************************************/

// declared in mu_util.h
class mu__int;      /* a base for a value */
class mu_boolean;   /* a base for a boolean */
class world_class;  /* class for variables in expanded state */
class state_queue;  /* class for search queue for bfs */
class state_stack;  /* class for search queue for dfs */
class state_set;    /* hash table for storing state examined */

// declared in mu_io.h
class Error_handler;/* class for error handling */
class argclass;     /* class for handling command line argument */

// declared in here
class dynBitVec;    /* class for bit vector , state */

// declared in mu_dep.h
class state;        /* a state in the state graph -- a bit vector */
class state_L;      /* a state in the state graph for liveness -- with more info */
class setofrules;   /* class to store a set of rules */
class sleepset;     /* sleepset for partial order reduction technique */
class rule_matrix;  /* class for square matrix of dimension numrules */


//CTRL classes definition for controller generation
class listPredecessor;
class listPredecessor_Head;
//class tablePredecessor;
class EndState_queue;
class predecessor_queue;
class ctrl;
class block_Ram;
class transition;
/*******************************************************************/

/****************************************
  external variables
 ****************************************/

// from murphi code
// extern const rulerec rules[];
extern const RULE_INDEX_TYPE numrules;   // Uli: unsigned short -> unsigned
extern const rulerec startstates[];
extern const unsigned short numstartstates;
extern const unsigned short numgoals;
extern const rulerec goals[];
extern const rulerec invariants[];
extern const unsigned short numinvariants;
extern const rulerec fairnesses[];
extern const unsigned short numfairnesses;
extern const liverec livenesses[];
extern const unsigned short numlivenesses;

#define STARTSTATE 0
#define CONDITION 1
#define RULE 2
#define INVARIANT 3

/*CTRL definizione di #define ENDSTATE 3*/
#define GOAL 3


class StartStateManager;

class RuleManager;
class PropertyManager;
class StateManager;
class SymmetryManager;
class POManager;
class ReportManager;
class AlgorithmManager;
class StorageManager;
class StatsManager;
class OutputManager;

extern StartStateManager *StartState;  // manager for all startstate related operation


extern RuleManager *Rules;             // manager for all rule related operation
extern PropertyManager *Properties;    // manager for all property related operation
extern StateManager *StateSet;         // manager for all state related information
extern SymmetryManager *Symmetry;      // manager for all symmetry information
extern POManager *PO;                  // manager for all symmetry information
extern ReportManager *Reporter;        // manager for all diagnostic messages
extern AlgorithmManager *Algorithm;    // manager for all algorithm related issue
extern StorageManager *Storage;
extern StatsManager *Stats;
extern OutputManager *Output;


extern Error_handler Error;       // general error handler.
extern argclass *args;            // the record of the arguments.
extern state *curstate;        // current state at the beginning of the rule-firing
extern state *const workingstate;   // Uli: this pointer points to a working-
//      buffer
extern world_class theworld;          // the set of global variables.
extern int category;                  // working on startstate, rule or invariant


#endif
