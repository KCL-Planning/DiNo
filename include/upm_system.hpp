/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#include <sys/stat.h>

/************************************************************/

// Uli: added omission probability calculation & printing

class StateManager
{
  state_set *the_states;  // the set of states found.
  state_queue *queue;     // the queue for active states.

  FILE *transitions;
  FILE *goals;
  FILE *errors;

  unsigned long NumStates;
  unsigned long num_goals;
  unsigned long num_errors;

  // Uli: for omission probability calculation
  long statesCurrentLevel;   // number of states in the level that
  //  is currently expanded
  long statesNextLevel;      // number of states in the next level
  long currentLevel;         // level that is currently expanded
  //  (startstates: level 0)
  double pno;   // Pr(particular state not omitted)

  double harmonic(double n);   // return harmonic number H_n

 public:
  StateManager(bool createqueue, unsigned long NumStates);
  ~StateManager();

  bool Add(state * s, bool valid, bool permanent, unsigned long *index=NULL, bool *iserror=NULL);
  void AddGoal(unsigned long s);
  void AddTransitions(unsigned long from, unsigned long *to, RULE_INDEX_TYPE *rule
#ifdef VARIABLE_WEIGHT
                      ,int *weight
#endif
#ifdef VARIABLE_DURATION
                      ,int *duration
#endif
                      ,int num);
  void AddErrorTransition(unsigned long from, unsigned long to, RULE_INDEX_TYPE rule
#ifdef VARIABLE_WEIGHT
                          ,int weight
#endif
#ifdef VARIABLE_DURATION
                          ,int duration
#endif
                         );
  bool QueueIsEmpty();
  state_and_index * QueueTop();
  state_and_index * QueueDequeue();
  RULE_INDEX_TYPE NextRuleToTry();   // Uli: unsigned short -> unsigned
  void NextRuleToTry(RULE_INDEX_TYPE r);

  // Uli: routines for omission probability calculation & printing
  void CheckLevel();
  void PrintProb();

  void print_all_states();
  void print_trace(StatePtr p);   // changes by Uli
  void print_trace_aux(StatePtr p);

  inline unsigned long Capacity()
  {
    return the_states->Capacity();
  }
  inline unsigned long NumElts();
  inline unsigned long MaxElts();
  inline unsigned long NumGoals();
  inline unsigned long NumErrors();
  inline bool TableIsFull(); //GDP
  inline unsigned long NumStatesInGraph();
  inline unsigned long NumEltsReduced();   // Uli
  inline unsigned long QueueNumElts();
  inline long CurrentLevel()
  {
    return currentLevel;
  }

};

class StartStateGenerator;

/************************************************************/
class StartStateManager
{

  unsigned short what_startstate; // for info at Error
  StartStateGenerator * generator;
  randomGen random;   // Uli: random number generator
  FILE *starts;
 public:
  static unsigned short numstartstates;
  StartStateManager();
  state * RandomStartState();
  state * LastStartState();
  void AllStartStates();
  state * NextStartState();
  state * StartState();
  char * LastStateName();
  char * StateName(StatePtr p);   // changes by Uli
  void AddStartState(unsigned long s);
};

class NextStateGenerator;

/************************************************************/
class RuleManager
{
  RULE_INDEX_TYPE what_rule;       // for execution and info at Error
  unsigned long rules_fired;
  unsigned long num_transitions; //actual number of distinct non-error transitions explored
  unsigned long * NumTimesFired; // array for storing the number of times fired for each rule
#ifdef DYNDBG
  dynBitVec *always_execute_rules; //bit vector for storing user 'always' answers in debug mode
  dynBitVec *always_exclude_rules; //bit vector for storing user 'exclude' answers in debug mode
#endif
#ifdef HAS_CLOCK
  double max_clock_so_far;
#endif
//  NextStateGenerator * generator; // // WP WP WP WP WP made public for RPG purposes

  setofrules * EnabledTransition();
  bool AllNextStates(setofrules * fire);
  state * NextState();
  state * HeuristicNextState(); // WP WP WP WP WP
  randomGen random;   // Uli: random number generator

  // Vitaly's additions
  int minp; 	// Minimum priority among all rules applicable
  // in the current state
  // End of Vitaly's additions

 public:
  enum rule_pddlclass //same as in struct simplerule!
  { Action, Event, Clock, DurativeStart, DurativeEnd, Other }; //UPMURPHI_BEGIN_END

