#include <iostream>
#include "vrp_pp_operation.h"
#include "vrp_pp_loading_list.h"
#include "vrp_pp_placed_item.h"
#include "vrp_pp_problem.h"

// version 2.0 - last modifications 25.02.2020 at 21:29
// version 2.1 - last manipulation 15.05.2020 at 17:47
// -- adding function get_total_length(void);
// -- adding function function evaluation(class VRP_PP_PROBLEM *P) (hull only)
// -- adding function place_items_along_longitudinal_axis(class VRP_PP_PROBLEM *P) (hull only)
// -- extended standard constructor in order to initialize the additional attributes total_length as well as feasible
// version 2.2 - last manipulation 17.05.2020 at 10:41
// -- integrated placed_load_item-attribute flipped
// -- adding (x;y;z)-position of cog
// version 3.0 - integration of VDI-2700-4-directives

using namespace std;

VRP_PP_LOADING_LIST::VRP_PP_LOADING_LIST()
{
    // standard constructor

    this->items = 0;
    this->in_op = NULL;
    this->out_op = NULL;
    this->total_cog_x_position = 0.0;
    this->total_cog_y_position = 0.0;
    this->total_cog_z_position = 0.0;
    this->total_load_weight = 0.0;
    this->feasible = VRP_PP_FALSE;
    this->total_length = 0.0;
    this->first_item = NULL;
    this->last_item = NULL;

    // complex exercise
    this->allowed_for_cog = 0.0;
}

VRP_PP_LOADING_LIST::~VRP_PP_LOADING_LIST()
{
    // standard destructor
    this->in_op = NULL;
    this->out_op = NULL;
    this->remove_all_items();

    delete this->first_item;
    delete this->last_item;
}

void VRP_PP_LOADING_LIST::append_item(struct VRP_PP_PLACED_ITEM TMP)
{
    // appends a new loading item to the list and fills it with TMP-values

    // create the new item
        struct VRP_PP_PLACED_ITEM *tmpPtr;
        tmpPtr = new struct VRP_PP_PLACED_ITEM(TMP.item_id,TMP.pos_x,TMP.pos_y,TMP.pos_z,TMP.cog_x, TMP.cog_y, TMP.cog_z, TMP.flipped);

    if( this->first_item != NULL )
    {
        // at least one item stored

        // append it to the list
        tmpPtr->previous = this->last_item;
        this->last_item->next = tmpPtr;
        this->last_item =tmpPtr;
    }
    else
    {
        // no items stored so far

        // add it to the list
        this->first_item = tmpPtr;
        this->last_item = tmpPtr;
    }

    // increase item counter
    this->items++;

}

void VRP_PP_LOADING_LIST::print(void)
{
    cout << "Loading List" << endl;
    cout << "global attribute values for the loading list" << endl;
    cout << this->items << " stored items" << endl;
    cout << this->total_length << "m required to store the items" << endl;
    cout << this->total_load_weight <<"to weight must be placed" << endl;
    cout << "feasibility flag" << this->feasible << endl;
    cout << "cog-position: (" << this->total_cog_x_position << ";" << this->total_cog_y_position << ";" << this->total_cog_z_position << ")" << endl;
    cout << "allowed for cog (in to): " << this->allowed_for_cog << endl;

    struct VRP_PP_VEHICLE AUX_VEH;

    // cout << "a: " << this->function_a(this->total_cog_x_position,AUX_VEH) << endl;
    // cout << "b: " << this->function_b(this->total_cog_x_position,AUX_VEH) << endl;
    // cout << "c: " << this->function_c(this->total_cog_x_position,AUX_VEH) << endl;
    // cout << "d: " << this->function_d(this->total_cog_x_position,AUX_VEH) << endl;
    // cout << "e: " << this->function_e(this->total_cog_x_position,AUX_VEH) << endl;

    if( this->in_op != NULL )
        cout << "associated inbound operation: " << this->in_op->id << endl;
    else
        cout << "no associated inbound operation" << endl;
     if( this->out_op != NULL )
        cout << "associated outbound operation: " << this->out_op->id << endl;
    else
        cout << "no associated outbound operation" << endl;
    cout << "item list:" << endl;
    struct VRP_PP_PLACED_ITEM *Tmp;
    Tmp = this->first_item;
    while( Tmp != NULL )
    {
        cout << "item id: " << Tmp->item_id;
        cout << " pos=("<<Tmp->pos_x<<";"<<Tmp->pos_y<<";"<<Tmp->pos_z<<";"<<")";
        cout << "cog=("<<Tmp->cog_x<<";"<<Tmp->cog_y<<";"<<Tmp->cog_z<<")";
        cout << "rotations before: " << Tmp->flipped;
        cout << endl;
        Tmp = Tmp->next;
    }

}

