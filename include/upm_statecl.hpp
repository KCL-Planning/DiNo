/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#ifndef ALIGN
#define BLOCKS_IN_WORLD (((BITS_IN_WORLD + (4*BITS( BIT_BLOCK )) - 1 ) / (4*BITS(BIT_BLOCK )))*4)
#else
#define BLOCKS_IN_WORLD (((BITS_IN_WORLD + (4*BITS( BIT_BLOCK )) - 1 ) / (4*BITS(BIT_BLOCK )))*4)
#endif
#include <string>
#include <set>
#include <vector>
#include <iostream>

/****************************************   // added by Uli
  class for pointer to previous state
  ****************************************/

class state;

class StatePtr
{
 private:
  union {
    state *sp;			// real pointer
    unsigned long lv;		// state number used in trace info file
  };

  // StatePtr is a member of the class state, so try to avoid adding
  // data members

  inline void sCheck();
  inline void lCheck();

 public:
  StatePtr()
  {
  } StatePtr(state * s);
  StatePtr(unsigned long l);

  state * get_sp(){return sp;}
  void set(state * s);
  void set(unsigned long l);
  void clear();
  state *sVal();
  unsigned long lVal();

  StatePtr previous();		// return StatePtr to previous state
  bool isStart();		// check if I point to a startstate
  bool compare(state * s);	// compare the state I point to with s

};


/****************************************
  class state.
  ****************************************/
// Warning: DO NOT add member variables to this class unless
// you know exactly what you're doing. Since an instance of the
// state class is created for each table entry during
// initialization, adding stuff here will eat lots of memory.
// Uli: this is only true if one does not use hash compaction

#ifdef HASHC
hash_function *h3;
#endif
class mu_0_boolean; // WP WP WP WP WP
//class mu_1__type_super; // WP WP WP WP WP
class mu__real; // WP WP WP WP WP

class state
{
 public:
  BIT_BLOCK bits[BLOCKS_IN_WORLD];
  StatePtr previous;		// state from which this state was reached.
#ifdef HASHC
#ifdef ALIGN
  // Uli: only in the aligned version the hashkeys are stored with the state
  unsigned long hashkeys[3];
#endif
#endif

  state()
    :previous()
  {
    memset((void *) bits, 0, (size_t) BLOCKS_IN_WORLD);	// Uli: avoid bzero
  };
  state(state * s)
  {
    StateCopy(this, s);
  };

  friend void StateCopy(state * l, state * r);
  friend int StateCmp(state * l, state * r);
  friend void copy_state(state * &s);
#ifdef HASHC
  friend class hash_function;	//gdp: fix for newer g++
#endif
//friend unsigned long* hash_function::hash(state*, bool);
// friend bool StateEquivalent(state * l, state * r);   // Uli: not necessary

  // get it with Horner\'s method.
  // size  <= the number of bits in an integer - 1
#ifndef ALIGN
  inline int get(const position * w) const  	// Uli: const added
  {
    unsigned int val, *l;
    l = (unsigned int *) (bits + w->longoffset);
    val = (l[0] & w->mask1) >> w->shift1;
    if (w->mask2)
      val |= (l[1] & w->mask2) << w->shift2;
    return (int) val;
  }
#else
  inline int get(int byteloc) const
  {
    return bits[byteloc];
  };
  inline int getlong(int byteloc) const
  {
    unsigned int val;
    unsigned char *p = (unsigned char *) &bits[byteloc];
    val = *p++;
    val |= *p++ << 8;
    val |= *p++ << 16;
    val |= *p++ << 24;
    return (int) val;
  }
#endif
  // set a field to value // size  <= the number of bits in an integer - 1
#ifndef ALIGN
  inline void set(position * w, int value)
  {
    unsigned int val, *l;
    l = (unsigned int *) (bits + w->longoffset);
    val = value << w->shift1;
    l[0] = (l[0] & ~w->mask1) | val;
    if (w->mask2) {
      val = (value >> w->shift2) & w->mask2;
      l[1] = (l[1] & ~w->mask2) | val;
    }
  };
#else
  inline void setlong(int byteloc, int value)
  {
    unsigned char *p = (unsigned char *) &bits[byteloc];
    *p++ = (unsigned) value & 0xff;
    *p++ = ((unsigned) value & 0xff00) >> 8;
    *p++ = ((unsigned) value & 0xff0000) >> 16;
    *p++ = ((unsigned) value & 0xff000000) >> 24;
  };
#endif

  // key for hash function, changes by Uli
  /* defines log_2(sizeof(long)) */
#define LOG2_UL 2

#ifndef HASHC
  unsigned long hashkey(void) const
  {
    unsigned long sum = 0;
    unsigned long *pt = (unsigned long *) bits;

    for (int i = BLOCKS_IN_WORLD >> LOG2_UL; i > 0; i--) {
      sum += *pt++;
    }
    return sum;
  }
#endif


  // operator== and operator!= only consider the bitvectors,
  // not the other components of a state.
  inline bool operator ==(state & other) const
  {
    return (memcmp(&bits, &other.bits, BLOCKS_IN_WORLD) == 0);
  };
  inline bool operator !=(state & other) const
  {
    return (memcmp(&bits, &other.bits, BLOCKS_IN_WORLD) != 0);
  };

  //WP fire processes (t_steps = run the processes a desired number of times)
  void fire_processes(int t_steps);

  //WP fire processes with add effects only (t_steps = run the processes a desired number of times)
  void fire_processes_plus(int t_steps);

  //WP fire processes with delete effects only(t_steps = run the processes a desired number of times)
  void fire_processes_minus(int t_steps);

  //WP get g(n)+h(n) value of a state
  double get_f_val();

  //WP setting a g(n)+h(n) value for a state
  void set_f_val();

  //WP get h(n) value of state
  double get_h_val();

  //WP setting a h(n) value for a state using FF-RPG
  void set_h_val();

  //WP setting a h(n) value for a state using a parameter
  void set_h_val(int hp);

  //WP get g(n) value of a state
  double get_g_val();

  //WP setting a g(n) value for a state
  void set_g_val(double g_n);


  //WP getting all declared simple boolean variables
  std::vector<mu_0_boolean*> get_mu_bools();

  //WP getting all declared booleans from arrays (even multi-dimensional)
  std::vector<mu_0_boolean*> get_mu_bool_arrays();

  //WP getting all declared simple numerical variables
  std::vector<mu__real*> get_mu_nums();

  //WP getting all declared numerical variables from arrays (even multi-dimensional)
  std::vector<mu__real*> get_mu_num_arrays();


  // scribbles over the current world variables.
  void print();

  // symmetry reduction
  void Normalize();

  // multiset reduction
  void MultisetSort();

};
