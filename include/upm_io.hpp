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
  There are 3 groups of declarations:
  1) Error_handler
  2) argclass
  3) general printing routine (not belong to any class)
  4) trace info file
 ****************************************/

/****************************************
  Error handler
 ****************************************/

class Error_handler
{
  char buffer[BUFFER_SIZE]; // for vsprintf'ing error messages prior to cout'ing them.

  int num_errors;
  int phase;
  int oldphase;
  bool has_error;
  int num_error_curstate;
  int phase_2_done;

 public:
  Error_handler()
    : num_errors(0), phase(1) {};
  ~Error_handler() {};

  void StartCountingCurstate()
  {
    num_error_curstate = 0;
  }
  int ErrorNumCurstate()
  {
    return num_error_curstate;
  }
  bool Phase2Done()
  {
    return phase_2_done;
  }

  void SpecialPhase()
  {
    oldphase = phase;
    phase = 3;
  };
  void NormalPhase()
  {
    phase = oldphase;
  };

  void ResetErrorFlag()
  {
    phase = 3;
    has_error = FALSE;
  }
  bool NoError()
  {
    return !has_error;
  };

  int NumError()
  {
    return num_errors;
  };

  void Error( const char *fmt, ... );       /* called like printf. */
  void Deadlocked( const char *fmt, ... ); /* When we\'re not in a rule.
					       currently only in deadlock */
  void Notrace( const char *fmt, ... );     /* Doesn\'t print a trace. */
};

/****************************************
  iterator for argument class
 ****************************************/

/* abstract class for mapping over a list of strings. */
class string_iterator
{
  /* restrictions:
   * Also, you can\'t have more than one of these going at a time. */
 public:
  virtual const char *value() = 0;
  virtual const char *nextvalue() = 0;
  virtual string_iterator& next() =0;
  virtual bool done() =0;
  virtual void start() =0;
};

class arg_iterator: public string_iterator
{
  int argc;
  char **argv;
  int index;
 public:
  arg_iterator (int argc, char **argv)
    :argc(argc), argv(argv), index(1) {}; /* index(1) to skip the program name. */
  virtual const char *value()
  {
    return argv[ index ];
  };
  virtual const char *nextvalue()
  {
    if (index+1>=argc) return "";
    else return argv[ index+1 ];
  };
  virtual string_iterator& next()
  {
    index++;
    return *this;
  }
  virtual bool done()
  {
    return (index >= argc);
  }
  virtual void start() { };
};

class strtok_iterator: public string_iterator
/* uses strtok() to break up into strings. */
{
  char *old;
  char *current;
 public:
  strtok_iterator (char *s)
    :old(s), current(NULL)
  {
    start();
  };
  virtual const char *value()
  {
    return current;
  };
  virtual string_iterator& next()
  {
    current = strtok(NULL," ");
    return *this;
  }
  virtual bool done()
  {
    return (current == NULL);
  }
  virtual void start()
  {
    if (old != NULL) current = strtok(tsprintf("%s",old), " ");
  };
  /* we can\'t count on strdup() being there, unfortunately. */
};

/****************************************
  argument class
 ****************************************/
class argmain_alg
{
 public:
  enum MainAlgorithmtype { Nothing, Simulate, Explore_bfs, Explore_dfs, SRPG };
  MainAlgorithmtype mode; /* What to do. */
 private:
  bool initialized;
  const char * name;
 public:
  argmain_alg(MainAlgorithmtype t, const char *n) : mode(t), initialized(FALSE), name(n) {};
  ~argmain_alg() {};
  void set(MainAlgorithmtype t)
  {
    if (!initialized) {
      initialized = TRUE;
      mode = t;
    } else if (mode != t)
      Error.Notrace("Conflicting options to %s.", name);
  };
};

class argsym_alg
{
 public:
  enum SymAlgorithmType {  Exhaustive_Fast_Canonicalize,
                           Heuristic_Fast_Canonicalize,
                           Heuristic_Small_Mem_Canonicalize,
                           Heuristic_Fast_Normalize
                        };
  SymAlgorithmType mode; /* What to do. */
 private:
  bool initialized;
  const char * name;
 public:
  argsym_alg(SymAlgorithmType t, const char *n) : mode(t), initialized(FALSE), name(n) {};
  ~argsym_alg() {};
  void set(SymAlgorithmType t)
  {
    if (!initialized) {
      initialized = TRUE;
      mode = t;
    } else if (mode != t)
      Error.Notrace("Conflicting options to %s.", name);
  };
};

class argsearch_alg;
class argoutput_fmt;


class argsearch_alg
{
 public:
  enum SearchAlgorithmtype { Feasible, Optimal, Universal, Universal_Optimal };
  SearchAlgorithmtype mode;
 private:
  bool initialized;
  const char * name;

 public:
  argsearch_alg(SearchAlgorithmtype t, const char *n) : mode(t), initialized(FALSE), name(n) {}
  ~argsearch_alg() {}
  void set(SearchAlgorithmtype t)
  {
    if (!initialized)  {
      initialized = TRUE;
      mode = t;
    } else if (mode != t)  Error.Notrace("Conflicting options to %s.", name);
  }
};


class argoutput_fmt
{
 public:
  enum OutputFormat { Raw, Text, Text_Verbose, PDDL, PDDL_Verbose, PDDL_VeryVerbose, CSV
#ifdef OBDD_COMPRESSION
  , OBDD
#endif
                    };
  OutputFormat mode;
 private:
  bool initialized;
  const char * name;
 public:
  argoutput_fmt(OutputFormat t, const char *n) : mode(t), initialized(FALSE), name(n) {};
  ~argoutput_fmt() {};
  void set(OutputFormat t)
  {
    if (!initialized) {
      initialized = TRUE;
      mode = t;
    } else if (mode != t)
      Error.Notrace("Conflicting options to %s.", name);
  };
};

