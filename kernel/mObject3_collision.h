#ifndef M_OBJECT3_COLLISION_H_
#define M_OBJECT3_COLLISION_H_

#include "../kernel/header.h"
#include "../math/mVector3.h"
#include "../math/mMatrix3.h"

Point3 collisionRayPlane(Plane3 pl, Ray3 r, float* t);
Point3 collisionRayPlane(Plane3 pl, Ray3 r, float* t) {
    float3 tmp = pl.n * r.v;
    float div = tmp.x + tmp.y + tmp.z;
    //float div = (pl.n.x * r.v.x + pl.n.y * r.v.y + pl.n.z * r.v.z);
    
    if (div == 0) div = FLT_MIN;
    
    float3 tmp2 = pl.n * r.p;
    *t = -(tmp2.x + tmp2.y + tmp2.z + pl.off) / div;
    //*t = -(pl.n.x * r.p.x + pl.n.y * r.p.y + pl.n.z * r.p.z + pl.off) / div;
    
    return r.p + (*t) * r.v;
}

Point3 collisionRayPlane_MöllerTrumbore(Plane3 pl, Ray3 r, float* t);
Point3 collisionRayPlane_MöllerTrumbore(Plane3 pl, Ray3 r, float* t) {
    //TODO implement
    return (Point3) {0, 0, 0};
}


//TODO remove caus' unused anymore
Vector3 getLocalPosition(Matrix3 b, Point3 o, Point3 p);
Vector3 getLocalPosition(Matrix3 b, Point3 o, Point3 p) {
    Vector3 ret;

    Vector3 off = -o;

    ret = multiply(b, (p + off));

    return ret;
}

int getCollisionRayTriangle(Triangle3 t, Ray3 r, float max_dist, Point3* global_point, Vector3* local_point, Vector3* normal, float* dist);
int getCollisionRayTriangle(Triangle3 t, Ray3 r, float max_dist, Point3* global_point, Vector3* local_point, Vector3* normal, float* dist) {
    *global_point = collisionRayPlane(t.pl, r, dist);
    if ( *dist > max_dist ) return 0;

    *local_point = multiply(t.base, *global_point - t.pl.p);

    float angle = getAngle(t.pl.n, r.v);
    *normal = (angle>0.5) ? t.pl.n: -(t.pl.n);
    
    int ret = 0 < local_point->x;
    ret &= 0 < local_point->y;
    ret &= local_point->x + local_point->y <= 1;
    ret &= *dist > 0.0001;//prevent auto-detect collision

    return ret;
}

#endif // M_OBJECT3_COLLISION_H_
