#include "vrp_pp_plan.h"
#include <vector>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <vector>
#include <cmath>
#include "vrp_pp_nbh.h"
#include <iomanip>
#include <cstring>
#include <stdlib.h>
#include "irrXML.h"

using namespace std;
using namespace irr; // irrXML is located in the namespace irr::io
using namespace io;

// needed to inform the compiler which external software can be expected to be installed
#include "vrp_pp_extern.h"

// version 2.0 - last modification 25.02.2020 at 18:27
// version 2.1 - last modification 18.04.2020 at 10:26 - removal of vrp_pp_objfoo-h include statement
// version 3.0 - last modification 12.03.2021 at 12:47 - addition of #define "vrp_pp_lp.h" statement

#ifdef VRP_PP_SOLVER
    #pragma message "lp_solve components are considered"
    #include "vrp_pp_lp.h"
    #include "lp_lib.h"
#else
    #pragma message "lp_solve components are not considered"
#endif // VRP_PP_SOLVER

#ifdef VRP_PP_OSM
    #pragma message "vrp++ osm components are considered"
    #include "vrp_pp_osm.h"
#else
    #pragma message "vrp++ osm components are not considered"
#endif // VRP_PP_OSM

VRP_PP_PLAN::~VRP_PP_PLAN()
{
	clear();
}


VRP_PP_PLAN::VRP_PP_PLAN()
	: routes(0),route(NULL), operations(0), eval_result(VRP_PP_EVALUATION())
{
}

VRP_PP_PLAN::VRP_PP_PLAN(const VRP_PP_PROBLEM *prob)
{
	// it sets the global values for a complete plan and initializes the routes of the available vehicles

	// allocate memory for the eval array
	this->eval_result = VRP_PP_EVALUATION();
	operations = 0;

	// allocate memory for the route routes
	this->routes = prob->vehicles;
	this->route = new VRP_PP_ROUTE*[prob->vehicles];

	// init vehicle routes / pathes
	for (int i = 0; i < prob->vehicles; i++)
	{
		this->route[i] = new VRP_PP_ROUTE(prob->vehicle[i]);/*, &P->node[0], &P->node[0]);*/
	}

}

VRP_PP_PLAN::VRP_PP_PLAN(const VRP_PP_PLAN & source)//: routes(source.routes), operations(source.operations), route(new VRP_PP_ROUTE*[routes]), eval_result(source.eval_result)
{
    this->routes = source.routes;
    this->operations = source.operations;
    this->eval_result = source.eval_result;
    this->route = new class VRP_PP_ROUTE*[this->routes];

	for (int i = 0; i < this->routes; i++)
	{
	    //cout << "ROUTE " << i << endl;
		this->route[i] = new VRP_PP_ROUTE(*source.route[i]);
	}
}

VRP_PP_PLAN & VRP_PP_PLAN::operator=(const VRP_PP_PLAN & source)
{
	if (&source == this)
		return *this;

	clear();

	operations = source.operations;
	routes = source.routes;
	eval_result = source.eval_result;

	route = new VRP_PP_ROUTE*[routes];
	for (int i = 0; i < routes; i++)
	{
	    //cout << "COPYCON ROUTE i=" << i << endl;
		route[i] = new VRP_PP_ROUTE(*source.route[i]);
	}


	return *this;
}


void VRP_PP_PLAN::print_as_operation_tables(const VRP_PP_PROBLEM *prob)const
{
	// this procedure prints the most important information associated with a plan
	// -> the routes (if already determined, otherwise empty routes)
	// -> evaluation results (if plan has been evaluated before)

	using namespace std;

	for (int i = 0; i < this->routes; i++)
	{
		cout << "Route " << i << endl;
		this->route[i]->print_as_operation_table(prob);
		cout << endl;
	}

	std::cout << eval_result;
}




void VRP_PP_PLAN::print_short(const VRP_PP_PROBLEM *prob)const
{
	int i;
	class VRP_PP_OPERATION *lauf;

	using namespace std;

	cout << "this plan consists of " << this->routes << " path(es)." << endl;
	cout << "only the operation sequence is shown" << endl;

	for (i = 0; i < this->routes; i++)
	{
		lauf = this->route[i]->start;
        cout << "---------------------------------------" << endl;

		cout << "path of vehicle " << i << " (avail. capacity: " << prob->get_vehicle_by_index(route[i]->vehicle_num).cap << ")" << endl;

		while (lauf != NULL)
		{
			cout << "-> operation (" << lauf->get_id() << "): ";
			cout << "[at node with id " << lauf->get_id_node();
			cout << ", arrival time " << lauf->get_reachtime() << "]";
			cout << endl;
            lauf = lauf->get_next();
		}
		cout << "---------------------------------------" << endl;
		cout << endl;
	}

}

void VRP_PP_PLAN::print_long(const VRP_PP_PROBLEM *prob)const
{
	int i;
	class VRP_PP_OPERATION *lauf;

	using namespace std;

	cout << "this plan consists of " << this->routes << " path(es)." << endl;
	cout << "used arcs are shown if specified before" << endl;

	for (i = 0; i < this->routes; i++)
	{
		lauf = this->route[i]->start;
        cout << "---------------------------------------" << endl;
		cout << "path of vehicle " << i << " (avail. capacity: " << prob->get_vehicle_by_index(route[i]->vehicle_num).cap << ")" << endl;

		while (lauf != NULL)
		{
			cout << "-> operation (" << lauf->get_id() << "): ";
			cout << "[at node with id " << lauf->get_id_node();
			cout << ", arrival time " << lauf->get_reachtime() << "]";
			cout << endl;

			if( lauf->get_next() != NULL )
            {
                cout << "jump along arc with id: ";
                if( lauf->get_out_arc() != NULL )
                    cout << " " << lauf->get_out_arc()->get_arc_id() << endl;
                else
                    cout << "no used arc specified here" << endl;
            }
            lauf = lauf->get_next();
		}
		cout << "---------------------------------------" << endl;
		cout << endl;
	}

}

void VRP_PP_PLAN::print_very_long(const VRP_PP_PROBLEM *prob)const
{
	int i;
	class VRP_PP_OPERATION *lauf;

	using namespace std;

	for (i = 0; i < this->routes; i++)
	{
		lauf = this->route[i]->start;

		cout << "vehicle " << i << " (avail. capacity: " << prob->get_vehicle_by_index(route[i]->vehicle_num).cap << ")" << endl;

		while (lauf != NULL)
		{
			const VRP_PP_REQUEST *request;
            cout << "---------------------------------------" << endl;
			if (lauf->get_type() == VRP_PP_START || lauf->get_type() == VRP_PP_STOP)
			{
				//request = &prob->get_depot_by_index(lauf->get_id());
				cout << "dummy operation at depot " << endl;
			}
			else
			{
				request = &prob->get_request_by_index(lauf->get_id());
				cout << "operation associated with customer request " << request->id << endl;
			}

            cout << "-> operation (" << lauf->get_id() << "): ";
			cout << "[reachtime = " << lauf->get_reachtime() << "]";
			cout << endl;

			if( lauf->get_ib_ll() != NULL )
            {
                cout << "inbound loading list" << endl;
                lauf->get_ib_ll()->print();
            }
            else
                cout << "no inbound loading list" << endl;

			if( lauf->get_next() != NULL )
            {
                cout << "jump along arc: ";
                if( lauf->get_out_arc() != NULL )
                    cout << " " << lauf->get_out_arc()->get_arc_id() << endl;
                else
                cout << "no used arc specified here" << endl;

            }
			if( lauf->get_ob_ll() != NULL)
            {
                cout << "outbound loading list" << endl;
                lauf->get_ob_ll()->print();
            }
            else
                cout << "no outbound loading list" << endl;

            lauf = lauf->get_next();
			cout << "---------------------------------------" << endl;

		}
		cout << endl;
	}

}
void VRP_PP_PLAN::print_as_node_sequences(const VRP_PP_PROBLEM *prob)const
{
    // eher nicht nutzen
	using namespace std;

	int i;
	VRP_PP_OPERATION *lauf;

	for (i = 0; i < this->routes; i++)
	{
		lauf = this->route[i]->start;

		cout << "vehicle " << i << "("<<this->route[i]->get_route_id()<<"): ";

		while (lauf != NULL)
		{
			int node_id = -1;
			if (lauf->get_type() == VRP_PP_START)
			{
				node_id = prob->get_depot_by_index(lauf->get_id()).id_node;
				cout << "START(depot=" << lauf->get_id() << "|node="<<node_id;
				if( lauf->get_previous() != nullptr )
                                cout << "p" << lauf->get_previous()->get_id();
                            else
                                cout << "p-";
                            if( lauf->get_next() != nullptr )
                                cout << "n" << lauf->get_next()->get_id();
                            else
                                cout << "n-";
			}
			else
                if (lauf->get_type() == VRP_PP_STOP)
                {
                    node_id = prob->get_depot_by_index(lauf->get_id()).id_node;
                    cout << "->STOP(depot=" << lauf->get_id() << "|node="<<node_id;
                    if( lauf->get_previous() != nullptr )
                                cout << "p" << lauf->get_previous()->get_id();
                            else
                                cout << "p-";
                            if( lauf->get_next() != nullptr )
                                cout << "n" << lauf->get_next()->get_id();
                            else
                                cout << "n-";
                }
                else
                {
                    if (lauf->get_type() == VRP_PP_TRANSIT)
                    {
                        // we have a node not connected to a request or depot
                        node_id = lauf->id_node;
                        cout << "->TRANSIT(node="<<node_id;
                        if( lauf->get_previous() != nullptr )
                                cout << "p" << lauf->get_previous()->get_id();
                            else
                                cout << "p-";
                            if( lauf->get_next() != nullptr )
                                cout << "n" << lauf->get_next()->get_id();
                            else
                                cout << "n-";
                    }
                    else
                    {
                        // we have a node associated with a request
                        if (lauf->get_type() == VRP_PP_PICKUP)
                        {
                            // we have a pickup operation
                            node_id = prob->get_request_by_index(lauf->get_id()).id_node;
                            cout << "->PICKUP(request=" << lauf->get_id() << "|node="<<node_id;
                            if( lauf->get_previous() != nullptr )
                                cout << "p" << lauf->get_previous()->get_id();
                            else
                                cout << "p-";
                            if( lauf->get_next() != nullptr )
                                cout << "n" << lauf->get_next()->get_id();
                            else
                                cout << "n-";
                        }
                        else
                        {
                            // we have a delivery operation
                            node_id = prob->get_request_by_index(lauf->get_id()).id_node;
                            cout << "->DELIVERY(request=" << lauf->get_id() << "|node="<<node_id;
                            if( lauf->get_previous() != nullptr )
                                cout << "p" << lauf->get_previous()->get_id();
                            else
                                cout << "p-";
                            if( lauf->get_next() != nullptr )
                                cout << "n" << lauf->get_next()->get_id();
                            else
                                cout << "n-";
                       }
                    }
                }

			cout << ")";
			lauf = lauf->get_next();
		}

		cout << "Number of Operations: " << this->route[i]->get_op_ctr() << endl;
		cout << endl;
	}

}


VRP_PP_PLAN* VRP_PP_PLAN::generate_random_plan(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType)
{
	int v;

	// This procedure first generates an operation for each delivery request.
	// In a second step it randomly distributes these operations among the available
	// vehicles and inserts the operations at random positions in their routes.
	// VRP_PP_TRACE trace(__FUNCTION__);
	// assert_pointer(prob);

	VRP_PP_PLAN*randPlan = new VRP_PP_PLAN(prob);

	VRP_PP_OPERATION *OP;
	for (int i = 0; i < prob->requests; i++)
	{
		// acquire memory
		OP = new VRP_PP_OPERATION(prob->request[i].id, prob->request[i].id_node, prob->request[i].type, "Request " + std::to_string(i));

		// second, select a vehicle v at random
		v = rand() % prob->vehicles;

		// third, insert the operation at a random position in route of vehicle v
		randPlan->get_route_by_index(v)->insert_at_rand_pos(OP);
	}

	randPlan->evaluate(prob, weigthType);
	return randPlan;
}


VRP_PP_PLAN* VRP_PP_PLAN::generate_random_plan_capacitated(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType)
{



	return NULL;
}


const VRP_PP_EVALUATION & VRP_PP_PLAN::get_evaluation() const
{
	return eval_result;
}

void VRP_PP_PLAN::evaluate(const VRP_PP_PROBLEM *P, const VRP_PP_ARC_WEIGHT& weigthType)
{
	// this procedure collects the evaluation results from all routes
	// and aggregates them by summing

	// reset the evaluation variables
	eval_result = VRP_PP_EVALUATION();
	eval_result.feasible = 1;
	operations = 0;
	for (int v = 0; v < this->routes; v++)
	{
		this->route[v]->evaluate(P, weigthType);
		//cout << "Length=" << this->route[v].eval_result->length << endl;
		eval_result += route[v]->get_evaluation();
		//cout << "objective value of route " << route[v]->vehicle_num << ": " << route[v]->get_evaluation().obj_value << endl;
		operations += route[v]->get_op_ctr();
        if( this->route[v]->get_evaluation().feasible < 0 )
            this->eval_result.feasible = -1;
	}
	std::cout << "\n objective value of 1 plan: " << eval_result.obj_value;
}

void VRP_PP_PLAN::print_as_kml_with_nodes(std::ostream& stream, const VRP_PP_PROBLEM*prob)const
{
	// VRP_PP_TRACE trace(__FUNCTION__);
	// assert_pointer(prob);

	using namespace std;
	// declare auxiliary variables
	VRP_PP_OPERATION *lauf;

	// print the xml-header
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
	stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << "\n";
	stream << "<Document>" << "\n";

	// first print all locations
	for (int r = 0; r < this->routes; r++)
	{
		if (this->route[r]->start->get_next() != this->route[r]->end)
		{
            // non-empty route found
			lauf = this->route[r]->start;
			while (lauf->get_next() != NULL)
			{
				const VRP_PP_NODE*node;

				stream << "<Placemark>" << "\n";
				if (lauf->get_type() == VRP_PP_START || lauf->get_type() == VRP_PP_STOP)
				{
					node = &prob->get_node_by_index(prob->get_depot_by_index(lauf->get_id()).id_node);
					stream << "<name>depot " << lauf->get_id() << "</name>" << "\n";
				}
				else
				{
					stream << "<name>request " << lauf->get_id() << "</name>" << "\n";
					node = &prob->get_node_by_index(prob->get_request_by_index(lauf->get_id()).id_node);
				}

				stream << "<description>" << lauf->get_desc() << "</description>" << "\n";
				stream << "<Point>" << "\n";
				stream << "<coordinates>" << node->longitude << "," << node->lattitude<< "," << 0 << "</coordinates>" << "\n";
				stream << "</Point>" << "\n";
				stream << "</Placemark>" << "\n";
				lauf = lauf->get_next();
			}
		}
	}

	// define 10 different line colors
	static string color[] = {
		"000000",    // black
		"191970",    // blue
		"FF0000",    // red
		"00FF00",    // green
		"EE00EE",    // oliv
		"00CED1",    // turquois
		"DAA520",    // gold
		"FFC1C1",    // orange
		"0000FF",    // pink
		"00F000",    // yellow
	};

	// now print the lines
	for (int r = 0; r < this->routes; r++)
	{
		if (this->route[r]->start->get_next() != this->route[r]->end)
		{
			stream << "<Placemark>" << "\n";
			//stream << "<Placemark>" << "\n";
			stream << "<name>route - ID" << this->route[r]->route_id << "</name>" << "\n";
			stream << "<description>route of vehicle " << this->route[r]->vehicle_num << "</description>" << "\n";
			stream << "<Style>" << "\n";
			stream << "<LineStyle>" << "\n";
			stream << "<color>" << color[r % int(color->size())] << "</color>" << "\n";
			stream << "<width>3</width>" << "\n";
			stream << "</LineStyle>" << "\n";
			stream << "</Style>" << "\n";
			stream << "<LineString>" << "\n";
			stream << "<extrude>1</extrude>" << "\n";
			stream << "<tessellate>1</tessellate>" << "\n";
			stream << "<altitudeMode>absolute</altitudeMode>" << "\n";
			stream << "<coordinates>" << "\n";
			lauf = this->route[r]->start;
			while (lauf != NULL)
			{
				stream << prob->get_node_by_index(lauf->get_id_node()).longitude << "," << prob->get_node_by_index(lauf->get_id_node()).lattitude<< "," << 0 << "\n";
				lauf = lauf->get_next();
			}
			stream << "</coordinates>" << "\n";
			stream << "</LineString>" << "\n";
			stream << "</Placemark>" << "\n";
		}
	}


	// print the xml-footer
	stream << "</Document>" << "\n";
	stream << "</kml>" << "\n";

	stream.flush();
}

void VRP_PP_PLAN::print_as_kml(std::ostream& stream, const VRP_PP_PROBLEM*prob)const
{
	// VRP_PP_TRACE trace(__FUNCTION__);
	// assert_pointer(prob);

	using namespace std;
	// declare auxiliary variables
	VRP_PP_OPERATION *lauf;

	// print the xml-header
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
	stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << "\n";
	stream << "<Document>" << "\n";

	// define 10 different line colors
	static string color[] = {
		"000000",    // black
		"191970",    // blue
		"FF0000",    // red
		"00FF00",    // green
		"EE00EE",    // oliv
		"00CED1",    // turquois
		"DAA520",    // gold
		"FFC1C1",    // orange
		"0000FF",    // pink
		"00F000",    // yellow
	};

	// now print the lines
	for (int r = 0; r < this->routes; r++)
	{
		if (this->route[r]->start->get_next() != this->route[r]->end)
		{
			stream << "<Placemark>" << "\n";
			//stream << "<Placemark>" << "\n";
			stream << "<name>route - ID" << this->route[r]->route_id << "</name>" << "\n";
			stream << "<description>route of vehicle " << this->route[r]->vehicle_num << "</description>" << "\n";
			stream << "<Style>" << "\n";
			stream << "<LineStyle>" << "\n";
			stream << "<color>" << color[r % int(color->size())] << "</color>" << "\n";
			stream << "<width>3</width>" << "\n";
			stream << "</LineStyle>" << "\n";
			stream << "</Style>" << "\n";
			stream << "<LineString>" << "\n";
			stream << "<extrude>1</extrude>" << "\n";
			stream << "<tessellate>1</tessellate>" << "\n";
			stream << "<altitudeMode>absolute</altitudeMode>" << "\n";
			stream << "<coordinates>" << "\n";
			lauf = this->route[r]->start;
			while (lauf != NULL)
			{
				stream << prob->get_node_by_index(lauf->get_id_node()).longitude << "," << prob->get_node_by_index(lauf->get_id_node()).lattitude<< "," << 0 << "\n";
				lauf = lauf->get_next();
			}
			stream << "</coordinates>" << "\n";
			stream << "</LineString>" << "\n";
			stream << "</Placemark>" << "\n";
		}
	}


	// print the xml-footer
	stream << "</Document>" << "\n";
	stream << "</kml>" << "\n";

	stream.flush();
}

#ifdef VRP_PP_OSM
    #pragma message "vrp++ osm components in vrp_pp_plan.ccp are considered!"
