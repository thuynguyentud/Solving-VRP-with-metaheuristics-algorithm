#include <iostream>
#include <iomanip>
#include <math.h>
#include <string.h>
#include "irrXML.h"

using namespace std;
using namespace irr; // irrXML is located in the namespace irr::io
using namespace io;

#include "vrp_pp_load_item.h"

#include "vrp_pp_problem.h"
#include "vrp_pp_operation.h"
#include "pugixml.hpp"

// Version 2.0 - last modificaton 25.02.2020 at 17:53


void VRP_PP_PROBLEM::print(void)
{
	int i;
	using namespace std;

	cout << "nodes" << endl;
	for (i = 0; i < this->nodes; i++)
	{
		cout << "node_" << this->node[i].id;
		cout << " at (" << this->node[i].x << ";" << this->node[i].y << ") ";
		cout << "geo - coordinates (" << this->node[i].longitude << ";" << this->node[i].lattitude << ")";
		cout << " " << this->node[i].desc << endl;
	}
	cout << "arcs" << endl;
	for (i = 0; i < this->arcs; i++)
	{
		cout << "arc-id: ";
		cout << this->arc[i].id << ": ",
			cout << "from node " << this->arc[i].origin;
		cout << " to node " << this->arc[i].dest;
		cout << " length: " << this->arc[i].length_euclid;
		cout << " time: " << this->arc[i].time;
		cout << " customerWeight: " << this->arc[i].length_custom;
		cout << " " << this->arc[i].desc << endl;
	}

	cout << "The distance matrice (Euclidean length)" << endl;
	for (int i = 0; i < this->nodes; i++)
	{
		for (int j = 0; j < this->nodes; j++)
		{
			cout << setw(8) << this->ARC[i][j].length_euclid;
		}
		cout << endl;
	}
	cout << "The distance matrice (travel time)" << endl;
	for (int i = 0; i < this->nodes; i++)
	{
		for (int j = 0; j < this->nodes; j++)
		{
			cout << setw(8) << this->ARC[i][j].time;
		}
		cout << endl;
	}
	cout << "The distance matrice (LatLong)" << endl;
	for (int i = 0; i < this->nodes; i++)
	{
		for (int j = 0; j < this->nodes; j++)
		{
			cout << setw(8) << this->ARC[i][j].length_latlong;
		}
		cout << endl;
	}

	cout << "The distance matrice (CustomerWeight)" << endl;
	for (int i = 0; i < this->nodes; i++)
	{
		for (int j = 0; j < this->nodes; j++)
		{
			cout << setw(8) << this->ARC[i][j].length_custom;
		}
		cout << endl;
	}
	cout << "vehicles" << endl;
	for (i = 0; i < this->vehicles; i++)
	{
		cout << "vehicle " << i << " capacity: " << this->vehicle[i].cap << " speed: " << vehicle[i].speed << endl;
	}
	cout << "requests" << endl;
	for (i = 0; i < this->requests; i++)
	{
		cout << "request: " << i << " ";
		cout << "node: " << this->request[i].id_node << " ";
		cout << "quantity: " << this->request[i].quantity << " ";
		cout << "win_open: " << this->request[i].win_open << " ";
		cout << "win_close: " << this->request[i].win_close << " ";
		cout << "group: " << this->request[i].group << endl;
	}
    cout << "load items" << endl;
	for (i = 0; i < this->load_items; i++)
	{
		this->load_item[i].print();
	}
}

