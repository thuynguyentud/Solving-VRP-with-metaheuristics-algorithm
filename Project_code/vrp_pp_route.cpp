#include "vrp_pp_plan.h" // (includes routes and problems)
#include "vrp_pp_loading_list.h"
#include "vrp_pp_placed_item.h"

// wichtiger Hinweis: bei Routen-Manipulationen jeglicher Art, müssen die
// Informationen zu USED_ARCS und zu LOADING_LISTS manuell aktualisiert werden bzw. die Korrektur
// muss explizit durch einen passenden Befehl initiiert werden. Sollte in den beiliegnden Funktionen aber
// schon so sein

// version 2.1 - last modifications 14.04.2020 at 18:09

// version 2.2 - last modifications 24.04.2020 at 19:17
// wrong pointer initialization in void VRP_PP_ROUTE::insert_at_rand_pos(VRP_PP_OPERATION *insert)

// version 2.2 - last modifications 29.04.2020 at 14:20
// forget to set veh_num in constructor VRP_PP_ROUTE::VRP_PP_ROUTE(int _route_id, VRP_PP_NODE _start, VRP_PP_NODE _stop)

// ToDo Replace in next release

#include <iostream>
#include <algorithm> // min/max
#include <iomanip>		// setw



long long thread_local VRP_PP_ROUTE::route_ctr = 0;

VRP_PP_ROUTE::VRP_PP_ROUTE()
//: start(new VRP_PP_OPERATION(-1,-1,VRP_PP_START)), end(new VRP_PP_OPERATION(-1, -1, VRP_PP_STOP)), route_id(route_ctr++), vehicle_num(-1), type(VRP_PP_DUMMY),op_ctr(0)
: route_id(route_ctr++), vehicle_num(-1), type(VRP_PP_DUMMY),op_ctr(0)
{
    // this is the default constructor. It declares a route but does not insert any operations

	this->start = NULL;
	this->end = NULL;
}

VRP_PP_ROUTE::~VRP_PP_ROUTE()
{
	// this is the object's standard destructor
    // remove used arc structures
	this->remove_used_arcs();
    // remove loading lists
    this->empty_loading_lists();
    this->remove_loading_lists();
	// remove operations
	clear();

	delete this->end;
	delete this->start;
}

VRP_PP_ROUTE::VRP_PP_ROUTE(const VRP_PP_VEHICLE& vehicle)
	: type(VRP_PP_REAL), vehicle_num(vehicle.id), route_id(route_ctr++), op_ctr(0)
{
	// this procedure is the constructor for the object VRP_PP_ROUTE
	// here, we do a "by vehicle" initialization and add the current vehicle start position
	// as starting point of the route. In addition, we add the same position as the terminating
	// operation of the route

	// add the dummy operations
	//start = new VRP_PP_OPERATION(this->get_op_ctr(), vehicle.id_node, VRP_PP_START, "StartDepot");
	start = new VRP_PP_OPERATION(this->get_op_ctr(), vehicle.id_node, VRP_PP_START, "StartDepot");

	this->op_ctr++;
	end = new VRP_PP_OPERATION(this->get_op_ctr(), vehicle.id_node, VRP_PP_STOP, "EndDepot");
    this->op_ctr++;

	// concatenate the dummy operations
	start->next = end;
	end->previous = start;

	// set the current route identifier
	start->id_route = end->id_route = route_id;
}

VRP_PP_ROUTE::VRP_PP_ROUTE(int _route_id, VRP_PP_NODE _start, VRP_PP_NODE _stop)
{
    // this procedure is the constructor for the object VRP_PP_ROUTE
    // it creates the start and end operation only and
    // links the end operations to the given _start and _stop nodes
    //
    // this constructor is "by nodes" since the initial and the last dummy operations
    // are explicitly given.

    // set the operation counter
    this->op_ctr = 0;

    // specify the start operation
	start = new VRP_PP_OPERATION(_start.id, _start.id, VRP_PP_START, "PathStart");
    this->op_ctr++;

	// specify the stop operation
	end = new VRP_PP_OPERATION(_stop.id, _stop.id, VRP_PP_STOP, "PathEnd");
    this->op_ctr++;

	// concatenate the two dummy operations
	start->next = end;
	end->previous = start;

	// set the correct route identifier
	start->id_route = end->id_route = _route_id;

	// set the vehicle number added by jsb 29.04.2020 14:23
	this->vehicle_num = _route_id;
}

VRP_PP_ROUTE::VRP_PP_ROUTE(const VRP_PP_ROUTE & source)
	: start(new VRP_PP_OPERATION(*source.start)), end(new VRP_PP_OPERATION(*source.end)),
	type(source.type),vehicle_num(source.vehicle_num), /*route_id(source.route_id), */route_id(route_ctr++), eval_result(source.eval_result), op_ctr(0)
{
	// this is a copy-constructor that installs a copy of a given source route

    route_id = source.route_id;
	start->next = end;
	end->previous = start;
	start->id_route = end->id_route = route_id;


	const VRP_PP_OPERATION*lauf = source.start->next;
	while (lauf != source.end)
	{
		// insert new (copy) operation at route end
		VRP_PP_OPERATION*copy = new VRP_PP_OPERATION(*lauf);
		insert_between(copy, end->previous, end);

		lauf = lauf->next;
	}

	// copy the used arcs
	class VRP_PP_OPERATION *lauf_source, *lauf_dest;
	class VRP_PP_USED_ARC *NewUsedArc;

	lauf_dest = this->start;
	lauf_source = source.start;

	while( lauf_source->next != NULL )
    {
        if( lauf_source->out_arc != NULL )
        {
            NewUsedArc = new class VRP_PP_USED_ARC(lauf_source->out_arc->get_arc_id(),lauf_dest,lauf_dest->next);
            NewUsedArc = NULL;
            delete NewUsedArc;
        }
        lauf_dest = lauf_dest->next;
        lauf_source = lauf_source->next;
    }

    // copy the loading lists
    class VRP_PP_LOADING_LIST *NewLL;

	lauf_dest = this->start;
	lauf_source = source.start;

	while( lauf_source->next != NULL )
    {
        if( lauf_source->ob_ll != NULL )
        {
            NewLL = new class VRP_PP_LOADING_LIST();
            NewLL->copy_from(lauf_source->ob_ll);
            lauf_dest->link_to_outbound_loading_list(NewLL);
            lauf_dest->next->link_to_inbound_loading_list(NewLL);
            NewLL = NULL;
            //delete NewLL;
        }
        lauf_dest = lauf_dest->next;
        lauf_source = lauf_source->next;
    }

    this->op_ctr = source.op_ctr;
}

