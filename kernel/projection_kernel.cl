#include "math/lib3D.h"
#include "render/color.h"

__constant const float CAM_XFOV_RAD = (80.0f * 3.14159) / 180;

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
    
    //get frame coordinates
    int x = get_global_id(0);
    int y = get_global_id(1);

    //init local var
    float z_value = -1;
    rgb color_value = {255, 0, 255};
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


    int i;for(i = 0; i<nb_triangle; i++) {
        //get collision with triangle
        Point3 global_pos = collisionRayPlane(triangles[i].pl, ray);
        Point3 pl_p_local = triangles[0].pl.p;
        Vector3* off1 = (Vector3*) &pl_p_local;//hack conversion from point to vector
        
        //local position in triangle
        Vector3 local_pos = getLocalPosition(triangles[i].base, *off1, global_pos);

        //does hit or not
        int hit = 0 < local_pos.x;
        hit &= 0 < local_pos.y;
        hit &= local_pos.x + local_pos.y <= 1;

        //update z_buffer
        if (hit) {
            float dist = getLength(global_pos, cam_point);
            if (z_value > dist || z_value == -1) {
                z_value = dist;
                //color_value = {255, 0, 255};
                //normal_buffer = {0, 0, 0};
                point_buffer = global_pos;
                obj_buffer = i;
            }
        }
    }
    //output final z_value to zbuffer
    z_buffer[y*x_res + x] = z_value;

    //output color
    float min = -1.0, max = 11.0;
    float grad = 1- (z_value - min) / (max - min);
    final_outut_buffer[y*x_res + x].r = grad * 255;
    final_outut_buffer[y*x_res + x].g = grad * 255;
    final_outut_buffer[y*x_res + x].b = grad * 255;
} 
