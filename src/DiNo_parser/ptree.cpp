/*-----------------------------------------------------------------------------
 Member functions for parse tree classes

 $Date: 2001/10/23 12:18:00 $
 $Revision: 3.2 $

 stephen.cresswell@cis.strath.ac.uk
 July 2001.

 Strathclyde Planning Group

 ----------------------------------------------------------------------------
 */

// prova per la commit

#include "ptree.h"
#include <memory>

/*-----------------------------------------------------------------------------
 Parse category
 ---------------------------------------------------------------------------*/

namespace PDDL2UPMurphi_parser
{

string get_original(string name)
{
  if (original_name_map->empty()) {
    cout<<"Error in retrieving the original name for symbol: "<<name<<endl;
    exit(-1);
  }
  return (*original_name_map)[name];
}
;

string special_val_expr::toMurphi(int indent)
{
  if (this->var == E_HASHT) {
    return " T ";
  } else if (this->var == E_DURATION_VAR && indent == -10) {
    return action_in_parsing_without_pars;
  } else if (this->var == E_DURATION_VAR) {
    return action_in_parsing;
  } else {
    string log = "special_val_expr not yet supported\n";
    log_error(E_WARNING, log.c_str());
    return "";
  }
}

string durative_action::toMurphi(int indent)
{
  action_in_parsing = this->name->name + "_clock";
  action_in_parsing_without_pars = this->name->name + "_clock";
  if (!clocks_with_par.empty()) {
    if (clocks_with_par.find(this->name->name) != clocks_with_par.end()) { // durative action has  paramenters
      action_in_parsing += clocks_with_par[this->name->name].statement;
    }
  }
  return this->toMurphi_start(indent) + this->toMurphi_end(indent);
}

// to fix
string proposition::toMurphi(int indent)
{
  string toReturn;
  string ground_statement;
  if (!this->args->isEmpty()) { // if exists some parameters
    if (!predicates_with_par.empty()) { //params are stored in the map structure
      if (make_ground_parameters_now) {
        toReturn = this->head->name;
        vector<parameter_symbol*> temp = this->args->symbol_list();
        vector<parameter_symbol*>::iterator it = temp.begin();
        for (; it != temp.end(); ++it) {
          ground_statement += "[";
          ground_statement += (*it)->name;
          ground_statement += "]";
        }
        toReturn += ground_statement;
      } else {
        vector<parameter_symbol*> temp = this->args->symbol_list();
        vector<parameter_symbol*>::iterator it = temp.begin();
        toReturn += this->head->name;
        for (; it != temp.end(); it++) {
          toReturn += "[" + (*it)->name + "]";
        }

      }
    } else {
      string
      log =
        "pred_term " + this->head->name
        + " has parameters but it is not stored in the object model\n";
      log_error(E_WARNING, log.c_str());
    }
  } else {
    toReturn = this->head->name;
  }
  return toReturn;
}

string assignment::toMurphi(int indent)
{
  string v;
  string ground_statement;
  string murphi_expression;
  string local_operator;
  /* create external function signature */

  if (make_ground_parameters_now) { // instantiates actual param with formal param
    v += this->f_term->func_sym->name;
    vector<parameter_symbol*> temp =
      (this->f_term)->param_list->symbol_list();
    vector<parameter_symbol*>::iterator it = temp.begin();
    for (; it != temp.end(); ++it) {
      ground_statement += "[";
      ground_statement += (*it)->name;
      ground_statement += "]";
    }
    v += ground_statement;
  } else {
    v += this->f_term->toMurphi(indent);
  }
  v += " := ";

  if (this->op == E_ASSIGN) {
    actual_external_effect.c_body += this->expr->toMurphi(-10);
    actual_external_effect.signature = "externfun assign_"
                                       + this->f_term->func_sym->name + "_" + operator_in_parsing
                                       + "(";
    actual_external_effect.signature_c = "double assign_"
                                         + this->f_term->func_sym->name + "_" + operator_in_parsing
                                         + "(";
    actual_external_effect.call_statement = "assign_"
                                            + this->f_term->func_sym->name + "_" + operator_in_parsing
                                            + "(";
    local_operator = ":=";
  } else if (this->op == E_INCREASE) {
    actual_external_effect.c_body += this->expr->toMurphi(-10);
    actual_external_effect.signature = "externfun increase_"
                                       + this->f_term->func_sym->name + "_" + operator_in_parsing
                                       + "(";
    actual_external_effect.signature_c = "double increase_"
                                         + this->f_term->func_sym->name + "_" + operator_in_parsing
                                         + "(";
    actual_external_effect.call_statement = "increase_"
                                            + this->f_term->func_sym->name + "_" + operator_in_parsing
                                            + "(";
    actual_external_effect.args_par.push_back(this->f_term->func_sym->name);
    actual_external_effect.args_par_with_par.push_back(
      this->f_term->toMurphi(0));
    local_operator = "+";
  } else if (this->op == E_DECREASE) {
    actual_external_effect.c_body += this->expr->toMurphi(-10);
    actual_external_effect.signature = "externfun decrease_"
                                       + this->f_term->func_sym->name + "_" + operator_in_parsing
                                       + "(";
    actual_external_effect.signature_c = "double decrease_"
                                         + this->f_term->func_sym->name + "_" + operator_in_parsing
                                         + "(";
    actual_external_effect.call_statement = "decrease_"
                                            + this->f_term->func_sym->name + "_" + operator_in_parsing
                                            + "(";
    actual_external_effect.args_par.push_back(this->f_term->func_sym->name);
    actual_external_effect.args_par_with_par.push_back(
      this->f_term->toMurphi(0));
    local_operator = "-";
  } else {
    log_error(E_WARNING, "assignment statement not yet supported\n");
  }

  //compute murphi and c signature (without parameters)
  this->expr->toMurphi_external_decl(indent);
  this->expr->toMurphi_external_call(indent);
  for (vector<string>::iterator it = actual_external_effect.args_par.begin(); it
       != actual_external_effect.args_par.end(); ++it) {
    actual_external_effect.signature += (*it) + " : real_type ; ";
    actual_external_effect.signature_c += "double " + (*it) + ", ";
  }

  // compute call statement with pars
  for (vector<string>::iterator it =
         actual_external_effect.args_par_with_par.begin(); it
       != actual_external_effect.args_par_with_par.end(); ++it) {
    actual_external_effect.call_statement += (*it) + " , ";
  }

  // insert the name of .h file only for the first external function signature
  if (external_effects.empty()) {
    actual_external_effect.signature += "): real_type \""
                                        + ext_func_filename + "\" ;";
  } else {
    actual_external_effect.signature += "): real_type ;";
  }

  unsigned long pos = 0;
  pos = actual_external_effect.call_statement.find_last_of(",");
  if (pos < actual_external_effect.call_statement.length() )
    actual_external_effect.call_statement
      = actual_external_effect.call_statement.replace(pos, 1, "");
  pos = actual_external_effect.signature_c.find_last_of(",");
  if (pos < actual_external_effect.signature_c.length() )
    actual_external_effect.signature_c
      = actual_external_effect.signature_c.replace(pos, 1, "");
  actual_external_effect.call_statement += ")";
  actual_external_effect.signature_c += ") {\n";
  if (this->op != E_ASSIGN) {
    actual_external_effect.signature_c += "\t return round_k_digits("
                                          + actual_external_effect.args_par[0] + local_operator + "("
                                          + actual_external_effect.c_body + ")," + precision + "); \n}\n";
  } else {
    actual_external_effect.signature_c += "\t return round_k_digits("
                                          + actual_external_effect.c_body + "," + precision + "); \n}\n";
  }

  if (mod_external_func) {
    v += actual_external_effect.call_statement;
    if (ext_func_stream.is_open())
      ext_func_stream << actual_external_effect.signature_c << endl;
    else {
      log_error(E_WARNING, "Error writing .h file\n");
    }
  } else { // reset if mod_external_func is false
    if ((functions_with_par)[this->f_term->func_sym->name].is_not_costant) {
      v += this->expr->toMurphi(indent);
    } else {
      v = "";
    }
    actual_external_effect.signature_c = "";
  }

  // insert element into vector
  if (mod_external_func) {
    if (!external_effects.empty()) { //search for elem signature into vector
      bool found = false;
      for (vector<external_effect_>::iterator it =
             external_effects.begin(); it != external_effects.end()
           & !found; ++it) {
        if ((*it).signature.compare(actual_external_effect.signature)
            == 0) { // element is present into vector
          found = true;
        }
      }
      if (!found)
        external_effects.push_back(actual_external_effect);
    } else {
      external_effects.push_back(actual_external_effect);
    }
  }
  actual_external_effect.call_statement = "";
  actual_external_effect.signature = "";
  actual_external_effect.signature_c = "";
  actual_external_effect.c_body = "";
  actual_external_effect.args_par.clear();
  actual_external_effect.args_par_with_par.clear();

  return v;

}


bool expression::exist(vector<string> v, string s)
{
  vector<string>::iterator it = v.begin();
  for (; it != v.end(); ++it) {
    if ((*it).compare(s) == 0)
      return true;
  }
  return false;
}

string binary_expression::toMurphi_external_decl(int indent)
{
  string left = this->arg1->toMurphi_external_decl(indent);
  string right = this->arg2->toMurphi_external_decl(indent);
  if (left.compare(right) == 0)
    return left;
  return left + right;

}

string binary_expression::toMurphi_external_call(int indent)
{
  string left = this->arg1->toMurphi_external_call(indent);
  string right = this->arg2->toMurphi_external_call(indent);
  if (left.compare(right) == 0)
    return left;
  return left + right;

}

string uminus_expression::toMurphi_external_decl(int indent)
{
  return this->arg1->toMurphi_external_decl(indent);
}

string uminus_expression::toMurphi_external_call(int indent)
{
  return this->arg1->toMurphi_external_call(indent);
}

string num_expression::toMurphi_external_decl(int indent)
{
  return ""; //numerical expression doesn't appears as fuction parameter
}

string num_expression::toMurphi_external_call(int indent)
{
  return ""; //numerical expression doesn't appears as fuction parameter
}

string func_term::toMurphi_external_decl(int indent)
{
  /*if ((functions_with_par)[this->func_sym->name].is_not_costant == true){
   return this->func_sym->name + " : real_type ; ";
   }*/
  string s = this->func_sym->name;
  if (!exist(actual_external_effect.args_par, s))
    actual_external_effect.args_par.push_back(s);
  return s;
}

string func_term::toMurphi_external_call(int indent)
{
  string s = this->toMurphi(0);
  if (!exist(actual_external_effect.args_par_with_par, s))
    actual_external_effect.args_par_with_par.push_back(s);
  return s;
}

string special_val_expr::toMurphi_external_decl(int indent)
{
  string s;
  if (this->var == E_HASHT) {
    s = "T";
    if (!exist(actual_external_effect.args_par, s))
      actual_external_effect.args_par.push_back(s);
    return s;
  } else if (this->var == E_DURATION_VAR) {
    if (!exist(actual_external_effect.args_par,
               action_in_parsing_without_pars))
      actual_external_effect.args_par.push_back(
        action_in_parsing_without_pars);
    return action_in_parsing_without_pars;
  } else {
    string log = "special_val_expr not yet supported\n";
    log_error(E_WARNING, log.c_str());
    return "";
  }
}

string special_val_expr::toMurphi_external_call(int indent)
{
  string s;
  if (this->var == E_HASHT) {
    s = "T";
    if (!exist(actual_external_effect.args_par_with_par, s))
      actual_external_effect.args_par_with_par.push_back(s);
    return s;
  } else if (this->var == E_DURATION_VAR) {
    if (!exist(actual_external_effect.args_par_with_par, action_in_parsing))
      actual_external_effect.args_par_with_par.push_back(
        action_in_parsing);
    return action_in_parsing;
  } else {
    string log = "special_val_expr not yet supported\n";
    log_error(E_WARNING, log.c_str());
    return "";
  }
}

string action::toMurphi(int indent)
{
  unsigned int number_of_ruleset = 0;
  string toReturn;
  operator_in_parsing = "action_" + this->name->name;
  /*	rulset definition */

  vector<var_symbol*> param = this->parameters->symbol_list();
  vector<var_symbol*>::iterator it = param.begin();
  for (; it != param.end(); ++it) {
    toReturn += "ruleset ";
    toReturn += (*it)->name;
    toReturn += ":";
    toReturn += (*it)->type->name;
    toReturn += " do \n ";
    number_of_ruleset++;
  }
  /* end of rulset definition */

  toReturn += "action ";
  toReturn += "rule \" " + this->name->name + " \" \n";
  toReturn += this->precondition->toMurphi(indent) + " ==> \n";
  toReturn += "pddlname: \" ";
  toReturn += get_original(this->name->name);
  toReturn += "\"; \n";

  toReturn += BBEGIN;
  toReturn += "\n";
  toReturn += this->effects->toMurphi(indent + 1);
  for (unsigned int i = 0; i < number_of_ruleset; i++) {
    toReturn += END;
    toReturn += "; ";
  }

  toReturn += END;
  toReturn += ";\n";
  return toReturn;
}

string problem::toMurphi(domain* the_domain)
{
  int indent;
  string toReturn;
  // enable startstate modality
  make_ground_parameters_now = true;
  toReturn += "\n\nstartstate \"start\" \nBEGIN \n";
  toReturn += GLOBAL_CLOCK;
  toReturn += " := 0.0;\n"
              + the_domain->CWA()
              + this->initial_state->toMurphi(indent);
  if (the_domain->ops->exists_durative_action()) {
    // inizialize the clocks (_start and real value) for the initial state for each durative action
    map<string, struct types_and_objects_>::iterator type_iter =
      types_and_objects.begin(); // type iterator
    vector<string>::iterator typed_obj_it; //typed object iterator
    // for each type
    map<string, struct clocks_with_par_>::iterator it =
      clocks_with_par.begin();
    for (; it != clocks_with_par.end(); ++it) {
      toReturn += "\n-- durative action \"";
      toReturn += (*it).second.name;
      toReturn += "\" clock initialization\n ";
      toReturn += (*it).second.FOR_signature;
      //toReturn += (*it).second.indentation;
      toReturn += (*it).second.name;
      toReturn += "_clock_started";
      toReturn += (*it).second.statement;
      toReturn += ":= false;\n";
      toReturn += (*it).second.indentation;
      toReturn += (*it).second.name;
      toReturn += "_clock";
      toReturn += (*it).second.statement;
      toReturn += ":= 0.0;\n";
      for (int i = 0; i < (*it).second.number_of_for; i++)
        toReturn += "END; ";
      toReturn += "-- for ends\n";
    }
    toReturn += "\n";
  }
  toReturn += "all_event_true := true;\n";
  toReturn += "g_n := 0;\n"; // WP WP WP
  toReturn += "h_n := 0;\n"; // WP WP WP
  toReturn += "f_n := 0;\n"; // WP WP WP
  toReturn += "END; -- close startstate\n\n";
  return toReturn;
}

void insert_into_types_vector(struct types_and_objects_* elem, string toInsert)
{
  elem->objects_vector.push_back(toInsert);
  //cout<<"Trovato oggetto:"<<toInsert<<endl;
}

void insert_into_object_vector(struct types_and_objects_ elem_of_type,
                               string position)
{
  types_and_objects[position] = elem_of_type;
  //cout<<"Ho inserito il tipo :"<<types_and_objects[position].type<<" e dentro ha size="<<types_and_objects[position].objects_vector.size()<<endl;
}

string float_expression::toMurphi(int indent)
{
  char buffer[50];

  //string p = "%#." + var_precision + "LG";
  //string p = "%#." + precision + "LG";
  string p = "%#LG";
  sprintf(buffer, p.c_str(), this->double_value());
  string toReturn(buffer);
  return toReturn;
}
string int_expression::toMurphi(int indent)
{
  char buffer[50];

  //string p = "%#." + var_precision + "LG";
  //string p = "%#." + precision + "LG";
  string p = "%#LG";
  sprintf(buffer, p.c_str(), this->double_value());
  string toReturn(buffer);
  return toReturn;
}

string durative_action::toMurphi_end(int indent)
{
  string toReturn = "\n\n";
  operator_in_parsing = "duraction_end_" + this->name->name;
  string FOR_signature;
  string for_indent;
  unsigned int number_of_ruleset = 0;

  /*	rulset definition */

  vector<var_symbol*> param = this->parameters->symbol_list();
  vector<var_symbol*>::iterator it = param.begin();
  for (; it != param.end(); ++it) {
    toReturn += "ruleset ";
    FOR_signature += "for ";
    toReturn += (*it)->name;
    FOR_signature += (*it)->name;
    toReturn += ":";
    FOR_signature += " : ";
    toReturn += (*it)->type->name;
    FOR_signature += (*it)->type->name;
    toReturn += " do \n ";
    FOR_signature += " do \n ";
    number_of_ruleset++;
    for_indent += "  ";
    FOR_signature += for_indent;
  }
  /* end of rulset definition */
  toReturn += "durative_end ";
  toReturn += "rule \" " + this->name->name + "_end \" \n" + "( "
              + this->name->name + "_clock_started";
  if (!clocks_with_par.empty()) {
    if (clocks_with_par.find(this->name->name) != clocks_with_par.end()) { // durative action has  parameters
      toReturn += clocks_with_par[this->name->name].statement;
      clocks_with_par[this->name->name].FOR_signature = FOR_signature;
      clocks_with_par[this->name->name].number_of_for = number_of_ruleset;
      clocks_with_par[this->name->name].indentation = for_indent;
    }

  }
  toReturn += ") & " + this->dur_constraint->toMurphi(-1);

  if (this->precondition->toMurphi(-2).compare("") != 0) {
    toReturn += " & ";
  }

  toReturn += this->precondition->toMurphi(-2);

  toReturn += " & ((" + this->name->name + "_clock" + clocks_with_par[this->name->name].statement + ") " + " > 0.0)"; // WP WP WP

  toReturn += " & all_event_true ==> \n";



  toReturn += "pddlname: \" ";
  toReturn += get_original(this->name->name);
  toReturn += "\"; \n";

  toReturn += BBEGIN;
  toReturn += "\n";
  //toReturn += DYNAMIC ; // no dynamic_check in durative action start rule
  //toReturn += this->name->name+"_clock := "+this->name->name + "_clock + T;\n";
  toReturn += this->name->name + "_clock_started";
  if (!clocks_with_par.empty()) {
    if (clocks_with_par.find(this->name->name) != clocks_with_par.end()) // durative action has  parameters
      toReturn += clocks_with_par[this->name->name].statement;
  }
  // reset durative action clock
  toReturn += ":= false;\n";
  toReturn += this->name->name + "_clock";
  if (!clocks_with_par.empty()) {
    if (clocks_with_par.find(this->name->name) != clocks_with_par.end()) // durative action has  parameters
      toReturn += clocks_with_par[this->name->name].statement;
  }
  toReturn += ":= 0.0;\n";
  toReturn += this->effects->toMurphi(-2);

  for (unsigned int i = 0; i < number_of_ruleset; i++) {
    toReturn += END;
    toReturn += "; ";
  }

  toReturn += END;
  toReturn += "; \n\n";
  return toReturn;
}

string durative_action::toMurphi_start(int indent)
{
  string toReturn = "\n\n";
  operator_in_parsing = "duraction_start_" + this->name->name;
  unsigned int number_of_ruleset = 0;
  vector<var_symbol*> param = this->parameters->symbol_list();
  vector<var_symbol*>::iterator it = param.begin();
  for (; it != param.end(); ++it) {
    toReturn += "ruleset ";
    toReturn += (*it)->name;
    toReturn += ":";
    toReturn += (*it)->type->name;
    toReturn += " do \n ";
    number_of_ruleset++;
  }
  toReturn += "durative_start ";
  toReturn += "rule \" " + this->name->name + "_start \" \n" + "( " + "!"
              + this->name->name + "_clock_started";
  if (!clocks_with_par.empty()) {
    if (clocks_with_par.find(this->name->name) != clocks_with_par.end()) // durative action has  parameters
      toReturn += clocks_with_par[this->name->name].statement;
  }
  toReturn += ")";
  if (this->precondition->toMurphi(-1).compare("") != 0) {
    toReturn += " & ";
  }
  toReturn += this->precondition->toMurphi(-1) + " & all_event_true ==> \n";
  toReturn += "pddlname: \" ";
  toReturn += get_original(this->name->name);
  toReturn += "\"; \n";
  toReturn += BBEGIN;
  toReturn += "\n";
  //toReturn += DYNAMIC ; // no dynamic_check in durative action start rule
  //toReturn += this->name->name+"_clock := "+this->name->name + "_clock + T;\n";
  toReturn += this->name->name + "_clock_started";
  if (!clocks_with_par.empty()) {
    if (clocks_with_par.find(this->name->name) != clocks_with_par.end()) // durative action has  parameters
      toReturn += clocks_with_par[this->name->name].statement;
  }
  toReturn += ":= true;\n";
  toReturn += this->effects->toMurphi(-1);

  for (unsigned int i = 0; i < number_of_ruleset; i++) {
    toReturn += END;
    toReturn += "; ";
  }
  toReturn += END;
  toReturn += "; \n\n";
  return toReturn;
}
string durative_action::toMurphi_process(int indent)
{
  string toReturn = "procedure process_" + this->name->name + "( ";
  operator_in_parsing = "duraction_process_" + this->name->name;
  string parameters;
  string statement;
  vector<var_symbol*> symbol_vector = this->parameters->symbol_list();
  vector<var_symbol*>::iterator it = symbol_vector.begin();
  unsigned int size = 0;
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
  toReturn += ");\n";
  toReturn += BBEGIN;
  toReturn += "\n";
  toReturn += "\t IF (" + this->name->name + "_clock_started";
  toReturn += parameters;
  toReturn += ") THEN \n";
  toReturn += "\t\t " + this->name->name + "_clock";
  toReturn += parameters;
  toReturn += ":= " + this->name->name + "_clock";
  toReturn += parameters;
  toReturn += " + T ;\n";
  toReturn += this->effects->toMurphi(-4);
  toReturn += "\t ENDIF;\n";
  toReturn += END;
  toReturn += ";\n";

  struct clocks_with_par_ elem;
  elem.statement = parameters;
  elem.name = this->name->name;
  clocks_with_par[this->name->name] = elem;

  return toReturn;
}

string pred_decl::toMurphi(int indent)
{
  string toReturn = "\t";
  toReturn += this->head->name + "[pddlname: \"" + get_original(this->head->name) + "\";]";
  toReturn += " : ";
  if (!this->args->isEmpty()) {
    vector<var_symbol*> symbol_vector = this->args->symbol_list();
    vector<var_symbol*>::iterator it = symbol_vector.begin();
    for (; it != symbol_vector.end(); ++it) {
      toReturn += (*it)->toMurphi(indent);
    }
  }
  toReturn += " boolean;\n";
  return toReturn;
}

string pred_decl::toMurphi_get_set(int indent)
{
  string toReturn_set, toSet_set, toReturn_get, toSet_get;
  string statement = this->head->name;
  struct predicates_with_par_ elem;
  toReturn_set += "procedure set_";
  toReturn_get += "function get_";
  toReturn_set += this->head->name;
  toSet_get += "\treturn ";
  toReturn_get += this->head->name;
  toReturn_set += "( ";
  toReturn_get += "( ";
  toSet_set += "\t";
  toSet_get += "\t";
  toSet_set += this->head->name;
  toSet_get += this->head->name;

  // parameters

  if (!this->args->isEmpty()) {
    vector<var_symbol*> symbol_vector = this->args->symbol_list();
    vector<var_symbol*>::iterator it = symbol_vector.begin();
    for (; it != symbol_vector.end(); ++it) {
      toReturn_set += (*it)->toMurphi_get_set(indent);
      toReturn_get += (*it)->toMurphi_get_set(indent);
      statement += "[";
      toSet_set += "[";
      toSet_set += (*it)->name;
      statement += (*it)->name;
      toSet_set += "]";
      statement += "]";
      toSet_get += "[";
      toSet_get += (*it)->name;
      toSet_get += "]";
      toReturn_set += " ; ";
      toReturn_get += " ; ";
    }
    string temp = toReturn_get;
    toReturn_get = temp.substr(0, temp.length() - 3);
  } else {
    string temp = toReturn_get;
    toReturn_get = temp.substr(0, temp.length() - 1);
  }
  // insert element into predicates map
  elem.statement = statement;
  elem.name = this->head->name;
  predicates_with_par[this->head->name] = elem;

  toSet_set += " := value;\n";
  toSet_get += ";\n";
  toReturn_set += " value : boolean);\n";
  toReturn_get += "): boolean;\n";
  toReturn_set += "BEGIN\n";
  toReturn_set += toSet_set;
  toReturn_set += "END;\n\n";
  toReturn_get += "BEGIN\n";
  toReturn_get += toSet_get;
  toReturn_get += "END;\n\n";

  return toReturn_set + toReturn_get;
}
// to fix
string func_term::toMurphi(int indent)
{
  string toReturn;
  if (!this->param_list->empty() && indent != -10) { // if exists some parameters
    if (!functions_with_par.empty()) { //params are stored in the map structure
      if (functions_with_par[this->func_sym->name].is_not_costant) { // the func term is present and it was declared as variable
        //toReturn = functions_with_par[this->func_sym->name].statement;
        toReturn = this->func_sym->name;
        vector<parameter_symbol*> temp =
          this->param_list->symbol_list();
        vector<parameter_symbol*>::iterator it = temp.begin();
        for (; it != temp.end(); ++it) {
          toReturn += "[" + (*it)->name + "]";
        }
      } else {// the func term is present and it was declared as costant
        toReturn = this->func_sym->name;
      }
    } else {
      string
      log =
        "func_term " + this->func_sym->name
        + " has parameters but it is not stored in the object model\n";
      log_error(E_WARNING, log.c_str());
    }
  } else {
    toReturn = this->func_sym->name;
  }
  return toReturn;
}

string func_decl_list::create_function_map(int indent)
{
  func_decl* p;
  string toReturn;
  string function_declaration;
  func_decl* buffer;
  pc_list<func_decl*>::iterator it = _Base::begin();
  map<string, struct functions_with_par_> temp;
  struct functions_with_par_ elem;
  for (; it != _Base::end(); it++) {
    buffer = *it;
    p = dynamic_cast<func_decl*> (buffer);
    if (p != 0) {
      toReturn += p->toMurphi(indent);
      elem.name = (p)->getFunction()->name;
      elem.declaration = p->toMurphi(indent);
      elem.statement = p->toMurphi_statement(indent);
      elem.type_vector = p->args->symbol_list();
      if (!p->args->isEmpty())
        elem.has_parameters = true;
      else
        elem.has_parameters = false;

      functions_with_par[(p)->getFunction()->name] = elem;
      //cout<<"toMurphi statement:"<<elem.statement;
      //cout<<"Ora la size e':"<<functions_with_par.size()<<endl;
    }
  }

  return toReturn;
}

bool analysis::check_assignment_effect_for_const_decl(string name)
{
  // for each function name with no pars, verify if exists an effect statement. Otherwise it can be considered as constant
  bool fterm_found = false;
  // to check if exist a TIL/TIF which uses that symbol
  list<timed_effect*>::iterator ait = this->the_problem->initial_state->timed_effects.begin();
  for(; ait != this->the_problem->initial_state->timed_effects.end(); ait++) {
    timed_initial_literal* til = dynamic_cast<timed_initial_literal*>((*ait));
    if (til != 0 && til->ts == E_AT && til->contains_assignment_to_fterm(name)) {
      fterm_found = true;

    }

  }

  list<operator_*>::iterator ja = this->the_domain->ops->begin();
  for (; ja != this->the_domain->ops->end() && !fterm_found; ja++) {  //for each operator
    list<timed_effect*>::iterator tef = (*ja)->effects->timed_effects.begin();
    for(; tef != (*ja)->effects->timed_effects.end() && !fterm_found; tef++) { //for each timed_effect
      list<assignment*>::iterator jefk = (*tef)->effs->assign_effects.begin();
      for(; jefk != (*tef)->effs->assign_effects.end() && !fterm_found; jefk++) { //scan the assignment eff
        if ((*jefk)->get_func_name().compare(name) == 0) {
          fterm_found = true;
        }
      }
    }

    list<assignment*>::iterator jef = (*ja)->effects->assign_effects.begin();
    for(; jef != (*ja)->effects->assign_effects.end() && !fterm_found; jef++) { //scan the assignment effects
      if ((*jef)->get_func_name().compare(name) == 0) {
        fterm_found = true;
      }
    }

  }
  return fterm_found;
}

string analysis::toMurphi_declaration(int indent)
{
  string declarations;
  // compute mantissa and exponent
  // new formula by Giuseppe Della Penna for fixing "out of bound" exceptions!
  int prec = atoi(precision.c_str());
  int exponent_c = atoi(mantissa.c_str()) - 1; 
  int mantissa_c = atoi(exponent.c_str()) + atoi(mantissa.c_str());
  char buffer [10];
  sprintf (buffer, "%d",mantissa_c);
  mantissa=buffer;
  sprintf (buffer, "%d",exponent_c);
  exponent=buffer;
  string type_decl = "type\n\t real_type: real(" + mantissa + "," + exponent
                     + ");\n\t" +
 		      "integer: -1000..1000;\n\n"; // WP WP WP
  // the mantissa and exponent for clock_type depends on the time_quantum used
  string mantissa_clock = mantissa;
  string exponent_clock = exponent;
  unsigned long pos = time_quantum.find(".");
  if (pos != string::npos) {
    string n_digits = time_quantum.substr(pos+1);
    if (n_digits.length() > 0) {
      exponent_clock = my_to_string(n_digits.length()+1);
      int t_v = (int) n_digits.length() + 6;
      mantissa_clock = my_to_string(t_v);
    }
  }

  type_decl += "\t ";
  type_decl += GLOBAL_CLOCK_TYPE;
  type_decl += ": real(" + mantissa_clock + "," + exponent_clock+ ");\n\n";

  string const_decl;
  string ext_fun;
  // extfun decl
  ext_fun = "[[EXT FUN TO REPLACE]]";
  bool not_to_all = false;
  if (this->the_problem == NULL) {
    log_error(E_EXIT, " No problem found!");
    cout << " Error: No problem found!" << endl << flush;
    exit(-1);
  }
  if (this->the_problem->objects != NULL)
    type_decl += this->the_problem->objects->toMurphi(indent);
  const_decl += "const \n\t T:" + time_quantum + ";\n\n";
  declarations += "var \n\t";
  //if (this->the_domain->ops->exists_durative_action())
  declarations += "all_event_true: boolean;\n";
  declarations += "\t "; // WP WP WP
  declarations += "h_n: integer;\n"; // WP WP WP
  declarations += "\t "; // WP WP WP
  declarations += "g_n: integer;\n"; // WP WP WP
  declarations += "\t "; // WP WP WP
  declarations += "f_n: integer;\n"; // WP WP WP
  declarations += "\t ";
  declarations += GLOBAL_CLOCK;
  declarations += "[pddlname:\"upmurphi_global_clock\";]:";
  declarations += GLOBAL_CLOCK_TYPE;
  declarations += ";\n";


  string toReturn;

  vector<symbol*> symbol_vector;
  vector<symbol*>::iterator i;

  // predicates definition
  toReturn += this->the_domain->predicates->toMurphi(indent);
  // extfun declaration
  toReturn += ext_fun;
  // predicates get/set methods definition
  toReturn += this->the_domain->predicates->toMurphi_get_set(indent);

  // functions declarations
  this->the_domain->functions->create_function_map(indent); // NB: create function map
  symbol_vector = this->func_tab.symbol_list();
  i = symbol_vector.begin();

  for (; i != symbol_vector.end(); i++) {
    string ris;
    if (state_creation_assistent
        && !functions_with_par[(*i)->name].has_parameters
        && !not_to_all) { // only for functions without parameters

      // for each function name with no pars, verify if exists an effect statement. Otherwise it can be considered as constant
      bool fterm_found = this->check_assignment_effect_for_const_decl((*i)->name);
      if (!fterm_found) {
        //cout<<" Note: ** "<<(*i)->name<<" is considered as constant "<<endl;
        string temp;
        //const_decl += (*i)->toMurphi_decl(0)+"\n\t";
        temp
          = current_analysis->the_problem->initial_state->get_toMurphi_by_name(
              (*i)->name);
        if (temp.find("=", 1) != string::npos)
          temp = temp.replace(temp.find("="), 1, "");
        const_decl += "\t" + temp;
        (functions_with_par)[(*i)->name].is_not_costant = false;

        // define PDDL fuction as state variable
        /*
        if (ris.compare("A") == 0 || ris.compare("a") == 0) {
            not_to_all = true;
        }
         */


        /*
         // old version that requires user intervention

        cout << "Do you want to use function " << (*i)->name
        << " as a constant value? Y/N or A for \" no to all  \" \n";
        cin >> ris;
        if ((ris.compare("Y") == 0) || (ris.compare("y") == 0)) {
            string temp;
            //const_decl += (*i)->toMurphi_decl(0)+"\n\t";
            temp
            = current_analysis->the_problem->initial_state->get_toMurphi_by_name(
                                                                                 (*i)->name);
            if (temp.find("=", 1) != string::npos)
                temp = temp.replace(temp.find("="), 1, "");
            const_decl += "\t" + temp;
            (functions_with_par)[(*i)->name].is_not_costant = false;
        } else { // define PDDL fuction as state variable
            if (ris.compare("A") == 0 || ris.compare("a") == 0) {
                not_to_all = true;
            }
            declarations += ((functions_with_par)[(*i)->name]).declaration;
            (functions_with_par)[(*i)->name].is_not_costant = true;
        }
         */
      } else {
        //not_to_all = true;
        declarations += ((functions_with_par)[(*i)->name]).declaration;
        (functions_with_par)[(*i)->name].is_not_costant = true;
      }

    } else { // no parameters
      declarations += ((functions_with_par)[(*i)->name]).declaration;
      (functions_with_par)[(*i)->name].is_not_costant = true;
    }
    toReturn += (*i)->toMurphi_get_set(0) + "\n";
  }

  // clocks declarations
  mod_external_func = true;
  there_are_durative_action = this->the_domain->ops->exists_durative_action();
  declarations += this->the_domain->ops->toMurphi_clocks_declarations(indent);
  declarations += "\n\n" + toReturn;
  declarations += this->the_domain->ops->toMurphi_process(indent);
  declarations += this->the_domain->ops->toMurphi_event_failure(indent);
    this->the_domain->ops->check_DA_duration_runtime();

  return type_decl + "\n" + const_decl + "\n" + declarations;
}

string pddl_typed_symbol::toMurphi(int indent)
{
  if (this->type != NULL)
    return this->type->toMurphi(indent);
  else
    log_error(E_EXIT, " All objects must be typed \n");
  return "";
}

string pddl_typed_symbol::toMurphi_get_set(int ind)
{
  if (this->type != NULL)
    return this->name + " : " + this->type->name;
  else
    log_error(E_EXIT, this->name + ": All objects must be typed \n");
  return "";
}
;

string effect_lists::toMurphi_add_effect(int indent)
{
  string toReturn;
  pc_list<simple_effect*>::iterator it = this->add_effects.begin();
  for (; it != this->add_effects.end(); it++) {
    toReturn += (*it)->toMurphi(indent);
    toReturn += ":= true; \n";
  }
  return toReturn;
}

string effect_lists::toMurphi_del_effect(int indent)
{
  string toReturn;
  pc_list<simple_effect*>::iterator it = this->del_effects.begin();
  for (; it != this->del_effects.end(); it++) {
    toReturn += (*it)->toMurphi(indent);
    toReturn += ":= false; \n";
  }
  return toReturn;
}

string effect_lists::toMurphi_forall_effect(int indent)
{
  string toReturn;
  pc_list<forall_effect*>::iterator it = this->forall_effects.begin();
  for (; it != this->forall_effects.end(); it++) {
    toReturn += (*it)->toMurphi(indent) + "\n";
  }
  return toReturn;
}

string effect_lists::toMurphi_cond_effect(int indent)
{
  string toReturn;
  pc_list<cond_effect*>::iterator it = this->cond_effects.begin();
  for (; it != this->cond_effects.end(); it++) {
    toReturn += (*it)->toMurphi(indent) + "\n";
  }
  return toReturn;
}

string effect_lists::toMurphi_assign_effect(int indent)
{
  string toReturn;
  string temp;
  pc_list<assignment*>::iterator it = this->assign_effects.begin();
  for (; it != this->assign_effects.end(); it++) {
    if (goal_printing) {
      //if ((*it)->present_in_startstate)
      temp = (*it)->toMurphi(indent);
    } else {
      temp = (*it)->toMurphi(indent);
    }
    if (temp.empty())
      toReturn += temp;
    else
      toReturn += temp + ";\n";
  }
  return toReturn;
}

string effect_lists::toMurphi_timed_effect(int indent)
{
  string toReturn;
  //  E_AT_START, E_AT_END, E_OVER_ALL, E_CONTINUOUS, E_AT

  pc_list<timed_effect*>::iterator it = this->timed_effects.begin();
  for (; it != this->timed_effects.end(); it++) {
    if ((indent == -1) && ((*it)->ts == E_AT_START)) { // at_start
      toReturn += (*it)->toMurphi(indent);
    } else if ((indent == -2) && ((*it)->ts == E_AT_END)) { // at_end
      toReturn += (*it)->toMurphi(indent);
    } else if ((indent == -3) && ((*it)->ts == E_OVER_ALL)) { // overall
      toReturn += (*it)->toMurphi(indent);
    } else if ((indent == -4) && ((*it)->ts == E_CONTINUOUS)) { // continuous
      toReturn += (*it)->toMurphi(indent);
    } else if ((indent == -5) && ((*it)->ts == E_AT)) { // at
      //log_error(E_WARNING, "TIL and TIF still sperimental \n");
      toReturn += (*it)->toMurphi(indent);
    } else {
      //toReturn += (*it)->toMurphi(indent);
    }
  }
  return toReturn;
}

string effect_lists::get_toMurphi_by_name(string name)
{
  pc_list<assignment*>::iterator it = this->assign_effects.begin();
  string elem_name;
  string toReturn;
  for (; it != this->assign_effects.end(); it++) {
    elem_name = (*it)->get_func_name();
    if (elem_name.compare(name) == 0) {
      (*it)->present_in_startstate = false;
      //return elem_name + ";\n";
      toReturn = (*it)->toMurphi(0);
      return toReturn + ";\n";
    }
  }
  string error = "No match found for assignment_effect named " + name + "\n;";
  log_error(E_EXIT, error);
  return toReturn;
}

void effect_lists::append_effects(effect_lists* from)
{
  // Splice lists in 'from' into lists in 'this'
  add_effects.splice(add_effects.begin(), from->add_effects);
  del_effects.splice(del_effects.begin(), from->del_effects);
  forall_effects.splice(forall_effects.begin(), from->forall_effects);
  cond_effects.splice(cond_effects.begin(), from->cond_effects);
  assign_effects.splice(assign_effects.begin(), from->assign_effects);
  timed_effects.splice(timed_effects.begin(), from->timed_effects);
}

string problem::toMurphi_ext_fun(domain* the_domain, string to_replace)
{
  string ext_fun = "\n\n-- External function declaration \n\n";
  string toReturn;
  ext_fun += "externfun ext_assignment(value : real_type) : real_type;\n";
  vector<external_effect_>::iterator it = external_effects.begin();
  for (; it != external_effects.end(); ++it) {
    ext_fun += (*it).signature + "\n";
  }

  size_t pos = to_replace.find("[[EXT FUN TO REPLACE]]");
  if (pos == string::npos) {
    cout << "ERROR in toMurphi_ext_fun\n" << endl << flush;
    exit(-1);
  } else {
    toReturn = to_replace.replace(pos, 22, ext_fun);
  }

  return toReturn + "\n\n";
}


string domain::toMurphi_TIL_TIF(int indent, problem* problem)
{
  string toReturn;
  toReturn += problem->initial_state->toMurphi_timed_effect(-5);
  return toReturn;
}

string domain::CWA()   // implements the Closed World Assumption
{
  string toReturn, statement, indent;
  unsigned int for_counter;
  vector<pred_decl*> pred = this->predicates->symbol_list();
  vector<pred_decl*>::iterator it = pred.begin();
  for (; it != pred.end(); ++it) { // for each predicates
    vector<var_symbol*> param = (*it)->args->symbol_list();
    toReturn += this->visit(param, &for_counter, statement, indent, ","); //get parameters
    if (!param.empty()) {
      toReturn += indent + "set_" + (*it)->head->name + "(" + statement
                  + ", false);\n";
      for (unsigned int i = 0; i < for_counter; i++)
        toReturn += "END; ";
      toReturn += " -- close for";
    } else {
      toReturn += indent + "set_" + (*it)->head->name + "(false);\n";
    }
    toReturn += "\n";
    statement = "";
    indent = "   ";
  }
  vector<func_decl*> func = this->functions->symbol_list();
  vector<func_decl*>::iterator it2 = func.begin();
  for (; it2 != func.end(); ++it2) { // for each fuction
    if ((functions_with_par)[(*it2)->head->name].is_not_costant) {
      vector<var_symbol*> param = (*it2)->args->symbol_list();
      toReturn += this->visit(param, &for_counter, statement, indent,
                              "]["); //get parameters
      if (!param.empty()) {
        toReturn += indent + (*it2)->head->name + "[" + statement
                    + "] := 0.0 ;\n";
        for (unsigned int i = 0; i < for_counter; i++)
          toReturn += "END; ";
        toReturn += " -- close for";
      } else {
        toReturn += indent + (*it2)->head->name + " := 0.0 ;\n";
      }
    }
    toReturn += "\n";
    statement = "";
    indent = "   ";
  }
  return toReturn;
}

}
;

