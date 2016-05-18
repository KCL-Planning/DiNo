/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/


/************************************************************/
/* GraphManager */
/************************************************************/


GraphManager::GraphManager(unsigned long size) : num_states(size), num_transitions(0)
{
  out_degree = new RULE_INDEX_TYPE[num_states];
  MEMTRACKALLOC
  memset(out_degree, 0, sizeof(RULE_INDEX_TYPE)*num_states);
  max_transitions = (unsigned long)((double) args->mem.value * 8 / (8 * sizeof(edge) + gPercentActiveStates * 8 * state_queue::BytesForOneState()));
}

GraphManager::~GraphManager()
{
  delete[] out_degree;
}

void GraphManager::inc_outgoing_num(unsigned long from)
{
  ++out_degree[from];
  
}

RULE_INDEX_TYPE GraphManager::OutDegreeMin()
{
  RULE_INDEX_TYPE min = INT_MAX;
  for(unsigned long i = 0; i< num_states; ++i)
    if (out_degree[i]<min) min = out_degree[i];
  return min;
}

RULE_INDEX_TYPE GraphManager::OutDegreeMax()
{
  RULE_INDEX_TYPE max = 0;
  for(unsigned long i = 0; i< num_states; ++i)
    if (out_degree[i]>max) max = out_degree[i];
  return max;
}

double GraphManager::OutDegreeAvg()
{
  double avg = 0;
  for(unsigned long i = 0; i< num_states; ++i)
    avg += (double)out_degree[i]/(double)num_states;
  return avg;
}

unsigned long GraphManager::OutDegreeTot()
{
  if (num_transitions==0) {
    for(unsigned long i = 0; i< num_states; ++i)
      num_transitions += out_degree[i];
  }
  return num_transitions;
}

unsigned long GraphManager::NumStates()
{
  return num_states;
}

int GraphManager::weightasc(const void* t1, const void* t2)
{
  return
#ifdef VARIABLE_WEIGHT
    (((edge*)t1)->weight > ((edge*)t2)->weight)?1:
    ((((edge*)t1)->weight < ((edge*)t2)->weight)?-1:0);
#else
    (Rules->RuleWeight(((edge*)t1)->rule) > Rules->RuleWeight(((edge*)t2)->rule))?1:
    (Rules->RuleWeight((((edge*)t1)->rule) < Rules->RuleWeight(((edge*)t2)->rule))?-1:0);
#endif
}

int GraphManager::weightdesc(const void* t1, const void* t2)
{
  return
#ifdef VARIABLE_WEIGHT
    (((edge*)t1)->weight < ((edge*)t2)->weight)?1:
    ((((edge*)t1)->weight > ((edge*)t2)->weight)?-1:0);
#else
    (Rules->RuleWeight(((edge*)t1)->rule) < Rules->RuleWeight(((edge*)t2)->rule))?1:
    (Rules->RuleWeight((((edge*)t1)->rule) > Rules->RuleWeight(((edge*)t2)->rule))?-1:0);
#endif
}



/////////////////////////////////////////////////

MemGraphManager::MemGraphManager(unsigned long size) : GraphManager(size)
{
  outgoing = new edge*[num_states];
  MEMTRACKALLOC
  memset(outgoing, 0, sizeof(edge*)*num_states);
}

MemGraphManager::~MemGraphManager()
{
  for(unsigned long i = 0; i<num_states; ++i)
    if (outgoing[i] != NULL) delete[] outgoing[i];
  delete[] outgoing;
}

unsigned long MemGraphManager::estimate_max_transitions()
{
  return (unsigned long)((double) args->mem.value * 8 / (8 * sizeof(edge) + gPercentActiveStates * 8 * state_queue::BytesForOneState()));
}

void MemGraphManager::set_outgoing_num(unsigned long from, unsigned long num)
{
  outgoing[from] = new edge[num];
  MEMTRACKALLOC
  out_degree[from] = 0;
  num_transitions = 0; //invalidate
}

