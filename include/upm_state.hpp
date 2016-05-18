/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/


#ifndef _STATE_
#define _STATE_

/****************************************
  There are three different declarations:
  1) state
  2) dynBitVec
  3) state queue
  4) state set
 ****************************************/

/****************************************
  The record for a single state.
  require : BITS_IN_WORLD in parameter file
 ****************************************/

/* BITS_IN_WORLD gets defined by the generated code. */
/* The extra addition is there so that we round up to the greater block. */

/****************************************
  Bit vector - copied straight from Andreas.
 ****************************************/
class dynBitVec
{
  // data
  unsigned long numBits;
  unsigned char* v;

  // Inquiries
  inline unsigned int Index( unsigned long index )
  {
    return index / 8;
  }
  inline unsigned int Shift( unsigned long index )
  {
    return index % 8;
  }

 public:
  // initializer
  dynBitVec( unsigned long nBits );
  // destructor
  virtual ~dynBitVec();

  // interface
  inline int NumBits( void )
  {
    return numBits;
  }
  inline int NumBytes( void )
  {
    return 1 + (numBits - 1) / 8;
  }
  inline void clear( unsigned long i )
  {
    v[ Index(i) ] &= ~(1 << Shift(i));
  }
  inline void set( unsigned long i )
  {
    v[ Index(i) ] |=  (1 << Shift(i));
  }
  inline int get( unsigned long i )
  {
    return (v[ Index(i) ] >> Shift(i)) & 1;
  }
};

class statelist
{
  state * s;
  statelist * next;
 public:
  statelist(state * s, statelist * next)
    : s(s), next(next) {};
};

struct state_and_index {
  state s;
  unsigned long i;
};


/****************************************
  The state queue.
 ****************************************/
class state_queue
{
 protected:
  state_and_index * stateArray;		/* The actual array. */
  //state_and_index_and_counter * stateArray;		/* The actual array. */
  const unsigned long max_active_states;	/* max size of queue */
  unsigned long front;		/* index of first active state. */
  unsigned long rear;		/* index of next free slot. */

  unsigned long global_rear, global_front;
  unsigned long num_elts_head, num_elts_tail;
  unsigned long head_begin, tail_begin;
  unsigned long head_size, tail_size;

#ifndef SPLITFILE
  FILE *paging_file_top, *paging_file_bottom;
#else
  splitFile *paging_file_top, *paging_file_bottom;
#endif

 public:
  // initializers
  state_queue(unsigned long mas);

  // destructor
  virtual ~ state_queue();

  // information interface
  inline unsigned long MaxElts(void)
  {
    return max_active_states;
  } unsigned long NumElts(void)
  {
    return num_elts_head + num_elts_tail + global_rear + global_front;
  }
  inline bool isempty(void)
  {
    return num_elts_head + num_elts_tail + global_rear + global_front == 0;
  }

  inline static int BytesForOneState(void);


  // storing and removing elements
  virtual void enqueue(state * &e, unsigned long index);
  virtual state_and_index *dequeue(void);
  //virtual state_and_index_and_counter *dequeue(void);
  virtual state_and_index *top(void);

  virtual void ReclaimFreeSpace();
  virtual void QueueEmptyFault();

  virtual RULE_INDEX_TYPE NextRuleToTry()	// Uli: unsigned short -> unsigned
  {
    Error.Notrace
    ("Internal: Getting next rule to try from a state queue instead of a state stack.");
    return 0;
  }
  virtual void NextRuleToTry(RULE_INDEX_TYPE r)
  {
    Error.Notrace
    ("Internal: Setting next rule to try from a state queue instead of a state stack.");
  }

  // printing routine
  void Print(void);
  virtual void print_capacity(void)
  {
    cout << "\t* Capacity in queue for breadth-first search: "
         << max_active_states << " states.\n"
         <<
         "\t   * Change the constant gPercentActiveStates in mu_prolog.inc\n"
         << "\t     to increase this, if necessary.\n";
  }
};

class state_stack: public state_queue
{
  RULE_INDEX_TYPE * nextrule_to_try;

 public:
  // initializers
  state_stack( unsigned long mas )
    : state_queue(mas)
  {
    unsigned long i;
    nextrule_to_try = new RULE_INDEX_TYPE [ mas ];
    MEMTRACKALLOC
    for ( i = 0; i < mas; i++)
      nextrule_to_try[i] = 0;
  };

  // destructor
  virtual ~state_stack()
  {
    delete[ OLD_GPP(max_active_states) ] nextrule_to_try; // Should be delete[].
  };

  virtual void print_capacity( void )
  {
    cout << "\t* Capacity in queue for depth-first search: "
         << max_active_states << " states.\n"
         << "\t   * Change the constant gPercentActiveStates in mu_prolog.inc\n"
         << "\t     to increase this, if necessary.\n";
  }
  virtual void enqueue( state* e );

