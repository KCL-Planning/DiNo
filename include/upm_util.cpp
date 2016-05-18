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
  There are four categories:
  1) utility functions (not in any class)
  2) mu__int and mu__boolean
  3) world_class
  4) timer (not in any class)
  5) setofrules
  6) rule_matrix
  7) random number generator
  ****************************************/

/****************************************
  Utility functions
  ****************************************/

/* g++ seems not to know about strstr, so we\'ll write our own. Bleah. */
/* Bad code, bad! But it\'s not on the critical path. */
/* Copied from Andreas. */
const char *StrStr(const char *super, const char *sub)
{
  int i, j;
  for (i = 0; super[i]; i++) {
    for (j = 0; sub[j] != '\0'; j++) {
      if (super[i + j] != sub[j])
        break;
    }
    if (j == strlen(sub))
      return &(super[i]);	/* Match. */
  }
  return NULL;			/* No match. */
}

void ErrAlloc(void *p)
/* I don\'t think this actually does anything; we should patch
 * new_handler instead. */
{
  if (p == NULL) {
    Error.Notrace("Unable to allocate memory.");
  }
}

void err_new_handler(void)
{
  Error.Notrace("Unable to allocate memory.");
}

/* we should #include <signal.h> at some point. */
// Uli: OLD_CATCH_DIV has to be defined or undefined depending on compiler
//      and operating system
#ifdef OLD_CATCH_DIV
void catch_div_by_zero(...)
#else
void catch_div_by_zero(int)
#endif
{
  Error.Error("Division by zero.");
}

/* From Andreas\' code. */
bool IsPrime(unsigned long n)
{
  unsigned long i = 3;
  if (n < 3)
    return TRUE;
  if (n % 2 == 0)
    return FALSE;
  while (i * i <= n) {
    if (n % i == 0) {
      return FALSE;
    }
    i += 2;
  }
  return TRUE;
}

unsigned long NextPrime(unsigned long n)
/* Well, this is only O ( n * sqrt(n) )--and more importantly, it only
 * gets executed once. */
{
  while (!IsPrime(n)) {
    n++;
  }
  return n;
}

unsigned long NumStatesGivenBytes(unsigned long bytes)
/* From Andreas\' code.*/
{
  unsigned long exactNumStates = (unsigned long)
                                 (
                                   (double) bytes * 8 /
                                   (state_set::bits_per_state() + gPercentActiveStates * 8 * state_queue::BytesForOneState())
                                 );
  return NextPrime(exactNumStates);
}

char *tsprintf(const char *fmt, char *str)
{
  static char temp_buffer[BUFFER_SIZE];
  char *newstr;
  sprintf(temp_buffer, fmt, str);
  newstr = new char[strlen(temp_buffer) + 1];
  MEMTRACKALLOC
  strcpy(newstr, temp_buffer);
  return newstr;
}

char *tsprintf(const char *fmt, char *str1, char *str2)
{
  static char temp_buffer[BUFFER_SIZE];
  char *newstr;
  sprintf(temp_buffer, fmt, str1, str2);
  newstr = new char[strlen(temp_buffer) + 1];
  MEMTRACKALLOC
  strcpy(newstr, temp_buffer);
  return newstr;
}

char *tsprintf(const char *fmt, ...)
/* sprintf's the arguments into dynamically allocated memory.  Returns the
 * dynamically allocated string. */
{
  static char temp_buffer[BUFFER_SIZE];	/* hope that\'s enough. */
  va_list argp;
  char *retval;

  va_start(argp, fmt);
  vsprintf(temp_buffer, fmt, argp);
  va_end(argp);

  if (strlen(temp_buffer) >= BUFFER_SIZE)
    Error.Error("Temporary buffer overflow.\n\
Please increase the constant BUFFER_SIZE in file mu_verifier.h and recompile your program\n\
(you may also reduce the length of expression by using function call.");

  retval = new char[strlen(temp_buffer) + 1];
  MEMTRACKALLOC
  strcpy(retval, temp_buffer);
  return (retval);
}

/****************************************
  The base class for a value
  ****************************************/

/* Now that we\'ve defined the state, we can go back and write
 * mu__int::to_state() and mu__int::from_state(). */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// mu__int

#ifndef NO_RUN_TIME_CHECKING
void mu__int::boundary_error(int val) const
{
  Error.Error("%d not in range for %s.", val, name);
}

int mu__int::undef_error() const
{
  switch (category) {
    case STARTSTATE:
      Error.Error
      ("The undefined value at %s is referenced\n\tin startstate:\n\t%s",
       name, StartState->LastStateName());
    case CONDITION:
      Error.Error
      ("The undefined value at %s is referenced\n\tin the guard of the rule:\n\t%s",
       name, Rules->LastRuleName());
    case RULE:
      Error.Error
      ("The undefined value at %s is referenced\n\tin rule:\n\t%s", name,
       Rules->LastRuleName());
    case INVARIANT:
      Error.Error
      ("The undefined value at %s is referenced\n\tin invariant:\n\t%s",
       name, Properties->LastInvariantName());

    default:
      Error.Error
      ("The undefined value at %s is referenced\n\tat a funny time\nnot recognized by the verifier.",
       name);
  }
  return lb;
};
#endif

