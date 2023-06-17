#ifndef M_VECTOR3_H_
#define M_VECTOR3_H_

#include "../kernel/header.h"

Vector3 newVector(Point3 p1, Point3 p2) {
    return (Vector3) {
        p2.x - p1.x, 
        p2.y - p1.y, 
        p2.z - p1.z
    };
}

//TODO: paralellize
Vector3 add_vector3(Vector3 v1, Vector3 v2) {
    return (Vector3) {
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z
    };
}

//TODO: paralellize
Vector3 minus_vector3(Vector3 v) {
    return (Vector3) {
        - v.x,
        - v.y,
        - v.z
    };
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

EE_FLOAT dotProduct(Vector3 v1, Vector3 v2) {
    EE_FLOAT ret = v1.x * v2.x;
    ret += v1.y * v2.y;
    ret += v1.z * v2.z;
    return ret;
}

Vector3 scale_vector3(EE_FLOAT s, Vector3 v) {
    return (Vector3) {
        s* v.x,
        s* v.y,
        s* v.z
    };
}

EE_FLOAT getLength(Point3 p1, Point3 p2) {
    EE_FLOAT d1 = p2.x - p1.x;
    EE_FLOAT d2 = p2.y - p1.y;
    EE_FLOAT d3 = p2.z - p1.z;
    EE_FLOAT ret = sqrt(d1*d1 + d2*d2 + d3*d3);
    return ret;
}

EE_FLOAT getLength2(Vector3 v) {
    EE_FLOAT ret = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    return ret;
}

Vector3 getNorm(Vector3 v) {
    EE_FLOAT len = getLength2(v);
    return (Vector3) {
        v.x / len, 
        v.y / len,
        v.z / len
    };
}

Vector3 getNorm2(Vector3 v1, Vector3 v2) {
    return getNorm(crossProduct(v1,v2));
}

EE_FLOAT getAngle(Vector3 v1, Vector3 v2) {
  EE_FLOAT ret;

  EE_FLOAT up = (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
  EE_FLOAT div = getLength2(v1) * getLength2(v2);
  ret = EE_ARCCOS(up/div);

  return ret;
}

#endif // M_VECTOR3_H_
