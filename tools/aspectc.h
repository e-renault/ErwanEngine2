#ifndef ASPECTC_H_
#define ASPECTC_H_


cl_mem aspectC_clCreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void * host_ptr, cl_int * errcode_ret) {
    cl_int status;
    cl_mem ret = clCreateBuffer(context, flags, size, host_ptr, &status);
    if (status != CL_SUCCESS) printf("%s Creating buffer error\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    return ret;
}

cl_int aspectC_clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void * arg_value) {
    cl_int status;
    status = clSetKernelArg(kernel, arg_index, arg_size, arg_value);
    if (status != CL_SUCCESS) printf("%s Set arg\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    return status;
}


cl_mem aspectC_clCreateImage(cl_context context, cl_mem_flags flags, const cl_image_format * image_format, const cl_image_desc * image_desc, void * host_ptr, cl_int * errcode_ret) {
    cl_int status;
    cl_mem ret = clCreateImage(context, flags, image_format, image_desc, host_ptr, &status);
    if (status != CL_SUCCESS) printf("%s Create image\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
    switch (status) {
        case CL_INVALID_CONTEXT: printf("CL_INVALID_CONTEXT\n");break;
        case CL_INVALID_PROPERTY: printf("CL_INVALID_PROPERTY\n");break;
        case CL_INVALID_VALUE: printf("CL_INVALID_VALUE\n");break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: printf("CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");break;
        case CL_INVALID_IMAGE_DESCRIPTOR: printf("CL_INVALID_IMAGE_DESCRIPTOR\n");break;
        case CL_INVALID_IMAGE_SIZE: printf("CL_INVALID_IMAGE_SIZE\n");break;
        case CL_INVALID_HOST_PTR: printf("CL_INVALID_HOST_PTR\n");break; 
        case CL_IMAGE_FORMAT_NOT_SUPPORTED: printf("CL_IMAGE_FORMAT_NOT_SUPPORTED\n");break; 
        case CL_MEM_OBJECT_ALLOCATION_FAILURE: printf("CL_MEM_OBJECT_ALLOCATION_FAILURE\n");break; 
        case CL_INVALID_OPERATION: printf("CL_INVALID_OPERATION\n");break; 
        case CL_OUT_OF_RESOURCES: printf("CL_OUT_OF_RESOURCES\n");break; 
        case CL_OUT_OF_HOST_MEMORY: printf("CL_OUT_OF_HOST_MEMORY\n");break; 
    }
    return ret;
}


cl_int aspectC_clEnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t size, const void * ptr, cl_uint num_events_in_wait_list, const cl_event * event_wait_list, cl_event * event) {
    cl_int status = clEnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
    if (status != CL_SUCCESS) printf("%s Write buffer\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));

    return status;
}


cl_int aspectC_clEnqueueWriteImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t * origin, const size_t * region, size_t input_row_pitch, size_t input_slice_pitch, const void * ptr, cl_uint num_events_in_wait_list, const cl_event * event_wait_list, cl_event * event) {
    cl_int status = clEnqueueWriteImage(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
    if (status != CL_SUCCESS) printf("%s Write texture image\n", (status == CL_SUCCESS)? SUCCESS_MSG:(ERROR_MSG));
        
    return status;
}


cl_int aspectC_clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t * global_work_offset, const size_t * global_work_size, const size_t * local_work_size, cl_uint num_events_in_wait_list, const cl_event * event_wait_list, cl_event * event) {
    cl_int status = clEnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
    
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

    return status;
}

#endif //ASPECTC_H_