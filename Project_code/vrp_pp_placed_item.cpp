#include <iostream>
#include "vrp_pp_placed_item.h"

// version 2.0 - last manipulation 25.02.2020 at 22:08
// Version 2.1 - last manipulation 17.05.2020 at 10:35
// -- adding attribute flipped to store rotation activities
// -- modfying non-standard constructor parameter list
// -- adding void set_flipped(int _flipped);
// -- standard constructor: set initial reference values to NULL

using namespace std;

VRP_PP_PLACED_ITEM::VRP_PP_PLACED_ITEM()
{
    // this is the standard constructor
    //cout << "this is VRP_PP_PLACED_ITEM standard constructor" << endl;
    this->next = NULL;
    this->previous = NULL;
}

VRP_PP_PLACED_ITEM::VRP_PP_PLACED_ITEM(int _item_id, double _pos_x, double _pos_y, double _pos_z, double _cog_x, double _cog_y, double _cog_z, int _flipped)
{
    this->item_id = _item_id;
    this->pos_x = _pos_x;
    this->pos_y = _pos_y;
    this->pos_z = _pos_z;
    this->cog_x = _cog_x;
    this->cog_y = _cog_y;
    this->cog_z = _cog_z;
    this->flipped = _flipped;
    this->next = NULL;
    this->previous = NULL;
}

void VRP_PP_PLACED_ITEM::print(void)
{
    cout << "-->";
    cout << " this is the placed item id=" << this->item_id;
    cout << " positioned at ("<<this->pos_x <<";"<<this->pos_y <<";"<<this->pos_z <<")";
    cout << " with cog at ("<<this->cog_x <<";"<<this->cog_y <<";"<<this->cog_z <<")";
    cout << " rotations before: " << this->flipped;
    cout << endl;
}

struct VRP_PP_PLACED_ITEM VRP_PP_PLACED_ITEM::get_placed_item(void)
{

    struct VRP_PP_PLACED_ITEM TmpPtr;

    TmpPtr.item_id = this->item_id;            // reference to the item in problem
    TmpPtr.pos_x = this->pos_x;           // absolute (x;y;z)-position of item in truck
    TmpPtr.pos_y = this->pos_y;
    TmpPtr.pos_z = this->pos_z;
    TmpPtr.cog_x = this->cog_x;           // absolute (x;y;z) position of center of gravity
    TmpPtr.cog_y = this->cog_y;           // of item on truck
    TmpPtr.cog_z = this->cog_z;
    TmpPtr.flipped = this->flipped;
    TmpPtr.next = this->next;
    TmpPtr.previous = this->previous;

    return(TmpPtr);
}

VRP_PP_PLACED_ITEM::~VRP_PP_PLACED_ITEM()
{
    // this is the standard destructor
    // cout << "this is VRP_PP_PLACED_ITEM standard destructor" << endl;
    this->next = NULL;
    delete this->next;
    this->previous = NULL;
    delete this->previous;
}

void VRP_PP_PLACED_ITEM::set_item_id(int _item_id)
{
    this->item_id = _item_id;
}

void VRP_PP_PLACED_ITEM::set_pos_x(double _pos_x)
{
    this->pos_x = _pos_x;
}

void VRP_PP_PLACED_ITEM::set_pos_y(double _pos_y)
{
    this->pos_y = _pos_y;
}

void VRP_PP_PLACED_ITEM::set_pos_z(double _pos_z)
{
    this->pos_z = _pos_z;
}

void VRP_PP_PLACED_ITEM::set_cog_x(double _cog_x)
{
    this->cog_x = _cog_x;
}

void VRP_PP_PLACED_ITEM::set_cog_y(double _cog_y)
{
    this->cog_y = _cog_y;
}

void VRP_PP_PLACED_ITEM::set_cog_z(double _cog_z)
{
    this->cog_z = _cog_z;
}

void VRP_PP_PLACED_ITEM::set_flipped(int _flipped)
{
    this->flipped = _flipped;
}

