#include <iostream>

// version 2.0 - last-modification 25.02.2020 at 19:21

using namespace std;

#include "vrp_pp_operation.h"
#include "vrp_pp_route.h"
#include "vrp_pp_loading_list.h"
#include "pugixml.hpp"

VRP_PP_OPERATION::VRP_PP_OPERATION()
{
	// this is the standard constructor
	id = -1;
	id_node = -1;
	id_route = -1;
	type = VRP_PP_DUMMY;
	reachtime = 0.0;
	starttime = 0.0;
	finishtime = 0.0;
	previous = NULL;
	next = NULL;
	desc = "";

	ob_ll = NULL;
	ib_ll = NULL;

	out_arc = NULL;
	in_arc = NULL;
}

VRP_PP_OPERATION::VRP_PP_OPERATION(const VRP_PP_OPERATION & source)
	: type(source.type), id_route(-1),
	id(source.id), reachtime(source.reachtime), starttime(source.starttime),
	finishtime(source.finishtime), next(NULL),id_node(source.id_node),
	previous(NULL), ob_ll(NULL), ib_ll(NULL), out_arc(NULL), in_arc(NULL)
{

}

VRP_PP_OPERATION::VRP_PP_OPERATION(int id, int id_node, int type, const std::string& desc)
{
	this->type = type;					// type of operation (VRP_PP_START, VRP_PP_STOP, VRP_PP_PICKUP, VRP_PP_DELIVERY)
	this->desc = desc;
	this->id_node = id_node;
	this->id_route = -1;
	this->id = id;

	this->reachtime = 0;	            // Zeitpunkt, zu dem das bedienende Fahrzeug am Ort dieser Operation ankommt
	this->starttime = 0;	            // Zeitpunkt, zu dem das bedienende Fahrzeug mit der Ausführung dieser Operation startet
	this->finishtime = 0;	            // Zeitpunkt, zu dem das bedienende Fahrzeug die Ausführung dieser Operation komplettiert

	this->previous = NULL;
	this->next = NULL;

	this->ob_ll = NULL;
	this->ib_ll = NULL;

	this->out_arc = NULL;
	this->in_arc = NULL;
}


VRP_PP_OPERATION::~VRP_PP_OPERATION()
{
    // this is the standard destructor

    this->previous = NULL;
	this->next = NULL;

	if( this->ob_ll != NULL )
    {
        this->ob_ll->remove_all_items();
        this->ob_ll->~VRP_PP_LOADING_LIST();
        delete this->ob_ll;
    }
    if( this->ib_ll != NULL )
    {
        this->ib_ll->remove_all_items();
        this->ib_ll->~VRP_PP_LOADING_LIST();
        delete this->ib_ll;
    }
}

void VRP_PP_OPERATION::print(void)
{
    // this is a simple print procedure
    cout << "ATTRIBUTES" << endl;
    cout << "operation id: " << this->id << endl;
    cout << "assoc. node: " << this->id_node << endl;
    cout << "assoc. route: " << this->id_route << endl;
    cout << "description: " << this->desc << endl;
    cout << "type: " << this->type << endl;
    if( this->previous != NULL )
        cout << "previous operation: " << this->previous->id << endl;
    if( this->next != NULL )
        cout << "next operation: " << this->next->id << endl;
    cout << "INBOUND LOADING LIST" << endl;
    if( this->ib_ll != NULL)
        this->ib_ll->print();
    else
        cout << "no inbound loading list installed" << endl;
    cout << "OUTBOUND LOADING LIST" << endl;
    if( this->ob_ll != NULL)
        this->ob_ll->print();
    else
        cout << "no outbound loading list installed" << endl;
    cout << "DECISISONS" << endl;
    if( this->out_arc != NULL)
        cout << "used outbound arc = " << this->out_arc->get_arc_id() << endl;
    else
        cout << "no outbound arc specified" << endl;
    if( this->in_arc != NULL)
        cout << "used inbound arc = " << this->out_arc->get_arc_id() << endl;
    else
        cout << "no inbound arc specified" << endl;

    cout << "reachtime: " << this->reachtime << endl;
    cout << "starttime: " << this->starttime << endl;
    cout << "finishtime: " << this->finishtime << endl;
}

