/*-----------------------------------------------------------------------------
 Class definitions for PDDL2.1 parse trees

 $Date: 2001/10/23 14:45:47 $
 $Revision: 3.2 $

 stephen.cresswell@cis.strath.ac.uk

 Strathclyde Planning Group
 http://planning.cis.strath.ac.uk/
 ----------------------------------------------------------------------------


 In general, data members are pointers to objects allocated using
 new.  Yacc (bison) is not C++-tolerant enough to allow object
 instances to be returned as semantic values, so it is necessary in
 general to return pointers instead.

 Deleting any parse_category class should automatically delete all
 contained structures.  Symbols are an exception to this, as a
 symbol is always owned by a symbol table.
 ----------------------------------------------------------------------------*/

#ifndef PTREE_H
#define PTREE_H

#include <list>
#include <memory>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>
#include <string.h>
#include "sStack.h"
#include "parse_error.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <set>
#include <cmath>


/*-----------------------------------------------------------------------------
 Forward declaration of classes,
 (because in some cases we have mutually referring structures).
 ----------------------------------------------------------------------------*/

using namespace std;
using std::list;
using std::map;
using std::cout;
using std::string;
using std::auto_ptr;
using std::ostream;
using std::vector;

namespace PDDL2UPMurphi_parser {

class parse_category;

class symbol;
class pred_symbol;
class func_symbol;
class pddl_typed_symbol;
class parameter_symbol;
class var_symbol;
class const_symbol;
class pddl_type;
class operator_symbol;

class proposition;
class proposition_list;
class pred_decl;
class func_decl;
class pred_decl_list;
class func_decl_list;

class expression;
class binary_expression;
class plus_expression;
class minus_expression;
class mul_expression;
class div_expression;
class uminus_expression;
class num_expression;
class int_expression;
class float_expression;
class func_term;
class special_val_expr;

class goal_list;
class goal;
class simple_goal;
class qfied_goal;
class conj_goal;
class disj_goal;
class imply_goal;
class neg_goal;
class timed_goal;
class comparison;

class effect;
class simple_effect;
class forall_effect;
class cond_effect;
class timed_effect;
class timed_initial_literal;
class assignment;

class structure_store;
class operator_list;
class derivations_list;
class structure_def;
class operator_;
class action;
class event;
class process;
class durative_action;
class derivation_rule;

class domain;

class metric_spec;
class length_spec;
class problem;

class plan_step;
class plan;

class var_symbol_table_stack;
class analysis;

class VisitController;

#define BBEGIN "BEGIN";
#define DYNAMIC "dynamic_check(); \n";
#define END "\nEND";
#define GLOBAL_CLOCK "TIME";
#define GLOBAL_CLOCK_TYPE "TIME_type";
    
    extern string get_original(string name);


enum quantifier {
	E_FORALL, E_EXISTS
};
enum effect_type {
	F_ADD, F_DEL, F_FORALL, F_COND, F_ASSIGN, F_TIMED
};
enum polarity {
	E_NEG, E_POS
};
enum assign_op {
	E_ASSIGN, E_INCREASE, E_DECREASE, E_SCALE_UP, E_SCALE_DOWN, E_ASSIGN_CTS
};
enum comparison_op {
	E_GREATER, E_GREATEQ, E_LESS, E_LESSEQ, E_EQUALS
};
enum optimization {
	E_MINIMIZE, E_MAXIMIZE
};
enum time_spec {
	E_AT_START, E_AT_END, E_OVER_ALL, E_CONTINUOUS, E_AT
};
enum special_val {
	E_HASHT, E_DURATION_VAR, E_TOTAL_TIME
};
enum length_mode {
	E_SERIAL, E_PARALLEL, E_BOTH
};
enum constraint_sort {
	E_ATEND,
	E_ALWAYS,
	E_SOMETIME,
	E_WITHIN,
	E_ATMOSTONCE,
	E_SOMETIMEAFTER,
	E_SOMETIMEBEFORE,
	E_ALWAYSWITHIN,
	E_HOLDDURING,
	E_HOLDAFTER
};
template<class symbol_class> class typed_symbol_list;

    
extern map<string,string>* original_name_map;
extern bool prompt;
extern bool customised;


    
/*---------------------------------------------------------------------------
 PDDL requirements flags
 -------------------------------------------------------------------------*/

typedef unsigned long pddl_req_flag;

// When changing these, also look at the function pddl_req_attribute_name()
enum pddl_req_attr {
	E_EQUALITY = 1,
	E_STRIPS = 2,
	E_TYPING = 4,
	E_DISJUNCTIVE_PRECONDS = 8,
	E_EXT_PRECS = 16,
	E_UNIV_PRECS = 32,
	E_COND_EFFS = 64,
	E_FLUENTS = 128,
	E_DURATIVE_ACTIONS = 256,
	E_TIME = 512, // Obsolete?
	E_DURATION_INEQUALITIES = 1024,
	E_CONTINUOUS_EFFECTS = 2048,
	E_NEGATIVE_PRECONDITIONS = 4096,
	E_DERIVED_PREDICATES = 8192,
	E_TIMED_INITIAL_LITERALS = 16384,
	E_PREFERENCES = 32768,
	E_CONSTRAINTS = 65536

// Attributes which are defined as combinations of others
// are expanded by parser, and don't need to be included here.
};

/*---------------------------------------------------------------------------
 Functions relating to error handling
 ---------------------------------------------------------------------------*/

void requires(pddl_req_flag);
//string pddl_req_flags_string(pddl_req_flag flags);
string pddl_req_flags_string(pddl_req_flag flags);
void log_error(error_severity sev, const string& description);
void replaceAll(std::string& str, const std::string& from, const std::string& to);

/*----------------------------------------------------------------------------
 --------------------------------------------------------------------------*/
extern string precision;
//static string var_precision = "2";
extern parse_category* top_thing;
extern analysis* current_analysis;
extern bool state_creation_assistent;
extern bool goal_printing;
extern bool make_ground_parameters_now;
extern bool domain_is_untyped;
extern string action_in_parsing;
extern string action_in_parsing_without_pars;
extern string mantissa, mantissa_orig, exponent;
extern string time_quantum;
extern ofstream ext_func_stream;
static bool there_are_durative_action;
extern bool mod_external_func;
extern string ext_func_filename;
extern string operator_in_parsing;

struct external_effect_ {
	string signature;
	string signature_c;
	string call_statement;
	string c_body;
	vector<string> args_par;
	vector<string> args_par_with_par;

};

extern vector<external_effect_> external_effects;
extern external_effect_ actual_external_effect;

struct types_and_objects_ {
	string type;
	vector<string> objects_vector;
};
static map<string, struct types_and_objects_> types_and_objects;

struct functions_with_par_ {
	string name;
	string signature;
	string statement;
	string declaration;
	vector<var_symbol*> type_vector;
	bool is_not_costant;
	bool has_parameters;
};

static map<string, struct functions_with_par_> functions_with_par;

struct predicates_with_par_ {
	string name;
	string statement;
	string declaration;
	bool has_parameters;
};

static map<string, struct predicates_with_par_> predicates_with_par;

struct clocks_with_par_ {
	string name; // durative action name
	string signature;
	string statement;
	string declaration;
	string FOR_signature;
	int number_of_for;
	string indentation;

};

static map<string, struct clocks_with_par_> clocks_with_par;

void insert_into_types_vector(struct types_and_objects_* elem, string toInsert);
void insert_into_object_vector(struct types_and_objects_ elem_of_type,
		string position);
    
    template <typename T> string my_to_string(T value)
    {
        std::ostringstream os ;
        os << std::setprecision(5) ;
        os << value ;
        return os.str() ;
    }
/*---------------------------------------------------------------------------*
 ---------------------------------------------------------------------------*/

class arguments_checker {
private:
	char** argv;
	int argc;
public:
	arguments_checker(char* argv[], int argc) {
		this->argc = argc;
		this->argv = argv;
		if (this->argc <= 2) { // no parameters
			error_message("");
		}
        if (this->argc > 3){
            if (strcmp(this->argv[3],"--prompt") == 0){
                prompt=true;
            }
            else if (strcmp(this->argv[3],"--custom") == 0){
                customised=true;
                if (this->argc == 7){
                    time_quantum = argv[4];
                    mantissa = argv[5];
                    mantissa_orig = mantissa;
                    exponent = argv[6];
                }else{
                    cout<<"Error: --custom modality requires three parameters\n";
                    cout<<usage()<<endl;
                    exit(-1);
                }
            }else{
                cout<<" wrong parameter inserted"<<endl;
                cout<<usage();
                exit(-1);
            }
        }
        
	}

	static string usage() {
		string toReturn;
        toReturn = "Three modalities allowed:\n";
        toReturn += "\t pddl2upm domain.pddl problem.pddl \n \t\t use default discretisation settings (Time: 0.1 / Real: 5 digits for the integer part, 2 digits for fractional part)\n";
        toReturn += "\t pddl2upm domain.pddl problem.pddl --prompt\n \t\t interactive parsing\n";
        toReturn += "\t pddl2upm domain.pddl problem.pddl --custom T Ri Rf\n \t\t specity the time discretisation T, the number of digits for representing the integer part of a real Ri, the number of digits for the fractional part Rf\n";
		return toReturn;
	}

	static string print_licence() {
		string toReturn;
		toReturn = "\n\nDiNo/UPMurphi PDDL+ Compiler Release 1.0R5\n ";
		toReturn
				+= "Copyright (C) 2015 G. Della Penna, B. Intrigila, D. Magazzeni, F. Mercorio\n";
		toReturn
				+= " Parts of this software are derived from the VAL PDDL+ validator\n";
		toReturn
				+= " developed by D. Long, R. Howey, S. Cresswell and M. Fox\n";
		toReturn
				+= "--------------------------------------------------------------------------\n";
		toReturn += "current mantainer: F. Mercorio\n";
		toReturn
				+= "Bugs, questions, and comments should be directed to \"fabio.mercorio@unimib.it\" \n\n";

		return toReturn;
	}

	void error_message(string err) {
		cout << print_licence() << usage() << endl << flush;
		exit(-1);
	}

};

class parse_category {
public:
	parse_category() {
	}
	;
	virtual ~parse_category() {
	}
	;
	virtual string toMurphi(int ind) {
		return "toMurphi di parse_category\n";
	}
};

ostream & operator <<(ostream & o, const parse_category & p);

/*---------------------------------------------------------------------------*
 Specialisation of list template.
 This is used as a list of pointers to parse category entities.
 ---------------------------------------------------------------------------*/

template<class pc>
class pc_list: public list<pc> , public parse_category {

public:
	typedef list<pc> _Base;
	virtual ~pc_list();
	virtual string toMurphi_process_and_events(int indent) {
		string toReturn;
		process * p;
		event * e;

		// print at first process and event
		typename pc_list<pc>::iterator it = _Base::begin();
		for (; it != _Base::end(); it++) {
			//(*it)->display(0);
			pc buffer = *it;
			p = dynamic_cast<process*> (buffer);
			e = dynamic_cast<event*> (buffer);
			if (p != 0 || e != 0) {
				toReturn += buffer->toMurphi(indent) + "\n";
			}
		}

		return toReturn;
	}
	;

	virtual string toMurphi_actions_duractions(int indent) {
		string toReturn;
		durative_action * d;
		action * a;

		// print all rules (actions and durative actions)
		typename pc_list<pc>::iterator it = _Base::begin();
		it = _Base::begin();
		for (; it != _Base::end(); it++) {
			//(*it)->display(0);
			pc buffer = *it;
			a = dynamic_cast<action*> (buffer);
			d = dynamic_cast<durative_action*> (buffer);
			if (a != 0 || d != 0)
				toReturn += buffer->toMurphi(indent) + "\n";
		}
		return toReturn;
	}
	;

