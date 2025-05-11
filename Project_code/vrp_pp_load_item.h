#ifndef VRP_PP_LOAD_ITEM_H_INCLUDED
#define VRP_PP_LOAD_ITEM_H_INCLUDED

// version 2.0 - last modification 25.02.2020 at 18:31
//
// class VRP_PP_LOAD_ITEM
//
// this structure is used to represent a loaded item that
// must be positioned on a vehicle. A load item must be specified in the problem description

class VRP_PP_LOAD_ITEM {
    //
    // this structure refers to a single item that is associated with a
    // request from type VRP_PP_REQUEST. In the xmö-file that contains the instance
    // description it can be found in the section <LOADITEMS> as a <LOADITEM>

    friend class VRP_PP_PROBLEM;

    // attributes
    private:
    int id;                 // unique identifier of this load item
    int request;            // refers to the request (type VRP_PP_REQUEST) to which
                            // this item belongs

    std::string desc;       // plain text description of the item

    double item_width;      // on edge parallel to vehicles bumper (in meters)
    double item_length;     // on edge parallel to road (in meters)
    double item_height;     // (in meters)

    double item_weight;     // measured in tons

    double cog_width;       // abs. position of center of gravity of item
    double cog_length;      // abs. position of center of gravity of item
    double cog_height;      // abs. position of center of gravity of item

    int flipping;           // describes type of allowed rotations before item is placed

    public:
    // services
    VRP_PP_LOAD_ITEM();        // standard constructor
    ~VRP_PP_LOAD_ITEM();       // standard destructor

    void print(void);
    int get_id(void);
    double get_cog_width(void);
    double get_cog_height(void);
    double get_cog_length(void);
    string get_desc(void);
    int get_flipping(void);
    double get_item_height(void);
    double get_item_width(void);
    double get_item_length(void);
    double get_item_weight(void);
    int get_request(void);
};

#endif // VRP_PP_LOAD_ITEM_H_INCLUDED