void VRP_PP_LOADING_LIST::remove_item_by_id(int _id)
{
    struct VRP_PP_PLACED_ITEM *Tmp;
    Tmp = this->first_item;
    while( Tmp != NULL )
    {
        if( Tmp->item_id == _id )
        {
            if( Tmp == this->first_item)
            {
                // first item in list must be removed
                if( Tmp == this->last_item)
                {
                    // only one item in list
                    this->first_item = NULL;
                    this->last_item = NULL;
                    delete Tmp;
                    this->items--;
                    Tmp = NULL;
                }
                else
                {
                    // more items in list
                    this->first_item->next->previous = NULL;
                    this->first_item = this->first_item->next;
                    Tmp->next = NULL;

                    delete Tmp;
                    this->items--;
                    Tmp = NULL;
                }
            }
            else
            {
                if( Tmp == this->last_item)
                {
                    // last item in list must be removed
                    this->last_item = Tmp->previous;
                    this->last_item->next = NULL;
                    Tmp->previous = NULL;

                    delete Tmp;
                    this->items--;
                    Tmp = NULL;
                }
                else
                {
                    // intermediate item must be removed
                    Tmp->previous->next = Tmp->next;
                    Tmp->next->previous = Tmp->previous;
                    Tmp->previous = NULL;
                    Tmp->next = NULL;

                    delete Tmp;
                    this->items--;
                    Tmp = NULL;
                }
            }
        }
        else
        {
            Tmp = Tmp->next;
        }
    }
}

void VRP_PP_LOADING_LIST::remove_all_items(void)
{
    // this procedure removes the last placed item as long such an item
    // exists. Result: list of placed items is empty

    struct VRP_PP_PLACED_ITEM *TmpPtr;

    TmpPtr = this->last_item;

    while( TmpPtr != NULL )
    {
        this->remove_item_by_id(this->last_item->item_id);
         // go to ptr of last placed item
        TmpPtr = this->last_item;
    }

    if( this->items > 0)
    {
        cout << "FEHLER" << this->items << endl;
        exit(0);
    }
}

void VRP_PP_LOADING_LIST::copy_from(class VRP_PP_LOADING_LIST *_SOURCE)
{
    // this procedure duplicates the loading list _SOURCE

    // first destroy the existing content
    this->remove_all_items();

    delete this->first_item;
    delete this->last_item;

    if( this->items > 0)
    {
        cout << "Energency brake" << endl;
        exit(0);
    }
    struct VRP_PP_PLACED_ITEM *lauf_SOURCE, *NewItem;
    lauf_SOURCE = _SOURCE->first_item;

    // copy the existing load item sequence
    while( lauf_SOURCE != NULL )
    {
        NewItem = new class VRP_PP_PLACED_ITEM(lauf_SOURCE->item_id,lauf_SOURCE->pos_x,lauf_SOURCE->pos_y,lauf_SOURCE->pos_z,lauf_SOURCE->cog_x,lauf_SOURCE->cog_y,lauf_SOURCE->cog_z,lauf_SOURCE->flipped);
        this->append_item(*NewItem);
        delete NewItem;
        lauf_SOURCE = lauf_SOURCE->next;
    }

    // copy the static values
    this->in_op = NULL;
    this->out_op = NULL;
    this->total_cog_x_position = _SOURCE->total_cog_x_position;
    this->total_cog_y_position = _SOURCE->total_cog_y_position;
    this->total_cog_z_position = _SOURCE->total_cog_z_position;
    this->total_load_weight = _SOURCE->total_load_weight;
    this->total_length = _SOURCE->total_length;
    this->feasible = _SOURCE->feasible;
};

int VRP_PP_LOADING_LIST::get_items(void)
{
    return(this->items);
}

double VRP_PP_LOADING_LIST::get_total_load_weight(void)
{
    return(this->total_load_weight);
}

double VRP_PP_LOADING_LIST::get_total_cog_x_position(void)
{
    return(this->total_cog_x_position);
}

double VRP_PP_LOADING_LIST::get_total_cog_y_position(void)
{
    return(this->total_cog_y_position);
}

double VRP_PP_LOADING_LIST::get_total_cog_z_position(void)
{
    return(this->total_cog_z_position);
}

class VRP_PP_OPERATION *VRP_PP_LOADING_LIST::get_out_op(void)
{
    return(this->out_op);
}

