#ifndef M_MQUATERNION_H_
#define M_MQUATERNION_H_

#ifndef FLT_MIN
  #include <float.h>
  #include <math.h>
  #ifdef __APPLE__
    #include <OpenCL/opencl.h>
  #else
    #include <CL/cl.h>
  #endif
#endif

#ifndef EE_FLOAT
  #ifndef FLT_MIN
    #define EE_FLOAT cl_float
    #define EE_INT cl_int
  #else
    #define EE_FLOAT float
    #define EE_INT int
  #endif
#endif

#include "mVector3.h"

typedef struct __attribute__ ((packed)) Quaternion {
  EE_FLOAT val[4];
} Quaternion;


#endif // M_MQUATERNION_H_