VRP_PP_ROUTE & VRP_PP_ROUTE::operator=(const VRP_PP_ROUTE & source)
{
    // this is a copy-constructor that installs a copy of a given source route
    // if this object is not empty, is it cleared at first.

	if (this == &source)
		return *this;

	clear();

	delete start;
	start = new VRP_PP_OPERATION(*source.start);
	start->id_route = route_id;

	delete end;
	end = new VRP_PP_OPERATION(*source.end);
	end->id_route = route_id;

	this->op_ctr = source.op_ctr;

	start->next = end;
	end->previous = start;

	const VRP_PP_OPERATION*lauf = source.start->next;
	while (lauf != source.end)
	{
		VRP_PP_OPERATION*copy = new VRP_PP_OPERATION(*lauf);
		insert_between(copy, end->previous, end);
		lauf = lauf->next;
	}

	eval_result = source.eval_result;
	//route_id = source.route_id;
	vehicle_num = source.vehicle_num;

    // copy the used arcs
	class VRP_PP_OPERATION *lauf_source, *lauf_dest;
	class VRP_PP_USED_ARC *NewUsedArc;

	lauf_dest = this->start;
	lauf_source = source.start;

	while( lauf_source->next != NULL )
    {
        if( lauf_source->out_arc != NULL )
        {
            NewUsedArc = new class VRP_PP_USED_ARC(lauf_source->out_arc->get_arc_id(),lauf_dest,lauf_dest->next);
            NewUsedArc = NULL;
            delete NewUsedArc;
        }
        lauf_dest = lauf_dest->next;
        lauf_source = lauf_source->next;
    }

    // copy the loading lists
    class VRP_PP_LOADING_LIST *NewLL;

	lauf_dest = this->start;
	lauf_source = source.start;

	while( lauf_source->next != NULL )
    {
        if( lauf_source->ob_ll != NULL )
        {
            NewLL = new class VRP_PP_LOADING_LIST();
            NewLL->copy_from(lauf_source->ob_ll);
            lauf_dest->link_to_outbound_loading_list(NewLL);
            lauf_dest->next->link_to_inbound_loading_list(NewLL);
            NewLL = NULL;
            delete NewLL;
        }
        lauf_dest = lauf_dest->next;
        lauf_source = lauf_source->next;
    }

    this->op_ctr = source.op_ctr;

    cout << "sjfhsdfsdhfh " << endl;
	return *this;
}

void VRP_PP_ROUTE::init(const VRP_PP_VEHICLE *vehicle)
{
	// this procedure initializes an existing object VRP_PP_ROUTE
    // no operations are added after a careful cleaning
	// delete all operations

	clear();

	type = VRP_PP_REAL;
	vehicle_num = vehicle->id;

	start->id = end->id = vehicle->id_depot;
	start->id_node = end->id_node = vehicle->id_node;

}


void VRP_PP_ROUTE::clear(void)
{
    // remove the used arcs
    this->remove_used_arcs();

    // remove the loading lists
    this->remove_loading_lists();

    // remove the operation chain
	//clear_linkedList(start, end);
    class VRP_PP_OPERATION *cur;
    cur = this->start->next;
	while (cur != this->end)
	{
		cur->next->previous = this->start;
		start->next = cur->next;

		class VRP_PP_OPERATION *ptrToDelete;
		ptrToDelete = cur;
		cur = cur->next;

		delete ptrToDelete;
	}

	eval_result = VRP_PP_EVALUATION();
	op_ctr = 0;
}


void VRP_PP_ROUTE::print_as_operation_table(const VRP_PP_PROBLEM *prob)const
{
	//VRP_PP_TRACE trace(__FUNCTION__);
	//assert_pointer(prob);

	using namespace std;

	VRP_PP_OPERATION *lauf;

	lauf = this->start;

	if (this->type == VRP_PP_DUMMY)
		cout << "DUMMY-route" << endl;
	else
		cout << "REAL-route" << endl;

	std::cout << eval_result;

	while (lauf != NULL)
	{
		const VRP_PP_REQUEST* request;
		if (lauf->type == VRP_PP_START)
		{
			request = &prob->get_depot_by_index(lauf->id);
			cout << "START";
		}
		else if (lauf->type == VRP_PP_STOP)
		{
			request = &prob->get_depot_by_index(lauf->id);
			cout << "STOP";
		}
		else
		{
			request = &prob->get_request_by_index(lauf->id);
			if (lauf->type == VRP_PP_DELIVERY) cout << "D(" << lauf->id << ")";
			else cout << "P(" << lauf->id << ")";
		}



		cout << "(" << prob->get_node_by_index(request->id_node).x << ";" << prob->get_node_by_index(request->id_node).y << ")\t";
		cout << "GEO(" <<setw(7)<< prob->get_node_by_index(request->id_node).lattitude << ";" << setw(7) << prob->get_node_by_index(request->id_node).longitude << ")\t";
		cout << "RT=" << setw(8) << lauf->reachtime << " ";
		cout << "ST=" << setw(8) << lauf->starttime << " ";
		cout << "FT=" << setw(8) << lauf->finishtime << " ";

		cout << "TW=[" << setw(4) << request->win_open << ";" << setw(4) << request->win_close << "] ";

		cout << "D=" << request->duration;
		cout << endl;

		lauf = lauf->next;
	}
}