void VRP_PP_PROBLEM::print_as_kml(std::ostream& stream)const
{
	//VRP_PP_TRACE trace(__FUNCTION__);

	//using namespace std;

	// print the xml-header
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
	stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << "\n";
	stream << "<Document>" << "\n";

	// print the depot(s)
	for (int i = 0; i < this->depots; i++)
	{
		stream << "<Placemark>" << "\n";
		stream << "<name>depot " << i << "</name>" << "\n";
		stream << "<description>" << this->depot[i].desc << "</description>" << "\n";
		stream << "<Point>" << "\n";
		stream << "<coordinates>" << this->node[this->depot[i].id_node].longitude << "," << this->node[this->depot[i].id_node].lattitude << "," << 0 << "</coordinates>" << "\n";
		stream << "</Point>" << "\n";
		stream << "</Placemark>" << "\n";
	}

	// print the requests(s)
	for (int i = 0; i < this->requests; i++)
	{
		stream << "<Placemark>" << "\n";
		stream << "<name>request " << i << "</name>" << "\n";
		stream << "<description>" << this->node[this->request[i].id_node].desc << " (quantity: " << this->request[i].quantity << ")</description>" << "\n";
		stream << "<Point>" << "\n";
		stream << "<coordinates>" << this->node[this->request[i].id_node].longitude << "," << this->node[this->request[i].id_node].lattitude << "," << 0 << "</coordinates>" << "\n";
		stream << "</Point>" << "\n";
		stream << "</Placemark>" << "\n";
	}

	// print the CVRP star
	for (int i = 0; i < this->requests; i++)
	{
		stream << "<Placemark>" << "\n";
		stream << "<name>request " << this->request[i].id << "</name>" << "\n";
		stream << "<description>" << this->request[i].desc << "</description>" << "\n";
		stream << "<Style>" << "\n";
		stream << "<LineStyle>" << "\n";
		stream << "<color>000000</color>" << "\n";
		stream << "<width>3</width>" << "\n";
		stream << "</LineStyle>" << "\n";
		stream << "</Style>" << "\n";
		stream << "<LineString>" << "\n";
		stream << "<altitudeMode>absolute</altitudeMode>" << "\n";
		stream << "<coordinates>" << "\n";
		stream << this->node[this->depot[0].id_node].longitude << "," << this->node[this->depot[0].id_node].lattitude << "," << 0 << "\n";
		stream << this->node[this->request[i].id_node].longitude << "," << this->node[this->request[i].id_node].lattitude << "," << 0 << "\n";
		stream << "</coordinates>" << "\n";
		stream << "</LineString>" << "\n";
		stream << "</Placemark>" << "\n";
	}
	// print the xml-footer
	stream << "</Document>" << "\n";
	stream << "</kml>" << "\n";


	stream.flush();
}

void VRP_PP_PROBLEM::print_network_as_kml(std::ostream& stream)const
{
	//VRP_PP_TRACE trace(__FUNCTION__);

	//using namespace std;

	// print the xml-header
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
	stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << "\n";
	stream << "<Document>" << "\n";

	// print the node(s)
	for (int i = 0; i < this->nodes; i++)
	{
		stream << "<Placemark>" << "\n";
		stream << "<name>node " << i << "</name>" << "\n";
		stream << "<description>" << this->node[i].id << "</description>" << "\n";
		stream << "<Point>" << "\n";
		stream << "<coordinates>" << this->node[i].longitude << "," << this->node[i].lattitude << "," << 0 << "</coordinates>" << "\n";
		stream << "</Point>" << "\n";
		stream << "</Placemark>" << "\n";
	}

        /*
	// print the requests(s)
	for (int i = 0; i < this->requests; i++)
	{
		stream << "<Placemark>" << "\n";
		stream << "<name>request " << i << "</name>" << "\n";
		stream << "<description>" << this->node[this->request[i].id_node].desc << " (quantity: " << this->request[i].quantity << ")</description>" << "\n";
		stream << "<Point>" << "\n";
		stream << "<coordinates>" << this->node[this->request[i].id_node].longitude << "," << this->node[this->request[i].id_node].lattitude << "," << 0 << "</coordinates>" << "\n";
		stream << "</Point>" << "\n";
		stream << "</Placemark>" << "\n";
	}
    */
	// print the arcs
	for (int i = 0; i < this->arcs; i++)
	{
		stream << "<Placemark>" << "\n";
		stream << "<name>arc " << this->arc[i].id << "</name>" << "\n";
		stream << "<description>" << this->arc[i].id << "</description>" << "\n";
		stream << "<Style>" << "\n";
		stream << "<LineStyle>" << "\n";
		stream << "<color>000000</color>" << "\n";
		stream << "<width>3</width>" << "\n";
		stream << "</LineStyle>" << "\n";
		stream << "</Style>" << "\n";
		stream << "<LineString>" << "\n";
		stream << "<altitudeMode>absolute</altitudeMode>" << "\n";
		stream << "<coordinates>" << "\n";
		stream << this->node[this->arc[i].origin].longitude << "," << this->node[this->arc[i].origin].lattitude << "," << 0 << "\n";
		stream << this->node[this->arc[i].dest].longitude << "," << this->node[this->arc[i].dest].lattitude << "," << 0 << "\n";
		stream << "</coordinates>" << "\n";
		stream << "</LineString>" << "\n";
		stream << "</Placemark>" << "\n";
	}

	// print the xml-footer
	stream << "</Document>" << "\n";
	stream << "</kml>" << "\n";


	stream.flush();
}

