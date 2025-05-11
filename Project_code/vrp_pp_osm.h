#ifndef VRP_PP_OSM_H_INCLUDED
#define VRP_PP_OSM_H_INCLUDED

#include <string>
#include "vrp_pp_extern.h"
#define VRP_PP_OSM
#ifdef VRP_PP_OSM
#pragma message "vrp++ osm components are considered"
#include "libpq-fe.h"


// this object template is used to get and store additional network data taken from
// the OSM-database

struct VRP_PP_OSM_WAYPOINT {
    // we use this struct to represent a way point in a way that connects origin and
    // destination of an arc (i->j is replaced by a detailed OSM-way)

    friend class VRP_PP_OSM_TWIN;
    friend class VRP_PP_PLAN;

    long SEQ;         // position in way sequence of arc
    long SEQ_ID;        // position in way sequence of arc
    long DB_NODE_ID;     // refers to the local db node id (in the osm-local-db)
    long DB_EDGE_ID;     // refers to the next edge to travers
    double ADD_COSTS;     // cost increment by traversing DB_EDGE_ID
    double AGG_COSTS;     // aggregated costs after having traversed DB_EDGE_ID
    double LON;           // longitude WGS84
    double LAT;             // latitude WGS84
    //change bool
    bool SYNCHRONIZED;   // indicator to check if node has successfully synchronized
    struct VRP_PP_OSM_WAYPOINT *next;   // refers to the direct follower
    struct VRP_PP_OSM_WAYPOINT *prev;   // refers to the direct predecessor

    void print(void);   // print the waypoint data to the console
    void sync_lonlat(class VRP_PP_OSM_TWIN *_TWIN); // try to find the lon/lat-value from db
};

struct VRP_PP_OSM_NODE {
    // we use this struct to store all information associated with the node ID which we have retrieved
    // from the OpenStreetMap-Database in use

    long ID;            // refers to the id of the current VRP_PP_PROBLEM object
    long DB_ID;         // refers to the id used in the currently used database
    long OSM_ID;        // refers to the id used in the one and only osm-world
    double OSM_LAT;     // refers to the latitude (y-value) found in the currently used database
    double OSM_LON;     // refers to the longitude (x-value) found in the currently used database
};

struct VRP_PP_OSM_ARC {
    // we use this struct to store all information associated with the arc ID which we have retrieved
    // from the OpenStreetMap-Database in use

    long ID;                    // refers to the id of the current VRP_PP_PROBLEM object
    long DB_ID;                 // refers to the id used in the currently used database
    long OSM_ID;                // refers to the id used in the one and only osm-world
    double OSM_LENGTH;          // this is the travel distance in the OSM-world (in kilometers)
    double OSM_MAXSPEED;        // not in use
    long ORIGIN_DB_ID;          // refers to the id of the start node used in the currently used database
    long ORIGIN_OSM_ID;         // refers to the id of the start node used in the one and only osm-world
    long DEST_DB_ID;            // refers to the id of the dest node used in the currently used database
    long DEST_OSM_ID;           // refers to the id of the dest node used in the one and only osm-world
    struct VRP_PP_OSM_WAYPOINT *FIRST_WAYPOINT;     // reference to the first assoc. waypoint
    struct VRP_PP_OSM_WAYPOINT *LAST_WAYPOINT;      // reference to the last assoc. waypoint

    // adding a waypoint as new last waypoint to the existing waypoint sequence
    void append_waypoint(struct VRP_PP_OSM_WAYPOINT *Ptr);

    // remove all empty waypoints from the stored waypoint-sequence
    void remove_waypoints(void);
};

using namespace std;

class VRP_PP_OSM_TWIN {
    // we use this object template to store all additional information about arcs and nodes
    // in our problem we have retrieved from an OSM database

    // granting direct access for our friends
    friend class VRP_PP_OSM_WAYPOINT;
    friend class VRP_PP_PLAN;

