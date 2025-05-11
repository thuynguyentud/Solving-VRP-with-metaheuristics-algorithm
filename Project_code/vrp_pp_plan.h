// version 2.0 - last modification 25.02.2020 at 18:29
// version 2.1 - last modification 27.04.2020 at 17:09 plan-attributes declared to be private
// version 3.0 - last modification 05.03.2021 at 11:39
// -- addition of the compiler control if lp_solve-based components are included

// version 4.0 - last modification 01.04.2022 at 19:30
// -- addition of the compiler control if osm components are included

#ifndef VRP_PP_PLAN_H
#define VRP_PP_PLAN_H

#include "vrp_pp_route.h"
#include "vrp_pp_problem.h"
#include "vrp_pp_extern.h"

class VRP_PP_PLAN
{

    friend class VRP_PP_OPERATION;
    friend class VRP_PP_OSM_TWIN;

public:
	// CREATE, COPY, ASSIGN and DESTROY
	VRP_PP_PLAN();											// default constructor: creates an empty plan with zero routes - use it for arrays
	VRP_PP_PLAN(const VRP_PP_PROBLEM *problem);				// standard constructor: creates a plan with with an empty route for each vehicle with the problem
	VRP_PP_PLAN(const VRP_PP_PLAN& source);					// copy constructor: required due to heap memory member -> cannot use compiler default copy constructor
	VRP_PP_PLAN& operator=(const VRP_PP_PLAN& source);		// overloaded assignment operator: now us can use r = r_assign_to  (assuming r and r_assign_to are VRP_PP_PLAN objects)
	~VRP_PP_PLAN();											// destructor: deallocation of heap memory
    VRP_PP_PLAN(const std::string& filename);               // constructor reading a plan from an xml-file (BE CAREFUL WITH IT!)

	// PRINT & EXPORT
	void print_as_operation_tables(const VRP_PP_PROBLEM *prob)const;		// prints the set of stored routes as well as important evaluation results
	void print_short(const VRP_PP_PROBLEM *prob)const;						// prints a compressed plan
	void print_long(const VRP_PP_PROBLEM *prob)const;						// prints a compressed plan including used arcs
	void print_very_long(const VRP_PP_PROBLEM *prob)const;					// prints a compressed plan including used arcs as well as loading lists
	void print_as_node_sequences(const VRP_PP_PROBLEM *prob)const;			// prints a very compressed plan
	void print_as_kml(std::ostream& stream, const VRP_PP_PROBLEM*prob, bool PrintBubbles)const;// prints polylines in kml-format
	void print_as_kml(std::ostream& stream, const VRP_PP_PROBLEM*prob)const;// prints polylines in kml-format
   	void print_as_kml_with_nodes(std::ostream& stream, const VRP_PP_PROBLEM*prob)const;// prints polylines in kml-format

	void print_as_tex(std::ostream& stream,const VRP_PP_PROBLEM*prob)const;
	void print_with_network_as_tex(std::ostream& stream, const VRP_PP_NODE_COORDINATES& NodeCoordinates, const VRP_PP_PROBLEM*prob)const;

	// GET
	const VRP_PP_EVALUATION& get_evaluation()const;							// returns the corresponding VRP_PP_EVALUATION object
	int get_index_of_a_non_empty_route()const;								// return the index of a non-empty route
	int get_operation_ctr()const;											// returns the number of operations within the plan
	int get_route_ctr()const;												// return the number of routes within the plan (empty + non-empty)
	VRP_PP_ROUTE* get_route_by_index(int index);							// returns route at provided index (in case of invalid index = throws an exception)
	VRP_PP_ROUTE*get_route_by_index(int index)const;

    // returns the index of a randomly selected non-empty route from the plan (added by JSB, 23.06.2019)
	int get_index_of_a_non_empty_route(void);

	// MODIFY
	void evaluate(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType = VRP_PP_ARC_WEIGHT::EUCLID);

	void add_route(VRP_PP_ROUTE*r);
	void clear();
    void establish_loading_lists(struct VRP_PP_PROBLEM *prob);
    void empty_loading_lists(void);
    void remove_loading_lists(void);

