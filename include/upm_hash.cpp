/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

// changes by Uli
hash_function::hash_function (int vsize)
{
  int i, j, k;
  randomGen random;
  unsigned long r;


  vec_size = vsize;

  hashmatrix = (unsigned long *)
               malloc (vec_size * sizeof (unsigned long) * 24);

  MEMTRACKALLOC

  // initialize hashmatrix
  for (i = 0; i < (vec_size * 24); i++) {
    // generate dummy random numbers to get rid of dependencies
    k = int ((r = random.next ()) % 11 + 13);
    for (j = 0; j < k; j++) {
      random.next ();
    }
    hashmatrix[i] = random.next () ^ (r << 16);	// generator only yields 31 bit
    // random numbers
  }

  oldvec = new unsigned char[vec_size];

  MEMTRACKALLOC

  for (i = 0; i < vec_size; i++) {
    oldvec[i] = 0;
  }
  key[0] = 0UL;
  key[1] = 0UL;
  key[2] = 0UL;
}

hash_function::~hash_function ()
{
  delete oldvec;
  free (hashmatrix);
}

// changes by Uli
inline unsigned long *
hash_function::hash (state * s, bool valid)
// Uli: calculates the hash function for a state
// - if valid is TRUE, curstate must point to a state and this state is
//  used for differential hashing (the hashkeys[] must have been set cor-
//  rectly)
// - otherwise, buffer oldvec is used
{
  register unsigned long l0, l1, l2;
  register unsigned char qq;
  register unsigned char *q = s->bits, *qp;
  register unsigned char mask;
  register int ind = 0, i;
  int h = vec_size;

  // set the correct old values of vector and key
  // only in the aligned version the hashkeys are stored with the state
#ifdef ALIGN
  if (valid) {
    l0 = curstate->hashkeys[0];
    l1 = curstate->hashkeys[1];
    l2 = curstate->hashkeys[2];
    qp = curstate->bits;
  } else
#endif
  {
    l0 = key[0];
    l1 = key[1];
    l2 = key[2];
    qp = oldvec;
  }

  do {
    if (qq = *qp ^ *q) {
      mask = 1;
      for (i = ind; i < ind + 24; i += 3) {	/* scan all bits of current byte */
        if (qq & mask) {
          l0 ^= hashmatrix[i];
          l1 ^= hashmatrix[i + 1];
          l2 ^= hashmatrix[i + 2];
        }
        mask = mask << 1;
      }
#ifdef ALIGN
      if (!valid)
#endif
        *qp = *q;		// set the oldvec
    }
    q++;
    qp++;
    ind += 24;
  } while (--h > 0);

#ifdef ALIGN
  s->hashkeys[0] = l0;
  s->hashkeys[1] = l1;
  s->hashkeys[2] = l2;
  if (!valid)
#endif
  {
    key[0] = l0;
    key[1] = l1;
    key[2] = l2;
  }
#ifdef ALIGN
  return s->hashkeys;
#else
  return key;
#endif
}
