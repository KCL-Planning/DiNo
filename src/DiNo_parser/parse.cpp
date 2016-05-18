/*
 main() for the PDDL2.2 parser

 $Date: 2001/08/02 16:44:19 $
 $Revision: 3.2 $

 This expects any number of filenames as arguments, although
 it probably doesn't ever make sense to supply more than two.

 stephen.cresswell@cis.strath.ac.uk

 Strathclyde Planning Group
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include "ptree.h"
#include "FlexLexer.h"

extern int yyparse();
extern int yydebug;
using std::ifstream;
using std::ofstream;
using namespace std;

namespace PDDL2UPMurphi_parser
{
map<string,string>* original_name_map;
bool prompt,customised;
parse_category* top_thing = NULL;
analysis an_analysis;
analysis* current_analysis;
bool state_creation_assistent;
bool goal_printing;
bool make_ground_parameters_now;
bool domain_is_untyped = false;
bool mod_external_func;
string mantissa,mantissa_orig ;
string exponent ;
string precision;
string time_quantum;
ofstream ext_func_stream;
vector<external_effect_> external_effects;
external_effect_ actual_external_effect;
yyFlexLexer* yfl;
string ext_func_filename;
string operator_in_parsing;
string action_in_parsing;
string action_in_parsing_without_pars;

}
;

char *current_filename, *domain_filename, *problem_filename;

using namespace PDDL2UPMurphi_parser;



string simplify(string s)
{
  string toReturn;
  string tosearch1 = "& (true)";
  string tosearch2 = "& true";
  size_t pos;
  while (((pos = s.find(tosearch1)) != string::npos)) {
    s.replace(pos, tosearch1.length(), "");
  }
  while (((pos = s.find(tosearch2)) != string::npos)) {
    s.replace(pos, tosearch2.length(), "");
  }
  return s;
}

void check_syntax()
{
  string cont;
  if (current_analysis->error_list.errors_number() != 0
      || current_analysis->error_list.warnings_number() != 0) {
    current_analysis->error_list.report();
    cout << "Your model has "
         << current_analysis->error_list.errors_number()
         << " errors and "
         << current_analysis->error_list.warnings_number()
         << " warning.\n Are you sure you want to proceed? (Y\\N)\n";
    cin >> cont;
    if (cont == "N" || cont == "n") {
      exit(-1);
    }
  }
}

void print_settings()
{
  cout<<" ----- DOMAIN SETTINGS ----- "<<endl;
  cout<<" Time discretisation: "<<time_quantum<<endl;
  cout<<" Real number: (Integer): "<<mantissa<< " digits for the integer part"<<endl;
  cout<<" Real number: (Fractional) "<<exponent<< " digits for the fractional part"<<endl;
}

void preanalisys(ofstream& outstream, ofstream& ext_func_stream,
                 char* current_filename)
{

  check_syntax();
  cout << "\n\n Start DiNo translation... \n\n";
  string filename = domain_filename;
  ext_func_filename = string(filename);
  replaceAll(filename,".pddl",".m");
  replaceAll(ext_func_filename,".pddl",".h");
  //string filename = current_analysis->the_domain->name;
  // old version, where the name of the file is composed by the name of the domain+problem
  //filename = filename + "_" + current_analysis->the_problem->name+".m";
  //filename = filename + "_" + current_analysis->the_problem->name+".m";

  //ext_func_filename =  current_analysis->the_domain->name + "_" + current_analysis->the_problem->name+".h";


  outstream.open(filename.c_str());
  if (!outstream.is_open()) {
    log_error(E_FATAL, "Failed to create output file\n");
    exit(-1);
  }

  if (!prompt && !customised) {
    time_quantum = "0.1";
    mantissa = "5";
    mantissa_orig = mantissa;
    exponent = "2";
  } else if (customised) {

  } else {
    cout << "Insert the dicretisation quantum\n";
    cin >> time_quantum;
    //cout << "Insert the number of decimal digits\n";
    cout << "Insert the number of digits to be used for representing the *integer* part of real numbers\n";
    cin >> mantissa;
    mantissa_orig = mantissa;
    cout << "Insert the number of digits to be used for representing the *fractional* part of real numbers\n";
    cin >> exponent;

  }
  precision = exponent;

  state_creation_assistent = true;
  goal_printing = true;
  make_ground_parameters_now = true;
  ext_func_stream.open(ext_func_filename.c_str());
  if (!ext_func_stream.is_open()) {
    log_error(E_FATAL, "Failed to create external functions file\n");
    exit(-1);
  } else {
    ext_func_stream
        << "#include <cmath>\n#include <iostream>\n#include <stdlib.h>\n#include <stdio.h>\n\n"
        << "double round_k_digits(double n, unsigned k){\n\t"
        << "double prec = pow(0.1,k);\n\t"
        << "double round = (n>0) ? (n+prec/2) : (n-prec/2);\n\t"
        << "return round-fmod(round,prec);\n" << "}\n\n"
        << "double ext_assignment(double n){\n\t"
        << "return round_k_digits(n,"
        << exponent <<
        ");\n}\n\n" << flush;
  }
  cout<<" ----- CONFIG FILE ----- "<<endl;
  cout << "PDDL Domain File: "<<domain_filename<<endl;
  cout << "PDDL Problem File: "<<problem_filename<<endl;
  cout << "The output model will be written on file: " << filename << "\n";
  cout << "The output external function file will be written on file: "
       << ext_func_filename << "\n";

  print_settings();

}



void merge_domain_problem(char* domain, char* problem)
{
  domain_filename = domain;
  problem_filename = problem;
  current_filename = (char *)"parsing_file.pddl";
  string err;

  ifstream domain_in_stream(domain);
  ifstream problem_in_stream(problem);
  if (domain_in_stream.good() && problem_in_stream.good()) {
    ofstream domain_problem_in_stream(current_filename, std::ofstream::trunc);
    domain_problem_in_stream<<domain_in_stream.rdbuf();
    domain_problem_in_stream<<problem_in_stream.rdbuf();
    domain_in_stream.close();
    problem_in_stream.close();
    domain_problem_in_stream.close();

  } else {
    int c = 0;
    if (!domain_in_stream.good()) {
      err = " domain ";
      c++;
    }
    if (!problem_in_stream.good()) {
      err += " problem ";
      c++;
    }
    string log = "error opening";
    if (c > 1)
      log = log + " both domain and problem files \n" ;
    else
      log = log + err + "file \n" ;
    cout<<log;
    log_error(E_FATAL, log.c_str());
    exit(-1);
  }

}

int main(int argc, char * argv[])
{
  arguments_checker ac(argv,argc);

  current_analysis = &an_analysis;
  ifstream *current_in_stream;
  ofstream outstream;
  yydebug = 0; // Set to 1 to output yacc trace

  yfl = new yyFlexLexer;

  // create a new file containing both domain and problem files into current_filename
  merge_domain_problem(argv[1],argv[2]);
  // and open it
  current_in_stream = new ifstream(current_filename);

  if (current_in_stream->bad()) {
    // Output a message now
    cout << "Failed to open\n";

    // Log an error to be reported in summary later
    line_no = 0;
    log_error(E_FATAL, "Failed to open file");
  } else {
    line_no = 1;
    original_name_map = new map<string,string>();
    // Switch the tokeniser to the current input stream
    yfl->switch_streams(current_in_stream, &cout);
    yyparse();

    //current_analysis->error_list.report();
    //current_analysis->error_list.clear();

    //now the file has been parsed.
    // DiNo/UPMurphi's translation can start!

    // default precision value
    precision = "3";
    preanalisys(outstream, ext_func_stream, current_filename);
    string to_print, to_print_init;


    to_print += current_analysis->toMurphi_declaration(0);
    to_print += current_analysis->the_domain->toMurphi(0,current_analysis->the_problem); //problem passed to retrieve TIL and TIF
    mod_external_func = false;
    to_print += current_analysis->the_problem->toMurphi(
                  current_analysis->the_domain);
    to_print += current_analysis->the_problem->toMurphi_goal(
                  current_analysis->the_domain);
    to_print += current_analysis->the_problem->toMurphi_invariant(
                  current_analysis->the_domain);
    to_print += current_analysis->the_problem->toMurphi_metric(
                  current_analysis->the_domain);
    to_print = current_analysis->the_problem->toMurphi_ext_fun(
                 current_analysis->the_domain, to_print);
    //current_analysis->error_list.report();
    /*
    map<string,string>::iterator it = original_name_map->begin();
    for(; it != original_name_map->end(); it++){
        cout<<it->first<<" \t\t"<<it->second<<endl;
    }
    */
    to_print_init += "domain: file \"";
    to_print_init += domain_filename;
    to_print_init += "\"\n" ;
    to_print_init += "problem: file \"";
    to_print_init += problem_filename;
    to_print_init += "\"\n" ;
    to_print_init += "message: \" Time Discretisation = ";
    to_print_init += time_quantum;
    to_print_init += "\"\n" ;
    to_print_init += "message: \" Digits for representing the integer part of a real =  ";
    to_print_init += mantissa_orig;
    to_print_init += "\"\n" ;
    to_print_init += "message: \" Digits for representing the fractional part of a real =  ";
    to_print_init += exponent;
    to_print_init += "\"\n" ;
    to_print_init += current_analysis->error_list.toMurphi();

    cout << "\n\n ...Translation completed \n\n";
    outstream << simplify(to_print_init+to_print);
    outstream.close();
    ext_func_stream.close();

    current_analysis->error_list.report();

    delete current_in_stream;
  }
  // Output the errors from all input files

  delete yfl;
}