	virtual string toMurphi_process(int indent);
	virtual string toMurphi_clocks_declarations(int indent);
	virtual string toMurphi_clocks_init(int indent);
	virtual string toMurphi_event_failure(int indent);
	//virtual string toMurphi_dynamic_check();
	virtual bool exists_durative_action() {
		typename pc_list<pc>::iterator it = _Base::begin();
		for (; it != _Base::end(); it++) {
			//(*it)->display(0);
			durative_action* d = dynamic_cast<durative_action*> (*it);
			if (d != 0) {
				return true;
			}
		}
		return false;
	}
    
	virtual vector<pc> symbol_list() {
		vector<pc> toReturn;
		typename pc_list<pc>::iterator i = _Base::begin();
		for (i = _Base::begin(); i != _Base::end(); ++i) {
			toReturn.push_back(*i);
		}
		return toReturn;
	}
    
    virtual void check_DA_duration_runtime();

};

    

    
/*---------------------------------------------------------------------------*
 Symbol tables
 We have various ways of looking up/adding symbols, depending on whether
 we expect symbol to be already present in the table.
 *---------------------------------------------------------------------------*/

template<class T>
struct SymbolFactory {
	virtual T * build(const string & name) {
		return new T(name);
	}
	;
	virtual ~SymbolFactory() {
	}
	;
};

template<class T, class U>
struct SpecialistSymbolFactory: public SymbolFactory<T> {
	T * build(const string & name) {
		return new U(name);
	}
	;
};

template<class symbol_class>
class symbol_table: public map<string, symbol_class*> {
private:
	typedef map<string, symbol_class*> _Base;
	auto_ptr<SymbolFactory<symbol_class> > factory;

public:

	symbol_table() :
		factory(new SymbolFactory<symbol_class> ()) {
	}
	;

	void setFactory(SymbolFactory<symbol_class> * sf) {
		auto_ptr<SymbolFactory<symbol_class> > x(sf);
		factory = x;
	}
	;

	template<class T>
	void replaceFactory() {
		auto_ptr<SymbolFactory<symbol_class> > x(new SpecialistSymbolFactory<
				symbol_class, T> ());
		factory = x;
	}
	;

	virtual string toMurphi(int indent) {
		return "sono toMurphi di template<class symbol_class> \n";

	}
	;

	typedef typename _Base::iterator iterator;

	typedef typename _Base::const_iterator const_iterator;

	// symbol_ref(string)
	// Don't care whether symbol is already present
	symbol_class* symbol_ref(const string& name) {
		iterator i = _Base::find(name);
		//symbol_table::iterator i= find(name);

		// If name is already in symbol table
		if (i != _Base::end()) {
			// Return existing symbol entry
			return i->second;
		} else {
			// Create new symbol for name and add to table
			symbol_class* sym = factory->build(name);

			this->insert(std::make_pair(name, sym));
			return sym;
		}
	}
	;

	// Look up symbol, returning NULL pointer if not already present.
	//  (so callers must check the result!).
	symbol_class* symbol_probe(const string& name) {
		iterator i = _Base::find(name);
		//symbol_table::iterator i= find(name);

		// If name is already in symbol table
		if (i != _Base::end()) {
			// Return existing symbol entry
			return i->second;
		} else {
			// Otherwise return null pointer
			return NULL;
		}
	}
	;

	vector<symbol*> symbol_list() {
		vector<symbol*> toReturn;

		for (iterator i = _Base::begin(); i != _Base::end(); ++i) {
			toReturn.push_back(i->second);
		}

		return toReturn;
	}

	// Look up symbol, requiring that symbol is already present
	symbol_class* symbol_get(const string& name) {
		iterator i = _Base::find(name);

		//symbol_table::iterator i= find(name);

		// If name is already in symbol table
		if (i != _Base::end()) {
			// Return found symbol
			return i->second;
		} else {
			// Log an error, then add symbol to table anyway.
			log_error(E_FATAL, "Undeclared symbol: " + name);
			symbol_class* sym = factory->build(name);
			this->insert(std::make_pair(name, sym));
			return (sym);
		}
	}
	;



	// Add symbol to table, requiring that symbol is not already present
	symbol_class* symbol_put(const string& name) {
        //string name = upmurphi_keyword_removal(original_name);
		iterator i = _Base::find(name);
		//symbol_table::iterator i= find(name);

		// If name is already in symbol table
		if (i != _Base::end()) {
			// Log an error
			log_error(E_WARNING, "Re-declaration of symbol in same scope: "
					+ name);
			return i->second;
		} else {
			// add new symbol
			symbol_class* sym = factory->build(name);
			this->insert(std::make_pair(name, sym));

			return (sym);
		}
	}
	;

	virtual ~symbol_table() {
		//	    for(symbol_table::iterator i= begin(); i!=end(); ++i)
		for (iterator i = _Base::begin(); i != _Base::end(); ++i)
			delete i->second;
	}
	;
};

// Refinements of symbol tables
typedef symbol_table<var_symbol> var_symbol_table;
typedef symbol_table<const_symbol> const_symbol_table;
typedef symbol_table<pddl_type> pddl_type_symbol_table;
typedef symbol_table<pred_symbol> pred_symbol_table;
typedef symbol_table<func_symbol> func_symbol_table;
typedef symbol_table<operator_symbol> operator_symbol_table;

/*-----------------------------------------------------------------------------
 Lists of symbols
 ---------------------------------------------------------------------------*/

// No destructor for symbol lists
// - destroying the symbols themselves is responsibility of a symbol_table
// - hence we don't make it a pc_list.
template<class symbol_class>
class typed_symbol_list: public list<symbol_class*> , public parse_category {
private:
	typedef list<symbol_class*> _Base;

	string get_one_type_from_list(bool* marks) {
		string toFind;
		string toReturn = "NULL";
		int index = 0;
		bool first = true;
		for (iterator i = _Base::begin(); i != _Base::end(); ++i, index++) {
			if ((*i)->type) {
				if (first && !marks[index]) {
					toFind = (*i)->type->name;
					first = false;

				}
				if ((toFind.compare((*i)->type->name) == 0) & !marks[index]) {
					toReturn = (*i)->type->name;
					marks[index] = true;
				}
			}
		}
		return toReturn;
	}

public:
	typedef typename _Base::iterator iterator;
	typedef typename _Base::const_iterator const_iterator;
	virtual bool isEmpty() {
		return _Base::empty();
	}

	virtual string toMurphi(int indent);

	void set_types(pddl_type* t) {
		//for (typed_symbol_list::iterator i= begin(); i!=end(); ++i)
		for (iterator i = _Base::begin(); i != _Base::end(); ++i) {
			if ((*i)->type) {
				(*i)->either_types = new typed_symbol_list<pddl_type> ;
				(*i)->either_types->push_back((*i)->type);
				(*i)->either_types->push_back(t);
				(*i)->type = 0;
				continue;
			};
			if ((*i)->either_types) {
				(*i)->either_types->push_back(t);
				continue;
			};
			(*i)->type = t;
		};
	}
	;

	void set_either_types(typed_symbol_list<pddl_type>* tl) {
		//for (typed_symbol_list::iterator i= begin(); i!=end(); ++i)
		iterator i = _Base::begin();
		if (i == _Base::end()) {
			return;
		};
		(*i)->either_types = tl;
		++i;
		for (; i != _Base::end(); ++i) {
			(*i)->either_types = new typed_symbol_list<pddl_type> (*tl);
		}
	}
	;

	vector<symbol_class*> symbol_list() {
		vector<symbol_class*> toReturn;

		for (iterator i = _Base::begin(); i != _Base::end(); ++i) {
			toReturn.push_back(*i);
		}

		return toReturn;
	}

	virtual ~typed_symbol_list() {
	}
	;
};

template<class symbol_class>
string typed_symbol_list<symbol_class>::toMurphi(int indent) {
	string toReturn, type_element = "";
	struct types_and_objects_ elem_of_type;
	int dim = (int) _Base::size();
	bool* marks = (bool*) malloc(dim * sizeof(bool));
	for (int i = 0; i < dim; i++) {
		marks[i] = false;
	}

	(type_element = get_one_type_from_list(marks));
	while (type_element.compare("NULL") != 0) {
		toReturn += "\t";
		toReturn += type_element;
		toReturn += " : Enum {";

		//create the types_and_objects structure with type name and list of relative objects elements
		elem_of_type.type = type_element;

		// look for relative objects for type in "type_element"
		for (iterator i = _Base::begin(); i != _Base::end(); ++i) {
			if ((*i)->type && type_element.compare((*i)->type->name) == 0) {
				toReturn += (*i)->name;
				toReturn += ",";
				insert_into_types_vector(&elem_of_type, (*i)->name);

			}
		}

		toReturn.replace(toReturn.length() - 1, 1, "");
		toReturn += "};\n";
		insert_into_object_vector(elem_of_type, type_element);
		type_element = get_one_type_from_list(marks);
		elem_of_type.objects_vector.clear();

	}
	return toReturn;
}

//class var_symbol_list : public typed_symbol_list<var_symbol> {};
typedef typed_symbol_list<var_symbol> var_symbol_list;
typedef typed_symbol_list<parameter_symbol> parameter_symbol_list;
typedef typed_symbol_list<const_symbol> const_symbol_list;
typedef typed_symbol_list<pddl_type> pddl_type_list;

/*----------------------------------------------------------------------------
 Symbols
 used for constants, variables, types, and predicate names.
 Generally, a pointer to a symbol will be used as a unique identifier.
 --------------------------------------------------------------------------*/

class symbol: public parse_category {
protected:

public:
	string name;
	string murphi_decl;
	string murphi_get_set;
	symbol() {
	}
	;
	symbol(const string& s) :
		name(s) {
            this->name = upmurphi_keyword_removal(this->name);
	}
	;