void VRP_PP_PLAN::print_as_kml(std::ostream& stream, const VRP_PP_PROBLEM*prob, class VRP_PP_OSM_TWIN *_TWIN, bool PrintBubbles)
{
	// declare auxiliary variables
	VRP_PP_OPERATION *lauf;
    struct VRP_PP_OSM_WAYPOINT *Ptr;


	// print the kml-header
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
	stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << "\n";
	stream << "<Document>" << "\n";

	// first print all locations if desired
	if( PrintBubbles == true )
    {
        for (int r = 0; r < this->routes; r++)
        {
            if (this->route[r]->start->get_next() != this->route[r]->end)
            {
                // non-empty route found
                lauf = this->route[r]->start;
                while (lauf->get_next() != NULL)
                {
                    const VRP_PP_NODE*node;

                    stream << "<Placemark>" << "\n";
                    if (lauf->get_type() == VRP_PP_START || lauf->get_type() == VRP_PP_STOP)
                    {
                        node = &prob->get_node_by_index(prob->get_depot_by_index(lauf->get_id()).id_node);
                        stream << "<name>depot " << lauf->get_id() << "</name>" << "\n";
                    }
                    else
                    {
                        stream << "<name>request " << lauf->get_id() << "</name>" << "\n";
                        node = &prob->get_node_by_index(prob->get_request_by_index(lauf->get_id()).id_node);
                    }

                    stream << "<description>" << lauf->get_desc() << "</description>" << "\n";
                    stream << "<Point>" << "\n";
                    stream << "<coordinates>" << node->longitude << "," << node->lattitude<< "," << 0 << "</coordinates>" << "\n";
                    stream << "</Point>" << "\n";
                    stream << "</Placemark>" << "\n";
                    lauf = lauf->get_next();
                }
            }
        }
    }

	// define 10 different line colors
	static string color[] = {
		"000000",    // black
		"191970",    // blue
		"FF0000",    // red
		"00FF00",    // green
		"EE00EE",    // oliv
		"00CED1",    // turquois
		"DAA520",    // gold
		"FFC1C1",    // orange
		"0000FF",    // pink
		"00F000",    // yellow
	};

	// now print the paths
	for (int r = 0; r < this->routes; r++)
	{
	    // check if the current route contains additional operations beside the depot
		if (this->route[r]->start->get_next() != this->route[r]->end)
		{
			stream << "<Placemark>" << "\n";
			stream << "<name>route - ID" << this->route[r]->vehicle_num << "</name>" << "\n";
			stream << "<description>route of vehicle " << this->route[r]->vehicle_num << "</description>" << "\n";
			stream << "<Style>" << "\n";
			stream << "<LineStyle>" << "\n";
			stream << "<color>" << color[r % int(color->size())] << "</color>" << "\n";
			stream << "<width>3</width>" << "\n";
			stream << "</LineStyle>" << "\n";
			stream << "</Style>" << "\n";
			stream << "<LineString>" << "\n";
			stream << "<extrude>1</extrude>" << "\n";
			stream << "<tessellate>1</tessellate>" << "\n";
			stream << "<altitudeMode>absolute</altitudeMode>" << "\n";
			stream << "<coordinates>" << "\n";
			lauf = this->route[r]->start;
			while (lauf->next != NULL)
			{
			    // first get the id of the used arc
                if( lauf->out_arc != nullptr)
                {
                    // go to the first stored waypoint
                    Ptr = _TWIN->OSM_ARC[lauf->out_arc->get_arc_id()].FIRST_WAYPOINT;
                    // iterate through all waypoints
                    while( Ptr != nullptr )
                    {
                        // print the coordinates of the currently considered waypoint
                        stream << Ptr->LON << "," << Ptr->LAT << "," << 0 << "\n";

                        // go to the next waypoint
                        Ptr = Ptr->next;
                    }
                }

                // go to the next operation in the route
				lauf = lauf->get_next();
			}

			// close the KML-tags
			stream << "</coordinates>" << "\n";
			stream << "</LineString>" << "\n";
			stream << "</Placemark>" << "\n";
		}
	}

	// print the kml-footer
	stream << "</Document>" << "\n";
	stream << "</kml>" << "\n";

	// empty the stream
	stream.flush();
}
#else
    #pragma message "vrp++ osm components in vrp_pp_plan.ccp not considered!"
#endif // VRP_PP_OSM

#ifdef VRP_PP_OSM
    #pragma message "vrp++ osm components in vrp_pp_plan.ccp are considered!"
void VRP_PP_PLAN::print_as_kml(std::ostream& stream, const VRP_PP_PROBLEM*prob, class VRP_PP_OSM_TWIN *_TWIN)
{
	// declare auxiliary variables
	VRP_PP_OPERATION *lauf;
    struct VRP_PP_OSM_WAYPOINT *Ptr;


	// print the kml-header
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
	stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << "\n";
	stream << "<Document>" << "\n";


	// define 10 different line colors
	static string color[] = {
		"000000",    // black
		"191970",    // blue
		"FF0000",    // red
		"00FF00",    // green
		"EE00EE",    // oliv
		"00CED1",    // turquois
		"DAA520",    // gold
		"FFC1C1",    // orange
		"0000FF",    // pink
		"00F000",    // yellow
	};

	// now print the paths
	for (int r = 0; r < this->routes; r++)
	{
	    // check if the current route contains additional operations beside the depot
		if (this->route[r]->start->get_next() != this->route[r]->end)
		{
			stream << "<Placemark>" << "\n";
			stream << "<name>route - ID" << this->route[r]->vehicle_num << "</name>" << "\n";
			stream << "<description>route of vehicle " << this->route[r]->vehicle_num << "</description>" << "\n";
			stream << "<Style>" << "\n";
			stream << "<LineStyle>" << "\n";
			stream << "<color>" << color[r % int(color->size())] << "</color>" << "\n";
			stream << "<width>3</width>" << "\n";
			stream << "</LineStyle>" << "\n";
			stream << "</Style>" << "\n";
			stream << "<LineString>" << "\n";
			stream << "<extrude>1</extrude>" << "\n";
			stream << "<tessellate>1</tessellate>" << "\n";
			stream << "<altitudeMode>absolute</altitudeMode>" << "\n";
			stream << "<coordinates>" << "\n";
			lauf = this->route[r]->start;
			while (lauf->next != NULL)
			{
			    // first get the id of the used arc
                if( lauf->out_arc != nullptr)
                {
                    // go to the first stored waypoint
                    Ptr = _TWIN->OSM_ARC[lauf->out_arc->get_arc_id()].FIRST_WAYPOINT;
                    // iterate through all waypoints
                    while( Ptr != nullptr )
                    {
                        // print the coordinates of the currently considered waypoint
                        stream << Ptr->LON << "," << Ptr->LAT << "," << 0 << "\n";

                        // go to the next waypoint
                        Ptr = Ptr->next;
                    }
                }

                // go to the next operation in the route
				lauf = lauf->get_next();
			}

			// close the KML-tags
			stream << "</coordinates>" << "\n";
			stream << "</LineString>" << "\n";
			stream << "</Placemark>" << "\n";
		}
	}

	// print the kml-footer
	stream << "</Document>" << "\n";
	stream << "</kml>" << "\n";

	// empty the stream
	stream.flush();
}
#else
    #pragma message "vrp++ osm components in vrp_pp_plan.ccp not considered!"
#endif // VRP_PP_OSM

#ifdef VRP_PP_OSM
    #pragma message "vrp++ osm components in vrp_pp_plan.ccp are considered!"
void VRP_PP_PLAN::print_as_kml_with_nodes(std::ostream& stream, const VRP_PP_PROBLEM*prob, class VRP_PP_OSM_TWIN *_TWIN)
{
	// declare auxiliary variables
	VRP_PP_OPERATION *lauf;
    struct VRP_PP_OSM_WAYPOINT *Ptr;

	// print the kml-header
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
	stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << "\n";
	stream << "<Document>" << "\n";

	// first print all locations if desired
        for (int r = 0; r < this->routes; r++)
        {
            if (this->route[r]->start->get_next() != this->route[r]->end)
            {
                // non-empty route found
                lauf = this->route[r]->start;
                while (lauf->get_next() != NULL)
                {
                    const VRP_PP_NODE*node;

                    stream << "<Placemark>" << "\n";
                    if (lauf->get_type() == VRP_PP_START || lauf->get_type() == VRP_PP_STOP)
                    {
                        node = &prob->get_node_by_index(prob->get_depot_by_index(lauf->get_id()).id_node);
                        stream << "<name>depot " << lauf->get_id() << "</name>" << "\n";
                    }
                    else
                    {
                        stream << "<name>request " << lauf->get_id() << "</name>" << "\n";
                        node = &prob->get_node_by_index(prob->get_request_by_index(lauf->get_id()).id_node);
                    }

                    stream << "<description>" << lauf->get_desc() << "</description>" << "\n";
                    stream << "<Point>" << "\n";
                    stream << "<coordinates>" << node->longitude << "," << node->lattitude<< "," << 0 << "</coordinates>" << "\n";
                    stream << "</Point>" << "\n";
                    stream << "</Placemark>" << "\n";
                    lauf = lauf->get_next();
                }
            }
        }

	// define 10 different line colors
	static string color[] = {
		"000000",    // black
		"191970",    // blue
		"FF0000",    // red
		"00FF00",    // green
		"EE00EE",    // oliv
		"00CED1",    // turquois
		"DAA520",    // gold
		"FFC1C1",    // orange
		"0000FF",    // pink
		"00F000",    // yellow
	};

	// now print the paths
	for (int r = 0; r < this->routes; r++)
	{
	    // check if the current route contains additional operations beside the depot
		if (this->route[r]->start->get_next() != this->route[r]->end)
		{
			stream << "<Placemark>" << "\n";
			stream << "<name>route - ID" << this->route[r]->vehicle_num << "</name>" << "\n";
			stream << "<description>route of vehicle " << this->route[r]->vehicle_num << "</description>" << "\n";
			stream << "<Style>" << "\n";
			stream << "<LineStyle>" << "\n";
			stream << "<color>" << color[r % int(color->size())] << "</color>" << "\n";
			stream << "<width>3</width>" << "\n";
			stream << "</LineStyle>" << "\n";
			stream << "</Style>" << "\n";
			stream << "<LineString>" << "\n";
			stream << "<extrude>1</extrude>" << "\n";
			stream << "<tessellate>1</tessellate>" << "\n";
			stream << "<altitudeMode>absolute</altitudeMode>" << "\n";
			stream << "<coordinates>" << "\n";
			lauf = this->route[r]->start;
			while (lauf->next != NULL)
			{
			    // first get the id of the used arc
                if( lauf->out_arc != nullptr)
                {
                    // go to the first stored waypoint
                    Ptr = _TWIN->OSM_ARC[lauf->out_arc->get_arc_id()].FIRST_WAYPOINT;
                    // iterate through all waypoints
                    while( Ptr != nullptr )
                    {
                        // print the coordinates of the currently considered waypoint
                        stream << Ptr->LON << "," << Ptr->LAT << "," << 0 << "\n";

                        // go to the next waypoint
                        Ptr = Ptr->next;
                    }
                }

                // go to the next operation in the route
				lauf = lauf->get_next();
			}

			// close the KML-tags
			stream << "</coordinates>" << "\n";
			stream << "</LineString>" << "\n";
			stream << "</Placemark>" << "\n";
		}
	}

	// print the kml-footer
	stream << "</Document>" << "\n";
	stream << "</kml>" << "\n";

	// empty the stream
	stream.flush();
}
#else
    #pragma message "vrp++ osm components in vrp_pp_plan.ccp not considered!"
#endif // VRP_PP_OSM

void VRP_PP_PLAN::print_as_tex(std::ostream& stream,const VRP_PP_PROBLEM*prob)const
{
	double maxX = -VRP_PP_BIGM, minX = VRP_PP_BIGM, maxY = -VRP_PP_BIGM, minY = VRP_PP_BIGM;
	for (int i = 0; i < prob->get_request_ctr(); i++)
	{
		const VRP_PP_NODE& node = prob->get_node_by_index(prob->get_request_by_index(i).id_node);

		if (node.x > maxX)
			maxX = node.x;
		else if (node.x < minX)
			minX = node.x;

		if (node.y > maxY)
			maxY = node.y;
		else if (node.y < minY)
			minY = node.y;
	}


	stream << "\\documentclass{article}\n"
		<< "\\usepackage[pdftex,active,tightpage]{preview}\n"
		<< "\\usepackage{tikz}\n"
		<< "\\PreviewEnvironment{tikzpicture}\n"
		<< "\\usepackage{pgfplots}\n"
		<< "\\pgfplotsset{compat=1.11}\n"

		<< "\\begin{document}\n"
		<< "\\begin{tikzpicture}\n"
		<< "\\begin{axis}[\n"
		<< "scale only axis, % The height and width argument only apply to the actual axis\n"
		<< "height=12.5cm, width=12.5cm, axis x line=middle, axis y line=middle,\n"
		<< "xlabel={$x$}, xlabel style={anchor=north}, ylabel={$y$}, ylabel style={anchor=south east},\n"
		<< "xmin=" << minX << ", xmax=" << maxX << ", ymin=" << minY << ", ymax=" << maxY << ",\n"
		<< "grid=both,grid style={line width=.2pt, draw=gray!50},major grid style={line width=.2pt,draw=gray!50},minor tick num=1]\n";

	for (int i = 0; i < prob->get_request_ctr(); i++)
		stream << "\\node at (" << prob->get_node_by_index(prob->get_request_by_index(i).id_node).x << ","
		<< prob->get_node_by_index(prob->get_request_by_index(i).id_node).y << "){$\\bullet$};\n";

	for (int v = 0; v < routes; v++)
	{
		VRP_PP_OPERATION*lauf = this->route[v]->get_start_depot();
		VRP_PP_OPERATION*pred = lauf->get_next();
		while (pred != NULL)
		{
			stream << "\\draw[black,very thick,->] (" << prob->get_node_by_index(lauf->get_id_node()).x
				<< "," << prob->get_node_by_index(lauf->get_id_node()).y << ") -- ("
				<< prob->get_node_by_index(pred->get_id_node()).x << ","
				<< prob->get_node_by_index(pred->get_id_node()).y << ");\n";
			lauf = pred;
			pred = pred->get_next();
		}
	}

	stream << "\\end{axis}\n"
		<< "\\end{tikzpicture}\n"
		<< "\\end{document}";
}


void VRP_PP_PLAN::print_with_network_as_tex(std::ostream& stream, const VRP_PP_NODE_COORDINATES& NodeCoordinates, const VRP_PP_PROBLEM*prob)const
{
        // Here are the available colours
    string COLOUR[10];
    COLOUR[0] = "Black";
    COLOUR[1] = "Red";
    COLOUR[2] = "SkyBlue";
    COLOUR[3] = "BurntOrange";
    COLOUR[4] = "Gray";
    COLOUR[5] = "Blue";
    COLOUR[6] = "RubineRed";
    COLOUR[7] = "Green";
    COLOUR[8] = "VioletRed";
    COLOUR[9] = "Yellow";
    int COLOURS = 10;

	double maxX = -VRP_PP_BIGM, minX = VRP_PP_BIGM, maxY = -VRP_PP_BIGM, minY = VRP_PP_BIGM;

	// first identify the area extension
	for (int i = 0; i < prob->get_node_ctr(); i++)
	{
		const VRP_PP_NODE& node = prob->get_node_by_index(i);

		switch (NodeCoordinates)
		{
            case VRP_PP_NODE_COORDINATES::EUCLID:
                if (node.x > maxX)
                    maxX = node.x;
                else if (node.x < minX)
                    minX = node.x;

                if (node.y > maxY)
                    maxY = node.y;
                else if (node.y < minY)
                    minY = node.y;

                break;

            case VRP_PP_NODE_COORDINATES::GEO:
                if (node.longitude > maxX)
                    maxX = node.longitude;
                else if (node.longitude < minX)
                    minX = node.longitude;

                if (node.lattitude > maxY)
                    maxY = node.lattitude;
                else if (node.lattitude < minY)
                    minY = node.lattitude;

                break;
		}
	}


	stream << "\\begin{tikzpicture}\n";
	stream << "\\tikzstyle{every node}=[draw,shape=circle];\n";

	// print the evaluation results
	stream << "% length = " << this->eval_result.length << endl;

    // print all nodes in the network
	for (int i = 0; i < prob->get_node_ctr(); i++)
    {
        switch (NodeCoordinates)
		{
            case VRP_PP_NODE_COORDINATES::EUCLID:
                stream << "\\node[fill=white, label=center:\\tiny{"<<i<<"}] at (";
                stream << (prob->get_node_by_index(i).x - minX) / (maxX - minX)<< "\\textwidth,";
                stream << (prob->get_node_by_index(i).y - minY) / (maxY - minY)<< "\\textwidth) [circle] (V"<<i<<") {};\n";
                break;
            case VRP_PP_NODE_COORDINATES::GEO:
                stream << "\\node[fill=white, label=center:\\tiny{"<<i<<"}] at (";
                stream << (prob->get_node_by_index(i).longitude - minX) / (maxX - minX)<< "\\textwidth,";
                stream << (prob->get_node_by_index(i).lattitude - minY) / (maxY - minY)<< "\\textwidth) [circle] (V"<<i<<") {};\n";
                break;
		}

    }

    stream << "% now the arcs\n" << endl;
    for (int i = 0; i < prob->get_arc_ctr(); i++)
    {
        stream << "\\draw[->,dotted, gray] (V"<<prob->get_arc_by_index(i).origin<<") to (V"<<prob->get_arc_by_index(i).dest<<");\n";
    }

    // print all network arcs

    // print the routes stored in the plan object
	for (int v = 0; v < this->routes; v++)
	{
		VRP_PP_OPERATION*lauf = this->route[v]->get_start_depot();
		VRP_PP_OPERATION*pred = lauf->get_next();
		while (pred != NULL)
		{
            stream << "% this is route "<<v<< endl;
			stream << "\\draw[ultra thick, "<<COLOUR[v % COLOURS]<<",->] (V" << prob->get_node_by_index(lauf->get_id_node()).id <<")";
			stream << " to (V" << prob->get_node_by_index(pred->get_id_node()).id << ");\n";
			lauf = pred;
			pred = pred->get_next();
		}
	}

    // print the route agenda
    stream << "% the route agenda" << endl;
    stream << "\\tikzstyle{every node}=[shape=circle];" << endl;
    for (int v = 0; v < routes; v++)
	{
	    stream << "% this is the agenda entry of route " << v << endl;
	    stream << "\\draw node[anchor=west, color="<<COLOUR[v % COLOURS]<<"] at (1.05\\textwidth,"<<1-v*0.05<<"\\textwidth){\\tiny route "<<v<<"};" << endl;
	}
	stream << "\\end{tikzpicture}\n";
}

void VRP_PP_PLAN::add_route(VRP_PP_ROUTE * r)
{

	VRP_PP_ROUTE** nRouteArr = new VRP_PP_ROUTE*[routes + 1]; int ctr = 0;
	std::for_each(route, route + routes, [&](VRP_PP_ROUTE* curR) {nRouteArr[ctr++] = std::move(curR); });
	nRouteArr[routes++] = r;

	delete[] route;
	route = nRouteArr;
}

void VRP_PP_PLAN::clear()
{
	if (route != NULL)
	{
		for (int i = 0; i < routes; i++)
		{
		    //cout << "+" << i << endl;
		    route[i]->clear();
			delete route[i];
			//cout << "-" << i << endl;
		}
		delete[] route;
		routes = 0;
		//cout << "READY" << endl;
		eval_result = VRP_PP_EVALUATION();
		route = NULL;
	}
}






int VRP_PP_PLAN::get_index_of_a_non_empty_route()const
{
	// added in version 1.03 - from 20.06.2018
	// this procedure draws a vehicle with a non empty route
	// if such a route does not exists => -1 is returned
	int ret_val = -1, veh_used = 0, v, *REQ_NUM;
	REQ_NUM = new int[this->routes];
	for (v = 0; v < this->routes; v++)
	{
		if (this->route[v]->start->get_next() != this->route[v]->end)
		{
			REQ_NUM[v] = 1;
			veh_used++;
		}
		else
			REQ_NUM[v] = 0;
	}
	if (veh_used > 0)
	{
		ret_val = -1;
		while (ret_val < 0)
		{
			ret_val = rand() % this->routes;
			if (REQ_NUM[ret_val] < 1)
				ret_val = -1;
		}
	}
	delete[] REQ_NUM;
	return (ret_val);
}

int VRP_PP_PLAN::get_route_ctr() const
{
	return routes;
}

int VRP_PP_PLAN::get_operation_ctr() const
{
	return operations;
}

VRP_PP_ROUTE*  VRP_PP_PLAN::get_route_by_index(int index)
{
	return route[index];
}

VRP_PP_ROUTE * VRP_PP_PLAN::get_route_by_index(int index) const
{
	return route[index];
}

