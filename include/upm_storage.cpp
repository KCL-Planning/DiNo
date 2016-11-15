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
/* StorageManager */
/************************************************************/
StorageManager::StorageManager() :
  transitions(NULL), goals(NULL), reachables(NULL), actions(NULL), plans(NULL), starts(NULL), errors(NULL),graph(NULL),
  transitions_mode(false), goals_mode(false), reachables_mode(false), actions_mode(false),plans_mode(false),starts_mode(false), errors_mode(false), graph_mode(false)
{

}

StorageManager::~StorageManager()
{
  char filename[255];
  if (transitions != NULL) {
    fclose(transitions);
    if (args->deleteintermediate.value) {
      strcpy(filename,PROTOCOL_NAME);
      strcat(filename,".transitions");
      remove(filename);
    }
  }
  if (goals != NULL) {
    fclose(goals);
    if (args->deleteintermediate.value) {
      strcpy(filename,PROTOCOL_NAME);
      strcat(filename,".goals");
      remove(filename);
    }
  }
  if (reachables != NULL) {
    fclose(reachables);
    if (args->deleteintermediate.value) {
      strcpy(filename,PROTOCOL_NAME);
      strcat(filename,".reachables");
      remove(filename);
    }
  }
  if (actions != NULL) {
    fclose(actions);
    if (args->deleteintermediate.value) {
      strcpy(filename,PROTOCOL_NAME);
      strcat(filename,".actions");
      remove(filename);
    }
  }
  if (plans != NULL) {
    fclose(plans);
    if (args->deleteintermediate.value) {
      strcpy(filename,PROTOCOL_NAME);
      strcat(filename,".plans");
      remove(filename);
    }
  }
  if (starts != NULL) {
    fclose(starts);
    if (args->deleteintermediate.value) {
      strcpy(filename,PROTOCOL_NAME);
      strcat(filename,".startstates");
      remove(filename);
    }
  }
  if (errors != NULL) {
    fclose(errors);
    if (args->deleteintermediate.value) {
      strcpy(filename,PROTOCOL_NAME);
      strcat(filename,".errors");
      remove(filename);
    }
  }
  if (graph != NULL) {
    fclose(graph);
    if (args->deleteintermediate.value) {
      strcpy(filename,PROTOCOL_NAME);
      strcat(filename,".graph");
      remove(filename);
    }
  }

}

FILE *StorageManager::getTransitionsFile(bool write)
{
  if (transitions != NULL) {
    if (!write || transitions_mode) return transitions;
    else fclose(transitions);
  }

  char filename[255];
  strcpy(filename,PROTOCOL_NAME);
  strcat(filename,".transitions");

  const char *mode = (write?"w+b":"rb");
  if ((transitions = fopen(filename,mode)) == NULL)
    Error.Notrace("Problems opening transitions file %s.", filename);

  transitions_mode = write;
  return transitions;
}


FILE *StorageManager::getGoalsFile(bool write)
{
  if (goals != NULL) {
    if (!write || goals_mode) return goals;
    else fclose(goals);
  }

  char filename[255];
  strcpy(filename,PROTOCOL_NAME);
  strcat(filename,".goals");

  const char *mode = (write?"w+b":"rb");
  if ((goals = fopen(filename,mode)) == NULL)
    Error.Notrace("Problems opening goals file %s.", filename);

  goals_mode = write;
  return goals;
}

FILE *StorageManager::getReachablesFile(bool write)
{
  if (reachables != NULL) {
    if (!write || reachables_mode) return reachables;
    else fclose(reachables);
  }

  char filename[255];
  strcpy(filename,PROTOCOL_NAME);
  strcat(filename,".reachables");

  const char *mode = (write?"w+b":"rb");
  if ((reachables = fopen(filename,mode)) == NULL)
    Error.Notrace("Problems opening reachables file %s.", filename);

  reachables_mode = write;
  return reachables;
}

FILE *StorageManager::getActionsFile(bool write)
{
  if (actions != NULL) {
    if (!write || actions_mode) return actions;
    else fclose(actions);
  }

  char filename[255];
  strcpy(filename,PROTOCOL_NAME);
  strcat(filename,".actions");

  const char *mode = (write?"w+b":"rb");
  if ((actions = fopen(filename,mode)) == NULL)
    Error.Notrace("Problems opening actions file %s.", filename);

  actions_mode = write;
  return actions;
}

