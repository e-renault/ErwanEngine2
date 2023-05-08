#include "math/lib3D.h"
#include "render/color.h"
#include "render/texture.h"
#include "light/lightSource.h"

__constant const Vector3 up = {0, 1, 0};//constant vector
#define PI 3.14159

__kernel void simpleCast (
        int x_res,
        int y_res,
        float FOV,
        __global rgb* final_outut_buffer,

        int nb_triangle,
        __constant Triangle3* triangles,
        __constant Texture* textures,
        int nb_lights,
        __constant LightSource3* lights,

        Vector3 sky_light_dir,
        Texture sky_color,

        Point3 cam_point,
        Vector3 cam_dir
        ) {
    int i, j;


    /*********** Init frame ***********/
    //get frame coordinates
    int x = get_global_id(0);
    int y = get_global_id(1);

    //temp
    const float CAM_XFOV_RAD = FOV * (PI / 180);
    float step_x_rad = CAM_XFOV_RAD / x_res;//TODO: prerender

    //init local var
    float z_value_buffer = FLT_MAX;
    Vector3 normal_buffer = {0, 0, 0};
    Point3 point_buffer;
    rgb color_value;
    int obj_buffer = -1;

    //convert frame location to rotation
    float theta_x = step_x_rad * (x - x_res/2);
    float theta_y = step_x_rad * (y - y_res/2);



    /*********** Init cam ***********/
    Vector3 vd_ray = cam_dir;
    vd_ray = rotateAround(vd_ray, up, theta_x);
    
    Vector3 right = crossProduct(cam_dir, up);
    vd_ray = rotateAround(vd_ray, right, theta_y);

    Ray3 ray = (Ray3){.p=cam_point, .v=vd_ray};
    
    /*********** Sky color ***********/
    float theta = getAngle(vd_ray, up);

    if (theta > 0.5){
        float heigth = 2*(theta - 0.5);
        color_value = getColor(sky_color, heigth, 0);
    } else {
        float heigth = 1- 2*theta;
        color_value = getColor(sky_color, 0, heigth);
    }


    /*********** first cast ***********/
    for(i = 0; i<nb_triangle; i++) {
        //get collision with triangle
        Point3 global_pos;Vector3 local_pos;Vector3 normal;float d;
        int hit = getCollisionRayTriangle(triangles[i], ray, &global_pos, &local_pos, &normal, &d);

        //update z_buffer
        if (hit) {
            float dist = getLength(global_pos, cam_point);
            if (z_value_buffer > dist) {
                obj_buffer = i;
                z_value_buffer = dist;
                normal_buffer = normal;
                point_buffer = global_pos;
                color_value = getColor(textures[i], local_pos.x, local_pos.y);
            }
        }
    }

    /*********** hard shadows ***********/
    rgb direct_light = sky_color.color2;
    if (obj_buffer != -1) {
        Ray3 r = (Ray3) {.p=point_buffer, .v=sky_light_dir};
        
        for(i = 0; i<nb_triangle; i++) {
            Point3 global_pos;Vector3 local_pos;Vector3 normal;float d;
            int hit = getCollisionRayTriangle(triangles[i], r, &global_pos, &local_pos, &normal, &d);

            if (hit) {
                direct_light = scaling_substractive(direct_light, 0);
                break;
            }
        }
        float angle = getAngle(normal_buffer, sky_light_dir);
        direct_light = scaling_substractive(direct_light, angle < 0.5 ? (1-angle*2):0);
    }

    /*********** soft shadows ***********/
    rgb scene_light = {0,0,0};
    if (obj_buffer != -1) {
        for(j = 0; j<nb_lights; j++) {
            Vector3 v = newVector(point_buffer, lights[j].source);
            Ray3 r = (Ray3) {.p=point_buffer, .v=v};
            
            float dist = getLength(lights[j].source, point_buffer);
            if (dist>lights[j].intensity) break;

            int hit = 0;
            for(i = 0; i<nb_triangle; i++) {
                Point3 global_pos;Vector3 local_pos;Vector3 normal;float d;
                hit &= getCollisionRayTriangle(triangles[i], r, &global_pos, &local_pos, &normal, &d);
                if (hit) break;
            }
                
            if (!hit) {
                float angle = getAngle(normal_buffer, v);
                rgb temp_light = lights[j].color;
                temp_light = scaling_substractive(temp_light, angle < 0.5 ? (1-angle*2):0);
                temp_light = scaling_substractive(temp_light, 1-(dist/lights[j].intensity));
                scene_light = synthese_additive(scene_light, temp_light);
            }
        }
    }

    rgb lignt_sum = synthese_additive(direct_light, scene_light);
    final_outut_buffer[y*x_res + x] = min_color(lignt_sum, color_value);
} 