void VRP_PP_ROUTE::insert_between(VRP_PP_OPERATION *insert, VRP_PP_OPERATION *predecessor, VRP_PP_OPERATION *successor)
{
	//VRP_PP_TRACE trace(__FUNCTION__);
	// assert_pointer(insert), assert_pointer(predecessor), assert_pointer(successor);


	if (1<2) //(predecessor->id_route==route_id && successor->id_route == route_id)
	{
		//printf("--> start insert between\n");

		class VRP_PP_OPERATION *VEC1, *VEC2;

		VEC1 = predecessor;
		VEC2 = successor;
		//printf("--> start insert between - A: %d\n",VEC2->reqnum);
		insert->next = VEC2;
		//printf("--> start insert between - B\n");
		VEC2->previous = insert;
		//printf("--> start insert between - C\n");
		insert->previous = VEC1;
		//printf("--> start insert between - D\n");
		VEC1->next = insert;
		//printf("--> start insert between\n");

		insert->id_route = route_id;
		op_ctr++;
	}
	else
	{
		//throw VRP_PP_EXCEPTION("Invalid p1 or p2 operation input", trace);
	}
}


void VRP_PP_ROUTE::dissolve_operation(class VRP_PP_OPERATION *dissolve)
{

	dissolve->previous->next = dissolve->next;
	dissolve->next->previous = dissolve->previous;
	dissolve->next = NULL;
	dissolve->previous = NULL;
	dissolve->id_route = -1;

	op_ctr--;
}

void VRP_PP_ROUTE::evaluate(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType)
{
	VRP_PP_OPERATION *Ptr1, *Lfd, *Ptr2;

	eval_result = VRP_PP_EVALUATION();

	//code Thuy adds - Create new Ptr and add the variable for arc loading quantity in a route
    Ptr2 = this->start;
    Ptr2 = Ptr2->next;

    double cur_load = 0.0;
    while(Ptr2 != this->end)
    {
        cur_load += prob->get_request_by_index(Ptr2->id).quantity;
        Ptr2 = Ptr2->next;
    };

    //std::cout << "\n Total loading quantity to start: " << cur_load << endl;

	// goto operation
	Ptr1 = this->start;

	// determine variable times
	Ptr1->reachtime = 0.0;

	// check the type of the first operation in this route
	if( (Ptr1->type == VRP_PP_START) || (Ptr1->type == VRP_PP_STOP) )
    {
        // we have to fetch all attribute values from the list of depots
        Ptr1->starttime = std::max(prob->get_depot_by_index(Ptr1->id).win_open, Ptr1->reachtime);
        Ptr1->finishtime = Ptr1->starttime + prob->get_depot_by_index(Ptr1->id).duration;
    }
    else
    {
        // we have to fetch all attribute values from the list of requests
        if( Ptr1->type == VRP_PP_TRANSIT )
        {
            // we have a transit node
            Ptr1->starttime = Ptr1->reachtime;
            Ptr1->finishtime = Ptr1->starttime;
        }
        else
        {
            // we have a pickup or a delivery operation: retrieve information from assoc. request
            Ptr1->starttime = std::max(prob->get_request_by_index(Ptr1->id).win_open, Ptr1->reachtime);
            Ptr1->finishtime = Ptr1->starttime + prob->get_request_by_index(Ptr1->id).duration;
        }

    }

    // now iterate over the sequence of routes
    do
	{
		double curDist = VRP_PP_BIGM, curTravelTime = VRP_PP_BIGM;
		switch (weigthType)
		{
		case VRP_PP_ARC_WEIGHT::EUCLID:
			curDist = prob->get_dist_euclid(Ptr1->id_node, Ptr1->next->id_node);
			//cout << "EUCLID Route " << this->route_id << ": " << curDist << endl;
			break;
		case VRP_PP_ARC_WEIGHT::GEO:
			curDist = prob->get_dist_latlong(Ptr1->id_node, Ptr1->next->id_node);
			//cout << "GEO Route " << this->route_id << ": " << curDist << endl;
			break;
		case VRP_PP_ARC_WEIGHT::CUSTOM:
		    {
		        curDist = prob->get_dist_custom(Ptr1->id_node, Ptr1->next->id_node);
		        //cout << "Custom Route " << this->route_id << ": " << curDist << endl;
			break;
		    }

		}

        //code Thuy adds - Formulation of objective function: Total fuel consumption for each arc in route
		eval_result.obj_value += (curDist / 100)*(prob->get_vehicle_by_index(this->vehicle_num).fuel_a + prob->get_vehicle_by_index(this->vehicle_num).fuel_b*cur_load)  ;

        //print values of each variable in each arc of the route to check correct logic
        //cur_load = 0 after final delivery operation
        cout << "\n Loading quantity for each arc:  " << cur_load << endl;

		eval_result.length += curDist;
		eval_result.travel_time += curTravelTime = curDist / std::max(VRP_PP_EPSILON, prob->get_vehicle_by_index(vehicle_num).speed);

		if (eval_result.cap_load > prob->get_vehicle_by_index(vehicle_num).cap)
		{
			eval_result.cap_error_num++;
		}

		// goto operation
		Ptr1 = Ptr1->next;

		// determine variable times
		Ptr1->reachtime = Ptr1->previous->finishtime + curTravelTime;

		if ( (Ptr1->type == VRP_PP_START) || (Ptr1->type == VRP_PP_STOP) )
		{
		    // we have to fetch all operation data from the depot list
			Ptr1->finishtime = Ptr1->starttime = Ptr1->reachtime;

			// check time window constraint
			if (Ptr1->starttime > prob->get_depot_by_index(Ptr1->id).win_close)
			{
				// violation detected
				this->eval_result.win_error_num++;
				this->eval_result.win_error += (Ptr1->starttime - prob->get_depot_by_index(Ptr1->id).win_close);
			}
		}
		else
		{
		    if( Ptr1->type == VRP_PP_TRANSIT )
            {
                // we have a transit node: nothing serious happens here
                Ptr1->starttime = Ptr1->reachtime;
                Ptr1->finishtime = Ptr1->starttime;
            }
            else
            {
                // we have to fetch all operation data from the request list
                Ptr1->starttime = std::max(prob->get_request_by_index(Ptr1->id).win_open, Ptr1->reachtime);
                Ptr1->finishtime = Ptr1->starttime + prob->get_request_by_index(Ptr1->id).duration;
                eval_result.cap_load += prob->get_request_by_index(Ptr1->id).quantity;

                //code Thuy adds - decreasing in current arc loading quantity after each iteration
                cur_load -= prob->get_request_by_index(Ptr1->id).quantity;

                // check time window constraint
                if (Ptr1->starttime > prob->get_request_by_index(Ptr1->id).win_close)
                {
                    // violation detected
                    this->eval_result.win_error_num++;
                    this->eval_result.win_error += (Ptr1->starttime - prob->get_request_by_index(Ptr1->id).win_close);
                }
            }
		}
	} while (Ptr1 != this->end);




    //std::cout << "vehicle_overloaded \n " << vehicle_overloaded;
    //std::cout << "eval_result.cap_load - prob->get_vehicle_by_index(vehicle_num).cap) \n " << eval_result.cap_load - prob->get_vehicle_by_index(vehicle_num).cap;
	// ensure that the vehicle id is stored in all operations
	Lfd = this->start;
	while(Lfd != NULL)
    {
        Lfd->id_route = this->vehicle_num;
        Lfd = Lfd->next;
    }

    //code Thuy adds - add 3 performance indicators
    //calculate total number of deployed vehicle
    if (eval_result.cap_load>0) {eval_result.used_veh_cntr = 1;} else {eval_result.used_veh_cntr = 0;}

    //calculate the number of overloaded vehicles
    if (eval_result.cap_error_num>0) {eval_result.veh_overload = 1;} else {eval_result.veh_overload = 0;}

    //calculate the number of overloaded pallets
    if (eval_result.veh_overload == 1) {eval_result.plt_overload = (eval_result.cap_load - prob->get_vehicle_by_index(vehicle_num).cap)*2;} else {eval_result.plt_overload = 0;}


    //add penalty values to obj_value
    eval_result.obj_value += prob->P1*eval_result.veh_overload + prob->P2*eval_result.plt_overload;

    //cout << "\n Objective values:  " << eval_result.obj_value << endl;
}

