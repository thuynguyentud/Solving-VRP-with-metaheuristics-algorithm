#include <iostream>
#include <iomanip>

#include "vrp_pp.h"
#include "vrp_pp_load_item.h"

// version 2.0 - last modification 25.02.2020 at 18:32

using namespace std;



VRP_PP_LOAD_ITEM::VRP_PP_LOAD_ITEM()
{
    // standard constructor;
}

VRP_PP_LOAD_ITEM::~VRP_PP_LOAD_ITEM()
{
    // standard destructor;
}

void VRP_PP_LOAD_ITEM::print(void)
{
    cout << "->load_item id " << this->id << endl;
    cout << "->cog_width=" << this->cog_width << endl;
    cout << "->cog_length=" << this->cog_length << endl;
    cout << "->cog_height=" << this->cog_height << endl;
    cout << "->desc=" << this->desc << endl;
    cout << "->flipping=" << this->flipping << endl;
    cout << "->item_height=" << this->item_height << endl;
    cout << "->item_width=" << this->item_width << endl;
    cout << "->item_length=" << this->item_length << endl;
    cout << "->request=" << this->request << endl;
}

int VRP_PP_LOAD_ITEM::get_id(void)
{
    return(this->id);
}

double VRP_PP_LOAD_ITEM::get_cog_width(void)
{
    return(this->cog_width);
}

double VRP_PP_LOAD_ITEM::get_cog_height(void)
{
    return(this->cog_height);
}

double VRP_PP_LOAD_ITEM::get_cog_length(void)
{
    return(this->cog_length);
}

string VRP_PP_LOAD_ITEM::get_desc(void)
{
    return(this->desc);
}

int VRP_PP_LOAD_ITEM::get_flipping(void)
{
    return(this->flipping);
}

double VRP_PP_LOAD_ITEM::get_item_height(void)
{
    return(this->item_height);
}

double VRP_PP_LOAD_ITEM::get_item_width(void)
{
    return(this->item_width);
}

double VRP_PP_LOAD_ITEM::get_item_length(void)
{
    return(this->item_length);
}

int VRP_PP_LOAD_ITEM::get_request(void)
{
    return(this->request);
}

double VRP_PP_LOAD_ITEM::get_item_weight(void)
{
    return(this->item_weight);
}
