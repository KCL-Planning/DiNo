/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#include <vector>

#define SSQ_PAGING_FILE_TOP "SSQ_PAGING_FILE_1"
#define SSQ_PAGING_FILE_BOTTOM "SSQ_PAGING_FILE_2"

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


/****************************************
  There are 3 groups of implementations:
  1) bit vector
  2) class StatePtr and state related stuff
  3) state queue and stack
  4) state set
  ****************************************/


void
state::print ()
{
  theworld.print ();
};

// WP
void
state::fire_processes(int t_steps){

	if (SRPG_ENABLED)
	for (int steps = 0; steps < t_steps; steps++){
		theworld.fire_processes();
	}

}
// WP
void
state::fire_processes_plus(int t_steps){

	if (SRPG_ENABLED)
	for (int steps = 0; steps < t_steps; steps++){
		theworld.fire_processes_plus();
	}

}
// WP
void
state::fire_processes_minus(int t_steps){

	if (SRPG_ENABLED)
	for (int steps = 0; steps < t_steps; steps++){
		theworld.fire_processes_minus();
	}

}

// WP
double
state::get_f_val()
{
	double f_val = theworld.get_f_val();
	return f_val;
}

// WP
void
state::set_f_val()
{
	theworld.set_f_val();
}

//WP TODO: add in appropriate places
double
state::get_h_val()
{
	double h_val = theworld.get_h_val();
	return h_val;
};

// WP
void
state::set_h_val()
{
	theworld.set_h_val();
};

// WP
void
state::set_h_val(int hp)
{
	theworld.set_h_val(hp);
};

//WP TODO: add in appropriate places
double
state::get_g_val()
{
	double g_n = theworld.get_g_val();
	return g_n;
};

// WP
void
state::set_g_val(double g_val)
{
	theworld.set_g_val(g_val);
};

// WP WP WP WP commented out for UAV tests
std::vector<mu_0_boolean*>
state::get_mu_bools()
{
	std::vector<mu_0_boolean*> aw2;
	if (SRPG_ENABLED){
	aw2 = theworld.get_mu_bools();
	}
	return (aw2);
}

// WP WP WP WP commented out for UAV tests
std::vector<mu_0_boolean*>
state::get_mu_bool_arrays()
{
	std::vector<mu_0_boolean*> aw2;
	if (SRPG_ENABLED){
	aw2 = theworld.get_mu_bool_arrays();
	}
	return (aw2);
}

// WP WP WP WP commented out for UAV tests
std::vector<mu__real*>
state::get_mu_nums()
{
	std::vector<mu__real*> aw2;

	if (SRPG_ENABLED){
		aw2 = theworld.get_mu_nums();
	}
	return (aw2);
}

// WP WP WP WP commented out for UAV tests
std::vector<mu__real*>
state::get_mu_num_arrays()
{
	std::vector<mu__real*> aw2;

	if (SRPG_ENABLED){
		aw2 = theworld.get_mu_num_arrays();
	}
	return (aw2);

}


/****************************************
  Bit vector - copied straight from Andreas.
  ****************************************/
dynBitVec::dynBitVec (unsigned long nBits):
  numBits (nBits)
{
  v = new unsigned char[NumBytes ()];	/* Allocate and clear vector. */
  MEMTRACKALLOC
  ErrAlloc (v);
  memset (v, 0, NumBytes ());
};

dynBitVec::~dynBitVec ()
{
  delete[OLD_GPP (NumBytes ())]v;	// should be delete[].
}

/****************************************
  class StatePtr and state related stuff.
  ****************************************/

inline void
StatePtr::sCheck ()
{
#ifdef HASHC
  if (args->trace_file.value)
    Error.Notrace ("Internal: Illegal Access to StatePtr.");
#endif
}

inline void
StatePtr::lCheck ()
{
#ifdef HASHC
  if (!args->trace_file.value)
    Error.Notrace ("Internal: Illegal Access to StatePtr.");
#endif
}

StatePtr::StatePtr (state * s)
{
  sCheck ();
  sp = s;
}
StatePtr::StatePtr (unsigned long l)
{
  lCheck ();
  lv = l;
}

void
StatePtr::set (state * s)
{
  sCheck ();
  sp = s;
}

void
StatePtr::set (unsigned long l)
{
  lCheck ();
  lv = l;
}

void
StatePtr::clear ()
{
#ifdef HASHC
  if (args->trace_file.value)
    lv = 0;
  else
#endif
    sp = NULL;
}

state *
StatePtr::sVal ()
{
  sCheck ();
  return sp;
}
unsigned long
StatePtr::lVal ()
{
  lCheck ();
  return lv;
}

StatePtr StatePtr::previous ()
{
  // return StatePtr to previous state
#ifdef HASHC
  if (args->trace_file.value)
    return TraceFile->read (lv)->previous;
  else
#endif
    return sp->previous.sp;
}

bool StatePtr::isStart ()
{
  // check if I point to a startstate
#ifdef HASHC
  if (args->trace_file.value) {
    if (TraceFile->read (lv)->previous == 0)
      return TRUE;
    return FALSE;
  } else
#endif
  {
    if (sp->previous.sp == NULL)
      return TRUE;
    return FALSE;
  }
}