	virtual ~symbol() {
	}
	;
	virtual string toMurphi(int ind) {
		return this->name;
	}
	virtual string toMurphi_decl(int ind) = 0;
	virtual string toMurphi_get_set(int ind) = 0;


    
    string upmurphi_keyword_removal(const string& original_name){
        // list of reserved DiNo/UPMurphi words that cannot be used as symbol name
        /*    
         alias           array           assert          begin
         boolean         by              case            clear
         const           do              else            elsif
         end             endalias        endexists       endfor
         endforall       endfunction     endif           endprocedure
         endrecord       endrule         endruleset      endstartstate
         endswitch       endwhile        enum            error
         exists          false           for             forall
         function        if              in              interleaved
         invariant       of              procedure       process
         program         put             record          return
         rule            ruleset         startstate      switch
         then            to              traceuntil      true
         type            var             while
         */
        string name = original_name;
        // to be extended with UMPurhi keywords
        replaceAll(name,"-","_");
        if (name.compare("alias")==0) replaceAll(name,"alias","alias_");
        if (name.compare("array")==0) replaceAll(name,"array","array_");
        if (name.compare("boolean")==0) replaceAll(name,"boolean","boolean_");
        if (name.compare("const")==0) replaceAll(name,"const","const_");
        if (name.compare("end")==0) replaceAll(name,"end","end_");
        if (name.compare("endforall")==0) replaceAll(name,"endforall","endforall_");
        if (name.compare("endrecord")==0) replaceAll(name,"endrecord","endrecord_");
        if (name.compare("endswitch")==0) replaceAll(name,"endswitch","endswitch_");
        if (name.compare("exists")==0) replaceAll(name,"exists","exists_");
        if (name.compare("function")==0) replaceAll(name,"function","function_");
        if (name.compare("invariant")==0) replaceAll(name,"invariant","invariant_");
        if (name.compare("program")==0) replaceAll(name,"program","program_");
        if (name.compare("rule")==0) replaceAll(name,"rule","rule_");
        if (name.compare("then")==0) replaceAll(name,"then","then_");
        if (name.compare("type")==0) replaceAll(name,"type","type_");
        if (name.compare("assert")==0) replaceAll(name,"assert","assert_");
        if (name.compare("by")==0) replaceAll(name,"by","by_");
        if (name.compare("do")==0) replaceAll(name,"do","do_");
        if (name.compare("endalias")==0) replaceAll(name,"endalias","endalias_");
        if (name.compare("endfunction")==0) replaceAll(name,"endfunction","endfunction_");
        if (name.compare("endrule")==0) replaceAll(name,"endrule","endrule_");
        if (name.compare("endwhile")==0) replaceAll(name,"endwhile","endwhile_");
        if (name.compare("FALSE")==0) replaceAll(name,"FALSE","FALSE_");
        if (name.compare("if")==0) replaceAll(name,"if","if_");
        if (name.compare("of")==0) replaceAll(name,"of","of_");
        if (name.compare("put")==0) replaceAll(name,"put","put_");
        if (name.compare("ruleset")==0) replaceAll(name,"ruleset","ruleset_");
        if (name.compare("to")==0) replaceAll(name,"to","to_");
        if (name.compare("var")==0) replaceAll(name,"var","var_");
        if (name.compare("begin")==0) replaceAll(name,"begin","begin_");
        if (name.compare("case")==0) replaceAll(name,"case","case_");
        if (name.compare("else")==0) replaceAll(name,"else","else_");
        if (name.compare("endexists")==0) replaceAll(name,"endexists","endexists_");
        if (name.compare("endif")==0) replaceAll(name,"endif","endif_");
        if (name.compare("endruleset")==0) replaceAll(name,"endruleset","endruleset_");
        if (name.compare("enum")==0) replaceAll(name,"enum","enum_");
        if (name.compare("for")==0) replaceAll(name,"for","for_");
        if (name.compare("in")==0) replaceAll(name,"in","in_");
        if (name.compare("procedure")==0) replaceAll(name,"procedure","procedure_");
        if (name.compare("record")==0) replaceAll(name,"record","record_");
        if (name.compare("startstate")==0) replaceAll(name,"startstate","startstate_");
        if (name.compare("traceuntil")==0) replaceAll(name,"traceuntil","traceuntil_");
        if (name.compare("while")==0) replaceAll(name,"while","while_");
        if (name.compare("clear")==0) replaceAll(name,"clear","clear_");
        if (name.compare("elsif")==0) replaceAll(name,"elsif","elsif_");
        if (name.compare("endfor")==0) replaceAll(name,"endfor","endfor_");
        if (name.compare("endprocedure")==0) replaceAll(name,"endprocedure","endprocedure_");
        if (name.compare("endstartstate")==0) replaceAll(name,"endstartstate","endstartstate_");
        if (name.compare("error")==0) replaceAll(name,"error","error_");
        if (name.compare("forall")==0) replaceAll(name,"forall","forall_");
        if (name.compare("interleaved")==0) replaceAll(name,"interleaved","interleaved_");
        if (name.compare("process")==0) replaceAll(name,"process","process_");
        if (name.compare("return")==0) replaceAll(name,"return","return_");
        if (name.compare("switch")==0) replaceAll(name,"switch","switch_");
        if (name.compare("TRUE")==0) replaceAll(name,"TRUE","TRUE_");
        if (name.compare("pddlname")==0) replaceAll(name,"pddlname","pddlname_");
        if (name.compare("durative_end")==0) replaceAll(name,"durative_end","durative_end_");
        if (name.compare("clock")==0) replaceAll(name,"clock","clock_");
        if (name.compare("action")==0) replaceAll(name,"action","action_");
        if (name.compare("weight")==0) replaceAll(name,"weight","weight_");
        if (name.compare("duration")==0) replaceAll(name,"duration","duration_");
        //remember the original symbol name
        (*original_name_map)[name] = original_name;
        return name;
    }
    ;
    
	const string getName() const {
		return name;
	}
	;
};

class pred_symbol: public symbol {
public:
	pred_symbol(const string& s) :
		symbol(s) {
	}
	;
	virtual ~pred_symbol() {
	}
	;

	virtual string toMurphi_decl(int ind) {
		this->murphi_decl = this->name + "_value : boolean;";
		return this->murphi_decl;
	}
	;
	virtual string toMurphi_get_set(int ind) {
		this->murphi_get_set = "procedure set_" + this->name
				+ "( value:boolean );\n\t begin\n\t\t" + this->name
				+ "_value := value;\n\t end;\n";
		this->murphi_get_set += "function " + this->name
				+ "():boolean;\n\t begin\n\t\t return " + this->name
				+ "_value;\n\t end;";
		return this->murphi_get_set;
	}
	;

};

class func_symbol: public symbol {
public:
	bool use_as_constant_value;
	func_symbol(const string& s) :
		symbol(s) {
	}
	;
	virtual ~func_symbol() {
	}
	;
	virtual string toMurphi_decl(int ind) {
		this->murphi_decl = this->name + " : real_type;";
		return this->murphi_decl;
	}
	;
	virtual string toMurphi_get_set(int ind) {
		this->murphi_get_set = "";
		return this->murphi_get_set;
	}
	;

	virtual string toMurphi(int ind) {
		return this->name;
	}
};

// Variables, constants or types.
class pddl_typed_symbol: public symbol {
public:
	pddl_type* type; // parent type
	pddl_type_list* either_types; // types declared with 'either'

	pddl_typed_symbol() :
		symbol(""), type(NULL), either_types(NULL) {
	}
	;
	pddl_typed_symbol(const string& s) :
		symbol(s), type(NULL), either_types(NULL) {
	}
	;

	virtual ~pddl_typed_symbol() {
		delete either_types;
	}
	;
	virtual string toMurphi_decl(int ind) {
		log_error(E_WARNING, "not yet supported in this version \n");
		return this->murphi_decl;
	}
	;
	virtual string toMurphi_get_set(int ind);

	virtual string toMurphi(int indent);
    
    virtual string get_par_name(){
        return "["+this->name+"]";
    }
};

// Parameters can be variables or constant symbols
class parameter_symbol: public pddl_typed_symbol {
public:
	parameter_symbol(const string& s) :
		pddl_typed_symbol(s) {
	}
	;
	virtual ~parameter_symbol() {
	}
	;
};

class var_symbol: public parameter_symbol {
public:
	var_symbol(const string& s) :
		parameter_symbol(s) {
	}
	;

	virtual ~var_symbol() {
	}
	;
};

class const_symbol: public parameter_symbol {
public:
	const_symbol(const string& s) :
		parameter_symbol(s) {
	}
	;
	virtual ~const_symbol() {
	}
	;

};

// PDDL types

class pddl_type: public pddl_typed_symbol {
public:
	pddl_type(const string& s) :
		pddl_typed_symbol(s) {
	}
	;
	virtual string toMurphi(int indent) {
		return "Array [" + this->name + "] of ";
	}

	virtual ~pddl_type() {
	}
	;
};

class operator_symbol: public symbol {
public:
	operator_symbol(const string& s) :
		symbol(s) {
	}
	;
	// probably need to also refer to operator itself to enable
	// lookup of operator by name
	virtual ~operator_symbol() {
	}
	;
	virtual string toMurphi_decl(int ind) {
		return this->murphi_decl;
	}
	;
	virtual string toMurphi_get_set(int ind) {
		return this->murphi_get_set;
	}
	;
};

/*---------------------------------------------------------------------------*
 Proposition
 *---------------------------------------------------------------------------*/

class proposition: public parse_category {
public:
	pred_symbol* head;
	parameter_symbol_list* args;

	proposition(pred_symbol* h, parameter_symbol_list* a) :
		head(h), args(a) {
	}
	;

	proposition(pred_symbol* h, var_symbol_list* a) :
		head(h), args(new parameter_symbol_list) {
		for (var_symbol_list::iterator i = a->begin(); i != a->end(); ++i) {
			args->push_back(*i);
		};
	}
	;

	virtual ~proposition() {
		// don't delete head - it belongs to a symbol table
		delete args;
	}
	;

	virtual string toMurphi(int indent);

};

class proposition_list: public pc_list<proposition*> {
};

// Nearly the same as a proposition, but:
//    The arguments must be variables.
//    These variables are local to the declaration,
//     so the pred_decl class has its own symbol table.

class pred_decl: public parse_category {
protected:
public:
	pred_symbol* head;
	var_symbol_list* args;
	var_symbol_table* var_tab;
	pred_decl(pred_symbol* h,
	//	      typed_symbol_list<var_symbol>* a,
			var_symbol_list* a, var_symbol_table* vt) :
		head(h), args(a), var_tab(vt) {
	}
	;

	const pred_symbol * getPred() const {
		return head;
	}
	;
	const var_symbol_list * getArgs() const {
		return args;
	}
	;

	void setTypes(proposition * p) const {
		var_symbol_list::iterator j = args->begin();
		for (parameter_symbol_list::iterator i = p->args->begin(); i
				!= p->args->end(); ++i, ++j) {
			(*i)->type = (*j)->type;
			(*i)->either_types = (*j)->either_types;
		};
	}
	;

	virtual ~pred_decl() {
		delete args;
		delete var_tab;
	}
	;
	virtual string toMurphi(int indent);
	virtual string toMurphi_get_set(int indent);
};

class func_decl: public parse_category {
private:

public:
	func_symbol* head;
	var_symbol_list* args;
	var_symbol_table* var_tab;

	func_decl(func_symbol* h,
	//	      typed_symbol_list<var_symbol>* a,
			var_symbol_list* a, var_symbol_table* vt) :
		head(h), args(a), var_tab(vt) {
	}
	;

	const func_symbol * getFunction() const {
		return head;
	}
	;
	const var_symbol_list * getArgs() const {
		return args;
	}
	;
	virtual string toMurphi(int indent) {
		string toReturn = "\t";
		toReturn += this->head->name + "[pddlname:\"" + get_original(this->head->name) + "\";]";
		toReturn += " : ";
		if (!this->args->isEmpty()) {
			vector<var_symbol*> symbol_vector = this->args->symbol_list();
			vector<var_symbol*>::iterator it = symbol_vector.begin();
			for (; it != symbol_vector.end(); ++it) {
				toReturn += (*it)->toMurphi(indent);
			}
		}
		toReturn += " real_type;\n";
		return toReturn;
	}
	virtual string toMurphi_statement(int indent) {
		string toReturn = this->head->name;
		if (!this->args->isEmpty()) {
			vector<var_symbol*> symbol_vector = this->args->symbol_list();
			vector<var_symbol*>::iterator it = symbol_vector.begin();

			for (; it != symbol_vector.end(); ++it) {
				toReturn += "[";
				toReturn += (*it)->name;
				toReturn += "]";
				toReturn += " , ";
			}
			string temp = toReturn;
			toReturn = temp.substr(0, temp.length() - 3);
		} else {
			toReturn = this->head->name;
		}
		return toReturn;
	}

	virtual ~func_decl() {
		delete args;
		delete var_tab;
	}
	;
};

class pred_decl_list: public pc_list<pred_decl*> {
public:
	virtual string toMurphi(int indent) {
		pred_decl* p;
		string toReturn;
		pred_decl* buffer;
		pc_list<pred_decl*>::iterator it = _Base::begin();
		for (; it != _Base::end(); it++) {
			buffer = *it;
			p = dynamic_cast<pred_decl*> (buffer);
			if (p != 0) {
				toReturn += p->toMurphi(indent);
			}
		}
		return toReturn;
	}
	virtual string toMurphi_get_set(int indent) {
		pred_decl* p;
		string toReturn;
		pred_decl* buffer;
		pc_list<pred_decl*>::iterator it = _Base::begin();
		for (; it != _Base::end(); it++) {
			buffer = *it;
			p = dynamic_cast<pred_decl*> (buffer);
			if (p != 0) {
				toReturn += p->toMurphi_get_set(indent);
			}
		}
		return toReturn;
	}
	virtual ~pred_decl_list() {
	}
	;
};

class func_decl_list: public pc_list<func_decl*> {
public:
	virtual string create_function_map(int indent);
	virtual ~func_decl_list() {
	}
	;
};

/*----------------------------------------------------------------------------
 Expressions
 --------------------------------------------------------------------------*/

class expression: public parse_category {
public:
    virtual string toMurphi(int indent) = 0;
	virtual string toMurphi_external_decl(int indent) = 0;
	virtual string toMurphi_external_call(int indent) = 0;
	virtual bool exist(vector<string> v, string s);

};

class binary_expression: public expression {

public:
    expression * arg1;
    expression * arg2;

