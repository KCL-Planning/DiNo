/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#include "upm_staged_rpg.hpp"
//#include <std>
#include <string>
#include <set>
#include <list>
#include <iterator>
#include <vector>
#include <map>
#include <ctime>
#include <time.h>

#define minim(a,b) (((a)<(b))?(a):(b))


upm_staged_rpg::upm_staged_rpg() {
	// TODO Auto-generated constructor stub
	init_facts = std::set<mu__any*>();
	bool_goal_fact_layer = get_bool_goal_conditions();
	num_goal_fact_layer = get_numeric_goal_conditions();
	rpg_state_facts = std::vector<std::pair<state*, state*> >();

	achieved_by = std::vector<std::map<mu__any*, std::set<int> > >();
	achieved_by_first = std::vector<std::map<mu__any*, int > >();
	action_pre = std::vector<std::map<int, std::vector<mu__any*> > >();

	a_started = std::map<int, mu_0_boolean*>();

	S = s_tuple();
	S.F = workingstate;
	S.E = std::vector<e_tuple>();
	S.T = std::vector<int>();

	ea = std::set<int>();
	t_min = std::map<int, double>();

	temporal_layers = std::vector< std::pair< std::pair<state*, state*>, double > >();

	all_goal_fact_layer = std::set<mu__any*>();




	// INITIALISING ARRAYS, MAPS AND SETS

	std::set<mu_0_boolean*>::iterator ita;
	for (ita=bool_goal_fact_layer.begin(); ita!=bool_goal_fact_layer.end(); ++ita){
		all_goal_fact_layer.insert(*ita);
	}


	std::map<mu__real*, std::pair<double, int> >::iterator itaa;
	for (itaa=num_goal_fact_layer.begin(); itaa!=num_goal_fact_layer.end(); ++itaa){
		all_goal_fact_layer.insert(itaa->first);
		goal_bounds[itaa->first] = (std::make_pair(0, 0));
	}


	start_snap_actions = std::vector<int>();
	for (int i=0; i < RULES_IN_WORLD; i++){
		if (Rules->RulePDDLClass(i) == RuleManager::DurativeStart) {
			start_snap_actions.push_back(i);

		}
	}

	end_snap_actions = std::vector<int>();
	for (int i=0; i < RULES_IN_WORLD; i++){
		if (Rules->RulePDDLClass(i) == RuleManager::DurativeEnd) {
			end_snap_actions.push_back(i);

			t_min.insert(std::make_pair(i, TIME_INFINITY));

		}
	}


	for (int isa = 0; isa < start_snap_actions.size(); isa++){
		snap_actions.insert(std::make_pair(start_snap_actions.at(isa), end_snap_actions.at(isa)));
	}


	bounds = get_initial_bounds();

	for( int i = 0; i < RULES_IN_WORLD; i++ ){
	    remaining_rules.push_back(i);
	}

}





void upm_staged_rpg::update_clocks(){

	std::map<int, int>::iterator isn;
	for (int i = 0; i < end_snap_actions.size(); i++){

			std::map<mu_0_boolean*, mu__real*> cl = Rules->generator->get_clocks(end_snap_actions.at(i));

			if ((*(Rules->generator->get_clocks(end_snap_actions.at(i)).begin())).first->value() == 1){
				(*(Rules->generator->get_clocks(end_snap_actions.at(i)).begin())).second->value((*(Rules->generator->get_clocks(end_snap_actions.at(i)).begin())).second->value() + mu_T);

			}
	}
}


void upm_staged_rpg::update_lower_bound(std::map<mu__real*, std::pair<double, double> > &bs){

	std::vector<mu__real*> v_temp = workingstate->get_mu_nums();
	std::vector<mu__real*> v_tempp = workingstate->get_mu_num_arrays();

	for(int i=0; i<v_temp.size();i++){
		if (bs.count(v_temp.at(i)) > 0 && (v_temp.at(i)->value() < bs[v_temp.at(i)].second)){
			bs[v_temp.at(i)].first = v_temp.at(i)->value();
		}
	}

	for(int i=0; i<v_tempp.size();i++){
		if (bs.count(v_tempp.at(i)) > 0 && (v_tempp.at(i)->value() < bs[v_tempp.at(i)].second)){
			bs[v_tempp.at(i)].first = v_tempp.at(i)->value();
		}
	}
}


