/*
 * upm_numeric_rpg.cpp
 *
 *  Created on: 2 Mar 2015
 *      Author: k1328088
 */

#include "upm_numeric_rpg.hpp"
//#include <std>
#include <string>
#include <set>
#include <list>
#include <iterator>
#include <vector>
#include <map>
#include <ctime>
#include <time.h>


upm_numeric_rpg::upm_numeric_rpg() {
	// TODO Auto-generated constructor stub
	init_facts = std::set<mu__any*>();
//	rpg_actions = std::vector<std::set<int> >();
	bool_goal_fact_layer = get_bool_goal_conditions();
	num_goal_fact_layer = get_numeric_goal_conditions();
	rpg_state_facts = std::vector<std::pair<state*, state*> >();

	achieved_by = std::vector<std::map<mu__any*, std::set<int> > >();
	achieved_by_first = std::vector<std::map<mu__any*, int > >();
	action_pre = std::vector<std::map<int, std::vector<mu__any*> > >();


	all_goal_fact_layer = std::set<mu__any*>();
	std::set<mu_0_boolean*>::iterator ita;

	for (ita=bool_goal_fact_layer.begin(); ita!=bool_goal_fact_layer.end(); ++ita){
		all_goal_fact_layer.insert(*ita);
	}

	std::map<mu_1_real_type*, std::pair<double, int> >::iterator itaa;

	for (itaa=num_goal_fact_layer.begin(); itaa!=num_goal_fact_layer.end(); ++itaa){
		all_goal_fact_layer.insert(itaa->first);
		goal_bounds[itaa->first] = (std::make_pair(0, 0));
	}




	bounds = get_initial_bounds();

	for( int i = 0; i < RULES_IN_WORLD; i++ )
	    remaining_rules.push_back(i);
}



void upm_numeric_rpg::update_lower_bound(std::map<mu_1_real_type*, std::pair<double, double> > &bs){

	std::vector<mu_1_real_type*> v_temp = workingstate->get_mu_nums();
	std::vector<mu_1_real_type*> v_tempp = workingstate->get_mu_num_arrays();

	for(int i=0; i<v_temp.size();i++){
		if (bs.count(v_temp.at(i)) > 0){
			bs[v_temp.at(i)].first = v_temp.at(i)->value();

//			cout << v_temp.at(i)->name << endl;
//			cout << v_temp.at(i)->value() << endl;

		}
	}

	for(int i=0; i<v_tempp.size();i++){
		if (bs.count(v_tempp.at(i)) > 0){
			bs[v_tempp.at(i)].first = v_tempp.at(i)->value();

//			cout << v_tempp.at(i)->name << endl;
//			cout << v_tempp.at(i)->value() << endl;
		}
	}
//	std::cout << " UPDATING LOWER BOUND " << std::endl;
}

void upm_numeric_rpg::update_upper_bound(std::map<mu_1_real_type*, std::pair<double, double> > &bs){

	std::vector<mu_1_real_type*> v_temp = workingstate->get_mu_nums();
	std::vector<mu_1_real_type*> v_tempp = workingstate->get_mu_num_arrays();

	for(int i=0; i<v_temp.size();i++){
		if (bs.count(v_temp.at(i)) > 0){
			bs[v_temp.at(i)].second = v_temp.at(i)->value();

//			cout << v_temp.at(i)->name << endl;
//			cout << v_temp.at(i)->value() << endl;
		}
	}

	for(int i=0; i<v_tempp.size();i++){
		if (bs.count(v_tempp.at(i)) > 0){
			bs[v_tempp.at(i)].second = v_tempp.at(i)->value();

//			cout << v_tempp.at(i)->name << endl;
//			cout << v_tempp.at(i)->value() << endl;
		}
	}
}




