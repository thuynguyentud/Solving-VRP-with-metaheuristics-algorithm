// this header file contains all those definitions
// needed by specialized classes and objects
//
// Version 3.0 - last modified 05.03.2021 - 08:17
// - introduction of a feasibility flag in the vrp_pp_evaluation structure

// Version 2.0 - last modified 25.02.2020

// first check if microsoft visual studio is used
#ifndef VRP_PP_GLOBAL_H
#define VRP_PP_GLOBAL_H

#include <string>
#include <exception>
#include <ostream>
#include <sstream>
#include <typeinfo>
#include <vector>
#include <thread>

using namespace std;

// the following names can be used in VRP++
#define VRP_PP_START 9
#define VRP_PP_STOP 8
#define VRP_PP_TRANSIT 10       // needed for stops in the SPP
#define VRP_PP_DELIVERY 0
#define VRP_PP_PICKUP 1

#define VRP_PP_DUMMY 0
#define VRP_PP_REAL 1

#define VRP_PP_TRUE 0
#define VRP_PP_FALSE -1

#define VRP_PP_EPSILON 0.000001
#define VRP_PP_BIGM 10000000.0
#define PI 3.14159265359

enum class VRP_PP_NODE_COORDINATES { GEO, EUCLID };

enum class VRP_PP_ARC_WEIGHT { GEO, EUCLID, CUSTOM };

//JSB these attributes are used to control if intermediate results are displayed
// NONE = no intermeidate results
// ITERACTIVE_SCREEN = displayed on the console
// ITERACTIVE_FILE = produces CURRENT_DATE_TIME_0.kml, CURRENT_DATE_TIME_1.kml, ...

enum class VRP_PP_DISPLAY { NONE, ITERACTIVE_SCREEN, ITERACTIVE_FILE };


// struct VRP_PP_eval_array
//
// this structure is used to save important / relevant evaluation results
// for a solution part in a VRP++-project

// Version 3.0 - last modified 05.03.2021 - 08:17
// - introduction of a feasibility flag in the vrp_pp_evaluation structure

struct VRP_PP_EVALUATION
{
	double obj_value;       // should carry a generalized objective function value (e.g. length + penalty)
	double length;          // should carry the total sum of travel distances in this obj.
	double travel_time;
	double cap_error;       // extent of vehicle capacity exceeding
	int cap_error_num;      // number of vehicles with observed capacity exceeding
	double win_error;       // extent of time window violations
	int win_error_num;      // number of requests with observed time window exceeding
	double cap_load;		// aktuelle Beladung
	double length_error;	// value of maximum distance constraint violation
	int length_error_num;	// number of maximum distance constraint violation

    //code Thuy adds - add 3 attributes: number of deployed vehicle, number of overloaded vehicles and number of overloaded pallet
    double used_veh_cntr ;    //number of deployed vehicle
    double veh_overload;  //number of overloaded vehicles
    double plt_overload;   //number of overloaded pallets
	int feasible;           // indicator if this solution is feasible; = -1 infeasible / = 1 feasible





	VRP_PP_EVALUATION();

	VRP_PP_EVALUATION& operator+=(const VRP_PP_EVALUATION& add);
	VRP_PP_EVALUATION& operator-=(const VRP_PP_EVALUATION&sub);
	VRP_PP_EVALUATION operator+(const VRP_PP_EVALUATION& add);
	VRP_PP_EVALUATION operator-(const VRP_PP_EVALUATION& sub);

	friend std::ostream& operator<<(std::ostream& stream, const VRP_PP_EVALUATION& eval);
private:
	VRP_PP_EVALUATION(double obj_value, double _length, double _travel_time, double _cap_error, int _cap_error_num, double _win_error, int _win_error_num,
		double _cap_load, double _length_error, int _length_error_num, double used_veh_cntr, double veh_overload, double plt_overload, int _feasible = 1); //code Thuy adds
};



//
// struct VRP_PP_NODE
//
// this structure is used to save node information in a VRP++-project