FILE *StorageManager::getPlansFile(bool write)
{
  if (plans != NULL) {
    if (!write || plans_mode) return plans;
    else fclose(plans);
  }

  char filename[255];
  strcpy(filename,PROTOCOL_NAME);
  strcat(filename,".plans");

  const char *mode = (write?"w+b":"rb");
  if ((plans = fopen(filename,mode)) == NULL)
    Error.Notrace("Problems opening plans file %s.", filename);

  plans_mode = write;
  return plans;
}

FILE *StorageManager::getErrorsFile(bool write)
{
  if (errors != NULL) {
    if (!write || errors_mode) return errors;
    else fclose(errors);
  }

  char filename[255];
  strcpy(filename,PROTOCOL_NAME);
  strcat(filename,".errors");

  const char *mode = (write?"w+b":"rb");
  if ((errors = fopen(filename,mode)) == NULL)
    Error.Notrace("Problems opening error states file %s.", filename);

  errors_mode = write;
  return errors;
}

FILE *StorageManager::getStartsFile(bool write)
{
  if (starts != NULL) {
    if (!write || starts_mode) return starts;
    else fclose(starts);
  }

  char filename[255];
  strcpy(filename,PROTOCOL_NAME);
  strcat(filename,".startstates");

  const char *mode = (write?"w+b":"rb");
  if ((starts = fopen(filename,mode)) == NULL)
    Error.Notrace("Problems opening start states file %s.", filename);

  starts_mode = write;
  return starts;
}

FILE *StorageManager::getGraphFile(bool write)
{
  if (graph != NULL) {
    if (!write || graph_mode) return graph;
    else fclose(graph);
  }

  char filename[255];
  strcpy(filename,PROTOCOL_NAME);
  strcat(filename,".graph");

  const char *mode = (write?"w+b":"rb");
  if ((graph = fopen(filename,mode)) == NULL)
    Error.Notrace("Problems opening graph file %s.", filename);

  starts_mode = write;
  return graph;
}

/************************************************************/
/* StatsManager */
/************************************************************/
StatsManager::StatsManager() :
  stats(NULL), num_props(6)
{
  initStatsFile();
}

StatsManager::~StatsManager()
{
  char filename[255];
  if (stats != NULL) {
    fclose(stats);
    if (args->deleteintermediate.value) {
      strcpy(filename,PROTOCOL_NAME);
      strcat(filename,".properties");
      remove(filename);
    }
  }
}

void StatsManager::initStatsFile()
{
  filename = new char[strlen(PROTOCOL_NAME)+14];
  strcpy(filename,PROTOCOL_NAME);
  strcat(filename,".properties");
  if ((stats = fopen(filename,"w+b")) == NULL)
    Error.Notrace("Problems opening properties file %s.", filename);

  fseek(stats,0,SEEK_END);
  if (ftell(stats)/sizeof(unsigned long) != num_props) {
    unsigned long dummy = 0;
    fseek(stats,0,SEEK_SET);
    for(int i=0; i<num_props; ++i)
      fwrite(&dummy,sizeof(unsigned long),1,stats);
  }
}

unsigned long StatsManager::getStatistic(int key)
{
  unsigned long value;

  fseek(stats,key*sizeof(unsigned long),SEEK_SET);
  if (fread(&value,sizeof(unsigned long),1,stats)==1) return value;
  else {
    Error.Notrace("Problems getting property %d", key);
    return 0;
  }
}

void StatsManager::setStatistic(int key, unsigned long value)
{
  fseek(stats,key*sizeof(unsigned long),SEEK_SET);
  fwrite(&value,sizeof(unsigned long),1,stats);
}

/************************************************************/
/* OutputManager */
/************************************************************/
OutputManager::OutputManager() : target(stdout), targetpath(NULL), reachables(NULL)
{

  interference_map = CreateInterferenceMap(); // WP WP WP WP WP

  if(needs_reachables_file()) reachables = Storage->getReachablesFile(false);
  actions = Storage->getActionsFile(false);
  if(needs_plans_file()) plans = Storage->getPlansFile(false);
}

