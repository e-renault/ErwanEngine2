#ifndef LIGHT_SOURCE_H_
#define LIGHT_SOURCE_H_

#include "../math/lib3D.h"
#include "../render/color.h"

typedef struct __attribute__ ((packed)) LightSource3 {
    rgb color;
    Vector3 dir;
    Point3 source;
    EE_FLOAT intensity;
} LightSource3;

#endif //LIGHT_SOURCE_H_