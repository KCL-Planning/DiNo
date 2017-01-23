/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#if 0
#define MEMTRACKALLOC fprintf(stderr,"---- MEMORY ALLOCATION: %s, %d\n",__FILE__, __LINE__);
#define DEBUGMARK fprintf(stderr,"---- HIGHLIGHT POINT HIT: %s, %d\n",__FILE__, __LINE__);
#define PAUSE {char dummy; cin >> dummy;}
#define BREAKPOINT(message) {cerr << endl << message; char dummy; cin >> dummy;}
#define LOG(message) {cerr << endl << __FILE__  << ":" << __LINE__ << ": " << message;}
#else
#define MEMTRACKALLOC
#define DEBUGMARK
#define PAUSE
#define BREAKPOINT(message)
#define LOG(message)
#endif

/*  Define this macro for pre 2.x G++ versions.
    It controls whether delete has a size argument,
    which is required by old g++ and generates a
    warning in new g++ compilers */

#ifdef OLDGPP
// g++ 1.x
#define OLD_GPP(arg) arg
#else
// g++ 2.x
#define OLD_GPP(arg)
#endif

/****************************************
  Default Value or Constant that can be set by user
 ****************************************/

/* number of times you can go around a while loop. */
#define DEF_LOOPMAX     1000

/* Default memory for Unix. */
//#define DEFAULT_MEM     8000000
#define DEFAULT_MEM (1000 * 0x100000L) //gdp: approx 1Gb

//#define DEFAULT_MEM_CTRL 8000000

// Uli: default number of bits for hash compaction and
//      suffix of trace info file
#ifdef HASHC
#define DEFAULT_BITS   40
#define TRACE_FILE     ".trace"
#endif

/* Default Maximum number of error to search when -finderrors is used */
#define DEFAULT_MAX_ERRORS 100

/* the size of search queues as a percentage
   of the maximum number of states */
#define gPercentActiveStates 0.8

// for use in tsprintf
#define BUFFER_SIZE 1024

//extra timespan added to action duration to set the next action time
#define DEFAULT_EPSILON_TIME_SEPARATION 0.001

#ifdef DISCRETIZATION
#define EPSILON_TIME_SEPARATION (double)min(DISCRETIZATION,DEFAULT_EPSILON_TIME_SEPARATION)
#endif

/****************************************
  Release information
 ****************************************/
/* last update */
#define INCLUDE_FILE_DATE "Dec 8 2015"

/* DiNo Version Number */
#define INCLUDE_FILE_VERSION "DiNo Release 1.1"

/****************************************
  Cached Murphi constants
 ****************************************/
//* gdp: length of split file segments */
#define SPLITFILE_LEN (1024 * 0x100000L)

#define RULE_INDEX_TYPE unsigned long

/****************************************
  main headers
****************************************/
#define TIME_INFINITY 999999

#define SRPG_ENABLED true

#include "upm_verifier.hpp"

/****************************************
  other headers
  ****************************************/
#ifdef HASHC
#include "upm_hash.hpp"
#endif

class mu_1_real_type;



#include "upm_sym.hpp"
#include "upm_util.hpp"
#include "upm_io.hpp"
#include "upm_state.hpp"
#include "upm_graph.hpp"
#include "upm_storage.hpp"
#include "upm_system.hpp"
#include "upm_staged_rpg.hpp"
#include <iostream>
#include <string>
#include <set>
#include <list>
#include <typeinfo>
#include <map>


//std::set<int> helpful_actions;
std::map<state*, std::set<int> > helpful_actions; // WP WP WP WP WP creating a map of tuples: state pointer and a set helpful actions for that state

// returns count of non-overlapping occurrences of 'sub' in 'str'
int countSubstring(const std::string& str, const std::string& sub)
{
	if (sub.length() == 0) return 0;
	int count = 0;
	for (size_t offset = str.find(sub); offset != std::string::npos;
	offset = str.find(sub, offset + sub.length()))
	{
		++count;
	}
	return count;
}


/****************************************
  real numbers extension
  ****************************************/
#include "upm_real.hpp"	//im