VRP_PP_OPERATION *VRP_PP_ROUTE::get_ptr_to_arb_op(void)
{
	VRP_PP_OPERATION *lauf;
	int counter, i, k;

	if (this->start->next != this->end)
	{
		counter = 0;
		lauf = this->start->next;
		while (lauf->next != NULL)
		{
			counter++;
			lauf = lauf->next;
		}

		i = rand() % (counter);
		lauf = this->start->next;

		for (k = 0; k < i; k++)
			lauf = lauf->next;

		return(lauf);
	}
	else
	{
		return(NULL);
	}
}

VRP_PP_OPERATION * VRP_PP_ROUTE::get_ptr_to_op(int index)
{
	int lauf_index = -1;
	VRP_PP_OPERATION*lauf = NULL;
	if (index > op_ctr / 2)
	{
		lauf = end->previous;
		lauf_index = op_ctr - 1;
		while (lauf_index > index)
		{
			lauf = lauf->previous;
			lauf_index--;
		}
	}
	else
	{
		lauf = start->next;
		lauf_index = 0;
		while (lauf_index < index)
		{
			lauf = lauf->next;
			lauf_index++;
		}
	}

	return lauf;
}

const VRP_PP_OPERATION * VRP_PP_ROUTE::get_ptr_to_op(int index) const
{

	int lauf_index = -1;
	VRP_PP_OPERATION*lauf = NULL;
	if (index > op_ctr / 2)
	{
		lauf = end->previous;
		lauf_index = op_ctr - 1;
		while (lauf_index > index)
		{
			lauf = lauf->previous;
			lauf_index--;
		}
	}
	else
	{
		lauf = start->next;
		lauf_index = 0;
		while (lauf_index < index)
		{
			lauf = lauf->next;
			lauf_index++;
		}
	}

	return lauf;
}

VRP_PP_OPERATION * VRP_PP_ROUTE::get_start_depot()
{
	return start;
}

const VRP_PP_OPERATION * VRP_PP_ROUTE::get_start_depot() const
{
	return start;
}

const VRP_PP_OPERATION * VRP_PP_ROUTE::get_end_depot() const
{
	return end;
}

VRP_PP_OPERATION * VRP_PP_ROUTE::get_end_depot()
{
	return end;
}


