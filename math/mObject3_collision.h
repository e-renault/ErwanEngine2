#ifndef M_OBJECT3_COLLISION_H_
#define M_OBJECT3_COLLISION_H_

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

#include "mObject3.h"

Point3 collisionRayPlane(Plane3 pl, Ray3 r) {
    Point3 ret;

    float div = (pl.n.x * r.v.x + pl.n.y * r.v.y + pl.n.z * r.v.z);
    if (div == 0) div = FLT_MIN;
    
    float t = -(pl.n.x * r.p.x + pl.n.y * r.p.y + pl.n.z * r.p.z + pl.off) / div;

    ret.x = r.p.x + t * r.v.x;
    ret.y = r.p.y + t * r.v.y;
    ret.z = r.p.z + t * r.v.z;

    return ret;
}

Vector3 getLocalPosition(Matrix3 b, Vector3 o, Point3 p) {
    Vector3 ret;

    Vector3* hp2d = (Vector3*) &p;
    Vector3 off = minus_vector3(o);

    ret = multiply(b, add_vector3(*hp2d, off));

    return ret;
}

EE_INT getCollisionRayTriangle(Triangle3 t, Ray3 r) {
    Point3 p = collisionRayPlane(t.pl, r);
    Vector3* off1 = (Vector3*) &t.pl.p;
    
    Vector3 pos = getLocalPosition(t.base, *off1, p);

    int ret = 0 < pos.x;
    ret &= 0 < pos.y;
    ret &= pos.x + pos.y <= 1;

    return ret;
}

#endif // M_OBJECT3_COLLISION_H_