struct VRP_PP_NODE
{
	int id;
	double x;
	double y;
	double lattitude;	// lattitude geo-coordinates (decimal degree representation)
	double longitude;	// longitude geo-coordinates (decimal degree representation)
	std::string desc;

	VRP_PP_NODE(int _id = -1, double _x = 0, double _y = 0, double _lattitude = 0, double _longitude = 0, const std::string& _desc = "not yet defined");


	friend double dist_euclid(const VRP_PP_NODE& p1, const VRP_PP_NODE& p2);
	friend double triangle(const VRP_PP_NODE&predecessor, const VRP_PP_NODE&insert, const VRP_PP_NODE&successor);
	friend double dist_lattlong(const VRP_PP_NODE&p1, const VRP_PP_NODE&p2);
	friend double polar_angle(const VRP_PP_NODE&node, const VRP_PP_NODE&depot);
};

double polar_angle(double x_long, double y_lat, double depot_x_long, double depot_y_lat);

//
// struct VRP_PP_ARC
//
// this structure is used to save node information in a VRP++-project

struct VRP_PP_ARC {
	int id;					// unique identifier for this arc
	int origin;				// id of origin node
	int dest;				// id of destination node
	double length_euclid;	// travel distances associated with this arc
	double length_latlong;	// distance measured by coordinates (longitude,latitude)
	double length_custom;
	double time;			// travel time associated with this arc
	std::string desc;		// verbal description of the arc

	VRP_PP_ARC(int _id = -1, const std::string& _desc = "not yet defined", int _origin = -1, int _dest = -1,
		double _length = VRP_PP_BIGM, double _time = VRP_PP_BIGM, double _length_latlong = VRP_PP_BIGM, double cWeigth = VRP_PP_BIGM) :
		id(_id), desc(_desc), origin(_origin), dest(_dest), length_euclid(_length), time(_time), length_latlong(_length_latlong), length_custom(cWeigth)
	{

	}

};
//
// struct VRP_PP_request
//
// this structure is used to save and origin-to-destination request in a VRP++-project

struct VRP_PP_REQUEST {
	int id;				// unique identifier of this request
	int id_node;			// refers to the node where the operation takes place
	int type;			// nicht benoetigt
	double duration;	// time needed for fulfillment of request onsite
	double quantity;	// demanded quantity
	double win_open;    // ready time
	double win_close;   // due time
	std::string desc;	// description of the request
	int group;          // we use this attribute to group several requests

	VRP_PP_REQUEST(int _group= -1, int _id = -1, int _node = -1, const std::string& _desc = "not yet defined", int _type = VRP_PP_DUMMY, double winOpen = 0, double winClose = 0, double _quantity = 0, double _duration = 0)
		: group(_group), id(_id), id_node(_node), desc(_desc), type(_type), win_open(winOpen), win_close(winClose), quantity(_quantity), duration(_duration)
	{

	}
};



//
// struct VRP_PP_VEHICLE
//
// this structure is used to represent a vehicle
// contained in a VRP++-project

struct VRP_PP_VEHICLE {
	int id;							// unique identifier of this vehicle
	int id_depot;					// identifier of depot
	int id_node;					// identifier of depot node
	double cap;						// max. payload capacity
	double maxDist;					// max. distance
	double speed;					// velocity of this vehicle
	std::string desc;               // verbal description of the vehicle
    double empty_mass;              // Version 3.0 - weight of unloaded vehicle
    double front_axle_load_empty;   // Version 3.0 - weight on front axle if vehicle is empty
    double front_axle_load_full;    // Version 3.0 - weight on front axle if vehicle is completely loaded
    double rear_axle_load_empty;    // Version 3.0 - weight on rear axle if vehicle is empty
    double rear_axle_load_full;     // Version 3.0 - weight on rear axle if vehicle is completely loaded
    double wheel_base;              // Version 3.0
    double dist_to_loadarea;        // Version 3.0
    double min_load_stearing;       // Version 3.0
    double min_load_traction;       // Version 3.0
    double load_bed_length;         // Version 3.0
    double cog_empty;               // Version 3.0 to be calculated

