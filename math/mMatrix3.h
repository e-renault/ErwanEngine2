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
Vector3 rotateX(Vector3 v, EE_FLOAT tetha) {
        Vector3 ret;

    ret.x = 1 * v.x
            + 0 * v.y
            + 0 * v.z;

    ret.y = 0 * v.x
            + cos(tetha) * v.y
            + -sin(tetha) * v.z;

    ret.z = 0 * v.x
            + sin(tetha) * v.y
            + cos(tetha) * v.z;

    return ret;
}

//TODO: paralellize
Vector3 rotateY(Vector3 v, EE_FLOAT tetha) {
        Vector3 ret;

    ret.x = cos(tetha) * v.x
            + 0 * v.y
            + sin(tetha) * v.z;

    ret.y = 0 * v.x
            + 1 * v.y
            + 0 * v.z;

    ret.z = -sin(tetha) * v.x
            + 0 * v.y
            + cos(tetha) * v.z;

    return ret;
}

//TODO: paralellize
Vector3 rotateZ(Vector3 v, EE_FLOAT tetha) {
        Vector3 ret;

    ret.x = cos(tetha) * v.x
            + -sin(tetha) * v.y
            + 0 * v.z;

    ret.y = sin(tetha) * v.x
            + cos(tetha) * v.y
            + 0 * v.z;

    ret.z = 0 * v.x
            + 0 * v.y
            + 1 * v.z;

    return ret;
}

#endif // M_MATRIX3_H_
