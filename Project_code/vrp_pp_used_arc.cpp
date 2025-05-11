#include <iostream>
#include "vrp_pp_operation.h"
#include "vrp_pp_used_arc.h"

// this file contains the definition of the services associated with the
// template object VRP_PP_USED_ARC

VRP_PP_USED_ARC::VRP_PP_USED_ARC()
{
    // standard constructor
    this->arc_id = -1;
    this->in_op = NULL;
    this->out_op = NULL;
}

VRP_PP_USED_ARC::VRP_PP_USED_ARC(int USER_GIVEN_ARC_ID)
{
    // constructor which uses a given arc number
    this->arc_id = USER_GIVEN_ARC_ID;
    this->in_op = NULL;
    this->out_op = NULL;
}

VRP_PP_USED_ARC::VRP_PP_USED_ARC(int USER_GIVEN_ARC_ID, class VRP_PP_OPERATION *_OUTBOUND_OP, class VRP_PP_OPERATION *_INBOUND_OP)
{
    // constructor connecting the arc directly with the node-sequence
    this->arc_id = USER_GIVEN_ARC_ID;
    this->in_op = _INBOUND_OP;
    if( this->in_op != nullptr )
    {
        this->in_op->in_arc = this;
    }
    this->out_op = _OUTBOUND_OP;
    if( this->out_op != nullptr )
    {
        this->out_op->out_arc = this;
    }
}

int VRP_PP_USED_ARC::get_arc_id()
{
    return(this->arc_id);
}

void VRP_PP_USED_ARC::attach_to_outbound_op(class VRP_PP_OPERATION *_Op)
{
    // this method connects this used_arc-object to the operation _Op as outbound operation (tail operation)

    this->out_op = _Op;
    _Op->out_arc = this;
}

void VRP_PP_USED_ARC::attach_to_inbound_op(class VRP_PP_OPERATION *_Op)
{
    // this method connects this used_arc-opject to the operation _Op as outbound operation (tail operation)

    this->in_op = _Op;
    _Op->in_arc = this;
}

void VRP_PP_USED_ARC::attach_to_ops(class VRP_PP_OPERATION *_Out_Op, class VRP_PP_OPERATION *_In_Op)
{
    this->attach_to_outbound_op(_Out_Op);
    this->attach_to_inbound_op(_In_Op);
}

class VRP_PP_OPERATION *VRP_PP_USED_ARC::get_out_op()
{
    return(this->out_op);
}

class VRP_PP_OPERATION *VRP_PP_USED_ARC::get_in_op()
{
    return(this->in_op);
}

void VRP_PP_USED_ARC::dissolve_from_ops(void)
{
    // the procedure cuts all connections from this arc template object
    // after this procedure has been called and if all references have been set
    // properly before, this object can be destroyed without causing any
    // inconsistencies in references
    if( this->out_op != NULL )
    {
        this->out_op->out_arc = NULL;
        this->out_op = NULL;
    }
    if( this->in_op != NULL )
    {
        this->in_op->in_arc = NULL;
        this->in_op = NULL;
    }
}


VRP_PP_USED_ARC::~VRP_PP_USED_ARC()
{
    // standard destructor
    this->in_op = NULL;
    this->out_op = NULL;
}
