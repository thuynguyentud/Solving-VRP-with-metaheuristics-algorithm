#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <stdlib.h>
#include <ctime>
#include <cstdlib>
#include <string>
#include <iomanip>

#include "vrp_pp.h"
#include "vrp_pp_extern.h"

using namespace std;

int main(int argc, char** argv)
{
    int seed; int nbh_size; int P1; int P2;

    // Set the random seed
    srand(seed);

    // create a configuration object to store global configuration information
    struct VRP_PP_CONFIG *CONFIG;
    CONFIG = new struct VRP_PP_CONFIG;
    CONFIG->weigthType = VRP_PP_ARC_WEIGHT::CUSTOM;

    //code to specify parameter values on the command line
    for (int k = 0; k < argc; k++)
    {
        if (strcmp(argv[k], "-p") == 0)
        {
            k++;
            CONFIG->FILE = argv[k];

        } else if (strcmp(argv[k], "-seed") == 0)
        {
            k++;
            seed = std::atoi(argv[k]);

        } else if (strcmp(argv[k], "-nbh") == 0)
        {
            k++;
            nbh_size = std::atoi(argv[k]);

        } else if (strcmp(argv[k], "-p1") == 0)
        {
            k++;
            P1 = std::atoi(argv[k]);

        } else if (strcmp(argv[k], "-p2") == 0)
        {
            k++;
            P2 = std::atoi(argv[k]);
        }
    }

    // declare a pointer to a VRP++ problem project and construct the problem-object from the parameters stored in the xml-file
    class VRP_PP_PROBLEM *MyProb;
    MyProb = new class VRP_PP_PROBLEM(CONFIG->FILE);

    MyProb->read_fuel_consumption_values(CONFIG);

    //load 3 variables nbh_size, P1,P2 as attributes of VRP_PP_Problem
    MyProb->set_3_var(nbh_size, P1, P2);

    //create pointer to a plan object and construct plan-object
    class VRP_PP_PLAN *MyPlan ;
    MyPlan = new class VRP_PP_PLAN();



    //call the random plan generator - constructs a solution of the CVRP scenarios using a random construction procedure
    MyPlan->CVRP_generate_random_plan_capacitated(MyProb);
    MyPlan->evaluate(MyProb,CONFIG->weigthType);
    struct VRP_PP_EVALUATION MyEval;
    MyEval = MyPlan->get_evaluation();

    MyPlan->print_short(MyProb);
    std::cout << "\n the total fuel consumption of initial random solution: " << MyEval.obj_value;
    std::cout << "\n travel length initial random solution: " << MyEval.length << std::endl;


    //improve the generated plan using the simple ls-procedure
    MyPlan->simple_ls(MyProb,CONFIG->weigthType);
    MyPlan->evaluate(MyProb,CONFIG->weigthType);
    MyEval = MyPlan->get_evaluation();




    //print out performance indicators into different columns
    std::cout << std::setw(10) << "seed" << std::setw(10) << "nbh_size" << std::setw(10) << "P1" << std::setw(10) << "P2"
              << std::setw(10) << "O" << std::setw(10) << "N" << std::setw(10) << "B" << std::setw(10) << "P" << std::endl;

    // Print values in fixed-width columns
    std::cout << std::setw(10) << seed << std::setw(10) << nbh_size << std::setw(10) << P1 << std::setw(10) << P2
              << std::setw(10) << MyEval.obj_value << std::setw(10) << MyEval.used_veh_cntr << std::setw(10) << MyEval.veh_overload
              << std::setw(10) << MyEval.plt_overload << std::endl;


//     class VRP_PP_OSM_TWIN *MyOsmTwin;
//     MyOsmTwin = new class VRP_PP_OSM_TWIN(MyProb);
//
//     // setup the database connection
//     MyOsmTwin->configure_connection("localhost","5432","osm_database","osm_user","123");
//     // is every database operation reported to the screen? => no
//     MyOsmTwin->set_silent(false);
//     // we assume a directed graph
//     MyOsmTwin->set_directed(true);
//     MyOsmTwin->establish_connection();
//     MyOsmTwin->sync_nodes(MyProb);
//     MyOsmTwin->sync_arcs(MyProb);
//     MyOsmTwin->sync_arcs_paths(MyProb);
//
//        int N_ARCS;
//        struct VRP_PP_OSM_ARC TmpOsmArc;
//        struct VRP_PP_ARC TmpArc;
//        N_ARCS = MyProb->get_arc_ctr();
//        for( long i=0 ; i < N_ARCS ; i++ )
//        {
//        // retrieve a copy of the arc
//        TmpArc = MyProb->get_arc_by_index(i);
//        // retrieve a copy of the twin arc
//        TmpOsmArc = MyOsmTwin->get_osm_arc_by_id(i);
//        // modify the local copy of the arc
//        if( TmpArc.origin != TmpArc.dest )
//        {
//        TmpArc.length_custom = fabs(TmpOsmArc.OSM_LENGTH);
//        }
//        else
//        TmpArc.length_custom = VRP_PP_BIGM;
//        // update the arc in the problem object
//        MyProb->update_arc_data(i,TmpArc);
//        // update the distance matrices
//        MyProb->set_dist_custom(TmpArc.length_custom,TmpArc.origin,TmpArc.dest);
//        }
//
//        //Print arcs with real travel distance into xml files format
//        MyProb->print_custom_arclength_as_xml();

//        MyOsmTwin->close_connection();

//        //create a file handler to export solutions in a kml-file

//        ofstream FILE2;
//        FILE2.open("solution_map.kml");
//        MyPlan->print_as_kml_with_nodes(FILE2,MyProb);
//        FILE2.close();

    // free memory
    delete MyProb;

    return 0;
}
