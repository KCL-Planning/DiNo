/*
 * upm_numeric_rpg.hpp
 *
 *  Created on: 2 Mar 2015
 *      Author: k1328088
 */

#ifndef UPM_NUMERIC_RPG_HPP_
#define UPM_NUMERIC_RPG_HPP_
//#include <std>
#include <iostream>
#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>

//class mu__any;
//class mu_1_real_type;


class upm_numeric_rpg {

private:
	std::set<mu__any*> all_goal_fact_layer;
	std::set<mu_0_boolean*> bool_goal_fact_layer;
	std::map<mu_1_real_type*, std::pair<double, int> > num_goal_fact_layer;
	std::vector<std::set<mu_0_boolean*> > rpg_facts;
	std::vector<std::pair<state*,state*> > rpg_state_facts;
	std::set<mu__any*> init_facts;
//	std::vector<std::set<int> > rpg_actions;
	std::vector<std::map<int, std::vector<mu__any*> > > action_pre;
	std::vector<std::map<mu__any*, std::set<int> > > achieved_by;
	std::vector<std::map<mu__any*, int > > achieved_by_first;
	std::vector<int> remaining_rules;
	std::map<mu_1_real_type*, std::pair<double, double> > bounds;
	std::map<mu_1_real_type*, std::pair<double, double> > goal_bounds;

	upm_numeric_rpg();
	upm_numeric_rpg(upm_numeric_rpg const&);
	void operator=(upm_numeric_rpg const&);

public:

	static upm_numeric_rpg& getInstance(){
			static upm_numeric_rpg instance;
			return instance;
	}

	double compute_rpg();
//	double get_rpg_value();
	std::set<mu__any*> get_fact_layer();
	std::set<mu__any*> get_fact_layer(state* st);
	std::map<mu_1_real_type*, std::pair<double, double> > get_initial_bounds();
	void update_lower_bound(std::map<mu_1_real_type*, std::pair<double, double> >&);
	void update_upper_bound(std::map<mu_1_real_type*, std::pair<double, double> >&);
	bool metric_condition(int r);
	bool goal_check();
	void clear_all(){
		std::vector<std::set<mu_0_boolean*> >().swap(rpg_facts);
//		std::vector<std::set<int> >().swap(rpg_actions);
		std::vector<std::pair<state*, state*> >().swap(rpg_state_facts);
		action_pre.clear();
		achieved_by.clear();
		achieved_by_first.clear();
	}
};

#endif /* UPM_NUMERIC_RPG_HPP_ */
