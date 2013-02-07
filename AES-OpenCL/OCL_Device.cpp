#include "stdafx.h"

OCL_Device::OCL_Device(int iPlatformNum, int iDeviceNum)
{
	// For error checking
	cl_int err;

	// Get Platfom Info
	cl_uint iNumPlatforms = 0;
	err = clGetPlatformIDs(NULL, NULL, &iNumPlatforms); 
	CHECK_OPENCL_ERROR(err);

	cl_platform_id* vPlatformIDs = 
		(cl_platform_id *) new cl_platform_id[iNumPlatforms];
	err = clGetPlatformIDs(iNumPlatforms, vPlatformIDs, NULL); 
	CHECK_OPENCL_ERROR(err);
	if (iPlatformNum >= iNumPlatforms)
	{
		printf("Platform index must me between 0 and %d.\n",iNumPlatforms-1);
		delete[] vPlatformIDs;
		return;
	}
	m_platform_id = vPlatformIDs[iPlatformNum];
	delete[] vPlatformIDs;

	// Get Device Info
	cl_uint iNumDevices = 0;
	err = clGetDeviceIDs(m_platform_id, CL_DEVICE_TYPE_ALL, NULL, NULL, 
		&iNumDevices); 
	CHECK_OPENCL_ERROR(err);

	cl_device_id* vDeviceIDs = (cl_device_id*) new cl_device_id[iNumDevices];	
	err = clGetDeviceIDs(m_platform_id, CL_DEVICE_TYPE_ALL, iNumDevices, 
		vDeviceIDs, &iNumDevices); 
	CHECK_OPENCL_ERROR(err);
	if (iDeviceNum >= iNumDevices)
	{
		printf("Device index must me between 0 and %d.\n", iNumDevices-1);
		delete[] vDeviceIDs;
		return;
	}
	m_device_id = vDeviceIDs[iDeviceNum];
	delete[] vDeviceIDs;

	cl_context_properties vProprieties[3] = {CL_CONTEXT_PLATFORM, 
		(cl_context_properties)m_platform_id, 0};
	m_context = clCreateContext(vProprieties, 1, &m_device_id, NULL, NULL, 
		&err); 
	CHECK_OPENCL_ERROR(err);

	m_queue = clCreateCommandQueue(m_context, m_device_id, NULL, &err); 
	CHECK_OPENCL_ERROR(err);
	
	char* m_sBuildOptions = "";
}

void OCL_Device::PrintInfo()
{	
	// Printing device and platform information
	printf("Using platform: ");
	PrintPlatformInfo(m_platform_id);
	printf("Using device:   ");
	PrintDeviceInfo(m_device_id);
}

cl_program OCL_Device::GetProgram (const char* sProgramName)
{
	std::string sFile = GetFileContents(sProgramName);
	const char* pFile = sFile.c_str();
	const size_t lFile = sFile.length();

	cl_int err;
	cl_program program = clCreateProgramWithSource(m_context, 1, 
		(const char** const)&pFile, &lFile, &err); 
	CHECK_OPENCL_ERROR(err);

	err = clBuildProgram(program, 1, &m_device_id, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		size_t size;
		clGetProgramBuildInfo (program, m_device_id, 
			CL_PROGRAM_BUILD_LOG, 0, NULL, &size);
		char* sLog = (char*) malloc (size);
		clGetProgramBuildInfo (program, m_device_id, 
			CL_PROGRAM_BUILD_LOG, size, sLog, NULL);
		
		printf ("\n");
		printf("Build Log:\n");
		printf("%s\n", sLog);
		free(sLog);

		CHECK_OPENCL_ERROR(err);
	}

	return program;
}

void OCL_Device::SetBuildOptions(const char* sBuildOptions)
{
	size_t iLen = strlen(sBuildOptions);
	m_sBuildOptions = (char*) malloc (iLen + 1);
	strncpy(m_sBuildOptions, sBuildOptions, iLen);
}

cl_kernel OCL_Device::GetKernel (const char* sProgramName, 
	const char* sKernelName)
{
	if (m_kernels.find(sProgramName) == m_kernels.end())
	{
		// Build program
		cl_program program = GetProgram(sProgramName);

		// Add to map
		m_kernels[sProgramName] = std::pair<cl_program, std::map<std::string, 
			cl_kernel> >(program, std::map<std::string, cl_kernel>());
	}

	if (m_kernels[sProgramName].second.find(sKernelName) == 
		m_kernels[sProgramName].second.end())
	{
		// Create kernel
		cl_int err;
		cl_kernel kernel = clCreateKernel(m_kernels[sProgramName].first, 
			sKernelName, &err);     
		CHECK_OPENCL_ERROR(err);
		
		// Add to map
		m_kernels[sProgramName].second[sKernelName] = kernel;
	}

	return m_kernels[sProgramName].second[sKernelName];
}

cl_mem OCL_Device::DeviceMalloc(int idx, size_t size)
{
	cl_int err;
	if (m_buffers.find(idx) != m_buffers.end())
	{
		err = clReleaseMemObject(m_buffers[idx]);
		CHECK_OPENCL_ERROR(err);
	}

	cl_mem mem = clCreateBuffer(m_context, CL_MEM_READ_WRITE, size, NULL, 
		&err);
	CHECK_OPENCL_ERROR(err);

	m_buffers[idx] = mem;
	
	return mem;
}

cl_command_queue OCL_Device::GetQueue()
{
	return m_queue;
}


void OCL_Device::CopyBufferToDevice(void* h_Buffer, int idx, size_t size)
{
	
	cl_int err = clEnqueueWriteBuffer (m_queue, m_buffers[idx], CL_TRUE, 0, 
		size, h_Buffer, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(err);
}

void OCL_Device::CopyBufferToHost  (void* h_Buffer, int idx, size_t size)
{
	cl_int err = clEnqueueReadBuffer (m_queue, m_buffers[idx], CL_TRUE, 0, 
		size, h_Buffer, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(err);
}

OCL_Device::~OCL_Device(void)
{
	// Clean buffer for build options
	free(m_sBuildOptions);

	// Clean OpenCL Buffers
	for (std::map<int, cl_mem>::iterator it = m_buffers.begin() ; 
		it != m_buffers.end(); it++ )
	{
		// Release Buffer
		clReleaseMemObject(it->second);
	}

	// Clean OpenCL Programs and Kernels
	for (std::map<std::string, std::pair<cl_program, 
		std::map<std::string, cl_kernel> > >::iterator it = m_kernels.begin(); 
		it != m_kernels.end(); it++ )
	{
		for (std::map<std::string, cl_kernel>::iterator 
			it2 = it->second.second.begin(); 
			it2 != it->second.second.end(); it2++ )
		{
			clReleaseKernel(it2->second);
		}	
		clReleaseProgram(it->second.first);
	}	
}