bool StatePtr::compare (state * s)
{
  // compare the state I point to with s
#ifdef HASHC
  if (args->trace_file.value) {
    unsigned long *
    key = h3->hash (s, FALSE);
    unsigned long
    c1 = key[1] &
         ((~0UL) <<
          (args->num_bits.value > 32 ? 0 : 32 - args->num_bits.value));
    unsigned long
    c2 =
      key[2] & (args->num_bits.value >
                32 ? (~0UL) << (64 - args->num_bits.value) : 0UL);

    return (c1 == TraceFile->read (lv)->c1 &&
            c2 == TraceFile->read (lv)->c2);
  } else
#endif
    return (StateCmp (sp, s) == 0);
}


void
StateCopy (state * l, state * r)
// Uli: uses default assignment operator
{
  *l = *r;
}

int
StateCmp (state * l, state * r)
{
  int
  i = BLOCKS_IN_WORLD / 4;
  register int *
  d = (int *) l->bits, *s = (int *) r->bits;

  while (i--)
    if (*d > *s)
      return 1;
    else if (*d++ < *s++)
      return -1;
  return 0;
}

void
copy_state (state * &s)
{
  state *
  h;

  MEMTRACKALLOC
  if ((h = new state) == NULL)
    Error.Notrace("New failed. Swap space probably too small for state queue.");

  *h = *s;
  s = h;
}

bool
StateEquivalent (state * l, StatePtr r)
{
  return match (l, r);
}

/****************************************
  class state_queue for searching the state space.
  ****************************************/
state_queue::state_queue(unsigned long mas)
  :  max_active_states(mas)
{
  stateArray = new state_and_index[max_active_states];
  MEMTRACKALLOC

#ifndef SPLITFILE

  if ((paging_file_top = tmpfile()) == NULL) {
    Error.Notrace("Internal: Error creating top paging file.");
  }

  if ((paging_file_bottom = tmpfile()) == NULL) {
    Error.Notrace("Internal: Error creating bottom paging file.");
  }
#else
  paging_file_top = new splitFile(SPLITFILE_LEN, false);
  MEMTRACKALLOC
  paging_file_bottom = new splitFile(SPLITFILE_LEN, false);
  MEMTRACKALLOC

  if (!
      (paging_file_top->
       open(paging_file_top->make_unique_filename(SSQ_PAGING_FILE_TOP),
            "w+b"))) {
    Error.
    Notrace("Internal: Error creating top paging file for the queue.");
  }

  if (!
      (paging_file_bottom->
       open(paging_file_bottom->
            make_unique_filename(SSQ_PAGING_FILE_BOTTOM), "w+b"))) {
    Error.
    Notrace
    ("Internal: Error creating bottom paging file for the queue.");
  }
#endif

  num_elts_head = num_elts_tail = 0;

  head_begin = 0;
  tail_begin = max_active_states / 2;

  head_size = max_active_states / 2;
  tail_size = max_active_states - head_size;

  global_front = global_rear = front = rear = 0;
}


state_queue::~state_queue()
{
  delete[OLD_GPP(max_active_states)] stateArray;	// Should be delete[].

#ifndef SPLITFILE
  fclose(paging_file_top);	//rmtmp();
  fclose(paging_file_bottom);	//rmtmp();
#else

  paging_file_top->close();	//rmtmp();
  paging_file_bottom->close();	//rmtmp();

  delete paging_file_top;
  delete paging_file_bottom;
#endif
}

int
state_queue::BytesForOneState(void)
{

#ifdef VER_PSEUDO
  // Pseudo ver: ptr + malloced state + approx. malloc&new overhead.
  return sizeof(state *) + sizeof(state) + 8;
#else
#if 0
  /* This is True for our queue, but leads to a wrong NumStates. et */
  return sizeof(state);		/* Our queue contains states, not ptr to states */
#endif

  /* This is FALSE for our queue, but,
     with the adj the queue creation leads to a correct NumStates. et  */
  return sizeof(state_and_index);
  //return sizeof(state_and_index_and_counter);
#endif
}


void state_queue::Print(void)
{
  unsigned long i;

  for (i = 0; i < num_elts_head; i++) {
    // convert to print in unsigned long format?
    cout << "State " << i << " [" << head_begin + i << "]:\n";
    stateArray[head_begin + i].s.print();
  }

  for (i = 0; i < num_elts_tail; i++) {
    // convert to print in unsigned long format?
    cout << "State " << i << " [" << tail_begin + i << "]:\n";
    stateArray[tail_begin + i].s.print();
  }
}




/**
 *
 * WP WP WP WP WP WP WP WP WP WP WP WP WP WP WP
 *
 * CURRENT WORKING QUEUE!!!
 *
 *
 */

