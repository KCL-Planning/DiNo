/*
 * upm_rpg.cpp
 *
 *  Created on: 7 Mar 2014
 *      Author: k1328088
 */

#include "upm_rpg.hpp"
//#include <std>
#include <string>
#include <set>
#include <list>
#include <iterator>
#include <vector>
#include <map>
#include <ctime>
#include <time.h>


upm_rpg::upm_rpg() {
	// TODO Auto-generated constructor stub
	init_facts = std::set<mu_0_boolean*>();
//	rpg_actions = std::vector<std::set<int> >();
	bool_goal_fact_layer = get_bool_goal_conditions();
	rpg_state_facts = std::vector<state*>();

	achieved_by = std::map<mu_0_boolean*, std::set<int> >();
	achieved_by_first = std::map<mu_0_boolean*, int >();
	action_pre = std::map<int, std::vector<mu_0_boolean*> >();

	for( int i = 0; i < RULES_IN_WORLD; i++ )
	    remaining_rules.push_back(i);
}




std::set<mu_0_boolean*> upm_rpg::get_fact_layer(){

	std::set<mu_0_boolean*> f_list;

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

std::set<mu_0_boolean*> upm_rpg::get_fact_layer(state* st){

	std::set<mu_0_boolean*> f_list;

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

double upm_rpg::compute_rpg(){

	std::set<int> ha;

//	helpful_actions.clear();

//	clock_t start_total = clock();

	action_pre.clear();
	achieved_by.clear();
	achieved_by_first.clear();
	rpg_state_facts.clear();
	rpg_facts.clear();
	init_facts.clear();

	if (bool_goal_fact_layer.size() == 0) return 999999;

	double rpg_length;

	static state* orig = new state;
	StateCopy(orig, workingstate);

	state* temp_ws = new state(workingstate);
	state* backup_state = new state();

	std::vector<int> temp_rem_rules = remaining_rules;
	std::set<int> temp_rem_rules2(remaining_rules.begin(), remaining_rules.end());

	state* next_rpg_state = new state(workingstate);

	StateCopy(backup_state, next_rpg_state);

	state* trpg0 = new state(workingstate);

	rpg_state_facts.push_back(temp_ws);

	init_facts = get_fact_layer(temp_ws);

	std::set<mu_0_boolean*> f_l;


	while (true){

		StateCopy(backup_state, workingstate);

		for (int ix = 0; ix < temp_rem_rules.size(); ix++){

			int ixxx = temp_rem_rules.at(ix);

			StateCopy(workingstate, backup_state);

			if (Rules->generator->Condition(ixxx)){

				action_pre.insert(std::make_pair(ixxx, Rules->generator->bool_precond_array(ixxx)));
//				action_pre[r_name] = Rules->generator->precond_array(ixxx);

				std::vector<mu_0_boolean*> aef = Rules->generator->effects_add_bool_array(ixxx);

				for (int ix44 = 0; ix44 < aef.size(); ix44++){

					if (init_facts.count(aef.at(ix44))==0){
						achieved_by_first.insert(std::make_pair(aef.at(ix44), ixxx));
//						achieved_by_first[aef.at(ix44)] = ixxx;
						StateCopy(workingstate, orig);
						if (Rules->generator->Condition(ixxx)){
							achieved_by[aef.at(ix44)].insert(ixxx);
							StateCopy(workingstate, backup_state);
						}
					}
				}

				StateCopy(workingstate, next_rpg_state);

				Rules->generator->Code_ff(ixxx);

				temp_rem_rules2.erase(ixxx);

				StateCopy(next_rpg_state, workingstate);
			}

		}  // FOR LOOP END


		StateCopy(workingstate, next_rpg_state);

		if (temp_rem_rules.size() == temp_rem_rules2.size()){
			StateCopy(workingstate, temp_ws);
//
////			cout << "\n\nTWO RULE ARRAYS ARE THE SAME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" << endl;
//
////			if ((new StatePtr(workingstate))->isStart()){
				return 999999;
////			}
//
////			return (workingstate->previous.get_sp()->get_h_val());
		}


		if ((rpg_state_facts.size() > 1 && StateCmp(rpg_state_facts.at(rpg_state_facts.size()-1), rpg_state_facts.at(rpg_state_facts.size()-2)) == 0)
				|| rpg_state_facts.size() > 60
			){
//			cout << "\n\nTWO LAYERS ARE THE SAME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" << endl;
			StateCopy(workingstate, temp_ws);
			return 999999;
		}



		temp_rem_rules.resize(temp_rem_rules2.size());
		std::copy(temp_rem_rules2.begin(), temp_rem_rules2.end(), temp_rem_rules.begin());

		StateCopy(backup_state, workingstate);

		state* trpg = new state(workingstate);

		rpg_state_facts.push_back(trpg);

		if (mu__goal__00())	{

//			cout << "                GOAL " << endl;

			break;
		}

	} // WHILE END


	/*************************
	 *
	 * BACKWARDS SEARCH
	 *
	 */


	std::set<mu_0_boolean*> backwards = bool_goal_fact_layer;
	std::set<int> rpg_final;
	int curr_action_layer = rpg_state_facts.size();
	std::set<mu_0_boolean*> g1 = get_fact_layer(rpg_state_facts.at(1));

	std::set<mu_0_boolean*>::iterator itt;
	std::set<mu_0_boolean*> temp_set;

//	ha.clear();



	while (curr_action_layer >= 0 ){

		for (itt=backwards.begin(); itt!=backwards.end(); ++itt){

				if ( rpg_final.count(achieved_by_first[*itt]) >= 1) continue;

				std::copy(action_pre[achieved_by_first[*itt]].begin(), action_pre[achieved_by_first[*itt]].end(), std::inserter(temp_set, temp_set.end()));

				if (init_facts.count(*itt) == 0){

					rpg_final.insert(achieved_by_first[*itt]);

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


//					cout << (*itt)->name << endl;
//					cout << Rules->RuleName(achieved_by_first[(*itt)]) << endl;

					if (g1.count(*itt)==1){
						ha.insert(achieved_by[(*itt)].begin(), achieved_by[(*itt)].end());
					}

				}
		}

//		cout << "------------------------" << endl;

		backwards = temp_set;
		curr_action_layer--;
		temp_set.clear();

		if (backwards.size() == 0) break;

	}


//	cout << "---------------------------------\n\n\n" << endl;


	rpg_length = rpg_final.size();

//	helpful_actions[temp_ws] = ha;

	StateCopy(workingstate, temp_ws);

	workingstate->set_h_val(rpg_length);
//	workingstate->set_g_val(workingstate->previous.get_sp()->get_g_val()+1);
	workingstate->set_g_val(0);
	workingstate->set_f_val();

	state * tend = new state;
	StateCopy(tend, workingstate);



	helpful_actions[tend] = ha;



//	cout << "\nNEW STATE: \n" << endl;
//	temp_ws->print();

//	std::set<int>::iterator itrpg;
//	cout << "\n\n" << endl;
//	for (itrpg=rpg_final.begin(); itrpg!=rpg_final.end(); ++itrpg){
//		cout << Rules->RuleName(*itrpg) << endl;
//	}
//	cout << "\n*****************\n\n" << endl;
//
//	std::set<int>::iterator itha;
//	for (itha=ha.begin(); itha!=ha.end(); ++itha){
//		cout << Rules->RuleName(*itha) << endl;
//	}
//	cout << "\n*****************\n\n" << endl;
//
//	cout << "CURRENT STATE HEURISTIC VALUE: " << rpg_length << endl;
//
//
//	cout << "\n\n\n---------------------------END OF RPG---------------------------\n\n\n\n" << endl;

	delete next_rpg_state;
//	delete temp_ws;
	delete backup_state;

//	start_total = clock() - start_total;
//	cout << "\nTOTAL RPG TIME: " << (((float)start_total)/CLOCKS_PER_SEC) << " (" << start_total << " clicks)\n\n ----------------------------------------------------" << endl;

	return (rpg_length);
}