	binary_expression(expression * a1, expression * a2) :
		arg1(a1), arg2(a2) {
	}
	;
	virtual ~binary_expression() {
		delete arg1;
		delete arg2;
	}
	;
	const expression * getLHS() const {
		return arg1;
	}
	;
	const expression * getRHS() const {
		return arg2;
	}
	;
	virtual string toMurphi(int indent) {
		return this->arg1->toMurphi(indent) + this->arg2->toMurphi(indent);
	}
	virtual string toMurphi_external_decl(int indent);
	virtual string toMurphi_external_call(int indent);
};

class plus_expression: public binary_expression {
public:
	plus_expression(expression *a1, expression *a2) :
		binary_expression(a1, a2) {
	}
	;

	virtual string toMurphi(int indent) {
		return "(" + this->arg1->toMurphi(indent) + ") + ("
				+ this->arg2->toMurphi(indent) + ")";
	}
};

class minus_expression: public binary_expression {
public:
	minus_expression(expression *a1, expression *a2) :
		binary_expression(a1, a2) {
	}
	;
	virtual string toMurphi(int indent) {
		return "(" + this->arg1->toMurphi(indent) + ") - ("
				+ this->arg2->toMurphi(indent) + ")";
	}
};

class mul_expression: public binary_expression {
public:
	mul_expression(expression *a1, expression *a2) :
		binary_expression(a1, a2) {
	}
	;
	virtual string toMurphi(int indent) {
		return "(" + this->arg1->toMurphi(indent) + ") * ("
				+ this->arg2->toMurphi(indent) + ")";
	}
};

class div_expression: public binary_expression {
public:
	div_expression(expression *a1, expression *a2) :
		binary_expression(a1, a2) {
	}
	;
	virtual string toMurphi(int indent) {
		return "(" + this->arg1->toMurphi(indent) + ") / ("
				+ this->arg2->toMurphi(indent) + ")";
	}
};

class uminus_expression: public expression {
private:
	expression *arg1;
public:
	uminus_expression(expression *a1) :
		arg1(a1) {
	}
	;
	virtual ~uminus_expression() {
		delete arg1;
	}
	;
	const expression * getExpr() const {
		return arg1;
	}
	;
	virtual string toMurphi(int indent) {
		return "-" + this->arg1->toMurphi(indent);
	}
	virtual string toMurphi_external_decl(int indent);
	virtual string toMurphi_external_call(int indent);
};

typedef long double NumScalar;

class num_expression: public expression {
public:
	virtual ~num_expression() {
	}
	;
	virtual const NumScalar double_value() const = 0;
	virtual string toMurphi(int indent) = 0;
	virtual string toMurphi_external_decl(int indent);
	virtual string toMurphi_external_call(int indent);
};

class int_expression: public num_expression {
private:
	int val;
public:
	int_expression(int v) :
		val(v) {
	}
	;
	virtual ~int_expression() {
	}
	;
	const NumScalar double_value() const {
		return static_cast<const NumScalar> (val);
	}
	;
	virtual string toMurphi(int indent);
};

class float_expression: public num_expression {
private:
	NumScalar val;
public:
	float_expression(NumScalar v) :
		val(v) {
	}
	;
	virtual ~float_expression() {
	}
	;
	const NumScalar double_value() const {
		return static_cast<const NumScalar> (val);
	}
	;
	virtual string toMurphi(int indent);
};

class func_term: public expression {
private:
public:
	func_symbol *func_sym;
	parameter_symbol_list *param_list;

	func_term(func_symbol *fs, parameter_symbol_list *psl) :
		func_sym(fs), param_list(psl) {
	}
	;
	virtual ~func_term() {
		delete param_list;
	}
	;
	virtual string toMurphi(int);
	virtual string toMurphi_external_decl(int indent);
	virtual string toMurphi_external_call(int indent);
	const func_symbol * getFunction() const {
		return func_sym;
	}
	;
	const parameter_symbol_list * getArgs() const {
		return param_list;
	}
	;
};

// FIX: this is the duration var
// This class for special values hasht and ?duration
// Not sure what should be done with these.
class special_val_expr: public expression {



public:
    const special_val var;
	special_val_expr(special_val v) :
		var(v) {
	}
	;
	virtual ~special_val_expr() {
	}
	;
	const special_val getKind() const {
		return var;
	}
	;
	virtual string toMurphi(int indent);
	virtual string toMurphi_external_decl(int indent);
	virtual string toMurphi_external_call(int indent);
};

class violation_term: public expression {
private:
	const string name;
public:
	violation_term(const char * n) :
		name(n) {
	}
	;
	const string getName() const {
		return name;
	}
	;
	virtual string toMurphi(int indent) {
		string log = "violation_term not yet supported\n";
		log_error(E_WARNING, log.c_str());
		return "";
	}
	virtual string toMurphi_external_decl(int indent) {
		string log = "violation_term not yet supported\n";
		log_error(E_WARNING, log.c_str());
		return "";
	}
	virtual string toMurphi_external_call(int indent) {
		string log = "violation_term not yet supported\n";
		log_error(E_WARNING, log.c_str());
		return "";
	}
};

// [ end of expression classes ]

/*---------------------------------------------------------------------------
 Goals
 ---------------------------------------------------------------------------*/

class goal_list: public pc_list<goal*> {
public:
	virtual ~goal_list() {
	}
	;
};

class goal: public parse_category {
public:
	virtual string toMurphi(int indent) = 0;
};

class con_goal: public goal {
	virtual string toMurphi(int indent) = 0;
};

class constraint_goal: public con_goal {
private:
	constraint_sort cons;
	goal * requirement;
	goal * trigger;
	double deadline;
	double from;
public:
	constraint_goal(constraint_sort c, goal * g) :
		cons(c), requirement(g), trigger(0), deadline(0), from(0) {
	}
	;
	constraint_goal(constraint_sort c, goal * req, goal * tri) :
		cons(c), requirement(req), trigger(tri), deadline(0), from(0) {
	}
	;
	constraint_goal(constraint_sort c, goal * req, goal * tri, double d,
			double f) :
		cons(c), requirement(req), trigger(tri), deadline(d), from(f) {
	}
	;
	constraint_sort getCons() const {
		return cons;
	}
	;
	goal * getTrigger() const {
		return trigger;
	}
	;
	goal * getRequirement() const {
		return requirement;
	}
	;
	double getDeadline() const {
		return deadline;
	}
	;
	double getFrom() const {
		return from;
	}
	;
	virtual string toMurphi(int indent) {
		string log = "constraint_goal not yet supported\n";
		log_error(E_WARNING, log.c_str());
		return "";
	}
	;
};

class preference: public con_goal {
private:
	string name;
	goal * gl;
public:
	preference(const char * nm, goal * g) :
		name(nm), gl(g) {
	}
	;
	preference(goal * g) :
		name("anonymous"), gl(g) {
	}
	;

	const string & getName() const {
		return name;
	}
	;
	goal * getGoal() const {
		return gl;
	}
	;
	virtual string toMurphi(int indent) {
		string log = "preference goal not yet supported\n";
		log_error(E_WARNING, log.c_str());
		return "";
	}
	;

};

class simple_goal: public goal {
private:
	polarity plrty; // +ve or -ve goals
	proposition* prop;

public:
	simple_goal(proposition* prp, polarity pol) :
		plrty(pol), prop(prp) {
	}
	;
	virtual ~simple_goal() {
		delete prop;
	}
	;
	const polarity getPolarity() const {
		return plrty;
	}
	;
	const proposition * getProp() const {
		return prop;
	}
	;
	virtual string toMurphi(int indent) {
		string toReturn;
		if (this->plrty == E_NEG)
			toReturn += "!";
		toReturn += this->prop->toMurphi(indent);
		return toReturn;
	}
	;

};

class qfied_goal: public con_goal {
private:
	const quantifier qfier;
	var_symbol_list* vars;
	var_symbol_table* sym_tab;
	goal* gl;

public:
	qfied_goal(quantifier q, var_symbol_list* vl, goal* g, var_symbol_table* s) :
		qfier(q), vars(vl), sym_tab(s), gl(g) {
	}
	;
	virtual ~qfied_goal() {
		delete vars;
		delete sym_tab;
		delete gl;
	}
	;
	const quantifier getQuantifier() const {
		return qfier;
	}
	;
	const var_symbol_list* getVars() const {
		return vars;
	}
	;
	const var_symbol_table* getSymTab() const {
		return sym_tab;
	}
	;
	const goal * getGoal() const {
		return gl;
	}
	;
	virtual string toMurphi(int indent) {
		string log = "quantified goal not yet supported\n";
		log_error(E_WARNING, log.c_str());
		return "";
	}
	;

};

class conj_goal: public con_goal {
private:
	goal_list* goals;
public:
	conj_goal(goal_list* gs) :
		goals(gs) {
	}
	;
	virtual ~conj_goal() {
		delete goals;
	}
	;
	const goal_list * getGoals() const {
		return goals;
	}
	;
	virtual string toMurphi(int indent) {
		string toReturn;
		unsigned long size = this->goals->size();
		list<goal*>::iterator it = this->goals->begin();
		for (int counter = 0; it != this->goals->end(); ++it, counter++) {
			toReturn += "(";
			toReturn += (*it)->toMurphi(indent);
			if (counter == size - 1)
				(toReturn += ")");
			else
				(toReturn += ") & ");
		}

		return toReturn;
	}
	;

};

class disj_goal: public goal {
private:
	goal_list* goals;
public:
	disj_goal(goal_list* gs) :
		goals(gs) {
	}
	;
	virtual ~disj_goal() {
		delete goals;
	}
	;
	const goal_list * getGoals() const {
		return goals;
	}
	;
	virtual string toMurphi(int indent) {
		string toReturn;
		unsigned long size = this->goals->size();
		list<goal*>::iterator it = this->goals->begin();
		for (int counter = 0; it != this->goals->end(); ++it, counter++) {
			toReturn += "(";
			toReturn += (*it)->toMurphi(indent);
			if (counter == size - 1)
				(toReturn += ")");
			else
				(toReturn += ") | ");
		}

		return toReturn;
	}
	;
};

class imply_goal: public goal {
private:
	goal* lhs;
	goal* rhs;

public:
	imply_goal(goal* lhs, goal* rhs) :
		lhs(lhs), rhs(rhs) {
	}
	;
	virtual ~imply_goal() {
		delete lhs;
		delete rhs;
	}
	;
	const goal * getAntecedent() const {
		return lhs;
	}
	;
	const goal * getConsequent() const {
		return rhs;
	}
	;
	virtual string toMurphi(int indent) {
		string toReturn;
		toReturn += "!( ";
		toReturn += this->lhs->toMurphi(indent);
		toReturn += ") | ( ";
		toReturn += this->rhs->toMurphi(indent);
		toReturn += ")";
		return toReturn;
	}
	;

};

class neg_goal: public goal {
private:
	goal* gl;

public:
	neg_goal(goal* g) :
		gl(g) {
	}
	;

	virtual ~neg_goal() {
		delete gl;
	}
	;
	virtual void destroy() {
		gl = 0;
		delete this;
	}
	;//do not delete gl
	const goal * getGoal() const {
		return gl;
	}
	;
	virtual string toMurphi(int indent) {
		string toReturn = "!(";
		toReturn += this->gl->toMurphi(indent);
		toReturn += ")";
		return toReturn;
	}
	;

};

class timed_goal: public goal {

public:
    goal* gl;
    time_spec ts;

