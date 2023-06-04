#ifndef M_MATRIX3_H_
#define M_MATRIX3_H_

#include "../kernel/header.h"
#include "mVector3.h"


//TODO: paralellize
Matrix3 invert(Matrix3 m) {
    Matrix3 ret;

    // Compute adjoints
    ret.mat.s0 = + m.mat.s5 * m.mat.sA - m.mat.s6 * m.mat.s9;
    ret.mat.s1 = - m.mat.s1 * m.mat.sA + m.mat.s2 * m.mat.s9;
    ret.mat.s2 = + m.mat.s1 * m.mat.s6 - m.mat.s2 * m.mat.s5;
    ret.mat.s4 = - m.mat.s4 * m.mat.sA + m.mat.s6 * m.mat.s8;
    ret.mat.s5 = + m.mat.s0 * m.mat.sA - m.mat.s2 * m.mat.s8;
    ret.mat.s6 = - m.mat.s0 * m.mat.s6 + m.mat.s2 * m.mat.s4;
    ret.mat.s8 = + m.mat.s4 * m.mat.s9 - m.mat.s5 * m.mat.s8;
    ret.mat.s9 = - m.mat.s0 * m.mat.s9 + m.mat.s1 * m.mat.s8;
    ret.mat.sA = + m.mat.s0 * m.mat.s5 - m.mat.s1 * m.mat.s4;

    // Compute determinant
    float det;
    det = m.mat.s0 * ret.mat.s0 + m.mat.s1 * ret.mat.s4 + m.mat.s2 * ret.mat.s8;

    // anti zero detection
    if (det == 0) det = FLT_MIN;


    det = 1.0f / det;

    ret.mat.s0 *= det;
    ret.mat.s1 *= det;
    ret.mat.s2 *= det;
    ret.mat.s4 *= det;
    ret.mat.s5 *= det;
    ret.mat.s6 *= det;
    ret.mat.s8 *= det;
    ret.mat.s9 *= det;
    ret.mat.sA *= det;

    return ret;
}

//TODO: paralellize
Vector3 multiply(Matrix3 m, Vector3 v) {
    Vector3 ret;

    ret.x = m.mat.s0 * v.x
            + m.mat.s1 * v.y
            + m.mat.s2 * v.z;

    ret.y = m.mat.s4 * v.x
            + m.mat.s5 * v.y
            + m.mat.s6 * v.z;

    ret.z = m.mat.s8 * v.x
            + m.mat.s9 * v.y
            + m.mat.sA * v.z;

    return ret;
}

//TODO: paralellize
Vector3 rotateAround(Vector3 v, Vector3 unit, EE_FLOAT tetha) {
    Vector3 ret;
    float costetha = cos(tetha);

    Vector3 p1 = scale_vector3(costetha, v);
    Vector3 p2 = scale_vector3((1-costetha) * dotProduct(v, unit), unit);
    Vector3 p3 = scale_vector3(sin(tetha), crossProduct(unit, v));
    
    //add up all
    ret = add_vector3(add_vector3(p1,p2), p3);
    
    return ret;
}



#endif // M_MATRIX3_H_
