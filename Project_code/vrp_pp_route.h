// first check if microsoft visual studio is used

#ifndef VRP_PP_ROUTE_H
#define VRP_PP_ROUTE_H

// version 2.0 - last_modification 25.02.2020 at 18:55

#include "vrp_pp_operation.h"

// predefintion
class VRP_PP_PROBLEM;
class VRP_PP_PLAN;

class VRP_PP_ROUTE
{
	friend class VRP_PP_PLAN;	// you can do anything to a route within a VRP_PP_PLAN method
    friend class VRP_PP_LOADING_LIST;

public:
	// CREATE, COPY, ASSIGN and DESTROY
	VRP_PP_ROUTE();											// default constructor - required for creating arrays
	VRP_PP_ROUTE(const VRP_PP_VEHICLE& vehicle);			// standard constructor
	VRP_PP_ROUTE(const VRP_PP_ROUTE& source);				// copy constructor (e.g. VRP_PP_ROUTE r(source) or VRP_PP_ROUTE r = VRP_PP_ROUTE(source), respectively
	VRP_PP_ROUTE& operator=(const VRP_PP_ROUTE&source);		// overloaded assignment operator (e.g. VRP_PP_ROUTE r = r2; or r = r2; - assuming that r and r2 are of type VRP_PP_ROUTE)
	VRP_PP_ROUTE(int _route_id, VRP_PP_NODE _start, VRP_PP_NODE _stop); // OD-path constructor using given start and given path end

	~VRP_PP_ROUTE();										// this is the destructor of this object (never call yourself! - the compiler will)


	// PRINT
	void print_as_operation_table(const VRP_PP_PROBLEM *prob)const;		// prints the route line-by-line
	void print_as_node_sequence(const VRP_PP_PROBLEM *prob)const;		// compact one line printing of route
    void print_as_alternating_sequence(const VRP_PP_PROBLEM *prob)const; // includes used_arcs

	// GET
	long long get_route_id()const;										// returns the unique route id of the current thread
	int get_op_ctr()const;												// returns the number of operations within the route
	int get_vehicle_num()const;											// returns the id of the corresponding vehicle
	const VRP_PP_EVALUATION& get_evaluation()const;						// returns a ptr to a random operations within the route (returns NULL if route is empty)
	VRP_PP_OPERATION *get_ptr_to_arb_op(void);							// returns a reference to an arbitrarily selected operation
	VRP_PP_OPERATION*get_ptr_to_op(int index);							// returns a ptr to the operations at route index i (throws an exception if index is out of bound)
	const VRP_PP_OPERATION*get_ptr_to_op(int index)const;				// returns a ptr to the operations at route index i (throws an exception if index is out of bound)
	VRP_PP_OPERATION *get_start_depot();
	const VRP_PP_OPERATION*get_start_depot()const;						// returns a ptr to start operation (=depot)
	const VRP_PP_OPERATION*get_end_depot()const;						// returns a ptr to end operation (=depot)
	VRP_PP_OPERATION*get_end_depot();
	bool is_predec(const VRP_PP_OPERATION *a, const VRP_PP_OPERATION *b)const;		// checks if operation A is predecessor of operation B

	// MODIFY
	void clear(void);																							// deletes all operations within the route and resets op_ctr and eval_result attribute
	void init(const VRP_PP_VEHICLE *vehicle);		// initialization of this object specifying start depot as well as end depot (required due to initializion of array elements)																		// removes all user-inserted operations
	void append_route(VRP_PP_ROUTE *append);																	// concatenate this with append (attach route R_APP behind route this)
	void insert_between(VRP_PP_OPERATION *insert, VRP_PP_OPERATION *predecessor, VRP_PP_OPERATION *successor);	// inserts operation n between operation p1 and operation p2
	void insert_at_rand_pos_after(VRP_PP_OPERATION *refPosition, VRP_PP_OPERATION *insert);						// inserts Ptr somewhere between RefPtr and the end
	void insert_at_rand_pos_before(VRP_PP_OPERATION *refPosition, VRP_PP_OPERATION *insert);					// inserts Ptr somewhere between start and refPosition
	void dissolve_operation(VRP_PP_OPERATION *dissolve);														// releases operation dissolve from route and keeps it alive
	void insert_at_rand_pos(VRP_PP_OPERATION *insert);															// inserts operation Ptr at an arbitrarily selected position in this route
	void invert_from_to(VRP_PP_OPERATION *from, VRP_PP_OPERATION *to);											// inverts op sequence between FROM and TO (inluding both)
	void evaluate(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType = VRP_PP_ARC_WEIGHT::EUCLID);
																// evaluates this route (update VRP_PP_EVALUATON object and all VRP_PP_OPERATIONS within)
    void two_opt_route_full(struct VRP_PP_PROBLEM *P, const VRP_PP_ARC_WEIGHT& weigthType = VRP_PP_ARC_WEIGHT::EUCLID);     // applies 2-opt-procedure to the route

    void establish_loading_lists(struct VRP_PP_PROBLEM *P);                                                     // attaches empty loading lists along the route
    void empty_loading_lists(void);                                                                             // empties all loading lists along this route
    void fill_loading_lists_VRPtype(struct VRP_PP_PROBLEM *prob);                                               // fills the loading lists
    void assign_used_arcs_unique(struct VRP_PP_PROBLEM *prob);                                                  // fills the used arc structures with the first found arc
    void remove_loading_lists(void);                                                                            // first empties then destroys al loading list structures
    void remove_used_arcs(void);                                                                                // destroys all used_arc structures

    void print_for_xml_export(std::ostream& stream, const VRP_PP_PROBLEM*prob);
    //chapter 4
    void CVRP_evaluate(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType);
    void CVRP_two_opt_route_full(struct VRP_PP_PROBLEM *P, const VRP_PP_ARC_WEIGHT& weigthType);

//private:
	VRP_PP_OPERATION *start;				// refers to the request-independent start-operation of a route
	VRP_PP_OPERATION *end;					// refers to the request-independent terminating-operation of a route
	int type;								// defines the type of a route
	int vehicle_num;						// refers to the vehicles that is assigned to this route
	int op_ctr;                             // counter of operations within the route
	VRP_PP_EVALUATION eval_result;			// evaluation results object attached to this
	long long route_id;						// each route created during the program run will automatically get a unique incremental id (thread internal) during construction

	static thread_local long long route_ctr;		// identifier - dont worry i take care of it
};


#endif
