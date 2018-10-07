// Add you host code
#include<stdio.h>
#include<stdlib.h>
#include"CL/cl.h"
#include <string.h>
#define ERR 0xdeadbeef

static const char s_target_platform[] = "Intel(R) OpenCL";
static const char s_device_name_substring[] = "Intel";

static char * _get_platform_name(cl_platform_id platform_id);

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
			if (clStatus != CL_SUCCESS)
			{
				printf("Error getting platform or device\n");
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

	if (platform_name != NULL)
	{
		free(platform_name);
	}
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