#include "vrp_pp_global.h"
#include "vrp_pp_lp.h"

VRP_PP_indexset2D::VRP_PP_indexset2D(int _i_size, int _j_size, int _offset)
{
    this->i_size = _i_size;
    this->j_size = _j_size;
    this->offset = _offset;
}

VRP_PP_indexset2D::~VRP_PP_indexset2D()
{

}

int VRP_PP_indexset2D::idx2int(struct VRP_PP_vector2D _Vec)
{
    return( this->offset + _Vec.i * this->j_size + _Vec.j );
}

int VRP_PP_indexset2D::idx2int(int _i, int _j)
{
    return( this->offset + _i * this->j_size + _j );
}

struct VRP_PP_vector2D VRP_PP_indexset2D::int2idx(int _I)
{
    struct VRP_PP_vector2D VEC;

    VEC.i = (_I - this->offset) / (this->j_size);
    VEC.j = (_I - this->offset) - VEC.i*this->j_size ;

    return(VEC);
};

int VRP_PP_indexset2D::get_i_size(void)
{
    return( this->i_size );
}

int VRP_PP_indexset2D::get_j_size(void)
{
    return( this->j_size );
}

int VRP_PP_indexset2D::get_offset(void)
{
    return( this->offset );
}

VRP_PP_indexset1D::VRP_PP_indexset1D(int _i_size, int _offset)
{
    this->i_size = _i_size;
    this->offset = _offset;
}

VRP_PP_indexset1D::~VRP_PP_indexset1D()
{

}

int VRP_PP_indexset1D::idx2int(int _i)
{
    return( this->offset + _i );
}

int VRP_PP_indexset1D::int2idx(int _i)
{
    return( _i - this->offset );
}

int VRP_PP_indexset1D::get_i_size(void)
{
    return( this->i_size );
}

int VRP_PP_indexset1D::get_offset(void)
{
    return( this->offset );
}

VRP_PP_indexset3D::VRP_PP_indexset3D(int _i_size, int _j_size, int _k_size, int _offset)
{
    this->i_size = _i_size;
    this->j_size = _j_size;
    this->k_size = _k_size;
    this->offset = _offset;
}

VRP_PP_indexset3D::~VRP_PP_indexset3D()
{

}

int VRP_PP_indexset3D::idx2int(struct VRP_PP_vector3D _Vec)
{
    return( this->offset + _Vec.k * (this->i_size * this->j_size) + _Vec.i * (this->j_size) + _Vec.j );
}

int VRP_PP_indexset3D::idx2int(int _i, int _j, int _k)
{
    return( this->offset + _k * (this->i_size * this->j_size) + _i * (this->j_size) + _j );
}

struct VRP_PP_vector3D VRP_PP_indexset3D::int2idx(int _I)
{
    struct VRP_PP_vector3D VEC;

    VEC.k = (_I - this->offset) / (this->i_size * this->j_size);
    VEC.i = ((_I - this->offset) - VEC.k * (this->i_size * this->j_size)) / this->j_size;
    VEC.j = (_I - this->offset) - VEC.k * (this->i_size * this->j_size) - VEC.i * this->j_size;

    return(VEC);
};

int VRP_PP_indexset3D::get_i_size(void)
{
    return( this->i_size );
}

int VRP_PP_indexset3D::get_j_size(void)
{
    return( this->j_size );
}

int VRP_PP_indexset3D::get_k_size(void)
{
    return( this->k_size );
}

int VRP_PP_indexset3D::get_offset(void)
{
    return( this->offset );
}
