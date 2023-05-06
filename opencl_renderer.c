//gcc opencl_renderer.c -o demo -lm -lOpenCL -lpthread -lGL -lglut 
//-DX_RES=1920 -DY_RES=1080 -DDEBUG_INFO=1 -DDEBUG_SETUP_INFO=1 -DDEBUG_RUN_INFO=1 -DSHOW_FPS=1
//cd /media/erenault/EXCHANGE/workspaces/ErwanEngine2/opencl/

#define CL_TARGET_OPENCL_VERSION 300
#define USE_DEVICE_ID 0
#define USE_PLATFORM_ID 0

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h> //malloc
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>

#include "math/lib3D.h"
#include "math/lib3D_debug.h"
#include "render/image.h"
#include "render/color.h"
#include "scene/sceneLoader.h"
#include "kernel/header.h"

#ifndef DEBUG_INFO
    #define DEBUG_INFO 0
#endif
#ifndef DEBUG_SETUP_INFO
    #define DEBUG_SETUP_INFO 0
#endif
#ifndef DEBUG_RUN_INFO
    #define DEBUG_RUN_INFO 0
#endif
#ifndef SHOW_FPS
    #define SHOW_FPS 1
#endif

#define SUCCESS_MSG "(Done)"
#define ERROR_MSG "(Error)"

#ifndef X_RES
    #define X_RES 800
#endif
#ifndef Y_RES
    #define Y_RES 600
#endif
#define RES X_RES*Y_RES

//TODO use argument ?
#define MAX_NB_TRIANGLE 650

//TODO relocate
static rgb* output_buffer_ret;
Point3 cam_coordinate = {0.0, 1.2, 2.5};
Vector3 cam_lookat = {0, -0.1, -1};

static int program_running_loop = 1;
static int caracter_moved = 1;
static int new_frame = 1;

