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
  There are 2 groups of declarations:
  1) setofrule and sleepset
  2) rule_matrix
  ****************************************/

/****************************************
  class setofrule and sleepset
  require RULES_IN_WORLD
  ****************************************/

#define BLOCKS_IN_SETOFRULES ( (RULES_IN_WORLD + BITS( BIT_BLOCK ) - 1 ) / \
			  BITS( BIT_BLOCK ))

/* RULES_IN_WORLD gets defined by the generated code. */
/* The extra addition is there so that we round up to the greater block. */

class setofrules
{
 protected:
  BIT_BLOCK bits[BLOCKS_IN_SETOFRULES];
  RULE_INDEX_TYPE NumRules;		// Uli: unsigned short -> unsigned

  int Index(int i) const
  {
    return i / BITS(BIT_BLOCK);
  };
  int Shift(int i) const
  {
    return i % BITS(BIT_BLOCK);
  };
  int Get1(int i) const
  {
    return (bits[Index(i)] >> Shift(i)) & 1;
  };
  void Set1(int i, int val)  	/* Set bit i to the low bit of val. */
  {
    if ((val & 1) != 0)
      bits[Index(i)] |= (1 << Shift(i));
    else
      bits[Index(i)] &= ~(1 << Shift(i));
  };

 public:

  // set of rules manipulation
  friend setofrules interset(setofrules rs1, setofrules rs2);
  friend setofrules different(setofrules rs1, setofrules rs2);
  friend bool subset(setofrules rs1, setofrules rs2);

  // conflict set manipulation
  friend setofrules conflict(RULE_INDEX_TYPE rule);

  setofrules()
    :    NumRules(0)
  {
    for (int i = 0; i < BLOCKS_IN_SETOFRULES; i++)
      bits[i] = 0;
  };

  virtual ~ setofrules()
  {
  };

  bool in(RULE_INDEX_TYPE rule) const
  {
    return (bool) Get1(rule);
  };
  void add(RULE_INDEX_TYPE rule)
  {
    if (!in(rule)) {
      Set1(rule, TRUE);
      NumRules++;
    }
  };
  void remove(RULE_INDEX_TYPE rule)
  {
    if (in(rule)) {
      Set1(rule, FALSE);
      NumRules--;
    }
  }
  bool nonempty()
  {
    return (NumRules != 0);
  };
  RULE_INDEX_TYPE size()
  {
    return NumRules;
  };
  void print()
  {
    cout << "The set of rules =\t";
    for (RULE_INDEX_TYPE i = 0; i < RULES_IN_WORLD; i++)
      cout << (Get1(i) ? 1 : 0) << ',';
    cout << "\n";
  };

  // for simulation
  void removeall()
  {
    for (RULE_INDEX_TYPE i = 0; i < RULES_IN_WORLD; i++)
      Set1(i, FALSE);
    NumRules = 0;
  };

  // for simulation
  void includeall()
  {
    for (RULE_INDEX_TYPE i = 0; i < RULES_IN_WORLD; i++)
      Set1(i, TRUE);
    NumRules = RULES_IN_WORLD;
  };
  RULE_INDEX_TYPE getnthrule(RULE_INDEX_TYPE rule)
  {
    RULE_INDEX_TYPE r = 0;
    RULE_INDEX_TYPE i = 0;
    while (1)
      if (Get1(i) && r == rule) {
        Set1(i, FALSE);
        NumRules--;
        return (RULE_INDEX_TYPE) i;
      } else if (Get1(i)) {
        i++;
        r++;
      } else {
        i++;
      }
  };
};