  virtual RULE_INDEX_TYPE NextRuleToTry()
  {
    return nextrule_to_try[ front ];
  }
  virtual void NextRuleToTry(RULE_INDEX_TYPE r)
  {
    nextrule_to_try[ front ] = r;
  }

#ifdef partial_order_opt
  // special interface with sleepset
  virtual void enqueue( state *e, sleepset s );
#endif
};

/****************************************
  A generic disk queue.
 ****************************************/
template<typename E>
class disk_queue
{
 protected:
  E * cache;
  const unsigned long max_length;	/* max size of queue */
  unsigned long front;		/* index of first active state. */
  unsigned long rear;		/* index of next free slot. */

  unsigned long global_rear, global_front;
  unsigned long num_elts_head, num_elts_tail;
  unsigned long head_begin, tail_begin;
  unsigned long head_size, tail_size;

#ifndef SPLITFILE
  FILE *paging_file_top, *paging_file_bottom;
#else
  splitFile *paging_file_top, *paging_file_bottom;
#endif

 public:
  // initializers
  disk_queue(unsigned long len);

  // destructor
  virtual ~ disk_queue();

  // information interface
  inline unsigned long MaxLength(void)
  {
    return max_length;
  }
  unsigned long NumElts(void)
  {
    return num_elts_head + num_elts_tail + global_rear + global_front;
  }
  inline bool isempty(void)
  {
    return num_elts_head + num_elts_tail + global_rear + global_front == 0;
  }

  // storing and removing elements
  virtual void enqueue(E &e);
  virtual E dequeue(void);
  virtual E top(void);

  virtual void ReclaimFreeSpace();
  virtual void QueueEmptyFault();
};

/****************************************
  The state set
  represented as a large open-addressed hash table.
 ****************************************/

class state_set
{

#ifdef HASHC
  typedef unsigned long Unsigned32;    // basic building block of the hash
  // table, slots may have different size
#endif

  // data
  unsigned long table_size;            /* max size of the hash table */
#ifndef HASHC
  state *table;                        /* pointer to the hash table */
#else
  Unsigned32 *table;
#endif
  unsigned long *disk_index;
  FILE *reachables;
  dynBitVec *Full;                     /* whether element table[i] is used. */
  unsigned long num_elts;              /* number of elements in table */
  unsigned long num_elts_reduced;   // Uli
  unsigned long num_collisions;        /* number of collisions in hashing */

  // internal routines
  bool is_empty( unsigned long i )     /* check if element table[i] is empty */
  {
    return Full->get(i) == 0;
  };

  void create_table (unsigned long table_size, int level);

 private:
  inline bool needs_expanded_states()
  {
    return (args->output_fmt.mode == argoutput_fmt::PDDL_VeryVerbose ||
            args->output_fmt.mode == argoutput_fmt::Text_Verbose  ||
            args->output_fmt.mode == argoutput_fmt::Raw  ||
            args->output_fmt.mode == argoutput_fmt::CSV
#ifdef OBDD_COMPRESSION
            || args->output_fmt.mode == argoutput_fmt::OBDD
#endif
           );
  }

 public:
  // constructors
  state_set ( unsigned long table_size );
  state_set ( void );

  friend void copy_state_set( state_set * set1, state_set * set2);
  void clear_state_set();

  // destructor
  virtual ~state_set();

  // checking the presence of state "in"
  bool simple_was_present( state *&in, bool, bool, unsigned long *index=NULL );
  /* old was_present without checking -sym */
  bool was_present( state *&in, bool, bool, unsigned long *index=NULL );
  /* checking -sym before calling simple_was_present() */


  // get the size of each state entry
#ifndef VER_PSEUDO
  static int bits_per_state(void);
#endif

  inline unsigned long Capacity()
  {
    return table_size;
  }
  inline unsigned long NumElts()
  {
    return num_elts;
  };
  inline unsigned long MaxElts()
  {
    return table_size;
  };
  inline bool IsFull()
  {
    return num_elts>=table_size;
  }; //GDP
  inline unsigned long NumEltsReduced()
  {
    return num_elts_reduced;
  };   // Uli

  // printing information
  void print_capacity( void );

  // print hashtable
  void print()
  {
    for (unsigned long i=0; i<table_size; i++)
      if (!is_empty(i)) {
        cout << "State " << i << "\n";
#ifdef HASHC
        cout << "... compressed\n";
#else
        StateCopy(workingstate,&table[i]);
        theworld.print();
#endif
        cout << "\n";
      }
  }
#ifndef HASHC
  state *
#else
  Unsigned32 *
#endif
  ret_table_addr()
  {
    return table;
  }
};


#endif
