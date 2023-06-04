#ifndef M_OBJECT3_H_
#define M_OBJECT3_H_

#include "../kernel/header.h"

Plane3 newPlane3(Vector3 v1, Vector3 v2, Point3 p) {
    Plane3 ret;
    ret.p = p;

    ret.n = getNorm2(v1, v2);
    ret.off = - (ret.n.x * p.x + ret.n.y * p.y + ret.n.z * p.z);

    return ret;
}

Triangle3 newTriangle3(Point3 p1, Point3 p2, Point3 p3) {
    Triangle3 ret;
    ret.p[0] = p1;
    ret.p[1] = p2;
    ret.p[2] = p3;

    ret.v[0] = newVector(p1, p2);
    ret.v[1] = newVector(p1, p3);

    ret.pl = newPlane3(ret.v[0], ret.v[1], p1);

    ret.binv.mat.s[0*4 +0] = ret.v[0].x;
    ret.binv.mat.s[0*4 +1] = ret.v[1].x;
    ret.binv.mat.s[0*4 +2] = ret.pl.n.x;

    ret.binv.mat.s[1*4 +0] = ret.v[0].y;
    ret.binv.mat.s[1*4 +1] = ret.v[1].y;
    ret.binv.mat.s[1*4 +2] = ret.pl.n.y;
    
    ret.binv.mat.s[2*4 +0] = ret.v[0].z;
    ret.binv.mat.s[2*4 +1] = ret.v[1].z;
    ret.binv.mat.s[2*4 +2] = ret.pl.n.z;
    
    ret.base = invert(ret.binv);
    return ret;
}

#endif // M_OBJECT3_H_