void VRP_PP_PROBLEM::print_nodes_as_kml(std::ostream& stream)const
{
	//VRP_PP_TRACE trace(__FUNCTION__);

	//using namespace std;

	// print the xml-header
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
	stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << "\n";
	stream << "<Document>" << "\n";

	// print the node(s)
	for (int i = 0; i < this->nodes; i++)
	{
		stream << "<Placemark>" << "\n";
		stream << "<name>node " << i << "</name>" << "\n";
		stream << "<description>" << this->node[i].id << "</description>" << "\n";
		stream << "<Point>" << "\n";
		stream << "<coordinates>" << this->node[i].longitude << "," << this->node[i].lattitude << "," << 0 << "</coordinates>" << "\n";
		stream << "</Point>" << "\n";
		stream << "</Placemark>" << "\n";
	}

	// print the xml-footer
	stream << "</Document>" << "\n";
	stream << "</kml>" << "\n";

	stream.flush();
}

int VRP_PP_PROBLEM::get_vehicle_ctr() const
{
	return vehicles;
}

int VRP_PP_PROBLEM::get_depot_ctr() const
{
	return depots;
}

int VRP_PP_PROBLEM::get_node_ctr() const
{
	return nodes;
}

int VRP_PP_PROBLEM::get_request_ctr() const
{
	return requests;
}

int VRP_PP_PROBLEM::get_arc_ctr() const
{
	return arcs;
}

int VRP_PP_PROBLEM::get_load_item_ctr() const
{
	return load_items;
}
double VRP_PP_PROBLEM::get_dist_euclid(int node1_index, int node2_index) const
{
	return ARC[node1_index][node2_index].length_euclid;
}

double VRP_PP_PROBLEM::get_dist_latlong(int node1_index, int node2_index) const
{
	return ARC[node1_index][node2_index].length_latlong;
}

double VRP_PP_PROBLEM::get_dist_custom(int node1_index, int node2_index) const
{
	return ARC[node1_index][node2_index].length_custom;
}

void VRP_PP_PROBLEM::set_dist_euclid(double value, int node1_index, int node2_index)
{
	this->ARC[node1_index][node2_index].length_euclid = value;
}

void VRP_PP_PROBLEM::set_dist_latlong(double value, int node1_index, int node2_index)
{
	this->ARC[node1_index][node2_index].length_latlong = value;
}

void VRP_PP_PROBLEM::set_dist_custom(double value, int node1_index, int node2_index)
{
	this->ARC[node1_index][node2_index].length_custom = value;
}

const VRP_PP_ARC & VRP_PP_PROBLEM::get_arc_by_od(int origin, int dest) const
{
    return ARC[origin][dest];
}

const VRP_PP_ARC & VRP_PP_PROBLEM::get_arc_by_index(int index) const
{
	return arc[index];
}

const VRP_PP_NODE & VRP_PP_PROBLEM::get_node_by_index(int index) const
{
	return node[index];
}

