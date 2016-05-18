/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#include <set>
#include <iterator>
#include <map>

/************************************************************/
class StorageManager
{
 protected:
  FILE *transitions, *goals, *reachables, *actions, *plans, *starts, *errors, *graph;
  bool transitions_mode, goals_mode, reachables_mode, actions_mode, plans_mode, starts_mode, errors_mode, graph_mode;

 public:
  StorageManager();
  ~StorageManager();
  FILE *getTransitionsFile(bool write=FALSE);
  FILE *getGoalsFile(bool write=FALSE);
  FILE *getReachablesFile(bool write=FALSE);
  FILE *getActionsFile(bool write=FALSE);
  FILE *getPlansFile(bool write=FALSE);
  FILE *getErrorsFile(bool write=FALSE);
  FILE *getStartsFile(bool write=FALSE);
  FILE *getGraphFile(bool write=FALSE);
};

/************************************************************/
class StatsManager
{
 protected:
  FILE *stats;
  char *filename;
  int num_props;
 public:
  enum stat_type {NumStates=0,NumStartStates=1,NumGoals=2,NumTransitions=3,NumActions=4,NumErrors=5};
  StatsManager();
  ~StatsManager();

  void initStatsFile();
  unsigned long getStatistic(int key);
  void setStatistic(int key, unsigned long value);
};

/************************************************************/
class OutputManager
{
 protected:
  typedef void(OutputManager::*actionwriter)(unsigned long,RULE_INDEX_TYPE,unsigned long);

  FILE *reachables, *actions, *plans, *target;
  char *targetpath;
  std::map<int, std::set<int> > interference_map; // WP WP WP WP WP map of < action, <set of actions interfered with by key action> >


  void DumpState(unsigned long  index, bool complete=false, const char *separator=", ");
  void DumpBinaryState(unsigned long  index);
  void WriteRawAction(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to, bool printendstate);
  void WriteTextAction(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to, bool printstartstate, bool printendstate);
  void WriteBinaryAction(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to);
  void WriteCSVAction(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to);
  std::map<int, std::set<int> > CreateInterferenceMap();	// WP WP WP WP WP Collects all effects and preconditions and stores interfering actions in a map.
  bool CheckInterference(int rule1, int rule2);				// WP WP WP WP WP checks if rule's effects interfere with another rule's preconditions.

 private:
  inline bool needs_reachables_file()
  {
    return (args->output_fmt.mode == argoutput_fmt::PDDL_VeryVerbose ||
            args->output_fmt.mode == argoutput_fmt::Text_Verbose  ||
            args->output_fmt.mode == argoutput_fmt::Raw  ||
            args->output_fmt.mode == argoutput_fmt::CSV
#ifdef OBDD_COMPRESSION
            ||  args->output_fmt.mode == argoutput_fmt::OBDD
#endif
           );
  }

  inline bool needs_plans_file()
  {
    return (args->output_fmt.mode == argoutput_fmt::PDDL_VeryVerbose ||
            args->output_fmt.mode == argoutput_fmt::PDDL_Verbose  ||
            args->output_fmt.mode == argoutput_fmt::PDDL);
  }

 public:
  OutputManager();
  OutputManager(FILE *target);
  OutputManager(char *pathname);
  ~OutputManager();
  void WriteController(actionwriter aw, bool verbose = false);
#ifdef OBDD_COMPRESSION
  void WriteOBDDController();
#endif
  void WriteBinaryController();
  void WritePlans();

  void actionwriter_actiontext(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to)
  {
    WriteTextAction(from, rule, to, false, false);
  }
  void actionwriter_actiontext_verbose(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to)
  {
    WriteTextAction(from, rule, to, true, true);
  }
  void actionwriter_actionbin(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to)
  {
    WriteBinaryAction(from, rule, to);
  }
  void actionwriter_actioncsv(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to)
  {
    WriteCSVAction(from, rule, to);
  }

  static void lfseek(FILE *f, unsigned long n, unsigned long size, int mode);

};

