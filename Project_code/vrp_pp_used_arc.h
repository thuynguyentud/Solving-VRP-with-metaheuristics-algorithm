#ifndef VRP_PP_USED_ARC_H_INCLUDED
#define VRP_PP_USED_ARC_H_INCLUDED

// version 2.0 - last modification 25.02.2020 at 19:51

//predefinition
class VRP_PP_OPERATION;

// this is the object to represent an arc as part of a vehicle route.
// It is the connecting arc of two consecutively processed vehicle operation

class VRP_PP_USED_ARC {

    friend class VRP_PP_LOADING_LIST;
    friend class VRP_PP_OPERATION;

private:
    // attributes
    int arc_id;                      // reference by id to the correct arc

    class VRP_PP_OPERATION *out_op;    // reference to the outbound operation (the tail node)
    class VRP_PP_OPERATION *in_op;    // reference to the inbound operation (the head node)

public:
    // SERVICES
    VRP_PP_USED_ARC();          // standard constructor
    VRP_PP_USED_ARC(int USER_GIVEN_ARC_ID);    // other constructor
    VRP_PP_USED_ARC(int USER_GIVEN_ARC_ID, class VRP_PP_OPERATION *OUT_OP, class VRP_PP_OPERATION *IN_OP);    // other constructor
    ~VRP_PP_USED_ARC();          // standard destructor

    // GET-SERVICES
    int get_arc_id();       // returns the id of the arc
    class VRP_PP_OPERATION *get_out_op();  // returns the ptr to the outbound operation (tail) of this arc
    class VRP_PP_OPERATION *get_in_op();   // returns the ptr to the inbound operation (head) of this arc

    // disconnect operation from route
    void dissolve_from_ops(void);

    // Connect with Operations in a route
    void attach_to_outbound_op(class VRP_PP_OPERATION *_Op);
    void attach_to_inbound_op(class VRP_PP_OPERATION *_Op);
    void attach_to_ops(class VRP_PP_OPERATION *_Out_Op, class VRP_PP_OPERATION *_In_Op);

};

#endif // VRP_PP_USED_ARC_H_INCLUDED
