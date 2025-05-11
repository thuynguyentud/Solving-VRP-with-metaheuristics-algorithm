#ifndef VRP_PP_OPERATION_H
#define VRP_PP_OPERATION_H

// version 2.0 - last modification 25.02.2020 at 19:09

#include "vrp_pp_global.h"
#include "vrp_pp_loading_list.h"
#include "vrp_pp_used_arc.h"

using namespace std;

class VRP_PP_OPERATION {

    friend class VRP_PP_LOADING_LIST;
    friend class VRP_PP_USED_ARC;
    friend class VRP_PP_ROUTE;

public:
	// request-specific information
	int id;				            // this id corresponds either to a depot or a request
	int id_node;					// redundant but makes lives simpler
	int type;			            // type of operation (VRP_PP_START, VRP_PP_STOP, VRP_PP_PICKUP, VRP_PP_DELIVERY)
	string desc;				// describes the requests
	long long id_route;				// determines in which route this operation is contained


	// scheduling information
	double reachtime;	            // arrival time of vehicle at this location
	double starttime;	            // time vehjcle starts being active at this location
	double finishtime;	            // duty completion time of serving vehicle at this node

	VRP_PP_OPERATION *next;        // referece to subsequent operation
	VRP_PP_OPERATION *previous;    // reference to predecessor operation

	VRP_PP_LOADING_LIST *ob_ll;
	VRP_PP_LOADING_LIST *ib_ll;
                                    // reference to a loading list applied after completing operation
    VRP_PP_USED_ARC *out_arc;
    VRP_PP_USED_ARC *in_arc;
                                    // reference to the arcs connecting this operation with its predecessor
                                    // as well as successor operation (if there is one)
	//CONSTRUCT & COPY
public:
	// this is the standard constructor
	VRP_PP_OPERATION();
	// this is the standard destructor
	~VRP_PP_OPERATION();
	// copy constructor - does NOT copy next and previous ptrs
	VRP_PP_OPERATION(const VRP_PP_OPERATION& source);
	// initialize from corresponding request and node
	VRP_PP_OPERATION(int id, int id_node, int type, const std::string& desc = "not yet defined");
	// a simple print procedure
	void print(void);
    void print_for_xml_export(std::ostream& stream, const VRP_PP_PROBLEM*prob);

	// connect with the outbound loading list
	void link_to_outbound_loading_list(class VRP_PP_LOADING_LIST *PtrToLoadingList);
	// connect with the inbound loading list
	void link_to_inbound_loading_list(class VRP_PP_LOADING_LIST *PtrToLoadingList);
	// uncouple Loading list from operation
	void uncouple_outbound_loading_list(class VRP_PP_LOADING_LIST *PtrToLoadingList);
	void uncouple_inbound_loading_list(class VRP_PP_LOADING_LIST *PtrToLoadingList);

	// GET
    int get_id(void);
	int get_id_node(void);
	int get_type(void);
	string get_desc(void);
	long long get_id_route(void);
	double get_reachtime(void);
	double get_starttime(void);
	double get_finishtime(void);
	class VRP_PP_OPERATION *get_next(void);
	class VRP_PP_OPERATION *get_previous(void);
	class VRP_PP_LOADING_LIST *get_ob_ll(void);
	class VRP_PP_LOADING_LIST *get_ib_ll(void);
    class VRP_PP_USED_ARC *get_out_arc(void);
    class VRP_PP_USED_ARC *get_in_arc(void);

    // SET
    void set_next(class VRP_PP_OPERATION *Ptr);
    void set_previous(class VRP_PP_OPERATION *Ptr);

    //
};





#endif
