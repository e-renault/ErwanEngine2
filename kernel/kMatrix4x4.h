#ifndef M_KMATRIX3X3_H_
#define M_KMATRIX3X3_H_

#include "../kernel/header.h"
#include "../kernel/kVector3.h"

//TODO: paralellize
Vector3 multiply(Matrix3 m, Vector3 v) {
    Vector3 ret;

    Vector3 x = *((EE_FLOAT3*) &(m.mat)) *v;
    ret.x = sum(x);

    Vector3 y = *((EE_FLOAT3*) &(m.mat) + 1) *v;
    ret.y = sum(y);

    Vector3 z = *((EE_FLOAT3*) &(m.mat) + 2) *v;
    ret.z = sum(z);

    return ret;
}

//TODO: paralellize
Vector3 rotateAround(Vector3 v, Vector3 unit, EE_FLOAT tetha) {
    Vector3 ret;
    float costetha = cos(tetha);

    Vector3 p1 = costetha * v;
    Vector3 p2 = ((1-costetha) * dotProduct(v, unit)) * unit;
    Vector3 p3 = sin(tetha)* crossProduct(unit, v);
    
    //add up all
    ret = p1 + p2 + p3;
    
    return ret;
}

#endif // M_KMATRIX3X3_H_