void upm_staged_rpg::update_upper_bound(std::map<mu__real*, std::pair<double, double> > &bs){

	std::vector<mu__real*> v_temp = workingstate->get_mu_nums();
	std::vector<mu__real*> v_tempp = workingstate->get_mu_num_arrays();

	for(int i=0; i<v_temp.size();i++){
		if (bs.count(v_temp.at(i)) > 0 && (v_temp.at(i)->value() > bs[v_temp.at(i)].second)){
			bs[v_temp.at(i)].second = v_temp.at(i)->value();
		}
	}

	for(int i=0; i<v_tempp.size();i++){
		if (bs.count(v_tempp.at(i)) > 0 && (v_tempp.at(i)->value() > bs[v_tempp.at(i)].second)){
			bs[v_tempp.at(i)].second = v_tempp.at(i)->value();
		}
	}
}


bool upm_staged_rpg::metric_condition(int r){

	std::map<mu__real*, std::pair<double, int> > num_conds = Rules->generator->num_precond_array(r);
	std::vector<mu_0_boolean*> bool_conds = Rules->generator->bool_precond_array(r);


	for (int i=0; i < bool_conds.size(); i++){
		if (bool_conds.at(i)->value() == 0){
			return false;
		}

	}

	std::map<mu__real*, std::pair<double, int> >::iterator ittt;
	for (ittt=num_conds.begin(); ittt!=num_conds.end(); ++ittt){

		if (ittt->second.second == 0){
			if (ittt->second.first > bounds[ittt->first].second || ittt->second.first < bounds[ittt->first].first) {
				return false;
			}
		}
		else if (ittt->second.second == 1){
			if (bounds[ittt->first].second < ittt->second.first) {
				return false;
			}
		}
		else if (ittt->second.second == 2){
			if (bounds[ittt->first].first > ittt->second.first) {
				return false;
			}
		}
		else if (ittt->second.second == 3){
			if (bounds[ittt->first].second <= ittt->second.first) {
				return false;
			}

		}
		else if (ittt->second.second == 4){
			if (bounds[ittt->first].first >= ittt->second.first) {
				return false;
			}
		}
	}

	return true;
}


