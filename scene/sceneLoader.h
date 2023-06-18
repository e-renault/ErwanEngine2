#ifndef SCENE_LOADER_H_
#define SCENE_LOADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "../math/lib3D.h"
#include "../math/lib3D_debug.h"

//TODO potential error, the whole file should be rewrited
int loadSceneFromFile(
        char* path,
        int* nb_triangle,
        Triangle3* triangles,
        Texture* textures,
        int* nb_lights,
        LightSource3* lights,
        Point3* cam_coordinate,
        Vector3* cam_lookat,
        Vector3* sky_light_dir,
        Texture* sky_light_texture
    ) {

    FILE *fptr;
    size_t len1, len2;
    char* line = NULL;
    char* rest;
    char* identifier;

    fptr = fopen(path,"r");
    if (fptr == NULL) {
        printf(" ##### /!\\ File not found ! ##### \n");
        return 0;
    }
    
    Point3 points[1000];
    EE_FLOAT2 texture_buffer[400] = {(0,0)};
    EE_FLOAT3 normal_buffer[400];
    char object[50][10] = {"missingno"};

    int texture_index = 1;
    int normal_index = 1;
    int point_index = 0;
    int object_index = 0;
    int triangle_index = 0;
    int _;
    *nb_lights = 0;
    *nb_triangle = 0;
    
    while ((len2 = getline(&line, &len1, fptr)) != -1) {
        identifier = strtok_r(line, " ", &rest);
        
        if (!strcmp(identifier, "o")) {
            sscanf(rest, " %s", object[object_index]);
            object_index++;
        } else if (!strcmp(identifier, "v")) {
            EE_FLOAT x, y, z;
            sscanf(rest, " %f %f %f", &x, &y, &z);
            points[point_index++] = (Point3) {x, y, z};
            
            //printPoint3(points[point_index-1]);printf("\n");
        } else if (!strcmp(identifier, "vt")) {
            EE_FLOAT c1, c2;
            sscanf(rest, " %f %f", &c1, &c2);
            texture_buffer[texture_index++] = (EE_FLOAT2) {c1, c2};
            
            //printf("(%f, %f, %f)\n", color[color_index-1].r, color[color_index-1].r, color[color_index-1].r);
        } else if (!strcmp(identifier, "vn")) {
            EE_FLOAT x, y, z;
            sscanf(rest, " %f %f %f", &x, &y, &z);
            normal_buffer[normal_index++] = (EE_FLOAT3) {x, y, z};
            
            //printPoint3(points[point_index-1]);printf("\n");
        } else if (!strcmp(identifier, "f")) {
            int v1, v2, v3;
            int vt1=0, vt2=0, vt3=0;
            int vn1, vn2, vn3;

            int ret=0;
            if (!ret) ret = 9==sscanf(rest, " %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);
            if (!ret) ret = 6==sscanf(rest, " %d//%d %d//%d %d//%d", &v1, &vn1, &v2, &vn2, &v3, &vn3);
            if (!ret) ret = 6==sscanf(rest, " %d/%d/ %d/%d/ %d/%d/", &v1, &vt1, &v2, &vt2, &v3, &vt3);
            if (!ret) ret = 3==sscanf(rest, " %d// %d// %d//", &v1, &v2, &v3);
            if (!ret) ret = 3==sscanf(rest, " %d %d %d", &v1, &v2, &v3);
            if (!ret) printf("Error while parsing triangle [%i]\n",ret);

            EE_FLOAT2 voff = texture_buffer[vt2];
            EE_FLOAT2 vty = {texture_buffer[vt1].x-voff.x, texture_buffer[vt1].y-voff.y};
            EE_FLOAT2 vtx = {texture_buffer[vt3].x-voff.x, texture_buffer[vt3].y-voff.y};
            
        
            textures[triangle_index] = (Texture) {.v1=vtx,.v2=vty,.voff=voff};
            triangles[triangle_index++] = newTriangle3(points[v2-1], points[v3-1], points[v1-1]);
            
            
            //printf("[%i](%i, %i, %i)\n",ret, v1, v2, v3);
            //printTriangle3(triangles[triangle_index]);
            //printf("vtx{%f,%f}, vty{%f,%f}, voff{%f,%f}\n", vtx.x, vtx.y, vty.x, vty.y, voff.x, voff.y);
        } else {
            //printf("[unidentified] {%s}\n", rest);//TODO: C'est vraiment de la merde ce truc
        }
        
    }
    *nb_triangle = triangle_index;

    printf("loaded : %s -> triangles : %i, points : %i, color : %i\n", object[0], triangle_index, point_index, texture_index);

    fclose(fptr);
    if (line)
        free(line);

    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {0,0,1},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {7,2,3},
        .intensity = 12
    };
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {1,0,0},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {0,2,7},
        .intensity = 12
    };
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {0,1,0},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {-7,2,3},
        .intensity = 12
    };
    
    //*nb_lights = 1;
    *cam_coordinate = (Point3) {0.5, 0.5, 2.0};
    *cam_lookat = (Vector3) {0, 0, -1};
    *sky_light_dir = (Vector3) {-0.7, -1, -0.5};
    *sky_light_texture = (Texture) {
        .color1={0.58,  0.78,  0.92},
        .color2={1.3,   1.3,   1.3},//sky lum base_ref
        .color3={0,     0,     1}
    };

    return *nb_triangle;
}