void MemGraphManager::setup_outgoing_lists()
{
  
  for(unsigned long i = 0; i< num_states; ++i) {
    
    outgoing[i] = new edge[out_degree[i]];
    MEMTRACKALLOC
    out_degree[i] = 0;
    
  }
  num_transitions = 0;
}

void MemGraphManager::add_outgoing(unsigned long from, unsigned long to, RULE_INDEX_TYPE rule
#ifdef VARIABLE_WEIGHT
                                   , int weight
#endif
#ifdef VARIABLE_DURATION
                                   , int duration
#endif
                                  )
{
  if (outgoing[from] != NULL) {
    outgoing[from][out_degree[from]].to = to;
    outgoing[from][out_degree[from]].rule = rule;
#ifdef VARIABLE_WEIGHT
    outgoing[from][out_degree[from]].weight = weight;
#endif
#ifdef VARIABLE_DURATION
    outgoing[from][out_degree[from]].duration = duration;
#endif
    out_degree[from]++;
    num_transitions++;
  } else {
    Error.Notrace("Accessing an undeclared outgoing list.");
  }
}

edge *MemGraphManager::reorder_list(unsigned long from, bool asc)
{
  if (outgoing[from] != NULL) {
    qsort(outgoing[from],out_degree[from],sizeof(edge),(asc?(GraphManager::weightasc):(GraphManager::weightdesc)));
    return &(outgoing[from][0]);
  } else {
    Error.Notrace("Accessing an undeclared outgoing list.");
  }
}

edge* MemGraphManager::GetOutgoing(unsigned long from, RULE_INDEX_TYPE num)
{
  if (outgoing[from] != NULL && num < out_degree[from] && num >= 0) {
    return &(outgoing[from][num]);
  } else {
    Error.Notrace("Accessing an undeclared edge from list.");
  }
}

void MemGraphManager::print()
{
  for(unsigned long i = 0; i<num_states; ++i) {
    cout << endl << "State " << i << ": ";
    for(RULE_INDEX_TYPE j = 0; j<out_degree[i]; ++j) {
      cout << "R" << outgoing[i][j].rule << "(W=" <<
#ifdef VARIABLE_WEIGHT
           outgoing[i][j].weight
#else
           Rules->RuleWeight(outgoing[i][j].rule)
#endif
           << ",D=" <<
#ifdef VARIABLE_DURATION
           outgoing[i][j].duration
#else
           Rules->RuleDuration(outgoing[i][j].rule)
#endif
           << ")->" << outgoing[i][j].to
           << " ";
    }
  }
}

//////////////////////////////////////////////////////////////


DiskGraphManager::DiskGraphManager(unsigned long size) : GraphManager(size)
{
  //se aperto in lettura/scrittura inizializzare tutto a zero, altrimenti caricare num_states e out_degree dal file
  graph = Storage->getGraphFile(true);
  out_position = new unsigned long[num_states];
  MEMTRACKALLOC
  memset(out_position, 0, sizeof(unsigned long)*num_states);

  max_transitions = ULONG_MAX; //a.k.a. infinity?

  fseek(graph,0,SEEK_SET);
  //write number of states
  fwrite(&num_states,sizeof(unsigned long),1,graph);
}

DiskGraphManager::~DiskGraphManager()
{
  delete[] out_position;
}

unsigned long DiskGraphManager::estimate_max_transitions()
{
  return ULONG_MAX; //aka infinity
}


void DiskGraphManager::seek_to_edge(unsigned long from, RULE_INDEX_TYPE n)
{
  fseek(graph,sizeof(unsigned long),SEEK_SET); //contatore stati
  OutputManager::lfseek(graph,num_states,sizeof(RULE_INDEX_TYPE),SEEK_CUR); //out_degrees
  OutputManager::lfseek(graph,out_position[from]+n,sizeof(edge),SEEK_CUR); //edge offset
}

edge *DiskGraphManager::get_edge(unsigned long from, RULE_INDEX_TYPE n)
{
  if (last_from != from || last_n != n-1) {
    seek_to_edge(from,n);
    last_from = from;
  }
  if (fread(&edge_buffer,sizeof(edge),1,graph)==1) return &edge_buffer;
  else Error.Notrace("Internal: unable to reload edge from graph file");
  last_n = n;

}

