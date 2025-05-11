#include <iostream>
#include <iomanip>

#include "vrp_pp_problem.h"
#include "vrp_pp_extern.h"

#include "vrp_pp_osm.h"
//ADD OSM
#define VRP_PP_OSM
#ifdef VRP_PP_OSM

    #pragma message "vrp++ osm components are considered"
    #include "libpq-fe.h"

VRP_PP_OSM_TWIN::VRP_PP_OSM_TWIN(class VRP_PP_PROBLEM *_PROB)
{
    #ifdef VRP_PP_OSM
    #pragma message "OSM interface of VRP++ is considered"
    // this is the constructor of the osm twin

    // init the db-conn-handler
    this->DBCON = nullptr;

    // init the db-conn-parameters
    sprintf(this->HOSTNAME,"%s","");
    sprintf(this->DB_PASSWORD,"%s","");
    sprintf(this->DB_USER,"%s","");
    sprintf(this->PORT,"%s","");
    sprintf(this->DB_NAME,"%s","");

    // determine if the underlying graph is assumed to be directed
    this->USE_DIRECTED_GRAPH = false;

    // define if the silent mode is active (no screen display)
    this->SILENT_MODE = true;

    // determine the number of network nodes
    this->NODES = _PROB->get_node_ctr();

    // if necessary, allocate the memory for the twin nodes and initialize them
    if( this->NODES > 0)
    {
        this->OSM_NODE = new struct VRP_PP_OSM_NODE [this->NODES];
        for(int i=0 ; i < this->NODES ; i++)
        {
            this->OSM_NODE[i].ID = i;
            this->OSM_NODE[i].OSM_ID = -1;
            this->OSM_NODE[i].DB_ID = -1;
            this->OSM_NODE[i].OSM_LON = 0.0;
            this->OSM_NODE[i].OSM_LAT = 0.0;
        }
    }
    else
    {
        this->OSM_NODE = nullptr;
    }

    // determine the number of network arcs
    this->ARCS = _PROB->get_arc_ctr();

    // if necessary, allocate the memory for the twin arcs and initialize them
    if( this->ARCS > 0)
    {
        this->OSM_ARC = new struct VRP_PP_OSM_ARC [this->ARCS];
        for(int i=0 ; i < this->ARCS ; i++)
        {
            this->OSM_ARC[i].ID = i;
            this->OSM_ARC[i].DB_ID = -1;
            this->OSM_ARC[i].OSM_ID = -1;
            this->OSM_ARC[i].OSM_LENGTH = 0.0;
            this->OSM_ARC[i].OSM_MAXSPEED = 0.0;
            this->OSM_ARC[i].ORIGIN_DB_ID = -1;
            this->OSM_ARC[i].ORIGIN_OSM_ID = -1;
            this->OSM_ARC[i].DEST_DB_ID = -1;
            this->OSM_ARC[i].DEST_OSM_ID = -1;
            this->OSM_ARC[i].FIRST_WAYPOINT = nullptr;
            this->OSM_ARC[i].LAST_WAYPOINT = nullptr;
        }
    }
    else
    {
        this->OSM_ARC = nullptr;
    }
    #else
        #pragma message "we do not consider the OSM extension components of VRP++"
        std::cout << "you have tried to use the OSM-interface of VRP++" << std::endl;
        std::cout << "please uncomment the line #define VRP_PP_OSM in vrp_pp_extern.h" << std::endl;
    #endif
}

VRP_PP_OSM_TWIN::~VRP_PP_OSM_TWIN()
{
    if( this->NODES > 0 )
        delete[] this->OSM_NODE;

    // before we delete the arc object we have to empty the list of waypoints
    for( int a=0 ; a < this->ARCS ; a++ )
    {
        this->OSM_ARC[a].remove_waypoints();
    }

    if( this->ARCS > 0 )
        delete[] this->OSM_ARC;
}

