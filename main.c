// cd /media/erenault/EXCHANGE/workspaces/ErwanEngine2/opencl/
// gcc main.c -lm -lOpenCL -lpthread -lGL -lglut
// optional: -DDEBUG_RUN_INFO=1
// ./a.out --file cheval.obj --FPS

#define CL_TARGET_OPENCL_VERSION 300
#define USE_DEVICE_ID 0
#define USE_PLATFORM_ID 0

#ifndef DEBUG_RUN_INFO
    #define DEBUG_RUN_INFO 0
#endif

#include <stdio.h>
#include <sys/stat.h> //extract kernel
#include <stdlib.h> //malloc
#include <pthread.h> //obvious
#include <sys/time.h> //FPS counter
#include <getopt.h> //argument parameter parser
#include "kernel/header.h" //opencl & cie

#include <GL/freeglut.h> //for window


// constants
static const char* SUCCESS_MSG = "(Done)";
static const char* ERROR_MSG = "(Error)";

/* Global vars */
// frame definition (default)
static int Y_RES = 600;
static int X_RES = 800;
static int RES = 480000;//Y_RES*X_RES
static int MAX_NB_TRIANGLE = 650;
static int MAX_NB_LIGHTSOURCE = 10;
static float FOV = 70.0;
static char scene_path[] = "src/obj/";
static char texture_path[] = "src/texture/";
static char obj_file_name[] = "default.obj";
static char texture_map_path[] = "minecraft.ppm";
static char normal_map_path[] = "gimp_raw.ppm";
static int SHOW_FPS = 0;
static int DEBUG_GLOBAL_INFO = 0;
static int DEBUG_KERNEL_INFO = 0;
static int DEBUG_HARDWARE_INFO = 0;


#include "math/lib3D.h"
#include "math/lib3D_debug.h"
#include "render/image.h"
#include "scene/sceneLoader.h"
#include "opencl_tools.h"


// scene description
static unsigned char* output_render_buffer;
static unsigned char* texture_map;
static unsigned char* normal_map;
static Point3 cam_coordinate;
static Vector3 cam_lookat;
static Vector3 sky_light_dir;//direction of the sun light
static Texture sky_light_texture;//sky texture see vvv
/*
    color1: horizon (color used for global ilumination)
    color2: void
    color3: Sky
*/

// program flow control
static int program_running_loop = 1;
static int scene_changed = 1;
static int cam_moved = 1;
static int new_frame = 1;
static int cube_demo = 0;