void DiskGraphManager::set_edge(unsigned long from, RULE_INDEX_TYPE n, edge *e)
{
  if (last_from != from || last_n != n-1) {
    seek_to_edge(from,n);
    last_from = from;
  }
  if (fwrite(e,sizeof(edge),1,graph)!=1)
    Error.Notrace("Internal: unable to store edge to graph file");
  last_n = n;
}

void DiskGraphManager::set_outgoing_num(unsigned long from, unsigned long num)
{
  Error.Notrace("Internal: set_outgoing_num not supported on DiskGraphManager.");
}

void DiskGraphManager::setup_outgoing_lists()
{
  unsigned long i,j,pos=0;
  edge dummy;
  dummy.to = 0;
  dummy.rule = 0;
#ifdef VARIABLE_WEIGHT
  dummy.weight = 0;
#endif
#ifdef VARIABLE_DURATION
  dummy.duration = 0;
#endif


  
  //write out degrees
  for(i = 0; i< num_states; ++i) {
    fwrite(&out_degree[i],sizeof(RULE_INDEX_TYPE),1,graph);
  }

  
  //write empty transitions (pad space)
  for(i = 0; i< num_states; ++i) {
    out_position[i] = pos;
    
    for(j = 0; j< out_degree[i]; ++j) fwrite(&dummy,sizeof(edge),1,graph);
    pos+=out_degree[i];
    out_degree[i]=0;
  }

  num_transitions = 0;
  seek_to_edge(0,0);
  last_from=0;
  last_n=0;
}

void DiskGraphManager::add_outgoing(unsigned long from, unsigned long to, RULE_INDEX_TYPE rule
#ifdef VARIABLE_WEIGHT
                                    , int weight
#endif
#ifdef VARIABLE_DURATION
                                    , int duration
#endif
                                   )
{
  edge dummy;
  dummy.to = to;
  dummy.rule = rule;
#ifdef VARIABLE_WEIGHT
  dummy.weight = weight;
#endif
#ifdef VARIABLE_DURATION
  dummy.duration = duration;
#endif

  set_edge(from,out_degree[from],&dummy);

  out_degree[from]++;
  num_transitions++;
}

edge *DiskGraphManager::reorder_list(unsigned long from, bool asc)
{
  edge *buffer = new edge[out_degree[from]];

  seek_to_edge(from,0);
  if (fread(buffer,sizeof(edge),out_degree[from],graph)==out_degree[from]) {
    qsort(buffer,out_degree[from],sizeof(edge),(asc?(GraphManager::weightasc):(GraphManager::weightdesc)));
    seek_to_edge(from,0);
    if (fwrite(buffer,sizeof(edge),out_degree[from],graph)!=out_degree[from]) {
      Error.Notrace("Error writing sorted edge list.");
    }
    edge_buffer = buffer[0];
    last_from = from;
    last_n = out_degree[from];
    return &edge_buffer;
  } else {
    Error.Notrace("Error reading edge list.");
  }
  delete[] buffer;
}

edge *DiskGraphManager::GetOutgoing(unsigned long from, RULE_INDEX_TYPE num)
{
  if (num < out_degree[from] && num >= 0) {
    return get_edge(from,num);
  } else {
    Error.Notrace("Accessing an unexisting edge.");
  }
}

void DiskGraphManager::print()
{
  fprintf(stderr,"\nDumping graph...\n");
  for(unsigned long i = 0; i<num_states; ++i) {
    cout << endl << "State " << i << ": ";
    for(RULE_INDEX_TYPE j = 0; j<out_degree[i]; ++j) {
      edge *e = GetOutgoing(i,j);
      cout << "R" << e->rule << "(W=" <<
#ifdef VARIABLE_WEIGHT
           e->weight
#else
           Rules->RuleWeight(e->rule)
#endif
           << ",D=" <<
#ifdef VARIABLE_DURATION
           e->duration
#else
           Rules->RuleDuration(e->rule)
#endif
           << ")->" << e->to
           << " ";
    }
  }
}
