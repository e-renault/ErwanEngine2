// cdee
// gcc main.c -lm -lOpenCL -lpthread -lGL -lglut
// ./a.out --obj maxwell.obj --texture maxwell.ppm --FPS

/*********** Please select you GPU there ***********/
#define USE_DEVICE_ID 0
#define USE_PLATFORM_ID 0



/*********** Static includes ***********/
#include "kernel/header.h"  //opencl & cie
#include <stdio.h>          //IO
#include <sys/stat.h>       //extract kernel
#include <stdlib.h>         //malloc & rand
#include <sys/time.h>       //FPS counter
#include <getopt.h>         //argument parameter parser
#include <pthread.h>        //obvious
#include <semaphore.h>      //obvious
#include <unistd.h>         //semaphore too ? I think.
#include <libgen.h>         //fore file basename

#include "loader/image.h"
#include "loader/scene.h"
#include "tools/eemacros.h"



/*********** Globals variables ***********/
#define SUCCESS_MSG "(Done)"
#define ERROR_MSG "(Error)"


// frame definition (default)
static int Y_RES = 600;
static int X_RES = 800;
static int RES = 600 * 800;//Y_RES*X_RES
static float FOV = 70.0;
static char scene_path[400] = "src/default/";
static char obj_file_name[100] = "default.obj";
static char default_texture_map_name[] = "default_texture.ppm";

// debug
static int DISPLAY_SCENE_INFO = 0;
static int DEBUG_GLOBAL_INFO = 0;
static int DEBUG_KERNEL_INFO = 0;
static int DEBUG_HARDWARE_INFO = 0;
static int DEBUG_RUN_INFO = 0;


//time control
struct timeval stop, start;

//output buffer
static unsigned char* output_render_buffer;

// scene description
static Point3 cam_coordinate;
static Vector3 cam_lookat;
static Vector3 sky_light_dir;//direction of the sun light


// program flow control
static int program_running_loop = 1;
static int scene_changed = 1;
static int cam_moved = 1;
static int new_frame = 0;
static int enqueueKernel = 1;

// semaphores
sem_t mutex;



/*********** Late include ***********/
#include "tools/opencl_tools.h"
#include "tools/gui_tools.h"
#include "tools/aspectc.h"



/** LOCAL FUNCTIONNALITIES **/
void extract_params(int argc, char *argv[]);
void extract_params(int argc, char *argv[]) {
    int c;
    int digit_optind = 0;

    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"RES",     required_argument, 0,  'r' },
            {"FOV",     required_argument, 0,  'f' },
            {"obj",     required_argument, 0,  'o' },

            {"XYZ",     no_argument,       &DISPLAY_SCENE_INFO,  1 },
            {"GINFO",   no_argument,       &DEBUG_GLOBAL_INFO,  1 },
            {"KINFO",   no_argument,       &DEBUG_KERNEL_INFO,  1 },
            {"HINFO",   no_argument,       &DEBUG_HARDWARE_INFO,  1 },
            {"RINFO",   no_argument,       &DEBUG_RUN_INFO,     1 },
            {0,         0,                 0,  0 }

        };
        c = getopt_long(argc, argv, "abc:d:012", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                printf("set %s", long_options[option_index].name);
                if (optarg)
                    printf(" to : %s", optarg);
                printf("\n");
                break;
            case 'r':
                if (2 == sscanf(optarg,"%dx%d", &X_RES, &Y_RES)){
                    printf("Y_RES set to '%i'\n", Y_RES);
                    printf("X_RES set to '%i'\n", X_RES);
                } else {
                    printf("Error in resolution format [%s]\n", optarg);
                }
                break;
            case 'f':
                FOV = atof(optarg);
                printf("FOV set to '%f'\n", FOV);
                break;
            case 'o':
                strcpy(obj_file_name, optarg);
                char* base = basename(optarg);
                char* dir = dirname(optarg);

                strcpy(obj_file_name, base);
                strcpy(scene_path, dir);strcat(scene_path, "/");
                
                printf("obj_file_name set to '%s'\n", obj_file_name);
                printf("scene_path set to '%s'\n", scene_path);
                break;
            case '?':
                break;

            default:
                printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

    RES = X_RES * Y_RES;
}


