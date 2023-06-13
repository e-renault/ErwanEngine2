#ifndef SCENE_LOADER_H_
#define SCENE_LOADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "../math/lib3D.h"
#include "../math/lib3D_debug.h"

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

    //TODO potential error
    Point3 points[1000];
    rgb color[400];
    color[0]=(rgb){1,1,1};
    char object[50][10] = {"missingno"};

    int color_index = 1;
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
            float x, y, z;
            sscanf(rest, " %f %f %f", &x, &y, &z);
            points[point_index] = (Point3) {x, y, z};
            point_index++;
            
            //printPoint3(points[point_index-1]);printf("\n");
        } else if (!strcmp(identifier, "vt")) {
            float c1, c2;
            sscanf(rest, " %f %f", &c1, &c2);
            color[color_index] = (rgb) {c1, c2, 1-(c1+c2)};
            color_index++;
            
            //printf("(%f, %f, %f)\n", color[color_index-1].r, color[color_index-1].r, color[color_index-1].r);
        } else if (!strcmp(identifier, "vc")) {
            float r, g, b;
            sscanf(rest, " %f %f %f", &r, &g, &b);
            color[color_index] = (rgb) {r, g, b};
            color_index++;
            
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

            textures[triangle_index] = (Texture) {.x_res = 1, .y_res = 1, .color1=color[vt1], .color2=color[vt2], .color3=color[vt3]};
            triangles[triangle_index] = newTriangle3(points[v1-1], points[v2-1], points[v3-1]);
            triangle_index++;
            
            //printf("[%i](%i, %i, %i)\n",ret, v1, v2, v3);
            //printTriangle3(triangles[triangle_index]);
        } else {
            //printf("[unidentified] {%s}\n", rest);
        }
        
    }
    *nb_triangle = triangle_index;

    printf("loaded : %s -> triangles : %i, points : %i, color : %i\n", object[0], triangle_index, point_index, color_index);

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
    *cam_coordinate = (Point3) {0.0, 0.1, 12.0};
    *cam_lookat = (Vector3) {0, 0, -1};
    *sky_light_dir = (Vector3) {-0.7, -1, -0.5};
    *sky_light_texture = (Texture) {
        .color1={0.58,  0.78,  0.92},
        .color2={1.3,   1.3,   1.3},//sky lum base_ref
        .color3={0,     0,     1}
    };

    return *nb_triangle;
}

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

    *nb_triangle = i;

    rgb random_colors[40] = {
        {0.036422119577474, 0.704109688303234, 0.45896783111661},
        {0.956269448285397, 0.0225273324605426, 0.92021036821545},
        {0.420983478522237, 0.941959452024163, 0.872646938139664},        
        {0.511912583274566, 0.873792707860716, 0.289653062852194},        
        {0.073244978282671, 0.409036657662119, 	0.57471693354335},
        {0.952614859803359, 0.696963677206315, 0.808357751569233},        
        {0.587739078058138, 0.388074065093731, 0.996909808547681},        
        {0.840783554242627, 0.840773691455328, 0.326636740994671},        
        {0.433044182104898, 0.865486673796387, 0.103800389361594},        
        {0.030073544872235, 0.008653624617048, 0.746564945594},
        {0.374376202921323, 0.242621578765991, 0.433938037871508},        
        {0.388967932380261, 0.919815685550748, 0.317127439679906},        
        {0.491511090163302, 0.627888033110743, 0.062552685579218},
        {0.896303796783786, 0.538532580810791, 0.563982140030735},        
        {0.582902661225595, 0.880817534389428, 0.745208654421003},        
        {0.156943572641026, 0.66437873703904, 0.610699686272349},
        {0.954385160276854, 0.445921688189521, 0.98975014618411},
        {0.082011468003816, 0.432656475032383, 	0.12380354853685},
        {0.675419855566087, 0.0494199145340208, 0.97066110137608},
        {0.672434702344021, 0.650425553397091, 0.482039137232547},        
        {0.209522442995634, 0.108872134366092, 0.873686250541582},        
        {0.091376754007102, 0.432874560106377, 0.46060482538636},
        {0.099706675595527, 0.959420719265034, 0.71281571676143},
        {0.934684749466345, 0.457964781814187, 0.277319652088188},        
        {0.289744386533678, 0.475310519058473, 0.160283028867265},        
        {0.61041733010124, 0.913035540638055, .882560694948989}, 
        {0.287841735186862, 0.0561812115307762, 0.28637401127652},
        {0.008838310763848, 0.91535707848396, 0.8164292832216},
        {0.116346871414582, 0.877979777046005, 0.014705205378353},        
        {0.277411672120203, 0.953316637720127, 0.705492149611232},        
        {0.705210590334554, 0.951415598863655, 0.337004898660462},        
        {0.069476201324669, 0.841603411752756, 0.00136731772498},
        {0.917187801632521, 0.0262881029625109, 0.19007638867168},
        };
    for (;i--;) {
        textures[i] = (Texture) {
            .color1=random_colors[i %32], 
            .color2=random_colors[i %32], 
            .color3=random_colors[i %32]
        };
    }

    return *nb_triangle;
}

#endif //SCENE_LOADER_H_
