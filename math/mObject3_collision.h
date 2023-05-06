#ifndef M_OBJECT3_COLLISION_H_
#define M_OBJECT3_COLLISION_H_


#include "../kernel/header.h"

#include "mObject3.h"

Point3 collisionRayPlane(Plane3 pl, Ray3 r, EE_FLOAT* t) {
    Point3 ret;

    float div = (pl.n.x * r.v.x + pl.n.y * r.v.y + pl.n.z * r.v.z);
    if (div == 0) div = FLT_MIN;
    
    *t = -(pl.n.x * r.p.x + pl.n.y * r.p.y + pl.n.z * r.p.z + pl.off) / div;

    ret.x = r.p.x + (*t) * r.v.x;
    ret.y = r.p.y + (*t) * r.v.y;
    ret.z = r.p.z + (*t) * r.v.z;

    return ret;
}

Vector3 getLocalPosition(Matrix3 b, Point3 o, Point3 p) {
    Vector3 ret;

    Vector3 off = minus_vector3(toVector(o));

    ret = multiply(b, add_vector3(toVector(p), off));

    return ret;
}

int getCollisionRayTriangle(Triangle3 t, Ray3 r, Point3* global_point, Vector3* local_point, Vector3* normal) {
    float k;
    *global_point = collisionRayPlane(t.pl, r, &k);

    *local_point = getLocalPosition(t.base, t.pl.p, *global_point);

    float angle = getAngle(t.pl.n, r.v);
    *normal = (angle>0.5) ? t.pl.n:minus_vector3(t.pl.n);
    
    int ret = 0 < local_point->x;
    ret &= 0 < local_point->y;
    ret &= local_point->x + local_point->y <= 1;
    ret &= k > 0.000001;//prevent auto-detect collision

    return ret;
}

#endif // M_OBJECT3_COLLISION_H_