bool upm_staged_rpg::goal_check(){

	std::map<mu__real*, std::pair<double, int> > num_conds = num_goal_fact_layer;
	std::set<mu_0_boolean*> bool_conds = bool_goal_fact_layer;

	std::set<mu_0_boolean*>::iterator bit;
	for (bit=bool_conds.begin(); bit != bool_conds.end(); ++bit){
		if ((*bit)->value() == 0){
			return false;
		}
	}


	std::map<mu__real*, std::pair<double, int> >::iterator ittt;
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


std::set<mu__any*> upm_staged_rpg::get_fact_layer(){

	std::set<mu__any*> f_list;

	std::vector<mu_0_boolean*> v_temp = workingstate->get_mu_bools();
	std::vector<mu_0_boolean*> v_tempp = workingstate->get_mu_bool_arrays();
	v_temp.insert(v_temp.end(), v_tempp.begin(), v_tempp.end());

	for (int ix2 = 0; ix2 < (v_temp.size()); ix2++){
		if (v_temp.at(ix2)->value() == true){
			f_list.insert(v_temp.at(ix2));
		}
	}


	std::vector<mu__real*> v_temp2 = workingstate->get_mu_nums();
	std::vector<mu__real*> v_tempp2 = workingstate->get_mu_num_arrays();
	v_temp2.insert(v_temp2.end(), v_tempp2.begin(), v_tempp2.end());

	for (int ix2 = 0; ix2 < (v_temp2.size()); ix2++){
			f_list.insert(v_temp2.at(ix2));
	}


	return (f_list);
}


std::set<mu__any*> upm_staged_rpg::get_fact_layer(state* st){

	std::set<mu__any*> f_list;

	state* temps = new state(workingstate);

	StateCopy(workingstate, st);

	std::vector<mu_0_boolean*> v_temp = st->get_mu_bools();
	std::vector<mu_0_boolean*> v_tempp = st->get_mu_bool_arrays();
	v_temp.insert(v_temp.end(), v_tempp.begin(), v_tempp.end());

	for (int ix2 = 0; ix2 < (v_temp.size()); ix2++){
				if (v_temp.at(ix2)->value() == 1){
					f_list.insert(v_temp.at(ix2));
				}
	}

	StateCopy(workingstate, temps);
	delete temps;

	return (f_list);
}


void upm_staged_rpg::print_bounds(){

	cout << "\n\n" << endl;
	std::map<mu__real*, std::pair<double, double> >::iterator ib;
	for (ib = bounds.begin(); ib != bounds.end(); ++ib){

		cout << (*ib).first->name << "  =  ( " << (*ib).second.first << " , " << (*ib).second.second << " )" << endl;

	}
	cout << "\n\n" << endl;
}


std::map<mu__real*, std::pair<double, double> > upm_staged_rpg::get_initial_bounds(){

	std::map<mu__real*, std::pair<double, double> > bs;

	static state* temp_numr = new state;
	StateCopy(temp_numr, workingstate);

	std::vector<mu__real*> v_temp = workingstate->get_mu_nums();
	std::vector<mu__real*> v_tempp = workingstate->get_mu_num_arrays();
	v_temp.insert(v_temp.end(), v_tempp.begin(), v_tempp.end());

	for (int ix2 = 0; ix2 < (v_temp.size()); ix2++){
		bs[v_temp.at(ix2)] = (std::make_pair(v_temp.at(ix2)->value(), v_temp.at(ix2)->value()));
	}

	StateCopy(workingstate, temp_numr);

	return bs;
}




double upm_staged_rpg::compute_rpg(){


	double curr_mu_time = mu_TIME.value();

	std::set<int> ha;

	bounds = get_initial_bounds();

	double curr_time = 0;

	action_pre.clear();
	achieved_by.clear();
	achieved_by_first.clear();
	rpg_state_facts.clear();
	rpg_facts.clear();
	init_facts.clear();

	temporal_layers.clear();

	if (bool_goal_fact_layer.size() == 0 && num_goal_fact_layer.size() == 0) return 999999;

	double rpg_length = 999;

	static state* orig = new state;
	StateCopy(orig, workingstate);

	state* temp_ws = new state(workingstate);
	state* backup_state_lower = new state(workingstate);
	state* backup_state_upper = new state(workingstate);

	state* next_rpg_state_lower = new state(workingstate);

	StateCopy(backup_state_lower, workingstate);

	state* next_rpg_state_upper = new state(workingstate);

	StateCopy(backup_state_upper, workingstate);

	state* trpg0 = new state(workingstate);


	temporal_layers.push_back( std::make_pair( std::make_pair(trpg0, trpg0), mu_TIME.value()) );

	init_facts = get_fact_layer(temp_ws);

	std::set<mu_0_boolean*> f_l;


	while (true){

		std::map<mu__any*, std::set<int> > curr_ach_by;
		std::map<int, std::vector<mu__any*> > curr_a_pre;
		std::map<mu__any*, int> curr_ach_by_first;

		achieved_by.push_back(curr_ach_by);
		action_pre.push_back(curr_a_pre);
		achieved_by_first.push_back(curr_ach_by_first);

		StateCopy(backup_state_lower, temporal_layers.at(temporal_layers.size()-1).first.first);
		StateCopy(backup_state_upper, temporal_layers.at(temporal_layers.size()-1).first.second);

		StateCopy(workingstate, backup_state_lower);

//		curr_time += (10*mu_T);

		for (int ix = 0; ix < RULES_IN_WORLD; ix++){

			StateCopy(workingstate, backup_state_lower);

			if (metric_condition(ix) == true){

					action_pre.at(action_pre.size()-1).insert(std::make_pair(ix, Rules->generator->all_precond_array(ix)));

					std::vector<mu__any*> aef = Rules->generator->effects_all_array(ix);

					for (int ix44 = 0; ix44 < aef.size(); ix44++){

						if (init_facts.count(aef.at(ix44))==0 && ix != 0){

							achieved_by_first.at(achieved_by_first.size()-1).insert(std::make_pair(aef.at(ix44), ix));


							StateCopy(workingstate, orig);
							if (Rules->generator->Condition(ix)){
								achieved_by.at(achieved_by.size()-1)[aef.at(ix44)].insert(ix);
							}
							StateCopy(workingstate, backup_state_lower);
						}
					}


				StateCopy(workingstate, next_rpg_state_lower);
				Rules->generator->Code_numeric_ff_minus(ix);
				StateCopy(next_rpg_state_lower, workingstate);

				StateCopy(workingstate, next_rpg_state_upper);
				Rules->generator->Code_numeric_ff_plus(ix);
				StateCopy(next_rpg_state_upper, workingstate);

			}

		}  // FOR LOOP END


		StateCopy(workingstate, next_rpg_state_upper);
		workingstate->fire_processes_plus(1);
		mu_event_check();
		StateCopy(next_rpg_state_upper, workingstate);



		StateCopy(workingstate, next_rpg_state_lower);
		workingstate->fire_processes_minus(1);
		mu_event_check();
		StateCopy(next_rpg_state_lower, workingstate);

		state* trpg_l = new state(next_rpg_state_lower);
//		cout << "\n\n*****LOWER STATE PRINT*****\n" << endl;
//		next_rpg_state_lower->print();
		update_lower_bound(bounds);
		update_lower_bound(goal_bounds);


		StateCopy(workingstate, next_rpg_state_upper);

		state* trpg_u = new state(next_rpg_state_upper);
//		cout << "\n\n*****UPPER STATE PRINT*****\n" << endl;
//		next_rpg_state_upper->print();
		update_upper_bound(bounds);
		update_upper_bound(goal_bounds);

		temporal_layers.push_back(std::make_pair(std::make_pair(trpg_l, trpg_u), mu_TIME.value()));

//		print_bounds();

		if (goal_check()){

			delete trpg_u;
			delete trpg_l;
			break;
		}


		if (
				(mu_TIME.value() > args->SRPG_horizon.value)
			){

				StateCopy(workingstate, temp_ws);
				workingstate->set_h_val(999);
				workingstate->set_g_val(0);
				workingstate->set_f_val();
				delete trpg0;
				delete backup_state_lower;
				delete backup_state_upper;
				delete next_rpg_state_upper;
				delete next_rpg_state_lower;
				delete temp_ws;
				delete trpg_u;
				delete trpg_l;
//				cout << "\n\nENDING THE LOOP WITH NO GOAL!\n\n" << endl;
				return 999;
		}

		delete trpg_u;
		delete trpg_l;

	} // WHILE END


	/*************************
	 *
	 * BACKWARDS SEARCH
	 *
	 */


	std::set<mu__any*> backwards = all_goal_fact_layer;


	std::vector<std::set<int> >rpg_final_all;
	int curr_action_layer = achieved_by_first.size()-1;
	std::set<mu__any*> g1 = get_fact_layer(temporal_layers.at(1).first.second);

	std::set<mu__any*>::iterator itt;
	std::set<mu__any*> temp_set;

	while (curr_action_layer >= 0 ){

		std::set<int>rpg_final;
		rpg_final.clear();

		for (itt=backwards.begin(); itt!=backwards.end(); ++itt){


			if ((*itt)->get_mu_type() == "mu_0_boolean" && curr_action_layer >= 1 && achieved_by_first.at(curr_action_layer-1).count((*itt)) > 0) {
				temp_set.insert((*itt));
				continue;
			}
			if (rpg_final.count(achieved_by_first.at(curr_action_layer)[*itt]) >= 1) {
				continue;
			}

				std::copy(action_pre.at(curr_action_layer)[achieved_by_first.at(curr_action_layer)[*itt]].begin(),
						action_pre.at(curr_action_layer)[achieved_by_first.at(curr_action_layer)[*itt]].end(),
						std::inserter(temp_set, temp_set.end()));

				if (init_facts.count(*itt) == 0 ){

					rpg_final.insert(achieved_by_first.at(curr_action_layer)[*itt]);

					if (g1.count(*itt)==1){
						ha.insert(achieved_by.at(curr_action_layer)[(*itt)].begin(), achieved_by.at(curr_action_layer)[(*itt)].end());
					}

				}
		}


		rpg_final_all.push_back(rpg_final);


		backwards = temp_set;
		curr_action_layer--;
		temp_set.clear();

		if (backwards.size() == 0) break;

	}

	if (ha.size() < 1){
		ha.insert(0);
	}

	for (int i = 1; i < rpg_final_all.size()-1; i++){
		if (rpg_final_all.at(i).size() == 0){
			rpg_final_all.at(i).insert(0);
		}
	}

	rpg_length = 0;
	for (int i = 0; i < rpg_final_all.size(); i++){
		rpg_length += rpg_final_all.at(i).size();
	}

	StateCopy(workingstate, temp_ws);


	workingstate->set_h_val(rpg_length);
	workingstate->set_g_val(0);
	workingstate->set_f_val();

	state * tend = new state;
	StateCopy(tend, workingstate);

	helpful_actions[tend] = ha;


	delete next_rpg_state_upper;
	delete next_rpg_state_lower;
	delete backup_state_lower;
	delete backup_state_upper;
    delete temp_ws;
    delete trpg0;
    delete tend;

	return (rpg_length);
}