VRP_PP_PLAN* VRP_PP_PLAN::generate_sweep_algorithm(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType)
{
    // added ab JSB
    int i, j;

    // Pre-Processing Phase / Initialization Phase

    // reserve the memory for the new plan
	VRP_PP_PLAN *SweepPlan = new VRP_PP_PLAN(prob);

	SweepPlan->print_as_node_sequences(prob);
	VRP_PP_OPERATION **OP;
	OP = new VRP_PP_OPERATION*[prob->requests];

    // generate the operations
    for( i=0 ; i < prob->requests ; i++ )
   		OP[i] = new VRP_PP_OPERATION(prob->request[i].id, prob->request[i].id_node, prob->request[i].type, "Request " + std::to_string(i));

    // calculate polar angles
    struct _OP_SORT_ARRAY {
        int OP_NUM = -1; double polar_ang = -10.0;
    };

    std::vector<_OP_SORT_ARRAY> OP_CANDIDATE_LIST(prob->requests);
    _OP_SORT_ARRAY TMP;

    for( i=0 ; i < prob->requests ; i++ )
    {
        OP_CANDIDATE_LIST[i].OP_NUM = i;
        if(weigthType==VRP_PP_ARC_WEIGHT::GEO)
            OP_CANDIDATE_LIST[i].polar_ang = polar_angle(
					prob->node[prob->request[i].id_node].longitude, prob->node[prob->request[i].id_node].lattitude,
					prob->node[prob->depot[0].id_node].longitude, prob->node[prob->depot[0].id_node].lattitude);
        else
             OP_CANDIDATE_LIST[i].polar_ang = polar_angle(
					prob->node[prob->request[i].id_node].x, prob->node[prob->request[i].id_node].y,
					prob->node[prob->depot[0].id_node].x, prob->node[prob->depot[0].id_node].y);
    }

    // sort the candidate list
    for( i=0 ; i < prob->requests-1 ; i++ )
        for( j=0 ; j < prob->requests-1 ; j++ )
            if( OP_CANDIDATE_LIST[j].polar_ang > OP_CANDIDATE_LIST[j+1].polar_ang)
            {
                TMP = OP_CANDIDATE_LIST[j];
                OP_CANDIDATE_LIST[j] = OP_CANDIDATE_LIST[j+1];
                OP_CANDIDATE_LIST[j+1] = TMP;
            }

    //for( i=0 ; i < prob->requests ; i++ )
    //    cout << "request " << OP_CANDIDATE_LIST[i].OP_NUM << ": " << OP_CANDIDATE_LIST[i].polar_ang << endl;

    // distribute operations among vehicles
    int CurVeh = 0;
    double CurLoad = 0.0;
    for( i=0 ; i < prob->requests ; i++ )
    {
        if( (CurLoad + prob->request[OP_CANDIDATE_LIST[i].OP_NUM].quantity) <= prob->vehicle[CurVeh].cap )
        {
            SweepPlan->route[CurVeh]->insert_between(OP[OP_CANDIDATE_LIST[i].OP_NUM],SweepPlan->route[CurVeh]->end->get_previous(),
                                                     SweepPlan->route[CurVeh]->end);
            CurLoad += prob->request[OP_CANDIDATE_LIST[i].OP_NUM].quantity;
        }
        else
        {
            // try to find another vehicle
            if( CurVeh + 1 < prob->vehicles )
            {
                CurVeh++;
                CurLoad = 0.0;
                SweepPlan->route[CurVeh]->insert_between(OP[OP_CANDIDATE_LIST[i].OP_NUM],SweepPlan->route[CurVeh]->end->get_previous(),
                                                         SweepPlan->route[CurVeh]->end);
                CurLoad += prob->request[OP_CANDIDATE_LIST[i].OP_NUM].quantity;
            }
            else
            {
                cout << "not enough vehicles available / program terminates prematurely" << endl;
                exit(0);
            }
        }
    }

    // Post-Processing Phase

	delete[] OP;

    SweepPlan->evaluate(prob, weigthType);

	return SweepPlan;
}

VRP_PP_PLAN* VRP_PP_PLAN::generate_savings_algorithm(VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType)
{
    // added ab JSB
    int i, j, orig_veh_num=-1;

    // check if artificial vehicles must be added in order to start with pendular routes
	if( prob->requests > prob->vehicles )
    {
        // first save original vehicle number
        orig_veh_num = prob->vehicles;

        // artificial vehicles needed: add them one after the other!
        for( i=prob->vehicles ; i < prob->requests ; i++ )
        {
            VRP_PP_VEHICLE AddVeh = VRP_PP_VEHICLE(i,0,prob->depot[0].id_node,
                                                   prob->vehicle[0].cap,
                                                   prob->vehicle[0].maxDist,
                                                   prob->vehicle[0].speed,
                                                   // code Thuy adds - add new attributes
                                                   prob->vehicle[0].fuel_a,
                                                   prob->vehicle[0].fuel_b,
                                                   prob->vehicle[0].evw,
                                                   "artificial vehicle"+ std::to_string(i));
            prob->append_vehicle(&AddVeh);
        }
    }
    // reserve the memory for the new plan
	VRP_PP_PLAN*SavingsPlan = new VRP_PP_PLAN(prob);

	// generate the operations
	VRP_PP_OPERATION **OP;
	OP = new VRP_PP_OPERATION*[prob->requests];
    for( i=0 ; i < prob->requests ; i++ )
    {
   		OP[i] = new VRP_PP_OPERATION(prob->request[i].id,
                                  prob->request[i].id_node,
                                  prob->request[i].type,
                                  "Request " + std::to_string(i));
    }

    // prepare to store the vehicle load
    double *Current_Veh_Load;
    Current_Veh_Load = new double [prob->vehicles];
    for( i=0 ; i < prob->vehicles ; i++)
        Current_Veh_Load[i] = 0.0;

    // initialization: generate pendular routes
    for( i=0 ; i < prob->requests ; i++ )
    {
        SavingsPlan->route[i]->insert_between(OP[i],SavingsPlan->route[i]->start,SavingsPlan->route[i]->end);
        Current_Veh_Load[i] = prob->get_request_by_index(OP[i]->get_id()).quantity;
    }

    // definition of savings object template
    struct SAV_TEMPLATE{
        int first_route_id;
        int first_node_id;
        int second_route_id;
        int second_node_id;
        int feasible;
        double savings;
    };

    // prepare iteration
    int Termination_Flag = false;

    int cnt=0;
    // iteration: try to concatenate two routes
    do {
        cout << "Iteration " << cnt << endl;
    cnt++;
        // start with calculating savings-values
        int cntr=0;
        struct SAV_TEMPLATE *SAV;
        SAV = new struct SAV_TEMPLATE[prob->vehicles * prob->vehicles - prob->vehicles];

        for( int i=0 ; i < prob->vehicles ; i++ )
            for( int j=0 ; j < prob->vehicles ; j++)
                if( i != j )
                {
                    SAV[cntr].first_route_id = i;
                    SAV[cntr].first_node_id = SavingsPlan->route[i]->end->get_previous()->get_id_node();
                    SAV[cntr].second_route_id = j;
                    SAV[cntr].second_node_id = SavingsPlan->route[j]->start->get_next()->get_id_node();
                    if(weigthType==VRP_PP_ARC_WEIGHT::GEO)
                        SAV[cntr].savings = prob->ARC[SAV[cntr].first_node_id][prob->depot[0].id_node].length_latlong +
                                        prob->ARC[prob->depot[0].id_node][SAV[cntr].second_node_id].length_latlong -
                                        prob->ARC[SAV[cntr].first_node_id][SAV[cntr].second_node_id].length_latlong;
                    else if(weigthType==VRP_PP_ARC_WEIGHT::EUCLID)
                        SAV[cntr].savings = prob->ARC[SAV[cntr].first_node_id][prob->depot[0].id_node].length_euclid +
                                        prob->ARC[prob->depot[0].id_node][SAV[cntr].second_node_id].length_euclid -
                                        prob->ARC[SAV[cntr].first_node_id][SAV[cntr].second_node_id].length_euclid;
                    else
                        SAV[cntr].savings = prob->ARC[SAV[cntr].first_node_id][prob->depot[0].id_node].length_custom +
                                        prob->ARC[prob->depot[0].id_node][SAV[cntr].second_node_id].length_custom -
                                        prob->ARC[SAV[cntr].first_node_id][SAV[cntr].second_node_id].length_custom;

                    if( Current_Veh_Load[i] + Current_Veh_Load[j] > prob->vehicle[i].cap )
                        SAV[cntr].feasible = -1;
                    else
                        SAV[cntr].feasible = 1;
                    cntr++;
                }


        // now sort the saving-pairs by decreasing saving-values
        struct SAV_TEMPLATE TMP;
        for( int i=0 ; i < cntr-1 ; i++ )
            for( int j=0 ; j < cntr-1 ; j++ )
                if( SAV[j].savings < SAV[j+1].savings )
                {
                    TMP = SAV[j];
                    SAV[j] = SAV[j+1];
                    SAV[j+1] = TMP;
                }

        // now move all feasible in front of infeasible pairs
        for( int i=0 ; i < cntr-1 ; i++ )
            for( int j=0 ; j < cntr-1 ; j++ )
                if( SAV[j].feasible < SAV[j+1].feasible )
                {
                    TMP = SAV[j];
                    SAV[j] = SAV[j+1];
                    SAV[j+1] = TMP;
                }

        if( (SAV[0].savings > 0.0) && (SAV[0].feasible > 0) )
        {
            cout << SAV[0].savings << endl;
            // merge the two routes saved in SAV[0]
            SavingsPlan->route[SAV[0].first_route_id]->append_route(SavingsPlan->route[SAV[0].second_route_id]);
            //SavingsPlan->route[SAV[0].first_route_id]->two_opt_route_full(prob, weigthType);
            Current_Veh_Load[SAV[0].first_route_id] += Current_Veh_Load[SAV[0].second_route_id];
            Current_Veh_Load[SAV[0].second_route_id] = 0;
        }
        else
        {
            // no option left: do not proceed and signalize to abort the iteration
            Termination_Flag = true;
        }
        delete[] SAV;
    }
    while( !Termination_Flag);

    // move the used routes to the beginning of the plan
    for(int i=0 ; i < prob->vehicles-1 ; i++ )
        for(int j=0 ; j < prob->vehicles-1 ; j++ )
            if( SavingsPlan->route[j]->start == SavingsPlan->route[j]->end->get_previous())
                if( SavingsPlan->route[j+1]->start != SavingsPlan->route[j+1]->end->get_previous())
                    SavingsPlan->route[j]->append_route(SavingsPlan->route[j+1]);

    // check, if the number of available vehicles is preserved
    for(int i=orig_veh_num ; i < prob->vehicles ; i++ )
    {
        if( SavingsPlan->route[i]->start != SavingsPlan->route[i]->end->get_previous() )
        {
            cout << "no feasible solution found => too much vehicles required" << endl;
            exit(0);
        }
    }

    // These are the steps to get a plan with the correct number of vehicles.
    // Note: delete a route from a plan is impossible

    // remove the artificial vehicles from problem
    int To_Be_Removed;
    To_Be_Removed = prob->vehicles - orig_veh_num;
    for(int i=0 ; i < To_Be_Removed ; i++ )
        prob->delete_last_vehicle();

     // setup and initialize a new plan
    class VRP_PP_PLAN *SavingsPlanFinal; // = new class VRP_PP_PLAN;
    SavingsPlanFinal = new class VRP_PP_PLAN(prob);

    // move the routes from the tentative plan to the final plan SavingsPlanFinal
    for(int i=0 ; i < prob->vehicles ; i++ )
    {
        SavingsPlanFinal->route[i]->append_route(SavingsPlan->route[i]);
        SavingsPlanFinal->route[i]->route_id = i;
    }


    // destroy the first plan that has to much vehicles
	delete SavingsPlan;

    // release previously temporarily reserved memory
    delete[] Current_Veh_Load;
	delete[] OP;

    // evaluate the plan
    SavingsPlanFinal->evaluate(prob, weigthType);

  return SavingsPlanFinal;
}

void VRP_PP_PLAN::simple_ls(VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType)
{
    // added by version 1.07 - from 01.07.2019
    // modified 04.06.2020 at 18:21
    // modified 06.06.2020 at 12:59 : cleaned from unnecessary comments and code

    // this procedure requires that this->plan has been already generated otherwise it produces non-sense

    // initialize the neighborhood
	struct NBH *N;
	N = new struct NBH(17);     // 17 has been selected to be the ID of this neighborhood

	// add a copy of the initial solution to the neighborhood
	class VRP_PP_PLAN *NEW_SOLUTION = new class VRP_PP_PLAN (*this);

    // evaluation the start solution
	struct VRP_PP_EVALUATION SOL_EVAL;
	SOL_EVAL = NEW_SOLUTION->get_evaluation();

	// add the start solution to the neighborhood and save its objective function value
	N->NBH_add_plan(0, NEW_SOLUTION);
	// the second parameter is used to declare if this plan is feasible (=1) or not (=0)
	N->NBH_update_fitness_at_pos(SOL_EVAL.obj_value,SOL_EVAL.feasible,0);

	//code Thuy adds - change objective values of simple_ls to obj_value
    double best_fitness = SOL_EVAL.obj_value;
    int best_feasible = SOL_EVAL.feasible;

    NEW_SOLUTION = nullptr;

	int TerminationCriterion = -1, Cntr;

	// now start the iteration process of the local search algorithm
	int ITERATION_CNTR = 0;
	while (TerminationCriterion < 0)
	{
       // fill up the neighborhood around so far best solution by outcomes of SOR-moves
       // code Thuy adds - adjust nbh size for different configuration
        for (Cntr = 1; Cntr < prob->nbh_size; Cntr++)
		{
		    class VRP_PP_PLAN *NEW_SOLUTION = new class VRP_PP_PLAN (*(N->NBH_get_pointer_to_plan_at_pos(0)));
		    NEW_SOLUTION->apply_random_move(prob,weigthType);

			N->NBH_add_plan(Cntr, NEW_SOLUTION);
			//cout << "\nEvaluating the " << Cntr << " neighbor" << endl;
			N->NBH_get_pointer_to_plan_at_pos(Cntr)->evaluate(prob,weigthType);

			SOL_EVAL = N->NBH_get_pointer_to_plan_at_pos(Cntr)->get_evaluation();
			N->NBH_update_fitness_at_pos(SOL_EVAL.obj_value,SOL_EVAL.feasible,Cntr);
			NEW_SOLUTION = nullptr;
		}

		// analyse neighboors and save the best solution found the current neighborhood
		NEW_SOLUTION = N->NBH_get_pointer_to_plan_at_pos(N->NBH_index_min_fitness());
		NEW_SOLUTION->evaluate(prob,weigthType);
        SOL_EVAL = NEW_SOLUTION->get_evaluation();

         // re-initialize the neighborhood
 		if (SOL_EVAL.obj_value < best_fitness)
		{
			best_fitness = SOL_EVAL.obj_value;
			best_feasible = SOL_EVAL.feasible;

			NEW_SOLUTION = nullptr;

			class VRP_PP_PLAN *NEW_SOLUTION = new class VRP_PP_PLAN (*N->NBH_get_pointer_to_plan_at_pos(N->NBH_index_min_fitness()));

            N->NBH_reset(prob);

            N->NBH_add_plan(0, NEW_SOLUTION);
  			N->NBH_get_pointer_to_plan_at_pos(0)->evaluate(prob,weigthType);
			SOL_EVAL = N->NBH_get_pointer_to_plan_at_pos(0)->get_evaluation();
			N->NBH_update_fitness_at_pos(SOL_EVAL.obj_value,SOL_EVAL.feasible,0);

			NEW_SOLUTION = nullptr;
			TerminationCriterion = -1;
		}
		else
			TerminationCriterion = 1;

		cout << "\n best objective function value after " << ITERATION_CNTR << " iterations: \t"<< best_fitness << " (feasible=" << best_feasible << ")" << endl;
		ITERATION_CNTR++;
 	}

 	// update the plan by the best found solution
	*this = *NEW_SOLUTION;
    //cout << "number of iterations in simple_ls " << ITERATION_CNTR << endl;
    // empty the neighborhood and delete the neighborhood object
	N->NBH_reset(prob);
	delete N;

}

void VRP_PP_PLAN::two_opt_full(struct VRP_PP_PROBLEM *_P, const VRP_PP_ARC_WEIGHT& weigthType)
{
    // added in version 1.03 - from 30.05.2018
    // modified on 16.06.2019 because route->two_opt_route_full has no return object anymore
	for (int v = 0; v < _P->vehicles; v++)
	{
	    //cout << "vehicle " << v << endl;
		this->route[v]->two_opt_route_full(_P, weigthType);
	}
}


void VRP_PP_PLAN::establish_loading_lists(struct VRP_PP_PROBLEM *prob)
{
    // this procedure adds empty loading lists to all arcs in all routes
    int i;
    for( i=0 ; i < this->routes ; i++)
        this->route[i]->establish_loading_lists(prob);
}

void VRP_PP_PLAN::fill_loading_lists_VRPtype(struct VRP_PP_PROBLEM *prob)
{
    // this procedures fill all loading lists in all routes in a VRP-style
    // assuming that the vehicle leaves the initial operation loaded with all
    // load items to be distributed along the route
    int i;
    for( i=0 ; i < this->routes ; i++)
        this->route[i]->fill_loading_lists_VRPtype(prob);
}

void VRP_PP_PLAN::empty_loading_lists(void)
{
    // this procedure empties all loading lists in all routes
    int i;
    for( i=0 ; i < this->routes ; i++)
    {
        //cout << "route " << i << endl;
        this->route[i]->empty_loading_lists();
    }
}

void VRP_PP_PLAN::remove_loading_lists(void)
{
    // this procedure removes all loading lists from all routes and
    // sets the corresponding references in the operations to NULL
    int i;
    for( i=0 ; i < this->routes ; i++)
    {
        this->route[i]->remove_loading_lists();
    }
}

void VRP_PP_PLAN::fill_used_arcs_unique(struct VRP_PP_PROBLEM *prob)
{
    // this procedure tries to assign a VRP_PP_USED_ARC-Object to each link
    // between two consecutively routed operations in all routes
    int i;
    for( i=0 ; i < this->routes ; i++)
        this->route[i]->assign_used_arcs_unique(prob);
}

void VRP_PP_PLAN::remove_used_arcs(void)
{
    // this procedure removes all VRP_PP_USED_ARCS-references from all
    // routes and sets the corresponding pointers in the operations to NULL
    int i;
    for( i=0 ; i < this->routes ; i++)
    {
        this->route[i]->remove_used_arcs();
    }
}


// not contained in the distributed version

// chapter 3
// this is the pure Dijkstra-Algorithm (added by JSB 26.04.2020 15:14
void VRP_PP_PLAN::SPP_dijkstra_algorithm(class VRP_PP_PROBLEM *_P, struct VRP_PP_CONFIG *_CFG)
{
    // preprocessing starts here
    // make sure that this plan is empty resp. make it empty :-)
    this->clear();

    // find the startnode as well as the stopnode
    struct VRP_PP_NODE StartNode = _P->get_node_by_index(_P->get_request_by_index(0).id_node);
    struct VRP_PP_NODE EndNode = _P->get_node_by_index(_P->get_request_by_index(1).id_node);

    // define a variable
    double CurrentArcLength = 0.0;

    // this defines the matrix entry for a label in the tabular version of Dijkstra's algorithm
    struct _LABEL {
        double value;       // the label value
        int type;           // 1=permanent 0=changeable
        int coming_from;    // remember the predecessor that leads to the current label-value
    };

    // make a label for each node
    struct _LABEL *NodeLabel;
    NodeLabel = new struct _LABEL[_P->nodes];

    // initialize the labels
    int i;
    for( i=0 ; i < _P->nodes ; i++)
    {
        NodeLabel[i].value = VRP_PP_BIGM;
        NodeLabel[i].type = 0;
        NodeLabel[i].coming_from = -1;
    }

    // update the label of the start node
    int CurrentNode, NextNode;
    CurrentNode = StartNode.id;
    NodeLabel[CurrentNode].value = 0;
    NodeLabel[CurrentNode].type = 1;

    // here starts the iteration
    int NoNodeLeft = VRP_PP_FALSE;

    while( NoNodeLeft == VRP_PP_FALSE)
    {

        // make the current node to a permantly labelled node
        NodeLabel[CurrentNode].type = 1;

        // update the labels according to Dijkstra
        for( i=0 ; i < _P->nodes ; i++ )
        {
            if ( i != CurrentNode)
            {
                // identify the correct metric for the arcs
                CurrentArcLength = _P->ARC[CurrentNode][i].length_custom;
                if( _CFG->weigthType == VRP_PP_ARC_WEIGHT::CUSTOM )
                    CurrentArcLength = _P->ARC[CurrentNode][i].length_custom;
                if( _CFG->weigthType == VRP_PP_ARC_WEIGHT::EUCLID )
                    CurrentArcLength = _P->ARC[CurrentNode][i].length_euclid;
                if( _CFG->weigthType == VRP_PP_ARC_WEIGHT::GEO )
                    CurrentArcLength = _P->ARC[CurrentNode][i].length_latlong;

                if( CurrentArcLength < VRP_PP_BIGM )
                {
                    // node i can be reached by an arc from CurrentNode
                    if( (NodeLabel[CurrentNode].value + CurrentArcLength) < NodeLabel[i].value )
                    {
                        // label of node i must be updated
                        NodeLabel[i].value = NodeLabel[CurrentNode].value + CurrentArcLength;
                        NodeLabel[i].coming_from = CurrentNode;
                    }
                }
            }
        }

        // try to go to the next node (with minimal lavel-value among all non-permantly labelled nodes)
        NoNodeLeft = VRP_PP_TRUE;

        NextNode = -1;      // needed to check if we are done
        double MinLabelValue = VRP_PP_BIGM;

        for( i=0 ; i < _P->nodes ; i++ )
        {
            if( i != CurrentNode )
            {
                if( NodeLabel[i].type == 0)
                {
                    if( NextNode < 0 )
                    {
                        // first non-permantly labelled node reached
                        MinLabelValue = NodeLabel[i].value;
                        NextNode = i;
                    }
                    else
                    {
                        // another non-permantly labelled node reached
                        if( NodeLabel[i].value < MinLabelValue)
                        {
                            // improved label value found
                            MinLabelValue = NodeLabel[i].value;
                            NextNode = i;
                        }
                    }
                }
            }
        }

        if( NextNode >= 0)
        {
            // we have found a next node
            NoNodeLeft = VRP_PP_FALSE;     // next iteration possible
            CurrentNode = NextNode;         // NextNode is labelled permanently next
        }
    }

    // here starts the post-processing
    // create a route object pointer
    struct VRP_PP_VEHICLE MySPPVehicle;
    MySPPVehicle = _P->get_vehicle_by_index(0);
    class VRP_PP_ROUTE *MySPPRoute;
    MySPPRoute = new class VRP_PP_ROUTE(MySPPVehicle);

    // set the information of the start as well as the end operation
    MySPPRoute->start->type = VRP_PP_PICKUP;
    MySPPRoute->start->id = _P->get_request_by_index(0).id;
    MySPPRoute->start->id_node = StartNode.id;
    MySPPRoute->start->desc = StartNode.desc;
    MySPPRoute->end->type = VRP_PP_DELIVERY;
    MySPPRoute->end->id = _P->get_request_by_index(1).id;;
    MySPPRoute->end->id_node = EndNode.id;
    MySPPRoute->end->desc = EndNode.desc;

    // add it to the plan
    this->add_route(MySPPRoute);

   // now reconstruct the optimal path backwards
    int LastVisitedNode;
    LastVisitedNode = EndNode.id;

    while( NodeLabel[LastVisitedNode].coming_from >= 0)
    {
        LastVisitedNode = NodeLabel[LastVisitedNode].coming_from;
        if( NodeLabel[LastVisitedNode].coming_from >= 0)
        {
            // create the operation-object of next intermediately visisted node...
            class VRP_PP_OPERATION *MyOp;
            MyOp = new class VRP_PP_OPERATION();
            // ... and set the minimal necessary attributes type, id, node_id and desc
            MyOp->type = VRP_PP_TRANSIT;
            MyOp->id = _P->node[LastVisitedNode].id;
            //MyOp->id = this->route[0]->get_op_ctr();
            MyOp->id_node = _P->node[LastVisitedNode].id;
            MyOp->desc = _P->node[LastVisitedNode].desc;
            // put next operation directly after operation at start node (due to backward generation of optimal path
            this->route[0]->insert_between(MyOp,this->route[0]->start,this->route[0]->start->next);
            // reset the pointer / uncouple this from the operation saved in the route object
            MyOp = NULL;
        }
    }

    // finally release the memory from all used auxiliary variables to avoid memory leaking
    delete[] NodeLabel;
    // delete NodeLabel;
}

