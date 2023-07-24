#ifndef SCENE_LOADER_H_
#define SCENE_LOADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "../math/mVector3.h"
#include "../math/mMatrix3.h"
#include "../math/mObject3.h"
#include "../math/lib3D_debug.h"


int load_mtl_file(
        char* mlt_path,
        char* mlt_file_name,
        int* nb_material,
        Material** materials
    ) {

    FILE *fptr;
    size_t len1, len2;
    char* line = NULL;
    char* rest;
    char* identifier;
    char file_path[1000] = "\0";strcat(file_path, mlt_path);strcat(file_path, mlt_file_name);

    fptr = fopen(file_path,"r");
    if (fptr == NULL) {
        printf(" ##### /!\\ Mlt file not found ! /!\\ #####  (path: %s)\n", file_path);
        return 0;
    }
    //printf("Loading mlt file : %s \n", mlt_file_name);

    *nb_material = 0;
    int material_current_index = 0;

    int nb_material_size = 8;
    *materials = malloc(sizeof(Material) * nb_material_size);

    while ((len2 = getline(&line, &len1, fptr)) != -1) {
        EE_FLOAT r, g, b;
        if (line[0] == '\n') {
        } else if (line[0] == '#') {
        } else if (strncmp(line, "newmtl", 6) == 0) {
            material_current_index = *(nb_material);
            (*nb_material)++;
            strncpy((*materials)[material_current_index].name, line + 7, 200);
            (*materials)[material_current_index].name[strlen((*materials)[material_current_index].name) -1] = '\0';
            //printf("Material: %s\n", (*materials)[material_current_index].name);
        } else if (strncmp(line, "Kd", 2) == 0) {
            sscanf(line, "Kd %f %f %f", &r, &g, &b);
            (*materials)[material_current_index].Kd = (rgb) {r, g, b, 1};
            //printf("Color base: %f %f %f\n", r, g, b);
        } else if (strncmp(line, "Ke", 2) == 0) {
            sscanf(line, "Ke %f %f %f", &r, &g, &b);
            (*materials)[material_current_index].Ke = (rgb) {r, g, b, 1};
            //printf("Color emmision: %f %f% f\n", r, g, b);
        } else if (strncmp(line, "map_Kd", 6) == 0) {
            char path[200];
            strncpy(path, line + 7, 200);
            path[strlen(path) -1] = '\0';

            strncpy((*materials)[material_current_index].texture_path, path, 200);
            (*materials)[material_current_index].hasTexture = 1;
            /**(*materials)[material_current_index].map_location = (Texture) {
                .v_off = (cl_int2) {0, 0},
                .v_size= (cl_int2) {32, 32},
            };**/
            
            //printf("Material (%i) has texture: %s\n", material_current_index, path);
        } else if (strncmp(line, "Ns", 2) == 0) {
        } else if (strncmp(line, "Ka", 2) == 0) {
        } else if (strncmp(line, "Ks", 2) == 0) {
        } else if (strncmp(line, "Ni", 2) == 0) {
        } else if (strncmp(line, "illum", 5) == 0) {
        } else if (strncmp(line, "d ", 2) == 0) {
        } else {
            //printf("Unkown parameter [%.*s]\n", (int) len2-1, line);
        }
        

        if (*nb_material >= nb_material_size) {
            nb_material_size*=2;
            *materials = (Material*) realloc(*materials, nb_material_size * sizeof(Material));
            printf("#info: increase material buffer by *2 (%i)\n", nb_material_size);
        }
    }

    printf("loaded : %s -> %i materials\n", mlt_file_name, *nb_material);

    fclose(fptr);
    if (line)
        free(line);

    return 1;
}