const VRP_PP_LOAD_ITEM & VRP_PP_PROBLEM::get_load_item_by_index(int index) const
{
	return load_item[index];
}

const VRP_PP_REQUEST & VRP_PP_PROBLEM::get_request_by_index(int index) const
{
	return request[index];
}

const VRP_PP_REQUEST & VRP_PP_PROBLEM::get_depot_by_index(int index) const
{
	return depot[index];
}

const VRP_PP_VEHICLE & VRP_PP_PROBLEM::get_vehicle_by_index(int id) const
{
	return vehicle[id];
}

VRP_PP_PROBLEM::~VRP_PP_PROBLEM()
{
	clear();
}

VRP_PP_PROBLEM::VRP_PP_PROBLEM(const std::string& filename)
{
	// declare the counters
	int NODES = 0;
	int ARCS = 0;
	int DEPOTS = 0;
	int VEHICLES = 0;
	int REQUESTS = 0;
	int LOAD_ITEMS = 0;

	// declare and establish the handler to the xml-file
	IrrXMLReader* xml = createIrrXMLReader(&filename[0]);

	// first read for the determination of the counters
	while (xml && xml->read())
	{
		if (xml->getNodeType() != 3)
		{

			if (strcmp("node", xml->getNodeName()) == 0)
			{
				NODES++;
			}else
			if (strcmp("arc", xml->getNodeName()) == 0)
				ARCS++;
			if (strcmp("depot", xml->getNodeName()) == 0)
				DEPOTS++;
			if (strcmp("vehicle", xml->getNodeName()) == 0)
				VEHICLES++;
			if (strcmp("request", xml->getNodeName()) == 0)
				REQUESTS++;
            if (strcmp("load_item", xml->getNodeName()) == 0)
				LOAD_ITEMS++;
		}
	}

	// delete the xml parser after usage
	delete xml;

	// initialize the distance-matrix;
	this->ARC = new VRP_PP_ARC*[NODES];
	for (int i = 0; i < NODES; i++)
	{
		this->ARC[i] = new VRP_PP_ARC[NODES];
		for (int j = 0; j < NODES; j++)
		{
			this->ARC[i][j].dest = j;
			this->ARC[i][j].origin = i;

			if (i == j)ARC[i][j].length_euclid = ARC[i][j].length_latlong = ARC[i][j].length_custom = ARC[i][j].time = VRP_PP_BIGM;
		}
	}

	//printf("NODES=%d\n", NODES);
	this->nodes = NODES;
	this->node = new VRP_PP_NODE[this->nodes];
	int CNTR_NODES = 0;

	//printf("ARCS=%d\n", ARCS);
	this->arcs = ARCS;
	this->arc = new VRP_PP_ARC[this->arcs];
	int CNTR_ARCS = 0;

	//printf("DEPOTS=%d\n", DEPOTS);
	this->depots = DEPOTS;
	this->depot = new VRP_PP_REQUEST[this->depots];
	int CNTR_DEPOTS = 0;

	//printf("VEHICLES=%d\n", VEHICLES);
	this->vehicles = VEHICLES;
	this->vehicle = new VRP_PP_VEHICLE[this->vehicles];
	int CNTR_VEHICLES = 0;

	//printf("REQUESTS=%d\n", REQUESTS);
	this->requests = REQUESTS;
	this->request = new VRP_PP_REQUEST[this->requests];
	int CNTR_REQUESTS = 0;

    //printf("LOAD_ITEMS=%d\n", LOAD_ITEMS);
	this->load_items = LOAD_ITEMS;
	this->load_item = new VRP_PP_LOAD_ITEM[this->load_items];
	int CNTR_LOAD_ITEMS = 0;

	// second read; fill the problem structure

	// declare and establish the handler to the xml-file
	IrrXMLReader* xml2 = createIrrXMLReader(&filename[0]);

	while (xml2 && xml2->read())
	{
		if (xml2->getNodeType() != 3)
		{
			if (strcmp("node", xml2->getNodeName()) == 0)
			{
				this->node[CNTR_NODES].id = xml2->getAttributeValueAsInt("id");
				this->node[CNTR_NODES].desc = xml2->getAttributeValueSafe("desc");
				this->node[CNTR_NODES].x = xml2->getAttributeValueAsFloat("x");
				this->node[CNTR_NODES].y = xml2->getAttributeValueAsFloat("y");
				this->node[CNTR_NODES].lattitude = xml2->getAttributeValueAsFloat("latt");
				this->node[CNTR_NODES].longitude = xml2->getAttributeValueAsFloat("long");
				CNTR_NODES++;
			}
			if (strcmp("arc", xml2->getNodeName()) == 0)
			{
				this->arc[CNTR_ARCS].id = xml2->getAttributeValueAsInt("id");
				this->arc[CNTR_ARCS].desc = xml2->getAttributeValueSafe("desc");
				this->arc[CNTR_ARCS].origin = xml2->getAttributeValueAsInt("origin");
				this->arc[CNTR_ARCS].dest = xml2->getAttributeValueAsInt("dest");
				this->arc[CNTR_ARCS].length_euclid = dist_euclid(node[arc[CNTR_ARCS].origin], node[arc[CNTR_ARCS].dest]);
				//cout << this->arc[CNTR_ARCS].origin << "->" << this->arc[CNTR_ARCS].dest << ": " <<dist_euclid(node[arc[CNTR_ARCS].origin], node[arc[CNTR_ARCS].dest]) << endl;
				const double dist_latlong = dist_lattlong(node[arc[CNTR_ARCS].origin], node[arc[CNTR_ARCS].dest]);
				arc[CNTR_ARCS].length_custom = xml2->getAttributeValueAsFloat("customWeight");
				this->arc[CNTR_ARCS].length_latlong = dist_latlong;
				this->arc[CNTR_ARCS].time = xml2->getAttributeValueAsFloat("time");

				// update distance matrice
				const int origin = this->arc[CNTR_ARCS].origin;
				const int dest = this->arc[CNTR_ARCS].dest;
				this->ARC[origin][dest].id = this->arc[CNTR_ARCS].id;
				this->ARC[origin][dest].desc = this->arc[CNTR_ARCS].desc;
				this->ARC[origin][dest].origin = this->arc[CNTR_ARCS].origin;
				this->ARC[origin][dest].dest = this->arc[CNTR_ARCS].dest;
				this->ARC[origin][dest].length_euclid = this->arc[CNTR_ARCS].length_euclid;
				this->ARC[origin][dest].length_latlong= this->arc[CNTR_ARCS].length_latlong;
				this->ARC[origin][dest].time = this->arc[CNTR_ARCS].time;
				ARC[origin][dest].length_custom = arc[CNTR_ARCS].length_custom;
				CNTR_ARCS++;
			}
			if (strcmp("depot", xml2->getNodeName()) == 0)
			{
				depot[CNTR_DEPOTS].id = xml2->getAttributeValueAsInt("id");
				depot[CNTR_DEPOTS].quantity = xml2->getAttributeValueAsFloat("quantity");
				depot[CNTR_DEPOTS].desc = xml2->getAttributeValue("desc");
				depot[CNTR_DEPOTS].id_node = xml2->getAttributeValueAsInt("node");
				depot[CNTR_DEPOTS].win_open = xml2->getAttributeValueAsInt("win_open");
				depot[CNTR_DEPOTS].win_close = xml2->getAttributeValueAsInt("win_close");
				depot[CNTR_DEPOTS].duration = xml2->getAttributeValueAsFloat("duration");
				const int type = xml2->getAttributeValueAsInt("type");
				switch (type)
				{
				case VRP_PP_DELIVERY:
					depot[CNTR_DEPOTS].type = VRP_PP_DELIVERY;
					break;
				case VRP_PP_PICKUP:
					depot[CNTR_DEPOTS].type = VRP_PP_PICKUP;
					break;
				case VRP_PP_START:
					depot[CNTR_DEPOTS].type = VRP_PP_START;
					break;
				case VRP_PP_STOP:
					depot[CNTR_DEPOTS].type = VRP_PP_STOP;
					break;
				default:
					depot[CNTR_DEPOTS].type = VRP_PP_DUMMY;
				}
				CNTR_DEPOTS++;
			}
			if (strcmp("vehicle", xml2->getNodeName()) == 0)
			{
				this->vehicle[CNTR_VEHICLES].id = xml2->getAttributeValueAsInt("id");
				this->vehicle[CNTR_VEHICLES].cap = xml2->getAttributeValueAsFloat("cap");
				this->vehicle[CNTR_VEHICLES].speed = xml2->getAttributeValueAsFloat("speed");
				this->vehicle[CNTR_VEHICLES].id_depot = xml2->getAttributeValueAsInt("startnode");
				this->vehicle[CNTR_VEHICLES].id_node = depot[vehicle[CNTR_VEHICLES].id_depot].id_node;
				this->vehicle[CNTR_VEHICLES].desc = xml2->getAttributeValue("desc");
				CNTR_VEHICLES++;
			}
			if (strcmp("request", xml2->getNodeName()) == 0)
			{
				this->request[CNTR_REQUESTS].id = xml2->getAttributeValueAsInt("id");
				this->request[CNTR_REQUESTS].quantity = xml2->getAttributeValueAsFloat("quantity");
				this->request[CNTR_REQUESTS].desc = xml2->getAttributeValue("desc");
				this->request[CNTR_REQUESTS].id_node = xml2->getAttributeValueAsInt("node");
				this->request[CNTR_REQUESTS].win_open = xml2->getAttributeValueAsInt("win_open");
				this->request[CNTR_REQUESTS].win_close = xml2->getAttributeValueAsInt("win_close");
				this->request[CNTR_REQUESTS].duration = xml2->getAttributeValueAsFloat("duration");
				this->request[CNTR_REQUESTS].group = xml2->getAttributeValueAsInt("group");     // added in 3.0
				const int type = xml2->getAttributeValueAsInt("type");
				switch (type)
				{
				case VRP_PP_DELIVERY:
					request[CNTR_REQUESTS].type = VRP_PP_DELIVERY;
					break;
				case VRP_PP_PICKUP:
					request[CNTR_REQUESTS].type = VRP_PP_PICKUP;
					break;
				default:
					request[CNTR_REQUESTS].type = VRP_PP_DUMMY;
				}
				CNTR_REQUESTS++;
			}
            if (strcmp("load_item", xml2->getNodeName()) == 0)
			{
				this->load_item[CNTR_LOAD_ITEMS].id = xml2->getAttributeValueAsInt("id");
				this->load_item[CNTR_LOAD_ITEMS].request = xml2->getAttributeValueAsInt("request");
                this->load_item[CNTR_LOAD_ITEMS].desc = xml2->getAttributeValue("desc");
                this->load_item[CNTR_LOAD_ITEMS].item_width = xml2->getAttributeValueAsFloat("item_width");
                this->load_item[CNTR_LOAD_ITEMS].item_length = xml2->getAttributeValueAsFloat("item_length");
                this->load_item[CNTR_LOAD_ITEMS].item_height = xml2->getAttributeValueAsFloat("item_height");
                this->load_item[CNTR_LOAD_ITEMS].cog_width = xml2->getAttributeValueAsFloat("cog_width");
                this->load_item[CNTR_LOAD_ITEMS].cog_length = xml2->getAttributeValueAsFloat("cog_length");
                this->load_item[CNTR_LOAD_ITEMS].cog_height = xml2->getAttributeValueAsFloat("cog_height");
                this->load_item[CNTR_LOAD_ITEMS].item_weight = xml2->getAttributeValueAsFloat("item_weight");
                this->load_item[CNTR_LOAD_ITEMS].flipping = xml2->getAttributeValueAsInt("flipping");
				CNTR_LOAD_ITEMS++;
			}
		}
	}


	delete xml2;
}

