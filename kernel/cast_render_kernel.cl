#include "kernel/mObject3_collision.h"
#include "kernel/texture.h"
#include "kernel/color.h"

__constant const Vector3 up = {0, 1, 0};//constant vector
#define PI 3.14159

__kernel void simpleCast (
        int x_res,
        int y_res,
        float FOV,

        //__global float* z_value_buffer,
        //__global Vector3* normal_buffer,
        //__global Point3* point_buffer,
        //__global rgb* color_value_buffer,
        //__global int* obj_buffer,
        //__global Ray3* ray_buffer,
        //__global rgb* direct_light_buffer,
        //__global rgb* scene_light_buffer,

        __write_only image2d_t final_outut_buffer,

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
    float step_x_rad = CAM_XFOV_RAD / x_res;

    //init local var
    float z_value_buffer = FLT_MAX;
    Vector3 normal_buffer;
    Point3 point_buffer;
    rgb color_value_buffer;
    int obj_buffer = -1;

    //convert frame location to rotation
    float theta_x = step_x_rad * (x - x_res/2);
    float theta_y = step_x_rad * (y - y_res/2);



    /*********** Init cam ***********/
    Vector3 vd_ray = cam_dir;
    vd_ray = rotateAround(vd_ray, up, theta_x);
    
    Vector3 right = crossProduct(vd_ray, up);
    vd_ray = rotateAround(vd_ray, right, theta_y);

    Ray3 ray = (Ray3){.p=cam_point, .v=vd_ray};
    
    /*********** Sky color ***********/
    float theta = getAngle(vd_ray, up);

    if (theta > 0.5){
        float heigth = 2*(theta - 0.5);
        color_value_buffer = getColor(sky_color, heigth, 0);
    } else {
        float heigth = 1- 2*theta;
        color_value_buffer = getColor(sky_color, 0, heigth);
    }

    /*********** first cast ***********/
    for(i = 0; i<nb_triangle; i++) {
        //get collision with triangle
        Point3 global_pos;Vector3 local_pos;Vector3 normal;float dist;
        int hit = getCollisionRayTriangle(triangles[i], ray, z_value_buffer, &global_pos, &local_pos, &normal, &dist);

        //update z_buffer
        if (hit) {
            obj_buffer = i;
            z_value_buffer = dist;
            normal_buffer = normal;
            point_buffer = global_pos;
            color_value_buffer = getColor(textures[i], local_pos.x, local_pos.y);
        }
    }

    /*********** hard shadows ***********/
    rgb direct_light_buffer = sky_color.color2;
    if (obj_buffer != -1) {
        Ray3 r = (Ray3) {.p=point_buffer, .v=-sky_light_dir};
        
        for(i = 0; i<nb_triangle; i++) {
            Point3 global_pos;Vector3 local_pos;Vector3 normal;float d;
            int hit = getCollisionRayTriangle(triangles[i], r, FLT_MAX, &global_pos, &local_pos, &normal, &d);

            if (hit) {
                direct_light_buffer = (direct_light_buffer * 0);
                break;
            }
        }
        float angle = getAngle(normal_buffer, -sky_light_dir);
        direct_light_buffer = (direct_light_buffer * (angle<0.5 ? (1-angle*2):0));
    }

    /*********** soft shadows ***********/
    rgb scene_light_buffer = (rgb) {0,0,0};
    if (obj_buffer != -1) {
        for(j = 0; j<nb_lights; j++) {
            Vector3 v = getNorm(newVector(point_buffer, lights[j].source));
            Ray3 r = (Ray3) {.p=point_buffer, .v=v};
            
            float max_dist = getLength(lights[j].source, point_buffer);
            //if (lights[j].intensity < max_dist) break;

            int hit = 0;
            for(i = 0; i<nb_triangle; i++) {
                Point3 global_pos;Vector3 local_pos;Vector3 normal;float d;
                hit = getCollisionRayTriangle(triangles[i], r, max_dist, &global_pos, &local_pos, &normal, &d);
                if (hit) break;
            }
                
            if (!hit) {
                float angle = getAngle(normal_buffer, v);
                rgb temp_light = lights[j].color;
                temp_light = (temp_light * (angle < 0.5 ? (1-angle*2):0));
                temp_light = (temp_light * (1-(max_dist/lights[j].intensity)));
                temp_light = cap(temp_light);
                scene_light_buffer = (scene_light_buffer + temp_light);
            }
        }
    }

    /* Build up final render */
    rgb lignt_sum = cap((direct_light_buffer + scene_light_buffer));
    //final_outut_buffer[y*x_res + x] = scene_light_buffer; //neon render
    rgb c = min(direct_light_buffer, color_value_buffer);//only sun illum
    //final_outut_buffer[y*x_res + x] = min(lignt_sum, color_value_buffer);//all in one render

    uint4 color = (uint4)(
        c.x*255, 
        c.y*255, 
        c.z*255, 
        255
    ); // Red color
    write_imageui(final_outut_buffer, (int2)(x, y), color);
} 