OutputManager::OutputManager(FILE *_target) : target(_target), targetpath(NULL), reachables(NULL)
{
  interference_map = CreateInterferenceMap(); // WP WP WP WP WP

  if(needs_reachables_file()) reachables = Storage->getReachablesFile(false);
  actions = Storage->getActionsFile(false);
  if(needs_plans_file()) plans = Storage->getPlansFile(false);
}

OutputManager::OutputManager(char *_targetpath)  : reachables(NULL)
{
  targetpath = new char[strlen(_targetpath)+1];
  strcpy(targetpath,_targetpath);

  interference_map = CreateInterferenceMap(); // WP WP WP WP WP

  if ((target = fopen(targetpath,"wb")) == NULL)
    Error.Notrace("Problems opening output file %s.", targetpath);

  if(needs_reachables_file()) reachables = Storage->getReachablesFile(false);
  actions = Storage->getActionsFile(false);
  if(needs_plans_file()) plans = Storage->getPlansFile(false);
}


OutputManager::~OutputManager()
{

  if (targetpath!=NULL) {
    delete[] targetpath;
    fclose(target);
  }
}


#ifdef OBDD_COMPRESSION
void OutputManager::WriteOBDDController()
{
  FILE* temp_target, *real_target;
  if ((temp_target = tmpfile()) == NULL) {
    Error.Notrace("Internal: Error creating temporary compression file.");
  }
  real_target = target;
  target = temp_target;
  WriteBinaryController();
  target = real_target;

  fseek(temp_target,0,SEEK_SET);
  WriteOBDD(temp_target,
            STATE_RULE,
            MODE_SAME,
            MODE_SAME,
            CUDD_REORDER_RANDOM, //riordinamento cudd
            0,  //prima transizione
            550000,  //ultima transizione
            real_target,
            DDDMP_MODE_BINARY,
            NULL,
            1);
  fclose(temp_target);
}
#endif

void OutputManager::WriteBinaryController()
{
  unsigned long nactions;
  unsigned int ssize;


  //fseek(actions,0,SEEK_END);
  //nactions = (unsigned long)(ftell(actions)/(sizeof(unsigned long)+sizeof(unsigned long)+sizeof(RULE_INDEX_TYPE)));
  nactions = Stats->getStatistic(StatsManager::NumActions);
  ssize = sizeof(state);
  fwrite(&nactions,sizeof(unsigned long),1,target);
  fwrite(&ssize,sizeof(unsigned int),1,target);
  WriteController(&OutputManager::actionwriter_actionbin,false);
}


// WP WP WP WP WP WP
// generate an map of action interference
std::map<int, std::set<int> > OutputManager::CreateInterferenceMap(){

	std::map<int, std::set<int> > interf_map;

	std::set<std::pair<mu_0_boolean*, int> >::iterator it_prec;
	std::set<std::pair<mu_0_boolean*, int> >::iterator it_eff;

	for(int iidx = 0; iidx < RULES_IN_WORLD; iidx++){

		std::set<int> interferees;

		std::set<std::pair<mu_0_boolean*, int> > inf_effs = Rules->generator->effects_bool_interference(iidx);

		for(int iidx2 = 0; iidx2 < RULES_IN_WORLD; iidx2++){

			if (inf_effs.size() <= 0) break;
			for (it_eff=inf_effs.begin(); it_eff!=inf_effs.end(); ++it_eff){

				std::set<std::pair<mu_0_boolean*, int> > inf_preconds = Rules->generator->precond_bool_interference(iidx2);
				if (inf_preconds.size() <= 0) break;
				for(it_prec=inf_preconds.begin(); it_prec!=inf_preconds.end(); it_prec++){
					if ((it_eff->first == it_prec->first)){
						interferees.insert(iidx2);
					}
				}
			}
		}
		interf_map.insert(std::make_pair(iidx, interferees));
	}

	return interf_map;
}

// WP WP WP WP WP
// Check if an action interferes with another action
bool OutputManager::CheckInterference(int rule1, int rule2){

	if(interference_map.size() <= 0 || interference_map[rule1].size() <= 0) return false;
	else if (interference_map[rule1].count(rule2) > 0){
		return true;
	}

	return false;
}



