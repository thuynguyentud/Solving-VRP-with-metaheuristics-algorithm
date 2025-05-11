#ifndef VRP_PP_EXTERN_H_INCLUDED
#define VRP_PP_EXTERN_H_INCLUDED

// we use this header file to control which external software is considered during
// compilation

// uncomment the next line if you want to consider lp_solve during compilation
//#define VRP_PP_SOLVER

// uncomment the next line if you want to consider the osm-application during compilation
#define VRP_PP_OSM

#endif // VRP_PP_EXTERN_H_INCLUDED