	timed_goal(goal* g, time_spec t) :
		gl(g), ts(t) {
	}
	;
	virtual ~timed_goal() {
		delete gl;
	}
	;
	goal * clearGoal() {
		goal * gl1 = gl;
		gl = 0;
		return gl1;
	}
	;
	const goal * getGoal() const {
		return gl;
	}
	;
	time_spec getTime() const {
		return ts;
	}
	;
	virtual string toMurphi(int indent) {
		//  E_AT_START, E_AT_END, E_OVER_ALL, E_CONTINUOUS, E_AT
		if ((indent == -1) && (this->ts == E_AT_START)) { // at_start
			return this->gl->toMurphi(indent);
		} else if ((indent == -2) && (this->ts == E_AT_END)) { // at_end
			return this->gl->toMurphi(indent);
		} else if ((indent == -3) && (this->ts == E_OVER_ALL)) { // overall
			return this->gl->toMurphi(indent);
		} else if ((indent == -4) && (this->ts == E_CONTINUOUS)) { // continuous
			log_error(E_WARNING,
					"timed goal E_CONTINUOUS not yet supported \n ");
			return "timed goal E_CONTINUOUS to do";
		} else if ((indent == -5) && (this->ts == E_AT)) { // at
			log_error(E_WARNING, "timed goal E_AT not yet supported \n");
			return "timed goal E_AT to do";
		}
		return "true";
	}
	;

};

class comparison: public goal, public binary_expression // proposition?
{
public:
    comparison_op op;
	comparison(comparison_op c_op, expression* e1, expression* e2) :
		binary_expression(e1, e2), op(c_op) {
	}
	;
	const comparison_op getOp() const {
		return op;
	}
	;
	virtual string toMurphi(int indent) {
		string toReturn = "(( ";
		toReturn += this->arg1->toMurphi(indent);
		toReturn += ")";
		switch (this->op) {
		case (0):
			toReturn += " > ";
			break;
		case (1):
			toReturn += " >= ";
			break;
		case (2):
			toReturn += " < ";
			break;
		case (3):
			toReturn += " <= ";
			break;
		case (4):
			toReturn += " = ";
			break;
		}
		toReturn += "(";
		toReturn += this->arg2->toMurphi(indent);
		toReturn += "))";
		return toReturn;
	}
	;

};

/*---------------------------------------------------------------------------*
 Effect lists
 - a single class containing a separate list of effects of each type
 *---------------------------------------------------------------------------*/

class effect_lists: public parse_category {
public:
	pc_list<simple_effect*> add_effects;
	pc_list<simple_effect*> del_effects;
	pc_list<forall_effect*> forall_effects;
	pc_list<cond_effect*> cond_effects;
	pc_list<assignment*> assign_effects;
	pc_list<timed_effect*> timed_effects;

	effect_lists() {
	}
	;

	virtual ~effect_lists() {
	}
	;
	void append_effects(effect_lists* from);
	virtual string toMurphi(int indent) {

		//F_ADD, F_DEL,F_FORALL , F_COND, F_ASSIGN, F_TIMED
		return this->toMurphi_add_effect(indent) + this->toMurphi_del_effect(
				indent) + this->toMurphi_forall_effect(indent)
				+ this->toMurphi_cond_effect(indent)
				+ this->toMurphi_timed_effect(indent)
				+ this->toMurphi_assign_effect(indent);
	}
	string get_toMurphi_by_name(string name);

private:
    friend class domain;
	virtual string toMurphi_add_effect(int indent);
	virtual string toMurphi_del_effect(int indent);
	virtual string toMurphi_forall_effect(int indent);
	virtual string toMurphi_cond_effect(int indent);
	virtual string toMurphi_timed_effect(int indent);
	virtual string toMurphi_assign_effect(int indent);
	//virtual string toMurphi_aux(effect_type choose_vector, int indent);
};

/*-----------------------------------------------------------------------------
 effect classes
 ---------------------------------------------------------------------------*/

class effect: public parse_category {
public:
	effect() {
	}
	;
	virtual ~effect() {
	}
	;
	virtual string toMurphi(int indent) {
		return " to do tomurphi effect\n";
	}
	;
};

class simple_effect: public effect {
public:
	proposition* prop;

	simple_effect(proposition* eff) :
		effect(), prop(eff) {
	}
	;
	virtual ~simple_effect() {
		delete prop;
	}
	;
	virtual string toMurphi(int indent) {
		return this->prop->toMurphi(indent);
	}

};

class forall_effect: public effect {
private:
	effect_lists* operand;
	var_symbol_list * vars;
	var_symbol_table* var_tab;

public:
	forall_effect(effect_lists* eff, var_symbol_list* vs, var_symbol_table* vt) :
		effect(), operand(eff), vars(vs), var_tab(vt) {
	}
	;

	virtual ~forall_effect() {
		delete operand;
		delete vars;
		delete var_tab;
	}
	;

	virtual string toMurphi(int indent) {
		string log = "forall_effect not yet supported\n";
		log_error(E_WARNING, log.c_str());
		return "";
	}

	const var_symbol_list * getVarsList() const {
		return vars;
	}
	;
	const var_symbol_table * getVars() const {
		return var_tab;
	}
	;
	const effect_lists * getEffects() const {
		return operand;
	}
	;
};

class cond_effect: public effect {
private:
	goal* cond;
	effect_lists* effects;

public:
	// Construct from a list
	cond_effect(goal* g, effect_lists* e) :
		effect(), cond(g), effects(e)

	{
	}
	;

	virtual ~cond_effect() {
		delete cond;
		delete effects;
	}
	;

	virtual string toMurphi(int indent) {
		string log = "cond_effect not yet supported\n";
		log_error(E_WARNING, log.c_str());
		return "";
	}

	const goal * getCondition() const {
		return cond;
	}
	;
	const effect_lists* getEffects() const {
		return effects;
	}
	;
};

class assignment: public effect {
private:
    friend class timed_initial_literal;
    friend class analysis;
	func_term *f_term; // Thing to which value is assigned.
	assign_op op; // Assignment operator, e.g.
	expression *expr; // Value that gets assigned
public:
	bool present_in_startstate;
	assignment(func_term *ft, assign_op a_op, expression *e) :
		f_term(ft), op(a_op), expr(e) {
		present_in_startstate = true;
	}
	;
    virtual string toMurphi(int indent);
	virtual ~assignment() {
		delete f_term;
		delete expr;
	}
	;
	const func_term * getFTerm() const {
		return f_term;
	}
	;
	const expression* getExpr() const {
		return expr;
	}
	;
	const assign_op getOp() const {
		return op;
	}
	;
	string getName() {
		return this->f_term->toMurphi(0);
	}
	string get_func_name() {
		return this->f_term->getFunction()->name;
	}
    bool operator==(const assignment& l){
        return this->f_term->func_sym->name.compare(l.f_term->func_sym->name) == 0;
        
    }
};

class timed_effect: public effect {
public:
	time_spec ts;
	effect_lists* effs;

	timed_effect(effect_lists* e, time_spec t) :
		ts(t), effs(e) {
	}
	;
	virtual ~timed_effect() {
		delete effs;
	}
	;
	virtual string toMurphi(int indent) {
		return this->effs->toMurphi(indent);
	}
	;

};

class timed_initial_literal: public timed_effect {
public:
	float time_stamp;
	~timed_initial_literal() {
		effs = 0;
	}
	;
	//effs->add_effects.clear();effs->del_effects.clear();effs->assign_effects.clear();effs->timed_effects.clear();};
	timed_initial_literal(effect_lists* e, long double t) :
		timed_effect(e, E_AT), time_stamp(t) {
	}
	;
    
    virtual bool contains_assignment_to_fterm(string name){
        if (!this->effs->assign_effects.empty()){
            pc_list<assignment*>::iterator it = this->effs->assign_effects.begin();
            for (; it != this->effs->assign_effects.end(); it++){
                if ((*it)->f_term->getFunction()->name.compare(name) == 0){
                    //cout<<"eccolo: "<<(*it)->f_term->getFunction()->name<<endl;
                    return true;
                }
                
            }
            
        }
        return false;
    };
	virtual string toMurphi(int indent) {
        string toReturn;
        char buffer [50];
        float time_quantum_f = strtof(time_quantum.c_str(),NULL);
        unsigned int time_quantum_u = time_quantum_f*10000;
        unsigned int time_stamp_u = time_stamp*10000;
        if (time_stamp_u % time_quantum_u != 0){
            string log = "The time step "+my_to_string(time_stamp)+" cannot be checked with the time-discretisation step used "+time_quantum;
            log_error(E_WARNING,log.c_str());
        }
            
        toReturn += "IF (";
        toReturn += GLOBAL_CLOCK;
        toReturn += " = ";
        //toReturn += my_to_string(this->time_stamp*2.5);
        sprintf(buffer,"%.3f",this->time_stamp);
        toReturn += buffer;
        toReturn += ") THEN\n";
        if (!this->effs->assign_effects.empty()){
            // assignment effects have to be addressed separately
            pc_list<assignment*>::iterator it = this->effs->assign_effects.begin();
            for (; it != this->effs->assign_effects.end(); it++){
                toReturn += (*it)->f_term->toMurphi(indent);
                toReturn += " := ";
                toReturn += "ext_assignment(";
                toReturn += (*it)->expr->toMurphi(indent);
                toReturn += ");\n";
            }
            
        }else{
            toReturn += this->effs->toMurphi(indent);
        }
        toReturn += "ENDIF;\n";
        return toReturn;

	}
	;
};

/*---------------------------------------------------------------------------
 * Structures
 * --------------------------------------------------------------------------*/

class structure_def: public parse_category {
public:
	virtual ~structure_def() {
	}
	;
	virtual void add_to(operator_list * ops, derivations_list * dvs) {
	}
	virtual string toMurphi(int indent) = 0;
};

/*----------------------------------------------------------------------------
 Operators
 --------------------------------------------------------------------------*/

class operator_list: public pc_list<operator_*> {
public:
	virtual ~operator_list() {
	}
	;

};

class operator_: public structure_def {
public:
	operator_symbol* name;
	var_symbol_table* symtab;
	var_symbol_list* parameters;
	goal* precondition;
	effect_lists* effects;

	operator_() {
	}
	;
	operator_(operator_symbol* nm, var_symbol_list* ps, goal* pre,
			effect_lists* effs, var_symbol_table* st) :
		name(nm), symtab(st), parameters(ps), precondition(pre), effects(effs)

	{
	}
	;
	virtual ~operator_() {
		delete parameters;
		delete precondition;
		delete effects;
		delete symtab;
	}
	;
	virtual string toMurphi(int indent) {
		//string toReturn = "toMurphi di operator " + this->name->name + " \n " + this->precondition->toMurphi(indent) + this->effects->toMurphi(indent);
		string toReturn = "toMurphi di operator " + this->name->name
				+ this->precondition->toMurphi(indent)
				+ this->effects->toMurphi(indent) + "\n";
		return toReturn;
	}
	virtual void add_to(operator_list * ops, derivations_list * dvs) {
		ops->push_back(this);
	}
	;

};

/*-------------------------------------------------------------------------
 * Structure store
 *-------------------------------------------------------------------------*/

class derivations_list: public pc_list<derivation_rule *> {
public:
	virtual ~derivations_list() {
	}
	;
};

template<class pc>
pc_list<pc>::~pc_list() {
	for (typename pc_list<pc>::iterator i = _Base::begin(); i != _Base::end(); ++i)
		delete (*i);
}
;

class structure_store: public parse_category {
private:
	operator_list * ops;
	derivations_list * dvs;
public:
	structure_store() :
		ops(new operator_list), dvs(new derivations_list) {
	}
	;