bool upm_numeric_rpg::metric_condition(int r){

	std::map<mu_1_real_type*, std::pair<double, int> > num_conds = Rules->generator->num_precond_array(r);
	std::vector<mu_0_boolean*> bool_conds = Rules->generator->bool_precond_array(r);


	for (int i=0; i < bool_conds.size(); i++){
		if (bool_conds.at(i)->value() == 0){
//			cout << "\n\nCONDITION INAPPLICABLE!!!!!!!!!!!!!!!!!" << endl;
//			cout << Rules->RuleName(r) << endl;
//			bool_preconds.at(i)->print();
//			cout << "\n\n\n\n" << endl;
			return false;
		}
//		cout << ittt->first->name << endl;
//		cout << ittt->second.first << endl;
//		cout << ittt->second.second << endl;
	}


	std::map<mu_1_real_type*, std::pair<double, int> >::iterator ittt;
	for (ittt=num_conds.begin(); ittt!=num_conds.end(); ++ittt){

		if (ittt->second.second == 0){
			if (ittt->second.first > bounds[ittt->first].second || ittt->second.first < bounds[ittt->first].first) {
//				cout << "\n\nEQ CONDITION INAPPLICABLE!!!!!!!!!!!!!!!!!" << endl;
//				cout << Rules->RuleName(r) << endl;
//				cout << ittt->first->name << endl;
//				cout << ittt->second.first << endl;
//				cout << bounds[ittt->first].first << endl;
//				cout << bounds[ittt->first].second << endl;
//				cout << "\n\n\n\n" << endl;
				return false;
			}
		}
		else if (ittt->second.second == 1){
			if (bounds[ittt->first].second < ittt->second.first) {
//				cout << "\n\nGTE CONDITION INAPPLICABLE!!!!!!!!!!!!!!!!!" << endl;
//				cout << Rules->RuleName(r) << endl;
//				cout << ittt->first->name << endl;
//				cout << ittt->second.first << endl;
//				cout << bounds[ittt->first].first << endl;
//				cout << bounds[ittt->first].second << endl;
//				cout << "\n\n\n\n" << endl;
				return false;
			}
		}
		else if (ittt->second.second == 2){
			if (bounds[ittt->first].first > ittt->second.first) {
//				cout << "\n\nLTE CONDITION INAPPLICABLE!!!!!!!!!!!!!!!!!" << endl;
//				cout << Rules->RuleName(r) << endl;
//				cout << ittt->first->name << endl;
//				cout << ittt->second.first << endl;
//				cout << bounds[ittt->first].first << endl;
//				cout << bounds[ittt->first].second << endl;
//				cout << "\n\n\n\n" << endl;
				return false;
			}
		}
		else if (ittt->second.second == 3){
			if (bounds[ittt->first].second <= ittt->second.first) {
//				cout << "\n\nGT CONDITION INAPPLICABLE!!!!!!!!!!!!!!!!!" << endl;
//				cout << (bounds[ittt->first].second) << " <= " << (ittt->second.first) << endl;
//				cout << Rules->RuleName(r) << endl;
//				cout << ittt->first->name << endl;
//				cout << ittt->second.first << endl;
//				cout << bounds[ittt->first].first << endl;
//				cout << bounds[ittt->first].second << endl;
//				cout << "\n\n\n\n" << endl;
				return false;
			}

		}
		else if (ittt->second.second == 4){
			if (bounds[ittt->first].first >= ittt->second.first) {
//				cout << "\n\nLT CONDITION INAPPLICABLE!!!!!!!!!!!!!!!!!" << endl;
//				cout << Rules->RuleName(r) << endl;
//				cout << ittt->first->name << endl;
//				cout << ittt->second.first << endl;
//				cout << bounds[ittt->first].first << endl;
//				cout << bounds[ittt->first].second << endl;
//				cout << "\n\n\n\n" << endl;
				return false;
			}
		}


//		cout << ittt->first->name << endl;
//		cout << ittt->second.first << endl;
//		cout << ittt->second.second << endl;
	}




//	cout << "\n\n							!!CONDITION APPLICABLE!!!!!!!!!!!!!!!!!" << endl;
//	cout << "								" << Rules->RuleName(r) << "\n\n" << endl;

	return true;
}


