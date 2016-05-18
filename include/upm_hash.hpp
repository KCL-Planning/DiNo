/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

class hash_function
{
 public:
  hash_function(int vec_size);
  ~hash_function();
  unsigned long *hash(state * s, bool valid);
 private:
  unsigned long *hashmatrix;
  int vec_size;
  unsigned char *oldvec;
  unsigned long key[3];
};