	void push_back(structure_def * s) {
		if (s) {
			s->add_to(ops, dvs);
		} else {
			log_error(E_FATAL, "Unreadable structure");
		};
	}
	;
	operator_list * get_operators() {
		return ops;
	}
	;
	derivations_list * get_derivations() {
		return dvs;
	}
	;
};

class derivation_rule: public structure_def {
private:
	var_symbol_table * vtab;
	proposition * head;
	goal * body;
	bool body_changed;
public:
	derivation_rule(proposition * p, goal * g, var_symbol_table * v) :
		vtab(v), head(p), body(g), body_changed(false) {
	}
	;
	var_symbol_table* get_vars() const {
		return vtab;
	}
	;
	proposition * get_head() const {
		return head;
	}
	;
	goal * get_body() const {
		return body;
	}
	;
	void set_body(goal * g) {
		body = g;
		body_changed = true;
	}
	;
	virtual string toMurphi(int indent) {
        string log = "derivation rules not yet supported\n";
        log_error(E_WARNING, log.c_str());
        return "";

	}
	virtual ~derivation_rule() {
		delete head;
		if (!body_changed)
			delete body;
	}
	;
	virtual void add_to(operator_list* ops, derivations_list* drvs) {
		drvs->push_back(this);
	}
	;
};

/*----------------------------------------------------------------------------
 Classes derived from operator:
 action
 event
 process
 durative_action
 --------------------------------------------------------------------------*/

class action: public operator_ {
public:
	action(operator_symbol* nm, var_symbol_list* ps, goal* pre,
			effect_lists* effs, var_symbol_table* st) :
		operator_(nm, ps, pre, effs, st) {
	}
	;
	virtual ~action() {
	}
	;
	virtual string toMurphi(int indent);
};

class event: public operator_ {
public:
	event(operator_symbol* nm, var_symbol_list* ps, goal* pre,
			effect_lists* effs, var_symbol_table* st) :
		operator_(nm, ps, pre, effs, st) {
	}
	;
	virtual ~event() {
	}
	;
	virtual string toMurphi(int indent) {
		operator_in_parsing = "event_" + this->name->name;
		string toReturn = "function " + this->name->name + " (";
		if (!this->parameters->isEmpty()) {
			vector<var_symbol*> temp = this->parameters->symbol_list();
			vector<var_symbol*>::iterator it = temp.begin();
			unsigned long size = temp.size();
			for (int i = 0; it != temp.end(); ++it, i++) {
				toReturn += (*it)->name + " : " + (*it)->type->name;
				if (i != size - 1)
					toReturn += " ; ";
			}
		}
		toReturn += ") : boolean; \n";
		toReturn += BBEGIN;
		toReturn += "\n";
		toReturn += "IF (";
		toReturn += this->precondition->toMurphi(indent);
		toReturn += ") THEN \n";
		toReturn += this->effects->toMurphi(indent + 1);
		toReturn += "\t\t return true; \n \t ELSE return false;\n";
		toReturn += "\t ENDIF;\n";
		toReturn += END
		toReturn += ";\n";
		return toReturn;
	}

};

class process: public operator_ {
public:
	process(operator_symbol* nm, var_symbol_list* ps, goal* pre,
			effect_lists* effs, var_symbol_table* st) :
		operator_(nm, ps, pre, effs, st) {
	}
	;
	virtual ~process() {
	}
	;
	virtual string toMurphi(int indent) {
		operator_in_parsing = "process_" + this->name->name;
		string toReturn = "procedure " + this->name->name + " (";
		if (!this->parameters->isEmpty()) {
			vector<var_symbol*> temp = this->parameters->symbol_list();
			vector<var_symbol*>::iterator it = temp.begin();
			unsigned long size = temp.size();
			for (int i = 0; it != temp.end(); ++it, i++) {
				toReturn += (*it)->name + " : " + (*it)->type->name;
				if (i != size - 1)
					toReturn += " ; ";
			}
		}
		toReturn += "); \n";
		toReturn += BBEGIN;
		toReturn += "\n";
		toReturn += "IF (";
		toReturn += this->precondition->toMurphi(indent);
		toReturn += ") THEN \n";
		toReturn += this->effects->toMurphi(-4);
		toReturn += "\nENDIF ; \n";
		toReturn += END
		toReturn += ";\n";
		return toReturn;
	}

};

class durative_action: public operator_ {
public:
    comparison_op duration_compar;
    expression *duration_expr;
	goal* dur_constraint;
	durative_action() {
	}
	;
	virtual ~durative_action() {
		delete dur_constraint;
	}
	;
	virtual string toMurphi(int indent);

	virtual string toMurphi_process(int indent);
    
    virtual string get_bool_clock(){
        string bool_clock = "\t" + this->name->name + "_clock_started";
        vector<var_symbol*> symbol_vector = this->parameters->symbol_list();
        vector<var_symbol*>::iterator it = symbol_vector.begin();
        if (!this->parameters->isEmpty()) { // if at least one parameter is present
            for (; it != symbol_vector.end(); ++it) {
                bool_clock += (*it)->get_par_name();
            }
            
        }
        return bool_clock;
    };
    
    virtual string get_clock(){
        string clock = this->name->name + "_clock";
        vector<var_symbol*> symbol_vector = this->parameters->symbol_list();
        vector<var_symbol*>::iterator it = symbol_vector.begin();
        if (!this->parameters->isEmpty()) { // if at least one parameter is present
            for (; it != symbol_vector.end(); ++it) {
                clock += (*it)->get_par_name();
            }
        }
        return clock;
        
    };

	virtual string toMurphi_clocks_declarations(int indent) {
		string clock = this->name->name + "_clock " + "[pddlname:\"" + get_original(this->name->name) + "\";] : " ;
		string bool_clock = "\t" + this->name->name + "_clock_started" + "[pddlname:\"" + get_original(this->name->name) + "\";] : " ;
		// boolean;\n\t" + this->name->name+"_clock : real_type;\n\t"
		vector<var_symbol*> symbol_vector = this->parameters->symbol_list();
		vector<var_symbol*>::iterator it = symbol_vector.begin();
		if (!this->parameters->isEmpty()) { // if at least one parameter is present
			for (; it != symbol_vector.end(); ++it) {
				clock += (*it)->toMurphi(indent);
				bool_clock += (*it)->toMurphi(indent);
			}

		}
		bool_clock += " boolean ;\n\t";
		//clock += " real_type ;\n";
        clock += " TIME_type ;\n";
		return bool_clock + clock;
	}

	virtual string toMurphi_event_failure(int indent) {
		string toReturn = "function event_" + this->name->name + "_failure( ";
		operator_in_parsing = "event_failure_" + this->name->name;
		string parameters;
		unsigned int size = 0;
		vector<var_symbol*> symbol_vector = this->parameters->symbol_list();
		vector<var_symbol*>::iterator it = symbol_vector.begin();
		if (!this->parameters->isEmpty()) { // if at least one parameter is present
			for (; it != symbol_vector.end(); ++it, size++) {
				toReturn += (*it)->name;
				toReturn += " : ";
				toReturn += (*it)->type->name;
				parameters += "[";
				parameters += (*it)->name;
				parameters += "]";
				if (size < symbol_vector.size() - 1)
					toReturn += "; ";
			}
		}
		toReturn += ") : boolean; \n", toReturn += BBEGIN;
		toReturn += "\n";
		toReturn += "\t IF (" + this->name->name + "_clock_started";
		toReturn += parameters;
		toReturn += ")";
		string precond = this->precondition->toMurphi(-3); // look for overall preconditions
		if (precond.compare("") != 0)
			toReturn += "& !(" + precond + ") ";
		toReturn += "THEN \n";
		toReturn += "\t\t " + this->name->name + "_clock";
		toReturn += parameters;
		toReturn += ":= ";
		toReturn += this->name->name + "_clock";
		toReturn += parameters;
		toReturn += "+ T ;\n";
		toReturn += "\t\t all_event_true := false ;\n";
		toReturn += "\t\t return true; \n \t ELSE return false;\n";
		toReturn += "\t ENDIF;\n";
		toReturn += END;
		toReturn += ";\n";

		return toReturn;

	}

private:
	virtual string toMurphi_start(int indent);

	virtual string toMurphi_end(int indent);

};

/*---------------------------------------------------------------------------
 Domain.
 ---------------------------------------------------------------------------*/

class domain: public parse_category {
public:
	operator_list* ops;
	derivations_list* drvs;
	string name;
	pddl_req_flag req;
	pddl_type_list* types;
	const_symbol_list* constants;
	var_symbol_table* pred_vars; // Vars used in predicate declarations
	pred_decl_list* predicates;
	func_decl_list* functions;
	con_goal * constraints;

	domain(structure_store * ss) :
		ops(ss->get_operators()), drvs(ss->get_derivations()), req(0), types(
				NULL), constants(NULL), pred_vars(NULL), predicates(NULL),
				functions(NULL), constraints(NULL) {
		delete ss;
	}
	;

	virtual ~domain() {
		delete drvs;
		delete ops;
		delete types;
		delete constants;
		delete predicates;
		delete functions;
		delete pred_vars;
		delete constraints;
	}
	;

	virtual string CWA();
    virtual string toMurphi_TIL_TIF(int indent, problem* problem);
    virtual string DAs_ongoing_in_goal_state(){
        string toReturn= "\n\n function DAs_ongoing_in_goal_state() : boolean ; \n var -- local vars declaration \n DA_still_ongoing : boolean;\n BEGIN\n DA_still_ongoing := false;\n";
        durative_action *d;
        string indent,statement;
        unsigned int for_counter = 0;
        vector<operator_*> operator_vector = this->ops->symbol_list();
        vector<operator_*>::iterator it = operator_vector.begin();
        
        string DA_to_be_included;
        for (; it != operator_vector.end(); ++it) {
            d = dynamic_cast<durative_action*> (*it);
            if (d != 0){ // it's a durative action. check the value added by d->check_DA_duration_runtime();
                vector<var_symbol*> param = d->parameters->symbol_list();
                DA_to_be_included += this->visit(param, &for_counter, statement, indent, ",");
                DA_to_be_included += "if (";
                DA_to_be_included += d->get_bool_clock();
                DA_to_be_included += " = true)";
                DA_to_be_included += " then return true;\n endif;\n";
                for (int i=0; i < for_counter; i++)
                    DA_to_be_included += "END; -- close for \n";
            }
        }
        
        toReturn += DA_to_be_included;
        toReturn += "\n return DA_still_ongoing; \n END; -- close begin\n";

        return toReturn;
    }

