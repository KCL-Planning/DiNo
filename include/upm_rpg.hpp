/*
 * upm_rpg.hpp
 *
 *  Created on: 7 Mar 2014
 *      Author: k1328088
 */

#ifndef UPM_RPG_HPP_
#define UPM_RPG_HPP_
//#include <std>
#include <iostream>
#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>

class mu_1_real_type;

class upm_rpg {

private:
	std::set<mu_0_boolean*> bool_goal_fact_layer;
	std::vector<std::set<mu_0_boolean*> > rpg_facts;
	std::vector<state*> rpg_state_facts;
	std::set<mu_0_boolean*> init_facts;
//	std::vector<std::set<int> > rpg_actions;
	std::map<int, std::vector<mu_0_boolean*> > action_pre;
	std::map<mu_0_boolean*, std::set<int> > achieved_by;
	std::map<mu_0_boolean*, int > achieved_by_first;
	std::vector<int> remaining_rules;

	upm_rpg();
	upm_rpg(upm_rpg const&);
	void operator=(upm_rpg const&);

public:
//	upm_rpg();
//	~upm_rpg(){delete this;};
//		delete this;
////		upm_rpg();
//	}

	static upm_rpg& getInstance(){
			static upm_rpg instance;
//			instance.clear_all();
			return instance;
	}

	double compute_rpg();
//	double get_rpg_value();
	std::set<mu_0_boolean*> get_fact_layer();
	std::set<mu_0_boolean*> get_fact_layer(state* st);
	void clear_all(){
		std::vector<std::set<mu_0_boolean*> >().swap(rpg_facts);
//		std::vector<std::set<int> >().swap(rpg_actions);
		std::vector<state*>().swap(rpg_state_facts);
		action_pre.clear();
		achieved_by.clear();
		achieved_by_first.clear();
	}
};

#endif /* UPM_RPG_HPP_ */
