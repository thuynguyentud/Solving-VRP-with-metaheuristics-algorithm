#ifdef _MSC_VER
#include "stdafx.h"
#endif

#ifndef VRP_PP_NBH_H
#define VRP_PP_NBH_H

#include <string>
#include <iostream>
#include "vrp_pp_extern.h"

// if necessary include the lp_solve symbols
#ifdef VRP_PP_SOLVER
    #pragma message "lp_solve components are considered"
    #include "lp_lib.h"
#else
    #pragma message "lp_solve components are not considered"
#endif // VRP_PP_SOLVER

// Version 3.0 - 05.03.2021 - 08:30
// added: NBH::NBH_sort_by_fitness_increasing_feasfirst()

//#include "global.h"			// embeds the global definitions
//#include "problem.h"		// embeds data structures and methods to manage problem data
//#include "plan_route.h"		// embeds data structures and methodes to manage a plan

// this file contains the definition of all data types needed to use neighborhoods of plans

// struct NBH_neighbor is one element of a neighborhod

//predefinition
class VRP_PP_PLAN;

struct NBH_neighbor {

    // to be used to store a unique key to identify this neighbor
	int NBH_id;

	// =0 if real neighbor =-1 if dummy first or last operation
	int NBH_type;

	// to store an evaluation value valuating this neighbor
	double NBH_fitness;

	// indivator for feasibility of neighbor
	int NBH_feasible;

	// reference to the PLAN that is the neighor
    class VRP_PP_PLAN *ref;

    // reference to the previous neighbor element
	struct NBH_neighbor *previous;

	// reference to the previous neighbor element
	struct NBH_neighbor *next;

	// constructor
	NBH_neighbor(int NBH_id,
              int NBH_type,
              double NBH_fitness,
              int NBH_feasible,
              struct NBH_neighbor *previous,
              struct NBH_neighbor *next,
              class VRP_PP_PLAN *ref);

	// deconstructor
	~NBH_neighbor();
};

// struct NBH is the neighborhood

struct NBH {

    // contains a unique key that identifies this neighborhood
    int NBH_id;

	// contains number of members in the neighborhood
    int NBH_members;

    // dummy first element in neighbors list
    struct NBH_neighbor *first;

    // dummy last element in neighbors list
    struct NBH_neighbor *last;

    // constructor
	NBH(int _ID);

	// destructor
	~NBH();

    void NBH_print_member_list(void);

    // adds the VRP_PP_PLAN-object _PLAN to the neighborhood
	void NBH_add_plan(int Member_ID, class VRP_PP_PLAN *_PLAN);
	void NBH_remove_plan(int Member_ID);
	void NBH_remove_plan_at_pos(int _POSITION);
    class VRP_PP_PLAN *NBH_get_pointer_to_plan_at_pos(int _POS);
	void NBH_update_fitness_at_pos(double _NewFitness, int _Feasible, int _POS);
	void NBH_reset(class VRP_PP_PROBLEM *prob);
	int NBH_index_min_fitness();

    void NBH_sort_by_fitness_increasing();
    void NBH_sort_by_fitness_increasing_feasfirst();
    void NBH_swap_with_next(struct NBH_neighbor *Ptr);
    double NBH_get_average_fitness_from_to(int StartPos, int EndPos);
};

#endif