VRP_PP_PROBLEM::VRP_PP_PROBLEM(const VRP_PP_PROBLEM & source)
	: vehicles(source.vehicles), depots(source.depots), nodes(source.nodes), requests(source.requests), arcs(source.arcs),
	vehicle(deep_copy_array(source.vehicle, vehicles)), depot(deep_copy_array(source.depot, depots)), node(deep_copy_array(source.node, nodes)), request(deep_copy_array(source.request, requests)),
	arc(deep_copy_array(source.arc, arcs)), ARC(deep_copy_2Darray(source.ARC, nodes, nodes)),load_item(deep_copy_array(source.load_item, load_items))
{
}

VRP_PP_PROBLEM & VRP_PP_PROBLEM::operator=(const VRP_PP_PROBLEM & source)
{
	// check for self assignment
	if (this == &source)
		return *this;

	clear();

	vehicles = source.vehicles;
	vehicle = deep_copy_array(source.vehicle, vehicles);

	depots = source.depots;
	depot = deep_copy_array(source.depot, depots);

	nodes = source.nodes;
	node = deep_copy_array(source.node, nodes);

	requests = source.requests;
	request = deep_copy_array(source.request, requests);

	arcs = source.arcs;
	arc = deep_copy_array(source.arc, arcs);

	load_items = source.load_items;
	load_item = deep_copy_array(source.load_item, load_items);

	ARC = deep_copy_2Darray(source.ARC, nodes, nodes);

	return *this;
}



