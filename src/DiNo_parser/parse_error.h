/*-----------------------------------------------------------------------------
  Parse error representation

  $Date: 2001/10/23 12:37:19 $
  $Revision: 3.2 $

  stephen.cresswell@cis.strath.ac.uk
  July 2001.

  Strathclyde Planning Group
 ----------------------------------------------------------------------------
*/


#ifndef PARSE_ERROR_H
#define PARSE_ERROR_H

#include<string>
#include<cstdio>
#include <iostream>

#include "ptree.h"

using std::list;
using std::string;
using std::cout;

extern int line_no;              // Line number global
extern char* current_filename;   // file global

namespace PDDL2UPMurphi_parser {

enum error_severity {E_WARNING,E_FATAL, E_EXIT};


class parse_error
{
private:
    error_severity severity;
    char* filename;
    int line;
    string description;

public:

    parse_error(error_severity s, const string& d) :
	severity(s),
	line(line_no),
	description(d)
	{
	    filename= current_filename;
	};

    // describe error
    void report()
	{
	    cout << filename
		 << ": line: "
		 << line
		 << ": ";

	    if (severity==E_FATAL)
		cout << "Error: ";
	    else
		cout << "Warning: ";

	    cout << description
		 << '\n';
	};

    string toMurphi(){
        string toReturn;
        toReturn += "message: \"";
        toReturn += filename;
        toReturn += ": ";
        if (severity==E_FATAL)
            toReturn += "Error: ";
        else
            toReturn += "Warning: ";
        
        toReturn += description;
        toReturn += "\"\n";
        return toReturn;
        
    };
};


// It seems to be more sensible to keep errors and warnings together in the
// same list, as we want to output them in the same order that they were
// found.

class parse_error_list : public list<parse_error*>
{
public:
    int errors;
    int warnings;

    parse_error_list() : errors(0), warnings(0) {};

    ~parse_error_list()
	{
	    for (iterator i=begin(); i!=end(); ++i)
		delete (*i);
	};


    // parse_error_list is reponsible for creating and
    // destroying parse_error objects,
    void add(error_severity sev, const string& description)
	{
	    //	Use yacc globals to retrieve file and line number;
	    push_back(new parse_error(sev,description));

	    if (sev==E_WARNING)
		++warnings;
	    else
		++errors;
	};

    void report()
	{
	    cout << "\nErrors: " << errors
		 << ", warnings: " << warnings << '\n';

	    for (iterator i=begin(); i!=end(); ++i)
		(*i)->report();
	};

    string toMurphi(){
        string toReturn;
        for (iterator i=begin(); i!=end(); ++i)
            toReturn += (*i)->toMurphi();
        return toReturn;
    };
    

    int errors_number(){
    	return this->errors;
    }

    int warnings_number(){
    	return this->warnings;
    }
};

};

#endif /* PARSE_ERROR_H */