class VRP_PP_OPERATION *VRP_PP_LOADING_LIST::get_in_op(void)
{
    return(this->in_op);
}

struct VRP_PP_PLACED_ITEM *VRP_PP_LOADING_LIST::get_first_item(void)
{
    return(this->first_item);
}

struct VRP_PP_PLACED_ITEM *VRP_PP_LOADING_LIST::get_last_item(void)
{
    return(this->last_item);
}

double VRP_PP_LOADING_LIST::get_total_length(void)
{
    return(this->total_length);
}

void VRP_PP_LOADING_LIST::print_item_list(void)
{
    cout << "Loading List with items: ";
    struct VRP_PP_PLACED_ITEM *Tmp;
    Tmp = this->first_item;
    while( Tmp != NULL )
    {
        cout << "item id: " << Tmp->item_id << ";";
        Tmp = Tmp->next;
    }
    cout << endl;
}

// HERE STARTS THE COMPLEX EXERCISE

void VRP_PP_LOADING_LIST::evaluate(const class VRP_PP_PROBLEM *P)
{
    //cout << "-------- START LOADING LIST EVALUATION ------" << endl;
    //this->print();
    //cout << "---------------------------------" << endl;

    // re-initialiize values
    this->total_length = 0.0;
    this->total_cog_x_position = 0.0;
    this->total_load_weight = 0.0;
    this->feasible = 1;
    this->allowed_for_cog = 0.0;

    // Number of items in the list is already stored

    // Total load weight and total cog determination

    class VRP_PP_LOAD_ITEM CurrentItem;

    double zaehler = 0.0;
    double nenner = 0.0;
    struct VRP_PP_PLACED_ITEM *Ptr;
    Ptr = this->get_first_item();
    while( Ptr != NULL )
    {
        CurrentItem = P->get_load_item_by_index(Ptr->item_id);

        zaehler = zaehler + (Ptr->cog_x * CurrentItem.get_item_weight());
        nenner = nenner + CurrentItem.get_item_weight();

        this->total_load_weight += CurrentItem.get_item_weight();
        Ptr = Ptr->next;
    }

    if( nenner > 0.0)
        this->total_cog_x_position = zaehler/nenner;
    else
        this->total_cog_x_position = 0;

    // set length
    // go to last item and fetch the associated load item
    Ptr = this->get_last_item();
    if( Ptr != NULL )
    {
        // there is at least one item in the list
        CurrentItem = P->get_load_item_by_index(Ptr->item_id);
        this->total_length = this->last_item->pos_x + CurrentItem.get_item_length();
    }

     // identify the current vehicle in order to get access to the vehicle data

    int CurrentVehicle;
    CurrentVehicle = this->get_out_op()->get_id_route();

    // check if load is placed completely on the load bed
    if( this->total_length > P->get_vehicle_by_index(CurrentVehicle).load_bed_length)
    {
        // partial placement of load outside load_bed_length
        this->feasible = -1;
    }


    // check weight
    if( this->total_load_weight > P->get_vehicle_by_index(CurrentVehicle).cap )
    {
        // load is to heavy
        this->feasible = -1;
    }


    // cout << "CurrentVehicle " << CurrentVehicle << endl;

    // Vergleich mit Lastverteilungsplan
    // cout << "total weight of payload " << this->get_total_load_weight() << endl;
    // cout << "cog position " << this->get_total_cog_x_position() << endl;
    // cout << "max allowed at cog " << this->lastverteilungsplan(this->get_total_cog_x_position(),P->get_vehicle_by_index(CurrentVehicle),P) << endl;;


    this->allowed_for_cog = this->lastverteilungsplan(this->get_total_cog_x_position(),P->get_vehicle_by_index(CurrentVehicle),P);
    if( this->allowed_for_cog < this->total_load_weight )
    {
        // cout << "INFEASIBLE PLACEMENT" << endl;
        // cout << "requested " << this->total_load_weight << endl;
        // cout << "allowed " << this->allowed_for_cog << endl;
        this->feasible = -2;
    }
    else
    {
        //cout << "FEASIBLE PLACEMENT" << endl;
        //cout << "requested " << this->total_load_weight << endl;
        //cout << "allowed " << this->allowed_for_cog << endl;
    }    // HIER

    //cout << "-------- End LOADING LIST EVALUATION -------" << endl;

}