void VRP_PP_ROUTE::insert_at_rand_pos(VRP_PP_OPERATION *insert)
{

	int i, ind;
	VRP_PP_OPERATION *lauf;

	cout << "A+" << insert->desc << endl;
	if (op_ctr > 0)
	{
		ind = rand() % op_ctr;
		cout << "ind=" << ind << endl;
		cout << "op_cntr=" << op_ctr << endl;
		// this is the old statement which is wrong
		//lauf = this->start->next;
		// this is the corrected statement
		lauf = this->start;
		for (i = 0; i < ind; i++)
			lauf = lauf->next;

       this->insert_between(insert, lauf->previous, lauf);
	}
	else
	{
		this->insert_between(insert, end->previous, end);
	}
	cout << "A-" << endl;
}

void VRP_PP_ROUTE::insert_at_rand_pos_after(VRP_PP_OPERATION *refPosition, VRP_PP_OPERATION *insert)
{

	int cntr, i, ind;
	class VRP_PP_OPERATION *lauf;

	// determine index of refPosition
	lauf = refPosition;
	cntr = 0;
	while (lauf != this->end)
	{
		cntr++;
		lauf = lauf->next;
	}

	if (cntr > 0)
	{
		ind = rand() % cntr;
		lauf = refPosition;
		for (i = 0; i < ind - 1; i++)
			lauf = lauf->next;

		this->insert_between(insert, lauf, lauf->next);
	}
	else
	{
		this->insert_between(insert, end->previous, end);
	}
}

void VRP_PP_ROUTE::insert_at_rand_pos_before(class VRP_PP_OPERATION *refPosition, class VRP_PP_OPERATION *insert)
{

	int cntr, i, ind;
	VRP_PP_OPERATION *lauf;

	// determine index of refPosition
	lauf = refPosition;
	cntr = 0;
	while (lauf != this->start)
	{
		cntr++;
		lauf = lauf->previous;
	}

	// insertion
	if (cntr > 0)
	{
		// draw random position
		ind = rand() % cntr;
		lauf = refPosition;
		for (i = 0; i < ind - 1; i++)
			lauf = lauf->previous;

		this->insert_between(insert, lauf->previous, lauf);
	}
	else
	{
		// empty route - there is only one possibility
		this->insert_between(insert, this->start, this->start->next);
	}
}






bool VRP_PP_ROUTE::is_predec(const VRP_PP_OPERATION *a, const VRP_PP_OPERATION *b)const
{

	if (a->id_route != b->id_route) return false;

	const VRP_PP_OPERATION *curPtr;

	curPtr = a;
	while (curPtr != NULL)
	{
		if (curPtr == b)
		{
			return true;
		}
		curPtr = curPtr->next;
	}

	return false;
}


void VRP_PP_ROUTE::print_as_node_sequence(const VRP_PP_PROBLEM *prob)const
{

	using namespace std;
	VRP_PP_OPERATION *lauf;

	lauf = this->start;
	while (lauf != NULL)
	{
		cout << (lauf->type == VRP_PP_START ? "" : "-") << "(" << lauf->id << "|" << lauf->id_node << ")";
		lauf = lauf->next;
	}
	cout << endl;
}

void VRP_PP_ROUTE::print_as_alternating_sequence(const VRP_PP_PROBLEM *prob)const
{
    // this procedure prints the list of operations and connecting arcs (if specified)

    using namespace std;
	VRP_PP_OPERATION *lauf;

    lauf = this->start;
	while (lauf != NULL)
	{
		cout << "operation " << (lauf->type == VRP_PP_START ? "START" : "CUSTOMER") << "(" << lauf->id_node << ")" << endl;
		if( lauf->out_arc != NULL )
            cout << "arc " << lauf->out_arc->get_arc_id() << " used" << endl;
        else
            cout << "no used arc specified" << endl;

		lauf = lauf->next;
	}
	cout << endl;

}
long long VRP_PP_ROUTE::get_route_id() const
{
	return route_id;
}

int VRP_PP_ROUTE::get_op_ctr() const
{
	return op_ctr;
}

int VRP_PP_ROUTE::get_vehicle_num() const
{
	return vehicle_num;
}

const VRP_PP_EVALUATION & VRP_PP_ROUTE::get_evaluation() const
{
	return eval_result;
}

void VRP_PP_ROUTE::invert_from_to(class VRP_PP_OPERATION *from, class VRP_PP_OPERATION *to)
{

	VRP_PP_OPERATION *ToBeMoved;
	if( from->next != to )
    {
        while (from != to)
        {
            to = to->previous;
            // set pointer to the next moved operation
            ToBeMoved = to->next;
            // releases operation to be moved from route
            this->dissolve_operation(ToBeMoved);
            // re-inserts operation to be moved at the binning of the route
            // part under consideration
            this->insert_between(ToBeMoved, from->previous, from);
            ToBeMoved = NULL;
        }
    }
    else
    {
        //cout << "GAGA" << endl;
        this->dissolve_operation(to);
        this->insert_between(to, from->previous, from);
    }
}

void VRP_PP_ROUTE::append_route(VRP_PP_ROUTE *append)
{
	// this procedure concatenates the routes this as well as R_APP
	// R_APP is inserted between this->end->previous and this->end
	VRP_PP_OPERATION*lauf = append->start->next;
	while (lauf != append->end)
	{
		VRP_PP_OPERATION*tmp = lauf->next;
		append->dissolve_operation(lauf);
		insert_between(lauf, end->previous, end);
		lauf = tmp;
	}
}

