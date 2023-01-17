#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h> //malloc
#include <math.h>
#include <sys/time.h>

#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <stdio.h>

#include "math/lib3D.h"
#include "math/lib3D_debug.h"
#include "render/image.h"
#include "render/color.h"


#define CL_TARGET_OPENCL_VERSION 300
#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif

#define USE_DEVICE_ID 0
#define USE_PLATFORM_ID 0

#define DEBUG_INFO 0
#define DEBUG_SETUP_INFO 0
#define DEBUG_RUN_INFO 0
#define DEBUG_RUN_TIME 0
#define SUCCESS_MSG "(Done)"
#define ERROR_MSG "(Error)"


const unsigned int x_res = 400;
const unsigned int y_res = 300;
float* sharedData;//displayed data buffer
int frame, time, timebase;//used for FPS counter


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

cl_kernel initKernelProgram(cl_uint numDevices, cl_device_id* devices, cl_context context, char* source);
cl_kernel initKernelProgram(cl_uint numDevices, cl_device_id* devices, cl_context context, char* source) {
    cl_int status;
        
    // STEP 5: load programm source
    if (DEBUG_SETUP_INFO) printf("\nSTEP 5: load programm source\n");
    
    char* programSource = load_program_source("kernel/projection_kernel.cl", &status);
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

    cl_kernel kernel = clCreateKernel(program, "simpleCast", &status);
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
    
    return kernel;
}

