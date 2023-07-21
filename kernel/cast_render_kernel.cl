#include "kernel/kObject3.h"
#include "kernel/kTexture.h"
#include "kernel/kColor.h"
#include "kernel/kRandom.h"

/**__kernel void rayCast ( ... ) {
    int i, j;

    *********** Init cam ***********
    Vector3 vd_ray = cam_dir;
    Vector3 right = crossProduct(cam_dir, UP);
    vd_ray = rotateAround(vd_ray, right, theta_y);
    
    Vector3 new_up = crossProduct(vd_ray, right);
    vd_ray = rotateAround(vd_ray, new_up, theta_x);

    Ray3 ray = (Ray3){.p=cam_point, .v=vd_ray};


    *********** soft shadows ***********
    rgb scene_light_buffer = (rgb) {0,0,0, 1};
    if (obj_buffer != -1) {
        for(j = 0; j<nb_lights; j++) {
            Vector3 v = getNorm(lights[j].source - point_buffer);
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
} **/

rgb max_rgb(rgb c1, rgb c2);
rgb max_rgb(rgb c1, rgb c2) {
    rgb ret = (rgb) {
        fmax(c1.x, c2.x),
        fmax(c1.y, c2.y),
        fmax(c1.z, c2.z),
        fmax(c1.w, c2.w)
    };
    return ret;
}

int coord(int x, int y, int x_res, int y_res) {
    x = x < 0 ? -x : x;
    y = y < 0 ? -y : y;
    x = x > x_res ? x_res-(x%x_res) : x;
    y = y > y_res ? y_res-(y%y_res) : y;
    return y * x_res + x;
}

__constant rgb sky_illum_color0 = (rgb) {1,     1,     1,      1};
__constant rgb sky_color_color1 = (rgb) {0.58,  0.78,  0.92,   1};// * sky_illum_color0;
__constant rgb sky_color_color2 = (rgb) {1.3,   1.3,   1.3,    1};// * sky_illum_color0;
__constant rgb sky_color_color3 = (rgb) {0,     0,     1,      1};// * sky_illum_color0;