void VRP_PP_OPERATION::link_to_outbound_loading_list(class VRP_PP_LOADING_LIST *PtrToLoadingList)
{
    // added by jsb 09.01.2020
    // this procedure connects this operation with the loading list stored at PtrToLoadingList

    this->ob_ll = PtrToLoadingList;
    PtrToLoadingList->out_op = this;
}

void VRP_PP_OPERATION::link_to_inbound_loading_list(class VRP_PP_LOADING_LIST *PtrToLoadingList)
{
    // added by jsb 09.01.2020
    // this procedure connects this operation with the loading list stored at PtrToLoadingList

    this->ib_ll = PtrToLoadingList;
    PtrToLoadingList->in_op = this;
}

void VRP_PP_OPERATION::uncouple_outbound_loading_list(class VRP_PP_LOADING_LIST *PtrToLoadingList)
{
    // added by jsb 09.01.2020
    // this procedure uncouples an outbound loading list from this operation.
    // the address of the outbound list is saved using PtrToLoadingList

    if( this->ob_ll != NULL )
    {
        // an outbound loading list exists here
        // save the address of the list
        PtrToLoadingList = this->ob_ll;

        // delete reference to operation
        this->ob_ll->out_op = NULL;

        // uncouple outbound loading list from operation
        this->ob_ll = NULL;
    }
}

void VRP_PP_OPERATION::uncouple_inbound_loading_list(class VRP_PP_LOADING_LIST *PtrToLoadingList)
{
    // added by jsb 09.01.2020
    // this procedure uncouples an inbound loading list from this operation.
    // the address of the inbound list is saved using PtrToLoadingList

    if( this->ib_ll != NULL )
    {
        // an inbound loading list exists here
        // save the address of the list
        PtrToLoadingList = this->ib_ll;

        // delete reference to operation
        this->ib_ll->in_op = NULL;

        // uncouple outbound loading list from operation
        this->ib_ll = NULL;
    }
}

int VRP_PP_OPERATION::get_id(void)
{
    return(this->id);
}

int VRP_PP_OPERATION::get_id_node(void)
{
    return(this->id_node);
}

int VRP_PP_OPERATION::get_type(void)
{
    return(this->type);
}

string VRP_PP_OPERATION::get_desc(void)
{
    return(this->desc);
}

double VRP_PP_OPERATION::get_reachtime(void)
{
    return(this->reachtime);
}

double VRP_PP_OPERATION::get_starttime(void)
{
    return(this->starttime);
}

double VRP_PP_OPERATION::get_finishtime(void)
{
    return(this->finishtime);
}

class VRP_PP_OPERATION *VRP_PP_OPERATION::get_next(void)
{
    return(this->next);
}

class VRP_PP_OPERATION *VRP_PP_OPERATION::get_previous(void)
{
    return(this->previous);
}

class VRP_PP_LOADING_LIST *VRP_PP_OPERATION::get_ob_ll(void)
{
    return(this->ob_ll);
}

class VRP_PP_LOADING_LIST *VRP_PP_OPERATION::get_ib_ll(void)
{
    return(this->ib_ll);
}

class VRP_PP_USED_ARC *VRP_PP_OPERATION::get_out_arc(void)
{
    return(this->out_arc);
}

class VRP_PP_USED_ARC *VRP_PP_OPERATION::get_in_arc(void)
{
    return(this->in_arc);
}

long long VRP_PP_OPERATION::get_id_route(void)
{
    return(this->id_route);
}
void VRP_PP_OPERATION::set_next(class VRP_PP_OPERATION *Ptr)
{
    this->next = Ptr;
}

void VRP_PP_OPERATION::set_previous(class VRP_PP_OPERATION *Ptr)
{
    this->previous = Ptr;
}

void VRP_PP_OPERATION::print_for_xml_export(std::ostream& stream, const VRP_PP_PROBLEM*prob)
{
    stream << "\t\t\t<operation id=\""<<this->id << "\"";
    stream << " id_node=\""<<this->id_node << "\"";
    stream << " type=\""<<this->type << "\"";
    stream << " desc=\""<<this->desc << "\"";
    stream << " id_route=\""<<this->id_route << "\"";
    stream << " reachtime=\""<<this->reachtime << "\"";
    stream << " starttime=\""<<this->starttime << "\"";
    stream << " finishtime=\""<<this->finishtime << "\"";
    stream << "/>\n";
}