    // this is the handler to the database
    PGconn *DBCON;

    // we use the following data to specify the access allowances to an OSM-database
    char HOSTNAME[256];
    char PORT[256];
    char DB_NAME[256];
    char DB_USER[256];
    char DB_PASSWORD[256];

    //change bool
    bool USE_DIRECTED_GRAPH; // true iff we assume that the underlying graph is directed
    bool SILENT_MODE;        // true iff we do not want to see SQL-access progress

    int NODES;      // the number of nodes of the problem under consideration
    int ARCS;       // the number of arc of the problem under consideration

    struct VRP_PP_OSM_NODE *OSM_NODE;   // array to store the additional node information
    struct VRP_PP_OSM_ARC *OSM_ARC;     // array to store the additional arc information

public:
    // this is the constructor. It uses the number of nodes and arcs in _PROB to install the OSM_NODE
    // and OSM_ARC arrays
    VRP_PP_OSM_TWIN(class VRP_PP_PROBLEM *_PROB);

    // this is the standard destructor. It releases all dynamically reserved memory when executed
    ~VRP_PP_OSM_TWIN();

    // print the currently stored OSM-data on the console output
    void print(class VRP_PP_PROBLEM *_PROB);

    // print the currently stored OSM-data of the twin nodes on the console output
    void print_nodes(class VRP_PP_PROBLEM *_PROB);

    // print the currently stored OSM-data of the twin arcs on the console output
    void print_arcs(class VRP_PP_PROBLEM *_PROB);

    // print the currently stored OSM-data of the twin arcs incl. path on the console output
    void print_arcs_paths(class VRP_PP_PROBLEM *_PROB);

    // used to specify the specification of the database connection and access previleges
    void configure_connection(char *_HOSTNAME, char *_PORT, char *_DB_NAME, char *_DB_USER, char *_DB_PASSWORD);

    // open the connection to the OSM database
    void establish_connection(void);

    // close the connection to the OSM datanase
    void close_connection(void);

    // returns a struct object of the _ID-th stored array from the OSM_NODE-node array
    struct VRP_PP_OSM_NODE get_osm_node_by_id(long _ID);

    // returns a struct object of the _ID-th stored array from the ARC_NODE-node array
    // the waypoint sequence is not returned
    struct VRP_PP_OSM_ARC get_osm_arc_by_id(long _ID);

    // call this to find all OSM-data associated with the nodes in the problem under consideration
    void sync_nodes(class VRP_PP_PROBLEM *_PROB);

    // call this to find all OSM-data associated with the node with id _NODE_ID in the problem under consideration
    void sync_node(int _NODE_ID, class VRP_PP_PROBLEM *_PROB);

    // call this to find all OSM-data associated with the arcs in the problem under consideration
    // waypoint sequences are not updated or searched
    void sync_arcs(class VRP_PP_PROBLEM *_PROB);

    // call this to find all OSM-data associated with the arc with id _ARC_ID in the problem under consideration
    // waypoint sequences are not updated or searched
    void sync_arc(int _ARC_ID, class VRP_PP_PROBLEM *_PROB);

    // call this to find all OSM-data associated with the arcs in the problem under consideration
    // waypoint sequences are defined (if existing)
    void sync_arcs_paths(class VRP_PP_PROBLEM *_PROB);

    // call this to find all OSM-data associated with the arc with id _ARC_ID in the problem under consideration
    // waypoint sequence is defined (if existing)
    void sync_arc_path(int _ARC_ID, class VRP_PP_PROBLEM *_PROB);

    //change bool
    // set the attribute USE_DIRECTED_GRAPH
    void set_directed(bool _ARG);

    // set the attribute SILENT_MODE
    void set_silent(bool _ARG);
};
#else
#pragma message "vrp++ osm components are not considered"
#endif // VRP_PP_OSM
#endif // VRP_PP_OSM_H_INCLUDED