bool upm_numeric_rpg::goal_check(){

	std::map<mu_1_real_type*, std::pair<double, int> > num_conds = num_goal_fact_layer;
	std::set<mu_0_boolean*> bool_conds = bool_goal_fact_layer;

	std::set<mu_0_boolean*>::iterator bit;
	for (bit=bool_conds.begin(); bit != bool_conds.end(); ++bit){
		if ((*bit)->value() == 0){
			return false;
		}
	}


	std::map<mu_1_real_type*, std::pair<double, int> >::iterator ittt;
	for (ittt=num_conds.begin(); ittt!=num_conds.end(); ++ittt){

		if (ittt->second.second == 0){
			if (ittt->second.first > goal_bounds[ittt->first].second || ittt->second.first < goal_bounds[ittt->first].first) {
				return false;
			}
		}
		else if (ittt->second.second == 1){
			if (goal_bounds[ittt->first].second < ittt->second.first) {
				return false;
			}
		}
		else if (ittt->second.second == 2){
			if (goal_bounds[ittt->first].first > ittt->second.first) {
				return false;
			}
		}
		else if (ittt->second.second == 3){
			if (goal_bounds[ittt->first].second <= ittt->second.first) {
				return false;
			}
		}
		else if (ittt->second.second == 4){
			if (goal_bounds[ittt->first].first >= ittt->second.first) {
				return false;
			}
		}
	}



	return true;
}




std::set<mu__any*> upm_numeric_rpg::get_fact_layer(){

	std::set<mu__any*> f_list;

	std::vector<mu_0_boolean*> v_temp = workingstate->get_mu_bools();
	std::vector<mu_0_boolean*> v_tempp = workingstate->get_mu_bool_arrays();
	v_temp.insert(v_temp.end(), v_tempp.begin(), v_tempp.end());

	for (int ix2 = 0; ix2 < (v_temp.size()); ix2++){
				if (v_temp.at(ix2)->value() == true){
					f_list.insert(v_temp.at(ix2));
				}
	}

	return (f_list);
}

std::set<mu__any*> upm_numeric_rpg::get_fact_layer(state* st){

	std::set<mu__any*> f_list;

	state* temps = new state(workingstate);

	StateCopy(workingstate, st);

//	workingstate->print();

	std::vector<mu_0_boolean*> v_temp = st->get_mu_bools();
	std::vector<mu_0_boolean*> v_tempp = st->get_mu_bool_arrays();
	v_temp.insert(v_temp.end(), v_tempp.begin(), v_tempp.end());

	for (int ix2 = 0; ix2 < (v_temp.size()); ix2++){
//		cout << v_temp.at(ix2)->Name() << endl;
				if (v_temp.at(ix2)->value() == 1){
					f_list.insert(v_temp.at(ix2));
				}
	}

	StateCopy(workingstate, temps);

	return (f_list);
}




std::map<mu_1_real_type*, std::pair<double, double> > upm_numeric_rpg::get_initial_bounds(){

	std::map<mu_1_real_type*, std::pair<double, double> > bs;

	static state* temp_numr = new state;
	StateCopy(temp_numr, workingstate);

	std::vector<mu_1_real_type*> v_temp = workingstate->get_mu_nums();
	std::vector<mu_1_real_type*> v_tempp = workingstate->get_mu_num_arrays();
	v_temp.insert(v_temp.end(), v_tempp.begin(), v_tempp.end());

	for (int ix2 = 0; ix2 < (v_temp.size()); ix2++){
		bs[v_temp.at(ix2)] = (std::make_pair(v_temp.at(ix2)->value(), v_temp.at(ix2)->value()));

		cout << "BOUNDS for: " << v_temp.at(ix2)->name << "  =  ( " <<v_temp.at(ix2)->value() << " , " << v_temp.at(ix2)->value() << " )" << endl;


	}

	StateCopy(workingstate, temp_numr);

	return bs;
}