void state_queue::enqueue(state * &e, unsigned long index)
{

//  cout << (num_elts_head+num_elts_tail) << " out of: " << max_active_states << endl;

  if (num_elts_tail >= tail_size) {	//memory full: reclaim more space by swapping out the current queue
	  ReclaimFreeSpace();
  }

//     at this point, ReclaimFreeSpace has obtained new space in the queue and
//     set the offsets (front, rear, ...) accordingly; so we proceed with
//     the insertion without checking...

  state * tempt = new state();
  StateCopy(tempt, workingstate);

  double e_f = e->get_f_val();
  double e_h = e->get_h_val();

  int stateArrayIndex = head_begin+front+num_elts_head-1;
//  cout << "Getting from state array index " << stateArrayIndex << endl;

  double f_head_end = -1;

//  assert(stateArrayIndex >= 0);
//  assert(stateArrayIndex < max_active_states);

  if (stateArrayIndex >= 0){
	  StateCopy(workingstate, &(stateArray[stateArrayIndex].s));
	  f_head_end = workingstate->get_f_val();
  }

  double f_tail_front = -1;
  if (num_elts_tail > 0){
	  StateCopy(workingstate, &(stateArray[tail_begin + rear].s));
	  f_tail_front = workingstate->get_f_val();
  }

//  cout << "\nHEAD END F VALUE: " << f_head_end << endl;
//  cout << "STATE ARRAY INDEX: " << stateArrayIndex << endl;

  StateCopy(workingstate, &(stateArray[head_begin+front].s));
  double f_head_begin_front = workingstate->get_f_val();

//  cout << "\nHEAD BEGIN + FRONT F VALUE: " << f_head_begin_front << endl;
//  cout << "STATE ARRAY INDEX: " << stateArrayIndex << endl;
//  cout << "HEAD BEGIN + FRONT: " << (head_begin+front) << endl;

//  if (num_elts_head > 0 && head_begin+front > 0 && e_h <= f_head_begin_front){
//
//	  front--;
//	  StateCopy(&(stateArray[head_begin + front].s), e);
//	  stateArray[head_begin+front].i = index;
//	  num_elts_head++;
//
////	  cout << "HEAD FRONT!!!!!!!!!!" << endl;
//
//  }

  state * temp = new state();

//  if (helpful_actions.count(Rules->LastRuleNumber()) > 0){
//	cout << "\n\n\n\nHELPFUL ACTION HIT: " << Rules->LastRuleName() << endl;
//  }

   if ((mu_TIME > args->SRPG_horizon.value) || (num_elts_tail == 0 && num_elts_head == 0) || (e_f >= f_head_end || num_elts_head == 0)) { // WP WP WP WP WP EDIT: added condition for exceeding the SRPG temporal horizon

		  StateCopy(&(stateArray[tail_begin + rear].s), e);
		  stateArray[tail_begin + rear].i = index;
		#ifdef HASHC
			delete e;
		#endif
		  e = &(stateArray[tail_begin + rear].s);

		  rear++;
		  num_elts_tail++;

//		  cout << "TAIL END" << endl;


		if (num_elts_tail >=2){

		  for (int ix = 1; ix < num_elts_tail; ix++){

			  StateCopy(workingstate, &(stateArray[tail_begin+rear-ix].s));
//			  cout << "ARRAY INDEX TAIL+REAR-(IX)" << (tail_begin+rear - (ix)) << endl;

			  int state_ix1 = stateArray[tail_begin+rear - ix].i;

			  double f1 = workingstate->get_f_val();
			  double h1 = workingstate->get_h_val();

			  StateCopy(workingstate, &(stateArray[tail_begin+rear-(ix+1)].s));

			  int state_ix2 = stateArray[tail_begin+rear - (ix+1)].i;
//			  cout << "ARRAY INDEX TAIL+REAR-(IX+1)" << (tail_begin+rear - (ix+1)) << endl;

			  double f2 = workingstate->get_f_val();
			  double h2 = workingstate->get_h_val();

//			  cout << "TAIL    IX HEURISTIC" << h1 << endl;
//
//			  cout << "TAIL    IX+1 HEURISTIC" << h2 <<endl;

			  if ((f1 < f2) || ((f1 == f2) && (h1 < h2))){

				  StateCopy(workingstate, &(stateArray[tail_begin+rear-ix].s));

//				  state * temp = new state();

				  StateCopy(temp, &(stateArray[tail_begin+rear-(ix+1)].s));

				  StateCopy(&(stateArray[tail_begin+rear-ix].s), temp);

				  stateArray[tail_begin + rear-ix].i = state_ix2;

				  StateCopy(&(stateArray[tail_begin+rear-(ix+1)].s), workingstate);

				  stateArray[tail_begin + rear-(ix+1)].i = state_ix1;

//				  cout << "TAIL  SWAP!!!!!!" << endl;

//				  delete temp;
			  }
			  else {
//				  cout << "TAIL  BREAK!!!!!" << endl;
				  break;
			  }
		  }
		}
	}

  else if ((head_begin+front > 0) && (num_elts_head > 0) && (f_head_end >= 0) && (e_f <= f_head_end || e_h == 0)){ // WP EDIT

	  StateCopy(&(stateArray[head_begin + front-1].s), e);
	  stateArray[head_begin + front-1].i = index;
	#ifdef HASHC
		delete e;
	#endif
	  e = &(stateArray[head_begin + front-1].s);


	  num_elts_head++;
	  front--;

//	  cout << "HEAD NON FRONT" << endl;

	  		if (num_elts_head >=2 && e_f > f_head_begin_front){

	  			  for (int ix = 0; ix < num_elts_head; ix++){

	  				  StateCopy(workingstate, &(stateArray[head_begin+front+ix].s));

	  				  int state_ix1 = stateArray[head_begin+front+ix].i;

	  				  double f1 = workingstate->get_f_val();
	  				  double h1 = workingstate->get_h_val();

	  				  StateCopy(workingstate, &(stateArray[head_begin+front+(ix+1)].s));

	  				  int state_ix2 = stateArray[head_begin+front+(ix+1)].i;

	  				  double f2 = workingstate->get_f_val();
	  				  double h2 = workingstate->get_h_val();

//	  				  cout << "HEAD  F   IX HEURISTIC" << f1 << endl;

//	  				  cout << "HEAD  F   IX+1 HEURISTIC" << f2 <<endl;


	  				  if ((f1 > f2) || ((f1 == f2) && (h1 > h2))){

	  					  StateCopy(workingstate, &(stateArray[head_begin+front+ix].s));

// 	  					  state * temp = new state();

	  					  StateCopy(temp, &(stateArray[head_begin+front+(ix+1)].s));

	  					  StateCopy(&(stateArray[head_begin+front+ix].s), temp);

	  					  stateArray[head_begin+front+ix].i = state_ix2;

	  					  StateCopy(&(stateArray[head_begin+front+(ix+1)].s), workingstate);

	  					  stateArray[head_begin+front+(ix+1)].i = state_ix1;

//	  					  cout << "HEAD  SWAP!!!!!!" << endl;

//	  					  delete temp;
	  				  }
	  				  else {
//	  					  cout << "HEAD  BREAK!!!!!" << endl;
	  					  break;
	  				  }
	  			}
	  		}
	  //		StateCopy(workingstate, tempt);
	  //		delete tempt;


  }

	else {


		  StateCopy(&(stateArray[head_begin+front+num_elts_head].s), e);
		  stateArray[head_begin + front + num_elts_head].i = index;
		#ifdef HASHC
			delete e;
		#endif
		  e = &(stateArray[head_begin + front + num_elts_head].s);

		  num_elts_head++;

//		  cout << num_elts_head << " OF " << head_size << endl;

		if (num_elts_head >=2){

			  for (int ix = 1; ix < num_elts_head; ix++){

				  StateCopy(workingstate, &(stateArray[head_begin+front+num_elts_head-ix].s));

				  int state_ix1 = stateArray[head_begin+front+num_elts_head-ix].i;

				  double f1 = workingstate->get_f_val();
				  double h1 = workingstate->get_h_val();

				  StateCopy(workingstate, &(stateArray[head_begin+front+num_elts_head-(ix+1)].s));

				  int state_ix2 = stateArray[head_begin+front+num_elts_head-(ix+1)].i;

				  double f2 = workingstate->get_f_val();
				  double h2 = workingstate->get_h_val();

//				  cout << "HEAD     IX HEURISTIC" << h1 << endl;

//				  cout << "HEAD     IX+1 HEURISTIC" << h2 <<endl;

//				  cout << max_active_states << endl;
//				  cout << (head_begin+front+num_elts_head-(ix)) << endl;
//				  cout << (head_begin+front+num_elts_head-(ix+1)) << endl;

				  if ((f1 < f2) || ((f1 == f2) && (h1 < h2))){ // WP NEEDS EDIT?

					  StateCopy(workingstate, &(stateArray[head_begin+front+num_elts_head-ix].s));

//					  cout << "F1: " << f1 << " - F2: " << f2 << endl;

//					  state * temp = new state();

					  StateCopy(temp, &(stateArray[head_begin+front+num_elts_head-(ix+1)].s));

					  StateCopy(&(stateArray[head_begin+front+num_elts_head-ix].s), temp);

					  stateArray[head_begin+front+num_elts_head-ix].i = state_ix2;

					  StateCopy(&(stateArray[head_begin+front+num_elts_head-(ix+1)].s), workingstate);

					  stateArray[head_begin+front+num_elts_head-(ix+1)].i = state_ix1;

//					  cout << "HEAD  SWAP!!!!!!" << endl;

//					  delete temp;
				  }
				  else {
//					  cout << "HEAD  BREAK!!!!!" << endl;
					  break;
				  }
			}
		}
//		StateCopy(workingstate, tempt);
//		delete tempt;
	}

	StateCopy(workingstate, tempt);
	delete tempt;
	delete temp;


// WP WP WP WP WP WP
//	COMMENTED OUT PRINT FOR TESTING PURPOSES
//  Print();
}