int main(int argc, char *argv[]) {
    //init vars
    extract_params(argc, argv);
    cl_int status;
    cl_int x, y, i;
    srand(time(NULL));
    

    printf("\n\n ### Starting program, creating datas, starting threads ###\n\n");
    
    // init scene vars
    output_render_buffer = (unsigned char*) calloc(RES, sizeof(char) * 4);
    cl_int nb_triangles = 0;
    cl_int nb_objects = 0;
    cl_int nb_materials = 0;
    Triangle3* triangles;
    UV* texture_uvs;
    Object* objects;
    Material* materials;

    status = load_obj_file(
        scene_path,
        obj_file_name,
        &nb_triangles, &triangles, &texture_uvs,
        &nb_objects, &objects,
        &nb_materials, &materials
    );


    //TODO: This entiere section is deprecated
    cl_int nb_lights = 0;
    LightSource3* lights = (LightSource3*) calloc(10, sizeof(LightSource3));
    status = load_scene_context(
        &nb_lights, lights,
        &cam_coordinate,&cam_lookat,
        &sky_light_dir
    );
    //end deprecated



    // start glut thread
    pthread_t glut_thread_id;
    glutInit(&argc, argv);
    status = pthread_create(&glut_thread_id, NULL, start_glut, NULL);
    if (DEBUG_GLOBAL_INFO) printf("%s Creating GLUT thread\n", (status == 0)? SUCCESS_MSG:(ERROR_MSG));

    //setup OPENCL
    cl_platform_id* platforms = init_platform();

    cl_uint numDevices = 0;
    cl_device_id* devices = init_device(platforms, &numDevices);
    free(platforms);

    cl_context context = init_context(numDevices, devices);
    
    cl_command_queue cmdQueue = init_queue(context, devices);

    cl_kernel kernel = init_kernel_program(numDevices, devices, context, "kernel/cast_render_kernel.cl", "rayTrace");
    clReleaseContext(context);
    free(devices);

    
    // STEP 3: Create device buffers
    cl_mem triangles_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_ONLY, nb_triangles * sizeof(Triangle3), NULL, &status);
    cl_mem uv_map_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_ONLY, nb_triangles * sizeof(UV), NULL, &status);
    cl_mem objects_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_ONLY, nb_objects * sizeof(Object), NULL, &status);
    cl_mem material_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_ONLY, nb_materials * sizeof(Material), NULL, &status);
    cl_mem lights_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_ONLY, RES * sizeof(LightSource3), NULL, &status);
    cl_mem pixel_data_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_WRITE, RES * sizeof(LocalPixelData), NULL, &status);

    // Create images
    cl_image_format image_format = (cl_image_format) {
        .image_channel_order = CL_RGBA,
        .image_channel_data_type = CL_UNSIGNED_INT8
    };

    //output image
    cl_image_desc output_image_desc = (cl_image_desc){
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = X_RES,
        .image_height = Y_RES,
    };
    cl_mem render_image = aspectC_clCreateImage(context, CL_MEM_WRITE_ONLY, &image_format, &output_image_desc, NULL, &status);
    
    //textures images
    cl_image_desc texture_image_desc = (cl_image_desc){
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = 5000,
        .image_height = 1000
    };
    cl_mem uv_map_image = aspectC_clCreateImage(context, CL_MEM_READ_ONLY, &image_format, &texture_image_desc, NULL, &status);
    



    // init window
    cl_int x_res = X_RES;
    cl_int y_res = Y_RES;
    cl_float fov = FOV;
    status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_X_RES, sizeof(cl_int), (void*) &x_res);
    status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_Y_RES, sizeof(cl_int), (void*) &y_res);
    status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_FOV, sizeof(cl_float), (void*) &fov);
    status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_O_BUFFER, sizeof(cl_mem), (void*) &render_image);
    status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_DTA_BUFF, sizeof(cl_mem), (void*) &pixel_data_buffer);
    

    int iteration_counter = 0;
    do {

        if (scene_changed) {
            scene_changed = 0;
            cam_moved = 1;
            enqueueKernel = 1;
            
            size_t offset_temp[3] = {0, 0};
            int texture_map_res_x, texture_map_res_y;
            for (int i = 0; i<nb_materials; i++) {
                if (materials[i].hasTexture) {
                    unsigned char* texture_map;
                    status = load_image(scene_path, materials[i].texture_path, &texture_map_res_x, &texture_map_res_y, 0, 0, &texture_map);
                    
                    materials[i].map_location.v_size= (cl_int2) {texture_map_res_x, texture_map_res_y};
                    materials[i].map_location.v_off = (cl_int2) {offset_temp[0], offset_temp[1]};
                    
                    const size_t texture_region[3] = {texture_map_res_x, texture_map_res_y, 1};
                    const size_t origin[3] = {offset_temp[0], offset_temp[1], 0};
                    
                    status = aspectC_clEnqueueWriteImage(cmdQueue, uv_map_image, CL_TRUE, origin, texture_region, 0, 0, texture_map, 0, NULL, NULL);
                    
                    printf("New texture : [%s], [%lix%li] off (+%li,+%li)\n", materials[i].texture_path, texture_region[0], texture_region[1], origin[0], origin[1]);
                    offset_temp[0] += texture_map_res_x;
                    //offset_temp[1] += texture_map_res_y;
                    free(texture_map);
                }
            }
            
            // STEP 4: Write host data to device buffers
            status = aspectC_clEnqueueWriteBuffer(cmdQueue, triangles_buffer, CL_TRUE, 0, nb_triangles * sizeof(Triangle3), triangles, 0, NULL, NULL);
            status = aspectC_clEnqueueWriteBuffer(cmdQueue, uv_map_buffer, CL_TRUE, 0, nb_triangles * sizeof(UV), texture_uvs, 0, NULL, NULL);
            status = aspectC_clEnqueueWriteBuffer(cmdQueue, objects_buffer, CL_TRUE, 0, nb_objects * sizeof(Object), objects, 0, NULL, NULL);
            status = aspectC_clEnqueueWriteBuffer(cmdQueue, material_buffer, CL_TRUE, 0, nb_materials * sizeof(Material), materials, 0, NULL, NULL);
            status = aspectC_clEnqueueWriteBuffer(cmdQueue, lights_buffer, CL_TRUE, 0, nb_lights * sizeof(LightSource3), lights, 0, NULL, NULL);
            

            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_NB_TRI, sizeof(cl_int), (void*) &nb_triangles);            
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_TRIS, sizeof(cl_mem), (void*) &triangles_buffer);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_UV_MAP, sizeof(cl_mem), (void*) &uv_map_buffer);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_NB_OBJ, sizeof(cl_int), (void*) &nb_objects);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_OBJECT, sizeof(cl_mem), (void*) &objects_buffer);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_NB_MAT, sizeof(cl_int), (void*) &nb_materials);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_MATERIAL, sizeof(cl_mem), (void*) &material_buffer);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_NB_LIGHT, sizeof(cl_int), (void*) &nb_lights);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_LIGHTS, sizeof(cl_mem), (void*) &lights_buffer);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_SKY_DIR, sizeof(Vector3), (void*) &sky_light_dir);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_TXT_M, sizeof(cl_mem), (void*) &uv_map_image);
            
        }

        if (cam_moved) {
            cam_moved = 0;
            enqueueKernel = 1;
            iteration_counter = 0;

            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_CAM_POS, sizeof(Point3), (void*) &cam_coordinate);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_CAM_DIR, sizeof(Vector3), (void*) &cam_lookat);
        }
        
        while (enqueueKernel) {
            gettimeofday(&start, NULL);
            enqueueKernel--;

            int r = rand();
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_IT_COUNT, sizeof(cl_int), (void*) &r);
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_RD_SEED, sizeof(cl_int), (void*) &iteration_counter);
            
            iteration_counter++;

            // STEP 10: Enqueue the kernel for execution
            size_t globalWorkSize[2]={X_RES, Y_RES};
            status = aspectC_clEnqueueNDRangeKernel( cmdQueue, kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);

            clFinish(cmdQueue);


            // STEP 12: Read the output buffer back to the host
            status = aspectC_clEnqueueReadImage(cmdQueue, render_image, CL_TRUE, (size_t[3]){0, 0, 0},(size_t[3]){X_RES, Y_RES, 1}, 0, 0, output_render_buffer, 0, NULL, NULL);
            
            gettimeofday(&stop, NULL);
            new_frame++;
        }
        
        sem_wait(&mutex);

    } while(program_running_loop);
    
    save_image(output_render_buffer, "_output/output.ppm", X_RES, Y_RES);

    // STEP 13: Release OpenCL resources
    clReleaseKernel(kernel);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(triangles_buffer);
    clReleaseMemObject(render_image);
    clReleaseMemObject(lights_buffer);
    clReleaseMemObject(uv_map_buffer);

    // Free host resources
    free(triangles);
    free(texture_uvs);
    free(lights);
    free(output_render_buffer);
}