	// CONSTRUCTION HEURISTICS
	static VRP_PP_PLAN* generate_random_plan(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType = VRP_PP_ARC_WEIGHT::EUCLID);								// creates a randomly generated vehicle route set
	static VRP_PP_PLAN* generate_random_plan_capacitated(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType = VRP_PP_ARC_WEIGHT::EUCLID);					// creates a randomly generated vehicle route set using non-standard operation constructors and respecting limited vehicle capacity
	// static VRP_PP_PLAN* generate_random_neighbor(const VRP_PP_PROBLEM *prob, VRP_PP_PLAN& origin, const VRP_PP_ARC_WEIGHT& weigthType = VRP_PP_ARC_WEIGHT::EUCLID);

    // JSB
    static VRP_PP_PLAN* generate_sweep_algorithm(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType);
    static VRP_PP_PLAN* generate_savings_algorithm(VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType);

    // loading plan filling for different scenarios
    void fill_loading_lists_VRPtype(struct VRP_PP_PROBLEM *prob);

    // try to identify deployed arcs in the routes
    void fill_used_arcs_unique(struct VRP_PP_PROBLEM *prob);

    // remove all used arc informations
    void remove_used_arcs(void);

    // IMPROVEMENT HEURISTICS (added by JSB)
    void two_opt_full(struct VRP_PP_PROBLEM *_P, const VRP_PP_ARC_WEIGHT& weigthType = VRP_PP_ARC_WEIGHT::EUCLID);       // added by JSB 16.06.2019
    // shifts a randomly selected operation from a randomly selecte dinto into another randomly selectd route at a randomly selected position
	void apply_random_move(struct VRP_PP_PROBLEM *_P, const VRP_PP_ARC_WEIGHT& weigthType = VRP_PP_ARC_WEIGHT::EUCLID);

	// not contained in the distributed version
	// this is the pure Dijkstra-Algorithm (added by JSB 26.04.2020 15:14)
	void SPP_dijkstra_algorithm(class VRP_PP_PROBLEM *_P, struct VRP_PP_CONFIG *_CFG);
    void TSP_random_path(const class VRP_PP_PROBLEM *_P, struct VRP_PP_NODE StartNode);
    void CVRP_generate_random_plan(const VRP_PP_PROBLEM *prob);
    void print_for_excel(const VRP_PP_PROBLEM *prob);
    void CVRP_two_opt_full(struct VRP_PP_PROBLEM *_P, const VRP_PP_ARC_WEIGHT& weigthType);
    void CVRP_generate_random_plan_capacitated(const VRP_PP_PROBLEM *prob);

    void print_for_xml_export(std::ostream& stream, const VRP_PP_PROBLEM*prob);

    // chapter linear programming for fleet deployment
    void SPP_LP_SOLVE(const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG);
    void TSP_LP_SOLVE(const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *CFG);
    void CVRP_LP_SOLVE(const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *CFG);
    void DCPP_LP_SOLVE(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType);

    // chapter analysis of plans
    void print_gantt_chart(std::ostream& stream, const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG);
    void print_3D(std::ostream& stream, const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG, int *_DISPLAYED);

    // to be removed for distribution
    void VRPTW_FIRST_ALGORITHM(const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG);
    void VRPTW_FIRST_ALGORITHM2(const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG);
    void VRPTW_SIMPLE_HEURISTIC(VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG);

    // local search
    void simple_ls(VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType);

    // application of the OSM-twin
    #ifdef VRP_PP_OSM
    #pragma message "vrp++ osm components in vrp_pp_plan.h are considered!"
    void print_as_kml(std::ostream& stream, const VRP_PP_PROBLEM*prob, class VRP_PP_OSM_TWIN *_TWIN, bool PrintBubbles);
    void print_as_kml(std::ostream& stream, const VRP_PP_PROBLEM*prob, class VRP_PP_OSM_TWIN *_TWIN);
    void print_as_kml_with_nodes(std::ostream& stream, const VRP_PP_PROBLEM*prob, class VRP_PP_OSM_TWIN *_TWIN);
    #else
    #endif // VRP_PP_OSM

private:
	// these are the attributes
	int routes;                             // number of routes in the plan object
	int operations;                         // number of contained operations
	VRP_PP_EVALUATION eval_result;			// reference to the evaluation array
	VRP_PP_ROUTE **route;                   // reference to an array of vehicle routes

};



#endif
