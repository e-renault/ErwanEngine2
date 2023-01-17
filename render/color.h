#ifndef COLOR_H_
#define COLOR_H_

#ifndef FLT_MIN
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

typedef struct rgb {
    EE_FLOAT r, g, b;
} rgb;

#endif //COLOR_H_