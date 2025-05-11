#ifndef VRP_PP_LP_H_INCLUDED
#define VRP_PP_LP_H_INCLUDED

// this header file contains those definitions needed to connect vrp_pp-objects with an lp-solver

// version 3.0 - 12.03.2021 at 12:40
// -- introduction of the class-object VRP_PP_indexset3D
// -- introduction of the class-object VRP_PP_indexset2D
// -- introduction of the class-object VRP_PP_indexset1D

class VRP_PP_indexset3D {
    int i_size = 0;         // number of elements in x-direction (1st dimension)
    int j_size = 0;         // number of elements in y-direction (2nd dimension)
    int k_size = 0;         // number of elements in z-direction (3rd dimension)
    int offset = 0;         // where the numbering starts if other index-array are also used

public:
    VRP_PP_indexset3D(int _i_size, int _j_size, int _k_size, int _offset);    // the constructor
    int idx2int(struct VRP_PP_vector3D _Vec);       // converts the (i;j;k)-index to a linear index
    int idx2int(int _i, int _j, int _k);            // converts the (i;j;k)-index to a linear index
    struct VRP_PP_vector3D int2idx(int _I);         // converts the linear index to the (i;j;k)-index;
    int get_i_size(void);
    int get_j_size(void);
    int get_k_size(void);
    int get_offset(void);
    ~VRP_PP_indexset3D();
};

class VRP_PP_indexset2D {
    int i_size = 0;         // number of elements in x-direction (1st dimension)
    int j_size = 0;         // number of elements in y-direction (2nd dimension)
    int offset = 0;         // where the numbering starts if other index-array are also used

public:
    VRP_PP_indexset2D(int _i_size, int _j_size, int _offset);    // the constructor
    int idx2int(struct VRP_PP_vector2D _Vec);       // converts the (i;j)-index to a linear index
    int idx2int(int _i, int _j);                    // converts the (i;j)-index to a linear index
    struct VRP_PP_vector2D int2idx(int _I);         // converts the linear index to the (i;j)-index;
    int get_i_size(void);
    int get_j_size(void);
    int get_offset(void);
    ~VRP_PP_indexset2D();
};

class VRP_PP_indexset1D {
    int i_size = 0;         // number of elements in x-direction (1st dimension)
    int offset = 0;         // where the numbering starts if other index-array are also used

public:
    VRP_PP_indexset1D(int _i_size, int _offset);    // the constructor
    int idx2int(int _i);    // converts dv index into pos in model DV-vector
    int int2idx(int _i);    // converts model dv-vector pos into dv index
    int get_i_size(void);
    int get_offset(void);
    ~VRP_PP_indexset1D();
};

struct VRP_PP_vector2D {
    int i;
    int j;
};

struct VRP_PP_vector3D {
    int i;
    int j;
    int k;
};


#endif // VRP_PP_LP_H_INCLUDED