    // code Thuy adds - declare new vehicle attributes
    double fuel_a = 0.0;					// fuel consumption for empty truck
	double fuel_b = 0.0;					// fuel consumption for payload
	double evw = 0.0;					    // gross weight of empty vehicle


	VRP_PP_VEHICLE(int _id = -1, int id_depot = -1, int id_node = -1, double _cap = VRP_PP_BIGM, double maxDist = VRP_PP_BIGM, double _speed = 1, double _fuel_a = VRP_PP_BIGM, double _fuel_b = VRP_PP_BIGM, double _evw = VRP_PP_BIGM,const std::string& _desc = "not defined yet")
		:id(_id), id_depot(id_depot),id_node(id_node), cap(_cap),maxDist(maxDist), speed(_speed), fuel_a(_fuel_a), fuel_b(_fuel_b), evw(_evw),desc(_desc)

	{
        if( empty_mass > 0 )
            cog_empty = ( rear_axle_load_empty * wheel_base) / empty_mass;
        else
            cog_empty = 0.0;
    }

};





class VRP_PP_VARIANT
{
private:
	std::string str;
public:
	VRP_PP_VARIANT(const std::string& _str) :str(_str) {}
	VRP_PP_VARIANT(int value) :str(std::to_string(value)) {}
	VRP_PP_VARIANT(double value);
	inline int toInt()const { return *this; }
	inline double toDouble()const { return *this; }
	inline std::string toString()const { return *this; }
	inline bool toBool()const { return toDouble(); }
	operator int()const;
	operator double()const;
	inline operator std::string()const { return str; }
	inline operator bool()const { return toDouble(); }
};


bool equal(double a, double b);


template<typename T>
T* deep_copy_array(const T*source, int size)
{

	T*copy = new T[size];

	for (int i = 0; i < size; i++)
		copy[i] = source[i];

	return copy;
}


template<typename T>
T** deep_copy_2Darray(T const* const*source, int rows, int cols)
{

	T**copy = new T*[rows];

	for (int i = 0; i < rows; i++)
	{
		copy[i] = new T[cols];
		for(int j = 0; j < cols;j++)
			copy[i][j] = source[i][j];
	}


	return copy;
}

template<typename T>
void clear_linkedList(T*first, T*last)
{

	T*cur = first->next;
	while (cur != last)
	{
		cur->next->previous = first;
		first->next = cur->next;

		T*ptrToDelete = cur;
		cur = cur->next;

		delete ptrToDelete;
	}
}




template<class BidiIter >
BidiIter random_unique(BidiIter begin, BidiIter end, size_t num_random) {
	size_t left = std::distance(begin, end);
	while (num_random--) {
		BidiIter r = begin;
		std::advance(r, rand() % left);
		std::swap(*begin, *r);
		++begin;
		--left;
	}
	return begin;
}



std::string ltrim(std::string str);

std::string rtrim(std::string str);

std::string trim(std::string str);

template<typename T>
void split(const std::string& str, std::vector<T>& sink, const std::string& delims)
{

	std::size_t current, previous = 0;
	current = str.find_first_of(delims);
	while (current != std::string::npos)
	{
		sink.push_back(std::move(VRP_PP_VARIANT(trim(str.substr(previous, current - previous)))));
		previous = current + 1;
		current = str.find_first_of(delims, previous);
	}
	sink.push_back(std::move(VRP_PP_VARIANT(trim(str.substr(previous, current - previous)))));
}

struct VRP_PP_CONFIG {
    // this VRP_PP-data-structure is used to forward information about global configuration values to
    // different parts of the source-code. It can be modified and extended as required

    string FILE = "";       // used to store the XML-file name (incl. path is specified)
    int LP_VERBOSE = 0;     // display nothing during LP-solver execution
    VRP_PP_ARC_WEIGHT weigthType = VRP_PP_ARC_WEIGHT::CUSTOM;
                            // set the default arc weight to CUSTOM-specified

};



#endif