// this is Übungsaufgabe Chapter 3 /added by JSB 01.05.2020 at 16:52
void VRP_PP_PLAN::TSP_random_path(const class VRP_PP_PROBLEM *_P, struct VRP_PP_NODE StartNode)
{

}

// chapter 4

void VRP_PP_PLAN::CVRP_generate_random_plan(const VRP_PP_PROBLEM *prob)
{
	int v;

	// This procedure first generates an operation for each delivery request.
	// In a second step it randomly distributes these operations among the available
	// vehicles and inserts the operations at random positions in their routes.
	// VRP_PP_TRACE trace(__FUNCTION__);
	// assert_pointer(prob);

	this->clear();

	// start with the generation of routes
	for(int j=0 ; j < prob->vehicles ; j++)
    {
        struct VRP_PP_VEHICLE MyVeh;
        MyVeh = prob->get_vehicle_by_index(j);

        class VRP_PP_ROUTE *MyRoute;
        MyRoute = new class VRP_PP_ROUTE(MyVeh);

        MyRoute->start->type = VRP_PP_START;
        MyRoute->start->id = 0;         // this operation happens at depot 0 ...
        MyRoute->start->id_node = prob->get_depot_by_index(0).id_node;  // ... and at the node of this depot

        MyRoute->end->type = VRP_PP_STOP;
        MyRoute->end->id = 0;         // this operation happens at depot 0 ...
        MyRoute->end->id_node = prob->get_depot_by_index(0).id_node;  // ... and at the node of this depot

        this->add_route(MyRoute);
        MyRoute = NULL;
    }

	VRP_PP_OPERATION *OP;
	for (int i = 0; i < prob->requests; i++)
	{
	    // first, select a vehicle v at random
		v = rand() % prob->vehicles;

		// second, acquire memory and create operations
		// op.id = request_number
		// op.node_id = number of involved network node
		OP = new VRP_PP_OPERATION(i, prob->request[i].id_node, prob->request[i].type, "Request " + std::to_string(i));

		// third, insert the operation at a random position in route of vehicle v
		this->get_route_by_index(v)->insert_at_rand_pos_after(this->get_route_by_index(v)->start,OP);
		OP = NULL;

	}
}

void VRP_PP_PLAN::print_for_excel(const VRP_PP_PROBLEM *prob)
{
    // this procedure prints a 8-column list of all routes tabular-separated

    class VRP_PP_OPERATION *MovingPtr;
    struct VRP_PP_NODE CurrentNode;
    for(int v=0 ; v < prob->get_vehicle_ctr(); v++)
    {
        MovingPtr = this->route[v]->start;
        while( MovingPtr != NULL )
        {
            cout << this->route[v]->vehicle_num;
            cout << "\t" << MovingPtr->get_id_node();
            CurrentNode = prob->get_node_by_index(MovingPtr->get_id_node());
            cout << "\t" << CurrentNode.x;
            cout << "\t" << CurrentNode.y;
            cout << "\t" << CurrentNode.longitude;
            cout << "\t" << CurrentNode.lattitude;
            cout << "\t" << CurrentNode.desc;
            cout << "\t" << MovingPtr->desc;
            cout << endl;
            MovingPtr = MovingPtr->next;
        }
    }
}


void VRP_PP_PLAN::CVRP_two_opt_full(struct VRP_PP_PROBLEM *_P, const VRP_PP_ARC_WEIGHT& weigthType)
{
    // added in version 1.03 - from 30.05.2018
    // modified on 16.06.2019 because route->two_opt_route_full has no return object anymore
    // modified on 10.05.2020 assigned to CVRP scenarios

	for (int v = 0; v < _P->vehicles; v++)
	{
       this->route[v]->CVRP_two_opt_route_full(_P, weigthType);
	}
}

void VRP_PP_PLAN::CVRP_generate_random_plan_capacitated(const VRP_PP_PROBLEM *prob)
{

	// define a struct to be used for the random but feasible vehicle selection
	struct _VEH_SORT_ARRAY {
		int veh_id;
		double current_load;
		double sort_value;
		int enough_cap;
	};

	// declare required variables
	_VEH_SORT_ARRAY TMP;
	int selected_vehicle;

    // make up the existing plan
    this->clear();

	// start with the generation of routes
	for(int j=0 ; j < prob->vehicles ; j++)
    {
        struct VRP_PP_VEHICLE MyVeh;
        MyVeh = prob->get_vehicle_by_index(j);

        class VRP_PP_ROUTE *MyRoute;
        MyRoute = new class VRP_PP_ROUTE(MyVeh);

        MyRoute->start->type = VRP_PP_START;
        MyRoute->start->id = 0;         // this operation happens at depot 0 ...
        MyRoute->start->id_node = prob->get_depot_by_index(0).id_node;  // ... and at the node of this depot

        MyRoute->end->type = VRP_PP_STOP;
        MyRoute->end->id = 0;         // this operation happens at depot 0 ...
        MyRoute->end->id_node = prob->get_depot_by_index(0).id_node;  // ... and at the node of this depot

        this->add_route(MyRoute);
        MyRoute = NULL;
        delete MyRoute;
    }


	// This procedure first generates an operation for each delivery request.
	// In a second step it randomly distributes these operations among the available
	// vehicles and inserts the operations at random positions in their routes.
	// The limited vehicle capacities are considered


	// initialize the vehicle list
	std::vector<_VEH_SORT_ARRAY> VEH_CANDIDATE_LIST(prob->vehicles);
	//VEH_CANDIDATE_LIST = new _VEH_SORT_ARRAY[prob->vehicles];
	for (int v = 0; v < prob->vehicles; v++)
	{
		VEH_CANDIDATE_LIST[v].veh_id = v;
		VEH_CANDIDATE_LIST[v].current_load = 0.0;
		VEH_CANDIDATE_LIST[v].sort_value = 0.0;
		VEH_CANDIDATE_LIST[v].enough_cap = 0;
	}

	// now generate all operations associated with the customer requests one after another
	// and try to find a vehicle with sufficient residual capacity
	VRP_PP_OPERATION *OP;

	for (int CurrentReq = 0; CurrentReq < prob->requests; CurrentReq++)
	{
		// sort the list of vehicles randomly following steps a)-c)

		// a) assign each vehicle a floating value randomly drawn from interval [0;1]
		for (int v = 0; v < prob->vehicles; v++)
		{
			VEH_CANDIDATE_LIST[v].sort_value = (double)(rand() % 1001) / 1000.0;
			if ((VEH_CANDIDATE_LIST[v].current_load + prob->request[CurrentReq].quantity) <= prob->vehicle[VEH_CANDIDATE_LIST[v].veh_id].cap)
			{
				VEH_CANDIDATE_LIST[v].enough_cap = 1;
			}
			else
			{
				VEH_CANDIDATE_LIST[v].enough_cap = 0;
			}
		}

		// b) sort list of vehicles by increasing sort_value using bubble sort
		std::sort(VEH_CANDIDATE_LIST.begin(), VEH_CANDIDATE_LIST.begin() + prob->vehicles, [](const _VEH_SORT_ARRAY&a, const _VEH_SORT_ARRAY&b) {return a.sort_value < b.sort_value; });
		std::sort(VEH_CANDIDATE_LIST.begin(), VEH_CANDIDATE_LIST.begin() + prob->vehicles, [](const _VEH_SORT_ARRAY&a, const _VEH_SORT_ARRAY&b) {return a.enough_cap > b.enough_cap; });

		// the id of the selected vehicle id VEH_CANIDATE_LIST[0].veh_id
		// if a vehicle with enough resiudal capacity exists it is in the first position of the candidate list
        selected_vehicle = VEH_CANDIDATE_LIST[0].veh_id;

		// acquire memory using non-standard _OPERATION-constructor
		OP = new class VRP_PP_OPERATION(prob->request[CurrentReq].id, prob->request[CurrentReq].id_node, VRP_PP_DELIVERY, "Request " + std::to_string(CurrentReq));

		// increase current_payload of selected vehicle
		VEH_CANDIDATE_LIST[0].current_load += prob->request[CurrentReq].quantity;

		// third, insert the operation at a random position in route of vehicle v
		this->get_route_by_index(selected_vehicle)->insert_at_rand_pos_after(this->get_route_by_index(selected_vehicle)->start,OP);

		// re-direct pointer away from currently inserted operation
		OP = NULL;
	}
}

// Here starts the local search partial

int VRP_PP_PLAN::get_index_of_a_non_empty_route()
{
    // added in version 1.07 - from 20.06.2019

    // this procedure draws a vehicle with a non empty route
    // if such a route does not exists => -1 is returned
	int ret_val = -1, veh_used=0, v, *REQ_NUM;
    int V_NUM;

    V_NUM = this->get_route_ctr();
	REQ_NUM = new int[V_NUM];

	for ( v = 0; v < V_NUM; v++)
	{
		if (this->route[v]->start->get_next() != this->route[v]->end)
		{
			REQ_NUM[v] = 1;
			veh_used++;
		}
        else
			REQ_NUM[v] = 0;
	}

	if (veh_used > 0)
	{
		ret_val = -1;
		while (ret_val < 0)
		{
			ret_val = rand() % V_NUM;
			if (REQ_NUM[ret_val] < 1)
				ret_val = -1;
		}
	}
	delete[] REQ_NUM;

	return(ret_val);
}

void VRP_PP_PLAN::apply_random_move(struct VRP_PP_PROBLEM *_PROB, const VRP_PP_ARC_WEIGHT& weigthType)
{
    // added in version 1.07 - from 24.06.2019
    // modified 04.06.2020 at 18:13

    // This procedures realized a SOR-move

	int Source_Route = -1, Target_Route = -1;
    double CapUsed = 0.0;

	// select a non-empty random route
	Source_Route = this->get_index_of_a_non_empty_route();

    if (Source_Route >= 0)
	{
		// label a randomly selected operation from donating route
		class VRP_PP_OPERATION *Op;
        Op = this->route[Source_Route]->get_ptr_to_arb_op();

        // try to find a receiving route with enough capacity
        struct SORT_Elem
        {
            int sort_value;
            int enough_capacity;
            int route_index;
        };

        int V_NUM = this->get_route_ctr();
        struct SORT_Elem CANDLIST[V_NUM], TmpListElem;
        for(int i=0 ; i < V_NUM ; i++)
        {
            // set the sorting values
            if( i != Source_Route)
                CANDLIST[i].sort_value = rand() % (2*V_NUM);
            else
                CANDLIST[i].sort_value = -1;

            // make the capacity check
            if( i!= Source_Route )
            {
                class VRP_PP_OPERATION *TmpPtr;
                TmpPtr = this->route[i]->start->get_next();
                CapUsed = 0.0;
                while( TmpPtr->get_next() != NULL)
                {
                    CapUsed += _PROB->get_request_by_index(TmpPtr->get_id()).quantity;
                    TmpPtr = TmpPtr->get_next();
                }
                if( (CapUsed + _PROB->get_request_by_index(Op->get_id()).quantity) <= _PROB->get_vehicle_by_index(i).cap)
                    CANDLIST[i].enough_capacity = 1;
                else
                    CANDLIST[i].enough_capacity = -1;
            }
            else
                CANDLIST[i].enough_capacity = -1;
            // set the route index
            if( i != Source_Route)
                CANDLIST[i].route_index = i;
            else
                CANDLIST[i].route_index = -1;
        }

        // now sort the CANDLIST by the sort-values to implement the randomization
        for(int i=0 ; i < V_NUM-1 ; i++ )
            for(int j=0 ; j < V_NUM-1 ; j++ )
                if( CANDLIST[j+1].sort_value < CANDLIST[j].sort_value )
                {
                    TmpListElem = CANDLIST[j+1];
                    CANDLIST[j+1] = CANDLIST[j];
                    CANDLIST[j] = TmpListElem;
                }

        // now sort the CANDLIST to move cap-feasible route to the top of the list
        for(int i=0 ; i < V_NUM-1 ; i++ )
            for(int j=0 ; j < V_NUM-1 ; j++ )
                if( CANDLIST[j+1].enough_capacity > CANDLIST[j].enough_capacity )
                {
                    TmpListElem = CANDLIST[j+1];
                    CANDLIST[j+1] = CANDLIST[j];
                    CANDLIST[j] = TmpListElem;
                }

        if( CANDLIST[0].enough_capacity > 0 )
            Target_Route = CANDLIST[0].route_index;

        //cout << "Source route: " << Source_Route << " and Target route " << Target_Route << endl;
        if( Target_Route >= 0 )
        {
            this->route[Source_Route]->dissolve_operation(Op);
            //this->route[Target_Route]->insert_at_rand_pos(Op);
            this->route[Target_Route]->insert_at_rand_pos_after(this->route[Target_Route]->start,Op);
        }

	}

}


void VRP_PP_PLAN::TSP_LP_SOLVE(const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *CFG)
{
#ifdef VRP_PP_SOLVER
    #pragma message "lp_solve components are considered"
    // first of all: clean up the existing plan structure
    this->clear();

    // save the Number of Nodes
    int N = prob->get_node_ctr();

    // start with the specification of the lp model structure
    lprec *lp;

    // setup an empty lp model structure
    lp = make_lp(0,0);

    // set the optimization sense to minimization
    set_minim(lp);

    // create an index-manager for the decision variables x_{ij}
    class VRP_PP_indexset2D X(N, N, 0);

    // install the decision variables x_{ij}
    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
            add_column(lp,NULL);

    // create an index-manager for the decision variables u_i
    class VRP_PP_indexset1D U(N, N*N);

    // install the decision variables u_{i}
    for(int i=0; i < N ; i++)
        add_column(lp,NULL);

    // create the auxiliary data structure
    struct VRP_PP_ARC ARC;

    // specify the decision variable type binary for all decision variables
    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
           set_binary(lp,1 + X.idx2int(i,j),TRUE);

    // define the objective function
    // NOTE: colum index are 1,...,DVARS

    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
        {
            ARC = prob->get_arc_by_od(i, j);
            // select and set the correct arc weight
            switch (CFG->weigthType)
            {
                case VRP_PP_ARC_WEIGHT::EUCLID:
                    set_obj(lp,1+X.idx2int(i,j),ARC.length_euclid); break;
                case VRP_PP_ARC_WEIGHT::GEO:
                    set_obj(lp,1+X.idx2int(i,j),ARC.length_latlong); break;
                case VRP_PP_ARC_WEIGHT::CUSTOM:
                    set_obj(lp,1+X.idx2int(i,j),ARC.length_custom); break;
                default:
                        set_obj(lp,1+X.idx2int(i,j),ARC.length_custom);
            }
        }

    // define the first constraint: each node is left exactly once
    REAL *COEFF;
    COEFF = new REAL [1 + get_Ncolumns(lp)];
    for(int i=0; i < N ; i++)
    {
        // initialize the coefficients
        for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
            COEFF[k] = 0.0;

        // set the required coefficients
        for( int j=0 ; j < N ; j++ )
            if( j != i )
                COEFF[1+X.idx2int(i,j)] = 1.0;

        // set the constraint of the model
        add_constraint(lp, COEFF, EQ, 1);
    }
    delete[] COEFF;

    // define the second constraint: each node is reached exactly once
    COEFF = new REAL [1 + get_Ncolumns(lp)];
    for(int i=0; i < N ; i++)
    {
        // initialize the coefficients
        for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
            COEFF[k] = 0.0;

        // set the required coefficients
        for( int j=0 ; j < N ; j++ )
            if( j != i )
                COEFF[1+X.idx2int(j,i)] = 1.0;

        // set the constraint of the model
        add_constraint(lp, COEFF, EQ, 1);
    }
    delete[] COEFF;

    // define the third constraint: short cycle prevention
    // initialize the vector of the coefficients
    COEFF = new REAL [1 + get_Ncolumns(lp)];

    // specify the Miller-Tucker-Zemlin constraints
    for(int i=0; i < N ; i++)
        if( prob->node[i].id != prob->get_depot_by_index(0).id_node)
        for( int j=0 ; j < N ; j++ )
        if( prob->node[j].id != prob->get_depot_by_index(0).id_node)
        {
            // initialize the coefficient vector
            for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
                COEFF[k] = 0.0;

            // set the necessary coefficents
            COEFF[1+U.idx2int(i)] = 1.0;
            COEFF[1+U.idx2int(j)] = -1.0;
            COEFF[1+X.idx2int(i,j)] = (double)(N);

            // set the constraint of the model
            add_constraint(lp, COEFF, LE, N-1);
        }

    // release the memory
    delete[] COEFF;

    // avoid any screen messages from lp_solve
    set_verbose(lp,CFG->LP_VERBOSE);

    // try to solve the model
    if( solve(lp) == 0 )
    {
       // an optimal solution has been found

        // prepare the reading of the optimal values
        double *VARVAL;
        VARVAL = new double [get_Ncolumns(lp)];

        // read the optimal decision variable values
        get_variables(lp, VARVAL);

        // create a vehicle object
        struct VRP_PP_VEHICLE MyTSPVehicle;
        MyTSPVehicle = prob->get_vehicle_by_index(0);

        // create a route object
        class VRP_PP_ROUTE *MyTSPRoute;
        MyTSPRoute = new class VRP_PP_ROUTE(MyTSPVehicle);

        // create all required operations
        class VRP_PP_OPERATION **MyOp;
        MyOp = new class VRP_PP_OPERATION* [N];

        // initialize the pointers
        for( int i=0 ; i < N ; i++ )
            MyOp[i] = nullptr;

        // start filling the first _P->nodes with the network nodes
        for( int i=0 ; i < N ; i++ )
        {
            // create the new operation object ...
            MyOp[i] = new class VRP_PP_OPERATION();
            // set the operation attribute to identify the relationship between node and operation
            MyOp[i]->type = VRP_PP_TRANSIT;
            MyOp[i]->id = i;
            MyOp[i]->id_node = i;
            MyOp[i]->id_route = 0;
            MyOp[i]->desc = prob->node[i].desc;
        }


        // set the information of the start as well as the end operation
        MyTSPRoute->start->type = VRP_PP_START;
        MyTSPRoute->start->id_node = prob->get_depot_by_index(0).id_node;
        MyTSPRoute->start->id = 0;
        MyTSPRoute->start->id_route = 0;
        MyTSPRoute->start->desc = prob->get_depot_by_index(0).desc;
        MyTSPRoute->end->type = VRP_PP_STOP;
        MyTSPRoute->end->id_node = prob->get_depot_by_index(0).id_node;
        MyTSPRoute->end->id = 0;
        MyTSPRoute->end->id_route = 0;
        MyTSPRoute->end->desc = prob->get_depot_by_index(0).desc;

        // add the route to the plan
        this->add_route(MyTSPRoute);

       // identify the determined operation sequence
        int ORIG_NODE = prob->get_depot_by_index(0).id_node, DEST_NODE = -1;

        // add the start operation
        DEST_NODE = -1;
        for( int j=0 ; j < N ; j++ )
            if( VARVAL[X.idx2int(ORIG_NODE,j)] > 0.5 )
            {
               DEST_NODE = j;
                j = N + 1;
            }
        // insert all remaining nodes in the determined optimal sequence
        while( DEST_NODE != prob->get_depot_by_index(0).id_node )
        {
           this->route[0]->insert_between(MyOp[DEST_NODE],this->route[0]->end->previous,this->route[0]->end);
            ORIG_NODE = DEST_NODE;
            DEST_NODE = -1;
            for( int j=0 ; j < N ; j++ )
            if( VARVAL[X.idx2int(ORIG_NODE,j)] > 0.5 )
            {
                DEST_NODE = j;
                j = N + 1;
            }
        }
        // release allocated memory
        delete[] VARVAL;

        for( int i=0 ; i < N ; i++ )
            MyOp[i] = nullptr;
        delete[] MyOp;

    }
    // delete the model
    delete_lp(lp);

#else
    #pragma message "lp_solve components are not considered"
    cout << "you called an lp_solve component which have not been considered during compilation!" << endl;
    cout << "please check if if you have the statement #define VRP_PP_SOLVER in vrp_pp_extern.h" << endl;
    exit(0);
#endif // VRP_PP_SOLVER
}

