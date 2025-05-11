#ifndef VRP_PP_LOADING_LIST_H_INCLUDED
#define VRP_PP_LOADING_LIST_H_INCLUDED

//predefinition
struct VRP_PP_PLACED_ITEM;
class VRP_PP_OPERATION;

// version 2.0 - last manipulation 25.02.2020 at 21:29
// version 2.1 - last manipulation 15.05.2020 at 17:45
// -- adding attribute total_length
// -- adding function get_total_length(void);
// -- adding prototype function evaluation
// -- adding place_items_along_longitudinal_axis(double x_offset, class VRP_PP_PROBLEM *P);
// -- adding attribute feasible
// -- adding (x;y;z)-position of cog

// this is the object to represent a loading list of a vehicle
// leaving an operations site

class VRP_PP_LOADING_LIST {

    friend class VRP_PP_ROUTE;

private:
    // attributes
    int items;                      // number of items added to this list
    double total_load_weight;       // sum of weights of included items
    double total_cog_x_position;      // location of common cog of list items
    double total_cog_y_position;      // location of common cog of list items
    double total_cog_z_position;      // location of common cog of list items
    double total_length;            // length of the placed items
    int feasible;                   // to be used to store if this list is feasible (VRP_PP_TRUE) or not (VRP_PP_FALSE)

    class VRP_PP_OPERATION *out_op;    // reference to the outbound operation
    class VRP_PP_OPERATION *in_op;    // reference to the inbound operation

    struct VRP_PP_PLACED_ITEM *first_item;
                                    // reference to the first placed item
    struct VRP_PP_PLACED_ITEM *last_item;
                                    // reference to the first placed item
    double allowed_for_cog;         // version 3.0 - maximal payload for given cog.

public:

    friend class VRP_PP_OPERATION;

    // SERVICES
    VRP_PP_LOADING_LIST();          // standard constructor
    ~VRP_PP_LOADING_LIST();          // standard destructor

    void append_item(struct VRP_PP_PLACED_ITEM TMP);    // generates new item at the end of item list
    void print(void);                                   // prints the loading list
    void print_item_list(void);                         // prints the item list only
    void remove_item_by_id(int _id);                    // tries to remove item with id=_id from list
    void remove_all_items(void);                                   // remove all loaded items, list is not deleted
    void copy_from(class VRP_PP_LOADING_LIST *_SOURCE);
                                                        // returns a duplicate from list SOURCE
    int get_items(void);
    double get_total_load_weight(void);
    double get_total_cog_x_position(void);
    double get_total_cog_y_position(void);
    double get_total_cog_z_position(void);
    double get_total_length(void);
    class VRP_PP_OPERATION *get_out_op(void);
    class VRP_PP_OPERATION *get_in_op(void);
    struct VRP_PP_PLACED_ITEM *get_first_item(void);
    struct VRP_PP_PLACED_ITEM *get_last_item(void);

    // from the VDI 2700-4 project
    void evaluate(const class VRP_PP_PROBLEM *P);   // version 3.0 -
    void place_items_along_longitudinal_axis(double x_offset, const class VRP_PP_PROBLEM *P); // version 3.0 -
    double lastverteilungsplan(double POS, const struct VRP_PP_VEHICLE VEH, const class VRP_PP_PROBLEM *P); // version 3.0 -
    double function_a(double POS, const struct VRP_PP_VEHICLE VEH); // version 3.0 -
    double function_b(double POS, const struct VRP_PP_VEHICLE VEH); // version 3.0 -
    double function_c(double POS, const struct VRP_PP_VEHICLE VEH); // version 3.0 -
    double function_d(double POS, const struct VRP_PP_VEHICLE VEH); // version 3.0 -
    double function_e(double POS, const struct VRP_PP_VEHICLE VEH); // version 3.0 -
    double get_allowed_for_cog(void); // version 3.0 -
};

#endif // VRP_PP_LOADING_LIST_H_INCLUDED