void indent(int ind)
{
  cout << '\n';
  for (int i = 0; i < ind; i++)
    cout << "   ";
}

namespace PDDL2UPMurphi_parser
{
/*---------------------------------------------------------------------------
 Functions called from parser that perform checks, and possibly generate
 errors.
 --------------------------------------------------------------------------*/

void requires(pddl_req_flag flags)
{
  if (!(flags & current_analysis->req)) {
    if (flags == E_TYPING) {
      current_analysis->error_list.add(E_EXIT, "PDDL Objects have to be typed for being correctly parsed: requirement "
                                       + pddl_req_flags_string(flags));
      domain_is_untyped = true;

    } else {
      current_analysis->error_list.add(E_WARNING, "Undeclared requirement "
                                       + pddl_req_flags_string(flags));
    }
  }

}

void log_error(error_severity sev, const string& description)
{
  if (sev == E_EXIT) {
    cout<<" *** FATAL ERROR *** "<<endl;
    cout<<description<<endl;
    exit(-1);
  }
  current_analysis->error_list.add(sev, description);
}


void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
  if(from.empty())
    return;
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
};

/*---------------------------------------------------------------------------
 Descriptions of requirements flags.
 Surely some way to avoid saying this again.
 -----------------------------------------------------------------------------*/

string pddl_req_flags_string(pddl_req_flag flags)
{
  string result;

  if (flags & E_EQUALITY)
    result += ":equality ";
  if (flags & E_STRIPS)
    result += ":strips ";
  if (flags & E_TYPING)
    result += ":typing ";
  if (flags & E_DISJUNCTIVE_PRECONDS)
    result += ":disjunctive-preconditions ";
  if (flags & E_EXT_PRECS)
    result += ":existential-preconditions ";
  if (flags & E_UNIV_PRECS)
    result += ":universal-preconditions ";
  if (flags & E_COND_EFFS)
    result += ":conditional-effects ";
  if (flags & E_FLUENTS)
    result += ":fluents ";
  if (flags & E_DURATIVE_ACTIONS)
    result += ":durative-actions ";
  if (flags & E_DURATION_INEQUALITIES)
    result += ":duration-inequalities ";
  if (flags & E_CONTINUOUS_EFFECTS)
    result += ":continuous-effects ";
  if (flags & E_NEGATIVE_PRECONDITIONS)
    result += ":negative-preconditions ";
  if (flags & E_DERIVED_PREDICATES)
    result += ":derived-predicates ";
  if (flags & E_TIMED_INITIAL_LITERALS)
    result += ":timed-initial-literals ";
  if (flags & E_PREFERENCES)
    result += ":preferences ";
  if (flags & E_CONSTRAINTS)
    result += ":constraints ";
  if (flags & E_TIME)
    result += ":time ";
  return result;
}

// Search tables from top of stack to find symbol matching name.
// If none found, add to top table.
// Return pointer to symbol.
var_symbol* var_symbol_table_stack::symbol_get(const string& name)
{
  var_symbol* sym = NULL;

  // Iterate through stack from top to bottom
  // (may need to change direction if changing underlying
  // impl. of stack)
  for (iterator i = begin(); i != end() && sym == NULL; ++i)
    sym = (*i)->symbol_probe(name);

  if (sym != NULL)
    // return found symbol
    return sym;
  else {
    // Log a warning
    // add new symbol to current table.
    log_error(E_WARNING, "Undeclared variable symbol: ?" + name);
    return top()->symbol_put(name);
  }
}
;




var_symbol* var_symbol_table_stack::symbol_put(const string& name)
{
  return top()->symbol_put(name);
}
;

}
;

