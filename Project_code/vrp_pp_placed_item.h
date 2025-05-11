#ifndef VRP_PP_PLACED_ITEM_H_INCLUDED
#define VRP_PP_PLACED_ITEM_H_INCLUDED

// Version 2.0
//
// 25.02.2020 - 21:40 jsb
//
// Version 2.1 - last manipulation 17.05.2020 at 10:32
// -- adding attribute flipped to store rotation activities
// -- modfying non-standard constructor parameter list
// -- adding void set_flipped(int _flipped);

class VRP_PP_PLACED_ITEM {

    friend class VRP_PP_LOADING_LIST;

private:
    // attributes
    int item_id;            // reference to the item in problem

    double pos_x;           // absolute (x;y;z)-position of item in truck
    double pos_y;
    double pos_z;

    double cog_x;           // absolute (x;y;z) position of center of gravity
    double cog_y;           // of item on truck
    double cog_z;

    int flipped;            // information about applied rotations of item to get recent orientation

    class VRP_PP_PLACED_ITEM *next;
    class VRP_PP_PLACED_ITEM *previous;

public:

    // services
    VRP_PP_PLACED_ITEM();       // standard constructor
    VRP_PP_PLACED_ITEM(int _item_id, double pos_x, double pos_y, double pos_z, double cog_x, double cog_y, double cog_z, int flipped);
    ~VRP_PP_PLACED_ITEM();      // standard desctructor

    // get
    class VRP_PP_PLACED_ITEM get_placed_item(void);

    // set
    void set_item_id(int item_id);
    void set_pos_x(double _pos_x);
    void set_pos_y(double _pos_y);
    void set_pos_z(double _pos_z);

    void set_cog_x(double _cog_x);
    void set_cog_y(double _cog_y);
    void set_cog_z(double _cog_z);

    void set_flipped(int _flipped);
    //others
    void print(void);
};

#endif // VRP_PP_PLACED_ITEM_H_INCLUDED
