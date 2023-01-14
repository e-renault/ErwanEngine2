#ifndef M_VECTOR3_H_
#define M_VECTOR3_H_

#ifndef FLT_MIN
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

typedef struct __attribute__ ((packed)) Vector3 {
  EE_FLOAT x, y, z;
} Vector3;

typedef struct __attribute__ ((packed)) Point3 {
  EE_FLOAT x, y, z;
} Point3;


Vector3 newVector(Point3 p1, Point3 p2) {
    Vector3 ret = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    return ret;
}

//TODO: paralellize
Vector3 add_vector3(Vector3 v1, Vector3 v2) {
    Vector3 ret;

    ret.x = v1.x + v2.x;
    ret.y = v1.y + v2.y;
    ret.z = v1.z + v2.z;

    return ret;
}

//TODO: paralellize
Vector3 minus_vector3(Vector3 v) {
    Vector3 ret;

    ret.x = - v.x;
    ret.y = - v.y;
    ret.z = - v.z;

    return ret;
}

//TODO: paralellize
Vector3 crossProduct(Vector3 v1, Vector3 v2) {
    Vector3 ret = {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
    return ret;
}

EE_FLOAT getLength(Point3 p1, Point3 p2) {
    float d1 = p2.x - p1.x;
    float d2 = p2.y - p1.y;
    float d3 = p2.z - p1.z;
    EE_FLOAT ret = sqrt(d1*d1 + d2*d2 + d3*d3);
    return ret;
}

EE_FLOAT getLength2(Vector3 v) {
    EE_FLOAT ret = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    return ret;
}

Vector3 getNorm(Vector3 v) {
    float len = getLength2(v);
    Vector3 ret = {v.x / len, v.y / len, v.z / len};
    return ret;
}

Vector3 getNorm2(Vector3 v1, Vector3 v2) {
    Vector3 ret = crossProduct(v1,v2);
    //TODO: renormalize
    //ret = getNorm(ret);
    return ret;
}


#endif // M_VECTOR3_H_
