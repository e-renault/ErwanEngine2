#ifndef OPENCL_TOOLS_H_
#define OPENCL_TOOLS_H_

cl_platform_id* init_platform();
cl_platform_id* init_platform() {
    cl_int status; int i;
    if (DEBUG_HARDWARE_INFO) printf("\n\n ### HARDWARE INFORMATIONS ###\n\n");
    
    // STEP 1: Discover and initialize the platforms  
    if (DEBUG_HARDWARE_INFO) printf(" *** Platform info *** \n");
    
    // Calcul du nombre de plateformes
    cl_uint numPlatforms = 0;  
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS || DEBUG_HARDWARE_INFO) printf("%s Number of platforms : [%d]\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG), numPlatforms);


    // Allocation de l'espace
    cl_platform_id* platforms = (cl_platform_id*) malloc(numPlatforms*sizeof(cl_platform_id));
    if (platforms == NULL || DEBUG_HARDWARE_INFO) printf("%s Memory platform allocation\n", (platforms != NULL)? SUCCESS_MSG:(ERROR_MSG));
    

    // Trouver les plateformes
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
    if (status != CL_SUCCESS || DEBUG_HARDWARE_INFO) printf("%s Platforms list\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    if (DEBUG_HARDWARE_INFO) {
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

cl_device_id* init_device(cl_platform_id* platforms, cl_uint* numDevices);
cl_device_id* init_device(cl_platform_id* platforms, cl_uint* numDevices) {
    cl_int status;int i;

    // STEP 2: Discover and initialize the devices
    if (DEBUG_HARDWARE_INFO) printf("\n *** Device info ***\n");

    // calcul du nombre de périphériques
    status = clGetDeviceIDs(platforms[USE_PLATFORM_ID], CL_DEVICE_TYPE_ALL, 0, NULL, numDevices);
    if (status != CL_SUCCESS || DEBUG_HARDWARE_INFO) printf("%s Number of devices : [%i]\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG), *numDevices);

    // Allocation de l'espace
    cl_device_id* devices = (cl_device_id*)malloc(*numDevices*sizeof(cl_device_id));
    if (devices == NULL || DEBUG_HARDWARE_INFO) printf("%s Memory devices allocation\n", (devices != NULL)? SUCCESS_MSG:(ERROR_MSG));
    
    // Trouver les périphériques
    status = clGetDeviceIDs(platforms[USE_PLATFORM_ID], CL_DEVICE_TYPE_ALL, *numDevices, devices, NULL);
    if (status != CL_SUCCESS || DEBUG_HARDWARE_INFO) printf("%s Devices list\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    if (DEBUG_HARDWARE_INFO) {
        char Name[1000];
        for (i=0; i<*numDevices; i++){
            status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(Name), Name, NULL);
            printf("%s Device %d info : \n\t - Name: %s\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG), i, Name);
        }
    }
    fflush(stdout);

    return devices;
}

cl_context init_context(cl_uint numDevices, cl_device_id* devices);
cl_context init_context(cl_uint numDevices, cl_device_id* devices) {
    cl_int status;

    if (DEBUG_HARDWARE_INFO) printf("\n\n ### SETUP CONFIG ###\n");
    if (DEBUG_HARDWARE_INFO) printf("\nSTEP 1: Create a context\n");

    // STEP 1: Create a context
    cl_context context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
    if (status != CL_SUCCESS || DEBUG_HARDWARE_INFO) printf("%s Create context\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    
    return context;
}

cl_command_queue init_queue(cl_context context, cl_device_id* devices);
cl_command_queue init_queue(cl_context context, cl_device_id* devices) {
    cl_int status;

    // STEP 2: Create a command queue
    if (DEBUG_HARDWARE_INFO) printf("\nSTEP 2: Create a command queue\n");

    cl_command_queue cmdQueue = clCreateCommandQueueWithProperties(context, devices[USE_DEVICE_ID], 0, &status);
    if (status != CL_SUCCESS || DEBUG_HARDWARE_INFO) printf("%s Create queue\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

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

cl_kernel init_kernel_program(cl_uint numDevices, cl_device_id* devices, cl_context context, char* source, char* function);
cl_kernel init_kernel_program(cl_uint numDevices, cl_device_id* devices, cl_context context, char* source, char* function) {
    cl_int status;
        
    // STEP 5: load programm source
    if (DEBUG_KERNEL_INFO) printf("\nSTEP 5: load programm source\n");
    
    char* programSource = load_program_source(source, &status);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO)  printf("%s Read cl programm\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));


    // STEP 6: Create and compile the program
    if (DEBUG_KERNEL_INFO) printf("\nSTEP 6: Create and compile the program\n");

    cl_program program = clCreateProgramWithSource(context, 1, (const char**) &programSource, NULL, &status);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO)  printf("%s Create program with source\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));


    status = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO)  printf("%s Compilation\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
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

    if (DEBUG_KERNEL_INFO) printf("\nSTEP 7: Create the kernel\n");

    // STEP 7: Create the kernel
    cl_kernel kernel = clCreateKernel(program, function, &status);
    if (status != CL_SUCCESS || DEBUG_KERNEL_INFO) printf("%s Create kernels\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    switch(status) {
        case CL_INVALID_PROGRAM:printf("CL_INVALID_PROGRAM\n");break;
        case CL_INVALID_PROGRAM_EXECUTABLE:printf("CL_INVALID_PROGRAM_EXECUTABLE\n");break;
        case CL_INVALID_KERNEL_NAME:printf("CL_INVALID_KERNEL_NAME\n");break;
        case CL_INVALID_KERNEL_DEFINITION:printf("CL_INVALID_KERNEL_DEFINITION\n");break;
        case CL_INVALID_VALUE:printf("CL_INVALID_VALUE\n");break;
        case CL_OUT_OF_HOST_MEMORY:printf("CL_OUT_OF_HOST_MEMORY\n");break;
    }

    clReleaseProgram(program);

    if (DEBUG_KERNEL_INFO) {//disabeled cause unused.    
        // STEP 8: Configure the work-item structure
        if (DEBUG_KERNEL_INFO) printf("\nSTEP 8: Configure the work-item structure\n");
        size_t MaxGroup;
        status = clGetDeviceInfo(devices[USE_DEVICE_ID],CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &MaxGroup, NULL);
        if (status != CL_SUCCESS || DEBUG_KERNEL_INFO)  {
            printf("%s Get device work group size\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
            printf(" - CL_DEVICE_MAX_WORK_GROUP_SIZE = %ld\n", MaxGroup);
        }

        size_t infosize;
        clGetDeviceInfo(devices[USE_DEVICE_ID], CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, NULL, &infosize);
        
        cl_ulong MaxItems[infosize];
        status = clGetDeviceInfo(devices[USE_DEVICE_ID], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(MaxItems), MaxItems, NULL);
        if (status != CL_SUCCESS || DEBUG_KERNEL_INFO)  {
            printf("%s Get device work group item size\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
            printf(" - CL_DEVICE_MAX_WORK_ITEM_SIZES = (%ld, %ld, %ld)\n", MaxItems[0], MaxItems[1], MaxItems[2]);
        }
    }
    
    return kernel;
}


#endif //OPENCL_TOOLS_H_