void OutputManager::WriteController(actionwriter aw, bool verbose)
{
  unsigned long from, to;
  RULE_INDEX_TYPE rule;
  int weight,duration;

  fseek(actions,0,SEEK_SET);
  while(!feof(actions) && (fread(&from,sizeof(unsigned long),1,actions)==1)) {
    (void) fread(&rule,sizeof(RULE_INDEX_TYPE),1,actions);
    (void) fread(&to,sizeof(unsigned long),1,actions);
#ifdef VARIABLE_WEIGHT
    (void) fread(&weight,sizeof(int),1,actions);
#else
    weight = Rules->RuleWeight(rule);
#endif
#ifdef VARIABLE_DURATION
    (void) fread(&duration,sizeof(int),1,actions);
#else
    duration = Rules->RuleDuration(rule);
#endif
    //write only controller actions!
    if (verbose || Rules->RulePDDLClass(rule) == RuleManager::Action || Rules->RulePDDLClass(rule) == RuleManager::DurativeStart) {
      (this->*aw)(from,rule,to);
    }
  }
}

void OutputManager::WritePlans()
{
  unsigned long from, to;
  int rule_weight;
  int rule_duration;
  RULE_INDEX_TYPE rule;
  unsigned weight, nplan=0;
  double time;
  unsigned temp,dur,iplan,lplan,max_lplan=0;
  edge* plan;
  bool first_happening_added = false;
  //double epsilon = min(DISCRETIZATION,EPSILON_TIME_SEPARATION);

  fseek(plans,0,SEEK_SET);
  //ancora una volta prescansioniamo per risparmiare spazio, ma sarebbe molto più comodo scrivercelo
  while(!feof(plans) && (fread(&from,sizeof(unsigned long),1,plans)==1)) {
    lplan=1;
    while(!feof(plans) && (fread(&rule,sizeof(RULE_INDEX_TYPE),1,plans)==1)) {
#ifdef VARIABLE_WEIGHT
      (void) fread(&rule_weight,sizeof(int),1,plans); //skip
#else
      rule_weight = Rules->RuleWeight(rule);
#endif
#ifdef VARIABLE_DURATION
      (void) fread(&rule_duration,sizeof(int),1,plans); //skip
#else
      rule_duration = Rules->RuleDuration(rule);
#endif
      (void) fread(&to,sizeof(unsigned long),1,plans);
      if (from != to) {
        from=to;
        lplan++;
      } else {
        break;
      }
      if (lplan>max_lplan) max_lplan=lplan;
    }
  }

  plan = new edge[max_lplan];
  MEMTRACKALLOC

  fseek(plans,0,SEEK_SET);
  while(!feof(plans) && (fread(&from,sizeof(unsigned long),1,plans)==1)) {
    //load plan
    lplan=1;
    plan[0].to=from; //first edge used only to store initial state
    while(!feof(plans) && (fread(&rule,sizeof(RULE_INDEX_TYPE),1,plans)==1)) {
#ifdef VARIABLE_WEIGHT
      (void) fread(&rule_weight,sizeof(int),1,plans); //skip
#else
      rule_weight = Rules->RuleWeight(rule);
#endif
#ifdef VARIABLE_DURATION
      (void) fread(&rule_duration,sizeof(int),1,plans); //skip
#else
      rule_duration = Rules->RuleDuration(rule);
#endif
      (void) fread(&to,sizeof(unsigned long),1,plans);
      if (from != to) {
        plan[lplan].to=to;
        plan[lplan].rule=rule;
#ifdef VARIABLE_WEIGHT
        plan[lplan].weight=rule_weight;
#endif
#ifdef VARIABLE_DURATION
        plan[lplan].duration=rule_duration;
#endif
        lplan++;
        from=to;
      } else {
        break;
      }
    }
    //write plan
    time=0;
    weight=0;
    fprintf(target,"; --Plan #%05d--------------------------\n",++nplan);
    if(args->output_fmt.mode == argoutput_fmt::PDDL_VeryVerbose) {
      fprintf(target,"; --Start state (%07ld)----------------\n; ",plan[0].to);
      DumpState(plan[0].to);
      fprintf(target,"\n");
    }
    fprintf(target,"; -- Discretisation: %0.3f----------------\n",DISCRETIZATION);
    fprintf(target,"; ---------------------------------------\n");
    for(iplan=1; iplan<lplan; ++iplan) {
      //ACTIONS
     // if (Rules->RulePDDLClass(plan[iplan].rule) == RuleManager::Action) {
     //  if (first_happening_added && (CheckInterference(plan[iplan-1].rule, plan[iplan].rule)== true)) time += EPSILON_TIME_SEPARATION; //separation  // WP WP WP WP WP if the previous action interferes with the current action - add epsilon
	//	    else first_happening_added=true;


      if (Rules->RulePDDLClass(plan[iplan].rule) == RuleManager::Action) {
        if (first_happening_added && ((CheckInterference(plan[iplan-1].rule, plan[iplan].rule)== true) ||
        		(Rules->RulePDDLClass(plan[iplan-1].rule) == RuleManager::Action) ||
        				(Rules->RulePDDLClass(plan[iplan-1].rule) == RuleManager::DurativeStart)))
        					time += EPSILON_TIME_SEPARATION; //separation
		    else first_happening_added=true;

        fprintf(target,"%0.3f: %s [%0.3f]",time,Rules->RulePDDLName(plan[iplan].rule),(double)DISCRETIZATION *(double)
#ifdef VARIABLE_DURATION
                plan[iplan].duration
#else
                Rules->RuleDuration(plan[iplan].rule)
#endif
               );
        if(args->output_fmt.mode == argoutput_fmt::PDDL_Verbose || args->output_fmt.mode == argoutput_fmt::PDDL_VeryVerbose) {
          fprintf(target,"; weight = %0.3d",
#ifdef VARIABLE_WEIGHT
                  plan[iplan].weight
#else
                  Rules->RuleWeight(plan[iplan].rule)
#endif
                 );
        }
        fprintf(target,"\n");
        //DURATIVE ACTIONS
      } //else if (Rules->RulePDDLClass(plan[iplan].rule) == RuleManager::DurativeStart) {
        //if (first_happening_added && (CheckInterference(plan[iplan-1].rule, plan[iplan].rule)== true)) time += EPSILON_TIME_SEPARATION; //separation	// WP WP WP WP WP if the previous action interferes with the current action - add epsilon
    		//else first_happening_added=true;

else if (Rules->RulePDDLClass(plan[iplan].rule) == RuleManager::DurativeStart) {
          if (first_happening_added && ((CheckInterference(plan[iplan-1].rule, plan[iplan].rule)== true) ||
          		(Rules->RulePDDLClass(plan[iplan-1].rule) == RuleManager::Action) ||
          				(Rules->RulePDDLClass(plan[iplan-1].rule) == RuleManager::DurativeStart)))
        	  	  	  	  	  time += EPSILON_TIME_SEPARATION; //separation
    		else first_happening_added=true;

        dur =
#ifdef VARIABLE_DURATION
          plan[iplan].duration;
#else
          Rules->RuleDuration(plan[iplan].rule);
#endif
        for (temp=iplan+1; temp<lplan; ++temp) {
          if (Rules->RulePDDLClass(plan[temp].rule) == RuleManager::DurativeEnd &&
              !strcmp(Rules->RulePDDLName(plan[iplan].rule),Rules->RulePDDLName(plan[temp].rule))) {
            break;
          } else {
            dur+=
#ifdef VARIABLE_DURATION
              plan[temp].duration;
#else
              Rules->RuleDuration(plan[temp].rule);
#endif
          }
        }
        fprintf(target,"%0.3f: %s [%0.3f]",time,Rules->RulePDDLName(plan[iplan].rule),(double)DISCRETIZATION *(double)dur);
        if(args->output_fmt.mode == argoutput_fmt::PDDL_Verbose || args->output_fmt.mode == argoutput_fmt::PDDL_VeryVerbose) {
          fprintf(target,"; weight = %0.3d",
#ifdef VARIABLE_WEIGHT
                  plan[iplan].weight
#else
                  Rules->RuleWeight(plan[iplan].rule)
#endif
                 );
        }
        fprintf(target,"\n");
        //EVENTS & co.
      } else if(args->output_fmt.mode == argoutput_fmt::PDDL_Verbose || args->output_fmt.mode == argoutput_fmt::PDDL_VeryVerbose) {
        fprintf(target,"; %0.3f: %s: %s [%0.3f]; weight = %0.3d\n",time,Rules->RulePDDLClassName(plan[iplan].rule),Rules->RulePDDLName(plan[iplan].rule),(double)DISCRETIZATION *(double)
#ifdef VARIABLE_DURATION
                plan[iplan].duration
#else
                Rules->RuleDuration(plan[iplan].rule)
#endif
                ,
#ifdef VARIABLE_WEIGHT
                plan[iplan].weight
#else
                Rules->RuleWeight(plan[iplan].rule)
#endif
               );
      }
      if(args->output_fmt.mode == argoutput_fmt::PDDL_VeryVerbose) {
        fprintf(target,"; ");
        DumpState(plan[iplan].to);
        fprintf(target,"\n");
      }
      time+= (double)DISCRETIZATION *(double)
#ifdef VARIABLE_DURATION
             plan[iplan].duration;
#else
             Rules->RuleDuration(plan[iplan].rule);
#endif
      weight+=
#ifdef VARIABLE_WEIGHT
        plan[iplan].weight;
#else
        Rules->RuleWeight(plan[iplan].rule);
#endif


    }
    fprintf(target,"; ---------------------------------------\n");
    fprintf(target,"; --Plan duration: %0.3f, weight: %04d----\n",time,weight);
    fprintf(target,"; ---------------------------------------\n\n");
  }


  // WP WP WP WP WP WP WP WP WP WP
  // deleted to avoid memory leaks!
  // delete plan;

}