void VRP_PP_PROBLEM::clear()
{
	delete[] this->node;
	delete[] this->arc;
	delete[] this->request;
	delete[] this->vehicle;
	delete[] this->depot;
	delete[] this->load_item;

	for (int i = 0; i < this->nodes; i++)
	{
		delete[] this->ARC[i];
	}
	delete[] this->ARC;
}


void VRP_PP_PROBLEM::append_vehicle(VRP_PP_VEHICLE *veh)
{
    // JSB
    // this procedure appends another vehicle to the list of saved vehicles
    // it is needed for the savings algorithm
    if( veh != nullptr)
    {
        // first, save vehicles from problem description
        VRP_PP_VEHICLE TMP_VEH[this->vehicles];
        for( int i=0 ; i < this->vehicles ; i++)
        {
            TMP_VEH[i] = this->vehicle[i];
        }
        // second, delete original vehicles
        delete[] this->vehicle;
        // third, get the memory for the extended vehicle list
        this->vehicle = new VRP_PP_VEHICLE [this->vehicles+1];
        // fourth, restore saved vehicles
        for( int i=0 ; i < this->vehicles ; i++)
        {
            this->vehicle[i] = TMP_VEH[i];
        }
        // fifth, store the new vehicle in the last array entry
        this->vehicle[this->vehicles] = *veh;
        // increase the number of saved vehicles
        this->vehicles++;
    }
}

