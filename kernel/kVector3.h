#ifndef M_KVECTOR3_H_
#define M_KVECTOR3_H_

#include "../kernel/header.h"

//TODO: remove ?
Vector3 newVector(Point3 p1, Point3 p2) {
    return p2 - p1;
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
    Vector3 vm = v1 * v2;
    float ret = vm.x + vm.y + vm.z;
    return ret;
}

EE_FLOAT getLength2(Vector3 v) {
    Vector3 vp = v * v;
    EE_FLOAT ret = sqrt(vp.x + vp.y + vp.z);
    return ret;
}

EE_FLOAT getLength(Point3 p1, Point3 p2) {
    EE_FLOAT ret = getLength2(p2-p1);
    return ret;
}

Vector3 getNorm(Vector3 v) {
    float len = 1/getLength2(v);
    return v*len;
}

Vector3 getNorm2(Vector3 v1, Vector3 v2) {
    return getNorm(crossProduct(v1,v2));
}

float getAngle(Vector3 v1, Vector3 v2) {
  float ret;

  Vector3 vup = v1*v2;
  float up = (vup.x + vup.y + vup.z);
  float div = getLength2(v1) * getLength2(v2);
  ret = EE_ARCCOS(up/div);

  return ret;
}
#endif // M_KVECTOR3_H_