void VRP_PP_ROUTE::two_opt_route_full(struct VRP_PP_PROBLEM *P, const VRP_PP_ARC_WEIGHT& weigthType)
{
    // added from version 1.03
    // 30.05.2018
    // modified 16.06.2019: function does not have a return object anymore

	int i, j;

	// determine number of non-dummy operations
	int OpNum = 0;
	class VRP_PP_OPERATION *TMP, *TMP_A;
	TMP = this->start->next;
	while (TMP != this->end)
	{
		OpNum++;
		TMP = TMP->next;
	}

    //cout << "OpNum=" << OpNum << endl;

	if (OpNum >= 2)
	{
		// determine number of possible swaps
		int SwapNum;
		SwapNum = (OpNum)*(OpNum-1)/2;

		struct _LIST_ELEM {
			double saving;
			class VRP_PP_OPERATION *SubRouteStart;
			class VRP_PP_OPERATION *SubRouteEnd;
		};

		struct _LIST_ELEM *CANDIDATE, TmpCand;
		CANDIDATE = new struct _LIST_ELEM[SwapNum];
		for (i = 0; i < SwapNum; i++)
		{
			CANDIDATE[i].saving = 1.0;
			CANDIDATE[i].SubRouteStart = NULL;
			CANDIDATE[i].SubRouteEnd = NULL;
		}

		// Iteration
		while (CANDIDATE[0].saving > 0)
		{
			// first step: fill the candidate list
			TMP = this->start->next;
			i = 0;

			while (TMP != this->end->previous)
			{
				TMP_A = TMP->next;
				while (TMP_A != this->end)
				{
					CANDIDATE[i].SubRouteStart = TMP;
					CANDIDATE[i].SubRouteEnd = TMP_A;
					CANDIDATE[i].saving = 1.0;
					i++;
					TMP_A = TMP_A->next;
				}
				TMP = TMP->next;
			}

			/*
			for( int i=0 ; i < SwapNum ; i++ )
            {
                cout << "i="
                    << i
                    << ": "
                    << CANDIDATE[i].SubRouteStart->id_node
                    << "->"
                    << CANDIDATE[i].SubRouteEnd->id_node
                    << endl;
            }
			cout << "i=" << i << endl;
			*/
			// second step: determine saving values
			VRP_PP_EVALUATION NewPlanEval;
			for (i = 0; i < SwapNum; i++)
			{
			    //cout << "i=" << i << " start " << endl;
				this->evaluate(P, weigthType);
                NewPlanEval = this->get_evaluation();
				CANDIDATE[i].saving = NewPlanEval.length;
				//cout << "HUH=" << NewPlanEval.length << endl;
				//this->print_as_node_sequence(P);
				//cout << CANDIDATE[i].SubRouteStart->id << endl;
				//cout << CANDIDATE[i].SubRouteEnd->id << endl;
				this->invert_from_to(CANDIDATE[i].SubRouteStart, CANDIDATE[i].SubRouteEnd);
				//this->print_as_node_sequence(P);
				//this->print_as_node_sequence(P);
				this->evaluate(P, weigthType);
				//cout << "C" << endl;
				NewPlanEval = this->get_evaluation();
				CANDIDATE[i].saving = CANDIDATE[i].saving - NewPlanEval.length;
				//cout << "D" << endl;
				this->invert_from_to(CANDIDATE[i].SubRouteEnd, CANDIDATE[i].SubRouteStart);
				//this->print_as_node_sequence(P);
				//cout << "B" << endl;
				this->evaluate(P, weigthType);
				//cout << "i=" << i << " stop " << endl;
            }


			// third step: sort CANDIDATES by decreasing saving-values
			for (i = 0; i < SwapNum - 1; i ++ )
				for (j = 0; j < SwapNum - 1; j++ )
					if (CANDIDATE[j].saving < CANDIDATE[j + 1].saving)
					{
						TmpCand = CANDIDATE[j];
						CANDIDATE[j] = CANDIDATE[j + 1];
						CANDIDATE[j + 1] = TmpCand;
					}

			// fourth step: try to implement swap
					if (CANDIDATE[0].saving > 0)
					{
						this->invert_from_to(CANDIDATE[0].SubRouteStart, CANDIDATE[0].SubRouteEnd);
						//cout << "MODIFICATION!" << endl;
					}
		}

		// release allocated memory
		delete[] CANDIDATE;
	}

}

void VRP_PP_ROUTE::establish_loading_lists(struct VRP_PP_PROBLEM *P)
{
    int cntr=0;

    class VRP_PP_OPERATION *lauf;
    lauf = this->start;
    while( lauf->next != NULL )
    {
        // create a new empty loading list
        class VRP_PP_LOADING_LIST *NewList;
        NewList = new class VRP_PP_LOADING_LIST;
        NewList->items = 0;
        // establish only an outbound loading list
        lauf->link_to_outbound_loading_list(NewList);
        // and use it as inbound loading list for op->next
        lauf->next->link_to_inbound_loading_list(NewList);

        NewList = NULL;
        lauf=lauf->next;
        cntr++;
        delete NewList;
    }
}

void VRP_PP_ROUTE::fill_loading_lists_VRPtype(struct VRP_PP_PROBLEM *prob)
{
    // added by jsb, 09.01.2020
    //
    //
    // this procedure first empties all loading lists in this route.
    // afterwards, it fills them according to VRP-logic (all packages loaded in start operation)

    // first empty all existing loading lists
    this->empty_loading_lists();

    // second assign moved items to loading lists along route
    class VRP_PP_OPERATION *lauf;

    class VRP_PP_LOAD_ITEM LoadItem;
    class VRP_PP_OPERATION *lauf_req;
    lauf = this->start->next;
    while( lauf->next != NULL )
    {
        lauf_req = this->start;
        while( lauf_req != lauf )
        {
            // fetch the load item that belongs to request lauf_req
            LoadItem = prob->get_load_item_by_index(lauf->id);
            struct VRP_PP_PLACED_ITEM NewItem(LoadItem.get_id(), -1.0, -1.0, -1.0, -1.0, -1.0, -1.0,-1);
            lauf_req->ob_ll->append_item(NewItem);
            lauf_req = lauf_req->next;
        }
        lauf=lauf->next;
    }
}