void VRP_PP_OSM_ARC::remove_waypoints(void)
{
    struct VRP_PP_OSM_WAYPOINT *Ptr;
    Ptr = this->FIRST_WAYPOINT;
    while( Ptr != nullptr )
    {
        if( this->FIRST_WAYPOINT != this->LAST_WAYPOINT )
        {
            // >= 2 waypoints in the list, remove and delete the first one
            this->FIRST_WAYPOINT = Ptr->next;
            this->FIRST_WAYPOINT->prev = nullptr;
            Ptr->next = nullptr;
            delete Ptr;
        }
        else
        {
            // =1 waypoint is in the list
            this->FIRST_WAYPOINT = nullptr;
            this->LAST_WAYPOINT = nullptr;
            delete Ptr;
        }
        Ptr = this->FIRST_WAYPOINT;
    }
}

void VRP_PP_OSM_TWIN::print(class VRP_PP_PROBLEM *_PROB)
{
    this->print_nodes(_PROB);
    this->print_arcs_paths(_PROB);
}

void VRP_PP_OSM_TWIN::print_arcs(class VRP_PP_PROBLEM *_PROB)
{
    std::cout << "OSM-TWIN-ARCS" << std::endl;
    struct VRP_PP_ARC ArcCopy;
    for( int i=0 ; i < this->ARCS ; i++ )
    {
        std::cout << this->OSM_ARC[i].ID;
        ArcCopy = _PROB->get_arc_by_index(this->OSM_ARC[i].ID);
        std::cout << ": " << ArcCopy.origin << "->" << ArcCopy.dest << " (";
        std::cout << "length=" << this->OSM_ARC[i].OSM_LENGTH << " km) /";
        std::cout << "DB-Nodes: " << this->OSM_ARC[i].ORIGIN_DB_ID << "->" << this->OSM_ARC[i].DEST_DB_ID << std::endl;
        if( this->OSM_ARC[i].FIRST_WAYPOINT != nullptr )
        {
            std::cout << "waypoint sequence stored" << std::endl;
        }
        else
        {
            std::cout << "no way/route stored" << std::endl;
        }
        //std::cout << std::endl;
    }
}

void VRP_PP_OSM_TWIN::print_arcs_paths(class VRP_PP_PROBLEM *_PROB)
{
    std::cout << "OSM-TWIN-ARCS" << std::endl;
    struct VRP_PP_ARC ArcCopy;
    for( int i=0 ; i < this->ARCS ; i++ )
    {
        std::cout << this->OSM_ARC[i].ID;
        ArcCopy = _PROB->get_arc_by_index(this->OSM_ARC[i].ID);
        std::cout << ": " << ArcCopy.origin << "->" << ArcCopy.dest << " (";
        std::cout << "length=" << this->OSM_ARC[i].OSM_LENGTH << " km) /";
        std::cout << "DB-Nodes: " << this->OSM_ARC[i].ORIGIN_DB_ID << "->" << this->OSM_ARC[i].DEST_DB_ID << std::endl;
        if( this->OSM_ARC[i].FIRST_WAYPOINT != nullptr )
        {
            struct VRP_PP_OSM_WAYPOINT *TmpPtr;
            TmpPtr = this->OSM_ARC[i].FIRST_WAYPOINT;
            while( TmpPtr != nullptr )
            {
                TmpPtr->print();
                TmpPtr = TmpPtr->next;
            }
        }
        else
        {
            std::cout << "no way/route stored" << std::endl;
        }
        //std::cout << std::endl;
    }
}

