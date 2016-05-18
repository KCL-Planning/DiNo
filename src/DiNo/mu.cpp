/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#include "mu.hpp"
#include <unistd.h>
#include <string.h>
#include <new>
#include <cstdlib>
#include <sys/stat.h>

using namespace std;

/********************
  variable declarations
  ********************/
program *theprog = NULL;
symboltable *symtab = NULL;
FILE *codefile;
char base_installation_path[1024]="";
char base_bin_path[1024]="";
char base_include_path[1024]="";



/********************
  class argclass
  ********************/
bool Sw(char *a, int ac, char **av)
{
  for (int i = 1; i < ac; i++)
    if (strcmp(av[i], a) == 0)
      return TRUE;
  return FALSE;
}

argclass::argclass(int ac, char **av)
  :
  argc(ac), argv(av), print_license(FALSE), help(FALSE), checking(TRUE),
  no_compression(FALSE), hash_compression(TRUE), UPMurphi_planner (FALSE), UPMurphi_disk(FALSE),
  variable_weight(FALSE),variable_duration(FALSE),m_filename(NULL),pddl_domain_filename(NULL),pddl_problem_filename(NULL),
  keep_source(TRUE),compile_source(TRUE),compile_pddl(TRUE),force_recompile(FALSE),dynamic_debug(FALSE),pddl_parser_prompt(FALSE),
  pddl_parser_set_1(0), pddl_parser_set_2(0), pddl_parser_set_3(0),warnings(FALSE)
{
  //bool initialized_filename = FALSE;
  int i;

  if (ac == 1) {		// if there is no argument, print help
    help = TRUE;
    PrintInfo();
    exit(1);
  }
  for (i = 1; i < ac; i++) {
    if (strcmp(av[i], "-l") == 0) {
      print_license = TRUE;
      continue;
    }
    if ((strcmp(av[i], "--nobitc")  == 0) || (strcmp(av[i], "-nb") == 0)) {
      no_compression = TRUE;
      continue;
    }
    if ((strcmp(av[i], "--nohashc") == 0) || (strcmp(av[i], "-nc") == 0)) {
      hash_compression = FALSE;
      continue;
    }

//UPMURPHI_BEGIN
    if (strcmp(av[i], "--planner") == 0) {
      UPMurphi_planner = TRUE;
      continue;
    }
    if (strcmp(av[i], "--disk") == 0) {
      UPMurphi_disk = TRUE;
      continue;
    }

    if (strcmp(av[i], "--clean") == 0) {
      keep_source = FALSE;
      continue;
    }

    if (strcmp(av[i], "--noexec") == 0) {
      compile_source = FALSE;
      continue;
    }

    if (strcmp(av[i], "--nopddl") == 0) {
      compile_pddl = FALSE;
      continue;
    }

    if (strcmp(av[i], "--force") == 0) {
      force_recompile = TRUE;
      continue;
    }

    if (strcmp(av[i], "--debug") == 0) {
      dynamic_debug = TRUE;
      continue;
    }

    if (strcmp(av[i], "--varweight") == 0) {
      variable_weight = TRUE;
      continue;
    }

    if (strcmp(av[i], "--varduration") == 0) {
      variable_duration = TRUE;
      continue;
    }
	
	if (strcmp(av[i], "--warnings") == 0) {
      warnings = TRUE;
      continue;
    }
	
    if (strcmp(av[i], "--prompt") == 0) {
      pddl_parser_prompt = TRUE;
      continue;
    }

    if (strcmp(av[i], "--custom") == 0) {
      if (sscanf( av[++i], "%lf", &pddl_parser_set_1 )!=1)	{
        fprintf(stderr, "Unrecognized custom discretization value 1.\n");
        exit(1);
      } else if (sscanf( av[++i], "%lf", &pddl_parser_set_2 )!=1) {
        fprintf(stderr, "Unrecognized custom discretization value 2.\n");
        exit(1);
      } else if (sscanf( av[++i], "%lf", &pddl_parser_set_3 )!=1) {
        fprintf(stderr, "Unrecognized custom discretization value 3.\n");
        exit(1);
      }
      continue;
    }

//UPMURPHI_END
    if (strcmp(av[i], "-h") == 0) {
      help = TRUE;
      PrintInfo();
      exit(1);
    }
//       if ( strncmp(av[i], "-sym", strlen("-sym") ) == 0 )
//       {
//        // we should change it to check whether the number after prefix
//        // is really a number
//         if ( strlen(av[i]) <= strlen("-sym") ) /* We have a space before the number,
//                                     * so it\'s in the next argument. */
//           sscanf( av[++i], "%d", &symmetry_algorithm_number);
//         else
//           sscanf( av[i] + strlen("-sym"), "%d", &symmetry_algorithm_number);
//         continue;
//       }

    if (av[i][0] != '-') {
      if (!strcmp(av[i]+strlen(av[i])-2,".m")) {
        if (m_filename == NULL) m_filename = av[i];
        else {
          fprintf(stderr, "Duplicate input model filename.\n");
          exit(1);
        }
      } else if (!strcmp(av[i]+strlen(av[i])-5,".pddl")) {
        if (pddl_domain_filename == NULL) pddl_domain_filename = av[i];
        else if (pddl_problem_filename == NULL) pddl_problem_filename = av[i];
        else {
          fprintf(stderr, "Too many input domain/problem filenames.\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Unrecognized input file extension: filenames must end with .pddl or .m\n");
        exit(1);
      }
      continue;
    }

    fprintf(stderr,
            "Unrecognized flag. Do '%s -h' for a list of valid arguments.\n",
            av[0]);
    exit(1);
  }

  if (m_filename == NULL && (pddl_domain_filename==NULL || pddl_problem_filename==NULL)) {	// print help
    help = TRUE;
    PrintInfo();
    exit(0);
  }

  if (!keep_source && !compile_source) {
    keep_source=TRUE;
    fprintf(stderr,"Warning: --clean with --noexec has no effect.\n");
  }

  PrintInfo();
}

void argclass::PrintInfo(void)
{
  if (print_license)
    PrintLicense();

  if (!print_license) {
    printf("Run with the -l flag or read the license file for terms\n");
    printf("and conditions of use.\n");
  }

  if (!help)
    printf("Run this program with \"-h\" for the list of options.\n");

  printf("Bugs, questions, and comments should be directed to\n");
  printf("\"wiktor.piotrowski@kcl.ac.uk\".\n\n");

  printf("DiNo/UPMurphi compiler last compiled date: %s\n", __DATE__);
  printf("\
===========================================================================\n");

  if (help)
    PrintOptions();
  fflush(stdout);
}

void argclass::PrintOptions(void)
{
  printf("The options are shown as follows:\n\
\n\
-h                   help\n\
-l                   print license\n\
-nb, --nobitc        disable bit-compacted states\n\
-nc, --nohashc       disable hash compaction\n\
--varweight          support state-dependent rule weight\n\
--varduration        support state-dependent rule duration\n\
--force              skip timestamp checks (force recompilation)\n\
--clean              delete intermediate sources after compilation\n\
--warnings           show model compilation warnings\n\
--noexec             do not (re)compile planner executable \n\
--nopddl             do not (re)compile pddl source \n\
--debug              use step-execution to debug model\n\
\n\
--prompt             make the PDDL parser prompt for various parameters (otherwise use defaults)\n\
--custom X Y Z       set the PDDL parser time quantum, real scale and real fraction digits parameters to X, Y and Z, respectively\n\
\n\
An argument without a leading '-' is taken to be in input filename.\n\
The program accepts two kinds of input files:\n\
* exactly two filenames ending with the .pddl extension,\n\
  which are considered to be the pddl domain and problem files, respectively.\n\
* exactly one filename ending with the .m extension,\n\
  which is considered to be a precompiled DiNo model file.\n");
// \t-sym <num> \t\tsymmetry reduction algorithm <num>\n
}

void argclass::PrintLicense(void)
{
  printf("License Notice: \n\n");

  printf("DiNo is free software; you can redistribute it and/or modify it\n\
under the terms of the GNU Lesser General Public license as\n\
published by the Free Software Foundation; either of the license, or\n\
(at your option) any later version.\n\
\n\
DiNo is currently in BETA, and is distributed in the hope that\n\
it will be useful, but WITHOUT ANY WARRANTY; without even the\n\
implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR\n\
PURPOSE. See the GNU Lesser General Public license for more details.\n\
\n\
You should have received a copy of the GNU Lesser General Public\n\
license along with this library; if not, write to the Free Software\n\
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307\n\
USA.\n\
\n\
The DiNo/UPMurphi modelling language and state space exploration\n\
algorithm are derived from the CMurphi model checker, which in turn\n\
is derived from the Murphi model checker by Stanford. The planning\n\
algorithms have been developed at the University of L'Aquila, Italy.\n\
The SRPG+ heuristic has been developed at King's College London, UK.\n\
\n\
To contact the DiNo development team, email to\n\
<wiktor.piotrowski@kcl.ac.uk>\n\
\n\
===========================================================================\n\
");

  fflush(stdout);
}

/********************
  variable declarations
  ********************/
argclass *args;


/********************
  void err_new_handler()
  ********************/
void err_new_handler()
{
  Error.FatalError("Unable to allocate enough memory.");
}

time_t get_last_modified(char *filename)
{
  struct stat s;
  if (stat(filename,&s)) return 0;
  return s.st_mtime;
}

/********************
  void init_globals()
  -- initialize all global variables
  ********************/
void init_globals()
{
  set_new_handler(&err_new_handler);
  booltype = new enumtypedecl(0, 1);
  booltype->declared = TRUE;
  booltype->mu_name = tsprintf("mu_0_boolean");
  inttype = new enumtypedecl(0, 10000);
  inttype->declared = TRUE;
  realtype = new realtypedecl(6, 2);
  realtype->declared = TRUE;

  /* An enum, not a subrange, because an integer is a primitive type. */
  /* The upper bound doesn\'t really matter; it doesn\'t get used. MAXINT
   * seems like a logical value, but that causes an overflow in the
   * arithmetic used to calculate the number of bits it requires. I\'ve
   * chosen a value that shouldn\'t create any problems, even with small
   * ints. */
  errortype = new errortypedecl((char *) "ERROR");
  voidtype = new typedecl((char *) "VOID_TYPE");
  errorparam = new param(errortype);
  errordecl = new error_decl((char *) "ERROR_DECL");
  error_expr = new expr(0, errortype);
  true_expr = new expr(TRUE, booltype);
  false_expr = new expr(FALSE, booltype);
  error_designator = new designator(NULL, errortype, FALSE, FALSE, FALSE);
  nullstmt = new stmt;
  error_rule = new simplerule(NULL, NULL, NULL, NULL, 0, simplerule::Other, NULL, NULL);
  symtab = new symboltable;
  theprog = new program;
  typedecl::origin = NULL;
}


char *get_inmfilename()
{

  return args->m_filename;
}

char *get_inpddldomainfilename()
{

  return args->pddl_domain_filename;
}

char *get_inpddlproblemfilename()
{

  return args->pddl_problem_filename;
}

char *get_pddldomainfilename()
{

  return get_inpddldomainfilename();
}

char *get_pddlproblemfilename()
{

  return get_inpddlproblemfilename();
}


char *get_mfilename()
{

  static char mfilename[1024];
  if (get_inmfilename()!=NULL) {
    if (Error.CondError(strlen(get_inmfilename()) >= 1015, "Model file name %s must have at most 1015 characters", get_inmfilename())) exit(10);
    strcpy(mfilename, get_inmfilename());
  } else if (get_inpddldomainfilename()!=NULL) {
    if (Error.CondError(strlen(get_inpddldomainfilename()) >= 1015, "Domain file name %s must have at most 1015 characters", get_inpddldomainfilename())) exit(10);
    strcpy(mfilename, get_inpddldomainfilename());
    if (!strcmp(mfilename+strlen(mfilename)-5,".pddl")) {
      mfilename[strlen(mfilename)-5]=0; //remove .pddl extension
    }
    strcat(mfilename,".m");
  } else {
    Error.FatalError("No PDDL or model filenale given");
  }
  return mfilename;
}

char *get_cppfilename()
{

  static char cppfilename[1024];
  strcpy(cppfilename, get_mfilename());
  if (!strcmp(cppfilename+strlen(cppfilename)-2,".m")) {
    cppfilename[strlen(cppfilename)-2]=0; //remove .m extension
  }
  strcat(cppfilename,".cpp");
  return cppfilename;
}

char *get_execfilename()
{

  static char execfilename[1024];
  strcpy(execfilename, get_mfilename());
  if (!strcmp(execfilename+strlen(execfilename)-2,".m")) {
    execfilename[strlen(execfilename)-2]=0; //remove .m extension
  }
  strcat(execfilename,"_planner");
  return execfilename;
}

char *setup_basepaths()
{
  char commandline[2048];
  char *upmurphi_home = getenv("UPMURPHI_HOME");

  if (base_installation_path[0]=='\0') {
    if (!upmurphi_home) {
      size_t len = readlink("/proc/self/exe", base_installation_path, sizeof(base_installation_path)-1);
      if (len != -1) {
        base_installation_path[len] = '\0';
        char *trail = strrchr(base_installation_path,'/');
        if (trail!=NULL) *trail = '\0';
        strcat(base_installation_path,"/");
        strcpy(base_bin_path,base_installation_path);
        strcpy(base_include_path,base_installation_path);
        strcat(base_include_path,"../include");
      } else {
        printf("Warning: UPMURPHI_HOME environment variable not set and unable to get the current executable path.\n");
        printf("Using current working directory.\n");
        base_installation_path[0]='\0';
        base_bin_path[0]='\0';
        base_include_path[0]='\0';
      }
    } else {
      strcpy(base_installation_path,upmurphi_home);
      if (base_installation_path[0]!=0 && base_installation_path[strlen(base_installation_path)-1]!='/') strcat(base_installation_path,"/");
      strcpy(base_bin_path,base_installation_path);
      strcpy(base_include_path,base_installation_path);
      strcat(base_bin_path,"bin/");
      strcat(base_include_path,"include");
    }
  }
  return base_installation_path;
}

int check_infile()
{
  time_t mod_pddl_domain=0, mod_pddl_problem=0, mod_m=0, mod_cpp=0, mod_exec=0;

  if (get_inpddldomainfilename()!=NULL && get_inpddlproblemfilename()!=NULL) {
    mod_pddl_domain = get_last_modified(get_pddldomainfilename());
    mod_pddl_problem = get_last_modified(get_pddlproblemfilename());

    printf("PDDL domain: %s (%s)\n",get_pddldomainfilename(),mod_pddl_domain!=0?"found":"not found");
    printf("PDDL problem: %s (%s)\n",get_pddlproblemfilename(),mod_pddl_problem!=0?"found":"not found");
  }
  mod_m = get_last_modified(get_mfilename());
  mod_cpp = get_last_modified(get_cppfilename());
  mod_exec = get_last_modified(get_execfilename());

  printf("DiNo model: %s (%s)\n",get_mfilename(),mod_m!=0?"found":"not found");
  printf("C++ source: %s (%s)\n",get_cppfilename(),mod_cpp!=0?"found":"not found");
  printf("Executable planner: %s (%s)\n",get_execfilename(),mod_exec!=0?"found":"not found");


  if (mod_pddl_domain!=0 && mod_pddl_problem!=0) return 1;
  else if (mod_m!=0) return 2;
  else Error.FatalError("No PDDL or model file found");

  return 0;
}

/********************
  void setup_mfile(char *filename)
  -- setup input file handler
  ********************/

void setup_mfile()
{
  char *filename = get_mfilename();
  gFileName = filename;
  yyin = fopen(filename, "r");	// yyin is flex\'s global for the input file
  if (yyin == NULL) {
    Error.FatalError("%s:No such file or directory.", filename);
  }
}

FILE *setup_cppfile()
{

  FILE *outfile;
  char *cppfilename = get_cppfilename();

  outfile = fopen(cppfilename, "w");
  if (outfile == NULL) {
    Error.FatalError("Unable to open/create %s", cppfilename);
  };
  return outfile;
}

/********************
  various print routines
  ********************/
void print_header()
{
  printf ("\n\
===========================================================================\n");

  printf ("%s\nDiscretised Nonlinear Heuristic Planner for PDDL+ models with continous processes and events.\n\n", DINO_VERSION);

  printf
  ("%s :\nCopyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio.\n",
   DINO_VERSION);
  printf ("%s is based on UPMurphi release 3.0.\n\n", DINO_VERSION);

  printf
  ("%s :\nCopyright (C) 2007 - 2015: G. Della Penna, B. Intrigila, D. Magazzeni, F. Mercorio.\n",
   MURPHI_VERSION);
  printf ("%s is based on CMurphi release 5.4.\n\n", MURPHI_VERSION);

  printf
  ("CMurphi Release 5.4 :\nCopyright (C) 2001 - 2003 by E. Tronci, G. Della Penna, B. Intrigila, I. Melatti, M. Zilli.");
  printf ("\nCMurphi Release 5.4 is based on Murphi release 3.1.\n\n");
  printf
  ("Murphi Release 3.1 :\nCopyright (C) 1992 - 1999 by the Board of Trustees of\nLeland Stanford Junior University.\n\n");
  printf("===========================================================================\n");
}

int compile_pddl()
{
  char *cppfilename = get_cppfilename();
  char commandline[2048];
  char parameters[50]="";

  if (!args->force_recompile && (get_last_modified(get_pddldomainfilename())<=get_last_modified(get_mfilename()))) {
    printf("DiNo model is up to date\n");
    return 0;
  } else {
    printf("Compiling PDDL to DiNo model, please wait...\n");
    if (args->pddl_parser_prompt) strcpy(parameters,"--prompt");
    else if  (args->pddl_parser_set_1 > 0 && args->pddl_parser_set_2 > 0 && args->pddl_parser_set_3 > 0 ) sprintf(parameters,"--custom %lf %lf %lf",args->pddl_parser_set_1,args->pddl_parser_set_2,args->pddl_parser_set_3);
    sprintf(commandline,"\"%s%s\" %s %s %s",base_bin_path,QUOTED_VALUE(PDDL_PARSER_NAME),get_pddldomainfilename(),get_pddlproblemfilename(),parameters);
    int error = system(commandline);
    if (!error) {
      printf("PDDL compilation successful, no errors\n");
      printf("DiNo model generated in file %s\n", get_mfilename());
    } else fprintf(stderr,"Executed command line: %s\n",commandline);
    return error;
  }
}

int compile_m()
{

  if (!args->force_recompile && (get_last_modified(get_mfilename())<=get_last_modified(get_cppfilename()))) {
    printf("C++ source code is up to date\n");
    return 0;
  } else {
    setup_mfile();
    printf("Compiling model...\n");
    int error = yyparse();
    if (!error && Error.getnumerrors() == 0) {
      //UPMurphi begin: find discretization constant in source
      lexid *lext = ltable.enter(QUOTED_VALUE(DISCRETIZATION_CONST_NAME));
      ste *stet = symtab->find(lext);
      if (stet!=NULL && stet->getvalue()->getclass() == decl::Const && stet->getvalue()->gettype()->gettypeclass() == typedecl::Real) {
        theprog->discretization = ((constdecl*)stet->getvalue())->getrvalue();
      } else {
        Error.FatalError("Unable to find discretization constant T in source file.");
        return 20;
      }
      //UPMurphi end
      //UPMurphi begin: scan for clock variable (TIME)
      lext = ltable.enter(QUOTED_VALUE(CLOCK_VAR_NAME));
      stet = symtab->find(lext);
      if (stet!=NULL && stet->getvalue()->getclass() == decl::Var && stet->getvalue()->gettype()->gettypeclass() == typedecl::Real) {
        theprog->clock_var_name = tsprintf("mu_%s",QUOTED_VALUE(CLOCK_VAR_NAME));
      }
      //UPMurphi end
      codefile = setup_cppfile();
      theprog->generate_code();
      fclose(codefile);
      printf("Model compilation successful, no errors\n");
      if (args->keep_source) printf("C++ source code generated in file %s\n", get_cppfilename());
      return error+Error.getnumerrors();
    }
  }
}

int compile_cpp()
{
  char *cppfilename = get_cppfilename();
  char commandline[2048];

  if (!args->force_recompile && (get_last_modified(get_cppfilename())<=get_last_modified(get_execfilename()))) {
    printf("Executable planner is up to date\n");
    return 0;
  } else {
    printf("Compiling executable planner, please wait...\n"); // WP WP WP WP WP ADDED "-g" FOR DEBUGGING WHEN COMPILING THE MODEL EXECUTABLE
    sprintf(commandline,"%s %s %s -g -I\"%s\" %s -o %s",QUOTED_VALUE(COMPILER_NAME),QUOTED_VALUE(COMPILER_DEFINES),get_cppfilename(),base_include_path,QUOTED_VALUE(COMPILER_SWITCHES),get_execfilename());
    int error = system(commandline);
    if (!error) {
      printf("Planner compilation successful, no errors\n");
      printf("Executable planner generated in file %s\n", get_execfilename());
    } else fprintf(stderr,"Executed command line: %s\n",commandline);
    return error;
  }
}

/********************
  main routines
  ********************/
int main(int argc, char *argv[])
{
  int error, result;
  args = new argclass(argc, argv);
  init_globals();
  print_header();
  setup_basepaths();
  result = check_infile();
  if (result==1 && args->compile_pddl) error = compile_pddl();
  else error=0;
  if (!error) {
    error = compile_m();
    if (!error) {
      if (args->compile_source) {
        error = compile_cpp();
        if (error) exit(2);
        if (!args->keep_source) remove(get_cppfilename());
        printf("Call ./%s to execute the planner with default options\n", get_execfilename());
      }
      exit(0);
    } else {
      exit(1);
    }
  } else {
    exit(1);
  }
}