//TODO: To be deleted (c'était drole mais bon à un moment faut stop)
int loadCubeScene(
        char* path,  
        int* nb_triangle,
        Triangle3* triangles,
        Texture* textures,
        int* nb_lights,
        LightSource3* lights,
        Point3* cam_coordinate,
        Vector3* cam_lookat,
        Vector3* sky_light_dir,
        Texture* sky_light_texture
    ) {

    struct timeval time;
    gettimeofday(&time, NULL);
    float _sec = ((float) time.tv_usec/1000000) + time.tv_sec % 100;
    float _secmod = _sec / 8;
    float _trunksecmod = _secmod - truncf(_secmod);
    float _rad = _trunksecmod * 2 * 3.14;
    float alphaz = sin(_rad) * 0.2f + 0.5f;
    float sin_alpha = sin(_rad) * 0.7f;
    float cos_alpha = cos(_rad) * 0.7f;
    float k = 2;

    Point3 pAL = {-sin_alpha*k,     alphaz,   -cos_alpha*k};
    Point3 pBL = {cos_alpha*k,      alphaz,   -sin_alpha*k};
    Point3 pCL = {sin_alpha*k,      alphaz,   cos_alpha*k};
    Point3 pDL = {-cos_alpha*k,     alphaz,   sin_alpha*k};

    Point3 pAU = {-sin_alpha*k,     alphaz+k,   -cos_alpha*k};
    Point3 pBU = {cos_alpha*k,      alphaz+k,   -sin_alpha*k};
    Point3 pCU = {sin_alpha*k,      alphaz+k,   cos_alpha*k};
    Point3 pDU = {-cos_alpha*k,     alphaz+k,   sin_alpha*k};

    Point3 pla = {-150, -0.00, -100};
    Point3 plb = {50, -0.00, -100};
    Point3 plc = {50, -0.00, 100};

    int i = 0;
    triangles[i++] = newTriangle3(pla, plb, plc);

    triangles[i++] = newTriangle3(pAL, pBL, pAU);
    triangles[i++] = newTriangle3(pBL, pCL, pBU);
    triangles[i++] = newTriangle3(pCL, pDL, pCU);
    //triangles[i++] = newTriangle3(pDL, pAL, pDU);

    triangles[i++] = newTriangle3(pAU, pBU, pBL);
    triangles[i++] = newTriangle3(pBU, pCU, pCL);
    triangles[i++] = newTriangle3(pCU, pDU, pDL);
    //triangles[i++] = newTriangle3(pDU, pAU, pAL);
    
    triangles[i++] = newTriangle3(pAU, pBU, pCU);
    triangles[i++] = newTriangle3(pAU, pCU, pDU);
    triangles[i++] = newTriangle3(pAL, pBL, pCL);
    triangles[i++] = newTriangle3(pAL, pCL, pDL);

    *nb_triangle = i;

    EE_FLOAT2 voff = {0, 0};
    EE_FLOAT2 vty = {0.5, 0};
    EE_FLOAT2 vtx = {0, 0.5};
    for (;i--;) {
        textures[i] = (Texture) {
            .v1=vtx,.v2=vty,.voff=voff
        };
    }
    *nb_lights = 0;
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {0.80,0.75,0.77},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {0,alphaz+k/2, 0},
        .intensity = 12
    };
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {cos(_rad), cos(_rad+3.14*0.666), cos(_rad+3.14*1.333)},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {cos_alpha*k*2,0.1,sin_alpha*k*2},
        .intensity = 12
    };
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {sin(_rad), sin(_rad+3.14*0.666), sin(_rad+3.14*1.333)},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {cos_alpha*k*2,0.1,cos_alpha*k*2},
        .intensity = 12
    };
    
    return *nb_triangle;
}

#endif //SCENE_LOADER_H_