//void state_queue::enqueue(state * &e, unsigned long index)
//{
//
//  if (num_elts_tail >= tail_size) {	//memory full: reclaim more space by swapping out the current queue
//    ReclaimFreeSpace();
//  }
//
//  /*
//     at this point, ReclaimFreeSpace has obtained new space in the queue and
//     set the offsets (front, rear, ...) accordingly; so we proceed with
//     the insertion without checking...
//   */
//
//
//  StateCopy(&(stateArray[tail_begin + rear].s), e);
//  stateArray[tail_begin + rear].i = index;
//#ifdef HASHC
//  delete e;
//#endif
//  e = &(stateArray[tail_begin + rear].s);
//
//  rear++;
//  num_elts_tail++;
//}

state_and_index *state_queue::dequeue(void)
{
  state_and_index *retval;
  //state_and_index_and_counter *retval;

  if (num_elts_head <= 0) {
    QueueEmptyFault();
  }

  retval = &stateArray[head_begin + front];
  front++;
  num_elts_head--;

  return retval;
}

state_and_index *state_queue::top(void)
//state_and_index_and_counter *state_queue::top(void)
{
  if (num_elts_head <= 0) {
    QueueEmptyFault();
  }

  return &stateArray[head_begin + front];
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++

void state_queue::ReclaimFreeSpace()
{

#ifndef SPLITFILE
  global_rear +=
    fwrite(&stateArray[tail_begin], sizeof(state_and_index), tail_size, paging_file_bottom);

#else
  global_rear +=
    paging_file_bottom->write(&stateArray[tail_begin], sizeof(state_and_index),tail_size);
#endif

  num_elts_tail = 0;
  rear = 0;

  /* too expensive ... may we can do without. et  */
#if 0
  //for ( long i = 0; i < tail_size; i++)   // Uli: avoid bzero
  //stateArray[tail_begin + i] = NULL;
#endif

}

void state_queue::QueueEmptyFault()
{
#ifndef SPLITFILE
  size_t read =
    fread(&stateArray[head_begin], sizeof(state_and_index), head_size,  paging_file_top);
#else
  size_t read =
    paging_file_top->read(&stateArray[head_begin], sizeof(state_and_index),head_size);
#endif

  if (read > 0 && global_front >= read) {	//ok, some states are swapped in

    num_elts_head = read;
    global_front -= read;
  } else if (read > 0 && global_front > 0 && global_front < read) {
    // same as above, but may have read more than allowed
    num_elts_head = global_front;
    global_front = 0;
  } else if (global_rear > 0) {	//paging_file_top is empty, but paging_file_bottom is not
#ifndef SPLITFILE
    fclose(paging_file_top);

    paging_file_top = paging_file_bottom;
    fseek(paging_file_top, 0, SEEK_SET);	//move to the beginning of the queue
    global_front = global_rear;

    if ((paging_file_bottom = tmpfile()) == NULL) {
      Error.Notrace("Internal: Error creating bottom paging file.");
    }
    global_rear = 0;		//bottom file is empty

    size_t read =
      fread(&stateArray[head_begin], sizeof(state_and_index), head_size, paging_file_top);

    num_elts_head = read;
    global_front -= read;
#else
    splitFile *fswap;
    fswap = paging_file_top;
    paging_file_top = paging_file_bottom;
    paging_file_bottom = fswap;
    paging_file_top->seek(0, SEEK_SET);	//move to the beginning of the queue
    paging_file_bottom->seek(0, SEEK_SET);	//reset bottom queue

    global_front = global_rear;
    global_rear = 0;		//bottom file is empty

    //now bottom entries are top entries and bottom file is empty. Reload a block!
    size_t read =
      paging_file_top->read(&stateArray[head_begin], sizeof(state_and_index),  head_size);

    num_elts_head = read;
    global_front -= read;
#endif
  } else if (num_elts_tail > 0) {	//paging_file_top AND paging_file_bottom are empty

    /* the disk queue is ended. this means that the only states we have
       to explore are the ones in the current tail window */
    int swap = tail_begin;
    tail_begin = head_begin;
    head_begin = swap;

    swap = tail_size;
    tail_size = head_size;
    head_size = swap;

    num_elts_head = num_elts_tail;
    num_elts_tail = 0;
    rear = 0;
  } else {			//no more states in both swap files, and the memory is empty: why do we call again?
    Error.Notrace("Internal: Attempt to read an empty state queue.");
  }

  front = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++

void state_stack::enqueue(state * e)
{
  /*
    if (num_elts < max_active_states) {
      front = front == 0 ? max_active_states - 1 : front - 1;
      stateArray[front] = e;
      nextrule_to_try[front] = 0;
      num_elts++;
    } else {
      Error.Notrace("Internal: Too many active states.");
    }
  */
}


/****************************************
  class disk_queue.
  ****************************************/
template<typename E>
disk_queue<E>::disk_queue(unsigned long len)
  :  max_length(len)
{
  cache = new E[max_length];
  MEMTRACKALLOC

#ifndef SPLITFILE
  if ((paging_file_top = tmpfile()) == NULL) {
    Error.Notrace("Internal: Error creating top paging file.");
  }

  if ((paging_file_bottom = tmpfile()) == NULL) {
    Error.Notrace("Internal: Error creating bottom paging file.");
  }
#else
  paging_file_top = new splitFile(SPLITFILE_LEN, false);
  MEMTRACKALLOC
  paging_file_bottom = new splitFile(SPLITFILE_LEN, false);
  MEMTRACKALLOC

  if (!
      (paging_file_top->
       open(paging_file_top->make_unique_filename(SSQ_PAGING_FILE_TOP),
            "w+b"))) {
    Error.
    Notrace("Internal: Error creating top paging file for the queue.");
  }

  if (!
      (paging_file_bottom->
       open(paging_file_bottom->
            make_unique_filename(SSQ_PAGING_FILE_BOTTOM), "w+b"))) {
    Error.
    Notrace
    ("Internal: Error creating bottom paging file for the queue.");
  }
#endif

  num_elts_head = num_elts_tail = 0;

  head_begin = 0;
  tail_begin = max_length / 2;

  head_size = max_length / 2;
  tail_size = max_length - head_size;

  global_front = global_rear = front = rear = 0;
}


template<typename E>
disk_queue<E>::~disk_queue()
{
  delete[OLD_GPP(max_length)] cache;	// Should be delete[].

#ifndef SPLITFILE
  fclose(paging_file_top);	//rmtmp();
  fclose(paging_file_bottom);	//rmtmp();
#else

  paging_file_top->close();	//rmtmp();
  paging_file_bottom->close();	//rmtmp();

  delete paging_file_top;
  delete paging_file_bottom;
#endif
}

template<typename E>
void disk_queue<E>::enqueue(E &e)
{

  if (num_elts_tail >= tail_size) {	//memory full: reclaim more space by swapping out the current queue
    ReclaimFreeSpace();
  }

  /*
     at this point, ReclaimFreeSpace has obtained new space in the queue and
     set the offsets (front, rear, ...) accordingly; so we proceed with
     the insertion without checking...
   */

  memcpy(&cache[tail_begin + rear], &e, sizeof(E));
  //e = &cache[tail_begin + rear];

  rear++;
  num_elts_tail++;
}

template<typename E>
E disk_queue<E>::dequeue(void)
{
  E retval;

  if (num_elts_head <= 0) {
    QueueEmptyFault();
  }

  retval = cache[head_begin + front];
  front++;
  num_elts_head--;

  return retval;
}

template<typename E>
E disk_queue<E>::top(void)
{
  if (num_elts_head <= 0) {
    QueueEmptyFault();
  }

  return cache[head_begin + front];
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename E>
void disk_queue<E>::ReclaimFreeSpace()
{

#ifndef SPLITFILE
  global_rear +=
    fwrite(&cache[tail_begin], sizeof(E), tail_size,
           paging_file_bottom);

#else
  global_rear +=
    paging_file_bottom->write(&cache[tail_begin], sizeof(E),
                              tail_size);
#endif

  num_elts_tail = 0;
  rear = 0;

  /* too expensive ... may we can do without. et  */
#if 0
  //for ( long i = 0; i < tail_size; i++)   // Uli: avoid bzero
  //stateArray[tail_begin + i] = NULL;
#endif

}

template<typename E>
void disk_queue<E>::QueueEmptyFault()
{
#ifndef SPLITFILE
  size_t read = fread(&cache[head_begin], sizeof(E), head_size,
                      paging_file_top);
#else
  size_t read =
    paging_file_top->read(&cache[head_begin], sizeof(E),
                          head_size);
#endif

  if (read > 0 && global_front >= read) {	//ok, some states are swapped in

    num_elts_head = read;
    global_front -= read;
  } else if (read > 0 && global_front > 0 && global_front < read) {
    // same as above, but may have read more than allowed
    num_elts_head = global_front;
    global_front = 0;
  } else if (global_rear > 0) {	//paging_file_top is empty, but paging_file_bottom is not
#ifndef SPLITFILE
    fclose(paging_file_top);

    paging_file_top = paging_file_bottom;
    fseek(paging_file_top, 0, SEEK_SET);	//move to the beginning of the queue
    global_front = global_rear;

    if ((paging_file_bottom = tmpfile()) == NULL) {
      Error.Notrace("Internal: Error creating bottom paging file.");
    }
    global_rear = 0;		//bottom file is empty

    size_t read = fread(&cache[head_begin], sizeof(E), head_size,
                        paging_file_top);

    num_elts_head = read;
    global_front -= read;
#else
    splitFile *fswap;
    fswap = paging_file_top;
    paging_file_top = paging_file_bottom;
    paging_file_bottom = fswap;
    paging_file_top->seek(0, SEEK_SET);	//move to the beginning of the queue
    paging_file_bottom->seek(0, SEEK_SET);	//reset bottom queue

    global_front = global_rear;
    global_rear = 0;		//bottom file is empty

    //now bottom entries are top entries and bottom file is empty. Reload a block!
    size_t read =
      paging_file_top->read(&cache[head_begin], sizeof(E),
                            head_size);

    num_elts_head = read;
    global_front -= read;
#endif
  } else if (num_elts_tail > 0) {	//paging_file_top AND paging_file_bottom are empty

    /* the disk queue is ended. this means that the only states we have
       to explore are the ones in the current tail window */
    int swap = tail_begin;
    tail_begin = head_begin;
    head_begin = swap;

    swap = tail_size;
    tail_size = head_size;
    head_size = swap;

    num_elts_head = num_elts_tail;
    num_elts_tail = 0;
    rear = 0;
  } else {			//no more states in both swap files, and the memory is empty: why do we call again?
    Error.Notrace("Internal: Attempt to read from an empty queue.");
  }

  front = 0;
}


/****************************************   // changes by Uli
  The Stateset implementation for recording all the states found.
  ****************************************/

int
state_set::bits_per_state ()
{
  int retVal;
#ifndef HASHC
  retVal = 8 * sizeof (state);
#else
  retVal = args->num_bits.value;
#endif

  retVal += 8 * sizeof(unsigned long); //gdp disk index
  //retVal += 8 * sizeof(unsigned long); //gdp counter

  return retVal;
}

state_set::state_set (unsigned long table_size):
  table_size (table_size),
  num_elts (0),
  num_elts_reduced (0),
  reachables(NULL)
  //num_tables (1)
{
  create_table(table_size,0);
  if(needs_expanded_states()) reachables = Storage->getReachablesFile(true);

}

state_set::~state_set ()
{
  /*
  	for(int i=0; i<num_tables; ++i) {
  		if (table[i]) {
  			delete[]table[i];
  			delete[]disk_index[i];
  			delete Full[i];
  		}
  	}
  */
  delete[] table;
  delete[] disk_index;
  delete Full;
}

void state_set::create_table (unsigned long table_size, int level)
{


  unsigned long i;

  //if (level>=MAX_TABLE_LEVELS) Error.Notrace("Hash tables full");
  //if (table[level]!=NULL) Error.Notrace("Internal: recreating an hash table in use");

#ifndef HASHC
  table = new state[table_size];
  MEMTRACKALLOC
#else
  assert (sizeof (Unsigned32) == 4);	// the implementation is pretty dependent on the 32 bits
  unsigned long size = (unsigned long) ((double) table_size * args->num_bits.value / 32) + 3;
  // higher precision necessary to avoid overflow
  // two extra elements needed in table
  table = new Unsigned32[size];
  MEMTRACKALLOC
  PAUSE

  for (i = 0; i < size; i++)  table[i] = 0UL;
#endif
  disk_index = new unsigned long[table_size];
  MEMTRACKALLOC
  PAUSE

  for (i = 0; i < table_size; i++) disk_index[i]=0;

  Full = new dynBitVec (table_size);
  MEMTRACKALLOC
  PAUSE

}

bool state_set::simple_was_present (state * &in, bool valid, bool permanent, unsigned long *index)
/* changes in to point to the first state found with that pattern. */
/* returns true iff the state was present in the hash table;
 * Otherwise, returns false and inserts the state. */
/* Algorithms directly from Andreas\' code. He cites CLR 235, 236. */
// Uli: pitfall: shift operators yield undefined values if the right
//               operand is equal to the length in bits of the left
//               operand (see ARM, pg.74)
// Uli: table_size must be prime
{
#ifndef HASHC
  unsigned long key = in->hashkey();
  unsigned long h1 = key % table_size;
  unsigned long h2 = 1 + key % (table_size - 1);
  unsigned long h = h1;
#else
  unsigned long *key = h3->hash(in, valid);
  unsigned long h1 = key[0] % table_size;
  unsigned long h2;
  register unsigned long h = h1;
  register unsigned long num_bits = args->num_bits.value;
  register unsigned long mask1 =
    (~0UL) << (num_bits > 32 ? 0 : 32 - num_bits);
  register unsigned long mask2 =
    num_bits > 32 ? (~0UL) << (64 - num_bits) : 0UL;
  register unsigned long addr, offset;
  register unsigned long c1 = key[1] & mask1;
  register unsigned long c2 = key[2] & mask2;
  register unsigned long t1, t2;
#endif

#ifdef VER_PSEUDO
  if (is_empty(h)) {
    Full->set(h);
    num_elts++;
    return FALSE;
  }
  return TRUE;

#else
  unsigned long probe = 0;

#ifndef HASHC
// no hash compaction, uses double hashing

  bool empty, equal = FALSE;

  while (!(empty = is_empty(h)) && !(equal = (*in == table[h])) && (probe < table_size)) {
    h = (h1 + probe * h2) % table_size;	// double hashing
    num_collisions++;
    probe++;
  }
  if (empty) {			/* Go ahead and insert the element. */
    table[h] = *in;

    if(needs_expanded_states()) fwrite(in,sizeof(state),1,reachables);
    //in->WriteTo(reachables);
    disk_index[h] = num_elts;
    if (index!=NULL) *index = disk_index[h];

    in = &table[h];
    Full->set(h);

    num_elts++;
    return FALSE;
  } else if (equal) {
    in = &table[h];
    if (index!=NULL) *index = disk_index[h];
    return TRUE;
  } else {
    Error.Notrace("Closed hash table full.");
    return FALSE;		/* it doesn\'t matter, but it shuts up g++. */
  }

#else
// hash compaction, uses ordered hashing
// the state-insertion is done in two steps: search and insertion

  h2 = 1 + c1 % (table_size - 1);	// calculation uses compressed value

  // search - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  do {
    // calculate address and offset in table
    // 32 bit arithmetic not sufficient and may cause overflow
    // addr = (h*num_bits) / 32
    // offset = (h*num_bits) % 32
    // Thus, h*num_bits is first done on the least 16 bits, and then on the
    // remaining 16 bits; since we have to divide by 32, we ignore the least 5
    // bits of the result
    offset = (h & 0xffffUL) * num_bits;
    addr = (((h >> 16) * num_bits) << 11) + (offset >> 5);
    offset &= 0x1fUL;

    if (is_empty(h))
      break;			// search unsuccessful

    // read compressed value from table
    t1 = (table[addr] << offset |  (offset == 0 ? 0 : table[addr + 1] >> (32 - offset))) & mask1;
    t2 = (table[addr + 1] << offset |  (offset == 0 ? 0 : table[addr + 2] >> (32 - offset))) & mask2;

    if (t1 == c1 ? t2 < c2 : t1 < c1) break;			// search unsuccessful

    if (t1 == c1 && t2 == c2) {
      //fprintf(stderr,"\n stato trovato nello slot %ld con indice %ld -- ",h,disk_index[h]);
      //cout << "O" << h << "-" << disk_index[h] << endl;
      if (index!=NULL) *index = disk_index[h];
      return TRUE;		// search successful
    }

    h = (h + h2) % table_size;
    probe++;
    if (probe == table_size) Error.Notrace("Closed hash table full.");

  } while (TRUE);

  // write trace info
  if (args->trace_file.value) TraceFile->write(c1, c2, in->previous.lVal());

  // insertion (WITH DESC SORTING) - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  if (num_elts == table_size) Error.Notrace("Closed hash table full.");

  //write state, get its index and prepare to return it
  unsigned long c1c2_disk_index = num_elts;
  if(needs_expanded_states()) fwrite(in,sizeof(state),1,reachables);
  if (index!=NULL) *index = c1c2_disk_index;

  while (!is_empty(h)) {	// until empty slot found
    //the element to be inserted (c1,c2) is greater than the current one (t1,t2)
    //thus, insert the new element here and shift all the others one position forward
    if (t1 == c1 ? t2 < c2 : t1 < c1) {
      //delete c1,c2 and write t1,t2 in the current slot (h)
      table[addr] ^= (c1 ^ t1) >> offset;
      table[addr + 1] ^= (offset == 0 ? 0 : (c1 ^ t1) << (32 - offset))
                         | (c2 ^ t2) >> offset;
      table[addr + 2] ^= (offset == 0 ? 0 : (c2 ^ t2) << (32 - offset));
      //continue with the insertion of (t1,t2) (value to be shifted forward)
      c1 = t1;
      c2 = t2;

      //gdp: shift disk indexes, too
      unsigned long temp_disk_index = disk_index[h];
      disk_index[h] = c1c2_disk_index;
      c1c2_disk_index = temp_disk_index;
    }

    //get next slot in the collision list
    h = (h + 1 + c1 % (table_size - 1)) % table_size;
    offset = (h & 0xffffUL) * num_bits;
    addr = (((h >> 16) * num_bits) << 11) + (offset >> 5);
    offset &= 0x1fUL;

    //read the slot (h) value
    t1 = (table[addr] << offset | (offset == 0 ? 0 : table[addr + 1] >> (32 - offset)))	& mask1;
    t2 = (table[addr + 1] << offset | (offset == 0 ? 0 : table[addr + 2] >> (32 - offset))) & mask2;
  }

  //here, we have an empty slot, thus simply write the value (c1,c2)
  table[addr] |= c1 >> offset;	// insertion
  table[addr + 1] |= (offset == 0 ? 0 : c1 << (32 - offset)) | c2 >> offset;
  table[addr + 2] |= (offset == 0 ? 0 : c2 << (32 - offset));

  copy_state(in);		// make copy of state
  disk_index[h] = c1c2_disk_index;
  //fprintf(stderr,"added state #%ld to disk",disk_index[h]);

  Full->set(h);

  //fprintf(stderr,"\n stato aggiunto nello slot %ld con indice %ld",h,disk_index[h]);
  //cout << "F" << h << "-" << (*index) << endl;

  num_elts++;
  if (permanent) num_elts_reduced++;

  return FALSE;

#endif

#endif
};

bool
state_set::was_present (state * &in, bool valid, bool permanent, unsigned long *index)
{

  if (args->symmetry_reduction.value)
    in->Normalize ();
  if (args->multiset_reduction.value && !args->symmetry_reduction.value)
    in->MultisetSort ();

  return simple_was_present (in, valid, permanent, index);
}


void state_set::print_capacity (void)
{
  cout <<
       "\t* The memory allocated for the hash table and state queue is\n\t  ";
  if (args->mem.value > 1000000)
    cout << (args->mem.value / 1000000) << " Mbytes.\n";
  else
    cout << (args->mem.value / 1000) << " kbytes.\n";

#ifndef HASHC
  cout << "\t  With two words of overhead per state, the maximum size of\n"
       << "\t  the state space is "
       << table_size << " states.\n"
       << "\t   * Use option \"-k\" or \"-m\" to increase this, if necessary.\n";
#else
  cout << "\t  With states hash-compressed to "
       << args->num_bits.value << " bits, the maximum size of\n"
       << "\t  the state space is "
       << table_size << " states.\n"
       << "\t   * Use option \"-k\" or \"-m\" to increase this, if necessary.\n";
#endif
}