cl_platform_id* initPlatform();
cl_platform_id* initPlatform() {
    cl_int status; int i;
    if (DEBUG_INFO) printf("\n\n ### HARDWARE INFORMATIONS ###\n\n");
    
    // STEP 1: Discover and initialize the platforms  
    if (DEBUG_INFO) printf(" *** Platform info *** \n");
    
    // Calcul du nombre de plateformes
    cl_uint numPlatforms = 0;  
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS || DEBUG_INFO) printf("%s Number of platforms : [%d]\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG), numPlatforms);


    // Allocation de l'espace
    cl_platform_id* platforms = (cl_platform_id*) malloc(numPlatforms*sizeof(cl_platform_id));
    if (platforms == NULL || DEBUG_INFO) printf("%s Memory platform allocation\n", (platforms != NULL)? SUCCESS_MSG:(ERROR_MSG));
    

    // Trouver les plateformes
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
    if (status != CL_SUCCESS || DEBUG_INFO) printf("%s Platforms list\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    if (DEBUG_INFO) {
        char profile[1000], version[1000], name[1000], vendor[1000], extension[10000];
        status = clGetPlatformInfo(platforms[USE_PLATFORM_ID], CL_PLATFORM_PROFILE, sizeof(profile), profile, NULL);
        status = clGetPlatformInfo(platforms[USE_PLATFORM_ID], CL_PLATFORM_VERSION, sizeof(version), version, NULL);
        status = clGetPlatformInfo(platforms[USE_PLATFORM_ID], CL_PLATFORM_NAME, sizeof(name), name, NULL);
        status = clGetPlatformInfo(platforms[USE_PLATFORM_ID], CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
        status = clGetPlatformInfo(platforms[USE_PLATFORM_ID], CL_PLATFORM_EXTENSIONS, sizeof(extension), extension, NULL);
        i=0;while (extension[i]) { if (extension[i] == ' ') extension[i]='\n'; i++;}
        printf("%s Platform 0 infos : \n\t - profile : \t%s\n\t - version : \t%s\n\t - name : \t\t%s\n\t - vendor : \t%s\n\t - extensions : \n%s\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG), profile, version, name, vendor, extension);
        switch(status) {
            case CL_INVALID_PLATFORM:printf("CL_INVALID_PLATFORM\n");break;
            case CL_INVALID_VALUE:printf("CL_INVALID_VALUE\n");break;
        }
    }
    fflush(stdout);
    return platforms;
}

cl_device_id* initDevice(cl_platform_id* platforms, cl_uint* numDevices);
cl_device_id* initDevice(cl_platform_id* platforms, cl_uint* numDevices) {
    cl_int status;int i;

    // STEP 2: Discover and initialize the devices
    if (DEBUG_INFO) printf("\n *** Device info ***\n");

    // calcul du nombre de périphériques
    status = clGetDeviceIDs(platforms[USE_PLATFORM_ID], CL_DEVICE_TYPE_ALL, 0, NULL, numDevices);
    if (status != CL_SUCCESS || DEBUG_INFO) printf("%s Number of devices : [%i]\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG), *numDevices);

    // Allocation de l'espace
    cl_device_id* devices = (cl_device_id*)malloc(*numDevices*sizeof(cl_device_id));
    if (devices == NULL || DEBUG_INFO) printf("%s Memory devices allocation\n", (devices != NULL)? SUCCESS_MSG:(ERROR_MSG));
    
    // Trouver les périphériques
    status = clGetDeviceIDs(platforms[USE_PLATFORM_ID], CL_DEVICE_TYPE_ALL, *numDevices, devices, NULL);
    if (status != CL_SUCCESS || DEBUG_INFO) printf("%s Devices list\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    if (DEBUG_INFO) {
        char Name[1000];
        for (i=0; i<*numDevices; i++){
            status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(Name), Name, NULL);
            printf("%s Device %d info : \n\t - Name: %s\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG), i, Name);
        }
    }
    fflush(stdout);

    return devices;
}

cl_context initContext(cl_uint numDevices, cl_device_id* devices);
cl_context initContext(cl_uint numDevices, cl_device_id* devices) {
    cl_int status;

    if (DEBUG_SETUP_INFO) printf("\n\n ### SETUP CONFIG ###\n");
    if (DEBUG_SETUP_INFO) printf("\nSTEP 1: Create a context\n");

    // STEP 1: Create a context
    cl_context context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Create context\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    return context;
}

cl_command_queue initQueue(cl_context context, cl_device_id* devices);
cl_command_queue initQueue(cl_context context, cl_device_id* devices) {
    cl_int status;

    // STEP 2: Create a command queue
    if (DEBUG_SETUP_INFO) printf("\nSTEP 2: Create a command queue\n");

    cl_command_queue cmdQueue = clCreateCommandQueueWithProperties(context, devices[USE_DEVICE_ID], 0, &status);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Create queue\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    return cmdQueue;
}

char* load_program_source(const char *filename, cl_int* status);
char* load_program_source(const char *filename, cl_int* status_ret) {
    FILE *fp;
    char *source;
    int sz=0;
    struct stat status;

    fp = fopen(filename, "r");
    if (fp == 0){
        printf("Source opening failed\n");
        *status_ret = 0;
        return 0;
    }

    if (stat(filename, &status) == 0)
        sz = (int) status.st_size;

    source = (char *) malloc(sz + 1);
    fread(source, sz, 1, fp);
    source[sz] = '\0';
    
    *status_ret = CL_SUCCESS;
    
    return source;
}

cl_kernel initKernelProgram(cl_uint numDevices, cl_device_id* devices, cl_context context, char* source, char* function);
cl_kernel initKernelProgram(cl_uint numDevices, cl_device_id* devices, cl_context context, char* source, char* function) {
    cl_int status;
        
    // STEP 5: load programm source
    if (DEBUG_SETUP_INFO) printf("\nSTEP 5: load programm source\n");
    
    char* programSource = load_program_source(source, &status);
    if (status != CL_SUCCESS || DEBUG_INFO)  printf("%s Read cl programm\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));


    // STEP 6: Create and compile the program
    if (DEBUG_SETUP_INFO) printf("\nSTEP 6: Create and compile the program\n");

    cl_program program = clCreateProgramWithSource(context, 1, (const char**) &programSource, NULL, &status);
    if (status != CL_SUCCESS || DEBUG_INFO)  printf("%s Create program with source\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));


    status = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);
    if (status != CL_SUCCESS || DEBUG_INFO)  printf("%s Compilation\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    switch(status) {
        case CL_INVALID_PROGRAM:printf("CL_INVALID_PROGRAM\n");break;
        case CL_INVALID_VALUE:printf("CL_INVALID_VALUE\n");break;
        case CL_INVALID_DEVICE:printf("CL_INVALID_DEVICE\n");break;
        case CL_INVALID_BINARY:printf("CL_INVALID_BINARY\n");break;
        case CL_INVALID_BUILD_OPTIONS:printf("CL_INVALID_BUILD_OPTIONS\n");break;
        case CL_COMPILER_NOT_AVAILABLE:printf("CL_COMPILER_NOT_AVAILABLE\n");break;
        case CL_BUILD_PROGRAM_FAILURE:printf("CL_BUILD_PROGRAM_FAILURE\n");break;
        case CL_INVALID_OPERATION:printf("CL_INVALID_OPERATION\n");break;
        case CL_OUT_OF_RESOURCES:printf("CL_OUT_OF_RESOURCES\n");break;
        case CL_OUT_OF_HOST_MEMORY:printf("CL_OUT_OF_HOST_MEMORY\n");break;
    }

    if (status == CL_BUILD_PROGRAM_FAILURE) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(program, devices[USE_DEVICE_ID], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, devices[USE_DEVICE_ID], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);
    }

    if (DEBUG_SETUP_INFO) printf("\nSTEP 7: Create the kernel\n");

    // STEP 7: Create the kernel
    cl_kernel kernel = clCreateKernel(program, function, &status);
    if (status != CL_SUCCESS || DEBUG_INFO) printf("%s Create kernels\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    switch(status) {
        case CL_INVALID_PROGRAM:printf("CL_INVALID_PROGRAM\n");break;
        case CL_INVALID_PROGRAM_EXECUTABLE:printf("CL_INVALID_PROGRAM_EXECUTABLE\n");break;
        case CL_INVALID_KERNEL_NAME:printf("CL_INVALID_KERNEL_NAME\n");break;
        case CL_INVALID_KERNEL_DEFINITION:printf("CL_INVALID_KERNEL_DEFINITION\n");break;
        case CL_INVALID_VALUE:printf("CL_INVALID_VALUE\n");break;
        case CL_OUT_OF_HOST_MEMORY:printf("CL_OUT_OF_HOST_MEMORY\n");break;
    }

    clReleaseProgram(program);

    if (DEBUG_SETUP_INFO) {//disabeled cause unused.    
        // STEP 8: Configure the work-item structure
        if (DEBUG_SETUP_INFO) printf("\nSTEP 8: Configure the work-item structure\n");
        size_t MaxGroup;
        status = clGetDeviceInfo(devices[USE_DEVICE_ID],CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &MaxGroup, NULL);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO)  {
            printf("%s Get device work group size\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
            printf(" - CL_DEVICE_MAX_WORK_GROUP_SIZE = %ld\n", MaxGroup);
        }

        size_t infosize;
        clGetDeviceInfo(devices[USE_DEVICE_ID], CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, NULL, &infosize);
        
        cl_ulong MaxItems[infosize];
        status = clGetDeviceInfo(devices[USE_DEVICE_ID], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(MaxItems), MaxItems, NULL);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO)  {
            printf("%s Get device work group item size\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
            printf(" - CL_DEVICE_MAX_WORK_ITEM_SIZES = (%ld, %ld, %ld)\n", MaxItems[0], MaxItems[1], MaxItems[2]);
        }
    }
    
    return kernel;
}


/**  DISPLAY SECTION  **/
void display();
void display(){
    if (new_frame) {
        new_frame=0;
        
        glClearColor( 0, 0, 0, 1 );
        glClear( GL_COLOR_BUFFER_BIT );

        //draw pixels
        glDrawPixels( X_RES, Y_RES, GL_RGB, GL_FLOAT, output_buffer_ret );
        glutSwapBuffers();
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int xmouse, int ymouse);
void keyboard(unsigned char key, int xmouse, int ymouse) {
    Vector3 UP = {0,1,0};
    Vector3 rightVector = crossProduct(cam_lookat, UP);
    switch (key) {
        case 'z':
            cam_coordinate = toPoint(add_vector3(toVector(cam_coordinate), scale_vector3(0.1, cam_lookat)));
            break;
        case 's':
            cam_coordinate = toPoint(add_vector3(toVector(cam_coordinate), scale_vector3(-0.1f, cam_lookat)));
            break;
        case 'q':
            cam_coordinate = toPoint(add_vector3(toVector(cam_coordinate), scale_vector3(0.1, rightVector)));
            break;
        case 'd':
            cam_coordinate = toPoint(add_vector3(toVector(cam_coordinate), scale_vector3(-0.1f, rightVector)));
            break;
        case ' ':
            cam_coordinate.y += 0.1;
            break;
        case 'e':
            cam_coordinate.y -= 0.1;
            break;
        case 'm':
            cam_lookat = rotateAround(cam_lookat, UP, 0.1f);
            break;
        case 'k':
            cam_lookat = rotateAround(cam_lookat, UP, -0.1f);
            break;
        case 'o':
            cam_lookat = rotateAround(cam_lookat, rightVector, 0.1f);
            break;
        case 'l':
            cam_lookat = rotateAround(cam_lookat, rightVector, -0.1f);
            break;
        case 'x':
            program_running_loop = 0;
            break;
        default:
            break;
    }
    caracter_moved = 1;
}

void* start_glut(void *vargp);
void* start_glut(void *vargp) {
    printf("GLUT Start\n");
    //init GLUT
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(X_RES, Y_RES);
    glutCreateWindow("ErwanEngine2.0");
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    //display loop
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    program_running_loop = 0;
    printf("GLUT Exit\n");
}



int main(int argc, char *argv[]) {
    cl_int status;
    cl_int x, y, i;
    
    /** Init datas **/
    if (DEBUG_INFO) printf(" ### Starting program, creating datas, starting threads ###");
    
    pthread_t glut_thread_id;
    glutInit(&argc, argv);
    status = pthread_create(&glut_thread_id, NULL, start_glut, NULL);
    if (DEBUG_INFO) printf("%s Creating GLUT thread\n", (status == 1)? SUCCESS_MSG:(ERROR_MSG));



    cl_platform_id* platforms = initPlatform();

    cl_uint numDevices = 0;
    cl_device_id* devices = initDevice(platforms, &numDevices);
    free(platforms);

    cl_context context = initContext(numDevices, devices);
    
    cl_command_queue cmdQueue = initQueue(context, devices);

    cl_kernel kernel = initKernelProgram(numDevices, devices, context, "kernel/projection_kernel.cl", "simpleCast");
    clReleaseContext(context);
    free(devices);


    
    // STEP 3: Create device buffers
    if (DEBUG_SETUP_INFO) printf("\nSTEP 3: Create device buffers\n");
    
    cl_mem z_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, RES * sizeof(cl_float), NULL, &status);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Create buffers 1\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    cl_mem triangle_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, MAX_NB_TRIANGLE * sizeof(Triangle3), NULL, &status);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Create buffers 2\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    //TODO: could be set as write only
    cl_mem output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, RES * sizeof(rgb), NULL, &status);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Create buffers output\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    output_buffer_ret = (rgb*) calloc(RES, sizeof(rgb));
    cl_float* zbuffer_ret = (cl_float*) calloc(RES, sizeof(cl_float));



    //scene build
    Triangle3* triangles = (Triangle3*) calloc(MAX_NB_TRIANGLE, sizeof(Triangle3));
    cl_int real_nb_triangles = 0;

    if (argc > 1) { //load scene if provided
        char path[500] = "/media/erenault/EXCHANGE/workspaces/ErwanEngine2/opencl/scene/";
        strcat(path, argv[1]);
        printf("Loading : %s\n", path);
        Point3 points[400];
        char scene_name[50];
        char object[50];
        real_nb_triangles = loadSceneFromFile(path, points, triangles, scene_name, object);
    }
    

    int frame = 0, timebase = 0;//used for FPS counter
    do {
        if (argc == 1) { //default if no scene loaded (bouncing square)
            struct timeval time;
            gettimeofday(&time, NULL);
            float _sec = ((float) time.tv_usec/1000000) + time.tv_sec % 100;
            real_nb_triangles = loadCubeScene(_sec, triangles);
        }
        

        // STEP 4: Write host data to device buffers
        if (DEBUG_SETUP_INFO) printf("\nSTEP 4: Write host data to device buffers\n");
        
        //status = clEnqueueWriteBuffer(cmdQueue, z_buffer, CL_TRUE, 0, RES * sizeof(cl_float), zbuffer_ret, 0, NULL, NULL);
        //if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Write buffers 1\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        status = clEnqueueWriteBuffer(cmdQueue, triangle_buffer, CL_TRUE, 0, real_nb_triangles * sizeof(Triangle3), triangles, 0, NULL, NULL);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Write buffers 2\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        //status = clEnqueueWriteBuffer(cmdQueue, output_buffer, CL_TRUE, 0, RES * sizeof(rgb), triangles, 0, NULL, NULL);
        //if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Write buffers 2\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));









        // STEP 9: Set the kernel arguments
        if (DEBUG_SETUP_INFO) printf("\nSTEP 9: Set the kernel arguments\n");

        cl_int x_res = X_RES;
        status = clSetKernelArg(kernel, 0, sizeof(cl_int), (void*) &x_res);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 0\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        cl_int y_res = Y_RES;
        status = clSetKernelArg(kernel, 1, sizeof(cl_int), (void*) &y_res);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 1\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        status = clSetKernelArg(kernel, 2, sizeof(Point3), (void*) &cam_coordinate);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 2\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        status = clSetKernelArg(kernel, 3, sizeof(Vector3), (void*) &cam_lookat);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 3\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        status = clSetKernelArg(kernel, 4, sizeof(cl_int), (void*) &real_nb_triangles);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 4\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        status = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*) &triangle_buffer);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 5\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        status = clSetKernelArg(kernel, 6, sizeof(cl_mem), (void*) &z_buffer);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 6\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        status = clSetKernelArg(kernel, 7, sizeof(cl_mem), (void*) &output_buffer);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 7\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));





    //do {    
        // STEP 10: Enqueue the kernel for execution
        if (DEBUG_SETUP_INFO)  printf("\nSTEP 10: Enqueue the kernel for execution\n");
        fflush(stdout);
        
        struct timeval stop, start;
        gettimeofday(&start, NULL);

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
        
        gettimeofday(&stop, NULL);
        //TODO: Check time
        unsigned long sec = stop.tv_sec - start.tv_sec;
        unsigned long milisec = (stop.tv_usec - start.tv_usec)/1000;
        unsigned long microsec = (stop.tv_usec - start.tv_usec) %1000;
        if (DEBUG_RUN_INFO) printf("took %lus %lums %luus\n", sec, milisec, microsec); 
        
        //FPS Counter
        frame++;
        if (start.tv_sec > timebase) {
            if (SHOW_FPS) printf("FPS:%i   \r", frame);fflush(stdout);
            timebase = start.tv_sec;
            frame = 0;
        }

        // STEP 12: Read the output buffer back to the host
        if (DEBUG_RUN_INFO) printf("\nSTEP 12: Read the output buffer back to the host\n");
        
        status = clEnqueueReadBuffer(cmdQueue, output_buffer, CL_TRUE, 0, RES *sizeof(rgb), output_buffer_ret, 0, NULL,NULL);
        if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Read results\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        status = clEnqueueReadBuffer(cmdQueue, z_buffer, CL_TRUE, 0, RES *sizeof(int), zbuffer_ret, 0, NULL,NULL);
        if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Read results\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

        new_frame = 1;
    } while(program_running_loop);
    

    //image generation
    frameRGB image = newFrame(X_RES, Y_RES);
    for (x = 0; x<X_RES; x++) {
        for (y = 0; y<Y_RES; y++) {
            image.frame[y *X_RES + x] = output_buffer_ret[y *X_RES + x];
        }
    }
    generateImg(image, "output");

    // STEP 13: Release OpenCL resources
    if (DEBUG_SETUP_INFO) printf("\nSTEP 13: Release OpenCL resources\n");
    clReleaseKernel(kernel);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(z_buffer);
    clReleaseMemObject(triangle_buffer);
    clReleaseMemObject(output_buffer);

    // Free host resources
    free(triangles);
    free(output_buffer_ret);
    free(zbuffer_ret);

    //test
}