void VRP_PP_PLAN::CVRP_LP_SOLVE(const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *CFG)
{
#ifdef VRP_PP_SOLVER
    #pragma message "lp_solve components are considered"
    // first of all: clean up to existing plan structure
    this->clear();

    // collect necessary information
    int N = prob->get_node_ctr();           // save the Number of nodes
    int V = prob->get_vehicle_ctr();        // save the Number of Vehicles
    int R = prob->get_request_ctr();        // save the number of requests
    int DepotNode = prob->depot[0].id_node; // save the id of the depot node

    // STEP A: installation of the model-object
    lprec *lp;          // start with the specification of the lp model structure
    lp = make_lp(0,0);  // setup an empty lp model structure
    set_minim(lp);      // set the optimization sense to minimization

    // STEP B: Declaration and Setup of all required decision variables
    // create an index-manager for the decision variables x_{ijk}
    class VRP_PP_indexset3D X(N, N, V, 0);

    char dv_name[100];
    // install the decision variables x_{ijk}
    for(int k=0; k < V ; k++)
        for(int i=0; i < N ; i++)
            for(int j=0; j < N ; j++)
                add_column(lp,NULL);

    for(int k=0; k < V ; k++)
        for(int i=0; i < N ; i++)
            for(int j=0; j < N ; j++)
            {
                sprintf(dv_name,"x[%d,%d,%d]",i,j,k);
                set_col_name(lp,1+X.idx2int(i,j,k),dv_name);

            }

    // create an index-manager for the decision variables y_{ik}
    class VRP_PP_indexset2D Y(R, V, N*N*V);

    // install the decision variables y_{ik}
    for(int r=0; r < R ; r++)
        for(int k=0; k < V ; k++)
        {
            add_column(lp,NULL);
            sprintf(dv_name,"y[%d,%d]",r,k);
            set_col_name(lp,1+Y.idx2int(r,k),dv_name);
        }

    // create an index-manager for the decision variables u_i
    class VRP_PP_indexset2D U(N, V, N*N*V + R*V);

    // install the decision variables u_{ik}
    for(int i=0; i < N ; i++)
        for(int k=0; k < V ; k++)
        {
            add_column(lp,NULL);
            sprintf(dv_name,"U[%d,%d]",i,k);
            set_col_name(lp,1+U.idx2int(i,k),dv_name);
        }

    // create the auxiliary data structure
    struct VRP_PP_ARC ARC;

    // specify the decision variable type binary for all X-decision variables
    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
            for(int k=0; k < V ; k++)
                set_binary(lp,1 + X.idx2int(i,j,k),TRUE);

    // specify the decision variable type binary for all Y-decision variables
    for(int r=0; r < R ; r++)
        for(int k=0; k < V ; k++)
            set_binary(lp,1 + Y.idx2int(r,k),TRUE);

    // step C - Specification of the objective function
    // define the objective function
    // NOTE: colum index are 1,...,DVARS

    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
        for(int k=0; k < V ; k++)
        {
            ARC = prob->get_arc_by_od(i, j);
            // select and set the correct arc weight
            switch (CFG->weigthType)
            {
                case VRP_PP_ARC_WEIGHT::EUCLID:
                    set_obj(lp,1+X.idx2int(i,j,k),ARC.length_euclid);
                    break;
                case VRP_PP_ARC_WEIGHT::GEO:
                    set_obj(lp,1+X.idx2int(i,j,k),ARC.length_latlong);
                    break;
                case VRP_PP_ARC_WEIGHT::CUSTOM:
                    set_obj(lp,1+X.idx2int(i,j,k),ARC.length_custom);
                    break;
                default:
                        set_obj(lp,1+X.idx2int(i,j,k),ARC.length_custom);

            }
        }

    // step D: specification of the constraints

    // specification of the coefficient array to define a constraint
    REAL *COEFF;
    COEFF = new REAL [1 + get_Ncolumns(lp)];

    // define the first constraint: each request is assigned to exactly one vehicle
    for(int r=0; r < R ; r++)
    {
        // initialize the coefficients
        for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
            COEFF[k] = 0.0;

        // set the required coefficients
        for( int k=0 ; k < V ; k++ )
            COEFF[1+Y.idx2int(r,k)] = 1.0;

        // set the constraint of the model
        add_constraint(lp, COEFF, EQ, 1);
    }


    // define the second constraint: if req i is assigned to vehicle k then k must go to node(i) from somewhere else
    for(int r=0; r < R ; r++)
        for( int k=0 ; k < V ; k++ )
        {
            // initialize the coefficients
            for(int c=0 ; c < 1 + get_Ncolumns(lp) ; c++)
                COEFF[c] = 0.0;

            // set the required coefficients

            for( int j=0 ; j < N ; j++ )
            {
                if ( j != prob->request[r].id_node )
                {
                    COEFF[1+X.idx2int(j,prob->request[r].id_node,k)] = 1.0;
                }
            }
            COEFF[1+Y.idx2int(r,k)] = -1.0;

            // set the constraint of the model
            add_constraint(lp, COEFF, EQ, 0);
        }

    // define the third constraint: flow preservation
    for(int i=0; i < N ; i++)
        for( int k=0 ; k < V ; k++ )
        {
            // initialize the coefficients
            for(int c=0 ; c < 1 + get_Ncolumns(lp) ; c++)
                COEFF[c] = 0.0;

            // set the required coefficients

            for( int j=0 ; j < N ; j++ )
            {
                if ( j != i )
                {
                    COEFF[1+X.idx2int(i,j,k)] = 1.0;
                    COEFF[1+X.idx2int(j,i,k)] = -1.0;
                }
            }

            // set the constraint of the model
            add_constraint(lp, COEFF, EQ, 0);
        }

    // define the fourth constraint: short cycle prevention
    for(int i=0; i < N ; i++)
        if( i != DepotNode )
            for( int j=0 ; j < N ; j++ )
                if( j != DepotNode )
                    for( int k=0 ; k < V ; k++ )
                    {
                        // initialize the coefficients
                        for(int c=0 ; c < 1 + get_Ncolumns(lp) ; c++)
                            COEFF[c] = 0.0;

                        // set the required coefficients
                        COEFF[1+U.idx2int(i,k)] = 1.0;
                        COEFF[1+U.idx2int(j,k)] = -1.0;
                        COEFF[1+X.idx2int(i,j,k)] = N+1;

                        // set the constraint of the model
                        add_constraint(lp, COEFF, LE, N);
                    }

    // define the fifth constraint: limited capacity
    for( int k=0 ; k < V ; k++ )
    {
        // initialize the coefficients
        for(int c=0 ; c < 1 + get_Ncolumns(lp) ; c++)
            COEFF[c] = 0.0;

        // set the required coefficients
        for( int r=0 ; r < R ; r++ )
            COEFF[1+Y.idx2int(r,k)] = prob->request[r].quantity;

        // set the constraint of the model
        add_constraint(lp, COEFF, LE, prob->vehicle[k].cap);
    }

    // select screen messages from lp_solve
    set_verbose(lp,CFG->LP_VERBOSE);

    set_scaling(lp,1);
    //set_presolve(lp,PRESOLVE_ROWS | PRESOLVE_COLS | PRESOLVE_LINDEP, get_presolveloops(lp));
    set_BFP(lp,"bfp_LUSOL");

    set_bb_rule(lp,NODE_FIRSTSELECT);
    set_bb_floorfirst(lp,BRANCH_AUTOMATIC);

    // step E: calling the model solver
    int SolveRes = solve(lp);

    // try to solve the model
    if(  SolveRes== 0 )
    {
        // an optimal solution has been found

        // step F: fetching the variable values from the optimal solution

        // prepare the reading of the optimal values
        double *VARVAL;
        VARVAL = new double [get_Ncolumns(lp)];

         // read the optimal decision variable values
        get_variables(lp, VARVAL);

        // step G: construct the solution in the VR++ - plan object

        // create all required operations
        class VRP_PP_OPERATION **MyOp;
        MyOp = new class VRP_PP_OPERATION* [N];

        // initialize the pointers
        for( int i=0 ; i < N ; i++ )
            MyOp[i] = nullptr;

        // start filling the operation with the request nodes
        for( int i=0 ; i < R ; i++ )
        {
            // create the new operation object ...
            MyOp[i] = new class VRP_PP_OPERATION();

            // set the operation attribute to identify the relationship between node and operation
            MyOp[i]->type = VRP_PP_DELIVERY;
            MyOp[i]->id = prob->get_request_by_index(i).id;
            MyOp[i]->id_node = prob->get_request_by_index(i).id_node;
            MyOp[i]->desc = prob->get_request_by_index(i).desc;
        }

        // create the routes in the now empty plan object
        struct VRP_PP_VEHICLE MyCVRPVehicle;
        for( int k=0 ; k < V ; k++ )
        {
            // create a new route object
            MyCVRPVehicle = prob->get_vehicle_by_index(k);
            class VRP_PP_ROUTE *MyCVRPRoute;
            MyCVRPRoute = new class VRP_PP_ROUTE(MyCVRPVehicle);

            //set the information of the start as well as the end operation
            MyCVRPRoute->start->type = VRP_PP_START;
            MyCVRPRoute->start->id = prob->get_depot_by_index(0).id;
            MyCVRPRoute->start->id_node = prob->node[DepotNode].id;
            MyCVRPRoute->start->desc = prob->node[DepotNode].desc;

            MyCVRPRoute->end->type = VRP_PP_STOP;
            MyCVRPRoute->end->id = prob->get_depot_by_index(0).id;
            MyCVRPRoute->end->id_node = prob->node[DepotNode].id;
            MyCVRPRoute->end->desc = prob->node[DepotNode].desc;

            // add the route object to the plan
            this->add_route(MyCVRPRoute);

            MyCVRPRoute = NULL;
            delete MyCVRPRoute;

            // identify the determined operation sequence
            int ORIG_NODE = DepotNode, DEST_NODE = -1;

            // add the start operation
            DEST_NODE = -1;
            for( int j=0 ; j < N ; j++ )
                if( VARVAL[X.idx2int(ORIG_NODE,j,k)] > 0.5 )
                {
                    DEST_NODE = j;
                    j = N + 1;
                }

            if( DEST_NODE >= 0 )
            // insert all remaining nodes in the determined optimal sequence
            while( DEST_NODE != DepotNode )
            {
                // which request is associated with node DEST_NODE?
                int OpNum = prob->get_request_id_by_node_id(DEST_NODE);
                if( OpNum >= 0 )
                {
                    // insert operation associated with node DEST_NODE
                    this->route[k]->insert_between(MyOp[OpNum],this->route[k]->end->previous,this->route[k]->end);
                    ARC = prob->get_arc_by_od(ORIG_NODE,DEST_NODE);

                }
                ORIG_NODE = DEST_NODE;
                DEST_NODE = DepotNode;
                for( int j=0 ; j < N ; j++ )
                    if( VARVAL[X.idx2int(ORIG_NODE,j,k)] > 0.5 )
                    {
                        DEST_NODE = j;
                        j = N + 1;
                    }
            }
        }

        // release allocated memory
        delete[] VARVAL;

        for( int i=0 ; i < N ; i++ )
            MyOp[i] = nullptr;
        delete[] MyOp;
    }
    // delete the model
    delete_lp(lp);

#else
    #pragma message "lp_solve components are not considered"
    cout << "you called an lp_solve component which have not been considered during compilation!" << endl;
    cout << "please check if if you have the statement #define VRP_PP_SOLVER in vrp_pp_extern.h" << endl;
    exit(0);
#endif // VRP_PP_SOLVER
}

void VRP_PP_PLAN::SPP_LP_SOLVE(const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG)
{
#ifdef VRP_PP_SOLVER
    #pragma message "lp_solve components are considered"
    // first of all: clean up the existing plan structure
    this->clear();

    // collect necessary information about the problem
    int N = prob->get_node_ctr();
    struct VRP_PP_NODE StartNode = prob->get_node_by_index(prob->request[0].id_node);
    struct VRP_PP_NODE EndNode = prob->get_node_by_index(prob->request[1].id_node);

    // start with the specification of the lp model structure
    lprec *lp;

    // setup an empty lp model structure
    lp = make_lp(0,0);

    // set the optimization sense to minimization
    set_minim(lp);

    // create an index-manager for the decision variables x_{ij}
    class VRP_PP_indexset2D X(N, N, 0);

    // install the decision variables x_{ij}
    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
            add_column(lp,NULL);

    // specify the decision variable type binary for all decision variables
    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
           set_binary(lp,1 + X.idx2int(i,j),TRUE);

    // define the objective function
    // NOTE: colum index are 1,...,DVARS

    // create the auxiliary data structure
    struct VRP_PP_ARC ARC;

    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
        {
            ARC = prob->get_arc_by_od(i, j);
            // select and set the correct arc weight
            switch (_CFG->weigthType)
            {
                case VRP_PP_ARC_WEIGHT::EUCLID:
                    set_obj(lp,1+X.idx2int(i,j),ARC.length_euclid); break;
                case VRP_PP_ARC_WEIGHT::GEO:
                    set_obj(lp,1+X.idx2int(i,j),ARC.length_latlong); break;
                default:
                        set_obj(lp,1+X.idx2int(i,j),ARC.length_custom);
            }
        }

    // define the first constraint: the start node is left exactly once
    REAL *COEFF;
    COEFF = new REAL [1 + get_Ncolumns(lp)];

    // initialize the coefficient vector
    for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
        COEFF[k] = 0.0;

    // set the required coefficients
    for( int j=0 ; j < N ; j++ )
        if( prob->ARC[StartNode.id][j].length_euclid < VRP_PP_BIGM )
                COEFF[1+X.idx2int(StartNode.id,j)] = 1.0;

    // set the constraint of the model
    add_constraint(lp, COEFF, EQ, 1);

    // define the second constraint: no inbound flow into the start node

    // initialize the coefficients
    for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
        COEFF[k] = 0.0;

    // set the required coefficients
    for( int j=0 ; j < N ; j++ )
        if( prob->ARC[j][StartNode.id].length_euclid < VRP_PP_BIGM )
                COEFF[1+X.idx2int(j,StartNode.id)] = 1.0;

    // set the constraint of the model
    add_constraint(lp, COEFF, EQ, 0);

    // define the third constraint: there is inbound flow into the stop node

    // initialize the coefficients
    for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
        COEFF[k] = 0.0;

    // set the required coefficients
    for( int j=0 ; j < N ; j++ )
        if( prob->ARC[j][EndNode.id].length_euclid < VRP_PP_BIGM )
                COEFF[1+X.idx2int(j,EndNode.id)] = 1.0;

    // set the constraint of the model
    add_constraint(lp, COEFF, EQ, 1);

    // define the fourth constraint: no outbound flow from the end node

    // initialize the coefficients
    for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
        COEFF[k] = 0.0;

    // set the required coefficients
    for( int j=0 ; j < N ; j++ )
        if( prob->ARC[StartNode.id][j].length_euclid < VRP_PP_BIGM )
                COEFF[1+X.idx2int(EndNode.id,j)] = 1.0;

    // set the constraint of the model
    add_constraint(lp, COEFF, EQ, 0);

    // define the fifth constraint: flow preservation at all nodes different from StartNode as well as EndNode

    for(int i=0; i < N ; i++)
        if( ( i != StartNode.id ) && ( i != EndNode.id ) )
        {
            // initialize the coefficient vector
            for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
                COEFF[k] = 0.0;

            for( int j=1 ; j < N ; j++ )
            {
                // set the necessary coefficents
                if( prob->ARC[j][i].length_euclid < VRP_PP_BIGM )
                    COEFF[1+X.idx2int(j,i)] = 1.0;
                if( prob->ARC[i][j].length_euclid < VRP_PP_BIGM )
                    COEFF[1+X.idx2int(i,j)] = -1.0;
            }

            // set the constraint of the model
            add_constraint(lp, COEFF, EQ, 0);
        }

    // release the memory
    delete[] COEFF;

    // avoid any screen messages from lp_solve
    set_verbose(lp,_CFG->LP_VERBOSE);

    // try to solve the model
    if( solve(lp) == 0 )
    {
        // an optimal solution has been found

        // prepare the reading of the optimal values
        double *VARVAL;
        VARVAL = new double [get_Ncolumns(lp)];

        // read the optimal decision variable values
        get_variables(lp, VARVAL);

       // create all required operations
        class VRP_PP_OPERATION **MyOp;
        MyOp = new class VRP_PP_OPERATION* [N];

        // initialize the pointers
        for( int i=0 ; i < N ; i++ )
            MyOp[i] = nullptr;

        // start filling the first _P->nodes with the network nodes
        for( int i=0 ; i < N ; i++ )
        {
            // create the new operation object ...
            MyOp[i] = new class VRP_PP_OPERATION();
            // set the operation attribute to identify the relationship between node and operation
            MyOp[i]->type = VRP_PP_TRANSIT;
            MyOp[i]->id = prob->node[i].id;
            MyOp[i]->id_node = prob->node[i].id;
            cout << "i=" << i << " id=" << MyOp[i]->id << " id_node=" << MyOp[i]->id_node << endl;
            MyOp[i]->desc = prob->node[i].desc;
        }

        // create a vehicle object
        struct VRP_PP_VEHICLE MySPPVehicle;
        MySPPVehicle = prob->get_vehicle_by_index(0);

        // create a route object
        class VRP_PP_ROUTE *MySPPRoute;
        MySPPRoute = new class VRP_PP_ROUTE(MySPPVehicle);

        // set the information of the start as well as the end operation
        MySPPRoute->start->type = VRP_PP_PICKUP;
        MySPPRoute->start->id = 0;
        MySPPRoute->start->id_node = MyOp[StartNode.id]->id_node;
        MySPPRoute->start->desc = MyOp[StartNode.id]->desc;
        MySPPRoute->end->type = VRP_PP_DELIVERY;
        MySPPRoute->end->id = 1;
        MySPPRoute->end->id_node = MyOp[EndNode.id]->id_node;
        MySPPRoute->end->desc = MyOp[EndNode.id]->desc;

        // add the route to the plan
        this->add_route(MySPPRoute);

        // identify the determined operation sequence
        int ORIG_NODE = StartNode.id, DEST_NODE = -1;

        // add the start operation
        DEST_NODE = -1;
        for( int j=0 ; j < N ; j++ )
            if( VARVAL[X.idx2int(ORIG_NODE,j)] > 0.5 )
            {
               DEST_NODE = j;
                j = N + 1;
            }

        // insert all remaining nodes in the determined optimal sequence
        struct VRP_PP_NODE Node;
        while( DEST_NODE != EndNode.id )
        {
            cout << "DEST_NODE=" << DEST_NODE << endl;
            this->route[0]->insert_between(MyOp[DEST_NODE],this->route[0]->end->previous,this->route[0]->end);
            ARC = prob->get_arc_by_od(ORIG_NODE,DEST_NODE);
            cout << "insertion done" << endl;
            ORIG_NODE = DEST_NODE;
            DEST_NODE = -EndNode.id;
            for( int j=0 ; j < N ; j++ )
            if( VARVAL[X.idx2int(ORIG_NODE,j)] > 0.5 )
            {
                DEST_NODE = j;
                j = N + 1;
            }
        }

        // release allocated memory
        delete[] VARVAL;

        for( int i=0 ; i < N ; i++ )
            MyOp[i] = nullptr;
        delete[] MyOp;
    }
    // delete the model
    delete_lp(lp);

#else
    #pragma message "lp_solve components are not considered"
    cout << "you called an lp_solve component which have not been considered during compilation!" << endl;
    cout << "please check if if you have the statement #define VRP_PP_SOLVER in vrp_pp_extern.h" << endl;
    exit(0);
#endif // VRP_PP_SOLVER
}

