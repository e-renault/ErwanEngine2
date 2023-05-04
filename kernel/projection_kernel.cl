#include "math/lib3D.h"
#include "render/color.h"

#define PI 3.14159
__constant const float CAM_XFOV_RAD = 70.0f * (PI / 180);
__constant const rgb colors[13] = {{1,0,0}, {0,1,0}, {0,0,1}, {1,1,0}, {0,1,1}, {1,1,1}, {0.5,0,0}, {0,0.5,0}, {0,0,0.5}, {0.5,0.5,0}, {0,0.5,0.5}, {0.5,0,0.5}, {0.5,0.5,0.5}};
__constant const Vector3 pseudo_aleat_vec[50] = {{0.093,0.026,0.101},{-0.264,0.194,0.073},{0.182,-0.175,0.157},{-0.033,-0.014,0.065},{0.123,-0.158,0.076},{0.045,0.154,0.209},{-0.049,0.238,-0.176},{0.097,-0.146,0.031},{0.07,-0.183,-0.15},{-0.288,0.121,0.089},{0.202,-0.128,-0.037},{0.155,-0.118,0.279},{0.209,0.271,0.072},{-0.265,-0.289,0.129},{-0.279,-0.138,0.143},{0.025,0.067,0.161},{0.249,0.117,-0.281},{0.297,0.115,0.272},{-0.289,-0.022,0.216},{0.045,0.19,-0.014},{0.128,-0.17,0.289},{0.132,-0.056,-0.262},{-0.041,-0.168,0.02},{0.121,-0.092,0.213},{0.146,-0.157,-0.053},{-0.026,0.065,0.174},{0.046,0.04,-0.216},{0.2,0.151,-0.21},{0.106,-0.29,-0.257},{0.009,-0.239,0.13},{0.119,0.221,-0.13},{0.028,0.16,0.035},{-0.255,0.269,-0.129},{-0.182,0.222,0.299},{-0.241,0.017,-0.133},{0.028,-0.274,0.106},{0.067,-0.033,0.094},{-0.145,-0.057,-0.225},{0.054,-0.243,0.08},{0.159,0.216,-0.205},{-0.121,-0.269,-0.119},{-0.022,0.268,-0.057},{0.163,0.178,-0.184},{-0.053,-0.216,-0.011},{0.119,-0.208,0.026},{0.1,-0.214,-0.266},{0.175,-0.099,-0.296},{0.029,0.025,-0.284},{0.237,-0.169,0.23},{-0.016,0.075,0.166}};
    
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
    int i, j;

    //get frame coordinates
    int x = get_global_id(0);
    int y = get_global_id(1);

    //init local var
    float z_value = -1;
    rgb color_value = {0.75, 0.87, 0.93};
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
    Vector3 up = {0, 1, 0};
    vd_ray = rotateAround(vd_ray, up, theta_x);
    
    Vector3 right = crossProduct(cam_dir, up);
    vd_ray = rotateAround(vd_ray, right, theta_y);

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
                normal_buffer = normal;
                point_buffer = global_pos;
                color_value = colors[obj_buffer %13];
            }
        }
    }
    //output final z_value to zbuffer
    z_buffer[y*x_res + x] = z_value;

    //compute proof color
    float min = -1.0, max = 3.0;
    float grad = 1- (z_value - min) / (max - min);

    //compute hard shadows
    float direct_light = 1;
    Vector3 directive_light_v = {0.6, 0.7, 1};//minused
    if (obj_buffer != -1) {
        Ray3 r = newRay3(point_buffer, directive_light_v);
        
        for(i = 0; i<nb_triangle; i++) {
            //get collision with triangle
            Point3 global_pos;Vector3 local_pos;Vector3 normal;
            int hit = getCollisionRayTriangle(triangles[i], r, &global_pos, &local_pos, &normal);

            if (hit) {
                direct_light = 0;
                break;
            }
        }
        float angle = getAngle(normal_buffer, directive_light_v);
        if (angle > 0.5) {
            direct_light = 0;
        } else {
            direct_light *= angle;
        }
    }
    float a = 1.1;
    direct_light = direct_light/a +(1-1/a);
    direct_light = -direct_light*direct_light + 2*direct_light;


    int SSAO_nb_iteraton = 30;
    float SSAO_light = 1;
    if (obj_buffer != -1) {
        int counter = SSAO_nb_iteraton;
        for (j=0; j<SSAO_nb_iteraton; j++) {
            float entropy = z_value * theta_x * theta_y * 100000000;

            Vector3 v = rotateAround(pseudo_aleat_vec[j], vd_ray, entropy);
            v = rotateAround(v, up, entropy);

            Ray3 r = newRay3(point_buffer, v);
            for(i = 0; i<nb_triangle; i++) {
                Point3 global_pos;Vector3 local_pos;Vector3 normal;
                int hit = getCollisionRayTriangle(triangles[i], r, &global_pos, &local_pos, &normal);
                if (hit && getLength(global_pos, point_buffer) < 0.2) {
                    counter--;
                    break;
                }
                    
            }
        }
        SSAO_light = (float) counter/SSAO_nb_iteraton;
    }

    final_outut_buffer[y*x_res + x].r = SSAO_light * direct_light * color_value.r;
    final_outut_buffer[y*x_res + x].g = SSAO_light * direct_light * color_value.g;
    final_outut_buffer[y*x_res + x].b = SSAO_light * direct_light * color_value.b;
} 
