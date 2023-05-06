#ifndef M_MQUATERNION_H_
#define M_MQUATERNION_H_


#include "../kernel/header.h"

#include "mVector3.h"

typedef union __attribute__ ((packed)) Quaternion {
  struct {
    EE_FLOAT a, b, c, d;
  };
  EE_FLOAT s[4];
} Quaternion;


#endif // M_MQUATERNION_H_