void VRP_PP_PROBLEM::delete_last_vehicle(void)
{
    // JSB
    // this procedure delete the last vehicle from the list of saved vehicles
    // it is needed for the savings algorithm
    if( this->vehicles > 0 )
    {
        // first, save vehicles from problem description except the last one
        VRP_PP_VEHICLE TMP_VEH[this->vehicles];
        for( int i=0 ; i < this->vehicles-1 ; i++)
        {
            TMP_VEH[i] = this->vehicle[i];
        }
        // second, delete original vehicles
        delete[] this->vehicle;
        // third, get the memory for the extended vehicle list
        this->vehicle = new VRP_PP_VEHICLE [this->vehicles-1];
        // fourth, restore saved vehicles
        for( int i=0 ; i < this->vehicles-1 ; i++)
        {
            this->vehicle[i] = TMP_VEH[i];
        }
        // decrease the number of saved vehicles
        this->vehicles--;
    }
}

void VRP_PP_PROBLEM::update_request_data(int Request_Id, struct VRP_PP_REQUEST _ReqData)
{
    this->request[Request_Id] = _ReqData;
}

void VRP_PP_PROBLEM::update_arc_data(int Arc_Id, struct VRP_PP_ARC _ArcData)
{
    this->arc[Arc_Id] = _ArcData;
}