  RuleManager();
  ~RuleManager();
  state * RandomNextState();
  state * SeqNextState();
  NextStateGenerator * generator; // // WP WP WP WP WP made public for RPG purposes
  bool AllNextStates();
  void ResetRuleNum();
  void SetRuleNum(RULE_INDEX_TYPE r);
  char * LastRuleName();
  const char * RuleName(RULE_INDEX_TYPE rule);
  const char * RulePDDLName(RULE_INDEX_TYPE rule);
  int RuleDuration(RULE_INDEX_TYPE rule);
  int RuleWeight(RULE_INDEX_TYPE rule);
  rule_pddlclass RulePDDLClass(RULE_INDEX_TYPE rule);
  const char * RulePDDLClassName(RULE_INDEX_TYPE rule);
  unsigned long NumRulesFired();
  unsigned long NumTransitions();
#ifdef HAS_CLOCK
  double MaxClockSoFar();
#endif

  void print_rules_information();
  void print_world_to_state(StatePtr p, bool fullstate);
  // changes by Uli
};

/************************************************************/
class PropertyManager
{
  unsigned short what_invariant;  // for info at Error
  unsigned short what_goal;
 public:
  PropertyManager();
  bool CheckInvariants();
  int CheckGoals();
  char * LastInvariantName();
  char * LastGoalName();
};

/************************************************************/
class SymmetryManager
{
  state_set *debug_sym_the_states;  // the set of states found without sym.
 public:
  SymmetryManager();
};

/************************************************************/
class POManager // Partial Order
{
  rule_matrix *conflict_matrix;
 public:
  POManager();
};

/************************************************************/
class AlgorithmManager
{
 protected:
  char phase;
  unsigned long num_controlled_states;
  unsigned long num_plans;

  unsigned max_plan_length;
  unsigned min_plan_length;
  float avg_plan_length;

  unsigned max_plan_duration;
  unsigned min_plan_duration;
  float avg_plan_duration;

  unsigned max_plan_weight;
  unsigned min_plan_weight;
  float avg_plan_weight;


  unsigned Build_Plan_From(unsigned long from, FILE *target);

 public:
  AlgorithmManager();
  void Plan();
  void Explore_bfs();
  void Explore_dfs();
  void Build_Dynamics();
  void Find_Paths();
  void Find_Paths2(); //debug
  void Collect_Plans();
  void Output_Results();
#if defined(VAL_PATHNAME) && defined(DOMAIN_FILENAME) && defined(PROBLEM_FILENAME) && defined(DISCRETIZATION)
  void Validate_Results();
#endif

  void simulate();
  char Phase()
  {
    return phase;
  }
  unsigned long ControlledStates()
  {
    return num_controlled_states;
  }
  //stats
  unsigned long NumPlans()
  {
    return num_plans;
  }
  unsigned MaxPlanLen()
  {
    return max_plan_length;
  }
  unsigned MinPlanLen()
  {
    return min_plan_length;
  }
  float AvgPlanLen()
  {
    return avg_plan_length;
  }
  //
  unsigned MaxPlanDuration()
  {
    return max_plan_duration;
  }
  unsigned MinPlanDuration()
  {
    return min_plan_duration;
  }
  float AvgPlanDuration()
  {
    return avg_plan_duration;
  }
  //
  unsigned MaxPlanWeight()
  {
    return max_plan_weight;
  }
  unsigned MinPlanWeight()
  {
    return min_plan_weight;
  }
  float AvgPlanWeight()
  {
    return avg_plan_weight;
  }
};

/************************************************************/
StartStateManager *StartState=NULL;  // manager for all startstate related operation
RuleManager *Rules=NULL;             // manager for all rule related operation
PropertyManager *Properties=NULL;    // manager for all property related operation
StateManager *StateSet=NULL;        // manager for all state related information
GraphManager *StateGraph=NULL;
SymmetryManager *Symmetry=NULL;  // manager for all symmetry information
POManager *PO=NULL;                  // manager for all symmetry information
ReportManager *Reporter=NULL;        // manager for all diagnostic messages
StorageManager *Storage=NULL;
StatsManager *Stats=NULL;
OutputManager *Output=NULL;
AlgorithmManager *Algorithm=NULL;    // manager for all algorithm related issue

Error_handler Error;       // general error handler.
argclass *args;            // the record of the arguments.
state *curstate;        // current state at the beginning of the rule-firing
unsigned long curstateindex; //GDP
//unsigned long curstatecounter; //GDP
state *const workingstate = new state;   // Uli: buffer for doing all state
//      manipulation
world_class theworld;          // the set of global variables.
int category;                  // working on startstate, rule or invariant

#ifdef HASHC
TraceFileManager* TraceFile;   // Uli: manager for trace info file
#endif
unsigned long NumCurState;     // Uli: number of the current state for trace
//      info file