void VRP_PP_ROUTE::empty_loading_lists(void)
{
    // added by JSB, 09.01.2020

    class VRP_PP_OPERATION *lauf;
    lauf = this->start;
    while( lauf != NULL )
    {
        if( lauf->ob_ll != NULL )
        {
            // a loading list exists
            lauf->ob_ll->remove_all_items();
        }
        if( lauf->ib_ll != NULL )
        {
            // a loading list exists
            lauf->ib_ll->remove_all_items();
        }
        lauf = lauf->next;
    }
}

void VRP_PP_ROUTE::remove_loading_lists(void)
{
    // last modified 04.06.2020 at 20:25
    //remove structures of loading lists
    // to be sure it is tried to remove each inbound as well as each outbound loading list

    class VRP_PP_OPERATION *lauf;
    lauf = this->start;
    while( lauf != NULL )
    {
        if( lauf->ob_ll != NULL )
        {
             // a loading list exists
           lauf->ob_ll->remove_all_items();
           if( lauf->ob_ll->out_op != NULL)
           {
               lauf->ob_ll->out_op = NULL;
           }
           if( lauf->ob_ll->in_op != NULL)
           {
               lauf->ob_ll->in_op->ib_ll = NULL;
               lauf->ob_ll->in_op = NULL; //
           }
           lauf->ob_ll->~VRP_PP_LOADING_LIST();
           delete lauf->ob_ll;
           lauf->ob_ll = NULL;
        }
        if( lauf->ib_ll != NULL )
        {
            // a loading list exists

            lauf->ib_ll->remove_all_items();
            if( lauf->ib_ll->in_op != NULL)
            {
               lauf->ib_ll->in_op = NULL;
            }
            if( lauf->ib_ll->out_op != NULL)
            {
               lauf->ib_ll->out_op->ob_ll = NULL;
               lauf->ib_ll->out_op = NULL; //
            }
            lauf->ib_ll->~VRP_PP_LOADING_LIST();
            delete lauf->ib_ll;
            lauf->ib_ll = NULL;
        }
        lauf = lauf->next;
    }
}

void VRP_PP_ROUTE::remove_used_arcs(void)
{
    // last modified 04.06.2020 at 19:30
    // CHANGE if( lauf->ib_ll != NULL ) TO if( lauf->in_arc != NULL )

    //remove structures of used arcs
    // to be sure it is tried to remove each inbound as well as each outbound used arc

    class VRP_PP_OPERATION *lauf;
    class VRP_PP_USED_ARC *TmpArc;
    lauf = this->start;
    while( lauf != NULL )
    {
        if( lauf->out_arc != NULL )
        {
            // an outbound arc structure exists
           TmpArc = lauf->out_arc;
           TmpArc->dissolve_from_ops();
           delete TmpArc;
        }
        if( lauf->in_arc != NULL )
        {
            // an inbound arc structure exists
            TmpArc = lauf->in_arc;
            TmpArc->dissolve_from_ops();
            delete TmpArc;
        }
        lauf = lauf->next;
    }
}

void VRP_PP_ROUTE::assign_used_arcs_unique(struct VRP_PP_PROBLEM *prob)
{
     // fills the used arc structures with the first found arc
     //
     // this procedure is looking for the FIRST arc in the arc list of *prob
     // matching tail and head of the two associated consecutive operations
     // and inserts this arc as used arc between tail and head

     class VRP_PP_OPERATION *lauf;
     struct VRP_PP_ARC PtrArc;
     lauf = this->start;
     while( lauf->next != NULL)
     {
         if( (lauf->id_node >= 0) && (lauf->id_node < prob->nodes) && (lauf->next->id_node >=0 ) && (lauf->next->id_node < prob->nodes) )
         {
            PtrArc = prob->get_arc_by_od(lauf->id_node,lauf->next->id_node);
            class VRP_PP_USED_ARC *UsedArc;
            UsedArc = new class VRP_PP_USED_ARC(PtrArc.id, lauf, lauf->next);
            UsedArc = NULL;
            delete UsedArc;
         }
         lauf = lauf->next;
     }
}