void VRP_PP_OSM_TWIN::print_nodes(class VRP_PP_PROBLEM *_PROB)
{
    std::cout << "OSM-TWIN-NODES" << std::endl;
    std::cout << "\t" << "ID";
    std::cout << "\t" << "DB_ID";
    std::cout << "\t" << "OSM_ID";
    std::cout << "\t" << "OSM_LON";
    std::cout << "\t" << "OSM_LAT";
    std::cout << std::endl;
    for( int i=0 ; i < this->NODES ; i++ )
    {
        std::cout << "\t" << this->OSM_NODE[i].ID;
        std::cout << "\t" << this->OSM_NODE[i].DB_ID;
        std::cout << "\t" << this->OSM_NODE[i].OSM_ID;
        std::cout << "\t" << this->OSM_NODE[i].OSM_LON;
        std::cout << "\t" << this->OSM_NODE[i].OSM_LAT;
        std::cout << std::endl;
    }
}

void VRP_PP_OSM_TWIN::establish_connection(void)
{

    this->DBCON = PQsetdbLogin(this->HOSTNAME,
                     this->PORT,
                     nullptr,
                     nullptr,
                     this->DB_NAME,
                     this->DB_USER,
                     this->DB_PASSWORD);

    switch(PQstatus(this->DBCON))
    {
        case CONNECTION_BAD:
            if( this->SILENT_MODE == false )
            {
                std::cout << "not Connected..." << std::endl;
                std::cout << PQerrorMessage(this->DBCON) << std::endl;
            }
            break;

        case CONNECTION_MADE:
            if( this->SILENT_MODE == false )
            {
                std::cout << "Connected to server..." << std::endl;
                std::cout << PQerrorMessage(this->DBCON) << std::endl;
            }
            break;
   }

   if( PQstatus(this->DBCON) != CONNECTION_BAD )
   {
        if( this->SILENT_MODE == false )
        {
            std::cout << "Server Version " << PQserverVersion(this->DBCON) << std::endl;
        }
       //PQfinish(this->DBCON);
   }
}

void VRP_PP_OSM_TWIN::close_connection(void)
{
    if( PQstatus(this->DBCON) != CONNECTION_BAD )
    {
       PQfinish(this->DBCON);
    }
}

void VRP_PP_OSM_TWIN::sync_nodes(class VRP_PP_PROBLEM *_PROB)
{
    int NODESNUM;
    NODESNUM = _PROB->get_node_ctr();

    for( int n=0 ; n < NODESNUM ; n++ )
    {
        this->sync_node(n,_PROB);
    }
}

void VRP_PP_OSM_TWIN::sync_arcs(class VRP_PP_PROBLEM *_PROB)
{
    int ARCNUM;
    ARCNUM = _PROB->get_arc_ctr();

    for( int a=0 ; a < ARCNUM ; a++ )
    {
        this->sync_arc(a,_PROB);
    }
}

void VRP_PP_OSM_TWIN::sync_arcs_paths(class VRP_PP_PROBLEM *_PROB)
{
    int ARCNUM;
    ARCNUM = _PROB->get_arc_ctr();

    for( int a=0 ; a < ARCNUM ; a++ )
    {
        this->sync_arc_path(a,_PROB);
    }
}

