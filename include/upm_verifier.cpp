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
  The Main() function:
  int main(int argc, char **argv)
  Note that Global Variables are in control.h
  ****************************************/

//UPMURPHI_BEGIN
int main(int argc, char **argv)
{
  args = new argclass(argc, argv);
  MEMTRACKALLOC
  Stats = new StatsManager();
  Storage = new StorageManager();
  MEMTRACKALLOC
  Algorithm = new AlgorithmManager();
  MEMTRACKALLOC


  if (args->main_alg.mode != argmain_alg::Nothing) Algorithm->Plan();

  cout.flush();
#ifdef HASHC
  if (args->trace_file.value)
    delete TraceFile;
#endif
  delete Algorithm; //gdp: fix: begin destruction chain
  delete Stats;
  delete Storage;

  /*---------*/
  exit(0);
}
//UPMURPHI_END