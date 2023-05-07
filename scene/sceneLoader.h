#ifndef SCENE_LOADER_H_
#define SCENE_LOADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../math/lib3D.h"

int loadSceneFromFile(
        char* path,  
        int* nb_triangle,
        Triangle3* triangles,
        Texture* textures
    ) {

    FILE *fptr;
    size_t len1, len2;
    char* line = NULL;
    char* rest;
    char* identifier;

    fptr = fopen(path,"r");
    if (fptr == NULL)
        return 0;


    Point3 points[400];
    rgb color[400];
    color[0]=(rgb){.r=1,.g=1,.b=1};
    char scene_name[50];
    char object[50];

    int color_index = 1;
    int point_index = 0;
    int triangle_index = 0;
    int _;
    
    while ((len2 = getline(&line, &len1, fptr)) != -1) {
        identifier = strtok_r(line, " ", &rest);
        
        if (!strcmp(identifier, "#")) {
            sscanf(rest, " %s", scene_name);
        } else if (!strcmp(identifier, "o")) {
            sscanf(rest, " %s", object);
        } else if (!strcmp(identifier, "v")) {
            float x, y, z;
            sscanf(rest, " %f %f %f", &x, &y, &z);
            points[point_index] = (Point3) {.c={x, y, z}};
            point_index++;
            
            //printPoint3(points[point_index-1]);printf("\n");
        } else if (!strcmp(identifier, "vc")) {
            float r, g, b;
            sscanf(rest, " %f %f %f", &r, &g, &b);
            color[color_index] = (rgb) {.r=r, .g=g, .b=b};
            color_index++;
            
            //printPoint3(points[point_index-1]);printf("\n");
        } else if (!strcmp(identifier, "f")) {
            int v1, v2, v3;
            int vt1=0, vt2=0, vt3=0;
            int vn1, vn2, vn3;
            int ret = sscanf(rest, " %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);
            if (ret != 9)
                ret = sscanf(rest, " %d//%d %d//%d %d//%d", &v1, &vn1, &v2, &vn2, &v3, &vn3);
            if(ret != 6)
                ret = sscanf(rest, " %d/%d/ %d/%d/ %d/%d/", &v1, &vt1, &v2, &vt2, &v3, &vt3);
            if(ret != 6)
                ret = sscanf(rest, " %d// %d// %d//", &v1, &v2, &v3);
            if(ret != 3)
                ret = sscanf(rest, " %d %d %d", &v1, &v2, &v3);

            textures[triangle_index] = (Texture) {.x_res = 1, .y_res = 1, .color1=color[vt1], .color2=color[vt2], .color3=color[vt3]};
            triangles[triangle_index] = newTriangle3(points[v1-1], points[v2-1], points[v3-1]);
            triangle_index++;
            
            //printf("(%i, %i, %i)\n", v1, v2, v3);
            //printTriangle3(triangles[triangle_index]);
        } else {
            //printf("[unidentified] {%s}\n", rest);
        }
        
    }
    *nb_triangle = triangle_index;

    printf("loaded : %s -> triangles : %i, points : %i\n", object, triangle_index, point_index);

    fclose(fptr);
    if (line)
        free(line);

    return *nb_triangle;
}

int loadCubeScene(float _sec, Triangle3* triangles) {
    float _secmod = _sec / 8;
    float _trunksecmod = _secmod - truncf(_secmod);
    float _rad = _trunksecmod * 2 * 3.14;
    float alphaz = sin(_rad) * 0.2f + 0.5f;
    float sin_alpha = sin(_rad) * 0.7f;
    float cos_alpha = cos(_rad) * 0.7f;


    Point3 pAL = {-sin_alpha,     alphaz,   -cos_alpha};
    Point3 pBL = {cos_alpha,      alphaz,   -sin_alpha};
    Point3 pCL = {sin_alpha,      alphaz,   cos_alpha};
    Point3 pDL = {-cos_alpha,     alphaz,   sin_alpha};

    Point3 pAU = {-sin_alpha,     alphaz+1,   -cos_alpha};
    Point3 pBU = {cos_alpha,      alphaz+1,   -sin_alpha};
    Point3 pCU = {sin_alpha,      alphaz+1,   cos_alpha};
    Point3 pDU = {-cos_alpha,     alphaz+1,   sin_alpha};

    Point3 pla = {-150, -0.00, -100};
    Point3 plb = {50, -0.00, -100};
    Point3 plc = {50, -0.00, 100};

    int i = 0;
    triangles[i++] = newTriangle3(pla, plb, plc);

    triangles[i++] = newTriangle3(pAL, pBL, pAU);
    //triangles[i++] = newTriangle3(pBL, pCL, pBU);
    triangles[i++] = newTriangle3(pCL, pDL, pCU);
    //triangles[i++] = newTriangle3(pDL, pAL, pDU);

    triangles[i++] = newTriangle3(pAU, pBU, pBL);
    //triangles[i++] = newTriangle3(pBU, pCU, pCL);
    triangles[i++] = newTriangle3(pCU, pDU, pDL);
    //triangles[i++] = newTriangle3(pDU, pAU, pAL);
    
    triangles[i++] = newTriangle3(pAU, pBU, pCU);
    triangles[i++] = newTriangle3(pAU, pCU, pDU);
    triangles[i++] = newTriangle3(pAL, pBL, pCL);
    triangles[i++] = newTriangle3(pAL, pCL, pDL);
    return i;
}

#endif //SCENE_LOADER_H_