void VRP_PP_OSM_TWIN::sync_arc(int _ARC_ID, class VRP_PP_PROBLEM *_PROB)
{
    if( PQstatus(this->DBCON) != CONNECTION_BAD )
    {
        // fetch the arc object
        struct VRP_PP_ARC TmpArc;
        TmpArc = _PROB->get_arc_by_index(_ARC_ID);
        if( this->SILENT_MODE == false )
        {
            std::cout << "ARC=" << _ARC_ID << std::endl;
            std::cout << "ARC-ID=" << TmpArc.id << std::endl;
            std::cout << "ARC-origin=" << TmpArc.origin << std::endl;
            std::cout << "ARC-dest=" << TmpArc.dest << std::endl;
        }

        // get the local ids
        long source_db_id;
        long target_db_id;
        source_db_id = this->OSM_NODE[TmpArc.origin].DB_ID;
        target_db_id = this->OSM_NODE[TmpArc.dest].DB_ID;

        // consolidate the SQL-statement
        char SQLSTR[256];
        if( this->USE_DIRECTED_GRAPH )
            //sprintf(SQLSTR,"SELECT * FROM pgr_dijkstra('SELECT gid as id, source, target, length_m AS cost, length_m AS reverse_cost FROM ways',%d,%d,true)",source_db_id,target_db_id );
            //sprintf(SQLSTR,"SELECT * FROM pgr_dijkstra('SELECT gid as id, source, target, length_m AS cost, length_m As reverse_costs FROM ways',%d,%d,true)",source_db_id,target_db_id );
            sprintf(SQLSTR,"SELECT * FROM pgr_dijkstra('SELECT gid as id, source, target, length_m AS cost, reverse_length_m AS reverse_cost FROM ways',%d,%d,true)",source_db_id,target_db_id );

        else
            sprintf(SQLSTR,"SELECT * FROM pgr_dijkstra('SELECT gid as id, source, target, length_m AS cost, reverse_length_m As reverse_cost FROM ways',%d,%d,false)",source_db_id,target_db_id );
        if( this->SILENT_MODE == false )
        {
            std::cout << "SQLSTR=" << SQLSTR << std::endl;
        }

        // prepare and execute the query
        PGresult *MyResult;
        MyResult = PQexec(this->DBCON,SQLSTR);

        if( this->SILENT_MODE == false )
        {
            std::cout << "tuples found = " << PQntuples(MyResult) << std::endl;
        }

        // save everything
        if( PQntuples(MyResult) > 0 )
        {
            //std::cout << "ABC" << std::endl;
            this->OSM_ARC[_ARC_ID].DB_ID = atoi(PQgetvalue(MyResult,0,0));
            this->OSM_ARC[_ARC_ID].OSM_ID = atoi(PQgetvalue(MyResult,0,1));

            this->OSM_ARC[_ARC_ID].ORIGIN_DB_ID = this->OSM_NODE[TmpArc.origin].DB_ID;
            this->OSM_ARC[_ARC_ID].ORIGIN_OSM_ID = this->OSM_NODE[TmpArc.origin].OSM_ID;

            this->OSM_ARC[_ARC_ID].DEST_DB_ID = this->OSM_NODE[TmpArc.dest].DB_ID;
            this->OSM_ARC[_ARC_ID].DEST_OSM_ID = this->OSM_NODE[TmpArc.dest].OSM_ID;

            //Thuy edits column number
            this->OSM_ARC[_ARC_ID].OSM_LENGTH = atof(PQgetvalue(MyResult,PQntuples(MyResult)-1,7)) / 1000.0;

            /*
            int NumWayPoints = PQntuples(MyResult);
            for( int j=0 ; j < NumWayPoints ; j++ )
            {
                struct VRP_PP_OSM_WAYPOINT *PtrWP;
                PtrWP = new struct VRP_PP_OSM_WAYPOINT;
                PtrWP->prev = nullptr;
                PtrWP->next = nullptr;
                //std::cout << "Way Point " << j;
                //std::cout << "\t seq=" << atoi(PQgetvalue(MyResult,j,0));
                PtrWP->SEQ = atoi(PQgetvalue(MyResult,j,0));
                //std::cout << "\t path-seq=" << atoi(PQgetvalue(MyResult,j,1));
                PtrWP->SEQ_ID = atoi(PQgetvalue(MyResult,j,1));
                //std::cout << "\t node DB-ID= " << atol(PQgetvalue(MyResult,j,2));
                PtrWP->DB_NODE_ID = atol(PQgetvalue(MyResult,j,2));
                //std::cout << "\t node enter edge= " << atol(PQgetvalue(MyResult,j,3));
                PtrWP->DB_EDGE_ID = atol(PQgetvalue(MyResult,j,3));
                //std::cout << "\t add costs= " << atof(PQgetvalue(MyResult,j,4));
                PtrWP->ADD_COSTS = atof(PQgetvalue(MyResult,j,4));
                //std::cout << "\t aggreated costs= " << atof(PQgetvalue(MyResult,j,5)) << std::endl;
                PtrWP->AGG_COSTS = atof(PQgetvalue(MyResult,j,5));
                // append the new way point object to the path of this arc

                PtrWP->sync_lonlat(this);

                this->OSM_ARC[_ARC_ID].append_waypoint(PtrWP);
            }
            */
            PQclear(MyResult);
        }
        else
        {
            // no map path found - store the direct connection
            this->OSM_ARC[_ARC_ID].DB_ID = -1;;
            this->OSM_ARC[_ARC_ID].OSM_ID = -1;

            this->OSM_ARC[_ARC_ID].ORIGIN_DB_ID = this->OSM_NODE[TmpArc.origin].DB_ID;
            this->OSM_ARC[_ARC_ID].ORIGIN_OSM_ID = this->OSM_NODE[TmpArc.origin].OSM_ID;

            this->OSM_ARC[_ARC_ID].DEST_DB_ID = this->OSM_NODE[TmpArc.dest].DB_ID;
            this->OSM_ARC[_ARC_ID].DEST_OSM_ID = this->OSM_NODE[TmpArc.dest].OSM_ID;

            struct VRP_PP_NODE Origin, Destination;
            Origin = _PROB->get_node_by_index(TmpArc.origin);
            Destination = _PROB->get_node_by_index(TmpArc.dest);

            this->OSM_ARC[_ARC_ID].OSM_LENGTH = -VRP_PP_BIGM; //-dist_lattlong(Origin, Destination);
        }
    }
}