/**  DISPLAY SECTION  **/
void display_callback();
void display_callback(){
    if (new_frame) {
        new_frame=0;
        
        glClearColor( 0, 0, 0, 1 );
        glClear( GL_COLOR_BUFFER_BIT );

        //draw pixels
        glDrawPixels( X_RES, Y_RES, GL_RGBA, GL_UNSIGNED_BYTE, output_render_buffer );
        glutSwapBuffers();
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int xmouse, int ymouse);
void keyboard(unsigned char key, int xmouse, int ymouse) {
    Vector3 rightVector = crossProduct(cam_lookat, UP);
    switch (key) {
        case 'z':cam_coordinate = add_vector3(cam_coordinate, scale_vector3(0.1, cam_lookat));cam_moved = 1;break;
        case 's':cam_coordinate = add_vector3(cam_coordinate, scale_vector3(-0.1f, cam_lookat));cam_moved = 1;break;
        case 'q':cam_coordinate = add_vector3(cam_coordinate, scale_vector3(-0.1, rightVector));cam_moved = 1;break;
        case 'd':cam_coordinate = add_vector3(cam_coordinate, scale_vector3(0.1f, rightVector));cam_moved = 1;break;
        case ' ':cam_coordinate.y += 0.1;cam_moved = 1;break;
        case 'e':cam_coordinate.y -= 0.1;cam_moved = 1;break;
        case 'm':cam_lookat = rotateAround(cam_lookat, UP, -0.1f);cam_moved = 1;break;
        case 'k':cam_lookat = rotateAround(cam_lookat, UP, 0.1f);cam_moved = 1;break;
        case 'o': if (getAngle(cam_lookat, UP) > 0.2f) cam_lookat = rotateAround(cam_lookat, rightVector, 0.1f);cam_moved = 1;break;
        case 'l': if (getAngle(cam_lookat, DOWN) > 0.2f) cam_lookat = rotateAround(cam_lookat, rightVector, -0.1f);cam_moved = 1;break;
        case 't':sky_light_dir.x +=0.1;scene_changed = 1;break;
        case 'g':sky_light_dir.x -=0.1;scene_changed = 1;break;
        case 'y':sky_light_dir.y +=0.1;scene_changed = 1;break;
        case 'h':sky_light_dir.y -=0.1;scene_changed = 1;break;
        case 'u':sky_light_dir.z +=0.1;scene_changed = 1;break;
        case 'j':sky_light_dir.z -=0.1;scene_changed = 1;break;
        case 'r':cube_demo = !cube_demo; break;
        case 'x':
            program_running_loop = 0;
            glutLeaveMainLoop();
            break;
        default:
            break;
    }
    if (SHOW_FPS) {
        printf("\rCam: {pos: ");
        printPoint3(cam_coordinate);
        printf(", dir: ");
        printVector3(cam_lookat);
        printf("} - Sun_light: {");
        printVector3(sky_light_dir);
        printf("}   ");
    }
}

void* start_glut(void *vargp);
void* start_glut(void *vargp) {
    if (DEBUG_GLOBAL_INFO) printf("GLUT Start\n");
    //init GLUT
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(X_RES, Y_RES);
    glutCreateWindow("ErwanEngine2.0");
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    //display loop
    glutDisplayFunc(display_callback);
    glutKeyboardFunc(keyboard);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    glutMainLoop();
    program_running_loop = 0;
    if (DEBUG_GLOBAL_INFO) printf("GLUT Exit\n");
}


/** LOCAL FUNCTIONNALITIES **/
void extract_params(int argc, char *argv[]);
void extract_params(int argc, char *argv[]) {
    int c;
    int digit_optind = 0;

    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"max_triangle",    required_argument, 0,  'r' },
            {"texture",         required_argument, 0,  't' },
            {"max_lightsource", required_argument, 0,  'l' },
            {"XRES",            required_argument, 0,  'x' },
            {"YRES",            required_argument, 0,  'y' },
            {"FOV",             required_argument, 0,  'f' },
            {"file",            required_argument, 0,  'o' },
            {"path",            required_argument, 0,  'p' },
            {"FPS",             no_argument,       &SHOW_FPS,  1 },
            {"GINFO",           no_argument,       &DEBUG_GLOBAL_INFO,  1 },
            {"KINFO",           no_argument,       &DEBUG_KERNEL_INFO,  1 },
            {"HINFO",           no_argument,       &DEBUG_HARDWARE_INFO,  1 },
            {0,                 0,                 0,  0 }

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

            case 't':
                strcpy(texture_map_path, optarg);
                printf("texture_map_path set to '%s'\n", texture_map_path);
                break;
            case 'r':
                MAX_NB_TRIANGLE = atoi(optarg);
                printf("MAX_NB_TRIANGLE set to '%i'\n", MAX_NB_TRIANGLE);
                break;
            case 'l':
                MAX_NB_LIGHTSOURCE = atoi(optarg);
                printf("MAX_NB_LIGHTSOURCE set to '%i'\n", MAX_NB_LIGHTSOURCE);
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
            case 'p':
                strcpy(scene_path, optarg);
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
    extract_params(argc, argv);
    
    if (DEBUG_GLOBAL_INFO) printf(" ### Starting program, creating datas, starting threads ###\n");
    
    cl_int status;
    cl_int x, y, i;

    // load scene components
    output_render_buffer = (unsigned char*) calloc(RES, sizeof(char) * 4);
    
    cl_int real_nb_triangles = 0;
    Triangle3* triangles = (Triangle3*) calloc(MAX_NB_TRIANGLE, sizeof(Triangle3));
    Texture* textures = (Texture*) calloc(MAX_NB_TRIANGLE, sizeof(Texture));

    cl_int real_nb_lights = 0;
    LightSource3* lights = (LightSource3*) calloc(MAX_NB_LIGHTSOURCE, sizeof(LightSource3));

    char path[500] = "\0";strcat(path, scene_path);strcat(path, obj_file_name);
    if (DEBUG_GLOBAL_INFO) printf("Loading : %s\n", path);
    status = loadSceneFromFile(
        path, 
        &real_nb_triangles, triangles, 
        textures, 
        &real_nb_lights, lights,
        &cam_coordinate,&cam_lookat,
        &sky_light_dir,&sky_light_texture
    );

    char path2[1000] = "\0";strcat(path2, texture_path);strcat(path2, texture_map_path);
    int texture_map_res_x, texture_map_res_y;
    texture_map = load_file(path2, &texture_map_res_x, &texture_map_res_y);
    //save_to_file(texture_map, "test.ppm", texture_map_res_x, texture_map_res_y);

    char path3[1000] = "\0";strcat(path3, texture_path);strcat(path3, normal_map_path);
    int normal_map_res_x, normal_map_res_y;
    normal_map = load_file(path3, &normal_map_res_x, &normal_map_res_y);


    // start glut thread
    pthread_t glut_thread_id;
    glutInit(&argc, argv);
    status = pthread_create(&glut_thread_id, NULL, start_glut, NULL);
    if (DEBUG_GLOBAL_INFO) printf("%s Creating GLUT thread\n", (status == 0)? SUCCESS_MSG:(ERROR_MSG));


    cl_platform_id* platforms = init_platform();

    cl_uint numDevices = 0;
    cl_device_id* devices = init_device(platforms, &numDevices);
    free(platforms);

    cl_context context = init_context(numDevices, devices);
    
    cl_command_queue cmdQueue = init_queue(context, devices);

    cl_kernel kernel = init_kernel_program(numDevices, devices, context, "kernel/cast_render_kernel.cl", "simpleCast");
    clReleaseContext(context);
    free(devices);

    
    // STEP 3: Create device buffers
    if (DEBUG_KERNEL_INFO) printf("\nSTEP 3: Create device buffers\n");
    
    cl_mem triangles_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, MAX_NB_TRIANGLE * sizeof(Triangle3), NULL, &status);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Create triangle buffer\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    cl_mem lights_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, RES * sizeof(LightSource3), NULL, &status);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Create light buffer\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    cl_mem textures_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, RES * sizeof(Texture), NULL, &status);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Create texture buffer\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    // Create images
    cl_image_format image_format = (cl_image_format) {
        .image_channel_order = CL_RGBA,
        .image_channel_data_type = CL_UNSIGNED_INT8
    };
    
    cl_image_desc output_image_desc = (cl_image_desc){
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = X_RES,
        .image_height = Y_RES,
    };
    cl_mem render_image = clCreateImage(context, CL_MEM_WRITE_ONLY, &image_format, &output_image_desc, NULL, &status);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Create render image\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));


    cl_image_desc texture_image_desc = (cl_image_desc){
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = texture_map_res_x,
        .image_height = texture_map_res_y
    };
    cl_mem texture_map_image = clCreateImage(context, CL_MEM_READ_ONLY, &image_format, &texture_image_desc, NULL, &status);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Create texture image\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    

    cl_image_desc normal_image_desc = (cl_image_desc){
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = normal_map_res_x,
        .image_height = normal_map_res_y
    };
    cl_mem normal_map_image = clCreateImage(context, CL_MEM_READ_ONLY, &image_format, &normal_image_desc, NULL, &status);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Create normal image\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));


    // init window
    cl_int x_res = X_RES;
    status = clSetKernelArg(kernel, 0, sizeof(cl_int), (void*) &x_res);
    if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set X_RES\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    cl_int y_res = Y_RES;
    status = clSetKernelArg(kernel, 1, sizeof(cl_int), (void*) &y_res);
    if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set Y_RES\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    cl_float fov = FOV;
    status = clSetKernelArg(kernel, 2, sizeof(cl_float), (void*) &fov);
    if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set FOV\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    status = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*) &render_image);
    if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set render_image*\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));




    int frame = 0, timebase = 0;//used for FPS counter
    do {
        struct timeval stop, start;
        gettimeofday(&start, NULL);
        if (cube_demo) {
            status = loadCubeScene(
                path, 
                &real_nb_triangles, triangles, 
                textures, 
                &real_nb_lights, lights,
                &cam_coordinate, &cam_lookat,
                &sky_light_dir, &sky_light_texture
            );
            scene_changed = 1;
        }

        if (scene_changed) {
            scene_changed = 0;
            cam_moved = 1;
            
            
            // STEP 4: Write host data to device buffers
            if (DEBUG_KERNEL_INFO) printf("\nSTEP 4: Write host data to device buffers\n");
            
            status = clEnqueueWriteBuffer(cmdQueue, triangles_buffer, CL_TRUE, 0, real_nb_triangles * sizeof(Triangle3), triangles, 0, NULL, NULL);
            if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Write triangle buffer\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clEnqueueWriteBuffer(cmdQueue, textures_buffer, CL_TRUE, 0, real_nb_triangles * sizeof(Texture), textures, 0, NULL, NULL);
            if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Write texture buffer\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clEnqueueWriteBuffer(cmdQueue, lights_buffer, CL_TRUE, 0, real_nb_lights * sizeof(LightSource3), lights, 0, NULL, NULL);
            if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Write light buffer\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            size_t origin[3] = {0, 0, 0};
            size_t texture_region[3] = {texture_map_res_x, texture_map_res_y, 1};
            status = clEnqueueWriteImage(cmdQueue, texture_map_image, CL_TRUE, origin, texture_region, 0, 0, texture_map, 0, NULL, NULL);
            if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Write texture image\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
        
            size_t normal_region[3] = {normal_map_res_x, normal_map_res_y, 1};
            status = clEnqueueWriteImage(cmdQueue, normal_map_image, CL_TRUE, origin, normal_region, 0, 0, normal_map, 0, NULL, NULL);
            if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Write normal image\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
        


            status = clSetKernelArg(kernel, 4, sizeof(cl_int), (void*) &real_nb_triangles);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set real_nb_triangles*\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*) &triangles_buffer);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set triangles_buffer*\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clSetKernelArg(kernel, 6, sizeof(cl_mem), (void*) &textures_buffer);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set textures_buffer*\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clSetKernelArg(kernel, 7, sizeof(cl_int), (void*) &real_nb_lights);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set real_nb_lights\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clSetKernelArg(kernel, 8, sizeof(cl_mem), (void*) &lights_buffer);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set lights_buffer*\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clSetKernelArg(kernel, 9, sizeof(Vector3), (void*) &sky_light_dir);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set sky_light_dir\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clSetKernelArg(kernel, 10, sizeof(Texture), (void*) &sky_light_texture);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set sky_light_texture\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clSetKernelArg(kernel, 13, sizeof(cl_mem), (void*) &texture_map_image);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set texture_map_image\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clSetKernelArg(kernel, 14, sizeof(cl_mem), (void*) &normal_map_image);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set normal_map_image\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        }

        if (cam_moved) {
            cam_moved = 0;

            status = clSetKernelArg(kernel, 11, sizeof(Point3), (void*) &cam_coordinate);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set cam_coordinate\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            status = clSetKernelArg(kernel, 12, sizeof(Vector3), (void*) &cam_lookat);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Set cam_lookat\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
        
            // STEP 10: Enqueue the kernel for execution
            if (DEBUG_RUN_INFO)  printf("\nSTEP 10: Enqueue the kernel for execution (new frame)\n");
            fflush(stdout);

            size_t globalWorkSize[2]={X_RES, Y_RES};
            status = clEnqueueNDRangeKernel( cmdQueue, kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);

            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s call\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
            switch(status) {
                case CL_INVALID_PROGRAM_EXECUTABLE:printf("CL_INVALID_PROGRAM_EXECUTABLE\n");break;
                case CL_INVALID_COMMAND_QUEUE:printf("CL_INVALID_COMMAND_QUEUE\n");break;
                case CL_INVALID_KERNEL:printf("CL_INVALID_KERNEL\n");break;
                case CL_INVALID_CONTEXT:printf("CL_INVALID_CONTEXT\n");break;
                case CL_INVALID_KERNEL_ARGS:printf("CL_INVALID_KERNEL_ARGS\n");break;
                case CL_INVALID_WORK_DIMENSION:printf("CL_INVALID_WORK_DIMENSION\n");break;
                case CL_INVALID_GLOBAL_WORK_SIZE:printf("CL_INVALID_GLOBAL_WORK_SIZE\n");break;
                case CL_INVALID_GLOBAL_OFFSET:printf("CL_INVALID_GLOBAL_OFFSET\n");break;
                case CL_INVALID_WORK_GROUP_SIZE:printf("CL_INVALID_WORK_GROUP_SIZE\n");break;
                case CL_INVALID_WORK_ITEM_SIZE:printf("CL_INVALID_WORK_ITEM_SIZE\n");break;
                case CL_MISALIGNED_SUB_BUFFER_OFFSET:printf("CL_MISALIGNED_SUB_BUFFER_OFFSET\n");break;
                case CL_INVALID_IMAGE_SIZE:printf("CL_INVALID_IMAGE_SIZE\n");break;
                case CL_IMAGE_FORMAT_NOT_SUPPORTED:printf("CL_IMAGE_FORMAT_NOT_SUPPORTED\n");break;
                case CL_OUT_OF_RESOURCES:printf("CL_OUT_OF_RESOURCES\n");break;
                case CL_MEM_OBJECT_ALLOCATION_FAILURE:printf("CL_MEM_OBJECT_ALLOCATION_FAILURE\n");break;
                case CL_INVALID_EVENT_WAIT_LIST:printf("CL_INVALID_EVENT_WAIT_LIST\n");break;
                case CL_INVALID_OPERATION:printf("CL_INVALID_OPERATION\n");break;
                case CL_OUT_OF_HOST_MEMORY:printf("CL_OUT_OF_HOST_MEMORY\n");break;
            }

            clFinish(cmdQueue);
        
            //FPS Counter
            frame++;
            if (start.tv_sec > timebase) {
                if (SHOW_FPS) printf("FPS:%i   \r", frame);fflush(stdout);
                timebase = start.tv_sec;
                frame = 0;
            }

            // STEP 12: Read the output buffer back to the host
            if (DEBUG_RUN_INFO) printf("\nSTEP 12: Read the output buffer back to the host\n");
            
            status = clEnqueueReadImage(cmdQueue, 
                render_image, 
                CL_TRUE, 
                (size_t[3]){0, 0, 0},
                (size_t[3]){X_RES, Y_RES, 1}, 
                0, 
                0, 
                output_render_buffer, 
                0, 
                NULL, 
                NULL
            );//status = clEnqueueReadBuffer(cmdQueue, render_image, CL_TRUE, 0, RES *sizeof(rgb), output_render_buffer, 0, NULL,NULL);
            if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Read results\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

            new_frame = 1;
        }
        
    
        


        gettimeofday(&stop, NULL);
        //TODO: Check time
        long int sec = stop.tv_sec - start.tv_sec;
        long int milisec = (stop.tv_usec - start.tv_usec)/1000;
        long int microsec = (stop.tv_usec - start.tv_usec) %1000;
        if (DEBUG_RUN_INFO) printf("took %lis %lims %lius\n", sec, milisec, microsec); 
        
    } while(program_running_loop);
    
    save_to_file(output_render_buffer, "_output/output.ppm", X_RES, Y_RES);

    // STEP 13: Release OpenCL resources
    if (DEBUG_GLOBAL_INFO) printf("\nSTEP 13: Release OpenCL resources\n");
    clReleaseKernel(kernel);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(triangles_buffer);
    clReleaseMemObject(render_image);
    clReleaseMemObject(lights_buffer);
    clReleaseMemObject(textures_buffer);

    // Free host resources
    free(triangles);
    free(textures);
    free(lights);
    free(output_render_buffer);
    free(texture_map);
    free(normal_map);
}