void VRP_PP_PLAN::DCPP_LP_SOLVE(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType)
{
#ifdef VRP_PP_SOLVER
    #pragma message "lp_solve components are considered"
    // first of all: clean up to existing plan structure
    this->clear();

    // save the Number of Nodes
    int N = prob->get_node_ctr();

    // start with the specification of the lp model structure
    lprec *lp;

    // setup an empty lp model structure
    lp = make_lp(0,0);

    // set the optimization sense to minimization
    set_minim(lp);

    // create an index-manager for the decision variables x_{ij}
    class VRP_PP_indexset2D X(N, N, 0);

    // install the decision variables x_{ij}
    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
            add_column(lp,NULL);


    // create the auxiliary data structure
    struct VRP_PP_ARC ARC;

    // specify the decision variable type binary for all decision variables
    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
           set_int(lp,1 + X.idx2int(i,j),TRUE);


    // define the objective function
    // NOTE: colum index are 1,...,DVARS

    for(int i=0; i < N ; i++)
        for(int j=0; j < N ; j++)
        {
            ARC = prob->get_arc_by_od(i, j);
            // select and set the correct arc weight
            switch (weigthType)
            {
                case VRP_PP_ARC_WEIGHT::EUCLID:
                    set_obj(lp,1+X.idx2int(i,j),ARC.length_euclid); break;
                case VRP_PP_ARC_WEIGHT::GEO:
                    set_obj(lp,1+X.idx2int(i,j),ARC.length_latlong); break;
                default:
                        set_obj(lp,1+X.idx2int(i,j),ARC.length_custom);
            }
        }


    // define the first constraint: flow preservation
    REAL *COEFF;
    COEFF = new REAL [1 + get_Ncolumns(lp)];
    for(int i=0; i < N ; i++)
    {
        // initialize the coefficients
        for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
            COEFF[k] = 0.0;

        // set the required coefficients
        for( int j=0 ; j < N ; j++ )
            if( prob->ARC[i][j].length_custom < VRP_PP_BIGM )
                COEFF[1+X.idx2int(i,j)] = 1.0;

        for( int j=0 ; j < N ; j++ )
            if( prob->ARC[j][i].length_custom < VRP_PP_BIGM )
                COEFF[1+X.idx2int(j,i)] = -1.0;

        // set the constraint of the model
        add_constraint(lp, COEFF, EQ, 0);
    }
    delete[] COEFF;

    // define the second constraint: each arc is traversed at least once
    COEFF = new REAL [1 + get_Ncolumns(lp)];
    for(int i=0; i < N ; i++)
        for( int j=0 ; j < N ; j++ )
            if( prob->ARC[i][j].length_custom < VRP_PP_BIGM )
            {
                // initialize the coefficients
                for(int k=0 ; k < 1 + get_Ncolumns(lp) ; k++)
                    COEFF[k] = 0.0;

                // set the required coefficients
                COEFF[1+X.idx2int(i,j)] = 1.0;

                // set the constraint of the model
                add_constraint(lp, COEFF, GE, 1);
            }
    delete[] COEFF;

    // avoid any screen messages from lp_solve
    set_verbose(lp,6);

    // try to solve the model
    if( solve(lp) == 0 )
    {
        // an optimal solution has been found

        // prepare the reading of the optimal values
        double *VARVAL;
        VARVAL = new double [get_Ncolumns(lp)];

        // read the optimal decision variable values
        get_variables(lp, VARVAL);

        for( int i=0 ; i < N ; i++ )
            for( int j=0 ; j < N ; j++ )
                if( prob->ARC[i][j].length_custom < VRP_PP_BIGM )
                    if( VARVAL[X.idx2int(i,j)] > 0.5 )
                        cout << i << "->" << j << ": " << VARVAL[X.idx2int(i,j)] << endl;

        // save the found solution
        int **F;
        F = new int* [N];
        for( int i=0 ; i < N ; i++ )
        {
            F[i] = new int [N];
            for( int j=0 ; j < N ; j++ )
                F[i][j] = 0;
        }
        for( int i=0 ; i < N ; i++ )
            for( int j=0 ; j < N ; j++ )
                if( prob->ARC[i][j].length_custom < VRP_PP_BIGM )
                    if( VARVAL[X.idx2int(i,j)] > 0.5 )
                        F[i][j] = VARVAL[X.idx2int(i,j)];

        for( int i=0 ; i < N ; i++ )
        {
            for( int j=0 ; j < N ; j++ )
                printf("%5d\t",F[i][j]);
            printf("\n");
        }

        // set the start node
        int OrigNode = prob->get_depot_by_index(0).id_node;
        int DestNode = -1;

        do
        {
            DestNode = -1;
            for( int j=0 ; j < N ; j++ )
            {
                if( F[OrigNode][j] > 0 )
                {
                    DestNode = j;
                    cout << OrigNode << "-->" << DestNode << endl;
                    F[OrigNode][DestNode]--;
                    OrigNode = DestNode;
                    j = N + 1;
                }
            }
        }
        while( DestNode > -1 );

        for( int i=0 ; i < N ; i++ )
        {
            for( int j=0 ; j < N ; j++ )
                printf("%5d\t",F[i][j]);
            printf("\n");
        }

        exit(0);
        // create all required operations
        class VRP_PP_OPERATION **MyOp;
        MyOp = new class VRP_PP_OPERATION* [N];

        // initialize the pointers
        for( int i=0 ; i < N ; i++ )
            MyOp[i] = nullptr;

        // start filling the first _P->nodes with the network nodes
        for( int i=0 ; i < N ; i++ )
        {
            // create the new operation object ...
            MyOp[i] = new class VRP_PP_OPERATION();
            // set the operation attribute to identify the relationship between node and operation
            MyOp[i]->id = this->get_operation_ctr();
            MyOp[i]->id_node = prob->node[i].id;
            MyOp[i]->desc = prob->node[i].desc;
        }

        // create a vehicle object
        struct VRP_PP_VEHICLE MyTSPVehicle;
        MyTSPVehicle = prob->get_vehicle_by_index(0);

        // create a route object
        class VRP_PP_ROUTE *MyTSPRoute;
        MyTSPRoute = new class VRP_PP_ROUTE(MyTSPVehicle);

        // set the information of the start as well as the end operation
        //MyTSPRoute->start->id_node = StartNode.id;
        //MyTSPRoute->start->desc = StartNode.desc;
        //MyTSPRoute->end->id_node = StartNode.id;
        //MyTSPRoute->end->desc = StartNode.desc;

        // add the route to the plan
        this->add_route(MyTSPRoute);

        // identify the determined operation sequence
        int ORIG_NODE = 0, DEST_NODE = -1;

        // add the start operation
        DEST_NODE = -1;
        for( int j=0 ; j < N ; j++ )
            if( VARVAL[X.idx2int(ORIG_NODE,j)] > 0.5 )
            {
               DEST_NODE = j;
                j = N + 1;
            }

        // insert all remaining nodes in the determined optimal sequence
        while( DEST_NODE > 0 )
        {
            this->route[0]->insert_between(MyOp[DEST_NODE],this->route[0]->end->previous,this->route[0]->end);
            ARC = prob->get_arc_by_od(ORIG_NODE,DEST_NODE);
            ORIG_NODE = DEST_NODE;
            DEST_NODE = -1;
            for( int j=0 ; j < N ; j++ )
            if( VARVAL[X.idx2int(ORIG_NODE,j)] > 0.5 )
            {
                DEST_NODE = j;
                j = N + 1;
            }
        }

        // release allocated memory
        delete[] VARVAL;

        for( int i=0 ; i < N ; i++ )
            MyOp[i] = nullptr;
        delete[] MyOp;
    }
    // delete the model
    delete_lp(lp);

#else
    #pragma message "lp_solve components are not considered"
    cout << "you called an lp_solve component which have not been considered during compilation!" << endl;
    cout << "please check if if you have the statement #define VRP_PP_SOLVER in vrp_pp_extern.h" << endl;
    exit(0);
#endif // VRP_PP_SOLVER
}

void VRP_PP_PLAN::print_for_xml_export(std::ostream& stream, const VRP_PP_PROBLEM*prob)
{
    using namespace std;
	// declare auxiliary variables
	VRP_PP_OPERATION *lauf;

	// print the xml-header
	stream << "<vrpplusplus name=\"test\" desc=\"result\">" << "\n";

    // print the plan-header
    stream << "\t<VRP_PP_PLAN>" << "\n";

    // print routes opener
    stream << "\t\t<routes>\n";

	// now print the routes
	for (int r = 0; r < this->routes; r++)
	{
        this->route[r]->print_for_xml_export(stream, prob);
 	}

	// print routes closure
    stream << "\t\t</routes>\n";

    // print operation opener
    stream << "\t\t<operations>\n";

    // now print the operations
    class VRP_PP_OPERATION *TmpPtr;

    for (int r = 0; r < this->routes; r++)
	{
        TmpPtr = this->route[r]->start;
        while( TmpPtr != NULL)
        {
            TmpPtr->print_for_xml_export(stream,prob);
            TmpPtr = TmpPtr->next;
        }

	}

	// print operation closer
    stream << "\t\t</operations>\n";

	// print the plan-header
    stream << "\t</VRP_PP_PLAN>" << "\n";

	// print the xml-footer
	stream << "</vrpplusplus>" << "\n";

	stream.flush();

}

VRP_PP_PLAN::VRP_PP_PLAN(const std::string& filename)
{
    // this is a plan-constructor that reads a plan from an xml-file

    // declare the plan attributes
    this->routes = 0;                             // number of routes in the plan object
	this->operations = 0;                         // number of contained operations

	// declare and establish the handler to the xml-file
	IrrXMLReader* xml = createIrrXMLReader(&filename[0]);

	// first read for the determination of the counters
	while (xml && xml->read())
	{

		if (xml->getNodeType() != 3)
		{
		    if (strcmp("route", xml->getNodeName() ) == 0)
				this->routes++;
            if (strcmp("operation", xml->getNodeName()) == 0)
				this->operations++;
		}
    }

	// delete the xml parser after usage
	delete xml;

	// now fill the plan object with the routes
	this->route = new class VRP_PP_ROUTE* [this->routes];
    for( int r=0 ; r < this->routes ; r++ )
    {
        this->route[r] = new class VRP_PP_ROUTE;
    }

	// second read; fill the problem structure

	// declare and establish the handler to the xml-file
	IrrXMLReader* xml2 = createIrrXMLReader(&filename[0]);

	int CNTR_ROUTES = 0;
	int CNTR_OPERATIONS = 0;
	int ROUTE_IDENTIFIER;
	class VRP_PP_OPERATION *MyOp;
	while (xml2 && xml2->read())
	{
		if (xml2->getNodeType() != 3)
		{
			if (strcmp("route", xml2->getNodeName()) == 0)
			{
			    ROUTE_IDENTIFIER = xml2->getAttributeValueAsInt("route_id");
				this->route[ROUTE_IDENTIFIER]->route_id = xml2->getAttributeValueAsInt("route_id");
				this->route[ROUTE_IDENTIFIER]->type = xml2->getAttributeValueAsInt("type");
				this->route[ROUTE_IDENTIFIER]->vehicle_num = xml2->getAttributeValueAsInt("vehicle_num");
				CNTR_ROUTES++;
			}
			if (strcmp("operation", xml2->getNodeName()) == 0)
			{
			    ROUTE_IDENTIFIER = xml2->getAttributeValueAsInt("id_route");
				MyOp = new class VRP_PP_OPERATION();
				MyOp->desc = xml2->getAttributeValueSafe("desc");
				MyOp->finishtime = xml2->getAttributeValueAsFloat("finishtime");
				MyOp->id = xml2->getAttributeValueAsInt("id");
				MyOp->id_node = xml2->getAttributeValueAsInt("id_node");
				MyOp->id_route = xml2->getAttributeValueAsInt("id_route");
				MyOp->reachtime = xml2->getAttributeValueAsFloat("reachtime");
				MyOp->starttime = xml2->getAttributeValueAsFloat("starttime");
				MyOp->type = xml2->getAttributeValueAsInt("type");

                if( this->route[ROUTE_IDENTIFIER]->start != NULL )
                {
                    MyOp->previous = this->route[ROUTE_IDENTIFIER]->end;
                    this->route[ROUTE_IDENTIFIER]->end->next = MyOp;
                    this->route[ROUTE_IDENTIFIER]->end = MyOp;
                    this->route[ROUTE_IDENTIFIER]->op_ctr++;
                }
                else
                {
                    // the current route object is empty
                    this->route[ROUTE_IDENTIFIER]->start = MyOp;
                    this->route[ROUTE_IDENTIFIER]->end = MyOp;
                    this->route[ROUTE_IDENTIFIER]->op_ctr++;
                }

				CNTR_OPERATIONS++;
				MyOp = NULL;
			}
		}
	}

	delete xml2;
}

void VRP_PP_PLAN::VRPTW_FIRST_ALGORITHM(const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG)
{
    // this is an example solution of HW-B c)

    // clear this vehicle
    this->clear();

    // some auxiliary variables
    struct VRP_PP_REQUEST MyRequest;
    struct VRP_PP_VEHICLE MyVehicle;
    struct VRP_PP_ARC MyArc;
    double CurrentTime = 0.0;
    int id_orig_node;
    int id_dest_node;
    int ID;

    // create a list of operations associated with the stored requests (a)
    class VRP_PP_OPERATION **MyOp;
    MyOp = new class VRP_PP_OPERATION* [prob->requests];

    for( int r=0 ; r < prob->requests ; r++ )
    {
        MyRequest = prob->get_request_by_index(r);
        MyOp[r] = new class VRP_PP_OPERATION(MyRequest.id,MyRequest.id_node,VRP_PP_DELIVERY,MyRequest.desc);
    }

    // create and initialize a list of labels (b)
    int *USED;
    USED = new int [prob->requests];
    for( int u=0 ; u < prob->requests ; u++ )
        USED[u] = 0;    // 0 = unused, 1 = used

    // create an array of vehicle routes
    class VRP_PP_ROUTE **MyRoute;
    MyRoute = new class VRP_PP_ROUTE* [prob->vehicles];

    for( int v=0 ; v < prob->vehicles ; v++ )
    {
        // initialize the route object for the v-th vehicle
        MyVehicle = prob->get_vehicle_by_index(v);
        MyRoute[v] = new class VRP_PP_ROUTE(MyVehicle);

        // set the first operation
        MyRoute[v]->start->type = VRP_PP_START;
        MyRoute[v]->start->id = 0;         // this operation happens at depot 0 ...
        MyRoute[v]->start->id_node = prob->get_depot_by_index(0).id_node;  // ... and at the node of this depot

        // set the last operation
        MyRoute[v]->end->type = VRP_PP_STOP;
        MyRoute[v]->end->id = 0;         // this operation happens at depot 0 ...
        MyRoute[v]->end->id_node = prob->get_depot_by_index(0).id_node;  // ... and at the node of this depot

        // add the route to the plan
        this->add_route(MyRoute[v]);
    }

    // initialize counter for request and vehicle
    int CurVehId = 0;
    int CurReqId = 0;
    ID=0;

    // the main loop that iterates over all vehicles
    while( CurVehId < prob->vehicles )
    {
        // (e) check if vehicle CurVehId can serve CurReqId in a feasible way
        id_orig_node = MyRoute[CurVehId]->end->previous->get_id_node();
        MyRequest = prob->get_request_by_index(ID);
        id_dest_node = MyRequest.id_node;
        MyArc = prob->get_arc_by_od(id_orig_node,id_dest_node);

        // save the request ID
        CurReqId = MyRequest.id;

        if( (CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed ) <= MyRequest.win_close )
        {
            // case A
            // cout << "case A" << endl;
            // append the operation to the existing vehicle
            // ADD HERE: insert_between-instruction to append request ID to route CurVehID
            this->route[CurVehId]->insert_between(MyOp[ID],this->route[CurVehId]->end->previous,this->route[CurVehId]->end);

            // Update the CurrentTime
            CurrentTime = CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed;

            // set the arrival time of the operation
            MyOp[CurReqId]->reachtime = CurrentTime;

            // set the operation start time but check if waiting is necessary before
            if( CurrentTime < MyRequest.win_open )
                CurrentTime = MyRequest.win_open;
            MyOp[CurReqId]->starttime = CurrentTime;

            // add the operations duration
            CurrentTime = CurrentTime + MyRequest.duration;

            // set the completion time of the operation
            MyOp[CurReqId]->finishtime = CurrentTime;

            // label request as used
            USED[CurReqId] = 1;
        }
        else
        {
            // case B
            // cout << "case B" << endl;

            // try to find the next unserved but feasible request
            CurReqId = -1;
            for( int r=ID ; r < prob->requests ; r++ )
            {
                id_orig_node = MyRoute[CurVehId]->end->previous->get_id_node();
                MyRequest = prob->get_request_by_index(r);
                id_dest_node = MyRequest.id_node;
                MyArc = prob->get_arc_by_od(id_orig_node,id_dest_node);

                if ( USED[r] < 1 )
                    if( ( CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed ) <= MyRequest.win_close )
                    {
                        // here is such a request that can be served in a feasible way
                        CurReqId = r;
                        r = prob->requests + 1;
                    }
            }


            if( CurReqId >= 0 ) // case C or D
            {
                // part C: an unserved but feasible request exists for this vehicle
                // cout << "-case C" << endl;

                // find the corresponding arc
                id_orig_node = MyRoute[CurVehId]->end->previous->get_id_node();
                MyRequest = prob->get_request_by_index(CurReqId);
                id_dest_node = MyRequest.id_node;
                MyArc = prob->get_arc_by_od(id_orig_node,id_dest_node);

                // append the operation to the existing vehicle
                // ADD HERE: insert_between-instruction to append request CurReqId to route CurVehID

                // Update the CurrentTime
                CurrentTime = CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed;

                // set the arrival time of the operation
                MyOp[CurReqId]->reachtime = CurrentTime;

                // set the operation start time but check if waiting is necessary before
                if( CurrentTime < MyRequest.win_open )
                    CurrentTime = MyRequest.win_open;
                MyOp[CurReqId]->starttime = CurrentTime;

                // add the operations duration
                CurrentTime = CurrentTime + MyRequest.duration;

                // set the completion time of the operation
                MyOp[CurReqId]->finishtime = CurrentTime;

                // label request as used
                USED[CurReqId] = 1;
            }
            else
            {
                // part D:
                // cout << "-case D" << endl;
                if( CurVehId < prob->vehicles-1 ) // case E or F
                {
                    // part E: we can open a new vehicle route to serve request ID
                    // cout << "--case E" << endl;

                    // select the route of the next vehicle
                    CurVehId++;

                    // reset the vehicle start time
                    CurrentTime = 0.0;

                    // get the next arc
                    id_orig_node = MyRoute[CurVehId]->end->previous->get_id_node();
                    MyRequest = prob->get_request_by_index(ID);
                    id_dest_node = MyRequest.id_node;
                    MyArc = prob->get_arc_by_od(id_orig_node,id_dest_node);

                    // append the operation to the existing vehicle
                    // ADD HERE: insert_between-instruction to append request ID to route CurVehID

                    // Update the CurrentTime
                    CurrentTime = CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed;

                    // set the arrival time of the operation
                    MyOp[ID]->reachtime = CurrentTime;

                    // set the operation start time but check if waiting is necessary before
                    if( CurrentTime < MyRequest.win_open )
                        CurrentTime = MyRequest.win_open;
                    MyOp[ID]->starttime = CurrentTime;

                    // add the operations duration
                    MyRequest = prob->get_request_by_index(ID);
                    CurrentTime = CurrentTime + MyRequest.duration;

                    // set the completion time of the operation
                    MyOp[ID]->finishtime = CurrentTime;

                    // label request as used
                    USED[ID] = 1;
                }
                else
                {
                    // case F: we cannot avoid infeasibilities
                    // cout << "--case F" << endl;
                    // we have reached the last vehicle. now we append ID to this route

                    // get the next arc
                    id_orig_node = MyRoute[CurVehId]->end->previous->get_id_node();
                    MyRequest = prob->get_request_by_index(ID);
                    id_dest_node = MyRequest.id_node;
                    MyArc = prob->get_arc_by_od(id_orig_node,id_dest_node);

                    // append the operation to the existing vehicle
                    // ADD HERE: insert_between-instruction to append request ID to route CurVehID

                    // Update the CurrentTime
                    CurrentTime = CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed;

                    // set the arrival time of the operation
                    MyOp[ID]->reachtime = CurrentTime;

                    // set the operation start time but check if waiting is necessary before
                    if( CurrentTime < MyRequest.win_open )
                        CurrentTime = MyRequest.win_open;
                    MyOp[ID]->starttime = CurrentTime;

                    // add the operations duration
                    MyRequest = prob->get_request_by_index(ID);
                    CurrentTime = CurrentTime + MyRequest.duration;

                    // set the completion time of the operation
                    MyOp[ID]->finishtime = CurrentTime;

                    // label request as used
                    USED[ID] = 1;

                    // we append all so far unused requests to this route
                    for( int r=0 ; r < prob->requests ; r++ )
                        if( USED[r] < 1 )
                        {
                            // ADD HERE: insert_between-instruction to append request r to route CurVehID
                            USED[r] = 1;
                        }
                    // we increase the vehicle counter to enforce the exit of this procedure
                    CurVehId++;
                }
            }
        }

        // if necessary, we try to identify the next unserved request (main loop conclusion)
        if( CurVehId < prob->vehicles)
        {
            ID=0;
            while( (USED[ID] > 0) && ( ID < prob->requests ) )
                ID++;
            if( ID >= prob->requests )
            {
                // no unserved request left => exit the while-loop
                CurVehId = prob->vehicles + 1;
            }
        }
    }

    // reset the pointers to the operations
    for( int r=0 ; r < prob->requests ; r++ )
        MyOp[r] = NULL;
    delete[] MyOp;

    // release the reserved memory
    delete[] USED;
}