// chapter 4
void VRP_PP_ROUTE::CVRP_evaluate(const VRP_PP_PROBLEM *prob, const VRP_PP_ARC_WEIGHT& weigthType)
{
    // added by JSB - 10.05.2020 - 08:52

	VRP_PP_OPERATION *Ptr1;

	eval_result = VRP_PP_EVALUATION();

	// goto start operation
	Ptr1 = this->start;

	// determine variable times
	Ptr1->reachtime = 0.0;
	Ptr1->starttime = std::max(prob->get_depot_by_index(Ptr1->id).win_open, Ptr1->reachtime);
	Ptr1->finishtime = Ptr1->starttime + prob->get_depot_by_index(Ptr1->id).duration;


	do
	{
		double curDist = VRP_PP_BIGM, curTravelTime = VRP_PP_BIGM;
		switch (weigthType)
		{
		case VRP_PP_ARC_WEIGHT::EUCLID:
			curDist = prob->get_dist_euclid(Ptr1->id_node, Ptr1->next->id_node); break;
		case VRP_PP_ARC_WEIGHT::GEO:
			curDist = prob->get_dist_latlong(Ptr1->id_node, Ptr1->next->id_node); break;
		default:
			curDist = prob->get_dist_custom(Ptr1->id_node, Ptr1->next->id_node);
		}

		eval_result.length += curDist;
		eval_result.travel_time += curTravelTime = curDist / std::max(VRP_PP_EPSILON, prob->get_vehicle_by_index(vehicle_num).speed) * 60;

		if (eval_result.cap_load > prob->get_vehicle_by_index(vehicle_num).cap)
		{
			eval_result.cap_error_num++;
		}

		// goto operation
		Ptr1 = Ptr1->next;

		// determine variable times
		Ptr1->reachtime = Ptr1->previous->finishtime + curTravelTime;

		if (Ptr1 == end)
		{
			Ptr1->finishtime = Ptr1->starttime = Ptr1->reachtime;

			// check time window constraint
			if (Ptr1->starttime > prob->get_depot_by_index(Ptr1->id).win_close)
			{
				// violation detected
				this->eval_result.win_error_num++;
				this->eval_result.win_error += (Ptr1->starttime - prob->get_depot_by_index(Ptr1->id).win_close);
			}
		}
		else
		{
			Ptr1->starttime = std::max(prob->get_request_by_index(Ptr1->id).win_open, Ptr1->reachtime);
			Ptr1->finishtime = Ptr1->starttime + prob->get_request_by_index(Ptr1->id).duration;
			eval_result.cap_load += prob->get_request_by_index(Ptr1->id).quantity;

			// check time window constraint
			if (Ptr1->starttime > prob->get_request_by_index(Ptr1->id).win_close)
			{
				// violation detected
				this->eval_result.win_error_num++;
				this->eval_result.win_error += (Ptr1->starttime - prob->get_request_by_index(Ptr1->id).win_close);
			}
		}
	} while (Ptr1 != this->end);
}

void VRP_PP_ROUTE::CVRP_two_opt_route_full(struct VRP_PP_PROBLEM *P, const VRP_PP_ARC_WEIGHT& weigthType)
{
    // added from version 1.03
    // 30.05.2018
    // modified 16.06.2019: function does not have a return object anymore
    // modified 10.05.2020: function assigne dto CVRP scenarios
	int i, j;

	// determine number of non-dummy operations
	int OpNum = 0;
	class VRP_PP_OPERATION *TMP, *TMP_A;
	TMP = this->start->next;
	while (TMP != this->end)
	{
		OpNum++;
		TMP = TMP->next;
	}

	if (OpNum >= 2)
	{
		// determine number of possible swaps
		int SwapNum;
		SwapNum = (OpNum)*(OpNum-1)/2;

		struct _LIST_ELEM {
			double saving;
			class VRP_PP_OPERATION *SubRouteStart;
			class VRP_PP_OPERATION *SubRouteEnd;
		};

		struct _LIST_ELEM *CANDIDATE, TmpCand;
		CANDIDATE = new struct _LIST_ELEM[SwapNum];
		for (i = 0; i < SwapNum; i++)
		{
			CANDIDATE[i].saving = 1.0;
			CANDIDATE[i].SubRouteStart = NULL;
			CANDIDATE[i].SubRouteEnd = NULL;
		}

		// Iteration
		while (CANDIDATE[0].saving > 0)
		{
			// first step: fill the candidate list
			TMP = this->start->next;
			i = 0;

			while (TMP != this->end->previous)
			{
				TMP_A = TMP->next;
				while (TMP_A != this->end)
				{
					CANDIDATE[i].SubRouteStart = TMP;
					CANDIDATE[i].SubRouteEnd = TMP_A;
					CANDIDATE[i].saving = 1.0;
					i++;
					TMP_A = TMP_A->next;
				}
				TMP = TMP->next;
			}

			// second step: determine saving values
			VRP_PP_EVALUATION NewPlanEval;
			for (i = 0; i < SwapNum; i++)
			{
				this->CVRP_evaluate(P, weigthType);
			    NewPlanEval = this->get_evaluation();
				CANDIDATE[i].saving = NewPlanEval.length;
				//cout << "HUH=" << NewPlanEval.length << endl;
				this->invert_from_to(CANDIDATE[i].SubRouteStart, CANDIDATE[i].SubRouteEnd);
				this->CVRP_evaluate(P, weigthType);
				NewPlanEval = this->get_evaluation();
				CANDIDATE[i].saving = CANDIDATE[i].saving - NewPlanEval.length;
				this->invert_from_to(CANDIDATE[i].SubRouteEnd, CANDIDATE[i].SubRouteStart);
				this->CVRP_evaluate(P, weigthType);
			}

			// third step: sort CANDIDATES by decreasing saving-values
			for (i = 0; i < SwapNum - 1; i ++ )
				for (j = 0; j < SwapNum - 1; j++ )
					if (CANDIDATE[j].saving < CANDIDATE[j + 1].saving)
					{
						TmpCand = CANDIDATE[j];
						CANDIDATE[j] = CANDIDATE[j + 1];
						CANDIDATE[j + 1] = TmpCand;
					}

			// fourth step: try to implement swap
				if (CANDIDATE[0].saving > 0)
					{
						this->invert_from_to(CANDIDATE[0].SubRouteStart, CANDIDATE[0].SubRouteEnd);
						//cout << "MODIFICATION!" << endl;
					}
		}

		// release allocated memory
		delete[] CANDIDATE;
	}

}

void VRP_PP_ROUTE::print_for_xml_export(std::ostream& stream, const VRP_PP_PROBLEM*prob)
{
    stream << "\t\t\t<route type=\""<<this->type<<"\" vehicle_num=\""<<this->vehicle_num <<"\" route_id=\""<<this->route_id<<"\"/>" << endl;
}