// Uli: simplified print_diff
void mu__int::print_diff(state * prevstate, FILE *target, const char *separator)
{
  /* We assume that prevstate is not null. */
#ifndef ALIGN
  if (prevstate->get(&where) != workingstate->get(&where))
#else
  if (prevstate->get(byteOffset) != workingstate->get(byteOffset))
#endif
    print(target,separator);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// mu__long

// Uli: simplified print_diff
void mu__long::print_diff(state * prevstate, FILE *target, const char *separator)
{
  /* We assume that prevstate is not null. */
#ifndef ALIGN
  if (prevstate->get(&where) != workingstate->get(&where))
#else
  if (prevstate->getlong(byteOffset) != workingstate->getlong(byteOffset))
#endif
    print(target,separator);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// mu_0_boolean

const char *mu_0_boolean::values[] = { "false", "true" };

void mu_0_boolean::Permute(PermSet & Perm, int i)
{
};

void mu_0_boolean::SimpleCanonicalize(PermSet & Perm)
{
};

void mu_0_boolean::Canonicalize(PermSet & Perm)
{
};

void mu_0_boolean::SimpleLimit(PermSet & Perm)
{
};

void mu_0_boolean::ArrayLimit(PermSet & Perm)
{
};

void mu_0_boolean::Limit(PermSet & Perm)
{
};

void mu__real::print_diff(state * prevstate, FILE *target, const char *separator)
{
  /* We assume that prevstate is not null. */
#ifndef ALIGN
  if (getrealb(prevstate) != getrealb(workingstate))
#else
  if (getreal(prevstate) != getreal(workingstate))
#endif
    print(target,separator);
}


#ifndef NO_RUN_TIME_CHECKING
void mu__real::undef_error() const
{
  switch (category) {
    case STARTSTATE:
      Error.Error
      ("The undefined value at %s is referenced\n\tin startstate:\n\t%s",
       name, StartState->LastStateName());
    case CONDITION:
      Error.Error
      ("The undefined value at %s is referenced\n\tin the guard of the rule:\n\t%s",
       name, Rules->LastRuleName());
    case RULE:
      Error.Error
      ("The undefined value at %s is referenced\n\tin rule:\n\t%s", name,
       Rules->LastRuleName());
    case INVARIANT:
      Error.Error
      ("The undefined value at %s is referenced\n\tin invariant:\n\t%s",
       name, Properties->LastInvariantName());

    default:
      Error.Error
      ("The undefined value at %s is referenced\n\tat a funny time\nnot recognized by the verifier.",
       name);
  }
};
#endif
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/****************************************
  world_class declarations
  ****************************************/

/* in the same vein, now we can write world_class::get_state(). */
state *world_class::getstate()
{
  return NULL;
};				// changed by Uli

/***************************
  Timer
  ***************************/

// changed by Uli, ideas from Liz
//   this way of getting the runtime seems much more portable than
//   the old way

#include <sys/times.h>		// for times() and related structs
#include <unistd.h>		// for sysconf()

// Returning a double here is better for Unix, since otherwise
// a long would only suffice for about 30 minutes.

double SecondsSinceStart(void)
{
  double retval;
  static double numTicksPerSec = (double) (sysconf(_SC_CLK_TCK));
  static struct tms clkticks;

  times(&clkticks);		// retrieve the time-usage information
  retval = ((double) clkticks.tms_utime + (double) clkticks.tms_stime)
           / numTicksPerSec;
  if (retval <= 0.1) {		/* Avoid div-by-zero errors. */
    retval = 0.1;
  }
  return retval;
}


/****************************************
  class setofrules
  ****************************************/

setofrules interset(setofrules rs1, setofrules rs2)
{
  setofrules retval;
  for (RULE_INDEX_TYPE r = 0; r < numrules; r++) {	// Uli: unsigned short -> unsigned
    if (rs1.in(r) && rs2.in(r))
      retval.add(r);
    else
      retval.remove(r);
  }
  return retval;
}

setofrules different(setofrules rs1, setofrules rs2)
{
  setofrules retval;
  for (RULE_INDEX_TYPE r = 0; r < numrules; r++) {
    if (rs1.in(r) && !rs2.in(r))
      retval.add(r);
    else
      retval.remove(r);
  }
  return retval;
}

bool subset(setofrules rs1, setofrules rs2)
{
  for (RULE_INDEX_TYPE r = 0; r < numrules; r++)
    if (rs1.in(r) && !rs2.in(r))
      return FALSE;
  return TRUE;
}


/***************************   // added by Uli
  random number generator
  ***************************/
// see Jain, The Art of Computer Systems Performance Analysis, pg.442 and 452
// generator: x[n] = 7^5 x[n-1] mod(2^31-1), period: 2^31-2

#include <sys/time.h>

randomGen::randomGen()
{
  struct timeval tp;
  struct timezone tzp;

  // select a "random" seed
  gettimeofday(&tp, &tzp);
  value = ((unsigned long) (tp.tv_sec ^ tp.tv_usec) * 2654435769ul) >> 1;
  if (value == 0)
    value = 46831694;
}

unsigned long
randomGen::next()
{
  long g;

  g = 16807 * (value % 127773) - 2836 * (value / 127773);
  return value = (g > 0) ? g : g + 2147483647;
}
