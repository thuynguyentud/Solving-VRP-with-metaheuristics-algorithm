#include "vrp_pp_global.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>

// version 2.0 - last modification 25.02.2020 at 17:36

VRP_PP_NODE::VRP_PP_NODE(int _id, double _x, double _y, double _lattitude, double _longitude, const std::string & _desc)
	: id(_id), desc(_desc), x(_x), y(_y), longitude(_longitude), lattitude(_lattitude)
{
}

std::ostream & operator<<(std::ostream & stream, const VRP_PP_EVALUATION & eval)
{
	stream << "\nDistanz:\t\t\t" << eval.length
		<< "\nReisezeit:\t\t\t" << eval.travel_time
		<< "\nLadung:\t\t\t\t" << eval.cap_load
		<< "\nKapazitaetsverstoesse (value):\t" << eval.cap_error
		<< "\nKapazitaetsverstoesse (Anzahl):\t" << eval.cap_error_num
		<< "\nZeitfensterverstoesse (value):\t" << eval.win_error
		<< "\nZeitfensterverstoesse (Anzahl):\t" << eval.win_error_num
		<< "\nDistanzverstoesse (value):\t" << eval.length_error
		<< "\nDistanzverstoesse (Anzahl):\t" << eval.length_error_num << "\n";

	return stream;
}

bool equal(double a, double b)
{
	return std::fabs(a - b) < VRP_PP_EPSILON;
}

std::string ltrim(std::string str)
{
	return  str.erase(0, str.find_first_not_of(" \n\r\t"));
}

std::string rtrim(std::string str)
{
	return str.erase(str.find_last_not_of(" \n\r\t") + 1);
}

std::string trim(std::string str)
{
	return ltrim(rtrim(str));
}


VRP_PP_EVALUATION::VRP_PP_EVALUATION(double _obj_value, double _length, double _travel_time, double _cap_error, int _cap_error_num,
	double _win_error, int _win_error_num, double _cap_load, double _length_error, int _length_error_num, double _used_veh_cntr, double _veh_overload, double _plt_overload, int _feasible) //code Thuy adds
	: obj_value(_obj_value), length(_length), travel_time(_travel_time), cap_error(_cap_error), cap_error_num(_cap_error_num),
	win_error(_win_error), win_error_num(_win_error_num), cap_load(_cap_load), length_error(_length_error), length_error_num(_length_error_num), used_veh_cntr(_used_veh_cntr), veh_overload(_veh_overload), plt_overload(_plt_overload), feasible(_feasible) //code Thuy adds
{

}


VRP_PP_EVALUATION::VRP_PP_EVALUATION() : VRP_PP_EVALUATION(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) {  }

VRP_PP_EVALUATION& VRP_PP_EVALUATION::operator+=(const VRP_PP_EVALUATION& add)
{
	*this = std::move(*this + add);
	return *this;
}
VRP_PP_EVALUATION& VRP_PP_EVALUATION::operator-=(const VRP_PP_EVALUATION&sub)
{
	*this = std::move(*this - sub);
	return *this;
}
VRP_PP_EVALUATION VRP_PP_EVALUATION::operator+(const VRP_PP_EVALUATION& add)
{
	return std::move(VRP_PP_EVALUATION(obj_value + add.obj_value, length + add.length, travel_time + add.travel_time,
		cap_error + add.cap_error, cap_error_num + add.cap_error_num, win_error + add.win_error, win_error_num + add.win_error_num,
		cap_load + add.cap_load, length_error + add.length_error, length_error_num + add.length_error_num, used_veh_cntr + add.used_veh_cntr, veh_overload + add.veh_overload, plt_overload + add.plt_overload));//code Thuy adds
}
VRP_PP_EVALUATION VRP_PP_EVALUATION::operator-(const VRP_PP_EVALUATION& sub)
{
	return std::move(VRP_PP_EVALUATION(obj_value - sub.obj_value, length - sub.length, travel_time - sub.travel_time,
		cap_error - sub.cap_error, cap_error_num - sub.cap_error_num, win_error - sub.win_error, win_error_num - sub.win_error_num,
		cap_load - sub.cap_load, length_error - sub.length_error, length_error_num - sub.length_error_num, used_veh_cntr - sub.used_veh_cntr, veh_overload - sub.veh_overload, plt_overload - sub.plt_overload));//code Thuy adds
}


// returns the distance between two VRP_PP_NODE objects
double dist_euclid(const VRP_PP_NODE&p1, const VRP_PP_NODE&p2)
{
	return sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2));
}


double triangle(const VRP_PP_NODE&predecessor, const VRP_PP_NODE&insert, const VRP_PP_NODE&successor)
{
	return dist_euclid(predecessor, insert) + dist_euclid(insert, successor) - dist_euclid(predecessor, successor);
}


double dist_lattlong(const VRP_PP_NODE&p1, const VRP_PP_NODE&p2)
{
	auto toRadians = [](double DS)
	{
		return(DS*PI / 180.0);
	};

	// use the ‘haversine’ formula
	double phi1;
	double phi2;
	double delta_phi;
	double delta_lambda;

	phi1 = toRadians(p1.lattitude);
	phi2 = toRadians(p2.lattitude);
	delta_phi = toRadians(p2.lattitude - p1.lattitude);
	delta_lambda = toRadians(p2.longitude - p1.longitude);

	double a;
	a = sin(delta_phi / 2)*sin(delta_phi / 2) + cos(phi1)*cos(phi2)*sin(delta_lambda / 2)*sin(delta_lambda / 2);
	double c;
	c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

	return(6371.0*c);
}


double polar_angle(double x_long, double y_lat, double depot_x_long, double depot_y_lat)
{
	double ret_val;

	if (x_long > depot_x_long)
	{
		double quotient = (y_lat - depot_y_lat) / (x_long - depot_x_long);
		ret_val = std::atan(quotient*PI / 180);
		//ret_val = acos((y) / (sqrt(pow(y - 0, 2) + pow(x - 0, 2))));
	}
	else if (x_long < depot_x_long&&y_lat >= depot_y_lat)
	{
		double quotient = (y_lat - depot_y_lat) / (x_long - depot_x_long);
		ret_val = atan(quotient*PI / 180) + PI;
	}
	else if (x_long < depot_x_long&&y_lat < depot_y_lat)
	{
		double quotient = (y_lat - depot_y_lat) / (x_long - depot_x_long);
		ret_val = atan(quotient*PI / 180) - PI;
	}
	else if (equal(x_long, depot_x_long) && y_lat > depot_y_lat)
	{
		ret_val = PI / 2;
	}
	else
	{
		ret_val = -PI / 2;
	}

	return ret_val < 0 ? (ret_val + 2 * PI) : ret_val;
}


double polar_angle(const VRP_PP_NODE & node, const VRP_PP_NODE & depot)
{
	return polar_angle(node.longitude,node.lattitude,depot.longitude,depot.lattitude);
}

VRP_PP_VARIANT::VRP_PP_VARIANT(double value) /*:str(std::move(std::to_string(value)))*/
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(8) << value;
	str = ss.str();
}


VRP_PP_VARIANT::operator int()const
{

	double iVal = VRP_PP_BIGM;

    iVal = std::stoi(str);

	return iVal;
}

VRP_PP_VARIANT::operator double()const
{

	double dVal = VRP_PP_BIGM;

    dVal = std::stod(str);

	return dVal;
}

