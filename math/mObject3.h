#ifndef M_OBJECT3_H_
#define M_OBJECT3_H_

#include "mVector3.h"
#include "mMatrix3.h"


#include "../kernel/header.h"


typedef struct __attribute__ ((packed)) Plane3 {
  Point3 p; //origine of space (0-0-0 coordinate, center of the plane)
  Vector3 n;
  EE_FLOAT off;
} Plane3;

Plane3 newPlane3(Vector3 v1, Vector3 v2, Point3 p) {
    Plane3 ret;
    ret.p = p;

    ret.n = getNorm2(v1, v2);
    ret.off = - (ret.n.x * p.x + ret.n.y * p.y + ret.n.z * p.z);

    return ret;
}



typedef struct __attribute__ ((packed)) Ray3 {
  Point3 p;
  Vector3 v;
} Ray3;

Ray3 newRay3(Point3 p, Vector3 v) {
    Ray3 ret;

    ret.p = p;
    ret.v = v;
    
    return ret;
}


//useless datas ?
typedef struct __attribute__ ((packed)) Triangle3 {
  Point3 p[3];//useless ?
  Vector3 v[2];//useless ?
  Plane3 pl;
  Matrix3 base;//useless ?
  Matrix3 binv;
} Triangle3;

Triangle3 newTriangle3(Point3 p1, Point3 p2, Point3 p3) {
    Triangle3 ret;
    ret.p[0] = p1;
    ret.p[1] = p2;
    ret.p[2] = p3;

    ret.v[0] = newVector(p1, p2);
    ret.v[1] = newVector(p1, p3);

    ret.pl = newPlane3(ret.v[0], ret.v[1], p1);

    ret.binv.val[0][0] = ret.v[0].x;
    ret.binv.val[0][1] = ret.v[1].x;
    ret.binv.val[0][2] = ret.pl.n.x;

    ret.binv.val[1][0] = ret.v[0].y;
    ret.binv.val[1][1] = ret.v[1].y;
    ret.binv.val[1][2] = ret.pl.n.y;
    
    ret.binv.val[2][0] = ret.v[0].z;
    ret.binv.val[2][1] = ret.v[1].z;
    ret.binv.val[2][2] = ret.pl.n.z;
    
    ret.base = invert(ret.binv);
    return ret;
}

#endif // M_OBJECT3_H_