__kernel void rayTrace (
        int x_res,
        int y_res,
        float FOV,

        __write_only image2d_t final_outut_buffer,
        __read_only image2d_t texture_map,
        __read_only image2d_t normal_map,

        int nb_triangle, __constant Triangle3* triangles, __constant Texture* uv_map,
        int nb_object, __constant Object* objects,
        int nb_material, __constant Material* materials,

        int nb_lights, __constant LightSource3* lights,


        Vector3 sky_light_dir,
        Point3 cam_point,
        Vector3 cam_dir,

        __global LocalPixelData* pixel,

        int iteration_count,
        unsigned int random_seed


        ) {

    /*********** Init frame ***********/
    //get frame coordinates
    int x = get_global_id(0);
    int y = get_global_id(1);
    int pos = y * x_res + x;
    int mid = y_res/2 * x_res + x_res/2;

    //random
    unsigned int random=random_seed;//random_int(pos, &random);


    if (iteration_count != 0) goto EEAO;//(= Erwan Engine Ambiant Occlusion)

    //init local var
    pixel[pos].z_value_buffer = FLT_MAX;
    pixel[pos].triangle_index = -1;
    pixel[pos].object_index = -1;


    /*********** Init cam ***********/

    //convert frame location to rotation
    const float CAM_XFOV_RAD = FOV * (PI / 180);
    float step_x_rad = CAM_XFOV_RAD / x_res;
    float theta_x = step_x_rad * (x - x_res/2);
    float theta_y = step_x_rad * (y - y_res/2);

    Vector3 vd_ray = cam_dir;
    
    Vector3 right = crossProduct(cam_dir, UP);
    vd_ray = rotateAround(vd_ray, right, theta_y);
    
    Vector3 new_up = crossProduct(cam_dir, right);
    vd_ray = rotateAround(vd_ray, new_up, theta_x);

    Ray3 ray = (Ray3){.p=cam_point, .v=vd_ray};
    

    /*********** Sky color ***********/
    float theta = getAngle(vd_ray, UP);

    if (theta > 0.5){
        pixel[pos].color_value_buffer = (sky_color_color2 * (2*theta - 1)) + (sky_color_color1 * (2- 2*theta));
    } else {
        pixel[pos].color_value_buffer = (sky_color_color3 * (1- 2*theta)) + (sky_color_color1 * (2*theta));
    }
    


    /*********** 1st cast ***********/
    for(int obj_i = 0; obj_i<nb_object; obj_i++) {
        int material_index = objects[obj_i].material_index;

        for(int i = objects[obj_i].t_buffer_start; i<=objects[obj_i].t_buffer_end; i++) {
            //get collision with triangle
            Point3 global_pos;Vector3 local_pos;Vector3 normal;float dist;
            int hit = getCollisionRayTriangle(triangles[i], ray, pixel[pos].z_value_buffer, &global_pos, &local_pos, &normal, &dist);
            
            
            //update buffers
            rgb color = materials[material_index].hasTexture ? 
                getColor(uv_map[i], texture_map, local_pos.x, local_pos.y) 
                : materials[material_index].Kd;
            if (hit && !(color.x == 1 && color.y == 0 && color.z == 1)) {
                pixel[pos].triangle_index = i;
                pixel[pos].z_value_buffer = dist;
                pixel[pos].normal_buffer = normal;
                pixel[pos].point_buffer = global_pos;
                pixel[pos].color_value_buffer = color;
                pixel[pos].object_index = obj_i;
                pixel[pos].material_index = material_index;
            }
        }
    }
    
    /*********** hard shadows ***********/
    pixel[pos].direct_light_buffer = sky_illum_color0;
    if (pixel[pos].triangle_index != -1) {
        Ray3 r = (Ray3) {.p=pixel[pos].point_buffer, .v=-sky_light_dir};
        
        for(int obj_i = 0; obj_i<nb_object; obj_i++) {
            for(int i = objects[obj_i].t_buffer_start; i<=objects[obj_i].t_buffer_end; i++) {
                Point3 global_pos;Vector3 local_pos;Vector3 normal;float dist;
                int hit = getCollisionRayTriangle(triangles[i], r, FLT_MAX, &global_pos, &local_pos, &normal, &dist);
                
                if (hit) {
                    pixel[pos].direct_light_buffer = (pixel[pos].direct_light_buffer * 0);
                    goto break_hard_shadow_collision_detection;
                }
            }
        }break_hard_shadow_collision_detection:;
        float angle = getAngle(pixel[pos].normal_buffer, -sky_light_dir);
        pixel[pos].direct_light_buffer = (pixel[pos].direct_light_buffer * (angle<0.5 ? (1-angle*2):0));
    }

    /*********** EEAO ***********/
    EEAO:;
    if (pixel[pos].triangle_index != -1) {
        Vector3 v1;
        if (!isColinear(pixel[pos].normal_buffer, UP)) {
            v1 = getNorm2(pixel[pos].normal_buffer, UP);
        } else {
            v1 = getNorm2(pixel[pos].normal_buffer, RIGHT);
        }

        rgb illum_increment = materials[pixel[pos].material_index].Ke;
        int hit = 0;
        int mat_id;
        int nb_iteration = 1;
        for (int cast_count=0; cast_count <nb_iteration; cast_count++) {
            float x = random_float(pos, &random);
            float a = 0.517f, b=1.1f;
            float y = pow(x, a*(b-x));
            Vector3 new_vd_ray = rotateAround(pixel[pos].normal_buffer, v1, y * (PI/2));
            new_vd_ray = rotateAround(new_vd_ray, pixel[pos].normal_buffer, random_float(pos, &random) * 2 * PI);
            Ray3 new_ray = (Ray3){.p=pixel[pos].point_buffer, .v=new_vd_ray};

            Point3 global_pos;Vector3 local_pos, normal;float dist;float max_z = 5.0;int hitted;
            for(int obj_i = 0; obj_i<nb_object; obj_i++) {
                for(int i = objects[obj_i].t_buffer_start; i<=objects[obj_i].t_buffer_end; i++) {
                    hitted = getCollisionRayTriangle(triangles[i], new_ray, max_z, &global_pos, &local_pos, &normal, &dist);
                    
                    if (hitted) {
                        mat_id = objects[obj_i].material_index;
                        max_z = dist;
                        hit |= hitted;
                    }
                }
            }
            if (hit) {
                illum_increment += materials[mat_id].Ke *10.0f;
            } else {
                illum_increment += sky_illum_color0 *1.0f;
            }
        }
        pixel[pos].global_illum_buffer = (pixel[pos].global_illum_buffer*iteration_count + illum_increment/nb_iteration) / (iteration_count+1);
        pixel[pos].global_illum_buffer = max_rgb(materials[pixel[pos].material_index].Ke, pixel[pos].global_illum_buffer);
    } else {
        pixel[pos].global_illum_buffer = sky_illum_color0;
    }
    
    /*********** noise filter ***********///
    /*
    rgb mean = {0,0,0,1};
    float div = 0;
    int kernel_size = 10;
    for (int xc = -kernel_size; xc<=kernel_size; xc++) {
        for (int yc = -kernel_size; yc<=kernel_size; yc++) {
            if (pixel[coord(xc +x, yc +y, x_res, y_res)].normal_buffer.x == pixel[pos].normal_buffer.x &&
            pixel[coord(xc +x, yc +y, x_res, y_res)].normal_buffer.y == pixel[pos].normal_buffer.y &&
            pixel[coord(xc +x, yc +y, x_res, y_res)].normal_buffer.z == pixel[pos].normal_buffer.z) {
                float p = pow(1 - sqrt((float) xc*xc + yc*yc) / (kernel_size*kernel_size), 5);
                div += p;
                mean += pixel[coord(xc +x, yc +y, x_res, y_res)].global_illum_buffer * p;
            }
            
        }
    }
    mean /= div;//*/
    
    /*********** Build up final render ***********///
    float direct_light_coef = 0.5f;
    rgb c = pixel[pos].color_value_buffer 
        * pixel[pos].global_illum_buffer
        * ((1-direct_light_coef) + pixel[pos].direct_light_buffer*direct_light_coef);
    //c = mean;
        //* mean
        //* pixel[pos].global_illum_buffer
    
    uint4 color = (uint4)(
        c.x*255, 
        c.y*255, 
        c.z*255, 
        255
    );
    
    write_imageui(final_outut_buffer, (int2)(x, y), color);
} 