void VRP_PP_PLAN::VRPTW_FIRST_ALGORITHM2(const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG)
{
    // this is an example solution of HW-B c)

    // clear this vehicle
    this->clear();

    // some auxiliary variables
    struct VRP_PP_REQUEST MyRequest;
    struct VRP_PP_VEHICLE MyVehicle;
    struct VRP_PP_ARC MyArc;
    double CurrentTime = 0.0;
    int id_orig_node;
    int id_dest_node;
    int ID;

    // create a list of operations associated with the stored requests
    class VRP_PP_OPERATION **MyOp;
    MyOp = new class VRP_PP_OPERATION* [prob->requests];

    for( int r=0 ; r < prob->requests ; r++ )
    {
        MyRequest = prob->get_request_by_index(r);
        MyOp[r] = new class VRP_PP_OPERATION(MyRequest.id,MyRequest.id_node,VRP_PP_DELIVERY,MyRequest.desc);
    }

    // create and initialize a list of labels
    int *USED;
    USED = new int [prob->requests];
    for( int u=0 ; u < prob->requests ; u++ )
        USED[u] = 0;    // 0 = unused, 1 = used

    // create an array of vehicle routes
    class VRP_PP_ROUTE **MyRoute;
    MyRoute = new class VRP_PP_ROUTE* [prob->vehicles];

    for( int v=0 ; v < prob->vehicles ; v++ )
    {
        // initialize the route object for the v-th vehicle
        MyVehicle = prob->get_vehicle_by_index(v);
        MyRoute[v] = new class VRP_PP_ROUTE(MyVehicle);

        // set the first operation
        MyRoute[v]->start->type = VRP_PP_START;
        MyRoute[v]->start->id = 0;         // this operation happens at depot 0 ...
        MyRoute[v]->start->id_node = prob->get_depot_by_index(0).id_node;  // ... and at the node of this depot

        // set the last operation
        MyRoute[v]->end->type = VRP_PP_STOP;
        MyRoute[v]->end->id = 0;         // this operation happens at depot 0 ...
        MyRoute[v]->end->id_node = prob->get_depot_by_index(0).id_node;  // ... and at the node of this depot

        // add the route to the plan
        this->add_route(MyRoute[v]);
    }

    // initialize counter for request and vehicle
    int CurVehId = 0;
    int CurReqId = 0;
    ID=0;

    // the main loop that iterates over all vehicles
    while( CurVehId < prob->vehicles )
    {
        // (e) check if vehicle CurVehId can serve CurReqId in a feasible way
        id_orig_node = MyRoute[CurVehId]->end->previous->get_id_node();
        MyRequest = prob->get_request_by_index(ID);
        id_dest_node = MyRequest.id_node;
        MyArc = prob->get_arc_by_od(id_orig_node,id_dest_node);

        // save the request ID
        CurReqId = MyRequest.id;

        if( (CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed ) <= MyRequest.win_close )
        {
            // case A
            // cout << "case A" << endl;
            // append the operation to the existing vehicle
            MyRoute[CurVehId]->insert_between(MyOp[CurReqId],MyRoute[CurVehId]->end->previous,MyRoute[CurVehId]->end);

            // Update the CurrentTime
            CurrentTime = CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed;

            // set the arrival time of the operation
            MyOp[CurReqId]->reachtime = CurrentTime;

            // set the operation start time but check if waiting is necessary before
            if( CurrentTime < MyRequest.win_open )
                CurrentTime = MyRequest.win_open;
            MyOp[CurReqId]->starttime = CurrentTime;

            // add the operations duration
            CurrentTime = CurrentTime + MyRequest.duration;

            // set the completion time of the operation
            MyOp[CurReqId]->finishtime = CurrentTime;

            // label request as used
            USED[CurReqId] = 1;
        }
        else
        {
            // case B
            // cout << "case B" << endl;

            // try to find the next unserved but feasible request
            CurReqId = -1;
            for( int r=ID ; r < prob->requests ; r++ )
            {
                id_orig_node = MyRoute[CurVehId]->end->previous->get_id_node();
                MyRequest = prob->get_request_by_index(r);
                id_dest_node = MyRequest.id_node;
                MyArc = prob->get_arc_by_od(id_orig_node,id_dest_node);

                if ( USED[r] < 1 )
                    if( ( CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed ) <= MyRequest.win_close )
                    {
                        // here is such a request that can be served in a feasible way
                        CurReqId = r;
                        r = prob->requests + 1;
                    }
            }


            if( CurReqId >= 0 )
            {
                // part C: an unserved but feasible request exists for this vehicle
                // cout << "-case C" << endl;

                // find the corresponding arc
                id_orig_node = MyRoute[CurVehId]->end->previous->get_id_node();
                MyRequest = prob->get_request_by_index(CurReqId);
                id_dest_node = MyRequest.id_node;
                MyArc = prob->get_arc_by_od(id_orig_node,id_dest_node);

                // append the operation to the existing vehicle
                MyRoute[CurVehId]->insert_between(MyOp[CurReqId],MyRoute[CurVehId]->end->previous,MyRoute[CurVehId]->end);

                // Update the CurrentTime
                CurrentTime = CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed;

                // set the arrival time of the operation
                MyOp[CurReqId]->reachtime = CurrentTime;

                // set the operation start time but check if waiting is necessary before
                if( CurrentTime < MyRequest.win_open )
                    CurrentTime = MyRequest.win_open;
                MyOp[CurReqId]->starttime = CurrentTime;

                // add the operations duration
                CurrentTime = CurrentTime + MyRequest.duration;

                // set the completion time of the operation
                MyOp[CurReqId]->finishtime = CurrentTime;

                // label request as used
                USED[CurReqId] = 1;
            }
            else
            {
                // part D:
                // cout << "-case D" << endl;
                if( CurVehId < prob->vehicles-1 )
                {
                    // part E: we can open a new vehicle route to serve request ID
                    // cout << "--case E" << endl;

                    // select the route of the next vehicle
                    CurVehId++;

                    // reset the vehicle start time
                    CurrentTime = 0.0;

                    // get the next arc
                    id_orig_node = MyRoute[CurVehId]->end->previous->get_id_node();
                    MyRequest = prob->get_request_by_index(ID);
                    id_dest_node = MyRequest.id_node;
                    MyArc = prob->get_arc_by_od(id_orig_node,id_dest_node);

                    // append the operation to the existing vehicle
                    MyRoute[CurVehId]->insert_between(MyOp[ID],MyRoute[CurVehId]->end->previous,MyRoute[CurVehId]->end);

                    // Update the CurrentTime
                    CurrentTime = CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed;

                    // set the arrival time of the operation
                    MyOp[ID]->reachtime = CurrentTime;

                    // set the operation start time but check if waiting is necessary before
                    if( CurrentTime < MyRequest.win_open )
                        CurrentTime = MyRequest.win_open;
                    MyOp[ID]->starttime = CurrentTime;

                    // add the operations duration
                    MyRequest = prob->get_request_by_index(ID);
                    CurrentTime = CurrentTime + MyRequest.duration;

                    // set the completion time of the operation
                    MyOp[ID]->finishtime = CurrentTime;

                    // label request as used
                    USED[ID] = 1;
                }
                else
                {
                    // case F: we cannot avoid infeasibilities
                    // cout << "--case F" << endl;
                    // we have reached the last vehicle. now we append ID to this route

                    // get the next arc
                    id_orig_node = MyRoute[CurVehId]->end->previous->get_id_node();
                    MyRequest = prob->get_request_by_index(ID);
                    id_dest_node = MyRequest.id_node;
                    MyArc = prob->get_arc_by_od(id_orig_node,id_dest_node);

                    // append the operation to the existing vehicle
                    MyRoute[CurVehId]->insert_between(MyOp[ID],MyRoute[CurVehId]->end->previous,MyRoute[CurVehId]->end);

                    // Update the CurrentTime
                    CurrentTime = CurrentTime + MyArc.length_custom / prob->vehicle[CurVehId].speed;

                    // set the arrival time of the operation
                    MyOp[ID]->reachtime = CurrentTime;

                    // set the operation start time but check if waiting is necessary before
                    if( CurrentTime < MyRequest.win_open )
                        CurrentTime = MyRequest.win_open;
                    MyOp[ID]->starttime = CurrentTime;

                    // add the operations duration
                    MyRequest = prob->get_request_by_index(ID);
                    CurrentTime = CurrentTime + MyRequest.duration;

                    // set the completion time of the operation
                    MyOp[ID]->finishtime = CurrentTime;

                    // label request as used
                    USED[ID] = 1;

                    // we append all so far unused requests to this route
                    for( int r=0 ; r < prob->requests ; r++ )
                        if( USED[r] < 1 )
                        {
                            MyRoute[CurVehId]->insert_between(MyOp[r],MyRoute[CurVehId]->end->previous,MyRoute[CurVehId]->end);
                            USED[r] = 1;
                        }
                    // we increase the vehicle counter to enforce the exit of this procedure
                    CurVehId++;
                }
            }
        }

        // if necessary, we try to identify the next unserved request (main loop conclusion)
        if( CurVehId < prob->vehicles)
        {
            ID=0;
            while( (USED[ID] > 0) && ( ID < prob->requests ) )
                ID++;
            if( ID >= prob->requests )
            {
                // no unserved request left => exit the while-loop
                CurVehId = prob->vehicles + 1;
            }
        }
    }

    // reset the pointers to the operations
    for( int r=0 ; r < prob->requests ; r++ )
        MyOp[r] = NULL;
    delete[] MyOp;

    // release the reserved memory
    delete[] USED;
}

void VRP_PP_PLAN::print_gantt_chart(std::ostream& _FILE, const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG)
{
    // this procedure creates a tikzpicture-environment containing a gantt-chart representation of the current
    // plan.
    // the horizontal axis shows the time line. On the vertical axis, we stack the vehicles
    // ensure that this plan is evaluated before you call this procedure.
    //
    // added in 3.1 - 21.06.2021 - 17:09

    // open the picture environment
    _FILE << "\\rotatebox{0}{"<< endl;
    _FILE << "\\begin{tikzpicture}" << endl;
    // first move to the routes to identify the largest relevant time point
    double max_time = 0.0;
    int OPNUM=0;
    struct VRP_PP_OPERATION *OpPtr;
    for( int i=0 ; i < this->routes ; i++ )
    {
        if( this->route[i]->end != NULL )
        {
            if( this->route[i]->end->finishtime > max_time )
                max_time = this->route[i]->end->finishtime;
            //cout << "route " << i << ": " << this->route[i]->end->finishtime << " " << max_time << endl;
        }
        OpPtr = this->route[i]->start;
        while( OpPtr != NULL )
        {
            OPNUM++;
            OpPtr = OpPtr->next;
        }
    }

    // prepare sorting the operation start times for labelling across routes
    struct OP_ELEM
    {
        double starttime;
        int route_number;
    };

    struct OP_ELEM OP[OPNUM];
    OPNUM=0;
    for( int i=0 ; i < this->routes ; i++ )
    {
        OpPtr = this->route[i]->start;
        while( OpPtr != NULL )
        {
            OP[OPNUM].starttime = OpPtr->starttime;
            OP[OPNUM].route_number = i;
            OPNUM++;
            OpPtr = OpPtr->next;
        }
    }

    struct OP_ELEM OP_TMP;
    for( int i=0 ; i < OPNUM ; i++ )
        for( int j=0 ; j < OPNUM-1 ; j++)
            if( OP[j].starttime > OP[j+1].starttime )
            {
                OP_TMP = OP[j];
                OP[j] = OP[j+1];
                OP[j+1] = OP_TMP;
            }

    // define constants
    double pic_width = 12.0; // Gesamtbreite chart (exclusive h_offset)
    double v_offset = 1.0; // 1cm
    double v_height = 1.0; // height of vehicle bar
    double h_offset = 1.0; // 1cm
    // second move through the routes
    class VRP_PP_OPERATION *Tmp;

    // print the vertical time line connections
    int OPCNTR=0;
    for( int OPCNTR=0 ; OPCNTR < OPNUM ; OPCNTR++ )
    {
        _FILE << "% vertical connections to time axis of vehicle V" << OPCNTR << endl;

            if( (OPCNTR % 2) == 0 )
            {
                _FILE << "\\draw [draw=black, dashed] ("<< h_offset + (OP[OPCNTR].starttime) / max_time * pic_width<<"cm,"<<v_offset*0.5 + (OP[OPCNTR].route_number+1)*v_height<<"cm) to ("<< h_offset + (OP[OPCNTR].starttime) / max_time * pic_width<<"cm,"<<0.8*v_height<<"cm); %con to time line" << endl;
                _FILE << "\\node [rotate=90, left] at ("<< h_offset + (OP[OPCNTR].starttime) / max_time * pic_width<<"cm,"<<0.8*v_height<<"cm){\\tiny "<<round(OP[OPCNTR].starttime*10)/10.0<<"};" << endl;
            }
            else
            {
                _FILE << "\\draw [draw=black, dashed] ("<< h_offset + (OP[OPCNTR].starttime) / max_time * pic_width<<"cm,"<<v_offset*0.5 + (OP[OPCNTR].route_number+1)*v_height<<"cm) to ("<< h_offset + (OP[OPCNTR].starttime) / max_time * pic_width<<"cm,"<<(this->routes+1)*v_height<<"cm); %con to time line" << endl;
                _FILE << "\\node [rotate=90, right] at ("<< h_offset + (OP[OPCNTR].starttime) / max_time * pic_width<<"cm,"<<(this->routes+1)*v_height<<"cm){\\tiny "<<round(OP[OPCNTR].starttime*10)/10.0<<"};" << endl;
            }
    }

    for( int i=0 ; i < this->routes ; i++ )
    {
        _FILE << "% schedule of vehicle V" << i << endl;
        Tmp = this->route[i]->start;
        while( Tmp != NULL )
        {
            _FILE << "\\filldraw [fill=Gray!10, opacity=0.75] (" << h_offset + (Tmp->reachtime) / max_time * pic_width<<"cm," <<v_offset + i*v_height+0.1<<"cm) rectangle ++("<<(Tmp->starttime / max_time - Tmp->reachtime / max_time) * pic_width<<"cm,"<<v_height-0.2<<"cm); %WAIT"<< endl;
            _FILE << "\\filldraw [fill=Gray!40, opacity=0.75] (" << h_offset + Tmp->starttime / max_time * pic_width<<"cm," <<v_offset + i*v_height+0.1<<"cm) rectangle ++("<<(Tmp->finishtime / max_time-Tmp->starttime / max_time) * pic_width<<"cm,"<<v_height-0.2<<"cm); %OP"<< endl;
            // draw the order label
            _FILE << "\\draw [draw=black] ("<< h_offset + (Tmp->reachtime) / max_time * pic_width<<"cm,"<<v_offset + i*v_height+0.5*v_height<<"cm) to ("<< h_offset + (Tmp->finishtime) / max_time * pic_width<<"cm,"<<v_offset + i*v_height+0.5*v_height<<"cm); %ABC" << endl;
            //cout << "\\draw [draw=black,line width=0.5mm] (" << h_offset + (Tmp->reachtime) / max_time * pic_width<<"cm," <<i*v_height+0.1<<"cm) rectangle ++("<<(Tmp->finishtime / max_time - Tmp->reachtime / max_time) * pic_width<<"cm,"<<v_height*0.8<<"cm)node[pos=0.5, fill=White]{"<<Tmp->id<<"}; %WAIT AND OP"<< endl;

            if( Tmp->next != NULL )
            {
                _FILE << "\\filldraw [fill=White, draw=black, opacity=0.75] (" << h_offset + Tmp->finishtime / max_time  * pic_width<<"cm," <<v_offset + i*v_height+0.1<<"cm) rectangle ++("<<(Tmp->next->reachtime / max_time-Tmp->finishtime / max_time)  * pic_width<<"cm,"<<v_height-0.2<<"cm); %DRIVE"<< endl;
                //cout << "\\node at ("<<h_offset + Tmp->finishtime / max_time  * pic_width + (Tmp->next->reachtime / max_time  * pic_width - Tmp->finishtime / max_time  * pic_width) /2.0 <<","<<v_height / 2.0 +i*v_height<<") {\\tiny $"<<Tmp->id_node<<"\\!\\!\\rightarrow\\!\\!"<<Tmp->next->id_node<<"$};" << endl;
            }
            Tmp = Tmp->next;
        }
    }
    double TW_CLOSE = 0.0;
    for( int i=0 ; i < this->routes ; i++ )
    {
        _FILE << "% order labels in vehicle V" << i << endl;
        Tmp = this->route[i]->start;
        while( Tmp != NULL )
        {
            // draw the order label
            if( Tmp->type == VRP_PP_START )
            {
                TW_CLOSE = prob->get_depot_by_index(Tmp->id).win_close;
                if( Tmp->starttime <= TW_CLOSE )
                    _FILE << "\\draw [draw=black,line width=0.5mm] (" << h_offset + (Tmp->reachtime) / max_time * pic_width<<"cm," <<v_offset + i*v_height+0.1<<"cm) rectangle ++("<<(Tmp->finishtime / max_time - Tmp->reachtime / max_time) * pic_width<<"cm,"<<v_height-0.2<<"cm)node[pos=0.5, fill=White, opacity=0.75]{\\tiny S"<<Tmp->id<<"}; %WAIT AND OP label"<< endl;
                else
                    _FILE << "\\draw [draw=black,line width=0.5mm] (" << h_offset + (Tmp->reachtime) / max_time * pic_width<<"cm," <<v_offset + i*v_height+0.1<<"cm) rectangle ++("<<(Tmp->finishtime / max_time - Tmp->reachtime / max_time) * pic_width<<"cm,"<<v_height-0.2<<"cm)node[pos=0.5, fill=White, opacity=0.75]{\\tiny S"<<Tmp->id<<"!}; %WAIT AND OP label"<< endl;
            }
            else
            if( Tmp->type == VRP_PP_STOP )
            {
                TW_CLOSE = prob->get_depot_by_index(Tmp->id).win_close;
                if( Tmp->starttime <= TW_CLOSE )
                    _FILE << "\\draw [draw=black,line width=0.5mm] (" << h_offset + (Tmp->reachtime) / max_time * pic_width<<"cm," <<v_offset + i*v_height+0.1<<"cm) rectangle ++("<<(Tmp->finishtime / max_time - Tmp->reachtime / max_time) * pic_width<<"cm,"<<v_height-0.2<<"cm)node[pos=0.5, fill=White, opacity=0.75]{\\tiny S"<<Tmp->id<<"}; %WAIT AND OP label"<< endl;
                else
                    _FILE << "\\draw [draw=black,line width=0.5mm] (" << h_offset + (Tmp->reachtime) / max_time * pic_width<<"cm," <<v_offset + i*v_height+0.1<<"cm) rectangle ++("<<(Tmp->finishtime / max_time - Tmp->reachtime / max_time) * pic_width<<"cm,"<<v_height-0.2<<"cm)node[pos=0.5, fill=White, opacity=0.75]{\\tiny S"<<Tmp->id<<"!}; %WAIT AND OP label"<< endl;
            }
            else
                {
                    TW_CLOSE = prob->get_request_by_index(Tmp->id).win_close;
                    if( Tmp->starttime <= TW_CLOSE )
                       _FILE << "\\draw [draw=black,line width=0.5mm] (" << h_offset + (Tmp->reachtime) / max_time * pic_width<<"cm," <<v_offset + i*v_height+0.1<<"cm) rectangle ++("<<(Tmp->finishtime / max_time - Tmp->reachtime / max_time) * pic_width<<"cm,"<<v_height-0.2<<"cm)node[pos=0.5, fill=White, opacity=0.75]{\\tiny "<<Tmp->id<<"}; %WAIT AND OP label"<< endl;
                    else
                       _FILE << "\\draw [draw=black,line width=0.5mm] (" << h_offset + (Tmp->reachtime) / max_time * pic_width<<"cm," <<v_offset + i*v_height+0.1<<"cm) rectangle ++("<<(Tmp->finishtime / max_time - Tmp->reachtime / max_time) * pic_width<<"cm,"<<v_height-0.2<<"cm)node[pos=0.5, fill=White, opacity=0.75]{\\tiny "<<Tmp->id<<"!}; %WAIT AND OP label"<< endl;
                }


            Tmp = Tmp->next;
        }
    }

    // third draw the y-axis labels
    _FILE << "% draw the y-axis labels" << endl;
    for( int i=0 ; i < this->routes ; i++ )
    {
        _FILE << "\\node at (0,"<<v_offset + v_height / 2.0 +i*v_height<<") {\\tiny $V_"<<i<<"$};" << endl;
    }
    //fourth print the agenda
    _FILE << "% draw the agenda" << endl;
    _FILE << "\\filldraw [fill=Gray!10, draw=black] (" << h_offset+0.000*pic_width<<"cm," << v_offset + (this->routes+1)*v_height+0.25<<"cm) rectangle ++("<<0.333*pic_width<<"cm,"<<0.5*v_height<<"cm) node[pos=0.5]{\\tiny waiting}; %WAIT"<< endl;
    _FILE << "\\filldraw [fill=Gray!40, draw=black] (" << h_offset+0.333*pic_width<<"cm," << v_offset + (this->routes+1)*v_height+0.25<<"cm) rectangle ++("<<0.333*pic_width<<"cm,"<<0.5*v_height<<"cm) node[pos=0.5]{\\tiny with customer}; %with cust"<< endl;
    _FILE << "\\filldraw [fill=White, draw=black] (" << h_offset+0.666*pic_width<<"cm," << v_offset + (this->routes+1)*v_height+0.25<<"cm) rectangle ++("<<0.333*pic_width<<"cm,"<<0.5*v_height<<"cm) node[pos=0.5]{\\tiny driving}; %driving"<< endl;
    //draw the axis
    _FILE << "% draw horizonzal axis" << endl;
    _FILE << "\\draw[->] (0cm,"<<v_offset<<"cm) to ("<<h_offset + pic_width<<"cm,"<<v_offset<<"cm);" << endl;
    _FILE << "\\node at ("<<0<<"cm,"<<0.5*v_offset <<") {\\tiny time};" << endl;

    // conclude this picture
    _FILE << "\\end{tikzpicture}}" << endl;
}