void OutputManager::WriteRawAction(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to, bool printendstate)
{

  if (printendstate) fprintf(target,"From state %07lu apply rule %lu to state %07lu\n",from,rule,to);
  else fprintf(target,"From state %07lu apply rule %02lu\n",from,rule);

}

void OutputManager::WriteTextAction(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to, bool printstartstate, bool printendstate)
{

  fprintf(target,"From state (%07ld)",from);
  if (printstartstate) {
    fprintf(target,"{\n");
    DumpState(from);
    fprintf(target,"}\n");
  }
  fprintf(target,"Apply rule \"%s\"",Rules->RulePDDLName(rule));
  if (printendstate) {
    fprintf(target," to state (%07ld) {\n",to);
    DumpState(to);
    fprintf(target,"}\n");
  } else {
    fprintf(target,"\n");
  }
  fprintf(target,"-----------------\n");

}

void OutputManager::WriteBinaryAction(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to)
{

  DumpBinaryState(from);
  fwrite(&rule,sizeof(RULE_INDEX_TYPE),1,target);
}

void OutputManager::WriteCSVAction(unsigned long from, RULE_INDEX_TYPE rule, unsigned long to)
{

  DumpState(from,false,";");
  fprintf(target,"%s\n",Rules->RulePDDLName(rule));
}