double upm_numeric_rpg::compute_rpg(){

	std::set<int> ha;

//	cout << "\n\n\n\n888888888888888888888888888888888888888888888888888888888888888888888888888888888\n\n NEW RPG!!!!!\n\n" << endl;

//	std::map<mu_1_real_type*, std::pair<double, int> >::iterator ittt;
//	for (ittt=num_goal_fact_layer.begin(); ittt!=num_goal_fact_layer.end(); ++ittt){
//		cout << ittt->first->name << endl;
//		cout << ittt->first->value() << endl;
//		cout << ittt->second.first << endl;
//	}

//	cout << "CURRENT STATE: \n" << endl;
//	workingstate->print();
//	cout << "\n\n\n" << endl;


//	clock_t start_total = clock();

	action_pre.clear();
	achieved_by.clear();
	achieved_by_first.clear();
	rpg_state_facts.clear();
	rpg_facts.clear();
	init_facts.clear();

	if (bool_goal_fact_layer.size() == 0 && num_goal_fact_layer.size() == 0) return 999999;

	double rpg_length = 999999;

	static state* orig = new state;
	StateCopy(orig, workingstate);

	state* temp_ws = new state(workingstate);
	state* backup_state_lower = new state();
	state* backup_state_upper = new state();

	state* next_rpg_state_lower = new state(workingstate);

	StateCopy(backup_state_lower, next_rpg_state_lower);

	state* next_rpg_state_upper = new state(workingstate);

	StateCopy(backup_state_upper, next_rpg_state_upper);

	state* trpg0 = new state(workingstate);

	rpg_state_facts.push_back(std::make_pair(temp_ws, temp_ws));

	init_facts = get_fact_layer(temp_ws);

	std::set<mu_0_boolean*> f_l;


	while (true){

		std::map<mu__any*, std::set<int> > curr_ach_by;
		std::map<int, std::vector<mu__any*> > curr_a_pre;
		std::map<mu__any*, int> curr_ach_by_first;


		achieved_by.push_back(curr_ach_by);
		action_pre.push_back(curr_a_pre);
		achieved_by_first.push_back(curr_ach_by_first);


		StateCopy(backup_state_lower, rpg_state_facts.at(rpg_state_facts.size()-1).first);
		StateCopy(backup_state_upper, rpg_state_facts.at(rpg_state_facts.size()-1).second);

		for (int ix = 0; ix < RULES_IN_WORLD; ix++){

			StateCopy(workingstate, backup_state_lower);

			if (metric_condition(ix) == true){

				action_pre.at(action_pre.size()-1).insert(std::make_pair(ix, Rules->generator->all_precond_array(ix)));

				std::vector<mu__any*> aef = Rules->generator->effects_all_array(ix);

				for (int ix44 = 0; ix44 < aef.size(); ix44++){

					if (init_facts.count(aef.at(ix44))==0 && ix != 0){

//						if ((achieved_by_first.size() > 1) && (achieved_by_first.at(achieved_by_first.size()-2).count(aef.at(ix44)) > 0)
//								&& (aef.at(ix44)->get_mu_type() == "mu_0_boolean") ){
//							achieved_by_first.at(achieved_by_first.size()-1).insert(std::make_pair(aef.at(ix44), 0));
//						}
//						else {
//							achieved_by_first.at(achieved_by_first.size()-1).insert(std::make_pair(aef.at(ix44), ix));
//						}

						achieved_by_first.at(achieved_by_first.size()-1).insert(std::make_pair(aef.at(ix44), ix));

						StateCopy(workingstate, orig);
						if (Rules->generator->Condition(ix)){
							achieved_by.at(achieved_by.size()-1)[aef.at(ix44)].insert(ix);
						}
						StateCopy(workingstate, backup_state_lower);
					}
				}


//				if (achieved_by_first.size() > 1){
//					std::map<mu__any*, int>::iterator itn;
//					for (itn = achieved_by_first.at(achieved_by_first.size()-2).begin(); itn != achieved_by_first.at(achieved_by_first.size()-2).end(); ++itn){
//						achieved_by_first.at(achieved_by_first.size()-1).insert(std::make_pair((*itn).first, 0));
////						(*itn).first->print();
//					}
//				}



				StateCopy(workingstate, next_rpg_state_lower);

				Rules->generator->Code_numeric_ff_minus(ix);

				StateCopy(next_rpg_state_lower, workingstate);



				StateCopy(workingstate, next_rpg_state_upper);

				Rules->generator->Code_numeric_ff_plus(ix);

				StateCopy(next_rpg_state_upper, workingstate);

			}

		}  // FOR LOOP END


		StateCopy(workingstate, next_rpg_state_lower);

		state* trpg_l = new state(next_rpg_state_lower);
//		cout << "\n\n*****LOWER STATE PRINT *****\n" << endl;
//		next_rpg_state_lower->print();
		update_lower_bound(bounds);
		update_lower_bound(goal_bounds);


		StateCopy(workingstate, next_rpg_state_upper);

		state* trpg_u = new state(next_rpg_state_upper);
//		cout << "\n\n*****UPPER STATE PRINT *****\n" << endl;
//		next_rpg_state_upper->print();
		update_upper_bound(bounds);
		update_upper_bound(goal_bounds);

		rpg_state_facts.push_back(std::make_pair(trpg_l, trpg_u));

//		std::map<mu_1_real_type*, std::pair<double, double> >::iterator itb;
//		for (itb=bounds.begin(); itb != bounds.end(); ++itb){
//			cout << "BOUNDS for: " << itb->first->name << "  =  ( " << bounds[itb->first].first << " , " << bounds[itb->first].second << " )" << endl;
//		}
//		cout << "\n-------------------------------------------------\n\n" << endl;
//
//		cout << "\n\n--------------------------------------------\nCURRENT GOAL RPG FACT LAYER BOUNDS:\n" << endl;
//
//		std::map<mu_1_real_type*, std::pair<double,double> >::iterator ittt;
//		for (ittt=goal_bounds.begin(); ittt!=goal_bounds.end(); ++ittt){
//			cout << ittt->first->name << endl;
//			cout << ittt->second.first << endl;
//			cout << ittt->second.second << endl;
//		}
//
//		cout << "\n-------------------------------------------------\n\n\n" << endl;


		if (goal_check()){
//			cout << "\n\n GOAL GOAL GOAL GOAL!!!\n" << endl;
			break;
		}

		if ((rpg_state_facts.size() > 1
				&& StateCmp(rpg_state_facts.at(rpg_state_facts.size()-1).first, rpg_state_facts.at(rpg_state_facts.size()-2).first) == 0
				 && StateCmp(rpg_state_facts.at(rpg_state_facts.size()-1).second, rpg_state_facts.at(rpg_state_facts.size()-2).second) == 0)
				|| rpg_state_facts.size() > 50
			){
//			cout << "\n\n OUT OF LAYERS!!!\n" << endl;
			StateCopy(workingstate, temp_ws);
			return 999999;
		}


	} // WHILE END


	/*************************
	 *
	 * BACKWARDS SEARCH
	 *
	 */


	std::set<mu__any*> backwards = all_goal_fact_layer;


//	cout << "\n\nGOAL FACTS:\n" << endl;
////
//	std::set<mu__any*>::iterator itg;
//	for (itg=backwards.begin(); itg!=backwards.end();++itg){
//		(*itg)->print();
//	}
//	cout << "=====================\n\n" << endl;


	std::vector<std::set<int> >rpg_final_all;
	int curr_action_layer = achieved_by_first.size()-1;
	std::set<mu__any*> g1 = get_fact_layer(rpg_state_facts.at(1).first);

	std::set<mu__any*>::iterator itt;
	std::set<mu__any*> temp_set;

//	ha.clear();


//	std::map<mu__any*, std::set<int> >::iterator itt2;
//	for (int i = 0; i < achieved_by.size(); i++){
//		for (itt2=achieved_by.at(i).begin(); itt2!=achieved_by.at(i).end(); ++itt2){
//			(*itt2).first->print();
//			std::set<int>::iterator its;
//			for (its = (*itt2).second.begin(); its != (*itt2).second.end(); ++its){
//				if((*its) == 0){
//					cout << " NO-OP" << endl;
//				}
//				else {
//					cout << Rules->RuleName((*its)) << endl;
//				}
//			}
//		}
//		cout << "\n******************************************************************************** \n\n" << endl;
//	}



//	cout << "THIS HERE IS THE NUMBER OF LAYERS IN THE RPG!!!!!! : " << rpg_state_facts.size() << endl;

//	state* temp_print = new state();
//	StateCopy(temp_print, workingstate);
//	for (int i =0; i < rpg_state_facts.size(); i++){
//		StateCopy(workingstate, rpg_state_facts.at(i).first);
//		workingstate->print();
//	}
//	StateCopy(workingstate, temp_print);


	while (curr_action_layer >= 0 ){

		std::set<int>rpg_final;
		rpg_final.clear();

		for (itt=backwards.begin(); itt!=backwards.end(); ++itt){

//				if ( rpg_final.count(achieved_by_first.at(curr_action_layer)[*itt]) >= 1) continue;
//
//				std::copy(action_pre.at(curr_action_layer)[achieved_by_first.at(curr_action_layer)[*itt]].begin(), action_pre.at(curr_action_layer)[achieved_by_first.at(curr_action_layer)[*itt]].end(), std::inserter(temp_set, temp_set.end()));
//
//				if (init_facts.count(*itt) == 0){
//
//					rpg_final.insert(achieved_by_first.at(curr_action_layer)[*itt]);


			if ((*itt)->get_mu_type() == "mu_0_boolean" && curr_action_layer >= 1 && achieved_by_first.at(curr_action_layer-1).count((*itt)) > 0) {
				temp_set.insert((*itt));
				continue;
			}
			if (rpg_final.count(achieved_by_first.at(curr_action_layer)[*itt]) >= 1) {
				continue;
			}

//			if (achieved_by_first.at(curr_action_layer)[*itt] == 0){
//				temp_set.insert((*itt));
//			}
//			else {
				std::copy(action_pre.at(curr_action_layer)[achieved_by_first.at(curr_action_layer)[*itt]].begin(),
						action_pre.at(curr_action_layer)[achieved_by_first.at(curr_action_layer)[*itt]].end(),
						std::inserter(temp_set, temp_set.end()));

				if (init_facts.count(*itt) == 0 /*&& achieved_by_first.at(curr_action_layer)[*itt] != 0*/){

					rpg_final.insert(achieved_by_first.at(curr_action_layer)[*itt]);





	//					if (achieved_by[*itt].size() == 1){
	//						rpg_final.insert(achieved_by_first[*itt]);
	//					} else if (achieved_by[*itt].size() > 1){
	//						std::set<int>::iterator itt2;
	//						for (itt2=achieved_by[*itt].begin(); itt2!=achieved_by[*itt].begin(); ++itt2){
	//							if ((*itt2) == achieved_by_first[*itt]){
	//								rpg_final.insert((*itt2));
	//							}
	//						}
	//					}


//						(*itt)->print();
//						cout << Rules->RuleName(achieved_by_first.at(curr_action_layer)[(*itt)]) << endl;

						if (g1.count(*itt)==1){
							ha.insert(achieved_by.at(curr_action_layer)[(*itt)].begin(), achieved_by.at(curr_action_layer)[(*itt)].end());
						}

				}
//			}
		}

		rpg_final_all.push_back(rpg_final);

//		rpg_final.clear();

//		cout << "------------------------" << endl;

		backwards = temp_set;
		curr_action_layer--;
		temp_set.clear();

		if (backwards.size() == 0) break;

	}


//	cout << "---------------------------------\n\n\n" << endl;

	rpg_length = 0;
	for (int i = 0; i < rpg_final_all.size(); i++){
		rpg_length += rpg_final_all.at(i).size();
	}

//	cout << "\n\n\nLAYERS: " << rpg_final_all.size() << "\n\n\n" << endl;

	StateCopy(workingstate, temp_ws);


	workingstate->set_h_val(rpg_length);
//	workingstate->set_g_val(workingstate->previous.get_sp()->get_g_val()+1);
	workingstate->set_g_val(0);
	workingstate->set_f_val();

//	cout << "\nNEW STATE: \n" << endl;
//	temp_ws->print();

	state * tend = new state;
	StateCopy(tend, workingstate);



	helpful_actions[tend] = ha;





//	std::set<int>::iterator itrpg;
//	cout << "\n\nRPG GRAPH\n\n" << endl;
//	for (int i =0; i < rpg_final_all.size(); i++){
//		for (itrpg=rpg_final_all.at(i).begin(); itrpg!=rpg_final_all.at(i).end(); ++itrpg){
//			cout << Rules->RuleName(*itrpg) << endl;
//		}
//		cout << "\n+++++++++++++++++++++++++++++++++++++\n" << endl;
//	}
//	cout << "/////////////////////////////////\n\n" << endl;
////
//	std::set<int>::iterator itha;
//	cout << "\n\nHELPFUL ACTIONS\n\n" << endl;
//	for (itha=ha.begin(); itha!=ha.end(); ++itha){
//		cout << Rules->RuleName(*itha) << endl;
//	}
//	cout << "\n*****************\n\n" << endl;
//
//	cout << "CURRENT STATE HEURISTIC VALUE: " << rpg_length << endl;
//
//	cout << "\n\n\n---------------------------END OF RPG---------------------------\n\n\n\n" << endl;

	delete next_rpg_state_upper;
	delete next_rpg_state_lower;
//	delete temp_ws;
	delete backup_state_lower;
	delete backup_state_upper;

//	start_total = clock() - start_total;
//	cout << "\nTOTAL RPG TIME: " << (((float)start_total)/CLOCKS_PER_SEC) << " (" << start_total << " clicks)\n\n ----------------------------------------------------" << endl;

	return (rpg_length);
}