void VRP_PP_PLAN::print_3D(std::ostream& _FILE, const VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG, int *_DISPLAYED)
{
    // Here are the available colours
    string COLOUR[10];
    COLOUR[0] = "Black";
    COLOUR[1] = "Red";
    COLOUR[2] = "SkyBlue";
    COLOUR[3] = "BurntOrange";
    COLOUR[4] = "Gray";
    COLOUR[5] = "Blue";
    COLOUR[6] = "RubineRed";
    COLOUR[7] = "Green";
    COLOUR[8] = "VioletRed";
    COLOUR[9] = "Yellow";
    int COLOURS = 10;

    // open the picture environment
    _FILE << "\\rotatebox{0}{"<< endl;
    _FILE << "\\tdplotsetmaincoords{75}{115}" << endl;
    _FILE << "\\begin{tikzpicture}[tdplot_main_coords,scale=6]" << endl;

    // at the very beginning: find out assignment ops to vehs
    int OPNUM=0;
    struct VRP_PP_OPERATION *OpPtr;
    // identify which vehicle serves which operation
    int OPVEH[OPNUM];
    for( int i=0 ; i < OPNUM ; i++ )
        OPVEH[i] = -1;

    OPNUM=0;
    for( int i=0 ; i < this->routes ; i++ )
    if( _DISPLAYED[i] == VRP_PP_TRUE )
    {
        OpPtr = this->route[i]->start;
        while( OpPtr != NULL )
        {
            OPVEH[OPNUM] = i;
            OPNUM++;
            OpPtr = OpPtr->next;
        }
    }

    // first move to the routes to identify the largest relevant time point
    double max_time = 0.0;
    OPNUM = 0;
    for( int i=0 ; i < this->routes ; i++ )
    if( _DISPLAYED[i] == VRP_PP_TRUE )
    {
        if( this->route[i]->end != NULL )
        {
            if( this->route[i]->end->finishtime > max_time )
                max_time = this->route[i]->end->finishtime;
            //cout << "route " << i << ": " << this->route[i]->end->finishtime << " " << max_time << endl;
        }
        //this->route[i]->print_as_operation_table(prob);
        //exit(0);
        OpPtr = this->route[i]->start;
        while( OpPtr != NULL )
        {
            OPNUM++;
            OpPtr = OpPtr->next;
        }
    }

    // now check if the time windows last longer
    //for( int r=0 ; r < prob->get_request_ctr() ; r++ )
    //    if( (prob->get_request_by_index(r).win_close >= 0) && (prob->get_request_by_index(r).win_close > max_time) )
    //        max_time = prob->get_request_by_index(r).win_close;

    // now move to the routes to identify the extreme longitudial/lattitudial values
    double min_long = VRP_PP_BIGM;
    double max_long = -VRP_PP_BIGM;
    double min_latt = VRP_PP_BIGM;
    double max_latt = -VRP_PP_BIGM;
    for( int i=0 ; i < this->routes ; i++ )
    if( _DISPLAYED[i] == VRP_PP_TRUE )
    {
        OpPtr = this->route[i]->start;
        while( OpPtr != NULL )
        {
            if( prob->get_node_by_index(OpPtr->id_node).longitude < min_long )
                min_long = prob->get_node_by_index(OpPtr->id_node).longitude;
            if( prob->get_node_by_index(OpPtr->id_node).longitude > max_long )
                max_long = prob->get_node_by_index(OpPtr->id_node).longitude;
            if( prob->get_node_by_index(OpPtr->id_node).lattitude < min_latt )
                min_latt = prob->get_node_by_index(OpPtr->id_node).lattitude;
            if( prob->get_node_by_index(OpPtr->id_node).lattitude > max_latt )
                max_latt = prob->get_node_by_index(OpPtr->id_node).lattitude;
            OpPtr = OpPtr->next;
        }
    }

    // print the scaling information as commentary into the source
    _FILE << "% min_time = " << 0 << endl;
    _FILE << "% max_time = " << max_time << endl;
    _FILE << "% min_long = " << min_long << endl;
    _FILE << "% max_long = " << max_long << endl;
    _FILE << "% min_latt = " << min_latt << endl;
    _FILE << "% max_latt = " << max_latt << endl;

    // print the coordinate system
    _FILE << "% print the coordinate system axis" << endl;
    _FILE << "\\draw[thick,->] (0,0,0) -- (1,0,0) node[anchor=north east]{$x/long$};" << endl;
    _FILE << "\\draw[thick,->] (0,0,0) -- (0,1,0) node[anchor=north west]{$y/latt$};" << endl;
    _FILE << "\\draw[thick,->] (0,0,0) -- (0,0,1) node[anchor=south]{$time$ ("<< 0 <<"-"<< max_time <<")};" << endl;

    _FILE << "%" << endl;
    _FILE << "\\draw[thick, dashed] (1,0,0) -- (1,1,0);" << endl;
    _FILE << "\\draw[thick, dashed] (0,1,0) -- (1,1,0);" << endl;
    _FILE << "\\draw[thick, dashed] (1,1,0) -- (1,1,1);" << endl;
    _FILE << "%" << endl;
    _FILE << "\\draw[thick, dashed] (0,0,1) -- (0,1,1);" << endl;
    _FILE << "\\draw[thick, dashed] (0,1,1) -- (1,1,1);" << endl;
    _FILE << "\\draw[thick, dashed] (1,1,1) -- (1,0,1);" << endl;
    _FILE << "\\draw[thick, dashed] (1,0,1) -- (0,0,1);" << endl;

    // now define all points in the space
    _FILE << "% define all needed points" << endl;
    _FILE << "\\tdplotsetcoord{O}{0}{0}{0}" << endl;
    double x_screen, y_screen, z_screen, radius;

    int PT_CNTR=0;
    double TW_OPEN = 0.0;
    double TW_CLOSE = 1.0;
    double AT = 0.0;
    double ST = 0.0;
    double FT = 0.0;
    for( int i=0 ; i < this->routes ; i++ )
    if( _DISPLAYED[i] == VRP_PP_TRUE )
    {
        OpPtr = this->route[i]->start;
        while( OpPtr != NULL )
        {
            // first convert into polar coordinares
            if( (OpPtr->type == VRP_PP_PICKUP) || (OpPtr->type == VRP_PP_DELIVERY) || (OpPtr->type == VRP_PP_TRANSIT) )
            {
                x_screen = 1 * ( prob->get_node_by_index(OpPtr->id_node).longitude - min_long) / ( max_long - min_long);
                y_screen = 1 * ( prob->get_node_by_index(OpPtr->id_node).lattitude - min_latt) / ( max_latt - min_latt);
                z_screen = 1 * ( OpPtr->reachtime -0 )/ ( max_time - 0);
                AT = 1 * ( OpPtr->reachtime -0 )/ ( max_time - 0);
                ST = 1 * ( OpPtr->starttime -0 )/ ( max_time - 0);
                FT = 1 * ( OpPtr->finishtime -0 )/ ( max_time - 0);
                if( prob->get_request_by_index(OpPtr->id).win_open <= 0 )
                    TW_OPEN = 0;
                else
                    TW_OPEN = ( prob->get_request_by_index(OpPtr->id).win_open -0 )/ ( max_time - 0);
                if( prob->get_request_by_index(OpPtr->id).win_close <= 0 )
                    TW_CLOSE = 1;
                else
                    TW_CLOSE = ( prob->get_request_by_index(OpPtr->id).win_close -0 )/ ( max_time - 0);
                _FILE << "% request at node " << OpPtr->id_node << endl;
            }
            else
            {
                x_screen = 1 * ( prob->get_node_by_index(OpPtr->id_node).longitude - min_long) / ( max_long - min_long);
                y_screen = 1 * ( prob->get_node_by_index(OpPtr->id_node).lattitude - min_latt) / ( max_latt - min_latt);
                z_screen = 1 * ( OpPtr->reachtime -0 )/ ( max_time - 0);
                if( prob->get_depot_by_index(OpPtr->id).win_open <= 0 )
                    TW_OPEN = 0;
                else
                    TW_OPEN = ( prob->get_depot_by_index(OpPtr->id).win_open -0 )/ ( max_time - 0);
                if( prob->get_depot_by_index(OpPtr->id).win_close <= 0 )
                    TW_CLOSE = 1;
                else
                    TW_CLOSE = ( prob->get_depot_by_index(OpPtr->id).win_close -0 )/ ( max_time - 0);
                AT = 1 * ( OpPtr->reachtime -0 )/ ( max_time - 0);
                ST = 1 * ( OpPtr->starttime -0 )/ ( max_time - 0);
                FT = 1 * ( OpPtr->finishtime -0 )/ ( max_time - 0);
                _FILE << "% at depot " << OpPtr->id <<" (node="<<OpPtr->id_node<<")" << endl;
            }
            if (TW_CLOSE > 1)
                TW_CLOSE = 1.0;
            //_FILE << "\\tdplotgetpolarcoords{"<<x_screen+0.001<<"}{"<<y_screen+0.001<<"}{"<<z_screen+0.001<<"};" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<") at ("<<x_screen<<","<<y_screen<<","<<z_screen<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"x) at ("<<x_screen<<","<<0<<","<<0<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"y) at ("<<0<<","<<y_screen<<","<<0<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"z) at ("<<0<<","<<0<<","<<z_screen<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"xy) at ("<<x_screen<<","<<y_screen<<","<<0<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"yz) at ("<<0<<","<<y_screen<<","<<z_screen<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"xz) at ("<<x_screen<<","<<0<<","<<z_screen<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"twopen) at ("<<x_screen<<","<<y_screen<<","<<TW_OPEN<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"twclose) at ("<<x_screen<<","<<y_screen<<","<<TW_CLOSE<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"MAX) at ("<<x_screen<<","<<y_screen<<","<<1<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"MAXx) at ("<<x_screen<<","<<0<<","<<1<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"MAXy) at ("<<0<<","<<y_screen<<","<<1<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"MAXz) at ("<<0<<","<<0<<","<<1<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"MAXxy) at ("<<x_screen<<","<<y_screen<<","<<1<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"MAXyz) at ("<<0<<","<<y_screen<<","<<1<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"MAXxz) at ("<<x_screen<<","<<0<<","<<1<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"AT) at ("<<x_screen<<","<<y_screen<<","<<AT<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"ST) at ("<<x_screen<<","<<y_screen<<","<<ST<<");" << endl;
            _FILE << "\\coordinate (PT"<<PT_CNTR<<"FT) at ("<<x_screen<<","<<y_screen<<","<<FT<<");" << endl;

            _FILE << "---------------------------" << endl;
            PT_CNTR++;
            OpPtr = OpPtr->next;
        }
    }

     // now draw the locations of the xy-plane
    _FILE << "% draw the routes" << endl;
    PT_CNTR=0;
    for( int i=0 ; i < this->routes ; i++ )
    if( _DISPLAYED[i] == VRP_PP_TRUE )
    {
        OpPtr = this->route[i]->start;
        while( OpPtr != NULL )
        {
           if( (OpPtr->type == VRP_PP_PICKUP) || (OpPtr->type == VRP_PP_DELIVERY) || (OpPtr->type == VRP_PP_TRANSIT) )
            {
                // we have customer location
                _FILE << "% customer location" << endl;
                _FILE << "% the projection to the xy-plane" << endl;
                _FILE << "\\draw node[anchor=south, color=Black] at (PT"<<PT_CNTR<<"twclose) {\\tiny "<<OpPtr->id<<"};" << endl;
                //_FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"y) -- (PT"<<PT_CNTR<<"xy);" << endl;
                // draw vertical line
                _FILE << "% the vertical line out of the xy-plane" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"xy) -- (PT"<<PT_CNTR<<"twopen);" << endl;
                _FILE << "\\draw[line cap=rect, line width=0.25mm, double distance = 2pt, color=gray] (PT"<<PT_CNTR<<"twopen) -- (PT"<<PT_CNTR<<"twclose);" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"twclose) -- (PT"<<PT_CNTR<<"MAX);" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"MAXx) -- (PT"<<PT_CNTR<<"MAX);" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"x) -- (PT"<<PT_CNTR<<"MAXxz);" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"MAXy) -- (PT"<<PT_CNTR<<"MAX);" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"y) -- (PT"<<PT_CNTR<<"MAXyz);" << endl;

            }
            else
            {
                // we have a depot
                _FILE << "% depot" << endl;
                _FILE << "% the projection to the xy-plane" << endl;
                _FILE << "\\draw node[anchor=south, color=Black] at (PT"<<PT_CNTR<<"twclose) {\\tiny D"<<OpPtr->id<<"};" << endl;
                //_FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"y) -- (PT"<<PT_CNTR<<"xy);" << endl;
                // draw vertical line
                _FILE << "% the vertical line out of the xy-plane" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"xy) -- (PT"<<PT_CNTR<<"twopen);" << endl;
                _FILE << "\\draw[line cap=rect, line width=0.25mm, double distance = 2pt, color=gray] (PT"<<PT_CNTR<<"twopen) -- (PT"<<PT_CNTR<<"twclose);" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"twclose) -- (PT"<<PT_CNTR<<"MAX);" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"MAXx) -- (PT"<<PT_CNTR<<"MAX);" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"x) -- (PT"<<PT_CNTR<<"MAXxz);" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"MAXy) -- (PT"<<PT_CNTR<<"MAX);" << endl;
                _FILE << "\\draw[dotted, thick, color=Gray] (PT"<<PT_CNTR<<"y) -- (PT"<<PT_CNTR<<"MAXyz);" << endl;

            }
            PT_CNTR++;
            OpPtr = OpPtr->next;
        }
    }

    // ... and now we come to the routes
    _FILE << "%and now the routes" << endl;
    PT_CNTR = 0;
    for( int i=0 ; i < this->routes ; i++ )
    if( _DISPLAYED[i] == VRP_PP_TRUE )
    {
        OpPtr = this->route[i]->start;
        while( OpPtr != NULL )
        {
            _FILE << "\\draw[dashed, ultra thick, color="<<COLOUR[i % COLOURS]<<"] (PT"<<PT_CNTR<<"AT) -- (PT"<<PT_CNTR<<"ST);" << endl;
            _FILE << "\\draw[dotted, ultra thick, color="<<COLOUR[i % COLOURS]<<"] (PT"<<PT_CNTR<<"ST) -- (PT"<<PT_CNTR<<"FT);" << endl;

            if( OpPtr->next != NULL )
            {
                _FILE << "\\draw[-stealth, ultra thick, color="<<COLOUR[i % COLOURS]<<"] (PT"<<PT_CNTR<<"FT) -- (PT"<<PT_CNTR+1<<"AT);" << endl;
                _FILE << "\\draw[dotted, thick, color="<<COLOUR[i % COLOURS]<<"] (PT"<<PT_CNTR<<"xy) -- (PT"<<PT_CNTR+1<<"xy);" << endl;
            }
            PT_CNTR++;
            OpPtr = OpPtr->next;
        }
    }

    // .. finally (hopefully), we print the vehicle agenda
    _FILE << "% we finally have the vehicle agenda" << endl;
    double zpos = 0.0;
    for( int i=0 ; i < this->routes ; i++ )
    if( _DISPLAYED[i] == VRP_PP_TRUE )
    {
        _FILE << "\\draw node[anchor=west, color="<<COLOUR[i % COLOURS]<<"] at (-0.4,0.9,"<<zpos<<"){\\tiny route "<<i<<"};"<<endl;
        zpos = zpos+0.1;
    }
        // conclude this picture
    _FILE << "\\end{tikzpicture}}" << endl;

}

void VRP_PP_PLAN::VRPTW_SIMPLE_HEURISTIC(VRP_PP_PROBLEM *prob, struct VRP_PP_CONFIG *_CFG)
{
    cout << "Dies ist die simple Heuristik" << endl;
}

void VRP_PP_PLAN::print_as_kml(std::ostream& stream, const VRP_PP_PROBLEM*prob, bool PrintBubbles)const
{
	// VRP_PP_TRACE trace(__FUNCTION__);
	// assert_pointer(prob);

	using namespace std;
	// declare auxiliary variables
	VRP_PP_OPERATION *lauf;

	// print the xml-header
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
	stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << "\n";
	stream << "<Document>" << "\n";

	// first print all locations if desired
	if( PrintBubbles == true )
    {

	for (int r = 0; r < this->routes; r++)
	{
		if (this->route[r]->start->get_next() != this->route[r]->end)
		{
            // non-empty route found
			lauf = this->route[r]->start;
			while (lauf->get_next() != NULL)
			{
				const VRP_PP_NODE*node;

				stream << "<Placemark>" << "\n";
				if (lauf->get_type() == VRP_PP_START || lauf->get_type() == VRP_PP_STOP)
				{
					node = &prob->get_node_by_index(prob->get_depot_by_index(lauf->get_id()).id_node);
					stream << "<name>depot " << lauf->get_id() << "</name>" << "\n";
				}
				else
				{
					stream << "<name>request " << lauf->get_id() << "</name>" << "\n";
					node = &prob->get_node_by_index(prob->get_request_by_index(lauf->get_id()).id_node);
				}

				stream << "<description>" << lauf->get_desc() << "</description>" << "\n";
				stream << "<Point>" << "\n";
				stream << "<coordinates>" << node->longitude << "," << node->lattitude<< "," << 0 << "</coordinates>" << "\n";
				stream << "</Point>" << "\n";
				stream << "</Placemark>" << "\n";
				lauf = lauf->get_next();
			}
		}
	}
    }

	// define 10 different line colors
	static string color[] = {
		"000000",    // black
		"191970",    // blue
		"FF0000",    // red
		"00FF00",    // green
		"EE00EE",    // oliv
		"00CED1",    // turquois
		"DAA520",    // gold
		"FFC1C1",    // orange
		"0000FF",    // pink
		"00F000",    // yellow
	};

	// now print the lines
	for (int r = 0; r < this->routes; r++)
	{
		if (this->route[r]->start->get_next() != this->route[r]->end)
		{
			stream << "<Placemark>" << "\n";
			//stream << "<Placemark>" << "\n";
			stream << "<name>route - ID" << this->route[r]->route_id << "</name>" << "\n";
			stream << "<description>route of vehicle " << this->route[r]->vehicle_num << "</description>" << "\n";
			stream << "<Style>" << "\n";
			stream << "<LineStyle>" << "\n";
			stream << "<color>" << color[r % int(color->size())] << "</color>" << "\n";
			stream << "<width>3</width>" << "\n";
			stream << "</LineStyle>" << "\n";
			stream << "</Style>" << "\n";
			stream << "<LineString>" << "\n";
			stream << "<extrude>1</extrude>" << "\n";
			stream << "<tessellate>1</tessellate>" << "\n";
			stream << "<altitudeMode>absolute</altitudeMode>" << "\n";
			stream << "<coordinates>" << "\n";
			lauf = this->route[r]->start;
			while (lauf != NULL)
			{
				stream << prob->get_node_by_index(lauf->get_id_node()).longitude << "," << prob->get_node_by_index(lauf->get_id_node()).lattitude<< "," << 0 << "\n";
				lauf = lauf->get_next();
			}
			stream << "</coordinates>" << "\n";
			stream << "</LineString>" << "\n";
			stream << "</Placemark>" << "\n";
		}
	}


	// print the xml-footer
	stream << "</Document>" << "\n";
	stream << "</kml>" << "\n";

	stream.flush();
}