int load_obj_file(
        char* obj_path,
        char* obj_file_name,
        int* triangle_index, Triangle3** triangles,UV** texture_uv,
        int* object_index, Object** objects,
        int* material_index, Material** materials
    ) {

    FILE *fptr;
    size_t len1, len2;
    char* line = NULL;
    char* rest;
    char* identifier;
    int i;

    char file_path[1000] = "\0";strcat(file_path, obj_path);strcat(file_path, obj_file_name);

    fptr = fopen(file_path,"r");
    if (fptr == NULL) {
        printf(" ##### /!\\ Obj file not found ! /!\\ #####  (path: %s)\n", file_path);
        return 0;
    }
    //printf("Loading obj file : %s \n", obj_file_name);

    

    Point3* point_buffer;
    EE_FLOAT2* texture_buffer;
    EE_FLOAT3* normal_buffer;

    int nb_triangle_size = 512;
    int nb_texture_size = 512;
    int nb_object_size = 8;
    int point_buffer_size = 512;
    int texture_buffer_size = 512;
    int normal_buffer_size = 512;
    
    *triangles      = (Triangle3*)  malloc(nb_triangle_size     * sizeof(Triangle3));
    *texture_uv     = (UV*)    malloc(nb_texture_size      * sizeof(UV));
    *objects        = (Object*)     malloc(nb_object_size       * sizeof(Object));
    point_buffer    = (Point3*)     malloc(point_buffer_size    * sizeof(Point3));
    texture_buffer  = (EE_FLOAT2*)  malloc(texture_buffer_size  * sizeof(EE_FLOAT2));
    normal_buffer   = (EE_FLOAT3*)  malloc(normal_buffer_size   * sizeof(EE_FLOAT3));

    texture_buffer[0] = (EE_FLOAT2) {(0,0)};//set index 0 for undefined and array shift
    normal_buffer[0] = (EE_FLOAT3) {(0,0,0)};//set index 0 for undefined and array shift
    *material_index = 1;
    *materials = (Material*) malloc(*material_index * sizeof(Material));
    (*materials)[0] = (Material) {
        .Kd = (rgb) {1, 1, 1, 1},
        .Ke = (rgb) {0, 0, 0, 1},
        .hasTexture = 0
    };

    *triangle_index = 0;
    *object_index = 0;
    int texture_uv_index = 0;
    int point_index = 0;
    int texture_coo_index = 1;
    int normal_index = 1;

    while ((len2 = getline(&line, &len1, fptr)) != -1) {
        EE_FLOAT x, y, z, c1, c2;
        switch (line[0]) {
        case '#':break;
        case '\n':break;
        case 'o':case 'g':
            //(*objects)[*object_index]->mat = 0;
            (*objects)[*object_index].t_buffer_start = *triangle_index;
            if (*object_index>0) (*objects)[*object_index-1].t_buffer_end = *triangle_index -1;
            (*object_index)++;
            //printf("object (%i) -> start:%i,  end:%i\n", (*object_index-2), (*objects)[*object_index-2].t_buffer_start, (*objects)[*object_index-2].t_buffer_end);
            break;
        case 'm':
            if (strncmp(line, "mtllib", 6) == 0) {
                char path[400];
                strncpy(path, line + 7, 400);
                path[strlen(path) -1] = '\0';
                load_mtl_file(obj_path, path, material_index, materials);
            } else {
                printf("Unkown parameter [%.*s]\n", (int) len2-1, line);
            }
            break;
        case 'u':
            if (strncmp(line, "usemtl", 6) == 0) {
                cl_char material_name[200];
                strncpy(material_name, line + 7, 200);
                material_name[strlen(material_name) -1] = '\0';
                (*objects)[*object_index-1].material_index = -1;

                for (i=0; i<*material_index; i++) {
                    if (strcmp((*materials)[i].name, material_name) == 0) {
                        //printf("[%s]:[%s] -> %i\n", (*materials)[i].name, material_name, i);
                        (*objects)[*object_index-1].material_index = i;
                        goto break1;
                    }
                }
                printf("Unkown Material [%s]\n", material_name);
                break1:
            } else {
                printf("Unkown parameter [%.*s]\n", (int) len2-1, line);
            }
            break;
        case 'v':
            switch (line[1]) {
                case ' ':
                    sscanf(line, "v %f %f %f", &x, &y, &z);
                    point_buffer[point_index++] = (Point3) {x, y, z};
                    break;
                case 't':
                    sscanf(line, "vt %f %f", &c1, &c2);
                    texture_buffer[texture_coo_index++] = (EE_FLOAT2) {c1, c2};
                    break;
                case 'n':
                    sscanf(line, "vn %f %f %f", &x, &y, &z);
                    normal_buffer[normal_index++] = (EE_FLOAT3) {x, y, z};
                    break;
                default:
                    printf("Unkown parameter [%.*s]\n", (int) len2-1, line);
                    break;
            }
            break;
        case 'f':
            EE_INT v1, v2, v3;
            EE_INT vt1=0, vt2=0, vt3=0;
            EE_INT vn1=0, vn2=0, vn3=0;

            int ret=0;
            if (!ret) ret = 9==sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);
            if (!ret) ret = 6==sscanf(line, "f %d//%d %d//%d %d//%d", &v1, &vn1, &v2, &vn2, &v3, &vn3);
            if (!ret) ret = 6==sscanf(line, "f %d/%d/ %d/%d/ %d/%d/", &v1, &vt1, &v2, &vt2, &v3, &vt3);
            if (!ret) ret = 3==sscanf(line, "f %d// %d// %d//", &v1, &v2, &v3);
            if (!ret) ret = 3==sscanf(line, "f %d %d %d", &v1, &v2, &v3);
            if (!ret) printf("Error while parsing triangle [%s]\n",line);

            EE_FLOAT2 voff = texture_buffer[vt2];
            EE_FLOAT2 vty = {texture_buffer[vt1].x-voff.x, texture_buffer[vt1].y-voff.y};
            EE_FLOAT2 vtx = {texture_buffer[vt3].x-voff.x, texture_buffer[vt3].y-voff.y};
        
            (*texture_uv)[texture_uv_index++] = (UV) {.v1=vtx,.v2=vty,.voff=voff};
            (*triangles)[(*triangle_index)++] = newTriangle3(point_buffer[v2-1], point_buffer[v3-1], point_buffer[v1-1]);
            break;
        case 's':break;
        default:
            printf("Unkown parameter [%.*s]\n", (int) len2-1, line);
            break;
        }

        if (*triangle_index >= nb_triangle_size) {
            nb_triangle_size*=2;
            *triangles = (Triangle3*) realloc(*triangles, nb_triangle_size * sizeof(Triangle3));
            printf("#info: increase triangle buffer by *2 (%i)\n", nb_triangle_size);
        }
        
        if (texture_uv_index >= nb_texture_size) {
            nb_texture_size*=2;
            *texture_uv = (UV*) realloc(*texture_uv, nb_texture_size * sizeof(UV));
            printf("#info: increase texture buffer by *2 (%i)\n", nb_texture_size);
        }

        if (*object_index >= nb_object_size) {
            nb_object_size*=2;
            *objects = (Object*) realloc(*objects, nb_object_size * sizeof(Object));
            printf("#info: increase object buffer by *2 (%i)\n", nb_object_size);
        }

        if (point_index >= point_buffer_size) {
            point_buffer_size*=2;
            point_buffer = (Point3*) realloc(point_buffer, point_buffer_size * sizeof(Point3));
            printf("#info: increase point buffer by *2 (%i)\n", point_buffer_size);
        }

        if (texture_coo_index >= texture_buffer_size) {
            texture_buffer_size*=2;
            texture_buffer = (EE_FLOAT2*) realloc(texture_buffer, texture_buffer_size * sizeof(EE_FLOAT2));
            printf("#info: increase texture coo buffer by *2 (%i)\n", texture_buffer_size);
        }

        if (normal_index >= normal_buffer_size) {
            normal_buffer_size*=2;
            normal_buffer = (EE_FLOAT3*) realloc(normal_buffer, normal_buffer_size * sizeof(EE_FLOAT3));
            printf("#info: increase normale buffer by *2 (%i)\n", normal_buffer_size);
        }
    }
    (*objects)[*object_index-1].t_buffer_end = *triangle_index -1;
           

    printf("loaded : %s -> triangles : %i, points : %i, UVs : %i\n", obj_file_name, *triangle_index, point_index, texture_coo_index);

    fclose(fptr);
    if (line)
        free(line);

    return 1;
}