class argnum
{
 public:
  unsigned long value;
  double doublevalue;
  bool initialized;
 private:
  const char * name;
 public:
  argnum(unsigned long val, const char * n) : value(val), initialized(FALSE), name(n) {};
  argnum(double val, const char * n, int k) : doublevalue(val), initialized(FALSE), name(n) {};
  ~argnum() {};
  void set(unsigned long val)
  {
    if (!initialized) {
      initialized = TRUE;
      value = val;
    } else if (val != value)
      Error.Notrace("Conflicting options to %s.", name);
  };

  void setd(double val)
  {
    if (!initialized) {
      initialized = TRUE;
      doublevalue = val;
    } else if (val != doublevalue)
      Error.Notrace("Conflicting options to %s.", name);
  };
};

class argbool
{
 public:
  bool value;
 private:
  bool initialized;
  const char * name;
 public:
  argbool(bool val, const char *n) : value(val), initialized(FALSE), name(n) {};
  ~argbool() {};
  void reset(bool val)
  {
    initialized = TRUE;
    value = val;
  }
  void set(bool val)
  {
    if (!initialized) {
      initialized = TRUE;
      value = val;
    } else if (val != value)
      Error.Notrace("Conflicting options to %s.", name);
  };
};


class argstring
{
 public:
  char *value;
  bool initialized;
 private:
  const char * name;
 public:
  argstring(const char* val, const char * n) : initialized(FALSE), name(n)
  {
    set(val);
    initialized=FALSE;
  };
  ~argstring()
  {
    if (initialized) delete[] value;
  };
  void set(const char *val)
  {
    if (!initialized)  {
      initialized = TRUE;
      value = new char[strlen(val)+1];
      strcpy(value,val);
    } else if (strcmp(val,value)) {
      Error.Notrace("Conflicting options to %s.", name);
    }
  };
};


/* Argclass inspired by Andreas\' code. */
class argclass
{
  int argc;
  char **argv;
 public:

  // trace options
  argbool print_trace;
  argbool full_trace;
  argbool trace_all;
  argbool find_errors;
  argnum  max_errors;

  // memory options
  argnum mem;

  argnum verbose_from_state;
  argbool use_verbose_from_state;

  // progress report options
  argnum progress_count;
  argbool print_progress;

  // main algorithm options
  argmain_alg main_alg;

  // symmetry option
  argbool symmetry_reduction;
  argbool multiset_reduction;
  argsym_alg sym_alg;
  argnum perm_limit;
  argbool debug_sym;
  argbool sim_report;

  //UPMURPHI_BEGIN
  argnum horizon;
  argnum SRPG_horizon; 	// WP WP WP WP WP
  argnum phase;
#ifdef HAS_CLOCK
  argnum maxtime;
#endif
  //argnum sim_val;
  //argnum sim_report_val;
  //argbool ctrl_print;
  //argbool table_print;
  //argbool sim;
  //argbool hash;
  //argbool startstates;
  argbool deleteintermediate;

  argsearch_alg search_alg;
  argoutput_fmt output_fmt;

  argstring output_file;
  argbool output_print;
  argbool skip_validate;

  // Uli: hash compaction options
#ifdef HASHC
  argnum num_bits;
  argbool trace_file;
#endif

  // testing parameter
  argnum test_parameter1;
  argnum test_parameter2;

  // miscelleneous
  argnum  loopmax;
  argbool verbose;
  argbool no_deadlock;
  argbool print_options;
  argbool print_license;
  argbool print_rule;
  argbool print_hash;

  // supporting routines
  argclass(int ac, char** av);
  ~argclass() {};
  void ProcessOptions(string_iterator *options);
  bool Flag(char *arg);
  void PrintInfo( void );
  void PrintOptions( void );
  void PrintLicense( void );

};

/****************************************
  Printing functions.
 ****************************************/

class ReportManager
{
  void print_trace_aux(StatePtr p);   // changed by Uli
  void PrintFormattedTime(double seconds);

 public:
  ReportManager();
  void CheckConsistentVersion();
  void StartSimulation();

  void print_algorithm();
  void print_warning();
  void print_header( void );
  void print_trace_with_theworld();
  void print_trace_with_curstate();
  void print_status( void );
  void print_progress( void );
  void print_report( void );
  void print_no_error( void );
  void print_summary(bool);   // print omission probabilities only if true


  void print_curstate( void );
  //void print_level();
  void print_dfs_deadlock( void );
  void print_retrack( void );
  void print_fire_startstate();
  void print_fire_rule();
  void print_fire_rule_diff(state * s);
  void print_trace_all();
  void print_verbose_header();
  void print_hashtable();
  void print_final_report();
};

/****************************************   // added by Uli
  trace info file.
 ****************************************/

#ifdef HASHC
class TraceFileManager
{
 public:
  struct Buffer {            // buffer for read
    unsigned long previous;
    unsigned long c1;
    unsigned long c2;
  };

 private:
  int numBytes;           // number of bytes for compressed values
  char name[256];         // filename for trace info file
  FILE* fp;               // file pointer
  Buffer buf;             // buffer for read
  unsigned long inBuf;    // number of state in buffer (0: empty)
  unsigned long last;     // number of last state written
  void writeLong(unsigned long l, int bytes);
  unsigned long readLong(int bytes);

 public:
  TraceFileManager(char*);
  ~TraceFileManager();
  void setBytes(int bits);
  unsigned long numLast();
  void write(unsigned long c1, unsigned long c2, unsigned long previous);
  const Buffer* read(unsigned long number);
};
#endif