void VRP_PP_LOADING_LIST::place_items_along_longitudinal_axis(double x_offset, const class VRP_PP_PROBLEM *P)
{
    // nur Position der Referenzpunkte und der Schwerpunkte bestimmen
    double StartPosRefPoint = x_offset;

    class VRP_PP_LOAD_ITEM CurrentItem;
    struct VRP_PP_PLACED_ITEM *Ptr;
    Ptr = this->get_last_item();
    while( Ptr != NULL )
    {
        Ptr->pos_x = StartPosRefPoint;
        CurrentItem = P->get_load_item_by_index(Ptr->item_id);
        Ptr->cog_x = Ptr->pos_x + CurrentItem.get_cog_length();
        StartPosRefPoint = StartPosRefPoint + CurrentItem.get_item_length();
        Ptr = Ptr->previous;
    }
}

double VRP_PP_LOADING_LIST::lastverteilungsplan(double POS, const struct VRP_PP_VEHICLE VEH, const class VRP_PP_PROBLEM *P)
{
    double value_a=-1.0, value_b=-1.0, value_c=-1.0, value_d=-1.0, value_e=-1.0;

    // this procedure determines the maximal allowed payload according to the lastverteilungsplan at position POS
    double ret_val = VRP_PP_BIGM;

    if( POS >= 0 )
    {
        if( POS <= VEH.load_bed_length )
        {
            // POS is in the definition area of the lastverteilungsplan function
            value_a = this->function_a(POS,VEH);
            if( value_a < ret_val )
                ret_val = value_a;
            value_b = this->function_b(POS,VEH);
            if( value_b < ret_val )
                ret_val = value_b;
            value_c = this->function_c(POS,VEH);
            if( value_c < ret_val )
                ret_val = value_c;
            value_d = this->function_d(POS,VEH);
            if( value_d < ret_val )
                ret_val = value_d;
            value_e = this->function_e(POS,VEH);
            if( value_e < ret_val )
                ret_val = value_e;
        }
        else
            ret_val = 0;
    }
    else
        ret_val = 0;

    return ret_val;
}

double VRP_PP_LOADING_LIST::function_a(double POS, const struct VRP_PP_VEHICLE VEH)
{
    // this procedure calculates the maximal allowed load of the loaded truck on the rear axle
    double ret_val = -1;

    double zaehler;
    double nenner;

    zaehler = VEH.front_axle_load_full*VEH.wheel_base - VEH.empty_mass*(VEH.wheel_base-VEH.cog_empty);
    nenner = VEH.wheel_base - VEH.dist_to_loadarea - POS;

    if( nenner > 0 )
        ret_val = zaehler / nenner;
    else
        ret_val = VEH.cap;

    return ret_val;
}

double VRP_PP_LOADING_LIST::function_b(double POS, const struct VRP_PP_VEHICLE VEH)
{
    double ret_val = -1;

    double zaehler;
    double nenner;

    zaehler = VEH.rear_axle_load_full * VEH.wheel_base - VEH.empty_mass*VEH.cog_empty;
    nenner = VEH.dist_to_loadarea + POS;

    if( nenner > 0 )
        ret_val = zaehler / nenner;
    else
        ret_val = VEH.cap;

    return ret_val;
}

double VRP_PP_LOADING_LIST::function_c(double POS, const struct VRP_PP_VEHICLE VEH)
{
    double ret_val = -1;

        if( VEH.cap <= 0)
            ret_val = 0.0;
        else
            ret_val = VEH.cap;

    return ret_val;
}

double VRP_PP_LOADING_LIST::function_d(double POS, const struct VRP_PP_VEHICLE VEH)
{
    double ret_val = -1;

    double zaehler;
    double nenner;

    zaehler = VEH.empty_mass*(VEH.wheel_base-VEH.cog_empty-VEH.min_load_stearing*VEH.wheel_base);
    nenner = VEH.min_load_stearing*VEH.wheel_base + VEH.dist_to_loadarea+POS-VEH.wheel_base;

    if( nenner > 0 )
        ret_val = zaehler / nenner;
    else
        ret_val = VEH.cap;

    return ret_val;
}

double VRP_PP_LOADING_LIST::function_e(double POS, const struct VRP_PP_VEHICLE VEH)
{
    double ret_val = -1;

    double zaehler;
    double nenner;

    zaehler = VEH.empty_mass*(VEH.min_load_traction*VEH.wheel_base - VEH.cog_empty);
    nenner = VEH.dist_to_loadarea+POS-VEH.min_load_traction*VEH.wheel_base;

    if( nenner > 0 )
        if( (zaehler / nenner) >= 0 )
            ret_val = zaehler / nenner;
        else
            ret_val = VEH.cap;
    else
        ret_val = VEH.cap;

    return ret_val;
}


double VRP_PP_LOADING_LIST::get_allowed_for_cog(void)
{
    return(this->allowed_for_cog);
}
