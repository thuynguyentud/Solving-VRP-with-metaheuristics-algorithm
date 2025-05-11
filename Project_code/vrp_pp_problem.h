// first check if microsoft visual studio is used
#ifndef VRP_PP_PROBLEM_H
#define VRP_PP_PROBLEM_H

//#include <stdio.h>
//#include <stdlib.h>
//#include <string>
//#include <math.h>
//#include <vector>
//#include "vrp_pp_plan.h"
//#include "vrp_pp_route.h"

// Version 2.0 - last modification 25.02.2020 at 17:48

#include "vrp_pp_global.h"
#include "vrp_pp_load_item.h"

#include <vector>

class VRP_PP_LOAD_ITEM;

class VRP_PP_PROBLEM
{
	friend class VRP_PP_PLAN;
	friend class VRP_PP_ROUTE;
	friend class VRP_PP_LOAD_ITEM;

public:
	// CREATE,COPY,ASSIGN & DESTROY
	VRP_PP_PROBLEM(const std::string& filename);				// read CVRP data from XML-file
	VRP_PP_PROBLEM(const VRP_PP_PROBLEM&source);				// copy constructor
	VRP_PP_PROBLEM& operator=(const VRP_PP_PROBLEM& source);	// overloaded assignment operator
	~VRP_PP_PROBLEM();											// this is the objects destructor - releases memory

	// PRINT & EXPORT
	void print(void);											// print problem data on screen
	void print_as_kml(std::ostream& stream)const;				// prepares an output in the kml scheme
    void print_nodes_as_kml(std::ostream& stream)const;       // print the network nodes in the kml scheme
    void print_network_as_kml(std::ostream& stream)const;       // print the network (nodes and arcs) in the kml scheme
	// GET
	int get_vehicle_ctr()const;
	int get_depot_ctr()const;
	int get_node_ctr()const;
	int get_request_ctr()const;
	int get_arc_ctr()const;				// DEVELOPER COMMENT: im Beispiel sind es arcs=nodes*nodes -> mgl. Änderungen in Zukunft beim Einlesen erforderlich
	int get_load_item_ctr()const;

	double get_dist_euclid(int node1_index, int node2_index)const;
	double get_dist_latlong(int node1_index, int node2_index)const;
	double get_dist_custom(int node1_index, int node2_index)const;

    void set_dist_euclid(double value, int node1_index, int node2_index);
	void set_dist_latlong(double value, int node1_index, int node2_index);
	void set_dist_custom(double value, int node1_index, int node2_index);

	const VRP_PP_ARC& get_arc_by_od(int origin, int dest)const;		// returns the arc from OD-Matrix (ARC[origin][dest]) - throws an exception if any index is out of bound
	const VRP_PP_ARC& get_arc_by_index(int index)const;				// returns arc[index] - throws an exception if index is out of bound
	const VRP_PP_NODE& get_node_by_index(int index)const;			// returns node[index] - throws an exception if index is out of bound
	const VRP_PP_REQUEST& get_request_by_index(int index)const;		// returns request[index] - throws an exception if index is out of bound
	const VRP_PP_REQUEST& get_depot_by_index(int index)const;		// returns depot[index] - throws an exception if index is out of bound
	const VRP_PP_VEHICLE& get_vehicle_by_index(int id)const;		// returns vehicle[id] - throws an exception if index is out of bound
    const VRP_PP_LOAD_ITEM& get_load_item_by_index(int id)const;	// returns load_item[id]
	void clear();					// private since there a no public refill functions - only required for destructions and assignments

	// manage
	void append_vehicle(VRP_PP_VEHICLE *veh);
    void delete_last_vehicle(void);

    // added 05.06.2020
    void update_request_data(int Request_Id, struct VRP_PP_REQUEST _ReqData);
    // added 27.03.2022 (V4.00)
    void update_arc_data(int Arc_Id, struct VRP_PP_ARC _ArcData);
    // added 04.04.2022 (V4.00)
    void update_node_data(int Node_Id, struct VRP_PP_NODE _NodeData);

    // added 16.06.2020
    void set_selfloops_zero(void);

    // added 08.03.2021
    void re_index_arcs(void);

    // added 09.03.2021
    const int get_request_id_by_node_id(int node_id)const;		// returns request associated with node indexed by node_id

    // added 22.05.2021
    void print_custom_arclength_as_xml(void);

    // Code Thuy adds
    void read_fuel_consumption_values(struct VRP_PP_CONFIG *_Config);


    // set 3 attributes (nbh_size, P1, P2)
    void set_3_var(int nbh_size, int P1, int P2);

    // get 3 added attributes (P1, P2)
    int get_nbh_size(void);
    int get_P1(void);
    int get_P2(void);

private:
	// attributes of object
	int vehicles;					// number of available vehicles
	int depots;						// number of depots
	int nodes;						// number of nodes
	int requests;					// number of requests
	int arcs;						// number of arcs
	int load_items;                 // number of load items in the scenario

	//code Thuy adds - new algorithm configuration
	int nbh_size;                   // size of nbh
	int P1;                         // a penalty-value for each overloaded vehicle
    int P2;                         // a penalty-value for each overloaded pallet

	// the attributes itself
	VRP_PP_VEHICLE*vehicle;			// pointer to array of vehicles 0,...,vehicles-1
	VRP_PP_REQUEST *depot;			// pointer to array of depots 0,...,depots-1
	VRP_PP_NODE*node;				// pointer to array of nodes 0,...,nodes-1
	VRP_PP_REQUEST*request;			// pointer to array of requests 0,...,requests-1
	VRP_PP_ARC* arc;				// pointer to array of arcs 0,...,arcs-1
	VRP_PP_ARC** ARC;				// root of the distance matrice
    VRP_PP_LOAD_ITEM *load_item;    // pointer to array of load_items 0,...,load_items-1

	void assert_node_index(int index)const;		// throws an excetion if index is out of bound
};


#endif
