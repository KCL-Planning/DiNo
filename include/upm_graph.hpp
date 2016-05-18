/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

typedef struct Sedge {
  unsigned long to;
  RULE_INDEX_TYPE rule;
#ifdef VARIABLE_WEIGHT
  int weight;
#endif
#ifdef VARIABLE_DURATION
  int duration;
#endif
} edge;


class GraphManager
{
 protected:
  RULE_INDEX_TYPE* out_degree; //we use RULE_INDEX_TYPE since a single state can have as many outgoing transitions as the total numer of rules!
  unsigned long num_states,num_transitions, max_transitions;

 public:
  GraphManager(unsigned long size);
  virtual ~GraphManager()=0;
  virtual void set_outgoing_num(unsigned long from, unsigned long num)=0;
  virtual void inc_outgoing_num(unsigned long from);
  virtual void setup_outgoing_lists()=0;
  virtual void add_outgoing(unsigned long from, unsigned long to, RULE_INDEX_TYPE rule
#ifdef VARIABLE_WEIGHT
                            , int weight
#endif
#ifdef VARIABLE_DURATION
                            , int duration
#endif
                           )=0;
  virtual edge *reorder_list(unsigned long from, bool asc=true)=0;
  virtual const char* ModeLabel()=0;

  static int weightasc(const void* t1, const void* t2);
  static int weightdesc(const void* t1, const void* t2);

  virtual edge* GetOutgoing(unsigned long from, RULE_INDEX_TYPE num)=0;

  RULE_INDEX_TYPE OutDegree(unsigned long from)
  {
    return out_degree[from];
  };
  unsigned long OutDegreeTot();
  RULE_INDEX_TYPE OutDegreeMin();
  RULE_INDEX_TYPE OutDegreeMax();
  double OutDegreeAvg();
  unsigned long NumStates();
  unsigned long MaxElts()
  {
    return max_transitions;
  };
  virtual void print()=0;
};


////////////////////////////////////////////////

class MemGraphManager : public GraphManager
{
  edge** outgoing;

 public:
  MemGraphManager(unsigned long size);
  virtual ~MemGraphManager();
  static unsigned long estimate_max_transitions();
  virtual void set_outgoing_num(unsigned long from, unsigned long num);
  virtual void setup_outgoing_lists();
  virtual void add_outgoing(unsigned long from, unsigned long to, RULE_INDEX_TYPE rule
#ifdef VARIABLE_WEIGHT
                            , int weight
#endif
#ifdef VARIABLE_DURATION
                            , int duration
#endif
                           );
  virtual edge *reorder_list(unsigned long from, bool asc=true);
  virtual const char* ModeLabel()
  {
    return "Memory Image";
  };
  virtual edge* GetOutgoing(unsigned long from, RULE_INDEX_TYPE num);
  virtual void print();
};

//////////////////////////////////////////////////////////

class DiskGraphManager : public GraphManager
{
  FILE *graph;
  unsigned long *out_position;
  unsigned long last_from;
  RULE_INDEX_TYPE last_n;
  edge edge_buffer;

  edge *get_edge(unsigned long from, RULE_INDEX_TYPE n);
  void set_edge(unsigned long from, RULE_INDEX_TYPE n, edge *e);
  void seek_to_edge(unsigned long from, RULE_INDEX_TYPE n);

 public:
  DiskGraphManager(unsigned long size);
  ~DiskGraphManager();
  static unsigned long estimate_max_transitions();
  void set_outgoing_num(unsigned long from, unsigned long num);
  void setup_outgoing_lists();
  void add_outgoing(unsigned long from, unsigned long to, RULE_INDEX_TYPE rule
#ifdef VARIABLE_WEIGHT
                    , int weight
#endif
#ifdef VARIABLE_DURATION
                    , int duration
#endif
                   );
  edge *reorder_list(unsigned long from, bool asc=true);
  virtual const char* ModeLabel()
  {
    return "Indexed Disk Image";
  };
  edge *GetOutgoing(unsigned long from, RULE_INDEX_TYPE num);
  void print();
};
