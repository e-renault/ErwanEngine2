#ifndef __OPENCL_VERSION__
  #include <math.h>
  #include <float.h>
  #include <stdio.h>
  #ifdef __APPLE__
    #include <OpenCL/opencl.h>
  #else
    #include <CL/cl.h>
  #endif
#endif

#ifndef EE_FLOAT
  #ifndef __OPENCL_VERSION__
    #define EE_ARCCOS(c) (acos(c)/M_PI)
    #define EE_FLOAT cl_float
    #define EE_INT cl_int
  #else
    #define EE_ARCCOS(c) (acospi(c))
    #define EE_FLOAT float
    #define EE_INT int
  #endif
#endif