void display();
void display(){
    glClearColor( 0, 0, 0, 1 );
    glClear( GL_COLOR_BUFFER_BIT );


    //FPS Counter
    frame++;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
        if (DEBUG_RUN_TIME) printf("FPS:%4.2f\n", frame*1000.0/(time-timebase));fflush(stdout);
	 	timebase = time;
		frame = 0;
    }

    //draw pixels
    glDrawPixels( x_res, y_res, GL_RGB, GL_FLOAT, sharedData );
    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char *argv[]) {
    cl_int status;
    cl_int res, x, y, i;
    
    /** Init datas **/
    if (DEBUG_INFO) printf(" ### Starting program, creating datas ###");
    
    //init GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(x_res, y_res);
    glutCreateWindow("ErwanEngine2.0");
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    //GLUT FPS counter
    frame=0;time=0;timebase=0;

    //scene build
    res = x_res * y_res;
    Point3 cam_coordinate = {0.2, 0.5, 2.5};
    Vector3 cam_lookat = {0.20, 0, -1};
    cl_int nbTriangles = 11;
    Triangle3* triangles = (Triangle3*) calloc(nbTriangles, sizeof(Triangle3));

    Point3 p000 = {0, 0, 0};
    Point3 p100 = {1, 0, 0};
    Point3 p110 = {1, 1, 0};
    Point3 p010 = {0, 1, 0};
    Point3 p001 = {0, 0, 1};
    Point3 p101 = {1, 0, 1};
    Point3 p111 = {1, 1, 1};
    Point3 p011 = {0, 1, 1};

    Point3 pla = {-0.001, -150, -100};
    Point3 plb = {-0.001, 50, -100};
    Point3 plc = {-0.001, 50, 100};

    triangles[0] = newTriangle3(pla, plb, plc);
    triangles[10] = newTriangle3(p000, p100, p110);
    triangles[1] = newTriangle3(p000, p010, p110);
    triangles[2] = newTriangle3(p100, p101, p111);
    triangles[3] = newTriangle3(p100, p110, p111);
    triangles[4] = newTriangle3(p010, p110, p111);
    triangles[5] = newTriangle3(p010, p011, p111);
    triangles[6] = newTriangle3(p000, p100, p101);
    triangles[7] = newTriangle3(p000, p101, p001);
    triangles[8] = newTriangle3(p000, p001, p011);
    triangles[9] = newTriangle3(p000, p010, p011);



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
    






    
    // STEP 2: Discover and initialize the devices
    if (DEBUG_INFO) printf("\n *** Device info ***\n");

    // calcul du nombre de périphériques
    cl_uint numDevices = 0;
    status = clGetDeviceIDs(platforms[USE_PLATFORM_ID], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
    if (status != CL_SUCCESS || DEBUG_INFO) printf("%s Number of devices : [%i]\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG), numDevices);


    // Allocation de l'espace
    cl_device_id* devices = (cl_device_id*)malloc(numDevices*sizeof(cl_device_id));
    if (devices == NULL || DEBUG_INFO) printf("%s Memory devices allocation\n", (devices != NULL)? SUCCESS_MSG:(ERROR_MSG));
    
    // Trouver les périphériques
    status = clGetDeviceIDs(platforms[USE_PLATFORM_ID], CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);
    if (status != CL_SUCCESS || DEBUG_INFO) printf("%s Devices list\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    if (DEBUG_INFO) {
        char Name[1000];
        for (i=0; i<numDevices; i++){
            status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(Name), Name, NULL);
            printf("%s Device %d info : \n\t - Name: %s\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG), i, Name);
        }
    }
    fflush(stdout);
    free(platforms);









    if (DEBUG_SETUP_INFO) printf("\n\n ### SETUP CONFIG ###\n");
    if (DEBUG_SETUP_INFO) printf("\nSTEP 1: Create a context\n");

    // STEP 1: Create a context
    cl_context context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Create context\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    
    // STEP 2: Create a command queue
    if (DEBUG_SETUP_INFO) printf("\nSTEP 2: Create a command queue\n");

    cl_command_queue cmdQueue = clCreateCommandQueueWithProperties(context, devices[USE_DEVICE_ID], 0, &status);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Create queue\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    // STEP 3: Create device buffers
    // STEP 4: Write host data to device buffers
    if (DEBUG_SETUP_INFO) printf("\nSTEP 3: Create device buffers & STEP 4: Write host data to device buffers\n");
    
    cl_mem z_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, res * sizeof(cl_float), NULL, &status);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Create buffers 1\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    //status = clEnqueueWriteBuffer(cmdQueue, z_buffer, CL_TRUE, 0, res * sizeof(cl_float), zbuffer_ret, 0, NULL, NULL);
    //if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Write buffers 1\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    cl_mem triangle_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, nbTriangles * sizeof(Triangle3), NULL, &status);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Create buffers 2\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    status = clEnqueueWriteBuffer(cmdQueue, triangle_buffer, CL_TRUE, 0, nbTriangles * sizeof(Triangle3), triangles, 0, NULL, NULL);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Write buffers 2\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    //TODO: could be set as write only
    cl_mem output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, res * sizeof(rgb), NULL, &status);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Create buffers output\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    //status = clEnqueueWriteBuffer(cmdQueue, output_buffer, CL_TRUE, 0, res * sizeof(rgb), triangles, 0, NULL, NULL);
    //if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Write buffers 2\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));








    // STEP 5: load programm source
    // STEP 6: Create and compile the program
    // STEP 7: Create the kernel
    cl_kernel kernel = initKernelProgram(numDevices, devices, context, "test");

    if (DEBUG_SETUP_INFO) {//disabeled cause unused.    
        // STEP 8: Configure the work-item structure
        if (DEBUG_SETUP_INFO) printf("\nSTEP 8: Configure the work-item structure\n");
        size_t MaxGroup;
        status = clGetDeviceInfo(devices[USE_DEVICE_ID],CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &MaxGroup, NULL);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO)  {
            printf("%s Get device work group size\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
            printf(" - CL_DEVICE_MAX_WORK_GROUP_SIZE = %d\n", MaxGroup);
        }

        size_t infosize;
        clGetDeviceInfo(devices[USE_DEVICE_ID], CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, NULL, &infosize);
        
        cl_ulong MaxItems[infosize];
        status = clGetDeviceInfo(devices[USE_DEVICE_ID], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(MaxItems), MaxItems, NULL);
        if (status != CL_SUCCESS || DEBUG_SETUP_INFO)  {
            printf("%s Get device work group item size\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
            printf(" - CL_DEVICE_MAX_WORK_ITEM_SIZES = (%d, %d, %d)\n", MaxItems[0], MaxItems[1], MaxItems[2]);
        }
    }
    
    clReleaseContext(context);
    free(devices);




    // STEP 9: Set the kernel arguments
    if (DEBUG_SETUP_INFO) printf("\nSTEP 9: Set the kernel arguments\n");

    status = clSetKernelArg(kernel, 0, sizeof(cl_int), (void*) &x_res);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 0\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    status = clSetKernelArg(kernel, 1, sizeof(cl_int), (void*) &y_res);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 1\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    status = clSetKernelArg(kernel, 2, sizeof(Point3), (void*) &cam_coordinate);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 2\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    status = clSetKernelArg(kernel, 3, sizeof(Vector3), (void*) &cam_lookat);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 3\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    status = clSetKernelArg(kernel, 4, sizeof(cl_int), (void*) &nbTriangles);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 4\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    status = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*) &triangle_buffer);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 5\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    status = clSetKernelArg(kernel, 6, sizeof(cl_mem), (void*) &z_buffer);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 6\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    status = clSetKernelArg(kernel, 7, sizeof(cl_mem), (void*) &output_buffer);
    if (status != CL_SUCCESS || DEBUG_SETUP_INFO) printf("%s Set arg 7\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));


    
    // STEP 10: Enqueue the kernel for execution
    if (DEBUG_SETUP_INFO)  printf("\nSTEP 10: Enqueue the kernel for execution\n");
    fflush(stdout);
    
    struct timeval stop, start;
    gettimeofday(&start, NULL);

    size_t globalWorkSize[2]={x_res, y_res};
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
    printf("took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec); 
    
    
    // STEP 12: Read the output buffer back to the host
    if (DEBUG_RUN_INFO) printf("\nSTEP 12: Read the output buffer back to the host\n");
    
    rgb* output_buffer_ret = (rgb*) calloc(res, sizeof(rgb));
    status = clEnqueueReadBuffer(cmdQueue, output_buffer, CL_TRUE, 0, res *sizeof(rgb), output_buffer_ret, 0, NULL,NULL);
    if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Read results\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));


    cl_float* zbuffer_ret = (cl_float*) calloc(res, sizeof(cl_float));
    status = clEnqueueReadBuffer(cmdQueue, z_buffer, CL_TRUE, 0, res *sizeof(int), zbuffer_ret, 0, NULL,NULL);
    if (status != CL_SUCCESS || DEBUG_RUN_INFO) printf("%s Read results\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    

    //image generation
    frameRGB image = newFrame(x_res, y_res);
    for (x = 0; x<x_res; x++) {
        for (y = 0; y<y_res; y++) {
            image.frame[y *x_res + x] = output_buffer_ret[y *x_res + x];
        }
    }
    generateImg(image, "output");

    //recover data
    float* mdata = calloc(x_res*y_res*3, sizeof(float));
    for( x = 0; x < x_res; ++x ) {
        for( y = 0; y < y_res; ++y ) {
            int pointer =       (y * x_res + x) * 3;
            int inv_pointer =   (y_res-y) * x_res + x;
            mdata[pointer + 0] = output_buffer_ret[inv_pointer].r;
            mdata[pointer + 1] = output_buffer_ret[inv_pointer].g;
            mdata[pointer + 2] = output_buffer_ret[inv_pointer].b;
        }
    }
    sharedData = mdata;

    //display loop
    glutDisplayFunc(display);
    glutMainLoop();
    

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