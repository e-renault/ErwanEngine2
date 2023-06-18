#ifndef M_OBJECT3_COLLISION_H_
#define M_OBJECT3_COLLISION_H_

#include "../kernel/header.h"
#include "../kernel/kVector3.h"
#include "../kernel/kMatrix4x4.h"

Point3 collisionRayPlane(Plane3 pl, Ray3 r, EE_FLOAT* t);
Point3 collisionRayPlane(Plane3 pl, Ray3 r, EE_FLOAT* t) {
    EE_FLOAT3 tmp = pl.n * r.v;
    EE_FLOAT div = sum(tmp);
    
    if (div == 0) div = FLT_MIN;
    
    EE_FLOAT3 tmp2 = pl.n * r.p;
    *t = -(sum(tmp2) + pl.off) / div;
    
    return r.p + (*t) * r.v;
}


int getCollisionRaySphere(Sphere3 sp, Ray3 r, EE_FLOAT max_dist, Point3* global_point, Vector3* local_point, Vector3* normal, EE_FLOAT* dist);
int getCollisionRaySphere(Sphere3 sp, Ray3 r, 
    EE_FLOAT max_dist, 
    Point3* global_point, 
    Vector3* local_point, 
    Vector3* normal, 
    EE_FLOAT* dist) {
        
    EE_FLOAT coefa = sum(r.v * r.v);
    EE_FLOAT3 sub = r.p - sp.center;
    EE_FLOAT coefb = 2* sum(sub * r.v);
    EE_FLOAT coefc = sum(sub*sub) - sp.radius*sp.radius;
    EE_FLOAT delta = coefb * coefb - 4 * coefa * coefc;

    if (delta > 0){
        EE_FLOAT t = (-coefb - sqrt(delta)) / (2 * coefa);
        if (t <= 0.0001) {//take second point
            t = (-coefb + sqrt(delta)) / (2 * coefa);
        }
        if (t <= 0.0001) {//prevent auto-detect collision
            return 0;
        }
        Vector3 p = r.v * t + r.p;
        *dist = t;
        *global_point = p;
        *normal =  p - sp.center;
        
        return 1;
    }

    return 0;
}

int getCollisionRayTriangle(Triangle3 t, Ray3 r, EE_FLOAT max_dist, Point3* global_point, Vector3* local_point, Vector3* normal, EE_FLOAT* dist);
int getCollisionRayTriangle(Triangle3 t, Ray3 r, EE_FLOAT max_dist, Point3* global_point, Vector3* local_point, Vector3* normal, EE_FLOAT* dist) {
    
    /** Sphere optimisation */
    Point3 global_pos;Vector3 local_pos;Vector3 normal_sphere;EE_FLOAT dist_sphere;
    int hit = getCollisionRaySphere(t.sphere, r, max_dist, &global_pos, &local_pos, &normal_sphere, &dist_sphere);
    if (! hit) return 0;

    /** Juste pour le bonheur des yeux*/
    /** if (t.sphere.radius < 1) {
        if (dist_sphere > max_dist) return 0;
        *dist = dist_sphere;
        *global_point = global_pos;
        *normal = normal_sphere;
        return 1;
    }*/

    /** get Intersection Point */
    *global_point = collisionRayPlane(t.pl, r, dist);
    if ( *dist > max_dist ) return 0;

    /** Check if in triangle */
    *local_point = multiply(t.base, *global_point - t.pl.p);

    EE_FLOAT angle = getAngle(t.pl.n, r.v);
    *normal = (angle>0.5) ? t.pl.n: -(t.pl.n);
    
    int ret = 0 <= local_point->x;
    ret &= 0 <= local_point->y;
    ret &= *dist > 0.0001;//prevent auto-detect collision
    ret &= local_point->x + local_point->y <= 1;
    //ret &= 1 >= local_point->y;
    //ret &= 1 >= local_point->x;

    return ret;
}


#endif // M_OBJECT3_COLLISION_H_
