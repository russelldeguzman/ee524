// Add you host code
#include<stdio.h>
#include<stdlib.h>
#include"CL/cl.h"
#include <string.h>
#include "read_source.h"
#define ERR 0xdeadbeef
#define NUM_ITEMS	16
#define BUFFER_SIZE (NUM_ITEMS * sizeof(float))
enum {
	WORK_DIM_0,
	NUM_WORK_DIMS,
};

static const char s_target_platform[] = "Intel(R) OpenCL";
static const char s_device_name_substring[] = "Intel";
const size_t global_work_size[NUM_WORK_DIMS] = { NUM_ITEMS };
const size_t local_work_size[NUM_WORK_DIMS] = { NUM_ITEMS };

static char * _get_platform_name(cl_platform_id platform_id);
static bool verifyResults(float *p_mappedBufferIN, float *p_mappedBufferOut, cl_int numValues);


int main(int argc, char** argv) 
{
	cl_platform_id *platforms = NULL;
	cl_uint num_platforms = 0;

	cl_device_id *device_list = NULL;
	cl_uint num_devices = 0;
	char * platform_name = NULL;
	cl_int clStatus;
	size_t param_size;
	char* device_name = NULL;
	cl_device_fp_config dev_fp_config = 0;
	cl_uint dev_preferred_vector_width_float = 0;
	cl_uint dev_clock_freq = 0;
	cl_uint dev_max_compute_units = 0;
	cl_uint dev_max_item_dimensions = 0;
	size_t *dev_max_work_item_sizes = 0;
	cl_uint target_platform_index = 0;
	cl_platform_id target_platform_id = 0;
	cl_device_id target_device_id = 0;
	cl_uint total_devices = 0;
	cl_context context = NULL;
	cl_context_properties context_properties = NULL;
	char * file_contents = NULL;
	size_t file_size = 0;
	cl_program program = NULL;
	cl_command_queue command_queue = NULL;
	cl_kernel kernel = NULL;
	float * in_buf_a;
	float * in_buf_b;
	float * out_buf;
	cl_mem bufa = NULL;
	cl_mem bufb = NULL;
	cl_mem outbuf = NULL;
	cl_uint work_dim = 0;
	float * mapped_buffer = NULL;

	// Get Platform Info
	clStatus |= clGetPlatformIDs(0, NULL, &num_platforms);
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id)* num_platforms);
	clStatus |= clGetPlatformIDs(num_platforms, platforms, NULL);
	// Get device info
	for (cl_uint i = 0; i < num_platforms; i++) {
		clStatus |= clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, NULL, NULL, &param_size);
		platform_name = (char *)malloc(sizeof(char) * param_size);
		clStatus |= clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, param_size, platform_name, NULL);
		printf("%s\n", platform_name);
		clStatus |= clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
		total_devices += num_devices;
		device_list = (cl_device_id*)realloc(device_list,sizeof(cl_device_id)* total_devices);
		clStatus |= clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_devices, (device_list + total_devices - num_devices), NULL);
		for (int j = 0; j < num_devices; j++)
		{
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_NAME, NULL, NULL, &param_size);
			device_name = (char *)malloc(sizeof(char) * param_size);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_NAME, param_size, device_name, NULL);
			printf("\t%s\n", device_name);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_DOUBLE_FP_CONFIG, NULL, NULL, &param_size);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_DOUBLE_FP_CONFIG, param_size, &dev_fp_config, NULL);
			printf("\tFP Config:%d\n", (int)dev_fp_config);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, NULL, NULL, &param_size);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, param_size, &dev_preferred_vector_width_float, NULL);
			printf("\tPreferred vector float width :%d\n", (int)dev_preferred_vector_width_float);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, NULL, NULL, &param_size);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, param_size, &dev_clock_freq, NULL);
			printf("\tMax Clk Freq:%d\n", (int)dev_clock_freq);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_MAX_COMPUTE_UNITS, NULL, NULL, &param_size);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_MAX_COMPUTE_UNITS, param_size, &dev_max_compute_units, NULL);
			printf("\tMax Clk Freq:%d\n", (int)dev_max_compute_units);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, NULL, NULL, &param_size);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, param_size, &dev_max_item_dimensions, NULL);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_MAX_WORK_ITEM_SIZES, NULL, NULL, &param_size);
			dev_max_work_item_sizes = (size_t *)malloc(sizeof(size_t) * param_size);
			clStatus |= clGetDeviceInfo(device_list[j], CL_DEVICE_MAX_WORK_ITEM_SIZES, param_size, dev_max_work_item_sizes, NULL);
			for (int k = 0; k < dev_max_item_dimensions; k++)
			{
				printf("\t\tMax work item sizes:%d\n", (int)dev_max_work_item_sizes[k]);
			}
			free(dev_max_work_item_sizes);
			free(device_name);
		}
		free(platform_name);
	}

	//Get “Intel(R) OpenCL”
	for (cl_uint id = 0; id < num_platforms; id++)
	{
		platform_name = _get_platform_name(platforms[id]);
		if (strcmp(s_target_platform, platform_name) == 0)
		{
			target_platform_index = id;
			clStatus |= clGetDeviceIDs(platforms[id], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
		}
	}
	target_platform_id = platforms[target_platform_index];

	//Get GPU
	//Note: Assuming we only need one GPU
	for (cl_uint id = 0; id < total_devices; id++)
	{
		if (device_list != NULL)
		{
			clStatus |= clGetDeviceInfo(device_list[id], CL_DEVICE_NAME, NULL, NULL, &param_size);
			device_name = (char *)malloc(sizeof(char) * param_size);
			clStatus |= clGetDeviceInfo(device_list[id], CL_DEVICE_NAME, param_size, device_name, NULL);
			if (strstr(device_name, s_device_name_substring))
			{
				target_device_id = device_list[id];
			}
			break;
		}
	}

	//error checking
	if (target_platform_id == 0 || target_device_id == 0)
	{
		printf("Error getting target platform or device!\n");
	}
	//create a context
	context = clCreateContext(&context_properties, num_devices, device_list, NULL, NULL, &clStatus);
	file_contents = read_source("vecadd_anyD.cl", &file_size);
	program = clCreateProgramWithSource(context, 1, (const char **) &file_contents, &file_size, NULL);
	if (clBuildProgram(program,1,&target_device_id,"-cl-std=CL2.0",NULL,NULL) != CL_SUCCESS)
	{
		printf("failure to build program\n");
		exit(-1);
	}
	else {
		printf("Success building program\n");
	}
	command_queue = clCreateCommandQueueWithProperties(context, target_device_id, NULL, &clStatus);//default properties
	if (clStatus != CL_SUCCESS)
	{
		printf("Error creating command queue\n");
		exit(-1);
	}
	kernel = clCreateKernel(program, "vecadd_anyD", &clStatus);
	if (clStatus != CL_SUCCESS)
	{
		printf("Error creating kernel\n");
		exit(-1);
	}
	printf("succesfully built command queue and kernel\n");

	in_buf_a = (float *)_aligned_malloc(BUFFER_SIZE, 4096);
	in_buf_b = (float *)_aligned_malloc(BUFFER_SIZE , 4096);
	out_buf = (float *)_aligned_malloc(BUFFER_SIZE , 4096);
	
	//I'm hardcoding example buffers here...
	//We will add together two buffers with the same values
	for (int i = 0; i < NUM_ITEMS; i++)
	{
		in_buf_a[i] = i;
		in_buf_b[i] = i;
	}

	bufa = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, BUFFER_SIZE, (void*)in_buf_a, &clStatus);
	if (clStatus != CL_SUCCESS) 
	{
		printf("buffer allocation failed\n");
		exit(-1);

	}

	bufb = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, BUFFER_SIZE, (void*)in_buf_b, &clStatus);
	if (clStatus != CL_SUCCESS)
	{
		printf("buffer allocation failed\n");
		exit(-1);

	}

	outbuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, BUFFER_SIZE, (void*)out_buf, &clStatus);
	if (clStatus != CL_SUCCESS)
	{
		printf("buffer allocation failed\n");
		exit(-1);
	}
	
	printf("Successfully allocated buffers\n");
	clStatus = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufa);
	if (clStatus != CL_SUCCESS) {
		printf("failed to set kernel args %d\n", clStatus);
		exit(-1);
	}
	if (clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufb) != CL_SUCCESS) {
		printf("failed to set kernel args\n");
		exit(-1);
	}
	if (clSetKernelArg(kernel, 2, sizeof(cl_mem), &outbuf) != CL_SUCCESS) {
		printf("failed to set kernel args\n");
		exit(-1);
	}
	
	clStatus = clEnqueueNDRangeKernel(command_queue, kernel, NUM_WORK_DIMS, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	if (clStatus != CL_SUCCESS)
	{
		printf("failed to enquee kernel\n");
		exit(-1);
	}
	if (clFinish(command_queue) != CL_SUCCESS)
	{
		printf("Kernel OP failed\n");
		exit(-1);
	}
	mapped_buffer = (float *)clEnqueueMapBuffer(command_queue, outbuf, CL_TRUE, CL_MAP_READ, 0, BUFFER_SIZE, 0, NULL, NULL, &clStatus);
	if (clStatus != CL_SUCCESS)
	{
		printf("Enqueing map buffer failed\n");
		exit(-1);
	}
	clStatus = clEnqueueUnmapMemObject(command_queue, outbuf, mapped_buffer, 0, NULL, NULL);
	if (clStatus != CL_SUCCESS)
	{
		printf("Unmapping buffer failed\n");
		exit(-1);
	}

	if (verifyResults(in_buf_a, mapped_buffer, NUM_ITEMS))
	{
		printf("Verified Kernel Success!\n");
	}
	else 
	{
		printf("fail :(\n");
	}

	_aligned_free(in_buf_a);
	_aligned_free(in_buf_b);
	_aligned_free(out_buf);
	if (platform_name != NULL)
	{
		free(platform_name);
	}
	free(file_contents);
	free(platforms);
	free(device_list);
}

static char * _get_platform_name(cl_platform_id platform_id)
{
	size_t param_size;
	char * pn = NULL;
	clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, NULL, NULL, &param_size);
	pn = (char *)malloc(sizeof(char) * param_size);
	clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, param_size, pn, NULL);
	return pn;
}

static bool verifyResults(float *p_mappedBufferIN, float *p_mappedBufferOut, cl_int numValues)
{
	bool ret = true;
	for (int i = 0; i < numValues; i++) {
		float expected_val = (p_mappedBufferIN[i] * 2); //This is the same as adding the two identical input buffers
		if (expected_val != p_mappedBufferOut[i])
		{
			ret = false;
			break;
		}
		printf("Expected Value: %f, Output Value: %f\n", expected_val, p_mappedBufferOut[i]);
	}
	return ret;
}