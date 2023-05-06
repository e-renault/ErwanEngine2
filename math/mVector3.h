#ifndef M_VECTOR3_H_
#define M_VECTOR3_H_

#include "../kernel/header.h"


typedef union __attribute__ ((packed)) Vector3 {
    struct {
        EE_FLOAT x, y, z;
    };
    EE_FLOAT v[3];
} Vector3;

typedef union __attribute__ ((packed)) Point3 {
    struct {
        EE_FLOAT x, y, z;
    };
    EE_FLOAT c[3];
} Point3;

//TODO this is an horrible conversion
Vector3 static inline toVector(Point3 p) {
    return *((Vector3*) &p);
}

//TODO this is an horrible conversion
Point3 static inline toPoint(Vector3 v) {
    return *((Point3*) &v);
}

Vector3 newVector(Point3 p1, Point3 p2) {
    Vector3 ret = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    return ret;
}

Point3 newPoint(EE_FLOAT x, EE_FLOAT y, EE_FLOAT z) {
    Point3 ret = {x, y, z};
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

float dotProduct(Vector3 v1, Vector3 v2) {
    float ret = v1.x * v2.x;
    ret += v1.y * v2.y;
    ret += v1.z * v2.z;
    return ret;
}

Vector3 static inline scale_vector3(float s, Vector3 v) {
    Vector3 ret;

    ret.x = s* v.x;
    ret.y = s* v.y;
    ret.z = s* v.z;

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
    ret = getNorm(ret);
    return ret;
}

float getAngle(Vector3 v1, Vector3 v2) {
  float ret;

  float up = (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
  float div = getLength2(v1) * getLength2(v2);
  ret = EE_ARCCOS(up/div);

  return ret;
}

#endif // M_VECTOR3_H_
