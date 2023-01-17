#include "math/lib3D.h"
#include "render/color.h"

#define PI 3.14159
__constant const float CAM_XFOV_RAD = (70.0f * PI) / 180;
__constant const rgb colors[13] = {{1,0,0}, {0,1,0}, {0,0,1}, {1,1,0}, {0,1,1}, {1,1,1}, {0.5,0,0}, {0,0.5,0}, {0,0,0.5}, {0.5,0.5,0}, {0,0.5,0.5}, {0.5,0,0.5}, {0.5,0.5,0.5}};
    
__kernel void simpleCast (
        //in
        int x_res,
        int y_res,
        Point3 cam_point,
        Vector3 cam_dir,
        int nb_triangle,
        __global Triangle3* triangles,

        //in-middle
        __global float* z_buffer,
        //__global color* color_buffer,
        //__global Vector3* normal_buffer,
        //__global Point3* point_buffer,
        //__global int* obj_buffer,

        //out
        __global rgb* final_outut_buffer
        ) {
    int i;

    //get frame coordinates
    int x = get_global_id(0);
    int y = get_global_id(1);

    //init local var
    float z_value = -1;
    rgb color_value = {1, 0, 1};
    Vector3 normal_buffer = {0, 0, 0};
    Point3 point_buffer;
    int obj_buffer = -1;

    //TODO: prerender
    float step_x_rad = CAM_XFOV_RAD / x_res;

    //convert frame location to rotation
    float theta_x = step_x_rad * (x - x_res/2);
    float theta_y = step_x_rad * (y - y_res/2);

    //generate ray
    Vector3 vd_ray = cam_dir;
    vd_ray = rotateX(vd_ray, theta_x);
    vd_ray = rotateY(vd_ray, theta_y);
    Ray3 ray = newRay3(cam_point, vd_ray);


    for(i = 0; i<nb_triangle; i++) {
        //get collision with triangle
        Point3 global_pos;
        Vector3 local_pos;
        Vector3 normal;
        int hit;

        hit = getCollisionRayTriangle(triangles[i], ray, &global_pos, &local_pos, &normal);

        //update z_buffer
        if (hit) {
            float dist = getLength(global_pos, cam_point);
            if (z_value > dist || z_value == -1) {
                obj_buffer = i;
                z_value = dist;
                color_value = colors[obj_buffer %13];
                normal_buffer = normal;
                point_buffer = global_pos;
            }
        }
    }
    //output final z_value to zbuffer
    z_buffer[y*x_res + x] = z_value;

    //compute proof color
    float min = -1.0, max = 3.0;
    float grad = 1- (z_value - min) / (max - min);

    //compute hard shadows
    float light = 1;
    Vector3 directive_light = {0.6, 0.7, 1};//minused
    if (obj_buffer != -1) {
        Ray3 r = newRay3(point_buffer, directive_light);
        
        for(i = 0; i<nb_triangle; i++) {
            //get collision with triangle
            Point3 global_pos;Vector3 local_pos;Vector3 normal;
            int hit = getCollisionRayTriangle(triangles[i], r, &global_pos, &local_pos, &normal);

            if (hit) {
                light = 0;
                break;
            }
        }
        float angle = getAngle(normal_buffer, directive_light);
        if (angle > 0.5) {
            light = 0;
        } else {
            light *= angle;
        }
    }
    float a = 1.1;
    light = light/a +(1-1/a);
    light = -light*light + 2*light;

    final_outut_buffer[y*x_res + x].r = color_value.r * light;
    final_outut_buffer[y*x_res + x].g = color_value.g * light;
    final_outut_buffer[y*x_res + x].b = color_value.b * light;
} 