//TODO: should be rethinked
int load_scene_context(
        int* nb_lights,
        LightSource3* lights,
        Point3* cam_coordinate,
        Vector3* cam_lookat,
        Vector3* sky_light_dir
    ) {

    *cam_coordinate = (Point3) {0.427321, 0.000000, 1.283136};
    *cam_lookat = (Vector3) {-0.469893, -0.198173, -0.860133};
    //*cam_coordinate = (Point3) {0, 0, 2.0};
    //*cam_lookat = (Vector3) {0, 0, -1};
    *sky_light_dir = (Vector3) {-0.7, -1, -0.5};

    *nb_lights = 0;
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {0,0,1},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {7,2,3},
        .luminosity = 12
    };
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {1,0,0},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {0,2,7},
        .luminosity = 12
    };
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {0,1,0},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {-7,2,3},
        .luminosity = 12
    };

    return 1;
}



//TODO: To be deleted (c'était drole mais bon à un moment faut stop)
int loadCubeScene(
        char* path,  
        int* nb_triangle,
        Triangle3* triangles,
        UV* texture_uv,
        int* nb_lights,
        LightSource3* lights,
        Point3* cam_coordinate,
        Vector3* cam_lookat,
        Vector3* sky_light_dir,
        UV* sky_light_texture
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
        texture_uv[i] = (UV) {
            .v1=vtx,.v2=vty,.voff=voff
        };
    }
    *nb_lights = 0;
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {0.80,0.75,0.77},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {0,alphaz+k/2, 0},
        .luminosity = 12
    };
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {cos(_rad), cos(_rad+3.14*0.666), cos(_rad+3.14*1.333)},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {cos_alpha*k*2,0.1,sin_alpha*k*2},
        .luminosity = 12
    };
    lights[(*nb_lights)++] = (LightSource3) {
        .color = (rgb) {sin(_rad), sin(_rad+3.14*0.666), sin(_rad+3.14*1.333)},
        .dir = (Vector3) {0,0,0},
        .source = (Point3) {cos_alpha*k*2,0.1,cos_alpha*k*2},
        .luminosity = 12
    };
    
    return *nb_triangle;
}

//TODO: To be deleted (c'était drole mais bon à un moment faut stop)
int loadMaxwellScene(
        char* path,
        Point3* cam_coordinate,
        Vector3* cam_lookat
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


    //*nb_lights = 1;
    *cam_coordinate = (Point3) {sin_alpha*10, 3, cos_alpha*10};
    *cam_lookat = getNorm((Vector3) {-cam_coordinate->x, -cam_coordinate->y, -cam_coordinate->z});
    
    return 1;
}

#endif //SCENE_LOADER_H_
