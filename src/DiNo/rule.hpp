/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/


#ifndef __RULE_H__
#define __RULE_H__

/********************
  In each class there are 4 sections
  1) initializer --> declaration in rule.C
  2) supporting routines --> declaration in rule.C
  3) code generation --> declaration in cpp_code.C
  ********************/

/********************
  class rule
  ********************/
struct rule:public TNode {
  // class identifier
  enum rule_class
  { Simple, Startstate, Invar, Quant, Choose, Alias, Fair, Live, Goal }; //UPMURPHI_BEGIN_END

  // variable
  rule *next;

  // initializer
  rule(void);

  // supporting routines
  virtual void set_name(char *name)
  {
    Error.Error("Internal: rule::set_name(%s)", name);
  };
  virtual rule_class getclass() const = 0;

  // code generation
  virtual const char *generate_code();
};

/********************
  extern variable
  ********************/
extern RULE_INDEX_TYPE simplerulenum;

/********************
  class simplerule
  ********************/
struct simplerule:rule {
  enum rule_pddlclass
  { Action, Event, Clock, DurativeStart, DurativeEnd, Other }; //UPMURPHI_BEGIN_END

  static simplerule *SimpleRuleList;
  simplerule *NextSimpleRule;

  // variable
  char *name;
  char *condname, *rulename;	/* internal names for rule and condition. */
  ste *enclosures;		/* enclosing ruleset params and aliases. */
  expr *condition;		/* condition for execution. */
  ste *locals;			/* things defined within the rule. */
  stmt *body;			/* code. */
  RULE_INDEX_TYPE rulenumber;
  RULE_INDEX_TYPE maxrulenumber;
  int size;

  // Vitaly's additions
  int priority;
  // Cardinality of parameters NOT mentioned in the condition
  int indep_card;
  // List of choose parameters NOT mentioned in the condition
  ste *indep_choose;
  // List of choose parameters mentioned in the condition
  ste *dep_choose;
  // End of Vitaly's additions

  //UPMURPHI_BEGIN
  expr *duration;		/* rule execution duration. */
  expr *weight;		/* rule execution weight. */
  char *pddlname;		/* rule pddl name. */
  rule_pddlclass pddlclass;
  //UPMURPHI_END

  // initializer
  simplerule(ste * enclosures,
             expr * condition,
             ste * locals, stmt * body, int priority,
             rule_pddlclass pddlclass, expr * duration, expr * weight);

  // supporting routines
  virtual void set_name(char *name)
  {
    this->name =
      name != NULL ? name : tsprintf("Rule %d", simplerulenum++);
  }
  virtual rule_class getclass() const
  {
    return Simple;
  };

  //UPMURPHI_BEGIN
  virtual rule_pddlclass getpddlclass()
  {
    return pddlclass;
  };
  virtual void setpddlclass(rule_pddlclass cl)
  {
    pddlclass=cl;
  };

  virtual expr *getduration()
  {
    return duration;
  };
  virtual void setduration(expr * e)
  {
    duration=e;
  };

  virtual expr *getweight()
  {
    return weight;
  };
  virtual void setweight(expr * e)
  {
    weight=e;
  };

  virtual char *get_pddlname()
  {
    return (pddlname!=NULL?pddlname:name);
  };
  virtual void set_pddlname(char *name)
  {
    pddlname=name;
  };
  //UPMURPHI_END

  // code generation
  virtual const char *generate_code();

  int CountSize(ste * enclosures);

  void rearrange_enclosures();

  int getsize()
  {
    return size;
  }
};

/********************
  extern variable
  ********************/
extern int startstatenum;

/********************
  class startstate
  ********************/
struct startstate:simplerule {
  // initializer
  startstate(ste * enclosures, ste * locals, stmt * body);

  // supporting routines
  virtual void set_name(char *name)
  {
    this->name =
      name != NULL ? name : tsprintf("Startstate %d", startstatenum++);
  } virtual rule_class getclass() const
  {
    return Startstate;
  };

  // code generation
  virtual const char *generate_code();
};

/********************
  extern variable
  ********************/
extern int invariantnum;
extern bool has_invar;

//UPMURPHI_BEGIN
extern int goalnum;
extern bool has_goal;
//UPMURPHI_END

/********************
  class invariant
  ********************/
struct invariant:simplerule {
  // initializer
  invariant(ste * enclosures, expr * test);

  // supporting routines
  virtual void set_name(char *name)
  {
    this->name =
      name != NULL ? name : tsprintf("Invariant %d", invariantnum++);
  } virtual rule_class getclass() const
  {
    return Invar;
  };

  // code generation
  virtual const char *generate_code();
};

//UPMURPHI_BEGIN
/********************
  class plangoal
  ********************/
struct plangoal:simplerule {
  // initializer
  plangoal (ste * enclosures, expr * test);

  // supporting routines
  virtual void set_name (char *name)
  {
    this->name =
      name != NULL ? name : tsprintf ("Goal %d", goalnum++);
  }

  virtual rule_class getclass () const
  {
    return Goal;
  };

  // code generation
  virtual const char *generate_code ();
};
//UPMURPHI_END

/********************
  class quantrule
  ********************/
struct quantrule:rule {
  ste *quant;
  rule *rules;

  // initializer and supporting routines
  quantrule(ste * param, rule * rules);
  virtual rule_class getclass() const
  {
    return Quant;
  };

  // code generation
  virtual const char *generate_code();
};

/********************
  class chooserule for multiset
  ********************/
struct chooserule:rule {
  ste *index;
  designator *set;
  rule *rules;

  // initializer and supporting routines
  chooserule(ste * index, designator * set, rule * rules);

  virtual rule_class getclass() const
  {
    return Choose;
  };

  // code generation
  virtual const char *generate_code();
};

/********************
  class aliasrule
  ********************/
struct aliasrule:rule {
  ste *aliases;
  rule *rules;

  // initializer and supporting routines
  aliasrule(ste * aliases, rule * rules);
  virtual rule_class getclass() const
  {
    return Alias;
  };

  // code generation
  virtual const char *generate_code();
};

/********************
  extern variable
  ********************/
extern simplerule *error_rule;

#endif