void VRP_PP_PROBLEM::update_node_data(int Node_Id, struct VRP_PP_NODE _NodeData)
{
    this->node[Node_Id] = _NodeData;
}

void VRP_PP_PROBLEM::set_selfloops_zero()
{
    // added 16.06.2020 at 12:55
    // this procedure sets the length_custom - Attribute to 0 for all arc (i;j)

    for(int i=0 ; i < this->arcs ; i++)
    {
        if( this->arc[i].origin == this->arc[i].dest )
        {
            this->arc[i].length_custom = 0.0;
            this->ARC[arc[i].origin][this->arc[i].dest].length_custom  = 0.0;
         }
    }
}

void VRP_PP_PROBLEM::re_index_arcs(void)
{
    // added 08.03.2021 at 17:14
    // this procedure sets the arc-ids continuously from 0 to get_arc_ctr()-1
    int N = this->get_arc_ctr();
    for( int a=0 ; a < N ; a++ )
        this->arc[a].id = a;
}

const int VRP_PP_PROBLEM::get_request_id_by_node_id(int node_id)const
{
    int sel_req = -1;
        for( int r=0 ; r < this->requests ; r++ )
            if( this->request[r].id_node == node_id )
            {
                sel_req = r;
                r = this->requests+1;
            }

    return(sel_req);
}

void VRP_PP_PROBLEM::print_custom_arclength_as_xml(void)
{
    // this procedure prints the arc-list in XML-style using lattlong-distance as customWeight
    for( int a=0 ; a < this->arcs ; a++ )
    {
        cout << "<arc id=" << char(34) << a << char(34);
        cout << " origin=" << char(34) << this->arc[a].origin << char(34);
        cout << " dest=" << char(34) << this->arc[a].dest << char(34);
        cout << " customWeight=" << char(34) << this->arc[a].length_custom << char(34);
        cout << " desc=" << char(34) << this->arc[a].desc << char(34);
        cout << "/>" << endl;
    }
}

//code Thuy adds - add pugi read DOC
void VRP_PP_PROBLEM::read_fuel_consumption_values(struct VRP_PP_CONFIG *_Config)
{
    //declare the variable DOC to host the XML-files information
    pugi::xml_document DOC;

    //read the information from XML-file and load the information
    DOC.load_file(_Config->FILE.c_str());

    //declare an XML-node
    pugi::xml_node FIRST_VEHICLE_NODE;

    //navigate to the first vehicle tag
    FIRST_VEHICLE_NODE = DOC.child("vrpplusplus").child("fleet").child("vehicle");

    //now iterate over through all vehicles
    for(pugi::xml_node VEHICLE = FIRST_VEHICLE_NODE;
    VEHICLE;
    VEHICLE = VEHICLE.next_sibling("vehicle"))
    {
        //std::cout << "read fuel data of vehicle" << VEHICLE.attribute("id").as_int() << std::endl;

        //get the vehicle id
        int VEH_ID =VEHICLE.attribute("id").as_int();

        //set the values
        this->vehicle[VEH_ID].fuel_a = VEHICLE.attribute("fuel_a").as_double();
        this->vehicle[VEH_ID].fuel_b = VEHICLE.attribute("fuel_b").as_double();
        this->vehicle[VEH_ID].evw = VEHICLE.attribute("evw").as_double();

    }

}


//code Thuy adds - add SET 3 CONFIG VARIABLES

void VRP_PP_PROBLEM::set_3_var(int _nbh_size ,int _P1, int _P2)
{
    nbh_size = _nbh_size;
    P1 = _P1;
    P2 = _P2;
}


int VRP_PP_PROBLEM::get_nbh_size(void)
{
    return(nbh_size);

}
int VRP_PP_PROBLEM::get_P1(void)
{
    return(P1);
}

int VRP_PP_PROBLEM::get_P2(void)
{
    return(P2);
}


