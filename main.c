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
static char scene_path[] = "src/default/";
static char obj_file_name[] = "default.obj";
static char default_texture_map_path[] = "src/default/default_texture.ppm";
static char default_normal_map_path[] = "src/default/default_normal.ppm";

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
            {"XRES",    required_argument, 0,  'x' },
            {"YRES",    required_argument, 0,  'y' },
            {"FOV",     required_argument, 0,  'f' },
            {"obj",     required_argument, 0,  'o' },
            {"path",    required_argument, 0,  'p' },

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
            case 'p':
                strcpy(scene_path, optarg);
                printf("scene_path set to '%s'\n", scene_path);
                break;
            case 'x':
                X_RES = atoi(optarg);
                printf("X_RES set to '%i'\n", X_RES);
                break;
            case 'y':
                Y_RES = atoi(optarg);
                printf("Y_RES set to '%i'\n", Y_RES);
                break;
            case 'f':
                FOV = atof(optarg);
                printf("FOV set to '%f'\n", FOV);
                break;
            case 'o':
                strcpy(obj_file_name, optarg);
                printf("obj_file_name set to '%s'\n", obj_file_name);
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
    

    if (DEBUG_GLOBAL_INFO) printf(" ### Starting program, creating datas, starting threads ###\n");
    
    // init scene vars
    output_render_buffer = (unsigned char*) calloc(RES, sizeof(char) * 4);
    cl_int nb_triangles = 0;
    cl_int nb_objects = 0;
    cl_int nb_materials = 0;
    Triangle3* triangles;
    Texture* texture_uvs;
    Object* objects;
    Material* materials;

    if (DEBUG_GLOBAL_INFO) printf("Loading : %s\n", obj_file_name);
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

    int texture_map_res_x, texture_map_res_y;
    unsigned char* texture_map;
    status = load_image(default_texture_map_path, &texture_map_res_x, &texture_map_res_y, 0, 0, &texture_map);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Load texture map\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    int normal_map_res_x, normal_map_res_y;
    unsigned char* normal_map;
    status = load_image(default_normal_map_path, &normal_map_res_x, &normal_map_res_y, 0, 0, &normal_map);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Load normal map\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
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
    if (DEBUG_KERNEL_INFO) printf("\nSTEP 3: Create device buffers\n");
    cl_mem triangles_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_ONLY, nb_triangles * sizeof(Triangle3), NULL, &status);
    cl_mem uv_map_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_ONLY, nb_triangles * sizeof(Texture), NULL, &status);
    cl_mem objects_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_ONLY, nb_objects * sizeof(Object), NULL, &status);
    cl_mem material_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_ONLY, nb_materials * sizeof(Material), NULL, &status);
    cl_mem lights_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_ONLY, RES * sizeof(LightSource3), NULL, &status);
    cl_mem pixel_data_buffer = aspectC_clCreateBuffer(context, CL_MEM_READ_WRITE, RES * sizeof(LocalPixelData), NULL, &status);

    // Create images
    cl_image_format image_format = (cl_image_format) {
        .image_channel_order = CL_RGBA,
        .image_channel_data_type = CL_UNSIGNED_INT8
    };
    



    //TODO: This entiere section is deprecated
    cl_image_desc output_image_desc = (cl_image_desc){
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = X_RES,
        .image_height = Y_RES,
    };
    cl_mem render_image = aspectC_clCreateImage(context, CL_MEM_WRITE_ONLY, &image_format, &output_image_desc, NULL, &status);
    
    cl_image_desc texture_image_desc = (cl_image_desc){
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = texture_map_res_x,
        .image_height = texture_map_res_y
    };
    cl_mem uv_map_image = aspectC_clCreateImage(context, CL_MEM_READ_ONLY, &image_format, &texture_image_desc, NULL, &status);
    
    cl_image_desc normal_image_desc = (cl_image_desc){
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = normal_map_res_x,
        .image_height = normal_map_res_y
    };
    cl_mem normal_map_image = aspectC_clCreateImage(context, CL_MEM_READ_ONLY, &image_format, &normal_image_desc, NULL, &status);
    //end deprecated



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
            
            
            // STEP 4: Write host data to device buffers
            if (DEBUG_KERNEL_INFO) printf("\nSTEP 4: Write host data to device buffers\n");
            
            status = aspectC_clEnqueueWriteBuffer(cmdQueue, triangles_buffer, CL_TRUE, 0, nb_triangles * sizeof(Triangle3), triangles, 0, NULL, NULL);
            status = aspectC_clEnqueueWriteBuffer(cmdQueue, uv_map_buffer, CL_TRUE, 0, nb_triangles * sizeof(Texture), texture_uvs, 0, NULL, NULL);
            status = aspectC_clEnqueueWriteBuffer(cmdQueue, objects_buffer, CL_TRUE, 0, nb_objects * sizeof(Object), objects, 0, NULL, NULL);
            status = aspectC_clEnqueueWriteBuffer(cmdQueue, material_buffer, CL_TRUE, 0, nb_materials * sizeof(Material), materials, 0, NULL, NULL);
            status = aspectC_clEnqueueWriteBuffer(cmdQueue, lights_buffer, CL_TRUE, 0, nb_lights * sizeof(LightSource3), lights, 0, NULL, NULL);
            
            size_t origin[3] = {0, 0, 0};
            size_t texture_region[3] = {texture_map_res_x, texture_map_res_y, 1};
            status = aspectC_clEnqueueWriteImage(cmdQueue, uv_map_image, CL_TRUE, origin, texture_region, 0, 0, texture_map, 0, NULL, NULL);
            
            size_t normal_region[3] = {normal_map_res_x, normal_map_res_y, 1};
            status = aspectC_clEnqueueWriteImage(cmdQueue, normal_map_image, CL_TRUE, origin, normal_region, 0, 0, normal_map, 0, NULL, NULL);
            

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
            status = aspectC_clSetKernelArg(kernel, ARGUMENT_INDEX_NML_M, sizeof(cl_mem), (void*) &normal_map_image);
            
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
            if (DEBUG_RUN_INFO)  printf("\nSTEP 10: Enqueue the kernel for execution (new frame)\n");fflush(stdout);

            size_t globalWorkSize[2]={X_RES, Y_RES};
            status = aspectC_clEnqueueNDRangeKernel( cmdQueue, kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);

            clFinish(cmdQueue);


            // STEP 12: Read the output buffer back to the host
            if (DEBUG_RUN_INFO) printf("\nSTEP 12: Read the output buffer back to the host\n");
            
            status = clEnqueueReadImage(cmdQueue, render_image, CL_TRUE, (size_t[3]){0, 0, 0},(size_t[3]){X_RES, Y_RES, 1}, 0, 0, output_render_buffer, 0, NULL, NULL);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Read results\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            gettimeofday(&stop, NULL);
            new_frame++;
        }
        
        sem_wait(&mutex);

    } while(program_running_loop);
    
    save_image(output_render_buffer, "_output/output.ppm", X_RES, Y_RES);

    // STEP 13: Release OpenCL resources
    if (DEBUG_GLOBAL_INFO) printf("\nSTEP 13: Release OpenCL resources\n");
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
    free(texture_map);
    free(normal_map);
}