void VRP_PP_OSM_TWIN::sync_node(int _NODE_ID, class VRP_PP_PROBLEM *_PROB)
{
    if( PQstatus(this->DBCON) != CONNECTION_BAD )
    {
        // fetch the node object
        struct VRP_PP_NODE TmpNode;
        //std::cout << "NODE=" << _NODE_ID << std::endl;
        TmpNode = _PROB->get_node_by_index(_NODE_ID);
        //std::cout << "NODE-ID=" << TmpNode.id << std::endl;

        // consolidate the SQL-statement
        char SQLSTR[256];
        sprintf(SQLSTR,"SELECT id,osm_id,lon,lat FROM ways_vertices_pgr ORDER BY the_geom <-> st_setsrid(st_makepoint(%f, %f), 4326) LIMIT 1",TmpNode.longitude,TmpNode.lattitude );

        // std::cout << "SQLSTR=" << SQLSTR << std::endl;
        // prepare and execute the query
        PGresult *MyResult;
        MyResult = PQexec(this->DBCON,SQLSTR);

        // save everything
        if( PQntuples(MyResult) > 0 )
        {
            this->OSM_NODE[_NODE_ID].DB_ID = atoi(PQgetvalue(MyResult,0,0));
            this->OSM_NODE[_NODE_ID].OSM_ID = atoi(PQgetvalue(MyResult,0,1));
            this->OSM_NODE[_NODE_ID].OSM_LON = atof(PQgetvalue(MyResult,0,2));
            this->OSM_NODE[_NODE_ID].OSM_LAT = atof(PQgetvalue(MyResult,0,3));
            //std::cout << "STORED DB_ID=" << this->OSM_NODE[_NODE_ID].DB_ID << std::endl;
            //std::cout << "STORED OSM_ID=" << this->OSM_NODE[_NODE_ID].OSM_ID << std::endl;
            //std::cout << "STORED LON=" << this->OSM_NODE[_NODE_ID].OSM_LON << std::endl;
            //std::cout << "STORED LAT=" << this->OSM_NODE[_NODE_ID].OSM_LAT << std::endl;
            PQclear(MyResult);
        }
    }

}

