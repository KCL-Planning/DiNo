/*
* DiNo Release 1.0
* Copyright (C) 2015: W. Piotrowski, M. Fox, D. Long, D. Magazzeni, F. Mercorio
*
* Read the file "LICENSE.txt" distributed with these sources, or call
* DiNo with the -l switch for additional information.
* Contact: (wiktor.piotrowski@kcl.ac.uk)
*
*/

#ifndef UPM_STAGED_RPG_HPP_
#define UPM_STAGED_RPG_HPP_
//#include <std>
#include <iostream>
#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>

//class mu__any;
//class mu_1_real_type;

struct e_tuple {
	char * op;
	int idx;
	double dmin;
	double dmax;

	e_tuple(char* o, int id, double dmi, double dma) : op (o), idx(id), dmin(dmi), dmax(dma) {}

	void print(){
		cout << "\n\nOP: " << op << "\nINDEX: " << idx << "\nDUR MIN: " << dmin << "\nDUR MAX: " << dmax << "\n\n";
	}

};

struct s_tuple {
	state* F;
	std::vector<e_tuple> E;
	std::vector<int> T;

//	s_tuple(state* f, std::vector<e_tuple> e, std::vector<int> t) : F(f), E(e), T(t) {}

};


class upm_staged_rpg {

private:
	std::set<mu__any*> all_goal_fact_layer;
	std::set<mu_0_boolean*> bool_goal_fact_layer;
	std::map<mu__real*, std::pair<double, int> > num_goal_fact_layer;
	std::vector<std::set<mu_0_boolean*> > rpg_facts;
	std::vector<std::pair<state*,state*> > rpg_state_facts;
	std::set<mu__any*> init_facts;
//	std::vector<std::set<int> > rpg_actions;
	std::vector<std::map<int, std::vector<mu__any*> > > action_pre;
	std::vector<std::map<mu__any*, std::set<int> > > achieved_by;
	std::vector<std::map<mu__any*, int > > achieved_by_first;
	std::vector<int> remaining_rules;
	std::map<mu__real*, std::pair<double, double> > bounds;
	std::map<mu__real*, std::pair<double, double> > goal_bounds;

	s_tuple S;

	std::vector<int> start_snap_actions;
	std::vector<int> end_snap_actions;
	std::map<int, mu_0_boolean*> a_started;
	std::map<int, int> snap_actions;
	std::vector< std::pair< std::pair<state*, state*>, double > > temporal_layers;

	std::set<int> ea;
	std::map<int, double> t_min;



	upm_staged_rpg();
	upm_staged_rpg(upm_staged_rpg const&);
	void operator=(upm_staged_rpg const&);

public:

	static upm_staged_rpg& getInstance(){
			static upm_staged_rpg instance;
			return instance;
	}

	double compute_rpg();
//	double get_rpg_value();
	std::set<mu__any*> get_fact_layer();
	std::set<mu__any*> get_fact_layer(state* st);
	std::map<mu__real*, std::pair<double, double> > get_initial_bounds();
	void update_lower_bound(std::map<mu__real*, std::pair<double, double> >&);
	void update_upper_bound(std::map<mu__real*, std::pair<double, double> >&);
	bool metric_condition(int r);
	bool goal_check();
	void update_clocks();
	void print_bounds();
	void clear_all(){
		std::vector<std::set<mu_0_boolean*> >().swap(rpg_facts);
//		std::vector<std::set<int> >().swap(rpg_actions);
		std::vector<std::pair<state*, state*> >().swap(rpg_state_facts);
//		std::vector< std::pair< std::pair<state*, state*>, std::pair< std::vector<e_tuple>, std::vector<int> > > >().swap(temporal_layers);
		action_pre.clear();
		achieved_by.clear();
		achieved_by_first.clear();
		ea.clear();
		t_min.clear();

	}
};

#endif /* UPM_STAGED_RPG_HPP_ */
