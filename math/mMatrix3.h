#ifndef M_MATRIX3_H_
#define M_MATRIX3_H_

#ifndef FLT_MIN
  #include <float.h>
  #include <math.h>
  #ifdef __APPLE__
    #include <OpenCL/opencl.h>
  #else
    #include <CL/cl.h>
  #endif
#endif

#ifndef EE_FLOAT
  #ifndef FLT_MIN
    #define EE_FLOAT cl_float
    #define EE_INT cl_int
  #else
    #define EE_FLOAT float
    #define EE_INT int
  #endif
#endif

#include "mVector3.h"

typedef struct __attribute__ ((packed)) Matrix3 {
  EE_FLOAT val[3][3];
} Matrix3;

//TODO: paralellize
Matrix3 invert(Matrix3 m) {
    Matrix3 ret;

    // Compute adjoints
    ret.val[0][0] = + m.val[1][1] * m.val[2][2] - m.val[1][2] * m.val[2][1];
    ret.val[0][1] = - m.val[0][1] * m.val[2][2] + m.val[0][2] * m.val[2][1];
    ret.val[0][2] = + m.val[0][1] * m.val[1][2] - m.val[0][2] * m.val[1][1];
    ret.val[1][0] = - m.val[1][0] * m.val[2][2] + m.val[1][2] * m.val[2][0];
    ret.val[1][1] = + m.val[0][0] * m.val[2][2] - m.val[0][2] * m.val[2][0];
    ret.val[1][2] = - m.val[0][0] * m.val[1][2] + m.val[0][2] * m.val[1][0];
    ret.val[2][0] = + m.val[1][0] * m.val[2][1] - m.val[1][1] * m.val[2][0];
    ret.val[2][1] = - m.val[0][0] * m.val[2][1] + m.val[0][1] * m.val[2][0];
    ret.val[2][2] = + m.val[0][0] * m.val[1][1] - m.val[0][1] * m.val[1][0];

    // Compute determinant
    float det;
    det = m.val[0][0] * ret.val[0][0] + m.val[0][1] * ret.val[1][0] + m.val[0][2] * ret.val[2][0];

    // anti zero detection
    if (det == 0) det = FLT_MIN;


    det = 1.0f / det;

    ret.val[0][0] *= det;
    ret.val[0][1] *= det;
    ret.val[0][2] *= det;
    ret.val[1][0] *= det;
    ret.val[1][1] *= det;
    ret.val[1][2] *= det;
    ret.val[2][0] *= det;
    ret.val[2][1] *= det;
    ret.val[2][2] *= det;

    return ret;
}

//TODO: paralellize
Vector3 multiply(Matrix3 m, Vector3 v) {
    Vector3 ret;

    ret.x = m.val[0][0] * v.x
            + m.val[0][1] * v.y
            + m.val[0][2] * v.z;

    ret.y = m.val[1][0] * v.x
            + m.val[1][1] * v.y
            + m.val[1][2] * v.z;

    ret.z = m.val[2][0] * v.x
            + m.val[2][1] * v.y
            + m.val[2][2] * v.z;

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