    virtual string DAs_end_check(){
        string toReturn= "\n\n function DAs_violate_duration() : boolean ; \n var -- local vars declaration \n DA_duration_violated : boolean;\n BEGIN\n DA_duration_violated := false;\n";
        durative_action *d;
        bool include_DA = false;
        string indent,statement;
        unsigned int for_counter = 0;
        vector<operator_*> operator_vector = this->ops->symbol_list();
        vector<operator_*>::iterator it = operator_vector.begin();
        
        string DA_to_be_included;
        for (; it != operator_vector.end(); ++it) {
            d = dynamic_cast<durative_action*> (*it);
            if (d != 0){ // it's a durative action. check the value added by d->check_DA_duration_runtime();
                string temp_DA;
                vector<var_symbol*> param = d->parameters->symbol_list();
                temp_DA += this->visit(param, &for_counter, statement, indent, ",");
                include_DA = false;
                temp_DA += "if (";
                temp_DA += d->get_clock();
                
                switch (d->duration_compar) {
                    case (0): // equal to >
                        break;
                    case (1): // equal to >=
                        break;
                    case (2): // equal to <
                        temp_DA += " >= "; // DA is violated in this case
                        include_DA = true;
                        break;
                    case (3): // equal to <=
                        temp_DA += " > "; // DA is violated in this case
                        include_DA = true;
                        break;
                    case (4): // equal to =
                        temp_DA += " > "; // DA is violated in this case
                        include_DA = true;
                        break;
                }
                if (d->duration_expr != NULL){
                    // What should be printed here? Duration expression or constraint?
                    temp_DA += d->duration_expr->toMurphi(0);
                    //temp_DA += d->dur_constraint->toMurphi(0);
                    
                }
                temp_DA += ") then return true;\n endif;\n";
                for (int i=0; i < for_counter; i++)
                    temp_DA += "END; -- close for \n";
                if (include_DA)
                    DA_to_be_included += temp_DA;
            }
        }
        
        toReturn += DA_to_be_included;
        toReturn += "\n return DA_duration_violated; \n END; -- close begin\n";
        return toReturn;
    }
	virtual string event_check_statement() {
		event* e;
		process* p;
		durative_action* d;
		vector<operator_*> operator_vector = this->ops->symbol_list();
		vector<operator_*>::iterator it = operator_vector.begin();
		string toReturn, process_string, durative_action_string, statement,
				parameters;
		string indent = "   ";
		toReturn = "BEGIN\n event_triggered := true;\n";
		string local_var_decl =
				"\n\nprocedure event_check();\n var -- local vars declaration \n";
		local_var_decl += indent + "event_triggered : boolean;\n";
		string event_name;
		string inside_while =
				"while (event_triggered) do \n event_triggered := false;\n";
		unsigned int unused;
		unsigned int for_counter;
		for (; it != operator_vector.end(); ++it) {
			e = dynamic_cast<event*> (*it);
			p = dynamic_cast<process*> (*it);
			d = dynamic_cast<durative_action*> (*it);
			// create param list for each event and process
			vector<var_symbol*> param = (*it)->parameters->symbol_list();
			if (e != 0) { // is an event
				event_name = (*it)->name->name;
			} else if (d != 0) { // is an event obtained by a durative action failure
				event_name = "event_" + (*it)->name->name + "_failure";
			}
			if (e != 0 || d != 0) { // pick an event
				local_var_decl += indent + this->create_local_var(param,
						event_name, "_triggered : ");
				toReturn += this->visit(param, &for_counter, statement, indent,
						","); //get parameters
				inside_while += this->visit(param, &unused, parameters, indent,
						"]["); //get parameters
				if (param.size() != 0) {
					toReturn += indent + event_name + "_triggered" + "["
							+ parameters + "] := false;\n";
					inside_while += indent + "if(! " + event_name
							+ "_triggered" + "[" + parameters + "]) then \n"
							+ indent + event_name + "_triggered" + "["
							+ parameters + "] := " + event_name + "("
							+ statement + ");\n" + indent
							+ "event_triggered := event_triggered | "
							+ event_name + "_triggered" + "[" + parameters
							+ "]; \n" + indent + "endif;\n";
				} else {
					toReturn += indent + event_name + "_triggered := false;";
					inside_while += indent + "if(! " + event_name
							+ "_triggered" + ") then \n" + indent + event_name
							+ "_triggered" + " := " + event_name + "();\n"
							+ indent + "event_triggered := event_triggered | "
							+ event_name + "_triggered" + "; \n" + indent
							+ "endif;\n";
				}
				if (!param.empty()) {
					toReturn += indent;
					for (unsigned int i = 0; i < for_counter; i++) {
						toReturn += "END;";
						inside_while += "END;";
					}
					toReturn += " -- close for";
					inside_while += " -- close for";
				}
				toReturn += "\n";
				inside_while += "\n";
			}
			statement = "";
			parameters = "";
			indent = "   ";
		}
		inside_while += "END; -- close while loop \n";
		toReturn += inside_while;
		toReturn += "END;\n\n";
		return local_var_decl + toReturn;
	}

	virtual string apply_continuous_change_statement() {
		event* e;
		process* p;
		durative_action* d;
		vector<operator_*> operator_vector = this->ops->symbol_list();
		vector<operator_*>::iterator it = operator_vector.begin();
		string toReturn, process_string, durative_action_string, statement,
				parameters;
		string indent = "   ";
		toReturn = "BEGIN\n process_updated := false; end_while := false;\n";
		string local_var_decl =
				"\n\nprocedure apply_continuous_change();\n var -- local vars declaration \n";
		local_var_decl += indent
				+ "process_updated : boolean;\n end_while : boolean;";
		string process_name;
		string inside_while = "while (!end_while) do \n ";
		unsigned int unused;
		unsigned int for_counter;
		for (; it != operator_vector.end(); ++it) {
			e = dynamic_cast<event*> (*it);
			p = dynamic_cast<process*> (*it);
			d = dynamic_cast<durative_action*> (*it);
			// create param list for each event and process
			vector<var_symbol*> param = (*it)->parameters->symbol_list();
			if (p != 0) { // is a process
				process_name = (*it)->name->name;
			} else if (d != 0) { // is a process given by a durative action
				process_name = "process_" + (*it)->name->name;
			}
			if (p != 0 || d != 0) { // pick a process
				local_var_decl += indent + this->create_local_var(param,
						process_name, "_enabled : ");
				toReturn += this->visit(param, &for_counter, statement, indent,
						","); //get parameters
				inside_while += this->visit(param, &unused, parameters, indent,
						"]["); //get parameters
				if (param.size() != 0) {
					toReturn += indent + process_name + "_enabled" + "["
							+ parameters + "] := false;\n";
					/*
					 inside_while += indent + "if(! " + process_name + "_enabled" + "[" + parameters + "]) then \n" +
					 indent +  process_name + "_enabled" + "[" + parameters + "] := true;\n" +
					 indent + process_name +"(" + statement + ");\n" +
					 indent + "process_updated := true; \n" +
					 indent + "endif;\n";
					 */
					inside_while += indent + "if ("
							+ (*it)->precondition->toMurphi(0)
							+ " & [[DA_CONDITION]] !" + process_name
							+ "_enabled[" + parameters + "]) then\n" + indent
							+ "process_updated := true;\n" + indent
							+ process_name + "(" + statement + ");\n" + indent
							+ process_name + "_enabled[" + parameters
							+ "] := true;\n" + indent + "endif;\n";
				} else {
					toReturn += indent + process_name + "_enabled := false;";
					inside_while += indent + "if ("
							+ (*it)->precondition->toMurphi(0)
							+ " & [[DA_CONDITION]] !" + process_name
							+ "_enabled) then\n" + indent
							+ "process_updated := true;\n" + indent
							+ process_name + "();\n" + indent + process_name
							+ "_enabled := true;\n" + indent + "endif;\n";
				}
				unsigned long pos = inside_while.find("[[DA_CONDITION]]");
				if (d != 0) { // is a durative action process
					if (pos == string::npos) {
						cout
								<< "Error in apply_continuous_change_statement()\n"
								<< flush;
						exit(-1);
					}
					if (param.empty()) {
						string added_condition = (*it)->name->name
								+ "_clock_started & ";
						inside_while = inside_while.replace(pos, 16,
								added_condition);
					} else {
						string added_condition = (*it)->name->name
								+ "_clock_started[" + parameters + "] & ";
						inside_while = inside_while.replace(pos, 16,
								added_condition);
					}
				} else { // is a process
					if (pos == string::npos) {
						cout
								<< "Error in apply_continuous_change_statement()\n"
								<< flush;
						exit(-1);
					}
					inside_while = inside_while.replace(pos, 16, "");
				}
				if (!param.empty()) {
					toReturn += indent;

					for (unsigned int i = 0; i < for_counter; i++) {
						toReturn += "END;";
						inside_while += "END;";
					}
					toReturn += " -- close for";

					inside_while += " -- close for";
				}
				toReturn += "\n";
				inside_while += "\n";
			}
			statement = "";
			parameters = "";
			indent = "   ";
		}
		inside_while
				+= "IF (!process_updated) then\n\t end_while:=true;\n else process_updated:=false;\nendif;";
		inside_while += "END; -- close while loop \n";
		toReturn += inside_while;

		toReturn += "END;\n\n";
		return local_var_decl + toReturn;
	}

	//	virtual string apply_continuous_change_statement(){
	//		event* e;
	//		process* p;
	//		durative_action* d;
	//		vector<operator_*> operator_vector = this->ops->symbol_list();
	//		vector<operator_*>::iterator it = operator_vector.begin();
	//		string toReturn,process_string,durative_action_string,statement;
	//		string indent = "   ";
	//		toReturn = "procedure dynamic_check();\nBEGIN\n";
	//		unsigned int for_counter;
	//		for(; it != operator_vector.end(); ++it){
	//			e = dynamic_cast<event*>(*it);
	//			p = dynamic_cast<process*>(*it);
	//			d = dynamic_cast<durative_action*>(*it);
	//			// create param list for each event and process
	//			vector<var_symbol*> param = (*it)->parameters->symbol_list();
	//			if (e != 0 || p != 0){	// events and processes
	//				toReturn += this->visit(param,&for_counter,statement,indent,",");	//get parameters
	//				toReturn += indent + (*it)->name->name + "(" + statement + " );\n";
	//				if (!param.empty()){
	//					for(unsigned int i=0; i < for_counter ; i++)
	//						toReturn += "END; ";
	//					toReturn += " -- close for";
	//				}
	//				toReturn += "\n";
	//			}else if (d != 0){		// durative action
	//				toReturn += this->visit(param,&for_counter,statement,indent,",");	//get parameters
	//				toReturn += indent + "process_" + (*it)->name->name + "(" + statement + " );\n";
	//				toReturn += indent + "event_" + (*it)->name->name + "_failure(" + statement + " );\n";
	//				if (!param.empty()){
	//					for(unsigned int i=0; i < for_counter ; i++)
	//						toReturn += "END; ";
	//					toReturn += " -- close for";
	//				}
	//				toReturn += "\n";
	//			}
	//			statement = "";
	//			indent = "   ";
	//		}
	//		toReturn += "END;";
	//		return toReturn;
	//	}

    
	virtual string toMurphi(int indent, problem* problem) {
		string toReturn;
		toReturn += this->ops->toMurphi_process_and_events(indent);
		toReturn += this->event_check_statement();
        toReturn += this->DAs_end_check();
        toReturn += this->DAs_ongoing_in_goal_state();
		toReturn += this->apply_continuous_change_statement();
		toReturn += this->ops->toMurphi_actions_duractions(indent);
		//if (this->ops->exists_durative_action()) {
        toReturn += "clock rule \" time passing \" \n "
            "(true) ==> \n"
            "BEGIN \n \t";
        toReturn += GLOBAL_CLOCK;
        toReturn += " := ";
        toReturn += GLOBAL_CLOCK;
        toReturn += " + T;\n";
        toReturn += this->toMurphi_TIL_TIF(indent, problem);
        toReturn += "\n \t event_check();\n\t apply_continuous_change();\n\t event_check();\n";
        toReturn += "END;\n";
        
		//}
		//toReturn += this->ops->toMurphi_dynamic_check();
		return toReturn;

	}
	bool isDurative() const {
		return req & (E_DURATIVE_ACTIONS | E_TIME);
	}
	;
	bool isTyped() const {
		return req & (E_TYPING);
	}
	;
private:
	string create_local_var(vector<var_symbol*> param, string event_name,
			string suffix) {
		vector<var_symbol*>::iterator j = param.begin();
		string toReturn;
		toReturn += event_name;
		toReturn += suffix;
		for (unsigned int for_counter = 0; j != param.end(); ++j, for_counter++) {
			toReturn += " Array [" + (*j)->type->name + "] of ";
		}
		toReturn += " boolean;\n";
		return toReturn;
	}

	string visit(vector<var_symbol*> param, unsigned int* for_counter,
			string& statement, string& indent, string separator) {
		vector<var_symbol*>::iterator j = param.begin();
		string toReturn;
		for (*for_counter = 0; j != param.end(); ++j, (*for_counter)++) {
			toReturn += indent + "for " + (*j)->name + " : " + (*j)->type->name
					+ " do \n";
			indent += "  ";
			if (*for_counter < param.size() - 1)
				statement += (*j)->name + separator;
			else
				statement += (*j)->name;
		}

		return toReturn;
	}
};

/*----------------------------------------------------------------------------
 Plan
 ----------------------------------------------------------------------------*/

class plan_step: public parse_category {
public:
	operator_symbol* op_sym;
	const_symbol_list* params;

