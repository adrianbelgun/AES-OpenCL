#ifndef _OCL_DEVICE_H_
#define _OCL_DEVICE_H_
#include "stdafx.h"

class OCL_Device
{
private:
	cl_platform_id		m_platform_id;
	cl_device_id		m_device_id;
	cl_context			m_context;
	cl_command_queue	m_queue;

	char* m_sBuildOptions;

	// Uniquely maps a kernel via a program name and a kernel name
	std::map<std::string, std::pair<cl_program, 
		std::map<std::string, cl_kernel> > > m_kernels;
	
	// Maps each buffer to a index
	std::map<int, cl_mem> m_buffers;

	cl_program GetProgram(const char* sProgramName);

public:
	OCL_Device(int iPlatformNum, int iDeviceNum);
	~OCL_Device(void);

	void SetBuildOptions (const char* sBuildOptions);
	cl_kernel GetKernel (const char* sProgramName, const char* sKernelName);
	cl_mem DeviceMalloc(int idx, size_t size);
	void CopyBufferToDevice(void* h_Buffer, int idx, size_t size);
	void CopyBufferToHost  (void* h_Buffer, int idx, size_t size);

	cl_command_queue GetQueue();

	void PrintInfo(void);
};
#endif

