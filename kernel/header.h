#ifndef HEADER_H_
#define HEADER_H_

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
    #define EE_FLOAT3 cl_float3
    #define EE_FLOAT4x4 cl_float16
    #define EE_INT cl_int
  #else
    #define EE_ARCCOS(c) (acospi(c))
    #define EE_FLOAT float
    #define EE_FLOAT3 float3
    #define EE_FLOAT4x4 float16
    #define EE_INT int
  #endif
#endif

typedef EE_FLOAT3 Vector3;
typedef EE_FLOAT3 Point3;
typedef EE_FLOAT3 rgb;

typedef union Matrix3 {
  EE_FLOAT __attribute__ ((packed)) val[4][4];
  EE_FLOAT4x4 mat;
} Matrix3;

typedef struct __attribute__ ((packed)) Plane3 {
  Point3 p; //origine of space (0-0-0 coordinate, center of the plane)
  Vector3 n;
  EE_FLOAT off;
} Plane3;

typedef struct __attribute__ ((packed)) Ray3 {
  Point3 p;
  Vector3 v;
} Ray3;

typedef struct __attribute__ ((packed)) Sphere3 {
  Point3 center;
  EE_FLOAT radius;
} Sphere3;

//useless datas ?
typedef struct __attribute__ ((packed)) Triangle3 {
  Point3 p[3];//useless ?
  Vector3 v[2];//useless ?
  Plane3 pl;
  Matrix3 base;//useless ?
  Matrix3 binv;
  Sphere3 sphere;//useless ?
} Triangle3;


typedef struct __attribute__ ((packed)) Texture {
    EE_INT x_res;
    EE_INT y_res;
    rgb color1;
    rgb color2;
    rgb color3;
} Texture;

typedef struct __attribute__ ((packed)) LightSource3 {
    rgb color;
    Vector3 dir;
    Point3 source;
    EE_FLOAT intensity;
} LightSource3;

#endif