void VRP_PP_OSM_ARC::append_waypoint(struct VRP_PP_OSM_WAYPOINT *Ptr)
{
    if( this->FIRST_WAYPOINT != nullptr )
    {
        // already stored waypoints
        Ptr->prev = this->LAST_WAYPOINT;
        this->LAST_WAYPOINT->next = Ptr;
        this->LAST_WAYPOINT = Ptr;
    }
    else
    {
        // the first waypoints
        this->FIRST_WAYPOINT = Ptr;
        this->LAST_WAYPOINT = Ptr;
    }
}

void VRP_PP_OSM_WAYPOINT::print(void)
{
    std::cout << this->SEQ;
    std::cout << "\t" << this->SEQ_ID;
    std::cout << "\t DB_ID_node=" << this->DB_NODE_ID;
    std::cout << "\t lon=" << this->LON;
    std::cout << "\t lat=" << this->LAT;
    if( this->SYNCHRONIZED )
        std::cout << "\t sync=ok";
    else
        std::cout << "\t sync=no";
    std::cout << "\t aggr. costs=" << this->AGG_COSTS;
    std::cout << std::endl;
}

void VRP_PP_OSM_WAYPOINT::sync_lonlat(class VRP_PP_OSM_TWIN *_TWIN)
{
    if( !_TWIN->SILENT_MODE )
        std::cout << "try to synchronize waypoint with DB_ID=" << this->DB_NODE_ID << std::endl;

    if( PQstatus(_TWIN->DBCON) != CONNECTION_BAD )
    {
        // consolidate the SQL-statement
        char SQLSTR[256];
        sprintf(SQLSTR,"SELECT id,osm_id,lon,lat FROM ways_vertices_pgr WHERE id=%d",this->DB_NODE_ID );

        if( !_TWIN->SILENT_MODE )
            std::cout << "SQLSTR=" << SQLSTR << std::endl;

        PGresult *MyResult;
        MyResult = PQexec(_TWIN->DBCON,SQLSTR);

        // save everything
        if( !_TWIN->SILENT_MODE )
            std::cout << "identified rows=" << PQntuples(MyResult) << std::endl;

        if( PQntuples(MyResult) > 0 )
        {
            this->LON = atof(PQgetvalue(MyResult,0,2));
            this->LAT = atof(PQgetvalue(MyResult,0,3));
            this->SYNCHRONIZED = true;
        }
    }
    if( !_TWIN->SILENT_MODE )
        std::cout << "end of synchronization" << std::endl;
}

void VRP_PP_OSM_TWIN::configure_connection(char *_HOSTNAME, char *_PORT, char *_DB_NAME, char *_DB_USER, char *_DB_PASSWORD)
{
    // set the db-conn-parameters
    sprintf(this->HOSTNAME,"%s",_HOSTNAME);
    sprintf(this->DB_PASSWORD,"%s",_DB_PASSWORD);
    sprintf(this->DB_USER,"%s",_DB_USER);
    sprintf(this->PORT,"%s",_PORT);
    sprintf(this->DB_NAME,"%s",_DB_NAME);
}