	bool start_time_given;
	bool duration_given;
	double start_time;
	double duration;
	double originalDuration; //for testing duration constraints when testing robustness

	plan_step(operator_symbol* o, const_symbol_list* p) :
		op_sym(o), params(p) {
	}
	;

	virtual ~plan_step() {
		delete params;
	}
	;

};

class plan: public pc_list<plan_step*> {
private:
	double timeTaken;
public:
	plan() :
		timeTaken(-1) {
	}
	;
	virtual ~plan() {
	}
	;
	void insertTime(double t) {
		timeTaken = t;
	}
	;
	double getTime() const {
		return timeTaken;
	}
	;
};

/*----------------------------------------------------------------------------
 PDDL+ entities
 --------------------------------------------------------------------------*/

class metric_spec: public parse_category {
public:
	optimization opt;
	expression* expr;

	metric_spec(optimization o, expression* e) :
		opt(o), expr(e) {
	}
	;
	virtual ~metric_spec() {
		delete expr;
	}
	;
};

class length_spec: public parse_category {
public:
	length_mode mode;
	int lengths;
	int lengthp;

	length_spec(length_mode m, int l) :
		mode(m), lengths(l), lengthp(l) {
	}
	;
	length_spec(length_mode m, int ls, int lp) :
		mode(m), lengths(ls), lengthp(lp) {
	}
	;
	virtual ~length_spec() {
	}
	;
};

/*----------------------------------------------------------------------------
 Problem definition
 --------------------------------------------------------------------------*/

class problem: public parse_category {

public:
	char * name;
	char * domain_name;
	pddl_req_flag req;
	pddl_type_list* types;
	const_symbol_list* objects;
	effect_lists* initial_state;
	goal* the_goal;
	con_goal *constraints;
	metric_spec* metric;
	length_spec* length;

	problem() :
		name(0), domain_name(0), req(0), types(NULL), objects(NULL),
				initial_state(NULL), the_goal(NULL), constraints(NULL), metric(
						NULL), length(NULL) {
	}
	;

	virtual ~problem() {
		delete[] name;
		delete[] domain_name;
		delete types;
		delete objects;
		delete initial_state;
		delete the_goal;
		delete constraints;
		delete metric;
		delete length;
	}
	;
	virtual string toMurphi(domain* the_domain);
	virtual string toMurphi_goal(domain* the_domain) {
		int indent;
		string toReturn;
		toReturn = "goal \"enjoy\" \n " + this->the_goal->toMurphi(indent) + "& !DAs_ongoing_in_goal_state()"
				+ "; \n\n";
		return toReturn;
	}
	virtual string toMurphi_invariant(domain* the_domain) {
		string toReturn;
		toReturn = "invariant \"todo bien\" \n all_event_true & !DAs_violate_duration();\n";
		return toReturn;
	}

	virtual string toMurphi_metric(domain* the_domain) {
		string toReturn;
		toReturn = "metric: ";
		if (this->metric != NULL){
			if (this->metric->opt == E_MAXIMIZE)
				toReturn += "maximize;\n";
			else if (this->metric->opt == E_MINIMIZE)
				toReturn += "minimize;\n";

			special_val_expr* d =
					dynamic_cast<special_val_expr*> (this->metric->expr);
			if (d == 0) {
				log_error(E_WARNING,
						"optimisation expression not yet supported. Default \"total-time\" used\n");
			}
		}else{ //minimize as default
			toReturn += "minimize;\n";
		}

		return toReturn;
	}

	virtual string toMurphi_ext_fun(domain* the_domain, string to_replace);

};

/*----------------------------------------------------------------------------
 We need to be able to search back through the tables in the stack to
 find a reference to a particular symbol.  The standard STL stack
 only allows access to top.

 The symbol_ref() member function does this, making use of its access
 to the iterator for the stack.
 --------------------------------------------------------------------------*/

class var_symbol_table_stack: public sStack<var_symbol_table*> {
public:
	var_symbol* symbol_get(const string& name);
	var_symbol* symbol_put(const string& name);
    ~var_symbol_table_stack() {
		for (deque<var_symbol_table*>::const_iterator i = begin(); i != end(); ++i)
			delete (*i);
	}
	;
};

/*---------------------------------------------------------------------------*
 Analysis.
 Here we store various symbol tables for constants, types, and predicates.
 For variables, we have a stack of symbol tables which is used during
 parsing.
 Operators and quantified constructs have their own local scope,
 and their own symbol tables.
 *---------------------------------------------------------------------------*/

class VarTabFactory {
public:
	virtual ~VarTabFactory() {
	}
	;
	virtual var_symbol_table * buildPredTab() {
		return new var_symbol_table;
	}
	;
	virtual var_symbol_table * buildFuncTab() {
		return new var_symbol_table;
	}
	;
	virtual var_symbol_table * buildForallTab() {
		return new var_symbol_table;
	}
	;
	virtual var_symbol_table * buildExistsTab() {
		return new var_symbol_table;
	}
	;
	virtual var_symbol_table * buildRuleTab() {
		return new var_symbol_table;
	}
	;
	virtual var_symbol_table * buildOpTab() {
		return new var_symbol_table;
	}
	;
};

class StructureFactory {
public:
	virtual ~StructureFactory() {
	}
	;
	virtual action * buildAction(operator_symbol* nm, var_symbol_list* ps,
			goal* pre, effect_lists* effs, var_symbol_table* st) {
		return new action(nm, ps, pre, effs, st);
	}
	;
	virtual durative_action * buildDurativeAction() {
		return new durative_action;
	}
	;
	virtual event * buildEvent(operator_symbol* nm, var_symbol_list* ps,
			goal* pre, effect_lists* effs, var_symbol_table* st) {
		return new event(nm, ps, pre, effs, st);
	}
	;
	virtual process * buildProcess(operator_symbol* nm, var_symbol_list* ps,
			goal* pre, effect_lists* effs, var_symbol_table* st) {
		return new process(nm, ps, pre, effs, st);
	}
	;
};

class analysis {
private:
	auto_ptr<VarTabFactory> varTabFactory;
	auto_ptr<StructureFactory> strucFactory;

public:
    bool check_assignment_effect_for_const_decl(string name);
	var_symbol_table * buildPredTab() {
		return varTabFactory->buildPredTab();
	}
	;
	var_symbol_table * buildFuncTab() {
		return varTabFactory->buildFuncTab();
	}
	;
	var_symbol_table * buildForallTab() {
		return varTabFactory->buildForallTab();
	}
	;
	var_symbol_table * buildExistsTab() {
		return varTabFactory->buildExistsTab();
	}
	;
	var_symbol_table * buildRuleTab() {
		return varTabFactory->buildRuleTab();
	}
	;
	var_symbol_table * buildOpTab() {
		return varTabFactory->buildOpTab();
	}
	;

	durative_action * buildDurativeAction() {
		return strucFactory->buildDurativeAction();
	}
	;
	action * buildAction(operator_symbol* nm, var_symbol_list* ps, goal* pre,
			effect_lists* effs, var_symbol_table* st) {
		return strucFactory->buildAction(nm, ps, pre, effs, st);
	}
	;
	event * buildEvent(operator_symbol* nm, var_symbol_list* ps, goal* pre,
			effect_lists* effs, var_symbol_table* st) {
		return strucFactory->buildEvent(nm, ps, pre, effs, st);
	}
	;
	process * buildProcess(operator_symbol* nm, var_symbol_list* ps, goal* pre,
			effect_lists* effs, var_symbol_table* st) {
		return strucFactory->buildProcess(nm, ps, pre, effs, st);
	}
	;

	void setFactory(VarTabFactory * vf) {
		auto_ptr<VarTabFactory> x(vf);
		varTabFactory = x;
	}
	;

	void setFactory(StructureFactory * sf) {
		auto_ptr<StructureFactory> x(sf);
		strucFactory = x;
	}
	;

	var_symbol_table_stack var_tab_stack;
	const_symbol_table const_tab;
	pddl_type_symbol_table pddl_type_tab;
	pred_symbol_table pred_tab;
	func_symbol_table func_tab;
	operator_symbol_table op_tab;
	pddl_req_flag req;

	parse_error_list error_list;

	domain* the_domain;
	problem* the_problem;

	analysis() :
		varTabFactory(new VarTabFactory), strucFactory(new StructureFactory),
				the_domain(NULL), the_problem(NULL) {
		// Push a symbol table on stack as a safety net
		var_tab_stack.push(new var_symbol_table);
	}

	virtual ~analysis() {
		delete the_domain;
		delete the_problem;
	}
	;

	virtual string toMurphi_declaration(int indent);
};

template<class pc>
string pc_list<pc>::toMurphi_process(int indent) {
	string toReturn;
	typename pc_list<pc>::iterator it = _Base::begin();
	for (; it != _Base::end(); it++) {
		//(*it)->display(0);
		durative_action* d = dynamic_cast<durative_action*> (*it);
		if (d != 0) {
			toReturn += d->toMurphi_process(indent);
		}
	}
	return toReturn;
}
;

template<class pc>
string pc_list<pc>::toMurphi_clocks_declarations(int indent) {
	string toReturn;
	typename pc_list<pc>::iterator it = _Base::begin();
	for (; it != _Base::end(); it++) {
		//(*it)->display(0);
		durative_action* d = dynamic_cast<durative_action*> (*it);
		if (d != 0) {
			toReturn += d->toMurphi_clocks_declarations(indent);
		}
	}
	return toReturn;
}
;

template<class pc>
string pc_list<pc>::toMurphi_clocks_init(int indent) {
	string toReturn;
	typename pc_list<pc>::iterator it = _Base::begin();
	for (; it != _Base::end(); it++) {
		//(*it)->display(0);
		durative_action* d = dynamic_cast<durative_action*> (*it);
		if (d != 0) {
			//toReturn += d->toMurphi_clocks_init(indent);
		}
	}
	return toReturn;
}
;

template<class pc>
string pc_list<pc>::toMurphi_event_failure(int indent) {
	string toReturn;
	typename pc_list<pc>::iterator it = _Base::begin();
	for (; it != _Base::end(); it++) {
		//(*it)->display(0);
		durative_action* d = dynamic_cast<durative_action*> (*it);
		if (d != 0) {
			toReturn += d->toMurphi_event_failure(indent);
		}
	}
	return toReturn;
}
;

    template<class pc>
    void pc_list<pc>::check_DA_duration_runtime() {
        typename pc_list<pc>::iterator it = _Base::begin();
        for (; it != _Base::end(); it++) {
            //(*it)->display(0);
            durative_action* d = dynamic_cast<durative_action*> (*it);
            if (d != 0) {
                timed_goal* tg = dynamic_cast<timed_goal*> (d->dur_constraint);
                if (tg != 0){
                    comparison* cp = dynamic_cast<comparison*> (tg->gl);
                    if (cp != 0){
                        d->duration_compar = cp->op;    // store the comparison operator within the DA
                        d->duration_expr = cp->arg2;    // store the duration specification (right side of expr)
                    }
                    
                }
            }
        }
    }
    

/*template<class pc>
 string pc_list<pc>::toMurphi_dynamic_check() {
 string toReturn = "procedure dynamic_check();\nBEGIN\n";
 typename pc_list<pc>::iterator it = _Base::begin();
 durative_action* d;
 process* p;
 action* a;
 event* e;
 for(; it != _Base::end(); it++) {
 d = dynamic_cast<durative_action*>(*it);
 a = dynamic_cast<action*>(*it);
 p = dynamic_cast<process*>(*it);
 e = dynamic_cast<event*>(*it);
 if (d != 0) {
 toReturn += "process_"+d->name->name+"();\n";
 toReturn += "event_"+d->name->name+"_failure();\n";
 } else if (p != 0) {
 toReturn += p->name->name+"();\n";
 } else if (e != 0) {
 toReturn += e->name->name+"();\n";
 }
 }
 toReturn +="END;\n";
 return toReturn;

 };*/
}
;

#endif /* PTREE_H */

