/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#ifndef __LEXTABLE_H__
#define __LEXTABLE_H__

/********************
  constants
  ********************/
#define LEXTABLESIZE 1024	/* must be a power of two. */
#define RESERVETABLESIZE 512	/* must be a power of two. */

/********************
  class lextable
         -- hash table with open chaining for lex's name table.
  ********************/
class lextable
{
  lexid *table[LEXTABLESIZE];	// table for user defined words
  lexid *rtable[RESERVETABLESIZE];	// table for reserved words
  int hash(const char *str) const;	// hash a string into an integer.
  int rehash(int h) const;	// if table[h] is taken, compute a new value.
 public:
  // initializer
  lextable(void);

  // supporting routines
  lexid *enter(const char *str, int lextype = ID);
  // return a pointer to the id lexid associated with str, creating the
  // lexid if necessary.
  bool reserved(const char *str);	// check to see if it is a reserved word
  lexid *enter_reserved(const char *str, int lextype = ID);
  // return a pointer to the id lexid associated with str, creating the
  // lexid if necessary.
};

/********************
  extern variables
  ********************/
extern lextable ltable;

#endif