void VRP_PP_OSM_TWIN::sync_arc_path(int _ARC_ID, class VRP_PP_PROBLEM *_PROB)
{
    if( PQstatus(this->DBCON) != CONNECTION_BAD )
    {
        // fetch the arc object
        struct VRP_PP_ARC TmpArc;
        TmpArc = _PROB->get_arc_by_index(_ARC_ID);
        if( this->SILENT_MODE == false )
        {
            std::cout << "ARC=" << _ARC_ID << std::endl;
            std::cout << "ARC-ID=" << TmpArc.id << std::endl;
            std::cout << "ARC-origin=" << TmpArc.origin << std::endl;
            std::cout << "ARC-dest=" << TmpArc.dest << std::endl;
        }

        // get the local ids
        long source_db_id;
        long target_db_id;
        source_db_id = this->OSM_NODE[TmpArc.origin].DB_ID;
        target_db_id = this->OSM_NODE[TmpArc.dest].DB_ID;

        // consolidate the SQL-statement
        char SQLSTR[256];
        if( this->USE_DIRECTED_GRAPH )
            sprintf(SQLSTR,"SELECT * FROM pgr_dijkstra('SELECT gid as id, source, target, length_m AS cost, reverse_length_m AS reverse_cost FROM ways',%d,%d,true)",source_db_id,target_db_id );
        else
            sprintf(SQLSTR,"SELECT * FROM pgr_dijkstra('SELECT gid as id, source, target, length_m AS cost, reverse_length_m AS reverse_cost FROM ways',%d,%d,false)",source_db_id,target_db_id );
        if( this->SILENT_MODE == false )
        {
            std::cout << "SQLSTR=" << SQLSTR << std::endl;
        }

        // prepare and execute the query
        PGresult *MyResult;
        MyResult = PQexec(this->DBCON,SQLSTR);

        if( this->SILENT_MODE == false )
        {
            std::cout << "tuples found = " << PQntuples(MyResult) << std::endl;
        }

        // save everything
        if( PQntuples(MyResult) > 0 )
        {
            //std::cout << "ABC" << std::endl;
            this->OSM_ARC[_ARC_ID].DB_ID = atoi(PQgetvalue(MyResult,0,0));
            this->OSM_ARC[_ARC_ID].OSM_ID = atoi(PQgetvalue(MyResult,0,1));

            this->OSM_ARC[_ARC_ID].ORIGIN_DB_ID = this->OSM_NODE[TmpArc.origin].DB_ID;
            this->OSM_ARC[_ARC_ID].ORIGIN_OSM_ID = this->OSM_NODE[TmpArc.origin].OSM_ID;

            this->OSM_ARC[_ARC_ID].DEST_DB_ID = this->OSM_NODE[TmpArc.dest].DB_ID;
            this->OSM_ARC[_ARC_ID].DEST_OSM_ID = this->OSM_NODE[TmpArc.dest].OSM_ID;

            //Thuy edits column number
            this->OSM_ARC[_ARC_ID].OSM_LENGTH = atof(PQgetvalue(MyResult,PQntuples(MyResult)-1,7)) / 1000.0;

            int NumWayPoints = PQntuples(MyResult);
            for( int j=0 ; j < NumWayPoints ; j++ )
            {
                struct VRP_PP_OSM_WAYPOINT *PtrWP;
                PtrWP = new struct VRP_PP_OSM_WAYPOINT;
                PtrWP->prev = nullptr;
                PtrWP->next = nullptr;
                //std::cout << "Way Point " << j;
                //std::cout << "\t seq=" << atoi(PQgetvalue(MyResult,j,0));
                PtrWP->SEQ = atoi(PQgetvalue(MyResult,j,0));
                //std::cout << "\t path-seq=" << atoi(PQgetvalue(MyResult,j,1));
                PtrWP->SEQ_ID = atoi(PQgetvalue(MyResult,j,1));
                //std::cout << "\t node DB-ID= " << atol(PQgetvalue(MyResult,j,2));
                PtrWP->DB_NODE_ID = atol(PQgetvalue(MyResult,j,2));
                //std::cout << "\t node enter edge= " << atol(PQgetvalue(MyResult,j,3));
                PtrWP->DB_EDGE_ID = atol(PQgetvalue(MyResult,j,3));
                //std::cout << "\t add costs= " << atof(PQgetvalue(MyResult,j,4));
                PtrWP->ADD_COSTS = atof(PQgetvalue(MyResult,j,4));
                //std::cout << "\t aggreated costs= " << atof(PQgetvalue(MyResult,j,5)) << std::endl;
                PtrWP->AGG_COSTS = atof(PQgetvalue(MyResult,j,5)) / 1000.0;
                // append the new way point object to the path of this arc

                PtrWP->sync_lonlat(this);

                this->OSM_ARC[_ARC_ID].append_waypoint(PtrWP);
            }

            PQclear(MyResult);
        }
        else
        {
            // no map path found - store the direct connection
            this->OSM_ARC[_ARC_ID].DB_ID = -1;;
            this->OSM_ARC[_ARC_ID].OSM_ID = -1;

            this->OSM_ARC[_ARC_ID].ORIGIN_DB_ID = this->OSM_NODE[TmpArc.origin].DB_ID;
            this->OSM_ARC[_ARC_ID].ORIGIN_OSM_ID = this->OSM_NODE[TmpArc.origin].OSM_ID;

            this->OSM_ARC[_ARC_ID].DEST_DB_ID = this->OSM_NODE[TmpArc.dest].DB_ID;
            this->OSM_ARC[_ARC_ID].DEST_OSM_ID = this->OSM_NODE[TmpArc.dest].OSM_ID;

            struct VRP_PP_NODE Origin, Destination;
            Origin = _PROB->get_node_by_index(TmpArc.origin);
            Destination = _PROB->get_node_by_index(TmpArc.dest);

            this->OSM_ARC[_ARC_ID].OSM_LENGTH = VRP_PP_BIGM; //-dist_lattlong(Origin, Destination);

            struct VRP_PP_OSM_WAYPOINT *OrigWP;
            OrigWP = new struct VRP_PP_OSM_WAYPOINT;
            OrigWP->prev = nullptr;
            OrigWP->next = nullptr;

            OrigWP->SEQ = 0;         // position in way sequence of arc
            OrigWP->SEQ_ID = 0;        // position in way sequence of arc
            OrigWP->DB_NODE_ID = this->OSM_NODE[TmpArc.origin].DB_ID;     // refers to the local db node id (in the osm-local-db)
            OrigWP->DB_EDGE_ID = -1 ;     // refers to the next edge to travers
            OrigWP->ADD_COSTS = 12345.67;     // cost increment by traversing DB_EDGE_ID
            OrigWP->AGG_COSTS = 12345.67;     // aggregated costs after having traversed DB_EDGE_ID

            OrigWP->sync_lonlat(this);
            this->OSM_ARC[_ARC_ID].append_waypoint(OrigWP);

            struct VRP_PP_OSM_WAYPOINT *DestWP;
            DestWP = new struct VRP_PP_OSM_WAYPOINT;
            DestWP->prev = nullptr;
            DestWP->next = nullptr;

            DestWP->SEQ = 0;         // position in way sequence of arc
            DestWP->SEQ_ID = 0;        // position in way sequence of arc
            DestWP->DB_NODE_ID = this->OSM_NODE[TmpArc.dest].DB_ID;     // refers to the local db node id (in the osm-local-db)
            DestWP->DB_EDGE_ID = -1 ;     // refers to the next edge to travers
            DestWP->ADD_COSTS = 12345.67;     // cost increment by traversing DB_EDGE_ID
            DestWP->AGG_COSTS = 12345.67;     // aggregated costs after having traversed DB_EDGE_ID

            DestWP->sync_lonlat(this);
            this->OSM_ARC[_ARC_ID].append_waypoint(DestWP);

        }
    }
}
//change bool
void VRP_PP_OSM_TWIN::set_directed(bool _ARG)
{
    this->USE_DIRECTED_GRAPH = _ARG;
}
//change bool
void VRP_PP_OSM_TWIN::set_silent(bool _ARG)
{
    this->SILENT_MODE = _ARG;
}

struct VRP_PP_OSM_NODE VRP_PP_OSM_TWIN::get_osm_node_by_id(long _ID)
{
    return(this->OSM_NODE[_ID]);
};

struct VRP_PP_OSM_ARC VRP_PP_OSM_TWIN::get_osm_arc_by_id(long _ID)
{
    return(this->OSM_ARC[_ID]);
};

#else
    #pragma message "vrp++ osm components are not considered"
#endif // VRP_PP_OSM