void OutputManager::DumpState(unsigned long index, bool complete, const char *separator)
{
  static state *originalstate = new state;
  MEMTRACKALLOC

  theworld.to_state(NULL); // trick : marks variables in world
  theworld.clear();
  StateCopy(originalstate,workingstate);
  lfseek(reachables,index,sizeof(state),SEEK_SET);
  if (fread(workingstate,sizeof(state),1,reachables)==1) {
    if (complete) theworld.print(target,separator);
    else theworld.pddlprint(target,separator);
    //fprintf(target,"\n");
  } else Error.Notrace("Internal: unable to reload state from reachables file");
  StateCopy(workingstate,originalstate);
}

void OutputManager::DumpBinaryState(unsigned long index)
{
  static state *originalstate = new state;
  MEMTRACKALLOC

  StateCopy(originalstate,workingstate);
  lfseek(reachables,index,sizeof(state),SEEK_SET);
  if (fread(workingstate,sizeof(state),1,reachables)==1) fwrite(workingstate,sizeof(state),1,target);
  else Error.Notrace("Internal: unable to reload state from reachables file");
  StateCopy(workingstate,originalstate);
}

void OutputManager::lfseek(FILE *f, unsigned long n, unsigned long size, int mode)
{
  if (mode == SEEK_END) {
    Error.Notrace("Internal: lfseek does not support SEEK_END mode");
  }
  unsigned long maxn = ULONG_MAX / size;
  while(n>=0) {
    if (maxn > n) {
      fseek(f,n*size,mode);
      break; //tutto fatto
    } else {
      fseek(f,maxn*size,mode);
      n-=maxn;
      mode = SEEK_CUR;
    }
  }
}

