#ifndef HEADER_H_
#define HEADER_H_

#ifndef __OPENCL_VERSION__
  #include <math.h>
  #include <float.h>
  #include <stdio.h>
  #define CL_TARGET_OPENCL_VERSION 300
  #ifdef __APPLE__
    #include <OpenCL/opencl.h>
  #else
    #include <CL/cl.h>
  #endif

  #define EE_ARCCOS(c) (acos(c)/M_PI)
  #define EE_FLOAT cl_float
  #define EE_FLOAT2 cl_float2
  #define EE_FLOAT3 cl_float3
  #define EE_FLOAT4 cl_float4
  #define EE_FLOAT4x4 cl_float16
  #define EE_INT cl_int
  #define EE_CONST const
  #define EE_MAX_INT CL_UINT_MAX
#else
  #define EE_ARCCOS(c) (acospi(c))
  #define EE_FLOAT float
  #define EE_FLOAT2 float2
  #define EE_FLOAT3 float3
  #define EE_FLOAT4 float4
  #define EE_FLOAT4x4 float16
  #define EE_INT int
  #define EE_CONST __constant
  #define PI 3.14159 
  #define EE_MAX_INT UINT_MAX
#endif

typedef EE_FLOAT3 Vector3;
typedef EE_FLOAT3 Point3;
typedef EE_FLOAT4 rgb;//TODO: should be refactored ?
EE_CONST Vector3 UP = {0, 1, 0};
EE_CONST Vector3 DOWN = {0, -1, 0};
EE_CONST Vector3 RIGHT = {1, 0, 0};

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

//TODO: useless datas ?
typedef struct __attribute__ ((packed)) Triangle3 {
  Point3 p[3];//useless ?
  Vector3 v[2];//useless ?
  Plane3 pl;
  Matrix3 base;//useless ?
  Matrix3 binv;
  Sphere3 sphere;
} Triangle3;

typedef struct __attribute__ ((packed)) Texture {
    EE_FLOAT2 v1;
    EE_FLOAT2 v2;
    EE_FLOAT2 voff;
} Texture;

//TODO: refactor ?
typedef struct __attribute__ ((packed)) Material {
    char newmtl[200];//useless ?
    EE_INT newmtl_index; //useless ?
    rgb Ka;
    rgb Ke;
    int hasTexture;//useless ?
    char texture_path[400];//useless ?
    Texture map_Kd;//useless ?
    EE_INT map_Kd_index;//useless ?
} Material;

typedef struct __attribute__ ((packed)) LocalPixelData {
    rgb color_value_buffer;
    rgb global_illum_buffer;
    rgb direct_light_buffer;
    float z_value_buffer;
    int obj_buffer;
    Vector3 normal_buffer;
    Point3 point_buffer;
} LocalPixelData;

//TODO: should be deleted ?
typedef struct __attribute__ ((packed)) LightSource3 {
    rgb color;
    Vector3 dir;
    Point3 source;
    EE_FLOAT luminosity;
} LightSource3;



#endif